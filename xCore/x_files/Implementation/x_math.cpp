//==============================================================================
//
//  x_math.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_MATH_HPP
#include "..\x_math.hpp"
#endif

#ifdef TARGET_XBOX
    #ifdef CONFIG_RETAIL
        #define D3DCOMPILE_PUREDEVICE 1
    #endif
    #include<xtl.h>
    #include<xgraphics.h>
    #include "xmmintrin.h"
    #include "D3dx8math.h"
    #include <xgmath.h>
#endif

//==============================================================================
//  DEFINES
//==============================================================================

#define M(row,column)   m_Cell[row][column]



//==============================================================================
//  FUNCTIONS
//==============================================================================

quaternion BlendSlow( const quaternion& Q0, 
                  const quaternion& Q1, f32 T )
{
    quaternion Q;
    f32 Cs,Sn;
    f32 Angle,InvSn,TAngle;
    f32 C0,C1;
    f32 x0,y0,z0,w0;

    // Determine if quats are further than 90 degrees
    Cs = Q0.X*Q1.X + Q0.Y*Q1.Y + Q0.Z*Q1.Z + Q0.W*Q1.W;

    // If dot is negative flip one of the quaterions
    if( Cs < 0.0f )
    {
       x0 = -Q0.X;
       y0 = -Q0.Y;
       z0 = -Q0.Z;
       w0 = -Q0.W;
       Cs = -Cs;
    }
    else
    {
       x0 = +Q0.X;
       y0 = +Q0.Y;
       z0 = +Q0.Z;
       w0 = +Q0.W;
    }

    // Compute sine of angle between Q0,Q1
    Sn = 1.0f - Cs*Cs;
    if( Sn < 0.0f ) Sn = -Sn;
    Sn = x_sqrt( Sn );

    // Check if quaternions are very close together
    if( (Sn < 1e-3) && (Sn > -1e-3) )
    {
        return Q0;
    }

    Angle   = x_atan2( Sn, Cs );
    InvSn   = 1.0f/Sn;    
    TAngle  = T*Angle;

    C0      = x_sin( Angle - TAngle) * InvSn;
    C1      = x_sin( TAngle ) * InvSn;    

    Q.X     = C0*x0 + C1*Q1.X;
    Q.Y     = C0*y0 + C1*Q1.Y;
    Q.Z     = C0*z0 + C1*Q1.Z;
    Q.W     = C0*w0 + C1*Q1.W;

    return Q;
}

//==============================================================================

quaternion BlendToIdentitySlow( const quaternion& Q0, f32 T )
{
    quaternion Q;
    f32 Cs,Sn;
    f32 Angle,InvSn,TAngle;
    f32 C0,C1;
    f32 x0,y0,z0,w0;

    // Determine if quats are further than 90 degrees
    Cs = Q0.W;

    // If dot is negative flip one of the quaterions
    if( Cs < 0.0f )
    {
       x0 = -Q0.X;
       y0 = -Q0.Y;
       z0 = -Q0.Z;
       w0 = -Q0.W;
       Cs = -Cs;
    }
    else
    {
       x0 = +Q0.X;
       y0 = +Q0.Y;
       z0 = +Q0.Z;
       w0 = +Q0.W;
    }

    // Compute sine of angle between Q0,Q1
    Sn = 1.0f - Cs*Cs;
    if( Sn < 0.0f ) Sn = -Sn;
    Sn = x_sqrt( Sn );

    // Check if quaternions are very close together
    if( (Sn < 1e-3) && (Sn > -1e-3) )
    {
        return Q0;
    }

    Angle   = x_atan2( Sn, Cs );
    InvSn   = 1.0f/Sn;    
    TAngle  = T*Angle;

    C0      = x_sin( Angle - TAngle) * InvSn;
    C1      = x_sin( TAngle ) * InvSn;    

    Q.X     = C0*x0;
    Q.Y     = C0*y0;
    Q.Z     = C0*z0;
    Q.W     = C0*w0 + C1;

    return Q;
}

//==============================================================================

