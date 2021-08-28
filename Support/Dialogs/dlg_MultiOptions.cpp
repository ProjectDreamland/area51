//=========================================================================
//
//  dlg_multi_options.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_textbox.hpp"
#include "ui\ui_edit.hpp"

#include "dlg_MultiOptions.hpp"
#include "dlg_popup.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Parsing/textin.hpp"
#include "StateMgr/mapList.hpp"
#include "Configuration/GameConfig.hpp"

//=========================================================================
//  Main Menu Dialog
//=========================================================================

#if defined(CONFIG_VIEWER) || defined(CONFIG_DEBUG) || defined(CONFIG_OPTDEBUG) 
#define ALLOW_EXTENDED_OPTIONS
#endif

enum controls
{
    IDC_MULTI_OPTIONS_TIME_LIMIT,
    IDC_MULTI_OPTIONS_SCORE,
#if defined (ALLOW_EXTENDED_OPTIONS)
    IDC_MULTI_OPTIONS_MUTATION,
    IDC_MULTI_OPTIONS_GAME_TYPE,
#endif
    IDC_MULTI_OPTIONS_TIME_SELECTOR,
    IDC_MULTI_OPTIONS_SCORE_SELECTOR,
#if defined (ALLOW_EXTENDED_OPTIONS)
    IDC_MULTI_OPTIONS_MUTATION_SELECTOR,
    IDC_MULTI_OPTIONS_TYPE_SELECTOR,
#endif
    IDC_MULTI_OPTIONS_CONTINUE,

    IDC_MULTI_OPTIONS_NAV_TEXT,
};


