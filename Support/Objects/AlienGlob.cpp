//=============================================================================
//  ALIENGLOB.CPP
//=============================================================================
//
// Throttle the number in the air at any one time
// Add smarter jump when lined up with target
// Separate render ball from collision ball
// Add merging of smaller into larger
// Hook up pain entry
// Make render radius animate
// Shatter them with shotgun
// Back away from the player
// 
//=============================================================================
 
//#define ENABLE_JUMP_THROTLE
 
//=============================================================================
// INCLUDES
//=============================================================================
#include "alienglob.hpp"
#include "e_Draw.hpp"
#include "audiomgr\AudioMgr.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "player.hpp"
#include "animsurface.hpp"
#include "turret.hpp"
#include "Group.hpp"
#include "Player.hpp"
#include "Decals\DecalMgr.hpp"
#include "Debris\debris_meson_lash.hpp"

//=============================================================================
// TWEAKS
//=============================================================================
static f32 GRAVITY_MULTIPLIER       = 1.0f;

//static f32 COL_CHECK_RADIUS         = 40.0f;
static f32 MAX_SPEED                = 1200.0f;
static f32 ALIEN_GRAVITY            = 980.0f * GRAVITY_MULTIPLIER;
static f32 NORMAL_GRAVITY           = 980.0f;
static f32 COLL_RADIUS              = 20.0f;
static f32 BIG_RENDER_RADIUS        = 30.0f;
//static f32 SMALL_RENDER_RADIUS      = 8.0f;
//static f32 MIN_JUMP_SPEED           = 600.0f;
//static f32 MAX_JUMP_SPEED           = 800.0f;
static f32 MIN_JUMP_DELAY           = 0.125f;
static f32 MAX_JUMP_DELAY           = 0.5f;
static f32 ZIG_ZAG_ANGLE            = R_40;
static f32 RENDER_CHASE_RATE        = 0.33f;
static f32 AIRTIME_KILL_TIME        = 5.0f;
static f32 JUMP_NORMAL_GRAVITY_DIST = 50.0f;
static f32 CALM_DOWN_NEAR           = 0.0f;
static f32 CALM_DOWN_FAR            = 600.0f;
static f32 CLOSE_TO_TARGET_DIST     = 30.0f;
static f32 SIGHT_DIST               = 400.0f * 8;
//static s32 NUM_CHILDREN             = 16;
static f32 ATTACK_TIME              = 0.8f;
//static f32 FEARLESS_TIME_FOR_TINY_GLOB = 3.0f;

//static f32 MAX_DIST_TO_LEADER       = 600.0f;
//static f32 MIN_DIST_BETWEEN_LEADER  = 400.0f;

//=============================================================================
// CONSTANTS
//=============================================================================

const char*     k_GlobLogicalName[] = { "GLOB_0", "GLOB_1", "GLOB_2", "GLOB_3" };
const char*     k_GlobFXName[]      = { "npc_alien_blob_small.fxo",
                                        "npc_alien_blob_medium.fxo",
                                        "npc_alien_blob_large.fxo",
                                        "npc_alien_blob_jumbo.fxo" };

const char*     k_GlobEndOfLifeFX[]       = { "Parasite_Impact_000.fxo",
                                              "ubertheta_spore_hit.fxo",
                                              "npc_alien_blob_impact.fxo"};

struct glob_tweak
{
    tweak_handle        hLifespan;
    tweak_handle        hMinChildren;
    tweak_handle        hMaxChildren;
    tweak_handle        hAttackDistance;
    tweak_handle        hMinSpeed;
    tweak_handle        hMaxSpeed;
    tweak_handle        hRandomLeapChance;
    tweak_handle        hMaxJumpAngle;
    tweak_handle        hRadius;
    tweak_handle        hFlockDistToLeader;
    tweak_handle        hFlockDistBetweenLeader;
};

static glob_tweak       s_GlobTweaks[4];
static tweak_handle     s_TweakScatterVelocity      ( "GLOB_Scatter_Velocity"            );
static tweak_handle     s_TweakCollRadiusMultiplier ( "GLOB_Collision_Radius_Multiplier" );
static tweak_handle     s_TweakInvulnerableTime     ( "GLOB_Invulnerable_Time"           );
static tweak_handle     s_TweakFearlessTimeStageZero( "GLOB_0_FearlessTime"              );

alien_glob_mgr  g_AlienGlobMgr;


//=============================================================================
// OBJECT DESC.
//=============================================================================
static struct alien_glob_desc : public object_desc
{
    alien_glob_desc( void ) : object_desc( 
            object::TYPE_ALIEN_GLOB, 
            "Alien Glob",
            "AI",

            object::ATTR_COLLIDABLE             |
            object::ATTR_BLOCKS_ALL_PROJECTILES |
            object::ATTR_RENDERABLE             |
            object::ATTR_TRANSPARENT            |
            object::ATTR_NEEDS_LOGIC_TIME       |
            object::ATTR_DAMAGEABLE             |
            object::ATTR_LIVING                 |
            object::ATTR_SPACIAL_ENTRY,

            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_IS_DYNAMIC            |
            FLAGS_NO_ICON) {}
    
    virtual object* Create( void ) { return new alien_glob; }
    
} s_alien_glob_desc ;

//=============================================================================================

const object_desc&  alien_glob::GetTypeDesc ( void ) const
{
    return s_alien_glob_desc;
}

//=============================================================================================

const object_desc&  alien_glob::GetObjectType ( void )
{
    return s_alien_glob_desc;
}

//=============================================================================================
//=============================================================================================
//=============================================================================================




//==-------------------------------------------------------------------------------------
//  SOUNDS
//
//  NOTE - these must be in the same order as the "glob_audio" enum in the manager above
//==-------------------------------------------------------------------------------------
const char* alien_glob_mgr::m_pAudioIdentifier[ alien_glob_mgr::GLOB_NUM_AUDIO_TYPES ] = 
                                                 { "Blob_Vox_Alert",
                                                   "Blob_Vox_Idle_Loop",
                                                   "Blob_Jump",
                                                   "Blob_Vox_MeleeAttack",
                                                   "Blob_Vox_Death",
                                                   "Blob_Split",
                                                   "Blob_Vox_Flee",
                                                   "Blob_Land",
                                                   "Blob_Jump" };



//==-------------------------------------------------------------

