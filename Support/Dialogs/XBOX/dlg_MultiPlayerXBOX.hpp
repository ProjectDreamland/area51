//==============================================================================
//  
//  dlg_MultiPlayer.hpp
//  
//==============================================================================

#ifndef DLG_MULTI_PLAYER_HPP
#define DLG_MULTI_PLAYER_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_blankbox.hpp"
#include "..\dlg_PopUp.hpp"

#include "StateMgr\StateMgr.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

enum multi_player_controls
{
	IDC_MULTI_PLAYER_ONE_DETAILS,
	IDC_MULTI_PLAYER_TWO_DETAILS,
	IDC_MULTI_PLAYER_THREE_DETAILS,
	IDC_MULTI_PLAYER_FOUR_DETAILS,
    IDC_MULTI_PLAYER_ONE_COMBO,
    IDC_MULTI_PLAYER_TWO_COMBO,
    IDC_MULTI_PLAYER_THREE_COMBO,
    IDC_MULTI_PLAYER_FOUR_COMBO,
    IDC_MULTI_PLAYER_ONE_TEXT,
    IDC_MULTI_PLAYER_TWO_TEXT,
    IDC_MULTI_PLAYER_THREE_TEXT,
    IDC_MULTI_PLAYER_FOUR_TEXT,
    IDC_MULTI_PLAYER_NAV_TEXT,
};

//==============================================================================
//  dlg_multi_player
//==============================================================================

extern void     dlg_multi_player_register  ( ui_manager* pManager );
extern ui_win*  dlg_multi_player_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_multi_player : public ui_dialog
{
public:
                        dlg_multi_player       ( void );
    virtual            ~dlg_multi_player       ( void );

    xbool               Create              ( s32                       UserID,
                                              ui_manager*               pManager,
                                              ui_manager::dialog_tem*   pDialogTem,
                                              const irect&              Position,
                                              ui_win*                   pParent,
                                              s32                       Flags,
                                              void*                     pUserData);
    virtual void        Destroy             ( void );

    virtual void        Render              ( s32 ox=0, s32 oy=0 );

    virtual void        OnPadNavigate       ( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX = FALSE, xbool WrapY = FALSE );
    virtual void        OnPadDelete         ( ui_win* pWin );
    virtual void        OnPadActivate       ( ui_win* pWin );
    virtual void        OnPadSelect         ( ui_win* pWin );
    virtual void        OnPadBack           ( ui_win* pWin );
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );

    void                RefreshProfileList  ( void );

    void                OnPollReturn        ( void );
    void                OnLoadProfileCB     ( void );

protected:
    ui_frame*           m_pFrame1;

    ui_blankbox*        m_pPlayerOneBox;
    ui_blankbox*        m_pPlayerTwoBox;
    ui_blankbox*        m_pPlayerThreeBox;
    ui_blankbox*        m_pPlayerFourBox;

    ui_combo*           m_pPlayerOneCombo;
    ui_combo*           m_pPlayerTwoCombo;
    ui_combo*           m_pPlayerThreeCombo;
    ui_combo*           m_pPlayerFourCombo;

    ui_text*            m_pPlayerOneText;
    ui_text*            m_pPlayerTwoText;
    ui_text*            m_pPlayerThreeText;
    ui_text*            m_pPlayerFourText;

    ui_text*            m_pNavText;

    xbool               m_PlayerReady[SM_MAX_PLAYERS];
    xbool               m_ReadyToLaunch;

    s32                 m_CreatePlayerIndex;                // player who is creating the profile
    s32                 m_CreateIndex;                      // index in the list where the create profile option is located

    s32                 m_CurrHL;

    xwstring            m_ProfileName;
    xbool               m_ProfileEntered;
    xbool               m_ProfileOK;

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;
    s32                 m_PopUpType;

    xbool               m_bEditProfile;

    s32                 m_BlocksRequired;
};

//==============================================================================
#endif // DLG_MULTI_PLAYER_HPP
//==============================================================================
