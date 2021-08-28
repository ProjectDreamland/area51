//==============================================================================
//  
//  dlg_OnlineLogin.hpp
//  
//==============================================================================

#ifndef DLG_ONLINE_LOGIN_HPP
#define DLG_ONLINE_LOGIN_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_listbox.hpp"
#include "ui\ui_blankbox.hpp"

#include "dialogs\dlg_popup.hpp"

#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"

//==============================================================================
//  dlg_online_login
//==============================================================================
enum dlg_login_state
{
    LOGIN_IDLE,
    LOGIN_INITIATE_CONNECTION,
    LOGIN_WAIT_FOR_LOGIN,
    LOGIN_ERROR,
    LOGIN_ACQUIRE_PASSWORD,
    LOGIN_PASSWORD_ERROR,
};


extern void     dlg_online_login_register  ( ui_manager* pManager );
extern ui_win*  dlg_online_login_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_online_login : public ui_dialog
{
public:
                        dlg_online_login       ( void );
    virtual            ~dlg_online_login       ( void );

    xbool               Create                  ( s32                       UserID,
                                                  ui_manager*               pManager,
                                                  ui_manager::dialog_tem*   pDialogTem,
                                                  const irect&              Position,
                                                  ui_win*                   pParent,
                                                  s32                       Flags,
                                                  void*                     pUserData);
    virtual void        Destroy                 ( void );

    virtual void        Render                  ( s32 ox=0, s32 oy=0 );

    virtual void        OnPadSelect             ( ui_win* pWin );
    virtual void        OnPadBack               ( ui_win* pWin );
    virtual void        OnUpdate                ( ui_win* pWin, f32 DeltaTime );
    
protected:
    void                SetLoginState           ( dlg_login_state NewState );
    void                EnterState              ( dlg_login_state State );
    void                ExitState               ( dlg_login_state State );
    void                UpdateState             ( f32 DeltaTime );
    void                OkDialog                ( const char* pString, const char* pButton = "IDS_NAV_OK" );
    void                ProgressDialog          ( const char* pString, xbool AllowBackout = TRUE );
    void                CloseDialog             ( void );
    const char*         GetStateName            ( dlg_login_state LoginState );
    void                SetState                ( dialog_states State );
    void                UpdateInitConnection    ( void );
    void                UpdateWaitForLogin      ( void );
    void                UpdateAcquirePassword   ( void );

    ui_text*            m_pNavText;

    dlg_popup*          m_pPopup;
    s32                 m_PopUpResult;
    dlg_login_state     m_LoginState;
    xwstring            m_ServerName;
    xbool               m_HasStartedConnect;
    xbool               m_JoinPasswordDone;
    xbool               m_JoinPasswordOK;
    xbool               m_FirstTimePassword;


    xwstring            m_JoinPassword;
};

//==============================================================================
#endif // dlg_online_login_HPP
//==============================================================================
