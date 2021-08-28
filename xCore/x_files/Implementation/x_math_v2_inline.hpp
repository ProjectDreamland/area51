//==============================================================================
//
//  x_math_v2_inline.hpp
//
//==============================================================================

#ifndef X_MATH_V2_INLINE_HPP
#define X_MATH_V2_INLINE_HPP
#else
#error "File " __FILE__ " has been included twice!"
#endif

#ifndef X_DEBUG_HPP
#include "../x_debug.hpp"
#endif

//==============================================================================

inline 
vector2::vector2( void )
{
}

//==============================================================================

inline 
vector2::vector2( const vector2& V )
{
    X = V.X; 
    Y = V.Y;
}

//==============================================================================

inline 
vector2::vector2( f32 aX, f32 aY )
{
    X = aX; 
    Y = aY;
}

//==============================================================================

inline 
vector2::vector2( const radian Angle )
{
    X = x_cos( Angle );
    Y = x_sin( Angle );
}


//==============================================================================

inline 
void vector2::Set( f32 aX, f32 aY )
{
    X = aX; 
    Y = aY;
}

//==============================================================================

inline 
void vector2::operator () ( f32 aX, f32 aY )
{
    X = aX; 
    Y = aY;
}

//==============================================================================

inline
f32 vector2::operator [] ( s32 Index ) const
{
    ASSERT( (Index >= 0) && (Index < 2) );
    return( ((f32*)this)[Index] );
}

//==============================================================================

inline
f32& vector2::operator [] ( s32 Index )
{
    ASSERT( (Index >= 0) && (Index < 2) );
    return( ((f32*)this)[Index] );
}

//==============================================================================

inline 
void vector2::Zero( void )
{
    X = Y = 0.0f;
}

//==============================================================================

inline 
void vector2::Negate( void )
{
    X = -X; 
    Y = -Y;
}

//==============================================================================

inline 
xbool vector2::Normalize( void )
{
    f32 N = x_1sqrt(X*X + Y*Y);

    if( x_isvalid(N) )
    {
        X *= N;
        Y *= N;
        return TRUE;
    }
    else
    {
        X = 0;
        Y = 0;
        return FALSE;
    }
}

//==============================================================================

inline 
xbool vector2::NormalizeAndScale( f32 Scalar )
{
    f32 N = x_1sqrt(X*X + Y*Y);

    if( x_isvalid(N) )
    {
        N *= Scalar;
        X *= N;
        Y *= N;
        return TRUE;
    }
    else
    {
        X = 0;
        Y = 0;
        return FALSE;
    }
}

//==============================================================================

inline 
void vector2::Scale( f32 Scalar )
{
    X *= Scalar; 
    Y *= Scalar;
}

//==============================================================================

inline 
f32 vector2::Length( void ) const
{
    return( x_sqrt( X*X + Y*Y ) );
}

//==============================================================================

inline
f32 vector2::LengthSquared( void ) const
{
    return( X*X + Y*Y );
}

//==============================================================================

inline 
f32 vector2::Angle( void ) const
{
    return( x_atan2( Y, X ) );
}

//==============================================================================

inline 
void vector2::Rotate( radian Angle )
{
    radian  S, C; 
    f32     x, y;

    x_sincos( Angle, S, C );

    x = X;
    y = Y;
    X = (C * x) - (S * y);
    Y = (C * y) + (S * x);
}

//==============================================================================

inline 
f32 vector2::Dot( const vector2& V )
{
    return( (X * V.X) + (Y * V.Y) );
}

//==============================================================================

inline 
f32 v2_Dot( const vector2& V1, const vector2& V2 )
{
    return( (V1.X * V2.X) + (V1.Y * V2.Y) );
}

//==============================================================================

inline
vector2::operator const f32* ( void ) const
{
    return( &X );
}

//==============================================================================

inline 
vector2 vector2::operator - ( void ) const
{
    return( vector2( -X, -Y ) );
}

//==============================================================================

inline 
const vector2& vector2::operator = ( const vector2& V )
{
    X = V.X; 
    Y = V.Y;
    return( *this );
}

//==============================================================================

inline 
vector2& vector2::operator += ( const vector2& V )
{
    X += V.X;
    Y += V.Y;
    return( *this );
}

//==============================================================================

inline 
vector2& vector2::operator -= ( const vector2& V )
{
    X -= V.X;
    Y -= V.Y;
    return( *this );
}

//==============================================================================

inline 
vector2& vector2::operator *= ( const f32 Scalar )
{
    X *= Scalar; 
    Y *= Scalar;
    return( *this );
}

//==============================================================================

inline 
vector2& vector2::operator /= ( const f32 Scalar )
{
    ASSERT( Scalar != 0.0f );
    f32 d = 1.0f / Scalar;
    X *= d; 
    Y *= d; 
    return( *this );
}

//==============================================================================

inline 
xbool vector2::operator == ( const vector2& V ) const
{
    return( (X == V.X) && (Y == V.Y) );
}

//==============================================================================

inline 
xbool vector2::operator != ( const vector2& V ) const
{
    return( (X != V.X) || (Y != V.Y) );
}

//==============================================================================

inline 
vector2 operator + ( const vector2& V1, const vector2& V2 )
{
    return( vector2( V1.X + V2.X, V1.Y + V2.Y ) );
}

//==============================================================================

inline 
vector2 operator - ( const vector2& V1, const vector2& V2 )
{
    return( vector2( V1.X - V2.X, V1.Y - V2.Y ) );
}

//==============================================================================

inline 
vector2 operator / ( const vector2& V, f32 S )
{
    ASSERT( (S > 0.00001f) || (S < -0.00001f) );
    S = 1.0f / S;
    return( vector2( V.X * S, V.Y * S ) );
}

//==============================================================================

inline 
vector2 operator * ( const vector2& V, f32 S )
{
    return( vector2( V.X * S, V.Y * S ) );
}

//==============================================================================

inline 
vector2 operator * ( f32 S, const vector2& V )
{
    return( vector2( V.X * S, V.Y * S ) );
}

//==============================================================================

inline
f32 vector2::Difference( const vector2& V ) const
{
    return (X-V.X)*(X-V.X) + (Y-V.Y)*(Y-V.Y);
}

//==============================================================================

inline
xbool vector2::InRange( f32 aMin, f32 aMax ) const
{
    return( (X>=aMin) && (X<=aMax) &&
            (Y>=aMin) && (Y<=aMax) );
}

//==============================================================================

inline
xbool vector2::IsValid( void ) const
{
    return( x_isvalid(X) && x_isvalid(Y) );
}

//==============================================================================
