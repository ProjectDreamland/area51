// xbmpViewer.h : main header file for the XBMPVIEWER application
//

#if !defined(AFX_XBMPVIEWER_H__37EF8F49_7BA5_49DB_B5F9_8BC56191DBA9__INCLUDED_)
#define AFX_XBMPVIEWER_H__37EF8F49_7BA5_49DB_B5F9_8BC56191DBA9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

/////////////////////////////////////////////////////////////////////////////
// CXbmpViewerApp:
// See xbmpViewer.cpp for the implementation of this class
//

class CXbmpViewerApp : public CWinApp
{
public:
	CXbmpViewerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXbmpViewerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

public:
	//{{AFX_MSG(CXbmpViewerApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XBMPVIEWER_H__37EF8F49_7BA5_49DB_B5F9_8BC56191DBA9__INCLUDED_)
