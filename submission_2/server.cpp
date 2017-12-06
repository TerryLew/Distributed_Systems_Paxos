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

class PrintThread: public ostringstream {
public:
    PrintThread() = default;
    ~PrintThread() {
        lock_guard<std::mutex> guard(_mutexPrint);
        cout << this->str();
    }
private:
    static mutex _mutexPrint;
};
mutex PrintThread::_mutexPrint{};

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
    okToTweet = false;
    curSlot = 0;
    maxSlot = 0;
    MyThread::InitMutex();
    int yes = 1;
    addr = address;
    userId = siteId;
    numSites = numSites_;
    //Each site(server) store the log containing tweet, block and unblock events
    log.readFromDisk(userId);
    restoreInfo();
    
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
        if (i.first == userId) continue;
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
    
    //get the slot to fill for the log from other sites by asking their accVal.size and compare
    sended_count = 0;
    PaxosMsg paxosMsg("GETSIZE", userId);
    string msg_str = paxosMsg.serialize();
    const char* msg = msg_str.c_str();
    refreshConnection();
    if(fdv.size() == 0){
        okToTweet = true;
    }
    //cout << "send start" << endl;
    
    for(auto itr = fdv.begin(); itr != fdv.end(); itr++){
        int suc = send(itr->first, msg, strlen(msg), 0);
        if(suc > 0) sended_count++;
        close(itr->first);
    }
    
    //cout << "send finish" << endl;
    
    while(1) {
        if(1){
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
                synod();
            } else if (cmd == "block") {
                getline(cin, input);
                input = input.substr(1);
                event.populate(userId, "block", input, string(asctime(lctm)), string(asctime(utctm)));
                myVal = event;
                synod();
                block(stoi(input));
            }
            else if (cmd == "unblock") {
                getline(cin, input);
                input = input.substr(1);
                event.populate(userId, "unblock", input, string(asctime(lctm)), string(asctime(utctm)));
                myVal = event;
                synod();
                unblock(stoi(input));
            }
            else if (cmd == "view") view();
            else if (cmd == "viewLog") viewLog();
            else if (cmd == "viewBlock") viewBlock();
            else if (cmd == "viewIsBlockedBy") viewIsBlockedBy();
            else cerr << "invalid command\n";
            usleep(500*1000);
        }

        
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
    console("recv", paxosMsg);
    if(paxosMsg.getType() == "GETSIZE"){
        string msg_str = "RETURNSIZE " + to_string(userId) + " " + to_string(accVal.size());
        const char* msg = msg_str.c_str();
        refreshConnection();
        for(auto itr = fdv.begin(); itr != fdv.end(); itr++){
            if (itr->second == paxosMsg.getSenderId()) {
                send(itr->first, msg, strlen(msg), 0);
                close(itr->first);
                break;
            }
        }
        
    }else if(paxosMsg.getType() == "RETURNSIZE"){
        //if received all the accVal size from the sended sites
        sended_count--;
        if (sended_count >= 0) maxSlot = max(maxSlot, paxosMsg.getSlot());
        if (sended_count <= 0) {
            //cout << "maxslot " << maxSlot << endl;
            myVal = Event();
            for(int i = curSlot; i < maxSlot; i++) {
                synod();
                PrintThread{} << "curSlot: " << curSlot << ", i: "<<i<< "\n";
                while(accVal[i].isDummy()) {}
            }
            rebuildInMemoryData();
        }
    }else if(paxosMsg.getType() == "prepare") {
        int proposer = paxosMsg.getSenderId();
        int n = paxosMsg.getNum();
        int slot = paxosMsg.getSlot();
        if (slot >= maxPrepare.size()) {
            promise(proposer, slot, -1, Event());
        } else if (n > maxPrepare[slot]) {
            maxPrepare[slot] = n;
            promise(proposer, slot, accNum[slot], accVal[slot]);
        } else try_again(proposer, slot);
    } else if (paxosMsg.getType() == "promise") {
        int n = paxosMsg.getNum();
        Event v = paxosMsg.getValue();
        while(accNum_tmp.size() <= curSlot) {
            accNum_tmp.push_back(INT_MIN);
            accVal_tmp.push_back(Event());
        }
        if (n==-1 && accVal_tmp[curSlot].isDummy()) {
            if (accNum_tmp[curSlot] <= myProposal) {
                accVal_tmp[curSlot] = myVal;
                accNum_tmp[curSlot] = myProposal;
            }
        } else if (accNum_tmp[curSlot] < n) {
            accVal_tmp[curSlot] = v;
            accNum_tmp[curSlot] = n;
        }
        //PrintThread{} << accNum_tmp[curSlot] << ", " << n << "\n";
        //PrintThread{} << accVal_tmp[curSlot].serialize();
        //cout << "promise curSlot is " << curSlot << endl;
        majorityPromise[curSlot]++;
        //cout <<  "majority: "<< majorityPromise[curSlot]<<"\n";
        if(majorityPromise[curSlot] > numSites/2) {
            majorityPromise[curSlot] = 0;
            Accept(curSlot, accNum_tmp[curSlot], accVal_tmp[curSlot]);
        }
    } else if (paxosMsg.getType() == "accept") {
        int n = paxosMsg.getNum();
        int slot = paxosMsg.getSlot();
        int proposer = paxosMsg.getSenderId();
        Event v = paxosMsg.getValue();
        while (maxPrepare.size() <= slot) {
            maxPrepare.push_back(INT_MIN);
            accNum.push_back(INT_MIN);
            accVal.push_back(Event());
        }
        if (n == 0 || n >= maxPrepare[slot]) {//leader election
            accNum[slot] = n;
            accVal[slot] = v;
            maxPrepare[slot] = n;
        }
        ack(proposer, slot, accNum[slot], accVal[slot]);
    } else if (paxosMsg.getType() == "ack") {
        Event v = paxosMsg.getValue();
        int n = paxosMsg.getNum();
        int slot = paxosMsg.getSlot();
        while (majorityAck.size()<=slot) majorityAck.push_back(0);
        majorityAck[slot]++;
        if(majorityAck[slot] > numSites/2) {
            majorityAck[slot] = 0;
            //PrintThread{} << "m["<<slot<<"]="<<majorityAck[slot] << ", "<< accNum.size() <<", " << slot << "\n";
            log.addToLog(slot, v);
            log.writeToDisk(userId);
            updateInDiskData();
            if (v.getOp() == "tweet") timeline.push_back(v);
            //cout << "dummy curSlot" << curSlot << endl;
            commit(curSlot, v);
            curSlot++;
            accNum[slot] = n;
            maxPrepare[slot] = n;
            accVal[slot] = v;
        }
        if (curSlot == maxSlot) okToTweet = true;
    } else if (paxosMsg.getType() == "commit") {
        Event v = paxosMsg.getValue();
        int slot = paxosMsg.getSlot();
        if(slot < curSlot) return;
        log.addToLog(slot, v);
        log.writeToDisk(userId);
        updateInDiskData();
        if (v.getOp() == "tweet" && isBlockedBy.count(v.getUserId())==0) timeline.push_back(v);
        //changed the blockeduser mathod
        if (v.getOp() == "block" && stoi(v.getData()) == userId)  {
            updateTimeline("block", v.getUserId());
        }else if(v.getOp() == "unblock" && stoi(v.getData()) == userId){
            updateTimeline("unblock", v.getUserId());
        }
        curSlot++;
    } else if (paxosMsg.getType() == "try_again") {
        tryagainSynod(paxosMsg.getSenderId());
    }
}

