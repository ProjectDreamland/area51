// 
// 
// 
// dlg_PressStart.cpp
// 
// 

//
// Includes
//

#include "entropy.hpp"

#include "ui\ui_manager.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_bitmap.hpp"

#include "dlg_PressStart.hpp"   
#include "dlg_PopUp.hpp"
#include "stringmgr\stringmgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "memcardmgr\memcardmgr.hpp"
#include "stateMgr/StateMgr.hpp"
#include "MemCardMgr/MemCardMgr.hpp"

#include "MoviePlayer/MoviePlayer.hpp"

#ifdef TARGET_PS2
#include "ps2\ps2_misc.hpp"
#endif

extern xstring SelectBestClip( const char* pName );


//=========================================================================
//  Press Start Dialog
//=========================================================================

enum
{
    POPUP_NO_SPACE,
    POPUP_BAD_SETTINGS,
};

enum controls
{
    IDC_A51_LOGO,
    IDC_START_BOX,
	IDC_PRESS_START,
};

ui_manager::control_tem Press_StartControls[] =
{
//  { IDC_A51_LOGO,    "IDS_NULL",             "bitmap",  40,  10, 400, 100, 0, 0, 0, 0, ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE  },
    { IDC_A51_LOGO,    "IDS_NULL",             "bitmap", 280,  10, 200,  50, 0, 0, 0, 0, ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE  },
    { IDC_START_BOX,   "IDS_NULL",             "bitmap",  90, 312, 300,  30, 0, 0, 0, 0, ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE  },
    { IDC_PRESS_START, "IDS_PRESS_START_TEXT", "text",     0, 307, 480,  30, 0, 0, 1, 1, ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE  },
};

ui_manager::dialog_tem Press_StartDialog =
{
    "IDS_PRESS_START_SCREEN",
    1, 9,
    sizeof(Press_StartControls)/sizeof(ui_manager::control_tem),
    &Press_StartControls[0],
    0
};

const f32 TIMEOUT_TIME = 60.0f;

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

void dlg_press_start_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "press start", &Press_StartDialog, &dlg_press_start_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_press_start_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_press_start* pDialog = new dlg_press_start;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_press_start
//=========================================================================

dlg_press_start::dlg_press_start( void )
{
}

//=========================================================================

dlg_press_start::~dlg_press_start( void )
{
    Destroy();
}


//=========================================================================

void dlg_press_start::DisableStartButton( void )
{
    m_State = DIALOG_STATE_TIMEOUT;
}

//=========================================================================

void dlg_press_start::EnableStartButton( void )
{
    m_WaitTime = 0.2f;
}

//=========================================================================
#ifdef TARGET_XBOX
xbool dlg_press_start::ValidateSettings( void )
{
    if( g_UIMemCardMgr.FoundSettings() )
    {
        if( g_StateMgr.GetSettingsCardSlot() == -1 )
        {
            // open damaged settings popup
            irect r = g_UiMgr->GetUserBounds( g_UiUserID );
            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
            m_PopUpType = POPUP_BAD_SETTINGS;

            // set nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NULL" )); //IDS_NAV_OK

            m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_NULL" ), 
                TRUE, 
                FALSE, 
                FALSE, 
                g_StringTableMgr( "ui", "IDS_DAMAGED_SETTINGS_MSG_XBOX" ),
                navText,
                &m_PopUpResult );

            m_pLogoBitmap       ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pFrameBitmap      ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pButtonPressStart ->SetFlag(ui_win::WF_VISIBLE, FALSE);

            m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;

            // bad settings data
            return FALSE;
        }
    }

    return TRUE;
}
#endif

s32 s_PressStartVoiceID = 0;

//=========================================================================

xbool dlg_press_start::Create( s32                        UserID,
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

    m_pLogoBitmap       = (ui_bitmap*) FindChildByID( IDC_A51_LOGO    );
    m_pFrameBitmap      = (ui_bitmap*) FindChildByID( IDC_START_BOX   );
    m_pButtonPressStart	= (ui_text*)   FindChildByID( IDC_PRESS_START );

    // initialize logo bitmap
    m_pLogoBitmap->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_BitmapID = g_UiMgr->LoadBitmap( "logo",  "UI_A51_Logo.xbmp" );
    m_pLogoBitmap->SetBitmap( m_BitmapID );

    // initialize start button frame
    //m_pFrameBitmap->SetFlag(ui_win::WF_VISIBLE, TRUE);
    m_pFrameBitmap->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pFrameBitmap->SetBitmap( g_UiMgr->FindElement( "button_combo1" ), TRUE, 0 );

    // initialize start button
    m_pButtonPressStart ->SetFlag(ui_win::WF_VISIBLE, TRUE);
    m_pButtonPressStart ->SetLabelColor( xcolor(230, 230, 230, 255) );
	GotoControl( (ui_control*)m_pButtonPressStart );

    m_PressStartState   = 0;
    m_DemoHoldTimer     = 0.0f;
    m_DemoFadeAlpha     = 255.0f;
    m_FadeControl       = -1.0f;

    m_FadeStartInAlpha  = 0;
    m_FadeStartIn       = TRUE;
    m_FadeAdjust        = 4;
    m_WaitTime          = 0;
    m_Timeout           = TIMEOUT_TIME;
    m_bPlayDemo         = FALSE;
    m_PopUp             = NULL;
    m_BlocksRequired    = 0;

    // start up the start movie
    g_StateMgr.PlayMovie( "StartScreen", TRUE, TRUE );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    s_PressStartVoiceID = g_AudioMgr.Play( "MUSIC_StartScreen" );

	// Return success code
    return Success;
}

