//==============================================================================
// StatusReport.cpp - Status message reporting.
//
// Author        : Vilas Kumar Chitrakaran
// Version       : 2.0 (Apr 2005)
// Compatibility : POSIX, GCC
//==============================================================================

#include "StatusReport.hpp"
#include <cstring>

//==============================================================================
// StatusReport::StatusReport()
//==============================================================================
StatusReport::StatusReport(int maxMsgLen, int maxNumMsgs, SR_buffer_type type)
{
 init(maxMsgLen, maxNumMsgs, type);
}


//==============================================================================
// StatusReport::~StatusReport()
//==============================================================================
StatusReport::~StatusReport()
{
 for(unsigned int i = 0; i < d_maxNumMsgs; i++)
  free(d_messages[i].message);
 
 free(d_messages);
}


//==============================================================================
// StatusReport::init()
//==============================================================================
void StatusReport::init(int maxMsgLen, int maxNumMsgs, SR_buffer_type type)
{
 d_bufferType = type;
 d_maxNumMsgs = 0;
 d_maxMsgLen = maxMsgLen;
 d_messages = NULL;
 d_reportNum = 0;

 // Allocate report array.
 d_messages = 
  (status_message_t *) calloc(maxNumMsgs, sizeof(status_message_t));
 
 if(d_messages == NULL)
  return;
 
 // Allocate memory for each message - one extra space for terminating \0.
 for(int i = 0; i < maxNumMsgs; i++)
 {
  if( (d_messages[i].message = (char *) calloc((maxMsgLen+1), sizeof(char))) == NULL)
   return;
  d_maxNumMsgs++;
 }

 return;
}


//==============================================================================
// StatusReport::setReport()
//==============================================================================
void StatusReport::setReport(int code, const char *message)
{
 d_reportNum++;
 char empty = '\0';

 if(d_bufferType == SR_LINEAR && d_reportNum > d_maxNumMsgs)
  return;
 
 int index = (d_reportNum-1) % d_maxNumMsgs;
 
 // note time of report
 clock_gettime(CLOCK_REALTIME, &d_messages[index].time);

 // copy message string
 if(message)
  strncpy(d_messages[index].message, message, d_maxMsgLen);
 else
  strncpy(d_messages[index].message, &empty, 1);

 // terminate message string
 d_messages[index].message[d_maxMsgLen] = '\0';

 // copy message code
 d_messages[index].code = code;
}


//==============================================================================
// StatusReport::getReportMessage()
//==============================================================================
const char *StatusReport::getReportMessage(unsigned int rN) const
{ 
 unsigned int numReports = getNumReports();
 if(rN > numReports || rN < 1)
 {
  return ("\0");
 }
 if(d_bufferType == SR_LINEAR)
  return (d_messages[(numReports - rN)].message);
 return(d_messages[(d_reportNum - rN) % d_maxNumMsgs].message);
}


//==============================================================================
// StatusReport::getReportCode()
//==============================================================================
int StatusReport::getReportCode(unsigned int rN) const
{ 
 unsigned int numReports = getNumReports();
 if(rN > numReports || rN < 1)
 {
  return (0);
 }
 if(d_bufferType == SR_LINEAR)
  return (d_messages[(numReports - rN)].code);
 return (d_messages[(d_reportNum - rN) % d_maxNumMsgs].code);
}


//==============================================================================
// StatusReport::getReportTimestamp()
//==============================================================================
struct timespec StatusReport::getReportTimestamp(unsigned int rN) const
{ 
 struct timespec time = {0,0};
 unsigned int numReports = getNumReports();

 if(rN > numReports || rN < 1)
  return (time);
 if(d_bufferType == SR_LINEAR)
  return (d_messages[(numReports - rN)].time);
 return (d_messages[(d_reportNum - rN) % d_maxNumMsgs].time);
}


//==============================================================================
// StatusReport::clearReports()
//==============================================================================
void StatusReport::clearReports()
{
 d_reportNum = 0;
}


//==============================================================================
// StatusReport::getNumReportsOverflow()
//==============================================================================
unsigned int StatusReport::getNumReportsOverflow() const
{
 if(d_reportNum > d_maxNumMsgs)
  return(d_reportNum - d_maxNumMsgs);
 return (0);
}


//==============================================================================
// StatusReport::getNumReports()
//==============================================================================
unsigned int StatusReport::getNumReports() const
{
 if(d_reportNum <= d_maxNumMsgs)
  return(d_reportNum);
 return d_maxNumMsgs;
}
