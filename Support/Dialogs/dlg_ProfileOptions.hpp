//==============================================================================
//  
//  dlg_ProfileOptions.hpp
//  
//==============================================================================

#ifndef DLG_PROFILE_OPTIONS_HPP
#define DLG_PROFILE_OPTIONS_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_blankbox.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_check.hpp"

#include "dlg_PopUp.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

enum profile_options_controls
{
	IDC_PROFILE_OPTIONS_CONTROLS,
    IDC_PROFILE_OPTIONS_AVATAR,
    IDC_PROFILE_OPTIONS_DIFFICULTY,
    IDC_PROFILE_ONLINE_STATUS,
    IDC_PROFILE_OPTIONS_AUTOSAVE,
    IDC_PROFILE_OPTIONS_ACCEPT,

    IDC_PROFILE_DIFFICULTY_BBOX,
    IDC_PROFILE_DIFFICULTY_SELECT,

    IDC_PROFILE_STATUS_BBOX,
    IDC_PROFILE_STATUS_SELECT,

    IDC_PROFILE_AUTOSAVE_BBOX,
    IDC_PROFILE_AUTOSAVE_SELECT,

    IDC_PROFILE_OPTIONS_NAV_TEXT,
};


enum profile_dlg_types
{
    PROFILE_OPTIONS_NORMAL,
    PROFILE_OPTIONS_PAUSE,
};

//==============================================================================
//  dlg_profile_options
//==============================================================================

extern void     dlg_profile_options_register  ( ui_manager* pManager );
extern ui_win*  dlg_profile_options_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_profile_options : public ui_dialog
{
public:
                        dlg_profile_options       ( void );
    virtual            ~dlg_profile_options       ( void );

    xbool               Create              ( s32                       UserID,
                                              ui_manager*               pManager,
                                              ui_manager::dialog_tem*   pDialogTem,
                                              const irect&              Position,
                                              ui_win*                   pParent,
                                              s32                       Flags,
                                              void*                     pUserData);
    
    void                Configure           ( profile_dlg_types         Type,
                                              xbool                     bCreate );

    virtual void        Destroy             ( void );

    virtual void        Render              ( s32 ox=0, s32 oy=0 );

    virtual void        OnPadSelect         ( ui_win* pWin );
    virtual void        OnPadBack           ( ui_win* pWin );

    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

    void                OnSaveProfileCB     ( void );

#if defined(TARGET_PC) || defined(TARGET_XBOX)
    void                OnProfileCreateCB   ( void );
#endif

protected:
    ui_frame*           m_pFrame1;

    ui_button*			m_pButtonControls;
    ui_button*          m_pButtonAvatar;
    ui_button*          m_pButtonDifficulty;
    ui_button*          m_pButtonOnlineStatus;
    ui_button*          m_pButtonAutosave;
    ui_button*          m_pButtonCreate;

    ui_blankbox*        m_pDifficultyBBox;
    ui_blankbox*        m_pOnlineStatusBBox;
    ui_blankbox*        m_pAutosaveBBox;

    ui_combo*           m_pDifficultySelect;
    ui_combo*           m_pOnlineStatusSelect;
    ui_combo*           m_pAutosaveSelect;

    ui_text*            m_pNavText;

    s32                 m_CurrHL;

    profile_dlg_types   m_Type;
    xbool               m_bCreate;
    s32                 m_iCard;

    dlg_popup*          m_PopUp;
    s32                 m_PopUpType;
    s32                 m_PopUpResult;

#ifdef TARGET_XBOX
    s32                 m_BlocksRequired;
#endif
};

//==============================================================================
#endif // DLG_PROFILE_OPTIONS_HPP
//==============================================================================
