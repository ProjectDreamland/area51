//=========================================================================
//
//  dlg_end_pause.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"

#include "dlg_EndPause.hpp"
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"

//=========================================================================
//  End Pause Dialog
//=========================================================================

enum controls
{
    IDC_WAIT_TEXT,
};


ui_manager::control_tem EndPauseControls[] = 
{
    { IDC_WAIT_TEXT,	    "IDS_WAIT_TEXT",          "text",     246, 300, 120, 40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem EndPauseDialog =
{
    "IDS_NULL",
    1, 1,
    sizeof(EndPauseControls)/sizeof(ui_manager::control_tem),
    &EndPauseControls[0],
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

void dlg_end_pause_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "end pause", &EndPauseDialog, &dlg_end_pause_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_end_pause_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_end_pause* pDialog = new dlg_end_pause;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_end_pause
//=========================================================================

dlg_end_pause::dlg_end_pause( void )
{
}

//=========================================================================

dlg_end_pause::~dlg_end_pause( void )
{
    // make sure screen scaling is off
    g_UiMgr->SetScreenScaling( FALSE );
    // kill dialog
    Destroy();
}

//=========================================================================

xbool dlg_end_pause::Create( s32                        UserID,
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

    // Get controls
    m_pWaitText = (ui_text*) FindChildByID( IDC_WAIT_TEXT );
    m_pWaitText->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pWaitText->SetLabelColor( xcolor(93,228,223,255) );

    // turn off the screen
    g_UiMgr->SetScreenOn(FALSE);
    SetFlag( WF_DISABLED, TRUE );

    // initialize screen scaling
    InitScreenScaling( Position );

    // initialize wait flag
    m_StartWaiting = FALSE;

    // disable the highlight
    g_UiMgr->DisableScreenHighlight();

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_end_pause::Destroy( void )
{
    ui_dialog::Destroy();
}

//=========================================================================

void dlg_end_pause::Render( s32 ox, s32 oy )
{
    // render background filter
    if (!m_StartWaiting)
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

void dlg_end_pause::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // update countdown
    if (m_Countdown)
    {
        if (--m_Countdown == 0)
        {
            // return to main menu
            m_State = DIALOG_STATE_SELECT;
        }
    }

    // scale window if necessary
    if (g_UiMgr->IsScreenScaling())
    {
        if ( UpdateScreenScaling( DeltaTime, FALSE ) == FALSE )
        {
            if (g_StateMgr.IsExiting())
            {
            #ifndef TARGET_XBOX
                // show wait text
                m_pWaitText->SetFlag(ui_win::WF_VISIBLE, TRUE);
            #endif
                // set flag to start loading screen
                m_StartWaiting = TRUE;
                m_Countdown = 10;
            }
            else
            {    
                // return control to game - kill pause mgr
                m_State = DIALOG_STATE_SELECT;
            }
        }
    }
}

//=========================================================================
