//=========================================================================
//
//  dlg_online_players.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_blankbox.hpp"
#include "ui\ui_playerlist.hpp"
#include "ui\ui_dlg_vkeyboard.hpp"

#include "dlg_OnlinePlayers.hpp"
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"

#ifdef CONFIG_VIEWER
#include "../../Apps/ArtistViewer/Config.hpp"
#else
#include "../../Apps/GameApp/Config.hpp"	
#endif


//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum controls
{
    IDC_SERVER_DETAILS,

    IDC_GAMETYPE_TEXT,
    IDC_CURRENTMAP_TEXT,
    IDC_NEXTMAP_TEXT,
    IDC_FRIENDLYFIRE_TEXT,
    IDC_PCTCOMPLETE_TEXT,
    IDC_CONNECTION_TEXT,

    IDC_INFO_GAMETYPE,
    IDC_INFO_CURRENTMAP,
    IDC_INFO_NEXTMAP,
    IDC_INFO_FRIENDLYFIRE,
    IDC_INFO_PCTCOMPLETE,
    IDC_INFO_CONNECTION,
   
    IDC_PLAYERLIST,
    IDC_PLAYERS_NAV_TEXT,
};

 
ui_manager::control_tem OnlinePlayersControls[] = 
{
    // Frames.
    { IDC_SERVER_DETAILS,   "IDS_NULL",             "blankbox",    48,  40, 416,  80, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_FRIENDLYFIRE_TEXT,"IDS_JOIN_FRIENDLYFIRE","text",        56,  62, 100,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PCTCOMPLETE_TEXT, "IDS_JOIN_PCTCOMPLETE", "text",        56,  78, 100,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_CONNECTION_TEXT,  "IDS_JOIN_CONNECTION",  "text",        56,  94, 100,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_GAMETYPE_TEXT,    "IDS_JOIN_GAMETYPE",    "text",       221,  62, 100,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_CURRENTMAP_TEXT,  "IDS_JOIN_CURRENTMAP",  "text",       221,  78, 100,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_NEXTMAP_TEXT,     "IDS_JOIN_NEXTMAP",     "text",       221,  94, 100,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_INFO_FRIENDLYFIRE,"IDS_NULL",             "text",       161,  62,  57,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_INFO_PCTCOMPLETE, "IDS_NULL",             "text",       161,  78,  57,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_INFO_CONNECTION,  "IDS_NULL",             "text",       161,  94,  57,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_INFO_GAMETYPE,    "IDS_NULL",             "text",       326,  62, 128,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_INFO_CURRENTMAP,  "IDS_NULL",             "text",       326,  78, 128,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_INFO_NEXTMAP,     "IDS_NULL",             "text",       326,  94, 128,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_PLAYERLIST,       "IDS_NULL",             "playerlist",  48, 130, 416, 180, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_PLAYERS_NAV_TEXT, "IDS_NULL",             "text",         0,   0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem OnlinePlayersDialog =
{
    "IDS_ONLINE_PLAYER_LIST",
    1, 9,
    sizeof(OnlinePlayersControls)/sizeof(ui_manager::control_tem),
    &OnlinePlayersControls[0],
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

void dlg_online_players_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "server players", &OnlinePlayersDialog, &dlg_online_players_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_online_players_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_online_players* pDialog = new dlg_online_players;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_online_players
//=========================================================================

dlg_online_players::dlg_online_players( void )
{
}

//=========================================================================

dlg_online_players::~dlg_online_players( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_online_players::Create( s32                        UserID,
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
    
    m_pPlayerList = (ui_playerlist*)FindChildByID( IDC_PLAYERLIST );
    m_pPlayerList->SetFlag(ui_win::WF_SELECTED, TRUE);
    m_pPlayerList->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pPlayerList->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pPlayerList->EnableHeaderBar();
    m_pPlayerList->SetHeaderBarColor( xcolor(19,59,14,196) );
    m_pPlayerList->SetHeaderColor( xcolor(255,252,204,255) );
    m_pPlayerList->SetExitOnSelect(FALSE);
    m_pPlayerList->SetExitOnBack(TRUE);
    m_pPlayerList->DisableFrame();
    m_pPlayerList->SetMaxPlayerWidth(350);
    GotoControl( (ui_control*)m_pPlayerList );
    m_CurrHL = 0;

    // set up nav text
    m_pNavText = (ui_text*) FindChildByID( IDC_PLAYERS_NAV_TEXT );
    
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_JOIN" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );

    m_pNavText->SetLabel( navText );
    m_pNavText->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);    

    // get server details box
    m_pServerDetails = (ui_blankbox*)FindChildByID( IDC_SERVER_DETAILS );
    m_pServerDetails->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pServerDetails->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pServerDetails->SetHasTitleBar( TRUE );
    m_pServerDetails->SetLabelColor( xcolor(255,252,204,255) );
    m_pServerDetails->SetTitleBarColor( xcolor(19,59,14,196) );

    // set up server info text
    m_pGameType             = (ui_text*)FindChildByID( IDC_GAMETYPE_TEXT     );
    m_pCurrentMap           = (ui_text*)FindChildByID( IDC_CURRENTMAP_TEXT   );
    m_pNextMap              = (ui_text*)FindChildByID( IDC_NEXTMAP_TEXT      );
    m_pFriendlyFire         = (ui_text*)FindChildByID( IDC_FRIENDLYFIRE_TEXT );
    m_pPctComplete          = (ui_text*)FindChildByID( IDC_PCTCOMPLETE_TEXT  );
    m_pConnectionSpeed      = (ui_text*)FindChildByID( IDC_CONNECTION_TEXT   );
    m_pGameTypeInfo         = (ui_text*)FindChildByID( IDC_INFO_GAMETYPE     );
    m_pCurrentMapInfo       = (ui_text*)FindChildByID( IDC_INFO_CURRENTMAP   );
    m_pNextMapInfo          = (ui_text*)FindChildByID( IDC_INFO_NEXTMAP      );
    m_pFriendlyFireInfo     = (ui_text*)FindChildByID( IDC_INFO_FRIENDLYFIRE );
    m_pPctCompleteInfo      = (ui_text*)FindChildByID( IDC_INFO_PCTCOMPLETE  );
    m_pConnectionSpeedInfo  = (ui_text*)FindChildByID( IDC_INFO_CONNECTION   );

    m_pGameType             ->UseSmallText( TRUE );
    m_pCurrentMap           ->UseSmallText( TRUE );
    m_pNextMap              ->UseSmallText( TRUE );
    m_pFriendlyFire         ->UseSmallText( TRUE );          
    m_pPctComplete          ->UseSmallText( TRUE );
    m_pConnectionSpeed      ->UseSmallText( TRUE );
    m_pGameTypeInfo         ->UseSmallText( TRUE );
    m_pCurrentMapInfo       ->UseSmallText( TRUE );
    m_pNextMapInfo          ->UseSmallText( TRUE );
    m_pFriendlyFireInfo     ->UseSmallText( TRUE );
    m_pPctCompleteInfo      ->UseSmallText( TRUE );
    m_pConnectionSpeedInfo  ->UseSmallText( TRUE );

    m_pGameType             ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pCurrentMap           ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pNextMap              ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pFriendlyFire         ->SetLabelFlags( ui_font::h_right|ui_font::v_center ); 
    m_pPctComplete          ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pConnectionSpeed      ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pGameTypeInfo         ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pCurrentMapInfo       ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pNextMapInfo          ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pFriendlyFireInfo     ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pPctCompleteInfo      ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pConnectionSpeedInfo  ->SetLabelFlags( ui_font::h_left|ui_font::v_center );

    m_pGameType             ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pCurrentMap           ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pNextMap              ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pFriendlyFire         ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pPctComplete          ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pConnectionSpeed      ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pGameTypeInfo         ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pCurrentMapInfo       ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pNextMapInfo          ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pFriendlyFireInfo     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pPctCompleteInfo      ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pConnectionSpeedInfo  ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    m_pGameType             ->SetLabelColor( xcolor(255,252,204,255) );
    m_pCurrentMap           ->SetLabelColor( xcolor(255,252,204,255) );
    m_pNextMap              ->SetLabelColor( xcolor(255,252,204,255) );
    m_pFriendlyFire         ->SetLabelColor( xcolor(255,252,204,255) );
    m_pPctComplete          ->SetLabelColor( xcolor(255,252,204,255) );
    m_pConnectionSpeed      ->SetLabelColor( xcolor(255,252,204,255) );
    m_pGameTypeInfo         ->SetLabelColor( xcolor(255,252,204,255) );
    m_pCurrentMapInfo       ->SetLabelColor( xcolor(255,252,204,255) );
    m_pNextMapInfo          ->SetLabelColor( xcolor(255,252,204,255) );
    m_pFriendlyFireInfo     ->SetLabelColor( xcolor(255,252,204,255) );
    m_pPctCompleteInfo      ->SetLabelColor( xcolor(255,252,204,255) );
    m_pConnectionSpeedInfo  ->SetLabelColor( xcolor(255,252,204,255) );

    // populate server info
    PopulateServerInfo( &g_PendingConfig.GetConfig() );

    // fill the match list
    FillPlayerList();

    // initialize screen scaling
    InitScreenScaling( Position );


    if( !(CONFIG_IS_AUTOSERVER || CONFIG_IS_AUTOCLIENT) )
    {
        // clear in-use controller flags
        for( int i=0; i<MAX_LOCAL_PLAYERS; i++)
        {
            g_StateMgr.SetControllerRequested(i, FALSE);
        }
    }

    m_JoinPasswordEntered   = FALSE;
    m_JoinPasswordOk        = FALSE;
    m_lockedOut             = TRUE;

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

	// Return success code
    return Success;
}

//=========================================================================

void dlg_online_players::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_online_players::Render( s32 ox, s32 oy )
{
    static s32 offset   =  0;
    static s32 gap      =  9;
    static s32 width    =  4;

	irect rb;
    
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

    // render the normal dialog stuff
    ui_dialog::Render( ox, oy );

#ifdef TARGET_PS2
    // render gamespy logo
    irect LogoPos = m_Position;
    LogoPos.r -= 35;
    LogoPos.l = LogoPos.r - 128;
    LogoPos.b = LogoPos.t + 32;
    g_UiMgr->RenderBitmap( g_UiMgr->FindBitmap( "gamespy_logo" ), LogoPos, XCOLOR_WHITE );
#endif

    // render the glow bar
    g_UiMgr->RenderGlowBar();
}

//=========================================================================

void dlg_online_players::OnNotify ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData )
{
    (void)pWin;
    (void)pSender;
    (void)Command;
    (void)pData;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if (Command == WN_LIST_ACCEPTED)
        {
        }
    } 
}

//=========================================================================

void dlg_online_players::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
}

