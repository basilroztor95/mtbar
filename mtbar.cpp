#include <X11/Xlib.h>
#include <bits/stdint-intn.h>
#include <csignal>
#include <cstddef>
#include <iostream>
#include <regex>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "modules.hpp"
#include "config.hpp"

using std::string;
using std::stoi;
using std::vector;
using std::thread;
using std::mutex;
using std::unique_lock;
using std::condition_variable;
using std::chrono::seconds;
using std::cerr;

using namespace DWMBspace;

/* number of possible real-time signals */
static const int sigRTNUM = 30;
/* condition variables that will respond to real-time signals */
static vector<condition_variable> signalCondition(sigRTNUM);

/*
 * make bar output
 * takes individual module outputs and puts them together for printing.
 * moduleOutput: vector of individual module outputs
 * delimiter: delimiter character(s) between modules
 * barText: compiled text to be printed to the bar
 */
void makeBarOutput(const vector<string> &moduleOutput, const string &delimiter, string &barText)
{
    barText.clear();
    for (auto moIt = moduleOutput.begin(); moIt != (moduleOutput.end() - 1); ++moIt) {
        barText += (*moIt) + delimiter;
    }
    barText += moduleOutput.back();
}

/*
 * render the bar
 * renders the bar text by printing the provided string to the root window.
 * This is how dwm handles status bars.
 * barOutput: text to be displayed
 */
void printRoot(const string &barOutput)
{
    Display *d = XOpenDisplay(NULL);
    if (d == nullptr) {
        return;         // fail silently
    }
    const int32_t screen = DefaultScreen(d);
    const Window root    = RootWindow(d, screen);
    XStoreName( d, root, barOutput.c_str() );
    XCloseDisplay(d);
}

/*
 * process real-time signals
 * receive and process real-time signals to trigger relevant modules.
 * sig: signal number (starting at `SIGRTMIN`)
 */
void processSignal(int sig)
{
    /* do nothing silently if wrong signal received */
    if ( (sig < SIGRTMIN) || (sig > SIGRTMAX) ) {
        return;
    }
    size_t sigInd = sig - SIGRTMIN;
    signalCondition[sigInd].notify_one();
}

int main()
{
    for (int sigID = SIGRTMIN; sigID <= SIGRTMAX; sigID++) {
        signal(sigID, processSignal);
    }

    /* the stuff enclosed in ^ characters are for the status2d patch */
    static const std::string delimFull = "" + delim + "";

    /* string that comes before the first module */
    static const std::string delimBeginFull = "" + delimBegin + "";

    /* string that comes after the last module */
    static const std::string delimEndFull = "" + delimEnd;

    mutex mtx;
    condition_variable commonCond; /* this triggers printing to the bar from individual modules */
    vector<string> topModuleOutputs( topModuleList.size() );
    vector<thread> moduleThreads;
    size_t moduleID = 0;

    for (auto &tb : topModuleList) {

        if (tb.size() != 3) {
            cerr << "ERROR: top bar module description vector must have exactly three elements, yours has " << tb.size() << " (module " << tb[0] << ")\n";
            exit(1);
        }

        int32_t interval = stoi(tb[1]);
        if (interval < 0) {
            cerr << "ERROR: refresh interval cannot be negative, yours is " << interval << " (module " << tb[0] << ")\n";
            exit(2);
        }

        int32_t rtSig = stoi(tb[2]);
        if (rtSig < 0) {
            cerr << "ERROR: real-time signal cannot be negative, yours is " << rtSig << " (module " << tb[0] << ")\n";
            exit(3);
        }

        moduleThreads.push_back(thread{ModuleExtern(interval, tb[0], &topModuleOutputs[moduleID], &commonCond, &signalCondition[rtSig])});
        moduleID++;
    }

    vector<string> bottomModuleOutputs;
    if (twoBars) {
        bottomModuleOutputs.resize( bottomModuleList.size() );
        moduleID = 0;

        for (auto &bb : bottomModuleList) {

            if (bb.size() != 3) {
                cerr << "ERROR: top bar module description vector must be have exactly four elements, yours has " << bb.size() << " (module " << bb[0] << ")\n";
                exit(1);
            }

            int32_t interval = stoi(bb[1]);
            if (interval < 0) {
                cerr << "ERROR: refresh interval cannot be negative, yours is " << interval << " (module " << bb[0] << ")\n";
                exit(2);
            }

            int32_t rtSig = stoi(bb[2]);
            if (rtSig < 0) {
                cerr << "ERROR: real-time signal cannot be negative, yours is " << rtSig << " (module " << bb[0] << ")\n";
                exit(3);
            }

            moduleThreads.push_back(thread{ModuleExtern(interval, bb[0], &bottomModuleOutputs[moduleID], &commonCond, &signalCondition[rtSig])});
            moduleID++;
        }
    }

    string barTextBottom;
    string barText;

    while (true) {

        unique_lock<mutex> lk(mtx);
        commonCond.wait(lk);
        makeBarOutput(topModuleOutputs, delimFull, barText);
        barText = delimBeginFull + barText + delimEndFull;

        if (twoBars) {
            makeBarOutput(bottomModuleOutputs, delimFull, barTextBottom);
            barText = barText + botTopDelimiter + delimBeginFull + barTextBottom + delimEndFull;
        }
        lk.unlock();

        /* replace newline characters with a empty string */
        std::regex newlines_regex("\n+");
        barText = std::regex_replace(barText,newlines_regex,"");

        printRoot(barText);
    }

    for (auto &t : moduleThreads) {
        if ( t.joinable() ) {
            t.join();
        }
    }
    exit(0);
}
