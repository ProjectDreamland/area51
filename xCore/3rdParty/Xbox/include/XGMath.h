/*==========================================================================;
 *
 *  Copyright (C) Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       xgmath.h
 *
 ****************************************************************************/
 
#ifndef __XGMATH_H__
#define __XGMATH_H__

#ifndef XGINLINE
#ifdef __cplusplus
#define XGINLINE inline
#else
#define XGINLINE _inline
#endif
#endif

#ifdef __cplusplus
extern "C++" {
#endif // __cplusplus

#include <math.h>

#pragma warning(disable:4201) // anonymous unions warning

//===========================================================================
//
// General purpose utilities
//
//===========================================================================
#define XG_PI    ((FLOAT)  3.141592654f)
#define XG_1BYPI ((FLOAT)  0.318309886f)

#define XGToRadian( degree ) ((degree) * (XG_PI / 180.0f))
#define XGToDegree( radian ) ((radian) * (180.0f / XG_PI))

//===========================================================================
//
// Vectors
//
//===========================================================================

//--------------------------
// 2D Vector
//--------------------------
typedef struct XGVECTOR2
{
#ifdef __cplusplus
public:
    XGVECTOR2() {};
    XGVECTOR2( CONST FLOAT * );
    XGVECTOR2( FLOAT x, FLOAT y );

    // casting
    operator FLOAT* ();
    operator CONST FLOAT* () const;

    // assignment operators
    XGVECTOR2& operator += ( CONST XGVECTOR2& );
    XGVECTOR2& operator -= ( CONST XGVECTOR2& );
    XGVECTOR2& operator *= ( FLOAT );
    XGVECTOR2& operator /= ( FLOAT );

    // unary operators
    XGVECTOR2 operator + () const;
    XGVECTOR2 operator - () const;

    // binary operators
    XGVECTOR2 operator + ( CONST XGVECTOR2& ) const;
    XGVECTOR2 operator - ( CONST XGVECTOR2& ) const;
    XGVECTOR2 operator * ( FLOAT ) const;
    XGVECTOR2 operator / ( FLOAT ) const;

    friend XGVECTOR2 operator * ( FLOAT, CONST XGVECTOR2& );

    BOOL operator == ( CONST XGVECTOR2& ) const;
    BOOL operator != ( CONST XGVECTOR2& ) const;


public:
#endif //__cplusplus
    FLOAT x, y;
} XGVECTOR2, *LPXGVECTOR2;


//--------------------------
// 3D Vector
//--------------------------
#ifdef __cplusplus
typedef struct XGVECTOR3 : public D3DVECTOR
{
public:
    XGVECTOR3() {};
    XGVECTOR3( CONST FLOAT * );
    XGVECTOR3( CONST D3DVECTOR& );
    XGVECTOR3( FLOAT x, FLOAT y, FLOAT z );

    // casting
    operator FLOAT* ();
    operator CONST FLOAT* () const;

    // assignment operators
    XGVECTOR3& operator += ( CONST XGVECTOR3& );
    XGVECTOR3& operator -= ( CONST XGVECTOR3& );
    XGVECTOR3& operator *= ( FLOAT );
    XGVECTOR3& operator /= ( FLOAT );

    // unary operators
    XGVECTOR3 operator + () const;
    XGVECTOR3 operator - () const;

    // binary operators
    XGVECTOR3 operator + ( CONST XGVECTOR3& ) const;
    XGVECTOR3 operator - ( CONST XGVECTOR3& ) const;
    XGVECTOR3 operator * ( FLOAT ) const;
    XGVECTOR3 operator / ( FLOAT ) const;

    friend XGVECTOR3 operator * ( FLOAT, CONST struct XGVECTOR3& );

    BOOL operator == ( CONST XGVECTOR3& ) const;
    BOOL operator != ( CONST XGVECTOR3& ) const;

} XGVECTOR3, *LPXGVECTOR3;

#else //!__cplusplus
typedef struct _D3DVECTOR XGVECTOR3, *LPXGVECTOR3;
#endif //!__cplusplus


//--------------------------
// 4D Vector
//--------------------------
typedef struct XGVECTOR4
{
#ifdef __cplusplus
public:
    XGVECTOR4() {};
    XGVECTOR4( CONST FLOAT* );
    XGVECTOR4( FLOAT x, FLOAT y, FLOAT z, FLOAT w );

    // casting
    operator FLOAT* ();
    operator CONST FLOAT* () const;

    // assignment operators
    XGVECTOR4& operator += ( CONST XGVECTOR4& );
    XGVECTOR4& operator -= ( CONST XGVECTOR4& );
    XGVECTOR4& operator *= ( FLOAT );
    XGVECTOR4& operator /= ( FLOAT );

    // unary operators
    XGVECTOR4 operator + () const;
    XGVECTOR4 operator - () const;

    // binary operators
    XGVECTOR4 operator + ( CONST XGVECTOR4& ) const;
    XGVECTOR4 operator - ( CONST XGVECTOR4& ) const;
    XGVECTOR4 operator * ( FLOAT ) const;
    XGVECTOR4 operator / ( FLOAT ) const;

    friend XGVECTOR4 operator * ( FLOAT, CONST XGVECTOR4& );

    BOOL operator == ( CONST XGVECTOR4& ) const;
    BOOL operator != ( CONST XGVECTOR4& ) const;

public:
#endif //__cplusplus
    FLOAT x, y, z, w;
} XGVECTOR4, *LPXGVECTOR4;


//===========================================================================
//
// Matrices
//
//===========================================================================
#ifdef __cplusplus
typedef __declspec(align(16)) struct XGMATRIX : public D3DMATRIX
{
public:

    XGMATRIX() {};
    XGMATRIX( CONST FLOAT * );
    XGMATRIX( CONST D3DMATRIX& );
    XGMATRIX( FLOAT _11, FLOAT _12, FLOAT _13, FLOAT _14,
                FLOAT _21, FLOAT _22, FLOAT _23, FLOAT _24,
                FLOAT _31, FLOAT _32, FLOAT _33, FLOAT _34,
                FLOAT _41, FLOAT _42, FLOAT _43, FLOAT _44 );

    // access grants
    FLOAT& operator () ( UINT Row, UINT Col );
    FLOAT  operator () ( UINT Row, UINT Col ) const;

    // casting operators
    operator FLOAT* ();
    operator CONST FLOAT* () const;

    // assignment operators
    XGMATRIX& operator *= ( CONST XGMATRIX& );
    XGMATRIX& operator += ( CONST XGMATRIX& );
    XGMATRIX& operator -= ( CONST XGMATRIX& );
    XGMATRIX& operator *= ( FLOAT );
    XGMATRIX& operator /= ( FLOAT );

    // unary operators
    XGMATRIX operator + () const;
    XGMATRIX operator - () const;

    // binary operators
    XGMATRIX operator * ( CONST XGMATRIX& ) const;
    XGMATRIX operator + ( CONST XGMATRIX& ) const;
    XGMATRIX operator - ( CONST XGMATRIX& ) const;
    XGMATRIX operator * ( FLOAT ) const;
    XGMATRIX operator / ( FLOAT ) const;

    friend XGMATRIX operator * ( FLOAT, CONST XGMATRIX& );

    BOOL operator == ( CONST XGMATRIX& ) const;
    BOOL operator != ( CONST XGMATRIX& ) const;

} XGMATRIX;

#else //!__cplusplus
typedef __declspec(align(16)) struct _D3DMATRIX XGMATRIX;
#endif //!__cplusplus

typedef struct XGMATRIX *LPXGMATRIX;

//===========================================================================
//
//    Quaternions
//
//===========================================================================
typedef struct XGQUATERNION
{
#ifdef __cplusplus
public:
    XGQUATERNION() {}
    XGQUATERNION( CONST FLOAT * );
    XGQUATERNION( FLOAT x, FLOAT y, FLOAT z, FLOAT w );

    // casting
    operator FLOAT* ();
    operator CONST FLOAT* () const;

    // assignment operators
    XGQUATERNION& operator += ( CONST XGQUATERNION& );
    XGQUATERNION& operator -= ( CONST XGQUATERNION& );
    XGQUATERNION& operator *= ( CONST XGQUATERNION& );
    XGQUATERNION& operator *= ( FLOAT );
    XGQUATERNION& operator /= ( FLOAT );

    // unary operators
    XGQUATERNION  operator + () const;
    XGQUATERNION  operator - () const;

    // binary operators
    XGQUATERNION operator + ( CONST XGQUATERNION& ) const;
    XGQUATERNION operator - ( CONST XGQUATERNION& ) const;
    XGQUATERNION operator * ( CONST XGQUATERNION& ) const;
    XGQUATERNION operator * ( FLOAT ) const;
    XGQUATERNION operator / ( FLOAT ) const;

    friend XGQUATERNION operator * (FLOAT, CONST XGQUATERNION& );

    BOOL operator == ( CONST XGQUATERNION& ) const;
    BOOL operator != ( CONST XGQUATERNION& ) const;

#endif //__cplusplus
    FLOAT x, y, z, w;
} XGQUATERNION, *LPXGQUATERNION;


//===========================================================================
//
// Planes
//
//===========================================================================
typedef struct XGPLANE
{
#ifdef __cplusplus
public:
    XGPLANE() {}
    XGPLANE( CONST FLOAT* );
    XGPLANE( FLOAT a, FLOAT b, FLOAT c, FLOAT d );

    // casting
    operator FLOAT* ();
    operator CONST FLOAT* () const;

    // unary operators
    XGPLANE operator + () const;
    XGPLANE operator - () const;

    // binary operators
    BOOL operator == ( CONST XGPLANE& ) const;
    BOOL operator != ( CONST XGPLANE& ) const;

#endif //__cplusplus
    FLOAT a, b, c, d;
} XGPLANE, *LPXGPLANE;


//===========================================================================
//
// Colors
//
//===========================================================================

typedef struct XGCOLOR
{
#ifdef __cplusplus
public:
    XGCOLOR() {}
    XGCOLOR( DWORD argb );
    XGCOLOR( CONST FLOAT * );
    XGCOLOR( CONST D3DCOLORVALUE& );
    XGCOLOR( FLOAT r, FLOAT g, FLOAT b, FLOAT a );

    // casting
    operator DWORD () const;

    operator FLOAT* ();
    operator CONST FLOAT* () const;

    operator D3DCOLORVALUE* ();
    operator CONST D3DCOLORVALUE* () const;

    operator D3DCOLORVALUE& ();
    operator CONST D3DCOLORVALUE& () const;

    // assignment operators
    XGCOLOR& operator += ( CONST XGCOLOR& );
    XGCOLOR& operator -= ( CONST XGCOLOR& );
    XGCOLOR& operator *= ( FLOAT );
    XGCOLOR& operator /= ( FLOAT );

    // unary operators
    XGCOLOR operator + () const;
    XGCOLOR operator - () const;

    // binary operators
    XGCOLOR operator + ( CONST XGCOLOR& ) const;
    XGCOLOR operator - ( CONST XGCOLOR& ) const;
    XGCOLOR operator * ( FLOAT ) const;
    XGCOLOR operator / ( FLOAT ) const;

    friend XGCOLOR operator * (FLOAT, CONST XGCOLOR& );

    BOOL operator == ( CONST XGCOLOR& ) const;
    BOOL operator != ( CONST XGCOLOR& ) const;

#endif //__cplusplus
    FLOAT r, g, b, a;
} XGCOLOR, *LPXGCOLOR;


//===========================================================================
//
// XGraphics math functions:
//
// NOTE:
//  * All these functions can take the same object as in and out parameters.
//
//  * Out parameters are typically also returned as return values, so that
//    the output of one function may be used as a parameter to another.
//
//===========================================================================

//--------------------------
// 2D Vector
//--------------------------

// inline

FLOAT XGVec2Length
    ( CONST XGVECTOR2 *pV );

FLOAT XGVec2LengthSq
    ( CONST XGVECTOR2 *pV );

FLOAT XGVec2Dot
    ( CONST XGVECTOR2 *pV1, CONST XGVECTOR2 *pV2 );

// Z component of ((x1,y1,0) cross (x2,y2,0))
FLOAT XGVec2CCW
    ( CONST XGVECTOR2 *pV1, CONST XGVECTOR2 *pV2 );

XGVECTOR2* XGVec2Add
    ( XGVECTOR2 *pOut, CONST XGVECTOR2 *pV1, CONST XGVECTOR2 *pV2 );

XGVECTOR2* XGVec2Subtract
    ( XGVECTOR2 *pOut, CONST XGVECTOR2 *pV1, CONST XGVECTOR2 *pV2 );

// Minimize each component.  x = min(x1, x2), y = min(y1, y2)
XGVECTOR2* XGVec2Minimize
    ( XGVECTOR2 *pOut, CONST XGVECTOR2 *pV1, CONST XGVECTOR2 *pV2 );

// Maximize each component.  x = max(x1, x2), y = max(y1, y2)
XGVECTOR2* XGVec2Maximize
    ( XGVECTOR2 *pOut, CONST XGVECTOR2 *pV1, CONST XGVECTOR2 *pV2 );

XGVECTOR2* XGVec2Scale
    ( XGVECTOR2 *pOut, CONST XGVECTOR2 *pV, FLOAT s );

// Linear interpolation. V1 + s(V2-V1)
XGVECTOR2* XGVec2Lerp
    ( XGVECTOR2 *pOut, CONST XGVECTOR2 *pV1, CONST XGVECTOR2 *pV2,
      FLOAT s );

// non-inline
#ifdef __cplusplus
extern "C" {
#endif

XGVECTOR2* WINAPI XGVec2Normalize
    ( XGVECTOR2 *pOut, CONST XGVECTOR2 *pV );

// Hermite interpolation between position V1, tangent T1 (when s == 0)
// and position V2, tangent T2 (when s == 1).
XGVECTOR2* WINAPI XGVec2Hermite
    ( XGVECTOR2 *pOut, CONST XGVECTOR2 *pV1, CONST XGVECTOR2 *pT1,
      CONST XGVECTOR2 *pV2, CONST XGVECTOR2 *pT2, FLOAT s );

// CatmullRom interpolation between V1 (when s == 0) and V2 (when s == 1)
XGVECTOR2* WINAPI XGVec2CatmullRom
    ( XGVECTOR2 *pOut, CONST XGVECTOR2 *pV0, CONST XGVECTOR2 *pV1,
      CONST XGVECTOR2 *pV2, CONST XGVECTOR2 *pV3, FLOAT s );

// Barycentric coordinates.  V1 + f(V2-V1) + g(V3-V1)
XGVECTOR2* WINAPI XGVec2BaryCentric
    ( XGVECTOR2 *pOut, CONST XGVECTOR2 *pV1, CONST XGVECTOR2 *pV2,
      XGVECTOR2 *pV3, FLOAT f, FLOAT g);

// Transform (x, y, 0, 1) by matrix.
XGVECTOR4* WINAPI XGVec2Transform
    ( XGVECTOR4 *pOut, CONST XGVECTOR2 *pV, CONST XGMATRIX *pM );

// Transform (x, y, 0, 1) by matrix, project result back into w=1.
XGVECTOR2* WINAPI XGVec2TransformCoord
    ( XGVECTOR2 *pOut, CONST XGVECTOR2 *pV, CONST XGMATRIX *pM );

// Transform (x, y, 0, 0) by matrix.
XGVECTOR2* WINAPI XGVec2TransformNormal
    ( XGVECTOR2 *pOut, CONST XGVECTOR2 *pV, CONST XGMATRIX *pM );

#ifdef __cplusplus
}
#endif


//--------------------------
// 3D Vector
//--------------------------

// inline

FLOAT XGVec3Length
    ( CONST XGVECTOR3 *pV );

FLOAT XGVec3LengthSq
    ( CONST XGVECTOR3 *pV );

FLOAT XGVec3Dot
    ( CONST XGVECTOR3 *pV1, CONST XGVECTOR3 *pV2 );

XGVECTOR3* XGVec3Cross
    ( XGVECTOR3 *pOut, CONST XGVECTOR3 *pV1, CONST XGVECTOR3 *pV2 );

XGVECTOR3* XGVec3Add
    ( XGVECTOR3 *pOut, CONST XGVECTOR3 *pV1, CONST XGVECTOR3 *pV2 );

XGVECTOR3* XGVec3Subtract
    ( XGVECTOR3 *pOut, CONST XGVECTOR3 *pV1, CONST XGVECTOR3 *pV2 );

// Minimize each component.  x = min(x1, x2), y = min(y1, y2), ...
XGVECTOR3* XGVec3Minimize
    ( XGVECTOR3 *pOut, CONST XGVECTOR3 *pV1, CONST XGVECTOR3 *pV2 );

// Maximize each component.  x = max(x1, x2), y = max(y1, y2), ...
XGVECTOR3* XGVec3Maximize
    ( XGVECTOR3 *pOut, CONST XGVECTOR3 *pV1, CONST XGVECTOR3 *pV2 );

XGVECTOR3* XGVec3Scale
    ( XGVECTOR3 *pOut, CONST XGVECTOR3 *pV, FLOAT s);

// Linear interpolation. V1 + s(V2-V1)
XGVECTOR3* XGVec3Lerp
    ( XGVECTOR3 *pOut, CONST XGVECTOR3 *pV1, CONST XGVECTOR3 *pV2,
      FLOAT s );

// non-inline
#ifdef __cplusplus
extern "C" {
#endif

XGVECTOR3* WINAPI XGVec3Normalize
    ( XGVECTOR3 *pOut, CONST XGVECTOR3 *pV );

// Hermite interpolation between position V1, tangent T1 (when s == 0)
// and position V2, tangent T2 (when s == 1).
XGVECTOR3* WINAPI XGVec3Hermite
    ( XGVECTOR3 *pOut, CONST XGVECTOR3 *pV1, CONST XGVECTOR3 *pT1,
      CONST XGVECTOR3 *pV2, CONST XGVECTOR3 *pT2, FLOAT s );

// CatmullRom interpolation between V1 (when s == 0) and V2 (when s == 1)
XGVECTOR3* WINAPI XGVec3CatmullRom
    ( XGVECTOR3 *pOut, CONST XGVECTOR3 *pV0, CONST XGVECTOR3 *pV1,
      CONST XGVECTOR3 *pV2, CONST XGVECTOR3 *pV3, FLOAT s );

// Barycentric coordinates.  V1 + f(V2-V1) + g(V3-V1)
XGVECTOR3* WINAPI XGVec3BaryCentric
    ( XGVECTOR3 *pOut, CONST XGVECTOR3 *pV1, CONST XGVECTOR3 *pV2,
      CONST XGVECTOR3 *pV3, FLOAT f, FLOAT g);

// Transform (x, y, z, 1) by matrix.
XGVECTOR4* WINAPI XGVec3Transform
    ( XGVECTOR4 *pOut, CONST XGVECTOR3 *pV, CONST XGMATRIX *pM );

// Transform (x, y, z, 1) by matrix, project result back into w=1.
XGVECTOR3* WINAPI XGVec3TransformCoord
    ( XGVECTOR3 *pOut, CONST XGVECTOR3 *pV, CONST XGMATRIX *pM );

// Transform (x, y, z, 0) by matrix.  If you transforming a normal by a
// non-affine matrix, the matrix you pass to this function should be the
// transpose of the inverse of the matrix you would use to transform a coord.
XGVECTOR3* WINAPI XGVec3TransformNormal
    ( XGVECTOR3 *pOut, CONST XGVECTOR3 *pV, CONST XGMATRIX *pM );

// Project vector from object space into screen space
XGVECTOR3* WINAPI XGVec3Project
    ( XGVECTOR3 *pOut, CONST XGVECTOR3 *pV, CONST D3DVIEWPORT8 *pViewport,
      CONST XGMATRIX *pProjection, CONST XGMATRIX *pView, CONST XGMATRIX *pWorld);

// Project vector from screen space into object space
XGVECTOR3* WINAPI XGVec3Unproject
    ( XGVECTOR3 *pOut, CONST XGVECTOR3 *pV, CONST D3DVIEWPORT8 *pViewport,
      CONST XGMATRIX *pProjection, CONST XGMATRIX *pView, CONST XGMATRIX *pWorld);

#ifdef __cplusplus
}
#endif



//--------------------------
// 4D Vector
//--------------------------

// inline

FLOAT XGVec4Length
    ( CONST XGVECTOR4 *pV );

FLOAT XGVec4LengthSq
    ( CONST XGVECTOR4 *pV );

FLOAT XGVec4Dot
    ( CONST XGVECTOR4 *pV1, CONST XGVECTOR4 *pV2 );

XGVECTOR4* XGVec4Add
    ( XGVECTOR4 *pOut, CONST XGVECTOR4 *pV1, CONST XGVECTOR4 *pV2);

XGVECTOR4* XGVec4Subtract
    ( XGVECTOR4 *pOut, CONST XGVECTOR4 *pV1, CONST XGVECTOR4 *pV2);

// Minimize each component.  x = min(x1, x2), y = min(y1, y2), ...
XGVECTOR4* XGVec4Minimize
    ( XGVECTOR4 *pOut, CONST XGVECTOR4 *pV1, CONST XGVECTOR4 *pV2);

// Maximize each component.  x = max(x1, x2), y = max(y1, y2), ...
XGVECTOR4* XGVec4Maximize
    ( XGVECTOR4 *pOut, CONST XGVECTOR4 *pV1, CONST XGVECTOR4 *pV2);

XGVECTOR4* XGVec4Scale
    ( XGVECTOR4 *pOut, CONST XGVECTOR4 *pV, FLOAT s);

// Linear interpolation. V1 + s(V2-V1)
XGVECTOR4* XGVec4Lerp
    ( XGVECTOR4 *pOut, CONST XGVECTOR4 *pV1, CONST XGVECTOR4 *pV2,
      FLOAT s );

// non-inline
#ifdef __cplusplus
extern "C" {
#endif

// Cross-product in 4 dimensions.
XGVECTOR4* WINAPI XGVec4Cross
    ( XGVECTOR4 *pOut, CONST XGVECTOR4 *pV1, CONST XGVECTOR4 *pV2,
      CONST XGVECTOR4 *pV3);

XGVECTOR4* WINAPI XGVec4Normalize
    ( XGVECTOR4 *pOut, CONST XGVECTOR4 *pV );

// Hermite interpolation between position V1, tangent T1 (when s == 0)
// and position V2, tangent T2 (when s == 1).
XGVECTOR4* WINAPI XGVec4Hermite
    ( XGVECTOR4 *pOut, CONST XGVECTOR4 *pV1, CONST XGVECTOR4 *pT1,
      CONST XGVECTOR4 *pV2, CONST XGVECTOR4 *pT2, FLOAT s );

// CatmullRom interpolation between V1 (when s == 0) and V2 (when s == 1)
XGVECTOR4* WINAPI XGVec4CatmullRom
    ( XGVECTOR4 *pOut, CONST XGVECTOR4 *pV0, CONST XGVECTOR4 *pV1,
      CONST XGVECTOR4 *pV2, CONST XGVECTOR4 *pV3, FLOAT s );

// Barycentric coordinates.  V1 + f(V2-V1) + g(V3-V1)
XGVECTOR4* WINAPI XGVec4BaryCentric
    ( XGVECTOR4 *pOut, CONST XGVECTOR4 *pV1, CONST XGVECTOR4 *pV2,
      CONST XGVECTOR4 *pV3, FLOAT f, FLOAT g);

// Transform vector by matrix.
XGVECTOR4* WINAPI XGVec4Transform
    ( XGVECTOR4 *pOut, CONST XGVECTOR4 *pV, CONST XGMATRIX *pM );

#ifdef __cplusplus
}
#endif


//--------------------------
// 4D Matrix
//--------------------------

// inline

XGMATRIX* XGMatrixIdentity
    ( XGMATRIX *pOut );

BOOL XGMatrixIsIdentity
    ( CONST XGMATRIX *pM );


// non-inline
#ifdef __cplusplus
extern "C" {
#endif

FLOAT WINAPI XGMatrixfDeterminant
    ( CONST XGMATRIX *pM );

// Matrix multiplication.  The result represents the transformation M2
// followed by the transformation M1.  (Out = M1 * M2)
XGMATRIX* WINAPI XGMatrixMultiply
    ( XGMATRIX *pOut, CONST XGMATRIX *pM1, CONST XGMATRIX *pM2 );

XGMATRIX* WINAPI XGMatrixTranspose
    ( XGMATRIX *pOut, CONST XGMATRIX *pM );

// Calculate inverse of matrix.  Inversion my fail, in which case NULL will
// be returned.  The determinant of pM is also returned it pfDeterminant
// is non-NULL.
XGMATRIX* WINAPI XGMatrixInverse
    ( XGMATRIX *pOut, FLOAT *pDeterminant, CONST XGMATRIX *pM );

// Build a matrix which scales by (sx, sy, sz)
XGMATRIX* WINAPI XGMatrixScaling
    ( XGMATRIX *pOut, FLOAT sx, FLOAT sy, FLOAT sz );

// Build a matrix which translates by (x, y, z)
XGMATRIX* WINAPI XGMatrixTranslation
    ( XGMATRIX *pOut, FLOAT x, FLOAT y, FLOAT z );

// Build a matrix which rotates around the X axis
XGMATRIX* WINAPI XGMatrixRotationX
    ( XGMATRIX *pOut, FLOAT Angle );

// Build a matrix which rotates around the Y axis
XGMATRIX* WINAPI XGMatrixRotationY
    ( XGMATRIX *pOut, FLOAT Angle );

// Build a matrix which rotates around the Z axis
XGMATRIX* WINAPI XGMatrixRotationZ
    ( XGMATRIX *pOut, FLOAT Angle );

// Build a matrix which rotates around an arbitrary axis
XGMATRIX* WINAPI XGMatrixRotationAxis
    ( XGMATRIX *pOut, CONST XGVECTOR3 *pV, FLOAT Angle );

// Build a matrix from a quaternion
XGMATRIX* WINAPI XGMatrixRotationQuaternion
    ( XGMATRIX *pOut, CONST XGQUATERNION *pQ);

// Yaw around the Y axis, a pitch around the X axis,
// and a roll around the Z axis.
XGMATRIX* WINAPI XGMatrixRotationYawPitchRoll
    ( XGMATRIX *pOut, FLOAT Yaw, FLOAT Pitch, FLOAT Roll );

// Build transformation matrix.  NULL arguments are treated as identity.
// Mout = Msc-1 * Msr-1 * Ms * Msr * Msc * Mrc-1 * Mr * Mrc * Mt
XGMATRIX* WINAPI XGMatrixTransformation
    ( XGMATRIX *pOut, CONST XGVECTOR3 *pScalingCenter,
      CONST XGQUATERNION *pScalingRotation, CONST XGVECTOR3 *pScaling,
      CONST XGVECTOR3 *pRotationCenter, CONST XGQUATERNION *pRotation,
      CONST XGVECTOR3 *pTranslation);

// Build affine transformation matrix.  NULL arguments are treated as identity.
// Mout = Ms * Mrc-1 * Mr * Mrc * Mt
XGMATRIX* WINAPI XGMatrixAffineTransformation
    ( XGMATRIX *pOut, FLOAT Scaling, CONST XGVECTOR3 *pRotationCenter,
      CONST XGQUATERNION *pRotation, CONST XGVECTOR3 *pTranslation);

// Build a lookat matrix. (right-handed)
XGMATRIX* WINAPI XGMatrixLookAtRH
    ( XGMATRIX *pOut, CONST XGVECTOR3 *pEye, CONST XGVECTOR3 *pAt,
      CONST XGVECTOR3 *pUp );

// Build a lookat matrix. (left-handed)
XGMATRIX* WINAPI XGMatrixLookAtLH
    ( XGMATRIX *pOut, CONST XGVECTOR3 *pEye, CONST XGVECTOR3 *pAt,
      CONST XGVECTOR3 *pUp );

// Build a perspective projection matrix. (right-handed)
XGMATRIX* WINAPI XGMatrixPerspectiveRH
    ( XGMATRIX *pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf );

// Build a perspective projection matrix. (left-handed)
XGMATRIX* WINAPI XGMatrixPerspectiveLH
    ( XGMATRIX *pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf );

// Build a perspective projection matrix. (right-handed)
XGMATRIX* WINAPI XGMatrixPerspectiveFovRH
    ( XGMATRIX *pOut, FLOAT fovy, FLOAT Aspect, FLOAT zn, FLOAT zf );

// Build a perspective projection matrix. (left-handed)
XGMATRIX* WINAPI XGMatrixPerspectiveFovLH
    ( XGMATRIX *pOut, FLOAT fovy, FLOAT Aspect, FLOAT zn, FLOAT zf );

// Build a perspective projection matrix. (right-handed)
XGMATRIX* WINAPI XGMatrixPerspectiveOffCenterRH
    ( XGMATRIX *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn,
      FLOAT zf );

// Build a perspective projection matrix. (left-handed)
XGMATRIX* WINAPI XGMatrixPerspectiveOffCenterLH
    ( XGMATRIX *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn,
      FLOAT zf );

// Build an ortho projection matrix. (right-handed)
XGMATRIX* WINAPI XGMatrixOrthoRH
    ( XGMATRIX *pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf );

// Build an ortho projection matrix. (left-handed)
XGMATRIX* WINAPI XGMatrixOrthoLH
    ( XGMATRIX *pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf );

// Build an ortho projection matrix. (right-handed)
XGMATRIX* WINAPI XGMatrixOrthoOffCenterRH
    ( XGMATRIX *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn,
      FLOAT zf );

// Build an ortho projection matrix. (left-handed)
XGMATRIX* WINAPI XGMatrixOrthoOffCenterLH
    ( XGMATRIX *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn,
      FLOAT zf );

// Build a matrix which flattens geometry into a plane, as if casting
// a shadow from a light.
XGMATRIX* WINAPI XGMatrixShadow
    ( XGMATRIX *pOut, CONST XGVECTOR4 *pLight,
      CONST XGPLANE *pPlane );

// Build a matrix which reflects the coordinate system about a plane
XGMATRIX* WINAPI XGMatrixReflect
    ( XGMATRIX *pOut, CONST XGPLANE *pPlane );

#ifdef __cplusplus
}
#endif


//--------------------------
// Quaternion
//--------------------------

// inline

FLOAT XGQuaternionLength
    ( CONST XGQUATERNION *pQ );

// Length squared, or "norm"
FLOAT XGQuaternionLengthSq
    ( CONST XGQUATERNION *pQ );

FLOAT XGQuaternionDot
    ( CONST XGQUATERNION *pQ1, CONST XGQUATERNION *pQ2 );

// (0, 0, 0, 1)
XGQUATERNION* XGQuaternionIdentity
    ( XGQUATERNION *pOut );

BOOL XGQuaternionIsIdentity
    ( CONST XGQUATERNION *pQ );

// (-x, -y, -z, w)
XGQUATERNION* XGQuaternionConjugate
    ( XGQUATERNION *pOut, CONST XGQUATERNION *pQ );


// non-inline
#ifdef __cplusplus
extern "C" {
#endif

// Compute a quaternin's axis and angle of rotation. Expects unit quaternions.
void WINAPI XGQuaternionToAxisAngle
    ( CONST XGQUATERNION *pQ, XGVECTOR3 *pAxis, FLOAT *pAngle );

// Build a quaternion from a rotation matrix.
XGQUATERNION* WINAPI XGQuaternionRotationMatrix
    ( XGQUATERNION *pOut, CONST XGMATRIX *pM);

// Rotation about arbitrary axis.
XGQUATERNION* WINAPI XGQuaternionRotationAxis
    ( XGQUATERNION *pOut, CONST XGVECTOR3 *pV, FLOAT Angle );

// Yaw around the Y axis, a pitch around the X axis,
// and a roll around the Z axis.
XGQUATERNION* WINAPI XGQuaternionRotationYawPitchRoll
    ( XGQUATERNION *pOut, FLOAT Yaw, FLOAT Pitch, FLOAT Roll );

// Quaternion multiplication.  The result represents the rotation Q2
// followed by the rotation Q1.  (Out = Q2 * Q1)
XGQUATERNION* WINAPI XGQuaternionMultiply
    ( XGQUATERNION *pOut, CONST XGQUATERNION *pQ1,
      CONST XGQUATERNION *pQ2 );

XGQUATERNION* WINAPI XGQuaternionNormalize
    ( XGQUATERNION *pOut, CONST XGQUATERNION *pQ );

// Conjugate and re-norm
XGQUATERNION* WINAPI XGQuaternionInverse
    ( XGQUATERNION *pOut, CONST XGQUATERNION *pQ );

// Expects unit quaternions.
// if q = (cos(theta), sin(theta) * v); ln(q) = (0, theta * v)
XGQUATERNION* WINAPI XGQuaternionLn
    ( XGQUATERNION *pOut, CONST XGQUATERNION *pQ );

// Expects pure quaternions. (w == 0)  w is ignored in calculation.
// if q = (0, theta * v); exp(q) = (cos(theta), sin(theta) * v)
XGQUATERNION* WINAPI XGQuaternionExp
    ( XGQUATERNION *pOut, CONST XGQUATERNION *pQ );

// Spherical linear interpolation between Q1 (s == 0) and Q2 (s == 1).
// Expects unit quaternions.
XGQUATERNION* WINAPI XGQuaternionSlerp
    ( XGQUATERNION *pOut, CONST XGQUATERNION *pQ1,
      CONST XGQUATERNION *pQ2, FLOAT t );

// Spherical quadrangle interpolation.
// Slerp(Slerp(Q1, Q4, t), Slerp(Q2, Q3, t), 2t(1-t))
XGQUATERNION* WINAPI XGQuaternionSquad
    ( XGQUATERNION *pOut, CONST XGQUATERNION *pQ1,
      CONST XGQUATERNION *pQ2, CONST XGQUATERNION *pQ3,
      CONST XGQUATERNION *pQ4, FLOAT t );

// Slerp(Slerp(Q1, Q2, f+g), Slerp(Q1, Q3, f+g), g/(f+g))
XGQUATERNION* WINAPI XGQuaternionBaryCentric
    ( XGQUATERNION *pOut, CONST XGQUATERNION *pQ1,
      CONST XGQUATERNION *pQ2, CONST XGQUATERNION *pQ3,
      FLOAT f, FLOAT g );

#ifdef __cplusplus
}
#endif


//--------------------------
// Plane
//--------------------------

// inline

// ax + by + cz + dw
FLOAT XGPlaneDot
    ( CONST XGPLANE *pP, CONST XGVECTOR4 *pV);

// ax + by + cz + d
FLOAT XGPlaneDotCoord
    ( CONST XGPLANE *pP, CONST XGVECTOR3 *pV);

// ax + by + cz
FLOAT XGPlaneDotNormal
    ( CONST XGPLANE *pP, CONST XGVECTOR3 *pV);

// non-inline
#ifdef __cplusplus
extern "C" {
#endif

// Normalize plane (so that |a,b,c| == 1)
XGPLANE* WINAPI XGPlaneNormalize
    ( XGPLANE *pOut, CONST XGPLANE *pP);

// Find the intersection between a plane and a line.  If the line is
// parallel to the plane, NULL is returned.
XGVECTOR3* WINAPI XGPlaneIntersectLine
    ( XGVECTOR3 *pOut, CONST XGPLANE *pP, CONST XGVECTOR3 *pV1,
      CONST XGVECTOR3 *pV2);

// Construct a plane from a point and a normal
XGPLANE* WINAPI XGPlaneFromPointNormal
    ( XGPLANE *pOut, CONST XGVECTOR3 *pPoint, CONST XGVECTOR3 *pNormal);

// Construct a plane from 3 points
XGPLANE* WINAPI XGPlaneFromPoints
    ( XGPLANE *pOut, CONST XGVECTOR3 *pV1, CONST XGVECTOR3 *pV2,
      CONST XGVECTOR3 *pV3);

// Transform a plane by a matrix.  The vector (a,b,c) must be normal.
// M must be an affine transform.
XGPLANE* WINAPI XGPlaneTransform
    ( XGPLANE *pOut, CONST XGPLANE *pP, CONST XGMATRIX *pM );

#ifdef __cplusplus
}
#endif


//--------------------------
// Color
//--------------------------

// inline

// (1-r, 1-g, 1-b, a)
XGCOLOR* XGColorNegative
    (XGCOLOR *pOut, CONST XGCOLOR *pC);

XGCOLOR* XGColorAdd
    (XGCOLOR *pOut, CONST XGCOLOR *pC1, CONST XGCOLOR *pC2);

XGCOLOR* XGColorSubtract
    (XGCOLOR *pOut, CONST XGCOLOR *pC1, CONST XGCOLOR *pC2);

XGCOLOR* XGColorScale
    (XGCOLOR *pOut, CONST XGCOLOR *pC, FLOAT s);

// (r1*r2, g1*g2, b1*b2, a1*a2)
XGCOLOR* XGColorModulate
    (XGCOLOR *pOut, CONST XGCOLOR *pC1, CONST XGCOLOR *pC2);

// Linear interpolation of r,g,b, and a. C1 + s(C2-C1)
XGCOLOR* XGColorLerp
    (XGCOLOR *pOut, CONST XGCOLOR *pC1, CONST XGCOLOR *pC2, FLOAT s);

// non-inline
#ifdef __cplusplus
extern "C" {
#endif

// Interpolate r,g,b between desaturated color and color.
// DesaturatedColor + s(Color - DesaturatedColor)
XGCOLOR* WINAPI XGColorAdjustSaturation
    (XGCOLOR *pOut, CONST XGCOLOR *pC, FLOAT s);

// Interpolate r,g,b between 50% grey and color.  Grey + s(Color - Grey)
XGCOLOR* WINAPI XGColorAdjustContrast
    (XGCOLOR *pOut, CONST XGCOLOR *pC, FLOAT c);

#ifdef __cplusplus
}
#endif

#include "xgmath.inl"

#pragma warning(default:4201)

#ifdef __cplusplus
}
#endif // __cplusplus

