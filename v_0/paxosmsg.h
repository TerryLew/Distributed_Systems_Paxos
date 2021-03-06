#include "event.h"

using namespace std;

class PaxosMsg {
private:
    string _type;
    int _proposer;
    int _slot;
    int _num;
    Event _value;
    
public:
    PaxosMsg(string& msg);
    PaxosMsg(string type, int num);
    PaxosMsg(string type, int accNum, Event value);
    PaxosMsg(string type, int slot, int accNum, Event value);
    
    string getType() { return _type; }
    int getProposer() { return _proposer; }
    int getSlot() { return _slot; }
    int getNum() { return _num; }
    Event getValue() { return _value; }
    
    string serialize();
    void deserialize(string& msg);
};
