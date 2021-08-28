#if !defined(AFX_XTSortListCtrl_H__A75B1D28_8CB7_49CC_AB15_9712E9ED0D88__INCLUDED_)
#define AFX_XTSortListCtrl_H__A75B1D28_8CB7_49CC_AB15_9712E9ED0D88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// XTSortListCtrl

class CXTSortListCtrl: public CXTListCtrl
{
protected:
    bool SortList( int nCol, bool bAscending );

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
public:
	virtual ~CXTSortListCtrl();
	CXTSortListCtrl();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CXTSortListCtrl)

/////////////////////////////////////////////////////////////////////////////
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXTSortListCtrl)
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

/////////////////////////////////////////////////////////////////////////////
protected:
	//{{AFX_MSG(CXTSortListCtrl)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CXTSortListCtrl_H__A75B1D28_8CB7_49CC_AB15_9712E9ED0D88__INCLUDED_)