quaternion Blend( const quaternion& Q0, 
                      const quaternion& Q1, f32 T )
{
    f32 Dot;
    f32 LenSquared;
    f32 OneOverL;
    f32 x0,y0,z0,w0;

    // Determine if quats are further than 90 degrees
    Dot = Q0.X*Q1.X + Q0.Y*Q1.Y + Q0.Z*Q1.Z + Q0.W*Q1.W;

    // If dot is negative flip one of the quaterions
    if( Dot < 0.0f )
    {
       x0 = -Q0.X;
       y0 = -Q0.Y;
       z0 = -Q0.Z;
       w0 = -Q0.W;
    }
    else
    {
       x0 = +Q0.X;
       y0 = +Q0.Y;
       z0 = +Q0.Z;
       w0 = +Q0.W;
    }

    // Compute interpolated values
    x0 = x0 + T*(Q1.X - x0);
    y0 = y0 + T*(Q1.Y - y0);
    z0 = z0 + T*(Q1.Z - z0);
    w0 = w0 + T*(Q1.W - w0);

    // Get squared length of new quaternion
    LenSquared = x0*x0 + y0*y0 + z0*z0 + w0*w0;

    // Use home-baked polynomial to compute 1/sqrt(LenSquared)
    // Input range is 0.5 <-> 1.0
    // Ouput range is 1.414213 <-> 1.0

    if( LenSquared<0.857f )
        OneOverL = (((0.699368f)*LenSquared) + -1.819985f)*LenSquared + 2.126369f;    //0.0000792
    else
        OneOverL = (((0.454012f)*LenSquared) + -1.403517f)*LenSquared + 1.949542f;    //0.0000373

    // Renormalize and return quaternion
    return quaternion( x0*OneOverL, y0*OneOverL, z0*OneOverL, w0*OneOverL );
}

//==============================================================================

quaternion BlendToIdentity( const quaternion& Q0, f32 T )
{
    f32 LenSquared;
    f32 OneOverL;
    f32 x0,y0,z0,w0;

    // If dot is negative flip one of the quaterions
    if( Q0.W < 0.0f )
    {
       x0 = -Q0.X;
       y0 = -Q0.Y;
       z0 = -Q0.Z;
       w0 = -Q0.W;
    }
    else
    {
       x0 = +Q0.X;
       y0 = +Q0.Y;
       z0 = +Q0.Z;
       w0 = +Q0.W;
    }

    // Compute interpolated values
    x0 = x0 + T*(0.0f - x0);
    y0 = y0 + T*(0.0f - y0);
    z0 = z0 + T*(0.0f - z0);
    w0 = w0 + T*(1.0f - w0);

    // Get squared length of new quaternion
    LenSquared = x0*x0 + y0*y0 + z0*z0 + w0*w0;

    // Use home-baked polynomial to compute 1/sqrt(LenSquared)
    // Input range is 0.5 <-> 1.0
    // Ouput range is 1.414213 <-> 1.0

    if( LenSquared<0.857f )
        OneOverL = (((0.699368f)*LenSquared) + -1.819985f)*LenSquared + 2.126369f;    //0.0000792
    else
        OneOverL = (((0.454012f)*LenSquared) + -1.403517f)*LenSquared + 1.949542f;    //0.0000373

    // Renormalize and return quaternion
    return quaternion( x0*OneOverL, y0*OneOverL, z0*OneOverL, w0*OneOverL );
}

//==============================================================================
// MATRIX4
//==============================================================================

void matrix4::CleanRotation( void )
{
/*
    static X_FILE* fp = NULL;
    if( !fp ) fp = x_fopen("c:/temp/cleanrot.txt","wt");
    ASSERT(fp);

    x_fprintf(fp,"----------------------------------------------\n");
    radian3 Rot = GetRotation();
    x_fprintf(fp,"Rot   (%16.10f,%16.10f,%16.10f)\n",RAD_TO_DEG(Rot.Pitch),RAD_TO_DEG(Rot.Yaw),RAD_TO_DEG(Rot.Roll));
*/
    {
        f32 ANGLE_SNAP_EPSILON = 0.001f;
        radian3 Rot = GetRotation();
        f32* pAngle = (f32*)(&Rot);
        for( s32 i=0; i<3; i++ )
        {
            radian Angle = RAD_TO_DEG(pAngle[i]);
            xbool bNegative = (Angle < 0);
            Angle = x_abs(Angle);
            
            s32 iAngle = (s32)Angle;
            f32 Delta = Angle - (f32)iAngle;
            if( Delta < ANGLE_SNAP_EPSILON ) 
            {
                // Snap to lower angle
                Angle = (f32)iAngle;
            }
            else
            if( Delta > (1.0f-ANGLE_SNAP_EPSILON) ) 
            {
                // Snap to upper angle
                Angle = (f32)(iAngle+1);
            }

            // Put sign back in and convert back into radians
            Angle = bNegative ? (-Angle) : (Angle);
            pAngle[i] = DEG_TO_RAD(Angle);
        }
        SetRotation(Rot);
    }
/*
    Rot = GetRotation();
    x_fprintf(fp,"Rot   (%16.10f,%16.10f,%16.10f)\n",RAD_TO_DEG(Rot.Pitch),RAD_TO_DEG(Rot.Yaw),RAD_TO_DEG(Rot.Roll));
    x_fflush(fp);
*/
}

