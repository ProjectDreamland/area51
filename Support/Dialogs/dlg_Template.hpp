//==============================================================================
//  
//  dlg_Template.hpp
//  
//==============================================================================

#ifndef DLG_TEMPLATE_HPP
#define DLG_TEMPLATE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"



//==============================================================================
//  dlg_template
//==============================================================================

extern void     dlg_template_register  ( ui_manager* pManager );
extern ui_win*  dlg_template_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_template : public ui_dialog
{
public:
                        dlg_template       ( void );
    virtual            ~dlg_template       ( void );

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
    virtual void        OnPadHelp           ( ui_win* pWin ){ OnPadSelect( pWin );}
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

protected:
    ui_frame*           m_pFrame1;
	ui_button*			m_pButtonOne;
	ui_button*			m_pButtonTwo; 		
	ui_button*			m_pButtonThree; 	
    s32                 m_CurrHL;
    irect               m_CurrPos;
    irect               m_RequestedPos;
    s32                 m_scaleX;
    s32                 m_scaleY;
    s32                 m_scaleCount;
};

//==============================================================================
#endif // DLG_TEMPLATE_HPP
//==============================================================================
