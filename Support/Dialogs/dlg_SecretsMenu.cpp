//=========================================================================
//
//  dlg_secrets_menu.cpp
//
//=========================================================================

#include "Entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_blankbox.hpp"
#include "ui\ui_textbox.hpp"

#include "dlg_SecretsMenu.hpp"
#include "StateMgr\StateMgr.hpp"
#include "StringMgr\StringMgr.hpp"
#include "StateMgr/mapList.hpp"
#include "StateMgr/SecretList.hpp"
#include "MoviePlayer/MoviePlayer.hpp"

#include "StateMgr/StateMgr.hpp"

#if defined(TARGET_PS2)
#include "Entropy\PS2\ps2_misc.hpp"
#endif

extern xstring SelectBestClip( const char* pName );

//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum controls
{   
    IDC_SECRETS_MAIN,
    IDC_SECRETS_DETAILS,
    IDC_SECRETS_SELECT,
    IDC_SECRETS_BUTTON_1,
    IDC_SECRETS_BUTTON_2,
    IDC_SECRETS_BUTTON_3,
    IDC_SECRETS_BUTTON_4,
    IDC_SECRETS_BUTTON_5,
    IDC_SECRETS_TEXT_1,
    IDC_SECRETS_TEXT_2,
    IDC_SECRETS_TEXT_3,
    IDC_SECRETS_BLACKOUT,
    IDC_SECRETS_TEXTBOX,
    IDC_NAV_TEXT,
};

