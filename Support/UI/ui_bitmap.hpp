//==============================================================================
//  
//  ui_bitmap.hpp
//  
//==============================================================================

#ifndef UI_BITMAP_HPP
#define UI_BITMAP_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#include "x_math.hpp"
#endif

#include "ui_control.hpp"

//==============================================================================
//  ui_bitmap
//==============================================================================

extern ui_win* ui_bitmap_factory( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags );

class ui_bitmap : public ui_control
{
public:
                    ui_bitmap           ( void );
    virtual        ~ui_bitmap           ( void );

    xbool           Create              ( s32           UserID,
                                          ui_manager*   pManager,
                                          const irect&  Position,
                                          ui_win*       pParent,
                                          s32           Flags );

    virtual void    Render              ( s32 ox=0, s32 oy=0 );
    virtual void    OnUpdate            ( f32 DeltaTime );

    void            SetBitmap           ( s32 BitmapID, xbool bIsElement = FALSE, s32 State = 0);

protected:
    s32     m_BitmapID;
    xbool   m_bIsElement;
    s32     m_RenderState;
};

//==============================================================================
#endif // UI_BITMAP_HPP
//==============================================================================
