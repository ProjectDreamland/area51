#if !defined(AFX_TabCtrlViewFix_H__C470295D_D622_42A3_81C8_30039D622991__INCLUDED_)
#define AFX_TabCtrlViewFix_H__C470295D_D622_42A3_81C8_30039D622991__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TabCtrlViewFix.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTabCtrlViewFix window

class CTabCtrlViewFix : public CXTTabCtrlBar
{
// Construction
public:
	CTabCtrlViewFix();
	virtual ~CTabCtrlViewFix();

    virtual void OnTabSelChange (int nIDCtrl, CXTTabCtrl* pTabCtrl);
    CWnd* GetWorkspaceView      ( CRuntimeClass *pViewClass );
    xbool IsTabControlActive    ( void ){ return m_bActive; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabCtrlViewFix)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CTabCtrlViewFix)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()


    int m_nLastActiveView;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TabCtrlViewFix_H__C470295D_D622_42A3_81C8_30039D622991__INCLUDED_)