alien_glob_mgr::alien_glob_mgr()
{
    x_memset( m_pJumping, 0, sizeof(alien_glob*) * 32 );
    m_nMaxJumping = 16;

    s32 i;
    for (i=0;i<GLOB_NUM_AUDIO_TYPES;i++)
    {
        s32 j;
        for (j=0;j<MAX_AUDIO_VOICES;j++)
        {
            m_VoiceData[i].VoiceID[j] = -1;
            m_VoiceData[i].Position[j].Set(0,1e20f,0);
        }
    }

    
    for (i=0;i<4;i++)
    {
        s_GlobTweaks[i].hAttackDistance.SetName         ( xfs("GLOB_%d_AttackDistance",i));
        s_GlobTweaks[i].hMinChildren.SetName            ( xfs("GLOB_%d_MinChildren",i));
        s_GlobTweaks[i].hMaxChildren.SetName            ( xfs("GLOB_%d_MaxChildren",i));
        s_GlobTweaks[i].hLifespan.SetName               ( xfs("GLOB_%d_Lifespan",i));
        s_GlobTweaks[i].hMinSpeed.SetName               ( xfs("GLOB_%d_MinSpeed",i));
        s_GlobTweaks[i].hMaxSpeed.SetName               ( xfs("GLOB_%d_MaxSpeed",i));
        s_GlobTweaks[i].hRandomLeapChance.SetName       ( xfs("GLOB_%d_RandomLeapChance",i));
        s_GlobTweaks[i].hMaxJumpAngle.SetName           ( xfs("GLOB_%d_MaxJumpAngle",i));
        s_GlobTweaks[i].hRadius.SetName                 ( xfs("GLOB_%d_Radius",i));
        s_GlobTweaks[i].hFlockDistBetweenLeader.SetName ( xfs("GLOB_%d_FlockDistBetweenLeader",i));
        s_GlobTweaks[i].hFlockDistToLeader.SetName      ( xfs("GLOB_%d_FlockDistToLeader",i));
    }

    m_CurrentThinker = NULL_GUID;
}

//==-------------------------------------------------------------

void alien_glob_mgr::Advance( f32 DeltaTime )
{    
    object* pObj = g_ObjMgr.GetObjectByGuid( m_CurrentThinker );
    if (NULL == pObj)
    {
        slot_id First = g_ObjMgr.GetFirst( object::TYPE_ALIEN_GLOB );
        if (SLOT_NULL == First)
        {
            // There are no globs
            return;
        }

        pObj = g_ObjMgr.GetObjectBySlot( First );
        ASSERT( pObj );
        if (NULL == pObj)
            return;

        m_CurrentThinker = pObj->GetGuid();       
    }

    ASSERT( pObj->IsKindOf( alien_glob::GetRTTI() ));

    alien_glob& Glob = alien_glob::GetSafeType( *pObj );

    Glob.OnThink( DeltaTime );

    slot_id Slot = g_ObjMgr.GetNext( Glob.GetSlot() );
    m_CurrentThinker = NULL_GUID;
    if (Slot != SLOT_NULL)
    {
        pObj = g_ObjMgr.GetObjectBySlot( Slot );
        if (pObj)
        {
            m_CurrentThinker = pObj->GetGuid();
        }
    }
}

//==-------------------------------------------------------------

xbool alien_glob_mgr::RequestJumpPermission( alien_glob* pGlob )
{
#ifndef ENABLE_JUMP_THROTLE
    return TRUE;
#endif
    s32 i;

#ifdef X_DEBUG
    for (i=0;i<32;i++)
    {
        ASSERT( m_pJumping[i] != pGlob );
    }
#endif

    for (i=0;i<m_nMaxJumping;i++)
    {
        if (m_pJumping[i] == NULL)
        {
            m_pJumping[i] = pGlob;
            return TRUE;
        }
    }
    return FALSE;
}

//==-------------------------------------------------------------

void alien_glob_mgr::ConfirmLanding( alien_glob* pGlob )
{
#ifndef ENABLE_JUMP_THROTLE
    return;
#endif

    s32 i;
    for (i=0;i<32;i++)
    {
        if (m_pJumping[i] == pGlob)
            m_pJumping[i] = NULL;
    }
}

//==-------------------------------------------------------------

void alien_glob_mgr::RequestAudio( alien_glob* pGlob, glob_audio Type )
{
    if (Type == GLOB_AUDIO_IDLE)
    {
        // We don't play idles, but its better to be safe
        return;
    }

    voice_data& VD = m_VoiceData[ Type ];

    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if (NULL == pPlayer)
        return;

    vector3 PlayerPos = pPlayer->GetL2W().GetTranslation();
    f32     DistToRequest = (pGlob->GetPosition() - PlayerPos).LengthSquared();

    s32 i;
    s32 iSlot = -1;
    // See if we can find an empty slot first.
    // This is preferable to killing another voice before it is done.
    for (i=0;i<MAX_AUDIO_VOICES;i++)
    {
        if (!g_AudioMgr.IsValidVoiceId( VD.VoiceID[ i ] ))
        {
            iSlot = i;
            break;
        }
    }
    if (iSlot == -1)
    {
        for (i=0;i<MAX_AUDIO_VOICES;i++)
        {
            f32 Dist = (VD.Position[ i ] - PlayerPos).LengthSquared();
            if (Dist > DistToRequest)
            {
                iSlot = i;
                break;
            }
        }
    }
    
    if ((iSlot >= 0) && (iSlot < MAX_AUDIO_VOICES))
    {
        if (g_AudioMgr.IsValidVoiceId( VD.VoiceID[ iSlot ] ))
        {
            g_AudioMgr.Release( VD.VoiceID[ iSlot ], 0 );
        }        
        VD.VoiceID[ iSlot ] = g_AudioMgr.Play( m_pAudioIdentifier[ Type ], pGlob->GetPosition(), pGlob->GetZone1(), TRUE );
        VD.Position[ iSlot ] = pGlob->GetPosition();
    }
}

//=============================================================================================
//=============================================================================================
//=============================================================================================



//=============================================================================================
alien_glob::alien_glob()
{
    m_DeltaTime         = 0;
    m_CollRadius         = COLL_RADIUS;
    m_RenderRadius      = BIG_RENDER_RADIUS;
    m_LifeSpan          = 10000.0f;
    m_TargetGuid        = 0;
    m_Age               = 0;
    m_JumpDelay         = 0;
    m_Color             = XCOLOR_WHITE;
    m_DesiredColor      = m_Color;
    m_Velocity.Zero();

    Land( vector3(0,0,0), vector3(0,1,0), NULL_GUID, FALSE );
    m_bHasLanded = FALSE;

    m_Position.Zero();
    m_DesiredPosition.Zero();
    m_OldPosition.Zero();
    m_RenderPosition.Zero();
    m_DesiredRenderPosition.Zero();

    m_RadiusAnimRate        = x_frand(0.5f,1.5f);

    m_InvulnerableTime      = 0;
    m_Stage                 = 3;
    m_JumpDelayBaseTime     = x_frand(MIN_JUMP_DELAY,MAX_JUMP_DELAY);

    m_bCommitedToAttack     = FALSE;
    m_bMustJumpImmediately  = FALSE;
    m_bActive               = TRUE;
    m_bFleeFromTarget       = FALSE;
    m_State                 = STATE_CHASE;
    m_gGroupContainer       = NULL_GUID;  
    m_bEndOfLife            = FALSE;
    
    m_LeaderGuid            = NULL_GUID;
    m_bIsLeader             = FALSE;
    m_bInvulnerable         = FALSE;

    m_gNotifyOnSpawn        = NULL_GUID;

    SetStage( 3 );
}

//=============================================================================================

alien_glob::~alien_glob()
{
 
}

//=============================================================================

void alien_glob::OnInit( void )
{
    
}

//=============================================================================================

bbox alien_glob::GetLocalBBox( void ) const 
{ 
    bbox BBox( vector3(0,0,0), 50.0f );
    return BBox;
}

//=============================================================================================

