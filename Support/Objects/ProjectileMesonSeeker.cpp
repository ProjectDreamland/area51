//=============================================================================================
// MESON SEEKER PROJECTILE
//=============================================================================================

//=============================================================================================
// INCLUDES
//=============================================================================================
#include "ProjectileMesonSeeker.hpp"
#include "Entropy\e_Draw.hpp"
#include "audiomgr\audiomgr.hpp"
#include "render\LightMgr.hpp"
#include "..\Support\Tracers\TracerMgr.hpp"
#include "Objects\actor\actor.hpp"
#include "Decals\DecalMgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"

#define MAX_COLLISON_PER_BULLET  MAX_COLLISION_MGR_COLLISIONS

//=============================================================================================
// DEFINES
//=============================================================================================
f32 mesonseeker_projectile::s_SeekerGrenade_Alert_Time = 1.2f;

const   xcolor  g_SeekerGrenadeTracer( 49, 49, 250, 200 );
f32 k_MinTimeBetweenBroadcasts = 0.5f;

struct mesonseeker_tweaks
{
    mesonseeker_tweaks( void );

    f32 g_Damage;                   // this is the scalable damage
    f32 g_StartScale;               // projectile size
    f32 g_ExplosionRadius;          // how big is the explosion
    f32 g_LightRadius;              // how big is the radius
    f32 g_LightTime;                // how long is the light effect alive
};

mesonseeker_tweaks::mesonseeker_tweaks( void )
{
    g_StartScale            = 0.2f;
    g_ExplosionRadius       = 100.0f;
    g_LightRadius           = 200.0f;
    g_LightTime             = 2.0f;
}

mesonseeker_tweaks g_MesonSeekerTweaks;

//=============================================================================================
// OBJECT DESC
//=============================================================================================
static struct mesonseeker_projectile_desc : public object_desc
{
    mesonseeker_projectile_desc( void ) : object_desc( 
            object::TYPE_MESONSEEKER_PROJECTILE , 
            "Meson Seeker Projectile", 
            "WEAPON",
            object::ATTR_NEEDS_LOGIC_TIME   |
            object::ATTR_NO_RUNTIME_SAVE    |
            object::ATTR_RENDERABLE,
            FLAGS_IS_DYNAMIC /*| FLAGS_GENERIC_EDITOR_CREATE*/ )
            {}

    virtual object* Create          ( void )
    {
        return new mesonseeker_projectile;
    }

} s_MesonSeeker_Projectile_Desc;

//=============================================================================================

const object_desc&  mesonseeker_projectile::GetTypeDesc     ( void ) const
{
    return s_MesonSeeker_Projectile_Desc;
}


//=============================================================================================
const object_desc&  mesonseeker_projectile::GetObjectType   ( void )
{
    return s_MesonSeeker_Projectile_Desc;
}

//=============================================================================================

mesonseeker_projectile::mesonseeker_projectile()
{
    m_Spin                  = radian3(x_frand(-R_45,R_45), 0.0f, x_frand(-R_30,R_30));
    m_TotalSpin             = radian3(1.0f, 1.0f, 1.0f);

	m_MaxAliveTime          = 10.0f;
    m_ParticleExplosion     = particle_emitter::MSN_SECONDARY_EXPLOSION;
    m_ExplosionRadius       = g_MesonSeekerTweaks.g_ExplosionRadius;
    m_YawRate               = R_60;
    m_SeekerTarget          = 0;
    m_bBroadcastAlert       = TRUE;
    m_TimeSinceLastBroadcast= 0.0f;
    m_bIsLargeProjectile    = TRUE;
    m_EffectTrail           = NULL_GUID;

    m_bThroughGlass         = FALSE;

#ifndef X_EDITOR
    LoadEffect( PRELOAD_FILE("UBER_MESON_GRENADE.FXO"), 
                vector3( 0.0f, 0.0f, 0.0f ), 
                radian3(  R_0,  R_0,  R_0 ) );
#endif
}

