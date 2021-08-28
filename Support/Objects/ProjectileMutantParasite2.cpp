//=============================================================================================
// MUTANT PARASITE PROJECTILE
//=============================================================================================

//=============================================================================================
// INCLUDES
//=============================================================================================
#include "ProjectileMutantParasite2.hpp"
#include "Entropy\e_Draw.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "render\LightMgr.hpp"
#include "Decals\DecalMgr.hpp"
#include "Objects\ParticleEmiter.hpp"
#include "Objects\Player.hpp"
#include "Characters\MutantTank\Mutant_Tank.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "NetworkMgr\NetworkMgr.hpp"


#include "Objects/ClothObject.hpp"
#include "Objects/Flag.hpp"
#include "Objects/Actor/Actor.hpp"
#include "Characters/Character.hpp"
#include "Objects/Corpse.hpp"
#include "Objects/DestructibleObj.hpp"
#include "Objects/SuperDestructible.hpp"
#include "Objects/Turret.hpp"
#include "Objects/AlienShield.hpp"
#include "Objects/alienglob.hpp"

#define ENABLE_LOGGING 0

#define REQUIRE_VISIBILITY_TO_TARGET 

#define EXPLODE_FADE_TIME (0.400f)

//=========================================================================
// EXTERNALS
//=========================================================================

extern s32 g_Difficulty;
extern const char* DifficultyText[];

//=============================================================================================
// STATICS & CONSTANTS
//=============================================================================================

static tweak_handle s_TWEAK_ParasiteTargetingRange      ("PARASITE_TargetingRange");
static tweak_handle s_TWEAK_ParasiteTargetingAngle      ("PARASITE_TargetingAngle");
static tweak_handle s_TWEAK_ParasiteStrikeDistance      ("PARASITE_StrikeDistance");

static tweak_handle s_TWEAK_ParasiteSpeed               ("MUTATION_PRIMARY_SPEED");
static tweak_handle s_TWEAK_ParasitePitchMax            ("PARASITE_PitchMax");
static tweak_handle s_TWEAK_ParasitePitchDist           ("PARASITE_PitchDist");

static const f32    k_TRAJECTORY_UPDATE_THRESHOLD       = R_5;
static       xbool  k_REQUIRE_VISIBILITY_CHECK          = TRUE;
static       f32    k_WANDER_RADIUS                     = 25.0f;
static       f32    k_WANDER_SPEED                      = 2.0f;
static       f32    k_INITIAL_LAUNCH_SPREAD_ANGLE       = R_7;

static radian s_MaxRotationSpeed = R_60;


struct parasite_type_constants
{
    const char*     m_ImpactLivingFXName;
    const char*     m_ImpactInanimateFXName;
    const char*     m_MainBodyFXName;

    const char*     m_AudioImpactLiving;
    const char*     m_AudioImpactInanimate;
    const char*     m_AudioFlying;
    const char*     m_AudioLaunch;
};


parasite_type_constants s_ParasiteTweak[2] = { {"Parasite_Impact_000.fxo",
                                                "MucousBurst_000.fxo",
                                                "Parasite_000.fxo",
                                                "PARASITE_PRIMARY_HIT_TARGET",
                                                "PARASITE_PRIMARY_MISS_TARGET",
                                                "PARASITE_PRIMARY_FLY_LOOP",
                                                "PARASITE_PRIMARY_FIRE" },

                                               {"Parasite_Impact_000.fxo",
                                                "MucousBurst_000.fxo",
                                                "Contagion_missle.fxo",
                                                "PARASITE_PRIMARY_HIT_TARGET",
                                                "PARASITE_PRIMARY_MISS_TARGET",
                                                "PARASITE_PRIMARY_FLY_LOOP",
                                                "PARASITE_PRIMARY_FIRE" } };



//=============================================================================================
// OBJECT DESC
//=============================================================================================
static struct mutant_parasite_projectile_desc : public object_desc
{
    mutant_parasite_projectile_desc( void ) : object_desc( 
            object::TYPE_MUTANT_PARASITE_PROJECTILE, 
            "Mutant Parasite Projectile", 
            "WEAPON",
            object::ATTR_NEEDS_LOGIC_TIME   |
            object::ATTR_COLLIDABLE         | 
            object::ATTR_BLOCKS_ALL_PROJECTILES | 
            object::ATTR_NO_RUNTIME_SAVE    |
            object::ATTR_RENDERABLE         | 
            object::ATTR_TRANSPARENT        |
            object::ATTR_SPACIAL_ENTRY,
            FLAGS_IS_DYNAMIC  )
            {}

    virtual object* Create          ( void )
    {
        return new mutant_parasite_projectile;
    }

} s_Mutant_Parasite_Projectile_Desc;

//=============================================================================================

const object_desc&  mutant_parasite_projectile::GetTypeDesc     ( void ) const
{
    return s_Mutant_Parasite_Projectile_Desc;
}


//=============================================================================================
const object_desc&  mutant_parasite_projectile::GetObjectType   ( void )
{
    return s_Mutant_Parasite_Projectile_Desc;
}

//=============================================================================================

mutant_parasite_projectile::mutant_parasite_projectile()
{
    m_LaunchFXGuid = NULL_GUID;

    m_FlyAudioID        = -1;
    m_LaunchAudioID     = -1;
    m_InfectAudioID     = -1;
    m_CollisionAudioID  = -1;
    
    m_FlyVoiceID        = -1;

    m_MaxAliveTime      = 10.0f;
    m_Age               = 0.0f;

    m_MaxRotationSpeed  = s_MaxRotationSpeed;

//  m_WanderOffset      = x_frand(0.7f,1.4f);
//  m_WanderPhase       = x_frand(0,2*PI);
    m_WanderOffset      = ((x_irand( 0, 255 ) / 255.0f) * 0.7f) + 0.7f;
    m_WanderPhase       = ((x_irand( 0, 255 ) / 255.0f) * 2*PI);

    m_RetargetingTimer  = 0.2f;

    m_BaseSpeed         = 800.0f;               //s_TWEAK_ParasiteSpeed.GetF32();
    m_ActualSpeed       = m_BaseSpeed;
    m_DesiredSpeed      = m_BaseSpeed;

    m_Target            = NULL_GUID; 
    m_TargetPoint.Set(0,0,0);

    m_Gravity = 0;        

    m_TypeIndex = 0;

    m_CalledSetupType      = FALSE;
    m_ExplodeEffectCreated = FALSE;

    // Setup for parasite
    m_SlowestSpeedMultiplier    = 0.25f; 
}

//=============================================================================================

mutant_parasite_projectile::~mutant_parasite_projectile()
{    
    if ( m_LaunchFXGuid )
    {
        g_ObjMgr.DestroyObject( m_LaunchFXGuid );
    }

    if (m_FXFly.Validate())
    {
        m_FXFly.KillInstance();
    }

    g_AudioMgr.Release( m_FlyVoiceID, 0.10f );
}

//=============================================================================

