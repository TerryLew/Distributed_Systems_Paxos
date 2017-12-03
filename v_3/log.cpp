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

void Log::writeToDisk(int userId) {
    string filename = "logn" + to_string(userId);
    ofstream output(filename.c_str());
    for(auto & i : Events)
        output << i.serializeForStoring() << "\n";
    output.close();
}

void Log::readFromDisk(int userId) {
    string filename = "logn" + to_string(userId);

    cout << "Reading " << filename << " from disk...\n";
    ifstream input(filename.c_str());
    //Events.clear();
    string str;
    while(getline(input, str)) {
        cout << str << endl;
        Event e(str);
        Events.push_back(e);
    }
    cout << "size is " << Events.size() <<endl;

    input.close();
}

void Log::addToLog(int slot, Event e) {
    if (slot < Events.size()) Events[slot]=e;
    else Events.push_back(e);
}

void Log::display() {
    cout <<"--------------------------------------------\n";
    cout << "size is " << Events.size() <<endl;
    for(auto itr = Events.begin(); itr != Events.end(); ++itr){
        cout << itr->serializeForView() << "\n";
        cout <<"--------------------------------------------\n";
    }
}



