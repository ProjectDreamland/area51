//=========================================================================
//
//  ui_tabbed_dialog.cpp
//
//=========================================================================

#include "entropy.hpp"
#include "..\AudioMgr\audioMgr.hpp"

#include "ui_tabbed_dialog.hpp"
#include "ui_manager.hpp"
#include "ui_control.hpp"
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

ui_win* ui_tabbed_dialog_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    ui_tabbed_dialog* pDialog = new ui_tabbed_dialog;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  ui_tabbed_dialog
//=========================================================================

ui_tabbed_dialog::ui_tabbed_dialog( void )
{
}

//=========================================================================

ui_tabbed_dialog::~ui_tabbed_dialog( void )
{
    Destroy();
}

//=========================================================================

xbool ui_tabbed_dialog::Create( s32                        UserID,
                                ui_manager*                pManager,
                                ui_manager::dialog_tem*    pDialogTem,
                                const irect&               Position,
                                ui_win*                    pParent,
                                s32                        Flags,
                                void*                      pUserData )
{
    xbool   Success = FALSE;

    (void)pDialogTem;
    (void)pUserData;
    ASSERT( pManager );
    
    // Get pointer to user
    ui_manager::user* pUser = pManager->GetUser( UserID );
    ASSERT( pUser );

    // Do window creation
    Success = ui_win::Create( UserID, pManager, Position, pParent, Flags );

    // Setup Dialog specific stuff
    m_iElementFrame = m_pManager->FindElement( "frame" );
    ASSERT( m_iElementFrame != -1 );
    m_iElementTab = m_pManager->FindElement( "tab" );
    ASSERT( m_iElementTab != -1 );
    m_BackgroundColor   = xcolor(0, 20, 30,192);//FECOL_DIALOG; //-- Jhowa
    m_OldCursorX        = pUser->CursorX;
    m_OldCursorY        = pUser->CursorY;
    m_iActiveTab        = -1;
    m_TabWidth          = -1;
    m_pTabTracker       = NULL;

    // Return success code
    return Success;
}

//=========================================================================

