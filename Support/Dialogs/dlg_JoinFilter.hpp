//==============================================================================
//  
//  dlg_JoinFilter.hpp
//  
//==============================================================================

#ifndef DLG_JOIN_FILTER_HPP
#define DLG_JOIN_FILTER_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_button.hpp"

#include "dialogs\dlg_popup.hpp"


//==============================================================================
//  dlg_join_filter
//==============================================================================

extern void     dlg_join_filter_register  ( ui_manager* pManager );
extern ui_win*  dlg_join_filter_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_join_filter : public ui_dialog
{
public:
                        dlg_join_filter ( void );
    virtual            ~dlg_join_filter ( void );

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

    void                OnSaveSettingsCB        ( void );

protected:
    ui_frame*           m_pFrame1;
    
    ui_text*            m_pGameTypeText;
    ui_text*            m_pNumPlayerText;
    ui_text*            m_pMutationModeText;
    ui_text*            m_pPasswordText;
    ui_text*            m_pHeadsetText;
    ui_text*            m_pNavText;

    ui_combo*           m_pGameTypeSelect;
    ui_combo*           m_pNumPlayerSelect;
    ui_combo*           m_pMutationSelect;
    ui_combo*           m_pPasswordSelect;
    ui_combo*           m_pHeadsetSelect;

    ui_button*          m_pContinueButton;

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;

    s32                 m_CurrHL;
};

//==============================================================================
#endif // DLG_JOIN_FILTER_HPP
//==============================================================================
