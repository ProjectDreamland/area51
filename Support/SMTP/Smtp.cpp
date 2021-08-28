/*
Module : SMTP.CPP
Purpose: Implementation for a MFC class encapsulation of the SMTP protocol
Created: PJN / 22-05-1998
History: PJN / 15-06-1998 1) Fixed the case where a single dot occurs on its own
                             in the body of a message
                                  2) Class now supports Reply-To Header Field
                          3) Class now supports file attachments
           PJN / 18-06-1998 1) Fixed a memory overwrite problem which was occurring 
                             with the buffer used for encoding base64 attachments
         PJN / 27-06-1998 1) The case where a line begins with a "." but contains
                          other text is now also catered for. See RFC821, Section 4.5.2
                          for further details.
                          2) m_sBody in CSMTPMessage has now been made protected.
                          Client applications now should call AddBody instead. This
                          ensures that FixSingleDot is only called once even if the 
                          same message is sent a number of times.
                          3) Fixed a number of problems with how the MIME boundaries
                          were defined and sent.
                          4) Got rid of an unreferenced formal parameter 
                          compiler warning when doing a release build
         PJN / 11-09-1998 1) VC 5 project file is now provided
                          2) Attachment array which the message class contains now uses
                          references instead of pointers.
                          3) Now uses Sleep(0) to yield our time slice instead of Sleep(100),
                          this is the preferred way of writting polling style code in Win32
                          without serverly impacting performance.
                          4) All Trace statements now display the value as returned from
                          GetLastError
                          5) A number of extra asserts have been added
                          6) A AddMultipleRecipients function has been added which supports added a 
                          number of recipients at one time from a single string
                          7) Extra trace statements have been added to help in debugging
         PJN / 12-09-98   1) Removed a couple of unreferenced variable compiler warnings when code
                          was compiled with Visual C++ 6.0
                          2) Fixed a major bug which was causing an ASSERT when the CSMTPAttachment
                          destructor was being called in the InitInstance of the sample app. 
                          This was inadvertingly introduced for the 1.2 release. The fix is to revert 
                          fix 2) as done on 11-09-1998. This will also help to reduce the number of 
                          attachment images kept in memory at one time.
         PJN / 18-01-99   1) Full CC & BCC support has been added to the classes
         PJN / 22-02-99   1) Addition of a Get and SetTitle function which allows a files attachment 
                          title to be different that the original filename
                          2) AddMultipleRecipients now ignores addresses if they are empty.
                          3) Improved the reading of responses back from the server by implementing
                          a growable receive buffer
                          4) timeout is now 60 seconds when building for debug
         PJN / 25-03-99   1) Now sleeps for 250 ms instead of yielding the time slice. This helps 
                          reduce CPU usage when waiting for data to arrive in the socket
         PJN / 14-05-99   1) Fixed a bug with the way the code generates time zone fields in the Date headers.
         PJN / 10-09-99   1) Improved CSMTPMessage::GetHeader to include mime field even when no attachments
                          are included.
         PJN / 16-02-00   1) Fixed a problem which was occuring when code was compiled with VC++ 6.0.
         PJN / 19-03-00   1) Fixed a problem in GetHeader on Non-English Windows machines
                          2) Now ships with a VC 5 workspace. I accidentally shipped a VC 6 version in one of the previous versions of the code.
                          3) Fixed a number of UNICODE problems
                          4) Updated the sample app to deliberately assert before connecting to the author's SMTP server.
         PJN / 28-03-00   1) Set the release mode timeout to be 10 seconds. 2 seconds was causing problems for slow dial
                          up networking connections.
         PJN / 07-05-00   1) Addition of some ASSERT's in CSMTPSocket::Connect
             PP  / 16-06-00   The following modifications were done by Puneet Pawaia
                                      1) Removed the base64 encoder from this file
                                      2) Added the base64 encoder/decoder implementation in a separate 
                                      file. This was done because base64 decoding was not part of 
                                        the previous implementation
                                      3) Added support for ESMTP connection. The class now attempts to 
                                      authenticate the user on the ESMTP server using the username and
                                        passwords supplied. For this connect now takes the username and 
                                        passwords as parameters. These can be null in which case ESMTP 
                                        authentication is not attempted
                                      4) This class can now handle AUTH LOGIN and AUTH LOGIN PLAIN authentication
                                      schemes on 
             PP  / 19-06-00   The following modifications were done by Puneet Pawaia
                                      1) Added the files md5.* containing the MD5 digest generation code
                                      after modifications so that it compiles with VC++ 6
                                      2) Added the CRAM-MD5 login procedure.
         PJN / 10-07-00   1) Fixed a problem with sending attachments > 1K in size
                          2) Changed the parameters to CPJNSMTPConnection::Connect
         PJN / 30-07-00   1) Fixed a bug in AuthLogin which was transmitting the username and password
                          with an extra "=" which was causing the login to failure. Thanks to Victor Vogelpoel for
                          finding this.
         PJN / 05-09-00   1) Added a CSMTP_NORSA preprocessor macro to allow the CPJNSMTPConnection code to be compiled
                          without the dependence on the RSA code.
         PJN / 28-12-2000 1) Removed an unused variable from ConnectESMTP.
                          2) Allowed the hostname as sent in the HELO command to be specified at run time 
                          in addition to using the hostname of the client machine
                          3) Fixed a problem where high ascii characters were not being properly encoded in
                          the quoted-printable version of the body sent.
                          4) Added support for user definable charset's for the message body.
                          5) Mime boundaries are now always sent irrespective if whether attachments are included or
                          not. This is required as the body is using quoted-printable.
                          6) Fixed a bug in sendLines which was causing small message bodies to be sent incorrectly
                          7) Now fully supports custom headers in the SMTP message
                          8) Fixed a copy and paste bug where the default port for the SMTP socket class was 110.
                          9) You can now specify the address on which the socket is bound. This enables the programmer
                          to decide on which NIC data should be sent from. This is especially useful on a machine
                          with multiple IP addresses.
                          10) Addition of functions in the SMTP connection class to auto dial and auto disconnect to 
                          the Internet if you so desire.
                          11) Sample app has been improved to allow Auto Dial and binding to IP addresses to be configured.
         PJN / 26-02-2001 1)  Charset now defaults to ISO 8859-1 instead of us-ascii
                          2)  Removed a number of unreferrenced variables from the sample app.
                          3)  Headers are now encoded if they contain non ascii characters.
                          4)  Fixed a bug in getLine, Thanks to Lev Evert for spotting this one.
                          5)  Made the charset value a member of the message class instead of the connection class
                          6)  Sample app now fully supports specifying the charset of the message
                          7)  Added a AddMultipleAttachments method to CSMTPMessage
                          8)  Attachments can now be copied to each other via new methods in CSMTPAttachment
                          9)  Message class now contains copies of the attachments instead of pointers to them
                          10) Sample app now allows multiple attachments to be added
                          11) Removed an unnecessary assert in QuotedPrintableEncode
                          12) Added a Mime flag to the CSMTPMessage class which allows you to decide whether or not a message 
                          should be sent as MIME. Note that if you have attachments, then mime is assumed.
                          13) CSMTPAttachment class has now become CSMTPBodyPart in anticipation of full support for MIME and 
                          MHTML email support
                          14) Updated copright message in source code and documentation
                          15) Fixed a bug in GetHeader related to _tsetlocale usage. Thanks to Sean McKinnon for spotting this
                          problem.
                          16) Fixed a bug in SendLines when sending small attachments. Thanks to Deng Tao for spotting this
                          problem.
                          17) Removed the need for SendLines function entirely.
                          18) Now fully supports HTML email (aka MHTML)
                          19) Updated copyright messages in code and in documentation
         PJN / 17-06-2001 1) Fixed a bug in CSMTPMessage::HeaderEncode where spaces were not being interpreted correctly. Thanks
                          to Jim Alberico for spotting this.
                          2) Fixed 2 issues with ReadResponse both having to do with multi-line responses. Thanks to Chris Hanson 
                          for this update.
         PJN / 25-06-2001 1) Code now links in Winsock and RPCRT40 automatically. This avoids client code having to specify it in 
                          their linker settings. Thanks to Malte and Phillip for spotting this issue.
                          2) Updated sample code in documentation. Thanks to Phillip for spotting this.
                          3) Improved the code in CSMTPBodyPart::SetText to ensure lines are correctly wrapped. Thanks to 
                          Thomas Moser for this fix.
         PJN / 01-07-2001 1) Modified QuotedPrintableEncode to prevent the code to enter in an infinite loop due to a long word i.e. 
                          bigger than SMTP_MAXLINE, in this case, the word is breaked. Thanks to Manuel Gustavo Saraiva for this fix.
                          2) Provided a new protected variable in CSMTPBodyPart called m_bQuotedPrintable to bypass the 
                          QuotedPrintableEncode function in cases that we don't want that kind of correction. Again thanks to 
                          Manuel Gustavo Saraiva for this fix.
         PJN / 15-07-2001 1) Improved the error handling in the function CSMTPMessage::AddMultipleAttachments. In addition the 
                          return value has been changed from BOOL to int
         PJN / 11-08-2001 1) Fixed a bug in QuotedPrintableEncode which was wrapping encoding characters across multiple lines. 
                          Thanks to Roy He for spotting this.
                          2) Provided a "SendMessage" method which sends a email directly from disk. This allows you 
                          to construct your own emails and the use the class just to do the sending. This function also has the 
                          advantage that it efficiently uses memory and reports progress.
                          3) Provided support for progress notification and cancelling via the "OnSendProgress" virtual method.
         PJN / 29-09-2001 1) Fixed a bug in ReadResponse which occured when you disconnected via Dial-Up Networking
                          while a connection was active. This was originally spotted in my POP3 class.
                          2) Fixed a problem in CSMTPBodyPart::GetHeader which was always including the "quoted-printable" even when 
                          m_bQuotedPrintable for that body part was FALSE. Thanks to "jason" for spotting this.
         PJN / 12-10-2001 1) Fixed a problem where GetBody was reporting the size as 1 bigger than it should have been. Thanks
                          to "c f" for spotting this problem.
                          2) Fixed a bug in the TRACE statements when a socket connection cannot be made.
                          3) The sample app now displays a filter of "All Files" when selecting attachments to send
                          4) Fixed a problem sending 0 byte attachments. Thanks to Deng Tao for spotting this problem.
         PJN / 11-01-2002 1) Now includes a method to send a message directly from memory. Thanks to Tom Allebrandi for this
                          suggestion.
                          2) Change function name "IsReadible" to be "IsReadable". I was never very good at English!.
                          3) Fixed a bug in CSMTPBodyPart::QuotedPrintableEncode. If a line was exactly 76 characters long plus 
                          \r\n it produced an invalid soft linebreak of "\r=\r\n\n". Thanks to Gerald Egert for spotting this problem.
         PJN / 29-07-2002 1) Fixed an access violation in CSMTPBodyPart::QuotedPrintableEncode. Thanks to Fergus Gallagher for 
                          spotting this problem.
                          2) Fixed a problem where in very rare cases, the QuotedPrintableEncode function produces a single dot in a 
                          line, when inserting the "=" to avoid the mail server's maxline limit. I inserted a call to FixSingleDot 
                          after calling the QuotedPrintableEncode function in GetBody. Thanks to Andreas Kappler for spotting this
                          problem.
                          3) Fixed an issue in CSMTPBodyPart::GetHeader where to ensure some mail clients can correctly handle
                          body parts and attachments which have a filename with spaces in it. Thanks to Andreas kappler for spotting
                          this problem.
                          4) QP encoding is now only used when you specify MIME. This fixes a bug as reported by David Terracino.
                          5) Removed now unused "API Reference" link in HTML file supported the code.
         PJN / 10-08-2002 1) Fixed a number of uncaught file exceptions in CSMTPBodyPart::GetBody (Tick), CSMTPMessage::SaveToDisk (Tick), and 
                          CPJNSMTPConnection::SendMessage. Thanks to John Allan Miller for reporting this problem.
                          2) The CPJNSMTPConnection::SendMessage version of the method which sends a file directly from disk, now fails if the
                          file is empty.
                          3) Improved the sample app by displaying a wait cursor while a message from file is being sent
                          4) Improved the sample app by warning the user if mail settings are missing and then bringing up the configuration
                          dialog.
         PJN / 20-09-2002 1) Fixed a problem where the code "Coder.EncodedMessage" was not being converted from an ASCII string to a UNICODE 
                          string in calls to CString::Format. This was occurring when sending the username and password for "AUTH LOGIN" support
                          in addition to sending the "username digest" for "AUTH CRAM-MD5" support. Thanks to Serhiy Pavlov for reporting this
                          problem.
                          2) Removed a number of calls to CString::Format and instead replaced with string literal CString constructors instead.
         PJN / 03-10-2002 1) Quoted printable encoding didn't work properly in UNICODE. (invalid conversion from TCHAR to BYTE). Thanks to
                          Serhiy Pavlov for reporting this problem.
                          2) Subject encoding didn't work properly in UNICODE. (invalid conversion from TCHAR to BYTE). Thanks to Serhiy Pavlov
                          for reporting this problem.
                          3) It didn't insert "charset" tag into root header for plain text messages (now it includes "charset" into plain text 
                          messages too). Thanks to Serhiy Pavlov for reporting this problem.
         PJN / 04-10-2002 1) Fixed an issue where the header / body separator was not being sent correctly for mails with attachments or when
                          the message is MIME encoded. Thanks to Serhiy Pavlov for reporting this problem.
                          2) Fixed a bug in QuotedPrintableEncode and HeaderEncode which was causing the errors. Thanks to Antonio Maiorano 
                          for reporting this problem.
                          3) Fixed an issue with an additional line separator being sent between the header and body of emails. This was only
                          evident in mail clients if a non mime email without attachments was sent.
         PJN / 11-12-2002 1) Review all TRACE statements for correctness
                          2) Provided a virtual OnError method which gets called with error information 
         PJN / 07-02-2003 1) Addition of a "BOOL bGracefully" argument to Disconnect so that when an application cancels the sending of a 
                          message you can pass FALSE and close the socket without properly terminating the SMTP conversation. Thanks to
                          "nabocorp" for this nice addition.
         PJN / 19-03-2003 1) Addition of copy constructors and operator= to CSMTPMessage class. Thanks to Alexey Vasilyev for this suggestion.
         PJN / 13-04-2003 1) Fixed a bug in the handling of EHLO responses. Thanks to "Senior" for the bug report and the fix.
         PJN / 16-04-2003 1) Enhanced the CSMTPAddress constructor to parse out the email address and friendly name rather than assume
                          it is an email address.
                          2) Reworked the syntax of the CSMTPMessage::ParseMultipleRecipients method. Also now internally this function
                          uses the new CSMTPAddress constructor
         PJN / 19-04-2003 1) Fixed a bug in the CSMTPAddress constructor where I was mixing up the friendly name in the "<>" separators,
                          when it should have been the email address. 
         PJN / 04-05-2003 1) Fixed an issue where the class doesn't convert the mail body and subject to the wanted encoding but rather changes 
                          the encoding of the email in the email header. Since the issue of supporting several languages is a complicated one 
                          I've decided that we could settle for sending all CPJNSMTPConnection emails in UTF-8. I've added some conversion functions 
                          to the class that - at this point - always converts the body and subject of the email to UTF-8. A big thanks to Itamar 
                          Kerbel for this nice addition.
                          2) Moved code and sample app to VC 6.
         PJN / 05-05-2003 1) Reworked the way UTF8 encoding is now done. What you should do if you want to use UTF-8 encoding is set the charset
                          to "UTF-8" and use either QP or base 64 encoding.
                          2) Reworked the automatic encoding of the subject line to use the settings as taken from the root SMTP body part
                          3) Only the correct headers according to the MIME RFC are now encoded.
                          4) QP encoding is the encoding mechanism now always used for headers.
                          5) Headers are now only encoded if required to be encoded.
         PJN / 12-05-2003 1) Fixed a bug where the X-Mailer header was being sent incorrectly. Thanks to Paolo Vernazza for reporting this problem.
                          2) Addition of X-Mailer header is now optional. In addition sample app now does not send the X-Mailer header.
                          3) The sample app now does not send the Reply-To header.
         PJN / 18-08-2003 1) Modified the return value from the ConnectToInternet method. Instead of it being a boolean, it is now an enum. This
                          allows client code to differentiate between two conditions that it couldn't previously, namely when an internet connection
                          already existed or if a new connection (presumably a dial-up connection was made). This allows client code to then
                          disconnect if a new connection was required. Thanks to Pete Arends for this nice addition.
         PJN / 05-10-2003 1) Reworked the CPJNSMTPConnection::ReadResponse method to use the timeout provided by IsReadable rather than calling sleep.
                          Thanks to Clarke Brunt for reporting this issue.
         PJN / 03-11-2003 1) Simplified the code in CPJNSMTPConnection::ReadResponse. Thanks to Clarke Brunt for reporting this issue.
         PJN / 03-12-2003 1) Made code which checks the Login responses which contain "Username" and "Password" case insensitive. Thanks to 
                          Zhang xiao Pan for reporting this problem.
         PJN / 11-12-2003 1) Fixed an unreferrenced variable in CPJNSMTPConnection::OnError as reported by VC.Net 2003
                          2) DEBUG_NEW macro is now only used on VC 6 and not on VC 7 or VC 7.1. This avoids a problem on these compilers
                          where a conflict exists between it and the STL header files. Thanks to Alex Evans for reporting this problem.
         PJN / 31-01-2004 1) Fixed a bug in CSMTPBodyPart::GetBody where the size of the body part was incorrectly calculating the size if the
                          encoded size was an exact multiple of 76. Thanks to Kurt Emanuelson and Enea Mansutti for reporting this problem.
         PJN / 07-02-2004 1) Fixed a bug in CSMTPBodyPart::SetText where the code would enter an endless loop in the Replace function. It has now 
                          been replaced with CString::Replace. This now means that the class will not now compile on VC 5. Thanks to Johannes Philipp 
                          Grohs for reporting this problem.
                          2) Fixed a number of warnings when the code is compiled with VC.Net 2003. Thanks to Edward Livingston for reporting
                          this issue.
         PJN / 18-02-2004 1) You can now optionally set the priority of an email thro the variable CSMTPMessage::m_Priority. Thanks to Edward 
                          Livingston for suggesting this addition.
         PJN / 04-03-2004 1) To avoid conflicts with the ATL Server class of the same name, namely "CSMTPConnection", the class is now called 
                          "CPJNSMTPConnection". To provide for easy upgrading of code, "CSMTPConnection" is now defined to be "CPJSMTPConnection" 
                          if the code detects that the ATL Server SMTP class is not included. Thanks to Ken Jones for reporting this issue.
         PJN / 13-03-2004 1) Fixed a problem where the CSMTPBodyPart::m_dwMaxAttachmentSize value was not being copied in the CSMTPBodyPart::operator=
                          method. Thanks to Gerald Egert for reporting this problem and the fix.



Copyright (c) 1998 - 2004 by PJ Naughter.  (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

Please note that I have been informed recently that C(PJN)SMTPConnection is being used to develop and send unsolicted bulk mail. 
This was not the intention of the code and the author explicitly forbids use of the code for any software of this kind without 
my explicit written consent.

*/


