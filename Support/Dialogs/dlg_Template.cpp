//=========================================================================
//
//  dlg_main_menu.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"

#include "frontend\FrontEnd_Mgr.hpp"
#include "stringmgr\stringmgr.hpp"

#include "dlg_Template.hpp"

//=========================================================================
//  Template Dialog
//=========================================================================

enum controls
{
	IDC_MAIN_MENU_ONE,
	IDC_MAIN_MENU_TWO,
	IDC_MAIN_MENU_THREE,
};


ui_manager::control_tem MainMenuControls[] = 
{
    // Frames.
    { IDC_MAIN_MENU_ONE,    "IDS_MAIN_MENU_SINGLE",     "button",   60, 120, 120, 40, 0, 0, 1, 1, ui_win::WF_VISIBLE },
    { IDC_MAIN_MENU_TWO,    "IDS_MAIN_MENU_MULTI",      "button",   60, 180, 120, 40, 0, 1, 1, 1, ui_win::WF_VISIBLE },
	{ IDC_MAIN_MENU_THREE,  "IDS_MAIN_MENU_OPTIONS",    "button",   60, 240, 120, 40, 0, 2, 1, 1, ui_win::WF_VISIBLE },
};


ui_manager::dialog_tem MainMenuDialog =
{
    "IDS_MAIN_MENU",
    1, 9,
    sizeof(MainMenuControls)/sizeof(ui_manager::control_tem),
    &MainMenuControls[0],
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

void dlg_main_menu_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "main menu", &MainMenuDialog, &dlg_main_menu_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_main_menu_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_main_menu* pDialog = new dlg_main_menu;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_main_menu
//=========================================================================

dlg_main_menu::dlg_main_menu( void )
{
}

//=========================================================================

dlg_main_menu::~dlg_main_menu( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_main_menu::Create( s32                        UserID,
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

    m_pButtonSinglePlayer	= (ui_button*)  FindChildByID( IDC_MAIN_MENU_SINGLE );
    m_pButtonMultiPlayer	= (ui_button*)  FindChildByID( IDC_MAIN_MENU_MULTI );
    m_pButtonOptions 	    = (ui_button*)  FindChildByID( IDC_MAIN_MENU_OPTIONS );

    GotoControl( (ui_control*)m_pButtonSinglePlayer );

    m_CurrHL = 0;

    // switch off the buttons to start
    m_pButtonSinglePlayer->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonMultiPlayer->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonOptions->SetFlag(ui_win::WF_VISIBLE, FALSE);    

    // set the number of players to 0
    g_FrontEndMgr.SetNumPlayers(0);

    // store requested frame size
    m_RequestedPos = Position;

    // set starting position
    m_CurrPos.Set(192, 192, 192+128, 192+64);
    
    // set up scaling
    m_scaleCount = 10;
    m_scaleX = (Position.l - m_CurrPos.l) / m_scaleCount-1;
    m_scaleY = (Position.t - m_CurrPos.t) / m_scaleCount-1;

    // set starting position
    SetPosition(m_CurrPos);

    //g_AudioMgr.SetSFXVolume( g_FrontEndMgr.GetSFXVol() / 100.0f);
    //g_AudioMgr.SetMusicVolume( g_FrontEndMgr.GetMainVol() / 100.0f);
    //g_AudioMgr.SetVoiceVolume( g_FrontEndMgr.GetVoiceVol() / 100.0f);

	// Return success code
    return Success;
}

//=========================================================================

void dlg_main_menu::Destroy( void )
{
    ui_dialog::Destroy();
}

//=========================================================================

void dlg_main_menu::Render( s32 ox, s32 oy )
{
	irect rb;

    // render transparent screen
	//rb.Set( 44, 16, 468, 432 );

    rb.l = m_CurrPos.l + 28;
    rb.t = m_CurrPos.t;
    rb.r = m_CurrPos.r - 28;
    rb.b = m_CurrPos.b;

    g_UiMgr->RenderGouraudRect(rb, xcolor(56,115,58,180),
                                   xcolor(56,115,58,180),
                                   xcolor(56,115,58,180),
                                   xcolor(56,115,58,180),FALSE);

    // render the normal dialog stuff
    ui_dialog::Render( ox, oy );
}

//=========================================================================

void dlg_main_menu::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if( pWin == (ui_win*)m_pButtonSinglePlayer )
    {
        //g_AudioMgr.Play("OptionSelect");
        g_FrontEndMgr.SetNumPlayers(1);
		g_FrontEndMgr.SetFrontEndState(FE_GOTO_LEVEL_SELECT);
	}
	else if( pWin == (ui_win*)m_pButtonMultiPlayer )
	{
        //g_AudioMgr.Play("OptionSelect");
        g_FrontEndMgr.SetNumPlayers(2);
		g_FrontEndMgr.SetFrontEndState(FE_GOTO_LEVEL_SELECT);
	}
	else if( pWin == (ui_win*)m_pButtonOptions )
	{
        //g_AudioMgr.Play("OptionSelect");
		//g_FrontEndMgr.SetFrontEndState( FE_GOTO_FE_OPTIONS );
	}
}

//=========================================================================

void dlg_main_menu::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    OWNER("dlg_main_menu::OnUpdate");
    (void)pWin;
    (void)DeltaTime;

    s32 highLight = -1;

    // scale window if necessary
    if (m_scaleCount)
    {
        if (--m_scaleCount == 0)
        {
            // last one - make sure window is correct size
            m_CurrPos = m_RequestedPos;

            // turn on the buttons
            m_pButtonSinglePlayer->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonMultiPlayer->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonOptions->SetFlag(ui_win::WF_VISIBLE, TRUE);    
    
            GotoControl( (ui_control*)m_pButtonSinglePlayer );
            m_pButtonSinglePlayer->SetFlag(WF_HIGHLIGHT, TRUE);        
        }
        else
        {
            m_CurrPos.l += m_scaleX;
            m_CurrPos.t += m_scaleY;
            m_CurrPos.r -= m_scaleX;
            m_CurrPos.b -= m_scaleY;   
        }

        // resize the window
        SetPosition(m_CurrPos);        
    }


    if( m_pButtonSinglePlayer->GetFlags(WF_HIGHLIGHT) )
        highLight = 0;
    else if( m_pButtonMultiPlayer->GetFlags(WF_HIGHLIGHT) )
        highLight = 1;
    else if( m_pButtonOptions->GetFlags(WF_HIGHLIGHT) )
        highLight = 2;

    if( highLight != m_CurrHL )
    {
        //g_AudioMgr.Play("CursorMove");
        m_CurrHL = highLight;
    }
}

//=========================================================================
