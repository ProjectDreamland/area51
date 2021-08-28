//==============================================================================
//
//  x_math_v4_inline.hpp
//
//==============================================================================

#ifndef X_MATH_V4_INLINE_HPP
#define X_MATH_V4_INLINE_HPP
#else
#error "File " __FILE__ " has been included twice!"
#endif

#ifndef X_DEBUG_HPP
#include "../x_debug.hpp"
#endif

//==============================================================================

inline 
vector4::vector4( void )
{
    FORCE_ALIGNED_16( this );
}

//==============================================================================

inline 
vector4::vector4( const vector3& V )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    asm( "vmove.xyz VOUT, VEC
          vmove.w   VOUT, vf00" : "=j VOUT" (XYZW) : "j VEC" (V.GetU128()) );
#else
    GetX() = V.GetX();
    GetY() = V.GetY();
    GetZ() = V.GetZ();
    GetW() = 0.0f;
#endif
}

//==============================================================================

inline 
vector4::vector4( f32 aX, f32 aY, f32 aZ, f32 aW )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128    XY;
    u128    ZW;
    asm( "pextlw X_Y,  AY,  AX"  : "=r X_Y"  (XY)   : "r AX"  (aX), "r AY"  (aY) );
    asm( "pextlw Z_W,  AW,  AZ"  : "=r Z_W"  (ZW)   : "r AZ"  (aZ), "r AW"  (aW) );
    asm( "pcpyld XYZW, Z_W, X_Y" : "=r XYZW" (XYZW) : "r X_Y" (XY), "r Z_W" (ZW) );
#else
    GetX() = aX;
    GetY() = aY;
    GetZ() = aZ;
    GetW() = aW;
#endif
}

//==============================================================================

#ifdef TARGET_PS2
inline
vector4::vector4( u128 V )
{
    XYZW = V;
}
#endif

//==============================================================================

inline
f32 vector4::GetX( void ) const
{
#if USE_VU0
    f32 Result;
    asm( "qmfc2 FOUT, VIN" : "=r FOUT" (Result) : "j VIN" (XYZW) );
    return Result;
#else
    return X;
#endif
}

//==============================================================================

inline
f32& vector4::GetX( void )
{
#if USE_VU0
    return ((f32*)this)[0];
#else
    return X;
#endif
}

//==============================================================================

inline
f32 vector4::GetY( void ) const
{
#if USE_VU0
    f32  Result;
    u128 Temp;
    asm( "vaddy.x TEMP, vf00, VIN" : "=j TEMP" (Temp)   : "j VIN"  (XYZW) );
    asm( "qmfc2  FOUT, TEMP"       : "=r FOUT" (Result) : "j TEMP" (Temp) );
    return Result;
#else
    return Y;
#endif
}

//==============================================================================

inline
f32& vector4::GetY( void )
{
#if USE_VU0
    return ((f32*)this)[1];
#else
    return Y;
#endif
}

//==============================================================================

inline
f32 vector4::GetZ( void ) const
{
#if USE_VU0
    f32  Result;
    u128 Temp;
    asm( "vaddz.x TEMP, vf00, VIN" : "=j TEMP" (Temp)   : "j VIN"  (XYZW) );
    asm( "qmfc2  FOUT, TEMP"       : "=r FOUT" (Result) : "j TEMP" (Temp) );
    return Result;
#else
    return Z;
#endif
}

//==============================================================================

inline
f32& vector4::GetZ( void )
{
#if USE_VU0
    return ((f32*)this)[2];
#else
    return Z;
#endif
}

//==============================================================================

inline
f32 vector4::GetW( void ) const
{
#if USE_VU0
    f32  Result;
    u128 Temp;
    asm( "vaddw.x TEMP, vf00, VIN" : "=j TEMP" (Temp)   : "j VIN"  (XYZW) );
    asm( "qmfc2  FOUT, TEMP"       : "=r FOUT" (Result) : "j TEMP" (Temp) );
    return Result;
#else
    return W;
#endif
}

//==============================================================================

inline
f32& vector4::GetW( void )
{
#if USE_VU0
    return ((f32*)this)[3];
#else
    return W;
#endif
}

//==============================================================================

inline
s32 vector4::GetIW( void ) const
{
#if USE_VU0
    return ((s32*)this)[3];
#else
    return iW;
#endif
}

