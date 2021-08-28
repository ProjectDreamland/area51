//=========================================================================
// ENERGY PROJECTILE
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "ProjectileEnergy.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "ClothObject.hpp"
#include "DestructibleObj.hpp"
#include "SuperDestructible.hpp"
#include "Turret.hpp"
#include "Actor\Actor.hpp"
#include "gamelib\StatsMgr.hpp"
#include "Decals\DecalMgr.hpp"
#include "Characters\Character.hpp"
#include "render\LightMgr.hpp"
#include "..\Support\Tracers\TracerMgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "Objects\player.hpp"

#ifndef X_EDITOR
#include "Objects\netghost.hpp"
#endif

//=============================================================================================
// TWEAKABLES
//=============================================================================================
f32 RANDOM_BOUNCE_FACTOR_X          = 0.075f;
f32 RANDOM_BOUNCE_FACTOR_Y          = 0.075f;
f32 RANDOM_BOUNCE_FACTOR_Z          = 0.075f;
u16 MAX_BOUNCES_PER_ADVANCE         = 2;
//f32 PROJECTILE_ELASTICITY           = 0.9f;
//f32 PROJ_STICK_ANGLE                = 30.0f;  // 30 degrees
f32 EXPLODE_TIME                    = 0.667f;   // 2/3 seconds
f32 ENERGY_PROJECTILE_DAMAGE        =  50.0f;
f32 ENERGY_FORCE_AMOUNT             =  50.0f;

f32 BBG_BounceFactorRun             = 1.0f;
f32 BBG_BounceFactorRise            = -1.0f;

//=============================================================================================
// DEFINES
//=============================================================================================
#define MAX_COLLISON_PER_PROJECTILE     5


//=========================================================================
// OBJECT DESC.
//=========================================================================
static struct energy_projectile_desc : public object_desc
{
    energy_projectile_desc( void ) : object_desc( 
            object::TYPE_ENERGY_PROJECTILE, 
            "Energy Projectile", 
            "WEAPON",
            object::ATTR_NEEDS_LOGIC_TIME |
            object::ATTR_NO_RUNTIME_SAVE,            
            FLAGS_IS_DYNAMIC /*| FLAGS_GENERIC_EDITOR_CREATE*/ )
            {}

    virtual object* Create          ( void )
    {
        return new energy_projectile;
    }

} s_Energy_Projectile_Desc;

//=============================================================================================

const object_desc&  energy_projectile::GetTypeDesc     ( void ) const
{
    return s_Energy_Projectile_Desc;
}

//=============================================================================================

const object_desc&  energy_projectile::GetObjectType   ( void )
{
    return s_Energy_Projectile_Desc;
}

//=============================================================================================

energy_projectile::energy_projectile()
{
	m_MaxAliveTime      = 3.0f;
    m_FxGuid            = NULL_GUID;
    m_ExplodeTimer      = EXPLODE_TIME;
    m_ExplosionRadius   = 100.0f;
    m_Gravity           = 0.0f;
    m_AtRest            = FALSE;

    m_BounceFactorRise  = BBG_BounceFactorRise;
    m_BounceFactorRun   = BBG_BounceFactorRun;

    m_bThroughGlass     = FALSE;

    m_ParticleExplosion = particle_emitter::MSN_PROJ_EXPLOSION;
//    m_CollisionData.Clear();

    SetSticky    ( TRUE );

    // Setup the rigidgeom & decal packages
    LoadDecalPackage( PRELOAD_FILE( "BulletHoles.decalpkg" ) );
    LoadInstance    ( PRELOAD_FILE( "WPN_GRAV_Bindpose.rigidgeom" ) );

#ifndef X_EDITOR
    LoadEffect( PRELOAD_FILE("MSN_PROJECTILE.FXO"), 
                vector3( 0.0f, 0.0f, 0.0f ), 
                radian3(  R_0,  R_0,  R_0 ) );
#endif
}

//=============================================================================================

energy_projectile::~energy_projectile()
{
    DestroyParticles();
}

