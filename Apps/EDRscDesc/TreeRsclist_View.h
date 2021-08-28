#if !defined(AFX_TREERSCLIST_VIEW_H__6F7D2F3A_8476_4FA1_9F22_F5EE6C708FA5__INCLUDED_)
#define AFX_TREERSCLIST_VIEW_H__6F7D2F3A_8476_4FA1_9F22_F5EE6C708FA5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TreeRsclist_View.h : header file
//
#include "..\WinControls\FileTreeCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CTreeRsclist_View view

class CTreeRsclist_View : public CView
{
protected:
    void UpdateAll( void );

protected:
    CFileTreeCtrl   m_PathTreeCtrl;


/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////
protected:
	CTreeRsclist_View();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CTreeRsclist_View)
	virtual ~CTreeRsclist_View();

/////////////////////////////////////////////////////////////////////////////
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTreeRsclist_View)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
protected:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

/////////////////////////////////////////////////////////////////////////////
protected:
	//{{AFX_MSG(CTreeRsclist_View)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnItemChanged( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TREERSCLIST_VIEW_H__6F7D2F3A_8476_4FA1_9F22_F5EE6C708FA5__INCLUDED_)
