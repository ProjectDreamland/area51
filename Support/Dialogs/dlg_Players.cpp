//=========================================================================
//
//  dlg_players.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_blankbox.hpp"
#include "ui\ui_friendlist.hpp"

#include "dlg_Players.hpp"
#include "dlg_SubMenu.hpp"
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "NetworkMgr\Voice\VoiceMgr.hpp"

//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum controls
{
	IDC_PLAYER_LIST,
    IDC_PLAYER_DETAILS,
    IDC_PLAYER_NAME_TEXT,
    IDC_PLAYER_GAME_TEXT,
    IDC_PLAYER_STATUS_TEXT,
    IDC_PLAYER_NAV_TEXT,
};


ui_manager::control_tem PlayersControls[] = 
{
    // Frames.
    { IDC_PLAYER_LIST,          "IDS_NULL",         "friendlist",  40,  40, 416, 222, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PLAYER_DETAILS,       "IDS_NULL",         "blankbox",    40, 272, 416,  60, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PLAYER_GAME_TEXT,     "IDS_NULL",         "text",        48, 294, 230,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PLAYER_STATUS_TEXT,   "IDS_NULL",         "text",        48, 310, 230,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_PLAYER_NAV_TEXT,      "IDS_NULL",         "text",         0,   0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem PlayersDialog =
{
    "IDS_PLAYERS_LIST",
    1, 9,
    sizeof(PlayersControls)/sizeof(ui_manager::control_tem),
    &PlayersControls[0],
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

void dlg_players_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "players list", &PlayersDialog, &dlg_players_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_players_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_players* pDialog = new dlg_players;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_players
//=========================================================================

dlg_players::dlg_players( void )
{
}

//=========================================================================

dlg_players::~dlg_players( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_players::Create( s32                        UserID,
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

    m_pPlayerList = (ui_friendlist*)FindChildByID( IDC_PLAYER_LIST );
    m_pPlayerList->SetLineHeight( 32 );
    m_pPlayerList->SetFlag(ui_win::WF_SELECTED, TRUE);
    m_pPlayerList->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pPlayerList->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pPlayerList->EnableHeaderBar();
    m_pPlayerList->SetHeaderBarColor( xcolor(19,59,14,196) );
    m_pPlayerList->SetHeaderColor( xcolor(255,252,204,255) );
    m_pPlayerList->SetExitOnSelect(FALSE);
    m_pPlayerList->SetExitOnBack(TRUE);
    m_pPlayerList->DisableFrame();
    GotoControl( (ui_control*)m_pPlayerList );
    m_CurrHL = 0;

    // Display players list instead of friends list (shows additional voice icons)
    m_pPlayerList->Configure( FALSE );

    // set up nav text
    m_pNavText = (ui_text*) FindChildByID( IDC_PLAYER_NAV_TEXT );
    
    m_pNavText->SetLabel( "" );
    m_pNavText->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);    

    // get server details box
    m_pPlayerDetails = (ui_blankbox*)FindChildByID( IDC_PLAYER_DETAILS );
    m_pPlayerDetails->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pPlayerDetails->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pPlayerDetails->SetHasTitleBar( TRUE );
    m_pPlayerDetails->SetLabelColor( xcolor(255,252,204,255) );
    m_pPlayerDetails->SetTitleBarColor( xcolor(19,59,14,196) );

    // set up server info text
    m_pPlayerGame   = (ui_text*)FindChildByID( IDC_PLAYER_GAME_TEXT   );
    m_pPlayerStatus = (ui_text*)FindChildByID( IDC_PLAYER_STATUS_TEXT );
    
    m_pPlayerGame   ->UseSmallText( TRUE );
    m_pPlayerStatus ->UseSmallText( TRUE );

    m_pPlayerGame   ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pPlayerStatus ->SetLabelFlags( ui_font::h_left|ui_font::v_center );

    m_pPlayerGame   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pPlayerStatus ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    m_pPlayerGame   ->SetLabelColor( xcolor(255,252,204,255) );
    m_pPlayerStatus ->SetLabelColor( xcolor(255,252,204,255) );

    m_PlayerMode = PLAYER_MODE_INGAME;

    // fill the players list

    // testing - put some crap in the list
    //m_pPlayerList->AddItem( xwstring("TechnicGravy"  ));
    //m_pPlayerList->AddItem( xwstring("Lord Data"     ));
    //m_pPlayerList->AddItem( xwstring("Wild Coyote"   ));
    //m_pPlayerList->AddItem( xwstring("xxHavokxx"     ));
    //m_pPlayerList->AddItem( xwstring("RainyDayGamer" ));
    //m_pPlayerList->AddItem( xwstring("Sleepy Floyd"  ));
    //m_pPlayerList->AddItem( xwstring("ManitiXX"      ));
    //m_pPlayerList->AddItem( xwstring("PsychoMadman"  ));

    // initialize submenu
    m_SubMenu = NULL;

    // initialize popup
    m_PopUp = NULL;
    m_SyncPopup = NULL;

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
void dlg_players::Configure( player_mode Mode )
{
    m_PlayerMode = Mode;
}

//=========================================================================

void dlg_players::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_players::Render( s32 ox, s32 oy )
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

void dlg_players::OnNotify ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData )
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

void dlg_players::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
}

//=========================================================================

void dlg_players::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    xwstring ButtonOne;
    xwstring ButtonTwo;
    xwstring ButtonThree;

    if( m_State != DIALOG_STATE_ACTIVE )
        return;

    if( m_pPlayerList->GetSelection() == -1 )
    {
        return;
    }

    xbool bPlayerIsRecent = m_pPlayerList->GetItemFlags( m_pPlayerList->GetSelection() ) & ui_friendlist::FLAG_ITEM_RECENT_PLAYER;

    buddy_info* pBuddy = (buddy_info*)m_pPlayerList->GetSelectedItemData( 0 );
    ASSERT( pBuddy != NULL );
    m_Buddy = *pBuddy;

    // display confirmation popup
    irect r (0, 0, 300, 200 );       
    m_SubMenu = (dlg_submenu*)g_UiMgr->OpenDialog(  m_UserID, "submenu", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER );

    // set button labels based on player status
    m_BuddyIndex = m_pPlayerList->GetSelectedItemData(1);
    ButtonOne = g_StringTableMgr( "ui", "IDS_SEND_FEEDBACK" );

    if( (m_PlayerMode == PLAYER_MODE_INGAME) && !bPlayerIsRecent )
    {
        if( g_VoiceMgr.GetLocallyMuted( m_BuddyIndex ) )
        {
            // unmute player option
            ButtonTwo = g_StringTableMgr( "ui", "IDS_UNMUTE_PLAYER" );
        }
        else
        {
            // mute player option
            ButtonTwo = g_StringTableMgr( "ui", "IDS_MUTE_PLAYER" );
        }
    }
    else
    {
        // On XBox we need to be able to mute and unmute players from the frontend
        #ifdef TARGET_XBOX
        {
            if( g_MatchMgr.IsPlayerMuted( m_Buddy.Identifier ) == TRUE )
            {
                // unmute player option
                ButtonTwo = g_StringTableMgr( "ui", "IDS_UNMUTE_PLAYER" );
            }
            else
            {
                // mute player option
                ButtonTwo = g_StringTableMgr( "ui", "IDS_MUTE_PLAYER" );
            }
        }
        #else
        ButtonTwo.Clear();
        #endif
    }

    // Setup the button for sending and cancelling friend requests
    {
        char*       pText  = "IDS_SEND_FRIEND_REQUEST";

        // Check if the selected player is a friend.  Note that this will
        // also find players that have not accepted our friend request.
        if( g_MatchMgr.FindBuddy( m_Buddy.Identifier ) != NULL )
        {
            // Check if we have sent a friend request to this player already
            if( m_Buddy.Flags & USER_REQUEST_SENT )
            {
                pText = "IDS_CANCEL_FRIEND_REQUEST";
            }
            else
            {
                // Player is already a friend so we should disable this button
                pText = NULL;
            }
        }
 
        if( pText != NULL )
            ButtonThree = g_StringTableMgr( "ui", pText );
    }

    // configure using correct strings
    m_SubMenu->Configure( 
        g_StringTableMgr( "ui", "IDS_PLAYERS_LIST" ),   // title
        ButtonOne,                                      // button one label
        ButtonTwo,                                      // button two label
        ButtonThree,                                    // button three label
        g_StringTableMgr( "ui", "IDS_NULL" ),           // optional message
        &m_SubMenuResult );                     
}

//=========================================================================

void dlg_players::OnPadBack( ui_win* pWin )
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

void dlg_players::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime; 

    PopulatePlayerInfo();
    FillPlayersList();

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // turn on the controls
            m_pPlayerList       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pPlayerDetails    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            
            GotoControl( (ui_control*)m_pPlayerList );
            g_UiMgr->SetScreenHighlight( m_pPlayerList->GetPosition() );

            // activate text
            m_pPlayerGame       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pPlayerStatus     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText          ->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    if ( m_SubMenu )
    {
        if ( m_SubMenuResult != DLG_SUBMESSAGE_IDLE )
        {
            // clear popup 
            m_SubMenu = NULL;

            if( m_pPlayerList->GetSelection() == -1 )
            {
                return;
            }

            xbool bPlayerIsRecent = m_pPlayerList->GetItemFlags( m_pPlayerList->GetSelection() ) & ui_friendlist::FLAG_ITEM_RECENT_PLAYER;

            // check result based on sub menu type
            switch( m_SubMenuResult )
            {
                // send feedback
                case DLG_SUBMESSAGE_ONE:
                {
                    // store the player name that we are sending feedback on
                    if( (m_PlayerMode == PLAYER_MODE_INGAME) && !bPlayerIsRecent )
                    {
                        const game_score&   Score           = GameMgr.GetScore();
                        g_StateMgr.SetFeedbackPlayer( Score.Player[m_BuddyIndex].NName, Score.Player[m_BuddyIndex].Identifier );
                    }
                    else
                    {
                        const buddy_info& Buddy = g_MatchMgr.GetRecentPlayer( m_BuddyIndex );
                        g_StateMgr.SetFeedbackPlayer( Buddy.Name, Buddy.Identifier );
                    }

                    // go to feedback screen
                    m_State = DIALOG_STATE_ACTIVATE;
                    return;
                }
                break;

                // mute/unmute player
                case DLG_SUBMESSAGE_TWO:
                {
                    //s32         BuddyIndex = m_pPlayerList->GetSelectedItemData( 1 );
                    //buddy_info* pBuddy     = (buddy_info*)m_pPlayerList->GetSelectedItemData( 0 );
                    //ASSERT( pBuddy != NULL );

                    xbool IsMuted;
                    
                    if( (m_PlayerMode == PLAYER_MODE_INGAME) && !bPlayerIsRecent )
                    {
                        // Toggle the mute state
                        IsMuted = !g_VoiceMgr.GetLocallyMuted( m_BuddyIndex );
                    
                        g_VoiceMgr.SetLocallyMuted( m_BuddyIndex, IsMuted );
                    }
                    else
                    {
                        // On XBox this will retrieve the mute state from the XBox Live service.
                        // Other platforms should never get here since this frontend option is XBox only.
                        IsMuted = !g_MatchMgr.IsPlayerMuted( m_Buddy.Identifier );
                    }

                    // On the XBox this will update the mute state at the XBox Live service
                    g_MatchMgr.SetIsPlayerMuted( m_Buddy.Identifier, IsMuted );
                }
                break;

                // Send or cancel a friend request
                case DLG_SUBMESSAGE_THREE:
                {
                    // Check if the selected player is a buddy already
                    if( g_MatchMgr.FindBuddy( m_Buddy.Identifier ) == NULL )
                    {
                        #ifdef TARGET_XBOX
                        {
                            // If the player is voice banned or does not have a headset
                            // attached they MUST NOT be allowed to create voice attachments.
                            if( !g_VoiceMgr.IsVoiceBanned() && (g_VoiceMgr.IsHeadsetPresent() == TRUE ) )
                            {                            
                                // Confirm to send attachment. (XBOX only)
                                irect r = g_UiMgr->GetUserBounds( g_UiUserID );
                                m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

                                // set nav text
                                xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
                                navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
                                m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

                                // configure message
                                m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_ADD_ATTACHMENT" ), 
                                    TRUE, 
                                    TRUE, 
                                    FALSE, 
                                    g_StringTableMgr( "ui", "IDS_ADD_ATTACHMENT_MSG" ),
                                    navText,
                                    &m_PopUpResult );

                                m_bAskAddAttachment = TRUE;
                            }
                            else
                            {
                                // Player is voice banned so just go ahead and start the friend request
                                g_MatchMgr.AddBuddy( m_Buddy );
                                ActivateSyncPopup(SYNC_MODE_ADD_BUDDY);
                            }
                        }
                        #else
                        {
                            // All non-XBox platforms just start the friend request
                            g_MatchMgr.AddBuddy( m_Buddy );
                        }
                        #endif
                    }
                    else
                    {
                        // Cancel the friend request
#ifdef TARGET_XBOX
                        ActivateSyncPopup(SYNC_MODE_REMOVE_BUDDY);
#endif
                        g_MatchMgr.DeleteBuddy( m_Buddy );
                    }
                }
                break;
            }       
        }
    }