#ifdef _USE_XGMATH

//--------------------------
// D3DX Compatibility defines
//--------------------------

#define D3DX_PI XG_PI
#define D3DX_1BYPI XG_1BYPI

#define D3DXToRadian XGToRadian
#define D3DXToDegree XGToDegree

#define D3DXVECTOR2 XGVECTOR2
#define LPD3DXVECTOR2 LPXGVECTOR2
#define D3DXVECTOR3 XGVECTOR3
#define LPD3DXVECTOR3 LPXGVECTOR3
#define D3DXVECTOR4 XGVECTOR4
#define LPD3DXVECTOR4 LPXGVECTOR4
#define D3DXMATRIX XGMATRIX
#define LPD3DXMATRIX LPXGMATRIX
#define D3DXQUATERNION XGQUATERNION
#define LPD3DXQUATERNION LPXGQUATERNION
#define D3DXPLANE XGPLANE
#define LPD3DXPLANE LPXGPLANE
#define D3DXCOLOR XGCOLOR
#define LPD3DXCOLOR LPXGCOLOR

#define D3DXVec2Length XGVec2Length
#define D3DXVec2LengthSq XGVec2LengthSq
#define D3DXVec2Dot XGVec2Dot
#define D3DXVec2CCW XGVec2CCW
#define D3DXVec2Add XGVec2Add
#define D3DXVec2Subtract XGVec2Subtract
#define D3DXVec2Minimize XGVec2Minimize
#define D3DXVec2Maximize XGVec2Maximize
#define D3DXVec2Scale XGVec2Scale
#define D3DXVec2Lerp XGVec2Lerp
#define D3DXVec2Normalize XGVec2Normalize
#define D3DXVec2Hermite XGVec2Hermite
#define D3DXVec2CatmullRom XGVec2CatmullRom
#define D3DXVec2BaryCentric XGVec2BaryCentric
#define D3DXVec2Transform XGVec2Transform
#define D3DXVec2TransformCoord XGVec2TransformCoord
#define D3DXVec2TransformNormal XGVec2TransformNormal

