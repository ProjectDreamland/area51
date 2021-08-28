//==============================================================================
// vector3 inlines
//==============================================================================

inline
f32 vector3t::GetX( void ) const
{
    f32 Result;
    asm( "qmfc2 FOUT, VIN" : "=r FOUT" (Result) : "j VIN" (XYZW) );
    return Result;
}

//==============================================================================

inline
f32& vector3t::GetX( void )
{
    return X;
}

//==============================================================================

inline
f32 vector3t::GetY( void ) const
{
    f32  Result;
    u128 Temp;
    asm( "vaddy.x TEMP, vf00, VIN" : "=j TEMP" (Temp)   : "j VIN"  (XYZW) );
    asm( "qmfc2  FOUT, TEMP"       : "=r FOUT" (Result) : "j TEMP" (Temp) );
    return Result;
}

//==============================================================================

inline
f32& vector3t::GetY( void )
{
    return Y;
}

//==============================================================================

inline
f32 vector3t::GetZ( void ) const
{
    f32  Result;
    u128 Temp;
    asm( "vaddz.x TEMP, vf00, VIN" : "=j TEMP" (Temp)   : "j VIN"  (XYZW) );
    asm( "qmfc2  FOUT, TEMP"       : "=r FOUT" (Result) : "j TEMP" (Temp) );
    return Result;
}

//==============================================================================

inline
f32& vector3t::GetZ( void )
{
    return Z;
}

//==============================================================================

inline 
void vector3t::Set( f32 aX, f32 aY, f32 aZ )
{
    FORCE_ALIGNED_16( this );

    u128    XY;
    u128    ZW;

    asm( "pextlw X_Y,  AY,  AX"  : "=r X_Y"  (XY)   : "r AX"  (aX), "r AY"  (aY) );
    asm( "pextlw Z_W,  $0,  AZ"  : "=r Z_W"  (ZW)   : "r AZ"  (aZ)               );
    asm( "pcpyld XYZW, Z_W, X_Y" : "=r XYZW" (XYZW) : "r X_Y" (XY), "r Z_W" (ZW) );
}

//==============================================================================

inline
void vector3t::Set( radian Pitch, radian Yaw )
{
    FORCE_ALIGNED_16( this );

    f32 PS, PC;
    f32 YS, YC;

    x_sincos( Pitch, PS, PC );
    x_sincos( Yaw,   YS, YC );

    Set( YS*PC, -PS, YC*PC );
}

//==============================================================================

inline
void vector3t::Set( const radian3& R )
{
    FORCE_ALIGNED_16( this );

    f32 sx, cx;
    f32 sy, cy;
    x_sincos( R.Pitch, sx, cx );
    x_sincos( R.Yaw,   sy, cy );

    Set( sy*cx, -sx, cx*cy );
}

//==============================================================================

inline
void vector3t::Zero( void )
{
    FORCE_ALIGNED_16( this );

    asm( "vsub.xyzw     XYZW, vf00, vf00" : "=j XYZW" (XYZW) );
}

//==============================================================================

inline
void vector3t::Negate( void )
{
    FORCE_ALIGNED_16( this );

    asm( "vsub.xyzw     VOUT, vf00, VIN" : "=j VOUT" (XYZW) : "j VIN" (XYZW) );
}

//==============================================================================

inline
void vector3t::Normalize( void )
{
    FORCE_ALIGNED_16( this );

    u128 DotProduct;
    asm( "vmul.xyz      DOT,  XYZW,  XYZW" : "=j DOT"  (DotProduct) : "j XYZW" (XYZW)       );
    asm( "vaddz.x       DOUT, DIN,   DINz" : "=j DOUT" (DotProduct) : "j DIN"  (DotProduct) );
    asm( "vaddy.x       DOUT, DIN,   DINy" : "=j DOUT" (DotProduct) : "j DIN"  (DotProduct) );
    asm( "vrsqrt        Q,    vf00w, DOTx
          vwaitq
          vmulq.xyzw    VOUT, VIN,   Q"    :
         "=j VOUT" (XYZW) :
         "j  VIN"  (XYZW),
         "j  DOT"  (DotProduct) );
}

