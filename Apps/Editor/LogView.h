#if !defined(AFX_LOGVIEW_VIEW_H__A75B1D28_8CB7_49CC_AB15_9712E9ED0D88__INCLUDED_)
#define AFX_LOGVIEW_VIEW_H__A75B1D28_8CB7_49CC_AB15_9712E9ED0D88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FlatRscList_View.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// Column data structure

struct col_data 
{ 
	LPCTSTR name; 
	int width; 
	int fmt;
    XT_DATA_TYPE type;
};


/////////////////////////////////////////////////////////////////////////////
// CLogViewview

class CLogView: public CXTListCtrl
{
protected:
    bool SortList( int nCol, bool bAscending );

public:
        
    void InsertColumns      ( col_data* pColumns, s32 ColCount );

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
public:
	virtual ~CLogView();
	CLogView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CLogView)

/////////////////////////////////////////////////////////////////////////////
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogView)
	public:
	virtual void OnInitialUpdate();
    virtual void OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint );
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

/////////////////////////////////////////////////////////////////////////////
protected:
	//{{AFX_MSG(CLogView)
    afx_msg void OnItemChanged( NMHDR* pNMHDR, LRESULT* pResult );
    afx_msg void OnDoubleClick( NMHDR* pNMHDR, LRESULT* pResult );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGVIEW_VIEW_H__A75B1D28_8CB7_49CC_AB15_9712E9ED0D88__INCLUDED_)