void mutant_parasite_projectile::OnAdvanceLogic( f32 DeltaTime )
{
    if( m_Exploded )
    {
        net_proj::OnAdvanceLogic( DeltaTime );
        
        m_ExplodeFadeTimer -= DeltaTime;
        if( m_ExplodeFadeTimer < 0.0f )
            m_ExplodeFadeTimer = 0.0f;
        u8 Alpha = (u8)(255 * m_ExplodeFadeTimer / EXPLODE_FADE_TIME);
        m_FXFly.SetColor( xcolor( 255,255,255, Alpha ) );
        m_FXFly.AdvanceLogic( DeltaTime );
        return;
    }

    ASSERT( m_StartVel.Length() > 1.0f );

    m_LastLogicDeltaTime = DeltaTime;

#ifndef X_EDITOR
    CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "OnAdvLogic: Advancing [%f]",DeltaTime);
    CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "OnAdvLogic: m_StartVel = [%f, %f, %f], Dirtybits=%08X",m_StartVel.GetX(),m_StartVel.GetY(),m_StartVel.GetZ(),m_NetDirtyBits);
#endif

    //net_proj::OnAdvanceLogic( DeltaTime );
    CheckInternalCollision( DeltaTime );

    ASSERT( m_StartVel.Length() > 1.0f );

    if( (GetAttrBits() & object::ATTR_DESTROY) )
        return;

    // Explode if lifetime has expired
    m_Age += DeltaTime;
    if( m_Age > m_MaxAliveTime )
    {
#ifndef X_EDITOR
        if( m_OwningClient == g_NetworkMgr.GetClientIndex() )
#endif
        {
            CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "OnAdvLogic: Lifetime exceeded.  Exploding.");
            OnExplode();
        }
        return;
    }

    // Age the blend
    m_BlendTimer -= DeltaTime;
    if( m_BlendTimer < 0.0f )
    {
        m_BlendTimer = 0.0f;
    }

    // UPDATE PARTICLE FX
    if (m_FXFly.Validate())
    {   
        matrix4 L2W = GetL2W();
        L2W.SetTranslation( m_WanderPosition );
#ifndef X_EDITOR
        L2W.Translate( net_GetBlendOffset() );
        f32 Scale = 1.0f - (m_BlendTimer / m_BlendTotalTime);
        L2W.SetScale( vector3( Scale, Scale, Scale ) );
#endif
        m_FXFly.SetTransform( L2W );
        m_FXFly.SetSuspended( FALSE );
        m_FXFly.AdvanceLogic( DeltaTime );
    }

    // UPDATE AUDIO
    if( m_FlyVoiceID )
    {
        f32 PitchShiftStartDist = s_TWEAK_ParasitePitchDist.GetF32();
        f32 PitchShiftMax = s_TWEAK_ParasitePitchMax.GetF32();
        f32 Dist = (m_TargetPoint - m_WanderPosition).Length();
        f32 Pitch = 1.0f;

        if (PitchShiftStartDist == 0)
            PitchShiftStartDist = 1200.0f;
        if (PitchShiftMax == 0)
            PitchShiftMax = 0.5f;

        if (Dist < PitchShiftStartDist)
        {
            f32 Delta = PitchShiftMax * (1.0f - (Dist / PitchShiftStartDist));
            Pitch += Delta;
        }
        g_AudioMgr.SetPosition(m_FlyVoiceID, m_WanderPosition, GetZone1() );
        g_AudioMgr.SetPitch( m_FlyVoiceID, Pitch );
    }  

    CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "OnAdvLogic: Updating trajectory");
    ASSERT( m_StartVel.Length() > 1.0f );
    UpdateTrajectory( DeltaTime );
    ASSERT( m_StartVel.Length() > 1.0f );

    // Do some more complex thinking on the origination machine
#ifndef X_EDITOR
    if( m_OwningClient != g_NetworkMgr.GetClientIndex() )
    {
        CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "OnAdvLogic: -- Done early - m_StartVel = [%f, %f, %f]",m_StartVel.GetX(),m_StartVel.GetY(),m_StartVel.GetZ());
        return;
    }
#endif

    CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "OnAdvLogic: Updating target");
    UpdateTarget( DeltaTime );

    CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "OnAdvLogic: -- Done: m_StartVel = [%f, %f, %f]",m_StartVel.GetX(),m_StartVel.GetY(),m_StartVel.GetZ());

    ASSERT( m_StartVel.Length() > 1.0f );
}

//=============================================================================

void mutant_parasite_projectile::Initialize( const vector3&         InitPos,
                                             const radian3&         InitRot,
                                             const vector3&         InheritedVelocity,
                                                   f32              Speed,
                                                   guid             OwnerGuid,
                                                   pain_handle      PainHandle,
                                                   s32              OwnerNetSlot,
                                                   xbool            bContagious )
{
    (void)OwnerGuid;

    CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), 
        "Initialize: Speed(%f) Contagious(%s)",Speed,bContagious?"TRUE":"FALSE");

    vector3 Velocity(0,0,Speed);
    Velocity.Rotate( InitRot );
    Velocity += InheritedVelocity;

    m_BaseSpeed         = Speed;
    m_ActualSpeed       = Speed;
    m_DesiredSpeed      = Speed;
    m_WanderPosition    = InitPos;
    m_OldWanderPosition = InitPos;
    m_bContagious       = bContagious;
    
    {
        vector3 Dir = m_Velocity;
        Dir.NormalizeAndScale( 10000.0f );
        m_TargetPoint = InitPos + Dir;
    }

    ASSERT( m_WanderPosition.IsValid() );

    if( !bContagious )
    {
        // Parasites have some randomness
        f32 X = x_frand(-1,1);
        f32 Y = x_frand(-1,1);
        f32 Z = x_frand(-1,1);
        vector3 Dir(X,Y,Z);
        Dir.Normalize();

        radian RandomRange = k_INITIAL_LAUNCH_SPREAD_ANGLE;
        quaternion Q(Dir, x_frand(-RandomRange,RandomRange));

        Velocity = Q * Velocity;
    }
    
    Setup( OwnerGuid, OwnerNetSlot, InitPos, InitRot, 
           Velocity, GetZone1(), GetZone2(), PainHandle );
    
    SetupType(); 
}

//=============================================================================

void mutant_parasite_projectile::OnColCheck( void )
{
    // do nothing
}

//=============================================================================

//=============================================================================
/*
void mutant_parasite_projectile::ResetTargetCheckDelay( void )
{
    m_TargetCheckDelay = x_frand( 0.5f, 0.75f );        
}
*/
//=============================================================================

void mutant_parasite_projectile::OnRender( void )
{
#ifdef X_EDITOR
#ifdef X_DEBUG
    if (!m_Velocity.IsValid())
        return;

    draw_ClearL2W();

    draw_Line( GetPosition(), m_TargetPoint, XCOLOR_RED );

    vector3 Dir = m_Velocity;
    Dir.NormalizeAndScale( 400.0f );

    draw_Line( GetPosition(), GetPosition() + Dir, XCOLOR_YELLOW );

    vector3 CurrentTraj = m_Velocity;
    vector3 IdealTraj   = m_TargetPoint - GetPosition();

    CurrentTraj.Normalize();
    IdealTraj.Normalize();

    vector3 Axis;
    radian  Angle;

    CurrentTraj.GetRotationTowards( IdealTraj, Axis, Angle );

    // Build angle ourselves
    f32 D = IdealTraj.Dot(CurrentTraj);
    Angle = x_acos(D);

    f32 MaxAngle = m_MaxRotationSpeed * m_LastLogicDeltaTime;

    Angle = MAX(-MaxAngle,MIN(MaxAngle,Angle));

    quaternion Rot( Axis, Angle );

    Axis.NormalizeAndScale(400.0f);
    draw_Line( GetPosition(), GetPosition() + Axis, XCOLOR_GREEN );

    Dir = Rot.Rotate( Dir );
    draw_Line( GetPosition(), GetPosition() + Dir, XCOLOR_AQUA );

    draw_Sphere( m_TargetPoint, 10, XCOLOR_RED );
#endif
#endif
}