//////////////// Includes ////////////////////////////////////////////

#include "stdafx.h"
#include "smtp.h"

#ifndef CSMTP_NORSA
#include "glob-md5.h"
#include "md5.h"
#endif

#ifndef _WININET_
//#pragma message("To avoid this message, put wininet.h in your PCH (usually stdafx.h)")
#include <wininet.h>
#endif



//////////////// Macros / Locals /////////////////////////////////////
#if (_MFC_VER < 0x700)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

#define SMTP_MAXLINE  76

//Link in Winsock dll automatically
#pragma comment(lib, "wsock32.lib")
//Link in RPC runtimes dll automatically
#pragma comment(lib, "rpcrt4.lib")



//////////////// Implementation //////////////////////////////////////

//Class which handles function which must be constructed at run time
//since we cannot absolutely gurantee wininet will be available. To avoid the loader
//bringing up a message such as "Failed to load due to missing export...", the
//functions are constructed using GetProcAddress. The SMTP functions then checks to 
//see if the function pointers are NULL and if it is it returns failure and sets the 
//error code ERROR_CALL_NOT_IMPLEMENTED which is what the OS would have done if it had 
//implemented a stub for it in the first place !!
class _WININET_DATA
{
public:
//Constructors /Destructors
  _WININET_DATA();
  ~_WININET_DATA();

//typedefs of the function pointers
  typedef BOOL (WINAPI INTERNETGETCONNECTEDSTATE)(LPDWORD, DWORD);
  typedef INTERNETGETCONNECTEDSTATE* LPINTERNETGETCONNECTEDSTATE;
  typedef BOOL (WINAPI INTERNETAUTODIALHANGUP)(DWORD);
  typedef INTERNETAUTODIALHANGUP* LPINTERNETAUTODIALHANGUP;
  typedef BOOL (WINAPI INTERNETATTEMPCONNECT)(DWORD);
  typedef INTERNETATTEMPCONNECT* LPINTERNETATTEMPCONNECT;


//Member variables
  HINSTANCE                   m_hWininet;  //Instance handle of the "Wininet.dll" which houses the 2 functions we want
  LPINTERNETGETCONNECTEDSTATE m_lpfnInternetGetConnectedState;
  LPINTERNETAUTODIALHANGUP    m_lpfnInternetAutoDialHangup;
  LPINTERNETATTEMPCONNECT     m_lpfnInternetAttemptConnect;
};

_WININET_DATA::_WININET_DATA()
{
  m_hWininet = LoadLibrary(_T("WININET.DLL"));
  if (m_hWininet)
  {
    m_lpfnInternetGetConnectedState = (LPINTERNETGETCONNECTEDSTATE) GetProcAddress(m_hWininet, "InternetGetConnectedState");
    m_lpfnInternetAutoDialHangup = (LPINTERNETAUTODIALHANGUP) GetProcAddress(m_hWininet, "InternetAutodialHangup");
    m_lpfnInternetAttemptConnect = (LPINTERNETATTEMPCONNECT) GetProcAddress(m_hWininet, "InternetAttemptConnect");
  }
}

_WININET_DATA::~_WININET_DATA()
{
  if (m_hWininet)
  {
    FreeLibrary(m_hWininet);
    m_hWininet = NULL;
  }
}



//The local variable which handle the function pointers

_WININET_DATA _WinInetData;



CSMTPSocket::CSMTPSocket()
{
  m_hSocket = INVALID_SOCKET; //default to an invalid scoket descriptor
}

CSMTPSocket::~CSMTPSocket()
{
  Close();
}

BOOL CSMTPSocket::Create()
{
  m_hSocket = socket(AF_INET, SOCK_STREAM, 0);
  return (m_hSocket != INVALID_SOCKET);
}

BOOL CSMTPSocket::Connect(LPCTSTR pszHostAddress, int nPort, LPCTSTR pszLocalBoundAddress)
{
  ASSERT(pszHostAddress); //Must have a valid host
  ASSERT(_tcslen(pszHostAddress)); //as above

    //For correct operation of the T2A macro, see MFC Tech Note 59
    USES_CONVERSION;

  //must have been created first
  ASSERT(m_hSocket != INVALID_SOCKET);

  //Bind to the local address if need be
  if (pszLocalBoundAddress && _tcslen(pszLocalBoundAddress))
  {
    LPSTR lpszAsciiLocalAddress = T2A((LPTSTR)pszLocalBoundAddress);

    SOCKADDR_IN sockLocalAddress;
    ZeroMemory(&sockLocalAddress, sizeof(sockLocalAddress));
    sockLocalAddress.sin_family = AF_INET;
    sockLocalAddress.sin_port = htons(0);
    sockLocalAddress.sin_addr.s_addr = inet_addr(lpszAsciiLocalAddress);

      //If the address is not dotted notation, then do a DNS 
      //lookup of it.
      if (sockLocalAddress.sin_addr.s_addr == INADDR_NONE)
      {
          LPHOSTENT lphost;
          lphost = gethostbyname(lpszAsciiLocalAddress);
          if (lphost != NULL)
              sockLocalAddress.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
          else
          {
        WSASetLastError(WSAEINVAL); 
              return FALSE;
          }
    }

    //Finally bind to the address
    if (bind(m_hSocket, (sockaddr*) &sockLocalAddress, sizeof(sockLocalAddress)) == SOCKET_ERROR)
      return FALSE;
  }

  
  //Work out the IP address of the machine we want to connect to
    LPSTR lpszAsciiDestination = T2A((LPTSTR)pszHostAddress);

    //Determine if the address is in dotted notation
    SOCKADDR_IN sockDestinationAddr;
    ZeroMemory(&sockDestinationAddr, sizeof(sockDestinationAddr));
    sockDestinationAddr.sin_family = AF_INET;
    sockDestinationAddr.sin_port = htons((u_short)nPort);
    sockDestinationAddr.sin_addr.s_addr = inet_addr(lpszAsciiDestination);

    //If the address is not dotted notation, then do a DNS 
    //lookup of it.
    if (sockDestinationAddr.sin_addr.s_addr == INADDR_NONE)
    {
        LPHOSTENT lphost;
        lphost = gethostbyname(lpszAsciiDestination);
        if (lphost != NULL)
            sockDestinationAddr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
        else
        {
      WSASetLastError(WSAEINVAL); 
            return FALSE;
        }
    }

    //Call the protected version which takes an address 
    //in the form of a standard C style struct.
    return Connect((SOCKADDR*)&sockDestinationAddr, sizeof(sockDestinationAddr));
}

BOOL CSMTPSocket::Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen)
{
    return (connect(m_hSocket, lpSockAddr, nSockAddrLen) != SOCKET_ERROR);
}

BOOL CSMTPSocket::Send(LPCSTR pszBuf, int nBuf)
{
  //must have been created first
  ASSERT(m_hSocket != INVALID_SOCKET);
  return (send(m_hSocket, pszBuf, nBuf, 0) != SOCKET_ERROR);
}

int CSMTPSocket::Receive(LPSTR pszBuf, int nBuf)
{
  //must have been created first
  ASSERT(m_hSocket != INVALID_SOCKET);

  return recv(m_hSocket, pszBuf, nBuf, 0); 
}

void CSMTPSocket::Close()
{
    if (m_hSocket != INVALID_SOCKET)
    {
        VERIFY(SOCKET_ERROR != closesocket(m_hSocket));
        m_hSocket = INVALID_SOCKET;
    }
}

BOOL CSMTPSocket::IsReadable(BOOL& bReadible, DWORD dwTimeout)
{
  timeval timeout;
  timeout.tv_sec = dwTimeout/1000;
  timeout.tv_usec = (dwTimeout%1000)*1000;
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(m_hSocket, &fds);
  int nStatus = select(0, &fds, NULL, NULL, &timeout);
  if (nStatus == SOCKET_ERROR)
  {
    return FALSE;
  }
  else
  {
    bReadible = !(nStatus == 0);
    return TRUE;
  }
}




CSMTPAddress::CSMTPAddress() 
{
}

CSMTPAddress::CSMTPAddress(const CSMTPAddress& address)
{
  *this = address;
}

CSMTPAddress::CSMTPAddress(const CString& sAddress)
{
  //The local variable which we will operate on
  CString sTemp(sAddress);
  sTemp.TrimLeft();
  sTemp.TrimRight();

    //divide the substring into friendly names and e-mail addresses
    int nMark = sTemp.Find(_T('<'));
    int nMark2 = sTemp.Find(_T('>'));
    if ((nMark != -1) && (nMark2 != -1) && (nMark2 > (nMark+1)))
    {
        m_sEmailAddress = sTemp.Mid(nMark+1, nMark2 - nMark - 1);
        m_sFriendlyName = sTemp.Left(nMark);
    m_sFriendlyName.TrimLeft();
    m_sFriendlyName.TrimRight();
  }
    else
    {
        m_sEmailAddress = sTemp;
    }
}

CSMTPAddress::CSMTPAddress(const CString& sFriendly, const CString& sAddress) : 
              m_sFriendlyName(sFriendly), m_sEmailAddress(sAddress) 
{
  ASSERT(m_sEmailAddress.GetLength()); //An empty address is not allowed
}

CSMTPAddress& CSMTPAddress::operator=(const CSMTPAddress& r) 
{ 
  m_sFriendlyName = r.m_sFriendlyName; 
    m_sEmailAddress = r.m_sEmailAddress; 
    return *this;
}

CString CSMTPAddress::GetRegularFormat(BOOL bEncode, const CString& sCharset) const
{
  ASSERT(m_sEmailAddress.GetLength()); //Email Address must be valid

  CString sAddress;
  if (m_sFriendlyName.IsEmpty())
    sAddress = m_sEmailAddress;  //Just transfer the address across directly
  else
  {
    if (bEncode)
    {
      std::string sAsciiEncodedFriendly = CSMTPBodyPart::HeaderEncode(m_sFriendlyName, sCharset);
      CString sEncodedFriendly(sAsciiEncodedFriendly.c_str());
      sAddress.Format(_T("%s <%s>"), sEncodedFriendly, m_sEmailAddress);
    }
    else
      sAddress.Format(_T("%s <%s>"), m_sFriendlyName, m_sEmailAddress);
  }

  return sAddress;
}




CSMTPBodyPart::CSMTPBodyPart() : m_sCharset(_T("iso-8859-1")), m_sContentType(_T("text/plain")), m_pParentBodyPart(NULL), m_bQuotedPrintable(TRUE), m_bBase64(FALSE), m_dwMaxAttachmentSize(52428800)
{
  //Automatically generate a unique boundary separator for this body part by creating a guid
  UUID uuid;
  UuidCreate(&uuid);
  
  //Convert it to a string
  #ifdef _UNICODE
  TCHAR* pszGuid = NULL;
  #else
  unsigned char* pszGuid = NULL;
  #endif
  UuidToString(&uuid, &pszGuid);

  m_sBoundary = pszGuid;

  //Free up the temp memory
  RpcStringFree(&pszGuid);
}

CSMTPBodyPart::CSMTPBodyPart(const CSMTPBodyPart& bodyPart)
{
  *this = bodyPart;
}

CSMTPBodyPart::~CSMTPBodyPart()
{
  //Free up the array memory
  for (int i=0; i<m_ChildBodyParts.GetSize(); i++)
    delete m_ChildBodyParts.GetAt(i);
  m_ChildBodyParts.RemoveAll();
}

CSMTPBodyPart& CSMTPBodyPart::operator=(const CSMTPBodyPart& bodyPart)
{
  m_sFilename           = bodyPart.m_sFilename;
  m_sText               = bodyPart.m_sText;       
  m_sTitle              = bodyPart.m_sTitle;      
  m_sContentType        = bodyPart.m_sContentType;
  m_sCharset            = bodyPart.m_sCharset;
  m_sContentBase        = bodyPart.m_sContentBase;
  m_sContentID          = bodyPart.m_sContentID;
  m_sContentLocation    = bodyPart.m_sContentLocation;
  m_pParentBodyPart     = bodyPart.m_pParentBodyPart;
  m_sBoundary           = bodyPart.m_sBoundary;
  m_bQuotedPrintable    = bodyPart.m_bQuotedPrintable;
  m_bBase64             = bodyPart.m_bBase64;
  m_dwMaxAttachmentSize = bodyPart.m_dwMaxAttachmentSize;

  //Free up the array memory
  for (int i=0; i<m_ChildBodyParts.GetSize(); i++)
    delete m_ChildBodyParts.GetAt(i);
  m_ChildBodyParts.RemoveAll();
  //Now copy over the new object
  for (i=0; i<bodyPart.m_ChildBodyParts.GetSize(); i++)
  {
    CSMTPBodyPart* pBodyPart = new CSMTPBodyPart(*bodyPart.m_ChildBodyParts.GetAt(i));
    pBodyPart->m_pParentBodyPart  = this;
    m_ChildBodyParts.Add(pBodyPart);
  }

  return *this;
}

BOOL CSMTPBodyPart::SetFilename(const CString& sFilename)
{
  ASSERT(sFilename.GetLength());  //Empty Filename !

  //determine the file size
  CFileStatus fs;
  if (!CFile::GetStatus(sFilename, fs))
  {
    TRACE(_T("CSMTPBodyPart::SetFilename, Failed to get the status for file %s, probably does not exist\n"), m_sFilename);
    return FALSE;
  }

    //Hive away the filename and form the title from the filename
  TCHAR sPath[_MAX_PATH];
  TCHAR sFname[_MAX_FNAME];
  TCHAR sExt[_MAX_EXT];
  _tsplitpath(sFilename, NULL, NULL, sFname, sExt);
  _tmakepath(sPath, NULL, NULL, sFname, sExt);
    m_sFilename = sFilename;
  m_sTitle = sPath;

  //Also sent the content type to be appropiate for an attachment
  m_sContentType = _T("application/octet-stream");

  return TRUE;
}

