//==============================================================================
//  
//  dlg_OnlineConnect.hpp
//  
//==============================================================================

#ifndef DLG_REPORT_ERROR_HPP
#define DLG_REPORT_ERROR_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_listbox.hpp"

#include "dialogs\dlg_popup.hpp"

#include "NetworkMgr/NetworkMgr.hpp"

//==============================================================================
//  dlg_report_error
//==============================================================================

extern void     dlg_report_error_register  ( ui_manager* pManager );
extern ui_win*  dlg_report_error_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_report_error : public ui_dialog
{
public:
                        dlg_report_error  ( void );
    virtual            ~dlg_report_error  ( void );
    xbool               Create              ( s32                       UserID,
                                              ui_manager*               pManager,
                                              ui_manager::dialog_tem*   pDialogTem,
                                              const irect&              Position,
                                              ui_win*                   pParent,
                                              s32                       Flags,
                                              void*                     pUserData);
    virtual void        Destroy             ( void );

    virtual void        Render              ( s32 ox=0, s32 oy=0 );

    virtual void        OnPadSelect         ( ui_win* pWin );
    virtual void        OnPadBack           ( ui_win* pWin );
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

protected:
    ui_text*            m_pNavText;
    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;
    xbool               m_CanTroubleshoot;
};

//==============================================================================
#endif // DLG_report_error_HPP
//==============================================================================
