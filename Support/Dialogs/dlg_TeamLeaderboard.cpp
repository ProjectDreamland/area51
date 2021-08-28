//=========================================================================
//
//  dlg_team_leaderboard.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_button.hpp"

#include "dlg_TeamLeaderboard.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "NetworkMgr\Voice\VoiceMgr.hpp"

#ifndef CONFIG_RETAIL
#include "InputMgr\monkey.hpp"
#endif

//
// Test data - remove when complete
//
//static player_score m_TempData[16];


//=========================================================================
//  Team TeamLeaderboard Dialog
//=========================================================================

ui_manager::control_tem TeamLeaderboardControls[] = 
{
    { IDC_TEAM_LEADERBOARD_DETAILS,         "IDS_NULL",        "blankbox",  159,  16, 334, 322, 0, 0, 1, 1, ui_win::WF_VISIBLE|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_TEAM_LEADERBOARD_FRAME_MAIN,      "IDS_NULL",        "frame",       8,   2, 496, 350, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_TEAM_LEADERBOARD_FRAME,           "IDS_NULL",        "frame",     157,  14, 336, 328, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_TEAM_LEADERBOARD_FRAME_HEADER1,   "IDS_NULL",        "frame",      22,  36, 137,  30, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_TEAM_LEADERBOARD_FRAME_HEADER2,   "IDS_NULL",        "frame",      22, 188, 137,  30, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_TEAM_LEADERBOARD_FRAME_TEAM1,     "IDS_NULL",        "frame",     157,  36, 336, 153, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_TEAM_LEADERBOARD_FRAME_TEAM2,     "IDS_NULL",        "frame",     157, 188, 336, 153, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_TEAM_LEADERBOARD_LOADING_TEXT,    "IDS_NULL",        "text",      235, 395, 230,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_TEAM_LEADERBOARD_LOADING_PIPS,    "IDS_NULL",        "text",      465, 395,  50,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_TEAM_LEADERBOARD_NAV_TEXT,        "IDS_NULL",        "text",       25, 395, 200,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem TeamLeaderboardDialog =
{
    "IDS_NULL",
    1, 9,
    sizeof(TeamLeaderboardControls)/sizeof(ui_manager::control_tem),
    &TeamLeaderboardControls[0],
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

extern s32 s_LineSpacing;

//=========================================================================
//  Registration function
//=========================================================================

void dlg_team_leaderboard_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "team leaderboard", &TeamLeaderboardDialog, &dlg_team_leaderboard_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_team_leaderboard_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_team_leaderboard* pDialog = new dlg_team_leaderboard;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_team_leaderboard
//=========================================================================

dlg_team_leaderboard::dlg_team_leaderboard( void )
{
#ifdef TARGET_PS2
    if( x_GetTerritory() != XL_TERRITORY_AMERICA )
    {
        s_LineSpacing = 19;
    }
#endif

#ifdef TARGET_XBOX
    m_XBOXNotificationOffsetX = -20;
    m_XBOXNotificationOffsetY = -12;
#endif
}

//=========================================================================

dlg_team_leaderboard::~dlg_team_leaderboard( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_team_leaderboard::Create( s32                        UserID,
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
    Success = dlg_mp_score::Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    m_pDetailsBox	    = (ui_blankbox*)    FindChildByID( IDC_TEAM_LEADERBOARD_DETAILS       );
    m_pFrame            = (ui_frame*)       FindChildByID( IDC_TEAM_LEADERBOARD_FRAME         );
    m_pFrameMain        = (ui_frame*)       FindChildByID( IDC_TEAM_LEADERBOARD_FRAME_MAIN    );
    m_pFrameTeamOne     = (ui_frame*)       FindChildByID( IDC_TEAM_LEADERBOARD_FRAME_TEAM1   );
    m_pFrameTeamTwo     = (ui_frame*)       FindChildByID( IDC_TEAM_LEADERBOARD_FRAME_TEAM2   );
    m_pFrameHeaderOne   = (ui_frame*)       FindChildByID( IDC_TEAM_LEADERBOARD_FRAME_HEADER1 );
    m_pFrameHeaderTwo   = (ui_frame*)       FindChildByID( IDC_TEAM_LEADERBOARD_FRAME_HEADER2 );
    m_pLoadingText      = (ui_text*)        FindChildByID( IDC_TEAM_LEADERBOARD_LOADING_TEXT  );
    m_pLoadingPips      = (ui_text*)        FindChildByID( IDC_TEAM_LEADERBOARD_LOADING_PIPS  );
    m_pNavText          = (ui_text*)        FindChildByID( IDC_TEAM_LEADERBOARD_NAV_TEXT      );

    // intialize frames
    m_pFrame->SetBackgroundColor( xcolor(0,0,0,0) );
    m_pFrame->ChangeElement("frame2");

    m_pFrameMain->SetBackgroundColor( xcolor(39,117,28,64) );
    m_pFrameMain->ChangeElement("frame2");

    m_pFrameTeamOne->SetBackgroundColor( xcolor(0,0,0,0) );
    m_pFrameTeamOne->ChangeElement("frame2");

    m_pFrameTeamTwo->SetBackgroundColor( xcolor(0,0,0,0) );
    m_pFrameTeamTwo->ChangeElement("frame2");

    m_pFrameHeaderOne->SetBackgroundColor( xcolor(19,59,14,196) );
    m_pFrameHeaderOne->ChangeElement("frame2");

    m_pFrameHeaderTwo->SetBackgroundColor( xcolor(19,59,14,196) );
    m_pFrameHeaderTwo->ChangeElement("frame2");

    // initialize team one details box
    m_pDetailsBox->SetFlag(ui_win::WF_VISIBLE, TRUE); 
    m_pDetailsBox->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pDetailsBox->SetHasTitleBar( TRUE );
    m_pDetailsBox->SetLabelColor( xcolor(255,252,204,255) );
    m_pDetailsBox->SetTitleBarColor( xcolor(19,59,14,196) );      

    // initialize loading text
    m_pLoadingText->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pLoadingText->SetLabelFlags( ui_font::h_right|ui_font::v_top );
    m_pLoadingText->UseSmallText(TRUE);

    // build loading text string
    xwstring LoadText;
    LoadText = g_StringTableMgr( "ui", "IDS_LOADING_MSG" );
    if( x_GetTerritory() == XL_TERRITORY_AMERICA )
    {
        LoadText += " ";
        LoadText += g_ActiveConfig.GetShortGameType();
        LoadText += ":";
        LoadText += g_ActiveConfig.GetLevelName();
    }
    m_pLoadingText->SetLabel( LoadText );

    // initialize loading pips    
    m_pLoadingPips->SetFlag( ui_win::WF_VISIBLE, TRUE );
    m_pLoadingPips->SetLabelFlags( ui_font::h_left|ui_font::v_top );
    m_pLoadingPips->UseSmallText(TRUE);

    // initialize nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_QUIT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_UNPAUSE" );
    navText += g_StringTableMgr( "ui", "IDS_NAV_ONLINE_MENU" );
  
    // Grab a bunch of strings.
    m_pStrLevelDesc  = g_StringTableMgr( "ui", "IDS_NAV_LEVEL_DESC"  );
    m_pStrPlayer     = g_StringTableMgr( "ui", "IDS_HEADER_PLAYER"   );
    m_pStrScore      = g_StringTableMgr( "ui", "IDS_HEADER_SCORE"    );
    m_pStrIconKills  = g_StringTableMgr( "ui", "IDS_ICON_KILLS"      );
    m_pStrIconDeaths = g_StringTableMgr( "ui", "IDS_ICON_DEATHS"     );
    m_pStrIconTKs    = g_StringTableMgr( "ui", "IDS_ICON_TEAM_KILLS" );
    m_pStrIconFlags  = g_StringTableMgr( "ui", "IDS_ICON_FLAGS"      );
    m_pStrIconVotes  = g_StringTableMgr( "ui", "IDS_ICON_VOTES"      );

    m_pNavText->SetLabel( navText );
    m_pNavText->SetFlag( ui_win::WF_VISIBLE, TRUE );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // disable the highlight
    g_UiMgr->DisableScreenHighlight();

    // Initialize player data
    s32 i;
    for( i=0 ; i<32 ; i++ )
    {
        m_PlayerData[i].ClearAll();
    } 

    //
    // Testing - fill in some test data
    // 
    //for( i=0 ; i<16 ; i++ )
    //{
    //    m_TempData[i].ClearAll();
    //
    //    if( i < 16 )
    //    {
    //        x_strcpy( m_TempData[i].NName, xfs("Player%2d", i ));
    //        m_TempData[i].IsInGame  = 1;
    //        m_TempData[i].Team      = x_irand(0, 1);
    //        m_TempData[i].Kills     = x_irand(0, 5);
    //        m_TempData[i].Deaths    = x_irand(0, 2);
    //        m_TempData[i].TKs       = x_irand(0, 1);
    //        m_TempData[i].Flags     = x_irand(0, 1);
    //        m_TempData[i].Votes     = x_irand(0, 1);
    //
    //        m_TempData[i].Score     = m_TempData[i].Kills - m_TempData[i].Deaths;
    //        m_TempData[i].Score    -= m_TempData[i].TKs;
    //    }
    //}



    // initialize popup
    m_pPopUp = NULL;

    // get the font
    m_Font = g_UiMgr->FindFont("small");

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_team_leaderboard::Destroy( void )
{
    dlg_mp_score::Destroy();
}

//=========================================================================

void dlg_team_leaderboard::Render( s32 ox, s32 oy )
{
    if( (GetFlags() & ui_win::WF_VISIBLE)==0 )
    {
        return;
    }

    irect rb;

	s32 XRes, YRes;
    eng_GetRes(XRes, YRes);
#ifdef TARGET_PS2
    // Nasty hack to force PS2 to draw to rb.l = 0
    rb.Set( -1, 0, XRes, YRes );
#else
    rb.Set( 0, 0, XRes, YRes );
#endif
    // render background filter
    g_UiMgr->RenderGouraudRect(rb, xcolor(0,0,0,180),
                                   xcolor(0,0,0,180),
                                   xcolor(0,0,0,180),
                                   xcolor(0,0,0,180),FALSE);
    
    
    // render the normal dialog stuff
    ui_dialog::Render( ox, oy );

    // render score text
    RenderScoreList( ox, oy );
}

//=========================================================================

void dlg_team_leaderboard::OnPadDelete( ui_win* pWin )
{
    (void)pWin;      
    
    if( m_Mode != LEADERBOARD_PAUSE )
        return;

    if( m_State == DIALOG_STATE_ACTIVE )
    {
        if( m_pPopUp == NULL )
        {
#ifndef CONFIG_RETAIL
            if ( g_MonkeyOptions.Enabled && g_MonkeyOptions.ModeEnabled[MONKEY_MENUMONKEY] )
                return;
#endif

            // Open a dialog to confirm quitting the online game component
            irect r = g_UiMgr->GetUserBounds( g_UiUserID );
            m_pPopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

            // get message text
            xwstring Message;
            if( g_NetworkMgr.IsOnline() && g_NetworkMgr.IsServer() )
            {
                Message = g_StringTableMgr( "ui", "IDS_QUIT_ONLINE_MSG" );
            }
            else
            {
                Message = g_StringTableMgr( "ui", "IDS_QUIT_MSG" );
            }

            // set nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

            m_pPopUp->Configure( g_StringTableMgr( "ui", "IDS_QUIT_POPUP" ), TRUE, TRUE, FALSE, Message, navText, &m_PopUpResult );
        }
    }
}

//=========================================================================

void dlg_team_leaderboard::OnPadSelect( ui_win* pWin )
{
    (void)pWin;
    if( m_State == DIALOG_STATE_ACTIVE )
    {
        // goto the pause menu
        m_State = DIALOG_STATE_SELECT;
    }
}

//=========================================================================

void dlg_team_leaderboard::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // update list box
    FillScoreList();

    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    xwstring NavText;

    switch( m_Mode )
    {
    case LEADERBOARD_FINAL:
        NavText = g_StringTableMgr( "ui", "IDS_NAV_OK" );
        break;

    case LEADERBOARD_PAUSE:
        // initialize nav text
        NavText  = g_StringTableMgr( "ui", "IDS_NAV_QUIT" );
        NavText += g_StringTableMgr( "ui", "IDS_NAV_UNPAUSE" );
        NavText += g_StringTableMgr( "ui", "IDS_NAV_ONLINE_MENU");
        break;

    case LEADERBOARD_INTERLEVEL:
        {
            m_pNavText->SetLabelFlags( ui_font::h_left|ui_font::v_top );
            NavText = m_pStrLevelDesc;

            xwstring LoadText("");
            switch( (s32)(m_LoadTimeElapsed*4.0f) % 4 )
            {
            case 0: LoadText += "   ";   break;
            case 1: LoadText += ".  ";   break;
            case 2: LoadText += ".. ";   break;
            case 3: LoadText += "...";   break;
            }
            m_pLoadingText->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pLoadingPips->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pLoadingPips->SetLabel( LoadText );
        }
        break;

    default:
        ASSERT( FALSE );
    }

    m_pNavText->SetLabel( NavText );

    if( m_pPopUp )
    {
        if( m_PopUpResult != DLG_POPUP_IDLE )
        {
            if( m_PopUpResult == DLG_POPUP_YES )
            {
                // quit game
                m_pPopUp = NULL;
                m_State = DIALOG_STATE_EXIT;
            }
            else
            {
                // return to dialog
                m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
                m_pPopUp = NULL;
            }
        }
    }

    // let the base class do its thing
    dlg_mp_score::OnUpdate( pWin, DeltaTime );
}


//=========================================================================
// Returns 0 if item 1 == item 2
// Returns -1 if item 1 < item 2
// Returns +1 if item 1 > item 2
//
static s32 sort_compare(const void* pItem1, const void* pItem2)
{
    player_score* pScore1 = (player_score*)pItem1;
    player_score* pScore2 = (player_score*)pItem2;
    // sort by in game state
    if( pScore1->IsInGame > pScore2->IsInGame )
    {
        return -1;
    }
    else if ( pScore1->IsInGame < pScore2->IsInGame )
    {
        return 1;
    }
    else
    {
        // sort by team
        if( pScore1->Team > pScore2->Team )
        {
            return 1;
        }
        else if( pScore1->Team < pScore2->Team )
        {
            return -1;
        }
        else
        {
            // sort by score
            if( pScore1->Score > pScore2->Score )
            {
                return -1;
            }
            else if( pScore1->Score < pScore2->Score )
            {
                return 1;
            }
            else
            {
                // same score, sort on name
                return( x_strcmp( pScore1->NName, pScore2->NName ) );
            }
        }
    }
}
//=========================================================================

void dlg_team_leaderboard::FillScoreList( void )
{
    s32         i;
    s32         Count = 0;

    // zero team totals
    m_PlayerTotals[0].ClearScore();
    m_PlayerTotals[1].ClearScore();
    m_TeamCount[0] = 0;
    m_TeamCount[1] = 0;

    // get player data
    const game_score& ScoreData = GameMgr.GetScore();

    s32 MySlot;

    // find out my player slot
    if( m_Mode != LEADERBOARD_PAUSE )
    {
        if( g_NetworkMgr.IsOnline() )
        {
            MySlot = g_StateMgr.GetLocalPlayerSlot();
        }
        else
        {
            // no highlight
            MySlot = -1;
        }
    }
    else
    {
        if( g_NetworkMgr.IsOnline() )
        {
            MySlot = g_NetworkMgr.GetLocalPlayerSlot(0);
        }
        else
        {
            // get local player slot for split screen
            MySlot = g_NetworkMgr.GetLocalPlayerSlot( g_StateMgr.GetActiveControllerID() );
        }
    }

    // Clear the entire list.
    x_memset( m_PlayerData, 0, sizeof(m_PlayerData) );

    // put the players in the list
    for( i=0; i<32; i++ )
    {
        if( ScoreData.Player[i].IsInGame )
        {
            // get the team
            u32 Team = ScoreData.Player[i].Team;
            x_memcpy( &m_PlayerData[Count], &ScoreData.Player[i], sizeof(player_score) );

            // is this player me?
            if( i == MySlot )
            {
                // it's me
                m_PlayerData[Count].ClientIndex = 1;
            }
            else
            {
                // not me!
                m_PlayerData[Count].ClientIndex = 0;
            }

            // is this player highlighted
            if( ScoreData.HighLightMask & ( 1 << i ) )
            {
                // NOTE: I am overloading the music level field to indicate
                // if the player is highlighted or not.
                // Music level is an unused variable for the leaderboard and
                // since I am making a local copy of the data, this will have
                // no ill effects.
                m_PlayerData[Count].MusicLevel = 1;
            }
            else
            {
                // not highlighted
                m_PlayerData[Count].MusicLevel = 0;
            }

            // Copy locally muted field.
            m_PlayerData[Count].IsVoiceAllowed = !g_VoiceMgr.GetLocallyMuted( i );

            m_PlayerTotals[Team].Score  += ScoreData.Player[i].Score;  //m_TempData[i].Score; 
            m_PlayerTotals[Team].Kills  += ScoreData.Player[i].Kills;  //m_TempData[i].Kills; 
            m_PlayerTotals[Team].Deaths += ScoreData.Player[i].Deaths; //m_TempData[i].Deaths; 
            m_PlayerTotals[Team].TKs    += ScoreData.Player[i].TKs;    //m_TempData[i].TKs; 
            m_PlayerTotals[Team].Flags  += ScoreData.Player[i].Flags;  //m_TempData[i].TKs; 
            m_PlayerTotals[Team].Votes  += ScoreData.Player[i].Votes;  //m_TempData[i].TKs; 
            m_TeamCount[Team]++;

            Count++;
        }
    }

    // now we have the player data sort it
    if( Count )
    {
        // sort each of the team lists
        x_qsort( &m_PlayerData[0], 32, sizeof(player_score), sort_compare );
    }

    // calculate how many lines to draw for each team
    if( m_TeamCount[0] > 8 )
    {
        m_DrawCount[0] = m_TeamCount[0];
        m_DrawCount[1] = 16 - m_DrawCount[0];
    }
    else if( m_TeamCount[1] > 8 )
    {
        m_DrawCount[1] = m_TeamCount[1];
        m_DrawCount[0] = 16 - m_DrawCount[1];
    }
    else
    {
        m_DrawCount[0] = 8;
        m_DrawCount[1] = 8;
    }



    // resize team data boxes
    irect currPos = m_pFrameTeamOne->GetPosition();
    currPos.b = 16 + 22 + 22 + (s_LineSpacing * m_DrawCount[0]);
#ifdef TARGET_PS2
    if( x_GetTerritory() != XL_TERRITORY_AMERICA )
    {
        currPos.t = 38;
    }
#endif
    m_pFrameTeamOne->SetPosition( currPos );

    currPos = m_pFrameTeamTwo->GetPosition();
    currPos.t = 16 + 22 + 21 + (s_LineSpacing * m_DrawCount[0]);
    m_pFrameTeamTwo->SetPosition( currPos );

    // determine if the score and the team name will fit on one line
    s32 Font = g_UiMgr->FindFont("large");
    irect TextRect;

    // calculate the width of team 0s name and score
    g_UiMgr->TextSize( Font, TextRect, ScoreData.Team[0].Name, 255 ); 
    m_TeamWidth[0] = TextRect.r;
    xwstring ScoreString( xfs("%d", ScoreData.Team[0].Score) );
    g_UiMgr->TextSize( Font, TextRect, ScoreString, 255 ); 
    m_TeamWidth[0] += TextRect.r;

    // calculate the width of team 1s name and score
    g_UiMgr->TextSize( Font, TextRect, ScoreData.Team[1].Name, 255 ); 
    m_TeamWidth[1] = TextRect.r;
    xwstring ScoreString2( xfs("%d", ScoreData.Team[1].Score) );
    g_UiMgr->TextSize( Font, TextRect, ScoreString2, 255 ); 
    m_TeamWidth[1] += TextRect.r;

    // resize team score boxes
    irect r = GetPosition();
    r.l += 22;
    r.r = r.l + 132;
    r.t += 16 + 22;
    r.b = r.t + 30;
    
    currPos = m_pFrameHeaderOne->GetPosition();
#ifdef TARGET_XBOX
    extern xbool g_b480P;
    if( g_b480P )
        currPos.t = r.t - 18;
    else
        currPos.t = r.t - 28;
#else
    currPos.t = r.t - 18;
#endif
    SetTeamRenderType( 0, currPos );
    m_pFrameHeaderOne->SetPosition( currPos );

    r.t += (s_LineSpacing * m_DrawCount[0]) + 24;
    currPos = m_pFrameHeaderTwo->GetPosition();
#ifdef TARGET_XBOX
    if( g_b480P )
        currPos.t = r.t - 19;
    else
        currPos.t = r.t - 29;
#elif defined TARGET_PS2
    if( x_GetTerritory() != XL_TERRITORY_AMERICA )
    {
        currPos.t = r.t - 20;
    }
    else
    {
        currPos.t = r.t - 19;
    }
#else
    currPos.t = r.t - 19;
#endif
    SetTeamRenderType( 1, currPos );
    m_pFrameHeaderTwo->SetPosition( currPos );
}
//=========================================================================

void dlg_team_leaderboard::SetTeamRenderType( u32 TeamID, irect& Position )
{
    u32 TeamID2;

    if(TeamID == 0)
        TeamID2 = 1;
    else
        TeamID2 = 0;

    if( m_TeamWidth[TeamID] > 122 )
    {
        switch( m_DrawCount[TeamID2] )
        {
        case 16:
            Position.b = Position.t + 26;
            m_TeamRenderType[TeamID] = RENDER_TEAM_SMALL;
            break;
        case 15:
            Position.b = Position.t + 42;
            m_TeamRenderType[TeamID] = RENDER_TEAM_TWO_LINES_SMALL;
            break;
        case 14:
        default:
            Position.b = Position.t + 58;
            m_TeamRenderType[TeamID] = RENDER_TEAM_TWO_LINES;
            break;
        }
    }
    else
    {
        switch( m_DrawCount[TeamID2] )
        {
        case 16:
            Position.b = Position.t + 26;
            m_TeamRenderType[TeamID] = RENDER_TEAM_SMALL;
            break;
        default:
            Position.b = Position.t + 30;
            m_TeamRenderType[TeamID] = RENDER_TEAM_NORMAL;
            break;
        }
    }
}

//=========================================================================

void dlg_team_leaderboard::RenderScoreList( s32 ox, s32 oy )
{
    (void)ox;
    (void)oy;

    s32 i;
    s32 j;
    irect r;
    xcolor ch(255,252,204,255);
    s32 Font = g_UiMgr->FindFont("large");
    
    s32 PlayerSize = 272;  // size allowed to player before any data columns

    s32 Count1 = m_DrawCount[0];
    s32 Count2 = m_DrawCount[1];

    const game_score& ScoreData = GameMgr.GetScore();

    // determine how many columns of data we need to display
    s32 ColumnCount = 0;
   
    if( ScoreData.ScoreFieldMask & SCORE_KILLS )
        ColumnCount++;
    if( ScoreData.ScoreFieldMask & SCORE_DEATHS )
        ColumnCount++;
    if( ScoreData.ScoreFieldMask & SCORE_TKS )
        ColumnCount++;
    if( ScoreData.ScoreFieldMask & SCORE_FLAGS )
        ColumnCount++;
    if( ScoreData.ScoreFieldMask & SCORE_VOTES )
        ColumnCount++;

    PlayerSize -= ( ColumnCount * 35 );

    // render the main title bar
    r = GetPosition();
    r.l += 159 + 5;
    r.t += 16;
    r.b = r.t + 16;

    // leave space for speaker icon   
    r.l += 16;

#ifdef TARGET_PS2
    if( x_GetTerritory() != XL_TERRITORY_AMERICA )
    {
        r.t += 2;
        r.b += 2;
    }
#endif

    m_pManager->RenderText( m_Font, r, ui_font::h_left|ui_font::v_center, ch, m_pStrPlayer );
    
    r.l += PlayerSize + 5;
    r.r = r.l + 30;

    if( ScoreData.ScoreFieldMask & SCORE_POINTS )
    {
        m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, m_pStrScore );
    }
    
    if( ScoreData.ScoreFieldMask & SCORE_KILLS )
    {
        r.l += 35;
        r.r += 35;
        m_pManager->RenderText( m_Font, r,  ui_font::h_right|ui_font::v_center, ch, m_pStrIconKills );
    }

    if( ScoreData.ScoreFieldMask & SCORE_DEATHS )
    {
        r.l += 35;
        r.r += 35;
        m_pManager->RenderText( m_Font, r,  ui_font::h_right|ui_font::v_center, ch, m_pStrIconDeaths );       
    }

    if( ScoreData.ScoreFieldMask & SCORE_TKS )
    {
        r.l += 35;
        r.r += 35;
        m_pManager->RenderText( m_Font, r,  ui_font::h_right|ui_font::v_center, ch, m_pStrIconTKs );       
    }

    if( ScoreData.ScoreFieldMask & SCORE_FLAGS )
    {
        r.l += 35;
        r.r += 35;
        m_pManager->RenderText( m_Font, r,  ui_font::h_right|ui_font::v_center, ch, m_pStrIconFlags );       
    }

    if( ScoreData.ScoreFieldMask & SCORE_VOTES )
    {
        r.l += 35;
        r.r += 35;
        m_pManager->RenderText( m_Font, r,  ui_font::h_right|ui_font::v_center, ch, m_pStrIconVotes );       
    }

    // render bars
    r = GetPosition();
    r.l += 159;
    r.r = r.l + 332;
    r.t += 16 + 22;
    r.b = r.t + 16;

    for( i=0; i<Count1; i++ )
    {
        // draw every other one
        if( i & 1 )
        {
            m_pManager->RenderRect( r, xcolor(20,58,14,64), FALSE );
        }

        // move down a spot
        r.t += s_LineSpacing;
        r.b += s_LineSpacing;
    }

    // render player data
    r = GetPosition();
    r.l += 159 + 5;
    r.r = r.l + 16;
#ifdef TARGET_PS2
    if( x_GetTerritory() != XL_TERRITORY_AMERICA )
    {
        r.t += 16 + 23;
    }
    else
#endif
    {
        r.t += 16 + 21;
    }
    r.b = r.t + 16;

    s32 StartL = r.l;
    s32 StartR = r.r;

    for( i=0; i<m_TeamCount[0]; i++ )
    {
        if( m_PlayerData[i].IsInGame )
        {
            xcolor c1(255,252,204,255);

            // check highlight mask 
            // NOTE: music level has been overloaded for this check
            if( m_PlayerData[i].MusicLevel )
            {
                // highlight this player in a different color
                c1.Set( 255, 255,  0, 255 );
            }

            // highlight this client player
            if( m_PlayerData[i].ClientIndex == 1 )
            {
                irect hr = r;
                hr.l -= 5;
                hr.r = hr.l + 332;
                hr.t += 2;
                hr.b += 2;
                m_pManager->RenderRect( hr, xcolor(255,252,204,64), FALSE );
            }

            // render player voice status
            if( !m_PlayerData[ i ].IsVoiceAllowed )
            {
                // player is muted
                m_pManager->RenderText( m_Font, r, ui_font::h_left|ui_font::v_center, c1, xwstring( xstring( (char)0x87 ) ) );
            }
            else if( m_PlayerData[ i ].Speaking )
            {
                // player has the mic!
                m_pManager->RenderText( m_Font, r, ui_font::h_left|ui_font::v_center, c1, xwstring( xstring( (char)0x86 ) ) );
            }

            r.l += 16;
            r.r = r.l + PlayerSize;
            m_pManager->RenderText( m_Font, r, ui_font::h_left|ui_font::v_center|ui_font::clip_ellipsis, c1, m_PlayerData[i].NName );            

            r.l += PlayerSize + 5;
            r.r = r.l + 30;

            if( ScoreData.ScoreFieldMask & SCORE_POINTS )
            {
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[i].Score) );
            }
            
            if( ScoreData.ScoreFieldMask & SCORE_KILLS )
            {
                r.l += 35;
                r.r += 35;
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[i].Kills) );
            }

            if( ScoreData.ScoreFieldMask & SCORE_DEATHS )
            {
                r.l += 35;
                r.r += 35;
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[i].Deaths) );
            }

            if( ScoreData.ScoreFieldMask & SCORE_TKS )
            {
                r.l += 35;
                r.r += 35;
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[i].TKs) );
            }

            if( ScoreData.ScoreFieldMask & SCORE_FLAGS )
            {
                r.l += 35;
                r.r += 35;
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[i].Flags) );
            }

            if( ScoreData.ScoreFieldMask & SCORE_VOTES )
            {
                r.l += 35;
                r.r += 35;
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[i].Votes) );
            }

            r.l = StartL;
            r.r = StartR;
            r.t += s_LineSpacing;
            r.b += s_LineSpacing;
        }
    }

    // render player data
    r = GetPosition();
    r.l += 159 + 5;
    r.r = r.l + PlayerSize;
    r.t += 16 + 22;
    r.b = r.t + 16;

    // Render Team 1 footer
    r = GetPosition();
    r.t += (16 + 22 + (s_LineSpacing * Count1));
    r.b = r.t + 22;
    r.l += 159;
    r.r = r.l + 332;
    m_pManager->RenderRect( r, xcolor(19,59,14,196), FALSE );

    r = GetPosition();
    r.l += 159 + 5;
    r.t += (16 + 22 + (s_LineSpacing * Count1));
    r.b = r.t + 16;

    r.l += 16;
