#include "Node.hpp"
#include "../io/string.hpp"

// if int or pos , just do it manually
// if user defined class, he has to define a function to serialize and
// deserialze, using our special class as a member serializer S; S.add(size of
// bytets, void* reference to object); S.size() SS.get() << will get the First
// in will be fisrst Out SS.get(i) << get at a position Type deserialize();

const char *Node::messageTypeName[16] = {
    "ADD_NODE",              // 0
    "ADD_NODE_BOOTSTRAP",    // 1
    "R_JOIN",                // 2 Request Join
    "R_JOIN_BOOTSTRAP",      // 3 Request Join	and get bootstrap information
    "R_OBJECT",              // 4 Request Object
    "R_OBJECT_FROM_NODE",    // 5
    "R_CHANNELOBJECT",       // 6 request/register channel object
    "N_CLOSING",             // 7 Notify Node is Closing
    "N_R_JOINACCEPTED",      // 8 Notify Request JOIN is Accepted
    "N_R_OBJECTACCEPTED",    // 9
    "N_R_OBJECTREJECTED",    // 10
    "N_R_OBJECTNOTFOUND",    // 11
    "N_R_OBJECTVALUE",       // 12
    "REMOVING_DEAD_NODE",    // 13 deleteing dead in-active node
    "MISSING_NODE_ACTIVITY", // 14 about to remove the dead an-active node
    "KEEP_NODE_ALVIE"        // 15 keep me alive, reply to other warnings
};

int Node::Log(string &message) {
  ////cout << "LOG: " << message << endl << flush;
  return 0;
};

int nodePeer::Log(string &message) {
  ////cout << "LOG: " << message << endl << flush;
  return 0;
};

nodePeer::nodePeer(bool dummy) {
  node = -5;
  optimizations = 0;
  runningFlags = 0;
  // optimizations = optimizations | ARRAYBUNCHFETCH;
  ready = dummy;
  status = ALIVEANDWELL;
  // peer = NULL;

  Port = -1;
  IP = "";
  nodeID = "";
  peer = NULL;
};
nodePeer::~nodePeer() {
  // delete peer;
}

int nodePeer::sendMessage(string *msg, const bool force) {
  int ret = -1;

  if (peer && msg) {
    cerr << "SENDIN MESSAGE : " << *msg << endl << flush;
    cerr << "SENDIN MESSAGE : " << (int)msg->at(0) << endl << flush;
    ret = peer->sendMessage(*msg);

    if (ret == -1) {
      status = nodePeer::FOREVERALONE;
      ret = -1;
    } else {
      status = nodePeer::ALIVEANDWELL;
      ret = 0;
    }
  }
  return ret;
}
string nodePeer::recvMessage() {
  // if(ready)
  {
    ////peer->flush();
    string s = peer->recvMessage();

    if (s.size()) {
      status = nodePeer::ALIVEANDWELL;
      return s;
    }

    if (peer->Error) {
      status = nodePeer::FOREVERALONE;
      return "";
    }
  }

  // cout <<"not reciving msg" << endl;
  return "";
}

Node::Node() {
  self.setNonBlocking(true);

  if (socketpair(AF_UNIX, SOCK_STREAM, 0, pollTrigger) < 0) {
    perror("opening stream socket pair");
    exit(1);
  }

  EPollFD = epoll_create1(EPOLL_CLOEXEC);

  pollOnSocket(pollTrigger[1], EPOLLIN);
  pollOnSocket(self.getSocketDescriptor(), EPOLLIN | EPOLLOUT);

  readready = false;
  dataVectorReady = false;
  checkTicks = 1000; // 1000 msec
};

Node::~Node() {

  // we need to notify all that we are dead ?
  for (unsigned int i = 0; i < nodes.size(); i++) {
    delete nodes.at(i).peer;
  }

  for (unsigned int i = 0; i < clients.size(); i++) {
    delete clients.at(i).peer;
  }
};

int Node::setNodeNumber(const int &node) {
  nodeNumber = node;
  return node;
};

int Node::requestObject(uint32_t var, int index, int nodeLevel) {
  nodeMessage message;
  message.setType(Node::R_OBJECT_FROM_NODE);
  message.setNodeIndex(nodeLevel);

  message.pushToken(numberToStr(nodeLevel));
  message.pushToken(numberToStr(var));
  message.pushToken(numberToStr(index));

  return pushRequestMessage(message, false);
};

