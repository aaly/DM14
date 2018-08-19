#include "Node.hpp"
#include <stdlib.h>
using namespace std;

// if int or pos , just do it manually
// if user defined class, he has to define a function to serialize and deserialze, using our special class as a member
// serializer S;
// S.add(size of bytets, void* reference to object);
// S.size()
// SS.get() << will get the First in will be fisrst Out
// SS.get(i) << get at a position
// Type deserialize();

// FIX  make serialize return a reference to strin not a copy of it :s

void lock(volatile int* lockBit)
{
	while (__sync_lock_test_and_set(lockBit, 1))
	{
		while(*lockBit);
        // Do nothing. This GCC builtin instruction
        // ensures memory barrier.
    }
}

void unlock(volatile int* lockBit)
{
	__sync_synchronize(); // Memory barrier.
	*lockBit = 0;
}



nodePeer::nodePeer(bool dummy)
{
	node = -5;
	optimizations = 0;
	runningFlags = 0;
	//optimizations = optimizations | ARRAYBUNCHFETCH;
	ready = dummy;
	status = ALIVEANDWELL;
	//peer = NULL;
	
	Port = -1;
	IP = "";
	nodeID = "";
	peer = NULL;
};
nodePeer::~nodePeer()
{
	//delete peer;
}

int	nodePeer::sendMessage(string* msg, const bool force)
{
	if (peer && msg)
	{
		
		int ret = peer->sendMessage(*msg);
		
		if (ret == -1)
		{
			status = nodePeer::FOREVERALONE;
			return ret;
		}
		
		if (ret)
		{
			status = nodePeer::ALIVEANDWELL;
			////peer->flush();
			//ready = true;
			return 0;
		}
		else
		{
			//ready = false;
			return -1;
		}
	}
	return -1;
}
string nodePeer::recvMessage()
{
	//if(ready)
	{
		////peer->flush();
		string s = peer->recvMessage();
		
		if(s.size())
		{
			status = nodePeer::ALIVEANDWELL;
			return s;
		}
		
		if(peer->Error)
		{
			status = nodePeer::FOREVERALONE;
			return "";
		}
	}
	
	//cout <<"not reciving msg" << endl;
	return "";
}

int const nodeMessage::getType()
{
	return m_type;
};

string& nodeMessage::getValue()
{
	return m_msg;
};

nodePeer& nodeMessage::getNode()
{
	return m_node;
};


nodeMessage::nodeMessage()
{
	m_type = 0;
	m_msg = "";
}

const char* nodeMessage::typeName[16] =
{
	"R_JOIN",				// 0 Request Join
	"R_JOIN_BOOTSTRAP",		// 1 Request Join	and get bootstrap information
	"R_OBJECT",				// 2 Request Object
	"N_CLOSING",				// 3 Notify Node is Closing
	"N_R_JOINACCEPTED",		// 4 Notify Request JOIN is Accepted
	"N_R_OBJECTACCEPTED",		// 5
	"N_R_OBJECTREJECTED",		// 6
	"R_OBJECT_FROM_NODE",		// 7
	"N_R_OBJECTNOTFOUND",		// 8
	"N_R_OBJECTVALUE",		// 9
	"ADD_NODE",				// 10
	"ADD_NODE_BOOTSTRAP",		// 11
	"R_CHANNELOBJECT",		// 12 request/register channel object
	"REMOVING_DEAD_NODE",		// 13 deleteing dead in-active node
	"MISSING_NODE_ACTIVITY",	// 14 about to remove the dead an-active node
	"KEEP_NODE_ALVIE"			// 15 keep me alive, reply to other warnings
};		


Node::Node()
{
	//exclusion = 0;
	//messagesQueueLock = 0;

	//serverInit = false;
	//Self = new MessageTCPSocket();	
	Self.setNonBlocking(true);
	
	//nodes = new Array<nodePeer>();
	//clients = new Array<nodePeer>();
	//messagesQueue = new Array<nodeMessage*>();

	//sem_init(&queueSema, 0, 1);
	//sem_init(&dataSema, 0, 2);
	readready = false;
	dataVectorReady = false;
	checkTicks = 1000;
	tickerBeforeChecking = 0;
};

Node::~Node()
{
	/*
	// we need to notify all that we are dead ?
	for (unsigned int i =0; i < nodes->size(); i++)
	{
		delete nodes->at(i).peer;
	}
	delete nodes;
	for (unsigned int i =0; i < nodes->size(); i++)
	{
		delete clients->at(i).peer;
	}
	delete clients;
	delete messagesQueue;
	delete Self;
	*/
};



int Node::setNodeNumber(const int& node)
{
	nodeNumber = node;
	return node;
};


int Node::changeNodeNumber(const int& node)
{
	nodeNumber = node;
	return node;
};

int Node::setCapacity(unsigned int n)
{
	_DM14VARIABLESMAP.reserve(n);
	/*__M14VSTATUSES.reserve(n);
	
	for ( unsigned int i =0; i < n; i++)
	{
		__M14VPOINTERS.push_back(NULL);
		__M14VSTATUSES.push_back(false);
	}*/
	return n;
};

