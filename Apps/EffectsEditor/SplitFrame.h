#if !defined(AFX_SPLITFRAME_H__1A0666D3_209C_42C9_8014_25663A1F32B0__INCLUDED_)
#define AFX_SPLITFRAME_H__1A0666D3_209C_42C9_8014_25663A1F32B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SplitFrame.h : header file
//

#include "XTToolBar.h"
#include "ViewportToolbar.h"

/////////////////////////////////////////////////////////////////////////////
// CSplitFrame window

class CSplitFrame : public CFrameWnd
{
	DECLARE_DYNCREATE(CSplitFrame)

// Construction
public:
	CSplitFrame();

// Attributes
public:
    bool                m_Focus;

// Operations
public:

	CXTToolBar          m_wndToolBar;
    CViewportToolbar    m_ViewportToolbar;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSplitFrame)
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSplitFrame();

	// Generated message map functions
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(CSplitFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPLITFRAME_H__1A0666D3_209C_42C9_8014_25663A1F32B0__INCLUDED_)
