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

void Dictionary::writeToDisk() {
    ofstream output("dict.txt");
    for(auto & i : Entry)
        output << i.first <<"#"<<i.second<<"\n";
    output.close();
}

set<pair<int, int> >::iterator Dictionary::Erase(set<pair<int, int> >::iterator& it) {
    auto res = Entry.erase(it);
    writeToDisk();
    return res;
}

void Dictionary::readFromDisk() {
    cout << "Reading dictionary from disk...\n";
    ifstream input("dict.txt");
    Entry.clear();
    string str;
    while(getline(input, str)) {
        int idx = str.find('#');
        int a = stoi(str.substr(0, idx));
        int b = stoi(str.substr(idx+1));
        Entry.insert({a, b});
    }
    input.close();
}


const string Event::covToString() const{
    string s = "";
    s += to_string(userID);
    s += " ";
    s += to_string(clock);
    s += " ";
    int len = op.length();//record the length of op for parsing
    s += to_string(len);
    s += " ";
    s += op;
    return s;
}

const string Event::convToStringForViewlog() const {
    string s = " | ";
    s += to_string(userID);
    s += " | ";
    s += to_string(clock);
    s += " | ";
    int len = op.length();//record the length of op for parsing
    s += to_string(len);
    s += " | ";
    s += op;
    s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
    s += " | ";
    int cur = s.size();
    s += "\n"+string(cur, '-');
    return s;
}

const string Event::convToStringForStoring() const {
    string s = to_string(userID)+"#"+to_string(clock)+"#"+op;
    s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
    return s;
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


void Dictionary::Insert(int user1, int user2){
    Entry.insert(make_pair(user1, user2));
    writeToDisk();
}
void Dictionary::Delete(int user1, int user2){
    if(Entry.find(make_pair(user1, user2)) != Entry.end()){
        Entry.erase(make_pair(user1,user2));
    }else{
        cerr << "Not able to unblock because it's not blocked";
    }
    writeToDisk();
}
void Dictionary::updatePartialLog(string op, int clock, int userID){
    Event newEvent;
    newEvent.op = op;
    newEvent.clock = clock;
    newEvent.userID = userID;
    PL.insert(newEvent);
}



