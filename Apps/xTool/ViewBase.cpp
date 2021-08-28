// ViewBase.cpp : implementation file
//

#include "stdafx.h"
#include "xTool.h"
#include "ViewBase.h"
#include "xToolDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewBase

IMPLEMENT_DYNCREATE(CViewBase, CView)

CViewBase::CViewBase()
{
}

CViewBase::~CViewBase()
{
}


BEGIN_MESSAGE_MAP(CViewBase, CView)
	//{{AFX_MSG_MAP(CViewBase)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewBase drawing

void CViewBase::OnDraw(CDC* pDC)
{
	CxToolDoc* pDoc = GetDocument();
}

/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
CxToolDoc* CViewBase::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CxToolDoc)));
	return (CxToolDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CViewBase message handlers

BOOL CViewBase::PreCreateWindow(CREATESTRUCT& cs)
{
    cs.style |= WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
    cs.style &= ~WS_BORDER;
	
	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