void alien_glob::OnAdvanceLogic( f32 DeltaTime )
{
    // If we're waiting to die just return
    if( GetAttrBits() & ATTR_DESTROY )
        return;

    if (!m_bActive)
        return;

    if (m_bEndOfLife)
    {
        // We're dying.  Only the death fx matter now.  Once they are done
        // so are we.
        m_hEndOfLifeFX.AdvanceLogic( DeltaTime );
        if (m_hEndOfLifeFX.IsFinished())
        {
            g_ObjMgr.DestroyObject( GetGuid() );
            return;
        }
        return;
    }

    m_hNucleusFX.AdvanceLogic( DeltaTime );

    m_DeltaTime = DeltaTime;

    // Update AI
    if (m_State == STATE_IDLE)
    {
        if (!UpdateIdleAI())
            return;
    }
    else
    {
        if( !UpdateAI() )
            return;
    }

    if( m_bHasLanded==FALSE )
    {
        BackupOldVelocities();
        UpdatePhysics();
        UpdateCollision();
        if( GetAttrBits() & ATTR_DESTROY )
            return;
        UpdateRenderPosition();
        CalculateNewVelocities();
        OnMove( m_Position );
    }

    UpdateRenderPosition();

    if (m_Stage > 0)
        m_FloorProperties.Update( GetPosition(), DeltaTime, TRUE );
        
    UpdateColor();
}

//=============================================================================================

xbool alien_glob::UpdateInAir( void )
{
    // If in air then update airtime
    if (( m_bHasLanded==FALSE ))
    {
        m_AirTime += m_DeltaTime;

        if( m_AirTime > AIRTIME_KILL_TIME )
        {
            StartEndOfLife( EOL_DEATH );
            return FALSE;
        }
    }

    // Check if we have fallen too far below our relative ground.
    // If we have then switch to normal gravity
    if( m_bHasLanded == FALSE )
    {
        f32 Altitude = m_GroundNormal.Dot( m_Position );
        if( (Altitude - m_JumpStartAltitude) < -JUMP_NORMAL_GRAVITY_DIST )
        {
            m_JumpStartAltitude = 0;
            m_GroundNormal = vector3(0,1,0);
            m_Gravity = vector3(0,-NORMAL_GRAVITY,0);
        }
    }

    return TRUE;
}


xbool alien_glob::UpdateIdleAI( void )
{
    // If we're waiting to die just return
    if( GetAttrBits() & ATTR_DESTROY )
        return FALSE;

    // Update invulnerability
    m_InvulnerableTime -= m_DeltaTime;
    if( m_InvulnerableTime < 0 )
        m_InvulnerableTime = 0;


    // Update Age only if we are not idle
    if (m_State != STATE_IDLE)
    {
        m_Age += m_DeltaTime;
        if( m_Age >= m_LifeSpan )
        {
            StartEndOfLife( EOL_DEATH );            
            return FALSE;
        }
    }

    if (m_bHasLanded)
    {
        m_JumpDelay -= m_DeltaTime;
        if (( m_JumpDelay <= 0 ) || (m_bMustJumpImmediately))
        {
            xbool bCanJump = FALSE;
            if (m_bMustJumpImmediately)
            {
                // Bypass the manager, we have to jump
                bCanJump = TRUE;                
            }
            else if (g_AlienGlobMgr.RequestJumpPermission( this ))
            {
                bCanJump = TRUE;
            }
            
            if (bCanJump)
            {
                vector3 Dir = m_GroundNormal;
                Dir.NormalizeAndScale( 150 );
                vector3 Pos = GetPosition() + Dir;
                radian P,Y;
                Dir.GetPitchYaw( P,Y );
                vector3 Delta(x_frand(-10,10), x_frand(-10,10), 50);
                Delta.Rotate( radian3(P,Y,0) );

                Pos += Delta;
                            
                Jump( Pos, TRUE, FALSE );
            }
        }
    }

    return UpdateInAir();
}

//=============================================================================================

xbool alien_glob::UpdateAI( void )
{
    // If we're waiting to die just return
    if( GetAttrBits() & ATTR_DESTROY )
        return FALSE;

    // Update Age
    {
        m_Age += m_DeltaTime;
        if( m_Age >= m_LifeSpan )
        {
            StartEndOfLife( EOL_DEATH );            
            return FALSE;
        }
    }
    
    if ((m_Stage == 0) && (!m_bFleeFromTarget) && (m_Age >= s_TweakFearlessTimeStageZero.GetF32()))
    {
        m_bFleeFromTarget = TRUE;
    }

    // Update invulnerability
    m_InvulnerableTime -= m_DeltaTime;
    if( m_InvulnerableTime < 0 )
        m_InvulnerableTime = 0;
    
    // If no target guid then choose the player
    if( m_TargetGuid==0 )
    {
        slot_id PlayerSlot = g_ObjMgr.GetFirst( object::TYPE_PLAYER ) ;
        object* pObject = g_ObjMgr.GetObjectBySlot( PlayerSlot );
        if( pObject )
            m_TargetGuid = pObject->GetGuid();
    }

    // Check if we have a target
    object* pObject = g_ObjMgr.GetObjectByGuid( m_TargetGuid );
    if( pObject )
    {
        // Do we have traction?
        if( m_bHasLanded )
        {
            // Decrement jump delay and take a jump!
            m_JumpDelay -= m_DeltaTime;
            if (( m_JumpDelay <= 0 ) || (m_bMustJumpImmediately))
            {
                vector3 TargetPos = pObject->GetPosition() + vector3(0,200,0);

                // Decide target's position on current plane
                plane Plane;
                vector3 MyPos = GetPosition();
                Plane.Setup( MyPos, m_GroundNormal );

                f32 DistToPlane = Plane.Distance( TargetPos );
                vector3 ClosestPtOnPlane = TargetPos - (m_GroundNormal * DistToPlane);

                m_TargetPosOnPlane = ClosestPtOnPlane;

                vector3 Delta = ClosestPtOnPlane - MyPos;
                vector3 WorldDelta = TargetPos - MyPos;
                f32     Dist       = Delta.Length();
                f32     WorldDist  = WorldDelta.Length();
                f32     AttackDist = s_GlobTweaks[m_Stage].hAttackDistance.GetF32();

                if( !m_bFleeFromTarget )
                {
                    if (WorldDist < AttackDist)
                    {
                        // Commit to the attack
                        m_bCommitedToAttack = TRUE;
                        SetupAttackData();
                        vector3 AttackPos = GetAttackDestination();
                        Jump( AttackPos, TRUE, TRUE );
                    }
                    else if (Dist < AttackDist)
                    {
                        // Help get off of walls and surfaces
                        vector3 AttackPos = GetAttackDestination();
                        Jump( AttackPos, TRUE, TRUE );
                    }
                    else
                    {
                        if( (m_Stage < 3) || (Dist < SIGHT_DIST) || (m_bMustJumpImmediately))
                        {
                            xbool bCanJump = FALSE;
                            if (m_bMustJumpImmediately)
                            {
                                // Bypass the manager, we have to jump
                                bCanJump = TRUE;                
                            }
                            else if (g_AlienGlobMgr.RequestJumpPermission( this ))
                            {
                                bCanJump = TRUE;
                            }

                            if (bCanJump)
                            {
                                // Boing!
                                if( (Dist>CLOSE_TO_TARGET_DIST) )
                                    Jump( pObject->GetPosition() + vector3(0,200,0), TRUE, FALSE );
                                else
                                    Jump( pObject->GetPosition() + vector3(0,200,0), TRUE, TRUE );
                            }
                        }
                        else if (Dist < AttackDist)
                        {
                            // psuedo-"Attack". This occurs when the glob
                            // is on a surface (usually something other than the floor)
                            // and is close enough to the target position when projected onto 
                            // the globs ground plane.
                            vector3 AttackPos = GetAttackDestination();
                            Jump( AttackPos, TRUE, TRUE );
                        }                        
                    }
                }
                else
                {
                    Jump( TargetPos, FALSE, FALSE );
                }
            }
        }
    }
    else
    {
        // Couldn't get target object pointer, so zero out the guid
        m_TargetGuid = NULL_GUID;
    }

    if (m_bCommitedToAttack)
    {
        m_AttackData.AttackTimer -= m_DeltaTime; 
        if (m_AttackData.AttackTimer < 0)
        {
            // Time to die
            StartEndOfLife( EOL_ATTACK_IMPACT );      
            return FALSE;
        }
    }
    else    // Handle things we can perform while not attacking
    {
        if (!UpdateInAir())
            return FALSE;
    }

    return TRUE;
}

