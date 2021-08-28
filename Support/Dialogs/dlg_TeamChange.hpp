//==============================================================================
//  
//  dlg_TeamChange.hpp
//  
//==============================================================================

#ifndef DLG_TEAM_CHANGE_HPP
#define DLG_TEAM_CHANGE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_playerlist.hpp"
#include "ui\ui_blankbox.hpp"

#include "dlg_PopUp.hpp"
#include "NetworkMgr\GameMgr.hpp"

//==============================================================================
//  dlg_team_change
//==============================================================================

extern void     dlg_team_change_register  ( ui_manager* pManager );
extern ui_win*  dlg_team_change_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_team_change : public ui_dialog
{
public:
                        dlg_team_change ( void );
    virtual            ~dlg_team_change ( void );

    xbool               Create                  ( s32                       UserID,
                                                  ui_manager*               pManager,
                                                  ui_manager::dialog_tem*   pDialogTem,
                                                  const irect&              Position,
                                                  ui_win*                   pParent,
                                                  s32                       Flags,
                                                  void*                     pUserData);
    virtual void        Destroy                 ( void );

    virtual void        Render                  ( s32 ox=0, s32 oy=0 );

    virtual void        OnNotify                ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData );
    virtual void        OnPadNavigate           ( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX = FALSE, xbool WrapY = FALSE );
    virtual void        OnPadBack               ( ui_win* pWin );
    virtual void        OnUpdate                ( ui_win* pWin, f32 DeltaTime );
    
    void                FillTeamLists           ( void );

protected:
    ui_frame*           m_pFrame1;

    ui_blankbox*        m_pAlphaTeamHeader;
    ui_blankbox*        m_pOmegaTeamHeader;

    ui_text*            m_pAlphaTeamName;
    ui_text*            m_pOmegaTeamName;
    ui_text*            m_pAlphaTeamScore;
    ui_text*            m_pOmegaTeamScore;

    ui_playerlist*      m_pAlphaTeamList;
    ui_playerlist*      m_pOmegaTeamList;

    ui_text*            m_pNavText;

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;
    s32                 m_PopUpType;

    s32                 m_CurrHL;

    player_score        m_AlphaPlayerData[32];
    player_score        m_OmegaPlayerData[32];
};

//==============================================================================
#endif // DLG_TEAM_CHANGE_HPP
//==============================================================================