void CSMTPBodyPart::SetText(const CString& sText)
{
  m_sText = sText;

  //Ensure lines are correctly wrapped
  m_sText.Replace(_T("\r\n"), _T("\n"));
  m_sText.Replace(_T("\r"), _T("\n"));
  m_sText.Replace(_T("\n"), _T("\r\n"));

  //Fix the case of a single dot on a line in the message body
  FixSingleDotT(m_sText);

  //Also set the content type while we are at it
  m_sContentType = _T("text/plain");
}

void CSMTPBodyPart::SetContentID(const CString& sContentID) 
{
  m_sContentLocation.Empty();
  m_sContentID = sContentID; 
}

CString CSMTPBodyPart::GetContentID() const 
{ 
  return m_sContentID; 
}

void CSMTPBodyPart::SetContentLocation(const CString& sContentLocation) 
{ 
  m_sContentID.Empty();
  m_sContentLocation = sContentLocation; 
}

CString CSMTPBodyPart::GetContentLocation() const 
{ 
  return m_sContentLocation; 
}

char CSMTPBodyPart::HexDigit(int nDigit)
{
  if (nDigit < 10)
    return (char) (nDigit + '0');
  else
    return (char) (nDigit - 10 + 'A');
}

//Converts text to its Quoted printable equivalent according to RFC 2045
std::string CSMTPBodyPart::QuotedPrintableEncode(const std::string& sText)
{
  USES_CONVERSION;

  //get the pointer to the internal buffer, its ASCII buffer
  LPCSTR pszAsciiText =  sText.c_str();
  std::string sTemp;
  int nSize = (int)strlen(pszAsciiText);
  for (int i=0; i<nSize; i++)
  {
    //Pull out the character to operate on
    BYTE c = pszAsciiText[i];
    
    if (((c >= 33) && (c <= 60)) || ((c >= 62) && (c <= 126)) || (c == '\r') || (c == '\n') || (c == '\t') || (c == ' '))
      sTemp += c;
    else
    {
      //otherwise must quote the text
      sTemp += '=';
      sTemp += HexDigit((c & 0xF0) >> 4);
      sTemp += HexDigit(c & 0x0F);
    }
  }

  //Now insert soft line breaks where appropiate
  std::string sOut;
  int nStartLine = 0;
  pszAsciiText =  sTemp.c_str();
  int nLen = (int)strlen(pszAsciiText);
  for (i=0; i<nLen; i++)
  {
    //Pull out the character to operate on
    BYTE c = pszAsciiText[i];
    
    if (c == '\n' || c == '\r' || i == (nLen-1))
    {
      sOut += sTemp.substr(nStartLine, i-nStartLine+1);
      nStartLine = i+1;
      continue;
    }

    if ((i - nStartLine) > SMTP_MAXLINE)
    {
      BOOL bInWord = TRUE;
      while (bInWord)
      {
        if (i>1)
          bInWord = (!isspace(c) && sTemp[i-2] != _T('='));
        if (bInWord)
        {
          --i;
          c = (BYTE) sTemp[i];
        }

            if (i == nStartLine)
            {
                i = nStartLine + SMTP_MAXLINE;
                break;
            }
      }

      sOut += sTemp.substr(nStartLine, i-nStartLine+1);
      sOut += "=\r\n";
      nStartLine = i+1;
    }
  }

  return sOut;
}

int CSMTPBodyPart::ConvertToUTF8(const CString& in, std::string &out)
{
    USES_CONVERSION;

    LPCWSTR psuBuff = T2CW(in);
    int nULength = (int)wcslen(psuBuff);
    LPSTR pUtf8 = NULL;

    //convert the data to utf-8
  int iLength = UnicodeToUTF8(psuBuff, nULength, pUtf8, 0);
  ASSERT(iLength);

    pUtf8 = new char[iLength+1];
  iLength = UnicodeToUTF8(psuBuff, nULength, pUtf8, iLength);
    ASSERT(iLength);
    pUtf8[iLength] = '\0';
    out = pUtf8;
    delete [] pUtf8;

    return iLength;
}

//Copy of the function AtlUnicodeToUTF8 in ATL Server. This allows the 
//SMTP class to operate on OS'es which do not support conversion to UTF-8 via
//WideCharToMultiByte
int CSMTPBodyPart::UnicodeToUTF8(LPCWSTR wszSrc, int nSrc, LPSTR szDest, int nDest)
{
    LPCWSTR pwszSrc = wszSrc;
    int     nU8 = 0;                // # of UTF8 chars generated
    DWORD   dwSurrogateChar;
    WCHAR   wchHighSurrogate = 0;
    BOOL    bHandled;

    while ((nSrc--) && ((nDest == 0) || (nU8 < nDest)))
    {
        bHandled = FALSE;

        //Check if high surrogate is available
        if ((*pwszSrc >= 0xd800) && (*pwszSrc <= 0xdbff))
        {
            if (nDest)
            {
                //Another high surrogate, then treat the 1st as normal Unicode character.
                if (wchHighSurrogate)
                {
                    if ((nU8 + 2) < nDest)
                    {
                        szDest[nU8++] = (char)(0xe0 | (wchHighSurrogate >> 12));
                        szDest[nU8++] = (char)(0x80 | (((wchHighSurrogate) & 0x0fc0) >> 6));
                        szDest[nU8++] = (char)(0x80 | (wchHighSurrogate & 0x003f));
                    }
                    else
                    {
                        //not enough buffer
                        nSrc++;
                        break;
                    }
                }
            }
            else
            {
                nU8 += 3;
            }
            wchHighSurrogate = *pwszSrc;
            bHandled = TRUE;
        }

        if (!bHandled && wchHighSurrogate)
        {
            if ((*pwszSrc >= 0xdc00) && (*pwszSrc <= 0xdfff))
            {
                 //valid surrogate pairs
                 if (nDest)
                 {
                     if ((nU8 + 3) < nDest)
                     {
                         dwSurrogateChar = (((wchHighSurrogate-0xD800) << 10) + (*pwszSrc - 0xDC00) + 0x10000);
                         szDest[nU8++] = (unsigned char) (0xf0 | (unsigned char)(dwSurrogateChar >> 18));           // 3 bits from 1st byte
                         szDest[nU8++] = (unsigned char) (0x80 | (unsigned char)((dwSurrogateChar >> 12) & 0x3f));  // 6 bits from 2nd byte
                         szDest[nU8++] = (unsigned char) (0x80 | (unsigned char)((dwSurrogateChar >> 6) & 0x3f));   // 6 bits from 3rd byte
                         szDest[nU8++] = (unsigned char) (0x80 | (unsigned char)(0x3f & dwSurrogateChar));          // 6 bits from 4th byte
                     }
                     else
                     {
                        //not enough buffer
                        nSrc++;
                        break;
                     }
                 }
                 else
                 {
                     //we already counted 3 previously (in high surrogate)
                     nU8 += 1;
                 }
                 bHandled = TRUE;
            }
            else
            {
                 //Bad Surrogate pair : ERROR
                 //Just process wchHighSurrogate , and the code below will
                 //process the current code point
                 if (nDest)
                 {
                     if ((nU8 + 2) < nDest)
                     {
                        szDest[nU8++] = (char)(0xe0 | (wchHighSurrogate >> 12));
                        szDest[nU8++] = (char)(0x80 | (((wchHighSurrogate) & 0x0fc0) >> 6));
                        szDest[nU8++] = (char)(0x80 | (wchHighSurrogate & 0x003f));
                     }
                     else
                     {
                        //not enough buffer
                        nSrc++;
                        break;
                     }
                 }
            }
            wchHighSurrogate = 0;
        }

        if (!bHandled)
        {
            if (*pwszSrc <= 0x007f)
            {
                //Found ASCII.
                if (nDest)
                {
                    szDest[nU8] = (char)*pwszSrc;
                }
                nU8++;
            }
            else if (*pwszSrc <= 0x07ff)
            {
                //Found 2 byte sequence if < 0x07ff (11 bits).
                if (nDest)
                {
                    if ((nU8 + 1) < nDest)
                    {
                        //Use upper 5 bits in first byte.
                        //Use lower 6 bits in second byte.
                        szDest[nU8++] = (char)(0xc0 | (*pwszSrc >> 6));
                        szDest[nU8++] = (char)(0x80 | (*pwszSrc & 0x003f));
                    }
                    else
                    {
                        //Error - buffer too small.
                        nSrc++;
                        break;
                    }
                }
                else
                {
                    nU8 += 2;
                }
            }
            else
            {
                //Found 3 byte sequence.
                if (nDest)
                {
                    if ((nU8 + 2) < nDest)
                    {
                        //Use upper  4 bits in first byte.
                        //Use middle 6 bits in second byte.
                        //Use lower  6 bits in third byte.
                        szDest[nU8++] = (char)(0xe0 | (*pwszSrc >> 12));
                        szDest[nU8++] = (char)(0x80 | (((*pwszSrc) & 0x0fc0) >> 6));
                        szDest[nU8++] = (char)(0x80 | (*pwszSrc & 0x003f));
                    }
                    else
                    {
                        //Error - buffer too small.
                        nSrc++;
                        break;
                    }
                }
                else
                {
                    nU8 += 3;
                }
            }
        }
        pwszSrc++;
    }

    //If the last character was a high surrogate, then handle it as a normal unicode character.
    if ((nSrc < 0) && (wchHighSurrogate != 0))
    {
        if (nDest)
        {
            if ((nU8 + 2) < nDest)
            {
                szDest[nU8++] = (char)(0xe0 | (wchHighSurrogate >> 12));
                szDest[nU8++] = (char)(0x80 | (((wchHighSurrogate) & 0x0fc0) >> 6));
                szDest[nU8++] = (char)(0x80 | (wchHighSurrogate & 0x003f));
            }
            else
            {
                nSrc++;
            }
        }
    }

    //Make sure the destination buffer was large enough.
    if (nDest && (nSrc >= 0))
    {
        return 0;
    }

    //Return the number of UTF-8 characters written.
    return nU8;
}

void CSMTPBodyPart::FreeHeader(LPSTR& pszHeader)
{
  //The CSMTPBodyPart class always allocates the memory for the header
  delete [] pszHeader;
  pszHeader = NULL;
}

void CSMTPBodyPart::FreeBody(LPSTR& pszBody)
{
  //The CSMTPBodyPart class allocates the memory for the body if it was not base 64 encoded
  if (pszBody)
  {
    delete [] pszBody;
    pszBody = NULL;
  }
}

void CSMTPBodyPart::FreeFooter(LPSTR& pszFooter)
{
  //The CSMTPBodyPart class always allocates the memory for the footer
  delete [] pszFooter;
  pszFooter = NULL;
}

BOOL CSMTPBodyPart::GetHeader(LPSTR& pszHeader, int& nHeaderSize)
{
  //For correct operation of the T2A macro, see MFC Tech Note 59
  USES_CONVERSION;

  //Assume the worst
  BOOL bSuccess = FALSE;
  CString sHeader;
  if (m_sFilename.GetLength())
  {
    //Ok, it's a file  

    //Form the header to go along with this body part
    if (GetNumberOfChildBodyParts())
          sHeader.Format(_T("\r\n\r\n--%s\r\nContent-Type: %s; charset=%s; name=%s; Boundary=\"%s\"\r\nContent-Transfer-Encoding: base64\r\nContent-Disposition: attachment; filename=\"%s\"\r\n"), 
                     m_pParentBodyPart->m_sBoundary, m_sContentType, m_sCharset, m_sTitle, m_sBoundary, m_sTitle);
    else
          sHeader.Format(_T("\r\n\r\n--%s\r\nContent-Type: %s; charset=%s; name=%s\r\nContent-Transfer-Encoding: base64\r\nContent-Disposition: attachment; filename=\"%s\"\r\n"), 
                     m_pParentBodyPart->m_sBoundary, m_sContentType, m_sCharset, m_sTitle, m_sTitle);

    bSuccess = TRUE;
  }
  else
  {
    //ok, it's some text

    //Form the header to go along with this body part
    ASSERT(m_pParentBodyPart);
    if (GetNumberOfChildBodyParts())
    {
      if (m_bBase64)
        sHeader.Format(_T("\r\n--%s\r\nContent-Type: %s; charset=%s; Boundary=\"%s\"\r\nContent-Transfer-Encoding: base64\r\n"),
                       m_pParentBodyPart->m_sBoundary, m_sContentType, m_sCharset, m_sBoundary);
      else if (m_bQuotedPrintable)
        sHeader.Format(_T("\r\n--%s\r\nContent-Type: %s; charset=%s; Boundary=\"%s\"\r\nContent-Transfer-Encoding: quoted-printable\r\n"),
                       m_pParentBodyPart->m_sBoundary, m_sContentType, m_sCharset, m_sBoundary);
      else
        sHeader.Format(_T("\r\n--%s\r\nContent-Type: %s; charset=%s; Boundary=\"%s\"\r\n"),
                       m_pParentBodyPart->m_sBoundary, m_sContentType, m_sCharset, m_sBoundary);
    }
    else
    {
      if (m_bBase64)
        sHeader.Format(_T("\r\n--%s\r\nContent-Type: %s; charset=%s\r\nContent-Transfer-Encoding: base64\r\n"),
                       m_pParentBodyPart->m_sBoundary, m_sContentType, m_sCharset);
      else if (m_bQuotedPrintable)
        sHeader.Format(_T("\r\n--%s\r\nContent-Type: %s; charset=%s\r\nContent-Transfer-Encoding: quoted-printable\r\n"),
                       m_pParentBodyPart->m_sBoundary, m_sContentType, m_sCharset);
      else
        sHeader.Format(_T("\r\n--%s\r\nContent-Type: %s; charset=%s\r\n"),
                       m_pParentBodyPart->m_sBoundary, m_sContentType, m_sCharset);
    }

    bSuccess = TRUE;
  }

  //Add the other headers
  if (m_sContentBase.GetLength())
  {
    CString sLine;
    sLine.Format(_T("Content-Base: %s\r\n"), m_sContentBase);
    sHeader += sLine;
  }
  if (m_sContentID.GetLength())
  {
    CString sLine;
    sLine.Format(_T("Content-ID: %s\r\n"), m_sContentID);
    sHeader += sLine;
  }
  if (m_sContentLocation.GetLength())
  {
    CString sLine;
    sLine.Format(_T("Content-Location: %s\r\n"), m_sContentLocation);
    sHeader += sLine;
  }
  sHeader += _T("\r\n");

  nHeaderSize = (int)_tcslen(sHeader);
  pszHeader = new char[nHeaderSize+1];
  strcpy(pszHeader, T2A((LPTSTR) (LPCTSTR) sHeader));

  return bSuccess;
}

