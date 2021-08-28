//=========================================================================
//
//  dlg_big_leaderboard.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_button.hpp"

#include "dlg_BigLeaderboard.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "NetworkMgr\Voice\VoiceMgr.hpp"

#ifndef CONFIG_RETAIL
#include "InputMgr\monkey.hpp"
#endif

//=========================================================================
//  Team Leaderboard Dialog
//=========================================================================

ui_manager::control_tem BigLeaderboardControls[] = 
{
    { IDC_BIG_LEADERBOARD_ONE_DETAILS,  "IDS_NULL",        "blankbox",   24,  36, 216, 283, 0, 0, 1, 1, ui_win::WF_VISIBLE|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_BIG_LEADERBOARD_TWO_DETAILS,  "IDS_NULL",        "blankbox",  256,  36, 216, 283, 0, 0, 1, 1, ui_win::WF_VISIBLE|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },   

    { IDC_BIG_LEADERBOARD_FRAME_ONE,    "IDS_NULL",        "frame",      22,  34, 220, 287, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_BIG_LEADERBOARD_FRAME_TWO,    "IDS_NULL",        "frame",     254,  34, 220, 287, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_BIG_LEADERBOARD_FRAME_MAIN,   "IDS_NULL",        "frame",      10,   24, 476, 309, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },

    { IDC_BIG_LEADERBOARD_LOADING_TEXT, "IDS_NULL",        "text",      235, 395, 230,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_BIG_LEADERBOARD_LOADING_PIPS, "IDS_NULL",        "text",      465, 395,  50,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_BIG_LEADERBOARD_NAV_TEXT,     "IDS_NULL",        "text",       25, 395, 200,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem BigLeaderboardDialog =
{
    "IDS_LEADERBOARD",
    1, 9,
    sizeof(BigLeaderboardControls)/sizeof(ui_manager::control_tem),
    &BigLeaderboardControls[0],
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

void dlg_big_leaderboard_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "big leaderboard", &BigLeaderboardDialog, &dlg_big_leaderboard_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_big_leaderboard_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_big_leaderboard* pDialog = new dlg_big_leaderboard;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_big_leaderboard
//=========================================================================

dlg_big_leaderboard::dlg_big_leaderboard( void )
{
#ifdef TARGET_PS2
    if( x_GetTerritory() != XL_TERRITORY_AMERICA )
    {
        s_LineSpacing = 19;
    }
#endif
}

//=========================================================================

dlg_big_leaderboard::~dlg_big_leaderboard( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_big_leaderboard::Create( s32                        UserID,
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

    m_pPlayerOneBox	= (ui_blankbox*)    FindChildByID( IDC_BIG_LEADERBOARD_ONE_DETAILS  );
    m_pPlayerTwoBox	= (ui_blankbox*)    FindChildByID( IDC_BIG_LEADERBOARD_TWO_DETAILS  );
    m_pLoadingText  = (ui_text*)        FindChildByID( IDC_BIG_LEADERBOARD_LOADING_TEXT );
    m_pLoadingPips  = (ui_text*)        FindChildByID( IDC_BIG_LEADERBOARD_LOADING_PIPS );
    m_pNavText      = (ui_text*)        FindChildByID( IDC_BIG_LEADERBOARD_NAV_TEXT     );
    m_pFrameOne     = (ui_frame*)       FindChildByID( IDC_BIG_LEADERBOARD_FRAME_ONE    );
    m_pFrameTwo     = (ui_frame*)       FindChildByID( IDC_BIG_LEADERBOARD_FRAME_TWO    );
    m_pFrameMain    = (ui_frame*)       FindChildByID( IDC_BIG_LEADERBOARD_FRAME_MAIN   );

    // intialize frames
    m_pFrameOne->SetBackgroundColor( xcolor(0,0,0,0) );
    m_pFrameOne->ChangeElement("frame2");

    m_pFrameTwo->SetBackgroundColor( xcolor(0,0,0,0) );
    m_pFrameTwo->ChangeElement("frame2");

    m_pFrameMain->SetBackgroundColor( xcolor(39,117,28,64) );
    m_pFrameMain->ChangeElement("frame2");

    // initialize boxes
    m_pPlayerOneBox->SetFlag(ui_win::WF_VISIBLE, TRUE); 
    m_pPlayerOneBox->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pPlayerOneBox->SetHasTitleBar( TRUE );
    m_pPlayerOneBox->SetLabelColor( xcolor(255,252,204,255) );
    m_pPlayerOneBox->SetTitleBarColor( xcolor(19,59,14,196) );
       
    m_pPlayerTwoBox->SetFlag(ui_win::WF_VISIBLE, TRUE);
    m_pPlayerTwoBox->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pPlayerTwoBox->SetHasTitleBar( TRUE );
    m_pPlayerTwoBox->SetLabelColor( xcolor(255,252,204,255) );
    m_pPlayerTwoBox->SetTitleBarColor( xcolor(19,59,14,196) );

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
    navText += g_StringTableMgr( "ui", "IDS_NAV_CYCLE_DATA" );
  
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

    // Initialize team data
    m_PlayerTotals[0].ClearAll();
    m_PlayerTotals[1].ClearAll();

    // Initialize player data
    s32 i;
    for( i=0 ; i<NET_MAX_PLAYERS ; i++ )
    {
        m_PlayerData[i].ClearAll();
    }

    // initialize selected data display type
    m_SelectedType = TYPE_NONE;
    CycleStatDisplay( TRUE );

    // determine wether this is a team game
    const game_score& ScoreData = GameMgr.GetScore();

    if( ScoreData.IsTeamBased )
    {
        // set number of teams
        m_NumTeams = 2;
        
        // resize frame1
        irect temp1 = m_pFrameOne->GetPosition();
#ifdef TARGET_PS2
        if( x_GetTerritory() != XL_TERRITORY_AMERICA )
        {
            temp1.t -= 28;
            temp1.b += 22;
        }
        else
#endif
        {
            temp1.t -= 26;
            temp1.b += 22;
        }
        m_pFrameOne->SetPosition( temp1 );
        
        // resize frame2
        temp1 = m_pFrameTwo->GetPosition();
#ifdef TARGET_PS2
        if( x_GetTerritory() != XL_TERRITORY_AMERICA )
        {
            temp1.t -= 28;
            temp1.b += 22;
        }
        else
#endif
        {
            temp1.t -= 26;
            temp1.b += 22;
        }
        m_pFrameTwo->SetPosition( temp1 );

        // resize main frame
        temp1 = m_pFrameMain->GetPosition();
        temp1.t -= 24;
        temp1.b += 20;
        m_pFrameMain->SetPosition( temp1 );
    }
    else
    {
        m_NumTeams = 1;
    }

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

void dlg_big_leaderboard::Destroy( void )
{
    dlg_mp_score::Destroy();
}

//=========================================================================

void dlg_big_leaderboard::Render( s32 ox, s32 oy )
{
	irect rb;

    if( (GetFlags() & ui_win::WF_VISIBLE)==0 )
    {
        return;
    }

    s32 XRes, YRes;
    eng_GetRes(XRes, YRes);
#ifdef TARGET_PS2
    // Nasty hack to force PS2 to draw to rb.l = 0
    rb.Set( -1, 0, XRes, YRes );
#else
    rb.Set( 0, 0, XRes, YRes );
#endif
    // render background rect
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

void dlg_big_leaderboard::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    (void)pWin;
    (void)Presses;
    (void)Repeats;
    (void)WrapX;
    (void)WrapY;


    // Which way are we moving
    switch( Code )
    {
        case ui_manager::NAV_LEFT:
            CycleStatDisplay( FALSE );
            break;
        case ui_manager::NAV_RIGHT:
            CycleStatDisplay( TRUE );
            break;
    }
}

//=========================================================================

void dlg_big_leaderboard::OnPadDelete( ui_win* pWin )
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

void dlg_big_leaderboard::OnPadSelect( ui_win* pWin )
{
    (void)pWin;
    if( m_State == DIALOG_STATE_ACTIVE )
    {
        // goto the pause menu
        m_State = DIALOG_STATE_SELECT;
    }
}

//=========================================================================

void dlg_big_leaderboard::OnUpdate ( ui_win* pWin, f32 DeltaTime )
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
// Returns -1 if item 1 > item 2
// Returns +1 if item 1 < item 2
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
//=========================================================================

void dlg_big_leaderboard::FillScoreList( void )
{
    s32         i;
    s32         Count = 0;
    s32         TeamCount[2];

    // zero team totals
    m_PlayerTotals[0].ClearScore();
    m_PlayerTotals[1].ClearScore();

    // get player data
    const game_score& ScoreData = GameMgr.GetScore();
    TeamCount[0] =  0;
    TeamCount[1] = 16;

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

    // put the servers in the list
    for( i=0 ; i<NET_MAX_PLAYERS ; i++ )
    {
        if( ScoreData.Player[i].IsInGame )
        {
            // get the team
            s32 Team = 0;
            if( ScoreData.IsTeamBased )
            {
                Team = ScoreData.Player[i].Team;                
            }
            ASSERT( IN_RANGE( 0, Team, 1 ) );

            // This leaderboard is simply not capable of dealing with more 
            // than 16 players on a team.  Rather than overwrite data or 
            // otherwise destroy data, we are just going to ignore this
            // player.  Hopefully the teams will balance back out shortly.
            if( ScoreData.IsTeamBased )
            {
                if( (Team == 0) && (TeamCount[0] >= 16) )
                    continue;

                if( (Team == 1) && ((TeamCount[1]-16) >= 16) )
                    continue;
            }

            x_memcpy( &m_PlayerData[TeamCount[Team]], &ScoreData.Player[i], sizeof(player_score) );

            // is this player me?
            if( i == MySlot )
            {
                // it's me
                m_PlayerData[TeamCount[Team]].ClientIndex = 1;
            }
            else
            {
                // not me!
                m_PlayerData[TeamCount[Team]].ClientIndex = 0;
            }

            // does this player need to be highlighted
            if( ScoreData.HighLightMask & (1 << i) )
            {
                // NOTE: I am overloading the music level field to indicate
                // if the player is highlighted or not.
                // Music level is an unused variable for the leaderboard and
                // since I am making a local copy of the data, this will have
                // no ill effects.
                m_PlayerData[TeamCount[Team]].MusicLevel = 1;
            }
            else
            {
                // not highlighted
                m_PlayerData[TeamCount[Team]].MusicLevel = 0;
            }

            // Copy locally muted field.
            m_PlayerData[Count].IsVoiceAllowed = !g_VoiceMgr.GetLocallyMuted( i );

            m_PlayerTotals[Team].Score  += ScoreData.Player[i].Score;
            m_PlayerTotals[Team].Kills  += ScoreData.Player[i].Kills;
            m_PlayerTotals[Team].Deaths += ScoreData.Player[i].Deaths;
            m_PlayerTotals[Team].TKs    += ScoreData.Player[i].TKs;
            m_PlayerTotals[Team].Flags  += ScoreData.Player[i].Flags;
            m_PlayerTotals[Team].Votes  += ScoreData.Player[i].Votes;
            TeamCount[Team]++;

            Count++;
        }
    }

    // now we have the player data sort it
    if( Count )
    {
        if( m_NumTeams == 2 )// team game
        {
            // sort each of the team lists
            x_qsort( &m_PlayerData[ 0], TeamCount[0]   , sizeof(player_score), sort_compare );
            x_qsort( &m_PlayerData[16], TeamCount[1]-16, sizeof(player_score), sort_compare );
        }
        else
        {
            // sort the one list
            x_qsort( &m_PlayerData[0], NET_MAX_PLAYERS, sizeof(player_score), sort_compare );
        }
    }
}

//=========================================================================

void dlg_big_leaderboard::RenderScoreList( s32 ox, s32 oy )
{
    (void)ox;
    (void)oy;
    s32 i;
    irect r;
    irect r2;
    xcolor th( 19, 59, 14,255);
    xcolor ch(255,252,204,255);

    const game_score& ScoreData = GameMgr.GetScore();

    // If team based, render the team title bar
    if( m_NumTeams == 2 )
    {
        s32 Font = g_UiMgr->FindFont("large");

        // render team score bar
        r = GetPosition();
        r.l += 24;
        r.r = r.l + 216;
        r.t += 10;
        r.b = r.t + 26;

        r2 = GetPosition();
        r2.l += 256;
        r2.r = r2.l + 216;
        r2.t += 10;
        r2.b = r2.t + 26;

        m_pManager->RenderRect( r, xcolor(255,252,204,128), FALSE );
        m_pManager->RenderRect( r2, xcolor(255,252,204,128), FALSE );

        r.l += 5;
        r.r = r.l + 155;
        r.b -= 6;
        r2.l += 5;
        r2.r = r2.l + 155;
        r2.b -= 6;
        m_pManager->RenderText( Font, r, ui_font::h_left|ui_font::v_center|ui_font::clip_ellipsis, th, ScoreData.Team[0].NName );
        m_pManager->RenderText( Font, r2, ui_font::h_left|ui_font::v_center|ui_font::clip_ellipsis, th, ScoreData.Team[1].NName );

        r.l += 160;
        r.r = r.l + 45;
        r2.l += 160;
        r2.r = r2.l + 45;
        m_pManager->RenderText( Font, r, ui_font::h_right|ui_font::v_center, th, xfs("%d", ScoreData.Team[0].Score) );
        m_pManager->RenderText( Font, r2, ui_font::h_right|ui_font::v_center, th, xfs("%d", ScoreData.Team[1].Score) );
    }

    // render the main title bar
    r = GetPosition();
    r.l += 24 + 5 + 16;
    r.t += 36 + s_LineSpacing - 16;
    r.b = r.t + 16;

    r2 = GetPosition();
    r2.l += 256 + 5 + 16;
    r2.t += 36 + s_LineSpacing - 16;
    r2.b = r2.t + 16;
    m_pManager->RenderText( m_Font, r , ui_font::h_left|ui_font::v_center, ch, m_pStrPlayer );
    m_pManager->RenderText( m_Font, r2, ui_font::h_left|ui_font::v_center, ch, m_pStrPlayer );
    
    r.l  += 136  - 16;
    r.r   = r.l  + 30;
    r2.l += 136  - 16;
    r2.r  = r2.l + 30;
    m_pManager->RenderText( m_Font, r , ui_font::h_right|ui_font::v_center, ch, m_pStrScore );
    m_pManager->RenderText( m_Font, r2, ui_font::h_right|ui_font::v_center, ch, m_pStrScore );
    
    r.l  += 35;
    r.r  += 35;
    r2.l += 35;
    r2.r += 35;

    // render icon based on selected data
    switch( m_SelectedType )
    {
        case TYPE_KILLS:
            m_pManager->RenderText( m_Font, r,  ui_font::h_right|ui_font::v_center, ch, m_pStrIconKills );
            m_pManager->RenderText( m_Font, r2, ui_font::h_right|ui_font::v_center, ch, m_pStrIconKills );
            break;

        case TYPE_DEATHS:
            m_pManager->RenderText( m_Font, r,  ui_font::h_right|ui_font::v_center, ch, m_pStrIconDeaths );       
            m_pManager->RenderText( m_Font, r2, ui_font::h_right|ui_font::v_center, ch, m_pStrIconDeaths );      
            break;

        case TYPE_TKS:
            m_pManager->RenderText( m_Font, r,  ui_font::h_right|ui_font::v_center, ch, m_pStrIconTKs );       
            m_pManager->RenderText( m_Font, r2, ui_font::h_right|ui_font::v_center, ch, m_pStrIconTKs );      
            break;

        case TYPE_FLAGS:
            m_pManager->RenderText( m_Font, r,  ui_font::h_right|ui_font::v_center, ch, m_pStrIconFlags );       
            m_pManager->RenderText( m_Font, r2, ui_font::h_right|ui_font::v_center, ch, m_pStrIconFlags );      
            break;

        case TYPE_VOTES:
            m_pManager->RenderText( m_Font, r,  ui_font::h_right|ui_font::v_center, ch, m_pStrIconVotes );       
            m_pManager->RenderText( m_Font, r2, ui_font::h_right|ui_font::v_center, ch, m_pStrIconVotes );      
            break;

        case TYPE_NONE:
            break;

        default:
            ASSERTS(FALSE, "Data type out of range!");
            break;
    }

    // render bars
    r = GetPosition();
    r.l += 24;
    r.r = r.l + 214;
    r.t += 36 + 24;
    r.b = r.t + s_LineSpacing;
    for( i=0; i<NET_MAX_PLAYERS ; i++ )
    {
        // make last bar a little thicker
        if(( i == 15 ) || (i == 31))
        {
            r.b += 3;
        }

        // draw every other one
        if( i & 1 )
        {
            m_pManager->RenderRect( r, xcolor(20,58,14,64), FALSE );
        }

        // check for last one on the left
        if( i == 15 )
        {
            // move to the right
            r = GetPosition();
            r.l += 256;
            r.r = r.l + 214;
            r.t += 36 + 24;
            r.b = r.t + 16;
        }
        else
        {
            // move down a spot
            r.t += s_LineSpacing;
            r.b += s_LineSpacing;
        }
    }

    // render player data
    r = GetPosition();
    r.l += 24 + 5;
    r.r = r.l + 16;
    r.t += 36 + 22;
    r.b = r.t + 16;

    for( i=0; i<NET_MAX_PLAYERS ; i++ )
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
                hr.r = hr.l + 214;
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
            r.r = r.l + 131 - 16;
            m_pManager->RenderText( m_Font, r, ui_font::h_left|ui_font::v_center|ui_font::clip_ellipsis, c1, m_PlayerData[i].NName );
            
            r.l += 120;
            r.r = r.l + 30;
            m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[i].Score) );
            
            r.l += 35;
            r.r += 35;

            // render selected data
            switch( m_SelectedType )
            {
                case TYPE_KILLS:
                    m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[i].Kills) );
                    break;

                case TYPE_DEATHS:
                    m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[i].Deaths) );
                    break;

                case TYPE_TKS:
                    m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[i].TKs) );
                    break;

                case TYPE_FLAGS:
                    m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[i].Flags) );
                    break;

                case TYPE_VOTES:
                    m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[i].Votes) );
                    break;

                case TYPE_NONE:
                    break;

                default:
                    ASSERTS(FALSE, "Data type out of range!");
                    break;
            }

            r.l -= 171;

            if( i != 15 )
            {
                r.t += s_LineSpacing;
                r.b += s_LineSpacing;
            }

            r.r = r.l + 16;
        }

        if( i == 15 )
        {
            r = GetPosition();
            r.l += 256 + 5;
            r.t += 36 + 22;
            r.b = r.t + 16;
        }
    }

    // Render footer
    if( m_NumTeams == 2 )
    {
        r = GetPosition();
        r.t += ( 319 + ((s_LineSpacing - 16)*16) );
        r.b = r.t + 22;
        r.l += 24;
        r.r = r.l + 215;
        m_pManager->RenderRect( r, xcolor(19,59,14,196), FALSE );

        r.l += 232;
        r.r += 232;
        m_pManager->RenderRect( r, xcolor(19,59,14,196), FALSE );

        r = GetPosition();
        r.l += 24 + 5 + 16;
        r.t += ( 319 + ((s_LineSpacing - 16)*16) );
        r.b = r.t + 16;

        r2 = GetPosition();
        r2.l += 256 + 5 + 16;
        r2.t += ( 319 + ((s_LineSpacing - 16)*16) );
        r2.b = r2.t + 16;
//        m_pManager->RenderText( m_Font, r, ui_font::h_left|ui_font::v_center, ch, "Player Totals" );
//        m_pManager->RenderText( m_Font, r2, ui_font::h_left|ui_font::v_center, ch, "Player Totals" );
        
        r.l += 136 - 16;
        r.r = r.l + 30;
        r2.l += 136 - 16;
        r2.r = r2.l + 30;
        m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d", m_PlayerTotals[0].Score) );
        m_pManager->RenderText( m_Font, r2, ui_font::h_right|ui_font::v_center, ch, xfs("%d", m_PlayerTotals[1].Score) );
        
        r.l += 35;
        r.r += 35;
        r2.l += 35;
        r2.r += 35;

        // render selected data
        switch( m_SelectedType )
        {
            case TYPE_KILLS:
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[0].Kills) );
                m_pManager->RenderText( m_Font, r2, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[1].Kills) );
                break;

            case TYPE_DEATHS:
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[0].Deaths) );
                m_pManager->RenderText( m_Font, r2, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[1].Deaths) );
                break;

            case TYPE_TKS:
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[0].TKs) );
                m_pManager->RenderText( m_Font, r2, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[1].TKs) );
                break;

            case TYPE_FLAGS:
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[0].Flags) );
                m_pManager->RenderText( m_Font, r2, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[1].Flags) );
                break;

            case TYPE_VOTES:
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[0].Votes) );
                m_pManager->RenderText( m_Font, r2, ui_font::h_right|ui_font::v_center, ch, xfs("%d",m_PlayerTotals[1].Votes) );
                break;

            case TYPE_NONE:
                break;

            default:
                ASSERTS(FALSE, "Data type out of range!");
                break;
        }
    }
}

