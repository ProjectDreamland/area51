//=========================================================================
//
//  dlg_quick_match.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_bitmap.hpp"

#include "..\dlg_PopUp.hpp"
#include "dlg_QuickMatch.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "Parsing/textin.hpp"
#include "../../Apps/GameApp/Config.hpp"

//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum controls
{
    IDC_QUICK_MATCH_DM,
    IDC_QUICK_MATCH_TDM,
    IDC_QUICK_MATCH_CTF,
    IDC_QUICK_MATCH_TAG,
    IDC_QUICK_MATCH_INF,
    IDC_QUICK_MATCH_CNH,
    IDC_QUICK_MATCH_STATUS_BOX,
    IDC_QUICK_MATCH_STATUS_TEXT,
    IDC_QUICK_MATCH_NAV_TEXT,
};

ui_manager::control_tem QuickMatchControls[] = 
{
    // Frames.
    { IDC_QUICK_MATCH_DM,           "IDS_GAMETYPE_DM",  "button",   60,  60, 190, 40, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_QUICK_MATCH_TDM,          "IDS_GAMETYPE_TDM", "button",   60, 100, 190, 40, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_QUICK_MATCH_CTF,          "IDS_GAMETYPE_CTF", "button",   60, 140, 190, 40, 0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_QUICK_MATCH_CNH,          "IDS_GAMETYPE_CNH", "button",   60, 180, 190, 40, 0, 3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_QUICK_MATCH_INF,          "IDS_GAMETYPE_INF", "button",   60, 220, 190, 40, 0, 4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
//  { IDC_QUICK_MATCH_TAG,          "IDS_GAMETYPE_TAG", "button",   60, 260, 150, 40, 0, 5, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_QUICK_MATCH_STATUS_BOX,   "IDS_NULL",         "bitmap",   35, 150, 240, 30, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_QUICK_MATCH_STATUS_TEXT,  "IDS_NULL",         "text",     65, 155, 200, 22, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

//  { IDC_QUICK_MATCH_STATUS_BOX,   "IDS_NULL",         "bitmap",  176, 215, 200, 30, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
//  { IDC_QUICK_MATCH_STATUS_TEXT,  "IDS_NULL",         "text",    241, 220,  90, 22, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    
    { IDC_QUICK_MATCH_NAV_TEXT,     "IDS_NULL",         "text",      0,   0,   0,  0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};

ui_manager::dialog_tem QuickMatchDialog =
{
    "IDS_QUICKMATCH_MAIN",
    1, 9,
    sizeof(QuickMatchControls)/sizeof(ui_manager::control_tem),
    &QuickMatchControls[0],
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

static s32  iController = 0;

//=========================================================================
//  Registration function
//=========================================================================

void dlg_quick_match_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "quick match", &QuickMatchDialog, &dlg_quick_match_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_quick_match_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_quick_match* pDialog = new dlg_quick_match;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_level_select
//=========================================================================

dlg_quick_match::dlg_quick_match( void )
{
}

//=========================================================================

dlg_quick_match::~dlg_quick_match( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_quick_match::Create( s32                        UserID,
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
    m_pGameTypeDM   = (ui_button*)  FindChildByID( IDC_QUICK_MATCH_DM  );
    m_pGameTypeTDM  = (ui_button*)  FindChildByID( IDC_QUICK_MATCH_TDM );
    m_pGameTypeCTF  = (ui_button*)  FindChildByID( IDC_QUICK_MATCH_CTF );
//  m_pGameTypeTAG  = (ui_button*)  FindChildByID( IDC_QUICK_MATCH_TAG );
    m_pGameTypeINF  = (ui_button*)  FindChildByID( IDC_QUICK_MATCH_INF );
    m_pGameTypeCNH  = (ui_button*)  FindChildByID( IDC_QUICK_MATCH_CNH );
    m_pStatusBox    = (ui_bitmap*)  FindChildByID( IDC_QUICK_MATCH_STATUS_BOX );
    m_pStatusText   = (ui_text*)    FindChildByID( IDC_QUICK_MATCH_STATUS_TEXT );
    m_pNavText      = (ui_text*)    FindChildByID( IDC_QUICK_MATCH_NAV_TEXT );

    // hide them
    m_pGameTypeDM   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pGameTypeTDM  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pGameTypeCTF  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
//  m_pGameTypeTAG  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pGameTypeINF  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pGameTypeCNH  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pNavText      ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // set up status box
    m_pStatusBox->SetBitmap( g_UiMgr->FindElement( "button_combo1" ), TRUE, 0 );
    m_pStatusBox->SetFlag( ui_win::WF_VISIBLE, FALSE );

    // set up status text
    m_pStatusText->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pStatusText->SetLabelFlags( ui_font::h_left|ui_font::v_top );
    m_pStatusText->UseSmallText(TRUE);    

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
   
    m_pNavText->SetLabel( navText );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // set initial focus
    s32 iControl = g_StateMgr.GetCurrentControl();
    if( (iControl == -1) || (GotoControl(iControl)==NULL) )
    {
        GotoControl( (ui_control*)m_pGameTypeDM );
        m_CurrentControl =  IDC_QUICK_MATCH_DM;
    }
    else
    {
        GotoControl( iControl );
        m_CurrentControl = iControl;
    }
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

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // search flag
    m_SearchStarted = FALSE;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_quick_match::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_quick_match::Render( s32 ox, s32 oy )
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

    // render the glow bar
    g_UiMgr->RenderGlowBar();
}

//=========================================================================

void dlg_quick_match::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        // Check to see if a search is already going
        if( m_SearchStarted )
            return;

        // figure out which game type to search for
        if( pWin == (ui_win*)m_pGameTypeDM )
        {
            g_AudioMgr.Play("Select_Norm");
            m_CurrentControl =  IDC_QUICK_MATCH_DM;
            m_RequestedType = GAME_DM;
        }
        else if( pWin == (ui_win*)m_pGameTypeTDM )
        {
            g_AudioMgr.Play("Select_Norm");
            m_CurrentControl = IDC_QUICK_MATCH_TDM;
            m_RequestedType = GAME_TDM;
        }
        else if( pWin == (ui_win*)m_pGameTypeCTF )
        {
            g_AudioMgr.Play("Select_Norm");
            m_CurrentControl = IDC_QUICK_MATCH_CTF;
            m_RequestedType = GAME_CTF;
        }
/*      else if( pWin == (ui_win*)m_pGameTypeTAG )
        {
            g_AudioMgr.Play("Select_Norm");
            m_CurrentControl = IDC_QUICK_MATCH_TAG;
            m_RequestedType = GAME_TAG;
        } */
        else if( pWin == (ui_win*)m_pGameTypeINF )
        {
            g_AudioMgr.Play("Select_Norm");
            m_CurrentControl = IDC_QUICK_MATCH_INF;
            m_RequestedType = GAME_INF;
        }
        else if( pWin == (ui_win*)m_pGameTypeCNH )
        {
            g_AudioMgr.Play("Select_Norm");
            m_CurrentControl = IDC_QUICK_MATCH_CNH;
            m_RequestedType = GAME_CNH;
        }

        // keep track of our controller
        iController = g_UiMgr->GetActiveController();
        g_MatchMgr.SetFilter( m_RequestedType, -1, -1, -1, 0, -1 );

        // Start the Quick match search
        g_MatchMgr.StartAcquisition( ACQUIRE_SERVERS );
        m_SearchStarted = TRUE;

        // show status box
        m_pStatusText->SetFlag( ui_win::WF_VISIBLE, TRUE );
        m_pStatusBox ->SetFlag( ui_win::WF_VISIBLE, TRUE );

        // hide buttons
        m_pGameTypeDM   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
        m_pGameTypeTDM  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
        m_pGameTypeCTF  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    //  m_pGameTypeTAG  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
        m_pGameTypeINF  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
        m_pGameTypeCNH  ->SetFlag(ui_win::WF_VISIBLE, FALSE);

        // turn off highlight
        g_UiMgr->DisableScreenHighlight();

        g_AudioMgr.Play("");
    }
}

//=========================================================================

void dlg_quick_match::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        g_AudioMgr.Play("Backup");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_quick_match::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    static s32  DotCount  = 0;
    s32         highLight = -1;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // turn on the controls
            m_pGameTypeDM   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pGameTypeTDM  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pGameTypeCTF  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
        //  m_pGameTypeTAG  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pGameTypeINF  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pGameTypeCNH  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText      ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            // set focus
            s32 iControl = g_StateMgr.GetCurrentControl();
            if( (iControl == -1) || (GotoControl(iControl)==NULL) )
            {
                GotoControl( (ui_control*)m_pGameTypeDM );
                m_pGameTypeDM->SetFlag(WF_HIGHLIGHT, TRUE);        
                g_UiMgr->SetScreenHighlight( m_pGameTypeDM->GetPosition() );
                m_CurrentControl =  IDC_QUICK_MATCH_DM;
            }
            else
            {
                ui_control* pControl = GotoControl( iControl );
                ASSERT( pControl );
                pControl->SetFlag(WF_HIGHLIGHT, TRUE);
                g_UiMgr->SetScreenHighlight(pControl->GetPosition() );
                m_CurrentControl = iControl;
            }
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    // update screen highlight
    if( m_pGameTypeDM->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pGameTypeDM->GetPosition() );
        highLight = 0;
    }
    else if( m_pGameTypeTDM->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pGameTypeTDM->GetPosition() );
        highLight = 1;
    }
    else if( m_pGameTypeCTF->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pGameTypeCTF->GetPosition() );
        highLight = 2;
    }
    else if( m_pGameTypeCNH->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pGameTypeCNH->GetPosition() );
        highLight = 3;
    }
    else if( m_pGameTypeINF->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pGameTypeINF->GetPosition() );
        highLight = 4;
    }
