//==============================================================================
//
//  CarObject.cpp
//
//  Finish building obstacle course
//  Get better controls for steering & acceleration
//  Work on stabilize
//  Add a reset
//  Tweak physics with design
//  Get running on PS2
//
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "CarObject.hpp"
#include "Entropy.hpp"
#include "Obj_mgr\obj_mgr.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "Debris\debris_mgr.hpp"

s32 DEBRIS_CHANCE = 0;
f32 DEBRIS_SPREAD = 0.1f;
f32 DEBRIS_TIME = 0.5f;

//==============================================================================
// TEMP DATA
//==============================================================================

struct car_object_def
{
    f32     m_Gravity;                      // Acceleration in cm/sec
    f32     m_ShockLength;                  // Maximum length of shock
    f32     m_ShockSpeedStartT;             // At what compression do we want zero speed
    f32     m_ShockSpeedFullyCompressed;    // Magic number
    f32     m_ShockStiffness;               // 0-basic 1-very stiff

    f32     m_WheelSideFriction;            // Magic number
    f32     m_WheelRadius;                  // Radius of the wheel in cm
    f32     m_TopSpeedForward;              // Miles per hour
    f32     m_TopSpeedReverse;              // Miles per hour
    f32     m_ZeroToTopSpeedTime;           // In Seconds for accelerating
    f32     m_TopSpeedToZeroBrakingTime;    // In Seconds for braking
    f32     m_TopSpeedToZeroCoastingTime;   // In Seconds for coasting
    f32     m_MaxSteeringAngle;             // In radians

    f32     m_WheelForwardTractionSpeedDiff;
    f32     m_WheelForwardTractionMin;
    f32     m_WheelForwardTractionMax;

    f32     m_WheelSideTractionSpeedDiff;
    f32     m_WheelSideTractionMin;
    f32     m_WheelSideTractionMax;

    f32     m_WheelVerticalLimit;           // Y-point where we lose traction 

    f32     m_WheelTurnRate;                // radians per second
};

// JEEP
static car_object_def  g_Def = 
{
    -2000.0f,        // m_Gravity
    35.0f,          // m_ShockLength
    0.3f,           // m_ShockSpeedStartT
    2500.0f,        // m_ShockSpeedFullyCompressed
    0.2f,           // m_ShockStiffness
    5.0f,           // m_WheelSideFriction
    35.0f,          // m_WheelRadius
    50.0f,          // m_TopSpeedForward
    20.0f,          // m_TopSpeedReverse
     3.0f,          // m_ZeroToTopSpeedTime
     1.0f,          // m_TopSpeedToZeroBrakingTime
     2.0f,          // m_TopSpeedToZeroCoastingTime
    0.8f,           // m_MaxSteeringAngle
     
    500.0f,         // m_WheelTractionForwardSpeedDiff
    0.25f,          // m_WheelForwardTractionMin
     0.5f,          // m_WheelForwardTractionMax

    250.0f,         // m_WheelTractionSideSpeedDiff
    0.01f,          // m_WheelSideTractionMin
     1.0f,          // m_WheelSideTractionMax

     0.5f,          // m_WheelVerticalLimit

     R_360,          // m_WheelTurnRate


};

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct car_object_desc : public object_desc
{
        car_object_desc( void ) : object_desc( 
            object::TYPE_VEHICLE, 
            "Car",
            "VEHICLE", 
            object::ATTR_NEEDS_LOGIC_TIME   |
            object::ATTR_COLLIDABLE         | 
            object::ATTR_RENDERABLE         | 
            object::ATTR_SPACIAL_ENTRY      |
            object::ATTR_DAMAGEABLE         |
            object::ATTR_LIVING,
            FLAGS_GENERIC_EDITOR_CREATE | FLAGS_IS_DYNAMIC )   { }

    virtual object* Create( void ) { return new car_object; }

#ifdef X_EDITOR
    s32 OnEditorRender( object& Object ) const
    {
        (void)Object;
        //if( Object.GetAttrBits() & object::ATTR_EDITOR_SELECTED )
        //    Object.OnDebugRender();

        return -1;
    }
#endif // X_EDITOR

} s_car_object_desc;

//=============================================================================

const object_desc& car_object::GetTypeDesc( void ) const
{
    return s_car_object_desc;
}

//=============================================================================

const object_desc& car_object::GetObjectType( void )
{
    return s_car_object_desc;
}
//==============================================================================
// WHEEL CLASS
//==============================================================================

wheel::wheel()
{
    m_iBone         = -1;      // Bone index
    m_Offset.Zero();           // Offset relative to car_object
    m_Steer         = 0;       // Steer yaw
    m_Spin          = 0;       // Spin roll
    m_SpinSpeed     = 0;       // Spinning speed
    m_Velocity.Zero();
    m_Position.Zero();
    m_OldPosition.Zero();
    m_ObjectGuid = 0;
    m_LastShockLength = 0;
    m_CurrentShockLength = 0;
    m_bPassive = FALSE;
}

//==============================================================================

void wheel::Init( s32 iBone, 
                  const matrix4& VehicleL2W, 
                  const vector3& ShockOffset, 
                  guid ObjectGuid,
                  xbool bPassive)
{
    m_iBone             = iBone;                                     // Bone index
    m_Offset            = ShockOffset;                               // Offset relative to car_object
    m_Steer             = 0;                                         // Steer yaw
    m_Spin              = 0;                                         // Spin roll
    m_SpinSpeed         = 0;                                         // Spinning speed
    m_ObjectGuid        = ObjectGuid;
    m_bHasTraction      = FALSE;
    m_Position          = VehicleL2W * m_Offset;
    m_OldPosition       = m_Position;
    m_CurrentShockLength= 0;
    m_LastShockLength   = 0;
    m_Velocity.Zero();
    m_bPassive          = bPassive;
    x_memset(m_History,0,sizeof(m_History));
    m_nLogicLoops = 0;
    m_DebrisTimer = 0;
}

//==============================================================================

void wheel::Reset( const matrix4& VehicleL2W )
{
    m_Steer                 = 0;
    m_Spin                  = 0;
    m_SpinSpeed             = 0;
    m_Position              = VehicleL2W * m_Offset;
    m_OldPosition           = m_Position;
    m_Velocity.Zero();
    m_CurrentShockLength    = 0;
    m_LastShockLength       = 0;
    m_bHasTraction          = FALSE;
    x_memset(m_History,0,sizeof(m_History));
    m_nLogicLoops = 0;
    m_DebrisTimer = 0;
}

