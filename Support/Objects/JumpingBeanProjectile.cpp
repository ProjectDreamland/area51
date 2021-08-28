//==============================================================================
// JUMPING BEAN PROJECTILE
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "JumpingBeanProjectile.hpp"
#include "Entropy\e_Draw.hpp"
#include "audiomgr\audiomgr.hpp"
#include "render\LightMgr.hpp"
#include "Objects\AnimSurface.hpp"
#include "objects\ClothObject.hpp"
#include "objects\DestructibleObj.hpp"
#include "Decals\DecalMgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "Gamelib/DebugCheats.hpp"
#include "Objects/Player.hpp"
#include "x_files/x_plus.hpp"

//==============================================================================
// DEFINES
//==============================================================================

#define NO_CHAOS

#define GRENADE_ELASTICITY          0.4f
#define MAX_BOUNCES_PER_ADVANCE     2

//==============================================================================
//  LOCAL STORAGE
//==============================================================================

f32 jumping_bean_projectile::s_JumpingBean_Alert_Time = 1.2f;

static random Random;

//==============================================================================
// Tweaks
//==============================================================================

// Bounce Run factor.  Basically a velocity loss/elasticity value.  
// Lower means it will slow down, higher it will speed up on a bounce.  
// Over 1 is will accelerate, under 1 will decelerate.  1 is a constant velocity. 
tweak_handle s_JBEAN_BounceRunTweak             ("JBEAN_BounceRun");

// Bounce Rise factor.  MUST BE NEGATIVE. Basically a velocity loss/elasticity value.  
// Lower means it will slow down, higher it will speed up on a bounce.  
// Over 1 is will accelerate, under 1 will decelerate.  1 is a constant velocity.
tweak_handle s_JBEAN_BounceRiseTweak            ("JBEAN_BounceRise");

// Randomness of bounce; range goes from -n to +n.
tweak_handle s_JBEAN_BounceFactorTweak          ("JBEAN_BounceFactor");

// Randomness of bounce for first throw.  Make the angle small if you want it to not go behind player. Range goes from -n to +n.
tweak_handle s_JBEAN_BounceFactor_InitialTweak  ("JBEAN_BounceFactor_Initial");

// In NORMAL mode, how many times can we bounce?
tweak_handle s_JBEAN_MaxBouncesTweak            ("JBEAN_MaxBounces");

// duh
tweak_handle s_JBEAN_Gravity                    ("JBEAN_Gravity");

// How big is the trail?
tweak_handle s_JBEAN_TrailScaleTweak            ("JBEAN_TrailScale");

// How big is the explosion particle?
tweak_handle s_JBEAN_ExplosionScaleTweak        ("JBEAN_ExplosionScale");

// When the JBG takes off after falling, how fast does it fly?
tweak_handle JBG_TakeOffSpeedTweak              ("JBEAN_TakeOffSpeed");

// What percentage of the forward velocity does the JBG keep after engage stage.
tweak_handle JBG_VelocityScalarTweak            ("JBEAN_VelocityScalar");

// How much (in degrees) do we offset the throw for the JBG (+ is to the left).
tweak_handle JBG_ThrowOffsetTweak               ("JBEAN_ThrowOffset");        

// How long (in seconds) do we live?
tweak_handle JBG_LifeTimeTweak                  ("JBEAN_LifeTime");

//=============================================================================================
// OBJECT DESC
//=============================================================================================
static struct jumping_bean_projectile_desc : public object_desc
{
    jumping_bean_projectile_desc( void ) : object_desc( 
            object::TYPE_JUMPING_BEAN_PROJECTILE, 
            "Grenade Jumping Bean", 
            "WEAPON",
            object::ATTR_NEEDS_LOGIC_TIME   |
            object::ATTR_RENDERABLE,
            FLAGS_IS_DYNAMIC )
            {}

    virtual object* Create          ( void )
    {
        return new jumping_bean_projectile;
    }

} s_JumpingBean_Projectile_Desc;

//=============================================================================================

const object_desc&  jumping_bean_projectile::GetTypeDesc     ( void ) const
{
    return s_JumpingBean_Projectile_Desc;
}


