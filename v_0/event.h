
class Event{
private:
    int _userId;
    string _op;
    string _data;
    struct tm* _localTime;
    struct tm* _utcTime;
    
public:
    
    int getUserId() { return _userId; }
    string getOp() { return _op; }
    string getData() { return _data; }
    struct tm* getLocalTime { return _localTime; }
    struct tm* getUtcTime() { return _utcTime; }
    
    const string serialize() const;
    const string serializeForView() const;
    const string serializeForStoring() const;
    
    void populate(int userId, string op, string data, struct tm* localTime, struct tm* utcTime);
};