//==============================================================================
u32 TRAP_WHEEL=0;
void wheel::ApplyTraction( f32 DeltaTime, const matrix4& VehicleL2W )
{
    xbool bTRAPPED = ((u32)this == TRAP_WHEEL );

    if( bTRAPPED )
        x_DebugMsg("TRAPPED:\n");

    // Back up shock length
    m_LastShockLength = m_CurrentShockLength;

    // Remember if we had traction last frame
    xbool bHadTraction = m_bHasTraction;
    m_bHasTraction = FALSE;

    // Get direction vectors for wheel
    vector3 WheelLeftDir;
    vector3 WheelUpDir(0,1,0);
    vector3 WheelForwardDir(0,0,1);
    WheelForwardDir.RotateY( m_Steer );
    WheelForwardDir = (VehicleL2W * WheelForwardDir) - VehicleL2W.GetTranslation();
    WheelUpDir      = (VehicleL2W * WheelUpDir) - VehicleL2W.GetTranslation();
    WheelLeftDir    = WheelUpDir.Cross(WheelForwardDir);

    // If we are upside down then no traction
    if( WheelUpDir.Y <= 0 )
    {
        // Add gravity to the velocity
        m_Velocity += vector3(0,g_Def.m_Gravity*1.5f,0)*DeltaTime;
        return;
    }

    // Compute sphere check to find ground
    f32     GroundCheckDistAbove = 100.0f;
    f32     GroundCheckDistBelow = 400.0f;//g_Def.m_ShockLength;
    vector3 S = m_Position + WheelUpDir * vector3(0, GroundCheckDistAbove,0);
    vector3 E = m_Position + WheelUpDir * vector3(0,-GroundCheckDistBelow,0);

    // Collect ground information
    plane   GroundPlane;
    plane   GroundSlipPlane;
    vector3 GroundPoint;
    f32     GroundT;
    f32     MaxTractionGroundT = (GroundCheckDistAbove+g_Def.m_ShockLength) / (GroundCheckDistAbove+GroundCheckDistBelow);
    f32     MinTractionGroundT = GroundCheckDistAbove / (GroundCheckDistAbove+GroundCheckDistBelow);

    // Prepare for collision
    g_CollisionMgr.SphereSetup( m_ObjectGuid, S, E, g_Def.m_WheelRadius );
    g_CollisionMgr.UseLowPoly();

    //
    // Perform collision and determine if we have traction
    //
    f32 CollT = 500.0f;
    if( g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE ) )
    {
        if( g_CollisionMgr.m_Collisions[0].Plane.Normal.Y >= g_Def.m_WheelVerticalLimit )
            CollT = g_CollisionMgr.m_Collisions[0].T;
    }

    if( CollT <= MaxTractionGroundT )
    {
        collision_mgr::collision& Coll = g_CollisionMgr.m_Collisions[0];
        GroundPlane = Coll.Plane;
        GroundPoint = Coll.Point;
        GroundSlipPlane = Coll.SlipPlane;
        GroundT     = Coll.T;

        if( GroundT < MinTractionGroundT )
        {
            // We are penetrating into the ground.  Must correct!
            m_bHasTraction = TRUE;
            vector3 SphereCenterAtImpact = S + GroundT*(E-S);
            m_Position = SphereCenterAtImpact;
            m_CurrentShockLength = 0;
        }
        else
        {
            // We are within the shock's reach.
            m_bHasTraction = TRUE;
            m_CurrentShockLength = g_Def.m_ShockLength*(GroundT - MinTractionGroundT)/(MaxTractionGroundT-MinTractionGroundT);
        }
    }
    else
    {
        m_bHasTraction = FALSE;
/*
        //
        // Double check collision
        //
        // Prepare for collision
        vector3 S = m_Position + WheelUpDir * vector3(0, GroundCheckDistAbove,0);
        vector3 E = m_Position + WheelUpDir * vector3(0,-400,0);
        g_CollisionMgr.SphereSetup( m_ObjectGuid, S, E, g_Def.m_WheelRadius );
        g_CollisionMgr.UseLowPoly();
        if( g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE ) )
        {
            f32 DistBelowPos = g_CollisionMgr.m_Collisions[0].T * (GroundCheckDistAbove+400.0f);
            DistBelowPos -= GroundCheckDistAbove;
            x_DebugMsg("Missed\n");
        }
        else
        {
            x_DebugMsg("Missed\n");
        }
*/
    }

    if( bTRAPPED )
    {
        x_DebugMsg("HasTraction: %s\n",(m_bHasTraction)?("YES"):("NO"));
    }

    //
    // Apply gravity
    //
    {
        // Add gravity to the velocity
        m_Velocity += vector3(0,g_Def.m_Gravity,0)*DeltaTime;
    }

    //
    // Apply friction and acceleration if we have traction
    //
    if( m_bHasTraction )
    {
        //
        // Get tire forward direction parallel to ground plane
        // Get tire side direction parallel to ground plane
        //
        vector3 WheelForwardParallelDir;
        vector3 WheelSideParallelDir;
        {
            vector3 WheelPar,WheelPerp;

            GroundSlipPlane.GetComponents( WheelForwardDir, WheelPar, WheelPerp );
            WheelForwardParallelDir = WheelPar;
            WheelForwardParallelDir.Normalize();

            GroundSlipPlane.GetComponents( WheelLeftDir, WheelPar, WheelPerp );
            WheelSideParallelDir = WheelPar;
            WheelSideParallelDir.Normalize();
        }

        //
        // Compute tire surface speed and ground speed
        //
        f32 TireSurfaceSpeed;   
        f32 GroundSpeed;
        {
            // Get current speed of tire surface
            TireSurfaceSpeed = m_SpinSpeed * g_Def.m_WheelRadius;

            // Get current speed of wheel
            GroundSpeed = WheelForwardParallelDir.Dot(m_Velocity);
        }


        //
        // Determine how much traction we have based on the speed 
        // difference between the tire surface speed and ground speed.
        // TractionT==0 No traction
        // TractionT==1 Full traction
        //
        f32 ForwardTractionT;
        {
            // Compute Traction
            ForwardTractionT = x_abs(TireSurfaceSpeed - GroundSpeed) / g_Def.m_WheelForwardTractionSpeedDiff;
            if( ForwardTractionT < 0 ) ForwardTractionT = 0;
            if( ForwardTractionT > 1 ) ForwardTractionT = 1;
            ForwardTractionT = g_Def.m_WheelForwardTractionMax + ForwardTractionT*(g_Def.m_WheelForwardTractionMin-g_Def.m_WheelForwardTractionMax);
        }

        f32 SideTractionT;
        {
            // Compute Traction
            SideTractionT = x_abs(TireSurfaceSpeed - GroundSpeed) / g_Def.m_WheelSideTractionSpeedDiff;
            if( SideTractionT < 0 ) SideTractionT = 0;
            if( SideTractionT > 1 ) SideTractionT = 1;
            SideTractionT = g_Def.m_WheelSideTractionMax + SideTractionT*(g_Def.m_WheelSideTractionMin-g_Def.m_WheelSideTractionMax);
        }

        //
        // Solve traction based on how steep the surface is.  When the surface
        // is vertical we have a traction of m_WheelVerticalTraction.  If 
        // it is horizontal we have a traction of 1.0f
        //
        f32 VerticalTractionT;
        {
            VerticalTractionT = (GroundSlipPlane.Normal.Y-g_Def.m_WheelVerticalLimit)/(1.0f-g_Def.m_WheelVerticalLimit);
            if( VerticalTractionT < 0 ) VerticalTractionT = 0;
            if( VerticalTractionT > 1 ) VerticalTractionT = 1;
            VerticalTractionT = x_sqrt(VerticalTractionT);
        }

        //
        // Dampen side velocity
        //
        {
            f32 SideSpeed   = WheelSideParallelDir.Dot(m_Velocity);
            f32 SpeedChange = SideSpeed * g_Def.m_WheelSideFriction * SideTractionT;

            // Since this is friction we only dampen up to the current amount
            if( x_abs(SpeedChange) > x_abs(SideSpeed) )
                SpeedChange = SideSpeed;

            m_Velocity -= WheelSideParallelDir*SpeedChange;
        }

        //
        // Adjust velocity along surface of ground
        //
        if( !m_bPassive )
        {
            // Get wheel velocity forward parallel to ground
            vector3 WheelPar,WheelPerp;
            GroundSlipPlane.GetComponents( WheelForwardDir, WheelPar, WheelPerp );
            WheelPar.Normalize();

            // Attempt to correct speed
            m_Velocity += WheelPar * (TireSurfaceSpeed-GroundSpeed) * VerticalTractionT * ForwardTractionT;
        }
    }

    //
    // Adjust shock
    //
    if( m_bHasTraction )
    {
        // Compute Shock Compression parametric
        // SCT=0 shock fully extended, SCT=1 shock fully compressed
        f32 SCT = 1 - (m_CurrentShockLength / g_Def.m_ShockLength);
        ASSERT( SCT >= 0.0f );
        ASSERT( SCT <= 1.0f );

        // Dampen any velocity into the shock based on how compressed it is
        f32 SpeedInDirectionOfTheShock = WheelUpDir.Dot( m_Velocity );
        if( SpeedInDirectionOfTheShock < 0 )
        {
            ASSERT((g_Def.m_ShockStiffness>=0) && (g_Def.m_ShockStiffness<=1));
            f32 DampenT = g_Def.m_ShockStiffness + SCT * (1.0f - g_Def.m_ShockStiffness);
                DampenT = x_sqrt(DampenT);
            m_Velocity += -WheelUpDir * SpeedInDirectionOfTheShock * DampenT;
        }

        if( bTRAPPED )
        {
            x_DebugMsg("CurrShockLength: %7.5f\n",m_CurrentShockLength);
            x_DebugMsg("LastShockLength: %7.5f\n",m_LastShockLength);
        }

        // Compute Current shock speed
//        f32 CurrentShockSpeed = (m_CurrentShockLength - m_LastShockLength) / DeltaTime;
        f32 CurrentShockSpeed = WheelUpDir.Dot( m_Velocity );

        ASSERT( g_Def.m_ShockSpeedStartT < 1.0f );
        f32 SpeedT = (SCT-g_Def.m_ShockSpeedStartT)/(1.0f-g_Def.m_ShockSpeedStartT);
        if( SpeedT < 0 ) SpeedT = 0;
        if( SpeedT > 1 ) SpeedT = 1;
        f32 DesiredShockSpeed = SpeedT*g_Def.m_ShockSpeedFullyCompressed;
        f32 ShockAccel = DesiredShockSpeed - CurrentShockSpeed;
        m_Velocity += WheelUpDir * ShockAccel * DeltaTime;
    }
    else
    {
        //
        // Since we don't have traction, extend shock completely and erase
        // shock speed
        //
        m_CurrentShockLength = g_Def.m_ShockLength;
        m_LastShockLength = m_CurrentShockLength;
    }

    if( (bHadTraction==FALSE) && (m_bHasTraction==TRUE) )
    {
        m_DebrisTimer += DEBRIS_TIME;
    }

}

