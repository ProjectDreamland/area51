//=========================================================================
//
//  dlg_multi_player.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_dlg_vkeyboard.hpp"

#include "dlg_MultiPlayerXBOX.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "MemCardMgr/MemCardMgr.hpp"
#include "Configuration/GameConfig.hpp"

extern xbool g_bControllerCheck;

//=========================================================================
//  Main Menu Dialog
//=========================================================================

ui_manager::control_tem MultiPlayerControls[] = 
{
    { IDC_MULTI_PLAYER_ONE_DETAILS,   "IDS_PLAYER_ONE",   "blankbox",   40,  40, 220, 150, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MULTI_PLAYER_TWO_DETAILS,   "IDS_PLAYER_TWO",   "blankbox",  276,  40, 220, 150, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MULTI_PLAYER_THREE_DETAILS, "IDS_PLAYER_THREE", "blankbox",   40, 206, 220, 150, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MULTI_PLAYER_FOUR_DETAILS,  "IDS_PLAYER_FOUR",  "blankbox",  276, 206, 220, 150, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_MULTI_PLAYER_ONE_COMBO,     "IDS_NULL",         "combo",      50,  80, 200,  40, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MULTI_PLAYER_TWO_COMBO,     "IDS_NULL",         "combo",     286,  80, 200,  40, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MULTI_PLAYER_THREE_COMBO,   "IDS_NULL",         "combo",      50, 246, 200,  40, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MULTI_PLAYER_FOUR_COMBO,    "IDS_NULL",         "combo",     286, 246, 200,  40, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_MULTI_PLAYER_ONE_TEXT,      "IDS_NULL",         "text",       50, 120, 200,  40, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MULTI_PLAYER_TWO_TEXT,      "IDS_NULL",         "text",      286, 120, 200,  40, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MULTI_PLAYER_THREE_TEXT,    "IDS_NULL",         "text",       50, 286, 200,  40, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MULTI_PLAYER_FOUR_TEXT,     "IDS_NULL",         "text",      286, 286, 200,  40, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_MULTI_PLAYER_NAV_TEXT,      "IDS_NULL",         "text",        0,   0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem MultiPlayerDialog =
{
    "IDS_MULTI_PLAYER",
    1, 9,
    sizeof(MultiPlayerControls)/sizeof(ui_manager::control_tem),
    &MultiPlayerControls[0],
    0
};

//=========================================================================
//  Defines
//=========================================================================

enum popup_type
{
    POPUP_TYPE_BADNAME,
    POPUP_TYPE_DAMAGED_PROFILE,
    POPUP_XBOX_FREE_MORE_BLOCKS,
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

void dlg_multi_player_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "multiplayer menu", &MultiPlayerDialog, &dlg_multi_player_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_multi_player_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_multi_player* pDialog = new dlg_multi_player;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_multi_player
//=========================================================================

dlg_multi_player::dlg_multi_player( void )
{
}

//=========================================================================

dlg_multi_player::~dlg_multi_player( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_multi_player::Create( s32                        UserID,
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

    m_pPlayerOneBox   = (ui_blankbox*)  FindChildByID( IDC_MULTI_PLAYER_ONE_DETAILS   );
    m_pPlayerTwoBox	  = (ui_blankbox*)  FindChildByID( IDC_MULTI_PLAYER_TWO_DETAILS   );
    m_pPlayerThreeBox = (ui_blankbox*)  FindChildByID( IDC_MULTI_PLAYER_THREE_DETAILS );
    m_pPlayerFourBox  = (ui_blankbox*)  FindChildByID( IDC_MULTI_PLAYER_FOUR_DETAILS  );

    m_pPlayerOneCombo   = (ui_combo*)  FindChildByID( IDC_MULTI_PLAYER_ONE_COMBO   );
    m_pPlayerTwoCombo   = (ui_combo*)  FindChildByID( IDC_MULTI_PLAYER_TWO_COMBO   );
    m_pPlayerThreeCombo = (ui_combo*)  FindChildByID( IDC_MULTI_PLAYER_THREE_COMBO );
    m_pPlayerFourCombo  = (ui_combo*)  FindChildByID( IDC_MULTI_PLAYER_FOUR_COMBO  );

    m_pPlayerOneText   = (ui_text*)  FindChildByID( IDC_MULTI_PLAYER_ONE_TEXT   );
    m_pPlayerTwoText   = (ui_text*)  FindChildByID( IDC_MULTI_PLAYER_TWO_TEXT   );
    m_pPlayerThreeText = (ui_text*)  FindChildByID( IDC_MULTI_PLAYER_THREE_TEXT );
    m_pPlayerFourText  = (ui_text*)  FindChildByID( IDC_MULTI_PLAYER_FOUR_TEXT  );
    m_pNavText         = (ui_text*)  FindChildByID( IDC_MULTI_PLAYER_NAV_TEXT   );

    // initialize text
    m_pPlayerOneText->UseSmallText( TRUE );
    m_pPlayerOneText->SetFlag(ui_win::WF_VISIBLE, FALSE); 
    m_pPlayerOneText->SetLabelColor( xcolor(255,252,204,255) );
    m_pPlayerOneText->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_SELECT") );

    m_pPlayerTwoText->UseSmallText( TRUE );
    m_pPlayerTwoText->SetFlag(ui_win::WF_VISIBLE, FALSE); 
    m_pPlayerTwoText->SetLabelColor( xcolor(255,252,204,255) );
    m_pPlayerTwoText->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_SELECT") );

    m_pPlayerThreeText->UseSmallText( TRUE );
    m_pPlayerThreeText->SetFlag(ui_win::WF_VISIBLE, FALSE); 
    m_pPlayerThreeText->SetLabelColor( xcolor(255,252,204,255) );
    m_pPlayerThreeText->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_SELECT") );

    m_pPlayerFourText->UseSmallText( TRUE );
    m_pPlayerFourText->SetFlag(ui_win::WF_VISIBLE, FALSE); 
    m_pPlayerFourText->SetLabelColor( xcolor(255,252,204,255) );
    m_pPlayerFourText->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_SELECT") );


    // initialize boxes
    m_pPlayerOneBox->SetFlag(ui_win::WF_VISIBLE, FALSE); 
    m_pPlayerOneBox->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pPlayerOneBox->SetHasTitleBar( TRUE );
    m_pPlayerOneBox->SetLabelColor( xcolor(255,252,204,255) );
    m_pPlayerOneBox->SetTitleBarColor( xcolor(19,59,14,196) );
       
    m_pPlayerTwoBox->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pPlayerTwoBox->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pPlayerTwoBox->SetHasTitleBar( TRUE );
    m_pPlayerTwoBox->SetLabelColor( xcolor(255,252,204,255) );
    m_pPlayerTwoBox->SetTitleBarColor( xcolor(19,59,14,196) );

    m_pPlayerThreeBox->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pPlayerThreeBox->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pPlayerThreeBox->SetHasTitleBar( TRUE );
    m_pPlayerThreeBox->SetLabelColor( xcolor(255,252,204,255) );
    m_pPlayerThreeBox->SetTitleBarColor( xcolor(19,59,14,196) );

    m_pPlayerFourBox->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pPlayerFourBox->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pPlayerFourBox->SetHasTitleBar( TRUE );
    m_pPlayerFourBox->SetLabelColor( xcolor(255,252,204,255) );
    m_pPlayerFourBox->SetTitleBarColor( xcolor(19,59,14,196) );
    
    // initialize profile combo boxes
    m_pPlayerOneCombo   ->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV );
    m_pPlayerTwoCombo   ->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV );
    m_pPlayerThreeCombo ->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV );
    m_pPlayerFourCombo  ->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV );

    m_pPlayerOneCombo   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pPlayerTwoCombo   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pPlayerThreeCombo ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pPlayerFourCombo  ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // initialize nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
  
    m_pNavText->SetLabel( navText );
    m_pNavText->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // copy the profile information
    RefreshProfileList();
    m_ProfileName = g_StringTableMgr("ui", "IDS_PROFILE_DEFAULT_PLAYER");
    m_ProfileEntered = FALSE;
    m_ProfileOK = FALSE;
    m_bEditProfile = FALSE;
    m_PopUp = NULL;
    m_BlocksRequired = 0;

    // set initial selection
    for (u32 p=0; p<SM_MAX_PLAYERS; p++)
    {
        m_PlayerReady[p] = FALSE;
    }
    m_pPlayerOneCombo   ->SetSelection( 0 );
    m_pPlayerTwoCombo   ->SetSelection( 0 );
    m_pPlayerThreeCombo ->SetSelection( 0 );
    m_pPlayerFourCombo  ->SetSelection( 0 );
    m_ReadyToLaunch = FALSE;

    // initialize screen scaling
    InitScreenScaling( Position );

    // disable the highlight
    g_UiMgr->DisableScreenHighlight();

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_multi_player::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_multi_player::Render( s32 ox, s32 oy )
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
}

