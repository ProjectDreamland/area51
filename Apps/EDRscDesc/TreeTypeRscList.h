#if !defined(AFX_TREETYPERSCLIST_H__018A9B32_049D_436D_A9D0_4FF99DA0B85A__INCLUDED_)
#define AFX_TREETYPERSCLIST_H__018A9B32_049D_436D_A9D0_4FF99DA0B85A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TreeTypeRscList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// TreeTypeRscList view

class TreeTypeRscList : public CTreeView
{
protected:

    void UpdateView ( void );

protected:

	CTreeCtrl*	m_pTreeCtrl;
	CImageList	m_ImageList;


/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
protected:
	virtual ~TreeTypeRscList();
	TreeTypeRscList();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(TreeTypeRscList)

/////////////////////////////////////////////////////////////////////////////
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TreeTypeRscList)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
protected:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

/////////////////////////////////////////////////////////////////////////////
protected:
	//{{AFX_MSG(TreeTypeRscList)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
    afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TREETYPERSCLIST_H__018A9B32_049D_436D_A9D0_4FF99DA0B85A__INCLUDED_)
