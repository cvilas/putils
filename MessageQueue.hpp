//==============================================================================
// MessageQueue.hpp - POSIX message queue wrapper class.
//
// Author        : Vilas Kumar Chitrakaran
// Version       : 2.0 (Apr 2005)
// Compatibility : POSIX, GCC
//==============================================================================

#ifndef _MESSAGEQUEUE_HPP_INCLUDED
#define _MESSAGEQUEUE_HPP_INCLUDED

#include <mqueue.h>
#include <string>
#include <errno.h>
#include <pthread.h>

//==============================================================================
// class MessageQueue
//------------------------------------------------------------------------------
// \brief
// Inter-process messaging mechanism.
//
// This class provides a wrapper for non-blocking message queues. Messages are
// sent at the priority of the sending process, and received highest priority 
// first. Messages of equal priority are received on a first-come-first-serve
// basis. When developing a client-server system using a MessageQueue object, 
// run the server at maximum possible priority as it waits for messages. 
// 
// This is an efficient mechanism only for sending small messages (because 
// messages get 'copied' from sender to OS, and then OS to receiver). Use 
// SharedMemory for passing large amounts of data between processes.
//
// <b>Example Program:</b>
//
// \include MessageQueue.t.cpp
//==============================================================================

class MessageQueue
{
 public: 
  MessageQueue();
   // Default constructor does nothing.
   
  ~MessageQueue();
   // Default destructor deletes the message queue if it was created 
   // by the object.
   
  int create(const char *name, int maxNumMsgs, int maxMsgLen = 1024);
   // Creates a message queue object with read/write access. 
   // Note that if a message queue with the same name already exists, this 
   // function will exit with an error (errno set).
   //  name        Name of the message queue. For portability, the name
   //              should begin with a leading "/" and contain no other 
   //              "/" characters.
   //  maxNumMsgs  Number of messages the queue must hold
   //  maxMsgLen   Maximum possible length (bytes) of each message (default 1kB)
   //  return      0 on sucess, -1 on error, 'errno' is set and can be 
   //              retrieved by a call to getErrnoError().

  int open(const char *name);
   // Opens an existing message queue for read/write access. 
   // Note that if the queue does not exist, this function will return with an 
   // error (errno set).
   //  name    Name of the queue to establish connection with.
   //  return  0 on sucess, -1 on error, 'errno' is set and can be 
   //          retrieved by a call to getErrnoError().

  int close();
   // Release access to the message queue. Note that the queue and any
   // messages it may contain are not deleted, and can be opened again.
   //  return  0 on sucess, -1 on error, 'errno' is set and can be 
   //          retrieved by a call to getErrnoError().

  int unlink();
   // If the process calling this function created the message queue, this 
   // function calls close() and marks the message queue for deletion. 
   // Message queues are persistant, i.e., if there are processes that 
   // have the queue open when this function is called then the destruction 
   // of the queue is delayed until all processes have closed their access 
   // to the queue (by calling close()).
   //  return  0 on sucess, -1 on error, 'errno' is set and can be 
   //          retrieved by a call to getErrnoError(). This function
   //          will return -1 if it is called by an object that did not create
   //          the message queue.

  int trySend(const char *msgBuffer, int msgSize);
   // Send a message without blocking.  
   //  msgBuffer  Buffer containing message to be sent.
   //  msgSize    Size of the message.
   //  return     0 on sucess, -1 on error. If the queue is full, 
   //             this function will return -1 with errno set to 
   //             EAGAIN (which can be retrieved by a call to getErrnoError()).

  int send(const char *msgBuffer, int msgSize);
   // Send a message. If the queue is full, this function blocks until
   // the message queue empties and the message can be placed in the queue  
   //  msgBuffer  Buffer containing message to be sent.
   //  msgSize    Size of the message.
   //  return     0 on sucess, -1 on error, 'errno' is set and can be 
   //             retrieved by a call to getErrnoError().

  int tryReceive(char *msgBuffer, int bufSize);
   // Removes a message from the head of the queue. If the queue is empty, 
   // returns immediately without blocking.
   //  msgBuffer  Buffer for received message.
   //  bufSize    Size of the buffer (must be atleast as large as the maximum 
   //             message size for the queue).
   //  return     Number of bytes in the received message, -1 on error. If the 
   //             queue is empty, this function will return -1 with errno set to 
   //             EAGAIN (which can be retrieved by a call to getErrnoError()).

  int receive(char *msgBuffer, int bufSize);
   // Removes a message from the head of the queue. If the queue is empty, 
   // it will block until there is something to read.
   //  msgBuffer  Buffer for received message.
   //  bufSize    Size of the buffer (must be atleast as large as the maximum 
   //             message size for the queue).
   //  return     Number of bytes in the received message, -1 on error. If the 
   //             queue is empty, this function will return -1 with errno set to 
   //             EAGAIN (which can be retrieved by a call to getErrnoError()).

  int notify(const struct sigevent *notification);
   // Notify the calling process asynchronously if a message appeared in the queue. 
   // This is useful if you don't want to keep polling the queue to find whether
   // a new message has arrived, for instance in handling emergency messages.
   // (See pp. 107, Programming for the Real World, POSIX.4 for an example.) 
   //  notification  NULL or a pointer to sigevent structure that describes how 
   //                you want to be notified. 
   //  return        0 on sucess, -1 on error, 'errno' is set and can be 
   //                retrieved by a call to getErrnoError().
   
  inline int getMaxNumMsgs() const;
   //  return  The maximum number of messages the queue can hold.
  
  inline int getMaxMsgLength() const;
   //  return  The maximum possible length (in bytes) for a message in queue.
  
  inline int getErrnoError() const;
   //  return  Error code from the last error that happened before
   //          this function was called, else 0. Error codes are defined
   //          in the standard header errno.h
  
  //======== END OF INTERFACE ========

 private:

  char *d_name;        // Name of the message queue
  mqd_t d_desc;        // Descriptor for the queue
  int d_maxNumMsgs;    // max. number of messages in queue
  int d_maxMsgLength;  // maximum length of a message in the queue
  bool d_isBlocking;   // true if this end of the queue is configured
                       // to block at receive (if queue) or write (when 
                       // queue is full).
  bool d_isOwner;      // true if this object created the queue, false if 
                       // this object only opened the queue
  int d_errno;         // Latest error message
};



//==============================================================================
// MessageQueue::getMaxNumMsgs
//==============================================================================
int MessageQueue::getMaxNumMsgs() const
{
 return d_maxNumMsgs;
}


//==============================================================================
// MessageQueue::getMaxMsgLength
//==============================================================================
int MessageQueue::getMaxMsgLength() const
{
 return d_maxMsgLength;
}


//==============================================================================
// MessageQueue::getErrnoError
//==============================================================================
int MessageQueue::getErrnoError() const
{
 return d_errno;
}


#endif // _MESSAGEQUEUE_HPP_INCLUDED
