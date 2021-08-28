// EventTabView.h : interface of the CEventTabView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_EventTabView_H__A6FF7A53_CE95_4659_9C2F_F4DBCA673A9A__INCLUDED_)
#define AFX_EventTabView_H__A6FF7A53_CE95_4659_9C2F_F4DBCA673A9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
class CEventEditorDoc;

class CEventTabView : public CXTTreeView
{
/////////////////////////////////////////////////////////////////////////////
public:

    void OnNewDir       ( void );
    void OnNewRootDir   ( void );
    void OnNewEvent     ( void );
    void OnAddEvent     ( void );
    void OnDelEvent     ( void );
    void FinishEditing  ( NMTVDISPINFO& Info     );

/////////////////////////////////////////////////////////////////////////////
public:

	CEventEditorDoc* GetDocument();
	CImageList	     m_imageList;

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////
public:
	virtual ~CEventTabView();
	CEventTabView();
	DECLARE_DYNCREATE(CEventTabView)

/////////////////////////////////////////////////////////////////////////////
protected:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEventTabView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

/////////////////////////////////////////////////////////////////////////////
protected:
	//{{AFX_MSG(CEventTabView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in EventTabView.cpp
inline CEventEditorDoc* CEventTabView::GetDocument()
   { return (CEventEditorDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EventTabView_H__A6FF7A53_CE95_4659_9C2F_F4DBCA673A9A__INCLUDED_)

