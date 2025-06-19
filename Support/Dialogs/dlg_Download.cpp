//=========================================================================
//
//  dlg_Download.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_text.hpp"
#include "ui\ui_font.hpp"
#include "ui\ui_manager.hpp"
#include "ui\ui_control.hpp"
#include "ui\ui_listbox.hpp"

#include "dlg_Download.hpp"
#include "StateMgr\StateMgr.hpp"
#include "stringmgr\stringmgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\MatchMgr.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "e_Memcard.hpp"
#include "MemCardMgr\MemCardMgr.hpp"
#include "dlg_MemcardSelect.hpp"

//=========================================================================
//  Main Menu Dialog
//=========================================================================

s32 g_DownloadCardSlot;

#define LABEL_STRING(x) case x: return ( #x );

enum report_error_controls
{
    IDC_LOCAL_MANIFEST_BOX,
    IDC_REMOTE_MANIFEST_BOX,
    IDC_DOWNLOAD_NAV_TEXT,
    IDC_MAP_DETAILS_BOX,

    IDC_GAME_TYPE_TEXT,       
    IDC_MAP_SIZE_TEXT,
    IDC_MAX_PLAYERS_TEXT,
    IDC_MAP_LOCATION_TEXT,

    IDC_INFO_MAP_NAME,
    IDC_INFO_GAME_TYPE,
    IDC_INFO_MAP_SIZE,
    IDC_INFO_MAP_LOCATION,
    IDC_INFO_MAX_PLAYERS,
};

ui_manager::control_tem DownloadControls[] = 
{
    { IDC_DOWNLOAD_NAV_TEXT,    "IDS_NULL",             "text",      0,   0,   0,   0, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_REMOTE_MANIFEST_BOX,  "IDS_DL_REMOTE_FILES",  "listbox",  35,  44, 190, 200, 0, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_LOCAL_MANIFEST_BOX,   "IDS_DL_LOCAL_FILES",   "listbox", 236,  44, 190, 200, 1, 0, 1, 1, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_MAP_DETAILS_BOX,      "IDS_DL_DETAILS_FOR",   "blankbox", 44, 260, 376,  78, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_GAME_TYPE_TEXT,       "IDS_DL_GAME_TYPE",     "text",     43, 282, 110,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MAP_LOCATION_TEXT,    "IDS_DL_LOCATION",      "text",     43, 300, 110,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MAX_PLAYERS_TEXT,     "IDS_DL_MAX_PLAYERS",   "text",     43, 318, 110,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_MAP_SIZE_TEXT,        "IDS_DL_SIZE",          "text",    236, 318, 110,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

    { IDC_INFO_GAME_TYPE,       "IDS_NULL",             "text",    160, 282, 120,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_INFO_MAP_LOCATION,    "IDS_NULL",             "text",    160, 300, 120,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_INFO_MAX_PLAYERS,     "IDS_NULL",             "text",    160, 318, 120,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },
    { IDC_INFO_MAP_SIZE,        "IDS_NULL",             "text",    354, 318, 120,  16, 0, 0, 0, 0, ui_win::WF_VISIBLE | ui_win::WF_SCALE_XPOS | ui_win::WF_SCALE_XSIZE },

};


ui_manager::dialog_tem DownloadDialog =
{
    "IDS_DL_TITLE",
    2, 1,
    sizeof(DownloadControls)/sizeof(ui_manager::control_tem),
    &DownloadControls[0],
    0
};

//=========================================================================
//  Defines
//=========================================================================

//const char* MANIFEST_LOCATION="http://download.area51-game.com/Content";
const char* MANIFEST_LOCATION="http://data.area51-game.com/Content";

//**************************************************************************
// The manifest format is defined in MapList.hpp

//=========================================================================
//  Structs
//=========================================================================

//=========================================================================
//  Data
//=========================================================================

//=========================================================================
//  Registration function
//=========================================================================

void dlg_download_register( ui_manager* pManager )
{
    pManager->RegisterDialogClass( "download", &DownloadDialog, &dlg_download_factory );
}

//=========================================================================
//  Factory function
//=========================================================================

ui_win* dlg_download_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData )
{
    dlg_download* pDialog = new dlg_download;
    pDialog->Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );

    return (ui_win*)pDialog;
}

//=========================================================================
//  dlg_download
//=========================================================================

dlg_download::dlg_download( void )
{
}

//=========================================================================

dlg_download::~dlg_download( void )
{
}

//=========================================================================

