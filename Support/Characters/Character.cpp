//=========================================================================
//
//  Character.cpp
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================


#ifndef X_EDITOR
#include "NetworkMgr\GameMgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#endif

#include "Character.hpp"
#include "Navigation\CoverNode.hpp"
#include "Navigation\AlarmNode.hpp"
#include "..\MiscUtils\TrajectoryGenerator.hpp"
#include "objects\GrenadeProjectile.hpp"
#include "objects\GravChargeProjectile.hpp"
#include "Objects\Player.hpp"
#include "Objects\Group.hpp"
#include "GameLib\StatsMgr.hpp"
#include "GameLib\RenderContext.hpp"
#include "Objects\Event.hpp"
#include "EventMgr\EventMgr.hpp"
#include "CharacterState.hpp"
#include "God.hpp"
#include "Triggerex\triggerex_object.hpp"
#include "Objects\Spawner\SpawnerObject.hpp"
#include "Debris\debris_rigid.hpp"
#include "objects\Corpse.hpp"
#include "Characters\TaskSystem\character_task_set.hpp"
#include "TriggerEx\TriggerEx_Object.hpp"
#include "TriggerEx\Actions\action_ai_base.hpp"
#include "TriggerEx\Actions\action_ai_attack_guid.hpp"
#include "TriggerEx\Actions\action_ai_pathto_guid.hpp"
#include "TriggerEx\Actions\action_ai_lookat_guid.hpp"
#include "TriggerEx\Actions\action_ai_play_anim.hpp"
#include "TriggerEx\Actions\action_ai_dialog_line.hpp"
#include "TriggerEx\Actions\action_ai_searchto_guid.hpp"
#include "TriggerEx\Actions\action_ai_death.hpp"
#include "Characters\BaseStates\Character_Cover_state.hpp"
#include "ConversationMgr\ConversationMgr.hpp"
#include "objects\NewWeapon.hpp"
#include "objects\turret.hpp"
#include "Sound\EventSoundEmitter.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "Animation\AnimData.hpp"
#include "Characters\ActorEffects.hpp"
#include "OccluderMgr\OccluderMgr.hpp"
#include "Characters\Soldiers\Soldier.hpp"
#include "Characters\MutantTank\Mutant_Tank.hpp"
#include "Objects\AlienGlob.hpp"
#include "characters\scientist\friendlyscientist.hpp"

xbool SPLIT_LONG_LOS = FALSE;
const f32 k_VerySmallNumber = 0.00000001f;

class dialog_tweak
{
public:

    dialog_tweak( const char* pName );

    tweak_handle     m_Tweak;
    xtick            m_LastTime;
};


dialog_tweak::dialog_tweak( const char* pName ) : m_Tweak(pName)
{
    m_LastTime = 0;
}

dialog_tweak g_DialogTweaks[actor::DIALOG_TYPE_COUNT+1] =
{
    ("DIALOG_NONE_RepeatTime"),
    ("DIALOG_ALERT_RepeatTime"),
    ("DIALOG_RUSH_RepeatTime"),
    ("DIALOG_KILL_RepeatTime"),
    ("DIALOG_CLEAR_RepeatTime"),
    ("DIALOG_FLEE_RepeatTime"),
    ("DIALOG_FRIENDLY_HIT_RepeatTime"),
    ("DIALOG_FRIENDLY_WOUND_RepeatTime"),
    ("DIALOG_GRENADE_THROW_RepeatTime"),
    ("DIALOG_GRENADE_SPOT_RepeatTime"),
    ("DIALOG_COVER_RepeatTime"),
    ("DIALOG_COVER_REQ_RepeatTime"),
    ("DIALOG_MANDOWN_RepeatTime"),
    ("DIALOG_RELOAD_RepeatTime"),
    ("DIALOG_UNDER_FIRE_RepeatTime"),
    ("DIALOG_HIT_RepeatTime"),
    ("DIALOG_HIT_MELEE_RepeatTime"),
    ("DIALOG_FLINCH_RepeatTime"),
    ("DIALOG_DIE_MELEE_RepeatTime"),
    ("DIALOG_DIE_GUNFIRE_RepeatTime"),
    ("DIALOG_DIE_EXPLOSION_RepeatTime"),
    ("DIALOG_DIE_FALL_RepeatTime")
};

static s32 s_DialogVoiceID = 0;

//=========================================================================
// DEFINES
//=========================================================================
#define FOOTFALL_COLLISION_DEPTH    10.0f
f32 g_MinTimeTalk    = 4.0f;
f32 g_MaxTimeTalk    = 15.0f;
f32 k_MinFallTransTime = 1.0f;
f32 k_MaxFallTransTime = 4.0f;
extern s32 g_Difficulty;
extern const char* DifficultyText[];

const f32 k_MaxTimeActorColliding       = 3.0f;
const f32 k_MaxTimeObjectColliding      = 10.0f;
const f32 k_TimeBetweenLocationUpdates  = 0.5f;
const f32 k_MinAtPosDist                = 400.0f;
const f32 k_MinTimeBetweenGetPath       = 0.5f;
const f32 k_MinNormalArriveDist         = 50.0f;
const f32 k_MinTimeBetweenDialogs       = 1.0f;
const f32 k_MinTimeBetweenScannerVO     = 5.0f;
const f32 k_StepHeight                  = 10.0f;
const f32 k_MaxDistAbove                =  300.0f;
const f32 k_MaxDistBelow                = -300.0f;
const f32 k_MaxTimeTurning              = 5.0f;
const f32 k_LongestSafeFragGrenadeDist  = 1000.0f;
const f32 k_LongestSafeGravGrenadeDist  = 2000.0f;
const f32 k_MinTimeBetweenRetreatJigs   = 3.0f;
const f32 k_MinTimeNotSeenCoverValid    = 8.0f;
const f32 k_MinDistToLookatMoveatSqr    = 50.0f * 50.0f;
const f32 k_MaxTimeWaitingForVoiceStart = 3.0f;
const f32 k_ShootIfThisCloseSqr         = 500.0f * 500.0f;
const f32 k_MinTimeBetweenCoverChecksNoCover = 1.0f;
const f32 k_MinTimeBetweenCoverChecksRunning = 4.0f;
const f32 k_MinTimeBetweenCoverChecksAtCover = 2.0f;
const f32 k_MinTimeBetweenTargetUpdates      = 3.0f;
const f32 k_MinTimeTillTargetLost       = 1.0f;
const f32 k_MinTimeToCloak              = 0.5f;
const f32 k_MinTimeToDecloak            = 0.5f;
const f32 k_MinTimeTillCombatClear      = 3.0f;
const f32 k_MinTimeBetweenPlayerDamageShots = 0.5f;
const f32 k_MinDistToRunBehind          = 500.0f;
const f32 k_MinDistAcceptableDotIncreases = 600.0f;
const f32 k_MinTimeBetweenLookatSwitch  = 0.5f;
const s32 k_MaxConnectionsContainingPoint = 4;
const f32 k_MinAllowedDistToEntity      = 150.0f;
const f32 k_MinTimeBetweenAlerts        = 1.0f;
const f32 k_MaxAvoidTime                = 0.6f;
const f32 k_MinAvoidTime                = 0.3f;
const f32 k_MinTimeInEscapeAttempt      = 1.0f;
const f32 k_MinDistForStuck             = 10.0f;
const f32 k_MinTimeToBeStuck            = 1.5f;
const f32 k_EscapeDelayTime             = 0.25f;
const f32 k_PlayerPrefPercent           = 0.7f;
const f32 k_CurrTargetPrefPercent       = 0.8f;
const f32 k_OtherTargettedPercent       = 0.5f;
const f32 k_AlertSmellDist              = 200.0f;
const f32 k_AttackSmellDist             = 400.0f;
const f32 k_MaxTimeInCover              = 4.0f;
const f32 k_MinDistInCoverSqr           = 150.0f*150.0f;
const f32 k_MinAnimPlayRate             = 0.85f;
const f32 k_MaxAnimPlayRate             = 1.15f;
const s32 k_GrenadeEvadePercent         = 100;
const f32 k_GrenadeEvadeDistance        = 900.0f * 900.0f;
const f32 k_MaxDistToAlarmNodeSqr       = 1500.0f * 1500.0f;
const f32 k_MinRunDistSqr               = 200.0f * 200.0f;
const f32 k_MaxProwlDistSqr             = 600.0f * 600.0f;
const f32 k_MaxDistRapidFiringSqr       = 500.0 * 500.0f;
const f32 k_MinTimeSinceLastPathToTargetCheck = 2.0f;
const f32 k_MinTimeAlert                = 0.5f;
const f32 k_MaxTimeAlert                = 4.0f;
const f32 k_MinSearchTime               = 4.0f;
const f32 k_MinShotBendAngle            = R_20;
const f32 k_MinShotFireAngle            = R_45;
const f32 k_MinDistToProvideAutofireSqr = 800.0f * 800.0f;
const f32 k_MaxNotSeenToFire            = 5.0f;
const f32 k_MinMeleeEventDistance       = 100.0f;
const s32 k_DropWeaponChance            = 0;
const f32 k_MinDistMeleeLookatSqr       = 600.0f * 600.0f;
const f32 k_IgnoreObjectsInCoverDist    = 300.0f;
const f32 k_MinAngleForBackwardsMove    = R_135;
const f32 k_MinDistForBackwardsMoveContinueSqr  = 200.0f * 200.0f;
const f32 k_MinDistForBackwardsMoveStartSqr     = 600.0f * 600.0f;
const f32 k_MinDistToTargetForBackwardsMoveSqr  = 400.0f * 400.0f;
const f32 k_MinTimeAtCoverTargetLost        = 3.0f;
const f32 k_AdditionalTimeAtCoverTargetLost  = 6.0f;
const f32 k_MinDistToObstacleToFireSqr      = 500.0f * 500.0f;
const f32 k_MinTimeOutsideConeToStartHeadLookat = 4.0f;
const f32 k_MinTimeNotSeenToKeepTarget  = 1.0f;
const f32 k_MinTimeToSayAllClear        = 4.0f;

#define CHARACTER_DATA_VERSION   1000

static f32 k_RethinkRetreat = 2.0f;

static          vector3 DefaultSize(50.0f,50.0f,50.0f);

#ifdef shird
#define AI_LOGGING
#endif

//=========================================================================

struct npc_table_entry
{
    s32         Item;
    const char* pName;
    const char* pIdentifier;
    s32         LastVOIndex;
};

static npc_table_entry s_NPCTable[] =
{
    { NPC_GENERIC,          "Generic NPC"       ,  "GENERIC"    , 4},
    { NPC_BRIDGES,          "Bridges"           ,  "BRIDGES"    , 4},
    { NPC_CARSON,           "Carson"            ,  "CARSON"     , 4},
    { NPC_CRISPY,           "Crispy"            ,  "CRISPY"     , 4},
    { NPC_CRISPY_MUTATED,   "Crispy - MUTATED"  ,  "CRISPY_MUT" , 4},
    { NPC_CHEW,             "Chew"              ,  "CHEW"       , 4},
    { NPC_DRCRAY,           "Dr. Cray"          ,  "DRCRAY"     , 4},
    { NPC_FERRI,            "Ferri"             ,  "FERRI"      , 4},
    { NPC_LEONARD,          "Leonard"           ,  "LEONARD"    , 4},
    { NPC_MCCANN,           "McCan"             ,  "MCCAN"      , 4},
    { NPC_MRWHITE,          "Mr. White"         ,  "MRWHITE"    , 4},
    { NPC_RAMIREZ,          "Ramirez"           ,  "RAMIREZ"    , 4},
    { NPC_VICTOR,           "Victor"            ,  "VICTOR"     , 4},
    { NPC_SCIENTIST,        "Scientist"         ,  "GENERIC"    , 4},
    { NPC_TECHNICIAN,       "Technician"        ,  "GENERIC"    , 4},
    { NPC_THETA,            "Theta"             ,  "THETA"      , 4},
    { NPC_ALIENGLOB,        "Glob"              ,  "UNKNOWN"    , 4},
    { NPC_MUTANT,           "Mutant"            ,  "MUTANT"     , 4},
    { NPC_BLACKOPS,         "Blackops"          ,  "BO"         , 4},
    { NPC_BLACKOP_LEADER,   "Blackops Leader"   ,  "BO_LEADER"  , 4},
    { NPC_UNKNOWN,          "Unknown"           ,  "UNKNOWN"    , 4},
    
};

#define NUM_NPC_TABLE_ENTRIES   ((s32)( sizeof(s_NPCTable) / sizeof(npc_table_entry) ))

#if defined( ksaffel )
xbool g_LogNPCs = TRUE;
#define NPC_LOGGING g_LogNPCs
#else
#define NPC_LOGGING 0
#endif

//=========================================================================
// EXTERNALS
//=========================================================================

extern xarray<audio_manager::receiver> g_ListenQueue;
extern xbool g_game_running;

//=========================================================================
// STATIC
//=========================================================================

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


//=============================================================================

//=========================================================================
// CHARACTER STATIC DATA
//=========================================================================

// Debug static booleans
#if !defined(X_RETAIL) || defined(X_QA)
xbool character::s_bDebugInGame = FALSE ;   // Debugs on PS2,XBOX etc
xbool character::s_bDebugLoco   = TRUE ;    // Loco lookat, moveat
xbool character::s_bDebugAI     = TRUE ;    // AI
xbool character::s_bDebugPath   = TRUE ;    // Pathfinding
xbool character::s_bDebugStats  = FALSE ;   // Shows # of meshes, verts, faces, bones etc
#endif // X_RETAIL

//=========================================================================
// Dialog Actors table
typedef enum_pair<character::eDialogActors> dialog_actor_enum_pair;

static dialog_actor_enum_pair s_DialogActorsTable[] = 
{
        dialog_actor_enum_pair("None",                  character::DIALOG_ACTOR_NONE),
        dialog_actor_enum_pair("A",                     character::DIALOG_ACTOR_A),
        dialog_actor_enum_pair("B",                     character::DIALOG_ACTOR_B),
        dialog_actor_enum_pair("C",                     character::DIALOG_ACTOR_C),
        dialog_actor_enum_pair("D",                     character::DIALOG_ACTOR_D),

        dialog_actor_enum_pair( k_EnumEndStringConst,   character::INVALID_ACTION_TYPES) //**MUST BE LAST**//
};

enum_table<character::eDialogActors> character::m_DialogActorsEnum ( s_DialogActorsTable ); 

//=========================================================================

guid GetGuidBlockingLOS( guid StartGuid, guid EndGuid, const vector3& StartPos, const vector3& EndPos, xbool bIgnoreLiving, xbool bIgnoreDestructables )
{
    // Do collision check with polycache
    g_CollisionMgr.LineOfSightSetup( 0, StartPos, EndPos );
    g_CollisionMgr.AddToIgnoreList( StartGuid );
    g_CollisionMgr.AddToIgnoreList( EndGuid );

    if( bIgnoreDestructables )
    {
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_CHARACTER_LOS, object::ATTR_COLLISION_PERMEABLE | object::ATTR_DESTRUCTABLE_OBJECT );
    }
    else
    {
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_CHARACTER_LOS, object::ATTR_COLLISION_PERMEABLE );
    }

    if( g_CollisionMgr.m_nCollisions != 0 )
        return g_CollisionMgr.m_Collisions[0].ObjectHitGuid;

    // No collision with polycache so test against actors
    if( !bIgnoreLiving )
    {
        actor* pActor = actor::m_pFirstActive;
        while( pActor )
        {
            if( (pActor->GetGuid()!=StartGuid) && (pActor->GetGuid()!=EndGuid) )
            {
                f32 T;
                bbox BBox = pActor->GetColBBox();
                if( BBox.Intersect(T,StartPos,EndPos) )
                    return pActor->GetGuid();
            }

            pActor = pActor->m_pNextActive;
        }
    }
    return 0;
}

//=========================================================================

guid GetGuidBlockingLOS( const vector3& StartPos, const vector3& EndPos, xbool bBlockOnLiving, xbool bIgnoreDestructables )
{
    return GetGuidBlockingLOS( 0, 0, StartPos, EndPos, bBlockOnLiving, bIgnoreDestructables );
}

//=========================================================================
// dialog_info class
//=========================================================================

void character::dialog_info::Clear( )
{
    m_DialogType    = DIALOG_NONE;
    m_DialogName[0] = 0;
    m_AnimName[0]   = 0;
    m_AnimGroup[0]  = 0;
    m_AnimFlags     = 0;
    m_DialogFlags   = 0;
    m_BlendOutTime  = 0.0f;
}

//=========================================================================
//=========================================================================

void character::dialog_info::SetData( eDialogType dialogType, const char* dialogName , const char* animName , const char* animPkg, u32 Flags, u8 DialogFlags, f32 BlendOutTime )
{
    m_DialogType = dialogType;
    if( dialogName )
    {
        x_strcpy(m_DialogName, dialogName);
    }
    if( animName )
    {
        x_strcpy(m_AnimName, animName);
    }
    if( animPkg )
    {
        x_strcpy(m_AnimGroup, animPkg);
    }
    m_AnimFlags     = Flags;
    m_DialogFlags   = DialogFlags;
    m_BlendOutTime  = BlendOutTime;
}

//=========================================================================
// character_hold_state class
//=========================================================================

character_hold_state::character_hold_state( character& Character, states State )
: character_state( Character, State )
{
}

//=========================================================================
// STRUCTURE FUNCTIONS
//=========================================================================

//=========================================================================
// CHARACTER FUNCTIONS
//=========================================================================

// Constructors
character::character() : 
m_pStateList( NULL),            // List of connected states
m_pActiveState( NULL ),            // Currently active state
m_TriggerState  ( *this, character_state::STATE_TRIGGER ),
m_HoldState     ( *this, character_state::STATE_HOLD ),
m_FollowState   ( *this, character_state::STATE_FOLLOW )
{
    //  -------------------------------------------------------------------
    //
    //  Clear locomotion pointer
    //
    m_pLoco         = NULL;

    m_BBox.Set(
        vector3( 0.0f,0.0f,0.0f),
        100);

    // Clear state vars
    m_InitialState      = character_state::STATE_IDLE;   // Startup state of character
    m_bFollowingPath    = FALSE ;
    m_CurrentPathStructIndex    = -1 ;
    m_bPassOnGuidToCorpse     =FALSE;
    m_CorpseGuid      = 0;
    m_NotifyOnDeath     = 0 ;
    m_GrenadeTrajectory = radian3(0.0f,0.0f,0.0f);
    m_TimesPlayerHasDamagedMe = 0;
    m_TimeSincePlayerDamagedMeLast = 0.0f;

    m_Subtype               = SUBTYPE_NONE;
    m_LastState     = character_state::STATE_IDLE;
    m_PointsPerSpline       = 0;
    m_CurrentSplinePoint    = -1;
    m_MaxDistToCover        = 1500.0f;
    m_bOnlyUsesCoverAhead   = FALSE;
    m_bCoverHopper          = FALSE;
    m_bAutoRagdoll          = FALSE;
    m_AnimYawDelta          = 0.0f;
    // Active vars
    m_bThinking             = FALSE;
    m_bTargetPlayer         = FALSE;
    m_GoalCompleted         = FALSE;
    m_GoalSucceeded         = FALSE;
    m_NewGoal               = FALSE;
    m_TimeInGoal            = 0.0f;
    m_DamageInCover         = 0.0f;
    m_TimeSinceLastLookatSwitch = k_MinTimeBetweenLookatSwitch;
    m_TimeSinceLastReload   = 5.0f;
    m_CloakDecloakTimer     = 0.0f;
    m_TimeSinceLastTargetUpdate = 0.0f;
    m_TimeSinceLastCoverCheck = 0.0f;
    m_TimeSinceLastRender   = 10.0f;
    m_AimToTargetYaw        = 0.0f;
    m_GoalRetreatToConnectionSlot = NULL_NAV_SLOT;
    m_CurrentConnectionSlot = NULL_NAV_SLOT;
    m_GoalsConnectionSlot   = NULL_NAV_SLOT;
    m_GroundMaterial        = MAT_TYPE_CONCRETE;
    m_IgnoreLocoInTransform = FALSE;
    m_AvoidAttemptTime      = 0.0f;
    m_PreviousActorCollisionRadius = 0.0f;
    m_LastListenTime        = 0;
    m_LastAlertTime         = g_ObjMgr.GetGameTime();
    m_LastAlertSentTime     = 0;
    m_SoundRange            = 2000.0f;
    m_TimeActorColliding    = 0.0f;
    m_TimeObjectColliding   = 0.0f;
    m_EscapeAttempts        = 0;
    m_MinStateTime          = 0.0f;
    m_AvoidActorAttempt     = AVOID_NONE;
    m_CombatReady           = FALSE;
    m_bNeverCombatReady     = FALSE;
    m_bHasClearJumpAttack   = FALSE;
    m_bHasClearLOSFeet      = FALSE;
    m_bHasClearLOSEyes      = FALSE;
    m_bTargetSeen           = FALSE;
    m_bPlayAllClearDialog   = FALSE;
    m_bIgnorePain           = FALSE;
    m_bIgnoreFlinches       = FALSE;
    m_NextShotMustHit       = FALSE;
    m_OverrridePreferedVoiceActor = DIALOG_ACTOR_NONE;
    m_LipSyncStartAnimFrame = -1.0f;
    m_bWasPlayingFullBodyLipSync = FALSE;
    m_PostLipsyncState      = character_state::STATE_NULL;   

    //create faction info
    m_Faction = FACTION_NOT_SET;
    m_FriendFlags = FACTION_NONE;
    m_TimeInEscapeAttempt   = k_MinTimeBetweenDialogs;
    m_TimeStuck             = 0.0f;
    m_IsStuck               = FALSE;
    m_TimeSinceLastDialog   = 0;
    m_AllowDialog           = TRUE;
    m_LastEscapeAttempt     = ESCAPE_NONE;
    m_CurrentEscapeAttempt  = ESCAPE_NONE;
    m_TimeSinceLastSound    = 1.0f;
    m_CurrentDialogType     = DIALOG_NONE;
    m_FriendlyBlocksTarget  = FALSE;

    m_LastAnimPainID        = -1;
    m_LastMeleeEventID      = -1;
    m_WeaponItem            = INVEN_NULL;
    m_GrenadeItem           = INVEN_NULL;
    m_WeaponFireAngle       = R_20;
    m_Reloading             = FALSE;
    m_Accuracy              = 75;
    m_MovingTargetAccuracy  = 50;
    m_InNavMap              = TRUE;
    m_CoverChanged          = FALSE;
    m_StickyCoverNode       = NULL_GUID;
    m_AimAtGuid             = NULL_GUID;
    m_StickyTurret          = NULL_GUID;
    m_pNoBlockDialogTrigger = NULL_GUID;
    m_HeadLookat            = NULL_GUID;
    m_HeadLookatTimer       = 0.0f;
    m_OutofLookatTimer      = k_MinTimeOutsideConeToStartHeadLookat;
    m_bForceWeaponFire      = FALSE;
    m_bCanReload            = TRUE;
    m_bNeverPlayReloadAnim  = FALSE;
    m_bDoRunLogic           = TRUE;
    m_bDoRender             = TRUE;
    m_bIsInHeadTracking     = FALSE;
    m_bProjectileAttached   = FALSE;

    m_RethinkRetreatTimer   = 0;
    m_TimeActive            = 0;

    m_JumpTimeRemaining     = 0;
    m_TotalJumpTime         = 0;
    m_JumpVelocity          = 600.0f;
    m_TimeSinceLastRetreatJig = 0.0f;
    m_DialogPrefixString    = -1;
    m_LogicalName           = -1;
    m_DesiredState          = character_state::STATE_NULL;
    m_WeaponDropPercent     = 50;
    m_AmmoDropPercent       = 50;
    m_GrenadeDropPercent    = 50;
    m_InventoryDropPercent  = 50;
    m_bDropWeapons          = TRUE;
    m_bDropAmmo             = TRUE;      
    m_bDropGrenades         = TRUE;    
    m_bDropInventory        = TRUE;

    m_LeashGuid             = 0;
    m_LeashDistance         = 0.0f;
    m_CanCloak              = FALSE;
    m_MustCloak             = FALSE;
    m_CloakShieldPainTimer  = 0.0f;
    m_BodiesFade            = TRUE;
    m_ShortMeleeRange       = -1.0f;
    m_LongMeleeRange        = -1.0f;

    m_CurrentDialogInfo.Clear();
    m_bRootToPositionWhenIdle       = FALSE;
    m_bCanUseAlarms         = TRUE;

    m_TimeOutofNavMap = 0.0f;
    m_NPCName               = NPC_GENERIC;
    m_ScannerVOIndex        = 1;  // they all start at 1
    m_StunTimer             = 0.0f;
    
#ifdef X_EDITOR
    m_nPreviousRequestedLines = 0;
    m_nScreenBlockMaxLines  = 0;
    m_iScreenBlockLine      = 0;
    m_ScreenBlockCursor.Set(0,0);
#endif // X_EDITOR
}

//=========================================================================

character::~character()
{
    SetIsActive( FALSE );
}

//=========================================================================
// INHERITED VIRTUAL FUNCTIONS FROM BASE CLASS
//=========================================================================

void character::OnInit( void )
{
    // Call base class
    actor::OnInit();
#if !defined(X_RETAIL) || defined(X_QA)
    m_CurrentPainRadius = 0.0f;
#endif

#ifndef X_RETAIL
    int c;
    for(c=0;c<4;c++)
    {    
        x_strcpy(m_StateChangeList[c],"");
        x_strcpy(m_PhaseChangeList[c],"");
        x_strcpy(m_GoalChangeList[c],"");
    }
#endif

    x_strcpy( m_WeaponlessAnimPackage, "" );
    // Initialize vars
    m_MeleeInited           = FALSE;
    m_LocoFallingOverride   = FALSE;
    m_Health.Reset();
    m_MaxHealth = 100.0f;
    m_LastSoundPos = GetPosition();
    m_LastLocationOfInterest = GetPosition();
    m_LastKnownLocationOfTarget = GetPosition();
    m_TargetSightedTimer    = 0.0f;
    m_NoTargetTimer         = k_MinTimeTillCombatClear;
    m_AwarenessLevel        = character::AWARENESS_NONE;
    m_TimeAtAwarenessLevel  = 0.0f;
    m_TimeSinceLastPathToTargetCheck = 0.0f;
    m_LastSeenLocationOfTarget  = GetPosition();
    m_SoundHeard            = FALSE;
    m_PreferedVoiceActor    = x_irand(0,GetNumberVoiceActors()-1);
    m_TimeSinceLastDialog   = x_frand(0,k_MinTimeBetweenDialogs);
    m_bWantsToDropWeapon    = ( x_irand(1,100) > k_DropWeaponChance );
    m_bCanAlwaysBackpeddle  = FALSE;
    // Sight vars
    m_LightSightRadius  = 100*15;                // Sight distance
    m_DarkSightRadius   = 100*5;                 // Sight distance
    m_IdleSightFOV      = DEG_TO_RAD(80);        // FOV
    m_AlertSightFOV     = DEG_TO_RAD(180);       // FOV
    m_LogStateChanges   = FALSE;

    m_HeadLookatDistance = m_DarkSightRadius;
    m_HeadLookatAngle    = m_IdleSightFOV;

    // Target vars
    m_TargetGuid                = 0;            // Our target
    m_TargetNotSeenTimer        = 10.0f;         // Time target has not been seen

    m_WantsToAim                = FALSE;
    m_WantsToFirePrimary        = FALSE;
    m_WantsToFireSecondary      = FALSE;
    m_CollisionIgnoreGuid       = 0;

    m_SinceLastShot             = 0.0f;
    m_ShootDelay                = 1.5f;

    // Lookat vars
    m_LookatMode                = LOOKAT_NONE;  // look at mode
    m_OverrideLookatMode        = LOOKAT_NONE;
    m_OverrideLookatInterest    = 0;
    m_OverrideTargetGuid        = 0;
    m_FollowTargetGuid          = 0;
    // State vars
    m_pActiveState = NULL;                 // Currently active state
    m_StateTime    = 0.0f;                 // Time in current state
    m_MinFallHeight = 600.0f;
    m_MaxFallHeight = 1500.0f;

    m_LastGrenadeThrowTime = g_ObjMgr.GetGameTime();

    // Initialize all the states
    character_state* pState = m_pStateList;
    while(pState)
    {
        pState->OnInit();
        pState = pState->m_pNext;
    }


    // Active vars
    SetIsActive( FALSE );
    m_bThinking = FALSE;

    // Path finding
    m_GroundMaterial = MAT_TYPE_CONCRETE;

    m_IgnoreInterest        = FALSE;

    // Setup loco transform?
    if (GetLocoPointer())
    {
        // Get transform info
        const matrix4& L2W = GetL2W();
        vector3 Pos = L2W.GetTranslation();
        radian  Yaw = L2W.GetRotation().Yaw;
        // Update loco
        GetLocoPointer()->SetPosition(Pos);
        GetLocoPointer()->SetYaw(Yaw + R_180); // SB +R_180 is legacy for old anim system
        //        GetLocoPointer()->SetFacingYawBias(Yaw + R_180); // SB +R_180 is legacy for old anim system
        GetLocoPointer()->SetArriveDistSqr(k_MinNormalArriveDist * k_MinNormalArriveDist);
        f32 animPlayRate = x_frand(k_MinAnimPlayRate,k_MaxAnimPlayRate);
        SetAnimRate(animPlayRate);
    }    
}

//=============================================================================

void character::InitMeleeValues()
{
    m_MeleeInited = TRUE;
    // decifer the short and long melee ranges.
    s32 shortIndex = GetLocoPointer()->GetAnimIndex( loco::ANIM_MELEE_SHORT );
    s32 longIndex = GetLocoPointer()->GetAnimIndex( loco::ANIM_MELEE_LONG );
    const anim_group::handle &animGroupHandle = GetLocoPointer()->GetAnimGroupHandle(); 

    if( shortIndex >= 0 )
    {
        // get the anim info
        const anim_info& shortAnimInfo = animGroupHandle.GetPointer()->GetAnimInfo( shortIndex );
        s32 numEvents = shortAnimInfo.GetNEvents();
        s32 c;
        for(c=0;c<numEvents;c++)
        {
            // first we want the horizontal distance from the event to the root. 
            anim_event& shortEvent = shortAnimInfo.GetEvent(c);
            if( x_strcmp(shortEvent.GetType(),"Pain") == 0 )
            {
                // we've found the pain event! when is it on and where is it at that time?
                vector3 totalTranslation;
                // find the translation
                anim_key OldKey;
                shortAnimInfo.GetInterpKey( 0, 0, OldKey );
                anim_key NewKey;
                shortAnimInfo.GetInterpKey( (f32)shortEvent.StartFrame(), 0, NewKey );
                totalTranslation = NewKey.Translation - OldKey.Translation;

                // load melee reach distance tweak
                f32 MeleeReachDistance = 0.0f;
                tweak_handle ReachDistanceTweak( xfs( "%s_MeleeReachDistance", GetLogicalName() ) );
                if( ReachDistanceTweak.Exists() )    
                {
                    MeleeReachDistance = ReachDistanceTweak.GetF32();
                }        
                else        
                {
                    tweak_handle DefaultReachDistanceTweak( "CHARACTER_MeleeReachDistance" );
                    MeleeReachDistance = DefaultReachDistanceTweak.GetF32();
                }

                // load melee sphere radius tweak
                f32 MeleeSphereRadius = 0.0f;
                tweak_handle SphereRadiusTweak ( xfs( "%s_MeleeSphereRadius", GetLogicalName() ) );
                if( SphereRadiusTweak.Exists() )    
                {
                    MeleeSphereRadius  = SphereRadiusTweak.GetF32();
                }        
                else        
                {    
                    tweak_handle DefaultSphereRadiusTweak ( "CHARACTER_MeleeSphereRadius" );
                    MeleeSphereRadius  = DefaultSphereRadiusTweak.GetF32();
                }

                m_ShortMeleeRange = ( MeleeReachDistance + totalTranslation.Length() +  MeleeSphereRadius) * 0.8f ;
                break;
            }
        }
    }

    // do the same for long melee
    if( longIndex >= 0 )
    {
        // get the anim info
        const anim_info& longAnimInfo = animGroupHandle.GetPointer()->GetAnimInfo( longIndex );
        s32 numEvents = longAnimInfo.GetNEvents();
        s32 c;
        for(c=0;c<numEvents;c++)
        {
            // first we want the horizontal distance from the event to the root. 
            anim_event& longEvent = longAnimInfo.GetEvent(c);
            if( x_strcmp(longEvent.GetType(),"Pain") == 0 )
            {
                // we've found the pain event! when is it on and where is it at that time?
                vector3 totalTranslation;
                // find the translation
                anim_key OldKey;
                longAnimInfo.GetInterpKey( 0, 0, OldKey );
                anim_key NewKey;
                longAnimInfo.GetInterpKey( (f32)longEvent.StartFrame(), 0, NewKey );
                totalTranslation = NewKey.Translation - OldKey.Translation;

                // load melee reach distance tweak
                f32 MeleeReachDistance = 0.0f;
                tweak_handle ReachDistanceTweak( xfs( "%s_MeleeReachDistance", GetLogicalName() ) );
                if( ReachDistanceTweak.Exists() )    
                {
                    MeleeReachDistance = ReachDistanceTweak.GetF32();
                }        
                else        
                {
                    tweak_handle DefaultReachDistanceTweak( "CHARACTER_MeleeReachDistance" );
                    MeleeReachDistance = DefaultReachDistanceTweak.GetF32();
                }

                // load melee sphere radius tweak
                f32 MeleeSphereRadius = 0.0f;
                tweak_handle SphereRadiusTweak ( xfs( "%s_MeleeSphereRadius", GetLogicalName() ) );
                if( SphereRadiusTweak.Exists() )    
                {
                    MeleeSphereRadius  = SphereRadiusTweak.GetF32();
                }        
                else        
                {    
                    tweak_handle DefaultSphereRadiusTweak ( "CHARACTER_MeleeSphereRadius" );
                    MeleeSphereRadius  = DefaultSphereRadiusTweak.GetF32();
                }

                m_LongMeleeRange = ( MeleeReachDistance + totalTranslation.Length() +  MeleeSphereRadius) * 0.8f;
                break;
            }
        }
    }
}

//=============================================================================

void character::InitLoco( void )
{
    actor::InitLoco();
    // Setup loco transform?
    if (GetLocoPointer())
    {
        GetLocoPointer()->SetArriveDistSqr(k_MinNormalArriveDist * k_MinNormalArriveDist);
        f32 animPlayRate = x_frand(k_MinAnimPlayRate,k_MaxAnimPlayRate);
        SetAnimRate(animPlayRate);
    }    
}

//=============================================================================

void character::InitTweaks()
{
    char *weaponName = new_weapon::GetWeaponPrefixFromInvType2(GetWeaponItem());
    char logicalAndWeaponName[128];
    // add the logical name,
    x_strcpy(logicalAndWeaponName,GetLogicalName());
    if( weaponName )
    {        
        x_strcat(logicalAndWeaponName,"_");
        x_strcat(logicalAndWeaponName,weaponName);
    }

    tweak_handle shootDelayTweak(xfs("%s_SHOOT_DELAY",logicalAndWeaponName));
    tweak_handle accuracyTweak(xfs("%s_ACCURACY",logicalAndWeaponName));
    tweak_handle movingAccuracyTweak(xfs("%s_MOVING_ACCURACY",logicalAndWeaponName));

    m_ShootDelay            = 1.55f;
    if( shootDelayTweak.Exists() )
    {
        m_ShootDelay = shootDelayTweak.GetF32();
    }

    m_Accuracy              = 51;
    if( accuracyTweak.Exists() )
    {
        m_Accuracy = accuracyTweak.GetS32();
    }

    m_MovingTargetAccuracy  = 26;
    if( movingAccuracyTweak.Exists() )
    {
        m_MovingTargetAccuracy = movingAccuracyTweak.GetS32();
    }


    m_AccuracyDifficultyScaler = 0.0f;
#ifndef X_EDITOR
    if( GameMgr.GetGameType() == GAME_CAMPAIGN )
    {
        tweak_handle AccuracyScalerTweak( xfs( "%s_Accuracy_%s",             
            logicalAndWeaponName , DifficultyText[g_Difficulty] ) );

        if( AccuracyScalerTweak.Exists() )
        {
            m_AccuracyDifficultyScaler = AccuracyScalerTweak.GetF32();
        }
    }
#endif

    // inventory drop percentages.
    tweak_handle weaponDropTweak(xfs("%s_WEAPON_DROP_PERCENT",logicalAndWeaponName));
    tweak_handle ammoDropTweak(xfs("%s_AMMO_DROP_PERCENT",logicalAndWeaponName));

    tweak_handle grenadeDropTweak(xfs("%s_GRENADE_DROP_PERCENT",logicalAndWeaponName));
    tweak_handle inventoryDropTweak(xfs("%s_INVENTORY_DROP_PERCENT",logicalAndWeaponName));

    m_WeaponDropPercent = 51;
    if( weaponDropTweak.Exists() )
    {
        m_WeaponDropPercent = (f32)weaponDropTweak.GetS32();
    }

    m_AmmoDropPercent = 51;
    if( ammoDropTweak.Exists() )
    {
        m_AmmoDropPercent = (f32)ammoDropTweak.GetS32();
    }

    m_GrenadeDropPercent = 51;
    if( grenadeDropTweak.Exists() )
    {
        m_GrenadeDropPercent = (f32)grenadeDropTweak.GetS32();
    }

    m_InventoryDropPercent = 51;
    if( inventoryDropTweak.Exists() )
    {
        m_InventoryDropPercent = (f32)inventoryDropTweak.GetS32();
    }

    // non weapon tweaks.
    tweak_handle lightSightRadiusTweak(xfs("%s_LIGHT_SIGHT_RADIUS",GetLogicalName()));
    tweak_handle darkSightRadiusTweak(xfs("%s_DARK_SIGHT_RADIUS",GetLogicalName()));
    tweak_handle soundRadiusTweak(xfs("%s_SOUND_RADIUS",GetLogicalName()));

    m_LightSightRadius = 2001.0;
    if( lightSightRadiusTweak.Exists() )
    {
        m_LightSightRadius = lightSightRadiusTweak.GetF32();
    }

    m_DarkSightRadius = 501.0;
    if( darkSightRadiusTweak.Exists() )
    {
        m_DarkSightRadius = darkSightRadiusTweak.GetF32();
    }

    m_SoundRange = 3001.0;
    if( soundRadiusTweak.Exists() )
    {
        m_SoundRange = soundRadiusTweak.GetF32();
    }

    // we are going to setup the fall height stuff here too.
    m_MinFallHeight = 600.0f;
    tweak_handle MinFallDist( xfs( "%s_MinFallDistToTakeDamage", GetLogicalName() ) );
    if( MinFallDist.Exists() )    
    {
        m_MinFallHeight = MinFallDist.GetF32();
    }        

    m_MaxFallHeight = 1500.0f;
    tweak_handle MaxFallDist( xfs( "%s_MaxFallDistToTakeDamage", GetLogicalName() ) );
    if( MaxFallDist.Exists() )    
    {
        m_MaxFallHeight = MaxFallDist.GetF32();
    }        

}

//=============================================================================

void character::SetAnimRate( f32 animRate )
{
    if( GetLocoPointer() )
    {    
        GetLocoPointer()->SetStateAnimRate(loco::STATE_IDLE,animRate);
        GetLocoPointer()->SetStateAnimRate(loco::STATE_MOVE,animRate);
        GetLocoPointer()->SetStateAnimRate(loco::STATE_PLAY_ANIM,animRate);
    }
}

//=============================================================================

f32 character::GetAnimRate()
{
    return GetLocoPointer()->GetStateAnimRate(loco::STATE_IDLE);
}

//=============================================================================

f32 character::GetLipSyncEventStartFrame( const char* pAnimGroup, const char* pName )
{
    if (!GetLocoPointer())
        return -1.0f;

    if (!pName || 
        x_strlen(pName) <= 0)
    {
        return -1.0f;
    }

    anim_group::handle hAnimGroup; 
    s32 AnimIndex = -1;
    if (!pAnimGroup || 
        x_strlen(pAnimGroup) > 0)
    {
        hAnimGroup.SetName(pAnimGroup);
        if (!hAnimGroup.GetPointer())
        {
            return -1.0f;
        }

        AnimIndex = hAnimGroup.GetPointer()->GetAnimIndex(pName);
    }
    else
    {
        hAnimGroup = m_hAnimGroup;
        AnimIndex = GetLocoPointer()->m_Player.GetAnimIndex(pName);
    }    

    if (AnimIndex == -1)
    {
        return -1.0f;
    }

    // Search for lip sync event
    const anim_info& ourAnimInfo = hAnimGroup.GetPointer()->GetAnimInfo( AnimIndex );
    f32 StartFrame = ourAnimInfo.FindLipSyncEventStartFrame();
    return StartFrame;
}

//=============================================================================

xbool character::PlayAnimation( const char* pAnimGroup, 
                               const char* pName, 
                               f32 BlendTime, 
                               u32 Flags,
                               f32 PlayTime )
{
    if (!GetLocoPointer())
        return FALSE;

    if (x_strlen(pName) <= 0)
    {
        x_DebugMsg("ERROR: Attempted to play an invalid animation! (no name specified)\n");
        return FALSE;
    }

    anim_group::handle hAnimGroup; 
    s32 AnimIndex = -1;
    if (x_strlen(pAnimGroup) > 0)
    {
        hAnimGroup.SetName(pAnimGroup);
        if (!hAnimGroup.GetPointer())
        {
#ifdef X_EDITOR
            x_try;
            x_throw(xfs("ERROR: Attempted to play an animation (%s) for invalid anim group (%s)!\n",
                pName, pAnimGroup));
            x_catch_display;
#endif // X_EDITOR
            return FALSE;
        }

        AnimIndex = hAnimGroup.GetPointer()->GetAnimIndex(pName);
    }
    else
    {
        hAnimGroup = m_hAnimGroup;
        AnimIndex = GetLocoPointer()->m_Player.GetAnimIndex(pName);
    }    

    if (AnimIndex == -1)
    {
#ifdef X_EDITOR
        x_try;
        x_throw(xfs("Error trying to play anim \"%s\" for character (%s)[%s], the anim package does not contain that many anims!",
            pName, GetTypeDesc().GetTypeName(), (const char*)guid_ToString(GetGuid()) ));
        x_catch_display;
#endif // X_EDITOR
        return FALSE;
    }
    m_CurrentPainEventID = pain_event::CurrentEventID++;
    if( pain_event::CurrentEventID >= S32_MAX )
    {
        pain_event::CurrentEventID = 0;
    }
    GetLocoPointer()->PlayAnim(hAnimGroup, AnimIndex, BlendTime, Flags, PlayTime) ;

#ifndef X_EDITOR
    net_PlayAnim( GetLocoPointer()->m_Player.GetAnimIndex(), BlendTime, Flags, PlayTime );
#endif

    return TRUE;
}

//=============================================================================

void character::StopAnimation( void )
{
    if( GetLocoPointer() && GetLocoPointer()->GetState() == loco::STATE_PLAY_ANIM )
        GetLocoPointer()->ChangeToPreviousState();
}

//=============================================================================

xbool character::IsAnimAtEnd( void )
{
    //no loco, assume at end, so new tasks can happen
    if (!GetLocoPointer())
        return TRUE;

    return GetLocoPointer()->m_Player.IsAtEnd();
}

//=============================================================================

void character::SetYawFacingTarget( radian TargetYaw, radian MaxDeltaYaw )
{
    if (!GetLocoPointer())
        return;

    GetLocoPointer()->SetYawFacingTarget( TargetYaw, MaxDeltaYaw );
}

//=============================================================================

xbool character::IsMeleeingPlayer()
{
    if( m_pActiveState )
    {
        return m_pActiveState->IsMeleeingPlayer();
    }
    else
    {
        return FALSE;
    }
}

//=============================================================================

radian character::GetCurrentFacingYaw( void )
{
    if (!GetLocoPointer())
        return R_0;

    return GetLocoPointer()->GetYaw();
}

//=============================================================================

void character::OnKill( void )
{
    // kinda hacky way to kill an NPC without going through deathstate.
    UnequipCurrentWeapon();   
    OnDeath();
    if( m_pActiveState )
    {
        m_pActiveState->OnExit();
    }
    // Call base class
    KillMe();
    actor::OnKill();
}

//=============================================================================

vector3 character::GetGoalTrueLocation()
{
    vector3 desiredLoc = vector3(0.0f,0.0f,0.0f);
    //    vector3 toLocation = vector3(0.0f,0.0f,0.0f);
    switch( m_GoalInfo.m_GoalType )
    {
    case character::GOAL_TURNTO_TARGET:
    case character::GOAL_LOOKAT_TARGET:
        desiredLoc = GetTargetPosWithOffset(m_GoalInfo.m_TargetGuid,OFFSET_EYES) + m_GoalInfo.m_Offset;
        break;
    case character::GOAL_GOTO_TARGET:
        desiredLoc = GetTargetPosWithOffset(m_GoalInfo.m_TargetGuid) + m_GoalInfo.m_Offset;
        break;
    case character::GOAL_RETREAT_FROM_TARGET:
        desiredLoc = m_PathFindStruct.m_vEndPoint;
        break;
    case character::GOAL_TURNTO_LOCATION:
    case character::GOAL_LOOKAT_LOCATION:
        desiredLoc = m_GoalInfo.m_Location;
        break;
    case character::GOAL_GOTO_LOCATION:
        desiredLoc = m_GoalInfo.m_Location;
        break;
    case character::GOAL_RETREAT_FROM_LOCATION:
        desiredLoc = m_PathFindStruct.m_vEndPoint;
        break;
    case character::GOAL_PLAY_ANIMATION:
    case character::GOAL_PLAY_ANIMATION_SCALED_TO_TARGET:
        break;        
    }
    return desiredLoc;
}

//=========================================================================

vector3 character::GetGoalTargetsLocation()
{
    vector3 desiredLoc = vector3(0.0f,0.0f,0.0f);
    switch( m_GoalInfo.m_GoalType )
    {
    case character::GOAL_TURNTO_TARGET:
    case character::GOAL_LOOKAT_TARGET:
        desiredLoc = GetTargetPosWithOffset(m_GoalInfo.m_TargetGuid,OFFSET_EYES) + m_GoalInfo.m_Offset;
        break;
    case character::GOAL_GOTO_TARGET:
    case character::GOAL_RETREAT_FROM_TARGET:
        desiredLoc = GetTargetPosWithOffset(m_GoalInfo.m_TargetGuid) + m_GoalInfo.m_Offset;
        break;
    case character::GOAL_TURNTO_LOCATION:
    case character::GOAL_LOOKAT_LOCATION:
    case character::GOAL_GOTO_LOCATION:
    case character::GOAL_RETREAT_FROM_LOCATION:
        desiredLoc = m_GoalInfo.m_Location;
        break;
    case character::GOAL_PLAY_ANIMATION:
    case character::GOAL_PLAY_ANIMATION_SCALED_TO_TARGET:
        break;        
    }
    return desiredLoc;
}

//=========================================================================

void character::AdvanceAIState( f32 DeltaTime )
{
    // see if we need to switch into our initial state.
    if ( m_InitialState != character_state::STATE_NULL )
    {
        if ( m_DesiredState != character_state::STATE_TRIGGER )
        {
            if( !HasState(m_InitialState) )
            {
                m_InitialState = character_state::STATE_IDLE;
            }
            SetupState( m_InitialState );
        }
        m_InitialState = character_state::STATE_NULL;
    }
    else if( !m_pActiveState && m_DesiredState == character_state::STATE_NULL ) 
    {
        // if no initial state and no current state, just go to idle...
        SetupState( character_state::STATE_IDLE );
    }

    // check for death
    if( m_pActiveState )
    {       
        if( (m_Health.GetHealth() <= 0.0f) && 
            HasState(character_state::STATE_DEATH) && 
            (GetActiveState() != character_state::STATE_DEATH) )
        {
            LOG_MESSAGE("character::AdvanceAIState","SetupState(STATE_DEATH)");
            SetupState(character_state::STATE_DEATH);
        }

        //only advance if we don't have a state we want to goto.
        if( m_DesiredState == character_state::STATE_NULL )
        {
            // advance our current state.
            m_pActiveState->OnAdvance(DeltaTime);

            // see if we want a new state
            character_state::states newState = m_pActiveState->UpdateState(DeltaTime);            
            if( newState && m_pActiveState->AllowStateSwitching() )
            {
                SetupState( newState );
            }

            m_pActiveState->PostUpdate();
        }
    }
    // here we handle using tasklists... whenever we are in idlestate, if we have a tasklist, we will attempt to do it instead.
    if( (m_pActiveState && m_pActiveState->m_State == character_state::STATE_IDLE) )
    {
        object* pObject = g_ObjMgr.GetObjectByGuid(m_PendingTaskListGuid);
        if ( pObject && pObject->IsKindOf( character_task_set::GetRTTI() ) == TRUE )
        {
            character_task_set* pSet = GetPendingTaskList();
            if( pSet && pSet->AssignNextTask(DeltaTime) )
            {
                m_TriggerStateData.m_TaskListGuid = m_PendingTaskListGuid;
            }
        }
    }            
    // here we handle using turrets
    if( m_pActiveState && m_pActiveState->m_State != character_state::STATE_TRIGGER && m_pActiveState->m_State != character_state::STATE_TURRET && HasState(character_state::STATE_TURRET) )
    {
        object* pObject = g_ObjMgr.GetObjectByGuid(m_StickyTurret);
        if ( pObject && pObject->IsKindOf( turret::GetRTTI() ) == TRUE )
        {
            SetupState(character_state::STATE_TURRET);
        }
    }        
    // finally here we change state.
    if( m_DesiredState != character_state::STATE_NULL )
    {    
        if( GetLogStateChanges() )
        {        
            if( m_pActiveState )
            {        
                LOG_MESSAGE( "Character::StateChange",
                    "OldState %s: Time In State %f: New State %s:",
                    GetStateName( m_pActiveState->GetStateType() ), 
                    m_pActiveState->m_TimeInState,
                    GetStateName( m_DesiredState ) );
#ifndef X_RETAIL
                AddToStateChangeList( 
                    xfs( "OldState %s: Time In State %f: New State %s: Time %f:",
                    GetStateName( m_pActiveState->GetStateType() ), 
                    m_pActiveState->m_TimeInState,
                    GetStateName( m_DesiredState ),
                    x_GetTimeSec() ) );
#endif
            }
            else
            {
                LOG_MESSAGE("Character::StateChange","First State %s:",GetStateName(m_DesiredState));
#ifndef X_RETAIL
                AddToStateChangeList( xfs("First State %s: Time %f:",GetStateName(m_DesiredState),x_GetTimeSec()) );
#endif
            }
        }
        if( ChangeState() )
        {            
            // if we have sucessfully changed state, then advance the new state.
            m_pActiveState->OnAdvance(DeltaTime);
        }
    }
}

//=========================================================================

void character::UpdateAllowMotion()
{
    // allow all motion unless we have a long way backwards to go... then disallow going backwards.
    // kinda wonky huh.
    if( DoingMovementGoal() && 
        GetTargetGuid() &&
        !m_bCanAlwaysBackpeddle )
    {   
        xbool couldMoveBackwards = GetLocoPointer()->IsMotionAllowed(loco::MOTION_BACK);
        xbool canMoveBackwards = TRUE;

        // take our lookat vs our goal dest. If we lie inbetween them we will be backing up.
        vector3 toGoalDest = GetGoalTrueLocation() - GetPosition();
        
        // angle 
        radian minAngleDiff = x_abs( x_MinAngleDiff(GetToTarget().GetYaw(),toGoalDest.GetYaw()) );
        
        // dist to goal dest
        f32 distToGoalDestSqr = toGoalDest.LengthSquared();

        // if the distance is deemed far enough turn moving backwards off.
        f32 distCheckSqr;
        if( couldMoveBackwards )
        {
            distCheckSqr = k_MinDistForBackwardsMoveStartSqr;
        }
        else
        {
            distCheckSqr = k_MinDistForBackwardsMoveContinueSqr;
        }

        if( (minAngleDiff > k_MinAngleForBackwardsMove &&
             distToGoalDestSqr > distCheckSqr) ||
             !HasWeaponEquiped() )
        {
            canMoveBackwards = FALSE;
        }

        // let's try the distance to our target determines if we turn or not... or should that just be a part?
        f32 toTargetSqr = GetToTarget().LengthSquared();
        if( toTargetSqr < k_MinDistToTargetForBackwardsMoveSqr || 
            AlwaysAllowMoveBackwards() )
        {
            canMoveBackwards = TRUE;
        }
        GetLocoPointer()->SetAllowMotion(loco::MOTION_BACK,canMoveBackwards);
    }
    else
    {
        GetLocoPointer()->SetAllowMotion(loco::MOTION_BACK,TRUE);
    }
}

//=========================================================================

void character::AdvanceLoco( f32 DeltaTime )
{
    // Should always have these...
    ASSERT( GetLocoPointer() );
    ASSERT( GetLocoPointer()->IsAnimLoaded() );
    // Advance loco
    GetLocoPointer()->OnAdvance(DeltaTime);

    if( GetLocoPointer()->m_Physics.GetFallMode() == TRUE && m_pActiveState && !m_pActiveState->IgnoreFalling() )
    {
        if( !m_LocoFallingOverride )
        {
            m_FellFromAltitude = GetPosition().GetY();
        }
        // once we have fallen the full just die.
        else if( m_FellFromAltitude - GetPosition().GetY() >= m_MaxFallHeight )
        {
            pain fallingPain;
            // Build a pain event to describe damage and apply to player
            pain Pain;
            Pain.Setup("NPC_FALL_DAMAGE",0,GetPosition());
            Pain.SetDirectHitGuid( GetGuid() );
            Pain.ApplyToObject( GetGuid() );
        }
        m_LocoFallingOverride = TRUE;
    }
    else
    {
        // if we've fallen enough then take damage.
        if( m_LocoFallingOverride && m_FellFromAltitude - GetPosition().GetY() >= m_MinFallHeight )
        {
            pain fallingPain;
            // Get parametric fall distance where 0=safe, 1=dead
            f32 T = x_parametric( m_FellFromAltitude, m_MinFallHeight, m_MaxFallHeight, TRUE );

            // Build a pain event to describe damage and apply to player
            pain Pain;
            Pain.Setup("NPC_FALL_DAMAGE",0,GetPosition());
            Pain.SetCustomScalar( T );
            Pain.SetDirectHitGuid( GetGuid() );
            Pain.ApplyToObject( GetGuid() );
        }
        m_LocoFallingOverride = FALSE;
    }
}

//=========================================================================

xbool character::WeaponReady()
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon && !m_Reloading )
    {
        if( pWeapon->GetAmmoCount() > 0 )
        {
            return TRUE;
        }
    }
    return FALSE;
}

//=========================================================================

xbool g_RunCharacterLogic = TRUE;

void character::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "character::OnAdvanceLogic" );

    if( m_bWasPlayingFullBodyLipSync && !IsPlayingFullBodyLipSync() && m_PostLipsyncState != character_state::STATE_NULL )
    {
        SetupState( m_PostLipsyncState );
        ChangeState();
        SetMinStateTime(2.0f);

        // TO DO: Play idle anim for this state with blend time unless we are going to hold state.
        if( m_PostLipsyncState != character_state::STATE_HOLD )
        {            
            GetLocoPointer()->PlayAnim( GetLocoPointer()->GetMoveStyleAnimIndex(loco::MOVE_STYLE_ANIM_IDLE), DEFAULT_BLEND_TIME, loco::ANIM_FLAG_INTERRUPT_BLEND );
        }            
        else
        {
            OnActivate(FALSE);
            m_PostLipsyncState = character_state::STATE_NULL;
            m_bWasPlayingFullBodyLipSync = IsPlayingFullBodyLipSync();
            return;
        }
        m_PostLipsyncState = character_state::STATE_NULL;
    }
    m_bWasPlayingFullBodyLipSync = IsPlayingFullBodyLipSync();

    if( !g_RunCharacterLogic || !m_bDoRunLogic )
    {
        return;
    }

    // Skip if not active ("god" updates this flag)
    if ( !IsActive() && m_DesiredState == character_state::STATE_NULL )
    {
        return;
    }

    STAT_LOGGER( temp, k_stats_AI_Advance );

    m_SinceLastGetPath          += DeltaTime;
    m_TimeSinceLastDialog       += DeltaTime;
    m_RethinkRetreatTimer       += DeltaTime;
    m_TimeActive                += DeltaTime;
    m_SinceLastShot             += DeltaTime;
    m_TimeAtAwarenessLevel      += DeltaTime;
    m_TimeSinceLastCoverCheck   += DeltaTime;
    m_TimeSinceLastLookatSwitch += DeltaTime;
    m_TimeSinceLastPathToTargetCheck += DeltaTime;
    m_TimeSinceLastReload       += DeltaTime;
    m_TimeSinceLastTargetUpdate += DeltaTime;
    m_TimeSincePlayerDamagedMeLast += DeltaTime;

    f32 allClearDelayTime = k_MinTimeToSayAllClear;
    tweak_handle AllClearDelayTweak( "AllClearDelay" );
    if( AllClearDelayTweak.Exists() )    
    {
        allClearDelayTime = AllClearDelayTweak.GetF32();
    }        

    if( m_bPlayAllClearDialog && GetTargetNotSeenTimer() > allClearDelayTime )
    {
        PlayDialog( DIALOG_CLEAR );
        m_bPlayAllClearDialog = FALSE;
    }

    // decrement how long we are going to look at the thinghy.
    if( m_bIsInHeadTracking )
    {
        if( m_HeadLookatTimer > 0.0f )
        {
            m_HeadLookatTimer -= DeltaTime;
        }
    }
    else if( m_OutofLookatTimer < k_MinTimeOutsideConeToStartHeadLookat )
    {
        m_OutofLookatTimer += DeltaTime;
    }

    if( m_MinStateTime > 0.0f )
    {
        m_MinStateTime -= DeltaTime;
    }
    else
    {
        m_MinStateTime = 0.0f;
    }

    if( !m_InNavMap )
    {
        m_TimeOutofNavMap += DeltaTime;
    }

    if( GetTargetGuid() == 0 )
    {
        m_NoTargetTimer += DeltaTime;
    }
    else
    {
        m_NoTargetTimer = 0.0f;
        // tell god of our target
        god* pGod = SMP_UTIL_Get_God();
        god::TargettingData tempTargetData( GetGuid(), GetTargetGuid(), GetToTarget().LengthSquared() );
        pGod->AddTargettingData( tempTargetData );
    }

    if( m_bAutoRagdoll ) 
    {
        m_Health.Dead();
    }

    if ( IsStaggering() )
    {
        m_LastTimeStaggered     -= DeltaTime;
    }

    if( !m_WeaponsCreated )
    {
        InitInventory();
    }
    if( !m_MeleeInited )
    {    
        InitMeleeValues();
    }


    // Update our lookahead.
//    UpdateLookahead();

    //
    // Catch up with riding platform if necessary
    //
    if( !m_bDumbAndFast )
    {
        GetLocoPointer()->m_Physics.CatchUpWithRidingPlatform(DeltaTime);
        UpdateAwarenessLevel();
    }
    m_TimeSinceLastSound        += DeltaTime;

    if( !IsPlayingFullBodyLipSync() )
    {    
        AdvanceAIState(DeltaTime);
        // if we come out of this in hold state, then we are done.
        if( m_pActiveState && m_pActiveState->GetStateType() == character_state::STATE_HOLD )
        {
            return;
        }
    }

    // Check if the character is dead..
    if (m_bDead)
    {
        #ifndef X_EDITOR
        // If this is a network game, do not destroy the character
        if( g_NetworkMgr.IsOnline() )
        {
            // Let base class deal with re-spawning
            actor::OnAdvanceLogic( DeltaTime );

            // Nothing else to do
            return;
        }
        #endif

        // Lookup guid info now since we are no longer valid after "DestroyObject" has been called
        guid Guid         = GetGuid() ;
        guid CorpseGuid = m_CorpseGuid ;

        xbool bPassOnGuidToCorpse = m_bPassOnGuidToCorpse;
        xbool bDrainableBody = GetHasDrainableCorpse();

        // Destroy immediately, otherwise the dead body will be incorrectly deleted!
        g_ObjMgr.DestroyObjectEx( GetGuid(), TRUE );

        // If there's a dead body, remap to use our guid?
        object_ptr<corpse> pCorpse( CorpseGuid ) ;
        if ( pCorpse )
        {
            if (bPassOnGuidToCorpse)
            {
                pCorpse->ChangeObjectGuid ( Guid ) ;
            }
            if( bDrainableBody )
            {
                pCorpse->SetDrainable(TRUE);
            }

        }
        return;
    }


    // skip a bunch of logic if we are falling.
    // it is possible that by doing this we are screwing up guys who are spawned up in the air then told to play an anim.
    // they immediately fall and don't get to do their anim. Badness ensues.
//    if( !m_LocoFallingOverride )
    {    
        // UPDATE GOALS.
        if( m_NewGoal && !IsPlayingFullBodyLipSync() )
        {
            ChangeGoal();
            m_NewGoal = FALSE;
        }        
        m_CurrentConnectionSlotChanged = FALSE;

        // Update our pathing, but only if we are not escaping or avoiding.
        UpdatePathing();

        // Update the angle from our aim to our target
        UpdateAimToTarget();

        // see if we are stuck
        UpdateIsStuck( DeltaTime );

        // Update the connection we are currently in.
        UpdateCurrentConnection();

        // Update our pathing.
        UpdateMoveTo( DeltaTime );       

        // Update what we are supposed to lookat
        UpdateLookAt( );

        // update our current weapon
        UpdateLocoMoveStyle();

        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if( pWeapon )
        {
            // advance our logic reguarding the weapon.
            if( !m_Reloading )
            {
                UpdateFiring();
            }
            else
            {
                UpdateReloading();
            }
        }    
    }

    // update our current voice.
    UpdateVoice(DeltaTime);



    // update weather or not we want to play the gun up anims.
    if( GetLocoPointer() && m_WantsToAim && WeaponReady() )
    {
        GetLocoPointer()->SetUseAimMoveStyles( TRUE );
    }
    else
    {
        GetLocoPointer()->SetUseAimMoveStyles( FALSE );
    }

    // update collision information.
    if( m_CollidedActor != 0 )
    {
        m_TimeActorColliding += DeltaTime;
    }
    else
    {
        m_TimeActorColliding = 0.0f;
    }

    if( GetLocoPointer()->m_Physics.GetNavCollided() )
    {
        m_TimeObjectColliding += DeltaTime;
    }
    else
    {
        m_TimeObjectColliding = 0.0f;
    }
    // clear the actor we have collided with
    m_CollidedActor = 0;

    // Advance loco?
    if ( (GetLocoPointer()) && (GetLocoPointer()->IsAnimLoaded()) )
    {               
        // first update what motions we can use
        UpdateAllowMotion();

        // Advance loco
        AdvanceLoco( DeltaTime );

        // Override loco if we are jumping
        if (m_JumpTimeRemaining > 0)
        {
            HandleJumpLogic( DeltaTime );
        }

        // Get loco info
        vector3 Pos = GetLocoPointer()->GetPosition();
        radian  Yaw = GetLocoPointer()->GetYaw();

        // Compute new L2W
        matrix4 L2W;
        L2W.Identity();
        L2W.SetTranslation(Pos);
        L2W.SetRotation(radian3(0, Yaw + R_180, 0)); // SB +R_180 is legacy for old anim system 

        // Update the transform, but leave the loco alone or the yaw blending will be broke
        m_bIgnoreLocoInTransform = TRUE;
        OnTransform(L2W);
        m_bIgnoreLocoInTransform = FALSE;
    }

    if( !m_bDumbAndFast )
        UpdateCloak(DeltaTime);

    // Send out animation events
    SendAnimEvents();

    m_GoalCompleted = UpdateGoal( DeltaTime );
    if( m_PathReset )
    {
        UpdatePathing();
        UpdateMoveTo(DeltaTime);
    }

    // Update not seen timer
    if( !m_CanSeeTarget )
    {    
        m_TargetNotSeenTimer += DeltaTime;
    }

    //
    // Watch for new riding platform
    //
    if( !m_bDumbAndFast )
        GetLocoPointer()->m_Physics.WatchForRidingPlatform();

    m_CoverChanged          = FALSE;

    // Call base class
    actor::OnAdvanceLogic(DeltaTime);

    // Stuff for network support.
    #ifndef X_EDITOR
    if( GameMgr.GetGameType() != GAME_CAMPAIGN )
    {
        // This is needed to get crouching to work on the ghost.
        switch( GetLocoPointer()->GetMoveStyle() )
        {
        case loco::MOVE_STYLE_CROUCH:
        case loco::MOVE_STYLE_CROUCHAIM:
            SetIsCrouching( TRUE );
            break;
        default:
            SetIsCrouching( FALSE );
            break;
        }

        // This is needed to get the airborn stuff working on the ghost.
        character_physics& Physics = GetLocoPointer()->m_Physics;
        SetIsAirborn( Physics.IsAirborn() );

        // For now, always assume orientation changes.
        m_NetDirtyBits |= ORIENTATION_BIT;

        // This is needed to get orientation to work on the ghost.
        m_Yaw   = GetLocoPointer()->GetYaw();
        m_Pitch = GetLocoPointer()->GetAimController().GetVertAim();
    }
    #endif
    
    // Update local bbox that is used to determine render clipping 
    loco* pLoco = GetLocoPointer();
    if( pLoco )
    {
        m_BBox = pLoco->m_Player.ComputeBBox();
    }        

    // Update any effects
    if( m_pEffects )
        m_pEffects->Update( this, DeltaTime );

}

//=============================================================================

void character::UpdateLocoMoveStyle()
{
    // this code switched between prowl and runaim depending upon distance to target.
    // removed because the designers may well want the guy to prowl even if the distance is long
    // plus when switched to runaim, other code will switch it to run if no target to aim at.
    /*    if( DoingMovementGoal() )
    {
    f32 distToGoal = ( GetGoalTrueLocation() - GetPosition() ).LengthSquared();
    // if we are prowling and target is far away, then run
    if( GetLocoPointer()->GetMoveStyle() == loco::MOVE_STYLE_PROWL &&
    distToGoal > k_MaxProwlDistSqr )
    {
    GetLocoPointer()->SetMoveStyle( loco::MOVE_STYLE_RUNAIM );
    }
    // if running and target is close prowl. 
    if( GetLocoPointer()->GetMoveStyle() == loco::MOVE_STYLE_RUNAIM &&
    distToGoal < k_MinRunDistSqr &&
    !GetTargetGuid() )
    {
    GetLocoPointer()->SetMoveStyle( loco::MOVE_STYLE_PROWL );
    }
    }*/
}

//=============================================================================

void character::UpdateIsStuck( f32 DeltaTime )
{
    // guy got stuck on the stairs trying to get back into nav. 
    // He just kept running into the stairs. Removing the InNavMap 
    // check should allow him to get out of that situation and back into the nav map.

    if( DoingMovementGoal() /*&& m_InNavMap*/ )
    {    
        if( m_CurrentEscapeAttempt != ESCAPE_NONE )
        {    
            m_TimeInEscapeAttempt += DeltaTime;    
        }
        else 
        {
            m_TimeInEscapeAttempt = 0.0f;
        }

        if( (GetPosition() - m_LastMovingPosition).LengthSquared() < k_MinDistForStuck * k_MinDistForStuck )
        {
            m_TimeStuck+=DeltaTime;
        }
        else
        {
            m_TimeStuck = 0.0f;
            m_LastMovingPosition = GetPosition();
        }
        m_IsStuck = ( m_TimeStuck >= k_MinTimeToBeStuck );
    }
    else
    {
        m_CurrentEscapeAttempt = ESCAPE_NONE;
        m_TimeInEscapeAttempt = 0.0f;
        m_TimeStuck = 0.0f;
        m_LastMovingPosition = GetPosition();
        m_EscapeAttempts = 0;
    }
}

//=============================================================================

void character::UpdateEscape()
{    
    if( DoingMovementGoal() )
    {
        xbool attemptingEscape = (m_CurrentEscapeAttempt != ESCAPE_NONE );
        xbool escapeSucceeded = FALSE;
        xbool escapeCompleted = FALSE;

        if( attemptingEscape )
        {
            escapeCompleted = UpdateEscapeAttempt(escapeSucceeded);
        }

        // if we are stuck and not attempting escape, or we failed our escape attempt then change our escape attempt
        if( (m_IsStuck && !attemptingEscape) ||
            (attemptingEscape && escapeCompleted && !escapeSucceeded) )
        {
            ChangeEscapeAttempt();
        }
        // if we succeeded at our escape attempt, then clear it.
        else if( (attemptingEscape && escapeCompleted && escapeSucceeded) )
        {
            m_CurrentEscapeAttempt = ESCAPE_NONE;
            m_EscapeAttempts = 0;   
            m_TimeStuck = 0.0f;
        }
    }
    else
    {
        m_CurrentEscapeAttempt = ESCAPE_NONE;
        m_EscapeAttempts = 0;   
        m_TimeStuck = 0.0f;
    }
}

//=============================================================================

xbool character::UpdateEscapeAttempt( xbool& success )
{
    switch(m_CurrentEscapeAttempt) 
    {
    case ESCAPE_GO_LEFT:
        {
            // always run for a min amount of time. 
            if( m_TimeInEscapeAttempt > k_MinTimeInEscapeAttempt )
            {
                // if we are still stuck, we have failed.
                if( m_IsStuck )
                {
                    success = FALSE;
                    return TRUE;
                }
                // are we completed?
                else
                {
                    // if it is safe to move towards our nav,
                    // or we can no longer go left then we are done
                    if( GetCanMoveForward() || !GetCanMoveLeft() )
                    {
                        m_TimeInEscapeAttempt = 0.0f;
                        m_CurrentEscapeAttempt = ESCAPE_SLIGHT_DELAY;
                    }
                }
            }
        }
        break;
    case ESCAPE_GO_RIGHT:
        {
            // always run for a min amount of time. 
            if( m_TimeInEscapeAttempt > k_MinTimeInEscapeAttempt )
            {
                // if we are still stuck, we have failed.
                if( m_IsStuck )
                {
                    success = FALSE;
                    return TRUE;
                }
                // are we completed?
                else
                {
                    // if it is safe to move towards our nav,
                    // or we can no longer go left then we are done
                    if( GetCanMoveForward() || !GetCanMoveRight() )
                    {
                        m_TimeInEscapeAttempt = 0.0f;
                        m_CurrentEscapeAttempt = ESCAPE_SLIGHT_DELAY;
                        //                        success = TRUE;
                        //                        return TRUE;
                    }
                }
            }
        }
        break;
    case ESCAPE_GO_BACK_LEFT:
        {
            // always run for a min amount of time. 
            if( m_TimeInEscapeAttempt > k_MinTimeInEscapeAttempt )
            {
                // if we are still stuck, we have failed.
                if( m_IsStuck )
                {
                    success = FALSE;
                    return TRUE;
                }
                // are we completed?
                else
                {
                    // this has 2 parts, first move back until we can go left
                    if( !m_EscapeDoneMovingBack )
                    {
                        vector3 checkLeft(0.0f,0.0f,200.0f);
                        if( GetCanMoveLeft() || !GetCanMoveBack() )
                        {
                            // start moveing left and reset the timer
                            m_EscapeDoneMovingBack = TRUE;
                            m_TimeInEscapeAttempt = 0.0f;
                            m_UnstuckVector = checkLeft;
                        }
                    }
                    // then go left until we can go forward
                    else
                    {
                        if( GetCanMoveForward() || !GetCanMoveLeft() )
                        {
                            m_TimeInEscapeAttempt = 0.0f;
                            m_CurrentEscapeAttempt = ESCAPE_SLIGHT_DELAY;
                            //                        success = TRUE;
                            //                        return TRUE;
                        }
                    }
                }
            }
        }
        break;
    case ESCAPE_GO_BACK_RIGHT:
        {
            // always run for a min amount of time. 
            if( m_TimeInEscapeAttempt > k_MinTimeInEscapeAttempt )
            {
                // if we are still stuck, we have failed.
                if( m_IsStuck )
                {
                    success = FALSE;
                    return TRUE;
                }
                // are we completed?
                else
                {
                    // this has 2 parts, first move back until we can go left
                    if( !m_EscapeDoneMovingBack )
                    {
                        vector3 checkRight(0.0f,0.0f,200.0f);
                        if( GetCanMoveRight() || !GetCanMoveBack() )
                        {
                            // start moveing left and reset the timer
                            m_EscapeDoneMovingBack = TRUE;
                            m_TimeInEscapeAttempt = 0.0f;
                            m_UnstuckVector = checkRight;
                        }
                    }
                    // then go left until we can go forward
                    else
                    {
                        if( GetCanMoveForward() || !GetCanMoveRight() )
                        {
                            m_TimeInEscapeAttempt = 0.0f;
                            m_CurrentEscapeAttempt = ESCAPE_SLIGHT_DELAY;
                        }
                    }
                }
            }
        }
        break;
    case ESCAPE_SLIGHT_DELAY:
        if( m_TimeInEscapeAttempt > k_EscapeDelayTime )
        {
            success = TRUE;
            return TRUE;
        }
        break;
    case ESCAPE_HEAD_EXPLODE:
        {
            m_Health.Dead();
        }
        break;
    }
    // if we've made it here we are not done yet.
    return FALSE;
}

//=============================================================================

void character::ChangeEscapeAttempt()    
{    
    xbool validUnstickDirection = FALSE;
    while( !validUnstickDirection )
    {
        switch(m_LastEscapeAttempt) 
        {
        case ESCAPE_NONE:
            {
                vector3 checkLeft(0.0f,0.0f,200.0f);
                m_CurrentEscapeAttempt = ESCAPE_GO_LEFT;
                // check left for validity
                if( GetCanMoveLeft() )
                {
                    m_UnstuckVector = checkLeft;
                    validUnstickDirection = TRUE;
                }
            }
            break;
        case ESCAPE_GO_LEFT:
            {
                vector3 checkRight(0.0f,0.0f,200.0f);
                m_CurrentEscapeAttempt = ESCAPE_GO_RIGHT;
                // check left for validity
                if( GetCanMoveRight() )
                {
                    m_UnstuckVector = checkRight;
                    validUnstickDirection = TRUE;
                }
            }
            break;
        case ESCAPE_GO_RIGHT:
            {
                vector3 checkBack(0.0f,0.0f,-200.0f);
                m_CurrentEscapeAttempt = ESCAPE_GO_BACK_RIGHT;
                // check left for validity
                if( GetCanMoveBack() )
                {
                    m_UnstuckVector = checkBack;
                    validUnstickDirection = TRUE;
                }
            }
            break;
        case ESCAPE_GO_BACK_LEFT:
            {
                vector3 checkLeft(0.0f,0.0f,200.0f);
                m_CurrentEscapeAttempt = ESCAPE_GO_LEFT;
                // check left for validity
                if( GetCanMoveLeft() )
                {
                    m_UnstuckVector = checkLeft;
                    validUnstickDirection = TRUE;
                }
            }
            break;
        case ESCAPE_GO_BACK_RIGHT:
            {
                vector3 checkBack(0.0f,0.0f,-200.0f);
                m_CurrentEscapeAttempt = ESCAPE_GO_BACK_LEFT;
                // check for validity of going back.
                if( GetCanMoveBack() )
                {
                    m_UnstuckVector = checkBack;
                    validUnstickDirection = TRUE;
                }
            }
            break;
        }
        // if we've tried em all, pretend we aren't stuck... try again.
        if( m_EscapeAttempts >= ESCAPE_HEAD_EXPLODE )
        {
            m_CurrentEscapeAttempt = ESCAPE_NONE;
            validUnstickDirection = TRUE;
            m_EscapeAttempts = 0;
            m_TimeStuck = 0.0f;
        }
        else
        {        
            m_EscapeAttempts++;
        }
        m_LastEscapeAttempt = m_CurrentEscapeAttempt;
    }
    m_EscapeDoneMovingBack = FALSE;
    m_TimeInEscapeAttempt = 0.0f;
}

//=============================================================================

const char* character::GetEscapeAttemptName()
{
    switch(m_CurrentEscapeAttempt) 
    {
    case ESCAPE_NONE:
        return "ESCAPE_NONE";
        break;
    case ESCAPE_GO_LEFT:
        return "ESCAPE_GO_LEFT";
        break;
    case ESCAPE_GO_RIGHT:
        return "ESCAPE_GO_RIGHT";
        break;
    case ESCAPE_GO_BACK_LEFT:
        return "ESCAPE_GO_BACK_LEFT";
        break;
    case ESCAPE_GO_BACK_RIGHT:
        return "ESCAPE_GO_BACK_RIGHT";
        break;
    case ESCAPE_HEAD_EXPLODE:
        return "ESCAPE_HEAD_EXPLODE";
        break;
    case ESCAPE_SLIGHT_DELAY:
        return "ESCAPE_SLIGHT_DELAY";
        break;
    default:
        return "ESCAPE_UNKNOWN_TYPE";
    }
}
//=============================================================================

const char* character::GetAvoidAttemptName()
{
    switch(m_AvoidActorAttempt) 
    {
    case AVOID_NONE:
        return "AVOID_NONE";
        break;
    case AVOID_THROUGH:
        return "AVOID_THROUGH";
        break;
    case AVOID_WAIT:
        return "AVOID_WAIT";
        break;
    default:
        return "ESCAPE_UNKNOWN_TYPE";
    }
}

//=============================================================================

const char* character::GetAwarenessLevelName()
{
    switch(GetAwarenessLevel()) 
    {
    case AWARENESS_NONE:
        return "AWARENESS_NONE";
        break;
    case AWARENESS_ALERT:
        return "AWARENESS_ALERT";
        break;
    case AWARENESS_COMBAT_READY:
        return "AWARENESS_COMBAT_READY";
        break;
    case AWARENESS_SEARCHING:
        return "AWARENESS_SEARCHING";
        break;
    case AWARENESS_ACQUIRING_TARGET:
        return "AWARENESS_ACQUIRING_TARGET";
        break;
    case AWARENESS_TARGET_LOST:
        return "AWARENESS_TARGET_LOST";
        break;
    case AWARENESS_TARGET_SPOTTED:
        return "AWARENESS_TARGET_SPOTTED";
        break;
    default:
        return "AWARENESS_UNKNOWN_TYPE";
    }
}

//=============================================================================

xbool character::DoingMovementGoal()
{
    return ( m_GoalInfo.m_GoalType == GOAL_GOTO_LOCATION ||
        m_GoalInfo.m_GoalType == GOAL_GOTO_TARGET ||
        m_GoalInfo.m_GoalType == GOAL_RETREAT_FROM_LOCATION ||
        m_GoalInfo.m_GoalType == GOAL_RETREAT_FROM_TARGET );   
}

//=============================================================================
//
//  HandleJumpLogic - NOTE: This is just prototype code.
//                    Arcs aren't properly represented,
//                    and neither is the anim driver.
//
void character::HandleJumpLogic( f32 DeltaTime )
{
    loco* pLoco = GetLocoPointer();

    m_JumpTimeRemaining -= DeltaTime;

    // Update move-at and look-at positions
    s32 iStepInPathToMoveAt = m_CurrentPathStructIndex+2;
    if (iStepInPathToMoveAt >= m_PathFindStruct.m_nSteps)
        iStepInPathToMoveAt = m_PathFindStruct.m_nSteps-1;

    vector3 MoveAtPos;

    nav_node_slot_id    iMoveAtNode = m_PathFindStruct.m_StepData[ iStepInPathToMoveAt ].m_NodeToPassThrough;
    if ( NULL_NAV_SLOT == iMoveAtNode )
        MoveAtPos = m_PathFindStruct.m_vEndPoint;
    else
    {
        const ng_node2&     MoveAtNode  = g_NavMap.GetNodeByID( iMoveAtNode );
        MoveAtPos = MoveAtNode.GetCenter();
    }

    pLoco->SetMoveAt( MoveAtPos );
    pLoco->SetLookAt( MoveAtPos + vector3(0,100,0) );

    vector3 CurPos = GetPosition();
    radian DesiredYaw = (MoveAtPos - CurPos).GetYaw();

    pLoco->SetYawFacingTarget( DesiredYaw, R_30 );

    // Turn off aiming
    pLoco->SetUseAimMoveStyles( FALSE );
    pLoco->SetMoveStyle( loco::MOVE_STYLE_RUN );

    if (m_JumpTimeRemaining <= 0.0f)
    {
        // Handle end of jump
        m_CurrentPathStructIndex ++;
        if (m_PathFindStruct.m_StepData[ m_CurrentPathStructIndex ].m_CurrentConnection != NULL_NAV_SLOT)
        {        
            m_CurrentConnectionSlot = m_PathFindStruct.m_StepData[ m_CurrentPathStructIndex ].m_CurrentConnection;
            m_CurrentConnectionSlotChanged = TRUE;
        }
        LOG_MESSAGE("character::HandleJumpLogic", "Jump completed" );
        pLoco->m_Physics.SetLocoGravityOn( TRUE );
        m_DesiredLocation = MoveAtPos;
        m_NextPathPoint = m_DesiredLocation;
        CalculateSplinePoints();
        m_PreviousPathPoint = GetPosition();
        //pLoco->SetMoveAt( GetPosition() );
        return;
    }

    vector3 Delta;
    vector3 Base;
    f32     T = 0;
    if (m_JumpTimeRemaining > (m_TotalJumpTime-m_JumpTimeToApex))
    {
        // Jumping Start->Apex
        Delta = m_JumpApexPos - m_JumpStartPos;
        T = m_TotalJumpTime-m_JumpTimeRemaining;
        T /= m_JumpTimeToApex;
        Base = m_JumpStartPos;
    }
    else
    {
        // Jumping Apex->LZ
        Delta = (m_JumpLandPos+vector3(0,-45,0)) - m_JumpApexPos;
        T = m_JumpTimeRemaining;
        T /= (m_TotalJumpTime-m_JumpTimeToApex);
        T = 1-T;
        Base = m_JumpApexPos;
    }  

    f32     AnimT = 1.0f - (m_JumpTimeRemaining / m_TotalJumpTime);

    Delta.Scale(T);

    matrix4 L2W = pLoco->GetL2W();

    L2W.SetTranslation( Base + Delta );

    pLoco->SetL2W( L2W );

    pLoco->m_Player.SetCurrAnimFrame( AnimT * pLoco->m_Player.GetNFrames() );
    pLoco->m_Physics.SetLocoGravityOn( FALSE );



}

//=============================================================================

void character::CalculateJumpInfo( void )
{
    ASSERT( m_CurrentPathStructIndex < (m_PathFindStruct.m_nSteps-1) );

    //
    //  Start position
    //
    m_JumpStartPos = GetPosition();

    //
    //  Calculate the landing position
    //
    //  The landing pos will be a point inside the overlap between the 2nd jump 
    //  connection, and the one that follows it.
    nav_connection_slot_id  iJump1Conn;
    nav_connection_slot_id  iJump2Conn;
    nav_connection_slot_id  iLZConn;
    nav_node_slot_id        iApexNode;
    nav_node_slot_id        iLZNode;

    iJump1Conn = m_PathFindStruct.m_StepData[m_CurrentPathStructIndex].m_CurrentConnection;
    iJump2Conn = m_PathFindStruct.m_StepData[m_CurrentPathStructIndex].m_DestConnection;
    iApexNode  = m_PathFindStruct.m_StepData[m_CurrentPathStructIndex].m_NodeToPassThrough;

    iLZConn    = m_PathFindStruct.m_StepData[m_CurrentPathStructIndex+1].m_DestConnection;
    iLZNode    = m_PathFindStruct.m_StepData[m_CurrentPathStructIndex+1].m_NodeToPassThrough;

#ifdef X_ASSERT
    const ng_connection2& CurConn = g_NavMap.GetConnectionByID( m_PathFindStruct.m_StepData[m_CurrentPathStructIndex].m_CurrentConnection );
    ASSERT( CurConn.GetFlags() & ng_connection2::HINT_JUMP );
#endif
    //==---------------------------------------------------
    //  To get a point on the overlap, choose two verts at random.
    //  Pick a point anywhere on the line segment between those
    //  two verts.
    //==---------------------------------------------------
    const ng_node2& LZNode = g_NavMap.GetNodeByID( iLZNode );
    //s32 nVerts = LZNode.GetVertCount();

    // For now, just jump to the center
    m_JumpLandPos = LZNode.GetPosition();

    //==---------------------------------------------------
    //  Figure out the time required and the horizontal
    //  speed of the jump
    //==---------------------------------------------------
    vector3 DeltaXZ = m_JumpLandPos - m_JumpStartPos;
    DeltaXZ.GetY() = 0;

    f32     HorizDist = DeltaXZ.Length();

    m_TotalJumpTime = HorizDist / m_JumpVelocity;    

    //==---------------------------------------------------
    // Calculate the apex point
    //==---------------------------------------------------
    const ng_node2& ApexNode = g_NavMap.GetNodeByID( iApexNode );
    vector3 ApexPt = ApexNode.GetPosition();
    f32     TimeToApex = 0;

    {
        vector3 A,B,C;
        A = m_JumpStartPos;
        B = m_JumpLandPos;
        C = ApexPt;

        A.GetY() = 0;
        B.GetY() = 0;
        C.GetY() = 0;

        ApexPt = C.GetClosestPToLSeg( A, B );
        TimeToApex = (ApexPt-A).Length();
    }
    ApexPt.GetY() = ApexNode.GetPosition().GetY();

    m_JumpApexPos = ApexPt;

    TimeToApex /= m_JumpVelocity;

    m_JumpTimeToApex = TimeToApex;

    //==---------------------------------------------------
    //  Setup
    //==---------------------------------------------------
    m_JumpTimeRemaining = m_TotalJumpTime;

    LOG_MESSAGE("character::CalculateJumpInfo", "Starting to jump" );

    GetLocoPointer()->PlayAnim( loco::ANIM_JUMP_OVER );
    GetLocoPointer()->SetLookAt( m_JumpLandPos );    
}

//=============================================================================

#ifdef X_EDITOR
void character::SetupDebugTextBlock( s32 nLines, s32 nCharsWide, xcolor Color )
{
    //
    //  Compute the start location in the world where
    //  we can start drawing labels
    //
    nLines = MAX(0,m_nPreviousRequestedLines);

    rect        Rect;
    bbox BBox = GetLocoPointer()->m_Physics.GetBBox();
    BBox.Transform( GetL2W() );

    vector3     Pos     = BBox.GetCenter();
    const view& View    = *eng_GetView();
    vector3 ScreenPos   = View.PointToScreen( Pos );
    f32     ScreenSize  = View.CalcScreenSize( Pos, BBox.GetRadius() );

    View.GetViewport( Rect );

    vector2 ScreenCenter = Rect.GetCenter();
    vector2 ScreenPos2D( ScreenPos.GetX(), ScreenPos.GetY() );
    vector2 VecToCenter = ScreenCenter - ScreenPos2D;

    VecToCenter.Normalize();

    // Compute a couple of dot products while we have it normalized
    f32 Vertical   = VecToCenter.Dot( vector2(0,-1) );
    f32 Horizontal = VecToCenter.Dot( vector2(-1,0) );

    Vertical = (Vertical+1.0f)/2.0f;
    Horizontal = (Horizontal+1.0f)/2.0f;


    VecToCenter.Scale( ScreenSize/2.0f );

    m_ScreenBlockCursor = ScreenPos2D + VecToCenter;   

    m_nScreenBlockMaxLines = nLines;
    m_iScreenBlockLine = 0;

    s32     CharWidth = 5;
    s32 Width = (s32)(nCharsWide * CharWidth);
    s32 Height = 10*nLines;


    m_ScreenBlockCursor.X -= (Width * Horizontal);
    m_ScreenBlockCursor.Y -= (Height * Vertical);

    irect   R;

    R.Set( (s32) m_ScreenBlockCursor.X, 
        (s32) m_ScreenBlockCursor.Y, 
        (s32)(m_ScreenBlockCursor.X + Width), 
        (s32)(m_ScreenBlockCursor.Y + Height) );
    draw_Rect( R, Color, FALSE );

    m_nPreviousRequestedLines = 0;
}

void character::PrintToTextBlock( xcolor Color, const char* pFormatString, ... )
{
    m_nPreviousRequestedLines++;

    if (m_iScreenBlockLine >= m_nScreenBlockMaxLines)
        return;

    char    Buf[ 256 ];
    x_va_list   Args;
    x_va_start( Args, pFormatString );
    x_vsprintf( Buf, pFormatString, Args );

    text_PushColor( Color );

    text_PrintPixelXY( Buf, (s32)m_ScreenBlockCursor.X, (s32)m_ScreenBlockCursor.Y );
    m_ScreenBlockCursor.Y += 10.0f;

    text_PopColor();
}
#endif // X_EDITOR

const char* character::GetLookatStateName()
{
    switch (m_LookatMode)
    {
    case LOOKAT_NONE:
        return "LOOKAT_NONE";
        break;
    case LOOKAT_FORWARD:                
        return "LOOKAT_FORWARD";
        break;
    case LOOKAT_LAST_LOCATION_OF_INTEREST:
        return "LOOKAT_LAST_LOCATION_OF_INTEREST";
        break;
    case LOOKAT_NAVIGATION:
        return "LOOKAT_NAVIGATION";
        break;
    case LOOKAT_NAV_FORWARD:
        return "LOOKAT_NAV_FORWARD";
        break;
    case LOOKAT_MARKER_ALIGN:
        return "LOOKAT_MARKER_ALIGN";
        break;
    case LOOKAT_CURRENT_TARGET:
        return "LOOKAT_CURRENT_TARGET";
        break;
    case LOOKAT_ENTERING_COVER:
        return "LOOKAT_ENTERING_COVER";
        break;
    case LOOKAT_DOING_GOAL:
        return "LOOKAT_DOING_GOAL";
        break;
    case LOOKAT_INTEREST_OBJECT:
        return "LOOKAT_INTEREST_OBJECT";
        break;
    default:
        return "UNKNOWN_LOOKAT_MODE";
    }
}
//=============================================================================

#if !defined(X_RETAIL) || defined(X_QA)
void character::OnDebugRender( void )
{
    // Lookup skin geometry
    skin_geom* pSkinGeom = m_SkinInst.GetSkinGeom();
    if (!pSkinGeom)
        return;

    // Compute LOD mask
    u64 LODMask = m_SkinInst.GetLODMask(GetL2W()) ;
    if (LODMask == 0)
        return ;

    // Render bboxes
    draw_ClearL2W();
    draw_SetL2W( GetL2W() );
    draw_BBox( m_BBox, XCOLOR_GREEN );
    draw_ClearL2W();
    draw_BBox( GetBBox(), XCOLOR_RED );

    //
    // Fire a sphere out from the eye the correct distance w/offset and 
    // determine if we hit anything.
    //
    {
        vector3 StartPos;
        vector3 EndPos;
        f32 MeleeSphereRadius;

        // load up start, end and radius for melee
        GetMeleeInfo( StartPos, EndPos, MeleeSphereRadius );   

        draw_Line( StartPos, EndPos, XCOLOR_RED );
        draw_Sphere( EndPos, MeleeSphereRadius, XCOLOR_RED);
    }

    // Draw current pain radius
    if( m_CurrentPainRadius > 0.1f )
    {
        draw_Sphere( m_CurrentPainCenter, m_CurrentPainRadius, XCOLOR_RED);
    }
    m_CurrentPainRadius = 0.0f;

    // sbroumley - I added this so we can see what's going on, on the PS2
#if defined( ENABLE_DEBUG_MENU )

    // Show loco info
    {
        GetLocoPointer()->RenderInfo(TRUE,               // bRenderLoco
            g_GameLogicDebug,   // bLabelLoco (only draw in the editor)
            FALSE,              // bRenderSkeleton
            FALSE) ;            // bLabelSkeleton
    }
    
    // Show type of pathing
    if( m_PathFindStruct.m_bStraightPath )
        draw_Label( GetPosition(), XCOLOR_WHITE, "S" );
    else        
        draw_Label( GetPosition(), XCOLOR_WHITE, "p" );

    if( !CanReachTarget() )
    {    
        draw_Label( GetPositionWithOffset( OFFSET_TOP_OF_BBOX ), XCOLOR_WHITE, "Can't reach target!" );
    }

#endif

#ifdef X_EDITOR
    SetupDebugTextBlock( 12, 70, xcolor(0,0,50,255) );
#endif // X_EDITOR

#ifdef X_EDITOR
    s32 i;
    
    // Debug loco?
    if( ( s_bDebugLoco ) && ( GetLocoPointer() ) )
    {
        // Show info
        GetLocoPointer()->RenderInfo( TRUE,             // bRenderLoco
                                      g_GameLogicDebug, // bLabelLoco (only draw in the editor)
                                      FALSE,            // bRenderSkeleton
                                      FALSE ) ;         // bLabelSkeleton
    }
    
    if( g_game_running )
    {
        // do these when the game is running
        if (s_bDebugAI)
        {
            object *targetObject = g_ObjMgr.GetObjectByGuid(GetTargetGuid());
            if( targetObject )
            {
                new_weapon* pWeapon = GetCurrentWeaponPtr();
                if( pWeapon )
                {                
                    vector3 FirePoint;
                    if( pWeapon->GetFiringBonePosition(FirePoint) )
                    {
                        vector3 vShootAt = GetTargetPosWithOffset(GetTargetGuid(),OFFSET_AIM_AT);
                        if( !CanSeeTarget() )
                        {
                            vShootAt = GetLastSeenLocationOfTarget();
                        }
                        draw_Line( FirePoint, vShootAt, XCOLOR_PURPLE );                
                    }
                }

                object *coverObject = g_ObjMgr.GetObjectByGuid( GetCurrentCover() );
                if( coverObject )
                {
                    xcolor coverColor;
                    if( GetCoverIsValid() )
                    {
                        coverColor = XCOLOR_GREEN;
                    }
                    else
                    {
                        coverColor = XCOLOR_RED;
                    }
                    draw_Line( GetPosition(), coverObject->GetPosition(),coverColor );
                    draw_BBox( bbox(coverObject->GetPosition(),50.0f), coverColor );

                }
                draw_Label( GetTargetPosWithOffset(OFFSET_TOP_OF_BBOX), XCOLOR_WHITE, xfs("Hit Chance: [%d]", GetHitChance()) );
            }

            // Location stuff
            draw_Sphere( m_LastSeenLocationOfTarget, 25, XCOLOR_YELLOW );
            PrintToTextBlock( XCOLOR_YELLOW, "Health:%.2f", GetHealth() );
            if (m_pActiveState)
                PrintToTextBlock( XCOLOR_YELLOW, GetStateName(m_pActiveState->m_State) );
            loco* pLoco = GetLocoPointer();                
            if( m_pActiveState && pLoco )
            {   
                // Lookup current anim info                            
                loco_motion_controller& CurrAnim  = pLoco->m_Player.GetCurrAnim();
                s32 nCurrAnimType = 1;
                s32 iCurrAnim     = CurrAnim.GetAnimIndex();
                s32 iCurrAnimType = CurrAnim.GetAnimTypeIndex();
                s32 iCurrAnimLoopFrame = 0;
                if( ( iCurrAnim != -1 ) && ( iCurrAnimType != -1 ) )
                {
                    const anim_info& AnimInfo = CurrAnim.GetAnimInfo( iCurrAnimType );
                    iCurrAnimLoopFrame = AnimInfo.GetLoopFrame();
                    nCurrAnimType = AnimInfo.GetNAnims();
                    iCurrAnim -= iCurrAnimType;
                }

                // Lookup blend anim info                            
                loco_motion_controller& BlendAnim  = pLoco->m_Player.GetBlendAnim();
                s32 nBlendAnimType = 1;
                s32 iBlendAnim     = BlendAnim.GetAnimIndex();
                s32 iBlendAnimType = BlendAnim.GetAnimTypeIndex();
                s32 iBlendAnimLoopFrame = 0;
                if( ( iBlendAnim != -1 ) && ( iBlendAnimType != -1 ) )
                {
                    const anim_info& AnimInfo = BlendAnim.GetAnimInfo( iBlendAnimType );
                    iBlendAnimLoopFrame = AnimInfo.GetLoopFrame();
                    nBlendAnimType = AnimInfo.GetNAnims();
                    iBlendAnim -= iBlendAnimType;
                }

                // Lookup lip sync
                loco_lip_sync_controller& LipSyncCont = pLoco->GetLipSyncController();

                PrintToTextBlock( XCOLOR_YELLOW, "State:%s",m_pActiveState->GetStateName() );
                PrintToTextBlock( XCOLOR_YELLOW, "Phase:%s",m_pActiveState->GetPhaseName() );
                PrintToTextBlock( XCOLOR_YELLOW, "GoalType:%s", GetGoalTypeName());
                PrintToTextBlock( XCOLOR_YELLOW, "AnimGroup:%s", CurrAnim.GetAnimGroupHandle().GetName() );
                
                if( nCurrAnimType == 1 )
                    PrintToTextBlock( XCOLOR_YELLOW, "AnimName:%s", CurrAnim.GetAnimName() ) ;
                else                    
                    PrintToTextBlock( XCOLOR_YELLOW, "AnimName:%s (%d of %d)", CurrAnim.GetAnimName(), iCurrAnim+1, nCurrAnimType ) ;
                PrintToTextBlock( XCOLOR_YELLOW, "AnimFrame:%d-%d LoopFrame:%d Rate:%.2f", (s32)CurrAnim.GetFrame(), CurrAnim.GetNFrames(), iCurrAnimLoopFrame, CurrAnim.GetRate() );
                
                if( pLoco->m_Player.IsBlending() )
                {
                    PrintToTextBlock( XCOLOR_YELLOW, "BlendAnimGroup:%s", BlendAnim.GetAnimGroupHandle().GetName() );
                
                    if( nBlendAnimType == 1 )
                        PrintToTextBlock( XCOLOR_YELLOW, "BlendAnimName:%s", BlendAnim.GetAnimName() ) ;
                    else                    
                        PrintToTextBlock( XCOLOR_YELLOW, "BlendAnimName:%s (%d of %d)", BlendAnim.GetAnimName(), iBlendAnim+1, nBlendAnimType ) ;
                    PrintToTextBlock( XCOLOR_YELLOW, "BlendAnimFrame:%d-%d LoopFrame:%d Rate:%.2f", (s32)BlendAnim.GetFrame(), BlendAnim.GetNFrames(), iBlendAnimLoopFrame, BlendAnim.GetRate() );
                }

                // Show additive anim0 info?
                loco_additive_controller& AddCont0 = pLoco->GetAdditiveController( loco::ANIM_FLAG_CONTROLLER0 );
                if( AddCont0.IsPlaying() )
                {
                    PrintToTextBlock( XCOLOR_YELLOW, "Additive0AnimName:%s", AddCont0.GetAnimName() );
                    PrintToTextBlock( XCOLOR_YELLOW, "Additive0AnimFrame:%d-%d Rate:%.2f", (s32)AddCont0.GetFrame(), AddCont0.GetNFrames(), AddCont0.GetRate() );
                }

                // Show additive anim1 info?
                loco_additive_controller& AddCont1 = pLoco->GetAdditiveController( loco::ANIM_FLAG_CONTROLLER1 );
                if( AddCont1.IsPlaying() )
                {
                    PrintToTextBlock( XCOLOR_YELLOW, "Additive1AnimName:%s", AddCont1.GetAnimName() );
                    PrintToTextBlock( XCOLOR_YELLOW, "Additive1AnimFrame:%d-%d Rate:%.2f", (s32)AddCont1.GetFrame(), AddCont1.GetNFrames(), AddCont1.GetRate() );
                }

                // Show lip sync info?
                if( LipSyncCont.IsPlaying() )
                {
                    PrintToTextBlock( XCOLOR_YELLOW, "LipSyncAnimGroup:%s", LipSyncCont.GetAnimGroupHandle().GetName() );
                    PrintToTextBlock( XCOLOR_YELLOW, "LipSyncAnimName:%s", LipSyncCont.GetAnimName() );
                    PrintToTextBlock( XCOLOR_YELLOW, "LipSyncAnimFrame:%d-%d Rate:%.2f", (s32)LipSyncCont.GetFrame(), LipSyncCont.GetNFrames(), LipSyncCont.GetRate() );
                        
                    u32 LipSyncFlags = LipSyncCont.GetAnimFlags();
                    PrintToTextBlock( XCOLOR_YELLOW, 
                                      "FullBody:%d UpperBody:%d Face:%d", 
                                      ( LipSyncFlags & loco::ANIM_FLAG_MASK_TYPE_FULL_BODY ) != 0,
                                      ( LipSyncFlags & loco::ANIM_FLAG_MASK_TYPE_UPPER_BODY ) != 0,
                                      ( LipSyncFlags & loco::ANIM_FLAG_MASK_TYPE_FACE ) != 0 );
                                      
                    // Display audio info?
                    s32 VoiceID = LipSyncCont.GetVoiceID();
                    if( g_AudioMgr.IsValidVoiceId( VoiceID ) )
                    {
                        // Lookup info
                        const char* pAudioName   = g_AudioMgr.GetVoiceDescriptor( VoiceID );
                        s32         AudioFrame   = (s32)( LipSyncCont.GetAudioTime() * 30.0f );
                        s32         AudioNFrames = (s32)( g_AudioMgr.GetLengthSeconds  ( VoiceID ) * 30.0f );

                        // Show info                        
                        PrintToTextBlock( XCOLOR_YELLOW, "LipSyncAudioName :%s",   pAudioName );
                        PrintToTextBlock( XCOLOR_YELLOW, "LipSyncAudioFrame:%d-%d", AudioFrame, AudioNFrames );
                    }
                }

                PrintToTextBlock( XCOLOR_YELLOW, "LocoState:%s",pLoco->GetStateName() );
                PrintToTextBlock( XCOLOR_YELLOW, "LocoStyle:%s",pLoco->GetMoveStyleName( pLoco->GetMoveStyle()) );
                PrintToTextBlock( XCOLOR_YELLOW, "LookatState:%s", GetLookatStateName()) ;
                
                PrintToTextBlock( XCOLOR_YELLOW, "LocoMotion:%s FaceIdles:%d", pLoco->GetMotionName( pLoco->GetMotion() ), 
                    ( pLoco->IsFaceIdleEnabled() ) && ( pLoco->GetAnimIndex( loco::ANIM_FACE_IDLE ) != -1 ) ); 
                    
                PrintToTextBlock( XCOLOR_YELLOW, "AccumYaw:%d AccumHoriz:%d AccumVert:%d", 
                    CurrAnim.GetAccumYawMotion(),
                    CurrAnim.GetAccumHorizMotion(),
                    CurrAnim.GetAccumVertMotion() );
                PrintToTextBlock( XCOLOR_YELLOW, "Collision:%d Gravity:%d",
                    CurrAnim.GetWorldCollision(),
                    CurrAnim.GetGravity() );
                PrintToTextBlock( XCOLOR_YELLOW, "Loco ExactMove:%d ExactLook:%d",
                    pLoco->IsExactMove(),
                    pLoco->IsExactLook() );
                PrintToTextBlock( XCOLOR_YELLOW, "Loco Prop:%d CinRel:%d CovRel:%d",
                    CurrAnim.IsUsingMotionProp(),
                    CurrAnim.IsCinemaRelativeMode(),
                    CurrAnim.IsCoverRelativeMode() );
                PrintToTextBlock( XCOLOR_YELLOW, "Blending:%d",pLoco->m_Player.IsBlending()) ;
                PrintToTextBlock( XCOLOR_YELLOW, "EscapeAttempt:%s",GetEscapeAttemptName()) ;
                PrintToTextBlock( XCOLOR_YELLOW, "AvoidAttempt:%s",GetAvoidAttemptName()) ;
                PrintToTextBlock( XCOLOR_YELLOW, "AwarenessLevel:%s",GetAwarenessLevelName()) ;
                if( CanSeeTarget() )
                {
                    PrintToTextBlock( XCOLOR_RED, "Can See Target" );
                }
                if( CanShootAtTarget() )
                {
                    PrintToTextBlock( XCOLOR_RED, "Can Shoot At Target" );
                }
                if( GetWantsToFirePrimary() )
                {
                    PrintToTextBlock( XCOLOR_RED, "Wants to shoot target" );
                }
                if( CanReachTarget() )
                {
                    PrintToTextBlock( XCOLOR_RED, "Can Reach Target" );
                }
                if( CanPathToTarget() )
                {
                    PrintToTextBlock( XCOLOR_RED, "Can Path To Target" );
                }
                if( GetCanMoveForward() )
                {
                    PrintToTextBlock( XCOLOR_BLUE, "Can Move Forward" );
                }
                if( GetCanMoveBack() )
                {
                    PrintToTextBlock( XCOLOR_BLUE, "Can Move Back" );
                }
                if( GetCanMoveLeft() )
                {
                    PrintToTextBlock( XCOLOR_BLUE, "Can Move Left" );
                }
                if( GetCanMoveRight() )
                {
                    PrintToTextBlock( XCOLOR_BLUE, "Can Move Right" );
                }
            }

            // Draw sight info
            radian ArcYaw = GetLocoPointer()->GetSightYaw();
            draw_Arc( GetPosition() + vector3(0,10,0), m_LightSightRadius, ArcYaw, m_IdleSightFOV,  XCOLOR_BLUE     );
            draw_Arc( GetPosition() + vector3(0,20,0), m_DarkSightRadius,  ArcYaw, m_IdleSightFOV,  XCOLOR_PURPLE   );
            draw_Arc( GetPosition() + vector3(0,10,0), m_LightSightRadius, ArcYaw, m_AlertSightFOV, XCOLOR_YELLOW   );
            draw_Arc( GetPosition() + vector3(0,20,0), m_DarkSightRadius,  ArcYaw, m_AlertSightFOV, XCOLOR_RED      );
            draw_Arc( GetPosition() + vector3(0,10,0), m_SoundRange,       ArcYaw, 2.0f*PI,         XCOLOR_WHITE    );

            // Show grenade path
            if (1)
            {
                f32     Gravity         = -1000.0f;
                f32     ForceOfThrow    = 1500.0f;
                // Get P0.
                vector3 Vel( 0.0f, 0.0f, ForceOfThrow );
                Vel.Rotate( m_GrenadeTrajectory );
                vector3 Delta = m_GrenadeDestination - m_GrenadeThrowStart;           
                Delta.GetY() = 0;
                f32 XZDist = Delta.Length();

                vector3 VelH( Vel.GetX(), 0, Vel.GetZ() );
                f32     SpeedH = VelH.Length();
                vector3 VelV( 0, Vel.GetY(), 0 );
                f32     SpeedV = Vel.GetY();

                f32 TotalT = XZDist / SpeedH;
                f32 ApexT = -SpeedV / Gravity;
                f32 T = ApexT;

                vector3 Pos( m_GrenadeThrowStart );
                Pos += vector3( VelH.GetX() * T,
                    SpeedV * T + 0.5f * Gravity * T*T,
                    VelH.GetZ() * T );
                draw_Sphere( Pos, 15.0f, XCOLOR_GREEN );

                draw_Sphere( m_GrenadeDestination, 15.0f, XCOLOR_GREEN );
                draw_Sphere( m_GrenadeThrowStart, 15.0f, XCOLOR_GREEN );

                s32 i;
                s32 nPoints = 20;
                vector3 LastPos = m_GrenadeThrowStart;

                for (i=0;i<=nPoints;i++)
                {
                    f32 T = (f32)i / (f32)nPoints * TotalT;

                    vector3 Pos( m_GrenadeThrowStart );
                    Pos += vector3( VelH.GetX() * T,
                        SpeedV * T + 0.5f * Gravity * T*T,
                        VelH.GetZ() * T );

                    draw_Sphere( Pos, 10.0f, XCOLOR_RED );  

                    g_CollisionMgr.LineOfSightSetup( GetGuid(), Pos, Pos+vector3(0,-10000,0) );
                    // Only need one collision to say that we can't throw there
                    g_CollisionMgr.SetMaxCollisions(1);

                    // Perform collision
                    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_LARGE_PROJECTILES, (object::object_attr)( object::ATTR_COLLISION_PERMEABLE ) );
                    if ( g_CollisionMgr.m_nCollisions != 0 )
                    {
                        draw_Line(Pos,g_CollisionMgr.m_Collisions[0].Point,XCOLOR_RED);
                        draw_Sphere( g_CollisionMgr.m_Collisions[0].Point, 10.0f, XCOLOR_PURPLE );
                    }

                    draw_Line( Pos, LastPos, XCOLOR_RED );
                    LastPos = Pos;
                }
            }

            /*            // Draw target related info
            draw_Label( GetLastSeenLocationOfTarget(), xcolor(255,100,100), "TargetSeenLast" );
            draw_Label( GetLastKnownLocationOfTarget(), xcolor(255,100,100), "TargetKnownLast" );
            draw_Label( GetLastLocationOfInterest(), xcolor(100,100,255), "Interest" );
            */

            // Render state
            if (m_pActiveState)
            {            
                m_pActiveState->OnDebugRender();
            }
            
            // Render path
            if (s_bDebugPath)
            {
                PrintToTextBlock( XCOLOR_AQUA, "CurrentConnection: %d",m_CurrentConnectionSlot );

                s32 i;
                vector3 MyPos = GetPosition();

                // Render spline
                if( m_CurrentSplinePoint >= 0 )
                {                
                    for(i=0;i<m_PointsPerSpline;i++)
                    {
                        draw_Sphere(m_SplinePointList[i],20.0f,xcolor( 255, (u8)(255*((f32)i)/((f32)m_PointsPerSpline)), 0 ));
                    }
                }
                draw_Sphere(m_NextPathPoint,20.0f,XCOLOR_WHITE);

                // Render path
                m_PathFindStruct.RenderPath( GetLocoPointer()->m_Physics.GetColRadius() );
                for (i=0;i<m_PathFindStruct.m_nSteps;i++)
                {
                    vector3 P1,P2;

                    if (m_PathFindStruct.m_StepData[i].m_NodeToPassThrough == NULL_NAV_SLOT)
                        break;

                    ng_node2&   Node = g_NavMap.GetNodeByID( m_PathFindStruct.m_StepData[i].m_NodeToPassThrough );
                    P2 = Node.GetPosition();

                    if (i==0)
                    {
                        P1 = GetPosition();                            
                    }
                    else
                    {
                        ng_node2&   Node = g_NavMap.GetNodeByID( m_PathFindStruct.m_StepData[i-1].m_NodeToPassThrough );
                        P1 = Node.GetPosition();
                    }

                    P1 += vector3(0,50,0);
                    P2 += vector3(0,50,0);

                    draw_Line(P1,P2,XCOLOR_RED);
                }

                for (i=0;i<m_PathFindStruct.m_nSteps;i++)
                {
                    vector3 vMoveTo;


                    {
                        vector3 Pos = GetPosition();
                        radian FacingYaw = GetLocoPointer()->m_Player.GetFacingYaw();
                        vector3 vFacingYaw(0,0,-1000);
                        vFacingYaw.RotateY( FacingYaw );
                        draw_Line( Pos, Pos+vFacingYaw, XCOLOR_YELLOW );
                    }

                    vector3 RemoteEnd;
                    if (i < (m_PathFindStruct.m_nSteps))
                    {
                        RemoteEnd = g_NavMap.GetBestGuessDestination( m_PathFindStruct.m_StepData[i].m_CurrentConnection,
                            m_PathFindStruct.m_StepData[i].m_DestConnection,
                            MyPos,
                            m_PathFindStruct.m_vEndPoint );
                    }
                    else
                        RemoteEnd = m_PathFindStruct.m_vEndPoint;

                    g_NavMap.GetClosestPointInOverlap( m_PathFindStruct.m_StepData[i].m_CurrentConnection, 
                        m_PathFindStruct.m_StepData[i].m_DestConnection, 
                        MyPos,
                        RemoteEnd,
                        GetLocoPointer()->m_Physics.GetColRadius(),
                        vMoveTo );

                    draw_Line(MyPos,vMoveTo,XCOLOR_GREEN);
                    MyPos = vMoveTo;
                }
                draw_Line(MyPos,m_PathFindStruct.m_vEndPoint,XCOLOR_GREEN);

                if (m_JumpTimeRemaining > 0)
                {
                    draw_Line( m_JumpStartPos, m_JumpApexPos, XCOLOR_YELLOW );
                    draw_Line( m_JumpApexPos, m_JumpLandPos, XCOLOR_BLUE );
                    draw_Label( m_JumpApexPos, XCOLOR_YELLOW, "APEX" );
                }
            }
        }
    }
    else
    {
        // do these when the game is not running
        // Show stats?
        if (s_bDebugStats)
        {
            // Count # of displayed verts and faces
            s32 nMeshes = 0 ;
            s32 nVerts  = 0 ;
            s32 nFaces  = 0 ;
            for (i = 0 ; i < pSkinGeom->m_nMeshes ; i++)
            {
                // Is this mesh on?
                if (LODMask & (1<<i))
                {
                    // Add to stats
                    nMeshes += 1 ;
                    nVerts  += pSkinGeom->m_pMesh[i].nVertices ;
                    nFaces  += pSkinGeom->m_pMesh[i].nFaces ;
                }
            }

            s32 nActiveBones = m_SkinInst.GetNActiveBones(LODMask) ;

            // Show info
#ifdef X_EDITOR
            // Find out many bones should be reander
            PrintToTextBlock( XCOLOR_WHITE, xfs("Meshes:%d\nVerts:%d\nFaces:%d\nBones:%d", nMeshes, nVerts, nFaces, nActiveBones) );
#endif // X_EDITOR

            // Show on PS2?
            if (s_bDebugInGame)
                draw_Label( GetPosition(), 
                XCOLOR_WHITE, 
                xfs("Meshes:%d\nVerts:%d\nFaces:%d\nBones:%d", nMeshes, nVerts, nFaces, nActiveBones) ) ;
        }
        // Reset transform
        draw_ClearL2W();
        if (s_bDebugAI)
        {
//            RenderHitLocations();
            radian ArcYaw = GetLocoPointer()->GetSightYaw();
            draw_Arc( GetPosition() + vector3(0,10,0), m_LightSightRadius, ArcYaw, m_IdleSightFOV,  XCOLOR_BLUE     );
            draw_Arc( GetPosition() + vector3(0,20,0), m_DarkSightRadius,  ArcYaw, m_IdleSightFOV,  XCOLOR_PURPLE   );
            draw_Arc( GetPosition() + vector3(0,10,0), m_LightSightRadius, ArcYaw, m_AlertSightFOV, XCOLOR_YELLOW   );
            draw_Arc( GetPosition() + vector3(0,20,0), m_DarkSightRadius,  ArcYaw, m_AlertSightFOV, XCOLOR_RED      );
            draw_Arc( GetPosition() + vector3(0,10,0), m_SoundRange,       ArcYaw, 2.0f*PI,         XCOLOR_WHITE    );


            // Draw leash info
            if (m_LeashGuid != 0)
            {
                object* pObj = g_ObjMgr.GetObjectByGuid( m_LeashGuid );
                if (pObj)
                {
                    draw_Arc( pObj->GetPosition(), m_LeashDistance, 0, R_360, XCOLOR_AQUA );
                    draw_Label( pObj->GetPosition(), XCOLOR_AQUA, "LEASH" );
                }
            }

        }
    }


    // Draw selection
    if (GetAttrBits() & ATTR_EDITOR_SELECTED)
    {
        draw_BBox(GetBBox(), XCOLOR_RED );

        if (GetLocoPointer())
        {
            bbox BBox = GetLocoPointer()->m_Physics.GetBBox();

            BBox.Transform( GetL2W() );

            draw_BBox(BBox, XCOLOR_GREEN );
        }
    }

    // Debug path?
#endif // X_EDITOR

}
#endif // X_RETAIL || X_QA

//=============================================================================

xbool g_DoCharacterRender = TRUE;

void character::OnRender( void )
{
    if( !g_DoCharacterRender || !m_bDoRender )
        return;

#if defined( X_EDITOR ) && defined( sbroumley )

    // Always render mutant tank attack state info...
    if( GetType() == object::TYPE_MUTANT_TANK )
    {
        if( m_pActiveState )
            m_pActiveState->OnDebugRender();
    }

    // Show loco info
    //{
        //GetLocoPointer()->RenderInfo(TRUE,               // bRenderLoco
            //g_GameLogicDebug,   // bLabelLoco (only draw in the editor)
            //FALSE,              // bRenderSkeleton
            //FALSE) ;            // bLabelSkeleton
    //}
#endif

#ifdef jfranklin
    new_weapon *pWeapon = GetCurrentWeaponPtr();

    // draw firing path
    if( pWeapon )
    {
        xbool isHit = TRUE;
        vector3 shotDest = CalculateShotDestination(isHit);

        vector3 firePosition; 
        pWeapon->GetFiringBonePosition(firePosition);
        vector3 aimPosition; 
        pWeapon->GetAimBonePosition(aimPosition);
        vector3 toDest = shotDest - firePosition;
        vector3 firePath = firePosition - aimPosition;       

        // if our shot dest bends the bullet too much, just fire forward instead.
        if( x_abs(v3_AngleBetween(toDest,firePath)) >= k_MinShotBendAngle )
        {
            firePath.NormalizeAndScale(100.0);
            shotDest = firePosition + firePath;
            isHit = FALSE;
        }

        draw_Line(firePosition, shotDest, XCOLOR_RED);
    }
#endif

#ifdef nmreed // draw collision volume
    {
        s32 nSlices = 20;
        s32 nPoints = 16;
        vector3 Position = GetPosition();
        f32 Height = GetCollisionHeight();

        draw_Begin( DRAW_LINES );
        draw_ClearL2W();
        draw_Color( XCOLOR_GREEN );
        for( s32 S=0; S<nSlices; S++ )
        {

            f32 ST = (f32)S / (f32)(nSlices-1);
            f32 Radius = x_sin(R_180*ST)*GetCollisionRadius();
            f32 Y      = ST * Height;

            vector3 PrevPos;
            for( s32 P=0; P<nPoints; P++ )
            {
                f32 PT = (f32)P/(f32)(nPoints-1);
                vector3 Pos(0,Y,Radius);
                Pos.RotateY( PT*R_360 );
                Pos += Position;

                if( P>0 )
                {
                    draw_Vertex(PrevPos);
                    draw_Vertex(Pos);
                }
                PrevPos = Pos;
            }

        }

        draw_Vertex( Position );
        draw_Vertex( Position + vector3( 0.0f, Height, 0.0f ) );
        draw_End();

    }
#endif

    // Handle occlusion of characters
    {
        xbool bOccluded = g_OccluderMgr.IsBBoxOccluded( GetBBox() );
        #if defined(athyssen) && defined(X_EDITOR)
            draw_BBox( GetBBox(), XCOLOR_RED );
            if( bOccluded )
            {
                draw_Line( GetPosition() - vector3(0,2000,0), GetPosition() + vector3(0,2000,0), XCOLOR_YELLOW );
            }
            else
            {
                draw_Line( GetPosition() - vector3(0,2000,0), GetPosition() + vector3(0,2000,0), XCOLOR_AQUA );
            }
        #endif
        if( bOccluded )
            return;
    }

    // Skin if dead body is present (can happen in network game)
    if( m_CorpseGuid )
        return;

    // Call base class render
    actor::OnRender();

    // Reset timer
    m_TimeSinceLastRender = 0.0f;    

    // Debug in game too?
#if !defined(X_RETAIL) || defined(X_QA)
    if (s_bDebugInGame)
        OnDebugRender() ;
#endif // !X_RETAIL || X_QA
}

//=============================================================================

character_state::states character::GetActiveState( void )
{
    if (m_pActiveState)
        return m_pActiveState->m_State;
    else
        return character_state::STATE_NULL;
}

//=============================================================================

void character::OnRenderTransparent( void )
{
    actor::OnRenderTransparent();

    // render weapon
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon )
    {
        pWeapon->SetRenderState( new_weapon::RENDER_STATE_NPC );
        pWeapon->OnRenderTransparent();
    }
}

//=============================================================================

s32 character::GetMaterial( void ) const
{
    return MAT_TYPE_FLESH;
}

//=========================================================================

bbox character::GetLocalBBox( void ) const
{ 
#if defined( X_EDITOR )

    // Use exapanded box that includes move at and look at?
    if (s_bDebugInGame)
    {
        // Start with regular bbox
        bbox BBox = m_BBox ;

        // Add in loco info
        if( m_pLoco )
        {
            // Lookup loco info
            const vector3  Pos    = m_pLoco->GetPosition() ;
            const vector3& LookAt = m_pLoco->GetHeadLookAt() ;
            const vector3& MoveAt = m_pLoco->GetMoveAt() ;

            // Add "look at" in local space if it's been initialized
            if (LookAt != vector3(0,0,0))
                BBox += LookAt - Pos ;

            // Add "move at" in local space if it's been initialized
            if (MoveAt != vector3(0,0,0))
                BBox += MoveAt - Pos ;
        }

        // Inflate for extra debug info such as text etc
        BBox.Inflate(100.0f, 100.0f, 100.0f) ;
        return BBox ;
    }

#endif

    // Default
    return m_BBox;
}

//=============================================================================

vector3 character::GetHitDestination()
{
    if( (CanSeeTarget() &&
        GetTargetGuid()) )
    {    
        if( x_irand(0,1) )
        {
            return GetTargetPosWithOffset(GetTargetGuid(),OFFSET_AIM_AT);
        }
        else
        {   
            return GetTargetPosWithOffset(GetTargetGuid(),OFFSET_EYES);
        }
    }
    else if ( m_AimAtGuid )
    {
        return GetTargetPosWithOffset(m_AimAtGuid,OFFSET_CENTER);
    }
    else 
    {
        return GetLastSeenLocationOfTarget();
    }
}

//=============================================================================

vector3 character::GetMissDestination()
{
    if( CanSeeTarget() &&
        GetTargetGuid() )
    {    
        vector3 position = GetTargetPosWithOffset(GetTargetGuid(),OFFSET_EYES);
        f32 distToTarget = GetToTarget().Length();
        f32 x = x_frand(-distToTarget/20.0f,distToTarget/20.0f);
        f32 y = x_frand(-distToTarget/20.0f,distToTarget/20.0f);
        f32 z = x_frand(-distToTarget/20.0f,distToTarget/20.0f);
        position += vector3( x, y, z );
        return position;
    }
    else if ( m_AimAtGuid )
    {
        return GetTargetPosWithOffset( m_AimAtGuid, OFFSET_EYES );
    }
    else 
    {
        return GetLastSeenLocationOfTarget();
    }
}


//=============================================================================

s32 character::GetHitChance()
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( !pWeapon )
    {
        return 0;
    }

    object *currentTarget = g_ObjMgr.GetObjectByGuid(GetTargetGuid());

    s32 hitChance = m_Accuracy;    
    if( currentTarget && currentTarget->IsKindOf(actor::GetRTTI()) )
    {
        actor &actorSource = actor::GetSafeType( *currentTarget );
        if( actorSource.IsRunning() )
        {
            // base accuracy vs. a running target
            hitChance = m_MovingTargetAccuracy;
        }
    }
    
    // mod by difficulty scalar.
    hitChance = hitChance + (s32)( ((f32)hitChance) * (m_AccuracyDifficultyScaler/100.0f) );  

    f32 weaponAccuracy = pWeapon->GetAccuracyPercent( GetToTarget().Length() );
    hitChance = (s32)(((f32)hitChance) * weaponAccuracy);
    return hitChance;
}

//=============================================================================

vector3 character::CalculateShotDestination( xbool& isHit, s32 accuracyModifier )
{    
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( !pWeapon )
    {
        return GetPosition();
    }

    s32 hitChance = GetHitChance();
    hitChance *= accuracyModifier;
    hitChance /= 100;

    if( x_irand(1,100) <= hitChance || m_NextShotMustHit )
    {
        isHit = TRUE;
        m_NextShotMustHit = FALSE;
        return GetHitDestination();
    }
    else
    {
        isHit = FALSE;
        return GetMissDestination();
    }
}

//=============================================================================

void character::CinemaFireWeapon( )
{
    new_weapon *pWeapon = GetCurrentWeaponPtr();
    if( !pWeapon )
    {
        return;
    }

#ifndef X_EDITOR
    m_Net.FireMode = 0;
    m_NetDirtyBits |= FIRE_BIT;
#endif

    vector3 firePosition; 
    pWeapon->GetFiringBonePosition(firePosition);
    vector3 aimPosition; 
    pWeapon->GetAimBonePosition(aimPosition);
    vector3 firePath = firePosition - aimPosition;       
    firePath.NormalizeAndScale(100.0f);

    pWeapon->NPCFireWeapon( vector3( 0.0f, 0.0f, 0.0f ) ,   // Base velocity
        firePosition + firePath,                      // Target's position
        GetGuid(),
        1.0f,
        TRUE );

#ifndef X_EDITOR
        m_Net.FireMode = 0;
#endif
}

//=============================================================================

void character::FireWeapon( xbool firingPrimary )
{    
    new_weapon *pWeapon = GetCurrentWeaponPtr();
    if( !pWeapon )
    {
        return;
    }

    if( (GetTargetGuid() ||
         m_bCrazyFire || 
         m_AimAtGuid) &&
        !m_FriendlyBlocksTarget )
    {    
        xbool isHit = TRUE;
        vector3 shotDest = CalculateShotDestination(isHit);

        vector3 firePosition; 
        pWeapon->GetFiringBonePosition(firePosition);
        vector3 aimPosition; 
        pWeapon->GetAimBonePosition(aimPosition);
        vector3 toDest = shotDest - firePosition;
        vector3 firePath = firePosition - aimPosition;       
        radian angleBetween = x_abs(v3_AngleBetween(toDest,firePath));

        // if our shot dest bends the bullet too much, just fire forward instead.
        if(  angleBetween >= k_MinShotBendAngle )
        {
            firePath.NormalizeAndScale(100.0);
            shotDest = firePosition + firePath;
            isHit = FALSE;
        }

        // added don't fire if it's too dang far from the shooting.
        if( angleBetween <= k_MinShotFireAngle ||
            m_bForceWeaponFire ||
            toDest.LengthSquared() <= k_ShootIfThisCloseSqr )
        {        
            #ifndef X_EDITOR
            m_Net.FireMode = 0;
            m_NetDirtyBits |= FIRE_BIT;
            #endif

            if( firingPrimary )
            {
                pWeapon->NPCFireWeapon( vector3( 0.0f, 0.0f, 0.0f ) ,   // Base velocity
                    shotDest ,                      // Target's position
                    GetGuid(),
                    1.0f,
                    isHit );

                #ifndef X_EDITOR
                m_Net.FireMode = 0;
                #endif
            }
            else
            {        
                pWeapon->NPCFireSecondary( vector3( 0.0f, 0.0f, 0.0f ) ,   // Base velocity
                    shotDest ,                      // Target's position
                    GetGuid(),
                    1.0f,
                    isHit );

                #ifndef X_EDITOR
                m_Net.FireMode = 1;
                #endif
            }
        }

        if( !CanShootAtTarget() )
        {
            // shut off the shoot track
            if( GetLocoPointer() ) 
            {
                GetLocoPointer()->GetAdditiveController(ANIM_FLAG_SHOOT_CONTROLLER).Clear();
                CancelAutofire();
            }        
        }

    }
    else
    {
        // shut off the shoot track
        if( GetLocoPointer() ) 
        {
            GetLocoPointer()->GetAdditiveController(ANIM_FLAG_SHOOT_CONTROLLER).Clear();
            CancelAutofire();
        }        
    }
}

//=============================================================================

void character::OnEvent( const event& Event )
{
    // Voice event?
    if( Event.Type == event::EVENT_VOICE )
    {
        if( g_ConverseMgr.IsActive( m_VoiceID ) )
            g_ConverseMgr.Stop( m_VoiceID );

        const voice_event& VoiceEvent = voice_event::GetSafeType( Event );

        m_VoiceID = VoiceEvent.VoiceID;
    }
    else if( Event.Type == event::EVENT_DIALOG )
    {
        const dialog_event& DialogEvent = dialog_event::GetSafeType( Event );
        PlayDialog( DIALOG_ANIM_EVENT, DialogEvent.DialogName, DialogEvent.HotVoice );
    }
    else if( Event.Type == event::EVENT_GENERIC )
    {
        const generic_event& GenericEvent = generic_event::GetSafeType( Event );

        if( !x_strcmp( GenericEvent.GenericType, "Face Idles Off" ) )
        {
            GetLocoPointer()->SetFaceIdleEnabled( FALSE );
        }
        else if( !x_strcmp( GenericEvent.GenericType, "Face Idles On" ) )
        {
            GetLocoPointer()->SetFaceIdleEnabled( TRUE );
        }
        else if( !x_strcmp( GenericEvent.GenericType, "Aimer Off" ) )
        {
            GetLocoPointer()->SetAimerWeight( 0.0f, 0.3f );    
        }
        else if( !x_strcmp( GenericEvent.GenericType, "Aimer On" ) )
        {
            GetLocoPointer()->SetAimerWeight( 1.0f, 0.3f );
        }
        else if( !x_strcmp( GenericEvent.GenericType, "Weapon Toss" ) )
        {
            DropWeapon(); 
            SetAnimPackage( m_WeaponlessAnimPackage );
        }
    }
    else if( Event.Type == event::EVENT_PAIN )
    {
        const pain_event& PainEvent = pain_event::GetSafeType( Event );  
#if !defined(X_RETAIL) || defined(X_QA)
        m_CurrentPainCenter = PainEvent.Position;
        m_CurrentPainRadius = PainEvent.PainRadius;
#endif
        switch( PainEvent.PainType ) 
        {
        case pain_event::EVENT_PAIN_MELEE :
            {
                EmitMeleePain( PainEvent );
                break;
            }
        case pain_event::EVENT_PAIN_LEAP_CHARGE :
            {
                pain Pain;
                Pain.Setup(xfs("%s_LEAP",GetLogicalName()), GetGuid(), PainEvent.Position );
                Pain.ApplyToWorld(TRUE);
                break;
            }
        case pain_event::EVENT_PAIN_SPECIAL :
            {
                pain Pain;
                Pain.Setup(xfs("%s_SPECIAL",GetLogicalName()), GetGuid(), PainEvent.Position );
                Pain.ApplyToWorld(TRUE);
                break;
            }
        }
    }

    else if( Event.Type == event::EVENT_WEAPON )
    {
        const weapon_event& WeaponEvent = weapon_event::GetSafeType( Event );
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if( pWeapon )
        {             
            switch( WeaponEvent.WeaponState )
            {
            case new_weapon::EVENT_FIRE:
                FireWeapon();
                break;
            case new_weapon::EVENT_ALT_FIRE:
                FireWeapon(FALSE);
                break;
            case new_weapon::EVENT_GRENADE:
            case new_weapon::EVENT_ALT_GRENADE:
                if( m_GrenadeItem == INVEN_GRENADE_FRAG )
                {                    
                    ThrowGrenade(WeaponEvent.Pos);
                }
                else if ( m_GrenadeItem == INVEN_GRENADE_GRAV )
                {
                    ThrowSecondaryGrenade(WeaponEvent.Pos);
                }
                break;
            case new_weapon::EVENT_CINEMA_FIRE:
                CinemaFireWeapon();
                break;
            default:
                break;

            }
        }
    }


    // Check for loco specific events
    if (GetLocoPointer())
    {
        // WorldCollision event?
        if (Event.Type == event::EVENT_WORLD_COLLISION)
        {
            const world_collision_event& WorldCollEvent = world_collision_event::GetSafeType(Event);
            GetLocoPointer()->m_Physics.SetLocoCollisionOn(WorldCollEvent.bWorldCollisionOn);
        }
        else
            // Gravity event?
            if (Event.Type == event::EVENT_GRAVITY)
            {
                const gravity_event& GravityEvent = gravity_event::GetSafeType(Event);
                GetLocoPointer()->m_Physics.SetLocoGravityOn(GravityEvent.bGravityOn);
            }
    }
}

//=============================================================================

void character::GetMeleeInfo( vector3& StartPos, vector3& EndPos, f32& MeleeSphereRadius )
{
    f32 MeleeReachDistance = 0.0f;

    // load melee reach distance tweak
    tweak_handle ReachDistanceTweak( xfs( "%s_MeleeReachDistance", GetLogicalName() ) );
    if( ReachDistanceTweak.Exists() )    
    {
        MeleeReachDistance = ReachDistanceTweak.GetF32();
    }        
    else        
    {
        tweak_handle DefaultReachDistanceTweak( "CHARACTER_MeleeReachDistance" );
        MeleeReachDistance = DefaultReachDistanceTweak.GetF32();
    }

    // load melee sphere radius tweak
    tweak_handle SphereRadiusTweak ( xfs( "%s_MeleeSphereRadius", GetLogicalName() ) );
    if( SphereRadiusTweak.Exists() )    
    {
        MeleeSphereRadius  = SphereRadiusTweak.GetF32();
    }        
    else        
    {    
        tweak_handle DefaultSphereRadiusTweak ( "CHARACTER_MeleeSphereRadius" );
        MeleeSphereRadius  = DefaultSphereRadiusTweak.GetF32();
    }

    // pick a distance
    vector3 RotateOffset = vector3(0.0f, 0.0f, MeleeReachDistance);

    // get where we are looking and rotate distance/offset vector to that yaw
    radian LookatYaw(GetLocoPointer()->GetAimerYaw());
    RotateOffset.RotateY( LookatYaw );

    // load start and end positions
    StartPos = GetPosition();
    EndPos   = StartPos + RotateOffset;        

    // pick this vector up off the ground a bit
    f32 Offset = GetCollisionHeight() * 0.5f;
    StartPos.GetY() += Offset;
    EndPos.GetY() += Offset;
}

//=============================================================================

void character::EmitMeleePain( const pain_event& PainEvent )
{
    // Has this event already hit something?
    if( m_LastMeleeEventID == m_CurrentPainEventID )
        return;

    ////////////////////////
    // setup pain sphere

    //
    // Fire a sphere out from the eye the correct distance w/offset and 
    // determine if we hit anything.
    //
    guid DirectHitGuid=0;
    vector3 HitPosition;
    {
        // Compute melee info from tweaks
        vector3 StartPos;
        vector3 EndPos;
        f32     MeleeSphereRadius;
        GetMeleeInfo( StartPos, EndPos, MeleeSphereRadius );

        g_CollisionMgr.SphereSetup( GetGuid(), StartPos, EndPos, MeleeSphereRadius );
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_DAMAGEABLE );
        if( g_CollisionMgr.m_nCollisions )
        {
            DirectHitGuid = g_CollisionMgr.m_Collisions[0].ObjectHitGuid;
            HitPosition = g_CollisionMgr.m_Collisions[0].Point;
        }
    }

    // If there was no direct hit then there's no pain to issue
    if( DirectHitGuid != 0 )
    {
        // Build pain
        pain Pain;

        Pain.Setup(xfs("%s_MELEE",GetLogicalName()),GetGuid(),HitPosition);
        Pain.SetDirectHitGuid( DirectHitGuid );
        Pain.SetCollisionInfo( g_CollisionMgr.m_Collisions[0] );

        // Setup direction
        object* pHitObject = g_ObjMgr.GetObjectByGuid( DirectHitGuid );
        ASSERT( pHitObject );

        vector3 HitSpot = pHitObject->GetPosition();
        HitSpot.GetY() = pHitObject->GetColBBox().Max.GetY();

        vector3 FromSpot = PainEvent.Position;
        FromSpot.GetY() = GetPosition().GetY();

        vector3 DeltaPos = GetLocoPointer()->GetDeltaPos();
        DeltaPos.GetY() = 0.0f;

        vector3 Dir = HitSpot - FromSpot;
        Dir += DeltaPos;
        Dir.Normalize();
        Pain.SetDirection( Dir );        

        // Finally, apply to object
        Pain.ApplyToObject( DirectHitGuid );

        // Turn off this event now        
        m_LastMeleeEventID = m_CurrentPainEventID;

        // Create an event sound emitter
        guid                 Guid         = g_ObjMgr.CreateObject( event_sound_emitter::GetObjectType() );
        object*              pSndEventObj = g_ObjMgr.GetObjectByGuid( Guid );
        event_sound_emitter& EventEmitter = event_sound_emitter::GetSafeType( *pSndEventObj );
        vector3              EventPos     = PainEvent.Position;
        char                 DescName[64];

        // Create sound type depending upon weapon type
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if( pWeapon )
        {
            // NPC has a weapon
            x_sprintf( DescName, "Melee_%s", EventEmitter.GetMaterialName(g_CollisionMgr.m_Collisions[0].Flags) );
            EventEmitter.PlayEmitter( DescName, 
                EventPos,
                GetZone1(), 
                event_sound_emitter::SINGLE_SHOT, 
                GetGuid() );

        }
        else
        {
            // NPC does not have a weapon
            x_sprintf( DescName, "MeleeSwipe_%s", EventEmitter.GetMaterialName(g_CollisionMgr.m_Collisions[0].Flags) );
            EventEmitter.PlayEmitter( DescName, 
                EventPos,
                GetZone1(), 
                event_sound_emitter::SINGLE_SHOT, 
                GetGuid() );
        }
    }
}

//=============================================================================
// VIRTUAL FUNCTIONS
//=============================================================================

//=============================================================================
// Collision functions
//=============================================================================

// SB - This function is not yet being used, but it shows how to use the bone
//      bboxes for collision.
void character::ApplyBoneBBoxCollision( void )
{
    ASSERT(FALSE);
    // Indices used to convert min + max of bbox into 8 corners
    static s32 CornerIndices[8*3]   = { 0,1,2, 
        4,1,2,
        0,5,2,
        4,5,2,
        0,1,6, 
        4,1,6,
        0,5,6,
        4,5,6 };

    // Indices used to convert 8 corners into a 4 sided NGon
    static s32 SideIndices[6*4]     = { 0,2,3,1,
        1,3,7,5,
        5,7,6,4,
        4,6,2,0,
        2,6,7,3,
        4,0,1,5 };

    // Locals
    s32         i,j;
    bbox        WorldBBox;
    vector3     Local;
    vector3     Corners[8];
    const s32*  pIndices;
    const f32*  pBBoxF;

    // Lookup skin geom
    geom* pGeom = m_SkinInst.GetGeom();
    if (!pGeom)
        return;

    // Lookup animation matrices
    const matrix4* pMatrices = NULL;
    if ( (GetLocoPointer()) && (GetLocoPointer()->IsAnimLoaded()) )
    {
        // Animation bones MUST match geometry bones
        ASSERT(GetLocoPointer()->GetNBones() == pGeom->m_nBones);
        pMatrices = GetLocoPointer()->ComputeL2W();
    }
    if (!pMatrices)
        return;

    // Loop through all bones in geom
    g_CollisionMgr.StartApply( GetGuid() );
    for (i = 0; i < pGeom->m_nBones; i++)
    {
        // Lookup bone info
        const bbox&    LocalBBox = pGeom->m_pBone[i].BBox;
        const matrix4& L2W       = pMatrices[i];

        // Calculate AA world bbox of bone
        WorldBBox = LocalBBox;
        WorldBBox.Transform(L2W);

        // If collision movement and AA world bbox DO NOT overlap, skip bone
        if (!g_CollisionMgr.GetDynamicBBox().Intersect(WorldBBox))
            continue;

        // In theory you could go to the polygon level here (yuk), but OO bboxes
        // should work fine for sniping etc (worked great in Turok, TribesPS2)

        // Transform all corners of the local AA bbox into world space
        pIndices = CornerIndices;
        pBBoxF   = (f32*)&LocalBBox.Min;
        for (j = 0; j < 8; j++ )
        {
            // Setup corner in local space
            Local.Set( pBBoxF[pIndices[0]],
                pBBoxF[pIndices[1]],
                pBBoxF[pIndices[2]] );

            // Transform into world space
            Corners[j] = L2W * Local;

            // Next vert
            pIndices += 3;
        }

        // Apply 6 sides of bbox - primitive key is set to bone index so specific bone test logic
        // can be checked for (eg. was head bone hit?)
        pIndices = SideIndices;
        for (j = 0; j < 6; j++)
        {
            // Indices of side plane are pIndices[0], pIndices[1], pIndices[2], pIndices[3]

            // Apply tri0
            g_CollisionMgr.ApplyTriangle(Corners[pIndices[0]],      // P0
                Corners[pIndices[1]],      // P1
                Corners[pIndices[2]],      // P2
                object::MAT_TYPE_FLESH,    // Flags
                i);                       // PrimitiveKey

            // Apply tri1
            g_CollisionMgr.ApplyTriangle(Corners[pIndices[2]],      // P0
                Corners[pIndices[3]],      // P1
                Corners[pIndices[0]],      // P2
                object::MAT_TYPE_FLESH,    // Flags
                i);                       // PrimitiveKey
            // Next side
            pIndices += 4;
        }
    }
    g_CollisionMgr.EndApply();
}


//=============================================================================
// State static functions
//=============================================================================

// Returns # of states
s32 character::GetStateCount( void )
{
    return character_state::STATE_TOTAL;
}

//=============================================================================

// Gets name, given index
const char* character::GetStateName( s32 Index )
{
    // Which state?
    switch(Index)
    {
    default:
        ASSERTS(0, "Add your new state to this list or properties will not work!");

    case character_state::STATE_NULL:                return "NULL";
    case character_state::STATE_HOLD:                return "HOLD";
    case character_state::STATE_IDLE:                return "IDLE";
    case character_state::STATE_ATTACK:              return "ATTACK";
    case character_state::STATE_FLEE:                return "FLEE";
    case character_state::STATE_DEATH:               return "DEATH";
    case character_state::STATE_ALERT:               return "ALERT";
    case character_state::STATE_SEARCH:              return "SEARCH";
    case character_state::STATE_COVER:               return "COVER";
    case character_state::STATE_SURPRISE   :         return "SURPRISE";
    case character_state::STATE_TRIGGER:             return "TRIGGER";
    case character_state::STATE_FOLLOW:              return "FOLLOW";
    case character_state::STATE_TURRET:              return "TURRET";
    case character_state::STATE_ALARM:               return "ALARM";
    }
}

//=============================================================================

// Returns states enum that can be used in property queries
const char* character::GetStatesEnum( void) 
{
    // Build enum list
    static char s_Enum[1024] = {0};

    // Already built?
    if (s_Enum[0])
        return s_Enum;

    // Add all states to enum
    char* pDest = s_Enum;
    for (s32 i = 0; i < GetStateCount(); i++)
    {
        // Lookup state name
        const char* pState = GetStateName(i);

        // Add to enum list
        x_strcpy(pDest, pState);

        // Next
        pDest += x_strlen(pState)+1;
    }

    // Make sure we didn't overrun the array!
    ASSERT(pDest <= &s_Enum[1024]);

    // Done
    return s_Enum;
}

//=============================================================================

//=============================================================================

// Returns STATE_NULL if not found
character_state::states character::GetStateByName( const char* pName )
{
    // Check all states
    for (s32 i = 0; i < GetStateCount(); i++)
    {
        // Found?
        if (x_stricmp(pName, GetStateName(i)) == 0)
            return (character_state::states)i;
    }

    // Not found
    return character_state::STATE_NULL;
}

//=============================================================================
// State virtual functions
//=============================================================================

void character::AdvanceState( f32 DeltaTime )
{
    // Advance current state if there is one
    if (m_pActiveState)
    {
        m_StateTime += DeltaTime;
        m_pActiveState->OnAdvance(DeltaTime);
    }
}

//=============================================================================

character_state *character::GetStateByType( character_state::states stateType )
{
    character_state* pState = m_pStateList;
    while(pState)
    {
        // Found?
        if (pState->m_State == stateType )
            return pState;

        // Check next state
        pState = pState->m_pNext;
    }
    return NULL;
}

//=============================================================================

character_state::states character::GetCurrentStateType( )
{
    if( m_pActiveState )
    {
        return m_pActiveState->GetStateType();
    }
    else
    {
        return character_state::STATE_NULL;
    }
}

//=============================================================================

xbool character::HasState( character_state::states stateType )
{
    character_state* pState = m_pStateList;
    while(pState)
    {
        // Found?
        if (pState->m_State == stateType )
            return TRUE;
        // Check next state
        pState = pState->m_pNext;
    }
    return FALSE;
}

//=============================================================================

character_task_set* character::GetPendingTaskList()
{
    character_task_set* pSet = NULL;
    object* pObject = g_ObjMgr.GetObjectByGuid(m_PendingTaskListGuid);
    if ( pObject && pObject->IsKindOf( character_task_set::GetRTTI() ) == TRUE )
    {
        pSet = (character_task_set*)pObject;
    }

    return pSet;
}

//=============================================================================

xbool character::SetupState( character_state::states State, void *pContext )
{
    character_state* pState = GetStateByType( State );

    // Nothing to do if state was not found
    if (!pState)
        return FALSE;

#ifndef X_EDITOR
    // Bot?
    if( g_NetworkMgr.IsOnline() == TRUE )
    {
        // Allow state
        m_DesiredState = State;
        m_StateContext = pContext;
        return TRUE;
    }
#endif

    // refuse to change if we are in death state...
    if( m_pActiveState && 
        m_pActiveState->GetStateType() == character_state::STATE_DEATH )
        return FALSE;

    // very special case... if we were about to go into trigger state ignore 
    // unless it's a call for trigger or death, then we need to release blocking
    if( m_DesiredState == character_state::STATE_TRIGGER )
    {
        // ignore unless important.
        if( State != character_state::STATE_DEATH &&
            State != character_state::STATE_TRIGGER )
        {
            return FALSE;
        }


        if( m_TriggerStateData.m_TriggerGuid && m_TriggerStateData.m_Blocking )
        {
            object* pObject = g_ObjMgr.GetObjectByGuid( m_TriggerStateData.m_TriggerGuid );
            if( pObject && pObject->IsKindOf(trigger_ex_object::GetRTTI()) )
            {
                trigger_ex_object &tempTrigger = trigger_ex_object::GetSafeType( *pObject );
                tempTrigger.ReleaseBlocking();
            }
        }
        else if( m_TriggerStateData.m_TaskListGuid )
        {
            object* pObject = g_ObjMgr.GetObjectByGuid( m_TriggerStateData.m_TaskListGuid );
            if( pObject && pObject->IsKindOf(character_task_set::GetRTTI()) )
            {
                character_task_set &taskSet = character_task_set::GetSafeType( *pObject );
                taskSet.OnTaskInterrupt();
            }
        }
    }

    if( m_pActiveState && 
        m_pActiveState->GetStateType() == character_state::STATE_TRIGGER &&
        !((character_trigger_state*)m_pActiveState)->GetTaskCompleted() )
    {        
        LOG_MESSAGE( "character::Trigger Break", "Trigger %08X:%08X Character %08X:%08X new State %s",
            ((character_trigger_state*)m_pActiveState)->GetTriggerGuid().GetHigh(),
            ((character_trigger_state*)m_pActiveState)->GetTriggerGuid().GetLow(),
            GetGuid().GetHigh(),
            GetGuid().GetLow(),
            GetStateName(State) );
    }

    /* mreed: 2/24/05: this is useful for debugging pain asserts sbroumley style
    //TEST
    if( ( m_pActiveState ) && ( m_pActiveState->m_State != character_state::STATE_DEATH )
        && ( State == character_state::STATE_DEATH ) )
    {
        guid GUID = m_PainThatKilledUs.GetOriginGuid();
        vector3 v3( m_PainThatKilledUs.GetForceDirection() );
        s32 a = 100;
        ++a;
    }
    */

    m_DesiredState = State;
    m_StateContext = pContext;
    return TRUE;
}

//=============================================================================

xbool character::ChangeState()
{
    if( m_MinStateTime > 0.0f )
        return FALSE;
    // Search for new state (it may not exist!)
    character_state* pState = GetStateByType( m_DesiredState );
    m_DesiredState = character_state::STATE_NULL;
    m_StateContext = NULL;

    // Nothing to do if state was not found
    if (!pState)
        return FALSE;

    // Nothing to do if we are already in this state, unless trigger state
    if (pState == m_pActiveState && pState->m_State != character_state::STATE_TRIGGER)
        return TRUE;

    // Exit out of current state
    if (m_pActiveState)
    {
        // Failed to exit out of state?
        if (!m_pActiveState->OnExit())
            return FALSE;

        // special case, when leaving hold state set the last listen time.
        if( m_pActiveState->GetStateType() == character_state::STATE_HOLD )
        {        
            m_LastListenTime = g_ObjMgr.GetGameTime();
            m_LastAlertTime = g_ObjMgr.GetGameTime();
        }

        m_LastState = m_pActiveState->m_State;
    }

    // Record and enter new state
    m_StateTime    = 0.0f;
    m_pActiveState = pState;

    // start with us happy at where we are.
    GetLocoPointer()->SetMoveAt( GetPosition() );

    pState->SetContext(m_StateContext);

    // Let the state set itself up
    pState->OnEnter();

    // Success
    return TRUE;
}

//=============================================================================

void character::ClearGoalData()
{
    m_GoalInfo.m_GoalType       = GOAL_NONE;
    m_GoalInfo.m_TargetGuid     = 0;
    m_GoalInfo.m_Offset.Zero() ;
    m_GoalInfo.m_Location.Zero() ;

    m_GoalInfo.m_RetreatGoalDestination.Zero() ;
    m_GoalInfo.m_DesiredDist    = 0.0f;
    m_GoalInfo.m_FOV            = 0.0f;
    m_GoalInfo.m_MoveStyle      = loco::MOVE_STYLE_NULL;
    m_GoalInfo.m_IsExact        = FALSE;

    m_GoalInfo.m_AnimType       = loco::ANIM_NULL;
    m_GoalInfo.m_AnimBlendTime  = DEFAULT_BLEND_TIME;
    m_GoalInfo.m_AnimName[0]    = 0;
    m_GoalInfo.m_AnimGroup[0]   = 0;
    m_GoalInfo.m_AnimFlags      = 0 ;
    m_GoalInfo.m_AnimPlayTime   = 1.0f;

    m_GoalInfo.m_UseCurrentTarget = FALSE;
    m_GoalInfo.m_DialogBlockOnDialog = TRUE;
    m_GoalInfo.m_DialogKillAnim = TRUE;
    m_GoalInfo.m_FaceTargetOnRetreat = TRUE;
    m_GoalInfo.m_DialogAnimBlendOutTime = 0.25f;

    m_GoalInfo.m_DialogName[0]  = 0;
    m_GoalInfo.m_DialogFlags    = 0;

    m_GoalCompleted = FALSE;
    m_GoalSucceeded = FALSE;
    m_NewGoal       = TRUE;

    // Reset move at to be current location to stop any stray movement before path is updated
    loco* pLoco = GetLocoPointer();
    ASSERT( pLoco );
    pLoco->SetMoveAt( pLoco->GetPosition() );
}

//=============================================================================

void character::SetIdleGoal( void )
{
    ClearGoalData();
    m_GoalInfo.m_GoalType       = GOAL_IDLE;
}

//=============================================================================

void character::SetTurnToTargetGoal( guid target, const vector3& offset, radian sightFOV, xbool turnExact )
{
    ClearGoalData();
    m_GoalInfo.m_GoalType       = GOAL_TURNTO_TARGET;
    m_GoalInfo.m_TargetGuid     = target;
    m_GoalInfo.m_Offset         = offset;
    m_GoalInfo.m_FOV            = sightFOV;
    m_GoalInfo.m_IsExact        = turnExact;
    m_GoalInfo.m_UseCurrentTarget = (target == GetTargetGuid());
}

//=============================================================================

void character::SetTurnToLocationGoal( const vector3& location, radian sightFOV, xbool turnExact )
{
    ClearGoalData();
    m_GoalInfo.m_GoalType       = GOAL_TURNTO_LOCATION;
    m_GoalInfo.m_Location       = location;
    m_GoalInfo.m_FOV            = sightFOV;
    m_GoalInfo.m_IsExact        = turnExact;
}

//=============================================================================

void character::SetLookAtTargetGoal( guid target, const vector3& offset, f32 distance , radian sightFOV, xbool lookatHead )
{
    ClearGoalData();
    m_GoalInfo.m_GoalType       = GOAL_LOOKAT_TARGET;
    m_GoalInfo.m_TargetGuid     = target;
    m_GoalInfo.m_Offset         = offset;
    m_GoalInfo.m_DesiredDist    = distance;
    m_GoalInfo.m_FOV            = sightFOV;
    m_GoalInfo.m_IsExact        = lookatHead;
    m_GoalInfo.m_UseCurrentTarget = ( target == GetTargetGuid() );
}

//=============================================================================

void character::SetLookAtLocationGoal( const vector3& location, f32 distance , radian sightFOV )
{
    ClearGoalData();
    m_GoalInfo.m_GoalType       = GOAL_LOOKAT_LOCATION;
    m_GoalInfo.m_Location       = location;
    m_GoalInfo.m_DesiredDist    = distance;
    m_GoalInfo.m_FOV            = sightFOV;
}

//=============================================================================

void character::SetGotoTargetGoal( guid target, const vector3& offset, loco::move_style moveStyle, f32 arriveDist, xbool gotoExact )
{
    ASSERT( (moveStyle>=loco::MOVE_STYLE_NULL) && (moveStyle<loco::MOVE_STYLE_COUNT) );

    ClearGoalData();
    m_GoalInfo.m_GoalType       = GOAL_GOTO_TARGET;
    m_GoalInfo.m_TargetGuid     = target;
    m_GoalInfo.m_Offset         = offset;
    m_GoalInfo.m_DesiredDist    = arriveDist; 
    m_GoalInfo.m_MoveStyle      = moveStyle;
    m_GoalInfo.m_IsExact        = gotoExact;
    m_GoalInfo.m_UseCurrentTarget = ( target == GetTargetGuid() );
}

//=============================================================================

void character::SetGotoLocationGoal( const vector3& location, loco::move_style moveStyle, f32 arriveDist, xbool gotoExact )
{
    ASSERT( (moveStyle>=loco::MOVE_STYLE_NULL) && (moveStyle<loco::MOVE_STYLE_COUNT) );

    ClearGoalData();
    m_GoalInfo.m_GoalType       = GOAL_GOTO_LOCATION;
    m_GoalInfo.m_Location       = location;
    m_GoalInfo.m_MoveStyle      = moveStyle;
    m_GoalInfo.m_DesiredDist    = arriveDist;
    m_GoalInfo.m_IsExact        = gotoExact;
}

//=============================================================================

void character::SetRetreatFromTargetGoal( guid target, const vector3& offset, loco::move_style moveStyle, f32 minDist, xbool faceTargetOnRetreat )
{
    ASSERT( (moveStyle>=loco::MOVE_STYLE_NULL) && (moveStyle<loco::MOVE_STYLE_COUNT) );

    ClearGoalData();
    m_GoalInfo.m_GoalType       = GOAL_RETREAT_FROM_TARGET;
    m_GoalInfo.m_TargetGuid     = target;
    m_GoalInfo.m_Offset         = offset;
    m_GoalInfo.m_MoveStyle      = moveStyle;
    m_GoalInfo.m_DesiredDist    = minDist; 
    m_GoalInfo.m_UseCurrentTarget = ( target == GetTargetGuid() );
    m_GoalInfo.m_FaceTargetOnRetreat = faceTargetOnRetreat;
}

//=============================================================================

void character::SetRetreatFromLocationGoal( const vector3& location, loco::move_style moveStyle, f32 minDist, xbool faceTargetOnRetreat )
{
    ASSERT( (moveStyle>=loco::MOVE_STYLE_NULL) && (moveStyle<loco::MOVE_STYLE_COUNT) );

    ClearGoalData();
    m_GoalInfo.m_GoalType       = GOAL_RETREAT_FROM_LOCATION;
    m_GoalInfo.m_Location       = location;
    m_GoalInfo.m_MoveStyle      = moveStyle;
    m_GoalInfo.m_DesiredDist    = minDist;
    m_GoalInfo.m_FaceTargetOnRetreat = faceTargetOnRetreat;
}

//=============================================================================

void character::SetPlayAnimationGoal( loco::anim_type AnimType, f32 BlendTime, u32 AnimFlags, f32 PlayTime )
{
    ClearGoalData();
    m_GoalInfo.m_GoalType       = GOAL_PLAY_ANIMATION;
    m_GoalInfo.m_AnimType       = AnimType;
    m_GoalInfo.m_AnimBlendTime  = BlendTime;
    m_GoalInfo.m_AnimFlags      = AnimFlags;
    m_GoalInfo.m_AnimPlayTime   = PlayTime;
}

//=============================================================================

void character::SetScaledPlayAnimationGoal( loco::anim_type newAnim, f32 BlendTime, u32 AnimFlags, const vector3& location )
{
    ClearGoalData();
    m_GoalInfo.m_GoalType       = GOAL_PLAY_ANIMATION_SCALED_TO_TARGET;
    m_GoalInfo.m_AnimType       = newAnim;
    m_GoalInfo.m_AnimBlendTime  = BlendTime;
    m_GoalInfo.m_AnimFlags      = AnimFlags;
    m_GoalInfo.m_Location       = location;
}

//=============================================================================

void character::SetPlayAnimationGoal( const char* animName, const char* animPkg, f32 BlendTime, u32 AnimFlags, f32 PlayTime )
{
    ClearGoalData();
    m_GoalInfo.m_GoalType       = GOAL_PLAY_ANIMATION;
    if( animName )
        x_strcpy( m_GoalInfo.m_AnimName,animName );
    if( animPkg )
        x_strcpy( m_GoalInfo.m_AnimGroup,animPkg );

    m_GoalInfo.m_AnimBlendTime  = BlendTime;
    m_GoalInfo.m_AnimFlags      = AnimFlags;
    m_GoalInfo.m_AnimPlayTime   = PlayTime ;

}

//=============================================================================

void character::SetScaledPlayAnimationGoal( const char* animName, const char* animPkg, f32 BlendTime, u32 AnimFlags, f32 PlayTime, const vector3& location )
{
    ClearGoalData();
    m_GoalInfo.m_GoalType       = GOAL_PLAY_ANIMATION_SCALED_TO_TARGET;
    if( animName )
        x_strcpy( m_GoalInfo.m_AnimName,animName );
    if( animPkg )
        x_strcpy( m_GoalInfo.m_AnimGroup,animPkg );

    m_GoalInfo.m_AnimPlayTime  = PlayTime;
    m_GoalInfo.m_AnimBlendTime = BlendTime;
    m_GoalInfo.m_AnimFlags     = AnimFlags;
    m_GoalInfo.m_Location      = location;
}

//=============================================================================

void character::SetSayDialogGoal( const char* dialogName, const char* animName, const char* animPkg, f32 BlendTime, u32 AnimFlags, u8 DialogFlags, xbool BlockOnDialog, xbool KillAnim, f32 BlendOutTime )
{
    ClearGoalData();
    m_GoalInfo.m_GoalType       = GOAL_SAY_DIALOG;
    if( dialogName )
        x_strcpy( m_GoalInfo.m_DialogName,dialogName );
    if( animName )
        x_strcpy( m_GoalInfo.m_AnimName,animName );
    if( animPkg )
        x_strcpy( m_GoalInfo.m_AnimGroup,animPkg );
    m_GoalInfo.m_AnimBlendTime          = BlendTime;
    m_GoalInfo.m_AnimFlags              = AnimFlags;
    m_GoalInfo.m_DialogFlags            = DialogFlags;
    m_GoalInfo.m_DialogBlockOnDialog    = BlockOnDialog;
    m_GoalInfo.m_DialogKillAnim         = KillAnim;
    m_GoalInfo.m_DialogAnimBlendOutTime = BlendOutTime;
}

//=============================================================================

void character::UpdateGoalLocation( const vector3& location )
{
    m_GoalInfo.m_Location = location;
    m_PathFindStruct.m_vEndPoint = GetGoalTrueLocation();
    m_PathReset = TRUE;
    m_CanReachGoalTarget = UpdateCanReachTarget( GetGoalTrueLocation(), 50.0f );
}

//=============================================================================

const char* character::GetGoalFailedReasonName() const
{
    switch( m_FailedReason )
    {
    case FAILED_GOAL_NONE:
        return "FAILED_GOAL_NONE";
    case FAILED_GOAL_STUCK:
        return "FAILED_GOAL_STUCK";
    case FAILED_GOAL_CANT_REACH:
        return "FAILED_GOAL_CANT_REACH";
    case FAILED_GOAL_BELOW_TARGET:
        return "FAILED_GOAL_BELOW_TARGET";
    case FAILED_GOAL_ABOVE_TARGET:
        return "FAILED_GOAL_ABOVE_TARGET";
    case FAILED_GOAL_TIMED_OUT:
        return "FAILED_GOAL_TIMED_OUT";
    case FAILED_GOAL_PLAY_ANIM_ERROR:
        return "FAILED_GOAL_PLAY_ANIM_ERROR";
    case FAILED_GOAL_PLAY_DIALOG_ERROR:
        return "FAILED_GOAL_PLAY_DIALOG_ERROR";
    case FAILED_GOAL_ACTOR_COLLISION:
        return "FAILED_GOAL_ACTOR_COLLISION";
    default:
        return "FAILED_UNKNOWN_ENUM_TYPE";
    }
}

//=============================================================================

/// this function sets up the next goal.
void character::ChangeGoal( void )
{
    // we have a new goal. Do whatever it is we are supposed to do for this goal.
    m_CurrentSplinePoint = -1;
    m_TimeInGoal = 0.0f;
    GetLocoPointer()->SetExactLook(FALSE);
    SetIsFollowingPath(FALSE);
    m_NeedsToRecalcPath = FALSE;  
    m_FailedReason = FAILED_GOAL_NONE;
    GetLocoPointer()->m_Physics.SetCollisionIgnoreGuid(m_CollisionIgnoreGuid);
    m_CollisionIgnoreGuid = 0;
    m_TimeObjectColliding = 0.0f;

    switch( m_GoalInfo.m_GoalType )
    {
    case character::GOAL_IDLE:
        if( GetLocoPointer()->GetState() == loco::STATE_PLAY_ANIM )
        {
            GetLocoPointer()->SetState( loco::STATE_IDLE );
        }
        break;
    case character::GOAL_TURNTO_TARGET:
    case character::GOAL_TURNTO_LOCATION:
        if( GetLocoPointer()->GetState() == loco::STATE_PLAY_ANIM )
        {
            GetLocoPointer()->SetState( loco::STATE_IDLE );
        }
        GetLocoPointer()->SetExactLook(TRUE);
        break;
    case character::GOAL_LOOKAT_TARGET:
    case character::GOAL_LOOKAT_LOCATION:
        break;
    case character::GOAL_RETREAT_FROM_TARGET:
    case character::GOAL_RETREAT_FROM_LOCATION:
        // this function call figures out the correct retreat location and stores it in m_GoalInfo.m_RetreatGoalDestination
        //CalculateValidRetreatDestination();
        if( m_GoalInfo.m_MoveStyle != loco::MOVE_STYLE_NULL )
            GetLocoPointer()->SetMoveStyle(m_GoalInfo.m_MoveStyle);

        m_GoalRetreatToConnectionSlot = GetNewRetreatPath( GetTargetPosWithOffset(m_GoalInfo.m_TargetGuid), m_GoalInfo.m_DesiredDist );

        if (!IsStaggering())  //TODO call new loco function here instead
        {
            GetLocoPointer()->SetState(loco::STATE_MOVE);   
        }
        m_GoalsConnectionSlot = m_GoalRetreatToConnectionSlot;
        m_CanReachGoalTarget = UpdateCanReachTarget( GetGoalTrueLocation(), 50.0f );
        break;        
    case character::GOAL_GOTO_TARGET:
    case character::GOAL_GOTO_LOCATION:
    {
        if( m_GoalInfo.m_MoveStyle != loco::MOVE_STYLE_NULL )
            GetLocoPointer()->SetMoveStyle(m_GoalInfo.m_MoveStyle);
        if (!IsStaggering())  //TODO call new loco function here instead
        {
            GetLocoPointer()->SetState(loco::STATE_MOVE);   
        }

        // dak - should we be pathfinding to a GoalTrueLocation that is NULL?  how can we skip this goal if it is invalid?
        vector3 GoalTrueLoc = GetGoalTrueLocation();
        if ( GoalTrueLoc.LengthSquared() < k_VerySmallNumber )
            m_CanReachGoalTarget = false;
        else
        {
            GetNewPath( GoalTrueLoc  );
            m_GoalsConnectionSlot = g_NavMap.GetNearestConnection( GoalTrueLoc  );
#ifdef AI_LOGGING
            LOG_MESSAGE( "character::ChangeGoal", "Target connection now [%d]", m_GoalsConnectionSlot );
#endif
            m_CanReachGoalTarget = UpdateCanReachTarget( GoalTrueLoc, 50.0f );            
        }
        break;
    }
    case character::GOAL_PLAY_ANIMATION_SCALED_TO_TARGET:
        {        
            // get the distance traveled by our animation and scale us to our target or to location if it was passed in
            s32 animIndex = -1;
            vector3 animMotion;
            f32 horizontalScale = 1.0f;
            f32 verticalScale = 1.0f;
            f32 horizAnimDist = 0.0f;
            f32 vertAnimDist  = 0.0f;

            anim_group::handle hAnimGroup; 
            if( m_GoalInfo.m_AnimType != loco::ANIM_NULL)
            {
                hAnimGroup = GetLocoPointer()->GetAnimGroupHandle();            
                animIndex = GetLocoPointer()->GetAnimIndex(m_GoalInfo.m_AnimType);                
            }
            else
            {
                if (x_strlen(m_GoalInfo.m_AnimGroup) > 0)
                {
                    hAnimGroup.SetName(m_GoalInfo.m_AnimGroup);
                    if (!hAnimGroup.GetPointer())
                    {
#ifdef X_EDITOR
                        x_try;
                        x_throw(xfs("ERROR: Attempted to play an animation (%s) for invalid anim group (%s)!\n",
                            m_GoalInfo.m_AnimName, m_GoalInfo.m_AnimGroup));
                        x_catch_display;
#endif // X_EDITOR
                        hAnimGroup = GetLocoPointer()->GetAnimGroupHandle();
                    }
                    animIndex = hAnimGroup.GetPointer()->GetAnimIndex(m_GoalInfo.m_AnimName);
                }
                else
                {
                    hAnimGroup = GetLocoPointer()->GetAnimGroupHandle();
                    animIndex = GetLocoPointer()->m_Player.GetAnimIndex(m_GoalInfo.m_AnimName);
                }    
            }

            if( animIndex >= 0 )
            {            
                animMotion = hAnimGroup.GetPointer()->GetAnimInfo(animIndex).GetTotalTranslation();

                vertAnimDist = animMotion.GetY();
                animMotion.GetY() = 0.0f;
                horizAnimDist = animMotion.Length();

                vector3 toTarget;
                if( m_GoalInfo.m_Location != vector3(0.0f,0.0f,0.0f) )
                {
                    toTarget = m_GoalInfo.m_Location - GetPosition();
                }
                else
                {
                    toTarget = GetToTarget();
                }
                // if there is vertical motion in the same direction.
                if( x_abs( vertAnimDist ) > 1.0f )
                {
                    verticalScale = toTarget.GetY()/vertAnimDist;
                }
                toTarget.GetY() = 0.0f;

                if( x_abs( horizAnimDist ) > 1.0f )
                {
                    horizontalScale = toTarget.Length()/horizAnimDist;
                }
                GetLocoPointer()->SetDeltaPosScale( vector3(horizontalScale,verticalScale,horizontalScale) );
            }
        }
        // no break here on purpose.        
    case character::GOAL_PLAY_ANIMATION:
        if( m_GoalInfo.m_AnimType != loco::ANIM_NULL)
        {        
            m_CurrentPainEventID = pain_event::CurrentEventID++;
            if( pain_event::CurrentEventID >= S32_MAX )
            {
                pain_event::CurrentEventID = 0;
            }
            if( !GetLocoPointer()->PlayAnim( m_GoalInfo.m_AnimType, m_GoalInfo.m_AnimBlendTime, m_GoalInfo.m_AnimFlags, m_GoalInfo.m_AnimPlayTime ) )
            {
                m_FailedReason = FAILED_GOAL_PLAY_ANIM_ERROR;
            }
            else
            {
#ifndef X_EDITOR
                net_PlayAnim( GetLocoPointer()->m_Player.GetAnimIndex(),
                    DEFAULT_BLEND_TIME, 
                    m_GoalInfo.m_AnimFlags, 
                    m_GoalInfo.m_AnimPlayTime );
#endif
                GetLocoPointer()->m_Player.SetYawDelta(m_AnimYawDelta);
                m_AnimYawDelta = 0.0f;
            }
        }
        else
        {
            if( !PlayAnimation( m_GoalInfo.m_AnimGroup, m_GoalInfo.m_AnimName, DEFAULT_BLEND_TIME, m_GoalInfo.m_AnimFlags, m_GoalInfo.m_AnimPlayTime ) )
            {
                m_FailedReason = FAILED_GOAL_PLAY_ANIM_ERROR;
            }
            else
            {
                GetLocoPointer()->m_Player.SetYawDelta(m_AnimYawDelta);
                m_AnimYawDelta = 0.0f;
            }
        }
        break;        
    case character::GOAL_SAY_DIALOG:  
        // special case, if the dialog has an associate animation with a begin dialog event
        // then we don't want to start the dialog yet... 
        if( !PlayDialog( DIALOG_GOAL_DIALOG, m_GoalInfo.m_DialogName, FALSE, m_GoalInfo.m_AnimName, m_GoalInfo.m_AnimGroup, 0, m_GoalInfo.m_DialogFlags, m_GoalInfo.m_DialogAnimBlendOutTime) )
        {
            m_FailedReason = FAILED_GOAL_PLAY_DIALOG_ERROR;
        }
        x_strcpy(m_CurrentDialogInfo.m_AnimName,m_GoalInfo.m_AnimName);
        x_strcpy(m_CurrentDialogInfo.m_AnimGroup,m_GoalInfo.m_AnimGroup);
        m_CurrentDialogInfo.m_AnimFlags = m_GoalInfo.m_AnimFlags;

        // if we are waiting for the anim event then we must start the anim up!
        if( m_CurrentDialogInfo.m_AnimName[0] != 0 && 
            m_LipSyncStartAnimFrame >= 0.0f )
        {                
            anim_group::handle hAnimGroup ;
            if (m_CurrentDialogInfo.m_AnimGroup[0] != 0)
                hAnimGroup.SetName(m_CurrentDialogInfo.m_AnimGroup);
            else
                hAnimGroup = GetLocoPointer()->GetAnimGroupHandle();

            // Start lip sync anim
            if( GetLocoPointer()->PlayLipSyncAnim( hAnimGroup, m_CurrentDialogInfo.m_AnimName, m_VoiceID, m_CurrentDialogInfo.m_AnimFlags ) )
            {
                GetLocoPointer()->GetLipSyncController().SetBlendOutTime( m_CurrentDialogInfo.m_BlendOutTime );
            }
            else
            {
                // if we failed to play the anim we can't wait on it for completion of the goal.
                m_GoalInfo.m_DialogBlockOnDialog = TRUE;
            }
        }
        else
        {
            m_LipSyncStartAnimFrame = -1.0f;
        }
        break;        
    }
#ifndef X_RETAIL
    AddToGoalChangeList( xfs("New Goal %s: Time %f",GetGoalTypeName(),x_GetTimeSec()) );
#endif
}

//=============================================================================

/// this function checks to see if our current goal 
/// has completed and updates the current goal
xbool character::UpdateGoal( f32 DeltaTime )
{
    CONTEXT( "character::UpdateGoal" );

    vector3 toLocation;
    f32 horizDistToLocation = 0.0f;
    xbool completed = FALSE;
    f32 goalDistance;
    radian goalFOV;
    f32 vertDistToLocation = 0.0f;

    m_TimeInGoal += DeltaTime;    
    m_TimeSinceLastRetreatJig += DeltaTime;  

    xbool targetChanged = FALSE;
    // see if our goal is to our current target, if so update our target.
    if( m_GoalInfo.m_UseCurrentTarget && GetTargetGuid() )
    {
        if( m_GoalInfo.m_TargetGuid != GetTargetGuid() )
        {        
            m_GoalInfo.m_TargetGuid = GetTargetGuid();
            targetChanged = TRUE;
        }
    }

    // step one if an error conditions we are done and have failed...
    if( m_FailedReason != FAILED_GOAL_NONE )
    {   
        m_GoalSucceeded = FALSE;
        completed = TRUE;
    }

    switch( m_GoalInfo.m_GoalType )
    {
    case character::GOAL_IDLE:
        // idling is always 'done'.
        m_GoalSucceeded  = TRUE;
        completed = TRUE;
        break;
    case character::GOAL_TURNTO_TARGET:
    case character::GOAL_TURNTO_LOCATION:        
        // target must be close.

        if( m_GoalInfo.m_IsExact )
        {
            goalFOV = 0.01f;
        }
        else if( m_GoalInfo.m_FOV <= 0.0f ) 
        {
            goalFOV = m_IdleSightFOV;
        }
        else
        {
            goalFOV = DEG_TO_RAD(m_GoalInfo.m_FOV);
        }

        if(         ( IsFacingTarget(GetGoalTrueLocation(), goalFOV) )
            ||  ( GetLocoPointer()->IsExactLookComplete() ) )
        {
            m_GoalSucceeded = TRUE;
            completed = TRUE;
        }
        else if ( m_TimeInGoal > k_MaxTimeTurning )
        {
            m_FailedReason = FAILED_GOAL_TIMED_OUT;
            m_GoalSucceeded = FALSE;
            completed = TRUE;
        }
        break;
    case character::GOAL_LOOKAT_TARGET:
    case character::GOAL_LOOKAT_LOCATION:
        if( m_GoalInfo.m_DesiredDist > 0.0f )
        {
            goalDistance = m_GoalInfo.m_DesiredDist;
        }
        else
        {
            goalDistance = m_LightSightRadius;
        }

        if( m_GoalInfo.m_FOV > 0.0f )
        {
            goalFOV = DEG_TO_RAD(m_GoalInfo.m_FOV);
        }
        else
        {
            goalFOV = m_IdleSightFOV;
        }

        // are we doing head only? If so just set it and say we are done.
        if( m_GoalInfo.m_IsExact )
        {
            SetHeadLookat( m_GoalInfo.m_TargetGuid, -1.0f, goalDistance, goalFOV );
            completed = TRUE;
            m_GoalSucceeded = TRUE;
        }
        else if( IsTargetInSightCone(GetGoalTrueLocation(), -1.0f, goalFOV) )
        {       
            m_GoalSucceeded = TRUE;
            completed = TRUE;
        }
        else if ( m_TimeInGoal > k_MaxTimeTurning )
        {
            m_FailedReason = FAILED_GOAL_TIMED_OUT;
            m_GoalSucceeded = FALSE;
            completed = TRUE;
        }
        break;
    case character::GOAL_GOTO_TARGET:
        // if we are on the last leg, call follow the path constantly to update the position
        m_PathFindStruct.m_vEndPoint = GetGoalTrueLocation();
        // no break here, continue on
    case character::GOAL_GOTO_LOCATION:
        {
            xbool pathable = TRUE;

            // Only need to do this if using the nav map
            if( m_PathingHints.bUseNavMap )
            {
                // If "straight path status" has changed, create a new path
                m_PathFindStruct.m_StartConnectionSlotID = m_CurrentConnectionSlot;
                m_PathFindStruct.m_vStartPoint           = GetPosition();            
                xbool bOldStraightPath = ( m_PathFindStruct.m_bStraightPath != 0 );
                xbool bNewStraightPath = g_NavMap.DoesStraightPathExist( m_PathFindStruct );
                if( bOldStraightPath != bNewStraightPath )
                    m_NeedsToRecalcPath = TRUE;
            }
                        
            // if we decided that we need a new path 
            if( m_NeedsToRecalcPath || targetChanged )
            {        
                pathable = GetNewPath( GetGoalTrueLocation() );
            }

            toLocation = GetPosition() - GetGoalTargetsLocation();
            vertDistToLocation = toLocation.GetY();
            toLocation.GetY() = 0.0f;
            horizDistToLocation = toLocation.Length();

            vector3 LeashPos(0,0,0);

            if (m_LeashDistance > 0.0f)
            {
                object* pLeashObj = NULL;
                pLeashObj = g_ObjMgr.GetObjectByGuid( m_LeashGuid );    
                if (pLeashObj)
                {
                    LeashPos = pLeashObj->GetPosition();
                }
            }

            if( (horizDistToLocation <= m_GoalInfo.m_DesiredDist && !m_GoalInfo.m_IsExact) )
            {
                if( vertDistToLocation < k_MaxDistBelow )
                {
                    m_FailedReason = FAILED_GOAL_ABOVE_TARGET;
                    m_GoalSucceeded = FALSE;
                    completed = TRUE;
                }
                else if( vertDistToLocation > k_MaxDistAbove )
                {
                    m_FailedReason = FAILED_GOAL_BELOW_TARGET;
                    m_GoalSucceeded = FALSE;
                    completed = TRUE;
                }
                else
                {
                    m_GoalSucceeded = TRUE;
                    completed = TRUE;
                }
            }
            else if( !pathable )
            {
                m_FailedReason = FAILED_GOAL_CANT_REACH;
                m_GoalSucceeded = FALSE;
                completed = FALSE;
            }
            else if( !GetIsFollowingPath() && GetLocoPointer()->IsAtDestination() && m_GoalInfo.m_IsExact )
            {
                // if I'm moving exact and I can reach my target but failed, get a new path.
                if( m_CanReachGoalTarget && horizDistToLocation > 25.0f )
                {
                    m_NeedsToRecalcPath = TRUE;
                }
                else if( vertDistToLocation < k_MaxDistBelow )
                {
                    m_FailedReason = FAILED_GOAL_ABOVE_TARGET;
                    m_GoalSucceeded = FALSE;
                    completed = TRUE;
                }
                else if( vertDistToLocation > k_MaxDistAbove )
                {
                    m_FailedReason = FAILED_GOAL_BELOW_TARGET;
                    m_GoalSucceeded = FALSE;
                    completed = TRUE;
                }
                else
                {
                    m_GoalSucceeded = TRUE;
                    completed = TRUE;
                }
            }
            else if( GetLocoPointer()->IsAtDestination() && !GetIsFollowingPath() )
            {
                m_NeedsToRecalcPath = TRUE;
            }
            else if( m_TimeActorColliding > k_MaxTimeActorColliding )
            {
                m_FailedReason = FAILED_GOAL_ACTOR_COLLISION;
                m_GoalSucceeded = FALSE;
                completed = TRUE;
            }
            else if( m_TimeObjectColliding > k_MaxTimeObjectColliding )
            {
                m_FailedReason = FAILED_GOAL_OBJECT_COLLISION;
                m_GoalSucceeded = FALSE;
                completed = TRUE;
            }
            else if( GetLocoPointer()->IsStuck() )
            {
                LOG_ERROR("GAMEPLAY","Character at %f, %f, %f is stuck",GetPosition().GetX(),GetPosition().GetY(),GetPosition().GetZ());
                m_FailedReason = FAILED_GOAL_STUCK;
                m_GoalSucceeded = FALSE;
                completed = TRUE;
            }
            else if( m_CurrentPathStructIndex >= m_PathFindStruct.m_nSteps && /*!m_LookaheadInGrid && */ !m_CanReachGoalTarget )
            {
                m_FailedReason = FAILED_GOAL_CANT_REACH;
                m_GoalSucceeded = FALSE;
                completed = TRUE;
            }
            else if ( (m_LeashDistance > 0.0f) && (m_LeashGuid != 0) && 
                (GetPosition() - LeashPos).Length() > m_LeashDistance )
            {
                m_FailedReason = FAILED_GOAL_OUTSIDE_LEASH;
                m_GoalSucceeded = FALSE;
                completed = TRUE;
            }
        }
        break;
    case character::GOAL_RETREAT_FROM_TARGET:

        // if we are on the last leg, call follow the path constantly to update the position
        //        m_PathFindStruct.m_vEndPoint = GetGoalTrueLocation();
        // no break here, continue on
    case character::GOAL_RETREAT_FROM_LOCATION:
        {
            // if we decided that we need a new path 
            if( m_NeedsToRecalcPath || targetChanged )
            {        
                m_GoalRetreatToConnectionSlot = GetNewRetreatPath( GetTargetPosWithOffset(m_GoalInfo.m_TargetGuid), m_GoalInfo.m_DesiredDist );
            }

            toLocation = GetPosition() - GetGoalTargetsLocation();
            toLocation.GetY() = 0;
            horizDistToLocation = toLocation.Length();

            vector3 LeashPos(0,0,0);

            if (m_LeashDistance > 0.0f)
            {
                object* pLeashObj = NULL;
                pLeashObj = g_ObjMgr.GetObjectByGuid( m_LeashGuid );    
                if (pLeashObj)
                {
                    LeashPos = pLeashObj->GetPosition();
                }
            }

            if( horizDistToLocation >= m_GoalInfo.m_DesiredDist && !m_GoalInfo.m_IsExact ) 
            {
                if( vertDistToLocation < k_MaxDistBelow )
                {
                    m_FailedReason = FAILED_GOAL_ABOVE_TARGET;
                    m_GoalSucceeded = FALSE;
                }
                else if( vertDistToLocation > k_MaxDistAbove )
                {
                    m_FailedReason = FAILED_GOAL_BELOW_TARGET;
                    m_GoalSucceeded = FALSE;
                }
                else
                {
                    m_GoalSucceeded = TRUE;
                }
                completed = TRUE;
            }
            else if( GetLocoPointer()->IsStuck() )
            {
                m_FailedReason = FAILED_GOAL_STUCK;
                m_GoalSucceeded = FALSE;
                completed = TRUE;
            }
            else if( m_CurrentPathStructIndex >= m_PathFindStruct.m_nSteps || !GetIsFollowingPath() || m_CurrentConnectionSlotChanged )
            {
                if( GetLocoPointer()->IsAtDestination() || m_CurrentConnectionSlotChanged )
                {
                    if( m_GoalRetreatToConnectionSlot == NULL_NAV_SLOT )
                    {
                        // we have completed our attempt once we get close to the end point.
                        m_FailedReason = FAILED_GOAL_NOWHERE_TO_RETREAT;
                        m_GoalSucceeded = FALSE;
                        completed = TRUE;
                    }
                    else
                    {
                        // update our current connection.
                        m_CurrentConnectionSlot = m_GoalRetreatToConnectionSlot;
                        m_CurrentConnectionSlotChanged = TRUE;

                        m_GoalRetreatToConnectionSlot = GetNewRetreatPath( GetTargetPosWithOffset(m_GoalInfo.m_TargetGuid), m_GoalInfo.m_DesiredDist );
                        m_GoalsConnectionSlot = m_GoalRetreatToConnectionSlot;
                        m_CanReachGoalTarget = UpdateCanReachTarget( GetGoalTrueLocation(), 50.0f );
                    }
                }
                else if( m_GoalRetreatToConnectionSlot == NULL_NAV_SLOT && m_TimeSinceLastRetreatJig >= k_MinTimeBetweenRetreatJigs )
                {
                    m_TimeSinceLastRetreatJig = 0.0f;
                    m_GoalRetreatToConnectionSlot = GetNewRetreatPath( GetTargetPosWithOffset(m_GoalInfo.m_TargetGuid), m_GoalInfo.m_DesiredDist );
                    m_GoalsConnectionSlot = m_GoalRetreatToConnectionSlot;
                    m_CanReachGoalTarget = UpdateCanReachTarget( GetGoalTrueLocation(), 50.0f );
                }
            }
            else if ( (m_LeashDistance > 0.0f) && 
                (GetPosition() - LeashPos).Length() > m_LeashDistance )
            {
                m_FailedReason = FAILED_GOAL_CANT_REACH;
                m_GoalSucceeded = FALSE;
                completed = TRUE;
            }
        }
        break;
    case character::GOAL_PLAY_ANIMATION:
    case character::GOAL_PLAY_ANIMATION_SCALED_TO_TARGET:
        // Wait for play anim to complete
        if (GetLocoPointer()->IsPlayAnimComplete())
        {
            m_GoalSucceeded = TRUE;
            completed = TRUE;
        }
        break;        
    case character::GOAL_SAY_DIALOG:
        if( !m_GoalInfo.m_DialogBlockOnDialog )
        {
            // we are done when the anim is done
            if( m_VoiceStarted && GetLocoPointer()->GetLipSyncController().IsAtEnd() )
            {
                // kill the lipsync track
                m_GoalSucceeded = TRUE;
                completed = TRUE;
            }
        }
        else
        {
            // we are done when the voice starts.
            if ( m_VoiceStarted && !g_ConverseMgr.IsPlaying(m_VoiceID) )
            {
                // kill the lipsync track
                if( m_GoalInfo.m_DialogKillAnim )
                {                
                    GetLocoPointer()->GetLipSyncController().Clear();
                }
                m_GoalSucceeded = TRUE;
                completed = TRUE;
            }
        }

        // error condition, we tried to play but it took too long so we gave up.
        if( m_TimeWaitingForVoiceToStart >= k_MaxTimeWaitingForVoiceStart &&
            m_LipSyncStartAnimFrame < 0.0f )
        {
            completed = TRUE;
            m_GoalSucceeded = FALSE;
            m_FailedReason = FAILED_GOAL_VOICE_TIMED_OUT;
            g_AudioMgr.Release( m_VoiceID, 0.0f );
            SetVoiceID(0);

#if defined( CONFIG_VIEWER ) || (defined( CONFIG_QA )&& !defined TARGET_XBOX)
//            x_DebugSetCause( xfs( "Voice Failed to Start!\n" ) );
//           *(u32*)1 = 0;
#endif
        }

        break;
    }
    return completed;
}

//=============================================================================

const char*character::GetGoalTypeName()
{
    switch( m_GoalInfo.m_GoalType )
    {
    case GOAL_NONE:
        return "GOAL_NONE";
        break;
    case GOAL_IDLE:
        return "GOAL_IDLE";
        break;
    case GOAL_TURNTO_TARGET:
        return "GOAL_TURNTO_TARGET";
        break;
    case GOAL_TURNTO_LOCATION:
        return "GOAL_TURNTO_LOCATION";
        break;
    case GOAL_LOOKAT_TARGET:
        return "GOAL_LOOKAT_TARGET";
        break;
    case GOAL_LOOKAT_LOCATION:
        return "GOAL_LOOKAT_LOCATION";
        break;
    case GOAL_GOTO_TARGET:
        return "GOAL_GOTO_TARGET";
        break;
    case GOAL_GOTO_LOCATION:
        return "GOAL_GOTO_LOCATION";
        break;
    case GOAL_RETREAT_FROM_TARGET:
        return "GOAL_RETREAT_FROM_TARGET";
        break;
    case GOAL_RETREAT_FROM_LOCATION:
        return "GOAL_RETREAT_FROM_LOCATION";
        break;
    case GOAL_PLAY_ANIMATION:
        return "GOAL_PLAY_ANIMATION";
        break;
    case GOAL_PLAY_ANIMATION_SCALED_TO_TARGET:
        return "GOAL_PLAY_ANIMATION_SCALED_TO_TARGET";
        break;
    case GOAL_SAY_DIALOG:
        return "GOAL_SAY_DIALOG";
        break;
    }
    return "UNKNOWN GOAL TYPE";
}

//=============================================================================

void character::UpdateVoice( f32 DeltaTime )
{
    if( m_VoiceID == 0 )
    {
        // special case, what to do if we lose our voice but anim was waiting for it?
        if( ( m_VoiceStarted == FALSE ) && ( m_LipSyncStartAnimFrame >= 0.0f ) )
        {
            m_LipSyncStartAnimFrame = -1.0f;
            GetLocoPointer()->GetLipSyncController().Clear();
        }
        return;
    }

    // If we have waited too long to play the voice then kill it.
    if( m_TimeWaitingForVoiceToStart >= k_MaxTimeWaitingForVoiceStart &&
        m_LipSyncStartAnimFrame < 0.0f)
    {
        g_AudioMgr.Release( m_VoiceID, 0.0f );
        SetVoiceID(0);
    }

    xbool bNeedToWait =
        ( !m_VoiceStarted && 
          g_AudioMgr.IsValidVoiceId( m_VoiceID ) &&
          ( m_TimeWaitingForVoiceToStart < k_MaxTimeWaitingForVoiceStart ||
            m_LipSyncStartAnimFrame >= 0.0f ) ); 

    (void)DeltaTime;
    if( bNeedToWait )
    {
        m_TimeWaitingForVoiceToStart += DeltaTime;

        // if the voice is has started and is warm 
        if ( g_ConverseMgr.IsReadyToPlay(m_VoiceID) &&
             GetLocoPointer()->m_Player.GetFrame() >= m_LipSyncStartAnimFrame )
        {
            // start the voice 
            m_VoiceStarted = TRUE;
            m_TimeWaitingForVoiceToStart = 0.0f;
            g_ConverseMgr.StartSound(m_VoiceID);

            // start the lip-sync animation
            if( m_CurrentDialogInfo.m_AnimName[0] != 0 && 
                m_LipSyncStartAnimFrame < 0.0f )
            {                
                anim_group::handle hAnimGroup ;
                if (m_CurrentDialogInfo.m_AnimGroup[0] != 0)
                    hAnimGroup.SetName(m_CurrentDialogInfo.m_AnimGroup);
                else
                    hAnimGroup = GetLocoPointer()->GetAnimGroupHandle();
                
                // Start lip sync anim
                if( GetLocoPointer()->PlayLipSyncAnim( hAnimGroup, m_CurrentDialogInfo.m_AnimName, m_VoiceID, m_CurrentDialogInfo.m_AnimFlags ) )
                {
                    GetLocoPointer()->GetLipSyncController().SetBlendOutTime( m_CurrentDialogInfo.m_BlendOutTime );
                }
                else
                {
                    // if we failed to play the anim we can't wait on it for completion of the goal.
                    m_GoalInfo.m_DialogBlockOnDialog = TRUE;
                }
            }
        }
        else if (!g_ConverseMgr.IsActive(m_VoiceID))
        {
            //hmm sound is not active, must have been bumped out of the list, re-warmup
            SetVoiceID( g_ConverseMgr.PlayStream(m_CurrentDialogInfo.m_DialogName, GetPosition(), GetGuid(), GetZone1(), IMMEDIATE_PLAY, FALSE, m_CurrentDialogInfo.m_DialogFlags ) );
        }
    }
    else if ( !g_ConverseMgr.IsPlaying(m_VoiceID) )
    {
        if( m_CurrentDialogType == DIALOG_COVER_REQ ||
            m_CurrentDialogType == DIALOG_RELOAD ||
            m_CurrentDialogType == DIALOG_GRENADE_THROW )
        {
            //BroadcastCoverRequestDialogDone();
            SendAlert( alert_package::ALERT_TYPE_REQUEST_COVER_FIRE_DIALOG_DONE, 0, TRUE );
        }

        if ( m_pNoBlockDialogTrigger != 0 )
        {
            //check for post delay  --bjt
            object* pObject = g_ObjMgr.GetObjectByGuid( m_pNoBlockDialogTrigger );
            if( pObject && pObject->IsKindOf(trigger_ex_object::GetRTTI()) )
            {
                trigger_ex_object &tempTrigger = trigger_ex_object::GetSafeType(*pObject);
                tempTrigger.ReleaseBlocking();
            }
            m_pNoBlockDialogTrigger = 0;
        }
        SetVoiceID(0);
    }
    else if ( g_ConverseMgr.IsPlaying(m_VoiceID) )
    {
        vector3 myPosition = GetPosition();
        g_ConverseMgr.SetPosition( m_VoiceID, myPosition, GetZone1() );
    }
}

//=============================================================================

void character::UpdateCurrentConnection()
{   
    CONTEXT( "character::UpdateCurrentConnection" );

    m_InNavMap = FALSE;
    nav_connection_slot_id newConnection = m_CurrentConnectionSlot; 

    // if we didn't have a connection
    if( newConnection == NULL_NAV_SLOT )
    {
        // then find the closest
        newConnection = g_NavMap.GetNearestConnection( GetPosition() );
        if( newConnection != NULL_NAV_SLOT )
        {
            // are we in it?
            ng_connection2 Conn = g_NavMap.GetConnectionByID( newConnection );
            m_InNavMap = Conn.IsPointInConnection( GetPosition() );
        }
    }
    // if we already had a connection...
    else 
    {
        ng_connection2 currentConnection = g_NavMap.GetConnectionByID( newConnection );
        if( !currentConnection.GetEnabled() )
        {
            newConnection = g_NavMap.GetNearestConnection( GetPosition() );
            if( newConnection != NULL_NAV_SLOT )
            {
                // are we in it?
                ng_connection2 Conn = g_NavMap.GetConnectionByID( newConnection );
                m_InNavMap = Conn.IsPointInConnection( GetPosition() );
            }
        }
        else if( currentConnection.GetEnabled() && currentConnection.IsPointInConnection(GetPosition()) )
        {
            // we are still in our connection. We are happy!
            m_InNavMap = TRUE;
        }
        else
        {
            // we are no longer in our connection. are we in a new one?
            if( g_NavMap.GetConnectionContainingPoint(newConnection, GetPosition()) )
            {
                m_InNavMap = TRUE;
            }
        }
    }
    // set our current connection slot and if it changed
    if( newConnection != NULL_NAV_SLOT && m_CurrentConnectionSlot != newConnection )
    {
        m_CurrentConnectionSlot = newConnection;
        m_CurrentConnectionSlotChanged = TRUE;
    }
    // if not using navmap, always considered in.
    if( m_PathingHints.bUseNavMap == FALSE )
    {
        m_InNavMap = TRUE;
    }
    // clear time out of nav map if inside it.
    if( m_InNavMap )
    {
        m_TimeOutofNavMap = 0.0f;
    }
}

//=============================================================================

void character::UpdateAimToTarget()
{
    CONTEXT( "character::UpdateAimToTarget" );

    vector3 Delta = GetTargetPosWithOffset() - GetPosition();
    Delta.GetY() = 0;

    // Outside of field of view?
    radian LookatYaw;
    LookatYaw = GetLocoPointer()->GetAimerYaw();

    radian TargetYaw = Delta.GetYaw();
    m_AimToTargetYaw = x_abs(x_MinAngleDiff(LookatYaw, TargetYaw));
}

//=============================================================================

void character::AttemptToAvoidActorCollision( f32 DeltaTime )
{
    object *pObject = g_ObjMgr.GetObjectByGuid( m_CollidedActor );
    vector3 toTarget = GetToTarget(m_CollidedActor);
    vector3 toNavTarget = m_DesiredLocation - GetPosition();
    f32 angleTargetNav = x_abs( x_MinAngleDiff(toTarget.GetYaw(),toNavTarget.GetYaw()) );
    xbool needsToAvoid = TRUE;

    // increment timer and, see if we need to avoid.
    m_AvoidAttemptTime += DeltaTime;

    if( !pObject || IsEnemyFaction(GetFactionForGuid(m_CollidedActor)) || angleTargetNav > R_80 )
    {
        needsToAvoid = FALSE;
    }

    // first figure out which avoid attempt we are going to use.
    switch( m_AvoidActorAttempt )
    {
    case AVOID_NONE:
        {
            if( needsToAvoid )
            {
                // see which way the collided actor is moving...
                if( pObject && pObject->IsKindOf(actor::GetRTTI()) )
                {
                    actor &actorObject = actor::GetSafeType(*pObject);
                    f32 angleDiff = x_MinAngleDiff(toNavTarget.GetYaw(),actorObject.GetLocoPointer()->m_Physics.GetVelocity().GetYaw());
                    xbool actorMoving = ( actorObject.GetLocoPointer()->GetState() == loco::STATE_MOVE );

                    // we will only wait if the actor is moving.
                    if( angleDiff < R_80 && actorMoving )
                    {
                        m_AvoidActorAttempt = AVOID_WAIT;
                        m_AvoidAttemptTime = x_frand(1.5f,2.5f);                
                        if( actorObject.IsKindOf(character::GetRTTI()) &&
                            actorObject.GetLocoPointer() )
                        {
                            character &characterObject = character::GetSafeType(actorObject);
                            f32 characterSpeed = characterObject.GetAnimRate();
                            if( characterSpeed < GetAnimRate() )
                            {
                                // swap anim rates.
                                characterObject.SetAnimRate(GetAnimRate());
                                SetAnimRate(characterSpeed);
                            }
                        }
                    }
                    else
                    {
                        m_AvoidActorAttempt = AVOID_THROUGH;
                        m_PreviousActorCollisionRadius = 0.0f;
                        m_AvoidAttemptTime = x_frand(-0.5f,0.5f);
                    }
                }
                else
                {                    
                    m_AvoidActorAttempt = AVOID_THROUGH;
                    m_PreviousActorCollisionRadius = 0.0f;
                    m_AvoidAttemptTime = x_frand(-0.5f,0.5f);
                }
            }
        }
        break;
    case AVOID_WAIT:
        {
            if( needsToAvoid )
            {
                if( m_AvoidAttemptTime > k_MaxAvoidTime )
                {
                    m_AvoidActorAttempt = AVOID_THROUGH;
                    m_PreviousActorCollisionRadius = 0.0f;
                    m_AvoidAttemptTime = x_frand(-0.5f,0.5f);
                }
            }
            else
            {
                if( m_AvoidAttemptTime > k_MinAvoidTime )
                {
                    m_AvoidAttemptTime = 0.0f;
                    m_AvoidActorAttempt = AVOID_NONE;
                }
            }
        }
        break;
    case AVOID_THROUGH:
        {
            // as long as we are colliding, reduce the size... goto 0 in 1/2 second.
            // return to normal in 1 second.
            if( m_ModifiedActorCollisionRadius >= m_PreviousActorCollisionRadius )
            {
                m_AvoidAttemptTime = 0.0f;
                m_AvoidActorAttempt = AVOID_NONE;
            }
        }
        break;
    }

    // now we actually handle the avoid type.
    switch( m_AvoidActorAttempt )
    {
    case AVOID_NONE:
        break;
    case AVOID_WAIT:
        {
            m_DesiredLocation = GetPosition();
        }
        break;
    case AVOID_THROUGH:
        {   
            // first time through?
            if( m_PreviousActorCollisionRadius == 0.0f )
            {
                m_PreviousActorCollisionRadius = GetLocoPointer()->m_Physics.GetActorCollisionRadius();
                m_ModifiedActorCollisionRadius = m_PreviousActorCollisionRadius;
            }
            
            if( needsToAvoid )
            {
                m_ModifiedActorCollisionRadius -= m_PreviousActorCollisionRadius*DeltaTime/0.5f;
                if( m_ModifiedActorCollisionRadius < 0.0f )
                {
                    m_ModifiedActorCollisionRadius = 0.0f;
                }
            }
            else
            {
                m_ModifiedActorCollisionRadius += m_PreviousActorCollisionRadius*DeltaTime/1.0f;
                if( m_ModifiedActorCollisionRadius > m_PreviousActorCollisionRadius )
                {
                    m_ModifiedActorCollisionRadius = m_PreviousActorCollisionRadius;
                }
            }
            GetLocoPointer()->m_Physics.SetActorCollisionRadius(m_ModifiedActorCollisionRadius);
        }
        break;
    }
}


//=============================================================================

void character::UpdateMoveTo( f32 DeltaTime )
{
    CONTEXT( "character::UpdateMoveTo" );

    if( DoingMovementGoal() )
    {
        if( m_PathingHints.bUseNavMap == FALSE )
        {
            m_DesiredLocation = m_PathFindStruct.m_vEndPoint;
            GetLocoPointer()->SetMoveAt( m_DesiredLocation );       
            GetLocoPointer()->SetExactMove( m_GoalInfo.m_IsExact );
            return;        
        }
    
        object *pObject = g_ObjMgr.GetObjectByGuid( m_CollidedActor );
        if( (pObject || m_AvoidActorAttempt != AVOID_NONE) && m_InNavMap )
        {
            AttemptToAvoidActorCollision(DeltaTime);
        }
        else if( m_CurrentEscapeAttempt != ESCAPE_NONE )
        {
            m_DesiredLocation = GetPosition() + m_UnstuckVector;
        }
        else
        {
            m_AvoidAttemptTime = 0.0f;
            m_AvoidActorAttempt = AVOID_NONE;
            m_CurrentEscapeAttempt = ESCAPE_NONE;
            m_EscapeAttempts = 0;   

            // restore our actor collision...
            if( GetLocoPointer()->m_Physics.GetActorCollisionRadius() <= 0.0f &&
                m_PreviousActorCollisionRadius > 0.0f )
            {
                GetLocoPointer()->m_Physics.SetActorCollisionRadius(m_PreviousActorCollisionRadius);
                m_PreviousActorCollisionRadius = 0.0f;
            }          
        }
        
        GetLocoPointer()->SetMoveAt( m_DesiredLocation );       
        
        if( m_GoalInfo.m_IsExact == FALSE )
        {
            GetLocoPointer()->SetExactMove(FALSE);
        }
        else if( m_DesiredLocation == m_PathFindStruct.m_vEndPoint ||
            !m_bCanReachPathDest )
        {        
            GetLocoPointer()->SetExactMove(TRUE);
        }
        else
        {
            GetLocoPointer()->SetExactMove(FALSE);
        }
    }
    else 
    {
        m_AvoidAttemptTime = 0.0f;
        m_AvoidActorAttempt = AVOID_NONE;
        m_CurrentEscapeAttempt = ESCAPE_NONE;
        m_EscapeAttempts = 0;   
        m_TimeStuck = 0.0f;

        // Update move at when playing animation back
        if( !GetLocoPointer()->IsExactMoveBlending() )
        {        
            GetLocoPointer()->SetMoveAt( GetPosition() );
        }
    }
}

f32 character::GetSqrHorzDistToNextNav()
{
    vector3 toNextNav = GetPosition() - m_NextPathPoint;
    toNextNav.GetY() = 0.0f;
    return toNextNav.LengthSquared();
}

void character::SwitchLookatMode()
{
    object *lookatObject = g_ObjMgr.GetObjectByGuid(m_OverrideLookatInterest);

    vector3 toLookat;
    radian angleToLookat = 0.0f;
    if( lookatObject )
    {
        toLookat = GetToTarget( lookatObject->GetGuid() );
        angleToLookat = x_abs( x_MinAngleDiff(toLookat.GetYaw(),GetLocoPointer()->GetYaw()) );
    }
    vector3 toCover;

    // first set ourselves to the correct lookatmode
    eLookatModes newMode = LOOKAT_NONE;

    // if we have a goal involving lookat, we must serve it but wait a sec just incase this 
    // goal is going to quick pop out.
    if( m_GoalInfo.m_GoalType == GOAL_TURNTO_LOCATION ||
        m_GoalInfo.m_GoalType == GOAL_TURNTO_TARGET || 
        m_GoalInfo.m_GoalType == GOAL_LOOKAT_LOCATION || 
        m_GoalInfo.m_GoalType == GOAL_LOOKAT_TARGET )
    {
        newMode = LOOKAT_DOING_GOAL;
    }
    // if I have a target and I see him, look at em.
    // however don't do this if fleeing when bool set.
    // or surprisingly if we can't move backwards...
    else if( GetAwarenessLevel() >= AWARENESS_TARGET_SPOTTED &&
             m_GoalInfo.m_FaceTargetOnRetreat &&
             GetLocoPointer()->IsMotionAllowed(loco::MOTION_BACK) )
    {
        newMode = LOOKAT_CURRENT_TARGET;
    }
    // otherwise check out our lookat if we have one
    else if( lookatObject && 
        (angleToLookat < R_90 || 
        GetLocoPointer()->GetState() != loco::STATE_PLAY_ANIM) )
    {
        newMode = LOOKAT_INTEREST_OBJECT;
    }
    // otherwise, if I'm moving and more than half a meter away, lookat where I'm going. 
    // if less than a meter, either look forward, or if going into cover, start aligning for cover.
    else if( DoingMovementGoal() )
    {
        toCover = GetGoalTargetsLocation() - GetPosition();
        f32 coverYDiff = x_abs(toCover.GetY());
        toCover.Set( toCover.GetX(),0.0f,toCover.GetZ() );
        f32 toTargetCheck = 100.0f * 100.0f;
        if( GetLocoPointer()->GetMoveStyle() == loco::MOVE_STYLE_RUN ||
            GetLocoPointer()->GetMoveStyle() == loco::MOVE_STYLE_RUNAIM )
        {
            toTargetCheck = 200.0f * 200.0f;
        }

        if( m_pActiveState && 
            m_pActiveState->GetStateType() == character_state::STATE_COVER && 
            coverYDiff <= 200.0f &&
            toCover.LengthSquared() < toTargetCheck &&
            m_CoverIsValid )
        {
            newMode = LOOKAT_ENTERING_COVER;
        }
        else if( m_pActiveState && 
            m_pActiveState->GetStateType() == character_state::STATE_TRIGGER && 
            coverYDiff <= 200.0f &&
            toCover.LengthSquared() < toTargetCheck &&
            ((character_trigger_state*)m_pActiveState)->GetTriggerData().m_UnionData.m_PathfindData.m_Distance == -1.0f )
        {
            newMode = LOOKAT_MARKER_ALIGN;
        }
        else if( GetSqrHorzDistToNextNav() >= k_MinDistToLookatMoveatSqr )
        {
            newMode = LOOKAT_NAVIGATION;
        }
        else
        {            
            newMode = LOOKAT_NAV_FORWARD;
        }
    }
    // or just look forward.
    // if I have a target and lost him, look at em.
    // however don't do this if fleeing when bool set.
    
    // changed this from spotted to lost so NPCs will look at
    // the last known location of thier target. Looked strange when they went back
    // to looking straight ahead
    else if( GetAwarenessLevel() >= AWARENESS_TARGET_LOST &&
             m_GoalInfo.m_FaceTargetOnRetreat &&
             ( HasWeaponEquiped() ||
               GetToTarget().LengthSquared() <= k_MinDistMeleeLookatSqr))
    {
        newMode = LOOKAT_CURRENT_TARGET;
    }
    else 
    {
        newMode = LOOKAT_FORWARD;
    }

    // if we have an override this takes priority.
    if( m_OverrideLookatMode != LOOKAT_NONE )
    {
        newMode = m_OverrideLookatMode;
    }

    if( newMode != m_LookatMode &&
        newMode != LOOKAT_NONE )
    {
        m_TimeSinceLastLookatSwitch = 0.0f;
        m_LastLookatMode = m_LookatMode;
        m_LookatMode = newMode;
    }
}

void character::UpdateLookAt( )
{
    CONTEXT( "character::UpdateLookAt" );

    vector3 Pos = GetPosition();
    radian  Yaw = GetLocoPointer()->m_Player.GetFacingYaw();
    vector3 Offset = vector3(0, 0, 1000);
    // Any loco?
    // do not update our lookat for the first half-second of our existance.
    // allows everything to get setup first.   
    if ( !GetLocoPointer() )
    {    
        return;
    }
    else if ( m_TimeActive < 0.5f )
    {
        // look straight ahead
        Offset.RotateY(Yaw);
        Pos += Offset + GetLocoPointer()->GetEyeOffset() ;
        GetLocoPointer()->SetLookAt(Pos);
        return;
    }

    // setup our lookatobject
    object *lookatObject = g_ObjMgr.GetObjectByGuid(m_OverrideLookatInterest);
    
    vector3 toLookat;
    radian angleToLookat = 0.0f;
    if( lookatObject )
    {
        toLookat = GetToTarget( lookatObject->GetGuid() );
        angleToLookat = x_abs( x_MinAngleDiff(toLookat.GetYaw(),GetLocoPointer()->GetYaw()) );
    }

    SwitchLookatMode();    
        
    switch (m_LookatMode)
    {
    case LOOKAT_FORWARD:      
        {        
            object *lookatObject = g_ObjMgr.GetObjectByGuid(m_OverrideLookatInterest);
            if( lookatObject )
            {
                GetLocoPointer()->SetLookAt( GetTargetPosWithOffset(lookatObject->GetGuid(),OFFSET_EYES) );
            }
            else
            {   
                // look straight ahead
                Offset.RotateY(Yaw);
                Pos += Offset + GetLocoPointer()->GetEyeOffset() ;
                GetLocoPointer()->SetLookAt(Pos);
            }
            break;
        }
    case LOOKAT_INTEREST_OBJECT:
        {
            object *lookatObject = g_ObjMgr.GetObjectByGuid(m_OverrideLookatInterest);
            if( lookatObject )
            {
                GetLocoPointer()->SetLookAt( GetTargetPosWithOffset(lookatObject->GetGuid(),OFFSET_EYES) );
            }
            else
            {   
                m_OverrideLookatInterest = 0;
            }
        }
        break;
    case LOOKAT_LAST_LOCATION_OF_INTEREST:
        // look at the last sound pos.
        GetLocoPointer()->SetLookAt(m_LastLocationOfInterest);
        break;
    case LOOKAT_NAV_FORWARD:      
        {        
            // look straight ahead
            Offset.RotateY(Yaw);
            Pos += Offset + GetLocoPointer()->GetEyeOffset() ;
            GetLocoPointer()->SetLookAt(Pos);
            break;
        }
    case LOOKAT_NAVIGATION:
        {        
            // Compute eye Y position
            f32 EyeOffset = GetLocoPointer()->GetEyeOffset().GetY();
            
            // Look up eye info
            vector3 BodyEye = GetPosition();
            vector3 PathEye = m_NextPathPoint;
            
            // If path point is more than 1.5 meters above current position, then look at it
            if( x_abs( BodyEye.GetY() - PathEye.GetY() ) > 150.0f )
            {
                // Look up/down at path point
                BodyEye.GetY() += EyeOffset;
                PathEye.GetY() += EyeOffset;
            }
            else
            {
                // Look horizontal
                BodyEye.GetY() += EyeOffset;
                PathEye.GetY() = BodyEye.GetY();
            }
            
            // Compute direction to look out from body to make sure look at is infront 
            // of eyes AND body (stop npcs wanting to flip 180 when look at is between body and eye)
            vector3 LookDir = PathEye - BodyEye;
            LookDir.NormalizeAndScale( 200.0f );
            
            // Finally, update look at 
            GetLocoPointer()->SetLookAt( BodyEye + LookDir );            
        }
        break;
    case LOOKAT_CURRENT_TARGET:
        // we look at our current target.
        GetLocoPointer()->SetLookAt( GetShootAtPosition() );
        break;
    case LOOKAT_ENTERING_COVER:
        if( m_pActiveState && 
            m_pActiveState->GetStateType() == character_state::STATE_COVER )
        {
            object *coverObject = g_ObjMgr.GetObjectByGuid(GetCurrentCover());
            if( coverObject && coverObject->IsKindOf(cover_node::GetRTTI()) )
            {
                cover_node& coverNode = cover_node::GetSafeType( *coverObject );
                Offset.RotateY( coverNode.GetNPCFacing() );
                Pos = coverNode.GetPosition();
                Pos += Offset + GetLocoPointer()->GetEyeOffset() ;
                GetLocoPointer()->SetLookAt(Pos);
            }
        }
        break;
    case LOOKAT_MARKER_ALIGN:
        if( m_pActiveState && 
            m_pActiveState->GetStateType() == character_state::STATE_TRIGGER )
            {
                object *markerObject = g_ObjMgr.GetObjectByGuid( ((character_trigger_state*)m_pActiveState)->GetTriggerData().m_ActionFocus );
                if( markerObject )
                {
                    vector3 positionFacing = vector3(0.0f, 0.0f, 500.0f);
                    positionFacing.RotateY( markerObject->GetL2W().GetRotation().Yaw );
                    positionFacing += markerObject->GetPosition();
                    positionFacing.Set(positionFacing.GetX(),GetPosition().GetY()+GetLocoPointer()->GetEyeOffset().GetY(),positionFacing.GetZ());
                    GetLocoPointer()->SetLookAt(positionFacing);
                }
            }
        break;
    case LOOKAT_DOING_GOAL:
        // look at the goal!
        GetLocoPointer()->SetLookAt( GetGoalTrueLocation() );
        break;
    }
    
    xbool isInLookat = FALSE;
    // finally we want to update the head lookat if we have one.
    object *headLookatObject = g_ObjMgr.GetObjectByGuid( m_HeadLookat );
    object *overrideLookat = g_ObjMgr.GetObjectByGuid( m_OverrideLookatInterest );
    if( headLookatObject &&
        !overrideLookat &&
        !IsPlayingFullBodyLipSync() )
    {
        // is the object close enough and in front?
        vector3 toTarget = GetToTarget( m_HeadLookat );
        if( toTarget.LengthSquared() <= m_HeadLookatDistance * m_HeadLookatDistance )
        {
            // is the player in front of us?
            // bias if already headtracking. 
            radian minAngleToLookat = m_HeadLookatAngle;
            if( m_bIsInHeadTracking )
            {
                minAngleToLookat += R_5;
            }
            if( x_abs( x_MinAngleDiff( toTarget.GetYaw(), GetYaw() ) ) <= DEG_TO_RAD(minAngleToLookat) )
            {
                isInLookat = TRUE;
            }
        }        

        // we have a target that is in our cone
        if( isInLookat )
        {
            // are we not looking and has he been outside the cone for a while? 
            if( m_OutofLookatTimer >= k_MinTimeOutsideConeToStartHeadLookat && 
                m_HeadLookatTimer <= 0.0f )
            {
                m_HeadLookatTimer = x_frand( 3.0f, 5.0f );
                m_OutofLookatTimer = 0.0f;
            }

            if( m_HeadLookatTimer > 0.0f )
            {
                GetLocoPointer()->SetHeadLookAt( GetTargetPosWithOffset(headLookatObject->GetGuid(),OFFSET_EYES) );
            }
        }
        // we have a target out of our cone
        else
        {
            m_HeadLookatTimer = 0.0f;
        }
    }    
    else
    {
        m_HeadLookatTimer = 0.0f;
    }
    m_bIsInHeadTracking = isInLookat;
}

//=============================================================================

void character::SetRecoverState( character_state::states NewState )
{
    ( void ) NewState;
}


//=============================================================================
//
//  Given that we know the target, and have a weapon, where should the bullet
//  go?
//
vector3 character::GetShootAtPosition()
{
    vector3 shootAtPosition = GetTargetPosWithOffset(0, OFFSET_EYES );
    if( !CanSeeTarget() )
    {
        shootAtPosition = GetLastSeenLocationOfTarget();
    }
    return shootAtPosition;
/*    
    object_ptr< new_weapon > WeaponPtr( m_WeaponGuid );

    if (!WeaponPtr.IsValid())
    {
        return GetLastLocationOfInterest();
    }

    vector3 FirePointPos;
    vector3 AimPointPos;
    if (WeaponPtr.m_pObject->GetFiringBonePosition(FirePointPos) && WeaponPtr.m_pObject->GetAimBonePosition(AimPointPos))
    {
        vector3 TargetingDir = FirePointPos - AimPointPos;
        TargetingDir.Normalize();
        TargetingDir.Scale(2000); //give it some distance
        return TargetingDir;
    }  
    else
    {
        return GetLastLocationOfInterest();
    }*/
}

//=============================================================================
// Handles event - returns TRUE if it was processed
//=============================================================================

xbool character::OnAnimEvent( const anim_event& Event, const vector3& WorldPos )
{
    // Let base class have a go first...
    if( actor::OnAnimEvent( Event, WorldPos ))
        return TRUE;

    // get our target and our weapon
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( !pWeapon )
        return FALSE;
        
    // Old event?
    if (x_stricmp(Event.GetType(), "Old Event") == 0)
    {
        switch ( Event.GetInt( anim_event::INT_IDX_OLD_TYPE) )
        {
        case ANIM_EVENT_PRIMARY_FIRE:
            FireWeapon();
            break;
        case ANIM_EVENT_SECONDARY_FIRE:
            FireWeapon(FALSE);
            break;
        case ANIM_EVENT_GRAB_GRENADE:
            //need to render the grenade geometry with the player.
//            m_bRenderGrenade = TRUE;
            break;
        case ANIM_EVENT_RELEASE_GRENADE:
//            m_bRenderGrenade = FALSE;
//            ThrowGrenade();
            break;
        }
    }

    // Event not handled
    return FALSE;
}

//=============================================================================

xbool character::GetIsKungFuTime()
{
    // only melee NPCs kung fu.
    if( GetCurrentWeaponPtr() )   
    {
        return FALSE;
    }
    
    object *targetObject = g_ObjMgr.GetObjectByGuid( GetTargetGuid() );

    god* pGod = SMP_UTIL_Get_God();
    if( targetObject &&
        targetObject->IsKindOf(player::GetRTTI()) &&
        !pGod->GetCanMeleePlayer(GetGuid()) )
    {
        return TRUE;
    }
    return FALSE;
}

//=============================================================================
// Misc virtual functions
//=============================================================================

xbool character::TakeDamage( const pain& Pain )
{
    // Call base class
//    if( !actor::TakeDamage( Pain ) )
//        return FALSE;
        
    ASSERT( Pain.ComputeDamageAndForceCalled() );

    f32 Damage = Pain.GetDamage();

    Damage = ModifyDamageByDifficulty(Damage);

    m_Health.Sub( Damage );

    factions sourceFaction = GetFactionForGuid(Pain.GetOriginGuid());
    
    // tell everyone we've taken damage.
//    Shout( Pain.GetOriginGuid() );
    object *painSource = g_ObjMgr.GetObjectByGuid( Pain.GetOriginGuid() );
    if( painSource && IsEnemyFaction(sourceFaction) )
    {
/*        if( Pain.GetHitType() == 4 )
        {
            PlayDialog( DIALOG_HIT_MELEE );
        }
        else
        {
            PlayDialog( DIALOG_HIT );
        }*/
        if( painSource->IsKindOf(actor::GetRTTI()) )
        {
            actor &actorSource = actor::GetSafeType( *painSource );        
            actorSource.OnDamagedEnemy( GetGuid() );
        }
    }

    if( painSource && 
        painSource->IsKindOf(player::GetRTTI()) )
    {
        if( IsFriendlyFaction(GetFactionForGuid(Pain.GetOriginGuid())) &&
            m_TimeSincePlayerDamagedMeLast >= k_MinTimeBetweenPlayerDamageShots &&
            GetTimeSinceLastCombat() >= k_MinTimeTillCombatClear )
        {
            m_TimesPlayerHasDamagedMe++;
            if( m_TimesPlayerHasDamagedMe >= 3 )
            {
//                BroadcastPlayerTurned();
                SendAlert( alert_package::ALERT_TYPE_PLAYER_TURNED, 0, TRUE );
                TurnAgainstPlayer();
            }
        }
        m_TimeSincePlayerDamagedMeLast = 0.0f;
    }

    if( m_Health.GetHealth() <= 0.0f )
    {    
        // save info on the pain that killed us.
        m_PainThatKilledUs = Pain;

        // unequip our weapon.
        UnequipCurrentWeapon();

        object *painSource = g_ObjMgr.GetObjectByGuid( Pain.GetOriginGuid() );
        if( painSource )
        {
//            BroadcastActorDeath( Pain.GetOriginGuid() );
            SendAlert( alert_package::ALERT_TYPE_ACTOR_DIED, Pain.GetOriginGuid(), TRUE );
            if( painSource->IsKindOf(actor::GetRTTI()) )
            {            
                actor &actorSource = actor::GetSafeType( *painSource );        
                if( IsEnemyFaction(sourceFaction) )
                {
                    actorSource.OnKilledEnemy( GetGuid() );
                }
                else
                {
                    actorSource.OnKilledAlly( GetGuid() );
                }
            }
        }

        // This is handled in actor::TakeDamage
        //m_PainThatKilledUs = Pain;
        OnDeath(); // You are DEAD!
        //Damage = 500.0f;    // "Spike" the pain for the net kill.
    }

    // if enemy set up as our target if closer than current target.
    if( IsEnemyFaction(sourceFaction) &&
        !IgnoreAttacks() && 
        IsValidTarget(Pain.GetOriginGuid()) )
    {
        m_TargetNotSeenTimer = 0.0f;
        if( (GetTargetGuid() == 0 || IsNewTargetCloser(Pain.GetOriginGuid())) )
            SetTargetGuid(Pain.GetOriginGuid());
    }

    // Damage taken
    return TRUE;
}

//=============================================================================

xbool character::IsValidTarget( guid targetGuid )
{
    object *targetObject = g_ObjMgr.GetObjectByGuid( targetGuid );
    if( !targetObject )
    {    
        return FALSE;
    }

    // dead, inactive, and invulnerable turrets are not valid.
    if( targetObject->IsKindOf(turret::GetRTTI()) )
    {
        turret &turretObject = turret::GetSafeType( *targetObject ); 
        if( !turretObject.IsActive() ||
            !turretObject.IsAlive() || 
            turretObject.IsIndestructable() ||
            turretObject.IsDestroyed() )
        {
            return FALSE;
        }
    }
    
    // dead, and inactive actors are not valid.
    if( targetObject->IsKindOf(actor::GetRTTI()) )
    {
        actor &actorObject = actor::GetSafeType( *targetObject ); 
        if( !actorObject.IsActive() ||
            !actorObject.IsAlive() )
        {
            return FALSE;
        }
    }

    // characters in turrets are not valid.
    if( targetObject->IsKindOf(character::GetRTTI()) )
    {
        character &characterObject = character::GetSafeType( *targetObject );
        if( characterObject.GetCurrentStateType() == character_state::STATE_TURRET )
        {
            return FALSE;
        }
    }

    return TRUE;
}

//=============================================================================

void character::OnKilledAlly( guid deadAlly )
{
    (void)deadAlly;
    PlayDialog( DIALOG_FRIENDLY_WOUND );
}

void character::OnKilledEnemy( guid deadEnemy )
{
    (void)deadEnemy;
    PlayDialog( DIALOG_KILL );
}

void character::OnDamagedEnemy( guid damagedEnemy )
{
    (void)damagedEnemy;
}

//=============================================================================

xbool character::IgnoreAttacks()
{
    if( !m_pActiveState || m_pActiveState->IgnoreAttacks() )
    {    
        return TRUE;
    }
    return FALSE;
}

//=============================================================================

// Handles being shot at...
void character::OnBeingShotAt( object::type ProjectileType , guid ShooterID )
{
    //call the active state's On Being Shot
    if ( m_pActiveState )
    {
        m_pActiveState->OnBeingShotAt( ProjectileType ,ShooterID );
    }
    
    // do we want to complain that we are being shot at?
    PlayDialog( DIALOG_UNDER_FIRE );
}

//=============================================================================

void character::OnHitByFriendly( guid ShooterID )
{
    if ( m_pActiveState )
    {
        PlayDialog( DIALOG_FRIENDLY_HIT );
        m_pActiveState->OnHitByFriendly( ShooterID );
    }
}

//=============================================================================

xbool character::OnPlayFullBodyImpactAnim( loco::anim_type AnimType, f32 BlendTime, u32 Flags )
{
    // Skip if playing full body lip sync
    if( IsPlayingFullBodyLipSync() )
        return FALSE;

    // State present?
    if( m_pActiveState )
    {
        // Let state decide
        return m_pActiveState->OnPlayFullBodyImpactAnim( AnimType, BlendTime, Flags );
    }
    else
    {           
        // No state, just use base class version
        return actor::OnPlayFullBodyImpactAnim( AnimType, BlendTime, Flags );
    }
}

//=============================================================================

void character::OnHitFriendly( guid FriendlyID )
{
    if ( m_pActiveState )
    {
        PlayDialog( DIALOG_FRIENDLY_WOUND );
        m_pActiveState->OnHitFriendly( FriendlyID );
    }
//  if we hit a friend, stop shooting.
    if( GetLocoPointer() ) 
    {
        GetLocoPointer()->GetAdditiveController(ANIM_FLAG_SHOOT_CONTROLLER).Clear();
    }
}

//=============================================================================

void character::SetCurrentCover( guid CoverNode )
{
    // Going to different cover node?
    if( GetCurrentCover() != CoverNode )
    {
        // Tell state cover has changed
        if( m_pActiveState )
            m_pActiveState->OnCoverChanged( CoverNode );
    }
    
    // Record
    m_CurrentCover = CoverNode;
}

//=============================================================================

xbool character::GetIsCoverSticky( guid coverGuid )
{
    object *coverObject = g_ObjMgr.GetObjectByGuid( coverGuid );
    object *stickyCoverObject = g_ObjMgr.GetObjectByGuid( GetStickyCoverNode() );
    if( coverObject &&
        stickyCoverObject )
    {
        if( stickyCoverObject->IsKindOf(cover_node::GetRTTI()) &&
            coverGuid == GetStickyCoverNode() ) 
        {
            return TRUE;
        }
        else if ( stickyCoverObject->IsKindOf(group::GetRTTI()) )
        {
            group &coverGroup = group::GetSafeType(*stickyCoverObject);
            if( coverGroup.ContainsGuid(coverGuid) )
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}
//=============================================================================

void character::UpdateCoverNode()
{
    character_cover_state *coverState = NULL;
    if( m_pActiveState &&
        m_pActiveState->GetStateType() == character_state::STATE_COVER )
    {
        coverState = (character_cover_state*)m_pActiveState;
    }


    // we only update if we can do something with the cover...
    if( !HasState(character_state::STATE_COVER) ||
        m_MaxDistToCover <= 0.0f )
    {
        SetCurrentCover( 0 );
        return;
    }

    if( GetCurrentCover() )
    {
        // if the cover object is gone...
        object* coverObject = g_ObjMgr.GetObjectByGuid(GetCurrentCover());
        if( !coverObject )
        {
            SetCurrentCover(0);
        }
        // finally, if we aren't combat ready and lack a target, then drop the cover.
        else if( !IsCombatReady() &&
                 !GetTargetGuid() && 
                 !GetIsCoverSticky(GetCurrentCover()) )
        {
            SetCurrentCover(0);
        }
        // if we only want covers ahead, then we may drop our current cover if we aren't in coverstate yet...
        // or even if we are in coverstate and we aren't close to the cover (1 meter).
        else if( m_bOnlyUsesCoverAhead &&
                 ( !coverState ||
                   m_pActiveState->GetCurrentPhase() < character_state::PHASE_BASE_COUNT ||
                   GetToTarget(GetCurrentCover()).LengthSquared() >= 100.0f * 100.0f) )
        {
            vector3 toTarget = GetToTarget();
            vector3 toNode = GetToTarget(GetCurrentCover());
            radian angleDiff = x_abs(x_MinAngleDiff(toTarget.GetYaw(),toNode.GetYaw()));
            if( angleDiff >= R_45 )
            {
                SetCurrentCover(0);
            }
        }
    }

    guid lastCover = GetCurrentCover(); 

    // if I have sticky cover, that is the only cover I can ever use...
    object *stickyCoverObject = g_ObjMgr.GetObjectByGuid(m_StickyCoverNode);

/*    // clear sticky if inactive.
    if( stickyCoverObject && 
        ! stickyCoverObject->IsActive() )
    {
        m_StickyCoverNode = 0;
        stickyCoverObject = NULL;
    }*/

    if( stickyCoverObject && stickyCoverObject->IsKindOf(cover_node::GetRTTI()) )
    {
        // let's make sure this is an object...
        SetCurrentCover( m_StickyCoverNode );
    }
    // bzzz wrongo! Do this and they won't switch from cover to cover
    // in their sticky group. Not a good thing. 
    else if( GetIsCoverSticky(GetCurrentCover()) &&
             m_bCoverHopper )
    {
        // we're good to go. Do nothing.
    }
    else if( stickyCoverObject && 
             stickyCoverObject->IsKindOf(group::GetRTTI()) &&
             !m_CurrentCover )
    {
        SetCurrentCover( FindNearestValidCover(COVER_PREF_CLOSER_TO_US) );
    }
    else
    {
        // if we have a max time in cover, invalidate the node when that time is up.
        if( m_bCoverHopper &&
            coverState &&
            coverState->GetTimeInCover() > coverState->GetTimeTillNextAction() &&
            GetIsInCover() )
        {
            // invalidate our current cover;
            object *coverObject = g_ObjMgr.GetObjectByGuid( GetCurrentCover() );
            if( coverObject && coverObject->IsKindOf(cover_node::GetRTTI()) )
            {
                cover_node &coverNode = cover_node::GetSafeType( *coverObject );
                coverNode.InvalidateNode();
                SetCurrentCover(0);
            }
        }
        // otherwise we only keep a valid cover around if we have a target.
        if( GetAwarenessLevel() >= AWARENESS_ACQUIRING_TARGET &&
            GetTargetNotSeenTimer() <= ( k_MinTimeAtCoverTargetLost + ( ( 1.0f - GetParametricHealth() ) * k_AdditionalTimeAtCoverTargetLost ) ) )
        {                   
            // if we have a target, but currently aren't in cover... update our cover
            if( !coverState )
            {
                if( m_TimeSinceLastCoverCheck >= k_MinTimeBetweenCoverChecksNoCover )
                {
                    SetCurrentCover( FindNearestValidCover( COVER_PREF_CLOSER_TO_TARGET, TRUE ) );
                }
            }
            else if( coverState->GetTimeInCover() <= 0.0f )
            {
                if( m_TimeSinceLastCoverCheck >= k_MinTimeBetweenCoverChecksRunning ||
                    !GetCurrentCover() )
                {
                    SetCurrentCover( FindNearestValidCover( COVER_PREF_CLOSER_TO_TARGET, TRUE ) );
                }
            }
            else
            {
                // when we are at a cover node, we will still only update our cover so often
                if( coverState &&
                    coverState->GetTimeInCover() >= k_MinTimeBetweenCoverChecksAtCover &&
                    m_TimeSinceLastCoverCheck >= k_MinTimeBetweenCoverChecksAtCover )
                {
                    if( !IsCoverNodeValid(GetCurrentCover()) ||
                        m_FriendlyBlocksTarget )
                    {
                        SetCurrentCover( FindNearestValidCover(COVER_PREF_CLOSER_TO_TARGET) );
                    }
                    else if ( coverState &&
                              coverState->GetTimeInCover() >= x_frand(k_MaxTimeInCover,k_MaxTimeInCover*2.0f) )
                    {
                        guid newCover = FindNearestValidCover( COVER_PREF_CLOSER_TO_TARGET );
                        if( newCover )
                        {
                            SetCurrentCover( newCover );
                            // changed to target spotted because otherwise ally NPCs were shooting 
                            // at the wall over and over providing covering fire for NPCs they knew
                            // were there but couldn't see. 
                            if( GetAwarenessLevel() >= AWARENESS_TARGET_SPOTTED )
                            {                            
                                if( RequestAutofire() )
                                {
                                    PlayDialog( DIALOG_RUSH );
                                }
                            }
                        }
                    }
                    else if ( CoverRetreatWhenDamaged() &&
                              m_DamageInCover >= m_MaxHealth/6.0f )
                    {
                        guid newCover = FindNearestValidCover( COVER_PREF_FURTHER_FROM_TARGET );
                        if( newCover )
                        {
                            SetCurrentCover( newCover );
                            if( GetAwarenessLevel() >= AWARENESS_TARGET_SPOTTED )
                            {                     
                                if( RequestAutofire() )
                                {                                
                                    PlayDialog( DIALOG_COVER_REQ );
                                }
                            }
                        }
                    }
                }
            }
        }
        else if ( GetAwarenessLevel() == AWARENESS_COMBAT_READY )
        {
            if( !GetCurrentCover() )
            {
                SetCurrentCover( FindNearestValidCover(COVER_PREF_CLOSER_TO_US) );
            }
        }
        else if ( !stickyCoverObject )
        {
            // if we aren't fighting the guys and we aren't in combat ready stance clear cover.
            object *coverObject = g_ObjMgr.GetObjectByGuid( GetCurrentCover() );
            if( coverObject && coverObject->IsKindOf(cover_node::GetRTTI()) )
            {
                cover_node &coverNode = cover_node::GetSafeType( *coverObject );
                coverNode.InvalidateNode();
            }
            SetCurrentCover( 0 );
        }
    }
    m_CoverIsValid = IsCoverNodeValid( GetCurrentCover() );   

    // in a very special circumstance we may actually lose our sticky cover.
    // I changed from cansee to pure awareness, I don't think LOS should determine 
    // if they come out, just if they are aware of the target.
    if( stickyCoverObject &&
        m_bCoverHopper &&
        GetIsInCover() &&
        coverState->GetCoverNode() == GetCurrentCover() &&
        GetAwarenessLevel() >= AWARENESS_ACQUIRING_TARGET &&
        !m_CoverIsValid )
    {
        SetStickCoverNode(0);
        object *coverObject = g_ObjMgr.GetObjectByGuid(lastCover);
        if( coverObject && coverObject->IsKindOf(cover_node::GetRTTI()) )
        {
            cover_node &coverNode = cover_node::GetSafeType(*coverObject);
            SetStickCoverNode(coverNode.GetNextStickyNode());
        }
        SetCurrentCover(0);
    }

    // reserve our current cover.
    object *coverNodeObject = g_ObjMgr.GetObjectByGuid( GetCurrentCover() );
    if( coverNodeObject && coverNodeObject->IsKindOf(cover_node::GetRTTI()) )
    {
        cover_node &coverNode = cover_node::GetSafeType( *coverNodeObject );        
        coverNode.ReserveNode( GetGuid() );   
        // update our anim group if we have a new cover.
        if( lastCover != GetCurrentCover() )
        {
            const char* coverAnimGroupName = coverNode.GetAnimGroupName( GetType(), GetLogicalName() );
            if( coverAnimGroupName == NULL )
            {
                ASSERT(FALSE);
                return;
            }
//            m_CoverAnimGroupHandle.SetName( coverAnimGroupName );
            m_CoverChanged = TRUE;
        }
    }
}


//=============================================================================

anim_group::handle character::GetCoverAnimGroupHandle( void )
{
    if( HasState(character_state::STATE_COVER) )
    {
        character_cover_state* coverState = (character_cover_state*)GetStateByType(character_state::STATE_COVER);
        return coverState->GetCoverAnimGroupHandle();
    }
    else
    {        
        anim_group::handle emptyHandle;
        return emptyHandle;
    }
}

//=============================================================================

void character::UpdateAlarmNode()
{
    // we only update if we can do something with the alarm...
    object *targetObject = g_ObjMgr.GetObjectByGuid( GetTargetGuid() );
    if( !HasState(character_state::STATE_ALARM) || 
        !targetObject ||
        !m_bCanUseAlarms )
    {
        m_CurrentAlarm = 0;
        return;
    }

    // we are lacking an alarm
    object *currentAlarmObject = g_ObjMgr.GetObjectByGuid( m_CurrentAlarm );
    if(!currentAlarmObject )
    {
        m_CurrentAlarm = FindNearestAlarm();
    }

    // let's check the validity of our current alarm
    currentAlarmObject = g_ObjMgr.GetObjectByGuid( m_CurrentAlarm );
    if( currentAlarmObject && currentAlarmObject->IsKindOf(alarm_node::GetRTTI()) )
    {
        alarm_node &alarmNode = alarm_node::GetSafeType(*currentAlarmObject);
        if( !IsAlarmNodeValid(currentAlarmObject->GetGuid()) )
        {
            m_CurrentAlarm = 0;
        }
        else
        {
            alarmNode.ReserveNode(GetGuid(),TRUE);
        }
    }
    else
    {
        m_CurrentAlarm = 0;
    }
}

//=============================================================================

void character::UpdateInNavMap()
{
    // If not using nav map, just set to TRUE to make rest of logic work
    if( m_PathingHints.bUseNavMap == FALSE )
    {
        m_InNavMap = TRUE;
        m_TimeOutofNavMap = 0.0f;
        return;
    }
    
    if (m_CurrentConnectionSlot != NULL_NAV_SLOT)
    {
        ng_connection2&     Conn = g_NavMap.GetConnectionByID( m_CurrentConnectionSlot );
        if( Conn.IsPointInConnection(GetPosition()) )
        {
            m_InNavMap = TRUE;
            m_TimeOutofNavMap = 0.0f;
            return;
        }
    }
    
    m_InNavMap = g_NavMap.IsPointInMap( GetPosition() );
    if( m_InNavMap )
    {
        m_TimeOutofNavMap = 0.0f;
    }
}

//=============================================================================

// Calls state think
void character::OnThink( void )
{
    if( !g_RunCharacterLogic || !m_bDoRunLogic )
    {
        return;
    }

    m_bThinking = TRUE;


    if( !GetCurrentWeaponPtr() )   
    {
        SelectWeapon();
    }

    // dumb and fast fights poorly.
    if( !m_bDumbAndFast )
    {    
        UpdateAlarmNode();
        UpdateHasClearLOS();
        UpdateHasAllies();
        UpdateHasClearJumpAttack();
    //  UpdatePlayerLOF();
    }
    UpdateCoverNode();

    nav_connection_slot_id connectionSlot;

    // check to see if we can reach our target
    
    UpdateInNavMap();
    UpdateEscape();
    
    // if there is no target / point of interest, then GetTargetPosWithOffset will return a null vector.  '
    // if this happens, don't call UpdateCanReachTarget, since we know we don't have one.
    vector3 TargetPosWithOffset = GetTargetPosWithOffset();
    if ( TargetPosWithOffset.LengthSquared() < k_VerySmallNumber )
        m_CanReachTarget = false;
    else
        m_CanReachTarget = UpdateCanReachTarget( GetTargetPosWithOffset(), GetShortMeleeRange() );

    if( m_CanReachTarget != m_CanPathToTarget &&
        m_TimeSinceLastPathToTargetCheck >= k_MinTimeSinceLastPathToTargetCheck )
    {
        object* targetObject = g_ObjMgr.GetObjectByGuid( GetTargetGuid() );
        if( targetObject )
        {
            m_CanPathToTarget = CanPathTo( targetObject->GetPosition() );
            m_TimeSinceLastPathToTargetCheck = 0.0f;
        }
    }

    m_CanMoveForward    = UpdateCanMoveForward();
    m_CanMoveBack       = UpdateCanMoveBack();
    m_CanMoveLeft       = UpdateCanMoveLeft();
    m_CanMoveRight      = UpdateCanMoveRight();

    switch( m_GoalInfo.m_GoalType ) 
    {
    case GOAL_RETREAT_FROM_TARGET:
        if (m_RethinkRetreatTimer > k_RethinkRetreat)
        {
            //CalculateValidRetreatDestination();
            m_GoalRetreatToConnectionSlot = GetNewRetreatPath( GetTargetPosWithOffset(m_GoalInfo.m_TargetGuid), m_GoalInfo.m_DesiredDist );
            m_RethinkRetreatTimer = 0;
        }
    case GOAL_GOTO_TARGET:
        m_CanReachGoalTarget = UpdateCanReachTarget( GetGoalTrueLocation(), 50.0f );

        vector3 Pt = GetGoalTrueLocation();
        if (!g_NavMap.GetConnectionContainingPoint( connectionSlot, Pt))
        {
            connectionSlot = g_NavMap.GetNearestConnection( Pt );
        }
                
        // doing this on a retreat when only one guy caused him to keep 
        // recalcing and thus running in circles.
        if( connectionSlot != m_GoalsConnectionSlot &&
            m_GoalInfo.m_GoalType == GOAL_GOTO_TARGET )
        {
            #ifdef AI_LOGGING
            LOG_MESSAGE( "character::OnThink", "Target has changed connections.  Old [%d] New[%d] activating repath request", m_GoalsConnectionSlot, connectionSlot );
            #endif
            m_NeedsToRecalcPath = TRUE;
            m_GoalsConnectionSlot = connectionSlot;
        }
        break;
    }

    // Call states think
    if( m_pActiveState && !m_bDumbAndFast )
    {    
        if( !m_pActiveState->IgnoreSight() )
        {        
            // this updates our can see and can shoot
            UpdateTarget();
        }
        if( !m_pActiveState->IgnoreAlerts() )
        {        
            ListenForAlerts();    
        }
        if( !m_pActiveState->IgnoreSound() )
        {        
            m_SoundHeard = ListenForSounds();    
        }
        m_pActiveState->OnThink();
    }

    // added so we rethink our pathing when out of the nav path.
    if( !GetInNavMap() )
    {
        m_NeedsToRecalcPath = TRUE;
    }

    m_bThinking = FALSE;
}

//=============================================================================

void character::OnPain( const pain& Pain )
{
    // If you are already dead, no pain!
    if( m_bDead )
        return;

    // Give our state a chance to ignore the pain.
    if( m_pActiveState && (m_pActiveState->OnPain(Pain)==FALSE) )
        return;

    // If the same pain event as the last one, ignore it.
    if( (Pain.GetAnimEventID() != -1) && 
        (Pain.GetAnimEventID() == m_LastAnimPainID) )
    {    
        return;
    }

    // if we are neutral, we ignore pain
    if ( m_SpawnNeutralTime > 0.0f )
    {
        return;
    }


    // Decide which health id to use
    health_handle HealthHandle;
    {
        switch( GetHitLocation( Pain ))
        {   
        case geom::bone::HIT_LOCATION_HEAD:
            HealthHandle.SetName(xfs("%s_H",GetLogicalName()));
            break;
        case geom::bone::HIT_LOCATION_LEGS:
            HealthHandle.SetName(xfs("%s_L",GetLogicalName()));
            break;
        default: // includes TORSO, ARMS
            HealthHandle.SetName(xfs("%s_B",GetLogicalName()));
            break;
        };
    }

    // Resolve Pain
    if( !Pain.ComputeDamageAndForce( HealthHandle, GetGuid(), GetColBBox().GetCenter() ) )
        return;

    // Setup defaults.
    xbool bApplyDamage      = TRUE;
    xbool bApplyEffects     = TRUE;
    xbool bDoLargeEffects   = TRUE;
    xbool bDoDebris         = TRUE;
    xbool bDoFlinches       = TRUE;

    object *pPainSource = g_ObjMgr.GetObjectByGuid( Pain.GetOriginGuid() );
    factions sourceFaction = GetFactionForGuid(Pain.GetOriginGuid());

    // do we apply damage,effects,etc...
    if( IgnorePain( Pain ) )
    {
        bApplyDamage = FALSE;
        if( Pain.IsFriendlyFire() )
        {
            bApplyEffects = FALSE;
            if( pPainSource->IsKindOf( character::GetRTTI()) )
            {                    
                bDoFlinches     = FALSE;            
            }
        }
    }
    else
    {
        if( Pain.GetOriginGuid() != GetGuid() )
        {
            if( pPainSource )
            {                    
                if( sourceFaction != FACTION_NONE &&
                    sourceFaction != INVALID_FACTION &&
                    sourceFaction != FACTION_NOT_SET &&
                    !IsEnemyFaction( sourceFaction ) )
                {                
                    bApplyDamage    = FALSE;
                    bApplyEffects = FALSE;
                    if( pPainSource->IsKindOf( character::GetRTTI()) )
                    {                    
                        bDoFlinches     = FALSE;            
                    }
                }
            }
        }
    }

    // Apply the damage
    if( bApplyDamage && m_bCanDie)
    {
        // if we are cloaked, damage our cloak shield
        if( m_CloakState == CLOAKING_ON )
        {
            m_CloakShieldPainTimer = 1.0f;

            // attach a pain effect to the bone that was hit
            if( Pain.HasCollision() )
            {
                const collision_mgr::collision& Col = Pain.GetCollision();
                s32 iBone = Col.PrimitiveKey & 0xFF;

                if (Col.PrimitiveKey == -1)
                    iBone = 0;

                actor_effects* pEffects = GetActorEffects( TRUE );
                pEffects->InitEffect( actor_effects::FX_CLOAK_PAIN, this, iBone );
            }
        }

        // Apply Damage?
        {
            // Update health.
            TakeDamage(Pain);
        }

        // Record this as the last pain eventID
        if( Pain.GetAnimEventID() >= 0 )
        {
            m_LastAnimPainID = Pain.GetAnimEventID();
        }
    }

    // Play flinch.  
    if( bDoFlinches && !m_bIgnoreFlinches )
    {
        PlayFlinch( Pain );

        // if we flinch, then let's talk about it.
        if( pPainSource && 
            sourceFaction != FACTION_NONE &&
            sourceFaction != INVALID_FACTION &&
            sourceFaction != FACTION_NOT_SET &&
            !IsEnemyFaction(sourceFaction) )
        {
            OnHitByFriendly( Pain.GetOriginGuid() );
            if( pPainSource->IsKindOf( actor::GetRTTI() ) )
            {            
                actor& pActorSource = actor::GetSafeType( *pPainSource );
                pActorSource.OnHitFriendly( GetGuid() );
            }
        }
        else
        {
            if( Pain.GetHitType() == 4 )
            {
                PlayDialog( DIALOG_HIT_MELEE );
            }
            else
            {
                PlayDialog( DIALOG_HIT );
            }
        }
    }

    // Create all the effects.
    if( bApplyEffects )
        CreateDamageEffects( Pain, bDoLargeEffects, bDoDebris );
}

//=============================================================================

void character::UpdateAwarenessLevel()
{
    switch( GetAwarenessLevel() )
    {
    case AWARENESS_NONE:
        // else if I see or hear someone, become alert...
        if( IsCombatReady() &&
            m_pActiveState && 
            m_pActiveState->GetStateType() != character_state::STATE_TRIGGER )
        {
            SetAwarenessLevel( AWARENESS_COMBAT_READY );
        }
        else if( (GetTargetGuid() ||
                 m_TimeSinceLastSound == 0.0f) &&
                 !GetRootWhenIdle() )
        {
            SetAwarenessLevel( AWARENESS_ALERT );
        }
        break;
    case AWARENESS_ALERT:
        if( GetTargetGuid() )
        {
            if( !m_bTargetSeen )
            {
                SetAwarenessLevel(AWARENESS_ACQUIRING_TARGET );
            }
            else if( GetTargetNotSeenTimer() >= k_MinTimeTillTargetLost )
            {
                SetAwarenessLevel( AWARENESS_TARGET_LOST );
            }
            else
            {
                SetAwarenessLevel( AWARENESS_TARGET_SPOTTED );
            }
        }
        else if ( m_TimeSinceLastSound == 0.0f &&
                  m_TimeAtAwarenessLevel >= k_MinTimeAlert )
        {
            SetAwarenessLevel( AWARENESS_SEARCHING );
        }
        else if ( !m_TargetGuid &&
                  m_TimeSinceLastSound != 0.0f &&
                  m_TimeAtAwarenessLevel >= k_MaxTimeAlert )
        {
            SetAwarenessLevel( AWARENESS_NONE );
        }
        break;
    case AWARENESS_SEARCHING:
        if( GetTargetGuid() )
        {
            if( !m_bTargetSeen )
            {
                SetAwarenessLevel(AWARENESS_ACQUIRING_TARGET );
            }
            else if( GetTargetNotSeenTimer() >= k_MinTimeTillTargetLost )
            {
                SetAwarenessLevel( AWARENESS_TARGET_LOST );
            }
            else
            {
                SetAwarenessLevel( AWARENESS_TARGET_SPOTTED );
            }
        }
        else if ( m_TimeAtAwarenessLevel >= k_MinSearchTime )
        {
            SetAwarenessLevel( AWARENESS_NONE );
        }
        break;
    case AWARENESS_COMBAT_READY:
        if( GetTargetGuid() )
        {
            if( !m_bTargetSeen )
            {
                SetAwarenessLevel(AWARENESS_ACQUIRING_TARGET );
            }
            else if( GetTargetNotSeenTimer() >= k_MinTimeTillTargetLost )
            {
                SetAwarenessLevel( AWARENESS_TARGET_LOST );
            }
            else
            {
                SetAwarenessLevel( AWARENESS_TARGET_SPOTTED );
            }
        }
        else if( !IsCombatReady() )
        {
            SetAwarenessLevel( AWARENESS_NONE );
        }
        break;
    case AWARENESS_TARGET_SPOTTED:
        if( !m_bTargetSeen )
        {
            SetAwarenessLevel(AWARENESS_ACQUIRING_TARGET );
        }
        else if( GetTargetNotSeenTimer() >= k_MinTimeTillTargetLost )
        {
            SetAwarenessLevel( AWARENESS_TARGET_LOST );
        }
        else if ( !GetTargetGuid() )
        {
            SetAwarenessLevel( AWARENESS_COMBAT_READY );
        }
        break;
    case AWARENESS_TARGET_LOST:
        if( !m_bTargetSeen )
        {
            SetAwarenessLevel(AWARENESS_ACQUIRING_TARGET );
        }
        else if( GetTargetNotSeenTimer() < k_MinTimeTillTargetLost )
        {
            SetAwarenessLevel( AWARENESS_TARGET_SPOTTED );
        }
        else if ( !GetTargetGuid() )
        {
            SetAwarenessLevel( AWARENESS_COMBAT_READY );
        }
        break;
    case AWARENESS_ACQUIRING_TARGET:
        if( m_bTargetSeen )
        {
            if( GetTargetNotSeenTimer() < k_MinTimeTillTargetLost )
            {
                SetAwarenessLevel( AWARENESS_TARGET_SPOTTED );
            }
            else
            {
                // this really should never occur....
                SetAwarenessLevel( AWARENESS_TARGET_LOST);
            }
        }
        else if ( !GetTargetGuid() )
        {
            SetAwarenessLevel( AWARENESS_COMBAT_READY );
        }
        break;
    }
    
    // special stay in idle flag
    if( GetAwarenessLevel() != AWARENESS_NONE &&
        GetRootWhenIdle() )
    {
        SetAwarenessLevel(AWARENESS_NONE);
    }

    // If playing a full body lip sync, then make sure awareness is set to none
    // so npcs (eg. victor) do not go to flee state etc.
    if( IsPlayingFullBodyLipSync() )
    {
        // Put AI into idle
        SetAwarenessLevel(AWARENESS_NONE);
    }
        
    // once we've seen combat, we are going to hang in cover
    if( GetAwarenessLevel() >= AWARENESS_ACQUIRING_TARGET &&
        !IsCombatReady() )
    {
        SetCombatReady(TRUE);
    }
}

//=============================================================================
void character::Cloak()
{
    actor::Cloak();
    m_CloakDecloakTimer = 0.0f;
}

void character::Decloak()
{
    actor::Decloak();
    m_CloakDecloakTimer = 0.0f;
}

void character::UpdateCloak( f32 DeltaTime )
{
    // are we allowed to cloak?
    if( !m_CanCloak )
    {
        Decloak();
        return;
    }

    // are we forced to be cloaked all the time?
    if( m_MustCloak )
    {
        Cloak();
        return;
    }

    // let the actor do his thing
    actor::UpdateCloak( DeltaTime );

    if( GetAwarenessLevel() >= AWARENESS_ACQUIRING_TARGET )
    {

        if( GetParametricHealth() > 0.15f )
        {        
            Cloak();
        }
        else
        {
            Decloak();
        }
    }
    else
    {
        Decloak();
    }
}


//=============================================================================

character_state::states character::GetStateFromAwareness( void )
{
    if( GetCurrentCover() &&
        GetStickyCoverNode() &&
        HasState(character_state::STATE_COVER) )
    {
        return character_state::STATE_COVER;
    }

    switch( GetAwarenessLevel() )
    {
    case AWARENESS_TARGET_SPOTTED:
    case AWARENESS_TARGET_LOST:
    case AWARENESS_ACQUIRING_TARGET:
        {
            if( HasState(character_state::STATE_ATTACK) )
            {
                return character_state::STATE_ATTACK;
            }
            else if( HasState(character_state::STATE_FLEE) )
            {
                return character_state::STATE_FLEE;
            }
        }
    case AWARENESS_SEARCHING:
        {
            if( HasState(character_state::STATE_SEARCH) )
            {
                return character_state::STATE_SEARCH;
            }
        }
    case AWARENESS_ALERT:
        {        
            if( HasState(character_state::STATE_ALERT) )
            {
                return character_state::STATE_ALERT;
            }
        }
    case AWARENESS_NONE:
        {        
            return character_state::STATE_IDLE;
        }
        break;
    case AWARENESS_COMBAT_READY:
        if( GetCurrentCover() &&            
            HasState(character_state::STATE_COVER) )
        {
            return character_state::STATE_COVER;
        }
        else
        {        
            return character_state::STATE_IDLE;
        }
        break;
    }
    return character_state::STATE_NULL;
}

//=============================================================================

void character::SetAwarenessLevel( eAwarenessLevel newAwarenessLevel )
{
    if( newAwarenessLevel != m_AwarenessLevel )
    {    
        if( newAwarenessLevel <= AWARENESS_COMBAT_READY &&
            m_AwarenessLevel >= AWARENESS_TARGET_LOST )
        {
            m_bPlayAllClearDialog = TRUE;
        }
        else if ( newAwarenessLevel >= AWARENESS_ALERT )
        {
            m_bPlayAllClearDialog = FALSE;
            if( m_AwarenessLevel <= AWARENESS_COMBAT_READY )
            {
                PlayDialog( DIALOG_ALERT );
            }
        }
        m_AwarenessLevel = newAwarenessLevel;
        m_TimeAtAwarenessLevel = 0.0f;
    }
}

//=============================================================================

void character::UpdateHasAllies()
{
    m_HasAllies = FALSE;
    s32 count;
    for(count=0;count<k_MaxAllies;count++)
    {
        m_AllyGuids[count]=0;
    }

    priority_queue<object *,f32,k_MaxAllies> allyQueue;
    actor* pActor = actor::m_pFirstActive;
    s32 contactCount = 0;
    s32 actorCount = 0;
    while( contactCount < k_MaxAllies && actorCount < actor::m_nActive && pActor )
    {
        //if valid and not myself
        if( pActor && 
            pActor->GetGuid() != GetGuid() && 
            pActor->GetFaction() == GetFaction() &&
            GetToTarget(pActor->GetGuid()).LengthSquared() <= 2000.0f * 2000.0f )
        {
            allyQueue.Push( pActor,GetToTarget(pActor->GetGuid()).LengthSquared());
            contactCount++;
            m_HasAllies = TRUE;
        }
        actorCount++;
        pActor = pActor->m_pNextActive;
    }

    if( m_HasAllies )
    {
        count = 0;
        while( !allyQueue.IsEmpty() &&
               count < k_MaxAllies )
        {
            m_AllyGuids[count] = allyQueue.Pop()->GetGuid();
            count++;
        }
    }
}

//=============================================================================

f32 character::GetTimeSinceLastCombat()
{
    // here we are going to make our best guess given our available data at how
    // long it has been since we were last in combat.
    
    // easiest test is time since we last had a target, so let's try that first.
    return m_NoTargetTimer;
}

void character::SetPostTriggerTarget( guid target )
{
    m_TriggerState.SetPostTriggerTarget(target);
}

//=============================================================================

void character::SetTriggerStateData( character_trigger_state::TriggerData pData )
{
    m_TriggerStateData = pData;
}

//=============================================================================

void character::SetTriggerStateData( action_ai_base* pContext )
{
    if ( pContext )
    {        
        if ( pContext->IsKindOf( action_ai_base::GetRTTI() ) )
        {
            m_TriggerStateData.m_ActionType = pContext->GetAIActionType();
            m_TriggerStateData.m_MustSucceed = pContext->GetMustSucceed();
            m_TriggerStateData.m_ResponseList.SetFlags(pContext->GetResponseFlags());
            m_TriggerStateData.m_TriggerGuid = pContext->GetTriggerGuid();
            m_TriggerStateData.m_Blocking = pContext->GetBlockUntilComplete();

            switch(m_TriggerStateData.m_ActionType)
            {
            case action_ai_base::AI_ATTACK_GUID :
                if ( pContext->IsKindOf( action_ai_attack_guid::GetRTTI() ) )
                {
                    m_TriggerStateData.m_ActionFocus    = ((action_ai_attack_guid*)pContext)->GetTargetGuid();
                    m_TriggerStateData.m_NextAiState    = character_state::STATE_ATTACK;
                }
                else
                {
                    ASSERT(FALSE);
                }
                break;
            case action_ai_base::AI_PATHFIND_TO_GUID:
                if ( pContext->IsKindOf( action_ai_pathto_guid::GetRTTI() ) )
                {
                    m_TriggerStateData.m_ActionFocus  = ((action_ai_pathto_guid*)pContext)->GetPathToGuid();
                    m_TriggerStateData.m_NextAiState  = ((action_ai_pathto_guid*)pContext)->GetNextState();                
                    if( ((action_ai_pathto_guid*)pContext)->GetGotoExact() )
                    {
                        m_TriggerStateData.m_UnionData.m_PathfindData.m_Distance = -1.0f;
                    }
                    else
                    {
                        m_TriggerStateData.m_UnionData.m_PathfindData.m_Distance = ((action_ai_pathto_guid*)pContext)->GetDistance();
                    }
                    m_TriggerStateData.m_UnionData.m_PathfindData.m_MoveStyle    = ((action_ai_pathto_guid*)pContext)->GetMoveStyle();
                    m_TriggerStateData.m_UnionData.m_PathfindData.m_Retreating   = ((action_ai_pathto_guid*)pContext)->GetRetreating();
                    if( ((action_ai_pathto_guid*)pContext)->GetAlignExact() )
                    {
                        m_TriggerStateData.m_UnionData.m_PathfindData.m_YawDifference = -1.0f;
                    }
                    else
                    {
                        m_TriggerStateData.m_UnionData.m_PathfindData.m_YawDifference = ((action_ai_pathto_guid*)pContext)->GetYawThreshold();
                    }
                }
                else
                {
                    ASSERT(FALSE);
                }
                break;
            case action_ai_base::AI_LOOK_AT_GUID:
                if ( pContext->IsKindOf( action_ai_lookat_guid::GetRTTI() ) )
                {
                    m_TriggerStateData.m_ActionFocus    = ((action_ai_lookat_guid*)pContext)->GetLookAtGuid();
                    m_TriggerStateData.m_NextAiState    = ((action_ai_lookat_guid*)pContext)->GetNextState();                
                    m_TriggerStateData.m_UnionData.m_LookatData.m_LookatHead     = ((action_ai_lookat_guid*)pContext)->GetHeadLookAt();
                    m_TriggerStateData.m_UnionData.m_LookatData.m_LookatDistance = ((action_ai_lookat_guid*)pContext)->GetLookAtDist();
                    m_TriggerStateData.m_UnionData.m_LookatData.m_LookatFOV      = ((action_ai_lookat_guid*)pContext)->GetLookAtFOV();
                }
                else
                {
                    ASSERT(FALSE);
                }
                break;
            case action_ai_base::AI_DIALOG_LINE:
                if ( pContext->IsKindOf( action_ai_dialog_line::GetRTTI() ) )
                {
                    m_TriggerStateData.m_NextAiState    = ((action_ai_dialog_line*)pContext)->GetNextState();
                    x_strcpy( m_TriggerStateData.m_UnionData.m_DialogData.m_AnimGroupName, (((action_ai_dialog_line*)pContext)->GetAnimGroupName()) );
                    x_strcpy( m_TriggerStateData.m_UnionData.m_DialogData.m_AnimName, (((action_ai_dialog_line*)pContext)->GetAnimName()) );
                    x_strcpy( m_TriggerStateData.m_UnionData.m_DialogData.m_SoundName, (((action_ai_dialog_line*)pContext)->GetSoundName()) );
                    m_TriggerStateData.m_UnionData.m_DialogData.m_AnimFlags      = ((action_ai_dialog_line*)pContext)->GetAnimFlags();
                    m_TriggerStateData.m_UnionData.m_DialogData.m_SoundFlags     = ((action_ai_dialog_line*)pContext)->GetSoundFlags();
                    m_TriggerStateData.m_UnionData.m_DialogData.m_BlockOnDialog  = ((action_ai_dialog_line*)pContext)->GetBlockOnDialog();
                    m_TriggerStateData.m_UnionData.m_DialogData.m_KillAnim       = ((action_ai_dialog_line*)pContext)->GetKillAnim();
                    m_TriggerStateData.m_UnionData.m_DialogData.m_AnimBlendTime  = ((action_ai_dialog_line*)pContext)->GetBlendTime();
                    //Init Pre Delay  --bjt
                    (((action_ai_dialog_line*)pContext)->InitPreDelay());
                    (((action_ai_dialog_line*)pContext)->InitPostDelay());
                }
                else
                {
                    ASSERT(FALSE);
                }
                break;
            case action_ai_base::AI_PLAY_ANIMATION:
                if ( pContext->IsKindOf( action_ai_play_anim::GetRTTI() ) )
                {
                    action_ai_play_anim* pPlayAnim = (action_ai_play_anim*)pContext;
                
                    m_TriggerStateData.m_NextAiState  = pPlayAnim->GetNextState();
                    x_strcpy( m_TriggerStateData.m_UnionData.m_PlayAnimData.m_AnimGroupName, pPlayAnim->GetAnimGroup() );
                    x_strcpy( m_TriggerStateData.m_UnionData.m_PlayAnimData.m_AnimName, pPlayAnim->GetAnimName() );
                    m_TriggerStateData.m_UnionData.m_PlayAnimData.m_AnimPlayTime = pPlayAnim->GetAnimPlayTime() ;
                    m_TriggerStateData.m_UnionData.m_PlayAnimData.m_AnimFlags    = pPlayAnim->GetAnimFlags() ;
                    m_TriggerStateData.m_UnionData.m_PlayAnimData.m_AnimBlendTime = pPlayAnim->GetAnimBlendTime();
                    m_TriggerStateData.m_ActionFocus = pPlayAnim->GetScaledTargetGuid() ;
                }
                else
                {
                    ASSERT(FALSE);
                }
                break;
            case action_ai_base::AI_SEARCHTO_GUID :
                if ( pContext->IsKindOf( action_ai_searchto_guid::GetRTTI() ) )
                {
                    m_TriggerStateData.m_ActionFocus    = ((action_ai_searchto_guid*)pContext)->GetTargetGuid();
                    m_TriggerStateData.m_NextAiState    = character_state::STATE_SEARCH;
                }
                else
                {
                    ASSERT(FALSE);
                }
                break;
                
            case action_ai_base::AI_DEATH:
                if ( pContext->IsKindOf( action_ai_death::GetRTTI() ) )
                {
                    action_ai_death* pAction = (action_ai_death*)pContext;
                    m_TriggerStateData.m_NextAiState  = pAction->GetNextState();
                    m_TriggerStateData.m_UnionData.m_DeathData.m_DeathType = pAction->GetDeathType();
                    x_strcpy( m_TriggerStateData.m_UnionData.m_DeathData.m_AnimGroupName, pAction->GetAnimGroup() );
                    x_strcpy( m_TriggerStateData.m_UnionData.m_DeathData.m_AnimName, pAction->GetAnimName() );
                    m_TriggerStateData.m_UnionData.m_DeathData.m_RagdollForceAmount = pAction->GetRagdollForceAmount();
                    m_TriggerStateData.m_UnionData.m_DeathData.m_RagdollForceRadius = pAction->GetRagdollForceRadius();
                    m_TriggerStateData.m_TriggerGuid = pAction->GetRagdollForceLocation();
                }                    
                else
                {
                    ASSERT(FALSE);
                }
                break;
                
            default:
                ASSERT(FALSE);
                break;
            }
            return;
        }
    }
    ASSERT(FALSE);
}

//=============================================================================

xbool character::TriggerState( character_trigger_state::TriggerData pData )
{
    // Skip if dead
    if( IsDead() )
        return FALSE;

#ifdef X_EDITOR
    if( GetLogStateChanges() )
    {        
        object *triggerObject = g_ObjMgr.GetObjectByGuid( pData.m_TriggerGuid );
        if( triggerObject && triggerObject->IsKindOf(trigger_ex_object::GetRTTI()) )
        {    
            trigger_ex_object &triggerEx = trigger_ex_object::GetSafeType( *triggerObject );
            LOG_MESSAGE( "Character::TriggerData",
                "Trigger: Guid%08X:%08X Name:%s Description:%s Must Succeed:%d Blocking:%d",
                triggerEx.GetGuid().GetHigh(),
                triggerEx.GetGuid().GetLow(),
                triggerEx.GetName(),
                triggerEx.GetTriggerDescription(),
                pData.m_MustSucceed,
                pData.m_Blocking );
        }
    }
#endif

    character_trigger_state *triggerState = (character_trigger_state*)GetStateByType(character_state::STATE_TRIGGER);
    if( triggerState )
    {
        SetupState( character_state::STATE_TRIGGER );
        SetTriggerStateData( pData );

        // special case: If we are playing a full bosy lip sync, 
        // we want to stop in order to process the trigger.
        if( IsPlayingFullBodyLipSync() )
        {
            loco* pLoco = GetLocoPointer();
            if( pLoco )
            {
                // Is full body lip sync anim playing?
                loco_lip_sync_controller& LipSyncCont = pLoco->GetLipSyncController();
                LipSyncCont.Stop();
            }
        }
    }
    return TRUE;
}

//=============================================================================

// Called by a link being triggered
xbool character::TriggerState( action_ai_base* pAction )
{
    // Skip if dead
    if( IsDead() || GetActiveState() == character_state::STATE_DEATH )
        return FALSE;
        
    // If in hold state and requesting to die, make active so that the ai gets a
    // tick to change to trigger state ready to kill the npc
    if(     ( GetActiveState() == character_state::STATE_HOLD )
        &&  ( pAction->GetAIActionType() == action_ai_base::AI_DEATH ) )
    {
        OnActivate( TRUE );
    }
            
    // there is one special case here... dialog that doesn't interrupt the AI
    if( pAction && pAction->GetAIActionType() == action_ai_base::AI_DIALOG_LINE )
    {
        action_ai_dialog_line *dialogAction = (action_ai_dialog_line*)pAction;
        if( !dialogAction->GetInterrupAI() )
        {
            if( dialogAction->GetBlockUntilComplete() )
            {      
                if( pAction->GetTriggerGuid() )
                {
                    m_pNoBlockDialogTrigger = pAction->GetTriggerGuid();
                }
                else
                {
                    m_pNoBlockDialogTrigger = m_PendingTaskListGuid;
                }
            }
            PlayDialog(DIALOG_TRIGGER_NO_INTERRUPT,
                       dialogAction->GetSoundName(),
                       FALSE,
                       dialogAction->GetAnimName(),
                       dialogAction->GetAnimGroupName(),
                       dialogAction->GetAnimFlags(),
                       dialogAction->GetSoundFlags(),
                       dialogAction->GetBlendTime());

            return TRUE;
        }
    }

    // If setting the look at without blocking, apply goal right now so designers
    // don't have to put a block 1 frame command (which can cause a 1 frame wait pop)
    if( pAction && 
        !pAction->GetBlockUntilComplete() && 
        pAction->GetAIActionType() == action_ai_base::AI_LOOK_AT_GUID )
    {
        action_ai_lookat_guid* pLookatAction = (action_ai_lookat_guid*)pAction;

        // if we are a head lookat, set that up.
        if( pLookatAction->GetHeadLookAt() )
        {
            SetHeadLookat( pLookatAction->GetLookAtGuid(), -1.0f, pLookatAction->GetLookAtDist(), pLookatAction->GetLookAtFOV() );
        }
        else
        {
            SetOverrideLookatInterest( pLookatAction->GetLookAtGuid() );
            UpdateLookAt( );
        }
        
        // if task list we have to it we are done though.
        object* pObject = g_ObjMgr.GetObjectByGuid( m_PendingTaskListGuid );
        if( pObject && pObject->IsKindOf(character_task_set::GetRTTI()) )
        {
            character_task_set &taskSet = character_task_set::GetSafeType( *pObject );
            taskSet.OnTaskItemComplete();
        }
        return TRUE;
    }

    // Playing an anim without interrupting AI?
    if( pAction && !pAction->GetBlockUntilComplete() && pAction->GetAIActionType() == action_ai_base::AI_PLAY_ANIMATION )
    {       
        // Lookup info
        action_ai_play_anim* pPlayAnim = (action_ai_play_anim*)pAction;
        if( !pPlayAnim->GetInterrupAI() )
        {        
            // Play the anim
            GetLocoPointer()->PlayAnim( pPlayAnim->GetAnimGroup(),
                                        pPlayAnim->GetAnimName(),
                                        pPlayAnim->GetAnimBlendTime(),
                                        pPlayAnim->GetAnimFlags(),
                                        pPlayAnim->GetAnimPlayTime() );
            return TRUE;
        }
    }

    // Put into trigger state
    character_trigger_state *triggerState = (character_trigger_state*)GetStateByType(character_state::STATE_TRIGGER);
    if( triggerState )
    {
        SetupState( character_state::STATE_TRIGGER );
        SetTriggerStateData( pAction );
        // special case: If we are playing a full bosy lip sync, 
        // we want to stop in order to process the trigger.
        if( IsPlayingFullBodyLipSync() )
        {
            loco* pLoco = GetLocoPointer();
            if( pLoco )
            {
                // Is full body lip sync anim playing?
                loco_lip_sync_controller& LipSyncCont = pLoco->GetLipSyncController();
                LipSyncCont.Stop();
            }
        }
    }
    return TRUE;
}

//===============================================================================

void character::KillMe ( void )
{
    m_bDead = TRUE;

    // Notify the object that we need to tell.
    if ( m_NotifyOnDeath != 0 )
    {
        object_ptr<object> pNotifyObject( m_NotifyOnDeath );
        if ( pNotifyObject.IsValid() )
        {
            // If we need to notify a spawner, do so.
            if ( pNotifyObject.m_pObject->GetType() == TYPE_SPAWNER_OBJECT )
            {
                spawner_object* pSpawner = ( spawner_object* ) pNotifyObject.m_pObject;
                pSpawner->OnSpawnedObjectKill( this );
            }
        }

        // We've notified the appropriate object.
        // ` the guid so we don't do it again.
        m_NotifyOnDeath = 0;
    }
}

//=============================================================================

void character::DropWeapon()
{
    // Should npc drop inventory?
    m_WeaponItem = INVEN_NULL;
    if( m_CurrentWeaponItem )
    {
        m_CurrentWeaponItem = INVEN_NULL; 
        for( s32 i=INVEN_WEAPON_FIRST ; i<INVEN_WEAPON_LAST ; i++ )
        {
            inven_item Item = (inven_item)i;
            if( m_Inventory2.HasItem( Item ) )
            {
                random r( x_rand() );
                vector3 Velocity = r.v3( -100.0f, 100.0f, 40.0f, 100.0f, -100.0f, 100.0f );
                pickup::CreatePickup( GetGuid(), net_GetSlot(), Item, m_Inventory2.GetAmount( Item ), 30.0f, GetPosition() + vector3(0.0f, 100.0f, 0.0f), radian3(0.0f,r.frand(R_0,R_360),0.0f), Velocity, GetZone1(), GetZone2() );

                // Clear the count
                m_Inventory2.SetAmount( Item, 0.0f );
            }
        }
    }   
}

void character::DropAmmoForWeapon( inven_item weaponItem )
{
    random r( x_rand() );
    vector3 Velocity = r.v3( -50.0f, 50.0f, 20.0f, 50.0f, -50.0f, 50.0f );
    new_weapon* pWeapon = GetWeaponPtr( weaponItem );
    if( pWeapon )
    {    
        switch( weaponItem )
        {
        case INVEN_WEAPON_DESERT_EAGLE:
            pickup::CreatePickup( GetGuid(), net_GetSlot(), INVEN_AMMO_DESERT_EAGLE, (f32)pWeapon->GetAmmoPerClip( new_weapon::AMMO_PRIMARY ), 30.0f, GetPosition(), radian3(0.0f,r.frand(R_0,R_360),0.0f), Velocity, GetZone1(), GetZone2() );
            break;
        case INVEN_WEAPON_SMP:
            pickup::CreatePickup( GetGuid(), net_GetSlot(), INVEN_AMMO_SMP, (f32)pWeapon->GetAmmoPerClip( new_weapon::AMMO_PRIMARY ), 30.0f, GetPosition(), radian3(0.0f,r.frand(R_0,R_360),0.0f), Velocity, GetZone1(), GetZone2() );
            break;
        case INVEN_WEAPON_SHOTGUN:
            pickup::CreatePickup( GetGuid(), net_GetSlot(), INVEN_AMMO_SHOTGUN, (f32)pWeapon->GetAmmoPerClip( new_weapon::AMMO_PRIMARY ), 30.0f, GetPosition(), radian3(0.0f,r.frand(R_0,R_360),0.0f), Velocity, GetZone1(), GetZone2() );
            break;
        case INVEN_WEAPON_SNIPER_RIFLE:
            pickup::CreatePickup( GetGuid(), net_GetSlot(), INVEN_AMMO_SNIPER_RIFLE, (f32)pWeapon->GetAmmoPerClip( new_weapon::AMMO_PRIMARY ), 30.0f, GetPosition(), radian3(0.0f,r.frand(R_0,R_360),0.0f), Velocity, GetZone1(), GetZone2() );
            break;
        case INVEN_WEAPON_MESON_CANNON:
            pickup::CreatePickup( GetGuid(), net_GetSlot(), INVEN_AMMO_MESON, (f32)pWeapon->GetAmmoPerClip( new_weapon::AMMO_PRIMARY ), 30.0f, GetPosition(), radian3(0.0f,r.frand(R_0,R_360),0.0f), Velocity, GetZone1(), GetZone2() );
            break;
        }
    }
}

//=============================================================================

void character::DropInventory( void )
{
    f32 weaponDropPercent   = m_WeaponDropPercent;
    f32 ammoDropPercent     = m_AmmoDropPercent;
    f32 grenadeDropPercent  = m_GrenadeDropPercent;
    f32 inventoryDropPercent = m_InventoryDropPercent; 

#ifndef X_EDITOR
    if( GameMgr.GetGameType() == GAME_CAMPAIGN )
    {
        tweak_handle WepDropScalerTweak( xfs( "Weapon_Drop_%s",             
            DifficultyText[g_Difficulty] ) );
    
        f32 WepDropScalar = 0.0f;

        if( WepDropScalerTweak.Exists() )
        {
            WepDropScalar = WepDropScalerTweak.GetF32();
        }
        // Scale based on difficulty level
        // the scalar could be +/- and is a whole percentage i.e. -20
        weaponDropPercent = weaponDropPercent + ( weaponDropPercent * (WepDropScalar/100.0f) );  

        tweak_handle AmmoDropScalerTweak( xfs( "Ammo_Drop_%s",             
            DifficultyText[g_Difficulty] ) );

        f32 AmmoDropScalar = 0.0f;

        if( AmmoDropScalerTweak.Exists() )
        {
            AmmoDropScalar = AmmoDropScalerTweak.GetF32();
        }
        // Scale based on difficulty level
        // the scalar could be +/- and is a whole percentage i.e. -20
        ammoDropPercent = ammoDropPercent + ( ammoDropPercent * (AmmoDropScalar/100.0f) );  

        tweak_handle GrenadeDropScalerTweak( xfs( "Grenade_Drop_%s",             
            DifficultyText[g_Difficulty] ) );

        f32 GrenadeDropScalar = 0.0f;

        if( GrenadeDropScalerTweak.Exists() )
        {
            GrenadeDropScalar = GrenadeDropScalerTweak.GetF32();
        }
        // Scale based on difficulty level
        // the scalar could be +/- and is a whole percentage i.e. -20
        grenadeDropPercent = grenadeDropPercent + ( grenadeDropPercent * (GrenadeDropScalar/100.0f) );  

        tweak_handle InvDropScalerTweak( xfs( "Inventory_Drop_%s",             
            DifficultyText[g_Difficulty] ) );

        f32 InvDropScalar = 0.0f;

        if( InvDropScalerTweak.Exists() )
        {
            InvDropScalar = InvDropScalerTweak.GetF32();
        }
        // Scale based on difficulty level
        // the scalar could be +/- and is a whole percentage i.e. -20
        inventoryDropPercent = inventoryDropPercent + ( inventoryDropPercent * (InvDropScalar/100.0f) );  
    }
#endif
    // Should npc drop inventory?
    m_WeaponItem = INVEN_NULL;

    // Let's iterate over the inventory and drop ammo for our weapons
    for( s32 i=INVEN_WEAPON_FIRST ; i<=INVEN_WEAPON_LAST ; i++ )
    {
        inven_item Item = (inven_item)i;
        if( m_Inventory2.HasItem( Item ) &&
            x_irand(1,100) <= ammoDropPercent )
        {
            DropAmmoForWeapon( Item );
        }
    }

    // Let's iterate over the inventory and drop weapons
    for( s32 i=INVEN_WEAPON_FIRST ; i<=INVEN_WEAPON_LAST ; i++ )
    {
        inven_item Item = (inven_item)i;
        if( m_Inventory2.HasItem( Item ) &&
            x_irand(1,100) <= weaponDropPercent )
        {
            random r( x_rand() );
            vector3 Velocity = r.v3( -50.0f, 50.0f, 20.0f, 50.0f, -50.0f, 50.0f );
            pickup::CreatePickup( GetGuid(), net_GetSlot(), Item, m_Inventory2.GetAmount( Item ), 30.0f, GetPosition(), radian3(0.0f,r.frand(R_0,R_360),0.0f), Velocity, GetZone1(), GetZone2() );

            // Clear the count
            m_Inventory2.SetAmount( Item, 0.0f );
        }
    }

    // Drop a grenade?
    if( x_irand(1,100) <= grenadeDropPercent )
    {
        random r( x_rand() );
        vector3 Velocity = r.v3( -50.0f, 50.0f, 20.0f, 50.0f, -50.0f, 50.0f );
        pickup::CreatePickup( GetGuid(), net_GetSlot(), INVEN_GRENADE_FRAG, 1, 30.0f, GetPosition(), radian3(0.0f,r.frand(R_0,R_360),0.0f), Velocity, GetZone1(), GetZone2() );
    }

    // Let's iterate over the inventory and drop items
    for( s32 i=INVEN_ITEM_FIRST ; i<=INVEN_ITEM_LAST ; i++ )
    {
        inven_item Item = (inven_item)i;
        if( m_Inventory2.HasItem( Item ) &&
            x_irand(1,100) <= inventoryDropPercent )
        {
            random r( x_rand() );
            vector3 Velocity = r.v3( -50.0f, 50.0f, 20.0f, 50.0f, -50.0f, 50.0f );
            pickup::CreatePickup( GetGuid(), net_GetSlot(), Item, m_Inventory2.GetAmount( Item ), 30.0f, GetPosition(), radian3(0.0f,r.frand(R_0,R_360),0.0f), Velocity, GetZone1(), GetZone2() );

            // Clear the count
            m_Inventory2.SetAmount( Item, 0.0f );
        }
    }
}

//=============================================================================
//  ListenForSounds
//  
//      Gets all the sound from the last time the characters check the listen queue
//      we only pay attention to threats over the min value [8/15/2003]
//=============================================================================

void character::ListenForAlerts( )
{
    alert_package *pAlertReciever;
    alert_package *currentAlertReciever;

    s32 ZoneID = GetZone1();

    if (ZoneID == 0)
    {
        return;
    }

    pAlertReciever = g_AudioManager.GetFirstAlertReceiverItem( m_LastAlertTime );
    m_LastAlertTime = g_ObjMgr.GetGameTime();

    while( pAlertReciever )
    {
        currentAlertReciever = pAlertReciever;
        pAlertReciever = g_AudioManager.GetAlertNextReceiverItem();

        // ignore our own sounds
        if( currentAlertReciever->m_Origin == GetGuid() )
            continue;

        // check the distance to this sound
        f32 Distance = (GetPosition() - currentAlertReciever->m_Position).LengthSquared();

        if( (Distance > x_sqr(m_SoundRange) ) )
        {
            continue;
        }

        OnAlert(*currentAlertReciever);
    }
}

xbool character::ListenForSounds( )
{
    audio_manager::receiver* pReceiver;
    audio_manager::receiver* currentReceiver;
        
    pReceiver = g_AudioManager.GetFirstReceiverItem( m_LastListenTime );
    m_LastListenTime = g_ObjMgr.GetGameTime();

    s32 ZoneID = GetZone1();

    if (ZoneID == 0)
    {
        return FALSE;
    }

    while( pReceiver )
    {
        currentReceiver = pReceiver;
        pReceiver = g_AudioManager.GetNextReceiverItem();

        // ignore our own sounds
        if( currentReceiver->Guid == GetGuid() )
            continue;

        // ignore allied sounds.
        object *soundSource = g_ObjMgr.GetObjectByGuid(currentReceiver->Guid);
        if( soundSource && soundSource->IsKindOf(new_weapon::GetRTTI()) )
        {
            new_weapon &weaponSource = new_weapon::GetSafeType( *soundSource );        
            soundSource = g_ObjMgr.GetObjectByGuid(weaponSource.GetParentGuid());
        }

        if( soundSource && soundSource->IsKindOf(actor::GetRTTI()) )
        {
            actor &actorSource = actor::GetSafeType( *soundSource );        
            if( IsFriendlyFaction(actorSource.GetFaction()) )
                continue;
        }

        // check the distance to this sound
        f32 Distance = (GetPosition() - currentReceiver->OriginalPos).LengthSquared();
        if( (Distance > x_sqr(m_SoundRange) ) )
        {
            continue;
        }

        xbool soundIsLoud = FALSE;
        vector3 soundLoc;

        object* pObj = g_ObjMgr.GetObjectByGuid( currentReceiver->Guid );
        switch( currentReceiver->Type )
        {
        case audio_manager::FOOT_STEP:
            {
                break;
            }
        case audio_manager::EXPLOSION:
        case audio_manager::GUN_SHOT:
            {            
                soundIsLoud = TRUE;
                soundLoc = currentReceiver->Pos;
                break;
            }
        case audio_manager::BULLET_IMPACTS:
            {
                if( pObj && IsEnemyFaction(GetFactionForGuid(currentReceiver->Guid)) )
                {
                    soundIsLoud = TRUE;
                    soundLoc = GetTargetPosWithOffset(currentReceiver->Guid,OFFSET_EYES);
                }
                break;
            }
        }

        
        if( soundIsLoud )
        {
            m_TimeSinceLastSound = 0.0f;
            m_LastSoundPos = soundLoc;
            if( !GetTargetGuid() )
            {
                m_LastLocationOfInterest = m_LastSoundPos;
            }
            return TRUE;
        }
    }
    return FALSE;
}


//=============================================================================

xbool character::SelectWeapon()
{
    for( s32 i=0 ; i<INVEN_NUM_WEAPONS ; i++ )
    {
        if( m_Inventory2.HasItem( inventory2::WeaponIndexToItem(i) ) )
        {
            EquipWeapon2( inventory2::WeaponIndexToItem(i) );
            return TRUE;
        }
    }
    return FALSE;
}

//=============================================================================

xbool character::HasFullClip()
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon )
    {
        return( pWeapon->GetAmmoCount( new_weapon::AMMO_PRIMARY ) >= pWeapon->GetAmmoPerClip( new_weapon::AMMO_PRIMARY ) );
    }
    else
    {
        return FALSE;
    }
   
}

//=============================================================================

void character::UpdateFiring( )
{
    // get our target and our weapon
    object *targetObject = g_ObjMgr.GetObjectByGuid( GetTargetGuid() );
    new_weapon* pWeapon = GetCurrentWeaponPtr();

    // return cases.
    if( !pWeapon )
    {
        return;
    }
    if ( pWeapon->GetAmmoCount() <= 0 && !m_Reloading )
    {
        if( m_bCanReload )
        {        
            if( m_pActiveState && 
                m_pActiveState->CanReload() )
            {            
                ReloadWeapon();
            }
            else
            {
                // clear our reloadtrack 
                GetLocoPointer()->GetAdditiveController(ANIM_FLAG_SHOOT_CONTROLLER).Clear();
            }
        }
        else
        {
            if( m_bWantsToDropWeapon )
            {            
                GetLocoPointer()->PlayAdditiveAnim(loco::ANIM_TOSS_WEAPON, 0.1f, 0.1f, ANIM_FLAG_SHOOT_CONTROLLER);        
            }
        }
        return;
    }
    if( !targetObject )
    {    
        return;
    }
    if( (!GetWantsToFirePrimary() && !GetWantsToFireSecondary()) )
    {    
        return;
    }

    // I'm allowing NPCs to SEE outside thier sightcones once alerted, so they should also shoot 
    // you at this point. If this is used NPCs will see you across the room, aim at you but never fire.
/*    if( !IsTargetInSightCone() )    
    {
        return;
    }*/

    xbool targetInCone = FALSE;
    if( CanSeeTarget() )
    {
        targetInCone = IsTargetInFiringCone( GetTargetPosWithOffset(GetTargetGuid(),OFFSET_EYES) );
    }
    else
    {
        targetInCone = IsTargetInFiringCone( GetLastSeenLocationOfTarget() );
    }
    
    if( targetInCone && pWeapon->IsWeaponReady( new_weapon::AMMO_PRIMARY ) )   
    {    
        if( GetWantsToFirePrimary() &&
            GetLocoPointer()->GetAdditiveController(character::ANIM_FLAG_SHOOT_CONTROLLER).IsAtEnd() )
        {        
            PlayPrimaryFiringAnimation();
            SetWantsToFirePrimary(FALSE);
        }
        else if ( GetWantsToFireSecondary() &&
                  GetLocoPointer()->GetAdditiveController(character::ANIM_FLAG_SHOOT_CONTROLLER).IsAtEnd() )
        {        
            PlaySecondaryFiringAnimation();
            SetWantsToFireSecondary(FALSE);
        }
    }

    // override so we always aim when firing.
    if ( !GetLocoPointer()->GetAdditiveController(character::ANIM_FLAG_SHOOT_CONTROLLER).IsAtEnd() )
    {
        //anim already playing
        SetWantsToAim(TRUE);
    }

}


//=============================================================================

void character::AutofireRequestOver()
{
    if( !HasAllies() )
    {
        return;
    }
    s32 count;
    for(count=0;count<k_MaxAllies;count++)
    {
        object* allyObject = g_ObjMgr.GetObjectByGuid(m_AllyGuids[count]);
        if( allyObject && allyObject->IsKindOf(character::GetRTTI()) )
        {
            character &allyCharacter = character::GetSafeType(*allyObject);
            allyCharacter.CancelAutofire();
        }
    }
}

//=============================================================================

xbool character::HasAlliesWithin( f32 distSqr )
{
    s32 c;
    for( c=0;c<k_MaxAllies;c++ )
    {
        object* allyObject = g_ObjMgr.GetObjectByGuid(m_AllyGuids[c]);
        if( allyObject )
        {
            if( GetToTarget(m_AllyGuids[c]).LengthSquared() <= distSqr )
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

//=============================================================================

xbool character::RequestAutofire()
{
    if( !HasAllies() )
    {
        return FALSE;
    }
    s32 count;
    for(count=0;count<k_MaxAllies;count++)
    {
        object* allyObject = g_ObjMgr.GetObjectByGuid(m_AllyGuids[count]);
        if( allyObject && allyObject->IsKindOf(character::GetRTTI()) )
        {
            character &allyCharacter = character::GetSafeType(*allyObject);
            // only request from allies that are reasonably close to me. 
            // had a guy that was way down a ramp and not really in the fight providing autofire through the wall. 
            // not good.
            if( GetToTarget(m_AllyGuids[count]).LengthSquared() <= k_MinDistToProvideAutofireSqr &&
                allyCharacter.ProvideAutofire() )
            {
                if( m_pActiveState )
                {
                    m_pActiveState->AutofireRequestSent();
                }
                return TRUE;
            }
        }
    }
    return FALSE;
}

//=============================================================================

xbool character::ProvideAutofire()
{
    if( HasState(character_state::STATE_COVER) )
    {
        character_cover_state *coverState = (character_cover_state *)GetStateByType( character_state::STATE_COVER );
        return coverState->ProvideAutofire();
    }
    return FALSE;
}

//=============================================================================

void character::CancelAutofire()
{
    if( HasState(character_state::STATE_COVER) )
    {
        character_cover_state *coverState = (character_cover_state *)GetStateByType( character_state::STATE_COVER );
        coverState->CancelAutofire();
    }
}

//=============================================================================

void character::UpdateReloading( )
{
    // if the reload animation finishes, increment the ammo count
    if ( GetLocoPointer()->GetMaskController().IsAtEnd() )
    {
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if( pWeapon )
        {        
            pWeapon->Reload( new_weapon::AMMO_PRIMARY );
            m_Reloading = FALSE;
            m_TimeSinceLastReload = 0.0f;
            AutofireRequestOver();
        }
        else
        {
            m_Reloading = FALSE;
        }
    }    
}
//=============================================================================

xbool character::IsPlayingIdleFiring( void )
{
    // Get index of current anim playing
    s32 iCurrAnimIndex = GetLocoPointer()->m_Player.GetCurrAnim().GetAnimTypeIndex();

    // Check against all anim types
    for( s32 i = loco::ANIM_SHOOT_IDLE_SMP; i <= loco::ANIM_SHOOT_CROUCH_IDLE_MUTANT; i++ )
    {
        // Found?
        if( GetLocoPointer()->GetAnimIndex((loco::anim_type)i) == iCurrAnimIndex )        // <--- this table maps enum -> anim index
            return TRUE;
    }

    // not found
    return FALSE;
}

//=============================================================================

void character::PlayPrimaryFiringAnimation( )
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    loco::anim_type AnimType = loco::ANIM_NULL ;

    // makes no sense to fire if we aren't aiming at least somewhat at the target.

    // Is npc idling and not staggering?        
    if(     ( GetLocoPointer()->GetState() == loco::STATE_IDLE || IsPlayingIdleFiring() ) &&
            ( GetLocoPointer()->IsAtDestination() ) &&
            ( !IsStaggering() ) )
    {    
        if( GetLocoPointer()->GetMoveStyle() == loco::MOVE_STYLE_CROUCHAIM )
        {        
            // Which anim?
            switch( pWeapon->GetInvenItem() )
            {
                // KSS -- TO ADD NEW WEAPON
            case INVEN_WEAPON_SHOTGUN: 
                AnimType = loco::ANIM_SHOOT_CROUCH_IDLE_SHOTGUN;      
                break;
            case INVEN_WEAPON_SMP: 
                AnimType = loco::ANIM_SHOOT_CROUCH_IDLE_SMP;
                break;
            case INVEN_WEAPON_SCANNER: 
                AnimType = loco::ANIM_SHOOT_CROUCH_IDLE_SCN;
                break;
            case INVEN_WEAPON_SNIPER_RIFLE: 
                AnimType = loco::ANIM_SHOOT_CROUCH_IDLE_SNIPER;      
                break;
            case INVEN_WEAPON_DESERT_EAGLE: 
                AnimType = loco::ANIM_SHOOT_CROUCH_IDLE_DESERT_EAGLE; 
                break;
            case INVEN_WEAPON_MESON_CANNON: 
                AnimType = loco::ANIM_SHOOT_CROUCH_IDLE_MSN; 
                break;
            case INVEN_WEAPON_BBG:
                AnimType = loco::ANIM_SHOOT_CROUCH_IDLE_BBG;          
                break;
            case INVEN_WEAPON_MUTATION:
                AnimType = loco::ANIM_SHOOT_CROUCH_IDLE_MUTANT;
                break;
            }
        }
        else
        {
            // Which anim?
            switch( pWeapon->GetInvenItem() )
            {
                // KSS -- TO ADD NEW WEAPON
            case INVEN_WEAPON_SHOTGUN: 
                AnimType = loco::ANIM_SHOOT_IDLE_SHOTGUN;      
                break;
            case INVEN_WEAPON_SMP: 
                AnimType = loco::ANIM_SHOOT_IDLE_SMP;          
                break;
            case INVEN_WEAPON_SCANNER: 
                AnimType = loco::ANIM_SHOOT_IDLE_SCN;
                break;
            case INVEN_WEAPON_SNIPER_RIFLE: 
                AnimType = loco::ANIM_SHOOT_IDLE_SNIPER;      
                break;
            case INVEN_WEAPON_DESERT_EAGLE: 
                AnimType = loco::ANIM_SHOOT_IDLE_DESERT_EAGLE; 
                break;
            case INVEN_WEAPON_MESON_CANNON: 
                AnimType = loco::ANIM_SHOOT_IDLE_MSN; 
                break;
            case INVEN_WEAPON_BBG:
                AnimType = loco::ANIM_SHOOT_IDLE_BBG;
                break;
            case INVEN_WEAPON_MUTATION:
                AnimType = loco::ANIM_SHOOT_IDLE_MUTANT;
                break;
            }
        }
    }

    // Anim found?
    if( AnimType != loco::ANIM_NULL &&
        HasAnim(AnimType) )
    {
        GetLocoPointer()->PlayAnim(AnimType);
        return;
    }

    // we didn't have the idle anim so....
    // Which anim?
    switch( pWeapon->GetInvenItem() )
    {
// KSS -- TO ADD NEW WEAPON
    case INVEN_WEAPON_SHOTGUN: 
        AnimType = loco::ANIM_SHOOT_SHOTGUN;      
        break;
    case INVEN_WEAPON_SMP: 
        AnimType = loco::ANIM_SHOOT_SMP;          
        break;
    case INVEN_WEAPON_SCANNER: 
        AnimType = loco::ANIM_SHOOT_SCN;
        break;
    case INVEN_WEAPON_SNIPER_RIFLE: 
        AnimType = loco::ANIM_SHOOT_SNIPER;      
        break;
    case INVEN_WEAPON_DESERT_EAGLE: 
        AnimType = loco::ANIM_SHOOT_DESERT_EAGLE; 
        break;
    case INVEN_WEAPON_MESON_CANNON: 
        AnimType = loco::ANIM_SHOOT_MSN; 
        break;
    case INVEN_WEAPON_BBG: 
        AnimType = loco::ANIM_SHOOT_BBG;
        break;
    case INVEN_WEAPON_MUTATION:
        AnimType = loco::ANIM_SHOOT_MUTANT;
        break;
    }
    // Anim found?
    if (AnimType != loco::ANIM_NULL)
        GetLocoPointer()->PlayAdditiveAnim(AnimType, 0.1f, 0.1f, ANIM_FLAG_SHOOT_CONTROLLER) ;
}

//=============================================================================

void character::PlaySecondaryFiringAnimation( )
{
    // Which anim?
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    loco::anim_type AnimType = loco::ANIM_NULL ;
    switch( pWeapon->GetInvenItem() )
    {
// KSS -- TO ADD NEW WEAPON
    case INVEN_WEAPON_SHOTGUN: 
        AnimType = loco::ANIM_SHOOT_SECONDARY_SHOTGUN;      
        break;
    case INVEN_WEAPON_SMP: 
        AnimType = loco::ANIM_SHOOT_SECONDARY_SMP;          
        break;
    case INVEN_WEAPON_SCANNER:
        AnimType = loco::ANIM_SHOOT_SECONDARY_SCN;
        break;
    case INVEN_WEAPON_SNIPER_RIFLE: 
        AnimType = loco::ANIM_SHOOT_SECONDARY_SNIPER;      
        break;
    case INVEN_WEAPON_DESERT_EAGLE: 
        AnimType = loco::ANIM_SHOOT_SECONDARY_DESERT_EAGLE; 
        break;
    case INVEN_WEAPON_MUTATION:
        AnimType = loco::ANIM_SHOOT_SECONDARY_MUTANT;
        break;
    }

    // Anim found?
    if (AnimType != loco::ANIM_NULL)
        GetLocoPointer()->PlayMaskedAnim(AnimType, loco::BONE_MASKS_TYPE_RELOAD_SHOOT, 0.1f) ;
}

//=============================================================================

void character::PlayReloadAnimation( )
{
    // Which anim?
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( !pWeapon )
    {
        return;
    }

    loco::anim_type AnimType = loco::ANIM_NULL ;
    switch( pWeapon->GetInvenItem() )
    {
    case INVEN_WEAPON_SHOTGUN: 
        AnimType = loco::ANIM_RELOAD_SHOTGUN;      
        break;
    case INVEN_WEAPON_SMP: 
        AnimType = loco::ANIM_RELOAD_SMP;          
        break;
    case INVEN_WEAPON_SNIPER_RIFLE: 
        AnimType = loco::ANIM_RELOAD_SNIPER;      
        break;
    case INVEN_WEAPON_DESERT_EAGLE: 
        AnimType = loco::ANIM_RELOAD_DESERT_EAGLE; 
        break;
    }

    // Anim found?
    if (AnimType != loco::ANIM_NULL)
        GetLocoPointer()->PlayMaskedAnim(AnimType, loco::BONE_MASKS_TYPE_RELOAD_SHOOT, 0.1f) ;
}

//=============================================================================

//=============================================================================

void character::ThrowGrenade( const vector3& grenadePosition  )
{
    if( m_GrenadeItem != INVEN_GRENADE_FRAG )
    {
        x_DebugMsg("ERROR: Attempted to throw frag grenade, but none in inventory!\n");
        return;
    }

    DetermineGrenadeTrajectory();

    // Create grenade.
    vector3 GrenadeInitPos  = grenadePosition;
    f32 ForceOfThrow = 1500.f;
    
    m_GrenadeThrowStart = GrenadeInitPos;

    guid GrenadeID = CREATE_NET_OBJECT( grenade_projectile::GetObjectType(), TYPE_GRENADE );
    grenade_projectile* pGrenade = ( grenade_projectile* ) g_ObjMgr.GetObjectByGuid( GrenadeID );
    pain_handle PainHandle("CHARACTER_GRENADE");

    // Compute velocity
    vector3 Velocity( 0.0f, 0.0f, ForceOfThrow );
    Velocity.Rotate( m_GrenadeTrajectory );

    pGrenade->Setup( GetGuid(),
#ifdef X_EDITOR
                     -1,
#else
                     m_NetSlot,
#endif
                     GrenadeInitPos,
                     radian3( R_0, R_0, R_0 ),
                     Velocity,
                     GetZone1(),
                     GetZone2(),
                     PainHandle );

    PlayDialog( DIALOG_GRENADE_THROW );
    m_LastGrenadeThrowTime = g_ObjMgr.GetGameTime();

    #ifndef X_EDITOR
    m_NetDirtyBits |= TOSS_BIT; // NETWORK
    #endif X_EDITOR
}

//=============================================================================

void character::ThrowSecondaryGrenade( const vector3& grenadePosition  )
{
    (void)grenadePosition;
    ASSERTS(0, "Grav Grenade is obsolete");
    /*
    if( m_GrenadeItem != INVEN_GRENADE_GRAV )
    {
        x_DebugMsg("ERROR: Attempted to throw grav grenade, but none in inventory!\n");
        return;
    }

    vector3 GrenadeInitPos  = grenadePosition ;

    guid GravChargeID = g_ObjMgr.CreateObject( grav_charge_projectile::GetObjectType() );
    grav_charge_projectile* pGravCharge = ( grav_charge_projectile* ) g_ObjMgr.GetObjectByGuid( GravChargeID );

    //make sure the grenade was created.
    ASSERT( pGravCharge );

    pGravCharge->LoadInstance( PRELOAD_FILE("WPN_GRAV_Bindpose.rigidgeom") );

    vector3 toTarget = GetLastKnownLocationOfTarget() - GrenadeInitPos;
    radian3 radianToTarget(toTarget.GetPitch(),toTarget.GetYaw(),0.0f);

    pain_handle PainHandle("CHARACTER_GRAVCHARGE");
    pGravCharge->Initialize( GrenadeInitPos, radianToTarget, vector3(0,0,0), 2000, GetGuid(), PainHandle );

    PlayDialog( DIALOG_GRENADE_THROW );
    m_LastGrenadeThrowTime = g_ObjMgr.GetGameTime();
    */
}

//=============================================================================

void character::AllowShootingNow()
{
    m_SinceLastShot = m_ShootDelay + 0.1f;
}

//=============================================================================

xbool character::IsFiring()
{
    xbool retVal = FALSE;
    if( !GetLocoPointer()->GetAdditiveController(ANIM_FLAG_SHOOT_CONTROLLER).IsAtEnd() )
    {
        retVal = TRUE;
    }
    else if( m_pActiveState && m_pActiveState->GetStateType() == character_state::STATE_COVER )
    {
        character_cover_state* coverState = (character_cover_state *)m_pActiveState;
        if( coverState->GetCurrentPhase() == character_cover_state::PHASE_COVER_OUT_IDLE ||
            coverState->GetCurrentPhase() == character_cover_state::PHASE_COVER_FULL_AUTO )
        {
            retVal = TRUE;
        }
    }
    return retVal;
}

//=============================================================================

void character::CheckShooting( void )
{
    object *targetObject = g_ObjMgr.GetObjectByGuid( GetTargetGuid() );
    if( WeaponReady() && 
        ( m_SinceLastShot > m_ShootDelay ||
          ( GetToTarget().LengthSquared() <= k_MaxDistRapidFiringSqr &&
            targetObject && !targetObject->IsKindOf(player::GetRTTI()))) )
    {
        if ( GetAimToTargetYaw() < R_40 )
        {
            SetWantsToAim(TRUE);
        }

        if ( CanShootAtTarget() )
        {
            m_SinceLastShot = 0.0f;
            SetWantsToFirePrimary(TRUE);
        }
    }
}

//=============================================================================
// UTIL FUNCTIONS
//=============================================================================

//=============================================================================

// Returns position of target
vector3 character::GetTargetPosWithOffset( guid target, eOffsetPos offset )
{
    object* pObject = g_ObjMgr.GetObjectByGuid( target );
    if( !pObject )
    {    
        pObject = g_ObjMgr.GetObjectByGuid( GetTargetGuid() );
    }

    if( pObject ) 
    {
        if( pObject->IsKindOf(actor::GetRTTI()) )
        {        
            actor &actorSource = actor::GetSafeType( *pObject );        
            return actorSource.GetPositionWithOffset( offset );
        }
        else
        {
            switch( offset )
            {
            case OFFSET_NONE:
                {
                    return pObject->GetPosition();
                    break;
                }
            case OFFSET_CENTER:
                {
                    return pObject->GetBBox().GetCenter();
                    break;
                }
            case OFFSET_AIM_AT:
            case OFFSET_EYES:
                {
                    return pObject->GetBBox().GetCenter();
                    break;
                }
            case OFFSET_TOP_OF_BBOX:
                {
                    return pObject->GetPosition() + vector3( 0.0f,pObject->GetBBox().Max.GetY(), 0.0f );
                    break;
                }
            default:
                {                
                    return pObject->GetPosition();
                    break;
                }
            }
        }
    } 
    else
    {
        return GetLastLocationOfInterest();
    }
}

//=============================================================================

xbool character::IsTargetEnemy( guid Guid /* = 0  */ )
{
    // if they didn't pass in a target, use our active target
    if( Guid != 0 )
    {
        return( IsEnemyFaction(GetFactionForGuid(Guid)));
    }
    else if( GetTargetGuid() != 0 )
    {
        return( IsEnemyFaction(GetFactionForGuid(GetTargetGuid())));
    }
    else
    {
        return FALSE;
    }
}

//=============================================================================

xbool character::UpdateCanReachTarget( const vector3& targetPosition, f32 reachRadius )
{
    // Ignore nav map?
    if( m_PathingHints.bUseNavMap == FALSE )
        return TRUE;
        
    nav_connection_slot_id targetConnectionID = NULL_NAV_SLOT;

    // if the target position is a null vector, then there is no target to reach
    if ( targetPosition.LengthSquared() < k_VerySmallNumber )
        return FALSE;

    // find the connection the target is in
    if( !g_NavMap.GetConnectionContainingPoint(targetConnectionID, targetPosition) )
    {
        targetConnectionID = g_NavMap.GetNearestConnection( targetPosition );
        if( targetConnectionID == NULL_NAV_SLOT )
        {
            // if no connection near target, bail.
            return FALSE;
        }

        vector3 vectorToConnection = g_NavMap.GetNearestPointOnConnection( targetConnectionID, targetPosition ) - targetPosition;
        vectorToConnection.GetY() = 0;
        if( vectorToConnection.Length() > reachRadius )
        {
            // if target is too far from connection, bail.
            return FALSE;
        }   
    }

    nav_connection_slot_id ourConnectionID = 0;
    // find the connection we are in, return FALSE if in no connections.
    if( !g_NavMap.GetConnectionContainingPoint(ourConnectionID, GetPosition()) )
    {
        ourConnectionID = g_NavMap.GetNearestConnection( GetPosition() );
        if( ourConnectionID == NULL_NAV_SLOT )
        {
            // if no connection near us, bail.
            return FALSE;
        }
    }

    return ( g_NavMap.GetConnectionGridID( targetConnectionID ) == g_NavMap.GetConnectionGridID( ourConnectionID ) );
}

//=============================================================================

xbool character::IsFacingTarget( radian SightFOV )
{
    return IsFacingTarget( GetTargetPosWithOffset(), SightFOV );
}

//=============================================================================

// Returns TRUE if facing target
xbool character::IsFacingTarget( const vector3& TargetPos, radian SightFOV )
{
    // Needs loco!
    if (!GetLocoPointer())
        return FALSE;

    // Get delta to from eye to object position just like the loco does!
    vector3 Delta = TargetPos - GetLocoPointer()->GetEyePosition() ;
    Delta.GetY() = 0;

    // Outside of field of view?
    radian FaceYaw   = GetLocoPointer()->GetYaw();//GetLocoPointer()->GetAimerYaw(); //GetLocoPointer()->m_Player.GetFacingYaw();
    radian TargetYaw = Delta.GetYaw();
    radian DeltaYaw  = x_abs(x_MinAngleDiff(FaceYaw, TargetYaw));
    if (DeltaYaw > ( SightFOV * 0.5f ) )
        return FALSE;

    // In cone
    return TRUE;
}

//=============================================================================

// Returns TRUE if the target is facing us.
xbool character::IsTargetFacingUs( radian SightFOV )
{
    object *targetObject = g_ObjMgr.GetObjectByGuid( GetTargetGuid() );
    if( !targetObject || !targetObject->IsKindOf(actor::GetRTTI()) )
    {
        return FALSE;
    }

    actor& targetActor = actor::GetSafeType(*targetObject);
    vector3 targetToUs = GetPosition() - GetTargetPosWithOffset();

    radian faceYaw = targetActor.GetLocoPointer()->GetYaw();    
    radian targetToUsYaw = targetToUs.GetYaw();

    radian DeltaYaw  = x_abs(x_MinAngleDiff(faceYaw, targetToUsYaw));
    if (DeltaYaw > ( SightFOV * 0.5f ) )
        return FALSE;

    // In cone
    return TRUE;
}

//=============================================================================

xbool character::IsTargetInSightCone( f32 SightRadius, radian SightFOV )
{
    if( CanSeeTarget() )
    {    
        return IsTargetInSightCone( GetTargetPosWithOffset(GetTargetGuid(),OFFSET_EYES),SightRadius,SightFOV);
    }
    else
    {
        return IsTargetInSightCone( GetLastSeenLocationOfTarget(),SightRadius,SightFOV);
    }
}

// Returns TRUE if facing target
xbool character::IsTargetInSightCone( const vector3& TargetPos, f32 SightRadius, radian SightFOV )
{
    // Needs loco!
    if (!GetLocoPointer())
        return FALSE;

    // Get object position
    vector3 Delta     = TargetPos - GetPosition();
    Delta.GetY() = 0;

    
    // Too far away?
    if ((Delta.LengthSquared() > x_sqr(SightRadius)) && (SightRadius >= 0.0f) )
        return FALSE;

    // Outside of field of view?
    radian LookatYaw;
    LookatYaw = GetSightYaw();

    radian TargetYaw = Delta.GetYaw();
    radian DeltaYaw  = x_abs(x_MinAngleDiff(LookatYaw, TargetYaw));
    if( DeltaYaw >= ( SightFOV * 0.5f ) && GetActiveState() != character_state::STATE_COVER )
        return FALSE;

    // In cone
    return TRUE;
}

//=============================================================================

// Returns TRUE if facing target
xbool character::IsTargetInSightCone( const vector3& TargetPos, u8 brightness )
{
    // Use character settings
    f32 sightRadius;
    radian sightFOV;
    if( brightness > 50 || GetAwarenessLevel() >= AWARENESS_ACQUIRING_TARGET )
    {
        sightRadius = m_LightSightRadius;
    }
    else
    {
        sightRadius = m_DarkSightRadius;
    }

    if( GetAwarenessLevel() >= AWARENESS_ALERT )
    {
        sightFOV = m_AlertSightFOV;
    }
    else
    {
        sightFOV = m_IdleSightFOV;
    }
    return IsTargetInSightCone(TargetPos, sightRadius, sightFOV );
}

//=============================================================================

// Returns TRUE if facing target
xbool character::IsTargetInSightCone( )
{
    // Use character settings
    object *targetObject = g_ObjMgr.GetObjectByGuid( GetTargetGuid() );
    if( !targetObject )
    {    
        return FALSE;
    }
    else
    {    
        u8 brightness = 255;
        // we only pay attention to brightness for players we haven't targeted.
        if( targetObject && targetObject->IsKindOf(player::GetRTTI()) && GetTargetGuid() != targetObject->GetGuid() )
        {
            player &playerSource = player::GetSafeType( *targetObject );        
            brightness  = (u8)playerSource.GetFloorIntensity();
        }
        f32 sightRadius;
        radian sightFOV;
        if( brightness > 50 )
        {
            sightRadius = m_LightSightRadius;
        }
        else
        {
            sightRadius = m_DarkSightRadius;
        }

        if( GetAwarenessLevel() >= AWARENESS_ALERT )
        {
            sightFOV = m_AlertSightFOV;
        }
        else
        {
            sightFOV = m_IdleSightFOV;
        }
        if( CanSeeTarget() )
        {        
            return IsTargetInSightCone( GetTargetPosWithOffset(GetTargetGuid(),OFFSET_EYES), sightRadius, sightFOV );
        }
        else
        {
            return IsTargetInSightCone( GetLastSeenLocationOfTarget(), sightRadius, sightFOV );
        }
    }
}

//=============================================================================

xbool character::GetIsAtDestination( void )
{
    if (!GetLocoPointer())
        return TRUE;

    return GetLocoPointer()->IsAtDestination();
}

//=========================================================================

void character::OverridePhysics( xbool bOverride )
{
    // Update loco?
    if (GetLocoPointer())
    {
        // Turn off gravity and collision if we are overriding
        GetLocoPointer()->m_Physics.SetLocoCollisionOn(bOverride == FALSE);
        GetLocoPointer()->m_Physics.SetLocoGravityOn  (bOverride == FALSE);
    }
}


//=========================================================================

void character::OnActivate( xbool Flag )
{
    m_LocoFallingOverride = FALSE;
    m_LastListenTime = g_ObjMgr.GetGameTime();
    m_LastAlertTime  = g_ObjMgr.GetGameTime();
    m_InitialState = character_state::STATE_NULL;
    if (Flag)
    {
        //goto idle state
        if( !m_pActiveState || m_pActiveState->GetStateType() == character_state::STATE_HOLD )
        {   
            character_state::states oldDesiredState = m_DesiredState;
            m_DesiredState = character_state::STATE_IDLE;
            if( ChangeState() )
            {            
                m_DesiredState = oldDesiredState;
            }
        }
    }
    else          
    {
        //goto hold state
        m_DesiredState = character_state::STATE_HOLD;
        ChangeState();

        // Make sure lip sync controller is stopped and cleared so that cinema blocking continues
        loco* pLoco = GetLocoPointer();
        if( pLoco )
        {
            // Stop and clear weight so it no longer returns TRUE for "IsPlaying"
            loco_lip_sync_controller& LipSyncCont = pLoco->GetLipSyncController();
            LipSyncCont.Stop();
            LipSyncCont.SetWeight( 0.0f );
        }        
    }
    actor::OnActivate(Flag);
}

//========================================================================================

// This function is only called in an online game on the server!
void character::OnDeath()
{
    object *activateObject = g_ObjMgr.GetObjectByGuid( m_ActivateOnDeath );
    if( activateObject )
    {
        activateObject->OnActivate(TRUE);
    }
    
    if ( m_pNoBlockDialogTrigger != 0)
    {
        object* pObject = g_ObjMgr.GetObjectByGuid( m_pNoBlockDialogTrigger );
        if( pObject && pObject->IsKindOf(trigger_ex_object::GetRTTI()) )
        {
            trigger_ex_object &tempTrigger = trigger_ex_object::GetSafeType(*pObject);
            tempTrigger.ReleaseBlocking();
        }
        m_pNoBlockDialogTrigger = 0;
    }
}

//========================================================================================

// This function is only called in an online game on the server!
void character::OnSpawn( void )
{
#ifndef X_EDITOR
    // Only call if online since the characters death state takes care of everything
    // in campaign mode
    if( g_NetworkMgr.IsOnline() == TRUE )
    {
        // Call base class
        actor::OnSpawn();

        // Re-intialize the AI
        SetupState( character_state::STATE_IDLE );
    }
#endif
}

//========================================================================================

void character::OnContagionDOT( void )
{
    pain Pain;
    Pain.Setup( "CONTAGION_TICK", GetGuid(), GetPosition() );
    Pain.SetDirectHitGuid( GetGuid() );
    Pain.ApplyToObject( GetGuid() );
}

//========================================================================================

void character::TurnAgainstPlayer()
{
    // removed for bug request #7509

/*    if( IsFriendlyFaction( FACTION_PLAYER_NORMAL ) )
    {
        SetFriendFlags( GetFriendFlags() &~ FACTION_PLAYER_NORMAL );
        SetFriendFlags( GetFriendFlags() &~ FACTION_PLAYER_STRAIN1 );
        SetFriendFlags( GetFriendFlags() &~ FACTION_PLAYER_STRAIN2 );
        SetFriendFlags( GetFriendFlags() &~ FACTION_PLAYER_STRAIN3 );
    }*/
}

//========================================================================================
void character::OnGrenadeAlert( alert_package& Package ) 
{
    // Skip if playing full body lip sync
    if( IsPlayingFullBodyLipSync() )
    {
        return;
    }
    
    if( GetAwarenessLevel() <= AWARENESS_SEARCHING )
    {
        PlayDialog( DIALOG_ALERT );
        SetOverrideLookatInterest( Package.m_Cause );
    }
    else if( m_pActiveState )
    {
        PlayDialog( DIALOG_GRENADE_SPOT);
        m_pActiveState->OnGrenadeAlert( Package );
    }
}

//========================================================================================

void character::OnAlert( alert_package& Package )
{
    // Skip if playing full body lip sync
    if( IsPlayingFullBodyLipSync() )
    {
        return;
    }

    if (!m_pActiveState)
        return;
    
    if( m_pActiveState->IgnoreAlerts() )
        return;

    if (Package.m_FactionsSpecific != FACTION_NOT_SET && 
        Package.m_FactionsSpecific != GetFaction())
    {
        //not my concern
        return;
    }

    if( IsAlive() )
    {

        switch ( Package.m_Type )
        {
        case alert_package::ALERT_TYPE_REQUEST_COVER_FIRE_DIALOG_DONE:
            {
                if( m_bCrazyFire )
                {
                    PlayDialog( DIALOG_COVER );
                }
                else
                {
                    // if we are in cover state and wanting to full auto but haven't yet
                    // then we will play the dialog once we are ready.
                    if( GetActiveState() == character_state::STATE_COVER )
                    {
                        ((character_cover_state *)m_pActiveState)->CoverRequestDialogDone();
                    }
                }
            }
            break;
        case alert_package::ALERT_TYPE_GRENADE:
            {            
                //this is a grenade alert, do something here
                // but only if an enemy grenade.
                if( IsFriendlyFaction(GetFactionForGuid(Package.m_Cause)) )
                {
                    break;
                }
                f32 toAlertSqr = ( Package.m_Position - GetPosition() ).LengthSquared();  
                if( x_irand(1,100) < k_GrenadeEvadePercent &&
                    toAlertSqr <= k_GrenadeEvadeDistance )
                {            
                    OnGrenadeAlert(Package);
                }
            }
            break;
        case alert_package::ALERT_TYPE_NPC_SHOUT:
            {
                if ( Package.m_Cause != 0 )
                {
                    if ( IsEnemyFaction(GetFactionForGuid(Package.m_Cause)) )
                    {
                        if( (GetTargetGuid() == 0 || IsNewTargetCloser(Package.m_Cause)) &&
                            GetTargetGuid() != Package.m_Cause )
                        {                            
                            // if we actually set the our target then we want to tell
                            // the one that called us so they can play the attack anim.
                            guid originalTarget = GetTargetGuid();
                            SetTargetGuid(Package.m_Cause,FALSE);
                            
                            if( originalTarget == 0 &&
                                GetTargetGuid() == Package.m_Cause )
                            {
                                // we want to tell the sender that we acquired the target. 
                                // No need to go through the alert system, just grab em and tell em.
                                object *senderObject = g_ObjMgr.GetObjectByGuid( Package.m_Origin );
                                if( senderObject && senderObject->IsKindOf(character::GetRTTI()) )
                                {
                                    character &senderCharacter = character::GetSafeType( *senderObject );
                                    senderCharacter.SetAllyAcquiredTarget(TRUE);
                                }
                            }
                        }
                    }
                }
                break;
            }
        case alert_package::ALERT_TYPE_SOUND:
            {
                if ( Package.m_Cause != 0 )
                {
                    if ( IsFriendlyFaction(GetFactionForGuid(Package.m_Cause)) )
                    {
                        m_LastSoundPos = Package.m_Position;
                        m_TimeSinceLastSound = 0.0f;
                    }
                }
                break;
            }        
        case alert_package::ALERT_TYPE_EXPLOSION:
            {
                if ( Package.m_Cause != 0 )
                {
                    if ( IsEnemyFaction(GetFactionForGuid(Package.m_Cause)) )
                    {
                        if( GetTargetGuid() == 0 || IsNewTargetCloser(Package.m_Cause) )
                        {                            
                            SetTargetGuid(Package.m_Cause,FALSE);
                        }
                    }
                }
                break;
            }
        case alert_package::ALERT_TYPE_ACTOR_DIED:
            {
                // What faction is the friendly player?
                factions deadActorFaction =  GetFactionForGuid(Package.m_Origin);
                factions painSourceFaction = GetFactionForGuid(Package.m_Cause);
                object *causeObject = g_ObjMgr.GetObjectByGuid(Package.m_Cause);

                if( IsFriendlyFaction(deadActorFaction) )
                {
                    PlayDialog( DIALOG_MANDOWN );
                    if( IsFriendlyFaction(painSourceFaction) &&
                        causeObject->IsKindOf(player::GetRTTI()) &&
                        GetTimeSinceLastCombat() >= k_MinTimeTillCombatClear )
                    {
                        TurnAgainstPlayer();
                    }
                }
                else
                {
                    // an enemy is dead... do we want to do anything in particular?
                }
                break;
            }
        case alert_package::ALERT_TYPE_PLAYER_TURNED:
            {
                TurnAgainstPlayer();
            }
        default:
            break;
        }
    }
}

//=============================================================================

xbool character::PlayDialog( eDialogType dialogType, const char* dialogName, xbool hotVoice, const char* animName , const char* animPkg, u32 Flags, u8 DialogFlags, f32 BlendOutTime )
{
    xtick CurrTime = g_ObjMgr.GetGameTime();
/*
    if( !hotVoice )
    {
        scePrintf("PlayDialog: %d %32s\n",hotVoice,(dialogName)?(dialogName):("noname"));
    }
*/
    // this is a request to play a dialog line.   
    // first decide if we are going to play this line or not.    
    xbool bPlayDialog = TRUE;
    if( g_ConverseMgr.IsPlaying(m_VoiceID) )
    {
        if( dialogName && g_AudioMgr.GetPriority(m_VoiceID) > g_AudioMgr.GetPriority(dialogName) )
        {            
            bPlayDialog = FALSE;
        }
    }

    switch( dialogType )
    {
        // these cases always interrupt.
    case DIALOG_DIE_MELEE:
    case DIALOG_DIE_GUNFIRE:
    case DIALOG_DIE_EXPLOSION:
    case DIALOG_DIE_FALL:
    case DIALOG_GOAL_DIALOG:
    case DIALOG_TRIGGER_NO_INTERRUPT:
    case DIALOG_ANIM_EVENT:
        if( !bPlayDialog )
        {
            return FALSE;
        }
        break;

    case DIALOG_SCANNER_VO:
        {
            // otherwise, we will not play if we have played a sound recently
            // or we are playing a sound right now.
            if( !bPlayDialog || m_TimeSinceLastDialog < k_MinTimeBetweenScannerVO )
            {
                return FALSE;
            }
            else
            {
                // gonna play, go ahead and increment VO index
                m_ScannerVOIndex++;

                // keep index in bounds
                if( m_ScannerVOIndex > s_NPCTable[m_NPCName].LastVOIndex )
                {
                    m_ScannerVOIndex = 1;
                }
            }
        }
        break;
    default:
        // otherwise, we will not play if we have played a sound recently
        // or we are playing a sound right now.
        if( !bPlayDialog || m_TimeSinceLastDialog < k_MinTimeBetweenDialogs )
        {
            return FALSE;
        }
        break;            
    }       

    if( (dialogType == DIALOG_GOAL_DIALOG)          ||
        (dialogType == DIALOG_ANIM_EVENT)           || 
        (dialogType == DIALOG_TRIGGER_NO_INTERRUPT) ||
        (dialogType == DIALOG_SCANNER_VO)
        )
    {
        // see if we have this dialog
        if( !g_ConverseMgr.HasDialog(dialogName) )
        {
            return FALSE;
        }
        // if we are playing a dialog, stop it
        if( g_ConverseMgr.IsActive(m_VoiceID) )
        {
            g_ConverseMgr.Stop( m_VoiceID );
        }

        m_CurrentDialogInfo.Clear();
        m_CurrentDialogInfo.SetData(dialogType, dialogName, animName ,animPkg, Flags, DialogFlags, BlendOutTime );

        // see if our anim has a begin dialog event... if so we do not want to start it yet... just warm it up.
        m_LipSyncStartAnimFrame = GetLipSyncEventStartFrame(animPkg,animName);

        if( hotVoice )
        {
            // next see if we have this dialog and try to play it.
            SetVoiceID( g_ConverseMgr.PlayHotVoice(dialogName, GetPosition(), GetZone1(), FALSE, DialogFlags) );        
        }
        else
        {
            // next see if we have this dialog and try to play it.
            SetVoiceID( g_ConverseMgr.PlayStream(dialogName, GetPosition(), GetGuid(), GetZone1(), IMMEDIATE_PLAY, FALSE, DialogFlags) );
            if( m_VoiceID == 0 )
            {
                //ASSERTS( 0, "Jason you need to queue up this dialog!" );
                m_VoiceID = g_AudioMgr.Play( "EGL_Fire" );
                g_AudioMgr.SetVolume( m_VoiceID, 0.0f );
            }
            s_DialogVoiceID = GetVoiceID();
        }
    }
    else
    {    
        // if we aren't allowed to play dialogs or we are doing the full body lip sync then return
        if( !m_AllowDialog || IsPlayingFullBodyLipSync() ) 
        {
            return FALSE;
        }

        // if we are currently playing a spiffy dialog voice, return.
        if ( g_ConverseMgr.IsPlaying( s_DialogVoiceID ) )
        {
            return FALSE;
        }

        // if there is a cinema going on, don't play.
        player* pPlayer = SMP_UTIL_GetActivePlayer();
        if ( pPlayer->IsCinemaRunning() )
        {
            return FALSE;
        }

        // build the character's name
        char characterName[32];

        // add prefix 
        x_strcpy( characterName,GetDialogPrefix() );

        // add prefered voice actor
        switch( m_PreferedVoiceActor )
        {
        case 0:
            x_strcat(characterName,"A");
            break;
        case 1:
            x_strcat(characterName,"B");
            break;
        case 2:
            x_strcat(characterName,"C");
            break;
        case 3:
            x_strcat(characterName,"D");
            break;
        default:
            x_strcat(characterName,"A");
            break;
        };

        // build the action's name
        char actionName[32];

        if( (dialogType <= DIALOG_TYPE_COUNT) && (dialogType >= DIALOG_NONE) )
        {
            f32 DeltaTime = g_ObjMgr.GetGameDeltaTime( g_DialogTweaks[dialogType].m_LastTime );

            LOG_MESSAGE( "character::PlayDialog", "DeltaTime: %f", DeltaTime ); 
            if( x_abs( DeltaTime ) < g_DialogTweaks[dialogType].m_Tweak.GetF32() )
            {
                LOG_MESSAGE( "character::PlayDialog", 
                    "Rejecting: %d", dialogType );
                return FALSE;
            }
        }

        switch( dialogType )    
        {
        case DIALOG_ALERT:
            x_strcpy(actionName,"ALERT");
            break;
        case DIALOG_RUSH:
            x_strcpy(actionName,"RUSH");
            break;
        case DIALOG_KILL:
            x_strcpy(actionName,"KILL");
            break;
        case DIALOG_CLEAR:
            x_strcpy(actionName,"CLEAR");
            break;
        case DIALOG_FLEE:
            x_strcpy(actionName,"FLEE");
            break;
        case DIALOG_FRIENDLY_HIT:
            x_strcpy(actionName,"FRIENDLYHIT");
            break;
        case DIALOG_FRIENDLY_WOUND:
            x_strcpy(actionName,"FRIENDLYWOUND");
            break;
        case DIALOG_GRENADE_THROW:
            x_strcpy(actionName,"GRENADETHROW");
            break;
        case DIALOG_GRENADE_SPOT:
            x_strcpy(actionName,"GRENADESPOT");
            break;
        case DIALOG_COVER:
            x_strcpy(actionName,"COVER");
            break;
        case DIALOG_COVER_REQ:
            x_strcpy(actionName,"COVERREQ");
            break;
        case DIALOG_MANDOWN:
            x_strcpy(actionName,"MANDOWN");
            break;
        case DIALOG_RELOAD:
            x_strcpy(actionName,"RELOAD");
            break;
        case DIALOG_UNDER_FIRE:
            x_strcpy(actionName,"UNDERFIRE");
            break;
        case DIALOG_HIT:
            x_strcpy(actionName,"HIT");
            break;
        case DIALOG_HIT_MELEE:
            x_strcpy(actionName,"HITMELEE");
            break;
        case DIALOG_FLINCH:
            x_strcpy(actionName,"FLINCH");
            break;
        case DIALOG_DIE_MELEE:
            x_strcpy(actionName,"DIEMELEE");
            break;
        case DIALOG_DIE_GUNFIRE:
            x_strcpy(actionName,"DIEGUNFIRE");
            break;
        case DIALOG_DIE_EXPLOSION:
            x_strcpy(actionName,"DIEEXPLOSION");
            break;
        case DIALOG_DIE_FALL:
            x_strcpy(actionName,"DIEFALL");
            break;
        };        

        // see if we have this dialog
        if( !g_ConverseMgr.HasDialog(characterName, actionName ) )
        {
            return FALSE;
        }

        // if we are playing a dialog, stop it
        if( g_ConverseMgr.IsActive(m_VoiceID) )
        {
            char Descriptor[MAX_DESCRIPTOR_NAME];
            x_sprintf( Descriptor, "%s_%s", characterName, actionName );

            if( g_AudioMgr.GetPriority(m_VoiceID) > g_AudioMgr.GetPriority(Descriptor) )
            {
                return FALSE;
            }
            else
            {
                g_ConverseMgr.Stop( m_VoiceID );
            }
        }

        m_CurrentDialogInfo.Clear();
        m_CurrentDialogInfo.SetData(dialogType, dialogName, animName ,animPkg, Flags, DialogFlags, BlendOutTime );

        // see if our anim has a begin dialog event... if so we do not want to start it yet... just warm it up.
        m_LipSyncStartAnimFrame = GetLipSyncEventStartFrame(animPkg,animName);

        if( hotVoice )
        {
            // next see if we have this dialog and try to play it.
            SetVoiceID(g_ConverseMgr.PlayHotVoice( characterName, actionName, GetPosition(), GetZone1(), 1.0f , FALSE));
        }
        else
        {        
            // next see if we have this dialog and try to play it.
            SetVoiceID(g_ConverseMgr.PlayStream( characterName, actionName, GetGuid(), GetZone1(), GetPosition(), IMMEDIATE_PLAY, FALSE ));
            if( m_VoiceID == 0 )
            {
//                ASSERTS( 0, "Jason you need to queue up this dialog!" );
                m_VoiceID = g_AudioMgr.Play( "EGL_Fire" );
                g_AudioMgr.SetVolume( m_VoiceID, 0.0f );
            }
        }
    }

    if( m_VoiceID == 0 )
    {    
        return FALSE;
    }
    else
    {            
        if( (dialogType <= DIALOG_TYPE_COUNT) && (dialogType >= DIALOG_NONE) )
        {
            LOG_MESSAGE( "character::PlayDialog", 
                "Reseting: %d",
                dialogType );
            g_DialogTweaks[dialogType].m_LastTime = CurrTime;
        }
        m_TimeSinceLastDialog = 0.0f;
        m_TimeWaitingForVoiceToStart = 0.0f;
        m_VoiceStarted = FALSE;
        m_CurrentDialogType = dialogType;
        return TRUE;
    }
}


//=============================================================================

static const s32 s_Num_Col_Iterations = 5;
static const f32 s_Col_Test_Mul =  ( 1.f / ( f32 ) s_Num_Col_Iterations ); 
static const f32 s_Dist_Per_Segment = 800.f;
static const f32 s_Dist_Per_Segment_Sqr = s_Dist_Per_Segment * s_Dist_Per_Segment;


//=============================================================================

// Returns TRUE if the character can Shoot At Target 
xbool character::UpdateCanShootAtTarget( )
{
    // You should only call when thinking!
    ASSERT(m_bThinking);
    m_FriendlyBlocksTarget = FALSE;
    xbool canShootAtTarget = FALSE;
    vector3 checkLocation = GetPositionWithOffset( OFFSET_EYES );
    vector3 vShootAt = GetShootAtPosition();

    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon && GetTargetNotSeenTimer() < k_MaxNotSeenToFire )
    {
        // do we have an aim point?
        vector3 FirePoint;
        if( pWeapon->GetFiringBonePosition(FirePoint) )
        {
            checkLocation = FirePoint;
            if( IsTargetInFiringCone( vShootAt ) )
            {
                canShootAtTarget = TRUE;
            }
        }
    }
    
    guid hitGuid = GetGuidBlockingLOS(GetGuid(),GetTargetGuid(),checkLocation,vShootAt,FALSE,TRUE);
    object *hitObject = g_ObjMgr.GetObjectByGuid( hitGuid );
    if( hitObject )
    {
        if( hitObject->IsKindOf(actor::GetRTTI()) )
        {
            actor &hitActor = actor::GetSafeType( *hitObject );
            if ( IsFriendlyFaction(hitActor.GetFaction()) )
            {
                m_FriendlyBlocksTarget = TRUE;
                canShootAtTarget = FALSE;
            }
        }
        else if( g_CollisionMgr.m_nCollisions != 0 )
        {
            // see if the point of collision is much closer to the target than 
            // to us. Otherwise is looks wierd. In addition make sure it is a 
            // min distance away or once again... looks wierd.

            vector3 toCollison = g_CollisionMgr.m_Collisions[0].Point - GetPositionWithOffset(OFFSET_EYES);
            if( toCollison.LengthSquared() < k_MinDistToObstacleToFireSqr ||
                toCollison.LengthSquared() < (GetToTarget().LengthSquared()/2.0f) )
            {
                canShootAtTarget = FALSE;
            }
        }
    }
    // made it through!
    return canShootAtTarget;
}


//=========================================================================

ng_node2 &character::GetClosestNode()
{
    return g_NavMap.GetNodeByID(g_NavMap.GetNearestNode( GetPosition() ));
}

//=========================================================================

xbool character::IsAlarmNodeValid( guid testNodeGuid )
{
    object *testNodeObject = g_ObjMgr.GetObjectByGuid(testNodeGuid);
    if( testNodeObject && testNodeObject->IsKindOf(alarm_node::GetRTTI()) )
    {
        alarm_node &testNode = alarm_node::GetSafeType( *testNodeObject );

        xbool retVal = TRUE;
        if( testNode.IsReserved(GetGuid()) )
        {
            retVal = FALSE;
        }
        else if( testNode.IsActive() )
        {
            retVal = FALSE;
        }
        else if( !testNode.IsFriendlyFaction(GetFaction()) )
        {
            retVal = FALSE;
        }
        else if( GetToTarget(testNodeGuid).LengthSquared() > k_MaxDistToAlarmNodeSqr )
        {
            retVal = FALSE;
        }
        else if( !UpdateCanReachTarget(testNode.GetPosition(),0.0f) )
        {
            retVal = FALSE;
        }
        return retVal;
    }
    return FALSE;
}

//=========================================================================

xbool character::IsCoverNodeValid( guid testNodeGuid )
{
    object *testNodeObject = g_ObjMgr.GetObjectByGuid(testNodeGuid);
    if( testNodeObject && testNodeObject->IsKindOf(cover_node::GetRTTI()) )
    {
        cover_node &testNode = cover_node::GetSafeType( *testNodeObject );

        inven_item WeaponItem = INVEN_NULL;
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if( pWeapon )
        {
            WeaponItem = pWeapon->GetInvenItem();
        }

        xbool retVal = TRUE;
        // if it's not our sticky and is inactive then not valid
        if( testNodeGuid != GetStickyCoverNode() &&
            !testNode.IsActive() )
        {
            retVal = FALSE;
        }
        // if reserved then not valid
        else if( testNode.IsReserved(GetGuid()) )
        {
            retVal = FALSE;
        }
        // if I can't see my target for a while and I'm not fleeing then not valid
        // removing this to see what happens, this will allow NPCs that can't see you 
        // ( but that you can see) to remain in cover and scan when shot at.
/*        else if( GetTargetGuid() && GetTargetNotSeenTimer() >= k_MinTimeNotSeenCoverValid && 
                 !HasState(character_state::STATE_FLEE) )
        {
            retVal = FALSE;
        }*/
        // if my cover is too far then not valid
        else if( GetToTarget(testNodeGuid).LengthSquared() > m_MaxDistToCover * m_MaxDistToCover )
        {
            retVal = FALSE;
        }
        // if it doesn't have the correct anims, then not valid
        else if( !testNode.HasValidAnims2(GetType(), GetLogicalName(), WeaponItem) )
        {
            retVal = FALSE;
        }
        // if this cover does not cover my target, then not valid
        else if( GetTargetGuid() && 
                 !testNode.IsCoverFromLocation(GetTargetPosWithOffset(GetTargetGuid(),OFFSET_CENTER)) )
        {
            retVal = FALSE;
        }       
        // if I can't reach it, then not valid
        else if( !UpdateCanReachTarget(testNode.GetPosition(),0.0f) )
        {
            retVal = FALSE;
        }
        return retVal;    
    }
    return FALSE;
}

//=========================================================================

guid character::FindNearestValidCover( eCoverPreference coverPref, xbool checkCurrent )
{
    //
    // WARNING!!!
    //
    // If this routine changes then IsCoverNodeValid() must
    // change as well.
    //
    cover_node* pBestCoverNode = NULL;
    cover_node* pFirstCoverNode = NULL;
    m_TimeSinceLastCoverCheck = 0.0f;
    //
    // Look up weapon inventory type
    //
    inven_item WeaponItem = INVEN_NULL;
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon )
    {
        WeaponItem = pWeapon->GetInvenItem();
    }

    //
    // Loop through all cover nodes in the level
    //
    const s32 kMAX_CONTACTS = 256;
    priority_queue<guid,f32,kMAX_CONTACTS> coverNodeQueue;
    s32 contactCount = 0;

    // we will use the first node if it's from a sticky group.
    xbool useFirstNode = FALSE;

    object *stickyObject = g_ObjMgr.GetObjectByGuid( m_StickyCoverNode );
    if( stickyObject && stickyObject->IsKindOf(group::GetRTTI()) )
    {
        group &stickyGroup = group::GetSafeType(*stickyObject);
        // cycle only through these... 
        s32 c=0;
        for(c=0;c<stickyGroup.GetNumChildren(object::TYPE_COVER_NODE)&&contactCount<kMAX_CONTACTS;c++)
        {
            useFirstNode = TRUE;
            // we always want one of these nodes, it is sticky. 
            object *pCoverNode = g_ObjMgr.GetObjectByGuid( stickyGroup.GetChild(c,object::TYPE_COVER_NODE) );
            if( pCoverNode && pCoverNode->IsKindOf(cover_node::GetRTTI()) )
            {
                f32 yDiff = x_abs( (GetPosition().GetY() - pCoverNode->GetPosition().GetY()) );
                f32 prefDist = ( GetPosition() - pCoverNode->GetPosition() ).LengthSquared();
                // prefer things on our same plane.
                prefDist += yDiff*3.0f;
                switch( coverPref )
                {
                case COVER_PREF_CLOSER_TO_TARGET:
                    // prefer one's clser to target (but not as important as close to us)
                    prefDist += ( (GetTargetPosWithOffset() - pCoverNode->GetPosition() ).LengthSquared()/4.0f) ;
                    break;
                case COVER_PREF_FURTHER_FROM_TARGET:
                    prefDist -= ( (GetTargetPosWithOffset() - pCoverNode->GetPosition() ).LengthSquared()/4.0f);
                    break;
                case COVER_PREF_CLOSER_TO_US:
                    break;
                }
                coverNodeQueue.Push( pCoverNode->GetGuid(),prefDist );
                contactCount++;
            }
        }
    }
    else
    {
        slot_id CoverSlot = g_ObjMgr.GetFirst( object::TYPE_COVER_NODE );
        while ( CoverSlot != SLOT_NULL && contactCount < kMAX_CONTACTS)
        {
            cover_node* pCoverNode = ( cover_node* )g_ObjMgr.GetObjectBySlot( CoverSlot );
            ASSERT( pCoverNode );

            f32 yDiff = x_abs( (GetPosition().GetY() - pCoverNode->GetPosition().GetY()) );
            f32 prefDist = ( GetPosition() - pCoverNode->GetPosition() ).LengthSquared();
            // prefer things on our same plane.
            prefDist += yDiff*3.0f;
            switch( coverPref )
            {
            case COVER_PREF_CLOSER_TO_TARGET:
                // prefer one's clser to target (but not as important as close to us)
                prefDist += ( (GetTargetPosWithOffset() - pCoverNode->GetPosition() ).LengthSquared()/4.0f) ;
                break;
            case COVER_PREF_FURTHER_FROM_TARGET:
                prefDist -= ( (GetTargetPosWithOffset() - pCoverNode->GetPosition() ).LengthSquared()/4.0f);
                break;
            case COVER_PREF_CLOSER_TO_US:
                //            prefDist = ( GetPosition() - pCoverNode->GetPosition() ).LengthSquared();
                break;
            }

            coverNodeQueue.Push( pCoverNode->GetGuid(),prefDist );
            contactCount++;
            CoverSlot = g_ObjMgr.GetNext( CoverSlot );
        }
    }

    xbool validNodeFound = FALSE;

    while( !validNodeFound && !coverNodeQueue.IsEmpty() )
    {
        // Lookup cover node
        guid coverGuid = coverNodeQueue.Pop();
        cover_node* pCoverNode = ( cover_node* )g_ObjMgr.GetObjectByGuid( coverGuid );
        if( pFirstCoverNode == NULL &&
            !pCoverNode->IsReserved(GetGuid()) )
        {
            pFirstCoverNode = pCoverNode;
        }
        

        // Is this cover node valid?
        if( pCoverNode && 
            IsCoverNodeValid( pCoverNode->GetGuid()) )
        {

            // if we only want covers ahead, then we need to check it here.
            xbool coverAheadValid = TRUE;
            if( m_bOnlyUsesCoverAhead )
            {
                vector3 toTarget = GetToTarget();
                vector3 toNode = GetToTarget( pCoverNode->GetGuid() );
                radian angleDiff = x_abs(x_MinAngleDiff(toTarget.GetYaw(),toNode.GetYaw()));
                if( angleDiff >= R_30 )
                {
                    coverAheadValid = FALSE;
                }
            }

            // Only if we aren't at this ocver or we are checking the current cover...
            if( (pCoverNode->GetGuid() != GetCurrentCover() ||
                 checkCurrent) &&
                 coverAheadValid )
            {
                validNodeFound = TRUE;
                pBestCoverNode = pCoverNode;
            }                
        }
    }
        
    // Did we find a cover?    
    if( pBestCoverNode )
    {
        return pBestCoverNode->GetGuid();
    }
    else
    {
        if( useFirstNode &&
            pFirstCoverNode )
        {
            return pFirstCoverNode->GetGuid();
        }
        else
        {        
            return 0;
        }
    }
}

//=========================================================================

guid character::FindNearestAlarm()
{
    alarm_node* pBestAlarmNode = NULL;
    //
    // Loop through all alarm nodes in the level
    //
    const s32 kMAX_CONTACTS = 256;
    priority_queue<slot_id,f32,kMAX_CONTACTS> slotQueue;
    s32 contactCount = 0;

    slot_id AlarmSlot = g_ObjMgr.GetFirst( object::TYPE_ALARM_NODE );
    while ( AlarmSlot != SLOT_NULL && contactCount < kMAX_CONTACTS)
    {
        alarm_node* pAlarmNode = ( alarm_node* )g_ObjMgr.GetObjectBySlot( AlarmSlot );
        ASSERT( pAlarmNode );
        f32 DistToAlarmNode = (pAlarmNode->GetPosition() - GetPosition()).LengthSquared();
        slotQueue.Push( AlarmSlot,DistToAlarmNode );

        contactCount++;
        AlarmSlot = g_ObjMgr.GetNext( AlarmSlot );
    }

    xbool validNodeFound = FALSE;
    while( !validNodeFound && !slotQueue.IsEmpty() )
    {
        AlarmSlot = slotQueue.Pop();
        alarm_node* pAlarmNode = ( alarm_node* )g_ObjMgr.GetObjectBySlot( AlarmSlot );
        if( IsAlarmNodeValid(pAlarmNode->GetGuid()) )
        {
            validNodeFound = TRUE;
            pBestAlarmNode = pAlarmNode;
        }
    }

    if( pBestAlarmNode )
    {
        return pBestAlarmNode->GetGuid();
    }
    else
    {
        return 0;
    }
}

//=========================================================================
#ifndef X_RETAIL
void character::AddToStateChangeList( const char *newText )
{
    x_strcpy(m_StateChangeList[3],m_StateChangeList[2]);
    x_strcpy(m_StateChangeList[2],m_StateChangeList[1]);
    x_strcpy(m_StateChangeList[1],m_StateChangeList[0]);
    x_strcpy(m_StateChangeList[0],newText);
}

//=========================================================================

void character::AddToPhaseChangeList( const char *newText )
{
    x_strcpy(m_PhaseChangeList[3],m_PhaseChangeList[2]);
    x_strcpy(m_PhaseChangeList[2],m_PhaseChangeList[1]);
    x_strcpy(m_PhaseChangeList[1],m_PhaseChangeList[0]);
    x_strcpy(m_PhaseChangeList[0],newText);
}

//=========================================================================

void character::AddToGoalChangeList( const char *newText )
{
    x_strcpy(m_GoalChangeList[3],m_GoalChangeList[2]);
    x_strcpy(m_GoalChangeList[2],m_GoalChangeList[1]);
    x_strcpy(m_GoalChangeList[1],m_GoalChangeList[0]);
    x_strcpy(m_GoalChangeList[0],newText);
}
#endif

//=========================================================================

void character::UpdateTargetData()
{
    object* pObject = g_ObjMgr.GetObjectByGuid( GetTargetGuid() );
    if( pObject )
    {
        m_LastKnownLocationOfTarget = GetTargetPosWithOffset( GetTargetGuid(), OFFSET_EYES );
        m_CanShootAtTarget = UpdateCanShootAtTarget();       
        if( CanSeeObject(GetTargetGuid(),TRUE) )
        {
            m_CanSeeTarget = TRUE;
            m_bTargetSeen = TRUE;
            m_TargetNotSeenTimer = 0.0f;
            m_LastLocationOfInterest = GetTargetPosWithOffset( GetTargetGuid(),OFFSET_EYES );
            m_LastSeenLocationOfTarget = m_LastLocationOfInterest;
        }
    }
}

//=========================================================================

void character::ChoseBestTarget()
{
    if( !m_pActiveState )
        return;

    const s32 kMAX_CONTACTS = 256;
    priority_queue<object *,f32,kMAX_CONTACTS> targetQueue;
    guid bestTarget = GetTargetGuid();
    actor* pActor = actor::m_pFirstActive;
    s32 contactCount = 0;
    s32 actorCount = 0;

    while( contactCount < kMAX_CONTACTS && actorCount < actor::m_nActive && pActor )
    {
        // Skip self
        if(pActor != this)
        {
            f32 DistToActor = (pActor->GetPosition() - GetPosition()).LengthSquared();


            // weight according to the targetting of others.
            god* pGod = SMP_UTIL_Get_God();
            god::TargettingData tempTargetData( GetGuid(), pActor->GetGuid(), DistToActor );
            s32 numTargetting = pGod->GetNumTargettingCloser( tempTargetData );

            // add k_OtherTargettedPercent per targetter closer. 
            DistToActor *= ( (1.0f + (k_OtherTargettedPercent * numTargetting)) * (1.0f + (k_OtherTargettedPercent * numTargetting)) );

            // prefer players....
            if( pActor->IsPlayer() )
            {
                DistToActor *= ( k_PlayerPrefPercent * k_PlayerPrefPercent );
            }

            // prefer your current target
            if( pActor->GetGuid() == m_TargetGuid )
            {
                DistToActor *= (k_CurrTargetPrefPercent * k_CurrTargetPrefPercent );
            }


            // quick tests
            if( IsEnemyFaction(pActor->GetFaction()) &&             
                IsValidTarget(pActor->GetGuid()) )
            {
                contactCount++;
                targetQueue.Push( pActor,DistToActor );
            }
        }

        // Check next
        actorCount++;
        pActor = pActor->m_pNextActive;
    }

    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_TURRET );
    while( contactCount < kMAX_CONTACTS && SlotID != SLOT_NULL )
    {    
        object* tempObject = g_ObjMgr.GetObjectBySlot(SlotID);
        if(tempObject && tempObject->IsKindOf(turret::GetRTTI()) )
        {
            turret &tempTurret = turret::GetSafeType( *tempObject );        
            f32 DistToTurret = (tempTurret.GetPosition() - GetPosition()).LengthSquared();
            // quick tests
            if( IsEnemyFaction(tempTurret.GetFaction()) && 
                IsValidTarget(tempTurret.GetGuid()) )
            {
                targetQueue.Push( tempObject,DistToTurret );
                contactCount++;
            }
        }
        // Check next
        SlotID = g_ObjMgr.GetNext(SlotID);
    }
    
    if( CanTargetGlobs() )
    {    
        slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_ALIEN_GLOB );
        while( contactCount < kMAX_CONTACTS && SlotID != SLOT_NULL )
        {   
            object* tempObject = g_ObjMgr.GetObjectBySlot(SlotID);
            if( tempObject )
            {
                f32 DistToGlob = (tempObject->GetPosition() - GetPosition()).LengthSquared();
                if( IsValidTarget(tempObject->GetGuid()) )
                {                
                    targetQueue.Push( tempObject,DistToGlob );
                    contactCount++;
                }
            }
            // Check next
            SlotID = g_ObjMgr.GetNext(SlotID);
        }
    }

    object *pObject = NULL;
    xbool validTargetFound = FALSE;
    while( !validTargetFound && !targetQueue.IsEmpty() )
    {        
        pObject = targetQueue.Pop();
        f32 distToTarget = GetToTarget(pObject->GetGuid()).LengthSquared();
        if( distToTarget < k_AlertSmellDist * k_AlertSmellDist ) 
        {
            bestTarget = pObject->GetGuid();
            validTargetFound = TRUE;
        }
        else if( GetAwarenessLevel() >= AWARENESS_TARGET_LOST && 
              distToTarget < k_AttackSmellDist * k_AttackSmellDist)
        {
            bestTarget = pObject->GetGuid();
            validTargetFound = TRUE;
        }
        // we keep our target as viable for a while after not seeing him
        else if( GetAwarenessLevel() >= AWARENESS_COMBAT_READY && 
                 ( CanSeeObject(pObject->GetGuid(),TRUE) ||
                   ( pObject->GetGuid() == GetTargetGuid() &&
                     GetTargetNotSeenTimer() <= k_MinTimeNotSeenToKeepTarget) ) )
        {
            bestTarget = pObject->GetGuid();
            validTargetFound = TRUE;
        }
        // this is causing NPCs to pop into the world and not be able to see or attack the player for a while...
        // what is it's purpose?
        else if ( /*GetTargetNotSeenTimer() > 5.0f && */
                  CanSeeObject(pObject->GetGuid()) )
        {
            bestTarget = pObject->GetGuid();
            validTargetFound = TRUE;
        }
    }
    if( bestTarget != m_TargetGuid )
    {    
        SetTargetGuid( bestTarget );
    }
}

//=========================================================================

// Updates the current target to attack
// Returns TRUE if there is a target that can be seen
// (FALSE doesn't neccessarily mean there isn't a target - check "m_TargetObjectGuid" to be sure)
//static const f32 s_Max_No_See_Time = 5.0f;

void character::UpdateTarget( )
{
    // You should only call when thinking!
    ASSERT(m_bThinking);
    m_CanSeeTarget = FALSE;
    m_CanShootAtTarget = FALSE;

    // first... do we have an override?
    object *pObject = g_ObjMgr.GetObjectByGuid(m_OverrideTargetGuid);
    if( pObject && pObject->IsAlive() )
    {
        SetTargetGuid( pObject->GetGuid() );
        UpdateTargetData();
        return;
    }
    else
    {
        m_OverrideTargetGuid = 0;
    }

    // is our current target valid?
    pObject = g_ObjMgr.GetObjectByGuid( GetTargetGuid() ) ;
    if( m_TargetGuid != 0 && 
        m_TargetGuid != m_OverrideTargetGuid &&
        !IsValidTarget( GetTargetGuid() ) )//ensure this is valid (target not destroyed)
    {
        SetTargetGuid(0);
    }

    if( !GetTargetGuid() ||
        m_TimeSinceLastTargetUpdate >= k_MinTimeBetweenTargetUpdates )
    {
        ChoseBestTarget();
    }

    UpdateTargetData();
}


//=========================================================================

void character::SetTargetGuid( guid Guid, xbool shoutAboutTarget )
{
    // don't target self, no need to target what we are already targetting
    if( Guid == GetGuid() || Guid == GetTargetGuid() ) 
    {    
        return;
    }

    // if we have an override target and this ain't it, ignore.
    object *overrideObject = g_ObjMgr.GetObjectByGuid(m_OverrideTargetGuid);
    if( Guid != m_OverrideTargetGuid && overrideObject && overrideObject->IsAlive() )
    {
        return;
    }

    // make sure this is a valid target
    if( Guid != 0 && 
        Guid != m_OverrideTargetGuid &&
        !IsValidTarget(Guid) )
    {
        return;
    }

    m_TargetGuid = Guid;
    object* pObject = g_ObjMgr.GetObjectByGuid(m_TargetGuid);
    if( pObject )
    {
        m_LastLocationOfInterest    = GetTargetPosWithOffset( m_TargetGuid, OFFSET_EYES );
        m_LastKnownLocationOfTarget = GetTargetPosWithOffset( m_TargetGuid, OFFSET_EYES );
        m_LastSeenLocationOfTarget  = GetTargetPosWithOffset( m_TargetGuid, OFFSET_EYES );
        m_bTargetSeen = FALSE;
        m_TargetNotSeenTimer = 0.0f;
        m_CanPathToTarget = CanPathTo( pObject->GetPosition() );
        if( shoutAboutTarget )
        {        
            SendAlert( alert_package::ALERT_TYPE_NPC_SHOUT, m_TargetGuid );
//            Shout( m_TargetGuid );
        }
    }
    else
    {
        m_TargetGuid = 0;
    }
}

//=========================================================================

xbool character::IsTargetInFiringCone( const vector3& Target )
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    vector3 FirePoint, AimPoint;

    if( !pWeapon )
        return FALSE;
                                                                           
    // must have fire point and aim point;
    if( !pWeapon->GetFiringBonePosition(FirePoint) ||
        !pWeapon->GetAimBonePosition(AimPoint) )
    {
        return FALSE;
    }

    vector3 TargetAngle = Target - GetPositionWithOffset(OFFSET_EYES);
    vector3 AimAngle = FirePoint - AimPoint;

    TargetAngle.Normalize();
    AimAngle.Normalize();

    radian AngleToCheck = v3_AngleBetween(TargetAngle, AimAngle);
    return ( AngleToCheck < m_WeaponFireAngle );
}

//=========================================================================

vector3 character::GetToTarget( guid target /* = NULL  */, eOffsetPos offset )
{
    return ( GetTargetPosWithOffset(target,offset) - GetPositionWithOffset(offset) );
}

//=========================================================================

xbool character::IsNewTargetCloser( guid Guid )
{
    object* pNewTarget = g_ObjMgr.GetObjectByGuid(Guid);
    if ( pNewTarget ) 
    {               
        if ( GetTargetGuid() == 0 ||
             GetToTarget().LengthSquared() > GetToTarget(Guid).LengthSquared() )
        {
            return TRUE;
        }
    }
    return FALSE;
}

//=========================================================================
const char* character::GetDialogPrefix( void )
{
    if (m_DialogPrefixString >= 0)
        return g_StringMgr.GetString( m_DialogPrefixString );
    
    return "";
}

//=========================================================================
void character::NotifyScanBegin( void )
{
    char pIdentifier[256] = {'\0'};

    // Identifier for voice responses.
    const char* pScanIdentifier = GetScanIdentifier();
   
    x_sprintf(pIdentifier, "%s_ANVO_%02d", pScanIdentifier, m_ScannerVOIndex );

    // play dialog
    PlayDialog( DIALOG_SCANNER_VO, pIdentifier);
}

//=========================================================================
void character::NotifyScanEnd( void )
{
}

//=========================================================================
f32 g_RecentlyInCombat = 10.0f;
xbool character::WasRecentlyInCombat( void )
{
    f32 TSLC_Time = GetTimeSinceLastCombat();

    if( TSLC_Time <= g_RecentlyInCombat )
    {
        return TRUE;
    }

    return FALSE;
}

//=========================================================================
const char* character::EnumToName( eNPCName NPC )
{
    ASSERT( (NPC >= 0) && (NPC < NPC_MAX) );
    const char* pName = s_NPCTable[NPC].pName;
    CLOG_MESSAGE( NPC_LOGGING, "character::EnumToName", "%d = '%s'", NPC, pName );
    return pName;
}

//=========================================================================
eNPCName character::GetIDFromSubtype( void )
{   
    switch( soldier::eCharacterSubtypes( m_Subtype ) ) 
    {
    case soldier::SUBTYPE_BLACKOP_LEADER:
        {
            return NPC_BLACKOP_LEADER;
        }
        break;
    case soldier::SUBTYPE_BLACKOPS:
        {
            return NPC_BLACKOPS;
        }
        break;
    case soldier::SUBTYPE_SPEC4:
        {
            return NPC_GENERIC;
        }
        break;
    case soldier::SUBTYPE_HAZMAT:
        {
            const char* pName = GetLogicalName();
            if( x_stristr(pName,"Grunt") )
                return NPC_MUTANT;
            else
                return NPC_GENERIC;
        }
        break;
    }
     
    return NPC_UNKNOWN;
}

//=========================================================================
const char* character::GetScanIdentifier( void )
{
    eNPCName NPC = m_NPCName;
    ASSERT( (NPC >= 0) && (NPC < NPC_MAX) );

    if( NPC == NPC_GENERIC )
    {
        if( m_Subtype != -1 )
        {
            // black ops, grunts, etc.
            NPC = GetIDFromSubtype();
        }
        else
        if( this->IsKindOf(mutant_tank::GetRTTI()) )
        {
            NPC = NPC_THETA;
        }
        else
        if( this->IsKindOf(alien_glob::GetRTTI()))
        {
            NPC = NPC_ALIENGLOB;
        }
        else
        if( this->IsKindOf(friendly_scientist::GetRTTI()))
        {
            NPC = NPC_SCIENTIST;
        }
    }

    const char* pIdentifier = s_NPCTable[NPC].pIdentifier;
    CLOG_MESSAGE( NPC_LOGGING, "character::GetScanIdentifier", "%d = '%s'", NPC, pIdentifier );
    return pIdentifier;
}

//=========================================================================
xstring character::GetEnumStringNPCs( void )
{
    xstring EnumString;

    for( s32 i=0 ; i<NUM_NPC_TABLE_ENTRIES ; i++ )
    {
        EnumString += EnumToName( (eNPCName)i );
        EnumString += '\0';
    }

    return EnumString;
}

//=========================================================================
eNPCName character::NameToEnum( const char* pName )
{
    // Search the table for the npc name
    for( s32 i=0 ; i<NUM_NPC_TABLE_ENTRIES ; i++ )
    {
        if( x_strcmp( pName, s_NPCTable[i].pName ) == 0 )
        {
            ASSERT( i == s_NPCTable[i].Item );
            CLOG_MESSAGE( NPC_LOGGING, "character::NameToEnum", "Found '%s' = %d", pName, i );
            return (eNPCName)i;
        }
    }

    // Not found so NULL
    CLOG_ERROR( NPC_LOGGING, "character::NameToEnum", " Not Found '%s'", pName );
    return NPC_GENERIC;
}

//=========================================================================
// EDITOR FUNCTIONS
//=========================================================================

void character::OnEnumProp ( prop_enum& List )
{
    actor::OnEnumProp(List);
    // Header
    List.PropEnumHeader( "Character", "Character is the base class for all NPCs in the game", 0 );
    
    // NPCs name
    List.PropEnumEnum  ( "Character\\NPCName",  GetEnumStringNPCs(),  "Which NPC is this (Generic for NPCs with no name).", PROP_TYPE_EXPOSE );
    
    //Rendering BBox
    List.PropEnumVector3("Character\\Render BBox", "Bounding Box used for rendering.", PROP_TYPE_MUST_ENUM );
    // SkinGeometry
    m_SkinInst.OnEnumProp(List);
    // Animation file
    List.PropEnumExternal("RenderInst\\Anim", "Resource\0anim\0", "Resource File", PROP_TYPE_MUST_ENUM);
    // Animfile when weapon dropped.
    List.PropEnumExternal("RenderInst\\Weaponless Anim", "Resource\0anim\0", "Resource File", PROP_TYPE_MUST_ENUM);
    // Held Weapon
    List.PropEnumEnum    ( "Character\\Weapon",  inventory2::GetEnumStringWeapons(),  "Weapon", PROP_TYPE_EXPOSE );
    // Held Grenade
    List.PropEnumEnum    ( "Character\\Grenade", inventory2::GetEnumStringGrenades(), "Grenades", PROP_TYPE_EXPOSE );
    // Audio resource.
    List.PropEnumExternal(   "Character\\Audio Package", "Resource\0audiopkg\0", "The audio package associated with this character.", 0 );
    // Dialog String
    List.PropEnumString(     "Character\\Dialog Prefix", "Prefix string used when playing audio", 0 );
    // Overide which Dialog actor to use
    List.PropEnumEnum(     "Character\\Override Dialog Actor", character::m_DialogActorsEnum.BuildString(), "Sets which actor to use, leave as none and it choses randomly", PROP_TYPE_EXPOSE );
    // Logical Name
    List.PropEnumString(     "Character\\Logical Name", "Our logical name", 0 );
    // Health
    List.PropEnumFloat(      "Character\\Health",  "Health is float value representing the health percentage (0-100) of the character", PROP_TYPE_EXPOSE );
    // do we start active?
    List.PropEnumBool (      "Character\\Start Active", "Is the character initially active or in hold state.", 0 );


    // Header
    List.PropEnumHeader(     "Character\\Flags", "Various Character bools", 0 );
    // Take no pain.        
    List.PropEnumBool(       "Character\\Flags\\Ignore Pain", "If TRUE we take no pain, though we may still flinch and throw blood about?", PROP_TYPE_EXPOSE );
    // Never flinch 
    List.PropEnumBool(       "Character\\Flags\\Ignore Flinches", "If TRUE we will not flinch, though we may still take pain and throw blood about?", PROP_TYPE_EXPOSE );
    // Can we use alarms?
    List.PropEnumBool (      "Character\\Flags\\Can Use Alarms", "Can I use alarms?",PROP_TYPE_EXPOSE);
    // Can I cloak?
    List.PropEnumBool (      "Character\\Flags\\Can Cloak", "Am I able to cloak?",PROP_TYPE_EXPOSE);
    
    List.PropEnumBool (      "Character\\Flags\\Must Cloak", "Am I forced to cloak?",PROP_TYPE_EXPOSE);

    List.PropEnumBool (      "Character\\Flags\\Combat Ready", "Are we preped for combat?",PROP_TYPE_EXPOSE);

    List.PropEnumBool (      "Character\\Flags\\Never Combat Ready", "Are we never preped for combat?",PROP_TYPE_EXPOSE);

    List.PropEnumBool (      "Character\\Flags\\Bodies Fade", "Do our dead bodies fade?",PROP_TYPE_EXPOSE);

    List.PropEnumBool (      "Character\\Flags\\Pass On Guid To Dead Body", "If TRUE, dead body object will inherit guid of character upon death.", PROP_TYPE_EXPOSE);

    List.PropEnumBool (      "Character\\Flags\\AutoRagdoll", "If set to true we die and ragdoll",PROP_TYPE_EXPOSE | PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_EXPORT );

    List.PropEnumBool (      "Character\\Flags\\Force Weapon Fire", "Are we going to fire the weapon reguardless of where we are aiming?",PROP_TYPE_EXPOSE );

    List.PropEnumBool       ( "Character\\Flags\\Root to position and idle", "Ignore all movement and stay in idle?", PROP_TYPE_EXPOSE );

    List.PropEnumBool(       "Character\\Flags\\Next Shot Must Hit", "If true then the next fired shot will be a hit bullet", PROP_TYPE_EXPOSE );

    List.PropEnumBool(       "Character\\Flags\\Reload Now", "If true we will reload.", PROP_TYPE_EXPOSE );

    List.PropEnumBool(       "Character\\Flags\\Never Play Reload Anim", "If TRUE we won't play the reload anim, we'll just reload.", PROP_TYPE_EXPOSE );

    List.PropEnumBool(       "Character\\Flags\\Dumb and Fast", "If true we cut out a lot of logic for speed, only set if you know all this means.", PROP_TYPE_EXPOSE );

    List.PropEnumBool(       "Character\\Flags\\Run Logic", "If false, this charcter runs no logic.", PROP_TYPE_EXPOSE );

    List.PropEnumBool(       "Character\\Flags\\Render", "If false character will not render.", PROP_TYPE_EXPOSE );

    List.PropEnumBool(       "Character\\Flags\\Is Alert", "True if character is alert.", PROP_TYPE_EXPOSE | PROP_TYPE_READ_ONLY );

    List.PropEnumBool(       "Character\\Flags\\Is Attacking", "True if character is attacking.", PROP_TYPE_EXPOSE | PROP_TYPE_READ_ONLY );

    List.PropEnumBool(       "Character\\Flags\\Can Always Backpeddle", "True NPCs will never turn their back when they want to face you.", PROP_TYPE_EXPOSE | PROP_TYPE_READ_ONLY );

    List.PropEnumHeader(    "Character\\Tweaks",  "Tweak data coming from the tweak table", 0 );

    List.PropEnumFloat(     "Character\\Tweaks\\Shoot Delay", "Min time between bursts", PROP_TYPE_EXPOSE ) ;

    List.PropEnumInt(       "Character\\Tweaks\\Accuracy", "How accurate is the AI with its weapons? 0-100", PROP_TYPE_EXPOSE | PROP_TYPE_DONT_SHOW | PROP_TYPE_DONT_EXPORT | PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_SAVE_MEMCARD );

    List.PropEnumInt(       "Character\\Tweaks\\Moving Target Accuracy", "How accurate is the AI with its weapons vs a running target? 0-100", PROP_TYPE_EXPOSE | PROP_TYPE_DONT_SHOW | PROP_TYPE_DONT_EXPORT | PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_SAVE_MEMCARD );

    List.PropEnumInt(       "Character\\Tweaks\\Weapon Drop Percent", "Deprecated", PROP_TYPE_EXPOSE );

    List.PropEnumInt(       "Character\\Tweaks\\Inventory Drop Percent", "Deprecated", PROP_TYPE_EXPOSE );

    List.PropEnumBool(       "Character\\Tweaks\\Drop Weapons", "Ok to drop our weapon?", PROP_TYPE_EXPOSE );

    List.PropEnumBool(       "Character\\Tweaks\\Drop Ammo", "Ok to drop ammo?", PROP_TYPE_EXPOSE );

    List.PropEnumBool(       "Character\\Tweaks\\Drop Grenades", "Ok to drop grenades?", PROP_TYPE_EXPOSE );

    List.PropEnumBool(       "Character\\Tweaks\\Drop Inventory", "Ok to drop our other inventory?", PROP_TYPE_EXPOSE );

    List.PropEnumFloat(     "Character\\Tweaks\\Light Sight Radius", "Maximum distance AI can see a target that is in the light", PROP_TYPE_EXPOSE );

    List.PropEnumFloat(     "Character\\Tweaks\\Dark Sight Radius", "Maximum distance AI can see a target that is in the dark", PROP_TYPE_EXPOSE );

    List.PropEnumFloat(     "Character\\Tweaks\\Sound Radius", "Maximum distance AI can hear, in cm" , PROP_TYPE_EXPOSE);
    
    List.PropEnumBool(       "Character\\Allow Play Dialog", "Should we ever play a dialog line.", 0 );

    List.PropEnumFloat(      "Character\\Max Cover Dist",
                                "Maximum distance AI will go to a cover node" , PROP_TYPE_EXPOSE);

    List.PropEnumGuid(       "Character\\Sticky Cover Node",
                                "The Cover Node we will stay at if at all possible", PROP_TYPE_EXPOSE );

    List.PropEnumGuid(       "Character\\Sticky Turret",
                                "Turret we will stay at if at all possible", PROP_TYPE_EXPOSE );

    List.PropEnumGuid(       "Character\\Activate On Death",
                                "Call activate on this object when we die.", PROP_TYPE_EXPOSE );

    // Follow Target
    m_FollowTargetAffector.OnEnumProp( List, "Character\\Follow Target", PROP_TYPE_EXPOSE );

    // Override Target
    m_OverrideTargetAffector.OnEnumProp( List, "Character\\Override Target", PROP_TYPE_EXPOSE );

    List.PropEnumGuid       ( "Character\\AimAt Guid",  "If no target and shooting, we'll try to hit this.", PROP_TYPE_EXPOSE );

    List.PropEnumBool       ( "Character\\Log AI State Changes", "Should we send info on this characters state changes to the log", PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_EXPORT );
    
    List.PropEnumGuid       ( "Character\\Leash Guid", "Object used as the leash anchor.", PROP_TYPE_EXPOSE );

    List.PropEnumFloat      ( "Character\\Leash Distance", "How far away from the Leash Guid that the character is allowed to navigate.", PROP_TYPE_EXPOSE );

    List.PropEnumEnum (       "Character\\Current State", GetStatesEnum(),
                                "AI's current state", PROP_TYPE_READ_ONLY | PROP_TYPE_EXPOSE );

    List.PropEnumHeader(     "Character\\Pathing",  
                                "Pathing controls for this NPC", 0 
                        );

    List.PropEnumBool (      "Character\\Pathing\\Use Small Paths",
                                "Can this NPC use paths with the SmallNPC flag enabled?", 0 );
    
    List.PropEnumBool (      "Character\\Pathing\\Use Navigation Map",
                                "If TRUE the npc will use the navigation map. If FALSE, it is always assumed the NPC can get to its target (used by Theta bosses)", 0 );

    List.PropEnumGuid(       "Character\\Override Idle Lookat",
                                "We will look at this when nothing else of interest to look at", PROP_TYPE_EXPOSE );

    List.PropEnumBool (      "Character\\Ignore Lookat Interest",
                                "NPC will ignore any interest objects except override interest", 0 );

    // States
    character_state* pState = m_pStateList;
    while( pState && pState->GetStateType() != character_state::STATE_NULL )
    {
        // Enum
        pState->OnEnumProp(List);

        // Do next
        pState = pState->m_pNext;
    }

    // Factions
    List.PropEnumHeader("Factions", "General Faction Info", 0 );
    s32 ID = List.PushPath( "Factions\\" );
    factions_manager::OnEnumProp( List );
    List.PopPath( ID );

    
}

//=============================================================================

void character::SetAnimPackage(  const char* animPackageName )
{   
    m_hAnimGroup.SetName( animPackageName );
    if (m_hAnimGroup.GetPointer())
    {
        // Tell loco
        GetLocoPointer()->OnInit( m_SkinInst.GetGeom(), m_hAnimGroup.GetName(), GetGuid() );

        SetupShoulderLight();

        // Init the character again now the loco is ready
        OnInit();
        InitPathingHints();
    }
}

//=============================================================================

xbool character::OnProperty ( prop_query& I )
{
    // Call base class
    if ( actor::OnProperty( I ) )
    {
        // Initialize the current nav-node for the character.
        if( I.IsVar( "Base\\Position" ) )
        {
            if (!I.IsRead())
            {
                m_InitialPosition = I.GetVarVector3();
            }
        }

        return TRUE;
    }
    // Geometry
    else if (m_SkinInst.OnProperty(I))
    {
        return TRUE;
    }
    else if ( GetLocoPointer() && GetLocoPointer()->OnProperty(I) )
    {
        return TRUE;
    }
    else if ( GetLocoPointer() && I.IsVar( "RenderInst\\Anim") )
    {
        if (I.IsRead())
            I.SetVarExternal(m_hAnimGroup.GetName(), RESOURCE_NAME_SIZE);
        else
        {
            // Anim changed?
            if( I.GetVarExternal()[0] )
            {
                SetAnimPackage(I.GetVarExternal());
            }
        }
        return TRUE;
    }
    else if ( GetLocoPointer() && I.IsVar( "RenderInst\\Weaponless Anim") )
    {
        if (I.IsRead())
        {
            I.SetVarExternal(m_WeaponlessAnimPackage, RESOURCE_NAME_SIZE);
        }
        else
        {
            // Anim changed?
            if( I.GetVarExternal()[0] )
            {
                x_strcpy( m_WeaponlessAnimPackage, I.GetVarExternal() );
            }
        }
        return TRUE;
    }
    // Skip this entire section if doesn't match 'Character'
    else if( I.IsBasePath("Character") )
    {
        if( I.IsVar("Character\\NPCName" ) )
        {
            if( I.IsRead() )
            {
                I.SetVarEnum( EnumToName( m_NPCName ) );
            }
            else
            {
                m_NPCName = (eNPCName)NameToEnum( I.GetVarEnum() );
            }

            return TRUE;
        }

        if( I.IsVar( "Character\\Weapon" ) )
        {
            if( I.IsRead() )
            {
                I.SetVarEnum( inventory2::ItemToName( m_WeaponItem ) );
            }
            else
            {
                m_WeaponItem = (inven_item)inventory2::NameToItem( I.GetVarEnum() );
            }
            return TRUE;
        }
        if( I.IsVar( "Character\\Grenade" ) )
        {
            if( I.IsRead() )
            {
                I.SetVarEnum( inventory2::ItemToName( m_GrenadeItem ) );
            }
            else
            {
                m_GrenadeItem = (inven_item)inventory2::NameToItem( I.GetVarEnum() );
            }
            return TRUE;
        }
        else if( I.IsVar( "Character\\Audio Package" ) )
        {
            if( I.IsRead() )
            {
                I.SetVarExternal( m_hAudioPackage.GetName(), RESOURCE_NAME_SIZE );
            }
            else
            {
                // Get the FileName
                const char* pString = I.GetVarExternal();

                if( pString[0] )
                {
                    if( xstring(pString) == "<null>" )
                    {
                        m_hAudioPackage.SetName( "" );
                    }
                    else
                    {
                        m_hAudioPackage.SetName( pString );                

                        // Load the audio package.
                        if( m_hAudioPackage.IsLoaded() == FALSE )
                            m_hAudioPackage.GetPointer();
                    }
                }
            }
            return( TRUE );
        }

        // Dialog prefix
        else if ( I.IsVar( "Character\\Dialog Prefix"))
        {
            if (I.IsRead())
            {
                if (m_DialogPrefixString >= 0)
                {
                    const char* String = g_StringMgr.GetString( m_DialogPrefixString );
                    I.SetVarString( String, x_strlen( String )+1 );
                }
                else
                    I.SetVarString( "<NONE>", 32 );                           
            }
            else
            {
                m_DialogPrefixString = g_StringMgr.Add( I.GetVarString() );
            }
            return TRUE;
        }
        else if ( I.IsVar( "Character\\Override Dialog Actor"))
        {
            if( I.IsRead() )
            {
                if ( character::m_DialogActorsEnum.DoesValueExist( m_OverrridePreferedVoiceActor ) )
                {
                    I.SetVarEnum( character::m_DialogActorsEnum.GetString( m_OverrridePreferedVoiceActor ) );
                }
                else
                {
                    I.SetVarEnum( "INVALID" );
                } 
            }
            else
            {
                character::eDialogActors DialogActorType;
                if( character::m_DialogActorsEnum.GetValue( I.GetVarEnum(), DialogActorType ) )
                {
                    m_OverrridePreferedVoiceActor = DialogActorType;
                    if( m_OverrridePreferedVoiceActor != DIALOG_ACTOR_NONE )
                    {
                        m_PreferedVoiceActor = m_OverrridePreferedVoiceActor;
                    }
                }
            }
            return( TRUE );
        }

        else if ( I.IsVar( "Character\\Logical Name"))
        {
            if (I.IsRead())
            {
                if (m_LogicalName >= 0)
                {
                    const char* String = g_StringMgr.GetString( m_LogicalName );
                    I.SetVarString( String, x_strlen( String )+1 );
                }
                else
                    I.SetVarString( "<NONE>", 32 );                           
            }
            else
            {
                m_LogicalName = g_StringMgr.Add( I.GetVarString() );
                InitTweaks();
            }
            return TRUE;
        }
        // Health
        else if( I.IsVar( "Character\\Health" ) )
        {
            if( I.IsRead() )
            {
                I.SetVarFloat( m_Health.GetHealth() );
            }
            else
            {
                m_Health.Reset( I.GetVarFloat() );
                m_MaxHealth = m_Health.GetHealth();
            }
            return TRUE;
        }
        else if( I.IsVar( "Character\\Start Active" ) )
        {
            if (I.IsRead())
            {
                I.SetVarBool(m_InitialState != character_state::STATE_HOLD);
            }
            else
            {
                if (I.GetVarBool())
                    m_InitialState = character_state::STATE_IDLE;
                else
                    m_InitialState = character_state::STATE_HOLD;
            }
            return TRUE;
        }
        else if( I.IsVar( "Character\\Ignore Pain") ||
                 I.IsVar( "Character\\Flags\\Ignore Pain"))
        {
            if( I.IsRead() )
            {
                xbool tempBool = m_bIgnorePain;
                I.SetVarBool(tempBool);
            }
            else
            {
                m_bIgnorePain = I.GetVarBool();
            }
            return TRUE;
        }
        else if( I.IsVar( "Character\\Ignore Flinches") ||
            I.IsVar( "Character\\Flags\\Ignore Flinches"))
        {
            if( I.IsRead() )
            {
                xbool tempBool = m_bIgnoreFlinches;
                I.SetVarBool(tempBool);
            }
            else
            {
                m_bIgnoreFlinches = I.GetVarBool();
            }
            return TRUE;
        }
        else if( I.IsVar( "Character\\Pass On Guid To Dead Body") ||
                 I.IsVar( "Character\\Flags\\Pass On Guid To Dead Body"))
        {
            if( I.IsRead() )
            {
                xbool tempBool = m_bPassOnGuidToCorpse;
                I.SetVarBool(tempBool);
            }
            else
            {
                m_bPassOnGuidToCorpse = I.GetVarBool();
            }
            return TRUE;
        }
        else if( I.IsVar( "Character\\Can Use Alarms" )  ||
            I.IsVar( "Character\\Flags\\Can Use Alarms"))
        {
            if (I.IsRead())
            {
                xbool tempBool = m_bCanUseAlarms;
                I.SetVarBool(tempBool);
            }
            else
            {
                m_bCanUseAlarms = I.GetVarBool();
            }
            return TRUE;
        }
        else if( I.IsVar( "Character\\Can Cloak" )  ||
            I.IsVar( "Character\\Flags\\Can Cloak"))
        {
            if (I.IsRead())
            {
                xbool tempBool = m_CanCloak;
                I.SetVarBool(tempBool);
            }
            else
            {
                if (I.GetVarBool())
                {
                    m_CanCloak = TRUE;
                }
                else
                {
                    m_CanCloak = FALSE;
                    m_MustCloak = FALSE;
                }
            }
            return TRUE;
        }
        else if( I.IsVar( "Character\\Bodies Fade" )  ||
            I.IsVar( "Character\\Flags\\Bodies Fade"))
        {
            if (I.IsRead())
            {
                xbool tempBool = m_BodiesFade;
                I.SetVarBool(tempBool);
            }
            else
            {
                m_BodiesFade = I.GetVarBool();                
            }
            return TRUE;
        }
        else if( I.IsVar( "Character\\AutoRagdoll" )  ||
            I.IsVar( "Character\\Flags\\AutoRagdoll"))
        {
            if (I.IsRead())
            {
                xbool tempBool = m_bAutoRagdoll;
                I.SetVarBool(tempBool);
            }
            else
            {
                m_bAutoRagdoll = I.GetVarBool();                
            }
            return TRUE;
        }
        else if( I.IsVar( "Character\\Force Weapon Fire" )  ||
            I.IsVar( "Character\\Flags\\Force Weapon Fire"))
        {
            if (I.IsRead())
            {
                xbool tempBool = m_bForceWeaponFire;
                I.SetVarBool(tempBool);
            }
            else
            {
                m_bForceWeaponFire = I.GetVarBool();                
            }
            return TRUE;
        }
        else if( I.IsVar( "Character\\Must Cloak" )  ||
            I.IsVar( "Character\\Flags\\Must Cloak"))
        {
            if (I.IsRead())
            {
                xbool tempBool = m_MustCloak;
                I.SetVarBool(tempBool);
            }
            else
            {
                if (I.GetVarBool())
                {
                    m_MustCloak = TRUE;
                    m_CanCloak = TRUE;
                }
                else
                {
                    m_MustCloak = FALSE;
                }
            }
            return TRUE;
        }
        else if( I.IsVar( "Character\\Combat Ready" )  ||
            I.IsVar( "Character\\Flags\\Combat Ready"))
        {
            if (I.IsRead())
            {
                xbool tempBool = m_CombatReady;
                I.SetVarBool(tempBool);
            }
            else
            {
                m_CombatReady = I.GetVarBool();
            }
            return TRUE;
        }

        else if( I.IsVar( "Character\\Never Combat Ready" ) ||
            I.IsVar( "Character\\Flags\\Never Combat Ready"))
        {
            if (I.IsRead())
            {
                xbool tempBool = m_bNeverCombatReady;
                I.SetVarBool(tempBool);
            }
            else
            {
                m_bNeverCombatReady = I.GetVarBool();
                if( m_bNeverCombatReady )
                {
                    m_CombatReady = FALSE;
                }
            }
            return TRUE;
        }

        else if (I.IsVar("Character\\Root to position and idle") ||
            I.IsVar("Character\\Flags\\Root to position and idle"))
        {
            if (I.IsRead())
            {
                xbool tempBool = m_bRootToPositionWhenIdle;
                I.SetVarBool( tempBool );
            }
            else
            {
                m_bRootToPositionWhenIdle = I.GetVarBool();
            }
            return TRUE;
        }
        else if (I.IsVar("Character\\Next Shot Must Hit") ||
                 I.IsVar("Character\\Flags\\Next Shot Must Hit") )
        {
            if (I.IsRead())
            {
                xbool tempBool = m_NextShotMustHit;
                I.SetVarBool( tempBool );
            }
            else
            {
                m_NextShotMustHit = I.GetVarBool();
            }
            return TRUE;
        }
        else if ( I.IsVar("Character\\Flags\\Reload Now") )
        {
            if (I.IsRead())
            {
                I.SetVarBool( FALSE );
            }
            else
            {
                xbool tempBool = I.GetVarBool();
                if( tempBool )
                {
                    ReloadWeapon();
                }
            }
            return TRUE;
        }
        else if ( I.IsVar("Character\\Flags\\Never Play Reload Anim") )
        {
            if (I.IsRead())
            {
                xbool tempBool = m_bNeverPlayReloadAnim;
                I.SetVarBool( tempBool );
            }
            else
            {
                m_bNeverPlayReloadAnim = I.GetVarBool();
            }
            return TRUE;
        }
        else if ( I.IsVar("Character\\Flags\\Dumb and Fast") )
        {
            if (I.IsRead())
            {
                xbool tempBool = m_bDumbAndFast;
                I.SetVarBool( tempBool );
            }
            else
            {
                m_bDumbAndFast = I.GetVarBool();
            }
            return TRUE;
        }
        else if ( I.IsVar("Character\\Flags\\Run Logic") )
        {
            if (I.IsRead())
            {
                xbool tempBool = m_bDoRunLogic;
                I.SetVarBool( tempBool );
            }
            else
            {
                m_bDoRunLogic = I.GetVarBool();
            }
            return TRUE;
        }
        else if ( I.IsVar("Character\\Flags\\Render") )
        {
            if (I.IsRead())
            {
                xbool tempBool = m_bDoRender;
                I.SetVarBool( tempBool );
            }
            else
            {
                m_bDoRender = I.GetVarBool();
            }
            return TRUE;
        }
        else if ( I.IsVar("Character\\Flags\\Is Alert") )
        {
            if (I.IsRead())
            {
                xbool tempBool = ( GetAwarenessLevel() >= AWARENESS_ALERT );
                I.SetVarBool( tempBool );
            }
            return TRUE;
        }
        else if ( I.IsVar("Character\\Flags\\Is Attacking") )
        {
            if (I.IsRead())
            {
                xbool tempBool = ( GetAwarenessLevel() >= AWARENESS_ACQUIRING_TARGET );
                I.SetVarBool( tempBool );
            }
            return TRUE;
        }
        else if ( I.IsVar("Character\\Flags\\Can Always Backpeddle") )
        {
            if (I.IsRead())
            {
                xbool tempBool = m_bCanAlwaysBackpeddle;
                I.SetVarBool( tempBool );
            }
            else
            {
                m_bCanAlwaysBackpeddle = I.GetVarBool();
            }
            return TRUE;
        }        
// Rendering BBox
        else if (I.IsVar( "Character\\Render BBox" ) )
        {
            SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSLATION );
            vector3 Size;

            if (I.IsRead())
            {
                Size = m_BBox.GetSize();

                I.SetVarVector3( Size );
            }
            else
            {
                Size = I.GetVarVector3();

                Size.GetX() /= 2;
                Size.GetZ() /= 2;

                m_BBox.Set( Size, vector3(-Size.GetX(),0,-Size.GetZ()) );
            }

            return TRUE;
        }
        else if (I.IsVar( "Character\\Allow Play Dialog" ) )
        {
            if (I.IsRead())
            {
                xbool tempBool = m_AllowDialog;
                I.SetVarBool( tempBool );
            }
            else
            {            
                m_AllowDialog = I.GetVarBool();
            }
            return TRUE;
        }
        
        // Cover dist
        else if (I.IsVar( "Character\\Max Cover Dist" ) )
        {
            if (I.IsRead())
                I.SetVarFloat( m_MaxDistToCover );
            else
                m_MaxDistToCover = I.GetVarFloat();
            return TRUE;
        }

        else if (I.IsVar( "Character\\Sticky Cover Node" ) )
        {
            if (I.IsRead())
            {
                I.SetVarGUID( m_StickyCoverNode );
            }
            else
            {            
                m_StickyCoverNode = I.GetVarGUID();
            }
            return TRUE;
        }

        else if (I.IsVar( "Character\\Sticky Turret" ) )
        {
            if (I.IsRead())
                I.SetVarGUID( m_StickyTurret );
            else
                m_StickyTurret = I.GetVarGUID();
            return TRUE;
        }

        else if (I.IsVar( "Character\\Activate On Death" ) )
        {
            if (I.IsRead())
                I.SetVarGUID( m_ActivateOnDeath );
            else
                m_ActivateOnDeath = I.GetVarGUID();
            return TRUE;
        }
        
        else if (I.IsVar( "Character\\AimAt Guid" ) )
        {
            if (I.IsRead())
                I.SetVarGUID( m_AimAtGuid );
            else
                m_AimAtGuid = I.GetVarGUID();
            return TRUE;
        }

        else if( m_FollowTargetAffector.OnProperty( I, "Character\\Follow Target" ) )
        {
            object *targetObject = m_FollowTargetAffector.GetObjectPtr();
            if( targetObject )
            {
                m_FollowTargetGuid = targetObject->GetGuid();
            }
            else
            {
                m_FollowTargetGuid  = 0;
            }
            return TRUE;
        }

        else if( m_OverrideTargetAffector.OnProperty( I, "Character\\Override Target" ) )
        {
            m_OverrideTargetGuid = m_OverrideTargetAffector.GetGuid();
            object *targetObject = g_ObjMgr.GetObjectByGuid(m_OverrideTargetGuid);
            if( targetObject )
            {            
                SetTargetGuid( m_OverrideTargetGuid );
            }
            return TRUE;
        }

        else if( I.VarFloat("Character\\Tweaks\\Shoot Delay", m_ShootDelay) )
        {
            return TRUE;
        }
        else if (I.IsVar( "Character\\Tweaks\\Accuracy" ) )
        {
#ifdef X_EDITOR
            if( !g_game_running )
            {            
                return TRUE;
            }
#endif

            if (I.IsRead())
            {            
                I.SetVarInt( m_Accuracy );
            }
            else
            {            
                m_Accuracy = I.GetVarInt();
                m_Accuracy = MAX(0,m_Accuracy);
                m_Accuracy = MIN(100,m_Accuracy);
            }
            return TRUE;
        }
        else if (I.IsVar( "Character\\Tweaks\\Moving Target Accuracy" ) )
        {
#ifdef X_EDITOR
            if( !g_game_running )
            {            
                return TRUE;
            }
#endif

            if (I.IsRead())
            {            
                I.SetVarInt( m_MovingTargetAccuracy );
            }
            else
            {            
                m_MovingTargetAccuracy = I.GetVarInt();
                m_MovingTargetAccuracy = MAX(0,m_MovingTargetAccuracy);
                m_MovingTargetAccuracy = MIN(100,m_MovingTargetAccuracy);
            }
            return TRUE;
        }
        else if (I.IsVar( "Character\\Tweaks\\Weapon Drop Percent" ) )
        {
            if (I.IsRead())
            {        
                I.SetVarInt( (s32)m_WeaponDropPercent );
            }
            else
            {        
                s32 weaponDrop = I.GetVarInt();
                if( weaponDrop <= 0 )
                {
                   m_bDropWeapons = FALSE;
                   m_WeaponDropPercent = 0;
                }
            }
            return TRUE;
        }
        else if (I.IsVar( "Character\\Tweaks\\Inventory Drop Percent" ) )
        {
            if (I.IsRead())
            {        
                I.SetVarInt( (s32)m_InventoryDropPercent );
            }
            else
            {        
                s32 inventoryDrop = I.GetVarInt();
                if( inventoryDrop <= 0 )
                {
                   m_bDropInventory = FALSE;
                   m_InventoryDropPercent = 0;
                }
            }
            return TRUE;
        }
        else if( I.IsVar( "Character\\Tweaks\\Drop Weapons" ) )
        {
            if (I.IsRead())
            {
                xbool tempBool = m_bDropWeapons;
                I.SetVarBool(tempBool);
            }
            else
            {
                m_bDropWeapons = I.GetVarBool();
            }
            return TRUE;
        }
        else if( I.IsVar( "Character\\Tweaks\\Drop Ammo" ) )
        {
            if (I.IsRead())
            {
                xbool tempBool = m_bDropAmmo;
                I.SetVarBool(tempBool);
            }
            else
            {
                m_bDropAmmo = I.GetVarBool();
            }
            return TRUE;
        }
        else if( I.IsVar( "Character\\Tweaks\\Drop Grenades" ) )
        {
            if (I.IsRead())
            {
                xbool tempBool = m_bDropGrenades;
                I.SetVarBool(tempBool);
            }
            else
            {
                m_bDropGrenades = I.GetVarBool();
            }
            return TRUE;
        }
        else if( I.IsVar( "Character\\Tweaks\\Drop Inventory" ) )
        {
            if (I.IsRead())
            {
                xbool tempBool = m_bDropInventory;
                I.SetVarBool(tempBool);
            }
            else
            {
                m_bDropInventory = I.GetVarBool();
            }
            return TRUE;
        }
        else if (I.IsVar( "Character\\Tweaks\\Light Sight Radius" ) )
        {
            if (I.IsRead())
                I.SetVarFloat( m_LightSightRadius );
            else
                m_LightSightRadius = I.GetVarFloat();
            return TRUE;
        }

        // SightRadius
        else if (I.IsVar( "Character\\Tweaks\\Dark Sight Radius" ) )
        {
            if (I.IsRead())
                I.SetVarFloat( m_DarkSightRadius );
            else
                m_DarkSightRadius = I.GetVarFloat();
            return TRUE;
        }

        // SoundRadius
        else if (I.IsVar( "Character\\Tweaks\\Sound Radius" ) )
        {
            if (I.IsRead())
                I.SetVarFloat( m_SoundRange );
            else
                m_SoundRange = I.GetVarFloat();
            return TRUE;
        }
        // InitialState -- For Backwards compatibility
        else if( I.IsVar( "Character\\Current State" ) )
        {
            if (I.IsRead())
            {
                s32 currentStateType = character_state::STATE_NULL;
                if( m_pActiveState )
                {
                    currentStateType = m_pActiveState->GetStateType();
                }

                I.SetVarEnum(GetStateName(currentStateType));
            }
            else
            {
                character_state::states newState = GetStateByName( I.GetVarEnum() );
                if( newState )
                {
                    SetupState( newState );
                }                
            }
            return TRUE;
        }

        else if (I.IsVar("Character\\Pathing\\Use Small Paths"))
        {
            if (I.IsRead())
            {
                if (m_PathingHints.SmallNPC != 0)
                    I.SetVarBool( TRUE );
                else 
                    I.SetVarBool( FALSE );
            }
            else
            {
                if (I.GetVarBool())
                    m_PathingHints.SmallNPC = 128;
                else
                    m_PathingHints.SmallNPC = 0;
            }
            return TRUE;
        }

        else if (I.IsVar("Character\\Pathing\\Use Navigation Map"))
        {
            if (I.IsRead())
            {
                I.SetVarBool( m_PathingHints.bUseNavMap );
            }
            else
            {
                m_PathingHints.bUseNavMap = I.GetVarBool();
            }
            return TRUE;
        }

        else if (I.IsVar( "Character\\Override Idle Lookat" ) )
        {
            if (I.IsRead())
                I.SetVarGUID( m_OverrideLookatInterest );
            else
                m_OverrideLookatInterest = I.GetVarGUID();
            return TRUE;
        }

        else if (I.IsVar("Character\\Ignore Lookat Interest"))
        {
            if (I.IsRead())
            {
                xbool tempBool = m_IgnoreInterest;
                I.SetVarBool( tempBool );
            }
            else
            {
                m_IgnoreInterest = I.GetVarBool();
            }
            return TRUE;
        }

        else if (I.IsVar("Character\\Log AI State Changes"))
        {
            if (I.IsRead())
            {
                xbool tempBool = m_LogStateChanges;
                I.SetVarBool( tempBool );
            }
            else
            {
                m_LogStateChanges = I.GetVarBool();
            }
            return TRUE;
        }

        else if (I.IsVar("Character\\Leash Guid"))
        {
            if (I.IsRead())
            {
                I.SetVarGUID( m_LeashGuid );
            }
            else
            {
                m_LeashGuid = I.GetVarGUID();

                if (m_LeashGuid != 0)
                {
                    object* pObj = g_ObjMgr.GetObjectByGuid( m_LeashGuid );
                    if (pObj)
                    {
                        m_PathingHints.LeashPt = pObj->GetPosition();
                        m_PathingHints.LeashDist = m_LeashDistance;
                    }                    
                    else
                    {
                        m_PathingHints.LeashPt.Set(0,0,0);
                        m_PathingHints.LeashDist = -1;
                    }
                }
                else
                {
                    m_PathingHints.LeashPt.Set(0,0,0);
                    m_PathingHints.LeashDist = -1;
                }
            }
            return TRUE;
        }
        else if (I.IsVar("Character\\Leash Distance"))
        {
            if (I.IsRead())
            {                
                I.SetVarFloat( m_LeashDistance );

                if (m_LeashGuid != 0)
                {
                    object* pObj = g_ObjMgr.GetObjectByGuid( m_LeashGuid );
                    if (pObj)
                    {
                        m_PathingHints.LeashPt = pObj->GetPosition();
                        m_PathingHints.LeashDist = m_LeashDistance;
                    }                    
                    else
                    {
                        m_PathingHints.LeashPt.Set(0,0,0);
                        m_PathingHints.LeashDist = -1;
                    }
                }
                else
                {
                    m_PathingHints.LeashPt.Set(0,0,0);
                    m_PathingHints.LeashDist = -1;
                }
            }
            else
            {   
                m_LeashDistance = MAX(0,I.GetVarFloat());                
            }
            return TRUE;
        }
    }
    else if ( actor::OnProperty( I ) )
    {
        return TRUE;
    }

    // States
    character_state* pState = m_pStateList;
    xbool found = FALSE;
    while(pState)
    {
        // Found?
        if( pState->GetStateType() != character_state::STATE_NULL && pState->OnProperty(I) )
        {
            found = TRUE;
        }
        // Do next
        pState = pState->m_pNext;
    }
    if( found )
    {
        return TRUE;
    }

    s32 ID = I.PushPath( "Factions\\" );
    if ( factions_manager::OnProperty( I, m_Faction, m_FriendFlags ) )
    {
        return TRUE;
    }
    I.PopPath( ID );

    // Not found
    return FALSE;
}

//=============================================================================

#ifdef X_EDITOR
s32 character::OnValidateProperties( xstring& ErrorMsg )
{
    s32 ErrorCount = 0;
    
    // Does character have animations?
    loco* pLoco = GetLocoPointer();
    if( ( pLoco ) && ( pLoco->IsAnimLoaded() ) )
    {
        // Check locomotion for a valid move style
        if( !pLoco->IsValidMoveStyle( pLoco->GetMoveStyle()) )
        {
            ErrorCount += 1;
            ErrorMsg += "Anim package has no complete move styles! - AI wil not work\n";
        }
        
        // Does character have simple death anim?
        xbool bHasDeathSimple = FALSE;
        const anim_group* pAnimGroup = GetAnimGroupPtr();
        if( pAnimGroup )
            bHasDeathSimple = ( pAnimGroup->GetAnimIndex( "DEATH_SIMPLE" ) != -1 );
            
        // Skip characters that have a simple death ie. don't need a ragdoll
        if( bHasDeathSimple == FALSE )
        {
            // If there aren't any rigid bodies for ragdoll, report error
            const skin_geom* pSkinGeom = m_SkinInst.GetSkinGeom();
            if( ( pSkinGeom ) && ( pSkinGeom->m_nRigidBodies == 0 ) )
            {
                ErrorCount += 1;
                ErrorMsg += "NO RAGDOLL PRESENT!\n.skingeom resource [" + 
                            xstring( m_SkinInst.GetSkinGeomName() ) + 
                            "] does not have a ragdoll physics .matx assigned.\n" +
                            "Character will just vanish when killed.";
            }
        }
    }
    
    return ErrorCount + actor::OnValidateProperties( ErrorMsg );
}
#endif

//=====================================================================================

xbool character::GetClearToMoveTo( const vector3& location, xbool doCollisionCheck )
{
    vector3 testPosition = location;

    nav_connection_slot_id holderSlot;
    if( !g_NavMap.GetConnectionContainingPoint(holderSlot,testPosition) || 
        holderSlot == NULL_NAV_SLOT || 
        m_CurrentConnectionSlot == NULL_NAV_SLOT )
    {
        return FALSE;
    }
    
    ng_connection2& holderConnection = g_NavMap.GetConnectionByID(holderSlot);
    ng_connection2& ourConnection = g_NavMap.GetConnectionByID(m_CurrentConnectionSlot);

    if( holderConnection.GetGridID() != ourConnection.GetGridID() )
    {
        return FALSE;
    }

    if( doCollisionCheck )
    {        
        // do a collision check... 
        g_CollisionMgr.LineOfSightSetup( GetGuid(), GetPositionWithOffset(OFFSET_CENTER), testPosition );
        g_CollisionMgr.SetMaxCollisions(1);
        // Perform collision
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_CHARACTER, object::ATTR_COLLISION_PERMEABLE );
        if ( g_CollisionMgr.m_nCollisions != 0 )
        {
            return FALSE;
        }
    }
    return TRUE;
}

//=====================================================================================

xbool character::UpdateCanMoveForward( f32 distance, xbool doCollisionCheck )
{
    radian ourYaw = GetLocoPointer()->m_Player.GetFacingYaw();
    vector3 forwardVec(0.0f,ourYaw); 
    forwardVec.Scale( distance );
    return GetClearToMoveTo(GetPositionWithOffset(OFFSET_CENTER) + forwardVec,doCollisionCheck);
}

//=====================================================================================

xbool character::UpdateCanMoveBack( f32 distance, xbool doCollisionCheck )
{
    radian ourYaw = GetLocoPointer()->m_Player.GetFacingYaw();
    vector3 backVec(0.0f,ourYaw); 
    backVec.Scale( -distance );
    return GetClearToMoveTo(GetPositionWithOffset(OFFSET_CENTER) + backVec,doCollisionCheck);
}

//=====================================================================================

xbool character::UpdateCanMoveLeft( f32 distance, xbool doCollisionCheck )
{
    radian ourYaw = GetLocoPointer()->m_Player.GetFacingYaw();
    vector3 leftVec(0.0f,ourYaw); 
    leftVec = leftVec.Cross( vector3(0.0f,-1.0f,0.0f) );
    leftVec.Scale( distance );
    return GetClearToMoveTo(GetPositionWithOffset(OFFSET_CENTER) + leftVec,doCollisionCheck);
}

//=====================================================================================

xbool character::UpdateCanMoveRight( f32 distance, xbool doCollisionCheck )
{
    radian ourYaw = GetLocoPointer()->m_Player.GetFacingYaw();
    vector3 rightVec(0.0f,ourYaw); 
    rightVec = rightVec.Cross( vector3(0.0f,1.0f,0.0f) );
    rightVec.Scale( distance );
    return GetClearToMoveTo(GetPositionWithOffset(OFFSET_CENTER) + rightVec,doCollisionCheck);
}

//=====================================================================================

xbool character::HasClearGrenadeThrow( xbool useTestLocation, const vector3& testLocation )
{
    switch( m_GrenadeItem )
    {
    case INVEN_NULL:
        return FALSE;
        break;
    case INVEN_GRENADE_FRAG:
        return HasClearFragThrow( useTestLocation,testLocation );
        break;
    case INVEN_GRENADE_GRAV:
        return HasClearGravThrow( useTestLocation,testLocation );
        break;
    }
    return FALSE;
}


xbool character::HasClearFragThrow(xbool useTestLocation, const vector3& testLocation )
{
    xbool bTrajectoryValid = DetermineGrenadeTrajectory( useTestLocation,testLocation );

    if( GetToTarget().LengthSquared() < 1200.0f * 1200.0f )
    {
        bTrajectoryValid = FALSE;
    }
    
    if( bTrajectoryValid )
    {
        vector3 TargetPos = GetLastKnownLocationOfTarget();
        // are there any alloes nearby? 
        bbox ProximityBox( TargetPos, 700.0F );
        g_ObjMgr.SelectBBox( object::ATTR_LIVING, ProximityBox, object::TYPE_ALL_TYPES );    
        slot_id aID = g_ObjMgr.StartLoop();
        while( (aID != SLOT_NULL) )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot(aID);
            //if valid and not myself
            if( pObject && IsFriendlyFaction(GetFactionForGuid(pObject->GetGuid())) ) 
            {
                bTrajectoryValid = FALSE;
            }
            aID = g_ObjMgr.GetNextResult( aID );
        }
        g_ObjMgr.EndLoop();
    }
    return bTrajectoryValid;
}

//========================================================================================

xbool character::HasClearGravThrow(xbool useTestLocation, const vector3& testLocation )
{
    xbool bTrajectoryValid = DetermineGrenadeStraightLine( useTestLocation,testLocation );

    if( GetToTarget().LengthSquared() < 1000.0f * 1000.0f )
    {
        bTrajectoryValid = FALSE;
    }

    if( bTrajectoryValid )
    {
        vector3 TargetPos = GetLastKnownLocationOfTarget();
        // are there any alloes nearby? 
        bbox ProximityBox( TargetPos, 700.0F );
        g_ObjMgr.SelectBBox( object::ATTR_LIVING, ProximityBox, object::TYPE_ALL_TYPES );    
        slot_id aID = g_ObjMgr.StartLoop();
        while( (aID != SLOT_NULL) )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot(aID);
            //if valid and not myself
            if( pObject && IsFriendlyFaction(GetFactionForGuid(pObject->GetGuid())) ) 
            {
                bTrajectoryValid = FALSE;
            }
            aID = g_ObjMgr.GetNextResult( aID );
        }
        g_ObjMgr.EndLoop();
    }
    return bTrajectoryValid;
}

//========================================================================================

void character::UpdateHasClearJumpAttack( )
{
    m_bHasClearJumpAttack = FALSE;
    s32 animIndex = GetLocoPointer()->GetAnimIndex(loco::ANIM_MELEE_LEAP);
    if( animIndex < 0 ||
        m_TargetGuid == 0 )
    {        
        return;
    }

    vector3 jumpDistance = GetLocoPointer()->GetAnimGroupHandle().GetPointer()->GetAnimInfo(animIndex).GetTotalTranslation();
    f32 jumpVertDistance = jumpDistance.GetY();

    // test from the eye position through the apex 
    vector3 topOfBox = GetPositionWithOffset(OFFSET_TOP_OF_BBOX);
    vector3 toTarget = GetToTarget();
    vector3 apex = toTarget;
    // apex is the eye position + half the distnace to our target then raised by the jumpvert distance
    apex.Scale(0.5f);
    apex += topOfBox;
    apex.GetY() += jumpVertDistance;

    // do a test from the eyes to the apex
    //==-----------------------------------------
    // We have the points, now check collisions.  
    g_CollisionMgr.LineOfSightSetup( GetGuid(), topOfBox, apex );
    // Only need one collision to say that we can't throw there
    g_CollisionMgr.SetMaxCollisions(1);

    // Perform collision
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_CHARACTER, object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING );
    if ( g_CollisionMgr.m_nCollisions != 0 )
    {
        return;
    }

    // do a test from the apex to the target 
    //==-----------------------------------------
    // We have the points, now check collisions.  
    g_CollisionMgr.LineOfSightSetup( GetGuid(), apex, topOfBox + toTarget );
    // Only need one collision to say that we can't throw there
    g_CollisionMgr.SetMaxCollisions(1);

    // Perform collision
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_CHARACTER, object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING );
    if ( g_CollisionMgr.m_nCollisions != 0 )
    {
        return;
    }
   
    // do a straight line test from us to the target
    //==-----------------------------------------
    // We have the points, now check collisions.  
    g_CollisionMgr.LineOfSightSetup( GetGuid(), GetPosition() + vector3(0.0f,k_StepHeight,0.0f), GetPosition() + toTarget + vector3(0.0f,k_StepHeight,0.0f) );
    // Only need one collision to say that we can't throw there
    g_CollisionMgr.SetMaxCollisions(1);

    // Perform collision
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_CHARACTER, object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING );
    if ( g_CollisionMgr.m_nCollisions != 0 )
    {
        return;
    }

    m_bHasClearJumpAttack = TRUE;
    return;
}

//========================================================================================

/*void character::UpdatePlayerLOF()
{
    // are we in the players LOF or
    // are we moving into the players LOF.
    // we only care if we are allies.
    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if( pPlayer && IsFriendlyFaction(pPlayer->GetFaction()) )
    {
        // do we care if the player is shooting? For now... no.
        // Let's start with a very simple player aim/bbox intersect test?
        vector3 playerFacing(pPlayer->GetPitch(),pPlayer->GetYaw());
        playerFacing.NormalizeAndScale(750.0f); // we will care out to 5 meters.

        bbox myBbox = GetLocoPointer()->m_Physics.GetBBox();
        myBbox.Translate( GetPosition() );
        f32 t; // t is how far along the path we impact.
        xbool intersect = myBbox.Intersect( t ,pPlayer->GetPositionWithOffset(OFFSET_EYES),pPlayer->GetPositionWithOffset(OFFSET_EYES)+playerFacing );
        m_bInPlayersLOF = ( intersect && 
                            t > 0.0f &&
                            t <= 1.0f);
    }
    else
    {
        m_bInPlayersLOF = FALSE;
        m_bMovingIntoPlayersLOF = FALSE;
    }
}*/

//========================================================================================

void character::EnableStun( f32 Duration )
{
    pain tempPain;
    if( HasAnim(loco::ANIM_MESON_STUN) && 
        !IgnorePain(tempPain) )
    {    
        m_StunTimer = Duration;
    }
}

//========================================================================================

void character::UpdateHasClearLOS()
{
    m_bHasClearLOSFeet = CheckClearLOS( OFFSET_NONE );
    m_bHasClearLOSEyes = CheckClearLOS( OFFSET_EYES );
}


//========================================================================================

xbool character::CheckClearLOS( eOffsetPos offset )
{
    if( GetTargetGuid() == 0 )
    {
        return FALSE;
    }

    vector3 ourPosition = GetPositionWithOffset(offset);
    vector3 targetPosition = GetTargetPosWithOffset(GetTargetGuid(),offset);
    
    // check slightly above the ground if foot to foot.
    if( offset == OFFSET_NONE )
    {    
        ourPosition.GetY() += 10.0f;
        targetPosition.GetY() += 10.0f;
    }

    g_CollisionMgr.LineOfSightSetup( GetGuid(), ourPosition, targetPosition );
    // Only need one collision to say that we can't throw there
    g_CollisionMgr.SetMaxCollisions(1);

    // Perform collision
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_CHARACTER_LOS, object::ATTR_COLLISION_PERMEABLE );
    if ( g_CollisionMgr.m_nCollisions != 0 )
    {
        return FALSE;
    }
    return TRUE;
}


//========================================================================================

xbool character::IgnorePain( const pain& Pain )
{
    // do not take damage if no sate or in a state that ignores pain
    if( !m_pActiveState || 
        m_pActiveState->IgnorePain( Pain ) ||
        m_bIgnorePain )
    {
        return TRUE;
    }

    return FALSE;
}

//========================================================================================

xbool character::IgnoreFullBodyFlinches( void )
{
    // Ignore if playing full body lip sync
    if( IsPlayingFullBodyLipSync() )
        return TRUE;

    // play an impact anim.
    if( !m_pActiveState || 
        m_pActiveState->IgnoreFullBodyFlinches() )
    {    
        return TRUE;
    }
    return FALSE;
}

actor::eHitType character::OverrideFlinchType( actor::eHitType hitType )
{
    // only take light hits when we are in cover.
    if( m_pActiveState &&
        m_pActiveState->GetStateType() == character_state::STATE_COVER &&
        !DoingMovementGoal() )
    {
        return HITTYPE_LIGHT;
    }

    return hitType;
}

//========================================================================================

xbool character::IgnoreFlinches( void )
{
    // play an impact anim.
    if( !m_pActiveState || 
        m_pActiveState->IgnoreFlinches() )
    {    
        return TRUE;
    }

    return FALSE;
}

//========================================================================================

void character::SendAlert( alert_package::alert_type alertType, guid CauseGuid, xbool forceSend ) 
{
    if( g_ObjMgr.GetGameDeltaTime( m_LastAlertSentTime ) < k_MinTimeBetweenAlerts &&
        !forceSend )
    {
        return;
    }
    m_LastAlertSentTime = g_ObjMgr.GetGameTime();

    alert_package NotifyPackage;
    NotifyPackage.m_Origin = GetGuid();
    NotifyPackage.m_Position = GetLocoPointer()->GetEyePosition();
    NotifyPackage.m_Type = alertType;
    NotifyPackage.m_Target = alert_package::ALERT_TARGET_NPC;
    NotifyPackage.m_AlertRadius = 500.0f;
    NotifyPackage.m_Cause = CauseGuid;
    NotifyPackage.m_FactionsSpecific = GetFaction();
    NotifyPackage.m_ZoneID = GetZone1();
    g_AudioManager.AppendAlertReceiver(NotifyPackage);
}

//========================================================================================

void character::SendAlert( alert_package::alert_type alertType, const vector3& sourceLocation, guid CauseGuid, xbool forceSend ) 
{
    if( g_ObjMgr.GetGameDeltaTime( m_LastAlertSentTime ) < k_MinTimeBetweenAlerts &&
        !forceSend )
    {
        return;
    }
    m_LastAlertSentTime = g_ObjMgr.GetGameTime();

    alert_package NotifyPackage;
    NotifyPackage.m_Origin = GetGuid();
    NotifyPackage.m_Position = sourceLocation;
    NotifyPackage.m_Type = alertType;
    NotifyPackage.m_Target = alert_package::ALERT_TARGET_NPC;
    NotifyPackage.m_AlertRadius = 500.0f;
    NotifyPackage.m_Cause = CauseGuid;
    NotifyPackage.m_FactionsSpecific = GetFaction();
    NotifyPackage.m_ZoneID = GetZone1();
    g_AudioManager.AppendAlertReceiver(NotifyPackage);
}
//========================================================================================

xbool character::DetermineGrenadeTrajectory( xbool useTestLocation, const vector3& testLocation )
{
    vector3 GrenadeInitPos = GetLocoPointer()->GetGrenadeBonePosition();
    if( useTestLocation )
    {
        GrenadeInitPos = testLocation;
    }

    vector3 TargetPos = GetLastKnownLocationOfTarget();
    object *aimAtTarget = g_ObjMgr.GetObjectByGuid( m_AimAtGuid );
    if( aimAtTarget )
    {
        TargetPos = aimAtTarget->GetPosition();
    }

    xbool retVal = TRUE;
    vector3 vDif = TargetPos - GrenadeInitPos;
    if( vDif.LengthSquared() < x_sqr(k_LongestSafeFragGrenadeDist) )
        retVal = FALSE;

    // if the target is above we must throw it at the target...
    if( vDif.GetY() <= 100.0f )
    {    
        // If the target is closer than 8 meters, throw it 8 meters.
        if ( vDif.LengthSquared() < x_sqr( 800.f) )
        {
            vector3 vDir( 0.f, 0.f, 800.f );
            vDir.RotateY( GetLocoPointer()->m_Player.GetFacingYaw() );
            TargetPos += vDir;
        }
        // Otherwise, throw the grenade 4 meters in front of the target so it bounces to the target.
        else
        {
            vDif.Normalize();
            vDif *= 400.f;
            TargetPos -= vDif;
        }
    }

    f32 Gravity        = -1000.f;
    f32 ForceOfThrow = 1500.f;
    radian arcs[2];

    GAMEUTIL_SolveForAngleOfElevation( ForceOfThrow, GrenadeInitPos, TargetPos, Gravity, arcs[0], arcs[1] );

    if ( x_isvalid( arcs[0] ) )
    {
        f32 fYawOffset = x_frand( -R_5, R_5 );
        f32 fPitchOffset = x_frand( -R_3, R_3 );
        vector3 vToTarget = TargetPos - GrenadeInitPos;

        radian GrenadeYaw = ( vToTarget ).GetYaw()  + fYawOffset;
        radian GrenadePitch = -1.0f * arcs[0] + fPitchOffset;

        // What's the grenade trajectory rotation.
        m_GrenadeTrajectory.Set( GrenadePitch , GrenadeYaw , 0.f );
        m_GrenadeDestination = TargetPos;
        
        if( !IsTrajectoryValid() )
        {
            retVal = FALSE;
        }            
        return retVal;
    }
    return FALSE;
}

//========================================================================================

xbool character::DetermineGrenadeStraightLine( xbool useTestLocation, const vector3& testLocation )
{
    vector3 GrenadeInitPos = GetLocoPointer()->GetGrenadeBonePosition();
    if( useTestLocation )
    {
        GrenadeInitPos = testLocation;
    }

    vector3 TargetPos = GetLastKnownLocationOfTarget();

    vector3 vDif = TargetPos - GrenadeInitPos;
    if( vDif.LengthSquared() < x_sqr(k_LongestSafeGravGrenadeDist) )
    {
        return FALSE;
    }

    // We have the points, now check collisions.  
    g_CollisionMgr.LineOfSightSetup( GetGuid(), GrenadeInitPos, TargetPos );

    // Only need one collision to say that we can't throw there
    g_CollisionMgr.SetMaxCollisions(1);

    // Perform collision
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_LARGE_PROJECTILES, object::ATTR_COLLISION_PERMEABLE );
    if ( g_CollisionMgr.m_nCollisions != 0 )
    {
        return FALSE;
    }
    return TRUE;
}

//========================================================================================

xbool character::IsTrajectoryValid( void )
{   
    //
    //  Pt = Po + v*t + 1/2*a*t^2
    //
    //  Pt = point on arc at t
    //  Po = start pos
    //   v = vel
    //   a = accel
    //
    //  Vertical component reaches apex when v+at=0, so t=-v/a
    //
    //  Target is reaches when horizontal component is completed.
    //  TimeToTarget = Dist on XZ plane / Speed on XZ plane
    //
    //
    //  We will test 3 line segs
    //  1) from start to a point on the first half of the arc
    //  2) from the point on the first half of the arc to the apex
    //  3) from the apex to a point on the second half of the arc
    //
    //
    vector3 GrenadeInitPos  = GetLocoPointer()->GetGrenadeBonePosition();
    m_GrenadeThrowStart = GrenadeInitPos;

    f32     Gravity         = -1000.0f;
    f32     ForceOfThrow    = 1500.0f;

    vector3 Vel( 0.0f, 0.0f, ForceOfThrow );
    Vel.Rotate( m_GrenadeTrajectory );

    vector3 Delta = m_GrenadeDestination - m_GrenadeThrowStart;           
    Delta.GetY() = 0;

    f32 XZDist = Delta.Length();

    vector3 VelH( Vel.GetX(), 0, Vel.GetZ() );
    f32     SpeedH = VelH.Length();
    f32     SpeedV = Vel.GetY();

    f32 TotalT =  XZDist / SpeedH;
    f32 ApexT  = -SpeedV / Gravity;

    if( ApexT < (TotalT * 0.25f) )
        ApexT = (TotalT * 0.25f);

    vector3 ApexPos( m_GrenadeThrowStart );
    ApexPos += vector3( VelH.GetX() * ApexT,
                        SpeedV * ApexT + 0.5f * Gravity * ApexT*ApexT,
                        VelH.GetZ() * ApexT );

    f32 FirstT = (ApexT / 2.0f);
    f32 SecondT = ApexT + (TotalT-ApexT)/2.0f;

    vector3 FirstHalfPoint( m_GrenadeThrowStart );
    vector3 SecondHalfPoint( m_GrenadeThrowStart );

    FirstHalfPoint += vector3( VelH.GetX() * FirstT,
                               SpeedV * FirstT + 0.5f * Gravity * FirstT*FirstT,
                               VelH.GetZ() * FirstT );

    SecondHalfPoint += vector3( VelH.GetX() * SecondT,
                                SpeedV * SecondT + 0.5f * Gravity * SecondT*SecondT,
                                VelH.GetZ() * SecondT );

    if( (m_GrenadeThrowStart - ApexPos).LengthSquared() > 1.0f )
    {
        //==-----------------------------------------
        //  FIRST CHECK - Start -> FirstHalfPoint
        //==-----------------------------------------
        // We have the points, now check collisions.  First from start pos to P0
        g_CollisionMgr.LineOfSightSetup( GetGuid(), m_GrenadeThrowStart, FirstHalfPoint );

        // Only need one collision to say that we can't throw there
        g_CollisionMgr.SetMaxCollisions(1);
        // Perform collision
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_LARGE_PROJECTILES, object::ATTR_COLLISION_PERMEABLE );
        if ( g_CollisionMgr.m_nCollisions != 0 )
        {
            return FALSE;
        }

        //==-----------------------------------------
        //  SECOND CHECK - FirstHalfPoint -> Apex
        //==-----------------------------------------
        g_CollisionMgr.LineOfSightSetup( GetGuid(), FirstHalfPoint, ApexPos );

        // Only need one collision to say that we can't throw there
        g_CollisionMgr.SetMaxCollisions(1);
        // Perform collision
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_LARGE_PROJECTILES, object::ATTR_COLLISION_PERMEABLE );
        if ( g_CollisionMgr.m_nCollisions != 0 )
        {
            return FALSE;
        }
    }

    vector3 FinalDest = m_GrenadeDestination;
    FinalDest.GetY() += 100.0f;

    if( (FinalDest - ApexPos).LengthSquared() > 1.0f )
    {
        //==-----------------------------------------
        //  THIRD CHECK - Apex -> SecondHalfPoint
        //==-----------------------------------------
        g_CollisionMgr.LineOfSightSetup( GetGuid(), ApexPos, SecondHalfPoint );

        // Only need one collision to say that we can't throw there
        g_CollisionMgr.SetMaxCollisions(1);

        // Perform collision
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_LARGE_PROJECTILES, object::ATTR_COLLISION_PERMEABLE );
        if ( g_CollisionMgr.m_nCollisions != 0 )
        {
            return FALSE;
        }

        //==-----------------------------------------
        //  FOURTH CHECK - Apex -> SecondHalfPoint
        //==-----------------------------------------
        g_CollisionMgr.LineOfSightSetup( GetGuid(), SecondHalfPoint, FinalDest );

        // Only need one collision to say that we can't throw there
        g_CollisionMgr.SetMaxCollisions(1);

        // Perform collision
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_LARGE_PROJECTILES, object::ATTR_COLLISION_PERMEABLE );
        if ( g_CollisionMgr.m_nCollisions != 0 )
        {
            return FALSE;
        }
    }


    return TRUE;
}


//========================================================================

void character::SetActiveListener( xbool bActiveListener )
{
    ( void ) bActiveListener;

    // This routine must be overloaded per character.  Only friendly
    // units to the player can call this function and they need to handle it
    // on their own.
    ASSERT( FALSE );
}

//========================================================================

xbool character::SetPotentialListener( xbool bActiveListener )
{
    // This routine must be overloaded per character.  Only friendly
    // units to the player can call this function and they need to handle it
    // on their own.
    ( void ) bActiveListener;
    return FALSE;
}

//===========================================================================

// Returns TRUE if the character can see another object
xbool character::CanSeeObject( guid Guid, xbool bIgnoreSightCone )
{
    // You should only call when thinking!
    ASSERT(m_bThinking);

    // Target to go for?
    if ((!GetLocoPointer()) || (Guid == 0))
        return FALSE;

    // Lookup object
    object* pObject = g_ObjMgr.GetObjectByGuid(Guid);
    if (!pObject)
        return FALSE;

    // Compute path towards objects
    vector3 DestPos = GetTargetPosWithOffset( pObject->GetGuid(),OFFSET_EYES );    
    u8 brightness = 255;   
    if( pObject && pObject->IsKindOf(player::GetRTTI()) )
    {
        player &playerSource = player::GetSafeType( *pObject ); 
        if( !playerSource.IsFlashlightOn() )
        {        
            brightness = (u8)playerSource.GetFloorIntensity();
        }
    }

    if (!bIgnoreSightCone)
    {
        if (!IsTargetInSightCone(DestPos,brightness))
            return FALSE;
    }

    // for actors we check eyes and feet.
    guid IgnoreObject = Guid;
    vector3 vPosition = GetPositionWithOffset(OFFSET_EYES);


    // removed because NPCs in cover near a wall would be able to see through the wall
    // and into a hallway nearby (no way they should be able to see here!)
/*    if( GetIsInCover() )
    {        
        vector3 toTarget = DestPos - vPosition;
        toTarget.NormalizeAndScale( k_IgnoreObjectsInCoverDist );
        // if I'm in cover then I ignore the first few meters of the test thus hopefully ignoring our cover. 
        vPosition += toTarget;
    }*/

    vector3 vToTest = DestPos - vPosition;
    f32 fTestLen = vToTest.LengthSquared();

    // added awareness test, otherwise NPCs that are aware of you (like you shoot them)
    // but beyond their light sight may just stand there not even looking at the player
    // (such as if stuck at a cover node).
    if( fTestLen > x_sqr(m_LightSightRadius) && GetAwarenessLevel() < AWARENESS_ACQUIRING_TARGET )
    {
        return FALSE;
    }

/*    // If length squared is less than s_Dist_Per_Segment_Sqr, just test it as usual.
    g_CollisionMgr.LineOfSightSetup( GetGuid(), vPosition, DestPos);   
    if( !bHighPoly ) g_CollisionMgr.UseLowPoly();
    g_CollisionMgr.AddToIgnoreList( IgnoreObject );
    g_CollisionMgr.SetMaxCollisions(1);

    // ignore destructables if I have a target;
    u32 collisionAttribs;
    if( GetAwarenessLevel() >= AWARENESS_ACQUIRING_TARGET )
    {
        collisionAttribs = ( object::ATTR_COLLISION_PERMEABLE | object::ATTR_DESTRUCTABLE_OBJECT );
        //        collisionAttribs = (object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING | object::ATTR_DESTRUCTABLE_OBJECT );
    }
    else
    {
        collisionAttribs = ( object::ATTR_COLLISION_PERMEABLE );
        //        collisionAttribs = (object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING);   
    }
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, collisionAttribs );*/


    return ( GetGuidBlockingLOS(GetGuid(),IgnoreObject,vPosition,DestPos,TRUE,GetAwarenessLevel() >= AWARENESS_ACQUIRING_TARGET) == 0 );
}

//===========================================================================
xbool character::GetIsInCover()
{
    if( m_pActiveState && 
        m_pActiveState->GetStateType() == character_state::STATE_COVER )
    {
        // only if I'm actually in cover, no just cover state...
        character_cover_state *coverState = (character_cover_state *)m_pActiveState;
        if( coverState->GetIsInCover() )
        {
            return TRUE;
        }
    }
    return FALSE;
}

//===========================================================================

xbool character::CanPathTo( const vector3& vDestination )
{
    // clear loco up.
    god* pGod = SMP_UTIL_Get_God();
    ASSERT( pGod );
    path_find_struct  tempStruct;

    xbool bValidPath = pGod->RequestPathWithEdges( this, vDestination, tempStruct, MAX_EDGES_IN_LIST, &m_PathingHints, FALSE );
    return bValidPath;
}

//===========================================================================

xbool character::GetNewPath( const vector3& vDestination )
{
    // clear loco up.
    GetLocoPointer()->ClearMoveFlags();

    god* pGod = SMP_UTIL_Get_God();
    ASSERT( pGod );
    m_SinceLastGetPath = 0.0f;

    m_PathFindStruct.Clear();
    xbool bValidPath = pGod->RequestPathWithEdges( this, vDestination, m_PathFindStruct, MAX_EDGES_IN_LIST, &m_PathingHints ); //, m_PathList, MAX_NODES_IN_PATH_LIST );
    m_CurrentPathStructIndex = -1;
    m_PathReset = TRUE;
    SetIsFollowingPath(TRUE);
    m_NeedsToRecalcPath = FALSE;
    m_PreviousPathPoint = m_PathFindStruct.m_vStartPoint;
    #ifdef AI_LOGGING
    LOG_MESSAGE( "character::GetNewPath", "Path rebuild.  # steps: %d",m_PathFindStruct.m_nSteps );
    #endif

    return bValidPath;
}

//===========================================================================

nav_connection_slot_id character::GetNewRetreatPath( const vector3& vRetreatFrom, f32 DistToAchieve )
{
    // clear loco up.
    GetLocoPointer()->ClearMoveFlags();

    (void)DistToAchieve;
    god* pGod = SMP_UTIL_Get_God();
    ASSERT( pGod );
    m_SinceLastGetPath = 0.0f;
    m_RethinkRetreatTimer = 0.0f;
    m_PathReset = TRUE;

    object *targetObject = g_ObjMgr.GetObjectByGuid(m_GoalInfo.m_TargetGuid);
    vector3 toTarget = vRetreatFrom - GetPosition();
    f32 distToTarget = toTarget.Length();

    toTarget.GetY() = 0;    
    vector3 dotCompareVector = toTarget;

    // Normally we will only consider nodes that lead us away from the threat, 
    // the dot must be less than 0.0f
    f32 minAcceptableDot = 0.0f;

    // If we are very close to the threat and the threat is an actor then
    // we would prefer to run away behind the threats facing so he has to turn 
    // to chase us. 
    if( targetObject && 
        targetObject->IsKindOf(actor::GetRTTI()) &&
        distToTarget <= k_MinDistToRunBehind )
    {
        if( targetObject->IsKindOf(player::GetRTTI()) )
        {
            player &targetPlayer = player::GetSafeType( *targetObject );
            radian pitch,yaw;
            targetPlayer.GetView().GetPitchYaw(pitch,yaw);
            dotCompareVector = vector3( 0.0f,yaw );
        }
        else
        {        
            actor &targetActor = actor::GetSafeType( *targetObject );
            dotCompareVector = vector3( 0.0f,targetActor.GetYaw() );
        }
    }
    // as the threat closes, we become more willing to run somewhat towards it in 
    // order to get away, to the minimum acceptabel dot increases.
    else if ( distToTarget < k_MinDistAcceptableDotIncreases )
    {
        minAcceptableDot = 1.0f * ((k_MinDistAcceptableDotIncreases-distToTarget)/k_MinDistAcceptableDotIncreases);
    }    

    nav_node_slot_id bestNavSlot = NULL_NAV_SLOT;
    f32 bestDot = minAcceptableDot;

    m_GoalRetreatToConnectionSlot = m_CurrentConnectionSlot;

    // we are mearly trying to find the next point to go to, not the complete path. 
    // step 1. See if there are any connctions meeting our requirements. We will prefer 
    // the one with the best dot result.

    if ( NULL_NAV_SLOT != m_GoalRetreatToConnectionSlot )
    {
        ng_connection2&     Conn = g_NavMap.GetConnectionByID( m_GoalRetreatToConnectionSlot );
        s32 i;

        for (i=0;i<Conn.GetOverlapCount();i++)
        {
            nav_node_slot_id iNode = Conn.GetOverlapNodeID( i );
            if (NULL_NAV_SLOT == iNode)
            {       
                continue;
            }
            ng_node2&   Node = g_NavMap.GetNodeByID( iNode );
            vector3 toNode = Node.GetPosition() - GetPosition();
            toNode.GetY() = 0;
            toNode.Normalize();
        
            // only go to nodes away from the target.
            // and where at least one of the connections gets you further away.           
            vector3 currConnToTarget = vRetreatFrom - g_NavMap.GetNearestPointOnConnection(m_CurrentConnectionSlot,vRetreatFrom);
            vector3 newConnToTarget = vRetreatFrom - g_NavMap.GetNearestPointOnConnection(Node.GetOtherConnectionID(m_CurrentConnectionSlot),vRetreatFrom);

            // prefer the node with the smallest dot or that we are inside 
            // and whose other connection gets us further away from the threat
            if ( newConnToTarget.LengthSquared() >= currConnToTarget.LengthSquared() &&
                 (toNode.Dot(dotCompareVector) < bestDot ||
                  (Node.IsPointInside(GetPosition()) &&
                   bestNavSlot == NULL_NAV_SLOT)) )
            {
                bestDot = toNode.Dot(dotCompareVector);
                bestNavSlot = iNode;
            }
        }
    }
    if( bestNavSlot != NULL_NAV_SLOT )
    {
        /// if we found a slot run to it!
        ng_node2&   Node = g_NavMap.GetNodeByID( bestNavSlot );            
        nav_connection_slot_id otherConnSlot = Node.GetOtherConnectionID(m_GoalRetreatToConnectionSlot);
        m_PathFindStruct.Clear();

        // choose a point the correct distance away from our target.
        vector3 goalPosition = toTarget;
        goalPosition.NormalizeAndScale( -DistToAchieve );
        goalPosition += vRetreatFrom;
        vector3 nextPosition = GetPosition();
        g_NavMap.GetClosestPointInOverlap(  m_GoalRetreatToConnectionSlot, 
                                            otherConnSlot, 
                                            GetPosition(),
                                            goalPosition,
                                            GetLocoPointer()->m_Physics.GetColRadius(),
                                            nextPosition );

        pGod->RequestPathWithEdges( this, nextPosition, m_PathFindStruct, MAX_EDGES_IN_LIST ); 

        m_PathFindStruct.m_EndConnectionSlotID = Node.GetOtherConnectionID(m_GoalRetreatToConnectionSlot);
        m_CurrentPathStructIndex = -1;
        SetIsFollowingPath(TRUE);
        m_NeedsToRecalcPath = FALSE;
        m_PreviousPathPoint = m_PathFindStruct.m_vStartPoint;

        LOG_MESSAGE( "character::GetNewRetreatPath", "Path rebuild.  # steps: %d",m_PathFindStruct.m_nSteps );
        return Node.GetOtherConnectionID(m_GoalRetreatToConnectionSlot);
    }
    else
    {
        // no valid slot found, run to one of the corners of this connection.        
        m_NeedsToRecalcPath = FALSE;
        if( SetupRetreatInsideConnection( vRetreatFrom ) )
        {
            LOG_MESSAGE( "character::GetNewRetreatPath", "Retreating within current connection" );
            return NULL_NAV_SLOT;
        }
        else
        {        
            LOG_MESSAGE( "character::GetNewRetreatPath", "Unable to build a retreat path" );
            return NULL_NAV_SLOT;
        }
    }
}

//===========================================================================
static f32 k_RetreatInside_BorderDist       = 100.0f;            // Distance to stay back from the connection edges.
static f32 k_RetreatInside_BorderPercent    = 0.15f;            // % multiplier on the minimum of connection width
                                                                // or height.  Used as the BorderDist
static f32 k_RetreatInside_Size             = 300.0f;           // If connection width/height is > than this
                                                                // value, use the k_RetreatInside_BorderDist
                                                                // otherwise use the k_RetreatInside_BorderPercent
                                                            
//
//  We should end up in SetupRetreatInsideConnection only if the NPC is
//  trapped and has nowhere to go.
//
xbool character::SetupRetreatInsideConnection( const vector3& vRetreatFrom )
{    
    if ( NULL_NAV_SLOT == m_CurrentConnectionSlot )
        return FALSE;
    // let's try a simplified version. Run to one of the 4 corners.

    object *targetObject = g_ObjMgr.GetObjectByGuid(m_GoalInfo.m_TargetGuid);
    vector3 MyPos       = GetPosition();
    vector3 vToThreat   = vRetreatFrom - MyPos;
    f32 distToThreat = vToThreat.Length();
    vector3 dotCompareVector = vToThreat;

    // Normally we will only consider corners that lead us away from the threat, 
    // the dot must be less than 0.0f
    f32 minAcceptableDot = 0.0f;
    
    // if we are very close to the threat and the threat is an actor then
    // we would prefer to run away behind the threats facing so he has to turn 
    // to chase us. 
    if( targetObject && 
        targetObject->IsKindOf(actor::GetRTTI()) &&
        distToThreat <= k_MinDistToRunBehind )
    {
        actor &targetActor = actor::GetSafeType( *targetObject );
        dotCompareVector = vector3( 0.0f,targetActor.GetLocoPointer()->GetYaw() );
    }
    // as the NPC get's closer, the angle we are willing to accept to run to changes. 
    else if( distToThreat < k_MinDistAcceptableDotIncreases )
    {
        minAcceptableDot = 1.0f * ((k_MinDistAcceptableDotIncreases-distToThreat)/k_MinDistAcceptableDotIncreases);
    }

    dotCompareVector.Normalize();

    // Now let's find the 4 corners.
    // Build connection edge endpoints
    ng_connection2& Conn    = g_NavMap.GetConnectionByID( m_CurrentConnectionSlot );
    vector3     vConnStart  = Conn.GetStartPosition();           // Start Position
    vector3     vConnEnd    = Conn.GetEndPosition();             // End Position
    radian      ConnYaw     = (vConnStart-vConnEnd).GetYaw();    // World YAW of connection centerline
    vector3     ConnEdge    = vector3(0,0,Conn.GetWidth());      // Width of connection
    vector3     Corner[4];

    ConnEdge.RotateY( ConnYaw+R_90 );           // Vector3 perpendicular to connection centerline
    Corner[0] = vConnStart + ConnEdge;          // Corner[0]           Corner[3]
    Corner[1] = vConnStart - ConnEdge;          //  +-----------------------+
    Corner[2] = vConnEnd   - ConnEdge;          //  |      Connection       |
    Corner[3] = vConnEnd   + ConnEdge;          //  |                       |
                                                //  +-----------------------+
                                                // Corner[1]           Corner[2]

    s32 bestCorner = -1;
    f32 bestDist = 0.0f;
    s32 c;

    // check each corner and see which best fulfills our requirments.
    // prefer the one furthest away when multiple valid.
    for(c=0;c<4;c++)
    {
        vector3 toCorner = Corner[c] - vRetreatFrom;
        toCorner.Normalize();
        if ( toCorner.Dot(dotCompareVector) < minAcceptableDot &&
             toCorner.LengthSquared() > bestDist )
        {
            bestDist = toCorner.LengthSquared();
            bestCorner = c;
        }       
    }

    if( bestCorner < 0 )
    {
        // rare case pick randomly
        bestCorner = x_irand(0,3);
    }

    f32 ConnSize = MIN( Conn.GetLength(), Conn.GetWidth() );
    f32 SafetyDist = k_RetreatInside_BorderDist;
    if (ConnSize < k_RetreatInside_Size)
        SafetyDist = ConnSize * k_RetreatInside_BorderPercent;

    vector3 Safety = MyPos - Corner[bestCorner];
    Safety.Normalize();
    Safety.Scale( SafetyDist );

    // push us back inot the connection some.
    Corner[bestCorner] += Safety;

    // setup the pathfind struct.
    m_PathFindStruct.Clear();
    m_PathFindStruct.m_nSteps                   = 0;
    m_PathFindStruct.m_vStartPoint              = MyPos;
    m_PathFindStruct.m_vEndPoint                = Corner[bestCorner];
    m_PathFindStruct.m_bStartPointOnConnection  = TRUE;
    m_PathFindStruct.m_bEndPointOnConnection    = TRUE;

    return TRUE;
}

//===========================================================================
/*
void character::UpdateLookahead( void )
{
    CONTEXT( "character::UpdateLookahead" );

    // only update the lookahead if we are moving.
    if( GetLocoPointer()->GetState() != loco::STATE_MOVE )
    {
        return;
    }
    vector3 nextPosition = GetPosition() + GetLocoPointer()->m_Physics.GetVelocity();
    if( m_CurrentConnectionSlot != NULL_NAV_SLOT )
    {
        ng_connection2& Conn = g_NavMap.GetConnectionByID( m_CurrentConnectionSlot );
        // first a simple test, otherwise the bigger test
        if( Conn.IsPointInConnection(nextPosition),0.0f)
        {
            m_LookaheadInGrid = TRUE;
        }
        else
        {
            m_LookaheadInGrid = g_NavMap.IsPointInGrid(nextPosition,Conn.GetGridID());
        }
    }
    else
    {
        m_LookaheadInGrid = FALSE;
    }
}
*/
//===========================================================================

void character::UpdateSpline()
{
    if( m_CurrentSplinePoint >= 0 &&
        GetCurrentConnection() != NULL_NAV_SLOT &&
        UseSplines() )
    {   
        // if we hit someone while splining, just drop the spline.
        if( m_AvoidActorAttempt != AVOID_NONE ||
            m_CurrentEscapeAttempt != ESCAPE_NONE )
        {
            m_CurrentSplinePoint = -1;
        }
        else
        {        
            // All we need to do is get sorta close to these and we'll move on.
            vector3 splineDiff = m_SplinePointList[m_CurrentSplinePoint] - GetPosition();
            splineDiff.Set( splineDiff.GetX(),0.0f,splineDiff.GetZ() );
            if( splineDiff.LengthSquared() <= 50.0f * 50.0f )
            {
                ng_connection2 &ourConnection = g_NavMap.GetConnectionByID( GetCurrentConnection() );
                do
                {            
                    m_CurrentSplinePoint++;
                    if( m_CurrentSplinePoint >= m_PointsPerSpline )
                    {
                        m_CurrentSplinePoint = -1;
                    }
                } while( m_CurrentSplinePoint >= 0 && !ourConnection.IsPointInConnection(m_SplinePointList[m_CurrentSplinePoint],0.0f) );
            }
        }
    }
    else
    {
        m_CurrentSplinePoint = -1;
    }
}

//===========================================================================

void character::CalculateSplinePoints()
{
    m_CurrentSplinePoint = -1;
    m_PointsPerSpline = -1;
    
    // No spline if going straight for target...
    if( m_PathingHints.bUseNavMap == FALSE ||
        m_PathFindStruct.m_bStraightPath )
        return;
    
    // are we doing splines and are we far enough in to start?
    if( m_CurrentPathStructIndex >= 0 )
    {
        vector3 previousPoint;
        vector3 firstPoint;
        vector3 secondPoint;
        vector3 nextPoint;

        // calculate the prev point
        previousPoint = m_PreviousPathPoint;
        
        // calculate the first point
        firstPoint = GetPosition();

        //calculate the second point
        secondPoint = m_NextPathPoint;

        // calculate the next point
        if( m_CurrentPathStructIndex+1 < m_PathFindStruct.m_nSteps )
        {
            nav_node_slot_id nextNodeID = m_PathFindStruct.m_StepData[ m_CurrentPathStructIndex+1 ].m_NodeToPassThrough;
            if ( nextNodeID == NULL_NAV_SLOT )
            {
                nextPoint = m_PathFindStruct.m_vEndPoint;
            }
            else
            {
                const ng_node2& nextNode = g_NavMap.GetNodeByID( nextNodeID );
                nextPoint = nextNode.GetCenter();
            }
        }
        else
        {
            nextPoint = m_PathFindStruct.m_vEndPoint;
        }

        // calculate the number of points per spline, no more than 1 for every 2 meters.
        m_PointsPerSpline = s32((firstPoint - secondPoint).Length()/200.0f);

        // make sure we aren't setting more points per spline than we have allocated
        if( m_PointsPerSpline > k_MaxSplinePoints )
        {
            m_PointsPerSpline = k_MaxSplinePoints;
        }

        // ok now that we have all the points we can fill in the spline list and set us to the first one.
        s32 c;
        for(c=0;c<m_PointsPerSpline;c++)
        {
            // set both normals to equal weight
            vector3 firstToSecond = firstPoint - secondPoint;
            vector3 firstNormal = secondPoint-previousPoint;
            firstNormal.NormalizeAndScale(firstToSecond.Length());
            vector3 secondNormal = nextPoint-firstPoint;
            secondNormal.NormalizeAndScale(firstToSecond.Length());
            m_SplinePointList[c] = GetSplinePos(firstPoint,firstNormal,secondPoint,secondNormal,(((f32)c+1)/((f32)m_PointsPerSpline+1)) );
        }
        // only start us if we have points!
        if( m_PointsPerSpline > 0 )
        {        
            m_CurrentSplinePoint = 0;
        }
    }
}

//===========================================================================

void character::UpdatePathing( void )
{
    CONTEXT( "character::UpdatePathing" );

    // we should only path if we are going somewhere...
    if( !DoingMovementGoal() )
    {
        return;
    }
    m_bCanReachPathDest = TRUE;

    // Skip nav map?
    if( m_PathingHints.bUseNavMap == FALSE ||
        m_PathFindStruct.m_bStraightPath )
    {
        m_NextPathPoint   = m_PathFindStruct.m_vEndPoint;
        m_DesiredLocation = m_PathFindStruct.m_vEndPoint;
        SetIsFollowingPath(FALSE);
        return;
    }

    // Finished path?
    if( m_CurrentPathStructIndex >= m_PathFindStruct.m_nSteps )
    {
        SetIsFollowingPath(FALSE);
    }

    xbool bJumping = !!(m_JumpTimeRemaining > 0);

    // Not following the path?
    if ( !GetIsFollowingPath() )
    {
        m_NextPathPoint = m_PathFindStruct.m_vEndPoint;
    }
    // Are we close to our destination?
    else if (( GetLocoPointer()->IsAtPosition(m_NextPathPoint) || m_PathReset ) && (!bJumping))
    {
        #ifdef AI_LOGGING
        LOG_MESSAGE( "character::UpdatePathing", "Pathing: %d of %d",m_CurrentPathStructIndex, m_PathFindStruct.m_nSteps );
        
        if (m_PathReset)
            LOG_MESSAGE( "character::UpdatePathing", "Path rest, updating desired location");
        else
        {
            LOG_MESSAGE( "character::UpdatePathing", "Reached move pos, updating desired location");
            LOG_MESSAGE( "character::UpdatePathing", "Goal completed: %d",GetGoalCompleted());
            LOG_MESSAGE( "character::UpdatePathing", "Goal succeeded: %d",GetGoalSucceeded());            
        }
        #endif

        vector3 vMoveTo = GetPosition();
        m_CurrentPathStructIndex++;
        m_PathReset = FALSE;

        if( m_PathFindStruct.m_EndConnectionSlotID != NULL_NAV_SLOT )
        {
            ng_connection2 endConnection = g_NavMap.GetConnectionByID( m_PathFindStruct.m_EndConnectionSlotID );
            if( endConnection.IsPointInConnection(GetPosition()) )
            {
                m_CurrentConnectionSlot = m_PathFindStruct.m_EndConnectionSlotID;
                m_CurrentConnectionSlotChanged = TRUE;
            }
        }

        xbool bStartNull = m_PathFindStruct.m_StepData[ m_CurrentPathStructIndex ].m_CurrentConnection == NULL_NAV_SLOT;
        xbool bEndNull   = m_PathFindStruct.m_StepData[ m_CurrentPathStructIndex ].m_DestConnection    == NULL_NAV_SLOT;

        if ( bStartNull && bEndNull )
        {
            #ifdef AI_LOGGING
            LOG_MESSAGE( "character::UpdatePathing", "Start & End connections are NULL");
            #endif
            // We either need to move to the final position or we are done at this point.
            SetIsFollowingPath(FALSE);
            vMoveTo = m_PathFindStruct.m_vEndPoint;
        }
        else
        if ( bEndNull )
        {
            #ifdef AI_LOGGING
            LOG_MESSAGE( "character::UpdatePathing", "End connection is NULL" );
            #endif
            // Move to the destination point once we are out of edges.
            // if the point is outside of the connection, move to a close point inside..
            if( !m_CanReachGoalTarget )
            {
                
                vMoveTo = g_NavMap.GetNearestPointInNavMap( m_PathFindStruct.m_vEndPoint );
                // we must move this point in some or the NPC will keep trying to reach it and failing.
                vector3 toMoveto = GetPosition() - vMoveTo;                
                toMoveto.NormalizeAndScale(150.0f);
                vMoveTo += toMoveto;
                m_bCanReachPathDest = FALSE;
            }
            else
            {                           
                vMoveTo = m_PathFindStruct.m_vEndPoint;
                SetIsFollowingPath(FALSE);
            }
        }
        else
        {
            //
            //  Handle moving through the next connection
            //  
            const ng_connection2& CurConn = g_NavMap.GetConnectionByID( m_PathFindStruct.m_StepData[m_CurrentPathStructIndex].m_CurrentConnection );

            if (CurConn.GetFlags() & ng_connection2::HINT_JUMP)
            {
                CalculateJumpInfo();
            }
            else
            {
                vector3 RemoteEnd = m_PathFindStruct.m_vEndPoint;
                if (m_CurrentPathStructIndex < (m_PathFindStruct.m_nSteps-1))
                {
                    if( m_PathFindStruct.m_StepData[m_CurrentPathStructIndex+1].m_NodeToPassThrough != NULL_NAV_SLOT )
                    {                    
                        ng_node2& ngNode = g_NavMap.GetNodeByID( m_PathFindStruct.m_StepData[m_CurrentPathStructIndex+1].m_NodeToPassThrough);
                        RemoteEnd = ngNode.GetCenter();
                    }
                }

                #ifdef AI_LOGGING
                LOG_MESSAGE( "character::UpdatePathing", "Moving to new position");
                #endif

                g_NavMap.GetClosestPointInOverlap( m_PathFindStruct.m_StepData[m_CurrentPathStructIndex].m_CurrentConnection, 
                                                                    m_PathFindStruct.m_StepData[m_CurrentPathStructIndex].m_DestConnection, 
                                                                    GetPosition(),
                                                                    RemoteEnd,
                                                                    GetLocoPointer()->m_Physics.GetColRadius(),
                                                                    vMoveTo );
            }
        }
        m_NextPathPoint = vMoveTo;
        CalculateSplinePoints();
        m_PreviousPathPoint = GetPosition();
    }
    UpdateSpline();
    if( m_CurrentSplinePoint >= 0 )
    {
        m_DesiredLocation = m_SplinePointList[m_CurrentSplinePoint];
    }
    else
    {    
        m_DesiredLocation = m_NextPathPoint;
    }
}

//========================================================================

void character::InitPathingHints( void )
{
    if (NULL == GetLocoPointer())
        return;
    if (!GetLocoPointer()->IsAnimLoaded())
        return;

    s32 iJumpOver = GetLocoPointer()->GetAnimIndex( loco::ANIM_JUMP_OVER );
    s32 iJumpUp   = GetLocoPointer()->GetAnimIndex( loco::ANIM_JUMP_UP   );
    s32 iJumpDown = GetLocoPointer()->GetAnimIndex( loco::ANIM_JUMP_DOWN );

    if ( (iJumpOver == -1) ||
         (iJumpUp   == -1) ||
         (iJumpDown == -1) )
    {
        // Do not allow jumping
        m_PathingHints.Jump = 0;
    }
    else
    {
        m_PathingHints.Jump = 128;
    }
}

//========================================================================
void character::HandleNavMapUpdate( nav_map::update_info& Update )
{
    s32 i;
    for (i=0;i<m_PathFindStruct.m_nSteps;i++)
    {
        if (m_PathFindStruct.m_StepData[i].m_CurrentConnection == Update.iUpdatedConnection)
        {
            // Invalidate the path!
            m_NeedsToRecalcPath = TRUE;
            break;
        }
    }
}

//========================================================================

void character::SetOverrideTarget( guid OverrideGuid )
{
    m_OverrideTargetGuid = OverrideGuid;
}

//========================================================================

xbool character::OfferOverrideTarget     ( guid OverrideGuid )
{
    if (0 != m_OverrideTargetGuid)
        return FALSE;

    SetOverrideTarget( OverrideGuid );
    return TRUE;
}

//========================================================================

const char* character::GetLogicalName( void )
{
    if( m_LogicalName >= 0 )
    {
        return g_StringMgr.GetString( m_LogicalName );
    }
    else
    {    
        return( "CHARACTER" );
    }
}

//==============================================================================

void character::CreateWeaponObject( void )
{
    if( m_WeaponItem != INVEN_NULL )
    {
        s32 Index = inventory2::ItemToWeaponIndex( m_WeaponItem );
        const char* pBlueprintName  = inventory2::ItemToBlueprintName( m_WeaponItem );

        if( pBlueprintName )
        {
            guid WeaponGUID = g_TemplateMgr.CreateSingleTemplate( pBlueprintName, vector3(0,0,0), radian3( 0,0,0 ), 0, 0 );
            if( WeaponGUID )
            {
                ASSERT( WeaponGUID );
                ASSERT( m_WeaponGuids[Index] == 0 );

                new_weapon* pWeapon = (new_weapon*)g_ObjMgr.GetObjectByGuid( WeaponGUID );
                ASSERT( pWeapon );

                pWeapon->InitWeapon( GetPosition(), new_weapon::RENDER_STATE_NPC, GetGuid() );
                pWeapon->BeginIdle();
                pWeapon->Reload( new_weapon::AMMO_PRIMARY );

                m_WeaponGuids[Index] = WeaponGUID;
                m_Inventory2.SetAmount( m_WeaponItem, 1.0f );
            }
        }
    }
}

//========================================================================

void character::InitInventory( void )
{
    if( !m_WeaponsCreated )
    {
        CreateWeaponObject();
        m_WeaponsCreated = TRUE;

        // TODO: CJ: WEAPONS: This shouldn't really be here, move it somewhere more appropriate
        // to the players life cycle

        // Start with the weapon
        m_CurrentWeaponItem = m_WeaponItem;

        // Give some grenades
        if( m_GrenadeItem != INVEN_NULL )
            m_Inventory2.AddAmount( m_GrenadeItem, 5.0f );
    }
}

//==============================================================================

f32 character::GetYaw()
{
    if( GetLocoPointer() )
    {    
        return GetLocoPointer()->GetYaw();
    }
    else
    {
        return actor::GetYaw();
    }
}

//==============================================================================

#ifdef cgalley
#define LOGGING_ENABLED 1
#else
#define LOGGING_ENABLED 0
#endif

#ifdef X_EDITOR

void character::EditorPreGame( void )
{
    actor::EditorPreGame();

    // Create weapon
    const char* pBlueprintName = inventory2::ItemToBlueprintName( m_WeaponItem );
    if( pBlueprintName )
    {
        s32 Index = inventory2::ItemToWeaponIndex( m_WeaponItem );
        guid Guid = g_TemplateMgr.EditorCreateSingleTemplateFromPath( pBlueprintName, vector3(0,0,0), radian3(0,0,0), -1, -1 ); 

        CLOG_MESSAGE( LOGGING_ENABLED, xfs("character::EditorPreGame %08x",(u32)this), "EditorCreateSingleTemplateFromPath '%s' %08x:%08x", pBlueprintName, Guid.GetHigh(), Guid.GetLow() );

        if( Guid )
        {
            new_weapon* pWeapon = (new_weapon*)g_ObjMgr.GetObjectByGuid( Guid );
            ASSERT( pWeapon );
            pWeapon->EditorPreGame();
            pWeapon->InitWeapon( GetPosition(), new_weapon::RENDER_STATE_NPC, GetGuid() );
            pWeapon->BeginIdle();
            g_ObjMgr.DestroyObjectEx(Guid,TRUE);
        }
    }

}

#endif // X_EDITOR

//========================================================================
