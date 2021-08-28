//==============================================================================
//  
//  dlg_EndPause.hpp
//  
//==============================================================================

#ifndef DLG_END_PAUSE_HPP
#define DLG_END_PAUSE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"



//==============================================================================
//  dlg_end_pause
//==============================================================================

extern void     dlg_end_pause_register  ( ui_manager* pManager );
extern ui_win*  dlg_end_pause_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_end_pause : public ui_dialog
{
public:
                        dlg_end_pause       ( void );
    virtual            ~dlg_end_pause       ( void );

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
    ui_text*            m_pWaitText;
    xbool               m_StartWaiting;
    s32                 m_Countdown;
};

//==============================================================================
#endif // DLG_END_PAUSE_HPP
//==============================================================================