int Node::requestObject(uint32_t var, int index, string nodeAddress) {
  nodePeer *node = findNode(nodeAddress);

  nodeMessage message;
  message.setType(Node::R_OBJECT_FROM_NODE);
  message.setNodeIndex(node->node);

  message.pushToken(numberToStr(node->node));
  message.pushToken(numberToStr(var));
  message.pushToken(numberToStr(index));

  return pushRequestMessage(message, false);
};

int Node::changeNodeNumber(const int &node) {
  nodeNumber = node;
  return node;
};

int Node::setCapacity(unsigned int n) {
  _DM14VARIABLESMAP.reserve(n);
  return _DM14VARIABLESMAP.size();
};

int Node::pollOnSocket(int socketFD,
                       uint32_t events = EPOLLIN | EPOLLHUP | EPOLLERR) {
  static struct epoll_event ev;
  ev.events = events;
  ev.data.fd = socketFD;
  return epoll_ctl(EPollFD, EPOLL_CTL_ADD, socketFD, &ev);
}

int Node::pollOffSocket(int socketFD) {
  static struct epoll_event ev;
  ev.events = 0;
  ev.data.fd = socketFD;
  return epoll_ctl(EPollFD, EPOLL_CTL_DEL, socketFD, &ev);
}

int Node::addNode(const string &address, const string &port,
                  bool bootstrap) // to be called by main
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
  // parent.ready = true;
  parent.status = nodePeer::ALIVEANDWELL;

  if (!parent.peer->connect(address, port)) {
    parent.peer->setNonBlocking(true);
    nodes.lock();
    nodes.push_back(parent);
    // pollOnSocket(parent.peer->getSocketDescriptor(), EPOLLIN);
    pollOnSocket(parent.peer->getSocketDescriptor());
    nodes.unlock();

    nodeMessage message;

    if (bootstrap) {
      message.setType(Node::ADD_NODE_BOOTSTRAP);
    } else {
      message.setType(Node::ADD_NODE);
    }

    nodes.lock();
    message.setNodeIndex(nodes.size() - 1);
    pushRequestMessage(message, !bootstrap);
    nodes.unlock();
  } else {
    cerr << "Failed to connect to parent " << address << ":" << port << endl
         << flush;
    exit(1);
  }
  return 0;
};

int Node::removeNode(nodePeer &node) {
  nodes.lock();
  messagesQueue.lock();
  int index = getNodeIndex(node);
  if (index != -1) {
    for (unsigned int i = 0; i < messagesQueue.size(); i++) {
      if (messagesQueue.at(i).getNodeIndex() == index) {
        messagesQueue.remove(i);
        i--;
      }
    }
    nodes.remove(index);
  }
  messagesQueue.flush();
  messagesQueue.unlock();
  nodes.unlock();
  return 0;
};

int Node::setServer(const string &address, const string &port,
                    const int &clients) // to be called by main
{
  // serverInit = true;

  localIP = address;
  bindPort = port;
  clientsSize = clients;

  if (self.setLocalAddressAndPort(localIP, bindPort) == -1) {
    displayError("Error starting own server");
  }

  return 0;
};

int Node::updateNodes() {
  // printNodes();
  // nodes.lock();
  for (unsigned int i = 0; i < nodes.size(); i++) {
    if (nodes.at(i).status == nodePeer::ALIVEANDWELL) {
      nodes.at(i).status = nodePeer::MISSEDYOU;
    } else if (nodes.at(i).status == nodePeer::MISSEDYOU) {
      string rp;
      rp += (char)Node::MISSING_NODE_ACTIVITY;
      nodes.at(i).sendMessage(&rp, true);
      nodes.at(i).status = nodePeer::DEADTOME;
    } else if (nodes.at(i).status == nodePeer::DEADTOME) {
      // notify it first we will remove it
      string rp;
      rp += (char)Node::REMOVING_DEAD_NODE;
      nodes.at(i).sendMessage(&rp, true);
    } else if (nodes.at(i).status == nodePeer::FOREVERALONE) {
      nodes.at(i).peer->Shutdown();
      nodes.lock();
      nodes.remove(i);
      nodes.unlock();
      i--;
    }
  }
  // nodes.unlock();
  // printNodes();
  return 0;
}

