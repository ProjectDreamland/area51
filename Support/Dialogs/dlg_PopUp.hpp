//==============================================================================
//  
//  dlg_PopUp.hpp
//  
//==============================================================================

#ifndef DLG_POPUP_HPP
#define DLG_POPUP_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "ui\ui_dialog.hpp"
#include "ui\ui_frame.hpp"
#include "ui\ui_text.hpp"
#include "NetworkMgr\Voice\Headset.hpp"
#include "NetworkMgr\MatchMgr.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

enum
{
    DLG_POPUP_IDLE,
    DLG_POPUP_YES,
    DLG_POPUP_NO,
    DLG_POPUP_MAYBE,
    DLG_POPUP_HELP,
    DLG_POPUP_OTHER,
    DLG_POPUP_BACK,
};

//==============================================================================
//  dlg_popup
//==============================================================================

extern void     dlg_popup_register  ( ui_manager* pManager );
extern ui_win*  dlg_popup_factory   ( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_button;

class dlg_popup : public ui_dialog
{
public:
                        dlg_popup  ( void );
    virtual            ~dlg_popup  ( void );

    void                Configure           ( const f32                 Timeout,
                                              const xwchar*             Title,
                                              const xwchar*             Message,
                                              const xwchar*             Message2 = NULL,
                                              const xwchar*             Message3 = NULL );

    void                Configure           (       irect&              Position,
                                              const f32                 Timeout,
                                              const xwchar*             Title,
                                              const xwchar*             Message,
                                              const xwchar*             Message2 = NULL,
                                              const xwchar*             Message3 = NULL );

    void                Configure           ( const xwchar*             Title,
                                              const xbool               Yes,
                                              const xbool               No,
                                              const xbool               Maybe,
                                              const xwchar*             Message,
                                              const xwchar*             pNavText,
                                                    s32*                pResult = NULL );

    void                Configure           (       irect&              Position,
                                              const xwchar*             Title,
                                              const xbool               Yes,
                                              const xbool               No,
                                              const xbool               Maybe,
                                              const xwchar*             Message,
                                              const xwchar*             pNavText,
                                                    s32*                pResult = NULL );

    void                Configure           ( const xwchar*             Title,
                                              const xbool               Yes,
                                              const xbool               No,
                                              const xbool               Maybe,
                                              const xbool               Help,
                                              const xwchar*             Message,
                                              const xwchar*             pNavText,
                                                    s32*                pResult = NULL );

    void                Configure           (       irect&              Position,
                                              const xwchar*             Title,
                                              const xbool               Yes,
                                              const xbool               No,
                                              const xbool               Maybe,
                                              const xbool               Help,
                                              const xwchar*             Message,
                                              const xwchar*             pNavText,
                                                    s32*                pResult = NULL );

    void                ConfigureRecordDialog(      irect&              Position,
                                              const xwchar*             Title,
                                              const xwchar*             Message,
                                                    s32*                pResult = NULL );

    void                ConfigurePlayDialog (       irect&              Position,
                                              const xwchar*             Title,
                                              const xwchar*             Message,
                                              const xwchar*             pNavText,
                                                    s32*                pResult = NULL );

    void                ConfigurePassword   (       irect&              Position,
                                              const xwchar*             Title,
                                              const xwchar*             Message,
                                              const xwchar*             pNavText,
                                                    s32*                pResult = NULL );

    void                ConfigureAutosaveDialog (   irect&              Position,
                                              const xwchar*             Title,
                                              const xwchar*             Message,
                                              const xwchar*             pNavText,
                                                    s32*                pResult = NULL );

    void                SetPassword         (       u8*                 Password );

    void                SetBuddyMode        (       s32                 Mode        );
    void                SetBuddy            ( const buddy_info&         Buddy       );
    buddy_info&         GetBuddy            ( void );

    void                Close               ( void );

    xbool               Create              ( s32                       UserID,
                                              ui_manager*               pManager,
                                              ui_manager::dialog_tem*   pDialogTem,
                                              const irect&              Position,
                                              ui_win*                   pParent,
                                              s32                       Flags,
                                              void*                     pUserData);
    virtual void        Destroy             ( void );

    virtual void        Render              ( s32 ox=0, s32 oy=0 );

    virtual void        OnUpdate            ( ui_win* pWin, f32 DeltaTime );
    virtual void        OnPadSelect         ( ui_win* pWin );
    virtual void        OnPadBack           ( ui_win* pWin );
    virtual void        OnPadDelete         ( ui_win* pWin );
    virtual void        OnPadActivate       ( ui_win* pWin );
    virtual void        OnPadHelp           ( ui_win* pWin );
    virtual void        OnPadNavigate       ( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX = FALSE , xbool WrapY = FALSE);
    virtual void        OnPadShoulder       ( ui_win* pWin, s32 Direction );

    void                OnPadRecord         ( void );
    void                OnPadStop           ( void );
    void                OnPadPlay           ( void );
    void                OnPadSend           ( void );
    void                OnUpdateRecordDialog( f32 DeltaTime );
    void                OnUpdatePlayDialog  ( f32 DeltaTime );
    void                OnUpdatePasswordDialog( f32 DelatTime );

    void                EnableBlackout      ( xbool bDoBlackout )       { m_bDoBlackout = bDoBlackout; }

    void                AddPassKey          ( s32 key );
    xbool               RemovePassKey       ( void );

    enum RECORD_DIALOG_STATES
    {
        RECORD_STATE_NONE = 0,
        RECORD_STATE_INIT,
        RECORD_STATE_IDLE,
        RECORD_STATE_RECORD_INIT,
        RECORD_STATE_RECORD,
        RECORD_STATE_PLAY_INIT,
        RECORD_STATE_PLAY,
        RECORD_STATE_STOP,
        RECORD_STATE_SEND,
        RECORD_STATE_SENDING,
        RECORD_STATE_STOP_PLAYBACK,
        RECORD_STATE_EXIT
    };

    enum BUDDY_MODE_STATES
    {
        BUDDY_MODE_ADD = 0,
        BUDDY_MODE_INVITE
    };

    enum PLAY_DIALOG_STATES
    {
        PLAY_STATE_NONE = 0,
        PLAY_STATE_IDLE,
        PLAY_STATE_START_DOWNLOAD,
        PLAY_STATE_DOWNLOADING,
        PLAY_STATE_MESSAGE_READY,
        PLAY_STATE_START_PLAY,
        PLAY_STATE_PLAY,
        PLAY_STATE_EXIT,
        PLAY_STATE_INVALID_MESSAGE,
        PLAY_STATE_INVALID_MESSAGE_EXIT,
        PLAY_STATE_STOP
    };

    
protected:
    ui_frame*           m_pFrame1;
    ui_text*            m_pNavText;

    s32                 m_iElement;
    xwstring            m_Message;
    xwstring            m_Message2;
    xwstring            m_Message3;
    f32                 m_Timeout;
    xbool               m_bDoTimeout;
    s32*                m_pResult;
    s32                 m_CurrHL;
    xbool               m_bCheckYes;
    xbool               m_bCheckNo;
    xbool               m_bCheckMaybe;
    xbool               m_bCheckHelp;
    xbool               m_bCheckOther;
    xbool               m_bDoBlackout;

    // Record Dialog
    s32                 m_RecordDialogState;
    headset*            m_Headset; 
    f32                 m_ProgressValue;

    buddy_info          m_Buddy;
    s32                 m_AskMode;

    ui_text*            m_pButtonText[4];
    xbool               m_EnableRecord;
    xbool               m_EnablePlay;
    xbool               m_EnableSend;
    xbool               m_ShowStop;
    xbool               m_IsVoiceMailPopUp;

    // Play Dialog
    s32                 m_PlayDialogState;

    // Password Dialog
    xbool               m_PasswordDialogActive;
    u8                  m_Password[4];
    u8                  m_CheckPassword[4];
    s32                 m_PasswordIndex;

};

//==============================================================================
#endif // DLG_POPUP_HPP
//==============================================================================
