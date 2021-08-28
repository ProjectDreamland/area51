//=========================================================================
//
//  ui_radio.cpp
//
//=========================================================================

#include "entropy.hpp"
#include "ui_radio.hpp"
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

ui_win* ui_radio_factory( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    ui_radio* pradio = new ui_radio;
    pradio->Create( UserID, pManager, Position, pParent, Flags );

    return (ui_win*)pradio;
}

//=========================================================================
//  ui_radio
//=========================================================================

ui_radio::ui_radio( void )
{
}

//=========================================================================

ui_radio::~ui_radio( void )
{
    Destroy();
}

//=========================================================================

xbool ui_radio::Create( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    xbool   Success;

    Success = ui_control::Create( UserID, pManager, Position, pParent, Flags );

    // Initialize Data
    m_iElement = m_pManager->FindElement( "button_check" );
    ASSERT( m_iElement != -1 );

    return Success;
}

//=========================================================================

void ui_radio::Render( s32 ox, s32 oy )
{
    s32     State = ui_manager::CS_NORMAL;

    // Only render is visible
    if( m_Flags & WF_VISIBLE )
    {
        xcolor  TextColor1 = XCOLOR_WHITE;
        xcolor  TextColor2 = XCOLOR_BLACK;

        // Calculate rectangle
        irect    r;
        r.Set( (m_Position.l+ox), (m_Position.t+oy), (m_Position.r+ox), (m_Position.b+oy) );

        // Render appropriate state
        if( m_Flags & WF_DISABLED )
        {
            State = ui_manager::CS_DISABLED;
            TextColor1 = XCOLOR_GREY;
            TextColor2 = xcolor(0,0,0,0);
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == WF_HIGHLIGHT )
        {
            State = ui_manager::CS_HIGHLIGHT;
            TextColor1 = XCOLOR_WHITE;
            TextColor2 = XCOLOR_BLACK;
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == WF_SELECTED )
        {
            State = ui_manager::CS_SELECTED;
            TextColor1 = XCOLOR_WHITE;
            TextColor2 = XCOLOR_BLACK;
        }
        else if( (m_Flags & (WF_HIGHLIGHT|WF_SELECTED)) == (WF_HIGHLIGHT|WF_SELECTED) )
        {
            State = ui_manager::CS_HIGHLIGHT_SELECTED;
            TextColor1 = XCOLOR_WHITE;
            TextColor2 = XCOLOR_BLACK;
        }
        else
        {
            State = ui_manager::CS_NORMAL;
            TextColor1 = XCOLOR_WHITE;
            TextColor2 = XCOLOR_BLACK;
        }
        m_pManager->RenderElement( m_iElement, r, State );

        // Add Highlight to list
        if( m_Flags & WF_HIGHLIGHT )
            m_pManager->AddHighlight( m_UserID, r );

        // Render Text
        r.Translate( 1, -1 );
        m_pManager->RenderText( g_UiMgr->FindFont("large"), r, ui_font::h_center|ui_font::v_center, TextColor2, m_Label );
        r.Translate( -1, -1 );
        m_pManager->RenderText( g_UiMgr->FindFont("large"), r, ui_font::h_center|ui_font::v_center, xcolor(150,150,150,255), m_Label );

        // Render children
        for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
        {
            m_Children[i]->Render( m_Position.l+ox, m_Position.t+oy );
        }
    }
}

//=========================================================================
