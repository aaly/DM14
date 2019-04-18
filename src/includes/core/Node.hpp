#ifndef __NODEHPP
#define __NODEHPP


#include <mutex> 
#include <string>            // For string
#include <exception>         // For exception class
#include <string.h>
#include <math.h>
#include <vector>
#include <stdlib.h>
#include "Socket.hpp"
#include <iostream>
#include "Thread.hpp"
#include "Array.hpp"
#include "common.hpp"
#include <sstream>
#include <unistd.h>
#include <semaphore.h>
#include <atomic>
#include <mutex.hpp>

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include "message.hpp"


#define DM14_NODE_MAX_POLL_FDS 1000

using namespace std;


void lock(volatile int* lockBit);
void unlock(volatile int* lockBit);

class LocalDataVectoryEntry;
class nodePeer;
class MessageTCPSocket;
class Node;
class nodeMessage;
//typedef MutexLock Mutex;
//typedef Semaphore readySemaphore;
typedef spinLock Mutex;
typedef Semaphore readySemaphore;


typedef atomic<bool> atomicBool;
typedef Array<LocalDataVectoryEntry*>* elementsArray;
typedef Array<void*>* bufferArray;
typedef Array<string> stringArray;
typedef Thread<Node> nodeThread;
typedef Array<nodePeer> nodePeerArray;
typedef Array<nodeMessage> nodeMessageArray;
typedef Array<LocalDataVectoryEntry> LocalDataVectoryEntryArray;
typedef struct pollfd pollfdStruct;
typedef Socket tcpSocket;

class LocalDataVectoryEntry // FIX:use one unsigned chat for all booleans ! and change need to accept that one char
{
	public:
		LocalDataVectoryEntry():array(false), stack(NULL), elements(NULL), recurrent(false), channel(false),  init(false), ready(false){buffer = new Array<void*>;};
		LocalDataVectoryEntry(bool parray, bool pready, void* pvar, bool precurrent, bool pchannel, const string& ptype):array(parray), stack(NULL), type(ptype), elements(NULL), recurrent(precurrent), channel(pchannel), init(false), ready(pready){buffer = new Array<void*>;};
		bool	array;
		void*	stack; // initialized locally ?
		string	type;
		elementsArray elements;
		bufferArray buffer;
		bool	recurrent;
		bool	channel;
		bool	init;	// memory allocated ?
		volatile bool	ready;
		
		//volatile int lockBit;
		Mutex lockBit;
		
		stringArray readersParty;
		stringArray writersParty;
		
		//stringArray activeNodesHistory;
		stringArray activeNodesHistory;
		string lastActiveNode;
		
		int bufferSize;
		
		int registerReader(const string& node);
		int registerWriter(const string& node);
		
		readySemaphore readySignal;
};



class nodePeer
{
	public:
		nodePeer(bool dummy = false);
		/*nodePeer(const nodePeer &np) {peer = np.peer; node = np.node; ready = np.ready; remainingMsgSize = np.remainingMsgSize;
									  optimizations = np.optimizations; runningFlags = np.runningFlags; status = np.status;
									  Port = np.Port;
									  IP = np.IP;
									  nodeID = np.nodeID;};*/
		~nodePeer();
		MessageTCPSocket*	peer;
		int					node;
		int					sendMessage(string*, const bool force = false);
		string				recvMessage();
		int					Log(string&);
		bool				ready;
		int					remainingMsgSize;
		unsigned int		optimizations;
		unsigned int		runningFlags;
		int					status;
		string				Port;
		string				IP;
		string				nodeID;
		
		enum RELATIONSHIPSTATE
		{
			ALIVEANDWELL=0,	//0
			MISSEDYOU,		//1
			DEADTOME,		//2
			FOREVERALONE	//3
		};
};


class Node
{
	public:
		Node();
		~Node();
		unsigned int	nodeNumber;
		void*			listener(void* par);
		bool 			dataVectorReady;
		int			setCapacity(unsigned int n);
		int 			setNodeNumber(const int& node);
		int			changeNodeNumber(const int& node);
		int			setNodeID(const string& name);
		int			addNode(const string& address, const string& port, bool bootstrap);
		int 			removeNode(nodePeer& node);
		int			sendMessage(nodePeer* node, string* message);
		string			receiveMessage(nodePeer* node);
		int			setServer(const string& address, const string& port, const int& clients);
		int			startListener(bool);
		int			serve(bool);
		nodeThread		listenerThread;
		nodeThread		listenerThread2;
		int			nodesMaximum;
		int			printNodes();
		int			printNodesAddress();
		int			updateNodes();
		nodePeer*		findNode(const string);
		int 			Exit(int);
		int			unmapVariable(uint32_t var, int index);
		
