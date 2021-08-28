//==============================================================================
//  
//  dlg_ProfileAV.hpp
//  
//==============================================================================

#ifndef DLG_PROFILE_AV_HPP
#define DLG_PROFILE_AV_HPP

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

//==============================================================================
//  dlg_profile_av
//==============================================================================

extern void     dlg_profile_av_register  ( ui_manager* pManager );
extern ui_win*  dlg_profile_av_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_profile_av : public ui_dialog
{
public:
                        dlg_profile_av       ( void );
    virtual            ~dlg_profile_av       ( void );

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
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

    void                EnableBlackout      ( void )                    { m_bRenderBlackout = TRUE; }
    void                EnableHeadset       ( void );

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
    ui_button*          m_pRestoreDefaults;

    ui_text*            m_pNavText;

    s32                 m_CurrHL;

    xbool               m_bRenderBlackout;

    player_profile      m_Profile;
};

//==============================================================================
#endif // DLG_PROFILE_AV_HPP
//==============================================================================