//=============================================================================================
const object_desc&  jumping_bean_projectile::GetObjectType   ( void )
{
    return s_JumpingBean_Projectile_Desc;
}

//=============================================================================================
// Jumping Bean
//=============================================================================================

jumping_bean_projectile::jumping_bean_projectile()
{
    m_MaxAliveTime          = JBG_LifeTimeTweak.GetF32();
    m_ParticleExplosion     = particle_emitter::JUMPING_BEAN_EXPLOSION;

    m_ExplosionRadius       = 500.0f;
    m_Gravity               = s_JBEAN_Gravity.GetF32();

    m_MaxBounces            = s_JBEAN_MaxBouncesTweak.GetS32();

    m_bIsLargeProjectile    = TRUE;
    m_bBroadcastAlert       = TRUE;

#ifndef CONFIG_RETAIL
    m_GrenadeMode           = DEBUG_EXPERT_JUMPINGBEAN ? GM_EXPERT : GM_NORMAL;
#else
    // default
    m_GrenadeMode = GM_NORMAL;
#endif

    m_bStageInit            = TRUE;  // is this a stage initialization? (expert mode only)
    m_GrenadeStage          = GS_STAGE_LIFTOFF; // initial stage for expert mode

    const char* pGeomName = PRELOAD_FILE( "WPN_GRAV_Missle.rigidgeom" );
    LoadInstance( pGeomName );

    m_RigidInst.SetVMeshBit("Shell",TRUE);
    m_RigidInst.SetVMeshBit("Missile",FALSE);

    #ifdef NO_CHAOS
    m_BounceFactorRise  = -0.20f;
    m_BounceFactorRun   =  0.75f;
    #else
    m_RandomSeed        = x_irand( 0, 255 );
    Random.srand( (m_RandomSeed << 7) | m_RandomSeed );
    m_BounceFactorRise  = Random.frand( -0.20f, s_JBEAN_BounceRiseTweak.GetF32() );
    m_BounceFactorRun   = Random.frand(  0.75f, s_JBEAN_BounceRunTweak.GetF32()  );
    #endif
}

//=============================================================================================
void jumping_bean_projectile::SetStart( const vector3& Position, 
                        const radian3& Orientation,
                        const vector3& Velocity,
                        s32      Zone1, 
                        s32      Zone2,
                        f32      Gravity )
{
    net_proj::SetStart(Position, Orientation, Velocity, Zone1, Zone2, Gravity);

    // In MP games, there is no expert mode.
    #ifndef X_EDITOR
    if( GameMgr.GetGameType() != GAME_CAMPAIGN )
    {
        m_GrenadeMode = GM_NORMAL;
        return;
    }
    #endif

    // Origin guid is set, see if we have the JBG lore acquired.
    object* pObj = g_ObjMgr.GetObjectByGuid( m_OriginGuid );
    if( pObj && pObj->IsKindOf( player::GetRTTI() ) )
    {
        player* pPlayer = (player*)pObj;

        // is our JBG lore acquired?  If so, set grenade to expert mode
        if( pPlayer->GetJBGLoreAcquired() )
        {
            m_GrenadeMode = GM_EXPERT;
        }
    }
}

//=============================================================================================

jumping_bean_projectile::~jumping_bean_projectile( void )
{
    DestroyParticles();
}

//=============================================================================================

bbox jumping_bean_projectile::GetLocalBBox( void ) const 
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
vector3 jumping_bean_projectile::GetExpertTargetPoint( void )
{
    vector3 EndPos;

    object* pObj = g_ObjMgr.GetObjectByGuid( m_OriginGuid );
    if( pObj && pObj->IsKindOf( player::GetRTTI() ) )
    {
        player* pPlayer = (player*)pObj;
        pPlayer->GetProjectileHitLocation(EndPos, FALSE);
    }
    else
    {
        return vector3(0.0f, 0.0f, 0.0f);
    }

    return EndPos;
}

//=============================================================================================
f32 g_JBG_StageTime[GS_STAGE_MAX] = { 0.1f, 0.4f, 1.0f };
f32 g_ExplosionScale = 1.0f;

f32 g_JBG_VelocityScalarY = 0.0f;

