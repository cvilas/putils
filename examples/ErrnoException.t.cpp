//==============================================================================
// ErrnoException.t.cpp - Example program for ErrnoException class.
//
// Author        : Vilas Kumar Chitrakaran
//==============================================================================

//==============================================================================
// NOTE: For older versions of gcc ( <= 2.96 ) make sure that you compile with 
//       exceptions enabled (-fexceptions switch) to avoid SIGABRT on exception.
//==============================================================================

#include "ErrnoException.hpp"
#include <iostream>
#include <string>

using namespace std;

// Just an example function that shows how 
// to throw an exception
void enterNumberBelowFive(int number)
{
 // throw an exception for invalid argument
 if(number >= 5)
  throw ErrnoException(EINVAL,"[enterNumberBelowFive]");
}


// The main function
int main()
{
 
 // Try something that may cause error
 try
 {
  cout << "First call to enterNumberBelowFive() with arg = 2 ... ";
  enterNumberBelowFive(2);    // this will go through
  cout << "worked." << endl;
  
  cout << "Second call to enterNumberBelowFive() with arg = 10 ... ";
  enterNumberBelowFive(10);   // this will throw exception  
  cout << "worked." << endl;  // this line should not print
 }

 // catch the first error that was thrown from within try block....
 catch(ErrnoException ex)
 {
  cout << "caught exception: " << ex.getErrorDesc() << ": " 
       << strerror(ex.getErrorCode()) << endl;
  
  // put error recovery code here, based on type of exception!
 }
 
 return 0;
}

