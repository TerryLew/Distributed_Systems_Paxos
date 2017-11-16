//
//  tweet.cpp
//  
//
//  Created by Ni Zhang on 10/2/17.
//
//

#include "tweet.h"
#include <time.h>

using namespace std;

Tweet::Tweet(Event& e) {
    userId = e.getUserId();
    message = e.getData();
    
    struct tm& ot;
    convertTime(e.getLocalTime(), ot);
    local = mktime(&ot);
    
    convertTime(e.getUtcTime(), ot);
    utc = mktime(&ot);
}


bool comparetime(const Tweet &i, const Tweet &j) {  return difftime(i.getUtcTime(),j.getUtcTime()) > 0.0;}

void convertTime(string itime, struct tm& ot){
    unordered_map<string, int> wdayt = { { "Sun" , 0 },
        { "Mon" , 1 },
        { "Tue" , 2 },
        { "Wed" , 3 },
        { "Thu" , 4 },
        { "Fri" , 5 },
        { "Sat" , 6 } };
    unordered_map<string, int> mont = { { "Jan" , 0 },
        { "Feb" , 1 },
        { "Mar" , 2 },
        { "Apr" , 3 },
        { "May" , 4 },
        { "Jun" , 5 },
        { "Jul" , 6 },
        { "Aug" , 7 },
        { "Sep" , 8 },
        { "Oct" , 9 },
        { "Nov" , 10 },
        { "Dec" , 11 } };
    string wday, mon, day, yy, t;
    int hour, min, sec;
    istringstream ss(itime);
    ss >> wday >> mon >> day >> t >> yy;
    hour = stoi(t.substr(0,2));
    min = stoi(t.substr(3,2));
    sec = stoi(t.substr(6,2));
    
    ot.tm_mday = stoi(day);
    ot.tm_hour = hour;
    ot.tm_min = min;
    ot.tm_sec = sec;
    ot.tm_year = stoi(yy)-1900;
    
    ot.tm_wday = wdayt[wday];
    ot.tm_mon = mont[mon];
}


