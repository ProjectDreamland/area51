//=============================================================================================
// HOMING PROJECTILE
//=============================================================================================

//=============================================================================================
// INCLUDES
//=============================================================================================
#include "ProjectileHoming.hpp"
#include "Entropy\e_Draw.hpp"
#include "audiomgr\audiomgr.hpp"
#include "render\LightMgr.hpp"
#include "Objects\actor\actor.hpp"
#include "Decals\DecalMgr.hpp"
#include "Characters\GenericNPC\GenericNPC.hpp"

//=============================================================================================
// STATICS & CONSTANTS
//=============================================================================================

f32 homing_projectile::s_SeekerGrenade_Alert_Time = 1.2f;

const   xcolor  g_SeekerGrenadeTracer( 49, 49, 250, 200 );
const   f32     g_FadeTime = 0.4f;
const f32       k_MinTimeBetweenBroadcasts = 0.5f;

struct homing_tweaks
{
    homing_tweaks( void );
    f32 m_InitialBlendPercent;
    f32 m_InitialVelocityBlendTime;
};

homing_tweaks::homing_tweaks() :
    m_InitialBlendPercent       ( 1.0f ),
    m_InitialVelocityBlendTime  ( 1.0f )
{
}

homing_tweaks g_HomingTweaks;

//=============================================================================================
// OBJECT DESC
//=============================================================================================
static struct homing_projectile_desc : public object_desc
{
    homing_projectile_desc( void ) : object_desc( 
            object::TYPE_HOMING_PROJECTILE , 
            "Homing Projectile", 
            "WEAPON",
            object::ATTR_NEEDS_LOGIC_TIME   |
            object::ATTR_NO_RUNTIME_SAVE    |
            object::ATTR_RENDERABLE,
            FLAGS_IS_DYNAMIC )
            {}

    virtual object* Create          ( void )
    {
        return new homing_projectile;
    }

} s_Homing_Projectile_Desc;

//=============================================================================================

const object_desc&  homing_projectile::GetTypeDesc     ( void ) const
{
    return s_Homing_Projectile_Desc;
}


//=============================================================================================
const object_desc&  homing_projectile::GetObjectType   ( void )
{
    return s_Homing_Projectile_Desc;
}

//=============================================================================================

homing_projectile::homing_projectile() :
    m_Target                    ( 0                 ),
    m_NoTargetPosition          ( 0.0f, 0.0f, 0.0f  ),
    m_YawRate                   ( 0                 ),
    m_MaxYawRate                ( R_60              ),
    m_YawIncreaseRate           ( R_40              ),
    m_LaunchConeSize            ( R_20              ),
    m_AccelerationPercent       ( 1.0f              ),
    m_SpeedPercent              ( 0.0f              ),
    m_AliveTime                 ( 0.0f              ),
    m_TimeSinceLastBroadcast    ( 0.0f              ),
	m_MaxAliveTime              ( 10.0f             ),
    m_InitialVelocity           ( 0.0f, 0.0f, 0.0f  ),
    m_InitialPosition           ( 0.0f, 0.0f, 0.0f  ),
    m_Traction                  ( 1.0f              ),
    m_DebugGoToPosition         ( 0.0f, 0.0f, 0.0f  ),
    m_bBroadcastAlert           ( TRUE              ),
    m_bFirstUpdate              ( 1                 ),
    m_bDestroyAfterLogic        ( FALSE             )
{
}

//=============================================================================================

homing_projectile::~homing_projectile()
{
    s32 a = 0; ++a;
}

//=============================================================================================

void homing_projectile::Initialize( const vector3&        InitPos,
                                  const radian3&        InitRot,
                                  const vector3&        InheritedVelocity,
                                        f32             Speed,
                                        guid            OwnerGuid,
                                        pain_handle     PainHandle,
                                        xbool           bHitLiving )
{
    base_projectile::Initialize( InitPos, InitRot, InheritedVelocity, Speed, OwnerGuid, PainHandle, bHitLiving );
    m_InitialVelocity = m_Velocity;
    m_InitialPosition = m_CurrentPosition;
}

//=============================================================================

bbox homing_projectile::GetLocalBBox( void ) const 
{ 
    return( bbox( vector3( 10.0f, 10.0f, 10.0f),
                  vector3(-10.0f,-10.0f,-10.0f) ) );
}

//=============================================================================

