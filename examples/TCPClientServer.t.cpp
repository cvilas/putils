//==============================================================================
// TCPClientServer.t.cpp - Example program for TCPClient/TCPServer
//
// Author        : Vilas Kumar Chitrakaran
//==============================================================================

#include "TCPClientServer.hpp"
#include <iostream>
#include <string>
#include <pthread.h>
#include <cstring>

using namespace std;

//==============================================================================
// class MyServer
//==============================================================================
class MyServer : public TCPServer
{
 public:
  MyServer(int port, int maxLen, int bdp) : TCPServer(port, maxLen, bdp){};
  ~MyServer() {};
  virtual const char *receiveAndReply(const char *inMsgBuf, int inMsgLen, int *outMsgLen); 
 private:
  char d_outMsgBuf[80];
};


const char *MyServer::receiveAndReply(const char *inMsgBuf, int inMsgLen, int *outMsgLen)
{
 cout << "MyServer: client said: ";
 for(int i = 0; i < inMsgLen; i++) cout << inMsgBuf[i];
 cout << endl;
 snprintf(d_outMsgBuf, 80, "%s", "Hi there client!!");
 *outMsgLen = strlen(d_outMsgBuf);
 return d_outMsgBuf;
}

//==============================================================================
// server
// - handles messages less than 8 bytes long from clients
//==============================================================================
void *server(void *arg)
{
 arg=arg;
 MyServer server(3000, 8, 100);

 server.enableIgnoreSigPipe();
 
 // check for errors
 if(server.getStatusCode())
  cout << "server: " << server.getStatusMessage() << endl;

 // serve clients
 server.doMessageCycle();
 
 // check for error
 if(server.getStatusCode())
  cout << "server: " << server.getStatusMessage() << endl;
 
 return NULL;
}


//==============================================================================
// client
// - alternately sends valid and invalid messages to server
//==============================================================================
int client()
{
 char outMsgBuf[80];
 char inMsgBuf[80];
 int outMsgLen = 0;
 int inMsgLen;
 bool flip = false;

 struct timeval timeout;
 timeout.tv_sec = 0;
 timeout.tv_usec = 5000; // 5 ms
 
 // initialize a client and connect to server
 TCPClient client("127.0.0.1", 3000, timeout, 100);
 if(client.getStatusCode())
 {
  cout << "client: " << client.getStatusMessage() << endl;
  return -1;
 }
 
 client.enableIgnoreSigPipe();
 int msgNum = 0;
 while(1)
 {
  if(msgNum > 100)
   return 0;

  // a message
  if(flip)
  {
   // improper message (longer than what server can handle)
   snprintf(outMsgBuf, 80, "%s %d", "Hello server", msgNum);
   outMsgLen = strlen(outMsgBuf);
   flip = !flip;
   cout << endl << "client  : Sending invalid (long) client msg. - " << outMsgBuf << endl;
  }
  else
  {
   // proper message
   snprintf(outMsgBuf, 80, "%s %d", "Hello", msgNum);
   outMsgLen = strlen(outMsgBuf);
   //flip = !flip;
   cout << endl << "client  : Sending valid client msg. - " << outMsgBuf << endl;
  }
  msgNum++;
   
  // Send a message and receive reply
  if( client.sendAndReceive(outMsgBuf, outMsgLen, inMsgBuf, 80, &inMsgLen) == -1)
  {
   cout << "client  : " << client.getStatusMessage() << endl;
   continue;
  }
 
  // reply received
  cout << "client  : server replied: ";
  for(int i = 0; i < inMsgLen; i++) cout << inMsgBuf[i];
  cout << endl;
 }

 // bye 
 if(client.getStatusCode())
  cout << "client  : " << client.getStatusMessage() << endl;
 
 return 0;
}


//==============================================================================
// main function
//==============================================================================
int main()
{
 pthread_t threadId;
 pthread_create(&threadId, NULL, &server, NULL);
 sleep(1);
 client();
 return 0;
}
