//==============================================================================
//  
//  dlg_DemoMainMenu.hpp
//  
//==============================================================================

#ifndef DLG_DEMO_MAIN_MENU_HPP
#define DLG_DEMO_MAIN_MENU_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

enum demo_main_menu_controls
{
	IDC_DEMO_MAIN_MENU_LEVEL_ONE,
	IDC_DEMO_MAIN_MENU_LEVEL_TWO,
    IDC_DEMO_MAIN_MENU_LEVEL_THREE,
    IDC_DEMO_MAIN_MENU_NAV_TEXT,
};


//==============================================================================
//  dlg_main_menu
//==============================================================================

extern void     dlg_demo_main_menu_register  ( ui_manager* pManager );
extern ui_win*  dlg_demo_main_menu_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_demo_main_menu : public ui_dialog
{
public:
                        dlg_demo_main_menu       ( void );
    virtual            ~dlg_demo_main_menu       ( void );

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
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

protected:
    ui_frame*           m_pFrame1;
	ui_button*			m_pButtonLevelOne;
	ui_button*			m_pButtonLevelTwo; 		
	ui_button*			m_pButtonLevelThree; 		
    ui_text*            m_pNavText;

    s32                 m_CurrHL;
};

//==============================================================================
#endif // DLG_DEMO_MAIN_MENU_HPP
//==============================================================================
