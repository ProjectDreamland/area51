#if !defined(AFX_CBaseFrame_H__03ADB321_6184_4D4D_B738_6B05330453CE__INCLUDED_)
#define AFX_CBaseFrame_H__03ADB321_6184_4D4D_B738_6B05330453CE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CBaseFrame.h : header file
//
class CBaseDocument;

/////////////////////////////////////////////////////////////////////////////
// CBaseFrame frame

class CBaseFrame : public CXTMDIChildWnd
{
	DECLARE_DYNCREATE(CBaseFrame)
public:

    CBaseFrame();           // protected constructor used by dynamic creation
	virtual ~CBaseFrame();
    CBaseDocument*  m_pBaseDoc;

    CBaseDocument* GetBaseDocument() { return m_pBaseDoc; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBaseFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void ActivateFrame(int nCmdShow);
	//}}AFX_VIRTUAL

// Generated message map functions
protected:
	// Generated message map functions
	//{{AFX_MSG(CBaseFrame)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	//}}AFX_MSG


	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CBaseFrame_H__03ADB321_6184_4D4D_B738_6B05330453CE__INCLUDED_)
