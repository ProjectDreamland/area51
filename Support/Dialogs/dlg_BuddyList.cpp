#if 0
//=========================================================================
//
//  dlg_buddy_list.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_listbox.hpp"
#include "ui\ui_dlg_vkeyboard.hpp"

#include "dlg_PopUp.hpp"
#include "dlg_BuddyList.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Parsing/textin.hpp"



//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum controls
{
	IDC_BUDDY_LIST_LISTBOX,
    IDC_BUDDY_LIST_NAV_TEXT,
};


ui_manager::control_tem BuddyListControls[] = 
{
    // Frames.
    { IDC_BUDDY_LIST_LISTBOX,   "IDS_BUDDY_LIST",   "listbox",  45,  60, 180, 238, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_BUDDY_LIST_NAV_TEXT,  "IDS_NULL",         "text",      0,   0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};

ui_manager::dialog_tem BuddyListDialog =
{
    "IDS_BUDDY_LIST_MENU",
    1, 9,
    sizeof(BuddyListControls)/sizeof(ui_manager::control_tem),
    &BuddyListControls[0],
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

void dlg_buddy_list_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "buddy list", &BuddyListDialog, &dlg_buddy_list_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_buddy_list_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_buddy_list* pDialog = new dlg_buddy_list;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_level_select
//=========================================================================

dlg_buddy_list::dlg_buddy_list( void )
{
}

//=========================================================================

dlg_buddy_list::~dlg_buddy_list( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_buddy_list::Create( s32                        UserID,
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
    m_pBuddyList       = (ui_listbox*)    FindChildByID( IDC_BUDDY_LIST_LISTBOX  );
    m_pNavText         = (ui_text*)       FindChildByID( IDC_BUDDY_LIST_NAV_TEXT );

    // hide them
    m_pBuddyList      ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pNavText        ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
    navText += g_StringTableMgr( "ui", "IDS_NAV_DELETE" );
   
    m_pNavText->SetLabel( navText );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);


    // set up level list
    m_pBuddyList->SetFlag(ui_win::WF_SELECTED, TRUE);
    m_pBuddyList->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pBuddyList->DisableFrame();
    m_pBuddyList->SetExitOnSelect(FALSE);
    m_pBuddyList->SetExitOnBack(TRUE);
    m_pBuddyList->EnableHeaderBar();
    m_pBuddyList->SetHeaderBarColor( xcolor(19,59,14,196) );
    m_pBuddyList->SetHeaderColor( xcolor(255,252,204,255) );

    // get buddy data
    RefreshBuddyList();
    m_pBuddyList->SetSelection( 0 );
    m_BuddyEntered = FALSE;
    m_BuddyOk = FALSE;

    // set initial focus
    m_CurrHL = 0;
    GotoControl( (ui_control*)m_pBuddyList );
    m_CurrentControl = IDC_BUDDY_LIST_LISTBOX;
    m_PopUp = NULL;

    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

	// Return success code
    return Success;
}

//=========================================================================

void dlg_buddy_list::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_buddy_list::Render( s32 ox, s32 oy )
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

void dlg_buddy_list::OnNotify ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData )
{
    (void)pWin;
    (void)pSender;
    (void)Command;
    (void)pData;
}

//=========================================================================

void dlg_buddy_list::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
}

//=========================================================================

