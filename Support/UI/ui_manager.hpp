//==============================================================================
//  
//  ui_manager.hpp
//  
//==============================================================================

#ifndef UI_MANAGER_HPP
#define UI_MANAGER_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#include "x_math.hpp"
#endif

#include "Obj_mgr\obj_mgr.hpp"

//==============================================================================
//  Controller button mapping
//==============================================================================

#if defined( TARGET_XBOX )
#define INPUT_BUTTON_UP         INPUT_XBOX_BTN_UP      
#define INPUT_BUTTON_DOWN       INPUT_XBOX_BTN_DOWN
#define INPUT_BUTTON_LEFT       INPUT_XBOX_BTN_LEFT
#define INPUT_BUTTON_RIGHT      INPUT_XBOX_BTN_RIGHT
#define INPUT_STICK_X           INPUT_XBOX_STICK_LEFT_X   
#define INPUT_STICK_Y           INPUT_XBOX_STICK_LEFT_Y   
#define INPUT_STICK_X_C         INPUT_XBOX_STICK_RIGHT_X
#define INPUT_STICK_Y_C         INPUT_XBOX_STICK_RIGHT_Y
#define INPUT_SELECT            INPUT_XBOX_BTN_A
#define INPUT_CANCEL            INPUT_XBOX_BTN_B
#define INPUT_PREV_MENU         INPUT_XBOX_R_TRIGGER
#define INPUT_NEXT_MENU         INPUT_XBOX_L_TRIGGER
#define INPUT_BUTTON_Z          INPUT_XBOX_BTN_BLACK
#define INPUT_BUTTON_START      INPUT_XBOX_BTN_START
#define INPUT_MAX_CONTROLLER_COUNT 4
#endif

#ifdef TARGET_PS2
#define INPUT_BUTTON_UP         INPUT_PS2_BTN_L_UP      
#define INPUT_BUTTON_DOWN       INPUT_PS2_BTN_L_DOWN
#define INPUT_BUTTON_LEFT       INPUT_PS2_BTN_L_LEFT
#define INPUT_BUTTON_RIGHT      INPUT_PS2_BTN_L_RIGHT
#define INPUT_STICK_X           INPUT_PS2_STICK_LEFT_X   
#define INPUT_STICK_Y           INPUT_PS2_STICK_LEFT_Y
#define INPUT_STICK_X_C         INPUT_PS2_STICK_RIGHT_X
#define INPUT_STICK_Y_C         INPUT_PS2_STICK_RIGHT_Y
#define INPUT_SELECT            INPUT_PS2_BTN_CROSS
#define INPUT_CANCEL            INPUT_PS2_BTN_SQUARE
#define INPUT_PREV_MENU         INPUT_PS2_BTN_L1
#define INPUT_NEXT_MENU         INPUT_PS2_BTN_R1
#define INPUT_BUTTON_Z          INPUT_PS2_BTN_R2
#define INPUT_BUTTON_START      INPUT_PS2_BTN_START
#define INPUT_MAX_CONTROLLER_COUNT 2
#endif

#if defined( TARGET_PC )
#define INPUT_BUTTON_UP         INPUT_KBD_UP      
#define INPUT_BUTTON_DOWN       INPUT_KBD_DOWN
#define INPUT_BUTTON_LEFT       INPUT_KBD_LEFT
#define INPUT_BUTTON_RIGHT      INPUT_KBD_RIGHT
#define INPUT_STICK_X           INPUT_PS2_STICK_LEFT_X   
#define INPUT_STICK_Y           INPUT_PS2_STICK_LEFT_Y
#define INPUT_STICK_X_C         INPUT_PS2_STICK_RIGHT_X
#define INPUT_STICK_Y_C         INPUT_PS2_STICK_RIGHT_Y
#define INPUT_SELECT            INPUT_PS2_BTN_CROSS
#define INPUT_CANCEL            INPUT_PS2_BTN_SQUARE
#define INPUT_PREV_MENU         INPUT_PS2_BTN_L1
#define INPUT_NEXT_MENU         INPUT_PS2_BTN_R1
#define INPUT_BUTTON_Z          INPUT_PS2_BTN_R2
#define INPUT_BUTTON_START      INPUT_PS2_BTN_START
#define INPUT_MAX_CONTROLLER_COUNT 2
#endif

