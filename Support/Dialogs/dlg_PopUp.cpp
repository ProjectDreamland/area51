//=========================================================================
//
//  dlg_PopUp.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_text.hpp"
#include "ui\ui_font.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"

#include "dlg_PopUp.hpp"
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"

#include "NetworkMgr\Voice\VoiceMgr.hpp"
#include "NetworkMgr/NetworkMgr.hpp"

//=========================================================================
//  Main Menu Dialog
//=========================================================================
enum popup_controls
{
    IDC_QUICK_MATCH_NAV_TEXT,
    IDC_NAV_TEXT_BUTTON_0,
    IDC_NAV_TEXT_BUTTON_1,
    IDC_NAV_TEXT_BUTTON_2,
    IDC_NAV_TEXT_BUTTON_3,
};

ui_manager::control_tem PopUpControls[] = 
{
    { IDC_NAV_TEXT_BUTTON_0,     "IDS_NULL", "text", -28, 175, 240, 25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_NAV_TEXT_BUTTON_1,     "IDS_NULL", "text",  46, 175, 240, 25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_NAV_TEXT_BUTTON_2,     "IDS_NULL", "text",   0, 175, 240, 25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_NAV_TEXT_BUTTON_3,     "IDS_NULL", "text",   0, 175, 240, 25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_QUICK_MATCH_NAV_TEXT,  "IDS_NULL", "text",   0, 175, 240, 25, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};

static char* s_pRecordButtonText[4] =
{
    "IDS_NAV_RECORD",
    "IDS_NAV_PLAY",
    "IDS_NAV_BACK",
    "IDS_NAV_SEND",
};

static char* s_pPlayButtonText[4] =
{
    "IDS_NAV_PLAY_PLAY",
    "IDS_NAV_BACK",
};

ui_manager::dialog_tem PopUpDialog =
{
    "IDS_NULL",
    1, 9,
    sizeof(PopUpControls)/sizeof(ui_manager::control_tem),
    &PopUpControls[0],
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

void dlg_popup_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "popup", &PopUpDialog, &dlg_popup_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_popup_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_popup* pDialog = new dlg_popup;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_popup
//=========================================================================

dlg_popup::dlg_popup( void )
{
    m_AskMode = BUDDY_MODE_ADD;
    m_PlayDialogState = PLAY_STATE_NONE;
    m_RecordDialogState = RECORD_STATE_NONE;
    m_PasswordDialogActive = FALSE;
    m_PasswordIndex = 0;
    m_IsVoiceMailPopUp = FALSE;
    x_memset( m_pButtonText, 0, sizeof( m_pButtonText ) );
#ifdef TARGET_XBOX
    m_bIsPopup = 1;
    m_XBOXNotificationOffsetX = 0;
#endif // TARGET_XBOX
}

//=========================================================================

dlg_popup::~dlg_popup( void )
{
    #ifdef TARGET_XBOX
    if( m_IsVoiceMailPopUp == TRUE )
    {
        // Kill Voice Buffer Recording.
        headset& Headset = g_VoiceMgr.GetHeadset();

        // We MUST forcefully stop any recording!
        if( Headset.GetVoiceIsRecording() == TRUE )
            Headset.StopVoiceRecording( TRUE );

        // We MUST forcefully stop any playing!
        if( Headset.GetVoiceIsPlaying() == TRUE )
            Headset.StopVoicePlaying( TRUE );

        if( Headset.GetVoiceMessageRec() != NULL )
            Headset.KillVoiceRecording();

        // Free internal memory used by the voice message
        g_MatchMgr.FreeMessageDownload();
    }
    #endif
}

//=========================================================================

void dlg_popup::Configure( const f32 Timeout, const xwchar* Title, const xwchar* Message, const xwchar* Message2, const xwchar* Message3 )
{
    SetLabel( Title );

    m_Message = Message;

    if( Message2 )
    {
        m_Message2 = Message2;
    }
    else
    {
        m_Message2 = xwstring( "" );
    }

    if( Message3 )
    {
        m_Message3 = Message3;
    }
    else
    {
        m_Message3 = xwstring("");
    }

    // initialize
    m_State       = DIALOG_STATE_ACTIVE;
    m_Timeout     = Timeout;     
    m_bDoTimeout  = TRUE;
    m_bDoBlackout = FALSE;

    // clear input flags
    m_bCheckYes   = FALSE;
    m_bCheckNo    = FALSE;
    m_bCheckMaybe = FALSE;
    m_bCheckHelp  = FALSE;
    m_bCheckOther = FALSE;
}

//=========================================================================

void dlg_popup::Configure( irect& Position, const f32 Timeout, const xwchar* Title, const xwchar* Message, const xwchar* Message2, const xwchar* Message3 )
{
    SetLabel( Title );

    m_Message = Message;

    if( Message2 )
    {
        m_Message2 = Message2;
    }
    else
    {
        m_Message2 = xwstring( "" );
    }

    if( Message3 )
    {
        m_Message3 = Message3;
    }
    else
    {
        m_Message3 = xwstring("");
    }

    // initialize
    m_State       = DIALOG_STATE_ACTIVE;
    if( m_Timeout >= 0.0f )
    {
        m_Timeout     = Timeout;     
        m_bDoTimeout  = TRUE;
    }
    else
    {
        m_bDoTimeout  = FALSE;
    }
    m_bDoBlackout = FALSE;

    // clear input flags
    m_bCheckYes   = FALSE;
    m_bCheckNo    = FALSE;
    m_bCheckMaybe = FALSE;
    m_bCheckHelp  = FALSE;
    m_bCheckOther = FALSE;

    // Set Size of window
    irect Bounds = g_UiMgr->GetUserBounds( g_UiUserID );

    // scale x pos and x size
    Position.l = (s32)( (f32)Position.l * g_UiMgr->GetScaleX() );
    Position.r = (s32)( (f32)Position.r * g_UiMgr->GetScaleX() );

    // center in screen
    Position.Translate( Bounds.GetWidth()/2-Position.GetWidth()/2, Bounds.GetHeight()/2-Position.GetHeight()/2 );
    SetPosition( Position );

    // reposition nav text
    irect NavPos = m_pNavText->GetPosition();
    NavPos.b = Position.b - Position.t;
    NavPos.t = NavPos.b - 25;
    NavPos.l = 0;
    NavPos.r = Position.r - Position.l;
    m_pNavText->SetPosition( NavPos );
}

//=========================================================================

void dlg_popup::Configure( const xwchar* Title, const xbool Yes, const xbool No, const xbool Maybe, const xwchar* Message, const xwchar* pNavText, s32* pResult )
{
    Configure(Title, Yes, No, Maybe, FALSE, Message, pNavText, pResult);
}

//=========================================================================

void dlg_popup::Configure( irect& Position, const xwchar* Title, const xbool Yes, const xbool No, const xbool Maybe, const xwchar* Message, const xwchar* pNavText, s32* pResult )
{
    Configure(Position, Title, Yes, No, Maybe, FALSE, Message, pNavText, pResult);
}

//=========================================================================

void dlg_popup::Configure( const xwchar* Title, const xbool Yes, const xbool No, const xbool Maybe, const xbool Help, const xwchar* Message, const xwchar* pNavText, s32* pResult )
{
    SetLabel( Title );

    m_bCheckYes   = Yes;
    m_bCheckNo    = No;
    m_bCheckMaybe = Maybe;
    m_bCheckHelp  = Help;
    m_bCheckOther = FALSE;

    m_Message       = Message;
    m_Message2      = xwstring( "" );
    m_Message3      = xwstring( "" );

    // set up nav text
    m_pNavText->SetLabel( pNavText );
    m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);

    m_pResult       = pResult;
    m_bDoTimeout    = FALSE;
    m_bDoBlackout   = TRUE;

    // initialize result
    if( m_pResult )
    {
        *m_pResult = DLG_POPUP_IDLE;
    }
}

//=========================================================================

void dlg_popup::Configure( irect& Position, const xwchar* Title, const xbool Yes, const xbool No, const xbool Maybe, const xbool Help, const xwchar* Message, const xwchar* pNavText, s32* pResult )
{
    SetLabel( Title );

    m_bCheckYes   = Yes;
    m_bCheckNo    = No;
    m_bCheckMaybe = Maybe;
    m_bCheckHelp  = Help;
    m_bCheckOther = FALSE;

    m_Message       = Message;
    m_Message2      = xwstring( "" );
    m_Message3      = xwstring( "" );

    // set up nav text
    if( pNavText )
    {
        m_pNavText->SetLabel( pNavText );
        m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
    }

    m_pResult       = pResult;
    m_bDoTimeout    = FALSE;
    m_bDoBlackout   = TRUE;

    // Set Size of window
    irect Bounds = g_UiMgr->GetUserBounds( g_UiUserID );

    // scale x pos and x size
    Position.l = (s32)( (f32)Position.l * g_UiMgr->GetScaleX() );
    Position.r = (s32)( (f32)Position.r * g_UiMgr->GetScaleX() );

    // center in screen
    Position.Translate( Bounds.GetWidth()/2-Position.GetWidth()/2, Bounds.GetHeight()/2-Position.GetHeight()/2 );
    SetPosition( Position );


    // reposition nav text
    irect NavPos = m_pNavText->GetPosition();
    NavPos.b = Position.b - Position.t;
    s32 Height = g_UiMgr->TextHeight( 1, pNavText, 256 );
    if( Height > 22 )
    {
        NavPos.t = NavPos.b - 40;
    }
    else
    {
        NavPos.t = NavPos.b - 25;
    }
    NavPos.l = 0;
    NavPos.r = Position.r - Position.l;
    m_pNavText->SetPosition( NavPos );

    // initialize result
    if( m_pResult )
    {
        *m_pResult = DLG_POPUP_IDLE;
    }
}

//=========================================================================

void dlg_popup::ConfigureAutosaveDialog( irect& Position, const xwchar* Title, const xwchar* Message, const xwchar* pNavText, s32* pResult /* = NULL  */)
{
    SetLabel( Title );

    m_bCheckYes   = TRUE;
    m_bCheckNo    = TRUE;
    m_bCheckMaybe = FALSE;
    m_bCheckHelp  = FALSE;
    m_bCheckOther = FALSE;

    m_Message       = Message;
    m_Message2      = xwstring( "" );
    m_Message3      = xwstring( "" );

    // set up nav text
    if( pNavText )
    {
        m_pNavText->SetLabel( pNavText );
        m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
    }

    m_pResult       = pResult;
    m_bDoTimeout    = FALSE;
    m_bDoBlackout   = TRUE;

    // Set Size of window
    irect Bounds = g_UiMgr->GetUserBounds( g_UiUserID );

    // scale x pos and x size
    Position.l = (s32)( (f32)Position.l * g_UiMgr->GetScaleX() );
    Position.r = (s32)( (f32)Position.r * g_UiMgr->GetScaleX() );

    // center in screen
    Position.Translate( Bounds.GetWidth()/2-Position.GetWidth()/2, Bounds.GetHeight()/2-Position.GetHeight()/2 );
    SetPosition( Position );

    // reposition nav text
    irect NavPos = m_pNavText->GetPosition();
    NavPos.b = Position.b - Position.t;
    if (x_GetLocale() == XL_LANG_ENGLISH)
    {
        NavPos.t = NavPos.b - 25;
    }
    else
    {
        NavPos.t = NavPos.b - 50;
    }
    NavPos.l = 0;
    NavPos.r = Position.r - Position.l;
    m_pNavText->SetPosition( NavPos );

    // initialize result
    if( m_pResult )
    {
        *m_pResult = DLG_POPUP_IDLE;
    }
}

//=========================================================================

void dlg_popup::ConfigureRecordDialog( irect& Position, const xwchar* Title, const xwchar* Message, s32* pResult /* = NULL  */)
{
    SetLabel( Title );

    m_IsVoiceMailPopUp = TRUE;

    m_bCheckYes   = TRUE;
    m_bCheckNo    = TRUE;
    m_bCheckMaybe = TRUE;
    m_bCheckHelp  = FALSE;
    m_bCheckOther = TRUE;
    m_RecordDialogState = RECORD_STATE_INIT;
    m_AskMode = BUDDY_MODE_ADD;

    m_ProgressValue = 0.0f;

    m_Message       = Message;
    m_Message2      = xwstring( "" );
    m_Message3      = xwstring( "" );

    m_pResult       = pResult;
    m_bDoTimeout    = FALSE;
    m_bDoBlackout   = TRUE;
    m_EnableRecord  = FALSE;
    m_EnablePlay    = FALSE;
    m_EnableSend    = FALSE;
    m_ShowStop      = FALSE;

    irect Bounds = g_UiMgr->GetUserBounds( g_UiUserID );

    // scale x pos and x size
    Position.l = (s32)( (f32)Position.l * g_UiMgr->GetScaleX() );
    Position.r = (s32)( (f32)Position.r * g_UiMgr->GetScaleX() );

    // center in screen
    Position.Translate( Bounds.GetWidth()/2-Position.GetWidth()/2, Bounds.GetHeight()/2-Position.GetHeight()/2 );
    SetPosition( Position );

    s32 iFont            = g_UiMgr->FindFont( "small" );
    s32 TotalStringWidth = 0;
    s32 Widths [4];
    s32 Offsets[4];

    // English gets wider spacing between strings
    s32 Spacing = (x_GetLocale() == XL_LANG_ENGLISH) ? 25 : 15;

    // Determine the maximum number of pixels needed horizontally for the 4 strings
    for( s32 j=0; j<4; j++ )
    {
        xwstring NavText( g_StringTableMgr( "ui", s_pRecordButtonText[j] ) );

        s32 StringWidth = g_UiMgr->TextWidth( iFont, NavText );
        Widths [j] = StringWidth;
        Offsets[j] = TotalStringWidth;

        StringWidth += Spacing;

        TotalStringWidth += StringWidth;
    }

    s32 Left = -(TotalStringWidth / 2);

    for( s32 i=0; i < 4; i++ )
    {
        xwstring NavText( g_StringTableMgr( "ui", s_pRecordButtonText[i] ) );

        irect NavPos = m_pButtonText[i]->GetPosition();

        NavPos.l = Left + Offsets[i] + (Widths[i] / 2);
        NavPos.b = Position.b - Position.t;
        NavPos.t = NavPos.b - 25;
        NavPos.r = NavPos.l + (Position.r - Position.l);

        m_pButtonText[i]->SetPosition( NavPos  );
        m_pButtonText[i]->SetLabel   ( NavText );
        m_pButtonText[i]->SetFlag    ( ui_win::WF_VISIBLE, TRUE );
    }

    // initialize result
    if( m_pResult )
    {
        *m_pResult = DLG_POPUP_IDLE;
    }
}

//=========================================================================

void dlg_popup::ConfigurePlayDialog( irect& Position, const xwchar* Title, const xwchar* Message, const xwchar* pNavText, s32* pResult /* = NULL  */)
{
    SetLabel( Title );

    m_IsVoiceMailPopUp = TRUE;

    m_bCheckYes   = TRUE;
    m_bCheckNo    = TRUE;
    m_bCheckMaybe = TRUE;
    m_bCheckHelp  = FALSE;
    m_bCheckOther = TRUE;
    m_PlayDialogState = PLAY_STATE_START_DOWNLOAD;

    m_ProgressValue = 0.0f;

    m_Message       = Message;
    m_Message2      = xwstring( "" );
    m_Message3      = xwstring( "" );

    m_pResult       = pResult;
    m_bDoTimeout    = FALSE;
    m_bDoBlackout   = TRUE;
    m_EnablePlay    = FALSE;
    m_ShowStop      = FALSE;

    irect Bounds = g_UiMgr->GetUserBounds( g_UiUserID );

    // scale x pos and x size
    Position.l = (s32)( (f32)Position.l * g_UiMgr->GetScaleX() );
    Position.r = (s32)( (f32)Position.r * g_UiMgr->GetScaleX() );

    // center in screen
    Position.Translate( Bounds.GetWidth()/2-Position.GetWidth()/2, Bounds.GetHeight()/2-Position.GetHeight()/2 );
    SetPosition( Position );

    if( pNavText )
    {
        m_pNavText->SetLabel( pNavText );
        m_pNavText->SetFlag (ui_win::WF_VISIBLE, TRUE);
    }

    // reposition "From: Player" text
    irect FromPos = m_pNavText->GetPosition();
    FromPos.b = (Position.b - Position.t) / 2;
    FromPos.t = FromPos.b - 25;
    FromPos.r = (Position.r - Position.l);
    m_pNavText->SetPosition( FromPos );

    for( s32 i=0; i < 2; i++ )
    {
        xwstring NavText( g_StringTableMgr( "ui", s_pPlayButtonText[i] ) );

        irect NavPos = m_pButtonText[i]->GetPosition();
        NavPos.b = Position.b - Position.t;
        NavPos.t = NavPos.b - 25;
        NavPos.r = NavPos.l + (Position.r - Position.l);

        m_pButtonText[i]->SetPosition( NavPos  );
        m_pButtonText[i]->SetLabel   ( NavText );
        m_pButtonText[i]->SetFlag    ( ui_win::WF_VISIBLE, TRUE );
    }

    // initialize result
    if( m_pResult )
    {
        *m_pResult = DLG_POPUP_IDLE;
    }
}

//=========================================================================

void dlg_popup::SetBuddy( const buddy_info& Buddy )
{
    m_Buddy = Buddy;
}

//=========================================================================

buddy_info& dlg_popup::GetBuddy( void )
{
    return( m_Buddy );
}

//=========================================================================

void dlg_popup::SetBuddyMode( s32 mode )
{
    m_AskMode = mode;
}

//=========================================================================

void dlg_popup::Close( void )
{
    ui_win* pParent = GetParent();

    if( pParent ) 
    {
        delete this;
    }
    else
    {
        g_UiMgr->EndDialog(m_UserID,TRUE);
    }
}

//=========================================================================


xbool dlg_popup::Create( s32                        UserID,
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

    // Set Size of window
    irect r( 0, 0, 240, 200 );

    // scale x pos and x size
    r.l = (s32)( (f32)r.l * g_UiMgr->GetScaleX() );
    r.r = (s32)( (f32)r.r * g_UiMgr->GetScaleX() );
    r.t = (s32)( (f32)r.t * g_UiMgr->GetScaleY() );
    r.b = (s32)( (f32)r.b * g_UiMgr->GetScaleY() );

    // center in screen
    r.Translate( Position.GetWidth()/2-r.GetWidth()/2, Position.GetHeight()/2-r.GetHeight()/2 );

    // Do dialog creation
	Success = ui_dialog::Create( UserID, pManager, pDialogTem, r, pParent, Flags );
   
    // Initialize nav text
    m_pNavText = (ui_text*)FindChildByID( IDC_QUICK_MATCH_NAV_TEXT );
    m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text|ui_font::set_position );
    m_pNavText->UseSmallText(TRUE);
    m_pNavText->SetLabelColor( xcolor(255,252,204,255) );

    for( s32 i=0; i < 4; i++ )
    {
        m_pButtonText[i] = (ui_text*)FindChildByID( IDC_NAV_TEXT_BUTTON_0 + i );

        m_pButtonText[i]->SetFlag      ( ui_win::WF_VISIBLE, FALSE );
        m_pButtonText[i]->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text|ui_font::set_position );
        m_pButtonText[i]->UseSmallText ( TRUE );
    }

    // Initialize Data
    m_iElement = m_pManager->FindElement( "frame2" );
    ASSERT( m_iElement != -1 );

    m_BackgroundColor   = xcolor (19,59,14,255); 
    m_Timeout           = -1.0f;     
    m_bDoTimeout        = FALSE;
    m_bDoBlackout       = TRUE;

    // Clear pointer to result code
    m_pResult = NULL;

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // play create sound
    g_AudioMgr.Play( "Dialog" );

	// Return success code
    return Success;
}

