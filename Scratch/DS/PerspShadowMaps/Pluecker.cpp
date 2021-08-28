#include "Pluecker.hpp"

#if 0

//==============================================================================
// Implementation
//==============================================================================

pluecker::pluecker( void )
{
}

//==============================================================================

pluecker::pluecker( const vector3& Start, const vector3& End )
{
    const vector3& P = Start;
    const vector3& S = End;
    
    pi0 = S.X*P.Y - P.X*S.Y;
    pi1 = S.X*P.Z - P.X*S.Z;
    pi2 = S.X - P.X;
    pi3 = S.Y*P.Z - P.Y*S.Z;
    pi4 = S.Z - P.Z;
    pi5 = P.Y - S.Y;
}

//==============================================================================

f32 pluecker::Dot( const pluecker& P ) const
{
    return ( pi0 * P.pi4 +
             pi1 * P.pi5 +
             pi2 * P.pi3 +
             pi3 * P.pi2 +
             pi4 * P.pi0 +
             pi5 * P.pi1 );
}

//==============================================================================

xbool pluecker::RayIntersectsPoly( const pluecker& Ray, s32 nSides, const pluecker* pPoly )
{
    f32   D        = Ray.Dot( pPoly[0] );
    xbool Positive = (D>0.0f);
    
    // NOTE: Using this test, intersections will occur with backfacing polys.
    // You'd just cut out negative or positive dots altogether if you didn't
    // want to intersect with backfaces or frontfaces.
    for ( s32 i = 1; i < nSides; i++ )
    {
        D = Ray.Dot( pPoly[i] );
        if ( (Positive && (D<=0.0f)) ||
             (!Positive && (D>0.0f)) )
        {
            return FALSE;
        }
    }
    
    return TRUE;
}

#endif