ui_manager::control_tem SecretsMenuControls[] = 
{
    // Frames.

    { IDC_SECRETS_SELECT,      "IDS_NULL",    "combo",      138,  40, 220,  40, 0, 0, 5, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_SECRETS_MAIN,        "IDS_NULL",    "blankbox",    40,  80, 416, 144, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_SECRETS_BUTTON_1,    "IDS_NULL",    "button",      56, 130,  64,  64, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_SECRETS_BUTTON_2,    "IDS_NULL",    "button",     136, 130,  64,  64, 1, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_SECRETS_BUTTON_3,    "IDS_NULL",    "button",     216, 130,  64,  64, 2, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_SECRETS_BUTTON_4,    "IDS_NULL",    "button",     296, 130,  64,  64, 3, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_SECRETS_BUTTON_5,    "IDS_NULL",    "button",     376, 130,  64,  64, 4, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_SECRETS_DETAILS,     "IDS_NULL",    "blankbox",    40, 240, 416,  94, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_SECRETS_TEXT_1,      "IDS_NULL",    "text",        48, 262, 400,  94, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_SECRETS_TEXT_2,      "IDS_NULL",    "text",        48, 278,  90,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_SECRETS_TEXT_3,      "IDS_NULL",    "text",        48, 294,  90,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

#ifdef TARGET_PS2
    { IDC_SECRETS_BLACKOUT,    "IDS_NULL",    "blankbox",    -9, -24, 513, 448, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_USE_ABSOLUTE },
#else
    { IDC_SECRETS_BLACKOUT,    "IDS_NULL",    "blankbox",  -113, -40, 722, 480, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_USE_ABSOLUTE },
#endif

    { IDC_SECRETS_TEXTBOX,     "IDS_NULL",    "textbox",     60, 240, 376,  93, 0, 2, 5, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_NAV_TEXT,         "IDS_NULL",    "text",         0,   0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem SecretsMenuDialog =
{
    "IDS_SECRETS_MENU_TITLE",
    5, 9,
    sizeof(SecretsMenuControls)/sizeof(ui_manager::control_tem),
    &SecretsMenuControls[0],
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

static xbool s_Scaled = FALSE;

//=========================================================================
//  Registration function
//=========================================================================

void dlg_secrets_menu_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "secrets", &SecretsMenuDialog, &dlg_secrets_menu_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_secrets_menu_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_secrets_menu* pDialog = new dlg_secrets_menu;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_secrets_menu
//=========================================================================

dlg_secrets_menu::dlg_secrets_menu( void )
{
}

//=========================================================================

dlg_secrets_menu::~dlg_secrets_menu( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_secrets_menu::Create( s32                        UserID,
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

    m_CurrHL = 0;
    s_Scaled = FALSE;

    // load bitmaps
    m_SecretsIconID[SECRET_TYPE_VIDEO]   = g_UiMgr->LoadBitmap( "SecretsVideo",  "UI_LoreVideo.xbmp"   );  // TODO: get some new bitmaps  
    m_SecretsIconID[SECRET_TYPE_CHEAT]   = g_UiMgr->LoadBitmap( "SecretsCheat",  "UI_LoreAudio.xbmp"   );  // for the secrets icons
    m_SecretsIconID[SECRET_TYPE_UNKNOWN] = g_UiMgr->LoadBitmap( "SecretsNull",   "UI_LoreUnknown.xbmp" );

    // set up nav text 
    m_pNavText = (ui_text*) FindChildByID( IDC_NAV_TEXT );
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_BACK" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_CYCLE_VAULT" );
    m_pNavText->SetLabel( navText );
    m_pNavText->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);    

    // setup secrets main box
    m_pSecretsMain = (ui_blankbox*)FindChildByID( IDC_SECRETS_MAIN );
    m_pSecretsMain->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pSecretsMain->SetFlag(ui_win::WF_STATIC, TRUE);
    m_pSecretsMain->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pSecretsMain->SetHasTitleBar( TRUE );
    m_pSecretsMain->SetLabel( g_StringTableMgr( "ui", "IDS_SECRETS_VAULT" ) );
    m_pSecretsMain->SetLabelColor( xcolor(255,252,204,255) );
    m_pSecretsMain->SetTitleBarColor( xcolor(19,59,14,196) );

    // setup secrets details box
    m_pSecretsDetails = (ui_blankbox*)FindChildByID( IDC_SECRETS_DETAILS );
    m_pSecretsDetails->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pSecretsDetails->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pSecretsDetails->SetFlag(ui_win::WF_STATIC, TRUE);
    m_pSecretsDetails->SetHasTitleBar( TRUE );
    m_pSecretsDetails->SetLabel( g_StringTableMgr( "ui", "IDS_SECRETS_DETAILS" ) );
    m_pSecretsDetails->SetLabelColor( xcolor(255,252,204,255) );
    m_pSecretsDetails->SetTitleBarColor( xcolor(19,59,14,196) );

    // setup blackout box
    m_pBlackOut = (ui_blankbox*)FindChildByID( IDC_SECRETS_BLACKOUT );
    m_pBlackOut->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pBlackOut->SetBackgroundColor( xcolor (0,0,0,0) );
    m_pBlackOut->SetFlag(ui_win::WF_STATIC, TRUE);

    // set up textbox
    m_pTextBox = (ui_textbox*)FindChildByID( IDC_SECRETS_TEXTBOX );
    
    m_pTextBox->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pTextBox->SetFlag( ui_win::WF_DISABLED, TRUE );
    m_pTextBox->SetExitOnBack( TRUE );
    m_pTextBox->SetExitOnSelect( TRUE );
    m_pTextBox->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pTextBox->DisableFrame();
    m_pTextBox->SetLabelFlags( ui_font::h_left|ui_font::v_top );
    
    // set up secrets combo
    m_pSecretsSelect = (ui_combo*)FindChildByID( IDC_SECRETS_SELECT );
    m_pSecretsSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT | ui_combo::CB_NOTIFY_PARENT );
    m_pSecretsSelect->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pSecretsSelect->AddItem( g_StringTableMgr( "ui", "IDS_SECRETS_VAULT_1" ), 0 );
    m_pSecretsSelect->AddItem( g_StringTableMgr( "ui", "IDS_SECRETS_VAULT_2" ), 1 );
    m_pSecretsSelect->AddItem( g_StringTableMgr( "ui", "IDS_SECRETS_VAULT_3" ), 2 );
    m_pSecretsSelect->AddItem( g_StringTableMgr( "ui", "IDS_SECRETS_VAULT_4" ), 3 );
    m_pSecretsSelect->AddItem( g_StringTableMgr( "ui", "IDS_SECRETS_VAULT_5" ), 4 );

    // set up buttons
    m_pSecretsButton[0]  = (ui_button*) FindChildByID( IDC_SECRETS_BUTTON_1 );
    m_pSecretsButton[1]  = (ui_button*) FindChildByID( IDC_SECRETS_BUTTON_2 );
    m_pSecretsButton[2]  = (ui_button*) FindChildByID( IDC_SECRETS_BUTTON_3 );
    m_pSecretsButton[3]  = (ui_button*) FindChildByID( IDC_SECRETS_BUTTON_4 );
    m_pSecretsButton[4]  = (ui_button*) FindChildByID( IDC_SECRETS_BUTTON_5 );

    m_pSecretsButton[0]  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pSecretsButton[1]  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pSecretsButton[2]  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pSecretsButton[3]  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pSecretsButton[4]  ->SetFlag(ui_win::WF_VISIBLE, FALSE);


    m_pSecretsLine1    = (ui_text*)FindChildByID( IDC_SECRETS_TEXT_1 );
    m_pSecretsLine2    = (ui_text*)FindChildByID( IDC_SECRETS_TEXT_2 );
    m_pSecretsLine3    = (ui_text*)FindChildByID( IDC_SECRETS_TEXT_3 );
    
    m_pSecretsLine1    ->UseSmallText( TRUE );
    m_pSecretsLine2    ->UseSmallText( TRUE );
    m_pSecretsLine3    ->UseSmallText( TRUE );

    m_pSecretsLine1    ->SetLabelFlags( ui_font::h_left|ui_font::v_top );
    m_pSecretsLine2    ->SetLabelFlags( ui_font::h_left|ui_font::v_top );
    m_pSecretsLine3    ->SetLabelFlags( ui_font::h_left|ui_font::v_top );

    m_pSecretsLine1    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pSecretsLine2    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pSecretsLine3    ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    m_pSecretsLine1    ->SetFlag(ui_win::WF_STATIC, TRUE);
    m_pSecretsLine2    ->SetFlag(ui_win::WF_STATIC, TRUE);
    m_pSecretsLine3    ->SetFlag(ui_win::WF_STATIC, TRUE);

    m_pSecretsLine1    ->SetLabelColor( xcolor(255,252,204,255) );
    m_pSecretsLine2    ->SetLabelColor( xcolor(255,252,204,255) );
    m_pSecretsLine3    ->SetLabelColor( xcolor(255,252,204,255) );

    m_pSelectedIcon     = m_pSecretsButton[0];

    // set focus
    GotoControl( (ui_control*)m_pSecretsSelect );

    // Initialize popup
    m_PopUp = NULL;

    // Initialize screen scale factors
    s32 XRes, YRes;
    eng_GetRes( XRes, YRes );
    m_ScreenScaleX = (f32)XRes / 512.0f;
    m_ScreenScaleY = (f32)YRes / 448.0f;

    // Initialize icon scaling
    m_scaleCount  = 0.0f;
    m_bScreenIsOn = FALSE;
    m_bScaleDown  = FALSE;
    m_TimeOut     = 0.0f;

    // get the active player profile
    player_profile& Profile = g_StateMgr.GetActiveProfile( 0 );
    // clear the new secret flag
    Profile.ClearNewSecretUnlocked();
    // checksum profile to prevent unwanted "changed" messages appearing
    Profile.Checksum();

    // initialize screen scaling
    InitScreenScaling( Position );

    m_pSecretsSelect->SetSelection( 0 );
    PopulateSecretsDetails( TRUE );

#ifdef TARGET_PS2
    // get video mode
    eng_GetPALMode( m_bPalMode );
#endif

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

	// Return success code
    return Success;
}

//=========================================================================

void dlg_secrets_menu::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();

#ifdef TARGET_PS2
    // wait until we finish drawing before we unload the logo bitmap
    DLIST.Flush();
    DLIST.WaitForTasks();
#endif

    // unload secrets bitmaps
    g_UiMgr->UnloadBitmap( "SecretsVideo" );
    g_UiMgr->UnloadBitmap( "SecretsCheat" );
    g_UiMgr->UnloadBitmap( "SecretsNull" );
    g_UiMgr->UnloadBitmap( "Still" );
}

//=========================================================================

void dlg_secrets_menu::Render( s32 ox, s32 oy )
{
    static s32 offset   =  0;
    static s32 gap      =  9;
    static s32 width    =  4;

	irect rb;
    
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

    // render the popup screen (if any)
    if( m_bScreenIsOn )
    {                    
        // render border
        g_UiMgr->RenderRect( m_DrawPos, xcolor(255,252,204,255), FALSE );

        // render movie/button bitmap
        if( m_scaleCount || m_TimeOut)
        {
            irect r = m_DrawPos;
            r.t += 2;
            r.l += 2;
            r.b -= 2;
            r.r -= 2;

            m_pManager->RenderBitmap( m_pSelectedIcon->GetBitmap(), r, xcolor(255,252,204,255) );
        }
        else
        {
            switch( m_CurrentType )
            {
                case SECRET_TYPE_VIDEO:
#if defined( TARGET_PC )
                    //if( Movie.IsPlaying() )
                    //{               
                    //    Movie.Render( vector3( 129.0f, 59.0f, 0.0f ), vector2( 256.0f, 192.0f ), TRUE );
                    //}
#endif
                    //break;

                default:
                    irect r = m_DrawPos;
                    r.t += 2;
                    r.l += 2;
                    r.b -= 2;
                    r.r -= 2;

                    // render bitmap
#ifdef TARGET_PS2
                    if( m_bPalMode )
                    {
                        m_pManager->RenderBitmap( m_StillBitmapID, r, XCOLOR_WHITE ); //PAL
                    }
                    else
                    {
                        vector2 UV0( 0.0f, 0.0625f );
                        vector2 UV1( 1.0f, 0.9375f );
                        m_pManager->RenderBitmapUV( m_StillBitmapID, r, UV0, UV1, XCOLOR_WHITE );
                    }
#elif defined TARGET_XBOX
                    vector2 UV0( 0.0f, 0.0625f );
                    vector2 UV1( 1.0f, 0.9375f );
                    m_pManager->RenderBitmapUV( m_StillBitmapID, r, UV0, UV1, XCOLOR_WHITE );
#else
                    m_pManager->RenderBitmap( m_StillBitmapID, r, XCOLOR_WHITE );
#endif
            }
        }
    }
}


//=========================================================================

void dlg_secrets_menu::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    // only allow navigation if active
    if( m_State == DIALOG_STATE_ACTIVE )
    {
        if( pWin == (ui_win*)m_pSecretsSelect )
        {
            switch( Code )
            {
                case ui_manager::NAV_LEFT:
                case ui_manager::NAV_RIGHT:
                    PopulateSecretsDetails( TRUE );                   
                    return;
            }
        }
        ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
    }
}

