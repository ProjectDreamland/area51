//=========================================================================
//
//  dlg_team_change.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_blankbox.hpp"
#include "ui\ui_playerlist.hpp"
#include "ui\ui_text.hpp"

#include "dlg_PopUp.hpp"
#include "dlg_TeamChange.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "NetworkMgr\GameMgr.hpp"


//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum controls
{
    IDC_ALPHA_TEAM_HEADER,
    IDC_ALPHA_TEAM_NAME,
    IDC_ALPHA_TEAM_SCORE,
    IDC_ALPHA_TEAM_LISTBOX,

    IDC_OMEGA_TEAM_HEADER,
    IDC_OMEGA_TEAM_NAME,
    IDC_OMEGA_TEAM_SCORE,
    IDC_OMEGA_TEAM_LISTBOX,

    IDC_TEAM_CHANGE_NAV_TEXT,
};

ui_manager::control_tem TeamChangeControls[] = 
{
    // Frames.
    { IDC_ALPHA_TEAM_HEADER,    "IDS_NULL",     "blankbox",      40,  40, 165,  26,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ALPHA_TEAM_NAME,      "IDS_NULL",     "text",          45,  40, 105,  20,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ALPHA_TEAM_SCORE,     "IDS_NULL",     "text",         170 , 40,  45,  20,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ALPHA_TEAM_LISTBOX,   "IDS_NULL",     "playerlist",    40,  66, 180, 222,  0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_OMEGA_TEAM_HEADER,    "IDS_NULL",     "blankbox",     240,  40, 165,  26,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_OMEGA_TEAM_NAME,      "IDS_NULL",     "text",         245,  40, 105,  20,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_OMEGA_TEAM_SCORE,     "IDS_NULL",     "text",         370,  40,  45,  20,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_OMEGA_TEAM_LISTBOX,   "IDS_NULL",     "playerlist",   240,  66, 180, 222,  1, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_TEAM_CHANGE_NAV_TEXT, "IDS_NULL",     "text",           0,   0,   0,   0,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};

ui_manager::dialog_tem TeamChangeDialog =
{
    "IDS_TEAM_CHANGE",
    2, 9,
    sizeof(TeamChangeControls)/sizeof(ui_manager::control_tem),
    &TeamChangeControls[0],
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
extern const char* HDD_MANIFEST_FILENAME;

//=========================================================================
//  Registration function
//=========================================================================

void dlg_team_change_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "team change", &TeamChangeDialog, &dlg_team_change_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_team_change_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_team_change* pDialog = new dlg_team_change;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_level_select
//=========================================================================

dlg_team_change::dlg_team_change( void )
{
}

//=========================================================================

dlg_team_change::~dlg_team_change( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_team_change::Create( s32                        UserID,
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
    m_pAlphaTeamList    = (ui_playerlist*)  FindChildByID( IDC_ALPHA_TEAM_LISTBOX   );
    m_pAlphaTeamHeader  = (ui_blankbox*)    FindChildByID( IDC_ALPHA_TEAM_HEADER    );
    m_pAlphaTeamName    = (ui_text*)        FindChildByID( IDC_ALPHA_TEAM_NAME      );
    m_pAlphaTeamScore   = (ui_text*)        FindChildByID( IDC_ALPHA_TEAM_SCORE     );

    m_pOmegaTeamList    = (ui_playerlist*)  FindChildByID( IDC_OMEGA_TEAM_LISTBOX   );
    m_pOmegaTeamHeader  = (ui_blankbox*)    FindChildByID( IDC_OMEGA_TEAM_HEADER    );
    m_pOmegaTeamName    = (ui_text*)        FindChildByID( IDC_OMEGA_TEAM_NAME      );
    m_pOmegaTeamScore   = (ui_text*)        FindChildByID( IDC_OMEGA_TEAM_SCORE     );

    m_pNavText          = (ui_text*)        FindChildByID( IDC_TEAM_CHANGE_NAV_TEXT );
 
    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
    m_pNavText->SetLabel( navText );
    m_pNavText->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);    


    // set up alpha team 
    m_pAlphaTeamList->SetFlag(ui_win::WF_SELECTED, TRUE);
    m_pAlphaTeamList->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pAlphaTeamList->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pAlphaTeamList->DisableFrame();
    m_pAlphaTeamList->SetExitOnSelect(FALSE);
    m_pAlphaTeamList->SetExitOnBack(TRUE);
    m_pAlphaTeamList->EnableHeaderBar();
    m_pAlphaTeamList->SetHeaderBarColor( xcolor(19,59,14,196) );
    m_pAlphaTeamList->SetHeaderColor( xcolor(255,252,204,255) );
    m_pAlphaTeamList->EnableParentNavigation();
    m_pAlphaTeamList->SetScoreFieldMask( SCORE_POINTS );
    m_pAlphaTeamList->SetMaxPlayerWidth( 115 );

    m_pAlphaTeamHeader->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pAlphaTeamHeader->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pAlphaTeamHeader->SetHasTitleBar( FALSE );
    m_pAlphaTeamHeader->SetBackgroundColor( xcolor(255,252,204,128) );
    
    m_pAlphaTeamName->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pAlphaTeamName->SetLabelColor( xcolor(19,59,14,255) );
    m_pAlphaTeamName->SetLabelFlags(ui_font::h_left|ui_font::v_center|ui_font::clip_ellipsis);

    m_pAlphaTeamScore->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pAlphaTeamScore->SetLabelColor( xcolor(19,59,14,255) );
    m_pAlphaTeamScore->SetLabelFlags(ui_font::h_right|ui_font::v_center);


    // set up omega team
    m_pOmegaTeamList->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pOmegaTeamList->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pOmegaTeamList->DisableFrame();
    m_pOmegaTeamList->SetExitOnSelect(FALSE);
    m_pOmegaTeamList->SetExitOnBack(TRUE);
    m_pOmegaTeamList->EnableHeaderBar();
    m_pOmegaTeamList->SetHeaderBarColor( xcolor(19,59,14,196) );
    m_pOmegaTeamList->SetHeaderColor( xcolor(255,252,204,255) );
    m_pOmegaTeamList->EnableParentNavigation();
    m_pOmegaTeamList->SetScoreFieldMask( SCORE_POINTS );
    m_pOmegaTeamList->SetMaxPlayerWidth( 115 );

    m_pOmegaTeamHeader->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pOmegaTeamHeader->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pOmegaTeamHeader->SetHasTitleBar( FALSE );
    m_pOmegaTeamHeader->SetBackgroundColor( xcolor(255,252,204,128) );
    
    m_pOmegaTeamName->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pOmegaTeamName->SetLabelColor( xcolor(19,59,14,255) );
    m_pOmegaTeamName->SetLabelFlags(ui_font::h_left|ui_font::v_center|ui_font::clip_ellipsis);

    m_pOmegaTeamScore->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pOmegaTeamScore->SetLabelColor( xcolor(19,59,14,255) );
    m_pOmegaTeamScore->SetLabelFlags(ui_font::h_right|ui_font::v_center);

    // set initial focus
    m_CurrHL = 0;
    GotoControl( (ui_control*)m_pAlphaTeamList );
    g_UiMgr->SetScreenHighlight( m_pAlphaTeamList->GetPosition() );
    m_PopUp = NULL;

    // populate the team lists
    FillTeamLists();

    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

	// Return success code
    return Success;
}

//=========================================================================

void dlg_team_change::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_team_change::Render( s32 ox, s32 oy )
{
    static s32 offset   =  0;
    static s32 gap      =  9;
    static s32 width    =  4;

	irect rb;

    // render background filter
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

void dlg_team_change::OnNotify ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData )
{
    (void)pWin;
    (void)pSender;
    (void)Command;
    (void)pData;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if (Command == WN_LIST_ACCEPTED)
        {
            // check which listbox
            if ( pSender == (ui_win*)m_pAlphaTeamList )
            {
                if( m_pAlphaTeamList->GetItemCount() > 0 )
                {
                    // change alpha team member to omega team
                    s32 VictimSlot  = m_pAlphaTeamList->GetSelectedItemData( 1 );
                    GameMgr.ChangeTeam( VictimSlot, TRUE );
                }
            }
            else
            {
                if( m_pOmegaTeamList->GetItemCount() > 0 )
                {
                    // change omega team member to alpha team
                    s32 VictimSlot  = m_pOmegaTeamList->GetSelectedItemData( 1 );
                    GameMgr.ChangeTeam( VictimSlot, TRUE );
                }
            }
        }
    }
}

//=========================================================================

void dlg_team_change::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
}

