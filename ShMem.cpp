//==============================================================================
// ShMem.cpp - POSIX shared memory wrapper class.
//
// Author        : Vilas Kumar Chitrakaran
// Version       : 2.0 (Apr 2005)
// Compatibility : POSIX, GCC
//==============================================================================

#include "ShMem.hpp"
#include <cstring>
#include <sys/stat.h>

//==============================================================================
// ShMem::ShMem
//==============================================================================
ShMem::ShMem()
{
 d_shmName = NULL;
 d_shmFd = -1;
 d_shmMem = NULL;
 d_isOwner = false;
 d_errno = 0;
}


//==============================================================================
// ShMem::~ShMem
//==============================================================================
ShMem::~ShMem()
{
 if( unlink() == -1 )
  close();
 
 if(d_shmName)
  delete [] d_shmName;
}


//==============================================================================
// ShMem::create
//==============================================================================
void *ShMem::create(const char *name, int size)
{
 int oflag, prot;
 mode_t mode, processMask;
 int nlen;
 
 // Copy the name
 nlen = strlen(name);
 if(d_shmName)
  delete [] d_shmName;
 d_shmName = new char [nlen+1];
 strncpy(d_shmName, name, nlen);
 d_shmName[nlen] = '\0';
 
 // Allow all access modes so that opening the shared
 // memory from another process doesn't fail.( see open() ).
 processMask = umask(0); // remove all process mode masks => can set any mode 
 mode = S_IRWXU | S_IRWXG | S_IRWXO; // and we set all modes

 // creation and access flags
 oflag = O_RDWR | O_CREAT | O_EXCL;  

 // create shared memory
 d_shmFd = shm_open(d_shmName, oflag, mode);
 if(d_shmFd == -1)
 {
  // name already exists? unlink and try again
  shm_unlink(d_shmName);
  d_shmFd = shm_open(d_shmName, oflag, mode);
  if(d_shmFd == -1)
  {
   d_errno = errno;
   umask(processMask);
   return NULL;
  }
 }
 umask(processMask); // reset process permissions
 d_isOwner = true;
 
 // set size of memory to map
 d_shmSize = size;
 if( ftruncate(d_shmFd, size) == -1 )
 {
  d_errno = errno;
  close();
  return NULL;
 }
 
 // map memory to my address space
 prot = PROT_READ | PROT_WRITE;
 d_shmMem = mmap(NULL, d_shmSize, prot, MAP_SHARED, d_shmFd, 0);
 if( d_shmMem == (void*)(-1))
 {
  d_errno = errno;
  close();
  return NULL;
 }
 
 ::close(d_shmFd); // dont need file descriptor anymore
 d_shmFd = -1;
 d_errno = 0;

 return(d_shmMem);
}


//==============================================================================
// ShMem::open
//==============================================================================
void *ShMem::open(const char *name, int size)
{
 int oflag, prot;
 mode_t mode;
 int nlen;
 
 d_isOwner = false;

 // Copy the name
 nlen = strlen(name);
 if(d_shmName)
  delete [] d_shmName;
 d_shmName = new char [nlen+1];
 strncpy(d_shmName, name, nlen);
 d_shmName[nlen] = '\0';
 
 // Allow full access to shared object.
 // All modes work because we set so in create()
 mode = S_IRWXU | S_IRWXG | S_IRWXO;

 // open shared memory
 oflag = O_RDWR;
 d_shmFd = shm_open(d_shmName, oflag, mode);
 if(d_shmFd == -1)
 {
  d_errno = errno;
  return NULL;
 }
 
 // set size of memory to map
 d_shmSize = size;
 
 // map memory to my address space
 prot = PROT_READ | PROT_WRITE;
 d_shmMem = mmap(NULL, d_shmSize, prot, MAP_SHARED, d_shmFd, 0);
 if( d_shmMem == (void*)(-1))
 {
  d_errno = errno;
  close();
  return NULL;
 }
 
 ::close(d_shmFd); // dont need file descriptor anymore
 d_shmFd = -1;
 d_errno = 0;
 
 return(d_shmMem);
}


//==============================================================================
// ShMem::close
//==============================================================================
int ShMem::close()
{
 // close
 if(d_shmFd != -1)
 {
  if( ::close(d_shmFd) == -1 )
  {
   d_errno = errno;
   return -1;
  }
 }
 
 // unmap
 if(d_shmMem != NULL || (d_shmMem != (void*)(-1)))
 {
  if( munmap(d_shmMem, d_shmSize) == -1 )
  {
   d_errno = errno;
   return -1;
  }
 }
 
 d_shmMem = NULL;
 d_shmFd = -1;
 d_errno = 0;
 
 return 0;
}


//==============================================================================
// ShMem::unlink
//==============================================================================
int ShMem::unlink()
{
 if(!d_isOwner)
 {
  d_errno = EACCES;
  return -1;
 }

 if( close() == -1 )
  return -1;
 
 if( shm_unlink(d_shmName) == -1)
 {
  d_errno = errno;
  return -1;
 }

 d_isOwner = false;
 d_errno = 0;
 
 return 0;
}


