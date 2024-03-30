#include "debris_meson_lash.hpp"
#include "NetworkMgr\Networkmgr.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "..\Objects\Player.hpp"
#include "objects\ParticleEmiter.hpp"
#include "Tracers\TracerMgr.hpp"
#include "..\CollisionMgr\CollisionMgr.hpp"
#include "render\LightMgr.hpp"
#include "Decals\DecalMgr.hpp"
#include "Characters\GenericNPC\GenericNPC.hpp"
#include "Characters\MutantTank\Mutant_Tank.hpp"
#include "Objects\AlienGlob.hpp"
#include "Characters\ActorEffects.hpp"
#include "Objects\Corpse.hpp"
#include "Characters\Gray\Gray.hpp"
#include "Objects\AlienShield.hpp"
#include "Objects\Turret.hpp"

#define ENABLE_LOGGING 0
//#define DEEP_LOGGING 


//
//  NETWORK - tasks marked with //TODO:NET
//



#define REQUIRE_LINE_OF_SIGHT_TO_TARGET

#define MAX_INITIAL_TARGETS                             16
#define MP_DUMB_EXPLOSION_MAX_WAIT_FOR_TARGET_INFO      3.0f        // How long the dumb explosion's lashes
                                                                    // are willing to wait for targeting info
                                                                    // before retracting on their own

//=============================================================================

static tweak_handle     s_TweakMaxRadius            ( "Meson_Explosion_Radius" );
static tweak_handle     s_TweakExpansionSpeed       ( "Meson_Explosion_Speed"  );

//=============================================================================

static const xcolor     k_START_COLOR(255,255,255,255);
static       f32        k_TIME_BETWEEN_LASHES           = 0.15f;
static       f32        k_MIN_TIME_BETWEEN_BEAMS        = 0.05f;
static       f32        k_MAX_TIME_BETWEEN_BEAMS        = 0.25f;
static       f32        k_BEAM_LIFE_TIME                = 0.65f;

static const f32        k_LOS_VERIFICATION_SIDE_DIST    = 50.0f;    // How far 

static const s32        k_MAX_SPLINE_POINTS             = 30;
static const s32        k_MIN_SPLINE_POINTS             = 15;
static const f32        k_MIN_KILLING_STATE_TIME        = 3.0f;

static const s32        k_MAX_BEAM_POINTS               = 15;

//static const f32        k_MAX_TIME_CAN_EXIST            = 8.0f;


debris_meson_explosion::target_behaviour debris_meson_explosion::m_TargetBehaviour[ TARGET_TYPE_MAX ] = {
//     EXTEND     HOLD  RETRACT
//    MIN   MAX   
    { 0.2f, 0.5f, 1.2f,  0.5f  },            // ACTOR
    { 0.2f, 0.5f, 0.25f, 0.5f  },            // NONACTOR - DAMAGEABLE
    { 0.2f, 0.5f, 0.25f, 0.5f  },            // NONACTOR - NONDAMAGEABLE
    { 0.2f, 0.4f, 0.1f,  0.25f },            // GLOB
    { 0.4f, 0.7f, 2.0f,  0.5f  },            // SHIELD
    { 0.4f, 0.7f, 2.6f,  0.5f  } };          // THETA


//  Sound names
static const char* s_pSoundNames[debris_meson_explosion::SOUND_MAX] = { "MSN_Explode_Loop",         // EXPLODE
                                                                        "MSN_Implode",              // IMPLODE

                                                                        "MSN_TendrilExtend_Loop",   // EXTEND
                                                                        "MSN_TendrilExtend_End",
                                                                        "MSN_TendrilGrab_Loop",     // HOLD
                                                                        "MSN_TendrilGrab_End",
                                                                        "MSN_TendrilRetract_Loop",  // RETRACT
                                                                        "MSN_TendrilRetract_End" }; 
                                                

static f32 s_LashSizes[] = { 5.0f, 8.0f, 4.0f };

//=============================================================================
// OBJECT DESC.
//=============================================================================
static struct debris_meson_explosion_desc : public object_desc
{
    debris_meson_explosion_desc( void ) : object_desc( 
        object::TYPE_DEBRIS_MESON_EXPLOSION, 
        "MesonCannonExplosionsDebris", 
        "EFFECTS",
        object::ATTR_NEEDS_LOGIC_TIME   |
        object::ATTR_RENDERABLE         | 
        object::ATTR_TRANSPARENT        |
        object::ATTR_NO_RUNTIME_SAVE,
        FLAGS_IS_DYNAMIC  )
    {}

    virtual object* Create          ( void )
    {
        return new debris_meson_explosion;
    }

} s_debris_meson_explosion_desc;

//=============================================================================================

const object_desc&  debris_meson_explosion::GetTypeDesc ( void ) const
{
    return s_debris_meson_explosion_desc;
}

//=============================================================================================

const object_desc&  debris_meson_explosion::GetObjectType ( void )
{
    return s_debris_meson_explosion_desc;
}

//=============================================================================

#define DME_COLOR_CHANGE(VC,CL) if (d##CL > 0) m_PrecisionColor.Get##VC() += MIN(d##CL,MaxColorChange); else if (d##CL < 0) m_PrecisionColor.Get##VC() += MAX(d##CL,-MaxColorChange);

void debris_meson_explosion::lash::AdvanceLogic( f32 DeltaTime )
{
    f32 dR = m_DesiredColor.R - m_PrecisionColor.GetX();
    f32 dG = m_DesiredColor.G - m_PrecisionColor.GetY();
    f32 dB = m_DesiredColor.B - m_PrecisionColor.GetZ();
    f32 dA = m_DesiredColor.A - m_PrecisionColor.GetW();

    f32 MaxColorChange = 255.0f / 30.0f;

    DME_COLOR_CHANGE(X,R)        
    DME_COLOR_CHANGE(Y,G)
    DME_COLOR_CHANGE(Z,B)
    DME_COLOR_CHANGE(W,A)

    m_Color.R = (u8)(m_PrecisionColor.GetX());
    m_Color.G = (u8)(m_PrecisionColor.GetY());
    m_Color.B = (u8)(m_PrecisionColor.GetZ());
    m_Color.A = (u8)(m_PrecisionColor.GetW());

    f32 MaxSizeChange = 30.0f / 30.0f;

    f32 dSize = m_DesiredSize - m_Size;
    if (dSize > 0)
        m_Size += MIN(dSize, MaxSizeChange);
    else if (dSize < 0)
        m_Size += MAX(dSize, -MaxSizeChange);

    switch( m_State )
    {
    case LASH_EXTEND:
        break;
    case LASH_INTERACT:
        ShakeCorpse( DeltaTime );
        break;
    case LASH_RETRACT:
        break;
    }
}

//=============================================================================

void debris_meson_explosion::lash::ShakeCorpse( f32 DeltaTime )
{
    (void)DeltaTime;

    if ( m_TargetType == TARGET_SHIELD )
    {
        object_ptr<alien_shield>pShield( m_TargetObject );
        if (pShield)
        {
            pShield->PlayPainFX( m_TargetPoint );
        }
        return;
    }
    if ( m_TargetType == TARGET_THETA )
    {
        return;
    }

    corpse *pCorpse = (corpse*)g_ObjMgr.GetObjectByGuid(m_CorpseGuid);
    if (NULL == pCorpse)
        return;

    if (m_iConstraint < 0)
        return;

    // Lookup physics instance and geometry
    physics_inst& PhysicsInst = pCorpse->GetPhysicsInst();

    // Keep all the corpse rigid bodies awake
    PhysicsInst.Activate();

    // Lookup constraint and update position
    m_ConstraintPosition = m_BaseConstraintPosition;

    f32 ShakeAmt = 15.0f;
    {
        f32 x = x_frand(-ShakeAmt,ShakeAmt);
        f32 y = x_frand(-ShakeAmt,ShakeAmt);
        f32 z = x_frand(-ShakeAmt,ShakeAmt);
        m_ConstraintPosition += vector3( x, y, z );
    }

    // Update constraint world position
    if (PhysicsInst.GetNBodyWorldConstraints() > m_iConstraint)
        PhysicsInst.SetBodyWorldConstraintWorldPos( m_iConstraint, m_ConstraintPosition );
}

//=============================================================================

void debris_meson_explosion::lash::ReleaseCorpse( void )
{
    corpse *pCorpse = (corpse*)g_ObjMgr.GetObjectByGuid(m_CorpseGuid);
    if (NULL == pCorpse)
        return;

    // Lookup physics instance and geometry
    physics_inst& PhysicsInst = pCorpse->GetPhysicsInst();
    PhysicsInst.ApplyBlast( m_BaseConstraintPosition, -m_PainDirection, 150.0f, 350.0f );
    PhysicsInst.DeleteAllBodyWorldConstraints();

    pCorpse->SetPermanent( FALSE );
    pCorpse->StartFading( 0.25f );

    m_iConstraint = -1;
    m_CorpseGuid = 0;   
}

//=============================================================================

xbool debris_meson_explosion::lash::AcquireCorpse( void )
{
    m_CorpseGuid    = NULL_GUID;
    m_iConstraint   = -1;

    CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::AcquireCorpse","%08X Trying to get a corpse",this);

    if (m_TargetType != TARGET_ACTOR)
        return FALSE;

    // Skip creating a ragdoll if the max limit has already been reached
    if( corpse::ReachedMaxActiveLimit() )
        return FALSE;              

    object_ptr<actor>pActor( m_TargetObject );
    if (!pActor.IsValid())
        return FALSE;

    // If the actor doesn't have any rigid bodies, just treat it like
    // a normal object that can be harmed instead.
    const geom* pActorGeom = pActor->GetGeomPtr();

    // If this is the player, use the third person skin geom and not the player hands!
    if( pActor->GetType() == object::TYPE_PLAYER )
    {
        pActorGeom = player::GetSafeType( *pActor ).GetThirdPersonInst().GetGeom();
    }

    ASSERT( pActorGeom );
    if (pActorGeom->m_nRigidBodies == 0)
        return FALSE;

    // Corpsify him and do the constraint setup
    guid    gCorpse = pActor->GetCorpseGuid();
    corpse *pCorpse = NULL;
    {
        object* pCorpseObject = NULL;
        if (gCorpse)
        {
            CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::AcquireCorpse","%08X Actor already had a corpse guid",this);

            pCorpseObject = g_ObjMgr.GetObjectByGuid( gCorpse );    
            if (pCorpseObject)
            {
                if (pCorpseObject->IsKindOf( corpse::GetRTTI() ))
                {
                    CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::AcquireCorpse","%08X Actor corpse guid was valid",this);
                    pCorpse = (corpse*)pCorpseObject;
                }
                else
                {
                    CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::AcquireCorpse","%08X Actor corpse guid was not valid",this);
                }
            }
            else
            {
                CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::AcquireCorpse","%08X Actor guid did not resolve to an object",this);
            }
        }
    }

    if (NULL == pCorpse)
    {
        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::AcquireCorpse","%08X Asking actor to create a corpse",this);
        pCorpse = pActor->CreateCorpseObject(TRUE);
    }

    
    if (pCorpse)
    {
        m_CorpseGuid = pCorpse->GetGuid(); 

        loco* pActorLoco = pActor->GetLocoPointer();
        ASSERT( pActorLoco );

        s32 iBone = -1; 
        if (iBone == -1)
            iBone = pActorLoco->m_Player.GetBoneIndex( "B_01_Spine02" );
        if (iBone == -1)
            iBone = pActorLoco->m_Player.GetBoneIndex( "B_01_Spine01" );
        if (iBone == -1)
            iBone = pActorLoco->m_Player.GetBoneIndex( "B_01_Neck" );

        if (iBone != -1)
        {
            // set constraint position
            m_BaseConstraintPosition = pActor->GetBonePos(iBone);
            m_ConstraintPosition = m_BaseConstraintPosition;
            m_BaseConstraintPosition += vector3(0,50,0);

            // create pin constraint

            // Lookup physics instance and geometry
            physics_inst& PhysicsInst = pCorpse->GetPhysicsInst();
            geom* pGeom = PhysicsInst.GetSkinInst().GetGeom();
            ASSERT( pGeom );

            // Get rigid body that bone is attached to        
            ASSERT( iBone < pGeom->m_nBones );
            s32 iRigidBody = pGeom->m_pBone[ iBone ].iRigidBody;
            ASSERT( iRigidBody != -1 );

            // Add constraint and store the index for later
            m_iConstraint = PhysicsInst.AddBodyWorldConstraint( iRigidBody, // Your rigid body
                m_ConstraintPosition,   // Where in the world you want the constraint
                0.0f );           // Set max dist to zero to keep it pinned exactly
            ASSERT( m_iConstraint != -1 );  

            PhysicsInst.ApplyBlast( m_BaseConstraintPosition, m_PainDirection, 150.0f, 150.0f );
        }

        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::AcquireCorpse","%08X Corpse acquired",this);

        return TRUE;
    }

    CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::AcquireCorpse","%08X Failed to acquire a corpse",this);

    return FALSE;
}

