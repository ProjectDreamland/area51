#if !defined(AFX_EDRSCDESC_VIEW_H__8F0DAF3B_6C59_4846_B34F_B9B6687EEA8C__INCLUDED_)
#define AFX_EDRSCDESC_VIEW_H__8F0DAF3B_6C59_4846_B34F_B9B6687EEA8C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EDRscDesc_View.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// EDRscDesc_View view

class EDRscDesc_View : public CXTTabView
{
protected:
	EDRscDesc_View();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(EDRscDesc_View)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EDRscDesc_View)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~EDRscDesc_View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(EDRscDesc_View)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDRSCDESC_VIEW_H__8F0DAF3B_6C59_4846_B34F_B9B6687EEA8C__INCLUDED_)
