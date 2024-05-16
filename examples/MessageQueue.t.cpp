//==============================================================================
// MessageQueue.t.cpp - Example program for MessageQueue class.
//
// Author        : Vilas Kumar Chitrakaran
//==============================================================================

#include "MessageQueue.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>

using namespace std;

//==============================================================================
// receiver thread
// - Creates a queue that can hold 2 messages at a time, each 1 byte long
// - Waits for, receives and prints 3 messages before exiting 
//==============================================================================
void *receiver(void *arg)
{
 arg = arg;
 MessageQueue rq;
 char rbuf[10];
 
 int numMsgs = 2;
 int msgSize = 1 * sizeof(char);
 
 int m = 0;
 
 // create receive queue
 if( rq.create("/rq", numMsgs, msgSize) == -1)
 {
  cout << "receiver: " << strerror(rq.getErrnoError()) << endl << flush;
  return NULL;
 }
 
 // stats
 cout << "receiver: queue created for " << rq.getMaxNumMsgs() << " messages, " 
      << rq.getMaxMsgLength() << " bytes long." << endl << flush;
 
 while(m < 3)
 {
  // receive a message from someone
  strncpy(rbuf, "\0", 10);
  if( rq.receive(rbuf, msgSize) == -1)
   cout << "receiver: " << strerror(rq.getErrnoError()) << endl << flush;
  else
   cout << "receiver: received msg: " << rbuf << endl << flush;
  m++;
 }
 
 // exit
 if( rq.unlink() == -1)
   cout << "receiver: " << strerror(rq.getErrnoError()) << endl << flush;
 cout << "receiver: exiting" << endl << flush;
 return NULL;
}


//==============================================================================
// sender thread
// - opens queue created by receiver
// - sends messages from user without blocking
//==============================================================================
int sender()
{
 MessageQueue sq;
 char sbuf;
 
 // open queue
 if( sq.open("/rq") == -1)
 {
  cout << "sender: " << strerror(sq.getErrnoError()) << endl << flush;
  return -1;
 }

 // stats
 cout << "sender: queue opened for " << sq.getMaxNumMsgs() << " messages, " 
      << sq.getMaxMsgLength() << " bytes long." << endl << flush;

 while(1)
 {
  // send message
  cout << endl << "sender: Enter message: " << endl << flush;
  cin >> sbuf;

  if ( sq.trySend(&sbuf, sizeof(char)) == -1 )
   cout << "sender: " << strerror(sq.getErrnoError()) 
        << ". Queue is full." << endl << flush;
 }
 return 0;
}


//==============================================================================
// main function
//==============================================================================
int main()
{
 pthread_t threadId;
 pthread_create(&threadId, NULL, &receiver, NULL);
 sleep(1);
 sender();
 return 0;
}
