//==============================================================================
//  
//  dlg_Autosave.hpp
//  
//==============================================================================

#ifndef DLG_AUTOSAVE_HPP
#define DLG_AUTOSAVE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"

#include "dlg_PopUp.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

enum autosave_controls
{
    IDC_AUTOSAVE_NAV_TEXT,
};

//==============================================================================
//  dlg_autosave
//==============================================================================

extern void     dlg_autosave_register  ( ui_manager* pManager );
extern ui_win*  dlg_autosave_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class dlg_autosave : public ui_dialog
{
public:
                        dlg_autosave       ( void );
    virtual            ~dlg_autosave       ( void );

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

    void                SetCard             ( s32 CardID )              { m_iCard = CardID; }

protected:
    ui_frame*           m_pFrame1;

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;

    s32                 m_iCard;
#ifdef TARGET_XBOX
    s32                 m_BlocksRequired;
#endif
};

//==============================================================================
#endif // DLG_AUTOSAVE_HPP
//==============================================================================
