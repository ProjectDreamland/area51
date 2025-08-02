//=========================================================================
//
//  dlg_profile_select.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_listbox.hpp"
#include "ui\ui_dlg_vkeyboard.hpp"
#include "ui\ui_blankbox.hpp"

#include "dlg_PopUp.hpp"
#include "dlg_ProfileSelect.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Parsing/textin.hpp"
#ifdef CONFIG_VIEWER
#include "../../Apps/ArtistViewer/Config.hpp"
#else
#include "../../Apps/GameApp/Config.hpp"	
#endif
#include "MemCardMgr/MemCardMgr.hpp"


//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum popup_type
{
    POPUP_TYPE_DELETE,
    POPUP_TYPE_BADNAME,
    POPUP_TYPE_DAMAGED_PROFILE,
    POPUP_XBOX_FREE_MORE_BLOCKS,
};

enum controls
{
	IDC_PROFILE_SELECT_LISTBOX,
    IDC_PROFILE_SELECT_INFOBOX,

    IDC_PROFILE_CARD_SLOT,
    IDC_PROFILE_CREATE_DATE,
    IDC_PROFILE_MODIFIED_DATE,

    IDC_PROFILE_INFO_CREATE_DATE,
    IDC_PROFILE_INFO_MODIFIED_DATE,

    IDC_PROFILE_SELECT_NAV_TEXT,
};


