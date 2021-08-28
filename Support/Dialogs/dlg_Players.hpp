//==============================================================================
//  
//  dlg_Players.hpp
//  
//==============================================================================

#ifndef DLG_PLAYERS_HPP
#define DLG_PLAYERS_HPP

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
#include "NetworkMgr\GameMgr.hpp"
#include "NetworkMgr\MatchMgr.hpp"

//==============================================================================
//  dlg_online_join
//==============================================================================

enum player_mode
{
    PLAYER_MODE_RECENT,
    PLAYER_MODE_INGAME,
};

extern void     dlg_players_register  ( ui_manager* pManager );
extern ui_win*  dlg_players_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_players : public ui_dialog
{
public:
                        dlg_players  ( void );
    virtual            ~dlg_players  ( void );

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
    
    void                EnableBlackout      ( void )                    { m_bRenderBlackout = TRUE; }
    void                Configure           ( player_mode Mode );

#ifdef TARGET_XBOX
    enum _SYNC_MODES
    {
        SYNC_MODE_ADD_BUDDY,
        SYNC_MODE_REMOVE_BUDDY,
    };
            void        ActivateSyncPopup   ( s32 SyncMode );
            void        CloseSyncPopup      ( void );
            void        UpdateSyncPopup     ( f32 DeltaTime );
#endif
protected:

    enum
    {
        MAX_NUM_PLAYERS = NET_MAX_PLAYERS + 10,
    };

    void                FillPlayersList     ( void );
    void                PopulatePlayerInfo  ( void );

    ui_frame*           m_pFrame1;
    ui_friendlist*      m_pPlayerList;
    ui_blankbox*        m_pPlayerDetails;

    ui_text*            m_pPlayerName;
    ui_text*            m_pPlayerGame;
    ui_text*            m_pPlayerStatus;
    ui_text*            m_pNavText;
    player_mode         m_PlayerMode;

    s32                 m_CurrHL;
    xbool               m_bRenderBlackout;

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;
    xbool               m_bAskAddAttachment;
    s32                 m_BuddySelected;
    buddy_info          m_Buddy;
    s32                 m_BuddyIndex;

    dlg_popup*          m_SyncPopup;
    s32                 m_SyncPopupResult;
    s32                 m_SyncMode;
    f32                 m_SyncTime;

    dlg_submenu*        m_SubMenu;
    s32                 m_SubMenuResult;

    buddy_info          m_Players[ MAX_NUM_PLAYERS ];
};

//==============================================================================
#endif // DLG_PLAYERS_HPP
//==============================================================================