//==============================================================================

inline
void vector3t::NormalizeAndScale( f32 Scalar )
{
    FORCE_ALIGNED_16( this );

    u128 DotProduct;
    u128 ScaleVec;
    asm( "qmtc2         FSCL, VSCL"        : "=j VSCL" (ScaleVec)   : "r FSCL" (Scalar)     );
    asm( "vmul.xyz      DOT,  XYZW,  XYZW" : "=j DOT"  (DotProduct) : "j XYZW" (XYZW)       );
    asm( "vaddz.x       DOUT, DIN,   DINz" : "=j DOUT" (DotProduct) : "j DIN"  (DotProduct) );
    asm( "vaddy.x       DOUT, DIN,   DINy" : "=j DOUT" (DotProduct) : "j DIN"  (DotProduct) );
    asm( "vrsqrt        Q,    VSCLx, DOTx
          vwaitq
          vmulq.xyzw    VOUT, VIN,   Q"    :
         "=j VOUT" (XYZW) :
         "j  VIN"  (XYZW),
         "j  VSCL" (ScaleVec),
         "j  DOT"  (DotProduct) );
}

//==============================================================================

inline 
xbool vector3t::SafeNormalize( void )
{
    FORCE_ALIGNED_16( this );

    u128    DotVec;
    f32     Dot;

    asm( "vmul.xyz  DOUT,   XYZW,   XYZW" : "=j DOUT" (DotVec) : "j XYZW" (XYZW) );
    asm( "vaddz.x   DOUT,   DIN,    DINz" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "vaddy.x   DOUT,   DIN,    DINy" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
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
}

//==============================================================================

inline 
xbool vector3t::SafeNormalizeAndScale( f32 Scalar )
{
    FORCE_ALIGNED_16( this );

    u128    DotVec;
    f32     Dot;

    asm( "vmul.xyz  DOUT,   XYZW,   XYZW" : "=j DOUT" (DotVec) : "j XYZW" (XYZW) );
    asm( "vaddz.x   DOUT,   DIN,    DINz" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "vaddy.x   DOUT,   DIN,    DINy" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
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
}

//==============================================================================

inline
void vector3t::Scale( f32 Scalar )
{
    FORCE_ALIGNED_16( this );

    u128 ScaleVec;
    asm( "qmtc2      FSCL, VSCL"       : "=j VSCL" (ScaleVec) : "r FSCL" (Scalar) );
    asm( "vmulx.xyzw VOUT, VIN, VSCLx" : "=j VOUT" (XYZW)     : "j VIN"  (XYZW), "j VSCL" (ScaleVec) );
}

//==============================================================================

inline
f32 vector3t::Length( void ) const
{
    FORCE_ALIGNED_16( this );

    u128 DotVec;
    f32  Dot;
    asm( "vmul.xyz  DOUT, XYZW, XYZW" : "=j DOUT" (DotVec) : "j XYZW" (XYZW) );
    asm( "vaddz.x   DOUT, DIN,  DINz" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "vaddy.x   DOUT, DIN,  DINy" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "qmfc2     DOT,  DIN"        : "=r DOT"  (Dot)    : "j DIN"  (DotVec) );

    return x_sqrt(Dot);
}

//==============================================================================

inline
f32 vector3t::LengthSquared( void ) const
{
    FORCE_ALIGNED_16( this );

    u128 DotVec;
    f32  Dot;
    asm( "vmul.xyz  DOUT, XYZW, XYZW" : "=j DOUT" (DotVec) : "j XYZW" (XYZW) );
    asm( "vaddz.x   DOUT, DIN,  DINz" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "vaddy.x   DOUT, DIN,  DINy" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "qmfc2     DOT,  DIN"        : "=r DOT"  (Dot)    : "j DIN"  (DotVec) );

    return Dot;
}

//==============================================================================

inline
f32 vector3t::Difference( const vector3t& V ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

    u128 DotVec;
    f32  Dot;
    asm( "vsub.xyz  DOUT, VEC0, VEC1" : "=j DOUT" (DotVec) : "j VEC0" (XYZW), "j VEC1" (V.XYZW) );
    asm( "vmul.xyz  DOUT, DIN,  DIN"  : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "vaddz.x   DOUT, DIN,  DINz" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "vaddy.x   DOUT, DIN,  DINy" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "qmfc2     DOT,  DIN"        : "=r DOT"  (Dot)    : "j DIN"  (DotVec) );

    return Dot;
}

//==============================================================================

inline
xbool vector3t::InRange( f32 aMin, f32 aMax ) const
{
    FORCE_ALIGNED_16( this );

    return( (GetX()>=aMin) && (GetX()<=aMax) &&
            (GetY()>=aMin) && (GetY()<=aMax) &&
            (GetZ()>=aMin) && (GetZ()<=aMax) );
}

//==============================================================================

inline
xbool vector3t::IsValid( void ) const
{
    FORCE_ALIGNED_16( this );

    return( x_isvalid(GetX()) && x_isvalid(GetY()) && x_isvalid(GetZ()) );
}

//==============================================================================

inline
f32 vector3t::Dot( const vector3t& V ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

    u128 DotVec;
    f32  Dot;
    asm( "vmul.xyz  DOUT, VEC0, VEC1" : "=j DOUT" (DotVec) : "j VEC0" (XYZW), "j VEC1" (V.XYZW) );
    asm( "vaddz.x   DOUT, DIN,  DINz" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "vaddy.x   DOUT, DIN,  DINy" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "qmfc2     DOT,  DIN"        : "=r DOT"  (Dot)    : "j DIN"  (DotVec) );

    return Dot;
}

//==============================================================================

inline
vector3t& vector3t::Snap( f32 GridX, f32 GridY, f32 GridZ )
{
    FORCE_ALIGNED_16( this );

    GetX() = x_round( GetX(), GridX );
    GetY() = x_round( GetY(), GridY );
    GetZ() = x_round( GetZ(), GridZ );

    return *this;
}

//==============================================================================

inline radian vector3t::GetPitch( void ) const
{
    FORCE_ALIGNED_16( this );

    f32  L;
    u128 LVec;
    asm( "vmul.xz   LVEC, XYZW, XYZW" : "=j LVEC" (LVec) : "j XYZW" (XYZW) );
    asm( "vaddz.x   LOUT, LIN,  LINz" : "=j LOUT" (LVec) : "j LIN"  (LVec) );
    asm( "qmfc2     FL,   LVEC"       : "=r FL"   (L)    : "j LVEC" (LVec) );
    L = x_sqrt(L);

    return( -x_atan2( GetY(), L ) );
}

//==============================================================================

inline 
vector3t::vector3t( void )
{
    FORCE_ALIGNED_16( this );
}

//==============================================================================

inline
vector3t::vector3t( f32 X, f32 Y, f32 Z )
{
    FORCE_ALIGNED_16( this );

    Set( X, Y, Z );
}

//==============================================================================

inline
vector3t::vector3t( radian Pitch, radian Yaw )
{
    FORCE_ALIGNED_16( this );

    Set( Pitch, Yaw );
}

//==============================================================================

inline
vector3t vector3t::Cross( const vector3t& V ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

    vector3t Result;
    asm( "vopmula.xyz   acc,    VEC0,   VEC1
          vopmsub.xyz   RES,    VEC1,   VEC0" :
         "=j RES"  (Result.XYZW) :
         "j  VEC0" (XYZW),
         "j  VEC1" (V.XYZW) );

    return Result;
}

//==============================================================================

inline
vector3t vector3t::Reflect( const vector3t& Normal ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Normal );

    f32      fTwo = 2.0f;
    vector3t Result;
    u128     DotVec;
    u128     TwoVec;

    asm( "vmul.xyz  DOUT, VEC0, VEC1"  : "=j DOUT" (DotVec)      : "j VEC0" (Normal.XYZW), "j VEC1" (XYZW)        );
    asm( "vaddz.x   DOUT, DIN,  DINz"  : "=j DOUT" (DotVec)      : "j DIN"  (DotVec)                              );
    asm( "vaddy.x   DOUT, DIN,  DINy"  : "=j DOUT" (DotVec)      : "j DIN"  (DotVec)                              );
    asm( "qmtc2     FTWO, VTWO"        : "=j VTWO" (TwoVec)      : "r FTWO" (fTwo)                                );
    asm( "vmulx.x   DOUT, DIN,  VTWOx" : "=j DOUT" (DotVec)      : "j DIN"  (DotVec),      "j VTWO" (TwoVec)      );
    asm( "vmulx.xyz RES,  NORM, DOUTx" : "=j RES"  (Result.XYZW) : "j NORM" (Normal.XYZW), "j DOUT" (DotVec)      );
    asm( "vsub.xyz  RES,  VEC0, VEC1"  : "=j RES"  (Result.XYZW) : "j VEC0" (XYZW),        "j VEC1" (Result.XYZW) );

    return Result;
}

//==============================================================================

inline 
void vector3t::RotateX( radian Angle )
{
    FORCE_ALIGNED_16( this );

    f32 S, C;
    x_sincos( Angle, S, C );

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
}

//==============================================================================

inline
void vector3t::RotateY( radian Angle )
{
    FORCE_ALIGNED_16( this );

    f32 S, C;
    x_sincos( Angle, S, C );

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
}

//==============================================================================

inline
void vector3t::RotateZ( radian Angle )
{
    FORCE_ALIGNED_16( this );
    
    f32 S, C;
    x_sincos( Angle, S, C );

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
}

//==============================================================================

inline 
void vector3t::Rotate( const radian3& R )
{
    FORCE_ALIGNED_16( this );

    f32 sx, cx;
    f32 sy, cy;
    f32 sz, cz;

    x_sincos( R.Pitch, sx, cx );
    x_sincos( R.Yaw,   sy, cy );
    x_sincos( R.Roll,  sz, cz );

    // fill out 3x3 rotations
    f32 sxsz = sx * sz;
    f32 sxcz = sx * cz;

    vector3t Mat0( cy*cz+sy*sxsz,   cx*sz,  cy*sxsz-sy*cz );
    vector3t Mat1( sy*sxcz-sz*cy,   cx*cz,  sy*sz+sxcz*cy );
    vector3t Mat2( cx*sy,           -sx,    cx*cy         );

    // 3x3 matrix times vector
    u128 Result;
    asm( "vmove.w       RES,    XYZW
          vmulaz.xyz    acc,    COL2,   XYZWz
          vmadday.xyz   acc,    COL1,   XYZWy
          vmaddx.xyz    RES,    COL0,   XYZWx" :
         "=&j RES"  (Result) :
         "j   XYZW" (XYZW), "j COL0" (Mat0.XYZW), "j COL1" (Mat1.XYZW), "j COL2" (Mat2.XYZW) );

    XYZW = Result;
}

//==============================================================================

inline 
vector3t vector3t::operator - ( void ) const
{
    FORCE_ALIGNED_16( this );

    vector3t    Result;

    asm( "vsub.xyz  RES, vf00, XYZW" : "=j RES" (Result.XYZW) : "j XYZW" (XYZW) );

    return Result;
}

//==============================================================================

inline
const vector3t& vector3t::operator = ( const vector4& V )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

    asm( "vmove.xyzw    XYZW, VEC4" : "=j XYZW" (XYZW) : "j VEC4" (V.GetU128()) );
    return( *this );
}