BOOL CSMTPBodyPart::GetBody(LPSTR& pszBody, int& nBodySize)
{
    //if the body is text we must convert is to the declared encoding, this could create some
    //problems since windows conversion functions (such as WideCharToMultiByte) uses UINT
    //code page and not a String like HTTP uses.
    //For now we will add the support for UTF-8, to support other encodings we have to 
    //implement a mapping between the "internet name" of the encoding and its LCID(UINT)
    
    //For correct operation of the T2A macro, see MFC Tech Note 59
    USES_CONVERSION;

  //Assume the worst
  BOOL bSuccess = FALSE;

  if (m_sFilename.GetLength())
  {
    //Ok, it's a file  

    //open up the file for reading in
    CFile infile;
    if (infile.Open(m_sFilename, CFile::modeRead | CFile::shareDenyWrite))
    {
      DWORD dwSize = 0;
      BOOL bGotLength = FALSE;
      try
      {
        #if (_MFC_VER >= 0x700)
        ULONGLONG ullSize = infile.GetLength();
        if (ullSize > m_dwMaxAttachmentSize)
        {
          TRACE(_T("CSMTPBodyPart::GetBody, File is greater than max attachment size, %s\n"), m_sFilename);
          return FALSE;
        }
        else
          dwSize = (DWORD) ullSize;
        #else 
        dwSize = infile.GetLength();
        #endif
        bGotLength = TRUE;
      }
      catch (CFileException* pEx)
      {
        pEx->Delete();
        TRACE(_T("CSMTPBodyPart::GetBody, Failed to get the length of the file, %s\n"), m_sFilename);
      }

      if (dwSize)
      {
        //read in the contents of the input file
        BYTE* pszIn = new BYTE[dwSize];
        try
        {
          infile.Read(pszIn, dwSize);
          bSuccess = TRUE;
        }
        catch(CFileException* pEx)
        {
          bSuccess = FALSE;
          pEx->Delete();
          TRACE(_T("CSMTPBodyPart::GetBody, Failed to read the body parts file contents, %s\n"), m_sFilename);
        }

        if (bSuccess)
        {
          //Do the encoding
          m_Coder.Encode(pszIn, dwSize);

          //delete the input buffer
          delete [] pszIn;

          //Form the body for this body part
          LPSTR pszEncoded = m_Coder.EncodedMessage();
          int nEncodedSize = m_Coder.EncodedMessageSize();
          nBodySize = nEncodedSize + (((nEncodedSize/76)+1)*2) + 1;
          pszBody = new char[nBodySize];
          --nBodySize; //We do not count the NULL terminator for the body size

          int nInPos = 0;
          int nOutPos = 0;
          while (nInPos < nEncodedSize)
          {
            int nThisLineSize = min(nEncodedSize - nInPos, SMTP_MAXLINE);
            CopyMemory(&pszBody[nOutPos], &pszEncoded[nInPos], nThisLineSize);
            nOutPos += nThisLineSize;
            CopyMemory(&pszBody[nOutPos], "\r\n", 2);
            nOutPos += 2;
            nInPos += nThisLineSize;
          }
          pszBody[nOutPos] = '\0'; //Don't forget to NULL terminate
          nBodySize = nOutPos;     //Make sure the output length is correct !!
        }
      }
      else
      {
        if (bGotLength)
        {
          bSuccess = TRUE;
          pszBody = NULL;
          nBodySize = 0;
        }
      }

      //Explicitly handle the closing of the file here with an exception handler
      try
      {
        infile.Close();
      }
      catch(CFileException* pEx)
      {
        pEx->Delete();
        TRACE(_T("CSMTPBodyPart::GetBody, A file exception occured while closing the file when saving the message to file %s\n"), m_sFilename);
      }
    }
    else
      TRACE(_T("CSMTPBodyPart::GetBody, No bodypart body text or filename specified!\n"));
  }
  else
  {
    //ok, it's some text
    if (m_bBase64)
    {
      //Do the UTF8 conversion if necessary
      std::string sBuff;
      if (m_sCharset.CompareNoCase(_T("UTF-8")) == 0)
          ConvertToUTF8(m_sText, sBuff);
      else
        sBuff = T2CA(m_sText);

      //Do the encoding
      m_Coder.Encode((const PBYTE) sBuff.c_str(), (int)sBuff.length());

      //Form the body for this body part
      LPSTR pszEncoded = m_Coder.EncodedMessage();
      int nEncodedSize = m_Coder.EncodedMessageSize();
      nBodySize = nEncodedSize + (((nEncodedSize/76)+1)*2) + 1;
      pszBody = new char[nBodySize];
      --nBodySize; //We do not count the NULL terminator for the body size

      int nInPos = 0;
      int nOutPos = 0;
      while (nInPos < nEncodedSize)
      {
        int nThisLineSize = min(nEncodedSize - nInPos, SMTP_MAXLINE);
        CopyMemory(&pszBody[nOutPos], &pszEncoded[nInPos], nThisLineSize);
        nOutPos += nThisLineSize;
        CopyMemory(&pszBody[nOutPos], "\r\n", 2);
        nOutPos += 2;
        nInPos += nThisLineSize;
      }
      pszBody[nOutPos] = '\0'; //Don't forget to NULL terminate
    }
    else if (m_bQuotedPrintable)
    {
      //Do the UTF8 conversion if necessary
      std::string sBuff;
      if (m_sCharset.CompareNoCase(_T("UTF-8")) == 0)
          ConvertToUTF8(m_sText, sBuff);
      else
        sBuff = T2CA(m_sText);

      //Do the encoding
        std::string sBody = QuotedPrintableEncode(sBuff);
      FixSingleDotA(sBody);

      nBodySize = (int)sBody.length();
      pszBody = new char[nBodySize+1];
      strcpy(pszBody, sBody.c_str());
    }
    else
    {
      //Do the UTF8 conversion if necessary
      std::string sBody;
      if (m_sCharset.CompareNoCase(_T("UTF-8")) == 0)
          ConvertToUTF8(m_sText, sBody);
      else
        sBody = T2CA(m_sText);

      //No encoding to do
      FixSingleDotA(sBody);

          nBodySize = (int)sBody.length();
          pszBody = new char[nBodySize+1];
          strcpy(pszBody, sBody.c_str());
    }

    bSuccess = TRUE;
  }

  return bSuccess;
}

BOOL CSMTPBodyPart::GetFooter(LPSTR& pszFooter, int& nFooterSize)
{
  //For correct operation of the T2A macro, see MFC Tech Note 59
  USES_CONVERSION;

  //Form the MIME footer
    CString sFooter;
  sFooter.Format(_T("\r\n--%s--"), m_sBoundary);
  nFooterSize = (int)_tcslen(sFooter);
  pszFooter = new char[nFooterSize+1];
  strcpy(pszFooter, T2A((LPTSTR) (LPCTSTR) sFooter));

  return TRUE;  
}

int CSMTPBodyPart::GetNumberOfChildBodyParts() const
{
  return (int)m_ChildBodyParts.GetSize();
}

int CSMTPBodyPart::AddChildBodyPart(CSMTPBodyPart& bodyPart)
{
  CSMTPBodyPart* pNewBodyPart = new CSMTPBodyPart(bodyPart);
  pNewBodyPart->m_pParentBodyPart = this;
  ASSERT(m_sContentType.GetLength()); //Did you forget to call SetContentType

  return (int)m_ChildBodyParts.Add(pNewBodyPart);
}

void CSMTPBodyPart::RemoveChildBodyPart(int nIndex)
{
  CSMTPBodyPart* pBodyPart = m_ChildBodyParts.GetAt(nIndex);
  delete pBodyPart;
  m_ChildBodyParts.RemoveAt(nIndex);
}

CSMTPBodyPart* CSMTPBodyPart::GetChildBodyPart(int nIndex)
{
  return m_ChildBodyParts.GetAt(nIndex);
}

CSMTPBodyPart* CSMTPBodyPart::GetParentBodyPart()
{
  return m_pParentBodyPart;
}

void CSMTPBodyPart::FixSingleDotT(CString& sBody)
{
  int nFind = sBody.Find(_T("\n."));
  if (nFind != -1)
  {
      CString sLeft(sBody.Left(nFind+1));
      CString sRight(sBody.Right(sBody.GetLength()-(nFind+1)));
      FixSingleDotT(sRight);
      sBody = sLeft + _T(".") + sRight;
  }
}

void CSMTPBodyPart::FixSingleDotA(std::string& sBody)
{
  int nFind = (int)sBody.find("\n.");
  if (nFind != -1)
  {
      std::string sLeft(sBody.substr(0, nFind+1));
      std::string sRight(sBody.substr(nFind+1));
      FixSingleDotA(sRight);
      sBody = sLeft + "." + sRight;
  }
}

CSMTPBodyPart* CSMTPBodyPart::FindFirstBodyPart(const CString sContentType)
{
  for (int i=0; i<m_ChildBodyParts.GetSize(); i++)
  {
    CSMTPBodyPart* pBodyPart = m_ChildBodyParts.GetAt(i);
    if (pBodyPart->m_sContentType == sContentType)
      return pBodyPart;
  }
  return NULL;
}

//Converts header text to its encoded form according to RFC 2047
std::string CSMTPBodyPart::QEncode(LPCSTR sText, LPCSTR sCharset)
{
  USES_CONVERSION;

  //Determine if a translation is needed
  BOOL bTranslationNeeded = FALSE;
  int nSize = (int)strlen(sText);
  for (int i=0; i<nSize && !bTranslationNeeded; i++)
  {
    BYTE c = (BYTE) sText[i];
    bTranslationNeeded = (c > 127);
  }

  std::string sOut;
  if (bTranslationNeeded)
  {
    sOut = "=?";
    sOut += sCharset;
    sOut += "?q?";
    for (i=0; i<nSize; i++)
    {
      BYTE c = sText[i];
      
      if (c == ' ') // A space
        sOut += _T('_');
      else if ((c > 127) || (c == '=') || (c == '?') || (c == '_'))
      {
        //Must Quote the text
        sOut += _T('=');
        sOut += CSMTPBodyPart::HexDigit((c & 0xF0) >> 4);
        sOut += CSMTPBodyPart::HexDigit(c & 0x0F);
      }
      else
        sOut += c;
    }
    sOut += "?=";
  }
  else
  {
    //Just pass the text thro unmodified
    sOut = sText;
  }

  return sOut;
}

std::string CSMTPBodyPart::HeaderEncode(const CString& sText, const CString& sCharset)
{
    USES_CONVERSION;

  //Do the UTF8 conversion if necessary
    std::string sLocalText;
  if (sCharset.CompareNoCase(_T("UTF-8")) == 0)
      CSMTPBodyPart::ConvertToUTF8(sText, sLocalText);
  else
    sLocalText = T2CA(sText);

  LPCSTR pszAsciiCharset = T2CA(sCharset);

  std::string sOut;
    LPCSTR itr = sLocalText.c_str();
  int nCurrentLineLength = (int)strlen(itr);
    while (nCurrentLineLength > 70)
    {
        //copy the current line and move the pointer forward
        int length = (70 > nCurrentLineLength) ? nCurrentLineLength : 70;

        char buf[71];
        strncpy(buf, itr, length);
    buf[length] = '\0';

    //Prepare for the next time around
        itr += length;
    nCurrentLineLength = (int)strlen(itr);

    //Encode the current
        sOut += QEncode(buf, pszAsciiCharset);
    }
    sOut += QEncode(itr, pszAsciiCharset);

  return sOut;
}



CSMTPMessage::CSMTPMessage() : m_sXMailer(_T("CPJNSMTPConnection v2.42")), m_bMime(FALSE), m_Priority(NO_PRIORITY)
{
}

CSMTPMessage::CSMTPMessage(const CSMTPMessage& message)
{
  *this = message;
}

CSMTPMessage& CSMTPMessage::operator=(const CSMTPMessage& message) 
{ 
  m_From     = message.m_From;
  m_sSubject = message.m_sSubject;
  m_sXMailer = message.m_sXMailer;
  m_ReplyTo  = message.m_ReplyTo;
  m_RootPart = message.m_RootPart;
  m_Priority = message.m_Priority;

  //Free up the To memory
  for (int i=0; i<m_ToRecipients.GetSize(); i++)
    delete m_ToRecipients.GetAt(i);
  m_ToRecipients.RemoveAll();

  //Now copy over the new object
  for (i=0; i<message.m_ToRecipients.GetSize(); i++)
  {
    CSMTPAddress* pAddress = new CSMTPAddress(*message.m_ToRecipients.GetAt(i));
    m_ToRecipients.Add(pAddress);
  }

  //Free up the CC memory
  for (i=0; i<m_CCRecipients.GetSize(); i++)
    delete m_CCRecipients.GetAt(i);
  m_CCRecipients.RemoveAll();

  //Now copy over the new object
  for (i=0; i<message.m_CCRecipients.GetSize(); i++)
  {
    CSMTPAddress* pAddress = new CSMTPAddress(*message.m_CCRecipients.GetAt(i));
    m_CCRecipients.Add(pAddress);
  }

  //Free up the BCC memory
  for (i=0; i<m_BCCRecipients.GetSize(); i++)
    delete m_BCCRecipients.GetAt(i);
  m_BCCRecipients.RemoveAll();

  //Now copy over the new object
  for (i=0; i<message.m_BCCRecipients.GetSize(); i++)
  {
    CSMTPAddress* pAddress = new CSMTPAddress(*message.m_BCCRecipients.GetAt(i));
    m_BCCRecipients.Add(pAddress);
  }

  m_CustomHeaders.Copy(message.m_CustomHeaders);
  m_bMime = message.m_bMime;

    return *this;
}

CSMTPMessage::~CSMTPMessage()
{
  //Free up the array memory
  for (int i=0; i<m_ToRecipients.GetSize(); i++)
    delete m_ToRecipients.GetAt(i);
  m_ToRecipients.RemoveAll();

  for (i=0; i<m_CCRecipients.GetSize(); i++)
    delete m_CCRecipients.GetAt(i);
  m_CCRecipients.RemoveAll();

  for (i=0; i<m_BCCRecipients.GetSize(); i++)
    delete m_BCCRecipients.GetAt(i);
  m_BCCRecipients.RemoveAll();
}

void CSMTPMessage::SetCharset(const CString& sCharset)
{
  m_RootPart.SetCharset(sCharset);
}

CString CSMTPMessage::GetCharset() const
{
  return m_RootPart.GetCharset();
}

int CSMTPMessage::GetNumberOfRecipients(RECIPIENT_TYPE RecipientType) const
{
  int nSize = 0;
  switch (RecipientType)
  {
    case TO:  nSize = (int)m_ToRecipients.GetSize();  break;
    case CC:  nSize = (int)m_CCRecipients.GetSize();  break;
    case BCC: nSize = (int)m_BCCRecipients.GetSize(); break;
    default: ASSERT(FALSE);                      break;
  }

    return nSize;
}

int CSMTPMessage::AddRecipient(CSMTPAddress& recipient, RECIPIENT_TYPE RecipientType)
{
  int nIndex = -1;

  CSMTPAddress* pNewRecipient = new CSMTPAddress(recipient);

  switch (RecipientType)
  {
    case TO:  nIndex = (int)m_ToRecipients.Add(pNewRecipient);  break;
    case CC:  nIndex = (int)m_CCRecipients.Add(pNewRecipient);  break;
    case BCC: nIndex = (int)m_BCCRecipients.Add(pNewRecipient); break;
    default: ASSERT(FALSE);                            break;
  }

  return nIndex;
}

void CSMTPMessage::RemoveRecipient(int nIndex, RECIPIENT_TYPE RecipientType)
{
  switch (RecipientType)
  {
    case TO:  
    {
      delete m_ToRecipients.GetAt(nIndex);
      m_ToRecipients.RemoveAt(nIndex);  
      break;
    }
    case CC:  
    {
      delete m_CCRecipients.GetAt(nIndex);
      m_CCRecipients.RemoveAt(nIndex);  
      break;
    }
    case BCC: 
    {
      delete m_BCCRecipients.GetAt(nIndex);
      m_BCCRecipients.RemoveAt(nIndex); 
      break;
    }
    default:  
    {
      ASSERT(FALSE);                    
      break;
    }
  }
}

CSMTPAddress* CSMTPMessage::GetRecipient(int nIndex, RECIPIENT_TYPE RecipientType)
{
  switch (RecipientType)
  {
    case TO:  return m_ToRecipients.GetAt(nIndex);   break;
    case CC:  return m_CCRecipients.GetAt(nIndex);   break;
    case BCC: return m_BCCRecipients.GetAt(nIndex);  break;
    default: ASSERT(FALSE); return (CSMTPAddress*) NULL; break;
  }
}

int CSMTPMessage::AddBodyPart(CSMTPBodyPart& Attachment)
{
  SetMime(TRUE); //Body parts implies Mime
    return m_RootPart.AddChildBodyPart(Attachment);
}

void CSMTPMessage::RemoveBodyPart(int nIndex)
{
    m_RootPart.RemoveChildBodyPart(nIndex);
}

CSMTPBodyPart* CSMTPMessage::GetBodyPart(int nIndex)
{
    return m_RootPart.GetChildBodyPart(nIndex);
}

int CSMTPMessage::GetNumberOfBodyParts() const
{
    return m_RootPart.GetNumberOfChildBodyParts();
}

void CSMTPMessage::AddCustomHeader(const CString& sHeader)
{
  m_CustomHeaders.Add(sHeader);
}

CString CSMTPMessage::GetCustomHeader(int nIndex)
{
  return m_CustomHeaders.GetAt(nIndex);
}

int CSMTPMessage::GetNumberOfCustomHeaders() const
{
  return (int)m_CustomHeaders.GetSize();
}

