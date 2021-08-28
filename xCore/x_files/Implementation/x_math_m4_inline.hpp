//==============================================================================
//
//  x_math_m4_inline.hpp
//
//==============================================================================

#ifndef X_MATH_M4_INLINE_HPP
#define X_MATH_M4_INLINE_HPP
#else
#error "File " __FILE__ " has been included twice!"
#endif

#ifndef X_DEBUG_HPP
#include "../x_debug.hpp"
#endif

#ifndef X_PLUS_HPP
#include "../x_plus.hpp"
#endif

#ifdef TARGET_XBOX
#include <xtl.h>
#include <xgmath.h>
#endif

//==============================================================================
//  DEFINES
//==============================================================================

#define M(row,column)   m_Cell[row][column]

//==============================================================================
//  META-TEMPLATES
//==============================================================================

/* identity */

template< class e,s32 N,s32 C=0,s32 R=0,s32 I=0 >struct meta_identity
{
    enum
    {
        NxtI = I+1,           // Counter
        NxtR =  NxtI%N,       // Row (inner loop)
        NxtC = (NxtI/N)%N     // Column (outer loop)
    };

    static void calc( e& m )
    {
        m( R,C )=( C==R );
        meta_identity< e,N,NxtC,NxtR,NxtI >::calc( m );
    }
};

/* identity specialisations */

template<>struct meta_identity< matrix4,4,0,0,4*4 >
{
    static void calc( matrix4& )
    {
    }
};

template<>struct meta_identity< matrix3,3,0,0,3*3 >
{
    static void calc( matrix3& )
    {
    }
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

inline
f32 matrix4::operator () ( s32 Column, s32 Row ) const
{
    FORCE_ALIGNED_16( this );
    return( m_Cell[Column][Row] );
}

//==============================================================================

inline
f32& matrix4::operator () ( s32 Column, s32 Row )
{
    FORCE_ALIGNED_16( this );
    return( m_Cell[Column][Row] );
}

//==============================================================================

inline 
void matrix4::Zero( void )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    asm( "vsub.xyzw  %0, vf00, vf00" : "=j" (m_Col0) );
    asm( "vsub.xyzw  %0, vf00, vf00" : "=j" (m_Col1) );
    asm( "vsub.xyzw  %0, vf00, vf00" : "=j" (m_Col2) );
    asm( "vsub.xyzw  %0, vf00, vf00" : "=j" (m_Col3) );
#else
    M(0,0) = M(1,0) = M(2,0) = M(3,0) = 0.0f;
    M(0,1) = M(1,1) = M(2,1) = M(3,1) = 0.0f;
    M(0,2) = M(1,2) = M(2,2) = M(3,2) = 0.0f;
    M(0,3) = M(1,3) = M(2,3) = M(3,3) = 0.0f;
#endif
}

//==============================================================================

inline 
void matrix4::Identity( void )
{    
    FORCE_ALIGNED_16( this );

#if USE_VU0
    // why does this cause an internal compiler error?
    //asm( "vmove.xyzw    COL3, vf00" : "=j COL3" (m_Col3) );
    // but this works? bah, stupid gcc
    asm( "vmove.xyzw    %0, vf00" : "=j" (m_Col3) );
    asm( "vmr32.xyzw    %0, vf00" : "=j" (m_Col2) );
    asm( "vmr32.xyzw    %0, %1"   : "=j" (m_Col1) : "j" (m_Col2) );
    asm( "vmr32.xyzw    %0, %1"   : "=j" (m_Col0) : "j" (m_Col1) );
#elif defined TARGET_XBOX
    meta_identity<matrix4,4>::calc( *this );
#else
             M(1,0) = M(2,0) = M(3,0) = 0.0f;
    M(0,1)          = M(2,1) = M(3,1) = 0.0f;
    M(0,2) = M(1,2)          = M(3,2) = 0.0f;
    M(0,3) = M(1,3) = M(2,3)          = 0.0f;

    M(0,0) = 1.0f;
    M(1,1) = 1.0f;
    M(2,2) = 1.0f;
    M(3,3) = 1.0f;
#endif
}

//==============================================================================

inline
void matrix4::Transpose( void )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0

    u128    Tmp0, Tmp1, Tmp2, Tmp3;

    asm( "pextlw %0, %1, %2" : "=r" (Tmp0)   : "r" (m_Col1), "r" (m_Col0) );
    asm( "pextuw %0, %1, %2" : "=r" (Tmp1)   : "r" (m_Col1), "r" (m_Col0) );
    asm( "pextlw %0, %1, %2" : "=r" (Tmp2)   : "r" (m_Col3), "r" (m_Col2) );
    asm( "pextuw %0, %1, %2" : "=r" (Tmp3)   : "r" (m_Col3), "r" (m_Col2) );
    asm( "pcpyld %0, %1, %2" : "=r" (m_Col0) : "r" (Tmp2),   "r" (Tmp0)   );
    asm( "pcpyud %0, %1, %2" : "=r" (m_Col1) : "r" (Tmp0),   "r" (Tmp2)   );
    asm( "pcpyld %0, %1, %2" : "=r" (m_Col2) : "r" (Tmp3),   "r" (Tmp1)   );
    asm( "pcpyud %0, %1, %2" : "=r" (m_Col3) : "r" (Tmp1),   "r" (Tmp3)   );

#elif defined TARGET_XBOX
    XGMatrixTranspose((XGMATRIX*)this,(CONST XGMATRIX*)this );
#else

    f32     T;    
   
    T      = M(1,0);
    M(1,0) = M(0,1);
    M(0,1) = T;
    
    T      = M(2,0);
    M(2,0) = M(0,2);
    M(0,2) = T;
    
    T      = M(3,0);
    M(3,0) = M(0,3);
    M(0,3) = T;

    T      = M(2,1);
    M(2,1) = M(1,2);
    M(1,2) = T;
    
    T      = M(3,1);
    M(3,1) = M(1,3);
    M(1,3) = T;

    T      = M(3,2);
    M(3,2) = M(2,3);
    M(2,3) = T;
#endif
}

//==============================================================================

inline
matrix4 m4_InvertRT( const matrix4& Src )
{
    FORCE_ALIGNED_16( &Src );

    matrix4 Dest;

#if USE_VU0

    u128    Tmp0, Tmp1, Tmp2, Tmp3;

    asm( "pextlw %0, %1, %2" : "=r" (Tmp0)        : "r" (Src.m_Col1), "r" (Src.m_Col0) );
    asm( "pextuw %0, %1, %2" : "=r" (Tmp1)        : "r" (Src.m_Col1), "r" (Src.m_Col0) );
    asm( "pextlw %0, $0, %1" : "=r" (Tmp2)        : "r" (Src.m_Col2)                   );
    asm( "pextuw %0, $0, %1" : "=r" (Tmp3)        : "r" (Src.m_Col2)                   );
    asm( "pcpyld %0, %1, %2" : "=r" (Dest.m_Col0) : "r" (Tmp2),       "r" (Tmp0)       );
    asm( "pcpyud %0, %1, %2" : "=r" (Dest.m_Col1) : "r" (Tmp0),       "r" (Tmp2)       );
    asm( "pcpyld %0, %1, %2" : "=r" (Dest.m_Col2) : "r" (Tmp3),       "r" (Tmp1)       );
    asm( "vmulaw.xyzw  acc,  vf00, vf00w
          vmsubaz.xyzw acc,  DIN2, COL3z
          vmsubay.xyzw acc,  DIN1, COL3y
          vmsubx.xyzw  DST3, DIN0, COL3x" :
         "=j DST3" (Dest.m_Col3) :
         "j  DIN0" (Dest.m_Col0),
         "j  DIN1" (Dest.m_Col1),
         "j  DIN2" (Dest.m_Col2),
         "j  COL3" (Src.m_Col3) );
#else
    Dest(0,0) = Src(0,0);
    Dest(0,1) = Src(1,0);
    Dest(0,2) = Src(2,0);
    Dest(1,0) = Src(0,1);
    Dest(1,1) = Src(1,1);
    Dest(1,2) = Src(2,1);
    Dest(2,0) = Src(0,2);
    Dest(2,1) = Src(1,2);
    Dest(2,2) = Src(2,2);
    Dest(0,3) = 0.0f;
    Dest(1,3) = 0.0f;
    Dest(2,3) = 0.0f;
    Dest(3,3) = 1.0f;
    Dest(3,0) = -(Src(3,0)*Dest(0,0) + Src(3,1)*Dest(1,0) + Src(3,2)*Dest(2,0));
    Dest(3,1) = -(Src(3,0)*Dest(0,1) + Src(3,1)*Dest(1,1) + Src(3,2)*Dest(2,1));
    Dest(3,2) = -(Src(3,0)*Dest(0,2) + Src(3,1)*Dest(1,2) + Src(3,2)*Dest(2,2));
#endif

    return Dest;
}

//==============================================================================

inline
xbool matrix4::InvertRT( void )
{
    FORCE_ALIGNED_16( this );

    *this = m4_InvertRT( *this );
    return TRUE;
}

//==============================================================================

