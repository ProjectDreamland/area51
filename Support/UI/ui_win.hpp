//==============================================================================
//  
//  ui_win.hpp
//  
//==============================================================================

#ifndef UI_WIN_HPP
#define UI_WIN_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#include "x_math.hpp"
#endif

//==============================================================================
//  ui_win
//==============================================================================

class ui_manager;

class ui_win
{
    friend ui_manager;

public:
    enum hitcode
    {
        HT_NONE         = 0,
        HT_CLIENT       = 1,
    };

    enum flags
    {
        WF_VISIBLE      = 0x00000001,                   // Is visible
        WF_STATIC       = 0x00000002,                   // Is static, should not respond to input
        WF_BORDER       = 0x00000004,                   // Has Border
        WF_TAB          = 0x00000008,                   // Is a page of a tabbed dialog
        
        WF_NO_ACTIVATE  = 0x00000010,                   // Do not activate first control of dialog
        WF_TITLE        = 0x00010000,                   // Has a title.
        WF_DLG_CENTER   = 0x00000020,                   // Center Dialog when it is opened

        WF_DISABLED     = 0x00000100,                   // Is disabled
        WF_SELECTED     = 0x00000200,                   // Is selected
        WF_HIGHLIGHT    = 0x00000400,                   // Is highlight

        WF_INPUTMODAL   = 0x00001000,                   // Is input modal, input stops here
        WF_RENDERMODAL  = 0x00002000,                   // Is render modal, rendering stops here

        WF_BUTTON_LEFT  = 0x00004000,                   // Button needs to be left just.
        WF_BUTTON_RIGHT = 0x00008000,                   // Button needs to be right just.

        WF_USE_ABSOLUTE = 0x00010000,                   // Use absolute co-ordinates

        WF_SCALE_XPOS    = 0x00020000,                  // Scale dialog object X position
        WF_SCALE_XSIZE   = 0x00040000,                  // Scale dialog object X size
        WF_SCALE_YPOS    = 0x00080000,                  // Scale dialog object Y position
        WF_SCALE_YSIZE   = 0x00100000,                  // Scale dialog object Y size
    };

    enum notifications
    {
        WN_COMBO_SELCHANGE  = 0x00000001,                   // Combo control selection change
        
        WN_SLIDER_CHANGE,                                   // Slider value has changed

        WN_CHECK_CHANGE,                                    // Check state change
        
        WN_LIST_SELCHANGE,                                  // List selection changed
        WN_LIST_ACCEPTED,                                   // New selection accepted
        WN_LIST_CANCELLED,                                  // New selection cancelled

        WN_TAB_CHANGE,                                      // Tabbed dialog changed tab

        WN_USER             = 0x40000000,                   // First User Message
    };

    enum messages
    {
    };

public:
                            ui_win              ( void );
    virtual                ~ui_win              ( void );

    xbool                   Create              ( s32           UserID,
                                                  ui_manager*   pManager,
                                                  const irect&  Position,
                                                  ui_win*       pParent,
                                                  s32           Flags );
    virtual void            Destroy             ( void );

    virtual void            Render              ( s32 ox=0, s32 oy=0 );

    virtual void            SetPosition         ( const irect& Position );
    virtual const irect&    GetPosition         ( void ) const;
    virtual const irect&    GetCreatePosition   ( void ) const;
    s32                     GetWidth            ( void ) const;
    s32                     GetHeight           ( void ) const;
    ui_win*                 GetWindowAtXY       ( s32 x, s32 y ) const;

    void                    SetFlags            ( s32 Flags );
    s32                     GetFlags            ( void ) const;
    void                    SetFlag             ( s32 Flag, s32 State );
    s32                     GetFlags            ( s32 Flag );

    virtual void            SetLabel            ( const xwstring&   Text );
    virtual void            SetLabel            ( const xwchar*     Text );
    virtual void            SetLabelColor       ( const xcolor&     color);
    virtual const xcolor&   GetLabelColor       ( void ) const;
    virtual const xwstring& GetLabel            ( void ) const;
    virtual void            SetLabelFlags       ( u32 Flags );

    void                    SetControlID        ( s32 ID );
    s32                     GetControlID        ( void ) const;

//    virtual void            SetText             ( const xstring& Text );
//    virtual void            SetText             ( const char*    Text );
//    virtual const xstring&  GetText             ( void ) const;
//    virtual void            SetTextFlags        ( u32 Flags );

    void                    SetParent           ( ui_win* pParent );
    ui_win*                 GetParent           ( void ) const;

    // Finding Children
//    ui_win*                 FindChildByLabel    ( const xstring& Label ) const;
//    ui_win*                 FindChildByLabel    ( const char*    Label ) const;
    ui_win*                 FindChildByID       ( s32 ID ) const;
    xbool                   IsChildOf           ( ui_win* pParent ) const;

    // Coordinate system conversions
    void                    LocalToScreen       ( s32& x, s32& y ) const;
    void                    ScreenToLocal       ( s32& x, s32& y ) const;
    void                    LocalToScreen       ( irect& r ) const;
    void                    ScreenToLocal       ( irect& r ) const;
    void                    LocalToScreenCreate ( irect& r ) const;
    void                    ScreenToLocalCreate ( irect& r ) const;

    // Messaging functions
    virtual void            OnUpdate            ( ui_win* pWin, f32 DeltaTime );
    virtual void            OnNotify            ( ui_win* pWin, ui_win* pSender, s32 Command, void* pData );
    virtual void            OnLBDown            ( ui_win* pWin );
    virtual void            OnLBUp              ( ui_win* pWin );
    virtual void            OnMBDown            ( ui_win* pWin );
    virtual void            OnMBUp              ( ui_win* pWin );
    virtual void            OnRBDown            ( ui_win* pWin );
    virtual void            OnRBUp              ( ui_win* pWin );
    virtual void            OnCursorMove        ( ui_win* pWin, s32 x, s32 y );
    virtual void            OnCursorEnter       ( ui_win* pWin );
    virtual void            OnCursorExit        ( ui_win* pWin );
    virtual void            OnKeyDown           ( ui_win* pWin, s32 Key );
    virtual void            OnKeyUp             ( ui_win* pWin, s32 Key );
    virtual void            OnPadNavigate       ( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX = FALSE, xbool WrapY = FALSE );
    virtual void            OnPadSelect         ( ui_win* pWin );
    virtual void            OnPadBack           ( ui_win* pWin );
    virtual void            OnPadDelete         ( ui_win* pWin );
    virtual void            OnPadHelp           ( ui_win* pWin );
    virtual void            OnPadActivate       ( ui_win* pWin );
    virtual void            OnPadShoulder       ( ui_win* pWin, s32 Direction );
    virtual void            OnPadShoulder2      ( ui_win* pWin, s32 Direction );

protected:
    ui_manager*         m_pManager;         // Pointer to ui manager
    s32                 m_UserID;           // UserID that owns this window

    ui_win*             m_pParent;          // Pointer to parent window
    xarray<ui_win*>     m_Children;         // List of child windows
    s32                 m_Flags;            // Window flags

    s32                 m_ID;               // Window ID
    irect               m_CreatePosition;   // Position of window at creation (before any resolution changes)
    irect               m_Position;         // Position of window
    xwstring				m_Label;            // Window Label
    u32                 m_LabelFlags;       // Window Label Flags
    xcolor              m_LabelColor;       // Window label color
    s32                 m_Font;             // Window Font
};

//==============================================================================
#endif // UI_WIN_HPP
//==============================================================================
