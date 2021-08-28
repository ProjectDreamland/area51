#if !defined(AFX_ERRORDIALOG_H__3DA44FF1_54BD_4F94_8A39_B883025168E5__INCLUDED_)
#define AFX_ERRORDIALOG_H__3DA44FF1_54BD_4F94_8A39_B883025168E5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ErrorDialog.h : header file
//

#include "Auxiliary/fx_core/errorlog.hpp"

/////////////////////////////////////////////////////////////////////////////
// CErrorDialog dialog

class CErrorDialog : public CDialog
{
// Construction
public:
	CErrorDialog(CWnd* pParent = NULL);   // standard constructor

public:
    const fx_core::error_log*    m_pErrorLog;

public:
    void    SetErrorLog ( const fx_core::error_log* pErrorLog ) { m_pErrorLog = pErrorLog; };

// Dialog Data
	//{{AFX_DATA(CErrorDialog)
	enum { IDD = IDD_ERRORDIALOG };
	CEdit	m_CtrlEdit;
	CString	m_ValEdit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CErrorDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CErrorDialog)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ERRORDIALOG_H__3DA44FF1_54BD_4F94_8A39_B883025168E5__INCLUDED_)
