//=========================================================================
//
//  dlg_online_main.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"

#include "dlg_OnlineMainXBOX.hpp"
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"

#ifndef X_RETAIL
#include "InputMgr\monkey.hpp"
#endif

//=========================================================================
//  Main Menu Dialog
//=========================================================================
static const s32 FrY = 145;     // Friends Y
static const s32 IcY = FrY + 9; // Friends Invite Icons Y

ui_manager::control_tem OnlineMenuControls[] = 
{
    // Frames.
    { IDC_ONLINE_QUICKMATCH,    "IDS_ONLINE_QUICKMATCH",    "button",   90,  40, 120, 40, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_OPTIMATCH,     "IDS_ONLINE_OPTIMATCH",     "button",   90,  75, 120, 40, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST,          "IDS_ONLINE_HOST",          "button",   90, 110, 120, 40, 0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_FRIENDS,       "IDS_ONLINE_FRIENDS",       "button",   90, FrY, 120, 40, 0, 3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_PLAYERS,       "IDS_ONLINE_PLAYERS",       "button",   90, 180, 120, 40, 0, 4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_EDIT_PROFILE,  "IDS_ONLINE_EDIT_PROFILE",  "button",   90, 215, 120, 40, 0, 5, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_VIEW_STATS,    "IDS_ONLINE_VIEW_STATS",    "button",   90, 250, 120, 40, 0, 6, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_SIGN_OUT,      "IDS_SIGN_OUT",             "button",   90, 285, 120, 40, 0, 7, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_NAV_TEXT,      "IDS_NULL",                 "text",      0,   0,   0,  0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_MP_FRIEND_INV, "IDS_NULL",                 "bitmap",   34, IcY,  26, 26, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_MP_GAME_INV,   "IDS_NULL",                 "bitmap",  242, IcY,  26, 26, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem OnlineMenuDialog =
{
    "IDS_ONLINE_MAIN",
    1, 9,
    sizeof(OnlineMenuControls)/sizeof(ui_manager::control_tem),
    &OnlineMenuControls[0],
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

void dlg_online_main_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "online menu", &OnlineMenuDialog, &dlg_online_main_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_online_main_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_online_main* pDialog = new dlg_online_main;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_online_main
//=========================================================================

dlg_online_main::dlg_online_main( void )
: m_pFriendInvite   ( NULL ),
  m_pGameInvite     ( NULL )
{
}

//=========================================================================

dlg_online_main::~dlg_online_main( void )
{
}

//=========================================================================

xbool dlg_online_main::Create( s32                        UserID,
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

    m_pButtonQuickMatch     = (ui_button*)  FindChildByID( IDC_ONLINE_QUICKMATCH   );
    m_pButtonOptiMatch      = (ui_button*)  FindChildByID( IDC_ONLINE_OPTIMATCH    );
    m_pButtonHost           = (ui_button*)  FindChildByID( IDC_ONLINE_HOST         );
    m_pButtonFriends        = (ui_button*)  FindChildByID( IDC_ONLINE_FRIENDS      );
    m_pButtonPlayers        = (ui_button*)  FindChildByID( IDC_ONLINE_PLAYERS      );
    m_pButtonEditProfile    = (ui_button*)  FindChildByID( IDC_ONLINE_EDIT_PROFILE );
    m_pButtonViewStats      = (ui_button*)  FindChildByID( IDC_ONLINE_VIEW_STATS   );
    m_pButtonSignOut        = (ui_button*)  FindChildByID( IDC_ONLINE_SIGN_OUT     );
    m_pNavText              = (ui_text*)    FindChildByID( IDC_ONLINE_NAV_TEXT     );

    m_pFriendInvite     = (ui_bitmap*)  FindChildByID( IDC_ONLINE_MP_FRIEND_INV  );
    m_pGameInvite       = (ui_bitmap*)  FindChildByID( IDC_ONLINE_MP_GAME_INV    );

    if ( m_pFriendInvite )
    {
        m_pFriendInvite->SetFlag( ui_win::WF_VISIBLE, FALSE);
        m_pFriendInvite->SetBitmap( g_UiMgr->FindBitmap( "icon_friend_req_rcvd" ) );
    }
    if ( m_pGameInvite )
    {
        m_pGameInvite->SetFlag( ui_win::WF_VISIBLE, FALSE);
        m_pGameInvite->SetBitmap( g_UiMgr->FindBitmap( "icon_invite_rcvd"       ) );

    }

    s32 iControl = g_StateMgr.GetCurrentControl();
    if( (iControl == -1) || (GotoControl(iControl)==NULL) )
    {
        GotoControl( (ui_control*)m_pButtonQuickMatch );
        m_CurrentControl = IDC_ONLINE_QUICKMATCH;
    }
    else
    {
        m_CurrentControl = iControl;
    }

    m_CurrHL = 0;

    // switch off the buttons to start
    m_pButtonQuickMatch     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonOptiMatch      ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonHost           ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonFriends        ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonPlayers        ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonEditProfile    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonViewStats      ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonSignOut        ->SetFlag(ui_win::WF_VISIBLE, FALSE);

#ifdef LAN_PARTY_BUILD
    m_pButtonOptiMatch  ->SetFlag(ui_win::WF_DISABLED, TRUE);
    m_pButtonFriends    ->SetFlag(ui_win::WF_DISABLED, TRUE);
    m_pButtonPlayers    ->SetFlag(ui_win::WF_DISABLED, TRUE);
#endif

    // initialize nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
  
    m_pNavText->SetLabel( navText );
    m_pNavText->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);    
    
    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;
    m_pPopUp = NULL;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_online_main::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_online_main::Render( s32 ox, s32 oy )
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

    // render the glow bar
    g_UiMgr->RenderGlowBar();
}

//=========================================================================

void dlg_online_main::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( pWin == (ui_win*)m_pButtonQuickMatch )
        {
            m_CurrentControl = IDC_ONLINE_QUICKMATCH;
            m_State = DIALOG_STATE_SELECT;
        }
        else if( pWin == (ui_win*)m_pButtonOptiMatch )
        {
            m_CurrentControl = IDC_ONLINE_OPTIMATCH;
            m_State = DIALOG_STATE_SELECT;
        }
        else if( pWin == (ui_win*)m_pButtonHost )
        {
            m_CurrentControl = IDC_ONLINE_HOST;
            m_State = DIALOG_STATE_SELECT;
        }
        else if( pWin == (ui_win*)m_pButtonFriends )
        {
            g_StateMgr.SetActiveControllerID( g_UiMgr->GetActiveController() );
            g_UiMgr->SetUserController( g_UiUserID, g_UiMgr->GetActiveController() );
            g_StateMgr.SetControllerRequested( g_UiMgr->GetActiveController(), TRUE );
            m_CurrentControl = IDC_ONLINE_FRIENDS;
            m_State = DIALOG_STATE_SELECT;
        }
        else if( pWin == (ui_win*)m_pButtonPlayers )
        {
            g_StateMgr.SetActiveControllerID( g_UiMgr->GetActiveController() );
            g_UiMgr->SetUserController( g_UiUserID, g_UiMgr->GetActiveController() );
            g_StateMgr.SetControllerRequested( g_UiMgr->GetActiveController(), TRUE );
            m_CurrentControl = IDC_ONLINE_PLAYERS;
            m_State = DIALOG_STATE_SELECT;
        }
        else if( pWin == (ui_win*)m_pButtonEditProfile )
        {
            g_StateMgr.InitPendingProfile( 0 );
            m_CurrentControl = IDC_ONLINE_EDIT_PROFILE;
            m_State = DIALOG_STATE_SELECT;
        }           
        else if( pWin ==(ui_win*)m_pButtonViewStats )
        {
            m_CurrentControl = IDC_ONLINE_VIEW_STATS;
            m_State = DIALOG_STATE_SELECT;
        }
        else if( pWin == (ui_win*)m_pButtonSignOut )
        {
            // Open a dialog to confirm quitting the online game component
            irect r = g_UiMgr->GetUserBounds( g_UiUserID );
            m_pPopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

            // get message text
            xwstring Message( g_StringTableMgr( "ui", "IDS_ONLINE_DISCONNECT" ));

            // set nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

            m_pPopUp->Configure( g_StringTableMgr( "ui", "IDS_NETWORK_POPUP" ), TRUE, TRUE, FALSE, Message, navText, &m_PopUpResult );

        }

        g_AudioMgr.Play("Select_Norm");
    }
}

