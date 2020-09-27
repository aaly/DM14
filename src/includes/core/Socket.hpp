#ifndef __SOCKETHPP
#define __SOCKETHPP

#include <iostream>
#include <sstream>
#include <string> 

#ifdef WIN32
	#include <winsock.h>         // For socket(), connect(), send(), and recv()
	typedef int socklen_t;
	typedef char raw_type;       // Type used for raw data on this platform
	static bool initialized = false;
#else
	#include <sys/types.h>       // For data types
	#include <sys/socket.h>      // For socket(), connect(), send(), and recv()
	#include <netdb.h>           // For gethostbyname()
	#include <arpa/inet.h>       // For inet_addr()
	#include <unistd.h>          // For close()
	#include <netinet/in.h>      // For sockaddr_in
	#include <netinet/tcp.h>		// for TCP_NODELAY
	typedef void raw_type;       // Type used for raw data on this platform
	#include <exception>         // For exception class
	#include <string.h>
	#include <math.h>
	#include <stdlib.h>
	#include <fcntl.h>
	#include <sys/select.h>
	#include <sys/time.h>
	#include <stdio.h>
	#include <netinet/tcp.h>
	#include <linux/sockios.h>
	#include <sys/ioctl.h>
	#include <errno.h>             // For errno
	#include <signal.h>
#endif


/* We want the .gnu.warning.SYMBOL section to be unallocated.  */
#define __make_section_unallocated(section_string)    \
  __asm__ (".section " section_string "\n\t.previous");

/* When a reference to SYMBOL is encountered, the linker will emit a
   warning message MSG.  */