void jumping_bean_projectile::StageLogic( f32 DeltaTime )
{
    (void)DeltaTime;
    switch( m_GrenadeStage )
    {
    case GS_STAGE_LIFTOFF:
        {
            if( m_bStageInit )
            {
                // store take off velocity vector for use in last stage (GS_STAGE_TAKEOFF)
                m_TakeoffTargetPoint = GetExpertTargetPoint();

                m_bStageInit = FALSE;

                // twist the velocity to the left a little so we can see the grenade
                vector3 TwistVelocity = m_StartVel;
                radian JBG_ThrowOffset = JBG_ThrowOffsetTweak.GetRadian();
                TwistVelocity.RotateY(JBG_ThrowOffset);
                m_StartVel = TwistVelocity;
            }
            else
            if( m_Age >= g_JBG_StageTime[m_GrenadeStage] )
            {
                // set up next stage
                m_GrenadeStage = GS_STAGE_FALL;
                m_bStageInit = TRUE;
            }
        }
        break;

    case GS_STAGE_FALL:
        {
            if( m_bStageInit )
            {
                m_bStageInit = FALSE;
                
                /*
                //render particle explosion
                m_FragmentParticle = particle_emitter::CreatePresetParticleAndOrient( 
                    particle_emitter::HARD_SPARK, 
                    vector3(0.0f,0.0f,0.0f), 
                    GetPosition() );
                    */

                rhandle<char> FxResource;
                FxResource.SetName( PRELOAD_FILE( "alien_grenade_frags.fxo" ) );

                // fragment
                m_FragmentParticle = particle_emitter::CreatePresetParticleAndOrient( 
                    FxResource.GetName(),
                    vector3(0.0f,0.0f,0.0f),
                    GetPosition() );

                m_RigidInst.SetVMeshBit("Shell",FALSE);
                m_RigidInst.SetVMeshBit("Missile",TRUE);
                
                m_StartPos = GetPosition();
                m_TimeT = 0.0f;

                vector3 FallVelocity = m_StartVel;
                f32 JBG_VelocityScalar = JBG_VelocityScalarTweak.GetF32()/100.0f; // make it a percentage
                FallVelocity.Set( m_StartVel.GetX() * JBG_VelocityScalar, m_StartVel.GetY() * g_JBG_VelocityScalarY, m_StartVel.GetZ() * JBG_VelocityScalar );
                m_StartVel = FallVelocity;

                object* pObj = g_ObjMgr.GetObjectByGuid( m_FragmentParticle );
                if( pObj )
                {
                    ((particle_emitter*)pObj)->SetScale( g_ExplosionScale );
                }

                g_AudioMgr.Play( "JBG_Launch", GetPosition(), GetZone1(), TRUE );
            }
            else
            if( m_Age >= g_JBG_StageTime[m_GrenadeStage] )
            {
                // set up next stage
                m_GrenadeStage = GS_STAGE_TAKEOFF;
                m_bStageInit = TRUE;
            }
        }
        break;

    case GS_STAGE_TAKEOFF:
        {
            if( m_bStageInit )
            {
                // clear variables
                m_Gravity  = 0.0f;                
                m_TimeT    = 0.0f;

                // set up new position and velocity         
                m_StartPos = GetPosition();

                // get the direction vector from the firing start position to where our trace hit
                vector3 ToTarget( m_TakeoffTargetPoint - GetPosition() );

                ToTarget.Normalize();

                // get take off velocity vector.
                m_StartVel = (ToTarget * JBG_TakeOffSpeedTweak.GetF32());

                rhandle<char> TrailFxResource;
                TrailFxResource.SetName( PRELOAD_FILE( "alien_grenade_trail.fxo" ) );

                // show expert smoke trail
                m_ExpertEffectTrail = particle_emitter::CreatePresetParticleAndOrient( 
                                                TrailFxResource.GetName(),
                                                vector3(0.0f,0.0f,0.0f),
                                                GetPosition() );

                rhandle<char> FxResource;
                FxResource.SetName( PRELOAD_FILE( "alien_grenade_Trail-Flame.fxo" ) );

                // show expert flame trail
                m_ExpertEffectTrailFlame = particle_emitter::CreatePresetParticleAndOrient( 
                    FxResource.GetName(),
                    vector3(0.0f,0.0f,0.0f),
                    GetPosition() );

                m_bStageInit = FALSE;

                // just make it hit default case
                m_GrenadeStage = GS_STAGE_MAX;
            }
        }
        break;

    default:
        break;
    }
}