int Node::addNode(const string& address, const int& port, bool bootstrap) // to be called by main
{	
	nodePeer parent;
	parent.node = 0;
	parent.peer = new MessageTCPSocket();
	parent.IP = address;
	parent.Port = port;
	
	stringstream SS;
	SS << port;
	SS << &parent;
	parent.nodeID = address + SS.str();
	//parent.ready = true;
	parent.status = nodePeer::ALIVEANDWELL;
	if (!parent.peer->connect(address, port))
	{	
		parent.peer->setNonBlocking(true);	
		//nodes.lock();
		nodes.push_back(parent);
		//nodes.unlock();
		
		string x;
		
		if(bootstrap)
		{
			x = string(1, (unsigned char)nodeMessage::ADD_NODE_BOOTSTRAP);
		}
		else
		{
			x = string(1, (unsigned char)nodeMessage::ADD_NODE);
		}
		
		pushRequestMessage(x, nodes.at(nodes.size()-1), true);
	}
	return 0;
};



int Node::removeNode(nodePeer node)
{	
	//nodes.lock();
	nodes.remove(getNode(node));
	//nodes.unlock();
	return 0;
};



int Node::setServer(const string& address, const int& port, const int& clients) // to be called by main
{
	//serverInit = true;
	
	localIP = address;
	bindPort = port;
	clientsSize = clients;
	
	if (Self.setLocalAddressAndPort(localIP, bindPort) == -1)
	{
		displayError("Error starting own server");
	}
	
	
	return 0;
};

int Node::updateNodes()
{
	//printNodes();
	//nodes.lock();
	for (unsigned int i=0; i < nodes.size(); i++)
	{
		if(nodes.at(i).status == nodePeer::ALIVEANDWELL)
		{
			nodes.at(i).status = nodePeer::MISSEDYOU;
		}
		else if(nodes.at(i).status == nodePeer::MISSEDYOU)
		{
			string rp;
			rp += (char)nodeMessage::MISSING_NODE_ACTIVITY;
			nodes.at(i).sendMessage(&rp, true);
			nodes.at(i).status = nodePeer::DEADTOME;
		}
		else if(nodes.at(i).status == nodePeer::DEADTOME)
		{
			//notify it first we will remove it
			string rp;
			rp += (char)nodeMessage::REMOVING_DEAD_NODE;
			nodes.at(i).sendMessage(&rp, true);
		}
		else if(nodes.at(i).status == nodePeer::FOREVERALONE)
		{
			nodes.remove(i);
			i--;
		}
	}
	//nodes.unlock();
	//printNodes();
	return 0;
}

void* Node::listener(void* par) // thread;
{
	//if(nodes.size() == 0)
	{
		//nodeNumber = 0;
	}

	Self.listen(clientsSize);
	
	Serve = true;
	
	// keep loop and accept new clients, push them to clients;
	while(Serve)
	{
		if (tickerBeforeChecking == checkTicks)
		{
			//cout << "updating nodes" << endl;
			updateNodes();
			tickerBeforeChecking = 0;
		}
		//1. accept clients if exists; should be while and not if ?
		nodePeer client;
		client.node = -2;
		//client.ready = true;
		client.status = nodePeer::ALIVEANDWELL;
		
		//MessageTCPSocket* cpeer;
		while ( (client.peer=Self.accept()) != NULL)
		{
			//client.peer = *cpeer;
			client.peer->setNonBlocking(true);
			clients.push_back(client);
		}

		//2. loop through clients and handleRecievedMessage for each client;
		for (unsigned int i=0, s = clients.size(); i < s; i++)
		{
			//string msg = clients.at(i).peer->recvMessage();
			string msg = clients.at(i).recvMessage();
			if ( msg.size() > 0 )
			{
				//cout << "got message 1 : " << msg <<  " -> " << (int)msg.at(0) << endl << flush;
				pushRequestMessage(msg, clients.at(i));
			}
		}
		
		//nodes.lock();
		for (unsigned int i=0,s = nodes.size(); i < s; i++)
		{
			for (unsigned int l=0; l < 5; l++)
			{
				string msg = nodes.at(i).recvMessage();
				if ( msg.size() > 0 )
				{
					//cout << "got message : " << msg <<  " -> " << (int)msg.at(0) << endl << flush;
					pushRequestMessage(msg, nodes.at(i));
				}
			}
		}
		//nodes.unlock();
		
		messagesQueue.lock();
		while (messagesQueue.size() > 0)
		{
			handleRequest(messagesQueue.at(0));	
		}
		
		messagesQueue.flush();
		messagesQueue.unlock();
		
		tickerBeforeChecking++;
	}
	return 0;
};