//    m_pManager->RenderText( m_Font, r, ui_font::h_left|ui_font::v_center, ch, "Totals" );
    
    r.l += PlayerSize + 5;
    r.r = r.l + 30;

    if( ScoreData.ScoreFieldMask & SCORE_POINTS )
    {
        m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d", m_PlayerTotals[0].Score) );
    }
    
    if( ScoreData.ScoreFieldMask & SCORE_KILLS )
    {
        r.l += 35;
        r.r += 35;
        m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[0].Kills) );
    }

    if( ScoreData.ScoreFieldMask & SCORE_DEATHS )
    {
        r.l += 35;
        r.r += 35;
        m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[0].Deaths) );
    }

    if( ScoreData.ScoreFieldMask & SCORE_TKS )
    {
        r.l += 35;
        r.r += 35;
        m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[0].TKs) );
    }

    if( ScoreData.ScoreFieldMask & SCORE_FLAGS )
    {
        r.l += 35;
        r.r += 35;
        m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[0].Flags) );
    }

    if( ScoreData.ScoreFieldMask & SCORE_VOTES )
    {
        r.l += 35;
        r.r += 35;
        m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[0].Votes) );
    }
    

    // render team2 bars
    r = GetPosition();
    r.l += 159;
    r.r = r.l + 332;
    r.t += ( 16 + 22 + 24 + (s_LineSpacing * Count1));
    r.b = r.t + 16;

    for( i=0; i<Count2; i++ )
    {
        // draw every other one
        if( i & 1 )
        {
            m_pManager->RenderRect( r, xcolor(20,58,14,64), FALSE );
        }

        // move down a spot
        r.t += s_LineSpacing;
        r.b += s_LineSpacing;
    }

    // render team2 player data
    r = GetPosition();
    r.l += 159 + 5;
    r.r = r.l + 16;
    r.t += (16 + 21 + 24 + (s_LineSpacing * Count1));
    r.b = r.t + 16;

    StartL = r.l;
    StartR = r.r;

    for( i=0, j=m_TeamCount[0]; i<m_TeamCount[1]; i++, j++ )
    {
        if( m_PlayerData[j].IsInGame )
        {
            xcolor c1(255,252,204,255);

            // check highlight mask 
            // NOTE: music level has been overloaded for this check
            if( m_PlayerData[j].MusicLevel )
            {
                // highlight this player in a different color
                c1.Set( 255, 255,  0, 255 );
            }

            // highlight this client player
            if( m_PlayerData[j].ClientIndex == 1 )
            {
                irect hr = r;
                hr.l -= 5;
                hr.r = hr.l + 332;
                hr.t += 1;
                hr.b += 1;
                m_pManager->RenderRect( hr, xcolor(255,252,204,64), FALSE );
            }

            // render player voice status
            if( !m_PlayerData[ j ].IsVoiceAllowed )
            {
                // player is muted
                m_pManager->RenderText( m_Font, r, ui_font::h_left|ui_font::v_center, c1, xwstring( xstring( (char)0x87 ) ) );
            }
            else if( m_PlayerData[ j ].Speaking )
            {
                // player has the mic!
                m_pManager->RenderText( m_Font, r, ui_font::h_left|ui_font::v_center, c1, xwstring( xstring( (char)0x86 ) ) );
            }

            r.l += 16;
            r.r = r.l + PlayerSize;
            m_pManager->RenderText( m_Font, r, ui_font::h_left|ui_font::v_center|ui_font::clip_ellipsis, c1, m_PlayerData[j].NName );            

            r.l += PlayerSize + 5;
            r.r = r.l + 30;

            if( ScoreData.ScoreFieldMask & SCORE_POINTS )
            {
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[j].Score) );
            }
            
            if( ScoreData.ScoreFieldMask & SCORE_KILLS )
            {
                r.l += 35;
                r.r += 35;
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[j].Kills) );
            }

            if( ScoreData.ScoreFieldMask & SCORE_DEATHS )
            {
                r.l += 35;
                r.r += 35;
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[j].Deaths) );
            }

            if( ScoreData.ScoreFieldMask & SCORE_TKS )
            {
                r.l += 35;
                r.r += 35;
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[j].TKs) );
            }

            if( ScoreData.ScoreFieldMask & SCORE_FLAGS )
            {
                r.l += 35;
                r.r += 35;
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[j].Flags) );
            }

            if( ScoreData.ScoreFieldMask & SCORE_VOTES )
            {
                r.l += 35;
                r.r += 35;
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[j].Votes) );
            }

            r.l = StartL; 
            r.r = StartR; 
            r.t += s_LineSpacing;
            r.b += s_LineSpacing;
        }
    }

    // Render Team 2 footer
    r = GetPosition();
    r.t += (318 + ((s_LineSpacing - 16)*16) ); //(16 + 24 + 22 + (s_LineSpacing * Count1) + (s_LineSpacing * Count2));
    r.b = r.t + 22;
    r.l += 159;
    r.r = r.l + 332;
    m_pManager->RenderRect( r, xcolor(19,59,14,196), FALSE );

    r = GetPosition();
    r.l += 159 + 5;
    r.t += (318 + ((s_LineSpacing - 16)*16) );
    r.b = r.t + 16;

    r.l += 16;
