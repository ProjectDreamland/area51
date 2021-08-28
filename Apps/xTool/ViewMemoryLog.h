#if !defined(AFX_VIEWMEMORYLOG_H__FDBC6B7F_082D_458F_94F7_5FF03E0F18E7__INCLUDED_)
#define AFX_VIEWMEMORYLOG_H__FDBC6B7F_082D_458F_94F7_5FF03E0F18E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewMemoryLog.h : header file
//

#include "ViewBase.h"
#include "ListMemoryLog.h"

/////////////////////////////////////////////////////////////////////////////

class log_memory;

/////////////////////////////////////////////////////////////////////////////
// CViewMemoryLog view

class CViewMemoryLog : public CViewBase
{
protected:
	CViewMemoryLog();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CViewMemoryLog)

// Attributes
public:
    CListMemoryLog  m_wndList;
    UINT            m_UpdateTimerID;
    BOOL            m_NewDataFlag;

// Operations
public:
    void        SelectMemBlock  ( log_memory* pEntry );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewMemoryLog)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CViewMemoryLog();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CViewMemoryLog)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWMEMORYLOG_H__FDBC6B7F_082D_458F_94F7_5FF03E0F18E7__INCLUDED_)
