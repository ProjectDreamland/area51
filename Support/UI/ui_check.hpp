//==============================================================================
//  
//  ui_check.hpp
//  
//==============================================================================

#ifndef UI_CHECK_HPP
#define UI_CHECK_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#include "x_math.hpp"
#endif

#include "ui_control.hpp"

//==============================================================================
//  ui_check
//==============================================================================

extern ui_win* ui_check_factory( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags );

class ui_check : public ui_control
{
public:
                    ui_check            ( void );
    virtual        ~ui_check            ( void );

    xbool           Create              ( s32           UserID,
                                          ui_manager*   pManager,
                                          const irect&  Position,
                                          ui_win*       pParent,
                                          s32           Flags );

    virtual void    Render              ( s32 ox=0, s32 oy=0 );

    virtual void    OnPadSelect         ( ui_win* pWin );
    virtual void    OnLBDown            ( ui_win* pWin );

    void            SetChecked          ( xbool State );
    xbool           IsChecked           ( void ) const;

protected:
    s32             m_iElement;
    xbool           m_bIsChecked;
};

//==============================================================================
#endif // UI_CHECK_HPP
//==============================================================================
