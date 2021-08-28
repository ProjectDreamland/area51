//=============================================================================================
// GRAVITON CHARGE PROJECTILE
//=============================================================================================

//=============================================================================================
// INCLUDES
//=============================================================================================
#include "GravChargeProjectile.hpp"
#include "Entropy\e_Draw.hpp"
#include "audiomgr\audiomgr.hpp"
#include "render\LightMgr.hpp"
#include "..\Support\Tracers\TracerMgr.hpp"
#include "Decals\DecalMgr.hpp"
#include "NetworkMgr/NetworkMgr.hpp"
#include "NetworkMgr/GameMgr.hpp"
#include "..\Debris\Debris_Mgr.hpp"


//=============================================================================================
// DEFINES
//=============================================================================================
#define GRENADE_ELASTICITY          0.4f
#define MAX_BOUNCES_PER_ADVANCE     2

f32 grav_charge_projectile::s_GravGrenade_Alert_Time = 1.2f;

const xcolor  g_GravGrenadeTracer( 49, 49, 250, 200 );
const f32     g_FadeTime = 0.4f;

//=============================================================================================
// OBJECT DESC
//=============================================================================================
static struct grav_charge_projectile_desc : public object_desc
{
    grav_charge_projectile_desc( void ) : object_desc( 
            object::TYPE_GRAV_CHARGE_PROJECTILE, 
            "Graviton Charge Projectile", 
            "WEAPON",
            object::ATTR_NEEDS_LOGIC_TIME   |
            object::ATTR_NO_RUNTIME_SAVE    |
            object::ATTR_RENDERABLE,
            FLAGS_IS_DYNAMIC)
            {}

    virtual object* Create          ( void )
    {
        return new grav_charge_projectile;
    }

} s_GravCharge_Projectile_Desc;

//=============================================================================================

const object_desc&  grav_charge_projectile::GetTypeDesc     ( void ) const
{
    return s_GravCharge_Projectile_Desc;
}


//=============================================================================================
const object_desc&  grav_charge_projectile::GetObjectType   ( void )
{
    return s_GravCharge_Projectile_Desc;
}

//=============================================================================================

grav_charge_projectile::grav_charge_projectile()
{
    m_AliveTime             = 0.0f;
    m_CollisionPoint        = vector3( 0.f , 0.f , 0.f );  
    m_NormalCollision       = vector3( 0.f , 0.f , 0.f );  
    m_Spin                  = radian3(x_frand(-R_45,R_45), 0.0f, x_frand(-R_30,R_30));
    m_TotalSpin             = radian3(0.0f, 0.0f, 0.0f);

    m_MaxAliveTime          = 5.0f;
    m_ParticleExplosion     = particle_emitter::GRAV_GRENADE_EXPLOSION;
    m_ExplosionRadius       = 800.0f;

    m_bBroadcastAlert       = TRUE;
    m_bDestroyed            = FALSE;
    m_bIsLargeProjectile    = TRUE;

    m_Gravity               = 0.0f;

    m_FlyVoiceID            = -1;

#ifndef X_EDITOR
    LoadEffect( PRELOAD_FILE("UBER_MESON_GRENADE.FXO"), 
                vector3( 0.0f, 0.0f, 0.0f ), 
                radian3(  R_0,  R_0,  R_0 ) );
#endif
}

//=============================================================================================

grav_charge_projectile::~grav_charge_projectile()
{
    DestroyParticles();

    if (m_FlyVoiceID != -1)
    {
        g_AudioMgr.Release( m_FlyVoiceID, 0 );
    }
}

//=============================================================================================
/*
void grav_charge_projectile::Initialize( const vector3&        InitPos,
                                  const radian3&        InitRot,
                                  const vector3&        InheritedVelocity,
                                        f32             Speed,
                                        guid            OwnerGuid,
                                        pain_handle     PainHandle,
                                        xbool           bHitLiving)
{
    // Call base class
    net_proj::Initialize(InitPos, InitRot, InheritedVelocity, Speed, OwnerGuid , PainHandle, bHitLiving ) ;

    // flip it around
    const vector3 Backwards( -m_Velocity );
    radian3 ParticleRotation( Backwards.GetPitch(), Backwards.GetYaw(), 0.0f );
    
    // Create projectile particle effect trail
    m_EffectTrail =  particle_emitter::CreatePresetParticleAndOrient(
        particle_emitter::GRAV_GRENADE_TRAIL, 
        ParticleRotation, 
        InitPos);

}
*/

//=============================================================================================

