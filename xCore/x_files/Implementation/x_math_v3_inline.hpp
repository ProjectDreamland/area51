//==============================================================================
//
//  x_math_v3_inline.hpp
//
//==============================================================================

#ifndef X_MATH_V3_INLINE_HPP
#define X_MATH_V3_INLINE_HPP
#else
#error "File " __FILE__ " has been included twice!"
#endif

#ifndef X_DEBUG_HPP
#include "../x_debug.hpp"
#endif

//==============================================================================

inline
f32 vector3::GetX( void ) const
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
f32& vector3::GetX( void )
{
#if USE_VU0
    return ((f32*)this)[0];
#else
    return X;
#endif
}

//==============================================================================

inline
f32 vector3::GetY( void ) const
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
f32& vector3::GetY( void )
{
#if USE_VU0
    return ((f32*)this)[1];
#else
    return Y;
#endif
}

//==============================================================================

inline
f32 vector3::GetZ( void ) const
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
f32& vector3::GetZ( void )
{
#if USE_VU0
    return ((f32*)this)[2];
#else
    return Z;
#endif
}

//==============================================================================

inline
s32 vector3::GetIW( void ) const
{
#if USE_VU0
    return ((s32*)this)[3];
#else
    return ((s32*)this)[3];
#endif
}

//==============================================================================

inline
s32& vector3::GetIW( void )
{
#if USE_VU0
    return ((s32*)this)[3];
#else
    return ((s32*)this)[3];
#endif
}

//==============================================================================

#ifdef TARGET_PS2
inline
u128 vector3::GetU128( void ) const
{
    return XYZW;
}

//==============================================================================

inline
void vector3::Set( u128 V )
{
    XYZW = V;
}
#endif

//==============================================================================

inline
vector3& vector3::Snap( f32 GridX, f32 GridY, f32 GridZ )
{
    FORCE_ALIGNED_16( this );

    GetX() = x_round( GetX(), GridX );
    GetY() = x_round( GetY(), GridY );
    GetZ() = x_round( GetZ(), GridZ );

    return *this;
}

//==============================================================================

inline 
vector3::vector3( void )
{
    FORCE_ALIGNED_16( this );
}

//==============================================================================

inline 
vector3::vector3( f32 aX, f32 aY, f32 aZ )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128    XY;
    u128    ZW;
    asm( "pextlw X_Y,  AY,  AX"  : "=r X_Y"  (XY)   : "r AX"  (aX), "r AY"  (aY) );
    asm( "pextlw Z_W,  $0,  AZ"  : "=r Z_W"  (ZW)   : "r AZ"  (aZ)               );
    asm( "pcpyld XYZW, Z_W, X_Y" : "=r XYZW" (XYZW) : "r X_Y" (XY), "r Z_W" (ZW) );
#else
    GetX() = aX;
    GetY() = aY;
    GetZ() = aZ;
#endif
}

//==============================================================================

#ifdef TARGET_PS2
inline
vector3::vector3( u128 V )
{
    XYZW = V;
}
#endif

//==============================================================================

inline 
void vector3::RotateX( radian Angle )
{
    FORCE_ALIGNED_16( this );

    radian S, C;
    x_sincos( Angle, S, C );

#if USE_VU0
    u128 Result;
    u128 CosVec;
    u128 SinVec;
    asm( "qmtc2     COS,    CVEC"           : "=j CVEC" (CosVec) : "r COS"  (C)      );
    asm( "qmtc2     SIN,    SVEC"           : "=j SVEC" (SinVec) : "r SIN"  (S)      );
    asm( "vaddx.xyz COUT,   vf00,   CINx"   : "=j COUT" (CosVec) : "j CIN"  (CosVec) );
    asm( "vaddx.xyz SOUT,   vf00,   SINx"   : "=j SOUT" (SinVec) : "j SIN"  (SinVec) );
    asm( "vmove.xw  RES,    XYZW
          vmulaz.z  acc,    CVEC,   XYZWz
          vmaddy.z  RES,    SVEC,   XYZWy
          vmulay.y  acc,    CVEC,   XYZWy
          vmsubz.y  RES,    SVEC,   XYZWz"  :
         "=&j RES"  (Result) :
         "j   XYZW" (XYZW), "j CVEC" (CosVec), "j SVEC" (SinVec) );

    XYZW = Result;
#else
    f32    y, z;
    y      = GetY();
    z      = GetZ();
    GetY() = (C * y) - (S * z);
    GetZ() = (C * z) + (S * y);
#endif
}

//==============================================================================

