#if !defined(AFX_XTDIALOGTOOLBAR_H__E0A667A5_28B2_4FF2_ADC2_B50CE15B0451__INCLUDED_)
#define AFX_XTDIALOGTOOLBAR_H__E0A667A5_28B2_4FF2_ADC2_B50CE15B0451__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// XTDialogToolBar.h : header file
//

//HACKED Class to get our toolbars to behave the way we want

/////////////////////////////////////////////////////////////////////////////
// CXTDialogToolBar window

class CXTDialogToolBar : public CXTToolBar
{
// Construction
public:
	CXTDialogToolBar();

// Attributes
public:

// Operations
public:
    CSize CalcLayout(
        // 
        DWORD nMode,
        // 
        int nLength = -1);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXTDialogToolBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CXTDialogToolBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CXTDialogToolBar)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XTDIALOGTOOLBAR_H__E0A667A5_28B2_4FF2_ADC2_B50CE15B0451__INCLUDED_)
