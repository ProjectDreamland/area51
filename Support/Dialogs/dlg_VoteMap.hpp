//==============================================================================
//  
//  dlg_VoteMap.hpp
//  
//==============================================================================

#ifndef DLG_VOTE_MAP_HPP
#define DLG_VOTE_MAP_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_maplist.hpp"

#include "dlg_PopUp.hpp"

//==============================================================================
//  dlg_vote_map
//==============================================================================

extern void     dlg_vote_map_register  ( ui_manager* pManager );
extern ui_win*  dlg_vote_map_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_vote_map : public ui_dialog
{
public:
                        dlg_vote_map ( void );
    virtual            ~dlg_vote_map ( void );

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

protected:
    ui_frame*           m_pFrame1;
    ui_maplist*         m_pMapList;

    ui_text*            m_pNavText;

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;

    s32                 m_CurrHL;
};

//==============================================================================
#endif // DLG_VOTE_MAP_HPP
//==============================================================================
