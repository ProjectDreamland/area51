//=========================================================================
//
//  ui_dialog.cpp
//
//=========================================================================

#include "entropy.hpp"
#include "ui_dialog.hpp"
#include "ui_manager.hpp"
#include "ui_control.hpp"
#include "ui_edit.hpp"
#include "ui_font.hpp"
#include "StringMgr\StringMgr.hpp"
#include "AudioMgr\AudioMgr.hpp"

#if defined(TARGET_PS2)
#include "Entropy\PS2\ps2_misc.hpp"
#endif

//=========================================================================
//  Defines
//=========================================================================

//=========================================================================
//  Structs
//=========================================================================

//=========================================================================
//  Data
//=========================================================================

// define the global dialog colors
xcolor   ui_dialog::m_TextColorNormal;
xcolor   ui_dialog::m_TextColorShadow;

//=========================================================================
//  Factory function
//=========================================================================

ui_win* ui_dialog_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    ui_dialog* pDialog = new ui_dialog;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  ui_dialog
//=========================================================================

ui_dialog::ui_dialog( void )
{
#ifdef TARGET_XBOX
    m_XBOXNotificationOffsetX = 28;
    m_XBOXNotificationOffsetY = 6;
    m_bIsPopup                = 0;
    m_bUseTopmost             = 0;
#endif //TARGET_XBOX
}

//=========================================================================

ui_dialog::~ui_dialog( void )
{
    Destroy();
}

//=========================================================================

xbool ui_dialog::Create( s32                        UserID,
                         ui_manager*                pManager,
                         ui_manager::dialog_tem*    pDialogTem,
                         const irect&               Position,
                         ui_win*                    pParent,
                         s32                        Flags,
                         void*                      pUserData)
{
    xbool   Success = FALSE;
    s32     i;
    s32     x, y;

    ASSERT( pManager );

    // Get pointer to user
    ui_manager::user* pUser = pManager->GetUser( UserID );
    ASSERT( pUser );

    // Do window creation
    Success = ui_win::Create( UserID, pManager, Position, pParent, Flags );

    // Setup Dialog specific stuff
    m_iElement          = m_pManager->FindElement( "frame" );
    ASSERT( m_iElement != -1 );
    m_pDialogTem        = pDialogTem;
    m_NavW              = pDialogTem ? pDialogTem->NavW : 0;
    m_NavH              = pDialogTem ? pDialogTem->NavH : 0;
    m_NavX              = 0;
    m_NavY              = 0;

//  m_BackgroundColor   = FECOL_DIALOG;			  //-- Jhowa
    m_BackgroundColor   = xcolor(40,40,0,192);//xcolor(0, 20, 30,192);  //-- Jhowa

    m_InputEnabled      = TRUE;
    m_pUserData         = pUserData;
    m_State             = DIALOG_STATE_INIT;
    m_CurrentControl    = -1;

	m_OldCursorX        = pUser->CursorX;
    m_OldCursorY        = pUser->CursorY;

    // Setup Navgraph size
    m_NavGraph.SetCapacity( m_NavW * m_NavH );
    for( i=0; i<(m_NavW*m_NavH); i++ )
        m_NavGraph.Append() = 0;

    if( pDialogTem )
	{
        const xwchar* pLabel = g_StringTableMgr( "ui", pDialogTem->StringID );
        ASSERTS(pLabel,".stringbin files might not be compiled!");
		m_Label = pLabel;
        //xwstring tempString("Test");
        //m_Label = tempString;
	}
 
    // Save Template Pointer
    m_pDialogTem = pDialogTem;

    // Create controls and add to navigation graph if template exists
    if( pDialogTem )
    {
        for( i=0; i<pDialogTem->nControls; i++ )
        {
            ui_manager::control_tem*    pControlTem = &pDialogTem->pControls[i];

            // For now create each control
            irect Pos;
            Pos.Set( pControlTem->x, pControlTem->y, pControlTem->x + pControlTem->w, pControlTem->y + pControlTem->h );

            // check for scaling
            if( 1 )//pControlTem->Flags & ui_win::WF_SCALE_XPOS )
            {
                Pos.l = (s32)( (f32)pControlTem->x * m_pManager->GetScaleX() );
            }

            if( 1 )//pControlTem->Flags & ui_win::WF_SCALE_YPOS )
            {
                Pos.t = (s32)( (f32)pControlTem->y * m_pManager->GetScaleY() );
            }

            if( 1 )//pControlTem->Flags & ui_win::WF_SCALE_XSIZE )
            {
                Pos.r = Pos.l + (s32)( (f32)pControlTem->w * m_pManager->GetScaleX() );
            }

            if( 1 )//pControlTem->Flags & ui_win::WF_SCALE_YSIZE )
            {
                Pos.b = Pos.t + (s32)( (f32)pControlTem->h * m_pManager->GetScaleY() );
            }

            ui_control* pControl = (ui_control*)pManager->CreateWin( UserID, pControlTem->pClass, Pos, this, pControlTem->Flags );
            ASSERT( pControl );

            // Assign control ID
            pControl->SetControlID( pControlTem->ControlID );

            // Configure the control
			pControl->SetLabel( g_StringTableMgr( "ui", pControlTem->StringID ) );
			//pControl->SetLabel( m_Label );

            if( x_strcmp( pControlTem->pClass, "edit" ) == 0 )
            {
                ui_edit*    pEdit = (ui_edit*)pControl;	
                pEdit->SetVirtualKeyboardTitle( g_StringTableMgr( "ui", pControlTem->StringID ) );
            }

            // Add the control to the navigation graph
            ASSERT( pControlTem->nx >= 0 );
            ASSERT( pControlTem->ny >= 0 );
            ASSERT( (pControlTem->nx + pControlTem->nw) <= m_NavW );
            ASSERT( (pControlTem->ny + pControlTem->nh) <= m_NavH );

            // Save navgrid position
            pControl->SetNavPos( irect( pControlTem->nx, pControlTem->ny, pControlTem->nx+pControlTem->nw, pControlTem->ny+pControlTem->nh ) );

            // Add control into navgrid
            for( y=pControlTem->ny; y<(pControlTem->ny+pControlTem->nh); y++ )
            {
                for( x=pControlTem->nx; x<(pControlTem->nx+pControlTem->nw); x++ )
                {
                    m_NavGraph[x+y*m_NavW] = pControl;
                }
            }
        }
    }

    // Return success code
    return Success;
}

