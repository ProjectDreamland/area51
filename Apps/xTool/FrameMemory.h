#if !defined(AFX_FRAMEMEMORY_H__E99C1A15_7B26_4D8D_B872_3366844B77DC__INCLUDED_)
#define AFX_FRAMEMEMORY_H__E99C1A15_7B26_4D8D_B872_3366844B77DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PaneLog.h : header file
//

#include "FrameBase.h"
#include "RulerBar.h"

/////////////////////////////////////////////////////////////////////////////
// CFrameMemory window

class CFrameMemory : public CFrameBase
{
// Construction
public:
	CFrameMemory();

// Attributes
public:
    CXTToolBar          m_wndToolBar;
    CRulerBar           m_wndRulerBar;
    int                 m_RulerBarID;
    CSplitterWnd        m_Splitter;
    CSplitterWnd        m_Splitter2;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFrameMemory)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFrameMemory();

	// Generated message map functions
protected:
	//{{AFX_MSG(CFrameMemory)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMemViewMalloc();
	afx_msg void OnMemViewFree();
	afx_msg void OnMemViewRealloc();
	afx_msg void OnUpdateMemViewMalloc(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMemViewFree(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMemViewRealloc(CCmdUI* pCmdUI);
	afx_msg void OnEnable(BOOL bEnable);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FRAMEMEMORY_H__E99C1A15_7B26_4D8D_B872_3366844B77DC__INCLUDED_)
