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

#include "dlg_OnlineConnect.hpp"
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\MatchMgr.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "MemCardMgr/MemCardMgr.hpp"
#include "e_memcard.hpp"
#include "e_Network.hpp"

#if defined(CONFIG_VIEWER)
#include "../../Apps/ArtistViewer/Config.hpp"
#else
#include "../../Apps/GameApp/Config.hpp"
#endif

extern void InitFrontEndMusic( void );
extern void KillFrontEndMusic( void );
//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum online_connect_controls
{
    IDC_ONLINE_CONNECT_LISTBOX,
    IDC_ONLINE_CONNECT_NAV_TEXT,
    IDC_ONLINE_CONFIG_LISTBOX,
};


ui_manager::control_tem OnlineConnectControls[] = 
{
    { IDC_ONLINE_CONNECT_LISTBOX,   "IDS_SIGN_IN",          "listbox",  70,  60, 220, 238, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_CONFIG_LISTBOX,    "IDS_SELECT_CONFIG",    "listbox",  26,  60, 368, 238, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
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
//
// Note: I may move all this crap in to the network layer.
//
static const char* s_ConfigDeviceList[]=
{
        "mc0:/BWNETCNF/BWNETCNF",
        "mc1:/BWNETCNF/BWNETCNF",
        //"pfs0:/BWNETCNF/BWNETCNF",
        NULL,
};
s32 s_LastCardStatus[2];
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
//  dlg_online_connect
//=========================================================================

dlg_online_connect::dlg_online_connect( void )    
{    
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
    m_pConfigList   = (ui_listbox*) FindChildByID( IDC_ONLINE_CONFIG_LISTBOX );

    // hide them
    m_pConfigList->SetFlag(ui_win::WF_VISIBLE, FALSE);
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

    // set up configuration list
    m_pConfigList->SetLineHeight( 32 );
    m_pConfigList->SetFlag(ui_win::WF_SELECTED, TRUE);
    m_pConfigList->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pConfigList->DisableFrame();
    m_pConfigList->SetExitOnSelect(FALSE);
    m_pConfigList->SetExitOnBack(TRUE);
    m_pConfigList->EnableHeaderBar();
    m_pConfigList->SetHeaderBarColor( xcolor(19,59,14,196) );
    m_pConfigList->SetHeaderColor( xcolor(255,252,204,255) );

    // set initial focus
    GotoControl( (ui_control*)m_pUserList );
    m_CurrentControl = IDC_ONLINE_CONNECT_LISTBOX;
    m_PopUp = NULL;

    // initialize connect state
    m_ConnectState = CONNECT_IDLE;

    // load bitmaps
    m_DNASIconID[LOGO_DNAS_OK]    = g_UiMgr->LoadBitmap( "DNAS_OK",     "UI_DNAS_OK.xbmp"      );
    m_DNASIconID[LOGO_DNAS_ERROR] = g_UiMgr->LoadBitmap( "DNAS_ERROR",  "UI_DNAS_ERROR.xbmp"   );

    // initialize DNAS logo controls
    m_bRenderLogo[LOGO_DNAS_OK]     = FALSE;
    m_bIsFading[LOGO_DNAS_OK]       = FALSE;
    m_bFadeIn[LOGO_DNAS_OK]         = FALSE;
    m_LogoAlpha[LOGO_DNAS_OK]       = 0.0f;

    m_bRenderLogo[LOGO_DNAS_ERROR]  = FALSE;
    m_bIsFading[LOGO_DNAS_ERROR]    = FALSE;
    m_bFadeIn[LOGO_DNAS_ERROR]      = FALSE;
    m_LogoAlpha[LOGO_DNAS_ERROR]    = 0.0f;


    // disable highlight
    g_UiMgr->DisableScreenHighlight();
    m_Position = Position;

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

    // unload bitmaps
    g_UiMgr->UnloadBitmap( "DNAS_OK"    );
    g_UiMgr->UnloadBitmap( "DNAS_ERROR" );
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
    // Nasty hack to force PS2 to draw to rb.l = 0
    rb.Set( -1, 0, XRes, YRes );
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

    // render DNAS logos
    s32 tempW = (m_CurrPos.l + m_CurrPos.r) / 2;
#if defined(TARGET_PS2)
    xbool PalMode;
    irect textPos;
    eng_GetPALMode( PalMode );
    if( PalMode )
    {
        textPos.Set( tempW-150, 370, tempW+150, 50 );
    }
    else
    {
        textPos.Set( tempW-150, 320, tempW+150, 50 );
    }
#else
	irect textPos;
	textPos.Set( tempW-150, 320, tempW+150, 50 );
#endif	

    // Now render any logos that should be on screen.
    rb.l = tempW - 64;
    rb.r = tempW + 64;
    rb.t = m_CurrPos.t + 10;
    rb.b = m_CurrPos.t + 10 + 128;

    for( s32 l=0; l<NUM_DNAS_LOGOS; l++ )
    {
        if( m_bRenderLogo[l] )
        {
            xcolor LogoColor( 255, 255, 255, (u8)m_LogoAlpha[l] );
            g_UiMgr->RenderBitmap( m_DNASIconID[l], rb, LogoColor );
            g_UiMgr->RenderText_Wrap( 1, textPos, ui_font::h_center|ui_font::v_top, xcolor(255,252,204,(u8)m_LogoAlpha[l]), g_StringTableMgr("ui", "IDS_DNAS_SONY_MESSAGE") );
        }
    }


    // render the glow bar
    g_UiMgr->RenderGlowBar();
}

//=========================================================================

void dlg_online_connect::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    // check for match manager busy
    if( g_MatchMgr.IsBusy() || g_StateMgr.IsBackgroundThreadRunning() )
    {
        // wait for the user accounts to populate the listbox
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
        {
            //
            // Since we can only associate one value with a particular item, let's just
            // cram two things in to an s32. Upper 16 bits, device index, lower 16 bits,
            // configuration index.
            //
            InitScreenScaling( m_Position ); // Resize dialog back to original width
            g_UiMgr->DisableScreenHighlight();
            m_pConfigList->SetFlag( ui_win::WF_VISIBLE, FALSE );
            m_pNavText->SetFlag( ui_win::WF_VISIBLE, FALSE );
            //
            // Did the memory card status change?
            //
            {
                s32 i;
                for( i=0; i<2; i++ )
                {
                    s32 Status = g_MemcardHardware.GetCardStatus( i );
                    if( Status != s_LastCardStatus[i] )
                    {
                        s_LastCardStatus[i]=Status;

                        // Stealing the text messages used in autosave. This is used to inform
                        // the user the status of the cards changed.
                        if( i==0 )
                        {
                            Failed( "IDS_AUTOSAVE_FAILED_CARD_CHANGED_SLOT_1", 0, CANCEL_RETRY_MANAGE, ACTIVATE_INIT );
                        }
                        else
                        {
                            Failed( "IDS_AUTOSAVE_FAILED_CARD_CHANGED_SLOT_2", 0, CANCEL_RETRY_MANAGE, ACTIVATE_INIT );
                        }
                        return;
                    }
                }
            }

            s32 DeviceIndex = m_pConfigList->GetSelectedItemData()>>16;
            s32 ConfigIndex = m_pConfigList->GetSelectedItemData() & 0xffff;
            // Yes, I know. Bad. We should not be using a hardcoded upperbound. Who cares eh?
            ASSERT( (DeviceIndex >= 0) && (DeviceIndex < 3) );

            // Just use the first one.
            xtimer t;
            t.Start();
            const char* pCardCheckString;
            if( DeviceIndex==0 )
            {
                pCardCheckString = "MC_CARD_CHECK_SLOT1";
            }
            else
            {
                pCardCheckString = "MC_CARD_CHECK_SLOT2";
            }

            irect r = irect(0,0,300,160);
            if( m_PopUp==NULL )
            {
                m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                    ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
            }

            m_PopUp->Configure( r,
                -1.0f,
                g_StringTableMgr( "ui", "IDS_NETWORK_POPUP"     ),
                g_StringTableMgr( "ui",  pCardCheckString ),
                xwstring( "" ) );
            
            g_StateMgr.StartBackgroundRendering();
            m_Status = net_SetConfiguration( s_ConfigDeviceList[DeviceIndex], ConfigIndex );
            g_StateMgr.StopBackgroundRendering();
            t.Stop();
            LOG_MESSAGE( "dlg_online_connect::OnSelect", "SetConfiguration took %2.02fms", t.ReadMs() );
            if( m_Status < 0 )
            {
                switch( m_Status )
                {
                case NET_ERR_OTHER_SYSTEM:
                    Failed("IDS_ONLINE_CONFIG_OTHER_SYSTEM", DeviceIndex+1);
                    break;
                case NET_ERR_NO_CONFIGURATION:
                case NET_ERR_NO_CARD:
                    Failed("IDS_ONLINE_CONFIG_NONE_PRESENT", 0, CANCEL_RETRY_MANAGE, CONNECT_MATCH_INIT );
                    break;
                case NET_ERR_NO_HARDWARE:
                    Failed( "IDS_ONLINE_CONFIG_NO_HARDWARE", 0, OK_ONLY );
                    break;
                default:
                    // We should not get any errors we don't know about
                    ASSERT( FALSE );
                    Failed("IDS_ONLINE_CONFIG_NONE_PRESENT");
                    break;
                }
            }
            else
            {
                SetConnectState( ACTIVATE_LOOP );
                m_Done = FALSE;
                m_Error = 0;
                m_Timeout = 0.0f;
            }
            m_pConfigList->SetFlag( ui_win::WF_VISIBLE, FALSE );
            m_pNavText->SetFlag( ui_win::WF_VISIBLE, FALSE );
            return;
        }
        break;
        //------------------------------------------------------
    case CONNECT_SELECT_USER:
        {
            // check if this is an existing user
            // get the profile index from the list
            s32 index = m_pUserList->GetSelectedItemData();
            if ( index < m_NumUsers ) //user exists 
            {
                // tell the match manager to start authentication
                g_MatchMgr.SetUserAccount( index );

                // try to connect using this user account
                g_MatchMgr.SetState( MATCH_CONNECT_MATCHMAKER );
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
    if( m_ConnectState==ACTIVATE_SELECT )
    {
        if( g_StateMgr.IsBackgroundThreadRunning() )
            return;

        // check for menu automation
        if( CONFIG_IS_AUTOSERVER || CONFIG_IS_AUTOCLIENT )
        {
            return;
        }

        if( GetState() == DIALOG_STATE_ACTIVE )
        {
            SetState( DIALOG_STATE_BACK );

            m_pNavText   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            // cancel connecting
            //g_AudioMgr.Play( "OptionBack" );
        }
    }
}

//=========================================================================

void dlg_online_connect::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    s32 Result = m_PopUpResult;
    // Limit deltatime size if we're debugging
    if( DeltaTime > 0.25f )
    {
        DeltaTime = 1.0f/30.0f;
    }
    m_Timeout += DeltaTime;
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
                irect r( 0, -10, 320, 290 );

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
                SetConnectState( CONNECT_INIT );
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
                LOG_MESSAGE( "dlg_online_connect::OnUpdate", "Opening dialog. ConnectState:%d", m_ConnectState );
                m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                    ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
                m_PopUpResult = DLG_POPUP_IDLE;

                r = irect(0,0,300,160);
                m_PopUp->Configure( r,
                    -1.0f,
                    g_StringTableMgr( "ui", "IDS_NETWORK_POPUP"     ),
                    g_StringTableMgr( "ui", "IDS_ONLINE_PLEASE_WAIT"),
                    g_StringTableMgr( "ui", "IDS_ONLINE_CONNECT_INIT") );

                m_PopUp->EnableBlackout( FALSE );               
            }
        }
    }
    else
    {
        // update DNAS logo fading
        for( s32 l=0; l<NUM_DNAS_LOGOS; l++ )
        {
            if( m_bIsFading[l] )
            {
                if( m_bFadeIn[l] )
                {
                    // fade in
                    m_LogoAlpha[l] += (m_FadeStep[l] * DeltaTime );

                    if( m_LogoAlpha[l] >= 255.0f )
                    {
                        m_LogoAlpha[l] = 255.0f;

                        if( l == LOGO_DNAS_ERROR )
                        {
                            // now fade out 
                            m_bIsFading[LOGO_DNAS_OK]       = TRUE;
                            m_bFadeIn[LOGO_DNAS_OK]         = FALSE;
                            m_LogoAlpha[LOGO_DNAS_OK]       = 255;
                            m_FadeStep[LOGO_DNAS_OK]        = 256.0f / 0.8f;

                            m_bIsFading[LOGO_DNAS_ERROR]    = TRUE;
                            m_bFadeIn[LOGO_DNAS_ERROR]      = FALSE;
                            m_LogoAlpha[LOGO_DNAS_ERROR]    = 255;
                            m_FadeStep[LOGO_DNAS_ERROR]     = 256.0f / 0.8f;
                        }
                        else
                        {
                            m_bIsFading[l] = FALSE;
                        }
                    }
                }
                else
                {
                    // fade out
                    m_LogoAlpha[l] -= (m_FadeStep[l] * DeltaTime );

                    if( m_LogoAlpha[l] <= 0.0f )
                    {
                        m_LogoAlpha[l]   = 0.0f;
                        m_bIsFading[l]   = FALSE;
                        m_bRenderLogo[l] = FALSE;
                    }
                }
            }
        }
        if( g_StateMgr.IsBackgroundThreadRunning() )
        {
            // update the glow bar
            g_UiMgr->UpdateGlowBar(DeltaTime);
            return;
        }

        if( m_PopUp && (m_PopUpResult!=DLG_POPUP_IDLE) )
        {
            m_PopUp = NULL;
        }
        if( ( g_UiMgr->IsWipeActive() == FALSE ) && ( m_bIsFading[LOGO_DNAS_OK] == FALSE ) && ( m_bIsFading[LOGO_DNAS_ERROR] == FALSE ) )
        {
            // window is scaled to correct size - do the main update
            switch( m_ConnectState )
            {
                //------------------------------------------
            case CONNECT_IDLE:
            case CONNECT_SELECT_USER:
                // wait for state change (user account selected)
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

                    m_PopUp->Configure( r,
                        0.25f,
                        g_StringTableMgr( "ui", "IDS_NETWORK_POPUP"            ),
                        g_StringTableMgr( "ui", "IDS_ONLINE_PLEASE_WAIT"       ),
                        g_StringTableMgr( "ui", "IDS_ONLINE_CONNECT_ESTABLISH" ) );

                    m_PopUp->EnableBlackout( FALSE );

                    SetConnectState( ACTIVATE_INIT );

                    // *** NOTE *** we must be very careful where we kick off the background
                    // rendering.
                    KillFrontEndMusic();
                    g_StateMgr.StartBackgroundRendering();
                    xtimer t;

                    t.Start();
                    g_NetworkMgr.SetOnline( TRUE );
                    t.Stop();
                    LOG_MESSAGE( "dlg_online_connect::OnUpdate", "SetOnline() call took %2.02fms", t.ReadMs() );
                    g_StateMgr.StopBackgroundRendering();
                    InitFrontEndMusic();
                }
                break;

                //------------------------------------------
            case ACTIVATE_INIT:
                UpdateActivateInit();
                break;

                //------------------------------------------
            case ACTIVATE_SELECT:
                ASSERT( g_StateMgr.IsBackgroundThreadRunning() == FALSE );
                m_pConfigList->SetFlag( ui_win::WF_VISIBLE, TRUE );
                m_pNavText->SetFlag( ui_win::WF_VISIBLE, TRUE );
                g_UiMgr->SetScreenHighlight( m_pConfigList->GetPosition() );
                break;

                //------------------------------------------
            case ACTIVATE_LOOP:
                {
                    if( m_Timeout < 30.0f )
                    {
                        m_Error = 0;
                        m_Error = net_GetAttachStatus(m_Error);

                        if ( (m_Error==ATTACH_STATUS_CONFIGURED) ||
                            (m_Error==ATTACH_STATUS_ATTACHED) )
                        {
                            net_ActivateConfig(TRUE);
                            // This will force us to load a patch from the memory card if it is present.
                            //*** This is only for PS2 ***
                            SetConnectState( ACTIVATE_WAIT_DHCP );
                        }
                        else
                        {
                            // Invalid net config file - its fatal!
                            Failed("IDS_ONLINE_CONFIG_OTHER_SYSTEM", m_LastErrorSlot+1);
                        }
                    }
                    else
                    {
                        // timed out!
                        Failed("IDS_ONLINE_CONNECT_MATCHMAKER_FAILED");
                    }

                }
                break;

                //------------------------------------------
            case ACTIVATE_WAIT_DHCP:
                {
                    // Wait until DHCP assigns us an address
                    if( m_Timeout < 30.0f )
                    {
                        irect r( 0, 0, 300, 160 );

                        //
                        // User pressed cancel. This test has to be done prior to the reconfigure as
                        // the configure function will clear out the popup result to idle.
                        //
                        if( Result != DLG_POPUP_IDLE )
                        {
                            m_PopUp = NULL;
                            Failed( "IDS_ONLINE_CONNECT_ABORTED", 0, OK_ONLY );
                            break;
                        }

                        if( m_PopUp == NULL )
                        {
                            LOG_MESSAGE( "dlg_online_connect::OnUpdate", "Opening dialog (ACTIVATE_WAIT_DHCP)" );
                            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                                ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
                            m_PopUpResult = DLG_POPUP_IDLE;
                        }

                        xwstring MessageText  = g_StringTableMgr( "ui", "IDS_ONLINE_PLEASE_WAIT" );
                        MessageText += "\n";
                        MessageText += g_StringTableMgr( "ui", "IDS_ONLINE_CONNECT_CONNECTING" );
                        MessageText += "\n";
                        MessageText += g_StringTableMgr( "ui", "IDS_ONLINE_TIMEOUT" );
                        MessageText += xwstring( xfs("%d",(30-(s32)m_Timeout)) );

                        m_PopUp->Configure( r,
                            g_StringTableMgr( "ui", "IDS_NETWORK_POPUP" ), 
                            FALSE, 
                            TRUE, 
                            FALSE, 
                            MessageText,
                            g_StringTableMgr( "ui", "IDS_NAV_CANCEL" ),
                            &m_PopUpResult );

                        m_PopUp->EnableBlackout( FALSE );

                        net_GetInterfaceInfo( -1, m_Info );
                        if( m_Info.Address )
                        {
                            SetConnectState( CONNECT_MATCH_INIT );
                            break;
                        }
                        if( m_Info.IsAvailable==FALSE )
                        {
                            Failed( "IDS_ONLINE_CHECK_CABLE", 0, OK_ONLY );
                            break;
                        }

                        s32 Error;

                        Error = net_GetAttachStatus(Error);
                        if( Error==ATTACH_STATUS_ERROR )
                        {
                            connect_status ConnStatus;

                            net_GetConnectStatus( ConnStatus );
                            // Literal error text from the connection
                            if( ConnStatus.ErrorText[0] )
                            {
                                Failed( ConnStatus.ErrorText );
                            }
                            else
                            {
                                Failed( "IDS_ONLINE_CANNOT_CONNECT" );
                            }
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
                if( g_StateMgr.IsBackgroundThreadRunning() )
                {
                    return;
                }

                {
                    irect r( 0, 0, 300, 160 );

                    if( m_PopUp==NULL )
                    {
                        LOG_MESSAGE( "dlg_online_connect::UpdateAuthMachine", "Opening dialog" );
                        m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                            ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
                        m_Timeout = 0.0f;
                        m_PopUpResult = DLG_POPUP_IDLE;
                    }

                    // fade in DNAS logo
                    m_bRenderLogo[LOGO_DNAS_OK]     = TRUE;
                    m_bIsFading[LOGO_DNAS_OK]       = TRUE;
                    m_bFadeIn[LOGO_DNAS_OK]         = TRUE;
                    m_LogoAlpha[LOGO_DNAS_OK]       = 0.0f;
                    m_FadeStep[LOGO_DNAS_OK]        = 256.0f / 0.8f;

                    xwstring MessageText  = g_StringTableMgr( "ui", "IDS_ONLINE_PLEASE_WAIT" );
                    MessageText += "\n";
                    MessageText += g_StringTableMgr( "ui", "IDS_ONLINE_CONNECT_AUTHENTICATING" );

                    m_PopUp->Configure( r,
                        g_StringTableMgr( "ui", "IDS_NETWORK_POPUP" ), 
                        FALSE, 
                        FALSE, 
                        FALSE, 
                        MessageText,
                        xwstring(""),
                        &m_PopUpResult );
                    m_PopUp->EnableBlackout( FALSE );

                    g_StateMgr.StartBackgroundRendering();
                    net_EndConfig();

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
                        m_Done = g_MatchMgr.Init( LocalSocket, Broadcast );
                    }
                    g_MatchMgr.SetState( MATCH_AUTHENTICATE_MACHINE );
                    g_StateMgr.StopBackgroundRendering();

                    m_Timeout = 0.0f;

                    SetConnectState( CONNECT_AUTHENTICATE_MACHINE );
                }
                break;

                if( m_PopUp == NULL )
                {
                }
                break;

                //------------------------------------------
            case CONNECT_AUTHENTICATE_MACHINE:
                UpdateAuthMachine();
                break;

                //------------------------------------------
            case CONNECT_AUTHENTICATE_USER:
                UpdateAuthUser();
                break;

                //------------------------------------------
            case CONNECT_FAILED:
                {
                    xbool bEnableYes = FALSE;
                    xbool bEnableNo = FALSE;
                    xbool bEnableMaybe = FALSE;


                    //SetState(  DIALOG_STATE_BACK;
                    //g_UiMgr->EndDialog( g_UiUserID, TRUE );
                    //return;

                    // set nav text
                    xwstring navText;

                    // don't want "Manage" option coming up.
                    switch( m_CancelMode )
                    {
                    case CANCEL_MANAGE:
                        navText = g_StringTableMgr( "ui", "IDS_NAV_NETWORK_TROUBLESHOOTER" );
                        navText += g_StringTableMgr( "ui", "IDS_NAV_SIGN_OUT" );
                        bEnableYes = TRUE;
                        bEnableNo = TRUE;
                        break;
                    case OK_ONLY:
                        navText = g_StringTableMgr( "ui", "IDS_NAV_OK" );
                        bEnableYes = TRUE;
                        break;
                    case CANCEL_RETRY_MANAGE:
                        navText = g_StringTableMgr( "ui", "IDS_NAV_NETWORK_TROUBLESHOOTER" );   // X
                        navText += g_StringTableMgr( "ui", "IDS_NAV_CONNECT_RETRY" );           // Square
                        navText += g_StringTableMgr( "ui", "IDS_NAV_SIGN_OUT" );                // Triangle
                        bEnableYes = TRUE;
                        bEnableNo = TRUE;
                        bEnableMaybe = TRUE;
                        break;

                    }

                    // pop up modal dialog and wait for response
                    irect r( 0, 0, 340, 260 );

                    xstring LabelText( g_StringTableMgr( "ui", m_LabelText ) );
                    xwstring ErrorText( xfs(LabelText, m_LastErrorCode ) );
                    xwstring Title( g_StringTableMgr( "ui", "IDS_SIGN_IN") );
                    // ** HACK ALERT **
                    // This is a hack to make sure the error dialog title matches the type of error.
                    // if the error message says "DNAS Error", we change the default title text 
                    // from "Network Error" to "DNAS Error". 
                    if( ErrorText.Find( g_StringTableMgr( "ui", "IDS_DNAS_ERROR")) != -1 )
                    {
                        Title = g_StringTableMgr( "ui", "IDS_DNAS_ERROR" );
                    }

                    if( m_PopUp==NULL )
                    {
                        m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                            ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
                    }
                    m_PopUp->Configure( r,
                        Title, 
                        bEnableYes, 
                        bEnableNo, 
                        bEnableMaybe, 
                        ErrorText,
                        navText,
                        &m_PopUpResult );

                    m_PopUp->EnableBlackout( FALSE );
                    SetConnectState( CONNECT_FAILED_WAIT );
                }
                break;

                //------------------------------------------
            case CONNECT_FAILED_WAIT:
                m_PopUpResult = DLG_POPUP_IDLE;
                // wait for response
                if( Result != DLG_POPUP_IDLE )
                {
                    switch( m_CancelMode )
                    {
                    case OK_ONLY:
                        SetConnectState( CONNECT_DISCONNECT );
                        break;
                    case CANCEL_MANAGE:
                        if( Result==DLG_POPUP_YES )
                        {
                            g_StateMgr.Reboot( REBOOT_MANAGE );
                            ASSERT( FALSE );
                        }
                        else
                        {
                            SetConnectState( CONNECT_DISCONNECT );
                        }
                        break;
                    case CANCEL_RETRY_MANAGE:
                        if( Result==DLG_POPUP_YES )
                        {
                            g_StateMgr.Reboot( REBOOT_MANAGE );
                            ASSERT( FALSE );
                        }
                        else if( Result==DLG_POPUP_NO )
                        {
                            SetConnectState( CONNECT_DISCONNECT );
                        }
                        else
                        {
                            SetConnectState( m_RetryDestination );
                        }
                        break;
                    }
                }
                break;

                //------------------------------------------
            case CONNECT_DISCONNECT:
                {
                    if( g_StateMgr.IsBackgroundThreadRunning() )
                    {
                        return;
                    }

                    irect r( 0, 0, 300, 260 );
                    xstring LabelText;

                    LabelText = g_StringTableMgr( "ui", m_LabelText );
                    xwstring ErrorText( xfs(LabelText, m_LastErrorCode ) );
                    xwstring Title( g_StringTableMgr( "ui", "IDS_SIGN_IN") );

                    // ** HACK ALERT **
                    // This is a hack to make sure the error dialog title matches the type of error.
                    // if the error message says "DNAS Error", we change the default title text 
                    // from "Network Error" to "DNAS Error". 
                    if( ErrorText.Find( g_StringTableMgr( "ui", "IDS_DNAS_ERROR")) != -1 )
                    {
                        Title = g_StringTableMgr( "ui", "IDS_DNAS_ERROR" );
                    }

                    ASSERT( m_PopUp==NULL );
                    if( m_PopUp )
                    {
                        g_UiMgr->EndDialog( g_UiUserID, TRUE );
                        m_PopUp = NULL;
                    }
                    LOG_MESSAGE( "dlg_online_connect::OnUpdate", "Opening dialog (CONNECT_FAILED)" );
                    m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                        ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
                    m_PopUpResult = DLG_POPUP_IDLE;

                    m_PopUp->Configure( r,
                        Title, 
                        FALSE, 
                        FALSE, 
                        FALSE, 
                        ErrorText,
                        g_StringTableMgr( "ui", "IDS_ONLINE_PLEASE_WAIT" ),                   // Nav text will be PLEASE WAIT.
                        &m_PopUpResult );

                    m_PopUp->EnableBlackout( FALSE );

                    m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);
                    KillFrontEndMusic();
                    g_StateMgr.StartBackgroundRendering();
                    net_ActivateConfig( FALSE );
                    g_NetworkMgr.SetOnline( FALSE );
                    g_StateMgr.StopBackgroundRendering();
                    //
                    // Reset exit reason so that we don't get two messages telling us the network is down.
                    //
                    g_ActiveConfig.SetExitReason( GAME_EXIT_CONTINUE );
                    InitFrontEndMusic();

                    // Once we have actually disconnected, then we go back.
                    SetState( DIALOG_STATE_BACK );
                }
                break;
                //------------------------------------------
            case CONNECT_AUTO_UPDATE:
                {
                    // A required update to the XBox live service

                    // set nav text
                    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_UPDATE_GAME" ));
                    navText += g_StringTableMgr( "ui", "IDS_NAV_SIGN_OUT" );

                    // pop up modal dialog and wait for response
                    irect r( 0, 0, 300, 160 );

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
                        m_PopUp = NULL;
                        if( m_PopUpResult == DLG_POPUP_YES )
                        {
                            g_StateMgr.Reboot( REBOOT_UPDATE );
                            // boot to xbox dashboard
                        }
                        else
                        {
                            // sign out
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
                    navText += g_StringTableMgr( "ui", "IDS_NAV_SIGN_OUT" );

                    // pop up modal dialog and wait for response
                    irect r( 0, 0, 300, 160 );

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
                        m_PopUp = NULL;
                        if( m_PopUpResult == DLG_POPUP_YES )
                        {
                            g_StateMgr.Reboot( REBOOT_MESSAGE );
                            // boot to xbox dashboard
                        }
                        else
                        {
                            // sign out
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
                    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_READ_LATER" ));
                    navText += g_StringTableMgr( "ui", "IDS_NAV_READ_MESSAGES" );

                    // pop up modal dialog and wait for response
                    irect r( 0, 0, 300, 160 );

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
                        m_PopUp = NULL;
                        if ( m_PopUpResult == DLG_POPUP_YES )
                        {
                            // continue with sign on
                        }
                        else
                        {
                            // boot to xbox dashboard
                        }
                    }
                }
                break;

                //------------------------------------------
            case CONNECT_DONE:

                if( m_LogoAlpha[LOGO_DNAS_OK] <= 0.0f )
                {
                    ASSERT( m_PopUp==NULL );
                    irect r(0,0,300,160);

                    m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                        ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );

                    xwstring MessageText( g_StringTableMgr( "ui", "IDS_ONLINE_CONNECTED" ) );

                    interface_info Info;

                    net_GetInterfaceInfo( -1, Info );
                    MessageText += "\n";
                    MessageText += g_StringTableMgr( "ui", "IDS_ONLINE_IP_ADDRESS" );
                    MessageText += " ";
                    MessageText += net_address( Info.Address, 0 ).GetStrIP();
                    MessageText += ".";

                    m_PopUp->Configure( r,
                        g_StringTableMgr( "ui", "IDS_NETWORK_POPUP" ), 
                        TRUE, 
                        FALSE, 
                        FALSE, 
                        MessageText,
                        g_StringTableMgr( "ui", "IDS_NAV_OK" ),
                        &m_PopUpResult );
                    m_PopUp->EnableBlackout( FALSE );
                    SetConnectState( CONNECT_DONE_WAIT );
                }
                break;
            case CONNECT_DONE_WAIT:
                if( CONFIG_IS_AUTOSERVER || CONFIG_IS_AUTOCLIENT )
                {
                    g_UiMgr->EndDialog( g_UiUserID, TRUE );
                    m_PopUp = NULL;
                    SetState( DIALOG_STATE_EXIT );
                    return;
                }

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
                    Position.Inflate( 80, 0 );

                    InitScreenScaling( Position );
                    m_PopUpResult = DLG_POPUP_IDLE;

                    // set state
                }
                else
                {
                    ASSERT( g_UiMgr->GetTopmostDialog( g_UiUserID ) == m_PopUp );
                    g_UiMgr->EndDialog( g_UiUserID, TRUE );
                    SetState( DIALOG_STATE_EXIT );
                    return;
                }
                break;
                //------------------------------------------
            case CONNECT_DISPLAY_MOTD:
                if( m_PopUpResult != DLG_POPUP_IDLE )
                {
                    m_PopUp = NULL;
                    // Dialog will self close.
                    SetState( DIALOG_STATE_EXIT );
                }
                break;
            }
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

    // get number user accounts from network mgr.
    m_NumUsers = g_MatchMgr.GetUserAccountCount();

    // clear the list
    m_pUserList->DeleteAllItems();

    // fill it with the user information
    for( i=0; i<m_NumUsers; i++ )
    {
        // add the profile to the list
        const online_user& Profile = g_MatchMgr.GetUserAccount(i);
        m_pUserList->AddItem( Profile.Name, i );
    }    

    // add an empty slot
    m_pUserList->AddItem( g_StringTableMgr("ui", "IDS_SIGN_IN_NEW_ACCOUNT"), m_NumUsers );

}

//=========================================================================
// Will give option of managing or dropping back to main menu
void dlg_online_connect::Failed( const char* pFailureReason, s32 ErrorCode, cancel_mode CancelMode, connect_states RetryDestination )
{
    (void)ErrorCode;

    if( g_StateMgr.IsBackgroundThreadRunning() )
    {
        g_StateMgr.StopBackgroundRendering();
    }

    LOG_MESSAGE( "dlg_online_connect::Failed", "Failure Reason:%s", pFailureReason );
    x_strcpy( m_LabelText, pFailureReason);

    m_LastErrorCode     = ErrorCode;
    m_CancelMode        = CancelMode;
    m_RetryDestination  = RetryDestination;

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
    case CONNECT_DONE_WAIT:             return "CONNECT_DONE_WAIT";
    case CONNECT_CHECK_MOTD:            return "CONNECT_CHECK_MOTD";
    case CONNECT_DISPLAY_MOTD:          return "CONNECT_DISPLAY_MOTD";
    case CONNECT_DISCONNECT:            return "CONNECT_DISCONNECT";
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
    switch( ConnectMode )
    {
    case CONNECT_MODE_AUTH_USER:
        // Start the matchmaker authenticating the user, then wait for the authentication 
        // to complete. This is not necessarily the prettiest way to do this but we are 
        // splitting up what was the authentication process in to two stages now since the 
        // profile needs to be picked first. XBOX Live may need to skip this stage as
        // the user, at this point, should already be authenticated as they select their
        // 'profile' seperately from their saved profile.
        g_MatchMgr.SetState( MATCH_CONNECT_MATCHMAKER );
        SetConnectState( CONNECT_AUTHENTICATE_USER );
        break;
    case CONNECT_MODE_CONNECT:
        // Set up default unique id from the hardware ID prior to
        // getting authentication from DNAS.
        SetConnectState( CONNECT_INIT );
        break;
    default:
        ASSERT(FALSE);
    }
}

//=========================================================================
s32 dlg_online_connect::PopulateConfigurationList( void )
{
    // We go through our list of potential devices holding the configuration, then we add
    // each to the list of configurations. When we return, we will respond with the # of
    // configs found. If there are 0, then we give an error. If there is 1, then we will
    // automatically connect with that one. Otherwise, the calling code will put up a dialog
    // with the options to select from.
    s32                 Status;
    net_config_list     ConfigList;
    s32                 LastUsefulError;
    s32                 Index = 0;
    s32                 i;
    xbool               FoundConfig;

    LastUsefulError = 0;
    FoundConfig = FALSE;
    m_pConfigList->DeleteAllItems();

    m_LastErrorSlot = 0;
    while( s_ConfigDeviceList[Index] )
    {
        const char* pCardCheckString;
        if( Index==0 )
        {
            pCardCheckString = "MC_CARD_CHECK_SLOT1";
        }
        else
        {
            pCardCheckString = "MC_CARD_CHECK_SLOT2";
        }
        irect r = irect(0,0,300,160);
        if( m_PopUp == NULL )
        {
            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
            m_PopUpResult = DLG_POPUP_IDLE;
        }

        m_PopUp->Configure( r,
            -1.0f,
            g_StringTableMgr( "ui", "IDS_NETWORK_POPUP"     ),
            g_StringTableMgr( "ui",  pCardCheckString ),
            xwstring( "" ) );

        Status = net_GetConfigList( s_ConfigDeviceList[Index], &ConfigList );
        // If we found any valid configurations, add it to the main config list.
        if( Status < 0 )
        {
            // Note: we may need to store additional info as to which device contained
            // the error so we can display it to the user. This is where that would be
            // done.
            if( (Status != NET_ERR_NO_CARD) && (LastUsefulError == 0 ) )
            {
                m_LastErrorSlot = Index;
                LastUsefulError = Status;
            }
        }
        else
        {
            if( (Status == 0) && (m_LastErrorSlot == -1) )
            {
                m_LastErrorSlot = Index;
            }
            for( i = 0; i < ConfigList.Count; i++ )
            {
                if( ConfigList.Available[i] )
                {
                    m_pConfigList->AddItem( xwstring(ConfigList.Name[i]), (Index<<16)|i );
                }
                else
                {
                    m_LastErrorSlot = Index;
                    LastUsefulError = NET_ERR_NO_CONFIGURATION;
                }
            }
        }

        Index++;
    }

    if( m_pConfigList->GetItemCount() > 0 )
    {
        m_pConfigList->SetSelection(0);
        return m_pConfigList->GetItemCount();
    }
    return LastUsefulError;
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
            LOG_MESSAGE( "dlg_online_connect::UpdateConnectInit", "Opening dialog" );
            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
            m_PopUpResult = DLG_POPUP_IDLE;
        }

        r.Set( 0, 0, 300, 160 );

        m_PopUp->Configure( r,
            0.5f,
            g_StringTableMgr( "ui", "IDS_NETWORK_POPUP"         ),
            g_StringTableMgr( "ui", "IDS_ONLINE_PLEASE_WAIT"    ),
            g_StringTableMgr( "ui", "IDS_ONLINE_CONNECT_INIT"   ) );

        m_PopUp->EnableBlackout( FALSE );

        SetConnectState( CONNECT_WAIT );
    }

}

