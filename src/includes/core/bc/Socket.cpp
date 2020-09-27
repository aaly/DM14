#include "Socket.hpp"
#include "Sleep.hpp"

// SocketException Code

SocketException::SocketException(const string &message, bool inclSysMsg)
  throw() : userMessage(message)
{
	if (inclSysMsg)
	{
		userMessage.append(": ");
		userMessage.append(strerror(errno));
	}
}

SocketException::~SocketException() throw()
{
	
}

const char *SocketException::what() const throw()
{
	return userMessage.c_str();
}

// Function to fill in address structure given an address and port
static void fillAddr(const string &address, unsigned short port, sockaddr_in &addr)
{
	memset(&addr, 0, sizeof(addr));  // Zero out address structure
	addr.sin_family = AF_INET;       // Internet address

	hostent *host;  // Resolve name
	if ((host = gethostbyname(address.c_str())) == NULL)
	{
		// strerror() will not work for gethostbyname() and hstrerror() 
		// is supposedly obsolete
		//throw SocketException("Failed to resolve name (gethostbyname())");
	}
	addr.sin_addr.s_addr = *((unsigned long *) host->h_addr_list[0]);
	addr.sin_port = htons(port);     // Assign port in network byte order
}








// Socket Code

//Socket::Socket()
//{
//	sockDesc = -1
//	Error = 0;
//}


Socket::Socket()
{
	Socket(SOCK_STREAM, IPPROTO_TCP);
	sockDesc = -1;
}

//Socket::Socket(const Socket &sock)
//{
//	cout <<  "SOCK:" << sockDesc << endl;
//	sockDesc = sock.sockDesc;
//	Error = 0;
//}


int Socket::getSocketDescriptor()
{
	return sockDesc;
}

int Socket::Close()
{
	return close(sockDesc);
}

int Socket::Shutdown()
{
	return shutdown(sockDesc, SHUT_RDWR);
}

void Socket::operator=(const Socket &sock)
{
	cout <<  "SOCK:" << sockDesc << endl;
	sockDesc = sock.sockDesc;
	Error = 0;
}

Socket::Socket(int type, int protocol)
{
  #ifdef WIN32
    if (!initialized) {
      WORD wVersionRequested;
      WSADATA wsaData;

      wVersionRequested = MAKEWORD(2, 0);              // Request WinSock v2.0
      if (WSAStartup(wVersionRequested, &wsaData) != 0) {  // Load WinSock DLL
        //throw SocketException("Unable to load WinSock DLL");
      }
      initialized = true;
    }
  #endif

  // Make a new socket
  if ((sockDesc = socket(AF_INET, type, protocol)) < 0)
  {
    //throw SocketException("Socket creation failed (socket())", true);
  }
  Error = 0;
  enableNagle(true);
}

Socket::Socket(int sockDesc)
{
	this->sockDesc = sockDesc;
	Error = 0;
}

Socket::~Socket()
{
  #ifdef WIN32
    ::closesocket(sockDesc);
  #else
    ::close(sockDesc);
  #endif
  sockDesc = -1;
  Error = 0;
}

string Socket::getLocalAddress()
{
  sockaddr_in addr;
  unsigned int addr_len = sizeof(addr);

  if (getsockname(sockDesc, (sockaddr *) &addr, (socklen_t *) &addr_len) < 0)
  {
	return "";
    //throw SocketException("Fetch of local address failed (getsockname())", true);
  }
  return inet_ntoa(addr.sin_addr);
}

unsigned short Socket::getLocalPort()
{
  sockaddr_in addr;
  unsigned int addr_len = sizeof(addr);

  if (getsockname(sockDesc, (sockaddr *) &addr, (socklen_t *) &addr_len) < 0)
  {
	return 1;
    //throw SocketException("Fetch of local port failed (getsockname())", true);
  }
  return ntohs(addr.sin_port);
}

int Socket::bind(unsigned short localPort)
{
  // Bind the socket to its port
  sockaddr_in localAddr;
  memset(&localAddr, 0, sizeof(localAddr));
  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  localAddr.sin_port = htons(localPort);

  if (::bind(sockDesc, (sockaddr *) &localAddr, sizeof(sockaddr_in)) < 0)
  {
	  return 1;
    //throw SocketException("Set of local port failed (bind())", true);
  }
  return 0;
}

