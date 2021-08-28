//=========================================================================
//
//  dlg_COPA.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_text.hpp"
#include "ui\ui_font.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"

#include "dlg_COPA.hpp"
#include "dlg_PopUp.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "MemCardMgr/MemCardMgr.hpp"

//=========================================================================
//  Main Menu Dialog
//=========================================================================
enum
{
    COPA_POPUP_PLAYER_UNDER_AGE,
    COPA_POPUP_REQUEST_SAVE,
};

enum copa_controls
{
    IDC_COPA_MESSAGE_TEXT,

    IDC_COPA_MONTH_TEXT,
    IDC_COPA_DAY_TEXT,
    IDC_COPA_YEAR_TEXT,

    IDC_COPA_MONTH_SELECT,
    IDC_COPA_DAY_SELECT,
    IDC_COPA_YEAR_SELECT,
    IDC_COPA_ACCEPT_BUTTON,

    IDC_COPA_NAV_TEXT,
};

ui_manager::control_tem COPAControls[] = 
{
    { IDC_COPA_MESSAGE_TEXT,    "IDS_COPA_MESSAGE",      "text",   40,  50, 200,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_COPA_MONTH_TEXT,      "IDS_COPA_SELECT_MONTH", "text",   40, 100,  80,  40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_COPA_DAY_TEXT,        "IDS_COPA_SELECT_DAY",   "text",   40, 135,  80,  40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_COPA_YEAR_TEXT,       "IDS_COPA_SELECT_YEAR",  "text",   40, 170,  80,  40, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_COPA_MONTH_SELECT,    "IDS_COPA_SELECT_MONTH", "combo", 140, 109, 100,  40, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_COPA_DAY_SELECT,      "IDS_COPA_SELECT_DAY",   "combo", 140, 144, 100,  40, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_COPA_YEAR_SELECT,     "IDS_COPA_SELECT_YEAR",  "combo", 140, 179, 100,  40, 0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_COPA_ACCEPT_BUTTON,   "IDS_COPA_ACCEPT",       "button", 40, 285, 220,  40, 0, 3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_COPA_NAV_TEXT,        "IDS_NULL",              "text",    0,   0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};

ui_manager::dialog_tem COPADialog =
{
    "IDS_NULL",
    1, 9,
    sizeof(COPAControls)/sizeof(ui_manager::control_tem),
    &COPAControls[0],
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

void dlg_copa_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "COPA", &COPADialog, &dlg_copa_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_copa_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_copa* pDialog = new dlg_copa;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_copa
//=========================================================================

dlg_copa::dlg_copa( void )
{
}

//=========================================================================

dlg_copa::~dlg_copa( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

xbool dlg_copa::Create( s32                        UserID,
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
   
    // initialize controls
    m_pButtonAccept = (ui_button*)FindChildByID( IDC_COPA_ACCEPT_BUTTON );
    m_pButtonAccept ->SetFlag( ui_win::WF_BUTTON_LEFT, TRUE );

    m_pMessageText  = (ui_text*)FindChildByID( IDC_COPA_MESSAGE_TEXT );
    m_pMessageText  ->SetLabelFlags( ui_font::h_center|ui_font::v_center );
    m_pMessageText  ->UseSmallText( TRUE );
    m_pMessageText  ->SetLabelColor( xcolor(255,252,204,255) );

    m_pMonthText    = (ui_text*)FindChildByID( IDC_COPA_MONTH_TEXT   );
    m_pDayText      = (ui_text*)FindChildByID( IDC_COPA_DAY_TEXT     );
    m_pYearText     = (ui_text*)FindChildByID( IDC_COPA_YEAR_TEXT    );

    m_pMonthText    ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pDayText      ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pYearText     ->SetLabelFlags( ui_font::h_left|ui_font::v_center );


    m_pMonthCombo   = (ui_combo*)FindChildByID( IDC_COPA_MONTH_SELECT );
    m_pDayCombo     = (ui_combo*)FindChildByID( IDC_COPA_DAY_SELECT   );
    m_pYearCombo    = (ui_combo*)FindChildByID( IDC_COPA_YEAR_SELECT  );

    // set up month combo box
    m_pMonthCombo->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT | ui_combo::CB_NOTIFY_PARENT );
    m_pMonthCombo->AddItem ( g_StringTableMgr( "ui", "IDS_MONTH1"  ),  1 );
    m_pMonthCombo->AddItem ( g_StringTableMgr( "ui", "IDS_MONTH2"  ),  2 );
    m_pMonthCombo->AddItem ( g_StringTableMgr( "ui", "IDS_MONTH3"  ),  3 );
    m_pMonthCombo->AddItem ( g_StringTableMgr( "ui", "IDS_MONTH4"  ),  4 );
    m_pMonthCombo->AddItem ( g_StringTableMgr( "ui", "IDS_MONTH5"  ),  5 );
    m_pMonthCombo->AddItem ( g_StringTableMgr( "ui", "IDS_MONTH6"  ),  6 );
    m_pMonthCombo->AddItem ( g_StringTableMgr( "ui", "IDS_MONTH7"  ),  7 );
    m_pMonthCombo->AddItem ( g_StringTableMgr( "ui", "IDS_MONTH8"  ),  8 );
    m_pMonthCombo->AddItem ( g_StringTableMgr( "ui", "IDS_MONTH9"  ),  9 );
    m_pMonthCombo->AddItem ( g_StringTableMgr( "ui", "IDS_MONTH10" ), 10 );
    m_pMonthCombo->AddItem ( g_StringTableMgr( "ui", "IDS_MONTH11" ), 11 );
    m_pMonthCombo->AddItem ( g_StringTableMgr( "ui", "IDS_MONTH12" ), 12 );

    // set up day combo box
    m_pDayCombo->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT );
    for( s32 d=1; d<31; d++ )
    {
        xwstring DayString( xfs("%d", d) );
        m_pDayCombo->AddItem ( DayString,  d );
    }

    // set up year combo box
    m_pYearCombo->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT | ui_combo::CB_NOTIFY_PARENT );
    for( s32 y=1920; y<2006; y++ )
    {
        xwstring YearString( xfs("%d", y ) );
        m_pYearCombo->AddItem ( YearString,  y );
    }

    // initialize date selection (to launch date 4/15/2005)
    m_pMonthCombo ->SetSelection( 3 );
    m_pDayCombo   ->SetSelection( 14 );
    m_pYearCombo  ->SetSelection( m_pYearCombo->GetItemCount()-1 );

    // Initialize nav text
    m_pNavText = (ui_text*)FindChildByID( IDC_COPA_NAV_TEXT );
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
    m_pNavText->SetLabel( navText );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // set initial highlight/control
    m_CurrHL = 3;
    m_pButtonAccept->SetFlag(ui_win::WF_SELECTED, TRUE);
    GotoControl( (ui_control*)m_pButtonAccept );
    g_UiMgr->SetScreenHighlight( m_pButtonAccept->GetPosition() );
    m_PopUp = NULL;

    // hide all controls during scaling
    m_pMessageText  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pMonthText    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pDayText      ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pYearText     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pMonthCombo   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pDayCombo     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pYearCombo    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pButtonAccept ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pNavText      ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_copa::Destroy( void )
{
    ui_dialog::Destroy();
}

//=========================================================================

void dlg_copa::Render( s32 ox, s32 oy )
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

void dlg_copa::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    s32 highLight = -1;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // hide all controls during scaling
            m_pMessageText  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pMonthText    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pDayText      ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pYearText     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pMonthCombo   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pDayCombo     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pYearCombo    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pButtonAccept ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText      ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            m_CurrHL = 3;
            m_pButtonAccept->SetFlag(ui_win::WF_SELECTED, TRUE);
            GotoControl( (ui_control*)m_pButtonAccept );
            g_UiMgr->SetScreenHighlight( m_pButtonAccept->GetPosition() );
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    if ( m_PopUp )
    {
        if ( m_PopUpResult != DLG_POPUP_IDLE )
        {
            if( m_PopUpType == COPA_POPUP_PLAYER_UNDER_AGE )
            {
                // we can't disconnect the player without their express consent
                // so we'll just let them try and enter their age again! 
                // I'm sure that they won't lie! hmmmm.....
                m_PopUp = NULL;
                m_State = DIALOG_STATE_ACTIVE;
                m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
            }
            else
            {
                // save changes?
                if( m_PopUpResult == DLG_POPUP_YES )
                {
                    // save changes.  Copy the active profile to the pending profile for saving
                    g_StateMgr.InitPendingProfile(0);

                    // check if this profile is saved
                    if( g_StateMgr.GetProfileNotSaved( 0 ) )
                    {
                        // not saved
                        // go to the profile select screen and pick a place to save it to
                        m_State = DIALOG_STATE_MEMCARD_ERROR;
                    }
                    else
                    {
                        // attempt to save the changes to the memcard
                        profile_info* pProfileInfo = &g_UIMemCardMgr.GetProfileInfo( 0 );
                        m_iCard = pProfileInfo->CardID;
                        g_UIMemCardMgr.SaveProfile( *pProfileInfo, 0, this, &dlg_copa::OnSaveProfileCB );
                        // clear popup pointer
                        m_PopUp = NULL;
                        // wait for the memcard manager to do it's thing
                        m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
                    }
                }
                else
                {
                    // no save required, continue on to authentication
                    m_State = DIALOG_STATE_SELECT;
                }
            }
        }
    }

    // update labels
    if( m_pMonthCombo->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 0;
        m_pMonthText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pMonthText->GetPosition() );
    }
    else
        m_pMonthText->SetLabelColor( xcolor(126,220,60,255) );

    if( m_pDayCombo->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 1;
        m_pDayText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pDayText->GetPosition() );
    }
    else
        m_pDayText->SetLabelColor( xcolor(126,220,60,255) );

    if( m_pYearCombo->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 2;
        m_pYearText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pYearText->GetPosition() );
    }
    else
        m_pYearText->SetLabelColor( xcolor(126,220,60,255) );

    if( m_pButtonAccept->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 3;
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

void dlg_copa::OnNotify ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData )
{
    (void)pWin;
    (void)pSender;
    (void)Command;
    (void)pData;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( Command == WN_COMBO_SELCHANGE )
        {
            if( pSender == (ui_win*)m_pMonthCombo )
            {
                OnMonthChange();
            }
            else if( pSender == (ui_win*)m_pYearCombo )
            {
                OnYearChange();
            }
        }
    }
}

