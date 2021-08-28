//=============================================================================================
// GRENADE PROJECTILE
//=============================================================================================

//=============================================================================================
// INCLUDES
//=============================================================================================
#include "GrenadeProjectile.hpp"
#include "Entropy\e_Draw.hpp"
#include "audiomgr\audiomgr.hpp"
#include "render\LightMgr.hpp"
#include "Objects\AnimSurface.hpp"
#include "objects\ClothObject.hpp"
#include "objects\DestructibleObj.hpp"
#include "Decals\DecalMgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "Objects/Actor/Actor.hpp"

//=============================================================================================
// DEFINES
//=============================================================================================
#define GRENADE_ELASTICITY          0.4f
#define MAX_BOUNCES_PER_ADVANCE     2
#define MAX_COLLISON_PER_GRENADE    5
const f32 k_MinTimeBeforeCollision = 0.75f;
//=============================================================================================
// 
//=============================================================================================
f32 grenade_projectile::s_Grenade_Alert_Time = 1.0f ;

f32 g_GrenadeBounceSoundSeconds = 0.1f;

tweak_handle FRAG_GRENADE_CookTimeTweak ( "FRAG_GRENADE_CookTime" );

//=============================================================================================
// OBJECT DESC
//=============================================================================================
static struct grenade_projectile_desc : public object_desc
{
    grenade_projectile_desc( void ) : object_desc( 
            object::TYPE_GRENADE_PROJECTILE, 
            "Grenade", 
            "WEAPON",
            object::ATTR_NEEDS_LOGIC_TIME   |
            object::ATTR_RENDERABLE,
            FLAGS_IS_DYNAMIC )
            {}

    virtual object* Create          ( void )
    {
        return new grenade_projectile;
    }

} s_Grenade_Projectile_Desc;

//=============================================================================================

const object_desc&  grenade_projectile::GetTypeDesc     ( void ) const
{
    return s_Grenade_Projectile_Desc;
}


//=============================================================================================
const object_desc&  grenade_projectile::GetObjectType   ( void )
{
    return s_Grenade_Projectile_Desc;
}

//=============================================================================================
// Grenade
//=============================================================================================
f32 g_GrenadeRise = -0.35f;
f32 g_GrenadeRun  =  0.60f;
grenade_projectile::grenade_projectile()
{
    m_Spin                  = radian3(x_frand(-R_45,R_45), 0.0f, x_frand(-R_30,R_30));
    m_TotalSpin             = radian3(0.0f, 0.0f, 0.0f);

    m_MaxAliveTime          = FRAG_GRENADE_CookTimeTweak.GetF32();
    m_ParticleExplosion     = particle_emitter::GRENADE_EXPLOSION;
    m_ExplosionRadius       = 500.0f;
    m_Gravity               = 980.0f;

    m_BounceFactorRise      = g_GrenadeRise;
    m_BounceFactorRun       = g_GrenadeRun;

    m_bIsLargeProjectile    = FALSE;
    m_SinceLastBroadcast    = 0.0f;
    m_bFromNPC              = FALSE;

    // make sure a sound happens the first time
    m_GrenadeSoundTime = 0.0f;

    const char* pGeomName = PRELOAD_FILE( "WPN_FRG_Bindpose.rigidgeom" );
    LoadInstance( pGeomName );
}

//=============================================================================================

grenade_projectile::~grenade_projectile( void )
{
    DestroyTrail();
}

//=============================================================================================

void grenade_projectile::DestroyTrail( void )
{
    if( m_EffectTrail != NULL_GUID ) 
    {
        if( g_ObjMgr.GetObjectByGuid( m_EffectTrail ) != NULL )
            g_ObjMgr.DestroyObject( m_EffectTrail );
    }
    m_EffectTrail = NULL_GUID;
}

//=============================================================================================

void grenade_projectile::Setup     ( guid           OriginGuid,
                                     s32            OriginNetSlot,
                                     const vector3& Position,
                                     const radian3& Orientation,
                                     const vector3& Velocity,
                                     s32            Zone1,
                                     s32            Zone2,
                                     pain_handle    PainHandle )
{
    SetOrigin    ( OriginGuid, OriginNetSlot );
    SetStart     ( Position, Orientation, Velocity, Zone1, Zone2, m_Gravity );
    SetPainHandle( PainHandle );

    /*
    rhandle<char> FxResource;
    FxResource.SetName( PRELOAD_FILE( "GrenadeFrag_Trail_000.fxo" ) );

    // show expert flame trail
    m_EffectTrail = particle_emitter::CreatePresetParticleAndOrient( 
                                    FxResource.GetName(),
                                    vector3(0.0f,0.0f,0.0f),
                                    Position );
                                    */
}

//=============================================================================================

xbool grenade_projectile::GetDoCollisions( void )
{
    if( m_bFromNPC &&
        m_Age < k_MinTimeBeforeCollision )
    {
        return FALSE;
    }
    return TRUE;
}

//=============================================================================================

bbox grenade_projectile::GetLocalBBox( void ) const 
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
radian3 g_GrenadeSpin = radian3(R_180, R_180, R_90);

