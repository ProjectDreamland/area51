//=========================================================================
//
//  dlg_memcard_select.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_blankbox.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_button.hpp"

#include "dlg_MemcardSelect.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "MemCardMgr/MemCardMgr.hpp"

#if defined(TARGET_PS2)
#include "Entropy\PS2\ps2_misc.hpp"
#endif


//=========================================================================
//  Main Menu Dialog
//=========================================================================

extern s32 g_DownloadCardSlot;

enum controls
{
    IDC_MEMCARD_SELECT_INFO_BOX_1,
    IDC_MEMCARD_SELECT_INFO_BOX_2,
    IDC_MEMCARD_ICON_1,
    IDC_MEMCARD_ICON_2,
    IDC_MEMCARD_SELECT_MSG_BOX,
    IDC_MEMCARD_INFO_TEXT,
    IDC_MEMCARD_SELECT_STATUS_TEXT_1,
    IDC_MEMCARD_SELECT_STATUS_TEXT_2,
    IDC_MEMCARD_MESSAGE_TEXT,
    IDC_MEMCARD_SELECT_NAV_TEXT,
};


ui_manager::control_tem MemcardSelectControls[] = 
{
    // Frames.
    { IDC_MEMCARD_SELECT_INFO_BOX_1,     "IDS_NULL",                    "blankbox",   40,  80, 282,  50, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MEMCARD_SELECT_INFO_BOX_2,     "IDS_NULL",                    "blankbox",   40, 140, 282,  50, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MEMCARD_ICON_1,                "IDS_NULL",                    "button",    342,  80,  48,  48, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MEMCARD_ICON_2,                "IDS_NULL",                    "button",    342, 140,  48,  48, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MEMCARD_SELECT_MSG_BOX,        "IDS_NULL",                    "blankbox",   40, 200, 340, 125, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MEMCARD_INFO_TEXT,             "IDS_MEMCARD_SELECT_LOCATION", "text",       50,  40, 240,  20, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MEMCARD_SELECT_STATUS_TEXT_1,  "IDS_MEMCARD_EMPTY_SLOT",      "text",       50, 105, 262,  20, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MEMCARD_SELECT_STATUS_TEXT_2,  "IDS_MEMCARD_EMPTY_SLOT",      "text",       50, 165, 262,  20, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MEMCARD_MESSAGE_TEXT,          "IDS_NULL",                    "text",       50, 205, 320, 115, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MEMCARD_SELECT_NAV_TEXT,       "IDS_NULL",                    "text",        0,   0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem MemcardSelectDialog =
{
    "IDS_MEMCARD_SELECT_MENU",
    1, 10,
    sizeof(MemcardSelectControls)/sizeof(ui_manager::control_tem),
    &MemcardSelectControls[0],
    0
}; 

//=========================================================================
//  Defines
//=========================================================================

enum MemcardIconTypes
{
    MEMCARD_GOOD = 0,
    MEMCARD_GOOD_SEL,
    MEMCARD_BAD,
    MEMCARD_BAD_SEL,
};

//=========================================================================
//  Structs
//=========================================================================

//=========================================================================
//  Data
//=========================================================================

//=========================================================================
//  Registration function
//=========================================================================

void dlg_memcard_select_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "memcard select", &MemcardSelectDialog, &dlg_memcard_select_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_memcard_select_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_memcard_select* pDialog = new dlg_memcard_select;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_memcard_select
//=========================================================================

dlg_memcard_select::dlg_memcard_select( void )
{
}

//=========================================================================

dlg_memcard_select::~dlg_memcard_select( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_memcard_select::Create( s32                        UserID,
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

    // load bitmaps 
    m_MemcardIconID[MEMCARD_GOOD]       = g_UiMgr->LoadBitmap( "GoodCard",    "UI_PS2_Memcard.xbmp"       );
    m_MemcardIconID[MEMCARD_GOOD_SEL]   = g_UiMgr->LoadBitmap( "GoodCardSel", "UI_PS2_Memcard_Sel.xbmp"   );
    m_MemcardIconID[MEMCARD_BAD]        = g_UiMgr->LoadBitmap( "BadCard",     "UI_PS2_Memcard_X.xbmp"     );
    m_MemcardIconID[MEMCARD_BAD_SEL]    = g_UiMgr->LoadBitmap( "BadCardSel",  "UI_PS2_Memcard_Sel_X.xbmp" );

    m_pInfoBox1     = (ui_blankbox*)    FindChildByID( IDC_MEMCARD_SELECT_INFO_BOX_1    );
    m_pInfoBox2     = (ui_blankbox*)    FindChildByID( IDC_MEMCARD_SELECT_INFO_BOX_2    );
    m_pMemcardOne   = (ui_button*)      FindChildByID( IDC_MEMCARD_ICON_1               );
    m_pMemcardTwo   = (ui_button*)      FindChildByID( IDC_MEMCARD_ICON_2               );
    m_pMessageBox   = (ui_blankbox*)    FindChildByID( IDC_MEMCARD_SELECT_MSG_BOX       );
    m_pStatusText1  = (ui_text*)        FindChildByID( IDC_MEMCARD_SELECT_STATUS_TEXT_1 );
    m_pStatusText2  = (ui_text*)        FindChildByID( IDC_MEMCARD_SELECT_STATUS_TEXT_2 );
    m_pInfoText     = (ui_text*)        FindChildByID( IDC_MEMCARD_INFO_TEXT            );
    m_pMessageText  = (ui_text*)        FindChildByID( IDC_MEMCARD_MESSAGE_TEXT         );
    m_pNavText      = (ui_text*)        FindChildByID( IDC_MEMCARD_SELECT_NAV_TEXT      );
    
    GotoControl( (ui_control*)m_pInfoBox1 );
    m_CurrentControl = IDC_MEMCARD_SELECT_INFO_BOX_1;
    m_CurrHL = 0;
    m_ActiveCard = -1;
    m_PopUp = NULL;

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
    navText += g_StringTableMgr( "ui", "IDS_NAV_CONT_NO_SAVE" );
    m_pNavText->SetLabel( navText );
    m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // set up info boxes
    m_pInfoBox1->SetHasTitleBar( TRUE );
    m_pInfoBox1->SetLabel( g_StringTableMgr( "ui", "IDS_MEMCARD_SLOT_1" ) );
    m_pInfoBox1->SetLabelColor( xcolor(255,252,204,255) );
    m_pInfoBox1->SetTitleBarColor( xcolor(19,59,14,196) );
    m_pInfoBox1->SetBackgroundColor( xcolor (39,117,28,128) );

    m_pInfoBox2->SetHasTitleBar( TRUE );
    m_pInfoBox2->SetLabel( g_StringTableMgr( "ui", "IDS_MEMCARD_SLOT_2" ) );
    m_pInfoBox2->SetLabelColor( xcolor(255,252,204,255) );
    m_pInfoBox2->SetTitleBarColor( xcolor(19,59,14,196) );
    m_pInfoBox2->SetBackgroundColor( xcolor (39,117,28,128) );

    // set up message box
    //m_pMessageBox->SetHasTitleBar( TRUE );
    //m_pMessageBox->SetLabel( g_StringTableMgr( "ui", "IDS_MEMCARD_SLOT_1" ) );
    //m_pMessageBox->SetLabelColor( xcolor(255,252,204,255) );
    //m_pMessageBox->SetTitleBarColor( xcolor(19,59,14,196) );
    m_pMessageBox->SetBackgroundColor( xcolor (39,117,28,128) );   

    // set up status text
    m_pStatusText1   ->UseSmallText( TRUE );
    m_pStatusText2   ->UseSmallText( TRUE );
    m_pInfoText      ->UseSmallText( TRUE );
    m_pMessageText   ->UseSmallText( TRUE );

    m_pStatusText1   ->SetLabelColor( xcolor(255,252,204,255) );
    m_pStatusText2   ->SetLabelColor( xcolor(255,252,204,255) );
    m_pInfoText      ->SetLabelColor( xcolor(255,252,204,255) );
    m_pMessageText   ->SetLabelColor( xcolor(255,252,204,255) );

    m_pStatusText1   ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pStatusText2   ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pInfoText      ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pMessageText   ->SetLabelFlags( ui_font::h_left|ui_font::v_center );

    // set up memcard icons
    m_pMemcardOne->UseNativeColor( TRUE );
    m_pMemcardTwo->UseNativeColor( TRUE );

    // hide elements during scaling
    m_pInfoBox1     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pInfoBox2     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pMemcardOne   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pMemcardTwo   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pMessageBox   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pStatusText1  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pStatusText2  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pInfoText     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pMessageText  ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // disable memcard boxes before card check
    //m_pInfoBox1->SetFlag(ui_win::WF_DISABLED, TRUE);
    //m_pInfoBox2->SetFlag(ui_win::WF_DISABLED, TRUE);
    //g_UiMgr->DisableScreenHighlight();

    // initialize memcard status strings
    m_pStatusText1->SetLabel( g_StringTableMgr( "ui", "IDS_NULL" ) );
    m_pStatusText2->SetLabel( g_StringTableMgr( "ui", "IDS_NULL" ) );

    // initialize screen scaling
    InitScreenScaling( Position );

    // disable background filter
    m_bRenderBlackout = FALSE;

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    //set default mode
    m_Mode = SM_CARDMODE_PROFILE;
    m_Size = 0;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_memcard_select::Configure( card_data_mode mode, s32 size = 0 )
{ 
    m_Mode = mode; 
    m_Size = size; 

    // set up mode specific stuff
    switch( mode )
    {       
    case SM_CARDMODE_PROFILE:
        break;
    case SM_CARDMODE_CONTENT:
        m_bRenderBlackout = TRUE;
        SetLabel( g_StringTableMgr( "ui", "IDS_MEMCARD_SELECT_CONTENT") );
        break;
    case SM_CARDMODE_SETTINGS:
        m_Size = g_StateMgr.GetSettingsSaveSize();
        SetLabel( g_StringTableMgr( "ui", "IDS_MEMCARD_SELECT_SETTINGS") );
        break;
    }
}

//=========================================================================

void dlg_memcard_select::Destroy( void )
{
#ifdef TARGET_PS2
    // wait until we finish drawing before we unload the logo bitmap
    DLIST.Flush();
    DLIST.WaitForTasks();
#endif

    g_UiMgr->UnloadBitmap( "GoodCard"    );
    g_UiMgr->UnloadBitmap( "GoodCardSel" );
    g_UiMgr->UnloadBitmap( "BadCard"     );
    g_UiMgr->UnloadBitmap( "BadCardSel"  );

    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_memcard_select::Render( s32 ox, s32 oy )
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

void dlg_memcard_select::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );

        UpdateCardMessageText();
    }
}
//=========================================================================

void dlg_memcard_select::OnPadActivate( ui_win* pWin )
{
    (void)pWin;

    if( m_Mode == SM_CARDMODE_CONTENT )
    {
        // can't continue without saving
        return;
    }

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        // continue without saving to memory card
        if( m_Mode == SM_CARDMODE_SETTINGS )
        {
            // Clear the poll callback
            g_UIMemCardMgr.ClearCallback();
            // clear card slot
            g_StateMgr.SetSettingsCardSlot( -1 );
            // update the changes in the settings
            g_StateMgr.ActivatePendingSettings(); 
            // apply the settings
            global_settings& Settings = g_StateMgr.GetActiveSettings();
            Settings.Commit();
            // continue to main menu
            m_State = DIALOG_STATE_SELECT;
        }
        else
        {
            // Clear the poll callback
            g_UIMemCardMgr.ClearCallback();
            // flag the profile as not saved
            g_StateMgr.SetProfileNotSaved( g_StateMgr.GetPendingProfileIndex(), TRUE );
            // Continue without saving
            g_StateMgr.ActivatePendingProfile();
            // continue to campaign menu
            m_State = DIALOG_STATE_SELECT;
        }
    }
}

//=========================================================================

void dlg_memcard_select::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( m_pInfoBox1->GetFlags(WF_HIGHLIGHT) )
        {
            // get the current status of the selected card
            MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( 0 );

                        
            if( Condition.bUnformatted )
            {
                // format the card
                if( m_Mode == SM_CARDMODE_PROFILE )
                    g_UIMemCardMgr.Format( 0, g_StateMgr.GetPendingProfileIndex(), m_Mode, this, &dlg_memcard_select::OnFormatCB );
                else
                    g_UIMemCardMgr.Format( 0, (MAX_PLAYER_SLOTS-1), m_Mode, this, &dlg_memcard_select::OnFormatCB );
            }
            else if( Condition.bInsufficientSpace || Condition.bIsFull )
            {
                if( ( m_Mode == SM_CARDMODE_SETTINGS ) && ( g_UIMemCardMgr.CardHasSettings(0) ) )
                {
                    // attempt to save settings
                    g_StateMgr.SetSettingsCardSlot( 0 );
                    g_UIMemCardMgr.OverwriteSettings( this, &dlg_memcard_select::OnCreateSettingsCB );
                }
                else
                {
                    // nothing to overwrite and no space!
                    return;
                }
            }
            else if( Condition.bDamaged || Condition.bNoCard )
            {
                // bad selection - do nothing
                return;
            }
            else
            {
                if( m_Mode == SM_CARDMODE_CONTENT )
                {
                    g_DownloadCardSlot = 0;
                    m_State = DIALOG_STATE_SELECT;
                    g_AudioMgr.Play( "Select_Norm" );
                    return;
                }
                else if( m_Mode == SM_CARDMODE_SETTINGS )
                {
                    // overwriting?
                    if( g_UIMemCardMgr.CardHasSettings( 0 ) )
                    {
                        // attempt to save settings
                        g_StateMgr.SetSettingsCardSlot( 0 );
                        g_UIMemCardMgr.OverwriteSettings( this, &dlg_memcard_select::OnCreateSettingsCB );
                    }
                    else
                    {
                        // attempt to save settings
                        g_StateMgr.SetSettingsCardSlot( 0 );
                        g_UIMemCardMgr.CreateSettings( this, &dlg_memcard_select::OnCreateSettingsCB );
                    }
                }
                else
                {
                    // attempt to save the profile to the memcard
                    g_UIMemCardMgr.CreateProfile( 0, g_StateMgr.GetPendingProfileIndex(), this, &dlg_memcard_select::OnProfileCreateCB );
                }
            }

            // set active card
            m_ActiveCard = 0;

            // attempt to save to slot 0
            g_AudioMgr.Play( "Select_Norm" );

            // change the dialog state to wait for the memcard
            m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
        }
        else if( m_pInfoBox2->GetFlags(WF_HIGHLIGHT) )
        {
            // get the current status of the selected card
            MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( 1 );

            if( Condition.bUnformatted )
            {
                // format the card
                if( m_Mode == SM_CARDMODE_PROFILE )
                    g_UIMemCardMgr.Format( 1, g_StateMgr.GetPendingProfileIndex(), m_Mode, this, &dlg_memcard_select::OnFormatCB );
                else
                    g_UIMemCardMgr.Format( 1, (MAX_PLAYER_SLOTS-1), m_Mode, this, &dlg_memcard_select::OnFormatCB );
            }
            else if( Condition.bInsufficientSpace || Condition.bIsFull )
            {
                if( ( m_Mode == SM_CARDMODE_SETTINGS ) && ( g_UIMemCardMgr.CardHasSettings( 1 ) ) )
                {
                    // attempt to save settings
                    g_StateMgr.SetSettingsCardSlot( 1 );
                    g_UIMemCardMgr.OverwriteSettings( this, &dlg_memcard_select::OnCreateSettingsCB );
                }
                else
                {
                    // nothing to overwrite and no space!
                    return;
                }
            }
            else if( Condition.bDamaged || Condition.bNoCard )
            {
                // bad selection - do nothing
                return;
            }
            else
            {
                if( m_Mode == SM_CARDMODE_CONTENT )
                {
                    g_DownloadCardSlot = 1;
                    m_State = DIALOG_STATE_SELECT;
                    g_AudioMgr.Play( "Select_Norm" );
                    return;
                }
                else if( m_Mode == SM_CARDMODE_SETTINGS )
                {
                    // overwriting?
                    if( g_UIMemCardMgr.CardHasSettings( 1 ) )
                    {
                        // attempt to save settings
                        g_StateMgr.SetSettingsCardSlot( 1 );
                        g_UIMemCardMgr.OverwriteSettings( this, &dlg_memcard_select::OnCreateSettingsCB );
                    }
                    else
                    {
                        // attempt to save settings
                        g_StateMgr.SetSettingsCardSlot( 1 );
                        g_UIMemCardMgr.CreateSettings( this, &dlg_memcard_select::OnCreateSettingsCB );
                    }
                }
                else
                {
                    // attempt to save the profile to the memcard
                    g_UIMemCardMgr.CreateProfile( 1, g_StateMgr.GetPendingProfileIndex(), this, &dlg_memcard_select::OnProfileCreateCB );
                }
            }

            // set active card
            m_ActiveCard = 1;

            // attempt to save to slot 1
            g_AudioMgr.Play( "Select_Norm" );

            // change the dialog state to wait for the memcard
            m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
        }
    }
}