//=========================================================================

void dlg_popup::Destroy( void )
{
    ui_dialog::Destroy();
}

//=========================================================================

void dlg_popup::Render( s32 ox, s32 oy )
{
    // dim the background dialog
    if( m_bDoBlackout )
    {
	    s32 XRes, YRes;
        eng_GetRes(XRes, YRes);
#ifdef TARGET_PS2
        // Nasty hack to force PS2 to draw to rb.l = 0
        irect rb( -1, 0, XRes, YRes );
#else
        irect rb( 0, 0, XRes, YRes );
#endif
        g_UiMgr->RenderGouraudRect(rb, xcolor(0,0,0,180),
                                       xcolor(0,0,0,180),
                                       xcolor(0,0,0,180),
                                       xcolor(0,0,0,180),FALSE);
    }

    // Get window rectangle
    irect   r = m_Position;

    // Render background color
    if( m_BackgroundColor.A > 0 )
    {
        irect   rb = r;
        rb.Deflate( 1, 1 );
        m_pManager->RenderRect( rb, m_BackgroundColor, FALSE );
    }

    // Render Title
    irect rect = r;
    rect.Deflate( 1, 1 );
    rect.SetHeight( 22 );
    xcolor c1 = xcolor (25,77,18,255); 
    xcolor c2 = xcolor (25,77,18,255); 
    m_pManager->RenderGouraudRect( rect, c1, c1, c2, c2, FALSE );

    rect.Deflate( 8, 0 );
    rect.Translate( 1, -1 );
    m_pManager->RenderText( 0, rect, ui_font::h_center|ui_font::v_center, xcolor(XCOLOR_BLACK), m_Label );
    rect.Translate( -1, -1 );
    m_pManager->RenderText( 0, rect, ui_font::h_center|ui_font::v_center, xcolor(255,252,204,255), m_Label );

    // Render frame
    m_pManager->RenderElement( m_iElement, r, 0 );
    

    // Render message
    rect = r;
    rect.Deflate(5, 30);
    rect.Translate( 1, -1 );
    g_UiMgr->RenderText_Wrap( 1, rect, ui_font::h_center|ui_font::v_top, xcolor(XCOLOR_BLACK), m_Message );
    rect.Translate( -1, -1 );
    g_UiMgr->RenderText_Wrap( 1, rect, ui_font::h_center|ui_font::v_top, xcolor(255,252,204,255), m_Message );

    if (m_Message2)
    {
        rect = r;
        rect.Deflate(5, 70);
        rect.Translate( 1, -1 );
        g_UiMgr->RenderText_Wrap( 1, rect, ui_font::h_center|ui_font::v_top, xcolor(XCOLOR_BLACK), m_Message2 );
        rect.Translate( -1, -1 );
        g_UiMgr->RenderText_Wrap( 1, rect, ui_font::h_center|ui_font::v_top, xcolor(255,252,204,255), m_Message2 );
    }
    
    if (m_Message3)
    {
        rect = r;
        rect.Deflate(5, 110);
        rect.Translate( 1, -1 );
        g_UiMgr->RenderText_Wrap( 1, rect, ui_font::h_center|ui_font::v_top, xcolor(XCOLOR_BLACK), m_Message3 );
        rect.Translate( -1, -1 );
        g_UiMgr->RenderText_Wrap( 1, rect, ui_font::h_center|ui_font::v_top, xcolor(255,252,204,255), m_Message3 );
    }

    if( m_RecordDialogState != RECORD_STATE_NONE || m_PlayDialogState != PLAY_STATE_NONE )
    {
        if( m_PlayDialogState != PLAY_STATE_INVALID_MESSAGE )
        {        
            // render a rect inside a rect ...
            irect rBar;
            rBar = r;
            rBar.Deflate( 20, 1 );
            rBar.SetHeight( 22 );
            rBar.Translate(0,140);
            g_UiMgr->RenderRect(rBar,xcolor (25,77,18,255),FALSE);

            // ? Render Progress Bar
            if( m_RecordDialogState == RECORD_STATE_RECORD || 
                m_RecordDialogState == RECORD_STATE_SENDING ||
                m_RecordDialogState == RECORD_STATE_PLAY ||
                m_PlayDialogState == PLAY_STATE_DOWNLOADING ||
                m_PlayDialogState == PLAY_STATE_PLAY
                )
            {
                rBar.Deflate( 5, 5 );
                s32 progress = (s32)(rBar.GetWidth() * m_ProgressValue);
                rBar.r = rBar.l + progress;
                g_UiMgr->RenderRect(rBar,xcolor (25,180,18,255),FALSE);
            }
        }
    }

    // If in Password mode then lets render out the passcodes
    if( m_PasswordDialogActive )
    {
        //
        irect rBar;
        rBar = r;
        rBar.Deflate( 20, 1 );
        rBar.SetHeight( 22 );
        rBar.Translate(0,140);
        g_UiMgr->RenderRect(rBar,xcolor (25,77,18,255),FALSE);
        rBar.Deflate( 5, 5 );

        xwstring navText;
        if( m_Password[0] != 0 )
            navText =  g_StringTableMgr( "ui", "IDS_PASSWORD_HIDDEN_KEY" );
        if( m_Password[1] != 0 )
            navText += g_StringTableMgr( "ui", "IDS_PASSWORD_HIDDEN_KEY" );
        if( m_Password[2] != 0 )
            navText += g_StringTableMgr( "ui", "IDS_PASSWORD_HIDDEN_KEY" );
        if( m_Password[3] != 0 )
            navText += g_StringTableMgr( "ui", "IDS_PASSWORD_HIDDEN_KEY" );

        g_UiMgr->RenderText_Wrap( 1, rBar, ui_font::h_center|ui_font::v_top, xcolor(XCOLOR_BLACK), navText );
        rBar.Translate( -1, -1 );
        g_UiMgr->RenderText_Wrap( 1, rBar, ui_font::h_center|ui_font::v_top, xcolor(255,252,204,255), navText );
    }

    // Setup control buttons on Recording/Playback dialog
    {
        // Show STOP button if there is a voice mail in progress
        xwstring Text;
        if( m_ShowStop == TRUE )
            Text = g_StringTableMgr( "ui", "IDS_NAV_STOP" );
        else
            Text = g_StringTableMgr( "ui", "IDS_NAV_BACK" );

        xwstring Title( g_StringTableMgr( "ui", "IDS_RECORD_ATTACHMENT" ) );
        xbool    IsRecordDialog = (x_wstrcmp( GetLabel(), Title ) == 0);

        if( IsRecordDialog == TRUE )
        {
            m_pButtonText[2]->SetLabel( Text );

            // Disable buttons that can't be used
            m_pButtonText[0]->SetFlag( ui_win::WF_DISABLED, !m_EnableRecord );
            m_pButtonText[1]->SetFlag( ui_win::WF_DISABLED, !m_EnablePlay   );
            m_pButtonText[3]->SetFlag( ui_win::WF_DISABLED, !m_EnableSend   );
        }
        else
        {
            m_pButtonText[0]->SetFlag( ui_win::WF_DISABLED, !m_EnablePlay   );
            m_pButtonText[1]->SetLabel( Text );
        }
    }


    // Render children
    for( s32 i=0 ; i<m_Children.GetCount() ; i++ )
    {
        m_Children[i]->Render( m_Position.l+ox, m_Position.t+oy );
    }
}

