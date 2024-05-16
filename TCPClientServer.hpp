//==============================================================================
// TCPClientServer.hpp - Client/server over TCP/IP based on BSD socket API 
//
// Author        : Vilas Kumar Chitrakaran
// Version       : 2.0 (Apr 2005)
// Compatibility : POSIX, GCC
//==============================================================================

#ifndef _TCPCLIENTSERVER_HPP_INCLUDED
#define _TCPCLIENTSERVER_HPP_INCLUDED

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include "StatusReport.hpp"

//==============================================================================
// class TCPServer
//------------------------------------------------------------------------------
// \brief
// This is the server part of the TCPServer/TCPClient pair.
//
// This implementation is the base class that provides server functionality
// in a client-server relationship over a TCP/IP network. The user must 
// reimplement atleast the receiveAndReply() function in a derived class 
// in order to have a functional server. This server can listen for upto 
// 20 waiting client connections. This implementation does not do endian 
// conversions to the data being sent/received over the network. Hence you 
// will have jumbled data when communicating between little endian and 
// big endian devices and vice-versa (no problems if both ends use same byte 
// order for data). Note also that this implementation provides a signal 
// handler to ignore SIGPIPE. This will allow server to keep running even after 
// send/recv data on illegal socket resulting from an unexpected client 
// termination.
//
// Use TCPClient/TCPServer when you want to reliably transfer data at slow
// speeds. Use UDPClient/UDPServer when your primary requirement is speed.
//
// <b>Example Program:</b>
// \include TCPClientServer.t.cpp
//==============================================================================

class TCPServer
{
 public:
  TCPServer();
   // The default constructor. Does nothing.

  TCPServer(int port, int maxMsgSize=1024, int bdp=0);
   // Initializes the sever. 
   //  port        The port on which the server will wait for clients.
   //  maxMsgSize  Maximum size (bytes) of the receive buffer. Client
   //              messages larger than this size are discarded.
   //  bdp         This is an advanced option. It allows the user to suggest
   //              the bandwidth-delay product in kilo bytes so that socket 
   //              buffers of optimal sizes can be created. Suppose you are 
   //              going to receive connections from a machine whose round-trip 
   //              time (delay between sending a packet and receiving 
   //              acknowledgement) is 50ms, and the link bandwidth is 100 Mbits
   //              per sec. Then your BDP is 100e6 * 50e-3 / 8 = 625 kilo bytes.
   //              You can use the 'ping' utility to get an approx. measure for
   //              the round-trip time. Set this to 0 to use system defaults.
   
  virtual ~TCPServer();
   // The destructor frees resources.
  
  int init(int port, int maxMsgSize, int bdp=0);
   // Initialize the server.
   //  port    The port number used by the server in listening
   //          for clients.
   //  maxMsgSize  Maximum size (bytes) of the receive buffer. Client
   //              messages larger than this size are discarded.
   //  bdp     Estimated BDP. See constructor for details.
   //  return  0 on success, -1 on failure.

  void doMessageCycle();
   // This function never returns, unless server initialization failed. It 
   // constantly checks for any waiting clients. When connected to a client 
   // it copies the message from the client into the message buffer and calls 
   // receiveAndReply(). Upon return from user implemented receiveAndReply() 
   // this function will reply back to the client if required ( see receiveAndReply() ). 

  int getStatusCode() const;
   //  return  0 on no error, else latest status code. See errno.h for codes.

  const char *getStatusMessage() const;
   //  return  Latest error status report
  
  int enableIgnoreSigPipe();
   // Call this function to ignore SIG_PIPE, and hence save server from terminating
   // due to client termination
   //  return  0 if no error, else -1
   
  int disableIgnoreSigPipe();
   // Call this function to disable SIG_PIPE handling. The server will terminate 
   // if client terminates
   //  return  0 if no error, else -1

 protected:  
  virtual const char *receiveAndReply(const char *inMsgBuf, int inMsgLen, int *outMsgLen); 
   // Re-implement this function in your derived class. This function is 
   // called by doMessageCycle() everytime it receives a message from a client. 
   // <br><hr>
   // <ul>
   // <li>If return value is set to NULL, the server will not attempt 
   // to reply back to the client.
   // <li>The message from client is discarded if the receive buffer size (set 
   // in the constructor) is not large enough.
   // </ul>
   //<hr><br> 
   //  inMsgBuf    Pointer to buffer containing message from client.
   //  inMsgLen    Length of the message (bytes) in the above buffer.
   //  outMsgLen   The length (bytes) of the reply buffer.
   //  return      NULL, or a pointer to reply buffer provided by you containing 
   //              reply message for the client.

 private:
  void setError(int code, const char *functionName);
   // Set an error report
   //  code          errno error code
   //  functionName  The unsuccessful function call
   
  int d_fd;
   // Socket file descriptor
  
  char *d_rcvBuf;
   // The receive buffer
  
  int d_rcvBufSize;
   // Size of above buffer

  bool d_init;
   // true if server initialized

  StatusReport d_status;
   // Status reports 
};


