//==============================================================================
//  
//  dlg_Team_Leaderboard.hpp
//  
//==============================================================================

#ifndef DLG_TEAM_LEADERBOARD_HPP
#define DLG_TEAM_LEADERBOARD_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_blankbox.hpp"
#include "dlg_PopUp.hpp"

#include "StateMgr\StateMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "dlg_leaderboard.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

enum team_leaderboard_controls
{
    IDC_TEAM_LEADERBOARD_DETAILS,
    IDC_TEAM_LEADERBOARD_FRAME,
    IDC_TEAM_LEADERBOARD_FRAME_MAIN,
    IDC_TEAM_LEADERBOARD_FRAME_TEAM1,
    IDC_TEAM_LEADERBOARD_FRAME_TEAM2,
    IDC_TEAM_LEADERBOARD_FRAME_HEADER1,
    IDC_TEAM_LEADERBOARD_FRAME_HEADER2,
    IDC_TEAM_LEADERBOARD_LOADING_TEXT,
    IDC_TEAM_LEADERBOARD_LOADING_PIPS,
    IDC_TEAM_LEADERBOARD_NAV_TEXT,
};

enum team_score_render_types
{
    RENDER_TEAM_TWO_LINES,
    RENDER_TEAM_TWO_LINES_SMALL,
    RENDER_TEAM_SMALL,
    RENDER_TEAM_NORMAL
};

//==============================================================================
//  dlg_team_leaderboard
//==============================================================================

extern void     dlg_team_leaderboard_register  ( ui_manager* pManager );
extern ui_win*  dlg_team_leaderboard_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_team_leaderboard : public dlg_mp_score
{
public:
                        dlg_team_leaderboard       ( void );
    virtual            ~dlg_team_leaderboard       ( void );

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
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

    void                FillScoreList       ( void );
    void                RenderScoreList     ( s32 ox, s32 oy );
    void                SetTeamRenderType   ( u32 TeamID, irect& Position );

protected:
    ui_frame*           m_pFrame1;
    ui_blankbox*        m_pDetailsBox;

    ui_frame*           m_pFrame;
    ui_frame*           m_pFrameMain;
    ui_frame*           m_pFrameTeamOne;
    ui_frame*           m_pFrameTeamTwo;
    ui_frame*           m_pFrameHeaderOne;
    ui_frame*           m_pFrameHeaderTwo;

    ui_text*            m_pLoadingText;
    ui_text*            m_pLoadingPips;
    ui_text*            m_pNavText;

    player_score        m_PlayerData[32];
    player_score        m_PlayerTotals[2];

    s32                 m_TeamCount[2];
    s32                 m_DrawCount[2];
    s32                 m_CurrHL;
    s32                 m_Font;

    u32                 m_TeamRenderType[2];
    s32                 m_TeamWidth[2];

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
#endif // DLG_TEAM_LEADERBOARD_HPP
//==============================================================================