//=============================================================================

xbool debris_meson_explosion::lash::HarmTarget( void )
{
    CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::HarmTarget","%08X Trying to harm target",this);

    // Bail for obvious reasons
    if (m_TargetObject == NULL_GUID)
        return FALSE;
    if (!m_bActive)
        return FALSE;    

    xbool bDishOutPain = FALSE;

    object* pObj = g_ObjMgr.GetObjectByGuid( m_TargetObject );
    if (NULL == pObj)
        return FALSE;

    if ((m_TargetType == TARGET_GLOB) && m_pOwner )
    {
        // We need to have feedback from globs about their demon spawn
        alien_glob& AG = alien_glob::GetSafeType( *pObj );
        AG.NotifyOnSpawn( m_pOwner->GetGuid() );
    }

    // Keep in mind, when networked, only the smart explosion
    // can cause pain.    
#ifndef X_EDITOR
    if (GameMgr.IsGameMultiplayer())
    {
        if (m_pOwner->net_GetOwningClient() != g_NetworkMgr.GetClientIndex())
        {
            return FALSE;
        }
    }
#endif

    xbool bTargetWasActorAndItDied = FALSE;

    switch (m_TargetType)
    {
    case TARGET_ACTOR:        
        {
            CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::HarmTarget","%08x Harming an actor",this);
            
            bDishOutPain = FALSE;

            // Get actor
            actor& Actor = actor::GetSafeType( *pObj );                                  
            xbool bDoActorEffect = TRUE;    
            
            if (bDoActorEffect)
            {
                actor_effects* pEffects = Actor.GetActorEffects( TRUE );
                if (pEffects)
                {
                    pEffects->InitEffect( actor_effects::FX_SHOCK, &Actor );
                    pEffects->SetShockTimer( 2.0f );
                }
            }

#ifndef X_EDITOR
            if (GameMgr.IsGameMultiplayer())
            {
                CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::HarmTarget","%08X Doing multiplayer harm via pain",this);

                // OnKill on a player seems to royally screw up splitscreen mode.
                // Trying lethal pain for now.
                pain_handle PainHandle("PLAYER_MESON_PRIMARY");
                m_PainDirection.Normalize();

                pain Pain;
                Pain.Setup( PainHandle, m_pOwner->GetGuidOfOwner(), m_TargetPoint );
                Pain.SetDirection( m_PainDirection );                
                Pain.SetDirectHitGuid( m_TargetObject );

                // Smack the target
                Pain.ApplyToObject( m_TargetObject );  
            }
            else
#endif
            {
                // In campaign, we are going to continue to use the OnKill.
                // We noticed trouble in The Ascension when killing blackops with
                // the meson cannon.  We have seen, rarely, cases where the physics
                // manager gets a whacko bounding box passed in.
                // The belief is that we are pinning the corpse in this function,
                // but the "Pain.ApplyToObject" path doesn't kill the npc immediatly.
                // The NPC exists through to the next frame, accepts the pain,
                // and possibly moves the corpse to another position determined
                // by an animation or some such.  This new position coupled with the
                // previously constrained bone causes the corpse to whiplash and
                // explode into a horrific mess.  That's the theory anyway.
                Actor.OnKill();
            }

            if (Actor.IsDead())
            {
                CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::HarmTarget","%08X Actor died form pain",this);
                bTargetWasActorAndItDied = TRUE;
            }
            else
            {
                CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::HarmTarget","%08X Actor survived the pain",this);
            }

            CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::HarmTarget","%08X Done harming actor",this);
        }   
        break;
    case TARGET_NONACTOR_DAMAGEABLE:
        {
            bDishOutPain = TRUE;
        }
        break;
    case TARGET_NONACTOR_NONDAMAGEABLE:
        {
            bDishOutPain = FALSE;
        }
        break;
    case TARGET_GLOB:
        {
            bDishOutPain = TRUE;
        }
        break;
    case TARGET_SHIELD:
        {
            bDishOutPain = TRUE;
        }
        break;
    case TARGET_THETA:
        {
            if (!m_bThetaHarmed)
            {
                bDishOutPain = TRUE;
            }

            /*
            if (!m_bThetaHarmed)
            {
                object_ptr<mutant_tank>pTheta( m_TargetObject );
                if (pTheta.IsValid())
                {
                    pTheta->EnableStun( m_TargetBehaviour[ m_TargetType ].InteractTime );
                }
                
                m_bThetaHarmed = TRUE;
            }
            */
        }
        break;
    }
    
    if (bDishOutPain)
    {
        // Build pain
        pain        Pain; 
        pain_handle PainHandle("PLAYER_MESON_PRIMARY");
        
        ASSERT( m_pOwner );

        m_PainDirection.Normalize();

        Pain.Setup( PainHandle, m_pOwner->GetGuidOfOwner(), m_TargetPoint );
        Pain.SetDirection( -m_PainDirection );        
        Pain.SetDirectHitGuid( m_TargetObject );

        // Smack the target
        Pain.ApplyToObject( m_TargetObject );            
    }

    if (m_TargetType == TARGET_THETA)
    {
        if (!m_bThetaHarmed && m_bStunTheta)
        {
            object_ptr<mutant_tank>pTheta( m_TargetObject );
            if (pTheta.IsValid())
            {
                pTheta->EnableStun( m_TargetBehaviour[ m_TargetType ].InteractTime );
            }
            bDishOutPain = TRUE;
            m_bThetaHarmed = TRUE;
        }
    }

    return bTargetWasActorAndItDied;
}

//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================

debris_meson_explosion::debris_meson_explosion()
{
    m_TotalTime             = 0;
    m_nLashTargets          = 0;
    m_iCurLash              = 0;
    m_iNextTarget           = 0;
    m_iNetCurrentTarget     = 0;
    m_BeamTimer             = 0.25f;
    m_State                 = STATE_VERIFYING_TARGETS;
    m_iNextToBeValidated    = 0;
    m_bDestroying           = FALSE;
    m_MainExplosionVoiceID  = -1; 

    s32 i;
    for (i=0;i<MAX_LASHES;i++)
    {
        m_Lash[i].m_pOwner = this;
    }

    for (i=0;i<MAX_BEAMS;i++)
    {
        m_Beam[i].m_Timer = 0;
    }


#ifndef X_EDITOR
    m_bNetCollapseSignaled          = FALSE;
    m_bActivated                    = FALSE;
    m_NetTargetStatusCounter        = 0;
    m_NetKilledMask                 = 0;
    m_bNetDumbTargetStatusSignaled  = FALSE;
#endif

    CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::constructor","%08X Constructor called",this);
    LOG_FLUSH();
}

//=============================================================================

debris_meson_explosion::~debris_meson_explosion()
{    
    if (m_CoreFancyFX.Validate())
        m_CoreFancyFX.KillInstance();

    if (m_MainExplosionFX.Validate( ))
        m_MainExplosionFX.KillInstance();

    if (m_MainExplosionVoiceID != -1)
        g_AudioMgr.Release( m_MainExplosionVoiceID, 0 );

    CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::destructor", "Destructor called" );
    LOG_FLUSH();
}

//=============================================================================

void debris_meson_explosion::OnInit              ( void )
{
    object::OnInit();

    m_LocalBBox.Set(vector3(0,0,0), 50 );


    // These audio packages are not preloaded here for now.
    // Marc says there is a preloading related problem, and instead, he is 
    // going to use dummy sound emitters in the levels to get the audio
    // "officially" loaded in the level.
    //
    // Explosion_Main.audiopkg
    // Explosion_Powder.audiopkg
    // CharMarks.decalpkg;

    m_MaxRadius = MAX( s_TweakMaxRadius.GetF32(), 100.0f );
    m_Texture.SetName( PRELOAD_FILE("Tracer_Lightning.xbmp") );
    m_hDecalPackage.SetName( PRELOAD_FILE( "BulletHoles.decalpkg" ) );
    
}

//=============================================================================