int Socket::setLocalAddressAndPort(const string &localAddress, unsigned short localPort)
{
	// Get the address of the requested host
	sockaddr_in localAddr;
	fillAddr(localAddress, localPort, localAddr);

	int res = ::bind(sockDesc, (struct sockaddr*) (&localAddr), sizeof(sockaddr_in));
	if (res < 0)
	{
		//throw SocketException("Set of local address and port failed (bind())", true);
		return -1;
	}
	return 0;
}

void Socket::cleanUp()
{
  #ifdef WIN32
    if (WSACleanup() != 0) {
      //throw SocketException("WSACleanup() failed");
    }
  #endif
}

unsigned short Socket::resolveService(const string &service,
                                      const string &protocol)
{
  struct servent *serv;        /* Structure containing service information */

  if ((serv = getservbyname(service.c_str(), protocol.c_str())) == NULL)
  {
	  return atoi(service.c_str());  /* Service is port number */
  }
  
    return ntohs(serv->s_port);    /* Found port (network byte order) by name */
}



int Socket::connect(const string &foreignAddress, unsigned short foreignPort)
{
  // Get the address of the requested host
  sockaddr_in destAddr;
  fillAddr(foreignAddress, foreignPort, destAddr);

  // Try to connect to the given port
  if (::connect(sockDesc, (sockaddr *) &destAddr, sizeof(destAddr)) < 0)
  {
    //throw SocketException("Connect failed (connect())", true);
    return 1;
  }
  
  return 0;
}


int Socket::send(const void *buffer, int bufferLen) 
{
	
	if(!buffer || bufferLen < 1)
	{
		return -1;
	}


	//int opts;

	//opts = fcntl(sockDesc, F_GETFL );
	//bool originalBlocking = opts & O_NONBLOCK;
	//setNonBlocking(false);
	int status = ::send(sockDesc, (raw_type *) buffer, bufferLen, MSG_NOSIGNAL);
	//int status = ::send(sockDesc, (raw_type *) buffer, bufferLen, 0);
	
	if (status < 0)
	{
		if(errno == 11) // EDEADLK deadlock, we are going very fast
		{
			while (status == -1)
			{
				status = ::send(sockDesc, (raw_type *) buffer, bufferLen, MSG_NOSIGNAL);
			}
		}
	//cout << errno << strerror(errno) << endl;;
	//cout <<"Socket error:" << "can not send:" << status << endl << flush;
	//return 1;
    ////throw SocketException("Send failed (send())", true);
  }
  
  //setNonBlocking(originalBlocking);
  
  //cout << "sent : " << status << endl;
  return status;
}

int Socket::recv(void *buffer, int bufferLen) 
{
	int rtn;
	//if ((rtn = ::recv(sockDesc, (raw_type *) buffer, bufferLen, MSG_ERRQUEUE)) < 0)
	if ((rtn = ::recv(sockDesc, (raw_type *) buffer, bufferLen, MSG_NOSIGNAL)) < 0)
	{
		if (errno != EWOULDBLOCK)
		{
			perror("  recv() failed");
		}
            
		if(rtn != 11)
		{
			return -1;
			//cout << strerror(rtn) << endl;;
			//cout <<"Socket error:" << "can not recieve:" << rtn << endl << flush;
		}
		////throw SocketException("Received failed (recv())", true);
	}

	return rtn;
}