ui_manager::control_tem ProfileSelectControls[] = 
{
    // Frames.
    { IDC_PROFILE_SELECT_LISTBOX,       "IDS_PROFILE_PROFILES",         "listbox",  45,  40, 240, 206, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PROFILE_SELECT_INFOBOX,       "IDS_PROFILE_INFO",             "blankbox", 45, 256, 240,  76, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_PROFILE_CARD_SLOT,            "IDS_NULL",                     "text",     53, 278, 120,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PROFILE_CREATE_DATE,          "IDS_PROFILE_CREATE_DATE",      "text",     53, 294, 120,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PROFILE_MODIFIED_DATE,        "IDS_PROFILE_MODIFIED_DATE",    "text",     53, 310, 120,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_PROFILE_INFO_CREATE_DATE,     "IDS_NULL",                     "text",    157, 294,  80,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PROFILE_INFO_MODIFIED_DATE,   "IDS_NULL",                     "text",    157, 310,  80,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_PROFILE_SELECT_NAV_TEXT,      "IDS_NULL",                     "text",      0,   0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};

ui_manager::dialog_tem ProfileSelectDialog =
{
    "IDS_PROFILE_MAIN_MENU",
    1, 9,
    sizeof(ProfileSelectControls)/sizeof(ui_manager::control_tem),
    &ProfileSelectControls[0],
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

void dlg_profile_select_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "profile select", &ProfileSelectDialog, &dlg_profile_select_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_profile_select_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_profile_select* pDialog = new dlg_profile_select;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_level_select
//=========================================================================

dlg_profile_select::dlg_profile_select( void )
{
}

//=========================================================================

dlg_profile_select::~dlg_profile_select( void )
{
}

//=========================================================================

xbool dlg_profile_select::Create( s32                        UserID,
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
    m_pProfileList     = (ui_listbox*)    FindChildByID( IDC_PROFILE_SELECT_LISTBOX  );
    m_pNavText         = (ui_text*)       FindChildByID( IDC_PROFILE_SELECT_NAV_TEXT );

    // hide them
    m_pProfileList    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pNavText        ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
    navText += g_StringTableMgr( "ui", "IDS_NAV_DELETE" );
   
    m_pNavText->SetLabel( navText );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    g_UIMemCardMgr.Poll( SM_CARDMODE_PROFILE, this, &dlg_profile_select::OnPollReturn );

    // set up level list
    m_pProfileList->SetFlag(ui_win::WF_SELECTED, TRUE);
    m_pProfileList->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pProfileList->DisableFrame();
    m_pProfileList->SetExitOnSelect(FALSE);
    m_pProfileList->SetExitOnBack(TRUE);
    m_pProfileList->EnableHeaderBar();
    m_pProfileList->SetHeaderBarColor( xcolor(19,59,14,196) );
    m_pProfileList->SetHeaderColor( xcolor(255,252,204,255) );

    // get profile details box
    m_pProfileDetails = (ui_blankbox*)FindChildByID( IDC_PROFILE_SELECT_INFOBOX );
    m_pProfileDetails->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pProfileDetails->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pProfileDetails->SetHasTitleBar( TRUE );
    m_pProfileDetails->SetLabelColor( xcolor(255,252,204,255) );
    m_pProfileDetails->SetTitleBarColor( xcolor(19,59,14,196) );

    // set up profile info text
    m_pCardSlot         = (ui_text*)FindChildByID( IDC_PROFILE_CARD_SLOT          );
    m_pCreationDate     = (ui_text*)FindChildByID( IDC_PROFILE_CREATE_DATE        );
    m_pModifiedDate     = (ui_text*)FindChildByID( IDC_PROFILE_MODIFIED_DATE      );
    m_pInfoCreationDate = (ui_text*)FindChildByID( IDC_PROFILE_INFO_CREATE_DATE   );
    m_pInfoModifiedDate = (ui_text*)FindChildByID( IDC_PROFILE_INFO_MODIFIED_DATE );

    m_pCardSlot         ->UseSmallText( TRUE );
    m_pCreationDate     ->UseSmallText( TRUE );
    m_pModifiedDate     ->UseSmallText( TRUE );
    m_pInfoCreationDate ->UseSmallText( TRUE );
    m_pInfoModifiedDate ->UseSmallText( TRUE );

    m_pCardSlot         ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pCreationDate     ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pModifiedDate     ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pInfoCreationDate ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pInfoModifiedDate ->SetLabelFlags( ui_font::h_left|ui_font::v_center );

    m_pCardSlot         ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pCreationDate     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pModifiedDate     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pInfoCreationDate ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pInfoModifiedDate ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    m_pCardSlot         ->SetLabelColor( xcolor(255,252,204,255) );
    m_pCreationDate     ->SetLabelColor( xcolor(255,252,204,255) );
    m_pModifiedDate     ->SetLabelColor( xcolor(255,252,204,255) );
    m_pInfoCreationDate ->SetLabelColor( xcolor(255,252,204,255) );
    m_pInfoModifiedDate ->SetLabelColor( xcolor(255,252,204,255) );


    // Initialize dialog mode
    m_Type = PROFILE_SELECT_NORMAL;
    m_bEditProfile = FALSE;

    // get profile data
    RefreshProfileList();
    m_ProfileName = g_StringTableMgr("ui", "IDS_PROFILE_DEFAULT_PLAYER");
    m_ProfileEntered = FALSE;
    m_ProfileOk = FALSE;

    // clear the selected profile
    //g_StateMgr.ClearSelectedProfile( 0 );

    // set initial focus
    m_CurrHL = 0;
    GotoControl( (ui_control*)m_pProfileList );
    m_CurrentControl = IDC_PROFILE_SELECT_LISTBOX;
    m_PopUp = NULL;
    m_BlocksRequired = 0;

    // BackupPopup
    m_BackupPopup = NULL;

    // initialize screen scaling
    InitScreenScaling( Position );

    // disable blackout
    m_bRenderBlackout = FALSE;

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

	// Return success code
    return Success;
}

//=========================================================================

void dlg_profile_select::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_profile_select::Configure( profile_select_type DialogType )
{
    m_Type = DialogType;

    switch( m_Type )
    {
        case PROFILE_SELECT_MANAGE:
        {
            SetLabel( g_StringTableMgr( "ui", "IDS_PROFILE_MANAGE_PROFILES" ) );
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
            navText += g_StringTableMgr( "ui", "IDS_NAV_DELETE" );
            navText += g_StringTableMgr( "ui", "IDS_NAV_EDIT" );
            m_pNavText->SetLabel( navText );
        }
        break;

        case PROFILE_SELECT_NORMAL:
            break;

        case PROFILE_SELECT_OVERWRITE:
            break;
    }
}

//=========================================================================

void dlg_profile_select::Render( s32 ox, s32 oy )
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
void dlg_profile_select::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    // only allow navigation if active
    if( m_State == DIALOG_STATE_ACTIVE )
    {
        ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
    }
}

//=========================================================================

void dlg_profile_select::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if( m_State == DIALOG_STATE_ACTIVE )
    {
        // check for bad profile
        if( m_pProfileList->GetSelectedItemData( 1 ) != PROFILE_OK )
        {
#ifdef TARGET_XBOX
            // open damaged profile popup
            irect r = g_UiMgr->GetUserBounds( g_UiUserID );
            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
            m_PopUpType = POPUP_TYPE_DAMAGED_PROFILE;

            // set nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_OK" ));
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

            m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_NULL" ), 
                TRUE, 
                FALSE, 
                FALSE, 
                g_StringTableMgr( "ui", "IDS_DAMAGED_PROFILE_MSG_XBOX" ),
                navText,
                &m_PopUpResult );

            return;            
#else
            g_AudioMgr.Play( "InvalidEntry" );
            return;
#endif
        }

        // get the profile index from the list
        s32 index = m_pProfileList->GetSelectedItemData();

        // check if this is a new profile
        if( index < m_CreateIndex )
        {
            if( m_Type == PROFILE_SELECT_OVERWRITE )
            {
                // get the profile list
                xarray<profile_info*>& ProfileNames = g_StateMgr.GetProfileList();

                // calculate a new hash string for the profile we are overwriting
                player_profile& NewProfile = g_StateMgr.GetPendingProfile();
                NewProfile.SetHash();

                // store the id of the selected profile
                g_StateMgr.SetSelectedProfile( g_StateMgr.GetPendingProfileIndex(), NewProfile.GetHash() );//ProfileNames[index]->Hash );

                // overwrite the selected profile
                m_iCard = ProfileNames[index]->CardID;
                g_UIMemCardMgr.OverwriteProfile( *ProfileNames[index], g_StateMgr.GetPendingProfileIndex(), this, &dlg_profile_select::OnSaveProfileCB );

                // change the dialog state to wait for the memcard
                m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
            }
            else
            {
                // load the selected profile

                // init the pending profile
                g_StateMgr.InitPendingProfile( 0 ); // always player 0 in campaign

                // get the profile list
                xarray<profile_info*>& ProfileNames = g_StateMgr.GetProfileList();

                // store the id of the selected profile
                g_StateMgr.SetSelectedProfile( 0, ProfileNames[index]->Hash );

                // attempt to load the selected profile
                m_iCard = ProfileNames[index]->CardID;
                g_UIMemCardMgr.LoadProfile( *ProfileNames[index], 0, this, &dlg_profile_select::OnLoadProfileCB );

                // change the dialog state to wait for the memcard
                m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
            }
        }
        else
        {
            // create a new profile           
            if( m_Type == PROFILE_SELECT_OVERWRITE )
            {
                // calculate the hash string for the new profile
                player_profile& NewProfile = g_StateMgr.GetPendingProfile();
                NewProfile.SetHash();

                // store the id of the selected profile
                g_StateMgr.SetSelectedProfile( g_StateMgr.GetPendingProfileIndex(), NewProfile.GetHash() );

                // save the profile data in memory to a new dir on the memcard
                m_State = DIALOG_STATE_CREATE;
            }
            else
            {
                // set up the new profile data
                m_State = DIALOG_STATE_POPUP;

                // set the profile up with default settings
                g_StateMgr.ResetProfile( 0 );

                // init the pending profile for player 0
                g_StateMgr.InitPendingProfile( 0 ); 

                // clear the selected profile
                g_StateMgr.ClearSelectedProfile( 0 );

                // Xbox intercepts this keypress so it can prompt the user
                // to go to the dashboard to free up space.
                #ifdef TARGET_XBOX
                {
                    MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition(0);
                    if( Condition.BytesFree < g_StateMgr.GetProfileSaveSize() )
                    {
                        // open confirmation dialog
                        irect r = g_UiMgr->GetUserBounds( g_UiUserID );
                        m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
                        m_PopUpType = POPUP_XBOX_FREE_MORE_BLOCKS;

                        // set nav text
                        xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_DONT_FREE_BLOCKS" ));
                        navText += g_StringTableMgr( "ui", "IDS_NAV_FREE_MORE_BLOCKS" );
                        m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

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
                                            TRUE, 
                                            FALSE, 
                                            xwstring( xfs( (const char*)xstring(g_StringTableMgr( "ui", "MC_NOT_ENOUGH_FREE_SPACE_SLOT1_XBOX" )), m_BlocksRequired ) ),
                                            navText,
                                            &m_PopUpResult );
                        return;
                    }
                }
                #endif

                // open a VK to enter the profile name
                irect   r = m_pManager->GetUserBounds( m_UserID );
                ui_dlg_vkeyboard* pVKeyboard = (ui_dlg_vkeyboard*)m_pManager->OpenDialog( m_UserID, "ui_vkeyboard", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
                pVKeyboard->Configure( TRUE );
#ifdef TARGET_XBOX
                pVKeyboard->ConfigureForProfile();
#endif
                pVKeyboard->SetLabel( g_StringTableMgr( "ui", "IDS_PROFILE_CREATE" ) );
                pVKeyboard->ConnectString( &m_ProfileName, SM_PROFILE_NAME_LENGTH );
                pVKeyboard->SetReturn( &m_ProfileEntered, &m_ProfileOk );

                // update nav text
                xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
                navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
                navText += g_StringTableMgr( "ui", "IDS_NAV_DELETE" );
                m_pNavText->SetLabel( navText );
            }
        }
    }
}

