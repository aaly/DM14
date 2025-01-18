#ifndef __NODEHPP
#define __NODEHPP

#include "Array.hpp"
#include "Socket.hpp"
#include "Thread.hpp"
#include "common.hpp"
#include <atomic>
#include <exception> // For exception class
#include <iostream>
#include <math.h>
#include <mutex>
#include <semaphore.h>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <string> // For string
#include <unistd.h>
#include <vector>

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>

#define DM14_NODE_MAX_FDS 100

using namespace std;

void lock(volatile int *lockBit);
void unlock(volatile int *lockBit);

class LocalDataVectoryEntry;
class nodePeer;
class MessageTCPSocket;
class Node;
class nodeMessage;
typedef MutexLock Mutex;

typedef atomic<bool> atomicBool;
typedef Array<LocalDataVectoryEntry *> *elementsArray;
typedef Array<void *> *bufferArray;
typedef Array<string> stringArray;
typedef Thread<Node> nodeThread;
typedef Array<nodePeer> nodePeerArray;
typedef Array<nodeMessage> nodeMessageArray;
typedef Array<LocalDataVectoryEntry> LocalDataVectoryEntryArray;
typedef struct pollfd pollfdStruct;
typedef Socket tcpSocket;

class LocalDataVectoryEntry // FIX:use one unsigned chat for all booleans ! and
                            // change need to accept that one char
{
public:
  // LocalDataVectoryEntry():array(false),ready(false),var(NULL),elements(new
  // Array<LocalDataVectoryEntry>()){};
  // LocalDataVectoryEntry():array(false),ready(false),var(NULL),
  // elements(NULL), recurrent(false), channel(false), init(false),
  // stack(false), lockBit(0){ buffer = new Array<void*>;};
  // LocalDataVectoryEntry():array(false),ready(false),var(NULL),
  // elements(NULL), recurrent(false), channel(false), init(false),
  // stack(false), lockBit(0){ buffer = new Array<void*>;};
  LocalDataVectoryEntry()
      : array(false), ready(false), var(NULL), elements(NULL), recurrent(false),
        channel(false), init(false), stack(false) {
    buffer = new Array<void *>;
  };
  LocalDataVectoryEntry(bool parray, bool pready, void *pvar, bool precurrent,
                        bool pchannel, const string &ptype)
      : array(parray), ready(pready), var(pvar), type(ptype), elements(NULL),
        recurrent(precurrent), channel(pchannel), init(false), stack(false) {
    buffer = new Array<void *>;
  };
  // LocalDataVectoryEntry(bool parray, bool pready, void* pvar, bool
  // precurrent, bool pchannel, const string&
  // ptype):array(parray),ready(pready),var(pvar),type(ptype), elements(NULL),
  // recurrent(precurrent), channel(pchannel), init(false), stack(false),
  // lockBit(0){buffer = new Array<void*>;};
  bool array;
  // bool	element;
  bool ready;
  void *var;
  string type;
  elementsArray elements;
  bufferArray buffer;
  bool recurrent;
  bool channel;
  bool init;
  bool stack;

  // volatile int lockBit;
  Mutex lockBit;

  stringArray readersParty;
  stringArray writersParty;

  stringArray activeNodesHistory;
  string lastActiveNode;

  int bufferSize;

  int registerReader(const string &node);
  int registerWriter(const string &node);
};

class nodePeer {
public:
  nodePeer(bool dummy = false);
  ~nodePeer();
  MessageTCPSocket *peer;
  int node;
  int sendMessage(string *, const bool force = false);
  string recvMessage();
  int Log(string &);
  bool ready;
  int remainingMsgSize;
  unsigned int optimizations;
  unsigned int runningFlags;
  int status;
  int Port;
  string IP;
  string nodeID;

  enum RELATIONSHIPSTATE {
    ALIVEANDWELL = 0, // 0
    MISSEDYOU,        // 1
    DEADTOME,         // 2
    FOREVERALONE      // 3
  };
};