//=============================================================================

void mutant_parasite_projectile::OnRenderTransparent( void )
{
    if (m_FXFly.Validate())
    {
        m_FXFly.Render();
    }

#ifdef X_EDITOR
#ifdef X_DEBUG
    if (!m_Velocity.IsValid())
        return;

    draw_ClearL2W();

    draw_Line( GetPosition(), m_TargetPoint, XCOLOR_RED );

    vector3 Dir = m_Velocity;
    Dir.NormalizeAndScale( 400.0f );

    draw_Line( GetPosition(), GetPosition() + Dir, XCOLOR_YELLOW );

    vector3 CurrentTraj = m_Velocity;
    vector3 IdealTraj   = m_TargetPoint - GetPosition();

    CurrentTraj.Normalize();
    IdealTraj.Normalize();

    vector3 Axis;
    radian  Angle;

    CurrentTraj.GetRotationTowards( IdealTraj, Axis, Angle );

    // Build angle ourselves
    f32 D = IdealTraj.Dot(CurrentTraj);
    Angle = x_acos(D);

    f32 MaxAngle = m_MaxRotationSpeed * m_LastLogicDeltaTime;

    Angle = MAX(-MaxAngle,MIN(MaxAngle,Angle));

    quaternion Rot( Axis, Angle );

    Axis.NormalizeAndScale(400.0f);
    draw_Line( GetPosition(), GetPosition() + Axis, XCOLOR_GREEN );

    Dir = Rot.Rotate( Dir );
    draw_Line( GetPosition(), GetPosition() + Dir, XCOLOR_AQUA );
#endif
#endif
}

//=============================================================================

// MP HACK MP HACK
// MP HACK MP HACK
// MP HACK MP HACK
f32 MPParasiteHealth = 10.0f;
// MP HACK MP HACK
// MP HACK MP HACK
// MP HACK MP HACK

xbool mutant_parasite_projectile::HandleCollision( collision_mgr::collision& Collision, object* pObjectHit )
{
    xbool bCollided = TRUE;

#ifndef X_EDITOR
    if( m_OwningClient != g_NetworkMgr.GetClientIndex() )
    {
        // We should take note of the most recent impact point,
        // but other than that, we can't act on this call.
        return FALSE;
    }
#endif

    if( pObjectHit )
    {
        if( pObjectHit->IsKindOf( mutant_parasite_projectile::GetRTTI() ) )
        {
            bCollided = FALSE;
        }
        else
        {            
            // If the object that was hit is not being destroyed
            // then dish some pain out to it.
            if( !(pObjectHit->GetAttrBits() & ATTR_DESTROY) )
            {    
                // Contagion doesn't harm?
                if( !m_bContagious )
                {
                    pain Pain;
                    Pain.Setup( m_PainHandle, m_OriginGuid, Collision.Point );
                    Pain.SetDirection( m_Velocity );
                    Pain.SetDirectHitGuid( pObjectHit->GetGuid() );
                    Pain.ApplyToObject( pObjectHit );
                }
            }

            // Shouldn't count things that aren't your enemy.
            xbool bIsEnemy       = FALSE;
            xbool bTargetIsActor = FALSE;

            object* pOrigin      = g_ObjMgr.GetObjectByGuid( m_OriginGuid );
            actor*  pOriginActor = NULL;

            // make sure our object is an actor
            if( pObjectHit->IsKindOf( actor::GetRTTI() ) )
            {
                bTargetIsActor = TRUE;

                actor* pActorHit = (actor*)pObjectHit;

                // make sure the owner is an actor
                if( pOrigin && pOrigin->IsKindOf( actor::GetRTTI() ) )
                {
                    pOriginActor = (actor*)pOrigin;

                    // if this isn't an enemy, we can blow up but don't get the health from it
                    if( pOriginActor->IsEnemyFaction( pActorHit->GetFaction() ) )
                    {
                        bIsEnemy = TRUE;
                    }
                }
            }

            // Launch appropriate effect and audio.  Make sure we are alive and 
            // an enemy.
            if( (pObjectHit->GetAttrBits() & ATTR_LIVING) && bIsEnemy )
            {
                particle_emitter::CreatePresetParticleAndOrient( 
                    s_ParasiteTweak[ m_TypeIndex ].m_ImpactLivingFXName, 
                    g_CollisionMgr.m_Collisions[0].Plane.Normal, 
                    GetPosition(),
                    GetZone1() );    

                // Give our owner some health because they are an actor.                
                if( pOriginActor )
                {
                    // Get actor.
                    actor& OriginActor = actor::GetSafeType( *pOrigin );  

                    if( m_bContagious )
                    {    
                        // Actors can become contagious
                        if( bTargetIsActor )
                        {
                            actor& ActorHit = actor::GetSafeType( *pObjectHit );

                            // Campaign?
                            if( ActorHit.IsCharacter() )
                            {
                                #ifndef X_EDITOR
                                ASSERT( !GameMgr.IsGameMultiplayer() );
                                #endif
                                ActorHit.InitContagion();
                            }

                            // Split screen?
                            if( ActorHit.IsPlayer() )
                            {
                                #ifndef X_EDITOR
                                ASSERT( GameMgr.IsGameMultiplayer() );
                                ActorHit.InitContagion( pOriginActor->net_GetSlot() );
                                #endif
                            }

                            // Online?
                            if( ActorHit.IsNetGhost() )
                            {
                                // We do NOT activate the contagion here.  It
                                // will get turned on by the server in a little
                                // while.  Applying the initial pain is the
                                // trigger.
                                #ifndef X_EDITOR
                                ASSERT( GameMgr.IsGameMultiplayer() );
                                #endif

                                // Give out some good pain lovin'.
                                pain Pain;
                                pain_handle PainHandle( "CONTAGION_INITIAL_INFECTION" );
                                Pain.Setup( PainHandle, pOriginActor->GetGuid(), 
                                            ActorHit.GetPosition() + vector3(0,100,0) );
                                Pain.SetDirectHitGuid( ActorHit.GetGuid() );

                                // Smack the target.
                                Pain.ApplyToObject( ActorHit.GetGuid() );                              
                            }
                        }
                    }
                    else
                    {
                        s32 Difficulty = 0; // Easy

                        // xstring to build string for health return difficulty tweak (if available)
                        xstring StringPrefix;

                        if( OriginActor.IsKindOf( mutant_tank::GetRTTI() ) )
                        {
                            // What percent of health does each parasite give the Theta when it hits the player?
                            //tweak_handle TWEAK_ThetaParasiteHealthTweak( xfs( "THETA_ParasiteHealthPct_%s", DifficultyText[Difficulty] ) );
                            //HealthTweak = TWEAK_ThetaParasiteHealthTweak.GetF32();
                            StringPrefix = (const char*)"THETA_ParasiteHealthPct";
                        }
                        else
                        {
                            // What percent of health does each parasite return when it hits?
                            //tweak_handle TWEAK_PrimaryHealthTweak( xfs( "MUTATION_PrimaryHealthPct_%s", DifficultyText[Difficulty] ) );
                            //HealthTweak = TWEAK_PrimaryHealthTweak.GetF32();
                            StringPrefix = (const char*)"MUTATION_PrimaryHealthPct";
                        }                        
#ifndef X_EDITOR
                        // get real difficulty
                        if( GameMgr.GetGameType() == GAME_CAMPAIGN )
#endif
                        {
                            // difficulty only applies in Campaign.
                            Difficulty = g_Difficulty;
                            
                            // append difficulty.  Online has its own tweaks and Editor should just be Easy.
                            StringPrefix += (const char*)xfs( "_%s", DifficultyText[Difficulty] );
                            
                        }
      
                        // get a const pointer as an alias so we can pass it in to tweak_handle constructor.
                        const char* pString = (const char*)StringPrefix;

                        // get tweak;
                        tweak_handle TWEAK_HealthReturnTweak( pString );

                        // Lookup health tweak
                        f32 HealthTweak = TWEAK_HealthReturnTweak.GetF32();

                        // Calculate percentage of max health
                        f32 HealthReturned = ((HealthTweak * OriginActor.GetMaxHealth()) / 100.0f);

                        #ifndef X_EDITOR

                        // MP HACK MP HACK
                        // MP HACK MP HACK
                        // MP HACK MP HACK
                        if( GameMgr.GetGameType() != GAME_CAMPAIGN )
                            HealthReturned = MPParasiteHealth;
                        // MP HACK MP HACK
                        // MP HACK MP HACK
                        // MP HACK MP HACK

                        if( g_NetworkMgr.IsClient() )
                        {
                            OriginActor.net_ReportNetPain( 
                                    OriginActor.net_GetSlot(),
                                    OriginActor.net_GetSlot(), 
                                    0,
                                    OriginActor.net_GetLifeSeq(),
                                    FALSE,
                                    -HealthReturned );
                        }
                        else
                        #endif
                        {
                            OriginActor.AddHealth( HealthReturned );
                        }
                    }
                }

                g_AudioMgr.Play( s_ParasiteTweak[ m_TypeIndex ].m_AudioImpactLiving, 
                                 GetPosition(), GetZone1(), TRUE );

                g_LightMgr.AddFadingLight( 
                    GetPosition(), 
                    xcolor(225,209,146),
                    300.0f,
                    0.75f,
                    0.1f );
            }
            else
            {
                particle_emitter::CreatePresetParticleAndOrient( 
                    s_ParasiteTweak[ m_TypeIndex ].m_ImpactInanimateFXName, 
                    g_CollisionMgr.m_Collisions[0].Plane.Normal, 
                    GetPosition(),
                    GetZone1() );    

                g_AudioMgr.Play( s_ParasiteTweak[ m_TypeIndex ].m_AudioImpactInanimate, 
                                 GetPosition(), GetZone1(), TRUE );
            }

            m_ExplodeEffectCreated = TRUE;
        }
    }        
    return bCollided;
}

