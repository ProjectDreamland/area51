// FXEditorView3D.cpp : implementation file
//

#include "stdafx.h"
#include "FXEditorView3D.h"

#include "parted.h"
#include "PartEdDoc.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFXEditorView3D

IMPLEMENT_DYNCREATE(CFXEditorView3D, CEditorView3D)

//===========================================================================

CFXEditorView3D::CFXEditorView3D()
 :  m_NavMode           ( NAV_NONE ),
    m_DrawZoomMarquee   ( false    )
{

}

//===========================================================================

CFXEditorView3D::~CFXEditorView3D()
{

}

//===========================================================================

BEGIN_MESSAGE_MAP(CFXEditorView3D, CEditorView3D)
	//{{AFX_MSG_MAP(CFXEditorView3D)
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFXEditorView3D diagnostics

#ifdef _DEBUG
void CFXEditorView3D::AssertValid() const
{
	CEditorView3D::AssertValid();
}

void CFXEditorView3D::Dump(CDumpContext& dc) const
{
	CEditorView3D::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFXEditorView3D message handlers

int CFXEditorView3D::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEditorView3D::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    // Load our Rt-Click menu
    m_PopupMenu.LoadMenu( IDR_MENU_VIEW3D );
	
    // Initialize the Camera
    UpdateView();

	return 0;
}

//===========================================================================

void CFXEditorView3D::OnDraw(CDC* pDC)
{
    f32             GlobalTime  = g_pMainFrame->GetGlobalTime();
    CPartEdDoc*     pDoc        = (CPartEdDoc*)GetDocument();
    ASSERT( pDoc );

    // Prepare the Entropy engine to render this viewport & start rendering
    PreRenderSetup();
    {
        eng_Begin( "FX_EditorView3D" );
        {
            if( m_ShowBackground )
            {
                pDoc->m_Effect.RenderBackground( GlobalTime );
            }

            if( m_ShowGrid )
            {
                m_Grid.Render();
            }

            /************************************************************************************************/
            /************* Need to change this so you can filter out drawing of different element types *****/
            /************************************************************************************************/
            pDoc->m_Effect.Render( GlobalTime );
            /************************************************************************************************/

            /************************************************************************************************/
            /******************************         DEBUG STUFF           ***********************************/
            /************************************************************************************************/

            draw_Point( vector3(0,0,0),          xcolor(255,255,255,255) ); // Origin
            draw_Point( vector3(50,0,50),        xcolor(  0,  0,255,255) ); // Orbit Point
            draw_Point( m_Camera.GetTargetPos(), xcolor(255,255,  0,255) ); // Camera Target

            draw_BBox( bbox(vector3(0,0,0), 50), xcolor(0,255,0,255) );

            /************************************************************************************************/

            eng_End();
        }
        
        eng_PageFlip(); // Done rendering all 3D stuff
    }

    // Draw Transform Gizmos
    if( m_ShowTransformGizmos )
    {
        g_ManipulatorMgr.Render( m_View );
    }

    /************************************************************************************************/
    // Need to figure out what to do with Marquees.
    // If they will both look the same, I should just have one bool represent both
    // If they will look different, then I should figure out what I want each to look like
    /************************************************************************************************/

    // Draw Selection/Zoom Marquee
    if( m_DrawSelectMarquee || m_DrawZoomMarquee )
    {
        DrawMarquee( pDC, m_MouseMgr.GetClickPos(), m_MouseMgr.GetPos() );
    }

    /************************************************************************************************/
    /******************************         DEBUG STUFF           ***********************************/
    /************************************************************************************************/

    x_printfxy( 1, 1, "%f", m_Camera.GetPosition().GetX() );
    x_printfxy( 1, 2, "%f", m_Camera.GetPosition().GetY() );
    x_printfxy( 1, 3, "%f", m_Camera.GetPosition().GetZ() );

    x_printfxy( 14, 1, "%f", m_Camera.GetTargetPos().GetX() );
    x_printfxy( 14, 2, "%f", m_Camera.GetTargetPos().GetY() );
    x_printfxy( 14, 3, "%f", m_Camera.GetTargetPos().GetZ() );

    x_printfxy( 40, 1, "Undos: %d", m_Camera.GetUndoCount() );
    x_printfxy( 40, 2, "Redos: %d", m_Camera.GetRedoCount() );

    /************************************************************************************************/
}

//===========================================================================

