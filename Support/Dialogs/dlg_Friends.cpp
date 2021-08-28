//=========================================================================
//
//  dlg_friends.cpp
//
//=========================================================================
 
#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_blankbox.hpp"
#include "ui\ui_friendlist.hpp" 

#include "dlg_Friends.hpp"
#include "dlg_SubMenu.hpp"  
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp" 
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\Voice\VoiceMgr.hpp"

//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum 
{
    FRIEND_POPUP_REMOVE,
    FRIEND_POPUP_JOIN,
    FRIEND_POPUP_ACCEPT_INVITE,
};

enum controls
{
	IDC_FRIEND_LIST,
    IDC_FRIEND_DETAILS,
    IDC_FRIEND_NAME_TEXT,
    IDC_FRIEND_GAME_TEXT,
    IDC_FRIEND_STATUS_TEXT,
    IDC_FRIEND_NAV_TEXT,
};

enum local_msg_type
{
   LOCAL_MSG_JOIN_FRIEND,
   LOCAL_MSG_SEND_GAME_INVITE,
   LOCAL_MSG_CANCEL_GAME_INVITE,
   LOCAL_MSG_ACCEPT_GAME_INVITE,
   LOCAL_MSG_DECLINE_GAME_INVITE,
   LOCAL_MSG_ACCEPT_FRIEND_REQ,
   LOCAL_MSG_DECLINE_FRIEND_REQ,
   LOCAL_MSG_BLOCK_FRIEND_REQ,
   LOCAL_MSG_CANCEL_FRIEND_REQ,
   LOCAL_MSG_REMOVE_FRIEND,
   LOCAL_MSG_VOICE_ATTACHMENT,
   LOCAL_MSG_FEEDBACK,
   LOCAL_MSG_NONE,
};

ui_manager::control_tem FriendsControls[] = 
{
    // Frames.
    { IDC_FRIEND_LIST,          "IDS_NULL",         "friendlist",  40,  40, 416, 222, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_FRIEND_DETAILS,       "IDS_NULL",         "blankbox",    40, 272, 416,  60, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_FRIEND_GAME_TEXT,     "IDS_NULL",         "text",        48, 294, 230,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_FRIEND_STATUS_TEXT,   "IDS_NULL",         "text",        48, 310, 230,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_FRIEND_NAV_TEXT,      "IDS_NULL",         "text",         0,   0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem FriendsDialog =
{
    "IDS_FRIENDS_LIST",
    1, 9,
    sizeof(FriendsControls)/sizeof(ui_manager::control_tem),
    &FriendsControls[0],
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

void dlg_friends_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "friends list", &FriendsDialog, &dlg_friends_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_friends_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_friends* pDialog = new dlg_friends;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_friends
//=========================================================================

dlg_friends::dlg_friends( void )
{
}

//=========================================================================

dlg_friends::~dlg_friends( void ) 
{
    Destroy();
}

//=========================================================================

xbool dlg_friends::Create( s32                        UserID,
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
    
    m_pFriendList = (ui_friendlist*)FindChildByID( IDC_FRIEND_LIST );
    m_pFriendList->SetLineHeight( 32 );
    m_pFriendList->SetFlag(ui_win::WF_SELECTED, TRUE);
    m_pFriendList->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pFriendList->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pFriendList->EnableHeaderBar();
    m_pFriendList->SetHeaderBarColor( xcolor(19,59,14,196) );
    m_pFriendList->SetHeaderColor( xcolor(255,252,204,255) );
    m_pFriendList->SetExitOnSelect(FALSE);
    m_pFriendList->SetExitOnBack(TRUE);
    m_pFriendList->DisableFrame();
    GotoControl( (ui_control*)m_pFriendList );
    m_CurrHL = 0;

    // Display friends list instead of players list - only shows voice allowed/banned status
    m_pFriendList->Configure( TRUE );

    // set up nav text
    m_pNavText = (ui_text*) FindChildByID( IDC_FRIEND_NAV_TEXT );
    
    m_pNavText->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);    

    // get server details box
    m_pFriendDetails = (ui_blankbox*)FindChildByID( IDC_FRIEND_DETAILS );
    m_pFriendDetails->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pFriendDetails->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pFriendDetails->SetHasTitleBar( TRUE );
    m_pFriendDetails->SetLabelColor( xcolor(255,252,204,255) );
    m_pFriendDetails->SetTitleBarColor( xcolor(19,59,14,196) );

    // set up server info text
    m_pFriendGame   = (ui_text*)FindChildByID( IDC_FRIEND_GAME_TEXT   );
    m_pFriendStatus = (ui_text*)FindChildByID( IDC_FRIEND_STATUS_TEXT );
    
    m_pFriendGame   ->UseSmallText( TRUE );
    m_pFriendStatus ->UseSmallText( TRUE );

    m_pFriendGame   ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pFriendStatus ->SetLabelFlags( ui_font::h_left|ui_font::v_center );

    m_pFriendGame   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pFriendStatus ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    m_pFriendGame   ->SetLabelColor( xcolor(255,252,204,255) );
    m_pFriendStatus ->SetLabelColor( xcolor(255,252,204,255) );

    // populate friend info
    //PopulateFriendInfo( friend info1 );

    // fill the friends list
    FillFriendsList();

    // testing - put some crap in the list
    //m_pFriendList->AddItem( xwstring("TechnicGravy"  ));
    //m_pFriendList->AddItem( xwstring("Lord Data"     ));
    //m_pFriendList->AddItem( xwstring("Wild Coyote"   ));
    //m_pFriendList->AddItem( xwstring("xxHavokxx"     ));
    //m_pFriendList->AddItem( xwstring("RainyDayGamer" ));
    //m_pFriendList->AddItem( xwstring("Sleepy Floyd"  ));
    //m_pFriendList->AddItem( xwstring("ManitiXX"      ));
    //m_pFriendList->AddItem( xwstring("PsychoMadman"  ));

    // initialize submenu
    m_SubMenu = NULL;
    m_PopUp   = NULL;
    m_SyncPopup = NULL;

    // initialize screen scaling
    InitScreenScaling( Position );

    // disable background filter
    m_bRenderBlackout = FALSE;

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // Record access popup
    m_bAskAddAttachment = FALSE;
    m_AttachPopUp = NULL;

    m_bAskPlayAttachment = FALSE;
    m_PlayAttachPopUp = NULL;

    // Default Popup
    m_DefaultPopUp = NULL;

	// Return success code
    return Success;
}

//=========================================================================

void dlg_friends::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_friends::Render( s32 ox, s32 oy )
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

#ifdef TARGET_PS2
    // render gamespy logo
    irect LogoPos = m_Position;
    LogoPos.r -= 35;
    LogoPos.l = LogoPos.r - 128;
    LogoPos.b = LogoPos.t + 32;
    g_UiMgr->RenderBitmap( g_UiMgr->FindBitmap( "gamespy_logo" ), LogoPos, XCOLOR_WHITE );
#endif

    // render the glow bar
    g_UiMgr->RenderGlowBar();
}

//=========================================================================

void dlg_friends::OnNotify ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData )
{
    (void)pWin;
    (void)pSender;
    (void)Command;
    (void)pData;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if (Command == WN_LIST_ACCEPTED)
        {
        }
    } 
}