int Node::processMessages() {
  messagesQueue.lock();
  while (messagesQueue.size() > 0) {
    handleRequest(messagesQueue.at(0));
  }
  messagesQueue.flush();
  messagesQueue.unlock();
  return 0;
}

nodePeer *Node::findNodeByFd(int fd) {
  nodes.lock();
  for (unsigned int k = 0, s = nodes.size(); k < s; k++) {
    if (nodes.at(k).peer->getSocketDescriptor() == fd) {
      nodePeer *peer = &nodes[k];
      nodes.unlock();
      return peer;
    }
  }
  nodes.unlock();
  return NULL;
}
void *Node::listener(void *par) // thread;
{

  self.listen(clientsSize);
  serve = true;
  int eventsnum = 100;
  // struct epoll_event events[eventsnum];
  struct epoll_event *events =
      (struct epoll_event *)calloc(eventsnum, sizeof(struct epoll_event));

  // keep loop and accept new clients, push them to clients;
  while (serve) {
    processMessages();
    int rc = epoll_wait(EPollFD, events, eventsnum, -1);
    // int rc = epoll_wait(EPollFD, events, eventsnum, 1000);
    if (rc < 0) {
      perror("epolll() failed");
      exit(rc);
      continue;
    }

    if (rc == 0) {
      // perror("poll() timed out.  End program.");
      // updateNodes();
      // exit(rc);
      continue;
    }

    for (int i = 0; i < rc; i++) {
      if (events[i].events & EPOLLHUP || events[i].events & EPOLLRDHUP ||
          events[i].events & EPOLLERR || events[i].events & EPOLLET) {
        cerr << "REMOVEEEEEEE" << endl << flush;
        pollOffSocket(events[i].data.fd);
        removeNode(*findNodeByFd(events[i].data.fd));

        continue;
      }

      // 1. accept clients if exists; should be while and not if ?
      if (events[i].data.fd ==
          self.getSocketDescriptor()) // && (monitoredDescriptors[i].revents &
                                      // POLLIN))
      {
        MessageTCPSocket *tcpsock = NULL;

        while ((tcpsock = self.accept()) != NULL) {
          // int poll_result =
          // pollOnSocket(nodes.at(index).peer->getSocketDescriptor());
          int poll_result = pollOnSocket(tcpsock->getSocketDescriptor());

          if (poll_result == 0) {
            int index = 0;
            nodes.lock();
            nodes.push_back(nodePeer());
            index = nodes.size() - 1;
            nodes.at(index).node = -2;
            nodes.at(index).peer = tcpsock;
            nodes.at(index).status = nodePeer::ALIVEANDWELL;
            nodes.at(index).ready = false;
            nodes.at(index).peer->setNonBlocking(true);
            nodes.unlock();

            // get any immediate messages
            nodeMessage msg = nodeMessage(nodes.at(index).recvMessage(), index);

            if (msg.getMessage().size() > 0) {
              pushRequestMessage(msg);
            }
          } else {
            cerr << "Error polling on a new client" << endl;
          }
        }
      }
      // just the trigger to fire pll ;)
      else if (events[i].data.fd == pollTrigger[1]) {
        char buff[100];
        read(pollTrigger[1], &buff, 1);
      }
      // 2. loop through clients/nodes and handleRecievedMessage for each
      // client;
      else {
        nodePeer *peer = findNodeByFd(events[i].data.fd);
        int peerIndex = findNodeIndex(peer->nodeID);
        if (peer) {
          nodeMessage msg = nodeMessage(peer->recvMessage(), peerIndex);
          if (msg.getMessage().size() > 0) {
            pushRequestMessage(msg, false);
          } else {
            cerr << "REMOVEEEEEEE2" << endl << flush;
            int count;
            ioctl(peer->peer->getSocketDescriptor(), FIONREAD, &count);
            if (!count) {
              pollOffSocket(events[i].data.fd);
              removeNode(*peer);
            }
          }
        }
      }
    }
  }
  return 0;
};

int Node::appendRequestMessage(nodeMessage R) {
  messagesQueue.push_back(R);
  return triggerEvents();
}

int Node::pushRequestMessage(nodeMessage &msg, const bool &insider) {
  // cout << "PUSH TYPE :" << nodeMessage::typeName[(int)msg.at(0)] << endl <<
  // flush; int ret = msg.getType();

  if (!insider) {
    messagesQueue.lock();
  }

  messagesQueue.push_back(msg);

  if (!insider) {
    messagesQueue.unlock();
  }

  return triggerEvents();
}

