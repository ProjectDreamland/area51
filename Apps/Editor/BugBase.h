#if !defined(AFX_BugBase_H__0D68EF86_0368_417A_8D3B_D3150917E55C__INCLUDED_)
#define AFX_BugBase_H__0D68EF86_0368_417A_8D3B_D3150917E55C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// View1.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBugBase html view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CBugBase : public CXTHtmlView
{
protected:
	CBugBase();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CBugBase)

    CXTToolBar          m_wndToolBar;
    BOOL                m_bFirstTimeVisible;
    BOOL                m_bInitialize;
    CAnimateCtrl	    m_wndAnimateBar;

    virtual void OnNavigateComplete2(LPCTSTR strURL);
    virtual void OnBeforeNavigate2  (LPCTSTR lpszURL, DWORD nFlags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel);

// html Data
public:
	//{{AFX_DATA(CBugBase)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBugBase)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CBugBase();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CBugBase)
	afx_msg void OnGoBack();
	afx_msg void OnGoFoward();
	afx_msg void OnStop();
    afx_msg void OnRefresh();
    afx_msg void OnGoHome();
	afx_msg void OnEnable(CCmdUI* pCmdUI);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BugBase_H__0D68EF86_0368_417A_8D3B_D3150917E55C__INCLUDED_)
