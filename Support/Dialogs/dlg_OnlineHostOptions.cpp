//=========================================================================
//
//  dlg_online_host_options.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_textbox.hpp"
#include "ui\ui_edit.hpp"

#include "dlg_OnlineHostOptions.hpp"
#include "dlg_popup.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Configuration/GameConfig.hpp"

//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum controls
{
    IDC_ONLINE_HOST_NUM_PLAYERS,
    IDC_ONLINE_HOST_SCORE,
    IDC_ONLINE_HOST_TIME_LIMIT,
    IDC_ONLINE_HOST_VOTE,
    IDC_ONLINE_HOST_MAP_SCALING,
    IDC_ONLINE_HOST_FFIRE,

    IDC_ONLINE_HOST_PLAYER_SELECTOR,
    IDC_ONLINE_HOST_SCORE_SELECTOR,
    IDC_ONLINE_HOST_TIME_SELECTOR,
    IDC_ONLINE_HOST_VOTE_SELECTOR,
    IDC_ONLINE_HOST_SCALING_SELECTOR,
    IDC_ONLINE_HOST_FFIRE_SELECTOR,
    IDC_ONLINE_HOST_CONTINUE,

    IDC_ONLINE_HOST_NAV_TEXT,
};

ui_manager::control_tem OnlineHostOptionsControls[] = 
{
    // Frames.
    { IDC_ONLINE_HOST_NUM_PLAYERS,          "IDS_HOST_NUM_PLAYERS",     "text",    40,  40, 220, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_SCORE,                "IDS_HOST_SCORE",           "text",    40,  75, 220, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_TIME_LIMIT,           "IDS_HOST_TIME_LIMIT",      "text",    40, 110, 220, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_VOTE,                 "IDS_HOST_VOTE_PASS",       "text",    40, 145, 220, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_MAP_SCALING,          "IDS_HOST_MAP_SCALING",     "text",    40, 180, 220, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_FFIRE,                "IDS_HOST_FRIENDLY_FIRE",   "text",    40, 215, 220, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_ONLINE_HOST_PLAYER_SELECTOR,      "IDS_NULL",                 "combo",  300,  49, 150, 40,  0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_SCORE_SELECTOR,       "IDS_NULL",                 "combo",  300,  84, 150, 40,  0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_TIME_SELECTOR,        "IDS_NULL",                 "combo",  300, 119, 150, 40,  0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_VOTE_SELECTOR,        "IDS_NULL",                 "combo",  300, 154, 150, 40,  0, 3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_SCALING_SELECTOR,     "IDS_NULL",                 "combo",  300, 189, 150, 40,  0, 4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_FFIRE_SELECTOR,       "IDS_NULL",                 "combo",  300, 224, 150, 40,  0, 5, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_CONTINUE,             "IDS_HOST_CONTINUE",        "button",  40, 285, 220, 40,  0, 6, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_ONLINE_HOST_NAV_TEXT,             "IDS_NULL",                 "text",     0,   0,   0,  0,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem OnlineHostOptionsDialog =
{
    "IDS_ONLINE_HOST_TITLE",
    1, 9,
    sizeof(OnlineHostOptionsControls)/sizeof(ui_manager::control_tem),
    &OnlineHostOptionsControls[0],
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

void dlg_online_host_options_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "host options", &OnlineHostOptionsDialog, &dlg_online_host_options_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_online_host_options_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_online_host_options* pDialog = new dlg_online_host_options;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_online_host_options
//=========================================================================

dlg_online_host_options::dlg_online_host_options( void )
{
}

//=========================================================================

dlg_online_host_options::~dlg_online_host_options( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_online_host_options::Create( s32                        UserID,
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

    // get pending game settings
    host_settings& Settings = g_StateMgr.GetPendingSettings().GetHostSettings();

    // Do dialog creation
	Success = ui_dialog::Create( UserID, pManager, pDialogTem, Position, pParent, Flags );

    m_pNumPlayerText    = (ui_text*)FindChildByID( IDC_ONLINE_HOST_NUM_PLAYERS );
    m_pScoreText        = (ui_text*)FindChildByID( IDC_ONLINE_HOST_SCORE       );
    m_pTimeLimitText    = (ui_text*)FindChildByID( IDC_ONLINE_HOST_TIME_LIMIT  );
    m_pVoteText         = (ui_text*)FindChildByID( IDC_ONLINE_HOST_VOTE        );
    m_pMapScalingText   = (ui_text*)FindChildByID( IDC_ONLINE_HOST_MAP_SCALING );
    m_pFriendlyFireText = (ui_text*)FindChildByID( IDC_ONLINE_HOST_FFIRE       );
    m_pNavText          = (ui_text*)FindChildByID( IDC_ONLINE_HOST_NAV_TEXT    );

    m_pNumPlayerSelect  = (ui_combo*)FindChildByID( IDC_ONLINE_HOST_PLAYER_SELECTOR  );
    m_pScoreSelect      = (ui_combo*)FindChildByID( IDC_ONLINE_HOST_SCORE_SELECTOR   );
    m_pTimeSelect       = (ui_combo*)FindChildByID( IDC_ONLINE_HOST_TIME_SELECTOR    );
    m_pVoteSelect       = (ui_combo*)FindChildByID( IDC_ONLINE_HOST_VOTE_SELECTOR    );
    m_pMapScalingSelect = (ui_combo*)FindChildByID( IDC_ONLINE_HOST_SCALING_SELECTOR );
    m_pFireSelect       = (ui_combo*)FindChildByID( IDC_ONLINE_HOST_FFIRE_SELECTOR   );

    m_pContinueButton   = (ui_button*)FindChildByID( IDC_ONLINE_HOST_CONTINUE );

    // set some text flags
    m_pNumPlayerText    ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pScoreText        ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pTimeLimitText    ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pVoteText         ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pMapScalingText   ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pFriendlyFireText ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
   
    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
    m_pNavText->SetLabel( navText );
    m_pNavText->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);    

    // set up num player selector 
    m_pNumPlayerSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT );
    s32 i;
    for( i = 2; i <= 16; i++ )
    {
        m_pNumPlayerSelect->AddItem( (const char*)xfs("%d",i), i );
    }
    /*
    m_pNumPlayerSelect->AddItem  (  "2",   2 );
    m_pNumPlayerSelect->AddItem  (  "3",   3 );
    m_pNumPlayerSelect->AddItem  (  "4",   4 );
    m_pNumPlayerSelect->AddItem  (  "5",   5 );
    m_pNumPlayerSelect->AddItem  (  "6",   6 );
    m_pNumPlayerSelect->AddItem  (  "7",   7 );
    m_pNumPlayerSelect->AddItem  (  "8",   8 );
    m_pNumPlayerSelect->AddItem  (  "9",   9 );
    m_pNumPlayerSelect->AddItem  ( "10",  10 );
    m_pNumPlayerSelect->AddItem  ( "11",  11 );
    m_pNumPlayerSelect->AddItem  ( "12",  12 );
    m_pNumPlayerSelect->AddItem  ( "13",  13 );
    m_pNumPlayerSelect->AddItem  ( "14",  14 );
    m_pNumPlayerSelect->AddItem  ( "15",  15 );
    m_pNumPlayerSelect->AddItem  ( "16",  16 );
//  m_pNumPlayerSelect->AddItem  ( "32",  32 );
    */

    // set up score selector
    m_pScoreSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT );
    {
        s32 Type = Settings.m_GameTypeID;
        s32 Limit[6][16] = 
        {
        /*DM  0*/  {   5,  10,  15,  20,  25,   30,   35,   40,   45,   50,   -1,   -1,   -1,   -1,   -1, -1 },
        /*TDM 1*/  {  10,  20,  30,  40,  50,   60,   70,   80,   90,  100,  110,  120,  130,  140,  150, -1 },
        /*CTF 2*/  {   1,   2,   3,   4,   5,    6,    7,    8,    9,   10,   -1,   -1,   -1,   -1,   -1, -1 },
        /*TAG 3*/  {  10,  20,  30,  40,  50,   60,   70,   80,   90,  100,  110,  120,  130,  140,  150, -1 },
        /*INF 4*/  {  50,  60,  70,  80,  90,  100,  120,  140,  160,  180,  200,   -1,   -1,   -1,   -1, -1 },
        /*CNH 5*/  { 500, 600, 700, 800, 900, 1000, 1200, 1400, 1600, 1800, 2000,   -1,   -1,   -1,   -1, -1 },
        };
        m_pScoreSelect->AddItem( g_StringTableMgr( "ui", "IDS_NO_LIMIT" ), -1 );
        for( s32 i = 0; Limit[Type][i] != -1; i++ )
        {
            m_pScoreSelect->AddItem( (const char*)xfs( "%d", Limit[Type][i] ), Limit[Type][i] );
        }
    }
    /*
    m_pScoreSelect->AddItem  (   "5",    5 );
    m_pScoreSelect->AddItem  (  "10",   10 );
    m_pScoreSelect->AddItem  (  "15",   15 );
    m_pScoreSelect->AddItem  (  "20",   20 );
    m_pScoreSelect->AddItem  (  "25",   25 );
    m_pScoreSelect->AddItem  (  "30",   30 );
    m_pScoreSelect->AddItem  (  "35",   35 );
    m_pScoreSelect->AddItem  (  "40",   40 );
    m_pScoreSelect->AddItem  (  "45",   45 );
    m_pScoreSelect->AddItem  (  "50",   50 );
    m_pScoreSelect->AddItem  (  "60",   60 );
    m_pScoreSelect->AddItem  (  "70",   70 );
    m_pScoreSelect->AddItem  (  "80",   80 );
    m_pScoreSelect->AddItem  (  "90",   90 );
    m_pScoreSelect->AddItem  ( "100",  100 );
    m_pScoreSelect->AddItem  ( "125",  125 );
    m_pScoreSelect->AddItem  ( "150",  150 );
    m_pScoreSelect->AddItem  ( "175",  175 );
    m_pScoreSelect->AddItem  ( "200",  200 );
    m_pScoreSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_NO_LIMIT" ), -1 );
    */

    // set up time selector
    m_pTimeSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT );
    m_pTimeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_NO_LIMIT" ), -1 );
    #ifndef CONFIG_RETAIL
    for( i = 1; i < 5; i++ )
    {
        m_pTimeSelect->AddItem( (const char*)xfs("%d:00",i), i * 60 );
    }
    #endif
    for( i = 5; i <= 60; i += 5 )
    {
        m_pTimeSelect->AddItem( (const char*)xfs("%d:00",i), i * 60 );
    }
    /*
    m_pTimeSelect->AddItem  (  "1:00",  1 * OneMin );
    m_pTimeSelect->AddItem  (  "2:00",  2 * OneMin );
    m_pTimeSelect->AddItem  (  "3:00",  3 * OneMin );
    m_pTimeSelect->AddItem  (  "4:00",  4 * OneMin );
    m_pTimeSelect->AddItem  (  "5:00",  5 * OneMin );
    m_pTimeSelect->AddItem  ( "10:00", 10 * OneMin );
    m_pTimeSelect->AddItem  ( "15:00", 15 * OneMin );
    m_pTimeSelect->AddItem  ( "20:00", 20 * OneMin );
    m_pTimeSelect->AddItem  ( "25:00", 25 * OneMin );
    m_pTimeSelect->AddItem  ( "30:00", 30 * OneMin );
    m_pTimeSelect->AddItem  ( "45:00", 45 * OneMin );
    m_pTimeSelect->AddItem  ( "60:00", 60 * OneMin );
    */

    // set up vote selector
    m_pVoteSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT );
    m_pVoteSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_VOTE_DISABLED" ), 0 );

    for( i = 5; i <= 100; i += 5 )
    {
        m_pVoteSelect->AddItem( (const char*)xfs("%d%%",i), i );
    }
    /*
    m_pVoteSelect->AddItem  (   "5%",   5 );
    m_pVoteSelect->AddItem  (  "10%",  10 );
    m_pVoteSelect->AddItem  (  "15%",  15 );
    m_pVoteSelect->AddItem  (  "20%",  20 );
    m_pVoteSelect->AddItem  (  "25%",  25 );
    m_pVoteSelect->AddItem  (  "30%",  30 );
    m_pVoteSelect->AddItem  (  "35%",  35 );
    m_pVoteSelect->AddItem  (  "40%",  40 );
    m_pVoteSelect->AddItem  (  "45%",  45 );
    m_pVoteSelect->AddItem  (  "50%",  50 );
    m_pVoteSelect->AddItem  (  "55%",  55 );
    m_pVoteSelect->AddItem  (  "60%",  60 );
    m_pVoteSelect->AddItem  (  "65%",  65 );
    m_pVoteSelect->AddItem  (  "70%",  70 );
    m_pVoteSelect->AddItem  (  "75%",  75 );
    m_pVoteSelect->AddItem  (  "80%",  80 );
    m_pVoteSelect->AddItem  (  "85%",  85 );
    m_pVoteSelect->AddItem  (  "90%",  90 );
    m_pVoteSelect->AddItem  (  "95%",  95 );
    m_pVoteSelect->AddItem  ( "100%", 100 );
    */

    // set up map scaling selector
    m_pMapScalingSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT );
    m_pMapScalingSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_OFF" ), 0 );
    m_pMapScalingSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_ON" ),  1 );
    
    // set up friendly fire selector
    m_pFireSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT );
    for( i = 0; i <= 100; i += 5 )
    {
        m_pFireSelect->AddItem( (const char*)xfs("%d%%",i), i );
    }
    /*
    m_pFireSelect->AddItem(   "0%",   0 );
    m_pFireSelect->AddItem(   "5%",   5 );
    m_pFireSelect->AddItem(  "10%",  10 );
    m_pFireSelect->AddItem(  "15%",  15 );
    m_pFireSelect->AddItem(  "20%",  20 );
    m_pFireSelect->AddItem(  "25%",  25 );
    m_pFireSelect->AddItem(  "30%",  30 );
    m_pFireSelect->AddItem(  "35%",  35 );
    m_pFireSelect->AddItem(  "40%",  40 );
    m_pFireSelect->AddItem(  "45%",  45 );
    m_pFireSelect->AddItem(  "50%",  50 );
    m_pFireSelect->AddItem(  "55%",  55 );
    m_pFireSelect->AddItem(  "60%",  60 );
    m_pFireSelect->AddItem(  "65%",  65 );
    m_pFireSelect->AddItem(  "70%",  70 );
    m_pFireSelect->AddItem(  "75%",  75 );
    m_pFireSelect->AddItem(  "80%",  80 );
    m_pFireSelect->AddItem(  "85%",  85 );
    m_pFireSelect->AddItem(  "90%",  90 );
    m_pFireSelect->AddItem(  "95%",  95 );
    m_pFireSelect->AddItem( "100%", 100 );
    */

    // switch off the buttons to start
    m_pNumPlayerText    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pScoreText        ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pTimeLimitText    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pVoteText         ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pMapScalingText   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pFriendlyFireText ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    m_pNumPlayerSelect  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pScoreSelect      ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pTimeSelect       ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pVoteSelect       ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pMapScalingSelect ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pFireSelect       ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pContinueButton   ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // set continue button alignment
    m_pContinueButton   ->SetFlag( ui_win::WF_BUTTON_LEFT, TRUE );

    // set initial highlight/control
    m_CurrHL = 5;
    m_pContinueButton->SetFlag(ui_win::WF_SELECTED, TRUE);
    GotoControl( (ui_control*)m_pContinueButton );

    // initialize num players
    s32 Sel = 0;
    for( s32 i=0; i< m_pNumPlayerSelect->GetItemCount(); i++ )
    {
        //if( g_PendingConfig.GetMaxPlayerCount() == m_pNumPlayerSelect->GetItemData( i, 0 ) )
        if( Settings.m_MaxPlayers == m_pNumPlayerSelect->GetItemData(i) )
        {
            Sel = i;
            break;
        }
    }
    m_pNumPlayerSelect->SetSelection( Sel );

    // initialize score select
    Sel = 0;
    for( s32 i=0; i<m_pScoreSelect->GetItemCount(); i++ )
    {    
        //if( g_PendingConfig.GetScoreLimit() == m_pScoreSelect->GetItemData( i, 0 ) )
        if( Settings.m_ScoreLimit == m_pScoreSelect->GetItemData(i) )
        {
            Sel = i;
            break;
        }
    }
    m_pScoreSelect->SetSelection( Sel );
   
    // initialize time limit
    Sel = 0;
    for( s32 i=0; i< m_pTimeSelect->GetItemCount(); i++ )
    {
        //if( g_PendingConfig.GetGameTime() == m_pTimeSelect->GetItemData( i, 0 ) )
        if( Settings.m_TimeLimit == m_pTimeSelect->GetItemData(i) )
        {
            Sel = i;
            break;
        }
    }
    m_pTimeSelect->SetSelection( Sel );

    // initialize vote pass percentage
    Sel = 0;
    for( s32 i=0; i< m_pVoteSelect->GetItemCount(); i++ )
    {
        //if( g_PendingConfig.GetVotePercent() == m_pVoteSelect->GetItemData( i, 0 ) )
        if( Settings.m_VotePassPct == m_pVoteSelect->GetItemData(i) )
        {
            Sel = i;
            break;
        }
    }
    m_pVoteSelect->SetSelection( Sel );

    // initialize map scaling
    if ( Settings.m_Flags & SERVER_ENABLE_MAP_SCALING )
    {
        m_pMapScalingSelect->SetSelection( 1 );
    }
    else
    {
        m_pMapScalingSelect->SetSelection( 0 );
    }

    // initialize friendly fire
    Sel = 0;
    for( s32 i=0; i<m_pFireSelect->GetItemCount(); i++ )
    {
        //if( g_PendingConfig.GetFirePercent() == m_pFireSelect->GetItemData( i, 0 ) )
        if( Settings.m_FFirePct == m_pFireSelect->GetItemData(i) )
        {
            Sel = i;
            break;
        }
    }
    m_pFireSelect->SetSelection( Sel );

    // update available options based on game type
    //switch( g_PendingConfig.GetGameTypeID() )
    switch( Settings.m_GameTypeID )
    {
        case GAME_DM:
            m_pFriendlyFireText ->SetFlag(ui_win::WF_DISABLED, TRUE);
            m_pFireSelect       ->SetFlag(ui_win::WF_DISABLED, TRUE);
            break;

        default:
            break;
    }

    // initialize controls
    m_CurrGameType    = -1;
    m_bRenderBlackout = FALSE;

    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_online_host_options::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_online_host_options::Render( s32 ox, s32 oy )
{
    static s32 offset   =  0;
    static s32 gap      =  9;
    static s32 width    =  4;

	irect rb;
    
    if( m_bRenderBlackout )
    {
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

    // render the glow bar
    g_UiMgr->RenderGlowBar();
}

//=========================================================================

void dlg_online_host_options::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
}