xbool dlg_download::Create( s32                        UserID,
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
    m_pNavText              = (ui_text*)        FindChildByID( IDC_DOWNLOAD_NAV_TEXT );
    m_pLocalManifestList    = (ui_listbox*)     FindChildByID( IDC_LOCAL_MANIFEST_BOX ); 
    m_pRemoteManifestList   = (ui_listbox*)     FindChildByID( IDC_REMOTE_MANIFEST_BOX ); 

    m_pMapDetails           = (ui_blankbox*)    FindChildByID( IDC_MAP_DETAILS_BOX );

    m_pGameTypeText         = (ui_text*)        FindChildByID( IDC_GAME_TYPE_TEXT );
    m_pMaxPlayersText       = (ui_text*)        FindChildByID( IDC_MAX_PLAYERS_TEXT );
    m_pMapLocationText      = (ui_text*)        FindChildByID( IDC_MAP_LOCATION_TEXT );
    m_pMapSizeText          = (ui_text*)        FindChildByID( IDC_MAP_SIZE_TEXT );

    m_pGameTypeInfo         = (ui_text*)        FindChildByID( IDC_INFO_GAME_TYPE );
    m_pMaxPlayersInfo       = (ui_text*)        FindChildByID( IDC_INFO_MAX_PLAYERS );
    m_pMapLocationInfo      = (ui_text*)        FindChildByID( IDC_INFO_MAP_LOCATION );
    m_pMapSizeInfo          = (ui_text*)        FindChildByID( IDC_INFO_MAP_SIZE );

    m_pNavText              ->UseSmallText( TRUE );
    m_pGameTypeText         ->UseSmallText( TRUE );
    m_pMaxPlayersText       ->UseSmallText( TRUE );
    m_pMapLocationText      ->UseSmallText( TRUE );
    m_pMapSizeText          ->UseSmallText( TRUE );
    m_pGameTypeInfo         ->UseSmallText( TRUE );
    m_pMaxPlayersInfo       ->UseSmallText( TRUE );
    m_pMapLocationInfo      ->UseSmallText( TRUE );
    m_pMapSizeInfo          ->UseSmallText( TRUE );

    m_pNavText              ->SetFlag( WF_VISIBLE, FALSE );
    m_pLocalManifestList    ->SetFlag( WF_VISIBLE, FALSE );
    m_pRemoteManifestList   ->SetFlag( WF_VISIBLE, FALSE );
    m_pMapDetails           ->SetFlag( WF_VISIBLE, FALSE );
    m_pGameTypeText         ->SetFlag( WF_VISIBLE, FALSE );
    m_pMaxPlayersText       ->SetFlag( WF_VISIBLE, FALSE );
    m_pMapLocationText      ->SetFlag( WF_VISIBLE, FALSE );
    m_pMapSizeText          ->SetFlag( WF_VISIBLE, FALSE );
    m_pGameTypeInfo         ->SetFlag( WF_VISIBLE, FALSE );
    m_pMaxPlayersInfo       ->SetFlag( WF_VISIBLE, FALSE );
    m_pMapLocationInfo      ->SetFlag( WF_VISIBLE, FALSE );
    m_pMapSizeInfo          ->SetFlag( WF_VISIBLE, FALSE );

    m_pGameTypeText         ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pMaxPlayersText       ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pMapLocationText      ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pMapSizeText          ->SetLabelFlags( ui_font::h_right|ui_font::v_center );
    m_pGameTypeInfo         ->SetLabelFlags( ui_font::h_left |ui_font::v_center );
    m_pMaxPlayersInfo       ->SetLabelFlags( ui_font::h_left |ui_font::v_center );
    m_pMapLocationInfo      ->SetLabelFlags( ui_font::h_left |ui_font::v_center );
    m_pMapSizeInfo          ->SetLabelFlags( ui_font::h_left |ui_font::v_center );

    m_pMapDetails           ->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pMapDetails           ->SetHasTitleBar( TRUE );
    m_pMapDetails           ->SetLabelColor( xcolor(255,252,204,255) );
    m_pMapDetails           ->SetTitleBarColor( xcolor(19,59,14,196) );

    // set up manifest list
    m_pLocalManifestList    ->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pLocalManifestList    ->DisableFrame();
    m_pLocalManifestList    ->SetExitOnSelect(FALSE);
    m_pLocalManifestList    ->SetExitOnBack(TRUE);
    m_pLocalManifestList    ->EnableHeaderBar();
    m_pLocalManifestList    ->SetHeaderBarColor( xcolor(19,59,14,196) );
    m_pLocalManifestList    ->SetHeaderColor( xcolor(255,252,204,255) );
    m_pLocalManifestList    ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // set up manifest list
    m_pRemoteManifestList   ->SetBackgroundColor( xcolor (39,117,28,128) );
    m_pRemoteManifestList   ->DisableFrame();
    m_pRemoteManifestList   ->SetExitOnSelect(FALSE);
    m_pRemoteManifestList   ->SetExitOnBack(TRUE);
    m_pRemoteManifestList   ->EnableHeaderBar();
    m_pRemoteManifestList   ->SetHeaderBarColor( xcolor(19,59,14,196) );
    m_pRemoteManifestList   ->SetHeaderColor( xcolor(255,252,204,255) );
    m_pRemoteManifestList   ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // hide them
    m_pNavText              ->SetFlag(ui_win::WF_VISIBLE, FALSE);

    // set up nav text
    xwstring navText(g_StringTableMgr( "ui", "IDS_NAV_OK" ));
   
    m_pNavText->SetLabel( navText );
    m_pNavText->SetLabelFlags( ui_font::h_center|ui_font::v_top|ui_font::is_help_text );
    m_pNavText->UseSmallText(TRUE);
    g_UIMemCardMgr.PollContent( TRUE, this, &dlg_download::OnPollReturn );

    // set initial focus
    m_pPopup                = NULL;
    m_DownloadState         = DOWNLOAD_IDLE;
    m_LocalManifestState    = LMF_IDLE;

    // disable highlight
    g_UiMgr->DisableScreenHighlight();
    m_Position = Position;
    m_ManifestCount[0] = -1;
    m_ManifestCount[1] = -1;
    m_pCardDialog = NULL;

    // initialize screen scaling
    InitScreenScaling( Position );

    // make the dialog active
    m_State = DIALOG_STATE_ACTIVE;

    // Return success code
    return Success;
}

