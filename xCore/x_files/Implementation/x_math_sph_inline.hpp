//==============================================================================
//
//  x_math_bb_inline.hpp
//
//==============================================================================

#ifndef X_MATH_SPH_INLINE_HPP
#define X_MATH_SPH_INLINE_HPP
#else
#error "File " __FILE__ " has been included twice!"
#endif

#ifndef X_DEBUG_HPP
#include "../x_debug.hpp"
#endif

//==============================================================================
// FUNCTIONS
//==============================================================================

//==============================================================================
inline
sphere::sphere( void )
{    
    FORCE_ALIGNED_16( this );
}

//==============================================================================
inline
sphere::sphere( const vector3& aPos, f32 aR )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &aPos );

    Pos = aPos;
    R   = aR;
}

//==============================================================================
inline
sphere::sphere( const bbox& BBox )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &BBox );

    Pos = BBox.GetCenter();
    R   = BBox.GetRadius();
}

//==============================================================================
inline
void sphere::Clear( void )
{
    FORCE_ALIGNED_16( this );

    Pos.Zero();
}

//==============================================================================
inline
void sphere::Set( const vector3& aPos, f32 aR )
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &aPos );

    Pos = aPos;
    R   = aR;
}

//==============================================================================
inline
bbox sphere::GetBBox( void ) const
{
    FORCE_ALIGNED_16( this );

    return bbox( Pos, R );
}

//==============================================================================
inline
xbool sphere::TestIntersect( const vector3& P0, const vector3& P1 ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &P0 );
    FORCE_ALIGNED_16( &P1 );

    const f32 SqrD = Pos.GetSqrtDistToLineSeg( P0, P1 );
    return SqrD <= (R*R);
}

//==============================================================================
inline
s32 sphere::Intersect( f32& t0, f32& t1, const vector3& P0, const vector3& P1 ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &P0 );
    FORCE_ALIGNED_16( &P1 );

    // set up quadratic Q(t) = a*t^2 + 2*b*t + c
    const vector3 Diff = P0 - Pos;
    const vector3 Dir  = (P1-P0);
    const f32     A    = Dir.LengthSquared();
    const f32     B    = Diff.Dot( Dir );
    const f32     C    = Diff.LengthSquared() - (R*R);

    // no intersection if Q(t) has no real roots
    const f32    Discr = B*B - A*C;

    if( Discr < 0.0f )
    {
        // The ray is completly contain inside the sphere
        if( C < 0 )
            return -0x1;

        return 0x0;
    }

    if( Discr > 0.0f )
    {
        const f32 Root = x_sqrt( Discr );
        const f32 InvA = 1.0f/A;
        t0 = (-B - Root) * InvA;
        t1 = (-B + Root) * InvA;

        // assert: t0 < t1 since A > 0        
        ASSERT( t0 < t1 );

        if( t0 > 1.0f || t1 < 0.0f )
            return 0x0;

        if( t0 >= 0.0f )
        {
            // there is an entrance point but not an exit
            if ( t1 > 1.0f )
                return 0x1;

            return 0x3;
        }
        else  // t1 >= 0
        {
            ASSERT( t1 >= 0 );

            // there is only one exit point
            return 0x2;
        }
    }
    else
    {
        t0 = -B/A;

        // there is an entrance point but not an exit
        if ( t0 >= 0.0f && t0 <= 1.0f )
            return 0x1;
    }

    return 0x0;
}

//==============================================================================
inline
xbool sphere::Intersect( f32& t0, const vector3& P0, const vector3& P1 ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &P0 );
    FORCE_ALIGNED_16( &P1 );

    f32 t1;
    const s32 C = Intersect( t0, t1, P0, P1 );
    
    // We didn't hit
    if( C == 0 )
        return 0;

    // If the ray is completly contain in the sphere
    if( C == -1 )
    {
        t0 = 0;
        return TRUE;
    }

    // If there is only an exit point then we consider that we were in.
    if( C == 2 )
    {
        t0 = 0;
        return TRUE;
    }

    // Anything with an entry hit is a good hit
    ASSERT( C & 1 );

    return TRUE;
}

//==============================================================================
inline
s32 sphere::TestIntersection( const plane& Plane ) const
{
    FORCE_ALIGNED_16( this );
    FORCE_ALIGNED_16( &Plane );

    const f32 Distance = Plane.Normal.Dot( Pos ) - Plane.D;

    if( Distance < -R || Distance > R ) 
        return 0;

    if( Distance < 0 ) return -1;
    return 1;
}



//xbool sphere::Intersect       ( vector3& P0, vector3& P1, vector3& P3 ) const;
//xbool sphere::Intersect       ( sphere& Sphere ) const;