void Server::synod(){
    chooseProposalNumber();
    while (maxPrepare.size() <= curSlot) {
        maxPrepare.push_back(myProposal); //push_back(n) changed to push_back(0)
        accNum.push_back(myProposal);
        accVal.push_back(myVal);
        accNum_tmp.push_back(INT_MIN);
        accVal_tmp.push_back(myVal);
        majorityPromise.resize(curSlot + 1, 0);
    }
    //cout << maxPrepare.size() << "curSlot is " << curSlot << endl;
    if(curSlot!=0 && accVal[curSlot-1].getUserId()==userId) {
        Accept(curSlot, 0, myVal);
    } else {
        int slot = maxPrepare.size() - 1;
        prepare(slot);
    }

}

void Server::tryagainSynod(int senderId){
    int slot = maxPrepare.size() - 1;
    //cout << "slot before after trying again is " << slot << endl;
    if(receivedTryAgainFrom == senderId || receivedTryAgainFrom == -1) {
        chooseProposalNumber();
        receivedTryAgainFrom = senderId;
        prepare(slot);
    }
}

void Server::prepare(int slot) {
    PaxosMsg paxosMsg("prepare", userId, slot, myProposal);
    string msg_str = paxosMsg.serialize();
    const char* msg = msg_str.c_str();
    refreshConnection();
    for(auto itr = fdv.begin(); itr != fdv.end(); itr++){
        send(itr->first, msg, strlen(msg), 0);
        close(itr->first);
    }
    console("send", paxosMsg, -1);
}

