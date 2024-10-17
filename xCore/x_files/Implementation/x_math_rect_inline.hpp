//==============================================================================
//
//  x_math_rect_inline.hpp
//
//==============================================================================

#ifndef X_MATH_RECT_INLINE_HPP
#define X_MATH_RECT_INLINE_HPP
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
rect::rect( void )
{
}

//==============================================================================

inline
rect::rect( const rect& Rect )
{
    Min = Rect.Min;
    Max = Rect.Max;
}

//==============================================================================

inline
rect::rect( const vector2& Min, const vector2& Max )
{
    Set( Min, Max );
}

//==============================================================================

inline
rect::rect( f32 l, f32 t, f32 r, f32 b )
{
    Set( l, t, r, b );
}

//==============================================================================

inline
void rect::Set( const vector2& aMin, const vector2& aMax )
{
    Min = aMin;
    Max = aMax;
}

//==============================================================================

inline
void rect::Set( f32 l, f32 t, f32 r, f32 b )
{
    Min.X = l;
    Min.Y = t;
    Max.X = r;
    Max.Y = b;
}

//==============================================================================

inline
void rect::Clear( void )
{
    Min(  F32_MAX,  F32_MAX );
    Max( -F32_MAX, -F32_MAX );
}

//==============================================================================

inline
xbool rect::Intersect( const rect& Rect )
{
    if( Min.X > Rect.Max.X )  return( FALSE );
    if( Min.Y > Rect.Max.Y )  return( FALSE );
    if( Max.X < Rect.Min.X )  return( FALSE );
    if( Max.Y < Rect.Min.Y )  return( FALSE );

    return( TRUE );
}

//==============================================================================

inline
xbool rect::Intersect( rect& R, const rect& Rect )
{
    if( Intersect( Rect ) == FALSE )
        return( FALSE );
        
    R.Min.X = MAX( Min.X, Rect.Min.X );
    R.Min.Y = MAX( Min.Y, Rect.Min.Y );
    R.Max.X = MIN( Max.X, Rect.Max.X );
    R.Max.Y = MIN( Max.Y, Rect.Max.Y );

    return( TRUE );
}

//==============================================================================
inline
void rect::AddPoint( const vector2& Point )
{
    Min.X = MIN( Min.X, Point.X );
    Min.Y = MIN( Min.Y, Point.Y );
    Max.X = MAX( Max.X, Point.X );
    Max.Y = MAX( Max.Y, Point.Y );
}

//==============================================================================
inline
void rect::AddRect( const rect& Rect )
{
    Min.X = MIN( Min.X, Rect.Min.X );
    Min.Y = MIN( Min.Y, Rect.Min.Y );
    Max.X = MAX( Max.X, Rect.Max.X );
    Max.Y = MAX( Max.Y, Rect.Max.Y );
}

//==============================================================================
inline
f32 rect::GetWidth( void ) const
{
    return Max.X - Min.X;
}

//==============================================================================
inline
f32 rect::GetHeight( void ) const
{
    return Max.Y - Min.Y;
}

//==============================================================================
inline
vector2 rect::GetSize( void ) const
{
    return Max - Min;
}

//==============================================================================
inline
vector2 rect::GetCenter( void ) const
{
    return (Min + Max) / 2.0f;
}

//==============================================================================
inline
void rect::SetWidth( f32 W )
{
    Max.X = Min.X + W;
}

//==============================================================================
inline
void rect::SetHeight( f32 H )
{
    Max.Y = Min.Y + H;
}

//==============================================================================
inline
void rect::SetSize( const vector2& S )
{
    Max.X = Min.X + S.X;
    Max.Y = Min.Y + S.Y;
}

//==============================================================================
inline
void rect::Translate( const vector2& T )
{
    Min += T;
    Max += T;
}

//==============================================================================
inline
void rect::Inflate( const vector2& V )
{
    Min.X -= V.X;
    Max.X += V.X;
    Min.Y -= V.Y;
    Max.Y += V.Y;
}

//==============================================================================
inline
void rect::Deflate( const vector2& V )
{
    Min.X += V.X;
    Max.X -= V.X;
    Min.Y += V.Y;
    Max.Y -= V.Y;
}

//==============================================================================

inline
f32 rect::Difference( const rect& R ) const
{
    return Min.Difference(R.Min) + Max.Difference(R.Max);
}

//==============================================================================

inline
xbool rect::InRange( f32 aMin, f32 aMax ) const
{
    return( (Min.X>=aMin) && (Min.X<=aMax) &&
            (Min.Y>=aMin) && (Min.Y<=aMax) &&
            (Max.X>=aMin) && (Max.X<=aMax) &&
            (Max.Y>=aMin) && (Max.Y<=aMax) );
}

//==============================================================================

