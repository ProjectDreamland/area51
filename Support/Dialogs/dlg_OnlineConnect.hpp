//==============================================================================
//  
//  dlg_OnlineConnect.hpp
//  
//==============================================================================

#ifndef DLG_ONLINE_CONNECT_HPP
#define DLG_ONLINE_CONNECT_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_listbox.hpp"

#include "dialogs\dlg_popup.hpp"

#include "NetworkMgr/NetworkMgr.hpp"

//==============================================================================
//  dlg_online_connect
//==============================================================================
enum connect_states
{
    CONNECT_IDLE = 0,
    CONNECT_INIT,
    CONNECT_WAIT,
    SIGN_IN_INIT,
    SIGN_IN_WAIT,
    CONFIG_INIT,
    CONNECT_AUTHENTICATE_MACHINE,
    CONNECT_SELECT_USER,
    ACTIVATE_INIT,
    ACTIVATE_SELECT,
    ACTIVATE_LOOP,
    ACTIVATE_WAIT_DHCP,
    CONNECT_MATCH_INIT,
    CONNECT_AUTHENTICATE_USER,
    CONNECT_FAILED,
    CONNECT_FAILED_WAIT,
    CONNECT_AUTO_UPDATE,
    CONNECT_AUTO_UPDATE_WAIT,
    CONNECT_REQUIRED_MESSAGE,
    CONNECT_REQUIRED_MESSAGE_WAIT,
    CONNECT_OPTIONAL_MESSAGE,
    CONNECT_OPTIONAL_MESSAGE_WAIT,
    CONNECT_DONE,
    CONNECT_DONE_WAIT,
    CONNECT_DISCONNECT,
    CONNECT_CHECK_MOTD,
    CONNECT_DISPLAY_MOTD,
    NUM_CONNECT_STATES,
};

enum connect_mode
{
    CONNECT_MODE_CONNECT,
    CONNECT_MODE_AUTH_USER,
};

enum dnas_logos
{
    LOGO_DNAS_OK = 0,
    LOGO_DNAS_ERROR,
    NUM_DNAS_LOGOS,
};

enum cancel_mode
{
    CANCEL_MANAGE,
    OK_ONLY,
    CANCEL_RETRY_MANAGE,

};

extern void     dlg_online_connect_register  ( ui_manager* pManager );
extern ui_win*  dlg_online_connect_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_online_connect : public ui_dialog
{
public:
                        dlg_online_connect  ( void );
    virtual            ~dlg_online_connect  ( void );
    xbool               Create              ( s32                       UserID,
                                              ui_manager*               pManager,
                                              ui_manager::dialog_tem*   pDialogTem,
                                              const irect&              Position,
                                              ui_win*                   pParent,
                                              s32                       Flags,
                                              void*                     pUserData);
    virtual void        Destroy             ( void );
    virtual void        Configure           ( connect_mode ConnectMode );

    virtual void        Render              ( s32 ox=0, s32 oy=0 );

    virtual void        OnPadSelect         ( ui_win* pWin );
    virtual void        OnPadBack           ( ui_win* pWin );
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

    void                RefreshUserList     ( void );

protected:
    void                Failed              ( const char* pFailureReason, s32 ErrorCode=0, cancel_mode CancelMode = CANCEL_MANAGE, connect_states RetryDestination = CONNECT_MATCH_INIT );
    void                SetConnectState     ( connect_states State );
    const char*         StateName           ( connect_states State );
    void                UpdateConnectInit   ( void );
    void                UpdateActivateInit  ( void );
    void                UpdateAuthMachine   ( void );
    void                UpdateAuthUser      ( void );



    s32                 PopulateConfigurationList( void );

    ui_frame*           m_pFrame1;
    ui_listbox*         m_pUserList;
    xbool               m_AccountsHaveChanged;
    ui_text*            m_pNavText;

    ui_listbox*         m_pConfigList;

    connect_states      m_ConnectState;
    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;

    dlg_popup*          m_PopUpConfirmPassword;
    s32                 m_PopUpConfirmPasswordResult;
    xbool               m_bAskForPassword;

    // network config
    f32                 m_Timeout;
    interface_info      m_Info;

    s32                 m_Status;
    s32                 m_Error;
    char                m_LabelText[256];     // String ID of reason for failure.
    xbool               m_Done;
    net_address         m_Broadcast;
    irect               m_Position;

    s32                 m_NumUsers;
    s32                 m_LastErrorSlot;
    s32                 m_LastErrorCode;
    s32                 m_RenderDelay;

    cancel_mode         m_CancelMode;
    connect_states      m_RetryDestination;

    // DNAS logo controls
    s32                 m_DNASIconID[NUM_DNAS_LOGOS];
    xbool               m_bRenderLogo[NUM_DNAS_LOGOS];
    xbool               m_bIsFading[NUM_DNAS_LOGOS];
    xbool               m_bFadeIn[NUM_DNAS_LOGOS];
    f32                 m_FadeStep[NUM_DNAS_LOGOS];
    f32                 m_LogoAlpha[NUM_DNAS_LOGOS];

    
};

//==============================================================================
#endif // DLG_ONLINE_CONNECT_HPP
//==============================================================================