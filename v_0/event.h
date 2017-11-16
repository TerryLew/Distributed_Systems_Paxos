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
=======
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
>>>>>>> 7f02ef0a5c923f4e0327372befce5a6fe6e1174e
