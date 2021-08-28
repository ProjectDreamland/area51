// EventTabView.cpp : implementation of the CEventTabView class
//

#include "StdAfx.h"
#include "EventEditorFrame.h"
//#include "EventTabDoc.h"
#include "EventTabView.h"
#include "resource.h"
#include "EventEditorDoc.h"
#include "..\PropertyEditor\PropertyEditorDoc.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CEventTabView
#define IDM_POPUP_MENU_NEW_DIR          101
#define IDM_POPUP_MENU_NEW_EVENT        102
#define IDM_POPUP_MENU_ADD_EVENT        103
#define IDM_POPUP_MENU_RENAME           104
#define IDM_POPUP_MENU_NEW_ROOT_DIR     105
#define IDM_POPUP_MENU_DELETE           106


s32 g_EventCount = 0;

IMPLEMENT_DYNCREATE(CEventTabView, CXTTreeView)

BEGIN_MESSAGE_MAP(CEventTabView, CXTTreeView)
	//{{AFX_MSG_MAP(CEventTabView)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_NCHITTEST()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND          ( IDM_POPUP_MENU_NEW_DIR,       OnNewDir        )
    ON_COMMAND          ( IDM_POPUP_MENU_NEW_ROOT_DIR,  OnNewRootDir    )
	ON_COMMAND          ( IDM_POPUP_MENU_NEW_EVENT,     OnNewEvent      )
    ON_COMMAND          ( IDM_POPUP_MENU_ADD_EVENT,     OnAddEvent      )
    ON_COMMAND          ( IDM_POPUP_MENU_DELETE,        OnDelEvent      )
    ON_NOTIFY_REFLECT   ( TVN_ENDLABELEDIT,             FinishEditing   )
   
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEventTabView construction/destruction

CEventTabView::CEventTabView()
{
	// TODO: add construction code here

}

CEventTabView::~CEventTabView()
{
	m_imageList.DeleteImageList();
}

BOOL CEventTabView::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CXTTreeView::PreCreateWindow( cs ))
		return FALSE;

	// Set the style for the tree control.
	cs.style |= TVS_HASBUTTONS|TVS_HASLINES|TVS_LINESATROOT|TVS_EDITLABELS;

	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CEventTabView diagnostics

#ifdef _DEBUG
void CEventTabView::AssertValid() const
{
	CXTTreeView::AssertValid();
}

void CEventTabView::Dump(CDumpContext& dc) const
{
	CXTTreeView::Dump(dc);
}

CEventEditorDoc* CEventTabView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CEventEditorDoc)));
	return (CEventEditorDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CEventTabView message handlers

void CEventTabView::OnPaint() 
{
	// Helps to reduce screen flicker.
	CPaintDC dc(this);
	CRect rectClient;
	GetClientRect(&rectClient);
	CXTMemDC memDC(&dc, rectClient, xtAfxData.clrWindow);
	CWnd::DefWindowProc( WM_PAINT, (WPARAM)memDC.m_hDC, 0 );
}

//=========================================================================

BOOL CEventTabView::OnEraseBkgnd(CDC* pDC) 
{
	// Helps to reduce screen flicker.
	UNUSED_ALWAYS(pDC);
	return TRUE;
}

//=========================================================================

int CEventTabView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CXTTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create the image list used by the tree control.
//	if (!m_imageList.Create (IDB_IMGLIST_VIEW, 16, 1, RGB(0,255,0)))
//		return -1;
	
	// Set the image list for the tree control.
//	GetTreeCtrl().SetImageList(&m_imageList, TVSIL_NORMAL);

	// Add the parent item
	HTREEITEM htItem = GetTreeCtrl().InsertItem(_T("Tree item 1"));
	GetTreeCtrl().SetItemState(htItem, TVIS_BOLD, TVIS_BOLD);
	
	// Add children
	for( int i = 1; i < 8; i++ ) {
		GetTreeCtrl().InsertItem(_T(xfs("Tree item %d",i)), 1, 1, htItem);
	}
	
	// Add children
//	GetTreeCtrl().InsertItem (_T("Tree item 3"), 2, 3, htItem);
	GetTreeCtrl().Expand(htItem, TVE_EXPAND);

	// TODO: Add your specialized creation code here

	return 0;
}