//=============================================================================================

void energy_projectile::Setup     ( guid           OriginGuid,
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

#ifdef X_EDITOR
    LoadEffect( PRELOAD_FILE("MSN_PROJECTILE.FXO"), 
        vector3( 0.0f, 0.0f, 0.0f ), 
        radian3(  R_0,  R_0,  R_0 ) );
#endif
}

//==============================================================================

void energy_projectile::SetStart( const vector3& Position, 
                                  const radian3& Orientation,
                                  const vector3& Velocity,
                                  s32            Zone1, 
                                  s32            Zone2,
                                  f32            Gravity )
{
    net_proj::SetStart( Position, Orientation, Velocity, Zone1, Zone2, Gravity );

    if( m_FxGuid )
    {
        object* pObject = g_ObjMgr.GetObjectByGuid( m_FxGuid );
        ASSERT( pObject );
#ifdef X_EDITOR
        pObject->OnMove( Position );
#else
        pObject->OnMove( Position + net_GetBlendOffset() );
#endif
    }
}

//=============================================================================================

void energy_projectile::DestroyParticles( void )
{
    // Destroy the particle effect.
    if( m_FxGuid )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_FxGuid );
        if( pObj )
        {
            particle_emitter& Particle = particle_emitter::GetSafeType( *pObj );
            Particle.DestroyParticle();
        }
        m_FxGuid = 0;
    }
}

//=============================================================================================

void energy_projectile::UpdateParticles( const vector3& NewPosition )
{
    // Update the particle on the final endpoint
    if( m_FxGuid != NULL_GUID )
    {        
        object* pObj = g_ObjMgr.GetObjectByGuid( m_FxGuid );

        if( pObj )
        {
            switch( m_ImpactCount )
            {
            case 1:
                {
                    const xcolor Color = xcolor(241, 75, 75, 255);
                    ((particle_emitter*)pObj)->SetColor( Color );
                }
                break;
            case 2:
                {
                    const xcolor Color = xcolor(75, 75, 255, 255);
                    ((particle_emitter*)pObj)->SetColor( Color );
                }
                break;
            case 3:
                {
                    const xcolor Color = xcolor(255, 255, 255, 255);
                    ((particle_emitter*)pObj)->SetColor( Color );
                }
                break;
            case 0:
            default:
                break;
            }

            pObj->OnMove( NewPosition );
        }
    }
}

//=============================================================================================

void energy_projectile::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "energy_projectile::OnAdvanceLogic" );
    LOG_STAT( k_stats_Projectiles);

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

    // Explode if attached and timer runs down
    if( m_bIsAttached )
    {
        m_ExplodeTimer -= DeltaTime;
        if( m_ExplodeTimer < 0.0f )
        {
#ifndef X_EDITOR
            if( m_OwningClient == g_NetworkMgr.GetClientIndex() )
#endif
            {
                // blow up with splash damage
                OnExplode();
                CausePain();
            }
            return;
        }
    }

    // This is at the bottom because the projectile can be destroyed in the call
    net_proj::OnAdvanceLogic( DeltaTime );
}

//==============================================================================

void energy_projectile::OnImpact( collision_mgr::collision& Coll, object* pTarget )
{
    net_proj::OnImpact( Coll, pTarget );

    xbool bReachedMaxImpacts = (m_ImpactCount > MAX_ENERGY_PROJ_IMPACTS);

    // Explode if number of impacts is exceeded or if we hit a player
#ifndef X_EDITOR
    if( bReachedMaxImpacts || pTarget->IsKindOf( player::GetRTTI() ) )
#else
    if( bReachedMaxImpacts || pTarget->IsKindOf( player::GetRTTI() ) )
#endif
    {
#ifndef X_EDITOR
        if( m_OwningClient == g_NetworkMgr.GetClientIndex() )
#endif
        {
            // Move to new position and explode
            OnMove( m_NewPos );

            // don't cause pain if we're just blowing up because of max impacts
            OnExplode();
            if( !bReachedMaxImpacts )
                CausePain();
        }
        m_Impact = TRUE;
        m_AtRest = TRUE;
        return;
    }

    // do pain to destructibles and continue on
    if( pTarget && !pTarget->IsKindOf(actor::GetRTTI()) )
    {
        DoImpactPain( pTarget->GetGuid(), Coll );
    }

    // Do the effects
    if( !m_bIsAttached )
        DoImpactEffects( Coll );
}