//=============================================================================

void mutant_parasite_projectile::SetTarget( guid Target )
{
    m_Target = Target;
    UpdateTargetPoint();
    //ResetTargetCheckDelay();
}

//=============================================================================

void mutant_parasite_projectile::OnExplode( void )
{
    if( m_Exploded )
        return;

    // Kill off effects
    if (m_FlyVoiceID != -1)
        g_AudioMgr.Release( m_FlyVoiceID, 0 );
    if (m_FXFly.Validate())
    {
    //  m_FXFly.KillInstance();
        m_FXFly.SetSuspended( TRUE );
        m_ExplodeFadeTimer = EXPLODE_FADE_TIME;
    }

    // Create explosion effect
    if( !m_ExplodeEffectCreated )
    {
        particle_emitter::CreatePresetParticleAndOrient( s_ParasiteTweak[ m_TypeIndex ].m_ImpactInanimateFXName, 
            g_CollisionMgr.m_Collisions[0].Plane.Normal, 
            GetPosition(),
            GetZone1() );    
        g_AudioMgr.Play( s_ParasiteTweak[ m_TypeIndex ].m_AudioImpactInanimate, GetPosition(), GetZone1(), TRUE );
        m_ExplodeEffectCreated = TRUE;
    }

    net_proj::OnExplode();
}

//=============================================================================

void mutant_parasite_projectile::OnImpact( collision_mgr::collision& Coll, object* pTarget )
{
    if (HandleCollision( Coll, pTarget ))
    {
        m_AtRest    = TRUE;
        m_Finished  = TRUE;
        m_Impact    = TRUE;

        CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "OnImpact: Collision detected");
#ifndef X_EDITOR
        if( m_OwningClient != g_NetworkMgr.GetClientIndex() )
        {
            CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "OnImpact: This is not the origination client, skipping OnExplode");            
        }
        else
#endif
        {
            CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "OnImpact: Calling OnExplode and destroying myself");            
            OnExplode();
        }
    }
}

//==============================================================================

void mutant_parasite_projectile::SetupType( void )
{
    if( m_CalledSetupType )
        return;

    m_CalledSetupType = TRUE;

    // Clean up and FX or audio in case they were already running
    if( m_FXFly.Validate() )
        m_FXFly.KillInstance();

    if( m_LaunchAudioID != -1 )
        g_AudioMgr.Release( m_LaunchAudioID, 0 );

    if( m_FlyAudioID != -1 )
        g_AudioMgr.Release( m_FlyAudioID, 0 );

    if( m_bContagious )
        m_TypeIndex = 1;

    // Fire up effects based on the contagious setting
    if (SMP_UTIL_InitFXFromString( s_ParasiteTweak[ m_TypeIndex ].m_MainBodyFXName, m_FXFly ))
    {
        m_FXFly.SetSuspended( TRUE );

        // set initial position
#ifdef X_EDITOR
        m_FXFly.SetTranslation( m_StartPos );
#else
        m_FXFly.SetTranslation( m_StartPos + net_GetBlendOffset() );
#endif
    }

    // Start up audio
    g_AudioMgr.Play( s_ParasiteTweak[ m_TypeIndex ].m_AudioLaunch, 
                     m_StartPos, GetZone1(), TRUE );

    m_FlyVoiceID = g_AudioMgr.Play( s_ParasiteTweak[ m_TypeIndex ].m_AudioFlying, 
                                    m_StartPos, GetZone1(), TRUE );    
}

//==============================================================================

void mutant_parasite_projectile::UpdateTarget( f32 DeltaTime )
{
    m_RetargetingTimer -= DeltaTime;
    if (m_RetargetingTimer > 0)
        return;

    // Only the origination machine should be calling this function, so it should
    // be ok to use the standard rand
    m_RetargetingTimer = x_frand( 0.25, 0.75f );

    xbool bFindNewTarget = FALSE;

    // Do we need to find a new target?
    if (NULL_GUID == m_Target)
    {
        bFindNewTarget = TRUE;
    }
    else    
    {
        // The guid is set at least
        object_ptr<object> pTarget( m_Target );
        if ( !pTarget.IsValid() )
        {
            // Target doesn't exist anymore
            bFindNewTarget = TRUE;
        }
        else
        {
            // Is it an actor?
            if (pTarget->IsKindOf( actor::GetRTTI() ))
            {
                object_ptr<actor> pActor( m_Target );
                if (pActor.IsValid())
                {
                    // Is it still alive?
                    if (!pActor->IsAlive())
                    {
                        // It's toast.
                        bFindNewTarget = TRUE;
                    }
                }
                else
                {
                    // This should never happen. (how could we have an object
                    // pointer that points to a non actor if the rtti matched?
                    bFindNewTarget = TRUE;
                }
            }
        }
    }

    if (bFindNewTarget)
    {
        // Grab a new target
        GetNewTarget();
    }
}