inline 
void vector3::RotateY( radian Angle )
{
    FORCE_ALIGNED_16( this );

    radian S, C;
    x_sincos( Angle, S, C );

#if USE_VU0
    u128 Result;
    u128 CosVec;
    u128 SinVec;
    asm( "qmtc2     COS,    CVEC"           : "=j CVEC" (CosVec) : "r COS" (C)      );
    asm( "qmtc2     SIN,    SVEC"           : "=j SVEC" (SinVec) : "r SIN" (S)      );
    asm( "vaddx.xyz COUT,   vf00,   CINx"   : "=j COUT" (CosVec) : "j CIN" (CosVec) );
    asm( "vaddx.xyz SOUT,   vf00,   SINx"   : "=j SOUT" (SinVec) : "j SIN" (SinVec) );
    asm( "vmove.yw  RES,    XYZW
          vmulaz.z  acc,    CVEC,   XYZWz
          vmsubx.z  RES,    SVEC,   XYZWx
          vmulax.x  acc,    CVEC,   XYZWx
          vmaddz.x  RES,    SVEC,   XYZWz"  :
         "=&j RES"  (Result) :
         "j   XYZW" (XYZW), "j CVEC" (CosVec), "j SVEC" (SinVec) );

    XYZW = Result;
#else
    f32    x, z;
    x   = GetX();
    z   = GetZ();
    GetX() = (C * x) + (S * z);
    GetZ() = (C * z) - (S * x);
#endif
}

//==============================================================================

inline 
void vector3::RotateZ( radian Angle )
{
    FORCE_ALIGNED_16( this );

    radian S, C;
    x_sincos( Angle, S, C );

#if USE_VU0
    u128 Result;
    u128 CosVec;
    u128 SinVec;
    asm( "qmtc2     COS,    CVEC"           : "=j CVEC" (CosVec) : "r COS" (C)      );
    asm( "qmtc2     SIN,    SVEC"           : "=j SVEC" (SinVec) : "r SIN" (S)      );
    asm( "vaddx.xyz COUT,   vf00,   CINx"   : "=j COUT" (CosVec) : "j CIN" (CosVec) );
    asm( "vaddx.xyz SOUT,   vf00,   SINx"   : "=j SOUT" (SinVec) : "j SIN" (SinVec) );
    asm( "vmove.zw  RES,    XYZW
          vmulax.x  acc,    CVEC,   XYZWx
          vmsuby.x  RES,    SVEC,   XYZWy
          vmulay.y  acc,    CVEC,   XYZWy
          vmaddx.y  RES,    SVEC,   XYZWx"  :
         "=&j RES"  (Result) :
         "j   XYZW" (XYZW), "j CVEC" (CosVec), "j SVEC" (SinVec) );

    XYZW = Result;
#else
    f32    x, y;
    x      = GetX();
    y      = GetY();
    GetX() = (C * x) - (S * y);
    GetY() = (C * y) + (S * x);
#endif
}

//==============================================================================

inline 
void vector3::Rotate( const radian3& R )
{
    FORCE_ALIGNED_16( this );

    f32 sx, cx;
    f32 sy, cy;
    f32 sz, cz;

    x_sincos( R.Pitch, sx, cx );
    x_sincos( R.Yaw,   sy, cy );
    x_sincos( R.Roll,  sz, cz );

    // fill out 3x3 rotations...this would give you the columns of a rotation
    // matrix that is roll, pitch, then yaw
    f32 sxsz = sx * sz;
    f32 sxcz = sx * cz;

    vector3 Mat0( cy*cz+sy*sxsz,   cx*sz,  cy*sxsz-sy*cz );
    vector3 Mat1( sy*sxcz-sz*cy,   cx*cz,  sy*sz+sxcz*cy );
    vector3 Mat2( cx*sy,           -sx,    cx*cy         );

#if USE_VU0
    u128 Result;
    asm( "vmove.w       RES,    XYZW
          vmulaz.xyz    acc,    COL2,   XYZWz
          vmadday.xyz   acc,    COL1,   XYZWy
          vmaddx.xyz    RES,    COL0,   XYZWx" :
         "=&j RES"  (Result) :
         "j   XYZW" (XYZW), "j COL0" (Mat0.XYZW), "j COL1" (Mat1.XYZW), "j COL2" (Mat2.XYZW) );

    XYZW = Result;
#else
    f32 x = GetX();
    f32 y = GetY();
    f32 z = GetZ();

    GetX() = Mat0.GetX()*x + Mat1.GetX()*y + Mat2.GetX()*z;
    GetY() = Mat0.GetY()*x + Mat1.GetY()*y + Mat2.GetY()*z;
    GetZ() = Mat0.GetZ()*x + Mat1.GetZ()*y + Mat2.GetZ()*z;
#endif
}


//==============================================================================

inline 
void vector3::Set( f32 aX, f32 aY, f32 aZ )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128    XY;
    u128    ZW;
    asm( "pextlw X_Y,  AY,  AX"  : "=r X_Y"  (XY)   : "r AX"  (aX), "r AY"  (aY) );
    asm( "pextlw Z_W,  $0,  AZ"  : "=r Z_W"  (ZW)   : "r AZ"  (aZ)               );
    asm( "pcpyld XYZW, Z_W, X_Y" : "=r XYZW" (XYZW) : "r X_Y" (XY), "r Z_W" (ZW) );
#else
    GetX() = aX; 
    GetY() = aY; 
    GetZ() = aZ;
#endif
}

//==============================================================================

inline
void vector3::Set( radian Pitch, radian Yaw )
{
    FORCE_ALIGNED_16( this );

    // If you take the vector (0,0,1) and pass it through a rotation matrix
    // created from pitch and yaw, this would be the resulting vector.
    // By knowing that the x and y components of the vector are 0, we have
    // removed lots of extra operations.
    f32 PS, PC;
    f32 YS, YC;

    x_sincos( Pitch, PS, PC );
    x_sincos( Yaw,   YS, YC );

    Set( (YS * PC), -(PS), (YC * PC) );
}

