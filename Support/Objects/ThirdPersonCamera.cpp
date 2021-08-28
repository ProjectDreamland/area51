//==============================================================================
//
//  ThirdPersonCamera.cpp
//
//==============================================================================

#include "ThirdPersonCamera.hpp"
#include "Obj_mgr\obj_mgr.hpp"

//=========================================================================
// DEFINES and CONSTS
//=========================================================================

static f32 DISTANCE_MIN_ACCELERATION    = 100.0f;
static f32 CAMERA_SPHERE_RADIUS         = 20.0f;

//static f32 DISTANCE_MAX_ACCELERATION    = 1000.0f;
//static f32 DISTANCE_ACCELERATION_ACCELERATION = 500.0f;
//static f32 DISTANCE_MAX_SPEED           = 350.0f;
//static f32 PITCH_SPEED                  = R_70;
//static f32 YAW_SPEED                    = R_70;
/*
static f32 ORBIT_MAX_SPEED              = 5000.0f;
static f32 ORBIT_MIN_SPEED              = 200.0f;
static f32 ORBIT_ACCELERATION           = 15000.0f;
static f32 ORBIT_FAST_RANGE             = 200.0f;
static f32 ORBIT_SLOW_RANGE             = 50.0f;
*/

//=========================================================================
// THIRD_PERSON_CAMERA
//=========================================================================

static struct third_person_camera_desc : public object_desc
{
    third_person_camera_desc( void ) : object_desc( 
            object::TYPE_THIRD_PERSON_CAMERA,
            "Third Person Camera", 
            "",
            object::ATTR_NEEDS_LOGIC_TIME,
            0 )
            {}

    virtual object* Create( void ) { return new third_person_camera; }

#ifdef X_EDITOR
    s32 OnEditorRender( object& Object ) const
    {
        (void)Object;
        //if( Object.GetAttrBits() & object::ATTR_EDITOR_SELECTED )
        //    Object.OnDebugRender();

        return -1;
    }
#endif // X_EDITOR

} s_Third_Person_Camera;

//=========================================================================
// VARIABLES
//=========================================================================

//=========================================================================
// FUNCTIONS
//=========================================================================

third_person_camera::third_person_camera( void ) :
    m_OrbitPoint            ( 0.0f, 0.0f, 0.0f ),
    m_DesiredOrbitPoint     ( 0.0f, 0.0f, 0.0f ),
    m_OrbitPointVelocity    ( 0.0f, 0.0f, 0.0f ),
    m_Distance              ( 0.0f ),
    m_DesiredDistance       ( 0.0f ),
    m_DistanceVelocity      ( 0.0f ),
    m_DistanceAcceleration  ( DISTANCE_MIN_ACCELERATION ),
    m_Pitch                 ( R_0 ),
    m_DesiredPitch          ( R_0 ),
    m_PitchVelocity         ( R_0 ),
    m_Yaw                   ( R_0 ),
    m_DesiredYaw            ( R_0 ),
    m_YawVelocity           ( R_0 ),
    m_HostPlayerGuid        ( NULL_GUID )
{
}

//==============================================================================

const object_desc& third_person_camera::GetTypeDesc( void ) const
{
    return s_Third_Person_Camera;
}

//==============================================================================

const object_desc& third_person_camera::GetObjectType( void )
{
    return s_Third_Person_Camera;
}

//==============================================================================

void third_person_camera::ComputeView( view& View ) const
{
    //View.OrbitPoint( m_OrbitPoint, m_Distance, m_Pitch, m_Yaw );
    View.LookAtPoint( m_CameraPos, m_DesiredOrbitPoint );
}

//==============================================================================

void third_person_camera::MoveTowards( const vector3& DesiredPosition )
{
    view View;
    View.OrbitPoint( m_DesiredOrbitPoint, m_DesiredDistance, m_DesiredPitch, m_DesiredYaw );
    View.SetPosition( DesiredPosition );
    View.GetPitchYaw( m_DesiredPitch, m_DesiredYaw );
    m_DesiredDistance = (View.GetPosition() - m_DesiredOrbitPoint).Length();
}

//==============================================================================

void third_person_camera::MoveTowards( radian Pitch, radian Yaw, f32 Distance )
{
    m_DesiredPitch      = Pitch;
    m_DesiredYaw        = Yaw;
    m_DesiredDistance   = Distance;
}

//==============================================================================

void third_person_camera::SetOrbitPoint( const vector3& DesiredOrbitPoint )
{
    m_DesiredOrbitPoint = DesiredOrbitPoint;
}

//==============================================================================

