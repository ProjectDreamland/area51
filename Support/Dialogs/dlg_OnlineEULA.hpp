//==============================================================================
//  
//  dlg_OnlineEULA.hpp
//  
//==============================================================================

#ifndef DLG_ONLINE_EULA_HPP
#define DLG_ONLINE_EULA_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_textbox.hpp"


//==============================================================================
//  dlg_online_eula
//==============================================================================

extern void     dlg_online_eula_register  ( ui_manager* pManager );
extern ui_win*  dlg_online_eula_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class dlg_online_eula : public ui_dialog
{
public:
                        dlg_online_eula       ( void );
    virtual            ~dlg_online_eula       ( void );

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
    ui_textbox*         m_pEULATextBox;
    ui_text*            m_pNavText;
    s32                 m_CurrHL;
    f32                 m_Timeout;
};

//==============================================================================
#endif // DLG_ONLINE_EULA_HPP
//==============================================================================