//==============================================================================

inline
void vector3::Set( const radian3& R )
{
    FORCE_ALIGNED_16( this );

    // Rotations are applied in the order roll, pitch, and yaw, and setting
    // this vector with a radian3 has the effect of rotating the vector (0,0,1)
    // by R. Which means the roll is useless.
    Set( R.Pitch, R.Yaw );
}

//==============================================================================

inline
vector3::vector3( radian Pitch, radian Yaw )
{
    FORCE_ALIGNED_16( this );

    Set( Pitch, Yaw );
}

//==============================================================================

inline
vector3::vector3( const radian3& R )
{
    FORCE_ALIGNED_16( this );

    Set( R );
}

//==============================================================================

inline 
void vector3::operator () ( f32 aX, f32 aY, f32 aZ )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128    XY;
    u128    ZW;
    asm( "pextlw X_Y,  AY,  AX"  : "=r X_Y"  (XY)   : "r AX"  (aX), "r AY"  (aY) );
    asm( "pextlw Z_W,  $0,  AZ"  : "=r Z_W"  (ZW)   : "r AZ"  (aZ)               );
    asm( "pcpyld XYZW, Z_W, X_Y" : "=r XYZW" (XYZW) : "r X_Y" (XY), "r Z_W" (ZW) );
#else
    GetX() = aX; 
    GetY() = aY; 
    GetZ() = aZ;
#endif
}

//==============================================================================

inline
f32 vector3::operator [] ( s32 Index ) const
{
    FORCE_ALIGNED_16( this );

    ASSERT( (Index >= 0) && (Index < 3) );
    return( ((f32*)this)[Index] );
}

//==============================================================================

inline
f32& vector3::operator [] ( s32 Index )
{
    FORCE_ALIGNED_16( this );

    ASSERT( (Index >= 0) && (Index < 3) );
    return( ((f32*)this)[Index] );
}

//==============================================================================

inline 
void vector3::Zero( void )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    asm( "vsub.xyzw     XYZW, vf00, vf00" : "=j XYZW" (XYZW) );
#else
    GetX() = GetY() = GetZ() = 0.0f;
#endif
}

//==============================================================================

inline 
void vector3::Negate( void )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    asm( "vsub.xyzw     VOUT, vf00, VIN" : "=j VOUT" (XYZW) : "j VIN" (XYZW) );
#else
    GetX() = -GetX(); 
    GetY() = -GetY(); 
    GetZ() = -GetZ();
#endif
}

//==============================================================================

inline
void vector3::Min( f32 Min )
{
#if USE_VU0
    u128 VMin;
    asm( "qmtc2         FMIN, VMIN"       : "=j VMIN" (VMin) : "r FMIN" (Min) );
    asm( "vminix.xyzw   VOUT, VIN, VMINx" : "=j VOUT" (XYZW) : "j VIN"  (XYZW), "j VMIN" (VMin) );
#else
    GetX() = MIN( GetX(), Min );
    GetY() = MIN( GetY(), Min );
    GetZ() = MIN( GetZ(), Min );
#endif
}

//==============================================================================

inline
void vector3::Max( f32 Max )
{
#if USE_VU0
    u128 VMax;
    asm( "qmtc2         FMAX, VMAX"       : "=j VMAX" (VMax) : "r FMAX" (Max) );
    asm( "vmaxx.xyzw    VOUT, VIN, VMAXx" : "=j VOUT" (XYZW) : "j VIN"  (XYZW), "j VMAX" (VMax) );
#else
    GetX() = MAX( GetX(), Max );
    GetY() = MAX( GetY(), Max );
    GetZ() = MAX( GetZ(), Max );
#endif
}

//==============================================================================

inline
void vector3::Min( const vector3& V )
{
#if USE_VU0
    asm( "vmini.xyzw    VOUT, VIN, VMIN" : "=j VOUT" (XYZW) : "j VIN" (XYZW), "j VMIN" (V.XYZW) );
#else
    GetX() = MIN( GetX(), V.GetX() );
    GetY() = MIN( GetY(), V.GetY() );
    GetZ() = MIN( GetZ(), V.GetZ() );
#endif
}

//==============================================================================

inline
void vector3::Max( const vector3& V )
{
#if USE_VU0
    asm( "vmax.xyzw     VOUT, VIN, VMAX" : "=j VOUT" (XYZW) : "j VIN" (XYZW), "j VMAX" (V.XYZW) );
#else
    GetX() = MAX( GetX(), V.GetX() );
    GetY() = MAX( GetY(), V.GetY() );
    GetZ() = MAX( GetZ(), V.GetZ() );
#endif
}

//==============================================================================