string Socket::recv() 
{
		/*
	int value = 0;

ioctl(sockDesc, SIOCINQ, &value);
if(value > 0)
{
	cout << "in buffer : " <<  value << endl;
}*/
	int bSize = 4096;
	char data[bSize];
	string recieved;
	//ssize_t bytes_received =  ::recv(sockDesc, (raw_type *) &data, bSize, MSG_PEEK);
	
	//if (bytes_received == -1)
	//{
	//	return "";
	//}
	
	//if ((size_t)bytes_received != buffer_len)
	//{
	//	return 0;
	//}
            
	int rtn;
	if ((rtn = ::recv(sockDesc, (raw_type *) &data, bSize, MSG_WAITALL)) < 0)
	{
		if(errno != 11)
		{
			//cout << strerror(rtn) << endl;;
			//cout <<"Socket error:" << "can not recieve:" << errno << endl << flush;
			//exit(1);
			Error = 1;
		}
		return "";
	}
	else if(rtn == 0)
	{
		return "";
	}
	else
	{
		for ( int i =0; i < rtn ; i++)
		{
			if(data[i] == '\0')
			{
				
				if ( !(i+1== rtn)  && (i != 0) )
				{
					//recieved += '0';
					//recieved += data[i];
				}
				continue;
			}
			recieved += data[i];
		}
	}

	//cout << "recvv : " << recieved << " : " << recieved.size() << " : " << rtn << endl << flush;
	return string(recieved);
	//return recieved;
}

string Socket::getForeignAddress() 
{
  sockaddr_in addr;
  unsigned int addr_len = sizeof(addr);

  if (getpeername(sockDesc, (sockaddr *) &addr,(socklen_t *) &addr_len) < 0)
  {
    //throw SocketException("Fetch of foreign address failed (getpeername())", true);
  }
  return inet_ntoa(addr.sin_addr);
}

unsigned short Socket::getForeignPort()
{
  sockaddr_in addr;
  unsigned int addr_len = sizeof(addr);

  if (getpeername(sockDesc, (sockaddr *) &addr, (socklen_t *) &addr_len) < 0)
  {
    //throw SocketException("Fetch of foreign port failed (getpeername())", true);
  }
  return ntohs(addr.sin_port);
}



int Socket::setNonBlocking(const bool b)
{

  int opts;

  opts = fcntl(sockDesc, F_GETFL );

  if ( opts < 0 )
    {
      return 1;
    }

  if ( b )
    opts = ( opts | O_NONBLOCK );
  else
    opts = ( opts & ~O_NONBLOCK );

  if ( (fcntl( sockDesc, F_SETFL, opts )) == -1)
  {
	  return 1;
  }
  
  return 0;

}

bool Socket::connected()
{
	if(sockDesc == -1)
	{
		return false;
	}
	return true;
}

/*
const Socket& Socket::operator << ( const std::string& s ) const
{
  if ( ! send ( s.c_str(), s.size()+1 ) )
    {
      //throw SocketException ( "Could not write to socket." );
    }

  return *this;

}


const Socket& Socket::operator >> ( std::string& s ) const
{
  if ( ! recv ( (void*)&s, 512 ) )
    {
      //throw SocketException ( "Could not read from socket." );
    }

  return *this;
}
*/

// TCPSocket Code

TCPSocket::TCPSocket() throw(SocketException) : Socket(SOCK_STREAM, IPPROTO_TCP)
{
	setopts();		
}

TCPSocket::TCPSocket(const string &foreignAddress, unsigned short foreignPort)  throw(SocketException) : 
																				Socket(SOCK_STREAM, IPPROTO_TCP)

{
	setopts();
	connect(foreignAddress, foreignPort);
}

TCPSocket::TCPSocket(int newConnSD) : Socket(newConnSD)
{
	setopts();
}


TCPSocket::TCPSocket(const string &localAddress, unsigned short localPort, int queueLen) 
    throw(SocketException) : Socket(SOCK_STREAM, IPPROTO_TCP)
{
	setopts();
	setLocalAddressAndPort(localAddress, localPort);
	listen(queueLen);
}

int TCPSocket::setopts()
{
	return 0;
}

TCPSocket* TCPSocket::accept()
{
  int newConnSD;
  if ((newConnSD = ::accept(sockDesc, NULL, 0)) < 0)
  {
    ////throw SocketException("Accept failed (accept())", true);
    return NULL;
  }

  return new TCPSocket(newConnSD);
}

int TCPSocket::listen(int queueLen)
{
  if (::listen(sockDesc, queueLen) < 0)
  {
    //throw SocketException("Set listening socket failed (listen())", true);
    return 1;
  }
  return 0;
}