		int			pollOnSocket(int socketFD, uint32_t events);
		int			pollOffSocket(int socketFD);
		int 			processMessages();
		nodePeer*		findNodeByFd(int fd);
		int			findNodeIndex(const string id);
		int			triggerEvents();
		int			appendRequestMessage(nodeMessage R);
		
		
		template<class T> void addVectorArrayData(uint32_t var, long index, bool parray, bool pready, void* pvar, bool	precurrent, bool pchannel,const string& ptype);
		template<class T> void addVectorClassData(uint32_t var, long index, bool parray, bool pready, void* pvar, bool precurrent, bool pchannel, const string& ptype);
		template<class T> void addVectorData(uint32_t var, long index, bool parray, bool pready, void* pvar, bool	precurrent, bool pchannel,const string& ptype);
		
		template<class T> void need(uint32_t var, int index, bool pointer, int node, T* variable, bool block, string nodeID ="");
		template<class T> void done(uint32_t var, int index, void* pointer, string nodeID ="");
		
		
		LocalDataVectoryEntryArray	_DM14VARIABLESMAP;
		string				NodeID;
		unsigned int			checkTicks;
		string				getLastActiveNode(uint32_t var, int index);
		int				requestObject(uint32_t var, int index, int nodeLevel);
		int				requestObject(uint32_t var, int index, string nodeAddress);
		
		
		enum messageType
		{
			ADD_NODE=0,				// 0
			ADD_NODE_BOOTSTRAP,		// 1
			R_JOIN,					// 2 Request Join
			R_JOIN_BOOTSTRAP,		// 3 Request Join	and get bootstrap information
			R_OBJECT,				// 4 Request Object
			R_OBJECT_FROM_NODE,		// 5
			R_CHANNELOBJECT,		// 6 request/register channel object
			N_CLOSING,				// 7 Notify Node is Closing
			N_R_JOINACCEPTED,		// 8 Notify Request JOIN is Accepted
			N_R_OBJECTACCEPTED,		// 9
			N_R_OBJECTREJECTED,		// 10
			N_R_OBJECTNOTFOUND,		// 11
			N_R_OBJECTVALUE,		// 12
			REMOVING_DEAD_NODE,		// 13 deleteing dead in-active node
			MISSING_NODE_ACTIVITY,	// 14 about to remove the dead an-active node
			KEEP_NODE_ALVIE			// 15 keep me alive, reply to other warnings
		};
		
	private:
		static const char* messageTypeName[16];
		int		pushRequestMessage(nodeMessage& msg, const bool& insider = false);
		int		handleRequest(nodeMessage& R);
		int		sendVariable(unsigned long var, long index, nodePeer* node);
		string		Serialize(uint32_t var, int index);
		int		deSerialize(uint32_t var, int index);
		int		Log(string&);
		long		getNodeIndex(nodePeer& node);
	

		MessageTCPSocket	Self;
		nodePeerArray		clients;
		nodePeerArray		nodes;
		nodeMessageArray	messagesQueue;
		string			localIP;
		string			bindPort;
		int			clientsSize;
		volatile bool		readready;
		atomicBool		Serve;
		int			pollTrigger[2];
		int			EPollFD;
		
};



