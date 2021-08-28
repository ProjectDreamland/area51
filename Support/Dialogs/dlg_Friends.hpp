//==============================================================================
//  
//  dlg_Friends.hpp
//  
//==============================================================================

#ifndef DLG_FRIENDS_HPP
#define DLG_FRIENDS_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_friendlist.hpp"
#include "ui\ui_blankbox.hpp"

#include "dlg_SubMenu.hpp"
#include "dlg_PopUp.hpp"

#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\MatchMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "NetworkMgr\MatchMgr.hpp"

//==============================================================================
//  dlg_online_join
//==============================================================================

extern void     dlg_friends_register  ( ui_manager* pManager );
extern ui_win*  dlg_friends_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_friends : public ui_dialog
{
public:
                        dlg_friends  ( void );
    virtual            ~dlg_friends  ( void );

    xbool               Create              ( s32                       UserID,
                                              ui_manager*               pManager,
                                              ui_manager::dialog_tem*   pDialogTem,
                                              const irect&              Position,
                                              ui_win*                   pParent,
                                              s32                       Flags,
                                              void*                     pUserData);
    virtual void        Destroy             ( void );

    virtual void        Render              ( s32 ox=0, s32 oy=0 );

    virtual void        OnNotify            ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData );
    virtual void        OnPadNavigate       ( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX = FALSE, xbool WrapY = FALSE );
    virtual void        OnPadSelect         ( ui_win* pWin );
    virtual void        OnPadBack           ( ui_win* pWin );
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );
    
    void                FillFriendsList     ( void );
    void                PopulateFriendInfo  ( void );
    void                EnableBlackout      ( void )                    { m_bRenderBlackout = TRUE; }

    void                OpenSubMenu         ( void );

    void                JoinFriend          ( void );
    void                AcceptInvite        ( void );
    enum PLAY_BACK_MODES
    {
        PLAY_BACK_MODE_GAME_INVITE=0,
        PLAY_BACK_MODE_FRIEND_REQ
    };

#ifdef TARGET_XBOX
    enum _SYNC_MODES
    {
        SYNC_MODE_INVITE_BUDDY,
    };
    void        ActivateSyncPopup   ( s32 SyncMode );
    void        CloseSyncPopup      ( void );
    void        UpdateSyncPopup     ( f32 DeltaTime );
#endif

protected:

    void                SetupLocalMessage   ( const buddy_info* pBuddy      );
    xbool               IsInSameGame        ( const buddy_info* pBuddy      ) const;
    const char*         GetMessageString    ( s32               MessageID   ) const;
    void                SetupInsertDiscPopup( xbool             IsInvite    );

    ui_frame*           m_pFrame1;
    ui_friendlist*      m_pFriendList;
    ui_blankbox*        m_pFriendDetails;

    ui_text*            m_pFriendName;
    ui_text*            m_pFriendGame;
    ui_text*            m_pFriendStatus;
    ui_text*            m_pNavText;

    s32                 m_CurrHL;
    xbool               m_bRenderBlackout;
    xbool               m_bAskAddAttachment;
    xbool               m_bAskPlayAttachment;
    buddy_info          m_Buddy;

    dlg_submenu*        m_SubMenu;
    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;
    s32                 m_PopUpType;

    dlg_popup*          m_AttachPopUp;
    s32                 m_AttachPopUpResult;

    dlg_popup*          m_PlayAttachPopUp;
    s32                 m_PlayAttachPopUpResult;

    dlg_popup*          m_DefaultPopUp;
    s32                 m_DefaultPopUpResult;

    s32                 m_AttachPlayBackMode;

    s32                 m_SubMenuResult;
    s32                 m_LocalMessage[5];

    // sync popup
    dlg_popup*          m_SyncPopup;
    s32                 m_SyncPopupResult;
    s32                 m_SyncMode;
    f32                 m_SyncTime;

};

//==============================================================================
#endif // DLG_FRIENDS_HPP
//==============================================================================
