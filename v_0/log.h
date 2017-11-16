//
//  Log.hpp
//  
//
//  Created by Ni Zhang on 10/2/17.
//
//

#ifndef LogAndDictionary_hpp
#define LogAndDictionary_hpp

#include <stdio.h>
#include "tweet.h"
#include <set>
#include <string>
#include <fstream>
#include <algorithm>

using namespace std;

class Event{
public:
    int userId;
    string op;
    string data;

    const string covToString() const;
    const string convToStringForViewlog() const;
    const string convToStringForStoring() const;

};

struct comp {
    bool operator()(const Event& a, const Event& b) {
        return a.covToString() < b.covToString();
    }
};

class Log {
public:
    //Log();
    vector<Event> Events;
    void writeToDisk();
    void readFromDisk();
    void insertLog(string op, int clock, int userID);
};



#endif
