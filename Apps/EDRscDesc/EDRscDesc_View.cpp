// EDRscDesc_View.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "EDRscDesc_View.h"
#include "FlatRscList_View.h"
#include "TreeTypeRscList.h"

#include "TreeRsclist_View.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// EDRscDesc_View

IMPLEMENT_DYNCREATE(EDRscDesc_View, CXTTabView)

EDRscDesc_View::EDRscDesc_View()
{
}

EDRscDesc_View::~EDRscDesc_View()
{
}


BEGIN_MESSAGE_MAP(EDRscDesc_View, CXTTabView)
	//{{AFX_MSG_MAP(EDRscDesc_View)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// EDRscDesc_View drawing

void EDRscDesc_View::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// EDRscDesc_View diagnostics

#ifdef _DEBUG
void EDRscDesc_View::AssertValid() const
{
	CXTTabView::AssertValid();
}

void EDRscDesc_View::Dump(CDumpContext& dc) const
{
	CXTTabView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// EDRscDesc_View message handlers

int EDRscDesc_View::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CXTTabView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	//AddView(_T("Tree View"), RUNTIME_CLASS(TreeTypeRscList), GetDocument() );
    AddView(_T("Path View"), RUNTIME_CLASS(CTreeRsclist_View), GetDocument() );    
	AddView(_T("Flat View"), RUNTIME_CLASS(CFlatRscList_View), GetDocument() );

	return 0;
}

BOOL EDRscDesc_View::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	return TRUE;
//	return CXTTabView::OnEraseBkgnd(pDC);
}
