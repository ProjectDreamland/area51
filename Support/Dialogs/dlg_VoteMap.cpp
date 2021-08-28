//=========================================================================
//
//  dlg_vote_map.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_maplist.hpp"

#include "dlg_PopUp.hpp"
#include "dlg_VoteMap.hpp"

#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "StateMgr\MapList.hpp"
#include "Objects\Player.hpp"

//=========================================================================
//  Vote Map Dialog
//=========================================================================

enum controls
{
	IDC_VOTE_MAP_LISTBOX,
    IDC_VOTE_MAP_NAV_TEXT,
};


ui_manager::control_tem VoteMapControls[] = 
{
    // Frames.
    { IDC_VOTE_MAP_LISTBOX,   "IDS_VOTE_MAP",     "maplist",  45,  60, 260, 238, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_VOTE_MAP_NAV_TEXT,  "IDS_NULL",         "text",      0,   0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
};

ui_manager::dialog_tem VoteMapDialog =
{
    "IDS_VOTE_MAP_MENU",
    1, 9,
    sizeof(VoteMapControls)/sizeof(ui_manager::control_tem),
    &VoteMapControls[0],
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

void dlg_vote_map_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "vote map", &VoteMapDialog, &dlg_vote_map_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_vote_map_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_vote_map* pDialog = new dlg_vote_map;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_level_select
//=========================================================================

dlg_vote_map::dlg_vote_map( void )
{
}

//=========================================================================

dlg_vote_map::~dlg_vote_map( void )
{
    Destroy();
}

//=========================================================================

xbool dlg_vote_map::Create( s32                        UserID,
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
    m_pMapList         = (ui_maplist*)    FindChildByID( IDC_VOTE_MAP_LISTBOX  );
    m_pNavText         = (ui_text*)       FindChildByID( IDC_VOTE_MAP_NAV_TEXT );

    // hide them
    m_pMapList        ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pNavText        ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_SELECT" ));
    navText += g_StringTableMgr( "ui", "IDS_NAV_BACK" );
   
    m_pNavText->SetLabel( navText );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);

    // set up map list
    m_pMapList->SetFlag(ui_win::WF_SELECTED, TRUE);
    m_pMapList->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pMapList->DisableFrame();
    m_pMapList->SetExitOnSelect(FALSE);
    m_pMapList->SetExitOnBack(TRUE);
    m_pMapList->EnableHeaderBar();
    m_pMapList->SetHeaderBarColor( xcolor(19,59,14,196) );
    m_pMapList->SetHeaderColor( xcolor(255,252,204,255) );
    m_pMapList->DeleteAllItems();

    // fill map list
    for( s32 i=0; i <g_MapList.GetCount(); i++ )
    {
        const map_entry& Entry = *g_MapList.GetByIndex( i );

        // check if loaded
        if( Entry.IsAvailable() && (Entry.GetGameType() != GAME_CAMPAIGN) )
        {
            // build the full map name (includes the game type)
            xwstring FullMapName( xfs( "%s %s", Entry.GetShortGameTypeName(), Entry.GetDisplayName() ) );                    
            // add an entry to the list
            m_pMapList->AddItem( FullMapName, (s32)&Entry );
        }
    }
    // sort the list
    m_pMapList->AlphaSortList();
    // set initial selection
    m_pMapList->SetSelection( 0 );

    // set initial focus
    m_CurrHL = 0;
    GotoControl( (ui_control*)m_pMapList );
    m_CurrentControl = IDC_VOTE_MAP_LISTBOX;
    m_PopUp = NULL;

    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

	// Return success code
    return Success;
}

//=========================================================================

void dlg_vote_map::Destroy( void )
{
    ui_dialog::Destroy();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_vote_map::Render( s32 ox, s32 oy )
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

void dlg_vote_map::OnPadSelect( ui_win* pWin )
{
    (void)pWin;

    if( m_State == DIALOG_STATE_ACTIVE )
    {
        // display confirmation popup
        irect r = g_UiMgr->GetUserBounds( g_UiUserID );
        m_PopUp = (dlg_popup*)g_UiMgr->OpenDialog(  m_UserID, "popup", r, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|WF_INPUTMODAL|ui_win::WF_USE_ABSOLUTE );

        // set nav text
        xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_YES" ));
        navText += g_StringTableMgr( "ui", "IDS_NAV_NO" );
        m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);

        m_PopUp->Configure( g_StringTableMgr( "ui", "IDS_VOTE_MAP" ), 
            TRUE, 
            TRUE, 
            FALSE, 
            g_StringTableMgr( "ui", "IDS_VOTE_MAP_MSG" ),
            navText,
            &m_PopUpResult );

        m_State = DIALOG_STATE_POPUP;
    }
}

//=========================================================================

void dlg_vote_map::OnPadBack( ui_win* pWin )
{
    (void)pWin;

    if ( m_State == DIALOG_STATE_ACTIVE )
    {
        g_AudioMgr.Play("Backup");
        m_State = DIALOG_STATE_BACK;
    }
}

//=========================================================================

void dlg_vote_map::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    // scale window if necessary
    if( g_UiMgr->IsScreenScaling() )
    {
        if( UpdateScreenScaling( DeltaTime ) == FALSE )
        {
            // turn on the controls
            m_pMapList  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            m_pNavText  ->SetFlag(ui_win::WF_VISIBLE, TRUE);
            GotoControl( (ui_control*)m_pMapList );
            g_UiMgr->SetScreenHighlight( m_pMapList->GetPosition() );
        }
    }

    // update the glow bar
    g_UiMgr->UpdateGlowBar(DeltaTime);

    // make sure a vote didn't start
    s32     NetSlot = g_NetworkMgr.GetLocalPlayerSlot(0);
    player* pPlayer = (player*)NetObjMgr.GetObjFromSlot( NetSlot );
    ASSERT( pPlayer );
    if ( pPlayer->GetVoteCanStartMap() == FALSE )
    {
        // kill any popups
        if( m_PopUp )
        {
            if ( m_PopUpResult != DLG_POPUP_IDLE )
            {
                ASSERT( g_UiMgr->GetTopmostDialog(m_UserID) == m_PopUp );
                g_UiMgr->EndDialog( m_UserID, TRUE );
                m_PopUp = NULL;
            }
        }

        // exit vote screen
        m_State = DIALOG_STATE_BACK;
        return;
    }

    if ( m_PopUp )
    {
        if ( m_PopUpResult != DLG_POPUP_IDLE )
        {
            if( m_PopUpResult == DLG_POPUP_YES )
            {
                // Initiate vote for map here!
                map_entry* pTableEntry = (map_entry*)m_pMapList->GetSelectedItemData();
                s32        NetSlot     = g_NetworkMgr.GetLocalPlayerSlot(0);
                player*    pPlayer     = (player*)NetObjMgr.GetObjFromSlot( NetSlot );
                s32        Map         = ((pTableEntry->GetGameType()) << 16) | (pTableEntry->GetMapID());
                ASSERT( pPlayer );

                pPlayer->VoteStartMap( Map );

                m_State = DIALOG_STATE_SELECT;
            }
            else
            {
                // re-enable dialog
                m_State = DIALOG_STATE_ACTIVE;
            }

            // clear popup 
            m_PopUp = NULL;

            // turn on nav text
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
    }
}

//=========================================================================