void dlg_buddy_list::OnPadSelect( ui_win* pWin )
{
    (void)pWin;
    
    char tempString[SM_BUDDY_STRING_LENGTH];

    // get the buddy index from the list
    u32 index = m_pBuddyList->GetSelectedItemData();


    // check if this is a new buddy string
    player_profile& CurrentProfile = g_StateMgr.GetPendingProfile();
    x_strcpy( tempString, CurrentProfile.GetBuddyString(index) );

    if ( x_strlen( tempString ) != 0 )
    {
        // edit a buddy name
        // open a VK to edit the buddy name
        irect   r = m_pManager->GetUserBounds( m_UserID );
        ui_dlg_vkeyboard* pVKeyboard = (ui_dlg_vkeyboard*)m_pManager->OpenDialog( m_UserID, "ui_vkeyboard", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
        pVKeyboard->SetLabel( g_StringTableMgr( "ui", "IDS_BUDDY_EDIT" ) );
        m_BuddyName = xwstring( tempString );
        pVKeyboard->ConnectString( &m_BuddyName, SM_BUDDY_STRING_LENGTH );
        pVKeyboard->SetReturn( &m_BuddyEntered, &m_BuddyOk );
    }
    else
    {
        // add a new buddy       
        // open a VK to enter the buddy name
        irect   r = m_pManager->GetUserBounds( m_UserID );
        ui_dlg_vkeyboard* pVKeyboard = (ui_dlg_vkeyboard*)m_pManager->OpenDialog( m_UserID, "ui_vkeyboard", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
        pVKeyboard->SetLabel( g_StringTableMgr( "ui", "IDS_BUDDY_CREATE" ) );
        m_BuddyName = xwstring( "" );
        pVKeyboard->ConnectString( &m_BuddyName, SM_BUDDY_STRING_LENGTH );
        pVKeyboard->SetReturn( &m_BuddyEntered, &m_BuddyOk );
    }

    // update nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
    navText += g_StringTableMgr( "ui", "IDS_NAV_DELETE" );
    m_pNavText->SetLabel( navText );
}

//=========================================================================

void dlg_buddy_list::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        //g_AudioMgr.Play("OptionBack");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_buddy_list::OnPadDelete( ui_win* pWin )
{
    (void)pWin;
    // delete the buddy

    // get the pending profile
    player_profile& CurrentProfile = g_StateMgr.GetPendingProfile();

    // get the profile index from the list
    u32 index = m_pBuddyList->GetSelectedItemData();

    // make sure the profile is valid
    if( x_strlen(CurrentProfile.GetBuddyString(index)) != 0 )
    {
        // open delete confirmation dialog
        irect r = g_UiMgr->GetUserBounds( g_UiUserID );
        m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

        // set nav text
        xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
        navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
        m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);
        

        m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_BUDDY_DELETE" ), 
                            TRUE, 
                            TRUE, 
                            FALSE, 
                            g_StringTableMgr( "ui", "IDS_BUDDY_DELETE_MSG" ),
                            navText,
                            &m_PopUpResult );

    }
}

//=========================================================================

void dlg_buddy_list::OnPadActivate( ui_win* pWin )
{
    (void)pWin;
}

//=========================================================================

void dlg_buddy_list::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // get the pending profile
    player_profile& CurrentProfile = g_StateMgr.GetPendingProfile();

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // turn on the controls
            m_pBuddyList ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            GotoControl( (ui_control*)m_pBuddyList );
            g_UiMgr->SetScreenHighlight( m_pBuddyList->GetPosition() );
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    // check for profile name entry
    if( m_BuddyEntered )
    {
        m_BuddyEntered = FALSE;
    
        if( m_BuddyOk )
        {
            m_BuddyOk = FALSE;

            // store the new profile name
            u32 index = m_pBuddyList->GetSelectedItemData();
            CurrentProfile.SetBuddyString( index, xstring(m_BuddyName) );
            RefreshBuddyList();
            m_pBuddyList->SetSelection( index );
        }
    }

    // get the profile index from the list
    s32 index = m_pBuddyList->GetSelectedItemData();

    if ( m_PopUp )
    {
        if ( m_PopUpResult != DLG_POPUP_IDLE )
        {
            if ( m_PopUpResult == DLG_POPUP_YES )
            {
                // Bye bye buddy, bye bye!
                CurrentProfile.SetBuddyString( index, "\0" );
                RefreshBuddyList();
                if ( --index >= 0 )
                    m_pBuddyList->SetSelection( index );
                else
                    m_pBuddyList->SetSelection( 0 );
            }

            // clear popup 
            m_PopUp = NULL;

            // turn on nav text
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
    }
    else
    {
        // check if this is a new profile
        if( x_strlen(CurrentProfile.GetBuddyString(m_pBuddyList->GetSelection())) != 0 )
        {
            // update nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
            navText += g_StringTableMgr( "ui", "IDS_NAV_DELETE" );
            m_pNavText->SetLabel( navText );
        }
        else
        {
            // update nav text
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
            navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
            m_pNavText->SetLabel( navText );
        }
    }
}

//=========================================================================

void dlg_buddy_list::RefreshBuddyList( void )
{
    char tempString[SM_BUDDY_STRING_LENGTH];

    // clear the list
    m_pBuddyList->DeleteAllItems();

    player_profile& CurrentProfile = g_StateMgr.GetPendingProfile();

    // fill it with the profile information
    for (s32 i=0; i<SM_NUM_BUDDY_STRINGS; i++)
    {
        x_strcpy( tempString, CurrentProfile.GetBuddyString(i) );

        if ( x_strlen( tempString ) != 0 )
        {
            // add the string to the list
            m_pBuddyList->AddItem( tempString, i );
        }
        else
        {
            // add an empty slot
            m_pBuddyList->AddItem( g_StringTableMgr("ui", "IDS_BUDDY_LIST_ADD"), i );

            // and don't add any more
            break;
        }
    }
}

//=========================================================================
#endif
