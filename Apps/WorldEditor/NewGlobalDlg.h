#if !defined(AFX_NEWGLOBALDLG_H__1AB1E602_AFA6_4EF2_AFE3_9F6D67210D12__INCLUDED_)
#define AFX_NEWGLOBALDLG_H__1AB1E602_AFA6_4EF2_AFE3_9F6D67210D12__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewGlobalDlg.h : header file
//
#include "resource.h"
#include "..\Support\Globals\Global_Variables_Manager.hpp"

/////////////////////////////////////////////////////////////////////////////
// CNewGlobalDlg dialog

class CNewGlobalDlg : public CDialog
{
// Construction
public:
	CNewGlobalDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewGlobalDlg)
	enum { IDD = IDD_NEW_GLOBAL_DLG };
	CEdit	m_EditGlobalName;
	int		m_RadioButtonData;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewGlobalDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:

    CString GetGlobalName() { return m_strName; }
    var_mngr::global_types GetGlobalType();

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewGlobalDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    CString m_strName;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWGLOBALDLG_H__1AB1E602_AFA6_4EF2_AFE3_9F6D67210D12__INCLUDED_)
