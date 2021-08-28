//==============================================================================
//  
//  dlg_StartGame.hpp
//  
//==============================================================================

#ifndef DLG_START_GAME_HPP
#define DLG_START_GAME_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"



//==============================================================================
//  dlg_start_game
//==============================================================================

extern void     dlg_start_game_register  ( ui_manager* pManager );
extern ui_win*  dlg_start_game_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_start_game : public ui_dialog
{
public:
                        dlg_start_game       ( void );
    virtual            ~dlg_start_game       ( void );

    xbool               Create              ( s32                       UserID,
                                              ui_manager*               pManager,
                                              ui_manager::dialog_tem*   pDialogTem,
                                              const irect&              Position,
                                              ui_win*                   pParent,
                                              s32                       Flags,
                                              void*                     pUserData);
    virtual void        Destroy             ( void );

    virtual void        Render              ( s32 ox=0, s32 oy=0 );
    
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

protected:
    ui_frame*           m_pFrame1;
    ui_text*            m_pLoadText;
    xbool               m_StartLoading;
    s32                 m_Countdown;
};

//==============================================================================
#endif // DLG_START_GAME_HPP
//==============================================================================
