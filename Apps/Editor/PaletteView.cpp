// PaletteView.cpp : implementation file
//

#include "BaseStdAfx.h"
#include "PaletteView.h"
/*
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
*/
//=========================================================================
// CPaletteView
//=========================================================================

IMPLEMENT_DYNCREATE(CPaletteView, CView)

//=========================================================================

CPaletteView::CPaletteView() :
m_ToolbarResourceId(0),
m_bActive(FALSE)
{
}

//=========================================================================

CPaletteView::~CPaletteView()
{
}

//=========================================================================

BEGIN_MESSAGE_MAP(CPaletteView, CView)
	//{{AFX_MSG_MAP(CPaletteView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=========================================================================
// CPaletteView drawing
//=========================================================================

void CPaletteView::OnDraw(CDC* pDC)
{
    //do nothing
}

//=========================================================================
// CPaletteView diagnostics
//=========================================================================

#ifdef _DEBUG
void CPaletteView::AssertValid() const
{
	CView::AssertValid();
}

void CPaletteView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

//=========================================================================
// CPaletteView message handlers
//=========================================================================

int CPaletteView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

    if (m_ToolbarResourceId != 0)
    {
        // Create the ToolBar
	    if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		    | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_FIXED | TBBS_WRAPPED))
	    {
		    TRACE0("Failed to create toolbar\n");
		    return -1;      // fail to create
	    }
        m_wndToolBar.LoadToolBar(m_ToolbarResourceId);
    }

	return 0;
}

//=========================================================================

CSize CPaletteView::SizeToolBar(int cx, int cy) 
{
	CSize size = m_wndToolBar.CalcLayout(LM_HORZ| LM_COMMIT,cx);
	m_wndToolBar.MoveWindow(0,0,cx,size.cy);
    return size;
}

//=========================================================================

void CPaletteView::OnTabActivate(BOOL bActivate) 
{
    m_bActive = bActivate;
}

//=========================================================================