//=========================================================================

void ui_dialog::Render( s32 ox, s32 oy )
{
#ifdef TARGET_PC
    // If this is not a TAB dialog page
/*    if( !(GetFlags() & ui_win::WF_TAB) )
    {
        // Adjust the position of the dialogs according to the resolution.
        s32 XRes, YRes;
        eng_GetRes( XRes, YRes );
        s32 midX = XRes>>1;
        s32 midY = YRes>>1;

        s32 dx = midX - 256;
        s32 dy = midY - 256;
        ox = dx;
        oy = dy;
    }
*/
#endif

    // Get the hardware resolution for use later
    s32 XRes;
    s32 YRes;
    eng_GetRes( XRes, YRes );

    // Only render is visible
    if( m_Flags & WF_VISIBLE )
    {
        // Get window rectangle
        irect  r;
        r.Set( m_Position.l+ox, m_Position.t+oy, m_Position.r+ox, m_Position.b+oy );
        irect  rb = r;
        rb.Deflate( 1, 1 );

        // clamp scissor region to XRes
        irect clipRect = r;
        if( clipRect.l < 0 )
        {
            clipRect.l = 0;
            clipRect.r = XRes;
        }

        if (g_UiMgr->IsWipeActive())
        {
            irect wipePos;
            g_UiMgr->GetWipePos(wipePos);

#ifdef TARGET_PS2
            gsreg_Begin( 1 );
            gsreg_SetScissor(   clipRect.l, 
                                clipRect.t,
                                clipRect.r, 
                                wipePos.t);
            gsreg_End();
#endif

#ifdef TARGET_XBOX
            D3DRECT Rects[1];
            Rects[0].x1 = clipRect.l;
            Rects[0].y1 = clipRect.t;
            Rects[0].x2 = clipRect.r;
            Rects[0].y2 = wipePos.t;
            g_pd3dDevice->SetScissors( 1,FALSE,Rects );
#endif
        }

        // render the background highlight
        g_UiMgr->RenderScreenHighlight();

        // Render children
        for( s32 i=0; i<m_Children.GetCount(); i++ )
        {
            m_Children[i]->Render( m_Position.l+ox, m_Position.t+oy );
        }

        #ifdef TARGET_XBOX

        if( g_UiMgr->IsWipeActive( ))
        {
            D3DRECT Rects[1];
            Rects[0].x1 = clipRect.l;
            Rects[0].y1 = clipRect.t;
            Rects[0].x2 = clipRect.r;
            Rects[0].y2 = clipRect.b;
            g_pd3dDevice->SetScissors( 1,FALSE,Rects );
        }

        #endif

        #ifdef TARGET_PS2
        if (g_UiMgr->IsWipeActive())
        {
            gsreg_Begin( 1 );
            gsreg_SetScissor(   clipRect.l, 
                                clipRect.t,
                                clipRect.r, 
                                clipRect.b );
            gsreg_End();
        }
        #endif

        // render wipe
        g_UiMgr->RenderScreenWipe();

        // render refresh bar
        g_UiMgr->RenderRefreshBar();

        // Render frame
        if( m_Flags & WF_BORDER )
        {
            // Render background color
//            if( m_BackgroundColor.A > 0 )
//            {
//                m_pManager->RenderRect( rb, m_BackgroundColor, FALSE );
//            }

            // Render Title Bar Gradient
//            rb.SetHeight( 40 );
//          xcolor c1 = xcolor(150,150,0,255);//xcolor(30,100,150,255);//FECOL_TITLE1; 
//          xcolor c2 = xcolor(150,150,0,0);//xcolor(30,100,150,  0);//FECOL_TITLE2; 
//          m_pManager->RenderGouraudRect( rb, c1, c1, c2, c2, FALSE );

            // Render the Frame
            if( m_Flags & WF_DISABLED )
            {
                // disabled version
                m_pManager->RenderElement( m_iElement, r, 1 );
            }
            else
            {
                // normal frame
                m_pManager->RenderElement( m_iElement, r, 0 );
            }


            // Render Title
            if (!m_pManager->IsScreenScaling())
            {
                rb.Deflate( 0, 5 );
                s32 FontID = g_UiMgr->FindFont("large");
                m_pManager->RenderText( FontID, rb, ui_font::h_center, m_TextColorShadow, m_Label );
                rb.Translate( -1, -1 );
                m_pManager->RenderText( FontID, rb, ui_font::h_center, m_TextColorNormal, m_Label );
            }

            // render screen glow effect
            g_UiMgr->RenderScreenGlow();
        }

        // restore scissor region
#ifdef TARGET_PS2
        gsreg_Begin( 1 );
        gsreg_SetScissor( 0, 0, XRes, YRes );
        gsreg_End();
#endif

#ifdef TARGET_XBOX
        // ensure we're not using a shrunken viewport
        g_pd3dDevice->SetViewport( NULL );
#endif
    }
}

