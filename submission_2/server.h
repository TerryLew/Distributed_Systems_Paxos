#ifndef _server_h_
#define _server_h_

#include <iostream>
#include <vector>
#include <string>
#include <list>

#include <mutex>
#include <climits>
#include <algorithm>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <unordered_set>
#include <unordered_map>

#include "mythread.h"
#include "log.h"
#include "tweet.h"
#include "event.h"
#include "paxosmsg.h"

#define BUFFER_SIZE 256

using namespace std;

class Server {

  private:
    int serverSock;
    struct sockaddr_in serverAddr, clientAddr;
    set<pair<int, int> > fdv;
    unordered_map<int, pair<string, string> > addr;
    
    int userId;
    int numSites;
    int sended_count;
    bool okToTweet;
    Log log;
    list<Event> timeline;
    unordered_set<int> isBlockedBy;
    unordered_set<int> blockedUser;
    
    int maxSlot;
    int curSlot;
    int myProposal;
    Event myVal;
    vector<int> maxPrepare;
    vector<int> accNum;
    vector<Event> accVal;
    vector<int> accNum_tmp;
    vector<Event> accVal_tmp;
    vector<int> majorityPromise;
    vector<int> majorityAck;
    int receivedTryAgainFrom;

  public:
    Server(int siteId, unordered_map<int, pair<string, string> >& address, int numSites);
    
    void start();
    static void* startTweet(void *args);
    void doStartTweet();
    static void * HandleClient(void *args);
    void doHandleClient(int fd);
    static void* acceptRequest(void *args);
    void doAcceptRequest();
    void refreshConnection();
    
    void doSomethingWithReceivedData(string& message);
    int chooseProposalNumber();
    void updateTimeline(string keyword, int target = -1);
    void updateInDiskData();
    void rebuildInMemoryData();
    
    void synod();
    void prepare(int slot);
    void promise(int proposer, int slot, int accNum, Event accVal);
    void Accept(int slot, int accNum, Event accVal);
    void ack(int proposer, int slot, int accNum, Event accVal);
    void commit(int slot, Event accVal);
    void try_again(int proposer, int slot);
    void tryagainSynod(int senderId);
    
    void block(int target);
    void unblock(int target);
    void view();
    void viewLog();
    void viewBlock();
    void viewIsBlockedBy();
    
    void restoreInfo();//getback the maxPrepare, accNum and accVal from the disk
    void console(string bound, PaxosMsg& paxosMsg, int recver = -1);
    
};

#endif
