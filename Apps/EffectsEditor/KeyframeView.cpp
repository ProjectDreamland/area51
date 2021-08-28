// KeyframeView.cpp : implementation file
//

#include "stdafx.h"
#include "PartEd.h"
#include "KeyframeView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CKeyframeView

CKeyframeView::CKeyframeView()
{
}

CKeyframeView::~CKeyframeView()
{
}

void CKeyframeView::OnUpdateCmdUI( CFrameWnd* pTarget, BOOL bDisableIfNoHndler )
{
}

CSize CKeyframeView::CalcFixedLayout( BOOL bStretch, BOOL bHorz )
{
        
    RECT Rect;
    GetParent()->GetClientRect(&Rect);

    return CSize(Rect.right, 64);
}


BEGIN_MESSAGE_MAP(CKeyframeView, CControlBar)
	//{{AFX_MSG_MAP(CKeyframeView)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CKeyframeView message handlers

void CKeyframeView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
    dc.Rectangle( 10, 10, 50, 30 );
	
	// Do not call CControlBar::OnPaint() for painting messages
}

BOOL CKeyframeView::OnEraseBkgnd(CDC* pDC) 
{
	pDC->SelectStockObject( LTGRAY_BRUSH );
    RECT Rect;
    GetClientRect( &Rect );
    pDC->Rectangle( &Rect );

	return TRUE;//CControlBar::OnEraseBkgnd(pDC);
}