//==============================================================================

f32 mutant_parasite_projectile::ScorePotentialTarget( object* pObj )
{
    vector3 Delta = pObj->GetPosition() - GetPosition();
    vector3 Dir   = m_StartVel;
    f32     Score = -2;
    f32     Dist  = Delta.Length();

    Delta.Normalize();
    Dir.Normalize();

    f32 D = Dir.Dot(Delta);

    D = MAX(-1,MIN(1,D));    

    f32 Angle = x_acos( D );
    f32 AngleDEG = RAD_TO_DEG( Angle );
    f32 MaxAngle = s_TWEAK_ParasiteTargetingAngle.GetF32();

    if (AngleDEG < MaxAngle)
    {
        // To give the user a little bit of a break from the exacting
        // nature of this aiming mechanism, we need to put in a little
        // slop.  
        // 
        // Scale the dot product result so that values above a certain
        // threshold are all clamped to max.  This makes everything within
        // a cone of fixed radius an equally viable target.
        // After scoring the angle, distance will be factored in, and 
        // nearer targets with similar angle scores will be preferable.
        f32 DotThreshold = x_cos( R_5 );
        DotThreshold /= 2.0f;
        D = (MIN(1.0f,(D / 2.0f + 0.5f) / (1.0f - DotThreshold)) -0.5f) * 2;

        // Smooth out the return a little bit.
        // drop it on cos [0,PI] such that
        // D=  1 maps to Cos(  0 )
        // D=  0 maps to Cos(PI/2)
        // D= -1 maps to Cos( PI )
        Score = x_cos(((-D+1)/2)*PI);
        
        if (Score > 0.99f)
        {
            // Now, factor in distance
            f32 MaxRange = s_TWEAK_ParasiteTargetingRange.GetF32();
            f32 DistT = Dist / MaxRange;

            // We want 1 up close, and 0 far away
            DistT = 1.0f - DistT;
            
            Score += DistT;
        }
    }

    return Score;
}

//==============================================================================

void mutant_parasite_projectile::GetNewTarget( void )
{
    static const s32 MAX_POTENTIAL_TARGETS = 16;
    struct target_score
    {
        target_score( void ) { m_pObject = NULL; }
        object* m_pObject;
        f32     m_Score;
    };

    CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "GetNewTarget: Looking for target...");    

    bbox            BBox;
    s32             nPotentialTargets = 0;
    target_score    PotentialTargets[MAX_POTENTIAL_TARGETS];
    object*         pOwner          = g_ObjMgr.GetObjectByGuid( m_OriginGuid );
    actor*          pActorOwner     = NULL;
    s32             iBest           = -1;
    f32             BestScore       = -2;
    vector3         MyPos           = GetPosition();

    f32             ScanRadius = s_TWEAK_ParasiteTargetingRange.GetF32();

    m_Target = NULL_GUID;
       
    // Build targeting bbox
    BBox.Clear();
    BBox.Set( GetPosition(), ScanRadius );
    
    // Setup owner pointer
    if ( ( pOwner ) && ( pOwner->IsKindOf( actor::GetRTTI () ) ) )
    {
        pActorOwner = &(actor::GetSafeType( *pOwner ));
    }

    // Select targets from world
    g_ObjMgr.SelectBBox( ATTR_LIVING, BBox );

    // Loop
    slot_id SlotID = g_ObjMgr.StartLoop();
    while ( ( SlotID != SLOT_NULL ) && ( nPotentialTargets < MAX_POTENTIAL_TARGETS ) )
    {
        object* pObject         = g_ObjMgr.GetObjectBySlot(SlotID); 
        object_ptr<actor>  pActor(pObject);
        
        xbool IsAlive = TRUE;
        xbool IsEnemy = FALSE;

        if (pActor.IsValid())
        {
            IsAlive = !pActor->IsDead();
        }

        // Determine if this target is really an enemy
        if ( pActorOwner && pObject )
        {
            const guid ObjectGuid = pObject->GetGuid();
            const factions Faction = pActorOwner->GetFactionForGuid( ObjectGuid );
            IsEnemy = pActorOwner->IsEnemyFaction( Faction );
        }
        
        // If it is an enemy, score it and add it to the list
        if ( (pObject->GetGuid() != m_OriginGuid) && IsEnemy && IsAlive )
        {
            f32 Score = ScorePotentialTarget( pObject );
            // Add to our list of potential targets
            PotentialTargets[nPotentialTargets].m_pObject = pObject;
            PotentialTargets[nPotentialTargets].m_Score   = Score;
            nPotentialTargets++;
        }

        // Check next object
        SlotID = g_ObjMgr.GetNextResult( SlotID );
    }
    g_ObjMgr.EndLoop();

    // Choose from the list
    s32 i;
    for (i=0;i<nPotentialTargets;i++)
    {
        xbool bVisible = TRUE;

#ifdef REQUIRE_VISIBILITY_TO_TARGET
        if (k_REQUIRE_VISIBILITY_CHECK)
        {
            m_Target = PotentialTargets[i].m_pObject->GetGuid();
            xbool bIndirect = UpdateTargetPoint();
            m_Target = NULL_GUID;
            
            //g_CollisionMgr.LineOfSightSetup( GetGuid(), MyPos, PotentialTargets[i].m_pObject->GetPosition());
            g_CollisionMgr.LineOfSightSetup( GetGuid(), MyPos, m_TargetPoint );
            g_CollisionMgr.SetMaxCollisions( 1 );
            g_CollisionMgr.AddToIgnoreList( m_OriginGuid );            
            g_CollisionMgr.AddToIgnoreList( PotentialTargets[i].m_pObject->GetGuid() );
            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                                            object::ATTR_BLOCKS_SMALL_PROJECTILES);
            if (g_CollisionMgr.m_nCollisions != 0)
            {
                bVisible = FALSE;
            }
            else
            {   
                if (bIndirect)
                {
                    vector3 TargetPos = PotentialTargets[i].m_pObject->GetPosition();
                    object_ptr<actor>pActor(PotentialTargets[i].m_pObject->GetGuid());
                    if (pActor)            
                    {
                        TargetPos = pActor->GetPositionWithOffset( actor::OFFSET_AIM_AT );
                    }

                    // On an indirect target point, we need to do a second los test
                    g_CollisionMgr.LineOfSightSetup( GetGuid(), m_TargetPoint, TargetPos );
                    g_CollisionMgr.SetMaxCollisions( 1 );
                    g_CollisionMgr.AddToIgnoreList( m_OriginGuid );            
                    g_CollisionMgr.AddToIgnoreList( PotentialTargets[i].m_pObject->GetGuid() );
                    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                        object::ATTR_BLOCKS_SMALL_PROJECTILES);
                    if (g_CollisionMgr.m_nCollisions != 0)
                    {
                        bVisible = FALSE;
                    }
                }
            }
        }
#endif

        if (bVisible)
        {
            if (PotentialTargets[i].m_Score > BestScore)
            {
                iBest = i;
                BestScore = PotentialTargets[i].m_Score;
            }
        }
    }

    if (iBest != -1)
    {
        ASSERT( NULL != PotentialTargets[ iBest ].m_pObject );
        SetTarget( PotentialTargets[ iBest ].m_pObject->GetGuid() );
#ifndef X_EDITOR
        CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "GetNewTarget: Target found");
        ResyncGhosts();
#endif
    }
    else
    {
        CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "GetNewTarget: No target available");        
    }
}

//==============================================================================

