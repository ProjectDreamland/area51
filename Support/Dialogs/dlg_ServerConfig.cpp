//=========================================================================
//
//  dlg_ServerConfig.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"

#include "dlg_ServerConfig.hpp"
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "NetworkMgr\GameMgr.hpp"

#ifndef CONFIG_RETAIL
#include "InputMgr\monkey.hpp"
#endif

//=========================================================================
//  Main Menu Dialog
//=========================================================================

ui_manager::control_tem ServerConfigControls[] = 
{
    // Frames.
    { IDC_SERVER_CONFIG_CHANGE_MAP,	    "IDS_CONFIG_CHANGE_MAP",            "button",  110,  40, 120, 40, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_SERVER_CONFIG_KICK_PLAYER,	"IDS_CONFIG_KICK_PLAYER",           "button",  110,  75, 120, 40, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_SERVER_CONFIG_CHANGE_TEAM,	"IDS_CONFIG_TEAM_CHANGE",           "button",  110, 110, 120, 40, 0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_SERVER_CONFIG_RESTART_MAP,	"IDS_CONFIG_RESTART_MAP",           "button",  110, 145, 120, 40, 0, 3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_SERVER_CONFIG_RECONFIGURE,	"IDS_CONFIG_RECONFIGURE_SERVER",    "button",  110, 180, 120, 40, 0, 4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_SERVER_CONFIG_SHUTDOWN,	    "IDS_CONFIG_SHUTDOWN_SERVER",       "button",  110, 215, 120, 40, 0, 5, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
	{ IDC_SERVER_CONFIG_NAV_TEXT,       "IDS_NULL",                         "text",      0,   0,   0,  0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem ServerConfigDialog =
{
    "IDS_SERVER_CONFIG_MENU",
    1, 9,
    sizeof(ServerConfigControls)/sizeof(ui_manager::control_tem),
    &ServerConfigControls[0],
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

void dlg_server_config_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "server config", &ServerConfigDialog, &dlg_server_config_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_server_config_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_server_config* pDialog = new dlg_server_config;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_server_config
//=========================================================================

dlg_server_config::dlg_server_config( void )
{
}

//=========================================================================

dlg_server_config::~dlg_server_config( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_server_config::Create( s32                        UserID,
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

    // get button handles
	m_pButtonChangeMap      = (ui_button*)  FindChildByID( IDC_SERVER_CONFIG_CHANGE_MAP  );
	m_pButtonKickPlayer     = (ui_button*)  FindChildByID( IDC_SERVER_CONFIG_KICK_PLAYER ); 	
    m_pButtonChangeTeam     = (ui_button*)  FindChildByID( IDC_SERVER_CONFIG_CHANGE_TEAM ); 	
    m_pButtonRestartMap     = (ui_button*)  FindChildByID( IDC_SERVER_CONFIG_RESTART_MAP ); 		    
    m_pButtonReconfigure    = (ui_button*)  FindChildByID( IDC_SERVER_CONFIG_RECONFIGURE ); 		
	m_pButtonShutdown       = (ui_button*)  FindChildByID( IDC_SERVER_CONFIG_SHUTDOWN    ); 	
    m_pNavText 	            = (ui_text*)    FindChildByID( IDC_SERVER_CONFIG_NAV_TEXT    );

    s32 iControl = g_StateMgr.GetCurrentControl();
    if( (iControl == -1) || (GotoControl(iControl)==NULL) )
    {
        GotoControl( (ui_control*)m_pButtonChangeMap );
        m_CurrentControl = IDC_SERVER_CONFIG_CHANGE_MAP;
    }
    else
    {
        m_CurrentControl = iControl;
    }

    m_CurrHL = 0;
    m_PopUp = NULL;

    // switch off the buttons to start
    m_pButtonChangeMap      ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonKickPlayer     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonChangeTeam     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonRestartMap     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonReconfigure    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonShutdown       ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pNavText              ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );   
    m_pNavText->SetLabel( navText );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // check for team based game
    const game_score& ScoreData = GameMgr.GetScore();
    if( !ScoreData.IsTeamBased )
    {
        m_pButtonChangeTeam->SetFlag(ui_win::WF_DISABLED, TRUE);
    }

#ifdef LAN_PARTY_BUILD
    m_pButtonKickPlayer ->SetFlag(ui_win::WF_DISABLED, TRUE);
    m_pButtonChangeTeam ->SetFlag(ui_win::WF_DISABLED, TRUE);
    m_pButtonRestartMap ->SetFlag(ui_win::WF_DISABLED, TRUE);
    m_pButtonReconfigure->SetFlag(ui_win::WF_DISABLED, TRUE);
    m_pButtonShutdown   ->SetFlag(ui_win::WF_DISABLED, TRUE);
#endif

    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

	// Return success code
    return Success;
}

//=========================================================================

void dlg_server_config::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_server_config::Render( s32 ox, s32 oy )
{
    static s32 offset   =  0;
    static s32 gap      =  9;
    static s32 width    =  4;

	irect rb;


    // render background filter
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

void dlg_server_config::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( pWin == (ui_win*)m_pButtonKickPlayer )
	    {
            // kick player
            m_CurrentControl = IDC_SERVER_CONFIG_KICK_PLAYER;
            m_State = DIALOG_STATE_SELECT;
            g_AudioMgr.Play( "Select_Norm" );
        }
        else if( pWin == (ui_win*)m_pButtonChangeMap )
        {
            // change map
            m_CurrentControl = IDC_SERVER_CONFIG_CHANGE_MAP;
            m_State = DIALOG_STATE_SELECT;
            g_AudioMgr.Play( "Select_Norm" );
	    }
        else if( pWin == (ui_win*)m_pButtonChangeTeam )
        {
            // change map
            m_CurrentControl = IDC_SERVER_CONFIG_CHANGE_TEAM;
            m_State = DIALOG_STATE_SELECT;
            g_AudioMgr.Play( "Select_Norm" );
        }
        else if( pWin == (ui_win*)m_pButtonRestartMap )
        {

#ifndef CONFIG_RETAIL
            if ( g_MonkeyOptions.Enabled && g_MonkeyOptions.ModeEnabled[MONKEY_MENUMONKEY] )
                return;
#endif

            // restart map
            m_CurrentControl = IDC_SERVER_CONFIG_RESTART_MAP;

            // open confirmation dialog
            irect r = g_UiMgr->GetUserBounds( g_UiUserID );
            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

            // set nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);


            m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_GAMEMGR_POPUP" ), 
                TRUE, 
                TRUE, 
                FALSE, 
                g_StringTableMgr( "ui", "IDS_ONLINE_RESTART_MAP_MSG" ),
                navText,
                &m_PopUpResult );

        }
        else if( pWin == (ui_win*)m_pButtonReconfigure )
        {
            // change map
            m_CurrentControl = IDC_SERVER_CONFIG_RECONFIGURE;
            m_State = DIALOG_STATE_SELECT;
            g_AudioMgr.Play( "Select_Norm" );
        }
        else if( pWin == (ui_win*)m_pButtonShutdown )
	    {

#ifndef CONFIG_RETAIL
            if ( g_MonkeyOptions.Enabled && g_MonkeyOptions.ModeEnabled[MONKEY_MENUMONKEY] )
                return;
#endif

            // Shutdown the server
            m_CurrentControl = IDC_SERVER_CONFIG_SHUTDOWN;
      
            // open confirmation dialog
            irect r = g_UiMgr->GetUserBounds( g_UiUserID );
            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

            // set nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);
            

            m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_GAMEMGR_POPUP" ), 
                                TRUE, 
                                TRUE, 
                                FALSE, 
                                g_StringTableMgr( "ui", "IDS_ONLINE_SHUTDOWN_SERVER_MSG" ),
                                navText,
                                &m_PopUpResult );
        
        }
    }
}

//=========================================================================

void dlg_server_config::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        m_State = DIALOG_STATE_BACK;
        g_AudioMgr.Play( "Backup" );
    }
}

