// RealTimeMessage.cpp : implementation file
//

#include "BaseStdAfx.h"
#include "editor.h"
#include "RealTimeMessage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRealTimeMessage

IMPLEMENT_DYNCREATE(CRealTimeMessage, CWinThread)

CRealTimeMessage::CRealTimeMessage( HWND hWnd, s32 SleepMiliseconds, u32 Message )
{
    m_hWnd   = hWnd;
    m_WakeUp = SleepMiliseconds;
    m_Message = Message;
}

CRealTimeMessage::CRealTimeMessage()
{
    ASSERT( 0 );
}

CRealTimeMessage::~CRealTimeMessage()
{
}

BOOL CRealTimeMessage::InitInstance()
{
	// TODO:  perform and per-thread initialization here
	return TRUE;
}

int CRealTimeMessage::ExitInstance()
{
	// TODO:  perform any per-thread cleanup here
	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CRealTimeMessage, CWinThread)
	//{{AFX_MSG_MAP(CRealTimeMessage)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRealTimeMessage message handlers

int CRealTimeMessage::Run() 
{
	// TODO: Add your specialized code here and/or call the base class
    while( GetWindowLong(m_hWnd, GWL_STYLE) )
    {
        // 
        Sleep( m_WakeUp );
        SendMessage( m_hWnd, m_Message, 0, 0 );
    }
	
	return ExitInstance();
}