//=========================================================================

void dlg_secrets_menu::OnNotify( ui_win* pWin, ui_win* pSender, s32 Command, void* pData )
{
    (void)pWin;
    (void)pSender;
    (void)Command;
    (void)pData;

    if( pSender == (ui_win*)m_pSecretsSelect )
    {
        if( Command == WN_COMBO_SELCHANGE )
        {
            if( !s_Scaled )
            {
                PopulateSecretsDetails( TRUE );
            }
        }
    }
}

//=========================================================================

void dlg_secrets_menu::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( pWin == (ui_win*)m_pSecretsSelect )
        {
            PopulateSecretsDetails( TRUE );
            return;
        }

        // handle secrets item selection here!
        if( pWin == (ui_win*)m_pSecretsButton[0] )
        {
            if( m_pSecretsButton[0]->GetBitmap() == m_SecretsIconID[SECRET_TYPE_UNKNOWN] )
            {
                g_AudioMgr.Play( "InvalidEntry" );
                return;
            }

            g_AudioMgr.Play("Select_Norm");
            m_pSelectedIcon = m_pSecretsButton[0];
            m_SelectedIndex = 0;
            InitIconScaling( FALSE );
            m_State = DIALOG_STATE_ACTIVATE;
        }
        else if( pWin == (ui_win*)m_pSecretsButton[1] )
        {
            if( m_pSecretsButton[1]->GetBitmap() == m_SecretsIconID[SECRET_TYPE_UNKNOWN] )
            {
                g_AudioMgr.Play( "InvalidEntry" );
                return;
            }

            g_AudioMgr.Play("Select_Norm");
            m_pSelectedIcon = m_pSecretsButton[1];
            m_SelectedIndex = 1;
            InitIconScaling( FALSE );
            m_State = DIALOG_STATE_ACTIVATE;
        }
        else if( pWin == (ui_win*)m_pSecretsButton[2] )
        {
            if( m_pSecretsButton[2]->GetBitmap() == m_SecretsIconID[SECRET_TYPE_UNKNOWN] )
            {
                g_AudioMgr.Play( "InvalidEntry" );
                return;
            }

            g_AudioMgr.Play("Select_Norm");
            m_pSelectedIcon = m_pSecretsButton[2];
            m_SelectedIndex = 2;
            InitIconScaling( FALSE );
            m_State = DIALOG_STATE_ACTIVATE;
        }
        else if( pWin == (ui_win*)m_pSecretsButton[3] )
        {
            if( m_pSecretsButton[3]->GetBitmap() == m_SecretsIconID[SECRET_TYPE_UNKNOWN] )
            {
                g_AudioMgr.Play( "InvalidEntry" );
                return;
            }

            g_AudioMgr.Play("Select_Norm");
            m_pSelectedIcon = m_pSecretsButton[3];
            m_SelectedIndex = 3;
            InitIconScaling( FALSE );
            m_State = DIALOG_STATE_ACTIVATE;
        }
        else if( pWin == (ui_win*)m_pSecretsButton[4] )
        {
            if( m_pSecretsButton[4]->GetBitmap() == m_SecretsIconID[SECRET_TYPE_UNKNOWN] )
            {
                g_AudioMgr.Play( "InvalidEntry" );
                return;
            }

            g_AudioMgr.Play("Select_Norm");
            m_pSelectedIcon = m_pSecretsButton[4];
            m_SelectedIndex = 4;
            InitIconScaling( FALSE );
            m_State = DIALOG_STATE_ACTIVATE;
        }
    }
    else if( m_State == DIALOG_STATE_ACTIVATE )
    {
        if( m_scaleCount == 0 )
        {
            const secret_entry* Entry = g_SecretList.GetByIndex( m_pSelectedIcon->GetData() );
            ASSERT( Entry );

            // if this is a movie, then play it
            if( Entry->SecretType == SECRET_TYPE_VIDEO )
            {
                // shut down background movie
                g_StateMgr.DisableBackgoundMovie();
#if defined( TARGET_PC )
            // play the selected movie
            PlaySimpleMovie( SelectBestClip(m_FileName) );
#endif
            // start up the background movie
            g_StateMgr.EnableBackgroundMovie();
        }

            // if this is cheat activate it?
            if( Entry->SecretType == SECRET_TYPE_CHEAT )
            {
                // activate/deactivate cheat here???
            }
        }
    }
}