int Node::pushRequestMessage(const string& msg, nodePeer node, const bool& insider)
{	
	
	//cout << "TYPE :" << (int)msg.at(0) << endl;
	nodeMessage R((int)msg.at(0), msg.substr(1, msg.size()), node);
	int ret = R.getType();
	if(!insider)
	{
		messagesQueue.lock();
	}
	
	//if(R.getNode().ready)
	if(R.getNode().ready || 
	(R.getType() == nodeMessage::R_JOIN || 
	 R.getType() == nodeMessage::R_JOIN_BOOTSTRAP || 
	 R.getType() == nodeMessage::N_R_JOINACCEPTED ||
	 R.getType() == nodeMessage::ADD_NODE ||
	 R.getType() == nodeMessage::ADD_NODE_BOOTSTRAP ||
	 R.getType() == nodeMessage::R_OBJECT_FROM_NODE))
	{
		//cout << "TYPE :" << (int)msg.at(0) << endl;
		messagesQueue.push_back(R);
	}
	
	if(!insider)
	{
		messagesQueue.unlock();
	}
	return ret;
}

string Node::Serialize(int var, int index)
{
	stringstream SS;
	
	string type = _DM14VARIABLESMAP.at(var).type;
	////cout << "TYPE:" << *(int*)_DM14VARIABLESMAP.at(var).elements->at(index)->var << "-" << var << "-" << index << endl;
	
	void** object = NULL;
	
	if (index == -1)
	{
		object = &_DM14VARIABLESMAP.at(var).var;
	}
	else
	{
		object = &_DM14VARIABLESMAP.at(var).elements->at(index)->var;
	}
	
	if(*object == NULL)
	{
		cout << "ERROR serialize" << endl << flush;
		//exit(1);
	}
	
	if (type == "int")
	{
		SS << *(int*)*object;
		return SS.str();
	}
	else if(type == "double")
	{
		SS << *(double*)*object;
		return SS.str();
	}
	else if(type == "char")
	{
		string str;
		str += *(char*)*object;
		SS.str(str);
		return SS.str();
	}
	else if(type == "string")
	{
		return *(string*)*object;
	}
	else
	{
		//.at(var)->serialize()
	}
	return 0;
};

int Node::deSerialize(int var, int index)
{
	stringstream SS;
	string type = _DM14VARIABLESMAP.at(var).type;
	
	void** object = NULL;
	//void* object = NULL;

	LocalDataVectoryEntry* variable = NULL;
		
	if (index == -1)
	{
		variable = &_DM14VARIABLESMAP.at(var);
		if (variable->channel || variable->recurrent)
		{
			object = &_DM14VARIABLESMAP.at(var).buffer->at(variable->buffer->size()-1);
		}
		else
		{
			object = &_DM14VARIABLESMAP.at(var).var;
		}
	}
	else
	{
		variable = _DM14VARIABLESMAP.at(var).elements->at(index);
		if (variable->channel || variable->recurrent)
		{
			object = &_DM14VARIABLESMAP.at(var).elements->at(index)->buffer->at(_DM14VARIABLESMAP.at(var).elements->at(index)->buffer->size()-1);
		}
		else
		{
			object = &_DM14VARIABLESMAP.at(var).elements->at(index)->var;
		}
	}
	
	//lock(&variable->lockBit);

	if (type == "int")
	{
		SS << *((string*)*object);
		int* tmp = new int;
		SS >> *tmp;
		delete (string*)*object;
		*object = tmp;
	}
	else if(type == "double")
	{
		
		SS << *(string*)*object;
		double* tmp = new double;
		SS >> *tmp;
		delete (string*)*object;
		*object = tmp;

	}
	else if(type == "char")
	{
		char* tmp = new char;
		*tmp = (*(string*)*object).at(0);
		delete (string*)*object;
		*object = tmp;
	}
	else if(type == "string")
	{
		string* tmp = new string();
		*tmp = *(string*)*object;
		delete (string*)*object;
		*object = tmp;
	}
	else
	{
		//.at(var)->deserialize()
	}
	
	//if(!variable->channel && !variable->recurrent)
	{
		variable->ready = true;
	}
	
	variable->lastActiveNode = variable->activeNodesHistory.at(0);
	variable->activeNodesHistory.erase(variable->activeNodesHistory.begin());;
	
	if(variable->lockBit)
	{
		unlock(&variable->lockBit);
	}
	//cout << " Done DES : " << var << endl << flush;

	return 0;
};

string Node::getLastActiveNode(int var, int index)
{
		LocalDataVectoryEntry* variable = NULL;
		
	if (index == -1)
	{
		return _DM14VARIABLESMAP.at(var).lastActiveNode;
	}
	else
	{
		return _DM14VARIABLESMAP.at(var).elements->at(index)->lastActiveNode;
	}
}