//==============================================================================
//  Externals
//==============================================================================

class ui_win;
class ui_font;
class ui_dialog;
class ui_control;

#if !defined(X_RETAIL)
// debug variable - GetControllerID() calls are only valid from within a
// limited context. this will allow asserts to work in that case.
extern xbool bInProcessInput;
#endif

//==============================================================================
//  Types
//==============================================================================
#define BUTTON_SPRITE_WIDTH     18

#if defined( TARGET_XBOX )
enum
{
    XBOX_BUTTON_A, 
    XBOX_BUTTON_B, 
    XBOX_BUTTON_X, 
    XBOX_BUTTON_Y,
    XBOX_BUTTON_DPAD_DOWN,
    XBOX_BUTTON_DPAD_LEFT,
    XBOX_BUTTON_DPAD_UP,
    XBOX_BUTTON_DPAD_RIGHT, 
    XBOX_BUTTON_DPAD_UPDOWN,
    XBOX_BUTTON_DPAD_LEFTRIGHT,
    XBOX_BUTTON_STICK_RIGHT, 
    XBOX_BUTTON_STICK_LEFT, 
    XBOX_BUTTON_TRIGGER_L,
    XBOX_BUTTON_TRIGGER_R, 
    XBOX_BUTTON_BLACK, 
    XBOX_BUTTON_WHITE,
    XBOX_BUTTON_START,
    KILL_ICON,
    TEAM_KILL_ICON,
    DEATH_ICON,
    FLAG_ICON,
    VOTE_ICON,
    NEW_CREDIT_PAGE,
    CREDIT_TITLE_LINE,
    CREDIT_END,
    NUM_BUTTON_TEXTURES,

};
#elif defined(TARGET_PS2)
enum
{
    PS2_BUTTON_CROSS, 
    PS2_BUTTON_SQUARE, 
    PS2_BUTTON_TRIANGLE, 
    PS2_BUTTON_CIRCLE,
    PS2_BUTTON_DPAD_DOWN,
    PS2_BUTTON_DPAD_LEFT,
    PS2_BUTTON_DPAD_UP,
    PS2_BUTTON_DPAD_RIGHT, 
    PS2_BUTTON_DPAD_UPDOWN,
    PS2_BUTTON_DPAD_LEFTRIGHT,
    PS2_BUTTON_STICK_RIGHT, 
    PS2_BUTTON_STICK_LEFT, 
    PS2_BUTTON_L1,
    PS2_BUTTON_L2, 
    PS2_BUTTON_R1, 
    PS2_BUTTON_R2,
    PS2_BUTTON_START,
    KILL_ICON,
    TEAM_KILL_ICON,
    DEATH_ICON,
    FLAG_ICON,
    VOTE_ICON,
    NEW_CREDIT_PAGE,
    CREDIT_TITLE_LINE,
    CREDIT_END,
    NUM_BUTTON_TEXTURES,
};
#elif defined(TARGET_PC)
enum
{
    PS2_BUTTON_CROSS, 
    PS2_BUTTON_SQUARE, 
    PS2_BUTTON_TRIANGLE, 
    PS2_BUTTON_CIRCLE,
    PS2_BUTTON_DPAD_DOWN,
    PS2_BUTTON_DPAD_LEFT,
    PS2_BUTTON_DPAD_UP,
    PS2_BUTTON_DPAD_RIGHT, 
    PS2_BUTTON_DPAD_UPDOWN,
    PS2_BUTTON_DPAD_LEFTRIGHT,
    PS2_BUTTON_STICK_RIGHT, 
    PS2_BUTTON_STICK_LEFT, 
    PS2_BUTTON_L1,
    PS2_BUTTON_L2, 
    PS2_BUTTON_R1, 
    PS2_BUTTON_R2,
    PS2_BUTTON_START,
    KILL_ICON,
    TEAM_KILL_ICON,
    DEATH_ICON,
    FLAG_ICON,
    VOTE_ICON,
    NEW_CREDIT_PAGE,
    CREDIT_TITLE_LINE,
    CREDIT_END,
    NUM_BUTTON_TEXTURES,
};
#endif

