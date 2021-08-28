#if !defined(AFX_VIEWGL_H__E48E5ADC_8C3E_4921_9404_2433AC9E2DBF__INCLUDED_)
#define AFX_VIEWGL_H__E48E5ADC_8C3E_4921_9404_2433AC9E2DBF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewGL.h : header file
//

#include "ViewBase.h"
#include "ListLog.h"

/////////////////////////////////////////////////////////////////////////////
// CViewGL view

class CViewGL : public CViewBase
{
protected:
	CViewGL();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CViewGL)

// Attributes
public:
    BOOL            m_MouseCaptured;
    BOOL            m_TrackingLeave;
    UINT            m_UpdateTimerID;
    BOOL            m_NewDataFlag;
    BOOL            m_IsDoubleBuffered;
    HDC             m_hDC;
    HGLRC           m_hRC;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewGL)
	protected:
    virtual BOOL PreCreateWindow( CREATESTRUCT& cs );
	virtual void OnDraw(CDC* pDC);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CViewGL();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CViewGL)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT nIDEvent);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
//    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
//    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
//    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
//    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
    afx_msg void OnListContextMenu ( NMHDR* pHeader, LRESULT* pResult );

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWGL_H__E48E5ADC_8C3E_4921_9404_2433AC9E2DBF__INCLUDED_)