int Node::handleRequest(nodeMessage& R)
{	
	cout << "Handling Request:" << nodeMessage::typeName[R.getType()] << " " <<  R.getValue() << " " << endl << flush;
	
	if (R.getType() == nodeMessage::ADD_NODE || R.getType() == nodeMessage::ADD_NODE_BOOTSTRAP)
	{
		//cout << "Handling Request: ADD node : " << R.getType() << " " <<  R.getValue() << " " << endl << flush;
		string rp;
		
		if(R.getType() == nodeMessage::ADD_NODE)
		{
			rp = (char)nodeMessage::R_JOIN;
		}
		else
		{
			rp = (char)nodeMessage::R_JOIN_BOOTSTRAP;
		}
		rp += Self.getLocalAddress();
		rp += ";";
		
		
		//stringstream* SS = new stringstream();
		stringstream SS;
		SS.str("");
		SS << Self.getLocalPort();
		rp += SS.str();
		
		rp += ";";
		
		SS.str("");
		SS << nodeNumber;
		rp += SS.str();
		//R->getNode()->peer->sendMessage(rp);
		//R->getNode()->sendMessage(&rp);
		//int n = getNode(R.getNode());
		nodes.at(getNode(R.getNode())).ready = false;
		nodes.at(getNode(R.getNode())).sendMessage(&rp);
		
	}
	else if (R.getType() == nodeMessage::R_JOIN || R.getType() == nodeMessage::R_JOIN_BOOTSTRAP)
	{
		//cout << "Handling Request Request Join :" << R.getType() << " " <<  R.getValue() << " " << endl << flush;
		// we should add request add client too  to ourseves:D
		string rp;
		rp += (char)nodeMessage::N_R_JOINACCEPTED;
		stringstream SS;
		SS << nodeNumber;
		rp += SS.str();
		
		//R.getNode().ready = true;
		
		//remove from clients, add to nodes with the node number
		for (unsigned int i =0; i < clients.size(); i++)
		{
			//if(&clients.at(i) == &*R.getNode())
			if(clients.at(i).IP == R.getNode().IP
				&& clients.at(i).Port == R.getNode().Port)
			{
				//cout << "removing client and adding node " << endl;
				clients.remove(i);
			}
		}
		
		nodePeer nodepeer;
		
		SS.str("");
		SS.clear();
		SS << R.getToken(2);
		//int node = 0;
		
		
		//R.getNode().node = node;
		SS >> nodepeer.node;
		nodepeer.status = nodePeer::ALIVEANDWELL;
		nodepeer.IP = R.getToken(0);
		nodepeer.peer = R.getNode().peer;
		nodepeer.ready = true;
		
		SS.str(""); 
		SS.clear();
		SS << R.getToken(1);
		SS >> nodepeer.Port;
		
		SS.str(""); 
		SS.clear();
		SS << nodepeer.Port;
		
		time_t t;
		/* Intializes random number generator */
		srand((unsigned) time(&t));

		//SS << &nodepeer;
		SS << rand();
      
		nodepeer.nodeID = nodepeer.IP + SS.str();
		
		// we had this node and now it is back again !
		nodePeer* nodeSearch = findNode(nodepeer.nodeID);
		if(!nodeSearch)
		{
			nodes.push_back(nodepeer);
		}
		else
		{
			cerr << "WE HAD ITTTTTTT" << endl << flush;
			nodeSearch->status = nodePeer::ALIVEANDWELL;
			// re-request all variables from this node !
		}
		//nodes.lock();
		
		
		if (R.getType() == nodeMessage::R_JOIN_BOOTSTRAP)
		{
			//cout << "BOOT STRAP" << endl;
			for (unsigned int i =0; i < nodes.size()-1; i++)
			{
				if(nodes.at(i).status != nodePeer::FOREVERALONE)
				//if(nodes.at(i).ready)
				{
					//cout << "I" << i << endl;
					rp += ';';
					//rp += nodes.at(i).peer->getForeignAddress();
					rp += nodes.at(i).IP;
					rp += ';';
					SS.str(""); 
					SS.clear();

					//fix, for WAN
					//SS << nodes.at(i).peer->getForeignPort();
					SS << nodes.at(i).Port;
					rp += SS.str();
					rp += ';';
					SS.str(""); 
					SS.clear();

					//SS << nodes.at(i).Port;
					SS << nodes.at(i).node;
					rp += SS.str();
					//rp += ";";
				}
			}
		}
		//cout << rp << endl;

		nodes.at(nodes.size()-1).sendMessage(&rp);
		
		
		//request channel objects too !
		for(unsigned int i =0; i < _DM14VARIABLESMAP.size(); i++)
		{
			if(! (_DM14VARIABLESMAP.at(i).channel || _DM14VARIABLESMAP.at(i).recurrent) )
			{
				continue;
			}
			//cout << "aaaaaaaadd late" << endl << flush;
			SS.str(""); 
			SS.clear();
			SS << -2;
			SS << ';';
			SS << i;
			SS << ';';
			SS << -1;
			nodeMessage R2(nodeMessage::R_OBJECT_FROM_NODE, SS.str(), R.getNode());
			messagesQueue.push_back(R2);
		}
		//nodes.unlock();
	}
	else if (R.getType() == nodeMessage::N_R_JOINACCEPTED)
	{
		//cout << "Handling ACCEPTED :" << R.getType() << " " <<  R.getValue() << " " << endl << flush;
		findNode(R.getNode().nodeID)->ready = true;
		stringstream SS;
		
		SS.str(""); 
		 SS.clear();
		SS.str(R.getValue().substr(0, R.getValue().find(';', 0)));
		SS >> (R.getNode()).node;
		
		for(unsigned int i =0; i < _DM14VARIABLESMAP.size(); i++)
		{
			if(! (_DM14VARIABLESMAP.at(i).channel || _DM14VARIABLESMAP.at(i).recurrent) )
			{
				continue;
			}
			//cout << "aaaaaaaadd late" << endl << flush;
			SS.str(""); 
			SS.clear();
			SS << -2;
			SS << ';';
			SS << i;
			SS << ';';
			SS << -1;
			nodeMessage R2(nodeMessage::R_OBJECT_FROM_NODE, SS.str(), R.getNode());

			messagesQueue.push_back(R2);
		}
		
		//nodes.at(getNode(R->getNode()).ready = true;
		//nodes.at(getNode(R->getNode()).status = nodePeer::ALIVEANDWELL;
		
		int i = 1;
		
		while (R.getToken(i).size() != 0)
		{
			int port = 0;
			string address = R.getToken(i);
			unsigned int node = 0;
			
			SS.str(""); 
		 SS.clear();
			SS << R.getToken(i+1);
			SS >> port;
			
			SS.str(""); 
		 SS.clear();
			SS << R.getToken(i+2);
			SS >> node;
			
			//cout << "Adding :" << address << ":" << port << endl;
			addNode(address, port, false);
			i += 3;
		}
	}
	
	//==========================================================
	// ONLY SERVE IF NODE IS READY (ACCEPTED)
	//==========================================================
	
	//if(R->getNode() != NULL)
	{
		//if (!R->getNode()->ready)
		{
			//return 1;
		}
	}
	if (R.getType() == nodeMessage::R_OBJECT_FROM_NODE)
	{
		cerr << "NEED : " << R.getValue() << endl << flush;
		stringstream SS;
		int node;
		unsigned int var = 0;
		int index = -1;
		
		//SS << R->getValue().substr(0, R->getValue().find(';'));
		SS << R.getToken(0);
		SS >> node;
		SS.str(""); 
		SS.clear();
		
		
		SS << R.getToken(1);
		SS >> var;
		SS.str(""); 
		SS.clear();
		

		SS << R.getToken(2);
		SS >> index;
		SS.str(""); 
		SS.clear();
		
		
		LocalDataVectoryEntry* variable = NULL;
		
		if(index > -1)
		{
			variable = _DM14VARIABLESMAP.at(var).elements->at(index);
		}
		else
		{
			variable = &_DM14VARIABLESMAP.at(var);
		}
		
		int sent = 0;
		//nodes.lock();
		for (unsigned int i=0; i < nodes.size(); i++)
		{
			
			if (node == -2 || nodes.at(i).node == node)
			{
				//if(!nodes.at(n).ready)
				if(nodes.at(i).status == nodePeer::FOREVERALONE || !nodes.at(i).ready)
				{
					cerr << "NEED 1 : " << R.getValue() << ":" << nodes.at(i).nodeID << endl << flush;
					sent = -1;
					continue;
				}
				
				string Req("");	
				if(node == -2)
				{
					if (variable->registerWriter(nodes.at(i).nodeID) == 1)
					{
						//??
						cerr << "NEED 2 : " << R.getValue() << ":" << nodes.at(i).nodeID << endl << flush;
						variable->registerReader(nodes.at(i).nodeID); // ???
						continue;
					}
					Req += (char)nodeMessage::R_CHANNELOBJECT;
				}
				else
				{
					Req += (char)nodeMessage::R_OBJECT;
				}
				
				Req += R.getToken(1);
				Req += ';';
				Req += R.getToken(2);

				//nodes.at(n).peer->sendMessage(Req);
				cerr << "Requesting element : " << nodes.at(i).nodeID << endl;
				cerr << "Requesting element : " << nodes.at(i).ready << endl;
				nodes.at(i).sendMessage(&Req);
				//nodes.at(n).peer->flush();
				//FIX, what if we didn't find any parent with that node yet ????? we should not remove the request !
				//messagesQueue->erase(messagesQueue->begin()+i);
			}
		}
		
		if(variable->buffer != NULL)
		{
			if(variable->buffer->size() > 0)
			{
				unlock(&variable->lockBit);
			}
		}
		
		if(sent == -1)
		{
			messagesQueue.append_before(messagesQueue.at(0));
			messagesQueue.remove(0);
			return nodeMessage::R_OBJECT_FROM_NODE;
		}
	}
	else if (R.getType() == nodeMessage::R_OBJECT || R.getType() == nodeMessage::R_CHANNELOBJECT)
	{
		//cout << "Handling Request:" << R.getType() << " " <<  R.getValue() << " " << endl << flush;
		//if(!R->getNode()->ready || !dataVectorReady)
		if(R.getNode().status == nodePeer::FOREVERALONE || !dataVectorReady)
		{
			cout << "Not ready" << endl << flush;
			//??messagesQueue.lock();
			messagesQueue.append_before(messagesQueue.at(0));
			messagesQueue.erase(messagesQueue.begin());
			//messagesQueue.unlock();
			return R.getType();
		}
		
		//cout << endl << "got request : " << R->getNode()->node << " : " << R->getValue() << endl;
		stringstream SS;
	
		unsigned long var = 0;
		int index = -1;
			
		SS << R.getToken(0);
		SS >> var;
		SS.str(""); 
		SS.clear();
		
		
		SS << R.getToken(1);
		SS >> index;
		SS.str(""); 
		SS.clear();
		
		
		if(var >= _DM14VARIABLESMAP.size())
		{
			displayError("NOT FOUND");
			messagesQueue.erase(messagesQueue.begin());
			return -1;
		}
		else if(index > -1 )
		{
			if (index >= (long)_DM14VARIABLESMAP.at(var).elements->size())
			{
				displayError("INDEX NOT FOUND");
				messagesQueue.erase(messagesQueue.begin());
				return -1;
			}
		}
		else if(index < -1 )
		{
			displayError("corrupted index, corrupted request message ?");
			//??messagesQueue.lock();
			messagesQueue.erase(messagesQueue.begin());
			//messagesQueue.unlock();
			return -1;
		}
		
		string type = _DM14VARIABLESMAP.at(var).type;
		
		LocalDataVectoryEntry* variable = NULL;
		
		if(index > -1)
		{
			variable = _DM14VARIABLESMAP.at(var).elements->at(index);
		}
		else
		{
			variable = &_DM14VARIABLESMAP.at(var);
		}

		
		
		if(index != -1) // array element
		{
			//cout << "here ? noway !" << endl;
			if (_DM14VARIABLESMAP.at(var).ready)
			{
				if (!_DM14VARIABLESMAP.at(var).elements->at(index)->ready)
				{
					// we dont have this
					string Req;
					SS.str(""); 
					SS.clear();
					//FIX, oject not ready or wait, but not send this stupid message ?
					Req += (char)nodeMessage::N_R_OBJECTNOTFOUND;
					SS.str(""); 
					SS.clear();
					
					SS << var;
					Req += SS.str();
					Req += ';';
					SS.str(""); 
					SS.clear();
					
					SS << index;
					Req += SS.str();

					nodes.at(getNode(R.getNode())).sendMessage(&Req);
					
					messagesQueue.erase(messagesQueue.begin());
					return nodeMessage::R_OBJECT;
				}
			}
		}

		//registoer to this channel variable party
		if(variable->channel || variable->recurrent)
		{
			variable->registerReader(R.getNode().nodeID);
		}
			
		if (!variable->ready)
		{
			variable->registerReader(R.getNode().nodeID);
			variable->registerWriter(R.getNode().nodeID);
			messagesQueue.erase(messagesQueue.begin());
			return nodeMessage::R_OBJECT;
		}
		else
		{
			if(variable->channel || variable->recurrent)
			{
				if(variable->registerReader(R.getNode().nodeID) == 0)
				{
					variable->registerWriter(R.getNode().nodeID); // ???
					sendVariable(var, index, &R.getNode());
				}
			}
			else
			{
				sendVariable(var, index, &R.getNode());
			}
		}
	}
	else if (R.getType() == nodeMessage::N_R_OBJECTNOTFOUND)
	{
		////cout << "NOT FOUND1" << endl << flush;
		int var = 0;
		int index = -1;
		stringstream SS;
		
		SS << R.getValue().substr(0, R.getValue().find(';'));
		SS >> var;
		SS.str(""); 
		SS.clear();
		
		SS << R.getValue().substr(R.getValue().find(';')+1, R.getValue().size());
		SS >> index;
		SS.str(""); 
		SS.clear();
		
		void** object = NULL;
		volatile bool* ready;
		
		if (index == -1)
		{
			object = &_DM14VARIABLESMAP.at(var).var;
			ready = &_DM14VARIABLESMAP.at(var).ready;
		}
		else
		{
			object = &_DM14VARIABLESMAP.at(var).elements->at(index)->var;
			ready = &_DM14VARIABLESMAP.at(var).elements->at(index)->ready;
		}
		
		if(*object == NULL)
		{
			*object = NULL;
			*ready = true;
		}
	}
	else if (R.getType() == nodeMessage::N_R_OBJECTVALUE)
	{
		//cout << "got item " << R->getValue() << " from :" << R->getNode()->node << endl << flush;
		int var = 0;
		int index = -1;
		int size = 0;
		LocalDataVectoryEntry* variable = NULL;
		stringstream SS;
		

		SS << R.getToken(0);
		SS >> var;
		SS.str(""); 
		 SS.clear();
		
		SS << R.getToken(1);
		SS >> index;
		SS.str(""); 
		 SS.clear();
		
		SS << R.getToken(2);
		SS >> size;
		SS.str(""); 
		 SS.clear();
		
		if(index != -1)
		{
			variable = _DM14VARIABLESMAP.at(var).elements->at(index);
		}
		else
		{
			variable = &_DM14VARIABLESMAP.at(var);
		}
			
		// 1 already here, and not channel or recurrent !
		if (variable->ready && !variable->recurrent && !variable->channel)
		{
			messagesQueue.erase(messagesQueue.begin());
			return nodeMessage::N_R_OBJECTACCEPTED;
		}
		
		// 2  reserve the size here
		void** object = NULL;
	
		//variable->buffer->lock();
		
		if ( variable->recurrent || variable->channel )
		{
			cout << "got item " << R.getValue() << " from :" << R.getNode().node << endl << flush;
			variable->buffer->push_back(NULL);
			object = &variable->buffer->at(variable->buffer->size()-1);
			variable->init = false;
			
			variable->registerReader(R.getNode().nodeID);
		}
		else
		{
			object = &variable->var;
		}


		*object = (void*)new string();
		((string*)*object)->reserve(size);
		variable->init=true;

		// 3 get the value	
		string* val = (string*)*object;
		*val += R.getToken(3, size);
		
		cout << size << ":" << (*val).size() << ":" <<  (*val).capacity() << endl << flush;
		//if ( (*val).size() == (*val).capacity() )
		if ( (*val).size() == size )
		{
			variable->activeNodesHistory.push_back(R.getNode().nodeID);
			deSerialize(var, index);
		}
	}
	else if (R.getType() == nodeMessage::N_CLOSING)
	{
		for (unsigned int n =0; n < nodes.size(); n++)
		{	
			nodes.at(getNode(R.getNode())).status = nodePeer::FOREVERALONE;
		}
		
		/*for (unsigned int n =0; n < nodes.size(); n++)
		{	
			if(nodes.at(n).nodeID == R.getNode().nodeID)
			{
				nodes.erase(nodes.begin() + n);
			}
		}*/
	}
	else if (R.getType() == nodeMessage::MISSING_NODE_ACTIVITY)
	{
		string rp;
		rp += (char)nodeMessage::KEEP_NODE_ALVIE;
		R.getNode().sendMessage(&rp, true);
	}
	else if (R.getType() == nodeMessage::REMOVING_DEAD_NODE)
	{
		string rp;
		rp += (char)nodeMessage::KEEP_NODE_ALVIE;
		R.getNode().sendMessage(&rp, true);
	}
	else if (R.getType() == nodeMessage::KEEP_NODE_ALVIE)
	{
		string rp;
		R.getNode().sendMessage(&rp, true);
	}
	

	if(messagesQueue.size())
	{
		messagesQueue.erase(messagesQueue.begin());
	}
	return R.getType();
};