MessageTCPSocket::MessageTCPSocket() 
    throw(SocketException) : TCPSocket()
    
{
	bufferSend = false;
	currentMessageLength = 0;
	originalMessageLength = 0;
	//sockDesc = -1;
	//MessageTCPSocket::setopts();
}

MessageTCPSocket::MessageTCPSocket(const string& foreignAddress, unsigned short foreignPort)
    throw(SocketException) : TCPSocket((const string&)foreignAddress, foreignPort)
    
{
	bufferSend = false;
	currentMessageLength = 0;
	originalMessageLength = 0;	
	//sockDesc = -1;
	MessageTCPSocket::setopts();
	//connect(foreignAddress, foreignPort);
}


MessageTCPSocket::MessageTCPSocket(int newConnSD) : TCPSocket(newConnSD)
{
	bufferSend = false;
	currentMessageLength = 0;
	originalMessageLength = 0;	
	//sockDesc = -1;
	MessageTCPSocket::setopts();
}



MessageTCPSocket::MessageTCPSocket(const string &localAddress, unsigned short localPort, int queueLen) 
    throw(SocketException) : TCPSocket(localAddress, localPort, queueLen) 
{
		setopts();
  /*setLocalAddressAndPort(localAddress, localPort);
  listen(queueLen);*/
}



MessageTCPSocket* MessageTCPSocket::accept()
{
	int newConnSD;
	if ((newConnSD = ::accept(sockDesc, NULL, 0)) < 0)
	{
		////throw SocketException("Accept failed (accept())", true);
		return NULL;
	}
	
	MessageTCPSocket::setopts();
	return new MessageTCPSocket(newConnSD);
}


string Socket::numberToStr(unsigned int number)
{
	stringstream SS;
	SS << number;
	return SS.str();
};

int Socket::strToNumber(const string& str)
{
	stringstream SS;
	SS << str;
	int num = 0;
	SS >> num;
	return num;
};



