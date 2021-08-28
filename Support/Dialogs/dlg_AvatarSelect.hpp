//==============================================================================
//  
//  dlg_AvatarSelect.hpp
//  
//==============================================================================

#ifndef DLG_AVATAR_SELECT_HPP
#define DLG_AVATAR_SELECT_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"

//==============================================================================
//  dlg_avatar_select
//==============================================================================

extern void     dlg_avatar_select_register  ( ui_manager* pManager );
extern ui_win*  dlg_avatar_select_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_avatar_select : public ui_dialog
{
public:
                        dlg_avatar_select ( void );
    virtual            ~dlg_avatar_select ( void );

    xbool               Create                  ( s32                       UserID,
                                                  ui_manager*               pManager,
                                                  ui_manager::dialog_tem*   pDialogTem,
                                                  const irect&              Position,
                                                  ui_win*                   pParent,
                                                  s32                       Flags,
                                                  void*                     pUserData);
    virtual void        Destroy                 ( void );

    virtual void        Render                  ( s32 ox=0, s32 oy=0 );

    virtual void        OnPadNavigate           ( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX = FALSE, xbool WrapY = FALSE );
    virtual void        OnPadBack               ( ui_win* pWin );
    virtual void        OnUpdate                ( ui_win* pWin, f32 DeltaTime );
    virtual void        OnPadSelect             ( ui_win* pWin );

    void                EnableBlackout          ( void )                    { m_bRenderBlackout = TRUE; }
protected:
    ui_frame*           m_pFrame1;
    ui_combo*           m_pAvatarSelect;

    ui_text*            m_pNavText;

    s32                 m_CurrHL;

    xbool               m_bRenderBlackout;
};

//==============================================================================
#endif // DLG_AVATAR_SELECT_HPP
//==============================================================================
