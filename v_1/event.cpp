#include "event.h"
#include <string>
#include <sstream>
#include <iostream>


Event::Event() {
    
}

Event::Event(string& str) {
    stringstream ss(str);
    string cont;
    
    ss >> cont;
    _userId = stoi(cont);
    ss >> cont;
    int len = stoi(cont);
    ss >> cont;
    _op = cont.substr(0, len);
}

const string Event::serialize() const {
    string s = "";
    s += to_string(_userId);
    s += " ";
    int len = _op.length();//record the length of op for parsing
    s += to_string(len);
    s += " ";
    s += _op;
    return s;
}

const string Event::serializeForView() const {
    string s = " | ";
    s += to_string(_userId);
    s += " | ";
    int len = _op.length();//record the length of op for parsing
    s += to_string(len);
    s += " | ";
    s += _op;
    s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
    s += " | ";
    int cur = s.size();
    s += "\n"+string(cur, '-');
    return s;
}

const string Event::serializeForStoring() const {
    string s = to_string(_userId)+"#"+_op;
    s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
    return s;
}

void Event::populate(int userId, string op, string data, string localTime, string utcTime) {
    
}