//=========================================================================

void dlg_press_start::Destroy( void )
{
    ui_dialog::Destroy();

    g_AudioMgr.Release( s_PressStartVoiceID, 0.0f );

#ifdef TARGET_PS2
    // wait until we finish drawing before we unload the logo bitmap
    DLIST.Flush();
    DLIST.WaitForTasks();
#endif
    // unload logo bitmap
    g_UiMgr->UnloadBitmap( "logo" );
}

//=========================================================================

void dlg_press_start::Render( s32 ox, s32 oy )
{
    m_FadeStartInAlpha+=m_FadeAdjust;
    
    if( m_FadeStartInAlpha > 255 )
    {
        m_FadeStartInAlpha = 255;
        m_FadeAdjust = -m_FadeAdjust;
    }

    if( m_FadeStartInAlpha < 64 )
    {
        m_FadeStartInAlpha = 64;
        m_FadeAdjust = -m_FadeAdjust;
    }

    m_pButtonPressStart->SetLabelColor( xcolor(230, 230, 230, m_FadeStartInAlpha) );

    // finally render all the normal dialog stuff
	ui_dialog::Render( ox, oy );
}

//=========================================================================

void dlg_press_start::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        // check for movie
        if( m_bPlayDemo )
        {
            m_bPlayDemo = FALSE;
            m_Timeout = TIMEOUT_TIME;
            g_StateMgr.CloseMovie();

            // restart the start movie
            g_StateMgr.PlayMovie( "StartScreen", TRUE, TRUE );
            // set text
            m_pButtonPressStart->SetLabel( g_StringTableMgr( "ui", "IDS_PRESS_START_TEXT" ) );
            m_pLogoBitmap->SetFlag(ui_win::WF_VISIBLE, FALSE);
        }
#if defined( TARGET_PC ) || defined( TARGET_PS2 )
        else
        {
            // set state
            m_State             = DIALOG_STATE_SELECT;
            m_CurrentControl    = IDC_PRESS_START;

            g_StateMgr.CloseMovie();
            g_StateMgr.PlayMovie( "MenuBackground", TRUE, TRUE );
        }
#endif
     }
}