#if USE_VU0
inline
matrix4 operator * ( const matrix4& LHS, const matrix4& RHS )
{
    FORCE_ALIGNED_16( &LHS );
    FORCE_ALIGNED_16( &RHS );

    matrix4 Result;
    asm( "vmulaw.xyzw  acc,  LHS3, RHS0w
          vmaddaz.xyzw acc,  LHS2, RHS0z
          vmadday.xyzw acc,  LHS1, RHS0y
          vmaddx.xyzw  DST0, LHS0, RHS0x" :
         "=j DST0" (Result.m_Col0) :
         "j  LHS0" (LHS.m_Col0),
         "j  LHS1" (LHS.m_Col1),
         "j  LHS2" (LHS.m_Col2),
         "j  LHS3" (LHS.m_Col3),
         "j  RHS0" (RHS.m_Col0) );
    asm( "vmulaw.xyzw  acc,  LHS3, RHS1w
          vmaddaz.xyzw acc,  LHS2, RHS1z
          vmadday.xyzw acc,  LHS1, RHS1y
          vmaddx.xyzw  DST1, LHS0, RHS1x" :
         "=j DST1" (Result.m_Col1) :
         "j  LHS0" (LHS.m_Col0),
         "j  LHS1" (LHS.m_Col1),
         "j  LHS2" (LHS.m_Col2),
         "j  LHS3" (LHS.m_Col3),
         "j  RHS1" (RHS.m_Col1) );
    asm( "vmulaw.xyzw  acc,  LHS3, RHS2w
          vmaddaz.xyzw acc,  LHS2, RHS2z
          vmadday.xyzw acc,  LHS1, RHS2y
          vmaddx.xyzw  DST2, LHS0, RHS2x" :
         "=j DST2" (Result.m_Col2) :
         "j  LHS0" (LHS.m_Col0),
         "j  LHS1" (LHS.m_Col1),
         "j  LHS2" (LHS.m_Col2),
         "j  LHS3" (LHS.m_Col3),
         "j  RHS2" (RHS.m_Col2) );
    asm( "vmulaw.xyzw  acc,  LHS3, RHS3w
          vmaddaz.xyzw acc,  LHS2, RHS3z
          vmadday.xyzw acc,  LHS1, RHS3y
          vmaddx.xyzw  DST3, LHS0, RHS3x" :
         "=j DST3" (Result.m_Col3) :
         "j  LHS0" (LHS.m_Col0),
         "j  LHS1" (LHS.m_Col1),
         "j  LHS2" (LHS.m_Col2),
         "j  LHS3" (LHS.m_Col3),
         "j  RHS3" (RHS.m_Col3) );

    return Result;
}
#elif defined TARGET_XBOX
__forceinline
matrix4 operator * ( const matrix4& LHS, const matrix4& RHS )
{
    matrix4 Result;
    XGMatrixMultiply((XGMATRIX*)&Result,(CONST XGMATRIX*)&RHS,(CONST XGMATRIX*)&LHS );
    return Result;
}
#endif

//==============================================================================

inline 
vector3 matrix4::GetScale( void ) const
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128 Sqrts;
    u128 Temp0;
    u128 Temp1;
    u128 Temp2;

    asm( "vmul.xyz  %0, %1, %1" : "=j" (Temp0) : "j" (m_Col0) );
    asm( "vmul.xyz  %0, %1, %1" : "=j" (Temp1) : "j" (m_Col1) );
    asm( "vmul.xyz  %0, %1, %1" : "=j" (Temp2) : "j" (m_Col2) );
    asm( "vaddy.x   SQRT, TMP0, TMP0y
          vaddz.y   SQRT, TMP1, TMP1z
          vaddx.z   SQRT, TMP2, TMP2x
          vaddz.x   SQRT, SQRT, TMP0z
          vaddx.y   SQRT, SQRT, TMP1x
          vaddy.z   SQRT, SQRT, TMP2y" :
         "=&j SQRT" (Sqrts) :
         "j   TMP0" (Temp0),
         "j   TMP1" (Temp1),
         "j   TMP2" (Temp2) );
    asm( "
          vsqrt     q,    %1x
          vwaitq
          vaddq.x   %0x,  vf00,   q
          vsqrt     q,    %1y
          vwaitq
          vaddq.y   %0y,  vf00,   q
          vsqrt     q,    %1z
          vwaitq
          vaddq.z   %0z,  vf00,   q
          " :
         "=j" (Sqrts) :
         "j"  (Sqrts) );
    
    return vector3(Sqrts);
#else
    return( vector3(x_sqrt( M(0,0)*M(0,0) + M(0,1)*M(0,1) +  M(0,2)*M(0,2) ),
                    x_sqrt( M(1,0)*M(1,0) + M(1,1)*M(1,1) +  M(1,2)*M(1,2) ),
                    x_sqrt( M(2,0)*M(2,0) + M(2,1)*M(2,1) +  M(2,2)*M(2,2) )) );
#endif
}

//==============================================================================

inline 
vector3 matrix4::GetTranslation( void ) const
{
    FORCE_ALIGNED_16( this );
    
#if USE_VU0
    return vector3( m_Col3 );
#else
    return( vector3( M(3,0), M(3,1), M(3,2) ) );
#endif
}

//==============================================================================

inline
radian3 matrix4::GetRotation( void ) const
{
    FORCE_ALIGNED_16( this );

    f32             Roll, Pitch, Yaw;
    const matrix4&  M = *this;

    // rot =  cy*cz-sx*sy*sz -cx*sz           cz*sy+cy*sx*sz
    //        cz*sx*sy+cy*sz  cx*cz          -cy*cz*sx+sy*sz
    //       -cx*sy           sx              cx*cy

    // Do we have to deal with scales?
    // We loose precision and speed if we have to.
#if USE_VU0
    const f32 l1 = x_abs( vector3( m_Col0 ).LengthSquared() );
    const f32 l2 = x_abs( vector3( m_Col1 ).LengthSquared() );
    const f32 l3 = x_abs( vector3( m_Col2 ).LengthSquared() );
#else
    const f32 l1 = x_abs( vector3( M(0,0), M(0,1), M(0,2) ).LengthSquared() );
    const f32 l2 = x_abs( vector3( M(1,0), M(1,1), M(1,2) ).LengthSquared() );
    const f32 l3 = x_abs( vector3( M(2,0), M(2,1), M(2,2) ).LengthSquared() );
#endif

    if( l1 > (1 + 0.001f) || l1 < (1 - 0.001f) ||
        l2 > (1 + 0.001f) || l2 < (1 - 0.001f) ||
        l3 > (1 + 0.001f) || l3 < (1 - 0.001f) )
    {
        matrix4 OM = M;
        OM.ClearScale();

        if ( OM(2,1) < 1.0f )
        {
            if ( OM(2,1) > -1.0f )
            {
                Roll  =  x_atan2( OM(0,1), OM(1,1));
                Pitch = -x_asin ( OM(2,1));
                Yaw   =  x_atan2( OM(2,0), OM(2,2));
            }
            else
            {
                // WARNING.  Not unique.  ZA - YA = atan(r02,r00)
                Roll  = x_atan2( OM(0,2), OM(0,0));
                Pitch = (PI/2.0f);
                Yaw   = 0.0f;
            }
        }
        else
        {
            // WARNING.  Not unique.  ZA + YA = -atan2(r02,r00)
            Roll  = -x_atan2( OM(0,2), OM(0,0));
            Pitch = -(PI/2.0f);
            Yaw   = 0.0f;
        }
    }
    else
    {
        if ( M(2,1) < 1.0f )
        {
            if ( M(2,1) > -1.0f )
            {
                Roll  =  x_atan2( M(0,1), M(1,1));
                Pitch = -x_asin ( M(2,1));
                Yaw   =  x_atan2( M(2,0), M(2,2));
            }
            else
            {
                // WARNING.  Not unique.  ZA - YA = atan(r02,r00)
                Roll  = x_atan2( M(0,2), M(0,0));
                Pitch = (PI/2.0f);
                Yaw   = 0.0f;
            }
        }
        else
        {
            // WARNING.  Not unique.  ZA + YA = -atan2(r02,r00)
            Roll  = -x_atan2( M(0,2), M(0,0));
            Pitch = -(PI/2.0f);
            Yaw   = 0.0f;
        }
    }
  
    return( radian3( Pitch, Yaw, Roll ) );
}

//==============================================================================

inline
void matrix4::DecomposeSRT( vector3& Scale, quaternion& Rot, vector3& Trans )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Scale );
    FORCE_ALIGNED_16( &Trans );

    Scale = GetScale();
    Rot   = GetQuaternion();
    Trans = GetTranslation();
}

//==============================================================================

inline
void matrix4::DecomposeSRT( vector3& Scale, radian3& Rot, vector3& Trans )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Scale );
    FORCE_ALIGNED_16( &Trans );

    Scale = GetScale();
    Rot   = GetRotation();
    Trans = GetTranslation();
}

//==============================================================================

