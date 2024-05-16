//==============================================================================
// TCPClientServer.cpp - Client/server over TCP/IP based on BSD socket API
//
// Author        : Vilas Kumar Chitrakaran
// Version       : 2.0 (Apr 2005)
// Compatibility : POSIX, GCC
//==============================================================================

#include "TCPClientServer.hpp"
#include <cstring>

//==============================================================================
// PROGRAMMER NOTE: 
// Data transfer method: Every transaction is two packets. The first packet is
// sizeof(int) and its value indicates the size of second packet which is the
// data we want to transfer. 
//==============================================================================


//#define DEBUG

#ifdef DEBUG
#include <iostream>
using namespace std;
#endif

//==============================================================================
// TCPServer::TCPServer
//==============================================================================
TCPServer::TCPServer()
{
 d_init = false;
 d_fd = -1;
 d_rcvBuf = NULL;
 d_rcvBufSize = 0;
 setError(0, "TCPServer");
}

TCPServer::TCPServer(int port, int maxMsgSize, int bdp)
{
 d_init = false;
 d_fd = -1;
 d_rcvBuf = NULL;
 d_rcvBufSize = 0;
 
 // initialize a socket
 if( init(port, maxMsgSize, bdp) == -1)
  return;
 
 setError(0, "TCPServer");
}


//==============================================================================
// TCPServer::~TCPServer
//==============================================================================
TCPServer::~TCPServer()
{
 if(d_fd)
 {
  close(d_fd);
  d_fd = -1;
 }

 if( d_rcvBuf )
 {
  free(d_rcvBuf);
  d_rcvBuf = NULL;
 }
 d_init = false;
}