int Node::startListener(bool wait)
{
	listenerThread.start(this, &Node::listener, NULL);
	if (wait)
	{
		listenerThread.wait();
	}
	return 0;
};


int Node::serve(bool wait)
{
	if (wait)
	{
		tickerBeforeChecking = 1000000;
		listenerThread.wait();
	}
	return 0;
};

int Node::Exit()
{
	string Req;
	Req += (char)nodeMessage::N_CLOSING;
	nodes.lock();
	for (unsigned int n =0; n < nodes.size(); n++)
	{	
		//nodes.at(n).peer->sendMessage(Req);
		nodes.at(n).sendMessage(&Req);
	}
	nodes.unlock();
	Serve = false;
	if (!listenerThread.wait())
	{
		listenerThread.kill();
	}
	exit(0);
	return 0;
}

int Node::Log(string& message)
{
	////cout << "LOG: " << message << endl << flush;
	return 0;
};

int nodePeer::Log(string& message)
{
	////cout << "LOG: " << message << endl << flush;
	return 0;
};


//void Exit(int code = 0)
//{
//	exit(code);	
//}

int Node::setNodeID(const string& name)
{
	NodeID = name;
	return 0;
};

long nodeMessage::getPosition(int order)
{
	if (order == 0)
	{
		return 0;
	}
	
	int tmp = 0;
	string const& msg = m_msg;
	unsigned int size =  msg.length();
	for (unsigned int i =0; i <size; i++)
	{
		if(msg.at(i) == ';')
		{
			tmp++;
		}
		
		if(tmp == order)
		{
			return i+1;
		}
	}
	return -1;
}

