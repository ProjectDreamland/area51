//=========================================================================
//
//  dlg_online_host.cpp
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

#include "dlg_OnlineHost.hpp"
#include "dlg_popup.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Parsing/textin.hpp"
#include "StateMgr/mapList.hpp"
#include "Configuration/GameConfig.hpp"

//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum controls
{
    IDC_ONLINE_HOST_SERVER_NAME,
    IDC_ONLINE_HOST_PASSWORD,
	IDC_ONLINE_HOST_GAME_TYPE,
    IDC_ONLINE_HOST_MUTATION,
    IDC_ONLINE_HOST_VOICE,
    IDC_ONLINE_HOST_PRIVATE,
    IDC_ONLINE_HOST_USER_SERVER,
    IDC_ONLINE_HOST_USER_PASSWORD,
    IDC_ONLINE_HOST_TYPE_SELECTOR,
    IDC_ONLINE_HOST_MUTATION_SELECTOR,
    IDC_ONLINE_HOST_VOICE_SELECTOR,
    IDC_ONLINE_HOST_PRIVATE_SELECTOR,
    IDC_ONLINE_HOST_CONTINUE,
    IDC_ONLINE_HOST_NAV_TEXT,
};

ui_manager::control_tem OnlineHostControls[] = 
{
    // Frames.
    { IDC_ONLINE_HOST_SERVER_NAME,          "IDS_HOST_SERVER_NAME",     "text",    40,  40, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_PASSWORD,             "IDS_HOST_PASSWORD",        "text",    40,  75, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_GAME_TYPE,            "IDS_HOST_GAME_TYPE",       "text",    40, 110, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_MUTATION,             "IDS_HOST_MUTATION",        "text",    40, 145, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_VOICE,                "IDS_HOST_VOICE_ENABLED",   "text",    40, 180, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_PRIVATE,              "IDS_HOST_PRIVATE_SERVER",  "text",    40, 215, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_ONLINE_HOST_USER_SERVER,          "IDS_HOST_SERVER_NAME",     "edit",   240,  50, 210, 40,  0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_USER_PASSWORD,        "IDS_HOST_PASSWORD",        "edit",   240,  85, 210, 40,  0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_TYPE_SELECTOR,        "IDS_NULL",                 "combo",  230, 119, 230, 40,  0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_MUTATION_SELECTOR,    "IDS_NULL",                 "combo",  230, 154, 230, 40,  0, 3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_VOICE_SELECTOR,       "IDS_NULL",                 "combo",  230, 189, 230, 40,  0, 4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_PRIVATE_SELECTOR,     "IDS_NULL",                 "combo",  230, 224, 230, 40,  0, 5, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_HOST_CONTINUE,             "IDS_HOST_CONTINUE",        "button",  40, 285, 220, 40,  0, 6, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_ONLINE_HOST_NAV_TEXT,             "IDS_NULL",                 "text",     0,   0,   0,  0,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem OnlineHostDialog =
{
    "IDS_ONLINE_HOST_TITLE",
    1, 9,
    sizeof(OnlineHostControls)/sizeof(ui_manager::control_tem),
    &OnlineHostControls[0],
    0
};

//=========================================================================
//  Defines
//=========================================================================

//=========================================================================
//  Structs
//=========================================================================

//=========================================================================
//  Registration function
//=========================================================================

void dlg_online_host_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "online host", &OnlineHostDialog, &dlg_online_host_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_online_host_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_online_host* pDialog = new dlg_online_host;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_online_host
//=========================================================================

dlg_online_host::dlg_online_host( void )
{
}

//=========================================================================

dlg_online_host::~dlg_online_host( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_online_host::Create( s32                        UserID,
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

    m_pServerNameText   = (ui_text*)FindChildByID( IDC_ONLINE_HOST_SERVER_NAME );
    m_pPasswordText     = (ui_text*)FindChildByID( IDC_ONLINE_HOST_PASSWORD    );
    m_pGameTypeText     = (ui_text*)FindChildByID( IDC_ONLINE_HOST_GAME_TYPE   );
    m_pMutationText     = (ui_text*)FindChildByID( IDC_ONLINE_HOST_MUTATION    );
    m_pVoiceText        = (ui_text*)FindChildByID( IDC_ONLINE_HOST_VOICE       );
    m_pPrivateText      = (ui_text*)FindChildByID( IDC_ONLINE_HOST_PRIVATE     );
    m_pNavText          = (ui_text*)FindChildByID( IDC_ONLINE_HOST_NAV_TEXT    );

    m_pUserServerEdit   = (ui_edit*)FindChildByID( IDC_ONLINE_HOST_USER_SERVER   );
    m_pUserPasswordEdit = (ui_edit*)FindChildByID( IDC_ONLINE_HOST_USER_PASSWORD );

    m_pGameTypeSelect   = (ui_combo*)FindChildByID( IDC_ONLINE_HOST_TYPE_SELECTOR     );
    m_pMutationSelect   = (ui_combo*)FindChildByID( IDC_ONLINE_HOST_MUTATION_SELECTOR );
    m_pVoiceSelect      = (ui_combo*)FindChildByID( IDC_ONLINE_HOST_VOICE_SELECTOR    );
    m_pPrivateSelect    = (ui_combo*)FindChildByID( IDC_ONLINE_HOST_PRIVATE_SELECTOR  );
    
    m_pContinueButton   = (ui_button*)FindChildByID( IDC_ONLINE_HOST_CONTINUE );
    
    // set some text flags
    m_pServerNameText   ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pPasswordText     ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pGameTypeText     ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pMutationText     ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pVoiceText        ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pPrivateText      ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
   
    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
    m_pNavText->SetLabel( navText );
    m_pNavText->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);    

    // set up edit boxes
    m_pUserServerEdit   ->SetBufferSize( NET_SERVER_NAME_LENGTH );
    m_pUserPasswordEdit ->SetBufferSize( NET_PASSWORD_LENGTH    );
    m_pUserPasswordEdit ->Configure( FALSE );

    // set up game type selector
    m_pGameTypeSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT | ui_combo::CB_NOTIFY_PARENT );
    m_pGameTypeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_GAMETYPE_DM"  ), GAME_DM  );
    m_pGameTypeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_GAMETYPE_TDM" ), GAME_TDM );
    m_pGameTypeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_GAMETYPE_CTF" ), GAME_CTF );
    m_pGameTypeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_GAMETYPE_CNH" ), GAME_CNH );
    m_pGameTypeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_GAMETYPE_INF" ), GAME_INF );
//  m_pGameTypeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_GAMETYPE_TAG" ), GAME_TAG );
    m_pGameTypeSelect->SetSelection( 0 );

    // set up mutation selector
    m_pMutationSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT );

    // set up voice enabled selector
    m_pVoiceSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT );
    m_pVoiceSelect->AddItem ( g_StringTableMgr( "ui", "IDS_ON"  ), 1 );
    m_pVoiceSelect->AddItem ( g_StringTableMgr( "ui", "IDS_OFF" ), 0 );

    // set up private server selector
    m_pPrivateSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT );
    m_pPrivateSelect->AddItem ( g_StringTableMgr( "ui", "IDS_YES" ), 1 );
    m_pPrivateSelect->AddItem ( g_StringTableMgr( "ui", "IDS_NO"  ), 0 );

    // switch off the buttons to start
    m_pServerNameText   ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pPasswordText     ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pGameTypeText     ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pMutationText     ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pVoiceText        ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pPrivateText      ->SetFlag( ui_win::WF_VISIBLE, FALSE );
                                                            
    m_pUserServerEdit   ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pUserPasswordEdit ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pGameTypeSelect   ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pVoiceSelect      ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pMutationSelect   ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pPrivateSelect    ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pContinueButton   ->SetFlag( ui_win::WF_VISIBLE, FALSE );

    // set continue button alignment
    m_pContinueButton   ->SetFlag( ui_win::WF_BUTTON_LEFT, TRUE );
    
    // set initial highlight/control
    m_CurrHL = 5;
    m_pContinueButton->SetFlag(ui_win::WF_SELECTED, TRUE);
    GotoControl( (ui_control*)m_pContinueButton );

    // get pending settings
    host_settings& Settings = g_StateMgr.GetPendingSettings().GetHostSettings();
    
    // set name and password