//=========================================================================

void dlg_team_change::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        g_AudioMgr.Play("Backup");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_team_change::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // turn on the controls
            m_pAlphaTeamList    ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pAlphaTeamHeader  ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pAlphaTeamName    ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pAlphaTeamScore   ->SetFlag( ui_win::WF_VISIBLE, TRUE );

            m_pOmegaTeamList    ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pOmegaTeamHeader  ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pOmegaTeamName    ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pOmegaTeamScore   ->SetFlag( ui_win::WF_VISIBLE, TRUE );

            m_pNavText          ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            
            GotoControl( (ui_control*)m_pAlphaTeamList );
            g_UiMgr->SetScreenHighlight( m_pAlphaTeamList->GetPosition() );
        }
    }

    // populate the team lists
    FillTeamLists();

    // check for result of popup box
    if ( m_PopUp )
    {
        if ( m_PopUpResult != DLG_POPUP_IDLE )
        {
            // clear popup 
            m_PopUp = NULL;

            // turn on nav text
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
    }


    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    s32 Highlight = 0;

    if (m_pAlphaTeamList->GetFlags() & ui_win::WF_SELECTED)
    {
        Highlight = 0;
    }
    else
    {
        Highlight = 1;
    }

    if ( m_CurrHL != Highlight )
    {
        switch( Highlight )
        {
            case 0:
            {
                // set list cursor
                s32 selection = m_pOmegaTeamList->GetSelection();
                s32 count = m_pAlphaTeamList->GetItemCount();

                if ( count )
                {
                    count--;

                    if ( selection > count )
                    {
                        m_pAlphaTeamList->SetSelection( count );
                    }
                    else if ( selection < 0 )
                    {
                        m_pAlphaTeamList->SetSelection( 0 );
                    }
                    else
                    {
                        m_pAlphaTeamList->SetSelection( selection );
                    }
                }
            }
            break;

            case 1:
            {
                // set list cursor
                s32 selection = m_pAlphaTeamList->GetSelection();
                s32 count = m_pOmegaTeamList->GetItemCount();

                if ( count )
                {
                    count--;

                    if ( selection > count )
                    {
                        m_pOmegaTeamList->SetSelection( count );
                    }
                    else if ( selection < 0 ) 
                    {
                        m_pOmegaTeamList->SetSelection( 0 );
                    }
                    else
                    {
                        m_pOmegaTeamList->SetSelection( selection );
                    }
                }
            }
            break;
        }

        m_CurrHL = Highlight;
    }
}