//==============================================================================

void wheel::EnforceCollision( void )
{
    vector3 CurrentPos = m_OldPosition;
    vector3 DesiredPos = m_Position;
    vector3 Delta  = DesiredPos - CurrentPos;
    plane   LastPlane;

    s32 nLoops = 3;
    while( 1 )
    {
        // Check if we have iterated too many times
        if( (nLoops--) == 0 )
        {
            Delta.Zero();
            break;
        }

        f32 DeltaLen = Delta.Length();
        if( DeltaLen < 0.01f )
            break;

        g_CollisionMgr.SphereSetup( m_ObjectGuid, CurrentPos, CurrentPos+Delta, g_Def.m_WheelRadius+2.0f );
        g_CollisionMgr.UseLowPoly();
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE );
        if( g_CollisionMgr.m_nCollisions == 0 )
            break;

        collision_mgr::collision& Coll = g_CollisionMgr.m_Collisions[0];

        // Pull back from collision a tad
        f32 CollT = Coll.T - (1.0f / DeltaLen);
        if( CollT < 0 ) CollT = 0;
        if( CollT > 1 ) CollT = 1;

        //
        // Update NewPos with part of Delta
        //
        CurrentPos += Delta * CollT;

        //
        // Compute the slide delta
        //
        f32 RemainingT = 1-CollT;
        ASSERT( RemainingT >= 0 );
        ASSERT( RemainingT <= 1 );
        Delta *= RemainingT;
        vector3 SlideDelta;
        vector3 Perp;
        vector3 OldDelta = Delta;

        //
        // Is this our 2nd collision on this move between acute planes?
        //
        if( (nLoops == 1) && (LastPlane.Normal.Dot( Coll.SlipPlane.Normal )<= 0) )
        {
            // we should move parallel to the crease between the planes
            SlideDelta = LastPlane.Normal.Cross( Coll.SlipPlane.Normal );
            SlideDelta.Normalize();
            const f32 Dist = Delta.Dot( SlideDelta );
            SlideDelta *= Dist;

            if( !( SlideDelta.LengthSquared() <= Delta.LengthSquared() + 0.01f ) )
            {
                SlideDelta = Delta;
            }

            if( !( SlideDelta.LengthSquared() <= OldDelta.LengthSquared() + 0.01f ) )
            {
                SlideDelta = OldDelta;
            }

        }
        else
        {
            LastPlane = Coll.SlipPlane;
            Coll.SlipPlane.GetComponents( Delta, SlideDelta, Perp );
        }

        //
        // Use the slide delta
        //
        Delta = SlideDelta;

    }

    //
    // Move whatever delta is left after the collisions
    //
    if( Delta.LengthSquared() > 0.001f )
        CurrentPos += Delta;

    m_Position = CurrentPos;

    // We've generated a safe new position!!

    //
    // Check if we fell through!
    //
    {
        vector3 RS = m_Position + vector3(0,300.0f,0);
        vector3 RE = RS - vector3(0,300.0f,0);
        g_CollisionMgr.RaySetup( m_ObjectGuid, RS, RE );
        g_CollisionMgr.UseLowPoly();
        if( g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE ) )
        {
            collision_mgr::collision& Coll = g_CollisionMgr.m_Collisions[0];

            if( Coll.Point.Y > (m_Position.Y-g_Def.m_WheelRadius) ) 
            {
                m_Position.Y = Coll.Point.Y + g_Def.m_WheelRadius + 1.0f;
            }
        }
    }
}

//==============================================================================

xbool EnforceCollision( const vector3& DesiredStartPos, 
                       const vector3& DesiredEndPos,
                       f32 Radius,
                       guid ObjectGuid,
                       vector3& FinalPos
                       )
{
    vector3 CurrentPos = DesiredStartPos;
    vector3 DesiredPos = DesiredEndPos;
    vector3 Delta  = DesiredPos - CurrentPos;
    plane   LastPlane;
    xbool   bCollided = FALSE;

    s32 nLoops = 3;
    while( 1 )
    {
        // Check if we have iterated too many times
        if( (nLoops--) == 0 )
        {
            Delta.Zero();
            break;
        }

        f32 DeltaLen = Delta.Length();
        if( DeltaLen < 0.01f )
            break;

        g_CollisionMgr.SphereSetup( ObjectGuid, CurrentPos, CurrentPos+Delta, Radius );
        g_CollisionMgr.UseLowPoly();
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE );
        if( g_CollisionMgr.m_nCollisions == 0 )
            break;

        bCollided = TRUE;

        collision_mgr::collision& Coll = g_CollisionMgr.m_Collisions[0];

        // Pull back from collision a tad
        f32 CollT = Coll.T - (1.0f / DeltaLen);
        if( CollT < 0 ) CollT = 0;
        if( CollT > 1 ) CollT = 1;

        //
        // Update NewPos with part of Delta
        //
        CurrentPos += Delta * CollT;

        //
        // Compute the slide delta
        //
        f32 RemainingT = 1-CollT;
        ASSERT( RemainingT >= 0 );
        ASSERT( RemainingT <= 1 );
        Delta *= RemainingT;
        vector3 SlideDelta;
        vector3 Perp;
        vector3 OldDelta = Delta;

        //
        // Is this our 2nd collision on this move between acute planes?
        //
        if( (nLoops == 1) && (LastPlane.Normal.Dot( Coll.SlipPlane.Normal )<= 0) )
        {
            // we should move parallel to the crease between the planes
            SlideDelta = LastPlane.Normal.Cross( Coll.SlipPlane.Normal );
            SlideDelta.Normalize();
            const f32 Dist = Delta.Dot( SlideDelta );
            SlideDelta *= Dist;

            if( !( SlideDelta.LengthSquared() <= Delta.LengthSquared() + 0.01f ) )
            {
                SlideDelta = Delta;
            }

            if( !( SlideDelta.LengthSquared() <= OldDelta.LengthSquared() + 0.01f ) )
            {
                SlideDelta = OldDelta;
            }

        }
        else
        {
            LastPlane = Coll.SlipPlane;
            Coll.SlipPlane.GetComponents( Delta, SlideDelta, Perp );
        }

        //
        // Use the slide delta
        //
        Delta = SlideDelta;

    }

    //
    // Move whatever delta is left after the collisions
    //
    if( Delta.LengthSquared() > 0.001f )
        CurrentPos += Delta;

    FinalPos = CurrentPos;

    // We've generated a safe new position!!
/*
    //
    // Check if we fell through!
    //
    {
        vector3 RS = m_Position + vector3(0,300.0f,0);
        vector3 RE = RS - vector3(0,300.0f,0);
        g_CollisionMgr.RaySetup( m_ObjectGuid, RS, RE );
        g_CollisionMgr.UseLowPoly();
        if( g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE ) )
        {
            collision_mgr::collision& Coll = g_CollisionMgr.m_Collisions[0];

            if( Coll.Point.Y > (m_Position.Y-g_Def.m_WheelRadius) ) 
            {
                m_Position.Y = Coll.Point.Y + g_Def.m_WheelRadius + 1.0f;
            }
        }
    }
*/
    return bCollided;
}

//==============================================================================

