#if !defined(AFX_RSCVIEW_H__2C656D74_13D1_4283_9693_BB881B7B3F1D__INCLUDED_)
#define AFX_RSCVIEW_H__2C656D74_13D1_4283_9693_BB881B7B3F1D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RscView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// RscView view

class RscView : public CXTListView //CListView
{
protected:
	RscView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(RscView)
    bool SortList( int nCol, bool bAscending );

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(RscView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~RscView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(RscView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RSCVIEW_H__2C656D74_13D1_4283_9693_BB881B7B3F1D__INCLUDED_)