#define silent_warning(symbol) \
  __make_section_unallocated (".gnu.warning." #symbol) 

silent_warning(gethostbyname)
silent_warning(getservbyname)


using namespace std;

/**
 *   Signals a problem with the execution of a socket call.
 */
class SocketException : public exception {
public:
  /**
   *   Construct a SocketException with a explanatory message.
   *   @param message explanatory message
   *   @param incSysMsg true if system message (from strerror(errno))
   *   should be postfixed to the user provided message
   */
  SocketException(const string &message, bool inclSysMsg = false) throw();

  /**
   *   Provided just to guarantee that no exceptions are thrown.
   */
  ~SocketException() throw();

  /**
   *   Get the exception message
   *   @return exception message
   */
  const char *what() const throw();

private:
  string userMessage;  // Exception message
};



void signal_callback_handler(int signum);





/**
 *   Base class representing basic communication endpoint
 */
class Socket {
	public:
		Socket();
		Socket(int type, int protocol);
		Socket(int sockDesc);
		//Socket(const Socket &sock);
		void operator=(const Socket &sock);
  
		/**
		*   Close and deallocate this socket
		*/
		~Socket();
		
		/**
		*   Get the local address
		*   @return local address of socket
		*   @exception SocketException thrown if fetch fails
		*/
		string getLocalAddress();

  /**
   *   Get the local port
   *   @return local port of socket
   *   @exception SocketException thrown if fetch fails
   */
  unsigned short getLocalPort();

  /**
   *   Set the local port to the specified port and the local address
   *   to any interface
   *   @param localPort local port
   *   @exception SocketException thrown if setting local port fails
   */
  int bind(unsigned short localPort);

  /**
   *   Set the local port to the specified port and the local address
   *   to the specified address.  If you omit the port, a random port 
   *   will be selected.
   *   @param localAddress local address
   *   @param localPort local port
   *   @exception SocketException thrown if setting local port or address fails
   */
  //int setLocalAddressAndPort(const string &localAddress, unsigned short localPort = 0);
  int setLocalAddressAndPort(const string &localAddress, const string& localPort = "0");

  /**
   *   If WinSock, unload the WinSock DLLs; otherwise do nothing.  We ignore
   *   this in our sample client code but include it in the library for
   *   completeness.  If you are running on Windows and you are concerned
   *   about DLL resource consumption, call this after you are done with all
   *   Socket instances.  If you execute this on Windows while some instance of
   *   Socket exists, you are toast.  For portability of client code, this is 
   *   an empty function on non-Windows platforms so you can always include it.
   *   @param buffer buffer to receive the data
   *   @param bufferLen maximum number of bytes to read into buffer
   *   @return number of bytes read, 0 for EOF, and -1 for error
   *   @exception SocketException thrown WinSock clean up fails
   */
  static void cleanUp();

  /**
   *   Resolve the specified service for the specified protocol to the
   *   corresponding port number in host byte order
   *   @param service service to resolve (e.g., "http")
   *   @param protocol protocol of service to resolve.  Default is "tcp".
   */
  static unsigned short resolveService(const string &service,
                                       const string &protocol = "tcp");
                                       

  /**
   *   Establish a socket connection with the given foreign
   *   address and port
   *   @param foreignAddress foreign address (IP address or name)
   *   @param foreignPort foreign port
   *   @exception SocketException thrown if unable to establish connection
   */
  //int connect(const string &foreignAddress, unsigned short foreignPort);
  int connect(const string &foreignAddress, const string& foreignPort);

  /**
   *   Write the given buffer to this socket.  Call connect() before
   *   calling send()
   *   @param buffer buffer to be written
   *   @param bufferLen number of bytes from buffer to be written
   *   @exception SocketException thrown if unable to send data
   */
  int send(const void *buffer, int bufferLen);

  /**
   *   Read into the given buffer up to bufferLen bytes data from this
   *   socket.  Call connect() before calling recv()
   *   @param buffer buffer to receive the data
   *   @param bufferLen maximum number of bytes to read into buffer
   *   @return number of bytes read, 0 for EOF, and -1 for error
   *   @exception SocketException thrown if unable to receive data
   */
  int recv(void *buffer, int bufferLen);
  string recv();

  /**
   *   Get the foreign address.  Call connect() before calling recv()
   *   @return foreign address
   *   @exception SocketException thrown if unable to fetch foreign address
   */
  string getForeignAddress();

  /**
   *   Get the foreign port.  Call connect() before calling recv()
   *   @return foreign port
   *   @exception SocketException thrown if unable to fetch foreign port
   */
  unsigned short getForeignPort();
  
  int setNonBlocking ( const bool b );
  
 
  
  const Socket& operator << ( const std::string& ) const;
  const Socket& operator >> ( std::string& ) const;
  
	static string numberToStr(unsigned int number);
	static  int strToNumber(const string& str);
	bool connected();
	bool isReadyToRead(int _socketHandle, const long &_lWaitTimeMicroseconds);
	bool enableNagle(bool);
	
	int Error;
	int getSocketDescriptor();
	int Close();
	int Shutdown();
public:
  // Prevent the user from trying to use value semantics on this object
  //Socket(const Socket &sock);
  //void operator=(const Socket &sock);

protected:
  int sockDesc;              // Socket descriptor
};









/**
 *   UDP socket for communication with other UDP sockets
 */
class UDPSocket : public Socket {
public:
  /**
   *   Construct a UDP socket with no connection
   *   @exception SocketException thrown if unable to create UDP socket
   */
  UDPSocket() throw(SocketException);

  /**
   *   Construct a UDP socket with a connection to the given foreign address
   *   and port
   *   @param foreignAddress foreign address (IP address or name)
   *   @param foreignPort foreign port
   *   @exception SocketException thrown if unable to create UDP socket
   */
  UDPSocket(const string &foreignAddress, const string& foreignPort) 
      throw(SocketException);

  /**
   *   Construct a UDP socket for use with a server, accepting connections
   *   on the specified port on the interface specified by the given address
   *   @param localAddress local interface (address) of server socket
   *   @param localPort local port of server socket
   *   @param queueLen maximum queue length for outstanding 
   *                   connection requests (default 5)
   *   @exception SocketException thrown if unable to create UDP server socket
   */
  UDPSocket(const string &localAddress, const string& localPort,
      int queueLen = 5) throw(SocketException);

  /**
   *   Blocks until a new connection is established on this socket or error
   *   @return new connection socket
   *   @exception SocketException thrown if attempt to accept a new connection fails
   */
  UDPSocket* accept();

	int listen(int queueLen);
	
	int setopts();
	
private:
  UDPSocket(int newConnSD);
};







/**
 *   TCP socket for communication with other TCP sockets
 */
class TCPSocket : public Socket {
public:
  /**
   *   Construct a TCP socket with no connection
   *   @exception SocketException thrown if unable to create TCP socket
   */
  TCPSocket() throw(SocketException);

  /**
   *   Construct a TCP socket with a connection to the given foreign address
   *   and port
   *   @param foreignAddress foreign address (IP address or name)
   *   @param foreignPort foreign port
   *   @exception SocketException thrown if unable to create TCP socket
   */
  TCPSocket(const string &foreignAddress, const string& foreignPort)
      throw(SocketException);

  /**
   *   Construct a TCP socket for use with a server, accepting connections
   *   on the specified port on the interface specified by the given address
   *   @param localAddress local interface (address) of server socket
   *   @param localPort local port of server socket
   *   @param queueLen maximum queue length for outstanding 
   *                   connection requests (default 5)
   *   @exception SocketException thrown if unable to create TCP server socket
   */
  TCPSocket(const string &localAddress, const string& localPort,
      int queueLen) throw(SocketException);

  /**
   *   Blocks until a new connection is established on this socket or error
   *   @return new connection socket
   *   @exception SocketException thrown if attempt to accept a new connection fails
   */
   
  TCPSocket(int newConnSD);
  TCPSocket* accept();

	int listen(int queueLen);
	
	int setopts();
	
private:

  
  
};


class MessageTCPSocket : public TCPSocket
{
	public:
	
		MessageTCPSocket() throw(SocketException);;
  /**
   *   Construct a TCP socket with a connection to the given foreign address
   *   and port
   *   @param foreignAddress foreign address (IP address or name)
   *   @param foreignPort foreign port
   *   @exception SocketException thrown if unable to create TCP socket
   */
  MessageTCPSocket(const string &foreignAddress, const string& foreignPort) 
      throw(SocketException);

  /**
   *   Construct a TCP socket for use with a server, accepting connections
   *   on the specified port on the interface specified by the given address
   *   @param localAddress local interface (address) of server socket
   *   @param localPort local port of server socket
   *   @param queueLen maximum queue length for outstanding 
   *                   connection requests (default 5)
   *   @exception SocketException thrown if unable to create TCP server socket
   */
  MessageTCPSocket(const string &localAddress, const string& localPort,
      int queueLen = 5) throw(SocketException);
      
      MessageTCPSocket(int newConnSD);
      
		string recvMessage();
		int sendMessage(const string&);
		MessageTCPSocket *accept();
		int setopts();
		int flush();
		int enableBufferSend(bool);
	private:
		int		currentMessageLength;
		unsigned int		originalMessageLength;
		string	buffer;
		string	sendBuffer;
		bool	bufferSend;
		
};


#endif //__SOCKETPP