void third_person_camera::OnAdvanceLogic( f32 DeltaTime )
{

    object::OnAdvanceLogic( DeltaTime );
/*
    //
    // Update pitch
    //
    radian Diff = x_MinAngleDiff( m_Pitch, m_DesiredPitch );

    if ( Diff < R_0 )
    {
        m_Pitch += PITCH_SPEED * DeltaTime;

        if ( x_MinAngleDiff( m_Pitch, m_DesiredPitch ) > R_0 )
        {
            m_Pitch = m_DesiredPitch;
        }
    }
    else if ( Diff > R_0 )
    {
        m_Pitch -= PITCH_SPEED * DeltaTime;

        if ( x_MinAngleDiff( m_Pitch, m_DesiredPitch ) < R_0 )
        {
            m_Pitch = m_DesiredPitch;
        }
    }

    //
    // Update Yaw
    //
    Diff = x_MinAngleDiff( m_Yaw, m_DesiredYaw );
    if ( Diff < R_0 )
    {
        m_Yaw += YAW_SPEED * DeltaTime;

        if ( x_MinAngleDiff( m_Yaw, m_DesiredYaw ) > R_0 )
        {
            m_Yaw = m_DesiredYaw;
        }
    }
    else if ( Diff > R_0 )
    {
        m_Yaw -= YAW_SPEED * DeltaTime;

        if ( x_MinAngleDiff( m_Yaw, m_DesiredYaw ) < R_0 )
        {
            m_Yaw = m_DesiredYaw;
        }
    }

    //
    // Update the distance
    //
    m_DistanceAcceleration += DISTANCE_ACCELERATION_ACCELERATION * DeltaTime;
    if ( m_DistanceAcceleration > DISTANCE_MAX_ACCELERATION )
    {
        m_DistanceAcceleration = DISTANCE_MAX_ACCELERATION;
    }

    if ( x_abs( m_Distance - m_DesiredDistance ) > 1.0f )
    {
        if ( m_Distance < m_DesiredDistance )
        {
            m_DistanceVelocity += m_DistanceAcceleration * DeltaTime;

            if ( m_DistanceVelocity > DISTANCE_MAX_SPEED )
            {
                m_DistanceVelocity = DISTANCE_MAX_SPEED;
            }
        }
        else 
        {
            m_DistanceVelocity -= m_DistanceAcceleration * DeltaTime;

            if ( m_DistanceVelocity < -DISTANCE_MAX_SPEED )
            {
                m_DistanceVelocity = -DISTANCE_MAX_SPEED;
            }
        }
        
        f32 NewDistance = m_Distance + m_DistanceVelocity * DeltaTime;
        if (   ((m_Distance < m_DesiredDistance) && (NewDistance > m_DesiredDistance ))
            || ((m_Distance > m_DesiredDistance) && (NewDistance < m_DesiredDistance )) )
        {
            NewDistance = m_DesiredDistance;
            m_DistanceVelocity = 0.0f;
        }

        m_Distance = NewDistance;
    }
    else
    {
        m_Distance = m_DesiredDistance;
        m_DistanceVelocity = 0.0f;
    }
*/

    /*
    f32 DeltaDist = m_DesiredDistance - m_Distance;
    f32 MaxDeltaDist = 400.0f * DeltaTime;

    DeltaDist = MINMAX( -MaxDeltaDist, DeltaDist, MaxDeltaDist );

    m_Distance += DeltaDist;
*/

/*
    //
    // Update the orbit point
    //
    view PreOrbitView;
    ComputeView( PreOrbitView ); // store out what we have so far, since changing
                                 // the orbit point can push the camera through stuff

    vector3 Delta( m_DesiredOrbitPoint - m_OrbitPoint );
    if ( Delta.LengthSquared() > 1.0f )
    {
        // update velocity
        vector3 DeltaVelocity( Delta );
        DeltaVelocity.NormalizeAndScale( ORBIT_ACCELERATION * DeltaTime );
        m_OrbitPointVelocity += DeltaVelocity;

        // cap the speed
        f32 MaxSpeed;
        const f32 Length = Delta.Length();
        if ( Length > ORBIT_FAST_RANGE )
        {
            MaxSpeed = ORBIT_MAX_SPEED;
        }
        else if ( Length < ORBIT_SLOW_RANGE )
        {
            MaxSpeed = ORBIT_MIN_SPEED;
        }
        else
        {
            // y= mx+b
            f32 Rise = ORBIT_MAX_SPEED - ORBIT_MIN_SPEED;
            f32 Run = ORBIT_FAST_RANGE - ORBIT_SLOW_RANGE;
            f32 m = Rise / Run;
            MaxSpeed = m * Length + ORBIT_MIN_SPEED;
        }

        if ( m_OrbitPointVelocity.LengthSquared() > (MaxSpeed * MaxSpeed) )
        {
            m_OrbitPointVelocity.NormalizeAndScale( MaxSpeed );
        }

        // cap our move amount
        vector3 MoveAmount( m_OrbitPointVelocity * DeltaTime );
        if ( MoveAmount.LengthSquared() > Delta.LengthSquared() )
        {
            MoveAmount = Delta;
        }

        // move
        m_OrbitPoint += MoveAmount;

    }
    else
    {
        m_OrbitPoint = m_DesiredOrbitPoint;
        m_OrbitPointVelocity.Zero();
    }



    if ( !HaveClearView() )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( g_CollisionMgr.m_Collisions[0].ObjectHitGuid );
        pObj->GetPosition();
        view View;
        ComputeView( View );
        vector3 Pos( View.GetPosition() );
        vector3 Delta( m_OrbitPoint - Pos );
        Delta *= g_CollisionMgr.m_Collisions[0].T;

        m_Distance = Delta.Length();
        m_DistanceAcceleration = 0.0f;
        m_DistanceVelocity = 0.0f;
    }
*/


/*
    // SH: I'm disabling this test code for now.
    // Going into the lan party build, we don't want the camera
    // moving at all, in order to elimiate zoning issues.
    
    // All we want to do for now is determine which endpoint we want to be at
    // and move toward it.
    f32 DistSq[2];
    DistSq[0] = (m_DesiredOrbitPoint - m_CameraRodEnds[0]).LengthSquared();
    DistSq[1] = (m_DesiredOrbitPoint - m_CameraRodEnds[1]).LengthSquared();

    if (DistSq[0] > DistSq[1])
        m_iDesiredRodEnd = 0;
    else
        m_iDesiredRodEnd = 1;

    // Move the camera pos toward the desired endpoint;
    f32 TAlongRod = ((m_CameraPos - m_CameraRodEnds[0]).Length()) / m_CameraRodLength;
    f32 MaxMove   = (800.0f / m_CameraRodLength) * DeltaTime;
    f32 Sign = (m_iDesiredRodEnd==0)?-1:1;   
    f32 NewT = TAlongRod + (MaxMove * Sign);

    NewT = MINMAX(0,NewT,1);

    m_CameraPos = m_CameraRodEnds[1] - m_CameraRodEnds[0];
    m_CameraPos *= NewT;
    m_CameraPos += m_CameraRodEnds[0];
*/



    // Now that everything is updated, handle the zone tracking
    view View;
    ComputeView( View );
    g_ZoneMgr.UpdateZoneTracking( *this, m_ZoneTracker, View.GetPosition() );
}

