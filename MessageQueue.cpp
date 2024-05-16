//==============================================================================
// MessageQueue.cpp - POSIX message queue wrapper class.
//
// Author        : Vilas Kumar Chitrakaran
// Version       : 2.0 (Apr 2005)
// Compatibility : POSIX, GCC
//==============================================================================

#include "MessageQueue.hpp"
#include <cstring>
#include <sys/stat.h>

//==============================================================================
// MessageQueue::MessageQueue
//==============================================================================
MessageQueue::MessageQueue()
{
 d_name = NULL;
 d_desc = (mqd_t)-1;
 d_maxNumMsgs = 0;
 d_maxMsgLength = 0;
 d_isBlocking = false;
 d_errno = 0;
 d_isOwner = false;
}


//==============================================================================
// MessageQueue::~MessageQueue
//==============================================================================
MessageQueue::~MessageQueue()
{
 if( unlink() == -1 )
  close();

 if(d_name)
  delete [] d_name;
}


//==============================================================================
// MessageQueue::create
//==============================================================================
int MessageQueue::create(const char *name, int maxNumMsgs, int maxMsgLen)
{
 int nlen;
 mode_t mode, processMask;
 int oflags;
 struct mq_attr mqAttrs;
 
 // copy the name
 nlen = strlen(name);
 if(d_name)
  delete [] d_name;
 d_name = new char [nlen+1];
 strncpy(d_name, name, nlen);
 d_name[nlen] = '\0';
 
 // Allow all access modes so that opening the queue
 // from another process doesn't fail.( see open() ).
 processMask = umask(0); // remove all process mode masks => can set any mode 

 // permissions bits default: nonblocking read/write for anyone
 mode = S_IRWXU | S_IRWXG | S_IRWXO;
 oflags = O_CREAT | O_EXCL | O_RDWR | O_NONBLOCK;
 mqAttrs.mq_maxmsg = maxNumMsgs;
 mqAttrs.mq_msgsize = maxMsgLen;
 mqAttrs.mq_flags = 0; // this is not used by mq_open at all!.
 
 // create queue
 d_desc = mq_open( d_name, oflags, mode, &mqAttrs );
 if(d_desc == (mqd_t)-1)
 {
  // name already exists? unlink and try again
  mq_unlink(d_name);
  d_desc = mq_open(d_name, oflags, mode, &mqAttrs);
  if(d_desc == (mqd_t)-1)
  {
   d_errno = errno;
   umask(processMask);
   return -1;
  }
 }
 umask(processMask);
 d_isOwner = true;
 
 // Read attributes back and store them (sanity check)
 mq_getattr( d_desc, &mqAttrs);
 d_maxNumMsgs = mqAttrs.mq_maxmsg;
 d_maxMsgLength = mqAttrs.mq_msgsize;
 (mqAttrs.mq_flags == O_NONBLOCK) ? (d_isBlocking = false) : 
                                    (d_isBlocking = true);
 
 return 0;
}


//==============================================================================
// MessageQueue::open
//==============================================================================
int MessageQueue::open(const char *name)
{
 int nlen;
 mode_t mode;
 int oflags;
 struct mq_attr mqAttrs;
 
 // copy the name
 nlen = strlen(name);
 if(d_name)
  delete [] d_name;
 d_name = new char [nlen+1];
 strncpy(d_name, name, nlen);
 d_name[nlen] = '\0';
 
 // permissions bits default: nonblocking read/write for anyone
 mode = S_IRWXU | S_IRWXG | S_IRWXO; // ** test this. do we need umask(0)? **
 oflags = O_RDWR | O_NONBLOCK;
 
 // open queue
 d_desc = mq_open( d_name, oflags, mode, &mqAttrs );
 if(d_desc == (mqd_t)-1)
 {
  d_errno = errno;
  return -1;
 }
 d_isOwner = false;
 
 // Read attributes back and store them
 mq_getattr( d_desc, &mqAttrs);
 d_maxNumMsgs = mqAttrs.mq_maxmsg;
 d_maxMsgLength = mqAttrs.mq_msgsize;
 (mqAttrs.mq_flags == O_NONBLOCK) ? (d_isBlocking = false) : 
                                    (d_isBlocking = true);
 
 return 0;
}


//==============================================================================
// MessageQueue::close
//==============================================================================
int MessageQueue::close()
{
 if( d_desc == (mqd_t)-1 )
  return 0;
  
 if( mq_close(d_desc) == -1 )
 {
  d_errno = errno;
  return -1;
 }

 d_desc = (mqd_t)-1;
 d_errno = 0;
 
 return 0;
}


