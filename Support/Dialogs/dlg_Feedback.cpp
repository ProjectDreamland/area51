//=========================================================================
//
//  dlg_feedback.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_blankbox.hpp"

#include "dlg_PopUp.hpp" 
#include "dlg_Feedback.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "MemCardMgr/MemCardMgr.hpp"

//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum controls
{
    IDC_PLAYER_DETAILS,
    IDC_PLAYER_NAME_TEXT,
    IDC_SESSION_TIME_TEXT,
    
    IDC_FEEDBACK_TEXT,
    IDC_GREAT_SESSION,
    IDC_GOOD_ATTITUDE,
    IDC_OFFENSIVE_MESSAGE,

    IDC_COMPLAINTS_TEXT,
    IDC_BAD_NAME,
    IDC_CURSING,
    IDC_SCREAMING,
    IDC_CHEATING,
    IDC_THREATS,

    IDC_FEEDBACK_NAV_TEXT,
};

ui_manager::control_tem FeedbackControls[] = 
{
    // Frames.
    { IDC_PLAYER_DETAILS,       "IDS_NULL",                     "blankbox",    40,  40, 416,  44, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PLAYER_NAME_TEXT,     "IDS_NULL",                     "text",        48,  62, 200,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_SESSION_TIME_TEXT,    "IDS_NULL",                     "text",        308, 62, 140,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_FEEDBACK_TEXT,        "IDS_FEEDBACK",                 "text",        40,  90, 220,  30, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_GREAT_SESSION,        "IDS_FEEDBACK_GREAT_SESSION",   "button",     200,  90, 120,  30, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_GOOD_ATTITUDE,        "IDS_FEEDBACK_GOOD_ATTITUDE",   "button",     200, 120, 120,  30, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_COMPLAINTS_TEXT,      "IDS_COMPLAINTS",               "text",        40, 165, 220,  30, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_BAD_NAME,             "IDS_BAD_NAME",                 "button",     200, 165, 120,  30, 0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_CURSING,              "IDS_CURSING",                  "button",     200, 195, 120,  30, 0, 3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_SCREAMING,            "IDS_SCREAMING",                "button",     200, 225, 120,  30, 0, 4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_CHEATING,             "IDS_CHEATING",                 "button",     200, 255, 120,  30, 0, 5, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_THREATS,              "IDS_THREATS",                  "button",     200, 285, 120,  30, 0, 6, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_OFFENSIVE_MESSAGE,    "IDS_OFFENSIVE",                "button",     200, 315, 120,  30, 0, 7, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_FEEDBACK_NAV_TEXT,    "IDS_NULL",                     "text",         0,   0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem FeedbackDialog =
{
    "IDS_FEEDBACK_TITLE",
    1, 9,
    sizeof(FeedbackControls)/sizeof(ui_manager::control_tem),
    &FeedbackControls[0],
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

void dlg_feedback_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "feedback", &FeedbackDialog, &dlg_feedback_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_feedback_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_feedback* pDialog = new dlg_feedback;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_feedback
//=========================================================================

dlg_feedback::dlg_feedback( void )
{
}

//=========================================================================

dlg_feedback::~dlg_feedback( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_feedback::Create( s32                        UserID,
                             ui_manager*                pManager,
                             ui_manager::dialog_tem*    pDialogTem,
                             const irect&               Position,
                             ui_win*                    pParent,
                             s32                        Flags,
                             void*                      pUserData )
{
    xbool   Success = FALSE;

    m_PopUpResult = DLG_POPUP_IDLE;
    m_Type        = NORMAL_FEEDBACK;

    (void)pUserData;

    ASSERT( pManager );

    // Do dialog creation
	Success = ui_dialog::Create( UserID, pManager, pDialogTem, Position, pParent, Flags );
    
    // set up nav text
    m_pNavText = (ui_text*) FindChildByID( IDC_FEEDBACK_NAV_TEXT );
    
    xwstring navText( g_StringTableMgr( "ui", "IDS_NAV_SELECT" ) );
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );

    m_pNavText->SetLabel( navText );
    m_pNavText->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);    

    // get player details box
    m_pPlayerDetails = (ui_blankbox*)FindChildByID( IDC_PLAYER_DETAILS );
    m_pPlayerDetails->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pPlayerDetails->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pPlayerDetails->SetHasTitleBar( TRUE );
    m_pPlayerDetails->SetLabelColor( xcolor(255,252,204,255) );
    m_pPlayerDetails->SetTitleBarColor( xcolor(19,59,14,196) );
    m_pPlayerDetails->SetLabel( g_StringTableMgr( "ui", "IDS_FEEDBACK_DETAILS" ) );
    
    // set up info text
    m_pPlayerName   = (ui_text*)FindChildByID( IDC_PLAYER_NAME_TEXT  );
    m_pSessionDate  = (ui_text*)FindChildByID( IDC_SESSION_TIME_TEXT );
    m_pFeedback     = (ui_text*)FindChildByID( IDC_FEEDBACK_TEXT     );
    m_pComplaints   = (ui_text*)FindChildByID( IDC_COMPLAINTS_TEXT   );
    
    m_pPlayerName   ->UseSmallText( TRUE );
    m_pSessionDate  ->UseSmallText( TRUE );
    //m_pFeedback     ->UseSmallText( TRUE );
    //m_pComplaints   ->UseSmallText( TRUE );

    m_pPlayerName   ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pSessionDate  ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pFeedback     ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pComplaints   ->SetLabelFlags( ui_font::h_left|ui_font::v_center );

    m_pPlayerName   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pSessionDate  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pFeedback     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pComplaints   ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    m_pPlayerName   ->SetLabelColor( xcolor(255,252,204,255) );
    m_pSessionDate  ->SetLabelColor( xcolor(255,252,204,255) );
    m_pFeedback     ->SetLabelColor( xcolor(255,252,204,255) );
    m_pComplaints   ->SetLabelColor( xcolor(255,252,204,255) );

    xwstring TempName = g_StringTableMgr( "ui", "IDS_FEEDBACK_NAME" );
    u64 Identifier;

    TempName += xwstring(" ");
    const char* pFeedbackName = g_StateMgr.GetFeedbackPlayer( Identifier );
    TempName += xwstring(pFeedbackName);
    m_pPlayerName->SetLabel( TempName );


    split_date TimeStamp = eng_SplitDate( eng_GetDate() );
    xwstring TempSession = g_StringTableMgr( "ui", "IDS_FEEDBACK_DATE" );
    const xwchar* Month = g_StringTableMgr( "ui", (const char*)xfs("IDS_MONTH%d", TimeStamp.Month));
    
    TempSession += xwstring(" ");
    TempSession += Month;
    TempSession += xwstring( xfs( "% 02i", TimeStamp.Day) );
    m_pSessionDate->SetLabel( TempSession );

    // set up buttons
    m_pButtonGreatSession   = (ui_button*)FindChildByID( IDC_GREAT_SESSION );
    m_pOffensiveMessage     = (ui_button*)FindChildByID( IDC_OFFENSIVE_MESSAGE );
    m_pButtonGoodAttitude   = (ui_button*)FindChildByID( IDC_GOOD_ATTITUDE );
    m_pButtonBadName        = (ui_button*)FindChildByID( IDC_BAD_NAME      );
    m_pButtonCursing        = (ui_button*)FindChildByID( IDC_CURSING       );
    m_pButtonScreaming      = (ui_button*)FindChildByID( IDC_SCREAMING     );
    m_pButtonCheating       = (ui_button*)FindChildByID( IDC_CHEATING      );
    m_pButtonThreats        = (ui_button*)FindChildByID( IDC_THREATS       );

    m_pButtonGreatSession   ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pOffensiveMessage     ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pButtonGoodAttitude   ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pButtonBadName        ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pButtonCursing        ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pButtonScreaming      ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pButtonCheating       ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pButtonThreats        ->SetFlag( ui_win::WF_VISIBLE, FALSE );

    m_pButtonGreatSession   ->SetFlag( ui_win::WF_BUTTON_LEFT, TRUE );
    m_pOffensiveMessage     ->SetFlag( ui_win::WF_BUTTON_LEFT, TRUE );
    m_pButtonGoodAttitude   ->SetFlag( ui_win::WF_BUTTON_LEFT, TRUE );
    m_pButtonBadName        ->SetFlag( ui_win::WF_BUTTON_LEFT, TRUE );
    m_pButtonCursing        ->SetFlag( ui_win::WF_BUTTON_LEFT, TRUE );
    m_pButtonScreaming      ->SetFlag( ui_win::WF_BUTTON_LEFT, TRUE );
    m_pButtonCheating       ->SetFlag( ui_win::WF_BUTTON_LEFT, TRUE );
    m_pButtonThreats        ->SetFlag( ui_win::WF_BUTTON_LEFT, TRUE );

    m_pOffensiveMessage     ->SetFlag( ui_win::WF_DISABLED, TRUE );


    // populate player info (from info setup in players screen)


    GotoControl( (ui_control*)m_pButtonGreatSession );
    m_CurrHL = 0;
    m_PopUp = NULL;

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

void dlg_feedback::ChangeConfig( u8 type )
{ 
    m_Type = type;

    if( m_Type == NORMAL_FEEDBACK )  
    {
        m_pOffensiveMessage     ->SetFlag( ui_win::WF_DISABLED , TRUE  );

        m_pButtonGreatSession   ->SetFlag( ui_win::WF_DISABLED, FALSE );
        m_pButtonGoodAttitude   ->SetFlag( ui_win::WF_DISABLED, FALSE );
        m_pButtonBadName        ->SetFlag( ui_win::WF_DISABLED, FALSE );
        m_pButtonCursing        ->SetFlag( ui_win::WF_DISABLED, FALSE );
        m_pButtonCheating       ->SetFlag( ui_win::WF_DISABLED, FALSE );
    }
    else if( m_Type == ATTACHMENT_FEEDBACK )
    {
        m_pOffensiveMessage     ->SetFlag( ui_win::WF_DISABLED , FALSE );

        m_pButtonGreatSession   ->SetFlag( ui_win::WF_DISABLED, TRUE );
        m_pButtonGoodAttitude   ->SetFlag( ui_win::WF_DISABLED, TRUE );
        m_pButtonBadName        ->SetFlag( ui_win::WF_DISABLED, TRUE );
        m_pButtonCursing        ->SetFlag( ui_win::WF_DISABLED, TRUE );
        m_pButtonCheating       ->SetFlag( ui_win::WF_DISABLED, TRUE );
    }
    else
    {
        ASSERT(0);
    }
}

//=========================================================================

void dlg_feedback::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_feedback::Render( s32 ox, s32 oy )
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

void dlg_feedback::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if( pWin == (ui_win*)m_pButtonGreatSession )
    {
        // send positive feedback - great session
        m_FeedbackType = FB_GREAT_SESSION;
    }

    if( pWin == (ui_win*)m_pButtonGoodAttitude )
    {
        // send positive feedback - good attitude
        m_FeedbackType = FB_GOOD_ATTITUDE;
    }

    if( pWin == (ui_win*)m_pOffensiveMessage )
    {                     
        m_FeedbackType = FB_OFFENSIVE_MESSAGE;
    }

    if( pWin == (ui_win*)m_pButtonBadName )
    {
        // send complaint - Bad name
        m_FeedbackType = FB_BAD_NAME;
    }

    if( pWin == (ui_win*)m_pButtonCursing )
    {
        // send complaint - Cursing
        m_FeedbackType = FB_CURSING;
    }

    if( pWin == (ui_win*)m_pButtonScreaming )
    {
        // send complaint - Screaming
        m_FeedbackType = FB_SCREAMING;
    }

    if( pWin == (ui_win*)m_pButtonCheating )
    {
        // send complaint - Cheating
        m_FeedbackType = FB_CHEATING;
    }

    if( pWin == (ui_win*)m_pButtonThreats )
    {
        // send complaint - Threats
        m_FeedbackType = FB_THREATS;
    }

    // check FB
    if( m_FeedbackType >= FB_BAD_NAME )
    {
        // display confirmation popup
        irect r = g_UiMgr->GetUserBounds( g_UiUserID );
        m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

        // set nav text
        xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
        navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
        m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

#ifdef TARGET_XBOX
        irect Position( 0, 0, 320, 270 );
#else
        irect Position( 0, 0, 280, 240 );
#endif

        m_PopUp->Configure( 
            Position,
            g_StringTableMgr( "ui", "IDS_SEND_FEEDBACK_POPUP" ), 
            TRUE, 
            TRUE, 
            FALSE, 
            g_StringTableMgr( "ui", "IDS_FEEDBACK_MSG" ),
            navText,
            &m_PopUpResult );
    }
    else
    {
        // send positive feedback
        // exit dialog
        u64         Identifier;
        const char* pName;

        pName = g_StateMgr.GetFeedbackPlayer( Identifier );
        // Send negative feedback
        g_MatchMgr.SendFeedback( Identifier, pName, m_FeedbackType );

        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_feedback::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        g_AudioMgr.Play("Backup");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_feedback::OnUpdate ( ui_win* pWin, f32 DeltaTime )
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
            m_pPlayerDetails    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            
            // activate text
            m_pPlayerName       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pSessionDate      ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pFeedback         ->SetFlag(ui_win::WF_VISIBLE, TRUE); 
            m_pComplaints       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText          ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            // activate buttons
            m_pButtonGreatSession   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pButtonGoodAttitude   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pButtonBadName        ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pButtonCursing        ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pButtonScreaming      ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pButtonCheating       ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pButtonThreats        ->SetFlag( ui_win::WF_VISIBLE, TRUE );

            if( m_Type == ATTACHMENT_FEEDBACK )
            {
                m_pOffensiveMessage->SetFlag( ui_win::WF_VISIBLE, TRUE ); 

                m_pButtonGreatSession   ->SetFlag( ui_win::WF_DISABLED, TRUE );
                m_pButtonGoodAttitude   ->SetFlag( ui_win::WF_DISABLED, TRUE );
                m_pButtonBadName        ->SetFlag( ui_win::WF_DISABLED, TRUE );
                m_pButtonCursing        ->SetFlag( ui_win::WF_DISABLED, TRUE );
                m_pButtonCheating       ->SetFlag( ui_win::WF_DISABLED, TRUE );

                GotoControl( (ui_control*)m_pButtonScreaming );
            }
            else
            {
                GotoControl( (ui_control*)m_pButtonGreatSession );
            }            

            g_UiMgr->SetScreenHighlight( m_pButtonGreatSession->GetPosition() );
        }
    }

    if ( m_PopUpResult != DLG_POPUP_IDLE )
    {
        if ( m_PopUpResult == DLG_POPUP_YES )
        {
            u64             Identifier;
            const char*     pName;

            pName = g_StateMgr.GetFeedbackPlayer( Identifier );
            // Send negative feedback
            g_MatchMgr.SendFeedback( Identifier, pName, m_FeedbackType );

            // return to players menu
            m_State = DIALOG_STATE_BACK;
        }
        // clear popup 
        m_PopUp = NULL;

        // turn on nav text
        m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    if( m_pButtonGreatSession->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pButtonGreatSession->GetPosition() );
        highLight = 0;
    }
    else if( m_pButtonGoodAttitude->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pButtonGoodAttitude->GetPosition() );
        highLight = 1;
    }
    else if( m_pButtonBadName->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pButtonBadName->GetPosition() );
        highLight = 2;
    }
    else if( m_pButtonCursing->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pButtonCursing->GetPosition() );
        highLight = 3;
    }
    else if( m_pButtonScreaming->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pButtonScreaming->GetPosition() );
        highLight = 4;
    }
    else if( m_pButtonCheating->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pButtonCheating->GetPosition() );
        highLight = 5;
    }
    else if( m_pButtonThreats->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pButtonThreats->GetPosition() );
        highLight = 6;
    }
    else if( m_pOffensiveMessage->GetFlags(WF_HIGHLIGHT) )
    {
        g_UiMgr->SetScreenHighlight( m_pOffensiveMessage->GetPosition() );
        highLight = 7;
    }


    if( highLight != m_CurrHL )
    {
        if( highLight != -1 )
            g_AudioMgr.Play("Cusor_Norm");

        m_CurrHL = highLight;
    }
}

//=========================================================================