//=========================================================================

void dlg_friends::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
}

//=========================================================================

void dlg_friends::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( m_pFriendList->GetSelection() == -1 )
        {
            return;
        }

        g_StateMgr.SetControllerRequested( g_UiMgr->GetActiveController(), TRUE );

        OpenSubMenu();
    }
}

//=========================================================================

void dlg_friends::OpenSubMenu( void )
{
    // display confirmation popup
    irect r (0, 0, 300, 200 );       
    m_SubMenu = (dlg_submenu*)g_UiMgr->OpenDialog(  m_UserID, "submenu", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER );

    buddy_info* pBuddy;

    pBuddy = (buddy_info*)m_pFriendList->GetSelectedItemData(0);
    ASSERT( pBuddy );

    SetupLocalMessage( pBuddy );

    m_Buddy = *pBuddy;

    // Setup the button strings
    xwstring Buttons[5];
    for( s32 i=0; i < 5; i++ )
    {
        if( m_LocalMessage[i] != -1 )
            Buttons[i] = g_StringTableMgr( "ui", GetMessageString( m_LocalMessage[i] ) );
    }

    // configure using correct strings
    m_SubMenu->Configure( 
        g_StringTableMgr( "ui", "IDS_FRIENDS_LIST" ),   // title
        Buttons[0],                                     // button one label
        Buttons[1],                                     // button two label
        Buttons[2],                                     // button three label
        Buttons[3],                                     // button four label
        Buttons[4],                                     // button five label
        g_StringTableMgr( "ui", "IDS_NULL" ),           // optional message
        &m_SubMenuResult );    
}

//=========================================================================