//=============================================================================================

mesonseeker_projectile::~mesonseeker_projectile()
{
    DestroyParticles();
}

//=============================================================================

void mesonseeker_projectile::Setup( guid             OriginGuid,
                                   s32               OriginNetSlot,
                                   const vector3&    Position,
                                   const radian3&    Orientation,
                                   const vector3&    Velocity,
                                   s32               Zone1,
                                   s32               Zone2,
                                   pain_handle       PainHandle,
                                   firing_stage      FiringStage)
{
    SetOrigin    ( OriginGuid, OriginNetSlot );
    SetStart     ( Position, Orientation, Velocity, Zone1, Zone2, 0.0f );
    SetPainHandle( PainHandle );

#ifdef X_EDITOR
    LoadEffect( PRELOAD_FILE("UBER_MESON_GRENADE.FXO"), 
        vector3( 0.0f, 0.0f, 0.0f ), 
        radian3(  R_0,  R_0,  R_0 ) );
#endif

    switch( FiringStage )
    {
    case FS_ONE:
        m_Scalar = 1.0f;
        break;
    
    case FS_TWO:
        m_Scalar = 1.5f;
        break;

    case FS_THREE:
        m_Scalar = 2.0f;
        break;

    default:
        ASSERTS(0, "Invalid Firing Stage");
        m_Scalar = 1.0f;
        break;
    }

    m_ExplosionRadius = g_MesonSeekerTweaks.g_ExplosionRadius;

    // Set particle scale
    m_Scale = f32(g_MesonSeekerTweaks.g_StartScale * m_Scalar);
}

//==============================================================================

void mesonseeker_projectile::SetStart( const vector3& Position, 
                                       const radian3& Orientation,
                                       const vector3& Velocity,
                                       s32            Zone1, 
                                       s32            Zone2,
                                       f32            Gravity )
{
    net_proj::SetStart( Position, Orientation, Velocity, Zone1, Zone2, Gravity );

    if( m_EffectTrail )
    {
        object* pObject = g_ObjMgr.GetObjectByGuid( m_EffectTrail );
        ASSERT( pObject );
        pObject->OnMove( Position );
    }
}

//=============================================================================

bbox mesonseeker_projectile::GetLocalBBox( void ) const 
{ 
    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();

    if( pRigidGeom )
    {
        return( pRigidGeom->m_Collision.BBox );
    }
    
    return( bbox( vector3( 10.0f, 10.0f, 10.0f),
                  vector3(-10.0f,-10.0f,-10.0f) ) );
}

//=============================================================================

/*
void mesonseeker_projectile::UpdatePhysics( const f32& DeltaTime )
{
    f32 fVelocity = m_Velocity.Length();
    if (fVelocity == 0.0f)
    {
        return;
    }

    // don't process collisions unless we've taken off
    if( m_bWasReleased )
    {
        OnProcessCollision( DeltaTime );
    }

    //update the spin
    f32 fTimeSpeed = DeltaTime * 30.0f;
    
    m_TotalSpin += radian3(m_Spin.Pitch * fTimeSpeed, m_Spin.Yaw * fTimeSpeed, m_Spin.Roll * fTimeSpeed);

    m_RenderL2W = GetL2W();
    m_RenderL2W.SetRotation(m_TotalSpin);

//    x_DebugMsg(xfs("GRENADE %g,%g,%g\n",m_RenderL2W.GetTranslation().X,m_RenderL2W.GetTranslation().Y,m_RenderL2W.GetTranslation().Z));
//    x_DebugMsg(xfs("OBJECT  %g,%g,%g\n",GetL2W().GetTranslation().X,GetL2W().GetTranslation().Y,GetL2W().GetTranslation().Z));
}
*/

//=============================================================================

