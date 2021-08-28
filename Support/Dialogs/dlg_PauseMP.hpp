//==============================================================================
//  
//  dlg_PauseMP.hpp
//  
//==============================================================================

#ifndef DLG_PAUSE_MP_HPP
#define DLG_PAUSE_MP_HPP

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

enum pause_mp_controls
{
	IDC_PAUSE_MP_QUIT,
    IDC_PAUSE_MP_SCORE,
    IDC_PAUSE_MP_OPTIONS,
    IDC_PAUSE_MP_SETTINGS,
#ifdef TARGET_XBOX
    IDC_PAUSE_MP_FRIENDS,
#endif
    IDC_PAUSE_MP_NAV_TEXT,

#ifdef TARGET_XBOX
    IDC_PAUSE_MP_FRIEND_INV, // friend invitation
    IDC_PAUSE_MP_GAME_INV,   // game invitation
#endif

};


//==============================================================================
//  dlg_pause_mp
//==============================================================================

extern void     dlg_pause_mp_register  ( ui_manager* pManager );
extern ui_win*  dlg_pause_mp_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_pause_mp : public ui_dialog
{
public:
                        dlg_pause_mp       ( void );
    virtual            ~dlg_pause_mp       ( void );

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
	ui_button*			m_pButtonQuit;
	ui_button*			m_pButtonScore; 	
    ui_button*          m_pButtonOptions;
    ui_button*          m_pButtonSettings;
#ifdef TARGET_XBOX
    ui_button*          m_pButtonFriends;
#endif
    ui_text*            m_pNavText;
    s32                 m_CurrHL;

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;

#ifdef TARGET_XBOX
    ui_bitmap*          m_pFriendInvite;
    ui_bitmap*          m_pGameInvite;
#endif
};

//==============================================================================
#endif // DLG_PAUSE_MP_HPP
//==============================================================================