void CSMTPMessage::RemoveCustomHeader(int nIndex)
{
  m_CustomHeaders.RemoveAt(nIndex);
}

std::string CSMTPMessage::getHeader()
{
    USES_CONVERSION;

  //Hive away the locale so that we can restore it later. We
  //require the English locale to ensure the date header is
  //formed correctly
  CString sOldLocale = _tsetlocale(LC_TIME, NULL);    
  _tsetlocale(LC_TIME, _T("english"));

  //Form the Timezone info which will form part of the Date header
  TIME_ZONE_INFORMATION tzi;
  int nTZBias;
  if (GetTimeZoneInformation(&tzi) == TIME_ZONE_ID_DAYLIGHT)
    nTZBias = tzi.Bias + tzi.DaylightBias;
  else
    nTZBias = tzi.Bias;
  CString sTZBias;
  sTZBias.Format(_T("%+.2d%.2d"), -nTZBias/60, nTZBias%60);

  //Create the "Date:" part of the header
  CTime now(CTime::GetCurrentTime());
  CString sDate(now.Format(_T("%a, %d %b %Y %H:%M:%S ")));
  sDate += sTZBias;

  CString sCharset = m_RootPart.GetCharset();

  //Create the "To:" part of the header
  CString sTo;
  for (int i=0; i<GetNumberOfRecipients(TO); i++)
  {
    CSMTPAddress* pRecipient = GetRecipient(i, TO);
    if (i)
          sTo += _T(",");
    ASSERT(pRecipient);
    sTo += pRecipient->GetRegularFormat(TRUE, sCharset);
  }

  //Create the "Cc:" part of the header
  CString sCc;
  for (i=0; i<GetNumberOfRecipients(CC); i++)
  {
    CSMTPAddress* pRecipient = GetRecipient(i, CC);
    if (i)
          sCc += _T(",");
    ASSERT(pRecipient);
    sCc += pRecipient->GetRegularFormat(TRUE, sCharset);
  }

  //No Bcc info added in header


  //Move to ASCII from CString at this stage

  //Add the From and To fields
  std::string suBuf("From: ");
  suBuf += T2CA(m_From.GetRegularFormat(TRUE, sCharset));
  suBuf += "\r\nTo: ";
  suBuf += T2CA(sTo);
  suBuf += "\r\n";

  //Add the CC field if there is any
  if (sCc.GetLength())
  {
    suBuf += "Cc: ";
    suBuf += T2CA(sCc);
    suBuf += "\r\n";
  }

  //add the subject
  suBuf += "Subject: ";
  suBuf += CSMTPBodyPart::HeaderEncode(m_sSubject, sCharset);

  //X-Mailer fields
  if (m_sXMailer.GetLength())
  {
    suBuf += "\r\nX-Mailer: ";
    suBuf += T2CA(m_sXMailer);
  }
  suBuf += "\r\n";

  //Add the Mime header if needed
  BOOL bHasChildParts = (m_RootPart.GetNumberOfChildBodyParts() != 0);

  if (m_bMime)
  {
    //QP and UTF8 encoding is only supported if the mail is being sent as MIME
    if (m_RootPart.m_bBase64)
      suBuf += "Content-Transfer-Encoding: base64\r\n";
    else if (m_RootPart.m_bQuotedPrintable)
      suBuf += "Content-Transfer-Encoding: quoted-printable\r\n";

    if (bHasChildParts)
    {
      CString sReply;
      sReply.Format(_T("MIME-Version: 1.0\r\nContent-Type: %s; boundary=\"%s\"\r\n"), m_RootPart.GetContentType(), m_RootPart.GetBoundary());
      suBuf += T2CA(sReply);
    }
    else
    {
      CString sReply;
      sReply.Format(_T("MIME-Version: 1.0\r\nContent-Type: %s\r\n"), m_RootPart.GetContentType());
      suBuf += T2CA(sReply);
    }
  }
  else
  {
    CString sReply;
    sReply.Format(_T("Content-Type: %s;\r\n\tcharset=%s\r\n"), m_RootPart.GetContentType(), m_RootPart.GetCharset());
    suBuf += T2CA(sReply);
  }
  
    //Add the optional Reply-To Field
    if (m_ReplyTo.m_sEmailAddress.GetLength())
    {
        suBuf += "Reply-To: ";
        suBuf += T2CA(m_ReplyTo.GetRegularFormat(TRUE, sCharset));
    suBuf += "\r\n";
    }

  //Date header
  suBuf += "Date: ";
  suBuf += T2CA(sDate);
  suBuf += "\r\n";

  //The Priorty header
  switch (m_Priority)
  {
    case NO_PRIORITY:
    {
      break;
    }
    case LOW_PRIORITY:
    {
      suBuf += "X-Priority: 5\r\n";
      break;
    }
    case NORMAL_PRIORITY:
    {
      suBuf += "X-Priority: 3\r\n";
      break;
    }
    case HIGH_PRIORITY:
    {
      suBuf += "X-Priority: 1\r\n";
      break;
    }
    default:
    {
      ASSERT(FALSE);
      break;
    }
  }

  //Add the custom headers
  int nCustomHeaders = (int)m_CustomHeaders.GetSize();
  for (i=0; i<nCustomHeaders; i++)
  {
    CString sHeader = m_CustomHeaders.GetAt(i);
    suBuf += T2CA(sHeader);
    
    //Add line separators for each header
    suBuf += "\r\n";
  }

  // restore original locale
  _tsetlocale(LC_TIME, sOldLocale);

    //Return the result
    return suBuf;
}

int CSMTPMessage::ParseMultipleRecipients(const CString& sRecipients, CSMTPAddressArray& recipients)
{
    ASSERT(sRecipients.GetLength()); //An empty string is now allowed

  //Empty out the array
  recipients.SetSize(0);
    
    //Loop through the whole string, adding recipients as they are encountered
    int length = sRecipients.GetLength();
    TCHAR* buf = new TCHAR[length + 1]; // Allocate a work area (don't touch parameter itself)
    _tcscpy(buf, sRecipients);
    for (int pos=0, start=0; pos<=length; pos++)
    {
        //Valid separators between addresses are ',' or ';'
        if ((buf[pos] == _T(',')) || (buf[pos] == _T(';')) || (buf[pos] == 0))
        {
            buf[pos] = 0;   //Redundant when at the end of string, but who cares.
      CString sTemp(&buf[start]);
      sTemp.TrimLeft();
      sTemp.TrimRight();

      //Let the CSMTPAddress constructor do its work
      CSMTPAddress To(sTemp);
      if (To.m_sEmailAddress.GetLength())
        recipients.Add(To);

      //Move on to the next position
            start = pos + 1;
        }
    }
    
  //Tidy up the heap memory we have used
    delete [] buf;

  //Return the number of recipients parsed
    return (int)recipients.GetSize();
}

BOOL CSMTPMessage::AddMultipleRecipients(const CString& sRecipients, RECIPIENT_TYPE RecipientType)
{
  CSMTPAddressArray Recipients;
  int nRecipients = ParseMultipleRecipients(sRecipients, Recipients);

  //Add them to the correct array
  for (int i=0; i<nRecipients; i++)
  {
    CSMTPAddress& address = Recipients.ElementAt(i);
    AddRecipient(address, RecipientType);
  }

    return (nRecipients != 0);
}

int CSMTPMessage::AddMultipleAttachments(const CString& sAttachments)
{
  //Assume the best
  BOOL nAttachments = 0;

    ASSERT(sAttachments.GetLength()); //An empty string is now allowed
    
    //Loop through the whole string, adding attachments as they are encountered
    int length = sAttachments.GetLength();
    TCHAR* buf = new TCHAR[length + 1]; // Allocate a work area (don't touch parameter itself)
    _tcscpy(buf, sAttachments);
    for (int pos=0, start=0; pos<=length; pos++)
    {
        //Valid separators between attachments are ',' or ';'
        if ((buf[pos] == _T(',')) || (buf[pos] == _T(';')) || (buf[pos] == 0))
        {
            buf[pos] = 0;   //Redundant when at the end of string, but who cares.
      CString sTemp(&buf[start]);

      //Finally add the new attachment to the array of attachments
      CSMTPBodyPart attachment;
            sTemp.TrimRight();
            sTemp.TrimLeft();
      if (sTemp.GetLength())
      {
        BOOL bAdded = attachment.SetFilename(sTemp);
        if (bAdded)
        {
          ++nAttachments;
          AddBodyPart(attachment);
        }
      }

      //Move on to the next position
            start = pos + 1;
        }
    }

  //Tidy up the heap memory we have used
    delete [] buf;

    return nAttachments;
}

CString CSMTPMessage::ConvertHTMLToPlainText(const CString& sHtml)
{
  //First pull out whats within the body tags
  CString sRet(sHtml);
  sRet.MakeUpper();
  int nStartCut = -1;
  int nStartBody = sRet.Find(_T("<BODY"));
  if (nStartBody != -1)
  {
    sRet = sHtml.Right(sHtml.GetLength() - nStartBody - 5);
    int nTemp = sRet.Find(_T('>'));
    nStartCut = nStartBody + nTemp + 6;
    sRet = sRet.Right(sRet.GetLength() - nTemp - 1);
  }
  sRet.MakeUpper();
  int nLength = sRet.Find(_T("</BODY"));

  //Finally do the actual cutting
  if (nLength != -1)
    sRet = sHtml.Mid(nStartCut, nLength);
  else
    sRet = sHtml;

  //Now strip all html tags
  int nStartTag = sRet.Find(_T('<'));
  int nEndTag = sRet.Find(_T('>'));
  while (nStartTag != -1 && nEndTag != -1)
  {
    sRet = sRet.Left(nStartTag) + sRet.Right(sRet.GetLength() - nEndTag - 1);
    nStartTag = sRet.Find(_T('<'));
    nEndTag = sRet.Find(_T('>'));
  }

  sRet.TrimLeft();
  sRet.TrimRight();
  return sRet;
}

void CSMTPMessage::AddTextBody(const CString& sBody)
{
  if (m_bMime)
  {
    CSMTPBodyPart* pTextBodyPart = m_RootPart.FindFirstBodyPart(_T("text/plain"));
    if (pTextBodyPart)
      pTextBodyPart->SetText(sBody);
    else
    {
      //Create a text body part
      CSMTPBodyPart oldRoot = m_RootPart;

      //Reset the root body part to be multipart/related
      m_RootPart.SetCharset(oldRoot.GetCharset());
      m_RootPart.SetText(_T("This is a multi-part message in MIME format"));
      m_RootPart.SetContentType(_T("multipart/mixed"));

      //Just add the text/plain body part (directly to the root)
      CSMTPBodyPart text;
      text.SetCharset(oldRoot.GetCharset());
      text.SetText(sBody);

      //Hook everything up to the root body part
      m_RootPart.AddChildBodyPart(text);
    }
  }
  else
  {
    //Let the body part class do all the work
    m_RootPart.SetText(sBody);
  }
}

void CSMTPMessage::AddHTMLBody(const CString& sBody, const CString& sContentBase)
{
  ASSERT(m_bMime); //You forgot to make this a MIME message using SetMime(TRUE)

  CSMTPBodyPart* pHtmlBodyPart = m_RootPart.FindFirstBodyPart(_T("text/html"));
  if (pHtmlBodyPart)
    pHtmlBodyPart->SetText(sBody);
  else
  {
    //Remember some of the old root settings before we write over it
    CSMTPBodyPart oldRoot = m_RootPart;

    //Reset the root body part to be multipart/related
    m_RootPart.SetCharset(oldRoot.GetCharset());
    m_RootPart.SetText(_T("This is a multi-part message in MIME format"));
    m_RootPart.SetContentType(_T("multipart/related"));

    //Just add the text/html body part (directly to the root)
    CSMTPBodyPart html;
    html.SetCharset(oldRoot.GetCharset());
    html.SetText(sBody);
    html.SetContentType(_T("text/html"));
    html.SetContentBase(sContentBase);

    //Hook everything up to the root body part
    m_RootPart.AddChildBodyPart(html);
  }
}

CString CSMTPMessage::GetHTMLBody()
{
  CString sRet;

  if (m_RootPart.GetNumberOfChildBodyParts())
  {
    CSMTPBodyPart* pHtml = m_RootPart.GetChildBodyPart(0);
    if (pHtml->GetContentType() == _T("text/html"))
      sRet = pHtml->GetText();
  }
  return sRet;
}

CString CSMTPMessage::GetTextBody()
{
  return m_RootPart.GetText();
}

void CSMTPMessage::SetMime(BOOL bMime)
{
  if (m_bMime != bMime)
  {
    m_bMime = bMime;

    //Reset the body body parts
    for (int i=0; i<m_RootPart.GetNumberOfChildBodyParts(); i++)
      m_RootPart.RemoveChildBodyPart(i);

    if (bMime)
    {
      CString sText = GetTextBody();

      //Remember some of the old root settings before we write over it
      CSMTPBodyPart oldRoot = m_RootPart;

      //Reset the root body part to be multipart/mixed
      m_RootPart.SetCharset(oldRoot.GetCharset());
      m_RootPart.SetText(_T("This is a multi-part message in MIME format"));
      m_RootPart.SetContentType(_T("multipart/mixed"));

      //Also readd the body if non - empty
      if (sText.GetLength())
        AddTextBody(sText);
    }
  }
}

BOOL CSMTPMessage::SaveToDisk(const CString& sFilename)
{
    //For correct operation of the T2A macro, see MFC Tech Note 59
    USES_CONVERSION;

  //Assume the worst
  BOOL bSuccess = FALSE;

  //Open the file for writing
  CFile outFile;
  if (outFile.Open(sFilename, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite))
  {
    try
    {
      //Write out the Message Header
      std::string sHeader = getHeader();
      int nCmdLength = (int)sHeader.length();
      outFile.Write(sHeader.c_str() , nCmdLength);

        //Write out the separator
      char* pszBodyHeader = "\r\n";
      nCmdLength = (int)strlen(pszBodyHeader);
      outFile.Write(pszBodyHeader, nCmdLength);

      //Write out the rest of the message
      BOOL bHasChildParts = (m_RootPart.GetNumberOfChildBodyParts() != 0);
      if (bHasChildParts || m_bMime)
      {
        //Write the root body part (and all its children)
        bSuccess = WriteToDisk(outFile, &m_RootPart, TRUE);
      }
      else
      {
        //Send the body
        char* pszBody = T2A((LPTSTR) (LPCTSTR) m_RootPart.GetText());
        ASSERT(pszBody);
        nCmdLength = (int)strlen(pszBody);

        //Send the body
        outFile.Write(pszBody, nCmdLength);
        bSuccess = TRUE;
      }
    }
    catch(CFileException* pEx)
    {
      TRACE(_T("CSMTPMessage::SaveToDisk, A file exception occured while trying to save the message to file %s\n"), sFilename);
      pEx->Delete();
    }

    //Explicitly handle the closing of the file here with an exception handler
    try
    {
      outFile.Close();
    }
    catch(CFileException* pEx)
    {
      TRACE(_T("CSMTPMessage::SaveToDisk, A file exception occured while closing the file when saving the message to file %s\n"), sFilename);
      pEx->Delete();
    }
  }

  return bSuccess;
}

BOOL CSMTPMessage::WriteToDisk(CFile& file, CSMTPBodyPart* pBodyPart, BOOL bRoot)
{
  //Assume success
  BOOL bSuccess = TRUE;

  if (!bRoot)
  {
    //First send this body parts header
    LPSTR pszHeader = NULL;
    int nHeaderSize = 0;
    if (!pBodyPart->GetHeader(pszHeader, nHeaderSize))
    {
          TRACE(_T("CSMTPMessage::WriteToDisk, Failed in call to send body parts header, GetLastError returns: %d\n"), GetLastError());
          return FALSE;
    }
        file.Write(pszHeader, nHeaderSize);

    //Free up the temp memory we have used
    pBodyPart->FreeHeader(pszHeader);
  }
  
  //Then the body parts body
  LPSTR pszBody = NULL;
  int nBodySize = 0;
  if (!pBodyPart->GetBody(pszBody, nBodySize))
  {
        TRACE(_T("CSMTPMessage::WriteToDisk, Failed in call to send body parts body, GetLastError returns: %d\n"), GetLastError());
        return FALSE;
  }
    file.Write(pszBody, nBodySize);

  //Free up the temp memory we have used
  pBodyPart->FreeBody(pszBody);

  //Recursively send all the child body parts
  int nChildBodyParts = pBodyPart->GetNumberOfChildBodyParts();
  for (int i=0; i<nChildBodyParts && bSuccess; i++)
  {
    CSMTPBodyPart* pChildBodyPart = pBodyPart->GetChildBodyPart(i);
    bSuccess = WriteToDisk(file, pChildBodyPart, FALSE);
  }

  //Then the MIME footer if need be
  BOOL bSendFooter = (pBodyPart->GetNumberOfChildBodyParts() != 0);
  if (bSendFooter)
  {
    LPSTR pszFooter = NULL;
    int nFooterSize = 0;
    if (!pBodyPart->GetFooter(pszFooter, nFooterSize))
    {
          TRACE(_T("CSMTPMessage::WriteToDisk, Failed in call to send body parts footer, GetLastError returns: %d\n"), GetLastError());
          return FALSE;
    }

      file.Write(pszFooter, nFooterSize);

    //Free up the temp memory we have used
    pBodyPart->FreeFooter(pszFooter);
  }
  
  return bSuccess;
}



