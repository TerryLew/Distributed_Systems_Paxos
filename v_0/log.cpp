//
//  Log.cpp
//  
//
//  Created by Ni Zhang on 10/2/17.
//
//

#include "log.h"
#include <set>
#include <iostream>
#include <string>

using namespace std;

void Log::writeToDisk() {
    ofstream output("log.txt");
    for(auto & i : Events)
        output << i.convToStringForStoring() << "\n";
    output.close();
}

void Log::readFromDisk() {
    cout << "Reading log from disk...\n";
    ifstream input("log.txt");
    Events.clear();
    string str;
    while(getline(input, str)) {
        int idx1 = str.find('#');
        int idx2 = str.find('#', idx1+1);
        Event e;
        e.userID = stoi(str.substr(0, idx1));
        e.clock = stoi(str.substr(idx1+1, idx2-idx1));
        e.op = str.substr(idx2+1);
        Events.insert(e);
    }
    input.close();
}

void Log::updateLog(string op, int clock, int userID){
    Event newEvent;
    newEvent.op = op;
    newEvent.clock = clock;
    newEvent.userID = userID;
    Events.insert(newEvent);
    writeToDisk();
}

void Log::updateLogUnblock(string op, int clock, int userID, int unblockID){
    bool caninsert = true;
    for(auto it = Events.begin(); it != Events.end();){
        if((*it).userID == userID && (*it).op == "block " + to_string(unblockID)){
            it = Events.erase(it);
            caninsert = false;
        }else{
            it++;
        }
    }
    
    if(caninsert){
        Event newEvent;
        newEvent.op = op;
        newEvent.clock = clock;
        newEvent.userID = userID;
        Events.insert(newEvent);
    }
    writeToDisk();
}



