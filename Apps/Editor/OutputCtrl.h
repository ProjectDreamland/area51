#if !defined(AFX_OUTPUTCTRL_H__03397353_7CB3_461B_A775_0BCF4AB52523__INCLUDED_)
#define AFX_OUTPUTCTRL_H__03397353_7CB3_461B_A775_0BCF4AB52523__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OutputCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COutputCtrl window

class COutputCtrl : public CRichEditCtrl
{
// Construction
public:
	COutputCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COutputCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~COutputCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(COutputCtrl)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnCopy();
	afx_msg void OnUpdateCopy(CCmdUI* pCmdUI);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OUTPUTCTRL_H__03397353_7CB3_461B_A775_0BCF4AB52523__INCLUDED_)
