//==============================================================================
//  
//  dlg_Feedback.hpp
//  
//==============================================================================

#ifndef DLG_FEEDBACK_HPP
#define DLG_FEEDBACK_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "ui\ui_blankbox.hpp"
#include "dlg_PopUp.hpp"
#include "networkmgr/matchmgr.hpp" 

//==============================================================================
//  dlg_feedback
//==============================================================================

extern void     dlg_feedback_register  ( ui_manager* pManager );
extern ui_win*  dlg_feedback_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_feedback : public ui_dialog
{
public:
    enum _TYPE
    {
        NORMAL_FEEDBACK = 0,
        ATTACHMENT_FEEDBACK,
    };


                        dlg_feedback  ( void );
    virtual            ~dlg_feedback  ( void );

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
    
    void                EnableBlackout      ( void )                    { m_bRenderBlackout = TRUE; }
    void                ChangeConfig        ( u8 type );

protected:

    u8                  m_Type;

    ui_frame*           m_pFrame1;
    ui_blankbox*        m_pPlayerDetails;

    ui_text*            m_pPlayerName;
    ui_text*            m_pSessionDate;
    ui_text*            m_pFeedback;
    ui_text*            m_pComplaints;
    ui_text*            m_pNavText;

    ui_button*          m_pButtonGreatSession;
    ui_button*          m_pOffensiveMessage;
    ui_button*          m_pButtonGoodAttitude;
    ui_button*          m_pButtonBadName;
    ui_button*          m_pButtonCursing;
    ui_button*          m_pButtonScreaming;
    ui_button*          m_pButtonCheating;
    ui_button*          m_pButtonThreats;

    s32                 m_CurrHL;

    dlg_popup*          m_PopUp;
    s32                 m_PopUpResult;

    xbool               m_bRenderBlackout;
    player_feedback     m_FeedbackType;
};

//==============================================================================
#endif // DLG_FEEDBACK_HPP
//==============================================================================
