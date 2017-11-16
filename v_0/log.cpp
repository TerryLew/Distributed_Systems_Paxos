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
        output << i.serializeForStoring() << "\n";
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

void Log::addToLog(int slot, Event e) {
    if (slot < Events.size()) Events[slot]=e;
    else Events.push_back(e);
}

void Log::display() {
    cout <<"____________________________________________\n";
    for(auto& itr = Events.begin(); itr != Events.end(); ++itr){
        cout << itr->serializeForView() << "\n";
    }
}



