//=========================================================================
//
//  dlg_main_menu.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"

#include "dlg_MainMenu.hpp"
#include "StateMgr\StateMgr.hpp"
#include "StringMgr\StringMgr.hpp"
#include "Configuration/GameConfig.hpp"

#ifdef CONFIG_VIEWER
#include "../../Apps/ArtistViewer/Config.hpp"
#else
#include "../../Apps/GameApp/Config.hpp"	
#endif

//=========================================================================
//  Main Menu Dialog
//=========================================================================

ui_manager::control_tem MainMenuControls[] = 
{
    { IDC_MAIN_MENU_CAMPAIGN,           "IDS_MAIN_MENU_CAMPAIGN",   "button",   60,  60, 120, 40, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MAIN_MENU_MULTI,              "IDS_MAIN_MENU_MULTI",      "button",   60, 100, 120, 40, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MAIN_MENU_ONLINE,             "IDS_MAIN_MENU_ONLINE",     "button",   60, 140, 120, 40, 0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MAIN_MENU_SETTINGS,           "IDS_MAIN_MENU_SETTINGS",   "button",   60, 180, 120, 40, 0, 3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MAIN_MENU_PROFILES,           "IDS_MAIN_MENU_PROFILES",   "button",   60, 220, 120, 40, 0, 4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MAIN_MENU_CREDITS,            "IDS_EXTRAS_ITEM_CREDITS",  "button",   60, 260, 120, 40, 0, 5, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MAIN_MENU_NAV_TEXT,           "IDS_NULL",                 "text",      0,   0,   0,  0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#ifdef TARGET_XBOX
    { IDC_MAIN_MENU_SILENT_LOGIN_TEXT,  "IDS_NULL",                 "text",    120, 320,   0,  0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#endif
}; 

ui_manager::dialog_tem MainMenuDialog =
{
    "IDS_MAIN_MENU",
    1, 9,
    sizeof(MainMenuControls)/sizeof(ui_manager::control_tem),
    &MainMenuControls[0],
    0
};

#ifdef TARGET_XBOX
static xbool s_PendingInviteRunOnce = FALSE;
#endif

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

void dlg_main_menu_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "main menu", &MainMenuDialog, &dlg_main_menu_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_main_menu_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_main_menu* pDialog = new dlg_main_menu;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_main_menu
//=========================================================================

dlg_main_menu::dlg_main_menu( void )
{
#ifdef TARGET_XBOX
    m_XBOXNotificationOffsetX = 28;    
    m_XBOXNotificationOffsetY = 36;    
#endif
}

//=========================================================================

dlg_main_menu::~dlg_main_menu( void )
{
}

//=========================================================================

xbool dlg_main_menu::Create( s32                        UserID,
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

    m_pButtonCampaign       = (ui_button*)  FindChildByID( IDC_MAIN_MENU_CAMPAIGN           );
    m_pButtonMultiPlayer    = (ui_button*)  FindChildByID( IDC_MAIN_MENU_MULTI              );
    m_pButtonOnline         = (ui_button*)  FindChildByID( IDC_MAIN_MENU_ONLINE             );
    m_pButtonSettings       = (ui_button*)  FindChildByID( IDC_MAIN_MENU_SETTINGS           );
    m_pButtonProfiles       = (ui_button*)  FindChildByID( IDC_MAIN_MENU_PROFILES           );
    m_pButtonCredits        = (ui_button*)  FindChildByID( IDC_MAIN_MENU_CREDITS            );
    m_pNavText              = (ui_text*)    FindChildByID( IDC_MAIN_MENU_NAV_TEXT           );
#ifdef TARGET_XBOX
    m_pSilentLoginText      = (ui_text*)    FindChildByID( IDC_MAIN_MENU_SILENT_LOGIN_TEXT  );
#endif

    s32 iControl = g_StateMgr.GetCurrentControl();
    if( (iControl == -1) || (GotoControl(iControl)==NULL) )
    {
        GotoControl( (ui_control*)m_pButtonCampaign );
        m_CurrentControl =  IDC_MAIN_MENU_CAMPAIGN;
    }
    else
    {
        GotoControl( iControl );
        m_CurrentControl = iControl;
    }

    m_CurrHL = 0;
    m_bCheckKeySequence = FALSE;

    // switch off the buttons to start
    m_pButtonCampaign     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonMultiPlayer  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonOnline       ->SetFlag(ui_win::WF_VISIBLE, FALSE);    
    m_pButtonSettings     ->SetFlag(ui_win::WF_VISIBLE, FALSE);    
    m_pButtonProfiles     ->SetFlag(ui_win::WF_VISIBLE, FALSE);    
    m_pButtonCredits      ->SetFlag(ui_win::WF_VISIBLE, FALSE);    
    m_pNavText            ->SetFlag(ui_win::WF_VISIBLE, FALSE);
#ifdef TARGET_XBOX
    m_pSilentLoginText    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
#endif

#ifdef LAN_PARTY_BUILD
    m_pButtonMultiPlayer ->SetFlag(ui_win::WF_DISABLED, TRUE);
#endif

    // set up nav text 
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));

    m_pNavText->SetLabel( xwstring(navText) );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

#ifdef TARGET_XBOX
    // set up silent signin text

    xwstring silentsigninText(g_StringTableMgr( "ui", "IDS_MAIN_MENU_SIGNED_IN_XBOX" ));
    m_pSilentLoginText->SetLabel( xwstring(silentsigninText) );
    m_pSilentLoginText->SetLabelFlags( ui_font::h_center|ui_font::v_top );
    m_pSilentLoginText->UseSmallText(TRUE);
#endif
    // set the number of players to 0
    g_PendingConfig.SetPlayerCount( 0 );

    // initialize the screen scaling
    InitScreenScaling( Position );

    // set the frame to be disabled (if coming from off screen)
    if (g_UiMgr->IsScreenOn() == FALSE)
        SetFlag( WF_DISABLED, TRUE );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    m_PopUp = NULL;

    // nuke any temporary profiles 
    //for( s32 i=0; i<SM_PROFILE_COUNT; i++ )
    //{
    //    g_StateMgr.ClearSelectedProfile( i );
    //}

    // Return success code
    return Success;
}

