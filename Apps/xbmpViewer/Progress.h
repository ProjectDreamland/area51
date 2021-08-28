#if !defined(AFX_PROGRESS_H__D096DD8C_3449_47A6_9210_6BEDCF11367A__INCLUDED_)
#define AFX_PROGRESS_H__D096DD8C_3449_47A6_9210_6BEDCF11367A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Progress.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProgress dialog

class CProgress : public CDialog
{
// Construction
public:
	CProgress(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProgress)
	enum { IDD = IDD_PROGRESS };
	CStatic	m_CtrlText;
	CProgressCtrl	m_CtrlProgress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgress)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
public:
    void        SetText( const char* pText );
    void        SetProgress( s32 Percentage );

protected:

	// Generated message map functions
	//{{AFX_MSG(CProgress)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESS_H__D096DD8C_3449_47A6_9210_6BEDCF11367A__INCLUDED_)
