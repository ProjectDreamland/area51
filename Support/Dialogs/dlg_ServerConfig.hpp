//==============================================================================
//  
//  dlg_ServerConfig.hpp
//  
//==============================================================================

#ifndef DLG_SERVER_CONFIG_HPP
#define DLG_SERVER_CONFIG_HPP

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

enum server_config_controls
{
    IDC_SERVER_CONFIG_CHANGE_MAP,
    IDC_SERVER_CONFIG_KICK_PLAYER,
    IDC_SERVER_CONFIG_CHANGE_TEAM,
    IDC_SERVER_CONFIG_RESTART_MAP,
    IDC_SERVER_CONFIG_RECONFIGURE,
    IDC_SERVER_CONFIG_SHUTDOWN,
    IDC_SERVER_CONFIG_NAV_TEXT,
};

//==============================================================================
//  dlg_server_config
//==============================================================================

extern void     dlg_server_config_register  ( ui_manager* pManager );
extern ui_win*  dlg_server_config_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_server_config : public ui_dialog
{
public:
                        dlg_server_config       ( void );
    virtual            ~dlg_server_config       ( void );

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
	ui_button*			m_pButtonChangeMap; 	
    ui_button*			m_pButtonKickPlayer; 	
    ui_button*			m_pButtonChangeTeam; 	
    ui_button*			m_pButtonRestartMap; 	
    ui_button*			m_pButtonReconfigure; 	
	ui_button*			m_pButtonShutdown; 	
    ui_text*            m_pNavText;

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;

    s32                 m_CurrHL;
};

//==============================================================================
#endif // DLG_SERVER_CONFIG_HPP
//==============================================================================