//=========================================================================

void dlg_online_players::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        // store the active controller
        g_StateMgr.SetActiveControllerID( g_UiMgr->GetActiveController() );
        g_StateMgr.SetControllerRequested( g_UiMgr->GetActiveController(), TRUE );
        m_State = DIALOG_STATE_SELECT;
    }
}

//=========================================================================

void dlg_online_players::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        g_AudioMgr.Play("Backup");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_online_players::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // turn on the controls
            m_pPlayerList->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pServerDetails->SetFlag(ui_win::WF_VISIBLE, TRUE);
            GotoControl( (ui_control*)m_pPlayerList );
            g_UiMgr->SetScreenHighlight( m_pPlayerList->GetPosition() );

            // activate text
            m_pGameType             ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pCurrentMap           ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            //m_pNextMap              ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pFriendlyFire         ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pPctComplete          ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pConnectionSpeed      ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pGameTypeInfo         ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pCurrentMapInfo       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            //m_pNextMapInfo          ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pFriendlyFireInfo     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pPctCompleteInfo      ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pConnectionSpeedInfo  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText              ->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
    }

    // populate server info
    PopulateServerInfo( &g_PendingConfig.GetConfig() );

    // update list box
    FillPlayerList();

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    // update everything else
    ui_dialog::OnUpdate( pWin, DeltaTime );
}