void wheel::ApplyInput( f32 DeltaTime, f32 Accel, f32 Brake )
{
    //
    // Compute numbers from Def.
    //
    f32 CircumferenceOfTire = (g_Def.m_WheelRadius*2)*3.14159f;
    f32 MPHToCMPSec = (12.0f*2.54f*5280.0f)/3600.0f;
    f32 TopForwardRotPerSecond = +(g_Def.m_TopSpeedForward*MPHToCMPSec) / CircumferenceOfTire;
    f32 TopReverseRotPerSecond = -(g_Def.m_TopSpeedReverse*MPHToCMPSec) / CircumferenceOfTire;
    f32 Acceleration = TopForwardRotPerSecond / g_Def.m_ZeroToTopSpeedTime;
    f32 BrakingRate = TopForwardRotPerSecond / g_Def.m_TopSpeedToZeroBrakingTime;
    f32 CoastingRate = TopForwardRotPerSecond / g_Def.m_TopSpeedToZeroCoastingTime;

    if( !m_bPassive )
    {
        if( input_IsPressed( INPUT_PS2_BTN_L2 ) )
        {
            Acceleration *= 1.4f;
            TopForwardRotPerSecond *= 1.4f;
            TopReverseRotPerSecond *= 1.4f;
        }

        // Apply acceleration
        if( Accel != 0 )
        {
            // Accelerating
            m_SpinSpeed += Accel * R_360 * Acceleration * DeltaTime;

            // Spinning forward
            if( m_SpinSpeed > (R_360 * TopForwardRotPerSecond) )
                m_SpinSpeed = R_360 * TopForwardRotPerSecond;

            // Spinning backward
            if( m_SpinSpeed < (R_360 * TopReverseRotPerSecond) )
                m_SpinSpeed = R_360 * TopReverseRotPerSecond;
        }

        if( ((Accel < 0) && (m_SpinSpeed>0)) ||
            ((Accel > 0) && (m_SpinSpeed<0)) )
        {
            Brake = 1;
        }
    }

    // Apply braking
    if( Brake != 0 )
    {
        if( m_SpinSpeed > 0 )
        {
            m_SpinSpeed -= Brake * R_360 * BrakingRate * DeltaTime;

            if( m_SpinSpeed < R_1 )
                m_SpinSpeed = 0;
        }
        else
        {
            m_SpinSpeed += Brake * R_360 * BrakingRate * DeltaTime;

            if( m_SpinSpeed > -R_1 )
                m_SpinSpeed = 0;
        }
    }

    // Apply natural friction
    if( (Accel==0) && (Brake==0) )
    {
        if( m_SpinSpeed > 0 )
        {
            m_SpinSpeed -= R_360 * CoastingRate * DeltaTime;

            if( m_SpinSpeed < R_1 )
                m_SpinSpeed = 0;
        }
        else
        if( m_SpinSpeed < 0 )
        {
            m_SpinSpeed += R_360 * CoastingRate * DeltaTime;

            if( m_SpinSpeed > -R_1 )
                m_SpinSpeed = 0;
        }
    }

    // Update current tire rotation
    m_Spin += m_SpinSpeed * DeltaTime;
   
}

//==============================================================================
void wheel::Advance( f32 DeltaTime, const matrix4& VehicleL2W, f32 Accel, f32 Brake )
{
    ApplyInput( DeltaTime, Accel, Brake );

    ApplyTraction( DeltaTime, VehicleL2W );

    // Step position
    m_Position += m_Velocity * DeltaTime;

    m_nLogicLoops++;

    history& H = m_History[m_nLogicLoops%WHEEL_HISTORY_LENGTH];
    H.ShockLength = m_CurrentShockLength;
    H.ShockSpeed  = (m_CurrentShockLength-m_LastShockLength)/DeltaTime;
    H.bHasTraction = m_bHasTraction;



    m_DebrisTimer -= DeltaTime;
    if( m_DebrisTimer <= 0 ) m_DebrisTimer = 0;
    if( m_DebrisTimer > 0 )
    {
        vector3 VelDir = m_Velocity;
        if( VelDir.Length() > 2.0f )
        {
            VelDir.Normalize();

            vector3 WheelDownDir = VehicleL2W.RotateVector(vector3(0,-1,0));
            vector3 WheelBottom = m_Position+WheelDownDir*(m_CurrentShockLength+g_Def.m_WheelRadius);

            if( x_irand(0,100) < DEBRIS_CHANCE )
            {
                VelDir += vector3(x_frand(-1,+1),x_frand(-1,+1),x_frand(-1,+1));
                debris_mgr::GetDebrisMgr()->CreateDebris(   WheelBottom,
                                                            -VelDir*300.0f ,
                                                            "DEB_CONC_Small.rigidgeom"    );
            }
        }
    }

}

//==============================================================================

void wheel::FinishAdvance( f32 DeltaTime, const matrix4& VehicleL2W )
{
    (void)VehicleL2W;
    //
    // Update velocity
    //
    m_Velocity = (m_Position - m_OldPosition) / DeltaTime;

    //
    // Backup current position
    //
    m_OldPosition = m_Position;
}

//==============================================================================
// CAR_OBJECT CLASS
//==============================================================================

car_object::car_object()
{
}

//==============================================================================

car_object::~car_object()
{
}

//==============================================================================

s32 NUM_BUBBLES = 3;
vector3 BUBBLE_OFFSET[3] = {vector3(0,100,100),vector3(0,100,0),vector3(0,100,-100)};
f32 BUBBLE_RADIUS[3] = {90,90,90};

//==============================================================================

void car_object::VehicleInit( void )
{
    s32 i;

    m_bDisplayShockInfo = FALSE;

    m_BackupL2W = GetL2W();

    rigid_geom* pGeom = m_RigidInst.GetRigidGeom();
    if( !pGeom )
        return;

    // Lookup animation pointer
    anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
    ASSERT(pAnimGroup);

    // Initialize wheels
    for (i = 0; i < 4; i++)
    {
        // Lookup bone index
        s32 iBone = -1;
        switch(i)
        {
            case 0: iBone = pAnimGroup->GetBoneIndex("whl_lft_frnt"); break;
            case 1: iBone = pAnimGroup->GetBoneIndex("whl_rt_frnt"); break;
            case 2: iBone = pAnimGroup->GetBoneIndex("whl_lft_bck");  break;
            case 3: iBone = pAnimGroup->GetBoneIndex("whl_rt_bck");  break;
        }

        // In case object is not found
        if (iBone == -1)
            iBone = 0;

        // Lookup offset 
        vector3 Offset = pAnimGroup->GetBone(iBone).BindTranslation;
        //Offset += vector3(0,20,0);

        // Initialize wheel
        m_Wheels[i].Init(iBone,                     // iBone
                         GetL2W(),
                         Offset,                    // Offset
                         GetGuid(),
                         FALSE);//(i<2) ? (TRUE) : (FALSE));    
    }

    m_OldStabilizationTheta = 0;

    m_RevTurbo = 0;

    //
    // Copy in collision bubbles
    //
    {
        m_nBubbles = NUM_BUBBLES;
        for( s32 i=0; i<NUM_BUBBLES; i++ )
        {
            m_BubbleOffset[i] = BUBBLE_OFFSET[i];
            m_BubbleRadius[i] = BUBBLE_RADIUS[i];
        }
    }

    m_AudioIdle = 0;
    m_AudioRev = 0;
}

//==============================================================================

void car_object::VehicleKill( void )
{
    g_AudioManager.Release( m_AudioIdle, 0 );
    g_AudioManager.Release( m_AudioRev, 0 );
}

//==============================================================================

void car_object::VehicleReset( void )
{
    s32 i;

    m_OldStabilizationTheta = 0;

    SetTransform( m_BackupL2W );

    // Reset wheels
    for (i = 0; i < 4; i++)
        m_Wheels[i].Reset( m_BackupL2W );

    vehicle_object::VehicleReset();

    m_RevTurbo = 0;

    //
    // Copy in collision bubbles
    //
    {
        m_nBubbles = NUM_BUBBLES;
        for( s32 i=0; i<NUM_BUBBLES; i++ )
        {
            m_BubbleOffset[i] = BUBBLE_OFFSET[i];
            m_BubbleRadius[i] = BUBBLE_RADIUS[i];
        }
    }

}

//==============================================================================

void car_object::ComputeL2W( void )
{
    s32 i;

    vector3 FLToFR = m_Wheels[1].m_Position - m_Wheels[0].m_Position;
    vector3 FLToBL = m_Wheels[2].m_Position - m_Wheels[0].m_Position;

    vector3 Up = FLToBL.Cross(FLToFR);
    vector3 Forward = -FLToBL;
    vector3 Left = Up.Cross(Forward);
    Up.Normalize();
    Forward.Normalize();
    Left.Normalize();

    matrix4 L2W;
    L2W.Identity();
    L2W.SetColumns(Left,Up,Forward);

    vector3 L2WPos(0,0,0);
    for( i=0; i<4; i++ )
    {
        vector3 Trans = m_Wheels[i].m_Position - (L2W * m_Wheels[i].m_Offset);
        L2WPos += Trans*0.25f;
    }

    L2W.SetTranslation( L2WPos );

    SetTransform(L2W);
    OnMove( L2WPos );
}

//==============================================================================