//==============================================================================

inline
vector3t& vector3t::operator += ( const vector3t& V )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

    asm( "vadd.xyzw     VOUT, VEC0, VEC1" : "=j VOUT" (XYZW) : "j VEC0" (XYZW), "j VEC1" (V.XYZW) );
    return( *this );
}

//==============================================================================

inline
vector3t& vector3t::operator -= ( const vector3t& V )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

    asm( "vsub.xyzw     VOUT, VEC0, VEC1" : "=j VOUT" (XYZW) : "j VEC0" (XYZW), "j VEC1" (V.XYZW) );
    return( *this );
}

//==============================================================================

inline 
vector3t& vector3t::operator *= ( const f32 Scalar )
{
    FORCE_ALIGNED_16( this );

    u128 ScaleVec;
    asm( "qmtc2         FSCL, VSCL"       : "=j VSCL" (ScaleVec) : "r FSCL" (Scalar) );
    asm( "vmulx.xyzw    VOUT, VIN, VSCLx" : "=j VOUT" (XYZW)     : "j VIN"  (XYZW), "j VSCL" (ScaleVec) );
    return( *this );
}

//==============================================================================

inline
vector3t& vector3t::operator /= ( const f32 Scalar )
{
    FORCE_ALIGNED_16( this );

    ASSERT( Scalar != 0.0f );
    f32 d = 1.0f / Scalar;

    u128 ScaleVec;
    asm( "qmtc2         FSCL, VSCL"       : "=j VSCL" (ScaleVec) : "r FSCL" (d) );
    asm( "vmulx.xyzw    VOUT, VIN, VSCLx" : "=j VOUT" (XYZW)     : "j VIN"  (XYZW), "j VSCL" (ScaleVec) );
    return( *this );
}