void mesonseeker_projectile::OnAdvanceLogic( f32 DeltaTime )
{
    if( m_Exploded )
    {
        net_proj::OnAdvanceLogic( DeltaTime );
        return;
    }

    // Update time since last broadcast
    m_TimeSinceLastBroadcast += DeltaTime;

//    UpdateVelocity( DeltaTime );

    // every second give off a warning to those around me...
    if( m_TimeSinceLastBroadcast > k_MinTimeBetweenBroadcasts )
    {
        alert_package NotifyPackage ;
        NotifyPackage.m_Origin              = GetGuid() ;
        NotifyPackage.m_Position            = GetPosition();
        NotifyPackage.m_Type                = alert_package::ALERT_TYPE_GRENADE ;
        NotifyPackage.m_Target              = alert_package::ALERT_TARGET_NPC ;
        NotifyPackage.m_AlertRadius         = m_Velocity.Length() * 0.5f;
        NotifyPackage.m_Cause               = m_OriginGuid ;
        NotifyPackage.m_FactionsSpecific    = FACTION_NOT_SET;
        NotifyPackage.m_ZoneID              = GetZone1();
        g_AudioManager.AppendAlertReceiver(NotifyPackage);

//        SMP_UTIL_Broadcast_Alert( NotifyPackage ) ;

        m_bBroadcastAlert           = FALSE;
        m_TimeSinceLastBroadcast    = 0.0f;
    }

    if( m_Age > m_MaxAliveTime )
    {
        //blow this thing up...
#ifndef X_EDITOR
        if( m_OwningClient == g_NetworkMgr.GetClientIndex() )
#endif
        {
            OnExplode();
        }
        return;
    }

    if( m_bBroadcastAlert && (m_Age > (m_MaxAliveTime - s_SeekerGrenade_Alert_Time )) )
    {
        //give 1 second warning
        alert_package NotifyPackage ;
        NotifyPackage.m_Origin              = GetGuid() ;
        NotifyPackage.m_Position            = GetPosition();
        NotifyPackage.m_Type                = alert_package::ALERT_TYPE_GRENADE ;
        NotifyPackage.m_Target              = alert_package::ALERT_TARGET_NPC ;
        NotifyPackage.m_AlertRadius         = m_ExplosionRadius * 1.1f ;
        NotifyPackage.m_Cause               = m_OriginGuid ;
        NotifyPackage.m_FactionsSpecific    = FACTION_NOT_SET;
        NotifyPackage.m_ZoneID              = GetZone1();
        g_AudioManager.AppendAlertReceiver(NotifyPackage);

//        SMP_UTIL_Broadcast_Alert( NotifyPackage ) ;

        m_bBroadcastAlert = FALSE;
    }

    // This is at the bottom because the projectile can be destroyed in the call
    net_proj::OnAdvanceLogic( DeltaTime );
}

//==============================================================================

void mesonseeker_projectile::OnImpact( collision_mgr::collision& Coll, object* pTarget )
{
    net_proj::OnImpact( Coll, pTarget );

    // Move to new position and explode
    OnMove( m_NewPos );
#ifndef X_EDITOR
    if( m_OwningClient == g_NetworkMgr.GetClientIndex() )
#endif
    {
        OnExplode();
    }
}

//=============================================================================

