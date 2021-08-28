//=========================================================================
//
//  dlg_PauseMP.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"

#include "dlg_PauseMP.hpp"
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"


#ifndef CONFIG_RETAIL
#include "InputMgr\monkey.hpp"
#endif

//=========================================================================
//  Main Menu Dialog
//=========================================================================
static const s32 FrY = 220;     // Friends Y
static const s32 IcY = FrY + 9; // Friends Invite Icons Y

ui_manager::control_tem PauseMPControls[] = 
{
    // Frames.
    { IDC_PAUSE_MP_QUIT,	    "IDS_PAUSE_MENU_QUIT",      "button",   60,  60, 120, 40, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PAUSE_MP_SCORE,       "IDS_PAUSE_MENU_SCORE",     "button",   60, 100, 120, 40, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PAUSE_MP_OPTIONS,     "IDS_PAUSE_MENU_OPTIONS",   "button",   60, 140, 120, 40, 0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PAUSE_MP_SETTINGS,    "IDS_PAUSE_MENU_SETTINGS",  "button",   60, 180, 120, 40, 0, 3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#ifdef TARGET_XBOX
    { IDC_PAUSE_MP_FRIENDS,     "IDS_PAUSE_MENU_FRIENDS",   "button",   60, FrY, 120, 40, 0, 4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#endif
    { IDC_PAUSE_MP_NAV_TEXT,    "IDS_NULL",                 "text",      0,   0,   0,  0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#if defined( TARGET_XBOX )
    { IDC_PAUSE_MP_FRIEND_INV,  "IDS_NULL",                         "bitmap",   34, IcY,  26, 26, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PAUSE_MP_GAME_INV,    "IDS_NULL",                         "bitmap",  182, IcY,  26, 26, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#endif
};


ui_manager::dialog_tem PauseMPDialog =
{
    "IDS_PAUSE_MENU",
    1, 9,
    sizeof(PauseMPControls)/sizeof(ui_manager::control_tem),
    &PauseMPControls[0],
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

void dlg_pause_mp_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "pause multiplayer", &PauseMPDialog, &dlg_pause_mp_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_pause_mp_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_pause_mp* pDialog = new dlg_pause_mp;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_pause_mp
//=========================================================================

dlg_pause_mp::dlg_pause_mp( void )
#if defined( TARGET_XBOX )
    : m_pFriendInvite   ( NULL ),
      m_pGameInvite     ( NULL )
#endif
{
}

//=========================================================================

dlg_pause_mp::~dlg_pause_mp( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_pause_mp::Create( s32                        UserID,
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
    m_pButtonQuit	    = (ui_button*)  FindChildByID( IDC_PAUSE_MP_QUIT     );
    m_pButtonScore 	    = (ui_button*)  FindChildByID( IDC_PAUSE_MP_SCORE    );
    m_pButtonOptions    = (ui_button*)  FindChildByID( IDC_PAUSE_MP_OPTIONS  );
    m_pButtonSettings   = (ui_button*)  FindChildByID( IDC_PAUSE_MP_SETTINGS );
#ifdef TARGET_XBOX
    m_pButtonFriends    = (ui_button*)  FindChildByID( IDC_PAUSE_MP_FRIENDS  );
#endif
    m_pNavText 	        = (ui_text*)    FindChildByID( IDC_PAUSE_MP_NAV_TEXT );

    s32 iControl = g_StateMgr.GetCurrentControl();
    if( (iControl == -1) || (GotoControl(iControl)==NULL) )
    {
        GotoControl( (ui_control*)m_pButtonQuit );
        m_CurrentControl = IDC_PAUSE_MP_QUIT;
    }
    else
    {
        m_CurrentControl = iControl;
    }

    m_CurrHL = 0;
    m_PopUp = NULL;

    // switch off the buttons to start
    m_pButtonQuit    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonScore   ->SetFlag(ui_win::WF_VISIBLE, FALSE);    
    m_pButtonOptions ->SetFlag(ui_win::WF_VISIBLE, FALSE);    
    m_pButtonSettings->SetFlag(ui_win::WF_VISIBLE, FALSE);    
#ifdef TARGET_XBOX
    m_pButtonFriends ->SetFlag(ui_win::WF_VISIBLE, FALSE);    
#endif
    m_pNavText       ->SetFlag(ui_win::WF_VISIBLE, FALSE);

#if defined( TARGET_XBOX )
    m_pFriendInvite     = (ui_bitmap*)  FindChildByID( IDC_PAUSE_MP_FRIEND_INV  );
    m_pGameInvite       = (ui_bitmap*)  FindChildByID( IDC_PAUSE_MP_GAME_INV    );

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


    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_UNPAUSE" );   
    m_pNavText->SetLabel( navText );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

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

void dlg_pause_mp::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_pause_mp::Render( s32 ox, s32 oy )
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

void dlg_pause_mp::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( pWin == (ui_win*)m_pButtonQuit )
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
                xwstring Message( g_StringTableMgr( "ui", "IDS_QUIT_MSG" ));

                // set nav text
                xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
                navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
                m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

                m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_QUIT_POPUP" ), TRUE, TRUE, FALSE, Message, navText, &m_PopUpResult );
            }
	    }
        else if( pWin == (ui_win*)m_pButtonScore )
        {
            g_AudioMgr.Play("Select_Norm");
            m_CurrentControl = IDC_PAUSE_MP_SCORE;
            m_State = DIALOG_STATE_SELECT;
        }
        else if( pWin == (ui_win*)m_pButtonOptions )
        {
            g_AudioMgr.Play("Select_Norm");
            m_CurrentControl = IDC_PAUSE_MP_OPTIONS;
            m_State = DIALOG_STATE_SELECT;
        }
        else if( pWin == (ui_win*)m_pButtonSettings )
        {
            g_AudioMgr.Play("Select_Norm");
            m_CurrentControl = IDC_PAUSE_MP_SETTINGS;
            m_State = DIALOG_STATE_SELECT;
        }
#ifdef TARGET_XBOX
        else if( pWin == (ui_win*)m_pButtonFriends )
        {
            g_AudioMgr.Play("Select_Norm");
            m_CurrentControl = IDC_PAUSE_MP_FRIENDS;
            m_State = DIALOG_STATE_SELECT;
        }
#endif
    }
}

//=========================================================================

void dlg_pause_mp::OnPadBack( ui_win* pWin )
{
    (void)pWin;
}

//=========================================================================

void dlg_pause_mp::OnUpdate ( ui_win* pWin, f32 DeltaTime )
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

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // turn on the buttons
            m_pButtonQuit    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonScore   ->SetFlag(ui_win::WF_VISIBLE, TRUE);    
            m_pButtonOptions ->SetFlag(ui_win::WF_VISIBLE, TRUE);    
            m_pButtonSettings->SetFlag(ui_win::WF_VISIBLE, TRUE);    
#ifdef TARGET_XBOX
            m_pButtonFriends ->SetFlag(ui_win::WF_VISIBLE, TRUE);    

            // disable friends list if we're not connected to Live.
            if( g_MatchMgr.GetAuthStatus() != AUTH_STAT_CONNECTED )
                m_pButtonFriends->SetFlag(ui_win::WF_DISABLED, TRUE);
#endif
            m_pNavText       ->SetFlag(ui_win::WF_VISIBLE, TRUE);    
    
#if defined( TARGET_XBOX )
            m_pFriendInvite     ->SetFlag(ui_win::WF_VISIBLE, HaveFriendRequest     );
            m_pGameInvite       ->SetFlag(ui_win::WF_VISIBLE, HaveGameInvitation    );
#endif

            s32 iControl = g_StateMgr.GetCurrentControl();
            if( (iControl == -1) || (GotoControl( iControl )==NULL) )
            {
                GotoControl( (ui_control*)m_pButtonQuit );
                m_pButtonQuit->SetFlag(WF_HIGHLIGHT, TRUE);        
                g_UiMgr->SetScreenHighlight( m_pButtonQuit->GetPosition() );
                m_CurrentControl =  IDC_PAUSE_MP_QUIT;
            }
            else
            {
                ui_control* pControl = GotoControl( iControl );
                pControl->SetFlag(WF_HIGHLIGHT, TRUE);
                g_UiMgr->SetScreenHighlight( pControl->GetPosition() );
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
                m_CurrentControl = IDC_PAUSE_MP_QUIT;
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

    if( m_pButtonQuit->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 0;
        g_UiMgr->SetScreenHighlight( m_pButtonQuit->GetPosition() );
    }
    else if( m_pButtonScore->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 1;
        g_UiMgr->SetScreenHighlight( m_pButtonScore->GetPosition() );
    }
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

    if( highLight != m_CurrHL )
    {
        if( highLight != -1 )
            g_AudioMgr.Play("Cusor_Norm");

        m_CurrHL = highLight;
    }
}

//=========================================================================