string nodeMessage::getToken(int pos, unsigned long size)
{
	long from = getPosition(pos);
	
	long to;
	
	if(size)
	{
		to = size;
	}
	else
	{
		to  = getPosition(pos+1)-getPosition(pos)-1;
	}
	
	//while(to < size)
	//{
		//++pos;
		//to  = getPosition(pos+1)-getPosition(pos)-1;
	//}
	
	if(from != -1 && to != -1)
	{
		return m_msg.substr(from, to);
	}
	
	return "";
}

int Node::unmapVariable(unsigned int var, int index)
{
	void** object = NULL;	
	LocalDataVectoryEntry* variable = NULL;
	
	if(index != -1)
	{
		variable = _DM14VARIABLESMAP.at(var).elements->at(index);
	}
	else
	{
		variable = &_DM14VARIABLESMAP.at(var);
	}
		
	object = &_DM14VARIABLESMAP.at(var).var;

	
	
	if (!variable->channel && !variable->recurrent)
	{
		*object = NULL;
	}
	
	variable->ready =  false;
	variable->init =  false;
	variable->stack = false;

	return 0;
}

int Node::printNodesAddress()
{
	
	for (unsigned int i =0; i < nodes.size(); i++)
	{
		//cout  << &(*nodes.at(i).peer) << endl;
	}
	return 0;
}

