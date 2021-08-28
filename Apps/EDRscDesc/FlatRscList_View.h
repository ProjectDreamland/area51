#if !defined(AFX_FLATRSCLIST_VIEW_H__A75B1D28_7CB7_49CC_AB15_9712E9ED0D88__INCLUDED_)
#define AFX_FLATRSCLIST_VIEW_H__A75B1D28_7CB7_49CC_AB15_9712E9ED0D88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FlatRscList_View.h : header file
//

#include "EDRscDesc_Doc.h"

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
// CFlatRscList_View view

class CFlatRscList_View : public CXTListView
{
protected:
    bool SortList( int nCol, bool bAscending );
    EDRscDesc_Doc&      GetDoc      ( void ) { return *((EDRscDesc_Doc*)GetDocument()); }
    rsc_desc_mgr&       GetMgr      ( void ) { return g_RescDescMGR; }
    void                SetDetails  ( s32 i, const char* pPath, const char* pTheme );

public:
    
    EDRscDesc_Doc* GetDocument();
    
    void UpdateAll          ( void );
    void UpdateSelected     ( void );
    void DeleteAllColumn    ( void );
    void DeleteColumn       ( s32 ColNum );
    void InsertColumns      ( col_data* pColumns, s32 ColCount );

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
protected:
	virtual ~CFlatRscList_View();
	CFlatRscList_View();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFlatRscList_View)

/////////////////////////////////////////////////////////////////////////////
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFlatRscList_View)
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
	//{{AFX_MSG(CFlatRscList_View)
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnItemChanged( NMHDR* pNMHDR, LRESULT* pResult );
    afx_msg void OnDoubleClick( NMHDR* pNMHDR, LRESULT* pResult );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

inline EDRscDesc_Doc* CFlatRscList_View::GetDocument()
   { return (EDRscDesc_Doc*)m_pDocument; }

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLATRSCLIST_VIEW_H__A75B1D28_7CB7_49CC_AB15_9712E9ED0D88__INCLUDED_)