//=============================================================================================

void energy_projectile::OnRender( void )
{
    // Setup Render Matrix
    m_RenderL2W = GetL2W();
#ifndef X_EDITOR
    m_RenderL2W.Translate( net_GetBlendOffset() );
#endif

    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();
    
    if( pRigidGeom )
    {
        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;
        
        m_RigidInst.Render( &m_RenderL2W, Flags );
    }
    else
    {
//        draw_BBox( GetBBox(), XCOLOR_RED );
    }
}

//=============================================================================================

void energy_projectile::OnMove( const vector3& NewPosition )
{
    net_proj::OnMove( NewPosition );

#ifdef X_EDITOR
    UpdateParticles( NewPosition );
#else
    UpdateParticles( NewPosition + net_GetBlendOffset() );
#endif
}

//=============================================================================

bbox energy_projectile::GetLocalBBox( void ) const 
{ 
    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();
    if( pRigidGeom )
    {
        return( pRigidGeom->m_Collision.BBox );
    }
    else
    {
        bbox BBox;
        BBox.Set( vector3( -2, -2, -2 ), vector3( 2, 2, 2) );
        return BBox;
    }
}

//=============================================================================================

xbool energy_projectile::LoadInstance( const char* pFileName )
{
    m_RigidInst.SetUpRigidGeom( pFileName );
	return TRUE;
}

//=============================================================================

xbool energy_projectile::LoadDecalPackage( const char* pFileName )
{
    m_hDecalPackage.SetName( pFileName );
    return TRUE;
}

//=============================================================================

xbool energy_projectile::LoadEffect( const char* pFileName, const vector3& InitPos, const radian3& InitRot )
{
    m_ProjectileFx.SetName( pFileName );
    ASSERT( m_FxGuid == 0 );
    m_FxGuid = particle_emitter::CreateGenericParticle( m_ProjectileFx.GetName(), 0 );

    // Set the particle effect up.
    if( m_FxGuid != NULL_GUID )
    {
        matrix4 Mat;
        Mat.SetRotation( InitRot );
        Mat.SetTranslation( InitPos );
        //Mat.InvertRT();
        //Mat.SetTranslation( GetL2W().GetTranslation() );

        object* pObj = g_ObjMgr.GetObjectByGuid( m_FxGuid );
        if( pObj )
        {
            pObj->OnTransform( Mat );
        }

        // KSS -- FIXME -- we are only scaling this because it is not the correct particle
        ((particle_emitter*)pObj)->SetScale( 0.5f );
    }
  
    return TRUE;
}

//=============================================================================

void energy_projectile::DoImpactEffects( collision_mgr::collision& Coll )
{
    // create a char decal
    decal_package* pPackage = m_hDecalPackage.GetPointer();
    if ( pPackage )
    {
        decal_definition* pDef = pPackage->GetDecalDef( "EnergyCharMark", 0 );
        if ( pDef )
        {            
            g_DecalMgr.CreateDecalAtPoint( *pDef, Coll.Point, Coll.Plane.Normal );            
        }
    }

    voice_id VoiceID = g_AudioMgr.Play( "MSN_Projectile_Ricochet", Coll.Point, GetZone1(), TRUE );
    g_AudioManager.NewAudioAlert( VoiceID, audio_manager::BULLET_IMPACTS, Coll.Point, GetZone1(), GetGuid() );

    // Create the paricle effect.
    particle_emitter::CreateProjectileCollisionEffect( Coll, m_OriginGuid );
   
}

//=============================================================================

