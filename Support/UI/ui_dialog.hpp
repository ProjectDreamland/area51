//==============================================================================
//  
//  ui_dialog.hpp
//  
//==============================================================================

#ifndef UI_DIALOG_HPP
#define UI_DIALOG_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#include "x_math.hpp"
#endif

#include "ui_win.hpp"
#include "ui_manager.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

enum dialog_states
{
    DIALOG_STATE_INIT = 0,
    DIALOG_STATE_ACTIVE,
    DIALOG_STATE_SELECT,
    DIALOG_STATE_BACK,
    DIALOG_STATE_ACTIVATE,
    DIALOG_STATE_DELETE,
    DIALOG_STATE_CANCEL,
    DIALOG_STATE_RETRY,
    DIALOG_STATE_TIMEOUT,
    DIALOG_STATE_EXIT,
    DIALOG_STATE_EDIT,
    DIALOG_STATE_CREATE,
    DIALOG_STATE_WAIT_FOR_MEMCARD,
    DIALOG_STATE_MEMCARD_ERROR,
    DIALOG_STATE_POPUP,
    NUM_DIALOG_STATES
};


//
// Macros
//

// "frame2" Macros to put a frame2 around a irect...
#define FRAME_TO_L(a) ((int)((a)-21))
#define FRAME_TO_T(a) ((int)((a)-21))
#define FRAME_TO_R(a) ((int)((a)+14))
#define FRAME_TO_B(a) ((int)((a)+14))

#ifdef TARGET_XBOX
#define SAFE_ZONE   5
#else
#define SAFE_ZONE   0
#endif


//==============================================================================
//  ui_dialog
//==============================================================================

extern ui_win* ui_dialog_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_dialog : public ui_win
{
    friend ui_manager;

public:
                            ui_dialog           ( void );
    virtual                ~ui_dialog           ( void );
                            
    xbool                   Create              ( s32                       UserID,
                                                  ui_manager*               pManager,
                                                  ui_manager::dialog_tem*   pDialogTem,
                                                  const irect&              Position,
                                                  ui_win*                   pParent,
                                                  s32                       Flags,
                                                  void*                     pUserData = NULL );

    virtual void            Render              ( s32 ox=0, s32 oy=0 );

    virtual const irect&    GetCreatePosition   ( void ) const;

    virtual void            OnPadNavigate       ( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX = FALSE, xbool WrapY = FALSE);
                            
    void                    SetBackgroundColor  ( xcolor Color );
    xcolor                  GetBackgroundColor  ( void ) const;
                            
    xbool                   GotoControl         ( ui_control* pControl );
    ui_control*             GotoControl         ( s32 iControl = 0 );
                            
    s32                     GetNumControls      ( void ) const;

    ui_manager::dialog_tem* GetTemplate         ( void );

    void                    SetState            ( dialog_states State)      { m_State = State; }
    dialog_states           GetState            ( void )                    { return m_State; }
    s32                     GetControl          ( void )                    { return m_CurrentControl; }

    // screen scaling functions
    void                    InitScreenScaling   ( const irect& Position );
    xbool                   UpdateScreenScaling ( f32 DeltaTime, xbool DoWipe = TRUE );

    // global dialog theme functions
    static void             SetTextColorNormal  ( xcolor color)             { m_TextColorNormal    = color; }
    static void             SetTextColorShadow  ( xcolor color)             { m_TextColorShadow    = color; }

protected:
    s32                     m_iElement;
    ui_manager::dialog_tem* m_pDialogTem;
    s32                     m_NavW;
    s32                     m_NavH;
    s32                     m_NavX;
    s32                     m_NavY;
    xarray<ui_win*>         m_NavGraph;
    xcolor                  m_BackgroundColor;
    s32                     m_OldCursorX;
    s32                     m_OldCursorY;
    s32                     m_XRes,m_YRes;
    xbool                   m_InputEnabled;
    void*                   m_pUserData;
    dialog_states           m_State;
    s32                     m_CurrentControl;

    // set once for all dialogs
    static xcolor           m_TextColorNormal;
    static xcolor           m_TextColorShadow;

    // dialog scaling controls
    irect                   m_CurrPos;
    irect                   m_RequestedPos;
    irect                   m_StartPos;
    f32                     m_scaleX;
    f32                     m_scaleY;
    f32                     m_totalX;
    f32                     m_scaleCount;
    f32                     m_scaleAngle;

#ifdef TARGET_XBOX
    s16                     m_XBOXNotificationOffsetX;
    s16                     m_XBOXNotificationOffsetY;
    u8                      m_bIsPopup:1,
                            m_bUseTopmost:1;
#endif // TARGET_XBOX
};

extern xcolor   m_TextColorNormal;
extern xcolor   m_TextColorShadow;

//==============================================================================
#endif // UI_DIALOG_HPP
//==============================================================================