//=========================================================================

void dlg_memcard_select::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        // open cancel save popup 
        irect r = g_UiMgr->GetUserBounds( g_UiUserID );
        m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

        // set nav text
        xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
        navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
        m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

        xwstring MessageHeader;
        xwstring MessageText;

        if( m_Mode == SM_CARDMODE_CONTENT )
        {
            MessageHeader = g_StringTableMgr( "ui", "IDS_DL_CONTENT_HEADER" );
            MessageText   = g_StringTableMgr( "ui", "IDS_DL_CONTENT_CANCEL_MSG" );            
        }
        else if( m_Mode == SM_CARDMODE_SETTINGS)
        {
            MessageHeader = g_StringTableMgr( "ui", "IDS_SAVE_SETTINGS_HEADER" );
            MessageText   = g_StringTableMgr( "ui", "IDS_SAVE_SETTINGS_CANCEL_MSG" );                        
        }
        else
        {
            MessageHeader = g_StringTableMgr( "ui", "IDS_PROFILE_CREATE" );
            MessageText   = g_StringTableMgr( "ui", "IDS_PROFILE_CANCEL_CREATE_MSG" );
        }

        m_PopUp->Configure( MessageHeader, 
            TRUE, 
            TRUE, 
            FALSE, 
            MessageText,
            navText,
            &m_PopUpResult );

        // set dialog state
        m_State = DIALOG_STATE_POPUP;

        return;
    }
}