inline 
void matrix4::ClearScale( void )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128 Sqrts;
    u128 Temp0;
    u128 Temp1;
    u128 Temp2;

    asm( "vmul.xyz  %0, %1, %1" : "=j" (Temp0) : "j" (m_Col0) );
    asm( "vmul.xyz  %0, %1, %1" : "=j" (Temp1) : "j" (m_Col1) );
    asm( "vmul.xyz  %0, %1, %1" : "=j" (Temp2) : "j" (m_Col2) );
    asm( "vaddy.x   SQRT, TMP0, TMP0y
          vaddz.y   SQRT, TMP1, TMP1z
          vaddx.z   SQRT, TMP2, TMP2x
          vaddz.x   SQRT, SQRT, TMP0z
          vaddx.y   SQRT, SQRT, TMP1x
          vaddy.z   SQRT, SQRT, TMP2y" :
         "=&j SQRT" (Sqrts) :
         "j   TMP0" (Temp0),
         "j   TMP1" (Temp1),
         "j   TMP2" (Temp2) );
    asm( "vrsqrt q, vf00w, SQRTx
          vwaitq
          vmulq.xyzw VOUT, VIN, q" :
         "=j VOUT" (m_Col0) :
         "j  VIN"  (m_Col0),
         "j  SQRT" (Sqrts) );
    asm( "vrsqrt q, vf00w, SQRTy
          vwaitq
          vmulq.xyzw VOUT, VIN, q" :
         "=j VOUT" (m_Col1) :
         "j  VIN"  (m_Col1),
         "j  SQRT" (Sqrts) );
    asm( "vrsqrt q, vf00w, SQRTz
          vwaitq
          vmulq.xyzw VOUT, VIN, q" :
         "=j VOUT" (m_Col2) :
         "j  VIN"  (m_Col2),
         "j SQRT" (Sqrts) );
#else
    vector3 S = GetScale();
    this->PreScale( vector3(1/S.GetX(), 1/S.GetY(), 1/S.GetZ()) );
#endif
}

//==============================================================================

