#if !defined(AFX_CProjectView_H__8F0DAF3B_6C59_4846_B34F_B9B6687EEA8C__INCLUDED_)
#define AFX_CProjectView_H__8F0DAF3B_6C59_4846_B34F_B9B6687EEA8C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CProjectView.h : header file
//

#include "ProjectViewFrame.h"
#include "TextEditor.h"

/////////////////////////////////////////////////////////////////////////////
// CProjectView view

class CProjectView : public CView
{
protected:
	CProjectView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CProjectView)

//    CXTToolBar          m_wndToolBar;
//    CXTTabCtrlBar       m_TabCtrl;
    CProjectViewFrame   m_Frame;

    CTextEditor&        GetTextEditor();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProjectView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CProjectView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CProjectView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDestroy();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CProjectView_H__8F0DAF3B_6C59_4846_B34F_B9B6687EEA8C__INCLUDED_)
