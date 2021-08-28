// TreeRsclist_View.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "TreeRsclist_View.h"
#include "..\Editor\Project.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTreeRsclist_View

IMPLEMENT_DYNCREATE(CTreeRsclist_View, CView)


BEGIN_MESSAGE_MAP(CTreeRsclist_View, CView)
	//{{AFX_MSG_MAP(CTreeRsclist_View)
	ON_WM_CREATE()
	ON_WM_SIZE()
    ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnItemChanged)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

//=========================================================================

#ifdef _DEBUG
void CTreeRsclist_View::AssertValid() const
{
	CView::AssertValid();
}

//=========================================================================
void CTreeRsclist_View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

//=========================================================================
CTreeRsclist_View::CTreeRsclist_View()
{
}

//=========================================================================
CTreeRsclist_View::~CTreeRsclist_View()
{
}

//=========================================================================
void CTreeRsclist_View::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}


//=========================================================================
int CTreeRsclist_View::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
    if (!m_PathTreeCtrl.Create(WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | 
                            TVS_SHOWSELALWAYS, CRect(0,0,0,0), this, IDR_TREE_PATH_CTRL))
    {
        return -1;	      
    }

    m_PathTreeCtrl.UsePreviousPathAsDisplay(TRUE);

	return 0;
}

//=========================================================================
void CTreeRsclist_View::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// TODO: Add your specialized code here and/or call the base class
    if( lHint )
    {
        UpdateAll();
    }
}

//=========================================================================
void CTreeRsclist_View::UpdateAll( void )
{
    m_PathTreeCtrl.ClearTree();
    char Path[MAX_PATH];
    for( s32 i = g_Project.GetFirstResourceDir( Path ); i != -1; i = g_Project.GetNextResourceDir( i, Path ) )
    {
        m_PathTreeCtrl.BuildTreeFromPath( Path, "*.*", "Resource" );
    }
}

//=========================================================================
void CTreeRsclist_View::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
    m_PathTreeCtrl.MoveWindow(0,0,cx,cy);
}

//=========================================================================

void CTreeRsclist_View::OnItemChanged( NMHDR* pNMHDR, LRESULT* pResult )
{
	NMLISTVIEW* lphdi = (NMLISTVIEW*)pNMHDR;
/*
    if( lphdi->uNewState == 3 )
    {
        x_try;

        CListCtrl* pList = &GetListCtrl();

        char Name[256];
        pList->GetItemText( lphdi->iItem, 0, Name, 256 );
        
        char Ext[256];
        pList->GetItemText( lphdi->iItem, 1, Ext, 256 );

        GetDocument()->SetActiveDesc( xfs("%s.%s",Name,Ext) );

        x_catch_display;
    }
    */
}

BOOL CTreeRsclist_View::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	
	return TRUE;//CView::OnEraseBkgnd(pDC);
}