//==============================================================================

xbool matrix4::Invert( void )
{
#ifdef TARGET_XBOX
    FLOAT Det;
    XGMatrixInverse((XGMATRIX*)this,&Det,(CONST XGMATRIX*)this );
#else
    f32 Scratch[4][8];
    f32 a;
    s32 i, j, k, jr, Pivot;
    s32 Row[4];    

    //
    // Initialize.
    //

    for( j = 0; j < 4; j++ )
    {
        for( k = 0; k < 4; k++ )
        {
            Scratch[j][k]   = M(j,k);
            Scratch[j][4+k] = 0.0f;
        }

        Scratch[j][4+j] = 1.0f;
        Row[j] = j;
    }

    //
    // Eliminate columns.
    //

    for( i = 0; i < 4; i++ )
    {
        // Find pivot.
        k = i;
        
        a = x_abs( Scratch[Row[k]][k] );
        
        for( j = i+1; j < 4; j++ )
        {
            jr = Row[j];

            if( a < x_abs( Scratch[jr][i] ) )
            {
                k = j;
                a = x_abs( Scratch[jr][i] );
            }
        }

        // Swap the pivot row (Row[k]) with the i'th row.
        Pivot  = Row[k];
        Row[k] = Row[i];
        Row[i] = Pivot;

        // Normalize pivot row.
        a = Scratch[Pivot][i];

        if( a == 0.0f ) 
            return( FALSE );

        Scratch[Pivot][i] = 1.0f;

        for( k = i+1; k < 8; k++ ) 
            Scratch[Pivot][k] /= a;

        // Eliminate pivot from all remaining rows.
        for( j = i+1; j < 4; j++ )
        {
            jr = Row[j];
            a  = -Scratch[jr][i];
            
            if( a == 0.0f ) 
                continue;

            Scratch[jr][i] = 0.0f;

            for( k = i+1; k < 8; k++ )
                Scratch[jr][k] += (a * Scratch[Pivot][k]);
        }
    }

    //
    // Back solve.
    //

    for( i = 3; i > 0; i-- )
    {
        Pivot = Row[i];
        for( j = i-1; j >= 0; j-- )
        {
            jr = Row[j];
            a  = Scratch[jr][i];

            for( k = i; k < 8; k++ )
                Scratch[jr][k] -= (a * Scratch[Pivot][k]);
        }
    }

    //
    // Copy inverse back into the matrix.
    //

    for( j = 0; j < 4; j++ )
    {
        jr = Row[j];
        for( k = 0; k < 4; k++ )
        {
            M(j,k) = Scratch[jr][k+4];
        }
    }
#endif

    // Success!
    return( TRUE );
}

//==============================================================================