//=========================================================================

void dlg_online_host_options::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        g_AudioMgr.Play("Backup");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_online_host_options::OnPadSelect( ui_win* pWin )
{
    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( pWin == (ui_win*)m_pContinueButton )
        {
            g_AudioMgr.Play("Select_Norm");

            // launch the game!
            if ( m_State == DIALOG_STATE_ACTIVE )
            {
                // store game options 
                g_PendingConfig.SetMaxPlayerCount    ( m_pNumPlayerSelect  ->GetSelectedItemData() );
                g_PendingConfig.SetScoreLimit        ( m_pScoreSelect      ->GetSelectedItemData() );
                g_PendingConfig.SetGameTime          ( m_pTimeSelect       ->GetSelectedItemData() );
                g_PendingConfig.SetVotePercent       ( m_pVoteSelect       ->GetSelectedItemData() );
                g_PendingConfig.SetMapScalingEnabled ( m_pMapScalingSelect ->GetSelectedItemData() );
                g_PendingConfig.SetFirePercent       ( m_pFireSelect       ->GetSelectedItemData() );

                // update game settings
                host_settings& Settings = g_StateMgr.GetPendingSettings().GetHostSettings();

                Settings.m_MaxPlayers   = m_pNumPlayerSelect ->GetSelectedItemData();
                Settings.m_ScoreLimit   = m_pScoreSelect     ->GetSelectedItemData();
                Settings.m_TimeLimit    = m_pTimeSelect      ->GetSelectedItemData();
                Settings.m_VotePassPct  = m_pVoteSelect      ->GetSelectedItemData();
                Settings.m_FFirePct     = m_pFireSelect      ->GetSelectedItemData();

                if(m_pMapScalingSelect->GetSelectedItemData() == 1 )
                    Settings.m_Flags |= SERVER_ENABLE_MAP_SCALING;
                else
                    Settings.m_Flags &= ~SERVER_ENABLE_MAP_SCALING;

                g_AudioMgr.Play("Select_Norm");
                m_State = DIALOG_STATE_SELECT;
            }
        }
    }
}