void debris_meson_explosion::GatherTargets( u32 Attribute, const bbox& WorldBBox, object::type ObjType )
{
    // Setup max target count
    s32 MaxTargets = MAX_LASH_TARGETS;
#ifndef X_EDITOR
    if (GameMgr.IsGameMultiplayer())    
        MaxTargets = 8;    
#endif

    if (m_nLashTargets >= MaxTargets)
        return;

    g_ObjMgr.SelectBBox( Attribute, WorldBBox, ObjType );

    pain        Pain; 
    pain_handle PainHandle("PLAYER_MESON_PRIMARY");
    Pain.Setup( PainHandle, m_GuidOfOwner, GetPosition() );

    // Gather targets
    slot_id SlotID = g_ObjMgr.StartLoop();
    while ((SlotID != SLOT_NULL) && (m_nLashTargets < MAX_INITIAL_TARGETS))
    {
        object* pObj = g_ObjMgr.GetObjectBySlot( SlotID );
        if (pObj)
        {
            lash_target& T = m_LashTargets[ m_nLashTargets ];
            xbool bSkip = FALSE;

            T.gObj              = pObj->GetGuid();
            T.bIsValidTarget    = FALSE;
            T.TargetType        = TARGET_NONACTOR_DAMAGEABLE;
            T.ThetaBoneIndex    = 0;
            T.NetActorLifeSeq   = 0;

            // Make sure we don't duplicate
            s32 i;
            for (i=0;i<m_nLashTargets;i++)
            {
                if (m_LashTargets[i].gObj == T.gObj)
                {
                    bSkip = TRUE;
                    break;
                }                
            }

            // If we are in a network game, skip any objects
            // that aren't network aware already
#ifndef X_EDITOR
            if( GameMgr.IsGameMultiplayer() )
            {
                if (!pObj->IsKindOf( netobj::GetRTTI() ))
                    bSkip = TRUE;
            }
#endif // X_EDITOR

            
            //There has been a change of opinion here.
            //The firing player is a valid target.  Feel free to
            //bite his hand clean off...
 
            // SH: 16-Dec ... Turns out we won't kill the player in campaign

            // Don't bite the hand that creates us.
#ifndef X_EDITOR
            if (!GameMgr.IsGameMultiplayer())
#endif
            {
                if (T.gObj == m_GuidOfOwner)
                {
                    bSkip = TRUE;
                }

            }
            

            if (!bSkip)
            {
                if (pObj->IsKindOf( alien_glob::GetRTTI() ))
                {
                    T.TargetType = TARGET_GLOB;
                }
                else
                if (pObj->IsKindOf( alien_shield::GetRTTI() ))
                {
                    T.TargetType = TARGET_SHIELD;
                }
                else
                // Grays need to be examined to determine
                // if they are shielded
                if (pObj->IsKindOf( gray::GetRTTI() ))
                {
                    gray& Gray = gray::GetSafeType( *pObj );  
                    if (Gray.IsShielded())
                    {
                        // Skip him.  The shield contains the gray, so the shield
                        // will be picked up as a target also.
                        bSkip = TRUE;                        
                    }
                    else
                    {
                        T.TargetType = TARGET_ACTOR;
                    }
                }
                else
                if (pObj->IsKindOf( mutant_tank::GetRTTI() ))
                {
                    object_ptr<mutant_tank>pTheta(pObj->GetGuid());
                    if (!pTheta.IsValid())
                    {
                        bSkip = TRUE;
                    }
                    else if (!pTheta->IsAlive() || !pTheta->GetCanDie())
                    {
                        bSkip = TRUE;
                    }
                    else
                    {
                        // Add primary target
                        T.TargetType = TARGET_THETA;
                        T.ThetaBoneIndex = -1;
                        T.bStunTheta     = TRUE;
                        
                        lash_target& T2 = m_LashTargets[ m_nLashTargets+1 ];
                        lash_target& T3 = m_LashTargets[ m_nLashTargets+2 ];

                        m_nLashTargets+=2;

                        // Get bone indices for secondary targets
                        s32 iBone[2] = {0};
                        T2.gObj = pObj->GetGuid();
                        
                        if (pTheta.IsValid())
                        {
                            anim_group::handle* hAnimGroup = pTheta->GetAnimGroupHandlePtr();                        
                            const anim_group *pAnimGroup   = hAnimGroup->GetPointer();
                            ASSERT(pAnimGroup);
                            if( pAnimGroup )
                            {
                                if (x_frand(0,1) > 0.5f)
                                    iBone[0] = pAnimGroup->GetBoneIndex( "B_01_Leg_R_Calf02" );
                                else
                                    iBone[0] = pAnimGroup->GetBoneIndex( "B_01_Arm_R_Forearm01" );
                                    
                                if (x_frand(0,1) > 0.5f)
                                    iBone[1] = pAnimGroup->GetBoneIndex( "B_01_Leg_L_Calf02" );
                                else
                                    iBone[1] = pAnimGroup->GetBoneIndex( "B_02_Arm_L_Hand" );
                            }
                        }

                        // Add secondary target
                        T2.gObj              = pObj->GetGuid();
                        T2.bIsValidTarget    = FALSE;
                        T2.TargetType        = TARGET_THETA;
                        T2.ThetaBoneIndex    = iBone[0];
                        T2.bStunTheta        = FALSE;
                        
                        T3.gObj              = pObj->GetGuid();
                        T3.bIsValidTarget    = FALSE;
                        T3.TargetType        = TARGET_THETA;
                        T3.ThetaBoneIndex    = iBone[1];                                       
                        T3.bStunTheta        = FALSE;
                    }
                }
                else
                // Just inhale normal actors
                if (pObj->IsKindOf( actor::GetRTTI() ))
                {
                    actor& Actor = actor::GetSafeType( *pObj );
                    if (!Actor.GetCanDie() || Actor.IgnorePain(Pain) || !Actor.IsAlive() || Actor.HasSpawnInvulnerability() )
                    {
                        bSkip = TRUE;
                    }                
                    T.TargetType = TARGET_ACTOR;
            #ifndef X_EDITOR
                    T.NetActorLifeSeq = Actor.net_GetLifeSeq();
            #else               
                    T.NetActorLifeSeq = -1;
            #endif
                }
                else
                if (pObj->IsKindOf( turret::GetRTTI() ))
                {                    
                    T.TargetType = TARGET_NONACTOR_DAMAGEABLE;
                }                
                else
                {
                    // Everything else is here.                    
                    // Only grab 50% of them
                    if (x_frand(0,1) < 0.5f)
                    {
                        bSkip = TRUE;
                    }
                    else
                    {
                        T.TargetType = TARGET_NONACTOR_DAMAGEABLE;                        
                    }
                }   

                // Do we keep it?
                if (!bSkip)
                {
                    m_nLashTargets++;
                    if (m_nLashTargets >= MAX_LASH_TARGETS)
                        break;
                }
            }
        }
        SlotID = g_ObjMgr.GetNextResult( SlotID );
    }
    g_ObjMgr.EndLoop();
}

//=============================================================================

static
s32 CompareLashTargets( const void* pA, const void* pB )
{
    const debris_meson_explosion::lash_target* A = (debris_meson_explosion::lash_target*)pA;
    const debris_meson_explosion::lash_target* B = (debris_meson_explosion::lash_target*)pB;

    // These priorities match the target type enum in the header
    static s32 TargetTypePriority[] = { 20,         //      TARGET_ACTOR
                                        40,         //      TARGET_NONACTOR_DAMAGEABLE
                                        100,        //      TARGET_NONACTOR_NONDAMAGEABLE
                                        0,          //      TARGET_GLOB
                                        30,         //      TARGET_SHIELD
                                        25,         //      TARGET_THETA
                                        15 };       //      TARGET_NETGHOST

    return TargetTypePriority[ A->TargetType ] - TargetTypePriority[ B->TargetType ];
}


void debris_meson_explosion::SortTargets( void )
{
    x_qsort( m_LashTargets, m_nLashTargets, sizeof(lash_target), CompareLashTargets );
}

//=============================================================================
//  
//  
//  
//=============================================================================
void debris_meson_explosion::DumbCreate( const vector3& PosWithOffset, const vector3& Normal, u32 Zones )
{
#ifdef DEEP_LOGGING
    CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::DumbCreate","");
    LOG_FLUSH();
#endif

    SetZones( Zones );
    OnMove( PosWithOffset );

    m_Origin       = PosWithOffset;
    m_OriginNormal = Normal;
    m_NetSlotOfOwner = -1;

    m_nLashTargets = 0;
    m_LashSpawnTimer = 0.0f;

    // Load the explosion core effect
    if (m_CoreFancyFX.Validate())
        m_CoreFancyFX.KillInstance();
    rhandle<char> FxResource;    
    FxResource.SetName( PRELOAD_FILE("MSN_Suction.fxo") );
    const char* pScrewedUpName = FxResource.GetPointer();
    if ( pScrewedUpName )
    {
        m_CoreFancyFX.InitInstance( pScrewedUpName );
        m_CoreFancyFX.SetSuspended( FALSE );
        m_CoreFancyFX.SetTranslation( m_Origin );        
        f32 S = 1.0f;
        m_CoreFancyFX.SetScale( vector3(S,S,S) );        
    }

    if (m_MainExplosionFX.Validate())
        m_MainExplosionFX.KillInstance();
    FxResource.SetName( PRELOAD_FILE("MSN_explode_electr.fxo") );
    pScrewedUpName = FxResource.GetPointer();
    if ( pScrewedUpName )
    {
        m_MainExplosionFX.InitInstance( pScrewedUpName );
        m_MainExplosionFX.SetSuspended( FALSE );
        m_MainExplosionFX.SetTranslation( m_Origin );        
        f32 S = 1.0f;
        m_MainExplosionFX.SetScale( vector3(S,S,S) );        
    }

    SwitchState( STATE_VERIFYING_TARGETS );

    if (m_MainExplosionVoiceID != -1)
        g_AudioMgr.Release( m_MainExplosionVoiceID, 0 );

    m_MainExplosionVoiceID = g_AudioMgr.Play( s_pSoundNames[ SOUND_MAIN_EXPLODE ], m_Origin, GetZone1(), TRUE );
}

void debris_meson_explosion::Create( const vector3& Pos, const vector3& Normal, u32 Zones, guid OwnerGuid )                                         
{    
#ifdef DEEP_LOGGING
    CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::Create","");
    LOG_FLUSH();
#endif
    SetZones( Zones );
    OnMove( Pos );

    bbox Box;
    Box.Set( GetPosition(), m_MaxRadius );

    m_Origin       = Pos;
    m_OriginNormal = Normal;
    m_GuidOfOwner  = OwnerGuid;
    m_NetSlotOfOwner = -1;

#ifndef X_EDITOR
    if( GameMgr.GetGameType() != GAME_CAMPAIGN )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( OwnerGuid );
        if (pObj)
        {
            m_NetSlotOfOwner = pObj->net_GetSlot();
        }
    }
#endif // X_EDITOR

    vector3 OffsetDelta = Normal;
    OffsetDelta.NormalizeAndScale( 50.0f );

    // Move away from the point of impact
    g_CollisionMgr.RaySetup( GetGuid(), m_Origin, m_Origin + OffsetDelta );
    g_CollisionMgr.SetMaxCollisions(1);            
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                                    object::ATTR_COLLIDABLE,
                                   (object::object_attr)(object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING));
    if (g_CollisionMgr.m_nCollisions > 0)
    {
        OffsetDelta *= (g_CollisionMgr.m_Collisions[0].T * 0.75f);
    }

    m_Origin += OffsetDelta;

    m_nLashTargets = 0;
    m_LashSpawnTimer = 0.0f;

    DumbCreate( m_Origin, Normal, Zones );

    // Gather targets in two passes.  We want to get ALL of the living targets in first.
    // If there is room left over, the rest of the damageables can be added.

    GatherTargets( object::ATTR_LIVING, Box, object::TYPE_ALL_TYPES );
    GatherTargets( object::ATTR_DAMAGEABLE, Box, object::TYPE_ALL_TYPES );

    SortTargets();


    // SANITY    
#ifdef X_DEBUG
    s32 i;
    for (i=0;i<m_nLashTargets;i++)
    {
        s32 j;
        for (j=0;j<m_nLashTargets;j++)
        {
            if (i==j)
                continue;
            ASSERT( ( m_LashTargets[i].gObj != m_LashTargets[j].gObj ) || (m_LashTargets[i].TargetType == TARGET_THETA) );
        }
    }
#endif

}

//=============================================================================

bbox debris_meson_explosion::GetLocalBBox        ( void ) const
{
    return m_LocalBBox;
}

//=============================================================================

void debris_meson_explosion::SwitchState( explosion_state State )
{
#ifdef DEEP_LOGGING
    const char* From;
    const char* To;
    switch(m_State)
    {
        case STATE_VERIFYING_TARGETS:
            From="STATE_VERIFYING_TARGETS";
            break;
        case STATE_KILLING_TARGETS:
            From="STATE_KILLING_TARGETS";
            break;
        case STATE_COLLAPSING:
            From="STATE_COLLAPSING";
            break;
    }

    switch(State)
    {
    case STATE_VERIFYING_TARGETS:
        To="STATE_VERIFYING_TARGETS";
        break;
    case STATE_KILLING_TARGETS:
        To="STATE_KILLING_TARGETS";
        break;
    case STATE_COLLAPSING:
        To="STATE_COLLAPSING";
        break;
    }

    CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::SwitchState", "From %s to %s",From,To);
    LOG_FLUSH();
#endif

    // Handle switching from state
    switch( m_State )
    {
    case STATE_VERIFYING_TARGETS:
        break;
    case STATE_KILLING_TARGETS:
        break;
    case STATE_COLLAPSING:
        break;
    }

    m_State = State;
    m_TimeInState = 0;

    // Handle switching to state
    switch( m_State )
    {
    case STATE_VERIFYING_TARGETS:
        break;
    case STATE_KILLING_TARGETS:
        break;
    case STATE_COLLAPSING: 
        {
            if (m_CoreFancyFX.Validate())
                m_CoreFancyFX.SetSuspended(TRUE);
            if (m_MainExplosionFX.Validate())
                m_MainExplosionFX.KillInstance();

            rhandle<char> FxResource;    
            FxResource.SetName( PRELOAD_FILE("MSN_Beeeeooop.fxo") );
            const char* pScrewedUpName = FxResource.GetPointer();
            if ( pScrewedUpName )
            {
                m_MainExplosionFX.InitInstance( pScrewedUpName );
                m_MainExplosionFX.SetSuspended( TRUE );
                m_MainExplosionFX.SetTranslation( m_Origin );
                m_MainExplosionFX.SetScale( vector3(1,1,1) );        
            }

            if (m_MainExplosionVoiceID != -1)
                g_AudioMgr.Release( m_MainExplosionVoiceID, 0 );
            m_MainExplosionVoiceID = -1;
            
            g_AudioMgr.Play( s_pSoundNames[ SOUND_MAIN_IMPLODE ], m_Origin, GetZone1(), TRUE );

            // Shut down lash effects
            s32 i;
            for (i=0;i<MAX_LASHES;i++)
            {
                if (m_Lash[i].m_FX.Validate())
                    m_Lash[i].m_FX.KillInstance();
                m_Lash[i].m_bActive = FALSE;
            }

            //TODO:NET Send notification to clients about collapse state starting.
    #ifndef X_EDITOR
            m_NetDirtyBits |= debris_meson_explosion::DIRTY_COLLAPSE;
    #endif
        }
        break;
    }
}