//=========================================================================

void dlg_copa::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( pWin == (ui_win*)m_pButtonAccept )
        {
            if( VerifyAge() )
            {
                // set the age verified bit in the profile
                g_StateMgr.GetActiveProfile(0).m_bAgeVerified = TRUE;

                // continue on to authentication
                //m_State = DIALOG_STATE_SELECT;
                //g_AudioMgr.Play( "Select_Norm" );

                // prompt to save
                irect r = g_UiMgr->GetUserBounds( g_UiUserID );
                m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

                // set nav text
                xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
                navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );

                // configure message
                m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_PROFILE_CHANGED_TITLE" ), 
                    TRUE, 
                    TRUE, 
                    FALSE, 
                    g_StringTableMgr( "ui", "IDS_PROFILE_CHANGED_MSG" ),
                    navText,
                    &m_PopUpResult );

                m_PopUpType = COPA_POPUP_REQUEST_SAVE;
            }
            else
            {
                // not old enough, display pop up to inform them
                irect r = g_UiMgr->GetUserBounds( g_UiUserID );
                m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

                // set nav text
                xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_OK" ));
                m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

                m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_NETWORK_POPUP" ), 
                    TRUE, 
                    FALSE, 
                    FALSE, 
                    g_StringTableMgr( "ui", "IDS_COPA_ACCESS_DENIED" ),
                    navText,
                    &m_PopUpResult );

                m_PopUpType = COPA_POPUP_PLAYER_UNDER_AGE;
            }
        }
    }
}

