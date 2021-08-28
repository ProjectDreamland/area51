//==============================================================================
//  
//  dlg_SecretsMenu.hpp
//  
//==============================================================================

#ifndef DLG_SECRETS_MENU_HPP
#define DLG_SECRETS_MENU_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_blankbox.hpp"
#include "ui\ui_textbox.hpp"

#include "StateMgr/SecretList.hpp"

#include "dialogs\dlg_popup.hpp"

//==============================================================================
//  dlg_secrets_menu
//==============================================================================

extern void     dlg_secrets_menu_register  ( ui_manager* pManager );
extern ui_win*  dlg_secrets_menu_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_secrets_menu : public ui_dialog
{
public:
                        dlg_secrets_menu       ( void );
    virtual            ~dlg_secrets_menu       ( void );

    xbool               Create              ( s32                       UserID,
                                              ui_manager*               pManager,
                                              ui_manager::dialog_tem*   pDialogTem,
                                              const irect&              Position,
                                              ui_win*                   pParent,
                                              s32                       Flags,
                                              void*                     pUserData);
    virtual void        Destroy             ( void );

    virtual void        Render              ( s32 ox=0, s32 oy=0 );

    virtual void        OnNotify            ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData );
    virtual void        OnPadNavigate       ( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX = FALSE, xbool WrapY = FALSE );
    virtual void        OnPadSelect         ( ui_win* pWin );
    virtual void        OnPadBack           ( ui_win* pWin );
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

    void                InitIconScaling     ( xbool ScaleDown );
    xbool               UpdateIconScaling   ( f32 DeltaTime );

    void                PopulateSecretsDetails ( xbool bVaultDetails );

protected:
    ui_frame*               m_pFrame1;

    ui_blankbox*            m_pSecretsMain;
    ui_blankbox*            m_pSecretsDetails;
    ui_blankbox*            m_pBlackOut;

    ui_combo*               m_pSecretsSelect;

    ui_button*              m_pSecretsButton[5];

    ui_text*                m_pSecretsLine1;
    ui_text*                m_pSecretsLine2;
    ui_text*                m_pSecretsLine3;

    ui_textbox*             m_pTextBox;

    ui_text*                m_pNavText;

    dlg_popup*              m_PopUp;

    s32                     m_SecretsIconID[5];

    s32                     m_CurrHL;

    f32                     m_ScreenScaleX;
    f32                     m_ScreenScaleY;

#ifdef TARGET_PS2
    xbool                   m_bPalMode;
#endif

    // icon scaling controls
    irect                   m_DrawPos;
    irect                   m_RequestedPos;
    irect                   m_StartPos;
    irect                   m_DiffPos;
    irect                   m_TotalMoved;
    f32                     m_scaleX;
    f32                     m_scaleY;
    f32                     m_totalX;
    f32                     m_scaleCount;
    f32                     m_scaleAngle;
    xbool                   m_bScreenIsOn;
    xbool                   m_bScaleDown;
    ui_button*              m_pSelectedIcon;

    // secrets list related
    s32                     m_SelectedIndex;
    //secrets_vault*          m_pSelectedVault;
    secret_type             m_CurrentType;
    //s32                     m_VaultIndex;
    char                    m_FileName[32];
    char                    m_FullDesc[32];
    s32                     m_NumItems;
    s32                     m_CurrItem;
    s32                     m_StillBitmapID;

    // fade controls
    u8                      m_FadeLevel;
    f32                     m_TimeOut;
};

//==============================================================================
#endif // DLG_SECRETS_MENU_HPP
//==============================================================================
