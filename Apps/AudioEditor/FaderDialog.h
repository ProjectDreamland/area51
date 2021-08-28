#if !defined(AFX_FADERDIALOG_H__0371E38C_43DF_4830_A25B_C92EA6D2093D__INCLUDED_)
#define AFX_FADERDIALOG_H__0371E38C_43DF_4830_A25B_C92EA6D2093D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FaderDialog.h : header file
//

//==============================================================================
// EXTERNALS
//==============================================================================

extern CString g_String;

//==============================================================================
// CFADERDIALOG DIALOG
//==============================================================================

class CFaderDialog : public CDialog
{
//==============================================================================
public:
	CFaderDialog(CWnd* pParent = NULL);   // standard constructor

    void OnNewFader ( void );

	enum { IDD = IDD_FADER_DIALOG };
	CEdit	m_FaderName;
	CListBox	m_ListBox;

//==============================================================================
protected:

	// Generated message map functions
	//{{AFX_MSG(CFaderDialog)
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//==============================================================================
// END
//==============================================================================

#endif // !defined(AFX_FADERDIALOG_H__0371E38C_43DF_4830_A25B_C92EA6D2093D__INCLUDED_)