//=========================================================================

void dlg_copa::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        m_State = DIALOG_STATE_BACK;
        g_AudioMgr.Play( "Backup" );
    }
}

//=========================================================================

void dlg_copa::OnYearChange( void )
{
    // check if we're currently on february
    if( m_pMonthCombo->GetSelectedItemData() == 2 )
    {
        // need to check for leap years
        OnMonthChange();
    }
}

//=========================================================================

void dlg_copa::OnMonthChange( void )
{
    s32 NumDays  = 0;
    s32 CurrDays = m_pDayCombo->GetItemCount()+1;
    s32 Sel      = m_pDayCombo->GetSelection();
    s32 Month    = m_pMonthCombo->GetSelectedItemData();

    // determine the number of days based on the month and year
    switch( Month )
    {
    case 4:
    case 6:
    case 9:
    case 11:
        NumDays = 31;
        break;

    case 2:
        // check for leap years
        if( m_pYearCombo->GetSelectedItemData()%4 == 0 )
            NumDays = 30;
        else
            NumDays = 29;
    
        break;

    default:
        NumDays = 32;
        break;
    }

    // check if number of days changed
    if( NumDays != CurrDays )
    {
        // reinitialize day combo box based on month
        m_pDayCombo->DeleteAllItems();
        for( s32 d=1; d<NumDays; d++ )
        {
            xwstring DayString( xfs("%d", d) );
            m_pDayCombo->AddItem ( DayString,  d );
        }

        // restore selection
        if( Sel < m_pDayCombo->GetItemCount() )
            m_pDayCombo->SetSelection( Sel );
        else
            m_pDayCombo->SetSelection( 0 );
    }
}

//=========================================================================

xbool dlg_copa::VerifyAge( void )
{
    // get current date from the system
    split_date DateStamp = eng_SplitDate( eng_GetDate() );
    
    // make sure we have a valid year
    if( DateStamp.Year < 1900 )
        DateStamp.Year += 2000;
    
    // verify the player is 13+ years old
    s32 MaxYear = (s32)(DateStamp.Year - 13);

    // check year
    if( m_pYearCombo->GetSelectedItemData() < MaxYear ) 
    {
        return TRUE;
    }
    else if( m_pYearCombo->GetSelectedItemData() == MaxYear )
    {
        // check month
        if( m_pMonthCombo->GetSelectedItemData() < DateStamp.Month )
        {
            return TRUE;
        }
        else if( m_pMonthCombo->GetSelectedItemData() == DateStamp.Month )
        {
            // check day
            if( m_pDayCombo->GetSelectedItemData() <= DateStamp.Day )
            {
                return TRUE;
            }
        }
    }

    // not old enough
    return FALSE;
}

//=========================================================================

void dlg_copa::OnSaveProfileCB( void )
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
        m_State = DIALOG_STATE_SELECT;            
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



