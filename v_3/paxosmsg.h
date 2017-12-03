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
    PaxosMsg(string type, int senderId);
    PaxosMsg(string type, int senderId, int slot, int num);
    PaxosMsg(string type, int senderId, int slot, int num, Event value);
    PaxosMsg(string type, int senderId, int slot, Event value);
    PaxosMsg(string type, int senderId, int slot);
    
    string getType() { return _type; }
    int getSenderId() { return _senderId; }
    int getSlot() { return _slot; }
    int getNum() { return _num; }
    Event getValue() { return _value; }
    
    string serialize();
    void deserialize(string& msg);
};
