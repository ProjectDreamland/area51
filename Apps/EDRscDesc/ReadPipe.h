#if !defined(AFX_READPIPE_H__1673160D_4EF6_4887_B3A1_FD7ED8717BFE__INCLUDED_)
#define AFX_READPIPE_H__1673160D_4EF6_4887_B3A1_FD7ED8717BFE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ReadPipe.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CReadPipe thread

class CReadPipe : public CWinThread
{
public:
    //CReadPipe( CXTListBox& Output, HANDLE hPipe ) : m_pOutput( &Output ), m_hPipe(hPipe) {};

    CReadPipe( CRichEditCtrl& Output, CProgressCtrl& MainProgress, 
               CProgressCtrl& SubProgress,  xbool bVerbose, const char* pTempDir );


    static void GlobalInit  ( void );
    static s32  GetNextIndex( const char* pTempDir, xstring& CmdLine, xstring& Output );

           int  RunDirectly     ( void );

protected:
    
           void ProcessString   ( const char* pString );
           void OutputString    ( const char* pString );        
           void ParseErrorsToLog( void );



protected:

    CProgressCtrl*              m_pSubProgress;
    CProgressCtrl*              m_pMainProgress;
    CRichEditCtrl*	            m_pOutput;
    xstring                     m_FmtData;
    xbool                       m_bVerbose;
    static s32                  s_nPipes;
    static CRITICAL_SECTION     s_CriticalSec;
    xthread*                    m_pxThread;
    CString                     m_TempDir;

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
public:
	DECLARE_DYNCREATE(CReadPipe)
    CReadPipe();
	virtual ~CReadPipe();

/////////////////////////////////////////////////////////////////////////////
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReadPipe)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual int Run();
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
protected:
	// Generated message map functions
	//{{AFX_MSG(CReadPipe)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_READPIPE_H__1673160D_4EF6_4887_B3A1_FD7ED8717BFE__INCLUDED_)
