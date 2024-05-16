//==============================================================================
// ShMem.hpp - POSIX shared memory wrapper class.
//
// Author        : Vilas Kumar Chitrakaran
// Version       : 2.0 (Apr 2005)
// Compatibility : POSIX, GCC
//==============================================================================

#ifndef _SHMEM_HPP_INCLUDED
#define _SHMEM_HPP_INCLUDED

#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <fcntl.h>
#include <stdlib.h>


//==============================================================================
// class ShMem
//------------------------------------------------------------------------------
// \brief
// Shared memory objects.
//
// This class provides just the basic shared memory functionality. The user 
// must provide the facility for synchronization of access to the shared object
// between multiple processes (using memory based semaphores, etc - see pp. 143,
// Programming for the Real World, POSIX.4).
// 
// <b>Example Program:</b>
//
// \include ShMem.t.cpp
//==============================================================================

class ShMem
{
 public:
  ShMem();
   // Default constructor does nothing
   
  ~ShMem();
   // Default destructor deletes shared memory region if it was created
   // by the object.
  
  void *create(const char *name, int size); 
   // Creates a shared memory object with read/write access and maps it to 
   // your process address space. Note that if the shared memory 
   // object already exists this function will exit with an error (errno set).
   //  name    Name of the shared memory object. For portability, the name
   //          should begin with a leading "/" and contain no other 
   //          "/" characters.
   //  size    Size (number of bytes) of the shared memory object
   //  return  If successful, a pointer to the starting memory location 
   //          of shared memory, else NULL, 'errno' is set and can be 
   //          retrieved by a call to getErrnoError().

  void *open(const char *name, int size);
   // Opens a shared memory object for read/write and maps it to your 
   // process address space. Note that if the shared memory 
   // object doesn't exist this function will exit with an error (errno set).
   //  name    Name of the shared memory object. For portability, the name
   //          should begin with a leading "/" and contain no other 
   //          "/" characters.
   //  size    Size (number of bytes) of the shared memory object
   //  return  If successful, a pointer to the starting memory location 
   //          of shared memory, else NULL, 'errno' is set and can be 
   //          retrieved by a call to getErrnoError().
   
  int close();
   // Unmaps the shared memory from process address space 
   // and closes the memory region to further access. The shared memory 
   // region and its contents are however not deleted and can be opened 
   // again (similar to a file open and close operation).
   //  return  0 on sucess, -1 on error, 'errno' is set and can be 
   //          retrieved by a call to getErrnoError().
   
  int unlink();
   // If the process calling this function created the shared memory, this 
   // function calls close() and marks the shared memory for deletion. 
   // Shared memory objects are persistant, i.e., if there are processes that 
   // have the object open when this function is called then the destruction 
   // of the object is delayed until all processes have closed their access 
   // to the object (by calling close()).
   //  return  0 on sucess, -1 on error, 'errno' is set and can be 
   //          retrieved by a call to getErrnoError(). NOTE: This function
   //          will return -1 if it is called by an object that did not 
   //          create the shared memory. 
   
  inline int getErrnoError() const;
   //  return  Error code from the last error that happened before
   //          this function was called, else 0. Error codes are defined
   //          in the standard header errno.h
   
  //======== END OF INTERFACE ========

 private:

  int d_shmFd;
   // File descriptor for open shared memory

  char *d_shmName;
   // Name of the shared memory object.
  
  int d_shmSize;
   // Size of shared memory
  
  void *d_shmMem;
   // The address of shared memory mapped to my process address space
   
  bool d_isOwner;
   // true if this object created the shared memory, false if 
   // this object only opened an existing shared memory.
  
  int d_errno;
   // Any standard errors are stored here.
  
};


//==============================================================================
// ShMem::getErrnoError
//==============================================================================
int ShMem::getErrnoError() const
{
 return d_errno;
}

#endif // _SHMEM_HPP_INCLUDED
