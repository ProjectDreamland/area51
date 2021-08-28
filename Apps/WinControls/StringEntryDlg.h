#if !defined(AFX_STRINGENTRYDLG_H__EB66F092_A6BB_4DAB_8433_A0842EB330B2__INCLUDED_)
#define AFX_STRINGENTRYDLG_H__EB66F092_A6BB_4DAB_8433_A0842EB330B2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StringEntryDlg.h : header file
//
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CStringEntryDlg dialog

class CStringEntryDlg : public CDialog
{
// Construction
public:
	CStringEntryDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CStringEntryDlg)
	enum { IDD = IDD_GET_STRING_DLG };
	CStatic	m_stDisplay;
	CEdit	m_edEntry;
	//}}AFX_DATA

    void SetDisplayText(CString strText) { m_strDisplayText = strText; }
    void SetEntryText(CString strText) { m_strEntryText = strText; }
    void SetTextLimit(int nLimit);
    CString GetEntryText() { return m_strEntryText; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStringEntryDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStringEntryDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    CString m_strDisplayText;
    CString m_strEntryText;
    int     m_nTextLimit;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STRINGENTRYDLG_H__EB66F092_A6BB_4DAB_8433_A0842EB330B2__INCLUDED_)