inline
void vector3::Normalize( void )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128 DotProduct;
    asm( "vmul.xyz      DOT,  XYZW,  XYZW" : "=j DOT"  (DotProduct) : "j XYZW" (XYZW)       );
    asm( "vaddz.x       DOUT, DIN,   DINz
          vaddy.x       DOUT, DOUT,  DINy" : "=j DOUT" (DotProduct) : "j DIN"  (DotProduct) );
    asm( "vrsqrt        Q,    vf00w, DOTx
          vwaitq
          vmulq.xyzw    VOUT, VIN,   Q"    :
         "=j VOUT" (XYZW) :
         "j  VIN"  (XYZW),
         "j  DOT"  (DotProduct) );
#else

    f32 LengthSquared = GetX()*GetX() + GetY()*GetY() + GetZ()*GetZ();
    if( LengthSquared > 0.0000001f )
    {
        f32 N = x_1sqrt( LengthSquared );
        ASSERT( x_isvalid(N) );

        GetX() *= N;
        GetY() *= N;
        GetZ() *= N;
    }
#endif
}

//==============================================================================

inline
void vector3::NormalizeAndScale( f32 Scalar )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128 DotProduct;
    u128 ScaleVec;
    asm( "qmtc2         FSCL, VSCL"        : "=j VSCL" (ScaleVec)   : "r FSCL" (Scalar)     );
    asm( "vmul.xyz      DOT,  XYZW,  XYZW" : "=j DOT"  (DotProduct) : "j XYZW" (XYZW)       );
    asm( "vaddz.x       DOUT, DIN,   DINz
          vaddy.x       DOUT, DOUT,  DINy" : "=j DOUT" (DotProduct) : "j DIN"  (DotProduct) );
    asm( "vrsqrt        Q,    VSCLx, DOTx
          vwaitq
          vmulq.xyzw    VOUT, VIN,   Q"    :
         "=j VOUT" (XYZW) :
         "j  VIN"  (XYZW),
         "j  VSCL" (ScaleVec),
         "j  DOT"  (DotProduct) );
#else
    f32 F = GetX()*GetX() + GetY()*GetY() + GetZ()*GetZ();
    if( F ) // F was often found to be zero
    {
        f32 N = x_1sqrt( F );
        ASSERT( x_isvalid( N ));
        N      *= Scalar;
        GetX() *= N;
        GetY() *= N;
        GetZ() *= N;
    }
#endif
}

//==============================================================================

inline 
xbool vector3::SafeNormalize( void )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128    DotVec;
    f32     Dot;

    asm( "vmul.xyz  DOUT,   XYZW,   XYZW" : "=j DOUT" (DotVec) : "j XYZW" (XYZW) );
    asm( "vaddz.x   DOUT,   DIN,    DINz
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
    f32 N = x_1sqrt(GetX()*GetX() + GetY()*GetY() + GetZ()*GetZ());

    if( x_isvalid(N) )
    {
        GetX() *= N;
        GetY() *= N;
        GetZ() *= N;
        return TRUE;
    }
    else
    {
        GetX() = 0.0f;
        GetY() = 0.0f;
        GetZ() = 0.0f;
        return FALSE;
    }
#endif
}

//==============================================================================

inline 
xbool vector3::SafeNormalizeAndScale( f32 Scalar )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128    DotVec;
    f32     Dot;

    asm( "vmul.xyz  DOUT,   XYZW,   XYZW" : "=j DOUT" (DotVec) : "j XYZW" (XYZW) );
    asm( "vaddz.x   DOUT,   DIN,    DINz
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
    f32 N = x_1sqrt(GetX()*GetX() + GetY()*GetY() + GetZ()*GetZ());

    if( x_isvalid(N) )
    {
        N   *= Scalar;
        GetX() *= N;
        GetY() *= N;
        GetZ() *= N;
        return TRUE;
    }
    else
    {
        GetX() = 0.0f;
        GetY() = 0.0f;
        GetZ() = 0.0f;
        return FALSE;
    }
#endif
}

//==============================================================================

inline 
void vector3::Scale( f32 Scalar )
{
    FORCE_ALIGNED_16( this );

    ASSERT( x_isvalid( Scalar ) );

#if USE_VU0
    u128 ScaleVec;
    asm( "qmtc2      FSCL, VSCL"       : "=j VSCL" (ScaleVec) : "r FSCL" (Scalar) );
    asm( "vmulx.xyzw VOUT, VIN, VSCLx" : "=j VOUT" (XYZW)     : "j VIN"  (XYZW), "j VSCL" (ScaleVec) );
#else
    GetX() *= Scalar; 
    GetY() *= Scalar; 
    GetZ() *= Scalar;
#endif
}

//==============================================================================

inline 
f32 vector3::Length( void ) const
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128 DotVec;
    f32  Dot;
    asm( "vmul.xyz  DOUT, XYZW, XYZW" : "=j DOUT" (DotVec) : "j XYZW" (XYZW) );
    asm( "vaddz.x   DOUT, DIN,  DINz
          vaddy.x   DOUT, DOUT, DINy" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "qmfc2     DOT,  DIN"        : "=r DOT"  (Dot)    : "j DIN"  (DotVec) );
    return x_sqrt(Dot);
#else
    return( x_sqrt( GetX()*GetX() + GetY()*GetY() + GetZ()*GetZ() ) );
#endif
}

//==============================================================================

