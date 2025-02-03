/* C++ modules for the status bar (implementation) */
/* Implementation of classes that provide output useful for display in the status bar. */

#include <cstddef>
#include <cstdio>
#include <sys/statvfs.h>
#include <ios>
#include <string>
#include <sstream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "modules.hpp"

using std::string;
using std::stringstream;
using std::fstream;
using std::ios;
using std::mutex;
using std::unique_lock;
using std::chrono::seconds;

using namespace DWMBspace;

void Module::operator()() const {
    if (refreshInterval_) { // if not zero, do a time-lapse loop
        mutex mtx;
        while (true) {
            runModule_();
            unique_lock<mutex> lk(mtx);
            signalCondition_->wait_for(lk, seconds(refreshInterval_));
        }
    } else { // wait for a real-time signal
        runModule_();
        mutex mtx;
        while (true) {
            unique_lock<mutex> lk(mtx);
            signalCondition_->wait(lk);
            runModule_();
            lk.unlock();
        }
    }
}

/* static member */
const size_t ModuleExtern::lengthLimit_ = 500;

void ModuleExtern::runModule_() const {
    char buffer[100];
    string output;
    FILE *pipe = popen(extCommand_.c_str(), "r");
    if (!pipe) { // fail silently
        return;
    }
    while ( !feof(pipe) ) {
        if (fgets(buffer, 100, pipe) != NULL) {
            output += buffer;
        }
        if (output.size() > lengthLimit_) {
            output.erase( output.begin()+lengthLimit_, output.end() );
            break;
        }
    }
    pclose(pipe);
    mutex mtx;
    unique_lock<mutex> lk(mtx);
    *outString_ = output;
    outputCondition_->notify_one();
    lk.unlock();
}