//=========================================================================

void dlg_popup::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    if( m_RecordDialogState != RECORD_STATE_NONE )
        OnUpdateRecordDialog( DeltaTime );
    else if( m_PlayDialogState != PLAY_STATE_NONE )
        OnUpdatePlayDialog( DeltaTime );

    if(m_PasswordDialogActive)
        OnUpdatePasswordDialog( DeltaTime );

    // check if the popup has a timeout
    if( m_bDoTimeout )
    {
        // update the timeout
        if( m_Timeout > DeltaTime )
        {
            m_Timeout -= DeltaTime;
        }
        else
        {
            ui_win* pParent = GetParent();
    
            if( pParent ) 
            {
                m_Timeout = 0.0f;
                m_State = DIALOG_STATE_TIMEOUT;
            }
            else
            {
                // Close dialog
                m_pManager->EndDialog( m_UserID, TRUE );
            }
        }
    }
}

//=========================================================================

void dlg_popup::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    // no early outs for timed popups
    if( m_bDoTimeout )
        return;

    // No input if scaling
    if( g_UiMgr->IsScreenScaling() )
    {
        return;
    }
    // record dialog override
    if( m_RecordDialogState != RECORD_STATE_NONE )
    {
        OnPadRecord();
        return;
    }
    else if ( m_PlayDialogState != PLAY_STATE_NONE )
    {
        OnPadRecord(); // Accept Friend
        return;
    }

    if( m_PasswordDialogActive )
    {
        if( 
            m_Password[0] == m_CheckPassword[0] &&  
            m_Password[1] == m_CheckPassword[1] &&  
            m_Password[2] == m_CheckPassword[2] &&  
            m_Password[3] == m_CheckPassword[3] 
            )
        {
            // check input flag

            // Set result and close dialog
            if( m_pResult )
                *m_pResult = DLG_POPUP_YES;

            g_AudioMgr.Play( "Select_Norm" );
            g_UiMgr->EndDialog( m_UserID, TRUE );
            return;
        }
        else
        {
            // Play ERROR SOUND
            g_AudioMgr.Play( "Select_Norm" );
            g_UiMgr->EndDialog( m_UserID, TRUE );
            return;
        }
    }

    // check input flag
    if( m_bCheckYes )
    {
        // Set result and close dialog
        if( m_pResult )
            *m_pResult = DLG_POPUP_YES;

        // Follow buddy needs to know which controller was used.
        g_StateMgr.SetControllerRequested(g_UiMgr->GetActiveController(), TRUE );

        g_AudioMgr.Play( "Select_Norm" );
        g_UiMgr->EndDialog( m_UserID, TRUE );
    }
}