class nodeMessage {
public:
  nodeMessage(const int &type, const string &msg, nodePeer &node)
      : m_type(type), m_msg(msg), m_node(node) {};
  nodeMessage();
  nodePeer &getNode();
  int const getType();
  string &getValue();

  enum Type {
    ADD_NODE = 0,          // 0
    ADD_NODE_BOOTSTRAP,    // 1
    R_JOIN,                // 2 Request Join
    R_JOIN_BOOTSTRAP,      // 3 Request Join	and get bootstrap information
    R_OBJECT,              // 4 Request Object
    R_OBJECT_FROM_NODE,    // 5
    R_CHANNELOBJECT,       // 6 request/register channel object
    N_CLOSING,             // 7 Notify Node is Closing
    N_R_JOINACCEPTED,      // 8 Notify Request JOIN is Accepted
    N_R_OBJECTACCEPTED,    // 9
    N_R_OBJECTREJECTED,    // 10
    N_R_OBJECTNOTFOUND,    // 11
    N_R_OBJECTVALUE,       // 12
    REMOVING_DEAD_NODE,    // 13 deleteing dead in-active node
    MISSING_NODE_ACTIVITY, // 14 about to remove the dead an-active node
    KEEP_NODE_ALVIE        // 15 keep me alive, reply to other warnings
  };

  static const char *typeName[16];

  string getToken(int pos, unsigned long size = 0);

private:
  int m_type;
  string m_msg;
  nodePeer m_node;
  long getPosition(int order);
};

class Node {
public:
  Node();
  ~Node();
  unsigned int nodeNumber;
  void *listener(void *par);
  bool dataVectorReady;
  int setCapacity(unsigned int n);
  int setNodeNumber(const int &node);
  int changeNodeNumber(const int &node);
  int setNodeID(const string &name);
  int addNode(const string &address, const int &port, bool bootstrap);
  int removeNode(nodePeer &node);
  int sendMessage(nodePeer *node, string *message);
  string receiveMessage(nodePeer *node);
  int setServer(const string &address, const int &port, const int &clients);
  int startListener(bool);
  int serve(bool);
  nodeThread listenerThread;
  nodeThread listenerThread2;
  int nodesMaximum;
  int printNodes();
  int printNodesAddress();
  int updateNodes();
  nodePeer *findNode(const string);
  int Exit();
  int unmapVariable(unsigned int var, int index);

  int pollOnSocket(int socketFD, short events);
  int pollOffSocket(int socketFD);
  int processMessages();
  nodePeer *findNodeByFd(int fd);

  template <class T>
  void addVectorArrayData(int var, long index, bool parray, bool pready,
                          void *pvar, bool precurrent, bool pchannel,
                          const string &ptype);
  template <class T>
  void addVectorClassData(int var, long index, bool parray, bool pready,
                          void *pvar, bool precurrent, bool pchannel,
                          const string &ptype);
  template <class T>
  void addVectorData(int var, long index, bool parray, bool pready, void *pvar,
                     bool precurrent, bool pchannel, const string &ptype);

  template <class T>
  void need(unsigned int var, int index, bool pointer, int node, T *variable,
            bool block, string nodeID = "");
  template <class T>
  void done(int var, int index, void *pointer, string nodeID = "");

  // template<class T> T& need(int var, string type, unsigned int node, T&
  // variable); void need(int var, string type, unsigned int node, void*
  // variable); Array<void*> 	__M14VPOINTERS; Array<bool>
  // __M14VSTATUSES;
  LocalDataVectoryEntryArray _DM14VARIABLESMAP;
  string NodeID;
  unsigned int checkTicks;
  string getLastActiveNode(int var, int index);

private:
  // int	handleRequestMessage(const string&, nodePeer& node);
  // int	sendMessage(nodePeer*, string&);
  int pushRequestMessage(const string &msg, nodePeer node = nodePeer(),
                         const bool &insider = false);
  int handleRequest(nodeMessage &R);
  int sendVariable(unsigned long var, long index, nodePeer *node);
  string Serialize(int var, int index);
  int deSerialize(int var, int index);
  int Log(string &);
  long getNodeIndex(nodePeer &node);

