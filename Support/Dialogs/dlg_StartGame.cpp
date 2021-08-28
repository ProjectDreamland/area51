//=========================================================================
//
//  dlg_start_game.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"

#include "dlg_StartGame.hpp"
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"

//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum controls
{
    IDC_START_GAME_TEXT,
};


ui_manager::control_tem StartGameControls[] = 
{
    { IDC_START_GAME_TEXT,	    "IDS_LOADING_MSG",          "text",     246, 300, 120, 40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem StartGameDialog =
{
    "IDS_NULL",
    1, 1,
    sizeof(StartGameControls)/sizeof(ui_manager::control_tem),
    &StartGameControls[0],
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

void dlg_start_game_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "start game", &StartGameDialog, &dlg_start_game_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_start_game_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_start_game* pDialog = new dlg_start_game;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_start_game
//=========================================================================

dlg_start_game::dlg_start_game( void )
{
}

//=========================================================================

dlg_start_game::~dlg_start_game( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_start_game::Create( s32                        UserID,
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

    m_pLoadText = (ui_text*) FindChildByID( IDC_START_GAME_TEXT );
    m_pLoadText->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLoadText->SetLabelColor( xcolor(126,220,60,255) );

    // turn off the screen
    g_UiMgr->SetScreenOn(FALSE);
    SetFlag( WF_DISABLED, TRUE );

    // initialize screen scaling
    InitScreenScaling( Position );

    // initialize loading flag
    m_StartLoading = FALSE;

    // disable highlight
    g_UiMgr->DisableScreenHighlight();

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_start_game::Destroy( void )
{
    ui_dialog::Destroy();
}

//=========================================================================

void dlg_start_game::Render( s32 ox, s32 oy )
{
    // render background filter
    if (!m_StartLoading)
    {
	    irect rb;

	    s32 XRes, YRes;
        eng_GetRes(XRes, YRes);
#ifdef TARGET_PS2
        // Nasty hack to force PS2 to draw to rb.l = 0
        rb.Set( -1, 0, XRes, YRes );
#else
        rb.Set( 0, 0, XRes, YRes );
#endif
        g_UiMgr->RenderGouraudRect(rb, xcolor(0,0,0,180),
                                       xcolor(0,0,0,180),
                                       xcolor(0,0,0,180),
                                       xcolor(0,0,0,180),FALSE);
    }
    
    // render the normal dialog stuff
    ui_dialog::Render( ox, oy );
}

//=========================================================================

void dlg_start_game::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // update countdown
    //if (m_Countdown)
    //{
    //    if (--m_Countdown == 0)
    //    {
    //        // trigger the load!
    //        m_State = DIALOG_STATE_SELECT;
    //    }
    //}

    // scale window if necessary
    if (g_UiMgr->IsScreenScaling())
    {
        if ( UpdateScreenScaling( DeltaTime, FALSE ) == FALSE )
        {
            // change the background to the loading screen
            //g_UiMgr->SetUserBackground( g_UiUserID, g_UiMgr->FindBackground("loadscreen") );

            // show loading text
            //m_pLoadText->SetFlag(ui_win::WF_VISIBLE, TRUE);

            // set flag to start loading screen
            //m_StartLoading = TRUE;
            //m_Countdown = 30;

            m_State = DIALOG_STATE_SELECT;
        }
    }
}

//=========================================================================
