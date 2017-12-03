#include "event.h"
#include <string>
#include <sstream>
#include <iostream>


Event::Event() {
    _empty=true;
}

Event::Event(string& str) {
    string userIdStr;
    istringstream ss(str);
    getline(ss, userIdStr, '#');
    cout << "here1" << endl;
    _userId = stoi(userIdStr);
    getline(ss, _op, '#');
    cout << "here2" << endl;
    getline(ss, _localTime, '#');
    getline(ss, _utcTime, '#');
    getline(ss, _data, '#');
    cout << "data in event is " << _data << endl;
    _empty = false;
}


//serialize event to be "userid op datalen#data#localtime#utctime"
const string Event::serialize() const {
    if(isEmpty()) return "";
    string s = to_string(_userId) + "#" +
                _op + "#" +
                _localTime + "#" +
                _utcTime+"#" +
                _data;
    s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
    return s;
}

const string Event::serializeForView() const {
    if (isEmpty()) return "[-]";
    string s = "\n\t[ site: " + to_string(_userId);
    s += ",\n\t  op: " + _op;
    s += ",\n\t  local: "+_localTime;
    s.erase(std::remove(s.begin()+s.size()-1, s.end(), '\n'), s.end());
    s += ",\n\t  utc: "+_utcTime;
    s.erase(std::remove(s.begin()+s.size()-1, s.end(), '\n'), s.end());
    s += ",\n\t  data: " + _data;
    s.erase(std::remove(s.begin()+s.size()-1, s.end(), '\n'), s.end());
    s += "\n\t]";
    return s;
}

const string Event::serializeForStoring() const {
    if (isEmpty()) return "";
    string s = to_string(_userId) + "#" + _op;
    s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
    s += "#" + _localTime + "#"+_utcTime+"#"+_data;
    return s;
}

void Event::populate(int userId, string op, string data, string localTime, string utcTime) {
    _userId = userId;
    _op = op;
    _data = data;
    _localTime = localTime;
    _utcTime = utcTime;
    _empty = false;
}
