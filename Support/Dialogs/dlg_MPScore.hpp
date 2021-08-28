//==============================================================================
//  
//  dlg_MPScore.hpp
//  
//==============================================================================

#ifndef DLG_MP_SCORE_HPP
#define DLG_MP_SCORE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"

//==============================================================================
//  dlg_mp_score
//==============================================================================

enum leaderboard_mode
{
    LEADERBOARD_FINAL,
    LEADERBOARD_INTERLEVEL,
    LEADERBOARD_PAUSE,
};

class dlg_mp_score : public ui_dialog
{
public:
                        dlg_mp_score        ( void );
    virtual            ~dlg_mp_score        ( void );

    virtual xbool       Create              ( s32                       UserID,
                                              ui_manager*               pManager,
                                              ui_manager::dialog_tem*   pDialogTem,
                                              const irect&              Position,
                                              ui_win*                   pParent,
                                              s32                       Flags,
                                              void*                     pUserData);
    virtual void        Destroy             ( void );

    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

    void                LoadingComplete     ( void );
    
    void                ForceExit           ( void )                            { m_ForceExit = TRUE; }
    xbool               GetForceExit        ( void )                            { return m_ForceExit; }

    virtual void        Configure           ( leaderboard_mode Mode );
    leaderboard_mode    GetLeaderboardMode  ( void )                            { return m_Mode; }

protected:
    xbool               m_LoadingComplete;
    xbool               m_ForceExit;
    leaderboard_mode    m_Mode;
    f32                 m_LoadTimeElapsed;
};

//==============================================================================
#endif // DLG_MP_SCORE_HPP
//==============================================================================
