#if !defined(AFX_LISTBOXDLG_H__80E14B8B_ACBA_42DC_8282_30F775203C24__INCLUDED_)
#define AFX_LISTBOXDLG_H__80E14B8B_ACBA_42DC_8282_30F775203C24__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ListBoxDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CListBoxDlg dialog
#include "resource.h"

class CListBoxDlg : public CDialog
{
// Construction
public:
	CListBoxDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CListBoxDlg)
	enum { IDD = IDD_LIST_BOX_DLG };
	CStatic	m_stDisplayText;
	CListBox	m_lbStrings;
	//}}AFX_DATA

    void SetDisplayText(CString strText) { m_strDisplayText = strText; }
    void AddString(CString strText) { m_lstStrings.AddTail(strText); }
    CString GetSelection() { return m_strSelectedString; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListBoxDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CListBoxDlg)
	afx_msg void OnDblclkListStrings();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    CString m_strDisplayText;
    CString m_strSelectedString;
    CStringList m_lstStrings;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTBOXDLG_H__80E14B8B_ACBA_42DC_8282_30F775203C24__INCLUDED_)