//=========================================================================

void dlg_download::Destroy( void )
{
    ui_dialog::Destroy();
    g_UIMemCardMgr.ClearCallback();

    // kill screen wipe
    g_UiMgr->ResetScreenWipe();
}

//=========================================================================

void dlg_download::Render( s32 ox, s32 oy )
{
    static s32 offset   =  0;
    static s32 gap      =  9;
    static s32 width    =  4;

	irect   rb;
	s32     XRes;
    s32     YRes;

    eng_GetRes( XRes, YRes );
#ifdef TARGET_PS2
    // Nasty hack to force PS2 to draw to rb.l = 0
    rb.Set( -1, 0, XRes, YRes );
#else
    rb.Set( 0, 0, XRes, YRes );
#endif

    if( GetFlags() & WF_VISIBLE )
    {
        g_UiMgr->RenderGouraudRect( rb, xcolor(0,0,0,180),
                                        xcolor(0,0,0,180),
                                        xcolor(0,0,0,180),
                                        xcolor(0,0,0,180), FALSE);
    
    
        // render transparent screen
        rb.l = m_CurrPos.l + 22;
        rb.t = m_CurrPos.t;
        rb.r = m_CurrPos.r - 23;
        rb.b = m_CurrPos.b;

        g_UiMgr->RenderGouraudRect( rb, xcolor(56,115,58,64),
                                        xcolor(56,115,58,64),
                                        xcolor(56,115,58,64),
                                        xcolor(56,115,58,64), FALSE);


        // render the screen bars
        s32 y = rb.t + offset;    

        while ( y < rb.b )
        {
            irect bar;

            if ( ( y + width ) > rb.b )
            {
                bar.Set( rb.l, y, rb.r, rb.b );
            }
            else
            {
                bar.Set( rb.l, y, rb.r, ( y + width ) );
            }

            // draw the bar
            g_UiMgr->RenderGouraudRect( bar, xcolor(56,115,58,30),
                                            xcolor(56,115,58,30),
                                            xcolor(56,115,58,30),
                                            xcolor(56,115,58,30), FALSE);

            y += gap;
        }
        
        // increment the offset
        if ( ++offset > 9 )
        {
            offset = 0;
        }

        // render the normal dialog stuff
        ui_dialog::Render( ox, oy );

        // render the glow bar
        g_UiMgr->RenderGlowBar();
    }
    else
    {
        // render the normal dialog stuff
        ui_dialog::Render( ox, oy );
    }
}

//=========================================================================

void dlg_download::OnPadSelect( ui_win* pWin )
{
    (void)pWin;
    if( pWin == (ui_win*)m_pRemoteManifestList )
    {
        s32 Selection = m_pRemoteManifestList->GetSelection();
        if( Selection != -1 )
        {
            ProgressDialog( "IDS_DL_FETCHING" );
            SetState( DOWNLOAD_FETCH_CONTENT );
        }
    }

    if( pWin == (ui_win*)m_pLocalManifestList )
    {
        // Ask them if they want to delete that specific manifest entry
        s32 Selection = m_pLocalManifestList->GetSelection();
        if( Selection != -1 )
        {
            SetState( DOWNLOAD_DELETE_CONTENT );
        }
    }
}

//=========================================================================

void dlg_download::OnPadBack( ui_win* pWin )
{
    if( m_State == DIALOG_STATE_ACTIVE )
    {
        if( ( pWin ==(ui_win*)m_pLocalManifestList ) ||
            ( pWin ==(ui_win*)m_pRemoteManifestList ) )
        {
            m_State = DIALOG_STATE_BACK;
            g_AudioMgr.Play( "Backup" );
        }

        if( pWin ==(ui_win*)m_pPopup )
        {
        }
        // Check for cancel states when 'please wait' dialog is up
    }
}

//=========================================================================

void dlg_download::OnUpdate ( ui_win* pWin, f32 DeltaTime )
{
    if( m_pCardDialog == NULL )
    {

        MEMORY_OWNER("dlg_download::OnUpdate");
        (void)pWin;
        (void)DeltaTime;
        ui_win* pWindowInFocus = g_UiMgr->GetWindowUnderCursor(m_UserID);

        if( pWindowInFocus == m_pRemoteManifestList )
        {
            xwstring NavText( g_StringTableMgr( "ui", "IDS_NAV_CONTENT_DOWNLOAD") );
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
            if( m_pLocalManifestList->GetItemCount() )
            {
                NavText += g_StringTableMgr( "ui", "IDS_NAV_CONTENT_GOTO_INSTALLED" );
            }
            m_pNavText->SetLabel( NavText );
        }
        else if( pWindowInFocus == m_pLocalManifestList )
        {
            xwstring NavText( g_StringTableMgr( "ui", "IDS_NAV_CONTENT_DELETE") );
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
            if( m_pRemoteManifestList->GetItemCount() )
            {
                NavText += g_StringTableMgr( "ui", "IDS_NAV_CONTENT_GOTO_AVAILABLE" );
            }
            m_pNavText->SetLabel( NavText );
        }

        // scale window if necessary
        if( g_UiMgr->IsScreenScaling() )
        {
            if( UpdateScreenScaling( DeltaTime ) == FALSE )
            {            
                m_pNavText              ->SetFlag( WF_VISIBLE, TRUE );
                m_pLocalManifestList    ->SetFlag( WF_VISIBLE, TRUE );
                m_pRemoteManifestList   ->SetFlag( WF_VISIBLE, TRUE );
                m_pMapDetails           ->SetFlag( WF_VISIBLE, TRUE );
                m_pGameTypeText         ->SetFlag( WF_VISIBLE, TRUE );
                m_pMaxPlayersText       ->SetFlag( WF_VISIBLE, TRUE );
                m_pMapLocationText      ->SetFlag( WF_VISIBLE, TRUE );
                m_pMapSizeText          ->SetFlag( WF_VISIBLE, TRUE );
                m_pGameTypeInfo         ->SetFlag( WF_VISIBLE, TRUE );
                m_pMaxPlayersInfo       ->SetFlag( WF_VISIBLE, TRUE );
                m_pMapLocationInfo      ->SetFlag( WF_VISIBLE, TRUE );
                m_pMapSizeInfo          ->SetFlag( WF_VISIBLE, TRUE );

                ProgressDialog( "IDS_DL_CHECKING" );
                SetState( DOWNLOAD_FETCH_MANIFEST );
            }
        }
        // update the glow bar
        g_UiMgr->UpdateGlowBar(DeltaTime);

        UpdateMapDetails();
        // update everything else
        ui_dialog::OnUpdate( pWin, DeltaTime );
    }

    // This is used to make sure that we can actually have the display rendered with the dialog
    // in place because some of the operations we're about to perform will take a bit of time and
    // are atomic. 
    UpdateState( DeltaTime );

}

