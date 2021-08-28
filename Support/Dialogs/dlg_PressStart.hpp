// 
// 
// 
// dlg_PressStart.hpp
// Thu Nov 21 09:45:59 2002
//
//

#ifndef __DLG_PRESSSTART__ 
#define __DLG_PRESSSTART__ 

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_bitmap.hpp"

#include "dlg_PopUp.hpp"

#include "Obj_mgr\obj_mgr.hpp"

//==============================================================================
//  dlg_press_start
//==============================================================================

extern void     dlg_press_start_register  ( ui_manager* pManager );
extern ui_win*  dlg_press_start_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_press_start : public ui_dialog
{
public:
                        dlg_press_start     ( void );
    virtual            ~dlg_press_start     ( void );

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
    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );
	virtual void		OnPadHelp			( ui_win* pWin );

    void                DisableStartButton  ( void );
    void                EnableStartButton   ( void );

#ifdef TARGET_XBOX
    xbool               ValidateSettings    ( void );
#endif


protected:
	ui_text*			m_pButtonPressStart;
    ui_bitmap*          m_pLogoBitmap;
    ui_bitmap*          m_pFrameBitmap;

    f32                 m_DemoFadeAlpha;
    f32                 m_FadeControl;
    f32                 m_DemoHoldTimer;
    s32                 m_PressStartState;
    s32                 m_FadeStartInAlpha;
    xbool               m_FadeStartIn;
    s32                 m_FadeAdjust;
    f32                 m_WaitTime;
    f32                 m_Timeout;
    xbool               m_bPlayDemo;
    s32                 m_Element;
    s32                 m_BitmapID;

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;
    s32                 m_PopUpType;

    s32                 m_BlocksRequired;
};

#endif // DLG_PRESSSTART
