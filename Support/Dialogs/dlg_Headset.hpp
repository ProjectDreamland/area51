//==============================================================================
//  
//  dlg_Headset.hpp
//  
//==============================================================================

#ifndef DLG_HEADSET_HPP
#define DLG_HEADSET_HPP

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
//  dlg_headset
//==============================================================================

extern void     dlg_headset_register  ( ui_manager* pManager );
extern ui_win*  dlg_headset_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_headset : public ui_dialog
{
public:
                        dlg_headset         ( void );
    virtual            ~dlg_headset         ( void );

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
    virtual void        OnPadDelete         ( ui_win* pWin );
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

    void                EnableBlackout      ( void )                    { m_bRenderBlackout = TRUE; }

protected:
    ui_frame*           m_pFrame1;

    ui_combo*           m_pToggleHeadsetAudio;

    ui_slider*          m_pVolumeSpeaker;
    ui_slider*          m_pVolumeMic;

    ui_text*            m_pToggleHeadsetAudioText;
    ui_text*            m_pVolumeSpeakerText;
    ui_text*            m_pVolumeMicText;
   
    ui_button*          m_pButtonAccept;

    ui_text*            m_pNavText;

    s32                 m_CurrHL;

    xbool               m_bRenderBlackout;

    player_profile      m_Profile;
};

//==============================================================================
#endif // DLG_HEADSET_HPP
//==============================================================================