//=========================================================================

void dlg_server_config::OnUpdate ( ui_win* pWin, f32 DeltaTime )
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
            m_pButtonChangeMap      ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonKickPlayer     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonChangeTeam     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonRestartMap     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonReconfigure    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonShutdown       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText              ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            s32 iControl = g_StateMgr.GetCurrentControl();
            if( (iControl == -1) || (GotoControl(iControl)==NULL) )
            {
                GotoControl( (ui_control*)m_pButtonChangeMap );
                m_pButtonChangeMap->SetFlag(WF_HIGHLIGHT, TRUE);        
                g_UiMgr->SetScreenHighlight( m_pButtonChangeMap->GetPosition() );
                m_CurrentControl =  IDC_SERVER_CONFIG_CHANGE_MAP;
            }
            else
            {
                ui_control* pControl = GotoControl( iControl );
                ASSERT( pControl );
                pControl->SetFlag(WF_HIGHLIGHT, TRUE);
                g_UiMgr->SetScreenHighlight(pControl->GetPosition() );
                m_CurrentControl = iControl;
            }

            if (g_UiMgr->IsScreenOn() == FALSE)
            {
                // enable the frame
                SetFlag( WF_DISABLED, FALSE );
                g_UiMgr->SetScreenOn(TRUE);
            }
        }
    }

