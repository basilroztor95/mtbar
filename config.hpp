#ifndef config_hpp
#define config_hpp

#include <string>
#include <vector>

/* delimiter between modules */
static const std::string delim(" | ");
static const std::string delimBegin("");
static const std::string delimEnd("");

/*
  list of modules
. module-name: is name or path to a script or program.
    can start with ~/... for stuff in your home directory.
. refresh-interval: in seconds, 0 means only update on receiving a real-time signal
. signal-id: SIGRTMIN signal id; must be between 0 and 30.*/

/* module-name, refresh-interval, signal-id */
static const std::vector< std::vector<std::string> > topModuleList = {
    {"ping -c 1 -q 8.8.8.8 &> /dev/null && echo OK || echo FAIL", "5", "2"},
    {"volume.sh", "0", "10"},
#ifdef LAPTOP
    {"echo laptop", "60", "15"},
#endif
    {"usd.sh", "3600", "6"},
    {"cpu-load.sh", "1", "4"},
    {"netusage.sh", "1", "3"},
    {"date +%T", "1", "1"},
};

/* enable if you have the dwm-extrabar patch. */
static const bool twoBars = false;

/* the delimiter used by the dwm-extrabar patch. */
static const std::string botTopDelimiter(";");

/* list of modules for extra (bottom) bar */
static const std::vector< std::vector<std::string> > bottomModuleList = {};

#endif