void dlg_friends::SetupLocalMessage( const buddy_info* pBuddy )
{
    m_LocalMessage[0] = -1;
    m_LocalMessage[1] = -1;
    m_LocalMessage[2] = -1;
    m_LocalMessage[3] = -1;
    m_LocalMessage[4] = -1;
    s32 AttachmentMsg = -1;
    s32 FeedbackMsg   = -1;

    #ifdef TARGET_XBOX
    {
        // Check if there is a voice attachment and the player is not voice banned
        if( (pBuddy->Flags & USER_VOICE_MESSAGE) && !g_VoiceMgr.IsVoiceBanned() )
        {
            AttachmentMsg = LOCAL_MSG_VOICE_ATTACHMENT;
            FeedbackMsg   = LOCAL_MSG_FEEDBACK;
        }
    }
    #endif

    if( pBuddy->Flags & USER_REQUEST_SENT )
    {
        m_LocalMessage[0] = LOCAL_MSG_CANCEL_FRIEND_REQ;
        return;
    }

    if( pBuddy->Flags & USER_REQUEST_RECEIVED )
    {
        m_LocalMessage[0] = LOCAL_MSG_ACCEPT_FRIEND_REQ;
        m_LocalMessage[1] = LOCAL_MSG_DECLINE_FRIEND_REQ;
        m_LocalMessage[2] = LOCAL_MSG_BLOCK_FRIEND_REQ;
        m_LocalMessage[3] = AttachmentMsg;
        m_LocalMessage[4] = FeedbackMsg;
        return;
    }

    if( pBuddy->Flags & USER_HAS_INVITE )
    {
        if( IsInSameGame( pBuddy ) == FALSE )
        {
            m_LocalMessage[0] = LOCAL_MSG_ACCEPT_GAME_INVITE;
            m_LocalMessage[1] = LOCAL_MSG_DECLINE_GAME_INVITE;
            m_LocalMessage[2] = LOCAL_MSG_REMOVE_FRIEND;
            m_LocalMessage[3] = AttachmentMsg;
            m_LocalMessage[4] = FeedbackMsg;
            return;
        }
    }

    // To be "invitable" a friend MUST NOT be playing in our game.
    // To be "joinable" a friend MUST be playing in a different game to ours.
    xbool IsFriendInvitable = !IsInSameGame( pBuddy );
    xbool IsFriendJoinable  =  IsFriendInvitable &&
                               ((pBuddy->Status == BUDDY_STATUS_INGAME) || (pBuddy->Flags & USER_IS_JOINABLE));

    if( g_NetworkMgr.IsOnline() == TRUE )
    {
        if( GameMgr.GameInProgress() == TRUE )
        {
            s32 Num = 0;

            if( IsFriendInvitable == TRUE )
            {
                // Determine whether the invite was answered.
                xbool IsInviteAnswered = (pBuddy->Flags & USER_REJECTED_INVITE) ||
                                         (pBuddy->Flags & USER_ACCEPTED_INVITE);

                // The invite message depends on whether we have sent a game invite or not.
                // If the game invite was sent but not yet answered, then we can cancel the invite.
                s32 InviteMessage =  (pBuddy->Flags & USER_SENT_INVITE) && !IsInviteAnswered    ?
                                     LOCAL_MSG_CANCEL_GAME_INVITE                               :
                                     LOCAL_MSG_SEND_GAME_INVITE;

                m_LocalMessage[ Num++ ] = InviteMessage;
            }

            if( IsFriendJoinable == TRUE )
            {
                m_LocalMessage[ Num++ ] = LOCAL_MSG_JOIN_FRIEND;
            }

            m_LocalMessage[ Num++ ] = LOCAL_MSG_REMOVE_FRIEND;
        }
        else
        {
            if( IsFriendJoinable == TRUE )
            {
                m_LocalMessage[0] = LOCAL_MSG_JOIN_FRIEND;
                m_LocalMessage[1] = LOCAL_MSG_REMOVE_FRIEND;
            }
            else
            {
                m_LocalMessage[0] = LOCAL_MSG_REMOVE_FRIEND;
            }
        }
    }
    else
    {
        if( IsFriendJoinable == TRUE )
        {
            m_LocalMessage[0] = LOCAL_MSG_JOIN_FRIEND;
            m_LocalMessage[1] = LOCAL_MSG_REMOVE_FRIEND;
        }
        else
        {
            m_LocalMessage[0] = LOCAL_MSG_REMOVE_FRIEND;
        }
    }
}

//=========================================================================

xbool dlg_friends::IsInSameGame( const buddy_info* pBuddy ) const
{
    xbool IsInSameGame = FALSE;

    if( GameMgr.GameInProgress() == TRUE )
    {
        const game_score& Score = GameMgr.GetScore();

        for( s32 i=0; i < NET_MAX_PLAYERS; i++ )
        {
            if( (Score.Player[i].Identifier  == pBuddy->Identifier) &&
                (Score.Player[i].IsConnected == TRUE) )
            {
                IsInSameGame = TRUE;
                break;
            }
        }

        #ifdef TARGET_XBOX
        // If a friend drops from the game, the code above will detect that fact instantly.
        // However, due to latency in the Xbox Live service, it may be a few seconds before
        // the friend status is updated.  For that reason we will also take into account
        // what the Xbox Live service thinks is the friend's session.  This has the effect
        // of returning TRUE from this function until the Live service has the correct status.
        if( GameMgr.IsGameOnline() == TRUE )
        {
            if( x_memcmp( &g_ActiveConfig.GetConfig().SessionID,
                          &pBuddy->SessionID,
                          sizeof( pBuddy->SessionID ) ) == 0 )
            {
                IsInSameGame = TRUE;
            }
        }
        #endif
    }

    return( IsInSameGame );
}

//=========================================================================

const char* dlg_friends::GetMessageString( s32 MessageID ) const
{
    switch( MessageID )
    {
        case LOCAL_MSG_JOIN_FRIEND          : return( "IDS_JOIN_FRIEND"             );
        case LOCAL_MSG_SEND_GAME_INVITE     : return( "IDS_SEND_GAME_INVITE"        );
        case LOCAL_MSG_CANCEL_GAME_INVITE   : return( "IDS_CANCEL_GAME_INVITE"      );
        case LOCAL_MSG_ACCEPT_GAME_INVITE   : return( "IDS_ACCEPT_GAME_INVITE"      );
        case LOCAL_MSG_DECLINE_GAME_INVITE  : return( "IDS_DECLINE_GAME_INVITE"     );
        case LOCAL_MSG_ACCEPT_FRIEND_REQ    : return( "IDS_ACCEPT_FRIEND_REQUEST"   );
        case LOCAL_MSG_DECLINE_FRIEND_REQ   : return( "IDS_DECLINE_FRIEND_REQUEST"  );
        case LOCAL_MSG_BLOCK_FRIEND_REQ     : return( "IDS_BLOCK_FRIEND_REQUEST"    );
        case LOCAL_MSG_CANCEL_FRIEND_REQ    : return( "IDS_CANCEL_FRIEND_REQUEST"   );
        case LOCAL_MSG_REMOVE_FRIEND        : return( "IDS_REMOVE_FRIEND"           );
        case LOCAL_MSG_VOICE_ATTACHMENT     : return( "IDS_LISTEN_TO_ATTACHMENT"    );
        case LOCAL_MSG_FEEDBACK             : return( "IDS_SEND_FEEDBACK"           );
        default                             : ASSERT( FALSE );
    }

    return( NULL );
}

