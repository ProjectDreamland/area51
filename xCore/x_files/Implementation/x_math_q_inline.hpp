//==============================================================================
//
//  x_math_q_inline.hpp
//
//==============================================================================

#ifndef X_MATH_Q_INLINE_HPP
#define X_MATH_Q_INLINE_HPP
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
quaternion::quaternion( void ) 
{
}

//==============================================================================

inline
quaternion::quaternion( const quaternion& Q )
{
    X = Q.X;
    Y = Q.Y;
    Z = Q.Z;
    W = Q.W;

    ASSERT( IN_RANGE( (0.95f * 0.95f), (X*X + Y*Y + Z*Z + W*W), (1.05f * 1.05f) ) );
}

//==============================================================================

inline
quaternion::quaternion( const radian3& R )
{
    Setup( R );
}

//==============================================================================

inline
quaternion::quaternion( const vector3& Axis, radian Angle )
{
    FORCE_ALIGNED_16( &Axis );

    Setup( Axis, Angle );
}

//==============================================================================

inline 
quaternion::quaternion( const matrix4&    M )
{
    FORCE_ALIGNED_16( &M );

    Setup(M);
}

//==============================================================================

inline
quaternion::quaternion( f32 aX, f32 aY, f32 aZ, f32 aW )
{
    X = aX;
    Y = aY;
    Z = aZ;
    W = aW;

    ASSERT( IN_RANGE( (0.95f * 0.95f), (X*X + Y*Y + Z*Z + W*W), (1.05f * 1.05f) ) );
}

//==============================================================================

inline
void quaternion::Identity( void )
{
    X = 0.0f;
    Y = 0.0f;
    Z = 0.0f;
    W = 1.0f;
}

//==============================================================================

inline
void quaternion::Invert( void )
{
    X = -X ;
    Y = -Y ;
    Z = -Z ;
}

//==============================================================================

inline
void quaternion::Normalize( void )
{
    f32 S = x_1sqrt( X*X + Y*Y + Z*Z + W*W );

    X *= S;
    Y *= S;
    Z *= S;
    W *= S;
}

//==============================================================================

inline
void quaternion::Setup( const radian3& R )
{
    Identity();
    RotateZ( R.Roll  );
    RotateX( R.Pitch );
    RotateY( R.Yaw   );
}

//==============================================================================

inline
void quaternion::Setup( const matrix4& M )
{
    FORCE_ALIGNED_16( &M );

    *this = M.GetQuaternion();
}

//==============================================================================

inline
void quaternion::Setup( const vector3& Axis, radian Angle )
{
    FORCE_ALIGNED_16( &Axis );

    f32 Sine, Cosine;

    x_sincos( Angle * 0.5f, Sine, Cosine );

    W = Cosine;           // cos( Angle/2 )
    X = Sine * Axis.GetX();  // sin( Angle/2 ) * X
    Y = Sine * Axis.GetY();  // sin( Angle/2 ) * Y
    Z = Sine * Axis.GetZ();  // sin( Angle/2 ) * Z
}

//==============================================================================

inline
vector3 quaternion::GetAxis( void ) const
{
    vector3 Axis;
    f32     Cosine;     // cos( Angle/2 )
    f32     Sine;       // sin( Angle/2 )

    Cosine = W;

    if( Cosine >  1.0f )    Cosine =  1.0f;
    if( Cosine < -1.0f )    Cosine = -1.0f;

    Sine = x_sin( x_acos( Cosine ) );

    if( (Sine > -0.00001f) && (Sine < 0.00001f) ) 
    {
        Axis.GetX() = 0.0f;
        Axis.GetY() = 0.0f;
        Axis.GetZ() = 0.0f;
    }
    else 
    {
        Sine = 1.0f / Sine;

        Axis.GetX() = X * Sine;
        Axis.GetY() = Y * Sine;
        Axis.GetZ() = Z * Sine;        
    }

    return( Axis );
}

//==============================================================================

inline
radian quaternion::GetAngle( void ) const
{
    f32 Cosine;     // cos( Angle/2 )

    Cosine = W;

    if( Cosine >  1.0f )    Cosine =  1.0f;
    if( Cosine < -1.0f )    Cosine = -1.0f;

    return( x_acos( Cosine ) * 2.0f );
}

//==============================================================================

inline
radian3 quaternion::GetRotation( void ) const
{
    f32 tx  = 2.0f * X;   // 2x
    f32 ty  = 2.0f * Y;   // 2y
    f32 tz  = 2.0f * Z;   // 2z
    f32 txw =   tx * W;   // 2x * w
    f32 tyw =   ty * W;   // 2y * w
    f32 tzw =   tz * W;   // 2z * w
    f32 txx =   tx * X;   // 2x * x
    f32 tyx =   ty * X;   // 2y * x
    f32 tzx =   tz * X;   // 2z * x
    f32 tyy =   ty * Y;   // 2y * y
    f32 tzy =   tz * Y;   // 2z * y
    f32 tzz =   tz * Z;   // 2z * z
                                
    radian  Pitch, Yaw, Roll;
    f32     s;

    // Get pitch (Rx).
    
    s = tzy - txw;    
    if( s >  1.0f )  s =  1.0f;
    if( s < -1.0f )  s = -1.0f;
    Pitch = x_asin( -s );

    // Get yaw (Ry) and roll (Rz).
    
    if( (Pitch > -R_89) || (Pitch < R_89) )
    {
        Yaw  = x_atan2( tzx + tyw, 1.0f-(txx+tyy) );
        Roll = x_atan2( tyx + tzw, 1.0f-(txx+tzz) );
    }
    else
    {
        Yaw  = 0.0f;
        Roll = x_atan2( tzx - tyw, 1.0f-(tyy+tzz) );
    }

    return( radian3( Pitch, Yaw, Roll ) );
}