//==============================================================================
// TCPServer::doMessageCycle
// * TODO * combine msg length and msg into a single send() operation
//==============================================================================
void TCPServer::doMessageCycle()
{
 struct sockaddr_in clntAddr;
 int clntAddrLen;
 fd_set readFds, master; // file descriptor lists
 int newFd;
 int fdMax; // max file desc. number
 char *clntIp; // ip address of a new client
 char info[80]; // buf for error messages
 
 
 // add listener to master set
 FD_ZERO(&master);
 FD_SET(d_fd, &master);
 fdMax = d_fd;
 clntAddrLen = sizeof( struct sockaddr_in );

 if(!d_init)
 {
  d_status.setReport(-1, "sendAndReceive: server not initialized");
  return;
 }

#ifdef DEBUG
 cerr << "DEBUG [doMessageCycle]: entering loop" << endl;
#endif

 // the loop starts here
 for(;;)
 {
  readFds = master; // make a copy
  if( select(fdMax+1, &readFds, NULL, NULL, NULL) == -1 )
  {
   setError(errno, "doMessageCycle(select)");
   break;
  } // end if select
  
  // check for activity
  for(int i = 0; i <= fdMax; i++)
  {
   if( FD_ISSET(i, &readFds) ) // got activity
   {
    if( i == d_fd) // activity on server socket. must be conn. req.
    {
     // accept connection
     if( (newFd = accept(d_fd, (struct sockaddr *)&clntAddr, 
         (socklen_t *)&clntAddrLen)) == -1)
      setError(errno, "doMessageCycle(accept)");
     else
     {
      FD_SET(newFd, &master); // add to master list
      if(newFd > fdMax) fdMax = newFd; // keep track of maximum
      clntIp = (char *)inet_ntoa(clntAddr.sin_addr);
      snprintf(info, 80, "accept %s (fd %d)", clntIp, newFd);
      d_status.setReport(0,info);
     }
    } // end if i = d_fd
    else // i != d_fd => client activity. handle client data.
    {
      // ***** Client message processing *****

     int msgSize; // client first sends size of message
     int nbytes;
     if( (nbytes = recv(i, &msgSize, sizeof(int), MSG_WAITALL)) 
          < (int)sizeof(int))
     {

#ifdef DEBUG
 cerr << "DEBUG [doMessageCycle]: disconnect or read error" << endl;
#endif

      close(i);
      FD_CLR(i, &master);
      snprintf(info, 80, "client disconnected (fd %d)", i);
      continue;
     } // end if nbytes <= 0
     else // nbytes > 0
     {

#ifdef DEBUG
 cerr << endl << "DEBUG [doMessageCycle]: got client header" << endl;
#endif

      // msgSize has size of incoming message.
      // Compare with our buffer and make sure we can accomodate
      // the incoming data
      if( msgSize > d_rcvBufSize)
      {

#ifdef DEBUG
 cerr << "DEBUG [doMessageCycle]: buffer not large enough. (fd " << i << ")" << endl;
#endif
       // read and discard data
       int readTillNow = 0;
       int numReadAttempts = 0;
       while( (readTillNow < msgSize) && (numReadAttempts < 3) )
       {
        int readNow;
        numReadAttempts++;
        readNow = recv(i, NULL, msgSize - readTillNow, MSG_WAITALL);
        if(readNow == -1)
         break;
        readTillNow += readNow;
       } // end while

       close(i);
       FD_CLR(i, &master);
       d_status.setReport(-1,"doMessageCycle: buffer not large enough.");
       continue; 
      } // end if msgSize > d_rcvBufSize
     
#ifdef DEBUG
 cerr << "DEBUG [doMessageCycle]: buffer for incoming message ok" << endl;
#endif

      // read data until done or try 3 times
      int readTillNow = 0;
      int numReadAttempts = 0;
      while( (readTillNow < msgSize) && (numReadAttempts < 3) )
      {
       int readNow;
       numReadAttempts++;
       readNow = recv(i, &(d_rcvBuf[readTillNow]), msgSize - readTillNow, 
                      MSG_WAITALL);
       if(readNow == -1)
        break;
       readTillNow += readNow;
      } // end while

      // check for read failure
      if( readTillNow < msgSize )
      {
       close(i);
       FD_CLR(i, &master);
       setError(EIO, "doMessageCycle(recv)");
#ifdef DEBUG
 cerr << "DEBUG [doMessageCycle]: *ERROR* reading client data " << readTillNow << "/" << msgSize << endl;
#endif
       continue;
      }

#ifdef DEBUG
 cerr << "DEBUG [doMessageCycle]: done reading client data" << endl;
#endif

      // send client data to user implemented function
      const char *outMsgBuf;
      int outMsgLen;
      outMsgBuf = receiveAndReply(d_rcvBuf, msgSize, &outMsgLen);
      
#ifdef DEBUG
 cerr << "DEBUG [doMessageCycle]: user function processed data" << endl;
#endif

      // reply to client
      if(outMsgBuf != NULL)
      {

#ifdef DEBUG
 cerr << "DEBUG [doMessageCycle]: will reply to client" << endl;
#endif

       // write header to client - length of outgoing data
       if( send(i, &outMsgLen, sizeof(int), 0) < (int)sizeof(int) )
       {
        close(i);
        FD_CLR(i, &master);
        setError(EIO, "doMessageCycle(send)");
        continue;
       }
      
#ifdef DEBUG
 cerr << "DEBUG [doMessageCycle]: header sent to client" << endl;
#endif

       // write data until done or try 3 times
       int wroteTillNow = 0;
       int numWriteAttempts = 0;
       while( (wroteTillNow < outMsgLen) && (numWriteAttempts < 3) )
       {
        int wroteNow;
        numWriteAttempts++;
        wroteNow = send(i, &(outMsgBuf[wroteTillNow]), 
                         outMsgLen - wroteTillNow, 0);

        if(wroteNow == -1)
         break;

        wroteTillNow += wroteNow;
       } // end while
       
       // check for write failure
       if( wroteTillNow < outMsgLen )
       {
        close(i);
        FD_CLR(i, &master);
        setError(EIO, "doMessageCycle(send)");
        continue;
       }
#ifdef DEBUG
 cerr << "DEBUG [doMessageCycle]: data sent to client" << endl;
#endif
      // ***** \Client message processing ****
      } // end client reply
     } // end else nbytes > 0
    } // end else i != d_fd
   } // end if FD_ISSET
  } // end for i = 0 to fdMax
 } // end for (main)
}


//==============================================================================
// TCPServer::receiveAndReply
//==============================================================================
const char *TCPServer::receiveAndReply(const char *inMsgBuf, int inMsgLen, int *outMsgLen)
{
 inMsgBuf=inMsgBuf;
 inMsgLen=inMsgLen;
 *outMsgLen = 0;
 return NULL;
}