//=========================================================================

void dlg_friends::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if( m_SubMenu )
        return;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        g_AudioMgr.Play("Backup");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_friends::JoinFriend( void )
{
    // attempt to join friends game. We fill out a dummy config structure to prevent the
    // commit validation function failing. We do not have any of the level info, # players
    // and so on until the login succeeds. The matchmgr will start to negotiate details of
    // this machines connection.
    xbool PlayingSameGame = g_MatchMgr.JoinBuddy( m_Buddy );

    // Determine if the buddy is joinable
    xbool IsJoinable  = ((m_Buddy.Status  == BUDDY_STATUS_INGAME) ||
        (m_Buddy.Flags   &  USER_IS_JOINABLE   ));

    // Determine if the buddy is playing the same title.
    // This can only get set to FALSE on XBox.
    xbool IsSameTitle = TRUE;

#ifdef TARGET_XBOX
    IsSameTitle = g_MatchMgr.IsFriendPlayingSameTitleID( m_Buddy.TitleID );
#endif

    if( (IsJoinable == TRUE) && (PlayingSameGame == FALSE) )
    {
        if( IsSameTitle == TRUE )
        {
            if( GameMgr.GameInProgress() )
            {
                g_ActiveConfig.SetExitReason( GAME_EXIT_FOLLOW_BUDDY );
            }
            else
            {
                m_State = DIALOG_STATE_SELECT;
            }
        }
        else
        {
            // NOTE: For now I am using the invite method to handle the Cross Title Joining.
            // On XBox this will write a file to the hard drive that is read at game startup.
            // XOnlineGameJoin looks like it accomplishes the same thing however. In any case, at
            // startup, cross title invites/joins are detected by OnlineGameInviteGetLatestAccepted (JP)
            g_MatchMgr.AnswerBuddyInvite( m_Buddy, BUDDY_ANSWER_YES );
            SetupInsertDiscPopup( FALSE );
        }
    }
}

//=========================================================================