//  m_pUserServerEdit   ->SetLabel( g_PendingConfig.GetServerName() );
//  m_pUserPasswordEdit ->SetLabel( g_PendingConfig.GetPassword() );
    m_pUserServerEdit   ->SetLabelWidth( NET_SERVER_NAME_LENGTH );
    m_pUserPasswordEdit ->SetLabelWidth( NET_PASSWORD_LENGTH );
    m_pUserServerEdit   ->SetLabel( Settings.m_ServerName );
    m_pUserPasswordEdit ->SetLabel( Settings.m_Password   );

    // initialize game type
    s32 gameType = 0;
    s32 i;

    for( i=0; i< m_pGameTypeSelect->GetItemCount(); i++ )
    {
        if( Settings.m_GameTypeID == m_pGameTypeSelect->GetItemData(i) )
        {
            gameType = i;
            break;
        }
    }    
    m_pGameTypeSelect   ->SetSelection( gameType );
    
    // initialize mutation select (based on gametype)
    InitializeMutationModes();
    s32 sel = 0;
    for( i=0; i<m_pMutationSelect->GetItemCount(); i++ )
    {    
        if( Settings.m_MutationMode ==  m_pMutationSelect->GetItemData(i) )
        {
            sel = i;
            break;
        }
    }
    m_pMutationSelect->SetSelection( sel );

    // initialize voice enabled selection
    //if( g_PendingConfig.IsVoiceEnabled() )
    if( Settings.m_Flags & SERVER_VOICE_ENABLED )
        m_pVoiceSelect->SetSelection( 0 );
    else
        m_pVoiceSelect->SetSelection( 1 );

    // initialize private server selection
    if( Settings.m_Flags & SERVER_IS_PRIVATE )    
        m_pPrivateSelect->SetSelection( 0 );
    else
        m_pPrivateSelect->SetSelection( 1 );

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

