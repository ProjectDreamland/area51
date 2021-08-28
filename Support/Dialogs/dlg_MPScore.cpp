//=========================================================================
//
//  dlg_MPScore.cpp
//
//=========================================================================

#include "entropy.hpp"

#include "ui\ui_manager.hpp"

#include "dlg_MPScore.hpp"
#include "StateMgr\StateMgr.hpp"

//=========================================================================
//  Multi-player score screen dialog
//=========================================================================

dlg_mp_score::dlg_mp_score( void )
{
    m_LoadingComplete = FALSE;
    m_ForceExit       = FALSE;
    m_LoadTimeElapsed = 0.0f;
}

//=========================================================================

dlg_mp_score::~dlg_mp_score( void )
{
}

//=========================================================================

xbool dlg_mp_score::Create( s32                       UserID,
                                 ui_manager*               pManager,
                                 ui_manager::dialog_tem*   pDialogTem,
                                 const irect&              Position,
                                 ui_win*                   pParent,
                                 s32                       Flags,
                                 void*                     pUserData )
{
    // default config
    m_Mode = LEADERBOARD_PAUSE;
    m_LoadingComplete = FALSE;
    m_LoadTimeElapsed = 0.0f;

    // The subclass should take care of everything else
    return ui_dialog::Create( UserID, pManager, pDialogTem, Position, pParent, Flags, pUserData );
}

//=========================================================================

void dlg_mp_score::Destroy( void )
{
    ui_dialog::Destroy();
}

//=========================================================================

void dlg_mp_score::OnUpdate( ui_win* pWin, f32 DeltaTime )
{
    (void)pWin;
    (void)DeltaTime;

    if( m_Mode == LEADERBOARD_INTERLEVEL )
        m_LoadTimeElapsed += DeltaTime;
    if( m_LoadingComplete )
        m_State = DIALOG_STATE_EXIT;
}

//=========================================================================

void dlg_mp_score::LoadingComplete( void )
{
    m_LoadingComplete = TRUE;
}

//=========================================================================

void dlg_mp_score::Configure( leaderboard_mode Mode )
{ 
    m_Mode = Mode; 
}

//=========================================================================