void ui_tabbed_dialog::Render( s32 ox, s32 oy )
{
    s32     i;
    
#ifdef TARGET_PC
    // Adjust where the parent gets drawn according to the resolution.
/*    if( m_pParent == NULL )
    {        
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

    // Only render is visible
    if( m_Flags & WF_VISIBLE )
    {
        // Get window rectangle
        irect   r;
        r.Set( m_Position.l+ox, m_Position.t+oy, m_Position.r+ox, m_Position.b+oy );

        // Render Border
        if( m_Flags & WF_BORDER )
        {
            irect   br = r;
            br.t += 21;

            // Render background color
            if( m_BackgroundColor.A > 0 )
            {
                irect   rb = br;
                rb.Deflate( 1, 1 );
                m_pManager->RenderRect( rb, m_BackgroundColor, FALSE );
            }

            // Render Frame
            m_pManager->RenderElement( m_iElementFrame, br, 0 );
        }

        // Render Tabs
        r.SetHeight( 21 );
        r.Translate( 4, 0 );
        for( i=0 ; i<m_Tabs.GetCount() ; i++ )
        {
            xcolor  TextColor1   = XCOLOR_WHITE;
            xcolor  TextColor2   = XCOLOR_BLACK;
            s32     State = ui_manager::CS_NORMAL;

            // Set Tab Width
            if( m_TabWidth == -1 )
                r.SetWidth( m_Tabs[i].w );
            else
                r.SetWidth( m_TabWidth );

            // Highlight if active tab
            if( m_iActiveTab == i )
            {
                TextColor1   = XCOLOR_WHITE;
                TextColor2   = XCOLOR_BLACK;
                State = ui_manager::CS_HIGHLIGHT;

                // Add Highlight to list
                m_pManager->AddHighlight( m_UserID, r, m_Flags & WF_HIGHLIGHT );
            }

            m_pManager->RenderElement( m_iElementTab, r, State );

            irect   tr = r;
            tr.Translate( 1, -1 );
            m_pManager->RenderText( m_Font, tr, ui_font::h_center|ui_font::v_center, TextColor2, m_Tabs[i].Label );
            tr.Translate( -1, -1 );
            m_pManager->RenderText( m_Font, tr, ui_font::h_center|ui_font::v_center, TextColor1, m_Tabs[i].Label );

            r.Translate( r.GetWidth(), 0 );
        }

        // Render children
        for( i=0 ; i<m_Children.GetCount() ; i++ )
        {
            m_Children[i]->Render( m_Position.l+ox, m_Position.t+oy );
        }
    }
}

//=========================================================================
//=========================================================================
//  Message Handler Functions
//=========================================================================
//=========================================================================

void ui_tabbed_dialog::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    (void)pWin;
    (void)Presses;
    (void)Repeats;
    (void)WrapX;
    (void)WrapY;

//    ui_manager::user*   pUser   = m_pManager->GetUser( m_UserID );
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

    // Check for left, right switching Tabs
    if( m_Tabs.GetCount() > 0 )
    {
        s32 iTab = m_iActiveTab + dx;
        if( iTab <                  0 ) iTab = m_Tabs.GetCount()-1;
        if( iTab >= m_Tabs.GetCount() ) iTab = 0;
        ActivateTab( iTab );
//        audio_Play( SFX_FRONTEND_CURSOR_MOVE_01,AUDFLAG_CHANNELSAVER );	//-- Jhowa
    }

    // If moving down then jump to the first available control in the dialog for the active tab
    if( dy == 1 )
    {
        if( m_iActiveTab != -1 )
        {
            for( s32 i=0 ; i<m_Tabs[m_iActiveTab].pDialog->GetNumControls() ; i++ )
            {
                if( m_Tabs[m_iActiveTab].pDialog->GotoControl( i ) )
                    break;
            }
        }
    }
}

//=========================================================================

void ui_tabbed_dialog::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    // Activate the dialog
    if( m_iActiveTab != -1 )
        m_Tabs[m_iActiveTab].pDialog->GotoControl( (s32)0 );
}

//=========================================================================

void ui_tabbed_dialog::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    m_pManager->EndDialog( m_UserID, TRUE );
//    audio_Play( SFX_FRONTEND_SELECT_01_CLOSE,AUDFLAG_CHANNELSAVER );	//-- Jhowa
}

//=========================================================================

void ui_tabbed_dialog::SetBackgroundColor( xcolor Color )
{
    m_BackgroundColor = Color;
}

//=========================================================================

xcolor ui_tabbed_dialog::GetBackgroundColor( void ) const
{
    return m_BackgroundColor;
}

//=========================================================================

s32 ui_tabbed_dialog::AddTab( const xwstring& Label, ui_dialog* pDialog )
{
    ASSERT( pDialog );

    // Get size of label
    irect r;
    m_pManager->TextSize( m_Font, r, (const xwchar*)Label, -1 );

    // Create a new tab
    tab& Tab = m_Tabs.Append();
    Tab.Label = Label;
    Tab.pDialog = pDialog;
    Tab.w = r.GetWidth() + 32;

    // Add to children
    pDialog->SetParent( this );

    // Set Dialog to be invisible
    pDialog->SetFlags( pDialog->GetFlags() & ~WF_VISIBLE );

    // Return Index of Tab
    return m_Children.GetCount()-1;
}

//=========================================================================

void ui_tabbed_dialog::FitTabs( void )
{
    s32 i;
    s32 w = 0;

    for( i=0 ; i<m_Tabs.GetCount() ; i++ )
    {
        w += m_Tabs[i].w;
    }

    s32 Overage = w - (GetWidth() - 32);
    Overage /= m_Tabs.GetCount();

    if( Overage > 0 )
    {
        for( i=0 ; i<m_Tabs.GetCount() ; i++ )
        {
            m_Tabs[i].w -= Overage;
        }
    }
}

//=========================================================================

void ui_tabbed_dialog::ActivateTab( s32 iTab )
{
    s32 OldActiveTab = m_iActiveTab;

    if( iTab >= m_Tabs.GetCount() ) iTab = 0;

/*
    // When only 1 tab goto the first available control on that page
    if( m_Tabs.GetCount() == 1 )
    {
        for( s32 i=0 ; i<m_Tabs[iTab].pDialog->GetNumControls() ; i++ )
        {
            if( m_Tabs[iTab].pDialog->GotoControl( i ) )
            {
                ui_dialog* pDialog = m_Tabs[iTab].pDialog;
                pDialog->SetFlags( pDialog->GetFlags() | WF_VISIBLE );
                return;
            }
        }
    }
*/

    // Hide previous Tab
    if( m_iActiveTab != -1 )
    {
        ui_dialog* pDialog = m_Tabs[m_iActiveTab].pDialog;
        pDialog->SetFlags( pDialog->GetFlags() & ~WF_VISIBLE );
    }

    // Activate new Tab
    m_iActiveTab = iTab;
    if( m_iActiveTab != -1 )
    {
        ASSERT( (m_iActiveTab >= 0) && (m_iActiveTab < m_Tabs.GetCount()) );
        ui_dialog* pDialog = m_Tabs[m_iActiveTab].pDialog;
        pDialog->SetFlags( pDialog->GetFlags() | WF_VISIBLE );
    }

    // Track the active tab
    if( m_pTabTracker )
        *m_pTabTracker = iTab;

    // Position cursor at center of Tab
    s32 x = 0;
    s32 y = 16;
    for( s32 i=0 ; i<m_iActiveTab ; i++ )
    {
        if( m_TabWidth == -1 )
            x += m_Tabs[i].w;
        else
            x += m_TabWidth;
    }
    if( m_TabWidth == -1 )
        x += m_Tabs[m_iActiveTab].w / 2;
    else
        x += m_TabWidth / 2;

    LocalToScreen( x, y );
    m_pManager->SetCursorPos( m_UserID, x, y );

    // Send notification on change of tab
    if( m_iActiveTab != OldActiveTab )
    {
        if( m_pParent )
            m_pParent->OnNotify( m_pParent, this, WN_TAB_CHANGE, (void*)m_iActiveTab );
    }
}