inline 
void matrix4::SetScale( f32 Scale )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128 ScaleVec;
    asm( "qmtc2     FSCL, VSCL" : "=j VSCL" (ScaleVec) : "r FSCL" (Scale) );
    asm( "vmove.yzw VOUT, VIN
          vaddx.x   VOUT, vf00, VSCLx" : "=&j VOUT" (m_Col0) : "j VIN" (m_Col0), "j VSCL" (ScaleVec) );
    asm( "vmove.xzw VOUT, VIN
          vaddx.y   VOUT, vf00, VSCLx" : "=&j VOUT" (m_Col1) : "j VIN" (m_Col1), "j VSCL" (ScaleVec) );
    asm( "vmove.xyw VOUT, VIN
          vaddx.z   VOUT, vf00, VSCLx" : "=&j VOUT" (m_Col2) : "j VIN" (m_Col2), "j VSCL" (ScaleVec) );
#else
    M(0,0) = Scale;
    M(1,1) = Scale;
    M(2,2) = Scale;
#endif
}

//==============================================================================

inline 
void matrix4::SetScale( const vector3& Scale )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Scale );

#if USE_VU0
    asm( "vmove.yzw VOUT, VIN
          vadd.x    VOUT, vf00, VSCL" : "=&j VOUT" (m_Col0) : "j VIN" (m_Col0), "j VSCL" (Scale.GetU128()) );
    asm( "vmove.xzw VOUT, VIN
          vadd.y    VOUT, vf00, VSCL" : "=&j VOUT" (m_Col1) : "j VIN" (m_Col1), "j VSCL" (Scale.GetU128()) );
    asm( "vmove.xyw VOUT, VIN
          vadd.z    VOUT, vf00, VSCL" : "=&j VOUT" (m_Col2) : "j VIN" (m_Col2), "j VSCL" (Scale.GetU128()) );
#else
    M(0,0) = Scale.GetX();
    M(1,1) = Scale.GetY();
    M(2,2) = Scale.GetZ();
#endif
}

//==============================================================================

inline 
void matrix4::SetTranslation( const vector3& Translation )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Translation );

#if USE_VU0
    asm( "vmove.w   VOUT, VIN
          vmove.xyz VOUT, TRAN" : "=&j VOUT" (m_Col3) : "j VIN" (m_Col3), "j TRAN" (Translation.GetU128()) );
#else
    M(3,0) = Translation.GetX();
    M(3,1) = Translation.GetY();
    M(3,2) = Translation.GetZ();
#endif
}

//==============================================================================

inline 
void matrix4::ClearRotation( void )
{                    
    FORCE_ALIGNED_16( this );

    vector3 S = GetScale();
    vector3 T = GetTranslation();
    Identity();
    SetScale( S );
    SetTranslation( T );
}

//==============================================================================

inline 
void matrix4::ClearTranslation( void )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    asm( "vmove.xyzw    COL3, vf00" : "=j COL3" (m_Col3) );
#else
    M(3,0) = M(3,1) = M(3,2) = 0.0f;
#endif
}

//==============================================================================

inline 
void matrix4::Scale( f32 Scale )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128 ScaleVec;
    asm( "qmtc2 FSCL, VSCL" : "=j VSCL" (ScaleVec) : "r FSCL" (Scale) );
    asm( "vmove.w   VOUT, VIN
          vmulx.xyz VOUT, VIN, VSCLx" : "=&j VOUT" (m_Col0) : "j VIN" (m_Col0), "j VSCL" (ScaleVec) );
    asm( "vmove.w   VOUT, VIN
          vmulx.xyz VOUT, VIN, VSCLx" : "=&j VOUT" (m_Col1) : "j VIN" (m_Col1), "j VSCL" (ScaleVec) );
    asm( "vmove.w   VOUT, VIN
          vmulx.xyz VOUT, VIN, VSCLx" : "=&j VOUT" (m_Col2) : "j VIN" (m_Col2), "j VSCL" (ScaleVec) );
    asm( "vmove.w   VOUT, VIN
          vmulx.xyz VOUT, VIN, VSCLx" : "=&j VOUT" (m_Col3) : "j VIN" (m_Col3), "j VSCL" (ScaleVec) );
#else
    // Scale the rotation.

    M(0,0) *= Scale;
    M(0,1) *= Scale;
    M(0,2) *= Scale;

    M(1,0) *= Scale;
    M(1,1) *= Scale;
    M(1,2) *= Scale;

    M(2,0) *= Scale;
    M(2,1) *= Scale;
    M(2,2) *= Scale;

    // Scale the translation.

    M(3,0) *= Scale;
    M(3,1) *= Scale;
    M(3,2) *= Scale;
#endif
}

//==============================================================================

inline 
void matrix4::Scale( const vector3& Scale )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Scale );

#if USE_VU0
    asm( "vmove.w   VOUT, VIN
          vmul.xyz  VOUT, VIN, VSCL" : "=&j VOUT" (m_Col0) : "j VIN" (m_Col0), "j VSCL" (Scale.GetU128()) );
    asm( "vmove.w   VOUT, VIN
          vmul.xyz  VOUT, VIN, VSCL" : "=&j VOUT" (m_Col1) : "j VIN" (m_Col1), "j VSCL" (Scale.GetU128()) );
    asm( "vmove.w   VOUT, VIN
          vmul.xyz  VOUT, VIN, VSCL" : "=&j VOUT" (m_Col2) : "j VIN" (m_Col2), "j VSCL" (Scale.GetU128()) );
    asm( "vmove.w   VOUT, VIN
          vmul.xyz  VOUT, VIN, VSCL" : "=&j VOUT" (m_Col3) : "j VIN" (m_Col3), "j VSCL" (Scale.GetU128()) );
#else
    // Scale the rotation.

    M(0,0) *= Scale.GetX();
    M(0,1) *= Scale.GetY();
    M(0,2) *= Scale.GetZ();

    M(1,0) *= Scale.GetX();
    M(1,1) *= Scale.GetY();
    M(1,2) *= Scale.GetZ();

    M(2,0) *= Scale.GetX();
    M(2,1) *= Scale.GetY();
    M(2,2) *= Scale.GetZ();

    // Scale the translation.

    M(3,0) *= Scale.GetX();
    M(3,1) *= Scale.GetY();
    M(3,2) *= Scale.GetZ();
#endif
}

//==============================================================================

inline 
void matrix4::RotateX( radian Rx )
{
    FORCE_ALIGNED_16( this );

    if( Rx == 0 )  return;

    f32 s, c;    
    x_sincos( Rx, s, c );

    // this is the slow method but easy to see what's going on:
    //matrix4 RM;
    //RM.Identity();
    //RM.M(1,1) =  c;
    //RM.M(2,1) = -s;
    //RM.M(1,2) =  s;
    //RM.M(2,2) =  c;
    //*this = RM * *this;    

    // this is the fast method with less multiplies and adds:
    f32 M01 = M(0,1);   M(0,1) = c*M01 - s*M(0,2);  M(0,2) = s*M01 + c*M(0,2);
    f32 M11 = M(1,1);   M(1,1) = c*M11 - s*M(1,2);  M(1,2) = s*M11 + c*M(1,2);
    f32 M21 = M(2,1);   M(2,1) = c*M21 - s*M(2,2);  M(2,2) = s*M21 + c*M(2,2);
    f32 M31 = M(3,1);   M(3,1) = c*M31 - s*M(3,2);  M(3,2) = s*M31 + c*M(3,2);
}

//==============================================================================

inline 
void matrix4::RotateY( radian Ry )
{
    FORCE_ALIGNED_16( this );

    if( Ry == 0 )  return;

    f32 s, c;
    x_sincos( Ry, s, c );

    // this is the slow method but easy to see what's going on:
    //matrix4 RM;
    //RM.Identity();
    //RM.M(0,0) =  c;
    //RM.M(2,0) =  s;
    //RM.M(0,2) = -s;
    //RM.M(2,2) =  c;
    //*this = RM * *this;

    // this is the fast method with less multiplies and adds:
    f32 M00 = M(0,0);   M(0,0) = c*M00 + s*M(0,2);  M(0,2) = c*M(0,2) - s*M00;
    f32 M10 = M(1,0);   M(1,0) = c*M10 + s*M(1,2);  M(1,2) = c*M(1,2) - s*M10;
    f32 M20 = M(2,0);   M(2,0) = c*M20 + s*M(2,2);  M(2,2) = c*M(2,2) - s*M20;
    f32 M30 = M(3,0);   M(3,0) = c*M30 + s*M(3,2);  M(3,2) = c*M(3,2) - s*M30;
}

//==============================================================================

inline 
void matrix4::RotateZ( radian Rz )
{
    FORCE_ALIGNED_16( this );

    if( Rz == 0 )  return;

    f32 s, c;
    x_sincos( Rz, s, c );

    // this is the slow method but easy to see what's going on:
    //matrix4 RM;
    //RM.Identity();
    //RM.M(0,0) =  c;
    //RM.M(1,0) = -s;
    //RM.M(0,1) =  s;
    //RM.M(1,1) =  c;
    //*this = RM * *this;

    // this is the fast method with less multiplies and adds:
    f32 M00 = M(0,0);   M(0,0) = c*M00 - s*M(0,1);  M(0,1) = s*M00 + c*M(0,1);
    f32 M10 = M(1,0);   M(1,0) = c*M10 - s*M(1,1);  M(1,1) = s*M10 + c*M(1,1);
    f32 M20 = M(2,0);   M(2,0) = c*M20 - s*M(2,1);  M(2,1) = s*M20 + c*M(2,1);
    f32 M30 = M(3,0);   M(3,0) = c*M30 - s*M(3,1);  M(3,1) = s*M30 + c*M(3,1);
}

//==============================================================================

inline 
void matrix4::Translate( const vector3& Translation )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Translation );

#if USE_VU0
    u128 NewTrans = Translation.GetU128();
    asm( "vmove.xyz   TOUT, TIN
          vsub.w      TOUT, TOUT, TOUT" : "=j TOUT" (NewTrans) : "j TIN" (NewTrans) );
    asm( "vmulaw.xyzw acc,  VIN,  vf00w
          vmaddw.xyzw VOUT, TRAN, VINw" : "=j VOUT" (m_Col0) : "j VIN" (m_Col0), "j TRAN" (NewTrans) );
    asm( "vmulaw.xyzw acc,  VIN,  vf00w
          vmaddw.xyzw VOUT, TRAN, VINw" : "=j VOUT" (m_Col1) : "j VIN" (m_Col1), "j TRAN" (NewTrans) );
    asm( "vmulaw.xyzw acc,  VIN,  vf00w
          vmaddw.xyzw VOUT, TRAN, VINw" : "=j VOUT" (m_Col2) : "j VIN" (m_Col2), "j TRAN" (NewTrans) );
    asm( "vmulaw.xyzw acc,  VIN,  vf00w
          vmaddw.xyzw VOUT, TRAN, VINw" : "=j VOUT" (m_Col3) : "j VIN" (m_Col3), "j TRAN" (NewTrans) );
#else
    //
    // If bottom row of M is [0 0 0 1] do optimized translation.
    //

    if( ( M(0,3) == 0.0f ) && 
        ( M(1,3) == 0.0f ) &&
        ( M(2,3) == 0.0f ) && 
        ( M(3,3) == 1.0f ) )
    {
        M(3,0) += Translation.GetX();
        M(3,1) += Translation.GetY();
        M(3,2) += Translation.GetZ();
    }
    else 
    {
        M(0,0) += M(0,3) * Translation.GetX();
        M(1,0) += M(1,3) * Translation.GetX();
        M(2,0) += M(2,3) * Translation.GetX();
        M(3,0) += M(3,3) * Translation.GetX();

        M(0,1) += M(0,3) * Translation.GetY();
        M(1,1) += M(1,3) * Translation.GetY();
        M(2,1) += M(2,3) * Translation.GetY();
        M(3,1) += M(3,3) * Translation.GetY();

        M(0,2) += M(0,3) * Translation.GetZ();
        M(1,2) += M(1,3) * Translation.GetZ();
        M(2,2) += M(2,3) * Translation.GetZ();
        M(3,2) += M(3,3) * Translation.GetZ();
    }
#endif

#if (defined bwatson) && (defined X_DEBUG)
    ASSERT( IsValid() );
#endif
}

//==============================================================================

inline 
void matrix4::PreScale( f32 Scale )
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    u128 ScaleVec;
    asm( "qmtc2      FSCL, VSCL"      : "=j VSCL" (ScaleVec) : "r FSCL" (Scale) );
    asm( "vmulx.xyzw VOUT, VIN, VSCL" : "=j VOUT" (m_Col0)   : "j VIN"  (m_Col0), "j VSCL" (ScaleVec) );
    asm( "vmulx.xyzw VOUT, VIN, VSCL" : "=j VOUT" (m_Col1)   : "j VIN"  (m_Col1), "j VSCL" (ScaleVec) );
    asm( "vmulx.xyzw VOUT, VIN, VSCL" : "=j VOUT" (m_Col2)   : "j VIN"  (m_Col2), "j VSCL" (ScaleVec) );
#else
    M(0,0) *= Scale;
    M(0,1) *= Scale;
    M(0,2) *= Scale;
    M(0,3) *= Scale;

    M(1,0) *= Scale;
    M(1,1) *= Scale;
    M(1,2) *= Scale;
    M(1,3) *= Scale;
    
    M(2,0) *= Scale;
    M(2,1) *= Scale;
    M(2,2) *= Scale;
    M(2,3) *= Scale;
#endif
}

//==============================================================================

inline 
void matrix4::PreScale( const vector3& Scale )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Scale );

#if USE_VU0
    asm( "vmulx.xyzw VOUT, VIN, VSCLx" : "=j VOUT" (m_Col0) : "j VIN" (m_Col0), "j VSCL" (Scale.GetU128()) );
    asm( "vmuly.xyzw VOUT, VIN, VSCLy" : "=j VOUT" (m_Col1) : "j VIN" (m_Col1), "j VSCL" (Scale.GetU128()) );
    asm( "vmulz.xyzw VOUT, VIN, VSCLz" : "=j VOUT" (m_Col2) : "j VIN" (m_Col2), "j VSCL" (Scale.GetU128()) );
#else
    M(0,0) *= Scale.GetX();
    M(0,1) *= Scale.GetX();
    M(0,2) *= Scale.GetX();
    M(0,3) *= Scale.GetX();

    M(1,0) *= Scale.GetY();
    M(1,1) *= Scale.GetY();
    M(1,2) *= Scale.GetY();
    M(1,3) *= Scale.GetY();
    
    M(2,0) *= Scale.GetZ();
    M(2,1) *= Scale.GetZ();
    M(2,2) *= Scale.GetZ();
    M(2,3) *= Scale.GetZ();
#endif
}

//==============================================================================

inline 
void matrix4::PreRotateX( radian Rx )
{
    FORCE_ALIGNED_16( this );

    if( Rx == 0 )  return;

    f32 s, c;
    x_sincos( Rx, s, c );

    // this is the slow method but easy to see what's going on:
    //matrix4 RM;
    //RM.Identity();
    //RM.M(1,1) =  c;
    //RM.M(2,1) = -s;
    //RM.M(1,2) =  s;
    //RM.M(2,2) =  c;
    //*this *= RM;    

    // this is the fast method with less multiplies and adds:
    f32 M10 = M(1,0);   M(1,0) = c*M10 + s*M(2,0);  M(2,0) = c*M(2,0) - s*M10;
    f32 M11 = M(1,1);   M(1,1) = c*M11 + s*M(2,1);  M(2,1) = c*M(2,1) - s*M11;
    f32 M12 = M(1,2);   M(1,2) = c*M12 + s*M(2,2);  M(2,2) = c*M(2,2) - s*M12;
    f32 M13 = M(1,3);   M(1,3) = c*M13 + s*M(2,3);  M(2,3) = c*M(2,3) - s*M13;
}

//==============================================================================

inline 
void matrix4::PreRotateY( radian Ry )
{
    FORCE_ALIGNED_16( this );

    if( Ry == 0 )  return;

    f32 s, c;
    x_sincos( Ry, s, c );

    // this is the slow method but easy to see what's going on:
    //matrix4 RM;
    //RM.Identity();
    //RM.M(0,0) =  c;
    //RM.M(2,0) =  s;
    //RM.M(0,2) = -s;
    //RM.M(2,2) =  c;
    //*this *= RM;    

    // this is the fast method with less multiplies and adds:
    f32 M00 = M(0,0);   M(0,0) = c*M00 - s*M(2,0);  M(2,0) = s*M00 + c*M(2,0);
    f32 M01 = M(0,1);   M(0,1) = c*M01 - s*M(2,1);  M(2,1) = s*M01 + c*M(2,1);
    f32 M02 = M(0,2);   M(0,2) = c*M02 - s*M(2,2);  M(2,2) = s*M02 + c*M(2,2);
    f32 M03 = M(0,3);   M(0,3) = c*M03 - s*M(2,3);  M(2,3) = s*M03 + c*M(2,3);
}

//==============================================================================

inline 
void matrix4::PreRotateZ( radian Rz )
{
    FORCE_ALIGNED_16( this );

    if( Rz == 0 )  return;

    f32 s, c;
    x_sincos( Rz, s, c );

    // this is the slow method but easy to see what's going on:
    //matrix4 RM;
    //RM.Identity();
    //RM.M(0,0) =  c;
    //RM.M(1,0) = -s;
    //RM.M(0,1) =  s;
    //RM.M(1,1) =  c;
    //*this *= RM;

    // this is the fast method with less multiplies and adds:
    f32 M00 = M(0,0);   M(0,0) = c*M00 + s*M(1,0);  M(1,0) = c*M(1,0) - s*M00;
    f32 M01 = M(0,1);   M(0,1) = c*M01 + s*M(1,1);  M(1,1) = c*M(1,1) - s*M01;
    f32 M02 = M(0,2);   M(0,2) = c*M02 + s*M(1,2);  M(1,2) = c*M(1,2) - s*M02;
    f32 M03 = M(0,3);   M(0,3) = c*M03 + s*M(1,3);  M(1,3) = c*M(1,3) - s*M03;
}

//==============================================================================

inline 
void matrix4::PreTranslate( const vector3& Translation )
{                       
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Translation );

#if USE_VU0
    asm( "vmulaw.xyzw  acc,  COL3, vf00w
          vmaddaz.xyzw acc,  COL2, TRANz
          vmadday.xyzw acc,  COL1, TRANy
          vmaddx.xyzw  VOUT, COL0, TRANx" :
         "=j VOUT" (m_Col3) :
         "j  COL0" (m_Col0),
         "j  COL1" (m_Col1),
         "j  COL2" (m_Col2),
         "j  COL3" (m_Col3),
         "j  TRAN" (Translation.GetU128()) );
#else
    M(3,0) += (M(0,0) * Translation.GetX()) + (M(1,0) * Translation.GetY()) + (M(2,0) * Translation.GetZ());
    M(3,1) += (M(0,1) * Translation.GetX()) + (M(1,1) * Translation.GetY()) + (M(2,1) * Translation.GetZ());
    M(3,2) += (M(0,2) * Translation.GetX()) + (M(1,2) * Translation.GetY()) + (M(2,2) * Translation.GetZ());
    M(3,3) += (M(0,3) * Translation.GetX()) + (M(1,3) * Translation.GetY()) + (M(2,3) * Translation.GetZ());
#endif
}

//==============================================================================

inline 
void matrix4::GetRows( vector3& V1, vector3& V2, vector3& V3 ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V1 );
    FORCE_ALIGNED_16( &V2 );
    FORCE_ALIGNED_16( &V3 );

#if USE_VU0
    u128 Tmp0;
    u128 Tmp1;
    u128 Tmp2;
    u128 Tmp3;
    u128 Row0;
    u128 Row1;
    u128 Row2;
    asm( "pextlw %0, %1, %2" : "=r" (Tmp0) : "r" (m_Col1), "r" (m_Col0) );
    asm( "pextuw %0, %1, %2" : "=r" (Tmp1) : "r" (m_Col1), "r" (m_Col0) );
    asm( "pextlw %0, %1, %2" : "=r" (Tmp2) : "r" (m_Col3), "r" (m_Col2) );
    asm( "pextuw %0, %1, %2" : "=r" (Tmp3) : "r" (m_Col3), "r" (m_Col2) );
    asm( "pcpyld %0, %1, %2" : "=r" (Row0) : "r" (Tmp2),   "r" (Tmp0)   );
    asm( "pcpyud %0, %1, %2" : "=r" (Row1) : "r" (Tmp0),   "r" (Tmp2)   );
    asm( "pcpyld %0, %1, %2" : "=r" (Row2) : "r" (Tmp3),   "r" (Tmp1)   );
    V1.Set(Row0);
    V2.Set(Row1);
    V3.Set(Row2);
#else
    V1.GetX() = M(0,0);    V1.GetY() = M(1,0);    V1.GetZ() = M(2,0);
    V2.GetX() = M(0,1);    V2.GetY() = M(1,1);    V2.GetZ() = M(2,1);
    V3.GetX() = M(0,2);    V3.GetY() = M(1,2);    V3.GetZ() = M(2,2);
#endif
}

//==============================================================================

inline 
void matrix4::GetColumns( vector3& V1, vector3& V2, vector3& V3 ) const
{
    FORCE_ALIGNED_16( this );

#if USE_VU0
    V1.Set(m_Col0);
    V2.Set(m_Col1);
    V3.Set(m_Col2);
#else
    V1.GetX() = M(0,0);    V2.GetX() = M(1,0);    V3.GetX() = M(2,0);
    V1.GetY() = M(0,1);    V2.GetY() = M(1,1);    V3.GetY() = M(2,1);
    V1.GetZ() = M(0,2);    V2.GetZ() = M(1,2);    V3.GetZ() = M(2,2);
#endif
}

//==============================================================================

#ifdef TARGET_PS2

inline
u128 matrix4::GetCol0_U128( void ) const
{
    return m_Col0;
}

inline
u128 matrix4::GetCol1_U128( void ) const
{
    return m_Col1;
}

inline
u128 matrix4::GetCol2_U128( void ) const
{
    return m_Col2;
}

inline
u128 matrix4::GetCol3_U128( void ) const
{
    return m_Col3;
}

#endif

//==============================================================================

inline 
void matrix4::SetRows( const vector3& V1, 
                       const vector3& V2, 
                       const vector3& V3 )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V1 );
    FORCE_ALIGNED_16( &V2 );
    FORCE_ALIGNED_16( &V3 );

    #if USE_VU0
    u128 Vec0 = V1.GetU128();
    u128 Vec1 = V2.GetU128();
    u128 Vec2 = V3.GetU128();
    u128 Tmp0;
    u128 Tmp1;
    u128 Tmp2;
    u128 Tmp3;
    asm( "pextlw %0, %1, %2" : "=r" (Tmp0) : "r" (Vec1), "r" (Vec0)   );
    asm( "pextuw %0, %1, %2" : "=r" (Tmp1) : "r" (Vec1), "r" (Vec0)   );
    asm( "pextlw %0, $0, %1" : "=r" (Tmp2) : "r" (Vec2)               );
    asm( "pextuw %0, $0, %1" : "=r" (Tmp3) : "r" (Vec2)               );
    asm( "pcpyld %0, %1, %2" : "=r" (Vec0) : "r" (Tmp2),   "r" (Tmp0) );
    asm( "pcpyud %0, %1, %2" : "=r" (Vec1) : "r" (Tmp0),   "r" (Tmp2) );
    asm( "pcpyld %0, %1, %2" : "=r" (Vec2) : "r" (Tmp3),   "r" (Tmp1) );
    asm( "vmove.xyz COL0, VEC0
          vmove.w   COL0, CIN0" : "=j COL0" (m_Col0) : "j VEC0" (Vec0), "j CIN0" (m_Col0) );
    asm( "vmove.xyz COL1, VEC1
          vmove.w   COL1, CIN1" : "=j COL1" (m_Col1) : "j VEC1" (Vec1), "j CIN1" (m_Col1) );
    asm( "vmove.xyz COL2, VEC2
          vmove.w   COL2, CIN2" : "=j COL2" (m_Col2) : "j VEC2" (Vec2), "j CIN2" (m_Col2) );
    #else
    M(0,0) = V1.GetX(); M(1,0) = V1.GetY(); M(2,0) = V1.GetZ();
    M(0,1) = V2.GetX(); M(1,1) = V2.GetY(); M(2,1) = V2.GetZ();
    M(0,2) = V3.GetX(); M(1,2) = V3.GetY(); M(2,2) = V3.GetZ();
    #endif
}

//==============================================================================

inline 
void matrix4::SetColumns( const vector3& V1, 
                          const vector3& V2, 
                          const vector3& V3 )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V1 );
    FORCE_ALIGNED_16( &V2 );
    FORCE_ALIGNED_16( &V3 );

#if USE_VU0
    asm( "vmove.xyz COL0, VEC1
          vmove.w   COL0, CIN0" : "=j COL0" (m_Col0) : "j VEC1" (V1.GetU128()), "j CIN0" (m_Col0) );
    asm( "vmove.xyz COL1, VEC2
          vmove.w   COL1, CIN1" : "=j COL1" (m_Col1) : "j VEC2" (V2.GetU128()), "j CIN1" (m_Col1) );
    asm( "vmove.xyz COL2, VEC3
          vmove.w   COL2, CIN2" : "=j COL2" (m_Col2) : "j VEC3" (V3.GetU128()), "j CIN2" (m_Col2) );
#else
    M(0,0) = V1.GetX(); M(1,0) = V2.GetX(); M(2,0) = V3.GetX();
    M(0,1) = V1.GetY(); M(1,1) = V2.GetY(); M(2,1) = V3.GetY();
    M(0,2) = V1.GetZ(); M(1,2) = V2.GetZ(); M(2,2) = V3.GetZ();
#endif
}

//==============================================================================

inline 
void matrix4::Orthogonalize( void )
{
    FORCE_ALIGNED_16( this );

#if UES_UV0
    vector3 VX( m_Col0 );
    vector3 VY( m_Col1 );
#else
    vector3 VX( M(0,0), M(0,1), M(0,2) );
    vector3 VY( M(1,0), M(1,1), M(1,2) );
#endif
    vector3 VZ;    

    VX.Normalize();
    VY.Normalize();

    VZ = v3_Cross( VX, VY );
    VY = v3_Cross( VZ, VX );

    SetColumns( VX, VY, VZ );
}

//==============================================================================

inline 
vector3 matrix4::operator * ( const vector3& V ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    u128 Ret;
    asm( "vmulaw.xyzw  acc,  COL3, vf00w
          vmaddaz.xyzw acc,  COL2, VINz
          vmadday.xyzw acc,  COL1, VINy
          vmaddx.xyzw  VOUT, COL0, VINx" :
         "=j VOUT" (Ret) :
         "j  COL0" (m_Col0),
         "j  COL1" (m_Col1),
         "j  COL2" (m_Col2),
         "j  COL3" (m_Col3),
         "j  VIN"  (V.GetU128()) );
    return vector3( Ret );
#else
    return( vector3( (M(0,0)*V.GetX()) + (M(1,0)*V.GetY()) + (M(2,0)*V.GetZ()) + M(3,0),
                     (M(0,1)*V.GetX()) + (M(1,1)*V.GetY()) + (M(2,1)*V.GetZ()) + M(3,1),
                     (M(0,2)*V.GetX()) + (M(1,2)*V.GetY()) + (M(2,2)*V.GetZ()) + M(3,2) ) );
#endif
}

//==============================================================================

inline 
vector4 matrix4::operator * ( const vector4& V ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    u128 Ret;
    asm( "vmulaw.xyzw  acc,  COL3, VINw
          vmaddaz.xyzw acc,  COL2, VINz
          vmadday.xyzw acc,  COL1, VINy
          vmaddx.xyzw  VOUT, COL0, VINx" :
         "=j VOUT" (Ret) :
         "j  COL0" (m_Col0),
         "j  COL1" (m_Col1),
         "j  COL2" (m_Col2),
         "j  COL3" (m_Col3),
         "j  VIN"  (V.GetU128()) );
    return vector4( Ret );
#else
    return( vector4( (M(0,0)*V.GetX()) + (M(1,0)*V.GetY()) + (M(2,0)*V.GetZ()) + (M(3,0)*V.GetW()),
                     (M(0,1)*V.GetX()) + (M(1,1)*V.GetY()) + (M(2,1)*V.GetZ()) + (M(3,1)*V.GetW()),
                     (M(0,2)*V.GetX()) + (M(1,2)*V.GetY()) + (M(2,2)*V.GetZ()) + (M(3,2)*V.GetW()),
                     (M(0,3)*V.GetX()) + (M(1,3)*V.GetY()) + (M(2,3)*V.GetZ()) + (M(3,3)*V.GetW())) );
#endif
}

//==============================================================================

inline
vector3 matrix4::Transform( const vector3& V ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

    return( (*this) * V );
}

//==============================================================================

inline
void matrix4::Transform(       vector3* pDest, 
                         const vector3* pSource, 
                               s32      NVerts ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( pDest );
    FORCE_ALIGNED_16( pSource );

    ASSERT( pDest   );
    ASSERT( pSource );

    while( NVerts > 0 )
    {
        *pDest = Transform( *pSource );

        pDest++;
        pSource++;
        NVerts--;
    }
}

//==============================================================================

inline
vector4 matrix4::Transform( const vector4& V ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

    return( (*this) * V );
}

//==============================================================================

inline
void matrix4::Transform(       vector4* pDest, 
                         const vector4* pSource, 
                               s32      NVerts ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( pDest );
    FORCE_ALIGNED_16( pSource );

    ASSERT( pDest   );
    ASSERT( pSource );

    while( NVerts > 0 )
    {
        *pDest = Transform( *pSource );

        pDest++;
        pSource++;
        NVerts--;
    }
}

//==============================================================================

// Rotates vector by matrix rotation
// (ignores matrix translation, but it will scale the vector if the matrix contains a scale)
inline
vector3 matrix4::RotateVector( const vector3& V ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    u128 OutVec;
    asm( "vmulaz.xyz  acc,  COL2, VINz
          vmadday.xyz acc,  COL1, VINy
          vmaddx.xyz  VOUT, COL0, VINx" :
         "=j VOUT" (OutVec) :
         "j  VIN"  (V.GetU128()),
         "j  COL0" (m_Col0),
         "j  COL1" (m_Col1),
         "j  COL2" (m_Col2) );
    return vector3(OutVec);
#else
    return( vector3( (M(0,0)*V.GetX()) + (M(1,0)*V.GetY()) + (M(2,0)*V.GetZ()),
                     (M(0,1)*V.GetX()) + (M(1,1)*V.GetY()) + (M(2,1)*V.GetZ()),
                     (M(0,2)*V.GetX()) + (M(1,2)*V.GetY()) + (M(2,2)*V.GetZ()) ) );
#endif
}

//==============================================================================

// Inverse rotates vector by matrix rotation
// (ignores matrix translation, but it will scale the vector if the matrix contains a scale)
inline
vector3 matrix4::InvRotateVector( const vector3& V ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

#if USE_VU0
    u128 Tmp0;
    u128 Tmp1;
    u128 Tmp2;
    u128 Tmp3;
    u128 Row0;
    u128 Row1;
    u128 Row2;
    u128 OutVec;
    asm( "pextlw %0, %1, %2" : "=r" (Tmp0) : "r" (m_Col1), "r" (m_Col0) );
    asm( "pextuw %0, %1, %2" : "=r" (Tmp1) : "r" (m_Col1), "r" (m_Col0) );
    asm( "pextlw %0, %1, %2" : "=r" (Tmp2) : "r" (m_Col3), "r" (m_Col2) );
    asm( "pextuw %0, %1, %2" : "=r" (Tmp3) : "r" (m_Col3), "r" (m_Col2) );
    asm( "pcpyld %0, %1, %2" : "=r" (Row0) : "r" (Tmp2),   "r" (Tmp0)   );
    asm( "pcpyud %0, %1, %2" : "=r" (Row1) : "r" (Tmp0),   "r" (Tmp2)   );
    asm( "pcpyld %0, %1, %2" : "=r" (Row2) : "r" (Tmp3),   "r" (Tmp1)   );
    asm( "vmulaz.xyz  acc,  ROW2, VINz
          vmadday.xyz acc,  ROW1, VINy
          vmaddx.xyz  VOUT, ROW0, VINx" :
         "=j VOUT" (OutVec) :
         "j  VIN"  (V.GetU128()),
         "j  ROW0" (Row0),
         "j  ROW1" (Row1),
         "j  ROW2" (Row2) );
    return vector3( OutVec );
#else
    return( vector3( (M(0,0)*V.GetX()) + (M(0,1)*V.GetY()) + (M(0,2)*V.GetZ()),
                     (M(1,0)*V.GetX()) + (M(1,1)*V.GetY()) + (M(1,2)*V.GetZ()),
                     (M(2,0)*V.GetX()) + (M(2,1)*V.GetY()) + (M(2,2)*V.GetZ()) ) );
#endif
}

//==============================================================================

inline
matrix4& matrix4::operator += ( const matrix4& aM )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &aM );

#if USE_VU0
    asm( "vadd.xyzw DST0, MT10, MT20" : "=j DST0" (m_Col0) : "j MT10" (m_Col0), "j MT20" (aM.m_Col0) );
    asm( "vadd.xyzw DST1, MT11, MT21" : "=j DST1" (m_Col1) : "j MT11" (m_Col1), "j MT21" (aM.m_Col1) );
    asm( "vadd.xyzw DST2, MT12, MT22" : "=j DST2" (m_Col2) : "j MT12" (m_Col2), "j MT22" (aM.m_Col2) );
    asm( "vadd.xyzw DST3, MT13, MT23" : "=j DST3" (m_Col3) : "j MT13" (m_Col3), "j MT23" (aM.m_Col3) );
#else
    M(0,0) += aM.M(0,0);
    M(0,1) += aM.M(0,1);
    M(0,2) += aM.M(0,2);
    M(0,3) += aM.M(0,3);

    M(1,0) += aM.M(1,0);
    M(1,1) += aM.M(1,1);
    M(1,2) += aM.M(1,2);
    M(1,3) += aM.M(1,3);

    M(2,0) += aM.M(2,0);
    M(2,1) += aM.M(2,1);
    M(2,2) += aM.M(2,2);
    M(2,3) += aM.M(2,3);

    M(3,0) += aM.M(3,0);
    M(3,1) += aM.M(3,1);
    M(3,2) += aM.M(3,2);
    M(3,3) += aM.M(3,3);
#endif

    return( *this );
}

//==============================================================================

inline
matrix4& matrix4::operator -= ( const matrix4& aM )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &aM );

#if USE_VU0
    asm( "vsub.xyzw DST0, MT10, MT20" : "=j DST0" (m_Col0) : "j MT10" (m_Col0), "j MT20" (aM.m_Col0) );
    asm( "vsub.xyzw DST1, MT11, MT21" : "=j DST1" (m_Col1) : "j MT11" (m_Col1), "j MT21" (aM.m_Col1) );
    asm( "vsub.xyzw DST2, MT12, MT22" : "=j DST2" (m_Col2) : "j MT12" (m_Col2), "j MT22" (aM.m_Col2) );
    asm( "vsub.xyzw DST3, MT13, MT23" : "=j DST3" (m_Col3) : "j MT13" (m_Col3), "j MT23" (aM.m_Col3) );
#else
    M(0,0) -= aM.M(0,0);
    M(0,1) -= aM.M(0,1);
    M(0,2) -= aM.M(0,2);
    M(0,3) -= aM.M(0,3);

    M(1,0) -= aM.M(1,0);
    M(1,1) -= aM.M(1,1);
    M(1,2) -= aM.M(1,2);
    M(1,3) -= aM.M(1,3);

    M(2,0) -= aM.M(2,0);
    M(2,1) -= aM.M(2,1);
    M(2,2) -= aM.M(2,2);
    M(2,3) -= aM.M(2,3);

    M(3,0) -= aM.M(3,0);
    M(3,1) -= aM.M(3,1);
    M(3,2) -= aM.M(3,2);
    M(3,3) -= aM.M(3,3);
#endif

    return( *this );
}

//==============================================================================

inline
matrix4& matrix4::operator *= ( const matrix4& aM )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &aM );

    *this = (*this) * aM;
    return( *this );
}