//==============================================================================
// TCPServer::init
//==============================================================================
int TCPServer::init(int port, int bufSize, int bdp)
{
 struct sockaddr_in name;
 int sockBufSize = bdp * 1024;
 
 d_init = false;
 if(d_fd)
 {
  close(d_fd);
  d_fd = -1;
 }

 // Create an endpoint for communication
 if( (d_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
 {
  setError(errno, "init(socket)");
  return -1;
 }
 
 // Allow reuse of port
 int yes = 1;
 if( setsockopt(d_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
 {
  setError(errno, "init(setsockopt-SO_REUSEADDR)");
  close(d_fd);
  d_fd = -1;
  return -1;
 }
 
 // Do not delay sending data packets
 if( setsockopt(d_fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(int)) == -1)
 {
  setError(errno, "init(setsockopt-TCP_NODELAY)");
  close(d_fd);
  d_fd = -1;
  return -1;
 }

 // set suggested optimal socket buffer sizes.
 if( sockBufSize != 0 ) {
  if( setsockopt(d_fd, SOL_SOCKET, SO_SNDBUF, (char *)&sockBufSize, sizeof(int)) == -1)
  {
   setError(errno, "init(setsockopt-SO_SNDBUF)");
   close(d_fd);
   d_fd = -1;
   return -1;
  }

  if( setsockopt(d_fd, SOL_SOCKET, SO_RCVBUF, (char *)&sockBufSize, sizeof(int)) == -1)
  {
   setError(errno, "init(setsockopt-SO_RCVBUF)");
   close(d_fd);
   d_fd = -1;
   return -1;
  }
 }
 
 // bind a name to the socket
 name.sin_family = AF_INET;
 name.sin_port = htons(port);
 name.sin_addr.s_addr = htonl(INADDR_ANY);
 memset(&(name.sin_zero), '\0', 8);
 if( bind(d_fd, (struct sockaddr *)&name, sizeof(struct sockaddr)) == -1)
 {
  setError(errno, "init(bind)");
  close(d_fd);
  d_fd = -1;
  return -1;
 }

 // listen for connections
 if( listen(d_fd, 20) == -1) // 20 = length of queue of waiting clients
 {
  setError(errno, "init(listen)");
  close(d_fd);
  d_fd = -1;
  return -1;
 }

 // Create the buffer to store client messages
 // The message from client is formatted as (int)msgLen + (char *)msg 
 d_rcvBuf = (char *)realloc(d_rcvBuf, bufSize * sizeof(char) + sizeof(int));
 if(d_rcvBuf == NULL)
 {
  setError(ENOMEM, "TCPServer(malloc)");
  close(d_fd);
  d_fd = -1;
  return -1;
 }
 d_rcvBufSize = bufSize;

 d_init = true;
 return 0;
}


//==============================================================================
// TCPServer::getStatusCode
//==============================================================================
int TCPServer::getStatusCode() const
{
 return d_status.getReportCode();
}


//==============================================================================
// TCPServer::getStatusMessage
//==============================================================================
const char *TCPServer::getStatusMessage() const
{
 return d_status.getReportMessage();
}


//==============================================================================
// TCPServer::setError
//==============================================================================
void TCPServer::setError(int code, const char *functionName)
{
 char buf[80];
 snprintf(buf, 80, "%s: %s", functionName, strerror(code));
 d_status.setReport(code, buf);
}


//==============================================================================
// TCPServer::enableIgnoreSigPipe
//==============================================================================
int TCPServer::enableIgnoreSigPipe()
{
 if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
 {
  d_status.setReport(-1, "enableIgnoreSigPipe: failed");
  return -1;
 }
 return 0;
}


//==============================================================================
// TCPServer::disableIgnoreSigPipe
//==============================================================================
int TCPServer::disableIgnoreSigPipe()
{
 if(signal(SIGPIPE, SIG_DFL) == SIG_ERR)
 {
  d_status.setReport(-1,"disableIgnoreSigPipe: failed");
  return -1;
 }
 return 0;
}


//==============================================================================
// TCPClient::TCPClient
//==============================================================================
TCPClient::TCPClient()
{
 d_fd = -1;
 d_init = false;
 d_serverName = NULL;
 d_serverPort = 0;
 d_recvTimeout.tv_sec = 1;
 d_recvTimeout.tv_usec = 0;
 d_bdp = 0;
 setError(0, "TCPClient");
}


TCPClient::TCPClient(const char *serverIp, int port, struct timeval &t, int bdp)
{
 d_init = false;
 d_serverName = NULL;
 d_serverPort = 0;
 d_recvTimeout.tv_sec = 1;
 d_recvTimeout.tv_usec = 0;
 d_fd = -1;
 d_bdp = 0;
 
 // init connection to server
 if( init(serverIp, port, t, bdp) == -1 )
  return;
 
 setError(0, "TCPClient");
}


//==============================================================================
// TCPClient::~TCPClient
//==============================================================================
TCPClient::~TCPClient()
{
 if(d_fd)
 {
  close(d_fd);
  d_fd = -1;
 }
 if(d_serverName)
  free(d_serverName);
}


//==============================================================================
// TCPClient::sendAndReceive
//==============================================================================
int TCPClient::sendAndReceive(char *outMsgBuf, int outMsgLen,
                     char *inMsgBuf, int inBufLen, int *inMsgLen)
{
 if(!d_init)
 {
  d_status.setReport(-1, "sendAndReceive: client not initialized");
  return -1;
 }
 
 // initialize connection again if we lost it due to error.
 if( fcntl(d_fd, F_GETFL) == -1 )
 {
  if( init(d_serverName, d_serverPort, d_recvTimeout, d_bdp) == -1 )
   return -1;
 }
 
 // check buffer pointers
 if( outMsgBuf == NULL )
 {
  d_status.setReport(EINVAL, "sendAndReceive: invalid buffer");
  return -1;
 }
  
 // write header (size) info to server
 if( send(d_fd, &outMsgLen, sizeof(int), 0) < (int)sizeof(int) )
 {
  setError(errno, "sendAndReceive(send)");
  close(d_fd);
  return -1;
 }

#ifdef DEBUG
 cerr << "DEBUG [sendAndReceive]: header sent to server" << endl;
#endif

 // write until done or try 3 times
 int wroteTillNow = 0;
 int numWriteAttempts = 0;
 while( (wroteTillNow < outMsgLen) && (numWriteAttempts < 3) )
 {
  int wroteNow;
  numWriteAttempts++;
  wroteNow = send(d_fd, &(outMsgBuf[wroteTillNow]), 
                         outMsgLen - wroteTillNow, 0);
  if(wroteNow == -1)
   break;
   
  wroteTillNow += wroteNow;
 } // end while
       
 // check for write failure
 if( wroteTillNow < outMsgLen )
 {
  setError(EIO, "sendAndReceive(send)"); // physical write failure
  close(d_fd);
  return -1;
 }
 
#ifdef DEBUG
 cerr << "DEBUG [sendAndReceive]: data sent to server" << endl;
#endif

 // check if interested in reply
 if(inMsgBuf == NULL)
  return 0;

#ifdef DEBUG
 cout << "DEBUG [sendAndReceive]: want reply from server" << endl;
#endif

 // read header packet for size of incoming data
 if( recv(d_fd, inMsgLen, sizeof(int), MSG_WAITALL) < (int)sizeof(int) )
 {
  setError(errno, "sendAndReceive(recv)");
  close(d_fd);
  return -1;
 }

#ifdef DEBUG
 cerr << "DEBUG [sendAndReceive]: received header from server" << endl;
#endif
 
 // inMsgLen has size of incoming message.
 // Compare with our buffer to make sure we can accomodate it.
 if( *inMsgLen > inBufLen)
 {

#ifdef DEBUG
 cerr << "DEBUG [sendAndReceive]: buffer size not enough" << endl;
#endif

  d_status.setReport(-1, "sendAndReceive: buffer not large enough.");
  close(d_fd);
  return -1;
 }
       
#ifdef DEBUG
 cerr << "DEBUG [sendAndReceive]: buffer size ok" << endl;
#endif

 // read until done or try 3 times
 int readTillNow = 0;
 int numReadAttempts = 0;
 while( (readTillNow < *inMsgLen) && (numReadAttempts < 3) )
 {
  int readNow;
  numReadAttempts++;
  readNow = recv(d_fd, &(inMsgBuf[readTillNow]), *inMsgLen - readTillNow, 
                 MSG_WAITALL);
  
  if(readNow == -1)
   break;
   
  readTillNow += readNow;
 } // end while

 // check for read failure
 if( readTillNow < *inMsgLen )
 {
  setError(EIO, "sendAndReceive(recv)"); // physical read failure
  close(d_fd);
  return -1;
 }

#ifdef DEBUG
 cerr << "DEBUG [sendAndReceive]: done reading reply from server" << endl;
#endif

 return 0;
} 
                   

//==============================================================================
// TCPClient::init
//==============================================================================
int TCPClient::init(const char *serverIp, int port, struct timeval &timeout, int bdp)
{
 struct hostent *server;
 char info[80];
 
 d_recvTimeout.tv_sec = timeout.tv_sec;
 d_recvTimeout.tv_usec = timeout.tv_usec;
 d_bdp = bdp;

 if(d_fd)
 {
  close(d_fd);
  d_fd = -1;
 }
 d_init = false;

 // copy the server name
 int nlen;
 nlen = strlen(serverIp);
 if( !d_serverName ) {
  d_serverName = (char *)realloc(d_serverName, (nlen+1) * sizeof(char));
  if( d_serverName == NULL ) {
   setError(-1, "init(realloc)");
   return -1;
  }
  strncpy(d_serverName, serverIp, nlen);
  d_serverName[nlen] = '\0';
 }
 else if(strncmp(d_serverName, serverIp, nlen)) {
  d_serverName = (char *)realloc(d_serverName, (nlen+1) * sizeof(char));
  if( d_serverName == NULL ) {
   setError(-1, "init(realloc)");
   return -1;
  }
  strncpy(d_serverName, serverIp, nlen);
  d_serverName[nlen] = '\0';
 }
 d_serverPort = port;
 

 // get network server entry
 server = gethostbyname(serverIp);
 if( server == NULL )
 {
  snprintf(info, 80, "gethostbyname %s", hstrerror(h_errno));
  d_status.setReport(h_errno, info);
  return -1;
 }
 
 // Create an endpoint for communication
 if( (d_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
 {
  setError(errno, "init(socket)");
  return -1;
 }
 
 d_server.sin_family = AF_INET;
 d_server.sin_port = htons(port);
 d_server.sin_addr.s_addr = *((in_addr_t *)server->h_addr);
 memset(&(d_server.sin_zero), '\0', 8);

 // Do not delay sending data packets
 int yes = 1;
 if( setsockopt(d_fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(int)) == -1)
 {
  setError(errno, "init(setsockopt-TCP_NODELAY)");
  close(d_fd);
  d_fd = -1;
  return -1;
 }

 // set suggested optimal socket buffer sizes.
 yes = d_bdp * 1024;
 if( yes != 0 ) {
  if( setsockopt(d_fd, SOL_SOCKET, SO_SNDBUF, (char *)&yes, sizeof(int)) == -1)
  {
   setError(errno, "init(setsockopt-SO_SNDBUF)");
   close(d_fd);
   d_fd = -1;
   return -1;
  }

  // set suggested optimal socket buffer sizes.
  if( setsockopt(d_fd, SOL_SOCKET, SO_RCVBUF, (char *)&yes, sizeof(int)) == -1)
  {
   setError(errno, "init(setsockopt-SO_RCVBUF)");
   close(d_fd);
   d_fd = -1;
   return -1;
  }
 }

 // set receive timeout 
 if( setsockopt(d_fd, SOL_SOCKET, SO_RCVTIMEO, &d_recvTimeout, 
     sizeof(struct timeval)) == -1)
 {
  setError(errno, "init(setsockopt) SO_RCVTIMEO");
  close(d_fd);
  d_fd = -1;
  return -1;
 }

 // connect to the server
 if( connect(d_fd, (struct sockaddr *)&d_server, 
    sizeof(struct sockaddr)) == -1)
 {
  setError(errno, "init(connect)");
  close(d_fd);
  d_fd = -1;
  return -1;
 }
 d_init = true; 
 return 0;
}


//==============================================================================
// TCPClient::getStatusCode
//==============================================================================
int TCPClient::getStatusCode() const
{
 return d_status.getReportCode();
}


//==============================================================================
// TCPClient::getStatusMessage
//==============================================================================
const char *TCPClient::getStatusMessage() const
{
 return d_status.getReportMessage();
}


//==============================================================================
// TCPClient::setError
//==============================================================================
void TCPClient::setError(int code, const char *functionName)
{
 char buf[80];
 snprintf(buf, 80, "%s: %s", functionName, strerror(code));
 d_status.setReport(code, buf);
}


//==============================================================================
// TCPClient::enableIgnoreSigPipe
//==============================================================================
int TCPClient::enableIgnoreSigPipe()
{
 if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
 {
  d_status.setReport(-1, "enableIgnoreSigPipe: failed");
  return -1;
 }
 return 0;
}


//==============================================================================
// TCPClient::disableIgnoreSigPipe
//==============================================================================
int TCPClient::disableIgnoreSigPipe()
{
 if(signal(SIGPIPE, SIG_DFL) == SIG_ERR)
 {
  d_status.setReport(-1, "disableIgnoreSigPipe: failed");
  return -1;
 }
 return 0;
}