//=========================================================================

void dlg_profile_select::OnLBDown( ui_win* pWin )
{
    OnPadSelect( pWin );
}

//=========================================================================

void dlg_profile_select::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( m_Type == PROFILE_SELECT_OVERWRITE )
        {
            // no backing out in this state
            return;
        }

        // wait for pending memcard ops
        while( !g_UIMemCardMgr.IsActionDone() )
        {
            g_UIMemCardMgr.Update( 0.001f );
            x_DelayThread( 1 );
        }

#ifdef TARGET_PS2
        // check for backing out during online connect phase
        if( g_StateMgr.GetState() == SM_ONLINE_PROFILE_SELECT )
        {
            CreateBackupPopup();
            return;
        }
#endif
        // Clear the poll callback
        g_AudioMgr.Play("Backup");
        g_UIMemCardMgr.ClearCallback();
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_profile_select::OnPadDelete( ui_win* pWin )
{
    (void)pWin;
    // delete the profile

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        // get the profile index from the list
        s32 index = m_pProfileList->GetSelectedItemData();

        // make sure the profile is valid
        if( index < m_CreateIndex )
        {
            // open delete confirmation dialog
            irect r = g_UiMgr->GetUserBounds( g_UiUserID );
            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
            m_PopUpType = POPUP_TYPE_DELETE;

            // set nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);
            

            m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_PROFILE_DELETE" ), 
                                TRUE, 
                                TRUE, 
                                FALSE, 
                                g_StringTableMgr( "ui", "IDS_PROFILE_DELETE_MSG" ),
                                navText,
                                &m_PopUpResult );
        }
        else
        {
            g_AudioMgr.Play( "InvalidEntry" );
        }
    }
}

