//=========================================================================
//
//  dlg_profile_options.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_blankbox.hpp"

#include "dlg_PopUp.hpp"
#include "dlg_ProfileOptions.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "MemCardMgr/MemCardMgr.hpp"

//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum
{
    OPTIONS_POPUP_CANCEL_CREATE,
    OPTIONS_POPUP_XBOX_FREE_BLOCKS,
    OPTIONS_POPUP_PROFILE_HAS_CHANGED,
};

// x, y, w, h
s32 s_EDX = 80;
s32 s_EDW = 140;

s32 s_PDX = 100;
s32 s_PDW = 185;

ui_manager::control_tem ProfileOptionsControls_PAL[] = 
{
    { IDC_PROFILE_OPTIONS_CONTROLS,         "IDS_PROFILE_OPTIONS_CONTROLS",     "button",   s_PDX,   40,    s_PDW, 40, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PROFILE_OPTIONS_AVATAR,           "IDS_PROFILE_OPTIONS_AVATAR",       "button",   s_PDX,   80,    s_PDW, 40, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PROFILE_OPTIONS_DIFFICULTY,       "IDS_PROFILE_OPTIONS_DIFFICULTY",   "button",   s_PDX,  120,    s_PDW, 40, 0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PROFILE_ONLINE_STATUS,            "IDS_PROFILE_OPTIONS_ONLINE_STATUS","button",   s_PDX,  160,    s_PDW, 40, 0, 4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PROFILE_OPTIONS_AUTOSAVE,         "IDS_PROFILE_OPTIONS_AUTOSAVE",     "button",   s_PDX,  200,    s_PDW, 40, 0, 6, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PROFILE_OPTIONS_ACCEPT,           "IDS_PROFILE_OPTIONS_ACCEPT",       "button",   s_PDX,  285,    s_PDW, 40, 0, 8, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_PROFILE_DIFFICULTY_BBOX,          "IDS_PROFILE_SELECT_DIFFICULTY",    "blankbox", 63,      120,     265, 70, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE | ui_win::WF_STATIC },
    { IDC_PROFILE_DIFFICULTY_SELECT,        "IDS_NULL",                         "combo",    100,     150,   s_PDW, 40, 0, 3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_PROFILE_STATUS_BBOX,              "IDS_PROFILE_SELECT_ONLINE_STATUS", "blankbox", 65,      160,     260, 70, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE | ui_win::WF_STATIC },
    { IDC_PROFILE_STATUS_SELECT,            "IDS_NULL",                         "combo",    85,      190,     220, 40, 0, 5, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_PROFILE_AUTOSAVE_BBOX,            "IDS_PROFILE_SELECT_AUTOSAVE",      "blankbox", 30,      200,     326, 70, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE | ui_win::WF_STATIC },
    { IDC_PROFILE_AUTOSAVE_SELECT,          "IDS_NULL",                         "combo",    45,      230,     295, 40, 0, 7, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_PROFILE_OPTIONS_NAV_TEXT,         "IDS_NULL",                         "text",      0,        0,       0,  0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

};

ui_manager::control_tem ProfileOptionsControls_ENG[] =
{
    { IDC_PROFILE_OPTIONS_CONTROLS,         "IDS_PROFILE_OPTIONS_CONTROLS",     "button",   s_EDX,   40,    s_EDW, 40, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PROFILE_OPTIONS_AVATAR,           "IDS_PROFILE_OPTIONS_AVATAR",       "button",   s_EDX,   80,    s_EDW, 40, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PROFILE_OPTIONS_DIFFICULTY,       "IDS_PROFILE_OPTIONS_DIFFICULTY",   "button",   s_EDX,  120,    s_EDW, 40, 0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PROFILE_ONLINE_STATUS,            "IDS_PROFILE_OPTIONS_ONLINE_STATUS","button",   s_EDX,  160,    s_EDW, 40, 0, 4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PROFILE_OPTIONS_AUTOSAVE,         "IDS_PROFILE_OPTIONS_AUTOSAVE",     "button",   s_EDX,  200,    s_EDW, 40, 0, 6, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PROFILE_OPTIONS_ACCEPT,           "IDS_PROFILE_OPTIONS_ACCEPT",       "button",   s_EDX,  285,    s_EDW, 40, 0, 8, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_PROFILE_DIFFICULTY_BBOX,          "IDS_PROFILE_SELECT_DIFFICULTY",    "blankbox", 50,      120,     200,   70, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE | ui_win::WF_STATIC },
    { IDC_PROFILE_DIFFICULTY_SELECT,        "IDS_NULL",                         "combo",    s_EDX,   150,   s_EDW, 40, 0, 3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_PROFILE_STATUS_BBOX,              "IDS_PROFILE_SELECT_ONLINE_STATUS", "blankbox", 50,      160,     200, 70, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE | ui_win::WF_STATIC },
    { IDC_PROFILE_STATUS_SELECT,            "IDS_NULL",                         "combo",    70,      190,     160, 40, 0, 5, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_PROFILE_AUTOSAVE_BBOX,            "IDS_PROFILE_SELECT_AUTOSAVE",      "blankbox", 45,      200,     210, 70, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE | ui_win::WF_STATIC },
    { IDC_PROFILE_AUTOSAVE_SELECT,          "IDS_NULL",                         "combo",    55,      230,     190, 40, 0, 7, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_PROFILE_OPTIONS_NAV_TEXT,         "IDS_NULL",                         "text",      0,        0,       0,  0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};

ui_manager::dialog_tem ProfileOptionsDialog =
{
    "IDS_PROFILE_OPTIONS_TITLE_EDIT",
        1, 10,
        sizeof(ProfileOptionsControls_ENG)/sizeof(ui_manager::control_tem),
        &ProfileOptionsControls_ENG[0],
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

void dlg_profile_options_register( ui_manager* pManager )
{
#ifndef X_EDITOR
    switch( x_GetLocale() )
    {        
    case XL_LANG_ENGLISH:    // English uses default
        break;

    default:  // PAL
        {
            // set up new profile options dialog controls
            ProfileOptionsDialog.StringID = "IDS_PROFILE_OPTIONS_TITLE_EDIT";
            ProfileOptionsDialog.NavW = 1;
            ProfileOptionsDialog.NavH = 10;
            ProfileOptionsDialog.nControls = sizeof(ProfileOptionsControls_PAL)/sizeof(ui_manager::control_tem);
            ProfileOptionsDialog.pControls = &ProfileOptionsControls_PAL[0];
            ProfileOptionsDialog.FocusControl = 0;
        }
        break;

    }
#endif    

    pManager->RegisterDialogClass( "profile options", &ProfileOptionsDialog, &dlg_profile_options_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_profile_options_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_profile_options* pDialog = new dlg_profile_options;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_profile_options
//=========================================================================

dlg_profile_options::dlg_profile_options( void )
{
}

//=========================================================================

dlg_profile_options::~dlg_profile_options( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_profile_options::Create( s32                        UserID,
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

    m_pButtonControls       = (ui_button*)  FindChildByID( IDC_PROFILE_OPTIONS_CONTROLS     );
    m_pButtonAvatar         = (ui_button*)  FindChildByID( IDC_PROFILE_OPTIONS_AVATAR       );
    m_pButtonDifficulty     = (ui_button*)  FindChildByID( IDC_PROFILE_OPTIONS_DIFFICULTY   );
    m_pButtonOnlineStatus   = (ui_button*)  FindChildByID( IDC_PROFILE_ONLINE_STATUS        );
    m_pButtonAutosave       = (ui_button*)  FindChildByID( IDC_PROFILE_OPTIONS_AUTOSAVE     );
    m_pButtonCreate         = (ui_button*)  FindChildByID( IDC_PROFILE_OPTIONS_ACCEPT       );

    m_pDifficultyBBox       = (ui_blankbox*)FindChildByID( IDC_PROFILE_DIFFICULTY_BBOX      );
    m_pDifficultySelect     = (ui_combo*)   FindChildByID( IDC_PROFILE_DIFFICULTY_SELECT    );

    m_pOnlineStatusBBox     = (ui_blankbox*)FindChildByID( IDC_PROFILE_STATUS_BBOX          );
    m_pOnlineStatusSelect   = (ui_combo*)   FindChildByID( IDC_PROFILE_STATUS_SELECT        );

    m_pAutosaveBBox         = (ui_blankbox*)FindChildByID( IDC_PROFILE_AUTOSAVE_BBOX        );
    m_pAutosaveSelect       = (ui_combo*)   FindChildByID( IDC_PROFILE_AUTOSAVE_SELECT      );

    m_pNavText              = (ui_text*)    FindChildByID( IDC_PROFILE_OPTIONS_NAV_TEXT     );

    s32 iControl = g_StateMgr.GetCurrentControl();
    if( (iControl == -1) || (GotoControl(iControl)==NULL) )
    {
        GotoControl( (ui_control*)m_pButtonControls );
        m_CurrentControl =  IDC_PROFILE_OPTIONS_CONTROLS;
    }
    else
    {
        m_CurrentControl = iControl;
    }

    m_CurrHL = 0;
    m_PopUp = NULL;

    // switch off the buttons to start
    m_pButtonControls       ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonAvatar         ->SetFlag(ui_win::WF_VISIBLE, FALSE);    
    m_pButtonDifficulty     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonAutosave       ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonCreate         ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pDifficultyBBox       ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pDifficultySelect     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonCreate         ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonOnlineStatus   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pOnlineStatusBBox     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pOnlineStatusSelect   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pAutosaveBBox         ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pAutosaveSelect       ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pNavText              ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // disable pop up selections
    m_pDifficultySelect     ->SetFlag(ui_win::WF_DISABLED, TRUE);
    m_pOnlineStatusSelect   ->SetFlag(ui_win::WF_DISABLED, TRUE);
    m_pAutosaveSelect       ->SetFlag(ui_win::WF_DISABLED, TRUE);

    // set up difficulty blankbox   
    m_pDifficultyBBox->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pDifficultyBBox->SetHasTitleBar( TRUE );
    m_pDifficultyBBox->SetLabelColor( xcolor(255,252,204,255) );
    m_pDifficultyBBox->SetTitleBarColor( xcolor(19,59,14,196) );

    // set up difficulty combo
    m_pDifficultySelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV );
    m_pDifficultySelect->AddItem  ( g_StringTableMgr( "ui", "IDS_DIFFICULTY_EASY"   ), DIFFICULTY_EASY   );
    m_pDifficultySelect->AddItem  ( g_StringTableMgr( "ui", "IDS_DIFFICULTY_MEDIUM" ), DIFFICULTY_MEDIUM );
    // check if hard is available
    player_profile& Profile = g_StateMgr.GetPendingProfile();
    if( Profile.m_bHardUnlocked )
    {
        m_pDifficultySelect->AddItem  ( g_StringTableMgr( "ui", "IDS_DIFFICULTY_HARD"   ), DIFFICULTY_HARD   );
    }

    // set up status blankbox
    m_pOnlineStatusBBox->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pOnlineStatusBBox->SetHasTitleBar( TRUE );
    m_pOnlineStatusBBox->SetLabelColor( xcolor(255,252,204,255) );
    m_pOnlineStatusBBox->SetTitleBarColor( xcolor(19,59,14,196) );

    // set up status combo
    m_pOnlineStatusSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV );
    m_pOnlineStatusSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_ONLINE_STATUS_APPEAR_OFFLINE" ), 0 );
    m_pOnlineStatusSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_ONLINE_STATUS_APPEAR_ONLINE"  ), 1 );    

    // set up autosave blankbox
    m_pAutosaveBBox->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pAutosaveBBox->SetHasTitleBar( TRUE );
    m_pAutosaveBBox->SetLabelColor( xcolor(255,252,204,255) );
    m_pAutosaveBBox->SetTitleBarColor( xcolor(19,59,14,196) );

    // set up the autosave combo
    m_pAutosaveSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV );
    m_pAutosaveSelect->AddItem( g_StringTableMgr( "ui", "IDS_PROFILE_AUTOSAVE_OFF" ), 0 );
    m_pAutosaveSelect->AddItem( g_StringTableMgr( "ui", "IDS_PROFILE_AUTOSAVE_ON" ),  1 );

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
  
    m_pNavText->SetLabel( navText );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);
    
    // initialize screen scaling
    InitScreenScaling( Position );

    // set the frame to be disabled (if coming from off screen)
    if (g_UiMgr->IsScreenOn() == FALSE)
        SetFlag( WF_DISABLED, TRUE );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // set the type
    m_Type = PROFILE_OPTIONS_NORMAL;

    // clear create flag
    m_bCreate = FALSE;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_profile_options::Configure( profile_dlg_types Type, xbool bCreate )
{
    // set create flag
    m_bCreate = bCreate;

    if( m_bCreate )
    {
        // change title bar
        SetLabel( g_StringTableMgr( "ui", "IDS_PROFILE_OPTIONS_TITLE_CREATE" ) );
    }

    // set type
    m_Type = Type;

    // reconfigure for pause state
    if( Type != PROFILE_OPTIONS_NORMAL )
    {
        // can't change avatar during pause
        m_pButtonAvatar ->SetFlag(ui_win::WF_DISABLED, TRUE);
    }
}

//=========================================================================

void dlg_profile_options::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_profile_options::Render( s32 ox, s32 oy )
{
    static s32 offset   =  0;
    static s32 gap      =  9;
    static s32 width    =  4;

    irect rb;
    

    // render background filter
    if( m_Type != PROFILE_OPTIONS_NORMAL )
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

void dlg_profile_options::OnSaveProfileCB( void )
{
#ifdef TARGET_PS2
    MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( m_iCard );
#else
    MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( 0 );
#endif
    // if the save was successful (OR user wants to continue without saving)
    if( Condition.SuccessCode )
    {
        // continue without saving?
        if( Condition.bCancelled )
        {
            // flag the profile as not saved
            g_StateMgr.SetProfileNotSaved( g_StateMgr.GetPendingProfileIndex(), TRUE ); 
        }

        // update the changes in the profile
        g_StateMgr.ActivatePendingProfile();

        // continue onward
        g_AudioMgr.Play( "Select_Norm" );
        m_State = DIALOG_STATE_ACTIVATE;            
    }
    else
    {
        // save failed!
        g_AudioMgr.Play( "Select_Norm" );
        m_State = DIALOG_STATE_MEMCARD_ERROR;
    }

    // get the profile list
    xarray<profile_info*>& ProfileNames = g_StateMgr.GetProfileList();
    // get the current list from the memcard manager
    g_UIMemCardMgr.GetProfileNames( ProfileNames );
}

//=========================================================================

void dlg_profile_options::OnPadSelect( ui_win* pWin )
{
    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( pWin == (ui_win*)m_pButtonControls )
        {
            g_AudioMgr.Play("Select_Norm");
            m_CurrentControl =  IDC_PROFILE_OPTIONS_CONTROLS;
            m_State = DIALOG_STATE_SELECT;
        }
        else if( pWin == (ui_win*)m_pButtonAvatar )
        {
            g_AudioMgr.Play("Select_Norm");
            m_CurrentControl = IDC_PROFILE_OPTIONS_AVATAR;
            m_State = DIALOG_STATE_SELECT;
        }
        else if( pWin == (ui_win*)m_pButtonDifficulty )
        {
            // disable other options
            m_pButtonControls       ->SetFlag(ui_win::WF_DISABLED, TRUE);
            m_pButtonAvatar         ->SetFlag(ui_win::WF_DISABLED, TRUE);
            m_pButtonDifficulty     ->SetFlag(ui_win::WF_DISABLED, TRUE);
            m_pButtonOnlineStatus   ->SetFlag(ui_win::WF_DISABLED, TRUE);
            m_pButtonAutosave       ->SetFlag(ui_win::WF_DISABLED, TRUE);
            m_pButtonCreate         ->SetFlag(ui_win::WF_DISABLED, TRUE);

            // hide some buttons
            m_pButtonDifficulty     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pButtonOnlineStatus   ->SetFlag(ui_win::WF_VISIBLE, FALSE);

            // display combo box
            m_pDifficultyBBox       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pDifficultySelect     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pDifficultySelect     ->SetFlag(ui_win::WF_DISABLED, FALSE);

            // update nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_ACCEPT" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_CANCEL" );
            m_pNavText->SetLabel( navText );

            player_profile& Profile = g_StateMgr.GetPendingProfile();
            m_pDifficultySelect->SetSelection( Profile.GetDifficultyLevel() );

            // set highlight
            g_UiMgr->SetScreenHighlight( m_pDifficultyBBox->GetPosition() );

            GotoControl( (ui_control*)m_pDifficultySelect );
        }
        else if( pWin == (ui_win*)m_pDifficultySelect )
        {
            // set difficulty level
            player_profile& Profile = g_StateMgr.GetPendingProfile();

            // did the setting change?
            if( Profile.GetDifficultyLevel() != m_pDifficultySelect->GetSelectedItemData() )
            {
                // if we've started a campaign, then flag that the difficulty level changed
                level_check_points& Checkpoint = Profile.GetCheckpoint(0);
                if( Checkpoint.MapID != -1 )
                {
                    // we've got a checkpoint, so we must have started a campaign
                    Profile.m_bDifficultyChanged = TRUE;
                }

                Profile.SetDifficultyLevel( m_pDifficultySelect->GetSelectedItemData() );
            }

            // hide combo box
            m_pDifficultyBBox       ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pDifficultySelect     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pDifficultySelect     ->SetFlag(ui_win::WF_DISABLED, TRUE);

            // show buttons
            m_pButtonDifficulty     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonOnlineStatus   ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            // enable other options
            m_pButtonControls       ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonDifficulty     ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonOnlineStatus   ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonAutosave       ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonCreate         ->SetFlag(ui_win::WF_DISABLED, FALSE);    

            // update nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
            m_pNavText->SetLabel( navText );

            if( m_Type != PROFILE_OPTIONS_PAUSE )
                m_pButtonAvatar     ->SetFlag(ui_win::WF_DISABLED, FALSE);


            // set highlight
            g_UiMgr->SetScreenHighlight( m_pButtonDifficulty->GetPosition() );

            GotoControl( (ui_control*)m_pButtonDifficulty );
        }
        else if( pWin == (ui_win*)m_pButtonOnlineStatus )
        {
            // disable other options
            m_pButtonControls       ->SetFlag(ui_win::WF_DISABLED, TRUE);
            m_pButtonAvatar         ->SetFlag(ui_win::WF_DISABLED, TRUE);
            m_pButtonDifficulty     ->SetFlag(ui_win::WF_DISABLED, TRUE);
            m_pButtonOnlineStatus   ->SetFlag(ui_win::WF_DISABLED, TRUE);
            m_pButtonAutosave       ->SetFlag(ui_win::WF_DISABLED, TRUE);
            m_pButtonCreate         ->SetFlag(ui_win::WF_DISABLED, TRUE);

            // hide status button
            m_pButtonOnlineStatus   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pButtonAutosave       ->SetFlag(ui_win::WF_VISIBLE, FALSE);

            // display combo box
            m_pOnlineStatusBBox     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pOnlineStatusSelect   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pOnlineStatusSelect   ->SetFlag(ui_win::WF_DISABLED, FALSE);

            // update nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_ACCEPT" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_CANCEL" );
            m_pNavText->SetLabel( navText );

            player_profile& Profile = g_StateMgr.GetPendingProfile();
            m_pOnlineStatusSelect->SetSelection( Profile.m_bIsVisibleOnline );

            // set highlight
            g_UiMgr->SetScreenHighlight( m_pOnlineStatusBBox->GetPosition() );

            GotoControl( (ui_control*)m_pOnlineStatusSelect );
        }
        else if( pWin == (ui_win*)m_pOnlineStatusSelect )
        {
            // set online status
            player_profile& Profile = g_StateMgr.GetPendingProfile();
            Profile.m_bIsVisibleOnline = m_pOnlineStatusSelect->GetSelectedItemData();

            // hide combo box
            m_pOnlineStatusBBox     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pOnlineStatusSelect   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pOnlineStatusSelect   ->SetFlag(ui_win::WF_DISABLED, TRUE);

            // show difficulty button
            m_pButtonOnlineStatus   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonAutosave       ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            // enable other options
            m_pButtonControls       ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonDifficulty     ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonOnlineStatus   ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonAutosave       ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonCreate         ->SetFlag(ui_win::WF_DISABLED, FALSE);    

            // update nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
            m_pNavText->SetLabel( navText );

            if( m_Type != PROFILE_OPTIONS_PAUSE )
                m_pButtonAvatar     ->SetFlag(ui_win::WF_DISABLED, FALSE);

            // set highlight
            g_UiMgr->SetScreenHighlight( m_pButtonOnlineStatus->GetPosition() );

            GotoControl( (ui_control*)m_pButtonOnlineStatus );
        }
        else if( pWin == (ui_win*)m_pButtonAutosave )
        {
            // disable other options
            m_pButtonControls       ->SetFlag(ui_win::WF_DISABLED, TRUE);
            m_pButtonAvatar         ->SetFlag(ui_win::WF_DISABLED, TRUE);
            m_pButtonDifficulty     ->SetFlag(ui_win::WF_DISABLED, TRUE);
            m_pButtonOnlineStatus   ->SetFlag(ui_win::WF_DISABLED, TRUE);
            m_pButtonAutosave       ->SetFlag(ui_win::WF_DISABLED, TRUE);
            m_pButtonCreate         ->SetFlag(ui_win::WF_DISABLED, TRUE);

            // hide autosave button
            m_pButtonAutosave       ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pButtonCreate         ->SetFlag(ui_win::WF_VISIBLE, FALSE);

            // display combo box
            m_pAutosaveBBox         ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pAutosaveSelect       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pAutosaveSelect       ->SetFlag(ui_win::WF_DISABLED, FALSE);

            // update nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_ACCEPT" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_CANCEL" );
            m_pNavText->SetLabel( navText );

            player_profile& Profile = g_StateMgr.GetPendingProfile();
            m_pAutosaveSelect->SetSelection( Profile.m_bAutosaveOn );

            // set highlight
            g_UiMgr->SetScreenHighlight( m_pAutosaveBBox->GetPosition() );

            GotoControl( (ui_control*)m_pAutosaveSelect );
        }
        else if( pWin == (ui_win*)m_pAutosaveSelect )
        {
            // set autosave status
            player_profile& Profile = g_StateMgr.GetPendingProfile();
            Profile.m_bAutosaveOn = m_pAutosaveSelect->GetSelectedItemData();

            // hide combo box
            m_pAutosaveBBox         ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pAutosaveSelect       ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pAutosaveSelect       ->SetFlag(ui_win::WF_DISABLED, TRUE);

            // show hidden buttons
            m_pButtonAutosave       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonCreate         ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            // enable other options
            m_pButtonControls       ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonDifficulty     ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonOnlineStatus   ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonAutosave       ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonCreate         ->SetFlag(ui_win::WF_DISABLED, FALSE);    

            // update nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
            m_pNavText->SetLabel( navText );

            if( m_Type != PROFILE_OPTIONS_PAUSE )
                m_pButtonAvatar     ->SetFlag(ui_win::WF_DISABLED, FALSE);

            // set highlight
            g_UiMgr->SetScreenHighlight( m_pButtonAutosave->GetPosition() );

            GotoControl( (ui_control*)m_pButtonAutosave );
        }
        else if( pWin == (ui_win*)m_pButtonCreate )
        {
            // are we creating this profile?
            if( m_bCreate )
            {
                g_AudioMgr.Play("Select_Norm");
                m_CurrentControl = IDC_PROFILE_OPTIONS_ACCEPT;

                // this is a NEW profile that we just created

                // calculate the hash string for the new profile
                player_profile& NewProfile = g_StateMgr.GetPendingProfile();
                NewProfile.SetHash();

                // select this new profile
                g_StateMgr.SetSelectedProfile( g_StateMgr.GetPendingProfileIndex(), NewProfile.GetHash() );

                // attempt to save the profile to the default device (HDD for Xbox, appropriate location for PC)
                // MemCardMgr should handle the platform difference. Card index 0 is used.
                g_UIMemCardMgr.CreateProfile( 0, g_StateMgr.GetPendingProfileIndex(), this, &dlg_profile_options::OnProfileCreateCB );

                // change the dialog state to wait for the memcard operation
                m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
            }
            else // Editing existing profile
            {
                // check for any changes made to the profile
                player_profile PendingProfile = g_StateMgr.GetPendingProfile();

                if( PendingProfile.HasChanged() )
                {
#ifdef TARGET_XBOX
                    if( GameMgr.GameInProgress() && g_NetworkMgr.IsOnline() )
                    {
                        // don't save to memcard whilst an online game is in progress

                        // update the changes in the profile
                        g_StateMgr.ActivatePendingProfile( TRUE ); // Mark Dirty

                        // return to pause menu
                        m_State = DIALOG_STATE_ACTIVATE;
                    }
                    else
#endif
                    {
                        // changes have been made - prompt to save
                        irect r = g_UiMgr->GetUserBounds( g_UiUserID );
                        m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
                        m_PopUpType = OPTIONS_POPUP_PROFILE_HAS_CHANGED;

                        // set nav text
                        xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
                        navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
                        m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);
                        m_State = DIALOG_STATE_POPUP;

                        // configure message
                        m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_PROFILE_EDIT" ),
                            TRUE,
                            TRUE,
                            FALSE,
                            g_StringTableMgr( "ui", "IDS_PROFILE_EDIT_MSG" ),
                            navText,
                            &m_PopUpResult );

                        return; // Wait for popup response
                    }
                }
                else
                {
                    // no changes
                    g_AudioMgr.Play("Select_Norm");
                    m_State = DIALOG_STATE_BACK;
                }
            }
        }
    }
}


//=========================================================================

#ifdef TARGET_XBOX
void dlg_profile_options::OnProfileCreateCB( void )
{
    MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( 0 );

    // If the save was successful OR user continues WITHOUT saving
    if( Condition.SuccessCode )
    {
        // continue without saving?
        if( !Condition.bCancelled )
        {
            // saved ok
            g_StateMgr.SetProfileNotSaved( g_StateMgr.GetPendingProfileIndex(), FALSE );
        }

        // update the changes in the profile
        g_StateMgr.ActivatePendingProfile();
        g_AudioMgr.Play( "Select_Norm" );

        // continue to campaign menu
        m_State = DIALOG_STATE_SELECT;
    }
    else
    {
        // save unsuccessful - return to profile select screen
        g_AudioMgr.Play( "Backup" );
        m_State = DIALOG_STATE_BACK;  
    }
}
#else
void dlg_profile_options::OnProfileCreateCB( void )
{
    MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( 0 );

    // If the save was successful OR user continues WITHOUT saving
    if( Condition.SuccessCode )
    {
        // continue without saving?
        if( !Condition.bCancelled )
        {
            // saved ok
            g_StateMgr.SetProfileNotSaved( g_StateMgr.GetPendingProfileIndex(), FALSE );
        }
        else
        {
			// save unsuccessful - return to profile select screen
            g_AudioMgr.Play( "Backup" );
            m_State = DIALOG_STATE_BACK;
            return;
        }

        // update the changes in the profile
        g_StateMgr.ActivatePendingProfile();
        g_AudioMgr.Play( "Select_Norm" );

        // continue to campaign menu or next logical step
        m_State = DIALOG_STATE_SELECT; 
    }
    else
    {
        // save unsuccessful - return to profile select screen or previous menu
        g_AudioMgr.Play( "Backup" );
        m_State = DIALOG_STATE_BACK;
    }
}
#endif

//=========================================================================

void dlg_profile_options::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( pWin == (ui_win*)m_pDifficultySelect )
        {
            // hide combo box
            m_pDifficultyBBox       ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pDifficultySelect     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pDifficultySelect     ->SetFlag(ui_win::WF_DISABLED, TRUE);

            // show difficulty button
            m_pButtonDifficulty     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonOnlineStatus   ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            // enable other options
            m_pButtonControls       ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonDifficulty     ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonOnlineStatus   ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonAutosave       ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonCreate         ->SetFlag(ui_win::WF_DISABLED, FALSE);    

            // update nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
            m_pNavText->SetLabel( navText );

            if( m_Type != PROFILE_OPTIONS_PAUSE )
                m_pButtonAvatar     ->SetFlag(ui_win::WF_DISABLED, FALSE);


            // set highlight
            g_UiMgr->SetScreenHighlight( m_pButtonDifficulty->GetPosition() );

            GotoControl( (ui_control*)m_pButtonDifficulty );

            return;
        }
        else if( pWin == (ui_win*)m_pOnlineStatusSelect )
        {
            // hide combo box
            m_pOnlineStatusBBox     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pOnlineStatusSelect   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pOnlineStatusSelect   ->SetFlag(ui_win::WF_DISABLED, TRUE);

            // show difficulty button
            m_pButtonOnlineStatus   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonAutosave       ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            // enable other options
            m_pButtonControls       ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonDifficulty     ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonOnlineStatus   ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonAutosave       ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonCreate         ->SetFlag(ui_win::WF_DISABLED, FALSE);    

            // update nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
            m_pNavText->SetLabel( navText );

            if( m_Type != PROFILE_OPTIONS_PAUSE )
                m_pButtonAvatar     ->SetFlag(ui_win::WF_DISABLED, FALSE);


            // set highlight
            g_UiMgr->SetScreenHighlight( m_pButtonOnlineStatus->GetPosition() );

            GotoControl( (ui_control*)m_pButtonOnlineStatus );

            return;
        }
        else if( pWin == (ui_win*)m_pAutosaveSelect )
        {
            // hide combo box
            m_pAutosaveBBox         ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pAutosaveSelect       ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pAutosaveSelect       ->SetFlag(ui_win::WF_DISABLED, TRUE);

            // show hidden buttons
            m_pButtonAutosave       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonCreate         ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            // enable other options
            m_pButtonControls       ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonDifficulty     ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonOnlineStatus   ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonAutosave       ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pButtonCreate         ->SetFlag(ui_win::WF_DISABLED, FALSE);    

            // update nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
            m_pNavText->SetLabel( navText );

            if( m_Type != PROFILE_OPTIONS_PAUSE )
                m_pButtonAvatar     ->SetFlag(ui_win::WF_DISABLED, FALSE);

            // set highlight
            g_UiMgr->SetScreenHighlight( m_pButtonAutosave->GetPosition() );

            GotoControl( (ui_control*)m_pButtonAutosave );

            return;
        }

        // are we creating a new profile or editing an existing one?
        if( m_bCreate )
        {
            // confirm abandon create profile
            irect r = g_UiMgr->GetUserBounds( g_UiUserID );
            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
            m_PopUpType = OPTIONS_POPUP_CANCEL_CREATE;
            m_State = DIALOG_STATE_POPUP;

            // set nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

            // configure message
            m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_PROFILE_CREATE" ), 
                TRUE, 
                TRUE, 
                FALSE, 
                g_StringTableMgr( "ui", "IDS_PROFILE_CANCEL_CREATE_MSG" ),
                navText,
                &m_PopUpResult );

            return;
        }

        // cancel changes - exit to previous screen
        g_AudioMgr.Play("Backup");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_profile_options::OnUpdate ( ui_win* pWin, f32 DeltaTime )
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
            m_pButtonControls       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonAvatar         ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonDifficulty     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonOnlineStatus   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonAutosave       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonCreate         ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText              ->SetFlag(ui_win::WF_VISIBLE, TRUE);