xbool matrix4::InvertSRT( void )
{
    matrix4 Src = *this;
    f32     Determinant;

    //
    // Calculate the determinant.
    //

    Determinant = ( Src.M(0,0) * ( Src.M(1,1) * Src.M(2,2) - Src.M(1,2) * Src.M(2,1) ) -
                    Src.M(0,1) * ( Src.M(1,0) * Src.M(2,2) - Src.M(1,2) * Src.M(2,0) ) +
                    Src.M(0,2) * ( Src.M(1,0) * Src.M(2,1) - Src.M(1,1) * Src.M(2,0) ) );

    if( x_abs( Determinant ) < 0.00001f ) 
        return( FALSE );

    Determinant = 1.0f / Determinant;

    //
    // Find the inverse of the matrix.
    //

    M(0,0) =  Determinant * ( Src.M(1,1) * Src.M(2,2) - Src.M(1,2) * Src.M(2,1) );
    M(0,1) = -Determinant * ( Src.M(0,1) * Src.M(2,2) - Src.M(0,2) * Src.M(2,1) );
    M(0,2) =  Determinant * ( Src.M(0,1) * Src.M(1,2) - Src.M(0,2) * Src.M(1,1) );
    M(0,3) = 0.0f;

    M(1,0) = -Determinant * ( Src.M(1,0) * Src.M(2,2) - Src.M(1,2) * Src.M(2,0) );
    M(1,1) =  Determinant * ( Src.M(0,0) * Src.M(2,2) - Src.M(0,2) * Src.M(2,0) );
    M(1,2) = -Determinant * ( Src.M(0,0) * Src.M(1,2) - Src.M(0,2) * Src.M(1,0) );
    M(1,3) = 0.0f;

    M(2,0) =  Determinant * ( Src.M(1,0) * Src.M(2,1) - Src.M(1,1) * Src.M(2,0) );
    M(2,1) = -Determinant * ( Src.M(0,0) * Src.M(2,1) - Src.M(0,1) * Src.M(2,0) );
    M(2,2) =  Determinant * ( Src.M(0,0) * Src.M(1,1) - Src.M(0,1) * Src.M(1,0) );
    M(2,3) = 0.0f;

    M(3,0) = -( Src.M(3,0) * M(0,0) + Src.M(3,1) * M(1,0) + Src.M(3,2) * M(2,0) );
    M(3,1) = -( Src.M(3,0) * M(0,1) + Src.M(3,1) * M(1,1) + Src.M(3,2) * M(2,1) );
    M(3,2) = -( Src.M(3,0) * M(0,2) + Src.M(3,1) * M(1,2) + Src.M(3,2) * M(2,2) );
    M(3,3) = 1.0f;
              
    // Success!
    return( TRUE );
}

//==============================================================================

matrix4 m4_InvertSRT( const matrix4& Src )
{
    matrix4 Dest;
    f32     Determinant;

    //
    // Calculate the determinant.
    //
    Determinant = ( Src.M(0,0) * ( Src.M(1,1) * Src.M(2,2) - Src.M(1,2) * Src.M(2,1) ) -
                    Src.M(0,1) * ( Src.M(1,0) * Src.M(2,2) - Src.M(1,2) * Src.M(2,0) ) +
                    Src.M(0,2) * ( Src.M(1,0) * Src.M(2,1) - Src.M(1,1) * Src.M(2,0) ) );

    ASSERT( x_abs( Determinant ) > 0.00001f );

    Determinant = 1.0f / Determinant;

    //
    // Find the inverse of the matrix.
    //
    Dest.M(0,0) =  Determinant * ( Src.M(1,1) * Src.M(2,2) - Src.M(1,2) * Src.M(2,1) );
    Dest.M(0,1) = -Determinant * ( Src.M(0,1) * Src.M(2,2) - Src.M(0,2) * Src.M(2,1) );
    Dest.M(0,2) =  Determinant * ( Src.M(0,1) * Src.M(1,2) - Src.M(0,2) * Src.M(1,1) );
    Dest.M(0,3) = 0.0f;

    Dest.M(1,0) = -Determinant * ( Src.M(1,0) * Src.M(2,2) - Src.M(1,2) * Src.M(2,0) );
    Dest.M(1,1) =  Determinant * ( Src.M(0,0) * Src.M(2,2) - Src.M(0,2) * Src.M(2,0) );
    Dest.M(1,2) = -Determinant * ( Src.M(0,0) * Src.M(1,2) - Src.M(0,2) * Src.M(1,0) );
    Dest.M(1,3) = 0.0f;

    Dest.M(2,0) =  Determinant * ( Src.M(1,0) * Src.M(2,1) - Src.M(1,1) * Src.M(2,0) );
    Dest.M(2,1) = -Determinant * ( Src.M(0,0) * Src.M(2,1) - Src.M(0,1) * Src.M(2,0) );
    Dest.M(2,2) =  Determinant * ( Src.M(0,0) * Src.M(1,1) - Src.M(0,1) * Src.M(1,0) );
    Dest.M(2,3) = 0.0f;

    Dest.M(3,0) = -( Src.M(3,0) * Dest.M(0,0) + Src.M(3,1) * Dest.M(1,0) + Src.M(3,2) * Dest.M(2,0) );
    Dest.M(3,1) = -( Src.M(3,0) * Dest.M(0,1) + Src.M(3,1) * Dest.M(1,1) + Src.M(3,2) * Dest.M(2,1) );
    Dest.M(3,2) = -( Src.M(3,0) * Dest.M(0,2) + Src.M(3,1) * Dest.M(1,2) + Src.M(3,2) * Dest.M(2,2) );
    Dest.M(3,3) = 1.0f;

    return Dest;            
}