//=========================================================================

void dlg_profile_select::OnPadActivate( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        switch( m_Type )
        {
            // continue without saving
            case PROFILE_SELECT_OVERWRITE:
            {
                // Clear the poll callback
                g_UIMemCardMgr.ClearCallback();
                // flag the profile as not saved
                g_StateMgr.SetProfileNotSaved( g_StateMgr.GetPendingProfileIndex(), TRUE ); 
                // Continue without saving
                g_StateMgr.ActivatePendingProfile();
                m_State = DIALOG_STATE_ACTIVATE;            
            }
            break;

            // load profile to edit
            case PROFILE_SELECT_MANAGE:
            {
                // check for bad profile
                if( m_pProfileList->GetSelectedItemData( 1 ) != PROFILE_OK )
                {
                    g_AudioMgr.Play( "InvalidEntry" );
                    return;
                }

                // get the profile index from the list
                s32 index = m_pProfileList->GetSelectedItemData();

                // check if this is a new profile
                if( index < m_CreateIndex )
                {
                    // edit the selected profile
                    m_bEditProfile = TRUE;

                    // init the pending profile
                    g_StateMgr.InitPendingProfile( 0 ); // always player 0 in campaign

                    // get the profile list
                    xarray<profile_info*>& ProfileNames = g_StateMgr.GetProfileList();

                    // store the id of the selected profile
                    g_StateMgr.SetSelectedProfile( 0, ProfileNames[index]->Hash );

                    // attempt to load the selected profile
                    m_iCard = ProfileNames[index]->CardID;
                    g_UIMemCardMgr.LoadProfile( *ProfileNames[index], 0, this, &dlg_profile_select::OnLoadProfileCB );

                    // change the dialog state to wait for the memcard
                    m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
                }
                else
                {
                    // can't edit the create option!
                    g_AudioMgr.Play( "InvalidEntry" );
                    return;
                }
            }
            break;
        }
    }
}

//=========================================================================

void dlg_profile_select::OnPollReturn( void )
{
}

//=========================================================================