//=============================================================================================

void alien_glob::SetupAttackData( void )
{
    m_AttackData.LeapOrigin  = GetPosition();
    m_AttackData.AttackTimer = ATTACK_TIME;
}

//=============================================================================================

vector3 alien_glob::GetAttackDestination( void )
{
    vector3 Ret = GetPosition();

    object_ptr<actor> pActor( m_TargetGuid );
    if ((object_ptr<actor>)NULL == pActor)
        return Ret;

    Ret = pActor->GetPositionWithOffset( actor::OFFSET_AIM_AT );

    return Ret;
}

//=============================================================================================

void alien_glob::UpdatePhysics( void )
{
    ASSERT( m_bHasLanded == FALSE );

    if (m_bCommitedToAttack)
    {
        // When attacking, lerp directly to the attack point
        f32 T = m_AttackData.AttackTimer / ATTACK_TIME;

        T = 1.0f - MIN(1,MAX(0,T));

        vector3 Dest = GetAttackDestination();
        vector3 Delta = Dest - m_AttackData.LeapOrigin;
        Delta *= T;
        m_Velocity.Zero();
        m_DesiredPosition = m_AttackData.LeapOrigin + Delta;
    }
    else
    {
        // Apply gravity.
        m_Velocity += m_Gravity * m_DeltaTime;

        // Update positions
        m_DesiredPosition = m_Position + m_Velocity * m_DeltaTime;
    }
}

//=============================================================================================

void alien_glob::UpdateCollision( void )
{
    vector3 DeltaPos = m_DesiredPosition - m_Position;
    f32     Speed = DeltaPos.Length();
    if( Speed < 0.01f )
        return;

    f32 CollRadius = m_CollRadius;
    if (m_bCommitedToAttack)
        CollRadius = 5.0f;

    g_CollisionMgr.UseLowPoly();
    g_CollisionMgr.SphereSetup( GetGuid(), m_Position, m_DesiredPosition, CollRadius );
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                                   (object::object_attr)(object::ATTR_BLOCKS_CHARACTER | object::ATTR_BLOCKS_CHARACTER_LOS),
                                   (object::object_attr)(object::ATTR_COLLISION_PERMEABLE) );

    // If there were no collisions then just return desired
    if( g_CollisionMgr.m_nCollisions == 0)
    {
        m_Position = m_DesiredPosition;
    }
    else
    {
        guid gObj = g_CollisionMgr.m_Collisions[0].ObjectHitGuid;


        // Process the collision
        vector3 Dir   = DeltaPos / Speed;
        f32 DistanceToMove = (Speed * g_CollisionMgr.m_Collisions[0].T) - 0.01f;
        m_Position = m_Position + DistanceToMove*Dir;
        Land( m_Position, 
              g_CollisionMgr.m_Collisions[0].Plane.Normal, 
              g_CollisionMgr.m_Collisions[0].ObjectHitGuid );

        if (m_bCommitedToAttack)
        {
            // Check to see if we hit the target
            if (gObj != m_TargetGuid)
            {                
                // We hit something else, switch out of attack mode
                m_bCommitedToAttack = FALSE;                
            }
            else             
            {
                // We hit the target while we were in attack mode
                vector3 PainDir = Dir;

                Dir.Normalize();
                pain Pain;
                pain_handle PainHandle( xfs("GLOB_%d",m_Stage) );
                Pain.Setup( PainHandle, GetGuid(), g_CollisionMgr.m_Collisions[0].Point );
                Pain.SetDirection( PainDir );
                Pain.SetDirectHitGuid( gObj );
                Pain.SetCollisionInfo( g_CollisionMgr.m_Collisions[0] );
                Pain.SetCustomScalar( 1 );
                Pain.ApplyToObject( gObj  );

                StartEndOfLife( EOL_ATTACK_IMPACT );                
            }
        }
    }
}

//=============================================================================================

void alien_glob::UpdateRenderPosition( void )
{
    // Update the desired render position
    if( m_bHasLanded )
    {
        m_DesiredRenderPosition = m_Position - 
                                (m_GroundNormal * m_CollRadius) +
                                (m_GroundNormal * m_RenderRadius);
    }
    else
    {
        m_DesiredRenderPosition = m_Position;
    }

    vector3 Delta = m_DesiredRenderPosition - m_RenderPosition;
    //f32 Len = Delta.Length();
    //if( Len < 0.01f )
    //    return;
    //vector3 Dir = Delta / Len;

    m_RenderPosition += Delta * RENDER_CHASE_RATE;    
}

//=============================================================================================

void alien_glob::BackupOldVelocities( void )
{
    m_OldPosition = m_Position;
}

//=============================================================================================

void alien_glob::CalculateNewVelocities( void )
{
    m_Velocity = (m_Position - m_OldPosition) / m_DeltaTime;

    // Be sure we peg the max velocity
    f32 Speed = m_Velocity.Length();
    if( Speed > MAX_SPEED )
    {
        m_Velocity.NormalizeAndScale( MAX_SPEED );
    }
}

//=============================================================================================

