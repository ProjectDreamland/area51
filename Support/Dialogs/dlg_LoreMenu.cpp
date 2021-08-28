//=========================================================================
//
//  dlg_lore_menu.cpp
//
//=========================================================================

#include "Entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_blankbox.hpp"
#include "ui\ui_textbox.hpp"

#include "dlg_LoreMenu.hpp"
#include "StateMgr\StateMgr.hpp"
#include "StringMgr\StringMgr.hpp"
#include "StateMgr/mapList.hpp"
#include "StateMgr/LoreList.hpp"
#include "MoviePlayer/MoviePlayer.hpp"

#if defined(TARGET_PS2)
#include "Entropy\PS2\ps2_misc.hpp"
#endif

//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum controls
{   
    IDC_LORE_MAIN,
    IDC_LORE_DETAILS,
    IDC_LORE_SELECT,
    IDC_LORE_BUTTON_1,
    IDC_LORE_BUTTON_2,
    IDC_LORE_BUTTON_3,
    IDC_LORE_BUTTON_4,
    IDC_LORE_BUTTON_5,
    IDC_LORE_TEXT_1,
    IDC_LORE_TEXT_2,
    IDC_LORE_TEXT_3,
    IDC_LORE_BLACKOUT,
    IDC_LORE_TEXTBOX,
    IDC_NAV_TEXT,
};

