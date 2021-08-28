//=========================================================================
//
//  dlg_level_desc.cpp
//
//=========================================================================

#include "entropy.hpp"
#include "dlg_LevelDesc.hpp"
#include "StateMgr\StateMgr.hpp"
#include "StateMgr\MapList.hpp"
#include "stringmgr\stringmgr.hpp"

#include "ui\ui_font.hpp"

#ifdef TARGET_PS2
#include "ps2\ps2_misc.hpp"
#endif

xcolor g_TextRectLoadScreen( 0, 0, 0, 128);

//=========================================================================
//  Level Desc Dialog
//=========================================================================

enum controls
{
    IDC_LEVEL_DESC,
    //IDC_LEVEL_DESC_OUTER_FRAME,
    //IDC_LEVEL_DESC_FRAME_ONE,
    //IDC_LEVEL_DESC_FRAME_TWO,
    IDC_MAP_TITLE_TEXT,
    IDC_MAP_DESC_TEXT,
    IDC_GAME_TYPE_TEXT,
    IDC_GAME_DESC_TEXT,
    IDC_LEVEL_DESC_LOADING_TEXT,
    IDC_LEVEL_DESC_LOADING_PIPS,
    IDC_LEVEL_DESC_NAV_TEXT
};

ui_manager::control_tem LevelDescControls[] =
{
    { IDC_LEVEL_DESC,               "IDS_NULL",        "text",        0, 308, 480,  30,  0, 0, 1, 1, ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE  },
//  { IDC_LEVEL_DESC_OUTER_FRAME,   "IDS_NULL",        "frame",       0,   0, 496, 352, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
//  { IDC_LEVEL_DESC_FRAME_ONE,     "IDS_NULL",        "frame",      10,  10, 476, 332, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
//  { IDC_LEVEL_DESC_FRAME_ONE,     "IDS_NULL",        "frame",      10,  10, 476, 230, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
//  { IDC_LEVEL_DESC_FRAME_TWO,     "IDS_NULL",        "frame",      10, 250, 476,  97, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_MAP_TITLE_TEXT,           "IDS_NULL",        "text",       40,   8, 426,  22, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_MAP_DESC_TEXT,            "IDS_NULL",        "text",       40,  38, 426,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_GAME_TYPE_TEXT,           "IDS_NULL",        "text",       40, 190, 426,  22, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_GAME_DESC_TEXT,           "IDS_NULL",        "text",       40, 220, 426,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_LEVEL_DESC_LOADING_TEXT,  "IDS_NULL",        "text",      235, 395, 230,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_LEVEL_DESC_LOADING_PIPS,  "IDS_NULL",        "text",      465, 395,  50,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_STATIC|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
    { IDC_LEVEL_DESC_NAV_TEXT,      "IDS_NULL",        "text",       25, 395, 200,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE|ui_win::WF_SCALE_XPOS|ui_win::WF_SCALE_XSIZE },
};

