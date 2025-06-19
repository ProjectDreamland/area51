//=========================================================================
//
//  dlg_online_level_select.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_listbox.hpp"

#include "dlg_PopUp.hpp"
#include "dlg_OnlineLevelSelect.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Parsing/textin.hpp"
#include "StateMgr/mapList.hpp"
#include "Configuration/GameConfig.hpp"
#ifdef CONFIG_VIEWER
#include "../../Apps/ArtistViewer/Config.hpp"
#else
#include "../../Apps/GameApp/Config.hpp"	
#endif
#include "e_Memcard.hpp"
#include "MemCardMgr/MemCardMgr.hpp"
#include "dlg_download.hpp"

#ifndef CONFIG_RETAIL
#include "InputMgr\monkey.hpp"
#endif


//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum controls
{
	IDC_ONLINE_LEVEL_SELECT_LISTBOX,
    IDC_ONLINE_LEVEL_SELECT_CYCLE,
    IDC_ONLINE_LEVEL_SELECT_LAUNCH,
    IDC_ONLINE_LEVEL_SELECT_NAV_TEXT,
};

enum popup_message
{
    POPUP_EMPTY_LIST,
    POPUP_RECONFIGURE,
    POPUP_SAVE_SETTINGS,
};

ui_manager::control_tem OnlineLevelSelectControls[] = 
{
    // Frames.
    { IDC_ONLINE_LEVEL_SELECT_LISTBOX,   "IDS_MAP_NAME",    "listbox",   40,  40, 180, 222,  0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_LEVEL_SELECT_CYCLE,     "IDS_MAP_CYCLE",   "listbox",  240,  40, 180, 222,  1, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_LEVEL_SELECT_LAUNCH,    "IDS_LAUNCH",      "button",    40, 285, 220,  40,  0, 1, 2, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_ONLINE_LEVEL_SELECT_NAV_TEXT,  "IDS_NULL",        "text",       0,   0,   0,   0,  0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};

ui_manager::dialog_tem OnlineLevelSelectDialog =
{
    "IDS_ONLINE_LEVEL_SELECT",
    2, 9,
    sizeof(OnlineLevelSelectControls)/sizeof(ui_manager::control_tem),
    &OnlineLevelSelectControls[0],
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

void dlg_online_level_select_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "online level select", &OnlineLevelSelectDialog, &dlg_online_level_select_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_online_level_select_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_online_level_select* pDialog = new dlg_online_level_select;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_level_select
//=========================================================================

dlg_online_level_select::dlg_online_level_select( void )
{
}

//=========================================================================

dlg_online_level_select::~dlg_online_level_select( void )
{
}

//=========================================================================

xbool dlg_online_level_select::Create( s32                        UserID,
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
    m_pLevelList     = (ui_listbox*)    FindChildByID( IDC_ONLINE_LEVEL_SELECT_LISTBOX  );
    m_pLevelCycle    = (ui_listbox*)    FindChildByID( IDC_ONLINE_LEVEL_SELECT_CYCLE    );
    m_pLaunchButton  = (ui_button*)     FindChildByID( IDC_ONLINE_LEVEL_SELECT_LAUNCH   );
    m_pNavText       = (ui_text*)       FindChildByID( IDC_ONLINE_LEVEL_SELECT_NAV_TEXT );

    // initialize type
    m_Type = MAP_SELECT_ONLINE;

    // get the map list from the game settings
    host_settings& Settings = g_StateMgr.GetPendingSettings().GetHostSettings();
    map_settings* pMapSettings = &Settings.m_MapSettings;
    
    // use default map cycle?
    if( pMapSettings->m_bUseDefault )
    {
        // clear the cycle
        g_StateMgr.ClearMapCycle();

        game_type GameType = g_PendingConfig.GetGameTypeID();

        for( s32 i=0; i <g_MapList.GetCount(); i++ )
        {
            const map_entry& Entry = *g_MapList.GetByIndex( i );

            // check if of the correct game type
            if( Entry.GetGameType() == GameType )
            {
                // check for player limit
                if( Entry.GetMaxPlayers() >= Settings.m_MaxPlayers )
                {
                    // check if loaded
                    if( Entry.IsAvailable() )
                    {
                        // store the map ID in the map cycle
                        g_StateMgr.InsertMapInCycle( Entry.GetMapID() );
                    }
                }
            }
        }

        // set default cycle flag
        g_StateMgr.SetUseDefaultMapCycle(TRUE);
    }
    else
    {
        // clear the cycle
        g_StateMgr.ClearMapCycle();

        // copy the map cycle from the game settings
        for( s32 i=0; i< pMapSettings->m_MapCycleCount; i++ )
        {
            // insert the map ID
            g_StateMgr.InsertMapInCycle( pMapSettings->m_MapCycle[i] );
        }

        // set the cycle index
        g_StateMgr.SetMapCycleIndex( pMapSettings->m_MapCycleIdx );
        // flag that we modified the list
        g_StateMgr.SetUseDefaultMapCycle( FALSE );
    }


    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
    navText += g_StringTableMgr( "ui", "IDS_NAV_ROTATE_CYCLE" );
    navText += g_StringTableMgr( "ui", "IDS_NAV_LAUNCH" );
    m_pNavText->SetLabel( navText );
    m_pNavText->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);    

    // set up level list
    FillLevelList();
    if( m_pLevelList->GetItemCount() )
    {
        m_pLevelList->SetSelection( 0 );
    }
    //m_pLevelList->SetFlag(ui_win::WF_SELECTED, TRUE);
    m_pLevelList->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLevelList->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pLevelList->DisableFrame();
    m_pLevelList->SetExitOnSelect(FALSE);
    m_pLevelList->SetExitOnBack(TRUE);
    m_pLevelList->EnableHeaderBar();
    m_pLevelList->SetHeaderBarColor( xcolor(19,59,14,196) );
    m_pLevelList->SetHeaderColor( xcolor(255,252,204,255) );
    m_pLevelList->EnableParentNavigation();

    // set up level cycle
    FillMapCycleList();
    m_pLevelCycle->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLevelCycle->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pLevelCycle->DisableFrame();
    m_pLevelCycle->SetExitOnSelect(FALSE);
    m_pLevelCycle->SetExitOnBack(TRUE);
    m_pLevelCycle->EnableHeaderBar();
    m_pLevelCycle->SetHeaderBarColor( xcolor(19,59,14,196) );
    m_pLevelCycle->SetHeaderColor( xcolor(255,252,204,255) );
    m_pLevelCycle->EnableParentNavigation();

    // set up launch button
    m_pLaunchButton->SetFlag( ui_win::WF_VISIBLE, FALSE );
    m_pLaunchButton->SetFlag( ui_win::WF_BUTTON_LEFT, TRUE );

    // set initial focus
    m_CurrHL = 2;
    GotoControl( (ui_control*)m_pLaunchButton );
    g_UiMgr->SetScreenHighlight( m_pLaunchButton->GetPosition() );
    m_PopUp = NULL;
    
    // clear controls
    m_bRenderBlackout = FALSE;
    m_bInGame         = FALSE;

    m_ManifestCount[0] = -1;
    m_ManifestCount[1] = -1;

    if( !(CONFIG_IS_AUTOSERVER || CONFIG_IS_AUTOCLIENT) )
    {
        // clear in-use controller flags for non-splitscreen game
        if( (g_StateMgr.GetState() != SM_MP_LEVEL_SELECT) &&
            (g_StateMgr.IsPaused() == FALSE) )
        {
            for( int i=0; i<MAX_LOCAL_PLAYERS; i++)
            {
                g_StateMgr.SetControllerRequested(i, FALSE);
            }
        }
    }

    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

	// Return success code
    return Success;
}

//=========================================================================

void dlg_online_level_select::Destroy( void )
{
    ui_dialog::Destroy();

    g_UIMemCardMgr.ClearCallback();
    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_online_level_select::Configure( map_select_type Type)
{
    m_Type = Type;

    switch( Type )
    {
        case MAP_SELECT_ONLINE_PAUSE:
        {
            // set up for reconfiguring online server
            m_bInGame = TRUE;
            m_pLaunchButton->SetLabel( g_StringTableMgr( "ui", "IDS_HOST_CONTINUE" ) );
        }
        break;

        case MAP_SELECT_MP:
        {
            // get the map list from the game settings
            multi_settings& Settings = g_StateMgr.GetPendingSettings().GetMultiplayerSettings();
            map_settings* pMapSettings = &Settings.m_MapSettings;

            // use default map cycle?
            if( pMapSettings->m_bUseDefault )
            {
                // clear the cycle
                g_StateMgr.ClearMapCycle();

                game_type GameType = g_PendingConfig.GetGameTypeID();

                for( s32 i=0; i <g_MapList.GetCount(); i++ )
                {
                    const map_entry& Entry = *g_MapList.GetByIndex( i );
                    // check if of the correct game type
                    if( Entry.GetGameType() == GameType )
                    {
                        // check if loaded
                        if( Entry.IsAvailable() )
                        {
                            // store the map ID in the map cycle
                            g_StateMgr.InsertMapInCycle( Entry.GetMapID() );
                        }
                    }
                }

                // set the cycle index
                g_StateMgr.SetMapCycleIndex( pMapSettings->m_MapCycleIdx );
                // set default cycle flag
                g_StateMgr.SetUseDefaultMapCycle(TRUE);
            }
            else
            {
                // clear the cycle
                g_StateMgr.ClearMapCycle();

                // copy the map cycle from the game settings
                for( s32 i=0; i< pMapSettings->m_MapCycleCount; i++ )
                {
                    // insert the map ID
                    g_StateMgr.InsertMapInCycle( pMapSettings->m_MapCycle[i] );
                }

                // flag that we modified the list
                g_StateMgr.SetUseDefaultMapCycle( FALSE );
            }

            // refresh the map cycle list
            FillMapCycleList();
        }
        break;
    }
}

//=========================================================================

void dlg_online_level_select::Render( s32 ox, s32 oy )
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

void dlg_online_level_select::OnNotify ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData )
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
            if ( pSender == (ui_win*)m_pLevelList )
            {
                // level selection listbox

                // add the selected map to the cycle
                if( g_StateMgr.GetMapCycleCount() < MAP_CYCLE_SIZE )
                {
                    // insert the map ID
                    map_entry* pEntry = (map_entry*)m_pLevelList->GetSelectedItemData();
                    g_StateMgr.InsertMapInCycle( pEntry->GetMapID() );

                    // flag that we modified the list
                    g_StateMgr.SetUseDefaultMapCycle( FALSE );

                    // refresh the map cycle list
                    FillMapCycleList();
                }
                else
                {
                    // play error sound
                    // pop up error dialog?
                }
            }
            else
            {
                // level cycle listbox

                // check we have something to delete
                if( g_StateMgr.GetMapCycleCount() > 0 )
                {
                    // store current selection
                    s32 Sel = m_pLevelCycle->GetSelection();

                    // remove the selected map from the cycle
                    g_StateMgr.DeleteMapFromCycle( Sel );

                    // flag that we modified the list
                    g_StateMgr.SetUseDefaultMapCycle( FALSE );

                    // refresh the map cycle list
                    FillMapCycleList();

                    // set the cursor position
                    s32 Count = m_pLevelCycle->GetItemCount();
                    if( Count > 0 )
                    {
                        if( Sel > 0 )
                        {
                            if ( Sel < Count )
                            {
                                m_pLevelCycle->SetSelection( Sel );
                            }
                            else
                            {
                                m_pLevelCycle->SetSelection( Count-1 );
                            }
                        }
                    }
                }
                else
                {
                    // play error sound
                    // pop-up error dialog?
                }
            }
        }
    }
}

