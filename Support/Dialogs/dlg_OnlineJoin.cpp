//=========================================================================
//
//  dlg_online_join.cpp
//
//=========================================================================

#include "Entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_listbox.hpp"
#include "ui\ui_blankbox.hpp"
#include "ui\ui_joinlist.hpp"
#include "ui\ui_dlg_vkeyboard.hpp"
#include "ui\ui_bitmap.hpp"

#include "dlg_OnlineJoin.hpp"
#include "StateMgr\StateMgr.hpp"
#include "StringMgr\StringMgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "Parsing/textin.hpp"
#include "Configuration/GameConfig.hpp"

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
	IDC_MATCHLIST,
    IDC_SERVER_DETAILS,

    IDC_HEADSET_TEXT,
    IDC_FRIEND_TEXT,
    IDC_PASSWORD_TEXT,
    IDC_MUTATION_TEXT,
    IDC_PCTCOMPLETE_TEXT,
    IDC_CONNECTION_TEXT,

    IDC_INFO_HEADSET,
    IDC_INFO_FRIEND,
    IDC_INFO_PASSWORD,
    IDC_INFO_MUTATION,
    IDC_INFO_PCTCOMPLETE,
    IDC_INFO_CONNECTION,

    IDC_FILTER_BOX,
    IDC_SORT_BOX,
    IDC_JOIN_BOX,
    IDC_REFRESH_BOX,
    IDC_PLAYERLIST_BOX,

    IDC_STATUS_BOX,
    IDC_STATUS_TEXT,
    IDC_NAV_TEXT,
};

enum sort_keys
{
    SORT_BY_PING,
    SORT_BY_GAME_TYPE,
    SORT_BY_NUM_PLAYERS,
    SORT_BY_SERVER_NAME,
    SORT_BY_PW_PROTECTED,
    SORT_BY_BUDDIES,

    SORT_NUM_TYPES,         // Must be last!
};


