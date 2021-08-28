//=========================================================================
//
//  dlg_stats.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"

#include "dlg_Stats.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"



//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum controls
{
    IDC_STATS_TIME,
    IDC_STATS_KILLS,
    IDC_STATS_DEATHS,
    IDC_STATS_GAMES,
    IDC_STATS_WINS,
    IDC_STATS_GOLDS,
    IDC_STATS_SILVERS,
    IDC_STATS_BRONZES,
#ifndef TARGET_XBOX
    IDC_STATS_KICKS,
    IDC_STATS_VOTES,
#endif
    IDC_STATS_TIME_TEXT,
    IDC_STATS_KILLS_TEXT,
    IDC_STATS_DEATHS_TEXT,
    IDC_STATS_GAMES_TEXT,
    IDC_STATS_WINS_TEXT,
    IDC_STATS_GOLDS_TEXT,
    IDC_STATS_SILVERS_TEXT,
    IDC_STATS_BRONZES_TEXT,
#ifndef TARGET_XBOX
    IDC_STATS_KICKS_TEXT,
    IDC_STATS_VOTES_TEXT,
#endif
    IDC_STATS_NAV_TEXT,
};

ui_manager::control_tem StatsControls[] = 
{
    // Frames.
    { IDC_STATS_TIME_TEXT,      "IDS_NULL",                 "text",  40,  60, 180,  25, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_STATS_KILLS_TEXT,     "IDS_NULL",                 "text",  40,  85, 180,  25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_STATS_DEATHS_TEXT,    "IDS_NULL",                 "text",  40, 110, 180,  25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_STATS_GAMES_TEXT,     "IDS_NULL",                 "text",  40, 135, 180,  25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_STATS_WINS_TEXT,      "IDS_NULL",                 "text",  40, 160, 180,  25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_STATS_GOLDS_TEXT,     "IDS_NULL",                 "text",  40, 185, 180,  25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_STATS_SILVERS_TEXT,   "IDS_NULL",                 "text",  40, 210, 180,  25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_STATS_BRONZES_TEXT,   "IDS_NULL",                 "text",  40, 235, 180,  25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#ifndef TARGET_XBOX
    { IDC_STATS_KICKS_TEXT,     "IDS_NULL",                 "text",  40, 260, 180,  25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_STATS_VOTES_TEXT,     "IDS_NULL",                 "text",  40, 285, 180,  25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#endif

    { IDC_STATS_TIME,           "IDS_ONLINE_STATS_TIME",    "text", 240,  60, 180,  25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_STATS_KILLS,          "IDS_ONLINE_STATS_KILLS",   "text", 240,  85, 180,  25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_STATS_DEATHS,         "IDS_ONLINE_STATS_DEATHS",  "text", 240, 110, 180,  25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_STATS_GAMES,          "IDS_ONLINE_STATS_GAMES",   "text", 240, 135, 180,  25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_STATS_WINS,           "IDS_ONLINE_STATS_WINS",    "text", 240, 160, 180,  25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_STATS_GOLDS,          "IDS_ONLINE_STATS_GOLD",    "text", 240, 185, 180,  25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_STATS_SILVERS,        "IDS_ONLINE_STATS_SILVER",  "text", 240, 210, 180,  25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_STATS_BRONZES,        "IDS_ONLINE_STATS_BRONZE",  "text", 240, 235, 180,  25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#ifndef TARGET_XBOX
    { IDC_STATS_KICKS,          "IDS_ONLINE_STATS_KICKS",   "text", 240, 260, 180,  25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_STATS_VOTES,          "IDS_ONLINE_STATS_VOTES",   "text", 240, 285, 180,  25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#endif
  
    { IDC_STATS_NAV_TEXT,     "IDS_NULL",         "text",      0,   0,   0,  0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};

ui_manager::dialog_tem StatsDialog =
{
    "IDS_ONLINE_STATS",
    1, 9,
    sizeof(StatsControls)/sizeof(ui_manager::control_tem),
    &StatsControls[0],
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

void dlg_stats_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "stats", &StatsDialog, &dlg_stats_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_stats_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_stats* pDialog = new dlg_stats;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_level_select
//=========================================================================

dlg_stats::dlg_stats( void )
{
}

//=========================================================================

dlg_stats::~dlg_stats( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_stats::Create( s32                        UserID,
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

    // find controls
    m_pLabelTime    = (ui_text*)  FindChildByID( IDC_STATS_TIME    );
    m_pLabelKills   = (ui_text*)  FindChildByID( IDC_STATS_KILLS   );
    m_pLabelDeaths  = (ui_text*)  FindChildByID( IDC_STATS_DEATHS  );
    m_pLabelGames   = (ui_text*)  FindChildByID( IDC_STATS_GAMES   );
    m_pLabelWins    = (ui_text*)  FindChildByID( IDC_STATS_WINS    );
    m_pLabelGolds   = (ui_text*)  FindChildByID( IDC_STATS_GOLDS   );
    m_pLabelSilvers = (ui_text*)  FindChildByID( IDC_STATS_SILVERS );
    m_pLabelBronzes = (ui_text*)  FindChildByID( IDC_STATS_BRONZES );
#ifndef TARGET_XBOX
    m_pLabelKicks   = (ui_text*)  FindChildByID( IDC_STATS_KICKS   );
    m_pLabelVotes   = (ui_text*)  FindChildByID( IDC_STATS_VOTES   );
#endif
    
    m_pTextTime     = (ui_text*)  FindChildByID( IDC_STATS_TIME_TEXT    );
    m_pTextKills    = (ui_text*)  FindChildByID( IDC_STATS_KILLS_TEXT   );
    m_pTextDeaths   = (ui_text*)  FindChildByID( IDC_STATS_DEATHS_TEXT  );
    m_pTextGames    = (ui_text*)  FindChildByID( IDC_STATS_GAMES_TEXT   );
    m_pTextWins     = (ui_text*)  FindChildByID( IDC_STATS_WINS_TEXT    );
    m_pTextGolds    = (ui_text*)  FindChildByID( IDC_STATS_GOLDS_TEXT   );
    m_pTextSilvers  = (ui_text*)  FindChildByID( IDC_STATS_SILVERS_TEXT );
    m_pTextBronzes  = (ui_text*)  FindChildByID( IDC_STATS_BRONZES_TEXT );
#ifndef TARGET_XBOX
    m_pTextKicks    = (ui_text*)  FindChildByID( IDC_STATS_KICKS_TEXT   );
    m_pTextVotes    = (ui_text*)  FindChildByID( IDC_STATS_VOTES_TEXT   );
#endif

    m_pNavText      = (ui_text*)    FindChildByID( IDC_STATS_NAV_TEXT );

    // hide them
    m_pLabelTime    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLabelKills   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLabelDeaths  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLabelGames   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLabelWins    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLabelGolds   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLabelSilvers ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLabelBronzes ->SetFlag(ui_win::WF_VISIBLE, FALSE);
#ifndef TARGET_XBOX
    m_pLabelKicks   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLabelVotes   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
#endif

    m_pTextTime     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pTextKills    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pTextDeaths   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pTextGames    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pTextWins     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pTextGolds    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pTextSilvers  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pTextBronzes  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
#ifndef TARGET_XBOX
    m_pTextKicks    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pTextVotes    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
#endif
    m_pNavText      ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // set up text labels
    m_pLabelTime    ->UseSmallText( TRUE );
    m_pLabelKills   ->UseSmallText( TRUE );
    m_pLabelDeaths  ->UseSmallText( TRUE );
    m_pLabelGames   ->UseSmallText( TRUE );
    m_pLabelWins    ->UseSmallText( TRUE );
    m_pLabelGolds   ->UseSmallText( TRUE );
    m_pLabelSilvers ->UseSmallText( TRUE );
    m_pLabelBronzes ->UseSmallText( TRUE );
#ifndef TARGET_XBOX
    m_pLabelKicks   ->UseSmallText( TRUE );
    m_pLabelVotes   ->UseSmallText( TRUE );
#endif

    m_pLabelTime    ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pLabelKills   ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pLabelDeaths  ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pLabelGames   ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pLabelWins    ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pLabelGolds   ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pLabelSilvers ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pLabelBronzes ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
#ifndef TARGET_XBOX
    m_pLabelKicks   ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pLabelVotes   ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
#endif

    m_pLabelTime    ->SetLabelColor( xcolor(126,220,60,255) );
    m_pLabelKills   ->SetLabelColor( xcolor(126,220,60,255) );
    m_pLabelDeaths  ->SetLabelColor( xcolor(126,220,60,255) );
    m_pLabelGames   ->SetLabelColor( xcolor(126,220,60,255) );
    m_pLabelWins    ->SetLabelColor( xcolor(126,220,60,255) );
    m_pLabelGolds   ->SetLabelColor( xcolor(126,220,60,255) );
    m_pLabelSilvers ->SetLabelColor( xcolor(126,220,60,255) );
    m_pLabelBronzes ->SetLabelColor( xcolor(126,220,60,255) );
#ifndef TARGET_XBOX
    m_pLabelKicks   ->SetLabelColor( xcolor(126,220,60,255) );
    m_pLabelVotes   ->SetLabelColor( xcolor(126,220,60,255) );
#endif

    // set up text data
    m_pTextTime     ->UseSmallText( TRUE );
    m_pTextKills    ->UseSmallText( TRUE );
    m_pTextDeaths   ->UseSmallText( TRUE );
    m_pTextGames    ->UseSmallText( TRUE );
    m_pTextWins     ->UseSmallText( TRUE );
    m_pTextGolds    ->UseSmallText( TRUE );
    m_pTextSilvers  ->UseSmallText( TRUE );
    m_pTextBronzes  ->UseSmallText( TRUE );
#ifndef TARGET_XBOX
    m_pTextKicks    ->UseSmallText( TRUE );
    m_pTextVotes    ->UseSmallText( TRUE );
#endif
    m_pTextTime     ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pTextKills    ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pTextDeaths   ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pTextGames    ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pTextWins     ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pTextGolds    ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pTextSilvers  ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pTextBronzes  ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
#ifndef TARGET_XBOX
    m_pTextKicks    ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pTextVotes    ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
#endif

    m_pTextTime     ->SetLabelColor( xcolor(255,252,204,255) );
    m_pTextKills    ->SetLabelColor( xcolor(255,252,204,255) );
    m_pTextDeaths   ->SetLabelColor( xcolor(255,252,204,255) );
    m_pTextGames    ->SetLabelColor( xcolor(255,252,204,255) );
    m_pTextWins     ->SetLabelColor( xcolor(255,252,204,255) );
    m_pTextGolds    ->SetLabelColor( xcolor(255,252,204,255) );
    m_pTextSilvers  ->SetLabelColor( xcolor(255,252,204,255) );
    m_pTextBronzes  ->SetLabelColor( xcolor(255,252,204,255) );
#ifndef TARGET_XBOX
    m_pTextKicks    ->SetLabelColor( xcolor(255,252,204,255) );
    m_pTextVotes    ->SetLabelColor( xcolor(255,252,204,255) );
#endif

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_BACK" ));  
    m_pNavText->SetLabel( navText );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // Get stats data for player here!
    player_stats MyStats = g_MatchMgr.GetAllCareerStats();
    s32 Hours   = ( MyStats.PlayTime / 60 );
    s32 Minutes = ( MyStats.PlayTime % 60 );
    m_pTextTime     ->SetLabel( xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "IDS_ONLINE_STATS_HOURS_MINS" )), Hours, Minutes ) ) );
    m_pTextKills    ->SetLabel( xwstring( xfs( "%d", MyStats.KillsAsHuman ) ) );
    m_pTextDeaths   ->SetLabel( xwstring( xfs( "%d", MyStats.Deaths       ) ) );
    m_pTextGames    ->SetLabel( xwstring( xfs( "%d", MyStats.Games        ) ) );
    m_pTextWins     ->SetLabel( xwstring( xfs( "%d", MyStats.Wins         ) ) );
    m_pTextGolds    ->SetLabel( xwstring( xfs( "%d", MyStats.Gold         ) ) );
    m_pTextSilvers  ->SetLabel( xwstring( xfs( "%d", MyStats.Silver       ) ) );
    m_pTextBronzes  ->SetLabel( xwstring( xfs( "%d", MyStats.Bronze       ) ) );
#ifndef TARGET_XBOX
    m_pTextKicks    ->SetLabel( xwstring( xfs( "%d", MyStats.Kicks        ) ) );
    m_pTextVotes    ->SetLabel( xwstring( xfs( "%d", MyStats.VotesStarted ) ) );
#endif

    // Disable highlight
    g_UiMgr->DisableScreenHighlight();

    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

	// Return success code
    return Success;
}

//=========================================================================

void dlg_stats::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_stats::Render( s32 ox, s32 oy )
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

void dlg_stats::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        g_UiMgr->EnableScreenHighlight();
        g_AudioMgr.Play("Backup");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_stats::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // turn on the controls
            m_pLabelTime    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pLabelKills   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pLabelDeaths  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pLabelGames   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pLabelWins    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pLabelGolds   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pLabelSilvers ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pLabelBronzes ->SetFlag(ui_win::WF_VISIBLE, TRUE);
#ifndef TARGET_XBOX
            m_pLabelKicks   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pLabelVotes   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
#endif

            m_pTextTime     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pTextKills    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pTextDeaths   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pTextGames    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pTextWins     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pTextGolds    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pTextSilvers  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pTextBronzes  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
#ifndef TARGET_XBOX
            m_pTextKicks    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pTextVotes    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
#endif

            m_pNavText      ->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);
}

//=========================================================================