xbool mutant_parasite_projectile::UpdateTargetPoint( void )
{
    f32 StrikeDistance = s_TWEAK_ParasiteStrikeDistance.GetF32();

    object_ptr<object> pObject( m_Target );
    if ( !pObject.IsValid() )
    {
        // Ack, no target
        vector3 Dir = m_Velocity;
        Dir.NormalizeAndScale( 10000.0f);
        m_TargetPoint = GetPosition() + Dir;
        return FALSE;
    }

    vector3 MyPos        = GetPosition();
    vector3 TargetObjPos = pObject->GetPosition();

    // At the very lease, let's set it to something close.
    m_TargetPoint = TargetObjPos;

    f32     DistToTarget = (MyPos - TargetObjPos).Length();
    
    if (pObject->IsKindOf( actor::GetRTTI() ))
    {
        object_ptr<actor>pActor(m_Target );
        ASSERT( pActor.IsValid() );
        if (pActor)    
            TargetObjPos = pActor->GetPositionWithOffset( actor::OFFSET_AIM_AT  );       
    }
    
    // If we are within strike distance, or are greater than 150cm above the target,
    // then use the TargetObjPos, otherwise, aim high
    if ((DistToTarget < StrikeDistance) || (MyPos.GetY() >= (TargetObjPos.GetY() + 25.0f)))
    {
        m_TargetPoint = TargetObjPos;
        return FALSE;
    }
    else
    {
        // Aim above
        vector3 FromTarget = GetPosition() - TargetObjPos;
        vector3 FromTargetFlat = FromTarget;
        FromTargetFlat.GetY() = 0;

        FromTargetFlat.NormalizeAndScale( 200 );
        FromTargetFlat.GetY() += 100.0f;

        m_TargetPoint = TargetObjPos + FromTargetFlat;
    }
    return TRUE;
}

//==============================================================================

void mutant_parasite_projectile::UpdateTrajectory( f32 DeltaTime )
{
    CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "UpdateTraj: Advancing [%f], Speed=%f, StartVel=(%f,%f,%f)",DeltaTime,m_ActualSpeed,m_StartVel.GetX(),m_StartVel.GetY(),m_StartVel.GetZ());
    ASSERT( m_StartVel.Length() > 1.0f );
    UpdateTargetPoint(); 
    ASSERT( m_StartVel.Length() > 1.0f );
    
    // Determine speed that parasite should be travelling
    m_DesiredSpeed = m_BaseSpeed;
    
    // Leave velocity direction unchanged if there is no target
    if ( NULL_GUID != m_Target )
    {           
        CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "UpdateTraj: I have a target");
        // Get deltas along current track, and desired to target
        vector3 CurrentTraj = m_StartVel;
        vector3 IdealTraj   = m_TargetPoint - GetPosition();
        f32     DistToTarget = IdealTraj.Length();

        // Normalize
        CurrentTraj.Normalize();
        IdealTraj.Normalize();

        // D is also used below.   D = dot between current trajectory and ideal trajectory
        //
        // D=  1 : On target!
        // D=  0 : Perp to target vector
        // D= -1 : Completely the wrong direction
        // 
        f32 D = IdealTraj.Dot(CurrentTraj);
            D = MAX(0.0f,MIN(1.0f,D));

        // Check to see if we are 
        f32 Angle = x_acos( D );
        if ( Angle <= k_TRAJECTORY_UPDATE_THRESHOLD )
        {
            // We are aimed at the target enough to be happy with the
            // current trajectory.
            return;
        }

        CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "UpdateTraj: D=%f",D);

        // Distort speed based on accuracy of the trajectory.
        f32 MaxSpeed = m_BaseSpeed;
        f32 MinSpeed = m_BaseSpeed * m_SlowestSpeedMultiplier;
        MinSpeed = MAX(MinSpeed, 200.0f);

        // Dot values below this will end up at the minimum speed.
        // Between this and 1.0, we ride a shaped curve
        f32 DotAtWhichToUseMinSpeed = 0.5f;

        f32 DClampedToRange = (D - DotAtWhichToUseMinSpeed) / (1.0f - DotAtWhichToUseMinSpeed);

        // DClampedToRange is still = 1 when aligned, and = 0 when sufficiently far off course
        // Use sin[PI,2PI] to soften the curve.  It has the characteristics we want
        f32 ShapedD = (x_sin( PI + (PI*DClampedToRange) ) / 2.0f) + 0.5f;
        
        m_DesiredSpeed = MinSpeed + (MaxSpeed - MinSpeed) * ShapedD;

        CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "UpdateTraj: DesiredSpeed=%f",m_DesiredSpeed);

        // Determine axis of rotation
        vector3 Axis;   
        radian JunkAngle;
        CurrentTraj.GetRotationTowards( IdealTraj, Axis, JunkAngle );
        
        // Figure out the max rotation speed.
        // At 150% StrikeDistance, rot speed is the default
        // At 50% StrikeDistance, rot speed is maxed out
        radian MinRotSpeed = m_MaxRotationSpeed;            // <<-- Min turn rate
        radian MaxRotSpeed = R_360 * 4;                     // <<-- Insane maximum turn rate  

        f32 StrikeDist = s_TWEAK_ParasiteStrikeDistance.GetF32();
        f32 MaxRotDist = StrikeDist * 1.5f;
        f32 MinRotDist = StrikeDist * 0.5f;

        f32 TDist = (DistToTarget - MinRotDist) / (MaxRotDist - MinRotDist);
        TDist = MAX(0.0f,MIN(1.0f,TDist));

        MaxRotSpeed = MinRotSpeed + (MaxRotSpeed - MinRotSpeed) * (1.0f - TDist);    

        // Bend the trajectory
        f32 MaxAngle = MaxRotSpeed * DeltaTime;
        ASSERT( MaxRotSpeed >= MinRotSpeed );
        
        Angle = MAX(-MaxAngle,MIN(MaxAngle,Angle));    

        vector3 OriginalVel = m_StartVel;
        vector3 DesiredVel = OriginalVel;    

        CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "UpdateTraj: Rotating %f degrees toward target",RAD_TO_DEG(Angle));

        quaternion Rot( Axis, Angle );
        if (DesiredVel.IsValid())
            DesiredVel = Rot.Rotate( DesiredVel );

        // Update tracking info from the netproj class
        // NOTE: This is a hack (obviously)
        m_StartPos = GetPosition();
        m_StartVel = DesiredVel;

        m_TimeT    = 0;
    }
    else
    {
        CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "UpdateTraj: I do not have a target, using fraction of base speed(%f)",m_BaseSpeed);
        // NO target
        m_DesiredSpeed = m_BaseSpeed;// * 0.7f;
    } 
    CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "UpdateTraj: desired speed(%f)",m_DesiredSpeed);

    ASSERT( m_StartVel.Length() > 1.0f );

    // Blend the actual speed toward the desired
    /*
    f32 MaxSpeedDelta = 600.0f * DeltaTime;
    f32 SpeedDelta = m_DesiredSpeed - m_ActualSpeed;
    SpeedDelta = MAX(-MaxSpeedDelta,MIN(MaxSpeedDelta,SpeedDelta));

    m_ActualSpeed += SpeedDelta;
    */

    m_ActualSpeed = m_DesiredSpeed;
    
    ASSERT( m_StartVel.Length() > 1.0f );
    m_StartVel.NormalizeAndScale( m_ActualSpeed );

    ASSERT( m_StartVel.IsValid() );
    ASSERT( m_StartVel.Length() > 1.0f );

    CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "UpdateTraj: Done. Speed=%f, StartVel=(%f,%f,%f)",m_ActualSpeed,m_StartVel.GetX(),m_StartVel.GetY(),m_StartVel.GetZ());
}