ui_manager::control_tem LoreMenuControls[] = 
{
    // Frames.

    { IDC_LORE_SELECT,      "IDS_NULL",    "combo",      108,  40, 280,  40, 0, 0, 5, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_LORE_MAIN,        "IDS_NULL",    "blankbox",    40,  80, 416, 144, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_LORE_BUTTON_1,    "IDS_NULL",    "button",      56, 130,  64,  64, 0, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_LORE_BUTTON_2,    "IDS_NULL",    "button",     136, 130,  64,  64, 1, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_LORE_BUTTON_3,    "IDS_NULL",    "button",     216, 130,  64,  64, 2, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_LORE_BUTTON_4,    "IDS_NULL",    "button",     296, 130,  64,  64, 3, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_LORE_BUTTON_5,    "IDS_NULL",    "button",     376, 130,  64,  64, 4, 1, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_LORE_DETAILS,     "IDS_NULL",    "blankbox",    40, 240, 416,  94, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_LORE_TEXT_1,      "IDS_NULL",    "text",        48, 262, 400,  94, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_LORE_TEXT_2,      "IDS_NULL",    "text",        48, 278,  90,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_LORE_TEXT_3,      "IDS_NULL",    "text",        48, 294,  90,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

#ifdef TARGET_PS2
    { IDC_LORE_BLACKOUT,    "IDS_NULL",    "blankbox",    -9, -24, 513, 448, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_USE_ABSOLUTE },
#else
    { IDC_LORE_BLACKOUT,    "IDS_NULL",    "blankbox",  -113, -40, 722, 480, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_USE_ABSOLUTE },
#endif

    { IDC_LORE_TEXTBOX,     "IDS_NULL",    "textbox",     60, 240, 376,  93, 0, 2, 5, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_NAV_TEXT,         "IDS_NULL",    "text",         0,   0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem LoreMenuDialog =
{
    "IDS_LORE_MENU_TITLE",
    5, 9,
    sizeof(LoreMenuControls)/sizeof(ui_manager::control_tem),
    &LoreMenuControls[0],
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

static xbool s_Scaled = FALSE;

//=========================================================================
//  Registration function
//=========================================================================

void dlg_lore_menu_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "lore main", &LoreMenuDialog, &dlg_lore_menu_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_lore_menu_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_lore_menu* pDialog = new dlg_lore_menu;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_lore_menu
//=========================================================================

dlg_lore_menu::dlg_lore_menu( void )
{
}

//=========================================================================

dlg_lore_menu::~dlg_lore_menu( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_lore_menu::Create( s32                        UserID,
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

    m_CurrHL = 0;
    s_Scaled = FALSE;

    // load bitmaps
    m_LoreIconID[LORE_TYPE_VIDEO]   = g_UiMgr->LoadBitmap( "LoreVideo",  "UI_LoreVideo.xbmp"   );
    m_LoreIconID[LORE_TYPE_AUDIO]   = g_UiMgr->LoadBitmap( "LoreAudio",  "UI_LoreAudio.xbmp"   );
    m_LoreIconID[LORE_TYPE_STILL]   = g_UiMgr->LoadBitmap( "LoreStill",  "UI_LoreStill.xbmp"   );
    m_LoreIconID[LORE_TYPE_TEXT]    = g_UiMgr->LoadBitmap( "LoreText",   "UI_LoreText.xbmp"    );
    m_LoreIconID[LORE_TYPE_UNKNOWN] = g_UiMgr->LoadBitmap( "LoreNull",   "UI_LoreUnknown.xbmp" );

    // set up nav text 
    m_pNavText = (ui_text*) FindChildByID( IDC_NAV_TEXT );
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_BACK" ));
    //navText += g_StringTableMgr( "ui", "IDS_NAV_CYCLE_VAULT" );
    m_pNavText->SetLabel( navText );
    m_pNavText->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);    

    // setup lore main box
    m_pLoreMain = (ui_blankbox*)FindChildByID( IDC_LORE_MAIN );
    m_pLoreMain->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLoreMain->SetFlag(ui_win::WF_STATIC, TRUE);
    m_pLoreMain->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pLoreMain->SetHasTitleBar( TRUE );
    m_pLoreMain->SetLabel( g_StringTableMgr( "ui", "IDS_LORE_VAULT" ) );
    m_pLoreMain->SetLabelColor( xcolor(255,252,204,255) );
    m_pLoreMain->SetTitleBarColor( xcolor(19,59,14,196) );

    // setup lore details box
    m_pLoreDetails = (ui_blankbox*)FindChildByID( IDC_LORE_DETAILS );
    m_pLoreDetails->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLoreDetails->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pLoreDetails->SetFlag(ui_win::WF_STATIC, TRUE);
    m_pLoreDetails->SetHasTitleBar( TRUE );
    m_pLoreDetails->SetLabel( g_StringTableMgr( "ui", "IDS_LORE_DETAILS" ) );
    m_pLoreDetails->SetLabelColor( xcolor(255,252,204,255) );
    m_pLoreDetails->SetTitleBarColor( xcolor(19,59,14,196) );

    // setup blackout box
    m_pBlackOut = (ui_blankbox*)FindChildByID( IDC_LORE_BLACKOUT );
    m_pBlackOut->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pBlackOut->SetBackgroundColor( xcolor (0,0,0,0) );
    m_pBlackOut->SetFlag(ui_win::WF_STATIC, TRUE);

    // set up textbox
    m_pTextBox = (ui_textbox*)FindChildByID( IDC_LORE_TEXTBOX );
    
    m_pTextBox->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pTextBox->SetFlag( ui_win::WF_DISABLED, TRUE );
    m_pTextBox->SetExitOnBack( TRUE );
    m_pTextBox->SetExitOnSelect( TRUE );
    m_pTextBox->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pTextBox->DisableFrame();
    m_pTextBox->SetLabelFlags( ui_font::h_left|ui_font::v_top );
    
    // set up lore combo
    m_pLoreSelect   = (ui_combo*)FindChildByID( IDC_LORE_SELECT );
    m_pLoreSelect   ->SetNavFlags( ui_combo::CB_CHANGE_ON_NAV | ui_combo::CB_CHANGE_ON_SELECT | ui_combo::CB_NOTIFY_PARENT );
    m_pLoreSelect   ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // get the active player profile
    player_profile& Profile = g_StateMgr.GetActiveProfile( 0 );

    s32 Count = 0;
    for( s32 i=0; i<g_MapList.GetCount(); i++ )
    {
        const map_entry& Entry = *g_MapList.GetByIndex( i );
        s32 MapID = Entry.GetMapID();

        // check if of the correct game type
        if( ( Entry.GetGameType() == GAME_CAMPAIGN ) && ( MapID < 2000 ) )
        {
            if( Count < NUM_VAULTS )
            {
                // look up the vault by the mapID
                s32 VaultIndex;
                g_LoreList.GetVaultByMapID( MapID, VaultIndex );

#if 0 //defined (mbillington) || (jhowa)
                // unlock it all!
                if( 1 )
#else
                // see if we unlocked ANYTHING in this vault
                if( Profile.GetLoreAcquired( VaultIndex, -1 ) )
#endif
                {
                    // add an entry to the list
                    m_pLoreSelect->AddItem( Entry.GetDisplayName(), (s32)&Entry );
                    // increment count
                    Count++;
                }
            }
        }
    }

    ASSERT( Count ); // Should have at least 1!

    // clear new lore flag
    Profile.ClearNewLoreUnlocked();
    // checksum profile to prevent unwanted "changed" messages appearing
    Profile.Checksum();

    // set up buttons
    m_pLoreButton[0]  = (ui_button*) FindChildByID( IDC_LORE_BUTTON_1 );
    m_pLoreButton[1]  = (ui_button*) FindChildByID( IDC_LORE_BUTTON_2 );
    m_pLoreButton[2]  = (ui_button*) FindChildByID( IDC_LORE_BUTTON_3 );
    m_pLoreButton[3]  = (ui_button*) FindChildByID( IDC_LORE_BUTTON_4 );
    m_pLoreButton[4]  = (ui_button*) FindChildByID( IDC_LORE_BUTTON_5 );

    m_pLoreButton[0]  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLoreButton[1]  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLoreButton[2]  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLoreButton[3]  ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLoreButton[4]  ->SetFlag(ui_win::WF_VISIBLE, FALSE);


    // set up server info text
    m_pLoreLine1    = (ui_text*)FindChildByID( IDC_LORE_TEXT_1 );
    m_pLoreLine2    = (ui_text*)FindChildByID( IDC_LORE_TEXT_2 );
    m_pLoreLine3    = (ui_text*)FindChildByID( IDC_LORE_TEXT_3 );
    
    m_pLoreLine1    ->UseSmallText( TRUE );
    m_pLoreLine2    ->UseSmallText( TRUE );
    m_pLoreLine3    ->UseSmallText( TRUE );

    m_pLoreLine1    ->SetLabelFlags( ui_font::h_left|ui_font::v_top );
    m_pLoreLine2    ->SetLabelFlags( ui_font::h_left|ui_font::v_top );
    m_pLoreLine3    ->SetLabelFlags( ui_font::h_left|ui_font::v_top );

    m_pLoreLine1    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLoreLine2    ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLoreLine3    ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    m_pLoreLine1    ->SetFlag(ui_win::WF_STATIC, TRUE);
    m_pLoreLine2    ->SetFlag(ui_win::WF_STATIC, TRUE);
    m_pLoreLine3    ->SetFlag(ui_win::WF_STATIC, TRUE);

    m_pLoreLine1    ->SetLabelColor( xcolor(255,252,204,255) );
    m_pLoreLine2    ->SetLabelColor( xcolor(255,252,204,255) );
    m_pLoreLine3    ->SetLabelColor( xcolor(255,252,204,255) );


    m_pLoreSelect->SetSelection( 0 );
    PopulateLoreDetails( TRUE );

    // set focus
    GotoControl( (ui_control*)m_pLoreSelect );

    // Initialize popup
    m_PopUp = NULL;

    // Initialize screen scale factors
    s32 XRes, YRes;
    eng_GetRes( XRes, YRes );
    m_ScreenScaleX = (f32)XRes / 512.0f;
    m_ScreenScaleY = (f32)YRes / 448.0f;

    // Initialize icon scaling
    m_scaleCount        = 0.0f;
    m_bScreenIsOn       = FALSE;
    m_bScaleDown        = FALSE;
    m_TimeOut           = 0.0f;
    m_bCycleBitmap      = FALSE;
    m_bFullScreenMode   = FALSE;

#ifdef TARGET_PS2
    // get video mode
    eng_GetPALMode( m_bPalMode );
#endif
    
    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

	// Return success code
    return Success;
}

//=========================================================================

void dlg_lore_menu::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();

#ifdef TARGET_PS2
    // wait until we finish drawing before we unload the logo bitmap
    DLIST.Flush();
    DLIST.WaitForTasks();
#endif

    // unload lore bitmaps
    g_UiMgr->UnloadBitmap( "LoreVideo" );
    g_UiMgr->UnloadBitmap( "LoreAudio" );
    g_UiMgr->UnloadBitmap( "LoreStill" );
    g_UiMgr->UnloadBitmap( "LoreText" );
    g_UiMgr->UnloadBitmap( "LoreNull" );
    g_UiMgr->UnloadBitmap( "Still" );
}

//=========================================================================

void dlg_lore_menu::Render( s32 ox, s32 oy )
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

    // render full screen?
    if( m_bFullScreenMode )
    {
        irect r = g_UiMgr->GetUserBounds( g_UiUserID );

#ifdef TARGET_PS2
        if( m_bPalMode )
        {
            m_pManager->RenderBitmap( m_StillBitmapID, r, XCOLOR_WHITE ); // PAL
        }
        else
        {
            vector2 UV0( 0.0f, 0.0625f );
            vector2 UV1( 1.0f, 0.9375f );
            m_pManager->RenderBitmapUV( m_StillBitmapID, r, UV0, UV1, XCOLOR_WHITE );
        }
#elif defined TARGET_XBOX
        switch( g_PhysW )
        {
            case 1280: 
            case  720:
                m_pManager->RenderBitmap( m_StillBitmapID, r, XCOLOR_WHITE );
                break;

            case  640:
            {
                vector2 UV0( 0.0556f, 0.0f );
                vector2 UV1( 0.9444f, 1.0f );
                m_pManager->RenderBitmapUV( m_StillBitmapID, r, UV0, UV1, XCOLOR_WHITE );
            }
            break;

            default:
                ASSERT(0);
                break;
        }
#else
        m_pManager->RenderBitmap( m_StillBitmapID, r, XCOLOR_WHITE );
#endif

        m_pNavText->Render();
        return;
    }

    // render the popup screen (if any)
    if( m_bScreenIsOn )
    {                    
        // dim the background dialog
	    //s32 XRes, YRes;
        //eng_GetRes(XRes, YRes);
//#ifdef TARGET_PS2
        // Nasty hack to force PS2 to draw to rb.l = 0
        //irect rb( -1, 0, XRes, YRes );
//#else
        //irect rb( 0, 0, XRes, YRes );
//#endif
        //g_UiMgr->RenderRect( rb, xcolor(0,0,0,m_FadeLevel), FALSE );

        // render border
        g_UiMgr->RenderRect( m_DrawPos, xcolor(255,252,204,255), FALSE );

        // render movie/button bitmap
        if( m_scaleCount || m_TimeOut)
        {
            irect r = m_DrawPos;
            r.t += 2;
            r.l += 2;
            r.b -= 2;
            r.r -= 2;

            m_pManager->RenderBitmap( m_pSelectedIcon->GetBitmap(), r, xcolor(255,252,204,255) );
        }
        else
        {
            switch( m_CurrentType )
            {
                case LORE_TYPE_VIDEO:
                case LORE_TYPE_AUDIO:
#if !defined( TARGET_PC )
                    if( Movie.IsPlaying() )
                    {               
                        Movie.Render( vector2( 129.0f, 59.0f ), vector2( 256.0f, 192.0f ), TRUE );
                    }
#endif
                    break;

                default:
                    irect r = m_DrawPos;
                    r.t += 2;
                    r.l += 2;
                    r.b -= 2;
                    r.r -= 2;

                    // change the bitmap
                    if( m_bCycleBitmap )
                    {
                        m_bCycleBitmap = FALSE;
                        g_UiMgr->UnloadBitmap( "Still" );
                        if( m_CurrentType == LORE_TYPE_TEXT )
                        {
                            m_StillBitmapID = g_UiMgr->LoadBitmap( "Still", xfs( "%s.xbmp", m_FileName ) );
                        }
                        else
                        {
                            m_StillBitmapID = g_UiMgr->LoadBitmap( "Still", xfs( "%s%d.xbmp", m_FileName, m_CurrItem+1 ) );
                        }
                    }

#ifdef TARGET_PS2
                    if( m_bPalMode )
                    {
                        m_pManager->RenderBitmap( m_StillBitmapID, r, XCOLOR_WHITE ); //PAL
                    }
                    else
                    {
                        vector2 UV0( 0.0f, 0.0625f );
                        vector2 UV1( 1.0f, 0.9375f );
                        m_pManager->RenderBitmapUV( m_StillBitmapID, r, UV0, UV1, XCOLOR_WHITE );
                    }
#elif defined TARGET_XBOX
                    switch( g_PhysW )
                    {
                        case 1280: 
                        case  720:
                            m_pManager->RenderBitmap( m_StillBitmapID, r, XCOLOR_WHITE );
                            break;

                        case  640:
                        {
                            vector2 UV0( 0.0556f, 0.0f );
                            vector2 UV1( 0.9444f, 1.0f );
                            m_pManager->RenderBitmapUV( m_StillBitmapID, r, UV0, UV1, XCOLOR_WHITE );
                        }
                        break;

                        default:
                            ASSERT(0);
                            break;
                    }
#else
                    m_pManager->RenderBitmap( m_StillBitmapID, r, XCOLOR_WHITE );
#endif
            }
        }
    }
}


//=========================================================================

void dlg_lore_menu::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    // only allow navigation if active
    if( m_State == DIALOG_STATE_ACTIVE )
    {
        if( pWin == (ui_win*)m_pLoreSelect )
        {
            switch( Code )
            {
                case ui_manager::NAV_LEFT:
                case ui_manager::NAV_RIGHT:
                    PopulateLoreDetails( TRUE );
                    return;
            }
        }
        ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
    }
    else if( m_State == DIALOG_STATE_ACTIVATE )
    {
        // check for cycling images
        if ( m_CurrentType == LORE_TYPE_STILL )
        {
            if( m_NumItems > 1 )
            {
                switch( Code )
                {
                    case ui_manager::NAV_LEFT:
                        if( --m_CurrItem < 0 )
                            m_CurrItem = m_NumItems-1;
                        break;

                    case ui_manager::NAV_RIGHT:
                        if( ++m_CurrItem == m_NumItems )
                            m_CurrItem = 0;
                        break;
                }

                // change the bitmap
                m_bCycleBitmap = TRUE;

                // change the text
                m_pTextBox->SetLabel( g_StringTableMgr( "lore", xfs( "%s_%d", m_FullDesc, m_CurrItem+1 ) ) );
            }
        }
    }
}

//=========================================================================

void dlg_lore_menu::OnNotify ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData )
{
    (void)pWin;
    (void)pSender;
    (void)Command;
    (void)pData;

    if( pSender == (ui_win*)m_pLoreSelect )
    {
        if( Command == WN_COMBO_SELCHANGE )
        {
            if( !s_Scaled )
            {
                PopulateLoreDetails( TRUE );
            }
        }
    }
}