void dlg_profile_select::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // turn on the controls
            m_pProfileList      ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pProfileDetails   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pCardSlot         ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pCreationDate     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pModifiedDate     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pInfoCreationDate ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pInfoModifiedDate ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText          ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            // Xbox requires slightly different messaging
            #ifdef TARGET_XBOX
            {
                s32 index = m_pProfileList->GetSelectedItemData();
                if( index >= m_CreateIndex )
                {
                    m_pCreationDate    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
                    m_pModifiedDate    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
                    m_pInfoCreationDate->SetFlag(ui_win::WF_VISIBLE, FALSE);
                    m_pInfoModifiedDate->SetFlag(ui_win::WF_VISIBLE, FALSE);
                }
            }
            #endif

            GotoControl( (ui_control*)m_pProfileList );
            g_UiMgr->SetScreenHighlight( m_pProfileList->GetPosition() );
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    // check for profile name entry
    if( m_ProfileEntered )
    {
        m_ProfileEntered = FALSE;
    
        if( m_ProfileOk )
        {
            m_ProfileOk = FALSE;
            
            // check for duplicate name entry
            for( s32 p=0; p<m_pProfileList->GetItemCount(); p++ )
            {
                if( x_wstrcmp( m_pProfileList->GetItemLabel(p), m_ProfileName ) == 0 )
                {
                    // open duplicate name error popup
                    irect r = g_UiMgr->GetUserBounds( g_UiUserID );
                    m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
                    m_PopUpType = POPUP_TYPE_BADNAME;

                    // set nav text
                    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_OK" ));
                    m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

                    m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_PROFILE_DUPLICATE_NAME" ), 
                        TRUE, 
                        FALSE, 
                        FALSE, 
                        g_StringTableMgr( "ui", "IDS_PROFILE_DUPLICATE_NAME_MSG" ),
                        navText,
                        &m_PopUpResult );

                    return;
                }
            }

            // store the new profile name
            g_StateMgr.InitPendingProfile( 0 );
            player_profile& NewProfile = g_StateMgr.GetPendingProfile();
            NewProfile.SetProfileName( xstring(m_ProfileName) );

            // go to the profile options screen
            m_State = DIALOG_STATE_ACTIVATE;
            return;
        }
        else
        {
            // re-enable dialog
            m_State = DIALOG_STATE_ACTIVE;
        }
    }

    // get the profile index from the list
    s32 index = m_pProfileList->GetSelectedItemData();

    if ( m_PopUp )
    {
        if ( m_PopUpResult != DLG_POPUP_IDLE )
        {
            switch( m_PopUpType )
            {
                ///////////////////////////////////////////////////////////////

                case POPUP_TYPE_DELETE:
                {
                    if ( m_PopUpResult == DLG_POPUP_YES )
                    {
                        // get the profile list
                        xarray<profile_info*>& ProfileNames = g_StateMgr.GetProfileList();

                        // Bye bye profile, bye bye!
                        m_iCard = ProfileNames[index]->CardID;
                        g_UIMemCardMgr.DeleteProfile( *ProfileNames[index], this, &dlg_profile_select::OnDeleteProfileCB );

                        // change the dialog state to wait for the memcard
                        m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
                    }
                    break;
                }

                case POPUP_TYPE_BADNAME:
                    // re-enable dialog
                    m_State = DIALOG_STATE_ACTIVE;
                    break;

                ///////////////////////////////////////////////////////////////

                #ifdef TARGET_XBOX

                case POPUP_TYPE_DAMAGED_PROFILE:
                    // nothing to do
                    break;

                case POPUP_XBOX_FREE_MORE_BLOCKS:
                {
                    if( m_PopUpResult == DLG_POPUP_YES )
                    {
                        // open a VK to enter the profile name
                        irect r = m_pManager->GetUserBounds( m_UserID );
                        ui_dlg_vkeyboard* pVKeyboard = (ui_dlg_vkeyboard*)m_pManager->OpenDialog( m_UserID, "ui_vkeyboard", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
                        pVKeyboard->Configure( TRUE );
#ifdef TARGET_XBOX
                        pVKeyboard->ConfigureForProfile();
#endif
                        pVKeyboard->SetLabel( g_StringTableMgr( "ui", "IDS_PROFILE_CREATE" ) );
                        pVKeyboard->ConnectString( &m_ProfileName, SM_PROFILE_NAME_LENGTH );
                        pVKeyboard->SetReturn( &m_ProfileEntered, &m_ProfileOk );

                        // update nav text
                        xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
                        navText += g_StringTableMgr( "ui", "IDS_NAV_DELETE" );
                        navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
                        if( m_Type == PROFILE_SELECT_MANAGE )
                        {
                            navText += g_StringTableMgr( "ui", "IDS_NAV_EDIT" );
                        }
                        m_pNavText->SetLabel( navText );
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
                    break;
                }
                #endif

                default:
                    ASSERT(0);
                    break;
            }

            // clear popup 
            m_PopUp = NULL;

            // turn on nav text
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
    }
    else if( m_BackupPopup )
    {
        UpdateBackupPopup();
    }
    else
    {
        if( m_State == DIALOG_STATE_ACTIVE )
        {
            if( CONFIG_IS_AUTOCLIENT || CONFIG_IS_AUTOSERVER )
            {
                if( m_pProfileList->GetItemCount() )
                {
                    g_StateMgr.ResetProfile( 0 );
                    g_StateMgr.InitPendingProfile(0);
                    // copy the data read into the profile
                    g_StateMgr.ActivatePendingProfile();

                    // tell the state mgr that we finished
                    m_State = DIALOG_STATE_SELECT;
                }
            }

            // poll the memcards in both slots
            g_UIMemCardMgr.Poll( SM_CARDMODE_PROFILE, this,&dlg_profile_select::OnPollReturn );

            // update the profile lists
            RefreshProfileList();

            // get current index
            index = m_pProfileList->GetSelectedItemData();

            // make sure we're not in the vkeyboard before changing the nav text.
            if( (dlg_profile_select*)m_pManager->GetTopmostDialog( g_UiUserID ) == this )
            {
                // check if this is a new profile
                if( index < m_CreateIndex )
                {
                    // update nav text
                    switch( m_Type )
                    {
                        case PROFILE_SELECT_OVERWRITE:
                        {
                            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SAVE_CHANGES" ));
                            navText += g_StringTableMgr( "ui", "IDS_NAV_DELETE" );
                            navText += g_StringTableMgr( "ui", "IDS_NAV_CONT_NO_SAVE" );
                            m_pNavText->SetLabel( navText );
                        }
                        break;

                        case PROFILE_SELECT_NORMAL:
                        {
                            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
                            navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
                            navText += g_StringTableMgr( "ui", "IDS_NAV_DELETE" );
                            m_pNavText->SetLabel( navText );
                        }
                        break;

                        case PROFILE_SELECT_MANAGE:
                        {
                            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
                            navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
                            navText += g_StringTableMgr( "ui", "IDS_NAV_DELETE" );
                            navText += g_StringTableMgr( "ui", "IDS_NAV_EDIT" );
                            m_pNavText->SetLabel( navText );
                        }
                        break;
                    }
                }
                else
                {
                    // update nav text
                    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));

                    if( m_Type == PROFILE_SELECT_OVERWRITE )
                    {
                        navText += g_StringTableMgr( "ui", "IDS_NAV_CONT_NO_SAVE" );
                    }
                    else
                    {
                        navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
                    }

                    m_pNavText->SetLabel( navText );
                }
            }
        }
    }
}