//=============================================================================

void debris_meson_explosion::OnAdvanceLogic      ( f32 DeltaTime )
{
    // Handle total time calculations
    m_TotalTime   += DeltaTime;
    m_TimeInState += DeltaTime;
    
    if (m_CoreFancyFX.Validate())
        m_CoreFancyFX.AdvanceLogic( DeltaTime );
    if (m_MainExplosionFX.Validate())
        m_MainExplosionFX.AdvanceLogic( DeltaTime );

    // Run sub-logic
    switch( m_State )
    {
    case STATE_VERIFYING_TARGETS:
        AdvanceVerificationLogic( DeltaTime );
        break;
    case STATE_KILLING_TARGETS:
        AdvanceAttackLogic( DeltaTime );
        break;
    case STATE_COLLAPSING:
        AdvanceCollapseLogic( DeltaTime );
        break;
    }

    // Update local BBOX
    m_LocalBBox.Set( vector3(0,0,0), 100.0f );
    s32 i;
    vector3 LocalPt[MAX_LASHES + MAX_BEAMS];
    for (i=0;i<MAX_LASHES;i++)
    {
        if (!m_Lash[i].m_bActive)
        {
            LocalPt[i].Zero();
            continue;
        }        
        LocalPt[i] = m_Lash[i].m_TargetPoint - m_Origin;        
    }
    for (i=0;i<MAX_BEAMS;i++)
    {
        if ((m_Beam[i].m_Timer > 0) || (m_Beam[i].m_ImpactFX.Validate()))
        {
            LocalPt[ MAX_LASHES + i ] = m_Beam[i].m_EndPt - m_Origin;
        }
        else
        {
            LocalPt[ MAX_LASHES + i ].Zero();
        }
    }
    m_LocalBBox.AddVerts( LocalPt, MAX_LASHES + MAX_BEAMS );
    m_LocalBBox.Inflate( 300, 300, 300 );

    // Update world bbox if we haven't been marked for destruction
    if (!m_bDestroying)
    {
        vector3 Pos = GetPosition();
        OnMove( Pos );
    }
}

//=============================================================================

void debris_meson_explosion::AdvanceAttackLogic( f32 DeltaTime )
{   
    // Advance the lashes
    s32 nActiveLashes = UpdateLashes( DeltaTime );
        
#ifndef X_EDITOR
    if (GameMgr.IsGameMultiplayer())
    {
        // In multiplayer, we limit ourselves to 8 lashes, and we send all of them
        // out as soon as we enter attack mode.        
        // Doesn't matter if it is client or server.  The only way to get here is to be in
        // the killing state.  Server gets there by finishing up targeting, and client
        // gets there by receiving all of the targeting information.
        if (m_iNextTarget < m_nLashTargets)
        {
            s32 i;
            for (i=0;i<8;i++)
            {
                nActiveLashes += CreateNewLashes(TRUE);
            }
        }
    }
    else
#endif
    {
        if (m_LashSpawnTimer > 0)
            m_LashSpawnTimer -= DeltaTime;
        nActiveLashes += CreateNewLashes(FALSE);
    }
    

    // Everyone can create new beams.  They do not interact with anything.
    // Advance the beams
    UpdateBeams( DeltaTime );
    
    m_BeamTimer -= DeltaTime;
    if (m_BeamTimer <= 0)
    {
        // Only create beams under 2 circumstances:
        // 1) we haven't been in attack state long enough to exit
        // 2) we still have targets.
        if ((m_TimeInState < k_MIN_KILLING_STATE_TIME) || (m_iNextTarget < m_nLashTargets))
        {
            if (CreateNewBeam())
                m_BeamTimer = x_frand( k_MIN_TIME_BETWEEN_BEAMS, k_MAX_TIME_BETWEEN_BEAMS );
        }
    } 

    // The weapon stops attacking when it out of targets and has been active for a specified amount of time.
    // It will also stop if it exceeds a specified amount of time.
    xbool bWantToCollapse = FALSE;

    if ((m_TimeInState >= k_MIN_KILLING_STATE_TIME) && (m_iNextTarget >= m_nLashTargets) && (nActiveLashes==0))       
    {
        bWantToCollapse = TRUE;
    }

#ifndef X_EDITOR
    // Only the originating machine can make the decision to switch to collapsing state
    // The remote machines need to wait until they get the collapse signal
    if( m_OwningClient != g_NetworkMgr.GetClientIndex() )
    {
        bWantToCollapse = FALSE;
        if (m_bNetCollapseSignaled && (m_iNextTarget >= m_nLashTargets))
        {
            bWantToCollapse = TRUE;
        }
    }    
#endif

    if (bWantToCollapse)
    {
        SwitchState( STATE_COLLAPSING );
    }

}

//=============================================================================

void debris_meson_explosion::AdvanceCollapseLogic( f32 DeltaTime )
{
    if (m_TimeInState >= 2.0f)
        m_MainExplosionFX.SetSuspended(FALSE);

    xbool bCollapseFXPlaying = FALSE;
    if (m_MainExplosionFX.Validate())
    {
        if (!m_MainExplosionFX.IsFinished())
        {
            bCollapseFXPlaying = TRUE;
        }
    }
    if ((m_TimeInState > 3.0f) && (!bCollapseFXPlaying))
    {
        DESTROY_NET_OBJECT( this );  
        m_bDestroying = TRUE;
        return;
    }    

    // Advance the beams
    UpdateBeams( DeltaTime );
}

//=============================================================================

void debris_meson_explosion::AdvanceVerificationLogic( f32 DeltaTime )
{
    (void)DeltaTime;

#ifndef X_EDITOR
    if( m_OwningClient != g_NetworkMgr.GetClientIndex() )
    {
        // NO thinking allowed if this isn't the origination machine
        return;
    }
#endif

    s32 nRayTestsRemaining = 12;

    while (nRayTestsRemaining > 0)
    {
        xbool bPureLOSOnly = FALSE;             // If TRUE, only test to aim at point.

        if (m_iNextToBeValidated == m_nLashTargets)
            break;

        lash_target& T = m_LashTargets[ m_iNextToBeValidated ];
        m_iNextToBeValidated++;

        // Verify target 
        object* pObj = g_ObjMgr.GetObjectByGuid( T.gObj );

        if (NULL == pObj)
        {
            T.bIsValidTarget = FALSE;
            continue;
        }

        // Build targeting points
        // AimAt( or center ), Side, OtherSide, Above
        vector3 Pt[4];

        if (pObj)
        {
            if (pObj->IsKindOf( actor::GetRTTI() ))
            {
                actor& Actor = actor::GetSafeType( *pObj );
                Pt[0] = Actor.GetPositionWithOffset( actor::OFFSET_AIM_AT );            
            }
            else
            {
                Pt[0] = pObj->GetBBox().GetCenter();
            }   

            if (pObj->IsKindOf( alien_glob::GetRTTI() ))
            {
                bPureLOSOnly = TRUE;
            }
        }    

        vector3 ToPt = Pt[0] - m_Origin;
        ToPt.Normalize();

        vector3 Side = ToPt.Cross( vector3(0,1,0) );

        Side.NormalizeAndScale( k_LOS_VERIFICATION_SIDE_DIST );

        Pt[1] = Pt[0] + Side;
        Pt[2] = Pt[0] - Side;
        Pt[3] = Pt[0] + vector3(0, k_LOS_VERIFICATION_SIDE_DIST, 0 );

        vector3 SightPoint = m_Origin;
        SightPoint == m_OriginNormal * 5.0f;

        // try to validate one of the points
        s32 j;
        s32 MaxPtsToTest = 4;

        if (bPureLOSOnly)
            MaxPtsToTest = 1;

        for (j=0;j<MaxPtsToTest;j++)
        {
            nRayTestsRemaining--;
            g_CollisionMgr.LineOfSightSetup( GetGuid(), SightPoint, Pt[j] );
            g_CollisionMgr.SetMaxCollisions(1);            
            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                                            object::ATTR_COLLIDABLE,
                                            (object::object_attr)(object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING | object::ATTR_DAMAGEABLE));
            if (g_CollisionMgr.m_nCollisions == 0)
            {
                break;
            }

        }

        if (j<4)
        {
            // This is not visible from the eyepoint
            T.bIsValidTarget = TRUE;
        }        
    }

    if (m_iNextToBeValidated == m_nLashTargets)
    {
        // We're done verifying everything
        CollapseTargetList();        
#ifndef X_EDITOR
        m_NetTargetStatusCounter = m_nLashTargets;
        m_NetDirtyBits |= DIRTY_TARGETING_INFO;
#endif
        SwitchState( STATE_KILLING_TARGETS );
    }
}

//=============================================================================

void debris_meson_explosion::CollapseTargetList( void )
{
    s32 i,iNextToFill = 0;
    s32 nTargets = 0;
    for (i=0;i<m_nLashTargets;i++)
    {
        if (m_LashTargets[i].bIsValidTarget)
        {
            nTargets++;
            if (i==iNextToFill)
            {
                iNextToFill++;
            }
            else
            {
                m_LashTargets[ iNextToFill ] = m_LashTargets[ i ];
                iNextToFill++;
            }
        }
    }
    m_nLashTargets = nTargets;
}

//=============================================================================

void debris_meson_explosion::OnMove				( const vector3& rNewPos )
{
    object::OnMove( rNewPos );
}

//=============================================================================

// Returns spline position
static vector3 GetSplinePos( const vector3& P0,
                            const vector3& V0,
                            const vector3& P1,
                            const vector3& V1,
                            f32 T )
{
    // Compute time powers
    f32 T2   = T*T ;
    f32 T3   = T2*T ;
    f32 T3x2 = T3*2 ;
    f32 T2x3 = T2*3 ;

    // Compute coefficients
    f32 w0 =  T3x2 - T2x3 + 1.0f ;
    f32 w1 = -T3x2 + T2x3 ;    
    f32 w2 =  T3  - (2.0f*T2) + T ; 
    f32 w3 =  T3  -  T2 ;   

    // Compute final spline position
    return ( (w0*P0) + (w1*P1) + (w2*V0) + (w3*V1) ) ;
}

