//==============================================================================
//  
//  dlg_Stats.hpp
//  
//==============================================================================

#ifndef DLG_STATS_HPP
#define DLG_STATS_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"

//==============================================================================
//  dlg_stats
//==============================================================================

extern void     dlg_stats_register  ( ui_manager* pManager );
extern ui_win*  dlg_stats_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_stats : public ui_dialog
{
public:
                        dlg_stats ( void );
    virtual            ~dlg_stats ( void );

    xbool               Create                  ( s32                       UserID,
                                                  ui_manager*               pManager,
                                                  ui_manager::dialog_tem*   pDialogTem,
                                                  const irect&              Position,
                                                  ui_win*                   pParent,
                                                  s32                       Flags,
                                                  void*                     pUserData);
    virtual void        Destroy                 ( void );

    virtual void        Render                  ( s32 ox=0, s32 oy=0 );

    virtual void        OnPadBack               ( ui_win* pWin );
    virtual void        OnUpdate                ( ui_win* pWin, f32 DeltaTime );

protected:
    ui_frame*           m_pFrame1;

    ui_text*            m_pLabelTime;
    ui_text*            m_pLabelKills;
    ui_text*            m_pLabelDeaths;
    ui_text*            m_pLabelGames;
    ui_text*            m_pLabelWins;
    ui_text*            m_pLabelGolds;
    ui_text*            m_pLabelSilvers;
    ui_text*            m_pLabelBronzes;
#ifndef TARGET_XBOX
    ui_text*            m_pLabelKicks;
    ui_text*            m_pLabelVotes;
#endif

    ui_text*            m_pTextTime;
    ui_text*            m_pTextKills;
    ui_text*            m_pTextDeaths;
    ui_text*            m_pTextGames;
    ui_text*            m_pTextWins;
    ui_text*            m_pTextGolds;
    ui_text*            m_pTextSilvers;
    ui_text*            m_pTextBronzes;
#ifndef TARGET_XBOX
    ui_text*            m_pTextKicks;
    ui_text*            m_pTextVotes;
#endif

    ui_text*            m_pNavText;
    s32                 m_CurrHL;
};

//==============================================================================
#endif // DLG_STATS_HPP
//==============================================================================
