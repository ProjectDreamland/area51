//=========================================================================
//
//  dlg_join_filter.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_textbox.hpp"

#include "dlg_JoinFilter.hpp"
#include "dlg_popup.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Configuration/GameConfig.hpp"
#include "MemCardMgr/MemCardMgr.hpp"


//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum controls
{
    IDC_JOIN_FILTER_GAME_TYPE,
    IDC_JOIN_FILTER_NUM_PLAYERS,
    IDC_JOIN_FILTER_MUTATION_MODE,
    IDC_JOIN_FILTER_PASSWORD,
    IDC_JOIN_FILTER_HEADSET,
    IDC_JOIN_FILTER_GAME_TYPE_SELECTOR,
    IDC_JOIN_FILTER_NUM_PLAYERS_SELECTOR,
    IDC_JOIN_FILTER_MUTATION_MODE_SELECTOR,
    IDC_JOIN_FILTER_PASSWORD_SELECTOR,
    IDC_JOIN_FILTER_HEADSET_SELECTOR,
    IDC_JOIN_FILTER_CONTINUE,
    IDC_JOIN_FILTER_NAV_TEXT,
};

ui_manager::control_tem JoinFilterControls_SPA[] = 
{
    // Frames.
    { IDC_JOIN_FILTER_GAME_TYPE,              "IDS_HOST_GAME_TYPE",       "text",    40,  40, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_NUM_PLAYERS,            "IDS_HOST_MIN_PLAYERS",     "text",    40,  80, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_MUTATION_MODE,          "IDS_HOST_MUTATION",        "text",    40, 120, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_PASSWORD,               "IDS_HOST_PASSWORD",        "text",    40, 160, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_HEADSET,                "IDS_HOST_VOICE_ENABLED",   "text",    40, 200, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_JOIN_FILTER_GAME_TYPE_SELECTOR,     "IDS_NULL",                 "combo",  190,  49, 230, 40,  0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_NUM_PLAYERS_SELECTOR,   "IDS_NULL",                 "combo",  300,  89, 120, 40,  0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_MUTATION_MODE_SELECTOR, "IDS_NULL",                 "combo",  207, 129, 213, 40,  0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_PASSWORD_SELECTOR,      "IDS_NULL",                 "combo",  300, 169, 120, 40,  0, 3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_HEADSET_SELECTOR,       "IDS_NULL",                 "combo",  300, 209, 120, 40,  0, 4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_CONTINUE,               "IDS_JOIN_FILTER_CONTINUE", "button",  50, 292, 100, 40,  0, 5, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_JOIN_FILTER_NAV_TEXT,               "IDS_NULL",                 "text",     0,   0,   0,  0,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};

ui_manager::control_tem JoinFilterControls_PAL[] = 
{
    // Frames.
    { IDC_JOIN_FILTER_GAME_TYPE,              "IDS_HOST_GAME_TYPE",       "text",    40,  40, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_NUM_PLAYERS,            "IDS_HOST_MIN_PLAYERS",     "text",    40,  80, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_MUTATION_MODE,          "IDS_HOST_MUTATION",        "text",    40, 120, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_PASSWORD,               "IDS_HOST_PASSWORD",        "text",    40, 160, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_HEADSET,                "IDS_HOST_VOICE_ENABLED",   "text",    40, 200, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_JOIN_FILTER_GAME_TYPE_SELECTOR,     "IDS_NULL",                 "combo",  220,  49, 205, 40,  0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_NUM_PLAYERS_SELECTOR,   "IDS_NULL",                 "combo",  220,  89, 205, 40,  0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_MUTATION_MODE_SELECTOR, "IDS_NULL",                 "combo",  220, 129, 205, 40,  0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_PASSWORD_SELECTOR,      "IDS_NULL",                 "combo",  220, 169, 205, 40,  0, 3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_HEADSET_SELECTOR,       "IDS_NULL",                 "combo",  220, 209, 205, 40,  0, 4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_CONTINUE,               "IDS_JOIN_FILTER_CONTINUE", "button",  50, 292, 100, 40,  0, 5, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_JOIN_FILTER_NAV_TEXT,               "IDS_NULL",                 "text",     0,   0,   0,  0,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};

ui_manager::control_tem JoinFilterControls_ENG[] = 
{
    // Frames.
    { IDC_JOIN_FILTER_GAME_TYPE,              "IDS_HOST_GAME_TYPE",       "text",    50,  40, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_NUM_PLAYERS,            "IDS_HOST_MIN_PLAYERS",     "text",    50,  80, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_MUTATION_MODE,          "IDS_HOST_MUTATION",        "text",    50, 120, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_PASSWORD,               "IDS_HOST_PASSWORD",        "text",    50, 160, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_HEADSET,                "IDS_HOST_VOICE_ENABLED",   "text",    50, 200, 100, 40,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_JOIN_FILTER_GAME_TYPE_SELECTOR,     "IDS_NULL",                 "combo",  240,  49, 180, 40,  0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_NUM_PLAYERS_SELECTOR,   "IDS_NULL",                 "combo",  240,  89, 180, 40,  0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_MUTATION_MODE_SELECTOR, "IDS_NULL",                 "combo",  240, 129, 180, 40,  0, 2, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_PASSWORD_SELECTOR,      "IDS_NULL",                 "combo",  240, 169, 180, 40,  0, 3, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_HEADSET_SELECTOR,       "IDS_NULL",                 "combo",  240, 209, 180, 40,  0, 4, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_JOIN_FILTER_CONTINUE,               "IDS_JOIN_FILTER_CONTINUE", "button",  50, 292, 100, 40,  0, 5, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_JOIN_FILTER_NAV_TEXT,               "IDS_NULL",                 "text",     0,   0,   0,  0,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem JoinFilterDialog =
{
    "IDS_JOIN_FILTER_TITLE",
    1, 9,
    sizeof(JoinFilterControls_ENG)/sizeof(ui_manager::control_tem),
    &JoinFilterControls_ENG[0],
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

void dlg_join_filter_register( ui_manager* pManager )
{
#ifndef X_EDITOR
    switch( x_GetLocale() )
    {        
    case XL_LANG_ENGLISH:    // English uses default
        break;

    case XL_LANG_SPANISH:
        {
            // set up new join options dialog controls
            JoinFilterDialog.StringID = "IDS_JOIN_FILTER_TITLE";
            JoinFilterDialog.NavW = 1;
            JoinFilterDialog.NavH = 9;
            JoinFilterDialog.nControls = sizeof(JoinFilterControls_SPA)/sizeof(ui_manager::control_tem);
            JoinFilterDialog.pControls = &JoinFilterControls_SPA[0];
            JoinFilterDialog.FocusControl = 0;
        }
        break;

    default:  // PAL
        {
            // set up new join options dialog controls
            JoinFilterDialog.StringID = "IDS_JOIN_FILTER_TITLE";
            JoinFilterDialog.NavW = 1;
            JoinFilterDialog.NavH = 9;
            JoinFilterDialog.nControls = sizeof(JoinFilterControls_PAL)/sizeof(ui_manager::control_tem);
            JoinFilterDialog.pControls = &JoinFilterControls_PAL[0];
            JoinFilterDialog.FocusControl = 0;
        }
        break;

    }
#endif    
    pManager->RegisterDialogClass( "join filter", &JoinFilterDialog, &dlg_join_filter_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_join_filter_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_join_filter* pDialog = new dlg_join_filter;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_join_filter
//=========================================================================

dlg_join_filter::dlg_join_filter( void )
{
}

//=========================================================================

dlg_join_filter::~dlg_join_filter( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_join_filter::Create( s32                        UserID,
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

    m_pGameTypeText     = (ui_text*)  FindChildByID( IDC_JOIN_FILTER_GAME_TYPE   );
    m_pNumPlayerText    = (ui_text*)  FindChildByID( IDC_JOIN_FILTER_NUM_PLAYERS );
    m_pMutationModeText = (ui_text*)  FindChildByID( IDC_JOIN_FILTER_MUTATION_MODE );
    m_pPasswordText     = (ui_text*)  FindChildByID( IDC_JOIN_FILTER_PASSWORD );
    m_pHeadsetText      = (ui_text*)  FindChildByID( IDC_JOIN_FILTER_HEADSET );
    m_pNavText          = (ui_text*)  FindChildByID( IDC_JOIN_FILTER_NAV_TEXT    );

    m_pGameTypeSelect   = (ui_combo*) FindChildByID( IDC_JOIN_FILTER_GAME_TYPE_SELECTOR   );
    m_pNumPlayerSelect  = (ui_combo*) FindChildByID( IDC_JOIN_FILTER_NUM_PLAYERS_SELECTOR );
    m_pMutationSelect   = (ui_combo*) FindChildByID( IDC_JOIN_FILTER_MUTATION_MODE_SELECTOR );
    m_pPasswordSelect   = (ui_combo*) FindChildByID( IDC_JOIN_FILTER_PASSWORD_SELECTOR );
    m_pHeadsetSelect    = (ui_combo*) FindChildByID( IDC_JOIN_FILTER_HEADSET_SELECTOR );

    m_pContinueButton   = (ui_button*)FindChildByID( IDC_JOIN_FILTER_CONTINUE );

    // set some text flags
    m_pGameTypeText     ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pNumPlayerText    ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pMutationModeText ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pPasswordText     ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
    m_pHeadsetText      ->SetLabelFlags( ui_font::h_left|ui_font::v_center );
   
    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
    m_pNavText->SetLabel( navText );
    m_pNavText->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);    

    // set up game type selector
    m_pGameTypeSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT );
    m_pGameTypeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_ANY"  ), -1  );
    m_pGameTypeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_GAMETYPE_DM"  ), GAME_DM  );
    m_pGameTypeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_GAMETYPE_TDM" ), GAME_TDM );
    m_pGameTypeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_GAMETYPE_CTF" ), GAME_CTF );
    m_pGameTypeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_GAMETYPE_CNH" ), GAME_CNH );
    m_pGameTypeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_GAMETYPE_INF" ), GAME_INF );
//  m_pGameTypeSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_GAMETYPE_TAG" ), GAME_TAG );


    // set up num player selector 
    m_pNumPlayerSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT );
    m_pNumPlayerSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_ANY" ), -1 );
    s32 i;
    for( i = 1; i < 16; i++ )
    {
        m_pNumPlayerSelect->AddItem( (const char*)xfs("%d",i), i );
    }
    /*
    m_pNumPlayerSelect->AddItem  ( "0",    0 );
    m_pNumPlayerSelect->AddItem  ( "1",    1 );
    m_pNumPlayerSelect->AddItem  ( "2",    2 );
    m_pNumPlayerSelect->AddItem  ( "3",    3 );
    m_pNumPlayerSelect->AddItem  ( "4",    4 );
    m_pNumPlayerSelect->AddItem  ( "5",    5 );
    m_pNumPlayerSelect->AddItem  ( "6",    6 );
    m_pNumPlayerSelect->AddItem  ( "7",    7 );
    m_pNumPlayerSelect->AddItem  ( "8",    8 );
    m_pNumPlayerSelect->AddItem  ( "9",    9 );
    m_pNumPlayerSelect->AddItem  ( "10",  10 );
    m_pNumPlayerSelect->AddItem  ( "11",  11 );
    m_pNumPlayerSelect->AddItem  ( "12",  12 );
    m_pNumPlayerSelect->AddItem  ( "13",  13 );
    m_pNumPlayerSelect->AddItem  ( "14",  14 );
    m_pNumPlayerSelect->AddItem  ( "15",  15 );
    */

    // set up MUTATION_MODE selector 
    m_pMutationSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT );
    m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_ANY" ), -1 );
    m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_CHANGE_AT_WILL"  ), MUTATE_CHANGE_AT_WILL  );
    m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_LOCK" ), MUTATE_HUMAN_LOCK );
    m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_LOCK" ), MUTATE_MUTANT_LOCK );
    m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_HUMAN_VS_MUTANT" ), MUTATE_HUMAN_VS_MUTANT );
    m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MAN_HUNT" ), MUTATE_MAN_HUNT );
    m_pMutationSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_MUTATION_MODE_MUTANT_HUNT" ), MUTATE_MUTANT_HUNT );

   // set up PASSWORD selector 
    m_pPasswordSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT );
    m_pPasswordSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_ANY" ), -1 );
    m_pPasswordSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_YES" ), 1  );
    m_pPasswordSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_NO"  ), 0  );
    // set up HEADSET selector 
    m_pHeadsetSelect->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT );
    m_pHeadsetSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_ANY" ), -1 );
    m_pHeadsetSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_YES" ), 1  );
    m_pHeadsetSelect->AddItem  ( g_StringTableMgr( "ui", "IDS_NO"  ), 0  );

    // set button alignment
    m_pContinueButton   ->SetFlag( ui_win::WF_BUTTON_LEFT, TRUE );

    // switch off the buttons to start
    m_pGameTypeText     ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pNumPlayerText    ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pMutationModeText ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pPasswordText     ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pHeadsetText      ->SetFlag( ui_win::WF_VISIBLE, FALSE );

    m_pGameTypeSelect   ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pNumPlayerSelect  ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pMutationSelect   ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pPasswordSelect   ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pHeadsetSelect    ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pContinueButton   ->SetFlag( ui_win::WF_VISIBLE, FALSE );

    // set initial highlight/control
    m_CurrHL = 0;
    m_pContinueButton->SetFlag(ui_win::WF_SELECTED, TRUE);
    GotoControl( (ui_control*)m_pContinueButton );

    // Clear popup pointer
    m_PopUp = NULL;

    // get pending settings
    join_settings& Settings = g_StateMgr.GetPendingSettings().GetJoinSettings();

    // intialize game type
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
    
    // intialize num players
    s32 Sel = 0;
    for( i=0; i< m_pNumPlayerSelect->GetItemCount(); i++ )
    {
        //if( g_PendingConfig.GetMaxPlayerCount() == m_pNumPlayerSelect->GetItemData( i, 0 ) )
        if( Settings.m_MinPlayers == m_pNumPlayerSelect->GetItemData(i) )
        {
            Sel = i;
            break;
        }
    }
    m_pNumPlayerSelect->SetSelection( Sel );

    // initialize mutation selection
    Sel = 0;
    for( i=0; i<m_pMutationSelect->GetItemCount(); i++)
    {
        if( Settings.m_MutationMode == m_pMutationSelect->GetItemData(i) )
        {
            Sel = i;
            break;
        }
    }
    m_pMutationSelect->SetSelection( Sel );

    Sel = 0;
    for( i=0; i<m_pPasswordSelect->GetItemCount(); i++)
    {
        if( Settings.m_PasswordEnabled == m_pPasswordSelect->GetItemData(i) )
        {
            Sel = i;
            break;
        }
    }
    m_pPasswordSelect->SetSelection( Sel );

    Sel = 0;
    for( i=0; i<m_pHeadsetSelect->GetItemCount(); i++)
    {
        if( Settings.m_VoiceEnabled == m_pHeadsetSelect->GetItemData(i) )
        {
            Sel = i;
            break;
        }
    }
    m_pHeadsetSelect->SetSelection( Sel );

    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_join_filter::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_join_filter::Render( s32 ox, s32 oy )
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

void dlg_join_filter::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
}

//=========================================================================

void dlg_join_filter::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        g_AudioMgr.Play("Backup");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_join_filter::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( pWin == (ui_win*)m_pContinueButton )
        {
            // store selections
            g_PendingConfig.SetGameType( m_pGameTypeSelect->GetSelectedItemLabel() );
            g_PendingConfig.SetMaxPlayerCount( m_pNumPlayerSelect->GetSelectedItemData() );
            g_PendingConfig.SetMutationMode((mutation_mode)m_pMutationSelect->GetSelectedItemData());
            g_PendingConfig.SetPasswordEnabled(m_pPasswordSelect->GetSelectedItemData());
            g_PendingConfig.SetVoiceEnabled(m_pHeadsetSelect->GetSelectedItemData());

            // update game settings
            join_settings& Settings = g_StateMgr.GetPendingSettings().GetJoinSettings();
            Settings.m_GameTypeID = m_pGameTypeSelect->GetSelectedItemData();
            Settings.m_MinPlayers = m_pNumPlayerSelect->GetSelectedItemData();
            Settings.m_MutationMode     = m_pMutationSelect->GetSelectedItemData();
            Settings.m_PasswordEnabled  = m_pPasswordSelect->GetSelectedItemData();
            Settings.m_VoiceEnabled     = m_pHeadsetSelect->GetSelectedItemData();

            g_MatchMgr.SetFilter( (game_type)Settings.m_GameTypeID, Settings.m_MinPlayers, -1 ,  Settings.m_MutationMode,Settings.m_PasswordEnabled,Settings.m_VoiceEnabled);
            // check if settings have changed
            if( g_StateMgr.GetPendingSettings().HasChanged() )
            {
                // settings have changed -save changes?
                irect r = g_UiMgr->GetUserBounds( g_UiUserID );
                m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

                // set nav text
                xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
                navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
                m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

                // configure message
                m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_GAMEMGR_POPUP" ), 
                    TRUE, 
                    TRUE, 
                    FALSE, 
                    g_StringTableMgr( "ui", "IDS_SAVE_FILTER_MSG" ),
                    navText,
                    &m_PopUpResult );

                return;
            }
            else
            {
                m_State = DIALOG_STATE_ACTIVATE;            
                g_AudioMgr.Play("Select_Norm");
            }
        }
    }
}

