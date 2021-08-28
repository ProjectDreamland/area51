// xTool.h : main header file for the XTOOL application
//

#if !defined(AFX_XTOOL_H__371F2E3F_23E0_4C26_AF20_0D9DF0E96C4A__INCLUDED_)
#define AFX_XTOOL_H__371F2E3F_23E0_4C26_AF20_0D9DF0E96C4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#include "implementation/x_tool_private.hpp"

/////////////////////////////////////////////////////////////////////////////
// debug_connection

class debug_connection
{
public:
    debug_connection( void )         { m_hPipe = NULL;  m_pDoc = NULL; m_pPacketData = NULL; m_PacketDataLength = 0; }
    debug_connection( HANDLE hPipe ) { m_hPipe = hPipe; m_pDoc = NULL; m_pPacketData = NULL; m_PacketDataLength = 0; }
    ~debug_connection( void )        { free( m_pPacketData ); }

    OVERLAPPED              m_Overlapped;
    HANDLE                  m_hPipe;
    bool                    m_IsConnected;
    class CxToolDoc*        m_pDoc;
    xtool::packet_header    m_PacketHeader;
    void*                   m_pPacketData;
    int                     m_PacketDataLength;
    HANDLE                  m_hExitEvent;
    HANDLE                  m_hExitedEvent;
    int                     m_BytesLeftToRead;
    char*                   m_pRead;
};

class debug_connection_list
{
protected:
    CCriticalSection                            m_CriticalSection;
    CList<debug_connection*,debug_connection*>  m_List;

public:
    void Append                 ( debug_connection* pConnection ) { m_CriticalSection.Lock() ; m_List.AddTail( pConnection ) ; m_CriticalSection.Unlock(); }
    debug_connection* GetNext   ( void ) { debug_connection* pConnection = NULL; m_CriticalSection.Lock() ; if( m_List.GetCount() > 0 ) pConnection = m_List.RemoveHead() ; m_CriticalSection.Unlock(); return pConnection; }
};

extern debug_connection_list    g_ConnectionList;
extern CPtrArray                g_Documents;

enum MyWindowsMessages
{
    AM_REBUILD_DOC_TITLE    = WM_USER+1,        // Rebuild the document title and post to the window
    AM_LOG_PACKET,                              // New log packet
    AM_UPDATE_DOC_VIEWS,                        // Update document views - Sent from a worker thread
    AM_FIND,                                    // Find next occurence of string - Sent to list usually
    AM_MARK_ALL,                                // Mark all occurences of string - Sent to list usually
};

/////////////////////////////////////////////////////////////////////////////
// CxToolApp:
// See xTool.cpp for the implementation of this class
//

class CxToolApp : public CWinApp
{
protected:
    CWinThread* m_pListenThread;

public:
	CxToolApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CxToolApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CxToolApp)
	afx_msg void OnAppAbout();
	afx_msg void OnUpdateViewTipoftheday(CCmdUI* pCmdUI);
	afx_msg void OnViewTipoftheday();
	//}}AFX_MSG
	afx_msg void OnTipOfTheDay();
	DECLARE_MESSAGE_MAP()

	void ShowTipAtStartup();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XTOOL_H__371F2E3F_23E0_4C26_AF20_0D9DF0E96C4A__INCLUDED_)
