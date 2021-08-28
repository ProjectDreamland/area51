//=========================================================================
//
//  dlg_profile_controls.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"

#include "dlg_ProfileControls.hpp"
#include "stringmgr\stringmgr.hpp"
#include "StateMgr\StateMgr.hpp"

//=========================================================================
//  Main Options Dialog
//=========================================================================

enum controls
{
    IDC_CONTROLS_INVERTY_TEXT,
    IDC_CONTROLS_SENSITIVITY_X_TEXT,
    IDC_CONTROLS_SENSITIVITY_Y_TEXT,
    IDC_CONTROLS_CROUCH_TEXT,
    //IDC_CONTROLS_LOOK_TEXT,
    IDC_CONTROLS_VIBRATION_TEXT,
    IDC_CONTROLS_AUTO_SWITCH_TEXT,

    IDC_CONTROLS_TOGGLE_INVERTY,
    IDC_CONTROLS_SENSITIVITY_X,
    IDC_CONTROLS_SENSITIVITY_Y,
    IDC_CONTROLS_TOGGLE_CROUCH,
    //IDC_CONTROLS_TOGGLE_LOOK,
    IDC_CONTROLS_TOGGLE_VIBRATION,
    IDC_CONTROLS_TOGGLE_AUTO_SWITCH,
    IDC_CONTROLS_BUTTON_ACCEPT,

    IDC_CONTROLS_NAV_TEXT,
};