void dlg_online_host::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_online_host::Render( s32 ox, s32 oy )
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

void dlg_online_host::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    //s32 OriginalSelection;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( pWin == (ui_win*)m_pGameTypeSelect )
        {
            switch( Code )
            {
                case ui_manager::NAV_LEFT:
                case ui_manager::NAV_RIGHT:
                    ResetMapCycle();                   
                    InitializeMutationModes();
                    break;
                default:
                    ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
                    break;
            }
        }
        else
        {
            ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
        }
    }
}

//=========================================================================

void dlg_online_host::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        g_AudioMgr.Play("Backup");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_online_host::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( pWin == (ui_win*)m_pGameTypeSelect )
        {
            // new game type selected - clear the map cycle
            ResetMapCycle();
            InitializeMutationModes();
        }
        else if( pWin == (ui_win*)m_pContinueButton )
        {
            // check if server name was entered.
            xwstring tempName(m_pUserServerEdit->GetLabel());
            if (x_wstrcmp(tempName, (const xwchar*)L"---") == 0)
            {
                // not entered!
                irect r = g_UiMgr->GetUserBounds( g_UiUserID );
                m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                    ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER );

                m_PopUp->Configure( 3.0f,
                    g_StringTableMgr( "ui", "IDS_NETWORK_POPUP"         ),
                    g_StringTableMgr( "ui", "IDS_ENTER_SERVER_NAME"        ) );

                return;
            }

            // update pending game settings
            host_settings& Settings = g_StateMgr.GetPendingSettings().GetHostSettings();

            // set the server name
            g_PendingConfig.SetServerName( tempName );
            x_wstrcpy( Settings.m_ServerName, tempName );

            // set server password
            const xwstring& Password = m_pUserPasswordEdit->GetLabel();
            if( ( Password != g_StringTableMgr( "ui", "IDS_NONE" )) &&
                (x_wstrlen(Password) != 0) )
            {
                g_PendingConfig.SetPassword( xstring(m_pUserPasswordEdit->GetLabel()) );
                g_PendingConfig.SetPasswordEnabled( TRUE );
                x_mstrcpy( Settings.m_Password, Password );
            }
            else
            {
                g_PendingConfig.SetPasswordEnabled( FALSE );
                x_strcpy( Settings.m_Password, xstring("") );
            }

            // set game type
            g_PendingConfig.SetGameType( m_pGameTypeSelect->GetItemLabel(m_pGameTypeSelect->GetSelection()) ); 

            // set short game type
            switch( m_pGameTypeSelect->GetSelection() )
            {
            case 0:     g_PendingConfig.SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_DM"  ) );     break;
            case 1:     g_PendingConfig.SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_TDM" ) );     break;
            case 2:     g_PendingConfig.SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_CTF" ) );     break;
            case 3:     g_PendingConfig.SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_TAG" ) );     break;
            case 4:     g_PendingConfig.SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_INF" ) );     break;
            case 5:     g_PendingConfig.SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_CNH" ) );     break;
            default:    ASSERTS(FALSE, "Unknown gametype");
            }
            g_PendingConfig.SetGameTypeID( (game_type)m_pGameTypeSelect->GetSelectedItemData( 0 ) );
            Settings.m_GameTypeID = m_pGameTypeSelect->GetSelectedItemData();

            // store game options in config
            g_PendingConfig.SetVoiceEnabled( m_pVoiceSelect->GetSelectedItemData() );
            if(m_pVoiceSelect->GetSelectedItemData() == 1 )
                Settings.m_Flags |= SERVER_VOICE_ENABLED;
            else
                Settings.m_Flags &= ~SERVER_VOICE_ENABLED;

            g_PendingConfig.SetMutationMode( (mutation_mode)m_pMutationSelect->GetSelectedItemData() );
            Settings.m_MutationMode = (mutation_mode)m_pMutationSelect->GetSelectedItemData();

            g_PendingConfig.SetPrivateServer( (xbool)m_pPrivateSelect->GetSelectedItemData() );
            if( m_pPrivateSelect->GetSelectedItemData() == 1 )
                Settings.m_Flags |= SERVER_IS_PRIVATE;
            else
                Settings.m_Flags &= ~SERVER_IS_PRIVATE;


            //g_PendingConfig.SetPlayerCount( 1 );
            s32 PlayerCount = 1;

            // if there is a game running, get the count.
            if( GameMgr.GameInProgress() )
            {
                PlayerCount = g_ActiveConfig.GetPlayerCount();
            }

            g_PendingConfig.SetPlayerCount( PlayerCount );
            
            g_AudioMgr.Play("Select_Norm");
            m_State = DIALOG_STATE_SELECT;
        }
    }
}

