#include "server.h"
#include "tweet.h"
#include "log.h"

#include <set>
#include <unordered_set>
#include <iostream>
#include <sstream>
#include <istream>
#include <string>
#include <time.h>
#include <vector>
#include <locale>
#include <iomanip>
#include <algorithm>

using namespace std;

struct serverAndFd {
    int fd;
    Server* server;
    serverAndFd(int f, Server* s) {
        fd = f;
        server = s;
    }
};

//Actually allocate clients
Server::Server(int siteId, unordered_map<int, pair<string, string> >& address, int numSites_) {
    MyThread::InitMutex();
    int yes = 1;
    addr = address;
    userId = siteId;
    numSites = numSites_;
    //Each site(server) store the log containing tweet, block and unblock events
    //Init the log, dictionary, timetable and clock for Wuu Bernstein algorithm
    Log log;
    log.readFromDisk();
    updateInMemoryData();
    
    string ip = address[userId].first;
    string port = address[userId].second;
    cerr << "my ip: "<<ip <<", my port: "<<stoi(port)<<"\n";
    //Init serverSock and start listen()'ing
    serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    bzero((char*) &serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(stoi(port));
    
    //Avoid bind error if the socket was not close()'d last time;
    setsockopt(serverSock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));
    
    if(::bind(serverSock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
        cerr << "Failed to bind";
    listen(serverSock, 5);
}

void Server::refreshConnection() {
    fdv.clear();//has to clear fd set, because some site can be down and up anytime
    for(auto & i : addr) {
        if (i.first == userId || blockedUsers.find(i.first) != blockedUsers.end()) continue;
        struct hostent *server;
        const char * ip = i.second.first.c_str();
        server = gethostbyname(ip);
        struct sockaddr_in svrAdd;
        int listenFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        bzero((char *) &svrAdd, sizeof(svrAdd));
        svrAdd.sin_family = AF_INET;
        bcopy((char *) server->h_addr, (char *) &svrAdd.sin_addr.s_addr, server->h_length);
        svrAdd.sin_port = htons(stoi(i.second.second));
        if (connect(listenFd, (struct sockaddr *) &svrAdd, sizeof(svrAdd))>=0) {
            fdv.insert(make_pair(listenFd, i.first));
        }
    }
}

void Server::start() {
    MyThread *t1 = new MyThread();
    MyThread *t2 = new MyThread();
    t2->Create((void *) startTweet, this);
    t1->Create((void *) acceptRequest, this);
    t2->Join();
    t1->Join();
}

void * Server::acceptRequest(void *args) {
    Server *s = (Server*)args;
    s->doAcceptRequest();
}

void Server::doAcceptRequest() {
    socklen_t cliSize = sizeof(clientAddr);
    while(1) {
        MyThread *t = new MyThread();
        int fd = accept(serverSock, (struct sockaddr *) &clientAddr, &cliSize);
        if (fd > 0) {
            struct serverAndFd* sfd = new serverAndFd(fd, this);
            t->Create((void *) HandleClient, sfd);
        }
    }
}

void *Server::startTweet(void *args) {
    Server* s = (Server*) args;
    s->doStartTweet();
}

void Server::doStartTweet() {
    while(1) {
        cout << ">> ";
        string cmd, input;
        cin >> cmd;
        time_t rawtime;
        struct tm *lctm;
        struct tm *utctm;
        time(&rawtime);
        lctm = localtime(&rawtime);//local time
        utctm = gmtime(&rawtime);//UTC time
        Event event;
        if(cmd == "tweet") {
            getline(cin, input);//get the following tweet words
            input = input.substr(1);
            event.populate(userId, "tweet", input, string(asctime(lctm)), string(asctime(utctm)));
            myVal = event;
            prepare();
        } else if (cmd == "block") {
            getline(cin, input);
            input = input.substr(1);
            event.populate(userId, "block", input, string(asctime(lctm)), string(asctime(utctm)));
            myVal = event;
            prepare();
            block(stoi(input));
        }
        else if (cmd == "unblock") {
            getline(cin, input);
            input = input.substr(1);
            event.populate(userId, "unblock", input, string(asctime(lctm)), string(asctime(utctm)));
            myVal = event;
            prepare();
            unblock(stoi(input));
        }
        else if (cmd == "view") view();
        else if (cmd == "viewLog") viewLog();
        else if (cmd == "viewdBlock") viewBlock();
        else cerr << "invalid command\n";
    }
}

void *Server::HandleClient(void *args) {
    struct serverAndFd* sfd = (struct serverAndFd*) args;
    int fd = sfd->fd;
    Server* s = sfd->server;
    s->doHandleClient(fd);
}

void Server::doHandleClient(int fd) {
    char buffer[BUFFER_SIZE];
    int n;
    string tmp;
    while((n = recv(fd, buffer, sizeof(buffer), 0))>0)
        tmp.append(buffer, n);
    doSomethingWithReceivedData(tmp);
}


void Server::doSomethingWithReceivedData(string& message) {
    PaxosMsg paxosMsg(message);
    if(paxosMsg.getType() == "prepare") {
        int proposer = paxosMsg.getProposer();
        int n = paxosMsg.getNum();
        int slot = paxosMsg.getSlot();
        if (slot >= maxPrepare.size()) {
            promise(proposer, -1, Event());
        } else if (n > maxPrepare[slot]) {
            maxPrepare[slot] = n;
            promise(proposer, accNum[slot], accVal[slot]);
        } else try_again(proposer, slot);
    } else if (paxosMsg.getType() == "promise") {
        int n = paxosMsg.getNum();
        Event v = paxosMsg.getValue();
        if (n==-1) {
            accVal_tmp[curSlot] = myVal;
            accNum_tmp[curSlot] = myProposal;
        } else if(accNum_tmp[curSlot] < n) {
            accVal_tmp[curSlot] = v;
            accNum_tmp[curSlot] = n;
        }
        majorityPromise[curSlot]++;
        if(majorityPromise[curSlot] > numSites/2) {
            Accept(curSlot, accNum_tmp[curSlot], accVal_tmp[curSlot]);
        }
    } else if (paxosMsg.getType() == "accept") {
        int n = paxosMsg.getNum();
        int slot = paxosMsg.getSlot();
        int proposer = paxosMsg.getProposer();
        Event v = paxosMsg.getValue();
        if (n >= maxPrepare[slot]) {
            accNum[slot] = n;
            accVal[slot] = v;
            maxPrepare[slot] = n;
            ack(proposer, accNum[slot], accVal[slot]);
        }
    } else if (paxosMsg.getType() == "ack") {
        Event v = paxosMsg.getValue();
        majorityAck[curSlot]++;
        if(majorityAck[curSlot] > numSites/2) {
            commit(curSlot, v);
        }
    } else if (paxosMsg.getType() == "commit") {
        Event v = paxosMsg.getValue();
        int slot = paxosMsg.getSlot();
        log.addToLog(slot, v);
        curSlot++;
    } else if (paxosMsg.getType() == "try_again") {
        prepare();
    }
}

void Server::prepare() {
    refreshConnection();
    int n = chooseProposalNumber();
    PaxosMsg paxosMsg("prepare", n);
    string msg_str = paxosMsg.serialize();
    const char* msg = msg_str.c_str();
    for(auto itr = fdv.begin(); itr != fdv.end(); itr++){
        send(itr->first, msg, strlen(msg), 0);
        close(itr->first);
    }
}

void Server::promise(int proposer, int accNum, Event accVal) {
    refreshConnection();
    PaxosMsg paxosMsg("promise", accNum, accVal);
    string msg_str = paxosMsg.serialize();
    const char* msg = msg_str.c_str();
    for(auto itr = fdv.begin(); itr != fdv.end(); itr++){
        if (itr->second == proposer) {
            send(itr->first, msg, strlen(msg), 0);
            close(itr->first);
            return;
        }
    }
}

void Server::Accept(int slot, int accNum, Event accVal) {
    refreshConnection();
    PaxosMsg paxosMsg("accept", slot, accNum, accVal);
    string msg_str = paxosMsg.serialize();
    const char* msg = msg_str.c_str();
    for(auto itr = fdv.begin(); itr != fdv.end(); itr++){
        send(itr->first, msg, strlen(msg), 0);
        close(itr->first);
    }
}

void Server::ack(int proposer, int accNum, Event accVal) {
    refreshConnection();
    PaxosMsg paxosMsg("ack", accNum, accVal);
    string msg_str = paxosMsg.serialize();
    const char* msg = msg_str.c_str();
    for(auto itr = fdv.begin(); itr != fdv.end(); itr++){
        if (itr->second == proposer) {
            send(itr->first, msg, strlen(msg), 0);
            close(itr->first);
            return;
        }
    }
}

void Server::commit(int slot, Event accVal) {
    refreshConnection();
    PaxosMsg paxosMsg("commit", slot, accVal);
    string msg_str = paxosMsg.serialize();
    const char* msg = msg_str.c_str();
    for(auto itr = fdv.begin(); itr != fdv.end(); itr++){
        send(itr->first, msg, strlen(msg), 0);
        close(itr->first);
    }
}

void Server::try_again(int proposer, int slot) {
    refreshConnection();
    PaxosMsg paxosMsg("try_again", slot);
    string msg_str = paxosMsg.serialize();
    const char* msg = msg_str.c_str();
    for(auto itr = fdv.begin(); itr != fdv.end(); itr++){
        if (itr->second == proposer) {
            send(itr->first, msg, strlen(msg), 0);
            close(itr->first);
            return;
        }
    }
}

void Server::block(int blockId) {
    blockedUsers.insert(blockId);
    updateTimeline("block", blockId);
}

void Server::unblock(int unblockId) {
    blockedUsers.erase(unblockId);
    updateTimeline("unblock", unblockId);
}

void Server::view() {
    vector<Tweet> vt;
    for(auto itr = timeline.begin(); itr != timeline.end(); itr++){
        vt.push_back(Tweet(*itr));
    }
    sort(vt.begin(), vt.end(), comparetime);
    cout << "Viewable tweets:" << endl;
    for (auto itr = vt.begin(); itr != vt.end(); ++itr){
        time_t local = itr->getLocalTime();
        cout << ctime(&(local))
             << ": User " << to_string(itr->getUserId())
             << " tweeted: " << itr->getData() << endl;
    }
    
}

void Server::viewLog(){
    log.display();
}

void Server::viewBlock() {
    for(auto it = blockedUsers.begin(); it != blockedUsers.end(); it++){
        cout << *it << endl;
    }
}

int Server::chooseProposalNumber() {
    if (myProposal==0) myProposal = userId;
    else myProposal += numSites;
    return myProposal;
}

void Server::updateTimeline(string keyword, int target) {
    
}

void Server::updateInMemoryData() {
    
}
                           


