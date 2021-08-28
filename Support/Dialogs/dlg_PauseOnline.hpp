//==============================================================================
//  
//  dlg_PauseOnline.hpp
//  
//==============================================================================

#ifndef DLG_PAUSE_ONLINE_HPP
#define DLG_PAUSE_ONLINE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_combo.hpp"

#include "dlg_PopUp.hpp"

#ifdef TARGET_XBOX
#include "ui\ui_bitmap.hpp"
#endif

//==============================================================================
//  DEFINES
//==============================================================================

enum pause_online_controls
{
    IDC_PAUSE_ONLINE_SUICIDE,
    IDC_PAUSE_ONLINE_SWITCH_TEAM,
    IDC_PAUSE_ONLINE_VOTE_MAP,
    IDC_PAUSE_ONLINE_VOTE_KICK,
    IDC_PAUSE_ONLINE_FRIENDS,
    IDC_PAUSE_ONLINE_PLAYERS,
    IDC_PAUSE_ONLINE_OPTIONS,
    IDC_PAUSE_ONLINE_SETTINGS,
    IDC_PAUSE_ONLINE_CONFIG,
    IDC_PAUSE_ONLINE_NAV_TEXT,
#ifdef TARGET_XBOX
    IDC_PAUSE_ONLINE_FRIEND_INV, // friend invitation
    IDC_PAUSE_ONLINE_GAME_INV,   // game invitation
#endif
    IDC_PAUSE_ONLINE_QUIT,
};


//==============================================================================
//  dlg_pause_online
//==============================================================================

extern void     dlg_pause_online_register  ( ui_manager* pManager );
extern ui_win*  dlg_pause_online_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_pause_online : public ui_dialog
{
public:
                        dlg_pause_online       ( void );
    virtual            ~dlg_pause_online       ( void );

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

protected:
    ui_frame*           m_pFrame1;
	ui_button*			m_pButtonSuicide;
    ui_button*			m_pButtonSwitchTeam; 	
	ui_button*			m_pButtonVoteMap; 	
    ui_button*			m_pButtonVoteKick; 	
    ui_button*			m_pButtonFriends; 	
    ui_button*			m_pButtonPlayers; 	
	ui_button*			m_pButtonOptions; 	
    ui_button*          m_pButtonSettings;
	ui_button*			m_pButtonConfig; 	
    ui_text*            m_pNavText;
#ifdef TARGET_XBOX
    ui_bitmap*          m_pFriendInvite;
    ui_bitmap*          m_pGameInvite;
#endif

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;

    s32                 m_CurrHL;
};

//==============================================================================
#endif // DLG_PAUSE_ONLINE_HPP
//==============================================================================
