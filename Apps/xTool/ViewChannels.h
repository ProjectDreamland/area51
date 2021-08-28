#if !defined(AFX_VIEWCHANNELS_H__87BA39DA_51D6_450F_90A6_AD79BD3A51E4__INCLUDED_)
#define AFX_VIEWCHANNELS_H__87BA39DA_51D6_450F_90A6_AD79BD3A51E4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewChannels.h : header file
//

#include "ViewBase.h"
#include "ListChannels.h"

/////////////////////////////////////////////////////////////////////////////
// CViewChannels view

class CViewChannels : public CViewBase
{
protected:
	CViewChannels();            // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CViewChannels)

// Attributes
public:
    CListChannels   m_wndList;
    UINT            m_UpdateTimerID;

// Operations
public:
    CListChannels*  GetList     ( void );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewChannels)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL
//    afx_msg void OnClick      ( NMHDR* pHeader, LRESULT* pResult );
    afx_msg void OnCheckChanged ( NMHDR* pHeader, LRESULT* pResult );

// Implementation
protected:
	virtual ~CViewChannels();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CViewChannels)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWCHANNELS_H__87BA39DA_51D6_450F_90A6_AD79BD3A51E4__INCLUDED_)