void mesonseeker_projectile::OnExplode( void )
{
    if( m_Exploded )
        return;

    net_proj::OnExplode();

    // Destroy the projectiles effect particles
    DestroyParticles();

    //add a light where the explosion occurred
    g_LightMgr.AddFadingLight( GetPosition(), xcolor(55, 55, 135, 255), 
                                g_MesonSeekerTweaks.g_LightRadius, 
                                1.0f, 
                                g_MesonSeekerTweaks.g_LightTime);
    
    //render particle explosion
    guid explodeGuid = particle_emitter::CreatePresetParticleAndOrient( 
            m_ParticleExplosion, 
            vector3(0.0f,0.0f,0.0f), 
            GetPosition() );

    // get tweak
    m_ExplosionRadius = g_MesonSeekerTweaks.g_ExplosionRadius;

    object *particleObject = g_ObjMgr.GetObjectByGuid( explodeGuid );
    if( particleObject && particleObject->IsKindOf(particle_emitter::GetRTTI()) )
    {
        particle_emitter &pEmitter = particle_emitter::GetSafeType( *particleObject );
        pEmitter.SetScale( m_Scale );
    }

    voice_id VoiceID = g_AudioMgr.Play( "MSN_Alt_Projectile_Explode", GetPosition(), GetZone1(), TRUE );
    g_AudioManager.NewAudioAlert( VoiceID, audio_manager::EXPLOSION, GetPosition(), GetZone1(), GetGuid() );

    // create a decal
    decal_package* pPackage = m_hDecalPackage.GetPointer();
    if ( pPackage )
    {
        decal_definition* pDef = pPackage->GetDecalDef( "CharMarks", 0 );
        if( pDef )
        {
	        //only paste a decal if the object is at the last collision
            // point (within radius + 20 centimeters)
            f32 Size        = x_frand( 150.0f, 200.0f );
            f32 fDistToTest = GetBBox().GetRadiusSquared() + (20.0f * 20.0f);
            if ( (m_ImpactPoint - GetBBox().GetCenter()).LengthSquared() < fDistToTest )
            {
                g_DecalMgr.CreateDecalAtPoint( *pDef,
                                               m_ImpactPoint,
                                               m_ImpactNormal,
                                               vector2( Size, Size ),
                                               pDef->RandomRoll() );
            }
        }
    }

    pain Pain;
    Pain.Setup( m_PainHandle, m_OriginGuid, GetPosition() );
    Pain.SetDirection( m_Velocity );
    Pain.SetCustomScalar(m_Scalar);  // scale damage up based on scalar
    Pain.ApplyToWorld();

    alert_package NotifyPackage ;
    NotifyPackage.m_Origin = GetGuid() ;
    NotifyPackage.m_Position = GetPosition() ;
    NotifyPackage.m_Type = alert_package::ALERT_TYPE_EXPLOSION ;
    NotifyPackage.m_Target = alert_package::ALERT_TARGET_NPC ;
    NotifyPackage.m_AlertRadius = m_ExplosionRadius * 2.f ;
    NotifyPackage.m_Cause = NULL_GUID ;
    NotifyPackage.m_FactionsSpecific = FACTION_NOT_SET;
    NotifyPackage.m_ZoneID = GetZone1();
    g_AudioManager.AppendAlertReceiver(NotifyPackage);
//    SMP_UTIL_Broadcast_Alert( NotifyPackage ) ;
}

//=========================================================================

void mesonseeker_projectile::OnMove( const vector3& NewPosition )
{
    net_proj::OnMove( NewPosition );
    UpdateParticles( NewPosition );
}

//=========================================================================

void mesonseeker_projectile::OnRender( void )
{
    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();
    
    if( pRigidGeom )
    {
        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;
        
        m_RigidInst.Render( &GetL2W(), Flags );
    }
    else
    {
//        draw_BBox( GetBBox(), XCOLOR_RED );
    }
}

//=========================================================================