void car_object::AdvanceSteering( f32 DeltaTime )
{
    (void)DeltaTime;
    s32 i;

    // Get camera yaw
    radian CameraYaw = m_Camera.m_Yaw;
    radian ForwardYaw = GetL2W().GetRotation().Yaw;

    radian WheelAngle;
    if( m_Input.m_Accel >= 0 )
        WheelAngle =  x_MinAngleDiff(CameraYaw,ForwardYaw);
    else
        WheelAngle = -x_MinAngleDiff(CameraYaw,ForwardYaw);


    // Peg wheel angles to limits
    if( WheelAngle < -g_Def.m_MaxSteeringAngle ) WheelAngle = -g_Def.m_MaxSteeringAngle;
    if( WheelAngle >  g_Def.m_MaxSteeringAngle ) WheelAngle =  g_Def.m_MaxSteeringAngle;
    radian MaxWheelRot = g_Def.m_WheelTurnRate * DeltaTime;

    // Rotate wheels as closely to camera yaw as we can.
    for( i=0; i<2; i++ )
    {
        radian WheelDiff = (WheelAngle - m_Wheels[i].m_Steer);
        if( WheelDiff > +MaxWheelRot ) WheelDiff = +MaxWheelRot;
        if( WheelDiff < -MaxWheelRot ) WheelDiff = -MaxWheelRot;
        
        m_Wheels[i].m_Steer += WheelDiff;

        if( m_Wheels[i].m_Steer > +g_Def.m_MaxSteeringAngle) m_Wheels[i].m_Steer = +g_Def.m_MaxSteeringAngle;
        if( m_Wheels[i].m_Steer < -g_Def.m_MaxSteeringAngle) m_Wheels[i].m_Steer = -g_Def.m_MaxSteeringAngle;
    }
/*
    // Rotate wheels as closely to camera yaw as we can.
    for( i=2; i<4; i++ )
    {
        radian WheelDiff = ((-WheelAngle) - m_Wheels[i].m_Steer);
        if( WheelDiff > +MaxWheelRot ) WheelDiff = +MaxWheelRot;
        if( WheelDiff < -MaxWheelRot ) WheelDiff = -MaxWheelRot;
        
        m_Wheels[i].m_Steer += WheelDiff;

        if( m_Wheels[i].m_Steer > +g_Def.m_MaxSteeringAngle) m_Wheels[i].m_Steer = +g_Def.m_MaxSteeringAngle;
        if( m_Wheels[i].m_Steer < -g_Def.m_MaxSteeringAngle) m_Wheels[i].m_Steer = -g_Def.m_MaxSteeringAngle;
    }
*/
}

//==============================================================================

f32 CAR_IDLE_VOLUME[4]      = { 0.50f,   0.60f,  0.60f,  0.60f };
f32 CAR_IDLE_PITCH[4]       = { 0.60f,   0.80f,  1.10f,  1.20f };
f32 CAR_IDLE_T[4]           = { 0.00f,   0.33f,  0.50f,  1.00f };
f32 CAR_REV_VOLUME[4]       = { 0.00f,   0.25f,  0.50f,  0.80f };
f32 CAR_REV_PITCH[4]        = { 0.80f,   0.80f,  1.00f,  1.20f };
f32 CAR_REV_T[4]            = { 0.00f,   0.33f,  0.66f,  1.00f };

f32 REV_TURBO_ACCEL = 1.0f;
f32 REV_TURBO_DECEL = 4.0f;
f32 REV_TURBO_MAX   = 2.0f;


void car_object::UpdateAudio( f32 DeltaTime )
{
    (void)DeltaTime;

    //
    // Try to gather audio
    //
    if( !m_AudioIdle )
        m_AudioIdle = g_AudioManager.Play("Jeep_Loop_01",GetPosition());

    if( !m_AudioRev )
        m_AudioRev  = g_AudioManager.Play("Jeep_Loop_02",GetPosition());


    //
    // Get average wheel speed
    //
    s32 i;
    f32 AvgWS=0;
    for( i=0; i<4; i++ )
        AvgWS += x_abs(m_Wheels[i].m_SpinSpeed)*0.25f;

    f32 CircumferenceOfTire = (g_Def.m_WheelRadius*2)*3.14159f;
    f32 MPHToCMPSec = (12.0f*2.54f*5280.0f)/3600.0f;
    f32 TopForwardRotPerSecond = +(g_Def.m_TopSpeedForward*MPHToCMPSec) / CircumferenceOfTire;
    f32 SpinT = AvgWS / (TopForwardRotPerSecond*R_360);
    if( SpinT < 0 ) SpinT = 0;
    if( SpinT > 1 ) SpinT = 1;

//    x_DebugMsg("SpinT: %6.2f\n",SpinT);

    f32 IdleV=0,IdleP=0;
    f32 RevV=0,RevP=0;
    for( i=1; i<=3; i++ )
    {
        if( (SpinT>=CAR_IDLE_T[i-1]) && (SpinT<=CAR_IDLE_T[i]) )
        {
            f32 T = (SpinT-CAR_IDLE_T[i-1]) / (CAR_IDLE_T[i]-CAR_IDLE_T[i-1]);
            IdleV = CAR_IDLE_VOLUME[i-1] + T*(CAR_IDLE_VOLUME[i]-CAR_IDLE_VOLUME[i-1]);
            IdleP = CAR_IDLE_PITCH[i-1] + T*(CAR_IDLE_PITCH[i]-CAR_IDLE_PITCH[i-1]);
        }

        if( (SpinT>=CAR_REV_T[i-1]) && (SpinT<=CAR_REV_T[i]) )
        {
            f32 T = (SpinT-CAR_REV_T[i-1]) / (CAR_REV_T[i]-CAR_REV_T[i-1]);
            RevV = CAR_REV_VOLUME[i-1] + T*(CAR_REV_VOLUME[i]-CAR_REV_VOLUME[i-1]);
            RevP = CAR_REV_PITCH[i-1] + T*(CAR_REV_PITCH[i]-CAR_REV_PITCH[i-1]);
        }
    }

    //
    // Check if REV is in turbo from lost traction
    //
    {
        for( i=0; i<4; i++ )
        if( m_Wheels[i].m_bHasTraction ) 
            break;
        if( i==4 )
        {
            // Add to turbo
            m_RevTurbo += DeltaTime * REV_TURBO_ACCEL;
            if( m_RevTurbo > REV_TURBO_MAX )
                m_RevTurbo = REV_TURBO_MAX;
        }
        else
        {
            // Reduce turbo
            m_RevTurbo -= DeltaTime * REV_TURBO_DECEL;
            if( m_RevTurbo < 0 )
                m_RevTurbo = 0;
        }
    }

    RevP += m_RevTurbo;

    g_AudioManager.SetVolume    ( m_AudioIdle,  IdleV );
    g_AudioManager.SetVolume    ( m_AudioRev,   RevV );
    g_AudioManager.SetPitch     ( m_AudioIdle,  IdleP );
    g_AudioManager.SetPitch     ( m_AudioRev,   RevP );
    g_AudioManager.SetPosition  ( m_AudioIdle,  GetPosition() );
    g_AudioManager.SetPosition  ( m_AudioRev,   GetPosition() );
}

//==============================================================================

void car_object::VehicleUpdate( f32 DeltaTime )
{
    extern xbool g_RenderFrameRateInfo;
   g_RenderFrameRateInfo = FALSE;

    AdvanceSteering( DeltaTime );

    // Do air stabilization
    UpdateAirStabilization( DeltaTime );

xtimer Timer;
Timer.Start();
    // Advance front wheels
    m_Wheels[0].Advance( DeltaTime, GetL2W(), m_Input.m_Accel, m_Input.m_Brake);
    m_Wheels[1].Advance( DeltaTime, GetL2W(), m_Input.m_Accel, m_Input.m_Brake);

    // Advance rear wheels
    m_Wheels[2].Advance( DeltaTime, GetL2W(), m_Input.m_Accel, m_Input.m_Brake);
    m_Wheels[3].Advance( DeltaTime, GetL2W(), m_Input.m_Accel, m_Input.m_Brake);
Timer.Stop();
//x_DebugMsg("Advance:    %5.2f\n",Timer.ReadMs());


Timer.Reset();
Timer.Start();
    // Enforce constraints
    EnforceConstraints();
Timer.Stop();
//x_DebugMsg("Enforce:    %5.2f\n",Timer.ReadMs());


    // Allow wheels to reorient
    m_Wheels[0].FinishAdvance(DeltaTime,GetL2W());
    m_Wheels[1].FinishAdvance(DeltaTime,GetL2W());
    m_Wheels[2].FinishAdvance(DeltaTime,GetL2W());
    m_Wheels[3].FinishAdvance(DeltaTime,GetL2W());

    UpdateAudio(DeltaTime);


    if( input_WasPressed( INPUT_PS2_BTN_L_UP ) )
        m_bDisplayShockInfo = !m_bDisplayShockInfo;
}

