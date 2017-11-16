#ifndef _server_h_
#define _server_h_

#include <iostream>
#include <vector>

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

#include "mythread.h"
#include "log.h"
#include "tweet.h"
#include <unordered_map>
#include <set>
#include <vector>
#include <string>
#define BUFFER_SIZE 256

using namespace std;

class Server {

  private:
    int serverSock, clientSock;
    struct sockaddr_in serverAddr, clientAddr;
    
    int userId;
    Log log;
    int numSites;
    set<Event> timeline;
    unordered_set<int> block;
    set<pair<int, int> > fdv;
    unordered_map<int, pair<string, string> > addr;
    

  public:
    Server(int siteID, unordered_map<int, pair<string, string> >& address, int numSites);
    static void * HandleClient(void *args);
    static void* startTweet(void *args);
    void doStartTweet();
    void doAcceptRequest();
    void doHandleClient(int fd);
    static void* acceptRequest(void *args);
    void start();
    
    void doSomethingWithReceivedData(string& message);
    
    void prepare();
    void promise(int proposer, int accNum, Event accVal);
    void accept(int slot, int accNum, Event accVal);
    void ack(int proposer, int accNum, Event accVal);
    void commit(int slot, Event accVal);
    void try_again(int proposer);
    
    void block(string& input);
    void unblock(string& input);
    void view();
    void viewlog();

    vector<vector<int> > recvTT(string& tmsg);
};

#endif