//==============================================================================

inline
void quaternion::RotateX( radian Rx )
{
    f32 s, c;
    x_sincos( Rx/2, s, c );
    quaternion Q( s, 0, 0, c );

    *this = Q * *this;
}

//==============================================================================

inline
void quaternion::RotateY( radian Ry )
{
    f32 s, c;
    x_sincos( Ry/2, s, c );
    quaternion Q( 0, s, 0, c );

    *this = Q * *this;
}

//==============================================================================

inline
void quaternion::RotateZ( radian Rz )
{
    f32 s, c;
    x_sincos( Rz/2, s, c );
    quaternion Q( 0, 0, s, c );

    *this = Q * *this;
}

//==============================================================================

inline
void quaternion::PreRotateX( radian Rx )
{
    f32 s, c;
    x_sincos( Rx/2, s, c );
    quaternion Q( s, 0, 0, c );

    *this *= Q;
}

//==============================================================================

inline
void quaternion::PreRotateY( radian Ry )
{
    f32 s, c;
    x_sincos( Ry/2, s, c );
    quaternion Q( 0, s, 0, c );

    *this *= Q;
}

//==============================================================================

inline
void quaternion::PreRotateZ( radian Rz )
{
    f32 s, c;
    x_sincos( Rz/2, s, c );
    quaternion Q( 0, 0, s, c );

    *this *= Q;
}

//==============================================================================
//
//  The "quaternion * vector3" operation uses 19 multiplications and 12 
//  additions.  See Eqn (20) of "A Comparison of Transforms and Quaternions in 
//  Robotics", Funda and Paul, Proceedings of International Conference on 
//  Robotics and Automation, 1988, p. 886-991.
//
//==============================================================================

inline
vector3 quaternion::operator * ( const vector3& V ) const
{
    FORCE_ALIGNED_16( &V );

    vector3 Result;
    vector3 v1;
    vector3 v2;

    Result.Set( X, Y, Z );
    v1     = v3_Cross( Result, V  );
    v2     = v3_Cross( Result, v1 );
    v1    *= 2.0f * W;
    v2    *= 2.0f;
    Result = V + v1 + v2;

    return( Result );
}

//==============================================================================

inline
vector3 quaternion::Rotate( const vector3& V ) const
{
    FORCE_ALIGNED_16( &V );

    return( *this * V );
}

//==============================================================================

inline
void quaternion::Rotate(       vector3* pDest, 
                         const vector3* pSource, 
                               s32      NVerts ) const
{
    FORCE_ALIGNED_16( pDest );
    FORCE_ALIGNED_16( pSource );

    s32 i;

    for( i = 0; i < NVerts; i++ )
    {
        *pDest = *this * *pSource;
        pDest++;
        pSource++;
    }
}

//==============================================================================

inline
const quaternion& quaternion::operator = ( const quaternion& Q )
{
    X = Q.X;
    Y = Q.Y;
    Z = Q.Z;
    W = Q.W;

    return( *this );
}

//==============================================================================

inline
quaternion& quaternion::operator *= ( const quaternion& R )
{
    quaternion L( *this );

    X = (L.W * R.X) + (R.W * L.X) + (L.Y * R.Z) - (L.Z * R.Y);
    Y = (L.W * R.Y) + (R.W * L.Y) + (L.Z * R.X) - (L.X * R.Z);
    Z = (L.W * R.Z) + (R.W * L.Z) + (L.X * R.Y) - (L.Y * R.X);
                                                             
    W = (L.W * R.W) - (R.X * L.X) - (L.Y * R.Y) - (L.Z * R.Z);

    return( *this );
}

//==========================================================================

inline 
quaternion operator * ( const quaternion& L, const quaternion& R )
{
    quaternion Q;

    Q.X = (L.W * R.X) + (R.W * L.X) + (L.Y * R.Z) - (L.Z * R.Y);
    Q.Y = (L.W * R.Y) + (R.W * L.Y) + (L.Z * R.X) - (L.X * R.Z);
    Q.Z = (L.W * R.Z) + (R.W * L.Z) + (L.X * R.Y) - (L.Y * R.X);
                                                               
    Q.W = (L.W * R.W) - (R.X * L.X) - (L.Y * R.Y) - (L.Z * R.Z);

    return( Q );
}

//==============================================================================

inline
void quaternion::Setup( const vector3& StartDir, const vector3& EndDir )
{
    FORCE_ALIGNED_16( &StartDir );
    FORCE_ALIGNED_16( &EndDir );

    vector3 Axis;
    radian Angle;

    StartDir.GetRotationTowards( EndDir, Axis, Angle );
    Setup( Axis, Angle );
}

//==============================================================================

inline
f32 quaternion::Difference( const quaternion& Q ) const
{
    return (X-Q.X)*(X-Q.X) + (Y-Q.Y)*(Y-Q.Y) + (Z-Q.Z)*(Z-Q.Z) + (W-Q.W)*(W-Q.W);
};

//==============================================================================

inline
xbool quaternion::InRange( void ) const
{
    return ( (X>=-1.0f) && (X<=1.0f) &&
             (Y>=-1.0f) && (Y<=1.0f) &&
             (Z>=-1.0f) && (Z<=1.0f) &&
             (W>=-1.0f) && (W<=1.0f) );
};

//==============================================================================

inline
xbool quaternion::IsValid( void ) const
{
    return( x_isvalid(X) && x_isvalid(Y) && x_isvalid(Z) && x_isvalid(W) );
};

//==============================================================================