//=========================================================================
void dlg_online_connect::UpdateActivateInit( void )
{
    interface_info Info;

    net_GetInterfaceInfo(-1,Info);
    if( Info.Address == 0 )
    {
        s32 Count;

        g_StateMgr.StartBackgroundRendering();
        xtimer t;
        t.Start();
        net_BeginConfig();
        net_ActivateConfig( FALSE );
        Count = PopulateConfigurationList();
        t.Stop();
        LOG_MESSAGE( "dlg_online_connect::OnUpdate", "Getting configuration list took %2.02fms", t.ReadMs() );
        g_StateMgr.StopBackgroundRendering();

        //
        // hack hack hack! Prepare for checking card status while running
        //
        s32 Index = 0;
        for( Index=0; Index<2; Index++ )
        {
            s_LastCardStatus[Index] = g_MemcardHardware.GetCardStatus( Index );
        }
        if( Count <= 0 )
        {
            switch( Count )
            {
            case 0:
            case NET_ERR_NO_CARD:
                Failed("IDS_ONLINE_CONFIG_NONE_PRESENT");
                break;
            case NET_ERR_NO_HARDWARE:
                Failed("IDS_ONLINE_CONFIG_NO_HARDWARE", 0, OK_ONLY );
                break;
            case NET_ERR_OTHER_SYSTEM:
                Failed("IDS_ONLINE_CONFIG_OTHER_SYSTEM", m_LastErrorSlot+1 );
                break;
            case NET_ERR_NO_CONFIGURATION:
                Failed("IDS_ONLINE_CONFIG_NONE_PRESENT");
                break;
            case NET_ERR_CARD_NOT_FORMATTED:
                Failed("IDS_ONLINE_CARD_NOT_FORMATTED", m_LastErrorSlot+1 );
                break;
            case NET_ERR_INVALID_CARD:
                Failed("IDS_ONLINE_INVALID_CARD", m_LastErrorSlot+1 );
                break;
            default:
                // We should not get any errors we don't know about dealing with.
                ASSERT( FALSE );
                break;
            }
        }
        else
        {
            //
            // If we have only one network configuration present, go ahead and activate it. We
            // only present the configuration options if needed.
            //
            if( Count == 1 )
            {
                ASSERT( m_pConfigList->GetItemCount() == 1 );
                s32 DeviceIndex = m_pConfigList->GetSelectedItemData()>>16;
                s32 ConfigIndex = m_pConfigList->GetSelectedItemData() & 0xffff;

                // Just use the first one.
                g_StateMgr.StartBackgroundRendering();
                xtimer t;

                t.Start();
                m_Status = net_SetConfiguration( s_ConfigDeviceList[DeviceIndex] , ConfigIndex );
                t.Stop();
                LOG_MESSAGE( "dlg_online_connect::OnUpdate", "SetConfiguration took %2.02fms", t.ReadMs() );
                g_StateMgr.StopBackgroundRendering();
                if( m_Status < 0 )
                {
                    switch( m_Status )
                    {
                    case NET_ERR_OTHER_SYSTEM:
                        Failed("IDS_ONLINE_CONFIG_OTHER_SYSTEM", DeviceIndex+1);
                        break;
                    case NET_ERR_NO_CONFIGURATION:
                    case NET_ERR_NO_CARD:
                        Failed("IDS_ONLINE_CONFIG_NONE_PRESENT");
                        break;
                    case NET_ERR_NO_HARDWARE:
                        Failed("IDS_ONLINE_CONFIG_NO_HARDWARE", 0, OK_ONLY );
                        break;
                    default:
                        // We should not get any errors we don't know about
                        ASSERT( FALSE );
                        Failed("IDS_ONLINE_CONFIG_NONE_PRESENT");
                        break;
                    }
                }
                else
                {
                    SetConnectState( ACTIVATE_LOOP );
                    m_Done = FALSE;
                    m_Error = 0;
                    m_Timeout = 0.0f;
                }
            }
            else
            {
                // Open dialog containing the selections to pick. It *should* already be
                // fully populated with the configurations available.
                irect Position = m_Position;
                Position.Inflate(30,0);
                InitScreenScaling( Position );

                SetConnectState( ACTIVATE_SELECT );
                m_Done = FALSE;
                m_Error = 0;
                // destroy pop-up
                ASSERT( g_UiMgr->GetTopmostDialog(m_UserID) == m_PopUp );
                g_UiMgr->EndDialog( m_UserID, TRUE );
                m_PopUp = NULL;
            }
        }
    }
    else
    {
        m_Done = TRUE;
        SetConnectState( CONNECT_IDLE );
    }
}