//=========================================================================

void dlg_online_host_options::OnUpdate ( ui_win* pWin, f32 DeltaTime )
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
            m_pNumPlayerText    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pScoreText        ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pTimeLimitText    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pVoteText         ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pMapScalingText   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pFriendlyFireText ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText          ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            m_pNumPlayerSelect  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pScoreSelect      ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pTimeSelect       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pVoteSelect       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pMapScalingSelect ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pFireSelect       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pContinueButton   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
    
            GotoControl( (ui_control*)m_pContinueButton );
            m_pContinueButton->SetFlag(ui_win::WF_HIGHLIGHT, TRUE);
            g_UiMgr->SetScreenHighlight( m_pContinueButton->GetPosition() );
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    // update labels  
    if( m_pNumPlayerSelect->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 0;
        m_pNumPlayerText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pNumPlayerText->GetPosition() );
    }
    else
        m_pNumPlayerText->SetLabelColor( xcolor(126,220,60,255) );

    if( m_pScoreSelect->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 1;
        m_pScoreText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pScoreText->GetPosition() );
    }
    else
        m_pScoreText->SetLabelColor( xcolor(126,220,60,255) );

    if( m_pTimeSelect->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 2;
        m_pTimeLimitText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pTimeLimitText->GetPosition() );
    }
    else
        m_pTimeLimitText->SetLabelColor( xcolor(126,220,60,255) );

    if( m_pVoteSelect->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 3;
        m_pVoteText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pVoteText->GetPosition() );
    }
    else
        m_pVoteText->SetLabelColor( xcolor(126,220,60,255) );

    if( m_pMapScalingSelect->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 4;
        m_pMapScalingText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pMapScalingText->GetPosition() );
    }
    else
        m_pMapScalingText->SetLabelColor( xcolor(126,220,60,255) );

    if( m_pFriendlyFireText->GetFlags(WF_DISABLED) == FALSE )
    {
        if( m_pFireSelect->GetFlags(WF_HIGHLIGHT) )
        {
            highLight = 5;
            m_pFriendlyFireText->SetLabelColor( xcolor(255,252,204,255) );
            g_UiMgr->SetScreenHighlight( m_pFriendlyFireText->GetPosition() );
        }
        else
            m_pFriendlyFireText->SetLabelColor( xcolor(126,220,60,255) );
    }

    if( m_pContinueButton->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 6;
        g_UiMgr->SetScreenHighlight( m_pContinueButton->GetPosition() );
    }

    if( highLight != m_CurrHL )
    {
        if( highLight != -1 )
            g_AudioMgr.Play("Cusor_Norm");

        m_CurrHL = highLight;
    }
}

//=========================================================================

