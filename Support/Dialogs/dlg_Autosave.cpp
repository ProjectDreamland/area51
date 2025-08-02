//=========================================================================
//
//  dlg_Autosave.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"

#include "dlg_Autosave.hpp"
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "Configuration/GameConfig.hpp"
#ifdef CONFIG_VIEWER
#include "../../Apps/ArtistViewer/Config.hpp"
#else
#include "../../Apps/GameApp/Config.hpp"	
#endif
#include "MemCardMgr/MemCardMgr.hpp"

//=========================================================================
//  Autosave Dialog
//=========================================================================

ui_manager::control_tem AutosaveControls[] = 
{
    { IDC_AUTOSAVE_NAV_TEXT,  "IDS_NULL",   "text", 0, 0, 0, 0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem AutosaveDialog =
{
    "IDS_AUTOSAVE_MENU",
    1, 9,
    sizeof(AutosaveControls)/sizeof(ui_manager::control_tem),
    &AutosaveControls[0],
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

void dlg_autosave_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "autosave", &AutosaveDialog, &dlg_autosave_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_autosave_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_autosave* pDialog = new dlg_autosave;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_autosave
//=========================================================================

dlg_autosave::dlg_autosave( void )
{
}

//=========================================================================

dlg_autosave::~dlg_autosave( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_autosave::Create( s32                        UserID,
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

    // intialize popup pointer
    m_PopUp = NULL;

    // initialize card id
    m_iCard = 0;

    // initialize screen scaling
    InitScreenScaling( Position );

    // set the frame to be disabled (if coming from off screen)
    if (g_UiMgr->IsScreenOn() == FALSE)
        SetFlag( WF_DISABLED, TRUE );

    g_UiMgr->DisableScreenHighlight();

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_autosave::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_autosave::Render( s32 ox, s32 oy )
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

void dlg_autosave::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;


    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            if (g_UiMgr->IsScreenOn() == FALSE)
            {
                // enable the frame
                SetFlag( WF_DISABLED, FALSE );
                g_UiMgr->SetScreenOn(TRUE);
            }

#ifdef TARGET_XBOX
            // Xbox intercepts this keypress so it can prompt the user
            // to go to the dashboard to free up space.
            MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition(0);
            // open confirmation dialog
            irect r = g_UiMgr->GetUserBounds( g_UiUserID );
            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

            // set nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_DONT_FREE_BLOCKS" ));
            
            xbool SecondOption = FALSE;
            if( GameMgr.GameInProgress() == FALSE )
            {
                navText += g_StringTableMgr( "ui", "IDS_NAV_FREE_MORE_BLOCKS" );
                SecondOption = TRUE;
            }

            // calculate blocks required
            m_BlocksRequired = ( (g_StateMgr.GetProfileSaveSize() - Condition.BytesFree) + 16383 ) / 16384;

            if( x_GetLocale() == XL_LANG_ENGLISH )
            {
                r.SetWidth(380);
                r.SetHeight(125);
            }
            else
            {
                r.SetWidth(400);
                r.SetHeight(145);
            }
            m_PopUp->Configure( r, g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ), 
                TRUE, 
                SecondOption, 
                FALSE, 
                xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NOT_ENOUGH_FREE_SPACE_SLOT1_XBOX" )), m_BlocksRequired ) ),
                navText,
                &m_PopUpResult );

            return;
#else
            // open a popup
            irect r = g_UiMgr->GetUserBounds( g_UiUserID );
            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

            // set nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_RETRY" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_CANCEL_AUTOSAVE" );

            // set message
            xwstring messageText;
            if( g_StateMgr.GetProfileNotSaved( 0 ) )
            {
                // could not save because we haven't selected a profile on a memory card/hdd                
                messageText = g_StringTableMgr( "ui", "IDS_AUTOSAVE_FAILED_SELECT_PROFILE" );
            }
            else
            {
                // failed due to a memory card error
                MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( m_iCard );

                if( Condition.bNoCard )
                {
                    if( m_iCard == 0 )
                        messageText = g_StringTableMgr( "ui", "IDS_AUTOSAVE_FAILED_NO_CARD_SLOT_1" );
                    else
                        messageText = g_StringTableMgr( "ui", "IDS_AUTOSAVE_FAILED_NO_CARD_SLOT_2" );
                }
                else
                {
                    if( m_iCard == 0 )
                        messageText = g_StringTableMgr( "ui", "IDS_AUTOSAVE_FAILED_CARD_CHANGED_SLOT_1" );
                    else
                        messageText = g_StringTableMgr( "ui", "IDS_AUTOSAVE_FAILED_CARD_CHANGED_SLOT_2" );                       
                }
            }

            if (x_GetLocale() == XL_LANG_ENGLISH)
            {
                r.Set( 0, 0, 270, 200 );
            }
            else
            {
                r.Set( 0, 0, 340, 220 );
            }

            // configure message
            m_PopUp->ConfigureAutosaveDialog( 
                r,
                g_StringTableMgr( "ui", "IDS_AUTOSAVE_FAILED_HEADER" ), 
                messageText,
                navText,
                &m_PopUpResult );
    
            m_PopUp->EnableBlackout( FALSE );

            return;
#endif
        }
    }

    // check popup dialog
    if ( m_PopUp )
    {
        if ( m_PopUpResult != DLG_POPUP_IDLE )
        {
#ifdef TARGET_XBOX
            if( m_PopUpResult == DLG_POPUP_YES )
            {
                // continue without saving
                g_AudioMgr.Play( "Select_Norm" );
                m_State = DIALOG_STATE_BACK;            
            }
            else
            {
                // If the player chose to go to the Dash, go to memory area
                LD_LAUNCH_DASHBOARD LaunchDash;
                LaunchDash.dwReason = XLD_LAUNCH_DASHBOARD_MEMORY;
                // This value will be returned to the title via XGetLaunchInfo
                // in the LD_FROM_DASHBOARD struct when the Dashboard reboots
                // into the title. If not required, set to zero.
                LaunchDash.dwContext = 0;
                // Specify the logical drive letter of the region where
                // data needs to be removed; either T or U.
                LaunchDash.dwParameter1 = DWORD( 'U' );
                // Specify the number of 16-KB blocks that are needed to save a profile (in total)
                LaunchDash.dwParameter2 = ( g_StateMgr.GetProfileSaveSize() + 16383 ) / 16384;
                // Launch the Xbox Dashboard
                XLaunchNewImage( NULL, (PLAUNCH_DATA)(&LaunchDash) );
            }
#else
            if ( m_PopUpResult == DLG_POPUP_YES )
            {
                // retry - goto select profile screen
                m_State = DIALOG_STATE_SELECT;
            }
            else
            {
                // continue without saving
                m_State = DIALOG_STATE_BACK;
            }
#endif

            // clear popup 
            m_PopUp = NULL;
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);
}

//=========================================================================
