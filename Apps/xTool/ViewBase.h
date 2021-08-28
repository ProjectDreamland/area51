#if !defined(AFX_VIEWBASE_H__87BA39DA_51D6_450F_90A6_AD79BD3A51E4__INCLUDED_)
#define AFX_VIEWBASE_H__87BA39DA_51D6_450F_90A6_AD79BD3A51E4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewBase.h : header file
//

class CxToolDoc;

/////////////////////////////////////////////////////////////////////////////
// CViewBase view

class CViewBase : public CView
{
protected:
	CViewBase();            // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CViewBase)

// Attributes
public:
    CxToolDoc*  GetDocument( void );

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewBase)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CViewBase();

	// Generated message map functions
protected:
	//{{AFX_MSG(CViewBase)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in xToolView.cpp
inline CxToolDoc* CViewBase::GetDocument()
   { return (CxToolDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWBASE_H__87BA39DA_51D6_450F_90A6_AD79BD3A51E4__INCLUDED_)
