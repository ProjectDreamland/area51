#if !defined(AFX_VIEWLOG_H__E48E5ADC_8C3E_4921_9404_2433AC9E2DBF__INCLUDED_)
#define AFX_VIEWLOG_H__E48E5ADC_8C3E_4921_9404_2433AC9E2DBF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewLog.h : header file
//

#include "ViewBase.h"
#include "ListLog.h"

/////////////////////////////////////////////////////////////////////////////
// CViewLog view

class CViewLog : public CViewBase
{
protected:
	CViewLog();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CViewLog)

// Attributes
public:
    CListLog        m_wndList;
    CFont           m_DefaultFont;
    CFont           m_Font;
    UINT            m_UpdateTimerID;
    BOOL            m_NewDataFlag;

// Operations
public:
    CListLog*       GetList     ( void );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewLog)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CViewLog();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CViewLog)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
    afx_msg void OnListContextMenu ( NMHDR* pHeader, LRESULT* pResult );

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWLOG_H__E48E5ADC_8C3E_4921_9404_2433AC9E2DBF__INCLUDED_)
