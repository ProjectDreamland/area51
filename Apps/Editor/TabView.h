// TabView.h : interface of the CTabView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TABVIEW_H__A6FF7A53_CE95_4659_9C2F_F4DBCA673A9A__INCLUDED_)
#define AFX_TABVIEW_H__A6FF7A53_CE95_4659_9C2F_F4DBCA673A9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTabView : public CXTTreeView
{
protected: // create from serialization only
	CTabView();
	DECLARE_DYNCREATE(CTabView)

// Attributes
public:
	CTabDoc* GetDocument();
	
// Operations
protected:

	CImageList	m_imageList;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTabView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTabView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg UINT OnNcHitTest(CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in TabView.cpp
inline CTabDoc* CTabView::GetDocument()
   { return (CTabDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABVIEW_H__A6FF7A53_CE95_4659_9C2F_F4DBCA673A9A__INCLUDED_)