ui_manager::control_tem OnlineJoinControls[] = 
{
    // Frames.
    { IDC_MATCHLIST,        "IDS_NULL",             "joinlist",    35,  40, 482, 190, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_SERVER_DETAILS,   "IDS_NULL",             "blankbox",    35, 238, 482,  76, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_FRIEND_TEXT,      "IDS_JOIN_FRIEND_TEXT", "text",        43, 260, 110,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_HEADSET_TEXT,     "IDS_JOIN_HEADSET",     "text",        43, 276, 110,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PASSWORD_TEXT,    "IDS_JOIN_PASSWORD",    "text",        43, 292, 110,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PCTCOMPLETE_TEXT, "IDS_JOIN_PCTCOMPLETE", "text",       209, 260, 110,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MUTATION_TEXT,    "IDS_JOIN_MUTATION",    "text",       209, 276, 110,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_CONNECTION_TEXT,  "IDS_JOIN_CONNECTION",  "text",       209, 292, 110,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_INFO_FRIEND,      "IDS_NULL",             "text",       155, 260,  55,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_INFO_HEADSET,     "IDS_NULL",             "text",       155, 276,  55,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_INFO_PASSWORD,    "IDS_NULL",             "text",       155, 292,  55,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_INFO_PCTCOMPLETE, "IDS_NULL",             "text",       321, 260, 180,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_INFO_MUTATION,    "IDS_NULL",             "text",       321, 276, 180,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_INFO_CONNECTION,  "IDS_NULL",             "text",       321, 292, 180,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_STATUS_BOX,       "IDS_NULL",             "bitmap",     176, 315, 200,  30, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_STATUS_TEXT,      "IDS_NULL",             "text",       241, 320,  90,  22, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
//  { IDC_STATUS_TEXT,      "IDS_NULL",             "text",       430, 242, 100,  22, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_NAV_TEXT,         "IDS_NULL",             "text",         0,   0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};

ui_manager::dialog_tem OnlineJoinDialog =
{
    "IDS_ONLINE_JOIN_TITLE",
    1, 9,
    sizeof(OnlineJoinControls)/sizeof(ui_manager::control_tem),
    &OnlineJoinControls[0],
    0
};

//=========================================================================
//  Defines
//=========================================================================
#if defined(X_DEBUG)
#define MAX_REFRESH_INTERVAL 2.0f
#else
#define MAX_REFRESH_INTERVAL 4.0f
#endif

//#define DUMMY_LIST      1

//=========================================================================
//  Structs
//=========================================================================

//=========================================================================
//  Data
//=========================================================================

static xbool    s_JoinPasswordEntered;
static xbool    s_JoinPasswordOk;
static s32      s_AutoServerIndex;
static s32      s_SortKey = SORT_BY_PING;

#ifdef DUMMY_LIST
static server_info DummyServer[10];
#endif


//=========================================================================
//  Registration function
//=========================================================================

void dlg_online_join_register( ui_manager* pManager )
{
#ifndef X_EDITOR
    switch( x_GetLocale() )
    {        
    case XL_LANG_FRENCH:
        {
            s32 Count = sizeof(OnlineJoinControls)/sizeof(ui_manager::control_tem);
            for( s32 i = 0; i < Count; i++ )
            {
                // move over the french label a bit... HACK!!
                if( OnlineJoinControls[i].ControlID == IDC_STATUS_TEXT )
                {
                    OnlineJoinControls[i].x = 193;
                    break;
                }
            }
        }
        break;

    default:  // ALL else    
        break;

    }
#endif
    pManager->RegisterDialogClass( "online join", &OnlineJoinDialog, &dlg_online_join_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_online_join_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_online_join* pDialog = new dlg_online_join;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_online_join
//=========================================================================

dlg_online_join::dlg_online_join( void )
{
}

//=========================================================================

dlg_online_join::~dlg_online_join( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_online_join::Create( s32                        UserID,
                             ui_manager*                pManager,
                             ui_manager::dialog_tem*    pDialogTem,
                             const irect&               Position,
                             ui_win*                    pParent,
                             s32                        Flags,
                             void*                      pUserData )
{
    xbool   Success = FALSE;

    m_LastCount   = -1;
    m_LastSortKey = SORT_BY_PING;

    (void)pUserData;

    ASSERT( pManager );

    // Do dialog creation
	Success = ui_dialog::Create( UserID, pManager, pDialogTem, Position, pParent, Flags );

    m_pMatchList     = (ui_listbox*)FindChildByID( IDC_MATCHLIST );
    
    GotoControl( (ui_control*)m_pMatchList );

    m_pMatchList->SetFlag(ui_win::WF_SELECTED, TRUE);
    m_pMatchList->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pMatchList->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pMatchList->EnableHeaderBar();
    m_pMatchList->SetHeaderBarColor( xcolor(19,59,14,196) );
    m_pMatchList->SetHeaderColor( xcolor(255,252,204,255) );
    m_pMatchList->SetExitOnSelect(FALSE);
    m_pMatchList->SetExitOnBack(TRUE);
    m_pMatchList->DisableFrame();
    m_CurrHL = 0;

    // set up nav text 
    m_pNavText = (ui_text*) FindChildByID( IDC_NAV_TEXT );
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_BACK" ));
    m_pNavText->SetLabel( navText );
    m_pNavText->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);    

    // set up status box
    m_pStatusBox = (ui_bitmap*) FindChildByID( IDC_STATUS_BOX );
    m_pStatusBox->SetBitmap( g_UiMgr->FindElement( "button_combo1" ), TRUE, 0 );
    m_pStatusBox->SetFlag( ui_win::WF_VISIBLE, FALSE );

    // set up status text
    m_pStatusText = (ui_text*) FindChildByID( IDC_STATUS_TEXT );
    m_pStatusText->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pStatusText->SetLabelFlags( ui_font::h_left|ui_font::v_top );
    m_pStatusText->UseSmallText(TRUE);    

    // get server details box
    m_pServerDetails = (ui_blankbox*)FindChildByID( IDC_SERVER_DETAILS );
    m_pServerDetails->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pServerDetails->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pServerDetails->SetHasTitleBar( TRUE );
    //m_pServerDetails->SetLabel( g_StringTableMgr( "ui", "IDS_SERVER_DETAILS" ) );
    m_pServerDetails->SetLabelColor( xcolor(255,252,204,255) );
    m_pServerDetails->SetTitleBarColor( xcolor(19,59,14,196) );

    // set up server info text
    m_pHeadset              = (ui_text*)FindChildByID( IDC_HEADSET_TEXT     );
    m_pFriend               = (ui_text*)FindChildByID( IDC_FRIEND_TEXT      );
    m_pPassword             = (ui_text*)FindChildByID( IDC_PASSWORD_TEXT    );
    m_pMutationMode         = (ui_text*)FindChildByID( IDC_MUTATION_TEXT    );
    m_pPctComplete          = (ui_text*)FindChildByID( IDC_PCTCOMPLETE_TEXT );
    m_pConnectionSpeed      = (ui_text*)FindChildByID( IDC_CONNECTION_TEXT  );

    m_pInfoHeadset          = (ui_text*)FindChildByID( IDC_INFO_HEADSET     );
    m_pInfoFriend           = (ui_text*)FindChildByID( IDC_INFO_FRIEND      );
    m_pInfoPassword         = (ui_text*)FindChildByID( IDC_INFO_PASSWORD    );
    m_pInfoMutationMode     = (ui_text*)FindChildByID( IDC_INFO_MUTATION    );
    m_pInfoPctComplete      = (ui_text*)FindChildByID( IDC_INFO_PCTCOMPLETE );
    m_pInfoConnectionSpeed  = (ui_text*)FindChildByID( IDC_INFO_CONNECTION  );


    m_pHeadset              ->UseSmallText( TRUE );
    m_pFriend               ->UseSmallText( TRUE );
    m_pPassword             ->UseSmallText( TRUE );
    m_pMutationMode         ->UseSmallText( TRUE );
    m_pConnectionSpeed      ->UseSmallText( TRUE );
    m_pPctComplete          ->UseSmallText( TRUE );
    m_pInfoHeadset          ->UseSmallText( TRUE );
    m_pInfoFriend           ->UseSmallText( TRUE );
    m_pInfoPassword         ->UseSmallText( TRUE );
    m_pInfoMutationMode     ->UseSmallText( TRUE );
    m_pInfoPctComplete      ->UseSmallText( TRUE );
    m_pInfoConnectionSpeed  ->UseSmallText( TRUE );

    m_pHeadset              ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pFriend               ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pPassword             ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pMutationMode         ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pConnectionSpeed      ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pPctComplete          ->SetLabelFlags( ui_font::h_right|ui_font::v_center );

    m_pInfoHeadset          ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pInfoFriend           ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pInfoPassword         ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pInfoMutationMode     ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pInfoPctComplete      ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pInfoConnectionSpeed  ->SetLabelFlags( ui_font::h_left|ui_font::v_center );

    m_pHeadset              ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pFriend               ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pPassword             ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pMutationMode         ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pConnectionSpeed      ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pPctComplete          ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pInfoHeadset          ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pInfoFriend           ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pInfoPassword         ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pInfoMutationMode     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pInfoPctComplete      ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pInfoConnectionSpeed  ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    m_pHeadset              ->SetLabelColor( xcolor(255,252,204,255) );
    m_pFriend               ->SetLabelColor( xcolor(255,252,204,255) );
    m_pPassword             ->SetLabelColor( xcolor(255,252,204,255) );
    m_pMutationMode         ->SetLabelColor( xcolor(255,252,204,255) );
    m_pConnectionSpeed      ->SetLabelColor( xcolor(255,252,204,255) );
    m_pPctComplete          ->SetLabelColor( xcolor(255,252,204,255) );
    m_pInfoHeadset          ->SetLabelColor( xcolor(255,252,204,255) );
    m_pInfoFriend           ->SetLabelColor( xcolor(255,252,204,255) );
    m_pInfoPassword         ->SetLabelColor( xcolor(255,252,204,255) );
    m_pInfoMutationMode     ->SetLabelColor( xcolor(255,252,204,255) );
    m_pInfoPctComplete      ->SetLabelColor( xcolor(255,252,204,255) );
    m_pInfoConnectionSpeed  ->SetLabelColor( xcolor(255,252,204,255) );

    m_AutoRefreshTimeout    = 6.0f;
    // populate initial server info
    PopulateServerInfo( NULL );



#ifdef DUMMY_LIST
    x_wstrcpy( DummyServer[0].Name, xwstring( "Test Server 1" ) );
    DummyServer[0].Flags            = SERVER_HAS_PASSWORD | SERVER_VOICE_ENABLED | SERVER_HAS_BUDDY;
    DummyServer[0].MaxPlayers       = 16;
    DummyServer[0].Players          = 15;
    DummyServer[0].MutationMode     = MUTATE_CHANGE_AT_WILL;
    DummyServer[0].PercentComplete  = 50;
    DummyServer[0].PingDelay        = 100.0f;
    x_wstrcpy( DummyServer[0].MissionName, xwstring( "CNH:Gazebo" ) );

    x_wstrcpy( DummyServer[1].Name, xwstring( "WWWWWWWiiiiiiii" ) );
    DummyServer[1].Flags            = SERVER_HAS_PASSWORD | SERVER_VOICE_ENABLED | SERVER_HAS_BUDDY;
    DummyServer[1].MaxPlayers       = 16;
    DummyServer[1].Players          = 15;
    DummyServer[1].MutationMode     = MUTATE_CHANGE_AT_WILL;
    DummyServer[1].PercentComplete  = 50;
    DummyServer[1].PingDelay        = 100.0f;
    x_wstrcpy( DummyServer[1].MissionName, xwstring( "TDM:Death Squared" ) );

    x_wstrcpy( DummyServer[2].Name, xwstring( "Technic Gravy 1" ) );
    DummyServer[2].Flags            = SERVER_HAS_PASSWORD | SERVER_VOICE_ENABLED | SERVER_HAS_BUDDY;
    DummyServer[2].MaxPlayers       = 16;
    DummyServer[2].Players          = 15;
    DummyServer[2].MutationMode     = MUTATE_CHANGE_AT_WILL;
    DummyServer[2].PercentComplete  = 50;
    DummyServer[2].PingDelay        = 100.0f;
    x_wstrcpy( DummyServer[2].MissionName, xwstring( "CTF:Close Quarters" ) );

    x_wstrcpy( DummyServer[3].Name, xwstring( "Sleepy Floyd" ) );
    DummyServer[3].Flags            = SERVER_HAS_PASSWORD | SERVER_VOICE_ENABLED | SERVER_HAS_BUDDY;
    DummyServer[3].MaxPlayers       = 16;
    DummyServer[3].Players          = 15;
    DummyServer[3].MutationMode     = MUTATE_CHANGE_AT_WILL;
    DummyServer[3].PercentComplete  = 50;
    DummyServer[3].PingDelay        = 100.0f;
    x_wstrcpy( DummyServer[3].MissionName, xwstring( "INF:Eight Track" ) );

    x_wstrcpy( DummyServer[4].Name, xwstring( "Pablo Picasso" ) );
    DummyServer[4].Flags            = SERVER_HAS_PASSWORD | SERVER_VOICE_ENABLED | SERVER_HAS_BUDDY;
    DummyServer[4].MaxPlayers       = 16;
    DummyServer[4].Players          = 15;
    DummyServer[4].MutationMode     = MUTATE_CHANGE_AT_WILL;
    DummyServer[4].PercentComplete  = 50;
    DummyServer[4].PingDelay        = 100.0f;
    x_wstrcpy( DummyServer[4].MissionName, xwstring( "DM:Two To Tango" ) );
#endif
    if( CONFIG_IS_AUTOCLIENT )
    {
        g_MatchMgr.SetFilter( GAME_MP, -1, -1, -1, -1, -1);
    }

    // 
    // Make the match manager start to acquire the list of servers
    //
    g_MatchMgr.StartAcquisition( ACQUIRE_SERVERS );
    m_RefreshLockedTimeout  = MAX_REFRESH_INTERVAL + x_frand(0,2);

    // fill the match list
    FillMatchList();

    // Initialize popup
    m_PopUp = NULL;

    if( !(CONFIG_IS_AUTOSERVER || CONFIG_IS_AUTOCLIENT) )
    {
        // clear in-use controller flags
        for( int i=0; i<MAX_LOCAL_PLAYERS; i++)
        {
            g_StateMgr.SetControllerRequested(i, FALSE);
        }
    }

    // initialize screen scaling
    InitScreenScaling( Position );

    s_JoinPasswordEntered   = FALSE;
    s_JoinPasswordOk        = FALSE;

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

	// Return success code
    return Success;
}

//=========================================================================

void dlg_online_join::Destroy( void )
{
    ui_dialog::Destroy();

    g_MatchMgr.SetState( MATCH_IDLE );
    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_online_join::Render( s32 ox, s32 oy )
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

    g_MatchMgr.LockLists();

    // The background reader code *may* have just got some more data
    // from the matchmaking service. So, to be careful, let's clear out
    // all the items in the list
#ifndef DUMMY_LIST
    if( g_MatchMgr.GetServerCount() < m_pMatchList->GetItemCount() )
    {
        m_pMatchList->DeleteAllItems();
    }
#endif
    // render the normal dialog stuff
    ui_dialog::Render( ox, oy );
    g_MatchMgr.UnlockLists();

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

void dlg_online_join::OnNotify ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData )
{
    (void)pWin;
    (void)pSender;
    (void)Command;
    (void)pData;
}

//=========================================================================

void dlg_online_join::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        // check for changing sort key
        switch( Code )
        {
            case ui_manager::NAV_LEFT:
                if( --s_SortKey < 0 )
                {
                    s_SortKey = SORT_NUM_TYPES - 1;
                }                
                break;

            case ui_manager::NAV_RIGHT:
                if( ++s_SortKey == SORT_NUM_TYPES )
                {
                    s_SortKey = 0;
                }                
                break;
        }
    }
}

//=========================================================================

void dlg_online_join::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if( CONFIG_IS_AUTOCLIENT )
        return;

    if( m_State == DIALOG_STATE_ACTIVE )
    {
        g_MatchMgr.LockLists();
        s32 Index = m_pMatchList->GetSelection();
        if( (Index >=0) && (Index < m_pMatchList->GetItemCount()) )
        {
            if( Index < g_MatchMgr.GetServerCount() )
            {
                // store the active controller
                g_StateMgr.SetActiveControllerID( g_UiMgr->GetActiveController() );
                g_StateMgr.SetControllerRequested( g_UiMgr->GetActiveController(), TRUE );

                g_PendingConfig.GetConfig() = *g_MatchMgr.GetServerInfo(m_pMatchList->GetSelectedItemData());
                m_State = DIALOG_STATE_SELECT;
            }
        }
        g_MatchMgr.UnlockLists();
    } 
}

//=========================================================================

void dlg_online_join::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    m_LastCount = -1;

    if ( CONFIG_IS_AUTOCLIENT )
        return;

    if( m_State == DIALOG_STATE_ACTIVE )
    {
        //g_AudioMgr.Play("OptionBack");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_online_join::OnPadDelete( ui_win* pWin )
{
    if ( CONFIG_IS_AUTOCLIENT )
        return;

    // Only allow refresh if we've already given it about 4 seconds.
    if( m_RefreshLockedTimeout <= 0.0f )
    {
        g_MatchMgr.StartAcquisition( ACQUIRE_SERVERS );
        m_RefreshLockedTimeout  = MAX_REFRESH_INTERVAL;
        m_LastCount = -1;
        FillMatchList();
    }

    (void)pWin;
}

//=========================================================================

void dlg_online_join::OnPadActivate( ui_win* pWin )
{
    (void)pWin;

#ifdef LAN_PARTY_BUILD
    return;
#else
    if ( CONFIG_IS_AUTOCLIENT )
        return;

    if( m_State == DIALOG_STATE_ACTIVE )
    {
        // if we have a valid server
        if( m_pMatchList->GetSelection() != -1 )
        {
            g_MatchMgr.LockLists();
            g_PendingConfig.GetConfig() = *g_MatchMgr.GetServerInfo(m_pMatchList->GetSelectedItemData());
            g_MatchMgr.UnlockLists();
            // goto players menu
            m_State = DIALOG_STATE_ACTIVATE;
        }
    }
#endif
}

//=========================================================================

void dlg_online_join::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // turn on the controls
            m_pMatchList->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pServerDetails->SetFlag(ui_win::WF_VISIBLE, TRUE);
            GotoControl( (ui_control*)m_pMatchList );
            g_UiMgr->SetScreenHighlight( m_pMatchList->GetPosition() );

            // activate text
            m_pHeadset              ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pFriend               ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pPassword             ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pMutationMode         ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pConnectionSpeed      ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pPctComplete          ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pInfoHeadset          ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pInfoFriend           ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pInfoPassword         ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pInfoMutationMode     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pInfoPctComplete      ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pInfoConnectionSpeed  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText              ->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    // update list box
    FillMatchList();

    if( m_RefreshLockedTimeout > 0.0f )
    {
        m_RefreshLockedTimeout -= DeltaTime;
    }

    // check for menu automation
    if ( CONFIG_IS_AUTOCLIENT )
    {   
        if ( (s_AutoServerIndex >=0) && (s_AutoServerIndex < m_pMatchList->GetItemCount()) )
        {
            g_MatchMgr.LockLists();
            const server_info* pInfo=g_MatchMgr.GetServerInfo(s_AutoServerIndex);
            // This is rather convoluted right now due to the way BecomeClient works.
            // It is currently synchronous and returns once the login response and map
            // request have been retrieved from the other server.
            g_PendingConfig.GetConfig() = *pInfo;
            g_MatchMgr.UnlockLists();
            m_State = DIALOG_STATE_SELECT;
        }
        else
        {
            m_AutoRefreshTimeout -= DeltaTime;
            if( m_AutoRefreshTimeout < 0.0f )
            {
                g_MatchMgr.StartAcquisition( ACQUIRE_SERVERS );
                FillMatchList();
                m_AutoRefreshTimeout = MAX_REFRESH_INTERVAL + x_frand(0,2);
            }
        }
    }    

    // update everything else
    ui_dialog::OnUpdate( pWin, DeltaTime );
}

//=========================================================================

static s32 CompareFn( const server_info* pItem1, 
                      const server_info* pItem2, 
                      const s32          Key,
                      const s32          Key2,
                      const s32          Key3,
                      const s32          Key4 )
{
    s32 StrCmp = 0;

    switch( Key )
    {
    case SORT_BY_PING:
        if( pItem1->PingDelay > pItem2->PingDelay ) return(  1 );
        if( pItem1->PingDelay < pItem2->PingDelay ) return( -1 );
        break;

    case SORT_BY_GAME_TYPE:
        StrCmp = x_wstricmp( pItem1->ShortGameType, pItem2->ShortGameType );
        if( StrCmp ) 
            return( StrCmp );
        StrCmp = x_wstricmp( pItem1->MissionName,   pItem2->MissionName   );
        if( StrCmp )
            return( StrCmp );
        break;

    case SORT_BY_NUM_PLAYERS:
        if( pItem1->Players    > pItem2->Players    ) return(  1 );
        if( pItem1->Players    < pItem2->Players    ) return( -1 );
        if( pItem1->MaxPlayers > pItem2->MaxPlayers ) return(  1 );
        if( pItem1->MaxPlayers < pItem2->MaxPlayers ) return( -1 );
        break;

    case SORT_BY_SERVER_NAME:
        StrCmp = x_wstricmp( pItem1->Name, pItem2->Name );
        if( StrCmp )
            return( StrCmp );
        break;

    case SORT_BY_PW_PROTECTED:
        if( (pItem1->Flags & SERVER_HAS_PASSWORD) > 
            (pItem2->Flags & SERVER_HAS_PASSWORD) )     return(  1 );

        if( (pItem1->Flags & SERVER_HAS_PASSWORD) < 
            (pItem2->Flags & SERVER_HAS_PASSWORD) )     return( -1 );

        break;

    case SORT_BY_BUDDIES:
        if( (pItem1->Flags & SERVER_HAS_BUDDY) > 
            (pItem2->Flags & SERVER_HAS_BUDDY) )        return( -1 );

        if( (pItem1->Flags & SERVER_HAS_BUDDY) < 
            (pItem2->Flags & SERVER_HAS_BUDDY) )        return(  1 );

        break;

    case -1:
    default:
        // This is the last ditch effort to relate these two items.
        if( pItem1->ID                 > pItem2->ID                 )   return(  1 );
        if( pItem1->ID                 < pItem2->ID                 )   return( -1 );
        if( pItem1->External.GetIP()   > pItem2->External.GetIP()   )   return(  1 );
        if( pItem1->External.GetIP()   < pItem2->External.GetIP()   )   return( -1 );
        if( pItem1->External.GetPort() > pItem2->External.GetPort() )   return(  1 );
        if( pItem1->External.GetPort() < pItem2->External.GetPort() )   return( -1 );
        ASSERT( FALSE );
        return( 1 );
    }

    // The Key was unable to differentiate between the two items.
    // Recurse with the next key.
    return( CompareFn( pItem1, pItem2, Key2, Key3, Key4, -1 ) );
}

//=========================================================================
// Returns +1 if item 1 >  item 2
// Returns  0 if item 1 == item 2
// Returns -1 if item 1 <  item 2

#ifndef DUMMY_LIST

static s32 CompareFn( const void* item1, const void* item2 )
{
    const server_info* pItem1 = g_MatchMgr.GetServerInfo(*(s32*)item1);
    const server_info* pItem2 = g_MatchMgr.GetServerInfo(*(s32*)item2);

    switch( s_SortKey ) 
    {   // ----------------------------------- Key1 ........... Key2 ................ Key3 ............... Key4
        case SORT_BY_PING:
            return( CompareFn( pItem1, pItem2, SORT_BY_BUDDIES, SORT_BY_PING,         SORT_BY_SERVER_NAME, -1 ) );
            break;

        case SORT_BY_GAME_TYPE:
            return( CompareFn( pItem1, pItem2, SORT_BY_BUDDIES, SORT_BY_GAME_TYPE,    SORT_BY_SERVER_NAME, -1 ) );
            break;

        case SORT_BY_NUM_PLAYERS:
            return( CompareFn( pItem1, pItem2, SORT_BY_BUDDIES, SORT_BY_NUM_PLAYERS,  SORT_BY_SERVER_NAME, -1 ) );
            break;

        case SORT_BY_SERVER_NAME:
            return( CompareFn( pItem1, pItem2, SORT_BY_BUDDIES, SORT_BY_SERVER_NAME,  SORT_BY_GAME_TYPE,   -1 ) );
            break;

        case SORT_BY_PW_PROTECTED:
            return( CompareFn( pItem1, pItem2, SORT_BY_BUDDIES, SORT_BY_PW_PROTECTED, SORT_BY_SERVER_NAME, -1 ) );
            break;

        case SORT_BY_BUDDIES:
            return( CompareFn( pItem1, pItem2, SORT_BY_BUDDIES, SORT_BY_SERVER_NAME,  SORT_BY_GAME_TYPE,   -1 ) );
            break;

        default:
            ASSERT( FALSE );    // Undefined sort key.
            break;
    }

    ASSERT( FALSE );
    return( 1 );
}

#endif

//=========================================================================

void dlg_online_join::FillMatchList( void )
{
#ifdef DUMMY_LIST
    // Clear the matchlist
    m_pMatchList->DeleteAllItems();

    for( s32 i=0; i<5; i++ )
    {
        m_pMatchList->AddItem( (const char*)xstring(DummyServer[i].Name),  (s32)&DummyServer[i] );
    }

#else
    s32         iSel        = m_pMatchList->GetSelection();
//  s32         GameType    = m_pGameType->GetSelectedItemData();
    s32         i;
    s32         Count;

    static s32  DotCount = 0;

    g_MatchMgr.LockLists();
    // remember which server we are looking at in the list
    const server_info* pCurrServer = NULL;
    if( (iSel>=0) && (iSel<g_MatchMgr.GetServerCount()) )
    {
        pCurrServer = g_MatchMgr.GetServerInfo(m_pMatchList->GetSelectedItemData());
    }

    // Get the current server count
    Count = g_MatchMgr.GetServerCount();

    if( ((Count != m_LastCount) || (s_SortKey != m_LastSortKey)) )
    {
        m_LastCount         = Count;
        m_LastSortKey       = s_SortKey;

        // Clear the matchlist
        m_pMatchList->DeleteAllItems();
        
        // Invalidate the automated server index
        s_AutoServerIndex = -1;

        // clear the sort list
        m_SortList.Clear();

        // put the servers in the sort list
        for( i=0 ; i<Count ; i++ )
        {
            if( !(g_MatchMgr.GetServerInfo( i )->Flags & SERVER_IS_PRIVATE) )
            {
                LOG_MESSAGE( "dlg_online_join::FillMatchList", "Appending serverlist entry %d to sortlist.", i );
                m_SortList.Append( i );
            }
        }
        
        // sort the list
        if( m_SortList.GetCount() )
        {
            x_qsort( m_SortList.GetPtr(), m_SortList.GetCount(), sizeof(s32), CompareFn );
        }
        
        // copy the sorted list into the match list for display
        for( i=0; i<m_SortList.GetCount(); i++ )
        {
            const server_info* pInfo = g_MatchMgr.GetServerInfo(m_SortList[i]);
        
            if( (pInfo->Flags & SERVER_IS_PRIVATE)==0 )
            {
                // Is this the server we were previously highlighting
                if( pCurrServer == pInfo )
                {
                    // store the new index
                    iSel = i;
                }
                m_pMatchList->AddItem( pInfo->Name, m_SortList[i] );
#if defined(TARGET_DEV)
                if ( ( CONFIG_IS_AUTOCLIENT ) && ( s_AutoServerIndex == -1 ) )
                {
                    extern const char* DEFAULT_NAME;
                    // check for OUR server (server name matched devkit server name)
                    if( x_wstricmp( pInfo->Name, xwstring(DEFAULT_NAME) ) == 0 )
                    {
                        // store the index to our server
                        s_AutoServerIndex = m_SortList[i];
                    }
                } 
#endif
            }
        }
    }

    // Limit Selection
    iSel = MINMAX( 0, iSel, m_pMatchList->GetItemCount()-1 );

    if( m_pMatchList->GetItemCount() != 0 )
    {
        // Set Selection
        m_pMatchList->SetSelection( iSel );
        PopulateServerInfo(	g_MatchMgr.GetServerInfo(m_pMatchList->GetSelectedItemData()) );
    }
    else
    {
        PopulateServerInfo( NULL );
    }  

    xwstring navText;

    if( Count > 0 )
    {
        // Add sort-by to the navtext
        switch( s_SortKey )
        {
            case SORT_BY_PING:
                navText += g_StringTableMgr( "ui", "IDS_NAV_SORT_BY_PING" );
                break;
            case SORT_BY_GAME_TYPE:
                navText += g_StringTableMgr( "ui", "IDS_NAV_SORT_BY_GAME_TYPE" );
                break;
            case SORT_BY_NUM_PLAYERS:
                navText += g_StringTableMgr( "ui", "IDS_NAV_SORT_BY_NUM_PLAYERS" );
                break;
            case SORT_BY_SERVER_NAME:
                navText += g_StringTableMgr( "ui", "IDS_NAV_SORT_BY_SERVER_NAME" );
                break;
            case SORT_BY_PW_PROTECTED:
                navText += g_StringTableMgr( "ui", "IDS_NAV_SORT_BY_PW_PROTECTED" );
                break;
            case SORT_BY_BUDDIES:
                navText += g_StringTableMgr( "ui", "IDS_NAV_SORT_BY_BUDDIES" );
                break;
            default:
                ASSERTS( FALSE, "Undefined sort key!" );
                navText += g_StringTableMgr( "ui", "IDS_NAV_SORT_BY_PING" );
                break;
        }
#ifndef LAN_PARTY_BUILD
        navText += g_StringTableMgr( "ui", "IDS_NAV_PLAYER_LIST" );
#endif
        navText += g_StringTableMgr( "ui", "IDS_NAV_JOIN" );
    }

    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );

    // This means we can refresh, so we add the refresh icon first to the help text
    if( m_RefreshLockedTimeout <= 0.0f )
    {
        navText += g_StringTableMgr( "ui", "IDS_NAV_REFRESH" );
    }

    if( g_MatchMgr.IsAcquireComplete() )
    {
        DotCount = 0;
        m_pStatusText->SetLabel( g_StringTableMgr( "ui", "IDS_NULL" ) );
        m_pStatusText->SetFlag( ui_win::WF_VISIBLE, FALSE );
        m_pStatusBox ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    }
    else
    {
        if( m_pMatchList->GetFlags() & ui_win::WF_VISIBLE )
        {
            m_pStatusText->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pStatusBox ->SetFlag( ui_win::WF_VISIBLE, TRUE );
        }

        xwstring statusText( g_StringTableMgr( "ui", "IDS_ONLINE_JOIN_SEARCHING" ) );

        
        for( s32 dot=0; dot<DotCount; dot++ )
        {
            if( ( dot & 3 ) == 0 )
            {
                statusText += xwstring( "." );
            }
        }

        if( ++DotCount > 15 )
            DotCount = 0;

        m_pStatusText->SetLabel( statusText );
    }

    m_pNavText->SetLabel( navText );

    g_MatchMgr.UnlockLists();
#endif
}

