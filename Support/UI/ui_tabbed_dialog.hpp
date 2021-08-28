//==============================================================================
//  
//  ui_tabbed_dialog.hpp
//  
//==============================================================================

#ifndef UI_TABBED_DIALOG_HPP
#define UI_TABBED_DIALOG_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#include "x_math.hpp"
#endif

#include "ui_win.hpp"
#include "ui_manager.hpp"
#include "ui_dialog.hpp"

//==============================================================================
//  ui_tabbed_dialog
//==============================================================================

extern ui_win* ui_tabbed_dialog_factory( s32 UserID, ui_manager* pManager, ui_manager::dialog_tem* pDialogTem, const irect& Position, ui_win* pParent, s32 Flags, void* pUserData );

class ui_tabbed_dialog : public ui_dialog
{
    friend ui_manager;

    struct tab
    {
        s32             w;
        xwstring        Label;
        ui_dialog*      pDialog;
    };

public:
                    ui_tabbed_dialog    ( void );
    virtual        ~ui_tabbed_dialog    ( void );

    xbool           Create              ( s32                       UserID,
                                          ui_manager*               pManager,
                                          ui_manager::dialog_tem*   pDialogTem,
                                          const irect&              Position,
                                          ui_win*                   pParent,
                                          s32                       Flags,
                                          void*                     pUserData );

    virtual void    Render              ( s32 ox=0, s32 oy=0 );

    virtual void    OnPadNavigate       ( ui_win* pWin, s32 Code, s32 Presses, s32 Repeats, xbool WrapX = FALSE, xbool WrapY = FALSE );
    virtual void    OnPadSelect         ( ui_win* pWin );
    virtual void    OnPadBack           ( ui_win* pWin );
    virtual void    OnLBDown            ( ui_win* pWin );
    virtual void    OnCursorMove        ( ui_win* pWin, s32 x, s32 y );

    void            SetBackgroundColor  ( xcolor Color );
    xcolor          GetBackgroundColor  ( void ) const ;

    s32             AddTab              ( const xwstring& Label, ui_dialog* pDialog );
    void            ActivateTab         ( s32 iTab );
    void            ActivateTab         ( ui_dialog* pDialog );
    s32             GetActiveTab        ( void ) const;

    void            SetTabWidth         ( s32 TabWidth );
    s32             GetTabWidth         ( void ) const;

    const xwstring& GetTabLabel         ( s32 iTab ) const;

    void            SetTabTracking      ( s32* pTabTracker );
    void            FitTabs             ( void );

protected:
    s32                     m_iElementFrame;
    s32                     m_iElementTab;
    xarray<tab>             m_Tabs;
    s32                     m_iActiveTab;
    s32                     m_TabWidth;
    s32*                    m_pTabTracker;
#ifdef TARGET_PC
    s32                     m_CursorX;
    s32                     m_CursorY;
#endif
};

//==============================================================================
#endif // UI_TABBED_DIALOG_HPP
//==============================================================================