//=========================================================================
void dlg_profile_select::RefreshProfileList( void )
{
    xbool Found = FALSE;
    u32 ProfileHashToSelect;

    // get the hash for the selected profile
    if( m_Type == PROFILE_SELECT_OVERWRITE )
    {
        ProfileHashToSelect = g_StateMgr.GetSelectedProfile( g_StateMgr.GetPendingProfileIndex() );
    }
    else
    {
        ProfileHashToSelect = g_StateMgr.GetSelectedProfile( 0 );
    }

    // get the profile list
    xarray<profile_info*>& ProfileNames = g_StateMgr.GetProfileList();
    // store the current selection
    s32 CurrentSelection = m_pProfileList->GetSelection();

    // clear the list
    m_pProfileList->DeleteAllItems();
    // get the current list from the memcard manager
    g_UIMemCardMgr.GetProfileNames( ProfileNames );
    // fill it with the profile information
    for (s32 i=0; i<ProfileNames.GetCount(); i++)
    {
        s32 ProfileState = PROFILE_OK;

        if( ProfileNames[i]->bDamaged )
        {
            ProfileState = PROFILE_CORRUPT;
        }
        else if ( ProfileNames[i]->Ver != PROFILE_VERSION_NUMBER )
        {
#ifdef RETAIL
            ProfileState = PROFILE_CORRUPT;
#else
            ProfileState = PROFILE_EXPIRED;
#endif
        }

        if( ProfileState != PROFILE_OK )
        {
            m_pProfileList->AddItem( g_StringTableMgr( "ui", "IDS_CORRUPT" ), i, ProfileState );
            m_pProfileList->SetItemColor( i, XCOLOR_RED );
        }
        else
        {
            m_pProfileList->AddItem( ProfileNames[i]->Name, i, ProfileState );
        }

        // look for a match for the selected profile hash
        if( ProfileHashToSelect != 0 )
        {
            if( ProfileNames[i]->Hash == ProfileHashToSelect )
            {
                if( CurrentSelection == -1 )
                    CurrentSelection = i;
                Found = TRUE;
            }
        }
    }

    // add a create option
    m_CreateIndex = ProfileNames.GetCount();
    m_pProfileList->AddItem( g_StringTableMgr("ui", "IDS_PROFILE_CREATE_NEW"), m_CreateIndex );

    // determine if profile selected
    {
        if( ( CurrentSelection >= 0 ) && (CurrentSelection < m_pProfileList->GetItemCount()) )
        {
            m_pProfileList->SetSelection( CurrentSelection );
        }
        else
        {
            m_pProfileList->SetSelection( 0 );
        }
    }

    // populate profile info box based on current selection
    s32 SelIndex = m_pProfileList->GetSelection();

    if( SelIndex == m_CreateIndex )
    {
        // Handle display for "Create New" option
        m_pProfileDetails->SetLabel( g_StringTableMgr( "ui", "IDS_PROFILE_CREATE_NEW" ) );

#if defined(TARGET_XBOX)
            m_pProfileDetails->SetLabel( g_StringTableMgr( "ui", "IDS_HARD_DISK_TITLE" ) );
            MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition(0);
            u32 nBlocksFree = (u32)(Condition.BytesFree/16384);

            if( nBlocksFree <= 50000 )
            {
                xwstring Msg( xfs("%d ",nBlocksFree) );
                if (nBlocksFree == 1)
                    Msg += g_StringTableMgr( "ui", "IDS_XBOX_LIVE_BLOCK_FREE" );
                else
                    Msg += g_StringTableMgr( "ui", "IDS_XBOX_LIVE_BLOCKS_FREE" );
                m_pCardSlot->SetLabel( Msg );
            }
            else
            {
                xwstring Msg( xfs("50000+ ") );
                Msg += g_StringTableMgr( "ui", "IDS_XBOX_LIVE_BLOCKS_FREE" );
                m_pCardSlot->SetLabel( Msg );
            }
            m_pCreationDate     ->SetFlag(WF_VISIBLE, FALSE);
            m_pModifiedDate     ->SetFlag(WF_VISIBLE, FALSE);
            m_pInfoCreationDate ->SetFlag(WF_VISIBLE, FALSE);
            m_pInfoModifiedDate ->SetFlag(WF_VISIBLE, FALSE);

#elif defined(TARGET_PC)
            m_pProfileDetails->SetLabel( g_StringTableMgr( "ui", "IDS_PROFILE_INFO" ) );
            m_pCardSlot         ->SetLabel( g_StringTableMgr( "ui", "IDS_PROFILE_SAVE_LOCATION_PC" ) ); //TEMP
            m_pCreationDate     ->SetFlag(WF_VISIBLE, FALSE);
            m_pModifiedDate     ->SetFlag(WF_VISIBLE, FALSE);
            m_pInfoCreationDate ->SetFlag(WF_VISIBLE, FALSE);
            m_pInfoModifiedDate ->SetFlag(WF_VISIBLE, FALSE);
#else
            m_pProfileDetails->SetLabel( g_StringTableMgr( "ui", "IDS_PROFILE_INFO" ) );
            m_pCardSlot         ->SetLabel( g_StringTableMgr( "ui", "IDS_NULL" ) );
            m_pCreationDate     ->SetFlag(WF_VISIBLE, TRUE);
            m_pModifiedDate     ->SetFlag(WF_VISIBLE, TRUE);
            m_pInfoCreationDate ->SetFlag(WF_VISIBLE, TRUE);
            m_pInfoModifiedDate ->SetFlag(WF_VISIBLE, TRUE);
            m_pInfoCreationDate ->SetLabel( xwstring(L"---") );
            m_pInfoModifiedDate ->SetLabel( xwstring(L"---") );
#endif
    }
    else // Displaying info for an existing profile
    {
        if( SelIndex >= 0 && SelIndex < ProfileNames.GetCount() )
        {
            m_pCreationDate     ->SetFlag(WF_VISIBLE, TRUE);
            m_pModifiedDate     ->SetFlag(WF_VISIBLE, TRUE);
            m_pInfoCreationDate ->SetFlag(WF_VISIBLE, TRUE);
            m_pInfoModifiedDate ->SetFlag(WF_VISIBLE, TRUE);

            m_pProfileDetails->SetLabel( g_StringTableMgr( "ui", "IDS_PROFILE_INFO" ) );

#if defined(TARGET_XBOX)
                m_pCardSlot->SetLabel( g_StringTableMgr( "ui", "IDS_HARD_DISK_TITLE" ) );
#elif defined(TARGET_PC)
                m_pCardSlot->SetLabel( g_StringTableMgr( "ui", "IDS_PROFILE_SAVE_LOCATION_PC" ) ); //TEMP
#else 
                if( ProfileNames[SelIndex]->CardID == 0 )
                {
                    m_pCardSlot     ->SetLabel( g_StringTableMgr( "ui", "IDS_PROFILE_CARD_SLOT_1" ) );
                }
                else
                {
                    m_pCardSlot     ->SetLabel( g_StringTableMgr( "ui", "IDS_PROFILE_CARD_SLOT_2" ) );
                }
#endif

            split_date TimeStamp;
            const xwchar* Month;

            // Creation Date
#ifdef TARGET_PS2
            TimeStamp = eng_SplitJSTDate( ProfileNames[SelIndex]->CreationDate );
#else
            TimeStamp = eng_SplitDate( ProfileNames[SelIndex]->CreationDate );
#endif
            Month = g_StringTableMgr( "ui", (const char*)xfs("IDS_MONTH%d", TimeStamp.Month));
            xwstring CreateStamp(xfs("%02i:%02i:%02i ",TimeStamp.Hour, TimeStamp.Minute, TimeStamp.Second));
            CreateStamp += Month;
            CreateStamp += (const char*)xfs(" %02i, %d", TimeStamp.Day, TimeStamp.Year);
            m_pInfoCreationDate->SetLabel(CreateStamp);

            // Modification Date
#ifdef TARGET_PS2
            TimeStamp = eng_SplitJSTDate( ProfileNames[SelIndex]->ModifiedDate );
#else
            TimeStamp = eng_SplitDate( ProfileNames[SelIndex]->ModifiedDate );
#endif
            Month = g_StringTableMgr( "ui", (const char*)xfs("IDS_MONTH%d", TimeStamp.Month));
            xwstring ModStamp(xfs("%02i:%02i:%02i ",TimeStamp.Hour, TimeStamp.Minute, TimeStamp.Second));
            ModStamp += Month;
            ModStamp += (const char*)xfs(" %02i, %d", TimeStamp.Day, TimeStamp.Year);
            m_pInfoModifiedDate->SetLabel(ModStamp);
        }
        else
        {
             m_pProfileDetails->SetLabel( g_StringTableMgr( "ui", "IDS_ERROR" ) );
             m_pCardSlot         ->SetLabel( L"" );
             m_pInfoCreationDate ->SetLabel( L"" );
             m_pInfoModifiedDate ->SetLabel( L"" );
        }
    }
}

