#include "paxosmsg.h"

PaxosMsg::PaxosMsg(string& msg) {
    deserialize(msg);
}

PaxosMsg::PaxosMsg(string type, int num) {
    
}

PaxosMsg::PaxosMsg(string type, int accNum, Event value) {
    
}

PaxosMsg::PaxosMsg(string type, int slot, int accNum, Event value) {
    
}

string PaxosMsg::serialize() {
    
}

void PaxosMsg::deserialize(string& msg) {
    
}
