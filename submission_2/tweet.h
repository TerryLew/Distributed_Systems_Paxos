#ifndef tweet_hpp
#define tweet_hpp

#include <stdio.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <unordered_map>
#include <string>
#include <sstream>

#include "event.h"

using namespace std;

class Tweet {
private:
    int userId;
    string data;
    time_t local;
    time_t utc;
    
    //record time in two format with the first being UTC to compare and second being local time for printing
public:
    Tweet(Event e);
    
    int getUserId() { return userId; }
    string getData() { return data; }
    time_t getUtcTime() const { return utc; }
    time_t getLocalTime() const { return local; }
};

bool comparetime(const Tweet& i,const Tweet& j);


#endif /* tweet_hpp */