//==============================================================================

inline 
vector3t operator + ( const vector3t& V1, const vector3t& V2 )
{
    FORCE_ALIGNED_16( &V1 );
    FORCE_ALIGNED_16( &V2 );

    vector3t    Result;
    asm( "vadd.xyzw   XYZW, V1, V2" : "=j XYZW" (Result.XYZW) : "j V1" (V1.XYZW), "j V2" (V2.XYZW) );
    return Result;
}

//==============================================================================

inline 
vector3t operator - ( const vector3t& V1, const vector3t& V2 )
{
    FORCE_ALIGNED_16( &V1 );
    FORCE_ALIGNED_16( &V2 );

    vector3t    Result;
    asm
    ("
        vsub.xyzw   XYZW,   V1, V2
    " :
    "=j XYZW" (Result.XYZW) :
    "j  V1"   (V1.XYZW),
    "j  V2"   (V2.XYZW) );

    return Result;
}

//==============================================================================

inline
vector3t operator / ( const vector3t& V, f32 S )
{
    FORCE_ALIGNED_16( &V );

    ASSERT( S != 0.0f );
    f32 d = 1.0f / S;

    u128     ScaleVec;
    vector3t Result;
    asm( "qmtc2         FSCL, VSCL"       : "=j VSCL" (ScaleVec)    : "r FSCL" (d) );
    asm( "vmulx.xyzw    VOUT, VIN, VSCLx" : "=j VOUT" (Result.XYZW) : "j VIN"  (V.XYZW), "j VSCL" (ScaleVec) );
    
    return Result;
}