//=========================================================================

void dlg_lore_menu::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVATE )
    {
        if( (!m_bFullScreenMode) && (m_scaleCount == 0) )
        {
            // go full screen
            m_bFullScreenMode = TRUE;
            xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_BACK" ));
            m_pNavText->SetLabel( navText );
            g_AudioMgr.Play("Select_Norm");
            return;
        }
    }

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if( pWin == (ui_win*)m_pLoreSelect )
        {
            // change selected vault
            PopulateLoreDetails( TRUE );
            return;
        }

        // handle lore item selection here!
        if( pWin == (ui_win*)m_pLoreButton[0] )
        {
            if( m_pLoreButton[0]->GetBitmap() == m_LoreIconID[LORE_TYPE_UNKNOWN] )
            {
                g_AudioMgr.Play( "InvalidEntry" );
                return;
            }

            g_AudioMgr.Play("Select_Norm");
            m_pSelectedIcon = m_pLoreButton[0];
            m_SelectedIndex = 0;
            InitIconScaling( FALSE );
            m_State = DIALOG_STATE_ACTIVATE;
        }
        else if( pWin == (ui_win*)m_pLoreButton[1] )
        {
            if( m_pLoreButton[1]->GetBitmap() == m_LoreIconID[LORE_TYPE_UNKNOWN] )
            {
                g_AudioMgr.Play( "InvalidEntry" );
                return;
            }

            g_AudioMgr.Play("Select_Norm");
            m_pSelectedIcon = m_pLoreButton[1];
            m_SelectedIndex = 1;
            InitIconScaling( FALSE );
            m_State = DIALOG_STATE_ACTIVATE;
        }
        else if( pWin == (ui_win*)m_pLoreButton[2] )
        {
            if( m_pLoreButton[2]->GetBitmap() == m_LoreIconID[LORE_TYPE_UNKNOWN] )
            {
                g_AudioMgr.Play( "InvalidEntry" );
                return;
            }

            g_AudioMgr.Play("Select_Norm");
            m_pSelectedIcon = m_pLoreButton[2];
            m_SelectedIndex = 2;
            InitIconScaling( FALSE );
            m_State = DIALOG_STATE_ACTIVATE;
        }
        else if( pWin == (ui_win*)m_pLoreButton[3] )
        {
            if( m_pLoreButton[3]->GetBitmap() == m_LoreIconID[LORE_TYPE_UNKNOWN] )
            {
                g_AudioMgr.Play( "InvalidEntry" );
                return;
            }

            g_AudioMgr.Play("Select_Norm");
            m_pSelectedIcon = m_pLoreButton[3];
            m_SelectedIndex = 3;
            InitIconScaling( FALSE );
            m_State = DIALOG_STATE_ACTIVATE;
        }
        else if( pWin == (ui_win*)m_pLoreButton[4] )
        {
            if( m_pLoreButton[4]->GetBitmap() == m_LoreIconID[LORE_TYPE_UNKNOWN] )
            {
                g_AudioMgr.Play( "InvalidEntry" );
                return;
            }

            g_AudioMgr.Play("Select_Norm");
            m_pSelectedIcon = m_pLoreButton[4];
            m_SelectedIndex = 4;
            InitIconScaling( FALSE );
            m_State = DIALOG_STATE_ACTIVATE;
        }
    }
}

