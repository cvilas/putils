//==============================================================================
// UDPClientServer.t.cpp - Example program for UDPClient/UDPServer
//
// Author        : Vilas Kumar Chitrakaran
//==============================================================================

#include "UDPClientServer.hpp"
#include <iostream>
#include <string>
#include <pthread.h>
#include <cstring>

using namespace std;

//==============================================================================
// class MyServer
//==============================================================================
class MyServer : public UDPServer
{
 public:
  MyServer(int port, int maxLen, int bdp) : UDPServer(port, maxLen, bdp){};
  ~MyServer() {};
 protected:
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
// - handles messages from clients
//==============================================================================
void *server(void *arg)
{
 arg=arg;
 MyServer server(3000, 80, 625);

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
//==============================================================================
int client()
{
 char outMsgBuf[80];
 char inMsgBuf[80];
 int outMsgLen = 0;
 int inMsgLen;
 
 struct timeval timeout;
 timeout.tv_sec = 0;
 timeout.tv_usec = 50000; // 50 ms

 // initialize a client and connect to server
 UDPClient client("127.0.0.1", 3000, timeout, 625);
 if(client.getStatusCode())
 {
  cout << "client: " << client.getStatusMessage() << endl;
  return -1;
 }
 
 int msgNum = 0;
 while(1)
 {
  if(msgNum > 100)
   break;

  // a message
  snprintf(outMsgBuf, 80, "%s %d", "Hello server", msgNum);
  outMsgLen = strlen(outMsgBuf);
  cout << endl << "client  : Sending msg. - " << outMsgBuf << endl;
  msgNum++;
   
  // Send the message and receive reply
  if( client.sendAndReceive(outMsgBuf, outMsgLen, inMsgBuf, 80, &inMsgLen) == -1)
  {
   cout << "client  : " << client.getStatusMessage() << endl;
   break;
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