//==============================================================================
// class TCPClient
//------------------------------------------------------------------------------
// \brief
// This is the client part of the TCPServer/TCPClient pair.
// 
// An object of this class can establish connection with an object of class
// (or derived from) TCPServer over a TCP/IP network. This implementation 
// does not do endian conversions to the data being sent/received over the 
// network. Hence you will have jumbled data when communicating between little 
// endian and big endian devices and vice-versa (no problems if both ends use 
// same byte order for data). Note that this implementation provides a signal 
// handler to ignore SIGPIPE. This will allow client to keep running even after 
// send/recv data on illegal socket resulting from an unexpected server 
// termination.
// 
// Use TCPClient/TCPServer when you want to reliably transfer data at slow
// speeds. Use UDPClient/UDPServer when your primary requirement is speed.
//
// <b>Example Program:</b>
// See example for TCPServer
//==============================================================================

class TCPClient
{
 public:
  TCPClient();
   // The default constructor. Does nothing.
   
  TCPClient(const char *serverIp, int port, struct timeval &timeout, int bdp=0);
   // This constructor initializes parameters for a connection 
   // to remote server BUT doesn't connect until sendAndReceive() is
   // called.
   //  serverIp  IP name of the remote server.
   //  port      Port address on which the remote server is listening
   //            for client connections.
   //  timeout   The sendAndReceive() function sends messages and 
   //            waits for replies from the server. This parameter 
   //            sets the timeout period in waiting for a reply. If a 
   //            reply is not received within this timeout period, 
   //            sendAndReceive() will exit with error.
   //  bdp       This is an advanced option. It allows the user to suggest
   //            the bandwidth-delay product in kilo bytes so that socket 
   //            buffers of optimal sizes can be created. Suppose you are 
   //            going to connect to a machine whose round-trip time (delay 
   //            between sending a packet and receiving acknowledgement) is 
   //            50ms, and the link bandwidth is 100 Mbits per sec. Then your 
   //            BDP is 100e6 * 50e-3 / 8 = 625 kilo bytes. You can use the 
   //            'ping' utility to get an approx. measure for the round-trip 
   //            time. Set this to 0 to use system defaults.
   
  ~TCPClient();
   // The destructor. Cleans up.
   
  int init(const char *serverIp, int port, struct timeval &timeout, int bdp=0);
   // Establish connection with a remote server.
   //  serverIp  IP name of the remote server.
   //  port      Port address on which the remote server is listening
   //            for client connections.
   //  timeout   The sendAndReceive() function sends messages and 
   //            waits for replies from the server. This parameter 
   //            sets the timeout period in waiting for a reply. If a 
   //            reply is not received within this timeout period, 
   //            sendAndReceive() will exit with error.
   //  bdp       This is an advanced option. It allows the user to suggest
   //            the bandwidth-delay product in kilo bytes so that socket 
   //            buffers of optimal sizes can be created. Suppose you are 
   //            going to connect to a machine whose round-trip time (delay 
   //            between sending a packet and receiving acknowledgement) is 
   //            50ms, and the link bandwidth is 100 Mbits per sec. Then your 
   //            BDP is 100e6 * 50e-3 / 8 = 625 kilo bytes. You can use the 
   //            'ping' utility to get an approx. measure for the round-trip 
   //            time. Set this to 0 to use system defaults.
   //  return    0 on success, -1 on error.

  int sendAndReceive(char *outMsgBuf, int outMsgLen, char *inMsgBuf,
                     int inBufLen, int *inMsgLen);
   // Send a message to the server, and receive a reply.
   // <br><hr>
   // <ul>
   // <li>This function will normally block waiting for reply from server 
   // unless you set \a inMsgBuf to NULL.
   // <li>The message from server is discarded if the receive buffer \a inMsgBuf 
   // is not large enough.
   // </ul>
   //<hr><br> 
   //  outMsgBuf  Pointer to buffer containing your message to server.
   //  outMsgLen  The length of your message above.
   //  inMsgBuf   A pointer to buffer provided by you to store the reply message
   //             from the server. Set this to NULL if you aren't interested in
   //             reply from server.
   //  inBufLen   The size (bytes) of the above buffer.
   //  inMsgLen   The actual length (bytes) of message received from the server.
   //  return     0 on success, -1 on error. Call getStatus....() for the
   //             error.

  int getStatusCode() const;
   //  return  Latest status code.
   
  const char *getStatusMessage() const;
   //  return  Latest error status report.

  int enableIgnoreSigPipe();
   // Call this function to ignore SIG_PIPE, and hence save client from terminating
   // due to server termination
   //  return  0 if no error, else -1
   
  int disableIgnoreSigPipe();
   // Call this function to disable SIG_PIPE handling. The client will terminate 
   // if server terminates
   //  return  0 if no error, else -1

 private:
  void setError(int code, const char *functionName);
   // Set a error report
   //  code          errno error code
   //  functionName  The unsuccessful function call
 
  struct sockaddr_in d_server;
   // server to connect to.
  
  char *d_serverName;
   // server name
  
  int d_serverPort;
   // server port
   
  int d_fd;
   // Socket file descriptor
  
  int d_bdp;
   // estimated bandwidth delay product.
  
  struct timeval d_recvTimeout;
   // receive timeout 
    
  bool d_init;
   // true if client initialized
 
  StatusReport d_status;
   // Error reports 
 
 //======== END OF INTERFACE ========
};



#endif // _TCPCLIENTSERVER_HPP_INCLUDED