//=========================================================================

void ui_tabbed_dialog::ActivateTab( ui_dialog* pDialog )
{
    // Find Tab for this dialog
    for( s32 i=0 ; i<m_Tabs.GetCount() ; i++ )
    {
        if( m_Tabs[i].pDialog == pDialog )
        {
            ActivateTab( i );
            return;
        }
    }
}

//=========================================================================

s32 ui_tabbed_dialog::GetActiveTab( void ) const
{
    return m_iActiveTab;
}

//=========================================================================

void ui_tabbed_dialog::SetTabWidth( s32 TabWidth )
{
    m_TabWidth = TabWidth;
}

//=========================================================================

s32 ui_tabbed_dialog::GetTabWidth( void ) const
{
    return m_TabWidth;
}

const xwstring& ui_tabbed_dialog::GetTabLabel( s32 iTab ) const
{
    ASSERT( (iTab >= 0) && (iTab < m_Tabs.GetCount()) );

    return m_Tabs[iTab].Label;
}

//=========================================================================

void ui_tabbed_dialog::OnLBDown ( ui_win* pWin )
{
    (void)pWin;

#ifdef TARGET_PC
    s32 x, y;
    s32 Width = 0;

    // Go through all the tabs.
    for( s32 i = 0; i < m_Tabs.GetCount(); i++ )
    {
        x = m_CursorX;
        y = m_CursorY;
        s32 w;

        // If the tab width is -1 then the width of the tab is stored inside m_Tabs.
        if( m_TabWidth  == -1 )
        {   
            w = m_Tabs[i].w;
        }
        else
        {
            w = m_TabWidth;
        }

        m_Tabs[i].pDialog->ScreenToLocal( x, y );
        
        // Accumulate the width from the previous tab.
        Width += w;
        x -= Width;
        if( x < 0 )
        {
            ActivateTab( i );
//            audio_Play( SFX_FRONTEND_CURSOR_MOVE_01,AUDFLAG_CHANNELSAVER );	//-- Jhowa
            return;
        }
    
    }
#endif
}

//=========================================================================

void ui_tabbed_dialog::OnCursorMove ( ui_win* pWin, s32 x, s32 y )
{
    (void)pWin;
    (void)x;
    (void)y;

#ifdef TARGET_PC
    m_CursorX = x;
    m_CursorY = y;    
#endif
}

//=========================================================================

void ui_tabbed_dialog::SetTabTracking( s32* pTabTracker )
{
    m_pTabTracker = pTabTracker;
}

//=========================================================================