void debris_meson_explosion::OnRender            ( void )
{
    /*
    {
        vector3 RayTowardPlayer(0,0,0);
        radian3 RotationIntoTestPlane(0,0,0);
        vector3 PtInFrontOfPlayerEyes;
        vector3 MyPos = GetPosition();

        player* pPlayer = SMP_UTIL_GetActivePlayer();
        if (NULL == pPlayer)
            return;

        RayTowardPlayer = pPlayer->GetPositionWithOffset( actor::OFFSET_EYES ) - MyPos;
        RayTowardPlayer.Normalize();

        RayTowardPlayer.GetPitchYaw( RotationIntoTestPlane.Pitch, RotationIntoTestPlane.Yaw );

        radian3 EyeRot(0,0,0);
        pPlayer->GetEyesPitchYaw( EyeRot.Pitch, EyeRot.Yaw );

        PtInFrontOfPlayerEyes.Set(0,0,500);
        PtInFrontOfPlayerEyes.Rotate( EyeRot );
        PtInFrontOfPlayerEyes+= pPlayer->GetPosition();

        vector3 Centerline = PtInFrontOfPlayerEyes - MyPos;
        Centerline.Normalize();

        f32 C = 0.75f * (PI/2);
        f32 R = x_sin( C ) * m_ShockwaveRadius;

        vector3 Origin = Centerline;
        Origin.Scale( m_ShockwaveRadius * 0.75f );
        Origin += MyPos;

        s32 i;
        for (i=0;i<60;i++)
        {
            vector3 Dir(0,1,0);
            Dir.RotateZ( i * R_6);
            Dir.Rotate( RotationIntoTestPlane );
            Dir.Scale( R );
            vector3 FinalPt = Origin + Dir;

            g_CollisionMgr.RaySetup( GetGuid(), Origin, FinalPt );
            g_CollisionMgr.SetMaxCollisions(1);            
            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                object::ATTR_COLLIDABLE,
                (object::object_attr)(object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING));

            if (g_CollisionMgr.m_nCollisions > 0)
            {

                FinalPt = g_CollisionMgr.m_Collisions[0].Point;
            }


            draw_Line( Origin, FinalPt );
        }

    }
    */

    
}

//=============================================================================

void debris_meson_explosion::OnRenderTransparent ( void )
{
    //==-------------------------------
    // RENDER ARCS
    //==-------------------------------
   // s32 i;
    draw_ClearL2W();
    /*
    draw_Begin( DRAW_TRIANGLES, DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_NO_ZWRITE | DRAW_CULL_NONE );        
    xbitmap* pBitmap = m_Texture.GetPointer();
    draw_SetTexture( *pBitmap );
*/
    /*
    for (i=0;i<MAX_IMPACTS;i++)
    {
        impact& Imp = m_Impact[i];
        if (Imp.iArcDest == -1)
            continue;

        // Don't render if the arc has already been rendered
        if ((i == m_Impact[ Imp.iArcDest ].iArcDest) && (Imp.iArcDest > m_Impact[ Imp.iArcDest ].iArcDest))
            continue;

        s32 j;
        f32 t;
        f32 Dot = 1.0f - (x_abs(Imp.Normal.Dot( m_Impact[ Imp.iArcDest ].Normal )));
        Dot = MAX(Dot,0.2f);

        f32 Scale = 150.0f * Dot * Imp.ArcScale;
        vector3 P0 = Imp.Pt;
        vector3 P1 = m_Impact[ Imp.iArcDest ].Pt;
        vector3 N0 = Imp.Normal * Scale;
        vector3 N1 = m_Impact[ Imp.iArcDest ].Normal * -Scale;
        vector3 Pt[6];

        Pt[0] = P0;
        for (j=0,t=0.2f;j<5;j++,t+=0.2f)
        {
            Pt[j+1] = GetSplinePos( P0, N0, P1, N1, t );            
        }

        f32 InitialU = x_frand(0,1);

        draw_OrientedStrand( Pt, 6, vector2(InitialU,0), vector2(InitialU + 0.3f,1), XCOLOR_WHITE, XCOLOR_WHITE, 5 );
    }
    draw_End();
*/
    /*
    for (i=0;i<MAX_IMPACTS;i++)
    {
        impact& Imp = m_Impact[i];
        draw_Sphere( Imp.Pt, 5, XCOLOR_RED );
    }
    */

//    draw_Sphere( GetPosition(), m_MaxRadius, XCOLOR_RED );
//    RenderImpacts();

    RenderLashes();   
    RenderBeams();

    s32 i;
    for (i=0;i<MAX_LASHES;i++)
    {
        if (!m_Lash[i].m_bActive)
            continue;
        if (m_Lash[i].m_FX.Validate())
        {
            m_Lash[i].m_FX.SetTranslation( m_Lash[i].m_EndPoint );
            m_Lash[i].m_FX.Render();
        }
    }
/*
    // Orient and render the core fx
    const view* pView = eng_GetView();

    vector3 Fwd = pView->GetViewZ();
    Fwd *= -1;
    radian3 Rot(0,0,0);
    Fwd.GetPitchYaw( Rot.Pitch, Rot.Yaw );

    Rot.Set( x_frand(-R_360,R_360), x_frand(-R_360,R_360), x_frand(-R_360,R_360) );
*/
    if (m_CoreFancyFX.Validate())
    {        
        //m_CoreFancyFX.SetRotation( Rot );
        m_CoreFancyFX.Render();
    }
    if (m_MainExplosionFX.Validate())
    {
        //m_MainExplosionFX.SetRotation( Rot );
        m_MainExplosionFX.Render();
    }
}

//=============================================================================

s32 debris_meson_explosion::UpdateLashes( f32 DeltaTime )
{
    s32 i;
    s32 nActive = 0;
    for (i=0;i<MAX_LASHES;i++)
    {
        lash& L = m_Lash[i];

        if (!L.m_bActive)
            continue;

        //==-------------------------------------
        // Advance FX
        //==-------------------------------------
        if (m_Lash[i].m_FX.Validate())
        {
            m_Lash[i].m_FX.AdvanceLogic( DeltaTime );
        }
        
        //==-------------------------------------
        // Update scale of lash
        //==-------------------------------------
        m_Lash[i].m_Scale += 0.2f * DeltaTime;       

        //==-------------------------------------
        // Check for end of state
        //==-------------------------------------
        L.m_CurTime += DeltaTime;

        if (L.m_CurTime > L.m_StateTime)
        {
            // State time has elapsed
            // Perform shutdown/state-switch actions
            switch(L.m_State)
            {
            case LASH_EXTEND:
                {
                    CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::UpdateLashes","%08X Lash %d - LASH_EXTEND finished. begin logic",this,i);                        
                    xbool               bSwitchToInteract = TRUE;
                    xbool               bSwitchToRetract  = FALSE;  
                    
                    
#ifndef X_EDITOR
                    if (L.m_TargetType == TARGET_ACTOR)
                    {
                        xbool               bLifeSeqMismatch  = FALSE;
                        object_ptr<actor>   pActor( L.m_TargetObject );

                        if (pActor)
                        {
                            if (GameMgr.IsGameMultiplayer())
                            {
                                // Multiplayer 
                                if (pActor->net_GetLifeSeq() != L.m_NetActorLifeSeq)
                                    bLifeSeqMismatch = TRUE;
                                
                                // Target is still alive and in the game
                                if (m_OwningClient == g_NetworkMgr.GetClientIndex())
                                {
                                    CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::UpdateLashes","%08X smart updating lash %d",this,i);

                                    // **==== Smart explosion ====**
                                    m_NetTargetStatusCounter--;

                                    if (bLifeSeqMismatch)
                                    {
                                        // Target has respawned for some reason.
                                        // Mark him as such and retract
                                        bSwitchToRetract  = TRUE;                                    
                                        bSwitchToInteract = FALSE;
                                        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::UpdateLashes","%08X Lash %d - life seq mismatch",this,i);
                                    }
                                    else
                                    {                                   
                                        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::UpdateLashes","%08X Lash %d - harming target",this,i);
                                        xbool bKilled     = L.HarmTarget();
                                        m_NetKilledMask  |= (bKilled?1:0) << i;                                

                                        if (bKilled)
                                        {
                                            bSwitchToInteract = TRUE;       
                                            bSwitchToRetract  = FALSE;
                                        }
                                        else
                                        {
                                            bSwitchToInteract = FALSE;
                                            bSwitchToRetract  = TRUE;
                                        }
                                    }

                                    // If we have now processed everyone, spread the status to the
                                    // dumb explosions.
                                    if (m_NetTargetStatusCounter <= 0)
                                    {
                                        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::UpdateLashes","%08X Lash %d - all complete now.  transmitting target info %08X",this,i,m_NetKilledMask);
                                        m_NetDirtyBits  |= DIRTY_TARGET_STATUS_INFO;
                                    }
                                }
                                else
                                {
                                    // **==== Dumb explosion ====**

                                    CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::UpdateLashes","%08X dumb updating lash %d",this,i);

                                    bSwitchToRetract  = FALSE;
                                    bSwitchToInteract = FALSE;

                                    // - can't do anything until m_bNetDumbTargetStatusSignaled is set.
                                    if (m_bNetDumbTargetStatusSignaled)
                                    {
                                        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::UpdateLashes","%08X Lash %d - target status is signaled %08X",this,i,m_NetKilledMask);
                                        u8 Bit = 1 << i;
                                        if (m_NetKilledMask & Bit)
                                        {
                                            CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::UpdateLashes","%08X Lash %d - this target marked to die",this,i);
                                            // We need to wait until he dies or respawns
                                            if (pActor->IsDead())
                                            {
                                                CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::UpdateLashes","%08X Lash %d - target has died, ok to interact now",this,i);
                                                bSwitchToInteract = TRUE;
                                                bSwitchToRetract  = FALSE;
                                            }
                                            else if (bLifeSeqMismatch)
                                            {
                                                CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::UpdateLashes","%08X Lash %d - target has respawned, retract",this,i);
                                                bSwitchToInteract = FALSE;
                                                bSwitchToRetract  = TRUE;
                                            }
                                            else
                                            {
                                                CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::UpdateLashes","%08X Lash %d - waiting for target to change state",this,i);
                                                bSwitchToRetract  = FALSE;
                                                bSwitchToInteract = FALSE;
                                            }
                                        }
                                        else
                                        {
                                            CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::UpdateLashes","%08X Lash %d - this target marked to survive",this,i);
                                            // Retract away from the target immediately
                                            bSwitchToInteract = FALSE;
                                            bSwitchToRetract  = TRUE;
                                        }
                                    }  
                                    else if (L.m_CurTime > MP_DUMB_EXPLOSION_MAX_WAIT_FOR_TARGET_INFO)
                                    {
                                        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::UpdateLashes","%08X Lash %d - waited too long for target info to be signaled.  retracting.",this,i);
                                        bSwitchToInteract = FALSE;
                                        bSwitchToRetract = TRUE;
                                    }
                                    else
                                    {
                                        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::UpdateLashes","%08X Lash %d - waiting for target status to be signalled",this,i);
                                    }
                                }                       
                            }
                            else
                            {
                                // Campaign uses the defaults
                                // Switch to interact
                                CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::UpdateLashes","%08X Lash %d - campaign mode.  direct to harm.",this,i);
                                L.HarmTarget();
                            }
                        }
                        else // pActor == NULL
                        {
                            
                            // Can't find the actor at all.  eek.

                            CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::UpdateLashes","%08X Lash %d - Target has disappeared.  retract immediately",this,i);
                            
                            bSwitchToRetract  = TRUE;
                            bSwitchToInteract = FALSE;

                            // If we're in MP, and we're the smart explosion, we still need to count this actor
                            if (GameMgr.IsGameMultiplayer() && (m_OwningClient == g_NetworkMgr.GetClientIndex()))
                            {                                            
                                m_NetTargetStatusCounter--;                            
                            }
                        }
                    }
                    else
                    {
                        // if it's not an actor, then just zap it and shake
                        L.HarmTarget();
                        bSwitchToRetract  = FALSE;
                        bSwitchToInteract = TRUE;
                    }
#else // X_EDITOR
                    if (L.HarmTarget())
                    {
                        bSwitchToInteract = TRUE;
                        bSwitchToRetract  = FALSE;
                    }
                    else
                    {
                        bSwitchToInteract = FALSE;
                        bSwitchToRetract  = TRUE;
                    }
#endif


                    // Logic is done.  Switch to new state if required
                    if (bSwitchToRetract)
                    {
                        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::UpdateLashes","%08X Lash %d - StateSwitch - RETRACT",this,i);                        
                        L.m_State     = LASH_RETRACT;
                        L.m_CurTime   = 0;
                        L.m_StateTime = m_TargetBehaviour[ L.m_TargetType ].RetractTime;
                        L.m_DesiredSize = s_LashSizes[ LASH_RETRACT ];
                        if (L.m_VoiceID != -1)
                            g_AudioMgr.Release( L.m_VoiceID, 0 );                                    
                        L.m_VoiceID = g_AudioMgr.Play( s_pSoundNames[ SOUND_LASH_RETRACT ], L.m_TargetPoint, GetZone1(), TRUE );
                        break;
                    }
                    else
                    if (bSwitchToInteract)
                    {                  
                        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::UpdateLashes","%08X Lash %d - StateSwitch - INTERACT",this,i);                        
                        L.AcquireCorpse();          //****************************** Can we bail if we don't get a corpse?
                        L.m_State     = LASH_INTERACT;
                        L.m_CurTime   = 0;                        
                        L.m_DesiredSize = s_LashSizes[ LASH_INTERACT ];
                        L.m_StateTime = m_TargetBehaviour[ L.m_TargetType ].InteractTime;   
                        if (L.m_VoiceID != -1)
                            g_AudioMgr.Release( L.m_VoiceID, 0 );
                        g_AudioMgr.Play( s_pSoundNames[ SOUND_LASH_EXTEND_STOP ], L.m_TargetPoint, GetZone1(), TRUE );
                        L.m_VoiceID = g_AudioMgr.Play( s_pSoundNames[ SOUND_LASH_HOLD ], L.m_TargetPoint, GetZone1(), TRUE );
                    }           
                }
                break;
            case LASH_INTERACT:
                {
                    CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::UpdateLashes","%08X Lash %d - LASH_INTERACT finished. begin logic",this,i);                        
                    L.ReleaseCorpse();
                    
                    L.m_State     = LASH_RETRACT;
                    L.m_CurTime   = 0;
                    L.m_StateTime = m_TargetBehaviour[ L.m_TargetType ].RetractTime;
                    L.m_DesiredSize = s_LashSizes[ LASH_RETRACT ];
                    if (L.m_VoiceID != -1)
                        g_AudioMgr.Release( L.m_VoiceID, 0 );
                    g_AudioMgr.Play( s_pSoundNames[ SOUND_LASH_HOLD_STOP ], L.m_TargetPoint, GetZone1(), TRUE );
                    L.m_VoiceID = g_AudioMgr.Play( s_pSoundNames[ SOUND_LASH_RETRACT ], L.m_TargetPoint, GetZone1(), TRUE );
                }
                break;
            case LASH_RETRACT:
                {
                    CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::lash::UpdateLashes","%08X Lash %d - LASH_RETRACT finished. begin logic",this,i);                        
                    L.m_bActive   = FALSE;
                    if (L.m_FX.Validate())
                        L.m_FX.KillInstance();

                    if (L.m_VoiceID != -1)
                        g_AudioMgr.Release( L.m_VoiceID, 0 );

                    L.m_iConstraint = -1;

                    g_AudioMgr.Play( s_pSoundNames[ SOUND_LASH_RETRACT_STOP ], L.m_TargetPoint, GetZone1(), TRUE );
                }
                break;
            }
        }
        else
        {
            L.AdvanceLogic( DeltaTime );
        }

        if (L.m_bActive)
        {
            // If its still running, increment counter
            // and update target point
            nActive++;
            UpdateTargetPoint( L );
        }
    }

    
    return nActive;
}