//=========================================================================

void dlg_online_join::PopulateServerInfo(const server_info *pServerInfo)
{
    xcolor responsecolor;
    xwchar buff[128];

    x_wstrcpy(buff, g_StringTableMgr( "ui", "IDS_SERVER_DETAILS" ));

    if (pServerInfo)
    {
        x_wstrcat( buff, xwstring(" ") );
        xwstring temp = pServerInfo->Name;
        x_wstrcat( buff, temp );

        //m_pInfoGameType->SetLabel( pServerInfo->GameType );

        if( pServerInfo->Flags & SERVER_VOICE_ENABLED )
        {
            m_pInfoHeadset->SetLabel( g_StringTableMgr( "ui", "IDS_YES" ) );
        }
        else
        {
            m_pInfoHeadset->SetLabel( g_StringTableMgr( "ui", "IDS_NO" ) );
        }

        if( pServerInfo->Flags & SERVER_HAS_BUDDY )
        {
            m_pInfoFriend->SetLabel( g_StringTableMgr( "ui", "IDS_YES" ) );
        }
        else
        {
            m_pInfoFriend->SetLabel( g_StringTableMgr( "ui", "IDS_NO" ) );
        }
        
        if( pServerInfo->Flags & SERVER_HAS_PASSWORD )
        {
            m_pInfoPassword->SetLabel( g_StringTableMgr( "ui", "IDS_YES" ) );
        }
        else
        {
            m_pInfoPassword->SetLabel( g_StringTableMgr( "ui", "IDS_NO" ) );
        }

        m_pInfoPctComplete->SetLabel((xwstring)xfs("%d%%",pServerInfo->PercentComplete));

        s32 response;
        response = g_UiMgr->PingToColor( pServerInfo->PingDelay, responsecolor );
        m_pInfoConnectionSpeed->SetLabel( xwstring( "||||||||||" + response ) );
        m_pInfoConnectionSpeed->SetLabelColor( responsecolor );

        // set mutation text based on server settings
        switch( pServerInfo->MutationMode )
        {
            case MUTATE_CHANGE_AT_WILL:     m_pInfoMutationMode->SetLabel( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_CHANGE_AT_WILL"  ) );  break;
            case MUTATE_HUMAN_LOCK:         m_pInfoMutationMode->SetLabel( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_LOCK"      ) );  break;
            case MUTATE_MUTANT_LOCK:        m_pInfoMutationMode->SetLabel( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_LOCK"     ) );  break;
            case MUTATE_HUMAN_VS_MUTANT:    m_pInfoMutationMode->SetLabel( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_VS_MUTANT" ) );  break;
            case MUTATE_MAN_HUNT:           m_pInfoMutationMode->SetLabel( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MAN_HUNT"        ) );  break;
            case MUTATE_MUTANT_HUNT:        m_pInfoMutationMode->SetLabel( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_HUNT"     ) );  break;
        }       
    }
    else
    {
        m_pInfoHeadset          ->SetLabel( xwstring("---") );
        m_pInfoFriend           ->SetLabel( xwstring("---") );
        m_pInfoPassword         ->SetLabel( xwstring("---") );
        m_pInfoMutationMode     ->SetLabel( xwstring("---") );
        m_pInfoPctComplete      ->SetLabel( xwstring("---") );
        m_pInfoConnectionSpeed  ->SetLabel( xwstring("---") );
    }

    m_pServerDetails->SetLabel( buff );
}
//=========================================================================