void Server::promise(int proposer, int slot, int accNum, Event accVal) {
    refreshConnection();
    PaxosMsg paxosMsg("promise", userId, slot, accNum, accVal);
    string msg_str = paxosMsg.serialize();
    const char* msg = msg_str.c_str();
    for(auto itr = fdv.begin(); itr != fdv.end(); itr++){
        if (itr->second == proposer) {
            send(itr->first, msg, strlen(msg), 0);
            close(itr->first);
            break;
        }
    }
    console("send", paxosMsg, proposer);
}

void Server::Accept(int slot, int accNum, Event accVal) {
    refreshConnection();
    PaxosMsg paxosMsg("accept", userId, slot, accNum, accVal);
    string msg_str = paxosMsg.serialize();
    const char* msg = msg_str.c_str();
    for(auto itr = fdv.begin(); itr != fdv.end(); itr++){
        send(itr->first, msg, strlen(msg), 0);
        close(itr->first);
    }
    console("send", paxosMsg, -1);
}

void Server::ack(int proposer, int slot, int accNum, Event accVal) {
    refreshConnection();
    PaxosMsg paxosMsg("ack", userId, slot, accNum, accVal);
    string msg_str = paxosMsg.serialize();
    const char* msg = msg_str.c_str();
    for(auto itr = fdv.begin(); itr != fdv.end(); itr++){
        if (itr->second == proposer) {
            send(itr->first, msg, strlen(msg), 0);
            close(itr->first);
            break;
        }
    }
    console("send", paxosMsg, proposer);
}

void Server::commit(int slot, Event accVal) {
    refreshConnection();
    PaxosMsg paxosMsg("commit", userId, slot, accVal);
    string msg_str = paxosMsg.serialize();
    const char* msg = msg_str.c_str();
    for(auto itr = fdv.begin(); itr != fdv.end(); itr++){
        send(itr->first, msg, strlen(msg), 0);
        close(itr->first);
    }
    console("send", paxosMsg, -1);
}

void Server::try_again(int proposer, int slot) {
    refreshConnection();
    PaxosMsg paxosMsg("try_again", userId, slot);
    string msg_str = paxosMsg.serialize();
    const char* msg = msg_str.c_str();
    for(auto itr = fdv.begin(); itr != fdv.end(); itr++){
        if (itr->second == proposer) {
            send(itr->first, msg, strlen(msg), 0);
            close(itr->first);
            break;
        }
    }
    console("send", paxosMsg, proposer);
}

void Server::block(int blockId) {
    blockedUser.insert(blockId);
}

void Server::unblock(int unblockId) {
    blockedUser.erase(unblockId);
}

void Server::view() {
    vector<Tweet> vt;
    for(auto itr = timeline.begin(); itr != timeline.end(); itr++){
        vt.push_back(Tweet(*itr));
    }
    sort(vt.begin(), vt.end(), comparetime);
    cout << "Viewable tweets:" << endl;
    cout <<"--------------------------------------------\n";
    for (auto itr = vt.begin(); itr != vt.end(); ++itr){
        time_t local = itr->getLocalTime();
        cout << ctime(&(local))
             << "Site " << to_string(itr->getUserId())
             << " tweeted: " << itr->getData() << endl;
        cout <<"--------------------------------------------\n";
    }
    
}

void Server::viewLog(){
    cout << "log:\n";
    log.display();
}

void Server::viewBlock() {
    for(auto it = blockedUser.begin(); it != blockedUser.end(); it++){
        cout << *it << endl;
    }
}

void Server::viewIsBlockedBy() {
    for(auto it = isBlockedBy.begin(); it != isBlockedBy.end(); it++){
        cout << *it << endl;
    }
}

int Server::chooseProposalNumber() {
    if (myProposal==0) myProposal = userId;
    else myProposal += numSites;
    return myProposal;
}

void Server::updateTimeline(string keyword, int target) {
    if (keyword == "block") {
        if (isBlockedBy.count(target)!=0) return;
        for(auto itr = timeline.begin(); itr!= timeline.end(); ) {
            if(itr->getUserId() == target) {
                itr = timeline.erase(itr);
            } else itr++;
        }
        isBlockedBy.insert(target);
    } else if (keyword == "unblock") {
        if(isBlockedBy.count(target)==0) return;
        for(auto & i : log.getEvents()) {
            if (i.getOp()=="tweet" && i.getUserId()==target) {
                timeline.push_back(i);
            }
        }
        isBlockedBy.erase(target);
    }
}


