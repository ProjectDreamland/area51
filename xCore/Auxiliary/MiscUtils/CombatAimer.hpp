#ifndef __COMBAT_AIMER_HPP__
#define __COMBAT_AIMER_HPP__

//===================================================================

#include "x_math.hpp"

//===================================================================


xbool CalculateLinearAimDirection( const vector3& aTargetPos,           // IN:  Position to shoot at
                                   const vector3& aTargetVel,           // IN:  Velocity of target
                                         radian   aTargetSightYaw,      // IN:  Direction target is looking
                                   const bbox&    aTargetBBox,          // IN:  BBox of target
                                   const vector3& aSourcePos,           // IN:  Where the projectile originates                                         
                                         f32      LinearSpeed,          // IN:  Speed of the projectile
                                         f32      LifetimeS,            // IN:  Lifetime of the projectile in seconds
                                         xbool    bAimToHit,            // IN:  Aiming to hit or miss?

                                         vector3& AimDirection,         // OUT: Direction to fire projectile
                                         vector3& AimPosition    );     // OUT: Position where projectile will go to
//===================================================================

#endif // __COMBAT_AIMER_HPP__