inline
f32 vector3::LengthSquared( void ) const
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128 DotVec;
    f32  Dot;
    asm( "vmul.xyz  DOUT, XYZW, XYZW" : "=j DOUT" (DotVec) : "j XYZW" (XYZW) );
    asm( "vaddz.x   DOUT, DIN,  DINz
          vaddy.x   DOUT, DOUT, DINy" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "qmfc2     DOT,  DIN"        : "=r DOT"  (Dot)    : "j DIN"  (DotVec) );
    return Dot;
#else
    return( GetX()*GetX() + GetY()*GetY() + GetZ()*GetZ() );
#endif
}

//==============================================================================

inline 
f32 vector3::Dot( const vector3& V ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    u128 DotVec;
    f32  Dot;
    asm( "vmul.xyz  DOUT, VEC0, VEC1" : "=j DOUT" (DotVec) : "j VEC0" (XYZW), "j VEC1" (V.XYZW) );
    asm( "vaddz.x   DOUT, DIN,  DINz
          vaddy.x   DOUT, DOUT, DINy" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "qmfc2     DOT,  DIN"        : "=r DOT"  (Dot)    : "j DIN"  (DotVec) );
    return Dot;
#else
    return( (GetX() * V.GetX()) + (GetY() * V.GetY()) + (GetZ() * V.GetZ()) );
#endif
}

//==============================================================================

inline 
vector3 vector3::Cross( const vector3& V ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    vector3 Result;
    asm( "vopmula.xyz   acc,    VEC0,   VEC1
          vopmsub.xyz   RES,    VEC1,   VEC0" :
         "=j RES"  (Result.XYZW) :
         "j  VEC0" (XYZW),
         "j  VEC1" (V.XYZW) );
    return Result;
#else
    return( vector3( (GetY() * V.GetZ()) - (GetZ() * V.GetY()),
                     (GetZ() * V.GetX()) - (GetX() * V.GetZ()),
                     (GetX() * V.GetY()) - (GetY() * V.GetX()) ) );
#endif
}

//==============================================================================

inline
vector3 vector3::Reflect( const vector3& Normal ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Normal );

#if USE_VU0
    f32     fTwo = 2.0f;
    vector3 Result;
    u128    DotVec;
    u128    TwoVec;

    asm( "vmul.xyz  DOUT, VEC0, VEC1"  : "=j DOUT" (DotVec)      : "j VEC0" (Normal.XYZW), "j VEC1" (XYZW)        );
    asm( "vaddz.x   DOUT, DIN,  DINz
          vaddy.x   DOUT, DOUT, DINy"  : "=j DOUT" (DotVec)      : "j DIN"  (DotVec)                              );
    asm( "qmtc2     FTWO, VTWO"        : "=j VTWO" (TwoVec)      : "r FTWO" (fTwo)                                );
    asm( "vmulx.x   DOUT, DIN,  VTWOx" : "=j DOUT" (DotVec)      : "j DIN"  (DotVec),      "j VTWO" (TwoVec)      );
    asm( "vmulx.xyz RES,  NORM, DOUTx" : "=j RES"  (Result.XYZW) : "j NORM" (Normal.XYZW), "j DOUT" (DotVec)      );
    asm( "vsub.xyz  RES,  VEC0, VEC1"  : "=j RES"  (Result.XYZW) : "j VEC0" (XYZW),        "j VEC1" (Result.XYZW) );
    return Result;
#else
    return *this - 2*(Normal.Dot(*this)) * Normal;
#endif
}

//==============================================================================

inline 
f32 v3_Dot( const vector3& V1, const vector3& V2 )
{
    FORCE_ALIGNED_16( &V1 );
    FORCE_ALIGNED_16( &V2 );

#if USE_VU0
    u128 DotVec;
    f32  Dot;
    asm( "vmul.xyz  DOUT, VEC0, VEC1" : "=j DOUT" (DotVec) : "j VEC0" (V1.XYZW), "j VEC1" (V2.XYZW) );
    asm( "vaddz.x   DOUT, DIN,  DINz
          vaddy.x   DOUT, DOUT, DINy" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "qmfc2     DOT,  DIN"        : "=r DOT"  (Dot)    : "j DIN"  (DotVec) );
    return Dot;
#else
    return( (V1.GetX() * V2.GetX()) + (V1.GetY() * V2.GetY()) + (V1.GetZ() * V2.GetZ()) );
#endif
}

//==============================================================================

inline 
vector3 v3_Cross( const vector3& V1, const vector3& V2 )
{
    FORCE_ALIGNED_16( &V1 );
    FORCE_ALIGNED_16( &V2 );

#if USE_VU0
    vector3 Result;
    asm( "vopmula.xyz   acc,    VEC0,   VEC1
          vopmsub.xyz   RES,    VEC1,   VEC0" :
         "=j RES"  (Result.XYZW) :
         "j  VEC0" (V1.XYZW),
         "j  VEC1" (V2.XYZW) );
    return Result;
#else
    return( vector3( (V1.GetY() * V2.GetZ()) - (V1.GetZ() * V2.GetY()),
                     (V1.GetZ() * V2.GetX()) - (V1.GetX() * V2.GetZ()),
                     (V1.GetX() * V2.GetY()) - (V1.GetY() * V2.GetX()) ) );
#endif
}

//==============================================================================

inline 
radian vector3::GetPitch( void ) const
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    f32  L;
    u128 LVec;
    asm( "vmul.xz   LVEC, XYZW, XYZW" : "=j LVEC" (LVec) : "j XYZW" (XYZW) );
    asm( "vaddz.x   LOUT, LIN,  LINz" : "=j LOUT" (LVec) : "j LIN"  (LVec) );
    asm( "qmfc2     FL,   LVEC"       : "=r FL"   (L)    : "j LVEC" (LVec) );
    L = x_sqrt(L);
#else
    f32 L = (f32)x_sqrt( GetX()*GetX() + GetZ()*GetZ() );
#endif
    return( -x_atan2( GetY(), L ) );
}

//==============================================================================

inline 
radian vector3::GetYaw( void ) const
{
    FORCE_ALIGNED_16( this );

    return( x_atan2( GetX(), GetZ() ) );
}

//==============================================================================

inline 
void vector3::GetPitchYaw( radian& Pitch, radian& Yaw ) const
{
    FORCE_ALIGNED_16( this );

    Pitch = GetPitch();
    Yaw   = GetYaw();
}

//==============================================================================
//           * Start
//           |
//           <--------------(* this )
//           | Return Vector
//           |
//           |
//           * End
//
// Such: 
//
// this.GetClosestVToLSeg(a,b).LengthSquared(); // Is the length square to the line segment
// this.GetClosestVToLSeg(a,b) + this;          // Is the closest point in to the line segment
// this.GetClosestPToLSegRatio(a,b)             // Is the ratio (0=a to 1=b) of the closest
//                                                 point in line segment.
//
//==============================================================================
inline 
vector3 vector3::GetClosestVToLSeg( const vector3& Start, const vector3& End ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Start );
    FORCE_ALIGNED_16( &End );

    vector3 Diff = *this - Start;
    vector3 Dir  = End   - Start;
    f32     T    = Diff.Dot( Dir );

    if( T > 0.0f )
    {
        f32 SqrLen = Dir.Dot( Dir );

        if ( T >= SqrLen )
        {
            Diff -= Dir;
        }
        else
        {
            T    /= SqrLen;
            Diff -= T * Dir;
        }
    }

    return -Diff;
}