void dlg_friends::AcceptInvite( void )
{
    xbool IsJoinable = (m_Buddy.Status == BUDDY_STATUS_INGAME) ||
        (m_Buddy.Flags   & USER_IS_JOINABLE   );

    if( IsJoinable == TRUE )
    {
        // Only XBox can set this to FALSE
        xbool IsSameTitle = TRUE;

#ifdef TARGET_XBOX
        IsSameTitle = g_MatchMgr.IsFriendPlayingSameTitleID( m_Buddy.TitleID );
#endif

        if( IsSameTitle == TRUE )
        {
            // NOTE: There is no need to check if we are playing the same game
            // because the context sensitive menu will not even give us the
            // option to accept an invite if that is the case. (JP)
            g_MatchMgr.AnswerBuddyInvite( m_Buddy, BUDDY_ANSWER_YES );

            g_PendingConfig.GetConfig().ConnectionType = CONNECT_INDIRECT;
            g_PendingConfig.GetConfig().Remote         = m_Buddy.Remote;

            if( GameMgr.GameInProgress() )
            {
                g_ActiveConfig.SetExitReason( GAME_EXIT_FOLLOW_BUDDY );
            }
            else
            {
                m_State = DIALOG_STATE_SELECT;
            }
        }
        else
        {
            // Handle cross title invite, buddy online but not playing this game
            g_MatchMgr.AnswerBuddyInvite( m_Buddy, BUDDY_ANSWER_YES );
            SetupInsertDiscPopup( TRUE );
        }
    }
    else
    {
        // Player invited me but now the session is over.
        irect r = g_UiMgr->GetUserBounds( g_UiUserID );
        m_DefaultPopUp = (dlg_popup*)g_UiMgr->OpenDialog( g_UiUserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
        xwstring InviteGameSessionOverMsgText(g_StringTableMgr( "ui", "IDS_ONLINE_SESSION_ENDED" ));                        
        m_DefaultPopUp->Configure( g_StringTableMgr( "ui", "IDS_ACCEPT" ), 
            FALSE, 
            TRUE, 
            FALSE, 
            InviteGameSessionOverMsgText,
            g_StringTableMgr( "ui", "IDS_NAV_CANCEL" ),
            &m_DefaultPopUpResult );

        g_MatchMgr.AnswerBuddyInvite( m_Buddy, BUDDY_ANSWER_NO );         // decline game invite from friend
    }
}

//=========================================================================

void dlg_friends::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    FillFriendsList();
    PopulateFriendInfo();

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // turn on the controls
            m_pFriendList       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pFriendDetails    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            
            GotoControl( (ui_control*)m_pFriendList );
            g_UiMgr->SetScreenHighlight( m_pFriendList->GetPosition() );

            // activate text
            m_pFriendGame       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pFriendStatus     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText          ->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
    }

    xbool UpdateSubMenu = TRUE;

    if( m_PopUp )
    {
        // Handle confirmation popup
        if( m_PopUpResult != DLG_POPUP_IDLE )
        {
            if( m_PopUpType == FRIEND_POPUP_REMOVE )
            {
                if( m_PopUpResult == DLG_POPUP_YES )
                {
                    s32 MessageID = m_LocalMessage[ m_SubMenuResult - DLG_SUBMESSAGE_ONE ];

                    if( MessageID == LOCAL_MSG_REMOVE_FRIEND )
                    {
                        // Remove friend from the friends list
                        g_MatchMgr.DeleteBuddy( m_Buddy );
                    }
                    else
                    {
                        // Block all future friend requests from this player
                        g_MatchMgr.AnswerBuddyRequest( m_Buddy, BUDDY_ANSWER_BLOCK );
                    }
                }
            }
            else if( m_PopUpType == FRIEND_POPUP_ACCEPT_INVITE )
            {
                // accept game invite?
                if( m_PopUpResult == DLG_POPUP_YES )
                {
                    AcceptInvite();
                }
            }
            else
            {
                // join friend in game?
                if( m_PopUpResult == DLG_POPUP_YES )
                {
                    JoinFriend();
                }
            }

            // Remove the popup
            m_PopUp = NULL;
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }

        // Popup is active so skip over the sub menu
        UpdateSubMenu = FALSE;
    }

    if( m_SubMenu && (UpdateSubMenu == TRUE) )
    {
        if( m_SubMenuResult != DLG_SUBMESSAGE_IDLE )
        {
            if( m_SubMenuResult != DLG_SUBMESSAGE_BACK )
            {
                //klkl: DLG_SUBMESSAGE_BACK causes index out of bounds
                s32 MessageID = m_LocalMessage[ m_SubMenuResult - DLG_SUBMESSAGE_ONE ];

                // process result based on message type
                switch( MessageID )
                {
                    case LOCAL_MSG_JOIN_FRIEND:
                    {
                        if( g_StateMgr.IsPaused() )
                        {
                            irect r = g_UiMgr->GetUserBounds( g_UiUserID );
                            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
                            m_PopUpType = FRIEND_POPUP_JOIN;

                            // set nav text
                            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
                            navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
                            m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

                            if( g_NetworkMgr.IsOnline() )
                            {                       
                                if( g_NetworkMgr.IsServer() == TRUE )
                                {
                                    // configure message
                                    m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_JOIN_FRIEND" ), 
                                        TRUE, 
                                        TRUE, 
                                        FALSE, 
                                        g_StringTableMgr( "ui", "IDS_QUIT_ONLINE_MSG" ),
                                        navText,
                                        &m_PopUpResult );
                                }
                                else
                                {
                                    m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_JOIN_FRIEND" ), 
                                        TRUE, 
                                        TRUE, 
                                        FALSE, 
                                        g_StringTableMgr( "ui", "IDS_QUIT_MSG" ),
                                        navText,
                                        &m_PopUpResult );
                                }
                            }
                            else
                            {
                                // configure message
                                m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_QUIT_POPUP" ), 
                                    TRUE, 
                                    TRUE, 
                                    FALSE, 
                                    g_StringTableMgr( "ui", "IDS_QUIT_DESTRUCTIVE_MSG" ),
                                    navText,
                                    &m_PopUpResult );
                            }
                        }
                        else
                        {
                            JoinFriend();
                        }
                    }
                    break;

                    case LOCAL_MSG_SEND_GAME_INVITE:
                    {
#ifdef TARGET_XBOX
                        // Player is given the option to record a voice message only
                        // if he is not voice banned and has a headset attached.
                        if( (g_VoiceMgr.IsVoiceBanned()    == FALSE) &&
                            (g_VoiceMgr.IsHeadsetPresent() == TRUE ) )
                        {                   
                            // confirm to 
                            irect r = g_UiMgr->GetUserBounds( g_UiUserID );
                            m_AttachPopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

                            // set nav text
                            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
                            navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
                            m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

                            // configure message
                            m_AttachPopUp->Configure( g_StringTableMgr( "ui", "IDS_ADD_ATTACHMENT" ), 
                                TRUE, 
                                TRUE, 
                                FALSE, 
                                g_StringTableMgr( "ui", "IDS_ADD_ATTACHMENT_MSG" ),
                                navText,
                                &m_AttachPopUpResult );

                            m_bAskAddAttachment = TRUE;
                        }
                        else
                        {
                            g_MatchMgr.InviteBuddy( m_Buddy );
                            ActivateSyncPopup(SYNC_MODE_INVITE_BUDDY);
                        }
#else
                        g_MatchMgr.InviteBuddy( m_Buddy );            // send game invite to friend
#endif
                    }
                    break;

                    case LOCAL_MSG_CANCEL_GAME_INVITE:
                    {
                        g_MatchMgr.CancelBuddyInvite( m_Buddy );       // cancel game invite sent to friend
                    }
                    break;

                    case LOCAL_MSG_ACCEPT_GAME_INVITE:
                    {
                        if( g_StateMgr.IsPaused() )
                        {
                            irect r = g_UiMgr->GetUserBounds( g_UiUserID );
                            m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
                            m_PopUpType = FRIEND_POPUP_ACCEPT_INVITE;

                            // set nav text
                            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
                            navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
                            m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

                            if( g_NetworkMgr.IsOnline() )
                            {                       
                                if( g_NetworkMgr.IsServer() == TRUE )
                                {
                                    // configure message
                                    m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_ACCEPT_GAME_INVITE" ), 
                                        TRUE, 
                                        TRUE, 
                                        FALSE, 
                                        g_StringTableMgr( "ui", "IDS_QUIT_ONLINE_MSG" ),
                                        navText,
                                        &m_PopUpResult );
                                }
                                else
                                {
                                    m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_ACCEPT_GAME_INVITE" ), 
                                        TRUE, 
                                        TRUE, 
                                        FALSE, 
                                        g_StringTableMgr( "ui", "IDS_QUIT_MSG" ),
                                        navText,
                                        &m_PopUpResult );
                                }
                            }
                            else
                            {
                                // configure message
                                m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_ACCEPT_GAME_INVITE" ), 
                                    TRUE, 
                                    TRUE, 
                                    FALSE, 
                                    g_StringTableMgr( "ui", "IDS_QUIT_DESTRUCTIVE_MSG" ),
                                    navText,
                                    &m_PopUpResult );
                            }
                        }
                        else
                        {
                            AcceptInvite();
                        }
                    }
                    break;

                    case LOCAL_MSG_DECLINE_GAME_INVITE:
                    {
                        g_MatchMgr.AnswerBuddyInvite( m_Buddy, BUDDY_ANSWER_NO );         // decline game invite from friend
                    }
                    break;

                    case LOCAL_MSG_ACCEPT_FRIEND_REQ:
                    {
                        g_MatchMgr.AnswerBuddyRequest( m_Buddy, BUDDY_ANSWER_YES );       // accept friend request
                    }
                    break;

                    case LOCAL_MSG_DECLINE_FRIEND_REQ:
                    {
                        g_MatchMgr.AnswerBuddyRequest( m_Buddy, BUDDY_ANSWER_NO );        // decline friend request 
                    }
                    break;

                    case LOCAL_MSG_BLOCK_FRIEND_REQ:
                    {
                        irect r = g_UiMgr->GetUserBounds( g_UiUserID );
                        m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
                        m_PopUpType = FRIEND_POPUP_REMOVE;

                        // set nav text
                        xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
                        navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
                        m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

                        // configure message
                        m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_BLOCK_REQUEST_TITLE" ), 
                            TRUE, 
                            TRUE, 
                            FALSE, 
                            g_StringTableMgr( "ui", "IDS_BLOCK_REQUEST_MSG" ),
                            navText,
                            &m_PopUpResult );
                    }
                    break;

                    case LOCAL_MSG_CANCEL_FRIEND_REQ:
                    {
                        g_MatchMgr.DeleteBuddy( m_Buddy );                                // cancel friend request sent
                    }
                    break;

                    case LOCAL_MSG_REMOVE_FRIEND:
                    {
                        irect r = g_UiMgr->GetUserBounds( g_UiUserID );
                        m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );
                        m_PopUpType = FRIEND_POPUP_REMOVE;

                        // set nav text
                        xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
                        navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
                        m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

                        // configure message
                        m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_REMOVE_FRIEND_TITLE" ), 
                            TRUE, 
                            TRUE, 
                            FALSE, 
                            g_StringTableMgr( "ui", "IDS_REMOVE_FRIEND_MSG" ),
                            navText,
                            &m_PopUpResult );
                    }
                    break;