//=========================================================================

void dlg_online_host::OnUpdate ( ui_win* pWin, f32 DeltaTime )
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
            m_pServerNameText   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pPasswordText     ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pGameTypeText     ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pMutationText     ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pVoiceText        ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pPrivateText      ->SetFlag( ui_win::WF_VISIBLE, TRUE );

            m_pUserServerEdit   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pUserPasswordEdit ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pGameTypeSelect   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pMutationSelect   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pVoiceSelect      ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pPrivateSelect    ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pContinueButton   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pNavText          ->SetFlag( ui_win::WF_VISIBLE, TRUE );
    
            GotoControl( (ui_control*)m_pContinueButton );
            m_pContinueButton->SetFlag(WF_HIGHLIGHT, TRUE);        
            g_UiMgr->SetScreenHighlight( m_pContinueButton->GetPosition() );
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    // update labels
    if( m_pUserServerEdit->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 0;
        m_pServerNameText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pServerNameText->GetPosition() );
    }
    else
        m_pServerNameText->SetLabelColor( xcolor(126,220,60,255) );

    if( m_pUserPasswordEdit->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 1;
        m_pPasswordText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pPasswordText->GetPosition() );
    }
    else
        m_pPasswordText->SetLabelColor( xcolor(126,220,60,255) );
    
    if( m_pGameTypeSelect->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 2;
        m_pGameTypeText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pGameTypeText->GetPosition() );
    }
    else
        m_pGameTypeText->SetLabelColor( xcolor(126,220,60,255) );
    
    if( m_pMutationSelect->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 3;
        m_pMutationText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pMutationText->GetPosition() );
    }
    else
        m_pMutationText->SetLabelColor( xcolor(126,220,60,255) );

    if( m_pVoiceSelect->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 4;
        m_pVoiceText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pVoiceText->GetPosition() );
    }
    else
        m_pVoiceText->SetLabelColor( xcolor(126,220,60,255) );

    if( m_pPrivateSelect->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 5;
        m_pPrivateText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pPrivateText->GetPosition() );
    }
    else
        m_pPrivateText->SetLabelColor( xcolor(126,220,60,255) );

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

void dlg_online_host::ResetMapCycle( void )
{
    // a new game type was selected so we must flush the map cycle
    host_settings& Settings = g_StateMgr.GetPendingSettings().GetHostSettings();
    map_settings* pMapSettings = &Settings.m_MapSettings;

    pMapSettings->m_bUseDefault     = TRUE;
    pMapSettings->m_MapCycleCount   = 0;
    pMapSettings->m_MapCycleIdx     = 0;

    for( s32 i=0; i<MAP_CYCLE_SIZE; i++ )
    {
        pMapSettings->m_MapCycle[i] = -1;
    }
}