//==============================================================================
//  Logging
//==============================================================================

extern xstring ui_log;

//==============================================================================
//  ui_manager
//==============================================================================

class ui_manager
{
public:

    //==========================================================================
    //  Templates for dialogs and controls
    //==========================================================================

    struct control_tem
    {
        s32             ControlID;
        const char*     StringID;
        const char*     pClass;
        s32             x, y, w ,h;
        s32             nx, ny, nw, nh;
        s32             Flags;
    };

    struct dialog_tem
    {
        const char*     StringID;
        s32             NavW, NavH;
        s32             nControls;
        control_tem*    pControls;
        s32             FocusControl;
    };

    //==========================================================================
    //  Typedefs for window and dialog factories
    //==========================================================================

    typedef ui_win* (*ui_pfn_winfact)( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags );
    typedef ui_win* (*ui_pfn_dlgfact)( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

    //==========================================================================
    //  Input Button Data
    //==========================================================================

    class button
    {
    public:
        xbool       State;
        f32         AnalogScaler;
        f32         AnalogEngage;
        f32         AnalogDisengage;
        f32         RepeatDelay;
        f32         RepeatInterval;
        f32         RepeatTimer;
        s32         nPresses;
        s32         nRepeats;
        s32         nReleases;

    public:
                    button              ( void ) { State = 0; nPresses = 0; nRepeats = 0; nReleases = 0; RepeatDelay = 0.200f; RepeatInterval = 0.060f; AnalogScaler = 1.0f; AnalogEngage = 0.5f; AnalogDisengage = 0.3f; };
                   ~button              ( void ) {};

        void        Clear               ( void )                    { State = 0; nPresses = 0; nRepeats = 0; nReleases = 0; };

        void        SetupRepeat         ( f32 Delay, f32 Interval ) { RepeatDelay = Delay; RepeatInterval = Interval; };
        void        SetupAnalog         ( f32 s, f32 e, f32 d )     { AnalogScaler = s ; AnalogEngage = e; AnalogDisengage = d; };
    };

    //==========================================================================
    //  Control States
    //==========================================================================

    enum control_state
    {
        CS_NORMAL               = 0,
        CS_HIGHLIGHT,
        CS_SELECTED,
        CS_HIGHLIGHT_SELECTED,
        CS_DISABLED
    };

    //==========================================================================
    //  User Data
    //==========================================================================

    enum user_navigate
    {
        NAV_UP,
        NAV_DOWN,
        NAV_LEFT,
        NAV_RIGHT,
    };

    struct user
    {
        xbool                   Enabled;
        s32                     ControllerID;
        irect                   Bounds;
        s32                     Data;
        s32                     Height;
        ui_win*                 pCaptureWindow;
        ui_win*                 pLastWindowUnderCursor;
        xstring                 Background;
        s32                     iHighlightElement;

        xbool                   CursorVisible;                // TRUE when Mouse Cursor Visible
        xbool                   MouseActive;                  // TRUE when Mouse Active
        s32                     CursorX;                      // Mouse Cursor X
        s32                     CursorY;                      // Mouse Cursor Y
        s32                     LastCursorX;                  // Last Mouse Cursor X
        s32                     LastCursorY;                  // Last Mouse Cursor Y
        button                  ButtonLB;
        button                  ButtonMB;
        button                  ButtonRB;

        button                  DPadUp            [INPUT_MAX_CONTROLLER_COUNT];
        button                  DPadDown        [INPUT_MAX_CONTROLLER_COUNT];
        button                  DPadLeft        [INPUT_MAX_CONTROLLER_COUNT];
        button                  DPadRight        [INPUT_MAX_CONTROLLER_COUNT];
        button                  PadSelect        [INPUT_MAX_CONTROLLER_COUNT];
        button                  PadBack            [INPUT_MAX_CONTROLLER_COUNT];
        button                  PadDelete        [INPUT_MAX_CONTROLLER_COUNT];
        button                  PadActivate     [INPUT_MAX_CONTROLLER_COUNT];
#if defined(TARGET_PS2) || defined(TARGET_XBOX)    
        button                  PadHelp            [INPUT_MAX_CONTROLLER_COUNT];
        button                  PadShoulderL    [INPUT_MAX_CONTROLLER_COUNT];
        button                  PadShoulderR    [INPUT_MAX_CONTROLLER_COUNT];
        button                  PadShoulderL2    [INPUT_MAX_CONTROLLER_COUNT];
        button                  PadShoulderR2    [INPUT_MAX_CONTROLLER_COUNT];
        button                  LStickUp        [INPUT_MAX_CONTROLLER_COUNT];
        button                  LStickDown        [INPUT_MAX_CONTROLLER_COUNT];
        button                  LStickLeft        [INPUT_MAX_CONTROLLER_COUNT];
        button                  LStickRight        [INPUT_MAX_CONTROLLER_COUNT];
#endif        

        xarray<ui_dialog*>      DialogStack;
    };

    //==========================================================================
    //  Window Class
    //==========================================================================

    struct winclass
    {
        xstring         ClassName;
        ui_pfn_winfact  pFactory;
    };

    //==========================================================================
    //  Graphic Element for UI
    //==========================================================================

    struct element
    {
        xstring           Name;
        //xbitmap         Bitmap;
        rhandle<xbitmap>  Bitmap;
        s32               nStates;
        s32               cx;
        s32               cy;
        xarray<irect>     r;
    };

    //==========================================================================
    //  Background
    //==========================================================================

    struct background
    {
        xstring           Name;
        xstring           BitmapName;
        rhandle<xbitmap>  Bitmap;
    };

    //==========================================================================
    //  Bitmap
    //==========================================================================

    struct bitmap
    {
        xstring           Name;
        xstring           BitmapName;
        rhandle<xbitmap>  Bitmap;
    };

    //==========================================================================
    //  Font
    //==========================================================================

    struct font
    {
        xstring         Name;
        ui_font*        pFont;
    };

    //==========================================================================
    //  Dialog Class
    //==========================================================================

    struct dialogclass
    {
        xstring         ClassName;
        ui_pfn_dlgfact  pFactory;
        dialog_tem*     pDialogTem;
    };

    //==========================================================================
    //  Clip Record
    //==========================================================================

    struct cliprecord
    {
        irect           r;
    };

    //==========================================================================
    //  Highlight
    //==========================================================================

    struct highlight
    {
        irect           r;
        s32             iElement;
        xbool           Flash;
    };

    //==========================================================================
    //  Wipe trail element
    //==========================================================================

    struct wipeElement
    {

        irect           Position;
        xbool           Active;
    };

//==============================================================================
//  Functions
//==============================================================================

protected:
    void            UpdateButton            ( ui_manager::button& Button, xbool State, f32 DeltaTime );
    void            UpdateAnalog            ( ui_manager::button& Button, f32 Value, f32 DeltaTime );

public:
                    ui_manager              ( void );
                   ~ui_manager              ( void );

    s32             Init                    ( void );
    void            Kill                    ( void );

    s32             LoadBackground          ( const char* pName, const char* pPathName );
    void            UnloadBackground        ( const char* pName );
    s32             FindBackground          ( const char* pName ) const;
    void            RenderBackground        ( const char* pName ) const;
    void            EnableBackground        ( xbool IsEnabled )                                     { m_EnableBackground = IsEnabled; }

    s32             LoadBitmap              ( const char* pName, const char* pPathName );
    void            UnloadBitmap            ( const char* pName );
    s32             FindBitmap              ( const char* pName );
    void            RenderBitmap            ( s32 iBitmap, const irect& Position, xcolor Color = XCOLOR_WHITE ) const;
    void            RenderBitmapUV          ( s32 iBitmap, const irect& Position, const vector2& UV0, const vector2& UV1, xcolor Color = XCOLOR_WHITE ) const;

    s32             LoadElement             ( const char* pName, const char* pPathName, s32 nStates, s32 cx, s32 cy );
    s32             FindElement             ( const char* pName ) const;
    void            RenderElement           ( s32 iElement, const irect& Position,       s32 State, const xcolor& Color = XCOLOR_WHITE, xbool IsAdditive = FALSE ) const;
    void            RenderElementUV         ( s32 iElement, const irect& Position, const irect& UV, const xcolor& Color = XCOLOR_WHITE, xbool IsAdditive = FALSE ) const;
    void            RenderElementUV         ( s32 iElement, const irect& Position, const vector2& UV0, const vector2& UV1, const xcolor& Color = XCOLOR_WHITE, xbool IsAdditive = FALSE ) const;
    const element*  GetElement              ( s32 iElement ) const;

    s32             LoadFont                ( const char* pName, const char* pPathName );
    s32             FindFont                ( const char* pName ) const;
    ui_font*        GetFont                 ( const char* pName) const;
    void            RenderText              ( s32 iFont, const irect& Position, s32 Flags, const xcolor& Color, const   char* pString, xbool IgnoreEmbeddedColor = TRUE, xbool UseGradient = TRUE, f32 FlareAmount = R_0  ) const;
    void            RenderText              ( s32 iFont, const irect& Position, s32 Flags, const xcolor& Color, const xwchar* pString, xbool IgnoreEmbeddedColor = TRUE, xbool UseGradient = TRUE, f32 FlareAmount = R_0  ) const;
    void            RenderText              ( s32 iFont, const irect& Position, s32 Flags,       s32     Alpha, const xwchar* pString, xbool IgnoreEmbeddedColor = TRUE, xbool UseGradient = TRUE, f32 FlareAmount = R_0  ) const;
    void            RenderText_Wrap         ( s32 iFont, const irect& Position, s32 Flags, const xcolor& Color, const xwstring& Text,  xbool IgnoreEmbeddedColor = TRUE, xbool UseGradient = TRUE, f32 FlareAmount = R_0  );

    s32             TextWidth               ( s32 iFont, const xwchar* pString, s32 Count = -1 ) const;
    s32             TextHeight              ( s32 iFont, const xwchar* pString, s32 Count = -1 ) const;
    void            TextSize                ( s32 iFont, irect& Rect, const xwchar* pString, s32 Count ) const;
    s32             GetLineHeight           ( s32 iFont ) const;

    void            RenderRect              ( const irect& r, const xcolor& Color, xbool IsWire=TRUE ) const;
    void            RenderGouraudRect       ( const irect& r, const xcolor& c1, const xcolor& c2, const xcolor& c3, const xcolor& c4, xbool IsWire=TRUE, xbool IsAdditive=FALSE ) const;

    xbool           RegisterWinClass        ( const char* ClassName, ui_pfn_winfact pFactory );
    ui_win*         CreateWin               ( s32 UserID, const char* ClassName, const irect& Position, ui_win* pParent, s32 Flags );

    xbool           RegisterDialogClass     ( const char* ClassName, dialog_tem* pDialogTem, ui_pfn_dlgfact pFactory );
    ui_dialog*      OpenDialog              ( s32 UserID, const char* ClassName, irect Position, ui_win* pParent, s32 Flags, void* pUserData = NULL );
    void            EndDialog               ( s32 UserID, xbool ResetCursor = FALSE );
    void            EndUsersDialogs         ( s32 UserID );
    s32             GetNumUserDialogs       ( s32 UserID );
    ui_dialog*      GetTopmostDialog        ( s32 UserID );

    s32             CreateUser              ( s32 ControllerID, const irect& Bounds, s32 Data = 0 );
    void            DeleteUser              ( s32 UserID );
    void            DeleteAllUsers          ( void );
    user*           GetUser                 ( s32 UserID ) const;
    s32             GetUserData             ( s32 UserID ) const;
    ui_win*         GetWindowUnderCursor    ( s32 UserID ) const;
    void            SetCursorVisible        ( s32 UserID, xbool State );
    xbool           GetCursorVisible        ( s32 UserID ) const;
    void            SetCursorPos            ( s32 UserID, s32  x, s32  y );
    void            GetCursorPos            ( s32 UserID, s32& x, s32& y ) const;
    ui_win*         SetCapture              ( s32 UserID, ui_win* pWin );
    void            ReleaseCapture          ( s32 UserID );
    void            SetUserBackground       ( s32 UserID, const char* pName );
    const irect&    GetUserBounds           ( s32 UserID ) const;
    void            EnableUser              ( s32 UserID, xbool State );
    xbool           IsUserEnabled           ( s32 UserID ) const;
    void            AddHighlight            ( s32 UserID, irect& r, xbool Flash = TRUE );
    void            SetUserController       ( s32 UserID, s32 ControllerID );

    void            PushClipWindow          ( const irect &r );
    void            PopClipWindow           ( void );

    ui_win*         GetWindowAtXY           ( user* pUser, s32 x, s32 y );
    xbool           ProcessInput            ( f32 DeltaTime );
    xbool           ProcessInput            ( f32 DeltaTime, s32 UserID );

    void            EnableUserInput         ( void );
    void            DisableUserInput        ( void );

    void            Update                  ( f32 DeltaTime );
    void            Render                  ( void );

    void            WordWrapString          ( s32 iFont, const irect& r, const char* pString, xwstring& RetVal );
    void            WordWrapString          ( s32 iFont, const irect& r, const xwstring& String, xwstring& RetVal );

    void            CheckRes                ( void );
    void            SetRes                  ( void );

    void            CheckForEndDialog       ( s32 UserID );

    u32             GetActiveController     ( void )                { return m_ActiveController; }

    f32             GetAlphaTime            ( void )                { return m_AlphaTime; }

    // button icons
    xbitmap*        GetButtonTexture        ( s32 buttonCode );      
    s32             LookUpButtonCode        ( const xwchar* pString, s32 iStart ) const;

    // scale factors
    f32             GetScaleX               ( void )                { return m_ScaleX; }
    f32             GetScaleY               ( void )                { return m_ScaleY; }

    // Screen wipe
    void            InitScreenWipe          ( void );
    void            RenderScreenWipe        ( void );
    void            UpdateScreenWipe        ( f32 DeltaTime );
    void            ResetScreenWipe         ( void );
    void            GetWipePos              ( irect &pos )          { pos = m_wipePos; }
    f32             GetWipeSpeed            ( void )                { return m_wipeSpeed; }
    xbool           IsWipeActive            ( void )                { return m_wipeActive; }

    // Refresh bar
    void            InitRefreshBar          ( void );
    void            RenderRefreshBar        ( void );
    void            UpdateRefreshBar        ( f32 deltaTime );

    // Screen controls
    xbool           IsScreenScaling         ( void )                { return m_isScaling; }
    void            SetScreenScaling        ( xbool val )           { m_isScaling = val; }
    void            GetScreenSize           ( irect& size )         { size = m_CurrScreenSize; }
    void            SetScreenSize           ( const irect& size );  
    xbool           IsScreenOn              ( void )                { return( m_ScreenIsOn ); }
    void            SetScreenOn             ( xbool state)          { m_ScreenIsOn = state; }

    // Screen highlight
    void            InitScreenHighlight     ( void );
    void            SetScreenHighlight      ( const irect& pos );
    void            RenderScreenHighlight   ( void );
    void            RenderScreenGlow        ( void );     
    void            EnableScreenHighlight   ( void )                { m_ScreenHighlightEnabled = TRUE; }
    void            DisableScreenHighlight  ( void )                { m_ScreenHighlightEnabled = FALSE; }
    s32             GetHighlightAlpha       ( s32 cycle );

    // Glow bar
    void            InitGlowBar             ( void );
    void            RenderGlowBar           ( void );
    void            UpdateGlowBar           ( f32 deltaTime );
    void            GetGlowBarPos           ( irect& pos )          { pos = m_GlowPos; }

    // Load progress bar
    f32             GetPercentLoaded        ( void )                { return m_PercentLoaded; }
    void            SetPercentLoaded        ( f32 percent );
    void            AddPercentLoaded        ( f32 percent );
    void            RenderProgressBar       ( xbool mustDraw );

    // debugging tools
    void            EnableSafeArea          ( void );
    void            DisableSafeArea         ( void );

    // ping color coding
    s32             PingToColor             ( f32 ping, xcolor& responsecolor );

#ifdef TARGET_XBOX

protected:
    void            RenderXBOXNotifications ( user* pUser );    

#endif // TARGET_XBOX
//==============================================================================
//  Data
//==============================================================================

protected:

    f32                     m_AlphaTime;

    xarray<user*>           m_Users;

    xarray<winclass>        m_WindowClasses;
    xarray<dialogclass>     m_DialogClasses;

    xarray<background*>     m_Backgrounds;
    xarray<bitmap*>         m_Bitmaps;
    xarray<element*>        m_Elements;
    xarray<font*>           m_Fonts;
    xbool                   m_EnableBackground;

    xarray<cliprecord>      m_ClipStack;

    xarray<highlight>       m_Highlights;

    xbool                   m_EnableUserInput;

    u32                     m_ActiveController;

    xbitmap                 m_Mouse;
    xcolor                  m_MouseColor;

    // button icons
    rhandle<xbitmap>        m_ButtonTextures[NUM_BUTTON_TEXTURES];

    xbool                   m_isScaling;

    // resolution scale factors
    f32                     m_ScaleX;
    f32                     m_ScaleY;

    // screen wipe controls
    xbool                   m_wipeActive;
    s32                     m_wipeStartY;
    s32                     m_wipeEndY;
    xbool                   m_wipeDown;
    f32                     m_wipeSpeed;
    irect                   m_wipePos;
    wipeElement             m_wipeTrail[16];
    s32                     m_wipeCount;
    u32                     m_wipeWidth;

    // refresh bar controls
    f32                     m_RefreshSpeed;
    u32                     m_RefreshWidth;
    irect                   m_RefreshPos;      

    // screen controls
    irect                   m_CurrScreenSize;
    xbool                   m_ScreenIsOn;

    // screen highlight 
    s32                     m_ScreenHighlightID;
    s32                     m_ScreenGlowID;
    irect                   m_ScreenHighlightPos;
    xbool                   m_ScreenHighlightEnabled;
    s32                     m_HighlightAlpha;
    xbool                   m_HighlightFadeUp;
    xbool                   m_CycleFadeUp;


    // glow bar controls
    s32                     m_GlowID;
    s32                     m_GlowStartX;
    s32                     m_GlowEndX;
    f32                     m_GlowSpeed;
    irect                   m_GlowPos;
    irect                   m_GlowTrail[8];
    xbool                   m_GlowOnTop;

    // progress bar
    f32                     m_PercentLoaded;
    f32                     m_LastProgressUpdatePercent;

    // debugging tools
    xbool                   m_RenderSafeArea;

#ifdef TARGET_PC
    xarray<ui_dialog*>      m_KillDialogStack;
#endif

#ifdef TARGET_XBOX
    enum xbox_notification_state
    {
        XNS_NOT_IN_USE,
        XNS_FADE_IN,
        XNS_HOLD,
        XNS_FADE_OUT,
        XNS_INVITE_PENDING,
    };
    f32                         m_XBOXNotificationTimer;
    xbox_notification_state     m_XBOXNotificationState;

#endif

public:
    xstring*            m_log;
};

extern ui_manager*    g_UiMgr;
extern s32            g_UiUserID;

//==============================================================================
#endif // UI_MANAGER_HPP
//==============================================================================