//=========================================================================

void dlg_lore_menu::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    // check if we're scaling an icon
    if( m_scaleCount )
    {
        return;
    }

    switch( m_State )
    {
        case DIALOG_STATE_ACTIVE:
        {
            // exit main dialog
            g_AudioMgr.Play("Backup");
            m_State = DIALOG_STATE_BACK;
        }
        break;

        case DIALOG_STATE_ACTIVATE:
        {
            if( m_bFullScreenMode )
            {
                // exit full screen mode
                m_bFullScreenMode = FALSE;
                xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
                navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
                m_pNavText->SetLabel( navText );
            }
            else
            {
                // exit sub menu / movie player
                InitIconScaling( TRUE );            
            }
            g_AudioMgr.Play("Backup");
        }
        break;
    }
    
}

//=========================================================================

void dlg_lore_menu::OnUpdate ( ui_win* pWin, f32 DeltaTime )
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
            m_pLoreMain      ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pLoreDetails   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pLoreSelect    ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pLoreButton[0] ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pLoreButton[1] ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pLoreButton[2] ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pLoreButton[3] ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pLoreButton[4] ->SetFlag( ui_win::WF_VISIBLE, TRUE );

            GotoControl( (ui_control*)m_pLoreSelect );
            irect Pos = m_pLoreSelect->GetPosition();
            Pos.Translate( 0, -8 );
            g_UiMgr->SetScreenHighlight( Pos );
            //m_CurrHL = 0;

            // activate text
            m_pLoreLine1    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pLoreLine2    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pLoreLine3    ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText      ->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
    }
    else
    {
        // update any icon scaling
        UpdateIconScaling( DeltaTime );
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    // update everything else
    ui_dialog::OnUpdate( pWin, DeltaTime );

    if( !s_Scaled )
    {
        // update highlight
        if( m_pLoreSelect->GetFlags(WF_HIGHLIGHT) )
        {
            highLight = 0;
            irect Pos = m_pLoreSelect->GetPosition();
            Pos.Translate( 0, -8 );
            g_UiMgr->SetScreenHighlight( Pos );
        }
        else if( m_pLoreButton[0]->GetFlags(WF_HIGHLIGHT) )
        {
            highLight = 1;
            g_UiMgr->SetScreenHighlight( m_pLoreMain->GetPosition() );
            m_pSelectedIcon = m_pLoreButton[0];
        }
        else if( m_pLoreButton[1]->GetFlags(WF_HIGHLIGHT) )
        {
            highLight = 2;
            g_UiMgr->SetScreenHighlight( m_pLoreMain->GetPosition() );
            m_pSelectedIcon = m_pLoreButton[1];
        }
        else if( m_pLoreButton[2]->GetFlags(WF_HIGHLIGHT) )
        {
            highLight = 3;
            g_UiMgr->SetScreenHighlight( m_pLoreMain->GetPosition() );
            m_pSelectedIcon = m_pLoreButton[2];
        }
        else if( m_pLoreButton[3]->GetFlags(WF_HIGHLIGHT) )
        {
            highLight = 4;
            g_UiMgr->SetScreenHighlight( m_pLoreMain->GetPosition() );
            m_pSelectedIcon = m_pLoreButton[3];
        }
        else if( m_pLoreButton[4]->GetFlags(WF_HIGHLIGHT) )
        {
            highLight = 5;
            g_UiMgr->SetScreenHighlight( m_pLoreMain->GetPosition() );
            m_pSelectedIcon = m_pLoreButton[4];
        }
    }

    if( highLight != m_CurrHL )
    {
        if( highLight != -1 )
            g_AudioMgr.Play("Cusor_Norm");

        m_CurrHL = highLight;

        if( highLight > 0 )
        {
            PopulateLoreDetails( FALSE );
        }
        else
        {
            PopulateLoreDetails( TRUE );
        }
    }
}

