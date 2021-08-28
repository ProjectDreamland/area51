//==============================================================================
//  
//  ui_dlg_list.hpp
//  
//==============================================================================

#ifndef UI_DLG_LIST_HPP
#define UI_DLG_LIST_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui_dialog.hpp"

//==============================================================================
//  ui_dlg_list
//==============================================================================

extern void ui_dlg_list_register( ui_manager* pManager );
extern ui_win* ui_dlg_list_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_listbox;

class ui_dlg_list : public ui_dialog
{
public:
                    ui_dlg_list         ( void );
    virtual        ~ui_dlg_list         ( void );

    xbool           Create              ( s32                       UserID,
                                          ui_manager*               pManager,
                                          ui_manager::dialog_tem*   pDialogTem,
                                          const irect&              Position,
                                          ui_win*                   pParent,
                                          s32                       Flags,
                                          void*                     pUserData );

    virtual void    Render              ( s32 ox=0, s32 oy=0 );

    virtual void    OnCursorMove        ( ui_win* pWin, s32 x, s32 y );
    virtual void    OnLBDown            ( ui_win* pWin );
    virtual void    OnNotify            ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData );

    ui_listbox*     GetListBox          ( void );
    void            SetResultPtr        ( s32* pResultPtr );

protected:
    ui_listbox*     m_pList;
    s32*            m_pResultPtr;
#ifdef TARGET_PC
    xbool           m_InsideListBox;        // Check if the mouse cursor is not inside the dialog
    s32             m_UserID;
#endif
};

//==============================================================================
#endif // UI_DLG_LIST_HPP
//==============================================================================