#define D3DXVec3Length XGVec3Length
#define D3DXVec3LengthSq XGVec3LengthSq
#define D3DXVec3Dot XGVec3Dot
#define D3DXVec3Cross XGVec3Cross
#define D3DXVec3Add XGVec3Add
#define D3DXVec3Subtract XGVec3Subtract
#define D3DXVec3Minimize XGVec3Minimize
#define D3DXVec3Maximize XGVec3Maximize
#define D3DXVec3Scale XGVec3Scale
#define D3DXVec3Lerp XGVec3Lerp
#define D3DXVec3Normalize XGVec3Normalize
#define D3DXVec3Hermite XGVec3Hermite
#define D3DXVec3CatmullRom XGVec3CatmullRom
#define D3DXVec3BaryCentric XGVec3BaryCentric
#define D3DXVec3Transform XGVec3Transform
#define D3DXVec3TransformCoord XGVec3TransformCoord
#define D3DXVec3TransformNormal XGVec3TransformNormal
#define D3DXVec3Project XGVec3Project
#define D3DXVec3Unproject XGVec3Unproject

#define D3DXVec4Length XGVec4Length
#define D3DXVec4LengthSq XGVec4LengthSq
#define D3DXVec4Dot XGVec4Dot
#define D3DXVec4Add XGVec4Add
#define D3DXVec4Subtract XGVec4Subtract
#define D3DXVec4Minimize XGVec4Minimize
#define D3DXVec4Maximize XGVec4Maximize
#define D3DXVec4Scale XGVec4Scale
#define D3DXVec4Lerp XGVec4Lerp
#define D3DXVec4Cross XGVec4Cross
#define D3DXVec4Normalize XGVec4Normalize
#define D3DXVec4Hermite XGVec4Hermite
#define D3DXVec4CatmullRom XGVec4CatmullRom
#define D3DXVec4BaryCentric XGVec4BaryCentric
#define D3DXVec4Transform XGVec4Transform