//=========================================================================

void dlg_main_menu::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_main_menu::Render( s32 ox, s32 oy )
{
    static s32 offset   =  0;
    static s32 gap      =  9;
    static s32 width    =  4;

    irect rb;
    
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

void dlg_main_menu::OnPadSelect( ui_win* pWin )
{
    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( pWin == (ui_win*)m_pButtonCampaign )
        {
            g_AudioMgr.Play("Select_Norm");
            m_CurrentControl =  IDC_MAIN_MENU_CAMPAIGN;
            m_State = DIALOG_STATE_SELECT;
        }
        else if( pWin == (ui_win*)m_pButtonMultiPlayer )
        {
            g_AudioMgr.Play("Select_Norm");
            m_CurrentControl = IDC_MAIN_MENU_MULTI;
            m_State = DIALOG_STATE_SELECT;
        }
        else if( pWin == (ui_win*)m_pButtonOnline )
        {
            g_AudioMgr.Play("Select_Norm");
            m_CurrentControl = IDC_MAIN_MENU_ONLINE;
            m_State = DIALOG_STATE_SELECT;
        }
        else if( pWin == (ui_win*)m_pButtonSettings )
        {
            g_AudioMgr.Play("Select_Norm");
            m_CurrentControl = IDC_MAIN_MENU_SETTINGS;
            m_State = DIALOG_STATE_SELECT;
        }
        else if( pWin == (ui_win*)m_pButtonProfiles )
        {
            g_AudioMgr.Play("Select_Norm");
            m_CurrentControl = IDC_MAIN_MENU_PROFILES;
            m_State = DIALOG_STATE_SELECT;
        }
        else if( pWin == (ui_win*)m_pButtonCredits )
        {
            g_AudioMgr.Play("Select_Norm");
            m_CurrentControl = IDC_MAIN_MENU_CREDITS;
            m_State = DIALOG_STATE_SELECT;
        }
    }
}