//==============================================================================

f32 mutant_parasite_projectile::GetWanderScale( void )
{
    f32 StartScale = 0;
    f32 EndScale   = 0;
    
    f32 DistFromTarget;
    
    // Closer than this distance, and we start bringing the wander
    // distance down.
    f32 DistFromTargetForMaxWander = 600.0f;


    if ((m_Target == NULL_GUID) || (!m_TargetPoint.IsValid()))
        DistFromTarget = DistFromTargetForMaxWander;
    else
        DistFromTarget = (m_TargetPoint - GetPosition()).Length();
    
    EndScale = DistFromTarget / DistFromTargetForMaxWander;
    
    EndScale = MAX(0,MIN(1,EndScale));

    // Parasite reaches max wander distance at this time from launch
    f32 TimeUntilMaxWnader = 1.5f;

    StartScale = m_Age / TimeUntilMaxWnader;
    StartScale = MAX(0,MIN(1,StartScale));

    //We use the minimum of the two wander scales;
    return MIN(StartScale,EndScale);
}

//==============================================================================

void mutant_parasite_projectile::CheckInternalCollision( f32 DeltaTime )
{
    // THIS IS AN ABRIDGED VERSION OF net_proj::OnAdvanceLogic

    // Age gracefully.
    m_Age   += DeltaTime;
    m_TimeT += DeltaTime;

    // Save off deltatime so we can consume it piece by piece
    m_DeltaTime = DeltaTime;

    // Although we haven't impacted yet, setting this to TRUE will simplify
    // the code needed to get us into the next loop.  m_Impact is set to FALSE
    // early within the loop.
    m_Impact = TRUE;    

    m_OldPos            = GetPosition();
    m_OldWanderPosition = m_WanderPosition;

    if ((GetAttrBits() & ATTR_DESTROY) )
        return;
   
    // Let's move!
    // Determine where we WANT to go.

    f32     NewT     = m_TimeT + m_DeltaTime;
    m_Velocity       = m_StartVel;

    // Now that we know where we want to go, attempt to move there.  If we 
    // do hit something, we will rebound and attempt to consume any 
    // remaining delta time.  If we hit too many times, we will just bail 
    // out.

    // Linear movement.
    m_NewPos   = m_StartPos + m_StartVel * NewT;

    // Layer wander on top of position
    vector3 Wander = GetWanderVector();

    m_WanderPosition = m_NewPos + Wander;
    ASSERT( m_WanderPosition.IsValid() );
    
    // Do collision using the "wander" position.  This is the point that is
    // swimming around the direction vector
    g_CollisionMgr.RaySetup( GetGuid(), m_OldWanderPosition, m_WanderPosition );
    g_CollisionMgr.SetMaxCollisions( 1 );
    g_CollisionMgr.AddToIgnoreList( m_OriginGuid );
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                                    object::ATTR_BLOCKS_SMALL_PROJECTILES);   

    // No collision?  Then bail out of this loop.
    if( g_CollisionMgr.m_nCollisions != 0 )
    {
            
        //
        // Backup collisions
        //
        s32 nCollisions = g_CollisionMgr.m_nCollisions;
        ASSERT( nCollisions <= MAX_COLLISION_MGR_COLLISIONS );
        if( nCollisions > MAX_COLLISION_MGR_COLLISIONS ) nCollisions = MAX_COLLISION_MGR_COLLISIONS;

        collision_mgr::collision CollBackup[MAX_COLLISION_MGR_COLLISIONS];
        x_memcpy( CollBackup, g_CollisionMgr.m_Collisions, sizeof( collision_mgr::collision )*nCollisions );

        // Process the collisions in order.
        for( s32 i=0; i<nCollisions; i++ )
        {
            collision_mgr::collision& Coll = CollBackup[i];

            // Skip over collisions with unidentifiable objects.
            if( Coll.ObjectHitGuid == 0 )
            {
                LOG_WARNING( (const char*)xfs("net_proj::OnAdvanceLogic %04d",net_GetSlot()),
                    "Collision with 'unidentifiable' object." );
                continue;
            }

            // Let's start interacting with the object we have hit.
            object* pObject = g_ObjMgr.GetObjectByGuid( Coll.ObjectHitGuid );
            if( !pObject )
            {
                LOG_WARNING( (const char*)xfs("net_proj::OnAdvanceLogic %04d",net_GetSlot()),
                    "Collision with 'unretrievable' object." );
                continue;
            }

            // Just go through cloth objects.
            if( pObject->IsKindOf( cloth_object::GetRTTI() ) )
            {
                // Notify the cloth of the impact.
                cloth_object* pClothObject = (cloth_object*)pObject;
                pClothObject->OnProjectileImpact( *this, 
                    m_Velocity,
                    Coll.PrimitiveKey, 
                    Coll.Point, FALSE, 40 );

                // Do NOT alter the trajectory as a result of the cloth.  
                // Doing so would screw up the networking.
                continue;                
            }

            // Just go through flag objects.
            if( pObject->IsKindOf( flag::GetRTTI() ) )
            {
                // Notify the cloth of the impact.
                flag* pFlag = (flag*)pObject;
                pFlag->OnProjectileImpact( *this, 
                    m_Velocity,
                    Coll.PrimitiveKey, 
                    Coll.Point, FALSE, 40 );

                // Do NOT alter the trajectory as a result of the cloth.  
                // Doing so would screw up the networking.
                continue;                
            }

            //
            // Is this a piece of glass
            //
            if( m_bThroughGlass && (Coll.Flags == object::MAT_TYPE_GLASS) )
            {
                //x_DebugMsg("PROJECTILE HIT GLASS!!! %d of %d\n",i,nCollisions);

                particle_emitter::CreateProjectileCollisionEffect( Coll, m_OriginGuid );

                pain Pain;
                Pain.Setup( m_PainHandle, m_OriginGuid, Coll.Point );
                Pain.SetDirection( m_Velocity );
                Pain.SetDirectHitGuid( pObject->GetGuid() );
                Pain.SetCollisionInfo( Coll );
                //Pain.SetCustomScalar( CalcDamageDegradation(Coll.Point) );
                Pain.ApplyToObject( pObject );

                // Don't stop processing other collisions.  We're going to
                // go right through the glass.
                continue;
            }

            // Just go through permeable objects.
            if( pObject->GetAttrBits() & object::ATTR_COLLISION_PERMEABLE )
            {
                pObject->OnColNotify( *this );
                continue;
            }

            // If we are here, then we have hit something worth a reaction.  The
            // object (which descends from this class and implements function
            // OnCollision) can react how it sees fit.
            OnImpact( Coll, pObject );

            // What are our options now?  And how does OnImpact signal this?
            //  + Object has come to a stop.
            //      - Set m_AtRest to TRUE.
            //      - Set m_Impact to TRUE to break out of this loop.
            //  + Object has blown up!
            //      - Set m_AtRest to TRUE.
            //      - Set m_Finished to TRUE.
            //      - Set m_RenderEffect as appropriate.
            //      - Set m_Impact to TRUE to break out of this loop.
            //  + Object has bounced.
            //      - Determine new starting point and velocity.
            //      - Leave m_AtRest at FALSE.
            //      - Set the DIRTY_START dirty bit.
            //      - Set m_Impact to TRUE to break out of this loop.
            //  + Object has decided to ignore this collision.
            //      - Leave m_Impact at FALSE to stay in this loop.

            if( m_Impact )
            {
                break;
                // This will break out of the 'for' loop and then iterate again
                // within the surrounding 'do/while' loop as long as we haven't
                // exceeded the iteration limit.
            }
        }
    }

    //
    // For better or worse, we have a new position.  Put the object there.
    //