//=============================================================================

void BuildPoints( vector3* pOutPos, s32 nPos, vector3* pSourcePt, vector3* pSourceNrm, s32 nSource, f32 T )
{
    f32 TStep = T / (nPos-1);
    f32 ChunkSize = 1.0f / (nSource - 1);

    ASSERT( pOutPos );
    ASSERT( pSourcePt );
    ASSERT( pSourceNrm );
    ASSERT( nPos > 0 );
    ASSERT( nSource > 1 );
    
    pOutPos[0] = pSourcePt[0];
    s32 i;
    f32 CurT = TStep;
    for (i=1;i<nPos;i++,CurT+=TStep)
    {       
        s32 iBase = (s32)(CurT / ChunkSize);
        f32 LocalT = (CurT - (iBase * ChunkSize)) / ChunkSize;

        pOutPos[i] = GetSplinePos( pSourcePt[ iBase ],
                                   pSourceNrm[ iBase ],
                                   pSourcePt[ iBase+1 ],
                                   pSourceNrm[ iBase+1 ],
                                   LocalT );
                                   
        ASSERT( pOutPos[i].IsValid() );
    }
}

static f32 LashScale = 1000.0f;
static f32 LashScale2 = 1.0f;
static f32 RadiusScale = 2.0f;

void debris_meson_explosion::RenderLashes( void )
{
    draw_ClearL2W();

    s32 i;
    
    vector3 Pt[3];
    vector3 Nrm[3];
    f32 MidPoint = 0.5f;

    f32 SecondsForSingleTile = 0.5f;

    f32 Ofs = (m_TotalTime - ((s32)(m_TotalTime / SecondsForSingleTile)*SecondsForSingleTile)) / SecondsForSingleTile;
    Ofs = x_frand(0,1);
    for (i=0;i<MAX_LASHES;i++)
    {
        if (!m_Lash[i].m_bActive)
            continue;

        lash& L = m_Lash[i];

        // Build the 3 nodes of the spline
        // 0 = origin
        // 1 = midpoint
        // 2 = target

        Pt[0] = L.m_Origin;
        Pt[2] = L.m_TargetPoint;

        vector3 DeltaToTarget = Pt[2] - Pt[0];
        vector3 DeltaToTargetNorm = DeltaToTarget;
        f32     MaxLength = DeltaToTarget.Length();
        DeltaToTargetNorm.Normalize();

        Pt[1] = Pt[0] + DeltaToTargetNorm * (DeltaToTarget.Length() * MidPoint);
        Pt[1].GetY() = Pt[1].GetY() + 50.0f;

        Nrm[0]  = L.m_NormalAtOrigin;
        Nrm[1]  = Pt[1] - Pt[0];
        Nrm[2]  = Pt[2] - Pt[1];         

        f32 Len[3];
        Len[0] = Nrm[0].Length();
        Len[1] = Nrm[1].Length();
        Len[2] = Nrm[2].Length();

        f32 SwimSpeed = 4.0f;
        f32 SwimY = R_45 * x_sin( SwimSpeed * m_TotalTime * L.m_SwimScalar );
        f32 SwimX = R_45 * x_sin( SwimSpeed * m_TotalTime * L.m_SwimScalar * 1.2f );
        Nrm[1].RotateY( SwimY );
        Nrm[1].RotateX( SwimX );

        SwimY = R_25 * x_sin( SwimSpeed * m_TotalTime * L.m_SwimScalar * 2.1f );
        SwimX = R_25 * x_cos( SwimSpeed * m_TotalTime * L.m_SwimScalar * 1.7f );
        Nrm[2].RotateY( SwimX );
        Nrm[2].RotateX( SwimY );
        
        // Tweak out the final normal
        switch( L.m_TargetType )
        {
        case TARGET_THETA:
            {
                if (L.m_iThetaBoneIndex == -1)
                {
                    Nrm[2].Set(0,-1,0);
                }
            }
            break;
        case TARGET_SHIELD:
            {
                Nrm[2] = L.m_TargetPoint - m_Origin;
            }
            break;
        case TARGET_NONACTOR_DAMAGEABLE:
            {
                object_ptr<turret> pTurret(L.m_TargetObject);
                if (pTurret)
                {
                    radian3 Rot;
                    vector3 Pos;
                    Nrm[2].Set(0,0,-1);
                    pTurret->GetSensorInfo( Pos,Rot );
                    Nrm[2].Rotate(Rot);
                }
                else
                {
                    Nrm[2].Set(0,-1,0);
                }
            }
            break;
        default:
            Nrm[2].Set(0,-1,0);
            break;
        }

        Nrm[0].NormalizeAndScale( LashScale * L.m_Scale );
        Nrm[1].NormalizeAndScale( Len[1] * LashScale2 * L.m_Scale );
        Nrm[2].NormalizeAndScale( Len[2] * LashScale2 * L.m_Scale );
       
        Nrm[0].Rotate( L.m_RandomRot );
        Nrm[2].Rotate( L.m_RandomRot );

        vector3 Pos[ k_MAX_SPLINE_POINTS ];
        f32 T = 0;

        switch( L.m_State )
        {
        case LASH_EXTEND:
            if (L.m_StateTime == 0)
                T = 1;
            else
                T = L.m_CurTime / L.m_StateTime;
            break;
        case LASH_INTERACT:
            T = 1.0f;
            break;
        case LASH_RETRACT:
            if (L.m_StateTime == 0)
                T = 0;
            else
                T = 1.0f - (L.m_CurTime / L.m_StateTime);
            break;
        }
        

        // Put T on a nice curve
        T = x_sin( (T * (PI/2)) + (3*PI/2) ) + 1.0f;
        T = MIN(1,MAX(0,T));

        s32 nPoints = (s32)((MaxLength * T) / 50.0f);
        nPoints = MAX(k_MIN_SPLINE_POINTS,MIN(k_MAX_SPLINE_POINTS,nPoints));

        BuildPoints( Pos, nPoints, Pt, Nrm, 3, T );
        
        draw_Begin( DRAW_TRIANGLES, DRAW_BLEND_ADD | DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_NO_ZWRITE | DRAW_CULL_NONE );        
        xbitmap* pBitmap = m_Texture.GetPointer();
        draw_SetTexture( *pBitmap );
        draw_OrientedStrand( Pos, nPoints, vector2(Ofs,0), vector2(1+Ofs,1), L.m_Color, L.m_Color, L.m_Size * RadiusScale );
        draw_End();  

        L.m_PainDirection = Pos[nPoints-1] - Pos[nPoints-2];    
        L.m_PainDirection.Normalize();
        L.m_EndPoint = Pos[nPoints-1];
    }

    
}

//=============================================================================