inline
xbool rect::IsValid( void ) const
{
    return( Min.IsValid() && Max.IsValid() );
}

//==============================================================================
//  irect
//==============================================================================

inline
irect::irect( void )
{
}

//==============================================================================

inline
irect::irect( const irect& Rect )
{
    l = Rect.l;
    t = Rect.t;
    r = Rect.r;
    b = Rect.b;
}

//==============================================================================

inline
irect::irect( s32 al, s32 at, s32 ar, s32 ab )
{
    Set( al, at, ar, ab );
}

//==============================================================================

inline
void irect::Set( s32 al, s32 at, s32 ar, s32 ab )
{
    l = al;
    t = at;
    r = ar;
    b = ab;
}

//==============================================================================

inline
void irect::Clear( void )
{
    l =  S32_MAX;
    t =  S32_MAX;
    r = -S32_MAX;
    b = -S32_MAX;
}

//==============================================================================

inline
xbool irect::Intersect( const irect& Rect )
{
    if( l > Rect.r )  return( FALSE );
    if( t > Rect.b )  return( FALSE );
    if( r < Rect.l )  return( FALSE );
    if( b < Rect.t )  return( FALSE );

    return( TRUE );
}

//==============================================================================

inline
xbool irect::Intersect( irect& R, const irect& Rect )
{
    if( Intersect( Rect ) == FALSE )
        return( FALSE );
        
    R.l = MAX( l, Rect.l );
    R.t = MAX( t, Rect.t );
    R.r = MIN( r, Rect.r );
    R.b = MIN( b, Rect.b );

    return( TRUE );
}

inline
xbool irect::PointInRect( s32 X, s32 Y)
{
    return ((X >= l) && (X <= r) && (Y >= t) && (Y <= b));
}
//==============================================================================
inline
void irect::AddPoint( s32 X, s32 Y )
{
    l = MIN( l, X );
    t = MIN( t, Y );
    r = MAX( r, X );
    b = MAX( b, Y );
}

//==============================================================================
inline
void irect::AddRect( const irect& Rect )
{
    l = MIN( l, Rect.l );
    t = MIN( t, Rect.t );
    r = MAX( r, Rect.r );
    b = MAX( b, Rect.b );
}

//==============================================================================
inline
s32 irect::GetWidth( void ) const
{
    return r - l;
}

//==============================================================================
inline
s32 irect::GetHeight( void ) const
{
    return b - t;
}

//==============================================================================
inline
vector2 irect::GetSize( void ) const
{
    return vector2( (f32)(r-l), (f32)(b-t) );
}

//==============================================================================
inline
vector2 irect::GetCenter( void ) const
{
    return vector2( (f32)(r-l)/2.0f, (f32)(b-t)/2.0f );
}

//==============================================================================
inline
void irect::SetWidth( s32 W )
{
    r = l + W;
}

//==============================================================================
inline
void irect::SetHeight( s32 H )
{
    b = t + H;
}

//==============================================================================
inline
void irect::SetSize( s32 W, s32 H )
{
    r = l + W;
    b = t + H;
}

//==============================================================================
inline
void irect::Translate( s32 X, s32 Y )
{
    l += X;
    r += X;
    t += Y;
    b += Y;
}

//==============================================================================
inline
void irect::Inflate( s32 X, s32 Y )
{
    l -= X;
    r += X;
    t -= Y;
    b += Y;
}

//==============================================================================
inline
void irect::Deflate( s32 X, s32 Y )
{
    l += X;
    r -= X;
    t += Y;
    b -= Y;
}

//==============================================================================

inline
f32 irect::Difference( const irect& R ) const
{
    return  (f32)((l-R.l)*(l-R.l) +
                  (t-R.t)*(t-R.t) +
                  (r-R.r)*(r-R.r) +
                  (b-R.b)*(b-R.b) );
}

//==============================================================================

inline
xbool irect::InRange( s32 Min, s32 Max ) const
{
    return( (l>=Min) && (l<=Max) &&
            (t>=Min) && (t<=Max) &&
            (r>=Min) && (r<=Max) && 
            (b>=Min) && (b<=Max) );
}


//==============================================================================

inline
xbool irect::IsEmpty( void ) const
{
    return( (l>=r) || (t>=b) );
}

//==============================================================================

inline
xbool irect::PointInRect( s32 X, s32 Y ) const
{
    return( (X>=l) && (X<r) && (Y>=t) && (Y<b) );
}

//==============================================================================

inline
bool irect::operator == ( const irect& R ) const
{
    return( (l == R.l) &&
            (r == R.r) &&
            (t == R.t) &&
            (b == R.b) );
}

inline
bool irect::operator != ( const irect& R ) const
{
    return( (l != R.l) ||
            (r != R.r) ||
            (t != R.t) ||
            (b != R.b) );
}

//==============================================================================