//=========================================================================

void dlg_profile_select::OnLoadProfileCB( void )
{
#ifdef TARGET_PS2
    MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( m_iCard );
#else
    MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( 0 );
#endif

    // if the load was successful
    if( Condition.SuccessCode )
    {
        // copy the data read into the profile
        g_StateMgr.SetProfileNotSaved( g_StateMgr.GetPendingProfileIndex(), FALSE );
        g_StateMgr.ActivatePendingProfile();

        // tell the state manager that we finished
        if( m_bEditProfile )
        {
            m_State = DIALOG_STATE_EDIT;
        }
        else
        {
            m_State = DIALOG_STATE_SELECT;
        }
    }
    else
    {
        // else we just continue with the status quo of polling the memory cards
        m_bEditProfile = FALSE;
        g_StateMgr.SetProfileNotSaved( g_StateMgr.GetPendingProfileIndex(), TRUE );
        g_StateMgr.ClearSelectedProfile(g_StateMgr.GetPendingProfileIndex() );

        // refresh the profile selection LB
        RefreshProfileList();

        // re-enable dialog
        m_State = DIALOG_STATE_ACTIVE;
    }
}

//=========================================================================

void dlg_profile_select::OnDeleteProfileCB( void )
{
#ifdef TARGET_PS2
    MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( m_iCard );
#else
    MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( 0 );
#endif

    // if the delete was successful
    if( Condition.SuccessCode )
    {
        // delete success - go back to polling
        m_State = DIALOG_STATE_ACTIVE;
    }
    else
    {
        // delete failed - go back to polling
        m_State = DIALOG_STATE_ACTIVE;
    }

    // turn off autosave and reset profile selection
    g_StateMgr.ClearSelectedProfile( 0 );
    g_StateMgr.SetProfileNotSaved( 0, TRUE ); 

    // refresh the profile selection LB
    RefreshProfileList();
}