//==============================================================================

inline
f32 vector3::GetSqrtDistToLineSeg( const vector3& Start, const vector3& End ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Start );
    FORCE_ALIGNED_16( &End );

    return GetClosestVToLSeg(Start,End).LengthSquared();
}

//==============================================================================

inline
vector3 vector3::GetClosestPToLSeg( const vector3& Start, const vector3& End ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Start );
    FORCE_ALIGNED_16( &End );

    return GetClosestVToLSeg(Start,End) + *this;
}

//==============================================================================

// Given the line defined by the points A and B, returns the ratio (0->1) of the 
// closest point on the line to P. 
// 0 = A
// 1 = B
// >0 and <1 means somewhere inbetween A and B
inline
f32 vector3::GetClosestPToLSegRatio( const vector3& A, const vector3& B ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &A );
    FORCE_ALIGNED_16( &B );

    // Get direction vector and length squared of line
    vector3 AB = B - A ;
    f32     L  = AB.LengthSquared() ;
    
    // Is line a point?
    if (L == 0.0f)
        return 0.0f ;

    // Get vector from start of line to point
    vector3 AP = (*this) - A ;

    // Calculate: Cos(theta)*|AP|*|AB|
    f32 Dot = AP.Dot(AB) ;

    // Before start?
    if (Dot <= 0)
        return 0.0f ;

    // After end?
    if (Dot >= L)
        return 1.0f ;

    // Let distance along AB = X = (|AP| dot |AB|) / |AB|
    // Ratio T is X / |AB| = (|AP| dot |AB|) / (|AB| * |AB|)
    f32 T = Dot / L ;
    ASSERT(T >= 0.0f) ;
    ASSERT(T <= 1.0f) ;

    // Slerp between A and B
    return T ;
}

//==============================================================================

inline
vector3::operator const f32* ( void ) const
{
    return( (f32*)this );
}

//==============================================================================

inline 
vector3 vector3::operator - ( void ) const
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    vector3 Result;
    asm( "vsub.xyz  RES, vf00, XYZW" : "=j RES" (Result.XYZW) : "j XYZW" (XYZW) );
    return Result;
#else
    return( vector3( -GetX(), -GetY(), -GetZ() ) );
#endif
}

//==============================================================================

inline 
const vector3& vector3::operator = ( const vector4& V )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    XYZW = V.XYZW;
    return( *this );
#else
    GetX() = V.GetX();
    GetY() = V.GetY();
    GetZ() = V.GetZ();
    return( *this );
#endif
}

//==============================================================================

inline 
vector3& vector3::operator += ( const vector3& V )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    asm( "vadd.xyzw     VOUT, VEC0, VEC1" : "=j VOUT" (XYZW) : "j VEC0" (XYZW), "j VEC1" (V.XYZW) );
#else
    GetX() += V.GetX();
    GetY() += V.GetY();
    GetZ() += V.GetZ();
#endif
    return( *this );
}