string Node::Serialize(uint32_t var, int index) {
  stringstream SS;

  string type = _DM14VARIABLESMAP.at(var).type;
  ////cout << "TYPE:" <<
  ///*(int*)_DM14VARIABLESMAP.at(var).elements->at(index)->var << "-" << var <<
  ///"-" << index << endl;

  void *object = NULL;

  if (index == -1) {
    object = _DM14VARIABLESMAP.at(var).stack;
  } else {
    object = _DM14VARIABLESMAP.at(var).elements->at(index)->stack;
  }

  if (object == NULL) {
    cout << "ERROR serialize" << endl << flush;
    // exit(1);
  }

  if (type == "int") {
    SS << *(int *)object;
    return SS.str();
  } else if (type == "double") {
    SS << *(double *)object;
    return SS.str();
  } else if (type == "float") {
    SS << *(float *)object;
    return SS.str();
  } else if (type == "char") {
    string str;
    str += *(char *)object;
    SS.str(str);
    return SS.str();
  } else if (type == "string") {
    return *(string *)object;
  } else {
    //.at(var)->serialize()
  }
  // cerr << "SERIALIZE :" << var << index << endl;
  return "";
};

int Node::deSerialize(uint32_t var, int index) {
  stringstream SS;
  string type = _DM14VARIABLESMAP.at(var).type;

  void **object = NULL;

  LocalDataVectoryEntry *variable = NULL;

  if (index == -1) {
    variable = &_DM14VARIABLESMAP.at(var);
    if (variable->channel || variable->recurrent) {
      object =
          &_DM14VARIABLESMAP.at(var).buffer->at(variable->buffer->size() - 1);
    } else {
      object = &_DM14VARIABLESMAP.at(var).stack;
    }
  } else {
    variable = _DM14VARIABLESMAP.at(var).elements->at(index);
    if (variable->channel || variable->recurrent) {
      object = &_DM14VARIABLESMAP.at(var).elements->at(index)->buffer->at(
          _DM14VARIABLESMAP.at(var).elements->at(index)->buffer->size() - 1);
    } else {
      object = &_DM14VARIABLESMAP.at(var).elements->at(index)->stack;
    }
  }

  if (type == "int") {
    SS << *(string *)*object;
    delete (string *)*object;
    *object = (void *)new int;
    SS >> *(int *)*object;
  } else if (type == "double") {
    SS << *(string *)*object;
    delete (string *)*object;
    *object = (void *)new double;
    SS >> *(double *)*object;
  } else if (type == "float") {
    SS << *(string *)*object;
    delete (string *)*object;
    *object = (void *)new float;
    SS >> *(float *)*object;
  } else if (type == "char") {
    SS << (*(string *)*object).at(0);
    delete (string *)*object;
    *object = (void *)new char;
    SS >> *(char *)*object;
  } else if (type == "string") {
    // already a string !
  } else {
    ; //.at(var)->deserialize()
  }
  return 0;
};

string Node::getLastActiveNode(uint32_t var, int index) {
  // LocalDataVectoryEntry* variable = NULL;

  string res;

  if (index == -1) {
    //_DM14VARIABLESMAP.at(var).lock();
    res = _DM14VARIABLESMAP.at(var).lastActiveNode;
    //_DM14VARIABLESMAP.at(var).unlock();
  } else {
    //_DM14VARIABLESMAP.at(var).elements->at(index)->lock();
    res = _DM14VARIABLESMAP.at(var).elements->at(index)->lastActiveNode;
    //_DM14VARIABLESMAP.at(var).elements->at(index)->unlock();
  }

  return res;
}

