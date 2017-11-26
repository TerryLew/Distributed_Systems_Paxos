#include "paxosmsg.h"
#include <string>
#include <sstream>
#include <iostream>


PaxosMsg::PaxosMsg(string& msg) {
    deserialize(msg);
}

PaxosMsg::PaxosMsg(string type, int slot){
    _type = type;
    _slot = slot;
}
PaxosMsg::PaxosMsg(string type, int num, int proposer, int slot) {
    _type = type;
    _num = num;
    _proposer = proposer;
    _slot = slot;
}

PaxosMsg::PaxosMsg(string type, int accNum, Event value) {
    _type = type;
    if(type == "promise"){
        _num = accNum;
        _value = value;
    }else if(type == "commit"){
        _slot = accNum;
        _value = value;
    }

}

PaxosMsg::PaxosMsg(string type, int slot, int accNum, Event value) {
    _type = type;
    _num = accNum;
    _value = value;
    _slot = slot;
    
}

PaxosMsg::PaxosMsg(string type, int accNum, Event value, int proposer) {
    _type = type;
    _num = accNum;
    _value = value;
    _proposer = proposer;
}


string PaxosMsg::serialize() {
    string str = _type;
    if(_type == "prepare"){
        str += " ";
        str += to_string(_num);
        str += " ";
        str += to_string(_proposer);
        str += " ";
        str += to_string(_slot);
        
    }else if(_type == "promise"){
        str += " ";
        str += to_string(_num);
        str += " ";
        str += _value.serialize();
    }else if(_type == "accept"){
        str += " ";
        str += to_string(_num);
        str += " ";
        str += to_string(_slot);
        str += " ";
        str += _value.serialize();
    }else if(_type == "ack"){
        str += " ";
        str += to_string(_num);
        str += " ";
        str += to_string(_proposer);
        str += " ";
        str += _value.serialize();
        
    }else if(_type == "commit"){
        str += " ";
        str += to_string(_slot);
        str += " ";
        str += _value.serialize();

    }else if(_type == "try_again"){
        str += " ";
        str += to_string(_slot);
    }
    
    return str;
    
}

void PaxosMsg::deserialize(string& msg) {
    stringstream ss(msg);
    string cont;
    ss >> cont;
    _type = cont;
    ss >> cont;
    if(_type == "prepare"){
        _num = stoi(cont);
        ss >> cont;
        _proposer = stoi(cont);
        ss >> cont;
        _slot = stoi(cont);
    }else if(_type == "promise" ){
        _num = stoi(cont);
        ss >> cont;
        Event e(cont);
        
    }else if(_type == "accept"){
        _num = stoi(cont);
        ss >> cont;
        _slot = stoi(cont);
        ss >> cont;
        Event e(cont);
    }else if(_type == "ack"){
        _num = stoi(cont);
        ss >> cont;
        _proposer = stoi(cont);
        ss >> cont;
        Event e(cont);
    }else if(_type == "commit"){
        _slot = stoi(cont);
        ss >> cont;
        Event e(cont);
    }else if(_type == "try_again"){
        _slot = stoi(cont);
    }
}