//=========================================================================

void dlg_secrets_menu::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if( m_scaleCount )
    {
        // currently scaling an icon
        return;
    }

    switch( m_State )
    {
        case DIALOG_STATE_ACTIVE:
        {
            // exit main dialog
            g_AudioMgr.Play("Backup");
            m_State = DIALOG_STATE_BACK;
        }
        break;

        case DIALOG_STATE_ACTIVATE:
        {
            // exit sub menu / movie player
            InitIconScaling( TRUE );            
        }
        break;
    }
    
}

//=========================================================================

void dlg_secrets_menu::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    s32 highLight = -1;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // turn on the controls
            m_pSecretsMain      ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pSecretsDetails   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pSecretsSelect    ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pSecretsButton[0] ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pSecretsButton[1] ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pSecretsButton[2] ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pSecretsButton[3] ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pSecretsButton[4] ->SetFlag( ui_win::WF_VISIBLE, TRUE );

            GotoControl( (ui_control*)m_pSecretsSelect );
            irect Pos = m_pSecretsSelect->GetPosition();
            Pos.Translate( 0, -8 );
            g_UiMgr->SetScreenHighlight( Pos );

            // activate text
            m_pSecretsLine1    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pSecretsLine2    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pSecretsLine3    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText      ->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
    }
    else
    {
        // update any icon scaling
        UpdateIconScaling( DeltaTime );
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    // update everything else
    ui_dialog::OnUpdate( pWin, DeltaTime );

    if( !s_Scaled )
    {
        // update highlight
        if( m_pSecretsSelect->GetFlags(WF_HIGHLIGHT) )
        {
            highLight = 0;
            irect Pos = m_pSecretsSelect->GetPosition();
            Pos.Translate( 0, -8 );
            g_UiMgr->SetScreenHighlight( Pos );
        }
        else if( m_pSecretsButton[0]->GetFlags(WF_HIGHLIGHT) )
        {
            highLight = 1;
            g_UiMgr->SetScreenHighlight( m_pSecretsMain->GetPosition() );
            m_pSelectedIcon = m_pSecretsButton[0];
        }
        else if( m_pSecretsButton[1]->GetFlags(WF_HIGHLIGHT) )
        {
            highLight = 2;
            g_UiMgr->SetScreenHighlight( m_pSecretsMain->GetPosition() );
            m_pSelectedIcon = m_pSecretsButton[1];
        }
        else if( m_pSecretsButton[2]->GetFlags(WF_HIGHLIGHT) )
        {
            highLight = 3;
            g_UiMgr->SetScreenHighlight( m_pSecretsMain->GetPosition() );
            m_pSelectedIcon = m_pSecretsButton[2];
        }
        else if( m_pSecretsButton[3]->GetFlags(WF_HIGHLIGHT) )
        {
            highLight = 4;
            g_UiMgr->SetScreenHighlight( m_pSecretsMain->GetPosition() );
            m_pSelectedIcon = m_pSecretsButton[3];
        }
        else if( m_pSecretsButton[4]->GetFlags(WF_HIGHLIGHT) )
        {
            highLight = 5;
            g_UiMgr->SetScreenHighlight( m_pSecretsMain->GetPosition() );
            m_pSelectedIcon = m_pSecretsButton[4];
        }
    }

    if( highLight != -1 )
    {
        if( highLight != m_CurrHL )
        {
            if( highLight != -1 )
                g_AudioMgr.Play("Cusor_Norm");

            m_CurrHL = highLight;

            if( highLight > 0 )
            {
                PopulateSecretsDetails( FALSE );
            }
            else
            {
                PopulateSecretsDetails( TRUE );
            }
        }
    }
}

