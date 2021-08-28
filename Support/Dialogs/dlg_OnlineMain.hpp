//==============================================================================
//  
//  dlg_OnlineMain.hpp
//  
//==============================================================================

#ifndef DLG_ONLINE_MAIN_HPP
#define DLG_ONLINE_MAIN_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"
#include "dlg_PopUp.hpp"
#include "NetworkMgr/Downloader/Downloader.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

enum online_main_controls
{
    IDC_ONLINE_JOIN,
    IDC_ONLINE_HOST,
    IDC_ONLINE_FRIENDS,
    IDC_ONLINE_PLAYERS,
    IDC_ONLINE_EDIT_PROFILE,
    IDC_ONLINE_VIEW_STATS,
    IDC_ONLINE_SIGN_OUT,
    IDC_ONLINE_DOWNLOAD,
    IDC_ONLINE_NAV_TEXT,
};

enum version_check
{
    VERSION_CHECK_INIT = 0,
    VERSION_CHECK_CONTENT_AVAILABLE,
    VERSION_CHECK_IS_NOTIFYING,
    VERSION_CHECK_NO_CONTENT_AVAILABLE,
    VERSION_CHECK_HAS_BEEN_NOTIFIED,
};

//==============================================================================
//  dlg_multi_player
//==============================================================================

extern void     dlg_online_main_register  ( ui_manager* pManager );
extern ui_win*  dlg_online_main_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_online_main : public ui_dialog
{
public:
                        dlg_online_main       ( void );
    virtual            ~dlg_online_main       ( void );

    xbool               Create              ( s32                       UserID,
                                              ui_manager*               pManager,
                                              ui_manager::dialog_tem*   pDialogTem,
                                              const irect&              Position,
                                              ui_win*                   pParent,
                                              s32                       Flags,
                                              void*                     pUserData);
    virtual void        Destroy             ( void );

    virtual void        Render              ( s32 ox=0, s32 oy=0 );

    virtual void        OnPadSelect         ( ui_win* pWin );
    virtual void        OnPadBack           ( ui_win* pWin );
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

protected:
    ui_frame*           m_pFrame1;
    ui_button*          m_pButtonJoin;      
    ui_button*          m_pButtonHost;
    ui_button*          m_pButtonFriends;
    ui_button*          m_pButtonPlayers;
    ui_button*          m_pButtonEditProfile;
    ui_button*          m_pButtonViewStats;
    ui_button*          m_pButtonSignOut;
    ui_button*          m_pButtonDownload;
    ui_text*            m_pNavText;
    s32                 m_CurrHL;
static version_check    m_VersionCheckState;

    s32                 m_NotifyDelay;
    dlg_popup*          m_pPopUp;
    s32                 m_PopUpResult;
};

//==============================================================================
#endif // DLG_ONLINE_MAIN_HPP
//==============================================================================