CPJNSMTPConnection::CPJNSMTPConnection()
{
  m_bConnected = FALSE;
#ifdef _DEBUG
  m_dwTimeout = 90000; //default timeout of 90 seconds when debugging
#else
  m_dwTimeout = 60000;  //default timeout of 60 seconds for normal release code
#endif
  m_nLastCommandResponseCode = 0;
  m_sHeloHostname = _T("auto");
}

CPJNSMTPConnection::~CPJNSMTPConnection()
{
  if (m_bConnected)
    Disconnect(TRUE);
}

#ifdef _DEBUG
void CPJNSMTPConnection::OnError(const CString& sError)
#else
void CPJNSMTPConnection::OnError(const CString& /*sError*/)
#endif
{
  //By default all we do is TRACE the string, derived classes can do
  //something more useful with the info
#ifdef _DEBUG
  TRACE(_T("%s\n"), sError);
#endif
}

BOOL CPJNSMTPConnection::Connect(LPCTSTR pszHostName, LoginMethod lm, LPCTSTR pszUsername, LPCTSTR pszPassword, int nPort, LPCTSTR pszLocalBoundAddress)
{
    //For correct operation of the T2A macro, see MFC Tech Note 59
    USES_CONVERSION;

    //Validate our parameters
  ASSERT(pszHostName);
  ASSERT(!m_bConnected);

  //Create the socket
  if (!m_SMTP.Create())
  {
    CString sError;
    sError.Format(_T("CPJNSMTPConnection::Connect, Failed to create client socket, GetLastError returns: %d\n"), GetLastError());
    OnError(sError);
    return FALSE;
  }

  //Connect to the SMTP Host
  if (!m_SMTP.Connect(pszHostName, nPort, pszLocalBoundAddress))
  {
    if (pszLocalBoundAddress  && _tcslen(pszLocalBoundAddress))
    {
      CString sError;
      sError.Format(_T("CPJNSMTPConnection::Connect, Could not connect to the SMTP server %s on port %d from LocalAddress:%s, GetLastError returns: %d"), pszHostName, nPort, pszLocalBoundAddress, GetLastError());
      OnError(sError);
    }
    else
    {
      CString sError;
      sError.Format(_T("CPJNSMTPConnection::Connect, Could not connect to the SMTP server %s on port %d, GetLastError returns: %d"), pszHostName, nPort, GetLastError());
      OnError(sError);
    }
    return FALSE;
  }
  else
  {
    //We're now connected !!
    m_bConnected = TRUE;

    //check the response to the login
    if (!ReadCommandResponse(220))
    {
      OnError(_T("CPJNSMTPConnection::Connect, An unexpected SMTP login response was received"));
      Disconnect(TRUE);
      return FALSE;
    }

      //retreive the localhost name (assuming we are not using a custom one)
        if (m_sHeloHostname == _T("auto"))
        {
            //retrieve the localhost name
            char sHostName[_MAX_PATH];
            gethostname(sHostName, sizeof(sHostName));
            TCHAR* pszHostName = A2T(sHostName);
            m_sHeloHostname = pszHostName;
        }

      // negotiate Extended SMTP connection
      BOOL bConnectOk = FALSE;
      if (lm != NoLoginMethod)
          bConnectOk = ConnectESMTP(m_sHeloHostname, pszUsername, pszPassword, lm);
    else
          bConnectOk = ConnectSMTP(m_sHeloHostname);

      // if bConnectOk is still false then connection failed
      if (!bConnectOk)
      {
      Disconnect(TRUE);
      OnError(_T("CPJNSMTPConnection::Connect, An unexpected HELO/EHLO response was received"));
      return FALSE;
      }

    return TRUE;
  }
}

// This function connects using one of the Extended SMTP methods i.e. EHLO
BOOL CPJNSMTPConnection::ConnectESMTP(LPCTSTR pszLocalName, LPCTSTR pszUsername, LPCTSTR pszPassword, LoginMethod lm)
{
  //Validate our parameters
  ASSERT(pszUsername);
  ASSERT(pszPassword);
  ASSERT(lm != NoLoginMethod);

    //For correct operation of the T2A macro, see MFC Tech Note 59
    USES_CONVERSION;

  //Send the EHLO command
    CString sBuf;
    sBuf.Format(_T("EHLO %s\r\n"), pszLocalName);
  LPCSTR pszData = T2A((LPTSTR) (LPCTSTR) sBuf);
  int nCmdLength = (int)strlen(pszData);
  if (!m_SMTP.Send(pszData, nCmdLength))
  {
    OnError(_T("CPJNSMTPConnection::ConnectESMTP, An unexpected error occurred while sending the EHLO command"));
    return FALSE;
  }
    
  //check the response to the EHLO command
  if (!ReadCommandResponse(250, TRUE))
  {
      OnError(_T("CPJNSMTPConnection::ConnectESMTP, An unexpected response was received occurred while sending the EHLO command"));
      return FALSE;
    }

    BOOL bLoginOk = FALSE;
  switch (lm)
  {
    #ifndef CSMTP_NORSA
    case CramMD5Method:
    {
      bLoginOk = CramLogin(pszUsername, pszPassword); // CRAM-MD5 authentication
      break;
    }
    #endif
    case AuthLoginMethod:
    {
      bLoginOk = AuthLogin(pszUsername, pszPassword); // LOGIN authentication
      break;
    }
    case LoginPlainMethod:
    {
      bLoginOk = AuthLoginPlain(pszUsername, pszPassword); // PLAIN authentication
      break;
    }
    default:
    {
      ASSERT(FALSE);
      break;
    }
  }

    return bLoginOk;
}

// This function connects using standard SMTP connection i.e. HELO
BOOL CPJNSMTPConnection::ConnectSMTP(LPCTSTR pszLocalName)
{
    //For correct operation of the T2A macro, see MFC Tech Note 59
    USES_CONVERSION;

  //Send the HELO command
    CString sBuf;
    sBuf.Format(_T("HELO %s\r\n"), pszLocalName);
    LPCSTR pszData = T2A((LPTSTR) (LPCTSTR) sBuf);
    int nCmdLength = (int)strlen(pszData);
    if (!m_SMTP.Send(pszData, nCmdLength))
    {
      OnError(_T("CPJNSMTPConnection::ConnectSMTP, An unexpected error occurred while sending the HELO command"));
      return FALSE;
    }

    //check the response to the HELO command
    return ReadCommandResponse(250);
}

BOOL CPJNSMTPConnection::Disconnect(BOOL bGracefully)
{
  BOOL bSuccess = FALSE;      

  //disconnect from the SMTP server if connected 
  if (m_bConnected)
  {
    if (bGracefully)
    {
      char sBuf[10];
      strcpy(sBuf, "QUIT\r\n");
      int nCmdLength = (int)strlen(sBuf);
      if (!m_SMTP.Send(sBuf, nCmdLength))
      {
        CString sError;
        sError.Format(_T("CPJNSMTPConnection::Disconnect, Failed in call to send QUIT command, GetLastError returns: %d"), GetLastError());
        OnError(sError);
      }

      //Check the reponse
      bSuccess = ReadCommandResponse(221);
      if (!bSuccess)
      {
        SetLastError(ERROR_BAD_COMMAND);
        OnError(_T("CPJNSMTPConnection::Disconnect, An unexpected QUIT response was received"));
      }
    }

    //Reset all the state variables
    m_bConnected = FALSE;
  }
  else
    OnError(_T("CPJNSMTPConnection::Disconnect, Already disconnected from SMTP server, doing nothing"));
 
  //free up our socket
  m_SMTP.Close();
 
  return bSuccess;
}

BOOL CPJNSMTPConnection::SendBodyPart(CSMTPBodyPart* pBodyPart, BOOL bRoot)
{
  //we must convert all the body parts to the declared encoding.
    
  //Assume success
  BOOL bSuccess = TRUE;

  if (!bRoot)
  {
    //First send this body parts header
    LPSTR pszHeader = NULL;
    int nHeaderSize = 0;
    if (!pBodyPart->GetHeader(pszHeader, nHeaderSize))
    {
      CString sError;
      sError.Format(_T("CPJNSMTPConnection::SendBodyPart, Failed in call to send body parts header, GetLastError returns: %d"), GetLastError());
          OnError(sError);
          return FALSE;
    }
        if (!m_SMTP.Send(pszHeader, nHeaderSize))
        {
      CString sError;
      sError.Format(_T("CPJNSMTPConnection::SendBodyPart, Failed in call to send body parts header, GetLastError returns: %d"), GetLastError());
            OnError(sError);
            bSuccess = FALSE;
        }
    //Free up the temp memory we have used
    pBodyPart->FreeHeader(pszHeader);
  }
  
  //Then the body parts body
  LPSTR pszBody = NULL;
  int nBodySize = 0;
  if (!pBodyPart->GetBody(pszBody, nBodySize))
  {
    CString sError;
    sError.Format(_T("CPJNSMTPConnection::SendBodyPart, Failed in call to send body parts body, GetLastError returns: %d"), GetLastError());
        OnError(sError);
        return FALSE;
  }
  

  if (!m_SMTP.Send(pszBody, nBodySize))
  {
      CString sError;
      sError.Format(_T("CPJNSMTPConnection::SendBodyPart, Failed in call to send body parts body, GetLastError returns: %d"), GetLastError());
      OnError(sError);
      bSuccess = FALSE;
  }
  //Free up the temp memory we have used
  pBodyPart->FreeBody(pszBody);

  //The recursively send all the child body parts
  int nChildBodyParts = pBodyPart->GetNumberOfChildBodyParts();
  for (int i=0; i<nChildBodyParts && bSuccess; i++)
  {
    CSMTPBodyPart* pChildBodyPart = pBodyPart->GetChildBodyPart(i);
    bSuccess = SendBodyPart(pChildBodyPart, FALSE);
  }

  //Then the MIME footer if need be
  BOOL bSendFooter = (pBodyPart->GetNumberOfChildBodyParts() != 0);
  if (bSendFooter)
  {
    LPSTR pszFooter = NULL;
    int nFooterSize = 0;
    if (!pBodyPart->GetFooter(pszFooter, nFooterSize))
    {
      CString sError;
      sError.Format(_T("CPJNSMTPConnection::SendBodyPart, Failed in call to send body parts footer, GetLastError returns: %d"), GetLastError());
          OnError(sError);
          return FALSE;
    }
      if (!m_SMTP.Send(pszFooter, nFooterSize))
      {
      CString sError;
      sError.Format(_T("CPJNSMTPConnection::SendBodyPart, Failed in call to send body parts footer, GetLastError returns: %d"), GetLastError());
          OnError(sError);
          bSuccess = FALSE;
      }
    //Free up the temp memory we have used
    pBodyPart->FreeFooter(pszFooter);
  }
  
  return bSuccess;
}

BOOL CPJNSMTPConnection::SendMessage(CSMTPMessage& Message)
{
    //For correct operation of the T2A macro, see MFC Tech Note 59
    USES_CONVERSION;

    //paramater validity checking
  ASSERT(m_bConnected); //Must be connected to send a message

  //Send the MAIL command
    ASSERT(Message.m_From.m_sEmailAddress.GetLength());
  CString sBuf;
  sBuf.Format(_T("MAIL FROM:<%s>\r\n"), Message.m_From.m_sEmailAddress);
  LPCSTR pszMailFrom = T2A((LPTSTR) (LPCTSTR) sBuf);
  int nCmdLength = (int)strlen(pszMailFrom);
  if (!m_SMTP.Send(pszMailFrom, nCmdLength))
  {
    CString sError;
    sError.Format(_T("CPJNSMTPConnection::SendMessage, Failed in call to send MAIL command, GetLastError returns: %d"), GetLastError());
    OnError(sError);
    return FALSE;
  }

  //check the response to the MAIL command
  if (!ReadCommandResponse(250))
  {
    SetLastError(ERROR_BAD_COMMAND);
    OnError(_T("CPJNSMTPConnection::SendMessage, An unexpected MAIL response was received"));
    return FALSE;
  } 

  //Send the RCPT command, one for each recipient (includes the TO, CC & BCC recipients)

  //Must be sending to someone
  ASSERT(Message.GetNumberOfRecipients(CSMTPMessage::TO) + 
         Message.GetNumberOfRecipients(CSMTPMessage::CC) + 
         Message.GetNumberOfRecipients(CSMTPMessage::BCC));

  //First the "To" recipients
  for (int i=0; i<Message.GetNumberOfRecipients(CSMTPMessage::TO); i++)
  {
    CSMTPAddress* pRecipient = Message.GetRecipient(i, CSMTPMessage::TO);
    ASSERT(pRecipient);
    if (!SendRCPTForRecipient(*pRecipient))
      return FALSE;
  }

  //Then the "CC" recipients
  for (i=0; i<Message.GetNumberOfRecipients(CSMTPMessage::CC); i++)
  {
    CSMTPAddress* pRecipient = Message.GetRecipient(i, CSMTPMessage::CC);
    ASSERT(pRecipient);
    if (!SendRCPTForRecipient(*pRecipient))
      return FALSE;
  }

  //Then the "BCC" recipients
  for (i=0; i<Message.GetNumberOfRecipients(CSMTPMessage::BCC); i++)
  {
    CSMTPAddress* pRecipient = Message.GetRecipient(i, CSMTPMessage::BCC);
    ASSERT(pRecipient);
    if (!SendRCPTForRecipient(*pRecipient))
      return FALSE;
  }

  //Send the DATA command
  char* pszDataCommand = "DATA\r\n";
  nCmdLength = (int)strlen(pszDataCommand);
  if (!m_SMTP.Send(pszDataCommand, nCmdLength))
  {
    CString sError;
    sError.Format(_T("CPJNSMTPConnection::SendMessage, Failed in call to send MAIL command, GetLastError returns: %d"), GetLastError());
    OnError(sError);
    return FALSE;
  }

  //check the response to the DATA command
  if (!ReadCommandResponse(354))
  {
    SetLastError(ERROR_BAD_COMMAND);
    OnError(_T("CPJNSMTPConnection::SendMessage, An unexpected DATA response was received"));
    return FALSE;
  } 

  //Send the Message Header
  std::string sHeader = Message.getHeader();
  nCmdLength = (int)sHeader.length();
  if (!m_SMTP.Send(sHeader.c_str(), nCmdLength))
  {
    CString sError;
    sError.Format(_T("CPJNSMTPConnection::SendMessage, Failed in call to send the header, GetLastError returns: %d"), GetLastError());
    OnError(sError);
    return FALSE;
  }

    //Send the Header / body Separator
  char* pszBodyHeader = "\r\n";
  nCmdLength = (int)strlen(pszBodyHeader);
  if (!m_SMTP.Send(pszBodyHeader, nCmdLength))
  {
    CString sError;
    sError.Format(_T("CPJNSMTPConnection::SendMessage, Failed in call to send the header / body separator, GetLastError returns: %d"), GetLastError());
    OnError(sError);
    return FALSE;
  }

  //Now send the contents of the mail    
  BOOL bHasChildParts = (Message.m_RootPart.GetNumberOfChildBodyParts() != 0);
  if (bHasChildParts || Message.m_bMime)
  {
    //Send the root body part (and all its children)
    if (!SendBodyPart(&Message.m_RootPart, TRUE))
      return FALSE;
  }
  else
  {
      LPCSTR pszBody = T2CA(Message.m_RootPart.GetText());
    ASSERT(pszBody);
    nCmdLength = (int)strlen(pszBody);

    //Send the body
    if (!m_SMTP.Send(pszBody, nCmdLength))
    {
      CString sError;
      sError.Format(_T("CPJNSMTPConnection::SendMessage, Failed in call to send the header, GetLastError returns: %d"), GetLastError());
      OnError(sError);
      return FALSE;
    }
  }

  //Send the end of message indicator
  char* pszEOM = "\r\n.\r\n";
    nCmdLength = (int)strlen(pszEOM);
  if (!m_SMTP.Send(pszEOM, nCmdLength))
  {
    CString sError;
    sError.Format(_T("CPJNSMTPConnection::SendMessage, Failed in call to send end of message indicator, GetLastError returns: %d"), GetLastError());
    OnError(sError);
    return FALSE;
  }

  //check the response to the End of Message command
  if (!ReadCommandResponse(250))
  {
    SetLastError(ERROR_BAD_COMMAND);
    OnError(_T("CPJNSMTPConnection::SendMessage, An unexpected end of message response was received"));
    return FALSE;
  } 

    return TRUE;
}