//==============================================================================

inline
s32& vector4::GetIW( void )
{
#if USE_VU0
    return ((s32*)this)[3];
#else
    return iW;
#endif
}

//==============================================================================

#ifdef TARGET_PS2
inline
u128 vector4::GetU128( void ) const
{
    return XYZW;
}
#endif

//==============================================================================

inline 
void vector4::Set( f32 aX, f32 aY, f32 aZ, f32 aW )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128    XY;
    u128    ZW;
    asm( "pextlw X_Y,  AY,  AX"  : "=r X_Y"  (XY)   : "r AX"  (aX), "r AY"  (aY) );
    asm( "pextlw Z_W,  AW,  AZ"  : "=r Z_W"  (ZW)   : "r AZ"  (aZ), "r AW"  (aW) );
    asm( "pcpyld XYZW, Z_W, X_Y" : "=r XYZW" (XYZW) : "r X_Y" (XY), "r Z_W" (ZW) );
#else
    GetX() = aX;
    GetY() = aY;
    GetZ() = aZ;
    GetW() = aW;
#endif
}

//==============================================================================

inline 
void vector4::operator () ( f32 aX, f32 aY, f32 aZ, f32 aW )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128    XY;
    u128    ZW;
    asm( "pextlw X_Y,  AY,  AX"  : "=r X_Y"  (XY)   : "r AX"  (aX), "r AY"  (aY) );
    asm( "pextlw Z_W,  AW,  AZ"  : "=r Z_W"  (ZW)   : "r AZ"  (aZ), "r AW"  (aW) );
    asm( "pcpyld XYZW, Z_W, X_Y" : "=r XYZW" (XYZW) : "r X_Y" (XY), "r Z_W" (ZW) );
#else
    GetX() = aX;
    GetY() = aY;
    GetZ() = aZ;
    GetW() = aW;
#endif
}

//==============================================================================

inline
f32 vector4::operator [] ( s32 Index ) const
{
    FORCE_ALIGNED_16( this );

    ASSERT( (Index >= 0) && (Index < 4) );
    return( ((f32*)this)[Index] );
}

//==============================================================================

inline
f32& vector4::operator [] ( s32 Index )
{
    FORCE_ALIGNED_16( this );

    ASSERT( (Index >= 0) && (Index < 4) );
    return( ((f32*)this)[Index] );
}

//==============================================================================

inline 
void vector4::Zero( void )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    asm( "vsub.xyzw XYZW, vf00, vf00" : "=j XYZW" (XYZW) );
#else
    GetX() = GetY() = GetZ() = GetW() = 0.0f;
#endif
}

//==============================================================================

inline 
void vector4::Negate( void )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    asm( "vsuba.xyzw  acc,  vf00, vf00
          vmsubw.xyzw VOUT, VIN,  vf00w" : "=j VOUT" (XYZW) : "j VIN" (XYZW) );
#else
    GetX() = -GetX();
    GetY() = -GetY();
    GetZ() = -GetZ();
    GetW() = -GetW();
#endif
}

//==============================================================================

inline 
void vector4::Normalize( void )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128 DotProduct;
    asm( "vmul.xyzw     DOT,  XYZW,  XYZW" : "=j DOT"  (DotProduct) : "j XYZW" (XYZW)       );
    asm( "vaddw.x       DOUT, DIN,   DINw
          vaddz.x       DOUT, DOUT,  DINz
          vaddy.x       DOUT, DOUT,  DINy" : "=j DOUT" (DotProduct) : "j DIN"  (DotProduct) );
    asm( "vrsqrt        Q,    vf00w, DOTx
          vwaitq
          vmulq.xyzw    VOUT, VIN,   Q"    :
         "=j VOUT" (XYZW) :
         "j  VIN"  (XYZW),
         "j  DOT"  (DotProduct) );
#else
    f32 N = x_1sqrt(GetX()*GetX() + GetY()*GetY() + GetZ()*GetZ() + GetW()*GetW());
    ASSERT( x_isvalid(N) );

    GetX() *= N;
    GetY() *= N;
    GetZ() *= N;
    GetW() *= N;
#endif
}

//==============================================================================