//=========================================================================
void dlg_press_start::OnPadHelp( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        // check for movie
        if( m_bPlayDemo )
        {
            m_bPlayDemo = FALSE;
            m_Timeout = TIMEOUT_TIME;
            g_StateMgr.CloseMovie();

            // restart the start movie
            g_StateMgr.PlayMovie( "StartScreen", TRUE, TRUE );
            // set text
            m_pButtonPressStart->SetLabel( g_StringTableMgr( "ui", "IDS_PRESS_START_TEXT" ) );
            m_pLogoBitmap->SetFlag(ui_win::WF_VISIBLE, FALSE);
        }
        else
        {
            // stop the movie
            g_StateMgr.CloseMovie();

#ifdef TARGET_XBOX
            // check HDD errors
            xbool bFoundProfile = g_UIMemCardMgr.FoundProfile();
            xbool bFoundSettings = g_UIMemCardMgr.FoundSettings();
            if( !( bFoundProfile && bFoundSettings ) )
            {
                MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( 0 );

                if( Condition.bIsFull || Condition.bInsufficientSpace )
                {
                    xwstring MessageText;
                    s32 BlocksToFree;

                    if( bFoundSettings )
                    {
                        m_BlocksRequired = ( g_StateMgr.GetProfileSaveSize() + 16383 ) / 16384;
                        BlocksToFree     = ( (g_StateMgr.GetProfileSaveSize() - Condition.BytesFree) + 16383 ) / 16384;
                        MessageText = xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NOT_ENOUGH_FREE_SPACE_SLOT1_XBOX" )), BlocksToFree ) );
                    }
                    else if( bFoundProfile )
                    {
                        m_BlocksRequired = ( g_StateMgr.GetSettingsSaveSize() + 16383 ) / 16384;
                        BlocksToFree     = ( (g_StateMgr.GetSettingsSaveSize() - Condition.BytesFree) + 16383 ) / 16384;
                        MessageText = xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NOT_ENOUGH_FREE_SPACE_SLOT1_SETTINGS_XBOX" )), BlocksToFree ) );
                    }
                    else
                    {
                        m_BlocksRequired = ( g_StateMgr.GetSettingsSaveSize() + g_StateMgr.GetProfileSaveSize() + 16383 ) / 16384;
                        BlocksToFree     = ( ( g_StateMgr.GetSettingsSaveSize() + g_StateMgr.GetProfileSaveSize() - Condition.BytesFree) + 16383 ) / 16384;
                        MessageText = xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NOT_ENOUGH_FREE_SPACE_SLOT1_ALL_XBOX" )), BlocksToFree ) );
                    }

                    xwstring NavText = g_StringTableMgr( "ui", "IDS_NAV_DONT_FREE_BLOCKS" );
                    NavText += g_StringTableMgr( "ui", "IDS_NAV_FREE_MORE_BLOCKS" );

                    irect r = g_UiMgr->GetUserBounds( g_UiUserID );
                    m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
                    m_PopUpType = POPUP_NO_SPACE;

                    irect Size( 0, 0, 400, 240 );
                    if( x_GetLocale() == XL_LANG_ENGLISH )
                    {
                        Size.SetWidth(400);
                        Size.SetHeight(240);
                    }
                    else
                    {
                        Size.SetWidth(400);
                        Size.SetHeight(145);
                    }                    

                    m_PopUp->Configure( 
                        Size,
                        g_StringTableMgr( "ui", "IDS_MEMCARD_HEADER" ), 
                        TRUE, 
                        TRUE, 
                        FALSE, 
                        MessageText,
                        NavText,
                        &m_PopUpResult );

                    m_pLogoBitmap       ->SetFlag(ui_win::WF_VISIBLE, FALSE);
                    m_pFrameBitmap      ->SetFlag(ui_win::WF_VISIBLE, FALSE);
                    m_pButtonPressStart ->SetFlag(ui_win::WF_VISIBLE, FALSE);

                    m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
                    return;
                }
            }

            // check validity of settings file
            if( ValidateSettings() == FALSE )
                return;
#endif
            // set state
            m_State             = DIALOG_STATE_SELECT;
            m_CurrentControl    = IDC_PRESS_START;

            g_StateMgr.PlayMovie( "MenuBackGround", TRUE, TRUE );
        }
    }
}

//=========================================================================

void dlg_press_start::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

#ifdef TARGET_XBOX
    if( m_PopUp )
    {
        if( m_PopUpType == POPUP_NO_SPACE )
        {
            switch( m_PopUpResult )
            {
            case DLG_POPUP_IDLE:
                // wait for response
                return;

            case DLG_POPUP_YES:
                // continue without saving

                // check validity of settings file
                if( ValidateSettings() == FALSE )
                    return;

                // settings valid, continue on.
                m_State             = DIALOG_STATE_SELECT;
                m_CurrentControl    = IDC_PRESS_START;
                g_StateMgr.PlayMovie( "MenuBackGround", TRUE, TRUE );
                return;

            case DLG_POPUP_NO:
                // free blocks
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
                // Specify the number of 16-KB blocks that are needed (in total)
                LaunchDash.dwParameter2 = m_BlocksRequired;
                // Launch the Xbox Dashboard
                XLaunchNewImage( NULL, (PLAUNCH_DATA)(&LaunchDash) );
                break;
            }
        }
        else
        {
            if( m_PopUpResult != DLG_POPUP_IDLE )
            {
                // settings valid, continue on.
                m_State             = DIALOG_STATE_SELECT;
                m_CurrentControl    = IDC_PRESS_START;
                g_StateMgr.PlayMovie( "MenuBackGround", TRUE, TRUE );
                return;
            }
        }
    }
#endif

#if !defined( TARGET_PC )
    if( m_bPlayDemo )
    {
        if( !Movie.IsPlaying() )
        {
            m_bPlayDemo = FALSE;
            m_Timeout = TIMEOUT_TIME;
            g_StateMgr.CloseMovie();
            g_StateMgr.PlayMovie( "StartScreen", TRUE, TRUE );
            m_pButtonPressStart->SetLabel( g_StringTableMgr( "ui", "IDS_PRESS_START_TEXT" ) );
            m_pLogoBitmap->SetFlag(ui_win::WF_VISIBLE, FALSE);
        }
    }
    else
    {
        m_Timeout -= DeltaTime;

        if( m_Timeout < 0 )
        {
            m_bPlayDemo = TRUE;
            m_pButtonPressStart->SetLabel( g_StringTableMgr( "ui", "IDS_DEMO_MODE" ) );
            m_pLogoBitmap->SetFlag(ui_win::WF_VISIBLE, TRUE);
            g_StateMgr.CloseMovie();
#ifdef TARGET_XBOX
            g_StateMgr.PlayMovie( "attract", TRUE, TRUE );
#else
            g_StateMgr.PlayMovie( "attract", FALSE, FALSE );
#endif
        }
    }
#endif
}

//=========================================================================