int Node::handleRequest(nodeMessage &R) {
  cerr << "Handling Requesttt:" << Node::messageTypeName[R.getType()] << " "
       << R.getMessage() << " " << endl
       << flush;

  if (R.getType() == Node::ADD_NODE ||
      R.getType() == Node::ADD_NODE_BOOTSTRAP) {
    nodeMessage reply;

    if (R.getType() == Node::ADD_NODE) {
      reply.setType(Node::R_JOIN);
    } else {
      reply.setType(Node::R_JOIN_BOOTSTRAP);
    }

    reply.pushToken(self.getLocalAddress());
    reply.pushToken(numberToStr(self.getLocalPort()));
    reply.pushToken(numberToStr(nodeNumber));

    nodes.at(R.getNodeIndex()).ready = false;
    string packedMessage = reply.getPackedMessage();
    nodes.at(R.getNodeIndex()).sendMessage(&packedMessage);
  } else if (R.getType() == Node::R_JOIN ||
             R.getType() == Node::R_JOIN_BOOTSTRAP) {
    cout << "Handling Request Request Join :" << R.getType() << " "
         << R.getMessage() << " " << endl
         << flush;
    // we should add request add client too  to ourseves:D
    nodeMessage reply;
    reply.setType(Node::N_R_JOINACCEPTED);
    reply.pushToken(numberToStr(nodeNumber));

    nodePeer *nodepeer = &nodes.at(R.getNodeIndex());

    nodepeer->node = R.getNodeIndex();
    nodepeer->status = nodePeer::ALIVEANDWELL;
    nodepeer->IP = R.getToken(0);
    nodepeer->peer = nodes.at(R.getNodeIndex()).peer;
    nodepeer->ready = true;
    nodepeer->Port = R.getToken(1);

    // Intializes random number generator
    time_t t;
    srand((unsigned)time(&t));

    nodepeer->nodeID = nodepeer->IP + nodepeer->Port + numberToStr(rand());
    nodepeer->status = nodePeer::ALIVEANDWELL;

    // nodes.lock();

    // cout << "ACCPTED : " << nodepeer->nodeID << endl;
    if (R.getType() == Node::R_JOIN_BOOTSTRAP) {
      // cout << "BOOT STRAP" << endl;
      for (unsigned int i = 0; i < nodes.size() - 1; i++) {
        if (nodes.at(i).status != nodePeer::FOREVERALONE)
        // if(nodes.at(i).ready)
        {
          reply.pushToken(nodes.at(i).IP);

          // fix, for WAN
          // SS << nodes.at(i).peer->getForeignPort();
          reply.pushToken(nodes.at(i).Port);
          reply.pushToken(numberToStr(nodes.at(i).node));
        }
      }
    }
    string packedMessage = reply.getPackedMessage();
    nodepeer->sendMessage(&packedMessage);
  } else if (R.getType() == Node::N_R_JOINACCEPTED) {
    // cout << "Handling ACCEPTED :" << R.getType() << " " <<  R.getMessage() <<
    // " " << endl << flush; findNode(R.getNode().nodeID)->ready = true;
    nodes.at(R.getNodeIndex()).ready = true;
    nodes.at(R.getNodeIndex()).node =
        strToNumber(R.getMessage().substr(0, R.getMessage().find(';', 0)));

    int i = 1;

    while (R.getToken(i).size() != 0) {
      string port = "";
      string address = R.getToken(i);
      unsigned int node = 0;

      port = R.getToken(i + 1);
      node = strToNumber(R.getToken(i + 2));
      addNode(address, port, false);
      i += 3;
    }
  }
  //==========================================================
  // ONLY SERVE IF NODE IS READY (ACCEPTED)
  //==========================================================
  else if (R.getType() == Node::R_OBJECT_FROM_NODE) {
    cerr << "NEED : " << R.getMessage() << endl << flush;
    string nodeAddress = "";

    if (R.getNodeIndex() != -1) {
      // nodes.at(R.getNodeIndex()).nodeID;
    }
    int node = strToNumber(R.getToken(0));
    unsigned int var = strToNumber(R.getToken(1));
    int index = strToNumber(R.getToken(2));

    cerr << "node : " << node << endl << flush;
    cerr << "var : " << var << endl << flush;
    cerr << "index : " << index << endl << flush;

    LocalDataVectoryEntry *variable = NULL;

    if (index > -1) {
      variable = _DM14VARIABLESMAP.at(var).elements->at(index);
    } else {
      variable = &_DM14VARIABLESMAP.at(var);
    }

    int sent = -1;

    nodes.lock();

    for (unsigned int i = 0; i < nodes.size(); i++) {
      if (nodeAddress == nodes.at(i).nodeID) {

        nodeMessage Req;
        if (node == -2) {
          variable->registerWriter(nodes.at(i).nodeID);
          Req.setType(Node::R_CHANNELOBJECT);
        } else {
          Req.setType(Node::R_OBJECT);
        }

        Req.pushToken(R.getToken(1));
        Req.pushToken(R.getToken(2));
        string packedMessage = Req.getPackedMessage();
        nodes.at(i).sendMessage(&packedMessage);
        sent = 0;
        break;
      } else if (node == -2 || nodes.at(i).node == node) {
        if (nodes.at(i).status == nodePeer::FOREVERALONE ||
            !nodes.at(i).ready) {
          sent = -1;
          continue;
        }

        nodeMessage Req;
        if (node == -2) {
          if (variable->registerWriter(nodes.at(i).nodeID) == 1) {
            continue;
          }
          Req.setType(Node::R_CHANNELOBJECT);
        } else {
          Req.setType(Node::R_OBJECT);
        }

        Req.pushToken(R.getToken(1));
        Req.pushToken(R.getToken(2));
        string packedMessage = Req.getPackedMessage();
        nodes.at(i).sendMessage(&packedMessage);
        sent = 0;
      }
    }
    nodes.unlock();

    if (sent == -1) {
      // cerr << "STILLLLLLLLLLLLLLLLLL" << endl;
      messagesQueue.append_before(R);
      messagesQueue.remove(0);
      return Node::R_OBJECT_FROM_NODE;
    }
  } else if (R.getType() == Node::R_OBJECT ||
             R.getType() == Node::R_CHANNELOBJECT) {
    // cerr << "CHANNEL OBJET:" << R.getType() << " " <<  R.getValue() << " " <<
    // endl << flush; if(R.getNode().status == nodePeer::FOREVERALONE)
    if (nodes.at(R.getNodeIndex()).status == nodePeer::FOREVERALONE) {
      // delete this request and return
      messagesQueue.remove(0);
      return R.getType();
    } else if (!dataVectorReady) {
      // append this request for another trial and retry later
      // appendRequestMessage(messagesQueue.at(0));
      // we are already inside a loop that is checking for size() > 0, so not
      // good to push_back ok ? ;)
      messagesQueue.append_before(R);
      messagesQueue.remove(0);
      // triggerEvents();
      return R.getType();
    }

    // cout << endl << "got request : " << R->getNode()->node << " : " <<
    // R->getValue() << endl;

    unsigned long var = 0;
    int index = -1;

    var = strToNumber(R.getToken(0));
    index = strToNumber(R.getToken(1));

    if (var >= _DM14VARIABLESMAP.size()) {
      displayError("NOT FOUND");
      messagesQueue.erase(messagesQueue.begin());
      return -1;
    } else if (index > -1) {
      if (index >= (long)_DM14VARIABLESMAP.at(var).elements->size()) {
        displayError("INDEX NOT FOUND");
        messagesQueue.erase(messagesQueue.begin());
        return -1;
      }
    } else if (index < -1) {
      displayError("corrupted index, corrupted request message ?");
      messagesQueue.erase(messagesQueue.begin());
      return -1;
    }

    string type = _DM14VARIABLESMAP.at(var).type;

    LocalDataVectoryEntry *variable = NULL;

    if (index > -1) {
      variable = _DM14VARIABLESMAP.at(var).elements->at(index);
    } else {
      variable = &_DM14VARIABLESMAP.at(var);
    }

    if (index != -1) // array element
    {
      if (_DM14VARIABLESMAP.at(var).ready || _DM14VARIABLESMAP.at(var).stack) {
        if (!_DM14VARIABLESMAP.at(var).elements->at(index)->init &&
            !_DM14VARIABLESMAP.at(var).elements->at(index)->stack) {
          // cerr << "NO Ready1" << endl << flush;
          //  we dont have this
          nodeMessage Req;
          var = strToNumber(R.getToken(0));
          // FIX, oject not ready or wait, but not send this stupid message ?
          Req.setType(Node::N_R_OBJECTNOTFOUND);

          Req.pushToken(numberToStr(var));
          Req.pushToken(numberToStr(index));
          string packedMessage = Req.getPackedMessage();
          nodes.at(R.getNodeIndex()).sendMessage(&packedMessage);
          messagesQueue.erase(messagesQueue.begin());
          return Node::R_OBJECT;
        }
      }
    }

    if (!variable->stack) {
      messagesQueue.append_before(R);
      messagesQueue.erase(messagesQueue.begin());
      return Node::R_OBJECT;
    } else {
      if (variable->registerReader(nodes.at(R.getNodeIndex()).nodeID) == 0) {
        sendVariable(var, index, &nodes.at(R.getNodeIndex()));
      }
    }
  } else if (R.getType() == Node::N_R_OBJECTNOTFOUND) {
    /*int var = 0;
    int index = -1;

    var = strToNumber(R.getMessage().substr(0, R.getMessage().find(';')));
    index = strToNumber(R.getMessage().substr(R.getMessage().find(';')+1,
    R.getMessage().size()));*/
  } else if (R.getType() == Node::N_R_OBJECTVALUE) {
    // cout << "got item " << R.getValue() << " from :" << R.getNode().nodeID <<
    // endl << flush; cout << "got item " << R.getValue() << " from :" <<
    // nodes.at(R.getNodeIndex()).nodeID << endl << flush;
    int var = 0;
    int index = -1;
    unsigned int size = 0;
    LocalDataVectoryEntry *variable = NULL;

    var = strToNumber(R.getToken(0));
    index = strToNumber(R.getToken(1));
    size = strToNumber(R.getToken(2));

    if (index != -1) {
      variable = _DM14VARIABLESMAP.at(var).elements->at(index);
    } else {
      variable = &_DM14VARIABLESMAP.at(var);
    }

    variable->lockBit.lock();

    // 1 already here, and not channel or recurrent !
    if (variable->init && !variable->recurrent && !variable->channel) {
      variable->lockBit.unlock();
      messagesQueue.erase(messagesQueue.begin());
      return Node::N_R_OBJECTACCEPTED;
    }

    // 2  reserve the size here
    variable->buffer->lock();

    string *object = NULL;

    if (!variable->init || variable->recurrent || variable->channel) {
      object = new string();
      object->reserve(size);
      variable->init = true;
    }

    if (variable->recurrent || variable->channel) {
      variable->buffer->push_back(object);
    } else {
      variable->stack = (void *)object;
    }

    // 3 get the value
    *object += R.getToken(3, size);

    if (object->size() == size) {
      variable->activeNodesHistory.push_back(nodes.at(R.getNodeIndex()).nodeID);
      deSerialize(var, index);
      variable->readySignal.unlock();
      variable->ready = true;
    }

    variable->buffer->unlock();
    variable->lockBit.unlock();

  } else if (R.getType() == Node::N_CLOSING) {
    for (unsigned int n = 0; n < nodes.size(); n++) {
      nodes.at(R.getNodeIndex()).status = nodePeer::FOREVERALONE;
    }

    /*for (unsigned int n =0; n < nodes.size(); n++)
            if(nodes.at(n).nodeID == R.getNode().nodeID)
            {
                    nodes.erase(nodes.begin() + n);
            }
    }*/
  } else if (R.getType() == Node::MISSING_NODE_ACTIVITY) {
    string rp;
    rp += (char)Node::KEEP_NODE_ALVIE;
    nodes.at(R.getNodeIndex()).sendMessage(&rp, true);
  } else if (R.getType() == Node::REMOVING_DEAD_NODE) {
    string rp;
    rp += (char)Node::KEEP_NODE_ALVIE;
    nodes.at(R.getNodeIndex()).sendMessage(&rp, true);
  } else if (R.getType() == Node::KEEP_NODE_ALVIE) {
    string rp;
    nodes.at(R.getNodeIndex()).sendMessage(&rp, true);
  }

  if (messagesQueue.size()) {
    messagesQueue.erase(messagesQueue.begin());
  }
  return R.getType();
};