//=========================================================================

void dlg_multi_player::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    if( m_State != DIALOG_STATE_ACTIVE )
        return;

    switch( g_UiMgr->GetActiveController() )
    {
        case 0:
        {
            // make sure the player hasn't chosen a profile yet
            if (m_pPlayerOneCombo->GetFlags() & ui_win::WF_HIGHLIGHT)
            {
                if ( pWin != (ui_win*)m_pPlayerOneCombo )
                {
                    if ((Code == ui_manager::NAV_LEFT)||(Code == ui_manager::NAV_RIGHT))
                    {
                        // change player one profile
                        m_pPlayerOneCombo->OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
                    }
                }
            }
        }
        break;

        case 1:
        {
            // make sure the player hasn't chosen a profile yet
            if (m_pPlayerTwoCombo->GetFlags() & ui_win::WF_HIGHLIGHT)
            {
                if ( pWin != (ui_win*)m_pPlayerTwoCombo )
                {
                    if ((Code == ui_manager::NAV_LEFT)||(Code == ui_manager::NAV_RIGHT))
                    {
                        // change player two profile
                        m_pPlayerTwoCombo->OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
                    }
                }
            }
        }
        break;

        case 2:
        {
            // make sure the player hasn't chosen a profile yet
            if (m_pPlayerThreeCombo->GetFlags() & ui_win::WF_HIGHLIGHT)
            {
                if ( pWin != (ui_win*)m_pPlayerThreeCombo )
                {
                    if ((Code == ui_manager::NAV_LEFT)||(Code == ui_manager::NAV_RIGHT))
                    {
                        // change player three profile
                        m_pPlayerThreeCombo->OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
                    }
                }
            }
        }
        break;

        case 3:
        {
            // make sure the player hasn't chosen a profile yet
            if (m_pPlayerFourCombo->GetFlags() & ui_win::WF_HIGHLIGHT)
            {
                if ( pWin != (ui_win*)m_pPlayerFourCombo )
                {
                    if ((Code == ui_manager::NAV_LEFT)||(Code == ui_manager::NAV_RIGHT))
                    {
                        // change player four profile
                        m_pPlayerFourCombo->OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
                    }
                }
            }
        }
        break;
    }
}

//=========================================================================

