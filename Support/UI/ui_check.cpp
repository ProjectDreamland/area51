//=========================================================================
//
//  ui_check.cpp
//
//=========================================================================

#include "entropy.hpp"
#include "..\AudioMgr\audioMgr.hpp"
#include "ui_check.hpp"
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

ui_win* ui_check_factory( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    ui_check* pcheck = new ui_check;
    pcheck->Create( UserID, pManager, Position, pParent, Flags );

    return (ui_win*)pcheck;
}

//=========================================================================
//  ui_check
//=========================================================================

ui_check::ui_check( void )
{
    m_bIsChecked = FALSE;
}

//=========================================================================

ui_check::~ui_check( void )
{
    Destroy();
}

//=========================================================================

xbool ui_check::Create( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    xbool   Success;

    Success = ui_control::Create( UserID, pManager, Position, pParent, Flags );

    // Initialize Data
    m_iElement = m_pManager->FindElement( "button_check" );
    ASSERT( m_iElement != -1 );

    return Success;
}

//=========================================================================

void ui_check::Render( s32 ox, s32 oy )
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
        else if( m_Flags & WF_HIGHLIGHT )
        {
            if( m_bIsChecked )
            {
                State = ui_manager::CS_HIGHLIGHT_SELECTED;
                TextColor1 = XCOLOR_WHITE;
                TextColor2 = XCOLOR_BLACK;
            }
            else
            {
                State = ui_manager::CS_HIGHLIGHT;
                TextColor1 = XCOLOR_WHITE;
                TextColor2 = XCOLOR_BLACK;
            }
        }
        else if( m_bIsChecked )
        {
            State = ui_manager::CS_SELECTED;
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

        // Render children
        for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
        {
            m_Children[i]->Render( m_Position.l+ox, m_Position.t+oy );
        }
    }
}

//=========================================================================

void ui_check::OnPadSelect( ui_win* pWin )
{
    if( pWin == (ui_win*)this )
    {
        m_Flags ^= WF_SELECTED;
        m_bIsChecked = !m_bIsChecked;

        // Notify Parent
        if( m_pParent )
            m_pParent->OnNotify( m_pParent, this, WN_CHECK_CHANGE, (void*)(m_Flags & WF_SELECTED) );

        //g_AudioMgr.Play("OptionSelect");
    }
}

//=========================================================================

void ui_check::SetChecked( xbool State )
{
    m_bIsChecked = State;
}

//=========================================================================

xbool ui_check::IsChecked( void ) const
{
    return m_bIsChecked;
}

//=========================================================================

void ui_check::OnLBDown ( ui_win* pWin )
{
    (void)pWin;

#ifndef TARGET_PC
    return;
#endif

    if( pWin == (ui_win*)this )
    {
        m_Flags ^= WF_SELECTED;

        // Notify Parent
        if( m_pParent )
            m_pParent->OnNotify( m_pParent, this, WN_CHECK_CHANGE, (void*)(m_Flags & WF_SELECTED) );

        //g_AudioMgr.Play("OptionSelect");
    }
}