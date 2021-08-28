//==============================================================================
//  
//  dlg_ProfileName.hpp
//  
//==============================================================================

#ifndef DLG_PROFILE_NAME_HPP
#define DLG_PROFILE_NAME_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_dlg_vkeyboard.hpp"

#include "dlg_PopUp.hpp"

//==============================================================================
//  dlg_profile_name
//==============================================================================

extern void     dlg_profile_name_register  ( ui_manager* pManager );
extern ui_win*  dlg_profile_name_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_profile_name : public ui_dialog
{
public:
                        dlg_profile_name ( void );
    virtual            ~dlg_profile_name ( void );

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
    virtual void        OnPadSelect             ( ui_win* pWin );
    virtual void        OnPadBack               ( ui_win* pWin );
    virtual void        OnUpdate                ( ui_win* pWin, f32 DeltaTime );

    void                EnableBlackout          ( void )                    { m_bRenderBlackout = TRUE;}

protected:
    ui_frame*           m_pFrame1;

    ui_dlg_vkeyboard*   m_pVKeyboard;

    ui_text*            m_pNavText;

    s32                 m_CurrHL;

    xwstring            m_ProfileName;
    xbool               m_ProfileEntered;
    xbool               m_ProfileOK;

    xbool               m_bRenderBlackout;
};

//==============================================================================
#endif // DLG_PROFILE_NAME_HPP
//==============================================================================
