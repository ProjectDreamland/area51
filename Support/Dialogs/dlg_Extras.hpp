//==============================================================================
//  
//  dlg_Extras.hpp
//  
//==============================================================================

#ifndef DLG_EXTRAS_HPP
#define DLG_EXTRAS_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_listbox.hpp"


//==============================================================================
//  dlg_extras
//==============================================================================

extern void     dlg_extras_register  ( ui_manager* pManager );
extern ui_win*  dlg_extras_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_extras : public ui_dialog
{
public:
                        dlg_extras       ( void );
    virtual            ~dlg_extras       ( void );

    xbool               Create              ( s32                       UserID,
                                              ui_manager*               pManager,
                                              ui_manager::dialog_tem*   pDialogTem,
                                              const irect&              Position,
                                              ui_win*                   pParent,
                                              s32                       Flags,
                                              void*                     pUserData);
    virtual void        Destroy             ( void );

    virtual void        Render              ( s32 ox=0, s32 oy=0 );

    virtual void        OnNotify            ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData );
    virtual void        OnPadNavigate       ( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX = FALSE, xbool WrapY = FALSE );
    virtual void        OnPadSelect         ( ui_win* pWin );
    virtual void        OnPadBack           ( ui_win* pWin );
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );
    
    void                FillExtrasList      ( void );

protected:
    ui_listbox*         m_pExtrasList;
    ui_text*            m_pNavText;
    s32                 m_CurrHL;
};

//==============================================================================
#endif // DLG_EXTRAS_HPP
//==============================================================================
