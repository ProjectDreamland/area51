//==============================================================================
//
//  x_math_p_inline.hpp
//
//==============================================================================

#ifndef X_MATH_P_INLINE_HPP
#define X_MATH_P_INLINE_HPP
#else
#error "File " __FILE__ " has been included twice!"
#endif

#ifndef X_DEBUG_HPP
#include "../x_debug.hpp"
#endif

//==============================================================================
//  FUNCTIONS
//==============================================================================

inline
void plane::Setup( f32 A, f32 B, f32 C, f32 aD )
{
    FORCE_ALIGNED_16( this );

    Normal.Set( A, B, C );
    D          = aD;

    f32 Len = Normal.LengthSquared();
    if( !IN_RANGE( 0.9999f, Len, 1.0001f ) )
    {
        f32 Factor = x_1sqrt(Len);
        Normal.Scale( Factor );
        D *= Factor;
    }
}

//==============================================================================

inline
void plane::Setup( const vector3& P1, const vector3& P2, const vector3& P3 )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &P1 );
    FORCE_ALIGNED_16( &P2 );
    FORCE_ALIGNED_16( &P3 );

    Normal = v3_Cross( P2-P1, P3-P1 );
    Normal.SafeNormalize();
    D = -Normal.Dot( P1 );
}

//==============================================================================

inline
void plane::Setup( const vector3& aNormal, f32 aDistance )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &aNormal );

    Normal = aNormal;
    D      = aDistance;

    f32 Len = Normal.LengthSquared();
    if( !IN_RANGE( 0.9999f, Len, 1.0001f ) )
    {
        f32 Factor = x_1sqrt(Len);
        Normal.Scale( Factor );
        D *= Factor;
    }
}

//==============================================================================

inline
void plane::Setup( const vector3& P, const vector3& aNormal )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &P );
    FORCE_ALIGNED_16( &Normal );

    Normal = aNormal;

    f32 Len = Normal.LengthSquared();
    if( !IN_RANGE( 0.9999f, Len, 1.0001f ) )
        Normal.Scale( x_1sqrt(Len) );

    D = -Normal.Dot( P );
}

//==============================================================================

inline 
plane::plane( void )
{
    FORCE_ALIGNED_16( this );
}

//==============================================================================

inline 
plane::plane( const vector3& P1, const vector3& P2, const vector3& P3 )
{
    FORCE_ALIGNED_16( this );
    
    Setup( P1, P2, P3 );
}

//==============================================================================

inline 
plane::plane( const vector3& aNormal, f32 aDistance )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &aNormal );

    Setup( aNormal, aDistance );
}

//==============================================================================

inline 
plane::plane( f32 A, f32 B, f32 C, f32 aD )
{
    FORCE_ALIGNED_16( this );

    Setup( A, B, C, aD );
}

//==============================================================================

inline
void plane::Negate( void )
{
    FORCE_ALIGNED_16( this );

    Normal.Negate();
    D      = -D;
}

//==============================================================================

inline
void plane::Transform( const matrix4& M )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &M );

    (*this) = M * (*this);

    /*
    vector3 Axis1, Axis2;
    vector3 P[3];

    GetOrthoVectors( Axis1, Axis2 );

    P[0] = Normal * -D;

    // Scale up D
    if( x_abs(D) > 1.0f )
    {
        Axis1 *= D; // Scaling up by D although it's unecessary to 
        Axis2 *= D; // help get P1,P2 around same magnitude as P0
    }

    P[1] = P[0] + Axis1;  
    P[2] = P[0] + Axis2;  

    M.Transform( P, P, 3 );

    Setup( P[0], P[1], P[2] );
    */
}

//==============================================================================

inline
void plane::GetOrthoVectors ( vector3& AxisA,
                              vector3& AxisB ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &AxisA );
    FORCE_ALIGNED_16( &AxisB );

    f32     AbsA, AbsB, AbsC;
    vector3 Dir;

    // Get a non-parallel axis to normal.
    AbsA = x_abs( Normal.GetX() );
    AbsB = x_abs( Normal.GetY() );
    AbsC = x_abs( Normal.GetZ() );
    if( (AbsA<=AbsB) && (AbsA<=AbsC) ) Dir = vector3(1,0,0);
    else
    if( (AbsB<=AbsA) && (AbsB<=AbsC) ) Dir = vector3(0,1,0);
    else                               Dir = vector3(0,0,1);

    AxisA = Normal.Cross(Dir);
    AxisB = Normal.Cross(AxisA);
    AxisA.Normalize();
    AxisB.Normalize();
}

//==============================================================================

inline 
f32 plane::Dot( const vector3& P ) const  
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &P );

    return( P.Dot(Normal) );
}

//==============================================================================

inline 
f32 plane::Distance( const vector3& P ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &P );

    return( P.Dot(Normal) + D );
}

//==============================================================================

inline 
xbool plane::InFront( const vector3& P ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &P );

    return( (P.Dot(Normal) + D) >= 0.0f );
}

//==============================================================================

inline 
xbool plane::InBack( const vector3& P ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &P );

    return( (P.Dot(Normal) + D) < 0.0f );
}

//==============================================================================

inline 
xbool plane::Intersect( f32& t, const vector3& P0, const vector3& P1 ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &P0 );
    FORCE_ALIGNED_16( &P1 );

    t = (P1 - P0).Dot( Normal );

    if( t == 0.0f ) 
        return( FALSE );
    else    
    {
        t = -Distance( P0 ) / t;
        return( TRUE );
    }
}

