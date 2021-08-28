//=========================================================================
//
//  ui_control.cpp
//
//=========================================================================

#include "entropy.hpp"
#include "ui_control.hpp"
#include "ui_manager.hpp"

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
//  ui_control
//=========================================================================

ui_control::ui_control( void )
{
    m_NavPos.Set( 0, 0, 0, 0 );
}

//=========================================================================

ui_control::~ui_control( void )
{
    Destroy();
}

//=========================================================================

xbool ui_control::Create( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    xbool   Success;

    Success = ui_win::Create( UserID, pManager, Position, pParent, Flags );

    return Success;
}

//=========================================================================

void ui_control::Render( s32 ox, s32 oy )
{
    // Only render is visible
    if( m_Flags & WF_VISIBLE )
    {
        // Render placeholder rectangle
        xcolor  Color = XCOLOR_WHITE;

        // Calculate rectangle
        irect    r;
        r.Set( (m_Position.l+ox), (m_Position.t+oy), (m_Position.r+ox), (m_Position.b+oy) );

        // Render appropriate state
        if( m_Flags & WF_DISABLED )
        {
            Color = XCOLOR_GREY;
        }
        else if( m_Flags & (WF_HIGHLIGHT|WF_SELECTED) )
        {
            Color = xcolor(255,0,0,255);
        }
        else
        {
            Color = XCOLOR_WHITE;
        }

        m_pManager->RenderRect( r, Color, TRUE );

        // Render children
        for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
        {
            m_Children[i]->Render( m_Position.l+ox, m_Position.t+oy );
        }
    }
}

//=========================================================================

const irect& ui_control::GetNavPos( void )
{
    return m_NavPos;
}

//=========================================================================

void ui_control::SetNavPos( const irect& r )
{
    m_NavPos = r;
}

//=========================================================================