//=========================================================================

void dlg_secrets_menu::InitIconScaling ( xbool ScaleDown )
{
    // scaling up or down
    m_bScaleDown  = ScaleDown;

    // store requested frame size
    if( m_bScaleDown )
    {
        m_FadeLevel = 255;
        m_RequestedPos.Set( 0, 0, (s32)(64*m_ScreenScaleX), (s32)(64*m_ScreenScaleY) );
        m_pSelectedIcon->LocalToScreen( m_RequestedPos );
#ifdef TARGET_PS2
        if( m_bPalMode )
        {
            m_StartPos = irect( 159, 64, (159+194), (64+194) ); // PAL
        }
        else
        {
            m_StartPos = irect( 145, 64, (145+222), (64+194) ); // NTSC
        }
#elif defined TARGET_XBOX
        switch( g_PhysW )
        {
        case 1280: 
        case  720:
            m_StartPos = irect( 215, (s32)(64*m_ScreenScaleY), 215+290, (s32)((64*m_ScreenScaleY)+194) );
            break;
        case  640:
            m_StartPos = irect( 191, (s32)(64*m_ScreenScaleY), 191+258, (s32)((64*m_ScreenScaleY)+194) );
            break;
        default:
            ASSERT(0);
            break;
        }
        extern xbool g_b480P;
        if( g_b480P )
        {
            m_RequestedPos.t -= 10;
            m_RequestedPos.b -= 10;
        }
#else
        // 640x480
        m_StartPos = irect( 191, (s32)(64*m_ScreenScaleY), 191+258, (s32)((64*m_ScreenScaleY)+194) );
#endif
        m_DrawPos = m_StartPos;
        m_TimeOut = 0.0f;

        // restart background movie
#if defined( TARGET_PC )
        Movie.Close();
#endif
        g_StateMgr.EnableBackgroundMovie();

        // turn off textbox (if any)
        m_pTextBox->SetFlag( ui_win::WF_VISIBLE, FALSE );
        m_pTextBox->SetFlag( ui_win::WF_DISABLED, TRUE );
        m_pTextBox->SetFlag( ui_win::WF_SELECTED, FALSE );
        
        g_UiMgr->EnableScreenHighlight();
        // turn on secrets details
        m_pSecretsDetails ->SetFlag(ui_win::WF_VISIBLE, TRUE);
        m_pSecretsLine1   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
        m_pSecretsLine2   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
        m_pSecretsLine3   ->SetFlag(ui_win::WF_VISIBLE, TRUE);

        xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
        navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
        m_pNavText->SetLabel( navText );

        // goto previous control
        GotoControl( (ui_control*)m_pSelectedIcon );
        s_Scaled = FALSE;
    }
    else
    {
        s_Scaled = TRUE;

        m_FadeLevel = 0;

#ifdef TARGET_PS2
        if( m_bPalMode )
        {
            m_RequestedPos = irect( 159, 64, (159+194), (64+194) ); // PAL
        }
        else
        {
            m_RequestedPos = irect( 145, 64, (145+222), (64+194) ); // NTSC
        }
#elif defined TARGET_XBOX
        switch( g_PhysW )
        {
        case 1280: 
        case  720:
            m_RequestedPos = irect( 215, (s32)(64*m_ScreenScaleY), 215+290, (s32)((64*m_ScreenScaleY)+194) );
            break;
        case  640:
            m_RequestedPos = irect( 191, (s32)(64*m_ScreenScaleY), 191+258, (s32)((64*m_ScreenScaleY)+194) );
            break;
        default:
            ASSERT(0);
            break;
        }
        extern xbool g_b480P;
        if( g_b480P )
        {
            m_RequestedPos.t -= 10;
            m_RequestedPos.b -= 10;
        }
#else
        // 640x480
        m_RequestedPos = irect( 191, (s32)(64*m_ScreenScaleY), 191+258, (s32)((64*m_ScreenScaleY)+194) );
#endif

        m_StartPos.Set( 0, 0, (s32)(64*m_ScreenScaleX), (s32)(64*m_ScreenScaleY) );
        m_pSelectedIcon->LocalToScreen( m_StartPos );
        m_DrawPos = m_StartPos;
        m_pSelectedIcon->SetFlag( ui_win::WF_VISIBLE, FALSE );
        // disable the highlight
        g_UiMgr->DisableScreenHighlight();
    }

    
    // set up scaling
    m_scaleCount = 0.3f; // time to scale in seconds
    m_scaleAngle = 180.0f / m_scaleCount;

    m_DiffPos.t = (s32)((m_RequestedPos.t - m_DrawPos.t) / 2.0f);
    m_DiffPos.l = (s32)((m_RequestedPos.l - m_DrawPos.l) / 2.0f);
    m_DiffPos.b = (s32)((m_RequestedPos.b - m_DrawPos.b) / 2.0f);
    m_DiffPos.r = (s32)((m_RequestedPos.r - m_DrawPos.r) / 2.0f);

    m_TotalMoved.Set( 0, 0, 0, 0 );
    
    // turn screen on
    m_bScreenIsOn = TRUE;
    m_pBlackOut->SetFlag(ui_win::WF_VISIBLE, TRUE);
    m_pBlackOut->SetBackgroundColor( xcolor (0,0,0,0) );

    // play scaling sound
    if( m_DiffPos.b > 0 )
    {
        g_AudioMgr.Play( "ResizeLarge" ); 
    }
    else
    {
        g_AudioMgr.Play( "ResizeSmall" );
    }
}

