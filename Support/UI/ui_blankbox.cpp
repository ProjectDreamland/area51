//=========================================================================
//
//  ui_blankbox.cpp
//
//=========================================================================

#include "entropy.hpp"
#include "..\AudioMgr\audioMgr.hpp"

#include "ui_blankbox.hpp"
#include "ui_manager.hpp"
#include "ui_font.hpp"

//=========================================================================
//  Defines
//=========================================================================

//=========================================================================
//  Structs
//=========================================================================

//=========================================================================
//  Data
//=========================================================================

//=========================================================================
//  Factory function
//=========================================================================

ui_win* ui_blankbox_factory( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    ui_blankbox* pbox = new ui_blankbox;
    pbox->Create( UserID, pManager, Position, pParent, Flags );

    return (ui_win*)pbox;
}

//=========================================================================
//  ui_blankbox
//=========================================================================

ui_blankbox::ui_blankbox( void )
{
}

//=========================================================================

ui_blankbox::~ui_blankbox( void )
{
    Destroy();
}

//=========================================================================

xbool ui_blankbox::Create( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    xbool   Success;

    Success = ui_control::Create( UserID, pManager, Position, pParent, Flags );

    // Set default text flags
    m_LabelFlags = ui_font::h_center|ui_font::v_center|ui_font::clip_ellipsis|ui_font::clip_l_justify;

    // Initialize data
    m_Font              = g_UiMgr->FindFont( "small" );
    m_HasTitleBar       = FALSE;
    m_TitleBarColor     = XCOLOR_BLACK;
    m_BackgroundColor   = xcolor( 0, 0, 0, 0 );
    m_BitmapID          = -1;

    return Success;
}

//=========================================================================

void ui_blankbox::Render( s32 ox, s32 oy )
{
    // Only render is visible
    if( m_Flags & WF_VISIBLE )
    {
        xcolor  TextColor1  = XCOLOR_WHITE;
        xcolor  TextColor2  = XCOLOR_BLACK;
        s32     State       = ui_manager::CS_NORMAL;

        // Calculate rectangle
        irect   br;
        br.Set( (m_Position.l+ox), (m_Position.t+oy), (m_Position.r+ox), (m_Position.b+oy) );

        // Render appropriate state
        if( m_Flags & WF_DISABLED )
        {
            State = ui_manager::CS_DISABLED;
            TextColor1  = XCOLOR_GREY;
            TextColor2  = xcolor(0,0,0,0);
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == WF_HIGHLIGHT )
        {
            State = ui_manager::CS_HIGHLIGHT;
            TextColor1  = XCOLOR_WHITE;
            TextColor2  = XCOLOR_BLACK;
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == WF_SELECTED )
        {
            State = ui_manager::CS_SELECTED;
            TextColor1  = XCOLOR_WHITE;
            TextColor2  = XCOLOR_BLACK;
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == (WF_HIGHLIGHT|WF_SELECTED) )
        {
            State = ui_manager::CS_HIGHLIGHT_SELECTED;
            TextColor1  = XCOLOR_WHITE;
            TextColor2  = XCOLOR_BLACK;
        }
        else
        {
            State = ui_manager::CS_NORMAL;
            TextColor1  = XCOLOR_WHITE;
            TextColor2  = XCOLOR_BLACK;
        }

        if ( m_HasTitleBar )
        {
            // Render title bar
            irect tb = br;        
            tb.SetHeight( 22 );

            m_pManager->RenderRect( tb, m_TitleBarColor, FALSE );

            // render title
            tb.Deflate( 8, 0 );
            tb.Translate( 1, -1 );
            m_pManager->RenderText( m_Font, tb, ui_font::h_left|ui_font::v_center, xcolor(XCOLOR_BLACK), m_Label );
            tb.Translate( -1, -1 );
            m_pManager->RenderText( m_Font, tb, ui_font::h_left|ui_font::v_center, m_LabelColor, m_Label );

            // Render background color
            br.t += 22;
            m_pManager->RenderRect( br, m_BackgroundColor, FALSE );
        }
        else
        {
            // Render background color
            m_pManager->RenderRect( br, m_BackgroundColor, FALSE );
        }

        // render bitmap
        if( m_BitmapID != -1 )
        {
            irect bp = m_BitmapPos;
            bp.l += br.l;
            bp.r += br.l;
            bp.t += br.t;
            bp.b += br.t;
            m_pManager->RenderBitmap( m_BitmapID, bp );
        }

        // Render children
        for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
        {
            m_Children[i]->Render( m_Position.l+ox, m_Position.t+oy );
        }
    }
}

//=========================================================================

void ui_blankbox::SetBackgroundColor( xcolor Color )
{
    m_BackgroundColor = Color;
}

//=========================================================================

xcolor ui_blankbox::GetBackgroundColor( void ) const
{
    return m_BackgroundColor;
}

//=========================================================================

void ui_blankbox::SetHasTitleBar( xbool hasTitle )
{
    m_HasTitleBar = hasTitle;
}

//=========================================================================

void ui_blankbox::SetTitleBarColor( xcolor Color )
{
    m_TitleBarColor = Color;
}

//=========================================================================

xcolor ui_blankbox::GetTitleBarColor( void ) const
{
    return m_TitleBarColor;
}

//=========================================================================

void ui_blankbox::SetBitmap( s32 BitmapID )
{
    m_BitmapID = BitmapID;
}

//=========================================================================

void ui_blankbox::SetBitmap( s32 BitmapID, irect& Pos )
{
    m_BitmapID = BitmapID;
    m_BitmapPos = Pos;
}

//=========================================================================

void ui_blankbox::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;  
}

//=========================================================================