//==============================================================================

quaternion matrix4::GetQuaternion( void ) const
{
    f32 T;
    f32 X2, Y2, Z2, W2;  // squared magniudes of quaternion components
    s32 i;
    quaternion Q;

    // remove scale from matrix
    matrix4 O = *this;
    O.ClearScale();

    // first compute squared magnitudes of quaternion components - at least one
    // will be greater than 0 since quaternion is unit magnitude
    W2 = 0.25f * (O(0,0) + O(1,1) + O(2,2) + 1.0f );
    X2 = W2 - 0.5f * (O(1,1) + O(2,2));
    Y2 = W2 - 0.5f * (O(2,2) + O(0,0));
    Z2 = W2 - 0.5f * (O(0,0) + O(1,1));

    // find maximum magnitude component
    i = (W2 > X2 ) ?
    ((W2 > Y2) ? ((W2 > Z2) ? 0 : 3) : ((Y2 > Z2) ? 2 : 3)) :
    ((X2 > Y2) ? ((X2 > Z2) ? 1 : 3) : ((Y2 > Z2) ? 2 : 3));

    // compute signed quaternion components using numerically stable method
    switch( i ) 
    {
        case 0:
                Q.W = x_sqrt(W2);
                T = 0.25f / Q.W;
                Q.X = (O(1,2) - O(2,1)) * T;
                Q.Y = (O(2,0) - O(0,2)) * T;
                Q.Z = (O(0,1) - O(1,0)) * T;
                break;
        case 1:
                Q.X = x_sqrt(X2);
                T = 0.25f / Q.X;
                Q.W = (O(1,2) - O(2,1)) * T;
                Q.Y = (O(1,0) + O(0,1)) * T;
                Q.Z = (O(2,0) + O(0,2)) * T;
                break;
        case 2:
                Q.Y = x_sqrt(Y2);
                T = 0.25f / Q.Y;
                Q.W = (O(2,0) - O(0,2)) * T;
                Q.Z = (O(2,1) + O(1,2)) * T;
                Q.X = (O(0,1) + O(1,0)) * T;
                break;
        case 3:
                Q.Z = x_sqrt(Z2);
                T = 0.25f / Q.Z;
                Q.W = (O(0,1) - O(1,0)) * T;
                Q.X = (O(0,2) + O(2,0)) * T;
                Q.Y = (O(1,2) + O(2,1)) * T;
                break;
    }

    // for consistency, force positive scalar component
/*
    if( Q.W < 0)
    {
        Q.X = -Q.X;
        Q.Y = -Q.Y;
        Q.Z = -Q.Z;
        Q.W = -Q.W;
    }
*/
    Q.Normalize();
    return Q;
}

//==============================================================================