inline 
void vector4::NormalizeAndScale( f32 Scalar )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128 DotProduct;
    u128 ScaleVec;
    asm( "qmtc2         FSCL, VSCL"        : "=j VSCL" (ScaleVec)   : "r FSCL" (Scalar)     );
    asm( "vmul.xyzw     DOT,  XYZW,  XYZW" : "=j DOT"  (DotProduct) : "j XYZW" (XYZW)       );
    asm( "vaddw.x       DOUT, DIN,   DINw
          vaddz.x       DOUT, DOUT,  DINz
          vaddy.x       DOUT, DOUT,  DINy" : "=j DOUT" (DotProduct) : "j DIN"  (DotProduct) );
    asm( "vrsqrt        Q,    VSCLx, DOTx
          vwaitq
          vmulq.xyzw    VOUT, VIN,   Q"    :
         "=j VOUT" (XYZW) :
         "j  VIN"  (XYZW),
         "j  VSCL" (ScaleVec),
         "j  DOT"  (DotProduct) );
#else
    f32 N = x_1sqrt(GetX()*GetX() + GetY()*GetY() + GetZ()*GetZ() + GetW()*GetW());
    ASSERT( x_isvalid(N) );

    N      *= Scalar;
    GetX() *= N;
    GetY() *= N;
    GetZ() *= N;
    GetW() *= N;
#endif
}

//==============================================================================

inline 
xbool vector4::SafeNormalize( void )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128    DotVec;
    f32     Dot;

    asm( "vmul.xyzw DOUT,   XYZW,   XYZW" : "=j DOUT" (DotVec) : "j XYZW" (XYZW) );
    asm( "vaddw.x   DOUT,   DIN,    DINw
          vaddz.x   DOUT,   DOUT,   DINz
          vaddy.x   DOUT,   DOUT,   DINy" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "qmfc2     DOT,    DIN"          : "=r DOT"  (Dot)    : "j DIN"  (DotVec) );
    
    Dot = x_1sqrt(Dot);
    if ( x_isvalid(Dot) )
    {
        asm( "qmtc2         FDOT, VDOT"        : "=j VDOT" (DotVec) : "r FDOT" (Dot) );
        asm( "vmulx.xyzw    VOUT, VIN,  DOTx"  : "=j VOUT" (XYZW)   : "j VIN"  (XYZW), "j DOT"  (DotVec) );
        return TRUE;
    }
    else
    {
        Zero();
        return FALSE;
    }
#else
    f32 N = x_1sqrt(GetX()*GetX() + GetY()*GetY() + GetZ()*GetZ() + GetW()*GetW());

    if( x_isvalid(N) )
    {
        GetX() *= N;
        GetY() *= N;
        GetZ() *= N;
        GetW() *= N;
        return TRUE;
    }
    else
    {
        GetX() = 0;
        GetY() = 0;
        GetZ() = 0;
        GetW() = 0;
        return FALSE;
    }
#endif
}

//==============================================================================

inline 
xbool vector4::SafeNormalizeAndScale( f32 Scalar )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128    DotVec;
    f32     Dot;

    asm( "vmul.xyzw DOUT,   XYZW,   XYZW" : "=j DOUT" (DotVec) : "j XYZW" (XYZW) );
    asm( "vaddw.x   DOUT,   DIN,    DINw
          vaddz.x   DOUT,   DOUT,   DINz
          vaddy.x   DOUT,   DOUT,   DINy" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "qmfc2     DOT,    DIN"          : "=r DOT"  (Dot)    : "j DIN"  (DotVec) );
    
    Dot = x_1sqrt(Dot);
    if ( x_isvalid(Dot) )
    {
        Dot *= Scalar;
        asm( "qmtc2         FDOT, VDOT"        : "=j VDOT" (DotVec) : "r FDOT" (Dot) );
        asm( "vmulx.xyzw    VOUT, VIN,  DOTx"  : "=j VOUT" (XYZW)   : "j VIN"  (XYZW), "j DOT" (DotVec) );
        return TRUE;
    }
    else
    {
        Zero();
        return FALSE;
    }
#else
    f32 N = x_1sqrt(GetX()*GetX() + GetY()*GetY() + GetZ()*GetZ() + GetW()*GetW());

    if( x_isvalid(N) )
    {
        N  *= Scalar;
        GetX() *= N;
        GetY() *= N;
        GetZ() *= N;
        GetW() *= N;
        return TRUE;
    }
    else
    {
        GetX() = 0.0f;
        GetY() = 0.0f;
        GetZ() = 0.0f;
        GetW() = 0.0f;
        return FALSE;
    }
#endif
}