//=========================================================================

void dlg_memcard_select::OnPollReturn( void )
{
    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        m_pStatusText1->SetLabel( g_StringTableMgr( "ui", "IDS_NULL" ) );
        m_pStatusText2->SetLabel( g_StringTableMgr( "ui", "IDS_NULL" ) );

        // get the current status of the memcards
        MemCardMgr::condition& Condition1 = g_UIMemCardMgr.GetCondition( 0 );
        MemCardMgr::condition& Condition2 = g_UIMemCardMgr.GetCondition( 1 );

        // update the card status text 
        if ( Condition1.bDamaged )
            m_pStatusText1->SetLabel( g_StringTableMgr( "ui", "IDS_MEMCARD_MSG_DAMAGED" ) );
        else if ( Condition1.bUnformatted )
            m_pStatusText1->SetLabel( g_StringTableMgr( "ui", "IDS_MEMCARD_MSG_UNFORMATTED" ) );
        else if ( !Condition1.bNoCard )
            m_pStatusText1->SetLabel( xwstring( xfs( "%d/%s", (u32)(Condition1.BytesFree/1024), (const char*)xstring(g_StringTableMgr( "ui", "IDS_MEMCARD_MSG_FREE")) ) ) );
        else
            m_pStatusText1->SetLabel( g_StringTableMgr( "ui", "IDS_MEMCARD_MSG_NO_CARD" ) );

        if ( Condition2.bDamaged )
            m_pStatusText2->SetLabel( g_StringTableMgr( "ui", "IDS_MEMCARD_MSG_DAMAGED" ) );
        else if ( Condition2.bUnformatted )
            m_pStatusText2->SetLabel( g_StringTableMgr( "ui", "IDS_MEMCARD_MSG_UNFORMATTED" ) );
        else if ( !Condition2.bNoCard )
            m_pStatusText2->SetLabel( xwstring( xfs( "%d/%s", (u32)(Condition2.BytesFree/1024), (const char*)xstring(g_StringTableMgr( "ui", "IDS_MEMCARD_MSG_FREE")) ) ) );
        else
            m_pStatusText2->SetLabel( g_StringTableMgr( "ui", "IDS_MEMCARD_MSG_NO_CARD" ) );

        // update the card message text
        UpdateCardMessageText();
    }
}