//=========================================================================

void dlg_profile_select::OnSaveProfileCB( void )
{
#ifdef TARGET_PS2
    MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( m_iCard );
#else
    MemCardMgr::condition& Condition = g_UIMemCardMgr.GetCondition( 0 );
#endif

    // if the save was successful (OR user wants to continue without saving)
    if( Condition.SuccessCode )
    {
        if( Condition.bCancelled )
        {
            // clear selected profile
            g_StateMgr.ClearSelectedProfile( 0 );
            g_StateMgr.SetProfileNotSaved( g_StateMgr.GetPendingProfileIndex(), TRUE ); 

            // return to polling
            m_State = DIALOG_STATE_ACTIVE;
        }
        else
        {
            // update the changes in the profile
            g_StateMgr.SetProfileNotSaved( g_StateMgr.GetPendingProfileIndex(), FALSE ); 
            g_StateMgr.ActivatePendingProfile();

            // save successful - onward to load game
            g_AudioMgr.Play( "Select_Norm" );
            m_State = DIALOG_STATE_SELECT;            
        }
    }
    else
    {
        // clear selected profile
        g_StateMgr.ClearSelectedProfile( 0 );
        g_StateMgr.SetProfileNotSaved( g_StateMgr.GetPendingProfileIndex(), TRUE ); 

        // refresh the profile selection LB
        RefreshProfileList();

        // return to polling
        m_State = DIALOG_STATE_ACTIVE;
    }

    // get the profile list
    xarray<profile_info*>& ProfileNames = g_StateMgr.GetProfileList();
    // get the current list from the memcard manager
    g_UIMemCardMgr.GetProfileNames( ProfileNames );
}

//=========================================================================

void dlg_profile_select::UpdateBackupPopup(void)
{
    if ( m_BackupPopupResult != DLG_POPUP_IDLE )
    {
        if( m_BackupPopupResult == DLG_POPUP_NO )
        {
            // stay in this dialog
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);

        }
        else if ( m_BackupPopupResult == DLG_POPUP_YES )
        {
            // Clear the poll callback
            g_UIMemCardMgr.ClearCallback();
            m_State = DIALOG_STATE_BACK;
        }
    }
}

//=========================================================================

void dlg_profile_select::CreateBackupPopup( void )
{
    // open duplicate name error popup
    irect r = g_UiMgr->GetUserBounds( g_UiUserID );
    m_BackupPopup = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

    // set nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
    navText += g_StringTableMgr("ui", "IDS_NAV_NO" );
    m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_BackupPopup->Configure( g_StringTableMgr( "ui", "IDS_NETWORK_POPUP" ), TRUE, TRUE, FALSE, g_StringTableMgr( "ui", "IDS_ONLINE_DISCONNECT" ), navText, &m_BackupPopupResult );
}

//=========================================================================