/*
    CLOG_MESSAGE( ENABLE_LOGGING, 
        (const char*)xfs("net_proj::OnAdvanceLogic %04d",net_GetSlot()), 
        "TimeT:%4.2f - Position:(%4.2f,%4.2f,%4.2f) - Velocity:(%4.2f,%4.2f,%4.2f)", 
        m_TimeT,
        m_NewPos.GetX(),   m_NewPos.GetY(),   m_NewPos.GetZ(),
        m_Velocity.GetX(), m_Velocity.GetY(), m_Velocity.GetZ() );
*/
    // Don't move objects marked for destroy
    if( !(GetAttrBits() & object::ATTR_DESTROY) )
    {
        vector3 Delta = m_NewPos - GetPosition();

        OnMove( m_NewPos );

        // Orient along velocity
        radian3 Rot(0,0,0);
        
        if (Delta.LengthSquared() > 0)
            Delta.GetPitchYaw(Rot.Pitch,Rot.Yaw);
        m_FXFly.SetRotation( Rot );

    }
/*
    CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("net_proj::OnAdvanceLogic %04d",net_GetSlot()),
        "DONE StartVel (%f,%f,%f)",m_StartVel.GetX(), m_StartVel.GetY(), m_StartVel.GetZ() );        
*/

}

//==============================================================================

vector3 mutant_parasite_projectile::GetWanderVector( void )
{
    f32 Scale = GetWanderScale();

    f32 X = x_sin( (m_Age + m_WanderPhase) * 1.5f * k_WANDER_SPEED * m_WanderOffset ) * k_WANDER_RADIUS;
    f32 Y = x_cos( (m_Age + m_WanderPhase) * 1.8f * k_WANDER_SPEED * m_WanderOffset ) * k_WANDER_RADIUS;

    vector3 Wander(X,Y,0);
    Wander *= Scale;

    radian3 Rot(0,0,0);
    m_StartVel.GetPitchYaw( Rot.Pitch, Rot.Yaw );
    Wander.Rotate( Rot );

    ASSERT( Wander.IsValid() );

    //return vector3(0,0,0);

    return Wander;

}













//==============================================================================
#ifndef X_EDITOR
//==============================================================================

void mutant_parasite_projectile::ResyncGhosts( void )
{
    // Something major has changed.
    // Resync everyone
    CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "Resync Ghosts: Resyncing ghosts");    

    const matrix4& L2W = GetL2W();
    SetStart( GetPosition(), L2W.GetRotation(), m_StartVel, GetZone1(), GetZone2(), 0.0f );    
}

//==============================================================================

void mutant_parasite_projectile::net_AcceptActivate( const bitstream& BS )
{
    BS.ReadFlag     ( m_bContagious );
    BS.ReadRangedF32( m_WanderPhase,  8, 0.0f, 2*PI );
    BS.ReadRangedF32( m_WanderOffset, 8, 0.7f, 1.4f );
    BS.ReadRangedS32( m_OriginNetSlot, -1, 31 );  
//  BS.ReadF32( m_BaseSpeed );
    m_BaseSpeed = 1333.0f;      // It is always 1333, so don't net it.

    if( m_OriginNetSlot != -1 )
    {
        netobj* pNetObj = NetObjMgr.GetObjFromSlot( m_OriginNetSlot );
        if( pNetObj )
        {
            m_OriginGuid = pNetObj->GetGuid();
        }   
    }
}

//==============================================================================

void mutant_parasite_projectile::net_ProvideActivate( bitstream& BS )
{
    BS.WriteFlag     ( m_bContagious );
    BS.WriteRangedF32( m_WanderPhase,  8, 0.0f, 2*PI );
    BS.WriteRangedF32( m_WanderOffset, 8, 0.7f, 1.4f );
    BS.WriteRangedS32( m_OriginNetSlot, -1, 31 );  
//  BS.WriteF32( m_BaseSpeed );     // It is always 1333, so don't net it.
#ifdef X_DEBUG
    ASSERT( IN_RANGE( 1330.0f, m_BaseSpeed, 1335.0f ) );
#endif
}

//==============================================================================

void mutant_parasite_projectile::net_AcceptStart( const bitstream& BS )
{    
    s32 TargetNetSlot = -1;

    BS.ReadRangedS32( TargetNetSlot, -1, 31 );
       
    m_Target = NULL_GUID;
    if( TargetNetSlot != -1 )
    {
        netobj* pNetObj = NetObjMgr.GetObjFromSlot( TargetNetSlot );
        if( pNetObj )
        {
            m_Target = pNetObj->GetGuid();
            UpdateTargetPoint();
        }    
    }

//  BS.ReadF32( m_ActualSpeed );
//  BS.ReadF32( m_DesiredSpeed );
//  m_ActualSpeed  = 1333.0f;
//  m_DesiredSpeed = 1333.0f;

    m_WanderPosition    = m_StartPos;
    m_OldWanderPosition = m_StartPos;

#ifdef X_EDITOR
    m_FXFly.SetTranslation( m_StartPos );
#else
    m_FXFly.SetTranslation( m_StartPos + net_GetBlendOffset() );
#endif

    CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "net_AcceptStart:  Syncing start: TargetSlot [%d] Target [%s] ActSpeed(%f)  DesSpeed(%f)",TargetNetSlot,(const char*)guid_ToString(m_Target),m_ActualSpeed,m_DesiredSpeed );    
}

//==============================================================================

void mutant_parasite_projectile::net_ProvideStart( bitstream& BS )
{
    s32 TargetNetSlot = -1;
    
    object_ptr<netobj>pNetObj( m_Target );
    if( pNetObj )
    {
        TargetNetSlot = pNetObj->net_GetSlot();
    }

    // Position and velocity are taken care of by the net proj
    // We just need to send over any additional data.
    BS.WriteRangedS32( TargetNetSlot, -1, 31 );
//  BS.WriteF32( m_ActualSpeed );    
//  BS.WriteF32( m_DesiredSpeed );    
//  ASSERT( IN_RANGE( 1330.0f, m_ActualSpeed , 1335.0f ) );
//  ASSERT( IN_RANGE( 1330.0f, m_DesiredSpeed, 1335.0f ) );

    CLOG_MESSAGE( ENABLE_LOGGING, (const char*)xfs("mutant_parasite_projectile %04d",net_GetSlot() ), "net_ProvideStart:  Sending restart info: TargetSlot [%d] ActSpeed [%f]  DesSpeed [%f]",TargetNetSlot,m_ActualSpeed, m_DesiredSpeed);    
}

//==============================================================================
#endif // ifndef X_EDITOR
//==============================================================================

void mutant_parasite_projectile::SetStart( const vector3& Position,
                                           const radian3& Orientation,
                                           const vector3& Velocity,
                                                 s32      Zone1,
                                                 s32      Zone2,
                                                 f32      Gravity )
{
    net_proj::SetStart( Position, Orientation, Velocity, Zone1, Zone2, Gravity );
    SetupType();
}

//==============================================================================