//=========================================================================

void dlg_online_main::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if( m_State == DIALOG_STATE_ACTIVE )
    {
        SetState( DIALOG_STATE_BACK );
    }
}

//=========================================================================

void dlg_online_main::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    s32 highLight = -1;

    //
    // Figure out if we have invitations to be friends or join games
    //
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

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // turn on the controls
            m_pButtonQuickMatch ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonOptiMatch  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonHost       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonFriends    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonPlayers    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonEditProfile->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonViewStats  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonSignOut    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText          ->SetFlag(ui_win::WF_VISIBLE, TRUE);

#if defined( TARGET_XBOX )
            m_pFriendInvite     ->SetFlag(ui_win::WF_VISIBLE, HaveFriendRequest     );
            m_pGameInvite       ->SetFlag(ui_win::WF_VISIBLE, HaveGameInvitation    );
#endif
            s32 iControl = g_StateMgr.GetCurrentControl();
            if( (iControl == -1) || (GotoControl(iControl)==NULL) )
            {
                GotoControl( (ui_control*)m_pButtonQuickMatch );
                m_pButtonQuickMatch->SetFlag(WF_HIGHLIGHT, TRUE);        
                g_UiMgr->SetScreenHighlight( m_pButtonQuickMatch->GetPosition() );
                m_CurrentControl = IDC_ONLINE_QUICKMATCH;
            }
            else
            {
                ui_control* pControl = GotoControl( iControl );
                ASSERT( pControl );
                pControl->SetFlag(WF_HIGHLIGHT, TRUE);
                g_UiMgr->SetScreenHighlight(pControl->GetPosition() );
                m_CurrentControl = iControl;
            }

            // Check for a pending invite for the signed in user.  Note we can only
            // ever have 1 player logged on to Xbox Live in A51 hence the index of 0.
            if( g_MatchMgr.GetPendingInviteAccepted( 0 ) )
            {
                m_State = DIALOG_STATE_ACTIVATE;
                return;
            }
        }
    }
    else
    {
        m_pFriendInvite     ->SetFlag(ui_win::WF_VISIBLE, HaveFriendRequest     );
        m_pGameInvite       ->SetFlag(ui_win::WF_VISIBLE, HaveGameInvitation    );
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    if( m_pButtonQuickMatch->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 0;
        g_UiMgr->SetScreenHighlight( m_pButtonQuickMatch->GetPosition() );
    }
    else if( m_pButtonOptiMatch->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 1;
        g_UiMgr->SetScreenHighlight( m_pButtonOptiMatch->GetPosition() );
    }
    else if( m_pButtonHost->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 2;
        g_UiMgr->SetScreenHighlight( m_pButtonHost->GetPosition() );
    }
    else if( m_pButtonFriends->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 3;
        g_UiMgr->SetScreenHighlight( m_pButtonFriends->GetPosition() );
    }
    else if( m_pButtonPlayers->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 4;
        g_UiMgr->SetScreenHighlight( m_pButtonPlayers->GetPosition() );
    }
    else if( m_pButtonEditProfile->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 5;
        g_UiMgr->SetScreenHighlight( m_pButtonEditProfile->GetPosition() );
    }
    else if( m_pButtonViewStats->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 6;
        g_UiMgr->SetScreenHighlight( m_pButtonViewStats->GetPosition() );
    }
    else if( m_pButtonSignOut->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 7;
        g_UiMgr->SetScreenHighlight( m_pButtonSignOut->GetPosition() );
    }
    if( m_pPopUp && (m_PopUpResult!=DLG_POPUP_IDLE) )
    {
        if( m_PopUpResult==DLG_POPUP_YES )
        {
            g_MatchMgr.SignOut();
            m_pPopUp = NULL;
            m_State = DIALOG_STATE_BACK;
        }
        else
        {
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pPopUp = NULL;
        }
    }
    if( highLight != m_CurrHL )
    {
        g_AudioMgr.Play("Cusor_Norm");
        m_CurrHL = highLight;
    }
}

//=========================================================================