#if (_MFC_VER >= 0x700)
BOOL CPJNSMTPConnection::OnSendProgress(DWORD /*dwCurrentBytes*/, ULONGLONG /*dwTotalBytes*/)
#else
BOOL CPJNSMTPConnection::OnSendProgress(DWORD /*dwCurrentBytes*/, DWORD /*dwTotalBytes*/)
#endif
{
  //By default just return TRUE to allow the mail to continue to be sent
  return TRUE; 
}

BOOL CPJNSMTPConnection::SendMessage(BYTE* pMessage, DWORD dwTotalBytes, CSMTPAddressArray& Recipients, const CSMTPAddress& From, DWORD dwSendBufferSize)
{
    //For correct operation of the T2A macro, see MFC Tech Note 59
    USES_CONVERSION;

    //paramater validity checking
  ASSERT(m_bConnected); //Must be connected to send a message

  //Send the MAIL command
    ASSERT(From.m_sEmailAddress.GetLength());
  CString sBuf;
  sBuf.Format(_T("MAIL FROM:<%s>\r\n"), From.m_sEmailAddress);
  LPCSTR pszMailFrom = T2A((LPTSTR) (LPCTSTR) sBuf);
  int nCmdLength = (int)strlen(pszMailFrom);
  if (!m_SMTP.Send(pszMailFrom, nCmdLength))
  {
    CString sError;
    sError.Format(_T("CPJNSMTPConnection::SendMessage, Failed in call to send MAIL command, GetLastError returns: %d"), GetLastError());
    OnError(sError);
    return FALSE;
  }

  //check the response to the MAIL command
  if (!ReadCommandResponse(250))
  {
    SetLastError(ERROR_BAD_COMMAND);
    OnError(_T("CPJNSMTPConnection::SendMessage, An unexpected MAIL response was received"));
    return FALSE;
  } 

  //Must be sending to someone
  int nRecipients = (int)Recipients.GetSize();
  ASSERT(nRecipients);

  //Send the RCPT command, one for each recipient
  for (int i=0; i<nRecipients; i++)
  {
    CSMTPAddress& recipient = Recipients.ElementAt(i);
    if (!SendRCPTForRecipient(recipient))
      return FALSE;
  }

  //Send the DATA command
  char* pszDataCommand = "DATA\r\n";
  nCmdLength = (int)strlen(pszDataCommand);
  if (!m_SMTP.Send(pszDataCommand, nCmdLength))
  {
    CString sError;
    sError.Format(_T("CPJNSMTPConnection::SendMessage, Failed in call to send MAIL command, GetLastError returns: %d"), GetLastError());
    OnError(sError);
    return FALSE;
  }

  //check the response to the DATA command
  if (!ReadCommandResponse(354))
  {
    SetLastError(ERROR_BAD_COMMAND);
    OnError(_T("CPJNSMTPConnection::SendMessage, An unexpected DATA response was received"));
    return FALSE;
  } 

  //Read and send the data a chunk at a time
  BOOL bMore = TRUE;
  BOOL bSuccess = TRUE;
  DWORD dwBytesSent = 0;
  BYTE* pSendBuf = pMessage; 
  do
  {
    DWORD dwRead = min(dwSendBufferSize, dwTotalBytes-dwBytesSent);
    dwBytesSent += dwRead;

    //Call the progress virtual method
    if (OnSendProgress(dwBytesSent, dwTotalBytes))
    {
      if (!m_SMTP.Send((LPCSTR)pSendBuf, dwRead))
      {
        CString sError;
        sError.Format(_T("CPJNSMTPConnection::SendMessage, Failed in call to send the message, GetLastError returns: %d"), GetLastError());
        OnError(sError);
        bSuccess = FALSE;
      }
    }
    else
    {
      //Abort the mail send (due to the progress virtual method returning FALSE
      bSuccess = FALSE;
    }

    //Prepare for the next time around
    pSendBuf += dwRead;
    bMore = (dwBytesSent < dwTotalBytes);
  }
  while (bMore && bSuccess);

  if (bSuccess)
  {
    //Send the end of message indicator
    char* pszEOM = "\r\n.\r\n";
      nCmdLength = (int)strlen(pszEOM);
    if (!m_SMTP.Send(pszEOM, nCmdLength))
    {
      CString sError;
      sError.Format(_T("CPJNSMTPConnection::SendMessage, Failed in call to send end of message indicator, GetLastError returns: %d"), GetLastError());
      OnError(sError);
      return FALSE;
    }

    //check the response to the End of Message command
    if (!ReadCommandResponse(250))
    {
      SetLastError(ERROR_BAD_COMMAND);
      OnError(_T("CPJNSMTPConnection::SendMessage, An unexpected end of message response was received"));
      return FALSE;
    } 

    return TRUE;
  }
  else
    return FALSE;
}

void CPJNSMTPConnection::SafeCloseFile(CFile& file, const CString& sError)
{
  //Explicitly handle the closing of the file here with an exception handler
  try
  {
    file.Close();
  }
  catch(CFileException* pEx)
  {
    pEx->Delete();
    OnError(sError);
  }
}

BOOL CPJNSMTPConnection::SendMessage(const CString& sMessageOnFile, CSMTPAddressArray& Recipients, const CSMTPAddress& From, DWORD dwSendBufferSize)
{
    //For correct operation of the T2A macro, see MFC Tech Note 59
    USES_CONVERSION;

    //paramater validity checking
  ASSERT(m_bConnected); //Must be connected to send a message

  //Open up the file
  CFile mailFile;
  if (mailFile.Open(sMessageOnFile, CFile::modeRead | CFile::shareDenyWrite))
  {
    //Form a string which we need in the calls to SafeCloseFile
    CString sTraceMsgOnClose;
    sTraceMsgOnClose.Format(_T("CPJNSMTPConnection::SendMessage, A file exception occured while closing the file when sending the message from the file %s\n"), sMessageOnFile);

    //Get the length of the file
    #if (_MFC_VER >= 0x700)
    ULONGLONG ullTotalBytes = 0;
    #else
    DWORD dwTotalBytes = 0;
    #endif
    try
    {
      #if (_MFC_VER >= 0x700)
      ullTotalBytes = mailFile.GetLength();
      #else  
      dwTotalBytes = mailFile.GetLength();
      #endif
    }
    catch (CFileException* pEx)
    {
      pEx->Delete();
      CString sError;
      sError.Format(_T("CPJNSMTPConnection::SendMessage, Failed to get the length of the file, %s"), sMessageOnFile);
      OnError(sError);
    }

    //Only try sending the mail if there is anything in the body
    #if (_MFC_VER >= 0x700)
    if (ullTotalBytes)
    #else
    if (dwTotalBytes)
    #endif
    {
      //Send the MAIL command
        ASSERT(From.m_sEmailAddress.GetLength());
      CString sBuf;
      sBuf.Format(_T("MAIL FROM:<%s>\r\n"), From.m_sEmailAddress);
      LPCSTR pszMailFrom = T2A((LPTSTR) (LPCTSTR) sBuf);
      int nCmdLength = (int)strlen(pszMailFrom);
      if (!m_SMTP.Send(pszMailFrom, nCmdLength))
      {
        CString sError;
        sError.Format(_T("CPJNSMTPConnection::SendMessage, Failed in call to send MAIL command, GetLastError returns: %d"), GetLastError());
        OnError(sError);
        SafeCloseFile(mailFile, sTraceMsgOnClose);
        return FALSE;
      }

      //check the response to the MAIL command
      if (!ReadCommandResponse(250))
      {
        SetLastError(ERROR_BAD_COMMAND);
        OnError(_T("CPJNSMTPConnection::SendMessage, An unexpected MAIL response was received"));
        SafeCloseFile(mailFile, sTraceMsgOnClose);
        return FALSE;
      } 

      //Must be sending to someone
      int nRecipients = (int)Recipients.GetSize();
      ASSERT(nRecipients);

      //Send the RCPT command, one for each recipient
      for (int i=0; i<nRecipients; i++)
      {
        CSMTPAddress& recipient = Recipients.ElementAt(i);
        if (!SendRCPTForRecipient(recipient))
        {
          SafeCloseFile(mailFile, sTraceMsgOnClose);
          return FALSE;
        }
      }

      //Send the DATA command
      char* pszDataCommand = "DATA\r\n";
      nCmdLength = (int)strlen(pszDataCommand);
      if (!m_SMTP.Send(pszDataCommand, nCmdLength))
      {
        CString sError;
        sError.Format(_T("CPJNSMTPConnection::SendMessage, Failed in call to send MAIL command, GetLastError returns: %d"), GetLastError());
        OnError(sError);
        SafeCloseFile(mailFile, sTraceMsgOnClose);
        return FALSE;
      }

      //check the response to the DATA command
      if (!ReadCommandResponse(354))
      {
        SetLastError(ERROR_BAD_COMMAND);
        OnError(_T("CPJNSMTPConnection::SendMessage, An unexpected DATA response was received"));
        SafeCloseFile(mailFile, sTraceMsgOnClose);
        return FALSE;
      } 

      //Allocate a buffer we will use in the sending
      char* pSendBuf = new char[dwSendBufferSize];

      //Read and send the data a chunk at a time
      BOOL bMore = TRUE;
      BOOL bSuccess = TRUE;
      int nBytesSent = 0; 
      do
      {
        try
        {
          //Read the chunk from file
          UINT nRead = mailFile.Read(pSendBuf, dwSendBufferSize);
          bMore = (nRead == dwSendBufferSize);

          //Send the chunk
          if (nRead)
          {
            nBytesSent += nRead;

            //Call the progress virtual method
            #if (_MFC_VER >= 0x700)
            if (OnSendProgress(nBytesSent, ullTotalBytes))
            #else
            if (OnSendProgress(nBytesSent, dwTotalBytes))
            #endif
            {
              if (!m_SMTP.Send(pSendBuf, nRead))
              {
                CString sError;
                sError.Format(_T("CPJNSMTPConnection::SendMessage, Failed in call to send the message, GetLastError returns: %d"), GetLastError());
                OnError(sError);
                SafeCloseFile(mailFile, sTraceMsgOnClose);
                bSuccess = FALSE;
              }
            }
            else
            {
              //Abort the mail send (due to the progress virtual method returning FALSE
              bSuccess = FALSE;
            }
          }
        }
        catch(CFileException* pEx)
        {
          bSuccess = FALSE;
          CString sError;
          sError.Format(_T("CPJNSMTPConnection::SendMessage, An error occurred reading from the file to send %s"), sMessageOnFile);
          OnError(sError);
          pEx->Delete();
        }
      }
      while (bMore && bSuccess);

      //Explicitly handle the closing of the file here with an exception handler
      SafeCloseFile(mailFile, sTraceMsgOnClose);

      //Tidy up the heap memory we have used
      delete [] pSendBuf;

      if (bSuccess)
      {
        //Send the end of message indicator
        char* pszEOM = "\r\n.\r\n";
          nCmdLength = (int)strlen(pszEOM);
        if (!m_SMTP.Send(pszEOM, nCmdLength))
        {
          CString sError;
          sError.Format(_T("CPJNSMTPConnection::SendMessage, Failed in call to send end of message indicator, GetLastError returns: %d"), GetLastError());
          OnError(sError);
          return FALSE;
        }

        //check the response to the End of Message command
        if (!ReadCommandResponse(250))
        {
          SetLastError(ERROR_BAD_COMMAND);
          OnError(_T("CPJNSMTPConnection::SendMessage, An unexpected end of message response was received"));
          return FALSE;
        } 

          return TRUE;
      }
      else
        return FALSE;
    }
    else
    {
      CString sError;
      sError.Format(_T("CPJNSMTPConnection::SendMessage, Could not send the file %s since it is 0 bytes in size"), sMessageOnFile);
      OnError(sError); 
      SafeCloseFile(mailFile, sTraceMsgOnClose);
      return FALSE;
    }
  }
  else
  {
    CString sError;
    sError.Format(_T("CPJNSMTPConnection::SendMessage, Could not open the file to send %s"), sMessageOnFile);
    OnError(sError);
    return FALSE;
  }
}

BOOL CPJNSMTPConnection::SendRCPTForRecipient(CSMTPAddress& recipient)
{
    //For correct operation of the T2A macro, see MFC Tech Note 59
    USES_CONVERSION;

    ASSERT(recipient.m_sEmailAddress.GetLength()); //must have an email address for this recipient

  CString sBuf;
  sBuf.Format(_T("RCPT TO:<%s>\r\n"), recipient.m_sEmailAddress);
  LPSTR pszRCPT = T2A((LPTSTR) (LPCTSTR) sBuf);

  int nCmdLength = (int)strlen(pszRCPT);
  if (!m_SMTP.Send(pszRCPT, nCmdLength))
  {
    CString sError;
    sError.Format(_T("CPJNSMTPConnection::SendRCPTForRecipient, Failed in call to send RCPT command, GetLastError returns: %d"), GetLastError());
    OnError(sError);
    return FALSE;
  }

  //check the response to the RCPT command
  if (!ReadCommandResponse(250))
  {
    SetLastError(ERROR_BAD_COMMAND);
    OnError(_T("CPJNSMTPConnection::SendRCPTForRecipient, An unexpected RCPT response was received"));
    return FALSE;
  } 
  
  return TRUE;
}

BOOL CPJNSMTPConnection::ReadCommandResponse(int nExpectedCode, BOOL bEHLO)
{
  LPSTR pszOverFlowBuffer = NULL;
  char sBuf[256];
  BOOL bSuccess = ReadResponse(sBuf, 256, "\r\n", nExpectedCode, &pszOverFlowBuffer, 4096, bEHLO);
  if (pszOverFlowBuffer)
    delete [] pszOverFlowBuffer;

  return bSuccess;
}