void CFXEditorView3D::OnLButtonDown(UINT nFlags, CPoint point)
{
    // Call Base Class first, to update MouseMgr
	CEditorView3D::OnLButtonDown(nFlags, point);

    // Update Navigation
    SetNavigateMode();
}

//===========================================================================

void CFXEditorView3D::OnMButtonDown(UINT nFlags, CPoint point) 
{
    // Call Base Class first, to update MouseMgr
	CEditorView3D::OnMButtonDown(nFlags, point);

    // Update Navigation
    SetNavigateMode();
}

//===========================================================================

void CFXEditorView3D::OnRButtonDown(UINT nFlags, CPoint point) 
{
    // Call Base Class first, to update MouseMgr
	CEditorView3D::OnRButtonDown(nFlags, point);

    if( m_NavMode != NAV_NONE )
    {
        // Update Navigation
        SetNavigateMode();
    }
}

//===========================================================================

void CFXEditorView3D::OnLButtonUp(UINT nFlags, CPoint point) 
{
    // Call Base Class first, to update MouseMgr
	CEditorView3D::OnLButtonUp(nFlags, point);

    // Update Navigation
    SetNavigateMode();
}

//===========================================================================

void CFXEditorView3D::OnMButtonUp(UINT nFlags, CPoint point) 
{
    // Call Base Class first, to update MouseMgr
	CEditorView3D::OnMButtonUp(nFlags, point);

    // Update Navigation
    SetNavigateMode();
}

//===========================================================================

void CFXEditorView3D::OnRButtonUp(UINT nFlags, CPoint point) 
{
    // Call Base Class first, to update MouseMgr
	CEditorView3D::OnRButtonUp(nFlags, point);

    // Update Navigation
    SetNavigateMode();
}

//===========================================================================

BOOL CFXEditorView3D::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
    // Call Base Class first, to update MouseMgr
    CEditorView3D::OnMouseWheel(nFlags, zDelta, pt);

    static f32  zoomAmount;
    zoomAmount  = (f32)zDelta / -1200.0f;   // Reset to +/- 0.1

    m_Camera.Navigate_Begin();
    m_Camera.Zoom( zoomAmount );
    m_Camera.Navigate_End();

    UpdateView();

    // Return whether mouse wheel scrolling is enabled or not
	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

//===========================================================================

void CFXEditorView3D::OnMouseMove(UINT nFlags, CPoint point) 
{
    // Call Base Class first, to update MouseMgr
	CEditorView3D::OnMouseMove(nFlags, point);

    // If NavMode changes, we need to end the current operation and start a new one
    if( GetNavMode() != m_NavMode )
    {
        SetNavigateMode();

        // Simulate re-clicking.  This will reset the mouse click/drag pos for our new navigation
        m_MouseMgr.OnButtonUp   ( nFlags, point );
        m_MouseMgr.OnButtonDown ( nFlags, point );
    }

    // Get the normalized mouse movement in X & Y
    f32 normX   = (f32)m_MouseMgr.GetDragPos().x / (f32)m_Width;
    f32 normY   = (f32)m_MouseMgr.GetDragPos().y / (f32)m_Height;

    switch( m_NavMode )
    {
        case    NAV_PAN:                { m_Camera.Pan       ( normX, normY );                   break; }
        case    NAV_FLY:                { m_Camera.Fly       ( normX, normY );                   break; }
        case    NAV_LOOK:               { m_Camera.Look      ( normX, normY );                   break; }
        case    NAV_ORBIT:              { m_Camera.Orbit     ( normX, normY );                   break; }
        case    NAV_ORBIT_SELECTED:     { m_Camera.OrbitPoint( normX, normY, vector3(50,0,50) ); break; }
        case    NAV_ZOOM:               { m_Camera.Zoom      ( normY );                          break; }
        case    NAV_ZOOM_REGION:        {                                                        break; }
    }

    if( m_NavMode != NAV_NONE )
    {
        UpdateView();
    }
}

//===========================================================================

void CFXEditorView3D::UpdateView( void ) 
{
    m_View.SetRotation( radian3(m_Camera.GetRotation()) );
    m_View.SetPosition( m_Camera.GetPosition() );

    RedrawWindow();
}

//===========================================================================

