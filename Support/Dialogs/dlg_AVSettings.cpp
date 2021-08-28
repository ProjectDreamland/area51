//=========================================================================
//
//  dlg_av_settings.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"

#include "dlg_AVSettings.hpp"
#include "stringmgr\stringmgr.hpp"
#include "StateMgr\StateMgr.hpp"
#include "MemCardMgr/MemCardMgr.hpp"

//=========================================================================
//  Main Options Dialog
//=========================================================================

enum
{
    POPUP_CONFIRM_SETTINGS,
    POPUP_OVERWRITE_SETTINGS
};

enum controls
{
    IDC_AV_VOLUME_SFX_TEXT,
    IDC_AV_VOLUME_MUSIC_TEXT,
    IDC_AV_VOLUME_SPEECH_TEXT,
#ifdef TARGET_PS2
    IDC_AV_TOGGLE_SPEAKER_TEXT,
#endif
#ifdef TARGET_XBOX
    IDC_AV_BRIGHTNESS_TEXT,
#endif

    IDC_AV_VOLUME_SFX,
    IDC_AV_VOLUME_MUSIC,
    IDC_AV_VOLUME_SPEECH,
#ifdef TARGET_PS2
    IDC_AV_TOGGLE_SPEAKER,
#endif
    IDC_AV_HEADSET_TEST,
#ifdef TARGET_XBOX
    IDC_AV_BRIGHTNESS,
#endif
    IDC_AV_BUTTON_ACCEPT,

    IDC_AV_NAV_TEXT,
};