//=============================================================================================
u8 g_JBG_BounceCount = 0;
void jumping_bean_projectile::OnAdvanceLogic( f32 DeltaTime )
{
    if( m_Exploded )
    {
        net_proj::OnAdvanceLogic( DeltaTime );
        return;
    }

    if( m_GrenadeMode == GM_EXPERT )
    {
        // clear bounces
        m_MaxBounces = g_JBG_BounceCount;

        // do stage logic
        StageLogic( DeltaTime );        
    }
    else
    {
        m_Gravity       = s_JBEAN_Gravity.GetF32();
        m_MaxBounces    = s_JBEAN_MaxBouncesTweak.GetS32();
    }

    net_proj::OnAdvanceLogic( DeltaTime );

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
        if (m_bBroadcastAlert && (m_Age > (m_MaxAliveTime - s_JumpingBean_Alert_Time )))
        {
            //give 1 second warning
            alert_package NotifyPackage ;
            NotifyPackage.m_Origin = GetGuid() ;
            NotifyPackage.m_Position = GetPosition();
            NotifyPackage.m_Type = alert_package::ALERT_TYPE_GRENADE ;
            NotifyPackage.m_Target = alert_package::ALERT_TARGET_NPC ;
            NotifyPackage.m_AlertRadius = m_ExplosionRadius * 0.8f ;
            NotifyPackage.m_Cause = m_OriginGuid ;
            NotifyPackage.m_FactionsSpecific = FACTION_NOT_SET;
            NotifyPackage.m_ZoneID = GetZone1();
            g_AudioManager.AppendAlertReceiver(NotifyPackage);
            m_bBroadcastAlert = FALSE;
        }
    }
}

//=============================================================================================

void jumping_bean_projectile::OnExplode( void )
{
    if( m_Exploded )
        return;

    net_proj::OnExplode();

    // Destroy the projectiles effect particles
    DestroyParticles();

    //add a light where the explosion occurred
    g_LightMgr.AddFadingLight( GetPosition(), xcolor(55, 55, 135, 255), 175.0f, 1.0f, 1.3f );
/*
    //render particle explosion
    guid ExplosionGuid = particle_emitter::CreatePresetParticleAndOrient( 
        m_ParticleExplosion, 
        vector3(0.0f,0.0f,0.0f), 
        GetPosition() );

    object* pObj = g_ObjMgr.GetObjectByGuid( ExplosionGuid );
    if( pObj )
    {
        f32 fExplosionScale = s_JBEAN_ExplosionScaleTweak.GetF32();
        ((particle_emitter*)pObj)->SetScale( fExplosionScale );
    }
*/
    debris_mgr::GetDebrisMgr()->CreateSpecializedDebris( GetPosition(), m_ImpactNormal, object::TYPE_DEBRIS_ALIEN_GRENADE_EXPLOSION, GetZones(),m_OriginGuid );

    voice_id VoiceID = g_AudioMgr.Play( "MSN_Projectile_Explode", GetPosition(), GetZone1(), TRUE );
    g_AudioManager.NewAudioAlert( VoiceID, audio_manager::EXPLOSION, GetPosition(), GetZone1(), GetGuid() );

    /*
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
    */

    CausePain();
}

//=============================================================================

void jumping_bean_projectile::CausePain( guid HitGuid )
{
#ifndef X_EDITOR
    // No pain when it's a ghost projectile
    if( m_OwningClient != g_NetworkMgr.GetClientIndex() )
        return;
#endif

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
        }
    }

    Pain.SetDirection( m_Velocity );
    //SH:Commenting this out for now, until I get corrected values for the pain.
    // The explosion will dish out pain also, and it is currently set to the same
    // pain numbers as the bean pain.
    //SH:02Nov2004-pain is back in so that design can have the dual pain setup, as per jim.
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