int Node::startListener(bool wait) {
  listenerThread.start(this, &Node::listener, NULL);
  if (wait) {
    listenerThread.wait();
  }
  return 0;
};

int Node::setServe(bool wait) {
  if (wait) {
    checkTicks = 1000000;
    listenerThread.wait();
  }
  return 0;
};

int Node::triggerEvents() {
  // write a byte to wake up the poll before it's timeout ;)
  if (pollTrigger[0] && write(pollTrigger[0], "w", 1) < 0) {
    perror("writing stream message");
    return 1;
  }
  return 0;
}

int Node::Exit(int status) {
  string Req;
  Req += (char)Node::N_CLOSING;
  nodes.lock();
  for (unsigned int n = 0; n < nodes.size(); n++) {
    // nodes.at(n).peer->sendMessage(Req);
    nodes.at(n).sendMessage(&Req);
  }
  nodes.unlock();
  serve = false;
  triggerEvents();
  if (!listenerThread.wait()) {
    listenerThread.kill();
  }
  exit(status);
  return status;
}

int Node::setNodeID(const string &name) {
  NodeID = name;
  return 0;
};

int Node::unmapVariable(unsigned int var, int index) {
  void **object = NULL;
  LocalDataVectoryEntry *variable = NULL;

  if (index != -1) {
    variable = _DM14VARIABLESMAP.at(var).elements->at(index);
  } else {
    variable = &_DM14VARIABLESMAP.at(var);
  }

  object = &_DM14VARIABLESMAP.at(var).stack;

  if (!variable->channel && !variable->recurrent) {
    *object = NULL;
  }

  // variable->ready =  false;
  variable->init = false;

  return 0;
}