//==============================================================================

inline
matrix4 operator + ( const matrix4& M1, const matrix4& M2 )
{
    FORCE_ALIGNED_16( &M1 );
    FORCE_ALIGNED_16( &M2 );

    matrix4 Result;

#if USE_VU0
    asm( "vadd.xyzw DST0, MT10, MT20" : "=j DST0" (Result.m_Col0) : "j MT10" (M1.m_Col0), "j MT20" (M2.m_Col0) );
    asm( "vadd.xyzw DST1, MT11, MT21" : "=j DST1" (Result.m_Col1) : "j MT11" (M1.m_Col1), "j MT21" (M2.m_Col1) );
    asm( "vadd.xyzw DST2, MT12, MT22" : "=j DST2" (Result.m_Col2) : "j MT12" (M1.m_Col2), "j MT22" (M2.m_Col2) );
    asm( "vadd.xyzw DST3, MT13, MT23" : "=j DST3" (Result.m_Col3) : "j MT13" (M1.m_Col3), "j MT23" (M2.m_Col3) );
#else
    Result.M(0,0) = M1.M(0,0) + M2.M(0,0);
    Result.M(0,1) = M1.M(0,1) + M2.M(0,1);
    Result.M(0,2) = M1.M(0,2) + M2.M(0,2);
    Result.M(0,3) = M1.M(0,3) + M2.M(0,3);
    
    Result.M(1,0) = M1.M(1,0) + M2.M(1,0);
    Result.M(1,1) = M1.M(1,1) + M2.M(1,1);
    Result.M(1,2) = M1.M(1,2) + M2.M(1,2);
    Result.M(1,3) = M1.M(1,3) + M2.M(1,3);
    
    Result.M(2,0) = M1.M(2,0) + M2.M(2,0);
    Result.M(2,1) = M1.M(2,1) + M2.M(2,1);
    Result.M(2,2) = M1.M(2,2) + M2.M(2,2);
    Result.M(2,3) = M1.M(2,3) + M2.M(2,3);
    
    Result.M(3,0) = M1.M(3,0) + M2.M(3,0);
    Result.M(3,1) = M1.M(3,1) + M2.M(3,1);
    Result.M(3,2) = M1.M(3,2) + M2.M(3,2);
    Result.M(3,3) = M1.M(3,3) + M2.M(3,3);
#endif

    return( Result );
}

