//==============================================================================
//  
//  dlg_MultiOptions.hpp
//  
//==============================================================================

#ifndef DLG_MULTI_OPTIONS_HPP
#define DLG_MULTI_OPTIONS_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_edit.hpp"

#include "dialogs\dlg_popup.hpp"



//==============================================================================
//  dlg_multi_options
//==============================================================================

extern void     dlg_multi_options_register  ( ui_manager* pManager );
extern ui_win*  dlg_multi_options_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_multi_options : public ui_dialog
{
public:
                        dlg_multi_options       ( void );
    virtual            ~dlg_multi_options       ( void );

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

    void                ResetMapCycle           ( void );
    void                InitializeMutationModes ( void );

protected:
    ui_frame*           m_pFrame1;
    
    ui_text*            m_pGameTypeText;
    ui_text*            m_pTimeLimitText;
    ui_text*            m_pMutationText;
    ui_text*            m_pScoreText;
    ui_text*            m_pNavText;

    ui_combo*           m_pGameTypeSelect;
    ui_combo*           m_pTimeSelect;
    ui_combo*           m_pMutationSelect;
    ui_combo*           m_pScoreSelect;

    ui_button*          m_pContinueButton;

    dlg_popup*          m_PopUp;

    s32                 m_CurrHL;

    s32                 m_CurrGameType;
};

//==============================================================================
#endif // DLG_MULTI_OPTIONS_HPP
//==============================================================================
