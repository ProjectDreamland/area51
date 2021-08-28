//==============================================================================
//
//  x_color_inline.hpp
//
//==============================================================================

#ifndef X_COLOR_INLINE_HPP
#define X_COLOR_INLINE_HPP
#else
#error "File " __FILE__ " has been included twice!"
#endif

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_PLUS_HPP
#include "x_plus.hpp"
#endif

//==============================================================================
//  FUNCTIONS
//==============================================================================

inline xcolor::xcolor( void )
{
}

//==============================================================================

inline xcolor::xcolor( const xcolor& C )
{
#ifdef TARGET_PS2
    ASSERT( ((u32)this & 0x03) == 0 );
    *(u32*)&B = *(u32*)&C.B;
#else
    A = C.A;
    R = C.R;
    G = C.G;
    B = C.B;
#endif
}

//==============================================================================

inline xcolor::xcolor( u8 aR, u8 aG, u8 aB, u8 aA )
{   
#ifdef TARGET_PS2
    *(u32*)&B = ((u32)aA<<24) | ((u32)aR<<16) | ((u32)aG<<8) | ((u32)aB);
#else
    A = aA;
    R = aR;
    G = aG;
    B = aB;
#endif
}

//==============================================================================

inline xcolor::xcolor( u32 ARGB )
{   
#ifdef TARGET_PS2
    ASSERT( ((u32)this & 0x03) == 0 );
    *(u32*)&B = ARGB;
#else
    A = (u8)((ARGB & 0xFF000000) >> 24);
    R = (u8)((ARGB & 0x00FF0000) >> 16);
    G = (u8)((ARGB & 0x0000FF00) >>  8);
    B = (u8)((ARGB & 0x000000FF) >>  0);
#endif
}

//==============================================================================

inline void xcolor::Set( u8 aR, u8 aG, u8 aB, u8 aA )
{   
    A = aA;
    R = aR;
    G = aG;
    B = aB;
}

//==============================================================================

inline void xcolor::Set( u32 ARGB )
{   
#ifdef TARGET_PS2
    ASSERT( ((u32)this & 0x03) == 0 );
    *(u32*)&B = ARGB;
#else
    A = (u8)((ARGB & 0xFF000000) >> 24);
    R = (u8)((ARGB & 0x00FF0000) >> 16);
    G = (u8)((ARGB & 0x0000FF00) >>  8);
    B = (u8)((ARGB & 0x000000FF) >>  0);
#endif
}

//==============================================================================

inline void xcolor::Random( u8 aA )
{
    A = aA;
    B = (u8)x_irand( 0, 255 );
    G = (u8)x_irand( 0, 255 );
    R = (u8)x_irand( 0, 255 );
}

//==============================================================================

inline const xcolor& xcolor::operator = ( const xcolor& C )
{
#ifdef TARGET_PS2
    ASSERT( ((u32)this & 0x03) == 0 );
    *(u32*)&B = *(u32*)&C.B;
#else
    A = C.A;
    R = C.R;
    G = C.G;
    B = C.B;
#endif

    return( *this );
}

//==============================================================================

inline const xcolor& xcolor::operator = ( u32 ARGB )
{   
#ifdef TARGET_PS2
    ASSERT( ((u32)this & 0x03) == 0 );
    *(u32*)&B = ARGB;
#else
    A = (u8)((ARGB & 0xFF000000) >> 24);
    R = (u8)((ARGB & 0x00FF0000) >> 16);
    G = (u8)((ARGB & 0x0000FF00) >>  8);
    B = (u8)((ARGB & 0x000000FF) >>  0);
#endif

    return( *this );
}

//==============================================================================

inline xcolor::operator const u32( void ) const
{
    return( *((u32*)this) );
}

//==============================================================================

inline u32 xcolor::GetRGBA( void ) const
{
    return ( R << 24 ) | ( G << 16 ) | ( B << 8 ) | A;
}

//==============================================================================

inline void xcolor::SetfRGBA( f32 aR, f32 aG, f32 aB, f32 aA )
{
    R = (u8)fMin( fMax(aR*255, 0 ), 255 );
    G = (u8)fMin( fMax(aG*255, 0 ), 255 );
    B = (u8)fMin( fMax(aB*255, 0 ), 255 );
    A = (u8)fMin( fMax(aA*255, 0 ), 255 );
}

//==============================================================================

inline void xcolor::GetfRGBA( f32& aR, f32& aG, f32& aB, f32& aA )
{
    f32 c = 1.0f/255.0f;
    aR = R*c;
    aG = G*c;
    aB = B*c;
    aA = A*c;
}

//==============================================================================
inline
xcolor operator * ( xcolor C, f32 I )
{
    I = fMin( 1, I );
    I = fMax( 0, I );
    return xcolor( (u8)(C.R * I), (u8)(C.G * I), (u8)(C.B * I), C.A );
}

//==============================================================================
inline
xcolor& xcolor::operator += ( xcolor C )
{
    R = (u8)iMin( 255, R + (s32)C.R);
    G = (u8)iMin( 255, G + (s32)C.G);
    B = (u8)iMin( 255, B + (s32)C.B);
    return *this;
}