void CFXEditorView3D::SetNavigateMode( void ) 
{
    // Get values we need to determine navigation...for easier reading
    xbool   Shift           = m_MouseMgr.GetKey_Shift();
    xbool   Control         = m_MouseMgr.GetKey_Control();
    xbool   Alt             = m_MouseMgr.GetKey_Alt();

    xbool   LeftButton      = m_MouseMgr.GetButton_Left();
    xbool   MiddleButton    = m_MouseMgr.GetButton_Middle();
    xbool   RightButton     = m_MouseMgr.GetButton_Right();

    xbool   SelectCount     = 1;    /***** Need to figure out how to handle scene selection integration ********/

    // Finish or Cancel the current operation...if there is one
    if( m_NavMode != NAV_NONE )
    {
        if( RightButton )
        {
            m_Camera.Navigate_Cancel();
            UpdateView();
            return;
        }
        else if( m_NavMode == NAV_ZOOM_REGION )
        {
            // Get the normalized mouse pos & click pos
            f32 normClickX  = (f32)m_MouseMgr.GetClickPos().x / (f32)m_Width;
            f32 normClickY  = (f32)m_MouseMgr.GetClickPos().y / (f32)m_Height;

            f32 normPosX    = (f32)m_MouseMgr.GetPos().x / (f32)m_Width;
            f32 normPosY    = (f32)m_MouseMgr.GetPos().y / (f32)m_Height;

            m_Camera.Navigate_Begin();
            m_Camera.ZoomRegion( normClickX, normClickY, normPosX, normPosY );
            m_Camera.Navigate_End();

            m_DrawZoomMarquee = false;
        }
        else
        {
            m_Camera.Navigate_End();
        }

        UpdateView();
        m_NavMode = NAV_NONE;
    }


    // Figure out the new navigation mode
    if( LeftButton )
    {
        /****************************************************************************************************/
        /******* THESE ARE ALL COMMANDS...NOT NAVIGATION MODES.....TEMPORARILY HERE FOR TESTING *************/
        /****************************************************************************************************/

             if( Shift && Control )             { m_Camera.Navigate_Undo(); UpdateView(); }
        else if( Shift && Alt     )             { m_Camera.Navigate_Redo(); UpdateView(); }
        else if( Control && Alt   )
        {
            m_Camera.Navigate_Begin();
            m_Camera.ZoomExtents( bbox(vector3(0,0,0), 50) );
            m_Camera.Navigate_End();
            UpdateView();
        }
        else
        {
            m_NavMode = NAV_ZOOM_REGION;
            m_DrawZoomMarquee = true;
        }

        /****************************************************************************************************/
    }
    else if( MiddleButton )
    {
             if( Shift )                        { m_NavMode   = NAV_FLY;            }
        else if( Control && Alt )               { m_NavMode   = NAV_ZOOM;           }
        else if( Control )                      { m_NavMode   = NAV_LOOK;           }
        else if( Alt && (SelectCount > 0) )     { m_NavMode   = NAV_ORBIT_SELECTED; }
        else if( Alt )                          { m_NavMode   = NAV_ORBIT;          }
        else                                    { m_NavMode   = NAV_PAN;            }

        // Begin navigation
        m_Camera.Navigate_Begin();
    }
    else if( RightButton )
    {

    }

    UpdateView();
}

//===========================================================================

CFXEditorView3D::NavMode    CFXEditorView3D::GetNavMode( void ) 
{
    // Get values we need to determine navigation...for easier reading
    xbool   Shift           = m_MouseMgr.GetKey_Shift();
    xbool   Control         = m_MouseMgr.GetKey_Control();
    xbool   Alt             = m_MouseMgr.GetKey_Alt();

    xbool   LeftButton      = m_MouseMgr.GetButton_Left();
    xbool   MiddleButton    = m_MouseMgr.GetButton_Middle();
    xbool   RightButton     = m_MouseMgr.GetButton_Right();

    xbool   SelectCount     = 1;    /***** Need to figure out how to handle scene selection integration ********/

    // Figure out the new navigation mode
    if( LeftButton )
    {
        if( !Shift && !Control && !Alt )        { return NAV_ZOOM_REGION;    }
    }
    if( MiddleButton )
    {
             if( Shift )                        { return NAV_FLY;            }
        else if( Control && Alt )               { return NAV_ZOOM;           }
        else if( Control )                      { return NAV_LOOK;           }
        else if( Alt && (SelectCount > 0) )     { return NAV_ORBIT_SELECTED; }
        else if( Alt )                          { return NAV_ORBIT;          }
        else                                    { return NAV_PAN;            }
    }

    return NAV_NONE;
}