s32 debris_meson_explosion::CreateNewLashes( xbool bNetworkForced )
{
    // We are only allowed to bail do to aesthetic timing reasons
    // when the network isn't forcing us to catch up.    
    if (!bNetworkForced)
    {
        if (m_LashSpawnTimer > 0)
            return 0;
    }

    // Bail if we are out of targets
    if (m_iNextTarget >= m_nLashTargets)
        return 0;

    // See if there is a free lash
    s32 i;
    for (i=0;i<MAX_LASHES;i++)
    {
        if (!m_Lash[i].m_bActive)
            break;
    }

    if (i == MAX_LASHES)
        return 0;

    // Setup a new lash 
    lash&        L = m_Lash[i];   
    lash_target& T = m_LashTargets[ m_iNextTarget ];

    // Skip index to next valid target
    m_iNextTarget++;
    while (m_iNextTarget < m_nLashTargets)
    {
        if (m_LashTargets[ m_iNextTarget ].bIsValidTarget)                   
            break;

        m_iNextTarget++;
    }

    // Bail out if this object no longer exists
    object* pObj = g_ObjMgr.GetObjectByGuid( T.gObj );
    if (NULL == pObj)
        return 0;    

    //TODO:NET  Send notification to ghosts that they need to 
    //          attack the new target
    m_iNetCurrentTarget = m_iNextTarget;

#ifndef X_EDITOR
    if (GameMgr.IsGameMultiplayer() && (net_GetOwningClient() == g_NetworkMgr.GetClientIndex()))
    {
        // If this is the smart explosion, notify others of the new target id
        m_NetDirtyBits |= debris_meson_explosion::DIRTY_NEXT_TARGET_ID;
    }
#endif

    // Note: we have to grab the target point on the fly because 
    // it is possible for a damageable object to be moving around
    L.m_Origin         = m_Origin;
    L.m_NormalAtOrigin = m_OriginNormal;
    L.m_Scale          = 1.0f;
    L.m_TargetObject   = T.gObj; 
    L.m_TargetPoint    = GetPosition();
    L.m_EndPoint       = m_Origin;
    L.m_bActive        = TRUE;
    L.m_StartTime      = m_TotalTime;
    L.m_CurTime        = 0;
    L.m_State          = LASH_EXTEND;
    L.m_StateTime      = x_frand( m_TargetBehaviour[ L.m_TargetType ].ExtendMinTime, m_TargetBehaviour[ L.m_TargetType ].ExtendMaxTime );
    L.m_TargetType     = T.TargetType;
    L.m_Color.Set( k_START_COLOR );
    L.m_DesiredColor.Set( k_START_COLOR );
    L.m_PrecisionColor.GetX() = L.m_Color.R;
    L.m_PrecisionColor.GetY() = L.m_Color.G;
    L.m_PrecisionColor.GetZ() = L.m_Color.B;
    L.m_PrecisionColor.GetW() = L.m_Color.A;
    L.m_Size = 2.0f;
    L.m_DesiredSize = s_LashSizes[ LASH_EXTEND ];
    L.m_bStunTheta      = T.bStunTheta;
    {
        f32 x = x_frand(-R_7,R_7);
        f32 y = x_frand(-R_7,R_7);
        f32 z = x_frand(-R_7,R_7);
        L.m_RandomRot.Set( x, y, z );
    }
    L.m_iThetaBoneIndex = T.ThetaBoneIndex;
    if (L.m_VoiceID != -1)
        g_AudioMgr.Release( L.m_VoiceID, 0 );    
    L.m_VoiceID = g_AudioMgr.Play( s_pSoundNames[ SOUND_LASH_EXTEND ], m_Origin, GetZone1(), TRUE );
    L.m_SwimScalar = x_frand(0.5f,1.5f);
    L.m_NetActorLifeSeq = T.NetActorLifeSeq;
    L.m_iConstraint = -1;
    L.m_CorpseGuid = NULL_GUID;    

#ifndef X_EDITOR
    if (GameMgr.IsGameMultiplayer())
    {
        L.m_StateTime = 0;
    }
#endif

    UpdateTargetPoint( L );    

    rhandle<char> FxResource;        
    FxResource.SetName( PRELOAD_FILE("MSN_Tenticle_head.fxo") );
    const char* pScrewedUpName = FxResource.GetPointer();
    if ( pScrewedUpName )
    {
        L.m_FX.InitInstance( pScrewedUpName );
        L.m_FX.SetSuspended( FALSE );
        L.m_FX.SetTranslation( L.m_Origin );
        L.m_FX.SetScale( vector3(1,1,1) );        
    }

    m_LashSpawnTimer = k_TIME_BETWEEN_LASHES + x_frand(0,0.25f);

    CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::CreateNewLashes", 
        "Created new lash" );

    return 1;
}

//=============================================================================

void debris_meson_explosion::UpdateTargetPoint( lash& L )
{
    // If we are attached to a corpse, just return
    // the main constrain position;
    if (L.m_CorpseGuid != NULL_GUID)
    {
        L.m_TargetPoint = L.m_ConstraintPosition;
        return;
    }    

    object* pObj = g_ObjMgr.GetObjectByGuid( L.m_TargetObject );

    if (pObj)
    {
        switch( L.m_TargetType )
        {
        case TARGET_ACTOR:
            {
                actor& Actor = actor::GetSafeType( *pObj );

                // Only update the position if the actor has now respawned.
                // Once he has respawned, we would look silly if we jumped
                // the arc to the new location.
#ifndef X_EDITOR
                xbool bValid = TRUE;
                if (GameMgr.IsGameMultiplayer())
                {
                    if (Actor.net_GetLifeSeq() != L.m_NetActorLifeSeq)
                        bValid = FALSE;
                }

                if (bValid)
#endif
                {
                    L.m_TargetPoint = Actor.GetPositionWithOffset( actor::OFFSET_AIM_AT );            
                }
            }
            break;        
        case TARGET_THETA:
            {
                f32    S     = 15.0f;
                actor& Actor = actor::GetSafeType( *pObj );

                if (L.m_iThetaBoneIndex == -1)
                {
                    L.m_TargetPoint  = Actor.GetPositionWithOffset( actor::OFFSET_AIM_AT );                     
                }
                else
                {
                    L.m_TargetPoint  = Actor.GetBonePos( L.m_iThetaBoneIndex );
                }
                f32 x = x_frand(-S,S);
                f32 y = x_frand(-S,S);
                f32 z = x_frand(-S,S);
                L.m_TargetPoint += vector3( x, y, z );
            }
            break;        
        case TARGET_SHIELD:
            {
                alien_shield& Shield = alien_shield::GetSafeType( *pObj );
                L.m_TargetPoint = Shield.GetAimAtPoint( GetPosition() );
            }
            break;
        case TARGET_NONACTOR_DAMAGEABLE:
            {
                if (pObj->IsKindOf( turret::GetRTTI() ))
                {
                    turret& Turret = turret::GetSafeType( *pObj );
                    radian3 Rot;
                    Turret.GetSensorInfo(L.m_TargetPoint, Rot);
                }
                else
                {
                    L.m_TargetPoint = pObj->GetBBox().GetCenter();
                }
            }
            break;
        default:
            L.m_TargetPoint = pObj->GetBBox().GetCenter();
            break;

        }
        if (pObj->IsKindOf( actor::GetRTTI() ))
        {
            
        }
        else
        {
            
        }        
    }    
}

//=============================================================================

void debris_meson_explosion::UpdateBeams( f32 DeltaTime )
{
    s32 i;
    for (i=0;i<MAX_BEAMS;i++)
    {
        if (m_Beam[i].m_ImpactFX.Validate())
        {
            m_Beam[i].m_ImpactFX.AdvanceLogic( DeltaTime );
            if (m_Beam[i].m_ImpactFX.IsFinished())
                m_Beam[i].m_ImpactFX.KillInstance();           
        }
        if (m_Beam[i].m_HeadFX.Validate())
        {
            m_Beam[i].m_HeadFX.AdvanceLogic( DeltaTime );                 
        }

        if (m_Beam[i].m_Timer > 0)
        {
            m_Beam[i].m_Timer -= DeltaTime;
            if (m_Beam[i].m_Timer <= 0)
            {
                if (m_Beam[i].m_HeadFX.Validate())
                    m_Beam[i].m_HeadFX.KillInstance();
            }
        }        
    }
}

//=============================================================================

void debris_meson_explosion::RenderBeams( void )
{
    s32 i;

    xcolor C1,C2;
    f32 R1,R2;

    C1 = C2 = k_START_COLOR;

    draw_Begin( DRAW_TRIANGLES, DRAW_BLEND_ADD | DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_NO_ZWRITE | DRAW_CULL_NONE );        
    xbitmap* pBitmap = m_Texture.GetPointer();
    draw_SetTexture( *pBitmap );
    vector3 EndPt[ MAX_BEAMS ];

    for (i=0;i<MAX_BEAMS;i++)
    {
        beam& B = m_Beam[i];
        if (B.m_Timer > 0)
        {
            // SH: I've changed this to make beams appear instantly.
            // They oscillate around and fade out very quickly
            B.m_ImpactFX.SetSuspended(FALSE);




            f32 T = B.m_Timer / k_BEAM_LIFE_TIME;
/*
            if ( T > 0.5f)
            {
                if (B.m_ImpactFX.Validate())
                    B.m_ImpactFX.SetSuspended(FALSE);
            }
*/
            T = 1-x_abs(2*T-1);

            f32 Falloff = x_sin(T * PI / 2);
            f32 Falloff2 = x_sin(T * (PI / 2) + (PI / 2)) + 1.0f;

            C1.A = (u8)(Falloff * 255);
            C2.A = (u8)(Falloff2 * 255);

            R1 = 7 * Falloff;
            R2 = 11 * Falloff;

            //draw_OrientedQuad( m_Origin, m_Beam[i].m_EndPt, vector2(0,0), vector2(4,1), C1, C2, R1, R2 );
    
            vector3 Pos[k_MAX_BEAM_POINTS];
            vector3 SrcPt[3];
            vector3 SrcNrm[3];


            SrcPt[0] = m_Origin;
            SrcPt[1] = m_Origin + ((B.m_EndPt-m_Origin)*0.5f);
            SrcPt[2] = B.m_EndPt;
            SrcNrm[0] = m_OriginNormal;
            SrcNrm[1] = SrcPt[2] - SrcPt[0];
            SrcNrm[2] = -B.m_EndNormal;
            
            f32 MaxDist = SrcNrm[1].Length();
            f32 Scale = MIN( MaxDist*2, 1000.0f );

            SrcNrm[0].NormalizeAndScale( Scale );
            SrcNrm[1].NormalizeAndScale( Scale );
            SrcNrm[2].NormalizeAndScale( Scale );

            f32 SwimSpeed = 4.0f;
            f32 SwimScalar = 1.0f;
            f32 Time = m_TotalTime + B.m_RandomOffset;
            f32 SwimY = R_25 * x_sin( SwimSpeed * Time * SwimScalar );
            f32 SwimX = R_25 * x_sin( SwimSpeed * Time * SwimScalar * 1.2f );
            SrcNrm[1].RotateY( SwimY );
            SrcNrm[1].RotateX( SwimX );

            SwimY = R_15 * x_sin( SwimSpeed * Time * SwimScalar * 2.1f );
            SwimX = R_15 * x_cos( SwimSpeed * Time * SwimScalar * 1.7f );
            SrcNrm[2].RotateY( SwimX );
            SrcNrm[2].RotateX( SwimY );

            T = 1.0f;

            BuildPoints( Pos, k_MAX_BEAM_POINTS, SrcPt, SrcNrm, 3, T );

            EndPt[ i ] = Pos[k_MAX_BEAM_POINTS-1];
            ASSERT( EndPt[i].IsValid() );
            if (!EndPt[i].IsValid())
                EndPt[i] = m_Origin;
            //C1.A = 255;
            
            draw_OrientedStrand( Pos, k_MAX_BEAM_POINTS, vector2(0,0), vector2(1,1), C1,C1, s_LashSizes[ LASH_EXTEND ] * RadiusScale );
            
        }
        else
        {
            EndPt[i] = m_Origin;
        }
    }
    draw_End();

    for (i=0;i<MAX_BEAMS;i++)
    {
        if (m_Beam[i].m_ImpactFX.Validate())
        {            
            m_Beam[i].m_ImpactFX.Render();
        }
        if (m_Beam[i].m_HeadFX.Validate())
        {            
            m_Beam[i].m_HeadFX.SetTranslation( EndPt[ i ] );
            m_Beam[i].m_HeadFX.Render();
        }
    }
}

//=============================================================================