int MessageTCPSocket::setopts()
{	
	int optval = 1;
	int optlen = sizeof(optval);
	if(setsockopt(sockDesc, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0)
	{
		perror("setsockopt(SO_KEEPALIVE)");
		//exit(EXIT_FAILURE);
	}
   
	if(setsockopt(sockDesc, SOL_SOCKET, SO_REUSEADDR, &optval, optlen) < 0)
	{
		perror("setsockopt(SO_REUSEADDR)");
		//exit(EXIT_FAILURE);
	}
   
	if(setsockopt(sockDesc, SOL_TCP, TCP_NODELAY, &optval, optlen) < 0)
	{
		perror("setsockopt(TCP_NODELAY)");
		//exit(EXIT_FAILURE);
	}

	if(setsockopt(sockDesc, SOL_TCP, TCP_QUICKACK, &optval, optlen) < 0)
	{
		perror("setsockopt(TCP_QUICKACK)");
	}
	
	//if(ioctl(sockDesc, FIONBIO, (char *)&one) < 0)
	{
		//perror("ioctl (FIONBIO)");
	}
	return 0;
}


string MessageTCPSocket::recvMessage()
{
	//if(!isReadyToRead(sockDesc, 20))
	//{
	//	return "";
	//}
	
	buffer += TCPSocket::recv();
	//cerr << "BUFFER:" << originalMessageLength << endl;
	if(originalMessageLength == 0)
	{
		if(buffer.size() < 4)
		{
			return "";
		}
		
		originalMessageLength = strToNumber(buffer.substr(0,4));
		//cerr << "BUFFER:" << originalMessageLength << endl;
		//if(originalMessageLength < 0)
		//{
			//FIX ???
		//}
		buffer = buffer.substr(4,buffer.size());
	}
	
	if(buffer.size() > 0)
	{
		if (buffer.size() >= originalMessageLength && originalMessageLength != 0)
		{
			string msg = buffer.substr(0,originalMessageLength);
			buffer = buffer.substr(originalMessageLength, buffer.size() );
			originalMessageLength = 0;
			//cerr << "BUFFER:" << originalMessageLength << endl;
			return msg;
		}
	}
	
	//cerr << "BUFFER:" << originalMessageLength << endl;
	return "";
}

int MessageTCPSocket::sendMessage(const string& msg)
{
	//tcp_push_pending_frames(sk);
	int ret = 0;
	if (msg.size() > 0)
	{
		string size = numberToStr(msg.size());
		//if((int)msg.at(0) > 22)
		{
			//cout << "wrong tyoe ?!?" << endl;
			//exit(1);
		}
		while (size.size() < 4)
		{
			size = '0' + size;
		}
		//if (msg.size() > 30)
		{
			//cout << endl << "Sending : " << msg << ":" <<  msg.size() << endl;
			//exit(1);
		}
		
		 msg = size + msg;
		
		if(bufferSend)
		{
			sendBuffer = sendBuffer + msg;
			ret = -2;
		}
		else
		{
			ret = Socket::send((void*)(msg.data()), msg.size());
		}
	}
	else
	{
		//cerr << " EMPTY MESSAGE " << msg.size() << endl;
	}

	// recursive to send all data or return -1 !
	// FIX
	//if(ret < msg.size())
	if(false)
	{
		if(ret != -1)
		{
			return sendMessage(msg.substr(ret, msg.size()));
		}
	}
	
	return ret;
	//return TCPSocket::send((sendmsg.c_str()), strlen( sendmsg.c_str() ));
}

int MessageTCPSocket::flush()
{
	//should be added before send directly to work (i tried that !)
	int state = 0;
	return setsockopt(sockDesc, IPPROTO_TCP, TCP_CORK, &state, sizeof(state));
}


bool Socket::enableNagle(bool cond)
{
	int state = (int)cond;
	int result = setsockopt(sockDesc, IPPROTO_TCP, TCP_NODELAY, &state, sizeof(state));
	
	if (result < 0)
	{
		cout << "Error setting TCP_NODELAY" << endl;
		cout << strerror(errno) << endl;
		return false;
	}
	return true;
}

bool Socket::isReadyToRead(int _socketHandle, const long &_lWaitTimeMicroseconds)
{
    int iSelectReturn = 0;  // Number of sockets meeting the criteria given to select()
    timeval timeToWait;
    int fd_max = -1;          // Max socket descriptor to limit search plus one.
    fd_set readSetOfSockets;  // Bitset representing the socket we want to read
                              // 32-bit mask representing 0-31 descriptors where each 
                              // bit reflects the socket descriptor based on its bit position.

    timeToWait.tv_sec  = 0;
    timeToWait.tv_usec = _lWaitTimeMicroseconds;

    FD_ZERO(&readSetOfSockets);
    FD_SET(_socketHandle, &readSetOfSockets);

    if(_socketHandle > fd_max)
    {
       fd_max = _socketHandle;
    }

    iSelectReturn = select(fd_max + 1, &readSetOfSockets, (fd_set*) 0, (fd_set*) 0, &timeToWait);

    // iSelectReturn -1: ERROR, 0: no data, >0: Number of descriptors found which pass test given to select()
    if ( iSelectReturn == 0 )  // Not ready to read. No valid descriptors
    {
        return false;
    }
    else if ( iSelectReturn < 0 )  // Handle error
    {
        cerr << "*** Failed with error "   << errno << " ***" << endl;

        close(_socketHandle);
        return false;
    }

    // Got here because iSelectReturn > 0 thus data available on at least one descriptor
    // Is our socket in the return list of readable sockets
    if ( FD_ISSET(_socketHandle, &readSetOfSockets) )
    {
        return true;
    }
    else
    {
        return false;
    }

    return false;
}


int MessageTCPSocket::enableBufferSend(bool cond)
{
	if(!cond)
	{	
		int ret = 0;
		if(sendBuffer.size())
		{
			ret = Socket::send((void*)(sendBuffer.data()), sendBuffer.size());
		}
		//cout << "Sending : " << buffer << endl;
		sendBuffer = "";
		bufferSend = false;
		return ret;
	}
	bufferSend = true;
	return 0;
}