#ifdef TARGET_XBOX
                    case LOCAL_MSG_VOICE_ATTACHMENT:
                    {
                        // Setup to record a voice message.
                        irect r;
                        r.Set(0,0,400,200);
                        m_PlayAttachPopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

                        xwstring navText( g_StringTableMgr( "ui", "IDS_PLAY_ATTACHMENT_FROM" ) );
                        navText += " ";
                        navText += m_Buddy.Name;
                        m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

                        // configure message
                        m_PlayAttachPopUp->ConfigurePlayDialog( r,
                            g_StringTableMgr( "ui", "IDS_PLAY_ATTACHMENT" ), 
                            g_StringTableMgr( "ui", "IDS_PLAY_ATTACHMENT_MSG" ),
                            navText,
                            &m_PlayAttachPopUpResult );

                        m_PlayAttachPopUp->SetBuddy( m_Buddy );
                        m_bAskPlayAttachment = FALSE;
                    }
                    break;

                    case LOCAL_MSG_FEEDBACK:
                    {
                        g_StateMgr.SetFeedbackPlayer( m_Buddy.Name, m_Buddy.Identifier );

                        // go to feedback screen
                        m_State = DIALOG_STATE_ACTIVATE;
                        return;
                    }
                    break;
#endif
                }
            }

            // clear popup 
            m_SubMenu = NULL;
        }
    }
 