//=========================================================================
void dlg_online_connect::UpdateAuthUser( void )
{
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
            Failed("IDS_ONLINE_CONNECT_MATCHMAKER_FAILED");
            g_MatchMgr.SetState( MATCH_IDLE );
            return;
        }
        //
        // User pressed cancel. This test has to be done prior to the reconfigure as
        // the configure function will clear out the popup result to idle.
        //
        if( m_PopUpResult != DLG_POPUP_IDLE )
        {
            m_PopUp = NULL;
            Failed( "IDS_ONLINE_CONNECT_ABORTED", 0, OK_ONLY );
            return;
        }

        if( m_PopUp == NULL )
        {
            irect r = g_UiMgr->GetUserBounds( g_UiUserID );
            LOG_MESSAGE( "dlg_online_connect::UpdateAuthUser", "Opening dialog" );
            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
            m_PopUpResult = DLG_POPUP_IDLE;
        }

        irect r(0,0,300,160);

        xwstring MessageText  = g_StringTableMgr( "ui", "IDS_ONLINE_PLEASE_WAIT" );
        MessageText += "\n";
        MessageText += g_StringTableMgr( "ui", "IDS_ONLINE_CONNECT_MATCHMAKER" );
        MessageText += "\n";
        MessageText += g_StringTableMgr( "ui", "IDS_ONLINE_TIMEOUT" );
        MessageText += xwstring( xfs("%d",(30-(s32)m_Timeout)) );

        m_PopUp->Configure( r,
            g_StringTableMgr( "ui", "IDS_NETWORK_POPUP" ), 
            FALSE, 
            TRUE, 
            FALSE, 
            MessageText,
            g_StringTableMgr( "ui", "IDS_NAV_CANCEL" ),
            &m_PopUpResult );

        m_PopUp->EnableBlackout( FALSE );
    }
    else
    {
        switch( g_MatchMgr.GetAuthStatus() )
        {
        case AUTH_STAT_CONNECTED:
            g_MatchMgr.SetUserStatus( BUDDY_STATUS_ONLINE );
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
        case AUTH_STAT_CANNOT_CONNECT:
            Failed( "IDS_ONLINE_CONNECT_MATCHMAKER_FAILED" );
            break;
        case AUTH_STAT_DISCONNECTED:
            Failed( "IDS_ONLINE_NETWORK_DOWN" );
            break;
        case AUTH_STAT_SECURITY_FAILED:
            Failed( "IDS_ONLINE_SECURITY_FAILED" );
            break;
        default:
            ASSERT( FALSE );
            //Failed( "IDS_SIGN_IN_NO_CONNECTION" );
            Failed( "IDS_ONLINE_CONNECT_MATCHMAKER_FAILED" );
            break;
        }
    }
}