//=========================================================================

void dlg_online_level_select::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
}

//=========================================================================

void dlg_online_level_select::OnPadSelect( ui_win* pWin )
{
    if( m_State == DIALOG_STATE_ACTIVE )
    {
#ifndef CONFIG_RETAIL
        if ( g_MonkeyOptions.Enabled && g_MonkeyOptions.ModeEnabled[MONKEY_MENUMONKEY] )
            return;
#endif

        if( pWin == (ui_win*)m_pLaunchButton )
        {
            LaunchServer();
        }
    }
}

//=========================================================================

void dlg_online_level_select::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        g_AudioMgr.Play("Backup");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_online_level_select::OnPadActivate( ui_win* pWin )
{
    (void)pWin;

    // check for <2 items
    if( m_pLevelCycle->GetItemCount() < 2 )
    {
        // nothing to change
        g_AudioMgr.Play( "InvalidEntry" );
        return;
    }

    // rotate the map cycle
    g_StateMgr.IncrementMapCycle();
    FillMapCycleList();
}

//=========================================================================

void dlg_online_level_select::OnPadDelete( ui_win* pWin )
{
    (void)pWin;

    if( m_State == DIALOG_STATE_ACTIVE )
    {
#ifndef CONFIG_RETAIL
        if ( g_MonkeyOptions.Enabled && g_MonkeyOptions.ModeEnabled[MONKEY_MENUMONKEY] )
            return;
#endif

        LaunchServer();
    }
}

