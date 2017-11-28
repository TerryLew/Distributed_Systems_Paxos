#include "event.h"

using namespace std;

class PaxosMsg {
private:
    string _type;
    int _senderId;
    int _slot;
    int _num;
    Event _value;
    
public:
    PaxosMsg(string& msg);
    PaxosMsg(string type, int slot);
    PaxosMsg(string type, int proposer, int slot);
    PaxosMsg(string type, int num, int proposer, int slot);
    PaxosMsg(string type, int accNum, Event value);
    PaxosMsg(string type, int slot, int accNum, Event value);
    PaxosMsg(string type, int accNum, Event value, int proposer);
    
    string getType() { return _type; }
    int getSenderId() { return _senderId; }
    int getSlot() { return _slot; }
    int getNum() { return _num; }
    Event getValue() { return _value; }
    
    string serialize();
    void deserialize(string& msg);
};
