#include "paxosmsg.h"
#include <string>
#include <sstream>
#include <iostream>


PaxosMsg::PaxosMsg(string& msg) {
    deserialize(msg);
}

PaxosMsg::PaxosMsg(string type, int senderId, int slot, int num){
    _type = type;
    _slot = slot;
    _num = num;
    _senderId = senderId;
}

PaxosMsg::PaxosMsg(string type, int senderId, int slot, int num, Event value) {
    _type = type;
    _senderId = senderId;
    _slot = slot;
    _num = num;
    _value = value;
}

PaxosMsg::PaxosMsg(string type, int senderId, int slot, Event value) {
    _type = type;
    _senderId = senderId;
    _slot = slot;
    _value = value;
}

PaxosMsg::PaxosMsg(string type, int senderId, int slot) {
    _type = type;
    _senderId = senderId;
    _slot = slot;
}


string PaxosMsg::serialize() {
    string str = _type + " " + to_string(_senderId) + " " + to_string(_slot) + " ";
    if(_type == "prepare"){
        str += to_string(_num);
    } else if(_type == "promise" || _type == "accept" || _type == "ack"){
        str += to_string(_num);
        str += " ";
        str += _value.serialize();
    } else if(_type == "commit"){
        str += _value.serialize();
    }
    return str;
}

void PaxosMsg::deserialize(string& msg) {
    //cout << "paxos deserialize: "<<msg << "\n";
    stringstream ss(msg);
    string cont;
    ss >> _type >> _senderId >> _slot;
    if(_type == "prepare"){
        ss >> _num;
    } else if(_type == "promise"){
        ss >> _num;
        if (_num == -1) _value = Event();
        else {
            getline(ss, cont);
            _value = Event(cont);
        }
    } else if(_type == "accept" || _type == "ack"){
        ss >> _num;
        getline(ss, cont);
        _value = Event(cont);
    } else if(_type == "commit"){
        getline(ss, cont);
        _value = Event(cont);
    }
}