//=========================================================================

static s32 sort_compare(const void* pItem1, const void* pItem2)
{
    player_score* pScore1 = (player_score*)pItem1;
    player_score* pScore2 = (player_score*)pItem2;

    if( pScore1->Score > pScore2->Score )
    {
        return -1;
    }
    else if( pScore1->Score < pScore2->Score )
    {
        return 1;
    }
    else
    {
        // same score, sort on name
        return( x_strcmp( pScore1->NName, pScore2->NName ) );
    }
}

//=========================================================================

void dlg_team_change::FillTeamLists( void )
{
    s32 i;
    const game_score& ScoreData = GameMgr.GetScore();

    // set team titles
    m_pAlphaTeamName->SetLabel( ScoreData.Team[0].Name );
    m_pOmegaTeamName->SetLabel( ScoreData.Team[1].Name );

    s32 iSelAlpha = m_pAlphaTeamList->GetSelection();
    s32 iSelOmega = m_pOmegaTeamList->GetSelection();
    
    // Clear listbox
    m_pAlphaTeamList->DeleteAllItems();
    m_pOmegaTeamList->DeleteAllItems();


    // Clear the local list.
    x_memset( m_AlphaPlayerData, 0, sizeof(m_AlphaPlayerData) );
    x_memset( m_OmegaPlayerData, 0, sizeof(m_OmegaPlayerData) );

    s32 AlphaCount = 0;
    s32 OmegaCount = 0;

    // put the players in the list
    for( i=0 ; i<32 ; i++ )
    {
        if(ScoreData.Player[i].IsInGame)
        {
            // check if on the same team
            if( ScoreData.Player[i].Team ==  0 ) 
            {
                x_memcpy( &m_AlphaPlayerData[AlphaCount], &ScoreData.Player[i], sizeof(player_score) );
                m_AlphaPlayerData[AlphaCount].ClientIndex = i;
                AlphaCount++;
            }
            else
            {
                x_memcpy( &m_OmegaPlayerData[OmegaCount], &ScoreData.Player[i], sizeof(player_score) );
                m_OmegaPlayerData[OmegaCount].ClientIndex = i;
                OmegaCount++;
            }
        }
    }

    // sort the lists
    if( AlphaCount )
    {
        // sort the one list
        x_qsort( &m_AlphaPlayerData[0], AlphaCount, sizeof(player_score), sort_compare );

        // finally add the sorted players to the player list
        for( i=0 ; i<AlphaCount ; i++ )
        {
            m_pAlphaTeamList->AddItem( m_AlphaPlayerData[i].Name, (s32)&m_AlphaPlayerData[i], m_AlphaPlayerData[i].ClientIndex );
        }
    }

    if( OmegaCount )
    {
        // sort the one list
        x_qsort( &m_OmegaPlayerData[0], OmegaCount, sizeof(player_score), sort_compare );

        // finally add the sorted players to the player list
        for( i=0 ; i<OmegaCount ; i++ )
        {
            m_pOmegaTeamList->AddItem( m_OmegaPlayerData[i].Name, (s32)&m_OmegaPlayerData[i], m_OmegaPlayerData[i].ClientIndex );
        }
    }

    // Limit Selection
    if( iSelAlpha <  0 ) iSelAlpha = 0;
    if( iSelAlpha >= m_pAlphaTeamList->GetItemCount() ) iSelAlpha = m_pAlphaTeamList->GetItemCount()-1;
    if( iSelOmega <  0 ) iSelOmega = 0;
    if( iSelOmega >= m_pOmegaTeamList->GetItemCount() ) iSelOmega = m_pOmegaTeamList->GetItemCount()-1;

    // Set Selection
    if( m_pAlphaTeamList->GetItemCount() > 0 )
    {
        m_pAlphaTeamList->SetSelection( iSelAlpha );
    }
    if( m_pOmegaTeamList->GetItemCount() > 0 )
    {
        m_pOmegaTeamList->SetSelection( iSelOmega );
    }
}

//=========================================================================
