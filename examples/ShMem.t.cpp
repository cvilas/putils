//==============================================================================
// ShMem.t.cpp - Example program for creating and writing
//                            into shared memory.
//
// Author        : Vilas Kumar Chitrakaran
//==============================================================================

#include <stdio.h>
#include <iostream>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "ShMem.hpp"
#include <cstring>

using namespace std;

//==============================================================================
// writer thread
// - Creates shared memory
// - modifies contents continuously
//==============================================================================
void *writer(void *arg)
{
 arg = arg;
 ShMem shm;
 double *counter;
 
 // create shared memory
 counter = (double *)shm.create( "/shm0", sizeof(double) );
 if( counter == NULL )
 {
  cout << "writer: " << strerror(shm.getErrnoError()) << endl;
  return NULL;
 }
 
 // change shared memory
 *counter = 0;
 while(*counter < 10)
 {
  cout << "writer: " << ++(*counter) << endl;;
  sleep(1);
 }
 
 // unlink
 shm.unlink();
 
 return NULL;
}

//==============================================================================
// reader thread
// - opens shared memory created by writer
// - reads shared memory continuously
//==============================================================================
int reader()
{
 ShMem shm;
 double *counter;
 
 // open existing shared memory
 counter = (double *)shm.open( "/shm0", sizeof(double) );
 if( counter == NULL )
 {
  cout << "reader: " << strerror(shm.getErrnoError()) << endl;
  return -1;
 }
 
 // read shared memory
 while(*counter < 10)
 {
  cout << "reader: " << *counter << endl;
  sleep(1);
 }
 
 // close
 shm.close();
 
 return 0;
}


//==============================================================================
// main function
//==============================================================================
int main()
{
 pthread_t threadId;
 pthread_create(&threadId, NULL, &writer, NULL);
 sleep(1);
 reader();
 return 0;
}