s32 debris_meson_explosion::CreateNewBeam( void )
{
    s32 i;
    s32 iBeam = -1;
    for (i=0;i<MAX_BEAMS;i++)
    {
        if ((m_Beam[i].m_Timer <= 0) && (!m_Beam[i].m_ImpactFX.Validate()))
        {
            // Here is a useable beam
            iBeam = i;
            break;
        }
    }

    if (iBeam == -1)
        return 0;

    // 
    beam& B = m_Beam[i];
    f32 x = x_frand(-1,1);
    f32 y = x_frand(-1,1);
    f32 z = x_frand(0.01f,1.0f);
    B.m_Dir.Set( x, y, z );
    B.m_Dir.Normalize();

    radian3 Rot(0,0,0);
    m_OriginNormal.GetPitchYaw( Rot.Pitch, Rot.Yaw );

    B.m_Dir.Rotate( Rot );

    vector3 Pt = (B.m_Dir * m_MaxRadius) + m_Origin;

    g_CollisionMgr.RaySetup( GetGuid(), m_Origin, Pt);    
    g_CollisionMgr.SetMaxCollisions(1);            
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES,
                                    object::ATTR_COLLIDABLE,
                                    (object::object_attr)(object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING));

    if (g_CollisionMgr.m_nCollisions > 0)
    {
        Pt = g_CollisionMgr.m_Collisions[0].Point;

        f32 Dist = (Pt - m_Origin).Length();
        if (Dist >= 600.0f)
        {
            // We have a valid beam!
            B.m_Timer = k_BEAM_LIFE_TIME;
            B.m_EndPt = Pt;
            B.m_EndNormal = g_CollisionMgr.m_Collisions[0].Plane.Normal;
            B.m_RandomOffset = x_frand(-PI,PI);
       
            rhandle<char> FxResource;    
            FxResource.SetName( PRELOAD_FILE("Spark_002.fxo") );
            const char* pScrewedUpName = FxResource.GetPointer();
            //pScrewedUpName = NULL;
            if ( pScrewedUpName )
            {
                radian3 Rot(0,0,0);
                g_CollisionMgr.m_Collisions[0].Plane.Normal.GetPitchYaw( Rot.Pitch, Rot.Yaw );
                Rot.Pitch *= -1;
                Rot.Yaw   *= -1;

                if (B.m_ImpactFX.Validate())
                    B.m_ImpactFX.KillInstance();

                B.m_ImpactFX.InitInstance( pScrewedUpName );
                B.m_ImpactFX.SetSuspended( TRUE );
                B.m_ImpactFX.SetTranslation( Pt );
                B.m_ImpactFX.SetRotation( Rot );
                B.m_ImpactFX.SetScale( vector3(2,2,2) );        
            }

            FxResource.SetName( PRELOAD_FILE("MSN_Tenticle_head_only.fxo") );
            pScrewedUpName = FxResource.GetPointer();
            if ( pScrewedUpName )
            {                
                if (B.m_HeadFX.Validate())
                    B.m_HeadFX.KillInstance();

                B.m_HeadFX.InitInstance( pScrewedUpName );
                B.m_HeadFX.SetSuspended( FALSE );
                B.m_HeadFX.SetTranslation( m_Origin );                
                B.m_HeadFX.SetScale( vector3(0.6f,0.6f,0.6f) );        
            }

            return 1;
        }
    }


    return 0;
}

//=============================================================================

void debris_meson_explosion::AddNewTarget( guid NewTarget )
{
    if (m_nLashTargets >= MAX_LASH_TARGETS)
        return;

    object* pObj = g_ObjMgr.GetObjectByGuid( NewTarget );
    if (NULL == pObj)
        return;
    
    lash_target& T = m_LashTargets[ m_nLashTargets++ ];

    T.gObj              = NewTarget;
    T.TargetType        = TARGET_NONACTOR_DAMAGEABLE;
    T.bIsValidTarget    = TRUE;    
}

//=============================================================================

#ifndef X_EDITOR

void debris_meson_explosion::net_AcceptUpdate( const bitstream& BS )
{
#ifdef DEEP_LOGGING
    CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::net_AcceptUpdate","DirtyBits %08X",m_NetDirtyBits);
    LOG_FLUSH();
#endif

    if( BS.ReadFlag() )
    {
        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::net_AcceptUpdate", 
            "ACTIVATE_BIT" );

        u16 Zones = 0;

        BS.ReadRangedS32( m_NetSlotOfOwner, -1, 31 );
        BS.ReadVector( m_Origin );
        BS.ReadVector( m_OriginNormal );
        BS.ReadU16( Zones );

        if (!m_bActivated)
        {
            m_GuidOfOwner = NULL_GUID;
            if( IN_RANGE( 0, m_NetSlotOfOwner, 31 ) )
            {
                actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( m_NetSlotOfOwner );

                if( pActor )
                { 
                    m_GuidOfOwner = pActor->GetGuid();
                }
            }

            
            SetZones( Zones );

            CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::net_AcceptUpdate", 
                "ACTIVATE_BIT causing DumbCreate to be called" );
            DumbCreate( m_Origin, m_OriginNormal, Zones );

            m_bActivated = TRUE;
        }
        else
        {
            CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::net_AcceptUpdate", 
                "ACTIVATE_BIT redundant" );
        }
    }

    if (BS.ReadFlag() )
    {
        m_NetDirtyBits |= DIRTY_TARGETING_INFO;

        // DIRTY_TARGETING_INFO
        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::net_AcceptUpdate", 
            "DIRTY_TARGETING_INFO" );

        BS.ReadRangedS32( m_nLashTargets, 0, MAX_LASH_TARGETS );

        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::net_AcceptUpdate", 
            "#Targets in stream: %d",m_nLashTargets );

        s32 i;
        for (i=0;i<m_nLashTargets;i++)
        {
            lash_target& T          = m_LashTargets[ i ];
            s32          NetSlot    = -1;
            xbool        bValid     = FALSE;
            s32          Type       = 0;
            s32          iBone      = 0;
            s32          LifeSeq    = 0;

            T.gObj = NULL_GUID;

            BS.ReadS32( NetSlot );
            BS.ReadRangedS32( Type, 0, TARGET_TYPE_MAX );
            BS.ReadRangedS32( iBone, S8_MIN, S8_MAX );
            BS.ReadFlag( bValid );
            BS.ReadRangedS32( LifeSeq, 0, 7 );

            T.bIsValidTarget = bValid;
            T.TargetType     = (target_type)Type;
            T.ThetaBoneIndex = iBone;
            T.NetActorLifeSeq = LifeSeq;

            if (NetSlot != -1)
            {                
                netobj* pObj = NetObjMgr.GetObjFromSlot( NetSlot );
                if (pObj)
                {
                    T.gObj = pObj->GetGuid();
                }
            }
            else
            {
                T.gObj = NULL_GUID;
            }

            CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::net_AcceptUpdate", 
                "Target %d: Slot %d, Type %d, iBone %d, Valid %s",
                i, NetSlot, Type, iBone, bValid?"TRUE":"FALSE" );
        }

        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::net_AcceptUpdate", 
            "Switching to kill target state" );
        SwitchState( STATE_KILLING_TARGETS );
    }

    if( BS.ReadFlag() )
    {
        m_NetDirtyBits |= DIRTY_NEXT_TARGET_ID;

        // DIRTY_NEXT_TARGET_ID
        BS.ReadRangedS32( m_iNetCurrentTarget, 0, MAX_LASH_TARGETS );        

        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::net_AcceptUpdate", 
            "DIRTY_NEXT_TARGET_ID: %d",m_iNetCurrentTarget);
    }

    if( BS.ReadFlag() )
    {
        m_NetDirtyBits |= DIRTY_COLLAPSE;

        // DIRTY_COLLAPSE
        m_bNetCollapseSignaled = TRUE;
        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::net_AcceptUpdate", 
            "DIRTY_COLLAPSE" );
    }

    if( BS.ReadFlag() )
    {
        // DIRTY_TARGET_STATUS_INFO
        m_NetDirtyBits |= DIRTY_TARGET_STATUS_INFO;

        u16 Data = 0;
        BS.ReadU16( Data, 8 );
        m_NetKilledMask                 = (u8)(Data & 0xFF);
        m_bNetDumbTargetStatusSignaled  = TRUE;

        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::net_AcceptUpdate", 
            "DIRTY_TARGET_STATUS_INFO" );
    }

    BS.ReadMarker();
}

//=============================================================================

void debris_meson_explosion::net_ProvideUpdate( bitstream& BS, 
                                                u32&       DirtyBits )
{
#ifdef DEEP_LOGGING
    CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::net_ProvideUpdate","DirtyBits %08X",m_NetDirtyBits);
    LOG_FLUSH();
#endif
    if( BS.WriteFlag( DirtyBits & netobj::ACTIVATE_BIT ) )
    {
        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::net_ProvideUpdate", 
            "ACTIVATE_BIT");

        u16 Zones = GetZones();
        BS.WriteRangedS32( m_NetSlotOfOwner, -1, 31 );  
        BS.WriteVector( m_Origin );
        BS.WriteVector( m_OriginNormal );
        BS.WriteU16( Zones );
    
        DirtyBits &= ~netobj::ACTIVATE_BIT;      
        //DirtyBits |= DIRTY_ALL;
    }

    if ( BS.WriteFlag( DirtyBits & debris_meson_explosion::DIRTY_TARGETING_INFO ) )
    {   
        DirtyBits &= ~debris_meson_explosion::DIRTY_TARGETING_INFO;
        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::net_ProvideUpdate", 
            "DIRTY_TARGETING_INFO");

        BS.WriteRangedS32( m_nLashTargets, 0, MAX_LASH_TARGETS );
        s32 i;
        for (i=0;i<m_nLashTargets;i++)
        {
            lash_target& T = m_LashTargets[ i ];
            object* pObj = g_ObjMgr.GetObjectByGuid( T.gObj );
            s32 NetSlot = -1;
            s32 ThetaBoneIndex = T.ThetaBoneIndex;
            if (pObj)
            {
                NetSlot = pObj->net_GetSlot();
            }

            BS.WriteS32( NetSlot );
            BS.WriteRangedS32( T.TargetType, 0, TARGET_TYPE_MAX );
            BS.WriteRangedS32( ThetaBoneIndex, S8_MIN, S8_MAX );
            BS.WriteFlag( T.bIsValidTarget );
            BS.WriteRangedS32( T.NetActorLifeSeq, 0, 7 );

            CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::net_ProvideUpdate", 
                "Target %d: Slot %d, Type %d, iBone %d, Valid %s",
                i, NetSlot, T.TargetType, ThetaBoneIndex, T.bIsValidTarget?"TRUE":"FALSE" );
        }
    }

    if ( BS.WriteFlag( DirtyBits & debris_meson_explosion::DIRTY_NEXT_TARGET_ID ) )
    {
        BS.WriteRangedS32( m_iNetCurrentTarget, 0, MAX_LASH_TARGETS );
        DirtyBits &= ~debris_meson_explosion::DIRTY_NEXT_TARGET_ID;
        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::net_ProvideUpdate", 
                                      "DIRTY_NEXT_TARGET_ID: %d",m_iNextTarget);
    }

    if ( BS.WriteFlag( DirtyBits & debris_meson_explosion::DIRTY_COLLAPSE ) )
    {
        DirtyBits &= ~debris_meson_explosion::DIRTY_COLLAPSE;
        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::net_ProvideUpdate", 
            "DIRTY_COLLAPSE");
    }

    if ( BS.WriteFlag( DirtyBits & debris_meson_explosion::DIRTY_TARGET_STATUS_INFO ) )
    {
        u16 Data = m_NetKilledMask;
        BS.WriteU16( Data, 8 );
        DirtyBits &= ~debris_meson_explosion::DIRTY_TARGET_STATUS_INFO;
        CLOG_MESSAGE( ENABLE_LOGGING, "debris_meson_explosion::net_ProvideUpdate", 
            "DIRTY_TARGET_STATUS_INFO");
    }

    

    BS.WriteMarker();
}


//=============================================================================

#endif // X_EDITOR

//=============================================================================

//=============================================================================