BOOL CPJNSMTPConnection::ReadResponse(LPSTR pszBuffer, int nInitialBufSize, LPSTR pszTerminator, int nExpectedCode, LPSTR* ppszOverFlowBuffer, int nGrowBy, BOOL bEHLO)
{
    ASSERT(ppszOverFlowBuffer);          //Must have a valid string pointer
    ASSERT(*ppszOverFlowBuffer == NULL); //Initially it must point to a NULL string

    //must have been created first
    ASSERT(m_bConnected);

    //Get length of terminator for later use
    int nTerminatorLen = (int)strlen(pszTerminator);

    //The local variables which will receive the data
    LPSTR pszRecvBuffer = pszBuffer;
    int nBufSize = nInitialBufSize;

    //retrieve the reponse using until we
    //get the terminator or a timeout occurs
    BOOL bFoundTerminator = FALSE;
    int nReceived = 0;
    while (!bFoundTerminator)
    {
        //check the socket for readability
        BOOL bReadible;
        if (!m_SMTP.IsReadable(bReadible, m_dwTimeout))
        {
            pszRecvBuffer[nReceived] = '\0';
            m_sLastCommandResponse = pszRecvBuffer; //Hive away the last command reponse
            return FALSE;
        }
        else if (!bReadible) //A timeout has occured so fail the function call
        {
            pszRecvBuffer[nReceived] = '\0';
            SetLastError(WSAETIMEDOUT);
            OnError(_T("CPJNSMTPConnection::ReadResponse, Timed Out waiting for response from SMTP server"));
            m_sLastCommandResponse = pszRecvBuffer; //Hive away the last command reponse
            return FALSE;
        }

        //receive the data from the socket
        int nBufRemaining = nBufSize-nReceived-1; //Allows allow one space for the NULL terminator
        if (nBufRemaining<0)
            nBufRemaining = 0;
        int nData = m_SMTP.Receive(pszRecvBuffer+nReceived, nBufRemaining);

        //Reset the idle timeout if data was received
        if (nData > 0)
        {
            //Increment the count of data received
            nReceived += nData;                            
        }

        //If an error occurred receiving the data
        if (nData < 1)
        {
            //NULL terminate the data received
            if (pszRecvBuffer)
            pszBuffer[nReceived] = '\0';
            m_sLastCommandResponse = pszRecvBuffer; //Hive away the last command reponse
            return FALSE; 
        }
        else
        {
            //NULL terminate the data received
            if (pszRecvBuffer)
              pszRecvBuffer[nReceived] = '\0';
            if (nBufRemaining-nData == 0) //No space left in the current buffer
            {
                //Allocate the new receive buffer
                nBufSize += nGrowBy; //Grow the buffer by the specified amount
                LPSTR pszNewBuf = new char[nBufSize];

                //copy the old contents over to the new buffer and assign 
                //the new buffer to the local variable used for retreiving 
                //from the socket
                if (pszRecvBuffer)
                  strcpy(pszNewBuf, pszRecvBuffer);
                pszRecvBuffer = pszNewBuf;

                //delete the old buffer if it was allocated
                if (*ppszOverFlowBuffer)
                  delete [] *ppszOverFlowBuffer;

                //Remember the overflow buffer for the next time around
                *ppszOverFlowBuffer = pszNewBuf;        
            }
        }

        // Check to see if the terminator character(s) have been found
        // (at the END of the response! otherwise read loop will terminate
        // early if there are multiple lines in the response)
        bFoundTerminator = (strncmp( &pszRecvBuffer[ nReceived - nTerminatorLen ], pszTerminator, nTerminatorLen ) == 0);

    if (bEHLO)
            bFoundTerminator &= strstr(pszRecvBuffer, "250 ") != NULL;
    }

    //Remove the terminator from the response data
  pszRecvBuffer[ nReceived - nTerminatorLen ] = '\0';

    // handle multi-line responses
    BOOL bSuccess = FALSE;
    do
    {
        // determine if the response is an error
        char sCode[4];
        strncpy(sCode, pszRecvBuffer, 3);
        sCode[3] = '\0';
        sscanf(sCode, "%d", &m_nLastCommandResponseCode );
        bSuccess = (m_nLastCommandResponseCode == nExpectedCode);

        // Hive away the last command reponse
        m_sLastCommandResponse = pszRecvBuffer;

        // see if there are more lines to this response
        pszRecvBuffer = strstr( pszRecvBuffer, pszTerminator );
        if (pszRecvBuffer)
            pszRecvBuffer += nTerminatorLen;    // skip past terminator

    } while ( !bSuccess && pszRecvBuffer );

    if (!bSuccess)
        SetLastError(WSAEPROTONOSUPPORT);

    return bSuccess;
}

// This function negotiates AUTH LOGIN
BOOL CPJNSMTPConnection::AuthLogin(LPCTSTR pszUsername, LPCTSTR pszPassword)
{
    //For correct operation of the T2A macro, see MFC Tech Note 59
    USES_CONVERSION;

  //Send the AUTH LOGIN command
    CString sBuf(_T("AUTH LOGIN\r\n"));
    LPCSTR pszData = T2A((LPTSTR) (LPCTSTR) sBuf);
    int nCmdLength = (int)strlen(pszData);
    if (!m_SMTP.Send(pszData, nCmdLength))
    {
      OnError(_T("CPJNSMTPConnection::AuthLogin, An unexpected error occurred while sending the AUTH command"));
      return FALSE;
    }

    //initialize the base64 encoder / decoder
    CBase64Coder Coder;
    if (!ReadCommandResponse(334))
  {
      OnError(_T("CPJNSMTPConnection::AuthLogin, Server does not support AUTH LOGIN"));
      return FALSE;
  }
    else
    {
        //send base64 encoded username
        CString sLastCommandString = m_sLastCommandResponse;
        sLastCommandString = sLastCommandString.Right(sLastCommandString.GetLength() - 4);
    LPCSTR pszLastCommandString = T2A((LPTSTR) (LPCTSTR) sLastCommandString);
        Coder.Decode(pszLastCommandString);
        if (stricmp(Coder.DecodedMessage(), "username:") == 0)
        {
            Coder.Encode(T2A((LPTSTR)pszUsername));
            sBuf.Format(_T("%s\r\n"), A2T(Coder.EncodedMessage()));
            pszData = T2A((LPTSTR) (LPCTSTR) sBuf);
            nCmdLength = (int)strlen(pszData);
            if (!m_SMTP.Send(pszData, nCmdLength))
            {
              OnError(_T("CPJNSMTPConnection::AuthLogin, An unexpected error occurred while sending the username"));
              return FALSE;
            }
        }
        else
        {
            OnError(_T("CPJNSMTPConnection::AuthLogin, An unexpected request received when expecting username request"));
            return FALSE;
        }
    }

    //check the response to the username 
    if (!ReadCommandResponse(334))
  {
      OnError(_T("CPJNSMTPConnection::AuthLogin, Server did not respond correctly to AUTH LOGIN username field"));
      return FALSE;
  }
    else
    {
        //send password as base64 encoded
        CString sLastCommandString = m_sLastCommandResponse;
        sLastCommandString = sLastCommandString.Right(sLastCommandString.GetLength() - 4);
    LPCSTR pszLastCommandString = T2A((LPTSTR) (LPCTSTR) sLastCommandString);
        Coder.Decode(pszLastCommandString);
        if (stricmp(Coder.DecodedMessage(), "password:") == 0)
        {
            Coder.Encode(T2A((LPTSTR)pszPassword));
            sBuf.Format(_T("%s\r\n"), A2T(Coder.EncodedMessage()));
            pszData = T2A((LPTSTR) (LPCTSTR) sBuf);
            nCmdLength = (int)strlen(pszData);
            if (!m_SMTP.Send(pszData, nCmdLength))
            {
              OnError(_T("CPJNSMTPConnection::AuthLogin, An unexpected error occurred while sending the password"));
              return FALSE;
            }

            //check if authentication is successful
            if (!ReadCommandResponse(235))
      {
        OnError(_T("CPJNSMTPConnection::AuthLogin, AUTH LOGIN authentication was unsuccessful"));
                return FALSE;
      }
        }
        else
        {
            OnError(_T("CPJNSMTPConnection::AuthLogin, An unexpected request received when expecting password request"));
            return FALSE;
        }
    }

    return TRUE;
}

// This function negotiates AUTH LOGIN PLAIN
BOOL CPJNSMTPConnection::AuthLoginPlain(LPCTSTR pszUsername, LPCTSTR pszPassword)
{
    //For correct operation of the T2A macro, see MFC Tech Note 59
    USES_CONVERSION;

  //Send the AUTH LOGIN PLAIN command
    CString sBuf(_T("AUTH LOGIN PLAIN\r\n"));
    LPCSTR pszData = T2A((LPTSTR) (LPCTSTR) sBuf);
    int nCmdLength = (int)strlen(pszData);
    if (!m_SMTP.Send(pszData, nCmdLength))
    {
      OnError(_T("CPJNSMTPConnection::AuthLoginPlain, An unexpected error occurred while sending the AUTH command"));
      return FALSE;
    }

    if (!ReadCommandResponse(334))
  {
      OnError(_T("CPJNSMTPConnection::AuthLoginPlain, Server does not support AUTH LOGIN PLAIN"));
      return FALSE;
  }
    else
    {
        //send username in plain
        CString sLastCommandString = m_sLastCommandResponse;
        sLastCommandString = sLastCommandString.Right(sLastCommandString.GetLength() - 4);
    LPCSTR pszLastCommandString = T2A((LPTSTR) (LPCTSTR) sLastCommandString);
        if (stricmp(pszLastCommandString, "username:") == 0)
        {
            sBuf.Format(_T("%s\r\n"), pszUsername);
            pszData = T2A((LPTSTR) (LPCTSTR) sBuf);
            nCmdLength = (int)strlen(pszData);
            if (!m_SMTP.Send(pszData, nCmdLength))
            {
              OnError(_T("CPJNSMTPConnection::AuthLoginPlain, An unexpected error occurred while sending the username"));
              return FALSE;
            }
        }
    }

    //check the response to the username 
    if (!ReadCommandResponse(334))
  {
      OnError(_T("CPJNSMTPConnection::AuthLoginPlain, Server did not response correctly to AUTH LOGIN PLAIN username field"));
      return FALSE;
  }
    else
    {
        //send password in plain
        CString sLastCommandString = m_sLastCommandResponse;
        sLastCommandString = sLastCommandString.Right(sLastCommandString.GetLength() - 4);
    LPCSTR pszLastCommandString = T2A((LPTSTR) (LPCTSTR) sLastCommandString);
        if (stricmp(pszLastCommandString, "password:") == 0)
        {
            sBuf.Format(_T("%s\r\n"), pszPassword);
            pszData = T2A((LPTSTR) (LPCTSTR) sBuf);
            nCmdLength = (int)strlen(pszData);
            if (!m_SMTP.Send(pszData, nCmdLength))
            {
              OnError(_T("CPJNSMTPConnection::AuthLoginPlain, An unexpected error occurred while sending the password"));
              return FALSE;
            }

            // check if authentication is successful
            if (!ReadCommandResponse(235))
      {
        OnError(_T("CPJNSMTPConnection::AuthLoginPlain, AUTH LOGIN PLAIN authentication was unsuccessful"));
                return FALSE;
      }
        }
    }

    return TRUE;
}

#ifndef CSMTP_NORSA
BOOL CPJNSMTPConnection::CramLogin(LPCTSTR pszUsername, LPCTSTR pszPassword)
{
    //For correct operation of the T2A macro, see MFC Tech Note 59
    USES_CONVERSION;

  //Send the AUTH CRAM-MD5 command
    CString sBuf(_T("AUTH CRAM-MD5\r\n"));
    LPCSTR pszData = T2A((LPTSTR) (LPCTSTR) sBuf);
    int nCmdLength = (int)strlen(pszData);
    if (!m_SMTP.Send(pszData, nCmdLength))
    {
      OnError(_T("CPJNSMTPConnection::CramLogin, An unexpected error occurred while sending the AUTH command"));
      return FALSE;
    }

    // initialize base64 encoder / decoder
    CBase64Coder Coder;
    if (!ReadCommandResponse(334))
  {
      OnError(_T("CPJNSMTPConnection::CramLogin, Server does not support AUTH CRAM-MD5"));
      return FALSE;
  }
    else
    {
        CString sLastCommandString = m_sLastCommandResponse;
        sLastCommandString = sLastCommandString.Right(sLastCommandString.GetLength() - 4);
    LPCSTR pszLastCommandString = T2A((LPTSTR) (LPCTSTR) sLastCommandString);
        Coder.Decode(pszLastCommandString);

        // Get the base64 decoded challange 
        LPCSTR pszChallenge = Coder.DecodedMessage();

        // test data as per RFC 2195
    //      pszChallenge = "<1896.697170952@postoffice.reston.mci.net>";
    //      pszUsername = "tim";
    //      pszAsciiPassword = "tanstaaftanstaaf";
        // generate the MD5 digest from the challange and password
        unsigned char digest[16];    // message digest
    LPSTR pszAsciiPassword = T2A((LPTSTR)pszPassword);
        MD5Digest((unsigned char*) pszChallenge, (int)strlen(pszChallenge), 
              (unsigned char*) pszAsciiPassword, (int)strlen(pszAsciiPassword), digest);
        
        // make the CRAM-MD5 response
        CString sCramDigest;
        sCramDigest = pszUsername;
        sCramDigest += " ";
        for (int i=0; i<16; i++)
        {
          CString csTemp;
            csTemp.Format(_T("%02x"), digest[i]);
            sCramDigest += csTemp;
        }
            
        // send base64 encoded username digest
    LPSTR pszCramDigest = T2A((LPTSTR) (LPCTSTR) sCramDigest);
        Coder.Encode(pszCramDigest);
        sBuf.Format(_T("%s\r\n"), A2T(Coder.EncodedMessage()));
        pszData = T2A((LPTSTR) (LPCTSTR) sBuf);
        nCmdLength = (int)strlen(pszData);
        if (!m_SMTP.Send(pszData, nCmdLength))
        {
          OnError(_T("CPJNSMTPConnection::CramLogin, An unexpected error occurred while sending the username"));
          return FALSE;
        }
    }

    // check if authentication is successful
    if (!ReadCommandResponse(235))
  {
    OnError(_T("CPJNSMTPConnection::CramLogin, AUTH CRAM-MD5 authentication was unsuccessful"));
        return FALSE;
  }

    return TRUE;
}
#endif

#ifndef CSMTP_NORSA
void CPJNSMTPConnection::MD5Digest(unsigned char* text, int text_len, unsigned char* key, int key_len, unsigned char* digest)
{
  unsigned char tk[16];

  // if key is longer than 64 bytes reset it to key=MD5(key)
  if (key_len > 64) 
  {
    MD5_CTX tctx;
    MD5Init(&tctx);
    MD5Update(&tctx, key, key_len);
    MD5Final(tk, &tctx);

    key = tk;
    key_len = 16;
  }

  // the HMAC_MD5 transform looks like:
  //
  // MD5(K XOR opad, MD5(K XOR ipad, text))
  //
  // where K is an n byte key
  // ipad is the byte 0x36 repeated 64 times
  // opad is the byte 0x5c repeated 64 times
  // and text is the data being protected

  //start out by storing key in pads
  unsigned char k_ipad[65];    // inner padding - key XORd with ipad
  memset(k_ipad, 0, 64);
  unsigned char k_opad[65];    // outer padding - key XORd with opad
  memset(k_opad, 0, 64);
  memcpy(k_ipad, key, key_len);
  memcpy(k_opad, key, key_len);

  // XOR key with ipad and opad values
  for (int i=0; i<64; i++) 
  {
    k_ipad[i] ^= 0x36;
    k_opad[i] ^= 0x5c;
  }

  //perform inner MD5
  MD5_CTX context;
  MD5Init(&context);                   // init context for 1st pass
  MD5Update(&context, k_ipad, 64);     // start with inner pad
  MD5Update(&context, text, text_len); // then text of datagram
  MD5Final(digest, &context);          // finish up 1st pass

  //perform outer MD5
  MD5Init(&context);                   // init context for 2nd pass
  MD5Update(&context, k_opad, 64);     // start with outer pad
  MD5Update(&context, digest, 16);     // then results of 1st hash
  MD5Final(digest, &context);          // finish up 2nd pass
}
#endif

CPJNSMTPConnection::ConnectToInternetResult CPJNSMTPConnection::ConnectToInternet()
{
  if (_WinInetData.m_lpfnInternetGetConnectedState && _WinInetData.m_lpfnInternetAttemptConnect)
  {
    //Check to see if an internet connection already exists.
    //bInternet = TRUE  internet connection exists.
    //bInternet = FALSE internet connection does not exist
    DWORD dwFlags = 0;
    BOOL bInternet = _WinInetData.m_lpfnInternetGetConnectedState(&dwFlags, 0);
    if (!bInternet)
    {
      //Attempt to establish internet connection, probably
      //using Dial-up connection. CloseInternetConnection() should be called when
      //as some time to drop the dial-up connection.
      DWORD dwResult = _WinInetData.m_lpfnInternetAttemptConnect(0);
      if (dwResult != ERROR_SUCCESS)
      {
        SetLastError(dwResult);
        return CTIR_Failure;
      }
      else
        return CTIR_NewConnection;
    }
    else
      return CTIR_ExistingConnection;
  }
  else
  {
    //Wininet is not available. Do what would happen if the dll
    //was present but the function call failed
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);  
    return CTIR_Failure;
  }
}

BOOL CPJNSMTPConnection::CloseInternetConnection()
{
  if (_WinInetData.m_lpfnInternetAutoDialHangup)
  {
    //Make sure any connection through a modem is 'closed'.
    return _WinInetData.m_lpfnInternetAutoDialHangup(0);
  }
  else
  {
    //Wininet is not available. Do what would happen if the dll
    //was present but the function call failed
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);  
    return FALSE;
  }
}

