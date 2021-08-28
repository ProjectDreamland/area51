//==============================================================================
//  
//  dlg_Download.hpp
//  
//==============================================================================

#ifndef DLG_DOWNLOAD_HPP
#define DLG_DOWNLOAD_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_listbox.hpp"
#include "ui\ui_blankbox.hpp"

#include "dialogs\dlg_popup.hpp"

#include "NetworkMgr/NetworkMgr.hpp"
#include "NetworkMgr/Downloader/Archive.hpp"
#include "NetworkMgr/GameMgr.hpp"
#include "StateMgr/MapList.hpp"

enum dlg_download_state
{
    DOWNLOAD_IDLE,
    DOWNLOAD_FETCH_MANIFEST,
    DOWNLOAD_SELECT_CONTENT,
    DOWNLOAD_FETCH_CONTENT,
    DOWNLOAD_MEMCARD_SELECT,
    DOWNLOAD_SAVE_CONTENT,
    DOWNLOAD_DELETE_CONTENT,
    DOWNLOAD_NONE_AVAILABLE,
    DOWNLOAD_MISSING_HARDWARE,
    DOWNLOAD_ERROR,
    DOWNLOAD_ERROR_EXIT,
};

enum download_lmf_state
{
    LMF_IDLE,
    LMF_DONE,
    LMF_START_FETCH,
    LMF_CHECK_CARDS,
    LMF_WAIT_CHECK_CARDS,
    LMF_WAIT_SET_DIRECTORY,
    LMF_ACQUIRE_MANIFEST,
    LMF_ACQUIRE_FILE_CONTENT,
};

#define MAX_MANIFEST_SIZE (16*1024)

class dlg_memcard_select;

//==============================================================================
//  dlg_download
//==============================================================================

extern s32 g_DownloadCardSlot;

extern void     dlg_download_register  ( ui_manager* pManager );
extern ui_win*  dlg_download_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;


class dlg_download : public ui_dialog
{
public:
                                dlg_download            ( void );
    virtual                    ~dlg_download            ( void );
    xbool                       Create                  ( s32                       UserID,
                                                          ui_manager*               pManager,
                                                          ui_manager::dialog_tem*   pDialogTem,
                                                          const irect&              Position,
                                                          ui_win*                   pParent,
                                                          s32                       Flags,
                                                          void*                     pUserData);
    virtual void                Destroy                 ( void );

    virtual void                Render                  ( s32 ox=0, s32 oy=0 );

    virtual void                OnPadSelect             ( ui_win* pWin );
    virtual void                OnPadBack               ( ui_win* pWin );
    virtual void                OnUpdate                ( ui_win* pWin, f32 DeltaTime );

    void                        OnPollReturn            ( void );
    void                        OnLoadContent           ( void );
    void                        OnDeleteContent         ( void );
    void                        OnSaveContent           ( void );

protected:
    void                        SetState                ( dlg_download_state NewState );
    void                        EnterState              ( dlg_download_state State );
    void                        ExitState               ( dlg_download_state State );
    void                        UpdateState             ( f32 DeltaTime );
    xbool                       PopulateContentList     ( const map_list& Local, const map_list& Remote );
    void                        PopulateLocalContent    ( void );
    void                        OkDialog                ( const char* pString, const char* pButton = "IDS_NAV_OK" );
    void                        ProgressDialog          ( const char* pString, xbool AllowBackout = TRUE );
    void                        CloseDialog             ( void );

    xbool                       SaveManifest            ( const char* pFilename, const map_list& MapList );
    void                        MoveManifestEntry       ( s32 MapID, map_list& SourceManifest, map_list& TargetManifest );
    void                        UpdateMapDetails        ( void );
    void                        UpdateFetchManifest     ( f32 DeltaTime );
    void                        UpdateFetchContent      ( f32 DeltaTime );  
    void                        SetLocalManifestState   ( download_lmf_state NewState );

    ui_text*                    m_pNavText;
    dlg_popup*                  m_pPopup;
    dlg_memcard_select*         m_pCardDialog;

    ui_listbox*                 m_pLocalManifestList;
    ui_listbox*                 m_pRemoteManifestList;
    ui_blankbox*                m_pMapDetails;

    ui_text*                    m_pGameTypeText;
    ui_text*                    m_pMaxPlayersText;
    ui_text*                    m_pMapLocationText;
    ui_text*                    m_pMapSizeText;

    ui_text*                    m_pGameTypeInfo;
    ui_text*                    m_pMaxPlayersInfo;
    ui_text*                    m_pMapLocationInfo;
    ui_text*                    m_pMapSizeInfo;

    archive                     m_Package;

    map_list                    m_RemoteManifest;
    map_list                    m_LocalManifest;
    s32                         m_ManifestCount[2];
    s32                         m_RemoteVersion;
    s32                         m_LocalVersion;
    s32                         m_PopUpResult;
    dlg_download_state          m_DownloadState;
    download_lmf_state          m_LocalManifestState;
    char                        m_TargetPath[128];
    s32                         m_CardIndex;
    s32                         m_FileIndex;
    void*                       m_pFileData;
};

//==============================================================================
#endif // DLG_DOWNLOAD_HPP
//==============================================================================