//=========================================================================

void dlg_popup::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    // no early outs for timed popups
    if( m_bDoTimeout )
        return;

    // No input if scaling
    if( g_UiMgr->IsScreenScaling() )
    {
        return;
    }
    // record dialog override
    if( m_RecordDialogState != RECORD_STATE_NONE || m_PlayDialogState != PLAY_STATE_NONE )
    {
        OnPadStop();
        return;
    }

    if( m_PasswordDialogActive )
    {
        // Remove pass key and if its the last then back out of the popup.
        if( RemovePassKey() )
        {
            // Player is backing out off enter Passcode.
            if( m_pResult )
                *m_pResult = DLG_POPUP_NO;

        }
        else
        {
            return;
        }
    }


    if( m_bCheckNo )
    {
        if( m_pResult )
            *m_pResult = DLG_POPUP_NO;

        g_AudioMgr.Play( "Select_Norm" );
        g_UiMgr->EndDialog( m_UserID, TRUE );
    }
}

//=========================================================================

void dlg_popup::OnPadDelete( ui_win* pWin )
{
    (void)pWin;

    // no early outs for timed popups
    if( m_bDoTimeout )
        return;

    // No input if scaling
    if( g_UiMgr->IsScreenScaling() )
    {
        return;
    }
    // record dialog override
    if( m_RecordDialogState != RECORD_STATE_NONE )
    {
        OnPadPlay();
        return;
    }
    else if ( m_PlayDialogState != PLAY_STATE_NONE )
    {
        OnPadPlay();
        return;
    }

    if( m_PasswordDialogActive )
    {
        // Add X to Password List.
#ifdef TARGET_XBOX
        AddPassKey(XONLINE_PASSCODE_GAMEPAD_X);
#endif
        return;
    }

    // Set result and close dialog
    if( m_bCheckMaybe )
    {
        if( m_pResult )
            *m_pResult = DLG_POPUP_MAYBE;

        g_AudioMgr.Play( "Select_Norm" );
        g_UiMgr->EndDialog( m_UserID, TRUE );
    }
}

