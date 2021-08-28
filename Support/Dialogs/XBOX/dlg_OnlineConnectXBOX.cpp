//=========================================================================
//
//  dlg_OnlineConnect.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_text.hpp"
#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_listbox.hpp"

#include "..\dlg_OnlineConnect.hpp"
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\MatchMgr.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "MemCardMgr/MemCardMgr.hpp"

#include "../../Apps/GameApp/Config.hpp"

//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum online_connect_controls
{
	IDC_ONLINE_CONNECT_LISTBOX,
    IDC_ONLINE_CONNECT_NAV_TEXT,
};


ui_manager::control_tem OnlineConnectControls[] = 
{
    { IDC_ONLINE_CONNECT_LISTBOX,   "IDS_SIGN_IN",          "listbox",  70,  60, 220, 238, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_CONNECT_NAV_TEXT,  "IDS_NULL",             "text",      0,   0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem OnlineConnectDialog =
{
    "IDS_ONLINE_CONNECT",
    1, 9,
    sizeof(OnlineConnectControls)/sizeof(ui_manager::control_tem),
    &OnlineConnectControls[0],
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

void dlg_online_connect_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "connect info", &OnlineConnectDialog, &dlg_online_connect_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_online_connect_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_online_connect* pDialog = new dlg_online_connect;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  static functions
//=========================================================================

xbool s_DirtyUserList = FALSE;
xbool IsDirtyUserList( void )
{
#ifdef TARGET_XBOX
extern xbool xbox_CheckDeviceChanges();
    return ( xbox_CheckDeviceChanges() || s_DirtyUserList );
#else
    return TRUE;
#endif
}

//=========================================================================

void SetDirtyUserList( xbool Dirty )
{
    s_DirtyUserList = Dirty;
}

//=========================================================================
//  dlg_online_connect
//=========================================================================

dlg_online_connect::dlg_online_connect( void )
{
    SetDirtyUserList( TRUE );
}

//=========================================================================

dlg_online_connect::~dlg_online_connect( void )
{
}

//=========================================================================

xbool dlg_online_connect::Create( s32                        UserID,
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
    m_pUserList     = (ui_listbox*) FindChildByID( IDC_ONLINE_CONNECT_LISTBOX  );
    m_pNavText      = (ui_text*)    FindChildByID( IDC_ONLINE_CONNECT_NAV_TEXT );

    // hide them
    m_pUserList  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pNavText   ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
   
    m_pNavText->SetLabel( navText );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);


    // set up user list
    m_pUserList->SetFlag(ui_win::WF_SELECTED, TRUE);
    m_pUserList->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pUserList->DisableFrame();
    m_pUserList->SetExitOnSelect(FALSE);
    m_pUserList->SetExitOnBack(TRUE);
    m_pUserList->EnableHeaderBar();
    m_pUserList->SetHeaderBarColor( xcolor(19,59,14,196) );
    m_pUserList->SetHeaderColor( xcolor(255,252,204,255) );

    // set initial focus
    GotoControl( (ui_control*)m_pUserList );
    m_CurrentControl = IDC_ONLINE_CONNECT_LISTBOX;
    m_PopUp = NULL;
    m_PopUpConfirmPassword = NULL;
    // initialize connect state
    m_ConnectState = CONNECT_IDLE;
    m_AccountsHaveChanged = FALSE;

    // disable highlight
    g_UiMgr->DisableScreenHighlight();
    m_Position = Position;

    // Force account enumeration when we're first in.
    extern xbool g_NeedAccountEnumeration;
    g_NeedAccountEnumeration = TRUE;
    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    SetState( DIALOG_STATE_ACTIVE );

    // Return success code
    return Success;
}

//=========================================================================

void dlg_online_connect::Destroy( void )
{
    ASSERT( g_StateMgr.IsBackgroundThreadRunning()==FALSE );

    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();

}

//=========================================================================

void dlg_online_connect::Render( s32 ox, s32 oy )
{
    static s32 offset   =  0;
    static s32 gap      =  9;
    static s32 width    =  4;

	irect   rb;
	s32     XRes;
    s32     YRes;

    eng_GetRes( XRes, YRes );
    rb.Set( 0, 0, XRes, YRes );
    g_UiMgr->RenderGouraudRect( rb, xcolor(0,0,0,180),
                                    xcolor(0,0,0,180),
                                    xcolor(0,0,0,180),
                                    xcolor(0,0,0,180), FALSE);
    
    
    // render transparent screen
    rb.l = m_CurrPos.l + 22;
    rb.t = m_CurrPos.t;
    rb.r = m_CurrPos.r - 23;
    rb.b = m_CurrPos.b;

    g_UiMgr->RenderGouraudRect( rb, xcolor(56,115,58,64),
                                    xcolor(56,115,58,64),
                                    xcolor(56,115,58,64),
                                    xcolor(56,115,58,64), FALSE);


    // render the screen bars
    s32 y = rb.t + offset;    

    while ( y < rb.b )
    {
        irect bar;

        if ( ( y + width ) > rb.b )
        {
            bar.Set( rb.l, y, rb.r, rb.b );
        }
        else
        {
            bar.Set( rb.l, y, rb.r, ( y + width ) );
        }

        // draw the bar
        g_UiMgr->RenderGouraudRect( bar, xcolor(56,115,58,30),
                                         xcolor(56,115,58,30),
                                         xcolor(56,115,58,30),
                                         xcolor(56,115,58,30), FALSE);

        y += gap;
    }
    
    // increment the offset
    if ( ++offset > 9 )
    {
        offset = 0;
    }

    // render the normal dialog stuff
    ui_dialog::Render( ox, oy );

    // render the glow bar
    g_UiMgr->RenderGlowBar();
}

//=========================================================================

void dlg_online_connect::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    // check for match manager busy
    if( g_MatchMgr.IsBusy() || (GetState() != DIALOG_STATE_ACTIVE) )
    {
        // wait for the user accounts to populate the listbox
        return;
    }

    if (m_pUserList)
    {
        if (m_pUserList->GetItemCount() == 0)
            return;
    }


    switch( m_ConnectState )
    {
    //
    // We're picking a network configuration to use. Even though this will not be used on xbox,
    // just leave the generic code model in there so we don't have so many ifdefs.
    //
    //------------------------------------------------------
    case ACTIVATE_SELECT:
        ASSERT(FALSE);      // Should not get here on xbox!
    break;
    //------------------------------------------------------
    case CONNECT_SELECT_USER:
    {
        // check if this is an existing user
        // get the profile index from the list
        s32 index = m_pUserList->GetSelectedItemData();
        if ( index < m_NumUsers ) //user exists 
        {
            const online_user& User = g_MatchMgr.GetUserAccount( index );
            if( User.Flags & USER_HAS_PASSWORD )
            {
                // Fire up a dialog that asks to enter in a password.
                irect r;
                r.Set(0,0,400,200);
                m_PopUpConfirmPassword = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

                // set nav text
                xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_ACCEPT" ));
                navText += g_StringTableMgr( "ui", "IDS_NAV_CANCEL" ); 
                m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

                xwstring MessageText( g_StringTableMgr( "ui", "IDS_ENTER_PASSCODE_MSG" ) );
                MessageText += " ";
                MessageText += User.Name;

                // configure message
                m_PopUpConfirmPassword->ConfigurePassword( r, g_StringTableMgr( "ui", "IDS_ENTER_PASSCODE" ), 
                    MessageText,
                    navText,
                    &m_PopUpConfirmPasswordResult );

                m_PopUpConfirmPassword->SetPassword((u8*)User.Password);

                m_AccountsHaveChanged = FALSE;

                m_bAskForPassword = TRUE;
            }
            else
            {
                // JHOWA if you change any of the follwoing code inside the else
                // let me know.. it  will need to be updated in the return from
                // password popup in OnUpdate();

                // tell the match manager to start authentication
                g_MatchMgr.SetUserAccount( index );

                // try to connect using this user account
                SetConnectState( CONNECT_INIT );

                // hide list
                m_pUserList ->SetFlag(ui_win::WF_VISIBLE, FALSE);
                m_pNavText  ->SetFlag(ui_win::WF_VISIBLE, FALSE);

                // disable highlight
                g_UiMgr->DisableScreenHighlight();
                irect r = GetPosition();
                r.Inflate( 50, 0 );
                InitScreenScaling( r ); // Resize dialog back to original width
                m_CurrentControl = 0;
            }
        }
        else
        {
            g_StateMgr.Reboot( REBOOT_NEWUSER );
            // create a new user account
            // XBOX- exit game and go to dashboard
            ASSERTS( FALSE ,"Boot to XBOX Dashboard here" );
        }
    }
    break;

    //------------------------------------------------------
    case CONNECT_FAILED_WAIT:
        g_StateMgr.Reboot( REBOOT_MANAGE );
    break;
    //------------------------------------------------------
    default:
        break;
// Brian, I removed this assert because it would trigger if you pressed the select button 
// at anytime during the connection logic.  Is that what you really wanted???
        //ASSERT(FALSE);
    }
}

//=========================================================================

void dlg_online_connect::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    // check for menu automation
    if( CONFIG_IS_AUTOSERVER || CONFIG_IS_AUTOCLIENT )
    {
        return;
    }

    if( GetState() == DIALOG_STATE_ACTIVE )
    {
        g_NetworkMgr.SetOnline( FALSE );
        // cancel connecting
        if( g_MatchMgr.GetAuthStatus()==AUTH_STAT_SELECT_USER )
        {
            g_MatchMgr.SetAuthStatus( AUTH_STAT_DISCONNECTED );
        }
        //g_AudioMgr.Play( "OptionBack" );
        SetState( DIALOG_STATE_BACK );
    }
}