//==============================================================================

void third_person_camera::RotateYaw( radian DeltaYaw )
{
    m_DesiredYaw += DeltaYaw;    
}

//==============================================================================

void third_person_camera::MoveTowardsPitch( radian NewPitch )
{
    m_DesiredPitch = NewPitch;    
}

//==============================================================================

xbool third_person_camera::CheckForObstructions( const vector3& Dir, f32 DistToCheck, f32& MaxDistFound )
{
    // Assume we have a clear line of sight.
    MaxDistFound = DistToCheck;

    // Figure out the end point for our ray test.
    vector3 Delta = Dir;
    Delta.NormalizeAndScale( DistToCheck );
    f32 Length = Delta.Length();

    // Do a ray test first to see how far we can move the camera.
    g_CollisionMgr.RaySetup( GetGuid(), m_OrbitPoint, m_OrbitPoint+Delta );
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
        (object::object_attr)(object::ATTR_BLOCKS_CHARACTER | object::ATTR_BLOCKS_CHARACTER_LOS),
        (object::object_attr)(object::ATTR_COLLISION_PERMEABLE) );

    s32 nRayCollisions = g_CollisionMgr.m_nCollisions;
    if( nRayCollisions > 0 )
    {
        // We hit something
        MaxDistFound = g_CollisionMgr.m_Collisions[0].T * Length;
        if (MaxDistFound < (CAMERA_SPHERE_RADIUS+5))
        {
            // The camera sphere would intersect geometry immediately
            // No point in doing the sphere test.  Just bail.
            MaxDistFound = 0;
            return TRUE;
        }
    }

    // The above collision check was a good start, but there are too many cracks in the world,
    // and places where we can squeeze through parts we shouldn't. (One example would be the
    // camera going through a stairwell.)

    // Next, do a sphere test to see how far back the camera can get and
    // still have a good shot of the target
    g_CollisionMgr.SphereSetup( GetGuid(), m_OrbitPoint, m_OrbitPoint+Delta, CAMERA_SPHERE_RADIUS );
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
        (object::object_attr)(object::ATTR_BLOCKS_CHARACTER | object::ATTR_BLOCKS_CHARACTER_LOS),
        (object::object_attr)(object::ATTR_COLLISION_PERMEABLE) );

    // What was the distance from our sphere check?
    f32 SphereMaxDist;
    if( g_CollisionMgr.m_nCollisions )
        SphereMaxDist = DistToCheck * g_CollisionMgr.m_Collisions[0].T;
    else
        SphereMaxDist = DistToCheck;
    
    // Did we shoot out of the world?
    if ( ((g_CollisionMgr.m_nCollisions == 0) && nRayCollisions) ||
         (SphereMaxDist > MaxDistFound) )
    {
        // This means we probably shot out of the world, or at least outside of a wall
        // that we were pressed against. If the camera angle is oblique enough and the
        // sphere is already intersecting a wall, then the ray test might have a collision
        // where the sphere test would fail or the sphere would intersect a further object.
        // We'll assume we could've gone outside the world and mark this as an obstructed
        // view.
        MaxDistFound = 0;
        return TRUE;
    }

    MaxDistFound = SphereMaxDist;

    // No collisions from the sphere test? Our camera is good to go...
    if( g_CollisionMgr.m_nCollisions == 0 )
        return FALSE;

    return TRUE;
}