//    m_pManager->RenderText( m_Font, r, ui_font::h_left|ui_font::v_center, ch, "Totals" );
    
    r.l += PlayerSize + 5;
    r.r = r.l + 30;
    
    if( ScoreData.ScoreFieldMask & SCORE_POINTS )
    {
        m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d", m_PlayerTotals[1].Score) );
    }

    if( ScoreData.ScoreFieldMask & SCORE_KILLS )
    {
        r.l += 35;
        r.r += 35;
        m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[1].Kills) );
    }

    if( ScoreData.ScoreFieldMask & SCORE_DEATHS )
    {
        r.l += 35;
        r.r += 35;
        m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[1].Deaths) );
    }

    if( ScoreData.ScoreFieldMask & SCORE_TKS )
    {
        r.l += 35;
        r.r += 35;
        m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[1].TKs) );
    }

    if( ScoreData.ScoreFieldMask & SCORE_FLAGS )
    {
        r.l += 35;
        r.r += 35;
        m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[1].Flags) );
    }

    if( ScoreData.ScoreFieldMask & SCORE_VOTES )
    {
        r.l += 35;
        r.r += 35;
        m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[1].Votes) );
    }

    // render team1 score bar
    r = GetPosition();
    r.l += 22 + 5;
    r.r = r.l + 122;
    r.t += 16 + 22;