#ifdef TARGET_XBOX
    if( m_PopUp )
    {
        if ( m_PopUpResult != DLG_POPUP_IDLE )
        {
            if( m_bAskAddAttachment )
            {
                // ? Send Voice attachment.
                if ( m_PopUpResult == DLG_POPUP_YES )
                {
                    // Setup to record a voice message.
                    irect r;
                    r.Set(0,0,460,200);
                    m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

                    m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);
 
                    // configure message
                    m_PopUp->ConfigureRecordDialog( r,
                                                    g_StringTableMgr( "ui", "IDS_RECORD_ATTACHMENT" ), 
                                                    g_StringTableMgr( "ui", "IDS_RECORD_ATTACHMENT_MSG" ),
                                                    &m_PopUpResult );
                        
                    m_PopUp->SetBuddyMode( dlg_popup::BUDDY_MODE_ADD );
                    m_PopUp->SetBuddy    ( m_Buddy );

                    m_bAskAddAttachment = FALSE;
                }
                else
                {
                    g_MatchMgr.AddBuddy( m_Buddy );
                    ActivateSyncPopup(SYNC_MODE_ADD_BUDDY);
                    m_PopUp = NULL;
                    m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
                }
            }
            else // ? record go well?
            {
                // Buddy Request is sent.
                m_PopUp = NULL;
                m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
                
                if ( m_PopUpResult == DLG_POPUP_OTHER )
                {
                    // Message was sent to wait for it to be done.
                    // Lets make sure that we have the messages sync'ed after this send attachment popup.
                    ActivateSyncPopup(SYNC_MODE_ADD_BUDDY);
                }
            }
        }
    }

    // Update Sync Popup.. if needed Xbox only
    UpdateSyncPopup(DeltaTime);
