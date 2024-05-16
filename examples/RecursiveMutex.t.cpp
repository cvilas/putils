//==============================================================================
// RecursiveMutex.t.cpp - Example program for RecursiveMutex class.
//
// Author        : Vilas Kumar Chitrakaran
//==============================================================================


#include "RecursiveMutex.hpp"
#include <iostream>
#include <string.h>
#include <time.h>
#include <stdlib.h>

//==============================================================================
// This program demostrates how access to a resource shared between two 
// threads is controlled using a RecursiveMutex object. 
//==============================================================================

using namespace std;

RecursiveMutex mutex; // mutex object
static int counter;     // shared resource

//==============================================================================
// Consumer thread
//==============================================================================
int consume()
{
 struct timespec delay;
 delay.tv_sec = 0;
 delay.tv_nsec = (long int)5e8;
 while(1)
 {
  if(mutex.tryLock() == -1)
   cout << "CONSUME: missed" << endl << flush;
  else
  {
   cout << "CONSUME : " << --counter << endl << flush;
   mutex.unlock();
  }
  nanosleep(&delay, NULL);
 }
 return 0;
}

//==============================================================================
// Producer thread
// NOTE that the mutex is recursively locked twice
//==============================================================================
void *produce(void *arg)
{
 arg=arg;
 struct timespec delay;
 delay.tv_sec = 0;
 delay.tv_nsec = (long int)1e8;
 while(1)
 {
  mutex.lock();
  mutex.lock(); // This is valid!
  cout << "PRODUCE: " << ++counter << endl << flush;
  mutex.unlock();
  nanosleep(&delay, NULL); // sleep a little before releasing second mutex
  mutex.unlock();
  nanosleep(&delay, NULL);
 }
 return NULL;
}



//==============================================================================
// main function
//==============================================================================
int main()
{
 pthread_t threadId;
 pthread_create(&threadId, NULL, &produce, NULL);
 consume();
 return 0;
}
