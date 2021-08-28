#if !defined(AFX_EXPLORERBAR_H__80A71474_9533_4F96_A99D_295077064A1C__INCLUDED_)
#define AFX_EXPLORERBAR_H__80A71474_9533_4F96_A99D_295077064A1C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExplorerBar.h : header file
//

#include "XTIncludes.h"

/////////////////////////////////////////////////////////////////////////////
// CExplorerBar window

class CExplorerBar : public CXTDockWindow
{
// Construction
public:
	CExplorerBar();

// Attributes
public:
    CXTShellTreeCtrl    m_Tree;
    CString             m_Path;

// Operations
public:
    void    SetPath     ( CString   Path );
    CString GetPath     ( void );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExplorerBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CExplorerBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CExplorerBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	//}}AFX_MSG
	afx_msg LRESULT OnUpdateShell(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPLORERBAR_H__80A71474_9533_4F96_A99D_295077064A1C__INCLUDED_)
