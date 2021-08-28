// EditorView3D.cpp : implementation file
//

#include "stdafx.h"
#include "EditorView3D.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditorView3D

IMPLEMENT_DYNCREATE(CEditorView3D, CView)

//===========================================================================
// Initialize static class variables
//===========================================================================

xbool   CEditorView3D::m_ShowTransformGizmos = true;
xbool   CEditorView3D::m_ShowEmitterIcons    = true;

//===========================================================================

CEditorView3D::CEditorView3D()
{
    // Display Values
    m_ShadingMode       = SHADING_SHADED;
    m_HiliteSelected    = false;
    m_ShowBackground    = false;
    m_ShowGrid          = true;
    m_ShowTrajectories  = false;
    m_ShowEffectBounds  = false;
    m_ShowElementBounds = false;

    // UI
    m_View.SetXFOV( R_60 );
    m_View.SetPosition( vector3(100,100,200) );
    m_View.LookAtPoint( vector3(  0,  0,  0) );
    m_View.SetZLimits ( 1, 50000 );
    m_IsViewActive      = false;

    m_Width             = 0;
    m_Height            = 0;

    m_IsPopupMenuActive = false;
    m_DrawSelectMarquee = false;
}

//===========================================================================

CEditorView3D::~CEditorView3D()
{

}

//===========================================================================

BEGIN_MESSAGE_MAP(CEditorView3D, CView)
	//{{AFX_MSG_MAP(CEditorView3D)
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditorView3D diagnostics

#ifdef _DEBUG
void CEditorView3D::AssertValid() const
{
	CView::AssertValid();
}

void CEditorView3D::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CEditorView3D message handlers

int CEditorView3D::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    // Set the window style
    ModifyStyleEx( WS_EX_CLIENTEDGE, 0 );

    // Attach our MouseMgr to our window
    m_MouseMgr.AttachToWindow( this );

	return 0;
}

//===========================================================================

BOOL CEditorView3D::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
    return CView::PreCreateWindow(cs);
}

//===========================================================================

void CEditorView3D::OnLButtonDown(UINT nFlags, CPoint point)
{
    m_MouseMgr.OnButtonDown( nFlags, point );

    /*******************************************************************************************/
    /******** TEMPORARY...Need to have a ViewportMgr handle viewport activation ****************/
    /*******************************************************************************************/
    m_IsViewActive  = true;
    /*******************************************************************************************/
}

void CEditorView3D::OnMButtonDown(UINT nFlags, CPoint point)
{
    m_MouseMgr.OnButtonDown( nFlags, point );

    /*******************************************************************************************/
    /******** TEMPORARY...Need to have a ViewportMgr handle viewport activation ****************/
    /*******************************************************************************************/
    m_IsViewActive  = true;
    /*******************************************************************************************/
}

void CEditorView3D::OnRButtonDown(UINT nFlags, CPoint point)
{
    m_MouseMgr.OnButtonDown( nFlags, point );

    /*******************************************************************************************/
    /******** TEMPORARY...Need to have a ViewportMgr handle viewport activation ****************/
    /*******************************************************************************************/
    m_IsViewActive  = true;
    /*******************************************************************************************/
}

//===========================================================================

void CEditorView3D::OnLButtonUp(UINT nFlags, CPoint point)
{
    m_MouseMgr.OnButtonUp( nFlags, point );
}

void CEditorView3D::OnMButtonUp(UINT nFlags, CPoint point)
{
    m_MouseMgr.OnButtonUp( nFlags, point );
}

void CEditorView3D::OnRButtonUp(UINT nFlags, CPoint point)
{
    m_MouseMgr.OnButtonUp( nFlags, point );
}

//===========================================================================

void CEditorView3D::OnMouseMove(UINT nFlags, CPoint point)      { m_MouseMgr.OnMove( nFlags, point ); }

//===========================================================================

BOOL CEditorView3D::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    m_MouseMgr.OnWheel( nFlags, pt, zDelta );
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

//===========================================================================

BOOL CEditorView3D::OnEraseBkgnd(CDC* pDC) 
{
	// Just return...otherwise it'll "erase" it to white which causes flicker
	return FALSE;  //return CView::OnEraseBkgnd(pDC);
}

//===========================================================================

void CEditorView3D::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
    CRect           rcClient;
    GetClientRect   ( &rcClient );

    m_Width         = rcClient.right;
    m_Height        = rcClient.bottom;
}

//===========================================================================

void CEditorView3D::OnDraw(CDC* pDC)
{
    //---------------------------------------------------------------------------
    //  DO NOT MODIFY THIS!!! - This is just a demo render loop
    //
    //  To render your own stuff:
    //      1. Derive your own class from CEditorView3D
    //      2. Over-ride the OnDraw function
    //---------------------------------------------------------------------------

    // Prepare the Entropy engine to render this viewport & start rendering
    PreRenderSetup();
    {
        eng_Begin( "EditorView3D" );
        {
            m_Grid.Render();
            eng_End();
        }
        eng_PageFlip(); // Done rendering all 3D stuff
    }

    // Draw Transform Gizmos
    if( m_ShowTransformGizmos )
    {
        g_ManipulatorMgr.Render( m_View );
    }

    // Draw Selection Marquee
    if( m_DrawSelectMarquee )
    {
        DrawMarquee( pDC, m_MouseMgr.GetClickPos(), m_MouseMgr.GetPos() );
    }
}

//-------------------------------------------------------------------------
// Drawing Functions
//-------------------------------------------------------------------------

void    CEditorView3D::PreRenderSetup( void )
{
    ASSERT( !eng_InBeginEnd() );    // Make sure the engine's not already in the middle of rendering

    d3deng_UpdateDisplayWindow  ( GetSafeHwnd() );
    eng_MaximizeViewport        ( m_View );
    eng_SetView                 ( m_View );
    eng_SetBackColor            ( xcolor(0x0f, 0x0f, 0x0f, 0xff) );
}

//===========================================================================

void    CEditorView3D::DrawMarquee( CDC* pDC, CPoint pt1, CPoint pt2 )
{
    pDC->SelectStockObject( WHITE_PEN );
    pDC->MoveTo( pt1.x, pt1.y );
    pDC->LineTo( pt2.x, pt1.y );
    pDC->LineTo( pt2.x, pt2.y );
    pDC->LineTo( pt1.x, pt2.y );
    pDC->LineTo( pt1.x, pt1.y );
}

//-------------------------------------------------------------------------
// Internal Utilities
//-------------------------------------------------------------------------