void alien_glob::Scatter( const vector3& Direction, xbool bDeleteSelf, s32 nStagesToDrop )
{
    s32 i;
    s32 NumChildren=0;
    f32 LifeSpan=0;
    f32 JumpDelayTime=0;
    s32 Stage=0;

    if (m_bEndOfLife)
        return;

    if( m_Stage > 1 )
    {
        // Setup stage info
        NumChildren     = x_irand( s_GlobTweaks[m_Stage].hMinChildren.GetS32(),
                                   s_GlobTweaks[m_Stage].hMaxChildren.GetS32() );
        Stage           = m_Stage - nStagesToDrop;
        switch( m_Stage )
        {
        case 3: 
            LifeSpan        = x_frand(300.0f,300.0f);
            JumpDelayTime   = x_frand(0.25f,0.5f);
            break;
        case 2: 
            LifeSpan        = x_frand(300.0f,300.0f);
            JumpDelayTime   = x_frand(0.125f,0.3f);
            break;
        case 1: 
            LifeSpan        = x_frand(10.0f,15.0f);
            JumpDelayTime   = x_frand(0.05f,0.1f);
            break;
        default:
            ASSERT(FALSE);
        }

        object_ptr<debris_meson_explosion>pME( m_gNotifyOnSpawn );

        for( i=0; i<NumChildren; i++ )
        {
            guid Guid = g_ObjMgr.CreateObject( "Alien Glob" );
            if (NULL_GUID != Guid)
            {
                alien_glob* pGlob = (alien_glob*)g_ObjMgr.GetObjectByGuid( Guid );
                if( pGlob )
                {
                    pGlob->m_Position       = m_Position;
                    pGlob->m_OldPosition    = m_OldPosition;
                    pGlob->m_TargetGuid     = m_TargetGuid;                
                    pGlob->m_JumpDelayBaseTime = JumpDelayTime;
                    pGlob->m_bHasLanded     = FALSE;
                    pGlob->SetZones( GetZones() );
                    g_ZoneMgr.InitZoneTracking( *this, pGlob->m_ZoneTracker );
                    pGlob->OnMove( m_Position );

                    // Create random velocity
                    vector3  Velocity(0,0,s_TweakScatterVelocity.GetF32());
                    Velocity.RotateX( x_frand(-R_60,R_60) );
                    Velocity.RotateY( x_frand(R_0,R_360) );
                    Velocity.Rotate( radian3(Direction.GetPitch(),Direction.GetYaw(),0) );

                    pGlob->m_Velocity = Velocity;
                    pGlob->m_LifeSpan = LifeSpan;
                    pGlob->m_Gravity = vector3(0,-NORMAL_GRAVITY,0);

                    pGlob->m_RenderPosition = m_RenderPosition;
                    pGlob->m_DesiredRenderPosition = m_DesiredRenderPosition;
                    pGlob->m_InvulnerableTime = s_TweakInvulnerableTime.GetF32();
                    pGlob->SetStage( Stage );
                    pGlob->m_gGroupContainer = m_gGroupContainer;

                    object_ptr<group> pGroup( m_gGroupContainer );
                    if (pGroup)
                    {
                        pGroup->AddGuid( Guid );
                    }
                }

                if (pME)
                    pME->AddNewTarget( Guid );
            }
        }
    }

    if (bDeleteSelf)
    {
        StartEndOfLife( EOL_SPLITTING );
    }

    g_AlienGlobMgr.RequestAudio( this, alien_glob_mgr::GLOB_AUDIO_SCATTER );
}

//=============================================================================================

void alien_glob::Jump( const vector3& TargetPos, xbool bToward, xbool bNormalGravity )
{
    ASSERT( m_bHasLanded );

    f32 MinJumpSpeed = s_GlobTweaks[m_Stage].hMinSpeed.GetF32();
    f32 MaxJumpSpeed = s_GlobTweaks[m_Stage].hMaxSpeed.GetF32();


    // Decide Jump direction
    vector3 Delta = TargetPos - m_Position;

    f32 Dist = Delta.Length();
    f32 CalmDownT = x_parametric( Dist, CALM_DOWN_NEAR, CALM_DOWN_FAR, TRUE );

    // Rotate delta around normal for zigzag
    {
        radian Angle;
        radian Min = -ZIG_ZAG_ANGLE;
        radian Max =  ZIG_ZAG_ANGLE;

        if ((m_LeaderGuid != NULL_GUID) && (!m_bCommitedToAttack))
        {
            object* pLeader = g_ObjMgr.GetObjectByGuid( m_LeaderGuid );
            
            if (pLeader)
            {
                vector3 LeaderPos = pLeader->GetL2W().GetTranslation();
                
                vector3 LeaderDelta = LeaderPos - m_Position;
                vector3 TargetDelta = Delta;

                LeaderDelta.Normalize();
                TargetDelta.Normalize();

                f32 Dot = MAX(0,LeaderDelta.Dot(TargetDelta));

                plane P(m_Position, m_Position + m_GroundNormal, TargetPos );

                Min = (1.0f - Dot) * R_60;
                Max = R_45 + (1.0f - Dot) * R_35;
                
                if (P.InBack( LeaderPos ))
                {
                    f32 Temp = Min;
                    Min = -Max;
                    Max = -Temp;
                }
            }
        }
    
        Angle = x_frand(Min,Max);
        if (m_bCommitedToAttack)
            Angle = 0;

        if( bToward==FALSE ) Angle += R_180;
        quaternion Q( m_GroundNormal, Angle );
        Delta = Q*Delta;
    }

    vector3 Axis    = m_GroundNormal.Cross(Delta);
                     
    Axis.Normalize();

    radian  MaxAngle = DEG_TO_RAD( s_GlobTweaks[ m_Stage ].hMaxJumpAngle.GetF32() );
    radian  Angle   = x_clamp(v3_AngleBetween( m_GroundNormal, Delta ),R_0,MaxAngle);
    
    if (!m_bCommitedToAttack)
    {
        if (x_frand(0,1) < s_GlobTweaks[m_Stage].hRandomLeapChance.GetF32())
        {
            vector3 Dest = m_GroundNormal;
            Dest.NormalizeAndScale( 1000 );
            Dest += m_Position;
            g_CollisionMgr.LineOfSightSetup( GetGuid(), m_Position, Dest );
            g_CollisionMgr.AddToIgnoreList( GetGuid() );
            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE );
            if (g_CollisionMgr.m_nCollisions > 0)
            {
                m_Gravity = -m_Gravity;
            }
        }
    }

    quaternion Q( Axis, Angle );
    vector3 Velocity = Q*m_GroundNormal;
    Velocity *= CalmDownT * x_frand(MinJumpSpeed,MaxJumpSpeed);

    m_Velocity += Velocity;

    m_bHasLanded    = FALSE;
    m_JumpDelay     = 0;
    m_AirTime       = 0;

    m_JumpStartAltitude = m_GroundNormal.Dot(m_Position);

    if( bNormalGravity )
    {
        m_GroundNormal = vector3(0,1,0);
        m_Gravity      = -m_GroundNormal * NORMAL_GRAVITY;
        m_JumpStartAltitude = m_GroundNormal.Dot(m_Position);
    }

    m_bMustJumpImmediately = FALSE;

    if (m_State != STATE_IDLE)
        g_AlienGlobMgr.RequestAudio( this, alien_glob_mgr::GLOB_AUDIO_JUMP );

    m_JumpDest = TargetPos;
}

//=============================================================================================