//==============================================================================
// MessageQueue::unlink
//==============================================================================
int MessageQueue::unlink()
{
 if(!d_isOwner)
 {
  d_errno = EACCES;
  return -1;
 }

 if( close() == -1 )
  return -1;
 
 if( mq_unlink(d_name) == -1)
 {
  d_errno = errno;
  return -1;
 }

 d_isOwner = false;
 d_errno = 0;
 
 return 0;
}


//==============================================================================
// MessageQueue::trySend
//==============================================================================
int MessageQueue::trySend(const char *msgBuffer, int msgSize)
{
 // change to unblocked if currently blocked
 if(d_isBlocking == true)
 {
  struct mq_attr mqAttr, prevAttr;
  if( mq_getattr(d_desc, &mqAttr) == -1)
  {
   d_errno = errno;
   return -1;
  }
  mqAttr.mq_flags = O_NONBLOCK;
  if( mq_setattr(d_desc, &mqAttr, &prevAttr) == -1)
  {
   d_errno = errno;
   return -1;
  }
  d_isBlocking = false;
 }
 
 // get current process priority
 int policy;
 struct sched_param param;
 if( pthread_getschedparam( pthread_self(), &policy, &param) == -1)
 {
  d_errno = errno;
  return -1;
 }

 // send message at same priority as process
 if( mq_send( d_desc, msgBuffer, msgSize, param.sched_priority) == -1)
 {
  d_errno = errno;
  return -1;
 }
 
 d_errno = 0;
 return 0;
}


//==============================================================================
// MessageQueue::send
//==============================================================================
int MessageQueue::send(const char *msgBuffer, int msgSize)
{
 // change to blocked queue if currently unblocked queue
 if(d_isBlocking == false)
 {
  struct mq_attr mqAttr, prevAttr;
  if( mq_getattr(d_desc, &mqAttr) == -1)
  {
   d_errno = errno;
   return -1;
  }
  mqAttr.mq_flags = 0; // is this correct?
  if( mq_setattr(d_desc, &mqAttr, &prevAttr) == -1)
  {
   d_errno = errno;
   return -1;
  }
  d_isBlocking = true;
 }
 
 // get current process priority
 int policy;
 struct sched_param param;
 if( pthread_getschedparam( pthread_self(), &policy, &param) == -1)
 {
  d_errno = errno;
  return -1;
 }

 // send message at same priority as process
 if( mq_send( d_desc, msgBuffer, msgSize, param.sched_priority) == -1)
 {
  d_errno = errno;
  return -1;
 }
 
 d_errno = 0;
 return 0;
}


//==============================================================================
// MessageQueue::tryReceive
//==============================================================================
int MessageQueue::tryReceive(char *msgBuffer, int bufSize)
{
 // change to unblocked if currently blocked
 if(d_isBlocking == true)
 {
  struct mq_attr mqAttr, prevAttr;
  if( mq_getattr(d_desc, &mqAttr) == -1)
  {
   d_errno = errno;
   return -1;
  }
  mqAttr.mq_flags = O_NONBLOCK;
  if( mq_setattr(d_desc, &mqAttr, &prevAttr) == -1)
  {
   d_errno = errno;
   return -1;
  }
  d_isBlocking = false;
 }
 
 // receive message
 unsigned int prio;
 int retVal;
 retVal = mq_receive(d_desc, msgBuffer, bufSize, &prio);
 if(retVal == -1)
 {
  d_errno = errno;
  return -1;
 }
 d_errno = 0;
 return retVal;
}


//==============================================================================
// MessageQueue::receive
//==============================================================================
int MessageQueue::receive(char *msgBuffer, int bufSize)
{
 // change to blocked queue if currently unblocked queue
 if(d_isBlocking == false)
 {
  struct mq_attr mqAttr, prevAttr;
  if( mq_getattr(d_desc, &mqAttr) == -1)
  {
   d_errno = errno;
   return -1;
  }
  mqAttr.mq_flags = 0;
  if( mq_setattr(d_desc, &mqAttr, &prevAttr) == -1)
  {
   d_errno = errno;
   return -1;
  }
  d_isBlocking = true;
 }

 // receive message
 unsigned int prio;
 int retVal;
 retVal = mq_receive(d_desc, msgBuffer, bufSize, &prio);
 if(retVal == -1)
 {
  d_errno = errno;
  return -1;
 }
 d_errno = 0;
 return retVal;
}


//==============================================================================
// MessageQueue::notify
//==============================================================================
int MessageQueue::notify(const struct sigevent *notification)
{
 if( mq_notify(d_desc, notification) == -1 )
 {
  d_errno = errno;
  return -1;
 }
 d_errno = 0;
 return 0;
}