void dlg_multi_player::OnPadSelect( ui_win* pWin )
{
    (void)pWin;      

    // select a profile
    if( m_State == DIALOG_STATE_ACTIVE )
    {
        switch ( g_UiMgr->GetActiveController() )
        {
            case 0:
            {
                if (m_pPlayerOneCombo->GetFlags() & ui_win::WF_HIGHLIGHT)
                {
                    // check for bad profile
                    if( m_pPlayerOneCombo->GetSelectedItemData( 1 ) != PROFILE_OK )
                    {
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
                    }

                    // get the profile index from the list
                    s32 index = m_pPlayerOneCombo->GetSelection();

                    // check if enabled
                    if( m_pPlayerOneCombo->GetSelectedItemEnabled() == FALSE )
                    {
                        // not enabled - in use by another player
                        g_AudioMgr.Play( "InvalidEntry" );

                        // bail out early
                        return;
                    }

                    // check if this is a new profile
                    if( index < m_CreateIndex )
                    {
                        // clear the edit flag
                        m_bEditProfile = FALSE;

                        // init the pending profile for player 0
                        g_StateMgr.InitPendingProfile( 0 ); 

                        // get the profile name array
                        xarray<profile_info*>& ProfileNames = g_StateMgr.GetProfileList();

                        // store the id of the selected profile
                        g_StateMgr.SetSelectedProfile( 0, ProfileNames[index]->Hash );

                        // attempt to load the profile
                        g_UIMemCardMgr.LoadProfile( *ProfileNames[index], 0, this, &dlg_multi_player::OnLoadProfileCB );

                        // change the dialog state to wait for the memcard
                        m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
                    }
                    else
                    {
                        // create a new profile      
                        m_CreatePlayerIndex = 0;

                        // set the profile up with default settings
                        g_StateMgr.ResetProfile( 0 );

                        // init the pending profile for player 0
                        g_StateMgr.InitPendingProfile( 0 ); 

                        // clear the selected profile
                        g_StateMgr.ClearSelectedProfile( 0 );

                        // set the default profile name
                        m_ProfileName = g_StringTableMgr("ui", "IDS_PROFILE_DEFAULT_PLAYER");

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

                                // tag the controller as being in use.
                                g_StateMgr.SetControllerRequested(0, TRUE);
                                m_State = DIALOG_STATE_POPUP;
                                return;
                            }
                        }
#endif

                        // open a VK to enter the profile name
                        irect   r = m_pManager->GetUserBounds( m_UserID );
                        ui_dlg_vkeyboard* pVKeyboard = (ui_dlg_vkeyboard*)m_pManager->OpenDialog( m_UserID, "ui_vkeyboard", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
                        pVKeyboard->Configure( TRUE );
                        pVKeyboard->ConfigureForProfile();
                        pVKeyboard->SetLabel( g_StringTableMgr( "ui", "IDS_PROFILE_CREATE" ) );
                        pVKeyboard->ConnectString( &m_ProfileName, SM_PROFILE_NAME_LENGTH );
                        pVKeyboard->SetReturn( &m_ProfileEntered, &m_ProfileOK );

                        m_State = DIALOG_STATE_POPUP;
                    }
                    // tag the controller as being in use.
                    g_StateMgr.SetControllerRequested(0, TRUE);
                }
                else
                {
                    // already selected
                    return;
                }
            }
            break;

            case 1:
            {
                if (m_pPlayerTwoCombo->GetFlags() & ui_win::WF_HIGHLIGHT)
                {
                    // check for bad profile
                    if( m_pPlayerTwoCombo->GetSelectedItemData( 1 ) != PROFILE_OK )
                    {
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
                    }

                    // get the profile index from the list
                    s32 index = m_pPlayerTwoCombo->GetSelection();

                    // check if enabled
                    if( m_pPlayerTwoCombo->GetSelectedItemEnabled() == FALSE )
                    {
                        // not enabled - in use by another player
                        g_AudioMgr.Play( "InvalidEntry" );

                        // bail out early
                        return;
                    }

                    // check if this is a new profile
                    if( index < m_CreateIndex )
                    {
                        // clear the edit flag
                        m_bEditProfile = FALSE;

                        // init the pending profile for player 1
                        g_StateMgr.InitPendingProfile( 1 ); 

                        // get the profile name array
                        xarray<profile_info*>& ProfileNames = g_StateMgr.GetProfileList();

                        // store the id of the selected profile
                        g_StateMgr.SetSelectedProfile( 1, ProfileNames[index]->Hash );

                        // attempt to load the profile
                        g_UIMemCardMgr.LoadProfile( *ProfileNames[index], 1, this, &dlg_multi_player::OnLoadProfileCB );

                        // change the dialog state to wait for the memcard
                        m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
                    }
                    else
                    {
                        // create a new profile      
                        m_CreatePlayerIndex = 1;

                        // set the profile up with default settings
                        g_StateMgr.ResetProfile( 1 );

                        // init the pending profile for player 1
                        g_StateMgr.InitPendingProfile( 1 ); 

                        // clear the selected profile
                        g_StateMgr.ClearSelectedProfile( 1 );

                        // set the default profile name
                        m_ProfileName = g_StringTableMgr("ui", "IDS_PROFILE_DEFAULT_PLAYER2");

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

                                // tag the controller as being in use.
                                g_StateMgr.SetControllerRequested(1, TRUE);
                                m_State = DIALOG_STATE_POPUP;
                                return;
                            }
                        }
#endif
                        // open a VK to enter the profile name
                        irect   r = m_pManager->GetUserBounds( m_UserID );
                        ui_dlg_vkeyboard* pVKeyboard = (ui_dlg_vkeyboard*)m_pManager->OpenDialog( m_UserID, "ui_vkeyboard", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
                        pVKeyboard->SetLabel( g_StringTableMgr( "ui", "IDS_PROFILE_CREATE" ) );
                        pVKeyboard->ConfigureForProfile();
                        pVKeyboard->ConnectString( &m_ProfileName, SM_PROFILE_NAME_LENGTH );
                        pVKeyboard->SetReturn( &m_ProfileEntered, &m_ProfileOK );
                        m_State = DIALOG_STATE_POPUP;
                    }
                    // tag the controller as being in use.
                    g_StateMgr.SetControllerRequested(1, TRUE);
                }
                else
                {
                    // already selected
                    return;
                }
            }
            break;

            case 2:
            {
                if (m_pPlayerThreeCombo->GetFlags() & ui_win::WF_HIGHLIGHT)
                {
                    // check for bad profile
                    if( m_pPlayerThreeCombo->GetSelectedItemData( 1 ) != PROFILE_OK )
                    {
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
                    }

                    // get the profile index from the list
                    s32 index = m_pPlayerThreeCombo->GetSelection();

                    // check if enabled
                    if( m_pPlayerThreeCombo->GetSelectedItemEnabled() == FALSE )
                    {
                        // not enabled - in use by another player
                        g_AudioMgr.Play( "InvalidEntry" );

                        // bail out early
                        return;
                    }

                    // check if this is a new profile
                    if( index < m_CreateIndex )
                    {
                        // clear the edit flag
                        m_bEditProfile = FALSE;

                        // init the pending profile for player 2
                        g_StateMgr.InitPendingProfile( 2 ); 

                        // get the profile name array
                        xarray<profile_info*>& ProfileNames = g_StateMgr.GetProfileList();

                        // store the id of the selected profile
                        g_StateMgr.SetSelectedProfile( 2, ProfileNames[index]->Hash );

                        // attempt to load the profile
                        g_UIMemCardMgr.LoadProfile( *ProfileNames[index], 2, this, &dlg_multi_player::OnLoadProfileCB );

                        // change the dialog state to wait for the memcard
                        m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
                    }
                    else
                    {
                        // create a new profile      
                        m_CreatePlayerIndex = 2;

                        // set the profile up with default settings
                        g_StateMgr.ResetProfile( 2 );

                        // init the pending profile for player 1
                        g_StateMgr.InitPendingProfile( 2 ); 

                        // clear the selected profile
                        g_StateMgr.ClearSelectedProfile( 2 );

                        // set the default profile name
                        m_ProfileName = g_StringTableMgr("ui", "IDS_PROFILE_DEFAULT_PLAYER3");

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

                                // tag the controller as being in use.
                                g_StateMgr.SetControllerRequested(2, TRUE);
                                m_State = DIALOG_STATE_POPUP;
                                return;
                            }
                        }
#endif
                        // open a VK to enter the profile name
                        irect   r = m_pManager->GetUserBounds( m_UserID );
                        ui_dlg_vkeyboard* pVKeyboard = (ui_dlg_vkeyboard*)m_pManager->OpenDialog( m_UserID, "ui_vkeyboard", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
                        pVKeyboard->SetLabel( g_StringTableMgr( "ui", "IDS_PROFILE_CREATE" ) );
                        pVKeyboard->ConfigureForProfile();
                        pVKeyboard->ConnectString( &m_ProfileName, SM_PROFILE_NAME_LENGTH );
                        pVKeyboard->SetReturn( &m_ProfileEntered, &m_ProfileOK );
                        m_State = DIALOG_STATE_POPUP;
                    }
                    // tag the controller as being in use.
                    g_StateMgr.SetControllerRequested(2, TRUE);
                }
                else
                {
                    // already selected
                    return;
                }
            }
            break;

            case 3:
            {
                if (m_pPlayerFourCombo->GetFlags() & ui_win::WF_HIGHLIGHT)
                {
                    // check for bad profile
                    if( m_pPlayerFourCombo->GetSelectedItemData( 1 ) != PROFILE_OK )
                    {
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
                    }

                    // get the profile index from the list
                    s32 index = m_pPlayerFourCombo->GetSelection();

                    // check if enabled
                    if( m_pPlayerFourCombo->GetSelectedItemEnabled() == FALSE )
                    {
                        // not enabled - in use by another player
                        g_AudioMgr.Play( "InvalidEntry" );

                        // bail out early
                        return;
                    }

                    // check if this is a new profile
                    if( index < m_CreateIndex )
                    {
                        // clear the edit flag
                        m_bEditProfile = FALSE;

                        // init the pending profile for player 3
                        g_StateMgr.InitPendingProfile( 3 ); 

                        // get the profile name array
                        xarray<profile_info*>& ProfileNames = g_StateMgr.GetProfileList();

                        // store the id of the selected profile
                        g_StateMgr.SetSelectedProfile( 3, ProfileNames[index]->Hash );

                        // attempt to load the profile
                        g_UIMemCardMgr.LoadProfile( *ProfileNames[index], 3, this, &dlg_multi_player::OnLoadProfileCB );

                        // change the dialog state to wait for the memcard
                        m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
                    }
                    else
                    {
                        // create a new profile      
                        m_CreatePlayerIndex = 3;

                        // set the profile up with default settings
                        g_StateMgr.ResetProfile( 3 );

                        // init the pending profile for player 1
                        g_StateMgr.InitPendingProfile( 3 ); 

                        // clear the selected profile
                        g_StateMgr.ClearSelectedProfile( 3 );

                        // set the default profile name
                        m_ProfileName = g_StringTableMgr("ui", "IDS_PROFILE_DEFAULT_PLAYER4");

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

                                // tag the controller as being in use.
                                g_StateMgr.SetControllerRequested(3, TRUE);
                                m_State = DIALOG_STATE_POPUP;
                                return;
                            }
                        }
#endif
                        // open a VK to enter the profile name
                        irect   r = m_pManager->GetUserBounds( m_UserID );
                        ui_dlg_vkeyboard* pVKeyboard = (ui_dlg_vkeyboard*)m_pManager->OpenDialog( m_UserID, "ui_vkeyboard", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
                        pVKeyboard->SetLabel( g_StringTableMgr( "ui", "IDS_PROFILE_CREATE" ) );
                        pVKeyboard->ConfigureForProfile();
                        pVKeyboard->ConnectString( &m_ProfileName, SM_PROFILE_NAME_LENGTH );
                        pVKeyboard->SetReturn( &m_ProfileEntered, &m_ProfileOK );
                        m_State = DIALOG_STATE_POPUP;
                    }
                    // tag the controller as being in use.
                    g_StateMgr.SetControllerRequested(3, TRUE);
                }
                else
                {
                    // already selected
                    return;
                }
            }
            break;
        }

        // play select sound
        g_AudioMgr.Play( "Select_Norm" );
    }
}

