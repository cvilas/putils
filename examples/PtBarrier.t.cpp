//==============================================================================
// PtBarrier.t.cpp - Example program for PtBarrier class.
//
// Author        : Vilas Kumar Chitrakaran
//==============================================================================
 
#include "PtBarrier.hpp"
#include <iostream>
#include <time.h>

using namespace std;

PtBarrier barrier(2);
// The synchronization object for two threads

//==============================================================================
// work 
// - does some work
// - waits at the barrier for the other thread to finish its work cycle
// - loop again
//==============================================================================
void *work(void *arg)
{
 struct timespec napTime;
 napTime.tv_sec = (int)(arg);
 napTime.tv_nsec = 0;
 while (1)
 {
  nanosleep( &napTime, NULL ); // work!!
  cout << "Thread " << pthread_self() << " worked " 
       << napTime.tv_sec << " secs."
       << endl << flush;

  // wait for the other thread
  barrier.wait();

  cout << "Thread " << pthread_self() << " sync."
       << endl << flush;
 }
 return NULL;
}


//==============================================================================
// main function
// - creates two synchronized threads that wait for each other
//   at the end of each cycle.
//==============================================================================
int main()
{
 pthread_t threadId;
 int arg;
 arg = 1;
 pthread_create(&threadId, NULL, work, (void *)arg);
 arg = 2;
 work((void *)arg);
 return 0;
}