int Node::printNodesAddress() {

  for (unsigned int i = 0; i < nodes.size(); i++) {
    // cout  << &(*nodes.at(i).peer) << endl;
  }
  return 0;
}

long Node::getNodeIndex(nodePeer &node) {
  // nodes.lock();
  for (unsigned int i = 0; i < nodes.size(); i++) {
    if (nodes.at(i).IP == node.IP && nodes.at(i).Port == node.Port)
    // if(&(*nodes.at(i).peer) == &(*node->peer))
    {
      // return &nodes.at(i);
      // cout << "NODE FOUNT" << node.IP << ":" << node.Port << endl;
      return i;
    }
  }
  // nodes.unlock();
  return -1;
}

int Node::findNodeIndex(const string id) {
  // nodes.lock();
  for (unsigned int i = 0; i < nodes.size(); i++) {
    if (nodes.at(i).nodeID == id) {
      return i;
    }
  }
  // nodes.unlock();
  return -1;
}

nodePeer *Node::findNode(const string id) {
  // nodes.lock();
  for (unsigned int i = 0; i < nodes.size(); i++) {
    if (nodes.at(i).nodeID == id) {
      return &nodes.at(i);
    }
  }
  // nodes.unlock();
  return NULL;
}

int Node::sendVariable(unsigned long var, long index, nodePeer *node) {
  if (!node) {
    displayError("RET ");
    return 0;
  }

  stringstream SS;

  string value = Serialize(var, index);

  string Req("");
  SS.str("");
  SS.clear();

  Req = "";
  Req += Node::N_R_OBJECTVALUE;
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

  // node->peer->enableBufferSend(false);
  ////cout << "sending variable : " << var << " to node : " << node->node << ":"
  ///<< Req << endl << flush;
  // cout << node->IP << ":" << node->Port << endl;
  node->sendMessage(&Req);
  node->peer->flush();
  return 0;
}