//=========================================================================

void dlg_popup::OnPadHelp( ui_win* pWin )
{
    (void)pWin;

    // no early outs for timed popups
    if( m_bDoTimeout )
        return;

    // No input if scaling
    if( g_UiMgr->IsScreenScaling() )
    {
        return;
    }
    // check input flag
    if( m_bCheckHelp )
    {
        // Set result and close dialog
        if( m_pResult )
            *m_pResult = DLG_POPUP_HELP; 

        g_AudioMgr.Play( "Select_Norm" );
        g_UiMgr->EndDialog( m_UserID, TRUE );
    }
}

//=========================================================================

void dlg_popup::OnPadActivate( ui_win* pWin )
{
    (void)pWin;
    
    // no early outs for timed popups
    if( m_bDoTimeout )
        return;

    // No input if scaling
    if( g_UiMgr->IsScreenScaling() )
    {
        return;
    }
    // record dialog override
    if( m_RecordDialogState != RECORD_STATE_NONE )
    {
        OnPadSend();
        return;
    }
    else if( m_PlayDialogState != PLAY_STATE_NONE )
    {
        OnPadSend(); // Decline Player.
        return;
    }
    
    if( m_PasswordDialogActive )
    {
        // Add X to Password List.
#ifdef TARGET_XBOX
        AddPassKey(XONLINE_PASSCODE_GAMEPAD_Y);
#endif
        return;
    }

    // Set result and close dialog
    if( m_bCheckOther )
    {
        if( m_pResult )
            *m_pResult = DLG_POPUP_OTHER;

        g_AudioMgr.Play( "Select_Norm" );
        g_UiMgr->EndDialog( m_UserID, TRUE );
    }
}