#endif


    // update everything else
    ui_dialog::OnUpdate( pWin, DeltaTime );
}

//=========================================================================

void dlg_players::FillPlayersList( void )
{
    s32                 iSel = m_pPlayerList->GetSelection();
    s32                 i;
    const game_score&   Score = GameMgr.GetScore();

    m_pPlayerList->DeleteAllItems();

    s32 NumCurrentPlayers = 0;

    //
    // Add all current ingame players to the players list
    //

    if( (g_NetworkMgr.IsOnline()  == TRUE) &&
        (GameMgr.GameInProgress() == TRUE) )
    {
        s32 MySlot = g_NetworkMgr.GetLocalPlayerSlot( 0 );

        for( i=0 ; i < NET_MAX_PLAYERS ; i++ )
        {
            // Ignore the local player
            if( (Score.Player[i].IsInGame == TRUE) && (i != MySlot) )
            {
                // Get the next free player slot
                buddy_info& Player = m_Players[ NumCurrentPlayers++ ];

                // Check if current player is a friend
                const buddy_info* pBuddy = g_MatchMgr.FindBuddy( Score.Player[i].Identifier );

                if( pBuddy == NULL )
                {
                    // The current player is not a friend yet so we must setup a local buddy_info for him
                    x_memset( &Player, 0, sizeof( Player ) );
                    x_memcpy( Player.Name, Score.Player[i].NName, sizeof( Player.Name ) );
                    Player.Identifier = Score.Player[i].Identifier;
                }
                else
                {
                    // The current player is a friend so copy his info into our local buddy_info
                    Player = *pBuddy;
                }

                m_pPlayerList->AddItem( Player.Name, (s32)&Player, i );

                // IMPORTANT NOTE: On XBox Live, the service only provides voice capable
                // status when a player is also a friend.  To provide the extra voice
                // information required to make the Player's list TCR compliant, we must
                // manually provide the additional voice information.  This information
                // can only be determined in-game and is NOT available from the frontend!
                {
                    Player.Flags &= ~FRIENDLIST_MASK;

                    if( Score.Player[i].IsVoiceAllowed == TRUE )
                        Player.Flags |= FRIENDLIST_IS_VOICE_ALLOWED;

                    if( Score.Player[i].IsVoiceCapable == TRUE )
                        Player.Flags |= FRIENDLIST_IS_VOICE_CAPABLE;

                    if( Score.Player[i].Speaking == TRUE )
                        Player.Flags |= FRIENDLIST_IS_TALKING;

                    if( g_VoiceMgr.GetLocallyMuted( i ) == TRUE )
                        Player.Flags |= FRIENDLIST_IS_MUTED;
                }
            }
        }
    }


    //
    // Add a separator
    //
    m_pPlayerList->AddItem( g_StringTableMgr("ui","IDS_NULL"), 0, 0, FALSE, ui_friendlist::FLAG_ITEM_SEPARATOR );


    //
    // Add the 10 most recent players to the list
    //

    {
        s32 RecentCount = g_MatchMgr.GetRecentPlayerCount();
        s32 NumRecent   = 0;

        for( i=0; i < RecentCount; i++ )
        {
            const buddy_info& Buddy = g_MatchMgr.GetRecentPlayer( i );

            // Check if we already have this recent player in the list
            s32  j;
            for( j = 0; j < NumCurrentPlayers; j++ )
            {
                if( m_Players[ j ].Identifier == Buddy.Identifier )
                    break;
            }

            // Recent player was not already in the list so we can add him
            if( j == NumCurrentPlayers )
            {
                // Get the next free player slot
                ASSERT( (NumCurrentPlayers + NumRecent) < MAX_NUM_PLAYERS );
                buddy_info& Player = m_Players[ NumCurrentPlayers + NumRecent ];

                // Check if the recent player was a friend
                const buddy_info* pBuddy = g_MatchMgr.FindBuddy( Buddy.Identifier );
                if( pBuddy == NULL )
                {
                    // Use the buddy_info from the "recent player" array
                    Player = Buddy;
                }
                else
                {
                    // Use the buddy_info from the friends list which will have up-to-date flags
                    Player = *pBuddy;
                }

                // Clear ALL the extended voice information flags
                Player.Flags &= ~FRIENDLIST_MASK;

                if( pBuddy != NULL )
                {
                    // The recent player is a friend, so assuming he has a headset plugged in,
                    // and is not banned, we can display the headset icon.
                    if( Player.Flags & USER_VOICE_ENABLED )
                    {
                        Player.Flags |= FRIENDLIST_IS_VOICE_ALLOWED |
                                        FRIENDLIST_IS_VOICE_CAPABLE;
                    }
                }

                // The only extended voice information we can determine is
                // whether the player is muted or not.
                if( g_MatchMgr.IsPlayerMuted( Player.Identifier ) == TRUE )
                    Player.Flags |= FRIENDLIST_IS_MUTED;

                m_pPlayerList->AddItem( Player.Name, (s32)&Player, i, TRUE, ui_friendlist::FLAG_ITEM_RECENT_PLAYER );

                NumRecent++;
                if( NumRecent == 10 )
                    break;
            }
        }
    }

    // Limit Selection
    if( iSel <  0 ) iSel = 0;
    if( iSel >= m_pPlayerList->GetItemCount() )  iSel = m_pPlayerList->GetItemCount()-1;

    if( m_pPlayerList->GetItemCount() != 0 )
    {
        // Set Selection
        m_pPlayerList->SetSelection( iSel );
    }
}

