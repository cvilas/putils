//==============================================================================
// RecursiveMutex.hpp - POSIX mutex wrapper class.
//
// Author        : Vilas Kumar Chitrakaran
// Version       : 2.0 (Apr 2005)
// Compatibility : POSIX, GCC
//==============================================================================

#ifndef _RECURSIVEMUTEX_HPP_INCLUDED
#define _RECURSIVEMUTEX_HPP_INCLUDED

#include <pthread.h>
#include "ErrnoException.hpp"

//==============================================================================
// class RecursiveMutex
//------------------------------------------------------------------------------
// \brief
// A wrapper for pthread mutex, with the added functionality that it is
// recursive.
//
// <ul>
// <li>A recursive mutex can be locked more than once by a thread without 
//     causing a deadlock.
// <li>The thread must call the unlock routine on the mutex the same number
//     of times that it called the lock routine before another thread can 
//     lock the same mutex.
// <li>This class is useful if the thread is already in a mutex protected
//     section of the code and needs to call another routine that locks
//     the same mutex again.
// <li>This class will throw an exception of type ErrnoException in case of
//     errors.
// </ul>
//
// The error checking code snippet used here is similar to John Nagle's 
// mutexlock.h.
//
// <b>Example Program:</b>
//
// \include RecursiveMutex.t.cpp
//==============================================================================

class RecursiveMutex
{
 public:
  inline RecursiveMutex();
   // Constructs a recursive mutex
 
  inline ~RecursiveMutex();
   // Deletes a recursive mutex

  inline void lock();
   // Locks a recursive mutex. If the mutex is locked by another thread, 
   // this thread is blocked until the mutex gets unlocked.

  inline void unlock();
   // Unlocks a recursive mutex.

  inline int tryLock();
   // Returns 0 and locks the mutex if it is not already locked by another 
   // thread, else returns -1.

  //======== END OF INTERFACE ========

 private:
  inline void errorCheck(int code);
   // This will throw an exception on any error.
   
  RecursiveMutex &operator=(RecursiveMutex &m);
   // prevents assignment operation
   
  RecursiveMutex(RecursiveMutex &m);
   // prevents copy of one mutex to another
  
  pthread_mutex_t d_mutex;
   // The mutex object
  
  pthread_cond_t d_condition;
   // A condition variable
  
  unsigned int d_recursionCount;
   // Depth of nested recursive calls to lock
  
  pthread_t d_ownerId;
   // Current owner of mutex
};


//==============================================================================
// RecursiveMutex::RecursiveMutex()
//==============================================================================
RecursiveMutex::RecursiveMutex()
{
 pthread_mutexattr_t mAttr;
 pthread_condattr_t cAttr;
 
 d_recursionCount = 0;
 d_ownerId = (pthread_t)NULL;
 
 errorCheck( pthread_mutexattr_init(&mAttr) ); // init to default attributes
 errorCheck( pthread_condattr_init(&cAttr) );
 
 // The following are actually default.
 errorCheck( pthread_mutexattr_setpshared(&mAttr, PTHREAD_PROCESS_PRIVATE) );
 errorCheck( pthread_condattr_setpshared(&cAttr, PTHREAD_PROCESS_PRIVATE) );
 //errorCheck( pthread_mutexattr_setprotocol(&mAttr, PTHREAD_PRIO_INHERIT) );
 // * doesnt work in linux *
 
 errorCheck( pthread_mutex_init(&d_mutex, &mAttr) ); // init mutex
 errorCheck( pthread_cond_init(&d_condition, &cAttr) ); // init condvar

 pthread_mutexattr_destroy(&mAttr);
 pthread_condattr_destroy(&cAttr);
}


RecursiveMutex::RecursiveMutex(RecursiveMutex &m)
{
 throw(ErrnoException(EINVAL));
}


//==============================================================================
// RecursiveMutex::~RecursiveMutex()
//==============================================================================
RecursiveMutex::~RecursiveMutex()
{
 pthread_mutex_destroy(&d_mutex);
 pthread_cond_destroy(&d_condition);
}


//==============================================================================
// RecursiveMutex::lock()
//==============================================================================
void RecursiveMutex::lock()
{
 pthread_t threadId = pthread_self();
 errorCheck( pthread_mutex_lock(&d_mutex) );
 if( pthread_equal(d_ownerId, threadId) )
  d_recursionCount++;
 else
 {
  while(d_recursionCount)
   errorCheck( pthread_cond_wait(&d_condition, &d_mutex) );
  d_recursionCount = 1;
  d_ownerId = threadId;
 }
 errorCheck( pthread_mutex_unlock(&d_mutex) );
}


//==============================================================================
// RecursiveMutex::unlock()
//==============================================================================
void RecursiveMutex::unlock()
{
 errorCheck( pthread_mutex_lock(&d_mutex) );
 if( pthread_equal(d_ownerId, pthread_self()) )
 {
  d_recursionCount--;
  if(!d_recursionCount)
  {
   d_ownerId = (pthread_t)NULL;
   errorCheck( pthread_cond_signal(&d_condition) );
  }
 }
 errorCheck( pthread_mutex_unlock(&d_mutex) );
}


//==============================================================================
// RecursiveMutex::tryLock()
//==============================================================================
int RecursiveMutex::tryLock()
{
 int retVal = -1;
 pthread_t threadId = pthread_self();
 errorCheck( pthread_mutex_lock(&d_mutex) );
 if( pthread_equal(d_ownerId, threadId) )
 {
  d_recursionCount++;
  retVal = 0;
 }
 else
 { 
  if(!d_recursionCount)
  {
   d_recursionCount = 1;
   d_ownerId = threadId;
   retVal = 0;
  }
 } 
 errorCheck( pthread_mutex_unlock(&d_mutex) );
 return retVal;
}


//==============================================================================
// RecursiveMutex::operator=()
//==============================================================================
RecursiveMutex &RecursiveMutex::operator=(RecursiveMutex &m)
{
 throw(ErrnoException(EINVAL));
}


//==============================================================================
// RecursiveMutex::errorCheck()
//==============================================================================
void RecursiveMutex::errorCheck(int code)
{
 if(code == 0) return;
 throw(ErrnoException(code, "[RecursiveMutex]"));
}


#endif // _RECURSIVE_MUTEX_HPP_INCLUDED
