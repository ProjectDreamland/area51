// 
// 
// 
// dlg_MCMessage.hpp
// Wed Jan 08 10:00:30 2003
//
//

#ifndef __DLG_MCMESSAGE__ 
#define __DLG_MCMESSAGE__ 

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"

enum
{
    DLG_MCMESSAGE_IDLE,
    DLG_MCMESSAGE_YES,
    DLG_MCMESSAGE_NO,
    DLG_MCMESSAGE_MAYBE,
    DLG_MCMESSAGE_BACK,
};

//==============================================================================
//  dlg_online
//==============================================================================

extern void     dlg_mcmessage_register( ui_manager* pManager );
extern ui_win*  dlg_mcmessage_factory ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;
class ui_text;

class dlg_mcmessage : public ui_dialog
{
public:
                    dlg_mcmessage         ( void );
    virtual        ~dlg_mcmessage         ( void );

    void            Configure           ( const xwchar*             Title,
                                          const xwchar*             Yes,
                                          const xwchar*             No,
                                          const xwchar*             Message,
                                          const xcolor              MessageColor,
                                          s32*                      pResult = NULL,
                                          const xbool               DefaultToNo = TRUE,
										  const xbool				AllowCancel = FALSE,
                                          const xbool               AllowSelect = TRUE );

    void            Configure           ( const xwchar*             Title,
                                          const xwchar*             Yes,
                                          const xwchar*             No,
                                          const xwchar*             Maybe,
                                          const xwchar*             Message,
                                          const xcolor              MessageColor,
                                          s32*                      pResult = NULL,
                                          const xbool               DefaultToNo = TRUE,
										  const xbool				AllowCancel = FALSE,
                                          const xbool               AllowSelect = TRUE );


    void            SetTimeout          (f32 timeout );
	void			Close				( void );
    xbool           Create              ( s32                       UserID,
                                          ui_manager*               pManager,
                                          ui_manager::dialog_tem*   pDialogTem,
                                          const irect&              Position,
                                          ui_win*                   pParent,
                                          s32                       Flags,
                                          void*                     pUserData );

    virtual void    Destroy             ( void );

    virtual void    Render              ( s32 ox=0, s32 oy=0 );

    virtual void    OnPadNavigate       ( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX = FALSE, xbool WrapY = FALSE );
    virtual void    OnPadSelect         ( ui_win* pWin );
    virtual void    OnPadBack           ( ui_win* pWin );
    virtual void    OnPadHelp           ( ui_win* pWin ){ OnPadSelect( pWin ); }
    virtual void    OnNotify            ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData );
    virtual void    OnUpdate            ( ui_win* pWin, f32 DeltaTime);
    void            EnableProgress      ( xbool IsEnabled, s32 MaxWidth=200 );
    void            SetProgress         ( f32 Progress );

protected:
    void            UpdateControls      ( void );

protected:
    ui_button*      m_pYes;
    ui_button*      m_pNo;
    ui_button*      m_pMaybe;

    xbool           m_EnableProgress;
    f32             m_Progress;
    s32             m_ProgressWidth;
    xwstring        m_Message;
    xcolor          m_MessageColor;
    s32             *m_pResult;
    f32             m_Timeout;
	xbool			m_AllowCancel;
    xbool           m_AllowSelect;
    xbool           m_bDoBlackout;
    s32             m_iElement;
    s32             m_CurrHL;
};



#endif // DLG_MCMESSAGE