//=========================================================================

void dlg_online_players::FillPlayerList( void )
{
    const extended_info* pInfo;

    // store current selection
    s32 iSel = m_pPlayerList->GetSelection();

    // Fill Listbox with levels
    m_pPlayerList->DeleteAllItems();

    pInfo = g_MatchMgr.GetExtendedServerInfo( g_PendingConfig.GetConfig().ID );
    if( pInfo )
    {
        s32         i;

        // put the players in the list
        for( i=0 ; i<NET_MAX_PLAYERS ; i++ )
        {
            if( pInfo->Score.Player[i].IsInGame )
            {
                m_pPlayerList->AddItem( pInfo->Score.Player[i].Name, (s32)&pInfo->Score.Player[i] );
            }
        }

        // Limit Selection
        if( iSel <  0 ) iSel = 0;
        if( iSel >= m_pPlayerList->GetItemCount() )  iSel = m_pPlayerList->GetItemCount()-1;

        if( m_pPlayerList->GetItemCount() != 0 )
        {
            // Set Selection
            m_pPlayerList->SetSelection( iSel );
        }

        // set scorefield mask
        //m_pPlayerList->SetScoreFieldMask( pInfo->Score.ScoreFieldMask );
        m_pPlayerList->SetScoreFieldMask( SCORE_POINTS | SCORE_DEATHS );
    }
    else
    {
        // set scorefield mask
        m_pPlayerList->SetScoreFieldMask( 0 );
    }
}