void grenade_projectile::OnAdvanceLogic( f32 DeltaTime )
{
    net_proj::OnAdvanceLogic( DeltaTime );
    if( m_Exploded )
        return;

    m_SinceLastBroadcast += DeltaTime;
    m_GrenadeSoundTime -= DeltaTime;

    if( m_Age > m_MaxAliveTime )
    {
        // Time to blow!
        #ifndef X_EDITOR
        if( m_OwningClient == g_NetworkMgr.GetClientIndex() )
        #endif
        {
            OnExplode();
        }
        return;
    }
    else
    {
        // if we haven't stopped, roll a little
        if( !m_AtRest )
        {            
            m_TotalSpin = m_TotalSpin + (g_GrenadeSpin * DeltaTime);            
        }

        // every second broadcast...
        if( m_SinceLastBroadcast >= s_Grenade_Alert_Time )
        {
            //give 1 second warning
            alert_package NotifyPackage ;
            NotifyPackage.m_Origin = GetGuid() ;
            NotifyPackage.m_Position = GetPosition();
            NotifyPackage.m_Type = alert_package::ALERT_TYPE_GRENADE ;
            NotifyPackage.m_Target = alert_package::ALERT_TARGET_NPC ;
            NotifyPackage.m_AlertRadius = m_ExplosionRadius;
            NotifyPackage.m_Cause = m_OriginGuid ;
            NotifyPackage.m_FactionsSpecific = FACTION_NOT_SET;
            NotifyPackage.m_ZoneID = GetZone1();
            g_AudioManager.AppendAlertReceiver(NotifyPackage);
            m_SinceLastBroadcast = 0.0f;
        }
    }
}

//=============================================================================================

void grenade_projectile::OnImpact( collision_mgr::collision& Coll, object* pTarget )
{
    if( CheckHitThreatObject(pTarget) )
    {
        // Time to blow!
#ifndef X_EDITOR
        if( m_OwningClient == g_NetworkMgr.GetClientIndex() )
#endif
        {
            OnExplode();
        }
        m_Impact = TRUE;
        m_AtRest = TRUE;
        m_ImpactPoint = Coll.Point;
        return;
    }

    // don't make bounce sound too often
    if( m_GrenadeSoundTime <= F32_MIN )
    {
        g_AudioMgr.PlayVolumeClipped( "GrenadeBounceConcrete", GetPosition(), GetZone1(), TRUE );
        m_GrenadeSoundTime = g_GrenadeBounceSoundSeconds;
    }

    object* pObj  = g_ObjMgr.GetObjectByGuid( m_EffectTrail );

    if( pObj != NULL )
    {
        particle_emitter *pParticle = (particle_emitter*)pObj;
        pParticle->DestroyParticle();
    }

    net_proj::OnImpact(Coll, pTarget);
}

//=============================================================================================

void grenade_projectile::OnMove( const vector3& NewPosition )
{
    net_proj::OnMove(NewPosition);

    // smoke trail
    object* pObj  = g_ObjMgr.GetObjectByGuid( m_EffectTrail );

    if( pObj != NULL )
    {
        matrix4 L2W;
        L2W.Identity();
        L2W.SetTranslation( NewPosition );

        // set proper trail rotation
        radian3 ParticleRotation( m_Velocity.GetPitch(), (m_Velocity.GetYaw()-R_90), R_0 );

        L2W.SetRotation( ParticleRotation );

        pObj->OnTransform( L2W );
        pObj->OnMove( NewPosition );
    }
    else
    {
        m_EffectTrail = NULL_GUID;
    }
}

//=============================================================================================

void grenade_projectile::OnExplode( void )
{
//  LOG_MESSAGE( "grenade_projectile::OnExplode", "Enter" );

    if( m_Exploded )
        return;

    // get rid of smokey trail
    DestroyTrail();

    net_proj::OnExplode();

    //add a light where the explosion occurred
    g_LightMgr.AddFadingLight( GetPosition(), xcolor(152, 152, 76, 255), 300.0f, 1.0f, 1.8f );

    /*
    //render particle explosion
    particle_emitter::CreatePresetParticleAndOrient( 
            m_ParticleExplosion, 
            vector3(0.0f,0.0f,0.0f), 
            GetPosition() );
    */
    //g_AudioMgr.Play( "Grenade_Explosion", GetPosition() );

    // Create debris
    debris_mgr::GetDebrisMgr()->CreateSpecializedDebris( GetPosition(), vector3(0,1,0), object::TYPE_DEBRIS_FRAG_EXPLOSION, GetZones(), m_OriginGuid );


    //voice_id VoiceID = g_AudioMgr.Play( "Grenade_Explosion", GetPosition() );
    //g_AudioManager.NewAudioAlert( VoiceID, audio_manager::EXPLOSION, GetPosition(), GetZone1(), GetGuid() );

/* TODO: Put decals back
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
*/

    // If the pain originates from a ghost, then no damage will be done.
    pain Pain;
    Pain.Setup( m_PainHandle, m_OriginGuid, GetPosition() );
    Pain.SetDirection( m_Velocity );
    Pain.ApplyToWorld();

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
 //    SMP_UTIL_Broadcast_Alert( NotifyPackage ) ;
}

//=============================================================================================

void grenade_projectile::OnRender( void )
{
    // Don't render if exploded
    if( m_Exploded )
        return;

    // Setup Render Matrix
    m_RenderL2W = GetL2W();
    m_RenderL2W.SetRotation(m_TotalSpin);

    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();
    
    if( pRigidGeom )
    {
        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;
        
        m_RigidInst.Render( &m_RenderL2W, Flags );
    }

#ifdef X_EDITOR
//    draw_Line( GetPosition() , GetPosition() + m_NormalCollision );
//    draw_Line( GetPosition() , GetPosition() + m_Velocity , XCOLOR_BLUE );
#endif // X_EDITOR
}

//=============================================================================================

xbool grenade_projectile::LoadInstance( const char* pFileName )
{
    m_RigidInst.SetUpRigidGeom( pFileName );
    return TRUE;
}

//=============================================================================================
