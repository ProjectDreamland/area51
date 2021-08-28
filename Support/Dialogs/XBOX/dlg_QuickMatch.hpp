//==============================================================================
//  
//  dlg_QuickMatch.hpp
//  
//==============================================================================

#ifndef DLG_QUICK_MATCH_HPP
#define DLG_QUICK_MATCH_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_button.hpp"
#include "ui\ui_bitmap.hpp"

#include "..\dlg_PopUp.hpp"

#include "NetworkMgr\GameMgr.hpp"

//==============================================================================
//  dlg_quick_match
//==============================================================================

extern void     dlg_quick_match_register  ( ui_manager* pManager );
extern ui_win*  dlg_quick_match_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_quick_match : public ui_dialog
{
public:
                        dlg_quick_match ( void );
    virtual            ~dlg_quick_match ( void );

    xbool               Create                  ( s32                       UserID,
                                                  ui_manager*               pManager,
                                                  ui_manager::dialog_tem*   pDialogTem,
                                                  const irect&              Position,
                                                  ui_win*                   pParent,
                                                  s32                       Flags,
                                                  void*                     pUserData);
    virtual void        Destroy                 ( void );

    virtual void        Render                  ( s32 ox=0, s32 oy=0 );

    virtual void        OnPadSelect             ( ui_win* pWin );
    virtual void        OnPadBack               ( ui_win* pWin );
    virtual void        OnUpdate                ( ui_win* pWin, f32 DeltaTime );

protected:
    ui_frame*           m_pFrame1;
    ui_button*          m_pGameTypeDM;
    ui_button*          m_pGameTypeTDM;
    ui_button*          m_pGameTypeCTF;
    ui_button*          m_pGameTypeTAG;
    ui_button*          m_pGameTypeINF;
    ui_button*          m_pGameTypeCNH;

    ui_text*            m_pNavText;
    s32                 m_CurrHL;

    ui_bitmap*          m_pStatusBox;
    ui_text*            m_pStatusText;

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;
	xbool   			m_SearchStarted;
    game_type           m_RequestedType;
};

//==============================================================================
#endif // DLG_QUICK_MATCH_HPP
//==============================================================================