//==============================================================================

inline 
vector3& vector3::operator -= ( const vector3& V )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    asm( "vsub.xyzw     VOUT, VEC0, VEC1" : "=j VOUT" (XYZW) : "j VEC0" (XYZW), "j VEC1" (V.XYZW) );
#else
    GetX() -= V.GetX();
    GetY() -= V.GetY();
    GetZ() -= V.GetZ();
#endif
    return( *this );
}

//==============================================================================

inline 
vector3& vector3::operator *= ( const f32 Scalar )
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
#endif
    return( *this );
}

//==============================================================================

inline 
vector3& vector3::operator /= ( const f32 Scalar )
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
#endif
    return( *this );
}

//==============================================================================

inline 
xbool vector3::operator == ( const vector3& V ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

    return( (GetX() == V.GetX()) && (GetY() == V.GetY()) && (GetZ() == V.GetZ()) );
}

//==============================================================================

inline 
xbool vector3::operator != ( const vector3& V ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

    return( (GetX() != V.GetX()) || (GetY() != V.GetY()) || (GetZ() != V.GetZ()) );
}

//==============================================================================

inline 
vector3 operator + ( const vector3& V1, const vector3& V2 )
{
    FORCE_ALIGNED_16( &V1 );
    FORCE_ALIGNED_16( &V2 );

#if USE_VU0
    vector3 Result;
    asm( "vadd.xyzw   XYZW, V1, V2" : "=j XYZW" (Result.XYZW) : "j V1" (V1.XYZW), "j V2" (V2.XYZW) );
    return Result;
#else
    return( vector3( V1.GetX() + V2.GetX(), 
                     V1.GetY() + V2.GetY(), 
                     V1.GetZ() + V2.GetZ() ) );
#endif
}

//==============================================================================
inline
vector3 operator * ( const vector3& V1, const vector3& V2 )
{
    FORCE_ALIGNED_16( &V1 );
    FORCE_ALIGNED_16( &V2 );

#if USE_VU0
    vector3 Result;
    asm( "vmul.xyzw     VOUT, VEC1, VEC2" : "=j VOUT" (Result.XYZW) : "j VEC1" (V1.XYZW), "j VEC2" (V2.XYZW) );
    return Result;
#else
    return( vector3( V1.GetX() * V2.GetX(),
                     V1.GetY() * V2.GetY(),
                     V1.GetZ() * V2.GetZ() ) );
#endif
}

//==============================================================================

inline 
vector3 operator - ( const vector3& V1, const vector3& V2 )
{
    FORCE_ALIGNED_16( &V1 );
    FORCE_ALIGNED_16( &V2 );

#if USE_VU0
    vector3 Result;
    asm( "vsub.xyzw XYZW, V1, V2" : "=j XYZW" (Result.XYZW) : "j V1" (V1.XYZW), "j V2" (V2.XYZW) );
    return Result;
#else
    return( vector3( V1.GetX() - V2.GetX(),
                     V1.GetY() - V2.GetY(),
                     V1.GetZ() - V2.GetZ() ) );
#endif
}

//==============================================================================

inline 
vector3 operator / ( const vector3& V, f32 S )
{
    FORCE_ALIGNED_16( &V );

    ASSERT( S != 0.0f );
    S = 1.0f / S;

#if USE_VU0
    u128    ScaleVec;
    vector3 Result;
    asm( "qmtc2         FSCL, VSCL"       : "=j VSCL" (ScaleVec)    : "r FSCL" (S) );
    asm( "vmulx.xyzw    VOUT, VIN, VSCLx" : "=j VOUT" (Result.XYZW) : "j VIN"  (V.XYZW), "j VSCL" (ScaleVec) );
    return Result;
#else
    return( vector3( V.GetX() * S, 
                     V.GetY() * S, 
                     V.GetZ() * S ) );
#endif
}

//==============================================================================

inline 
vector3 operator * ( const vector3& V, f32 S )
{
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    u128    ScaleVec;
    vector3 Result;
    asm( "qmtc2         FSCL, VSCL"       : "=j VSCL" (ScaleVec)    : "r FSCL" (S) );
    asm( "vmulx.xyzw    VOUT, VIN, VSCLx" : "=j VOUT" (Result.XYZW) : "j VIN"  (V.XYZW), "j VSCL" (ScaleVec) );
    return Result;
#else
    return( vector3( V.GetX() * S, 
                     V.GetY() * S, 
                     V.GetZ() * S ) );
#endif
}

//==============================================================================

inline 
vector3 operator * ( f32 S, const vector3& V )
{
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    u128    ScaleVec;
    vector3 Result;
    asm( "qmtc2         FSCL, VSCL"       : "=j VSCL" (ScaleVec)    : "r FSCL" (S) );
    asm( "vmulx.xyzw    VOUT, VIN, VSCLx" : "=j VOUT" (Result.XYZW) : "j VIN"  (V.XYZW), "j VSCL" (ScaleVec) );
    return Result;
#else
    return( vector3( V.GetX() * S, 
                     V.GetY() * S, 
                     V.GetZ() * S ) );
#endif
}

//==============================================================================