#ifdef TARGET_XBOX
    r.t += 4;
#endif
    irect TextRect;
    xwstring ScoreString( xfs("%d", ScoreData.Team[0].Score) );

    switch( m_TeamRenderType[0] )
    {
    case RENDER_TEAM_TWO_LINES:
        r.b = r.t + 27;
        m_pManager->RenderText( Font, r, ui_font::h_left|ui_font::v_center|ui_font::clip_ellipsis, ch, ScoreData.Team[0].NName );
        r.t += 27;
        r.b += 27;
        m_pManager->RenderText( Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d", ScoreData.Team[0].Score) );
        break;
    case RENDER_TEAM_TWO_LINES_SMALL:
        r.b = r.t + 16;
        m_pManager->RenderText( m_Font, r, ui_font::h_left|ui_font::v_center|ui_font::clip_ellipsis, ch, ScoreData.Team[0].NName );
        r.t += 16;
        r.b += 16;
        m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d", ScoreData.Team[0].Score) );
        break;
    case RENDER_TEAM_NORMAL:
        r.b = r.t + 30 + 2;
        g_UiMgr->TextSize( Font, TextRect, ScoreString, 255 ); 
        r.r = r.l + 122 - TextRect.r;
        m_pManager->RenderText( Font, r, ui_font::h_left|ui_font::v_top|ui_font::clip_ellipsis, ch, ScoreData.Team[0].NName );
        r.r = r.l + 122;
        m_pManager->RenderText( Font, r, ui_font::h_right|ui_font::v_top, ch, xfs("%d", ScoreData.Team[0].Score) );
        break;
    case RENDER_TEAM_SMALL:
        r.b = r.t + 16;
        g_UiMgr->TextSize( m_Font, TextRect, ScoreString, 255 ); 
        r.r = r.l + 122 - TextRect.r;
        m_pManager->RenderText( m_Font, r, ui_font::h_left|ui_font::v_center|ui_font::clip_ellipsis, ch, ScoreData.Team[0].NName );
        r.r = r.l + 122;
        m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d", ScoreData.Team[0].Score) );
        break;
    }

    // render team2 score bar
    r = GetPosition();
    r.l += 22 + 5;
    r.r = r.l + 122;
    r.t += 16 + 22 + (s_LineSpacing * Count1) + 24;
#ifdef TARGET_XBOX
    r.t += 4;
#endif
    xwstring ScoreString2( xfs("%d", ScoreData.Team[1].Score) );

    switch( m_TeamRenderType[1] )
    {
    case RENDER_TEAM_TWO_LINES:
        r.b = r.t + 27; 
        m_pManager->RenderText( Font, r, ui_font::h_left|ui_font::v_center|ui_font::clip_ellipsis, ch, ScoreData.Team[1].NName );
        r.t += 27;
        r.b += 27;
        m_pManager->RenderText( Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d", ScoreData.Team[1].Score) );
        break;
    case RENDER_TEAM_TWO_LINES_SMALL:
        r.b = r.t + 16; 
        m_pManager->RenderText( m_Font, r, ui_font::h_left|ui_font::v_center|ui_font::clip_ellipsis, ch, ScoreData.Team[1].NName );
        r.t += 16;
        r.b += 16;
        m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d", ScoreData.Team[1].Score) );
        break;
    case RENDER_TEAM_NORMAL:
        r.b = r.t + 30 + 2; 
        g_UiMgr->TextSize( Font, TextRect, ScoreString2, 255 ); 
        r.r = r.l + 122 - TextRect.r;
        m_pManager->RenderText( Font, r, ui_font::h_left|ui_font::v_top|ui_font::clip_ellipsis, ch, ScoreData.Team[1].NName );
        r.r = r.l + 122;
        m_pManager->RenderText( Font, r, ui_font::h_right|ui_font::v_top, ch, xfs("%d", ScoreData.Team[1].Score) );
        break;
    case RENDER_TEAM_SMALL:
        r.b = r.t + 16; 
        g_UiMgr->TextSize( m_Font, TextRect, ScoreString2, 255 ); 
        r.r = r.l + 122 - TextRect.r;
        m_pManager->RenderText( m_Font, r, ui_font::h_left|ui_font::v_center|ui_font::clip_ellipsis, ch, ScoreData.Team[1].NName );
        r.r = r.l + 122;
        m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d", ScoreData.Team[1].Score) );
        break;
    }
}

//=========================================================================