//=========================================================================

void dlg_multi_player::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if( m_State == DIALOG_STATE_ACTIVE )
    {
        // clear the profiles and controller requests
        // this will prevent issues associated with default profiles.
        for(s32 p=0; p < SM_MAX_PLAYERS; p++)
        {
            g_StateMgr.ClearSelectedProfile( p );
            g_StateMgr.SetControllerRequested(p, FALSE);
        }

        g_AudioMgr.Play("Backup");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_multi_player::OnPadDelete( ui_win* pWin )
{
    (void)pWin;

    // deselect a profile
    if( m_State == DIALOG_STATE_ACTIVE )
    {
        switch ( g_UiMgr->GetActiveController() )
        {
            case 0:
            {
                if (m_pPlayerOneCombo->GetFlags() & ui_win::WF_HIGHLIGHT)
                {
                    // not selected
                    return;
                }
                else
                {
                    m_pPlayerOneCombo->SetFlag( ui_win::WF_HIGHLIGHT, TRUE );
                    m_pPlayerOneText->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_SELECT") );

                    // clear the selected profile
                    g_StateMgr.ClearSelectedProfile( 0 );

                    // update the profile lists
                    RefreshProfileList();

                    // clear controller flag
                    g_StateMgr.SetControllerRequested(0, FALSE);
                }
            }
            break;

            case 1:
            {
                if (m_pPlayerTwoCombo->GetFlags() & ui_win::WF_HIGHLIGHT)
                {
                    // not selected
                    return;
                }
                else
                {
                    m_pPlayerTwoCombo->SetFlag( ui_win::WF_HIGHLIGHT, TRUE );
                    m_pPlayerTwoText->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_SELECT") );

                    // clear the selected profile
                    g_StateMgr.ClearSelectedProfile( 1 );

                    // update the profile lists
                    RefreshProfileList();

                    // clear controller flag
                    g_StateMgr.SetControllerRequested(1, FALSE);
                }
            }
            break;

            case 2:
            {
                if (m_pPlayerThreeCombo->GetFlags() & ui_win::WF_HIGHLIGHT)
                {
                    // not selected
                    return;
                }
                else
                {
                    m_pPlayerThreeCombo->SetFlag( ui_win::WF_HIGHLIGHT, TRUE );
                    m_pPlayerThreeText->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_SELECT") );

                    // clear the selected profile
                    g_StateMgr.ClearSelectedProfile( 2 );

                    // update the profile lists
                    RefreshProfileList();

                    // clear controller flag
                    g_StateMgr.SetControllerRequested(2, FALSE);
                }
            }
            break;

            case 3:
            {
                if (m_pPlayerFourCombo->GetFlags() & ui_win::WF_HIGHLIGHT)
                {
                    // not selected
                    return;
                }
                else
                {
                    m_pPlayerFourCombo->SetFlag( ui_win::WF_HIGHLIGHT, TRUE );
                    m_pPlayerFourText->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_SELECT") );

                    // clear the selected profile
                    g_StateMgr.ClearSelectedProfile( 3 );

                    // update the profile lists
                    RefreshProfileList();

                    // clear controller flag
                    g_StateMgr.SetControllerRequested(3, FALSE);
                }
            }
            break;
        }

        // play select sound
        g_AudioMgr.Play( "Select_Norm" );
    }

}

//=========================================================================

void dlg_multi_player::OnPadActivate( ui_win* pWin )
{
    (void)pWin;

    // launch game
    if( m_State == DIALOG_STATE_ACTIVE )
    {
        // make sure at least two players are ready before launching
        if ( m_ReadyToLaunch )
        {
            // how many players are there?
            u32 count = 0;
            for (u32 p=0; p<SM_MAX_PLAYERS; p++)
            {
                if ( m_PlayerReady[p] )
                {
                    count++;
                }
            }

            // set the player count
            g_PendingConfig.SetPlayerCount( count );

            // tell the state manager to move on
            m_State = DIALOG_STATE_ACTIVATE;
            g_AudioMgr.Play( "Select_Norm" );

            // reset the active controller
            g_StateMgr.SetActiveControllerID( -1 );
            // re-select the player's controller 
            for (s32 iPad = 0; iPad < SM_MAX_PLAYERS; iPad++ )
            {
                if( g_StateMgr.GetSelectedProfile(iPad) != 0 )
                {
                    g_StateMgr.SetControllerRequested(iPad, TRUE );
                }
            }
            return;

            // we're outta here!
            return;
        }
    }
}

//=========================================================================

void dlg_multi_player::OnPollReturn( void )
{
}

//=========================================================================

void dlg_multi_player::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // complete!  turn on the elements
            m_pPlayerOneBox   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pPlayerTwoBox   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pPlayerThreeBox ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pPlayerFourBox  ->SetFlag( ui_win::WF_VISIBLE, TRUE );

            m_pPlayerOneCombo   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pPlayerTwoCombo   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pPlayerThreeCombo ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pPlayerFourCombo  ->SetFlag( ui_win::WF_VISIBLE, TRUE );

            m_pPlayerOneCombo   ->SetFlag( ui_win::WF_HIGHLIGHT, TRUE );
            m_pPlayerTwoCombo   ->SetFlag( ui_win::WF_HIGHLIGHT, TRUE );
            m_pPlayerThreeCombo ->SetFlag( ui_win::WF_HIGHLIGHT, TRUE );
            m_pPlayerFourCombo  ->SetFlag( ui_win::WF_HIGHLIGHT, TRUE );
    
            m_pPlayerOneText   ->SetFlag( ui_win::WF_VISIBLE, TRUE ); 
            m_pPlayerTwoText   ->SetFlag( ui_win::WF_VISIBLE, TRUE ); 
            m_pPlayerThreeText ->SetFlag( ui_win::WF_VISIBLE, TRUE ); 
            m_pPlayerFourText  ->SetFlag( ui_win::WF_VISIBLE, TRUE ); 
            m_pNavText         ->SetFlag( ui_win::WF_VISIBLE, TRUE );
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);


    // check for profile name entry
    if( m_ProfileEntered )
    {
        m_ProfileEntered = FALSE;

        if( m_ProfileOK )
        {
            m_ProfileOK = FALSE;

            // check for duplicate name entry
            for( s32 p=0; p<m_pPlayerOneCombo->GetItemCount(); p++ )
            {
                if( x_wstrcmp( m_pPlayerOneCombo->GetItemLabel(p), m_ProfileName ) == 0 )
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
            g_StateMgr.InitPendingProfile( m_CreatePlayerIndex );
            player_profile& NewProfile = g_StateMgr.GetPendingProfile();
            NewProfile.SetProfileName( xstring(m_ProfileName) );

            // go to the MP profile options screen
            m_State = DIALOG_STATE_CREATE;
        }
        else
        {
            // Re-enable dialog
            m_State = DIALOG_STATE_ACTIVE;
        }
    }
    else if ( m_PopUp )
    {
        if ( m_PopUpResult != DLG_POPUP_IDLE )
        {
            switch( m_PopUpType )
            {
                case POPUP_TYPE_BADNAME:
                    // re-enable dialog
                    m_State = DIALOG_STATE_ACTIVE;
                    break;

                case POPUP_XBOX_FREE_MORE_BLOCKS:
                {
                    if( m_PopUpResult == DLG_POPUP_YES )
                    {
                        // open a VK to enter the profile name
                        irect   r = m_pManager->GetUserBounds( m_UserID );
                        ui_dlg_vkeyboard* pVKeyboard = (ui_dlg_vkeyboard*)m_pManager->OpenDialog( m_UserID, "ui_vkeyboard", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
                        pVKeyboard->Configure( TRUE );
                        pVKeyboard->SetLabel( g_StringTableMgr( "ui", "IDS_PROFILE_CREATE" ) );
                        pVKeyboard->ConfigureForProfile();
                        pVKeyboard->ConnectString( &m_ProfileName, SM_PROFILE_NAME_LENGTH );
                        pVKeyboard->SetReturn( &m_ProfileEntered, &m_ProfileOK );
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
                }
                break;

                default:
                    break;
            }

            // clear popup 
            m_PopUp = NULL;

            // turn on nav text
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
    }
    else
    {
        // don't update if waiting for another operation
        if( m_State != DIALOG_STATE_ACTIVE )
            return;

        // poll the memcards in both slots
        g_UIMemCardMgr.Poll( SM_CARDMODE_PROFILE, this, &dlg_multi_player::OnPollReturn );

        // update the profile lists
        RefreshProfileList();

        // check for pulled controllers
        if( g_bControllerCheck )
        {
            for (s32 p = 0; p < SM_MAX_PLAYERS; p++)
            {
                // Has a requested controller been pulled?
                if(g_StateMgr.GetSelectedProfile(p) && !input_IsPresent(INPUT_XBOX_QRY_PAD_PRESENT, p ))
                {
                    // deselect player profile
                    switch(p)
                    {
                    case 0:
                        m_pPlayerOneCombo->SetFlag( ui_win::WF_HIGHLIGHT, TRUE );
                        m_pPlayerOneText->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_SELECT") );
                        break;
                    case 1:
                        m_pPlayerTwoCombo->SetFlag( ui_win::WF_HIGHLIGHT, TRUE );
                        m_pPlayerTwoText->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_SELECT") );
                        break;
                    case 2:
                        m_pPlayerThreeCombo->SetFlag( ui_win::WF_HIGHLIGHT, TRUE );
                        m_pPlayerThreeText->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_SELECT") );
                        break;
                    case 3:
                        m_pPlayerFourCombo->SetFlag( ui_win::WF_HIGHLIGHT, TRUE );
                        m_pPlayerFourText->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_SELECT") );
                        break;
                    }

                    // clear the selected profile
                    g_StateMgr.ClearSelectedProfile( p );

                    // update the profile lists
                    RefreshProfileList();

                    // clear controller flag
                    g_StateMgr.SetControllerRequested(p, FALSE);
                }
            }
        }

        // update navigation
        u32 count = 0;
        for (u32 p=0; p<SM_MAX_PLAYERS; p++)
        {
            if ( m_PlayerReady[p] )
            {
                count++;
            }
        }

        xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
        navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
        m_ReadyToLaunch = FALSE;

        if ( count > 0 )
        {
            // display deselect option
            navText += g_StringTableMgr( "ui", "IDS_NAV_DESELECT" );
        }

        if( count > 1 )
        {
            // display continue option
            navText += g_StringTableMgr( "ui", "IDS_NAV_MP_CONTINUE" );
            m_ReadyToLaunch = TRUE;
        }

        m_pNavText->SetLabel( navText );

    }
}

//=========================================================================

void dlg_multi_player::RefreshProfileList()
{
    xbool Found1 = FALSE;
    xbool Found2 = FALSE;
    xbool Found3 = FALSE;
    xbool Found4 = FALSE;

    u32 Profile1 = g_StateMgr.GetSelectedProfile( 0 );
    u32 Profile2 = g_StateMgr.GetSelectedProfile( 1 );
    u32 Profile3 = g_StateMgr.GetSelectedProfile( 2 );
    u32 Profile4 = g_StateMgr.GetSelectedProfile( 3 );

    // get the profile list
    xarray<profile_info*>& ProfileNames = g_StateMgr.GetProfileList();

    // store the current selections
    s32 CurrentSelection1 = m_pPlayerOneCombo   ->GetSelection();
    s32 CurrentSelection2 = m_pPlayerTwoCombo   ->GetSelection();
    s32 CurrentSelection3 = m_pPlayerThreeCombo ->GetSelection();
    s32 CurrentSelection4 = m_pPlayerFourCombo  ->GetSelection();

    // clear the lists
    m_pPlayerOneCombo   ->DeleteAllItems();
    m_pPlayerTwoCombo   ->DeleteAllItems();
    m_pPlayerThreeCombo ->DeleteAllItems();
    m_pPlayerFourCombo  ->DeleteAllItems();

    // get the current list from the memcard manager
    g_UIMemCardMgr.GetProfileNames( ProfileNames );

    // fill it with the profile information
    for (s32 i=0; i<ProfileNames.GetCount(); i++)
    {
        // add the profile to the list (even if damaged - error is shown when we attempt to load it NOT before)
        m_pPlayerOneCombo   ->AddItem( ProfileNames[i]->Name, i );
        m_pPlayerTwoCombo   ->AddItem( ProfileNames[i]->Name, i );
        m_pPlayerThreeCombo ->AddItem( ProfileNames[i]->Name, i );
        m_pPlayerFourCombo  ->AddItem( ProfileNames[i]->Name, i );

        // look for match for selected profiles
        if( Profile1 != 0 )
        {
            if( ProfileNames[i]->Hash == Profile1 )
            {
                CurrentSelection1 = i;
                m_pPlayerTwoCombo   ->SetItemEnabled( i, FALSE );
                m_pPlayerThreeCombo ->SetItemEnabled( i, FALSE );
                m_pPlayerFourCombo  ->SetItemEnabled( i, FALSE );
                Found1 = TRUE;
            }
        }

        if( Profile2 != 0 )
        {
            if( ProfileNames[i]->Hash == Profile2 )
            {
                CurrentSelection2 = i;
                m_pPlayerOneCombo   ->SetItemEnabled( i, FALSE );
                m_pPlayerThreeCombo ->SetItemEnabled( i, FALSE );
                m_pPlayerFourCombo  ->SetItemEnabled( i, FALSE );
                Found2 = TRUE;
            }
        }

        if( Profile3 != 0 )
        {
            if( ProfileNames[i]->Hash == Profile3 )
            {
                CurrentSelection3 = i;
                m_pPlayerOneCombo   ->SetItemEnabled( i, FALSE );
                m_pPlayerTwoCombo   ->SetItemEnabled( i, FALSE );
                m_pPlayerFourCombo  ->SetItemEnabled( i, FALSE );
                Found3 = TRUE;
            }
        }

        if( Profile4 != 0 )
        {
            if( ProfileNames[i]->Hash == Profile4 )
            {
                CurrentSelection4 = i;
                m_pPlayerOneCombo   ->SetItemEnabled( i, FALSE );
                m_pPlayerTwoCombo   ->SetItemEnabled( i, FALSE );
                m_pPlayerThreeCombo ->SetItemEnabled( i, FALSE );
                Found4 = TRUE;
            }
        }
    }

    // get the current count
    m_CreateIndex = ProfileNames.GetCount();

    // add any profiles created that aren't on the memory card
    if(  g_StateMgr.GetSelectedProfile( 0 ) && g_StateMgr.GetProfileNotSaved( 0 ) )
    {
        m_pPlayerOneCombo   ->AddItem( g_StateMgr.GetProfileName( 0 ), m_CreateIndex );
        m_pPlayerTwoCombo   ->AddItem( g_StateMgr.GetProfileName( 0 ), m_CreateIndex );
        m_pPlayerThreeCombo ->AddItem( g_StateMgr.GetProfileName( 0 ), m_CreateIndex );
        m_pPlayerFourCombo  ->AddItem( g_StateMgr.GetProfileName( 0 ), m_CreateIndex );
        
        m_pPlayerTwoCombo   ->SetItemEnabled( m_CreateIndex, FALSE );
        m_pPlayerThreeCombo ->SetItemEnabled( m_CreateIndex, FALSE );
        m_pPlayerFourCombo  ->SetItemEnabled( m_CreateIndex, FALSE );
        
        CurrentSelection1 = m_CreateIndex;
        m_CreateIndex++;
    }
    if(  g_StateMgr.GetSelectedProfile( 1 ) && g_StateMgr.GetProfileNotSaved( 1 ) )
    {
        m_pPlayerOneCombo   ->AddItem( g_StateMgr.GetProfileName( 1 ), m_CreateIndex );
        m_pPlayerTwoCombo   ->AddItem( g_StateMgr.GetProfileName( 1 ), m_CreateIndex );
        m_pPlayerThreeCombo ->AddItem( g_StateMgr.GetProfileName( 1 ), m_CreateIndex );
        m_pPlayerFourCombo  ->AddItem( g_StateMgr.GetProfileName( 1 ), m_CreateIndex );

        m_pPlayerOneCombo   ->SetItemEnabled( m_CreateIndex, FALSE );
        m_pPlayerThreeCombo ->SetItemEnabled( m_CreateIndex, FALSE );
        m_pPlayerFourCombo  ->SetItemEnabled( m_CreateIndex, FALSE );

        CurrentSelection2 = m_CreateIndex;
        m_CreateIndex++;
    }
    if(  g_StateMgr.GetSelectedProfile( 2 ) && g_StateMgr.GetProfileNotSaved( 2 ) )
    {
        m_pPlayerOneCombo   ->AddItem( g_StateMgr.GetProfileName( 2 ), m_CreateIndex );
        m_pPlayerTwoCombo   ->AddItem( g_StateMgr.GetProfileName( 2 ), m_CreateIndex );
        m_pPlayerThreeCombo ->AddItem( g_StateMgr.GetProfileName( 2 ), m_CreateIndex );
        m_pPlayerFourCombo  ->AddItem( g_StateMgr.GetProfileName( 2 ), m_CreateIndex );

        m_pPlayerOneCombo   ->SetItemEnabled( m_CreateIndex, FALSE );
        m_pPlayerTwoCombo   ->SetItemEnabled( m_CreateIndex, FALSE );
        m_pPlayerFourCombo  ->SetItemEnabled( m_CreateIndex, FALSE );

        CurrentSelection3 = m_CreateIndex;
        m_CreateIndex++;
    }
    if(  g_StateMgr.GetSelectedProfile( 3 ) && g_StateMgr.GetProfileNotSaved( 3 ) )
    {
        m_pPlayerOneCombo   ->AddItem( g_StateMgr.GetProfileName( 3 ), m_CreateIndex );
        m_pPlayerTwoCombo   ->AddItem( g_StateMgr.GetProfileName( 3 ), m_CreateIndex );
        m_pPlayerThreeCombo ->AddItem( g_StateMgr.GetProfileName( 3 ), m_CreateIndex );
        m_pPlayerFourCombo  ->AddItem( g_StateMgr.GetProfileName( 3 ), m_CreateIndex );

        m_pPlayerOneCombo   ->SetItemEnabled( m_CreateIndex, FALSE );
        m_pPlayerTwoCombo   ->SetItemEnabled( m_CreateIndex, FALSE );
        m_pPlayerThreeCombo ->SetItemEnabled( m_CreateIndex, FALSE );

        CurrentSelection4 = m_CreateIndex;
        m_CreateIndex++;
    }

    // add a create a profile option
    m_pPlayerOneCombo   ->AddItem( g_StringTableMgr("ui", "IDS_PROFILE_CREATE_NEW"), m_CreateIndex );
    m_pPlayerTwoCombo   ->AddItem( g_StringTableMgr("ui", "IDS_PROFILE_CREATE_NEW"), m_CreateIndex );
    m_pPlayerThreeCombo ->AddItem( g_StringTableMgr("ui", "IDS_PROFILE_CREATE_NEW"), m_CreateIndex );
    m_pPlayerFourCombo  ->AddItem( g_StringTableMgr("ui", "IDS_PROFILE_CREATE_NEW"), m_CreateIndex );

    // determine if profile selected
    if( g_StateMgr.GetSelectedProfile( 0 ) )
    {
        if( Found1 )
        {
            // set the selected profile as the active profile
            m_PlayerReady[0] = TRUE;
            m_pPlayerOneCombo ->SetFlag( ui_win::WF_HIGHLIGHT, FALSE );
            m_pPlayerOneText  ->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_READY") );
            m_pPlayerOneCombo ->SetSelection( CurrentSelection1 );
        }
        else if(  g_StateMgr.GetProfileNotSaved( 0 ) )
        {
            // set the unsaved profile as the active profile
            m_PlayerReady[0] = TRUE;
            m_pPlayerOneCombo ->SetFlag( ui_win::WF_HIGHLIGHT, FALSE );
            m_pPlayerOneText  ->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_READY") );
            m_pPlayerOneCombo ->SetSelection( CurrentSelection1 );
        }
        else
        {
            // profile not found, deselect it
            m_PlayerReady[0] = FALSE;
            m_pPlayerOneCombo ->SetFlag( ui_win::WF_HIGHLIGHT, TRUE );
            m_pPlayerOneText  ->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_SELECT") );

            // clear the selected profile
            g_StateMgr.ClearSelectedProfile( 0 );

            // set default selection
            m_pPlayerOneCombo ->SetSelection( m_CreateIndex );
        }
    }
    else
    {
        // restore selection for player 1
        m_PlayerReady[0] = FALSE;
        if( ( m_pPlayerOneCombo->GetItemCount() > CurrentSelection1 ) && ( CurrentSelection1 >= 0 ) )
        {
            m_pPlayerOneCombo->SetSelection( CurrentSelection1 );
        }
        else
        {
            m_pPlayerOneCombo->SetSelection( m_CreateIndex );
        }
    }

    if( g_StateMgr.GetSelectedProfile( 1 ) )
    {
        if( Found2 )
        {
            // set the selected profile as the active profile
            m_PlayerReady[1] = TRUE;
            m_pPlayerTwoCombo ->SetFlag( ui_win::WF_HIGHLIGHT, FALSE );
            m_pPlayerTwoText  ->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_READY") );
            m_pPlayerTwoCombo ->SetSelection( CurrentSelection2 );
        }
        else if(  g_StateMgr.GetProfileNotSaved( 1 ) )
        {
            // set the unsaved profile as the active profile
            m_PlayerReady[1] = TRUE;
            m_pPlayerTwoCombo ->SetFlag( ui_win::WF_HIGHLIGHT, FALSE );
            m_pPlayerTwoText  ->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_READY") );
            m_pPlayerTwoCombo ->SetSelection( CurrentSelection2 );
        }
        else
        {
            // profile not found, deselect it
            m_PlayerReady[1] = FALSE;
            m_pPlayerTwoCombo ->SetFlag( ui_win::WF_HIGHLIGHT, TRUE );
            m_pPlayerTwoText  ->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_SELECT") );

            // clear the selected profile
            g_StateMgr.ClearSelectedProfile( 1 );

            // set default selection
            m_pPlayerTwoCombo ->SetSelection( m_CreateIndex );
        }
    }
    else
    {
        // restore selection for player 2
        m_PlayerReady[1] = FALSE;
        if( ( m_pPlayerTwoCombo->GetItemCount() > CurrentSelection2 ) && ( CurrentSelection2 >= 0 ) )
        {
            m_pPlayerTwoCombo->SetSelection( CurrentSelection2 );
        }
        else
        {
            m_pPlayerTwoCombo->SetSelection( m_CreateIndex );
        }
    }

    if( g_StateMgr.GetSelectedProfile( 2 ) )
    {
        if( Found3 )
        {
            // set the selected profile as the active profile
            m_PlayerReady[2] = TRUE;
            m_pPlayerThreeCombo ->SetFlag( ui_win::WF_HIGHLIGHT, FALSE );
            m_pPlayerThreeText  ->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_READY") );
            m_pPlayerThreeCombo ->SetSelection( CurrentSelection3 );
        }
        else if(  g_StateMgr.GetProfileNotSaved( 2 ) )
        {
            // set the unsaved profile as the active profile
            m_PlayerReady[2] = TRUE;
            m_pPlayerThreeCombo ->SetFlag( ui_win::WF_HIGHLIGHT, FALSE );
            m_pPlayerThreeText  ->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_READY") );
            m_pPlayerThreeCombo ->SetSelection( CurrentSelection3 );
        }
        else
        {
            // profile not found, deselect it
            m_PlayerReady[0] = FALSE;
            m_pPlayerThreeCombo ->SetFlag( ui_win::WF_HIGHLIGHT, TRUE );
            m_pPlayerThreeText  ->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_SELECT") );

            // clear the selected profile
            g_StateMgr.ClearSelectedProfile( 2 );

            // set default selection
            m_pPlayerThreeCombo ->SetSelection( m_CreateIndex );
        }
    }
    else
    {
        // restore selection for player 3
        m_PlayerReady[2] = FALSE;
        if( ( m_pPlayerThreeCombo->GetItemCount() > CurrentSelection3 ) && ( CurrentSelection3 >= 0 ) )
        {
            m_pPlayerThreeCombo->SetSelection( CurrentSelection3 );
        }
        else
        {
            m_pPlayerThreeCombo->SetSelection( m_CreateIndex );
        }
    }

    if( g_StateMgr.GetSelectedProfile( 3 ) )
    {
        if( Found4 )
        {
            // set the selected profile as the active profile
            m_PlayerReady[3] = TRUE;
            m_pPlayerFourCombo ->SetFlag( ui_win::WF_HIGHLIGHT, FALSE );
            m_pPlayerFourText  ->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_READY") );
            m_pPlayerFourCombo ->SetSelection( CurrentSelection4 );
        }
        else if(  g_StateMgr.GetProfileNotSaved( 3 ) )
        {
            // set the unsaved profile as the active profile
            m_PlayerReady[3] = TRUE;
            m_pPlayerFourCombo ->SetFlag( ui_win::WF_HIGHLIGHT, FALSE );
            m_pPlayerFourText  ->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_READY") );
            m_pPlayerFourCombo ->SetSelection( CurrentSelection4 );
        }
        else
        {
            // profile not found, deselect it
            m_PlayerReady[3] = FALSE;
            m_pPlayerFourCombo ->SetFlag( ui_win::WF_HIGHLIGHT, TRUE );
            m_pPlayerFourText  ->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_SELECT") );

            // clear the selected profile
            g_StateMgr.ClearSelectedProfile( 3 );

            // set default selection
            m_pPlayerFourCombo ->SetSelection( m_CreateIndex );
        }
    }
    else
    {
        // restore selection for player 4
        m_PlayerReady[3] = FALSE;
        if( ( m_pPlayerFourCombo->GetItemCount() > CurrentSelection4 ) && ( CurrentSelection4 >= 0 ) )
        {
            m_pPlayerFourCombo->SetSelection( CurrentSelection4 );
        }
        else
        {
            m_pPlayerFourCombo->SetSelection( m_CreateIndex );
        }
    }
}

//=========================================================================

void dlg_multi_player::OnLoadProfileCB( void )
{
    MemCardMgr::condition& Condition1 = g_UIMemCardMgr.GetCondition( 0 );

    // if the load was successful
    if( Condition1.SuccessCode )
    {
        if( m_bEditProfile )
        {
            // edit profile
            m_State = DIALOG_STATE_EDIT;
        }
        else
        {
            s32 PlayerID = g_StateMgr.GetPendingProfileIndex();

            switch( PlayerID )
            {
                case 0:
                {
                    
                    // set the selected profile as the active profile
                    m_PlayerReady[0] = TRUE;
                    m_pPlayerOneCombo->SetFlag( ui_win::WF_HIGHLIGHT, FALSE );
                    m_pPlayerOneText->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_READY") );
                }
                break;

                case 1:
                {
                    // set the selected profile as the active profile
                    m_PlayerReady[1] = TRUE;
                    m_pPlayerTwoCombo->SetFlag( ui_win::WF_HIGHLIGHT, FALSE );
                    m_pPlayerTwoText->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_READY") );
                }
                break;

                case 2:
                {
                    // set the selected profile as the active profile
                    m_PlayerReady[2] = TRUE;
                    m_pPlayerThreeCombo->SetFlag( ui_win::WF_HIGHLIGHT, FALSE );
                    m_pPlayerThreeText->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_READY") );
                }
                break;

                case 3:
                {
                    // set the selected profile as the active profile
                    m_PlayerReady[3] = TRUE;
                    m_pPlayerFourCombo->SetFlag( ui_win::WF_HIGHLIGHT, FALSE );
                    m_pPlayerFourText->SetLabel( g_StringTableMgr("ui", "IDS_MULTI_PLAYER_READY") );
                }
                break;

            }
            // flag the profile as saved
            g_StateMgr.SetProfileNotSaved( g_StateMgr.GetPendingProfileIndex(), FALSE );
            // copy the data read into the profile
            g_StateMgr.ActivatePendingProfile();

            // make the dialog active again
            m_State = DIALOG_STATE_ACTIVE;
        }
    }
    else
    {
        // deactivate
        // clear the selected profile
        g_StateMgr.ClearSelectedProfile( g_StateMgr.GetPendingProfileIndex() );

        //continue with the status quo of polling the memory cards
        m_State = DIALOG_STATE_ACTIVE;
    }
}

//=========================================================================