void alien_glob::Land( const vector3& Position, const vector3& NewTractionNormal, guid GuidHit, xbool bPlayAudio )
{
    m_bHasLanded        = TRUE;
    m_Position          = Position;
    m_GroundNormal      = NewTractionNormal;
    m_Gravity           = -m_GroundNormal * ALIEN_GRAVITY;
    m_JumpDelay         = m_JumpDelayBaseTime * x_frand(0.75f,1.25f);
    m_AirTime           = 0;

    g_AlienGlobMgr.ConfirmLanding( this );

    object_ptr<object> pObj(GuidHit);
    if (pObj)
    {
        xbool bDanger = FALSE;
        if (pObj->IsKindOf( anim_surface::GetRTTI() ))
            bDanger = TRUE;
        if (pObj->IsKindOf( actor::GetRTTI() ))
            bDanger = TRUE;
        if (pObj->IsKindOf( turret::GetRTTI() ))
            bDanger = TRUE;

        if (bDanger)
        {
            m_JumpDelay = 0;
            m_bMustJumpImmediately = TRUE;
        }
    }

    m_FloorProperties.ForceUpdate( GetPosition() );
/*DECAL
    //
    // create the decal
    //
    decal_package*    pPackage = m_hDecalPackage.GetPointer();
    if ( pPackage && (pPackage->GetNDecalDefs(0)>0) )
    {
        decal_definition& Def = pPackage->GetDecalDef( 0, 0 );        
        vector3 Start = GetPosition();
        vector3 End   = -m_GroundNormal;
        End *= 1000.0f;
        End += Start;
         g_DecalMgr.CreateDecalFromRayCast( Def, Start, End );
    }
*/
    if (m_State == STATE_IDLE)
        bPlayAudio = FALSE;

    if (bPlayAudio)
        g_AlienGlobMgr.RequestAudio( this, alien_glob_mgr::GLOB_AUDIO_LAND );
}

//=============================================================================================

void alien_glob::OnPain( const pain& Pain )
{
    (void)Pain;
    if( GetAttrBits() & ATTR_DESTROY )
        return;

    if (m_bEndOfLife || m_bInvulnerable)
        return;

    if( m_InvulnerableTime == 0 )
    {
/*
        vector3 Direction = Pain.GetDirection();

        plane Plane;
        Plane.Setup( m_GroundNormal, GetPosition() );
        Direction = Plane.ReflectVector(Direction);

        if( Direction.LengthSquared() < 0.01f )
            Direction = m_GroundNormal;
*/
        s32 nStagesToDrop = 1;

        guid gSource = Pain.GetOriginGuid();
        if (NULL_GUID != gSource)
        {
            object* pObj = g_ObjMgr.GetObjectByGuid( gSource );
            if (pObj)
            {
                if (pObj->IsKindOf( debris_meson_explosion::GetRTTI() ))
                    nStagesToDrop = 2;
            }
        }
        Scatter(m_GroundNormal,TRUE,nStagesToDrop);
    }
}

//=============================================================================================

void alien_glob::OnColCheck( void )
{
    if (( m_Stage > 0 ) && (!m_bEndOfLife))
    {
        g_CollisionMgr.StartApply( GetGuid() );
        g_CollisionMgr.ApplySphere( GetPosition(), m_CollRadius*3.0f );
        g_CollisionMgr.EndApply();
    }
}

//=============================================================================================

void alien_glob::OnMove( const vector3& NewPos )
{
    vector3 RenderPositionDelta = m_RenderPosition - m_Position;
    vector3 DesiredRenderPositionDelta = m_DesiredRenderPosition - m_Position;

    m_Position = NewPos;
    m_OldPosition = NewPos;

    m_RenderPosition        = m_Position + RenderPositionDelta;
    m_DesiredRenderPosition = m_Position + DesiredRenderPositionDelta;

    object::OnMove( NewPos );

    // Update zone tracking
    g_ZoneMgr.UpdateZoneTracking( *this, m_ZoneTracker, NewPos );
}

//=============================================================================================

void alien_glob::OnTransform( const matrix4& L2W )
{
    // Call base class
    object::OnTransform(L2W) ;

    // Update zone tracking
    g_ZoneMgr.UpdateZoneTracking( *this, m_ZoneTracker, L2W.GetTranslation() );
}

//=========================================================================

void alien_glob::OnRender( void )
{

    return;

    if (m_bEndOfLife)
        return;

    matrix4 L2W;
    L2W.Identity();
    L2W.SetTranslation( GetPosition() );
}

//=========================================================================

void alien_glob::UpdateColor( void )
{

    // If we are in the air then do full brightness
    if( m_bHasLanded==FALSE )
    {
        m_DesiredColor = xcolor(255,255,255);
    }
    else
    {

        // Get floor brightness
        f32 FloorBrightness;
        {
            xcolor FloorColor = m_FloorProperties.GetColor();
            f32 R = FloorColor.R/255.0f;
            f32 G = FloorColor.G/255.0f;
            f32 B = FloorColor.B/255.0f;
            #ifdef TARGET_PS2
                R *= 2.0f;
                G *= 2.0f;
                B *= 2.0f;
            #endif
            
            FloorBrightness = x_sqrt(R*R + G*G + B*B);
        }

        // Peg FloorBrightness to not allow too dark
        FloorBrightness = MAX( 0.25f, FloorBrightness );
        //ASSERT( (FloorBrightness>=0) && (FloorBrightness<=1));
        if( FloorBrightness > 1.0f ) FloorBrightness = 1.0f;

        // Setup color
        m_DesiredColor.R = (u8)(255.0f * FloorBrightness);
        m_DesiredColor.G = (u8)(255.0f * FloorBrightness);
        m_DesiredColor.B = (u8)(255.0f * FloorBrightness);
    }

    // Blend current color to desired color
    f32 T = 0.125f;
    m_Color.R = (u8)( m_Color.R + T*((f32)m_DesiredColor.R - (f32)m_Color.R) );
    m_Color.G = (u8)( m_Color.G + T*((f32)m_DesiredColor.G - (f32)m_Color.G) );
    m_Color.B = (u8)( m_Color.B + T*((f32)m_DesiredColor.B - (f32)m_Color.B) );
}

//=========================================================================

void alien_glob::OnRenderTransparent( void )
{
    if (m_bEndOfLife)
    {
        matrix4 L2W;
        L2W.Identity();
        L2W.SetTranslation( GetPosition() );        
        m_hEndOfLifeFX.SetTranslation( m_Position );
        m_hEndOfLifeFX.SetRotation( radian3(0,0,0) );
        m_hEndOfLifeFX.Render();
        return;
    }

    xcolor C;
    switch( m_Stage )
    {
    case 3: C = XCOLOR_RED; break;
    case 2: C = XCOLOR_GREEN; break;
    case 1: C = XCOLOR_BLUE; break;
    case 0: C = XCOLOR_YELLOW; break;
    default: ASSERT(FALSE);
    };

    if (m_bCommitedToAttack)
        C.Random();

    //draw_Sphere( m_RenderPosition, m_RenderRadius, C );
    
/*
#if (defined X_EDITOR) && (defined shird)
    draw_Line( GetPosition(), m_TargetPosOnPlane, XCOLOR_YELLOW );
    draw_Sphere( m_TargetPosOnPlane, 5, XCOLOR_YELLOW );

    draw_Line( GetPosition(), m_JumpDest, XCOLOR_GREEN );
    draw_Sphere( m_JumpDest, 5, XCOLOR_GREEN );
#endif
*/


    

    matrix4 L2W;
    L2W.Identity();
    L2W.SetTranslation( GetPosition() );

    m_hNucleusFX.SetColor( m_Color );
    m_hNucleusFX.SetTranslation( m_Position );
    m_hNucleusFX.SetRotation( radian3(0,0,0) );
    //m_hNucleusFX.SetScale( vector3(Scale,Scale,Scale) );
    m_hNucleusFX.Render();

    //draw_Sphere( m_Position, m_CollRadius*3, XCOLOR_RED );

#if (defined X_EDITOR) && (defined shird)
    {        
        xcolor Clr = m_FloorProperties.GetColor();
//#ifdef TARGET_PS2
        Clr += Clr;
//#endif
        draw_Marker( GetPosition() + (m_GroundNormal * m_RenderRadius * 1.5f), Clr );

        if (m_bThinking)
        {
            draw_Marker( GetPosition() + (m_GroundNormal * m_RenderRadius * 3.0f), XCOLOR_RED );
            m_bThinking = FALSE;
        }
        if (m_bIsLeader)
        {
            draw_Marker( GetPosition() + (m_GroundNormal * m_RenderRadius * 2.25), XCOLOR_YELLOW );
            m_bThinking = FALSE;
        }

        draw_ClearL2W();
        if ((m_LeaderGuid != NULL_GUID))// && (!m_bFleeFromTarget))
        {
            object* pObj = g_ObjMgr.GetObjectByGuid( m_LeaderGuid );
            if (pObj)
            {
                draw_Line( m_Position, pObj->GetL2W().GetTranslation(), XCOLOR_WHITE );
            }
        }
    }
#endif
}