//=========================================================================

const char* GetStateName( dlg_download_state State )
{
    switch( State )
    {
    LABEL_STRING( DOWNLOAD_IDLE );
    LABEL_STRING( DOWNLOAD_FETCH_MANIFEST );
    LABEL_STRING( DOWNLOAD_SELECT_CONTENT );
    LABEL_STRING( DOWNLOAD_FETCH_CONTENT );
    LABEL_STRING( DOWNLOAD_SAVE_CONTENT );
    LABEL_STRING( DOWNLOAD_MEMCARD_SELECT );
    LABEL_STRING( DOWNLOAD_DELETE_CONTENT );
    LABEL_STRING( DOWNLOAD_NONE_AVAILABLE );
    LABEL_STRING( DOWNLOAD_MISSING_HARDWARE );
    LABEL_STRING( DOWNLOAD_ERROR );
    LABEL_STRING( DOWNLOAD_ERROR_EXIT );
    default:
        ASSERT( FALSE );
        return "<unknown>";
    }
}

//=========================================================================
void dlg_download::SetState( dlg_download_state NewState )
{
    if( NewState != m_DownloadState )
    {
        LOG_MESSAGE( "dlg_download::SetState", "State transition from %s to %s", GetStateName( m_DownloadState ), GetStateName( NewState ) );
        ExitState( m_DownloadState );
        m_DownloadState = NewState;
        EnterState( m_DownloadState );
    }
}

//=========================================================================

void dlg_download::EnterState( dlg_download_state State )
{
    s32             Selection;
    char            ManifestLocation[256];
    map_entry*      pEntry;
    char*           pFilename;
    const map_info* pMap;


    switch( State )
    {

    case DOWNLOAD_FETCH_MANIFEST:
        x_sprintf(ManifestLocation,"%s/%s_Manifest.txt",MANIFEST_LOCATION, x_GetLocaleString() );
        g_MatchMgr.InitDownload( ManifestLocation );
        break;

    case DOWNLOAD_FETCH_CONTENT:
        Selection = m_pRemoteManifestList->GetSelection();
        ASSERT( Selection != -1 );
        pEntry = m_RemoteManifest.GetByIndex( m_pRemoteManifestList->GetSelectedItemData() );

        pMap = m_RemoteManifest.GetMapInfo( pEntry->GetMapID() );
        ASSERT( pMap );
        x_sprintf( ManifestLocation,"%s/%s", MANIFEST_LOCATION, (const char*)pMap->Filename );
        x_strcpy( m_TargetPath, pMap->Filename );
        pFilename = m_TargetPath + x_strlen(m_TargetPath);
        while( (pFilename != m_TargetPath) && (*pFilename !='\\') && (*pFilename != '/') )
        {
            pFilename--;
        }
        if( pFilename != m_TargetPath )
        {
            *pFilename = 0x0;
        }
        LOG_MESSAGE( "dlg_download::EnterState","Fetch Content: URL:%s, Target Path:%s", ManifestLocation, m_TargetPath );
        g_MatchMgr.InitDownload( ManifestLocation );
        break;
    case DOWNLOAD_MEMCARD_SELECT:
        // Open the memcard select dialog
        {
#if !defined (TARGET_PC)
            irect mainarea( 46, 24, 466, 448-72 );
            m_pCardDialog = (dlg_memcard_select*)g_UiMgr->OpenDialog( g_UiUserID, "memcard select", mainarea, NULL, ui_win::WF_VISIBLE|ui_win::WF_BORDER );
            s32 Length;
            g_MatchMgr.GetDownloadData( Length );
            g_DownloadCardSlot = -1;
            m_pCardDialog->Configure( SM_CARDMODE_CONTENT, Length );
            m_pNavText->SetFlag( ui_win::WF_VISIBLE, FALSE );
#endif
            // hide the main dialog
            SetFlag( WF_VISIBLE, FALSE );
        }
        break;
    case DOWNLOAD_SAVE_CONTENT:
        //
        // Start the save dialog
        //
        // First, form a temporary manifest that contains just the map we want to save.
        {
            map_entry   Entry = m_RemoteManifest[ m_pRemoteManifestList->GetSelectedItemData() ];
            //*** BIG NOTE** CardSlot needs to be set to the slot selected by the user to save the
            // downloaded map to.
            ASSERT( g_DownloadCardSlot >= 0 );
            Entry.SetLocation( g_DownloadCardSlot );
            g_UIMemCardMgr.SaveContent( m_RemoteManifest, Entry, this, &dlg_download::OnSaveContent );
        }
        break;

    case DOWNLOAD_DELETE_CONTENT:
        {
            map_entry Entry = m_LocalManifest[ m_pLocalManifestList->GetSelectedItemData() ];
            g_UIMemCardMgr.DeleteContent( Entry, this, &dlg_download::OnDeleteContent );
        }
        break;

    default:
        break;
    }
}

