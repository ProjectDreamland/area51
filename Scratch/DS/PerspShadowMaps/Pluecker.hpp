#ifndef __PLUECKER_HPP_INCLUDED__
#define __PLUECKER_HPP_INCLUDED__

#include "x_files.hpp"

#if 0
//==============================================================================
// Struct for dealing with the pluecker coordinate system (useful for doing
// ray poly intersection tests quickly). The ideas behind pluecker coordinates
// get really hairy, but the math boils down to some simple formulas. I suggest
// reading up on them if you're curious, as it would take too long to put the
// explanation into comments. Flipcode has a good article that explains them,
// though.
//==============================================================================

struct pluecker
{
public:
    pluecker( void );
    pluecker( const vector3& Start, const vector3& End );

    f32     Dot                 ( const pluecker& P ) const;
    xbool   RayIntersectsPoly   ( const pluecker& Ray, s32 nSides, const pluecker* pPoly );

    f32 pi0, pi1, pi2, pi3, pi4, pi5;
};

//==============================================================================

#endif

#endif // __PLUECKER_HPP_INCLUDED__