//=========================================================================

UINT CEventTabView::OnNcHitTest(CPoint point) 
{
#if _MSC_VER >= 1200 //MFC 6.0
	UINT uFlag=0;

	// Get the cursor location in client coordinates.
	CPoint pt = point;	
	ScreenToClient(&pt);

	// Get a pointer to the tooltip control.
	CToolTipCtrl* pCtrl = GetTreeCtrl().GetToolTips();

	// If we have a valid tooltip pointer and the cursor
	// is over a tree item, the bring the tooltip control
	// to the top of the Z-order.
	if (pCtrl && GetTreeCtrl().HitTest(pt, &uFlag)){
		pCtrl->SetWindowPos(&wndTop,0, 0, 0, 0,
			SWP_NOACTIVATE | SWP_NOSIZE |SWP_NOMOVE);
	}
#endif //MFC 6.0
	
	return CXTTreeView::OnNcHitTest(point);
}

//=========================================================================

void CEventTabView::OnRButtonDown(UINT nFlags, CPoint point) 
{	
    CXTMenu menu;
    menu.CreatePopupMenu();
    
    menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_NEW_ROOT_DIR,     "New Root Directory"    );
    menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_NEW_DIR,          "New Directory"         );    
    menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_NEW_EVENT,        "New Event"             );
    menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_ADD_EVENT,        "Add Event"             );
    menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_RENAME,           "Rename"                );
    menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_DELETE,           "Delete"                );

    ClientToScreen(&point);
    menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,point.x,point.y,this);

    CXTTreeView::OnRButtonDown(nFlags, point);
}

//=========================================================================

void CEventTabView::OnNewDir( void )
{
    HTREEITEM htItem = GetTreeCtrl().GetSelectedItem();
    
    if( !htItem )
        htItem = TVI_ROOT;

    GetTreeCtrl().InsertItem(_T("New Dir"), htItem);
    GetTreeCtrl().Expand(htItem, TVE_EXPAND);
}

//=========================================================================

void CEventTabView::OnNewEvent()
{    
    HTREEITEM hItem = GetTreeCtrl().GetSelectedItem();
    if( !hItem )
        hItem = TVI_ROOT;

    GetTreeCtrl().InsertItem(_T("New Event"), hItem);
    GetTreeCtrl().Expand(hItem, TVE_EXPAND);
//    GetTreeCtrl().SendMessage( TVN_BEGINLABELEDIT );
}

//=========================================================================

void CEventTabView::OnAddEvent()
{
    CEventEditorDoc&  Doc = *GetDocument();

    HTREEITEM hItem = GetTreeCtrl().GetSelectedItem();
    
    x_try;
    
    // Get the string from the item.
    if( Doc.m_EventEditor.m_pDesc )
        Doc.m_EventEditor.AddEvent( (LPCTSTR)GetTreeCtrl().GetItemText( hItem ) );

    x_catch_display;
    
    Doc.m_pPropEditor->Refresh();  // Need to get all the properties again

    g_EventCount++;
}

//=========================================================================

void CEventTabView::OnNewRootDir ( void )
{
    // Create a new root directory.
    GetTreeCtrl().InsertItem(_T("New Dir"), TVI_ROOT);
}

//=========================================================================

void CEventTabView::FinishEditing( NMTVDISPINFO& Info )// TVITEM
{
    // Set the new label text.
    HTREEITEM hItem = GetTreeCtrl().GetSelectedItem();

    GetTreeCtrl().SetItemText(hItem, Info.item.pszText);
}

//=========================================================================

void CEventTabView::OnDelEvent ( void )
{
    HTREEITEM hItem = GetTreeCtrl().GetSelectedItem();
    if( !hItem )
        return;

    GetTreeCtrl().DeleteItem( hItem);
}

//=========================================================================

