//=========================================================================
//
//  dlg_PauseMain.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"

#include "dlg_PauseMain.hpp"
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "Configuration/GameConfig.hpp"

#ifdef CONFIG_VIEWER
#include "../../Apps/ArtistViewer/Config.hpp"
#else
#include "../../Apps/GameApp/Config.hpp"	
#endif

#ifndef CONFIG_RETAIL
#include "InputMgr\monkey.hpp"
#endif

// always display build info (for now!)
#ifndef CONFIG_RETAIL
#define DISPLAY_BUILD_INFO
#endif

#if defined( DISPLAY_DEBUG_INFO )
#include "objects\player.hpp"
#include "StateMgr/mapList.hpp"
#endif

extern          s32         g_Changelist;
extern  const   char*       g_pBuildDate;

//=========================================================================
//  Main Menu Dialog
//=========================================================================
static const s32 FrY = 220;     // Friends Y
static const s32 IcY = FrY + 9; // Friends Invite Icons Y

ui_manager::control_tem PauseMenuControls[] = 
{
#if CONFIG_IS_DEMO
    { IDC_PAUSE_MENU_RESUME,    "IDS_PAUSE_MENU_RESUME",    "button",   60,  70, 120, 40, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PAUSE_MENU_QUIT,      "IDS_PAUSE_MENU_QUIT",      "button",   60, 130, 120, 40, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PAUSE_MENU_INVERTY,   "IDS_PAUSE_MENU_INVERTY",   "button",   60, 190, 120, 40, 0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PAUSE_MENU_VIBRATION, "IDS_RUMBLE",               "button",   60, 250, 120, 40, 0, 3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PAUSE_MENU_NAV_TEXT,  "IDS_NULL",                 "text",      0,   0,   0,  0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#else
    // Frames.
    { IDC_PAUSE_MENU_RESUME,    "IDS_PAUSE_MENU_RESUME",    "button",   60,  60, 120, 40, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PAUSE_MENU_QUIT,      "IDS_PAUSE_MENU_QUIT",      "button",   60, 100, 120, 40, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PAUSE_MENU_OPTIONS,   "IDS_PAUSE_MENU_OPTIONS",   "button",   60, 140, 120, 40, 0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PAUSE_MENU_SETTINGS,  "IDS_PAUSE_MENU_SETTINGS",  "button",   60, 180, 120, 40, 0, 3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#ifdef TARGET_XBOX
    { IDC_PAUSE_MENU_FRIENDS,   "IDS_PAUSE_MENU_FRIENDS",   "button",   60, FrY, 120, 40, 0, 4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#endif
    { IDC_PAUSE_MENU_NAV_TEXT,  "IDS_NULL",                 "text",      0,   0,   0,  0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#endif

#if defined(DISPLAY_DEBUG_INFO)
    { IDC_PAUSE_MENU_INVERTY,   "IDS_NULL",                 "text",     60,  30, 120, 40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PAUSE_MENU_ZONE,      "IDS_NULL",                 "text",     60, 260, 120, 40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PAUSE_MENU_LEVEL,     "IDS_NULL",                 "text",     60, 280, 120, 40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#endif
#if defined(DISPLAY_BUILD_INFO)
    { IDC_PAUSE_MENU_CHANGE,    "IDS_NULL",                 "text",     60, 300, 120, 40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PAUSE_MENU_BUILDDATE, "IDS_NULL",                 "text",     60, 320, 120, 40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#endif

#if defined( TARGET_XBOX )
    { IDC_PAUSE_MENU_FRIEND_INV,  "IDS_NULL",             "bitmap",   34, IcY,  26, 26, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PAUSE_MENU_GAME_INV,    "IDS_NULL",             "bitmap",  182, IcY,  26, 26, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#endif
};


ui_manager::dialog_tem PauseMenuDialog =
{
    "IDS_PAUSE_MENU",
    1, 11,
    sizeof(PauseMenuControls)/sizeof(ui_manager::control_tem),
    &PauseMenuControls[0],
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

void dlg_pause_main_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "pause main", &PauseMenuDialog, &dlg_pause_main_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_pause_main_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_pause_main* pDialog = new dlg_pause_main;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_pause_main
//=========================================================================

dlg_pause_main::dlg_pause_main( void )
#if defined( TARGET_XBOX )
    : m_pFriendInvite   ( NULL ),
      m_pGameInvite     ( NULL )
#endif
{
}

//=========================================================================

dlg_pause_main::~dlg_pause_main( void )
{
    Destroy();
}

//=========================================================================
#if defined( DISPLAY_DEBUG_INFO )
static xwstring zoneText;
#endif 

xbool dlg_pause_main::Create( s32                        UserID,
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
    m_pButtonResume     = (ui_button*)  FindChildByID( IDC_PAUSE_MENU_RESUME    );
    m_pButtonQuit       = (ui_button*)  FindChildByID( IDC_PAUSE_MENU_QUIT      );
    m_pNavText          = (ui_text*)    FindChildByID( IDC_PAUSE_MENU_NAV_TEXT  );

#if defined( DISPLAY_DEBUG_INFO )
    m_pInvertYText      = (ui_text*)    FindChildByID( IDC_PAUSE_MENU_INVERTY   );
    m_pZoneText         = (ui_text*)    FindChildByID( IDC_PAUSE_MENU_ZONE      );
    m_pLevelText        = (ui_text*)    FindChildByID( IDC_PAUSE_MENU_LEVEL     );
#endif
#if defined(DISPLAY_BUILD_INFO)
    m_pChangeText       = (ui_text*)    FindChildByID( IDC_PAUSE_MENU_CHANGE    );
    m_pBuildText        = (ui_text*)    FindChildByID( IDC_PAUSE_MENU_BUILDDATE );
#endif

#if CONFIG_IS_DEMO
    m_pButtonInvert     = (ui_button*)  FindChildByID( IDC_PAUSE_MENU_INVERTY   );
    m_pButtonVibration  = (ui_button*)  FindChildByID( IDC_PAUSE_MENU_VIBRATION );
    ASSERT( m_pButtonInvert );
    ASSERT( m_pButtonVibration );
    m_pButtonInvert   ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pButtonVibration->SetFlag( ui_win::WF_VISIBLE, FALSE );
#else
    m_pButtonOptions    = (ui_button*)  FindChildByID( IDC_PAUSE_MENU_OPTIONS   );
    m_pButtonSettings   = (ui_button*)  FindChildByID( IDC_PAUSE_MENU_SETTINGS  );
#ifdef TARGET_XBOX
    m_pButtonFriends    = (ui_button*)  FindChildByID( IDC_PAUSE_MENU_FRIENDS   );
    m_pButtonFriends ->SetFlag(ui_win::WF_VISIBLE, FALSE);    
#endif
    m_pButtonOptions ->SetFlag(ui_win::WF_VISIBLE, FALSE);    
    m_pButtonSettings->SetFlag(ui_win::WF_VISIBLE, FALSE);    
#endif

#if defined( TARGET_XBOX )
    m_pFriendInvite     = (ui_bitmap*)  FindChildByID( IDC_PAUSE_MENU_FRIEND_INV  );
    m_pGameInvite       = (ui_bitmap*)  FindChildByID( IDC_PAUSE_MENU_GAME_INV    );

    if ( m_pFriendInvite )
    {
        m_pFriendInvite->SetFlag(ui_win::WF_VISIBLE, FALSE);
        m_pFriendInvite->SetBitmap( g_UiMgr->FindBitmap( "icon_friend_req_rcvd" ) );
    }
    if ( m_pGameInvite )
    {
        m_pGameInvite->SetFlag(ui_win::WF_VISIBLE, FALSE);
        m_pGameInvite->SetBitmap( g_UiMgr->FindBitmap( "icon_invite_rcvd"     ) );

    }
#endif


    //s32 iControl = g_StateMgr.GetCurrentControl();
    //if( (iControl == -1) || (GotoControl(iControl)==NULL) )
    //{
        GotoControl( (ui_control*)m_pButtonResume );
        m_CurrentControl = IDC_PAUSE_MENU_RESUME;
    //}
    //else
    //{
    //    m_CurrentControl = iControl;
    //}

    m_CurrHL = 0;
    m_PopUp = NULL;

    // switch off the buttons to start
    m_pButtonResume  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonQuit    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pNavText       ->SetFlag(ui_win::WF_VISIBLE, FALSE);
#if defined( DISPLAY_DEBUG_INFO )
    m_pInvertYText   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pZoneText      ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLevelText     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
#endif
#if defined(DISPLAY_BUILD_INFO)
    m_pChangeText    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pBuildText     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
#endif

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_UNPAUSE" );   
    m_pNavText->SetLabel( navText );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // set up debug info text
#if defined( DISPLAY_DEBUG_INFO )
    // invertY toggle
    player_profile& Profile = g_StateMgr.GetActiveProfile( 0 );
    if( Profile.m_bInvertY )
    {
        m_pInvertYText->SetLabel( xwstring( "Y-Axis INVERTED" ) );
    }
    else
    {
        m_pInvertYText->SetLabel( xwstring( "Y-Axis NORMAL" ) );
    }
    m_pInvertYText->SetLabelFlags( ui_font::h_center|ui_font::v_top );
    m_pInvertYText->UseSmallText(TRUE);

    // zone
    player* pPlayer = SMP_UTIL_GetActivePlayer();

    xstring zone = "";
    if( pPlayer )
    {
        vector3 pos = pPlayer->GetPosition();
        s32 playerZone = (s32)g_ZoneMgr.FindZone(pos);
        zone.Format("Zone: %d", playerZone);
    }
    zoneText = zone;
    m_pZoneText->SetLabel( zoneText );
    m_pZoneText->SetLabelFlags( ui_font::h_center|ui_font::v_top );
    m_pZoneText->UseSmallText(TRUE);

    // level name
    s32 levelID = g_ActiveConfig.GetLevelID();
    static xwstring levelText  = (const char *)xfs( "%s", g_MapList.GetDisplayName(levelID));
    m_pLevelText->SetLabel( levelText );
    m_pLevelText->SetLabelFlags( ui_font::h_center|ui_font::v_top );
    m_pLevelText->UseSmallText(TRUE);
#endif

#if defined(DISPLAY_BUILD_INFO)
    // change list 
    static xwstring changeText  = (const char *)xfs( "build: %d", g_Changelist );
    m_pChangeText->SetLabel( changeText );
    m_pChangeText->SetLabelFlags( ui_font::h_center|ui_font::v_top );
    m_pChangeText->UseSmallText(TRUE);

    // build date
    static xwstring buildText  = (const char *)xfs( "%s", g_pBuildDate );
    m_pBuildText->SetLabel( buildText );
    m_pBuildText->SetLabelFlags( ui_font::h_center|ui_font::v_top );
    m_pBuildText->UseSmallText(TRUE);
#endif

    // initialize screen scaling
    InitScreenScaling( Position );

    // set the frame to be disabled (if coming from off screen)
    if (g_UiMgr->IsScreenOn() == FALSE)
        SetFlag( WF_DISABLED, TRUE );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_pause_main::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_pause_main::Render( s32 ox, s32 oy )
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

void dlg_pause_main::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( pWin == (ui_win*)m_pButtonResume )
        {
            g_AudioMgr.Play("Select_Norm");
            g_UiMgr->DisableScreenHighlight();
            m_CurrentControl = IDC_PAUSE_MENU_RESUME;
            m_State = DIALOG_STATE_SELECT;
        }
        else if( pWin == (ui_win*)m_pButtonQuit )
        {
          
#ifndef CONFIG_RETAIL
            if ( g_MonkeyOptions.Enabled && g_MonkeyOptions.ModeEnabled[MONKEY_MENUMONKEY] )
                return;
#endif

            if( m_PopUp == NULL )
            {
                // Open a dialog to confirm quitting the online game component
                irect r = g_UiMgr->GetUserBounds( g_UiUserID );
                m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

                // get message text
                xwstring Message( g_StringTableMgr( "ui", "IDS_QUIT_DESTRUCTIVE_MSG" )); 

                // set nav text
                xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
                navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
                m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

                m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_QUIT_POPUP" ), TRUE, TRUE, FALSE, Message, navText, &m_PopUpResult );
            }
        }
#if CONFIG_IS_DEMO
        else if( pWin == m_pButtonInvert )
        {
            // toggle invert Y
            player_profile& Profile = g_StateMgr.GetActiveProfile(0);
            Profile.m_bInvertY = !Profile.m_bInvertY;
        }
        else if( pWin == m_pButtonVibration )
        {
            // toggle invert Y
            player_profile& Profile = g_StateMgr.GetActiveProfile(0);
            Profile.m_bVibration = !Profile.m_bVibration;
            input_EnableFeedback( Profile.m_bVibration );
        }
#else
        else if( pWin == (ui_win*)m_pButtonOptions )
        {
            g_StateMgr.InitPendingProfile( 0 );
            m_CurrentControl = IDC_PAUSE_MENU_OPTIONS;
            m_State = DIALOG_STATE_SELECT;
            g_AudioMgr.Play("Select_Norm");
        }
        else if( pWin ==(ui_win*)m_pButtonSettings )
        {
            m_CurrentControl = IDC_PAUSE_MENU_SETTINGS;
            m_State = DIALOG_STATE_SELECT;
            g_AudioMgr.Play("Select_Norm");
        }
#ifdef TARGET_XBOX
        else if( pWin == (ui_win*)m_pButtonFriends )
        {
            m_CurrentControl = IDC_PAUSE_MENU_FRIENDS;
            m_State = DIALOG_STATE_SELECT;
            g_AudioMgr.Play("Select_Norm");
        }
#endif
#endif
    }
}

//=========================================================================

void dlg_pause_main::OnPadBack( ui_win* pWin )
{
    (void)pWin;
}

//=========================================================================

void dlg_pause_main::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    s32 highLight = -1;

    //
    // Figure out if we have invitations to be friends or join games
    //
#if defined( TARGET_XBOX )
    xbool HaveFriendRequest     = FALSE;
    xbool HaveGameInvitation    = FALSE;
    if( g_MatchMgr.GetAuthStatus() == AUTH_STAT_CONNECTED )
    {
        s32 i;
        const s32 nBuddies = g_MatchMgr.GetBuddyCount();
        for ( i = 0; i < nBuddies; ++i )
        {
            buddy_info Buddy = g_MatchMgr.GetBuddy( i );
            if ( Buddy.Flags & USER_REQUEST_RECEIVED )
            {
                HaveFriendRequest = TRUE;
            }

            if ( Buddy.Flags & USER_HAS_INVITE )
            {
                HaveGameInvitation = TRUE;
            }

            if ( HaveGameInvitation && HaveFriendRequest )
            {
                // No need to search further, we're rendering both
                break;
            }
        }
    }
#endif // TARGET_XBOX

#if CONFIG_IS_DEMO
    player_profile& Profile = g_StateMgr.GetActiveProfile( 0 );

    xfs Vibration( "Vibration %s", Profile.m_bVibration?"Enabled":"Disabled" );
    xfs InvertY  ( "Invert Y %s", Profile.m_bInvertY?"Enabled":"Disabled" );

    m_pButtonVibration->SetLabel( (const xwchar*)xwstring( (const char*)Vibration ) );
    m_pButtonInvert->SetLabel( (const xwchar*)xwstring( (const char*)InvertY ) );

#endif

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // turn on the buttons
            m_pButtonResume     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonQuit       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText          ->SetFlag(ui_win::WF_VISIBLE, TRUE);    
#if CONFIG_IS_DEMO
            m_pButtonVibration  ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pButtonInvert     ->SetFlag( ui_win::WF_VISIBLE, TRUE );
#else
            m_pButtonOptions    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonSettings   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
#ifdef TARGET_XBOX
            m_pButtonFriends    ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            // disable friends list if we're not connected to Live.
            if( g_MatchMgr.GetAuthStatus() != AUTH_STAT_CONNECTED )
                m_pButtonFriends->SetFlag(ui_win::WF_DISABLED, TRUE);

            m_pFriendInvite     ->SetFlag(ui_win::WF_VISIBLE, HaveFriendRequest     );
            m_pGameInvite       ->SetFlag(ui_win::WF_VISIBLE, HaveGameInvitation    );
#endif
#endif

#if defined( DISPLAY_DEBUG_INFO )
            m_pInvertYText      ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pZoneText         ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pLevelText        ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            
            // update zone information (may have chnaged).
            player* pPlayer = SMP_UTIL_GetActivePlayer();
            xstring zone = "";
            if( pPlayer )
            {
                vector3 pos = pPlayer->GetPosition();
                s32 playerZone = (s32)g_ZoneMgr.FindZone(pos);
                zone.Format("Zone: %d", playerZone);
            }
            zoneText = zone;
            m_pZoneText->SetLabel( zoneText );
#endif
#if defined(DISPLAY_BUILD_INFO)
            m_pChangeText    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pBuildText     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
#endif
    
            //s32 iControl = g_StateMgr.GetCurrentControl();
            //if( (iControl == -1) || (GotoControl(iControl)==NULL) )
            //{
                GotoControl( (ui_control*)m_pButtonResume );
                m_pButtonResume->SetFlag(WF_HIGHLIGHT, TRUE);        
                g_UiMgr->SetScreenHighlight( m_pButtonResume->GetPosition() );
                m_CurrentControl =  IDC_PAUSE_MENU_RESUME;
            //}
            //else
            //{
            //    ui_control* pControl = GotoControl( iControl );
            //    pControl->SetFlag(WF_HIGHLIGHT, TRUE);
            //    g_UiMgr->SetScreenHighlight(pControl->GetPosition() );
            //    m_CurrentControl = iControl;
            //}

            if (g_UiMgr->IsScreenOn() == FALSE)
            {
                // enable the frame
                SetFlag( WF_DISABLED, FALSE );
                g_UiMgr->SetScreenOn(TRUE);
            }
        }
    }
#if defined( TARGET_XBOX )
    else
    {
        m_pFriendInvite     ->SetFlag(ui_win::WF_VISIBLE, HaveFriendRequest     );
        m_pGameInvite       ->SetFlag(ui_win::WF_VISIBLE, HaveGameInvitation    );
    }
#endif

    // check popup dialog
    if ( m_PopUp )
    {
        if ( m_PopUpResult != DLG_POPUP_IDLE )
        {
            if ( m_PopUpResult == DLG_POPUP_YES )
            {
                // quit game
                g_UiMgr->DisableScreenHighlight();
                m_CurrentControl = IDC_PAUSE_MENU_QUIT;
                m_State = DIALOG_STATE_SELECT;
            }

            // clear popup 
            m_PopUp = NULL;

            // turn on nav text
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    if( m_pButtonResume->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 0;
        g_UiMgr->SetScreenHighlight( m_pButtonResume->GetPosition() );
    }
    else if( m_pButtonQuit->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 1;
        g_UiMgr->SetScreenHighlight( m_pButtonQuit->GetPosition() );
    }
#if CONFIG_IS_DEMO
    else if( m_pButtonInvert->GetFlags( WF_HIGHLIGHT ) )
    {
        highLight = 2;
        g_UiMgr->SetScreenHighlight( m_pButtonInvert->GetPosition() );

    }
    else if( m_pButtonVibration->GetFlags( WF_HIGHLIGHT ) )
    {
        highLight = 3;
        g_UiMgr->SetScreenHighlight( m_pButtonVibration->GetPosition() );
    }
#else
    else if( m_pButtonOptions->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 2;
        g_UiMgr->SetScreenHighlight( m_pButtonOptions->GetPosition() );
    }
    else if( m_pButtonSettings->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 3;
        g_UiMgr->SetScreenHighlight( m_pButtonSettings->GetPosition() );
    }
#ifdef TARGET_XBOX
    else if( m_pButtonFriends->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 4;
        g_UiMgr->SetScreenHighlight( m_pButtonFriends->GetPosition() );
    }
#endif
#endif

    if( highLight != m_CurrHL )
    {
        if( highLight != -1 )
            g_AudioMgr.Play("Cusor_Norm");

        m_CurrHL = highLight;
    }
}

//=========================================================================