//=========================================================================

void dlg_memcard_select::OnFormatCB( void )
{
    ASSERT( m_ActiveCard != -1 );

    // get the condition of the card that we were formatting
    MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( m_ActiveCard );

    // check result
    if( Condition.SuccessCode )
    {
        if( Condition.bCancelled )
        {
            // format aborted by user, return to polling
            m_State = DIALOG_STATE_ACTIVE;  
        }
        else
        {
            if( m_Mode == SM_CARDMODE_PROFILE )
            {
                // formatted and saved profile successfully

                // update the changes in the profile
                g_StateMgr.SetProfileNotSaved( g_StateMgr.GetPendingProfileIndex(), FALSE );
                g_StateMgr.ActivatePendingProfile();
            }
            else if( m_Mode == SM_CARDMODE_SETTINGS )
            {
                // activate the new settings
                g_StateMgr.ActivatePendingSettings();
            }

            g_AudioMgr.Play( "Select_Norm" );
            // continue to campaign menu
            m_State = DIALOG_STATE_SELECT;
        }
    }
    else
    {
        // action cancelled by user, return to polling
        m_State = DIALOG_STATE_ACTIVE;  
    }
}

//=========================================================================

void dlg_memcard_select::OnProfileCreateCB( void )
{
    ASSERT( m_ActiveCard != -1 );

    // get the condition of the card that we were formatting
    MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( m_ActiveCard );

    // If the save was successful 
    if( Condition.SuccessCode )
    {
        // saved ok

        // update the changes in the profile
        g_StateMgr.SetProfileNotSaved( g_StateMgr.GetPendingProfileIndex(), FALSE );
        g_StateMgr.ActivatePendingProfile();
        g_AudioMgr.Play( "Select_Norm" );

        // continue to campaign menu
        m_State = DIALOG_STATE_SELECT;
    }
    else
    {
        // action cancelled by user, return to polling
        m_State = DIALOG_STATE_ACTIVE;  
    }
}

