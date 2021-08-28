//==============================================================================
//  
//  ui_listbox.hpp
//  
//==============================================================================

#ifndef UI_LISTBOX_HPP
#define UI_LISTBOX_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#include "x_math.hpp"
#endif

#include "ui_control.hpp"

//==============================================================================
//  ui_listbox
//==============================================================================

#define LISTBOX_DATA_FIELDS     2

extern ui_win* ui_listbox_factory( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags );

class ui_listbox : public ui_control
{
public:
    struct item
    {
        xbool       Enabled;
        xwstring    Label;
        s32         Data[LISTBOX_DATA_FIELDS];
        xcolor      Color;
        u32         Flags;
    };

                    ui_listbox              ( void );
    virtual        ~ui_listbox              ( void );

    xbool           Create                  ( s32           UserID,
                                              ui_manager*   pManager,
                                              const irect&  Position,
                                              ui_win*       pParent,
                                              s32           Flags );

    virtual void    Render                  ( s32 ox=0, s32 oy=0 );
    virtual void    RenderItem              ( irect r, const item& Item, const xcolor& c1, const xcolor& c2 );

    virtual void    SetPosition             ( const irect& Position );

    virtual void    OnPadNavigate           ( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX = FALSE, xbool WrapY = FALSE );
    virtual void    OnPadShoulder           ( ui_win* pWin, s32 Direction );
    virtual void    OnPadShoulder2          ( ui_win* pWin, s32 Direction );
    virtual void    OnPadSelect             ( ui_win* pWin );
    virtual void    OnPadBack               ( ui_win* pWin );
    virtual void    OnCursorMove            ( ui_win* pWin, s32 x, s32 y );
    virtual void    OnLBDown                ( ui_win* pWin );
    virtual void    OnLBUp                  ( ui_win* pWin );
    virtual void    OnUpdate                ( ui_win* pWin, f32 DeltaTime );
    virtual void    OnCursorEnter           ( ui_win* pWin );
    virtual void    OnCursorExit            ( ui_win* pWin );

    void            SetLineHeight           ( s32 Height );

    void            SetExitOnSelect         ( xbool State );
    void            SetExitOnBack           ( xbool State )                             { m_ExitOnBack = State; }

    s32             AddItem                 ( const xwstring& Item, s32 Data = 0, s32 Data2 = 0 , xbool State = TRUE, u32 Flags = 0 );
    s32             AddItem                 ( const xwchar*   Item, s32 Data = 0, s32 Data2 = 0 , xbool State = TRUE, u32 Flags = 0 );
    void            DeleteAllItems          ( void );
    void            DeleteItem              ( s32 iItem );
    void            DeleteSelectedItem      ( void );
    void            EnableItem              ( s32 iItem, xbool State );
    u32             GetItemFlags            ( s32 iItem );

    void            EnableBorders           ( void )                                    { m_ShowBorders = TRUE; }
    void            DisableBorders          ( void )                                    { m_ShowBorders = FALSE; }

    void            EnableFrame             ( void )                                    { m_ShowFrame = TRUE; }
    void            DisableFrame            ( void )                                    { m_ShowFrame = FALSE; }

    void            EnableHeaderBar         ( void );
    void            DisableHeaderBar        ( void );

    void            SetHeaderBarColor       ( xcolor Color )                            { m_HeaderBarColor = Color; }
    void            SetHeaderColor          ( xcolor Color )                            { m_HeaderColor = Color; }

    void            EnableParentNavigation  ( void )                                    { m_AllowParentNavigate = TRUE;  }
    void            DisableParentNavigation ( void )                                    { m_AllowParentNavigate = FALSE; }

    void            EnableCursor            ( void )                                    { m_DisableCursor = FALSE;  }
    void            DisableCursor           ( void )                                    { m_DisableCursor = TRUE; }

    item&           GetItem                 ( s32 Index )                               { return m_Items[Index]; }
    s32             GetItemCount            ( void ) const;
    const xwstring& GetItemLabel            ( s32 iItem ) const;
    void            SetItemLabel            ( s32 iItem, const xwstring& Label );
    s32             GetItemData             ( s32 iItem, s32 Index = 0 ) const;
    const xwstring& GetSelectedItemLabel    ( void ) const;
    s32             GetSelectedItemData     ( s32 Index = 0 ) const;
    void            SetItemColor            ( s32 iItem, const xcolor& Color );
    xcolor          GetItemColor            ( s32 iItem ) const;

    s32             FindItemByLabel         ( const xwstring& Label );
    s32             FindItemByData          ( s32 Data, s32 Index = 0 );

    s32             GetSelection            ( void ) const;
    void            SetSelection            ( s32 iSelection );
    void            ClearSelection          ( void );

    void            EnsureVisible           ( s32 iItem );
    s32             GetNumEnabledItems      ( void );
    
    s32             GetCursorOffset         ( void );
    void            SetSelectionWithOffset  ( s32 iSelection, s32 Offset );

    void            SetBackgroundColor      ( xcolor Color );
    xcolor          GetBackgroundColor      ( void ) const;

    void            AlphaSortList           ( void );

protected:
    xbool           m_ExitOnSelect;
    xbool           m_ExitOnBack;
    xbool           m_ShowBorders;
    xbool           m_ShowFrame;
    xbool           m_ShowHeaderBar;
    xbool           m_AllowParentNavigate;
    xbool           m_DisableCursor;
    s32             m_iElementFrame;
    s32             m_iElement_sb_arrowdown;
    s32             m_iElement_sb_arrowup;
    s32             m_iElement_sb_container;
    s32             m_iElement_sb_thumb;
#ifdef TARGET_PC
    s32             m_TrackHighLight;
    s32             m_CursorX;
    s32             m_CursorY;
    irect           m_UpArrow;
    irect           m_DownArrow;
    irect           m_ScrollBar;
    xbool           m_MouseDown;
    xbool           m_ScrollDown;
    f32             m_ScrollTime;
#endif

    xarray<item>    m_Items;
    s32             m_iSelection;
    s32             m_iSelectionBackup;
    s32             m_iFirstVisibleItem;
    s32             m_nVisibleItems;
    s32             m_LineHeight;

    xcolor          m_BackgroundColor;
    xcolor          m_HeaderBarColor;
    xcolor          m_HeaderColor;
};

//==============================================================================
#endif // UI_LISTBOX_HPP
//==============================================================================