//==============================================================================
radian STABILIZE_THETA = R_30;
void car_object::UpdateAirStabilization( f32 DeltaTime )
{
    (void)DeltaTime;
    return;
/*
    //s32 i;
    (void)DeltaTime;
    vector3 UpDir = GetL2W().RotateVector(vector3(0,1,0));
    if( UpDir.Y <= 0.0f )
    {
        // Shoot ray downward
        g_CollisionMgr.RaySetup( GetGuid(), GetPosition(), GetPosition() + vector3(0,-150.0f,0) );
        if( g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE ) )
        {
            // Blow up the car!
            vehicle_object::VehicleReset();
        }
    }

    return;
*/

    s32 i;

    // Determine current UP axis and current theta
    vector3 UpDir = GetL2W().RotateVector(vector3(0,1,0));
    radian  CurrentTheta = v3_AngleBetween( UpDir, vector3(0,1,0) );

    // If any wheels have traction then don't stabilize
    for( i=0; i<4; i++ )
    if( m_Wheels[i].m_bHasTraction )
        break;
    if( i < 4 )
    {
        return;
    }
    
    // If no wheels have traction then try and stabilize
    x_printfxy(50,15,"!!!STABILIZING!!!");

    if( CurrentTheta > STABILIZE_THETA )
    {
        // Compute axis to rotate on and build quaternion
        vector3 RotAxis = UpDir.Cross(vector3(0,1,0));
        quaternion Q;
        Q.Setup(RotAxis,(CurrentTheta-STABILIZE_THETA));

        // Loop through the wheels and apply stabilization
        vector3 CarPosition = GetPosition();
        for( i=0; i<4; i++ )
        {
            vector3 OldP = m_Wheels[i].m_Position;
            vector3 NewP = Q*(OldP-CarPosition) + CarPosition;
            m_Wheels[i].m_Position = NewP;

            vector3 Dir = NewP - OldP;
            f32 Len = Dir.Length();
            if( Len > 0.0001f )
            {
                Dir /= Len;
                f32 SpeedInDir = Dir.Dot(m_Wheels[i].m_Velocity);
                m_Wheels[i].m_Velocity -= Dir * SpeedInDir;
            }
        }
    }
/*
    // Compute current Theta speed
    radian CurrentThetaSpeed = (CurrentTheta - OldTheta) / DeltaTime;

    // Determine expansion T
    radian MaxTheta = R_20;
    radian StabilizationSpeed = -R_20;

    f32 ExpT = CurrentTheta / MaxTheta;
    if( ExpT < 0 ) ExpT = 0;
    if( ExpT > 1 ) ExpT = 1;

    radian DesiredThetaSpeed = ExpT*StabilizationSpeed;

    // Compute angle to rotate this frame
    radian ThetaAccel = (DesiredThetaSpeed - CurrentThetaSpeed);

    radian ThetaCorrection  = ThetaAccel * DeltaTime;

    // Compute axis to rotate on and build quaternion
    vector3 RotAxis = UpDir.Cross(vector3(0,1,0));
    quaternion Q;
    Q.Setup(RotAxis,ThetaCorrection);

    // Determine average of the wheel positions
    vector3 AvgPos(0,0,0);
    for( i=0; i<4; i++ )
        AvgPos += m_Wheels[i].m_Position;
    AvgPos *= 0.25f;

    // Loop through the wheels and apply stabilization
    vector3 CarPosition = GetPosition();
    for( i=0; i<4; i++ )
    {
        vector3 P = m_Wheels[i].m_Position;
        P -= CarPosition;
        P = Q*P;
        P += CarPosition;
        m_Wheels[i].m_Position = P;
    }

    // Save new theta
    m_OldStabilizationTheta = CurrentTheta + ThetaCorrection;
*/
}

//=========================================================================

xbool ComputeTriSphereMovement( const vector3& aP0,
                               const vector3& aP1,
                               const vector3& aP2,
                               const vector3& SphereCenter,
                                     f32      SphereRadius,
                                     f32      SphereHalfHeight,
                                     vector3& FinalMovement )
{
    f32 SphereRadiusSquared = SphereRadius * SphereRadius;
    f32 WorldToSphereScale = SphereRadius / SphereHalfHeight;

    // Compute points in sphere space
    vector3 SphereSpacePt[4];
    SphereSpacePt[0] = aP0 - SphereCenter;
    SphereSpacePt[1] = aP1 - SphereCenter;
    SphereSpacePt[2] = aP2 - SphereCenter;
    SphereSpacePt[0].Y *= WorldToSphereScale;
    SphereSpacePt[1].Y *= WorldToSphereScale;
    SphereSpacePt[2].Y *= WorldToSphereScale;
    SphereSpacePt[3] = SphereSpacePt[0];
    plane SphereSpacePlane(SphereSpacePt[0],SphereSpacePt[1],SphereSpacePt[2]) ;

    //
    // Sphere center is now at (0,0,0) and the sphere 
    // has radius SphereRadius
    //

    //
    // Are we completely in front of the plane?
    //
    f32 SphereCenterDistFromPlane = SphereSpacePlane.D;
    if( (SphereCenterDistFromPlane >  SphereRadius) || 
        (SphereCenterDistFromPlane < -SphereRadius))
        return FALSE;


    vector3 BestPushDir;
    f32     BestPushDist = 0;
    
    //
    // Check if closest point to plane is inside triangle
    //
    {
        vector3 ClosestPtOnPlaneToSphereCenter = - SphereSpacePlane.Normal * SphereCenterDistFromPlane;

        s32 i;
        for( i=0; i<3; i++ )
        {
            vector3& PA = SphereSpacePt[i];
            vector3& PB = SphereSpacePt[i+1];
            vector3 EdgeNormal = SphereSpacePlane.Normal.Cross( PB - PA );
            if( EdgeNormal.Dot( ClosestPtOnPlaneToSphereCenter - PA ) <= 0 )
                break;
        }

        if( i==3 )
        {
            // Setting up the push depends on which side of the plane we are on
            if( SphereCenterDistFromPlane > 0 )
            {
                // We are in front
                BestPushDir = -ClosestPtOnPlaneToSphereCenter;
                BestPushDist = SphereRadius - SphereCenterDistFromPlane;
            }
            else
            {
                // We are behind
                //BestPushDir = ClosestPtOnPlaneToSphereCenter;
                //BestPushDist = SphereRadius + (-SphereCenterDistFromPlane);
            }
        }
    }

    //
    // Loop through the edges and find closest pt to sphere
    //
    if( BestPushDist == 0.0f )
    {
        vector3 Zero(0,0,0);

        for( s32 i=0; i<3; i++ )
        {
            vector3& PA = SphereSpacePt[i];
            vector3& PB = SphereSpacePt[i+1];

            // Get closest pt between edge and sphere center
            vector3 CP = Zero.GetClosestVToLSeg(PA,PB);

            f32 LenSquared = CP.LengthSquared();
            if( LenSquared < SphereRadiusSquared )
            {
                f32 PushDist = SphereRadius - x_sqrt(LenSquared);
                if( PushDist > BestPushDist )
                {
                    BestPushDir = -CP;
                    BestPushDist = BestPushDist;
                }
            }
        }
    }

    // If there were no close calls then bail
    if( BestPushDist == 0 )
        return FALSE;

    // Normalize the push direction, move it into world space and extend it 1cm
    BestPushDir.Normalize();
    BestPushDir *= BestPushDist;
    BestPushDir.Y /= WorldToSphereScale;
    f32 BPDLen = BestPushDir.Length();
    BestPushDir /= BPDLen;
    BestPushDir *= (BPDLen + 1.0f);

    FinalMovement = BestPushDir;

    //ASSERT( DoubleCheck(aP0,aP1,aP2,SphereCenter+BestPushDir,SphereRadius,SphereHalfHeight) );
    return TRUE;
}

//=========================================================================
s32 N_ITERATIONS_1 = 4;
s32 N_ITERATIONS_2 = 4;
s32 N_ITERATIONS_3 = 2;