void grav_charge_projectile::Setup(guid           OriginGuid,
                                   s32            OriginNetSlot,
                                   const vector3& Position,
                                   const radian3& Orientation,
                                   const vector3& Velocity,
                                   s32            Zone1,
                                   s32            Zone2,
                                   f32            Gravity,
                                   pain_handle    PainHandle )
{
    SetOrigin    ( OriginGuid, OriginNetSlot );
    SetStart     ( Position, Orientation, Velocity, Zone1, Zone2, Gravity );
    SetPainHandle( PainHandle );
    SetSticky    ( FALSE );    

#ifdef X_EDITOR
    LoadEffect( PRELOAD_FILE("UBER_MESON_GRENADE.FXO"), 
        vector3( 0.0f, 0.0f, 0.0f ), 
        radian3(  R_0,  R_0,  R_0 ) );
#endif
}

//=============================================================================

/*
void grav_charge_projectile::Initialize( const vector3& InitPos , const radian3& InitRot , const vector3& InitVel , const guid& OwnerGuid )
{
    //initialize the base class
    base_projectile::Initialize( InitPos , InitRot , InitVel , OwnerGuid );
}

//=============================================================================================

void grav_charge_projectile::Initialize( const vector3& InitPos , const matrix4& InitMat , const vector3& InitVel , const guid& OwnerGuid )
{
    //initialize the base class
    base_projectile::Initialize( InitPos , InitMat , InitVel , OwnerGuid );
}

//=============================================================================

void grav_charge_projectile::Initialize( const guid&    OwnerGuid,
                                         const vector3& InitPos, 
                                         const vector3& InitVelocity )
{
    base_projectile::Initialize( OwnerGuid, InitPos, InitVelocity );

    // flip it around
    const vector3 Backwards( -m_Velocity );
    radian3 ParticleRotation( Backwards.GetPitch(), Backwards.GetYaw(), 0.0f );

    // Create projectile particle effect trail
    m_EffectTrail =  particle_emitter::CreatePresetParticleAndOrient(
        particle_emitter::GRAV_GRENADE_TRAIL, 
        ParticleRotation, 
        InitPos);
}
*/

//==============================================================================

void grav_charge_projectile::SetStart( const vector3& Position, 
                                      const radian3& Orientation,
                                      const vector3& Velocity,
                                      s32            Zone1, 
                                      s32            Zone2,
                                      f32            Gravity )
{
    net_proj::SetStart( Position, Orientation, Velocity, Zone1, Zone2, Gravity );

    m_FlyVoiceID = g_AudioMgr.PlayVolumeClipped( "MSN_IdleSpecial_Loop", Position, Zone1, TRUE );
    if (m_FlyVoiceID != -1)
    {
        // Setting this unusually high because it is supposed to clamp 
        // internally to 1.0f after all calculations are done.
        g_AudioMgr.SetVolume( m_FlyVoiceID, 10.0f );
        g_AudioMgr.SetFalloff( m_FlyVoiceID, 1, 1 );
    }

    if( m_EffectTrail )
    {
        object* pObject = g_ObjMgr.GetObjectByGuid( m_EffectTrail );
        ASSERT( pObject );
        pObject->OnMove( Position );
    }
}
//=============================================================================

bbox grav_charge_projectile::GetLocalBBox( void ) const 
{ 
    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();

    if( pRigidGeom )
    {
        return( pRigidGeom->m_Collision.BBox );
    }
    
    return( bbox( vector3( 10.0f, 10.0f, 10.0f),
                  vector3(-10.0f,-10.0f,-10.0f) ) );
}

//=============================================================================================

void grav_charge_projectile::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "grav_charge_projectile::OnAdvanceLogic" );

    if( m_Exploded )
    {
        net_proj::OnAdvanceLogic( DeltaTime );
        return;
    }

    // Explode if lifetime has expired
    if( m_Age > m_MaxAliveTime )
    {
#ifndef X_EDITOR
        if( m_OwningClient == g_NetworkMgr.GetClientIndex() )
#endif
        {
            OnExplode();
        }
        return;
    }

    // This is at the bottom because the projectile can be destroyed in the call
    net_proj::OnAdvanceLogic( DeltaTime );
}

//==============================================================================

void grav_charge_projectile::OnImpact( collision_mgr::collision& Coll, object* pTarget )
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
    m_Impact = TRUE;
    m_AtRest = TRUE;
}

//=============================================================================

