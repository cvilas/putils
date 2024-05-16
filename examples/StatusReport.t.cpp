//==============================================================================
// StatusReport.t.cpp - Example program for StatusReport class.
//
// Author        : Vilas Kumar Chitrakaran
//==============================================================================

#include "StatusReport.hpp"
#include <iostream>
#include <unistd.h>

using namespace std;

// use message
void usage(char *argv[])
{
 cout << endl
      << "Usage: " << argv[0] << " -[cl]" << endl
      << " where:" << endl
      << "  -c		use circular buffer" << endl
      << "  -l		use linear buffer" << endl << endl;
}

//==============================================================================
// main function
//==============================================================================
int main(int argc, char *argv[])
{
 StatusReport *buffer;
 char timeBuf[26];
 time_t time;
 unsigned int i;
 int opt;
 SR_buffer_type type = SR_CIRCULAR;
 
 // check command line arguments
 if(argc <= 1)
 {
  usage(argv);
  return 0;
 }
  
 // parse command line option for type of buffer
 while( (opt = getopt(argc, argv, "cl")) != -1)
 {
  switch(opt)
  {
   case 'c': // circular buffer
    type = SR_CIRCULAR;
    break;
   case 'l': // linear buffer
    type = SR_LINEAR;
    break;
   default:
    usage(argv);
    return 0;
    break;
  }
 } 
 
 // create buffer for 2 messages 80 chars long
 buffer = new StatusReport(80,2,type);

 // Add two error messages
 buffer->setReport(0x100);
 buffer->setReport(0x0, "Second message");
 
 // Add another message - this will overflow
 // for linear buffer and replace first message
 // for a circular buffer
 buffer->setReport(0x300, "Third message");
 
 // Print the reports 
 cout << "num reports: " << buffer->getNumReports() << endl
      << "num overflow: " << buffer->getNumReportsOverflow() << endl;
 for(i = 1; i <= buffer->getNumReports(); i++)
 {
  time = (time_t)buffer->getReportTimestamp(i).tv_sec;
  ctime_r(&time, timeBuf);
  timeBuf[24] = '\0';
  cout << "[Report: " << (dec) << i 
       << "] [mesg: " << buffer->getReportMessage(i) 
       << "] [code: " << "0x" << (hex) << buffer->getReportCode(i) 
       << "] [time: " << timeBuf << "]" << endl;
 }
 
 delete buffer;

 return 0;
}