//=========================================================================

void dlg_join_filter::OnUpdate ( ui_win* pWin, f32 DeltaTime )
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
            m_pGameTypeText     ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pNumPlayerText    ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pMutationModeText ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pPasswordText     ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pHeadsetText      ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pContinueButton   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pNavText          ->SetFlag( ui_win::WF_VISIBLE, TRUE );

            m_pGameTypeSelect   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pNumPlayerSelect  ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pMutationSelect   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pPasswordSelect   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pHeadsetSelect    ->SetFlag( ui_win::WF_VISIBLE, TRUE );
    
            GotoControl( (ui_control*)m_pContinueButton );
            m_pContinueButton->SetFlag(ui_win::WF_HIGHLIGHT, TRUE);
            g_UiMgr->SetScreenHighlight( m_pContinueButton->GetPosition() );
        }
    }

    // check for result of popup box
    if ( m_PopUp )
    {
        if ( m_PopUpResult != DLG_POPUP_IDLE )
        {
            // clear popup 
            m_PopUp = NULL;

            // turn on nav text
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);

            switch( m_PopUpResult )
            {
                case DLG_POPUP_YES:
                {
                    // save changes
                    // check if the settings are saved 
                    if( g_StateMgr.GetSettingsCardSlot() == -1 )
                    {
#ifdef TARGET_PS2
                        // not saved - goto memory card select screen
                        m_State = DIALOG_STATE_MEMCARD_ERROR;
#else
                        // attempt to create settings
                        g_StateMgr.SetSettingsCardSlot( 0 );
                        g_UIMemCardMgr.CreateSettings( this, &dlg_join_filter::OnSaveSettingsCB );
                        m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
#endif
                    }
                    else
                    {                            
                        // OK. save changes
                        g_AudioMgr.Play("Select_Norm");
                        // attempt to save the changes to the memcard
                        g_UIMemCardMgr.SaveSettings( this, &dlg_join_filter::OnSaveSettingsCB );
                        m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
                    }
                }
                break;

                case DLG_POPUP_NO:
                {
                    // continue without saving
                    m_State = DIALOG_STATE_ACTIVATE;
                    // Activate the pending settings right now. Even though the player opted not to
                    // save, the settings should be preserved locally.
                    g_StateMgr.ActivatePendingSettings();
                }
                break;
            }
        }
    }


    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    // update labels  
    if( m_pGameTypeSelect->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 0;
        m_pGameTypeText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pGameTypeText->GetPosition() );
    }
    else
    {
        m_pGameTypeText->SetLabelColor( xcolor(126,220,60,255) );
    }

    if( m_pNumPlayerSelect->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 1;
        m_pNumPlayerText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pNumPlayerText->GetPosition() );
    }
    else
    {
        m_pNumPlayerText->SetLabelColor( xcolor(126,220,60,255) );
    }

   if( m_pMutationSelect->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 2;
        m_pMutationModeText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pMutationModeText->GetPosition() );
    }
    else
    {
        m_pMutationModeText->SetLabelColor( xcolor(126,220,60,255) );
    }

   if( m_pPasswordSelect->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 3;
        m_pPasswordText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pPasswordText->GetPosition() );
    }
    else
    {
        m_pPasswordText->SetLabelColor( xcolor(126,220,60,255) );
    }

   if( m_pHeadsetSelect->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 4;
        m_pHeadsetText->SetLabelColor( xcolor(255,252,204,255) );
        g_UiMgr->SetScreenHighlight( m_pHeadsetText->GetPosition() );
    }
    else
    {
        m_pHeadsetText->SetLabelColor( xcolor(126,220,60,255) );
    }



    if( m_pContinueButton->GetFlags(WF_HIGHLIGHT) )
    {
        highLight = 5;
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

void dlg_join_filter::OnSaveSettingsCB( void )
{
    // check if the save was successful (OR user wants to continue without saving)
#ifdef TARGET_PS2
    MemCardMgr::condition& Condition1 = g_UIMemCardMgr.GetCondition( g_StateMgr.GetSettingsCardSlot() );
#else
    MemCardMgr::condition& Condition1 = g_UIMemCardMgr.GetCondition( 0 );
#endif
    if( Condition1.SuccessCode )
    {
        // activate the new settings
        g_StateMgr.ActivatePendingSettings();

        // continue without saving?
        if( Condition1.bCancelled )
        {
            // clear settings card slot
            g_StateMgr.SetSettingsCardSlot(-1);
        }

        // save successful - onward to load game
        g_AudioMgr.Play( "Select_Norm" );
        m_State = DIALOG_STATE_ACTIVATE;            
    }
    else
    {
        // save failed! - goto memcard select dialog
        g_AudioMgr.Play( "Backup" );

        // clear settings card slot
        g_StateMgr.SetSettingsCardSlot(-1);

        // handle error condition
        m_State = DIALOG_STATE_MEMCARD_ERROR;
    }
}

//=========================================================================

