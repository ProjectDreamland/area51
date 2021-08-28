// TreeTypeRscList.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "TreeTypeRscList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TreeTypeRscList

IMPLEMENT_DYNCREATE(TreeTypeRscList, CTreeView)

TreeTypeRscList::TreeTypeRscList()
{
    m_pTreeCtrl = NULL;
}

TreeTypeRscList::~TreeTypeRscList()
{
    m_ImageList.DeleteImageList();
}


BEGIN_MESSAGE_MAP(TreeTypeRscList, CTreeView)
	//{{AFX_MSG_MAP(TreeTypeRscList)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_NCHITTEST()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TreeTypeRscList drawing

void TreeTypeRscList::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// TreeTypeRscList diagnostics

#ifdef _DEBUG
void TreeTypeRscList::AssertValid() const
{
	CTreeView::AssertValid();
}

void TreeTypeRscList::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// TreeTypeRscList message handlers

BOOL TreeTypeRscList::PreCreateWindow(CREATESTRUCT& cs) 
{
	if( !CTreeView::PreCreateWindow( cs ))
		return FALSE;

	// Set the style for the tree control.
    cs.style |= TVS_HASBUTTONS|TVS_HASLINES|TVS_LINESATROOT|TVS_EDITLABELS;

	return TRUE;
}

int TreeTypeRscList::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Create the image list used by the tree control.
	if (!m_ImageList.Create(IDB_RSCDESC_TREEBITMAP, 16, 1, RGB(0,255,0)))
		return -1;
	
	// Get a pointer to the tree control, and set its imagelist.
	m_pTreeCtrl = &GetTreeCtrl();
	m_pTreeCtrl->SetImageList(&m_ImageList, TVSIL_NORMAL);

	// Initialize the view.
	UpdateView();

	// TODO: Add your specialized creation code here
	
	return 0;
}

void TreeTypeRscList::OnPaint() 
{
    // Use a "Offscreen" DC to fill rect and send to DefWindowProc...  
	// Background is already filled in gray
	CPaintDC dc(this);

	// Get the client rect.
	CRect rectClient;
	GetClientRect(&rectClient);

	// Paint to a memory device context to help
	// eliminate screen flicker.
	CXTMemDC memDC(&dc, rectClient);

	// Let the window do its default painting.
	CWnd::DefWindowProc( WM_PAINT, (WPARAM)memDC.m_hDC, 0 );
}

UINT TreeTypeRscList::OnNcHitTest(CPoint point) 
{
	UINT uFlag=0;

	// Get the cursor location in client coordinates.
	CPoint pt = point;	
	ScreenToClient(&pt);

	// Get a pointer to the tooltip control.
	CToolTipCtrl* pCtrl = (CToolTipCtrl*)CWnd::FromHandle(
		(HWND)::SendMessage(m_hWnd, TVM_GETTOOLTIPS, 0, 0L));

	// If we have a valid tooltip pointer and the cursor
	// is over a tree item, the bring the tooltip control
	// to the top of the Z-order.
	if (pCtrl && m_pTreeCtrl->HitTest(pt, &uFlag)){
		pCtrl->SetWindowPos(&wndTop,0, 0, 0, 0,
			SWP_NOACTIVATE | SWP_NOSIZE |SWP_NOMOVE);
	}
	
	return CTreeView::OnNcHitTest(point);
}

BOOL TreeTypeRscList::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	UNUSED_ALWAYS(pDC);
	return TRUE;
}

void TreeTypeRscList::UpdateView( void )
{
	if( m_pTreeCtrl && m_pTreeCtrl->GetSafeHwnd( ))
	{
		m_pTreeCtrl->LockWindowUpdate();
		
		CString strTreeItems[] = { _T("Tab classes"), _T("CClassName"), _T("Globals") };
		
		// Add the parent item
		HTREEITEM htItem = m_pTreeCtrl->InsertItem(strTreeItems[0]);
		m_pTreeCtrl->SetItemState( htItem, TVIS_BOLD, TVIS_BOLD );
		
		// Add children
		for( int i = 1; i < 8; i++ ) {
			m_pTreeCtrl->InsertItem (strTreeItems[1], 1, 1, htItem, TVI_LAST);
		}
		
		// Add children
		m_pTreeCtrl->InsertItem (strTreeItems[2], 2, 3, htItem, TVI_LAST);
		m_pTreeCtrl->Expand(htItem, TVE_EXPAND);
		
		m_pTreeCtrl->UnlockWindowUpdate();
	}
}
