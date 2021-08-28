// 
// 
// 
// dlg_esrb.hpp
// Thu Nov 21 09:45:59 2002
//
//

#ifndef __DLG_ESRB__ 
#define __DLG_ESRB__ 

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"

#include "Obj_mgr\obj_mgr.hpp"

//==============================================================================
//  dlg_esrb
//==============================================================================

extern void     dlg_esrb_register  ( ui_manager* pManager );
extern ui_win*  dlg_esrb_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class dlg_esrb : public ui_dialog
{
public:
                        dlg_esrb            ( void );
    virtual            ~dlg_esrb            ( void );

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
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );
	virtual void		OnPadHelp			( ui_win* pWin );

protected:
	ui_text*			m_pESRBText;
    f32                 m_WaitTime;
};

#endif // DLG_ESRB