//=========================================================================

void dlg_lore_menu::InitIconScaling ( xbool ScaleDown )
{
    // scaling up or down
    m_bScaleDown  = ScaleDown;

    // store requested frame size
    if( m_bScaleDown )
    {
        m_FadeLevel = 255;
        m_RequestedPos.Set( 0, 0, (s32)(64*m_ScreenScaleX), (s32)(64*m_ScreenScaleY) );
        m_pSelectedIcon->LocalToScreen( m_RequestedPos );         

#ifdef TARGET_PS2
        if( m_bPalMode )
        {
            m_StartPos = irect( 159, 64, (159+194), (64+194) ); // PAL
        }
        else
        {
            m_StartPos = irect( 145, 64, (145+222), (64+194) ); // NTSC
        }
#elif defined TARGET_XBOX
        switch( g_PhysW )
        {
        case 1280: 
        case  720:
            m_StartPos = irect( 215, (s32)(64*m_ScreenScaleY), 215+290, (s32)((64*m_ScreenScaleY)+194) );
            break;
        case  640:
            m_StartPos = irect( 191, (s32)(64*m_ScreenScaleY), 191+258, (s32)((64*m_ScreenScaleY)+194) );
            break;
        default:
            ASSERT(0);
            break;
        }
        extern xbool g_b480P;
        if( g_b480P )
        {
            m_StartPos.t -= 10;
            m_StartPos.b -= 10;
        }
#else
        // 640x480
        m_StartPos = irect( 191, (s32)(64*m_ScreenScaleY), 191+258, (s32)((64*m_ScreenScaleY)+194) );
#endif
        m_DrawPos = m_StartPos;
        m_TimeOut = 0.0f;

        // restart background movie
#if !defined( TARGET_PC )
        Movie.Close();
#endif
        g_StateMgr.EnableBackgroundMovie();

        // turn off textbox (if any)
        m_pTextBox->SetFlag( ui_win::WF_VISIBLE, FALSE );
        m_pTextBox->SetFlag( ui_win::WF_DISABLED, TRUE );
        m_pTextBox->SetFlag( ui_win::WF_SELECTED, FALSE );
        
        g_UiMgr->EnableScreenHighlight();
        // turn on lore details
        m_pLoreDetails ->SetFlag(ui_win::WF_VISIBLE, TRUE);
        m_pLoreLine1   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
        m_pLoreLine2   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
        m_pLoreLine3   ->SetFlag(ui_win::WF_VISIBLE, TRUE);

        // goto previous control
        GotoControl( (ui_control*)m_pSelectedIcon );
        s_Scaled = FALSE;
    }
    else
    {
        s_Scaled = TRUE;

        m_FadeLevel = 0;
#ifdef TARGET_PS2
        if( m_bPalMode )
        {
            m_RequestedPos = irect( 159, 64, (159+194), (64+194) ); // PAL
        }
        else
        {
            m_RequestedPos = irect( 145, 64, (145+222), (64+194) ); // NTSC
        }
#elif defined TARGET_XBOX
        switch( g_PhysW )
        {
            case 1280: 
            case  720:
                m_RequestedPos = irect( 215, (s32)(64*m_ScreenScaleY), 215+290, (s32)((64*m_ScreenScaleY)+194) );
                break;
            case  640:
                m_RequestedPos = irect( 191, (s32)(64*m_ScreenScaleY), 191+258, (s32)((64*m_ScreenScaleY)+194) );
                break;
            default:
                ASSERT(0);
                break;
        }
        extern xbool g_b480P;
        if( g_b480P )
        {
            m_RequestedPos.t -= 10;
            m_RequestedPos.b -= 10;
        }
#else
        // 640x480
        m_RequestedPos = irect( 191, (s32)(64*m_ScreenScaleY), 191+258, (s32)((64*m_ScreenScaleY)+194) );
#endif

        m_StartPos.Set( 0, 0, (s32)(64*m_ScreenScaleX), (s32)(64*m_ScreenScaleY) );
        m_pSelectedIcon->LocalToScreen( m_StartPos );
        m_DrawPos = m_StartPos;
        m_pSelectedIcon->SetFlag( ui_win::WF_VISIBLE, FALSE );
        // disable the highlight
        g_UiMgr->DisableScreenHighlight();
    }

    
    // set up scaling
    m_scaleCount = 0.3f; // time to scale in seconds
    m_scaleAngle = 180.0f / m_scaleCount;

    m_DiffPos.t = (s32)((m_RequestedPos.t - m_DrawPos.t) / 2.0f);
    m_DiffPos.l = (s32)((m_RequestedPos.l - m_DrawPos.l) / 2.0f);
    m_DiffPos.b = (s32)((m_RequestedPos.b - m_DrawPos.b) / 2.0f);
    m_DiffPos.r = (s32)((m_RequestedPos.r - m_DrawPos.r) / 2.0f);

    m_TotalMoved.Set( 0, 0, 0, 0 );
    
    // turn screen on
    m_bScreenIsOn = TRUE;
    m_pBlackOut->SetFlag(ui_win::WF_VISIBLE, TRUE);
    m_pBlackOut->SetBackgroundColor( xcolor (0,0,0,0) );

    // play scaling sound
    if( m_DiffPos.b > 0 )
    {
        g_AudioMgr.Play( "ResizeLarge" ); 
    }
    else
    {
        g_AudioMgr.Play( "ResizeSmall" );
    }
}

