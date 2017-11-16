#ifndef _log_h_
#define _log_h_


#include <stdio.h>
#include <set>
#include <string>
#include <fstream>
#include <algorithm>

#include "tweet.h"
#include "event.h"

using namespace std;

struct comp {
    bool operator()(const Event& a, const Event& b) {
        return a.serialize() < b.serialize();
    }
};

class Log {
public:
    void writeToDisk();
    void readFromDisk();
    void addToLog(int slot, Event e);
    void display();
private:
    vector<Event> Events;
};

#endif