//==============================================================================

inline
matrix4 operator - ( const matrix4& M1, const matrix4& M2 )
{
    FORCE_ALIGNED_16( &M1 );
    FORCE_ALIGNED_16( &M2 );

    matrix4 Result;

#if USE_VU0
    asm( "vsub.xyzw DST0, MT10, MT20" : "=j DST0" (Result.m_Col0) : "j MT10" (M1.m_Col0), "j MT20" (M2.m_Col0) );
    asm( "vsub.xyzw DST1, MT11, MT21" : "=j DST1" (Result.m_Col1) : "j MT11" (M1.m_Col1), "j MT21" (M2.m_Col1) );
    asm( "vsub.xyzw DST2, MT12, MT22" : "=j DST2" (Result.m_Col2) : "j MT12" (M1.m_Col2), "j MT22" (M2.m_Col2) );
    asm( "vsub.xyzw DST3, MT13, MT23" : "=j DST3" (Result.m_Col3) : "j MT13" (M1.m_Col3), "j MT23" (M2.m_Col3) );
#else

    Result.M(0,0) = M1.M(0,0) - M2.M(0,0);
    Result.M(0,1) = M1.M(0,1) - M2.M(0,1);
    Result.M(0,2) = M1.M(0,2) - M2.M(0,2);
    Result.M(0,3) = M1.M(0,3) - M2.M(0,3);
    
    Result.M(1,0) = M1.M(1,0) - M2.M(1,0);
    Result.M(1,1) = M1.M(1,1) - M2.M(1,1);
    Result.M(1,2) = M1.M(1,2) - M2.M(1,2);
    Result.M(1,3) = M1.M(1,3) - M2.M(1,3);
    
    Result.M(2,0) = M1.M(2,0) - M2.M(2,0);
    Result.M(2,1) = M1.M(2,1) - M2.M(2,1);
    Result.M(2,2) = M1.M(2,2) - M2.M(2,2);
    Result.M(2,3) = M1.M(2,3) - M2.M(2,3);
    
    Result.M(3,0) = M1.M(3,0) - M2.M(3,0);
    Result.M(3,1) = M1.M(3,1) - M2.M(3,1);
    Result.M(3,2) = M1.M(3,2) - M2.M(3,2);
    Result.M(3,3) = M1.M(3,3) - M2.M(3,3);
#endif

    return( Result );
}