//=============================================================================

void alien_glob::OnEnumProp      ( prop_enum&    List )
{
    object::OnEnumProp( List );

    List.PropEnumHeader( "Glob",                "Alien Glob Properties",    0 );
    List.PropEnumButton( "Glob\\Scatter",       "Scatters bones",           PROP_TYPE_EXPOSE | PROP_TYPE_DONT_SHOW | PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_EXPORT | PROP_TYPE_DONT_SAVE_MEMCARD  );
    List.PropEnumGuid  ( "Glob\\Target Guid",   "Target to head towards",   PROP_TYPE_EXPOSE );
    List.PropEnumInt   ( "Glob\\Stage",         "Stage of life",            PROP_TYPE_EXPOSE );
    List.PropEnumBool  ( "Glob\\IsActive",      "Is the glob active?",      PROP_TYPE_EXPOSE );
    List.PropEnumBool  ( "Glob\\IsIdle",        "Is the glob idle?",        PROP_TYPE_EXPOSE );
    List.PropEnumBool  ( "Glob\\IsInvulnerable","Is the glob invulnerable?",PROP_TYPE_EXPOSE );

    List.PropEnumVector3( "Glob\\GroundNormal", "", PROP_TYPE_DONT_SHOW );
    List.PropEnumVector3( "Glob\\Gravity",      "", PROP_TYPE_DONT_SHOW );
    List.PropEnumGuid   ( "Glob\\Group",        "", PROP_TYPE_DONT_SHOW );
    List.PropEnumGuid   ( "Glob\\Target",       "", PROP_TYPE_DONT_SHOW );

    /*DECAL
    List.PropEnumExternal("Glob\\Splat Decal Package",      "Resource\0decalpkg\0", "The package of the splat decal this object will leave behind.", 0 );
    */

#ifdef X_EDITOR
    List.PropEnumButton( "Glob\\Stick To Nearest Surface",  "Attach the glob to the nearest surface", 0 );
#endif
}

//=============================================================================

xbool alien_glob::OnProperty      ( prop_query&   I    )
{
    if( object::OnProperty( I ) )
    {
        // Initialize the zone tracker
        if( I.IsVar( "Base\\Position" )) 
        {
            g_ZoneMgr.InitZoneTracking( *this, m_ZoneTracker );
        }
    }
    else if ( I.VarGUID( "Glob\\Target", m_TargetGuid ))
    {        
    }
    else if ( I.VarGUID( "Glob\\Group", m_gGroupContainer))
    {        
    }
    else if ( I.VarVector3( "Glob\\GroundNormal", m_GroundNormal ))
    {        
    }
    else if ( I.VarVector3( "Glob\\Gravity", m_Gravity))
    {        
    }
#ifdef X_EDITOR
    else if (I.IsVar("Glob\\Stick To Nearest Surface"))
    {
        if (I.IsRead())
        {
            I.SetVarButton("Stick To Nearest Surface");
        }
        else
        {
            StickToNearestSurface();
        }
    }
#endif // X_EDITOR
    else if (I.IsVar("Glob\\IsInvulnerable"))
    {
        if (I.IsRead())
        {
            xbool Temp = m_bInvulnerable;
            I.SetVarBool( Temp );
        }
        else
        {
            m_bInvulnerable = I.GetVarBool();            
        }
    }
    else if (I.IsVar("Glob\\IsActive"))
    {
        if (I.IsRead())
        {
            xbool Temp = m_bActive;
            I.SetVarBool( Temp );
        }
        else
        {
            m_bActive = I.GetVarBool();            
        }
    }
    else if (I.IsVar("Glob\\IsIdle"))
    {
        if (I.IsRead())
        {
            xbool Temp = m_State == STATE_IDLE?TRUE:FALSE;
            I.SetVarBool( Temp );
        }
        else
        {
            state NewState;
            if (I.GetVarBool())
                NewState = STATE_IDLE;
            else
                NewState = STATE_CHASE;            

            if ((NewState != m_State) && (NewState == STATE_CHASE))
            {
                g_AlienGlobMgr.RequestAudio( this, alien_glob_mgr::GLOB_AUDIO_ALERT );
            }

            m_State = NewState;
        }
    }
    else if (I.IsVar("Glob\\Stage"))
    {
        if (I.IsRead()) 
        {
            I.SetVarInt( m_Stage );
        }
        else
        {
            s32 Stage = I.GetVarInt();
            Stage = MIN(3,MAX(0,Stage));

            SetStage( Stage );

            m_Velocity.Zero();
            m_LifeSpan = s_GlobTweaks[m_Stage].hLifespan.GetF32();
            m_InvulnerableTime = 0;            
        }
    }

    else if (I.IsVar("Glob\\Scatter"))
    {
        if (I.IsRead()) I.SetVarButton("SCATTER");
        else            Scatter(vector3(0,0,0));
    }
    else if (I.VarGUID( "Glob\\Target Guid", m_TargetGuid  ))
    {
        // do nothing
    }
/*DECAL
    else if ( I.IsVar( "Glob\\Splat Decal Package" ) )
    {
        if ( I.IsRead() )
        {
            I.SetVarExternal( m_hDecalPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            m_hDecalPackage.SetName( I.GetVarExternal() );
            m_hDecalPackage.GetPointer();
        }

        return TRUE;
    }
*/
    else 
    {
        return FALSE;
    }
    return TRUE;
}

//=========================================================================

const char* alien_glob::GetLogicalName( void )
{
    if ((m_Stage < 0) || (m_Stage > 3))
        return NULL;

    return k_GlobLogicalName[ m_Stage ];
}

//=========================================================================

void alien_glob::OnActivate( xbool Flag )
{
    m_bActive = Flag;
}

//=========================================================================