void grav_charge_projectile::OnExplode( void )
{
    if( m_Exploded )
        return;

    net_proj::OnExplode();

    //add a light where the explosion occurred
    g_LightMgr.AddFadingLight( GetPosition(), xcolor(55, 55, 135, 255), 300.0f, 1.0f, 1.8f );

    // Destroy the projectiles effect particles
    DestroyParticles();

    //render particle explosion
    guid ExplosionGuid = particle_emitter::CreatePresetParticleAndOrient( 
            m_ParticleExplosion, 
            vector3(0.0f,0.0f,0.0f), 
            GetPosition() );

    if( ExplosionGuid )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( ExplosionGuid );
        if( pObj )
        {
            // KSS -- FIXME -- we are only scaling this because it is not the correct particle
            ((particle_emitter*)pObj)->SetScale( 0.2f );
        }
    }

    //g_AudioManager.Play( "Grenade_Explosion", GetPosition() );

    voice_id VoiceID = g_AudioMgr.Play( "GravCharge_Explosion", GetPosition(), GetZone1(), TRUE );
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
            if ( (m_CollisionPoint - GetBBox().GetCenter()).LengthSquared() < fDistToTest )
            {
                g_DecalMgr.CreateDecalAtPoint( *pDef,
                                               m_CollisionPoint,
                                               m_NormalCollision,
                                               vector2( Size, Size ),
                                               pDef->RandomRoll() );
            }
        }
    }
/*
    pain Pain;
    Pain.Setup( m_PainHandle, m_OriginGuid, GetPosition() );
    Pain.SetDirection( m_Velocity );
    Pain.ApplyToWorld();
*/
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
    
#ifndef X_EDITOR
    // Only the originating machine should create the explosion.
    // The other clients will get the explosion via the normal 
    // instantiation process...
    if( m_OwningClient == g_NetworkMgr.GetClientIndex() )
    {
        debris_mgr::GetDebrisMgr()->CreateSpecializedDebris( GetPosition(), m_ImpactNormal, object::TYPE_DEBRIS_MESON_EXPLOSION, GetZones(), m_OriginGuid );
    }
#else
    debris_mgr::GetDebrisMgr()->CreateSpecializedDebris( GetPosition(), m_ImpactNormal, object::TYPE_DEBRIS_MESON_EXPLOSION, GetZones(), m_OriginGuid );
#endif


    //destroy the grenade
    m_bDestroyed = TRUE;
}

//=============================================================================

void grav_charge_projectile::OnMove( const vector3& NewPosition )
{
    net_proj::OnMove( NewPosition );
    UpdateParticles( NewPosition );
}

//=========================================================================

void grav_charge_projectile::OnRender( void )
{
    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();
    
    if( pRigidGeom )
    {
        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;
        
        m_RigidInst.Render( &GetL2W(), Flags );
    }
    else
    {
        //draw_BBox( GetBBox(), XCOLOR_RED );
    }

    net_proj::OnRender();

#ifdef X_EDITOR
//    draw_Line( GetPosition() , GetPosition() + m_NormalCollision );
//    draw_Line( GetPosition() , GetPosition() + m_Velocity , XCOLOR_BLUE );
#endif // X_EDITOR
}

//=============================================================================================

xbool grav_charge_projectile::LoadInstance( const char* pFileName )
{
    // Initialize the skin
    //m_RigidInst.OnProperty( g_PropQuery.WQueryExternal( "RenderInst\\File", pFileName ) );
    m_RigidInst.SetUpRigidGeom( pFileName );
    return TRUE;

}

xbool grav_charge_projectile::LoadEffect( const char* pFileName, const vector3& InitPos, const radian3& InitRot )
{
    (void)InitRot;

    m_ProjectileFx.SetName( pFileName );

    // homing projectile
    if( 0 )
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

void grav_charge_projectile::DestroyParticles( void )
{
    if( m_EffectTrail )
    {
        particle_emitter* pEmitter = (particle_emitter*)g_ObjMgr.GetObjectByGuid( m_EffectTrail );
        if( pEmitter != NULL )
        {
            ASSERT( pEmitter->IsKindOf( particle_emitter::GetRTTI() ) );
            pEmitter->DestroyParticle();
            m_EffectTrail = NULL_GUID;
        }
    }
}

//=============================================================================================

void grav_charge_projectile::UpdateParticles( const vector3& Position )
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

    if (m_FlyVoiceID != -1)
    {
        g_AudioMgr.SetPosition( m_FlyVoiceID, Position, GetZone1() );
    }
}

//=============================================================================================
#ifndef X_EDITOR

void grav_charge_projectile::net_Deactivate( void )
{
//    if( (GameMgr.GameInProgress()) && 
//        (m_OwningClient != g_NetworkMgr.GetClientIndex()) )
//    {
//        DESTROY_NET_OBJECT( this );
//    }
}

#endif

//=============================================================================================

