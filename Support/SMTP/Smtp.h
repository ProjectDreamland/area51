/*
Module : SMTP.H
Purpose: Defines the interface for a MFC class encapsulation of the SMTP protocol
Created: PJN / 22-05-1998

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


/////////////////////////////// Defines ///////////////////////////////////////

#pragma once

#ifndef __SMTP_H__
#define __SMTP_H__

#ifndef __AFXTEMPL_H__
//#pragma message("To avoid this message, put afxtempl.h in your PCH (usually stdafx.h)")
#include <afxtempl.h>
#endif

#ifndef _WINSOCKAPI_
//#pragma message("To avoid this message, put afxsock.h or winsock.h in your PCH (usually stdafx.h)")
#include <winsock.h>
#endif

#ifndef __AFXPRIV_H__
//#pragma message("To avoid this message, put afxpriv.h in your PCH (usually stdafx.h)")
#include <afxpriv.h>
#endif

#pragma warning(push, 3) //Avoid all the level 4 warnings in STL
#ifndef _STRING_
//#pragma message("To avoid this message, put string in your PCH (usually stdafx.h)")
#include <string>
#endif
#pragma warning(pop)

#include "Base64Coder.h"



/////////////////////////////// Classes ///////////////////////////////////////

// Simple Socket wrapper class
class CSMTPSocket
{
public:
    // Constructors / Destructors
    CSMTPSocket();
    virtual ~CSMTPSocket();

    // Methods
    BOOL  Create();
    BOOL  Connect(LPCTSTR pszHostAddress, int nPort, LPCTSTR pszLocalBoundAddress);
    BOOL  Send(LPCSTR pszBuf, int nBuf);
    void  Close();
    int   Receive(LPSTR pszBuf, int nBuf);
    BOOL  IsReadable(BOOL& bReadible, DWORD dwTimeout);

protected:
    BOOL   Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen);
    SOCKET m_hSocket;
};

                     
//Encapsulation of an SMTP email address, used for recipients and in the From: field
class CSMTPAddress
{
public: 
    // Constructors / Destructors
    CSMTPAddress();
    CSMTPAddress(const CSMTPAddress& address);
    CSMTPAddress(const CString& sAddress);
    CSMTPAddress(const CString& sFriendly, const CString& sAddress);

    CSMTPAddress& operator=(const CSMTPAddress& r);

    // Methods
    CString GetRegularFormat(BOOL bEncode, const CString& sCharset) const;

    //Data members
    CString m_sFriendlyName; //Would set it to contain something like "PJ Naughter"
    CString m_sEmailAddress; //Would set it to contains something like "pjna@naughter.com"
};


// Encapsulatation of an SMTP MIME body part
class CSMTPBodyPart
{
public:
    // Constructors / Destructors
    CSMTPBodyPart();
    CSMTPBodyPart(const CSMTPBodyPart& bodyPart);
    virtual ~CSMTPBodyPart();

    CSMTPBodyPart& operator=(const CSMTPBodyPart& bodyPart);

//Accessors / Mutators
    BOOL    SetFilename(const CString& sFilename);
    CString GetFilename() const { return m_sFilename; }; 

    void    SetText(const CString& sText);
    CString GetText() const { return m_sText; };

    void    SetTitle(const CString& sTitle) { m_sTitle = sTitle; };
    CString GetTitle() const { return m_sTitle; };

    void    SetContentType(const CString& sContentType) { m_sContentType = sContentType; };
    CString GetContentType() const { return m_sContentType; };

    void    SetCharset(const CString& sCharset) { m_sCharset = sCharset; };
    CString GetCharset() const { return m_sCharset; };

    void    SetContentBase(const CString& sContentBase) { m_sContentBase = sContentBase; };
    CString GetContentBase() const { return m_sContentBase; };

    void    SetContentID(const CString& sContentID);
    CString GetContentID() const;

    void    SetContentLocation(const CString& sContentLocation);
    CString GetContentLocation() const;

    CString GetBoundary() const { return m_sBoundary; };

    // Misc methods
    BOOL            GetHeader               (LPSTR& pszHeader, int& nHeaderSize);
    BOOL            GetBody                 (LPSTR& pszBody, int& nBodySize);
    BOOL            GetFooter               (LPSTR& pszFooter, int& nFooterSize);
    void            FreeHeader              (LPSTR& pszHeader);
    void            FreeBody                (LPSTR& pszBody);
    void            FreeFooter              (LPSTR& pszFooter);
    CSMTPBodyPart*  FindFirstBodyPart       (const CString sContentType);
    void            SetQuotedPrintable      (BOOL bValue) { m_bQuotedPrintable = bValue; };
    BOOL            GetQuotedPrintable      () const { return m_bQuotedPrintable; };
    void            SetBase64               (BOOL bValue) { m_bBase64 = bValue; };
    BOOL            GetBase64               () const { return m_bBase64; };
    void            SetMaxAttachmentSize    (DWORD dwSize) { m_dwMaxAttachmentSize = dwSize; };
    DWORD           GetMaxAttachementSize   () const { return m_dwMaxAttachmentSize; };

//Child Body part methods
    int             GetNumberOfChildBodyParts   () const;
    int             AddChildBodyPart            (CSMTPBodyPart& bodyPart);
    void            RemoveChildBodyPart         (int nIndex);
    CSMTPBodyPart*  GetChildBodyPart            (int nIndex);
    CSMTPBodyPart*  GetParentBodyPart           ();

//Static methods
static std::string  QuotedPrintableEncode   (const std::string& sText);
static int          ConvertToUTF8           (const CString& in, std::string &);
static int          UnicodeToUTF8           (LPCWSTR wszSrc, int nSrc, LPSTR szDest,int nDest);
static char         HexDigit                (int nDigit);
static std::string  HeaderEncode            (const CString& sText, const CString& sCharset);
static std::string  QEncode                 (LPCSTR sText, LPCSTR sCharset);

protected:
    // Member variables
    CString      m_sFilename;                                 //The file you want to attach
    CString      m_sTitle;                                    //What is it to be know as when emailed
    CString      m_sContentType;                              //The mime content type for this body part
    CString      m_sCharset;                                  //The charset for this body part
    CString      m_sContentBase;                              //The absolute URL to use for when you need to resolve any relative URL's in this body part
    CString      m_sContentID;                                //The uniqiue ID for this body part (allows other body parts to refer to us via a CID URL)
    CString      m_sContentLocation;                          //The relative URL for this body part (allows other body parts to refer to us via a relative URL)
    CString      m_sText;                                     //If using strings rather than file, then this is it!
    CBase64Coder m_Coder;                                     //Base64 encoder / decoder instance for this body part
    CArray<CSMTPBodyPart*, CSMTPBodyPart*&> m_ChildBodyParts; //Child body parts for this body part
    CSMTPBodyPart* m_pParentBodyPart;                         //The parent body part for this body part
    CString      m_sBoundary;                                 //String which is used as the body separator for all child mime parts
    BOOL         m_bQuotedPrintable;                          //Should the body text by quoted printable encoded
    BOOL         m_bBase64;                                   //Should the body be base64 encoded. Overrides "m_bQuotedPrintable"
    DWORD        m_dwMaxAttachmentSize;                       //The maximum size this body part can be if it is a file attachment (Defaults to 50 MB)

    // Methods
    void FixSingleDotA(std::string& sBody);
    void FixSingleDotT(CString& sBody);

    friend class CSMTPMessage;
    friend class CPJNSMTPConnection;
};



////////////////// typedefs ////////////////////////////////////////////////////

typedef CArray<CSMTPAddress, CSMTPAddress&> CSMTPAddressArray;



////////////////// Forward declaration /////////////////////////////////////////

class CPJNSMTPConnection;



// Encapsulation of an SMTP message
class CSMTPMessage
{
public:
    // Enums
    enum RECIPIENT_TYPE { TO, CC, BCC };
    enum PRIORITY { NO_PRIORITY, LOW_PRIORITY, NORMAL_PRIORITY, HIGH_PRIORITY };

    // Constructors / Destructors
    CSMTPMessage();
    CSMTPMessage(const CSMTPMessage& message);
    CSMTPMessage& operator=(const CSMTPMessage& message);
    virtual ~CSMTPMessage();

    //Recipient support
    int           GetNumberOfRecipients     (RECIPIENT_TYPE RecipientType = TO) const;
    int           AddRecipient              (CSMTPAddress& recipient, RECIPIENT_TYPE RecipientType = TO);
    void          RemoveRecipient           (int nIndex, RECIPIENT_TYPE RecipientType = TO);
    CSMTPAddress* GetRecipient              (int nIndex, RECIPIENT_TYPE RecipientType = TO);
    BOOL          AddMultipleRecipients     (const CString& sRecipients, RECIPIENT_TYPE RecipientType);
    static int    ParseMultipleRecipients   (const CString& sRecipients, CSMTPAddressArray& recipients);

    // Body Part support
    int            GetNumberOfBodyParts     () const;
    int            AddBodyPart              (CSMTPBodyPart& bodyPart);
    void           RemoveBodyPart           (int nIndex);
    CSMTPBodyPart* GetBodyPart              (int nIndex);
    int            AddMultipleAttachments   (const CString& sAttachments);

    // Misc methods
    virtual std::string  getHeader                  ();
    void                 AddTextBody                (const CString& sBody);
    CString              GetTextBody                ();
    void                 AddHTMLBody                (const CString& sBody, const CString& sContentBase);
    CString              GetHTMLBody                ();
    void                 AddCustomHeader            (const CString& sHeader);
    CString              GetCustomHeader            (int nIndex);
    int                  GetNumberOfCustomHeaders   () const;
    void                 RemoveCustomHeader         (int nIndex);
    void                 SetCharset                 (const CString& sCharset);
    CString              GetCharset                 () const;
    void                 SetMime                    (BOOL bMime);
    BOOL                 GetMime                    () const { return m_bMime; };
    BOOL                 SaveToDisk                 (const CString& sFilename);
                                
    // Data Members
    CSMTPAddress  m_From;
    CString       m_sSubject;
    CString       m_sXMailer;
    CSMTPAddress  m_ReplyTo;
    CSMTPBodyPart m_RootPart;
    PRIORITY      m_Priority;

protected:
    // Methods
    BOOL        WriteToDisk             (CFile& file, CSMTPBodyPart* pBodyPart, BOOL bRoot);
    CString     ConvertHTMLToPlainText  (const CString& sHtml);

    //Member variables
    CArray<CSMTPAddress*, CSMTPAddress*&> m_ToRecipients;
    CArray<CSMTPAddress*, CSMTPAddress*&> m_CCRecipients;
    CArray<CSMTPAddress*, CSMTPAddress*&> m_BCCRecipients;
    CStringArray                          m_CustomHeaders;
    BOOL                                  m_bMime;

    friend class CPJNSMTPConnection;
};



//The main class which encapsulates the SMTP connection
class CPJNSMTPConnection
{
public:

    // typedefs
    enum LoginMethod
    {
        NoLoginMethod=0,
        CramMD5Method=1,
        AuthLoginMethod=2,
        LoginPlainMethod=3
    };

    enum ConnectToInternetResult
    {
        CTIR_Failure=0,
        CTIR_ExistingConnection=1,
        CTIR_NewConnection=2,
    };

    // Constructors / Destructors
    CPJNSMTPConnection();
    virtual ~CPJNSMTPConnection();

    // Methods
    BOOL    Connect                     (LPCTSTR pszHostName, LoginMethod lm=NoLoginMethod, LPCTSTR pszUsername=NULL, LPCTSTR pszPassword=NULL, int nPort=25, LPCTSTR pszLocalBoundAddress=NULL);
    BOOL    Disconnect                  (BOOL bGracefully = TRUE);
    CString GetLastCommandResponse      () const { return m_sLastCommandResponse; };
    int     GetLastCommandResponseCode  () const { return m_nLastCommandResponseCode; };
    DWORD   GetTimeout                  () const { return m_dwTimeout; };
    void    SetTimeout                  (DWORD dwTimeout) { m_dwTimeout = dwTimeout; };
    BOOL    SendMessage                 (CSMTPMessage& Message);
    BOOL    SendMessage                 (const CString& sMessageOnFile, CSMTPAddressArray& Recipients, const CSMTPAddress& From, DWORD dwSendBufferSize = 4096);
    BOOL    SendMessage                 (BYTE* pMessage, DWORD dwMessageSize, CSMTPAddressArray& Recipients, const CSMTPAddress& From, DWORD dwSendBufferSize = 4096);
    void    SetHeloHostname             (const CString& sHostname) { m_sHeloHostname = sHostname; };
    CString GetHeloHostName             () const { return m_sHeloHostname; };

//Static methods
  static ConnectToInternetResult ConnectToInternet();
  static BOOL CloseInternetConnection();

//Virtual Methods
#if (_MFC_VER >= 0x700)
  virtual BOOL OnSendProgress(DWORD dwCurrentBytes, ULONGLONG dwTotalBytes);
#else
  virtual BOOL OnSendProgress(DWORD dwCurrentBytes, DWORD dwTotalBytes);
#endif  

protected:
    // methods
#ifndef CSMTP_NORSA
    void    MD5Digest                   (unsigned char*text, int text_len, unsigned char*key, int key_len, unsigned char* digest);
#endif
    BOOL    ConnectESMTP                (LPCTSTR pszLocalName, LPCTSTR pszUsername, LPCTSTR pszPassword, LoginMethod lm);
    BOOL    ConnectSMTP                 (LPCTSTR pszLocalName);
#ifndef CSMTP_NORSA
    BOOL    CramLogin                   (LPCTSTR pszUsername, LPCTSTR pszPassword);
#endif
    BOOL    AuthLogin                   (LPCTSTR pszUsername, LPCTSTR pszPassword);
    BOOL    AuthLoginPlain              (LPCTSTR pszUsername, LPCTSTR pszPassword);
    BOOL    SendRCPTForRecipient        (CSMTPAddress& recipient);
     BOOL   SendBodyPart                (CSMTPBodyPart* pBodyPart, BOOL bRoot);
    virtual BOOL ReadCommandResponse    (int nExpectedCode, BOOL bEHLO = FALSE);
    virtual BOOL ReadResponse           (LPSTR pszBuffer, int nInitialBufSize, LPSTR pszTerminator, 
                                        int nExpectedCode, LPSTR* ppszOverFlowBuffer, int nGrowBy=4096, BOOL bEHLO = FALSE);
    void    SafeCloseFile               (CFile& File, const CString& sError);
    virtual void OnError                (const CString& sError);

    // Member variables
    CSMTPSocket m_SMTP;
    BOOL        m_bConnected;
    CString     m_sLastCommandResponse;
    CString     m_sHeloHostname;
    DWORD       m_dwTimeout;
    int         m_nLastCommandResponseCode;
};

//Provide for backward compatability be defining CSMTPConnection as a preprocessor define
//for CPJNSMTPConnection if we are not using the ATL Server's "CSMTPConnection" class.
#ifndef __ATLSMTPCONNECTION_H__
#define CSMTPConnection CPJNSMTPConnection
#endif

#endif //__SMTP_H__