void alien_glob::OnAddedToGroup( guid gGroup )
{
    m_gGroupContainer = gGroup;
}

//=========================================================================

void alien_glob::SetStage( s32 iNewStage )
{
    iNewStage = MIN(3,MAX(-1,iNewStage));
    m_Stage = iNewStage;

    f32 Radius = s_GlobTweaks[ m_Stage ].hRadius.GetF32();
    m_RenderRadius = Radius;
    m_CollRadius = Radius * s_TweakCollRadiusMultiplier.GetF32();

    if (m_hNucleusFX.Validate())
        m_hNucleusFX.KillInstance();

    rhandle<char> FxResource;
    FxResource.SetName( k_GlobFXName[ m_Stage ] );

    const char* pScrewedUpName = FxResource.GetPointer();
    if( pScrewedUpName )
    {
        m_hNucleusFX.InitInstance( pScrewedUpName );
        m_hNucleusFX.SetSuspended( FALSE );  
    }

    m_FloorProperties.Init( Radius, 1.0f );
    m_FloorProperties.ForceUpdate( GetPosition() );
}

//=========================================================================

void alien_glob::StartEndOfLife( endoflife EOLReason )
{
    m_bEndOfLife = TRUE;
    rhandle<char> FxResource;
    FxResource.SetName( k_GlobEndOfLifeFX[ EOLReason ] );
    const char* pScrewedUpName = FxResource.GetPointer();
    if (pScrewedUpName)
    {
        m_hEndOfLifeFX.InitInstance( pScrewedUpName );
        m_hEndOfLifeFX.SetSuspended( FALSE );   
    }

    g_AlienGlobMgr.RequestAudio( this, alien_glob_mgr::GLOB_AUDIO_DEATH );
}

//=========================================================================

#ifdef X_EDITOR

void alien_glob::StickToNearestSurface( void )
{
    vector3     Dir[] = { vector3(0,0,1),
                          vector3(0,0,-1),
                          vector3(0,1,0),
                          vector3(0,-1,0),
                          vector3(1,0,0),
                          vector3(-1,0,0) };

    s32 iBestDir = -1;
    f32 iBestT = 1e30f;
    f32 CheckDist = 400.0f;
    collision_mgr::collision Coll;

    s32 i;
    for (i=0;i<6;i++)
    {
        vector3 Dest = Dir[i];
        Dest.NormalizeAndScale( CheckDist );
        Dest += m_Position;

        g_CollisionMgr.SphereSetup( GetGuid(), m_Position, Dest, m_CollRadius );
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
            (object::object_attr)(object::ATTR_BLOCKS_CHARACTER | object::ATTR_BLOCKS_CHARACTER_LOS),
            (object::object_attr)(object::ATTR_COLLISION_PERMEABLE) );

        if (g_CollisionMgr.m_nCollisions > 0)
        {
            if (g_CollisionMgr.m_Collisions[0].T < iBestT)
            {
                iBestT = g_CollisionMgr.m_Collisions[0].T;
                iBestDir = i;
                Coll = g_CollisionMgr.m_Collisions[0];
            }
        }
    }

    if (iBestDir >= 0)
    {
        vector3 Temp = Dir[ iBestDir ];
        Temp.NormalizeAndScale( iBestT * CheckDist );        
        
        vector3 NewPos = m_Position + Temp;
        Temp.NormalizeAndScale(-10);
        NewPos += Temp;
        OnMove( NewPos );

        Land( m_Position, Coll.Plane.Normal, Coll.ObjectHitGuid );
    }


}

#endif // X_EDITOR

//=========================================================================

void alien_glob::OnThink( f32 DeltaTime )
{
    (void)DeltaTime;
    m_bThinking = TRUE;

    // If we are running away, the leader doesn't matter. Save yourself!!
    if (m_bFleeFromTarget || m_bCommitedToAttack)
        return;

    f32 DistToLeader        = s_GlobTweaks[ m_Stage ].hFlockDistToLeader.GetF32();
    f32 DistBetweenLeader   = s_GlobTweaks[ m_Stage ].hFlockDistBetweenLeader.GetF32();

    DistToLeader = DistToLeader*DistToLeader;
    DistBetweenLeader = DistBetweenLeader*DistBetweenLeader;

    if (m_bIsLeader)
    {
        // If already a leader, all we have to do is check to see if there
        // is another leader nearby.  If there is, give up personal leadership
        // in favor of the remote leader
        slot_id Slot = g_ObjMgr.GetFirst( object::TYPE_ALIEN_GLOB );
        while ( SLOT_NULL != Slot )
        {
            object* pObj = g_ObjMgr.GetObjectBySlot( Slot );
            if (NULL == pObj)
                break;            
        
            alien_glob& Glob = alien_glob::GetSafeType( *pObj );
            if (Glob.IsLeader())
            {
                vector3 Delta  = (Glob.GetPosition() - GetPosition());
                f32     DistSq = Delta.LengthSquared();

                if (DistSq < (DistBetweenLeader))
                {
                    m_bIsLeader  = FALSE;
                    m_LeaderGuid = NULL_GUID;
                    break;
                }
            }

            Slot = g_ObjMgr.GetNext( Slot );
        }
    }

    // If we are still a leader, just bail
    if (m_bIsLeader)
        return;

    xbool bFindLeader = FALSE;

    //
    //  Verify that the leader still exists and is a leader.
    //  If that is not the case, find a new leader.
    //  If a new leader cannot be found, become one.
    //

    if (m_LeaderGuid == NULL_GUID)
    {
        bFindLeader = TRUE;
    }
    else 
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_LeaderGuid );
        if (NULL == pObj)
        {
            bFindLeader = TRUE;
        }
        else
        {
            object_ptr<alien_glob> Glob( pObj->GetGuid() );
            if (!Glob.IsValid())
            {
                bFindLeader = TRUE;
            }
            else
            {
                if (!Glob->IsLeader())
                {
                    bFindLeader = TRUE;
                }
                else
                {
                    if ( (Glob->GetPosition() - GetPosition()).LengthSquared() > (DistToLeader))
                    {
                        bFindLeader = TRUE;
                    }
                }
            }
        }
    }


    if (bFindLeader)
    {
        slot_id Slot = g_ObjMgr.GetFirst( object::TYPE_ALIEN_GLOB );
        m_LeaderGuid = NULL_GUID;
        while ( SLOT_NULL != Slot )
        {
            object* pObj = g_ObjMgr.GetObjectBySlot( Slot );
            if (NULL == pObj)
            {
                break;            
            }

            alien_glob& Glob = alien_glob::GetSafeType( *pObj );
            if (!Glob.IsLeader())
            {
                Slot = g_ObjMgr.GetNext( Slot );
                continue;
            }

            vector3 Delta  = (Glob.GetPosition() - GetPosition());
            f32     DistSq = Delta.LengthSquared();

            if (DistSq < (DistToLeader))
            {
                m_LeaderGuid = Glob.GetGuid();
                break;
            }

            Slot = g_ObjMgr.GetNext( Slot );
        }

        if (m_LeaderGuid == NULL_GUID)
        {
            // Become the leader
            m_bIsLeader = TRUE;           
        }
    }
}

//=========================================================================

//=========================================================================

//=========================================================================

//=========================================================================
