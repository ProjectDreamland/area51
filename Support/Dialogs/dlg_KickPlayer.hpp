//==============================================================================
//  
//  dlg_KickPlayer.hpp
//  
//==============================================================================

#ifndef DLG_KICK_PLAYER_HPP
#define DLG_KICK_PLAYER_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_playerlist.hpp"

#include "dlg_PopUp.hpp"
#include "NetworkMgr\GameMgr.hpp"

//==============================================================================
//  dlg_kick_player
//==============================================================================

extern void     dlg_kick_player_register  ( ui_manager* pManager );
extern ui_win*  dlg_kick_player_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_kick_player : public ui_dialog
{
public:
                        dlg_kick_player ( void );
    virtual            ~dlg_kick_player ( void );

    xbool               Create                  ( s32                       UserID,
                                                  ui_manager*               pManager,
                                                  ui_manager::dialog_tem*   pDialogTem,
                                                  const irect&              Position,
                                                  ui_win*                   pParent,
                                                  s32                       Flags,
                                                  void*                     pUserData);
    virtual void        Destroy                 ( void );

    virtual void        Render                  ( s32 ox=0, s32 oy=0 );

    virtual void        OnPadSelect             ( ui_win* pWin );
    virtual void        OnPadBack               ( ui_win* pWin );
    virtual void        OnUpdate                ( ui_win* pWin, f32 DeltaTime );

    void                RefreshPlayerList       ( void );
protected:
    ui_frame*           m_pFrame1;
    ui_playerlist*      m_pPlayerList;

    ui_text*            m_pNavText;

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;

    player_score        m_PlayerData[NET_MAX_PLAYERS];

    s32                 m_CurrHL;
};

//==============================================================================
#endif // DLG_KICK_PLAYER_HPP
//==============================================================================