  // volatile int exclusion;
  // volatile int messagesQueueLock;
  MessageTCPSocket Self;
  nodePeerArray clients;
  nodePeerArray nodes;
  nodeMessageArray messagesQueue;
  string localIP;
  int bindPort;
  int clientsSize;
  // MutexLock 		dataMutex;
  // MutexLock 		queueMutex;
  // sem_t				queueSema;
  // sem_t				dataSema;
  // volatile bool		writing;
  volatile bool readready;
  // bool				serverInit;
  atomicBool Serve;
  pollfdStruct monitoredDescriptors[DM14_NODE_MAX_FDS];
  int currentMonitoredDescriptorsCount;
  // tcpSocket				pollTrigger;
  int pollTrigger[2];
};

template <class T>
void Node::need(unsigned int var, int index, bool pointer, int node,
                T *variable, bool block, string nodeID) {
  LocalDataVectoryEntry *entry = NULL;

  if (index != -1) {
    entry = _DM14VARIABLESMAP.at(var).elements->at(index);
  } else {
    entry = &_DM14VARIABLESMAP.at(var);
  }

  //?lock(&entry->lockBit);
  entry->lockBit.lock();

  if ((entry->channel || entry->recurrent) && entry->stack) {
    // indicate we read our own value once so next time we don't
    entry->stack = false;
    entry->ready = false;
    *variable = T();
    //?unlock(&entry->lockBit);
    entry->lockBit.unlock();
    cerr << "RETTTTTTTTT" << endl;
    return;
  }

  if (!entry->ready || entry->channel || entry->recurrent || !block) {
    // if ( !(!block && entry->ready))
    if (!entry->ready) {
      stringstream SS;
      SS << (char)nodeMessage::R_OBJECT_FROM_NODE;
      SS << node;
      SS << ';';
      SS << var;
      SS << ';';
      SS << index;
      // pushRequestMessage(SS.str(), *(nodePeer*)NULL);
      pushRequestMessage(SS.str());

      if ((entry->channel || entry->recurrent) && variable) {
        *variable = T();
      }

      if (!block) {
        //?unlock(&entry->lockBit);
        entry->lockBit.unlock();
        return;
      }
    }

    if (!(entry->channel || entry->recurrent)) {

      if (block) {
        //?lock(&entry->lockBit);
        entry->lockBit.lock();
      }

      *variable = *(T *)entry->var;

      if ((T *)entry->var) {
        entry->init = false;
        delete (T *)entry->var;
      }

      if (pointer) {
        // variable = (T*)entry->var;
      } else {
        //*variable = *(T*)entry->var;
      }
      // delete (T*)*object;
    } else {

      if (!entry->stack && block) {
        if (variable != NULL) {
          // delete (T*)variable;
        }
      }

      if (!entry->buffer->size() && block) {
        //?lock(&entry->lockBit);
        entry->lockBit.lock();
      }

      // entry->buffer->lock();
      if (!entry->stack) {
        *variable = *(T *)entry->buffer->at(0);
        delete (T *)entry->buffer->at(0);
        entry->buffer->remove(0);
        entry->ready = false;
      } else {
        entry->stack = false;
        entry->ready = false;
      }
      // entry->buffer->unlock();

      if (pointer) {
        // variable = (T*)entry->buffer->at(0);
      } else {
        //*variable = *(T*)entry->buffer->at(0);
      }
    }
    // unlock(&entry->lockBit);
  }

  entry->lockBit.unlock();
}