//=========================================================================

xbool dlg_lore_menu::UpdateIconScaling( f32 DeltaTime )
{
    // scale window if necessary
    if (m_scaleCount)
    {
        // apply delta time
        m_scaleCount -= DeltaTime;

        if (m_scaleCount <= 0)
        {
            // last one - make sure window is correct size
            m_scaleCount = 0;
            m_DrawPos = m_RequestedPos;

            if( m_bScaleDown )
            {
                m_FadeLevel = 0;
                m_bScreenIsOn = FALSE;
                m_pBlackOut->SetFlag(ui_win::WF_VISIBLE, FALSE);
                m_State = DIALOG_STATE_ACTIVE;
                m_pSelectedIcon->SetFlag( ui_win::WF_VISIBLE, TRUE );
            }
            else
            {
                m_FadeLevel = 205;
                m_pBlackOut->SetBackgroundColor( xcolor (0,0,0,m_FadeLevel) );

                // get the lore description
                const lore_entry* Entry = g_LoreList.Find( m_pSelectedIcon->GetData() );
                ASSERT( Entry );

                // what type of lore item do we have
                switch( Entry->LoreType )
                {
                    case LORE_TYPE_VIDEO:
                    case LORE_TYPE_AUDIO:
                        // set filename
                        x_strcpy( m_FileName, Entry->FileName );
                        // set timeout
                        m_TimeOut = 0.5f;
                        break;
                    case LORE_TYPE_STILL:
                    case LORE_TYPE_TEXT:
                        // set filename for still
                        g_UiMgr->UnloadBitmap( "Still" );
                        if( Entry->NumItems > 1 )
                        {
                            m_NumItems = Entry->NumItems;
                            m_CurrItem = 0;
                            x_strcpy( m_FileName, Entry->FileName );
                            m_StillBitmapID = g_UiMgr->LoadBitmap( "Still", xfs( "%s%d.xbmp", m_FileName, m_CurrItem+1 ) );
                        }
                        else
                        {
                            m_NumItems = 1;
                            m_CurrItem = 0;
                            x_strcpy( m_FileName, Entry->FileName );    
#ifdef TARGET_PS2
                            m_StillBitmapID = g_UiMgr->LoadBitmap( "Still", xfs( "%s_PS2.xbmp", m_FileName ) );
#elif defined TARGET_XBOX
                            m_StillBitmapID = g_UiMgr->LoadBitmap( "Still", xfs( "%s_XBOX.xbmp", m_FileName ) );
#else
                            m_StillBitmapID = g_UiMgr->LoadBitmap( "Still", xfs( "%s.xbmp", m_FileName ) );
#endif
                        }

                        // load the correct bitmap
                        //m_bCycleBitmap = TRUE;

                        // turn off lore details
                        m_pLoreDetails ->SetFlag(ui_win::WF_VISIBLE, FALSE);
                        m_pLoreLine1   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
                        m_pLoreLine2   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
                        m_pLoreLine3   ->SetFlag(ui_win::WF_VISIBLE, FALSE);

                        // make text box visible and fill with text
                        m_pTextBox->SetFlag( ui_win::WF_VISIBLE, TRUE );
                        m_pTextBox->SetFlag( ui_win::WF_DISABLED, FALSE );
                        m_pTextBox->SetFlag( ui_win::WF_SELECTED, TRUE );
                        x_strcpy( m_FullDesc, Entry->FullDesc );
                        if( Entry->NumItems > 1 )
                        {
                            m_pTextBox->SetLabel( g_StringTableMgr( "lore", xfs( "%s_%d", m_FullDesc, m_CurrItem+1 ) ) );
                        }
                        else
                        {
                            m_pTextBox->SetLabel( g_StringTableMgr( "lore", m_FullDesc ) );
                        }
                        GotoControl( (ui_control*)m_pTextBox );
                        break;
                }
            }
        }
        else
        {
            m_TotalMoved.t = m_DiffPos.t + (s32)( m_DiffPos.t * x_cos( DEG_TO_RAD( m_scaleAngle * m_scaleCount ) ) );
            m_TotalMoved.l = m_DiffPos.l + (s32)( m_DiffPos.l * x_cos( DEG_TO_RAD( m_scaleAngle * m_scaleCount ) ) );
            m_TotalMoved.r = m_DiffPos.r + (s32)( m_DiffPos.r * x_cos( DEG_TO_RAD( m_scaleAngle * m_scaleCount ) ) );
            m_TotalMoved.b = m_DiffPos.b + (s32)( m_DiffPos.b * x_cos( DEG_TO_RAD( m_scaleAngle * m_scaleCount ) ) );

            m_DrawPos.t = m_StartPos.t + m_TotalMoved.t;
            m_DrawPos.l = m_StartPos.l + m_TotalMoved.l;
            m_DrawPos.r = m_StartPos.r + m_TotalMoved.r;
            m_DrawPos.b = m_StartPos.b + m_TotalMoved.b;

            // update fade level
            if( m_bScaleDown)
            {
                m_FadeLevel -= (u8)( 700.0f * DeltaTime );
            }
            else
            {
                m_FadeLevel += (u8)( 700.0f * DeltaTime );
            }

            m_pBlackOut->SetBackgroundColor( xcolor (0,0,0,m_FadeLevel) );

            // still more to do!
            return TRUE;
        }
    }
    else
    {
        // update movie timeout
        if( m_TimeOut )
        {
            m_TimeOut -= DeltaTime;

            if (m_TimeOut <= 0)
            {
                // shut down background movie
                g_StateMgr.DisableBackgoundMovie();
                // open lore movie (or sound file, or text)
#if !defined( TARGET_PC )
                Movie.Open(m_FileName, FALSE, TRUE);
#endif
                // reset timeout
                m_TimeOut = 0;
            }
        }
    }

    // we're done!
    return FALSE;
}
//=========================================================================

