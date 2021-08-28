//==============================================================================
//  
//  dlg_ProfileSelect.hpp
//  
//==============================================================================

#ifndef DLG_PROFILE_SELECT_HPP
#define DLG_PROFILE_SELECT_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_listbox.hpp"
#include "ui\ui_blankbox.hpp"

#include "dlg_PopUp.hpp"

//==============================================================================
//  dlg_profile_select
//==============================================================================

enum profile_select_type
{
    PROFILE_SELECT_MANAGE,
    PROFILE_SELECT_NORMAL,
    PROFILE_SELECT_OVERWRITE,
};

extern void     dlg_profile_select_register  ( ui_manager* pManager );
extern ui_win*  dlg_profile_select_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_profile_select : public ui_dialog
{
public:
                        dlg_profile_select ( void );
    virtual            ~dlg_profile_select ( void );

    xbool               Create                  ( s32                       UserID,
                                                  ui_manager*               pManager,
                                                  ui_manager::dialog_tem*   pDialogTem,
                                                  const irect&              Position,
                                                  ui_win*                   pParent,
                                                  s32                       Flags,
                                                  void*                     pUserData);
    virtual void        Destroy                 ( void );

    virtual void        Render                  ( s32 ox=0, s32 oy=0 );

    virtual void        OnPadNavigate           ( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX = FALSE, xbool WrapY = FALSE );
    virtual void        OnPadSelect             ( ui_win* pWin );
    virtual void        OnLBDown                ( ui_win* pWin );
    virtual void        OnPadBack               ( ui_win* pWin );
    virtual void        OnPadDelete             ( ui_win* pWin );
    virtual void        OnPadActivate           ( ui_win* pWin );
    virtual void        OnUpdate                ( ui_win* pWin, f32 DeltaTime );

    void                Configure               ( profile_select_type DialogType ); 
    void                RefreshProfileList      ( void );

    void                OnPollReturn            ( void );
    void                OnLoadProfileCB         ( void );
    void                OnDeleteProfileCB       ( void );
    void                OnSaveProfileCB         ( void );

    void                UpdateBackupPopup       ( void );
    void                CreateBackupPopup       ( void );

    void                EnableBlackout          ( void )                    { m_bRenderBlackout = TRUE; }
protected:
    ui_frame*                           m_pFrame1;
    ui_listbox*                         m_pProfileList;
    ui_blankbox*                        m_pProfileDetails;

    ui_text*                            m_pProfileName;
    ui_text*                            m_pCardSlot;
    ui_text*                            m_pCreationDate;
    ui_text*                            m_pModifiedDate;

    ui_text*                            m_pInfoProfileName;
    ui_text*                            m_pInfoCardSlot;
    ui_text*                            m_pInfoCreationDate;
    ui_text*                            m_pInfoModifiedDate;

    ui_text*                            m_pNavText;

    dlg_popup*                          m_PopUp;
    s32                                 m_PopUpResult;
    s32                                 m_PopUpType;

    dlg_popup*                          m_BackupPopup;
    s32                                 m_BackupPopupResult;

    s32                                 m_CurrHL;

    xwstring                            m_ProfileName;
    xbool                               m_ProfileEntered;
    xbool                               m_ProfileOk;

    s32                                 m_CreateIndex;
    s32                                 m_iCard;

    profile_select_type                 m_Type;

    xbool                               m_bEditProfile;
    xbool                               m_bRenderBlackout;

    s32                                 m_BlocksRequired;
};

//==============================================================================
#endif // DLG_PROFILE_SELECT_HPP
//==============================================================================
