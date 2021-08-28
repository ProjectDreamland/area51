// TabView.cpp : implementation of the CTabView class
//

#include "BaseStdAfx.h"
#include "Editor.h"
#include "TabDoc.h"
#include "TabView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabView

IMPLEMENT_DYNCREATE(CTabView, CXTTreeView)

BEGIN_MESSAGE_MAP(CTabView, CXTTreeView)
	//{{AFX_MSG_MAP(CTabView)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_NCHITTEST()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabView construction/destruction

CTabView::CTabView()
{
	// TODO: add construction code here

}

CTabView::~CTabView()
{
	m_imageList.DeleteImageList();
}

BOOL CTabView::PreCreateWindow(CREATESTRUCT& cs)
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
// CTabView diagnostics

#ifdef _DEBUG
void CTabView::AssertValid() const
{
	CXTTreeView::AssertValid();
}

void CTabView::Dump(CDumpContext& dc) const
{
	CXTTreeView::Dump(dc);
}

CTabDoc* CTabView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTabDoc)));
	return (CTabDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTabView message handlers

void CTabView::OnPaint() 
{
	// Helps to reduce screen flicker.
	CPaintDC dc(this);
	CRect rectClient;
	GetClientRect(&rectClient);
	CXTMemDC memDC(&dc, rectClient, xtAfxData.clrWindow);
	CWnd::DefWindowProc( WM_PAINT, (WPARAM)memDC.m_hDC, 0 );
}

BOOL CTabView::OnEraseBkgnd(CDC* pDC) 
{
	// Helps to reduce screen flicker.
	UNUSED_ALWAYS(pDC);
	return TRUE;
}

int CTabView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CXTTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create the image list used by the tree control.
	if (!m_imageList.Create (IDB_IMGLIST_VIEW, 16, 1, RGB(0,255,0)))
		return -1;
	
	// Set the image list for the tree control.
	GetTreeCtrl().SetImageList(&m_imageList, TVSIL_NORMAL);

	// Add the parent item
	HTREEITEM htItem = GetTreeCtrl().InsertItem(_T("Tree item 1"));
	GetTreeCtrl().SetItemState(htItem, TVIS_BOLD, TVIS_BOLD);
	
	// Add children
	for( int i = 1; i < 8; i++ ) {
		GetTreeCtrl().InsertItem(_T("Tree item 2"), 1, 1, htItem);
	}
	
	// Add children
	GetTreeCtrl().InsertItem (_T("Tree item 3"), 2, 3, htItem);
	GetTreeCtrl().Expand(htItem, TVE_EXPAND);

	// TODO: Add your specialized creation code here

	return 0;
}

UINT CTabView::OnNcHitTest(CPoint point) 
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