template<class T> void Node::need(uint32_t var, int index, bool pointer, int node, T* variable, bool block, string nodeID)
{
	LocalDataVectoryEntry* entry = NULL;
	
	if(index != -1)
	{
		entry = _DM14VARIABLESMAP.at(var).elements->at(index);
	}
	else
	{
		entry = &_DM14VARIABLESMAP.at(var);
	}
	
			

	entry->lockBit.lock();
	bool lock = true;
	
	if(!entry->ready)
	{
		if(nodeID.size())
		{
			requestObject(var, index, nodeID);
		}
		else
		{
			requestObject(var, index, node);
		}
		
		if(!block)
		{
			entry->lockBit.unlock();
			return;
		}
		entry->lockBit.unlock();
		lock = false;
		entry->readySignal.lock();
	}
	
	if(!lock)
	{
		entry->lockBit.lock();
	}
	
		
	if(!entry->channel && !entry->recurrent)
	{
		if((T*)entry->stack)
		{
			entry->ready = true;
			*variable = *(T*)entry->stack;
			delete (T*)entry->stack;
			entry->stack = 0;
		}
	}
	else
	{
		entry->init= false;
		entry->ready= false;
		
		entry->buffer->lock();
		if(entry->buffer->size())
		{
			*variable = *(T*)entry->buffer->at(0);
			delete (T*)entry->buffer->at(0);
			entry->buffer->remove(0);
		}
		else
		{
			cerr << "ERRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR" << endl;
		}
		entry->buffer->unlock();
	}
	
	// get the last writer
	if(entry->activeNodesHistory.size())
	{
		entry->lastActiveNode = entry->activeNodesHistory.at(0);
		entry->activeNodesHistory.erase(entry->activeNodesHistory.begin());
	}
	
	
	entry->lockBit.unlock();
}


template<class T> void Node::done(uint32_t var, int index, void* pointer, string nodeID)
{
	LocalDataVectoryEntry* variable = NULL;
	if(index != -1)
	{
		variable = _DM14VARIABLESMAP.at(var).elements->at(index);
	}
	else
	{
		variable = &_DM14VARIABLESMAP.at(var);
	}
	
	
	if(!variable->stack || variable->channel || variable->recurrent || index != -1)
	{
		variable->lockBit.lock();
		
		if(variable->stack && variable->stack != pointer)
		{
			delete (T*)variable->stack;
		}
		
		T* tmp = new T();
		*tmp = *(T*)pointer;
		variable->stack = (void*) tmp;
		variable->ready = true;
		variable->lockBit.unlock();
	}
	
	if(nodeID.size())
	{
		sendVariable(var, index, findNode(nodeID));
	}
	
	triggerEvents();
}

template<class T> void Node::addVectorArrayData(uint32_t var, long index, bool parray, bool pready, void* pvar, bool precurrent, bool pchannel, const string& ptype)
{
	if(parray)
	{
		_DM14VARIABLESMAP.push_back(LocalDataVectoryEntry(parray, pready, pvar, precurrent, pchannel, ptype));
		_DM14VARIABLESMAP.at(var).elements = new Array<LocalDataVectoryEntry*>();
		
		T* array = (T*)pvar;
		for ( long i =0; i <index; i++)
		{
			_DM14VARIABLESMAP.at(var).elements->push_back(new LocalDataVectoryEntry());
			addVectorData<T>(var, i, false, pready, &array->at(i),precurrent, pchannel, ptype);
		}
	}
};


template<class T> void Node::addVectorClassData(uint32_t var, long index, bool parray, bool pready, void* pvar, bool precurrent, bool pchannel, const string& ptype)
{
	if(_DM14VARIABLESMAP.size()-1 < var)
	{
		_DM14VARIABLESMAP.push_back(LocalDataVectoryEntry(parray, pready, pvar, precurrent, pchannel, ptype));
	}
	
	if (index > -1)
	{
		
		if(!_DM14VARIABLESMAP.at(var).elements)
		{
			_DM14VARIABLESMAP.at(var).elements = new Array<LocalDataVectoryEntry*>();
		}
		_DM14VARIABLESMAP.at(var).elements->push_back(new LocalDataVectoryEntry(parray, pready, pvar, precurrent, pchannel, ptype));
	}
	//else
	{
		//_DM14VARIABLESMAP.push_back(LocalDataVectoryEntry(parray, pready, pvar, precurrent, pchannel, ptype));
	}
};

template<class T> void Node::addVectorData(uint32_t var, long index, bool parray, bool pready, void* pvar, bool precurrent, bool	pchannel, const string& ptype)
{
	if (index > -1)
	{
		_DM14VARIABLESMAP.at(var).elements->at(index) = new LocalDataVectoryEntry(parray, pready, pvar, precurrent, pchannel, ptype);
	}
	else
	{
		_DM14VARIABLESMAP.push_back(LocalDataVectoryEntry(parray, pready, pvar, precurrent, pchannel, ptype));
	}
};


#endif //__NODEPP
