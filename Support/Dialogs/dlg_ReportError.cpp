//=========================================================================
//
//  dlg_ReportError.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_text.hpp"
#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_listbox.hpp"

#include "dlg_ReportError.hpp"
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\MatchMgr.hpp"
#include "AudioMgr\AudioMgr.hpp"

//=========================================================================
//  Main Menu Dialog
//=========================================================================
enum report_error_controls
{
    IDC_REPORT_ERROR_LISTBOX,
    IDC_REPORT_ERROR_NAV_TEXT,
};

ui_manager::control_tem ReportErrorControls[] = 
{
    { IDC_REPORT_ERROR_NAV_TEXT,  "IDS_NULL",             "text",      0,   0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem ReportErrorDialog =
{
    "IDS_NULL",
    1, 9,
    sizeof(ReportErrorControls)/sizeof(ui_manager::control_tem),
    &ReportErrorControls[0],
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

void dlg_report_error_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "report error", &ReportErrorDialog, &dlg_report_error_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_report_error_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_report_error* pDialog = new dlg_report_error;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_report_error
//=========================================================================

dlg_report_error::dlg_report_error( void )
{
}

//=========================================================================

dlg_report_error::~dlg_report_error( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_report_error::Create( s32                        UserID,
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
    m_pNavText      = (ui_text*)    FindChildByID( IDC_REPORT_ERROR_NAV_TEXT );

    // hide them
    m_pNavText   ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_OK" ));
   
    m_pNavText->SetLabel( navText );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // set initial focus
    m_PopUp = NULL;

    // disable highlight
    g_UiMgr->DisableScreenHighlight();
    m_Position = Position;

    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;


    // Return success code
    return Success;
}

//=========================================================================

void dlg_report_error::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_report_error::Render( s32 ox, s32 oy )
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

void dlg_report_error::OnPadSelect( ui_win* pWin )
{
    (void)pWin;
    g_NetworkMgr.SetOnline( FALSE );
    if( m_State == DIALOG_STATE_ACTIVE )
    {
        //g_AudioMgr.Play( "OptionBack" );
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_report_error::OnPadBack( ui_win* pWin )
{
    (void)pWin;

}

//=========================================================================

void dlg_report_error::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    MEMORY_OWNER("dlg_report_error::OnUpdate");
    (void)pWin;
    (void)DeltaTime;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {            

            irect r = g_UiMgr->GetUserBounds( g_UiUserID );

            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
                ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );

            r.Set( 0, 0, 300, 200 );

            const char* pLabel = NULL;
            m_CanTroubleshoot = FALSE;

            switch( g_ActiveConfig.GetExitReason() )
            {
            case GAME_EXIT_NETWORK_DOWN:
            {
                interface_info Info;
                net_GetInterfaceInfo( -1, Info );
                if( Info.IsAvailable )
                {
#ifdef TARGET_XBOX
                    pLabel = "IDS_ONLINE_CHECK_CABLE_XBOX";
#else
                    pLabel = "IDS_ONLINE_NETWORK_DOWN";
#endif
                }
                else
                {
                    pLabel = "IDS_ONLINE_CHECK_CABLE";
                }
                break;
            }
            case GAME_EXIT_DUPLICATE_LOGIN:     pLabel = "IDS_ONLINE_DUPLICATE_LOGIN";          break;
            case GAME_EXIT_PLAYER_KICKED:       pLabel = "IDS_ONLINE_KICKED";                   break;
            case GAME_EXIT_PLAYER_DROPPED:      pLabel = "IDS_ONLINE_DROPPED";                  break;
            case GAME_EXIT_SERVER_SHUTDOWN:     pLabel = "IDS_ONLINE_SERVER_SHUTDOWN";          break;
            case GAME_EXIT_SERVER_BUSY:         pLabel = "IDS_ONLINE_DROPPED";                  break;
            case GAME_EXIT_CONNECTION_LOST:     pLabel = "IDS_ONLINE_CONNECTION_LOST";          break;
            case GAME_EXIT_INVALID_MISSION:     pLabel = "IDS_ONLINE_INVALID_MISSION";          break;
            case GAME_EXIT_SERVER_FULL:         pLabel = "IDS_ONLINE_LOGIN_SERVER_FULL";        break;
            case GAME_EXIT_BAD_PASSWORD:        pLabel = "IDS_ONLINE_LOGIN_BAD_PASSWORD";       break;
            case GAME_EXIT_CANNOT_CONNECT:      pLabel = "IDS_ONLINE_LOGIN_CANNOT_CONNECT";     break;
            case GAME_EXIT_LOGIN_REFUSED:       pLabel = "IDS_ONLINE_LOGIN_REFUSED";            break;
            case GAME_EXIT_CLIENT_BANNED:       pLabel = "IDS_ONLINE_LOGIN_BANNED";             break;
            case GAME_EXIT_SESSION_ENDED:       pLabel = "IDS_ONLINE_SESSION_ENDED";            break;
            default:                            ASSERT( FALSE );                                break;
            }
#if defined(TARGET_XBOX)
            if( g_ActiveConfig.GetExitReason() == GAME_EXIT_NETWORK_DOWN )
            {
                m_CanTroubleshoot = TRUE;
            }
#endif
            if( m_CanTroubleshoot )
            {
                xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_NETWORK_TROUBLESHOOTER" ));
                navText += g_StringTableMgr( "ui", "IDS_NAV_CANCEL" );

                m_PopUp->Configure( r,
                    g_StringTableMgr( "ui", "IDS_NETWORK_POPUP" ), 
                    TRUE, 
                    TRUE, 
                    FALSE, 
                    g_StringTableMgr( "ui", pLabel ),
                    navText,
                    &m_PopUpResult );
            }
            else
            {
                m_PopUp->Configure( r,
                    g_StringTableMgr( "ui", "IDS_NETWORK_POPUP" ), 
                    TRUE, 
                    TRUE, 
                    FALSE, 
                    g_StringTableMgr( "ui", pLabel ),
#if defined(TARGET_XBOX)
                    g_StringTableMgr( "ui", "IDS_NAV_NETWORK_CONTINUE" ),
#else
                    g_StringTableMgr( "ui", "IDS_NAV_OK" ),
#endif
                    &m_PopUpResult );
            }
            g_UiMgr->SetScreenOn(TRUE);
        }
    }
    else
    {
        if( m_PopUpResult != DLG_POPUP_IDLE )
        {
            if( (m_PopUpResult == DLG_POPUP_YES) && m_CanTroubleshoot )
            {
                g_StateMgr.Reboot( REBOOT_MANAGE );
            }
            else
            {
                m_State = DIALOG_STATE_BACK;
                g_UiMgr->EndDialog( g_UiUserID, TRUE );
            }
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    // update everything else
    ui_dialog::OnUpdate( pWin, DeltaTime );

}

/*
//=========================================================================
// Will give option of managing or dropping back to main menu
void dlg_report_error::Failed( const char* pFailureReason, s32 ErrorCode )
{
    (void)ErrorCode;
#if defined(TARGET_PS2)
    net_ActivateConfig( FALSE );
#endif
    g_NetworkMgr.SetOnline( FALSE );
    x_strcpy( m_LabelText, pFailureReason);
    m_LastErrorCode = ErrorCode;
    m_ConnectState = CONNECT_FAILED;
}
*/
