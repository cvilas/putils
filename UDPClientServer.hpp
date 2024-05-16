//==============================================================================
// UDPClientServer.hpp - UDP Client/server based on BSD socket API 
//
// Author        : Vilas Kumar Chitrakaran
// Version       : 2.0 (Apr 2005)
// Compatibility : POSIX, GCC
//==============================================================================

#ifndef _UDPCLIENTSERVER_HPP_INCLUDED
#define _UDPCLIENTSERVER_HPP_INCLUDED

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "StatusReport.hpp"

//==============================================================================
// class UDPServer
//------------------------------------------------------------------------------
// \brief
// This is the server part of the UDPServer/UDPClient pair.
//
// UDPServer/UDPClient uses the User Datagram Protocol (UDP, IETF RFC768) for 
// fast, 'unreliable' data transfer between two devices over the ethernet. The 
// protocol is unreliable because there is no guarantee that data packets will 
// reach their destination, or that they will reach the destination in the right 
// sequence. UDP prioritizes speed over reliability. Use (the much slower) 
// TCPServer/TCPClient if reliability and data integrity is more important in 
// your application.   
//
// <b>Example Program:</b>
// \include UDPClientServer.t.cpp
//==============================================================================

class UDPServer
{
 public:
  UDPServer();
   // The default constructor. Does nothing.
   
  UDPServer(int port, int maxMsgSize, int bdp=0);
   // Initializes the sever. 
   //  port        The port on which the server will listen for data packets
   //              from clients.
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
   //              the round-trip time. Set this value to 0 to use system defaults.

  virtual ~UDPServer();
   // The destructor frees resources.

  int init(int port, int maxMsgSize, int bdp=0);
   // Initialize the server.
   //  port    The port number used by the server in listening
   //          for clients.
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
   //              the round-trip time. Set this value to 0 to use system defaults.
   //  return  0 on success, -1 on failure.

  void doMessageCycle();
   // This function never returns, waiting for any data packets in an
   // infinite loop. When a data packet is received, it copies the 
   // data into the message buffer and calls the user implemented function 
   // receiveAndReply().

  int getStatusCode() const;
   //  return  0 on no error, else latest status code. See errno.h for codes.

  const char *getStatusMessage() const;
   //  return  Latest error status report

 protected:
  virtual const char *receiveAndReply(const char *inMsgBuf, int inMsgLen, int *outMsgLen); 
   // Re-implement this function in your derived class. This function is 
   // called by doMessageCycle() everytime it receives a data packet from a client. 
   // <br><hr>
   // <ul>
   // <li>If return value is set to NULL, the server will not attempt 
   // to reply back to the client.
   // <li>If the data packet is too long to fit into the receive buffer (whose length is 
   // set in the constructor), the excess message from client is discarded.
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
// class UDPClient
//------------------------------------------------------------------------------
// \brief
// This is the client part of the UDPServer/UDPClient pair.
//
// UDPServer/UDPClient uses the User Datagram Protocol (UDP, IETF RFC768) for 
// fast, 'unreliable' data transfer between two devices over the ethernet. The 
// protocol is unreliable because there is no guarantee that data packets will 
// reach their destination, or that they will reach the destination in the right 
// sequence. UDP prioritizes speed over reliability. Use (the much slower) 
// TCPServer/TCPClient if reliability and data integrity is more important in 
// your application.   
//
// <b>Example Program:</b>
// See example for UDPServer
//==============================================================================

class UDPClient
{
 public:
  UDPClient();
   // The default constructor. Does nothing.

  UDPClient(const char *serverIp, int port, struct timeval &timeout, int bdp=0);
   // This constructor initializes parameters for a connection 
   // to remote server.
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
   //            time. Set this value to 0 to use system defaults.

  ~UDPClient();
   // The destructor. Cleans up.

  int init(const char *serverIp, int port, struct timeval &timeout, int bdp=0);
   // Establish connection with a remote server.
   //  serverIp IP name of the remote server.
   //  port     Port address on which the remote server is listening
   //           for client connections.
   //  timeout  The sendAndReceive() function sends messages and 
   //           waits for replies from the server. This parameter 
   //           sets the timeout period in waiting for a reply. If a 
   //           reply is not received within this timeout period, 
   //           sendAndReceive() will exit with error.
   //  bdp      This is an advanced option. It allows the user to suggest
   //           the bandwidth-delay product in kilo bytes so that socket 
   //           buffers of optimal sizes can be created. Suppose you are 
   //           going to connect to a machine whose round-trip time (delay 
   //           between sending a packet and receiving acknowledgement) is 
   //           50ms, and the link bandwidth is 100 Mbits per sec. Then your 
   //           BDP is 100e6 * 50e-3 / 8 = 625 kilo bytes. You can use the 
   //           'ping' utility to get an approx. measure for the round-trip 
   //           time. Set this value to 0 to use system defaults.
   //  return   0 on success, -1 on error.

  int getStatusCode() const;
   //  return  Latest status code.
   
  const char *getStatusMessage() const;
   //  return  Latest error status report.

  int sendAndReceive(char *outMsgBuf, int outMsgLen, char *inMsgBuf,
                     int inBufLen, int *inMsgLen);
   // Send a message to the server, and receive a reply. 
   // <br><hr>
   // <ul>
   // <li>If \a inMsgBuf is set to NULL, the function will return immediately
   //     after sending a message. It won't wait for any reply.
   // <li>If \a inMsgBuf is set, this function will block until a reply is received 
   //     or until timeout (set in init()). 
   // <li>If the data packet is too long to fit into the receive buffer  
   // \a inMsgBuf, the excess message from client is discarded.
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
   // bandwidth-delay product
  
  bool d_init;
   // true if client initialized
 
  struct timeval d_recvTimeout;
   // receive timout
   
  StatusReport d_status;
   // Error reports 
};


#endif // _UDPCLIENTSERVER_HPP_INCLUDED
