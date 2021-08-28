//==============================================================================
//  
//  ui_control.hpp
//  
//==============================================================================

#ifndef UI_CONTROL_HPP
#define UI_CONTROL_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#include "x_math.hpp"
#endif

#include "ui_win.hpp"

//==============================================================================
//  ui_control
//==============================================================================

class ui_control : public ui_win
{
public:
    enum state
    {
        CS_DISABLED = 0,
        CS_ACTIVE   = 1,
        CS_PASSIVE  = 2,
    };

public:
                    ui_control          ( void );
    virtual        ~ui_control          ( void );

    xbool           Create              ( s32           UserID,       
                                          ui_manager*   pManager,
                                          const irect&  Position,
                                          ui_win*       pParent,
                                          s32           Flags );

    virtual void    Render              ( s32 ox=0, s32 oy=0 );

    const irect&    GetNavPos           ( void );
    void            SetNavPos           ( const irect& r );

protected:
    irect               m_NavPos;
};

//==============================================================================
#endif // UI_CONTROL_HPP
//==============================================================================