void jumping_bean_projectile::OnMove( const vector3& NewPosition )
{
   net_proj::OnMove(NewPosition);

   UpdateParticles(NewPosition);
}

//=============================================================================================

void jumping_bean_projectile::UpdateParticles( const vector3& NewPosition )
{
    // Exit if the projectile already exploded
    if( m_Exploded )
        return;

    // trail
    if( !m_EffectTrail && (m_GrenadeMode != GM_EXPERT) )
    {
        // Create the effect trail.
        m_EffectTrail = particle_emitter::CreatePresetParticleAndOrient( particle_emitter::JUMPING_BEAN_TRAIL, vector3(0.0f,0.0f,0.0f), NewPosition );

        object* pObj = g_ObjMgr.GetObjectByGuid( m_EffectTrail );
        if( pObj )
        {
            f32 fTrailScale = s_JBEAN_TrailScaleTweak.GetF32();
            ((particle_emitter*)pObj)->SetScale( fTrailScale );
        }
    }  

    if( m_GrenadeMode != GM_EXPERT )
    {
        object* pObj  = g_ObjMgr.GetObjectByGuid( m_EffectTrail );

        if( pObj != NULL )
        {
            matrix4 L2W;
            L2W.Identity();
            L2W.SetTranslation( NewPosition );

            // set proper trail rotation
            radian3 ParticleRotation( m_Velocity.GetPitch(), m_Velocity.GetYaw(), R_0 );

            L2W.SetRotation( ParticleRotation );

            pObj->OnTransform( L2W );
            pObj->OnMove( NewPosition );

            // change color        
            switch( m_ImpactCount )
            {
            case 1:
                {
                    ((particle_emitter*)pObj)->SetColor( XCOLOR_BLUE );
                }
                break;
            case 2:
                {
                    ((particle_emitter*)pObj)->SetColor( XCOLOR_RED );
                }
                break;
            case 3:
                {
                    ((particle_emitter*)pObj)->SetColor( XCOLOR_WHITE );
                }
                break;

            case 0:
            default:
                ((particle_emitter*)pObj)->SetColor( XCOLOR_PURPLE );
                break;
            }
        }
        else
        {
            m_EffectTrail = NULL_GUID;
            return;
        }
    }
    else // Expert Mode
    {
        // smoke 
        object* pObj  = g_ObjMgr.GetObjectByGuid( m_ExpertEffectTrail );

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
            m_ExpertEffectTrail = NULL_GUID;
        }

        // flames
        pObj  = g_ObjMgr.GetObjectByGuid( m_ExpertEffectTrailFlame );

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
            m_ExpertEffectTrailFlame = NULL_GUID;
        }
    }    
}

//=============================================================================================

void jumping_bean_projectile::DestroyParticles( void )
{
    // Destroy the particle effect.
    if( m_EffectTrail )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_EffectTrail );
        if( pObj )
        {
            particle_emitter& Particle = particle_emitter::GetSafeType( *pObj );
            Particle.DestroyParticle();
        }
        m_EffectTrail = NULL_GUID;
    }

    // expert mode smoke trail
    if( m_ExpertEffectTrail )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_ExpertEffectTrail );
        if( pObj )
        {
            particle_emitter& Particle = particle_emitter::GetSafeType( *pObj );
            Particle.DestroyParticle();
        }
        m_ExpertEffectTrail = NULL_GUID;
    }

    // expert mode fire trail
    if( m_ExpertEffectTrailFlame )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_ExpertEffectTrailFlame );
        if( pObj )
        {
            particle_emitter& Particle = particle_emitter::GetSafeType( *pObj );
            Particle.DestroyParticle();
        }
        m_ExpertEffectTrailFlame = NULL_GUID;
    }

    // fragmentation particles
    if( m_FragmentParticle )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_FragmentParticle );
        if( pObj )
        {
            particle_emitter& Particle = particle_emitter::GetSafeType( *pObj );
            Particle.DestroyParticle();
        }
        m_FragmentParticle = NULL_GUID;
    }
}