//=========================================================================

const irect& ui_dialog::GetCreatePosition( void ) const
{
    return m_CreatePosition;
}

//=========================================================================
//=========================================================================
//  Message Handler Functions
//=========================================================================
//=========================================================================

void ui_dialog::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    (void)pWin;
    (void)Presses;
    (void)Repeats;

    if (!m_InputEnabled)
        return;

    ui_manager::user*   pUser   = m_pManager->GetUser( m_UserID );
    s32                 x       = m_NavX;
    s32                 y       = m_NavY;
    s32                 dx      = 0;
    s32                 dy      = 0;

    // Which way are we moving
    switch( Code )
    {
    case ui_manager::NAV_UP:
        dy = -1;
        break;
    case ui_manager::NAV_DOWN:
        dy = 1;
        break;
    case ui_manager::NAV_LEFT:
        dx = -1;
        break;
    case ui_manager::NAV_RIGHT:
        dx = 1;
        break;
    }

/*
    x += dx;
    y += dy;
    if( x <       0 ) x = m_NavW-1;
    if( x >= m_NavW ) x = 0;
    if( y <       0 ) y = m_NavH-1;
    if( y >= m_NavH ) y = 0;
*/

    xbool bWrapped = FALSE;

    // Scan until the control to move to is found
    ui_win* pCurrentWin = pUser->pLastWindowUnderCursor;
