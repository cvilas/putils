//==============================================================================
// RWLock.t.cpp - Example program for RWLock class.
//
// Author        : Vilas Kumar Chitrakaran
//==============================================================================

#include "RWLock.hpp"
#include <iostream>
#include <math.h>
#include <stdlib.h>

using namespace std;

// A structure for checking accounts
typedef struct 
{
 double balance;
 RWLock key;
}bank_account;


// My checking account
bank_account myAccnt;


//========================================================================  
// balance
//========================================================================  
void *balance(void *)
{
 while(1)
 {
  if( myAccnt.key.tryReadLock() == -1 )
   cout << "Account info: Busy updating" << endl;
  else
  {
   cout << "Account info: $ " << myAccnt.balance << endl;
   myAccnt.key.unlock();
  }
 }
 return NULL;
}


//========================================================================  
// credit
//========================================================================  
void credit(double amount)
{
 myAccnt.key.writeLock();
 myAccnt.balance += amount;
 myAccnt.key.unlock();
}


//========================================================================  
// main function
//========================================================================  
int main()
{
 double amnt;
 pthread_t threadId;
 pthread_create(&threadId, NULL, balance, NULL);
 while(1)
 {
  amnt = 10.0 * (rand()/(double)RAND_MAX - 0.5);
  credit(amnt);
 }
 return 0;
}