#ifndef LAN_PARTY_BUILD
    // ensure that there are still players to kick
    {
        const game_score& ScoreData = GameMgr.GetScore();

        // find out my player slot
        s32 NetSlot = g_NetworkMgr.GetLocalPlayerSlot(0);
        s32 Count  = 0;

        // count the players
        for( s32 i=0 ; i<32 ; i++ )
        {
            if( (ScoreData.Player[i].IsInGame) && (i != NetSlot) )
            {
                Count++;
            }
        }

        // check count
        if( Count == 0 )
        {
            // nobody to kick!
            if( m_pButtonKickPlayer->GetFlags(WF_HIGHLIGHT) )
            {
                OnPadNavigate( this, ui_manager::NAV_UP, 1, 0 );
            }
            m_pButtonKickPlayer->SetFlag( WF_DISABLED, TRUE );
        }
        else
        {
            m_pButtonKickPlayer->SetFlag( WF_DISABLED, FALSE );
        }
    }
#endif

    // check for pop up result
    if ( m_PopUp )
    {
        if ( m_PopUpResult != DLG_POPUP_IDLE )
        {
            switch( m_CurrentControl )
            {
                case IDC_SERVER_CONFIG_SHUTDOWN:
                    if ( m_PopUpResult == DLG_POPUP_YES )
                    {
                        // shutdown the server here
                        m_State = DIALOG_STATE_SELECT;
                    }
                    break;

                case IDC_SERVER_CONFIG_RESTART_MAP:
                    if( m_PopUpResult == DLG_POPUP_YES )
                    {
                        m_State = DIALOG_STATE_SELECT;
                        // restart the current map
                    }
            }

            // clear popup 
            m_PopUp = NULL;

            // turn on nav text
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    if( m_pButtonChangeMap->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 0;
        g_UiMgr->SetScreenHighlight( m_pButtonChangeMap->GetPosition() );
    }
    else if( m_pButtonKickPlayer->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 1;
        g_UiMgr->SetScreenHighlight( m_pButtonKickPlayer->GetPosition() );
    }
    else if( m_pButtonChangeTeam->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 2;
        g_UiMgr->SetScreenHighlight( m_pButtonChangeTeam->GetPosition() );
    }
    else if( m_pButtonRestartMap->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 3;
        g_UiMgr->SetScreenHighlight( m_pButtonRestartMap->GetPosition() );
    }
    else if( m_pButtonReconfigure->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 4;
        g_UiMgr->SetScreenHighlight( m_pButtonReconfigure->GetPosition() );
    }
    else if( m_pButtonShutdown->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 5;
        g_UiMgr->SetScreenHighlight( m_pButtonShutdown->GetPosition() );
    }

    if( highLight != m_CurrHL )
    {
        if( highLight != -1 )
            g_AudioMgr.Play("Cusor_Norm");

        m_CurrHL = highLight;
    }
}

//=========================================================================
