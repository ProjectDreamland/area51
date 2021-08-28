//=========================================================================
//
//  dlg_demo_main_menu.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"

#include "dlg_DemoMainMenu.hpp"
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "StateMgr/mapList.hpp"
#include "Configuration/GameConfig.hpp"

//=========================================================================
//  Main Menu Dialog
//=========================================================================

ui_manager::control_tem DemoMainMenuControls[] = 
{
    { IDC_DEMO_MAIN_MENU_LEVEL_ONE,     "IDS_NULL",     "button",   60, 100, 120, 40, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_DEMO_MAIN_MENU_LEVEL_TWO,	    "IDS_NULL",     "button",   60, 160, 120, 40, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_DEMO_MAIN_MENU_LEVEL_THREE,	"IDS_NULL",     "button",   60, 220, 120, 40, 0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_DEMO_MAIN_MENU_NAV_TEXT,      "IDS_NULL",     "text",      0,   0,   0,  0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};

ui_manager::dialog_tem DemoMainMenuDialog =
{
    "IDS_MAIN_MENU",
    1, 9,
    sizeof(DemoMainMenuControls)/sizeof(ui_manager::control_tem),
    &DemoMainMenuControls[0],
    0
};

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
//  Registration function
//=========================================================================

void dlg_demo_main_menu_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "demo menu", &DemoMainMenuDialog, &dlg_demo_main_menu_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_demo_main_menu_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_demo_main_menu* pDialog = new dlg_demo_main_menu;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_demo_main_menu
//=========================================================================

dlg_demo_main_menu::dlg_demo_main_menu( void )
{
}

//=========================================================================

dlg_demo_main_menu::~dlg_demo_main_menu( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_demo_main_menu::Create( s32                        UserID,
                             ui_manager*                pManager,
                             ui_manager::dialog_tem*    pDialogTem,
                             const irect&               Position,
                             ui_win*                    pParent,
                             s32                        Flags,
                             void*                      pUserData )
{
    xbool   Success = FALSE;

    (void)pUserData;

    ASSERT( pManager );

    // Do dialog creation
    Success = ui_dialog::Create( UserID, pManager, pDialogTem, Position, pParent, Flags );

    m_pButtonLevelOne   = (ui_button*)  FindChildByID( IDC_DEMO_MAIN_MENU_LEVEL_ONE   );
    m_pButtonLevelTwo	= (ui_button*)  FindChildByID( IDC_DEMO_MAIN_MENU_LEVEL_TWO   );
    m_pButtonLevelThree = (ui_button*)  FindChildByID( IDC_DEMO_MAIN_MENU_LEVEL_THREE );
    m_pNavText          = (ui_text*)    FindChildByID( IDC_DEMO_MAIN_MENU_NAV_TEXT    );

    GotoControl( (ui_control*)m_pButtonLevelOne );
    m_CurrentControl = 	IDC_DEMO_MAIN_MENU_LEVEL_ONE;

    m_CurrHL = 0;

    // switch off the buttons to start
    m_pButtonLevelOne   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonLevelTwo   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonLevelThree ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pNavText          ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // set labels
    m_pButtonLevelOne   ->SetLabel( xwstring("Dr. Cray") );
    m_pButtonLevelTwo   ->SetLabel( xwstring("Lies of the Past") );
    m_pButtonLevelThree ->SetLabel( xwstring("The Grays") );

    // set up nav text 
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));

    m_pNavText->SetLabel( xwstring(navText) );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // set the number of players to 0
    g_PendingConfig.SetPlayerCount( 0 );

    // initialize the screen scaling
    InitScreenScaling( Position );

    // set the frame to be disabled (if coming from off screen)
    if (g_UiMgr->IsScreenOn() == FALSE)
        SetFlag( WF_DISABLED, TRUE );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

	// Return success code
    return Success;
}

//=========================================================================

void dlg_demo_main_menu::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_demo_main_menu::Render( s32 ox, s32 oy )
{
    static s32 offset   =  0;
    static s32 gap      =  9;
    static s32 width    =  4;

    irect rb;
    
    // render the screen (if we're correctly sized)
    if (g_UiMgr->IsScreenOn())
    {
        // render transparent screen
        rb.l = m_CurrPos.l + 22;
        rb.t = m_CurrPos.t;
        rb.r = m_CurrPos.r - 23;
        rb.b = m_CurrPos.b;

        g_UiMgr->RenderGouraudRect(rb, xcolor(56,115,58,64),
                                       xcolor(56,115,58,64),
                                       xcolor(56,115,58,64),
                                       xcolor(56,115,58,64),FALSE);


        // render the screen bars
        s32 y = rb.t + offset;    

        while (y < rb.b)
        {
            irect bar;

            if ((y+width) > rb.b)
            {
                bar.Set(rb.l, y, rb.r, rb.b);
            }
            else
            {
                bar.Set(rb.l, y, rb.r, y+width);
            }

            // draw the bar
            g_UiMgr->RenderGouraudRect(bar, xcolor(56,115,58,30),
                                            xcolor(56,115,58,30),
                                            xcolor(56,115,58,30),
                                            xcolor(56,115,58,30),FALSE);

            y+=gap;
        }
    
        // increment the offset
        if (++offset > 9)
            offset = 0;
    }
  
    // render the normal dialog stuff
    ui_dialog::Render( ox, oy );

    // render the glow bar
    g_UiMgr->RenderGlowBar();

}

//=========================================================================

void dlg_demo_main_menu::OnPadSelect( ui_win* pWin )
{
    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( pWin == (ui_win*)m_pButtonLevelOne )
        {
            // Dr Cray
            g_StateMgr.SetLevelIndex( 9 );
            g_PendingConfig.SetLevelID( 1080 );
	    }
	    else if( pWin == (ui_win*)m_pButtonLevelTwo )
	    {
            // Lies of the past
            g_StateMgr.SetLevelIndex( 12 );
            g_PendingConfig.SetLevelID( 1100 );
	    }
        else if( pWin == (ui_win*)m_pButtonLevelThree )
        {
            // The Greys
            g_StateMgr.SetLevelIndex( 15 );
            g_PendingConfig.SetLevelID( 1115 );
        }

        g_AudioMgr.Play("SelectNorm");
        m_State = DIALOG_STATE_SELECT;
    }
}

//=========================================================================

void dlg_demo_main_menu::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    s32 highLight = -1;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // turn on the buttons
            m_pButtonLevelOne       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonLevelTwo      ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonLevelThree ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText               ->SetFlag(ui_win::WF_VISIBLE, TRUE);
    
            GotoControl( (ui_control*)m_pButtonLevelOne );
            m_pButtonLevelOne->SetFlag(WF_HIGHLIGHT, TRUE);        
            g_UiMgr->SetScreenHighlight( m_pButtonLevelOne->GetPosition() );

            if (g_UiMgr->IsScreenOn() == FALSE)
            {
                // enable the frame
                SetFlag( WF_DISABLED, FALSE );
                g_UiMgr->SetScreenOn(TRUE);
            }
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    if( m_pButtonLevelOne->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pButtonLevelOne->GetPosition() );
        highLight = 0;
    }
    else if( m_pButtonLevelTwo->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pButtonLevelTwo->GetPosition() );
        highLight = 1;
    }
    else if( m_pButtonLevelThree->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pButtonLevelThree->GetPosition() );
        highLight = 2;
    }

    if( highLight != m_CurrHL )
    {
        //g_AudioMgr.Play("CursorMove");
        m_CurrHL = highLight;
    }
}

//=========================================================================