//=========================================================================

void dlg_players::PopulatePlayerInfo( void )
{
    xwchar buff[128];
    xwstring tempLabel;
    xwstring HelpText;

    x_wstrcpy(buff, g_StringTableMgr( "ui", "IDS_PLAYERLIST_DETAILS" ));

    buddy_info* pInfo = NULL;

    if( m_pPlayerList->GetItemCount() && (m_pPlayerList->GetSelection() >= 0) )
    {
        pInfo = (buddy_info*)m_pPlayerList->GetSelectedItemData();

        if( pInfo != NULL )
        {
            // this player is selectable
            HelpText = g_StringTableMgr( "ui", "IDS_NAV_SELECT" );

            // Check if the selected player is a friend
            const buddy_info* pBuddy = g_MatchMgr.FindBuddy( pInfo->Identifier );
            if( pBuddy == NULL )
            {
                // Player is not a friend so we won't show any details for him
                pInfo = NULL;
            }
        }
    }

    if( pInfo ) // player selected
    {
        // TODO: fill in text labels with friend info
        tempLabel = g_StringTableMgr( "ui", "IDS_PLAYERLIST_GAME" );
        tempLabel += xwstring( " " );
        tempLabel += xwstring(pInfo->Location);
        m_pPlayerGame->SetLabel( tempLabel );

        tempLabel = g_StringTableMgr( "ui", "IDS_PLAYERLIST_STATUS" );

        tempLabel += xwstring( " " );

        switch( pInfo->Status )
        {
        case BUDDY_STATUS_OFFLINE:      tempLabel += g_StringTableMgr( "ui", "IDS_BUDDY_STATUS_OFFLINE" );    break;
        case BUDDY_STATUS_ONLINE:       tempLabel += g_StringTableMgr( "ui", "IDS_BUDDY_STATUS_ONLINE"  );    break;
        case BUDDY_STATUS_INGAME:       tempLabel += g_StringTableMgr( "ui", "IDS_BUDDY_STATUS_INGAME"  );    break;
        case BUDDY_STATUS_ADD_PENDING:  tempLabel += g_StringTableMgr( "ui", "IDS_BUDDY_STATUS_PENDING" );    break;
        default:                        tempLabel += g_StringTableMgr( "ui", "IDS_BUDDY_STATUS_UNKNOWN" );    break;
        }

        m_pPlayerStatus->SetLabel( tempLabel );
        x_mstrcat( buff, " " );
        x_mstrcat(buff, pInfo->Name );
    }
    else
    {
        tempLabel = g_StringTableMgr( "ui", "IDS_PLAYERLIST_GAME" );
        tempLabel += xwstring(" ---");
        m_pPlayerGame->SetLabel( tempLabel );

        tempLabel = g_StringTableMgr( "ui", "IDS_PLAYERLIST_STATUS" );
        tempLabel += xwstring(" ---");
        m_pPlayerStatus->SetLabel( tempLabel );
    }


    HelpText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );

    m_pNavText->SetLabel( HelpText );

    m_pPlayerDetails->SetLabel( buff );
}
//=========================================================================
#ifdef TARGET_XBOX
void dlg_players::ActivateSyncPopup( s32 SyncMode )
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
        case SYNC_MODE_ADD_BUDDY:
            Message = g_StringTableMgr("ui","IDS_MESSAGE_ADD_BUDDY");
        break;

        case SYNC_MODE_REMOVE_BUDDY:
            Message = g_StringTableMgr("ui","IDS_MESSAGE_REMOVE_BUDDY");
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

void dlg_players::CloseSyncPopup( void )
{
    // Only close if the sync popup is the topmost dialog
    if( g_UiMgr->GetTopmostDialog( m_UserID ) == m_SyncPopup )
    {
        m_SyncPopup->Close();
        m_SyncPopup = NULL;
    }
}

//=========================================================================

void dlg_players::UpdateSyncPopup( f32 DeltaTime )
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
            case SYNC_MODE_ADD_BUDDY:
                Message = g_StringTableMgr("ui","IDS_MESSAGE_ADD_BUDDY");
                break;

            case SYNC_MODE_REMOVE_BUDDY:
                Message = g_StringTableMgr("ui","IDS_MESSAGE_REMOVE_BUDDY");
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


            if( m_SyncMode == SYNC_MODE_ADD_BUDDY && pBuddy != NULL )             
            {
                // buddy is now a friend
                CloseSyncPopup();
            }

            if( m_SyncMode == SYNC_MODE_REMOVE_BUDDY && pBuddy == NULL )
            {
                // We removed the friend invite.
                CloseSyncPopup();
            }
        }
    }
}
#endif
//=========================================================================