#define D3DXMatrixIdentity XGMatrixIdentity
#define D3DXMatrixIsIdentity XGMatrixIsIdentity
#define D3DXMatrixfDeterminant XGMatrixfDeterminant
#define D3DXMatrixMultiply XGMatrixMultiply
#define D3DXMatrixTranspose XGMatrixTranspose
#define D3DXMatrixInverse XGMatrixInverse
#define D3DXMatrixScaling XGMatrixScaling
#define D3DXMatrixTranslation XGMatrixTranslation
#define D3DXMatrixRotationX XGMatrixRotationX
#define D3DXMatrixRotationY XGMatrixRotationY
#define D3DXMatrixRotationZ XGMatrixRotationZ
#define D3DXMatrixRotationAxis XGMatrixRotationAxis
#define D3DXMatrixRotationQuaternion XGMatrixRotationQuaternion
#define D3DXMatrixRotationYawPitchRoll XGMatrixRotationYawPitchRoll
#define D3DXMatrixTransformation XGMatrixTransformation
#define D3DXMatrixAffineTransformation XGMatrixAffineTransformation
#define D3DXMatrixLookAtRH XGMatrixLookAtRH
#define D3DXMatrixLookAtLH XGMatrixLookAtLH
#define D3DXMatrixPerspectiveRH XGMatrixPerspectiveRH
#define D3DXMatrixPerspectiveLH XGMatrixPerspectiveLH
#define D3DXMatrixPerspectiveFovRH XGMatrixPerspectiveFovRH
#define D3DXMatrixPerspectiveFovLH XGMatrixPerspectiveFovLH
#define D3DXMatrixPerspectiveOffCenterRH XGMatrixPerspectiveOffCenterRH
#define D3DXMatrixPerspectiveOffCenterLH XGMatrixPerspectiveOffCenterLH
#define D3DXMatrixOrthoRH XGMatrixOrthoRH
#define D3DXMatrixOrthoLH XGMatrixOrthoLH
#define D3DXMatrixOrthoOffCenterRH XGMatrixOrthoOffCenterRH
#define D3DXMatrixOrthoOffCenterLH XGMatrixOrthoOffCenterLH
#define D3DXMatrixShadow XGMatrixShadow
#define D3DXMatrixReflect XGMatrixReflect

