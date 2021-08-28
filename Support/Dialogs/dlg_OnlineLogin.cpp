//=========================================================================
//
//  dlg_OnlineLogin.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_text.hpp"
#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_listbox.hpp"
#include "ui\ui_dlg_vkeyboard.hpp"

#include "dlg_OnlineLogin.hpp"
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\MatchMgr.hpp"
#include "NetworkMgr/GameClient.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "Configuration/GameConfig.hpp"

#if defined(TARGET_PS2)
#include "IOManager\Device_DVD\io_device_cache.hpp"
#endif
//=========================================================================
//  Main Menu Dialog
//=========================================================================
enum login_controls
{
    IDC_LOGIN_NAV_TEXT,
};

ui_manager::control_tem LoginControls[] = 
{
    { IDC_LOGIN_NAV_TEXT,    "IDS_NULL",             "text",      0,   0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem LoginDialog =
{
    "IDS_ONLINE_LOGIN_TITLE",
    2, 1,
    sizeof(LoginControls)/sizeof(ui_manager::control_tem),
    &LoginControls[0],
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

void dlg_online_login_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "login", &LoginDialog, &dlg_online_login_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_online_login_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_online_login* pDialog = new dlg_online_login;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_online_login
//=========================================================================

dlg_online_login::dlg_online_login( void )
{
}

//=========================================================================

dlg_online_login::~dlg_online_login( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_online_login::Create( s32                        UserID,
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
    m_pNavText              = (ui_text*)    FindChildByID( IDC_LOGIN_NAV_TEXT );

    // hide them
    m_pNavText   ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_OK" ));
   
    m_pNavText->SetLabel( navText );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // set initial focus
    m_pPopup = NULL;

    // disable highlight
    g_UiMgr->DisableScreenHighlight();
    m_Position = Position;
    m_HasStartedConnect = FALSE;

    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_LoginState        = LOGIN_IDLE;
    m_FirstTimePassword = TRUE;

    m_JoinPasswordDone  = FALSE;
    m_JoinPasswordOK    = FALSE;

    SetState( DIALOG_STATE_ACTIVE );
    // Everything in the active configuration should be invalid. However, we do need
    // to reset the exit reason immediately as the cable disconnect/connection lost code
    // will kick in if the exit reason has changed.
    g_ActiveConfig.Invalidate();
    g_ActiveConfig.SetExitReason( GAME_EXIT_CONTINUE );
    // set the number of players to 1
    g_PendingConfig.SetPlayerCount( 1 );


    // Return success code
    return Success;
}

//=========================================================================

void dlg_online_login::Destroy( void )
{
    if( m_pPopup )
    {
        CloseDialog();
    }
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_online_login::Render( s32 ox, s32 oy )
{
    static s32 offset   =  0;
    static s32 gap      =  9;
    static s32 width    =  4;

	irect   rb;
	s32     XRes;
    s32     YRes;

    eng_GetRes( XRes, YRes );
#ifdef TARGET_PS2
    // Nasty hack to force PS2 to draw to rb.l = 0
    rb.Set( -1, 0, XRes, YRes );
#else
    rb.Set( 0, 0, XRes, YRes );
#endif
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

void dlg_online_login::OnPadSelect( ui_win* pWin )
{
    (void)pWin;
}

//=========================================================================

void dlg_online_login::OnPadBack( ui_win* pWin )
{
    (void)pWin;
    if( m_State == DIALOG_STATE_ACTIVE )
    {
        // Make sure matchmgr gives up on the login request.
        g_NetworkMgr.Disconnect();
        SetState( DIALOG_STATE_CANCEL );
        // Cancel login attempt
    }
}

//=========================================================================

void dlg_online_login::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    MEMORY_OWNER("dlg_online_login::OnUpdate");
    (void)pWin;
    (void)DeltaTime;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {            
            if( m_HasStartedConnect == FALSE )
            {
                SetLoginState( LOGIN_INITIATE_CONNECTION );
                m_HasStartedConnect = TRUE;
            }
        }
    }

    // This is used to make sure that we can actually have the display rendered with the dialog
    // in place because some of the operations we're about to perform will take a bit of time and
    // are atomic. 
    UpdateState( DeltaTime );

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    // update everything else
    ui_dialog::OnUpdate( pWin, DeltaTime );

}

//=========================================================================

void dlg_online_login::SetLoginState( dlg_login_state NewState )
{
    ASSERT( NewState != m_LoginState );
    if( NewState != m_LoginState )
    {
        LOG_MESSAGE( "dlg_online_login::SetLoginState",
                     "State transition from %s to %s (%s)",
                     GetStateName     ( m_LoginState ),
                     GetStateName     ( NewState ),
                     GetExitReasonName( g_ActiveConfig.GetExitReason() ) );

        ExitState( m_LoginState );
        m_LoginState = NewState;
        EnterState( NewState );
    }
}

//=========================================================================

void dlg_online_login::EnterState( dlg_login_state State )
{

    switch( State )
    {
    case LOGIN_IDLE:
        break;
    case LOGIN_INITIATE_CONNECTION:
        g_ActiveConfig.SetExitReason( GAME_EXIT_CONTINUE );
        g_PendingConfig.SetGameTypeID( GAME_MP );
        g_MatchMgr.StartIndirectLookup();
        ProgressDialog( "IDS_ONLINE_LOGIN_CONNECTING" );
        break;
    case LOGIN_ACQUIRE_PASSWORD:
        {
            // open a VK to enter the password
            irect r = m_pManager->GetUserBounds( m_UserID );
            ui_dlg_vkeyboard* pVKeyboard = (ui_dlg_vkeyboard*)m_pManager->OpenDialog( m_UserID, "ui_vkeyboard", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_INPUTMODAL );
            pVKeyboard->Configure( FALSE );
            pVKeyboard->SetLabel( g_StringTableMgr( "ui", "IDS_ENTER_PASSWORD" ) );
            pVKeyboard->ConnectString( &m_JoinPassword, NET_PASSWORD_LENGTH );
            pVKeyboard->SetReturn( &m_JoinPasswordDone, &m_JoinPasswordOK );
            m_JoinPasswordDone  = FALSE;
            m_JoinPasswordOK    = FALSE;
        }
        break;
    case LOGIN_PASSWORD_ERROR:
        {
            irect r = g_UiMgr->GetUserBounds( g_UiUserID );

            m_pNavText   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            if( m_pPopup == NULL )
            {
                m_pPopup = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                    ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
            }

            r.Set( 0, 0, 300, 160 );
            xwstring NavText;
            NavText  = g_StringTableMgr( "ui", "IDS_NAV_RETRY" );
            NavText += g_StringTableMgr( "ui", "IDS_NAV_CANCEL" );

            m_pPopup->Configure( r,                                         // Position
                g_StringTableMgr( "ui", "IDS_NETWORK_POPUP" ),              // Title 
                TRUE,                                                       // Yes
                TRUE,                                                       // No
                FALSE,                                                      // Maybe
                g_StringTableMgr( "ui", "IDS_ONLINE_LOGIN_BAD_PASSWORD" ),  // Message
                NavText,                                                    // Nav text
                &m_PopUpResult );                                           // Result
        }
        break;
    case LOGIN_WAIT_FOR_LOGIN:
        LOG_APP_NAME( "CLIENT" );
        g_NetworkMgr.SetLocalPlayerCount(1);
        g_PendingConfig.SetPassword( xstring(m_JoinPassword) );
        g_PendingConfig.SetGameTypeID( GAME_MP );
        game_config::Commit();
        g_MatchMgr.StartLogin();
        break;

    default:
        break;
    }
}

//=========================================================================

void dlg_online_login::ExitState( dlg_login_state State )
{
    (void)State;
}

//=========================================================================

void dlg_online_login::UpdateState( f32 DeltaTime )
{
    (void)DeltaTime;

    if( m_pPopup && (m_PopUpResult != DLG_POPUP_IDLE) )
    {
        // The popup dialog automatically closed itself
        m_pPopup = NULL;
    }

    switch( m_LoginState )
    {
        //---------------------------------------------
    case LOGIN_IDLE:
        if( m_PopUpResult != DLG_POPUP_IDLE )
        {
            //m_pNavText   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
        break;
        //---------------------------------------------
    case LOGIN_INITIATE_CONNECTION:
        UpdateInitConnection();
        break;

        //---------------------------------------------
    case LOGIN_WAIT_FOR_LOGIN:
        UpdateWaitForLogin();
        break;
        //---------------------------------------------
    case LOGIN_ACQUIRE_PASSWORD:
        UpdateAcquirePassword();
        break;

        //---------------------------------------------
    case LOGIN_ERROR:
        SetState( DIALOG_STATE_BACK );
        break;
        //---------------------------------------------
    case LOGIN_PASSWORD_ERROR:
        if( m_PopUpResult != DLG_POPUP_IDLE )
        {
            m_pPopup=NULL;
            if( m_PopUpResult == DLG_POPUP_YES )
            {
                SetLoginState( LOGIN_ACQUIRE_PASSWORD );
            }
            else 
            {
                SetState( DIALOG_STATE_CANCEL );
            }
        }
        break;
        //---------------------------------------------
    default:
        ASSERT(FALSE);
        break;
    }
}

//=========================================================================

void dlg_online_login::OkDialog( const char* pString, const char* pButton )
{
    irect r = g_UiMgr->GetUserBounds( g_UiUserID );

    m_pNavText   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    if( m_pPopup == NULL )
    {
        m_pPopup = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
            ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
    }
    
    r.Set( 0, 0, 300, 160 );

    m_pPopup->Configure( r,                                     // Position
        g_StringTableMgr( "ui", "IDS_NETWORK_POPUP" ),          // Title 
        TRUE,                                                   // Yes
        FALSE,                                                  // No
        FALSE,                                                  // Maybe
        g_StringTableMgr( "ui", pString ),                      // Message
        g_StringTableMgr( "ui", pButton ),                      // Nav text
        &m_PopUpResult );                                       // Result
}

//=========================================================================

void dlg_online_login::CloseDialog( void )
{
    if( m_pPopup )
    {
        ASSERT( g_UiMgr->GetTopmostDialog(m_UserID) == m_pPopup );
        g_UiMgr->EndDialog( m_UserID, TRUE );
        m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
        m_pPopup = NULL;
    }
}

//=========================================================================

void dlg_online_login::ProgressDialog( const char* pString, xbool AllowBackout )
{
    irect r = g_UiMgr->GetUserBounds( g_UiUserID );

    m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);


    if( m_pPopup == NULL )
    {
        m_pPopup = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
            ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
    }

    r.Set( 0, 0, 380, 160 );

    const xwchar* pCancelText;

    if( AllowBackout )
    {
        pCancelText = g_StringTableMgr( "ui", "IDS_NAV_CANCEL" );
    }
    else
    {
        pCancelText = g_StringTableMgr( "ui", "IDS_NULL" );
    }

    const xwchar* pServerName = g_PendingConfig.GetServerName();
    if( x_wstrlen(pServerName) )
    {
        m_ServerName = pServerName;
    }

    m_pPopup->Configure( r,                                                     // Position
                        m_ServerName,                                           // Title
                        FALSE,                                                  // Yes
                        AllowBackout,                                           // No
                        FALSE,                                                  // Maybe
                        g_StringTableMgr( "ui", pString          ),             // Message
                        pCancelText,                                            // Nav text
                        &m_PopUpResult );                                       // Result
}


//------------------------------------------------------------------------------
const char* dlg_online_login::GetStateName( dlg_login_state LoginState )
{
    switch( LoginState )
    {
    case LOGIN_IDLE:                    return "LOGIN_IDLE";
    case LOGIN_INITIATE_CONNECTION:     return "LOGIN_INITIATE_CONNECTION";
    case LOGIN_WAIT_FOR_LOGIN:          return "LOGIN_WAIT_FOR_LOGIN";
    case LOGIN_ERROR:                   return "LOGIN_ERROR";
    case LOGIN_PASSWORD_ERROR:          return "LOGIN_PASSWORD_ERROR";
    case LOGIN_ACQUIRE_PASSWORD:        return "LOGIN_WAIT_FOR_PASSWORD";
    default:                            return "<Unknown>";
    }
}

//------------------------------------------------------------------------------
void dlg_online_login::UpdateInitConnection( void )
{
    // Wait until we get a response: connected or failed
    if( g_MatchMgr.GetState() == MATCH_IDLE )
    {
        if( g_MatchMgr.GetConnectStatus() != MATCH_CONN_CONNECTING )
        {
            SetLoginState( LOGIN_ERROR );
        }
        else
        {
            SetLoginState( LOGIN_WAIT_FOR_LOGIN );
        }
    }
    // The user must have cancelled the login attempt
    if( (m_PopUpResult!=DLG_POPUP_IDLE) && (GetState()==DIALOG_STATE_ACTIVE) )
    {
        // Make sure matchmgr gives up on the login request.
        g_NetworkMgr.Disconnect();
        SetState( DIALOG_STATE_CANCEL );
    }
}


//------------------------------------------------------------------------------
void dlg_online_login::UpdateWaitForLogin( void )
{
    match_conn_status ConnectStatus;

    ConnectStatus = g_MatchMgr.GetConnectStatus();
    if( ConnectStatus == MATCH_CONN_CONNECTED )
    {
        client_state State = g_NetworkMgr.GetClientObject().GetState();
        if( State == STATE_CLIENT_LOAD_MISSION )
        {
            ASSERT( g_ActiveConfig.GetExitReason()==GAME_EXIT_CONTINUE );
            SetState( DIALOG_STATE_SELECT );
        }
    } 
    else if( ConnectStatus != MATCH_CONN_CONNECTING )
    {
        g_NetworkMgr.Disconnect();
        ASSERT( g_ActiveConfig.GetExitReason()!=GAME_EXIT_CONTINUE );

        if( g_ActiveConfig.GetExitReason()==GAME_EXIT_BAD_PASSWORD )
        {
            if( m_FirstTimePassword )
            {
                m_FirstTimePassword = FALSE;
                SetLoginState( LOGIN_ACQUIRE_PASSWORD );
            }
            else
            {
                SetLoginState( LOGIN_PASSWORD_ERROR );
            }
        }
        else
        {
            SetLoginState( LOGIN_ERROR );
        }
    }

    // The user must have cancelled the login attempt
    if( (m_PopUpResult!=DLG_POPUP_IDLE) && (GetState()==DIALOG_STATE_ACTIVE) )
    {
        // Make sure matchmgr gives up on the login request.
        g_NetworkMgr.Disconnect();
        SetState( DIALOG_STATE_CANCEL );
    }
}

//------------------------------------------------------------------------------
void dlg_online_login::UpdateAcquirePassword( void )
{
    if( m_JoinPasswordDone )
    {
        if( m_JoinPasswordOK )
        {
            SetLoginState( LOGIN_INITIATE_CONNECTION );
        }
        else
        {
            SetState( DIALOG_STATE_CANCEL );
        }
    }
}

//------------------------------------------------------------------------------
void dlg_online_login::SetState( dialog_states State )
{
    m_State = State;
    CloseDialog();
}