ui_manager::control_tem ProfileControlsControls[] = 
{
    // Frames.
    { IDC_CONTROLS_INVERTY_TEXT,        "IDS_OPTIONS_TOGGLE_INVERTY",   "text",      40,  40, 220, 40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_CONTROLS_SENSITIVITY_X_TEXT,  "IDS_OPTIONS_SENSITIVITY_X",    "text",      40,  70, 220, 40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_CONTROLS_SENSITIVITY_Y_TEXT,  "IDS_OPTIONS_SENSITIVITY_Y",    "text",      40, 100, 220, 40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_CONTROLS_CROUCH_TEXT,         "IDS_OPTIONS_TOGGLE_CROUCH",    "text",      40, 130, 220, 40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    //{ IDC_CONTROLS_LOOK_TEXT,           "IDS_OPTIONS_LOOKSPRING",       "text",      40, 160, 220, 40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_CONTROLS_VIBRATION_TEXT,      "IDS_OPTIONS_VIBRATION",        "text",      40, 160, 220, 40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_CONTROLS_AUTO_SWITCH_TEXT,    "IDS_OPTIONS_AUTO_SWITCH",      "text",      40, 190, 220, 40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_CONTROLS_TOGGLE_INVERTY,      "IDS_NULL",                     "check",    343,  52, 120, 40, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_CONTROLS_SENSITIVITY_X,       "IDS_NULL",                     "slider",   290,  80, 120, 40, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_CONTROLS_SENSITIVITY_Y,       "IDS_NULL",                     "slider",   290, 110, 120, 40, 0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_CONTROLS_TOGGLE_CROUCH,       "IDS_NULL",                     "check",    343, 142, 120, 40, 0, 3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    //{ IDC_CONTROLS_TOGGLE_LOOK,         "IDS_NULL",                     "check",    343, 172, 120, 40, 0, 4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_CONTROLS_TOGGLE_VIBRATION,    "IDS_NULL",                     "check",    343, 172, 120, 40, 0, 5, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_CONTROLS_TOGGLE_AUTO_SWITCH,  "IDS_NULL",                     "check",    343, 202, 120, 40, 0, 6, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_CONTROLS_BUTTON_ACCEPT,       "IDS_PROFILE_OPTIONS_ACCEPT",   "button",    40, 285, 220, 40, 0, 7, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_CONTROLS_NAV_TEXT,            "IDS_NULL",                     "text",       0,   0,   0,  0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem ProfileControlsDialog =
{
    "IDS_PROFILE_CONTROLS",
    1, 9,
    sizeof(ProfileControlsControls)/sizeof(ui_manager::control_tem),
    &ProfileControlsControls[0],
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

void dlg_profile_controls_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "profile controls", &ProfileControlsDialog, &dlg_profile_controls_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_profile_controls_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_profile_controls* pDialog = new dlg_profile_controls;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_profile_controls
//=========================================================================

dlg_profile_controls::dlg_profile_controls( void )
{
}

//=========================================================================

dlg_profile_controls::~dlg_profile_controls( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_profile_controls::Create( s32                        UserID,
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

    m_pToggleInvertY        = (ui_check*)   FindChildByID( IDC_CONTROLS_TOGGLE_INVERTY     );    
    m_pSensitivityX         = (ui_slider*)  FindChildByID( IDC_CONTROLS_SENSITIVITY_X      );
    m_pSensitivityY         = (ui_slider*)  FindChildByID( IDC_CONTROLS_SENSITIVITY_Y      );
    m_pToggleCrouch         = (ui_check*)   FindChildByID( IDC_CONTROLS_TOGGLE_CROUCH      );    
    //m_pToggleLookspring     = (ui_check*)   FindChildByID( IDC_CONTROLS_TOGGLE_LOOK        );    
    m_pToggleVibration      = (ui_check*)   FindChildByID( IDC_CONTROLS_TOGGLE_VIBRATION   );
    m_pToggleAutoSwitch     = (ui_check*)   FindChildByID( IDC_CONTROLS_TOGGLE_AUTO_SWITCH );
    m_pButtonAccept         = (ui_button*)  FindChildByID( IDC_CONTROLS_BUTTON_ACCEPT      );

    m_pInvertYText          = (ui_text*)    FindChildByID( IDC_CONTROLS_INVERTY_TEXT       );
    m_pSensitivityXText     = (ui_text*)    FindChildByID( IDC_CONTROLS_SENSITIVITY_X_TEXT );
    m_pSensitivityYText     = (ui_text*)    FindChildByID( IDC_CONTROLS_SENSITIVITY_Y_TEXT );
    m_pCrouchText           = (ui_text*)    FindChildByID( IDC_CONTROLS_CROUCH_TEXT        );
    //m_pLookspringText       = (ui_text*)    FindChildByID( IDC_CONTROLS_LOOK_TEXT          );
    m_pVibrationText        = (ui_text*)    FindChildByID( IDC_CONTROLS_VIBRATION_TEXT     );
    m_pAutoSwitchText       = (ui_text*)    FindChildByID( IDC_CONTROLS_AUTO_SWITCH_TEXT   );
    m_pNavText              = (ui_text*)    FindChildByID( IDC_CONTROLS_NAV_TEXT           );

    GotoControl( (ui_control*)m_pToggleInvertY );
    m_CurrentControl = IDC_CONTROLS_TOGGLE_INVERTY;
    m_CurrHL = 0;

    // set range
    m_pSensitivityX->SetRange( 0, 100 );
    m_pSensitivityY->SetRange( 0, 100 );

    // switch off the controls to start
    m_pToggleInvertY    ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pSensitivityX     ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pSensitivityY     ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pToggleCrouch     ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    //m_pToggleLookspring ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pToggleVibration  ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pToggleAutoSwitch ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pButtonAccept     ->SetFlag( ui_win::WF_VISIBLE, FALSE );

    m_pInvertYText      ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pSensitivityXText ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pSensitivityYText ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pCrouchText       ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    //m_pLookspringText   ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pVibrationText    ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pAutoSwitchText   ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pNavText          ->SetFlag( ui_win::WF_VISIBLE, FALSE );

    m_pInvertYText      ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pSensitivityXText ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pSensitivityYText ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pCrouchText       ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    //m_pLookspringText   ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pVibrationText    ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pAutoSwitchText   ->SetLabelFlags( ui_font::h_left|ui_font::v_center );

    // set button alignment
    m_pButtonAccept     ->SetFlag( ui_win::WF_BUTTON_LEFT, TRUE );

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_CANCEL" );
    navText += g_StringTableMgr( "ui", "IDS_NAV_RESTORE_DEFAULTS" );
  
    m_pNavText->SetLabel( navText );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // set default values (should be set from options data)
    player_profile& Profile = g_StateMgr.GetPendingProfile();

    m_pSensitivityX->SetValue( Profile.GetSensitivity(0) );
    m_pSensitivityY->SetValue( Profile.GetSensitivity(1) );

    m_pToggleInvertY    ->SetChecked( Profile.m_bInvertY            );
    m_pToggleCrouch     ->SetChecked( Profile.m_bCrouchOn           );
    //m_pToggleLookspring ->SetChecked( Profile.m_bLookspringOn       );
    m_pToggleVibration  ->SetChecked( Profile.m_bVibration          );
    m_pToggleAutoSwitch ->SetChecked( Profile.GetWeaponAutoSwitch() );

    // initialize screen scaling
    InitScreenScaling( Position );

    // disable background filter
    m_bRenderBlackout = FALSE;

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_profile_controls::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_profile_controls::Render( s32 ox, s32 oy )
{
    static s32 offset   =  0;
    static s32 gap      =  9;
    static s32 width    =  4;

    irect rb;    
    
    // render background filter
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

void dlg_profile_controls::OnPadSelect( ui_win* pWin )
{
    // accept changes
    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( pWin == (ui_win*)m_pButtonAccept )
        {
            g_AudioMgr.Play("Select_Norm");
                       
            // get the pending profile
            player_profile& Profile = g_StateMgr.GetPendingProfile();

            // update the settings from the controls
            Profile.SetSensitivity( 0, m_pSensitivityX->GetValue() );
            Profile.SetSensitivity( 1, m_pSensitivityY->GetValue() );

            Profile.m_bCrouchOn     = m_pToggleCrouch     ->IsChecked();
            Profile.m_bInvertY      = m_pToggleInvertY    ->IsChecked();
            Profile.m_bVibration    = m_pToggleVibration  ->IsChecked();
            //  Profile.m_bLookspringOn = m_pToggleLookspring ->IsChecked();

            Profile.SetWeaponAutoSwitch( m_pToggleAutoSwitch->IsChecked() );

            m_State = DIALOG_STATE_BACK;
        }
    }
}

//=========================================================================

void dlg_profile_controls::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    // cancel
    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        g_AudioMgr.Play( "Backup" );
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_profile_controls::OnPadDelete( ui_win* pWin )
{
    (void)pWin;

    // restore defaults
    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        // get the default controller settings
        player_profile TempProfile;
        TempProfile.RestoreControlDefaults();

        // set controls to default settings
        m_pSensitivityX     ->SetValue  ( TempProfile.GetSensitivity(0)     );
        m_pSensitivityY     ->SetValue  ( TempProfile.GetSensitivity(1)     );
        m_pToggleInvertY    ->SetChecked( TempProfile.m_bInvertY            );
        m_pToggleCrouch     ->SetChecked( TempProfile.m_bCrouchOn           );
        //m_pToggleLookspring ->SetChecked( TempProfile.m_bLookspringOn       );
        m_pToggleVibration  ->SetChecked( TempProfile.m_bVibration          );
        m_pToggleAutoSwitch ->SetChecked( TempProfile.GetWeaponAutoSwitch() );

        g_AudioMgr.Play("Select_Norm");
    }
}

//=========================================================================

void dlg_profile_controls::OnUpdate ( ui_win* pWin, f32 DeltaTime )
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
            m_pToggleInvertY    ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pSensitivityX     ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pSensitivityY     ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pToggleCrouch     ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            //m_pToggleLookspring ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pToggleVibration  ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pToggleAutoSwitch ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pButtonAccept     ->SetFlag( ui_win::WF_VISIBLE, TRUE );

            m_pInvertYText      ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pSensitivityXText ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pSensitivityYText ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pCrouchText       ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            //m_pLookspringText   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pVibrationText    ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pAutoSwitchText   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pNavText          ->SetFlag( ui_win::WF_VISIBLE, TRUE );

            GotoControl( (ui_control*)m_pToggleInvertY );
            m_pToggleInvertY->SetFlag( WF_HIGHLIGHT, TRUE );
            g_UiMgr->SetScreenHighlight( m_pInvertYText->GetPosition() );
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    // update labels
    if( m_pToggleInvertY->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 0;
        m_pInvertYText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pInvertYText->GetPosition() );
    }
    else
        m_pInvertYText->SetLabelColor( xcolor(126,220,60,255) );

    if( m_pSensitivityX->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 1;
        m_pSensitivityXText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pSensitivityXText->GetPosition() );
    }
    else
        m_pSensitivityXText->SetLabelColor( xcolor(126,220,60,255) );
    
    if( m_pSensitivityY->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 2;
        m_pSensitivityYText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pSensitivityYText->GetPosition() );
    }
    else
        m_pSensitivityYText->SetLabelColor( xcolor(126,220,60,255) );
    
    if( m_pToggleCrouch->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 3;
        m_pCrouchText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pCrouchText->GetPosition() );
    }
    else
        m_pCrouchText->SetLabelColor( xcolor(126,220,60,255) );
    
    //if( m_pToggleLookspring->GetFlags(WF_HIGHLIGHT) )
    //{
    //    highLight = 4;
    //    m_pLookspringText->SetLabelColor( xcolor(255,252,204,255) );
    //    g_UiMgr->SetScreenHighlight( m_pLookspringText->GetPosition() );
    //}
    //else
    //    m_pLookspringText->SetLabelColor( xcolor(126,220,60,255) );

    if( m_pToggleVibration->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 5;
        m_pVibrationText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pVibrationText->GetPosition() );
    }
    else
        m_pVibrationText->SetLabelColor( xcolor(126,220,60,255) );

    if( m_pToggleAutoSwitch->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 6;
        m_pAutoSwitchText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pAutoSwitchText->GetPosition() );
    }
    else
        m_pAutoSwitchText->SetLabelColor( xcolor(126,220,60,255) );

    if( m_pButtonAccept->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 7;
        m_pButtonAccept->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pButtonAccept->GetPosition() );
    }
    else
        m_pButtonAccept->SetLabelColor( xcolor(126,220,60,255) );

    if( highLight != m_CurrHL )
    {
        if( highLight != -1 )
            g_AudioMgr.Play("Cusor_Norm");

        m_CurrHL = highLight;
    }
}

//=========================================================================
