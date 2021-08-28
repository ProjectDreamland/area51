// EventEditorView.cpp : implementation file
//

#include "stdafx.h"
#include "editor.h"
#include "EventEditorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEventEditorView
#define IDM_POPUP_MENU_TEST1        101
#define IDM_POPUP_MENU_TEST2        102

IMPLEMENT_DYNCREATE(CEventEditorView, CView)

CEventEditorView::CEventEditorView()
{
}

CEventEditorView::~CEventEditorView()
{
}


BEGIN_MESSAGE_MAP(CEventEditorView, CView)
	//{{AFX_MSG_MAP(CEventEditorView)
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_COMMAND( IDM_POPUP_MENU_TEST1, OnMenuItem )
	ON_COMMAND( IDM_POPUP_MENU_TEST2, OnMenuItem )
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEventEditorView drawing

void CEventEditorView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CEventEditorView diagnostics

#ifdef _DEBUG
void CEventEditorView::AssertValid() const
{
	CView::AssertValid();
}

void CEventEditorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CEventEditorView message handlers

void CEventEditorView::OnRButtonUp(UINT nFlags, CPoint point) 
{
    CXTMenu menu;
    menu.CreatePopupMenu();
    menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_TEST1, "Test1" );
    menu.AppendMenu( MF_STRING|MF_ENABLED, IDM_POPUP_MENU_TEST2, "Test2" );
    ClientToScreen(&point);
    menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,point.x,point.y,this);

	CView::OnRButtonUp(nFlags, point);
}

void CEventEditorView::OnMenuItem()
{
    TRACE("MENU ITEM SELECTED!!!\n");
}

int CEventEditorView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_Resource.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP |
        LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS
        , CRect(0,0,0,0),this,101);	

	LPTSTR lpszCols[] = {_T("Sound"),_T("Particle"),0};
	LV_COLUMN   lvColumn;
	//initialize the columns
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 100;
	for(int x = 0; lpszCols[x]!=NULL; x++)
    {
		lvColumn.pszText = lpszCols[x];
		m_Resource.InsertColumn(x,&lvColumn);
    }
	
	return 0;
}

void CEventEditorView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	m_Resource.MoveWindow( 0, 0, cx, cy );
	
}