long Node::getNode(nodePeer node)
{
	//nodes.lock();
	for (unsigned int i =0; i < nodes.size(); i++)
	{
		if(nodes.at(i).IP == node.IP && nodes.at(i).Port == node.Port)
		//if(&(*nodes.at(i).peer) == &(*node->peer))
		{
			//return &nodes.at(i);
			//cout << "NODE FOUNT" << node.IP << ":" << node.Port << endl;
			return i;
		}
	}
	//nodes.unlock();
	return -1;
}


nodePeer* Node::findNode(const string id)
{
	//nodes.lock();
	for (unsigned int i =0; i < nodes.size(); i++)
	{
		if(nodes.at(i).nodeID == id)
		{
			return &nodes.at(i);
		}
	}
	//nodes.unlock();
	cout << "NULL : " << id << endl;
	return NULL;
}

int Node::sendVariable(unsigned long var, long index, nodePeer* node)
{
	if(!node)
	{
		displayError("RET ");
		return 0;
	}
	
	stringstream SS;
	
	string value = Serialize(var, index);
	
	string Req("");
	SS.str("");
	SS.clear();
	
	Req = "";
	Req += nodeMessage::N_R_OBJECTVALUE;
	SS.str("");
	SS.clear();
	SS << var;
	Req += SS.str();
	Req += ';';
	
	SS.str("");
	SS.clear();
	SS << index;
	Req += SS.str();
	Req += ';';
	
	SS.str("");
	SS.clear();
	SS << value.size();
	Req += SS.str();
	Req += ';';
	
	SS.str("");
	SS.clear();
	Req += value;
	
	/*if(messagesQueue.size()-1 >= 1)
	{
		if (messagesQueue.at(1)->getNode()->node != node->node)
		{
			node->runningFlags  = node->runningFlags | ~ ARRAYBUNCHFETCH;
			node->peer->enableBufferSend(false);
		}
	}
	else
	{
		node->peer->enableBufferSend(false);
	}*/
	
	//node->peer->enableBufferSend(false);
	//cout << "sending variable : " << var << " to node : " << node->node << ":" << Req << endl << flush;
	//cout << node->IP << ":" << node->Port << endl;
	node->sendMessage(&Req);
	node->peer->flush();
	return 0;
}