//=========================================================================

xbool dlg_secrets_menu::UpdateIconScaling( f32 DeltaTime )
{
    // scale window if necessary
    if (m_scaleCount)
    {
        // apply delta time
        m_scaleCount -= DeltaTime;

        if (m_scaleCount <= 0)
        {
            // last one - make sure window is correct size
            m_scaleCount = 0;
            m_DrawPos = m_RequestedPos;

            if( m_bScaleDown )
            {
                m_FadeLevel = 0;
                m_bScreenIsOn = FALSE;
                m_pBlackOut->SetFlag(ui_win::WF_VISIBLE, FALSE);
                m_State = DIALOG_STATE_ACTIVE;
                m_pSelectedIcon->SetFlag( ui_win::WF_VISIBLE, TRUE );
            }
            else
            {
                m_FadeLevel = 205;
                m_pBlackOut->SetBackgroundColor( xcolor (0,0,0,m_FadeLevel) );

                // get the secrets description
                const secret_entry* Entry = g_SecretList.GetByIndex( m_pSelectedIcon->GetData() );
                ASSERT( Entry );

                // what type of secrets item do we have
                if( Entry->SecretType == SECRET_TYPE_VIDEO )
                {
                    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_PLAY_MOVIE" ));
                    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
                    m_pNavText->SetLabel( navText );
                }

                // set filename for still
                g_UiMgr->UnloadBitmap( "Still" );
                m_NumItems = 1;
                m_CurrItem = 0;
                x_strcpy( m_FileName, Entry->FileName );                            
                m_StillBitmapID = g_UiMgr->LoadBitmap( "Still", xfs( "%s.xbmp", m_FileName ) );

                // turn off secrets details
                m_pSecretsDetails ->SetFlag(ui_win::WF_VISIBLE, FALSE);
                m_pSecretsLine1   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
                m_pSecretsLine2   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
                m_pSecretsLine3   ->SetFlag(ui_win::WF_VISIBLE, FALSE);

                // make text box visible and fill with text
                m_pTextBox->SetFlag( ui_win::WF_VISIBLE, TRUE );
                m_pTextBox->SetFlag( ui_win::WF_DISABLED, FALSE );
                m_pTextBox->SetFlag( ui_win::WF_SELECTED, TRUE );
                x_strcpy( m_FullDesc, Entry->FullDesc );
                m_pTextBox->SetLabel( g_StringTableMgr( "lore", m_FullDesc ) );

                GotoControl( (ui_control*)m_pTextBox );
            }
        }
        else
        {
            m_TotalMoved.t = m_DiffPos.t + (s32)( m_DiffPos.t * x_cos( DEG_TO_RAD( m_scaleAngle * m_scaleCount ) ) );
            m_TotalMoved.l = m_DiffPos.l + (s32)( m_DiffPos.l * x_cos( DEG_TO_RAD( m_scaleAngle * m_scaleCount ) ) );
            m_TotalMoved.r = m_DiffPos.r + (s32)( m_DiffPos.r * x_cos( DEG_TO_RAD( m_scaleAngle * m_scaleCount ) ) );
            m_TotalMoved.b = m_DiffPos.b + (s32)( m_DiffPos.b * x_cos( DEG_TO_RAD( m_scaleAngle * m_scaleCount ) ) );

            m_DrawPos.t = m_StartPos.t + m_TotalMoved.t;
            m_DrawPos.l = m_StartPos.l + m_TotalMoved.l;
            m_DrawPos.r = m_StartPos.r + m_TotalMoved.r;
            m_DrawPos.b = m_StartPos.b + m_TotalMoved.b;

            // update fade level
            if( m_bScaleDown)
            {
                m_FadeLevel -= (u8)( 700.0f * DeltaTime );
            }
            else
            {
                m_FadeLevel += (u8)( 700.0f * DeltaTime );
            }

            m_pBlackOut->SetBackgroundColor( xcolor (0,0,0,m_FadeLevel) );

            // still more to do!
            return TRUE;
        }
    }

    // we're done!
    return FALSE;
}
//=========================================================================