//=========================================================================

void dlg_memcard_select::OnCreateSettingsCB( void )
{
    ASSERT( m_ActiveCard != -1 );

    // get the condition of the card that we were formatting
    MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( m_ActiveCard );

    // If the save was successful 
    if( Condition.SuccessCode )
    {
        // continue without saving?
        if( Condition.bCancelled )
        {
            // clear settings card slot
            g_StateMgr.SetSettingsCardSlot( -1 );

            // return to polling
            m_State = DIALOG_STATE_ACTIVE;  
        }
        else
        {
            // saved ok
            g_AudioMgr.Play( "Select_Norm" );

            // activate the new settings
            g_StateMgr.ActivatePendingSettings();

            // continue to next menu
            m_State = DIALOG_STATE_SELECT;
        }
    }
    else
    {
        // action cancelled by user, clear card slot
        g_StateMgr.SetSettingsCardSlot( -1 );

        // return to polling
        m_State = DIALOG_STATE_ACTIVE;  
    }
}

//=========================================================================

void dlg_memcard_select::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
#if defined(cgalley) && defined(TARGET_PC)
    // continue to campaign menu
    m_State = DIALOG_STATE_SELECT;
#endif

    (void)pWin;
    (void)DeltaTime;

    s32 highLight = -1;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // complete!  turn on the elements
            m_pStatusText1  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pStatusText2  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pMemcardOne   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pMemcardTwo   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pMessageBox   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pInfoBox1     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pInfoBox2     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText      ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pInfoText     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pMessageText  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
        return;
    }

    if ( m_PopUp )
    {
        if ( m_PopUpResult != DLG_POPUP_IDLE )
        {
            if( m_PopUpResult == DLG_POPUP_YES )
            {
                // cancel save 
                g_AudioMgr.Play("Backup");
                m_State = DIALOG_STATE_BACK;
                g_UIMemCardMgr.ClearCallback();

                if( m_Mode == SM_CARDMODE_PROFILE )
                {
                    g_StateMgr.ClearSelectedProfile( g_StateMgr.GetPendingProfileIndex() );
                }

                if( m_Mode == SM_CARDMODE_SETTINGS )
                {
                    // clear settings card slot
                    g_StateMgr.SetSettingsCardSlot( -1 );
                }
            }
            else
            {
                // re-enable the dialog
                m_State = DIALOG_STATE_ACTIVE;
            }

            // clear popup 
            m_PopUp = NULL;

            // turn on nav text
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
    }
    else if ( m_State == DIALOG_STATE_ACTIVE )
    {
        // poll the memcards to keep the status of each card current.
        g_UIMemCardMgr.Poll( m_Mode, this, &dlg_memcard_select::OnPollReturn );

        if( m_pInfoBox1->GetFlags(WF_HIGHLIGHT) )
        {
            g_UiMgr->SetScreenHighlight( m_pInfoBox1->GetPosition() );
            highLight = 0;
        }
        
        if( m_pInfoBox2->GetFlags(WF_HIGHLIGHT) )
        {
            g_UiMgr->SetScreenHighlight( m_pInfoBox2->GetPosition() );
            highLight = 1;
        }
        
        if( highLight != m_CurrHL )
        {
            if( highLight != -1 )
                g_AudioMgr.Play("Cusor_Norm");

            m_CurrHL = highLight;
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);
}

//=========================================================================

void dlg_memcard_select::UpdateCardMessageText( void )
{
    // get the current status of the memcards
    MemCardMgr::condition& Condition1 = g_UIMemCardMgr.GetCondition( 0 );
    MemCardMgr::condition& Condition2 = g_UIMemCardMgr.GetCondition( 1 );

    // update message text according to selected card status
    if( m_pInfoBox1->GetFlags(WF_HIGHLIGHT) )
    {
        if( ( m_Mode == SM_CARDMODE_SETTINGS ) && ( g_UIMemCardMgr.CardHasSettings(0) ) )
        {
            // overwrite settings file
            m_pMessageText->SetLabel( g_StringTableMgr( "ui", "MC_OVERWRITE_SETTINGS_SLOT1_MSG" ) );            
        }
        else if ( Condition1.bUnformatted )
        {
            m_pMessageText->SetLabel( g_StringTableMgr( "ui", "MC_FORMAT_SLOT1_MSG" ) );
        }
        else if( Condition1.bDamaged )
        {
            m_pMessageText->SetLabel( g_StringTableMgr( "ui", "MC_DATA_CORRUPT_SLOT1" ) );
        }
        else if ( Condition1.bNoCard )
        {
            m_pMessageText->SetLabel( g_StringTableMgr( "ui", "MC_NO_CARD_SLOT1" ) );
        }
#if 0
        else if ( Condition1.bIsFull )
        {
            switch( m_Mode )
            {
                case SM_CARDMODE_PROFILE:
                    m_pMessageText->SetLabel( xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_CARD_FULL_SLOT1" )), g_StateMgr.GetProfileSaveSize()/1024 ) ) );
                    break;
                case SM_CARDMODE_SETTINGS:
                    m_pMessageText->SetLabel( xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_CARD_FULL_SLOT1_SETTINGS" )), g_StateMgr.GetSettingsSaveSize()/1024 ) ) );                  
                    break;
                default:
                    m_pMessageText->SetLabel( xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_CARD_FULL" )), 1, m_Size/1024 ) ) );
                    break;
            }
        }
#endif
        else if ( Condition1.bInsufficientSpace || Condition1.bIsFull )
        {
            switch( m_Mode )
            {
                case SM_CARDMODE_PROFILE:
                    m_pMessageText->SetLabel( xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NOT_ENOUGH_FREE_SPACE_SLOT1" )), g_StateMgr.GetProfileSaveSize()/1024 ) ) );
                    break;
                case SM_CARDMODE_SETTINGS:
                    m_pMessageText->SetLabel( xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NOT_ENOUGH_FREE_SPACE_SLOT1_SETTINGS" )), g_StateMgr.GetSettingsSaveSize()/1024 ) ) );
                    break;
                default:
                    m_pMessageText->SetLabel( xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NOT_ENOUGH_FREE_SPACE" )), 1, m_Size/1024 ) ) );
                    break;
            }
        }
        else
        {
            // card is OK and has enough space to save the profile
            switch( m_Mode )
            {
                case SM_CARDMODE_PROFILE:
                    m_pMessageText->SetLabel( xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "IDS_MEMCARD_SPACE_MSG" )), g_StateMgr.GetProfileSaveSize()/1024 ) ) );
                    break;
                case SM_CARDMODE_SETTINGS:
                    m_pMessageText->SetLabel( xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "IDS_MEMCARD_SPACE_MSG_SETTINGS" )), g_StateMgr.GetSettingsSaveSize()/1024 ) ) );
                    break;
                default:
                    m_pMessageText->SetLabel( xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "IDS_MEMCARD_SPACE" )), m_Size/1024 ) ) );
                    break;
            }
        }

        // update memcard icons
        if( ( m_Mode == SM_CARDMODE_SETTINGS ) && ( g_UIMemCardMgr.CardHasSettings(0) ) )
        {
            m_pMemcardOne->SetBitmap( m_MemcardIconID[MEMCARD_GOOD_SEL] );

            // update nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_OVERWRITE" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
            navText += g_StringTableMgr( "ui", "IDS_NAV_CONT_NO_SAVE" );
            m_pNavText->SetLabel( navText );
        }
        else if( Condition1.bUnformatted )
        {
            m_pMemcardOne->SetBitmap( m_MemcardIconID[MEMCARD_GOOD_SEL] );

            // update nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
            if( m_Mode != SM_CARDMODE_CONTENT )
            {
                navText += g_StringTableMgr( "ui", "IDS_NAV_CONT_NO_SAVE" );
            }
            m_pNavText->SetLabel( navText );
        }
        else if( Condition1.bDamaged || Condition1.bNoCard || Condition1.bInsufficientSpace || Condition1.bIsFull )
        {
            m_pMemcardOne->SetBitmap( m_MemcardIconID[MEMCARD_BAD_SEL] );

            // update nav text
            xwstring navText( g_StringTableMgr( "ui", "IDS_NAV_BACK" ) );
            if( m_Mode != SM_CARDMODE_CONTENT )
            {
                navText += g_StringTableMgr( "ui", "IDS_NAV_CONT_NO_SAVE" );
            }
            m_pNavText->SetLabel( navText );
        }
        else
        {
            m_pMemcardOne->SetBitmap( m_MemcardIconID[MEMCARD_GOOD_SEL] );

            // update nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
            if( m_Mode != SM_CARDMODE_CONTENT )
            {
                navText += g_StringTableMgr( "ui", "IDS_NAV_CONT_NO_SAVE" );
            }
            m_pNavText->SetLabel( navText );
        }

        if( ( m_Mode == SM_CARDMODE_SETTINGS ) && ( g_UIMemCardMgr.CardHasSettings(1) ) )
            m_pMemcardTwo->SetBitmap( m_MemcardIconID[MEMCARD_GOOD] );
        else if( Condition2.bUnformatted )
            m_pMemcardTwo->SetBitmap( m_MemcardIconID[MEMCARD_GOOD] );
        else if( Condition2.bDamaged || Condition2.bNoCard || Condition2.bInsufficientSpace || Condition2.bIsFull )
            m_pMemcardTwo->SetBitmap( m_MemcardIconID[MEMCARD_BAD] );
        else
            m_pMemcardTwo->SetBitmap( m_MemcardIconID[MEMCARD_GOOD] );

    }
    else
    {
        if( ( m_Mode == SM_CARDMODE_SETTINGS ) && ( g_UIMemCardMgr.CardHasSettings(1) ) )
        {
            // overwrite settings file
            m_pMessageText->SetLabel( g_StringTableMgr( "ui", "MC_OVERWRITE_SETTINGS_SLOT2_MSG" ) );            
        }
        else if ( Condition2.bUnformatted )
        {
            m_pMessageText->SetLabel( g_StringTableMgr( "ui", "MC_FORMAT_SLOT2_MSG" ) );
        }
        else if( Condition2.bDamaged )
        {
            m_pMessageText->SetLabel( g_StringTableMgr( "ui", "MC_DATA_CORRUPT_SLOT2" ) );
        }
        else if ( Condition2.bNoCard )
        {
            m_pMessageText->SetLabel( g_StringTableMgr( "ui", "MC_NO_CARD_SLOT2" ) );
        }
#if 0
        else if ( Condition2.bIsFull )
        {
            switch( m_Mode )
            {
            case SM_CARDMODE_PROFILE:
                m_pMessageText->SetLabel( xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_CARD_FULL_SLOT2" )), g_StateMgr.GetProfileSaveSize()/1024 ) ) );
                break;
            case SM_CARDMODE_SETTINGS:
                m_pMessageText->SetLabel( xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_CARD_FULL_SLOT2_SETTINGS" )), g_StateMgr.GetSettingsSaveSize()/1024 ) ) );                  
                break;
            default:
                m_pMessageText->SetLabel( xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_CARD_FULL" )), 2, m_Size/1024 ) ) );
                break;
            }
        }
#endif
        else if ( Condition2.bInsufficientSpace || Condition2.bIsFull )
        {
            switch( m_Mode )
            {
            case SM_CARDMODE_PROFILE:
                m_pMessageText->SetLabel( xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NOT_ENOUGH_FREE_SPACE_SLOT2" )), g_StateMgr.GetProfileSaveSize()/1024 ) ) );
                break;
            case SM_CARDMODE_SETTINGS:
                m_pMessageText->SetLabel( xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NOT_ENOUGH_FREE_SPACE_SLOT2_SETTINGS" )), g_StateMgr.GetSettingsSaveSize()/1024 ) ) );
                break;
            default:
                m_pMessageText->SetLabel( xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NOT_ENOUGH_FREE_SPACE" )), 2, m_Size/1024 ) ) );
                break;
            }
        }
        else
        {
            // card is OK and has enough space to save the profile
            switch( m_Mode )
            {
            case SM_CARDMODE_PROFILE:
                m_pMessageText->SetLabel( xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "IDS_MEMCARD_SPACE_MSG" )), g_StateMgr.GetProfileSaveSize()/1024 ) ) );
                break;
            case SM_CARDMODE_SETTINGS:
                m_pMessageText->SetLabel( xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "IDS_MEMCARD_SPACE_MSG_SETTINGS" )), g_StateMgr.GetSettingsSaveSize()/1024 ) ) );
                break;
            default:
                m_pMessageText->SetLabel( xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "IDS_MEMCARD_SPACE" )), m_Size/1024 ) ) );
                break;
            }
        }

        // update memcard icons
        if( ( m_Mode == SM_CARDMODE_SETTINGS ) && ( g_UIMemCardMgr.CardHasSettings(0) ) )
            m_pMemcardOne->SetBitmap( m_MemcardIconID[MEMCARD_GOOD] );
        else if( Condition1.bUnformatted )
            m_pMemcardOne->SetBitmap( m_MemcardIconID[MEMCARD_GOOD] );
        else if( Condition1.bDamaged || Condition1.bNoCard || Condition1.bInsufficientSpace || Condition1.bIsFull )
            m_pMemcardOne->SetBitmap( m_MemcardIconID[MEMCARD_BAD] );
        else
            m_pMemcardOne->SetBitmap( m_MemcardIconID[MEMCARD_GOOD] );

        if( ( m_Mode == SM_CARDMODE_SETTINGS ) && ( g_UIMemCardMgr.CardHasSettings(1) ) )
        {
            m_pMemcardTwo->SetBitmap( m_MemcardIconID[MEMCARD_GOOD_SEL] );

            // update nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_OVERWRITE" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
            navText += g_StringTableMgr( "ui", "IDS_NAV_CONT_NO_SAVE" );
            m_pNavText->SetLabel( navText );
        }
        else if( Condition2.bUnformatted )
        {
            m_pMemcardTwo->SetBitmap( m_MemcardIconID[MEMCARD_GOOD_SEL] );

            // update nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
            if( m_Mode != SM_CARDMODE_CONTENT )
            {
                navText += g_StringTableMgr( "ui", "IDS_NAV_CONT_NO_SAVE" );
            }
            m_pNavText->SetLabel( navText );
        }
        else if( Condition2.bDamaged || Condition2.bNoCard || Condition2.bInsufficientSpace || Condition2.bIsFull )
        {
            m_pMemcardTwo->SetBitmap( m_MemcardIconID[MEMCARD_BAD_SEL] );

            // update nav text
            xwstring navText( g_StringTableMgr( "ui", "IDS_NAV_BACK" ) );
            if( m_Mode != SM_CARDMODE_CONTENT )
            {
                navText += g_StringTableMgr( "ui", "IDS_NAV_CONT_NO_SAVE" );
            }
            m_pNavText->SetLabel( navText );
        }
        else
        {
            m_pMemcardTwo->SetBitmap( m_MemcardIconID[MEMCARD_GOOD_SEL] );

            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
            if( m_Mode != SM_CARDMODE_CONTENT )
            {
                navText += g_StringTableMgr( "ui", "IDS_NAV_CONT_NO_SAVE" );
            }
            m_pNavText->SetLabel( navText );
        }
    }
}

//=========================================================================