//==============================================================================

inline 
void vector4::Scale( f32 Scalar )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128 ScaleVec;
    asm( "qmtc2      FSCL, VSCL"       : "=j VSCL" (ScaleVec) : "r FSCL" (Scalar) );
    asm( "vmulx.xyzw VOUT, VIN, VSCLx" : "=j VOUT" (XYZW)     : "j VIN"  (XYZW), "j VSCL" (ScaleVec) );
#else
    GetX() *= Scalar; 
    GetY() *= Scalar; 
    GetZ() *= Scalar;
    GetW() *= Scalar;
#endif
}

//==============================================================================

inline 
f32 vector4::Length( void ) const
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128 DotVec;
    f32  Dot;
    asm( "vmul.xyzw DOUT, XYZW, XYZW" : "=j DOUT" (DotVec) : "j XYZW" (XYZW) );
    asm( "vaddw.x   DOUT, DIN,  DINw
          vaddz.x   DOUT, DOUT, DINz
          vaddy.x   DOUT, DOUT, DINy" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "qmfc2     DOT,  DIN"        : "=r DOT"  (Dot)    : "j DIN"  (DotVec) );
    return x_sqrt(Dot);
#else
    return( x_sqrt( GetX()*GetX() + GetY()*GetY() + GetZ()*GetZ() + GetW()*GetW() ) );
#endif
}

//==============================================================================

inline
f32 vector4::LengthSquared( void ) const
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128 DotVec;
    f32  Dot;
    asm( "vmul.xyzw DOUT, XYZW, XYZW" : "=j DOUT" (DotVec) : "j XYZW" (XYZW) );
    asm( "vaddw.x   DOUT, DIN,  DINw
          vaddz.x   DOUT, DOUT, DINz
          vaddy.x   DOUT, DOUT, DINy" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "qmfc2     DOT,  DIN"        : "=r DOT"  (Dot)    : "j DIN"  (DotVec) );
    return Dot;
#else
    return( GetX()*GetX() + GetY()*GetY() + GetZ()*GetZ() + GetW()*GetW() );
#endif
}

//==============================================================================

inline
vector4::operator const f32* ( void ) const
{
    FORCE_ALIGNED_16( this );
    return( (f32*)this );
}

//==============================================================================

inline 
vector4 vector4::operator - ( void ) const
{
    FORCE_ALIGNED_16( this );
#if USE_VU0
    vector4 Result;
    asm( "vsuba.xyzw  acc,  vf00, vf00
          vmsubw.xyzw VOUT, VIN,  vf00w" : "=j VOUT" (Result.XYZW) : "j VIN" (XYZW) );
    return Result;
#else
    return( vector4( -GetX(), -GetY(), -GetZ(), -GetW() ) );
#endif
}

//==============================================================================

inline 
const vector4& vector4::operator = ( const vector3& V )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    asm( "vmove.xyz  XYZW, VEC
          vmove.w    XYZW, vf00" : "=j XYZW" (XYZW) : "j VEC" (V.GetU128()) );
#else
    GetX() = V.GetX();
    GetY() = V.GetY();
    GetZ() = V.GetZ();
    GetW() = 0.0f;
#endif
    return( *this );
}

//==============================================================================

inline 
vector4& vector4::operator += ( const vector4& V )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    asm( "vadd.xyzw     VOUT, VEC0, VEC1" : "=j VOUT" (XYZW) : "j VEC0" (XYZW), "j VEC1" (V.XYZW) );
#else
    GetX() += V.GetX();
    GetY() += V.GetY();
    GetZ() += V.GetZ();
    GetW() += V.GetW();
#endif
    return( *this );
}

//==============================================================================

inline 
vector4& vector4::operator -= ( const vector4& V )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    asm( "vsub.xyzw     VOUT, VEC0, VEC1" : "=j VOUT" (XYZW) : "j VEC0" (XYZW), "j VEC1" (V.XYZW) );
#else
    GetX() -= V.GetX();
    GetY() -= V.GetY();
    GetZ() -= V.GetZ();
    GetW() -= V.GetW();
