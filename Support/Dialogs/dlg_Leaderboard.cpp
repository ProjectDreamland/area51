//=========================================================================
//
//  dlg_leaderboard.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_button.hpp"

#include "dlg_Leaderboard.hpp"
#include "dlg_TeamLeaderboard.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "NetworkMgr\Voice\VoiceMgr.hpp"

#ifdef CONFIG_VIEWER
#include "../../Apps/ArtistViewer/Config.hpp"
#else
#include "../../Apps/GameApp/Config.hpp"	
#endif

#ifndef CONFIG_RETAIL
#include "InputMgr\monkey.hpp"
#endif

//=========================================================================
//  Team Leaderboard Dialog
//=========================================================================

ui_manager::control_tem LeaderboardControls[] = 
{
    { IDC_LEADERBOARD_DETAILS,       "IDS_NULL",        "blankbox",   79,  36, 338, 283, 0, 0, 1, 1, ui_win::WF_VISIBLE|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_LEADERBOARD_FRAME_ONE,     "IDS_NULL",        "frame",      77,  34, 340, 285, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_LEADERBOARD_FRAME_MAIN,    "IDS_NULL",        "frame",      10,  24, 476, 309, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_LEADERBOARD_FRAME_TIMEOUT, "IDS_NULL",        "frame",     275, 345, 104,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_LEADERBOARD_TIMEOUT_TEXT,  "IDS_NULL",        "text",      135, 345, 104,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
#ifdef TARGET_XBOX
    { IDC_LEADERBOARD_LOADING_TEXT,  "IDS_NULL",        "text",      235, 395, 230,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_LEADERBOARD_LOADING_PIPS,  "IDS_NULL",        "text",      465, 395,  50,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_LEADERBOARD_NAV_TEXT,      "IDS_NULL",        "text",       25, 395, 200,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
#else
    { IDC_LEADERBOARD_LOADING_TEXT,  "IDS_NULL",        "text",      235, 395, 230,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_LEADERBOARD_LOADING_PIPS,  "IDS_NULL",        "text",      465, 395,  50,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_LEADERBOARD_NAV_TEXT,      "IDS_NULL",        "text",       25, 395, 200,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
#endif
};

ui_manager::dialog_tem LeaderboardDialog =
{
    "IDS_LEADERBOARD",
    1, 9,
    sizeof(LeaderboardControls)/sizeof(ui_manager::control_tem),
    &LeaderboardControls[0],
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

s32 s_LineSpacing = 16;

//=========================================================================
//  Registration function
//=========================================================================

void dlg_leaderboard_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "leaderboard", &LeaderboardDialog, &dlg_leaderboard_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_leaderboard_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_leaderboard* pDialog = new dlg_leaderboard;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_leaderboard
//=========================================================================

dlg_leaderboard::dlg_leaderboard( void )
{
#ifdef TARGET_PS2
    if( x_GetTerritory() != XL_TERRITORY_AMERICA )
    {
        s_LineSpacing = 19;
    }
#endif

#ifdef TARGET_XBOX
    m_XBOXNotificationOffsetX = 16;
    m_XBOXNotificationOffsetY = 44;
    m_bUseTopmost             = 1;
#endif
}

//=========================================================================

dlg_leaderboard::~dlg_leaderboard( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_leaderboard::Create( s32                        UserID,
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

    m_pPlayerBox	= (ui_blankbox*)    FindChildByID( IDC_LEADERBOARD_DETAILS       );
    m_pNavText      = (ui_text*)        FindChildByID( IDC_LEADERBOARD_NAV_TEXT      );
    m_pFrameOne     = (ui_frame*)       FindChildByID( IDC_LEADERBOARD_FRAME_ONE     );
    m_pFrameMain    = (ui_frame*)       FindChildByID( IDC_LEADERBOARD_FRAME_MAIN    );
    m_pFrameTimeOut = (ui_frame*)       FindChildByID( IDC_LEADERBOARD_FRAME_TIMEOUT );
    m_pLoadingText  = (ui_text*)        FindChildByID( IDC_LEADERBOARD_LOADING_TEXT  );
    m_pLoadingPips  = (ui_text*)        FindChildByID( IDC_LEADERBOARD_LOADING_PIPS  );

    // intialize frames
    m_pFrameOne->SetBackgroundColor( xcolor(0,0,0,0) );
    m_pFrameOne->ChangeElement("frame2");

    m_pFrameMain->SetBackgroundColor( xcolor(39,117,28,64) );
    m_pFrameMain->ChangeElement("frame2");

    m_pFrameTimeOut->SetBackgroundColor( xcolor(0,0,0,0) );
    m_pFrameTimeOut->ChangeElement("frame2");
    m_pFrameTimeOut->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // initialize player details box
    m_pPlayerBox->SetFlag(ui_win::WF_VISIBLE, TRUE); 
    m_pPlayerBox->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pPlayerBox->SetHasTitleBar( TRUE );
    m_pPlayerBox->SetLabelColor( xcolor(255,252,204,255) );
    m_pPlayerBox->SetTitleBarColor( xcolor(19,59,14,196) );      

    // intialize loading text
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
    m_pNavText->SetLabel( g_StringTableMgr( "ui", "IDS_NULL") );
    m_pNavText->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);
               
    // Grab a bunch of strings.
    m_pStrLevelDesc  = g_StringTableMgr( "ui", "IDS_NAV_LEVEL_DESC"  );
    m_pStrPlayer     = g_StringTableMgr( "ui", "IDS_HEADER_PLAYER"   );
    m_pStrScore      = g_StringTableMgr( "ui", "IDS_HEADER_SCORE"    );
    m_pStrIconKills  = g_StringTableMgr( "ui", "IDS_ICON_KILLS"      );
    m_pStrIconDeaths = g_StringTableMgr( "ui", "IDS_ICON_DEATHS"     );
    m_pStrIconTKs    = g_StringTableMgr( "ui", "IDS_ICON_TEAM_KILLS" );
    m_pStrIconVotes  = g_StringTableMgr( "ui", "IDS_ICON_VOTES"      );

    // disable the highlight
    g_UiMgr->DisableScreenHighlight();

    // Initialize player data
    for( s32 i=0; i<32; i++ )
    {
        m_PlayerData[i].ClearAll();
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

void dlg_leaderboard::Destroy( void )
{
    dlg_mp_score::Destroy();
}

//=========================================================================

void dlg_leaderboard::Render( s32 ox, s32 oy )
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

void dlg_leaderboard::OnPadDelete( ui_win* pWin )
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
#if !defined( TARGET_XBOX )
void dlg_leaderboard::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if( m_Mode != LEADERBOARD_PAUSE )
    {
        return;
    }
    if( m_State == DIALOG_STATE_ACTIVE )
    {
        // return to the game
        m_State = DIALOG_STATE_BACK;
    }
}
#endif // !TARGET_XBOX

//=========================================================================

void dlg_leaderboard::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if( m_State == DIALOG_STATE_ACTIVE )
    {
        // goto the pause menu 
        m_State = DIALOG_STATE_SELECT;
    }
}

//=========================================================================

void dlg_leaderboard::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // update list box
    FillScoreList();

    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->SetFlag( ui_win::WF_VISIBLE, TRUE );

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

void dlg_leaderboard::Configure( leaderboard_mode Mode )
{ 
    // nothing to do
    if( m_Mode == Mode )
        return;

    // let the base class do its thing
    dlg_mp_score::Configure( Mode );

    switch( m_Mode )
    {
    case LEADERBOARD_FINAL:
        break;
    case LEADERBOARD_INTERLEVEL:
        m_pNavText    ->SetLabelFlags( ui_font::h_left|ui_font::v_top );
        m_pNavText    ->SetFlag( ui_win::WF_VISIBLE, TRUE );
        m_pLoadingText->SetFlag( ui_win::WF_VISIBLE, TRUE );
        m_pLoadingPips->SetFlag( ui_win::WF_VISIBLE, TRUE );
        break;
    case LEADERBOARD_PAUSE:
        break;
    }
}

//=========================================================================
// Returns 0 if item 1 == item 2
// Returns -1 if item 1 < item 2
// Returns +1 if item 1 > item 2
//
static s32 sort_compare( const void* pItem1, const void* pItem2 )
{
    player_score* pScore1 = (player_score*)pItem1;
    player_score* pScore2 = (player_score*)pItem2;

    // sort by in game state
    if( pScore1->IsInGame > pScore2->IsInGame )
    {
        return -1;
    }
    else if( pScore1->IsInGame < pScore2->IsInGame )
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

void dlg_leaderboard::FillScoreList( void )
{
    s32         i;
    s32         Count = 0;

    // get player data
    const game_score& ScoreData = GameMgr.GetScore();

    // find out my player slot
    s32 MySlot;
    
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
    for( i=0 ; i<32 ; i++ )
    {
        if( ScoreData.Player[i].IsInGame )
        {
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

            // does this player need to be highlighted
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

            Count++;
        }
    }

    // now we have the player data sort it
    if( Count )
    {
        // sort the one list
        x_qsort( &m_PlayerData[0], 32, sizeof(player_score), sort_compare );
    }
}

//=========================================================================

void dlg_leaderboard::RenderScoreList( s32 ox, s32 oy )
{
    (void)ox;
    (void)oy;
    s32 i;
    irect r;
    xcolor ch(255,252,204,255);
    s32 PlayerSize = (s32)(274.0f* m_pManager->GetScaleX());  // size allowed to player before any data columns

    // get score data
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

    PlayerSize -= (s32)( ( ColumnCount * 35.0f )* m_pManager->GetScaleX());  // size allowed to player before any data columns

    // render the main title bar
    r = GetPosition();
    r.l += (s32)((79.0f + 5.0f)* m_pManager->GetScaleX());
    r.t += 36;
    r.b = r.t + 16;

#ifdef TARGET_PS2
    if( x_GetTerritory() != XL_TERRITORY_AMERICA )
    {
        r.t += 5;
        r.b += 5;
    }
#endif

    // leave space for speaker icon   
    r.l += 16;

    m_pManager->RenderText( m_Font, r, ui_font::h_left|ui_font::v_center, ch, m_pStrPlayer );
    
    r.l += PlayerSize + (s32)(5.0f * m_pManager->GetScaleX());
    r.r = r.l + (s32)( 30.0f * m_pManager->GetScaleX());

    if( ScoreData.ScoreFieldMask & SCORE_POINTS )
    {
        m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, ch, m_pStrScore );
    }
    
    if( ScoreData.ScoreFieldMask & SCORE_KILLS )
    {
        r.l += (s32)(35.0f * m_pManager->GetScaleX());
        r.r += (s32)(35.0f * m_pManager->GetScaleX());
        m_pManager->RenderText( m_Font, r,  ui_font::h_right|ui_font::v_center, ch, m_pStrIconKills );
    }

    if( ScoreData.ScoreFieldMask & SCORE_TKS )
    {
        r.l += (s32)(35.0f * m_pManager->GetScaleX());
        r.r += (s32)(35.0f * m_pManager->GetScaleX());
        m_pManager->RenderText( m_Font, r,  ui_font::h_right|ui_font::v_center, ch, m_pStrIconTKs );
    }

    if( ScoreData.ScoreFieldMask & SCORE_DEATHS )
    {
        r.l += (s32)(35.0f * m_pManager->GetScaleX());
        r.r += (s32)(35.0f * m_pManager->GetScaleX());
        m_pManager->RenderText( m_Font, r,  ui_font::h_right|ui_font::v_center, ch, m_pStrIconDeaths );       
    }

    if( ScoreData.ScoreFieldMask & SCORE_VOTES )
    {
        r.l += (s32)(35.0f * m_pManager->GetScaleX());
        r.r += (s32)(35.0f * m_pManager->GetScaleX());
        m_pManager->RenderText( m_Font, r,  ui_font::h_right|ui_font::v_center, ch, m_pStrIconVotes );       
    }

    // render bars
    r = GetPosition();
    r.l += (s32)(79.0f * m_pManager->GetScaleX());
    r.r = r.l + (s32)(336.0f * m_pManager->GetScaleX());
    r.t += 36 + 24;
    r.b = r.t + 16;
    for( i=0; i<16; i++ )
    {
        // make last bar a little thicker
        if( i == 15 )
        {
            r.b += 3;
        }

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
    r.l += (s32)((79.0f + 5.0f) * m_pManager->GetScaleX());
    r.r = r.l + 16;
#ifdef TARGET_PS2
    if( x_GetTerritory() != XL_TERRITORY_AMERICA )
    {
        r.t += 36 + 24;
    }
    else
#endif
    {
        r.t += 36 + 22;
    }

    r.b = r.t + 16;

    s32 StartL = r.l;
    s32 StartR = r.r;

    for( i=0; i<16; i++ )
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
                //c1 = XCOLOR_GREEN;
                irect hr = r;
                hr.l -= (s32)(5.0f * m_pManager->GetScaleX());
                hr.r = hr.l + (s32)(336.0f * m_pManager->GetScaleX());
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

            r.l += PlayerSize + (s32)(5.0f * m_pManager->GetScaleX());
            r.r = r.l + (s32)(30.0f * m_pManager->GetScaleX());

            if( ScoreData.ScoreFieldMask & SCORE_POINTS )
            {
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[i].Score) );
            }
            
            if( ScoreData.ScoreFieldMask & SCORE_KILLS )
            {
                r.l += (s32)(35.0f * m_pManager->GetScaleX());
                r.r += (s32)(35.0f * m_pManager->GetScaleX());
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[i].Kills) );
            }

            if( ScoreData.ScoreFieldMask & SCORE_TKS )
            {
                r.l += (s32)(35.0f * m_pManager->GetScaleX());
                r.r += (s32)(35.0f * m_pManager->GetScaleX());
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[i].TKs) );
            }

            if( ScoreData.ScoreFieldMask & SCORE_DEATHS )
            {
                r.l += (s32)(35.0f * m_pManager->GetScaleX());
                r.r += (s32)(35.0f * m_pManager->GetScaleX());
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[i].Deaths) );
            }

            if( ScoreData.ScoreFieldMask & SCORE_VOTES )
            {
                r.l += (s32)(35.0f * m_pManager->GetScaleX());
                r.r += (s32)(35.0f * m_pManager->GetScaleX());
                m_pManager->RenderText( m_Font, r, ui_font::h_right|ui_font::v_center, c1, xfs("%d",m_PlayerData[i].Votes) );
            }

            r.l = StartL; //-= (s32)(275.0f * m_pManager->GetScaleX());
            r.r = StartR; //r.l + PlayerSize;
            r.t += s_LineSpacing;
            r.b += s_LineSpacing;
        }
    }
}

//=========================================================================