//=========================================================================

void dlg_online_level_select::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // turn on the controls
            m_pLevelList    ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pLevelCycle   ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pLaunchButton ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            m_pNavText      ->SetFlag( ui_win::WF_VISIBLE, TRUE );
            
            GotoControl( (ui_control*)m_pLaunchButton );
            g_UiMgr->SetScreenHighlight( m_pLaunchButton->GetPosition() );
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

            switch( m_PopUpType )
            {
                case POPUP_EMPTY_LIST:
                {
                    // empty cycle handled!

                    // turn on nav text
                    m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
                }
                break;

                case POPUP_RECONFIGURE:
                {
                    switch( m_PopUpResult ) 
                    {
                        case DLG_POPUP_YES:
                            // apply changes NOW
                            m_State = DIALOG_STATE_SELECT;
                            break;

                        case DLG_POPUP_NO:
                            // cancel
                            break;

                        case DLG_POPUP_MAYBE:
                            // apply changes AFTER current map 
                            g_StateMgr.ActivatePendingSettings( TRUE ); // mark dirty
                            m_State = DIALOG_STATE_ACTIVATE;

#if 0 
                            // MAB: Don't save to the memory card during online session!
                            // check if the settings are saved 
                            if( g_StateMgr.GetSettingsCardSlot() == -1 )
                            {
#ifdef TARGET_PS2
                                // not saved - goto memory card select screen
                                m_State = DIALOG_STATE_MEMCARD_ERROR;
#else
                                // attempt to create settings
                                g_StateMgr.SetSettingsCardSlot( 0 );
                                g_UIMemCardMgr.CreateSettings( this, &dlg_online_level_select::OnSaveSettingsCB );
                                m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
#endif
                            }
                            else
                            {                            
                                // attempt to save the changes to the memcard
                                g_UIMemCardMgr.SaveSettings( this, &dlg_online_level_select::OnSaveSettingsCB );
                                m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
                            }
#endif
                            break;
                    }                    
                }
                break;

                case POPUP_SAVE_SETTINGS:
                {
                    switch( m_PopUpResult )
                    {
                        case DLG_POPUP_YES:
                        {
                            // save changes
                            g_AudioMgr.Play("Select_Norm");

                            // check if the settings are saved 
                            if( g_StateMgr.GetSettingsCardSlot() == -1 )
                            {
#ifdef TARGET_PS2
                                // not saved - goto memory card select screen
                                m_State = DIALOG_STATE_MEMCARD_ERROR;
#else
                                // attempt to create settings
                                g_StateMgr.SetSettingsCardSlot( 0 );
                                g_UIMemCardMgr.CreateSettings( this, &dlg_online_level_select::OnSaveSettingsCB );
                                m_State = DIALOG_STATE_WAIT_FOR_MEMCARD;
#endif
                            }
                            else
                            {                            
                                // attempt to save the changes to the memcard
                                g_UIMemCardMgr.SaveSettings( this, &dlg_online_level_select::OnSaveSettingsCB );
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
                break;
            }
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    s32 Highlight;

    if (m_pLevelList->GetFlags() & ui_win::WF_SELECTED)
    {
        Highlight = 0;
        g_UiMgr->SetScreenHighlight( m_pLevelList->GetPosition() );
    }
    else if (m_pLevelCycle->GetFlags() & ui_win::WF_SELECTED)
    {
        Highlight = 1;
        g_UiMgr->SetScreenHighlight( m_pLevelList->GetPosition() );
    }
    else 
    {
        Highlight = 2;
        g_UiMgr->SetScreenHighlight( m_pLaunchButton->GetPosition() );
    }

    if ( m_CurrHL != Highlight )
    {
        switch( Highlight )
        {
            case 0:
            {
                xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_ADD_MAP" ));
                navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
                navText += g_StringTableMgr( "ui", "IDS_NAV_ROTATE_CYCLE" );
                navText += g_StringTableMgr( "ui", "IDS_NAV_LAUNCH" );
                m_pNavText->SetLabel( navText );

                // set list cursor
                s32 selection = m_pLevelCycle->GetSelection();
                s32 count = m_pLevelList->GetItemCount();

                if ( count )
                {
                    count--;

                    if ( selection > count )
                    {
                        m_pLevelList->SetSelection( count );
                    }
                    else if ( selection < 0 )
                    {
                        m_pLevelList->SetSelection( 0 );
                    }
                    else
                    {
                        m_pLevelList->SetSelection( selection );
                    }
                }
            }
            break;

            case 1:
            {
                xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_DELETE_MAP" ));
                navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
                navText += g_StringTableMgr( "ui", "IDS_NAV_ROTATE_CYCLE" );
                navText += g_StringTableMgr( "ui", "IDS_NAV_LAUNCH" );
                m_pNavText->SetLabel( navText );

                // set list cursor
                s32 selection = m_pLevelList->GetSelection();
                s32 count = m_pLevelCycle->GetItemCount();

                if ( count )
                {
                    count--;

                    if ( selection > count )
                    {
                        m_pLevelCycle->SetSelection( count );
                    }
                    else if ( selection < 0 ) 
                    {
                        m_pLevelCycle->SetSelection( 0 );
                    }
                    else
                    {
                        m_pLevelCycle->SetSelection( selection );
                    }
                }
            }
            break;

            default:
            {
                xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
                navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
                navText += g_StringTableMgr( "ui", "IDS_NAV_ROTATE_CYCLE" );
                navText += g_StringTableMgr( "ui", "IDS_NAV_LAUNCH" );
                m_pNavText->SetLabel( navText );
            }
            break;
        }

        m_CurrHL = Highlight;
    }
}

//=========================================================================

void dlg_online_level_select::FillLevelList( void )
{
    s32            iSel     = m_pLevelList->GetSelection();
    host_settings& Settings = g_StateMgr.GetPendingSettings().GetHostSettings();
    
    // Clear listbox
    m_pLevelList->DeleteAllItems();

    for( s32 i=0; i<g_MapList.GetCount(); i++ )
    {
        const map_entry& Entry = *g_MapList.GetByIndex( i );

        // check if of the correct game type
        if( Entry.GetGameType() == g_PendingConfig.GetGameTypeID() )
        {
            if( Entry.GetMaxPlayers() >= Settings.m_MaxPlayers )
            {
                // check if loaded
                if( Entry.IsAvailable() )
                {
                    // add an entry to the list
                    m_pLevelList->AddItem( Entry.GetDisplayName(), (s32)&Entry );
                }
            }
        }
    }

    // Limit Selection
    if( iSel <  0 ) iSel = 0;
    if( iSel >= m_pLevelList->GetItemCount() ) iSel = m_pLevelList->GetItemCount()-1;

    // Set Selection
    m_pLevelList->SetSelection( iSel );
}

//=========================================================================

void dlg_online_level_select::FillMapCycleList( void )
{
    // Clear listbox
    m_pLevelCycle->DeleteAllItems();

    // populate the listbox using the map cycle data
    s32 Count = g_StateMgr.GetMapCycleCount();
    s32 Total = Count;
    s32 Index = g_StateMgr.GetMapCycleIndex();

    // loop through map cycle filling the list
    while( Count > 0 )
    {
        // get the map ID for this entry
        s32 MapID = g_StateMgr.GetMapCycleMapID( Index );

        const map_entry* pMapEntry = g_MapList.Find( MapID );
        // **NOTE** map entry may be null as the user downloaded maps may not yet be
        // available if the memory card has not completed scanning.
        if( pMapEntry )
        {
            // get the display name from the map ID and add to the list
            m_pLevelCycle->AddItem( pMapEntry->GetDisplayName(), m_pLevelList->GetSelection() );
        }

        // get the next entry index
        Index++;

        // check for wrapping
        if( Index >= Total )
            Index = 0;

        // Decrement count
        Count--;
    }

    // set selection
    if( m_pLevelCycle->GetItemCount() > 0 )
    {
        m_pLevelCycle->SetSelection( 0 );
    }
}

//=========================================================================

void dlg_online_level_select::OnSaveSettingsCB( void )
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

        // save successful - return to main menu
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

map_settings& dlg_online_level_select::GetMapSettings( void )
{
    if( m_Type == MAP_SELECT_MP )
    {
        return g_StateMgr.GetPendingSettings().GetMultiplayerSettings().m_MapSettings;
    }
    else
    {
        return g_StateMgr.GetPendingSettings().GetHostSettings().m_MapSettings;
    }
}

//=========================================================================

void dlg_online_level_select::LaunchServer( void )
{
    // attempt to launch the server

    // check for empty map cycle
    if( g_StateMgr.GetMapCycleCount() == 0 )
    {
        irect r = g_UiMgr->GetUserBounds( g_UiUserID );
        m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

        // set nav text
        xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_OK" ));
        m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

        // configure message
        m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_MAP_CYCLE" ), 
            TRUE, 
            FALSE, 
            FALSE, 
            g_StringTableMgr( "ui", "IDS_EMPTY_CYCLE_MSG" ),
            navText,
            &m_PopUpResult );

        m_PopUpType = POPUP_EMPTY_LIST;
        return;
    }

    if( m_bInGame )
    {
        // confirm reconfigure server
        irect r = g_UiMgr->GetUserBounds( g_UiUserID );
        m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

        // set nav text
        xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_APPLY_NOW" ));
        navText += g_StringTableMgr( "ui", "IDS_NAV_APPLY_AFTER" );
        navText += g_StringTableMgr( "ui", "IDS_NAV_CANCEL" );
        m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

        irect ApplyRect;
        ApplyRect.Set(0,0,280,240);

        // configure message
        m_PopUp->Configure( ApplyRect, g_StringTableMgr( "ui", "IDS_RECONFIGURE_POPUP" ), 
            TRUE, 
            TRUE, 
            TRUE, 
            g_StringTableMgr( "ui", "IDS_CONFIRM_RECONFIG_MSG" ),
            navText,
            &m_PopUpResult );

        m_PopUpType = POPUP_RECONFIGURE;
        g_PendingConfig.SetLevelID( g_StateMgr.GetMapCycleMapID( g_StateMgr.GetMapCycleIndex() ) );
        return;
    }
    else
    {
        map_settings& MapSettings = GetMapSettings();
        s32 i;

        // Copy the map settings from the statemgr in to the pending settings.
        //
        MapSettings.m_bUseDefault   = g_StateMgr.GetUseDefaultMapCycle();

        if( MapSettings.m_bUseDefault )
        {
            x_memset( MapSettings.m_MapCycle, -1, sizeof(MapSettings.m_MapCycle) );
            MapSettings.m_MapCycleIdx = 0;
            MapSettings.m_MapCycleIdx = 0;
        }
        else
        {
            MapSettings.m_MapCycleIdx = g_StateMgr.GetMapCycleIndex();
            MapSettings.m_MapCycleCount = g_StateMgr.GetMapCycleCount();
            for( i=0; i<MapSettings.m_MapCycleCount; i++ )
            {
                MapSettings.m_MapCycle[i] = g_StateMgr.GetMapCycleMapID( i );
            }
        }

        // set the map id - first item in the map cycle
        s32 MapCycleIndex = g_StateMgr.GetMapCycleIndex();
        s32 InitialMapCycleIndex = g_StateMgr.GetMapCycleIndex();
        while( TRUE )
        {
            if( g_MapList.Find( g_StateMgr.GetMapCycleMapID(MapCycleIndex) ) )
            {
                break;
            }
            MapCycleIndex++;
            if( MapCycleIndex >= g_StateMgr.GetMapCycleCount() )
            {
                MapCycleIndex = 0;
                if( MapCycleIndex == InitialMapCycleIndex )
                {
                    // This means we have no visible maps.
                    ASSERT( FALSE );
                }
            }
        }
        g_PendingConfig.SetLevelID( g_StateMgr.GetMapCycleMapID( MapCycleIndex ) );

        // Set the level name
        s32 Index = 0;
        map_entry* pEntry = (map_entry*)m_pLevelList->GetItemData( 0, 0 );

        // We need to translate from our index to the index within the maplist.
        while( TRUE )
        {
            if( g_MapList.GetByIndex( Index ) == pEntry )
            {
                break;
            }
            Index++;
        }
        g_StateMgr.SetLevelIndex( Index );

        // store the active controller (if NOT multiplayer)
        if( g_StateMgr.GetState() != SM_MP_LEVEL_SELECT )
        {
            g_StateMgr.SetActiveControllerID( g_UiMgr->GetActiveController() );
            g_StateMgr.SetControllerRequested( g_UiMgr->GetActiveController(), TRUE );
        }

        if( g_StateMgr.GetPendingSettings().HasChanged() )
        {
            // settings have changed - save changes?
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
                g_StringTableMgr( "ui", "IDS_SAVE_SETTINGS_MSG" ),
                navText,
                &m_PopUpResult );

            m_PopUpType = POPUP_SAVE_SETTINGS;
            return;
        }
        else
        {
            m_State = DIALOG_STATE_ACTIVATE;            
        }
    }
}

//=========================================================================