void SolveSphereCollisions( guid IgnoreGuid, s32 nSpheres, vector3* pSphereOffset, vector3* pSpherePos, f32* pSphereRadius )
{
    s32 i,j;

    //
    // Build full bbox and gather clusters
    //
    {
        fbbox FullBBox;
        FullBBox.Clear();
        for( i=0; i<nSpheres; i++ )
        {
            FullBBox += bbox( pSpherePos[i], pSphereRadius[i] );
        }
        FullBBox.Inflate(100,100,100);

        //
        // Gather factored out list of clusters in dynamic area
        //
        g_PolyCache.BuildClusterList( FullBBox, object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE );
    }

    s32 nIterations = N_ITERATIONS_1;
    while( nIterations-- )
    {
        //
        // Run constraints
        //
        {
            s32 MaxLoops = N_ITERATIONS_2;
            f32 CorrectionFactor = 1.0f / (f32)MaxLoops;
            s32 nLoops = MaxLoops;
            while( nLoops-- )
            {
                xbool bCorrected = FALSE;

                for( i=0; i<nSpheres; i++ )
                {
                    vector3 SphereAPos = pSpherePos[i];
                    vector3 SphereAOffset = pSphereOffset[i];

                    for( j=1; j<nSpheres; j++ )
                    {
                        s32 J = (i+j)%nSpheres;
                        vector3 SphereBPos = pSpherePos[J];
                        vector3 SphereBOffset = pSphereOffset[J];

                        vector3 OffsetDelta = SphereBOffset - SphereAOffset;
                        f32     ConstLength = OffsetDelta.Length();
                        vector3 Delta       = SphereBPos - SphereAPos;
                        f32     Length      = Delta.Length();
                        f32     LenDiff     = ConstLength - Length;

                        if( x_abs(LenDiff) > 1.0f )
                        {
                            vector3 CorrDelta   = (Delta / Length) * LenDiff;
                            pSpherePos[i] -= CorrDelta * CorrectionFactor;
                            pSpherePos[J] += CorrDelta * CorrectionFactor;
                            bCorrected = TRUE;
                        }
                    }
                }

                if( !bCorrected )
                    break;
            }
        }

        //
        // Push each sphere away from any penetration
        //
        {
            for( i=0; i<nSpheres; i++ )
            {
                s32 MaxLoops = N_ITERATIONS_3;
                s32 nLoops;
                fbbox MoveBounds;
                MoveBounds.Min.Zero();
                MoveBounds.Max.Zero();

                for( nLoops=0; nLoops < MaxLoops; nLoops++ )
                {
                    s32 nPenetrations = 0;
                    vector3 AccumMoveDelta;
                    AccumMoveDelta.Zero();

                    //
                    // Build sphere info
                    //
                    f32 SphereRadius = pSphereRadius[i];
                    vector3 SphereCenter = pSpherePos[i];
                    fbbox SphereBBox = bbox( SphereCenter, SphereRadius );

                    //
                    // Process clusters
                    //
                    if( g_PolyCache.m_nClusters )
                    {
                        //
                        // Loop through the clusters and process the triangles
                        //
                        for( s32 iCL=0; iCL<g_PolyCache.m_nClusters; iCL++ )
                        {
                            xbool bHitThisCluster=FALSE;
                            poly_cache::cluster& CL = *g_PolyCache.m_ClusterList[iCL];

                            if( CL.Guid == IgnoreGuid )
                                continue;

                            if( !CL.BBox.Intersect(SphereBBox) )
                                continue;

                            s32 iQ = -1;
                            while( 1 )
                            {
                                // Do tight loop on bbox checks
                                {
                                    fbbox* pBBox = (fbbox*)(CL.pBounds);
                                    iQ++;
                                    while( iQ < CL.nQuads )
                                    {
                                        // Do bbox culling
                                        if( SphereBBox.Intersect( pBBox[iQ] ) ) 
                                        {
                                            break;
                                        }
                                        iQ++;
                                    }
                                    if( iQ==CL.nQuads )
                                        break;
                                }

                                // Process this quad
                                poly_cache::cluster::quad& QD = CL.pQuad[iQ];

                                {
                                    vector3  MoveDelta;
                                    vector3* PT[4];
                                    PT[0] = (vector3*)(&CL.pPoint[ QD.iP[0] ]);
                                    PT[1] = (vector3*)(&CL.pPoint[ QD.iP[1] ]);
                                    PT[2] = (vector3*)(&CL.pPoint[ QD.iP[2] ]);
                                    PT[3] = (vector3*)(&CL.pPoint[ QD.iP[3] ]);

                                    if( ComputeTriSphereMovement( *PT[0], *PT[1], *PT[2], SphereCenter, SphereRadius, SphereRadius, MoveDelta ) )
                                    {
                                        nPenetrations++;
                                        MoveBounds += MoveDelta;
                                        AccumMoveDelta += MoveDelta;
                                        bHitThisCluster = TRUE;
                                    }

                                    if( CL.pBounds[iQ].Flags & BOUNDS_IS_QUAD )
                                    {
                                        if( ComputeTriSphereMovement( *PT[0], *PT[2], *PT[3], SphereCenter, SphereRadius, SphereRadius, MoveDelta ) )
                                        {
                                            nPenetrations++;
                                            MoveBounds += MoveDelta;
                                            AccumMoveDelta += MoveDelta;
                                            bHitThisCluster = TRUE;
                                        }
                                    }
                                }
                            }
                        }
        
                    }

                    // No collisions found so bail
                    if( nPenetrations == 0 )
                        break;

                    vector3 FinalMoveDelta;
                    FinalMoveDelta.X = MoveBounds.Min.X + MoveBounds.Max.X;
                    FinalMoveDelta.Y = MoveBounds.Min.Y + MoveBounds.Max.Y;
                    FinalMoveDelta.Z = MoveBounds.Min.Z + MoveBounds.Max.Z;

                    pSpherePos[i] += FinalMoveDelta;
                }
            }
        }
    }
}

//==============================================================================

void car_object::EnforceConstraints( void )
{
    s32 i;

    s32 nSpheres=0;
    vector3 SphereOffset[16];
    vector3 SpherePos[16];
    f32 SphereRadius[16];

    for( i=0; i<4; i++ )
    {
        SphereOffset[nSpheres]  = m_Wheels[i].m_Offset;
        SpherePos[nSpheres]     = m_Wheels[i].m_Position;
        SphereRadius[nSpheres]  = g_Def.m_WheelRadius;
        nSpheres++;
    }

    SolveSphereCollisions( GetGuid(), 4, SphereOffset, SpherePos, SphereRadius );

    ComputeL2W();

    for( i=0; i<m_nBubbles; i++ )
    {
        SphereOffset[nSpheres]  = m_BubbleOffset[i];
        SpherePos[nSpheres]     = GetL2W() * m_BubbleOffset[i];
        SphereRadius[nSpheres]  = m_BubbleRadius[i];
        nSpheres++;
    }

    SolveSphereCollisions( GetGuid(), nSpheres, SphereOffset, SpherePos, SphereRadius );

    nSpheres = 0;
    for( i=0; i<4; i++ )
    {
        m_Wheels[i].m_Position = SpherePos[nSpheres];
        nSpheres++;
    }

    ComputeL2W();

/*
//    matrix4 OldL2W = GetL2W();

    s32 nMainLoops = 1;
    while( nMainLoops-- )
    {
        //
        // Apply wheel constraints and collision
        //
        {
            s32 nCollisionEnforces=0;
            s32 MaxLoops = 4;
            f32 CorrectionFactor = 1.0f / (f32)MaxLoops;
            s32 nLoops = MaxLoops;
            while( nLoops-- )
            {
                xbool bCorrected = FALSE;

                for( i=0; i<4; i++ )
                {
                    wheel& WA = m_Wheels[i];
                    f32 WAWeight = 0.1f;
                    if(!WA.m_bPassive ) WAWeight += 1.0f;
                    if( WA.m_bHasTraction ) WAWeight += 1.0f;

                    for( j=1; j<=3; j++ )
                    {
                        wheel& WB = m_Wheels[(i+j)%3];

                        vector3 OffsetDelta = WB.m_Offset - WA.m_Offset;
                        f32     ConstLength = OffsetDelta.Length();
                        vector3 Delta       = WB.m_Position - WA.m_Position;
                        f32     Length      = Delta.Length();
                        f32     LenDiff     = ConstLength - Length;

                        if( x_abs(LenDiff) > 1.0f )
                        {
                            vector3 CorrDelta   = (Delta / Length) * LenDiff;
                            WA.m_Position -= CorrDelta * CorrectionFactor;
                            WB.m_Position += CorrDelta * CorrectionFactor;
                            bCorrected = TRUE;
                        }
                    }
                }

                if( !bCorrected )
                    break;
            }

            if( nCollisionEnforces == 0 )
            {
                for( i=0; i<4; i++ )
                    m_Wheels[i].EnforceCollision();
            }
        }

        //
        // Compute new vehicle orientation
        //
        ComputeL2W();
        //
        // Compute new L2W
        //

        // Reset wheel positions
        for( i=0; i<4; i++ )
            m_Wheels[i].m_Position = GetL2W() * m_Wheels[i].m_Offset;
    }
*/

}