#define D3DXQuaternionLength XGQuaternionLength
#define D3DXQuaternionLengthSq XGQuaternionLengthSq
#define D3DXQuaternionDot XGQuaternionDot
#define D3DXQuaternionIdentity XGQuaternionIdentity
#define D3DXQuaternionIsIdentity XGQuaternionIsIdentity
#define D3DXQuaternionConjugate XGQuaternionConjugate
#define D3DXQuaternionToAxisAngle XGQuaternionToAxisAngle
#define D3DXQuaternionRotationMatrix XGQuaternionRotationMatrix
#define D3DXQuaternionRotationAxis XGQuaternionRotationAxis
#define D3DXQuaternionRotationYawPitchRoll XGQuaternionRotationYawPitchRoll
#define D3DXQuaternionMultiply XGQuaternionMultiply
#define D3DXQuaternionNormalize XGQuaternionNormalize
#define D3DXQuaternionInverse XGQuaternionInverse
#define D3DXQuaternionLn XGQuaternionLn
#define D3DXQuaternionExp XGQuaternionExp
#define D3DXQuaternionSlerp XGQuaternionSlerp
#define D3DXQuaternionSquad XGQuaternionSquad
#define D3DXQuaternionBaryCentric XGQuaternionBaryCentric

#define D3DXPlaneDot XGPlaneDot
#define D3DXPlaneDotCoord XGPlaneDotCoord
#define D3DXPlaneDotNormal XGPlaneDotNormal
#define D3DXPlaneNormalize XGPlaneNormalize
#define D3DXPlaneIntersectLine XGPlaneIntersectLine
#define D3DXPlaneFromPointNormal XGPlaneFromPointNormal
#define D3DXPlaneFromPoints XGPlaneFromPoints
#define D3DXPlaneTransform XGPlaneTransform

#define D3DXColorNegative XGColorNegative
#define D3DXColorAdd XGColorAdd
#define D3DXColorSubtract XGColorSubtract
#define D3DXColorScale XGColorScale
#define D3DXColorModulate XGColorModulate
#define D3DXColorLerp XGColorLerp
#define D3DXColorAdjustSaturation XGColorAdjustSaturation
#define D3DXColorAdjustContrast XGColorAdjustContrast

#endif // _USE_XGMATH

#endif // __XGMATH_H__