void energy_projectile::OnExplode( void )
{
    if( m_Exploded )
        return;

    net_proj::OnExplode();

    // Destroy the projectiles effect particles
    DestroyParticles();

    //add a light where the explosion occurred
    g_LightMgr.AddFadingLight( GetPosition(), xcolor(55, 55, 135, 255), 175.0f, 1.0f, 1.3f );

    //render particle explosion
    guid ExplosionGuid = particle_emitter::CreatePresetParticleAndOrient( 
            m_ParticleExplosion, 
            vector3(0.0f,0.0f,0.0f), 
            GetPosition() );

    object* pObj = g_ObjMgr.GetObjectByGuid( ExplosionGuid );
    if( pObj )
    {
        ((particle_emitter*)pObj)->SetScale( 0.3f );
    }

    voice_id VoiceID = g_AudioMgr.Play( "MSN_Projectile_Explode", GetPosition(), GetZone1(), TRUE );
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
}

//=============================================================================

void energy_projectile::CausePain( guid HitGuid )
{
#ifndef X_EDITOR
    // No pain when it's a ghost projectile
    if( m_OwningClient != g_NetworkMgr.GetClientIndex() )
        return;
#endif

    // notify NPCs even if this is just a bounce
    alert_package NotifyPackage ;
    NotifyPackage.m_Origin = GetGuid() ;
    NotifyPackage.m_Position = GetPosition() ;
    NotifyPackage.m_Type = alert_package::ALERT_TYPE_EXPLOSION ;
    NotifyPackage.m_Target = alert_package::ALERT_TARGET_NPC ;
    NotifyPackage.m_AlertRadius = m_ExplosionRadius * 0.8f ;
    NotifyPackage.m_Cause = NULL_GUID ;
    NotifyPackage.m_FactionsSpecific = FACTION_NOT_SET;
    NotifyPackage.m_ZoneID = GetZone1();
    g_AudioManager.AppendAlertReceiver(NotifyPackage);

    pain Pain;
    Pain.Setup( m_PainHandle, m_OriginGuid, GetPosition() );
    
    if( m_bIsAttached )
    {
        Pain.SetDirectHitGuid(m_AttachedObjectGuid);
    }
    else
    {
        // for destructibles and such
        if( HitGuid != 0 )
        {
            object* pObj = g_ObjMgr.GetObjectByGuid( HitGuid );
            Pain.SetDirectHitGuid(HitGuid);
            Pain.ApplyToObject(pObj);

            // we only it to damage what it bounced off of if this has a HitGuid
            return;
        }
    }

    Pain.SetDirection( m_Velocity );
    Pain.ApplyToWorld();

    //    SMP_UTIL_Broadcast_Alert( NotifyPackage ) ;
}

//=============================================================================================
void energy_projectile::OnAttachToObject( collision_mgr::collision& Coll, object* pTarget )
{
    // this will set m_AttachedObjectGuid if pTarget is valid
    net_proj::OnAttachToObject(Coll, pTarget);

    // this is for things the projectile will attach to... make them have a pain reaction
    DoImpactPain( Coll.ObjectHitGuid, Coll );
}

//=============================================================================================
void energy_projectile::DoImpactPain( guid HitGuid, collision_mgr::collision& Coll )
{
    pain_handle PainHandle( "BBG_TINYPAIN" );
    pain Pain;
    Pain.Setup( PainHandle, m_OriginGuid, Coll.Point );

    object *pTarget = g_ObjMgr.GetObjectByGuid( HitGuid );
    
    if( pTarget )
    {
        Pain.SetDirectHitGuid(HitGuid);
        Pain.SetCollisionInfo( Coll );
        Pain.SetDirection( m_Velocity );
        Pain.ApplyToObject(pTarget);
    }
}

//=============================================================================================
#ifndef X_EDITOR

void energy_projectile::net_Deactivate( void )
{
//    if( (GameMgr.GameInProgress()) && 
//        (m_OwningClient != g_NetworkMgr.GetClientIndex()) )
//    {
//        DESTROY_NET_OBJECT( this );
//    }
}

#endif

//=============================================================================
