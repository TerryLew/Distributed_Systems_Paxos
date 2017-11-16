class PaxosMsg {
private:
    int _proposer;
    int _slot;
    int _num;
    Event _value;
    
public:
    PaxosMsg(string& msg);
    PaxosMsg(string type, int num);
    PaxosMsg(string type, int accNum, Event value);
    PaxosMsg(string type, int slot, int accNum, Event value);
    
    int getProposer() { return _proposer; }
    int getSlot() { return _slot; }
    int getNum() { return _num; }
    Event getValue() { return _value; }
    
    string serialize();
    void deserialize(string& msg);
};
=======
class PaxosMsg {
private:
    int _proposer;
    int _slot;
    int _num;
    Event _value;
    
public:
    PaxosMsg(string& msg);
    PaxosMsg(string type, int num);
    PaxosMsg(string type, int accNum, Event value);
    PaxosMsg(string type, int slot, int accNum, Event value);
    
    int getProposer() { return _proposer; }
    int getSlot() { return _slot; }
    int getNum() { return _num; }
    Event getValue() { return _value; }
    
    string serialize();
    void deserialize(string& msg);
};
>>>>>>> 7f02ef0a5c923f4e0327372befce5a6fe6e1174e