//=========================================================================

void dlg_download::ExitState( dlg_download_state State )
{
    (void)State;
}

//=========================================================================

void dlg_download::UpdateState( f32 DeltaTime )
{
    (void)DeltaTime;

    if( m_pPopup && (m_PopUpResult != DLG_POPUP_IDLE) )
    {
        // The popup dialog automatically closed itself
        m_pPopup = NULL;
    }
    switch( m_DownloadState )
    {
    case DOWNLOAD_IDLE:
        if( m_PopUpResult != DLG_POPUP_IDLE )
        {
            m_pNavText   ->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
        g_UIMemCardMgr.PollContent( FALSE, this, &dlg_download::OnPollReturn );
        // Here we could check for a memory card state change and reload
        // all the contents of the card.
        break;
    case DOWNLOAD_FETCH_MANIFEST:
        UpdateFetchManifest( DeltaTime );
        break;

    case DOWNLOAD_NONE_AVAILABLE:
        OkDialog( "IDS_DL_NO_CONTENT" );
        SetState( DOWNLOAD_ERROR_EXIT );
        break;

    case DOWNLOAD_SELECT_CONTENT:
        break;

    case DOWNLOAD_FETCH_CONTENT:
        UpdateFetchContent( DeltaTime );
        break;

    case DOWNLOAD_DELETE_CONTENT:
        // ** NOTE ** state change will happen in the callback
        break;

    case DOWNLOAD_ERROR_EXIT:
        if( m_PopUpResult != DLG_POPUP_IDLE )
        {
            m_State = DIALOG_STATE_BACK;
        }
        break;
    case DOWNLOAD_MEMCARD_SELECT:
        ASSERT( m_pCardDialog );
        switch( m_pCardDialog->GetState() )
        {
        case DIALOG_STATE_SELECT:
            // save the content to the selected card slot
            SetState( DOWNLOAD_SAVE_CONTENT );
            break;
        case DIALOG_STATE_BACK:
            // cancel save
            g_MatchMgr.KillDownload();
            SetState( DOWNLOAD_IDLE );
            break;
        default:
            break;
        }

        if( m_pCardDialog->GetState() != DIALOG_STATE_ACTIVE )
        {
            // show the main dialog
            SetFlag( WF_VISIBLE, TRUE );
            ASSERT( g_UiMgr->GetTopmostDialog(m_UserID) == m_pCardDialog );
            g_UiMgr->EndDialog( m_UserID, TRUE );
            g_UiMgr->DisableScreenHighlight();
            m_pCardDialog = NULL;
            m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
        }
        break;
    case DOWNLOAD_SAVE_CONTENT:
        // ** NOTE ** state change will happen in the callback
        break;

    default:
        ASSERT(FALSE);
        break;
    }
}

//=========================================================================
void dlg_download::PopulateLocalContent( void )
{
    s32 i,j;

    m_LocalManifest.Clear();
    for( i=0; i<2; i++ )
    {
        map_list& List = g_UIMemCardMgr.GetManifest(i);
        for( j=0; j<List.GetCount();j++ )
        {

            if( m_LocalManifest.Find(List[j])==-1 )
            {
                // Make sure it's location has been defined.
                List[j].SetLocation( i );
                m_LocalManifest.Append(List[j],&List);
            }
        }
    }
}

//=========================================================================
xbool dlg_download::PopulateContentList( const map_list& Local, const map_list& Remote )
{
    xwstring    Description;
    xstring     Location;
    s32         i,j;

    m_pRemoteManifestList->DeleteAllItems( );
    m_pLocalManifestList->DeleteAllItems( );

    // Create the list of locally available content. This is built from the Manifest.txt stored
    // on the local HDD. The manifest will potentially contain multiple entries for each map
    // name but each entry may have a different game type. We only want to display one entry
    // at a time.

    for( i=0; i<Local.GetCount(); i++ )
    {
        xbool Found;
        Found = FALSE;
        for( j=0; j<m_pLocalManifestList->GetItemCount(); j++ )
        {
            if( Local[m_pLocalManifestList->GetItemData(j)].GetMapID() == Local[i].GetMapID() )
            {
                Found = TRUE;
                break;
            }
        }

        if( !Found )
        {
            // If we didn't find one, then form a new entry for the local manifest list.
            // This contains the 'DisplayName' followed by each different available game
            // type.
            const map_info* pMapInfo;

            pMapInfo = Local.GetMapInfo( Local[i].GetMapID() );
            ASSERT( pMapInfo );
            m_pLocalManifestList->AddItem( (const char*)pMapInfo->DisplayName, i );
        }
    }

    // Create the list of remotely available content. Exclude ANY content that has already been
    // downloaded but the form of the list is otherwise identical to that above.
    for( i=0; i<Remote.GetCount(); i++ )
    {
        xbool Found;

        // Skip any entry that is present in the local list
        Found = Local.Find(Remote[i]) != -1;

        for( j=0; j<m_pRemoteManifestList->GetItemCount(); j++ )
        {
            if( Remote[m_pRemoteManifestList->GetItemData(j)].GetMapID() == Remote[i].GetMapID() )
            {
                Found = TRUE;
                break;
            }
        }

        if( !Found )
        {
            // If we didn't find one, then form a new entry for the local manifest list.
            // This contains the 'DisplayName' followed by each different available game
            // type.
            const map_info* pMapInfo;

            pMapInfo = Remote.GetMapInfo( Remote[i].GetMapID() );
            ASSERT( pMapInfo );
            m_pRemoteManifestList->AddItem( (const char*)pMapInfo->DisplayName, i );
        }
    }

    if( m_pLocalManifestList->GetItemCount() )
    {
        m_pLocalManifestList->SetSelection( 0 );
        m_pLocalManifestList->SetFlag( ui_win::WF_DISABLED, FALSE );
        GotoControl( m_pLocalManifestList );
    }
    else
    {
        m_pLocalManifestList->SetFlag( ui_win::WF_DISABLED, TRUE );
    }

    if( m_pRemoteManifestList->GetItemCount() )
    {
        m_pRemoteManifestList->SetSelection( 0 );
        m_pRemoteManifestList->SetFlag( ui_win::WF_DISABLED, FALSE );
        GotoControl( m_pRemoteManifestList );
    }
    else
    {
        m_pRemoteManifestList->SetFlag( ui_win::WF_DISABLED, TRUE );
    }

    if( m_pRemoteManifestList->GetItemCount() || m_pLocalManifestList->GetItemCount() )
    {
        // Enable the remote and local manifest lists
        m_pRemoteManifestList->SetFlag(ui_win::WF_VISIBLE, TRUE);
        m_pLocalManifestList->SetFlag(ui_win::WF_VISIBLE, TRUE);
        if( m_pRemoteManifestList->GetItemCount() )
        {
            GotoControl( (ui_control*)m_pRemoteManifestList );
        }
        else
        {
            GotoControl( (ui_control*)m_pLocalManifestList );
        }
        return TRUE;
    }

    m_pRemoteManifestList->SetFlag(ui_win::WF_VISIBLE, FALSE);
    m_pLocalManifestList->SetFlag(ui_win::WF_VISIBLE, FALSE);
    // Hide the remote and local manifest lists
    return FALSE;
}

//=========================================================================

void dlg_download::OkDialog( const char* pString, const char* pButton )
{
    irect r = g_UiMgr->GetUserBounds( g_UiUserID );

    m_pNavText   ->SetFlag(ui_win::WF_VISIBLE, FALSE);
    if( m_pPopup == NULL )
    {
        m_pPopup = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
            ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
    }
    
    r.Set( 0, 0, 300, 160 );

    m_pPopup->Configure( r,                                     // Position
        g_StringTableMgr( "ui", "IDS_NETWORK_POPUP" ),          // Title 
        TRUE,                                                   // Yes
        FALSE,                                                  // No
        FALSE,                                                  // Maybe
        g_StringTableMgr( "ui", pString ),                      // Message
        g_StringTableMgr( "ui", pButton ),                      // Nav text
        &m_PopUpResult );                                       // Result
}

//=========================================================================

void dlg_download::CloseDialog( void )
{
    ASSERT( m_pPopup );

    ASSERT( g_UiMgr->GetTopmostDialog(m_UserID) == m_pPopup );
    g_UiMgr->EndDialog( m_UserID, TRUE );
    m_pPopup = NULL;
    m_pNavText->SetFlag(ui_win::WF_VISIBLE, TRUE);
}

//=========================================================================

void dlg_download::ProgressDialog( const char* pString, xbool AllowBackout )
{
    irect r = g_UiMgr->GetUserBounds( g_UiUserID );

    m_pNavText->SetFlag(ui_win::WF_VISIBLE, FALSE);


    if( m_pPopup == NULL )
    {
        m_pPopup = (dlg_popup*)g_UiMgr->OpenDialog(  g_UiUserID, "popup", r, NULL, 
            ui_win::WF_VISIBLE|ui_win::WF_BORDER|ui_win::WF_DLG_CENTER|ui_win::WF_USE_ABSOLUTE|WF_INPUTMODAL );
    }

    r.Set( 0, 0, 300, 160 );

    const xwchar* pCancelText;

    if( AllowBackout )
    {
        pCancelText = g_StringTableMgr( "ui", "IDS_NAV_CANCEL" );
    }
    else
    {
        pCancelText = g_StringTableMgr( "ui", "IDS_NULL" );
    }

    m_pPopup->Configure( r,                                                     // Position
                        g_StringTableMgr( "ui", "IDS_WAIT_TEXT" ),              // Title
                        FALSE,                                                  // Yes
                        AllowBackout,                                           // No
                        FALSE,                                                  // Maybe
                        g_StringTableMgr( "ui", pString          ),             // Message
                        pCancelText,                                            // Nav text
                        &m_PopUpResult );                                       // Result
}

//=========================================================================
xbool dlg_download::SaveManifest( const char* pFilename, const map_list& ManifestList )
{
    xstring Manifest;

    Manifest = ManifestList.Serialize();
    return Manifest.SaveFile( pFilename );
}

//=========================================================================
void dlg_download::MoveManifestEntry( s32 MapID, map_list& SourceManifest, map_list& TargetManifest )
{
(    void)MapID;
(void) SourceManifest;
(void)TargetManifest;
#if 0
    s32 i;

    for( i=0; i< SourceManifest.GetCount(); i++ )
    {
        if( SourceManifest[i].MapID == MapID )
        {
            TargetManifest.Append( SourceManifest[i] );
            SourceManifest.Delete(i);
            i--;
        }
    }
#endif
}

//=========================================================================
const char* GetStateName( download_lmf_state State )
{
    switch( State )
    {
        LABEL_STRING( LMF_IDLE );
        LABEL_STRING( LMF_DONE );
        LABEL_STRING( LMF_START_FETCH );
        LABEL_STRING( LMF_CHECK_CARDS );
        LABEL_STRING( LMF_WAIT_CHECK_CARDS );
        LABEL_STRING( LMF_WAIT_SET_DIRECTORY );
        LABEL_STRING( LMF_ACQUIRE_MANIFEST );
        LABEL_STRING( LMF_ACQUIRE_FILE_CONTENT );
    default:
        ASSERT( FALSE );
        return( "<unknown>" );
    }
}

//=========================================================================
void dlg_download::SetLocalManifestState( download_lmf_state NewState )
{
    LOG_MESSAGE( "dlg_download::SetLocalManifestState", "Local Manifest state changed from %s to %s", GetStateName( m_LocalManifestState ), GetStateName( NewState ) );
    m_LocalManifestState = NewState;
}

//=========================================================================
void dlg_download::UpdateFetchManifest( f32 DeltaTime )
{
    (void)DeltaTime;
    s32 Length;

    switch( g_MatchMgr.GetDownloadStatus() )
    {
        //-------------------------------------------------
    case DL_STAT_BUSY:
        if( m_PopUpResult != DLG_POPUP_IDLE )
        {
            g_MatchMgr.KillDownload();
            m_State = DIALOG_STATE_BACK;
            SetState( DOWNLOAD_IDLE );
        }
        break;

        //-------------------------------------------------
    case DL_STAT_OK:
        m_RemoteManifest.Parse( (const char*)g_MatchMgr.GetDownloadData( Length ), MF_DOWNLOAD_MAP, -1 );
        g_MatchMgr.KillDownload();
        PopulateContentList( m_LocalManifest, m_RemoteManifest );
        if( m_LocalManifest.GetCount() )
        {
        }
        else if( m_RemoteManifest.GetCount() )
        {
        }
        if( (m_LocalManifest.GetCount()==0) && (m_RemoteManifest.GetCount()==0) )
        {
            SetState( DOWNLOAD_NONE_AVAILABLE );
        }
        else
        {
            SetState( DOWNLOAD_IDLE );
            CloseDialog();
        }
        break;
        //-------------------------------------------------
    case DL_STAT_NOT_FOUND:
        // Change the dialog so we notify them there was no data.
        SetState( DOWNLOAD_NONE_AVAILABLE );
        g_MatchMgr.KillDownload();
        break;
        //-------------------------------------------------
    default:
        ASSERT( FALSE );
        break;
    }
}

//=========================================================================
void dlg_download::UpdateFetchContent( f32 DeltaTime )
{
    (void)DeltaTime;

    switch( g_MatchMgr.GetDownloadStatus() )
    {
        //-------------------------------------------------
    case DL_STAT_BUSY:
        if( m_PopUpResult != DLG_POPUP_IDLE )
        {
            ASSERT( m_PopUpResult == DLG_POPUP_NO );
            g_MatchMgr.KillDownload();
            m_pPopup = NULL;
            OkDialog( "IDS_DL_CANCELLED" );
            SetState( DOWNLOAD_IDLE );
        }
        else
        {
            xwstring Progress;
            s32      Complete;

            Complete = (s32)(g_MatchMgr.GetDownloadProgress() * 100.0f );

            Progress = g_StringTableMgr( "ui", "IDS_DL_FETCHING");

            Progress += (const char*)xfs("%d",Complete);
            Progress += g_StringTableMgr( "ui", "IDS_DL_PERCENT" );

            irect r(0,0,300,160);

            m_pPopup->Configure( r,                                     // Position
                g_StringTableMgr( "ui", "IDS_WAIT_TEXT" ),              // Title
                FALSE,                                                  // Yes
                TRUE,                                                   // No
                FALSE,                                                  // Maybe
                Progress,                                               // Message
                g_StringTableMgr( "ui", "IDS_NAV_CANCEL" ),             // Nav text
                &m_PopUpResult );                                       // Result
        }

        break;

        //-------------------------------------------------
    case DL_STAT_OK:
        // Change the dialog so we notify them there was no data.
        CloseDialog();
        SetState( DOWNLOAD_MEMCARD_SELECT );
        break;

        //-------------------------------------------------
    case DL_STAT_NOT_FOUND:
        g_MatchMgr.KillDownload();
        OkDialog( "IDS_DL_FAILED" );
        SetState( DOWNLOAD_IDLE );
        break;
        //-------------------------------------------------
    case DL_STAT_ERROR:
        g_MatchMgr.KillDownload();
        OkDialog( "IDS_DL_FAILED" );
        SetState( DOWNLOAD_IDLE );
        break;
        //-------------------------------------------------
    default:
        ASSERT( FALSE );
        break;
    }
}

//=========================================================================

//=========================================================================
void dlg_download::UpdateMapDetails( void )
{
    xwstring                BannerText;
    xwstring                GameTypeText;
    xwstring                LocationText;
    xwstring                MaxPlayersText;
    xwstring                ContentSizeText;
    map_list*               pManifest = NULL;
    s32                     MapID;
    s32                     Index = 0;
    xbool                   IsLocal = FALSE;

    BannerText = g_StringTableMgr( "ui", "IDS_DL_DETAILS_FOR" );

    if( (m_pLocalManifestList->GetFlags() & ui_win::WF_SELECTED) && (m_pLocalManifestList->GetSelection()!=-1) )
    {
        // More details on a local map
        pManifest = &m_LocalManifest;
        Index = m_pLocalManifestList->GetSelectedItemData();
        IsLocal = TRUE;
        map_entry& Entry = m_LocalManifest[Index];
        if( Entry.GetLocation() )
        {
            LocationText = g_StringTableMgr( "ui", "IDS_DL_MEMCARD_2" );
        }
        else
        {
            LocationText = g_StringTableMgr( "ui", "IDS_DL_MEMCARD_1" );
        }
    }

    if( (m_pRemoteManifestList->GetFlags() & ui_win::WF_SELECTED) && (m_pRemoteManifestList->GetSelection()!=-1) )
    {
        // More details on a remote map
        pManifest = &m_RemoteManifest;
        Index = m_pRemoteManifestList->GetSelectedItemData();
        IsLocal = FALSE;
        LocationText = g_StringTableMgr( "ui", "IDS_DL_CONTENT_SERVER" );
    }

    s32 Length;

    Length = 0;
    if( pManifest )
    {
        map_list& Manifest = *pManifest;

        const map_info* pMapInfo = Manifest.GetMapInfo( Manifest[Index].GetMapID() );

        ASSERT( pMapInfo );

        xbool NeedComma;
        NeedComma = FALSE;
        MapID = Manifest[ Index ].GetMapID();
        BannerText += " ";
        BannerText += (const char*)pMapInfo->DisplayName;
        Length = pMapInfo->Length;

        while( (Index < Manifest.GetCount()) && (Manifest[Index].GetMapID() == MapID) )
        {
            const game_type_info* pGameInfo = Manifest.GetGameTypeInfo( Manifest[Index].GetGameType() );
            ASSERT( pGameInfo );

            if( NeedComma )
            {
                GameTypeText += ", ";
            }
            GameTypeText += (const char*)pGameInfo->ShortTypeName;
            NeedComma = TRUE;
            Index++;
        }
    }
    m_pMapDetails       ->SetLabel( BannerText );
    m_pGameTypeInfo     ->SetLabel( GameTypeText );
    m_pMapLocationInfo  ->SetLabel( LocationText );
    m_pMapSizeInfo      ->SetLabel( (const char*)xfs("%dK", Length/1024) );
    m_pMaxPlayersInfo   ->SetLabel( "16" );
}

//=========================================================================
void dlg_download::OnPollReturn( void )
{
    if( (g_UIMemCardMgr.GetManifest(0).GetCount() != m_ManifestCount[0]) || (g_UIMemCardMgr.GetManifest(1).GetCount() != m_ManifestCount[1]) )
    {
        PopulateLocalContent();
        PopulateContentList( m_LocalManifest, m_RemoteManifest );
        m_ManifestCount[0] = g_UIMemCardMgr.GetManifest(0).GetCount();
        m_ManifestCount[1] = g_UIMemCardMgr.GetManifest(1).GetCount();
    }
}

//=========================================================================
void dlg_download::OnSaveContent( void )
{
    MemCardMgr::condition& Condition1 = g_UIMemCardMgr.GetCondition( 0 );
#ifdef TARGET_PS2
    MemCardMgr::condition& Condition2 = g_UIMemCardMgr.GetCondition( 1 );

    g_MatchMgr.KillDownload();
    // if the load was successful
    if( Condition1.SuccessCode || Condition2.SuccessCode )
#else
    if( Condition1.SuccessCode )
#endif
    {
        SetState( DOWNLOAD_IDLE );
    }
    else
    {
        OkDialog( "IDS_DL_SAVE_FAILED" );
        SetState( DOWNLOAD_IDLE );
    }
}

//=========================================================================
void dlg_download::OnDeleteContent( void )
{
    MemCardMgr::condition& Condition1 = g_UIMemCardMgr.GetCondition( 0 );
#ifdef TARGET_PS2
    MemCardMgr::condition& Condition2 = g_UIMemCardMgr.GetCondition( 1 );

    // if the load was successful
    if( Condition1.SuccessCode || Condition2.SuccessCode )
#else
    if( Condition1.SuccessCode )
#endif
    {
        SetState( DOWNLOAD_IDLE );
    }
    else
    {
        OkDialog( "IDS_DL_DELETE_FAILED" );
        SetState( DOWNLOAD_IDLE );
    }
}