template <class T>
void Node::done(int var, int index, void *pointer, string nodeID) {
  //* should we loop through requests and sent them ? <<<< just tell them it is
  //ready
  //* or let the clients loop and get it ? <<<< better because they are waiting
  //for it anyway !
  LocalDataVectoryEntry *variable = NULL;
  if (index != -1) {
    variable = _DM14VARIABLESMAP.at(var).elements->at(index);
  } else {
    variable = &_DM14VARIABLESMAP.at(var);
  }

  if (!variable->stack || variable->channel || variable->recurrent ||
      index != -1) {
    // variable->lockBit.lock();

    if (variable->stack) {
      delete (T *)variable->var;
    }
    variable->var = new T();
    *(T *)variable->var = *(T *)pointer;
    variable->ready = true;
    variable->init = true;
    variable->stack = true;

    variable->lastActiveNode = NodeID;

    // printNodes();
    // cout << " SIZE : "  << variable->readersParty.size() << endl;
    // cout << " SIZE : "  << variable->writersParty.size() << endl;

    if (!nodeID.size()) {
      for (unsigned int i = 0; i < variable->readersParty.size(); i++) {
        // displayError("Sending Channel obect to " +
        // variable->readersParty.at(i));
        sendVariable(var, index, findNode(variable->readersParty.at(i)));

        if (!variable->channel && !variable->recurrent) {
          variable->readersParty.remove(i);
          i--;
        }
      }
    }

    // variable->lockBit.unlock();
  }

  if (nodeID.size()) {
    // displayError("Sending obect to " + nodeID);
    // variable->lockBit.unlock();
    sendVariable(var, index, findNode(nodeID));
  }

  variable->lockBit.unlock();
}

template <class T>
void Node::addVectorArrayData(int var, long index, bool parray, bool pready,
                              void *pvar, bool precurrent, bool pchannel,
                              const string &ptype) {
  if (parray) {
    ////cout << "adding: " << var << "-" << index << "-" << ptype << "-" <<
    ///endl;
    _DM14VARIABLESMAP.push_back(LocalDataVectoryEntry(
        parray, pready, pvar, precurrent, pchannel, ptype));
    _DM14VARIABLESMAP.at(var).elements = new Array<LocalDataVectoryEntry *>();

    T *array = (T *)pvar;
    for (long i = 0; i < index; i++) {
      _DM14VARIABLESMAP.at(var).elements->push_back(
          new LocalDataVectoryEntry());
      addVectorData<T>(var, i, false, pready, &array->at(i), precurrent,
                       pchannel, ptype);
    }
  }
};

template <class T>
void Node::addVectorClassData(int var, long index, bool parray, bool pready,
                              void *pvar, bool precurrent, bool pchannel,
                              const string &ptype) {
  if (_DM14VARIABLESMAP.size() - 1 < var) {
    _DM14VARIABLESMAP.push_back(LocalDataVectoryEntry(
        parray, pready, pvar, precurrent, pchannel, ptype));
  }

  if (index > -1) {

    if (!_DM14VARIABLESMAP.at(var).elements) {
      _DM14VARIABLESMAP.at(var).elements = new Array<LocalDataVectoryEntry *>();
    }
    _DM14VARIABLESMAP.at(var).elements->push_back(new LocalDataVectoryEntry(
        parray, pready, pvar, precurrent, pchannel, ptype));
  }
  // else
  {
    //_DM14VARIABLESMAP.push_back(LocalDataVectoryEntry(parray, pready, pvar,
    //precurrent, pchannel, ptype));
  }
};

template <class T>
void Node::addVectorData(int var, long index, bool parray, bool pready,
                         void *pvar, bool precurrent, bool pchannel,
                         const string &ptype) {
  if (index > -1) {
    _DM14VARIABLESMAP.at(var).elements->at(index) = new LocalDataVectoryEntry(
        parray, pready, pvar, precurrent, pchannel, ptype);
  } else {
    _DM14VARIABLESMAP.push_back(LocalDataVectoryEntry(
        parray, pready, pvar, precurrent, pchannel, ptype));
  }
};

#endif //__NODEPP
