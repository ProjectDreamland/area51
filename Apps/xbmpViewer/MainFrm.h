// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__44438927_DCDC_4A58_89AB_78AF16FD525F__INCLUDED_)
#define AFX_MAINFRM_H__44438927_DCDC_4A58_89AB_78AF16FD525F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FileList.h"
#include "ExplorerBar.h"
#include "PreviewBar.h"

enum indicators
{
    INDICATOR_NULL = 0,
	INDICATOR_COLOR,
    INDICATOR_TOTAL,
    INDICATOR_SELECTED,
    INDICATOR_FOCUS,
};

class CMainFrame : public CXTFrameWnd
{
	
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:

// Operations
public:
    void    SetStatusPane   ( int Index, const CString& String );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

    afx_msg LRESULT OnDirChanged(WPARAM, LPARAM);
    afx_msg LRESULT OnNewBitmap(WPARAM, LPARAM);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
    CExplorerBar    m_wndExplorerBar;
    CPreviewBar     m_wndPreviewBar;
	CXTStatusBar    m_wndStatusBar;
	CXTToolBar      m_wndToolBar;
	CFileList       m_wndFileList;

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
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnConvertTga();
	afx_msg void OnUpdateConvertTga(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

extern CMainFrame* g_pMainFrame;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__44438927_DCDC_4A58_89AB_78AF16FD525F__INCLUDED_)