void dlg_lore_menu::PopulateLoreDetails( xbool bVaultDetails )
{
    if( bVaultDetails )
    {
        player_profile& Profile = g_StateMgr.GetActiveProfile( 0 );
        map_entry* pEntry = (map_entry*)m_pLoreSelect->GetSelectedItemData();

        lore_vault* m_pSelectedVault = g_LoreList.GetVaultByMapID( pEntry->GetMapID(), m_VaultIndex );

        u32 Count = 0;
        for( s32 i=0; i<NUM_PER_VAULT; i++ )
        {
#if defined (mbillington) || (jhowa) || (shird)
            if( 1 )
#else
            if( Profile.GetLoreAcquired( m_VaultIndex, i ) )
#endif
            {
                // set the bitmap based on the lore type            
                m_pLoreButton[i]->SetBitmap( m_LoreIconID[ g_LoreList.GetType( m_pSelectedVault->LoreID[i] ) ] );
                m_pLoreButton[i]->SetData( m_pSelectedVault->LoreID[i] );
                Count++;
            }
            else
            {
                m_pLoreButton[i]->SetBitmap( m_LoreIconID[LORE_TYPE_UNKNOWN] );
                m_pLoreButton[i]->SetData( m_pSelectedVault->LoreID[i] );
            }
        }

        m_pLoreLine1->SetLabel( pEntry->GetDisplayName() );
        m_pLoreLine2->SetLabel( xwstring(xfs("%s   : %d/%d", (const char*)xstring(g_StringTableMgr("ui", "IDS_LEVEL")), Count, NUM_PER_VAULT)) );
        m_pLoreLine3->SetLabel( xwstring(xfs("%s : %d/90", (const char*)xstring(g_StringTableMgr("ui", "IDS_OVERALL")), Profile.GetTotalLoreAcquired())) );

        //xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_CYCLE_VAULT" ));
        //navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
        xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
        navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
        m_pNavText->SetLabel( navText );
    }
    else
    {
        // get the lore info
        const lore_entry* pLoreEntry = g_LoreList.Find( m_pSelectedIcon->GetData() );
        ASSERT( pLoreEntry );

        // update details text based on selected icon
        const xwchar* pLoreText;

#if defined (mbillington) || (jhowa) || (shird)
        if( 1 )
#else
        // get the active profile
        player_profile& Profile = g_StateMgr.GetActiveProfile( 0 );

        if( Profile.GetLoreAcquired( m_VaultIndex, m_CurrHL-1 ) )
#endif
        {
            pLoreText = g_StringTableMgr( "lore", pLoreEntry->ShortDesc );
        }
        else
        {
            pLoreText = g_StringTableMgr( "lore", pLoreEntry->Clue );
        }

        ui_font* pFont      = g_UiMgr->GetFont( "small" );
        xwstring Wrapped;
        pFont->TextWrap( pLoreText, m_pLoreLine1->GetPosition(), Wrapped );
        m_pLoreLine1->SetLabel( Wrapped );
        m_pLoreLine2->SetLabel( g_StringTableMgr( "ui", "IDS_NULL" ) );
        m_pLoreLine3->SetLabel( g_StringTableMgr( "ui", "IDS_NULL" ) );

        m_CurrentType = pLoreEntry->LoreType;

        xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
        navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
        m_pNavText->SetLabel( navText );
    }
}

//=========================================================================
