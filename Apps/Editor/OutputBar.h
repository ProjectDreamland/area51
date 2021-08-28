// OutputBar.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_OUTPUTBAR_H__71ED6C04_9ADF_4BC9_9224_F227C7EB94E3__INCLUDED_)
#define AFX_OUTPUTBAR_H__71ED6C04_9ADF_4BC9_9224_F227C7EB94E3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "..\EDRscDesc\CompErrorDisplayCtrl.h"
#include "..\Editor\OutputCtrl.h"
#include "LogView.h"

/////////////////////////////////////////////////////////////////////////////
// COutputBar dock window class

class COutputBar : public CXTDockWindow
{
	DECLARE_DYNAMIC(COutputBar)


// Construction / destruction
public:
	COutputBar();
	virtual ~COutputBar();

// Attributes
public:

	CXTFlatTabCtrl	        m_flatTabCtrl;
	CXTListBox		        m_sheet2;
    CFont                   m_Font;
    CompErrorDisplayCtrl    m_CompileOutput;
    COutputCtrl             m_DebugMsgOutput;
    CLogView                m_LogMsgOutput;
    COutputCtrl             m_SelectionsOutput;


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COutputBar)
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(COutputBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OUTPUTBAR_H__71ED6C04_9ADF_4BC9_9224_F227C7EB94E3__INCLUDED_)