#endif
    return( *this );
}

//==============================================================================

inline 
vector4& vector4::operator *= ( const f32 Scalar )
{
    FORCE_ALIGNED_16( this );
#if USE_VU0
    u128 ScaleVec;
    asm( "qmtc2         FSCL, VSCL"       : "=j VSCL" (ScaleVec) : "r FSCL" (Scalar) );
    asm( "vmulx.xyzw    VOUT, VIN, VSCLx" : "=j VOUT" (XYZW)     : "j VIN"  (XYZW), "j VSCL" (ScaleVec) );
#else
    GetX() *= Scalar; 
    GetY() *= Scalar; 
    GetZ() *= Scalar;
    GetW() *= Scalar;
#endif
    return( *this );
}

//==============================================================================

inline 
vector4& vector4::operator /= ( const f32 Scalar )
{
    FORCE_ALIGNED_16( this );

    ASSERT( Scalar != 0.0f );
    f32 d = 1.0f / Scalar;

#if USE_VU0
    u128 ScaleVec;
    asm( "qmtc2         FSCL, VSCL"       : "=j VSCL" (ScaleVec) : "r FSCL" (d) );
    asm( "vmulx.xyzw    VOUT, VIN, VSCLx" : "=j VOUT" (XYZW)     : "j VIN"  (XYZW), "j VSCL" (ScaleVec) );
#else
    GetX() *= d; 
    GetY() *= d; 
    GetZ() *= d;
    GetW() *= d;
#endif
    return( *this );
}

//==============================================================================

inline 
xbool vector4::operator == ( const vector4& V ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

    return( (GetX() == V.GetX()) && (GetY() == V.GetY()) && (GetZ() == V.GetZ()) && (GetW() == V.GetW()) );
}

//==============================================================================

inline 
xbool vector4::operator != ( const vector4& V ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

    return( (GetX() != V.GetX()) || (GetY() != V.GetY()) || (GetZ() != V.GetZ()) || (GetW() != V.GetW()) );
}

//==============================================================================

inline 
vector4 operator + ( const vector4& V1, const vector4& V2 )
{
    FORCE_ALIGNED_16( &V1 );
    FORCE_ALIGNED_16( &V2 );

#if USE_VU0
    vector4 Result;
    asm( "vadd.xyzw   XYZW, V1, V2" : "=j XYZW" (Result.XYZW) : "j V1" (V1.XYZW), "j V2" (V2.XYZW) );
    return Result;
#else
    return( vector4( V1.GetX() + V2.GetX(),
                     V1.GetY() + V2.GetY(),
                     V1.GetZ() + V2.GetZ(),
                     V1.GetW() + V2.GetW() ) );
#endif
}

//==============================================================================

inline 
vector4 operator - ( const vector4& V1, const vector4& V2 )
{
    FORCE_ALIGNED_16( &V1 );
    FORCE_ALIGNED_16( &V2 );

#if USE_VU0
    vector4 Result;
    asm( "vsub.xyzw XYZW, V1, V2" : "=j XYZW" (Result.XYZW) : "j V1" (V1.XYZW), "j V2" (V2.XYZW) );
    return Result;
#else
    return( vector4( V1.GetX() - V2.GetX(),
                     V1.GetY() - V2.GetY(),
                     V1.GetZ() - V2.GetZ(),
                     V1.GetW() - V2.GetW() ) );
#endif
}

//==============================================================================

inline 
vector4 operator / ( const vector4& V, f32 S )
{
    FORCE_ALIGNED_16( &V );

    ASSERT( (S > 0.00001f) || (S < -0.00001f) );
    S = 1.0f / S;

#if USE_VU0
    u128    ScaleVec;
    vector4 Result;
    asm( "qmtc2         FSCL, VSCL"       : "=j VSCL" (ScaleVec)    : "r FSCL" (S) );
    asm( "vmulx.xyzw    VOUT, VIN, VSCLx" : "=j VOUT" (Result.XYZW) : "j VIN"  (V.XYZW), "j VSCL" (ScaleVec) );
    return Result;
#else
    return( vector4( V.GetX() * S, 
                     V.GetY() * S, 
                     V.GetZ() * S, 
                     V.GetW() * S ) );
#endif
}

//==============================================================================