//==============================================================================

vector2 SHOCK_HISTORY_FRAME_OFFSET[4] =
{
    vector2(32,64),
    vector2(350,64),
    vector2(32,250),
    vector2(350,250),
};

f32 SHOCK_SPEED_LIMIT = 200.0f;

void car_object::RenderShockHistory( void )
{
    s32 i,j;
    s32 iOffset = m_Wheels[0].m_nLogicLoops%WHEEL_HISTORY_LENGTH;
    f32 WindowWidth = 1*WHEEL_HISTORY_LENGTH;
    f32 WindowHeight = 128;
    xcolor FRAME_COLOR(0,128,0);        // GREEN
    xcolor SHOCK_COLOR(128,128,0);      // YELLOW
    xcolor SHOCK_SPEED_COLOR(128,0,0);  // RED
    xcolor TRACTION_COLOR(0,0,128);     // BLUE

    draw_Begin( DRAW_LINES, DRAW_2D );

    for( i=0; i<4; i++ )
    {

        vector3 LastShockPt;
        vector3 LastShockSpeedPt;
        for( j=0; j<WHEEL_HISTORY_LENGTH; j++ )
        {
            wheel::history& H = m_Wheels[i].m_History[ (iOffset+j+1)%WHEEL_HISTORY_LENGTH ];
            f32 X = SHOCK_HISTORY_FRAME_OFFSET[i].X + j*((f32)WindowWidth/WHEEL_HISTORY_LENGTH);

            vector3 ShockPt(0,0,0);
            f32 ShockT = H.ShockLength / g_Def.m_ShockLength;
            ShockPt.X = X;
            ShockPt.Y = SHOCK_HISTORY_FRAME_OFFSET[i].Y + (WindowHeight/2) - ShockT*(WindowHeight/2);


            vector3 ShockSpeedPt(0,0,0);
            f32 ShockSpeedT = H.ShockSpeed / SHOCK_SPEED_LIMIT;
            if( ShockSpeedT < -1) ShockSpeedT = -1;
            if( ShockSpeedT > +1) ShockSpeedT = +1;
            ShockSpeedPt.X = X;
            ShockSpeedPt.Y = SHOCK_HISTORY_FRAME_OFFSET[i].Y + (WindowHeight/2) - ShockSpeedT*(WindowHeight/2);

            if( j>0 )
            {
                draw_Color( SHOCK_COLOR );
                draw_Vertex( LastShockPt );
                draw_Vertex( ShockPt );

                draw_Color( SHOCK_SPEED_COLOR );
                draw_Vertex( LastShockSpeedPt );
                draw_Vertex( ShockSpeedPt );
            }

            LastShockPt = ShockPt;
            LastShockSpeedPt = ShockSpeedPt;

            // Render if we have traction
            f32 TractionHeight = H.bHasTraction ? (WindowHeight/10) : (WindowHeight/20);
            draw_Color( TRACTION_COLOR );
            draw_Vertex( X, SHOCK_HISTORY_FRAME_OFFSET[i].Y + WindowHeight - TractionHeight, 0 );
            draw_Vertex( X, SHOCK_HISTORY_FRAME_OFFSET[i].Y + WindowHeight, 0 );
        }

        f32 WW = WindowWidth + 4;
        f32 WH = WindowHeight + 4;

        draw_Color( FRAME_COLOR );

        draw_Vertex(SHOCK_HISTORY_FRAME_OFFSET[i].X-2,SHOCK_HISTORY_FRAME_OFFSET[i].Y-2,0);
        draw_Vertex(SHOCK_HISTORY_FRAME_OFFSET[i].X+WW-2,SHOCK_HISTORY_FRAME_OFFSET[i].Y-2,0);

        draw_Vertex(SHOCK_HISTORY_FRAME_OFFSET[i].X-2,SHOCK_HISTORY_FRAME_OFFSET[i].Y+(WH/2)-2,0);
        draw_Vertex(SHOCK_HISTORY_FRAME_OFFSET[i].X+WW-2,SHOCK_HISTORY_FRAME_OFFSET[i].Y+(WH/2)-2,0);

        draw_Vertex(SHOCK_HISTORY_FRAME_OFFSET[i].X+WW-2,SHOCK_HISTORY_FRAME_OFFSET[i].Y-2,0);
        draw_Vertex(SHOCK_HISTORY_FRAME_OFFSET[i].X+WW-2,SHOCK_HISTORY_FRAME_OFFSET[i].Y+WH-2,0);

        draw_Vertex(SHOCK_HISTORY_FRAME_OFFSET[i].X+WW-2,SHOCK_HISTORY_FRAME_OFFSET[i].Y+WH-2,0);
        draw_Vertex(SHOCK_HISTORY_FRAME_OFFSET[i].X-2,SHOCK_HISTORY_FRAME_OFFSET[i].Y+WH-2,0);

        draw_Vertex(SHOCK_HISTORY_FRAME_OFFSET[i].X-2,SHOCK_HISTORY_FRAME_OFFSET[i].Y+WH-2,0);
        draw_Vertex(SHOCK_HISTORY_FRAME_OFFSET[i].X-2,SHOCK_HISTORY_FRAME_OFFSET[i].Y-2,0);
    }

    draw_End();
}

//==============================================================================

// Renders geometry
void car_object::VehicleRender( void )
{
    s32 i;
   
    // Lookup animation pointer
    anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
    if( !pAnimGroup )
        return;
    
    // Allocate matrices
    s32      NBones    = pAnimGroup->GetNBones();
    matrix4* pMatrices = (matrix4*)smem_BufferAlloc(NBones * sizeof(matrix4));
    if (!pMatrices)
        return;

    // Setup matrices
    for (i = 0; i < NBones; i++)
    {
        pMatrices[i] = GetL2W();
        //pMatrices[i].PreTranslate(m_RigidBody.m_RenderOffset);
    }

    // Setup wheel matrices
    for (i = 0; i < 4; i++)
    {
        wheel& Wheel = m_Wheels[i];

        matrix4 VehicleL2W = GetL2W();

        matrix4 L2W;
        L2W.Identity();
        L2W.RotateX( Wheel.m_Spin );
        L2W.RotateY( Wheel.m_Steer );
        L2W.Translate(vector3(0,-Wheel.m_CurrentShockLength,0));
        L2W.Translate(Wheel.m_Offset);

        pMatrices[Wheel.m_iBone] = VehicleL2W * L2W * pAnimGroup->GetBoneBindInvMatrix(Wheel.m_iBone);
    }


    // Draw geometry
    m_RigidInst.Render(pMatrices, render::NORMAL | render::CLIPPED);


    if( m_bDisplayShockInfo )
    {
        // Draw debug info
        draw_ClearL2W();
        for (i = 0; i < 4; i++)
        {
            // Get wheel
            wheel& Wheel = m_Wheels[i];

            // Get pos
    //        vector3 Pos = Wheel.m_Position;

            vector3 WheelDownDir = GetL2W().RotateVector(vector3(0,-1,0));
            vector3 WheelCenter = Wheel.m_Position+WheelDownDir*Wheel.m_CurrentShockLength;
            draw_Sphere(WheelCenter, g_Def.m_WheelRadius, XCOLOR_RED);
        
            /*
            draw_Line(WheelCenter,Wheel.m_Position,XCOLOR_WHITE);
        
            draw_Point(Wheel.m_Position,XCOLOR_WHITE);

            vector3 WheelSteerDir(0,0,1);
            WheelSteerDir.RotateY( Wheel.m_Steer );

            vector3 WheelForward = GetL2W().RotateVector(WheelSteerDir);
            draw_Line( Pos, Pos+WheelForward*150.0f, XCOLOR_GREEN );
            */
        }
    /*
        // Draw bbox
        draw_SetL2W(GetL2W());
        draw_BBox(GetLocalBBox(),XCOLOR_WHITE );
    */
        for( i=0; i<m_nBubbles; i++ )
        {
            vector3 P = GetL2W() * m_BubbleOffset[i];
            draw_Sphere( P, m_BubbleRadius[i], XCOLOR_WHITE );
        }

            RenderShockHistory();
    }
}

//==============================================================================