//save MaxPrepare accNum and accVal
void Server::updateInDiskData() {
    string filename1 = "MaxPrepare" + to_string(userId);
    ofstream output1(filename1.c_str());
    for(auto & i : maxPrepare)
        output1 << i << "\n";
    output1.close();
    
    string filename2 = "accNum" + to_string(userId);
    ofstream output2(filename2.c_str());
    for(auto & i : accNum)
        output2 << i << "\n";
    output2.close();
    
    string filename3 = "accVal" + to_string(userId);
    ofstream output3(filename3.c_str());
    for(auto & i : accVal)
        output3 << i.serializeForStoring() << "\n";
    output3.close();
}

void Server::rebuildInMemoryData() {
    for(auto & event : log.getEvents()) {
        if (event.getOp() == "block") {
            if (event.getUserId()==userId)
                blockedUser.insert(stoi(event.getData()));
            else if (stoi(event.getData()) == userId)
                isBlockedBy.insert(event.getUserId());
        } else if (event.getOp() == "unblock") {
            if (event.getUserId() == userId)
                blockedUser.erase(stoi(event.getData()));
            else if (stoi(event.getData()) == userId)
                isBlockedBy.erase(event.getUserId());
        }
    }
    timeline.clear();
    for(auto & event : log.getEvents()) {
        if (event.getOp() == "tweet" &&
            isBlockedBy.count(event.getUserId())==0)
            timeline.push_back(event);
    }
}

//read MaxPrepare accNum and accVal
void Server::restoreInfo(){
    string filename1 = "MaxPrepare" + to_string(userId);
    cout << "Reading " << filename1 <<" from disk...\n";
    ifstream input1(filename1.c_str());
    string str1;
    maxPrepare.clear();
    while(getline(input1, str1)) {
        int a1 = stoi(str1);
        maxPrepare.push_back(a1);
    }
    input1.close();
    
    string filename2 = "accNum" + to_string(userId);
    cout << "Reading " << filename2 << " from disk...\n";
    ifstream input2(filename2.c_str());
    string str2;
    accNum.clear();
    while(getline(input2, str2)) {
        int a2 = stoi(str2);
        accNum.push_back(a2);
    }
    input2.close();
    
    string filename3 = "accVal" + to_string(userId);
    cout << "Reading " << filename3 << " from disk...\n";
    ifstream input3(filename3.c_str());
    string str3;
    accVal.clear();
    while(getline(input3, str3)) {
        if(str3.empty()){
            accVal.push_back(Event());
        }else{
            Event e(str3);
            accVal.push_back(e);
        }
        
    }
    input3.close();
    curSlot = log.getEvents().size();
    receivedTryAgainFrom = -1;
}

void Server::console(string bound, PaxosMsg& paxosMsg, int recver) {
    string str;
    if(bound == "send")
        str = to_string(userId) + " sends to " +
            (recver==-1?"all acceptors":to_string(recver)) + ": ";
    else if(bound == "recv")
        str = to_string(userId) + " recv from " +
                to_string(paxosMsg.getSenderId())+": ";
    
    if (paxosMsg.getType() == "prepare") {
        PrintThread{} << str << "prepare(" << paxosMsg.getNum() <<
            ") for slot "<<paxosMsg.getSlot()<<"\n";
    } else if (paxosMsg.getType() == "promise") {
        PrintThread{} << str<< "promise(" << paxosMsg.getNum() <<","<<
            paxosMsg.getValue().serializeForView()<<") for slot "<<
            paxosMsg.getSlot()<<"\n";
    } else if (paxosMsg.getType() == "accept") {
        PrintThread{} << str << "accept(" << paxosMsg.getNum() <<","<<
            paxosMsg.getValue().serializeForView()<<") for slot "<<
            paxosMsg.getSlot()<<"\n";
    } else if (paxosMsg.getType() == "ack") {
        PrintThread{} << str << "ack(" << paxosMsg.getNum() <<","<<
            paxosMsg.getValue().serializeForView()<<") for slot "<<
            paxosMsg.getSlot()<<"\n";
    } else if (paxosMsg.getType() == "commit") {
        PrintThread{} << str << "commit(" << paxosMsg.getValue().serializeForView()<<
            ") for slot "<< paxosMsg.getSlot()<<"\n";
    } else if (paxosMsg.getType() == "try_again"){
        PrintThread{} << str << "try_again(" << paxosMsg.getSenderId() <<
            ") for slot "<< paxosMsg.getSlot()<<"\n";
    } else if (paxosMsg.getType() == "GETSIZE"){
        PrintThread{} << str << "GETSIZE from" << paxosMsg.getSenderId() << "\n";
        
    }else if(paxosMsg.getType() == "RETURNSIZE"){
        PrintThread{} << str << "RETURNSIZE from " << paxosMsg.getSenderId() << " current max slot number is " << paxosMsg.getSlot() << "\n";
    }
}