//    while( ((x != m_NavX) && (dx != 0)) || ((y != m_NavY) && (dy != 0)) )
    while( ((x < m_NavW) && (x >= 0)) && ((y < m_NavH) && (y >= 0)) )
    {
        ui_win* pWin = m_NavGraph[x+y*m_NavW];
        if( pWin && (pWin != pCurrentWin) )
        {
            xbool Found = FALSE;

            if( pWin->GetFlags() & ui_win::WF_DISABLED )
            {
                if( dy != 0 )
                {
                    // look to the left and right
                    s32 xleft = x;
                    s32 xright = x;
                    xbool doneLeft = 0;
                    xbool doneRight = 0;
                    
                    while( 1 )
                    {
                        // look left
                        if( xleft > 0 )
                        {
                            xleft--;
                            pWin = m_NavGraph[xleft+y*m_NavW];
                            if( pWin && (pWin != pCurrentWin) && !( pWin->GetFlags() & ui_win::WF_DISABLED ) )
                            {
                                x = xleft;
                                Found = TRUE;
                                break;
                            }
                        }
                        else
                        {
                            doneLeft = TRUE;
                        }

                        // look right
                        if( xright < (m_NavW-1) )
                        {
                            xright++;
                            pWin = m_NavGraph[xright+y*m_NavW];
                            if( pWin && (pWin != pCurrentWin) && !( pWin->GetFlags() & ui_win::WF_DISABLED ) )
                            {
                                x = xright;
                                Found = TRUE;
                                break;
                            }
                        }
                        else
                        {
                            doneRight = TRUE;
                        }

                        if( doneLeft && doneRight )
                            break;
                    }
                }
            }
            else
            {
                Found = TRUE;
            }

            if( Found )
            {
                irect r = pWin->GetPosition();
                s32 cx = (r.r - r.l)/2;
                s32 cy = (r.b - r.t)/2;
                pWin->LocalToScreen( cx, cy );
                m_pManager->SetCursorPos( m_UserID, cx, cy );
                m_NavX = x;
                m_NavY = y;
                break;
            }
        }

        // Advance to next position
        x += dx;
        y += dy;

        // Check for wrapping
        if ( (WrapX) && (!bWrapped) )
        {
            if (x < 0)
            {
                x = m_NavW-1;
                bWrapped = TRUE;
            }

            if (x == m_NavW)
            {
                x = 0;
                bWrapped = TRUE;
            }
        }

        if ( (WrapY) && (!bWrapped) )
        {
            if (y < 0)
            {
                y = m_NavH-1;
                bWrapped = TRUE;
            }

            if (y == m_NavH)
            {
                y = 0;
                bWrapped = TRUE;
            }
        }
/*
        if( x <       0 ) x = m_NavW-1;
        if( x >= m_NavW ) x = 0;
        if( y <       0 ) y = m_NavH-1;
        if( y >= m_NavH ) y = 0;
*/
    }
}

//=========================================================================

void ui_dialog::SetBackgroundColor( xcolor Color )
{
    m_BackgroundColor = Color;
}

//=========================================================================

xcolor ui_dialog::GetBackgroundColor( void ) const
{
    return m_BackgroundColor;
}

//=========================================================================

ui_control* ui_dialog::GotoControl( s32 iControl )
{
    ui_control* pControl = NULL;

    ASSERT( (iControl >= 0) && (iControl < m_Children.GetCount()) );

    if( (iControl < 0) || (iControl >= m_Children.GetCount()) )
    {
        return NULL;
    }

    ui_control* pChild = (ui_control*)m_Children[iControl];

    ASSERT( pChild );
    if( !pChild ) return NULL;

    // Only goto non-static & enabled controls
    if( (!(pChild->GetFlags() & ui_win::WF_STATIC  )) &&
        (!(pChild->GetFlags() & ui_win::WF_DISABLED)) )
    {
        s32 x = pChild->GetWidth() / 2;
        s32 y = pChild->GetHeight() / 2;

        // Position cursor over Child
        pChild->LocalToScreen( x, y );
        m_pManager->SetCursorPos( m_UserID, x, y );

        // Set Navigation cursor to center of the control
        const irect& r = pChild->GetNavPos( );
        m_NavX = r.l + r.GetWidth() / 2;
        m_NavY = r.t + r.GetHeight() / 2;

        // store index of current control
        m_CurrentControl = iControl;

        pControl = pChild;
    }
    return pControl;
}