void homing_projectile::UpdatePhysics( const f32& DeltaTime )
{
#if defined(mreed)
    LOG_MESSAGE( xfs( "%0xd", GetGuid() ), "Velocity:( %5.2f, %5.2f, %5.2f ) Pos:( %5.2f, %5.2f, %5.2f )", m_Velocity.GetX(), m_Velocity.GetY(), m_Velocity.GetZ(), GetPosition().GetX(), GetPosition().GetY(), GetPosition().GetZ() ); 
#endif
    f32 fVelocity = m_Velocity.Length();
    if (fVelocity == 0.0f)
    {
        return;
    }

    if (m_bFirstUpdate)
    {
        
        m_bFirstUpdate = FALSE;
        
        vector3 Vel( m_Velocity );
        radian  P,Y;
        Vel.GetPitchYaw(P,Y);
        P += x_frand( -m_LaunchConeSize/2, m_LaunchConeSize/2 );
        Y += x_frand( -m_LaunchConeSize/2, m_LaunchConeSize/2 );

        Vel.Set(0,0,m_Velocity.Length());
        Vel.Rotate( radian3(P,Y,0) );
        m_Velocity = Vel;
    }

    //
    // Blend in our initial velocity
    //
    if ( m_AliveTime < g_HomingTweaks.m_InitialVelocityBlendTime )
    {
        // how far are we through our blend time
        const f32 BlendTimePercent = m_AliveTime / g_HomingTweaks.m_InitialVelocityBlendTime;
        const f32 BlendPercent = (g_HomingTweaks.m_InitialBlendPercent * (1 - BlendTimePercent));
        m_Velocity += (m_InitialVelocity * BlendPercent);
    }
    
    OnProcessCollision( DeltaTime );
}

//=============================================================================

void homing_projectile::OnAdvanceLogic( f32 DeltaTime )
{
    m_AliveTime += DeltaTime;
    m_TimeSinceLastBroadcast += DeltaTime;
    m_SpeedPercent += m_AccelerationPercent * DeltaTime;
    m_SpeedPercent = MIN( 1.0f, m_SpeedPercent );

    if (m_AliveTime <= m_MaxAliveTime)
    {
        UpdateVelocity( DeltaTime );

        if (m_bBroadcastAlert && (m_AliveTime > (m_MaxAliveTime - s_SeekerGrenade_Alert_Time )))
        {
            //give 1 second warning
            alert_package NotifyPackage ;
            NotifyPackage.m_Origin = GetGuid() ;
            NotifyPackage.m_Position = GetPosition();
            NotifyPackage.m_Type = alert_package::ALERT_TYPE_GRENADE ;
            NotifyPackage.m_Target = alert_package::ALERT_TARGET_NPC ;
            NotifyPackage.m_Cause = m_OwnerGuid ;
            NotifyPackage.m_FactionsSpecific = FACTION_NOT_SET;
            NotifyPackage.m_ZoneID = GetZone1();
            g_AudioManager.AppendAlertReceiver(NotifyPackage);

            m_bBroadcastAlert = FALSE;
        }

        UpdatePhysics( DeltaTime );
        OnMove( m_CurrentPosition );
    }

    if ( m_bDestroyAfterLogic || (m_AliveTime > m_MaxAliveTime) )
    {
        g_ObjMgr.DestroyObject( GetGuid() );
    }

}

//=============================================================================

void homing_projectile::OnMove( const vector3& NewPos )
{
    if( !(GetAttrBits() & ATTR_DESTROY) )
    {        
        object::OnMove(NewPos);
    }
}

//=========================================================================

void homing_projectile::OnRender( void )
{
    base_projectile::OnRender();

#ifdef X_EDITOR
    //draw_Line( GetPosition() , GetPosition() + m_Velocity , XCOLOR_BLUE );
#endif // X_EDITOR
}

//=========================================================================

vector3 homing_projectile::GetAimAtPosition( void )
{
    vector3 AimAt;

    if ( m_Target )
    {
        object *pTargetObject = g_ObjMgr.GetObjectByGuid( m_Target );
        if( pTargetObject )
        {
            AimAt = GetTargetPoint(pTargetObject);
        }
        else
        {
            // bad target guid
            m_Target = NULL_GUID;
            vector3 FarAway( m_Velocity * 100000.0f );
            m_NoTargetPosition = GetPosition() + FarAway;
            AimAt = m_NoTargetPosition;
        }
    }
    else
    {
        AimAt = m_NoTargetPosition;
    }

    return AimAt;
}
//=========================================================================