//==============================================================================

inline
vector3t operator * ( const vector3t& V, f32 S )
{
    FORCE_ALIGNED_16( &V );

    u128     ScaleVec;
    vector3t Result;
    asm( "qmtc2         FSCL, VSCL"       : "=j VSCL" (ScaleVec)    : "r FSCL" (S) );
    asm( "vmulx.xyzw    VOUT, VIN, VSCLx" : "=j VOUT" (Result.XYZW) : "j VIN"  (V.XYZW), "j VSCL" (ScaleVec) );

    return Result;
}

//==============================================================================

inline
vector3t operator * ( f32 S, const vector3t& V )
{
    FORCE_ALIGNED_16( &V );

    u128     ScaleVec;
    vector3t Result;
    asm( "qmtc2         FSCL, VSCL"       : "=j VSCL" (ScaleVec)    : "r FSCL" (S) );
    asm( "vmulx.xyzw    VOUT, VIN, VSCLx" : "=j VOUT" (Result.XYZW) : "j VIN"  (V.XYZW), "j VSCL" (ScaleVec) );

    return Result;
}

//==============================================================================

inline
vector3t operator * ( const vector3t& V1, const vector3t& V2 )
{
    FORCE_ALIGNED_16( &V1 );
    FORCE_ALIGNED_16( &V2 );

    vector3t Result;
    asm( "vmul.xyzw     VOUT, VEC1, VEC2" : "=j VOUT" (Result.XYZW) : "j VEC1" (V1.XYZW), "j VEC2" (V2.XYZW) );
    return Result;
}