//=========================================================================

xbool ui_dialog::GotoControl( ui_control* pControl )
{
    xbool   Success = FALSE;
    s32     iControl = -1;

    ui_control* pChild = NULL;

    // Locate the control
    for( s32 i=0; i<m_Children.GetCount(); i++ )
    {
        if( m_Children[i] == pControl )
        {
            pChild = (ui_control*)m_Children[i];
            iControl = i;
        }
    }
    ASSERT( pChild );

    // Only goto non-static & enabled controls
    if( (!(pChild->GetFlags() & ui_win::WF_STATIC  )) &&
        (!(pChild->GetFlags() & ui_win::WF_DISABLED)) )
    {
        s32 x = pChild->GetWidth() / 2;
        s32 y = pChild->GetHeight() / 2;

        // Position cursor over Child
        pChild->LocalToScreen( x, y );
        m_pManager->SetCursorPos( m_UserID, x, y );

        // Set Navigation cursor to center of the control
        const irect& r = pChild->GetNavPos( );
        m_NavX = r.l + r.GetWidth() / 2;
        m_NavY = r.t + r.GetHeight() / 2;

        // store index of current control
        m_CurrentControl = iControl;

        Success = TRUE;
    }

    return Success;
}

//=========================================================================

s32 ui_dialog::GetNumControls( void ) const
{
    return m_Children.GetCount();
}

//=========================================================================

void ui_dialog::InitScreenScaling( const irect& Position )
{
    // store requested frame size
    m_RequestedPos = Position;

    // set starting position
    g_UiMgr->GetScreenSize(m_CurrPos);
    m_StartPos = m_CurrPos;
    
    // set up scaling
    m_scaleCount = 0.3f; // time to scale in seconds
    m_scaleAngle = 180.0f / m_scaleCount;
    m_scaleX = (f32)(Position.l - m_CurrPos.l) / 2.0f;
    m_totalX = 0.0f;

    // set starting position
    SetPosition(m_CurrPos);
    g_UiMgr->SetScreenScaling(TRUE);

    // play scaling sound
    if( g_UiMgr->IsScreenOn() )
    {
        if( m_scaleX > 0 )
        {
            g_AudioMgr.Play( "ResizeLarge" ); 
        }
        else
        {
            g_AudioMgr.Play( "ResizeSmall" );
        }
    }
    else
    {
        if( m_scaleX > 0 )
        {
            g_AudioMgr.Play( "Bars_Out" ); 
        }
        else
        {
            g_AudioMgr.Play( "Bars_In" );
        }
    }
}

//=========================================================================

xbool ui_dialog::UpdateScreenScaling( f32 DeltaTime, xbool DoWipe )
{
    // scale window if necessary
    if (m_scaleCount)
    {
#if defined( TARGET_PC ) && !defined( X_EDITOR )
        s32 XRes, YRes;
        eng_GetRes( XRes, YRes );
        DeltaTime = DeltaTime * YRes / 448;
#endif
        // apply delta time
        m_scaleCount -= DeltaTime;

        if (m_scaleCount <= 0)
        {
            // last one - make sure window is correct size
            m_scaleCount = 0;
            m_CurrPos = m_RequestedPos;

            // resize the window
            SetPosition(m_CurrPos);        
            g_UiMgr->SetScreenSize(m_CurrPos);
            g_UiMgr->SetScreenScaling(FALSE);

            // start a screen wipe
            if ( DoWipe )
            {
                g_UiMgr->InitScreenWipe();
            }
        }
        else
        {
            m_totalX = m_scaleX + (m_scaleX * x_cos( DEG_TO_RAD( m_scaleAngle * m_scaleCount ) ) );
            m_CurrPos.l = m_StartPos.l + (s32)m_totalX;
            m_CurrPos.r = m_StartPos.r - (s32)m_totalX;

            // resize the window
            SetPosition(m_CurrPos);        
            g_UiMgr->SetScreenSize(m_CurrPos);

            // still more to do!
            return TRUE;
        }
    }

    // we're done!
    return FALSE;
}

//=========================================================================
//=========================================================================


//=========================================================================

ui_manager::dialog_tem* ui_dialog::GetTemplate( void )
{
    return m_pDialogTem;
}

//=========================================================================