#ifdef TARGET_XBOX
            {
                if( g_MatchMgr.GetAuthStatus() == AUTH_STAT_CONNECTED )
                {
                    m_pButtonOnlineStatus->SetFlag( ui_win::WF_DISABLED, FALSE );
                }
                else
                {
                    m_pButtonOnlineStatus->SetFlag( ui_win::WF_DISABLED, TRUE  );
                }
            }
#endif

            s32 iControl = g_StateMgr.GetCurrentControl();

            if( m_bCreate )
            {
                if( (iControl == -1) || (GotoControl(iControl)==NULL) )
                {
                    GotoControl( (ui_control*)m_pButtonCreate );
                    m_pButtonCreate->SetFlag(WF_HIGHLIGHT, TRUE);
                    g_UiMgr->SetScreenHighlight( m_pButtonCreate->GetPosition() );
                }
                else
                {
                    ui_control* pControl = GotoControl( iControl );
                    pControl->SetFlag(WF_HIGHLIGHT, TRUE);
                    g_UiMgr->SetScreenHighlight(pControl->GetPosition() );
                    m_CurrentControl = iControl;
                }
            }
            else
            {
                if( (iControl == -1) || (GotoControl(iControl)==NULL) )
                {
                    GotoControl( (ui_control*)m_pButtonControls );
                    m_pButtonControls->SetFlag(WF_HIGHLIGHT, TRUE);
                    g_UiMgr->SetScreenHighlight( m_pButtonControls->GetPosition() );
                }
                else
                {
                    // range check control ID
                    if( iControl > 4 ) // Assuming max control index is 4 for edit mode (Controls, Avatar, Difficulty, Online Status, Autosave)
                    {
                        iControl = 0;
                    }

                    ui_control* pControl = GotoControl( iControl );
                    ASSERT( pControl );
                    pControl->SetFlag(WF_HIGHLIGHT, TRUE);
                    g_UiMgr->SetScreenHighlight(pControl->GetPosition() );
                    m_CurrentControl = iControl;
                }
            }

            if (g_UiMgr->IsScreenOn() == FALSE)
            {
                // enable the frame
                SetFlag( WF_DISABLED, FALSE );
                g_UiMgr->SetScreenOn(TRUE);
            }
        }
    }

    // check for result of popup box
    if ( m_PopUp )
    {
        if ( m_PopUpResult != DLG_POPUP_IDLE )
        {
            switch( m_PopUpType )
            {
                case OPTIONS_POPUP_CANCEL_CREATE:
                {
                    // abandon create?
                    if ( m_PopUpResult == DLG_POPUP_YES )
                    {
                        // abandon changes and return to profile select screen
                        g_AudioMgr.Play("Backup");
                        m_State = DIALOG_STATE_BACK;
                    }
                    else
                    {
                        // re-enable dialog
                        m_State = DIALOG_STATE_ACTIVE;
                    }
                }
                break;

#ifdef TARGET_XBOX
                case OPTIONS_POPUP_XBOX_FREE_BLOCKS:
                {
                    if( m_PopUpResult == DLG_POPUP_YES )
                    {
                        // continue without saving
                        // update the changes in the profile
                        g_StateMgr.ActivatePendingProfile();

                        // continue onward
                        g_AudioMgr.Play( "Select_Norm" );
                        m_State = DIALOG_STATE_ACTIVATE;
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
                        // Specify the number of 16-KB blocks that need to be freed
                        LaunchDash.dwParameter2 = ( g_StateMgr.GetProfileSaveSize() + 16383 ) / 16384;
                        // Launch the Xbox Dashboard
                        XLaunchNewImage( NULL, (PLAUNCH_DATA)(&LaunchDash) );
                    }
                }
                break;
#endif

                case OPTIONS_POPUP_PROFILE_HAS_CHANGED:
                {
                    // save changes?
                    if ( m_PopUpResult == DLG_POPUP_YES )
                    {
                        // check if this profile exists on disk already
                        if( g_StateMgr.GetProfileNotSaved( g_StateMgr.GetPendingProfileIndex() ) )
                        {
                            // Profile exists in memory but not saved to disk yet.
                            // Attempt to create/save it for the first time.
                            // MemCardMgr should handle platform specifics (Xbox HDD / PC location) using card index 0.
                            g_UIMemCardMgr.CreateProfile( 0, g_StateMgr.GetPendingProfileIndex(), this, &dlg_profile_options::OnProfileCreateCB );
                            m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
                        }
                        else
                        {
                            // Profile already exists on disk. Save the changes.
                            g_AudioMgr.Play("Select_Norm");

                            m_CurrentControl = IDC_PROFILE_OPTIONS_ACCEPT;

                            // attempt to save the changes to the memcard/device
                            profile_info* pProfileInfo = &g_UIMemCardMgr.GetProfileInfo( g_StateMgr.GetPendingProfileIndex() );
                            m_iCard = pProfileInfo->CardID; // Get the correct card ID where the profile resides
                            g_UIMemCardMgr.SaveProfile( *pProfileInfo, g_StateMgr.GetPendingProfileIndex(), this, &dlg_profile_options::OnSaveProfileCB );

                            m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
                        }
                    }
                    else // User chose "No" to saving changes
                    {
                        // abandon changes
                        g_AudioMgr.Play("Backup");
                        m_State = DIALOG_STATE_BACK;
                    }
                }
                break;
            }
            // clear popup
            m_PopUp = NULL;

            // turn on nav text
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
    }


    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    if( m_pButtonControls->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 0;
        g_UiMgr->SetScreenHighlight( m_pButtonControls->GetPosition() );
    }
    else if( m_pButtonAvatar->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 1;
        g_UiMgr->SetScreenHighlight( m_pButtonAvatar->GetPosition() );
    }
    else if( m_pButtonDifficulty->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 2;
        g_UiMgr->SetScreenHighlight( m_pButtonDifficulty->GetPosition() );
    }
    else if( m_pButtonOnlineStatus->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 3;
        g_UiMgr->SetScreenHighlight( m_pButtonOnlineStatus->GetPosition() );
    }
    else if( m_pButtonAutosave->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 4;
        g_UiMgr->SetScreenHighlight( m_pButtonAutosave->GetPosition() );
    }
    else if( m_pButtonCreate->GetFlags(WF_HIGHLIGHT) )
    {
        // Assuming 'Create/Accept' button corresponds to highlight index 6
        // (Indices seem to skip based on control layout/disabling)
        highLight = 6;
        g_UiMgr->SetScreenHighlight( m_pButtonCreate->GetPosition() );
    }

    if( highLight != m_CurrHL )
    {
        if( highLight != -1 )
            g_AudioMgr.Play("Cusor_Norm");

        m_CurrHL = highLight;
    }
}

//=========================================================================
