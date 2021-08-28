//==============================================================================
//  
//  dlg_MainMenu.hpp
//  
//==============================================================================

#ifndef DLG_MAIN_MENU_HPP
#define DLG_MAIN_MENU_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"
#include "dlg_PopUp.hpp"
//==============================================================================
//  DEFINES
//==============================================================================

enum menu_menu_controls
{
    IDC_MAIN_MENU_CAMPAIGN,
    IDC_MAIN_MENU_MULTI,
    IDC_MAIN_MENU_ONLINE,
    IDC_MAIN_MENU_SETTINGS,
    IDC_MAIN_MENU_PROFILES,
    IDC_MAIN_MENU_CREDITS,
    IDC_MAIN_MENU_NAV_TEXT,
    IDC_MAIN_MENU_SIGN_IN,
    IDC_MAIN_MENU_SILENT_LOGIN_TEXT,
};


//==============================================================================
//  dlg_main_menu
//==============================================================================

extern void     dlg_main_menu_register  ( ui_manager* pManager );
extern ui_win*  dlg_main_menu_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_main_menu : public ui_dialog
{
public:
                        dlg_main_menu       ( void );
    virtual            ~dlg_main_menu       ( void );

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
    ui_button*          m_pButtonCampaign;
    ui_button*          m_pButtonMultiPlayer;       
    ui_button*          m_pButtonOnline;    
    ui_button*          m_pButtonSettings;
    ui_button*          m_pButtonProfiles;
    ui_button*          m_pButtonCredits;
    ui_text*            m_pNavText;
    ui_text*            m_pSilentLoginText;

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;

    s32                 m_CurrHL;

    xbool               m_bCheckKeySequence;
};

//==============================================================================
#endif // DLG_MAIN_MENU_HPP
//==============================================================================