#ifdef TARGET_XBOX
    // Record a Attachment ASK / RECORD popups handled here.
    if ( m_AttachPopUp )
    {
        if ( m_AttachPopUpResult != DLG_POPUP_IDLE )
        {
            if( m_bAskAddAttachment )
            {
                // ? Send Voice attachment.
                if ( m_AttachPopUpResult == DLG_POPUP_YES )
                {
                    // Setup to record a voice message.
                    irect r;
                    r.Set(0,0,460,200);
                    m_AttachPopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

                    // configure message
                    m_AttachPopUp->ConfigureRecordDialog( r,
                                                          g_StringTableMgr( "ui", "IDS_RECORD_ATTACHMENT" ),
                                                          g_StringTableMgr( "ui", "IDS_RECORD_ATTACHMENT_MSG" ),
                                                          &m_AttachPopUpResult );

                    m_AttachPopUp->SetBuddy    ( m_Buddy );
                    m_AttachPopUp->SetBuddyMode( dlg_popup::BUDDY_MODE_INVITE );

                    m_bAskAddAttachment = FALSE; 
                }
                else
                {
                    g_MatchMgr.InviteBuddy( m_Buddy );            // send game invite to friend
                    ActivateSyncPopup(SYNC_MODE_INVITE_BUDDY);
                    m_AttachPopUp = NULL;
                    m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
                }                
            }
            else // ? record go well?
            {
                if ( m_AttachPopUpResult == DLG_POPUP_OTHER )
                {
                    ActivateSyncPopup(SYNC_MODE_INVITE_BUDDY);
                    m_AttachPopUp = NULL;
                }
                else
                {
                    m_AttachPopUp = NULL;
                }
            }
        }        
    }


    // Play Attachment ASK / PLAY popups handled here.
    if( m_PlayAttachPopUp )

    {
        if ( m_PlayAttachPopUpResult != DLG_POPUP_IDLE )
        {
            if ( m_PlayAttachPopUpResult == DLG_POPUP_OTHER )
            {
                // result ok.. return to the submenu.
                OpenSubMenu();
            }
            else
            {
                // something went wrong and DLG_POPUP_NO was sent back.. this 
                // is normal when the invite or other was canceled and thus
                // the attachment is now invalid. So we should just return to
                // the players list.
            }

            m_PlayAttachPopUp = NULL;
        }
    }