void homing_projectile::UpdateVelocity( f32 DeltaTime )
{
    // get the vector to target.
    vector3 TargetPosition( GetAimAtPosition( ) );

    vector3     ToTarget            ( TargetPosition - GetPosition() );
    vector3     InitialToTarget     ( TargetPosition - m_InitialPosition );
    vector3     InitialTargetDir    ( InitialToTarget );
    InitialTargetDir.Normalize();
    const f32   TargetDist          = ToTarget.Length();
    const f32   InitialTargetDist   = InitialToTarget.Length();
    f32         FinalTraction;

    m_YawRate += m_YawIncreaseRate * m_SpeedPercent * DeltaTime;
    const f32 PercentToTarget = MAX( (InitialTargetDist - TargetDist) / InitialTargetDist, 0.1f );
    f32 CurMaxYawRate = PercentToTarget * m_MaxYawRate;
    m_YawRate = MIN( m_YawRate, CurMaxYawRate );


    static f32 VALUE = 200.0f; // maximum distance ahead (traction point)
    if ( TargetDist < VALUE )
    {
        f32 Total = VALUE - m_Traction;
        f32 Percent = TargetDist / VALUE;
        FinalTraction = Total * Percent;
    }
    else
    {
        FinalTraction = MIN( TargetDist, m_Traction );
    }
    vector3     GoToPosition        ( TargetPosition - (InitialTargetDir * (TargetDist - FinalTraction) ) );
    vector3     ToGoTo              ( GoToPosition - GetPosition() );
    vector3     GoToDir             ( ToGoTo );
    GoToDir.Normalize();

    m_DebugGoToPosition = GoToPosition;

    vector3 OurDir( m_Velocity );
    OurDir.Normalize();       

    radian RotAngle = x_acos( GoToDir.Dot( OurDir ) );
    radian RotAmount = m_YawRate * DeltaTime;

    if( RotAngle < RotAmount )
    {
        m_Velocity = GoToDir;
        m_Velocity.NormalizeAndScale(m_Speed);
    }
    else
    {
        vector3 Axis;
        radian  DummyRotation;
        OurDir.GetRotationTowards( GoToDir, Axis, DummyRotation );

        matrix4 Rot;
        Rot.Setup( Axis, RotAmount );
        OurDir = Rot.Transform( OurDir );

        const f32 SpeedScale = x_sqrt( ABS( OurDir.Dot( GoToDir )));

        m_Velocity = OurDir;
        m_Velocity.NormalizeAndScale(m_Speed * SpeedScale * m_SpeedPercent );

    }
}
//=============================================================================================

void homing_projectile::OnEnumProp( prop_enum& List )
{
    base_projectile::OnEnumProp( List );
        
    List.PropEnumHeader( "Homing Projectile",                        "Homing Projectile Properties", 0 );
    List.PropEnumAngle ( "Homing Projectile\\Max Yaw Rate",          "How many degrees can the projectile turn per second.", 0 );
    List.PropEnumAngle ( "Homing Projectile\\Yaw Increase Rate",     "How many degrees does the projectile yaw rate increase per second.  It starts at zero at launch, and increases until it reaches the max rate.  This property controls how quickly the yaw increases.", 0 );
    List.PropEnumAngle ( "Homing Projectile\\Launch Cone Size",      "How large is the launch cone.  The cone determines how much the shot spreads when it is fired.", 0 );
    List.PropEnumFloat ( "Homing Projectile\\Acceleration Percent",  "Acceleration from start, in percent of max speed per second. Range from 0-1", 0 );
    List.PropEnumFloat ( "Homing Projectile\\Traction",              "Distance, greater than 0 that the parasite uses for wandering. Higher traction means less wandering.", 0 );
}

//=============================================================================


xbool homing_projectile::OnProperty( prop_query& I )
{
    if ( base_projectile::OnProperty( I ) )
    {
        // Nothing to do
    }
    else if ( I.VarAngle( "Homing Projectile\\Max Yaw Rate", m_MaxYawRate ))
    {
        // Nothing to do
    }
    else if ( I.VarAngle( "Homing Projectile\\Yaw Increase Rate", m_YawIncreaseRate ))
    {
        // Nothing to do
    }
    else if ( I.VarAngle( "Homing Projectile\\Launch Cone Size", m_LaunchConeSize ))
    {
        // Nothing to do
    }
    else if ( I.VarFloat( "Homing Projectile\\Acceleration Percent", m_AccelerationPercent ))
    {
        m_AccelerationPercent = MINMAX( 0.0f, m_AccelerationPercent, 1.0f );
    }
    else if ( I.VarFloat( "Homing Projectile\\Traction", m_Traction ) )
    {
        m_Traction = MAX( 0.0f, m_Traction );
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//=============================================================================

vector3 homing_projectile::GetTargetPoint( object* pTarget )
{
    vector3 TargetPos = vector3(0.0f, 0.0f, 0.0f );

    if( pTarget && pTarget->IsKindOf(actor::GetRTTI()) )
    {
        actor &ActorSource = actor::GetSafeType( *pTarget );

        // generic NPC such as dust mites
        if( pTarget->IsKindOf(genericNPC::GetRTTI()) )
        {
            // no vector realignment for things like dust mites
            TargetPos = ActorSource.GetColBBox().GetCenter();
        }
        else
        {
            // return a point a little above the center

            TargetPos = ActorSource.GetPositionWithOffset( actor::OFFSET_CENTER ) + vector3( 0.0f, 55.0f, 0.0f);
        }
    }
    else
    {
        TargetPos = pTarget->GetPosition();
    }

    return TargetPos;
}

//=============================================================================