int LocalDataVectoryEntry::registerReader(const string &node) {
  bool push = true;
  readersParty.lock();
  for (unsigned int i = 0; i < readersParty.size(); i++) {
    if (readersParty.at(i) == node) {
      push = false;
    }
  }

  if (push) {
    // cout << "add reader : " << node << endl;
    readersParty.push_back(node);
    readersParty.unlock();
    return 0;
  }
  readersParty.unlock();
  return 1;
}

int LocalDataVectoryEntry::registerWriter(const string &node) {
  bool push = true;

  writersParty.lock();
  for (unsigned int i = 0; i < writersParty.size(); i++) {
    if (writersParty.at(i) == node) {
      push = false;
    }
  }

  if (push) {

    writersParty.push_back(node);
    writersParty.unlock();
    return 0;
  }
  writersParty.unlock();

  return 1;
}

int Node::printNodes() {
  displayInfo("Connected Nodes :");
  for (unsigned int i = 0; i < nodes.size(); i++) {
    cout << "Node [" << i << "][ " << nodes.at(i).nodeID
         << " ]: " << nodes.at(i).IP << ":" << nodes.at(i).Port
         << ", Level : " << nodes.at(i).node;
    if (nodes.at(i).ready) {
      cout << " Is Ready";
    } else {
      cout << " Is Not Ready";
    }

    if (nodes.at(i).status == nodePeer::ALIVEANDWELL) {
      cout << " Status : Alive and well";
    } else if (nodes.at(i).status == nodePeer::MISSEDYOU) {
      cout << " Status : Missed you";
    } else if (nodes.at(i).status == nodePeer::DEADTOME) {
      cout << " Status : Dead to me";
    } else if (nodes.at(i).status == nodePeer::FOREVERALONE) {
      cout << " Status : Forever alone";
    }
    cout << endl << flush;
  }
  return 0;
};
