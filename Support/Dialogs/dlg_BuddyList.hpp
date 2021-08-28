//==============================================================================
//  
//  dlg_BuddyList.hpp
//  
//==============================================================================

#ifndef DLG_BUDDY_LIST_HPP
#define DLG_BUDDY_LIST_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_listbox.hpp"

#include "dlg_PopUp.hpp"

//==============================================================================
//  dlg_buddy_list
//==============================================================================

extern void     dlg_buddy_list_register  ( ui_manager* pManager );
extern ui_win*  dlg_buddy_list_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_buddy_list : public ui_dialog
{
public:
                        dlg_buddy_list ( void );
    virtual            ~dlg_buddy_list ( void );

    xbool               Create                  ( s32                       UserID,
                                                  ui_manager*               pManager,
                                                  ui_manager::dialog_tem*   pDialogTem,
                                                  const irect&              Position,
                                                  ui_win*                   pParent,
                                                  s32                       Flags,
                                                  void*                     pUserData);
    virtual void        Destroy                 ( void );

    virtual void        Render                  ( s32 ox=0, s32 oy=0 );

    virtual void        OnNotify                ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData );
    virtual void        OnPadNavigate           ( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX = FALSE, xbool WrapY = FALSE );
    virtual void        OnPadSelect             ( ui_win* pWin );
    virtual void        OnPadBack               ( ui_win* pWin );
    virtual void        OnPadDelete             ( ui_win* pWin );
    virtual void        OnPadActivate           ( ui_win* pWin );
    virtual void        OnUpdate                ( ui_win* pWin, f32 DeltaTime );

    void                RefreshBuddyList      ( void );

protected:
    ui_frame*           m_pFrame1;
    ui_listbox*         m_pBuddyList;

    ui_text*            m_pNavText;

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;

    s32                 m_CurrHL;

    xwstring            m_BuddyName;
    xbool               m_BuddyEntered;
    xbool               m_BuddyOk;
};

//==============================================================================
#endif // DLG_BUDDY_LIST_HPP
//==============================================================================