//========================================================================

void dlg_popup::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX /* = FALSE  */, xbool WrapY /* = FALSE */)
{
    (void)pWin;
    (void)Code;
    (void)Presses;
    (void)Repeats;
    (void)WrapX;
    (void)WrapY;
#ifdef TARGET_XBOX
    if( !m_PasswordDialogActive )
    {
        return;
    }

    switch( Code )
    {
        case ui_manager::NAV_LEFT:  AddPassKey(XONLINE_PASSCODE_DPAD_LEFT);  break;
        case ui_manager::NAV_RIGHT: AddPassKey(XONLINE_PASSCODE_DPAD_RIGHT); break;
        case ui_manager::NAV_UP:    AddPassKey(XONLINE_PASSCODE_DPAD_UP);    break;
        case ui_manager::NAV_DOWN:  AddPassKey(XONLINE_PASSCODE_DPAD_DOWN);  break;
    }
#endif
}

//========================================================================

void dlg_popup::OnPadShoulder( ui_win* pWin, s32 Direction )
{
    (void)pWin;
    (void)Direction;
#ifdef TARGET_XBOX
    if( !m_PasswordDialogActive )
    {
        return;
    }

    switch( Direction )
    {
        case -1: // LEFT?
            AddPassKey(XONLINE_PASSCODE_GAMEPAD_LEFT_TRIGGER);
        break;
        case 1: // RIGHT?
            AddPassKey(XONLINE_PASSCODE_GAMEPAD_RIGHT_TRIGGER);
        break;
    }
#endif
}

//=========================================================================

void dlg_popup::OnUpdateRecordDialog( f32 DeltaTime )
{
    (void)DeltaTime;

#ifdef TARGET_XBOX

    headset& Headset = g_VoiceMgr.GetHeadset();

    // Dynamically enable and disable dialog control buttons
    {
        xbool OperationInProgress = Headset.GetVoiceIsPlaying() ||
                                    Headset.GetVoiceIsRecording();

        m_ShowStop     =  OperationInProgress;
        m_EnableSend   = !OperationInProgress;
        m_EnableRecord = !OperationInProgress && (Headset.IsHardwarePresent() == TRUE);
        m_EnablePlay   = !OperationInProgress && (Headset.GetVoiceNumBytesRec() > 0);
    }

    switch( m_RecordDialogState )
    {
        case RECORD_STATE_NONE:

        break;

        case RECORD_STATE_INIT:

            Headset.InitVoiceRecording();
            m_RecordDialogState = RECORD_STATE_IDLE;

        break;

        case RECORD_STATE_RECORD_INIT:
        {
            m_ProgressValue = 0.0f;

            if( Headset.IsHardwarePresent() == TRUE )
            {
                Headset.StartVoiceRecording();
                m_RecordDialogState = RECORD_STATE_RECORD;
            }
            else
            {
                m_RecordDialogState = RECORD_STATE_IDLE;
            }
        }
        break;

        case RECORD_STATE_RECORD:

            if( Headset.GetVoiceIsRecording() == FALSE )
            {
                // Done Recording
                m_RecordDialogState = RECORD_STATE_STOP;
                m_ProgressValue     = 0.0f;
            }
            else
            {        
                // Show Progress bar.
                m_ProgressValue = Headset.GetVoiceRecordingProgress();
            }

        break;

        case RECORD_STATE_PLAY_INIT:  
        {
            s32 NumBytes   = Headset.GetVoiceNumBytesRec();
            s32 DurationMS = Headset.GetVoiceDurationMS();

            m_ProgressValue = 0.0f;

            if( NumBytes > 0 )
            {
                byte*   pVoiceMessage = Headset.GetVoiceMessageRec();
                ASSERT( pVoiceMessage != NULL );
                Headset.StartVoicePlaying( pVoiceMessage, DurationMS, NumBytes );
                m_RecordDialogState = RECORD_STATE_PLAY;
            }
            else
            {
                m_RecordDialogState = RECORD_STATE_IDLE;
            }
        }
        break;

        case RECORD_STATE_PLAY:

            if( Headset.GetVoiceIsPlaying() == FALSE )
            {
                // Finished with the voice message so destroy it
                m_RecordDialogState = RECORD_STATE_STOP_PLAYBACK;
                m_ProgressValue     = 0.0f;
            }
            else
            {
                // Print Play Progress
                m_ProgressValue = Headset.GetVoicePlayingProgress();
            }   

        break;

        case RECORD_STATE_STOP:
        {                        
            Headset.StopVoiceRecording();
            m_RecordDialogState = RECORD_STATE_IDLE;
        }
        break;

        case RECORD_STATE_STOP_PLAYBACK:
        {
            g_VoiceMgr.GetHeadset().StopVoicePlaying();
            m_RecordDialogState = RECORD_STATE_IDLE;
        }
        break;
        
        case RECORD_STATE_SEND:
        {            
            // Only allow voice mail to be sent if there is any data
            if( Headset.GetVoiceNumBytesRec() > 0 )
            {
                // Set voice attachment to be sent
                f32 Duration = Headset.GetVoiceDurationMS() / 1000.0f;
                g_MatchMgr.SetVoiceMessage( Headset.GetVoiceMessageRec(), Headset.GetVoiceNumBytesRec(), Duration );
            }

            if( m_AskMode == BUDDY_MODE_ADD )
            {
                g_MatchMgr.AddBuddy( m_Buddy );
                m_RecordDialogState = RECORD_STATE_SENDING;
            }
            else
            {
                g_MatchMgr.InviteBuddy( m_Buddy );
                m_RecordDialogState = RECORD_STATE_SENDING;
            }

            m_ProgressValue = 0.0f;
        }
        break;
        
        case RECORD_STATE_SENDING:

            if( g_MatchMgr.IsSendingMessage() == TRUE )
            {
                m_ProgressValue = g_MatchMgr.GetMessageSendProgress();
            }
            else
            {
                m_ProgressValue = 0.0f;
                g_MatchMgr.ClearVoiceMessage();

                // Done..
                if( m_pResult )
                    *m_pResult = DLG_POPUP_OTHER;
            }

        break;

        case RECORD_STATE_EXIT:
            
            if( m_pResult )
                *m_pResult = DLG_POPUP_NO;

        break;

        case RECORD_STATE_IDLE:
            m_ProgressValue = 0.0f;
        break;

        default:
        break;
    }

    if( m_pResult )
    {
        if( *m_pResult != DLG_POPUP_IDLE )
        {
            if( g_UiMgr->GetTopmostDialog( m_UserID ) == this )
                g_UiMgr->EndDialog( m_UserID, TRUE );
        }
    }

#endif
}

