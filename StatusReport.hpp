//==============================================================================
// StatusReport.hpp - Status message reporting.
//
// Author        : Vilas Kumar Chitrakaran
// Version       : 2.0 (Apr 2005)
// Compatibility : POSIX, GCC
//==============================================================================

#ifndef _STATUSREPORT_HPP_INCLUDED
#define _STATUSREPORT_HPP_INCLUDED

#include <string>
#include <stdlib.h>
#include <time.h>

//==============================================================================
// enum SR_buffer_type
//==============================================================================
enum SR_buffer_type
{
 SR_LINEAR = 0,
 SR_CIRCULAR,
};


//==============================================================================
// class StatusReport
//------------------------------------------------------------------------------
// \brief
// An object for storing status messages containing a integer code, a 
// message and a timestamp.
//
// Objects of this class can be embedded in any software module that requires
// status reporting capabilities to users or other connected subsystems. Notes 
// about usability to your application:
// <ul>
// <li>User must specify the maximum number of reports to store and maximum 
//     possible length of reports apriori. 
// <li>Reports longer than maximum specified report length will get truncated. 
// <li>If the buffer is initialized as circular, once the buffer is full new 
//     messages will overwrite oldest messages from the beginning of the buffer 
//     so that if the buffer size is 'n', you will always have the last 'n' 
//     messages available to you.  
// <li>The class constructor does dynamic memory allocation. Create objects 
//     of this class outside realtime code.
// </ul>
//
// <b>Example Program:</b>
//
// \include StatusReport.t.cpp
//==============================================================================

class StatusReport
{
 public:
  StatusReport(int maxMsgLen = 80, int maxNumMsgs = 1, 
                      SR_buffer_type type = SR_CIRCULAR);
   // The constructor. Creates required buffers and initializes.
   //  maxMsgLen  Maximum length of messages in the buffer (default = 80 chars).
   //  maxNumMsgs Number of messages the buffer can hold (default = 1).
   //  type       The type of buffer, either circular (SR_CIRCULAR) or 
   //             linear (SR_LINEAR). If the buffer is initialized as circular, 
   //             once the buffer is full new messages will overwrite oldest 
   //             messages from the beginning of the buffer so that if the 
   //             buffer size is 'n', you will always have the last 'n' 
   //             messages available to you (default SR_CIRCULAR). 
  
  ~StatusReport();
   // The destructor. Frees allocated memory.
  
  void setReport(int code, const char *message = NULL);
   // Add a report message and code to the buffer.
   //  code     An integer code. 
   //  message  The message to add to the report.

  const char *getReportMessage(unsigned int reportNum = 1) const;
   // Return a report message.
   //  reportNum  The desired report number for which 
   //             you want the message. Note that reports are 
   //             collected first-in last out. Hence if no 
   //             report number is specified, the most recent 
   //             report is returned; i.e. report number 1 is 
   //             most recent.
  
  int getReportCode(unsigned int reportNum = 1) const;
   // Return a report code. 
   //  reportNum  The desired report number for which 
   //             you want the message. Note that reports are 
   //             collected first-in last out. Hence if no 
   //             report number is specified, the most recent 
   //             report is returned; i.e. report number 1 is 
   //             most recent.

  struct timespec getReportTimestamp(unsigned int reportNum = 1) const;
   // Return the timestamp when a report was received. 
   //  reportNum  The desired report number for which 
   //             you want the message. Note that reports are 
   //             collected first-in last out. Hence if no 
   //             report number is specified, the most recent 
   //             report is returned; i.e. report number 1 is 
   //             most recent.

  void clearReports();
   // Clear all reports.
  
  unsigned int getNumReports() const;
   // Return the number of reports available to the user from
   // the reports buffer.
  
  unsigned int getNumReportsOverflow() const;
   // Return the number of reports that were not recorded
   // because of insufficient buffer space. If the buffer is circular 
   // this function will return number of messages lost because they were 
   // overwritten. 

  //======== END OF INTERFACE ========
 
 private:
  void init(int maxMsgLen, int maxNumMsgs, SR_buffer_type type);
   // Initialize or resize the message buffers
   //  maxMsgLen  Maximum length of messages in the buffer.
   //  maxNumMsgs Number of messages the buffer can hold.
   //  type       The type of buffer, either circular (SR_CIRCULAR) or 
   //             linear (SR_LINEAR). If the buffer is initialized as circular, 
   //             once the buffer is full new messages will overwrite oldest 
   //             messages from the beginning of the buffer so that if the 
   //             buffer size is 'n', you will always have the last 'n' 
   //             messages available to you. 

  typedef struct status_message
  { 
   int code;
   char *message;
   struct timespec time;
  }status_message_t;
 
  status_message_t *d_messages; // the reports
  SR_buffer_type d_bufferType;  // circular or linear buffer
  unsigned int d_reportNum;     // number of available reports
  unsigned int d_maxMsgLen;     // max length of reports
  unsigned int d_maxNumMsgs;    // number of reports
};


#endif // _STATUSREPORT_HPP_INCLUDED