//=========================================================================

void dlg_online_host::InitializeMutationModes( void )
{
    // update the available mutation modes based on the game type
    switch( m_pGameTypeSelect->GetSelectedItemData() )
    {
        case GAME_DM:
            m_pMutationSelect->DeleteAllItems();
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_CHANGE_AT_WILL"  ), MUTATE_CHANGE_AT_WILL  );
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_LOCK"      ), MUTATE_HUMAN_LOCK      );
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_LOCK"     ), MUTATE_MUTANT_LOCK     );
        //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_VS_MUTANT" ), MUTATE_HUMAN_VS_MUTANT );
        //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MAN_HUNT"        ), MUTATE_MAN_HUNT        );
        //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_HUNT"     ), MUTATE_MUTANT_HUNT     );
            break;

        case GAME_TDM:
            m_pMutationSelect->DeleteAllItems();
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_CHANGE_AT_WILL"  ), MUTATE_CHANGE_AT_WILL  );
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_LOCK"      ), MUTATE_HUMAN_LOCK      );
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_LOCK"     ), MUTATE_MUTANT_LOCK     );
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_VS_MUTANT" ), MUTATE_HUMAN_VS_MUTANT );
        //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MAN_HUNT"        ), MUTATE_MAN_HUNT        );
        //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_HUNT"     ), MUTATE_MUTANT_HUNT     );
            break;

        case GAME_CTF:
            m_pMutationSelect->DeleteAllItems();
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_CHANGE_AT_WILL"  ), MUTATE_CHANGE_AT_WILL  );
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_LOCK"      ), MUTATE_HUMAN_LOCK      );
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_LOCK"     ), MUTATE_MUTANT_LOCK     );
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_VS_MUTANT" ), MUTATE_HUMAN_VS_MUTANT );
        //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MAN_HUNT"        ), MUTATE_MAN_HUNT        );
        //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_HUNT"     ), MUTATE_MUTANT_HUNT     );
            break;

        case GAME_TAG:
            m_pMutationSelect->DeleteAllItems();
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_CHANGE_AT_WILL"  ), MUTATE_CHANGE_AT_WILL  );
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_LOCK"      ), MUTATE_HUMAN_LOCK      );
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_LOCK"     ), MUTATE_MUTANT_LOCK     );
        //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_VS_MUTANT" ), MUTATE_HUMAN_VS_MUTANT );
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MAN_HUNT"        ), MUTATE_MAN_HUNT        );
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_HUNT"     ), MUTATE_MUTANT_HUNT     );
            break;

        case GAME_INF:
            m_pMutationSelect->DeleteAllItems();
        //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_CHANGE_AT_WILL"  ), MUTATE_CHANGE_AT_WILL  );
        //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_LOCK"      ), MUTATE_HUMAN_LOCK      );
        //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_LOCK"     ), MUTATE_MUTANT_LOCK     );
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_VS_MUTANT" ), MUTATE_HUMAN_VS_MUTANT );
        //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MAN_HUNT"        ), MUTATE_MAN_HUNT        );
        //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_HUNT"     ), MUTATE_MUTANT_HUNT     );            
            break;

        case GAME_CNH:
            m_pMutationSelect->DeleteAllItems();
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_CHANGE_AT_WILL"  ), MUTATE_CHANGE_AT_WILL  );
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_LOCK"      ), MUTATE_HUMAN_LOCK      );
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_LOCK"     ), MUTATE_MUTANT_LOCK     );
            m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_VS_MUTANT" ), MUTATE_HUMAN_VS_MUTANT );
        //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MAN_HUNT"        ), MUTATE_MAN_HUNT        );
        //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_HUNT"     ), MUTATE_MUTANT_HUNT     );
            break;

        default:    
            ASSERTS( FALSE, "Unknown gametype" );
    }

    m_pMutationSelect->SetFlag( ui_win::WF_DISABLED, (m_pMutationSelect->GetItemCount() <= 1) );

    // set default selection
    m_pMutationSelect->SetSelection( 0 );
}

//=========================================================================