//=========================================================================

void dlg_popup::OnUpdatePlayDialog( f32 DeltaTime )
{
    (void)DeltaTime;
#ifdef TARGET_XBOX

    headset& Headset = g_VoiceMgr.GetHeadset();

    // Disable Play button if we are downloading or playing message
    m_EnablePlay = (m_PlayDialogState != PLAY_STATE_DOWNLOADING) &&
                   (Headset.GetVoiceIsPlaying() == FALSE);
    m_ShowStop   =  Headset.GetVoiceIsPlaying();

    switch( m_PlayDialogState )
    {
        case PLAY_STATE_NONE:

        break;

        case PLAY_STATE_IDLE:
            m_ProgressValue = 0.0f;
        break;

        case PLAY_STATE_START_DOWNLOAD:

            // Attempt to start the voice attachment download.
            if( g_MatchMgr.StartMessageDownload( m_Buddy ) == TRUE )
            {
                m_PlayDialogState = PLAY_STATE_DOWNLOADING;
            }
            else
            {
                // TODO: We need a dialog that says "This message is no longer available".
                m_PlayDialogState = PLAY_STATE_INVALID_MESSAGE;

                // Change the popup message.
                m_Message       =    g_StringTableMgr("ui", "IDS_INVALID_MESSAGE");
                xwstring NavText( g_StringTableMgr( "ui", "IDS_NAV_OK" ) );
                m_pButtonText[0]->SetLabel  ( NavText );
                irect NavTextPos = m_pButtonText[0]->GetPosition();
                NavTextPos.l = m_pNavText->GetPosition().l;
                NavTextPos.r = m_pNavText->GetPosition().r;
                m_pButtonText[0]->SetPosition(NavTextPos);
                m_pButtonText[1]->SetFlag( ui_win::WF_VISIBLE, FALSE);
                m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);
            }

        break;

        case PLAY_STATE_INVALID_MESSAGE:
        {
            // wait for OK input.
        }
        break;

        case PLAY_STATE_DOWNLOADING:

            if( g_MatchMgr.IsDownloadingMessage() == TRUE )
            {
                m_ProgressValue = g_MatchMgr.GetMessageRecProgress();
            }
            else
            {
                m_ProgressValue   = 0.0f;
                m_PlayDialogState = PLAY_STATE_IDLE;
            }

        break;

        case PLAY_STATE_START_PLAY:
        {
            byte* pVoiceMessage = NULL;
            s32   NumBytes;
            f32   Duration;

            g_MatchMgr.GetVoiceMessage( &pVoiceMessage, &NumBytes, &Duration );
            ASSERT( pVoiceMessage != NULL );

            if( NumBytes > 0 )
            {
                Headset.StartVoicePlaying( pVoiceMessage, s32(Duration * 1000.0f), NumBytes );
                m_PlayDialogState = PLAY_STATE_PLAY;
            }
        }
        break;

        case PLAY_STATE_PLAY:

            if( Headset.GetVoiceIsPlaying() == FALSE )
            {
                // Finished with the voice message so destroy it
                m_PlayDialogState = PLAY_STATE_STOP;
                m_ProgressValue   = 0.0f;
            }
            else
            {
                // Print Play Progress
                m_ProgressValue = Headset.GetVoicePlayingProgress();
            }

        break;

        case PLAY_STATE_STOP:
        {
            Headset.StopVoicePlaying();
            m_PlayDialogState = PLAY_STATE_IDLE;
        }
        break;

        case PLAY_STATE_EXIT:
        {
            if( m_pResult )
                *m_pResult = DLG_POPUP_OTHER;
        }
        break;

        case PLAY_STATE_INVALID_MESSAGE_EXIT:
        {
            if( m_pResult )
                *m_pResult = DLG_POPUP_NO;
        }
        break;
    }

    if( m_pResult )
    {
        if( *m_pResult != DLG_POPUP_IDLE )
        {
            if( g_UiMgr->GetTopmostDialog( m_UserID ) == this )
                g_UiMgr->EndDialog( m_UserID, TRUE );
        }
    }