//=============================================================================================
void jumping_bean_projectile::OnImpact( collision_mgr::collision& Coll, object* pTarget )
{
    xbool bCanBounce = TRUE;

    if( m_GrenadeMode != GM_EXPERT )
    {
        #ifndef NO_CHAOS
        s32 Seed = m_RandomSeed + m_ImpactCount;
        Random.srand( (Seed << 7) | Seed );
        f32 fJBGBounceFactor = (m_ImpactCount == 0) 
                             ? s_JBEAN_BounceFactor_InitialTweak.GetF32() 
                             : s_JBEAN_BounceFactorTweak.GetF32();
        radian Angle = Random.frand( -fJBGBounceFactor, fJBGBounceFactor );
    //  CLOG_MESSAGE( (m_ImpactCount == 0), 
    //                "jumping_bean_projectile::OnImpact", 
    //                "Seed:%d - Angle:%g", m_RandomSeed, Angle );
        m_Velocity.RotateY( Angle );
        #endif
    }
    else
    {
        if( m_GrenadeStage != GS_STAGE_TAKEOFF )
        {
            bCanBounce = TRUE;
        }
        else
        {
            bCanBounce = FALSE;
        }
    }

    net_proj::OnImpact( Coll, pTarget );

    xbool bHitObject = CheckHitValidObject(pTarget);

    // Explode if number of impacts is exceeded
    if( (bCanBounce && (m_ImpactCount > m_MaxBounces)) || bHitObject )
    {
#ifndef X_EDITOR
        if( m_OwningClient == g_NetworkMgr.GetClientIndex() )
#endif
        {
            // Move to new position and explode.
            OnMove( m_NewPos );
            OnExplode();
        }

        m_AtRest = TRUE;

        return;
    }
    else
    {
        // play bounce sound
        if( bCanBounce && !bHitObject )
        {
            g_AudioMgr.Play( "JBG_Bounce", GetPosition(), GetZone1(), TRUE );
        }
    }
}

//=============================================================================================
void jumping_bean_projectile::OnRender( void )
{
    // Don't render if exploded
    if( m_Exploded )
        return;

    // Setup Render Matrix
    m_RenderL2W = GetL2W();

    radian Pitch, Yaw;
    m_Velocity.GetPitchYaw(Pitch, Yaw);
    radian3 Rot(R_0, Yaw, R_0);
    
    // take off stage, need to take pitch into account.  Make sure we aren't in init phase or rotation will still be in "fall" state
    if( m_GrenadeStage >= GS_STAGE_TAKEOFF && !m_bStageInit )
    {
        Rot.Set(Pitch, Yaw, R_0);    
    }

    m_RenderL2W.SetRotation(Rot);

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

xbool jumping_bean_projectile::LoadInstance( const char* pFileName )
{
    m_RigidInst.SetUpRigidGeom( pFileName );
    return TRUE;
}

//==============================================================================
#ifndef X_EDITOR
//==============================================================================

void jumping_bean_projectile::net_AcceptActivate( const bitstream& BS )
{
    (void)BS;
    #ifdef NO_CHAOS
    m_BounceFactorRise  = -0.20f;
    m_BounceFactorRun   =  0.75f;
    #else
    BS.ReadRangedS32( m_RandomSeed, 0, 255 );
    Random.srand( (m_RandomSeed << 7) | m_RandomSeed );
    m_BounceFactorRise  = Random.frand( -0.20f, s_JBEAN_BounceRiseTweak.GetF32() );
    m_BounceFactorRun   = Random.frand(  0.75f, s_JBEAN_BounceRunTweak.GetF32()  );
    #endif
}

//==============================================================================

void jumping_bean_projectile::net_ProvideActivate( bitstream& BS )
{
    (void)BS;
    #ifndef NO_CHAOS
    BS.WriteRangedS32( m_RandomSeed, 0, 255 );
    #endif
}

//==============================================================================

void jumping_bean_projectile::net_AcceptStart( const bitstream& BS )
{
    BS.ReadRangedS32( m_ImpactCount, 0, 7 );
}

//==============================================================================

void jumping_bean_projectile::net_ProvideStart( bitstream& BS )
{
    ASSERT( m_MaxBounces <= 7 );
    BS.WriteRangedS32( m_ImpactCount, 0, 7 );
}

//==============================================================================
#endif // ifndef X_EDITOR
//==============================================================================