#if !USE_VU0 && !(defined TARGET_XBOX)
matrix4 operator * ( const matrix4& L, const matrix4& R )
{
    matrix4 Result;
    for( s32 i=0; i<4; i++ )
    {
        Result.M(i,0) = (L.M(0,0)*R.M(i,0)) + (L.M(1,0)*R.M(i,1)) + (L.M(2,0)*R.M(i,2)) + (L.M(3,0)*R.M(i,3));
        Result.M(i,1) = (L.M(0,1)*R.M(i,0)) + (L.M(1,1)*R.M(i,1)) + (L.M(2,1)*R.M(i,2)) + (L.M(3,1)*R.M(i,3));
        Result.M(i,2) = (L.M(0,2)*R.M(i,0)) + (L.M(1,2)*R.M(i,1)) + (L.M(2,2)*R.M(i,2)) + (L.M(3,2)*R.M(i,3));
        Result.M(i,3) = (L.M(0,3)*R.M(i,0)) + (L.M(1,3)*R.M(i,1)) + (L.M(2,3)*R.M(i,2)) + (L.M(3,3)*R.M(i,3));
    }
    return Result;

/*
    matrix4 Result;

    // If the bottom row of both L and R are [0 0 0 1], then we can do a
    // streamlined matrix multiplication.  Otherwise, we must do a full force
    // multiplication.

    if( (L.M(0,3) == 0.0f) && (R.M(0,3) == 0.0f) &&
        (L.M(1,3) == 0.0f) && (R.M(1,3) == 0.0f) && 
        (L.M(2,3) == 0.0f) && (R.M(2,3) == 0.0f) &&
        (L.M(3,3) == 1.0f) && (R.M(3,3) == 1.0f) )
    {
        Result.M(0,0) = (L.M(0,0)*R.M(0,0)) + (L.M(1,0)*R.M(0,1)) + (L.M(2,0)*R.M(0,2));
        Result.M(1,0) = (L.M(0,0)*R.M(1,0)) + (L.M(1,0)*R.M(1,1)) + (L.M(2,0)*R.M(1,2));
        Result.M(2,0) = (L.M(0,0)*R.M(2,0)) + (L.M(1,0)*R.M(2,1)) + (L.M(2,0)*R.M(2,2));
        Result.M(3,0) = (L.M(0,0)*R.M(3,0)) + (L.M(1,0)*R.M(3,1)) + (L.M(2,0)*R.M(3,2)) + L.M(3,0);

        Result.M(0,1) = (L.M(0,1)*R.M(0,0)) + (L.M(1,1)*R.M(0,1)) + (L.M(2,1)*R.M(0,2));
        Result.M(1,1) = (L.M(0,1)*R.M(1,0)) + (L.M(1,1)*R.M(1,1)) + (L.M(2,1)*R.M(1,2));
        Result.M(2,1) = (L.M(0,1)*R.M(2,0)) + (L.M(1,1)*R.M(2,1)) + (L.M(2,1)*R.M(2,2));
        Result.M(3,1) = (L.M(0,1)*R.M(3,0)) + (L.M(1,1)*R.M(3,1)) + (L.M(2,1)*R.M(3,2)) + L.M(3,1);

        Result.M(0,2) = (L.M(0,2)*R.M(0,0)) + (L.M(1,2)*R.M(0,1)) + (L.M(2,2)*R.M(0,2));
        Result.M(1,2) = (L.M(0,2)*R.M(1,0)) + (L.M(1,2)*R.M(1,1)) + (L.M(2,2)*R.M(1,2));
        Result.M(2,2) = (L.M(0,2)*R.M(2,0)) + (L.M(1,2)*R.M(2,1)) + (L.M(2,2)*R.M(2,2));
        Result.M(3,2) = (L.M(0,2)*R.M(3,0)) + (L.M(1,2)*R.M(3,1)) + (L.M(2,2)*R.M(3,2)) + L.M(3,2);

        Result.M(0,3) = 0.0f;
        Result.M(1,3) = 0.0f;
        Result.M(2,3) = 0.0f;
        Result.M(3,3) = 1.0f;
    }
    else
    {
        Result.M(0,0) = (L.M(0,0)*R.M(0,0)) + (L.M(1,0)*R.M(0,1)) + (L.M(2,0)*R.M(0,2)) + (L.M(3,0)*R.M(0,3));
        Result.M(1,0) = (L.M(0,0)*R.M(1,0)) + (L.M(1,0)*R.M(1,1)) + (L.M(2,0)*R.M(1,2)) + (L.M(3,0)*R.M(1,3));
        Result.M(2,0) = (L.M(0,0)*R.M(2,0)) + (L.M(1,0)*R.M(2,1)) + (L.M(2,0)*R.M(2,2)) + (L.M(3,0)*R.M(2,3));
        Result.M(3,0) = (L.M(0,0)*R.M(3,0)) + (L.M(1,0)*R.M(3,1)) + (L.M(2,0)*R.M(3,2)) + (L.M(3,0)*R.M(3,3));

        Result.M(0,1) = (L.M(0,1)*R.M(0,0)) + (L.M(1,1)*R.M(0,1)) + (L.M(2,1)*R.M(0,2)) + (L.M(3,1)*R.M(0,3));
        Result.M(1,1) = (L.M(0,1)*R.M(1,0)) + (L.M(1,1)*R.M(1,1)) + (L.M(2,1)*R.M(1,2)) + (L.M(3,1)*R.M(1,3));
        Result.M(2,1) = (L.M(0,1)*R.M(2,0)) + (L.M(1,1)*R.M(2,1)) + (L.M(2,1)*R.M(2,2)) + (L.M(3,1)*R.M(2,3));
        Result.M(3,1) = (L.M(0,1)*R.M(3,0)) + (L.M(1,1)*R.M(3,1)) + (L.M(2,1)*R.M(3,2)) + (L.M(3,1)*R.M(3,3));

        Result.M(0,2) = (L.M(0,2)*R.M(0,0)) + (L.M(1,2)*R.M(0,1)) + (L.M(2,2)*R.M(0,2)) + (L.M(3,2)*R.M(0,3));
        Result.M(1,2) = (L.M(0,2)*R.M(1,0)) + (L.M(1,2)*R.M(1,1)) + (L.M(2,2)*R.M(1,2)) + (L.M(3,2)*R.M(1,3));
        Result.M(2,2) = (L.M(0,2)*R.M(2,0)) + (L.M(1,2)*R.M(2,1)) + (L.M(2,2)*R.M(2,2)) + (L.M(3,2)*R.M(2,3));
        Result.M(3,2) = (L.M(0,2)*R.M(3,0)) + (L.M(1,2)*R.M(3,1)) + (L.M(2,2)*R.M(3,2)) + (L.M(3,2)*R.M(3,3));

        Result.M(0,3) = (L.M(0,3)*R.M(0,0)) + (L.M(1,3)*R.M(0,1)) + (L.M(2,3)*R.M(0,2)) + (L.M(3,3)*R.M(0,3));
        Result.M(1,3) = (L.M(0,3)*R.M(1,0)) + (L.M(1,3)*R.M(1,1)) + (L.M(2,3)*R.M(1,2)) + (L.M(3,3)*R.M(1,3));
        Result.M(2,3) = (L.M(0,3)*R.M(2,0)) + (L.M(1,3)*R.M(2,1)) + (L.M(2,3)*R.M(2,2)) + (L.M(3,3)*R.M(2,3));
        Result.M(3,3) = (L.M(0,3)*R.M(3,0)) + (L.M(1,3)*R.M(3,1)) + (L.M(2,3)*R.M(3,2)) + (L.M(3,3)*R.M(3,3));
    }

    return( Result );
*/
}
#endif