//=========================================================================

s32 dlg_online_players::PingToColor( f32 ping, xcolor& responsecolor )
{
    xcolor S_RED   ( 255,128, 96,224 );
    xcolor S_YELLOW( 230,230,  0,230 );
    xcolor S_GREEN ( 128,255, 96,224 );
    //xcolor S_GRAY  ( 170,170,170,224 );

    if      (ping > 500.0f)  { responsecolor = S_RED;   return 8;   }
    else if (ping > 400.0f)  { responsecolor = S_RED;   return 7;   }
    else if (ping > 300.0f)  { responsecolor = S_RED;   return 6;   }
    else if (ping > 250.0f)  { responsecolor = S_YELLOW;return 5;   }
    else if (ping > 200.0f)  { responsecolor = S_YELLOW;return 4;   }
    else if (ping > 150.0f)  { responsecolor = S_YELLOW;return 3;   }
    else if (ping > 125.0f)  { responsecolor = S_GREEN; return 2;   }
    else if (ping > 100.0f)  { responsecolor = S_GREEN; return 2;   }
    else if (ping > 75.0f)   { responsecolor = S_GREEN; return 1;   }
    else                     { responsecolor = S_GREEN; return 0;   }
}

//=========================================================================

void dlg_online_players::PopulateServerInfo(const server_info *pServerInfo)
{
    xcolor responsecolor;
    xwchar buff[128];
    xwstring tempLabel;

    x_wstrcpy(buff, g_StringTableMgr( "ui", "IDS_SERVER_DETAILS" ));

    if (pServerInfo)
    {
        x_wstrcat( buff, xwstring(" ") );
        xwstring temp = pServerInfo->Name;
        x_wstrcat( buff, temp );

        if ( pServerInfo->Flags & SERVER_FRIENDLY_FIRE )
        {
            m_pFriendlyFireInfo->SetLabel( g_StringTableMgr( "ui", "IDS_YES" ) );
        }
        else
        {
            m_pFriendlyFireInfo->SetLabel( g_StringTableMgr( "ui", "IDS_NO" ) );
        }
        m_pPctCompleteInfo->SetLabel( (xwstring)xfs("%d%%",pServerInfo->PercentComplete) );

        s32 response;
        response = PingToColor( pServerInfo->PingDelay, responsecolor );
        m_pConnectionSpeedInfo->SetLabel( xwstring( "||||||||||" + response ) );
        m_pConnectionSpeedInfo->SetLabelColor( responsecolor );

        m_pGameTypeInfo->SetLabel( xwstring(pServerInfo->GameType) );
        m_pCurrentMapInfo->SetLabel( pServerInfo->MissionName );
        // TODO: set next map info label
    }
    else
    {
        m_pFriendlyFireInfo     ->SetLabel( xwstring("---") );
        m_pPctCompleteInfo      ->SetLabel( xwstring("---") );
        m_pConnectionSpeedInfo  ->SetLabel( xwstring("---") );
        m_pGameTypeInfo         ->SetLabel( xwstring("---") );
        m_pCurrentMapInfo       ->SetLabel( xwstring("---") );
        //m_pNextMapInfo          ->SetLabel( xwstring("---") );
    }

    m_pServerDetails->SetLabel( buff );
}
//=========================================================================