int LocalDataVectoryEntry::registerReader(const string& node)
{
	bool push = true;		
	for (unsigned int i =0; i < readersParty.size(); i++)
	{
		if(readersParty.at(i) == node)
		{
			push = false;
		}
	}

	if(push)
	{
		//cout << "add reader : " << node << endl;
		readersParty.push_back(node);
		return 0;
	}
	return 1;
}


int LocalDataVectoryEntry::registerWriter(const string& node)
{
	bool push = true;
	
	for (unsigned int i =0; i < writersParty.size(); i++)
	{
		if(writersParty.at(i) == node)
		{
			push = false;
		}
	}

	if(push)
	{
		writersParty.push_back(node);
		return 0;
	}
	
	return 1;
}



int Node::printNodes()
{
	displayInfo("Connected Nodes :");
	for (unsigned int i =0; i < nodes.size(); i++)
	{
		cout << "Node [" << i << "][ " << nodes.at(i).nodeID << " ]: " << nodes.at(i).IP << ":" << nodes.at(i).Port << ", Level : " << nodes.at(i).node;
		/*if(nodes.at(i).ready)
		{
			cout << " Is Ready";
		}
		else
		{
			cout << " Is Not Ready";
		}*/
		
		if(nodes.at(i).status == nodePeer::ALIVEANDWELL)
		{
			cout << " Status : Alive and well";
		}
		else if(nodes.at(i).status == nodePeer::MISSEDYOU)
		{
			cout << " Status : Missed you";
		}
		else if(nodes.at(i).status == nodePeer::DEADTOME)
		{
			cout << " Status : Dead to me";
		}
		else if(nodes.at(i).status == nodePeer::FOREVERALONE)
		{
			cout << " Status : Forever alone";
		}
		cout << endl << flush;
	}
	return 0;
};
