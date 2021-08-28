//=============================================================================
//  DEBRIS.CPP
//=============================================================================

//=============================================================================
// INCLUDES
//=============================================================================
#include "debris.hpp"
#include "e_Draw.hpp"
#include "audiomgr\AudioMgr.hpp"
#include "..\Support\GameLib\StatsMgr.hpp"

xbool DEBRIS_USE_POLY_CACHE = TRUE;
//=============================================================================
// OBJECT DESC.
//=============================================================================
static struct debris_desc : public object_desc
{
    debris_desc( void ) : object_desc( 
        object::TYPE_DEBRIS, 
        "Debris", 
        "EFFECTS",
        object::ATTR_NEEDS_LOGIC_TIME   |
        object::ATTR_RENDERABLE         | 
        object::ATTR_NO_RUNTIME_SAVE,
        FLAGS_IS_DYNAMIC  )
    {}
    
    virtual object* Create          ( void )
    {
        return new debris;
    }
    
} s_debris_desc ;

//=============================================================================================

const object_desc&  debris::GetTypeDesc ( void ) const
{
    return s_debris_desc;
}

//=============================================================================================

const object_desc&  debris::GetObjectType ( void )
{
    return s_debris_desc;
}

//=============================================================================================
debris::debris(void) :
    m_KillMe(false),
    m_Elasticity(0.4f),
    m_IgnoreLiving(false)
{

    m_BounceSoundID = -1;
    m_Spin.Zero();
    m_TotalSpin.Zero();
    m_Inactive  = false;
    m_IgnoreCollision = TRUE;
}

//=============================================================================================

debris::~debris()
{
 
}

//=============================================================================

void debris::OnInit( void )
{
    
}

//=============================================================================

void debris::Create( const char*            rigidFileName,
                     const vector3&         startingPosition,
                     const vector3&         startingVelocity,
                     f32                    lifeSpan,
                     xbool                  bBounces )
{
    (void)rigidFileName;
    (void)startingVelocity;
    
    OnMove              ( startingPosition );
    m_Velocity          = startingVelocity ;

    m_LifeSpan          = lifeSpan;

    m_BounceCount = 0;

    m_IgnoreCollision = !bBounces;
}

//=============================================================================

void debris::Create( rigid_inst&            Inst,
                     const vector3&         startingPosition,
                     const vector3&         startingVelocity,
                     f32                    lifeSpan,
                     xbool                  bBounces )
{
    (void)Inst;
    (void)startingVelocity;
    
    OnMove              ( startingPosition );
    m_Velocity          = startingVelocity ;

    m_LifeSpan          = lifeSpan;

    m_BounceCount = 0;

    m_IgnoreCollision = !bBounces;
}

//=============================================================================================

bbox debris::GetLocalBBox( void ) const 
{ 
    bbox BBox;
    BBox.Set( vector3( -10, -10, -10 ), vector3( 10, 10, 10) );
    return BBox;
}

//=============================================================================================

void debris::OnAdvanceLogic      ( f32 DeltaTime )
{
    LOG_STAT( k_stats_Debris );
    CONTEXT("debris::OnAdvanceLogic");

    m_LifeSpan -= DeltaTime;

    UpdatePhysics( DeltaTime );
    vector3 newPos = OnProcessCollision( DeltaTime );
    matrix4 NewL2W( vector3( 1.0f, 1.0f, 1.0f ), m_TotalSpin, newPos );
    OnTransform( NewL2W );

    if( (m_LifeSpan < 0.0f) || m_KillMe || (newPos.GetY() < -8000.0f) )
    {
        g_ObjMgr.DestroyObject( GetGuid( ) );
    }
}

//=============================================================================================

void debris::UpdatePhysics       (  f32 DeltaTime )
{
    f32 gravity = 980.0f;
    m_Velocity.GetY() -= DeltaTime * gravity ;

    // This is being done at render time in the child classes
    //m_RenderL2W = GetL2W();
    //m_RenderL2W.SetRotation( m_TotalSpin );
}

//=============================================================================================

void debris::OnMove				( const vector3& rNewPos )
{
    object::OnMove( rNewPos );
}

//=============================================================================================

void debris::OnRender            ( void )
{
#if !defined( CONFIG_RETAIL )
    draw_Sphere( GetPosition(), GetLocalBBox().GetRadius() );
#endif // !defined( CONFIG_RETAIL )
}

//=============================================================================

void debris::OnBounce(void )
{
    if( (m_BounceSoundID != -1) && m_BounceCount == 0 )
    {
        g_AudioMgr.PlayVolumeClipped( g_StringMgr.GetString( m_BounceSoundID ), GetPosition(), GetZone1(), TRUE );
    }
    m_BounceCount++;   
}

//=============================================================================

vector3 debris::OnProcessCollision( f32 DeltaTime )
{
    CONTEXT("debris::OnProcessCollision");

    if(m_Inactive)
        return GetPosition();

    //
    // Compute new positions
    //
    vector3 CurrentPos      = GetPosition();
    vector3 DeltaPos        = m_Velocity * DeltaTime;
    vector3	DesiredPos	    = CurrentPos + DeltaPos;

    //
    // If ignoring collisions just return
    //
    if( m_IgnoreCollision )
        return DesiredPos;

    //
    // Collect collisions
    //
    g_CollisionMgr.SphereSetup( GetGuid(), CurrentPos, DesiredPos, GetBBox().GetRadius() );
    g_CollisionMgr.UseLowPoly();
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_SMALL_DEBRIS, (object::object_attr)(object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING) );

    // If there were no collisions then just return desired
    if( g_CollisionMgr.m_nCollisions == 0)
        return DesiredPos;

    // Get speed and normalized direction
    f32     Speed = DeltaPos.Length();
    vector3 Dir   = DeltaPos / Speed;

    // Process the collision
    f32 DistanceToMove = ( Speed * g_CollisionMgr.m_Collisions[0].T ) - 0.5f;
    if( DistanceToMove < 0.0f )
        DistanceToMove = 0.0f;
    vector3 NewPos = CurrentPos + DistanceToMove*Dir;

    //  if the velocity squared is less than slow velocity squared (100*100 = 10,000)
    const f32 minVelocityToWorryAbout = 100.0f;
    if( m_Velocity.LengthSquared() < (minVelocityToWorryAbout * minVelocityToWorryAbout) )
    {
        m_Inactive = TRUE;
        m_Velocity.Zero();
        return NewPos;
    }

    // Register a bounce for audio purposes
    OnBounce();

    // Reflect velocity for bounce
    m_Velocity = g_CollisionMgr.m_Collisions[0].Plane.ReflectVector( m_Velocity );
    m_Velocity = m_Velocity * m_Elasticity;
    static radian BounceVelocityChange = R_30;
    f32 FinalBounceVelocityChange = BounceVelocityChange; /* / m_BounceCount;*/
    {
        f32 x = x_frand( -FinalBounceVelocityChange, FinalBounceVelocityChange );
        f32 y = x_frand( -FinalBounceVelocityChange, FinalBounceVelocityChange );
        m_Velocity.Rotate( radian3( x, y, 0.0f ) );
    }

    static radian BounceSpinRate = R_60;
    {
        f32 x = x_frand(-BounceSpinRate,BounceSpinRate);
        f32 y = x_frand(-BounceSpinRate,BounceSpinRate);
        f32 z = x_frand(-BounceSpinRate,BounceSpinRate);
        m_Spin = radian3( x, y, z );
    }

    return NewPos;
}

//=============================================================================


