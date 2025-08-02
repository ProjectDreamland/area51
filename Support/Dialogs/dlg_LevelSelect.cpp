//=========================================================================
//
//  dlg_level_select.cpp
//
//=========================================================================

#ifndef CONFIG_RETAIL

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_listbox.hpp"

#include "dlg_LevelSelect.hpp"
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Parsing/textin.hpp"
#include "StateMgr/mapList.hpp"
#include "NetworkMgr/GameMgr.hpp"
#include "Configuration/GameConfig.hpp"

#ifdef CONFIG_VIEWER
#include "../../Apps/ArtistViewer/Config.hpp"
#else
#include "../../Apps/GameApp/Config.hpp"	
#endif


//=========================================================================
//  Main Menu Dialog
//=========================================================================

enum controls
{
	IDC_LEVEL_SELECT_LISTBOX,
    IDC_LEVEL_SELECT_NAV_TEXT,
};


ui_manager::control_tem LevelSelectControls[] = 
{
    // Frames.
    { IDC_LEVEL_SELECT_LISTBOX,   "IDS_NULL",           "listbox",  40, 60, 200, 232, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_LEVEL_SELECT_NAV_TEXT,  "IDS_NULL",           "text",      0,  0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};


ui_manager::dialog_tem LevelSelectDialog =
{
    "IDS_LEVEL_SELECT",
    1, 9,
    sizeof(LevelSelectControls)/sizeof(ui_manager::control_tem),
    &LevelSelectControls[0],
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

void dlg_level_select_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "level select", &LevelSelectDialog, &dlg_level_select_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_level_select_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_level_select* pDialog = new dlg_level_select;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_level_select
//=========================================================================

dlg_level_select::dlg_level_select( void )
{
}

//=========================================================================

dlg_level_select::~dlg_level_select( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_level_select::Create( s32                        UserID,
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

    m_pLevelList    = (ui_listbox*) FindChildByID( IDC_LEVEL_SELECT_LISTBOX  );
    m_pNavText      = (ui_text*)    FindChildByID( IDC_LEVEL_SELECT_NAV_TEXT );
    
    GotoControl( (ui_control*)m_pLevelList );
    m_CurrentControl = IDC_LEVEL_SELECT_LISTBOX;

    FillLevelList();
    m_pLevelList->SetSelection( 0 );
    m_pLevelList->SetFlag(ui_win::WF_SELECTED, TRUE);
    m_pLevelList->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLevelList->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pLevelList->DisableFrame();
    m_pLevelList->SetExitOnSelect(FALSE);
    m_pLevelList->SetExitOnBack(TRUE);
    m_CurrHL = 0;

    if( !(CONFIG_IS_AUTOSERVER || CONFIG_IS_AUTOCLIENT) )
    {
        // clear in-use controller flags
        for( int i=0; i<MAX_LOCAL_PLAYERS; i++)
        {
            g_StateMgr.SetControllerRequested(i, FALSE);
        }
    }

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" ); 
    m_pNavText->SetLabel( navText );
    m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_level_select::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_level_select::Render( s32 ox, s32 oy )
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

void dlg_level_select::OnNotify ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData )
{
    (void)pWin;
    (void)pSender;
    (void)Command;
    (void)pData;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        if (Command == WN_LIST_ACCEPTED)
        {
            // Set the level name
            map_entry*  pTableEntry = (map_entry*)m_pLevelList->GetItemData(m_pLevelList->GetSelection());
            ASSERT( pTableEntry->GetMapID() );
            s32 Index = 0;
            // We need to translate from our index to the index within the maplist.
            while( TRUE )
            {
                if( g_MapList.GetByIndex( Index ) == pTableEntry )
                {
                    break;
                }
                Index++;
                // We should NOT reach the end of the list here.
                ASSERT( g_MapList.GetByIndex( Index )->GetMapID() != -1 );
            }
            g_StateMgr.SetLevelIndex( Index );
            g_PendingConfig.SetLevelID( pTableEntry->GetMapID() );
            m_State = DIALOG_STATE_SELECT;
        }
    }
}

//=========================================================================

void dlg_level_select::OnPadNavigate( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX, xbool WrapY )
{
    ui_dialog::OnPadNavigate( pWin, Code, Presses, Repeats, WrapX, WrapY );
}

//=========================================================================

void dlg_level_select::OnPadSelect( ui_win* pWin )
{
    // store the active controller
    g_StateMgr.SetActiveControllerID( g_UiMgr->GetActiveController() );
    g_StateMgr.SetControllerRequested( g_UiMgr->GetActiveController(), TRUE );
    g_AudioMgr.Play( "Select_Norm" );
    m_pLevelList->OnPadActivate( pWin );
}

//=========================================================================

void dlg_level_select::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        g_AudioMgr.Play("Backup");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_level_select::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // complete!  turn on the elements
            m_pLevelList ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText   ->SetFlag(ui_win::WF_VISIBLE, TRUE);

            GotoControl( (ui_control*)m_pLevelList );
            g_UiMgr->SetScreenHighlight( m_pLevelList->GetPosition() );
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    // update list box
    // Level selection within range?
    if( (m_pLevelList->GetSelection() >= 0) && (m_pLevelList->GetSelection() < m_pLevelList->GetItemCount()) )
    {
        // Get Pointer to mission definition
        //fegl.pMissionDef = (mission_def*)m_pMissions->GetSelectedItemData();
        //ASSERT( fegl.pMissionDef );
        //fegl.pNextMissionDef = NULL;

        // Set Name and Game Type
        //x_strcpy( fegl.MissionName, fegl.pMissionDef->Folder );
        //fegl.GameType = fegl.pMissionDef->GameType;
    }

}

//=========================================================================

void dlg_level_select::FillLevelList( void )
{
    s32 iSel = m_pLevelList->GetSelection();
    
    // Clear listbox
    m_pLevelList->DeleteAllItems();

    for( s32 i=0; i <g_MapList.GetCount(); i++ )
    {
        const map_entry& Entry = *g_MapList.GetByIndex( i );

        // check if of the correct game type
        if( Entry.GetGameType() == GAME_CAMPAIGN )
        {
            // check if loaded
            if( Entry.IsAvailable() )
            {
                // add an entry to the list
                m_pLevelList->AddItem( Entry.GetDisplayName(), (s32)&Entry );
            }
        }
    }

    ASSERTS( m_pLevelList->GetItemCount() > 0, "No maps are present!" );
    // Limit Selection
    if( iSel <  0 ) iSel = 0;
    if( iSel >= m_pLevelList->GetItemCount() ) 
        iSel  = m_pLevelList->GetItemCount()-1;

    // Set Selection
    m_pLevelList->SetSelection( iSel );
}

#endif // CONFIG_RETAIL

//=========================================================================
