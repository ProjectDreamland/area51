/////////////////////////////////////////////////////////////////////////////
// INCLUDES
/////////////////////////////////////////////////////////////////////////////
#include "BaseStdAfx.h"
#include "BaseView.h"

/////////////////////////////////////////////////////////////////////////////
// MFC STUFF
/////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CBaseView, CView)

#ifdef _DEBUG
void CBaseView::AssertValid() const             { CView::AssertValid(); }
void CBaseView::Dump(CDumpContext& dc) const    { CView::Dump(dc); }
#endif //_DEBUG


BEGIN_MESSAGE_MAP(CBaseView, CView)
	//{{AFX_MSG_MAP(CBaseView)
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SYSKEYDOWN()	
	ON_WM_SYSKEYUP()	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

//===========================================================================

CBaseView::CBaseView()
{
    // Types of actions
    m_bActionView        = FALSE;

    // Initialize the mouse info
    m_MouseOldPos.x     = 0;
    m_MouseOldPos.y     = 0;
    m_MouseDeltaX       = 0;
    m_MouseDeltaY       = 0;
    m_MouseLeftButton   = FALSE;
    m_MouseRightButton  = FALSE;
}

//===========================================================================

CBaseView::~CBaseView()
{
}

//===========================================================================

void CBaseView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

//===========================================================================

void CBaseView::OnLButtonUp(UINT nFlags, CPoint point) 
{
    m_MouseLeftButton  = FALSE;
	CView::OnLButtonUp(nFlags, point);

    if( GetMouseLeftButton() == FALSE && GetMouseRightButton() == FALSE )
    {
        ::ReleaseCapture();
    }
}

//===========================================================================

void CBaseView::OnLButtonDown(UINT nFlags, CPoint point) 
{
    if( GetMouseLeftButton() == FALSE && GetMouseRightButton() == FALSE )
    {
        ::SetCapture( this->GetSafeHwnd() );
    }

    m_MouseLeftButton  = TRUE;
    m_MouseOldPos      = point;	
	CView::OnLButtonDown(nFlags, point);
}

//===========================================================================

void CBaseView::OnMouseMove(UINT nFlags, CPoint point) 
{
    if( nFlags & MK_LBUTTON    ) m_MouseLeftButton   = TRUE;
    else                         m_MouseLeftButton   = FALSE;

    if( nFlags & MK_RBUTTON    ) m_MouseRightButton  = TRUE;
    else                         m_MouseRightButton  = FALSE;

    m_MouseDeltaX   = point.x - m_MouseOldPos.x;
    m_MouseDeltaY   = point.y - m_MouseOldPos.y;
    m_MouseOldPos   = point;
	CView::OnMouseMove(nFlags, point);
}

//===========================================================================

void CBaseView::OnRButtonDown(UINT nFlags, CPoint point) 
{
    if( GetMouseLeftButton() == FALSE && GetMouseRightButton() == FALSE )
    {
        ::SetCapture( this->GetSafeHwnd() );
    }

    m_MouseRightButton = TRUE;
    m_MouseOldPos      = point;	
	CView::OnRButtonDown(nFlags, point);
}

//===========================================================================

void CBaseView::OnRButtonUp(UINT nFlags, CPoint point) 
{
    m_MouseRightButton = FALSE;
	CView::OnRButtonUp(nFlags, point);

    if( GetMouseLeftButton() == FALSE && GetMouseRightButton() == FALSE )
    {
        if( GetKeyState( VK_CONTROL ) & ~1 ) 
            m_bActionView = FALSE;
        ::ReleaseCapture();
    }
}

//===========================================================================

void CBaseView::OnSysKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	// regular menu stuff will still work (such as ALT-F for File)
    if ( nChar == VK_CONTROL )
    {
        m_bActionView = FALSE;
    }
}

//===========================================================================

void CBaseView::OnSysKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags )
{
	// regular menu stuff will still work (such as ALT-F for File)
    if ( nChar == VK_CONTROL )
    {
        m_bActionView = TRUE;
    }
}

//===========================================================================

xbool CBaseView::IsActionMode( void )
{
    return ( GetKeyState( VK_CONTROL ) & ~1 ) != 0;
}
    


void CBaseView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView) 
{
	// TODO: Add your specialized code here and/or call the base class
	m_bViewActive = bActivate;
	CView::OnActivateView(bActivate, pActivateView, pDeactiveView);
}
