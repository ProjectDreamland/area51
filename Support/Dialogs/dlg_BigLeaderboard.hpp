//==============================================================================
//  
//  dlg_BigLeaderboard.hpp
//  
//==============================================================================

#ifndef DLG_BIG_LEADERBOARD_HPP
#define DLG_BIG_LEADERBOARD_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_blankbox.hpp"
#include "dlg_PopUp.hpp"
#include "dlg_leaderboard.hpp"

#include "StateMgr\StateMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

enum big_leaderboard_controls
{
	IDC_BIG_LEADERBOARD_ONE_DETAILS,
	IDC_BIG_LEADERBOARD_TWO_DETAILS,
    IDC_BIG_LEADERBOARD_FRAME_ONE,
    IDC_BIG_LEADERBOARD_FRAME_TWO,
    IDC_BIG_LEADERBOARD_FRAME_MAIN,
    IDC_BIG_LEADERBOARD_LOADING_TEXT,
    IDC_BIG_LEADERBOARD_LOADING_PIPS,
    IDC_BIG_LEADERBOARD_NAV_TEXT,
};

enum player_data_types
{
    TYPE_KILLS = 0,
    TYPE_DEATHS,
    TYPE_TKS,
    TYPE_FLAGS,
    TYPE_VOTES,

    NUM_DATA_TYPES,

    TYPE_NONE
};

//==============================================================================
//  dlg_big_leaderboard
//==============================================================================

extern void     dlg_big_leaderboard_register  ( ui_manager* pManager );
extern ui_win*  dlg_big_leaderboard_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_big_leaderboard : public dlg_mp_score
{
public:
                        dlg_big_leaderboard       ( void );
    virtual            ~dlg_big_leaderboard       ( void );

    virtual xbool       Create              ( s32                       UserID,
                                              ui_manager*               pManager,
                                              ui_manager::dialog_tem*   pDialogTem,
                                              const irect&              Position,
                                              ui_win*                   pParent,
                                              s32                       Flags,
                                              void*                     pUserData);
    virtual void        Destroy             ( void );

    virtual void        Render              ( s32 ox=0, s32 oy=0 );

    virtual void        OnPadNavigate       ( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX = FALSE, xbool WrapY = FALSE );
    virtual void        OnPadSelect         ( ui_win* pWin );
    virtual void        OnPadDelete         ( ui_win* pWin );
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

    void                FillScoreList       ( void );
    void                RenderScoreList     ( s32 ox, s32 oy );
    void                CycleStatDisplay    ( xbool bForward );

protected:
    ui_frame*           m_pFrame1;

    ui_blankbox*        m_pPlayerOneBox;
    ui_blankbox*        m_pPlayerTwoBox;

    ui_frame*           m_pFrameOne;
    ui_frame*           m_pFrameTwo;
    ui_frame*           m_pFrameMain;

    ui_text*            m_pLoadingText;
    ui_text*            m_pLoadingPips;
    ui_text*            m_pNavText;

    player_score        m_PlayerData[NET_MAX_PLAYERS];
    player_score        m_PlayerTotals[2];

    s8                  m_NumTeams;
    s8                  m_SelectedType;
    s32                 m_CurrHL;
    s32                 m_Font;

    dlg_popup*          m_pPopUp;
    s32                 m_PopUpResult;

    const xwchar*       m_pStrLevelDesc;
    const xwchar*       m_pStrPlayer;
    const xwchar*       m_pStrScore;
    const xwchar*       m_pStrIconKills;
    const xwchar*       m_pStrIconDeaths;
    const xwchar*       m_pStrIconTKs;
    const xwchar*       m_pStrIconFlags;
    const xwchar*       m_pStrIconVotes;
};

//==============================================================================
#endif // DLG_BIG_LEADERBOARD_HPP
//==============================================================================