//==============================================================================

void third_person_camera::Setup( const vector3& InitialOrbitPoint, const vector3& IdealAimDirection, f32 StartDist, f32 EndDist, object* pOrbitObject )
//( const vector3& OrbitPoint, radian Pitch, radian Yaw, f32 Distance )
{
    radian P,Y;
    IdealAimDirection.GetPitchYaw(P,Y);

    m_OrbitPoint = InitialOrbitPoint;

    matrix4 L2W;
    L2W.Setup( vector3(1,1,1),radian3(0,0,0),m_OrbitPoint );
    OnTransform( L2W );

    s32 i;

    P = -R_20;
    
    f32     BestDist    = 0;
    radian  BestY       = x_ModAngle( -Y );
    radian  StepY       = R_360 / 16;

    
    // We'll try the prefered yaw, but then step around a circle looking for something better
    for (i=0;i<16;i++)
    {
        vector3 Dir(0,0,MAX(EndDist,StartDist));
        Dir.RotateX( P );
        Dir.RotateY( Y );
        Dir.Normalize();

        f32 DistFound;

        if (CheckForObstructions( Dir, StartDist, DistFound ))
        {
            if (DistFound > BestDist)
            {
                BestDist = DistFound;
                BestY    = Y;
            }
        }
        else
        {
            // We found an unobstructed direction!
            BestY = Y;
            BestDist = StartDist;
            break;
        }

        Y += StepY;
        if (Y > R_360)
            Y -= R_360;
    }

    m_Distance = BestDist;
    m_Pitch = P;
    m_Yaw = BestY;    
    m_DistanceAcceleration = DISTANCE_MIN_ACCELERATION;
/*
    // See if this position is going to work for us
    if ( !HaveClearView() )
    {
        view View;
        ComputeView( View );
        m_Distance = (m_OrbitPoint-View.GetPosition()).Length() * g_CollisionMgr.m_Collisions[0].T;
    }
*/
    m_CameraPos(0,0,m_Distance);
    m_CameraPos.RotateX( m_Pitch );
    m_CameraPos.RotateY( m_Yaw );
    m_CameraPos += InitialOrbitPoint;

    m_CameraRodEnds[0] = InitialOrbitPoint;
    m_CameraRodEnds[1] = m_CameraPos;
    m_CameraRodLength  = (m_CameraRodEnds[0] - m_CameraRodEnds[1]).Length();
    m_iDesiredRodEnd = 1;    

    //MoveTowards( P+R_5,Y+R_15, EndDist );

    ASSERT( pOrbitObject );    
    SetZones( pOrbitObject->GetZones() );    
    g_ZoneMgr.InitZoneTracking( *pOrbitObject, m_ZoneTracker );
    g_ZoneMgr.UpdateZoneTracking( *this, m_ZoneTracker, m_CameraPos );
    
}

//==============================================================================
xbool third_person_camera::HaveClearView( void ) const
{
    view View;
    ComputeView( View );

    vector3 NewPos( View.GetPosition() );
    g_CollisionMgr.SphereSetup( m_HostPlayerGuid, m_OrbitPoint, NewPos, 10.0f );
    g_CollisionMgr.UseLowPoly();
    g_CollisionMgr.SetMaxCollisions(MAX_COLLISION_MGR_COLLISIONS);
    g_CollisionMgr.CheckCollisions( 
        object::TYPE_ALL_TYPES,                                 // these types
        object::ATTR_COLLIDABLE,                                // these attributes
        object::ATTR_PLAYER | object::ATTR_CHARACTER_OBJECT );  // not these attributes

    const xbool Result = !(g_CollisionMgr.m_nCollisions > 0);
    return Result;
}