/*
void mesonseeker_projectile::UpdateVelocity( f32 DeltaTime )
{
    // get the vector to target.
    object *targetObject = g_ObjMgr.GetObjectByGuid( m_SeekerTarget );
    if( targetObject && m_bHoming )
    {
        vector3 targetPosition = targetObject->GetPosition();
        if( targetObject && targetObject->IsKindOf(actor::GetRTTI()) )
        {
            actor &actorSource = actor::GetSafeType( *targetObject );
            targetPosition = actorSource.GetPositionWithOffset( actor::OFFSET_CENTER );
        }

        vector3 toTarget = targetPosition - GetPosition();
        toTarget.Normalize();
        vector3 ourVelocity = m_Velocity;
        ourVelocity.Normalize();       

        vector3 rotAngle = toTarget - ourVelocity;
        f32 rotAmount = m_YawRate * DeltaTime;
        if( rotAngle.LengthSquared() < rotAmount * rotAmount )
        {
            m_Velocity = toTarget;
            m_Velocity.NormalizeAndScale(m_Speed);
        }
        else
        {
            rotAngle.NormalizeAndScale(rotAmount);
            ourVelocity+=rotAngle;
            m_Velocity = ourVelocity;
            m_Velocity.NormalizeAndScale(m_Speed);
        }
    }
}
*/

//=============================================================================================

xbool mesonseeker_projectile::LoadInstance( const char* pFileName )
{
	// Initialize the skin
	//m_RigidInst.OnProperty( g_PropQuery.WQueryExternal( "RenderInst\\File", pFileName ) );
    m_RigidInst.SetUpRigidGeom( pFileName );
	return TRUE;

}

//=============================================================================

xbool mesonseeker_projectile::LoadEffect( const char* pFileName, const vector3& InitPos, const radian3& InitRot )
{
    (void)InitRot;

    m_ProjectileFx.SetName( pFileName );

    // homing projectile
    if( 1 )
    {
        // give it a trail
        ASSERT( m_EffectTrail == NULL_GUID );
        m_EffectTrail =  particle_emitter::CreatePresetParticleAndOrient( particle_emitter::MSN_SECONDARY_TRAIL, vector3(0.0f,0.0f,0.0f), InitPos );

        // set up correct explosion
        m_ParticleExplosion     = particle_emitter::MSN_SECONDARY_EXPLOSION;
    }
    else // tap fire, use different particle
    {
        // give it a trail
        ASSERT( m_EffectTrail == NULL_GUID );
        m_EffectTrail =  particle_emitter::CreatePresetParticleAndOrient( particle_emitter::GRAV_GRENADE_TRAIL, vector3(0.0f,0.0f,0.0f), InitPos );

        // different explosion too
        m_ParticleExplosion = particle_emitter::GRAV_GRENADE_EXPLOSION;
    }

    return TRUE;
}

//=============================================================================

void mesonseeker_projectile::DestroyParticles( void )
{
    if( m_EffectTrail )
    {
        if (g_ObjMgr.GetObjectByGuid( m_EffectTrail ) != NULL)
        {
            g_ObjMgr.DestroyObject( m_EffectTrail );
            m_EffectTrail = NULL_GUID;
        }
    }
}

//=============================================================================================

void mesonseeker_projectile::UpdateParticles( const vector3& Position )
{
    // trail
    if( m_EffectTrail )
    {
        object* pEffectObject  = g_ObjMgr.GetObjectByGuid( m_EffectTrail );
        
        if( pEffectObject != NULL )
        {
            matrix4 L2W;
            L2W.Identity();
            L2W.SetTranslation( Position );

            const vector3 Backwards( -m_Velocity );
            radian3 ParticleRotation( Backwards.GetPitch(), Backwards.GetYaw(), R_0 );

            //L2W.SetRotation(  radian3( m_Velocity.GetPitch(), m_Velocity.GetYaw(), R_0 )  );
            L2W.SetRotation( ParticleRotation );

            pEffectObject->OnTransform( L2W );
            pEffectObject->OnMove( Position );
        }
        else
        {
            m_EffectTrail = NULL_GUID;
        }
    }
}

//=============================================================================================
#ifndef X_EDITOR

void mesonseeker_projectile::net_Deactivate( void )
{
//    if( (GameMgr.GameInProgress()) && 
//        (m_OwningClient != g_NetworkMgr.GetClientIndex()) )
//    {
//        DESTROY_NET_OBJECT( this );
//    }
}

#endif

//=============================================================================================