ui_manager::dialog_tem LevelDescDialog =
{
    "IDS_LEVEL_DESC",
    1, 9,
    sizeof(LevelDescControls)/sizeof(ui_manager::control_tem),
    &LevelDescControls[0],
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

void dlg_level_desc_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "level desc", &LevelDescDialog, &dlg_level_desc_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_level_desc_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_level_desc* pDialog = new dlg_level_desc;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_level_desc
//=========================================================================

dlg_level_desc::dlg_level_desc( void )
{
#ifdef TARGET_XBOX
    m_XBOXNotificationOffsetX = -15;
    m_XBOXNotificationOffsetY = 0;
    m_bUseTopmost             = 1;
#endif
}

//=========================================================================

dlg_level_desc::~dlg_level_desc( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_level_desc::Create( s32                        UserID,
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

    // initialize screen scaling
    //InitScreenScaling( Position );

    // Initialize the frames
    //m_pFrameOuter        = (ui_frame*)FindChildByID( IDC_LEVEL_DESC_OUTER_FRAME  );
    //m_pFrameGameTypeDesc = (ui_frame*)FindChildByID( IDC_LEVEL_DESC_FRAME_ONE    );
    //m_pFrameLevelDesc    = (ui_frame*)FindChildByID( IDC_LEVEL_DESC_FRAME_TWO    );
    m_MapTitleText      = (ui_text*) FindChildByID( IDC_MAP_TITLE_TEXT          );
    m_MapDescText       = (ui_text*) FindChildByID( IDC_MAP_DESC_TEXT           );
    m_GameTypeText      = (ui_text*) FindChildByID( IDC_GAME_TYPE_TEXT          );
    m_GameDescText      = (ui_text*) FindChildByID( IDC_GAME_DESC_TEXT          );
    m_pLoadingText      = (ui_text*) FindChildByID( IDC_LEVEL_DESC_LOADING_TEXT );
    m_pLoadingPips      = (ui_text*) FindChildByID( IDC_LEVEL_DESC_LOADING_PIPS );
    m_pNavText          = (ui_text*) FindChildByID( IDC_LEVEL_DESC_NAV_TEXT     );

#ifdef TARGET_XBOX
    extern xbool g_b480P;
    if( g_b480P )
    {
        m_MapTitleText->SetPosition( irect( 40,  8+10, 40+426,  8+22 ));
        m_MapDescText ->SetPosition( irect( 40, 38+10, 40+426, 38+16 ));
        m_GameTypeText->SetPosition( irect( 40,190+10, 40+426,190+22 ));
        m_GameDescText->SetPosition( irect( 40,220+10, 40+426,220+16 ));
        m_pLoadingText->SetPosition( irect(235,395+10,235+230,395+16 ));
        m_pLoadingPips->SetPosition( irect(465,395+10,465+ 50,395+16 ));
        m_pNavText    ->SetPosition( irect( 25,395+10, 25+200,395+16 ));
    }
#endif

    //m_pFrameOuter->SetBackgroundColor( xcolor(39,117,28,64) );
    //m_pFrameOuter->ChangeElement("frame2");
    //m_pFrameGameTypeDesc->SetBackgroundColor( xcolor(39,117,28,64) );
    //m_pFrameGameTypeDesc->ChangeElement("frame2");
    //m_pFrameLevelDesc->SetBackgroundColor( xcolor(39,117,28,64) );
    //m_pFrameLevelDesc->ChangeElement("frame2");


    //m_pFrameOuter        ->SetFlag( ui_win::WF_VISIBLE, FALSE );
    //m_pFrameGameTypeDesc ->SetFlag( ui_win::WF_VISIBLE, FALSE );

    // initialize text strings    
    m_MapTitleText->SetFlag( ui_win::WF_VISIBLE, TRUE );
    m_MapTitleText->SetLabelFlags( ui_font::h_right|ui_font::v_top );//|ui_font::clip_r_justify );

    m_MapDescText->SetFlag( ui_win::WF_VISIBLE, TRUE );
    
    // descriptions are longer in most PAL languages, so, left justify the text
    if( x_GetTerritory() == XL_TERRITORY_EUROPE )
    {
        m_MapDescText->SetLabelFlags( ui_font::h_left|ui_font::v_top ); //|ui_font::clip_r_justify );
    }
    else
    {
        m_MapDescText->SetLabelFlags( ui_font::h_right|ui_font::v_top ); //|ui_font::clip_r_justify );
    }

    m_MapDescText->UseSmallText(TRUE);

    m_GameTypeText->SetFlag( ui_win::WF_VISIBLE, TRUE );
    m_GameTypeText->SetLabelFlags( ui_font::h_left|ui_font::v_top );

    m_GameDescText->SetFlag( ui_win::WF_VISIBLE, TRUE );
    m_GameDescText->SetLabelFlags( ui_font::h_left|ui_font::v_top );
    m_GameDescText->UseSmallText(TRUE);

    // get text from map list
    const map_entry* pEntry = g_MapList.Find( g_ActiveConfig.GetLevelID(), g_ActiveConfig.GetGameTypeID() );
    if( pEntry )
    {
        m_MapTitleText  ->SetLabel( pEntry->GetDisplayName()  );
        m_MapDescText   ->SetLabel( pEntry->GetDescription()  );
        m_GameTypeText  ->SetLabel( pEntry->GetGameTypeName() );
        m_GameDescText  ->SetLabel( pEntry->GetGameRules()    );
    }

    // initialize loading text
    m_pLoadingText->SetFlag( ui_win::WF_VISIBLE, TRUE );
    m_pLoadingText->SetLabelFlags( ui_font::h_right|ui_font::v_top );
    m_pLoadingText->UseSmallText(TRUE);

    // build loading text string
    xwstring LoadText;
    LoadText = g_StringTableMgr( "ui", "IDS_LOADING_MSG" );
    if( x_GetTerritory() == XL_TERRITORY_AMERICA )
    {
        LoadText += " ";
        LoadText += g_ActiveConfig.GetShortGameType();
        LoadText += ":";
        LoadText += g_ActiveConfig.GetLevelName();
    }
    m_pLoadingText->SetLabel( LoadText );

    // initialize loading pips    
    m_pLoadingPips->SetFlag( ui_win::WF_VISIBLE, TRUE );
    m_pLoadingPips->SetLabelFlags( ui_font::h_left|ui_font::v_top );
    m_pLoadingPips->UseSmallText(TRUE);

    // center loading text
    //s32 Width = g_UiMgr->TextWidth( g_UiMgr->FindFont("small"), LoadText );
    //s32 XRes, YRes;
    //eng_GetRes( XRes, YRes );
    //irect NewPos = m_pLoadingText->GetPosition();
    //NewPos.l = (XRes - Width) / 2;
    //NewPos.r = NewPos.l + Width;
    //m_pLoadingText->SetPosition( NewPos );

    m_pNavText->SetLabel( g_StringTableMgr( "ui", "IDS_NULL") );
    m_pNavText->SetFlag( ui_win::WF_VISIBLE, TRUE );
    m_pNavText->SetLabelFlags( ui_font::h_left|ui_font::v_top );
    m_pNavText->UseSmallText(TRUE);

    // load the appropriate background
#ifdef TARGET_XBOX
    xwstring LoadName( "UI_LoadScreen_MP_XBOX_" );
#else
    xwstring LoadName( "UI_LoadScreen_MP_" );
#endif
    if( IN_RANGE( 2000, g_ActiveConfig.GetLevelID(), 2999 ) &&
        ( x_GetTerritory() == XL_TERRITORY_AMERICA ) )
    {
        //LoadName += g_ActiveConfig.GetLevelName();
        LoadName += (const char*)xfs("%d", g_ActiveConfig.GetLevelID());
    }
    else
    {
        LoadName += "PAL";
    }
    LoadName += ".xbmp";
    g_UiMgr->LoadBackground ( "mp_load", xstring(LoadName) );

    // disable the highlight
    g_UiMgr->DisableScreenHighlight();

    // make the dialog active
    m_State           = DIALOG_STATE_ACTIVE;
    m_LoadingComplete = FALSE;
    m_ForceExit       = FALSE;
    m_Mode            = LEVEL_DESC_INITIAL;
    m_LoadTimeElapsed = 0.0f;

    g_UiMgr->SetUserBackground( g_UiUserID, "mp_load" );

    // Return success code
    return Success;
}

//=========================================================================

void dlg_level_desc::Destroy( void )
{
    ui_dialog::Destroy();

#ifdef TARGET_PS2
    // wait until we finish drawing before we unload the logo bitmap
    DLIST.Flush();
    DLIST.WaitForTasks();
#endif
    g_UiMgr->UnloadBackground( "mp_load" );
    g_UiMgr->SetUserBackground( g_UiUserID, "" );

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_level_desc::Render( s32 ox, s32 oy )
{
    (void)ox;
    (void)oy;

    if( (GetFlags() & ui_win::WF_VISIBLE)==0 )
    {
        return;
    }

    // descriptions are longer in most PAL languages, so, since the text is left justified
    // we'll need a dark rect to cover the screen to darken the map bitmap so the white text
    // won't bleed into it.
    if( x_GetTerritory() == XL_TERRITORY_EUROPE )
    {
        const view* pView = eng_GetView();
        rect r;
        pView->GetViewport( r );
        irect ir( (s32)r.Min.X, (s32)r.Min.Y, (s32)r.Max.X, (s32)r.Max.Y );

        draw_Rect( ir, g_TextRectLoadScreen, FALSE );
    }

    // render the normal ui dialog
    ui_dialog::Render( ox, oy );

#if 0
    // render the main title bar
    irect r;
    xcolor ch(255,252,204,255);
    s32 FontID = g_UiMgr->FindFont("small");

    r = m_pFrameGameTypeDesc->GetPosition();
    r.l += 35;
    r.r -= 5;
    r.t += 30;
    r.b -= 5;
    m_pManager->RenderText( m_Font, r, ui_font::h_left|ui_font::v_top, ch, g_StringTableMgr( "ui", "IDS_TEST_GAME_TYPE_TITLE" ) );
    r.t += 22;
    m_pManager->RenderText( FontID, r, ui_font::h_left|ui_font::v_top, ch, g_StringTableMgr( "ui", "IDS_TEST_GAME_TYPE_DESC" ) );

    //r = m_pFrameLevelDesc->GetPosition();
    //r.l += 15;
    //r.r -= 5;
    //r.t += 17;
    //r.b -= 5;
    s32 Height = m_pManager->TextHeight( FontID, g_StringTableMgr( "ui", "IDS_TEST_GAME_TYPE_DESC" ) );
    r.t += Height + 16;
    m_pManager->RenderText( m_Font, r, ui_font::h_left|ui_font::v_top, ch, g_StringTableMgr( "ui", "IDS_TEST_MAP_TITLE" ) );
    r.t += 22;
    m_pManager->RenderText( FontID, r, ui_font::h_left|ui_font::v_top, ch, g_StringTableMgr( "ui", "IDS_TEST_MAP_DESC" ) );
#endif



    // render loading text
    xwstring LoadText("");

    switch( (s32)(m_LoadTimeElapsed*4.0f) % 4 )
    {
    case 0: LoadText += "   ";   break;
    case 1: LoadText += ".  ";   break;
    case 2: LoadText += ".. ";   break;
    case 3: LoadText += "...";   break;
    }
    m_pLoadingPips->SetLabel( LoadText );
}

//=========================================================================

void dlg_level_desc::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if( m_Mode != LEVEL_DESC_INTERLEVEL )
        return;

    if( m_State == DIALOG_STATE_ACTIVE )
    {
        m_State = DIALOG_STATE_SELECT;
    }
}

//=========================================================================

void dlg_level_desc::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    m_LoadTimeElapsed += DeltaTime;

    // get text from map list
    const map_entry* pEntry = g_MapList.Find( g_ActiveConfig.GetLevelID(), g_ActiveConfig.GetGameTypeID() );
    if( pEntry )
    {
        m_MapTitleText  ->SetLabel( pEntry->GetDisplayName()  );
        m_MapDescText   ->SetLabel( pEntry->GetDescription()  );
        m_GameTypeText  ->SetLabel( pEntry->GetGameTypeName() );
        m_GameDescText  ->SetLabel( pEntry->GetGameRules()    );
    }

    if( m_LoadingComplete )
    {
        m_State = DIALOG_STATE_EXIT;
    }

    if( m_Mode == LEVEL_DESC_INTERLEVEL )
    {
        m_pNavText->SetLabel( g_StringTableMgr( "ui", "IDS_NAV_LEADERBOARD" ) );
    }

}

//=========================================================================

void dlg_level_desc::LoadingComplete( void )
{
    m_LoadingComplete = TRUE;
}

//=========================================================================

void dlg_level_desc::Configure( level_desc_mode Mode )
{
    m_Mode = Mode;
}

//=========================================================================
