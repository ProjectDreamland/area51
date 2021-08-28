//==============================================================================
//  
//  dlg_CampaignMenu.hpp
//  
//==============================================================================

#ifndef DLG_CAMPAIGN_MENU_HPP
#define DLG_CAMPAIGN_MENU_HPP

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

enum campaign_menu_controls
{
    IDC_CAMPAIGN_MENU_NEW_CAMPAIGN,
    IDC_CAMPAIGN_MENU_RESUME_CAMPAIGN,
    IDC_CAMPAIGN_MENU_EDIT_PROFILE,
    IDC_CAMPAIGN_MENU_LORE,
    IDC_CAMPAIGN_MENU_SECRETS,
    IDC_CAMPAIGN_MENU_EXTRAS,
#ifndef CONFIG_RETAIL
    IDC_CAMPAIGN_MENU_LEVEL_SELECT,
#endif
    IDC_CAMPAIGN_MENU_NAV_TEXT,
};


//==============================================================================
//  dlg_campaign_menu
//==============================================================================

extern void     dlg_campaign_menu_register  ( ui_manager* pManager );
extern ui_win*  dlg_campaign_menu_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_campaign_menu : public ui_dialog
{
public:
                        dlg_campaign_menu       ( void );
    virtual            ~dlg_campaign_menu       ( void );

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
    virtual void        OnPadBack           ( ui_win* pWin );
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

protected:
    ui_frame*           m_pFrame1;
    ui_button*          m_pButtonNewCampaign;
    ui_button*          m_pButtonResumeCampaign;
    ui_button*          m_pButtonEditProfile; 
    ui_button*          m_pButtonLore;
    ui_button*          m_pButtonSecrets;
    ui_button*          m_pButtonExtras;
#ifndef CONFIG_RETAIL
    ui_button*          m_pButtonLevelSelect;       
#endif
    ui_text*            m_pNavText;

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;

    s32                 m_CurrHL;

    xbool               m_bCheckKeySequence;
};

//==============================================================================
#endif // DLG_CAMPAIGN_MENU_HPP
//==============================================================================
