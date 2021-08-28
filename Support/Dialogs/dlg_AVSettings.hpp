//==============================================================================
//  
//  dlg_AVSettings.hpp
//  
//==============================================================================

#ifndef DLG_AV_SETTINGS_HPP
#define DLG_AV_SETTINGS_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_slider.hpp"
#include "ui\ui_check.hpp"
#include "ui\ui_button.hpp"

#include "StateMgr\StateMgr.hpp"
#include "dlg_PopUp.hpp"
#include "dlg_AVSettings.hpp"

//==============================================================================
//  dlg_av_settings
//==============================================================================

extern void     dlg_av_settings_register  ( ui_manager* pManager );
extern ui_win*  dlg_av_settings_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_av_settings : public ui_dialog
{
public:
                        dlg_av_settings       ( void );
    virtual            ~dlg_av_settings       ( void );

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
    virtual void        OnPadSelect         ( ui_win* pWin );
    virtual void        OnPadBack           ( ui_win* pWin );
    virtual void        OnPadDelete         ( ui_win* pWin );
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

    void                EnableBlackout      ( void )                    { m_bRenderBlackout = TRUE; }

    void                OnSaveSettingsCB    ( void );

protected:
    ui_frame*           m_pFrame1;

    ui_slider*          m_pVolumeSFX;
    ui_slider*          m_pVolumeMusic;
    ui_slider*          m_pVolumeSpeech;
    ui_slider*          m_pBrightness;

    ui_text*            m_pVolumeSFXText;
    ui_text*            m_pVolumeMusicText;
    ui_text*            m_pVolumeSpeechText;
    ui_text*            m_pBrightnessText;

#ifdef TARGET_PS2
    ui_combo*           m_pToggleSpeakerSetup;
    ui_text*            m_pToggleSpeakerSetupText;
#endif

    ui_button*          m_pHeadsetTest;
    ui_button*          m_pButtonAccept;

    ui_text*            m_pNavText;

    s32                 m_CurrHL;

    xbool               m_bRenderBlackout;

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;
    s32                 m_PopUpType;

    global_settings     m_Settings;
};

//==============================================================================
#endif // DLG_AV_SETTINGS_HPP
//==============================================================================