//==============================================================================

inline
f32 matrix4::Difference( const matrix4& M ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &M );

#if USE_VU0
    u128 Tmp0;
    u128 Tmp1;
    u128 Tmp2;
    u128 Tmp3;
    f32  Diff;
    asm( "vsub.xyzw TMP0, MT10, MT20"  : "=j TMP0" (Tmp0) : "j MT10" (m_Col0), "j MT20" (M.m_Col0) );
    asm( "vsub.xyzw TMP1, MT11, MT21"  : "=j TMP1" (Tmp1) : "j MT11" (m_Col1), "j MT21" (M.m_Col1) );
    asm( "vsub.xyzw TMP2, MT12, MT22"  : "=j TMP2" (Tmp2) : "j MT12" (m_Col2), "j MT22" (M.m_Col2) );
    asm( "vsub.xyzw TMP3, MT13, MT23"  : "=j TMP3" (Tmp3) : "j MT13" (m_Col3), "j MT23" (M.m_Col3) );
    asm( "vmul.xyzw TMP0, VIN,  VIN"   : "=j TMP0" (Tmp0) : "j VIN"  (Tmp0) );
    asm( "vmul.xyzw TMP1, VIN,  VIN"   : "=j TMP1" (Tmp1) : "j VIN"  (Tmp1) );
    asm( "vmul.xyzw TMP2, VIN,  VIN"   : "=j TMP2" (Tmp2) : "j VIN"  (Tmp2) );
    asm( "vmul.xyzw TMP3, VIN,  VIN"   : "=j TMP3" (Tmp3) : "j VIN"  (Tmp3) );
    asm( "vadd.xyzw TMP0, VIN0, VIN1"  : "=j TMP0" (Tmp0) : "j VIN0" (Tmp0), "j VIN1" (Tmp1) );
    asm( "vadd.xyzw TMP2, VIN2, VIN3"  : "=j TMP2" (Tmp2) : "j VIN2" (Tmp2), "j VIN3" (Tmp3) );
    asm( "vadd.xyzw TMP0, VIN0, VIN2"  : "=j TMP0" (Tmp0) : "j VIN0" (Tmp0), "j VIN2" (Tmp2) );
    asm( "vaddw.x   TMP0, VIN0, VIN0w
          vaddz.x   TMP0, TMP0, VIN0z
          vaddy.x   TMP0, TMP0, VIN0y" : "=j TMP0" (Tmp0) : "j VIN0" (Tmp0) );
    asm( "qmfc2     %0,   %1"          : "=r"      (Diff) : "j"      (Tmp0) );
    return Diff;
#else
    f32 Diff=0.0f;
    for( s32 i=0; i<16; i++ )
        Diff += (((f32*)this)[i] - ((f32*)(&M))[i])*
                (((f32*)this)[i] - ((f32*)(&M))[i]);
    return Diff;
#endif
}

//==============================================================================

inline
xbool matrix4::IsIdentity( void ) const
{
    FORCE_ALIGNED_16( this );

    matrix4 I;
    I.Identity();
    return (this->Difference(I) < 0.0000001f);
}

//==============================================================================

inline
xbool matrix4::InRange( f32 Min, f32 Max ) const
{
    FORCE_ALIGNED_16( this );

    for( s32 i=0; i<16; i++ )
    {
        if( (((f32*)this)[i] < Min) || (((f32*)this)[i] > Max) )
            return FALSE;
    }

    return TRUE;
}

//==============================================================================

inline
xbool matrix4::IsValid( void ) const
{
    FORCE_ALIGNED_16( this );

    for( s32 i=0; i<16; i++ )
    {
        if( !x_isvalid( ((f32*)this)[i] ) )
            return FALSE;
    }

    return TRUE;
}

//==============================================================================

