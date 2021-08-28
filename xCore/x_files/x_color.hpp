//==============================================================================
//  
//  x_color.hpp
//  
//==============================================================================

#ifndef X_COLOR_HPP
#define X_COLOR_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

//==============================================================================
//  TYPES
//==============================================================================

struct xcolor
{

//------------------------------------------------------------------------------
//  Fields

    u8  B, G, R, A;     // Values are in [ 0, 255 ].

//------------------------------------------------------------------------------
//  Functions

                xcolor              ( void );
                xcolor              ( const xcolor& C );
                xcolor              ( u8 R, u8 G, u8 B, u8 A = 255 );
                xcolor              ( u32 ARGB );
                            
        void    Set                 ( u8 R, u8 G, u8 B, u8 A = 255 );
        void    Set                 ( u32 ARGB );
        void    SetfRGBA            ( f32 R, f32 G, f32 B, f32 A = 1);                     
        void    GetfRGBA            ( f32& aR, f32& aG, f32& aB, f32& aA );
        void    Random              ( u8 A = 255 );

const   xcolor& operator =          ( const xcolor& C );
const   xcolor& operator =          ( u32 ARGB );
        xcolor& operator +=         ( xcolor C );

        u32     GetRGBA             ( void ) const;

                operator const u32  ( void ) const;

    // How about:
    //  c *  c
    //  c *= c
    //  c *  s
    //  s *  c
    //  c *= s
    //  division also

    friend xcolor operator * ( xcolor C, f32 I );
};                 

#include "Implementation/x_color_inline.hpp"

//==============================================================================

#define ARGB(a,r,g,b) (((u32)(a)<<24)|((u32)(r)<<16)|((u32)(g)<<8)|(u32)(b))

#define XCOLOR_BLACK    xcolor(   0,   0,   0 )
#define XCOLOR_WHITE    xcolor( 255, 255, 255 )
#define XCOLOR_RED      xcolor( 255,   0,   0 )   
#define XCOLOR_GREEN    xcolor(   0, 255,   0 )   
#define XCOLOR_BLUE     xcolor(   0,   0, 255 )   
#define XCOLOR_AQUA     xcolor(   0, 255, 255 )   
#define XCOLOR_PURPLE   xcolor( 255,   0, 255 )
#define XCOLOR_YELLOW   xcolor( 255, 255,   0 )   
#define XCOLOR_GREY     xcolor( 127, 127, 127 )

//==============================================================================
#endif // X_COLOR_HPP
//==============================================================================
