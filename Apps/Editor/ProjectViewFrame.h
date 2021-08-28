#if !defined(AFX_CProjectViewFrame_H__8F0DAF3B_6C59_4846_B34F_B9B6687EEA8C__INCLUDED_)
#define AFX_CProjectViewFrame_H__8F0DAF3B_6C59_4846_B34F_B9B6687EEA8C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CProjectViewFrame.h : header file
//

#include "TextEditor.h"

/////////////////////////////////////////////////////////////////////////////
// CProjectView view

class CProjectViewFrame : public CFrameWnd
{
public:
	CProjectViewFrame();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CProjectViewFrame)

    CTextEditor&        GetTextEditor();

protected:
    CXTToolBar          m_wndToolBar;
    CXTTabCtrlBar       m_TabCtrl;

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProjectViewFrame)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CProjectViewFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CProjectViewFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDestroy();
protected:
    virtual void PostNcDestroy();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CProjectViewFrame_H__8F0DAF3B_6C59_4846_B34F_B9B6687EEA8C__INCLUDED_)
