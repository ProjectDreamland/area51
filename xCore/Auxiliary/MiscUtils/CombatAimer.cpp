#include "CombatAimer.hpp"


static f32 k_MissFutureTime             = 0.55f;

//=========================================================================
static
xbool CalculateLinearAimDirection( const vector3&   TargetPos,
                                   const vector3&   TargetVel,
                                   const vector3&   SourcePos,
                                   const vector3&   SourceVel,
                                         f32        VelInheritFactor,
                                         f32        LinearSpeed,
                                         f32        LifetimeS,
                                         vector3&   AimDirection,
                                         vector3&   AimPosition       )
{
    const vector3 EffTargetPos = TargetPos - SourcePos;
    const vector3 EffTargetVel = TargetVel - (SourceVel * VelInheritFactor);

    AimDirection.Zero();

    //
    // This is a straightforward law of cosines calculation.  
    // We're not being especially efficient here, but hey.
    //
    vector3 NormPos = EffTargetPos;
    vector3 NormVel = EffTargetVel;
    if( !NormPos.SafeNormalize() )  return( FALSE );
    NormVel.Normalize();

    const f32 A = EffTargetVel.LengthSquared() - (LinearSpeed*LinearSpeed);
    const f32 B = 2 * EffTargetPos.Length() * EffTargetVel.Length() * NormPos.Dot(NormVel);
    const f32 C = EffTargetPos.LengthSquared();

    const f32 det = (B * B) - (4 * A * C);

    if( IN_RANGE( -0.0001f, A, 0.0001f ) )
    {
        // Value A is 0 or at least damn close to 0.  The division for Sol1 a 
        // few lines below will either die or blow up.  So, bail out 
        // meainingfully.
        AimPosition  = TargetPos;
        AimDirection = (TargetPos - SourcePos);

        return( AimDirection.SafeNormalize() );
    }

    if (det < 0.0) 
    {
        // No solution is possible in the real numbers
        return FALSE;
    }

    const f32 Sol1 = (-B + x_sqrt(det)) / (2 * A);
    const f32 Sol2 = (-B - x_sqrt(det)) / (2 * A);

    f32 T;

    if (Sol2 > 0.0f)    T = Sol2;
    else                T = Sol1;
    
    if (T < 0.0f) 
    {
        return FALSE;
    }

    // Once we know how long the projectile's path will take, it's
    // straightforward to find out where it should go...
    AimPosition  = TargetPos + (EffTargetVel * T);
    AimDirection = (EffTargetPos / (LinearSpeed * T)) + (EffTargetVel / LinearSpeed);

    return( AimDirection.SafeNormalize() && ((T * 0.001f) <= LifetimeS) );
}

//=========================================================================




xbool CalculateLinearAimDirection( const vector3& aTargetPos,           // IN:  Position to shoot at
                                   const vector3& aTargetVel,           // IN:  Velocity of target
                                         radian   aTargetSightYaw,      // IN:  Direction target is looking
                                   const bbox&    aTargetBBox,          // IN:  BBox of target
                                   const vector3& aSourcePos,           // IN:  Where the projectile originates
                                         
                                         f32      LinearSpeed,          // IN:  Speed of the projectile
                                         f32      LifetimeS,            // IN:  Lifetime of the projectile in seconds
                                         xbool    bAimToHit,            // IN:  Aiming to hit or miss?
                                         vector3& AimDirection,         // OUT: Direction to fire projectile
                                         vector3& AimPosition    )      // OUT: Position where projectile will go to
{
    vector3 TargetPos = aTargetPos;

    if (!bAimToHit)
    {
        // Adjust the aim to be some small offset
        // below the target pos (more toward the floor)
        TargetPos.GetY() -= x_frand(0,75);

        // Displace the target pos forward by a small amount
        // based on the target's sight direction.                    
        vector3 MissDir = aTargetVel;
        f32     VelScale = aTargetVel.Length();
        radian  TargetYaw = R_0;

        MissDir.Set(0,0,VelScale);
        MissDir.RotateY( aTargetSightYaw );                        
                
        // Aim into the future
        TargetPos += MissDir * k_MissFutureTime;

        // Now, offset the position along the
        // vector perpendicular to the ToTargetVector
        vector3 ToTarget = TargetPos - aSourcePos;
        ToTarget.GetY() = 0;
        ToTarget.Normalize();
        MissDir.Set(0,0,1);
        MissDir.RotateY( TargetYaw );                                            

        // This gets us a scale value for how much to
        // aim along the perp (0,1);
        f32 PerpMagnitude = x_abs(ToTarget.Dot(MissDir));

        // Find out how wide the target is
        f32 Radius = aTargetBBox.GetRadius();

        // Get the perp vector
        vector3 Perp = ToTarget.Cross( vector3(0,1,0) );
        Perp.Normalize();

        //Scale the perp vector
        // Get it in the range [-1,-0.75] or [0.75,1]
        f32 MissThickness = 0.25;
        PerpMagnitude *= x_frand(-MissThickness,MissThickness);
        if (PerpMagnitude<0)
            PerpMagnitude -= (1-MissThickness);
        else
            PerpMagnitude += (1-MissThickness);
        
        PerpMagnitude *= Radius;

        Perp.Scale(PerpMagnitude);
        
        TargetPos += Perp;
    }

    

    xbool Ret = CalculateLinearAimDirection( TargetPos,
                                             aTargetVel,
                                             aSourcePos,
                                             vector3(0,0,0),
                                             0,
                                             LinearSpeed,
                                             LifetimeS,
                                             AimDirection,
                                             AimPosition );
    
    /*
    if (Ret)
    {                    
        // Verify that we can actually aim to this point
        vector3     vToAimPos = AimPos - InitPos;
        vector3     vAimDir(0,0,1);

        vToAimPos.Normalize();
        vAimDir.Rotate( InitRot );

        f32 T = vAimDir.Dot( vToAimPos );

        if (T > 0.9f)
        {
            AimDir.GetPitchYaw( InitRot.Pitch, InitRot.Yaw );
            InitRot.Roll = 0;
        }
    }*/

    return Ret;
}

