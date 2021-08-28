//==============================================================================
//  
//  dlg_COPA.hpp
//  
//==============================================================================

#ifndef DLG_COPA_HPP
#define DLG_COPA_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_text.hpp"

#include "dlg_PopUp.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
//  dlg_COPA
//==============================================================================

extern void     dlg_copa_register  ( ui_manager* pManager );
extern ui_win*  dlg_copa_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_copa : public ui_dialog
{
public:
                        dlg_copa  ( void );
    virtual            ~dlg_copa  ( void );

    xbool               Create              ( s32                       UserID,
                                              ui_manager*               pManager,
                                              ui_manager::dialog_tem*   pDialogTem,
                                              const irect&              Position,
                                              ui_win*                   pParent,
                                              s32                       Flags,
                                              void*                     pUserData);
    virtual void        Destroy             ( void );
    void                Close               ( void );

    virtual void        Render              ( s32 ox=0, s32 oy=0 );

    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );
    virtual void        OnNotify            ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData );
    virtual void        OnPadSelect         ( ui_win* pWin );
    virtual void        OnPadBack           ( ui_win* pWin );

    void                OnYearChange        ( void );
    void                OnMonthChange       ( void );

    xbool               VerifyAge           ( void );
    void                OnSaveProfileCB     ( void );
protected:
    ui_text*            m_pMessageText;
    ui_text*            m_pNavText;

    ui_text*            m_pMonthText;
    ui_text*            m_pDayText;
    ui_text*            m_pYearText;

    ui_combo*           m_pMonthCombo;
    ui_combo*           m_pDayCombo;
    ui_combo*           m_pYearCombo;

    ui_button*          m_pButtonAccept; 

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;
    s32                 m_PopUpType;
    s32                 m_iCard;

    xwstring            m_Message;
    s32                 m_CurrHL;
};

//==============================================================================
#endif // DLG_COPA_HPP
//==============================================================================