//=========================================================================
void dlg_online_connect::UpdateAuthMachine( void )
{
    //
    // This is the correct form of the return state of the matchmgr. At this point, more than one
    // status can be returned depending on why the matchmgr went idle. This will happen if there
    // is more than one user account, and the player needs to provide input, or needs to be presented
    // with other options (such as boot-to-dash etc).
    //
    // wait for logo to fade in
    if( m_bIsFading[LOGO_DNAS_OK] )
        return;

    if( g_MatchMgr.IsBusy() )
    {
        if( m_Timeout > 90.0f )
        {
            m_Done = FALSE;
            Failed("IDS_ONLINE_CONNECT_TIMEOUT");
            g_MatchMgr.SetState( MATCH_IDLE );
            return;
        }

        if( m_PopUpResult != DLG_POPUP_IDLE )
        {
            // Popup will have autoclosed itself.
            m_PopUp = NULL;
            Failed( "IDS_ONLINE_CONNECT_ABORTED", 0, OK_ONLY );
            return;
        }
        irect r( 0, 0, 300, 160 );

        if( m_PopUp==NULL )
        {
            LOG_MESSAGE( "dlg_online_connect::UpdateAuthMachine", "Opening dialog" );
            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
            m_Timeout = 0.0f;
            m_PopUpResult = DLG_POPUP_IDLE;
        }

        xwstring MessageText  = g_StringTableMgr( "ui", "IDS_ONLINE_PLEASE_WAIT" );
        MessageText += "\n";
        MessageText += g_StringTableMgr( "ui", "IDS_ONLINE_CONNECT_AUTHENTICATING" );

        m_PopUp->Configure( r,
            g_StringTableMgr( "ui", "IDS_NETWORK_POPUP" ), 
            FALSE, 
            FALSE, 
            FALSE, 
            MessageText,
            xwstring(""),
            &m_PopUpResult );
        m_PopUp->EnableBlackout( FALSE );
    }
    else
    {
        switch( g_MatchMgr.GetAuthStatus() )
        {
        case AUTH_STAT_CONNECTED:
            {
                if( CONFIG_IS_AUTOSERVER || CONFIG_IS_AUTOCLIENT )
                {
                    if( CONFIG_IS_AUTOSERVER  )
                    {
                        g_StateMgr.GetActiveSettings().Commit();
                    }

                    SetConnectState( CONNECT_DONE );
                    m_PopUp->Close();
                    m_PopUpResult = DLG_POPUP_YES;
                }
                else
                {
                    SetConnectState( CONNECT_DONE );
                    m_PopUp->Close();
                    m_PopUp=NULL;
                }
                // fade out DNAS logo
                m_bIsFading[LOGO_DNAS_OK]       = TRUE;
                m_bFadeIn[LOGO_DNAS_OK]         = FALSE;
                m_LogoAlpha[LOGO_DNAS_OK]       = 255.0f;
                m_FadeStep[LOGO_DNAS_OK]        = 256.0f / 0.8f;
            }
            break;
        case AUTH_STAT_SELECT_USER:
        case AUTH_STAT_NO_ACCOUNT:
            {
                // refresh the user account list
                RefreshUserList();
                m_pUserList->SetSelection( 0 );

                // destroy pop-up?
                ASSERT( g_UiMgr->GetTopmostDialog(m_UserID) == m_PopUp );
                g_UiMgr->EndDialog( m_UserID, TRUE );
                m_PopUp = NULL;
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
            Failed("IDS_ONLINE_CONNECT_MATCHMAKER_REFUSED");
            break;
        case AUTH_STAT_URGENT_MESSAGE:
            SetConnectState( CONNECT_REQUIRED_MESSAGE );
            break;
        case AUTH_STAT_CANNOT_CONNECT:            // DNAS failure on ps2, xbox may have other error problem.
            //
            {
#if 0
                s32 ErrorCode = g_MatchMgr.GetConnectErrorCode();
                if( (ErrorCode>-800) && (ErrorCode<=-700) )
                {
                    Failed( g_MatchMgr.GetConnectErrorMessage(), g_MatchMgr.GetConnectErrorCode(), CANCEL_MANAGE );
                }
                else
#endif
                {
                    Failed( g_MatchMgr.GetConnectErrorMessage(), g_MatchMgr.GetConnectErrorCode(), CANCEL_RETRY_MANAGE );
                }
            }
            break;
        case AUTH_STAT_DISCONNECTED:
            Failed( "IDS_ONLINE_NETWORK_DOWN" );
            break;
        default:
            ASSERT( FALSE );
            //Failed( "IDS_SIGN_IN_NO_CONNECTION" );
            Failed( "IDS_ONLINE_CONNECT_MATCHMAKER_FAILED" );
            break;
        }

        if( g_MatchMgr.GetAuthStatus() != AUTH_STAT_CONNECTED )
        {
            // fade in DNAS failed logo
            m_bIsFading[LOGO_DNAS_ERROR]    = TRUE;
            m_bFadeIn[LOGO_DNAS_ERROR]      = TRUE;
            m_bRenderLogo[LOGO_DNAS_ERROR]  = TRUE;
            m_LogoAlpha[LOGO_DNAS_ERROR]    = 0.0f;
            m_FadeStep[LOGO_DNAS_ERROR]     = 256.0f / 0.267f;
        }
    }
}