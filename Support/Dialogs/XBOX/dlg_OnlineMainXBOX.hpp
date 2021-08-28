//==============================================================================
//  
//  dlg_OnlineMain.hpp
//  
//==============================================================================

#ifndef DLG_ONLINE_MAIN_HPP
#define DLG_ONLINE_MAIN_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"
#include "ui\ui_bitmap.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

enum online_main_controls
{
    IDC_ONLINE_QUICKMATCH,
    IDC_ONLINE_OPTIMATCH,
    IDC_ONLINE_HOST,
    IDC_ONLINE_FRIENDS,
    IDC_ONLINE_PLAYERS,
    IDC_ONLINE_EDIT_PROFILE,
    IDC_ONLINE_VIEW_STATS,
    IDC_ONLINE_SIGN_OUT,
    IDC_ONLINE_NAV_TEXT,
    IDC_ONLINE_MP_FRIEND_INV, // friend invitation
    IDC_ONLINE_MP_GAME_INV,   // game invitation
};


//==============================================================================
//  dlg_multi_player
//==============================================================================

extern void     dlg_online_main_register  ( ui_manager* pManager );
extern ui_win*  dlg_online_main_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;
class dlg_popup;

class dlg_online_main : public ui_dialog
{
public:
                        dlg_online_main       ( void );
    virtual            ~dlg_online_main       ( void );

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
    ui_button*          m_pButtonQuickMatch;        
    ui_button*          m_pButtonOptiMatch;         
    ui_button*          m_pButtonHost;
    ui_button*          m_pButtonFriends;
    ui_button*          m_pButtonPlayers;
    ui_button*          m_pButtonEditProfile;
    ui_button*          m_pButtonViewStats;
    ui_button*          m_pButtonSignOut;
    ui_text*            m_pNavText;
    s32                 m_CurrHL;

    dlg_popup*          m_pPopUp;
    s32                 m_PopUpResult;

    ui_bitmap*          m_pFriendInvite;
    ui_bitmap*          m_pGameInvite;
};

//==============================================================================
#endif // DLG_ONLINE_MAIN_HPP
//==============================================================================
