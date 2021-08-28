// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__978F5AF6_87DE_4C05_A36D_D724AE163173__INCLUDED_)
#define AFX_MAINFRM_H__978F5AF6_87DE_4C05_A36D_D724AE163173__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MyMDIWndTab.h"

class CMainFrame : public CXTMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:

// Operations
public:
    void ReplaceMenu( UINT n_IDResource );
protected:
    int  FindMenuItem(CMenu* Menu, LPCTSTR MenuString);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CXTStatusBar    m_wndStatusBar;
	CXTToolBar      m_wndToolBar;
    CMyMDIWndTab    m_Tabs;

// Generated message map functions
protected:
	CXTWindowPos     m_wndPosition;

public:
	// Overrode CWnd implementation to restore saved window position.
	BOOL ShowWindowEx(int nCmdShow);

protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnWindowCloseAll();
	//}}AFX_MSG

    BOOL            IsValidDocument( CxToolDoc* pDoc );

	afx_msg LRESULT OnRebuildDocTitle(WPARAM, LPARAM);
    afx_msg LRESULT OnLogPacket(WPARAM, LPARAM);
	afx_msg LRESULT OnUpdateDocViews(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__978F5AF6_87DE_4C05_A36D_D724AE163173__INCLUDED_)