//=========================================================================

void dlg_main_menu::OnUpdate ( ui_win* pWin, f32 DeltaTime )
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
            m_pButtonCampaign     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonMultiPlayer  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonOnline       ->SetFlag(ui_win::WF_VISIBLE, TRUE);    
            m_pButtonSettings     ->SetFlag(ui_win::WF_VISIBLE, TRUE);    
            m_pButtonProfiles     ->SetFlag(ui_win::WF_VISIBLE, TRUE);    
            m_pButtonCredits      ->SetFlag(ui_win::WF_VISIBLE, TRUE);    
            m_pNavText            ->SetFlag(ui_win::WF_VISIBLE, TRUE);
#ifdef TARGET_XBOX
            m_pSilentLoginText    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
#endif  
            s32 iControl = g_StateMgr.GetCurrentControl();
            if( (iControl == -1) || (GotoControl(iControl)==NULL) )
            {
                GotoControl( (ui_control*)m_pButtonCampaign );
                m_pButtonCampaign->SetFlag(WF_HIGHLIGHT, TRUE);        
                g_UiMgr->SetScreenHighlight( m_pButtonCampaign->GetPosition() );
                m_CurrentControl =  IDC_MAIN_MENU_CAMPAIGN;
            }
            else
            {
                ui_control* pControl = GotoControl( iControl );
                ASSERT( pControl );
                pControl->SetFlag(WF_HIGHLIGHT, TRUE);
                g_UiMgr->SetScreenHighlight(pControl->GetPosition() );
                m_CurrentControl = iControl;
            }

            if (g_UiMgr->IsScreenOn() == FALSE)
            {
                // enable the frame
                SetFlag( WF_DISABLED, FALSE );
                g_UiMgr->SetScreenOn(TRUE);
            }

            #ifdef TARGET_XBOX
            if( s_PendingInviteRunOnce == FALSE )
            {
                // Ensure we never get this dialog again
                s_PendingInviteRunOnce = TRUE;

                if( g_MatchMgr.GetPendingInviteAccepted() == TRUE )
                {
                    xwstring InvitingUser;
                    xwstring AcceptedUser;
                    
                    if( g_MatchMgr.GetInviteAcceptedUsers( InvitingUser, AcceptedUser ) == TRUE )
                    {
                        // pending accept popup
                        irect r = g_UiMgr->GetUserBounds( m_UserID );
                        m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog( m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

                        xwstring Format( g_StringTableMgr( "ui", "IDS_MAIN_MENU_PENDING_INVITE" ) );

                        s32 Index = Format.Find( xwstring( "%s" ) );
                        ASSERT( Index != -1 );

                        xwstring Message( Format.Left ( Index ) );
                        xwstring Right  ( Format.Right( Format.GetLength() - Index - 2 ) );

                        Message += InvitingUser;
                        Message += Right;

                        m_PopUp->Configure( AcceptedUser,
                                            TRUE, 
                                            FALSE, 
                                            FALSE, 
                                            Message,
                                            g_StringTableMgr( "ui", "IDS_NAV_OK" ),
                                            &m_PopUpResult );
                    }
                }
            }
            #endif
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    if( m_pButtonCampaign->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pButtonCampaign->GetPosition() );
        highLight = 0;
    }
    else if( m_pButtonMultiPlayer->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pButtonMultiPlayer->GetPosition() );
        highLight = 1;
    }
    else if( m_pButtonOnline->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pButtonOnline->GetPosition() );
        highLight = 2;
    }
    else if( m_pButtonSettings->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pButtonSettings->GetPosition() );
        highLight = 3;
    }
    else if( m_pButtonProfiles->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pButtonProfiles->GetPosition() );
        highLight = 4;
    }
    else if( m_pButtonCredits->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pButtonCredits->GetPosition() );
        highLight = 5;
    }
    if( highLight != m_CurrHL )
    {
        if( highLight != -1 )
            g_AudioMgr.Play("Cusor_Norm");

        m_CurrHL = highLight;
    }