#endif

    if( m_DefaultPopUp )
    {
        if( m_DefaultPopUpResult != DLG_POPUP_IDLE )
        {
            m_DefaultPopUp = NULL;
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

#ifdef TARGET_XBOX
    UpdateSyncPopup(DeltaTime);
#endif

    // update everything else
    ui_dialog::OnUpdate( pWin, DeltaTime );
}

//=========================================================================

void dlg_friends::SetupInsertDiscPopup( xbool IsInvite )
{
	//insert new game disk pop up message
    irect r;
    r.Set(0,0,400,200);
	m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog( g_UiUserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER);

    xwstring Format( g_StringTableMgr( "ui", "IDS_MAIN_MENU_INSERT_GAME_DISC_FOR_XBOX" ) );

    s32 Index = Format.Find( xwstring( "%s" ) );
    ASSERT( Index != -1 );

    xwstring Message( Format.Left ( Index ) );
    xwstring Right  ( Format.Right( Format.GetLength() - Index - 2 ) );

    Message += m_Buddy.Location;
    Message += Right;

    const char* pTitle = (IsInvite == TRUE) ? "IDS_ACCEPT" : "IDS_JOIN_FRIEND";

	m_PopUp->Configure( r,
	                    g_StringTableMgr( "ui", pTitle ), 
	                    FALSE, 
	                    TRUE, 
	                    FALSE, 
	                    Message,
	                    g_StringTableMgr( "ui", "IDS_NAV_CANCEL" ),
	                    &m_PopUpResult );
}

//=========================================================================

void dlg_friends::FillFriendsList( void )
{
    s32     i;
    s32     iSel = m_pFriendList->GetSelection();

    // Fill Listbox with levels
    m_pFriendList->DeleteAllItems();

    for( i=0; i<g_MatchMgr.GetBuddyCount(); i++ )
    {
        buddy_info& Buddy = g_MatchMgr.GetBuddy(i);

        if( g_MatchMgr.IsPlayerMuted( Buddy.Identifier ) )
            Buddy.Flags |= FRIENDLIST_IS_MUTED;
        else
            Buddy.Flags &= ~FRIENDLIST_IS_MUTED;

        // Only add this guy if we're not ignoring him.
        if( (Buddy.Flags & USER_REQUEST_IGNORED) == 0 )
        {
            m_pFriendList->AddItem( Buddy.Name, (s32)&Buddy );
        }
    }

    // Limit Selection
    if( iSel <  0 ) iSel = 0;
    if( iSel >= m_pFriendList->GetItemCount() )  iSel = m_pFriendList->GetItemCount()-1;

    xwstring navText;
    

    if( m_pFriendList->GetItemCount() != 0 )
    {
        // Set Selection
        m_pFriendList->SetSelection( iSel );
        navText += g_StringTableMgr( "ui", "IDS_NAV_SELECT" );
    }

    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
    m_pNavText->SetLabel( navText );
}

//=========================================================================

void dlg_friends::PopulateFriendInfo( void )
{
    s32     iSel = m_pFriendList->GetSelection();
    xwchar buff[128];
    xwstring tempLabel;

    x_wstrcpy(buff, g_StringTableMgr( "ui", "IDS_FRIENDLIST_DETAILS" ));
    x_mstrcat(buff, " " );

    if( iSel >=0  ) // friend selected
    {
        buddy_info* pInfo;

        pInfo = (buddy_info*)m_pFriendList->GetSelectedItemData();

        // TODO: fill in text labels with friend info
        tempLabel = g_StringTableMgr( "ui", "IDS_FRIENDLIST_GAME" );
        tempLabel += ' ';

        tempLabel += xwstring( pInfo->Location );
        m_pFriendGame->SetLabel( tempLabel );

        tempLabel = g_StringTableMgr( "ui", "IDS_FRIENDLIST_STATUS" );
        tempLabel += ' ';
        switch( pInfo->Status )
        {
        case BUDDY_STATUS_OFFLINE:      tempLabel += g_StringTableMgr( "ui", "IDS_BUDDY_STATUS_OFFLINE" );    break;
        case BUDDY_STATUS_ONLINE:       tempLabel += g_StringTableMgr( "ui", "IDS_BUDDY_STATUS_ONLINE"  );    break;
        case BUDDY_STATUS_INGAME:       tempLabel += g_StringTableMgr( "ui", "IDS_BUDDY_STATUS_INGAME"  );    break;
        case BUDDY_STATUS_ADD_PENDING:  tempLabel += g_StringTableMgr( "ui", "IDS_BUDDY_STATUS_PENDING" );    break;
        default:                        tempLabel += g_StringTableMgr( "ui", "IDS_BUDDY_STATUS_UNKNOWN" );    break;
        }

        m_pFriendStatus->SetLabel( tempLabel );
        x_mstrcat(buff, pInfo->Name );
    }
    else
    {
        tempLabel = g_StringTableMgr( "ui", "IDS_FRIENDLIST_GAME" );
        tempLabel += xwstring("---");
        m_pFriendGame->SetLabel( tempLabel );

        tempLabel = g_StringTableMgr( "ui", "IDS_FRIENDLIST_STATUS" );
        tempLabel += xwstring("---");
        m_pFriendStatus->SetLabel( tempLabel );
    }

    m_pFriendDetails->SetLabel( buff );
}
//=========================================================================
#ifdef TARGET_XBOX
void dlg_friends::ActivateSyncPopup( s32 SyncMode )
{
    m_SyncMode = SyncMode;
    m_SyncTime = 30.0f;

    // Confirm to send attachment. (XBOX only)
    irect r = g_UiMgr->GetUserBounds( g_UiUserID );
    m_SyncPopup = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

    // message
    xwstring Message;
    
    switch( SyncMode )
    {
        case SYNC_MODE_INVITE_BUDDY:
        Message = g_StringTableMgr("ui","IDS_MESSAGE_INVITE_BUDDY");
        break;
    }

    Message += "\n";
    Message += g_StringTableMgr("ui","IDS_ONLINE_TIMEOUT");
    Message += xwstring( xfs("%d",((s32)m_SyncTime)) );

    // configure message
    m_SyncPopup->Configure( g_StringTableMgr( "ui", "IDS_ONLINE_PLEASE_WAIT" ), 
                            FALSE, 
                            FALSE, 
                            FALSE, 
                            Message,
                            xwstring(""),
                            &m_SyncPopupResult );
}

//=========================================================================

void dlg_friends::CloseSyncPopup( void )
{
    m_SyncPopup->Close();
    m_SyncPopup = NULL;
}

//=========================================================================

void dlg_friends::UpdateSyncPopup( f32 DeltaTime )
{
    if( m_SyncPopup )
    {
        m_SyncTime -= DeltaTime;

        const buddy_info* pBuddy = g_MatchMgr.FindBuddy( m_Buddy.Identifier );             

        if ( m_SyncPopupResult != DLG_POPUP_IDLE || m_SyncTime <= 0.0f )
        {
            CloseSyncPopup();
        }
        else
        {
            xwstring Message;

            switch( m_SyncMode )
            {
                case SYNC_MODE_INVITE_BUDDY:
                    Message = g_StringTableMgr("ui","IDS_MESSAGE_INVITE_BUDDY");
                break;
            }

            Message += "\n";
            Message += g_StringTableMgr("ui","IDS_ONLINE_TIMEOUT");
            Message += xwstring( xfs("%d",((s32)m_SyncTime)) );

            m_SyncPopup->Configure( g_StringTableMgr( "ui", "IDS_ONLINE_PLEASE_WAIT" ), 
                                    FALSE, 
                                    FALSE, 
                                    FALSE, 
                                    Message,
                                    xwstring(""),
                                    &m_SyncPopupResult );


            if( m_SyncMode == SYNC_MODE_INVITE_BUDDY && pBuddy->Flags & USER_SENT_INVITE )             
            {
                // buddy invite sent
                CloseSyncPopup();
            }
            else if ( pBuddy == NULL )
            {
                // Buddy is now missing.. or has stopped being our friend
                CloseSyncPopup();
            }
        }
    }
}
#endif
//=========================================================================