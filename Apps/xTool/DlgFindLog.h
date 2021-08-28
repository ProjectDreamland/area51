#if !defined(AFX_DLGFINDLOG_H__CEA10D17_7E0E_463B_B65C_086DA7D91216__INCLUDED_)
#define AFX_DLGFINDLOG_H__CEA10D17_7E0E_463B_B65C_086DA7D91216__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgFindLog.h : header file
//

#include "MyComboBox.h"
#include "RecentList.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgFindLog dialog

class CDlgFindLog : public CDialog
{
// Construction
public:
	CDlgFindLog(LPCTSTR pTitle, CWnd* pParent = NULL);   // standard constructor

    void    SetRecentList( CRecentList& RecentList );

// Dialog Data
	//{{AFX_DATA(CDlgFindLog)
	enum { IDD = IDD_LOG_FIND };
	CMyComboBox	m_CtrlString;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgFindLog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    CString         m_Title;
    CRecentList*    m_pRecentList;

    void    UpdateMostRecent    ( void );

	// Generated message map functions
	//{{AFX_MSG(CDlgFindLog)
	afx_msg void OnSelchangeString();
	virtual BOOL OnInitDialog();
	afx_msg void OnFindNext();
	afx_msg void OnMarkAll();
	//}}AFX_MSG
    afx_msg void OnEnterString();
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGFINDLOG_H__CEA10D17_7E0E_463B_B65C_086DA7D91216__INCLUDED_)
