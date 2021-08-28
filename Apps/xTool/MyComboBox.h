#if !defined(AFX_MYCOMBOBOX_H__AC7AA8BE_15C3_4FE9_AF1C_E62087C83F0A__INCLUDED_)
#define AFX_MYCOMBOBOX_H__AC7AA8BE_15C3_4FE9_AF1C_E62087C83F0A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MyComboBox.h : header file
//

#define CBN_ENTER   100

/////////////////////////////////////////////////////////////////////////////
// CMyComboBox window

class CMyComboBox : public CComboBox
{
// Construction
public:
	CMyComboBox();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyComboBox)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMyComboBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMyComboBox)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYCOMBOBOX_H__AC7AA8BE_15C3_4FE9_AF1C_E62087C83F0A__INCLUDED_)