ui_manager::control_tem MultiOptionsControls[] = 
{
    // Frames.
    { IDC_MULTI_OPTIONS_TIME_LIMIT,         "IDS_HOST_TIME_LIMIT",      "text",    40,  40, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MULTI_OPTIONS_SCORE,              "IDS_HOST_SCORE",           "text",    40,  75, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#if defined (ALLOW_EXTENDED_OPTIONS)
    { IDC_MULTI_OPTIONS_MUTATION,           "IDS_HOST_MUTATION",        "text",    40, 110, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MULTI_OPTIONS_GAME_TYPE,          "IDS_HOST_GAME_TYPE",       "text",    40, 145, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#endif

    { IDC_MULTI_OPTIONS_TIME_SELECTOR,      "IDS_NULL",                 "combo",  230,  49, 200, 40,  0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MULTI_OPTIONS_SCORE_SELECTOR,     "IDS_NULL",                 "combo",  230,  84, 200, 40,  0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#if defined (ALLOW_EXTENDED_OPTIONS)
    { IDC_MULTI_OPTIONS_MUTATION_SELECTOR,  "IDS_NULL",                 "combo",  230, 119, 200, 40,  0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MULTI_OPTIONS_TYPE_SELECTOR,      "IDS_NULL",                 "combo",  230, 154, 200, 40,  0, 3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
#endif
    { IDC_MULTI_OPTIONS_CONTINUE,           "IDS_HOST_CONTINUE",        "button",  40, 285, 180, 40,  0, 4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_MULTI_OPTIONS_NAV_TEXT,           "IDS_NULL",                 "text",     0,   0,   0,  0,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem MultiOptionsDialog =
{
    "IDS_MULTI_PLAYER_OPTIONS",
    1, 9,
    sizeof(MultiOptionsControls)/sizeof(ui_manager::control_tem),
    &MultiOptionsControls[0],
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

void dlg_multi_options_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "multi options", &MultiOptionsDialog, &dlg_multi_options_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_multi_options_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_multi_options* pDialog = new dlg_multi_options;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_multi_options
//=========================================================================

dlg_multi_options::dlg_multi_options( void )
{
}

//=========================================================================

dlg_multi_options::~dlg_multi_options( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_multi_options::Create( s32                        UserID,
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

    m_pTimeLimitText    = (ui_text*)FindChildByID( IDC_MULTI_OPTIONS_TIME_LIMIT );
    m_pScoreText        = (ui_text*)FindChildByID( IDC_MULTI_OPTIONS_SCORE      );
#if defined (ALLOW_EXTENDED_OPTIONS)
    m_pMutationText     = (ui_text*)FindChildByID( IDC_MULTI_OPTIONS_MUTATION   );
    m_pGameTypeText     = (ui_text*)FindChildByID( IDC_MULTI_OPTIONS_GAME_TYPE  );
#endif
    m_pNavText          = (ui_text*)FindChildByID( IDC_MULTI_OPTIONS_NAV_TEXT   );

    m_pTimeSelect       = (ui_combo*)FindChildByID( IDC_MULTI_OPTIONS_TIME_SELECTOR     );
    m_pScoreSelect      = (ui_combo*)FindChildByID( IDC_MULTI_OPTIONS_SCORE_SELECTOR    );
#if defined (ALLOW_EXTENDED_OPTIONS)
    m_pMutationSelect   = (ui_combo*)FindChildByID( IDC_MULTI_OPTIONS_MUTATION_SELECTOR );
    m_pGameTypeSelect   = (ui_combo*)FindChildByID( IDC_MULTI_OPTIONS_TYPE_SELECTOR     );
#endif

    // set up continue button 
    m_pContinueButton   = (ui_button*)FindChildByID( IDC_MULTI_OPTIONS_CONTINUE );
    m_pContinueButton   ->SetFlag( ui_win::WF_BUTTON_LEFT, TRUE );

    // set some text flags
    m_pTimeLimitText    ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pScoreText        ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
#if defined (ALLOW_EXTENDED_OPTIONS)
    m_pMutationText     ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pGameTypeText     ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
   
    // set up game type selector
    m_pGameTypeSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT | ui_combo::CB_NOTIFY_PARENT );
    m_pGameTypeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_GAMETYPE_DM"  ), GAME_DM  );
    m_pGameTypeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_GAMETYPE_TDM" ), GAME_TDM );
    m_pGameTypeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_GAMETYPE_CTF" ), GAME_CTF );
    m_pGameTypeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_GAMETYPE_CNH" ), GAME_CNH );
    m_pGameTypeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_GAMETYPE_INF" ), GAME_INF );
//  m_pGameTypeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_GAMETYPE_TAG" ), GAME_TAG );
    m_pGameTypeSelect->SetSelection( 0 );

    // set up mutation selector
    m_pMutationSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT );
    m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_CHANGE_AT_WILL"  ), MUTATE_CHANGE_AT_WILL  );
    m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_LOCK"      ), MUTATE_HUMAN_LOCK      );
    m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_LOCK"     ), MUTATE_MUTANT_LOCK     );
    m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_VS_MUTANT" ), MUTATE_HUMAN_VS_MUTANT );
    m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MAN_HUNT"        ), MUTATE_MAN_HUNT        );
    m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_HUNT"     ), MUTATE_MUTANT_HUNT     );
#endif

    // set up time selector
    s32 OneMin = 60;
    m_pTimeSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT );
    m_pTimeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_NO_LIMIT" ),   -1 );
    m_pTimeSelect->AddItem  (  "1:00",  1 * OneMin );
    m_pTimeSelect->AddItem  (  "2:00",  2 * OneMin );
    m_pTimeSelect->AddItem  (  "5:00",  5 * OneMin );
    m_pTimeSelect->AddItem  ( "10:00", 10 * OneMin );
    m_pTimeSelect->AddItem  ( "15:00", 15 * OneMin );
    m_pTimeSelect->AddItem  ( "20:00", 20 * OneMin );
    m_pTimeSelect->AddItem  ( "30:00", 30 * OneMin );
    m_pTimeSelect->AddItem  ( "45:00", 45 * OneMin );
    m_pTimeSelect->AddItem  ( "60:00", 60 * OneMin );

    // set up score selector
    m_pScoreSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT );
    m_pScoreSelect->AddItem  (   "5",    5 );
    m_pScoreSelect->AddItem  (  "10",   10 );
    m_pScoreSelect->AddItem  (  "15",   15 );
    m_pScoreSelect->AddItem  (  "20",   20 );
    m_pScoreSelect->AddItem  (  "25",   25 );
    m_pScoreSelect->AddItem  (  "30",   30 );
    m_pScoreSelect->AddItem  (  "35",   35 );
    m_pScoreSelect->AddItem  (  "40",   40 );
    m_pScoreSelect->AddItem  (  "45",   45 );
    m_pScoreSelect->AddItem  (  "50",   50 );
    m_pScoreSelect->AddItem  (  "60",   60 );
    m_pScoreSelect->AddItem  (  "70",   70 );
    m_pScoreSelect->AddItem  (  "80",   80 );
    m_pScoreSelect->AddItem  (  "90",   90 );
    m_pScoreSelect->AddItem  ( "100",  100 );
    m_pScoreSelect->AddItem  ( "125",  125 );
    m_pScoreSelect->AddItem  ( "150",  150 );
    m_pScoreSelect->AddItem  ( "175",  175 );
    m_pScoreSelect->AddItem  ( "200",  200 );
    m_pScoreSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_NO_LIMIT" ), -1 );

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
  
    m_pNavText->SetLabel( navText );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // switch off the buttons to start
    m_pTimeLimitText    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pScoreText        ->SetFlag(ui_win::WF_VISIBLE, FALSE);
#if defined (ALLOW_EXTENDED_OPTIONS)
    m_pMutationText     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pGameTypeText     ->SetFlag(ui_win::WF_VISIBLE, FALSE);
#endif
    m_pContinueButton   ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    m_pTimeSelect       ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pScoreSelect      ->SetFlag(ui_win::WF_VISIBLE, FALSE);
#if defined (ALLOW_EXTENDED_OPTIONS)
    m_pMutationSelect   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pGameTypeSelect   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
#endif

    m_pNavText          ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // set initial highlight/control
    m_CurrHL = 4;
    m_pContinueButton->SetFlag(ui_win::WF_SELECTED, TRUE);
    GotoControl( (ui_control*)m_pContinueButton );

    // get pending settings 
    multi_settings& Settings = g_StateMgr.GetPendingSettings().GetMultiplayerSettings();

    s32 i;
    s32 sel = 0;
#if defined (ALLOW_EXTENDED_OPTIONS)
    // initialize game type
    s32 gameType = 0;

    for( i=0; i< m_pGameTypeSelect->GetItemCount(); i++ )
    {
        //if( x_wstrcmp( g_PendingConfig.GetGameType(), m_pGameTypeSelect->GetItemLabel(i) )==0 )
        if( Settings.m_GameTypeID == m_pGameTypeSelect->GetItemData(i) )
        {
            gameType = i;
            break;
        }
    }
    m_pGameTypeSelect->SetSelection( gameType );

    // initialize mutation select
    InitializeMutationModes();
    sel = 0;
    for( i=0; i<m_pMutationSelect->GetItemCount(); i++ )
    {    
        if( Settings.m_MutationMode ==  m_pMutationSelect->GetItemData(i) )
        {
            sel = i;
            break;
        }
    }
    m_pMutationSelect->SetSelection( sel );
#endif
    
    // initialize game time
    sel = 0;
    for( i=0; i< m_pTimeSelect->GetItemCount(); i++ )
    {
        //if( g_PendingConfig.GetGameTime() == m_pTimeSelect->GetItemData( i, 0 ) )
        if( Settings.m_TimeLimit == m_pTimeSelect->GetItemData(i) )
        {
            sel = i;
            break;
        }
    }
    m_pTimeSelect->SetSelection( sel );
    
    // initialize score select
    sel = 0;
    for( i=0; i<m_pScoreSelect->GetItemCount(); i++ )
    {    
        //if( g_PendingConfig.GetScoreLimit() == m_pScoreSelect->GetItemData( i, 0 ) )
        if( Settings.m_ScoreLimit ==  m_pScoreSelect->GetItemData(i) )
        {
            sel = i;
            break;
        }
    }
    m_pScoreSelect->SetSelection( sel );

    // initialize controls
    m_CurrGameType = -1;

    // initialize the screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_multi_options::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_multi_options::Render( s32 ox, s32 oy )
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

void dlg_multi_options::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    if ( m_State == DIALOG_STATE_ACTIVE )
    {
#if defined (ALLOW_EXTENDED_OPTIONS)
        if( pWin == (ui_win*)m_pGameTypeSelect )
        {
            switch( Code )
            {
                case ui_manager::NAV_LEFT:
                case ui_manager::NAV_RIGHT:
                    // new game type selected - clear the map cycle
                    ResetMapCycle();
                    // reinitialize available mutation modes
                    InitializeMutationModes();
                    break;
                default:
                    ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
                    break;
            }
        }
        else
#endif
        {
            ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
        }
    }
}

//=========================================================================

void dlg_multi_options::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        g_AudioMgr.Play("Backup");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_multi_options::OnPadSelect( ui_win* pWin )
{
    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if ( pWin == (ui_win*)m_pContinueButton )
        {
            // get the pending settings
            multi_settings& Settings = g_StateMgr.GetPendingSettings().GetMultiplayerSettings();
    
            // update pending game settings
#if defined (ALLOW_EXTENDED_OPTIONS)
            Settings.m_GameTypeID   = m_pGameTypeSelect ->GetSelectedItemData();
            Settings.m_MutationMode = (mutation_mode)m_pMutationSelect ->GetSelectedItemData();
#else
            Settings.m_GameTypeID   = GAME_DM;
            Settings.m_MutationMode = MUTATE_CHANGE_AT_WILL;
#endif
            Settings.m_TimeLimit    = m_pTimeSelect     ->GetSelectedItemData();
            Settings.m_ScoreLimit   = m_pScoreSelect    ->GetSelectedItemData();

            // set max players
#ifdef TARGET_XBOX
            g_PendingConfig.SetMaxPlayerCount( 4 );
#else
            g_PendingConfig.SetMaxPlayerCount( 2 );
#endif

            // set game type
#if defined (ALLOW_EXTENDED_OPTIONS)
            g_PendingConfig.SetGameType( m_pGameTypeSelect->GetItemLabel(m_pGameTypeSelect->GetSelection()) ); 

            // set short game type
            switch( m_pGameTypeSelect->GetSelection() )
            {
            case 0:     g_PendingConfig.SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_DM"  ) );     break;
            case 1:     g_PendingConfig.SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_TDM" ) );     break;
            case 2:     g_PendingConfig.SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_CTF" ) );     break;
            case 3:     g_PendingConfig.SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_TAG" ) );     break;
            case 4:     g_PendingConfig.SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_INF" ) );     break;
            case 5:     g_PendingConfig.SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_CNH" ) );     break;
            default:    ASSERTS(FALSE, "Unknown gametype");
            }
            g_PendingConfig.SetGameTypeID( (game_type)m_pGameTypeSelect->GetSelectedItemData( 0 ) );

            // mutation
            g_PendingConfig.SetMutationMode( (mutation_mode)m_pMutationSelect->GetSelectedItemData() );
#else
            // always DM!
            g_PendingConfig.SetGameType( g_StringTableMgr( "ui", "IDS_GAMETYPE_DM" ) ); 
            g_PendingConfig.SetShortGameType( g_StringTableMgr( "ui", "IDS_SHORT_GAMETYPE_DM"  ) );
            g_PendingConfig.SetGameTypeID( GAME_DM );
            g_PendingConfig.SetMutationMode( MUTATE_CHANGE_AT_WILL );
#endif
            // set time limit (in seconds???)
            g_PendingConfig.SetGameTime( m_pTimeSelect->GetSelectedItemData( 0 ) );

            // set score limit
            g_PendingConfig.SetScoreLimit( m_pScoreSelect->GetSelectedItemData( 0 ) );

            // force maps to scale down
            g_PendingConfig.SetMapScalingEnabled ( TRUE );

            // goto the map select screen
            g_AudioMgr.Play("Select_Norm");
            m_State = DIALOG_STATE_SELECT;
        }
#if defined (ALLOW_EXTENDED_OPTIONS)
        else if( pWin == (ui_win*)m_pGameTypeSelect )
        {
            // new game type selected - clear the map cycle
            ResetMapCycle();
            // reinitialize available mutation modes
            InitializeMutationModes();
        }
#endif
    }
}

//=========================================================================

void dlg_multi_options::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    s32 highLight = -1;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // turn on the buttons
            m_pTimeLimitText    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pScoreText        ->SetFlag(ui_win::WF_VISIBLE, TRUE);
#if defined (ALLOW_EXTENDED_OPTIONS)
            m_pMutationText     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pGameTypeText     ->SetFlag(ui_win::WF_VISIBLE, TRUE);
#endif

            m_pTimeSelect       ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pScoreSelect      ->SetFlag(ui_win::WF_VISIBLE, TRUE);
#if defined (ALLOW_EXTENDED_OPTIONS)
            m_pMutationSelect   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pGameTypeSelect   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
#endif
            m_pContinueButton   ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            m_pNavText          ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            GotoControl( (ui_control*)m_pContinueButton );
            m_pContinueButton->SetFlag(WF_HIGHLIGHT, TRUE);  
            g_UiMgr->SetScreenHighlight( m_pContinueButton->GetPosition() );
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    // update labels
    if( m_pTimeSelect->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 0;
        m_pTimeLimitText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pTimeLimitText->GetPosition() );
    }
    else
        m_pTimeLimitText->SetLabelColor( xcolor(126,220,60,255) );

    if( m_pScoreSelect->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 1;
        m_pScoreText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pScoreText->GetPosition() );
    }
    else
        m_pScoreText->SetLabelColor( xcolor(126,220,60,255) );

#if defined (ALLOW_EXTENDED_OPTIONS)
    if( m_pMutationSelect->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 2;
        m_pMutationText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pMutationText->GetPosition() );
    }
    else
        m_pMutationText->SetLabelColor( xcolor(126,220,60,255) );

    if( m_pGameTypeSelect->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 3;
        m_pGameTypeText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pGameTypeText->GetPosition() );
    }
    else
        m_pGameTypeText->SetLabelColor( xcolor(126,220,60,255) );
