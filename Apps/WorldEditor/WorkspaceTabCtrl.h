#if !defined(AFX_WORKSPACETABCTRL_H__C470295D_D622_42A3_81C8_30039D622991__INCLUDED_)
#define AFX_WORKSPACETABCTRL_H__C470295D_D622_42A3_81C8_30039D622991__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WorkspaceTabCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceTabCtrl window

class CWorkspaceTabCtrl : public CXTTabCtrlBar
{
// Construction
public:
	CWorkspaceTabCtrl();
	virtual ~CWorkspaceTabCtrl();

    virtual void OnTabSelChange(int nIDCtrl, CXTTabCtrl* pTabCtrl);
    CWnd* GetWorkspaceView( CRuntimeClass *pViewClass );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWorkspaceTabCtrl)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CWorkspaceTabCtrl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

    void UpdateView(int Index, BOOL bActivate);

    int m_nLastActiveView;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WORKSPACETABCTRL_H__C470295D_D622_42A3_81C8_30039D622991__INCLUDED_)