/*  else if( m_pGameTypeTAG->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pGameTypeTAG->GetPosition() );
        highLight = 3;
    } */

    if( highLight != m_CurrHL )
    {
        if( highLight != -1 )
            g_AudioMgr.Play("Cusor_Norm");

        m_CurrHL = highLight;
    }


    // check for no available games popup
    if( m_PopUp )
    {
        if( m_PopUpResult != DLG_POPUP_IDLE )
        {
            if( m_PopUpResult == DLG_POPUP_YES )
            {
                // set up the server details
                switch( m_RequestedType )
                {
                case GAME_DM:   
                    g_PendingConfig.SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_DM"  ) );
                    g_PendingConfig.SetGameType( g_StringTableMgr("ui","IDS_GAMETYPE_DM") ); 
                    break;
                case GAME_TDM:  
                    g_PendingConfig.SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_TDM" ) );     
                    g_PendingConfig.SetGameType( g_StringTableMgr("ui","IDS_GAMETYPE_TDM") ); 
                    break;
                case GAME_CTF:  
                    g_PendingConfig.SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_CTF" ) );    
                    g_PendingConfig.SetGameType( g_StringTableMgr("ui","IDS_GAMETYPE_CTF") ); 
                    break;
                case GAME_TAG:  
                    g_PendingConfig.SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_TAG" ) );     
                    g_PendingConfig.SetGameType( g_StringTableMgr("ui","IDS_GAMETYPE_TAG") ); 
                    break;
                case GAME_INF:  
                    g_PendingConfig.SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_INF" ) );     
                    g_PendingConfig.SetGameType( g_StringTableMgr("ui","IDS_GAMETYPE_INF") ); 
                    break;
                case GAME_CNH:  
                    g_PendingConfig.SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_CNH" ) );     
                    g_PendingConfig.SetGameType( g_StringTableMgr("ui","IDS_GAMETYPE_CNH") ); 
                    break;
                default:        
                    ASSERTS(FALSE, "Unknown gametype");
                    break;
                }
                g_PendingConfig.SetGameTypeID( m_RequestedType );

                // Go to create a game (host) screen using the selected game type
                m_State = DIALOG_STATE_ACTIVATE;
            }

            // reset connection status
            //g_MatchMgr.SetConnectStatus(MATCH_CONN_CONNECTED);
            // kill the popup
            m_PopUp = NULL;

            // set nav text
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);

            return;
        }
    }

    // check if waiting for a search
    if( !m_SearchStarted )
        return;

    // update status text
    if( m_pStatusText->GetFlags() & ui_win::WF_VISIBLE )
    {
        xwstring statusText = g_StringTableMgr( "ui", "IDS_QUICKMATCH_SEARCHING" );

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

    // check search status
    if( (g_MatchMgr.GetState()          == MATCH_IDLE) &&
        (g_MatchMgr.IsAcquireComplete() == TRUE) )
    {
        s32 ServerCount = g_MatchMgr.GetServerCount();
        if( ServerCount > 0 )
        {

            g_MatchMgr.LockLists();
            // did we find a game server?
            s32 Index = x_irand( 0, ServerCount-1 );
            s32 i;

            for( i=0; i<ServerCount; i++ )
            {
                const server_info* pServerInfo = g_MatchMgr.GetServerInfo( Index );
                if( pServerInfo && ( (pServerInfo->Flags & (SERVER_HAS_PASSWORD|SERVER_IS_PRIVATE) )==0 ) )
                {
                    // hide status box
                    DotCount = 0;
                    m_pStatusText->SetLabel( g_StringTableMgr( "ui", "IDS_NULL" ) );
                    m_pStatusText->SetFlag( ui_win::WF_VISIBLE, FALSE );
                    m_pStatusBox ->SetFlag( ui_win::WF_VISIBLE, FALSE );

                    // set our map to the server map
                    g_PendingConfig.GetConfig() = *pServerInfo;
                    // yes - go to the lobby screen
                    // tell the state mgr that we finished
                    m_State = DIALOG_STATE_SELECT;
                    g_MatchMgr.UnlockLists();

                    // set the active controller
                    g_StateMgr.SetActiveControllerID( iController );
                    g_StateMgr.SetControllerRequested( iController, TRUE );

                    return;
                }
                Index++;
                if( Index >= ServerCount )
                {
                    Index = 0;
                }

            }
            g_MatchMgr.UnlockLists();
        }
        
        if( g_MatchMgr.IsAcquireComplete() )
        {           
            // no servers of this gametype found!
            m_SearchStarted = FALSE;

            // show buttons
            m_pGameTypeDM   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pGameTypeTDM  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pGameTypeCTF  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
        //  m_pGameTypeTAG  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pGameTypeINF  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pGameTypeCNH  ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            // turn on highlight
            g_UiMgr->EnableScreenHighlight();

            // hide status box
            DotCount = 0;
            m_pStatusText->SetLabel( g_StringTableMgr( "ui", "IDS_NULL" ) );
            m_pStatusText->SetFlag( ui_win::WF_VISIBLE, FALSE );
            m_pStatusBox ->SetFlag( ui_win::WF_VISIBLE, FALSE );

            // popup none found message
            irect r = g_UiMgr->GetUserBounds( g_UiUserID );
            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

            xwstring Message( g_StringTableMgr( "ui", "IDS_QUICKMATCH_NO_GAMES" ) );
            Message += xwstring(" ");
            switch(m_RequestedType)
            {
            case GAME_DM:   Message += g_StringTableMgr("ui","IDS_GAMETYPE_DM");    break;
            case GAME_TDM:  Message += g_StringTableMgr("ui","IDS_GAMETYPE_TDM");   break;
            case GAME_CTF:  Message += g_StringTableMgr("ui","IDS_GAMETYPE_CTF");   break;
            case GAME_TAG:  Message += g_StringTableMgr("ui","IDS_GAMETYPE_TAG");   break;
            case GAME_INF:  Message += g_StringTableMgr("ui","IDS_GAMETYPE_INF");   break;
            case GAME_CNH:  Message += g_StringTableMgr("ui","IDS_GAMETYPE_CNH");   break;
            }
            Message += xwstring(" ");
            Message += g_StringTableMgr( "ui", "IDS_QUICKMATCH_NO_GAMES_2" );

            // set nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_CREATE_GAME" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

            m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_NETWORK_POPUP" ), TRUE, TRUE, FALSE, Message, navText, &m_PopUpResult );
        }
    }

}

//=========================================================================