//=========================================================================

void dlg_big_leaderboard::CycleStatDisplay( xbool bForward )
{
    xbool Done = FALSE;
    s32   Count = 0;
    const game_score& ScoreData = GameMgr.GetScore();

    // check for no stats
    if( m_SelectedType == TYPE_NONE )
    {
        m_SelectedType = TYPE_KILLS;
    }

    // cycle display data backwards
    while( !Done )
    {
        Count++;

        if( bForward )
        {
            m_SelectedType++;

            if( m_SelectedType == NUM_DATA_TYPES )
            {
                // wrap
                m_SelectedType = TYPE_KILLS;
            }
        }
        else
        {
            m_SelectedType--;

            if( m_SelectedType < TYPE_KILLS )
            {
                // wrap
                m_SelectedType = NUM_DATA_TYPES - 1;
            }
        }

        switch( m_SelectedType )
        {
            case TYPE_KILLS:
                if( ScoreData.ScoreFieldMask & SCORE_KILLS )    
                    Done = TRUE; 
                break;
            case TYPE_DEATHS:
                if( ScoreData.ScoreFieldMask & SCORE_DEATHS )    
                    Done = TRUE; 
                break;
            case TYPE_TKS:
                if( ScoreData.ScoreFieldMask & SCORE_TKS )    
                    Done = TRUE; 
                break;
            case TYPE_FLAGS:
                if( ScoreData.ScoreFieldMask & SCORE_FLAGS )    
                    Done = TRUE; 
                break;
            case TYPE_VOTES:
                if( ScoreData.ScoreFieldMask & SCORE_VOTES )    
                    Done = TRUE; 
                break;
        }

        // check if we looped the whole list
        if( (Count == NUM_DATA_TYPES ) && ( !Done ) )
        {
            // nothing to display
            m_SelectedType = TYPE_NONE;
            Done = TRUE;
        }
    }
}

//=========================================================================
