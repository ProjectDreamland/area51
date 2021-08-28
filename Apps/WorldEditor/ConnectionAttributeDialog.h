#if !defined(AFX_CONNECTIONATTRIBUTEDIALOG_H__D28AED34_C638_4AED_99CA_52FEE0B10720__INCLUDED_)
#define AFX_CONNECTIONATTRIBUTEDIALOG_H__D28AED34_C638_4AED_99CA_52FEE0B10720__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ConnectionAttributeDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ConnectionAttributeDialog dialog

class ConnectionAttributeDialog : public CDialog
{
// Construction
public:
	ConnectionAttributeDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ConnectionAttributeDialog)
	enum { IDD = IDD_DIALOG_CONNECTION_ATTR };
	CComboBox	m_HintSneaky;
	CComboBox	m_HintPatrolRoute;
	CComboBox	m_HintOneWay;
	CComboBox	m_HintDark;
	CComboBox	m_HintCover;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ConnectionAttributeDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ConnectionAttributeDialog)
	afx_msg void OnClose();
	afx_msg void OnButtonBatchFlag();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CONNECTIONATTRIBUTEDIALOG_H__D28AED34_C638_4AED_99CA_52FEE0B10720__INCLUDED_)