inline
radian v3_AngleBetween ( const vector3& V1, const vector3& V2 )
{
    FORCE_ALIGNED_16( &V1 );
    FORCE_ALIGNED_16( &V2 );

    f32 D, Cos;
    
    D = V1.Length() * V2.Length();
    
    if( D == 0.0f )  return( R_0 );
    
    Cos = v3_Dot( V1, V2 ) / D;
    
    if     ( Cos >  1.0f )  Cos =  1.0f;
    else if( Cos < -1.0f )  Cos = -1.0f;
    
    return( x_acos( Cos ) );
}

//==============================================================================

inline
void  vector3::GetRotationTowards( const vector3& DestV,
                                         vector3& RotAxis, 
                                         radian&  RotAngle ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &DestV );
    FORCE_ALIGNED_16( &RotAxis );

    // Get lengths of vectors
    f32 D = this->Length() * DestV.Length();
    
    // vectors are zero length
    if( D == 0.0f )  
    {
        RotAxis = vector3(1,0,0);
        RotAngle = 0;
        return;
    }
    
    // Get unit dot product of the vectors
    f32 Dot = this->Dot(DestV) / D;
    
    if     ( Dot >  1.0f )  Dot =  1.0f;
    else if( Dot < -1.0f )  Dot = -1.0f;
    
    // Get axis to rotate about
    RotAxis = v3_Cross( *this, DestV );

    if( RotAxis.SafeNormalize() )
    {
        RotAngle = x_acos(Dot);
    }
    else
    {
        RotAxis = vector3(1,0,0);

        if( Dot > 0 )
            RotAngle = 0;       // Facing same direction
        else
            RotAngle = R_180;   // Facing opposite directions
    }
}

//==============================================================================

inline
f32 vector3::Difference( const vector3& V ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    u128 DotVec;
    f32  Dot;
    asm( "vsub.xyz  DOUT, VEC0, VEC1" : "=j DOUT" (DotVec) : "j VEC0" (XYZW), "j VEC1" (V.XYZW) );
    asm( "vmul.xyz  DOUT, DIN,  DIN"  : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "vaddz.x   DOUT, DIN,  DINz
          vaddy.x   DOUT, DOUT, DINy" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "qmfc2     DOT,  DIN"        : "=r DOT"  (Dot)    : "j DIN"  (DotVec) );
    return Dot;
#else
    return (*this - V).LengthSquared();
#endif
}

//==============================================================================

inline
xbool vector3::InRange( f32 aMin, f32 aMax ) const
{
    FORCE_ALIGNED_16( this );

    return( (GetX()>=aMin) && (GetX()<=aMax) &&
            (GetY()>=aMin) && (GetY()<=aMax) &&
            (GetZ()>=aMin) && (GetZ()<=aMax) );
}

//==============================================================================

inline
xbool vector3::IsValid( void ) const
{
    FORCE_ALIGNED_16( this );

    return( x_isvalid(GetX()) && x_isvalid(GetY()) && x_isvalid(GetZ()) );
}

//==============================================================================

inline
vector3p::vector3p( void )
{
}

//==============================================================================

inline
vector3p::vector3p( f32 aX, f32 aY, f32 aZ )
{
    Set( aX, aY, aZ );
}

//==============================================================================

inline
vector3p::vector3p( const vector3& V )
{
    Set( V.GetX(), V.GetY(), V.GetZ() );
}

//==============================================================================

inline
void vector3p::Set( f32 aX, f32 aY, f32 aZ )
{
    X = aX;
    Y = aY;
    Z = aZ;
}

//==============================================================================

inline
const vector3p& vector3p::operator = ( const vector3& V )
{
    Set( V.GetX(), V.GetY(), V.GetZ() );
    return( *this );
}

//==============================================================================

inline
vector3p::operator const vector3( void ) const
{
    return vector3( X, Y, Z );
}

//==============================================================================
inline
f32 vector3::ClosestPointToRectangle( 
    const vector3& P0,                      // Origin from the edges. 
    const vector3& E0, 
    const vector3& E1, 
    vector3&       OutClosestPoint ) const
{
    vector3 kDiff    = P0 - *this;
    f32     fA00     = E0.LengthSquared();
    f32     fA11     = E1.LengthSquared();
    f32     fB0      = kDiff.Dot( E0 );
    f32     fB1      = kDiff.Dot( E1 );
    f32     fS       = -fB0;
    f32     fT       = -fB1;
    f32     fSqrDist = kDiff.LengthSquared();

    if( fS < 0.0f )
    {
        fS = 0.0f;
    }
    else if( fS <= fA00 )
    {
        fS /= fA00;
        fSqrDist += fB0*fS;
    }
    else
    {
        fS = 1.0f;
        fSqrDist += fA00 + 2.0f*fB0;
    }

    if( fT < 0.0f )
    {
        fT = 0.0f;
    }
    else if( fT <= fA11 )
    {
        fT /= fA11;
        fSqrDist += fB1*fT;
    }
    else
    {
        fT = 1.0f;
        fSqrDist += fA11 + 2.0f*fB1;
    }

    // Set the closest point
    OutClosestPoint = P0 + (E0 * fS) + (E1 * fT);

    return x_abs(fSqrDist);
}

