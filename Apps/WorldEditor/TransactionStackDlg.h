#if !defined(AFX_TRANSACTIONSTACKDLG_H__22FA16D8_64C7_4DB0_8E4B_BA783BC552E1__INCLUDED_)
#define AFX_TRANSACTIONSTACKDLG_H__22FA16D8_64C7_4DB0_8E4B_BA783BC552E1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TransactionStackDlg.h : header file
//
#include "..\WinControls\StackListBox.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CTransactionStackDlg dialog

class CTransactionStackDlg : public CDialog
{
// Construction
public:
	CTransactionStackDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTransactionStackDlg)
	enum { IDD = IDD_TRANSACTION_STACK_DIALOG };
	CStackListBox	m_lbStack;
	CButton	m_btnOk;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTransactionStackDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTransactionStackDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSelchangeListTransactions();
	afx_msg void OnDblclkListTransactions();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    int     m_nCurrentPos;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRANSACTIONSTACKDLG_H__22FA16D8_64C7_4DB0_8E4B_BA783BC552E1__INCLUDED_)