//=========================================================================

void dlg_online_connect::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // Limit deltatime size if we're debugging
    if( DeltaTime > 0.25f )
    {
        DeltaTime = 1.0f/30.0f;
    }
    m_Timeout += DeltaTime;
    // Did the popup automatically close itself? This would only happen
    // if the CANCEL/BACK button was pressed
    if( m_PopUp && (m_PopUpResult!= DLG_POPUP_IDLE) )
    {
        if( m_PopUpResult == DLG_POPUP_NO )
            OnPadBack( pWin );

        m_PopUp = NULL;
    }
    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {            
            // connect
            // Do we need this? Won't the state already be CONNECT_INIT? Scaling the dialog
            // would cause us not to enter the state.
            if( m_ConnectState == CONNECT_DISPLAY_MOTD )
            {
                // set nav text
                xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_OK" ));

                // pop up modal dialog and wait for response
                irect r( 0, -10, 400, 290 );

                xwstring MessageText( g_MatchMgr.GetMessageOfTheDay() );

                m_PopUp->Configure( r,
                    g_StringTableMgr( "ui", "IDS_ONLINE_MOTD" ), 
                    TRUE, 
                    TRUE, 
                    FALSE, 
                    MessageText,
                    navText,
                    &m_PopUpResult );

                m_PopUp->EnableBlackout( FALSE );
            }
            else if( m_ConnectState == CONNECT_IDLE )
            {
                m_pUserList ->SetFlag(ui_win::WF_VISIBLE, TRUE);
                m_pNavText  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
                GotoControl( (ui_control*)m_pUserList );

                // set highlight
                g_UiMgr->SetScreenHighlight( m_pUserList->GetPosition() );
                if( (g_MatchMgr.GetAuthStatus()==AUTH_STAT_CONNECTED) &&
                    (g_MatchMgr.GetOptionalMessage() == FALSE) )
                {
                    g_MatchMgr.SetUserStatus( BUDDY_STATUS_ONLINE );
                    SetConnectState( CONNECT_CHECK_MOTD );
                }
                else
                {
                    SetConnectState( CONNECT_SELECT_USER );
                }
            }
            else if( m_ConnectState == CONNECT_SELECT_USER )
            {
                m_pUserList ->SetFlag(ui_win::WF_VISIBLE, TRUE);
                m_pNavText  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
                GotoControl( (ui_control*)m_pUserList );

                // set highlight
                g_UiMgr->SetScreenHighlight( m_pUserList->GetPosition() );
            }
            else if( (m_ConnectState == CONNECT_INIT) || (m_ConnectState == CONNECT_AUTHENTICATE_USER) )
            {
                ASSERT( m_PopUp == NULL );
                irect r = g_UiMgr->GetUserBounds( g_UiUserID );
                m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                    ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
                m_PopUpResult = DLG_POPUP_IDLE;

                r = irect(0,0,300,160);
                m_PopUp->Configure( r,
                    -1.0f,
                    g_StringTableMgr( "ui", "IDS_MAIN_MENU_ONLINE_XBOX"     ),
                    g_StringTableMgr( "ui", "IDS_ONLINE_PLEASE_WAIT"),
                    g_StringTableMgr( "ui", "IDS_ONLINE_CONNECT_INIT") );

                m_PopUp->EnableBlackout( FALSE );
            }
        }
    }
    else
    {
        if( g_StateMgr.IsBackgroundThreadRunning() )
        {
            // update the glow bar
            g_UiMgr->UpdateGlowBar(DeltaTime);
            return;
        }

        if( g_UiMgr->IsWipeActive() == FALSE )
        {
            // window is scaled to correct size - do the main update
            switch( m_ConnectState )
            {
                //------------------------------------------
                case CONNECT_IDLE:
                    break;

                case CONNECT_SELECT_USER:
                    // wait for state change (user account selected)
                    RefreshUserList();
                    break;

                //------------------------------------------
                case CONNECT_INIT:
                    UpdateConnectInit();
                    break;
                break;

                //------------------------------------------
                case CONNECT_WAIT:
                {
                    SetConnectState( CONFIG_INIT );
                }
                break;

                //------------------------------------------
                case CONFIG_INIT:
                {

                    irect r( 0, 0, 300, 160 );

                    if( m_PopUp == NULL )
                    {
                        m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                            ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
                    }
                    m_PopUp->Configure( r,
                                        0.25f,
                                        g_StringTableMgr( "ui", "IDS_MAIN_MENU_ONLINE_XBOX"            ),
                                        g_StringTableMgr( "ui", "IDS_ONLINE_PLEASE_WAIT"       ),
                                        g_StringTableMgr( "ui", "IDS_ONLINE_CONNECT_MATCHMAKER_XBOX" ) );

                    m_PopUp->EnableBlackout( FALSE );

                    SetConnectState( ACTIVATE_WAIT_DHCP );
                    xtimer t;

                    t.Start();
                    g_NetworkMgr.SetOnline( TRUE );
                    t.Stop();
                    LOG_MESSAGE( "dlg_online_connect::OnUpdate", "SetOnline() call took %2.02fms", t.ReadMs() );
                }
                break;

                //------------------------------------------
                case ACTIVATE_WAIT_DHCP:
                {
                    // Wait until DHCP assigns us an address
                    if( m_Timeout < 30.0f )
                    {
                        irect r( 0, 0, 300, 160 );

                        if( m_PopUp == NULL )
                        {
                            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                                ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
                        }
                        //
                        // User pressed cancel. This test has to be done prior to the reconfigure as
                        // the configure function will clear out the popup result to idle.
                        //
                        if( m_PopUpResult != DLG_POPUP_IDLE )
                        {
                            m_PopUp = NULL;
                            Failed( "IDS_ONLINE_CONNECT_ABORTED" );
                            break;
                        }

                        interface_info Info;
                        net_GetInterfaceInfo(-1, Info );
                        if( Info.IsAvailable==FALSE )
                        {
                            Failed( "IDS_ONLINE_CHECK_CABLE" );
                            break;
                        }
                        if( Info.NeedsServicing )
                        {
                            Failed( "IDS_SIGN_IN_NO_CONNECTION" );
                            break;
                        }
                        xwstring MessageText  = g_StringTableMgr( "ui", "IDS_ONLINE_PLEASE_WAIT" );
                                 MessageText += "\n";
                                 MessageText += g_StringTableMgr( "ui", "IDS_ONLINE_CONNECT_MATCHMAKER_XBOX" );
                                 MessageText += "\n";
                                 MessageText += g_StringTableMgr( "ui", "IDS_ONLINE_TIMEOUT" );
                                 MessageText += xwstring( xfs("%d",(30-(s32)m_Timeout)) );

                        m_PopUp->Configure( r,
                                            g_StringTableMgr( "ui", "IDS_MAIN_MENU_ONLINE_XBOX" ), 
                                            FALSE, 
                                            FALSE, 
                                            FALSE, 
                                            MessageText,
                                            xwstring( "" ),
                                            &m_PopUpResult );

                        m_PopUp->EnableBlackout( FALSE );

                        net_GetInterfaceInfo( -1, m_Info );
                        if( m_Info.Address )
                        {
                            SetConnectState( CONNECT_MATCH_INIT );
                            break;
                        }
                    }
                    else
                    {
                        Failed("IDS_ONLINE_CONNECT_TIMEOUT");
                        break;
                    }
                    break;
                }
                break;

                //------------------------------------------
                case CONNECT_MATCH_INIT:
                {
                    s32 SystemId = net_GetSystemId();

                    g_MatchMgr.SetUniqueId( (const byte*)&SystemId, sizeof(SystemId) );
                    
                    net_socket& LocalSocket = g_NetworkMgr.GetSocket();
                    if( LocalSocket.IsEmpty()  )
                    {
                        m_Status = LocalSocket.Bind( NET_GAME_PORT, NET_FLAGS_BROADCAST );
                    
                        net_address     Broadcast;
                        Broadcast = net_address( m_Info.Broadcast, LocalSocket.GetPort() );
                        ASSERT( m_Status );
                        x_DebugMsg( "Network socket opened. Address is %s\n",LocalSocket.GetStrAddress() );
                        // Second parameter is country code. This will be determined when we get the
                        // country of origin for the DNAS validation above.
                        m_Done = g_MatchMgr.Init( LocalSocket, Broadcast );
                    }
                    m_Timeout = 0.0f;

                    if( (g_MatchMgr.GetAuthStatus() == AUTH_STAT_CONNECTED) &&
                        (g_MatchMgr.GetOptionalMessage() == FALSE) )
                    {
                        g_MatchMgr.SetUserStatus( BUDDY_STATUS_ONLINE );
                        SetConnectState( CONNECT_CHECK_MOTD );
                    }
                    else
                    {
                        SetConnectState( CONNECT_AUTHENTICATE_USER );
                    }
                }
                break;

                //------------------------------------------
                case CONNECT_AUTHENTICATE_USER:
                    UpdateAuthUser();
                break;

                //------------------------------------------
                case CONNECT_FAILED:
                {
                    //SetState(  DIALOG_STATE_BACK;
                    //g_UiMgr->EndDialog( g_UiUserID, TRUE );
                    //return;

                    // set nav text
                    xwstring navText;

                    if( x_strcmp(m_LabelText, "IDS_ONLINE_CONNECT_MATCHMAKER_REFUSED") == 0 )
                    {
                        // account may be locked - boot to account mangager.
                        navText = g_StringTableMgr( "ui", "IDS_NAV_ACCOUNT_MANAGER" );
                        navText += g_StringTableMgr( "ui", "IDS_NAV_CANCEL" );
                    }
                    else if( x_strcmp(m_LabelText, "IDS_ONLINE_CONNECT_MATCHMAKER_FAILED") == 0 )
                    {
                        // timeout TCR requires a retry.
                        navText = g_StringTableMgr( "ui", "IDS_NAV_TRYAGAIN" );
                        navText += g_StringTableMgr( "ui", "IDS_NAV_CANCEL" );
                    }
                    else
                    {
                        // otherwise, reboot to troubleshooter
                        navText = g_StringTableMgr( "ui", "IDS_NAV_NETWORK_TROUBLESHOOTER" );
                        navText += g_StringTableMgr( "ui", "IDS_NAV_CANCEL" );
                    }

                    // pop up modal dialog and wait for response
                    irect r( 0, 0, 300, 160 );

                    if (x_GetLocale() != XL_LANG_ENGLISH)
                    {
                        r.Set( 0, 0, 380, 160 );
                    }

                    xstring LabelText( g_StringTableMgr( "ui", m_LabelText ) );
                    xwstring ErrorText( xfs(LabelText, m_LastErrorCode ) );

                    if( m_PopUp == NULL )
                    {
                        m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                            ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
                        m_PopUpResult = DLG_POPUP_IDLE;
                    }
                    m_PopUp->Configure( r,
                                        g_StringTableMgr( "ui", "IDS_SIGN_IN" ), 
                                        TRUE, 
                                        TRUE, 
                                        FALSE, 
                                        ErrorText,
                                        navText,
                                        &m_PopUpResult );

                    m_PopUp->EnableBlackout( FALSE );

                    // set state
                    SetConnectState( CONNECT_FAILED_WAIT );
                }
                break;

                //------------------------------------------
                case CONNECT_FAILED_WAIT:
                {
                    if( g_NetworkMgr.IsOnline() )
                    {
                        g_NetworkMgr.SetOnline( FALSE );
                    }
                    // wait for response
                    if( m_PopUpResult != DLG_POPUP_IDLE )
                    {
                        if( m_PopUpResult == DLG_POPUP_YES )
                        {
                            // boot to xbox network troubleshooter
                            // or xbox account manager
                            if( x_strcmp(m_LabelText, "IDS_ONLINE_CONNECT_MATCHMAKER_REFUSED") == 0 )
                            {
                                // account may be locked - boot to account mangager.
                                g_StateMgr.Reboot( REBOOT_MESSAGE );
                                ASSERT( FALSE );
                            }
                            else if( x_strcmp(m_LabelText, "IDS_ONLINE_CONNECT_MATCHMAKER_FAILED") == 0 )
                            {
                                // busy timeout retry
                                g_MatchMgr.SetState( MATCH_CONNECT_MATCHMAKER );
                                SetConnectState( CONNECT_AUTHENTICATE_USER );
                                m_PopUp = NULL;
                                {
                                    irect r = GetPosition();
                                    r.Deflate( 50, 0 );
                                    InitScreenScaling( r ); // Resize dialog back to original width
                                }
                                return;
                            }
                            else
                            {
                                // other errors - troubleshooter
                                g_StateMgr.Reboot( REBOOT_MANAGE );
                                ASSERT( FALSE );
                            }
                        }
                        else
                        {
                            // sign out
                            g_MatchMgr.SignOut();
                            SetState( DIALOG_STATE_BACK );
                            g_ActiveConfig.SetExitReason( GAME_EXIT_CONTINUE );
                            return;
                        }
                    }
                }
                break;

                //------------------------------------------
                case CONNECT_AUTO_UPDATE:
                {
                    // A required update to the XBox live service

                    // set nav text
                    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_UPDATE_GAME" ));
                    navText += g_StringTableMgr( "ui", "IDS_NAV_CANCEL" );

                    // pop up modal dialog and wait for response
                    irect r( 0, 0, 300, 160 );

                    if( m_PopUp == NULL )
                    {
                        m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                            ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
                    }
                    m_PopUp->Configure( r,
                                        g_StringTableMgr( "ui", "IDS_SIGN_IN" ), 
                                        TRUE, 
                                        TRUE, 
                                        FALSE, 
                                        g_StringTableMgr( "ui", "IDS_SIGN_IN_REQUIRED_UPDATE" ),
                                        navText,
                                        &m_PopUpResult );

                    m_PopUp->EnableBlackout( FALSE );

                    // set state
                    SetConnectState( CONNECT_AUTO_UPDATE_WAIT );
                }
                break;

                //------------------------------------------
                case CONNECT_AUTO_UPDATE_WAIT:
                {
                    // wait for response
                    if( m_PopUpResult != DLG_POPUP_IDLE )
                    {
                        if( m_PopUpResult == DLG_POPUP_YES )
                        {
                            g_StateMgr.Reboot( REBOOT_UPDATE );
                            // boot to xbox dashboard
                        }
                        else
                        {
                            // sign out
                            g_MatchMgr.SignOut();
                            SetState( DIALOG_STATE_BACK );
                            ASSERT( g_UiMgr->GetTopmostDialog(m_UserID) == this );
                            g_UiMgr->EndDialog( g_UiUserID, TRUE );
                            return;
                        }
                    }
                }
                break;

                //------------------------------------------
                case CONNECT_REQUIRED_MESSAGE:
                {
                    // A required message to read from the XBox live service

                    // set nav text
                    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_DASHBOARD" ));
                    navText += g_StringTableMgr( "ui", "IDS_NAV_CANCEL" );

                    // pop up modal dialog and wait for response
                    irect r( 0, 0, 300, 160 );

                    if( m_PopUp == NULL )
                    {
                        m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                            ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
                    }
                    m_PopUp->Configure( r,
                                        g_StringTableMgr( "ui", "IDS_SIGN_IN" ), 
                                        TRUE, 
                                        TRUE, 
                                        FALSE, 
                                        g_StringTableMgr( "ui", "IDS_SIGN_IN_IMPORTANT_MESSAGE" ),
                                        navText,
                                        &m_PopUpResult );

                    m_PopUp->EnableBlackout( FALSE );

                    // set state
                    SetConnectState( CONNECT_REQUIRED_MESSAGE_WAIT );
                }
                break;

                //------------------------------------------
                case CONNECT_REQUIRED_MESSAGE_WAIT:
                {
                    // wait for response
                    if( m_PopUpResult != DLG_POPUP_IDLE )
                    {
                        if( m_PopUpResult == DLG_POPUP_YES )
                        {
                            g_StateMgr.Reboot( REBOOT_MESSAGE );
                            // boot to xbox dashboard
                        }
                        else
                        {
                            // sign out
                            g_MatchMgr.SignOut();
                            SetState( DIALOG_STATE_BACK );
                            ASSERT( g_UiMgr->GetTopmostDialog(m_UserID) == this );
                            g_UiMgr->EndDialog( g_UiUserID, TRUE );
                            return;
                        }
                    }
                }
                break;

                //------------------------------------------
                case CONNECT_OPTIONAL_MESSAGE:
                {
                    // An optional message to read from the XBox live service

                    // set nav text
                    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_READ_NOW" ));
                    navText += g_StringTableMgr( "ui", "IDS_NAV_READ_LATER" );

                    // pop up modal dialog and wait for response
                    irect r( 0, 0, 300, 160 );

                    if( m_PopUp == NULL )
                    {
                        m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                            ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
                    }
                    m_PopUp->Configure( r,
                                        g_StringTableMgr( "ui", "IDS_SIGN_IN" ), 
                                        TRUE, 
                                        TRUE, 
                                        FALSE, 
                                        g_StringTableMgr( "ui", "IDS_SIGN_IN_NEW_MESSAGE" ),
                                        navText,
                                        &m_PopUpResult );

                    m_PopUp->EnableBlackout( FALSE );

                    // set state
                    SetConnectState( CONNECT_OPTIONAL_MESSAGE_WAIT );
                }
                break;

                //------------------------------------------
                case CONNECT_OPTIONAL_MESSAGE_WAIT:
                {
                    // wait for response
                    if ( m_PopUpResult != DLG_POPUP_IDLE )
                    {
                        if ( m_PopUpResult == DLG_POPUP_YES )
                        {
                            // boot to xbox dashboard
                            g_StateMgr.Reboot( REBOOT_MESSAGE );
                        }
                        else
                        {
                            // Just continue to sign in
                            SetState( DIALOG_STATE_EXIT );
                        }
                    }
                }
                break;

                //------------------------------------------
                case CONNECT_DONE:
                    if( m_PopUpResult != DLG_POPUP_IDLE )
                    {
                        // Dialog will self close.
                        m_PopUp = NULL;
                        SetState( DIALOG_STATE_EXIT );
                        return;
                    }
                    break;

                    //------------------------------------------
                case CONNECT_CHECK_MOTD:
                    if( g_MatchMgr.GetMessageOfTheDay() )
                    {
                        SetConnectState( CONNECT_DISPLAY_MOTD );
                        irect Position = m_Position;
                        Position.Inflate( 110, 0 );

                        InitScreenScaling( Position );
                        m_PopUpResult = DLG_POPUP_IDLE;

                        // set state
                    }
                    else
                    {
                        if (m_PopUp)
                        {
#if defined(X_ASSERT)
                            ui_dialog* pCurrent = g_UiMgr->GetTopmostDialog( g_UiUserID );
                            ASSERT( pCurrent == m_PopUp );
#endif                        
                            g_UiMgr->EndDialog( g_UiUserID, TRUE );
                            m_PopUp = NULL;
                        }
                        SetState( DIALOG_STATE_EXIT );
                        return;
                    }
                break;
                //------------------------------------------
                case CONNECT_DISPLAY_MOTD:
                    if( m_PopUpResult != DLG_POPUP_IDLE )
                    {
                        // Dialog will self close.
                        SetState( DIALOG_STATE_EXIT );
                    }
                    break;
            }
        }
    }

    if( m_PopUpConfirmPassword )
    {
        if( m_AccountsHaveChanged )
        {
            ASSERT( g_UiMgr->GetTopmostDialog( g_UiUserID ) == m_PopUpConfirmPassword );
            g_UiMgr->EndDialog( g_UiUserID, TRUE );
            m_PopUpConfirmPassword = NULL;
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
            return;
        }

        if ( m_PopUpConfirmPasswordResult != DLG_POPUP_IDLE )
        {
            if( m_PopUpConfirmPasswordResult == DLG_POPUP_YES )
            {
                // tell the match manager to start authentication
                g_MatchMgr.SetUserAccount( m_pUserList->GetSelectedItemData() );

                // try to connect using this user account
                g_NetworkMgr.SetOnline( TRUE );
                net_socket& LocalSocket = g_NetworkMgr.GetSocket();
                if( LocalSocket.IsEmpty()  )
                {
                    m_Status = LocalSocket.Bind( NET_GAME_PORT, NET_FLAGS_BROADCAST );
                
                    net_address     Broadcast;
                    Broadcast = net_address( m_Info.Broadcast, LocalSocket.GetPort() );
                    ASSERT( m_Status );
                    x_DebugMsg( "Network socket opened. Address is %s\n",LocalSocket.GetStrAddress() );
                    // Second parameter is country code. This will be determined when we get the
                    // country of origin for the DNAS validation above.
                    m_Done = g_MatchMgr.Init( LocalSocket, Broadcast );
                }
                SetConnectState( CONNECT_AUTHENTICATE_USER );

                // hide list
                m_pUserList ->SetFlag(ui_win::WF_VISIBLE, FALSE);
                m_pNavText  ->SetFlag(ui_win::WF_VISIBLE, FALSE);

                // disable highlight
                g_UiMgr->DisableScreenHighlight();
                irect r = GetPosition();
                r.Inflate( 50, 0 );
                InitScreenScaling( r ); // Resize dialog back to original width
                m_CurrentControl = 0;
            }
            else
            {
                // ERROR with password.
            }
            m_PopUpConfirmPassword = NULL;
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    // update everything else
    ui_dialog::OnUpdate( pWin, DeltaTime );
}

//=========================================================================

void dlg_online_connect::RefreshUserList( void )
{
    s32         i;

    // We can't refresh the list while the match manager is busy enumerating devices
    if( g_MatchMgr.UpdateLiveAccounts() == FALSE )
        return;

    s32 Selection = m_pUserList->GetSelection();

    m_AccountsHaveChanged = TRUE;
    // clear the list
    m_pUserList->DeleteAllItems();

    // get number user accounts from network mgr.
    m_NumUsers = g_MatchMgr.GetUserAccountCount();

    // fill it with the user information
    for( i=0; i<m_NumUsers; i++ )
    {
        // add the profile to the list
        const online_user& Profile = g_MatchMgr.GetUserAccount(i);
        m_pUserList->AddItem( Profile.Name, i );
    }    

    // add an empty slot
    m_pUserList->AddItem( g_StringTableMgr("ui", "IDS_SIGN_IN_NEW_ACCOUNT"), m_NumUsers );

    s32 ItemCount = m_pUserList->GetItemCount();

    if( ItemCount > 0 )
    {
        Selection = MINMAX( 0, Selection, ItemCount - 1 );
        m_pUserList->SetSelection( Selection );
    }

    SetDirtyUserList( FALSE );
}

//=========================================================================
// Will give option of managing or dropping back to main menu
void dlg_online_connect::Failed( const char* pFailureReason, s32 ErrorCode, cancel_mode Mode, connect_states RetryDestination )
{
    (void)ErrorCode;
    (void)RetryDestination;

    if( g_StateMgr.IsBackgroundThreadRunning() )
    {
        g_StateMgr.StopBackgroundRendering();
    }
    
    x_strcpy( m_LabelText, pFailureReason);

    m_LastErrorCode = ErrorCode;
    m_CancelMode    = Mode;

    SetConnectState( CONNECT_FAILED );
}

//=========================================================================
void dlg_online_connect::SetConnectState( connect_states State )
{
    LOG_MESSAGE( "dlg_online_connect::SetConnectState","State transition from %s to %s",StateName( m_ConnectState ), StateName( State ) );
    m_ConnectState = State;
    m_Timeout = 0.0f;
}

//=========================================================================
const char* dlg_online_connect::StateName( connect_states State )
{
    switch( State )
    {
    case CONNECT_IDLE:                  return "CONNECT_IDLE";
    case CONNECT_INIT:                  return "CONNECT_INIT";
    case CONNECT_WAIT:                  return "CONNECT_WAIT";
    case SIGN_IN_INIT:                  return "SIGN_IN_INIT";
    case SIGN_IN_WAIT:                  return "SIGN_IN_WAIT";
    case CONFIG_INIT:                   return "CONFIG_INIT";
    case CONNECT_AUTHENTICATE_MACHINE:  return "CONNECT_AUTHENTICATE_MACHINE";
    case CONNECT_SELECT_USER:           return "CONNECT_SELECT_USER";
    case ACTIVATE_INIT:                 return "ACTIVATE_INIT";
    case ACTIVATE_SELECT:               return "ACTIVATE_SELECT";
    case ACTIVATE_LOOP:                 return "ACTIVATE_LOOP";
    case ACTIVATE_WAIT_DHCP:            return "ACTIVATE_WAIT_DHCP";
    case CONNECT_MATCH_INIT:            return "CONNECT_MATCH_INIT";
    case CONNECT_AUTHENTICATE_USER:     return "CONNECT_AUTHENTICATE_USER";
    case CONNECT_FAILED:                return "CONNECT_FAILED";
    case CONNECT_FAILED_WAIT:           return "CONNECT_FAILED_WAIT";
    case CONNECT_AUTO_UPDATE:           return "CONNECT_AUTO_UPDATE";
    case CONNECT_AUTO_UPDATE_WAIT:      return "CONNECT_AUTO_UPDATE_WAIT";
    case CONNECT_REQUIRED_MESSAGE:      return "CONNECT_REQUIRED_MESSAGE";
    case CONNECT_REQUIRED_MESSAGE_WAIT: return "CONNECT_REQUIRED_MESSAGE_WAIT";
    case CONNECT_OPTIONAL_MESSAGE:      return "CONNECT_OPTIONAL_MESSAGE";
    case CONNECT_OPTIONAL_MESSAGE_WAIT: return "CONNECT_OPTIONAL_MESSAGE_WAIT";
    case CONNECT_DONE:                  return "CONNECT_DONE";
    case CONNECT_CHECK_MOTD:            return "CONNECT_CHECK_MOTD";
    case CONNECT_DISPLAY_MOTD:          return "CONNECT_DISPLAY_MOTD";
    case NUM_CONNECT_STATES:            return "NUM_CONNECT_STATES";
    default:                            ASSERT( FALSE );
    }
    return "";
}

//=========================================================================
// Right now, this is only ever used to make sure we start user authentication
// instead of initial machine authentication.
void dlg_online_connect::Configure( connect_mode ConnectMode )
{
    net_socket& LocalSocket = g_NetworkMgr.GetSocket();
    auth_status Status = g_MatchMgr.GetAuthStatus();
    switch( ConnectMode )
    {
    case CONNECT_MODE_AUTH_USER:
        // Start the matchmaker authenticating the user, then wait for the authentication 
        // to complete. This is not necessarily the prettiest way to do this but we are 
        // splitting up what was the authentication process in to two stages now since the 
        // profile needs to be picked first. XBOX Live may need to skip this stage as
        // the user, at this point, should already be authenticated as they select their
        // 'profile' seperately from their saved profile.
        g_NetworkMgr.SetOnline( TRUE );
        if( LocalSocket.IsEmpty()  )
        {
            m_Status = LocalSocket.Bind( NET_GAME_PORT, NET_FLAGS_BROADCAST );
        
            net_address     Broadcast;
            Broadcast = net_address( m_Info.Broadcast, LocalSocket.GetPort() );
            ASSERT( m_Status );
            x_DebugMsg( "Network socket opened. Address is %s\n",LocalSocket.GetStrAddress() );
            // Second parameter is country code. This will be determined when we get the
            // country of origin for the DNAS validation above.
            m_Done = g_MatchMgr.Init( LocalSocket, Broadcast );
        }
        g_MatchMgr.SetState( MATCH_CONNECT_MATCHMAKER );
        SetConnectState( CONNECT_AUTHENTICATE_USER );
        break;
    case CONNECT_MODE_CONNECT:
        if( (Status == AUTH_STAT_CONNECTING)    ||
            (Status == AUTH_STAT_CONNECTED ) )
        {
            SetConnectState( CONNECT_MATCH_INIT );
        }
        else
        {
            SetConnectState( CONNECT_SELECT_USER );
        }
        break;
    default:
        ASSERT(FALSE);
    }
}

//=========================================================================
void dlg_online_connect::UpdateConnectInit( void )
{
    if( g_NetworkMgr.IsOnline() )
    {
        // Already connected - bail out early!
        m_ConnectState  = CONNECT_IDLE;
        m_State         = DIALOG_STATE_EXIT;
        ASSERT( g_UiMgr->GetTopmostDialog( g_UiUserID ) == m_PopUp );
        g_UiMgr->EndDialog( g_UiUserID, TRUE );
    }
    else
    {
        irect r = g_UiMgr->GetUserBounds( g_UiUserID );

        if( m_PopUp == NULL )
        {
            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
            m_PopUpResult = DLG_POPUP_IDLE;
        }

        r.Set( 0, 0, 300, 160 );

        m_PopUp->Configure( r,
            0.5f,
            g_StringTableMgr( "ui", "IDS_MAIN_MENU_ONLINE_XBOX"         ),
            g_StringTableMgr( "ui", "IDS_ONLINE_PLEASE_WAIT"    ),
            g_StringTableMgr( "ui", "IDS_ONLINE_CONNECT_INIT"   ) );

        m_PopUp->EnableBlackout( FALSE );

        SetConnectState( CONNECT_WAIT );
    }

}

//=========================================================================
void dlg_online_connect::UpdateActivateInit( void )
{
    SetConnectState( ACTIVATE_WAIT_DHCP );
}


//=========================================================================
void dlg_online_connect::UpdateAuthUser( void )
{
    if( m_PopUp == NULL )
    {
        irect r = g_UiMgr->GetUserBounds( g_UiUserID );
        m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
            ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
        m_PopUpResult = DLG_POPUP_IDLE;
        m_PopUp->EnableBlackout( FALSE );
    }

    //
    // This is the correct form of the return state of the matchmgr. At this point, more than one
    // status can be returned depending on why the matchmgr went idle. This will happen if there
    // is more than one user account, and the player needs to provide input, or needs to be presented
    // with other options (such as boot-to-dash etc).
    //

    if( g_MatchMgr.IsBusy() )
    {
        if( m_Timeout > 30.0f )
        {
            m_Done = FALSE;
            Failed( "IDS_SIGN_IN_NO_CONNECTION" );
            g_MatchMgr.SetState( MATCH_IDLE );
            return;
        }
        if( m_PopUp == NULL )
        {
            irect r = g_UiMgr->GetUserBounds( g_UiUserID );
            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
            m_PopUpResult = DLG_POPUP_IDLE;
        }
        //
        // User pressed cancel. This test has to be done prior to the reconfigure as
        // the configure function will clear out the popup result to idle.
        //
        if( m_PopUpResult != DLG_POPUP_IDLE )
        {
            m_PopUp = NULL;
            Failed( "IDS_ONLINE_CONNECT_ABORTED" );
            return;
        }
        irect r(0,0,300,160);

        xwstring MessageText  = g_StringTableMgr( "ui", "IDS_ONLINE_PLEASE_WAIT" );
        MessageText += "\n";
        MessageText += g_StringTableMgr( "ui", "IDS_ONLINE_CONNECT_MATCHMAKER" );
        MessageText += "\n";
        MessageText += g_StringTableMgr( "ui", "IDS_ONLINE_TIMEOUT" );
        MessageText += xwstring( xfs("%d",(30-(s32)m_Timeout)) );

        m_PopUp->Configure( r,
            g_StringTableMgr( "ui", "IDS_MAIN_MENU_ONLINE_XBOX" ), 
            FALSE, 
            FALSE, 
            FALSE, 
            MessageText,
            xwstring( "" ),
            &m_PopUpResult );

        m_PopUp->EnableBlackout( FALSE );
    }
    else
    {
        switch( g_MatchMgr.GetAuthStatus() )
        {
        case AUTH_STAT_CONNECTED:
            g_MatchMgr.SetUserStatus( BUDDY_STATUS_ONLINE );

            if( g_MatchMgr.GetOptionalMessage() == TRUE )
                SetConnectState( CONNECT_OPTIONAL_MESSAGE );
            else
                SetConnectState( CONNECT_CHECK_MOTD );
            break;
        case AUTH_STAT_SELECT_USER:
            {

                // destroy pop-up?
                if( m_PopUp )
                {
                    m_PopUp->Close();
                    m_PopUp = NULL;
                }

                // refresh the user account list
                RefreshUserList();
                g_UiMgr->DisableScreenHighlight();
                m_pUserList->SetSelection( 0 );

                m_pUserList ->SetFlag(ui_win::WF_VISIBLE, TRUE);
                m_pNavText  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
                GotoControl( (ui_control*)m_pUserList );

                // set highlight
                g_UiMgr->SetScreenHighlight( m_pUserList->GetPosition() );
                // wait for input
                SetConnectState( CONNECT_SELECT_USER );
                break;
            }
        case AUTH_STAT_NEED_UPDATE:
            SetConnectState( CONNECT_AUTO_UPDATE );
            break;
        case AUTH_STAT_INVALID_ACCOUNT:
            Failed( "IDS_ONLINE_CONNECT_MATCHMAKER_REFUSED" );
            break;
        case AUTH_STAT_URGENT_MESSAGE:
            SetConnectState( CONNECT_REQUIRED_MESSAGE );
            break;  
        case AUTH_STAT_CANNOT_CONNECT:            // DNAS failure on ps2, xbox may have other error problem.
            Failed( "IDS_SIGN_IN_NO_CONNECTION" );
            break;
        case AUTH_STAT_DISCONNECTED:
            Failed( "IDS_ONLINE_CHECK_CABLE" );
            break;
        case AUTH_STAT_SECURITY_FAILED:
            Failed( "IDS_ONLINE_SECURITY_FAILED" );
            break;
        case AUTH_STAT_SERVER_BUSY:
            Failed("IDS_ONLINE_CONNECT_MATCHMAKER_FAILED");
            break;
        default:
            ASSERT( FALSE );
            Failed( "IDS_SIGN_IN_NO_CONNECTION" );
            break;
        }
    }
}