ui_manager::control_tem AVSettingsControls[] = 
{
    // Frames.
    { IDC_AV_VOLUME_SFX_TEXT,       "IDS_OPTIONS_SFX_VOLUME",       "text",      40,  40, 220, 40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_AV_VOLUME_MUSIC_TEXT,     "IDS_OPTIONS_MUSIC_VOLUME",     "text",      40,  75, 220, 40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_AV_VOLUME_SPEECH_TEXT,    "IDS_OPTIONS_SPEECH_VOLUME",    "text",      40, 110, 220, 40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#if defined(TARGET_PS2)
    { IDC_AV_TOGGLE_SPEAKER_TEXT,   "IDS_OPTIONS_MONO_STEREO",      "text",      40, 145, 220, 40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#elif defined(TARGET_XBOX)
    { IDC_AV_BRIGHTNESS_TEXT,       "IDS_OPTIONS_BRIGHTNESS",       "text",      40, 160, 220, 40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#endif

    { IDC_AV_VOLUME_SFX,            "IDS_NULL",                     "slider",   310,  50, 120, 40, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_AV_VOLUME_MUSIC,          "IDS_NULL",                     "slider",   310,  85, 120, 40, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_AV_VOLUME_SPEECH,         "IDS_NULL",                     "slider",   310, 120, 120, 40, 0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#if defined(TARGET_PS2)
    { IDC_AV_TOGGLE_SPEAKER,        "IDS_NULL",                     "combo",    300, 154, 140, 40, 0, 3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_AV_HEADSET_TEST,          "IDS_OPTIONS_HEADSET_TEST",     "button",    40, 180, 220, 40, 0, 4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#elif defined TARGET_XBOX
    { IDC_AV_BRIGHTNESS,            "IDS_NULL",                     "slider",   310, 170, 120, 40, 0, 4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_AV_HEADSET_TEST,          "IDS_OPTIONS_HEADSET_TEST",     "button",    40, 216, 220, 40, 0, 5, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#else
    { IDC_AV_HEADSET_TEST,          "IDS_OPTIONS_HEADSET_TEST",     "button",    40, 180, 220, 40, 0, 4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#endif
    { IDC_AV_BUTTON_ACCEPT,         "IDS_PROFILE_OPTIONS_ACCEPT",   "button",    40, 285, 220, 40, 0, 6, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_AV_NAV_TEXT,              "IDS_NULL",                     "text",       0,   0,   0,  0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem AVSettingsDialog =
{
    "IDS_AV_SETTINGS",
    1, 9,
    sizeof(AVSettingsControls)/sizeof(ui_manager::control_tem),
    &AVSettingsControls[0],
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

static s32 CurrSettingsSlot = -1;

//=========================================================================
//  Registration function
//=========================================================================

void dlg_av_settings_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "av settings", &AVSettingsDialog, &dlg_av_settings_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_av_settings_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_av_settings* pDialog = new dlg_av_settings;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_av_settings
//=========================================================================

dlg_av_settings::dlg_av_settings( void )
{
}

//=========================================================================

dlg_av_settings::~dlg_av_settings( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_av_settings::Create( s32                        UserID,
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

    m_pVolumeSFX                = (ui_slider*)  FindChildByID( IDC_AV_VOLUME_SFX           );    
    m_pVolumeMusic	            = (ui_slider*)  FindChildByID( IDC_AV_VOLUME_MUSIC         );
    m_pVolumeSpeech	            = (ui_slider*)  FindChildByID( IDC_AV_VOLUME_SPEECH        );
#ifdef TARGET_XBOX
    m_pBrightness               = (ui_slider*)  FindChildByID( IDC_AV_BRIGHTNESS           );
#endif
#ifdef TARGET_PS2
    m_pToggleSpeakerSetup	    = (ui_combo*)   FindChildByID( IDC_AV_TOGGLE_SPEAKER       );  
#endif
    m_pHeadsetTest              = (ui_button*)  FindChildByID( IDC_AV_HEADSET_TEST         );
    m_pButtonAccept             = (ui_button*)  FindChildByID( IDC_AV_BUTTON_ACCEPT        );

    m_pVolumeSFXText	        = (ui_text*)    FindChildByID( IDC_AV_VOLUME_SFX_TEXT      );
    m_pVolumeMusicText	        = (ui_text*)    FindChildByID( IDC_AV_VOLUME_MUSIC_TEXT    );
    m_pVolumeSpeechText	        = (ui_text*)    FindChildByID( IDC_AV_VOLUME_SPEECH_TEXT   );
#ifdef TARGET_PS2
    m_pToggleSpeakerSetupText	= (ui_text*)    FindChildByID( IDC_AV_TOGGLE_SPEAKER_TEXT  );
#endif
#ifdef TARGET_XBOX
    m_pBrightnessText	        = (ui_text*)    FindChildByID( IDC_AV_BRIGHTNESS_TEXT      );
#endif
    m_pNavText                  = (ui_text*)    FindChildByID( IDC_AV_NAV_TEXT             );

    GotoControl( (ui_control*)m_pVolumeSFX );
    m_CurrentControl = IDC_AV_VOLUME_SFX;
    m_CurrHL = 0;
    m_PopUp = NULL;

    // set range
    m_pVolumeSFX    ->SetRange( 0, 100 );
    m_pVolumeMusic  ->SetRange( 0, 100 );
    m_pVolumeSpeech ->SetRange( 0, 100 );
#ifdef TARGET_XBOX
    m_pBrightness   ->SetRange( 0, 100 );
#endif

    // switch off the controls to start
    m_pVolumeSFX                ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pVolumeMusic              ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pVolumeSpeech             ->SetFlag( ui_win::WF_VISIBLE, FALSE );
#ifdef TARGET_XBOX
    m_pBrightness               ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pBrightness->SetValue( s32(xbox_GetBrightness()*UI_MAX_BRIGHTNESS_RANGE) );
#endif
#ifdef TARGET_PS2
    m_pToggleSpeakerSetup       ->SetFlag( ui_win::WF_VISIBLE, FALSE );
#endif
    m_pHeadsetTest              ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pButtonAccept             ->SetFlag( ui_win::WF_VISIBLE, FALSE );

    m_pVolumeSFXText            ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pVolumeMusicText          ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pVolumeSpeechText         ->SetFlag( ui_win::WF_VISIBLE, FALSE );
#ifdef TARGET_PS2
    m_pToggleSpeakerSetupText   ->SetFlag( ui_win::WF_VISIBLE, FALSE );
#endif
#ifdef TARGET_XBOX
    m_pBrightnessText           ->SetFlag( ui_win::WF_VISIBLE, FALSE );
#endif
    m_pNavText                  ->SetFlag( ui_win::WF_VISIBLE, FALSE );

    m_pVolumeSFXText            ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pVolumeMusicText          ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pVolumeSpeechText         ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
#ifdef TARGET_PS2
    m_pToggleSpeakerSetupText   ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
#endif
#ifdef TARGET_XBOX
    m_pBrightnessText           ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
#endif

    // set button alignment
    m_pHeadsetTest              ->SetFlag( ui_win::WF_BUTTON_LEFT, TRUE );
    m_pButtonAccept             ->SetFlag( ui_win::WF_BUTTON_LEFT, TRUE );

#ifdef TARGET_PS2
    // initialize the speaker setup toggle    
    m_pToggleSpeakerSetup->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT );
    m_pToggleSpeakerSetup->AddItem  ( g_StringTableMgr( "ui", "IDS_MONO"     ), SPEAKERS_MONO              );
    m_pToggleSpeakerSetup->AddItem  ( g_StringTableMgr( "ui", "IDS_STEREO"   ), SPEAKERS_STEREO            );
    m_pToggleSpeakerSetup->AddItem  ( g_StringTableMgr( "ui", "IDS_PROLOGIC" ), SPEAKERS_PROLOGIC          );
#endif

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_CANCEL" );
    navText += g_StringTableMgr( "ui", "IDS_NAV_RESTORE_DEFAULTS" );
  
    m_pNavText->SetLabel( navText );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // set default values from pending settings
    m_Settings = g_StateMgr.GetPendingSettings();

    m_pVolumeSFX    ->SetValue( m_Settings.GetVolume( VOLUME_SFX    ) );
    m_pVolumeMusic  ->SetValue( m_Settings.GetVolume( VOLUME_MUSIC  ) );
    m_pVolumeSpeech ->SetValue( m_Settings.GetVolume( VOLUME_SPEECH ) );
#ifdef TARGET_XBOX
    m_pBrightness   ->SetValue( m_Settings.GetBrightness() );
#endif

#ifdef TARGET_PS2
    m_pToggleSpeakerSetup->SetSelection( m_Settings.GetSpeakerSet() );
#endif

    // initialize screen scaling
    InitScreenScaling( Position );

    // disable background filter
    m_bRenderBlackout = FALSE;

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // turn off default slider sounds
    m_pVolumeSFX    ->UseDefaultSound(FALSE);
    m_pVolumeMusic  ->UseDefaultSound(FALSE);
    m_pVolumeSpeech ->UseDefaultSound(FALSE);

    // Return success code
    return Success;
}

//=========================================================================

void dlg_av_settings::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_av_settings::Render( s32 ox, s32 oy )
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

void dlg_av_settings::OnNotify( ui_win* pWin, ui_win* pSender, s32 Command, void* pData )
{
    (void)pWin;
    (void)pSender;
    (void)Command;
    (void)pData;

    static s32 SpeechVoiceID;

    switch (Command)
    {    
        case WN_SLIDER_CHANGE:
        {
            if ( pSender == (ui_win*)m_pVolumeSFX )
            {
                if( m_pVolumeSFX->GetValue() != m_Settings.GetVolume( VOLUME_SFX ) )
                {
                    m_Settings.SetVolume( VOLUME_SFX, m_pVolumeSFX->GetValue() );
                    m_Settings.CommitAudio();
                    g_AudioMgr.Play("Fader_SFX");
                }
            }
            else if ( pSender == (ui_win*)m_pVolumeMusic )
            {
                if( m_pVolumeMusic->GetValue() != m_Settings.GetVolume( VOLUME_MUSIC ) )
                {
                    m_Settings.SetVolume( VOLUME_MUSIC, m_pVolumeMusic->GetValue() );
                    m_Settings.CommitAudio();
                    g_AudioMgr.Play("Music_Slider");
                }
            }
            else if ( pSender == (ui_win*)m_pVolumeSpeech )
            {
                if( m_pVolumeSpeech->GetValue() != m_Settings.GetVolume( VOLUME_SPEECH ) )
                {
                    g_AudioMgr.SetVolume( SpeechVoiceID, (f32)m_Settings.GetVolume( VOLUME_SPEECH ) / 100.0f );
                    m_Settings.SetVolume( VOLUME_SPEECH, m_pVolumeSpeech->GetValue() );
                    m_Settings.CommitAudio();
                    g_AudioMgr.Release( SpeechVoiceID, 1.0f );
                    SpeechVoiceID = g_AudioMgr.Play("Voice_Slider");
                }
            }
        #ifdef TARGET_XBOX
            else if( pSender == (ui_win*)m_pBrightness )
            {
                s32 Brightness = m_pBrightness->GetValue();
                xbox_SetBrightness( f32(Brightness)/f32(UI_MAX_BRIGHTNESS_RANGE) );
                m_Settings.SetBrightness( Brightness );
            }
        #endif
        }
        break;

    #ifdef TARGET_PS2
        case WN_COMBO_SELCHANGE:
        {
            if ( pSender == (ui_win*)m_pToggleSpeakerSetup )
            {
                m_Settings.SetSpeakerSet( m_pToggleSpeakerSetup->GetSelectedItemData() );
            }
        }
        break;
    #endif
        default:
            break;
    }
}

//=========================================================================

void dlg_av_settings::OnPadSelect( ui_win* pWin )
{
    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( pWin == (ui_win*)m_pHeadsetTest )
        {
            // get the pending settings
            global_settings& Settings = g_StateMgr.GetPendingSettings();

            // copy changes
            Settings = m_Settings;

            // update the fader volumes
            Settings.CommitAudio();

            // goto headset test screen
            g_AudioMgr.Play("Select_Norm");
            m_State = DIALOG_STATE_SELECT;
        }
        else if( pWin == (ui_win*)m_pButtonAccept )
        {
            g_AudioMgr.Play("Select_Norm");
            
            // get the pending settings
            global_settings& Settings = g_StateMgr.GetPendingSettings();

            // copy changes
            Settings = m_Settings;

            // update the fader volumes
            Settings.CommitAudio();

            // check if anything changed
            if( g_StateMgr.GetPendingSettings().HasChanged() )
            {            
                if( GameMgr.GameInProgress() && g_NetworkMgr.IsOnline() )
                {
                    // don't save whilst online

                    // update the changes in the profile
                    g_StateMgr.ActivatePendingSettings(); 
                    // apply the settings
                    global_settings& Settings = g_StateMgr.GetActiveSettings();
                    Settings.Commit();
                    // Mark Dirty so we prompt to save when we exit the game
                    Settings.MarkDirty();

                    // return to pause menu
                    m_State = DIALOG_STATE_BACK;
                }
                else
                {
                    // ask to save

                    // changes have been made - prompt to save
                    irect r = g_UiMgr->GetUserBounds( g_UiUserID );
                    m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
                    m_PopUpType = POPUP_CONFIRM_SETTINGS;

                    // set nav text
                    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
                    navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
                    m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

                    // configure message
                    m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_SETTINGS_EDIT" ), 
                        TRUE, 
                        TRUE, 
                        FALSE, 
                        g_StringTableMgr( "ui", "IDS_SETTINGS_CHANGED_MSG" ),
                        navText,
                        &m_PopUpResult );

                    m_State = DIALOG_STATE_POPUP;
                    return;
                }
            }
            else
            {
                // no changes - return to previous screen
                g_AudioMgr.Play( "Backup" );
                m_State = DIALOG_STATE_BACK;
            }
        }
    }
}

//=========================================================================

void dlg_av_settings::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        // no changes - return to previous screen
        g_AudioMgr.Play( "Backup" );
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_av_settings::OnPadDelete( ui_win* pWin )
{
    (void)pWin;

    // restore defaults
    if( m_State == DIALOG_STATE_ACTIVE )
    {
        g_AudioMgr.Play("Select_Norm");

        // get default AV settings
        m_Settings.Reset( RESET_AUDIO );

        // update the fader volumes
        m_Settings.CommitAudio();

        // update controls with changes
        m_pVolumeSFX    ->SetValue( m_Settings.GetVolume( VOLUME_SFX    ) );
        m_pVolumeMusic  ->SetValue( m_Settings.GetVolume( VOLUME_MUSIC  ) );
        m_pVolumeSpeech ->SetValue( m_Settings.GetVolume( VOLUME_SPEECH ) );

#ifdef TARGET_PS2
        m_pToggleSpeakerSetup->SetSelection( m_Settings.GetSpeakerSet() );
#endif
#ifdef TARGET_XBOX
        m_pBrightness->SetValue( m_Settings.GetBrightness() );
#endif
    }
}

//=========================================================================

void dlg_av_settings::OnUpdate ( ui_win* pWin, f32 DeltaTime )
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
            m_pVolumeSFX                ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pVolumeMusic              ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pVolumeSpeech             ->SetFlag( ui_win::WF_VISIBLE, TRUE );
#ifdef TARGET_PS2
            m_pToggleSpeakerSetup       ->SetFlag( ui_win::WF_VISIBLE, TRUE );
#endif
#ifdef TARGET_XBOX
            m_pBrightness               ->SetFlag( ui_win::WF_VISIBLE, TRUE );
#endif
            m_pButtonAccept             ->SetFlag( ui_win::WF_VISIBLE, TRUE );

            m_pVolumeSFXText            ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pVolumeMusicText          ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pVolumeSpeechText         ->SetFlag( ui_win::WF_VISIBLE, TRUE );
#ifdef TARGET_PS2
            m_pToggleSpeakerSetupText   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
#endif
#ifdef TARGET_XBOX
            m_pBrightnessText           ->SetFlag( ui_win::WF_VISIBLE, TRUE );

            if( g_MatchMgr.GetAuthStatus() == AUTH_STAT_CONNECTED )
            {
                m_pHeadsetTest          ->SetFlag( ui_win::WF_DISABLED, FALSE );
                m_pHeadsetTest          ->SetFlag( ui_win::WF_VISIBLE,  TRUE  );
            }
            else
            {
                m_pHeadsetTest          ->SetFlag( ui_win::WF_DISABLED, TRUE  );
                m_pHeadsetTest          ->SetFlag( ui_win::WF_VISIBLE,  TRUE  );
            }
#endif
            m_pNavText                  ->SetFlag( ui_win::WF_VISIBLE, TRUE );

            if( m_pHeadsetTest->GetFlags( ui_win::WF_DISABLED ) == FALSE )
            {
                m_pHeadsetTest          ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            }

            GotoControl( (ui_control*)m_pVolumeSFX );
            m_pVolumeSFX->SetFlag(WF_HIGHLIGHT, TRUE);        
            g_UiMgr->SetScreenHighlight( m_pVolumeSFXText->GetPosition() );
        }
    }

    // check for result of popup box
    if ( m_PopUp )
    {
        if ( m_PopUpResult != DLG_POPUP_IDLE )
        {
            if( m_PopUpType == POPUP_CONFIRM_SETTINGS )
            {
                // save changes?
                if ( m_PopUpResult == DLG_POPUP_YES )
                {
                    // check if the settings are saved 
                    if( g_StateMgr.GetSettingsCardSlot() == -1 )
                    {
#ifdef TARGET_PS2
                        // not saved - goto memory card select screen
                        m_State = DIALOG_STATE_MEMCARD_ERROR;
#else
                        // check for corrupt settings
                        if( g_UIMemCardMgr.FoundSettings() )
                        {
                            // ask permission to overwrite old settings
                            irect r = g_UiMgr->GetUserBounds( g_UiUserID );
                            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
                            m_PopUpType = POPUP_OVERWRITE_SETTINGS;

                            // set nav text
                            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
                            navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
                            m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

                            // configure message
                            m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_SETTINGS_EDIT" ), 
                                TRUE, 
                                TRUE, 
                                FALSE, 
                                g_StringTableMgr( "ui", "IDS_SETTINGS_CHANGED_OVERWRITE" ),
                                navText,
                                &m_PopUpResult );

                            return;
                        }
                        else
                        {
                            // attempt to create settings
                            g_StateMgr.SetSettingsCardSlot( 0 );
                            g_UIMemCardMgr.CreateSettings( this, &dlg_av_settings::OnSaveSettingsCB );
                            m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
                        }
#endif
                    }
                    else
                    {
                        // OK. attempt to save changes to memcard
                        g_AudioMgr.Play("Select_Norm");
                        CurrSettingsSlot = g_StateMgr.GetSettingsCardSlot();
                        g_UIMemCardMgr.SaveSettings(this, &dlg_av_settings::OnSaveSettingsCB );
                        m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
                    }
                }
                else
                {
                    // Activate the pending settings right now. Even though the player opted not to
                    // save, the settings should be preserved locally.
                    g_StateMgr.ActivatePendingSettings();
                    // apply settings
                    global_settings& Settings = g_StateMgr.GetActiveSettings();
                    Settings.Commit();
                    g_AudioMgr.Play("Backup");
                    m_State = DIALOG_STATE_BACK;
                }

                // clear popup 
                m_PopUp = NULL;

                // turn on nav text
                m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
            }
            else
            {
                if( m_PopUpResult == DLG_POPUP_YES )
                {
                    // overwrite the old settings
                    g_AudioMgr.Play("Select_Norm");
                    g_StateMgr.SetSettingsCardSlot( 0 );
                    g_UIMemCardMgr.SaveSettings( this, &dlg_av_settings::OnSaveSettingsCB );
                    m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
                }
                else
                {
                    // Activate the pending settings right now. Even though the player opted not to
                    // save, the settings should be preserved locally.
                    g_StateMgr.ActivatePendingSettings();
                    // apply settings
                    global_settings& Settings = g_StateMgr.GetActiveSettings();
                    Settings.Commit();
                    g_AudioMgr.Play("Backup");
                    m_State = DIALOG_STATE_BACK;
                }

                // clear popup 
                m_PopUp = NULL;

                // turn on nav text
                m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
            }
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    // update labels
    if( m_pVolumeSFX->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 0;
        m_pVolumeSFXText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pVolumeSFXText->GetPosition() );
    }
    else
        m_pVolumeSFXText->SetLabelColor( xcolor(126,220,60,255) );

    if( m_pVolumeMusic->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 1;
        m_pVolumeMusicText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pVolumeMusicText->GetPosition() );
    }
    else
        m_pVolumeMusicText->SetLabelColor( xcolor(126,220,60,255) );
    
    if( m_pVolumeSpeech->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 2;

        m_pVolumeSpeechText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pVolumeSpeechText->GetPosition() );
    }
    else
        m_pVolumeSpeechText->SetLabelColor( xcolor(126,220,60,255) );
    
#ifdef TARGET_PS2
    if( m_pToggleSpeakerSetup->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 3;
        m_pToggleSpeakerSetupText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pToggleSpeakerSetupText->GetPosition() );
    }
    else
        m_pToggleSpeakerSetupText->SetLabelColor( xcolor(126,220,60,255) );
#endif
       
    if( m_pHeadsetTest->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 4;
        g_UiMgr->SetScreenHighlight( m_pHeadsetTest->GetPosition() );
    }

#ifdef TARGET_XBOX
    if( m_pBrightness->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 5;
        m_pBrightnessText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pBrightnessText->GetPosition() );
    }
    else
        m_pBrightnessText->SetLabelColor( xcolor(126,220,60,255) );
#endif

    if( m_pButtonAccept->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 6;
        g_UiMgr->SetScreenHighlight( m_pButtonAccept->GetPosition() );
    }

    if( highLight != m_CurrHL )
    {
        if( highLight != -1 )
            g_AudioMgr.Play("Cusor_Norm");

        m_CurrHL = highLight;
    }
}

//=========================================================================

void dlg_av_settings::OnSaveSettingsCB( void )
{
    // check if the save was successful (OR user wants to continue without saving)
#ifdef TARGET_PS2
    s32 CheckCardSlot;
    if( g_StateMgr.GetSettingsCardSlot() == -1 )
    {
        CheckCardSlot = CurrSettingsSlot;
    }
    else
    {
        CheckCardSlot = g_StateMgr.GetSettingsCardSlot();
    }
    MemCardMgr::condition& Condition1 = g_UIMemCardMgr.GetCondition( CheckCardSlot );
#else
    MemCardMgr::condition& Condition1 = g_UIMemCardMgr.GetCondition( 0 );
#endif
    if( Condition1.SuccessCode )
    {
        // activate the new settings
        g_StateMgr.ActivatePendingSettings();

        // continue without saving?
        if( Condition1.bCancelled )
        {
            // clear settings card slot
            g_StateMgr.SetSettingsCardSlot(-1);
        }

        // save successful - return to main menu
        g_AudioMgr.Play( "Select_Norm" );
        m_State = DIALOG_STATE_BACK;            
    }
    else
    {
        // save failed! - goto memcard select dialog
        g_AudioMgr.Play( "Select_Norm" );

        // clear settings card slot
        g_StateMgr.SetSettingsCardSlot(-1);

        // handle error state
        m_State = DIALOG_STATE_MEMCARD_ERROR;
    }
}

//=========================================================================
