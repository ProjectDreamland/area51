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

#include "../dlg_PopUp.hpp"
#include "dlg_MemcardSelectXBOX.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "MemCardMgr/MemCardMgr.hpp"



//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum controls
{
    IDC_MEMCARD_SELECT_INFO_BOX_1,
    IDC_MEMCARD_SELECT_STATUS_TEXT_1,
    IDC_MEMCARD_SELECT_NAV_TEXT,
};


ui_manager::control_tem MemcardSelectControls[] = 
{
    // Frames.
    { IDC_MEMCARD_SELECT_INFO_BOX_1,     "IDS_NULL",    "blankbox",  40,  80, 282, 50, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MEMCARD_SELECT_STATUS_TEXT_1,  "IDS_NULL",    "text",      50, 105, 262, 20, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MEMCARD_SELECT_NAV_TEXT,       "IDS_NULL",    "text",       0,   0,   0,  0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem MemcardSelectDialog =
{
    "IDS_MEMCARD_SELECT_MENU",
    1, 9,
    sizeof(MemcardSelectControls)/sizeof(ui_manager::control_tem),
    &MemcardSelectControls[0],
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

       
    m_pInfoBox1     = (ui_blankbox*)    FindChildByID( IDC_MEMCARD_SELECT_INFO_BOX_1    );
    m_pStatusText1  = (ui_text*)        FindChildByID( IDC_MEMCARD_SELECT_STATUS_TEXT_1 );
    m_pNavText      = (ui_text*)        FindChildByID( IDC_MEMCARD_SELECT_NAV_TEXT      );
    
    GotoControl( (ui_control*)m_pInfoBox1 );
    m_CurrentControl = IDC_MEMCARD_SELECT_INFO_BOX_1;
    m_CurrHL = 0;
    m_PopUp = NULL;

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" ); 
    m_pNavText->SetLabel( navText );
    m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // set up info box
    m_pInfoBox1->SetHasTitleBar( TRUE );
    m_pInfoBox1->SetLabel( g_StringTableMgr( "ui", "IDS_HARD_DISK_TITLE" ) );
    m_pInfoBox1->SetLabelColor( xcolor(255,252,204,255) );
    m_pInfoBox1->SetTitleBarColor( xcolor(19,59,14,196) );
    m_pInfoBox1->SetBackgroundColor( xcolor (39,117,28,128) );

    m_pStatusText1   ->UseSmallText( TRUE );
    m_pStatusText1   ->SetLabelColor( xcolor(255,252,204,255) );
    m_pStatusText1   ->SetLabelFlags( ui_font::h_left|ui_font::v_center );

    // hide elements during scaling
    m_pInfoBox1     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pStatusText1  ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // initialize memcard status strings
    m_pStatusText1->SetLabel( g_StringTableMgr( "ui", "IDS_NULL" ) );

    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_memcard_select::Destroy( void )
{
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
            // attempt to save the profile to the hdd
            g_UIMemCardMgr.CreateProfile( 0, g_StateMgr.GetPendingProfileIndex(), this, &dlg_memcard_select::OnProfileCreateCB );

            g_AudioMgr.Play( "Select_Norm" );
        }

        // change the dialog state to wait for the memcard
        m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
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

        m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_PROFILE_CREATE" ), 
            TRUE, 
            TRUE, 
            FALSE, 
            g_StringTableMgr( "ui", "IDS_PROFILE_CANCEL_CREATE_MSG" ),
            navText,
            &m_PopUpResult );
        return;
    }
}

//=========================================================================

void dlg_memcard_select::OnPollReturn( void )
{
    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        m_pStatusText1->SetLabel( g_StringTableMgr( "ui", "IDS_NULL" ) );

        // get the current status of the memcards
        MemCardMgr::condition& Condition1 = g_UIMemCardMgr.GetCondition( 0 );

        // update the card status text
        if ( Condition1.bDamaged )
            m_pStatusText1->SetLabel( g_StringTableMgr( "ui", "IDS_MEMCARD_MSG_DAMAGED" ) );
        else if ( Condition1.bUnformatted )
            m_pStatusText1->SetLabel( g_StringTableMgr( "ui", "IDS_MEMCARD_MSG_UNFORMATTED" ) );
        else if ( !Condition1.bNoCard )
        {
            if( (u32)(Condition1.BytesFree/16384) > 50000 )
            {
                m_pStatusText1->SetLabel( xwstring( "%d+ %s", 50000, g_StringTableMgr( "ui", "IDS_BLOCKS") ) );
            }
            else
            {
                m_pStatusText1->SetLabel( xwstring( xfs( "%d %s", (u32)(Condition1.BytesFree/16384), "IDS_BLOCKS" ) ) );
            }
        }
    }
}

//=========================================================================

void dlg_memcard_select::OnProfileCreateCB( void )
{
    MemCardMgr::condition& Condition1 = g_UIMemCardMgr.GetCondition( 0 );

    // If the save was successful OR user continues WITHOUT saving
    if( Condition1.SuccessCode )
    {
        // saved ok
        // or continue without saving 

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
            m_pInfoBox1     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pStatusText1  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText      ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            // set the highlight
            g_UiMgr->EnableScreenHighlight();
            g_UiMgr->SetScreenHighlight( m_pInfoBox1->GetPosition() );
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
                g_StateMgr.ClearSelectedProfile( g_StateMgr.GetPendingProfileIndex() );
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
        g_UIMemCardMgr.Poll( this,&dlg_memcard_select::OnPollReturn );
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);
}

//=========================================================================

