#include "event.h"
#include <string>
#include <sstream>
#include <iostream>


Event::Event() {
    
}

Event::Event(string& str) {
    stringstream ss(str);
    string cont, str2;
    ss >> cont;
    _userId = stoi(cont);
    ss >> cont;
    _op = cont;
    getline(ss, str2);
    stringstream ss2(str2);
    getline(ss2, cont, '#');
    int datalen = stoi(cont);
    _data = str2.substr(to_string(datalen).length()+1, datalen);
    stringstream left(str2.substr(to_string(datalen).length()+2+ datalen));
    getline(left, cont, '#');
    cout << "localtime: " << cont << endl; //might need to convert to ctime
    _localTime = cont;
    getline(left, cont, '#');
    cout << "utctime: " << cont << endl; //might need to convert to ctime
    _utcTime = cont;
    
}


//serialize event to be "userid op datalen#data#localtime#utctime"
const string Event::serialize() const {
    string s = "";
    s += to_string(_userId);
    s += " ";
    s += _op;
    s += " ";
    int datalen = _data.length();
    s += to_string(datalen) + "#" + _data + "#" + _localTime + "#" + _utcTime + "#";
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
    _userId = userId;
    _op = op;
    _data = data;
    _localTime = localTime;
    _utcTime = utcTime;
}