inline 
vector4 operator * ( const vector4& V, f32 S )
{
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    u128    ScaleVec;
    vector4 Result;
    asm( "qmtc2         FSCL, VSCL"       : "=j VSCL" (ScaleVec)    : "r FSCL" (S) );
    asm( "vmulx.xyzw    VOUT, VIN, VSCLx" : "=j VOUT" (Result.XYZW) : "j VIN"  (V.XYZW), "j VSCL" (ScaleVec) );
    return Result;
#else
    return( vector4( V.GetX() * S, 
                     V.GetY() * S, 
                     V.GetZ() * S, 
                     V.GetW() * S ) );
#endif
}

//==============================================================================

inline 
vector4 operator * ( f32 S, const vector4& V )
{
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    u128    ScaleVec;
    vector4 Result;
    asm( "qmtc2         FSCL, VSCL"       : "=j VSCL" (ScaleVec)    : "r FSCL" (S) );
    asm( "vmulx.xyzw    VOUT, VIN, VSCLx" : "=j VOUT" (Result.XYZW) : "j VIN"  (V.XYZW), "j VSCL" (ScaleVec) );
    return Result;
#else
    return( vector4( V.GetX() * S, 
                     V.GetY() * S, 
                     V.GetZ() * S, 
                     V.GetW() * S ) );
#endif
}

//==============================================================================

inline
vector4 operator * ( const vector4& V1, const vector4& V2 )
{
    FORCE_ALIGNED_16( &V1 );
    FORCE_ALIGNED_16( &V2 );

#if USE_VU0
    vector4 Result;
    asm( "vmul.xyzw VOUT, VEC1, VEC2" : "=j VOUT" (Result.XYZW) : "j VEC1" (V1.XYZW), "j VEC2" (V2.XYZW) );
    return Result;
#else
    return( vector4( V1.GetX() * V2.GetX(),
                     V1.GetY() * V2.GetY(),
                     V1.GetZ() * V2.GetZ(),
                     V1.GetW() * V2.GetW() ) );
#endif
}

//==============================================================================

inline
f32 vector4::Dot( const vector4& V ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    u128 DotVec;
    f32  Dot;
    asm( "vmul.xyzw DOUT, VEC0, VEC1" : "=j DOUT" (DotVec) : "j VEC0" (XYZW), "j VEC1" (V.XYZW) );
    asm( "vaddw.x   DOUT, DIN,  DINw
          vaddz.x   DOUT, DOUT, DINz
          vaddy.x   DOUT, DOUT, DINy" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "qmfc2     DOT,  DIN"        : "=r DOT"  (Dot)    : "j DIN"  (DotVec) );
    return Dot;
#else
    return (V.GetX()*GetX() + V.GetY()*GetY() + V.GetZ()*GetZ() + V.GetW()*GetW());
#endif
}

//==============================================================================

inline
f32 vector4::Difference( const vector4& V ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    u128 DotVec;
    f32  Dot;
    asm( "vsub.xyzw DOUT, VEC0, VEC1" : "=j DOUT" (DotVec) : "j VEC0" (XYZW), "j VEC1" (V.XYZW) );
    asm( "vmul.xyzw DOUT, DIN,  DIN"  : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "vaddw.x   DOUT, DIN,  DINw
          vaddz.x   DOUT, DOUT, DINz
          vaddy.x   DOUT, DOUT, DINy" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "qmfc2     DOT,  DIN"        : "=r DOT"  (Dot)    : "j DIN"  (DotVec) );
    return Dot;
#else
    return (*this - V).LengthSquared();
#endif
}

//==============================================================================

inline
xbool vector4::InRange( f32 aMin, f32 aMax ) const
{
    FORCE_ALIGNED_16( this );

    return( (GetX()>=aMin) && (GetX()<=aMax) &&
            (GetY()>=aMin) && (GetY()<=aMax) &&
            (GetZ()>=aMin) && (GetZ()<=aMax) &&
            (GetW()>=aMin) && (GetW()<=aMax) );
}

//==============================================================================

inline
xbool vector4::IsValid( void ) const
{
    FORCE_ALIGNED_16( this );

    return( x_isvalid(GetX()) && x_isvalid(GetY()) && x_isvalid(GetZ()) && x_isvalid(GetW()) );
}

//==============================================================================

