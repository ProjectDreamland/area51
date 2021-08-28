//==============================================================================
//  
//  dlg_Leaderboard.hpp
//  
//==============================================================================

#ifndef DLG_LEADERBOARD_HPP
#define DLG_LEADERBOARD_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_blankbox.hpp"
#include "dlg_MPScore.hpp"
#include "dlg_PopUp.hpp"

#include "StateMgr\StateMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

enum leaderboard_controls
{
	IDC_LEADERBOARD_DETAILS,
    IDC_LEADERBOARD_FRAME_ONE,
    IDC_LEADERBOARD_FRAME_MAIN,
    IDC_LEADERBOARD_FRAME_TIMEOUT,
    IDC_LEADERBOARD_TIMEOUT_TEXT,
    IDC_LEADERBOARD_LOADING_TEXT,
    IDC_LEADERBOARD_LOADING_PIPS,
    IDC_LEADERBOARD_NAV_TEXT,
};

//==============================================================================
//  dlg_leaderboard
//==============================================================================

extern void     dlg_leaderboard_register  ( ui_manager* pManager );
extern ui_win*  dlg_leaderboard_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_leaderboard : public dlg_mp_score
{
public:
                        dlg_leaderboard       ( void );
    virtual            ~dlg_leaderboard       ( void );

    virtual xbool       Create              ( s32                       UserID,
                                              ui_manager*               pManager,
                                              ui_manager::dialog_tem*   pDialogTem,
                                              const irect&              Position,
                                              ui_win*                   pParent,
                                              s32                       Flags,
                                              void*                     pUserData);
    virtual void        Destroy             ( void );

    virtual void        Render              ( s32 ox=0, s32 oy=0 );

    virtual void        OnPadSelect         ( ui_win* pWin );
    virtual void        OnPadDelete         ( ui_win* pWin );
#if !defined( TARGET_XBOX )
    virtual void        OnPadBack           ( ui_win* pWin );
#endif
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

    virtual void        Configure           ( leaderboard_mode Mode );

    void                FillScoreList       ( void );
    void                RenderScoreList     ( s32 ox, s32 oy );

protected:
    ui_frame*           m_pFrame1;

    ui_blankbox*        m_pPlayerBox;

    ui_frame*           m_pFrameOne;
    ui_frame*           m_pFrameMain;
    ui_frame*           m_pFrameTimeOut;

    ui_text*            m_pLoadingText;
    ui_text*            m_pLoadingPips;
    ui_text*            m_pNavText;

    player_score        m_PlayerData[32];

    s32                 m_CurrHL;
    s32                 m_Font;

    dlg_popup*          m_pPopUp;
    s32                 m_PopUpResult;

    f32                 m_Time;
    f32                 m_TimeOut;

    const xwchar*       m_pStrLevelDesc;
    const xwchar*       m_pStrPlayer;
    const xwchar*       m_pStrScore;
    const xwchar*       m_pStrIconKills;
    const xwchar*       m_pStrIconDeaths;
    const xwchar*       m_pStrIconTKs;
    const xwchar*       m_pStrIconVotes;
};

//==============================================================================
#endif // DLG_LEADERBOARD_HPP
//==============================================================================