//==============================================================================

inline
f32 v3_Dot( const vector3t& V1, const vector3t& V2 )
{
    FORCE_ALIGNED_16( &V1 );
    FORCE_ALIGNED_16( &V2 );

    u128 DotVec;
    f32  Dot;
    asm( "vmul.xyz  DOUT, VEC0, VEC1" : "=j DOUT" (DotVec) : "j VEC0" (V1.XYZW), "j VEC1" (V2.XYZW) );
    asm( "vaddz.x   DOUT, DIN,  DINz" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "vaddy.x   DOUT, DIN,  DINy" : "=j DOUT" (DotVec) : "j DIN"  (DotVec) );
    asm( "qmfc2     DOT,  DIN"        : "=r DOT"  (Dot)    : "j DIN"  (DotVec) );

    return Dot;
}

//==============================================================================

inline
vector3t v3_Cross( const vector3t& V1, const vector3t& V2 )
{
    FORCE_ALIGNED_16( &V1 );
    FORCE_ALIGNED_16( &V2 );

    vector3t Result;
    asm( "vopmula.xyz   acc,    VEC0,   VEC1
          vopmsub.xyz   RES,    VEC1,   VEC0" :
         "=j RES"  (Result.XYZW) :
         "j  VEC0" (V1.XYZW),
         "j  VEC1" (V2.XYZW) );

    return Result;
}

//==============================================================================

inline
vector3t v3_Reflect( const vector3t& V,  const vector3t& Normal )
{
    FORCE_ALIGNED_16( &V );
    FORCE_ALIGNED_16( &Normal );

    f32 fTwo = 2.0f;

    vector3t Result;
    u128     DotVec;
    u128     TwoVec;

    asm( "vmul.xyz  DOUT, VEC0, VEC1"  : "=j DOUT" (DotVec)      : "j VEC0" (Normal.XYZW), "j VEC1" (V.XYZW)      );
    asm( "vaddz.x   DOUT, DIN,  DINz"  : "=j DOUT" (DotVec)      : "j DIN"  (DotVec)                              );
    asm( "vaddy.x   DOUT, DIN,  DINy"  : "=j DOUT" (DotVec)      : "j DIN"  (DotVec)                              );
    asm( "qmtc2     FTWO, VTWO"        : "=j VTWO" (TwoVec)      : "r FTWO" (fTwo)                                );
    asm( "vmulx.x   DOUT, DIN,  VTWOx" : "=j DOUT" (DotVec)      : "j DIN"  (DotVec),      "j VTWO" (TwoVec)      );
    asm( "vmulx.xyz RES,  NORM, DOUTx" : "=j RES"  (Result.XYZW) : "j NORM" (Normal.XYZW), "j DOUT" (DotVec)      );
    asm( "vsub.xyz  RES,  VEC0, VEC1"  : "=j RES"  (Result.XYZW) : "j VEC0" (V.XYZW),      "j VEC1" (Result.XYZW) );

    return Result;
}