//==============================================================================

inline
void plane::GetComponents( const vector3& V,
                                 vector3& Parallel,
                                 vector3& Perpendicular ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );
    FORCE_ALIGNED_16( &Parallel );
    FORCE_ALIGNED_16( &Perpendicular );

    Perpendicular = Normal.Dot(V) * Normal;
    Parallel      = V - Perpendicular;
}

//==============================================================================

inline
vector3 plane::ReflectVector( const vector3& V ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &V );

    return( V - (Normal.Dot(V)*2*Normal) );
}

//==============================================================================

inline
xbool plane::ClipNGon(       vector3* pDst, s32& NDstVerts, 
                       const vector3* pSrc, s32  NSrcVerts ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( pDst );
    FORCE_ALIGNED_16( pSrc );

    f32   D0, D1;
    s32   P0, P1;
    xbool Clipped = FALSE;

    NDstVerts = 0;
    P1 = NSrcVerts-1;
    D1 = Distance( pSrc[P1] );

    for( s32 i=0; i<NSrcVerts; i++ ) 
    {
        P0 = P1;
        D0 = D1;
        P1 = i;
        D1 = Distance(pSrc[P1]);

        // Do we keep starting vert?
        if( D0 >= 0 )
        {
            pDst[NDstVerts++] = pSrc[P0];
        }

        // Do we need to compute intersection?
        if( ((D0>=0)&&(D1<0)) || ((D0<0)&&(D1>=0)) )
        {
            f32 d = (D1-D0);
            if( x_abs(d) < 0.00001f )  d = 0.00001f;
            f32 t = (0-D0) / d;
            pDst[NDstVerts++] = pSrc[P0] + t*(pSrc[P1]-pSrc[P0]);
            Clipped = TRUE;
        }
    }

    return( Clipped );
}

//==============================================================================

inline
void plane::GetBBoxIndices  ( s32* pMinIndices, s32* pMaxIndices ) const
{
    FORCE_ALIGNED_16( this );

    if( pMinIndices )
    {
        if( Normal.GetX() >= 0 ) pMinIndices[0] = 0;
        else                     pMinIndices[0] = 4;
                                                                        
        if( Normal.GetY() >= 0 ) pMinIndices[1] = 1;
        else                     pMinIndices[1] = 5;
                                                                      
        if( Normal.GetZ() >= 0 ) pMinIndices[2] = 2;
        else                     pMinIndices[2] = 6;
    }

    if( pMaxIndices )
    {
        if( Normal.GetX() >= 0 ) pMaxIndices[0] = 4;
        else                     pMaxIndices[0] = 0;
                                               
        if( Normal.GetY() >= 0 ) pMaxIndices[1] = 5;
        else                     pMaxIndices[1] = 1;
                                               
        if( Normal.GetZ() >= 0 ) pMaxIndices[2] = 6;
        else                     pMaxIndices[2] = 2;
    }
}

//==============================================================================

inline
void plane::ComputeD( const vector3& P )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &P );

    D = -Normal.Dot( P );
}

//==============================================================================

inline 
plane operator * ( const matrix4& M, const plane& Plane )
{
    FORCE_ALIGNED_16( &M );
    FORCE_ALIGNED_16( &Plane );

    matrix4     Adjoint;
    plane       NewPlane;
    vector3     V;

    // Transform a point in the plane by M
    V = M * ( Plane.Normal * -Plane.D );

    // Compute the Transpouse of the Inverse of the Matrix
    Adjoint.Identity();
    Adjoint.SetColumns( vector3( M(1,1) * M(2,2) - M(1,2) * M(2,1),
                                 M(1,2) * M(2,0) - M(1,0) * M(2,2),
                                 M(1,0) * M(2,1) - M(1,1) * M(2,0) ),
                        vector3( M(2,1) * M(0,2) - M(2,2) * M(0,1),
                                 M(2,2) * M(0,0) - M(2,0) * M(0,2),
                                 M(2,0) * M(0,1) - M(2,1) * M(0,0) ),
                        vector3( M(0,1) * M(1,2) - M(0,2) * M(1,1),
                                 M(0,2) * M(1,0) - M(0,0) * M(1,2),
                                 M(0,0) * M(1,1) - M(0,1) * M(1,0) ) );

    // Transform the normal
    NewPlane.Normal = Adjoint.RotateVector( Plane.Normal );

    // Renormalize
    NewPlane.Normal.Normalize();

    // Recompute D by in the transform point
    NewPlane.ComputeD( V );

    return NewPlane;
}

//==============================================================================

inline
f32 plane::Difference( const plane& P ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &P );

    vector3 N = Normal - P.Normal;
    return( N.Dot(N) + (D-P.D)*(D-P.D) );
}

//==============================================================================

inline
xbool plane::InRange( f32 Min, f32 Max ) const
{
    FORCE_ALIGNED_16( this );

    return( (Normal.GetX()>=-1.0f) && (Normal.GetX()<=1.0f) &&
            (Normal.GetY()>=-1.0f) && (Normal.GetY()<=1.0f) &&
            (Normal.GetZ()>=-1.0f) && (Normal.GetZ()<=1.0f) &&
            (D>=Min) && (D<=Max) );
}

//==============================================================================

inline
xbool plane::IsValid( void ) const
{
    FORCE_ALIGNED_16( this );

    return( Normal.IsValid() && x_isvalid(D) );
}

//==============================================================================