#endif
}


//=========================================================================

void dlg_popup::OnPadRecord( void )
{
    // OnPadSelect

    if( m_RecordDialogState != RECORD_STATE_NONE )
    {
        
        if( m_RecordDialogState == RECORD_STATE_INIT )
            return;

        if( 
            m_RecordDialogState != RECORD_STATE_RECORD_INIT && 
            m_RecordDialogState != RECORD_STATE_RECORD &&
            m_RecordDialogState != RECORD_STATE_PLAY &&
            m_RecordDialogState != RECORD_STATE_PLAY_INIT
        )
            m_RecordDialogState = RECORD_STATE_RECORD_INIT;
    }
    else
    {
        // ? Invalid Message
        if( m_PlayDialogState == PLAY_STATE_INVALID_MESSAGE )
        {
            // then we need to exit now.
            m_PlayDialogState = PLAY_STATE_INVALID_MESSAGE_EXIT;
            return;
        }

        // Play attachment
        if( 
            m_PlayDialogState != PLAY_STATE_PLAY &&
            m_PlayDialogState != PLAY_STATE_START_DOWNLOAD &&
            m_PlayDialogState != PLAY_STATE_DOWNLOADING
            )
        {
            m_PlayDialogState = PLAY_STATE_START_PLAY;
        }
    }
    return;
}

//=========================================================================

void dlg_popup::OnPadStop( void )
{
    // OnPadBack

    if( m_RecordDialogState != RECORD_STATE_NONE )
    {
        if( m_RecordDialogState == RECORD_STATE_RECORD )
        {
            // Stop Record
            m_RecordDialogState = RECORD_STATE_STOP;
        }
        else if( m_RecordDialogState == RECORD_STATE_PLAY )
        {
            // Stop Playback.
            m_RecordDialogState = RECORD_STATE_STOP_PLAYBACK;
        }
        else
        {
            // Back out.
            m_RecordDialogState = RECORD_STATE_EXIT;
        }
    }
    else
    {
        // Stop playback
        if( m_PlayDialogState == PLAY_STATE_PLAY )
            m_PlayDialogState = PLAY_STATE_STOP;

        if( m_PlayDialogState == PLAY_STATE_IDLE )
            m_PlayDialogState = PLAY_STATE_EXIT;
    }
}

//=========================================================================

void dlg_popup::OnPadPlay( void )
{
    // OnPadDelete
    if( m_RecordDialogState != RECORD_STATE_NONE )
    {
        if( 
            m_RecordDialogState != RECORD_STATE_RECORD_INIT &&
            m_RecordDialogState != RECORD_STATE_RECORD && 
            m_RecordDialogState != RECORD_STATE_PLAY 
            )
        {
            m_RecordDialogState = RECORD_STATE_PLAY_INIT;
        }
    }
}

//=========================================================================

void dlg_popup::OnPadSend( void )
{
    // OnPadActivate
    if( m_RecordDialogState != RECORD_STATE_NONE )
    {
   
        if( m_RecordDialogState != RECORD_STATE_PLAY &&
            m_RecordDialogState != RECORD_STATE_RECORD &&
            m_RecordDialogState != RECORD_STATE_RECORD_INIT &&
            m_RecordDialogState != RECORD_STATE_STOP &&
            m_RecordDialogState != RECORD_STATE_SEND &&
            m_RecordDialogState != RECORD_STATE_SENDING

            )
        {
            // SEND
            m_RecordDialogState = RECORD_STATE_SEND;
        }
    }
}

//========================================================================

void dlg_popup::ConfigurePassword( irect& Position, const xwchar* Title, const xwchar* Message, const xwchar* pNavText, s32* pResult /* = NULL  */)
{
    SetLabel( Title );

    m_bCheckYes   = TRUE;
    m_bCheckNo    = TRUE;
    m_bCheckMaybe = TRUE;
    m_bCheckOther = TRUE;
    m_PasswordDialogActive = TRUE;

    m_PasswordIndex = -1;
    m_Password[0] = 0;
    m_Password[1] = 0;
    m_Password[2] = 0;
    m_Password[3] = 0;

    m_ProgressValue = 0.0f;

    m_Message       = Message;
    m_Message2      = xwstring( "" );
    m_Message3      = xwstring( "" );

    // set up nav text
    if( pNavText )
    {
        m_pNavText->SetLabel( pNavText );
        m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
    }

    m_pResult       = pResult;
    m_bDoTimeout    = FALSE;
    m_bDoBlackout   = TRUE;

    irect Bounds = g_UiMgr->GetUserBounds( g_UiUserID );

    // scale x pos and x size
    Position.l = (s32)( (f32)Position.l * g_UiMgr->GetScaleX() );
    Position.r = (s32)( (f32)Position.r * g_UiMgr->GetScaleX() );

    // center in screen
    Position.Translate( Bounds.GetWidth()/2-Position.GetWidth()/2, Bounds.GetHeight()/2-Position.GetHeight()/2 );
    SetPosition( Position );

    // reposition nav text
    irect NavPos = m_pNavText->GetPosition();
    NavPos.b = Position.b - Position.t;
    NavPos.t = NavPos.b - 25;
    NavPos.l = 0;
    NavPos.r = Position.r - Position.l;
    m_pNavText->SetPosition( NavPos );

    // initialize result
    if( m_pResult )
    {
        *m_pResult = DLG_POPUP_IDLE;
    }
}

//========================================================================

void dlg_popup::OnUpdatePasswordDialog( f32 DelatTime )
{
    (void)DelatTime;   
}

//========================================================================

void dlg_popup::AddPassKey( s32 key )
{
    if( m_PasswordIndex == 3 )
        return;

    m_PasswordIndex++;
    m_Password[m_PasswordIndex] = key;
}

//========================================================================

xbool dlg_popup::RemovePassKey( void )
{
    if( m_PasswordIndex > -1 )
    {
        m_Password[0] = 0;
        m_Password[1] = 0;
        m_Password[2] = 0;
        m_Password[3] = 0;
        m_PasswordIndex = -1;
    }
    else
    {
        return TRUE;
    }
    return FALSE;
}

//========================================================================

void dlg_popup::SetPassword( u8* password )
{
    m_CheckPassword[0] = password[0];
    m_CheckPassword[1] = password[1];
    m_CheckPassword[2] = password[2];
    m_CheckPassword[3] = password[3];
}

//========================================================================