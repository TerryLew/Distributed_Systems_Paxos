
class Event{
private:
    int _userId;
    string _op;
    string _data;
    string _localTime;
    string _utcTime;
    
public:
    
    int getUserId() { return _userId; }
    string getOp() { return _op; }
    string getData() { return _data; }
    string getLocalTime { return _localTime; }
    string getUtcTime() { return _utcTime; }
    
    const string serialize() const;
    const string serializeForView() const;
    const string serializeForStoring() const;
    
    void populate(int userId, string op, string data, string localTime, string utcTime);
    bool isDummy() { return op==""; }
};
