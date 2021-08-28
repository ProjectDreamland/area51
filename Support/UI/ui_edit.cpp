//=========================================================================
//
//  ui_edit.cpp
//
//=========================================================================

#include "entropy.hpp"
#include "ui_edit.hpp"
#include "ui_manager.hpp"
#include "ui_font.hpp"
#include "ui_dlg_vkeyboard.hpp"

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

ui_win* ui_edit_factory( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    ui_edit* pedit = new ui_edit;
    pedit->Create( UserID, pManager, Position, pParent, Flags );

    return (ui_win*)pedit;
}

//=========================================================================
//  ui_edit
//=========================================================================

ui_edit::ui_edit( void )
{
    m_bName = TRUE;
}

//=========================================================================

ui_edit::~ui_edit( void )
{
    Destroy();
}

//=========================================================================

xbool ui_edit::Create( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags )
{
    xbool   Success;

    Success = ui_control::Create( UserID, pManager, Position, pParent, Flags );

    // Initialize data
//    m_iElement1 = m_pManager->FindElement( "button_edit" );
//    m_iElement2 = m_pManager->FindElement( "button_edit" );
//    ASSERT( m_iElement1 != -1 );
//    ASSERT( m_iElement2 != -1 );

    m_iElement1 = m_pManager->FindElement( "button_edit" );
    ASSERT( m_iElement1 != -1 );

    m_LabelWidth    = 0;
    m_BufferSize    = -1;

    m_Font = g_UiMgr->FindFont("small");

    return Success;
}

//=========================================================================

void ui_edit::Render( s32 ox, s32 oy )
{
    s32     State = ui_manager::CS_NORMAL;

    // Only render is visible
    if( m_Flags & WF_VISIBLE )
    {
//        xcolor  LabelColor1 = XCOLOR_WHITE;
//        xcolor  LabelColor2 = XCOLOR_BLACK;
        xcolor  TextColor1  = XCOLOR_WHITE;
        xcolor  TextColor2  = XCOLOR_BLACK;

        // Calculate rectangle
        irect    r, r2;
        r.Set( (m_Position.l+ox), (m_Position.t+oy), (m_Position.r+ox), (m_Position.b+oy) );
        r2 = r;
//        r.r = r.l + m_LabelWidth;
//        r2.l = r.r;

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
        m_pManager->RenderElement( m_iElement1, r2, State );

        // Add Highlight to list
        if( m_Flags & WF_HIGHLIGHT )
            m_pManager->AddHighlight( m_UserID, r );

        // Render Label Text
//        r.Translate( 1-3, -2 );
//        m_pManager->RenderText( m_Font, r, ui_font::h_center|ui_font::v_center, LabelColor2, m_Label );
//        r.Translate( -1, -1 );
//        m_pManager->RenderText( m_Font, r, ui_font::h_center|ui_font::v_center, LabelColor1, m_Label );

/*        // Render Edit Text
        {
            r2.Deflate( 4, 0 );
            r2.Translate( 1, -1 );
            m_pManager->RenderText( m_Font, r2, ui_font::h_center|ui_font::v_center|ui_font::clip_character|ui_font::clip_l_justify, TextColor2, m_Label );
            r2.Translate( -1, -1 );
            m_pManager->RenderText( m_Font, r2, ui_font::h_center|ui_font::v_center|ui_font::clip_character|ui_font::clip_l_justify, TextColor1, m_Label );
        }
*/
        // Set a clip window to render the text
        r2.Deflate( 4, 1 );
        m_pManager->PushClipWindow( r2 );

        // Render Text
        irect rt = r2;
        rt.l += 1;
        rt.r -= 3;
        //rt.Translate(  3, -2 );
        //m_pManager->RenderText( m_Font, rt, ui_font::h_center|ui_font::v_center|ui_font::clip_ellipsis|ui_font::clip_l_justify, TextColor2, m_Label );
        //rt.Translate( -1, -1 );
        //m_pManager->RenderText( m_Font, rt, ui_font::h_center|ui_font::v_center|ui_font::clip_ellipsis|ui_font::clip_l_justify, TextColor1, m_Label );
        rt.Translate( 0, -10 );
        m_pManager->RenderText( m_Font, rt, ui_font::h_center|ui_font::v_center|ui_font::clip_ellipsis|ui_font::clip_l_justify, xcolor(255,252,204,255), m_Label );

        // Clear the clip window
        m_pManager->PopClipWindow();

        // Render children
        for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
        {
            m_Children[i]->Render( m_Position.l+ox, m_Position.t+oy );
        }
    }
}

//=========================================================================

void ui_edit::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    irect   r = m_pManager->GetUserBounds( m_UserID );

    // Open virtual keyboard dialog and connect it to the edit string
    ui_dlg_vkeyboard* pVKeyboard = (ui_dlg_vkeyboard*)m_pManager->OpenDialog( m_UserID, "ui_vkeyboard", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_INPUTMODAL );
    pVKeyboard->Configure( m_bName );
    pVKeyboard->ConnectString( &m_Label, m_BufferSize );
    pVKeyboard->SetLabel( m_VirtualKeyboardTitle );
}

//=========================================================================

void ui_edit::SetLabelWidth( s32 Width )
{
    m_LabelWidth = Width;
}

//=========================================================================

void ui_edit::SetBufferSize( s32 BufferSize )
{
    m_BufferSize = BufferSize;
}

//=========================================================================

void ui_edit::SetVirtualKeyboardTitle( const xwstring& Title )
{
    m_VirtualKeyboardTitle = Title;
}

/*
//=========================================================================

void ui_edit::SetText( const xstring& Text )
{
    m_Text = Text;
}

//=========================================================================

void ui_edit::SetText( const char* Text )
{
    m_Text = Text;
}

//=========================================================================

const xstring& ui_edit::GetText( void ) const
{
    return m_Text;
}
*/

//=========================================================================

void ui_edit::OnKeyDown ( ui_win* pWin, s32 Key )
{
    (void)pWin;
    (void)Key;
}

//=========================================================================

void ui_edit::OnKeyUp ( ui_win* pWin, s32 Key )
{
    (void)pWin;
    (void)Key;

}

//=========================================================================

void ui_edit::OnCursorEnter ( ui_win* pWin )
{
    (void)pWin;
    ui_win::OnCursorEnter( pWin );
}

//=========================================================================
