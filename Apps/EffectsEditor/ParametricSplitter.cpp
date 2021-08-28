// ParametricSplitter.cpp : implementation file
//

#include "stdafx.h"
#include "parted.h"
#include "ParametricSplitter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CParametricSplitter

//IMPLEMENT_DYNCREATE(CParametricSplitter, CSplitterWnd)

CParametricSplitter::CParametricSplitter()
{
    m_IsMaximized           = FALSE;
    m_MaximizedViewportID   = 0;
    m_ActiveViewportID      = 0;

    m_SplitX = 0.5f;
    m_SplitY = 0.5f;
}

CParametricSplitter::~CParametricSplitter()
{
}

void CParametricSplitter::SetActiveViewport( s32 ID )
{
    m_ActiveViewportID = ID;
}

xbool CParametricSplitter::IsViewportMaximized( void )
{
    return m_IsMaximized;
}

void CParametricSplitter::MaximizeViewport( void )
{
    CWnd* pView;

    CWnd* pWnd = GetActiveWindow();
    if( IsChild( pWnd ) )
    {
        m_ActiveViewportID = pWnd->GetDlgCtrlID();
    }
    else
    {
        pWnd = pWnd->GetParent();
        if( IsChild( pWnd ) )
        {
            m_ActiveViewportID = pWnd->GetDlgCtrlID();
        }
    }

    if( m_ActiveViewportID == 0 )
        m_ActiveViewportID = IdFromRowCol(1,1);

    pView = GetDlgItem(IdFromRowCol(0,0));
    if( pView )
        pView->ShowWindow( (IdFromRowCol(0,0) == m_ActiveViewportID) ? SW_SHOW : SW_HIDE );

    pView = GetDlgItem(IdFromRowCol(0,1));
    if( pView )
        pView->ShowWindow( (IdFromRowCol(0,1) == m_ActiveViewportID) ? SW_SHOW : SW_HIDE );

    pView = GetDlgItem(IdFromRowCol(1,0));
    if( pView )
        pView->ShowWindow( (IdFromRowCol(1,0) == m_ActiveViewportID) ? SW_SHOW : SW_HIDE );

    pView = GetDlgItem(IdFromRowCol(1,1));
    if( pView )
        pView->ShowWindow( (IdFromRowCol(1,1) == m_ActiveViewportID) ? SW_SHOW : SW_HIDE );

    m_IsMaximized           = TRUE;
    m_MaximizedViewportID   = m_ActiveViewportID;

    pView = GetDlgItem( m_MaximizedViewportID );
    if( pView != NULL )
    {
        CRect rectClient;
        GetClientRect( &rectClient );
        pView->MoveWindow( rectClient );
    }
}

void CParametricSplitter::UnmaximizeViewport( void )
{
    m_IsMaximized = FALSE;

    CWnd* pView;

    pView = GetDlgItem(IdFromRowCol(0,0));
    if( pView )
        pView->ShowWindow( SW_SHOW );

    pView = GetDlgItem(IdFromRowCol(0,1));
    if( pView )
        pView->ShowWindow( SW_SHOW );

    pView = GetDlgItem(IdFromRowCol(1,0));
    if( pView )
        pView->ShowWindow( SW_SHOW );

    pView = GetDlgItem(IdFromRowCol(1,1));
    if( pView )
        pView->ShowWindow( SW_SHOW );

    RecalcLayout();
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CParametricSplitter, CSplitterWnd)
	//{{AFX_MSG_MAP(CParametricSplitter)
	ON_WM_SIZE()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CParametricSplitter message handlers

void CParametricSplitter::OnSize(UINT nType, int cx, int cy) 
{
    // Reset split position based on parametrics
    CRect rectInside;
	GetInsideRect(rectInside);
    if( m_pColInfo )
        m_pColInfo[0].nIdealSize = (s32)((rectInside.Width()  - m_cxSplitterGap) * m_SplitX);
    if( m_pRowInfo )
        m_pRowInfo[0].nIdealSize = (s32)((rectInside.Height() - m_cySplitterGap) * m_SplitY);

    if( m_IsMaximized )
    {
        CWnd* pView;
        pView = GetDlgItem( m_MaximizedViewportID );
        if( pView != NULL )
        {
            CRect rectClient;
            GetClientRect( &rectClient );
            pView->MoveWindow( rectClient );
        }
    }
    else
    {
        // Call parent
        CSplitterWnd::OnSize(nType, cx, cy);
    }
}

void CParametricSplitter::OnLButtonUp(UINT nFlags, CPoint point) 
{
    // Call parent
	CSplitterWnd::OnLButtonUp(nFlags, point);

    // Determine parametrics from current split position
    CRect rectInside;
	GetInsideRect(rectInside);
    m_SplitX = (f32)m_pColInfo[0].nCurSize / (f32)rectInside.Width();
    m_SplitY = (f32)m_pRowInfo[0].nCurSize / (f32)rectInside.Height();
}

void CParametricSplitter::OnRButtonDown(UINT nFlags, CPoint point) 
{
    // Capture the mouse
    SetCapture();
}

void CParametricSplitter::OnRButtonUp(UINT nFlags, CPoint point)
{
    // Call parent
	CSplitterWnd::OnRButtonUp(nFlags, point);

    // Re-center the splitters....but only if we originally Rt-clicked on the splitter
    if( GetCapture() == this )
    {
        ::ReleaseCapture();

        // Reset parametric splitters values to be centered
        m_SplitX = 0.5f;
        m_SplitY = 0.5f;

        // Reset split position based on parametrics
        CRect rectInside;
        GetInsideRect(rectInside);

        if( m_pColInfo )
            m_pColInfo[0].nIdealSize = (s32)((rectInside.Width()  - m_cxSplitterGap) * m_SplitX);
        if( m_pRowInfo )
            m_pRowInfo[0].nIdealSize = (s32)((rectInside.Height() - m_cySplitterGap) * m_SplitY);

        RecalcLayout();
    }
}

BOOL CParametricSplitter::PreCreateWindow(CREATESTRUCT& cs) 
{
//	cs.style |= WS_CLIPCHILDREN;

	return CSplitterWnd::PreCreateWindow(cs);
}

void CParametricSplitter::OnPaint() 
{
    if( m_IsMaximized )
    {
    	CPaintDC dc(this);
    }
    else
    {
        CSplitterWnd::OnPaint();
    }
}

void CParametricSplitter::RecalcLayout()
{
    if( !m_IsMaximized )
        CSplitterWnd::RecalcLayout();

    x_DebugMsg( "RecalcLayout\n" );
}
