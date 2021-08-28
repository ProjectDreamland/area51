// CRulerBar : implementation file
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Resource.h"
#include "RulerBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

#define SCALE_HEIGHT        22
#define SCROLLBAR_HEIGHT    (::GetSystemMetrics(SM_CYHSCROLL))
#define RULER_HEIGHT        (SCALE_HEIGHT+SCROLLBAR_HEIGHT)

/////////////////////////////////////////////////////////////////////////////
// CRulerBar

CRulerBar::CRulerBar()
{
	// TODO: add construction code here.

}

CRulerBar::~CRulerBar()
{
	// TODO: add destruction code here.

}

/////////////////////////////////////////////////////////////////////////////

BOOL CRulerBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
	return CreateEx(pParentWnd, 0, dwStyle, CRect(m_cxLeftBorder, m_cyTopBorder, m_cxRightBorder, m_cyBottomBorder), nID);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CRulerBar::CreateEx(CWnd* pParentWnd, DWORD dwCtrlStyle, DWORD dwStyle, CRect rcBorders, UINT nID)
{
	ASSERT_VALID(pParentWnd);	// must have a parent
	ASSERT (!((dwStyle & CBRS_SIZE_FIXED) && (dwStyle & CBRS_SIZE_DYNAMIC)));

	SetBorders(rcBorders);

	// save the style
	m_dwStyle = (dwStyle & (CBRS_ALL|CBRS_GRIPPER));
	if (nID == AFX_IDW_TOOLBAR)
		m_dwStyle |= CBRS_HIDE_INPLACE;

	dwStyle &= ~CBRS_ALL;
	dwStyle |= CCS_NOPARENTALIGN|CCS_NOMOVEY|CCS_NODIVIDER|CCS_NORESIZE|WS_CLIPCHILDREN;
	dwStyle |= dwCtrlStyle;

	// create the HWND
	CRect rect; rect.SetRectEmpty();
	if (!CWnd::Create( NULL, NULL, dwStyle, rect, pParentWnd, nID))
		return FALSE;

	// Set the font.
	SetFont(&m_font);

    // Create the scrollbar
    VERIFY(m_ScrollBar.Create( SBS_HORZ | SBS_BOTTOMALIGN | WS_CHILD, CRect(0, SCALE_HEIGHT, GetParentClientWidth(), SCROLLBAR_HEIGHT), this, 100));
    SCROLLINFO Info;
    Info.cbSize = sizeof(Info);
    Info.fMask = SIF_ALL;
    Info.nMin = 0;
    Info.nMax = 256000;
    Info.nPage = MAX(1,GetParentClientWidth());
    Info.nPos = 0;
    Info.nTrackPos = 0;
    m_ScrollBar.SetScrollInfo( &Info );
    m_ScrollBar.ShowScrollBar();

	// Note: Parent must resize itself for control bar to be resized
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////

int CRulerBar::GetParentClientWidth( void )
{
    CFrameWnd* pFrame = GetDockingFrame();
    ASSERT( pFrame );
    CRect r;
    pFrame->GetClientRect( &r );
    return r.Width();
}

/////////////////////////////////////////////////////////////////////////////

CSize CRulerBar::CalcDynamicLayout(int nLength, DWORD dwMode)
{
    return CSize( GetParentClientWidth(), RULER_HEIGHT );
}

/////////////////////////////////////////////////////////////////////////////

void CRulerBar::DoPaint(CDC* pDC)
{
    CRect r;
    GetClientRect( &r );
    r.bottom = r.top + SCALE_HEIGHT;

    SCROLLINFO Info;
    m_ScrollBar.GetScrollInfo( &Info );
    m_Ruler.DrawRuler( pDC, r, Info.nPos, 1 );
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CRulerBar, CXTControlBar)
	//{{AFX_MSG_MAP(CRulerBar)
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

void CRulerBar::OnUpdateCmdUI( CFrameWnd* pTarget, BOOL bDisableIfNoHndler )
{
}

/////////////////////////////////////////////////////////////////////////////
// CRulerBar message handlers

void CRulerBar::OnSize(UINT nType, int cx, int cy) 
{
	CXTControlBar::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	if( m_ScrollBar.GetSafeHwnd() )
    {
        // Reset the page size
        SCROLLINFO Info;
        m_ScrollBar.GetScrollInfo( &Info );
        Info.cbSize = sizeof(Info);
        Info.fMask = SIF_PAGE;
        Info.nPage = MAX(1,cx);
        m_ScrollBar.SetScrollInfo( &Info, FALSE );

        // Move the scrollbar
        m_ScrollBar.MoveWindow( 0, SCALE_HEIGHT, cx, cy-SCALE_HEIGHT );
    }
}

/////////////////////////////////////////////////////////////////////////////

void CRulerBar::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
    SCROLLINFO  Info;
    pScrollBar->GetScrollInfo( &Info, SIF_ALL );

    // Get the current position of scroll box.
    int Current = Info.nPos;

    // Update current position
	switch( nSBCode )
    {
    case SB_LEFT:
        Current = Info.nMin;
        break;
    case SB_RIGHT:
        Current = Info.nMax;
        break;
    case SB_ENDSCROLL:
        break;
    case SB_LINELEFT:
        if( Current > Info.nMin )
            Current--;
        break;
    case SB_LINERIGHT:
        if( Current < Info.nMax )
            Current++;
        break;
    case SB_PAGELEFT:
        Current = MAX( Info.nMin, Current - (int)Info.nPage );
        break;
    case SB_PAGERIGHT:
        Current = MIN( Info.nMax, Current + (int)Info.nPage );
        break;
    case SB_THUMBPOSITION:
        Current = Info.nTrackPos;
        break;
    case SB_THUMBTRACK:
        Current = Info.nTrackPos;
        break;
    }

    // Set new position & redraw ruler
    if( Current != Info.nPos )
    {
        pScrollBar->SetScrollPos( Current );
        Invalidate();
    }

	CXTControlBar::OnHScroll(nSBCode, nPos, pScrollBar);
}

/////////////////////////////////////////////////////////////////////////////