inline
void matrix4::SetRotation( const radian3& Rotation )
{
    FORCE_ALIGNED_16( this );

    f32 sx, cx;
    f32 sy, cy;
    f32 sz, cz;
    f32 sxsz;
    f32 sxcz;

    x_sincos( Rotation.Pitch, sx, cx );
    x_sincos( Rotation.Yaw,   sy, cy );
    x_sincos( Rotation.Roll,  sz, cz );

    sxsz = sx * sz;
    sxcz = sx * cz;

    // Fill out 3x3 rotations.

    M(0,0) = cy*cz + sy*sxsz;   M(0,1) = cx*sz;   M(0,2) = cy*sxsz - sy*cz;
    M(1,0) = sy*sxcz - sz*cy;   M(1,1) = cx*cz;   M(1,2) = sy*sz + sxcz*cy;
    M(2,0) = cx*sy;             M(2,1) = -sx;     M(2,2) = cx*cy;
}

//==============================================================================

inline
void matrix4::SetRotation( const quaternion& Q )
{
    FORCE_ALIGNED_16( this );

    f32 tx  = 2.0f * Q.X;   // 2x
    f32 ty  = 2.0f * Q.Y;   // 2y
    f32 tz  = 2.0f * Q.Z;   // 2z
    f32 txw =   tx * Q.W;   // 2x * w
    f32 tyw =   ty * Q.W;   // 2y * w
    f32 tzw =   tz * Q.W;   // 2z * w
    f32 txx =   tx * Q.X;   // 2x * x
    f32 tyx =   ty * Q.X;   // 2y * x
    f32 tzx =   tz * Q.X;   // 2z * x
    f32 tyy =   ty * Q.Y;   // 2y * y
    f32 tzy =   tz * Q.Y;   // 2z * y
    f32 tzz =   tz * Q.Z;   // 2z * z
                                
    // Fill out 3x3 rotations.

    M(0,0) = 1.0f-(tyy+tzz); M(0,1) = tyx + tzw;      M(0,2) = tzx - tyw;           
    M(1,0) = tyx - tzw;      M(1,1) = 1.0f-(txx+tzz); M(1,2) = tzy + txw;           
    M(2,0) = tzx + tyw;      M(2,1) = tzy - txw;      M(2,2) = 1.0f-(txx+tyy);    
}

//==============================================================================

inline
void matrix4::Setup( const radian3& Rotation )
{                           
    FORCE_ALIGNED_16( this );

    SetRotation( Rotation );

    M(0,3) = 0.0f;
    M(1,3) = 0.0f;
    M(2,3) = 0.0f;
    M(3,3) = 1.0f;
    M(3,2) = 0.0f;
    M(3,1) = 0.0f;
    M(3,0) = 0.0f;
}

//==============================================================================

inline
void matrix4::Setup( const quaternion& Q )
{                           
    FORCE_ALIGNED_16( this );

    SetRotation( Q );

    M(0,3) = 0.0f;
    M(1,3) = 0.0f;
    M(2,3) = 0.0f;
    M(3,3) = 1.0f;
    M(3,2) = 0.0f;
    M(3,1) = 0.0f;
    M(3,0) = 0.0f;
}

//==============================================================================

inline
void matrix4::Setup( const vector3& Scale, 
                     const radian3& Rotation, 
                     const vector3& Translation )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Scale );
    FORCE_ALIGNED_16( &Translation );

    /*
    Identity   ( );
    SetRotation( Rotation );
    PreScale   ( Scale );
    Translate  ( Translation );
    */

    f32 sx, cx;
    f32 sy, cy;
    f32 sz, cz;
    f32 sxsz;
    f32 sxcz;

    x_sincos( Rotation.Pitch, sx, cx );
    x_sincos( Rotation.Yaw,   sy, cy );
    x_sincos( Rotation.Roll,  sz, cz );

    sxsz = sx * sz;
    sxcz = sx * cz;
                                
    // Fill out 3x3 rotations.
    M(0,0) = (cy*cz + sy*sxsz)  * Scale.GetX();
    M(0,1) = (cx*sz)            * Scale.GetX();
    M(0,2) = (cy*sxsz - sy*cz)  * Scale.GetX();
    M(1,0) = (sy*sxcz - sz*cy)  * Scale.GetY();
    M(1,1) = (cx*cz)            * Scale.GetY();
    M(1,2) = (sy*sz + sxcz*cy)  * Scale.GetY();
    M(2,0) = (cx*sy)            * Scale.GetZ();
    M(2,1) = (-sx)              * Scale.GetZ();
    M(2,2) = (cx*cy)            * Scale.GetZ();

    // Translate
    M(3,0) = Translation.GetX();
    M(3,1) = Translation.GetY();
    M(3,2) = Translation.GetZ();

    // Clear bottom row
    M(0,3) = 0.0f;
    M(1,3) = 0.0f;
    M(2,3) = 0.0f;
    M(3,3) = 1.0f;
}

//==============================================================================

inline
void matrix4::Setup( const vector3& Scale, 
                     const quaternion& Rotation,
                     const vector3& Translation )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Scale );
    FORCE_ALIGNED_16( &Translation );

    /*
    Identity   ( );
    SetRotation( Rotation );
    PreScale   ( Scale );
    Translate  ( Translation );
    */

    f32 tx  = 2.0f * Rotation.X;   // 2x
    f32 ty  = 2.0f * Rotation.Y;   // 2y
    f32 tz  = 2.0f * Rotation.Z;   // 2z
    f32 txw =   tx * Rotation.W;   // 2x * w
    f32 tyw =   ty * Rotation.W;   // 2y * w
    f32 tzw =   tz * Rotation.W;   // 2z * w
    f32 txx =   tx * Rotation.X;   // 2x * x
    f32 tyx =   ty * Rotation.X;   // 2y * x
    f32 tzx =   tz * Rotation.X;   // 2z * x
    f32 tyy =   ty * Rotation.Y;   // 2y * y
    f32 tzy =   tz * Rotation.Y;   // 2z * y
    f32 tzz =   tz * Rotation.Z;   // 2z * z
                                
    // Fill out 3x3 rotations.
    M(0,0) = (1.0f-(tyy+tzz))   * Scale.GetX();
    M(0,1) = (tyx + tzw)        * Scale.GetX();
    M(0,2) = (tzx - tyw)        * Scale.GetX();
    M(1,0) = (tyx - tzw)        * Scale.GetY();
    M(1,1) = (1.0f-(txx+tzz))   * Scale.GetY();
    M(1,2) = (tzy + txw)        * Scale.GetY();
    M(2,0) = (tzx + tyw)        * Scale.GetZ();
    M(2,1) = (tzy - txw)        * Scale.GetZ();
    M(2,2) = (1.0f-(txx+tyy))   * Scale.GetZ();

    // Translate
    M(3,0) = Translation.GetX();
    M(3,1) = Translation.GetY();
    M(3,2) = Translation.GetZ();

    // Clear bottom row
    M(0,3) = 0.0f;
    M(1,3) = 0.0f;
    M(2,3) = 0.0f;
    M(3,3) = 1.0f;

}

//==============================================================================

inline
void matrix4::Setup( const vector3& Axis, radian Angle )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Axis );

    Setup( quaternion( Axis, Angle ) );
}

//==============================================================================

inline 
void matrix4::Setup( const vector3& V1, 
                     const vector3& V2, 
                           radian   Angle )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V1 );
    FORCE_ALIGNED_16( &V2 );

    Setup( V2 - V1, Angle );

    PreTranslate( -V1 );
    Translate   (  V1 );
}

//==============================================================================

inline
matrix4         m4_Transpose        ( const matrix4& M )
{
    FORCE_ALIGNED_16( &M );

    matrix4 Mout = M;
    Mout.Transpose();
    return Mout;
}

//==============================================================================

inline 
matrix4::matrix4( void )
{
    FORCE_ALIGNED_16( this );
}

//==============================================================================

inline 
matrix4::matrix4( const radian3& R )
{   
    FORCE_ALIGNED_16( this );

    Setup( R );
}

//==============================================================================

inline 
matrix4::matrix4( const quaternion& Q )
{   
    FORCE_ALIGNED_16( this );

    Setup( Q );
}

//==============================================================================

inline 
matrix4::matrix4( const vector3& Scale,
                  const radian3& Rotation,
                  const vector3& Translation )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Scale );
    FORCE_ALIGNED_16( &Translation );

    Setup( Scale, Rotation, Translation );
}

//==============================================================================

inline 
matrix4::matrix4( const vector3& Scale,
                  const quaternion& Rotation,
                  const vector3& Translation )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Scale );
    FORCE_ALIGNED_16( &Translation );

    Setup( Scale, Rotation, Translation );
}

//==============================================================================

inline 
void matrix4::Rotate( const radian3& Rotation )
{
    FORCE_ALIGNED_16( this );

    matrix4 RM( Rotation );
    *this = RM * *this;    
}

//==============================================================================

inline 
void matrix4::Rotate( const quaternion& Q )
{
    FORCE_ALIGNED_16( this );

    matrix4 RM( Q );
    *this = RM * *this;    
}

//==============================================================================

inline 
void matrix4::PreRotate( const radian3& Rotation )
{
    FORCE_ALIGNED_16( this );

    matrix4 RM( Rotation );
    *this *= RM;    
}

//==============================================================================

inline 
void matrix4::PreRotate( const quaternion& Q )
{
    FORCE_ALIGNED_16( this );

    matrix4 RM( Q );
    *this *= RM;    
}

//==============================================================================

//==============================================================================
//  CLEAR DEFINES
//==============================================================================

#undef M

//==============================================================================