#endif

    if( m_pContinueButton->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 4;
        g_UiMgr->SetScreenHighlight( m_pContinueButton->GetPosition() );
    }

    if( highLight != m_CurrHL )
    {
        if( highLight != -1 )
            g_AudioMgr.Play("Cusor_Norm");

        m_CurrHL = highLight;
    }
}

//=========================================================================

void dlg_multi_options::ResetMapCycle( void )
{
    // a new game type was selected so we must flush the map cycle
    multi_settings& Settings = g_StateMgr.GetPendingSettings().GetMultiplayerSettings();
    map_settings* pMapSettings = &Settings.m_MapSettings;

    pMapSettings->m_bUseDefault     = TRUE;
    pMapSettings->m_MapCycleCount   = 0;
    pMapSettings->m_MapCycleIdx     = 0;

    for( s32 i=0; i<MAP_CYCLE_SIZE; i++ )
    {
        pMapSettings->m_MapCycle[i]  = -1;
    }

}

//=========================================================================

void dlg_multi_options::InitializeMutationModes( void )
{
#if defined (ALLOW_EXTENDED_OPTIONS)
   // update the available mutation modes based on the game type
    switch( m_pGameTypeSelect->GetSelectedItemData() )
    {
    case GAME_DM:
        m_pMutationSelect->DeleteAllItems();
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_CHANGE_AT_WILL"  ), MUTATE_CHANGE_AT_WILL  );
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_LOCK"      ), MUTATE_HUMAN_LOCK      );
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_LOCK"     ), MUTATE_MUTANT_LOCK     );
    //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_VS_MUTANT" ), MUTATE_HUMAN_VS_MUTANT );
    //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MAN_HUNT"        ), MUTATE_MAN_HUNT        );
    //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_HUNT"     ), MUTATE_MUTANT_HUNT     );
        break;

    case GAME_TDM:
        m_pMutationSelect->DeleteAllItems();
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_CHANGE_AT_WILL"  ), MUTATE_CHANGE_AT_WILL  );
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_LOCK"      ), MUTATE_HUMAN_LOCK      );
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_LOCK"     ), MUTATE_MUTANT_LOCK     );
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_VS_MUTANT" ), MUTATE_HUMAN_VS_MUTANT );
    //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MAN_HUNT"        ), MUTATE_MAN_HUNT        );
    //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_HUNT"     ), MUTATE_MUTANT_HUNT     );
        break;

    case GAME_CTF:
        m_pMutationSelect->DeleteAllItems();
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_CHANGE_AT_WILL"  ), MUTATE_CHANGE_AT_WILL  );
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_LOCK"      ), MUTATE_HUMAN_LOCK      );
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_LOCK"     ), MUTATE_MUTANT_LOCK     );
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_VS_MUTANT" ), MUTATE_HUMAN_VS_MUTANT );
    //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MAN_HUNT"        ), MUTATE_MAN_HUNT        );
    //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_HUNT"     ), MUTATE_MUTANT_HUNT     );
        break;

    case GAME_TAG:
        m_pMutationSelect->DeleteAllItems();
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_CHANGE_AT_WILL"  ), MUTATE_CHANGE_AT_WILL  );
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_LOCK"      ), MUTATE_HUMAN_LOCK      );
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_LOCK"     ), MUTATE_MUTANT_LOCK     );
    //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_VS_MUTANT" ), MUTATE_HUMAN_VS_MUTANT );
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MAN_HUNT"        ), MUTATE_MAN_HUNT        );
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_HUNT"     ), MUTATE_MUTANT_HUNT     );
        break;

    case GAME_INF:
        m_pMutationSelect->DeleteAllItems();
    //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_CHANGE_AT_WILL"  ), MUTATE_CHANGE_AT_WILL  );
    //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_LOCK"      ), MUTATE_HUMAN_LOCK      );
    //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_LOCK"     ), MUTATE_MUTANT_LOCK     );
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_VS_MUTANT" ), MUTATE_HUMAN_VS_MUTANT );
    //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MAN_HUNT"        ), MUTATE_MAN_HUNT        );
    //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_HUNT"     ), MUTATE_MUTANT_HUNT     );            
        break;

    case GAME_CNH:
        m_pMutationSelect->DeleteAllItems();
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_CHANGE_AT_WILL"  ), MUTATE_CHANGE_AT_WILL  );
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_LOCK"      ), MUTATE_HUMAN_LOCK      );
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_LOCK"     ), MUTATE_MUTANT_LOCK     );
        m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_VS_MUTANT" ), MUTATE_HUMAN_VS_MUTANT );
    //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MAN_HUNT"        ), MUTATE_MAN_HUNT        );
    //  m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_HUNT"     ), MUTATE_MUTANT_HUNT     );
        break;

    default:    
        ASSERTS( FALSE, "Unknown gametype" );
    }

    // set default selection
    m_pMutationSelect->SetSelection( 0 );
#endif
}

//=========================================================================
