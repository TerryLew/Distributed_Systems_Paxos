#include "event.h"

const string Event::serialize() const{
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

const string Event::serializeForView() const {
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

const string Event::serializeForStoring() const {
    string s = to_string(userID)+"#"+to_string(clock)+"#"+op;
    s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
    return s;
}

void Event::populate(int userId, string op, string data, string localTime, string utcTime) {
    
}