//==============================================================================
xbool bbox::Intersect( const vector3* pVert,
                             s32      nVerts ) const
{
    // Build planes
    plane   BoxPlane[6];
    BoxPlane[0].Setup( Min, vector3(1,0,0) );
    BoxPlane[1].Setup( Min, vector3(0,1,0) );
    BoxPlane[2].Setup( Min, vector3(0,0,1) );
    BoxPlane[3].Setup( Max, vector3(-1,0,0) );
    BoxPlane[4].Setup( Max, vector3(0,-1,0) );
    BoxPlane[5].Setup( Max, vector3(0,0,-1) );

    // Check if bboxes meet
    bbox NGonBBox;
    NGonBBox.AddVerts( pVert, nVerts );

    if( !NGonBBox.Intersect( *this ) )
        return FALSE;

    // Clip ngon to bbox
    vector3 Vert0[64];
    vector3 Vert1[64];
    s32     nVerts0;
    s32     nVerts1;

    BoxPlane[0].ClipNGon( Vert0, nVerts0, pVert, nVerts );
    BoxPlane[1].ClipNGon( Vert1, nVerts1, Vert0, nVerts0 );
    BoxPlane[2].ClipNGon( Vert0, nVerts0, Vert1, nVerts1 );
    BoxPlane[3].ClipNGon( Vert1, nVerts1, Vert0, nVerts0 );
    BoxPlane[4].ClipNGon( Vert0, nVerts0, Vert1, nVerts1 );
    BoxPlane[5].ClipNGon( Vert1, nVerts1, Vert0, nVerts0 );

    ASSERT( nVerts1 <= 64 );

    if( nVerts1 == 0 )
        return FALSE;

    return TRUE;
}

//==============================================================================
//  CLEAR DEFINES
//==============================================================================

#undef M

//==============================================================================