void dlg_secrets_menu::PopulateSecretsDetails( xbool bVaultDetails )
{
    if( bVaultDetails )
    {
        player_profile& Profile = g_StateMgr.GetActiveProfile( 0 );
        s32 VaultIdx = m_pSecretsSelect->GetSelectedItemData();

#if defined (mbillington) || (jhowa) || (ksaffel) || (sbroumley)
        s32 NumUnlocked = 20;
#else
        s32 NumUnlocked = Profile.GetNumSecretsUnlocked() - 1;
#endif
        s32 Min = VaultIdx * 5;
        s32 Max = Min + 5;
        s32 Count = 0;
        s32 j=0;
    
        // check for last vault
        if( VaultIdx == 4 )
        {
            // turn off outer buttons (only three items)
            m_pSecretsButton[0]  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pSecretsButton[1]  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pSecretsButton[3]  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            m_pSecretsButton[4]  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
            
            m_pSecretsButton[0]  ->SetFlag(ui_win::WF_DISABLED, TRUE);
            m_pSecretsButton[1]  ->SetFlag(ui_win::WF_DISABLED, TRUE);
            m_pSecretsButton[3]  ->SetFlag(ui_win::WF_DISABLED, TRUE);
            m_pSecretsButton[4]  ->SetFlag(ui_win::WF_DISABLED, TRUE);

            // limit selection
            j = 2;

            // we must make this bigger than Min or it won't go through the for loop
            // below which will cause page 5's button data to not be set properly.
            Max = Min + 1;  

            m_pSecretsButton[j]->SetBitmap( m_SecretsIconID[SECRET_TYPE_UNKNOWN] );
            m_pSecretsButton[j]->SetData( -1 );
        }
        else
        {
            // enable all five buttons
            if( g_UiMgr->IsScreenScaling() == FALSE )
            {
                m_pSecretsButton[0]  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
                m_pSecretsButton[1]  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
                m_pSecretsButton[3]  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
                m_pSecretsButton[4]  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            }
            m_pSecretsButton[0]  ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pSecretsButton[1]  ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pSecretsButton[3]  ->SetFlag(ui_win::WF_DISABLED, FALSE);
            m_pSecretsButton[4]  ->SetFlag(ui_win::WF_DISABLED, FALSE);
        }

        for( s32 i=Min; i<Max; i++, j++ )
        {
            if( i > NumUnlocked )
            {
                m_pSecretsButton[j]->SetBitmap( m_SecretsIconID[SECRET_TYPE_UNKNOWN] );
                m_pSecretsButton[j]->SetData( -1 );
            }
            else
            {
                // set the bitmap based on the secrets type      
                secret_entry *pSecret = g_SecretList.GetByIndex(i); 
                m_pSecretsButton[j]->SetBitmap( m_SecretsIconID[ pSecret->SecretType ] );
                m_pSecretsButton[j]->SetData( i );
                Count++;
            }
        }

        m_pSecretsLine1->SetLabel( m_pSecretsSelect->GetSelectedItemLabel() );

        // TODO: Ctetrick - This may be an issue in localized versions!! The colons should align.
        if( VaultIdx == 4 )
            m_pSecretsLine2->SetLabel( xwstring(xfs("%s : %d/%d", (const char*)xstring(g_StringTableMgr("ui", "IDS_UNLOCKED")), Count, 1)) );
        else
            m_pSecretsLine2->SetLabel( xwstring(xfs("%s : %d/%d", (const char*)xstring(g_StringTableMgr("ui", "IDS_UNLOCKED")), Count, 5)) );

        m_pSecretsLine3->SetLabel( xwstring(xfs("%s  : %d/21", (const char*)xstring(g_StringTableMgr("ui", "IDS_OVERALL")), Profile.GetNumSecretsUnlocked())) );

        xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_CYCLE_VAULT" ));
        navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
        m_pNavText->SetLabel( navText );
    }
    else
    {
        // update details text based on selected icon
        const xwchar* pSecretsText;

        if( m_pSelectedIcon->GetBitmap() == m_SecretsIconID[SECRET_TYPE_UNKNOWN] )
        {
            pSecretsText = g_StringTableMgr( "lore", "IDS_SECRET_UNLOCK_INFO" );
            m_CurrentType = SECRET_TYPE_UNKNOWN;
        }
        else
        {
            // get the secrets info
            const secret_entry* pEntry = g_SecretList.GetByIndex( m_pSelectedIcon->GetData() );
            ASSERT( pEntry );
            pSecretsText = g_StringTableMgr( "lore", pEntry->ShortDesc );
            m_CurrentType = pEntry->SecretType;
        }

        ui_font* pFont      = g_UiMgr->GetFont( "small" );
        xwstring Wrapped;
        pFont->TextWrap( pSecretsText, m_pSecretsLine1->GetPosition(), Wrapped );
        m_pSecretsLine1->SetLabel( Wrapped );
        m_pSecretsLine2->SetLabel( g_StringTableMgr( "ui", "IDS_NULL" ) );
        m_pSecretsLine3->SetLabel( g_StringTableMgr( "ui", "IDS_NULL" ) );

        xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
        navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
        m_pNavText->SetLabel( navText );
    }
}

//=========================================================================
