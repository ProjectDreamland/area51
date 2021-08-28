//==============================================================================
//  
//  dlg_MemcardSelect.hpp
//  
//==============================================================================

#ifndef DLG_MEMCARD_SELECT_HPP
#define DLG_MEMCARD_SELECT_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_blankbox.hpp"
#include "ui\ui_button.hpp"
#include "dlg_PopUp.hpp"

#include "StateMgr\StateMgr.hpp"

//==============================================================================
//  dlg_memcard_select
//==============================================================================

extern void     dlg_memcard_select_register  ( ui_manager* pManager );
extern ui_win*  dlg_memcard_select_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_memcard_select : public ui_dialog
{
public:
                        dlg_memcard_select      ( void );
    virtual            ~dlg_memcard_select      ( void );

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
    virtual void        OnPadActivate           ( ui_win* pWin );
    virtual void        OnUpdate                ( ui_win* pWin, f32 DeltaTime );
    
    void                OnPollReturn            ( void );
    void                OnProfileCreateCB       ( void );
    void                OnCreateSettingsCB      ( void );
    void                OnFormatCB              ( void );
    void                UpdateCardMessageText   ( void );

    void                EnableBlackout          ( void )                    { m_bRenderBlackout = TRUE; }
    void                Configure               ( card_data_mode mode, s32 size = 0 );

protected:
    ui_frame*           m_pFrame1;

    // load/save info boxes
    ui_blankbox*        m_pInfoBox1;
    ui_blankbox*        m_pInfoBox2;
    ui_blankbox*        m_pMessageBox;

    ui_text*            m_pStatusText1;
    ui_text*            m_pStatusText2;
    ui_text*            m_pInfoText;
    ui_text*            m_pMessageText;

    ui_button*          m_pMemcardOne;
    ui_button*          m_pMemcardTwo;

    ui_text*            m_pNavText;

    s32                 m_CurrHL;
    s32                 m_ActiveCard;

    s32                 m_MemcardIconID[4];

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;

    xbool               m_bRenderBlackout;

    card_data_mode    m_Mode;
    s32                 m_Size;
};

//==============================================================================
#endif // DLG_MEMCARD_SELECT_HPP
//==============================================================================