#ifdef TARGET_XBOX
//klkl: Add silent login messages for Xbox
    xwstring silentsigninText;

    {
        switch( g_MatchMgr.GetAuthStatus() )
        {
        case AUTH_STAT_CONNECTED:
            {
                PXONLINE_USER user = XOnlineGetLogonUsers();
                if( !user )
                {
                    silentsigninText = g_StringTableMgr("ui", "IDS_MAIN_MENU_NOT_SIGNED_IN_XBOX");
                    break;
                }
                silentsigninText = g_StringTableMgr("ui", "IDS_MAIN_MENU_SIGNED_IN_XBOX");
                //add gamertag
                silentsigninText += "\n";
                silentsigninText += user->szGamertag;
            }
            break;
        case AUTH_STAT_NEED_PASSWORD:
            silentsigninText = g_StringTableMgr("ui", "IDS_MAIN_MENU_PASSCODE_NEEDED_XBOX");
            break;
        case AUTH_STAT_CANNOT_CONNECT:
        case AUTH_STAT_URGENT_MESSAGE:
            silentsigninText = g_StringTableMgr("ui", "IDS_MAIN_MENU_SIGNIN_FAILED_XBOX");
            break;
        case AUTH_STAT_CONNECTING:
            silentsigninText = g_StringTableMgr("ui", "IDS_MAIN_MENU_SIGNING_IN_XBOX");
            break;
        case AUTH_STAT_DISCONNECTED:
        case AUTH_STAT_NO_ACCOUNT:
        default:
            silentsigninText = g_StringTableMgr("ui", "IDS_MAIN_MENU_NOT_SIGNED_IN_XBOX");
            break;
        }
    }

    m_pSilentLoginText->SetLabel(silentsigninText);
#endif

#ifndef CONFIG_RETAIL

    // check for enabling autoclient/server
    if( !m_bCheckKeySequence )
    {
    #if defined(TARGET_PS2) || defined(TARGET_PC)
        if( input_IsPressed( INPUT_PS2_BTN_START,   0 ) &&
            input_IsPressed( INPUT_PS2_BTN_SELECT,  0 ) )
    #elif defined TARGET_XBOX
        if( input_IsPressed( INPUT_XBOX_BTN_START,  0 ) &&
            input_IsPressed( INPUT_XBOX_BTN_B,      0 ) )
    #else
        ASSERT(0);
    #endif
        {
            // enable escape sequence checking
            m_bCheckKeySequence = TRUE;
        }
    }
    else
    {
    #if defined(TARGET_PS2) || defined(TARGET_PC)
        if( input_WasPressed( INPUT_PS2_BTN_L_UP,  0 ) )
    #elif defined TARGET_XBOX
        if( input_WasPressed( INPUT_XBOX_BTN_UP, 0 ) )
    #else
        ASSERT(0);
    #endif
        {
            g_Config.AutoServer = TRUE;
        }
    #if defined(TARGET_PS2) || defined(TARGET_PC)
        else if( input_WasPressed( INPUT_PS2_BTN_L_DOWN, 0 ) )
    #elif defined TARGET_XBOX
        else if( input_WasPressed( INPUT_XBOX_BTN_DOWN, 0 ) )
    #else
        ASSERT(0);
    #endif
        {
            g_Config.AutoClient = TRUE;
        }
    }
#endif
}

//=========================================================================
