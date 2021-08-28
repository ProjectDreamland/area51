//==============================================================================
//
//  PlayerObject.cpp
// 
//==============================================================================
#if defined(bwatson)
#define X_SUPPRESS_LOGS
#endif
#include "player.hpp"
#include "InputMgr\GamePad.hpp"
#include "objects\ParticleEmiter.hpp"
#include "objects\Render\PostEffectMgr.hpp"
#include "objects\SpawnPoint.hpp"
#include "Objects\Event.hpp"
#include "Sound\EventSoundEmitter.hpp"
#include "..\support\templatemgr\TemplateMgr.hpp"
#include "characters\Character.hpp"
#include "Characters\Conversation_Packet.hpp"
#include "GameLib\StatsMgr.hpp"
#include "GameLib\RenderContext.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "objects\WeaponSniper.hpp"
#include "objects\ThirdPersonCamera.hpp"
#include "objects\WeaponSMP.hpp"
#include "objects\Corpse.hpp"
#include "NetworkMgr/NetObjMgr.hpp"
#include "NetworkMgr/Voice/VoiceMgr.hpp"
#include "Objects\Ladders\Ladder_Field.hpp"
#include "Objects\GrenadeProjectile.hpp"
#include "Objects\GravChargeProjectile.hpp"
#include "Objects\JumpingBeanProjectile.hpp"
#include "render\LightMgr.hpp"
#include "Objects\Door.hpp"
#include "objects\Projector.hpp"
#include "objects\WeaponMutation.hpp"
#include "StateMgr\StateMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "objects\HudObject.hpp"
#include "Characters\ActorEffects.hpp"
#include "Configuration/GameConfig.hpp"
#include "objects\turret.hpp"
#include "objects\WeaponShotgun.hpp"
#include "Gamelib/DebugCheats.hpp"
#include "objects\FocusObject.hpp"
#include "PerceptionMgr\PerceptionMgr.hpp"
#include "Objects\LoreObject.hpp"
#include "Objects\Camera.hpp"
#include "Objects\LevelSettings.hpp"
#include "GameTextMgr\GameTextMgr.hpp"
#include "StringMgr\StringMgr.hpp"
#include "e_Audio.hpp"

#ifndef X_RETAIL
#include "InputMgr\Monkey.hpp"
#endif

#ifdef X_EDITOR
#include "../Apps/Editor/Project.hpp"
#else
#include "NetworkMgr\MsgMgr.hpp"
#include "Menu\DebugMenu2.hpp"
#endif

#ifdef cgalley
#define LOGGING_ENABLED 1
#else
#define LOGGING_ENABLED 0
#endif

mp_tweaks g_MPTweaks =
{
    FALSE,   // Active
    1.30f,   // JumpSpeed      
    1.45f,   // Gravity    
    0.10f    // AirControl
};

#if defined(aharp) && !defined(X_EDITOR)
#include "NetworkMgr\MsgMgr.hpp"
#include "GameTextMgr\GameTextMgr.hpp"
#endif

#ifndef X_EDITOR
#include "NetworkMgr\GameMgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "Objects\NetGhost.hpp"
#endif

//=========================================================================
// DEFINES and CONSTS
//=========================================================================

static const f32    k_PainParticleDisplace      = 20.0f;
static const f32    s_NotifyTime                = 1.f;
static const f32    k_Modfactor                 = 2.5f;
static const f32    DeathCamStartBackDist       = 800.0f;
static const f32    DeathCamEndDist             = 500.0f;
static const f32    ZERO                        = 0.00001f;

             f32     g_EnemyShootBBoxPct = 1.20f; // percent of screen bbox width to inflate for an aiming HIT bbox
//static      f32     s_AimAssistRadius   = 20.0f;
//static      f32     s_AimAssistDeadZone = 20.0f;
extern xbool g_MirrorWeapon;
extern mtwt  s_MapToWeaponTable[];
extern f32   g_SpawnFadeTime;

#if defined( TARGET_PS2 ) && !defined( CONFIG_RETAIL )
extern xbool g_FreeCam;
#endif

view player::m_Views[MAX_LOCAL_PLAYERS];

static const f32 s_run_state_transition_speed   = 150.f;
static const f32 DEATH_VIEW_YAW_DELAY           = 0.5f; // seconds before camera starts rotating
f32 TESTTIME = 0.f;
f32 TESTTIMETWO = 0.f;

xbool s_WeaponInventoryStrip    = FALSE;

f32 s_MaxPainTime               = 2.0f;

// flashlight/batter stuff
f32 BATTERY_BURN_SECONDS        =      2.0f;
f32 BATTERY_BURN_AMOUNT         =      3.0f; // amount of burn per BATTERY_BURN_SECONDS

f32 BATTERY_GAIN_SECONDS        =      2.0f; // gain per x seconds
f32 BATTERY_GAIN_AMOUNT         =      4.0f; // amount of gain per BATTERY_GAIN_SECONDS

extern tweak_handle Lore_Max_Detect_DistanceTweak; // what is the max distance at which our Geiger will pick up a lore object

// while in mutant form
tweak_handle MutagenChangeMutant_AtWill_Tweak   ("Mutagen_Change_Mutant_At_Will");    // percent of mutagen change per second in At Will MP mode.
tweak_handle MutagenChangeMutant_Forced_Tweak   ("Mutagen_Change_Mutant_Forced");     // percent of mutagen change per second in Force MP mode.
tweak_handle MutagenChangeMutant_Campaign_Tweak ("Mutagen_Change_Mutant_Campaign");   // percent of mutagen change per second in campaign mode.

// While in human form
tweak_handle MutagenChangeHuman_AtWill_Tweak    ("Mutagen_Change_Human_At_Will");    // percent of mutagen change per second in At Will MP mode.
tweak_handle MutagenChangeHuman_Forced_Tweak    ("Mutagen_Change_Human_Forced");     // percent of mutagen change per second in Force MP mode.
tweak_handle MutagenChangeHuman_Campaign_Tweak  ("Mutagen_Change_Human_Campaign");   // percent of mutagen change per second in campaign mode.

// message timing tweaks
tweak_handle Item_Full_Msg_FadeTimeTweak  ("Item_Full_Msg_FadeTime");
tweak_handle Item_Full_Msg_DelayTimeTweak ("Item_Full_Msg_DelayTime");

tweak_handle Item_Acquired_Msg_FadeTimeTweak  ("Item_Acquired_Msg_FadeTime");
tweak_handle Item_Acquired_Msg_DelayTimeTweak ("Item_Acquired_Msg_DelayTime");

//==============================================================================
// CONSTANTS FOR FOOTFALL AUDIO
//==============================================================================
static const f32 kForwardDelay     = 250.0f;
static const f32 kBackwardDelay    = 175.0f;
static const f32 kFalloff          = 0.85f;
static const f32 kMaxForwardVel    = 6.0f;
static const f32 kFeetsPerInitStep = 3.0f;
static const f32 kFeetsSpeedMod    = 4.0f;
static const f32 kMaxWalkVolume    = 0.5f;
static const f32 kMaxRunVolume     = 1.0f;
static const f32 kMinRunVolume     = 0.65f;
static const f32 kMaxWalkVel       = 2.5f;
static const f32 kMaxStrafeDelay   = 500.0f;
static const f32 kMinStrafeDelay   = 250.0f;
static const f32 kStrafeInit       = 0.5f;
static const f32 kVertStrafeCutOff = 0.3f;
static const f32 kLowestVolume     = 0.1f;

f32 g_GrenadeThrowForce   = 2000.0f;
#if defined(X_EDITOR)
xbool g_ShowPlayerPos     = TRUE;
#else
xbool g_ShowPlayerPos     = FALSE;
#endif // X_EDITOR

xbool g_RenderTendrilCollision = FALSE;

extern xbool    g_game_running;

//=============================================================================
// Aim and bullet assist stuff
//=============================================================================

// debug menu aim assist stuff
#if !defined( CONFIG_RETAIL )
extern xbool        g_AimAssist_Render_Reticle;
extern xbool        g_AimAssist_Render_Bullet;
extern xbool        g_AimAssist_Render_Turn;
extern xbool        g_AimAssist_Render_Bullet_Angle;
extern xbool        g_AimAssist_Render_Player_Pills;
#endif // !defined( CONFIG_RETAIL )

struct convulsion_tweaks
{
    f32 m_MutagenConvulsionMultiplierPeriod;
    f32 m_MinConvulsionPeriod;
    f32 m_ConvulsionDuration;
} g_ConvulsionTweaks =
{
    0.2f,
    2.0f,
    2.0f
};

// structure constructor
AimAssistData::AimAssistData( void )
{
    BulletAssistDir         = vector3(0.0f, 0.0f, 0.0f);
    bReticleOn              = FALSE;
    BulletAssistBestDist    = F32_MAX;
    TurnDampeningT          = 0.0f;
    TargetGuid              = 0;
    LOFCollisionDist        = 2500.0f;
    LOFSpineDist            = 0.0f;
    SpinePt                 = vector3(0.0f, 0.0f, 0.0f);
    LOFPt                   = vector3(0.0f, 0.0f, 0.0f);
    LOFPtT                  = 0.0f;
    SpinePtT                = 0.0f;
    LOFPtDist               = 1.0f;

    ReticleRadius           = 0.0f;    
    BulletInnerRadius       = 0.0f;
    BulletOuterRadius       = 0.0f;
    TurnInnerRadius         = 0.0f;
    TurnOuterRadius         = 0.0f;  

    // online stuff
    OnlineFriendlyTargetGuid= 0;
    AimDelta                = vector3(0.0f, 0.0f, 0.0f);   
}

// tweak values
f32 AimAssist_LOF_Dist;

f32 AimAssist_Reticle_Near_Dist;
f32 AimAssist_Reticle_Far_Dist;
f32 AimAssist_Reticle_Near_Radius;
f32 AimAssist_Reticle_Far_Radius;

f32 AimAssist_Bullet_Inner_Near_Dist;
f32 AimAssist_Bullet_Inner_Far_Dist;
f32 AimAssist_Bullet_Inner_Near_Radius;
f32 AimAssist_Bullet_Inner_Far_Radius;

f32 AimAssist_Bullet_Outer_Near_Dist;
f32 AimAssist_Bullet_Outer_Far_Dist;
f32 AimAssist_Bullet_Outer_Near_Radius;
f32 AimAssist_Bullet_Outer_Far_Radius;

f32 AimAssist_Bullet_Angle;

f32 AimAssist_Turn_Inner_Near_Dist;
f32 AimAssist_Turn_Inner_Far_Dist;
f32 AimAssist_Turn_Inner_Near_Radius;
f32 AimAssist_Turn_Inner_Far_Radius;

f32 AimAssist_Turn_Outer_Near_Dist;
f32 AimAssist_Turn_Outer_Far_Dist;
f32 AimAssist_Turn_Outer_Near_Radius;
f32 AimAssist_Turn_Outer_Far_Radius;

f32 AimAssist_Turn_Damp_Near_Dist;
f32 AimAssist_Turn_Damp_Far_Dist;


// Tweak handles
tweak_handle AimAssist_LOF_Dist_Tweak;

tweak_handle AimAssist_Reticle_Near_Dist_Tweak ;
tweak_handle AimAssist_Reticle_Far_Dist_Tweak;
tweak_handle AimAssist_Reticle_Near_Radius_Tweak;
tweak_handle AimAssist_Reticle_Far_Radius_Tweak;

tweak_handle AimAssist_Bullet_Inner_Near_Dist_Tweak;
tweak_handle AimAssist_Bullet_Inner_Far_Dist_Tweak;
tweak_handle AimAssist_Bullet_Inner_Near_Radius_Tweak;
tweak_handle AimAssist_Bullet_Inner_Far_Radius_Tweak;

tweak_handle AimAssist_Bullet_Outer_Near_Dist_Tweak;
tweak_handle AimAssist_Bullet_Outer_Far_Dist_Tweak;
tweak_handle AimAssist_Bullet_Outer_Near_Radius_Tweak;
tweak_handle AimAssist_Bullet_Outer_Far_Radius_Tweak;

tweak_handle AimAssist_Bullet_Angle_Tweak;

tweak_handle AimAssist_Turn_Inner_Near_Dist_Tweak;
tweak_handle AimAssist_Turn_Inner_Far_Dist_Tweak;
tweak_handle AimAssist_Turn_Inner_Near_Radius_Tweak;
tweak_handle AimAssist_Turn_Inner_Far_Radius_Tweak;

tweak_handle AimAssist_Turn_Outer_Near_Dist_Tweak;
tweak_handle AimAssist_Turn_Outer_Far_Dist_Tweak;
tweak_handle AimAssist_Turn_Outer_Near_Radius_Tweak;
tweak_handle AimAssist_Turn_Outer_Far_Radius_Tweak;

tweak_handle AimAssist_Turn_Damp_Near_Dist_Tweak;
tweak_handle AimAssist_Turn_Damp_Far_Dist_Tweak;

// flashlight tweaks
tweak_handle FlashlightAutoOffSecondsTweak("FlashlightAutoOffSeconds");        // how long before we turn off flashlight
tweak_handle FlashlightAutoOffBrightnessTweak("FlashlightAutoOffBrightness");  // what is the brightness level we shut the flashlight off at


tweak_handle Mutagen_Convulsion_Color_R( "MUTATION_ConvulsionColorR" );
tweak_handle Mutagen_Convulsion_Color_G( "MUTATION_ConvulsionColorG" );
tweak_handle Mutagen_Convulsion_Color_B( "MUTATION_ConvulsionColorB" );

void player::LoadAimAssistTweakHandles( void )
{
    // get weapon pointer
    new_weapon* pWeapon = (new_weapon*)g_ObjMgr.GetObjectByGuid( m_WeaponGuids[inventory2::ItemToWeaponIndex(m_CurrentWeaponItem)] );
    if( !pWeapon )
    {
        return;
    }

    m_bTweakHandlesLoaded = TRUE;

    // load up strings
    xstring StringPrefix = (const char*)xfs( "%s_%d", pWeapon->GetLogicalName(), pWeapon->GetZoomStep() );
    const char* pString = (const char*)StringPrefix;

    AimAssist_LOF_Dist_Tweak.SetName                ( xfs("%s_LOF_Dist", pString) );

    AimAssist_Reticle_Near_Dist_Tweak.SetName       ( xfs("%s_Reticle_Near_Dist", pString) );
    AimAssist_Reticle_Far_Dist_Tweak.SetName        ( xfs("%s_Reticle_Far_Dist", pString) );
    AimAssist_Reticle_Near_Radius_Tweak.SetName     ( xfs("%s_Reticle_Near_Rad", pString) );
    AimAssist_Reticle_Far_Radius_Tweak.SetName      ( xfs("%s_Reticle_Far_Rad", pString) );

    AimAssist_Bullet_Inner_Near_Dist_Tweak.SetName  ( xfs("%s_Blt_In_Near_Dist", pString) );
    AimAssist_Bullet_Inner_Far_Dist_Tweak.SetName   ( xfs("%s_Blt_In_Far_Dist", pString) );
    AimAssist_Bullet_Inner_Near_Radius_Tweak.SetName( xfs("%s_Blt_In_Near_Rad", pString) );
    AimAssist_Bullet_Inner_Far_Radius_Tweak.SetName ( xfs("%s_Blt_In_Far_Rad", pString) );
       
    AimAssist_Bullet_Outer_Near_Dist_Tweak.SetName  ( xfs("%s_Blt_Out_Near_Dist", pString) );
    AimAssist_Bullet_Outer_Far_Dist_Tweak.SetName   ( xfs("%s_Blt_Out_Far_Dist", pString) );
    AimAssist_Bullet_Outer_Near_Radius_Tweak.SetName( xfs("%s_Blt_Out_Near_Rad", pString) );
    AimAssist_Bullet_Outer_Far_Radius_Tweak.SetName ( xfs("%s_Blt_Out_Far_Rad", pString) );

    AimAssist_Bullet_Angle_Tweak.SetName            ( xfs("%s_Blt_Angle", pString) );

    AimAssist_Turn_Inner_Near_Dist_Tweak.SetName    ( xfs("%s_Turn_In_Near_Dist", pString) );
    AimAssist_Turn_Inner_Far_Dist_Tweak.SetName     ( xfs("%s_Turn_In_Far_Dist", pString) );
    AimAssist_Turn_Inner_Near_Radius_Tweak.SetName  ( xfs("%s_Turn_In_Near_Rad", pString) );
    AimAssist_Turn_Inner_Far_Radius_Tweak.SetName   ( xfs("%s_Turn_In_Far_Rad", pString) );

    AimAssist_Turn_Outer_Near_Dist_Tweak.SetName    ( xfs("%s_Turn_Out_Near_Dist", pString) );
    AimAssist_Turn_Outer_Far_Dist_Tweak.SetName     ( xfs("%s_Turn_Out_Far_Dist", pString) );
    AimAssist_Turn_Outer_Near_Radius_Tweak.SetName  ( xfs("%s_Turn_Out_Near_Rad", pString) );
    AimAssist_Turn_Outer_Far_Radius_Tweak.SetName   ( xfs("%s_Turn_Out_Far_Rad", pString) );

    // how much emphasis on turn dampening at close ranges?
    AimAssist_Turn_Damp_Near_Dist_Tweak.SetName  ( xfs("%s_Turn_Damp_Near_Dist", pString) );
    AimAssist_Turn_Damp_Far_Dist_Tweak.SetName  ( xfs("%s_Turn_Damp_Far_Dist", pString) );
}

void player::LoadAimAssistTweaks( void )
{
    // make sure weapon has been loaded
    new_weapon* pWeapon = (new_weapon*)g_ObjMgr.GetObjectByGuid( m_WeaponGuids[inventory2::ItemToWeaponIndex(m_CurrentWeaponItem)] );
    if( !pWeapon || !m_bTweakHandlesLoaded )
    {
        return;
    }

    AimAssist_LOF_Dist                  = AimAssist_LOF_Dist_Tweak.GetF32();

    AimAssist_Reticle_Near_Dist         = AimAssist_Reticle_Near_Dist_Tweak.GetF32();
    AimAssist_Reticle_Far_Dist          = AimAssist_Reticle_Far_Dist_Tweak.GetF32();
    AimAssist_Reticle_Near_Radius       = AimAssist_Reticle_Near_Radius_Tweak.GetF32();
    AimAssist_Reticle_Far_Radius        = AimAssist_Reticle_Far_Radius_Tweak.GetF32();

    AimAssist_Bullet_Inner_Near_Dist    = AimAssist_Bullet_Inner_Near_Dist_Tweak.GetF32();
    AimAssist_Bullet_Inner_Far_Dist     = AimAssist_Bullet_Inner_Far_Dist_Tweak.GetF32();
    AimAssist_Bullet_Inner_Near_Radius  = AimAssist_Bullet_Inner_Near_Radius_Tweak.GetF32();
    AimAssist_Bullet_Inner_Far_Radius   = AimAssist_Bullet_Inner_Far_Radius_Tweak.GetF32();

    AimAssist_Bullet_Outer_Near_Dist    = AimAssist_Bullet_Outer_Near_Dist_Tweak.GetF32();
    AimAssist_Bullet_Outer_Far_Dist     = AimAssist_Bullet_Outer_Far_Dist_Tweak.GetF32();
    AimAssist_Bullet_Outer_Near_Radius  = AimAssist_Bullet_Outer_Near_Radius_Tweak.GetF32();
    AimAssist_Bullet_Outer_Far_Radius   = AimAssist_Bullet_Outer_Far_Radius_Tweak.GetF32();

    AimAssist_Bullet_Angle              = AimAssist_Bullet_Angle_Tweak.GetF32();

    AimAssist_Turn_Inner_Near_Dist      = AimAssist_Turn_Inner_Near_Dist_Tweak.GetF32();
    AimAssist_Turn_Inner_Far_Dist       = AimAssist_Turn_Inner_Far_Dist_Tweak.GetF32();
    AimAssist_Turn_Inner_Near_Radius    = AimAssist_Turn_Inner_Near_Radius_Tweak.GetF32();
    AimAssist_Turn_Inner_Far_Radius     = AimAssist_Turn_Inner_Far_Radius_Tweak.GetF32();

    AimAssist_Turn_Outer_Near_Dist      = AimAssist_Turn_Outer_Near_Dist_Tweak.GetF32();
    AimAssist_Turn_Outer_Far_Dist       = AimAssist_Turn_Outer_Far_Dist_Tweak.GetF32();
    AimAssist_Turn_Outer_Near_Radius    = AimAssist_Turn_Outer_Near_Radius_Tweak.GetF32();
    AimAssist_Turn_Outer_Far_Radius     = AimAssist_Turn_Outer_Far_Radius_Tweak.GetF32();

    AimAssist_Turn_Damp_Near_Dist       = AimAssist_Turn_Damp_Near_Dist_Tweak.GetF32();
    AimAssist_Turn_Damp_Far_Dist        = AimAssist_Turn_Damp_Far_Dist_Tweak.GetF32();
}

//=========================================================================
// PLAYER
//=========================================================================

static struct player_desc : public object_desc
{
    player_desc( void ) : object_desc( 
            object::TYPE_PLAYER, 
            "Player", 
            "ACTOR",
            object::ATTR_PLAYER                 |
            object::ATTR_NEEDS_LOGIC_TIME       |
            object::ATTR_COLLIDABLE             | 
            object::ATTR_BLOCKS_ALL_PROJECTILES | 
            object::ATTR_BLOCKS_RAGDOLL         | 
            object::ATTR_BLOCKS_CHARACTER_LOS   | 
            object::ATTR_BLOCKS_PLAYER_LOS      | 
            object::ATTR_BLOCKS_SMALL_DEBRIS    | 
            object::ATTR_RENDERABLE             | 
            object::ATTR_CAST_SHADOWS           |
            object::ATTR_SPACIAL_ENTRY          | 
            object::ATTR_DAMAGEABLE             |
            object::ATTR_TRANSPARENT            |
            object::ATTR_LIVING,

            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_IS_DYNAMIC )
            {}

    virtual object* Create( void ) { return new player; }

#ifdef X_EDITOR
    s32 OnEditorRender( object& Object ) const
    {
        player& Player = player::GetSafeType( Object );
        Player.OnEditorRender();

        (void)Object;
        //if( Object.GetAttrBits() & object::ATTR_EDITOR_SELECTED )
        //    Object.OnDebugRender();

        return -1;
    }
#endif // X_EDITOR

} s_Player;

//=========================================================================
// VARIABLES
//=========================================================================
guid     player::s_ActivePlayerGuid( 0 );
xbool    player::s_bPlayerDied( FALSE );

//=========================================================================
// FUNCTIONS
//=========================================================================


//=========================================================================
player* player::GetActivePlayer( void )
{
    return (player*)g_ObjMgr.GetObjectByGuid(s_ActivePlayerGuid);
}

//=========================================================================

player::player( void ) : 
    m_RespawnPosition(0,0,0),
    m_RespawnZone(0),
    m_ThirdPersonCameraGuid( 0 ),
    m_PitchAccelTime( 3.f ),
    m_YawAccelTime( 3.f ),
    m_PitchRate( 0.0f ),
    m_YawRate( 0.0f ),
    m_PitchAccelFactor( 0.0f ),
    m_YawAccelFactor( 0.0f ),
    m_PitchMax( R_87 ),
    m_PitchMin( -R_87 ),
    m_DesiredPitchMax( R_87 ),
    m_DesiredPitchMin( -R_87 ),
    m_fYawStickSensitivity( 1.05f ),
    m_fPitchStickSensitivity( 1.0f ),
    m_fOriginalYawStickSensitivity( 1.0f ),
    m_fOriginalPitchStickSensitivity( 1.0f ),
    m_EyesOffset( 0.0f, -25.0f, -5.0f ),
    m_ShakeTime( 0.0f ),
    m_ShakeAngle( 0.0f ),
    m_ShakeAmount(0),
    m_ShakeSpeed(0),
    m_ActivePlayerPad(-1),
    m_LocalSlot( -1 ),
    m_MaxFowardVelocity( 100.0f ),
    m_JumpVelocity( 200.0f ),
    m_ForwardVelocity( 0.0f , 0.0f , 0.0f ),
    m_StrafeVelocity( 0.0f , 0.0f , 0.0f ),
    m_fForwardAccel( 1000.0f ),
    m_fStrafeAccel( 1000.0f ),
    m_fForwardSpeed( 0.0f ),
    m_fCurrentYawOffset( 0.0f ),
    m_fCurrentPitchOffset( 0.0f ),
    m_fPitchChangeSpeed( PI * .45f ),
    m_fPrevForwardSpeed( 0.0f ),
    m_fStrafeSpeed( 0.0f ),
    m_fDecelerationFactor( 2.5f ),
    m_SoftLeanAmount                    ( 0.0f                  ),
    m_LeanWeaponOffset                  ( 0.0f, 0.0f, 0.0f      ),
    m_PitchArmsScalerPositive           ( 1.0f                  ),
    m_PitchArmsScalerNegative           ( 1.0f                  ),
    m_fCurrentCrouchFactor              ( 0.0f                  ),
    m_fCrouchChangeRate                 ( 10.0f                 ),
    m_bInTurret                         ( FALSE                 ),
    m_pPlayerTitle( NULL ),
    m_fMinWalkSpeed( 0.0f ),
    m_fMinRunSpeed( 0.0f ),
    m_NonExclusiveStateBitFlag( NE_STATE_NULL ),
    m_PreStunPitch(0),
    m_PreStunYaw(0),
    m_PreStunRoll(0),
    m_fStunnedTime(0),
    m_fShakeAmpScalar( 1.f ),
    m_fShakeFreqScalar( 1.f ),
    m_fShakeMaxPitch( 5.f ),
    m_fShakeMaxYaw( 2.f ),
    m_bAllLoreObjectsCollected( FALSE ),
    m_ReticleMovementDegrade( 0.5f ),
    m_InvalidSoundTimer( 0.0f ),
    m_fRigMoveOffsetVelocity( 10.0f ),
    m_fRigStrafeOffsetVelocity( 20.0f ),
    m_fCurrentMoveRigOffset( 0.0f ),
    m_fCurrentStrafeRigOffset( 0.0f ),
    m_RigLookOffset( radian3( 0.0f, 0.0f, 0.0f ) ),
    m_RigLookMaxVertOffset( R_2 ),
    m_RigLookMaxHorozOffset( R_3 ),
    m_RigLookVertVelocity( R_10 ),
    m_RigLookHorozVelocity( R_20 ),
    m_CurrentVertRigOffset( R_0 ),
    m_CurrentHorozRigOffset( R_0 ),
    m_fCurrentPitchAimModifier( 1.f ),
    m_fCurrentYawAimModifier( 1.f ),
    m_fFineTuneThreshold( 0.4f ),
    m_fYawValueAtFineTuneThreshold( 0.15f ),
    m_fPitchValueAtFineTuneThreshold( 0.15f ),
    m_fMidRangeThreshold( 0.99f ),
    m_fYawValueAtMidrangeThreshold( 0.6f ),
    m_fPitchValueAtMidrangeThreshold( 0.6f ),
    m_YawAimOffset( R_0 ),
    m_TimeSinceLastZonePain( 0.0f ),
    m_fMoveValue( 0.0f ),
    m_fStrafeValue( 0.0f ),
    m_fYawValue( 0.0f ),
    m_fPitchValue( 0.0f ),
    m_fRawControllerYaw(0),
    m_fRawControllerPitch(0),
    m_bVoteButtonPressed(FALSE),
    m_bRespawnButtonPressed(FALSE),
    m_fPreviousYawValue(0),
    m_fPreviousPitchValue(0),
    m_LastTimeSeenByEnemy(0.0f),
    m_LastTimeTookDamage(0.0f),
    m_PositionOfLastSafeSpot(0,0,0),
    m_ZoneIDOfLastSafeSpot(0),
    m_NextPositionOfLastSafeSpot(0,0,0),
    m_NextZoneIDOfLastSafeSpot(0),
    m_AimDegradation(0.0f),
    m_AimRecoverSpeed(0.0f),
    m_YawMod(0.0f),
    m_PitchMod(0.0f),
    m_RollMod(0.0f),
    m_ShakePitch(0.0f),
    m_ShakeYaw(0.0f),
    m_NearbyObjectCounter(0),
    m_GameSpeakCounter(0),
    m_SpeakToGuid( 0 ),
    m_GameSpeakEmitterGuid( 0 ),
    m_ProximityAlertRadius( 300.0f ),
    m_TimeStartTick                     ( 0 ),
    m_ViewCinematicPlaying              (FALSE),
    m_CurrentViewNode                   ( 0 ),
    m_StartView                         ( 0, 0, 0, 1 ),
    m_DesiredView                       ( 0, 0, 0, 1 ),
    m_CScale                            ( 1.0f ),
    m_CTimeSum                          ( 0.0f ),
    m_bSpeaking                         ( FALSE ),
    m_LastLadderGuid                    ( 0 ),
    m_JumpedOffLadderGuid               ( 0 ),
    m_WeaponState                       ( WEAPON_STATE_NONE ),
    m_ReticleRadius                     ( 0.0f ),
    m_ReticleGrowSpeed                  ( 0.0f ),    
    m_ArmsOffset                        ( 0.0f, 0.0f, 0.0f ),
    m_ArmsVelocity                      ( 0.0f, 0.0f, 0.0f ),
    m_WeaponCollisionOffset             ( 0.0f, 0.0f, 0.0f ),
    m_LastWeaponCollisionOffsetScalar   ( 0.0f ),
    m_WeaponCollisionOffsetScalar       ( 0.0f ),
    m_FlashlightGuid                    ( 0 ),
    m_BatteryChangeTime                 ( 0.0f ),
    m_Battery                           ( 100.0f ),
    m_MaxBattery                        ( 100.0f ),
    m_FlashlightTimeout                 ( 0.0f),
    m_fLastItemFullTime                 ( 0.0f ),
    m_fLastItemAcquiredTime             ( 0.0f ),
    m_bStrainInitialized                ( FALSE ),
    m_CurrentAnimState                  ( ANIM_STATE_UNDEFINED ),
    m_PreviousAnimState                 ( ANIM_STATE_UNDEFINED ),
    m_NextAnimState                     ( ANIM_STATE_UNDEFINED ),
    m_AnimStage                         ( ANIM_STATE_UNDEFINED ),
    m_MeleeAnimStateIndex               ( -1 ),
    m_CurrentAnimIndex                  ( -1 ),
    m_PreviousAnimIndex                 ( -1 ),
    m_CurrentAnimStateIndex             ( -1 ),
    m_PreviousAnimStateIndex            ( -1 ),
    m_fAnimationTime                    ( 0.0f ),
    m_fMaxAnimTime                      ( 0.0f ),
    m_WpnHoldTime                       ( 0.0f ),
    m_LastTimeWeaponFired               ( 0.0f ),
    m_bOnLadder                         ( FALSE ),
    m_LadderOutDir                      ( 0, 0, 0 ),
    m_MaxAnimWeaponHoldTime             ( 20.0f ),
    m_nLoreDiscoveries                  ( 0 ),
    m_DebounceTime                      ( 0.0f ),
    m_bWasMutated                       ( FALSE ),
    m_bIsMutantVisionOn                 ( FALSE ),
    m_PreMutationWeapon2                ( INVEN_NULL ),
    m_bMutationMeleeEnabled             ( FALSE ),
    m_bPrimaryMutationFireEnabled       ( FALSE ),
    m_bSecondaryMutationFireEnabled     ( FALSE ),
    m_bMeleeLunging                     ( FALSE ),
    m_bHolsterWeapon                    ( FALSE ),
    m_MeleeDamage                       ( 120.0f ),
    m_MeleeForce                        ( 30.0f ),
    m_bInMutationTutorial               ( FALSE ),
    m_ConvulsionFeedbackDuration        ( 0.5f ),
    m_ConvulsionFeedbackIntensity       ( 1.0f ),
    m_bHitCombo                         ( FALSE ),
    m_bCanRequestCombo                  ( FALSE ),
    m_bLastMeleeHit                     ( FALSE ),
    m_ComboCount                        ( 0 ),

    m_bTweakHandlesLoaded               ( FALSE ),
    
    // From ghost.cpp
    m_Mutagen                           ( 100.0f ),
    m_MaxMutagen                        ( 100.0f ),
    m_EyesPosition                      ( 0.0f, 0.0f, 0.0f ),
    m_EyesPitch                         ( 0.0f ),
    m_EyesYaw                           ( 0.0f ),
    m_SuckingMutagenLoopID              ( 0 ),
#if !defined( CONFIG_RETAIL )
    m_bRenderSkeleton                   ( FALSE ),
    m_bRenderSkeletonNames              ( TRUE ),
    m_bRenderBBox                       ( TRUE ),
#endif // !defined( CONFIG_RETAIL )
    m_PrevWeaponItem                    ( INVEN_NULL ),
    m_NextWeaponItem                    ( INVEN_NULL ),
    m_bJustLanded                       ( FALSE ),
    m_DeltaPos                          ( 0.0f, 0.0f, 0.0f ),
    m_bCanJump                          ( TRUE ),
    m_DeltaTime                         ( 0.0f ),
    m_TimeInState                       ( 0.0f ),
    m_MissionFailedTableName            ( -1 ),
    m_MissionFailedReasonName           ( -1 ),
    m_VoteMode                          ( FALSE ),
    m_DelayTillNextStep                 ( 0.0f ),
    m_DistanceTraveled                  ( 0.0f ),
    m_DelayCountDown                    ( 0.0f ),
    m_HeelID                            ( 0 ),
    m_SlideID                           ( 0 ),
    m_ToeID                             ( 0 ),
    m_TrailStep                         ( 0 ),
    m_MutationChangeTime                ( 0.0f ),
    m_UseTime                           ( 0.0f )
{
    s32 i;

    // SB: Initialize all cinema vars to zero
    x_memset( &m_Cinema, 0, sizeof( m_Cinema ) );

    SetIsActive( TRUE );

    InitializeMeleeAnimStateList();

    m_MaxStrafeVelocity = .75f * m_MaxFowardVelocity;

    m_vRigOffset.Set( 0.0f, 0.0f, 0.0f );
 
    // The title for this player
    m_pPlayerTitle = "Unknown Mutation";
    
    m_PeakLandVelocity      = -1.0f;
    m_PeakJumpVelocity      = -1.0f;

    // Get the players ear id.
    m_AudioEarID = g_AudioMgr.CreateEar();

    m_RespawnZone = 0;
    
    m_SpeakToGuid = 0 ;
    m_CurrentTargetingModifation.Zero();
    m_OffsetToTarget.Zero();

    m_bActivePlayer = TRUE;

    m_fRigMaxMoveOffset = 2.0f;
    m_fRigMaxStrafeOffset = 3.0f;

    m_bHidePlayerArms = FALSE;
    m_bArmsWereHidden = FALSE;
    m_bPlaySwitchTo   = TRUE;

    // cinema stuff
    m_Cinema.m_bCinemaOn                = FALSE;
    m_Cinema.m_bPlayerZoneInitialized   = FALSE;
    m_Cinema.m_LookAtTargetGuid         = 0;
    m_Cinema.m_BlendInTime              = 3.0f;
    m_Cinema.m_CurrentBlendInTime       = 0.0f;

    m_iCameraBone = -1;
    m_iCameraTargetBone = -1;

    // Initialize our arrays.
    x_memset( m_fAnimPriorityPercentage, 0, sizeof( f32 ) * MAX_ANIM_PER_STATE );

    m_StrainFriendFlags = 0;
    
    m_CurrentGrenadeType2           = INVEN_GRENADE_FRAG;
    m_PlayMeleeSound                = TRUE;
    m_IsRunning                     = FALSE;
    m_MutationAudioLoopSfx          = 0;
    m_NeedRelaodIn                  = TRUE;
    m_LastFireAnimStateIndex        = 0;
    m_bUsingFlashlight              = FALSE;
    m_bUsingFlashlightBeforeCinema  = FALSE;

    // Reset all weapon anim tables
    for( i=0; i<INVEN_NUM_WEAPONS; i++ )
    {
        inven_item WeaponItem = inventory2::WeaponIndexToItem(i);
        ResetWeaponAnimTable2( WeaponItem );
    }

    // Clear the last pain event.
    m_LastPainEvent.Clear();
    m_LastPainEvent.SetGrowAmount( 4 );

    m_CurrentAnimState  = ANIM_STATE_UNDEFINED;

    // on initialize, the JBG is not in expert mode
    m_bJBGLoreAcquired = FALSE;

    // initialize tap fire time
    m_TapRefireTime = 0.0f;

    // the first time, don't let it "double fire"
    m_bCanTapFire = FALSE;

    // Initalize the FlyBys!
    for( i=0 ; i<MAX_FLY_BYS ; i++ )
    {
        x_memset( (void*)&m_BulletFlyBy[i], 0, sizeof(m_BulletFlyBy[i]) );
    }

    LOG_MESSAGE( "player::player", "Addr:%08X", this );

    //#ifndef X_EDITOR
    //s32 nPlayers = g_StateMgr.GetPlayerCount();
    //LOG_MESSAGE( "player::player", 
    //             "Addr:%08X - LocalSlot:%d - TotalLocalPlayers:%d - NetSlot:%d",
    //             this, m_LocalSlot, nPlayers, m_NetSlot );
    //#endif
    

    //==========================================================================
    // Begin code from ghost.cpp
    //==========================================================================

    m_Faction     = FACTION_PLAYER_NORMAL;
    m_FriendFlags = FACTION_WORKERS;   

    // Start out DEAD.  Then spawn.
    m_Health.Dead();
    m_bDead        = TRUE;
    m_bWantToSpawn = TRUE;

    #ifndef X_EDITOR
    actor::m_Net.LifeSeq = -1;  // Odd number means 'dead'.
    #endif

    // Setup pointer to loco for base class to use.
    m_pLoco = &m_Loco;
    m_pLoco->SetGhostMode( TRUE );  // Player controls movement, not animations
   
    //==========================================================================
    // End code from ghost.cpp
    //==========================================================================

#ifndef X_EDITOR
    if( g_StateMgr.UseDefaultLoadOut() )
    {
        DebugSetupInventory( g_ActiveConfig.GetLevelPath() );
    }
#else
    DebugSetupInventory( "<null>" );
#endif

    m_Turret.TurretGuid     = 0;
    m_Turret.Turret2Guid     = 0;
    m_Turret.Turret3Guid     = 0;
    m_Turret.PreviousWeapon = INVEN_WEAPON_SMP;
    m_Turret.AnchorL2W.Identity();

    for( i = 0; i < MAX_LORE_ITEMS; i++ )
    {
        m_LoreObjectGuids[i] = NULL_GUID;
    }

    // KSS -- new cinema code
    {
        m_Cinema.m_ViewCorrectionDelta.Zero();        
        m_Cinema.m_CinemaCameraGuid     = 0;
        m_Cinema.m_bUseViewCorrection   = FALSE;
    }

    //-- Mission Failer code
    {
       m_MissionFailedBmp.SetName( PRELOAD_FILE("UI_Mission_failed.xbmp") );
    }  
}

//===========================================================================
    
player::strain_control_modifiers::strain_control_modifiers() :
    m_StrainProximityAlertRadius( 300.0f ),
    m_StrainMaxFowardVelocity( 600.f ),
    m_StrainMaxStrafeVelocity( 450.f ),
    m_StrainJumpVelocity( 500.f ),
    m_StrainMaxHealth( 100.f ), 
//  m_StrainStickSensitivity( 1.0f ),
    m_StrainMinWalkSpeed( 200.0f ),
    m_StrainMinRunSpeed( 400.0f ), 
    m_StrainDecelerationFactor( 2.5f ),   
    m_StrainCrouchChangeRate( 10.0f ),
    m_StrainReticleMovementDegrade( 0.5f ),
    m_fStrainForwardAccel( 3000.0f ),
    m_fStrainStrafeAccel( 2000.0f ),
    m_fStrainYawSensitivity( 5.5f ),
    m_fStrainPitchSensitivity( 1.5f ),
    m_StrainYawAccelTime( 1.5f ),
    m_StrainPitchAccelTime( 0.2f )
{
    m_StrainEyesOffset.Set(0,-25,-20);
}

//=========================================================================

player::~player( void )
{
    SetIsActive( FALSE );

    // Destroy flashlight
    object* pFlashlight = m_FlashlightGuid != 0 ? g_ObjMgr.GetObjectByGuid( m_FlashlightGuid ) : NULL;
    
    if ( pFlashlight != NULL )
    {
        g_ObjMgr.DestroyObjectEx( m_FlashlightGuid, TRUE );
    }

    m_FlashlightGuid = 0;

    // Remove the player's ear from the audio manager.
    g_AudioMgr.DestroyEar( m_AudioEarID );
}

//==============================================================================

const object_desc& player::GetTypeDesc( void ) const
{
    return s_Player;
}

//==============================================================================

const object_desc& player::GetObjectType( void )
{
    return s_Player;
}

//=========================================================================

void player::OnInit( void )
{
//  LOG_MESSAGE( "player::OnInit", "" );

    //#ifndef X_EDITOR
    //s32 nPlayers = g_StateMgr.GetPlayerCount();
    //LOG_MESSAGE( "player::OnInit",
    //             "Addr:%08X - LocalSlot:%d - TotalLocalPlayers:%d - NetSlot:%d",
    //             this, m_LocalSlot, nPlayers, m_NetSlot );
    //#endif

    #ifdef X_EDITOR

    ////////////////////
    // NON-NETWORKING //
    ////////////////////

    // HACK for the editor.  Since the networking stuff does not work in 
    // the editor, we need to take care of a little business manually.
    //
    // And, since OnInit seems to get called twice, make sure we don't do this
    // stuff a second time.

    {
        SetLocalPlayer( 0 );
    }

    #endif // X_EDITOR


    if ( UsingLoco() )
    {
        InitLoco();
    }

    m_Physics.Init( GetGuid() );
    m_Physics.SetSolveActorCollisions( TRUE );

    // Call base
    actor::OnInit();

    //
    // Initialize EVERYTHING
    //

    // character physics init
    m_Physics.SetColHeight               ( 180.0f    );
    m_Physics.SetColRadius               ( 30.0f     );                                                                         
    m_Physics.SetColCrouchOffset         ( 70.0f     );
    m_Physics.SetHandlePermeable         ( TRUE      );

    if ( UsingLoco() )
    {
        // loco MOVE_STYLE_WALK
        loco::move_style_info_default Defaults;
        Defaults.m_IdleBlendTime         = 0.2f;
        Defaults.m_MoveBlendTime         = 0.2f;
        Defaults.m_FromPlayAnimBlendTime = 0.2f;
        Defaults.m_MoveTurnRate          = R_180;
        m_Loco.SetMoveStyleDefaults( loco::MOVE_STYLE_WALK,      Defaults );

        // loco MOVE_STYLE_RUN, MOVE_STYLE_RUN_AIM, MOVE_STYLE_CHARGE
        Defaults.m_IdleBlendTime         = 0.125f;
        Defaults.m_MoveBlendTime         = 0.125f;
        Defaults.m_FromPlayAnimBlendTime = 0.125f;
        Defaults.m_MoveTurnRate          = R_360;
        m_Loco.SetMoveStyleDefaults( loco::MOVE_STYLE_RUN,       Defaults );
        m_Loco.SetMoveStyleDefaults( loco::MOVE_STYLE_RUNAIM,    Defaults );
        m_Loco.SetMoveStyleDefaults( loco::MOVE_STYLE_CHARGE,    Defaults );

        // loco MOVE_STYLE_PROWL, MOVE_STYLE_CROUCH, MOVE_STYLE_CROUCHAIM
        Defaults.m_IdleBlendTime         = 0.2f;
        Defaults.m_MoveBlendTime         = 0.2f;
        Defaults.m_FromPlayAnimBlendTime = 0.2f;
        Defaults.m_MoveTurnRate          = R_90;
        m_Loco.SetMoveStyleDefaults( loco::MOVE_STYLE_PROWL,     Defaults );
        m_Loco.SetMoveStyleDefaults( loco::MOVE_STYLE_CROUCH,    Defaults );
        m_Loco.SetMoveStyleDefaults( loco::MOVE_STYLE_CROUCHAIM, Defaults );
    }

    // Fall damage
    tweak_handle SafeFallTweak (xfs("%s_MinFallDistToTakeDamage",GetLogicalName()));
    tweak_handle DeathFallTweak(xfs("%s_MaxFallDistToTakeDamage",GetLogicalName()));
    m_SafeFallAltitude = SafeFallTweak.GetF32();
    m_DeathFallAltitude = DeathFallTweak.GetF32();

    // Blood Decals
    m_hBloodDecalPackage.SetName( PRELOAD_FILE( "Blood.decalpkg" ) );
    m_BloodDecalGroup = 0;

    // Mutant vision
    m_bAllowedToGlow = TRUE;
    m_FriendlyGlowColor.Set(  50, 255, 0, 255 );
    m_EnemyGlowColor.Set   ( 255,  50, 0, 255 );

    // player stuff
    m_ViewInfo.XFOV             = R_60;
    m_OriginalViewInfo.XFOV     = m_ViewInfo.XFOV;
    m_nLoreDiscoveries          = 0;
#if !defined( CONFIG_RETAIL )
    m_bRenderSkeleton           = FALSE;
    m_bRenderSkeletonNames      = TRUE;
    m_bRenderBBox               = TRUE;
#endif // !defined( CONFIG_RETAIL )
    m_PitchArmsScalerPositive   = 1.0f;
    m_PitchArmsScalerNegative   = 1.0f;
    m_bCanDie                   = TRUE;

    // Avatar (only if we're not in split screen)
    if( UsingLoco() )
    {
        PrepPlayerAvatar();
    }

    // Arms
    {
        m_Skin.SetUpSkinGeom( PRELOAD_FILE("FP_PLR_Human_BIND.skingeom") );
        m_Skin.SetVMeshMask( 0 );
        m_Skin.SetVMeshBit( "MESH_Arms_Hazmat", TRUE );
        m_Skin.SetVMeshBit( "MESH_Hands_Hazmat", TRUE );

        // WARNING:
        // It may be some problem here. The resource handles can't start counting references 
        // untill a name has been assign to them. Not only that but when a new name is set it
        // must make sure that the old name is decremented reference wise.
        m_AnimGroup.SetName( PRELOAD_FILE("FP_PlayerArms.anim") );

        // Make sure that this are clear not matter what
        m_iCameraBone        = -1;
        m_iCameraTargetBone  = -1;

        // If we can load this anim group then we need to extract some info
        if( m_AnimGroup.GetPointer() )
        {
            OnAnimationInit();
        }
    }

    m_MaxStunPitchOffset     = R_1;
    m_MaxStunYawOffset       = R_8;
    m_MaxStunRollOffset      = R_3;
    m_fStunYawChangeSpeed    = 0.5f;
    m_fStunPitchChangeSpeed  = 0.1f;
    m_fStunRollChangeSpeed   = 0.2f;
    m_fMaxStunTime           = 5.0f;
    
    m_StrainFriendFlags          = 0;
    m_StrainFriendFlags          |= FACTION_PLAYER_NORMAL;
    m_StrainFriendFlags          |= FACTION_NEUTRAL;
    m_StrainFriendFlags          |= FACTION_MILITARY;
    m_StrainFriendFlags          |= FACTION_WORKERS;

#ifndef X_EDITOR
    if( GameMgr.GetGameType() != GAME_CAMPAIGN )
    {
        m_bMutationMeleeEnabled         = TRUE;
        m_bPrimaryMutationFireEnabled   = TRUE;
        m_bSecondaryMutationFireEnabled = TRUE;
    }
    else
#endif
    {
        m_bMutationMeleeEnabled         = FALSE;
        m_bPrimaryMutationFireEnabled   = FALSE;
        m_bSecondaryMutationFireEnabled = FALSE;
    }

    m_bInTurret     = FALSE;

    // load all the Lore Object guids for us
    LoadAllLoreObjects();
}

//=========================================================================

f32 player::GetMovementNoiseLevel()
{
    f32 MaxVelocity = GetMaxVelocity();
    
    if( MaxVelocity == 0.0f )
    {
        return MaxVelocity;
    }
    else
    {
        return GetCurrentVelocity()/MaxVelocity;
    }
}

//=========================================================================

void player::ResetView( void )
{
    m_ViewInfo.XFOV = m_OriginalViewInfo.XFOV;
}

//=========================================================================

void player::UpdateZoneTrack ( void )
{
    // Must use view position for horizontally flat portals.
    g_ZoneMgr.UpdateZoneTracking( *this, m_ZoneTracker, GetEyesPosition() ); 
}

//=========================================================================

zone_mgr::zone_id player::GetPlayerObjectZone ( void ) const
{
    // Use zone tracker zone
    return m_ZoneTracker.GetMainZone();
}

//=========================================================================

zone_mgr::zone_id player::GetPlayerViewZone( void ) const
{
    // SB: 2/1/2005 
    
    // NOTE: For the next project, the cinema process REALLY needs to be fixed.
    // The player should not be dragged around to the camera (unless the cinema
    // requires this), and this function SHOULD return the camera's zone for ALL
    // cinemas.
    
    // Also, the fading, letter box etc should be automatic and come from the logic
    // in the cinema object. Right now it's done with triggers and the property system
    // (very ugly!).

    // DS: 2/2/2005 
    // NOTE: The third-person death camera is similar to a cinema and so
    // we should be using the camera's zone and NOT the player's zone for
    // rendering. BLECH...our zone and portal system is much too error-prone
    // as seen from this example, Steve's note above, and the numerous data
    // problems coming from art and design.
    third_person_camera* pThirdPersonCam = GetThirdPersonCamera();
    if( pThirdPersonCam )
    {
        return (zone_mgr::zone_id)pThirdPersonCam->GetZone1();
    }
    
    // Use object zone
    return GetPlayerObjectZone();
}

//=========================================================================

void player::UpdateSafeSpot( f32 DeltaTime )
{
    const f32 timeBetweenSafeSpotChecks = 10.0f;
    static f32 timeSinceSafeSpotCheck = 0.0f;
    
    timeSinceSafeSpotCheck += DeltaTime;
    if( timeSinceSafeSpotCheck > timeBetweenSafeSpotChecks )
    {
        timeSinceSafeSpotCheck = 0.0f;
        
        //  If we currently feel safe
        if( GetIsSafeSpot() )
        {
            SetCurrentSpotAsSafeSpot();
        }
    }
}

//=========================================================================

void player::OnReset( void )
{
    m_Health.Reset();

    OnMove( m_RespawnPosition );

    // Zone change, must update the tracker zone.
    UpdateZone(m_RespawnZone);
}

//==============================================================================
hud_object* player::GetHud( void )
{
    slot_id SlotID  = g_ObjMgr.GetFirst( object::TYPE_HUD_OBJECT );

    if( SlotID != SLOT_NULL )
    {
        object* pObj = g_ObjMgr.GetObjectBySlot( SlotID );

        // for some reason the object isn't valid
        if( pObj )
        {
            // get the HUD from the object
            return &hud_object::GetSafeType( *pObj );
        }
    }

    // failed
    return NULL;
}


void player::ShakeView ( f32 Time, f32 Amount, f32 Speed )
{
    m_ShakeTime             = Time;
    m_ShakeAngle            = 0.0f;      
    m_ShakeAmount           = Amount;
    m_ShakeSpeed            = Speed;
}

//==============================================================================

xbool player::InvalidSound( void )
{
    if( m_InvalidSoundTimer <= 0.0f )
    {
        g_AudioMgr.Play( "Klaxon_01_Shot", TRUE );
        m_InvalidSoundTimer = 3.0f;
        return TRUE;
    }
    
    return FALSE;
}

//==============================================================================
void player::UpdateCameraShake( f32 DeltaTime )
{
    // Calculate shake using square fall off - the shake is bigger and faster at 
    // the start, and gets slowers as it dampens to zero
    f32    Freq      = SQR(DEG_TO_RAD(( MIN(1.0f,m_ShakeTime) * m_fShakeFreqScalar * 360.0f)*m_ShakeSpeed)) ;
    
    // Apply shake
    m_ShakeAngle += Freq * DeltaTime;

    m_ShakeAngle = x_fmod(m_ShakeAngle, 360.0f );

    // Update shake time
    m_ShakeTime = MAX(0, m_ShakeTime - DeltaTime) ;

    //apply view modification from pain force
    if (m_PitchMod > 0)
        m_PitchMod = MAX(0, MIN(m_PitchMod - (R_40*DeltaTime), R_20));
    else if (m_PitchMod < 0)
        m_PitchMod = MIN(0, MAX(m_PitchMod + (R_40*DeltaTime), -R_20));
        
    if (m_YawMod > 0)
        m_YawMod   = MAX(0, MIN(m_YawMod   - (R_40*DeltaTime), R_20));
    else if (m_YawMod < 0)
        m_YawMod   = MIN(0, MAX(m_YawMod   + (R_40*DeltaTime), -R_20));

    if (m_RollMod > 0)
        m_RollMod  = MAX(0, MIN(m_RollMod  - (R_40*DeltaTime), R_20));
    else if (m_RollMod < 0)
        m_RollMod  = MIN(0, MAX(m_RollMod  + (R_40*DeltaTime), -R_20));
}

//==============================================================================
void player::ClearPainEvent( void )
{
    m_LastPainEvent.Clear();
}

//==============================================================================

//static f32 PLAYER_FORCE_VEL = 600.0f;
static f32 PLAYER_FORCE_RUMBLE_DURATION  = 0.25f;
static f32 PLAYER_FORCE_RUMBLE_INTENSITY = 1.0f;
static f32 PLAYER_FORCE_SHAKE = 0.5f;
static f32 PLAYER_FORCE_BLUR = 1.0f;
static f32 PLAYER_FORCE_ROTATE = 0.05f;

//==============================================================================

void player::DoBasicPainFeedback( f32 Force )
{                                                                                                                                                                                           
    // Shake the camera
    ShakeView( PLAYER_FORCE_SHAKE * Force );

    //force feedback
    DoFeedback(PLAYER_FORCE_RUMBLE_DURATION  * Force, PLAYER_FORCE_RUMBLE_INTENSITY * Force);

    // Start up the shaky-blur pain post-effect
    f32 EffectForce = PLAYER_FORCE_BLUR * Force;
    g_PostEffectMgr.StartPainBlur( GetLocalSlot(),
        MIN(EffectForce * 20.0f, 20.0f),
        xcolor( (u8)MIN(255, 200 + (EffectForce * 55.0f) ), 128, 128, (u8)MIN(180, 100 + (EffectForce * 80.0f) ) ) );
}

//==============================================================================

void player::RespondToPain( const pain& Pain )
{

    // Get force and damage from pain
    f32 Force = Pain.GetForce();

    //
    // Do additional effects
    //
#ifndef X_EDITOR
    // take no damage from friendly sources.
    if( !g_NetworkMgr.IsOnline() )
    {
#endif
        DoBasicPainFeedback( Force );
#ifndef X_EDITOR
    }
#endif

    //
    // Rotate player based on pain direction
    //
    if( 1 )        
    {
        //mess with pitch and yaw
        radian rPainAngleYaw   = v3_AngleBetween(vector3(0, m_EyesYaw), Pain.GetDirection());
        radian rPainAnglePitch = v3_AngleBetween(vector3(m_EyesPitch, 0), Pain.GetDirection());

        vector3 PlayerFaceDir(0,0,1);
        PlayerFaceDir.RotateY( m_Yaw );
        plane Plane( vector3(0,0,0), PlayerFaceDir, vector3(0,1,0) );
        
        if( Plane.InFront( Pain.GetDirection() ) )
        {
            rPainAngleYaw = rPainAngleYaw;
        }
        else
        {
            rPainAngleYaw = -rPainAngleYaw;
        }

        f32 RotateForce = Force * PLAYER_FORCE_ROTATE;
        
        m_YawMod   -= rPainAngleYaw   * RotateForce;
        m_PitchMod -= rPainAnglePitch * RotateForce;
        m_RollMod  -= rPainAngleYaw   * RotateForce;
    }
    
/* CJ: Removed force on player from pain to fix several bugs.
    //
    // Push the player using the force
    //
    if( !m_bInTurret )
    {
        vector3 ForceVel = Pain.GetForceVelocity() * PLAYER_FORCE_VEL;
        m_Physics.AddVelocity( ForceVel );
    }
*/
}

//==============================================================================

actor::eHitType player::OverrideFlinchType( actor::eHitType hitType )
{
    // Players 3rd person avatar can only play light (additive) hits
    switch( hitType )
    {
        case HITTYPE_HARD:
        case HITTYPE_LIGHT:
        case HITTYPE_IDLE:
        case HITTYPE_PLAYER_MELEE_1:
            return HITTYPE_LIGHT;
        ASSERTS( 0, "Need to add new hit type here..." );
        default:
            return hitType;
    }
}

//==============================================================================

void player::OnPain( const pain& Pain )
{
    CONTEXT("player::OnPain");

    // Player cannot take damage while viewing a cinematic..
    if (m_ViewCinematicPlaying)
        return;

    // I have no idea what the line above does.  Is that code even used?
    // Let's check the real cinema system...
    if (m_Cinema.m_bCinemaOn)
        return;

    // If you are already dead, no pain!
    if( m_bDead )
        return;

    // If the same pain event as the last one, ignore it.
    if( (Pain.GetAnimEventID()!=-1) && 
        (Pain.GetAnimEventID() == m_LastAnimPainID) )
        return;

    // if we are neutral, we ignore pain
    if ( m_SpawnNeutralTime > 0.0f )
    {
        return;
    }
      
#ifndef X_EDITOR
    // take no damage from friendly sources.
    if( !GameMgr.IsGameMultiplayer() )
    {
#endif
        // If we didn't create the pain, then early-out if it's friendly
        if( (Pain.GetOriginGuid() != GetGuid()) && IsFriendlyFaction(GetFactionForGuid(Pain.GetOriginGuid())) )
        {
            return;
        }
#ifndef X_EDITOR
    }
#endif

    xstring StringPrefix = (const char*)xfs( "%s", GetLogicalName() );

    // Modify damage on mutated players.
    if( m_bIsMutated )
    {
        StringPrefix += "_MUTANT";
    }

    // Decide which health id to use
#ifndef X_EDITOR
    if( GameMgr.IsGameMultiplayer() )
    {
        switch( GetHitLocation( Pain ) )
        {   
        case geom::bone::HIT_LOCATION_HEAD:
            StringPrefix += "_H";
            break;
        case geom::bone::HIT_LOCATION_LEGS:
            StringPrefix += "_L";
            break;
        default: // includes TORSO, ARMS
            StringPrefix += "_B";
            break;
        };
    }
    else
#endif
    {
        StringPrefix += "_B";
    }


    // turn into string pointer
    const char* pString = (const char*)StringPrefix;

    // Decide which health id to use
    health_handle HealthHandle( pString );

    // Resolve Pain
    if( !Pain.ComputeDamageAndForce( HealthHandle, GetGuid(), GetBBox().GetCenter() ) )
        return;

    /*
#ifndef X_EDITOR
#ifdef DATA_VAULT_KEEP_NAMES
    LOG_MESSAGE( "player::OnPain",
        "Player %d taking %f damage in %s from weapon %s.",
        m_NetSlot, Pain.GetDamage(), pString, Pain.GetPainHealthHandle().GetName() );
#else
    LOG_MESSAGE( "player::OnPain",
        "Player %d taking %f damage in %s from weapon %d.",
        m_NetSlot, Pain.GetDamage(), pString, Pain.GetHitType() );
#endif
#endif
    */

    //
    // Apply the damage
    //
    TakeDamage( Pain );

    // If this is not the active player, then it needs to become it
    // Is this needed anymore !?!
    if( !m_bActivePlayer )
    {
        ASSERT(FALSE);
        player* pPlayer = SMP_UTIL_GetActivePlayer();
        if( pPlayer ) g_ObjMgr.DestroyObject( pPlayer->GetGuid() );
        SetAsActivePlayer(TRUE);
    }

    // Record the last pain event and time.
    m_LastPainEvent.Append( Pain );

    // Do shakes and pushes and rumbles
    RespondToPain( Pain );
    
#ifndef X_EDITOR
    // For multi-player flinch 3rd person avatar and create blood
    if( GameMgr.IsGameMultiplayer() )
    {
        // Do flinches, blood, impact sounds etc
        DoMultiplayerPainEffects( Pain );
    }
#endif
}

//=============================================================================

void player::BackUpCurrentState  ( void )
{
    OnCopy( m_SaveSpotProperties );
}       
//=============================================================================

void player::RestoreState        ( void )
{
    if(m_SaveSpotProperties.GetCount() > 0 )
    {    
        OnPaste(m_SaveSpotProperties);
    }
}

//----------------------------------------------------------------------------------------------------------------
//
void player::ParseOnPainForEffects ( const pain& Pain )
{
    // Skip?
    if( !IsBloodEnabled() )
        return;

    // Create blood impact if blood decals are assigned
    const decal_package* pBloodDecalPackage = m_hBloodDecalPackage.GetPointer();
    if( pBloodDecalPackage )
    {
        // Create blood based on pain type and use color of assigned blood decal group
        particle_emitter::CreateOnPainEffect( Pain, 
                                              k_PainParticleDisplace, 
                                              particle_emitter::UNINITIALIZED_PARTICLE, 
                                              pBloodDecalPackage->GetGroupColor( m_BloodDecalGroup ) );
    }                                                  
}

//===========================================================================

xbool player::GetIsSafeSpot ( void )
{
    const f32 k_MIN_TIME_NOT_SPOTTED_TO_CONSIDER_SAFE = 10.0f;
    const f32 k_MIN_TIME_NOT_INJURED_TO_CONSIDER_SAFE = 10.0f;
    f32 currentTime = (f32)x_GetTimeSec();

    if( ( currentTime - m_LastTimeSeenByEnemy ) > k_MIN_TIME_NOT_SPOTTED_TO_CONSIDER_SAFE && 
        ( currentTime - m_LastTimeTookDamage  ) > k_MIN_TIME_NOT_INJURED_TO_CONSIDER_SAFE )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//===========================================================================

void player::SetCurrentSpotAsSafeSpot ( void )
{
    m_PositionOfLastSafeSpot     = m_NextPositionOfLastSafeSpot;
    m_ZoneIDOfLastSafeSpot       = m_NextZoneIDOfLastSafeSpot;
    m_NextPositionOfLastSafeSpot = GetPosition(); 
    m_NextZoneIDOfLastSafeSpot   = GetPlayerObjectZone() ;    
}

//===========================================================================

void player::ResetToLastSafeSpot ( void )
{
    if( m_PositionOfLastSafeSpot.LengthSquared() != 0.0f )
    {
        OnMove    ( m_PositionOfLastSafeSpot );
        UpdateZone( m_ZoneIDOfLastSafeSpot   );
    }
    else
    {
        OnMove    ( m_RespawnPosition );
        UpdateZone( m_RespawnZone     );
    }
}

//===========================================================================

void player::PushViewCinematic ( lock_view_node* pLockViewBuffer )
{
    for (s32 i = 0; i < lock_player_view::MAX_TABLE_SIZE; i++)
        m_LockViewTable[i] = pLockViewBuffer[i];

    m_ViewCinematicPlaying  = TRUE;
    m_CurrentViewNode       = 0;
    m_TimeStartTick         = g_ObjMgr.GetGameTime();
    
    m_StartView.Identity();
    m_DesiredView.Identity();
    
    m_StartView.Setup( radian3( m_Pitch, m_Yaw, 0.0f ) );

    lock_view_node* rNode = &m_LockViewTable[0];

    //by convention -1 is an end node..
    if (rNode->m_TimeTo <= 0.0f)
    {
        m_ViewCinematicPlaying = FALSE;
        return;
    }

    vector3 LookAtVector = rNode->m_LookAt - GetEyesPosition();

    m_DesiredView.Setup( radian3(LookAtVector.GetPitch(), LookAtVector.GetYaw(), 0.0f) );
    
    m_fYawValue     = 0.0f;
    m_fPitchValue   = 0.0f;

    m_CScale    = 1.0f/(rNode->m_TimeTo*k_Modfactor);
    m_CTimeSum  = 0.0f;

    m_ForwardVelocity.Zero();
    m_StrafeVelocity.Zero();

//    g_Hud.PlayCinematic( TRUE );
}

//===========================================================================

void player::UpdateViewCinematic ( const f32& rDeltaTime )
{
    //play through the cinematic 
    
    (void) rDeltaTime;

    ASSERT( m_CurrentViewNode < lock_player_view::MAX_TABLE_SIZE && m_CurrentViewNode >= 0);

    lock_view_node* rNode = &m_LockViewTable[m_CurrentViewNode];
    
    f32 TimeTotal = g_ObjMgr.GetGameDeltaTime( m_TimeStartTick );
      
    if (rNode->m_TimeTo + rNode->m_Linger < TimeTotal)
    {
        m_CurrentViewNode++;
        
        if (m_CurrentViewNode >= lock_player_view::MAX_TABLE_SIZE)
        {
            m_ViewCinematicPlaying = FALSE;
//            g_Hud.PlayCinematic( FALSE );
            return;
        }
        else
        {
            rNode = &m_LockViewTable[m_CurrentViewNode];
        }
        
        if (rNode->m_TimeTo <= 0.0f)
        {
            m_ViewCinematicPlaying = FALSE;
//            g_Hud.PlayCinematic( FALSE );
            return;
        }
         
        m_CScale    = 1.0f/(rNode->m_TimeTo*k_Modfactor);
        m_CTimeSum  = 0.0f;

        m_StartView = m_DesiredView;

        m_TimeStartTick = g_ObjMgr.GetGameTime();
        
        TimeTotal = g_ObjMgr.GetGameDeltaTime( m_TimeStartTick );
        
        vector3 LookAtVector = rNode->m_LookAt - GetEyesPosition();
        
        m_DesiredView.Setup( radian3(LookAtVector.GetPitch(), LookAtVector.GetYaw(), 0.0f) );
    }

    ASSERT( rNode->m_TimeTo > 0.0f );
    
    if ( rNode->m_TimeTo > TimeTotal )
    {   
        f32 T = (TimeTotal/rNode->m_TimeTo);
        
        f32 Tquad = -(T*T - T)*m_CScale;
        
        m_CTimeSum += Tquad;
        
        if (m_CTimeSum > 0.0f && m_CTimeSum < 1.0f)
        {
            quaternion BlendView = Blend( m_StartView, m_DesiredView, m_CTimeSum );
            radian3    BlendViewDir = BlendView.GetRotation();
            
            m_Pitch = BlendViewDir.Pitch;
            m_Yaw   = BlendViewDir.Yaw;
        }
        else if (m_CTimeSum > 1.0f)
        {
            radian3    ViewDir = m_DesiredView.GetRotation();
            
            m_Pitch = ViewDir.Pitch;
            m_Yaw   = ViewDir.Yaw;
        }
    }
    else
    {
        radian3    ViewDir = m_DesiredView.GetRotation();
        
        m_Pitch = ViewDir.Pitch;
        m_Yaw   = ViewDir.Yaw;
    }
}

//----------------------------------------------------------------------------------------------------------------

void player::ResetPlayer ( const vector3& rPos, const radian3& rViewRot )
{
    // Reset position
    OnMove(rPos);

    // Reset rotation
    m_Pitch = rViewRot.Pitch;
    m_Yaw   = rViewRot.Yaw;

    // Reset zone tracking
    g_ZoneMgr.InitZoneTracking( *this, m_ZoneTracker );
}
   
//=============================================================================
//=============================================================================
//  Checks for a LOS to an object
//=============================================================================

xbool player::CanSeeObject(object* pObject)
{
    ASSERT( pObject );
    g_CollisionMgr.LineOfSightSetup( GetGuid(),                             // MovingObjGuid,
                                     GetEyesPosition(),                     // WorldStart,
                                     pObject->GetBBox().GetCenter() );      // WorldEnd,
    g_CollisionMgr.AddToIgnoreList ( pObject->GetGuid() );
    g_CollisionMgr.IgnoreGlass     ();

    g_CollisionMgr.CheckCollisions ( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_PLAYER_LOS, (object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING)) ;
    if( g_CollisionMgr.m_nCollisions )
    {
        return FALSE;
    }
    
    return TRUE;
}

//===========================================================================

void player::OnMoveFreeCam( view& View )
{
    // View's position
    s32 i = 0;
    vector3 vPos = View.GetPosition();

    // Update zone tracking
    g_ZoneMgr.UpdateZoneTracking( *this, m_ZoneTracker, vPos ); 

    // Cast a couple rays:
    // 0 = down, 1 = up, 2 = left, 3 = right, 4 = front, 5 = back
    vector3 vDir[6];

    vDir[0].Set( 0.0f, -1500.0f, 0.0f );
    vDir[1].Set( 0.0f, 500.0f, 0.0f );

    for ( i = 2; i < 6; i++ )
    {
        vDir[i].Set( 0.0f, 0.0f, 500.0f );
        vDir[i].RotateY( (i - 2) * R_90 );
    }

    for ( i = 5; i >= 0; i-- )
    {
        g_CollisionMgr.RaySetup( GetGuid(), vPos, vDir[i] + vPos );
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, (object::object_attr)(object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING)) ;

        if ( g_CollisionMgr.m_nCollisions > 0 )
        {
            guid HitGuid = g_CollisionMgr.m_Collisions[0].ObjectHitGuid;
            object* pObject = g_ObjMgr.GetObjectByGuid( HitGuid );
            if ( pObject == NULL )
            {
                continue;
            }

            // if the object that we hit isn't in the zone that we're in, we need to make the
            // object's zone the active one
            if ( pObject->GetZone1() != GetZone1() )
            {
                SetZone1( pObject->GetZone1() );
            }
        }
    }

    // print out the zone info
    s32 ZoneID = GetZone1();
    x_printfxy( 1, 1, "FlyModeZone=%d", ZoneID );
}

//===========================================================================

extern view g_View;

void player::OnExitFreeCam( vector3& NewPos )
{
    // CJ: This just ensures the position is where it wants to go and avoids any collisions
    // or other logic on the way by calling the object move function before calling the
    // regular move function
    object::OnMove( NewPos );
    OnMove( NewPos );
    m_Physics.SetPosition( NewPos );

    // Grab the current view orientation, clear any death states
#if !defined( X_EDITOR )
    g_View.GetPitchYaw( m_Pitch, m_Yaw );
    SetAnimState( ANIM_STATE_IDLE );
    EndDeath();
    GetView().SetRotation( radian3( m_Pitch, m_Yaw, R_0 ) );
#endif
}

//===========================================================================
void player::DoFeedback( f32 Duration, f32 Intensity )
{
    // don't do feedback if we're dead
    if( m_ActivePlayerPad != -1 && !IsDead() )
    {
        input_Feedback(Duration, Intensity, g_IngamePad[m_ActivePlayerPad].GetControllerID());
    }
}

//===========================================================================

#if !defined( CONFIG_RETAIL )

void player::DrawLabelInFront( const char* pLabel )
{
    xbool bInDraw = eng_InBeginEnd();
    xbool bBegun = FALSE;
    if ( ! bInDraw )
    {
        bBegun = eng_Begin( "Generic Debug" );
    }

    if( bInDraw || bBegun )
    {
        vector3 vLabelPos(0.0f, 0.0f, 150.0f );
        vLabelPos.Rotate( radian3( m_EyesPitch, m_EyesYaw, 0.0f ) );
        vLabelPos += m_EyesPosition;

        draw_Label( vLabelPos, XCOLOR_WHITE, pLabel );
    }

    if ( bBegun )
    {
        eng_End();        
    }
}

#endif // !defined( CONFIG_RETAIL )

//===========================================================================

void player::OnKill( void )
{
    actor::OnKill();

    // free our slot
    if( m_LocalSlot != -1 )
    {
        m_LocalSlot = -1;
    }    
}

//===========================================================================
static f32 s_ArmViewPct = 0.3f;
static const f32 s_MaxCameraDelta = 28.0f;
xbool g_UseOldCameraDefaultPos = FALSE;
vector3 player::GetDefaultViewPos( void )
{
    vector3 FinalPos( 0.0f, 0.0f, 0.0f );
    if( m_iCameraBone > -1 )
    {
        if( g_UseOldCameraDefaultPos )
        {
            FinalPos 
                = m_vRigOffset 
                + m_AnimPlayer.GetBonePosition( m_iCameraBone ) 
                - (m_ArmsOffset * s_ArmViewPct) 
                - m_LeanWeaponOffset;

            FinalPos += m_Cinema.m_ViewCorrectionDelta;
        }
        else
        // KSS -- new cinema code
        {
            vector3 AnimBonePos = m_AnimPlayer.GetBonePosition( m_iCameraBone );
            
            vector3 Offset = m_vRigOffset - (m_ArmsOffset * s_ArmViewPct);
            FinalPos = AnimBonePos + Offset;
            FinalPos += m_Cinema.m_ViewCorrectionDelta;
        }


        // CJ: This wacky code is here to prevent the maximum XZ plane translation of the camera
        // from the vertical center of the player exceeding the s_MaxCameraDelta variable
        // This stops the camera crashing through surfaces as the players head tilts back, it's
        // not an ideal solution but it was the easiest to implement.

        // Construct the plane to limit the camera motion behind the player
        vector3 Forward ( radian3( 0.0f, m_Yaw, 0.0f ) );
        vector3 Pos( GetPosition() - s_MaxCameraDelta * Forward );
        plane p;
        p.Setup( Pos, Forward );
        
        // Is camera behind the plane, if so move it to the plane
        if( p.InBack( FinalPos ) )
        {
            f32 d = p.Distance( FinalPos );
            FinalPos -= p.Normal * d;
        }
    }
    else
    {
        vector3 Position( GetPosition() );
        vector3 EyesOffSet( m_EyesOffset );
        EyesOffSet.RotateY( m_Yaw );

        FinalPos.Set( Position.GetX(), GetBBox().Max.GetY(), Position.GetZ() );
        FinalPos += EyesOffSet;
    }

    return FinalPos;
}

//===========================================================================

inline void player::ComputeStunnedPitchYawOffset( radian PitchOffset, radian YawOffset )
{
    f32 YawRotFactor = m_fStunnedTime * m_fStunYawChangeSpeed;
    f32 PitchRotFactor = m_fStunnedTime * m_fStunPitchChangeSpeed;

    YawOffset = x_sin( YawRotFactor );
    PitchOffset = x_sin( PitchRotFactor );

    YawOffset *= m_MaxStunPitchOffset;
    PitchOffset *= m_MaxStunYawOffset;
}

//===========================================================================
static f32 s_ViewRollTune = 3987.63f; 

void player::ComputeView( view& View, view_flags Flags )
{
    third_person_camera* pThirdPersonCamera = GetThirdPersonCamera();
    if ( pThirdPersonCamera )
    {
        pThirdPersonCamera->ComputeView( View );
    }
    else
    {
        (void)Flags;
        View.SetXFOV( m_ViewInfo.XFOV );

        // SB: Using player letter box cinema?
        if( m_Cinema.m_bCinemaOn == TRUE )
        {
            // Using a camera object?
            object* pCamera = g_ObjMgr.GetObjectByGuid( m_Cinema.m_CinemaCameraGuid );
            if( pCamera )
            {
                // Use exact cinema camera info
                View.SetV2W( m_Cinema.m_CameraV2W );
                View.SetXFOV( m_Cinema.m_CameraXFOV );

                // Update the eyes so that zone tracking works
                m_EyesPosition = View.GetPosition();

                // Get out of here so position, rotation and field-of-view are WYSIWYG!
                return;
            }        
        }

        radian3 Rot( 0.0f, 0.0f, 0.0f );
        vector3 Pos( GetDefaultViewPos() );

        // Use camera bone?
        if( ( m_CurrentWeaponItem != INVEN_NULL ) && ( m_iCameraBone != -1 ) )
        {
            // Get camera rotation in engine world space
            Rot = m_AnimPlayer.GetBoneL2W( m_iCameraBone ).GetRotation();

            Rot.Yaw   += R_180;
            Rot.Pitch = - Rot.Pitch; 
            Rot.Roll  = - Rot.Roll;

            // Counter act the rig offset that has already been applied to the anim player L2W
            // (which is a parent of the camera bone) otherwise it won't come through since the camera
            // is also moved by the rig offset along with the arms.
            Rot.Yaw   -= m_CurrentHorozRigOffset;
            Rot.Pitch -= m_CurrentVertRigOffset;

        }        
        else
        {
            // Use current pitch and yaw
            Rot.Set( m_Pitch, m_Yaw, -DEG_TO_RAD( GetTweakF32( "LeanMaxDegrees" ) * m_SoftLeanAmount ) );
        }

        //
        // Ask the current weapon how far zoomed we are
        //
        if( (m_CurrentAnimState != ANIM_STATE_FALLING_TO_DEATH) && (IsAlive()) )
        {
            new_weapon* pWeapon = GetCurrentWeaponPtr();

            if( pWeapon )
            {
                if( pWeapon->IsZoomEnabled() )
                {
                    View.SetXFOV( pWeapon->GetXFOV() );
                    m_ViewInfo.XFOV = View.GetXFOV();
                }
                else
                {
                    m_ViewInfo.XFOV = m_OriginalViewInfo.XFOV;
                }
            }
        }

        //
        // If we're not dead, respond to pain and stuns
        //
        if( (m_CurrentAnimState != ANIM_STATE_DEATH) &&
            (m_CurrentAnimState != ANIM_STATE_FALLING_TO_DEATH) )
        {
            if ( m_NonExclusiveStateBitFlag & NE_STATE_STUNNED )
            {
                radian PitchOffset  = 0.0f;
                radian YawOffset    = 0.0f;
                ComputeStunnedPitchYawOffset( PitchOffset, YawOffset );
                Rot.Set( m_PreStunPitch + PitchOffset, m_PreStunYaw + YawOffset, 0.0f );
            }

            // Apply camera shake
            f32 Amp      = SQR(MIN(1.0f, m_ShakeTime * m_fShakeAmpScalar)) ;
            m_ShakePitch = (DEG_TO_RAD(m_fShakeMaxPitch) * Amp * x_sin(m_ShakeAngle* 0.981f))*m_ShakeAmount;
            m_ShakeYaw   = (DEG_TO_RAD(m_fShakeMaxYaw)   * Amp * x_cos(m_ShakeAngle * 1.375f) )*m_ShakeAmount;
            Rot.Pitch   -= m_ShakePitch;
            Rot.Yaw     -= m_ShakeYaw;

            // Apply pain force
            Rot.Pitch   += m_PitchMod;
            Rot.Yaw     += m_YawMod;

            // Apply movement roll
            if ( m_DeltaTime > 0.0f )
            {
                radian RollAmount = ((m_StrafeVelocity.Length() / m_DeltaTime) / s_ViewRollTune) * R_1;
                vector3 Forward;
                Forward.Set( radian3( 0.0f, Rot.Yaw, 0.0f ) );
                if ( m_StrafeVelocity.Cross( Forward ).GetY() < 0 )
                {
                    RollAmount = -RollAmount;
                }

                Rot.Roll += RollAmount;
            }

        }

        View.SetPosition( Pos );
        View.SetRotation( Rot );

        m_EyesPosition  = View.GetPosition();
        View.GetPitchYaw( m_EyesPitch, m_EyesYaw );

        UpdateZoneTrack();
    }

    // Now make sure the player's near and far planes match the level
    // settings near and far planes
    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_LEVEL_SETTINGS );
    if( SlotID != SLOT_NULL )
    {
        object*         pObj = g_ObjMgr.GetObjectBySlot( SlotID );
        level_settings& LevelSettings = level_settings::GetSafeType( *pObj );

        View.SetZLimits( 10.0f, LevelSettings.GetFarPlane() );
    }
    else
    {
        View.SetZLimits( 10.0f, 8000.0f );
    }
}

//===========================================================================

void player::InitializeMeleeAnimStateList(void)
{
    // set index to invalid so GetNextMeleeState() will be setup correctly
    m_MeleeAnimStateIndex = -1;

    // initialize list
    for( s32 i=0; i < MAX_MELEE_STATES; i++ )
    {
        m_MeleeAnimStates[i] = animation_state(ANIM_STATE_MELEE+1+i);
    }

    // randomize it initially
    RandomizeMeleeAnimStateList();
}

//===========================================================================

void player::RandomizeMeleeAnimStateList( void )
{
    s32 j = 0;
    animation_state temp_AnimState;
    s32 maxStates = MAX_MELEE_STATES - 1;

    // save off last anim state in list so we can make sure it's not the top one in the new list
    animation_state LastAnimState = m_MeleeAnimStates[maxStates];
    for( s32 i=0; i < MAX_MELEE_STATES; i++ )
    {
        j = x_irand(0, maxStates);
        temp_AnimState = m_MeleeAnimStates[i];
        m_MeleeAnimStates[i] = m_MeleeAnimStates[j];
        m_MeleeAnimStates[j] = temp_AnimState;
    }

    // oops the last anim in the old list is at the top of the new list, switch.
    if( LastAnimState == m_MeleeAnimStates[0] )
    {
        // put it at the end again (doing this because we at least know the list is as big as 0 -> maxStates).
        temp_AnimState = m_MeleeAnimStates[0];
        m_MeleeAnimStates[0] = m_MeleeAnimStates[maxStates];
        m_MeleeAnimStates[maxStates] = temp_AnimState;
    }
}

//=============================================================================
player::animation_state player::GetNextMeleeState( void )
{
    m_MeleeAnimStateIndex++;
    if( m_MeleeAnimStateIndex >= MAX_MELEE_STATES )
    {
        RandomizeMeleeAnimStateList();
        m_MeleeAnimStateIndex = 0;
    }
    
    ASSERT( m_MeleeAnimStateIndex < MAX_MELEE_STATES && m_MeleeAnimStateIndex >= 0 );

    return m_MeleeAnimStates[m_MeleeAnimStateIndex];
}

//=============================================================================
third_person_camera* player::GetThirdPersonCamera( void ) const
{
    return( (third_person_camera*)g_ObjMgr.GetObjectByGuid(m_ThirdPersonCameraGuid) );
}

//=============================================================================
void player::SetupThirdPersonCamera( void )
{
    // Do not create a 3rd person camera for network ghosts.
    if( m_LocalSlot == -1 )
        return;

    if ( !GetThirdPersonCamera() )
    {
        guid Guid = g_ObjMgr.CreateObject( "Third Person Camera" );
        ASSERT( Guid );

        m_ThirdPersonCameraGuid = Guid;
        ASSERT( GetThirdPersonCamera() );
    }

    view& View = GetView();
    radian Pitch;
    radian Yaw;
    vector3 CamTarget = View.GetPosition();

    View.GetPitchYaw( Pitch, Yaw );

    //
    //  If we can, give the 3rd person cam a hint
    //  indicating what direction the killing pain
    //  came from.
    //
    const corpse_pain& ThePain = GetCorpseDeathPain();

    vector3 DirTowardSource(0,0,1);
    
    if (ThePain.GetOriginGuid())
    {
        // If we know the object, let's aim at it
        object* pObj = g_ObjMgr.GetObjectByGuid( ThePain.GetOriginGuid() );
        if (pObj)
        {
            vector3 Pos = pObj->GetPosition();
            DirTowardSource = Pos - GetPosition();
        }
    }
    else
    {        
        if (ThePain.IsDirectHit())
        {
            // Otherwise, try to aim back along the pain dir
            DirTowardSource = -ThePain.GetDirection();
        }
    }

    DirTowardSource.Normalize();

    third_person_camera* pCam = GetThirdPersonCamera();
    
    pCam->SetOrbitPoint( CamTarget );
    pCam->Setup( CamTarget, DirTowardSource, DeathCamStartBackDist, DeathCamEndDist, this );    
}

//=============================================================================

void player::UpdateThirdPersonCamera( void )
{
    if ( GetThirdPersonCamera() )
    {
        object_ptr<corpse> pCorpse( m_CorpseGuid );
        if ( pCorpse )
        {
            physics_inst& PhysicsInst = pCorpse->GetPhysicsInst();
            if ( PhysicsInst.GetNRigidBodies() > 0 )
            {
                const rigid_body& RigidBody = PhysicsInst.GetRigidBody( 0 );
                const vector3 Pos( RigidBody.GetWorldBBox().GetCenter() );
                GetThirdPersonCamera()->SetOrbitPoint( Pos );
            }
            else
            {
                GetThirdPersonCamera()->SetOrbitPoint( GetBBox().GetCenter() );
            }
        }
        else
        {
            // There should never be anything interesting at the players bbox location.
            // The corpse is where the fun is.  The camera is initialized based on
            // the players bbox, and the corpse will conveniently be there also.
            // Once things get rolling, the corpse can end up somewhere entirely
            // different than the player.  Let the camera follow the corpse, and when
            // we no longer have one, leave the camera alone.
            //GetThirdPersonCamera()->SetOrbitPoint( GetBBox().GetCenter() );
        }
    }
}

//=============================================================================

void player::SetLocalPlayer( s32 LocalIndex )
{
    ASSERT( IN_RANGE( 0, LocalIndex, MAX_LOCAL_PLAYERS-1 ) );
    ASSERT( m_LocalSlot <= 0 ); 

    m_LocalSlot = LocalIndex;

    m_ActivePlayerPad = LocalIndex;

#if !defined(X_EDITOR)
    // find a controller to assign
    s32 nIndex = LocalIndex;
    s32 iPad;
    for( iPad = 0; iPad < MAX_LOCAL_PLAYERS; iPad++ )
    {
        if( g_StateMgr.GetControllerRequested(iPad) )
        {
            if( nIndex == 0 )
                break;

            nIndex--;
        }
    }
    ASSERT( iPad < MAX_LOCAL_PLAYERS );

    g_IngamePad[LocalIndex].SetControllerID( iPad );

    // enable vibration based on profile settings.
    player_profile& Profile = g_StateMgr.GetActiveProfile( g_StateMgr.GetProfileListIndex(LocalIndex));
    input_EnableFeedback( Profile.m_bVibration, iPad );

#else
    // always controller 0.
    g_IngamePad[LocalIndex].SetControllerID( 0 );
#endif

    ASSERT( g_IngamePad[LocalIndex].GetControllerID() != -1 );

    g_IngamePad[LocalIndex].EnableContext( INGAME_CONTEXT );

    //#ifndef X_EDITOR
    //s32 nPlayers = g_StateMgr.GetPlayerCount();
    //LOG_MESSAGE( "player::SetLocalPlayer",
    //             "Addr:%08X - LocalSlot:%d - TotalLocalPlayers:%d - NetSlot:%d",
    //             this, m_LocalSlot, nPlayers, m_NetSlot );
    //#endif
}

//=============================================================================

void player::Push( const vector3& PushVector )
{
    if ( !m_bInTurret )
    {
        m_Physics.SetPosition( GetPosition() );
        m_Physics.Push(PushVector);
        OnMove( m_Physics.GetPosition() );
    }
}


void player::UpdateWeaponPullback( void )
{
    if ( m_CurrentAnimState == ANIM_STATE_DEATH )
    {
        m_WeaponCollisionOffset.Zero();
    }
    else{
        //
        // Check to see if the gun will collide with something. If so, move 
        // it back using m_WeaponCollisionOffset.
        //
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if ( pWeapon && (pWeapon->GetType() != TYPE_WEAPON_MUTATION) )
        {
            vector3 FirePos;
            vector3 SingleCollisionOffset( 0.0f, 0.0f, 0.0f );
            vector3 LeftCollisionOffset  ( 0.0f, 0.0f, 0.0f );
            vector3 RightCollisionOffset  ( 0.0f, 0.0f, 0.0f );
            f32     SingleCollisionScalar = 0.0f;
            f32     LeftCollisionScalar   = 0.0f;
            f32     RightCollisionScalar  = 0.0f;
            const vector3 ToWeapon             ( m_EyesPosition - pWeapon->GetPosition() );

            if ( pWeapon->GetFiringBonePosition( FirePos, new_weapon::FIRE_POINT_DEFAULT ) )
            {
                FirePos += ToWeapon;
                SingleCollisionOffset = GetWeaponCollisionOffset( pWeapon->GetGuid(), FirePos );
                SingleCollisionScalar = m_WeaponCollisionOffsetScalar;
            }

            if ( pWeapon->GetFiringBonePosition( FirePos, new_weapon::FIRE_POINT_LEFT ) )
            {
                FirePos += ToWeapon;
                LeftCollisionOffset = GetWeaponCollisionOffset( pWeapon->GetGuid(), FirePos );
                LeftCollisionScalar = m_WeaponCollisionOffsetScalar;
            }

            if ( pWeapon->GetFiringBonePosition( FirePos, new_weapon::FIRE_POINT_RIGHT ) )
            {
                FirePos += ToWeapon;
                RightCollisionOffset = GetWeaponCollisionOffset( pWeapon->GetGuid(), FirePos );
                RightCollisionScalar = m_WeaponCollisionOffsetScalar;
            }

            if ( SingleCollisionScalar > LeftCollisionScalar )
            {
                if ( SingleCollisionScalar > RightCollisionScalar )
                {
                    m_WeaponCollisionOffset         = SingleCollisionOffset;
                    m_WeaponCollisionOffsetScalar   = SingleCollisionScalar;
                }
                else
                {
                    m_WeaponCollisionOffset         = RightCollisionOffset;
                    m_WeaponCollisionOffsetScalar   = RightCollisionScalar;
                }
            }
            else
            {
                if ( LeftCollisionScalar > RightCollisionScalar )
                {
                    m_WeaponCollisionOffset         = LeftCollisionOffset;
                    m_WeaponCollisionOffsetScalar   = LeftCollisionScalar;
                }
                else
                {
                    m_WeaponCollisionOffset         = RightCollisionOffset;
                    m_WeaponCollisionOffsetScalar   = RightCollisionScalar;
                }
            }
        }
        else
        {
            m_WeaponCollisionOffset.Zero();
        }
    }
    m_LastWeaponCollisionOffsetScalar = m_WeaponCollisionOffsetScalar;


}


//=============================================================================

// this adds in the arms offset and keeps the weapon
void player::MoveAnimPlayer( const vector3& Pos )
{
    m_AnimPlayer.SetPosition( Pos + GetAnimPlayerOffset() );
}

//=============================================================================
void player::SetCurrentStrain( void )
{
    if( m_bStrainInitialized == FALSE )
    {
        // HACK
#ifndef X_EDITOR
        if( !GameMgr.IsGameMultiplayer() )
#endif
        {
            m_Faction = FACTION_PLAYER_NORMAL; 
        }

        // Set all movement / control variables to match this strain.
        strain_control_modifiers& StrainControl = m_StrainControls;

        // Move all of the data from the strain control structure to the places where it will get used.
        m_fCrouchChangeRate      = StrainControl.m_StrainCrouchChangeRate;
        m_ProximityAlertRadius   = StrainControl.m_StrainProximityAlertRadius;
        m_MaxFowardVelocity      = StrainControl.m_StrainMaxFowardVelocity;
        m_MaxStrafeVelocity      = StrainControl.m_StrainMaxStrafeVelocity;
        m_JumpVelocity           = StrainControl.m_StrainJumpVelocity;
        m_MaxHealth              = StrainControl.m_StrainMaxHealth;
        m_EyesOffset             = StrainControl.m_StrainEyesOffset;
        m_fYawStickSensitivity   = StrainControl.m_fStrainYawSensitivity;
        m_fPitchStickSensitivity = StrainControl.m_fStrainPitchSensitivity;
        m_fMinWalkSpeed          = StrainControl.m_StrainMinWalkSpeed;
        m_fMinRunSpeed           = StrainControl.m_StrainMinRunSpeed;
        m_fDecelerationFactor    = StrainControl.m_StrainDecelerationFactor;
        m_ReticleMovementDegrade = StrainControl.m_StrainReticleMovementDegrade;
        m_fForwardAccel          = StrainControl.m_fStrainForwardAccel;
        m_fStrafeAccel           = StrainControl.m_fStrainStrafeAccel;
        m_YawAccelTime           = StrainControl.m_StrainYawAccelTime;
        m_PitchAccelTime         = StrainControl.m_StrainPitchAccelTime;

        m_FriendFlags            = m_StrainFriendFlags;

        // Finish necessary initializations.
        m_fOriginalPitchStickSensitivity = m_fPitchStickSensitivity;
        m_fOriginalYawStickSensitivity = m_fYawStickSensitivity;
        m_fMinWalkSpeed *= m_fMinWalkSpeed;
        m_fMinRunSpeed *= m_fMinRunSpeed;

        // Setup the physics
        m_Physics.CopyValues( m_Physics );

        //
        // NOTE: Make sure that the CurrentStrain is set before we go set the weapons up  for this strain!!!
        //
        for( s32 i = 0; i < INVEN_NUM_WEAPONS; i++ )
        {
            new_weapon* pWeapon = GetWeaponPtr( inventory2::WeaponIndexToItem(i) );
            if( pWeapon )
            {
                pWeapon->SetupRenderInformation( );
            }
        }
        // Restart the current state so the new animations start.  This will 
        // most likely change when we get some anims.
        if ( m_bStrainInitialized )
        {
            ShakeView( 1.0f );
        }
    }    
}

//===========================================================================
static f32 s_ArmsVelocityCarryover = 0.5f;
static xbool s_DumpWeapons = FALSE;
f32 CINEMA_DELTA_FADE_T = 0.9f;
extern xbool s_bUseTestMap;
extern s32 s_ScannerTestMap;

#define MAX_JBG_LORE 2

struct s_JBG_LoreIDs
{
    s32 MapID;
    s32 LoreIndex;
};

s_JBG_LoreIDs g_LoreIDs[MAX_JBG_LORE] = 
{   {1075, 4},              // black 0-2, lore index 4 = IDS_LORE_DESC_45 in C:\GameData\A51\Release\PS2\LoreList.txt
    {1090, 0}               // black 2-0, lore index 0 = IDS_LORE_DESC_51 in C:\GameData\A51\Release\PS2\LoreList.txt
};

void player::OnAdvanceLogic( f32 DeltaTime )
{
#ifdef mreed
    static xbool s_PrintSizes = TRUE;

    if ( s_PrintSizes )
    {
        s_PrintSizes = FALSE; // only once
        x_DebugMsg( "===================================================================\n" );
        x_DebugMsg( "Total player size: %7d\n", sizeof( player ) );
        x_DebugMsg( "player:            %7d\n", sizeof( player ) - sizeof( actor ) );
        x_DebugMsg( "actor:             %7d\n", sizeof( actor ) - sizeof( object ) );
        x_DebugMsg( "===================================================================\n" );


    }

    m_bPrimaryMutationFireEnabled = TRUE;
#endif

    CONTEXT( "player::OnAdvanceLogic" );
    LOG_STAT(k_stats_Player);

    // set vibration
#ifndef X_EDITOR
    s32             iPad    = g_IngamePad[ m_LocalSlot ].GetControllerID();
    player_profile& Profile = g_StateMgr.GetActiveProfile( g_StateMgr.GetProfileListIndex(m_LocalSlot));
    input_EnableFeedback( Profile.m_bVibration, iPad );
#endif

    g_ZoneMgr.UpdateEar( m_AudioEarID );
/*
    for( s32 i=0 ; i<ZONELESS ; i++ )
    {
        g_AudioMgr.UpdateEarZoneVolume( m_AudioEarID, i, 0.0f );
    }

    g_AudioMgr.UpdateEarZoneVolume( m_AudioEarID, ZONELESS, 1.0f );
    g_AudioMgr.UpdateEarZoneVolume( m_AudioEarID, GetZone1(), 1.0f );
*/
    //==========================================================================================
    // Setup debug stuff
    //==========================================================================================

#ifndef CONFIG_RETAIL
    m_bCanDie = !DEBUG_INVULNERABLE;
#endif

    //==========================================================================================
    // Begin code from ghost.cpp
    //==========================================================================================
    
    xbool bFlung = m_Physics.Flung();

    DeltaTime *= g_PerceptionMgr.GetPlayerTimeDialation();
    m_DeltaTime = DeltaTime;

    if ( IsChangingMutation() )
    {
        m_MutationChangeTime = 0.0f;
    }
    else
    {
        m_MutationChangeTime += DeltaTime;
    }

    m_UseTime += DeltaTime;

    if( s_DumpWeapons )
    {
        //LogWeapons();
        s_DumpWeapons = 0;
    }

    // KSS -- new cinema code
    if( m_Cinema.m_bUseViewCorrection )
    {
        m_Cinema.m_ViewCorrectionDelta *= CINEMA_DELTA_FADE_T;
    }

    // Store a pointer to the current weapon.  This pointer is only valid 
    // this frame and can only be used in my methods, not engine overloads.  
    // Must be very careful with this.
    new_weapon* pWeapon = GetCurrentWeaponPtr();

    // Check to see if our current weapon is still in our inventory
    if( pWeapon && !m_Inventory2.HasItem( m_CurrentWeaponItem ) )
    {
        // We no longer have the weapon, drop it
        pWeapon = NULL;
    }

    // SB - Added to keep the weapon in sync with the player hands
    //      (player::OnAdvanceLogic now updates the weapon)
    if (( pWeapon ) && ( pWeapon->GetRenderState() != new_weapon::RENDER_STATE_PLAYER ))
    {
        pWeapon->OnAdvanceLogic( DeltaTime );
    }

    actor::OnAdvanceLogic( DeltaTime );    

    UpdateConvulsion();

#ifndef X_EDITOR

    // Online game?
    if( GameMgr.IsGameMultiplayer() )
    {
        // Catch all for the mysteriously "undead" player
        if( ( IsDead() ) || ( GetHealth() <= 0.0f ) )
        {
            // Make sure player is in death state
            if( m_CurrentAnimState != ANIM_STATE_DEATH )
            {
                // Make sure third person camera is setup
                SetupThirdPersonCamera();
                third_person_camera* pThirdPersonCamera = GetThirdPersonCamera();
                if ( pThirdPersonCamera )
                {
                    pThirdPersonCamera->MoveTowardsPitch( -R_30 );
                }
                ForceMutationChange( FALSE );
                
                // Put player into death state!
                SetAnimState( ANIM_STATE_DEATH );
            }
            
            // Test for re-spawn being pressed 
            // (just in case for some reason we can't get into death state)
            xbool bPrimaryPressed = (xbool)g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_PRIMARY ).WasValue;
            if( bPrimaryPressed )
            {
                m_bWantToSpawn = TRUE;
                m_NetDirtyBits |= WANT_SPAWN_BIT;  // NETWORK
                m_bRespawnButtonPressed = TRUE;
            }
        }
    }

    // if we have just lost our spawn neutrality, play switch-to
    if ( GameMgr.GetGameType() != GAME_CAMPAIGN )
    {
        if ( m_SpawnNeutralTime > 0.0f )
        {
            m_bHidePlayerArms = TRUE;
        }
        else 
        {
            m_bHidePlayerArms = FALSE;
        }
    }

    // see if we have the JBG lore item
    if( !GetJBGLoreAcquired() )
    {
        player_profile& Profile = g_StateMgr.GetActiveProfile( g_StateMgr.GetProfileListIndex(0) );
        s32 VaultIndex, i;        

        for( i = 0; i < MAX_JBG_LORE; i++ )
        {
            if( s_bUseTestMap )
            {
                // For testing
                g_LoreList.GetVaultByMapID( s_ScannerTestMap, VaultIndex );
            }
            else
            {
                g_LoreList.GetVaultByMapID( g_LoreIDs[i].MapID, VaultIndex );
            }

            if( Profile.GetLoreAcquired( VaultIndex, g_LoreIDs[i].LoreIndex ) )
            {
                // found a lore object that unlocks expert mode.
                m_bJBGLoreAcquired = TRUE;

                // Get out.
                break;
            }
        }
    }
#endif

    // If we have turned on cinema mode and aren't in the cinema state then switch states
    if(    (m_Cinema.m_bCinemaOn ==  TRUE) 
        && (m_CurrentAnimState   !=  ANIM_STATE_CINEMA) )
    {
        if ( m_CurrentAnimState != ANIM_STATE_MISSION_FAILED )
        {
            if ( !IsChangingMutation() )
            {
                SetAnimState( ANIM_STATE_CINEMA );
            }
        }
        else
        {
            BeginCinema();                              // Get the hud and stuff set up
            SetAnimState( ANIM_STATE_MISSION_FAILED );  // Don't lose our state
            m_Cinema.m_bCinemaOn = FALSE;               // Avoid doing this next time
        }
    }
    
    if( m_bHidePlayerArms )
    {
        // arms weren't hidden but now they are, make sure we clear zoom
        if( !m_bArmsWereHidden )
        {
            // we have a weapon and our arms 
            if( GetCurrentWeaponPtr() )
            {
                GetCurrentWeaponPtr()->ClearZoom();

                // make sure we give it a proper state
                SetAnimState(ANIM_STATE_IDLE);
                m_NextAnimState = ANIM_STATE_UNDEFINED;
            }
        }

        // be sure we set this so we know the previous state of the arms being hidden
        m_bArmsWereHidden = TRUE;
    }
    else
    {
        // the arms WERE hidden and now they're not, play switch to animation.
        if( m_bArmsWereHidden && m_bPlaySwitchTo )
        {
            SetAnimState( ANIM_STATE_SWITCH_TO );
            m_bArmsWereHidden = FALSE;
        }
    }   

    pWeapon = NULL;

    // We need to update the airborn state even if we don't have an avatar.
    SetIsAirborn( m_Physics.IsAirborn() );

    if( m_Physics.GetFallMode() )
    {
        m_bFalling = TRUE; // used for pain when we land
    }
    else if( m_bFalling )
    {
        // We've just landed
        m_bJustLanded = TRUE;
        // Hurt?
        if( !bFlung )
        {
            TakeFallPain();
        }

        m_bFalling = FALSE;
    }

    //make sure the jumpguid get's cleared if we aren't falling.
    if( !m_bFalling )
    {
        m_JumpedOffLadderGuid = 0;
    }

    // Handled by player
    WakeUpDoors();
    m_InvalidSoundTimer = x_max( 0.0f, m_InvalidSoundTimer - DeltaTime );

    //==========================================================================================
    // End code from ghost.cpp
    //==========================================================================================
    
    xbool bJumpingBeanFirst = FALSE;

#ifndef X_EDITOR
    // in campaign mode, use special rule
    if( GameMgr.GetGameType() == GAME_CAMPAIGN )
    {
        // LevelID 1030 = Blue 1-2.  While in this map the JBG takes priority even though it's non-expert mode
        bJumpingBeanFirst = (g_ActiveConfig.GetLevelID() == 1030);
    }
    else
    {
        bJumpingBeanFirst = TRUE;
    }
#endif

    // we have expert mode, JBG goes first in order
    if( GetJBGLoreAcquired() )
    {
        bJumpingBeanFirst = TRUE;
    }

    xbool bOutOfFrag = (m_Inventory2.GetAmount( INVEN_GRENADE_FRAG ) <= 0);

    // make sure we use the "jumping beans" first
#ifndef CONFIG_RETAIL
    if( (m_Inventory2.GetAmount( INVEN_GRENADE_JBEAN ) > 0) && (bJumpingBeanFirst || DEBUG_EXPERT_JUMPINGBEAN || bOutOfFrag) )
#else
    if( (m_Inventory2.GetAmount( INVEN_GRENADE_JBEAN ) > 0) && (bJumpingBeanFirst || bOutOfFrag) )
#endif
    {
        m_CurrentGrenadeType2 = INVEN_GRENADE_JBEAN;
    }
    else 
    {
        m_CurrentGrenadeType2 = INVEN_GRENADE_FRAG;
    }

    UpdateSafeSpot ( DeltaTime );
    UpdateUserInput( DeltaTime ); 
    UpdateBulletSounds( DeltaTime );

    // recharge/burn our mutagen if we have the mutation ability
    if( m_Inventory2.HasItem( INVEN_WEAPON_MUTATION ) )
    {
        UpdateMutagen( DeltaTime );
    }
    
    // drain/charge flashlight battery
    UpdateFlashlightBattery(DeltaTime);

    pWeapon = GetCurrentWeaponPtr();
    if( m_bIsMutated && !m_bWasMutated )
    {
        SetMutated( TRUE );
    }

    m_bWasMutated = m_bIsMutated;

    UpdateState( DeltaTime );

    m_DebounceTime += DeltaTime; 

    if( s_WeaponInventoryStrip )
    {
        RemoveAllWeaponInventory();
        s_WeaponInventoryStrip = FALSE;
    }

    // First advance initializes the strain.
    if( !m_bStrainInitialized )
    {
        SetCurrentStrain( );
        m_bStrainInitialized = TRUE;
    }   

    UpdateAudio( DeltaTime );    
    GatherGameSpeakGuid();
    
    UpdateActiveNonExclusiveStates( DeltaTime );
    
    // SB: Update ghost loco for split-screen / network games
    //     NOTE: The location of this is crucial (after player position and weapon switching
    //           have been processed, before weapon is positioned) to keep the avatars weapons
    //           and animation in sync. If you need to move it, please come talk to me first! 
    //           Thanks.
    if( !m_bDead && UsingLoco() )
    {
        //
        // Update loco (NOTE: Loco position will get updated from actor::OnMove)
        //
        m_pLoco->SetGhostIsMoving( m_Physics.GetVelocity().LengthSquared() > x_sqr( 0.01f ) );
        m_pLoco->SetPitch( m_Pitch );
        m_pLoco->SetYaw( m_Yaw );
        OnAdvanceGhostLogic( DeltaTime );
    }
    
    // Advance the animation for the arms.
    if( m_AnimGroup.GetPointer() )
    {
        vector3 Pos;
        m_AnimPlayer.Advance( DeltaTime, Pos );
        
        // Store a pointer to the current weapon.  This pointer is only valid 
        // this frame and can only be used in my methods, not engine overloads.  
        // Must be very careful with this.
        if( pWeapon )
        {
            // Update weapon here so muzzle fx is in sync!
            AttachWeapon();

            // run logic
            pWeapon->OnAdvanceLogic( DeltaTime );
        }
    }   

    //handle the animation events
    OnAnimEvents();

    if ( !m_Physics.GetFallMode() && m_bJustLanded )
    {
        // We've just landed
        // Keep the arms going
               const f32 DistanceFell   = m_FellFromAltitude - GetPosition().GetY(); // we need this before TakeFallPain()
        
        // SB: In the editor, this can be negative when you first run the game?!
        if( DistanceFell > 0.001f )
        {               
         static const f32 Gravity        = 980.0f;
                const f32 TimeFalling    = x_sqrt( DistanceFell / Gravity );
                const f32 FallSpeed      = Gravity * TimeFalling * s_ArmsVelocityCarryover;
            m_ArmsVelocity += vector3( 0.0f, -FallSpeed, 0.0f );
        }
                    
        m_bJustLanded = FALSE;
    }

    pWeapon = NULL;

    // Update any effects
    if( m_pEffects )
        m_pEffects->Update( this, DeltaTime );
}

//==============================================================================
xbool player::UseFocusObject( void )
{
    // 
    // Look for all focus objects in the world, and try to press every single
    // one of them until we find one that works.  
    //
    xbool UsePressed = (xbool)g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_USE ).WasValue;
#if defined(TARGET_PC) && !defined(X_EDITOR)
    UsePressed |= input_WasPressed( INPUT_MOUSE_BTN_C );
#endif

    if( UsePressed && (m_CurrentAnimState != ANIM_STATE_THROW ) && !IsChangingMutation() )
    {
        slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_FOCUS_OBJECT );
        while( SlotID != SLOT_NULL )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
            SlotID = g_ObjMgr.GetNext(SlotID);

            if (pObject && pObject->IsKindOf( focus_object::GetRTTI()))
            {
                focus_object* pFocusObject = (focus_object*)pObject;
                if( pFocusObject->TestPress() )
                {
                    // If we successfully used the FO, then we don't want to 
                    // do anything else with this button press.
                    m_UseTime = 0.0f;
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

//==============================================================================

xbool player::NearMutagenReservoir( void )
{
    xbool RetVal = FALSE;

    g_ObjMgr.SelectBBox( ATTR_ALL, GetBBox(), TYPE_MUTAGEN_RESERVOIR );
    slot_id ObjectSlot = SLOT_NULL;
    object* pObject = NULL;
    for ( ObjectSlot = g_ObjMgr.StartLoop();
          ObjectSlot != SLOT_NULL;
          ObjectSlot = g_ObjMgr.GetNextResult( ObjectSlot ) )
    {
        pObject = g_ObjMgr.GetObjectBySlot( ObjectSlot );
        if ( pObject
          && GetBBox().Intersect( pObject->GetBBox() ) )
        {
            RetVal = TRUE;
            break;
        }
    }

    g_ObjMgr.EndLoop();

    return RetVal;
}

//==============================================================================
void player::AcquireAllLoreObjects( void )
{
    // already done
    if( m_bAllLoreObjectsCollected )
    {
        return;
    }

    // make sure flag is set
    m_bAllLoreObjectsCollected = TRUE;

#ifndef X_EDITOR
    // acquire all lore objects
    int i = 0;
    for( i = 0; i < MAX_LORE_ITEMS; i++ )
    {
        lore_object* pLoreObject = (lore_object*)g_ObjMgr.GetObjectByGuid(m_LoreObjectGuids[i]);

        // if valid and is still active, get it.
        if( pLoreObject && pLoreObject->IsActivated() )
        {
            pLoreObject->OnAcquire();
        }
    }
#endif
    
}

//==============================================================================
void player::LoadAllLoreObjects( void )
{
    int count = 0;

    // load up all the lore objects for collecting and for updating the Geiger counter
    slot_id SlotID = g_ObjMgr.GetFirst(object::TYPE_LORE_OBJECT);
    while( SlotID != SLOT_NULL && count < MAX_LORE_ITEMS )
    {
        // Lookup lore object
        object* pObj              = g_ObjMgr.GetObjectBySlot(SlotID);
        lore_object*  pLoreObject = &lore_object::GetSafeType( *pObj );

        if( !pLoreObject )
        {
            ASSERT(0);
            continue;
        }

        // true lore object
        if( pLoreObject->IsTrueLoreObject() )
        {
#ifndef X_EDITOR
            s32 VaultIndex = -1;
            m_LoreObjectGuids[count] = pLoreObject->GetGuid();

            if( s_bUseTestMap )
            {
                // For testing
                g_LoreList.GetVaultByMapID( s_ScannerTestMap, VaultIndex );
            }
            else
            {
                s32 MapID = g_ActiveConfig.GetLevelID();
                g_LoreList.GetVaultByMapID( MapID, VaultIndex );
            }

            // only do this if we're in campaign mode
            if( GameMgr.GetGameType() == GAME_CAMPAIGN )
            {
                player_profile& Profile = g_StateMgr.GetActiveProfile( g_StateMgr.GetProfileListIndex(0) );
                
                if( VaultIndex != -1 )
                {
                    // we have this one
                    if( Profile.GetLoreAcquired( VaultIndex, pLoreObject->GetLoreID() ) )
                    {
                        // silently acquire the lore object.
                        pLoreObject->OnAcquire( TRUE );
                    }
                }
            }
#endif
            count++;
        }
    
        // Get next lore object
        SlotID = g_ObjMgr.GetNext(SlotID);
    }

    m_bAllLoreObjectsCollected = FALSE;
}

//==============================================================================

xbool player::GetClosestLoreObjectDist( f32 &ClosestDist )
{
    if( m_bAllLoreObjectsCollected )
    {
        return FALSE;
    }

    ClosestDist = Lore_Max_Detect_DistanceTweak.GetF32() + 100.0f; // 31 meters; 1 meter greater than max distance
    int i;
    xbool bAllCollected = TRUE;
    xbool bFound        = FALSE;

    for( i = 0; i < MAX_LORE_ITEMS; i++ )
    {
        lore_object* pLoreObject = (lore_object*)g_ObjMgr.GetObjectByGuid(m_LoreObjectGuids[i]);
        if( pLoreObject && pLoreObject->IsActivated() && (pLoreObject->GetLoreID() != -1) )
        {
            // we found an active one
            bAllCollected = FALSE;

            f32 dist = (GetPosition() - pLoreObject->GetPosition()).Length();

            // is this one closer?
            if( dist < ClosestDist )
            {
                bFound = TRUE;
                ClosestDist = dist;
            }
        }
    }

    // all are activated
    if( bAllCollected )
    {
        m_bAllLoreObjectsCollected = TRUE;
    }

    return bFound;
}

//==============================================================================
void player::InitFlashlight( const vector3& rInitPos )
{
    // create the flashlight (todo--make this part of the weapon blueprint so artists/designers can play.)
    if( m_FlashlightGuid != 0 )
    {
        // already created
        return;
    }
    else
    {
        m_FlashlightGuid = g_ObjMgr.CreateObject( projector_obj::GetObjectType() );
    }

    object_ptr<projector_obj> ProjObj(m_FlashlightGuid);

    if( ProjObj.IsValid() )
    {
        texture::handle Texture;

        // set visuals
        Texture.SetName( PRELOAD_FILE("Flashlight.xbmp") );
        ProjObj.m_pObject->SetShadow( FALSE );
        ProjObj.m_pObject->SetActive( FALSE );
        ProjObj.m_pObject->SetIsFlashlight( TRUE );
        ProjObj.m_pObject->SetFOV( R_60 );
        ProjObj.m_pObject->SetLength( 2000.0f );
        ProjObj.m_pObject->SetTextureHandle( Texture );
        ProjObj.m_pObject->OnMove( rInitPos );
        ProjObj.m_pObject->SetZone1( GetZone1() );
        ProjObj.m_pObject->SetZone2( GetZone2() );
    }
    else
    {
        ASSERTS(0, "Invalid Flashlight");
    }
}

//==============================================================================

xbool player::IsFlashlightActive( void )
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    
    // validate weapon
    if( pWeapon )
    {
        object_ptr<projector_obj> ProjObj(m_FlashlightGuid);

        // is the flashlight object valid and is the bone valid?
        if ( ProjObj.IsValid() && pWeapon->CheckFlashlightPoint() )
        {
            return ProjObj.m_pObject->IsActive();
        }
    }
    
    return FALSE;
}

//==============================================================================

void player::SetFlashlightActive( xbool bOn )
{

// Just for MsgMgr testing purposes.
#if defined(aharp) && !defined(X_EDITOR)
    //MsgMgr.Message( MSG_STRING, 0, (s32)((const xwchar*)xwstring("Toggling Flashlight!")) );
    //static s32 GoalID = -1;              
/*
    static xbool bRegistered = FALSE;
    static msg_id NewMsg = (msg_id)3;

    if( g_NetworkMgr.IsServer() )
    {
        if( !bRegistered )
        {
            NewMsg =
                MsgMgr.RegMsg( (msg_id)-1, HEAR_ALL, IMPACT_URGENT, ARG_NONE, ARG_NONE, ARG_NONE, ARG_NONE, 
                    (const xwchar*)xwstring("This is a test of the emergency broadcasting system"), (const xwchar*)NULL, (const xwchar*)NULL, 
                    (const char*)xstring("FLAG_CAP_BAD"), 
                    NULL, 
                    BONUS, TRUE );

            bRegistered = TRUE;
        }
        MsgMgr.Message( NewMsg, 0 );
    }
*/
    /*
    if( bOn )
    {
        MsgMgr.Message( MSG_GOAL_FLASHLIGHT, 0 );
    } 
    else
    {
        MsgMgr.Message( MSG_BONUS, 0 );
        MsgMgr.Message( MSG_SOUND, 0 );
    }
    */

#endif


    new_weapon* pWeapon = GetCurrentWeaponPtr();

    // validate weapon
    if( pWeapon )
    {
        object_ptr<projector_obj> ProjObj(m_FlashlightGuid);
        
        // is the flashlight object valid and is the bone valid?
        if ( ProjObj.IsValid() && pWeapon->CheckFlashlightPoint() )
        {
            ProjObj.m_pObject->SetActive(bOn);
        }

        MoveFlashlight();

        const xbool bWasUsingFlashlight = m_bUsingFlashlight;
        m_bUsingFlashlight = bOn;

        if ( bWasUsingFlashlight != m_bUsingFlashlight )
        {
            actor_effects* pActorEffects = GetActorEffects( TRUE );

            if ( m_bUsingFlashlight )
            {
                if ( pActorEffects )
                {
                    pActorEffects->InitEffect( actor_effects::FX_FLASHLIGHT, this );
                }
            }
            else
            {
                if( pActorEffects )
                {
                    pActorEffects->KillEffect( actor_effects::FX_FLASHLIGHT );
                }
            }
        }

        #ifndef X_EDITOR
        m_NetDirtyBits |= FLASHLIGHT_BIT;
        #endif
    }
    else
    {
        // set using flashlight to false because weapon is invalid
        m_bUsingFlashlight = FALSE;

        #ifndef X_EDITOR
        m_NetDirtyBits |= FLASHLIGHT_BIT;
        #endif
    }

    // flashlight is off, reset flashlight timeout
    if( m_bUsingFlashlight == FALSE )
    {
        m_FlashlightTimeout = FlashlightAutoOffSecondsTweak.GetF32();
    }
}

//==============================================================================
void player::UpdateFlashlightBattery( f32 nDeltaTime )
{
    // KSS -- people hate the flashlight battery, so removing it.
    // battery "conservation" :)
    if( IsFlashlightActive() )
    {
        // if we are playing a cinematic, don't burn battery
        if( IsCinemaRunning() )
        {
            if ( m_bUsingFlashlight )
            {
                m_bUsingFlashlightBeforeCinema = TRUE;
                // if the flashlight is active and a cinema is running, turn it off and get out
                SetFlashlightActive(FALSE);
            }

            return;
        }

        u8 Brightness = (u8)GetFloorIntensity();
        
        // if light levels are high, turn off flashlight after FLASHLIGHT_AUTOOFF_TIME seconds
        if( Brightness > FlashlightAutoOffBrightnessTweak.GetF32() )
        {
            m_FlashlightTimeout -= nDeltaTime;
            
            if( m_FlashlightTimeout <= F32_MIN )
            {
                // this will reset timer as well
                SetFlashlightActive(FALSE);
            }
        }
        else
        {
            m_FlashlightTimeout = FlashlightAutoOffSecondsTweak.GetF32();
        }
    }
}
//==============================================================================

xbool player::AddBattery( const f32& nDeltaBattery )
{        
    // do not allow Battery to go above max.
    if( m_Battery == m_MaxBattery && nDeltaBattery > 0.0f )
    {
        return FALSE;
    }
    else if( (m_Battery + nDeltaBattery) < 0.0f )  // does what we are using take us below 0?
    {
        // don't have enough
        return false;
    }
    else
    {
        // add/subtract Battery
        m_Battery = fMin( m_Battery + nDeltaBattery , m_MaxBattery );
        m_Battery = fMax( m_Battery , 0.0f );

        return TRUE;
    }

    return FALSE;
}

//==============================================================================

void player::MoveFlashlight( void )
{
    if( IsFlashlightActive() )
    {
        new_weapon* pWeapon = GetCurrentWeaponPtr();
    
        // validate weapon
        if( pWeapon )
        {
            object_ptr<projector_obj> ProjObj(m_FlashlightGuid);
            
            // validate flashlight object and flashlight bone
            if ( ProjObj.IsValid() && pWeapon->CheckFlashlightPoint() )
            {
                matrix4 L2W;
                vector3 Vect;
                
                // transform if we are in the proper state
                if( pWeapon->GetFlashlightTransformInfo(L2W, Vect) )
                {
                    L2W.PreTranslate(Vect);
                    L2W.PreRotateY(R_180);
                    L2W.PreTranslate( vector3(0.0f, 0.0f, -100.0f) );
                    ProjObj.m_pObject->OnTransform( L2W );

                    // set flashlight zones to the player's zones
                    ProjObj.m_pObject->SetZones(GetZones());
                }
            }
            else
            {
                // kill flashlight if we don't have a weapon
                ProjObj.m_pObject->SetActive(FALSE);
            }            
        }
    }    
}

//==============================================================================

void player::OnDeath( void )
{
    #ifndef X_EDITOR
//  LOG_MESSAGE( "player::OnDeath", "Slot:%d", m_NetSlot );
    #endif

    new_weapon *pWeapon = GetCurrentWeaponPtr();

    // get rid of our weapon (mostly for multiplayer)
    if( pWeapon )
    {
        // Make sure to stop any looping weapon sfx
        pWeapon->ReleaseAudio();
    
        // end zoom
        pWeapon->ClearZoom();

        // shut off flashlight
        SetFlashlightActive(FALSE);        

        // no moving
        ClearStickInput();    
    
        // clear weapon
        SetAnimState(ANIM_STATE_IDLE);
        m_NextAnimState = ANIM_STATE_UNDEFINED;
    }

    actor::OnDeath();

#ifndef X_EDITOR
    if ( GameMgr.IsGameMultiplayer() )
    {
        // we need to use a third-person camera
        SetupThirdPersonCamera();
        third_person_camera* pThirdPersonCamera = GetThirdPersonCamera();
        if ( pThirdPersonCamera )
        {
/* mreed: 11/12/04 this didn't work yet, so it's not ready to go in
            const s32 Count = m_LastPainEvent.GetCount();
            if ( Count > 0 )
            {
                //
                // Figure out where to point the camera
                //
                pain& Pain = m_LastPainEvent[Count - 1];

                // Default PainPos
                vector3 PainPos = Pain.GetPosition();

                // Try to determine the origin of the pain
                object* pPainOrigin = g_ObjMgr.GetObjectByGuid( Pain.GetOriginGuid() );
                if ( pPainOrigin )
                {
                    PainPos = pPainOrigin->GetBBox().GetCenter();
                }

                // Now tell the camera to face PainPos
                const vector3 ToPain( PainPos - GetPosition() );
                static f32 s_TPCDist = 5000.0f;
                pThirdPersonCamera->MoveTowards( -R_30, ToPain.GetYaw(), s_TPCDist );
            }
            else
*/
            {
                pThirdPersonCamera->MoveTowardsPitch( -R_30 );
            }
        }
        ForceMutationChange( FALSE );
    }
    else
    {
        // Set this so the state mgr knows what is going on.
        s_bPlayerDied = TRUE;
    }

#endif

    SetAnimState( ANIM_STATE_DEATH );

    // tell perception manager we died
    if (IsMutated())
        g_PerceptionMgr.EndMutate();    

    ClearAllNonExclusiveStates();

    if ( m_bInTurret )
    {
        // make sure we don't go flying
        GetLocoPointer()->m_Physics.SetVelocity( vector3(0.0f,0.0f,0.0f) );
    }
    
    m_bInTurret     = FALSE;

    // reset lore flag.  It will get set to true again if all are actually collected (self-fixing).
    m_bAllLoreObjectsCollected = FALSE;

    m_LeanState      = LEAN_NONE;
    m_SoftLeanAmount = 0.0f;
    m_LeanWeaponOffset.Zero();
}

//==============================================================================

void player::OnMissionFailed( s32 TableName, s32 ReasonName )
{
#ifndef X_EDITOR
    LOG_MESSAGE( "player::OnMissionFailed", "Slot:%d", m_NetSlot );
#endif

    m_MissionFailedTableName    = TableName;
    m_MissionFailedReasonName   = ReasonName;

    // We need to die, then go into ANIM_STATE_MISSION_FAILED
    OnDeath();
    SetAnimState( ANIM_STATE_MISSION_FAILED );
}

//==============================================================================

void player::OnSpawn( void )
{
    #ifndef X_EDITOR
//  LOG_MESSAGE( "player::OnSpawn", "Slot:%d", m_NetSlot );
    #endif

    actor::OnSpawn();

#ifndef X_EDITOR
    m_NetDirtyBits |= WEAPON_BIT;  // NETWORK
#endif X_EDITOR

    // initialize the zone tracker to the player position
    g_ZoneMgr.InitZoneTracking( *this, m_ZoneTracker );

    // Activate the sound emitters.
    //extern void ActivateSoundEmitters( const vector3& Position );
    //ActivateSoundEmitters( GetPosition() );

    // Reset the state.
    SetAnimState( ANIM_STATE_IDLE );

    // refill mutagen
    AddMutagen(GetMaxMutagen());
}

//=============================================================================
extern xbool g_ShowLoreObjectCollision;
extern f32 g_LO_SphereSize;
extern f32 g_LO_RenderDist;
extern f32 g_Dist;

#ifdef DEBUG_GRENADE_THROWING
    xbool g_ShowGrenadeEventCollision = FALSE;
    vector3 g_EventPos = vector3(0.0f, 0.0f, 0.0f);
    vector3 g_NewEventPos = vector3(0.0f, 0.0f, 0.0f);
#endif

void player::OnRenderTransparent(void)
{
    actor::OnRenderTransparent();

#ifndef X_EDITOR
    if ( m_CurrentAnimState == ANIM_STATE_MISSION_FAILED )
    {

        if( x_GetLocale() == XL_LANG_ENGLISH )
        {
            xbitmap* pBitmap = m_MissionFailedBmp.GetPointer();
            if( pBitmap )
            {
                draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_USE_ALPHA | DRAW_2D | DRAW_NO_ZBUFFER );
                draw_SetTexture(*pBitmap);
                draw_DisableBilinear();
                f32 X = f32( 256 - (m_MissionFailedBmp.GetPointer()->GetWidth()/2) );
                f32 Y = 100.0f;
                draw_Sprite( 
                    vector3(X,Y,0.0f), 
                    vector2((f32)m_MissionFailedBmp.GetPointer()->GetWidth(), (f32)m_MissionFailedBmp.GetPointer()->GetHeight()), 
                    g_HudColor);
                draw_End();
            }
        }
        else
        {
            // display a localized text message instead of the bitmap            
            irect Rect( 0, 100, 512, 130 );
            RenderLine( (xwchar*)g_StringTableMgr( "ui", "IDS_MISSION_FAILED" ), Rect, 255, g_HudColor, 0, ui_font::h_center | ui_font::v_top  );
        }

        const s32 mfx = 0;
        const s32 mfy = 180;
        xcolor Color( XCOLOR_RED );
        irect Rect;
        Rect.Set( mfx, mfy + 50, 512, mfy+51 );
        Color.Set( XCOLOR_YELLOW );
        RenderLine( (xwchar*)g_StringTableMgr( g_StringMgr.GetString( m_MissionFailedTableName ), g_StringMgr.GetString( m_MissionFailedReasonName ) ), Rect, 255, Color, 0, ui_font::h_center | ui_font::v_top  );
    }
#endif

#ifndef X_RETAIL
    if( g_ShowLoreObjectCollision )
    {
        vector3 StartPos, EndPos;
        s32 i = 0;

        for( i = 0; i < MAX_LORE_ITEMS; i++ )
        {
            lore_object* pLoreObject = (lore_object*)g_ObjMgr.GetObjectByGuid(m_LoreObjectGuids[i]);

            if( !pLoreObject ) continue;

            pLoreObject->DoCollisionCheck(this, StartPos, EndPos);

            vector3 Diff = EndPos-StartPos;
            g_Dist = Diff.Length();

            // only render the debug stuff for the one that is close to us
            if( g_Dist < g_LO_RenderDist )
            {
                // default modifier to full distance in case the collision manager returns no collisions
                f32 DistModifier = 1.0f;

                // if we don't hit anything, T is undefined
                if( g_CollisionMgr.m_nCollisions > 0 )
                {
                    DistModifier = g_CollisionMgr.m_Collisions[0].T;

                    object *pHitObj = g_ObjMgr.GetObjectByGuid(g_CollisionMgr.m_Collisions[0].ObjectHitGuid);

                    if( pHitObj )
                    {
                        draw_Line(StartPos, pHitObj->GetPosition());
                        draw_Sphere(pHitObj->GetPosition(), g_LO_SphereSize, XCOLOR_GREEN);
                    }
                }

                // get our new end position
                EndPos = StartPos + (DistModifier*Diff);

                //draw_Sphere(Pos, 10.0f, XCOLOR_WHITE);
                draw_Line(StartPos, EndPos);

                if( g_CollisionMgr.m_nCollisions )
                {
                    draw_Sphere(EndPos, g_LO_SphereSize, XCOLOR_RED);
                }
                else
                {
                    // we can see it
                    draw_Sphere(EndPos, g_LO_SphereSize, XCOLOR_WHITE);
                }

                pLoreObject->OnColRender( TRUE );
            }
        }
    }

#ifdef DEBUG_GRENADE_THROWING
    if( g_ShowGrenadeEventCollision )
    {
        // get player position
        vector3 Point1 = vector3(0.0f, 0.0f, 0.0f);
        // not set yet
        vector3 Point2 = vector3(0.0f, 0.0f, 0.0f);    
        // get event position
        vector3 Point3 = g_EventPos;

        GetThrowPoints( Point1, Point2, Point3 );

        draw_ClearL2W();

        draw_Sphere(Point1, 5.0f);
        draw_Sphere(Point2, 6.0f, XCOLOR_GREEN);
        draw_Sphere(Point3, 7.0f, XCOLOR_YELLOW);
        draw_Sphere(g_NewEventPos, 8.0f, XCOLOR_RED);
    }
#endif // DEBUG_GRENADE_THROWING

#endif // X_RETAIL

    //render weapon
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon )
    {
        // set the proper render state so we can refrain from drawing the 1st person muzzle fx
        if( IsAvatar() && pWeapon->IsUsingSplitScreen() )
        {
            pWeapon->SetRenderState( new_weapon::RENDER_STATE_NPC );
        }

        pWeapon->OnRenderTransparent();

        // put renderstate back like it was
        pWeapon->SetRenderState( new_weapon::RENDER_STATE_PLAYER );
    }
}

//===========================================================================

void player::OnRender( void )
{
    CONTEXT( "player::OnRender" );

    //
    // Make sure 1st person/3rd person weapon is in correct position for rendering!
    // This chunk of code makes sure that the weapon is in the right place for the
    // weapon pullback calculations, preventing it from being in an avatar position,
    // when it's supposed to be in the player rig's hands
    //
#ifdef X_EDITOR
    const xbool bIsSplitScreen = FALSE;
#else
    const xbool bIsSplitScreen = (g_NetworkMgr.GetLocalPlayerCount() > 1);
#endif
    if( bIsSplitScreen )
    {
        if( IsAvatar() )
        {
            // this will move the weapon into the avatar's hands
            actor::MoveWeapon( TRUE );  // 3rd person
        }
        else
        {
            // this will move the weapon into the player rig hands
            OnMoveWeapon(); // 1st person
        }
    }

    UpdateWeaponPullback();

#ifndef X_EDITOR
    // If the rendering isn't short circuited then dead players will
    // still be rendered in split screen as their corpse falls out of them.
    if( IsDead() && (g_RenderContext.NetPlayerSlot != m_NetSlot) )
    {
        return;
    }
#endif

    if ( GetThirdPersonCamera() && IsDead() )
    {
        return;
    }

    if ( !m_bIsMutated )
    {
        if ( m_Inventory2.GetAmount( INVEN_GLOVES ) > 0.0f )
        {
            // set virtual mesh for gloves
            m_Skin.SetVMeshBit( "MESH_Arms_Hazmat",  TRUE  );
            m_Skin.SetVMeshBit( "MESH_Hands_Bare",   FALSE );
            m_Skin.SetVMeshBit( "MESH_Hands_Hazmat", TRUE  );

        }
        else
        {
            // set virtual mesh for no gloves
            m_Skin.SetVMeshBit( "MESH_Arms_Hazmat",  TRUE  );
            m_Skin.SetVMeshBit( "MESH_Hands_Bare",   TRUE  );
            m_Skin.SetVMeshBit( "MESH_Hands_Hazmat", FALSE );
        }
    }

#if defined(X_EDITOR)
    if ( g_ShowPlayerPos )
    {
        vector3 Pos = GetPosition();
        x_printfxy( 1, 2, "Player( %7.1f, %7.1f, %7.1f )", Pos.GetX(), Pos.GetY(), Pos.GetZ() );
    }
#endif

#ifndef X_RETAIL

#ifdef ksaffel
    //x_printfxy( 0, 20, "Battery: %3.0f/%3.0f", GetBattery(), GetMaxBattery() );
    //u8 Brightness = (u8)GetFloorIntensity();
    //x_printfxy( 0, 20, "Brightness: %d", Brightness );
#endif

    if( g_RenderTendrilCollision )
    {
        guid DirectHitGuid=0;
        vector3 HitPosition;
        {
            DoTendrilCollision();
            if( g_CollisionMgr.m_nCollisions )
            {
                DirectHitGuid = g_CollisionMgr.m_Collisions[0].ObjectHitGuid;
                HitPosition = g_CollisionMgr.m_Collisions[0].Point;

                vector3 StartPos = GetView().GetPosition();
                // default modifier to full distance in case the collision manager returns no collisions
                f32 DistModifier = 1.0f;

                // if we don't hit anything, T is undefined
                if( g_CollisionMgr.m_nCollisions > 0 )
                {
                    DistModifier = g_CollisionMgr.m_Collisions[0].T;
                }

                tweak_handle ReachDistanceTweak("PLAYER_TendrilReachDistance");
                tweak_handle SphereRadiusTweak("PLAYER_TendrilCheckRadius");
                vector3 EndPos = StartPos + (GetView().GetViewZ() * ReachDistanceTweak.GetF32());

                // get our new end position
                EndPos = StartPos + (DistModifier*(EndPos-StartPos));

                draw_Line(StartPos, EndPos);
                draw_Sphere(EndPos, SphereRadiusTweak.GetF32());
            }
        }
    }
#endif

    // We need to render debug stuff at least, if sniper zoom is enabled
    RenderAimAssistDebugInfo();

    if( RenderSniperZoom()   ||                 // if we're in sniper mode, don't render player arms and such
        m_Cinema.m_bCinemaOn ||                 // if we are playing a cinematic, don't draw arms
        (m_bHidePlayerArms && !IsAvatar()) )    // Has a trigger or something turned off our arms?
    {
        // KSS -- FIXME -- HACK -- This will cause sniper zoom on moving platforms to now work.
        // PREVIOUSLY, you would get locked in and were not able to YAW at all.
        const matrix4& mat = GetL2W();
        (void)mat;

        return;
    }

    //x_printfxy( 2,10, "Zones: %d %d", m_ZoneTracker.GetMainZone(), m_ZoneTracker.GetZone2() );

    if( !IsAvatar() )
    {
        // gather flags and ambient color
        xcolor Ambient;
        u32    Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;
        if ( g_RenderContext.m_bIsMutated && !g_RenderContext.m_bIsPipRender && m_bAllowedToGlow )
        {
            Flags  |= render::GLOWING;

            // TODO: Fill in the logic for determining if this is friend or foe.

            // TODO: This color should come from the blueprint properties (m_EnemyGlowColor)
            Ambient = xcolor(255,200,200,255);
        }
        else
        {
            Ambient = GetFloorColor();
        }


#ifdef X_EDITOR
        if( m_bRenderBBox )
        {
            if( GetAttrBits() & ATTR_EDITOR_SELECTED )
            {
                draw_BBox( GetBBox(), XCOLOR_RED );
                draw_Frustum( GetView() );
            }
        }
#endif // X_EDITOR

        if ( !m_bActivePlayer )
            return;

        if( m_LocalSlot == -1 )
            return;

#if defined(X_EDITOR)
        const view* ActiveView = eng_GetView();
        if ( ActiveView && ((ActiveView->GetPosition() - GetView().GetPosition()).LengthSquared() > 0.5f) )
        {
           return;
        }
#endif // X_EDITOR

        void* pPtr1 = m_AnimGroup.GetPointer();
        void* pPtr2 = m_Skin.GetSkinGeom();

        // Don't render the player arms if he doesn't have a weapon
        // GaryW -> Commented out GetCurrentWeaponPtr() because it was
        // causing a bug where the player was unable to turn while standing
        // on an Anim Surface.  Bug was added to the BugBase to have the
        // appropriate person find a resultion to this problem.
        if(    pPtr1 
            && pPtr2 
            && (GetCurrentWeaponPtr() || (m_CurrentAnimState == ANIM_STATE_DEATH))
            && !(m_bIsMutated && (m_CurrentAnimState == ANIM_STATE_DEATH)) )
        {
            s32            nBones    = m_AnimPlayer.GetNBones();
            matrix4*       pBone     = (matrix4*)smem_BufferAlloc( nBones * sizeof( matrix4 ) );
            const matrix4* pAnimBone = m_AnimPlayer.GetBoneL2Ws();

            for( s32 i=0; i<nBones; i++ )
            {
                pBone[i] = pAnimBone[i];
                pBone[i].Translate( m_WeaponCollisionOffset );
            }

#if !defined( CONFIG_RETAIL )
            if( m_bRenderSkeleton )
            {
                m_AnimPlayer.RenderSkeleton( m_bRenderSkeletonNames );
            }
#endif // !defined( CONFIG_RETAIL )

            // Handle fade-in on spawn
            if( m_SpawnFadeTime > 0.0f )
            {
                Flags |= render::FADING_ALPHA;
                f32 Alpha = 1.0f - (m_SpawnFadeTime / g_SpawnFadeTime);
                Alpha = MIN( Alpha, 1.0f );
                Alpha = MAX( Alpha, 0.0f );
                Ambient.A  = (u8)(Alpha*255.0f);
            }

            skin_inst& SkinInst = m_Skin;
            SkinInst.Render( &GetL2W(), 
                            pBone, 
                            nBones, 
                            Flags | render::CLIPPED | render::DISABLE_SPOTLIGHT, 
                            SkinInst.GetLODMask(GetL2W()),
                            Ambient );
        }
        else
        if( !GetCurrentWeaponPtr() )
        {
            // KSS -- FIXME -- HACK -- This will cause no weapon on moving platforms to now work.
            // PREVIOUSLY, you would get locked in and were not able to YAW at all.
            const matrix4& mat = GetL2W();
            (void)mat;
        }

        if ( m_CurrentAnimState == ANIM_STATE_CHANGE_MUTATION && ( m_AnimStage > 1 ) && ( m_AnimStage < 3 ) ) //stage 1 is the switch from
        {
            //special case
            return;
        }

        //render weapon
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if ( pWeapon )
        {
            pWeapon->SetRenderState( new_weapon::RENDER_STATE_PLAYER );
            //AttachWeapon();
            pWeapon->RenderWeapon( TRUE, Ambient, FALSE );
        }
    }
    else
    {
        actor::OnRender();
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if( pWeapon )
            pWeapon->SetRenderState( new_weapon::RENDER_STATE_PLAYER );
    }

#ifdef MONKEY_DEBUG
    radian Pitch, Yaw;
    GetView().GetPitchYaw(Pitch, Yaw);
    // FIND NEAREST HOSTILE CHARACTEr

    // Collect living targets
    f32 ClosestDistSq   = 1500.f * 1500.f;
    const f32 MaxDistSq = ClosestDistSq;
    actor* pClosestHostile = NULL;

    actor* pNextActor = actor::m_pFirstActive;
    while( pNextActor )
    {
        // Get ptr to actor and advance to next
        actor* pActor = pNextActor;
        pNextActor = pNextActor->m_pNextActive;

        if( (pActor->GetGuid() != GetGuid()) && 
            pActor->IsKindOf( actor::GetRTTI() ) &&
            !pActor->IsDead() )
        {
            if( IsEnemyFaction( pActor->GetFaction() ) )
            {
                f32 DistSq = (pActor->GetPosition() - GetPosition()).LengthSquared();
                if ( DistSq < ClosestDistSq && DistSq < MaxDistSq )
                {
                    ClosestDistSq = DistSq;
                    pClosestHostile = pActor;
                }
            }
        }
    }

    // pass the monkey all the data it wants
    vector3 closest_hostile_position(0,0,0);
    if ( pClosestHostile )
    {
        closest_hostile_position = pClosestHostile->GetPosition();

        // TEMP:  log crap about angle to target            
        vector3 DirToHostile = closest_hostile_position - GetPosition();
        DirToHostile.Normalize();

        vector3 ForwardDir(Pitch, Yaw);
        ForwardDir.Normalize();

        vector3 ForwardDirNoY(ForwardDir);
        ForwardDirNoY.GetY() = 0.f;
        ForwardDirNoY.Normalize();

        f32 yaw_dot_to_hostile = ForwardDirNoY.Dot(DirToHostile);
        vector3 cross1         = ForwardDirNoY.Cross(DirToHostile);
        radian RotAngleYaw     = x_acos(yaw_dot_to_hostile) * (cross1.GetY() < 0.f ? 1.f : -1.f);

        vector3 DirToHostileNoY(DirToHostile);
        DirToHostileNoY.GetY() = ForwardDir.GetY();
        DirToHostileNoY.Normalize();

        f32 pitch_dot_to_hostile = DirToHostileNoY.Dot(DirToHostile);
        radian RotAnglePitch     = x_acos(pitch_dot_to_hostile) * (DirToHostileNoY.GetY() > DirToHostile.GetY() ? 1.0f : -1.f);

        x_printfxy( 0, 3, "yaw-RotAngle:  %2.2f", RAD_TO_DEG(RotAngleYaw));
        x_printfxy( 0, 4, "pit-RotAngle:  %2.2f", RAD_TO_DEG(RotAnglePitch));

        draw_Line( GetPosition(), closest_hostile_position, XCOLOR_RED );
    }
#endif
}

//===========================================================================

void player::OnRenderShadowCast( u64 ProjMask )
{
    if( IsAvatar() )
    {
        actor::OnRenderShadowCast( ProjMask );
    }
}

//===========================================================================

#if !defined(X_RETAIL) && !defined(X_EDITOR)

// used by monkey (and potentially other diagnostic tools) to give the player all weaponry

void player::AddAllWeaponsToInventory( void )
{
    AddItemToInventory2(INVEN_WEAPON_DESERT_EAGLE);    
    AddItemToInventory2(INVEN_WEAPON_SMP);    
    AddItemToInventory2(INVEN_WEAPON_SHOTGUN);    
    AddItemToInventory2(INVEN_WEAPON_SNIPER_RIFLE);
    AddItemToInventory2(INVEN_WEAPON_BBG);
    AddItemToInventory2(INVEN_WEAPON_MESON_CANNON);

    AddAmmoToInventory2(INVEN_AMMO_SMP, 150);
    AddAmmoToInventory2(INVEN_AMMO_SHOTGUN, 40);
    AddAmmoToInventory2(INVEN_AMMO_SNIPER_RIFLE, 10);
    AddAmmoToInventory2(INVEN_AMMO_MESON, 1);
    AddAmmoToInventory2(INVEN_AMMO_DESERT_EAGLE, 1);
    AddAmmoToInventory2(INVEN_GRENADE_FRAG, 3);
    AddAmmoToInventory2(INVEN_GRENADE_GRAV, 3);
    AddAmmoToInventory2(INVEN_GRENADE_JBEAN, 3);
    
    ReloadAllWeapons();
}

//===========================================================================

// monkey should add all weapons if in Gunman or Grenadier mode and not already in possession of all weapons

xbool player::ShouldMonkeyAddAllWeapons( void )
{
    if ( g_Monkey.GetCurrentMode() == MONKEY_GUNMAN || g_Monkey.GetCurrentMode() == MONKEY_GRENADIER )
    {
        if ( HasItemInInventory2(INVEN_WEAPON_DESERT_EAGLE) &&             
             HasItemInInventory2(INVEN_WEAPON_SMP) &&             
             HasItemInInventory2(INVEN_WEAPON_SHOTGUN) &&
             HasItemInInventory2(INVEN_WEAPON_SNIPER_RIFLE) &&
             HasItemInInventory2(INVEN_WEAPON_BBG) &&
             HasItemInInventory2(INVEN_WEAPON_MESON_CANNON) )
             return FALSE;

        return TRUE;
    }

    return FALSE;
}

#endif // monkey only defined in non-retail / non-editor builds

//===========================================================================

void player::OnAliveLogic( f32 DeltaTime )
{
    // Recover aim (double the recovery just to make it faster).
    m_AimDegradation = MAX( 0.0f, m_AimDegradation - (m_AimRecoverSpeed*DeltaTime*2.0f) );

    f32 AbsForwardSpeed = x_abs( m_fForwardSpeed );
    f32 AbsStrafeSpeed  = x_abs( m_fStrafeSpeed );

    f32 ScalerForward   = (AbsForwardSpeed/m_MaxFowardVelocity) * m_ReticleMovementDegrade;
    f32 ScalerStrafe    = (AbsStrafeSpeed /m_MaxStrafeVelocity) * m_ReticleMovementDegrade;

    f32 ShootDegrade    = 1.0f - m_ReticleMovementDegrade;
    f32 AimDegrade      = MIN( 1.0f, (m_AimDegradation*ShootDegrade) + MAX( ScalerStrafe, ScalerForward ) );

    if( AimDegrade < 0.0f )
        AimDegrade = 1.0f;

    f32 AlteredDeltaTime = DeltaTime;

    if( ( m_NonExclusiveStateBitFlag & NE_STATE_STUNNED ) != 0 )
    {
        AlteredDeltaTime *= 0.3f;
    }

#ifndef X_EDITOR
    if( GameMgr.IsZoneLocked( GetZone1() ) )
    {
        m_TimeSinceLastZonePain += DeltaTime;

        if( m_TimeSinceLastZonePain > 0.5f )
        {
            //Do Damage
            pain Pain;

            pain_handle PainHandle( "ZONE_PAIN" );
            Pain.Setup( PainHandle, 0, GetBBox().GetCenter() );

            Pain.SetCustomScalar( m_TimeSinceLastZonePain );

            Pain.ApplyToObject( this );

            m_TimeSinceLastZonePain = 0.0f;
        }
    }
#endif

    //==========================================================================================
    // Begin code from ghost.cpp
    //==========================================================================================
    
    // Let physics keep track of riding on platform
    m_Physics.CatchUpWithRidingPlatform( DeltaTime );
    m_Physics.WatchForRidingPlatform();

    UpdateMovement( DeltaTime );

    // Call base class
    actor::OnAliveLogic( DeltaTime );
    
    //==========================================================================================
    // End code from ghost.cpp
    //==========================================================================================

    // Handles rotation.
    UpdateRotation          ( AlteredDeltaTime );
    UpdateCharacterRotation ( AlteredDeltaTime );
    UpdateCrouchHeight      ( AlteredDeltaTime );

    //==========================================================================================
    // feed the monkey -- non-retail builds only
    //==========================================================================================
#if !defined(X_RETAIL) && !defined(X_EDITOR)
    if ( g_MonkeyOptions.Enabled )
    {           
        // add all weapons when entering certain monkey modes
        if ( ShouldMonkeyAddAllWeapons() )
        {
            AddAllWeaponsToInventory();
        }

        // check for out-of-world monkey
        if ( g_MonkeyOptions.bTestOutOfWorld ) 
        {
            if ( g_ZoneMgr.FindZone( GetPosition() ) == 0 )
            {
                ASSERTS( 0, "Monkey has fallen out of the world!" );
            }
        }

        // get view pitch and yaw
        radian Pitch, Yaw;
        GetView().GetPitchYaw(Pitch, Yaw);

        // get player profile to pass in bool representing invert-y setting
        player_profile& p = g_StateMgr.GetActiveProfile(g_StateMgr.GetProfileListIndex(m_LocalSlot));

        // FIND NEAREST HOSTILE CHARACTEr
        vector3 closest_hostile_position(0,0,0);
        
        // Collect living targets
        f32 ClosestDistSq   = 1500.f * 1500.f;
        const f32 MaxDistSq = ClosestDistSq;
        actor* pClosestHostile = NULL;

        actor* pNextActor = actor::m_pFirstActive;
        while( pNextActor )
        {
            // Get ptr to actor and advance to next
            actor* pActor = pNextActor;
            pNextActor = pNextActor->m_pNextActive;

            if( (pActor->GetGuid() != GetGuid()) && 
                pActor->IsKindOf( actor::GetRTTI() ) &&
                !pActor->IsDead() )
            {
                if( IsEnemyFaction( pActor->GetFaction() ) )
                {
                    f32 DistSq = (pActor->GetPosition() - GetPosition()).LengthSquared();
                    if ( DistSq < ClosestDistSq && DistSq < MaxDistSq )
                    {
                        ClosestDistSq = DistSq;
                        pClosestHostile = pActor;
                    }
                }
            }
        }

        // pass the monkey all the data it wants        
        if ( pClosestHostile )
            closest_hostile_position = pClosestHostile->GetPosition();

        g_Monkey.SetPlayerInfo( GetPosition(), Pitch, Yaw, p.m_bInvertY, closest_hostile_position, IsMutated() );
    }  
#endif 
}

//===========================================================================

void player::OnDeathLogic( f32 DeltaTime )
{
    //==========================================================================================
    // Begin code from ghost.cpp
    //==========================================================================================

    // Keep physics going for death while falling.
    m_Physics.Advance( m_Physics.GetPosition(), DeltaTime, TRUE );

    OnMove( m_Physics.GetPosition() );

    if ( UsingLoco() )
    {
        if( m_pLoco && m_pLoco->IsPlayAnimComplete() )
        {
            CreateCorpse();
        }
    }

    // Call base class
    actor::OnDeathLogic( DeltaTime );
    
    //==========================================================================================
    // End code from ghost.cpp
    //==========================================================================================
}

//===========================================================================
void player::OnMove( const vector3& rNewPos )
{
    CONTEXT( "player::OnMove" );

    OnMoveViewPosition( rNewPos );

    //==========================================================================================
    // Begin code from ghost.cpp
    //==========================================================================================
    
    if( GetAttrBits() & object::ATTR_DESTROY )
        return;

    m_DeltaPos = rNewPos - GetPosition();

    // HACKOMOTRON - 
    //
    // Problem: When the ghost spawns or is created, he is created essentially 
    // at the origin and then moved to his starting point.  The above 
    // computation for m_DeltaPos results in a large vector which is, in turn,
    // used by the physics the first time the ghost runs his logic.  The poor
    // ghost then proceeds to collide with several walls.
    //
    // Proper solution:
    // (1) The ghost should not run physics or collision.
    // (2) Upon creation or spawning, the ghost should not attempt to travel
    //     long distances in the first frame following.
    //
    // HACK: If m_DeltaPos is too large, set it to 0.

    if( m_DeltaPos.LengthSquared() > (500*500) )  // 5 meters squared
        m_DeltaPos.Zero();

    actor::OnMove( rNewPos );

    m_Physics.SetPosition( rNewPos );

    //==========================================================================================
    // End code from ghost.cpp
    //==========================================================================================
}

//===========================================================================
void player::OnMoveViewPosition( const vector3& rNewPos )
{
    (void)rNewPos;
    vector3 EyesOffSet = m_EyesOffset;

    EyesOffSet.RotateY( m_Yaw );

    vector3 vViewPosition;
    f32 Height = GetBBox().Max.GetY();

    vViewPosition.Set( rNewPos.GetX(), Height, rNewPos.GetZ() );
    vViewPosition += EyesOffSet;

    MoveAnimPlayer( vViewPosition );
}

//=========================================================================

void player::OnTransform( const matrix4& L2W )
{
    //==========================================================================================
    // Begin code from ghost.cpp
    //==========================================================================================

    actor::OnTransform(L2W);

    //update physics
    m_Physics.SetPosition( L2W.GetTranslation() );

    //==========================================================================================
    // End code from ghost.cpp
    //==========================================================================================

    m_Yaw = L2W.GetRotation().Yaw;
    m_AnimPlayer.SetYaw( m_fCurrentYawOffset + m_Yaw );
    
    OnMoveViewPosition( L2W.GetTranslation() );
    MoveAnimPlayer( m_EyesPosition );

    //set tracker info for portal/zone
    UpdateZoneTrack();
}

//=========================================================================
static f32 s_ArmsDampen = 0.2f;
static f32 s_ArmReturnForceMultiplier = 75.0f;

// TODO: Debug Code
//#ifdef cgalley
#if 0
extern xbool  g_LogCharacterPhysics;
#endif

void player::UpdateMovement( f32 DeltaTime )
{
// TODO: Debug Code
//#ifdef cgalley
#if 0
    g_LogCharacterPhysics = TRUE;
#endif

    // Skip if in a cinema
    if (m_ViewCinematicPlaying)
        return ;

    // Try ladder movement first
    if (UpdateLadderMovement(DeltaTime))
        return ;

    vector3 ViewX( 1,0,0 );
    vector3 ViewZ( 0,0,1 );
    
    ViewX.RotateY( m_Yaw );
    ViewZ.RotateY( m_Yaw );

    CalculateRigOffset( DeltaTime );
    
    // Compute normal movement
    CalculateStrafeVelocity ( ViewX , DeltaTime );
    CalculateForwardVelocity( ViewZ , DeltaTime );
    
    vector3 FinalVel = m_StrafeVelocity + m_ForwardVelocity;

    //if we're crouching, move a little slower
    if ( m_bIsCrouching )
    {
        FinalVel *= .45f;
    }

    m_DeltaPos = FinalVel;

    if ( !m_ViewCinematicPlaying && !m_bInTurret )
    {
        //==========================================================================================
        // Begin code from ghost.cpp
        //==========================================================================================
    
        m_Physics.Advance( 
            m_Physics.GetPosition() + m_DeltaPos, 
            DeltaTime );
        
        //==========================================================================================
        // End code from ghost.cpp
        //==========================================================================================
    }
    
    //
    // Update the arms velocity and position
    //

    ASSERT( m_ArmsOffset.IsValid() );
    ASSERT( m_ArmsVelocity.IsValid() );

    // return force...
    vector3 ReturnForce( -m_ArmsOffset );
    ReturnForce *= s_ArmReturnForceMultiplier;
    m_ArmsVelocity += ReturnForce    * DeltaTime;

    ASSERT( m_ArmsOffset.IsValid() );
    ASSERT( m_ArmsVelocity.IsValid() );

    // dampen with drag
    m_ArmsVelocity -= m_ArmsVelocity * s_ArmsDampen;
    m_ArmsOffset   += m_ArmsVelocity * DeltaTime;

    ASSERT( m_ArmsOffset.IsValid() );
    ASSERT( m_ArmsVelocity.IsValid() );

    UpdateArmsOffsetForLean();

    OnMove( m_Physics.GetPosition() );

// TODO: Debug Code
//#ifdef cgalley
#if 0
    g_LogCharacterPhysics = FALSE;
#endif
}

//===========================================================================

void player::UpdateAudio( f32 DeltaTime )
{
    if( GetLocalSlot() == -1 )
        return;

    // Update the ear.
    view& View = GetView();
    ComputeView( View );
    g_AudioMgr.SetEar( m_AudioEarID, View.GetW2V(), GetPosition(), GetZone1(), 1.0f );

    if( DoFootfallCollisions() )
        PlayFootfall( DeltaTime );
    ProcessSfxEvents();    
}

//==============================================================================

void player::UpdateCharacterRotation( const f32& DeltaTime )
{
    CalculateLookHorozOffset( DeltaTime );
    CalculateLookVertOffset( DeltaTime );


    //set the pitch and yaw of the rig.
    m_AnimPlayer.SetYaw( m_Yaw + m_CurrentHorozRigOffset - m_ShakeYaw );
    m_AnimPlayer.SetPitch( -m_Pitch - m_CurrentVertRigOffset - m_ShakePitch );
    m_AnimPlayer.SetRoll( m_SoftLeanAmount * DEG_TO_RAD( GetTweakF32( "LeanMaxDegrees" ) ) );
}

//==============================================================================

void player::UpdateCrouchHeight( const f32& rDeltaTime )
{
    if ( m_bIsCrouching  )
    {
        //trying to crouch
        f32 NewCrouchFactor = MIN( 1.f ,  m_fCurrentCrouchFactor + m_fCrouchChangeRate * rDeltaTime );
        if ( m_Physics.SetCrouchParametric( NewCrouchFactor ) )
        {
            m_fCurrentCrouchFactor = NewCrouchFactor;
        }
    }
    else
    if ( m_fCurrentCrouchFactor > 0.f )
    {
        f32 NewCrouchFactor = MAX( 0.f ,  m_fCurrentCrouchFactor - m_fCrouchChangeRate * rDeltaTime );
        if ( m_Physics.SetCrouchParametric( NewCrouchFactor ) )
        {
            m_fCurrentCrouchFactor = NewCrouchFactor;
        }
    }

#ifdef nmreed
    x_printfxy( 0, 10, "Crouch: %5.2f", m_fCurrentCrouchFactor );
    x_printfxy( 0, 11, "Coll Height: %5.2f", GetCollisionHeight() );
    x_printfxy( 0, 12, "Player Y: %5.2f", m_Physics.GetPosition().GetY() );
    x_printfxy( 0, 13, "Bone Y: %5.2f", __AnimPlayer.GetBonePosition( m_iCameraBone ).GetY() );
    x_printfxy( 0, 14, "AnimPlayer Y: %5.2f", m_AnimPlayer.GetPosition().GetY() );
#endif
}

//===========================================================================

void player::OnEnumProp( prop_enum&  rList )
{
    actor::OnEnumProp       ( rList );
    
    //
    // Player info
    //
    rList.PropEnumHeader     ( "Player", "Player/Mutation information", PROP_TYPE_HEADER );

    rList.PropEnumAngle      ( "Player\\CamFOV",       "This is the Field of View in degrees.", PROP_TYPE_EXPOSE );
    rList.PropEnumRotation   ( "Player\\View Rotation","This rotation sets up the player view on startup. Use the cyan cone as your pointer", 0 );

    rList.PropEnumInt        ( "Player\\LoreDiscoveries", "", PROP_TYPE_EXPOSE | PROP_TYPE_DONT_SHOW );
    rList.PropEnumBool       ( "Player\\RenderSkeleton",  "Renders the skeleton of the player. This is use for debugging.", PROP_TYPE_DONT_SAVE_MEMCARD );
    rList.PropEnumBool       ( "Player\\RenderBoneNames", "When the Skeleton is render whether you want to render the name of the bones as well", PROP_TYPE_DONT_SAVE_MEMCARD );
    rList.PropEnumBool       ( "Player\\RenderBBox",      "This allows to turn off and on the BBox of the player.", PROP_TYPE_DONT_SAVE_MEMCARD );

    rList.PropEnumFloat      ( "Player\\ArmPitchModifier+1", "This is a scaler value use to multiply the camera pitch (when>0)so that the arms don't fallow exactly the camera.", PROP_TYPE_DONT_SAVE_MEMCARD );
    rList.PropEnumFloat      ( "Player\\ArmPitchModifier-1", "This is a scaler value use to multiply the camera pitch (when<0)so that the arms don't fallow exactly the camera.", PROP_TYPE_DONT_SAVE_MEMCARD );

    rList.PropEnumBool       ( "Player\\Can Die" , "Determines if the player can die.", PROP_TYPE_DONT_SAVE_MEMCARD | PROP_TYPE_EXPOSE );
    rList.PropEnumBool       ( "Player\\Can Jump" , "Determines if the player can jump.", PROP_TYPE_DONT_SAVE_MEMCARD | PROP_TYPE_EXPOSE );
    rList.PropEnumBool       ( "Player\\Hide Player Arms" , "Do we need to hide the player arms for a special event?.", PROP_TYPE_DONT_SAVE_MEMCARD | PROP_TYPE_EXPOSE );
    rList.PropEnumBool       ( "Player\\Play SwitchTo", "If this is true, this will play the SwitchTo animation after arms re-appear from Hide Player Arms(default is TRUE).", PROP_TYPE_DONT_SAVE_MEMCARD | PROP_TYPE_EXPOSE );
    rList.PropEnumBool       ( "Player\\Using Flashlight", "Indicates if the player is using the flashlight", PROP_TYPE_EXPOSE | PROP_TYPE_DONT_SHOW | PROP_TYPE_DONT_SAVE | PROP_TYPE_READ_ONLY );
    rList.PropEnumFloat      ( "Player\\Melee Damage", "This is the damage dished out in one direct melee hit -- not mutation melee", 0 );
    rList.PropEnumFloat      ( "Player\\Melee Force", "This is the force dished out in one direct melee hit -- not mutation melee", 0 );
    rList.PropEnumFloat      ( "Player\\Health", "Player's Health (1-100, 100 = Full Health)", PROP_TYPE_EXPOSE );
    rList.PropEnumFloat      ( "Player\\Mutagen", "Player's Mutagen level (0-100, 100 = Full Mutagen)", PROP_TYPE_EXPOSE );    
    rList.PropEnumBool      ( "Player\\In Mutation Tutorial", "TRUE if we are the mutation tutorial is running, changing mutagen behavior", PROP_TYPE_EXPOSE );
    // Third Person Geometry
    m_SkinInst.OnEnumProp       ( rList );
    rList.PropEnumExternal           ( "RenderInst\\Anim", "Resource\0anim\0", "Resource File", PROP_TYPE_MUST_ENUM | PROP_TYPE_DONT_SAVE_MEMCARD );

    // Skins and animations
    // Enumerate the different strains.
    rList.PropEnumHeader( "Player\\Human Strain", "Properties of the human strain", 0 );
    rList.PropEnumHeader( "Player\\Strain One", "Properties of strain one.", 0 );
    rList.PropEnumHeader( "Player\\Strain Two", "Properties of strain two.", 0 );
    rList.PropEnumHeader( "Player\\Strain Three", "Properties of strain three.", 0 );

//    rList.PropEnumEnum (  "Player\\Current Strain", GetStrainEnum(), "Current strain.", PROP_TYPE_EXPOSE  );

    rList.PropEnumBool (  "Player\\Holster Weapon", "When TRUE, player weapon is hidden, and the user can't cycle or fire", PROP_TYPE_EXPOSE );
    
    // Enumerate the human specific properties
    s32 PathIndex = rList.PushPath( "Player\\Human Strain\\" );
    m_Physics.OnEnumProp( rList );
    m_Skin.OnEnumProp( rList );
    rList.PropEnumExternal( "RenderInst\\AnimFile", "Resource\0anim", "Resource Animation File", PROP_TYPE_MUST_ENUM | PROP_TYPE_DONT_SAVE_MEMCARD );
    rList.PropEnumExternal( "Audio Package", "Resource\0audiopkg","The audio package associated with human strain.", PROP_TYPE_DONT_SAVE_MEMCARD );
    EnumrateStrainControls( rList );
    rList.PropEnumHeader("Friendly Factions", "Reticle doesn't highlight on friends", 0 );
    s32 ID = rList.PushPath( "Friendly Factions\\" );
    factions_manager::OnEnumFriends( rList );
    rList.PopPath( ID );
    rList.PopPath( PathIndex );

    //  Mutation properties
    rList.PropEnumHeader( "Player\\Mutation", "Properties for mutation behavior", 0 );
    rList.PropEnumBool( "Player\\Mutation\\Is Mutated", "READ ONLY: Indicates if the player is mutated or not.", PROP_TYPE_READ_ONLY | PROP_TYPE_EXPOSE );
    rList.PropEnumBool( "Player\\Mutation\\Can Melee", "TRUE if the player can use the mutant melee attack.", PROP_TYPE_EXPOSE );
    rList.PropEnumBool( "Player\\Mutation\\Can Fire Primary Ammo", "TRUE if the player can use primary mutation ammo.", PROP_TYPE_EXPOSE  );
    rList.PropEnumBool( "Player\\Mutation\\Can Fire Secondary Ammo", "TRUE if the player can use secondary mutation ammo.", PROP_TYPE_EXPOSE  );
    rList.PropEnumBool( "Player\\Mutation\\User Can Toggle Mutation", "TRUE if the user can control mutation through the controller.", PROP_TYPE_EXPOSE );
    rList.PropEnumBool( "Player\\Mutation\\Force To Mutant", "Set to TRUE to force the player to mutate, assuming he has the mutation weapon.", PROP_TYPE_EXPOSE );
    rList.PropEnumBool( "Player\\Mutation\\Force To Human", "Set to TRUE to force the player to become human, assuming he has the mutation weapon.", PROP_TYPE_EXPOSE  );
    rList.PropEnumFloat("Player\\Mutation\\Convulsion Feedback Duration", "Duration, in seconds, of the controller feedback for mutation convulsions.", PROP_TYPE_EXPOSE );
    rList.PropEnumFloat("Player\\Mutation\\Convulsion Feedback Intensity", "Intensity, from 0.0 to 1.0, of the controller feedback for mutation convulsions. Full normal force is 1.0.", PROP_TYPE_EXPOSE );


    rList.PropEnumHeader( "Player\\Cinema", "Properties for cinemas", 0 );
    rList.PropEnumBool  ( "Player\\Cinema\\CinemaOn", "Turns cinema mode and off", PROP_TYPE_EXPOSE | PROP_TYPE_DONT_SAVE );
    rList.PropEnumGuid  ( "Player\\Cinema\\CinemaCameraGuid", "This points to the camera that should be the player's view", PROP_TYPE_EXPOSE | PROP_TYPE_DONT_SAVE );     
    rList.PropEnumBool  ( "Player\\Cinema\\UseViewCorrection", "Do we correct the view at the end of cinema. Don't use this if you are popping player to position", PROP_TYPE_EXPOSE | PROP_TYPE_DONT_SAVE );
    rList.PropEnumGuid  ( "Player\\Cinema\\LookAtTarget", "Object to focus camera on", PROP_TYPE_EXPOSE | PROP_TYPE_DONT_SAVE );
    rList.PropEnumFloat ( "Player\\Cinema\\BlendInTime", "How long to blend to desired view", PROP_TYPE_EXPOSE | PROP_TYPE_DONT_SAVE );
}

//===========================================================================

void player::EnumrateStrainControls( prop_enum& List )
{
    List.PropEnumFloat   ( "MaxHealth",    "Maximum health value", 0 );
    List.PropEnumFloat   ( "Proximity Alert Radius", "Radius of Proximity Broadcast which will alert NPC's to player presence.", 0 );

    List.PropEnumHeader  ( "Controls" , "Variables that effect the player's control" , PROP_TYPE_HEADER );
    List.PropEnumFloat   ( "Controls\\MaxFowardVel", "How fast does the player move foward at this mutation level", 0 );
    List.PropEnumFloat   ( "Controls\\MaxStrafeVel", "How fast does the player strafe at this mutation level", 0 );
    List.PropEnumFloat   ( "Controls\\FowardAccel", "Players foward acceleration at this mutation level", 0 );
    List.PropEnumFloat   ( "Controls\\StrafeAccel", "Players strafe acceleration at this mutation level", 0 );
    List.PropEnumFloat   ( "Controls\\Pitch Accel Time", "How long it takes to get to full pitch change speed.", 0 );
    List.PropEnumFloat   ( "Controls\\Yaw Accel Time", "How long it takes to get to full pitch change speed.", 0 );
    List.PropEnumFloat   ( "Controls\\JumpVelocity", "How fast does the player jump. This is the initial velocity for the jump.", 0 );
    List.PropEnumVector3 ( "Controls\\EyesOffSet",   "This is where the eyes are relative to the top of his head", 0 );
    List.PropEnumFloat   ( "Controls\\Pitch Stick Sensitivity" , "How sensitive is the right thumb stick pitch.  Expects a number above zero.", 0 );
    List.PropEnumFloat   ( "Controls\\Yaw Stick Sensitivity" , "How sensitive is the right thumb stick pitch.  Expects a number above zero.", 0 );
    List.PropEnumFloat   ( "Controls\\MinWalkSpeed" , "Minimum speed at which the player walks.", 0 );
    List.PropEnumFloat   ( "Controls\\MinRunSpeed" , "Minimum speed at which the player runs.", 0 );
    List.PropEnumFloat   ( "Controls\\Deceleration Multiplier" , "This is how many times faster the player slows down than speeds up.", 0 );
    List.PropEnumFloat   ( "Controls\\Crouch Change Rate" , "This is how fast the player crouches.  10 is a good place to start tweaking.  Lower is slower, higher is faster", 0 );
    List.PropEnumFloat   ( "Controls\\Movement Aim Degradation", "How much aim you are going to lose by movement 0 -> 1", 0 );

    List.PropEnumHeader  ( "Controls\\Stun Properties", "Effects the way this guy is stunned", 0 );
    List.PropEnumAngle   ( "Controls\\Stun Properties\\MaxStunPitchOffset",   "How far does the pitch go?", 0 );
    List.PropEnumAngle   ( "Controls\\Stun Properties\\MaxStunYawOffset",     "How far does the yaw go?", 0 );
    List.PropEnumAngle   ( "Controls\\Stun Properties\\MaxStunRollOffset",    "How far does the roll go?", 0 );
    List.PropEnumFloat   ( "Controls\\Stun Properties\\StunYawChangeSpeed",   "How fast does the yaw change", 0 );
    List.PropEnumFloat   ( "Controls\\Stun Properties\\StunPitchChangeSpeed", "How fast does the yaw change", 0 );
    List.PropEnumFloat   ( "Controls\\Stun Properties\\StunRollChangeSpeed",  "How fast does the yaw change", 0 );
    List.PropEnumFloat   ( "Controls\\Stun Properties\\Stun Time",  "How long does he stay stunned?", 0 );
}

//===========================================================================

xbool player::OnStrainControlProperty( prop_query& I )
{
    if( I.VarFloat( "Proximity Alert Radius", m_StrainControls.m_StrainProximityAlertRadius ) )
    {
        return TRUE;
    }

    if( I.VarFloat( "Controls\\MaxFowardVel", m_StrainControls.m_StrainMaxFowardVelocity ) )
    {
        return TRUE;
    }
    
    if( I.VarFloat( "Controls\\MaxStrafeVel", m_StrainControls.m_StrainMaxStrafeVelocity ) )
    {
        return TRUE;
    }
    
    if( I.VarFloat( "Controls\\JumpVelocity", m_StrainControls.m_StrainJumpVelocity ) )
    {
        return TRUE;
    }
    
    if( I.VarVector3( "Controls\\EyesOffSet", m_StrainControls.m_StrainEyesOffset ) )
    {
        return TRUE;
    }
    

    if( I.VarFloat( "MaxHealth", m_StrainControls.m_StrainMaxHealth ) )
    {
        return TRUE;
    }

    if( I.VarFloat( "Controls\\FowardAccel" , m_StrainControls.m_fStrainForwardAccel) )
    {
        return TRUE;
    }

    if( I.VarFloat( "Controls\\StrafeAccel" , m_StrainControls.m_fStrainStrafeAccel) )
    {
        return TRUE;
    }


    if( I.VarFloat( "Controls\\Pitch Stick Sensitivity" , m_StrainControls.m_fStrainPitchSensitivity ) )
    {
        return TRUE;
    }

    if( I.VarFloat( "Controls\\Yaw Stick Sensitivity" , m_StrainControls.m_fStrainYawSensitivity ) )
    {
        return TRUE;
    }

    if( I.VarFloat( "Controls\\Yaw Accel Time" , m_StrainControls.m_StrainYawAccelTime ) )
    {
        return TRUE;
    }

    if( I.VarFloat( "Controls\\Pitch Accel Time" , m_StrainControls.m_StrainPitchAccelTime ) )
    {
        return TRUE;
    }

    
    if( I.VarFloat( "Controls\\MinWalkSpeed" , m_StrainControls.m_StrainMinWalkSpeed ) )
    {

        return TRUE;
    }

    if( I.VarFloat  ( "Controls\\MinRunSpeed" , m_StrainControls.m_StrainMinRunSpeed ) )
    {

        return TRUE;
    }

    if( I.VarFloat( "Controls\\Deceleration Multiplier" , m_StrainControls.m_StrainDecelerationFactor ) )
    {
        return TRUE;
    }

    if( I.VarFloat( "Controls\\Crouch Change Rate" , m_StrainControls.m_StrainCrouchChangeRate ) )
    {
        return TRUE;
    }

    if( I.VarFloat( "Controls\\Movement Aim Degradation" , m_StrainControls.m_StrainReticleMovementDegrade, 0.0f, 1.0f ) )
    {
        return TRUE;
    }

    if ( I.VarAngle( "Controls\\Stun Properties\\MaxStunPitchOffset", m_MaxStunPitchOffset ) )
    {
        return TRUE;
    }
    
    if ( I.VarAngle( "Controls\\Stun Properties\\MaxStunYawOffset", m_MaxStunYawOffset ) )
    {
        return TRUE;
    }

    if ( I.VarAngle( "Controls\\Stun Properties\\MaxStunRollOffset", m_MaxStunRollOffset ) )
    {
        return TRUE;
    }
   
    if ( I.VarFloat( "Controls\\Stun Properties\\StunYawChangeSpeed", m_fStunYawChangeSpeed ) )
    {
        return TRUE;
    }

    if ( I.VarFloat( "Controls\\Stun Properties\\StunPitchChangeSpeed", m_fStunPitchChangeSpeed ) )
    {
        return TRUE;
    }

    if ( I.VarFloat( "Controls\\Stun Properties\\StunRollChangeSpeed", m_fStunRollChangeSpeed ) )
    {
        return TRUE;
    }
    
    if ( I.VarFloat( "Controls\\Stun Properties\\Stun Time", m_fMaxStunTime ) )
    {
        return TRUE;
    }
    
    return FALSE;    
}

//===========================================================================

xbool player::OnProperty( prop_query& rPropQuery )
{
    if( rPropQuery.VarBool( "Player\\Can Die", m_bCanDie ) )
    {
        return TRUE;
    }

    if( rPropQuery.VarBool( "Player\\Can Jump", m_bCanJump ) )
    {
        return TRUE;
    }

    if( rPropQuery.VarBool( "Player\\Hide Player Arms", m_bHidePlayerArms ) )
    {
        return TRUE;
    }

    if( rPropQuery.VarBool( "Player\\Play SwitchTo", m_bPlaySwitchTo ) )
    {
        return TRUE;
    }    

    if ( rPropQuery.VarBool( "Player\\Using Flashlight", m_bUsingFlashlight ) )
    {
        return TRUE;
    }

    if ( rPropQuery.IsVar(  "Player\\Cinema\\CinemaOn" ) )
    {
        if ( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarBool( m_Cinema.m_bCinemaOn );
        }
        else
        {
            m_Cinema.m_bCinemaOn = rPropQuery.GetVarBool();

            // Make sure zone flag is cleared ready for cinema
            m_Cinema.m_bPlayerZoneInitialized = FALSE;

            if ( m_bIsMutated )
            {
                // make sure our mutant vision is on, since the cinema will interrupt any
                // animations, and this will prevent the mutant vision event from firing
#ifdef mreed 
                LOG_MESSAGE( "Mutation", "m_bIsMutantVisionOn = TRUE (CinemaOn property)" );
#endif
                m_bIsMutantVisionOn = TRUE;

                if ( m_Cinema.m_bCinemaOn )
                {
                    // we need to turn off the mutant perception stuff.
                    g_PerceptionMgr.EndMutate();
                }
                else
                {
                    // we need to turn mutant perception stuff back on.
                    g_PerceptionMgr.BeginMutate();

                    // Force switch to mutation weapon?
                    new_weapon* pWeapon = GetCurrentWeaponPtr();
                    if( ( !pWeapon ) || ( !pWeapon->IsKindOf( weapon_mutation::GetRTTI() ) ) )
                    {
                        m_NextWeaponItem = INVEN_WEAPON_MUTATION;
                        ForceNextWeapon();
                    }

                    // Make sure the weapon mesh is set properly
                    pWeapon = GetCurrentWeaponPtr();
                    ASSERT( pWeapon && pWeapon->IsKindOf( weapon_mutation::GetRTTI() ) );
                    render_inst* pInst = pWeapon->GetRenderInstPtr();
                    ASSERT( pInst );
                    pInst->SetVMeshMask( 0xffffffff );
                }
            }
            else
            {
                // make sure our mutant vision is OFF, since the cinema will interrupt any
                // animations, and this will prevent the mutant vision event from firing
#ifdef mreed 
                LOG_MESSAGE( "Mutation", "m_bIsMutantVisionOn = FALSE (CinemaOn property)" );
#endif
                m_bIsMutantVisionOn = FALSE;

                if ( !m_Cinema.m_bCinemaOn )
                {
                    // Make sure the weapon mesh is set properly
                    new_weapon* pWeapon = GetCurrentWeaponPtr();
                    if( pWeapon && pWeapon->IsKindOf( weapon_mutation::GetRTTI() ) )
                    {
                        // Turn off mutant hands
                        render_inst* pInst = pWeapon->GetRenderInstPtr();
                        ASSERT( pInst );
                        pInst->SetVMeshMask( 0 );
                        
                        // Force switch to previous weapon?
                        m_NextWeaponItem = m_PreMutationWeapon2;
                        ForceNextWeapon();
                    }
                }
            }

            if ( m_bIsCrouching )
            {
                SetIsCrouching( FALSE );
            }
        }

        if ( m_Cinema.m_bCinemaOn )
        {
            // Stop leaning
            m_LeanAmount     = 0.0f;
            m_SoftLeanAmount = 0.0f;
        }
        return TRUE;
    }

    if( rPropQuery.VarGUID( "Player\\Cinema\\CinemaCameraGuid", m_Cinema.m_CinemaCameraGuid ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarGUID( "Player\\Cinema\\LookAtTarget", m_Cinema.m_LookAtTargetGuid ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarBool( "Player\\Cinema\\UseViewCorrection", m_Cinema.m_bUseViewCorrection ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarFloat( "Player\\Cinema\\BlendInTime", m_Cinema.m_BlendInTime ) )
    {
        return TRUE;
    }

    if( rPropQuery.IsVar( "Player\\Health" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarFloat( m_Health.GetHealth() );
        }
        else
        {
            // Get new health value
            f32 NewHealth = rPropQuery.GetVarFloat();
            if( NewHealth < 1.0f )
            {
                NewHealth = 1.0f;
            }

            // Now add the difference between the new health and current health
            f32 DeltaHealth = NewHealth - m_Health.GetHealth();
            if( DeltaHealth != 0.0f )
            {
                m_Health.Add( DeltaHealth, FALSE );
            }
        }

        return TRUE;
    }

    if ( rPropQuery.VarFloat( "Player\\Mutagen", m_Mutagen, 0, 100.0f ) )
    {
        return TRUE;
    }
    
    const xbool WasInTutorial = m_bInMutationTutorial;
    if ( rPropQuery.VarBool( "Player\\In Mutation Tutorial", m_bInMutationTutorial ) )
    {
        if ( WasInTutorial != m_bInMutationTutorial )
        {
            if ( m_bInMutationTutorial )
            {
                // Start the convulsion system
                m_ConvulsionInfo.m_bConvulsingNow           = FALSE;
                m_ConvulsionInfo.m_TimeSinceLastConvulsion  = 0.0f;
                m_ConvulsionInfo.m_ConvulseAtTime           = m_Mutagen * g_ConvulsionTweaks.m_MutagenConvulsionMultiplierPeriod;
            }
            else
            {
                // No more convulsions
                m_ConvulsionInfo.m_bConvulsingNow           = FALSE;
            }
        }
        return TRUE;
    }

    if( object::OnProperty( rPropQuery ) )
    {
        if( rPropQuery.IsVar( "Base\\Position" ) )
        {
            if( rPropQuery.IsRead() )
            {
                m_PositionOfLastSafeSpot = GetPosition();
                m_NextPositionOfLastSafeSpot = m_PositionOfLastSafeSpot ;
                m_RespawnPosition =  GetPosition();
            }
            else
            {
                m_PositionOfLastSafeSpot = rPropQuery.GetVarVector3();
                m_NextPositionOfLastSafeSpot = m_PositionOfLastSafeSpot ;
                m_RespawnPosition = rPropQuery.GetVarVector3();
            }
        }
        else if ( rPropQuery.IsVar( "Base\\ZoneInfo" )  && !rPropQuery.IsRead())
        {
            m_NextZoneIDOfLastSafeSpot = (u8)GetZone1();
            m_ZoneIDOfLastSafeSpot = (u8)GetZone1();
            m_RespawnZone = (u8)GetZone1();
        }
        return TRUE;
    }


    if( rPropQuery.VarString( "Player", m_pPlayerTitle, 256 ) )
    {
        // You can only read this guy
        ASSERT( rPropQuery.IsRead() == TRUE );
        return TRUE;
    }

    if( rPropQuery.VarInt(   "Player\\LoreDiscoveries", m_nLoreDiscoveries) )
    {
        return TRUE;
    }
    
#if !defined( CONFIG_RETAIL )
    if( rPropQuery.VarBool( "Player\\RenderSkeleton", m_bRenderSkeleton ) )
    {
        return TRUE;
    }

    if( rPropQuery.VarBool( "Player\\RenderBoneNames", m_bRenderSkeletonNames ) )
    {
        return TRUE;
    }

    if( rPropQuery.VarBool( "Player\\RenderBBox", m_bRenderBBox ) )
    {
        return TRUE;
    } 
#endif // !defined( CONFIG_RETAIL )

    if( rPropQuery.VarFloat( "Player\\ArmPitchModifier+1", m_PitchArmsScalerPositive ) )
    {
        return TRUE;
    } 

    if( rPropQuery.VarFloat( "Player\\ArmPitchModifier-1", m_PitchArmsScalerNegative ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarFloat( "Player\\Melee Damage", m_MeleeDamage ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarFloat( "Player\\Melee Force", m_MeleeForce) )
    {
        return TRUE;
    }


    if( rPropQuery.IsVar( "Player\\CamFOV" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarAngle( m_ViewInfo.XFOV );
        }
        else
        {
            m_ViewInfo.XFOV = rPropQuery.GetVarAngle();
            m_OriginalViewInfo.XFOV = m_ViewInfo.XFOV;
        }
        return TRUE;
    }

    radian3 Rot( m_Pitch, m_Yaw, 0.0f );
    if ( rPropQuery.VarRotation( "Player\\View Rotation", Rot ) )
    {
        m_Pitch     = Rot.Pitch;
        m_Yaw       = Rot.Yaw;
        return TRUE;
    }

    xbool bIsMutated = m_bIsMutated;
    if ( rPropQuery.VarBool( "Player\\Mutation\\Is Mutated", bIsMutated ) )
    {
        m_bIsMutated = bIsMutated;
        return TRUE;
    }

    if ( rPropQuery.VarBool( "Player\\Mutation\\Can Melee", m_bMutationMeleeEnabled ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarBool( "Player\\Mutation\\Can Fire Primary Ammo", m_bPrimaryMutationFireEnabled ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarBool( "Player\\Mutation\\Can Fire Secondary Ammo", m_bSecondaryMutationFireEnabled ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarFloat( "Player\\Mutation\\Convulsion Feedback Duration", m_ConvulsionFeedbackDuration ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarFloat( "Player\\Mutation\\Convulsion Feedback Intensity", m_ConvulsionFeedbackIntensity ) )
    {
        return TRUE;
    }

    xbool Temp = m_bCanToggleMutation;
    if ( rPropQuery.VarBool( "Player\\Mutation\\User Can Toggle Mutation", Temp ) )
    {
        m_bCanToggleMutation = Temp;
        return TRUE;
    }

    xbool TempForce = FALSE;
    if ( rPropQuery.VarBool( "Player\\Mutation\\Force To Mutant", TempForce ) )
    {
        // if we're not mutated, get there
        if ( TempForce && m_Inventory2.HasItem( INVEN_WEAPON_MUTATION ) && !IsMutated() )
        {
            ForceMutationChange( TRUE );
        }

        return TRUE;
    }

    if ( rPropQuery.VarBool( "Player\\Mutation\\Force To Human", TempForce ) )
    {
        // if we're mutant, go human
        if ( TempForce && IsMutated() )
        {
            SetupMutationChange( FALSE );
        }

        return TRUE;
    }

    if( actor::OnProperty( rPropQuery ) )
    {
        return TRUE;
    }

    // Human
    s32 PathIndex = rPropQuery.PushPath( "Player\\Human Strain\\" );
    {
        if ( OnStrainProperty( rPropQuery ) )
        {
            return TRUE;
        }
        if ( OnStrainControlProperty( rPropQuery ) )
        {
            return TRUE;
        }
    }
    rPropQuery.PopPath( PathIndex );

    return FALSE;
}

//===========================================================================

xbool player::OnStrainProperty( prop_query& rPropQuery )
{
    if( m_Skin.OnProperty( rPropQuery ) )
    {
        return TRUE;
    }

    if ( m_Physics.OnProperty( rPropQuery ) )
    {
        return TRUE;
    }

    if( rPropQuery.IsVar( "RenderInst\\AnimFile" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_AnimGroup.GetName(), RESOURCE_NAME_SIZE );
        }
        else            
        {
            // WARNING:
            // It may be some problem here. The resurce handles can't start counting references 
            // untill a name has been assign to them. Not only that but when a new name is set it
            // must make sure that the old name is decremented reference wise.
            m_AnimGroup.SetName( rPropQuery.GetVarExternal() );

            // Make sure that this are clear not matter what
            m_iCameraBone        = -1;
            m_iCameraTargetBone  = -1;

            // If we can load this animgoup then we need to extract some info
            if( m_AnimGroup.GetPointer() )
                OnAnimationInit( );

            // Notify the user if we don't have certain key bones
            if( m_iCameraBone == -1 )
                x_DebugMsg( "WARNING: There is not Camera bone (bone_cam) in the skeleton of the player(%d)\n", m_pPlayerTitle );

            if( m_iCameraTargetBone == -1 )
                x_DebugMsg( "WARNING: There is not Camera TargetBone bone(bone_cam.Target) in the skeleton of the player(%d)\n", m_pPlayerTitle );
        }
        return TRUE;
    }

    // External
    if( rPropQuery.IsVar( "Audio Package" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_hAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = rPropQuery.GetVarExternal();

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
        return TRUE;
    } 
    
     s32 ID = rPropQuery.PushPath( "Friendly Factions\\" );
     if ( factions_manager::OnPropertyFriends( rPropQuery, m_StrainFriendFlags ) )
     {
         return TRUE;
     }
     rPropQuery.PopPath( ID );



    return FALSE;
}

//===========================================================================

// Animation initialization methods.
void player::OnAnimationInit( void )
{
    s32 i;

    LOG_MESSAGE( "player::OnAnimationInit", "" );

    m_AnimPlayer.SetAnimGroup( m_AnimGroup );
    m_AnimPlayer.SetAnim( 0, TRUE, TRUE );

    if( m_AnimPlayer.GetBoneIndex( "bone_cam") != -1 ) 
        m_iCameraBone = m_AnimPlayer.GetBoneIndex( "bone_cam");

    if( m_AnimPlayer.GetBoneIndex( "bone_cam.Target") != -1 )
        m_iCameraTargetBone = m_AnimPlayer.GetBoneIndex( "bone_cam.Target");

    // Reset our animations for this strain.
    ResetAnimationTable( );

    // Start collecting animations for each state
    animation_state AnimIndex = ANIM_STATE_UNDEFINED;
    const anim_group& AnimGroup = m_AnimPlayer.GetAnimGroup();

    for(  i=0; i < AnimGroup.GetNAnims(); i++ )
    {
        s32 WeaponIndex = -1;
        AnimIndex = ANIM_STATE_UNDEFINED;

        const anim_info& AnimInfo   = AnimGroup.GetAnimInfo( i );
        const char*      pAnimName  = AnimInfo.GetName();

        // This animation is always there...
        if ( x_strcmp( pAnimName, "BIND_POSE" ) == 0 )
            continue;

        WeaponIndex = inventory2::ItemToWeaponIndex( GetWeaponFromAnimName( pAnimName ) );
        AnimIndex   = GetAnimStateFromName( pAnimName );

        if(   !((WeaponIndex == 0) && (AnimIndex == ANIM_STATE_DEATH)) 
            && ((WeaponIndex <= 0) || (AnimIndex < 0)) )
        {
            //x_try;
            //x_throw( xfs( "WARNING!: Don't know what to do with animation %s for player" , pAnimName ));
            //x_catch_display;
            continue;
        }

        //We have valid index to the animation table, now set the values
        state_anims& State =  m_Anim[WeaponIndex][AnimIndex];
        if( State.nPlayerAnims >= MAX_ANIM_PER_STATE )
        {
            x_throw( xfs( "Too many animations of this type %s for player" , pAnimName ));
            return;
        }
        else
        {
            if ( AnimIndex >= ANIM_STATE_DEATH )
            {
                s32 nAnimState = AnimIndex;
                for ( s32 j = 0; j < INVEN_NUM_WEAPONS; j++ )
                {
                    m_Anim[j][nAnimState].PlayerAnim[ANIM_PRIORITY_DEFAULT] = i;
                    m_Anim[j][nAnimState].nPlayerAnims = 1;
                    m_Anim[j][nAnimState].nWeaponAnims++;
                }
            }

            // Set the index of the animation in the table at the appropriate place.
            ASSERT(State.nPlayerAnims < MAX_ANIM_PER_STATE);
            State.PlayerAnim[State.nPlayerAnims] = i;
            State.nPlayerAnims++;
        }
        
    }
}

//==============================================================================

void player::ResetAnimationTable( void )
{
    LOG_MESSAGE( "player::ResetAnimationTable", "" );

    s32 i,j;

    //clear the animation array.
    for( i=0; i < INVEN_NUM_WEAPONS; i++ )
    {
        for ( j = 0; j < ANIM_STATE_MAX; j++ )
        {
            m_Anim[i][j].nPlayerAnims = 0;
            m_Anim[i][j].nWeaponAnims = 0;
        }
    }
}

//==============================================================================

inven_item player::GetWeaponFromAnimName( const char* pAnimName )
{
    //parse for weapon name.  Documentation on naming conventions used for the player and weapon animations can
    //be found in C:\GameData\A51\Source\Art\Characters\Mut 01_02 - Arms\NOTES_MUT01_01.txt
    inven_item retValue = INVEN_NULL;

    if( x_stristr( pAnimName, "SMP_" ) )
    {
        retValue = INVEN_WEAPON_SMP;
    }

// KSS -- TO ADD NEW WEAPON
    else if( x_stristr( pAnimName, "SHT_" ) )
    {
        retValue = INVEN_WEAPON_SHOTGUN;
    }
    else if( x_stristr( pAnimName, "SCN_" ) )
    {
        retValue = INVEN_WEAPON_SCANNER;
    }
    else if( x_stristr( pAnimName, "SNI_" ) )
    {
        retValue = INVEN_WEAPON_SNIPER_RIFLE;
    }
    else if ( x_stristr( pAnimName, "EGL_" ) )
    {
        retValue = INVEN_WEAPON_DESERT_EAGLE;
    }
    else if( x_stristr( pAnimName, "MSN_" ) )
    {
        retValue = INVEN_WEAPON_MESON_CANNON;
    }
    else if( x_stristr( pAnimName, "BBG_" ) )
    {
        retValue = INVEN_WEAPON_BBG;
    }
    else if( x_stristr( pAnimName, "TRA_" ) )
    {
        retValue = INVEN_WEAPON_TRA;
    }
    else if( x_stristr( pAnimName, "2MP_" ) )
    {
        retValue = INVEN_WEAPON_DUAL_SMP;
    }
    else if( x_stristr( pAnimName, "2SH_" ) )
    {
        retValue = INVEN_WEAPON_DUAL_SHT;
    }
    else if( x_stristr( pAnimName, "MUT_" ) )
    {
        retValue = INVEN_WEAPON_MUTATION;
    }

    return retValue;
}
 
//==============================================================================

player::animation_state player::GetAnimStateFromName( const char* pAnimName )
{
    //parse for animation state.  Documentation on naming conventions used for the player and weapon animations can
    //be found in C:\GameData\A51\Source\Art\Characters\Mut 01_02 - Arms\NOTES_MUT01_01.txt
    animation_state retValue = ANIM_STATE_UNDEFINED;

    if( x_stristr( pAnimName, "_Idle" ) )
    {
        retValue = ANIM_STATE_IDLE;
    }
    else if( x_stristr( pAnimName, "_Switch_To" ) )
    {
        retValue = ANIM_STATE_SWITCH_TO;
    }
    else if( x_stristr( pAnimName, "_Switch_From" ) )
    {
        retValue = ANIM_STATE_SWITCH_FROM;
    }
    else if( x_stristr( pAnimName, "_Pickup" ) )
    {
        retValue = ANIM_STATE_PICKUP;
    }
    else if( x_stristr( pAnimName, "_Discard" ) )
    {
        retValue = ANIM_STATE_DISCARD;
    }
//
// NEEDS TO RESOLVE THIS "_Reload" will return true when looking at ".._Reload_IN"
// for now we will check the Reload_In and Reload_Out stuff first.
//
//
    else if( x_stristr( pAnimName, "_Load_IN" ) )
    {
        retValue = ANIM_STATE_RELOAD_IN;
    }
    else if( x_stristr( pAnimName, "_Load_OUT" ) )
    {
        retValue = ANIM_STATE_RELOAD_OUT;
    }

    else if( x_stristr( pAnimName, "_Reload" ) )
    {
        retValue = ANIM_STATE_RELOAD;
    }
    else if( x_stristr( pAnimName, "_Fire" ) )
    {
        retValue = ANIM_STATE_FIRE;
    }
    else if( x_stristr( pAnimName, "_AltFire" ) )
    {
        retValue = ANIM_STATE_ALT_FIRE;
    }
    else if( x_stristr( pAnimName, "_Grenade" ) )
    {
        retValue = ANIM_STATE_GRENADE;
    }
    else if( x_stristr( pAnimName, "_AltGrenade" ) )
    {
        retValue = ANIM_STATE_ALT_GRENADE;
    }
    // this is the mutation melee "spear"
    else if( x_stristr( pAnimName, "_Spear" ) )
    {
        retValue = ANIM_STATE_MUTATION_SPEAR;
    }    
/////////////////////////////////////////
// START -- Melee Section
    else if( x_stristr( pAnimName, "_Melee" ) )
    {
        retValue = ANIM_STATE_MELEE;
    }
    else if( x_stristr( pAnimName, "_AttackFromCenter" ) )
    {
        retValue = ANIM_STATE_MELEE_FROM_CENTER;
    }
    else if( x_stristr( pAnimName, "_AttackFromDown" ) )
    {
        retValue = ANIM_STATE_MELEE_FROM_DOWN;
    }
    else if( x_stristr( pAnimName, "_AttackFromLeft" ) )
    {
        retValue = ANIM_STATE_MELEE_FROM_LEFT;
    }
    else if( x_stristr( pAnimName, "_AttackFromRight" ) )
    {
        retValue = ANIM_STATE_MELEE_FROM_RIGHT;
    }
    else if( x_stristr( pAnimName, "_AttackFromUp" ) )
    {
        retValue = ANIM_STATE_MELEE_FROM_UP;
    }
// END -- Melee Section
/////////////////////////////////////////

/////////////////////////////////////////
// START -- Combo Section
    else if( x_stristr( pAnimName, "_ComboBegin"))
    {
        retValue = ANIM_STATE_COMBO_BEGIN;
    }
    else if( x_stristr( pAnimName, "_ComboHit"))
    {
        retValue = ANIM_STATE_COMBO_HIT;
    }
    else if( x_stristr( pAnimName, "_ComboEnd"))
    {
        retValue = ANIM_STATE_COMBO_END;
    }
// END -- Combo Section
/////////////////////////////////////////

    else if( x_stristr( pAnimName, "_Ramp_Up" ) )
    {
        retValue = ANIM_STATE_RAMP_UP;
    }
    else if( x_stristr( pAnimName, "_Ramp_Down" ) )
    {
        retValue = ANIM_STATE_RAMP_DOWN;
    }
    else if( x_stristr( pAnimName, "_Hold" ) )
    {
        retValue = ANIM_STATE_HOLD;
    }
    else if ( x_stristr( pAnimName, "_AltHold" ) )
    {
        retValue =  ANIM_STATE_ALT_HOLD;
    }
    else if( x_stristr( pAnimName, "_Run" ) )
    {
        retValue = ANIM_STATE_RUN;
    }
    else if( x_stristr( pAnimName, "_Death01" ) )
    {
        retValue = ANIM_STATE_DEATH;
    }
    else if ( x_stristr( pAnimName, "_Mutation_" ) )
    {
        retValue = ANIM_STATE_CHANGE_MUTATION;
    }
    else if ( x_stristr( pAnimName, "_AltRamp_Up" ) )
    {
        retValue = ANIM_STATE_ALT_RAMP_UP;
    }
    else if ( x_stristr( pAnimName, "_AltRamp_Down" ) )
    {
        retValue = ANIM_STATE_ALT_RAMP_DOWN;
    }
    else if ( x_stristr( pAnimName, "_ZoomIn" ) )
    {
        retValue = ANIM_STATE_ZOOM_IN;
    }
    else if ( x_stristr( pAnimName, "_ZoomOut" ) )
    {
        retValue = ANIM_STATE_ZOOM_OUT;
    }
    else if ( x_stristr( pAnimName, "_ZoomIdle" ) )
    {
        retValue = ANIM_STATE_ZOOM_IDLE;
    }
    else if ( x_stristr( pAnimName, "_ZoomRun" ) )
    {
        retValue = ANIM_STATE_ZOOM_RUN;
    }
    else if ( x_stristr( pAnimName, "_ZoomFire" ) )
    {
        retValue = ANIM_STATE_ZOOM_FIRE;
    }
    else
    {
        retValue = ANIM_STATE_UNDEFINED;
    }

    return retValue;
}

//===========================================================================
static f32 s_TEST_min_fall_vel = 400.f;
static f32 s_TEST_min_damage_vel = 1100.f;
static f32 s_TEST_volume_attenuation = 900.f;

void player::ProcessSfxEvents ( void )
{
    // Did we jumped.
    if( (m_Physics.GetFallMode()) && (m_Physics.GetVelocity().GetY() > 0.0f) && (m_PeakJumpVelocity == -1.0f) )
    {
        m_PeakJumpVelocity = m_Physics.GetVelocity().GetY();
        g_AudioMgr.PlayVolumeClipped( "HumanMale_JumpGrunt", GetPosition(), GetZone1(), TRUE );
    }
    
    if( (m_Physics.GetVelocity().GetY() <= 0.0f) && !(m_Physics.GetFallMode()) )
    {
        m_PeakJumpVelocity = -1.0f;
    }
            
    // Are we going to land.
    if( (m_Physics.GetFallMode()) && (m_Physics.GetVelocity().GetY() < 0.0f) )
    {
        m_PeakLandVelocity = (m_Physics.GetVelocity().GetY()) * -1.0f;
    }
    else if( (m_PeakLandVelocity != -1.0f) && !(m_Physics.GetFallMode()) )
    {        
        if( m_PeakLandVelocity > s_TEST_min_fall_vel )
        {
                
            if( m_PeakLandVelocity > s_TEST_min_damage_vel )
            {
                g_AudioMgr.PlayVolumeClipped( "HumanMale_LandGrunt", GetPosition(), GetZone1(), TRUE );
            }

            f32 ImpactVolume = m_PeakLandVelocity/s_TEST_volume_attenuation;

            if( ImpactVolume > 1.0f )
                ImpactVolume = 1.0f;

            m_PeakLandVelocity = -1.0f;

            s32 VoiceId = g_AudioMgr.PlayVolumeClipped( GetFootfallLandSweetner( GetFloorMaterial() ), GetPosition(), GetZone1(), TRUE );
            g_AudioMgr.SetVolume( VoiceId, ImpactVolume );

            g_AudioMgr.PlayVolumeClipped( GetFootfallHeel( GetFloorMaterial() ), GetPosition(), GetZone1(), TRUE );
         }
    }
}

//===========================================================================
player::animation_state player::GetMotionTransitionAnimState( void )
{
    animation_state retState = ANIM_STATE_UNDEFINED;

    f32     VelocitySquared = m_Physics.GetVelocity().LengthSquared();
    xbool   bIdle           = VelocitySquared < m_fMinRunSpeed;

    if ( bIdle )
    {
        retState = ANIM_STATE_IDLE;
    }
    else
    {
        //if we're transitioning from 
        retState = ANIM_STATE_RUN;
    }

    return retState;    
}

//==============================================================================

void player::SetAnimation( const animation_state& AnimState , const s32& nAnimIndex , const f32& fBlendTime )
{
    // increment our pain event ID whenever we change animations;
    m_CurrentPainEventID = pain_event::CurrentEventID++;
    if( pain_event::CurrentEventID >= S32_MAX )
    {
        pain_event::CurrentEventID = 0;
    }

    //Get a reference to the state that we are considering
    s32 WeaponIndex = inventory2::ItemToWeaponIndex( m_CurrentWeaponItem );
    state_anims& State          = m_Anim[WeaponIndex][AnimState];
    state_anims& WeaponState    = m_Anim[WeaponIndex][AnimState];
    

    xbool bResetFrame       = (m_CurrentAnimStateIndex == nAnimIndex) ? TRUE : FALSE;

    m_PreviousAnimIndex     = m_CurrentAnimIndex;
    m_CurrentAnimIndex      = WeaponState.WeaponAnim[nAnimIndex];

    m_PreviousAnimStateIndex= m_CurrentAnimStateIndex;
    m_CurrentAnimStateIndex = nAnimIndex;

    if( State.nPlayerAnims > nAnimIndex &&  WeaponState.nWeaponAnims > nAnimIndex )
    {
        //set the animation in the player.
        m_AnimPlayer.SetAnim( State.PlayerAnim[nAnimIndex], TRUE, TRUE , fBlendTime , bResetFrame );
        
        switch ( m_CurrentAnimState )
        {
        case ANIM_STATE_DEATH:              // Intentional fallthrough
        case ANIM_STATE_CINEMA:             // Intentional fallthrough
        case ANIM_STATE_MISSION_FAILED:
            // do nothing
            break; 
        default:
            {
                //set the animation for the weapon
                new_weapon* pWeapon = GetCurrentWeaponPtr();
                if ( pWeapon )
                {
                    pWeapon->SetAnimation( WeaponState.WeaponAnim[nAnimIndex] , fBlendTime , bResetFrame );
                }
            }
        }
    }
}

//==============================================================================

void player::CameraFall( f32 fPercentHeight )
{
    f32 fHeight =  GetBBox().Min.GetY() + ((GetBBox().GetSize().GetY() + m_EyesOffset.GetY() ) * fPercentHeight) + 10.0f; //10 cm buffer
    m_PosOverrideCamera = vector3( GetPosition().GetX(), fHeight, GetPosition().GetZ() );
    MoveAnimPlayer( m_PosOverrideCamera );
}

//==============================================================================

void player::OnAnimEvents( void )
{
    g_EventMgr.HandleSuperEvents( m_AnimPlayer, this );
}

//===========================================================================
f32 s_FireShakeTime     = 0.75f;
f32 s_FireShakeAmount   = 0.9f;
f32 s_FireShakeSpeed    = 1.0f;
f32 s_FireFeedbackDuration  = 0.1f;
f32 s_FireFeedbackIntensity = 0.75f;

f32 s_AltFireShakeTime     = 0.85f;
f32 s_AltFireShakeAmount   = 1.4f;
f32 s_AltFireShakeSpeed    = 1.0f;
f32 s_AltFireFeedbackDuration  = 0.15f;
f32 s_AltFireFeedbackIntensity = 0.9f;

// how much (in degrees) do we offset the pitch so the JBG will start off higher.
tweak_handle JBEAN_PitchThrowOffsetTweak("JBEAN_PitchThrowOffset");

// this is the highest angle at which the pitch offset will apply.
tweak_handle JBEAN_PitchThrowAngleTweak("JBEAN_PitchThrowAngle");

//===========================================================================
void player::OnEvent( const event& Event )
{
    (void)Event;

    if( m_ActivePlayerPad == -1 )
        return;

    if( Event.Type == event::EVENT_INTENSITY )
    {
         const intensity_event& IntensityEvent = intensity_event::GetSafeType( Event );
         
         DoFeedback(IntensityEvent.ControllerDuration, IntensityEvent.ControllerIntensity);
         ShakeView(IntensityEvent.CameraShakeTime, IntensityEvent.CameraShakeAmount, IntensityEvent.CameraShakeSpeed );
    }

    if( Event.Type == event::EVENT_GENERIC )
    {
        const generic_event& GenericEvent = generic_event::GetSafeType( Event );
        if( x_strcmp( GenericEvent.GenericType, "FP_Mutation_Switch" ) == 0 )
        {
            s32 WeaponIndex = inventory2::ItemToWeaponIndex( m_CurrentWeaponItem );
            s32 nAnimIndex = m_Anim[WeaponIndex][ANIM_STATE_CHANGE_MUTATION].PlayerAnim[0];
            f32 nFrame = m_AnimPlayer.GetFrame();
            
            m_AnimPlayer.SetAnim( nAnimIndex, TRUE, TRUE, 0.f );
            m_AnimPlayer.SetFrame(nFrame);

            SetAnimState( ANIM_STATE_CHANGE_MUTATION );
//          g_PostEffectMgr.AddToHowlBlur( 0.4f, 0.5f, 1.0f, .3f );
        }
        else if( x_strcmp( GenericEvent.GenericType, "Player_Death" ) == 0 )
        {
            if ((m_CurrentAnimState == ANIM_STATE_DEATH) && (m_AnimStage > 1))
            {
                const anim_event& Event = m_AnimPlayer.GetEvent( GenericEvent.EventIndex );
                // the timerange here is for falling
                f32 nTotalFramesForEvent = (f32)Event.GetInt(anim_event::INT_IDX_END_FRAME) - Event.GetInt(anim_event::INT_IDX_START_FRAME);
                f32 nCurrentEventFrame = m_AnimPlayer.GetFrame() - Event.GetInt(anim_event::INT_IDX_START_FRAME);
                f32 fPercentHeight =  ((nTotalFramesForEvent-nCurrentEventFrame)/nTotalFramesForEvent);
                CameraFall(fPercentHeight);
            }
        }
        else if( x_strcmp( GenericEvent.GenericType, "Spear_Out_Left" ) == 0 )
        {
            GetMutationMeleeWeapon()->FireTendril( GetEyesPosition() , 
                                                    m_ForwardVelocity + m_StrafeVelocity, 
                                                    GetProjectileTrajectory() , 
                                                    GetGuid(), TRUE );
        }
        else if( x_strcmp( GenericEvent.GenericType, "Spear_Out_Right" ) == 0 )
        {
            GetMutationMeleeWeapon()->FireTendril( GetEyesPosition(), 
                                                   m_ForwardVelocity + m_StrafeVelocity, 
                                                   GetProjectileTrajectory(), 
                                                   GetGuid(), FALSE );
        }
        else if( x_strcmp( GenericEvent.GenericType, "Spear_In_Left" ) == 0 )
        {
            GetMutationMeleeWeapon()->RetractTendril(TRUE);
        }
        else if( x_strcmp( GenericEvent.GenericType, "Spear_In_Right" ) == 0 )
        {
            GetMutationMeleeWeapon()->RetractTendril(FALSE);
        }
        else if( x_strcmp( GenericEvent.GenericType, "Mutant Vision" ) == 0 )
        {
            if ( IsMutated() )
                m_bIsMutantVisionOn = TRUE;
            else
                m_bIsMutantVisionOn = FALSE;

#ifdef mreed 
            LOG_MESSAGE( "Mutation", "m_bIsMutantVisionOn = %s (Event)", IsMutated() ? "TRUE" : "FALSE" );
#endif

        }
        // check for mutant melee stuff here
        else if( x_stristr( GenericEvent.GenericType, "MeleeFrom" ) )
        {
            GetMutationMeleeWeapon()->DoExtremeMelee();
        }
        else if( x_stristr(GenericEvent.GenericType, "Combo_Start") )
        {
            // threshold start
            m_bCanRequestCombo = TRUE;
        }
        else if( x_stristr(GenericEvent.GenericType, "Combo_End") )
        {
            // threshold timed out
            m_bCanRequestCombo = FALSE;
        }
        else if( x_stristr(GenericEvent.GenericType, "CompletedReload") )
        {
#ifdef ksaffel
            //x_printfxy(2,0,"====== RELOAD COMPLETE ======");
#endif

            // reload sequence has gone far enough to count, the rest is fluff
            new_weapon *pWeapon = GetCurrentWeaponPtr();
            if( pWeapon )
            {
                pWeapon->SetReloadCompleted(TRUE);
            }
        }
    }
    else if( Event.Type == event::EVENT_WEAPON )
    {
        const weapon_event& WeaponEvent = weapon_event::GetSafeType( Event );
        
        switch( WeaponEvent.WeaponState )
        {
            case new_weapon::EVENT_FIRE:
            case new_weapon::EVENT_FIRE_LEFT: 
            case new_weapon::EVENT_FIRE_RIGHT: 
            {
                // don't allow player to switch weapons, zoom in, attack, etc.
                if( m_bHidePlayerArms )
                {
                    break;
                }

                s32 iFirePoint = -1;
                
                switch( WeaponEvent.WeaponState )
                {
                    case new_weapon::EVENT_FIRE:        iFirePoint = new_weapon::FIRE_POINT_DEFAULT; break;
                    case new_weapon::EVENT_FIRE_LEFT:   iFirePoint = new_weapon::FIRE_POINT_LEFT;    break;
                    case new_weapon::EVENT_FIRE_RIGHT:  iFirePoint = new_weapon::FIRE_POINT_RIGHT;   break;
                }

                new_weapon* pWeapon = GetCurrentWeaponPtr();
                
                if( pWeapon )
                {
                    new_weapon::reticle_radius_parameters ReticleParams = GetReticleParams();
                    m_ReticleShotPenalty += ReticleParams.m_PenaltyForShot;
                    const f32 MaxPenalty = (ReticleParams.m_MaxRadius- ReticleParams.m_MaxMovementPenalty) - ReticleParams.m_MinRadius;
                    m_ReticleShotPenalty = MIN( MaxPenalty, m_ReticleShotPenalty );
                    pWeapon->SetTarget( GetEnemyOnReticle() );
                    pWeapon->FireWeapon( GetEyesPosition() , m_ForwardVelocity + m_StrafeVelocity, m_WpnHoldTime, GetProjectileTrajectory() , GetGuid(), iFirePoint );
                }

                //ShakeView( s_FireShakeTime, s_FireShakeAmount, s_FireShakeSpeed );
                //DoFeedback(s_FireFeedbackDuration, s_FireFeedbackIntensity );
            }
            break;
            case new_weapon::EVENT_ALT_FIRE:
            case new_weapon::EVENT_ALT_FIRE_LEFT: 
            case new_weapon::EVENT_ALT_FIRE_RIGHT: 
            {
                // don't allow player to switch weapons, zoom in, attack, etc.
                if( m_bHidePlayerArms )
                {
                    break;
                }

                s32 iFirePoint = -1;
                
                switch( WeaponEvent.WeaponState )
                {
                    case new_weapon::EVENT_ALT_FIRE:        iFirePoint = new_weapon::FIRE_POINT_DEFAULT; break;
                    case new_weapon::EVENT_ALT_FIRE_LEFT:   iFirePoint = new_weapon::FIRE_POINT_LEFT;    break;
                    case new_weapon::EVENT_ALT_FIRE_RIGHT:  iFirePoint = new_weapon::FIRE_POINT_RIGHT;   break;
                }

                new_weapon* pWeapon = GetCurrentWeaponPtr();
                if( pWeapon )
                {
                    new_weapon::reticle_radius_parameters ReticleParams = GetReticleParams();
                    m_ReticleShotPenalty += ReticleParams.m_PenaltyForShot;
                    const f32 MaxPenalty = (ReticleParams.m_MaxRadius- ReticleParams.m_MaxMovementPenalty) - ReticleParams.m_MinRadius;
                    m_ReticleShotPenalty = MIN( MaxPenalty, m_ReticleShotPenalty );
                    pWeapon->SetTarget( GetEnemyOnReticle() );
                    pWeapon->FireSecondary( GetEyesPosition() , m_ForwardVelocity + m_StrafeVelocity, m_WpnHoldTime, GetProjectileTrajectory() , GetGuid(), iFirePoint );
                }

                //ShakeView( s_AltFireShakeTime, s_AltFireShakeAmount, s_AltFireShakeSpeed );
                //DoFeedback( s_AltFireFeedbackDuration, s_AltFireFeedbackIntensity );
            }
            break;
            case new_weapon::EVENT_GRENADE:
            {
                // don't allow player to switch weapons, zoom in, attack, etc.
                if( m_bHidePlayerArms )
                {
                    break;
                }

                tweak_handle SpeedTweak("PLAYER_GrenadeThrowSpeed");
                // Compute velocity
                vector3 Dir = GetView().GetViewZ();
                // TODO: Tweak throw speed based on pitch of vector, less power when looking down, etc.
                vector3 Velocity = Dir * SpeedTweak.GetF32();
                Velocity += m_ForwardVelocity + m_StrafeVelocity;

                pain_handle PainHandle(xfs("%s_GRENADE",GetLogicalName()));

                // which grenade do we throw?
                if( m_CurrentGrenadeType2 == INVEN_GRENADE_FRAG )
                {
                    // Create the Grenade projectile.
                    guid GrenadeID = CREATE_NET_OBJECT( grenade_projectile::GetObjectType(), TYPE_GRENADE );
                    grenade_projectile* pFragGrenade = ( grenade_projectile* ) g_ObjMgr.GetObjectByGuid( GrenadeID );
                    
                    // make sure the grenade was created.
                    ASSERT( pFragGrenade );

                    // New Position
                    vector3 NewEventPos = SetupGrenadeThrow( WeaponEvent.Pos );

                    pFragGrenade->Setup( GetGuid(),
                        net_GetSlot(),
                        NewEventPos,
                        radian3(0.0f,0.0f,0.0f),
                        Velocity,
                        GetZone1(),
                        GetZone2(),
                        PainHandle );
                
                    if( !DEBUG_INFINITE_AMMO )
                    {
                        m_Inventory2.RemoveAmount( m_CurrentGrenadeType2, 1.0f );
                    }

                    #ifndef X_EDITOR
                    m_NetDirtyBits |= TOSS_BIT; // NETWORK
                    #endif X_EDITOR 
                }
            }    
            break;
            case new_weapon::EVENT_ALT_GRENADE:
            {
                // don't allow player to switch weapons, zoom in, attack, etc.
                if( m_bHidePlayerArms )
                {
                    break;
                }

                pain_handle PainHandle(xfs("%s_JBEAN",GetLogicalName()));

                // which grenade do we throw?
                if( m_CurrentGrenadeType2 == INVEN_GRENADE_JBEAN )
                { 
                    // Create the Jumping Bean Grenade projectile.
                    guid GrenadeID = CREATE_NET_OBJECT( jumping_bean_projectile::GetObjectType(), TYPE_JBEAN_GRENADE );
                    jumping_bean_projectile* pJBeanGrenade = ( jumping_bean_projectile* ) g_ObjMgr.GetObjectByGuid( GrenadeID );

                    // make sure the grenade was created.
                    ASSERT( pJBeanGrenade );

                    if( !pJBeanGrenade )
                    {
                        return;
                    }

                    f32 Speed = 0.0f;
                    xbool bExpert = (pJBeanGrenade->GetGrenadeMode() == GM_EXPERT);
                    #ifndef X_EDITOR
                    if( GameMgr.GetGameType() != GAME_CAMPAIGN )
                    {
                        bExpert = FALSE;
                    }
                    #endif
                    if( bExpert )
                    {
                        tweak_handle SpeedTweak("JBEAN_ThrowSpeed");
                        Speed = SpeedTweak.GetF32();
                    }
                    else
                    {
                        tweak_handle SpeedTweak("JBEAN_ThrowSpeed_Normal");
                        Speed = SpeedTweak.GetF32();
                    }

                    // Compute velocity
                    vector3 Dir = GetView().GetViewZ();
                    // TODO: Tweak throw speed based on pitch of vector, less power when looking down, etc.
                    vector3 Velocity = Dir * Speed;
                    Velocity += m_ForwardVelocity + m_StrafeVelocity;

                    if( bExpert )
                    {
                        radian Pitch, Yaw;
                        Velocity.GetPitchYaw(Pitch, Yaw);

                        radian JBG_PitchAngle = JBEAN_PitchThrowAngleTweak.GetRadian();

                        // up to a point, rotate the way for a faked "lob" of the grenade
                        if( Pitch > JBG_PitchAngle )
                        {
                            f32 T = 1.0f;

                            // if we're looking upwards, scale pitch offset
                            if( Pitch < R_0 )
                            {
                                T = Pitch/JBG_PitchAngle;

                                // since the values have to be backwards to work, must flip T
                                T = 1.0f - T;
                            }

                            radian JBG_PitchOffset = JBEAN_PitchThrowOffsetTweak.GetRadian();
                            Pitch -= JBG_PitchOffset*T;
                            f32 Scalar = Velocity.Length();
                            Velocity.Set(Pitch, Yaw);
                            Velocity.Scale(Scalar);
                        }
                    }

                    vector3 NewEventPos = SetupGrenadeThrow( WeaponEvent.Pos );

                    pJBeanGrenade->Setup( GetGuid(),
                        net_GetSlot(),
                        NewEventPos,
                        radian3(0.0f,0.0f,0.0f),
                        Velocity,
                        GetZone1(),
                        GetZone2(),
                        PainHandle );
                
                    if( !DEBUG_INFINITE_AMMO )
                    {
                        m_Inventory2.RemoveAmount( m_CurrentGrenadeType2, 1.0f );
                    }

                    #ifndef X_EDITOR
                    m_NetDirtyBits |= TOSS_BIT; // NETWORK
                    #endif X_EDITOR
                }
            }   
            break;

            default:
            break;
        }
    }
    else if( Event.Type == event::EVENT_PAIN )
    {
        const pain_event& PainSuperEvent = pain_event::GetSafeType( Event );

        // check if this is a melee pain and kick it off
        if( ( PainSuperEvent.PainType == pain_event::EVENT_PAIN_MELEE ) )
        {
            EmitMeleePain();
        }
    }
}

//=============================================================================
vector3 player::SetupGrenadeThrow( const vector3 &EventPos )
{
    vector3 NewEventPos = EventPos;

#ifdef DEBUG_GRENADE_THROWING
    // for drawing spheres and such
    g_EventPos = NewEventPos;
#endif
    // do some LOS checks here so we don't throw the grenade through the wall.
    {
        /*
        3
        /|
        / |
        /  |
        /   |
        /    |
        /     |
        1------2
        (arm)
        1 = eyes position
        2 = point perpendicular to eyes position
        3 = event pos

        To make sure we can throw the grenade at the event pos, we need to:
        ~ make sure that we can see point 2 from point 1
        AND, If you can see it then,
        ~ make sure that we can see point 3 from point 2
        if not, it failed right off

        This will ensure all bases are covered in events such as:

        =====================================================================
        (FAIL)
                    3           - Actual Event Position (3)
        --------------------    - Wall
                    N           - New event position (N)
             1------|2          - Player Eye Position (1) and point perpendicular to eyes position (2)
        (arm)

        * You can't see point 2 from put 1 and you can't see point 3 from point 2

        =====================================================================
        (FAIL) 
                      (wall)
                         /
            3      _2  /N           - Actual Event Position (3) then New Event Position (N)
                    |/
                   /|               - Point perpendicular to eyes position (2) 
                 /  | (arm)         
               /    1               - Player Eye Position (1)

        * You can't see point 2 from point 1 even though you can see point 3 from point 2

        =====================================================================  
        (PASS)

                3            - Actual Event Position (3)
        -------              - Wall                                        
         1------|2           - Player Eye Position (1) and point perpendicular to eyes position (2)
        (arm)

        * You can see point 2 from point 1 and you can see point 3 from point 2
        =====================================================================  
        */

        // get player position
        vector3 Point1 = vector3(0.0f, 0.0f, 0.0f);
        // not set yet
        vector3 Point2 = vector3(0.0f, 0.0f, 0.0f);
        // get event position
        vector3 Point3 = NewEventPos;

        // load up our points for calculations
        GetThrowPoints( Point1, Point2, Point3 );

        // ---------------------------------------------------------------------------------
        // Check point 1 to point 2 (player 'eye' position to 'shoulder')
        g_CollisionMgr.RaySetup( GetGuid(),     // MovingObjGuid,
                                 Point1,        // WorldStart,
                                 Point2);       // WorldEnd,

        g_CollisionMgr.IgnoreGlass();

        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                                        object::ATTR_BLOCKS_PLAYER_LOS, 
                                        (object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING)) ;

        // we have collisions, this failed
        if( g_CollisionMgr.m_nCollisions )
        {
            // back off the wall a bit
            vector3 theNormal = g_CollisionMgr.m_Collisions[0].Plane.Normal;
            NewEventPos = g_CollisionMgr.m_Collisions[0].Point + (theNormal * 3.5f);
        }
        else  // clear from eyes to shoulder
        {                        
            // ---------------------------------------------------------------------------------
            // Check point 2 to point 3 ('shoulder' position to event position)
            g_CollisionMgr.RaySetup( GetGuid(),     // MovingObjGuid,
                                     Point2,        // WorldStart,
                                     Point3);       // WorldEnd,                        

            g_CollisionMgr.IgnoreGlass();

            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                                            object::ATTR_BLOCKS_LARGE_PROJECTILES, 
                                            (object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING)) ;

            // if we have collisions, pick new point
            if( g_CollisionMgr.m_nCollisions )
            {
                // back off the wall a bit
                vector3 theNormal = g_CollisionMgr.m_Collisions[0].Plane.Normal;
                NewEventPos = g_CollisionMgr.m_Collisions[0].Point + (theNormal * 3.5f);
            }
        }                        
    }

#ifdef DEBUG_GRENADE_THROWING
    // for drawing spheres and such
    g_NewEventPos = NewEventPos;
#endif

    return NewEventPos;
}

//=============================================================================
void player::GetThrowPoints( vector3 &Point1, vector3 &Point2, vector3 &Point3 )
{    
    // get the player position and raise the "eyes" up to the event pos.
    Point1 = GetPosition();
    Point1.GetY() = Point3.GetY();

    // get our view vector (normalized)
    vector3 ViewZ = GetView().GetViewZ();

    // get the vector closest to the line segment from player position to event position
    vector3 Closest = ViewZ.Cross(Point3 - Point1);

    // get distance between player position and event position
    f32 d = Closest.Length();

    // get the lean
    vector3 lean = (GetView().GetViewX());

    if( m_CurrentGrenadeType2 == INVEN_GRENADE_FRAG )
    {
        d = -d;
    }

    // this is our "shoulder" position
    Point2 = Point1 + (lean * d);  // perpendicular position
}

//=============================================================================
//===========================================================================
// Need to use some anti mu?

xbool player::UseAntiMu( collectable_anti_mutagen* pAntiMu )
{
    ( void ) pAntiMu;

    return FALSE;
}

//===========================================================================

void player::SetNonExclusiveState( non_exclusive_states nStateBit )
{
    // don't set the state if we're already in it
    if ( m_NonExclusiveStateBitFlag & nStateBit )
        return;

    m_NonExclusiveStateBitFlag |= nStateBit;
    BeginNonExclusiveState( nStateBit );
}

//===========================================================================

void player::ClearNonExclusiveState( non_exclusive_states nStateBit )
{
    if ( nStateBit & m_NonExclusiveStateBitFlag )
    {
        EndNonExclusiveState( nStateBit );
        m_NonExclusiveStateBitFlag &= ~nStateBit;
    }

}

//===========================================================================
// NOTE:  When new non exclusive states are added, you must clear them here.
void player::ClearAllNonExclusiveStates( void )
{
    ClearNonExclusiveState( NE_STATE_STUNNED );
}

//===========================================================================

void player::BeginNonExclusiveState( non_exclusive_states nStateBit )
{
    switch( nStateBit )
    {
        case NE_STATE_STUNNED:
            BeginStunnedNE();
            break;
        default:
            break;
    }
}

//===========================================================================

void player::UpdateActiveNonExclusiveStates( f32 DeltaTime )
{
    if ( m_NonExclusiveStateBitFlag & NE_STATE_STUNNED )
    {
        UpdateStunnedNE( DeltaTime );
    }
}

//===========================================================================

void player::EndNonExclusiveState( non_exclusive_states nStateBit )
{
    switch( nStateBit )
    {
        case NE_STATE_STUNNED:
            EndStunnedNE();
            break;
        default:
            break;
    }
}

//===========================================================================

void player::ProcessStunnedPain( const pain& Pain )
{
    (void)Pain;
    SetNonExclusiveState( NE_STATE_STUNNED );
}

//===========================================================================

void player::BeginStunnedNE( void )
{
    // Get the current rotation from the view
    m_fStunnedTime = 0.f;

    GetEyesPitchYaw( m_PreStunPitch, m_PreStunYaw );
}

//===========================================================================
void player::UpdateStunnedNE( f32 DeltaTime )
{
    m_fStunnedTime += DeltaTime;

    f32 YawRotFactor = m_fStunnedTime * m_fStunYawChangeSpeed;
    f32 PitchRotFactor = m_fStunnedTime * m_fStunPitchChangeSpeed;


    f32 YawOffset = x_sin( YawRotFactor );
    f32 PitchOffset = x_sin( PitchRotFactor );
    YawOffset *= m_MaxStunPitchOffset;
    PitchOffset *= m_MaxStunYawOffset;

    m_AnimPlayer.SetPitch( m_PreStunPitch + PitchOffset );
    m_AnimPlayer.SetYaw( m_PreStunYaw + YawOffset );

    if ( !m_bInTurret )
    {
        m_Physics.Advance( m_Physics.GetPosition() , DeltaTime );
        OnMove( m_Physics.GetPosition() );
    }

    if ( m_fStunnedTime >= m_fMaxStunTime )
    {
        ClearNonExclusiveState( NE_STATE_STUNNED );
    }
}

//===========================================================================

void player::EndStunnedNE( void )
{
}

//==============================================================================

void player::PlayFootfall( f32 DeltaTime )
{
    //not the active player, don't play footfalls
    if (!IsActivePlayer())
        return;
    
    // Use the player velocity.
    f32 XVel = (m_MaxStrafeVelocity * m_fStrafeValue)/m_MaxStrafeVelocity;
    f32 YVel = (m_MaxFowardVelocity * m_fMoveValue)/m_MaxFowardVelocity;

    f32 AbsYVel         = x_abs( YVel );
    f32 AbsXVel         = x_abs( XVel );
    f32 ComboVel        = x_sqrt( x_sqr( YVel ) + x_sqr( XVel ) );

    f32 CurrentVel    = kMaxForwardVel*ComboVel;
    f32 FeetsPerStep  = (kFeetsPerInitStep + (ComboVel*kFeetsSpeedMod));
    f32 MeterPerStep  = FeetsPerStep*0.3048f;

    m_DelayTillNextStep -= (DeltaTime*1000.0f);
    if( m_DelayTillNextStep < 0.0f )
        m_DistanceTraveled = m_DistanceTraveled + (CurrentVel*DeltaTime);
    m_IsRunning = FALSE;

    // Don't pitch down the footfalls...
    f32 Pitch = 1.0f / g_PerceptionMgr.GetAudioTimeDialation();

    // Going forward.
    if( YVel >= 0.0f )
    {
        if( m_DistanceTraveled > MeterPerStep )
        {
            // Get the delay till toe hit.
            m_DelayCountDown = kForwardDelay * ( ( (kFalloff-ComboVel) < 0.0f ) ? 0.0f : (kFalloff-ComboVel)/kFalloff );
            
            // Play the heel sound and set the volume level depending on the speed.
            m_HeelID = g_AudioMgr.Play( GetFootfallHeel( GetFloorMaterial() ), GetPosition(), GetZone1(), TRUE );
            g_AudioMgr.SetPitch( m_HeelID, Pitch );
            g_AudioManager.NewAudioAlert( m_HeelID, audio_manager::FOOT_STEP, GetPosition(), GetZone1(), GetGuid() );

            f32 Volume  = 0.0f;
            if( (ComboVel*kMaxForwardVel) < kMaxWalkVel || m_bIsCrouching )
            {
                Volume = kLowestVolume + (kMaxWalkVolume-kLowestVolume)*MIN(ComboVel, kMaxWalkVolume);
            }
            else
            {            
                m_IsRunning = TRUE;
                Volume = kMinRunVolume + (kMaxRunVolume-kMinRunVolume)*MIN(ComboVel, kMaxRunVolume);
            }

            g_AudioMgr.SetVolume( m_HeelID, Volume );

            if( AbsXVel > kStrafeInit )
            {
                m_SlideID = g_AudioMgr.Play( GetFootfallSlide( GetFloorMaterial()) );
                g_AudioMgr.SetPitch( m_SlideID, Pitch );
            }
            
            f32 StrafeVolume = MAX(AbsXVel-kStrafeInit, 0.0f)*(1.0f/kStrafeInit);
            StrafeVolume     = StrafeVolume * (AbsYVel > kVertStrafeCutOff) ? 0.0f : ( (kVertStrafeCutOff-AbsYVel)/kVertStrafeCutOff );
            g_AudioMgr.SetVolume( m_SlideID, StrafeVolume );
            
            m_DistanceTraveled = 0;
        }

        if( m_HeelID && ((AbsYVel < kFalloff) || (AbsXVel > kStrafeInit) ) )
        {
            m_DelayCountDown -= (DeltaTime*1000.0f);

            if( m_DelayCountDown < 0.0f )
            {   
                f32 Volume  = kLowestVolume + (1.0f-kLowestVolume)*(MIN(ComboVel, 1.0f)*(1.0f/kFalloff));

                m_ToeID     = g_AudioMgr.PlayVolumeClipped( GetFootfallToe( GetFloorMaterial() ), GetPosition(), GetZone1(), TRUE );
                g_AudioMgr.SetPitch( m_ToeID, Pitch );
                g_AudioMgr.SetVolume( m_ToeID, Volume );
                m_HeelID = 0;

                m_TrailStep ^= (1<<0);
            }
        }

        if( (AbsXVel >= AbsYVel) && m_TrailStep && (m_DelayTillNextStep <= 0.0f) )
            m_DelayTillNextStep = ((kMaxStrafeDelay - kMinStrafeDelay) * (1.0f-AbsXVel)) + kMinStrafeDelay;
        else
            m_DelayTillNextStep = 0.0f;

    }
    // Going backwards.
    else
    {
        if( m_DistanceTraveled > MeterPerStep )
        {
            if( (AbsYVel < kFalloff) || (AbsXVel > kStrafeInit) )
            {
                f32 Volume  = kLowestVolume + (1.0f-kLowestVolume)*(AbsYVel*(1.0f/kFalloff));
                m_ToeID     = g_AudioMgr.PlayVolumeClipped( GetFootfallToe( GetFloorMaterial()), GetPosition(), GetZone1(), TRUE );
                g_AudioMgr.SetPitch( m_ToeID, Pitch );
                g_AudioMgr.SetVolume( m_ToeID, Volume );
            }
        
            m_TrailStep ^= (1<<0);
            
            m_HeelID = 0;
            m_DelayCountDown    = kBackwardDelay * ( ( (kFalloff-ComboVel) < 0.0f ) ? 0.0f : (kFalloff-ComboVel)/kFalloff );
            m_DistanceTraveled  = 0;
        }

        if( !m_HeelID )
        {
            m_DelayCountDown -= (DeltaTime*1000.0f);

            if( m_DelayCountDown < 0.0f )
            {   
                //f32 Volume = g_LowestVolume + (1.0f-g_LowestVolume)*MIN( ComboVel, 1.0f );
                f32 Volume  = 0.0f;
                if( (ComboVel*kMaxForwardVel) < kMaxWalkVel )
                    Volume = kLowestVolume + (kMaxWalkVolume-kLowestVolume)*MIN(ComboVel, kMaxWalkVolume);
                else
                    Volume = kMinRunVolume + (kMaxRunVolume-kMinRunVolume)*MIN(ComboVel, kMaxRunVolume);

                m_HeelID = g_AudioMgr.Play( GetFootfallHeel( GetFloorMaterial()), GetPosition(), GetZone1(), TRUE );
                g_AudioManager.NewAudioAlert( m_HeelID, audio_manager::FOOT_STEP, GetPosition(), GetZone1(), GetGuid() );
                g_AudioMgr.SetPitch( m_HeelID, Pitch );
                g_AudioMgr.SetVolume( m_HeelID, Volume );

                if( AbsXVel > kStrafeInit )
                {
                    m_SlideID = g_AudioMgr.Play( GetFootfallSlide( GetFloorMaterial()) );
                    g_AudioMgr.SetPitch( m_SlideID, Pitch );
                }

                f32 StrafeVolume = MAX(AbsXVel-kStrafeInit, 0.0f)*(1.0f/kStrafeInit);
                StrafeVolume = StrafeVolume * (AbsYVel > kVertStrafeCutOff) ? 0.0f : ( (kVertStrafeCutOff-AbsYVel)/kVertStrafeCutOff );
                g_AudioMgr.SetVolume( m_SlideID, StrafeVolume );

            }
        }

        if( (AbsXVel >= AbsYVel) && m_TrailStep && (m_DelayTillNextStep <= 0.0f) )
            m_DelayTillNextStep = ((kMaxStrafeDelay - kMinStrafeDelay) * (1.0f-AbsXVel)) + kMinStrafeDelay;
        else
            m_DelayTillNextStep = 0.0f;
    }
}

//===========================================================================

char* player::GetFootfallHeel( s32 Material )
{
    switch( Material )
    {        
        case MAT_TYPE_NULL:                 return "FF_Boot_Null_Heel";                  break;
        case MAT_TYPE_EARTH:                return "FF_Boot_Earth_Heel";                 break;
        case MAT_TYPE_ROCK:                 return "FF_Boot_Rock_Heel";                  break;
        case MAT_TYPE_CONCRETE:             return "FF_Boot_Concrete_Heel";              break;
        case MAT_TYPE_SOLID_METAL:          return "FF_Boot_Metal_Heel";                 break;
        case MAT_TYPE_HOLLOW_METAL:         return "FF_Boot_HollowMetal_Heel";           break;
        case MAT_TYPE_METAL_GRATE:          return "FF_Boot_MetalGrate_Heel";            break;
        case MAT_TYPE_PLASTIC:              return "FF_Boot_Plastic_Heel";               break;
        case MAT_TYPE_WATER:                return "FF_Boot_Water_Heel";                 break;
        case MAT_TYPE_WOOD:                 return "FF_Boot_Wood_Heel";                  break;
        case MAT_TYPE_ENERGY_FIELD:         return "FF_Boot_EnergyField_Heel";           break;
        case MAT_TYPE_BULLET_PROOF_GLASS:   return "FF_Boot_BulletProofGlass_Heel";      break;
        case MAT_TYPE_ICE:                  return "FF_Boot_Ice_Heel";                   break;

        case MAT_TYPE_LEATHER:              return "FF_Boot_Leather_Heel";               break;
        case MAT_TYPE_EXOSKELETON:          return "FF_Boot_Exoskeleton_Heel";           break;
        case MAT_TYPE_FLESH:                return "FF_Boot_Flesh_Heel";                 break;
        case MAT_TYPE_BLOB:                 return "FF_Boot_Blob_Heel";                  break;
        
        case MAT_TYPE_FIRE:                 return "FF_Boot_Fire_Heel";                  break;
        case MAT_TYPE_GHOST:                return "FF_Boot_Ghost_Heel";                 break;
        case MAT_TYPE_FABRIC:               return "FF_Boot_Fabric_Heel";                break;
        case MAT_TYPE_CERAMIC:              return "FF_Boot_Ceramic_Heel";               break;
        case MAT_TYPE_WIRE_FENCE:           return "FF_Boot_WireFence_Heel";             break;

        case MAT_TYPE_GLASS:                return "FF_Boot_Glass_Heel";                 break;
        default:
                                            return "Null";
        break;
    }
}

//===========================================================================

char* player::GetFootfallToe( s32 Material )
{
    switch( Material )
    {        
        case MAT_TYPE_NULL:                 return "FF_Boot_Null_Toe";                  break;
        case MAT_TYPE_EARTH:                return "FF_Boot_Earth_Toe";                 break;
        case MAT_TYPE_ROCK:                 return "FF_Boot_Rock_Toe";                  break;
        case MAT_TYPE_CONCRETE:             return "FF_Boot_Concrete_Toe";              break;
        case MAT_TYPE_SOLID_METAL:          return "FF_Boot_Metal_Toe";                 break;
        case MAT_TYPE_HOLLOW_METAL:         return "FF_Boot_HollowMetal_Toe";           break;
        case MAT_TYPE_METAL_GRATE:          return "FF_Boot_MetalGrate_Toe";            break;
        case MAT_TYPE_PLASTIC:              return "FF_Boot_Plastic_Toe";               break;
        case MAT_TYPE_WATER:                return "FF_Boot_Water_Toe";                 break;
        case MAT_TYPE_WOOD:                 return "FF_Boot_Wood_Toe";                  break;
        case MAT_TYPE_ENERGY_FIELD:         return "FF_Boot_EnergyField_Toe";           break;
        case MAT_TYPE_BULLET_PROOF_GLASS:   return "FF_Boot_BulletProofGlass_Toe";      break;
        case MAT_TYPE_ICE:                  return "FF_Boot_Ice_Toe";                   break;

        case MAT_TYPE_LEATHER:              return "FF_Boot_Leather_Toe";               break;
        case MAT_TYPE_EXOSKELETON:          return "FF_Boot_Exoskeleton_Toe";           break;
        case MAT_TYPE_FLESH:                return "FF_Boot_Flesh_Toe";                 break;
        case MAT_TYPE_BLOB:                 return "FF_Boot_Blob_Toe";                  break;
        
        case MAT_TYPE_FIRE:                 return "FF_Boot_Fire_Toe";                  break;
        case MAT_TYPE_GHOST:                return "FF_Boot_Ghost_Toe";                 break;
        case MAT_TYPE_FABRIC:               return "FF_Boot_Fabric_Toe";                break;
        case MAT_TYPE_CERAMIC:              return "FF_Boot_Ceramic_Toe";               break;
        case MAT_TYPE_WIRE_FENCE:           return "FF_Boot_WireFence_Toe";             break;

        case MAT_TYPE_GLASS:                return "FF_Boot_Glass_Toe";                 break;
        default:
                                            return "Null";
        break;
    }
}

//===========================================================================

char* player::GetFootfallLandSweetner( s32 Material )
{
    switch( Material )
    {        
        case MAT_TYPE_NULL:                 return "FF_Boot_Land_Sweetner_Null";                  break;
        case MAT_TYPE_EARTH:                return "FF_Boot_Land_Sweetner_Earth";                 break;
        case MAT_TYPE_ROCK:                 return "FF_Boot_Land_Sweetner_Rock";                  break;
        case MAT_TYPE_CONCRETE:             return "FF_Boot_Land_Sweetner_Concrete";              break;
        case MAT_TYPE_SOLID_METAL:          return "FF_Boot_Land_Sweetner_Metal";                 break;
        case MAT_TYPE_HOLLOW_METAL:         return "FF_Boot_Land_Sweetner_HollowMetal";           break;
        case MAT_TYPE_METAL_GRATE:          return "FF_Boot_Land_Sweetner_MetalGrate";            break;
        case MAT_TYPE_PLASTIC:              return "FF_Boot_Land_Sweetner_Plastic";               break;
        case MAT_TYPE_WATER:                return "FF_Boot_Land_Sweetner_Water";                 break;
        case MAT_TYPE_WOOD:                 return "FF_Boot_Land_Sweetner_Wood";                  break;
        case MAT_TYPE_ENERGY_FIELD:         return "FF_Boot_Land_Sweetner_EnergyField";           break;
        case MAT_TYPE_BULLET_PROOF_GLASS:   return "FF_Boot_Land_Sweetner_BulletProofGlass";      break;
        case MAT_TYPE_ICE:                  return "FF_Boot_Land_Sweetner_Ice";                   break;

        case MAT_TYPE_LEATHER:              return "FF_Boot_Land_Sweetner_Leather";               break;
        case MAT_TYPE_EXOSKELETON:          return "FF_Boot_Land_Sweetner_Exoskeleton";           break;
        case MAT_TYPE_FLESH:                return "FF_Boot_Land_Sweetner_Flesh";                 break;
        case MAT_TYPE_BLOB:                 return "FF_Boot_Land_Sweetner_Blob";                  break;
        
        case MAT_TYPE_FIRE:                 return "FF_Boot_Land_Sweetner_Fire";                  break;
        case MAT_TYPE_GHOST:                return "FF_Boot_Land_Sweetner_Ghost";                 break;
        case MAT_TYPE_FABRIC:               return "FF_Boot_Land_Sweetner_Fabric";                break;
        case MAT_TYPE_CERAMIC:              return "FF_Boot_Land_Sweetner_Ceramic";               break;
        case MAT_TYPE_WIRE_FENCE:           return "FF_Boot_Land_Sweetner_WireFence";             break;

        case MAT_TYPE_GLASS:                return "FF_Boot_Land_Sweetner_Glass";                 break;
        default:
                                            return "FF_Boot_Land_Sweetner_Null";
        break;
    }
}

//===========================================================================

char* player::GetFootfallSlide( s32 Material )
{
    switch( Material )
    {        
        case MAT_TYPE_NULL:                 return "FF_Boot_Null_Slide";                  break;
        case MAT_TYPE_EARTH:                return "FF_Boot_Earth_Slide";                 break;
        case MAT_TYPE_ROCK:                 return "FF_Boot_Rock_Slide";                  break;
        case MAT_TYPE_CONCRETE:             return "FF_Boot_Concrete_Slide";              break;
        case MAT_TYPE_SOLID_METAL:          return "FF_Boot_Metal_Slide";                 break;
        case MAT_TYPE_HOLLOW_METAL:         return "FF_Boot_HollowMetal_Slide";           break;
        case MAT_TYPE_METAL_GRATE:          return "FF_Boot_MetalGrate_Slide";            break;
        case MAT_TYPE_PLASTIC:              return "FF_Boot_Plastic_Slide";               break;
        case MAT_TYPE_WATER:                return "FF_Boot_Water_Slide";                 break;
        case MAT_TYPE_WOOD:                 return "FF_Boot_Wood_Slide";                  break;
        case MAT_TYPE_ENERGY_FIELD:         return "FF_Boot_EnergyField_Slide";           break;
        case MAT_TYPE_BULLET_PROOF_GLASS:   return "FF_Boot_BulletProofGlass_Slide";      break;
        case MAT_TYPE_ICE:                  return "FF_Boot_Ice_Slide";                   break;

        case MAT_TYPE_LEATHER:              return "FF_Boot_Leather_Slide";               break;
        case MAT_TYPE_EXOSKELETON:          return "FF_Boot_Exoskeleton_Slide";           break;
        case MAT_TYPE_FLESH:                return "FF_Boot_Flesh_Slide";                 break;
        case MAT_TYPE_BLOB:                 return "FF_Boot_Blob_Slide";                  break;
        
        case MAT_TYPE_FIRE:                 return "FF_Boot_Fire_Slide";                  break;
        case MAT_TYPE_GHOST:                return "FF_Boot_Ghost_Slide";                 break;
        case MAT_TYPE_FABRIC:               return "FF_Boot_Fabric_Slide";                break;
        case MAT_TYPE_CERAMIC:              return "FF_Boot_Ceramic_Slide";               break;
        case MAT_TYPE_WIRE_FENCE:           return "FF_Boot_WireFence_Slide";             break;

        case MAT_TYPE_GLASS:                return "FF_Boot_Glass_Slide";                 break;
        default:
                                            return "Null";
        break;
    }
}

//===========================================================================

xbool player::DoFootfallCollisions( void )
{
    if( m_Physics.GetJumpMode() || m_Physics.GetFallMode() )
    {
        return FALSE;
    }

    return TRUE;
}

//==============================================================================
xbool player::IsAnimStateAvailable2( inven_item WeaponItem, animation_state AnimState )
{
    // Get a reference to the state that we are considering
    s32 WeaponIndex = inventory2::ItemToWeaponIndex( WeaponItem );
    state_anims& State = m_Anim[WeaponIndex][AnimState];

    if( (State.nWeaponAnims > 0) && (State.nPlayerAnims > 0) )
        return TRUE;

    return FALSE;
}


//===========================================================================
#ifdef STUN_PLAYER
void player::StunPlayer( void )
{
    SetNonExclusiveState( NE_STATE_STUNNED );
}
#endif


//==============================================================================

xbool player::AddHealth( f32 DeltaHealth )
{
    // Check to see if the player took damage
    if( DeltaHealth < 0.0f )
    {
        m_LastTimeTookDamage = (f32)x_GetTimeSec();
    }

    return actor::AddHealth( DeltaHealth );
}

//==============================================================================
xbool player::RenderSniperZoom( void )
{
    return (m_CurrentWeaponItem == INVEN_WEAPON_SNIPER_RIFLE)
        && m_bActivePlayer
        && (   (m_CurrentAnimState == ANIM_STATE_ZOOM_IDLE)
            || (m_CurrentAnimState == ANIM_STATE_ZOOM_RUN)
            || (m_CurrentAnimState == ANIM_STATE_ZOOM_FIRE));
}

//=============================================================================

#ifdef X_EDITOR

void player::OnEditorRender( void )
{
    const view* ActiveView = eng_GetView();

    if ( ActiveView && ((ActiveView->GetPosition() - GetView().GetPosition()).LengthSquared() > 0.5f) )
    {
        //
        // Draw the player orientation axes
        //
        vector3 Z           ( 0.0f,    0.0f,   150.0f  );
        vector3 EyesPosition( GetPosition() + vector3( 0.0f, 172.5f, 0.0f ) + m_EyesOffset );
        matrix4 L2W;
        L2W.Identity();
        L2W.Rotate( radian3( GetPitch(), GetYaw(), 0.0f ) );
        L2W.Translate( EyesPosition );
        Z = L2W.Transform( Z );

        draw_Begin( DRAW_TRIANGLES );
        // draw a cone
        // draw each vertex in the z plane then transform it
        static const s32    nVertex    = 25;
        static const f32    Radius  = 10.0f;
        static const radian Step    = R_360 / nVertex;
        radian              Angle   = Step;
        s32                 i;
        const vector3       Center      ( L2W.Transform( vector3( 0.0f, Radius, 0.0f ) ) );
        vector3             LastVertex  ( Center );

        for ( i = 0; i < nVertex; ++i )
        {
            vector3 Vertex( 0.0f, Radius, 0.0f );
            Vertex.RotateZ( Angle );
            Vertex = L2W.Transform( Vertex );

            plane Plane( LastVertex, Vertex, Z );
            s32 ColorShade = (s32)((Plane.Normal.GetY() + 1.0f) * 127);
            draw_Color( xcolor( 0, ColorShade, ColorShade ) );
            draw_Vertex( LastVertex );
            draw_Vertex( Vertex );
            draw_Vertex( Z );

            Plane.Setup( Vertex, LastVertex, Center );
            ColorShade = (s32)((Plane.Normal.GetY() + 1.0f) * 127);
            draw_Color( xcolor( 0, ColorShade, ColorShade ) );
            draw_Vertex( Vertex );
            draw_Vertex( LastVertex );
            draw_Vertex( Center );

            LastVertex = Vertex;
            Angle += Step;
        }

        draw_End();
    }
}

#endif // X_EDITOR

//=============================================================================
xbool player::SetMutated( xbool bMutate )
{
    if( actor::SetMutated( bMutate ) )
    {
        if( bMutate )
        {
            if( (GetMutagen() <= F32_MIN) && !m_bDead )
            {
#ifdef mreed 
                LOG_MESSAGE( "Mutation", "m_bIsMutated = FALSE (SetMutated)" );
#endif
                m_bIsMutated = FALSE;

                // this failed
                return FALSE;
            }
        }
        
#ifdef mreed 
        LOG_MESSAGE( "Mutation", "m_bIsMutated = %s (SetMutated, = bMutate)", bMutate?"TRUE":"FALSE" );
#endif
        m_bIsMutated = bMutate;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//=============================================================================

void player::SetupMutationChange( xbool bMutate )
{
    //SB: This breaks forced mutations (eg. in mp infection game)
    //if( (m_CurrentAnimState == ANIM_STATE_GRENADE) ||
        //(m_CurrentAnimState == ANIM_STATE_ALT_GRENADE) ||
        //(m_CurrentAnimState == ANIM_STATE_DISCARD) )
        //return;

    xbool bIsMutated = m_bIsMutated;
    if( bIsMutated == bMutate )
        return;

//  LOG_MESSAGE( "player::SetupMutationChange", "Mutate:%d", bMutate );

    if( bMutate )
    {
        // can we mutate?
        if( SetMutated(TRUE) )
        {
            m_PreMutationWeapon2 = m_CurrentWeaponItem;
            SetNextWeapon2( INVEN_WEAPON_MUTATION );
        //  LOG_MESSAGE( "player::SetupMutationChange", "SetNextWeapon2( %s )", inventory2::ItemToName( INVEN_WEAPON_MUTATION ) );
            m_MutationAudioLoopSfx = g_AudioMgr.Play( "Mutation_Vision_Loop" );
            g_PerceptionMgr.BeginMutate();
        }

        // turn off flashlight when mutating
        SetFlashlightActive( FALSE );
    }
    else
    {
        g_PerceptionMgr.EndMutate();
        static const f32 FADE_TIME = 0.25f;
        g_AudioMgr.Release( m_MutationAudioLoopSfx, FADE_TIME );
        // see if our previous weapon is in inventory
        if( m_Inventory2.HasItem( m_PreMutationWeapon2 ) && 
            (m_PreMutationWeapon2 != INVEN_WEAPON_MUTATION) )
        {
            SetMutated(FALSE);
            SetNextWeapon2( m_PreMutationWeapon2 );
        //  LOG_MESSAGE( "player::SetupMutationChange", "SetNextWeapon2( %s )", inventory2::ItemToName( m_PreMutationWeapon2 ) );
        }
        else
        {
            // see if we have another weapon in inventory
            s32 i;
            for( i = 0; i < INVEN_NUM_WEAPONS; ++i )
            {
                if ( inventory2::WeaponIndexToItem(i) == INVEN_WEAPON_MUTATION )
                {
                    continue;
                }
                else if ( m_Inventory2.HasItem( inventory2::WeaponIndexToItem(i) ) )
                {
                    SetMutated(FALSE);
                    SetNextWeapon2( inventory2::WeaponIndexToItem(i) );
                }
            }
        }
    }
}

//=============================================================================

f32 MPMutagenBurn = 1.0f;

void player::UpdateMutagen( f32 DeltaTime )
{
    // if we are playing a cinematic, don't burn mutagen
    if( m_Cinema.m_bCinemaOn )
    {
        return;
    }

    f32 AmountToChange = 0.0f;  // If this is negative, for instance, when in mutant form in campaign mode, then it acts as a burn.
    f32 pct = 1.0f;

    switch( GetMutagenBurnMode() )
    {
    case MBM_AT_WILL:
        {
            pct = IsMutated() ? MutagenChangeMutant_AtWill_Tweak.GetF32() : MutagenChangeHuman_AtWill_Tweak.GetF32();
            pct = pct/100.0f; // make it a percentage, tweaks are whole numbers.
            AmountToChange = (pct * GetMaxMutagen() * DeltaTime);
            AmountToChange *= MPMutagenBurn;
        }
        break;

    case MBM_FORCED:
        {
            pct = IsMutated() ? MutagenChangeMutant_Forced_Tweak.GetF32() : MutagenChangeHuman_Forced_Tweak.GetF32();
            pct = pct/100.0f; // make it a percentage, tweaks are whole numbers.
            AmountToChange = (pct * GetMaxMutagen() * DeltaTime);
        }
        break;

    default:
    case MBM_NORMAL_CAMPAIGN:
        {
            pct = IsMutated() ? MutagenChangeMutant_Campaign_Tweak.GetF32() : MutagenChangeHuman_Campaign_Tweak.GetF32();
            pct = pct/100.0f; // make it a percentage, tweaks are whole numbers.
            AmountToChange = (pct * GetMaxMutagen() * DeltaTime);
        }
        break;
    }

    // this is removing mutagen
    if( AmountToChange < 0.0f )
    {
        // see if the change takes us below 0 (remember AmountToChange is negative here so we have to add)
        if( (GetMutagen() + AmountToChange) < 0.0f )
        {
            // only burn as much as we have, otherwise the sanity check in AddMutagen won't let this happen
            AmountToChange = -GetMutagen();
        }
    }

    // Don't burn mutagen if we have unlimited ammo
    if( DEBUG_INFINITE_AMMO )
    {
        // fill it up if we have infinite ammo
        AmountToChange = GetMaxMutagen();
    }

    // this will only remove mutagen if AmountToChange is negative.
    AddMutagen(AmountToChange);

    if( IsMutated() )
    {
        // We don't burn mutagen if we have unlimited ammo, so, we should never run out.
        if( !DEBUG_INFINITE_AMMO )
        {
            // check if we need to de-mutate
            if( GetMutagen() < F32_MIN )
            {
                if ( m_bInMutationTutorial )
                {
                    pain_handle PainHandle("GENERIC_LETHAL");
                    pain Pain;
                    Pain.Setup( "GENERIC_LETHAL", 0, GetPosition() );
                    Pain.SetDirectHitGuid( GetGuid() );
                    Pain.ApplyToObject( this );
                }
                else
                {
                    SetupMutationChange(FALSE);
                }
            }
        }
    }    
}

//=============================================================================
// Begin code from ghost.cpp
//==============================================================================

vector3 player::GetPositionWithOffset( eOffsetPos offset )
{
    switch( offset ) 
    {
    case OFFSET_NONE:
        return GetPosition();
        break;
    case OFFSET_CENTER:
        return GetBBox().GetCenter();
        break;
    case OFFSET_AIM_AT:
        return GetBBox().GetCenter() + GetLeanOffset();
        break;
    case OFFSET_EYES:
        return GetEyesPosition();
        break;
    case OFFSET_TOP_OF_BBOX:
        return GetPosition() + vector3( 0.0f, GetBBox().Max.GetY(), 0.0f );
        break;
    default:
        return GetPosition();
        break;
    }
}

//==============================================================================

xbool player::AddMutagen( const f32& nDeltaMutagen )
{        
    if( m_bDead ) 
    {
        return FALSE;
    }
    // do not allow Mutagen to go above max.
    else if( m_Mutagen == m_MaxMutagen && nDeltaMutagen > 0.0f )
    {
        return FALSE;
    }
    else if( (m_Mutagen + nDeltaMutagen) < 0.0f )  // does what we are using take us below 0?
    {
        // don't have enough
        return FALSE;
    }
    else if ( m_bInMutationTutorial && ((m_Mutagen + nDeltaMutagen) < 1.0f) )
    {
        return FALSE;
    }
    else if ( m_Cinema.m_bCinemaOn )
    {
        return FALSE;
    }
    else
    {
        // add/subtract mutagen
        m_Mutagen = fMin( m_Mutagen + nDeltaMutagen , m_MaxMutagen );
        m_Mutagen = fMax( m_Mutagen , 0.0f );

        return TRUE;
    }

    return FALSE;
}

//==============================================================================

const matrix4& player::GetL2W( void ) const
{
    matrix4& L2W = *(matrix4*)(&actor::GetL2W()); // de-constification
    const vector3 Pos( GetPosition() );
    L2W.Identity();
    L2W.RotateY( m_Yaw );
    L2W.Translate( Pos );
    return L2W;
}

//==============================================================================
/*
void player::OnDeath( void )
{
m_Health = m_MaxHealth;

#ifndef X_EDITOR
m_NetDirtyBits |= HEALTH_BIT;
#endif // X_EDITOR
}
*/

//==============================================================================

bbox player::GetLocalBBox( void ) const
{
    bbox BBox = m_Physics.GetBBox();

    // Take lean into account so leaning ghosts/players can be hit in MP
    f32  LeanDist = x_abs( GetLeanAmount() * 100.0f );
    BBox.Inflate( LeanDist, 0.0f, LeanDist );

    return BBox;
}

//==============================================================================

bbox player::GetColBBox( void )
{
    // Start with physics bbox
    bbox BBox = m_Physics.GetBBox();

    // Take lean into account so leaning ghosts/players can be hit in MP
    f32  LeanDist = x_abs( GetLeanAmount() * 100.0f );
    BBox.Inflate( LeanDist, 0.0f, LeanDist );

    // Convert into world space
    BBox.Transform( GetL2W() );
    return BBox;
}

//==============================================================================

void player::OnColCheck( void )
{
#ifndef X_EDITOR
    // For multi-player use bone bboxes so call base class
    if( GameMgr.IsGameMultiplayer() )
    {
        actor::OnColCheck();
        return;
    }
#endif

    vector3 Pos = GetPosition();
    vector3 Offset  ( GetLeanOffset() );
    Offset.GetY() = 0.0f;
    Pos += Offset;

    vector3 SpherePos[16];
    s32     nSpheres;
    s32     i;

    g_CollisionMgr.StartApply( GetGuid() );

    nSpheres = g_CollisionMgr.GetCylinderSpherePositions(  
        Pos,
        Pos + vector3(0,m_Physics.GetColHeight(),0),
        m_Physics.GetColRadius(),
        SpherePos,
        object::MAT_TYPE_FLESH );

    for( i=0; i<nSpheres; i++ )
    {
        g_CollisionMgr.ApplySphere( SpherePos[i], m_Physics.GetColRadius(), object::MAT_TYPE_FLESH );
    }

    g_CollisionMgr.EndApply();
}

//==============================================================================

#ifndef X_RETAIL
void player::OnColRender( xbool bRenderHigh )
{    
    (void)bRenderHigh;
    m_Physics.RenderCollision();
} 
#endif // X_RETAIL

//==============================================================================
void player::TakeFallPain( void )
{
    const f32 CurrentAltitude = GetPosition().GetY();
    const f32 FallDist = m_FellFromAltitude - CurrentAltitude;

    if( FallDist > m_SafeFallAltitude )
    {
        // Get parametric fall distance where 0=safe, 1=dead
        f32 T = x_parametric( FallDist, m_SafeFallAltitude, m_DeathFallAltitude, TRUE );

        // Build a pain event to describe damage and apply to player
        pain Pain;
        Pain.Setup(xfs("%s_FALL_DAMAGE",GetLogicalName()),0,GetPosition());
        Pain.SetCustomScalar( T );
        Pain.SetDirectHitGuid( GetGuid() );
        Pain.ApplyToObject( GetGuid() );

        // reset fall altitude
        m_FellFromAltitude = GetPosition().GetY();
    }
}

//==============================================================================

xbool player::AddItemToInventory2( inven_item Item )
{
    xbool bFirstTimePickedUp = FALSE;

    // if we're dead, don't add this to our inventory.
    if( IsDead() )
    {
        return FALSE;
    }

    // is this the first time they've picked up this weapon on this level
    if( !m_Inventory2.HasItem(Item) )
    {
        bFirstTimePickedUp = TRUE;
    }

    xbool ItemAdded = actor::AddItemToInventory2( Item );

    if( ItemAdded && inventory2::IsAWeapon( Item ) )
    {
        // Adding a weapon -> Determine if we should switch to the new one
        if( ShouldSwitchToWeapon2( Item, bFirstTimePickedUp ) )
        {
            // force the switch if this is the first time we've picked it up
            SetNextWeapon2( Item, bFirstTimePickedUp );
        }
    }
   
    return ItemAdded;
}

//==============================================================================

xbool player::RemoveItemFromInventory2( inven_item Item, xbool bRemoveAll )
{
    if( Item == m_CurrentWeaponItem )
    {
        // Removing the current weapon -> Switch to having no weapon at all.
        // NOTE:  This logic assumes that when you remove the player's current
        // weapon, they shouldn't have any weapons.
        SwitchWeapon2( INVEN_NULL );
    }

    return actor::RemoveItemFromInventory2( Item, bRemoveAll );
}

//==============================================================================
void player::ItemAcquiredMessage( inven_item Item )
{
#if !defined(X_EDITOR)    
    f32 currentTime = (f32)x_GetTimeSec();

    // make sure we don't flood the player with info.
    if( (currentTime - m_fLastItemAcquiredTime) > Item_Acquired_Msg_DelayTimeTweak.GetF32() )
    {
        // tell the player that they picked up something.
        MsgMgr.Message( MSG_ACQUIRED_ITEM, net_GetSlot(), Item );

        // reset time
        m_fLastItemAcquiredTime = currentTime;
    }
#endif
}

//==============================================================================

void player::ItemFullMessage( inven_item Item )
{
#if !defined(X_EDITOR)
    f32 currentTime = (f32)x_GetTimeSec();

    // make sure we don't flood the player with info.
    if( (currentTime - m_fLastItemFullTime) > Item_Full_Msg_DelayTimeTweak.GetF32() )
    {
        // tell the player that they can't carry anymore of these.
        MsgMgr.Message( MSG_FULL_ITEM, net_GetSlot(), Item );

        // reset time
        m_fLastItemFullTime = currentTime;
    }
#endif
}

//==============================================================================

void player::NoWeapon_NoAmmoPickupMessage( inven_item Item )
{
#if !defined(X_EDITOR)
    f32 currentTime = (f32)x_GetTimeSec();

    // make sure we don't flood the player with info.
    if( (currentTime - m_fLastItemFullTime) > Item_Full_Msg_DelayTimeTweak.GetF32() )
    {
        // tell the player that they can't carry anymore of these.
        MsgMgr.Message( MSG_NO_ITEM_AMMO_FAIL, net_GetSlot(), Item );

        // reset time
        m_fLastItemFullTime = currentTime;
    }
#endif    
}

//==============================================================================
xbool player::CanTakePickup( pickup& Pickup )
{
//  LOG_MESSAGE( "player::CanTakePickup", "" );

    if( m_bDead )
    {
        LOG_WARNING( "player::CanTakePickup", "Dead player trying to take a pickup!" );
        return FALSE;
    }
    
    if( !Pickup.GetTakeable() )
    {
        return FALSE;
    }   

    inven_item Item = Pickup.GetItem();

    // **HEALTH**
    if( Item == INVEN_HEALTH )
    {
        return( GetHealth() < 100.0f );
    }

    // **MUTAGEN**
    if( Item == INVEN_MUTAGEN )
    {
        return( GetMutagen() < 100.0f );
    }

    // **WEAPONS**
    // The only time you CAN'T take a weapon pickup is when you already have
    // the weapon AND you already have the maximum ammo allowed.
    if( IN_RANGE( INVEN_WEAPON_FIRST, Item, INVEN_WEAPON_LAST ) )
    {
        #ifndef X_EDITOR
        // Special case: In multiplayer, you can't take a weapon pickup if you
        // are currently mutated and (a) you already have the weapon, or (b)
        // you can't un-mutate.
        if( GameMgr.GetGameType() != GAME_CAMPAIGN )
        {
            if( m_bIsMutated && 
                    ((m_Inventory2.GetAmount( Item ) > 0) ||
                     (!m_bCanToggleMutation)) )
            {
                return( FALSE );
            }
        }
        #endif

        if( m_Inventory2.GetAmount( Item ) < m_Inventory2.GetMaxAmount( Item ) )
        {   
            return( TRUE );
        }

        new_weapon* pWeapon = GetWeaponPtr( Item );

        if( ( (Item == INVEN_WEAPON_SMP)          && (m_Inventory2.GetAmount( Item ) == 1.0f) ) ||            
            ( (Item == INVEN_WEAPON_SHOTGUN)      && (m_Inventory2.GetAmount( Item ) == 1.0f) ) ||
            ( (Item == INVEN_WEAPON_DESERT_EAGLE) && (m_Inventory2.GetAmount( Item ) == 1.0f) ) )
        {
            // get the dual weapon
            inven_item DualItem = new_weapon::GetDualWeaponID( Item );
            new_weapon* pDualWeapon = GetWeaponPtr( DualItem );

            // make sure the dual weapon is valid and that we already have the dual in our inventory.
            if( pDualWeapon && m_Inventory2.GetAmount( DualItem ) == 1.0f )
            {
                s32 Current = pDualWeapon->GetTotalPrimaryAmmo();
                s32 Limit   = pDualWeapon->GetMaxPrimaryAmmo();

                // full of ammo, don't get it.
                if( Current >= Limit )
                {
                    // don't pick it up
                    return FALSE;
                }
            }

            // dual weapon and we're missing some ammo or we haven't gotten a dual yet... get it.
            return TRUE;
        }
        
        if( pWeapon )
        {
            s32 Current = pWeapon->GetTotalPrimaryAmmo();
            s32 Limit   = pWeapon->GetMaxPrimaryAmmo();

            // never pickup a BBG for AMMO
            if( Item == INVEN_WEAPON_BBG )
            {
                return FALSE;
            }
            else
            if( (Current < Limit) )
            {
                // pick it up
                return TRUE;
            }
            else
            {
                // ammo full, don't pick it up and notify weapon for message
                pWeapon->NotifyAmmoFull(this);
                return FALSE;
            }
        }
        else
        {
            ASSERT( FALSE );    // We should have a weapon.
            return( FALSE );
        }
    }

    // Special case: In multiplayer, when mutated, you cannot take ammo pickups
    // (which includes grenades).
    #ifndef X_EDITOR
    if( GameMgr.GetGameType() != GAME_CAMPAIGN )
    {
        if( m_bIsMutated )
        {
            return( FALSE );
        }
    }
    #endif

    // **AMMO**
    // The only time you CAN'T take an ammo pickup is when you already have the
    // maximum ammo allowed.
    if( IN_RANGE( INVEN_AMMO_FIRST, Item, INVEN_AMMO_LAST ) )
    {
        // BBG ammo is just for debugging and such, NEVER pick it up
        if( Item == INVEN_AMMO_BBG )
        {
            ASSERTS(0, "You should not be placing BBG ammo, it is for debugging only" );
            return FALSE;
        }

        inven_item  Weapon  = m_Inventory2.AmmoToWeapon( Item );
        new_weapon* pWeapon = GetWeaponPtr( Weapon );
        if( pWeapon )
        {
            if( m_Inventory2.HasItem( Weapon ) )
            {
                s32 Current = pWeapon->GetTotalPrimaryAmmo();
                s32 Limit   = pWeapon->GetMaxPrimaryAmmo();

                if( Current < Limit )
                {
                    // pick it up
                    return TRUE;
                }
                else
                {
                    // ammo full, don't pick it up and notify weapon for message
                    pWeapon->NotifyAmmoFull(this);
                    return FALSE;
                }
            }
            else
            {
                // weapon hasn't been gotten yet, don't pick up ammo.
                NoWeapon_NoAmmoPickupMessage( Item );

                return( FALSE );
            }
        }
        else
        {
            ASSERT( FALSE );    // We should have a weapon.
            return( FALSE );
        }
    }

    // **EVERYTHING ELSE**
    // Otherwise, just check the limit within the inventory.
    if( m_Inventory2.CanHoldMore( Item ) )
    {
        return TRUE;
    }
    else
    {
        /* REMOVED because we couldn't get the German translation to work.
        ItemFullMessage( Item );
        */
        return FALSE;
    }
}    

//==============================================================================

void player::TakePickup( pickup& Pickup )
{
//  LOG_MESSAGE( "player::TakePickup", "" );

    ItemAcquiredMessage(Pickup.GetItem());

    if( m_bDead )
    {
        LOG_WARNING( "player::TakePickup", "A dead player got a pickup!" );
        return;
    }

    inven_item Item   =      Pickup.GetItem();
    s32        Amount = (s32)Pickup.GetAmount();

#ifndef X_EDITOR
    if( !g_NetworkMgr.IsServer() )
    {
        if( (Item == INVEN_WEAPON_MESON_CANNON) ||
            (Item == INVEN_AMMO_MESON) )
        {
            DirtyAmmo();
        }
    }
#endif

    // **WEAPONS**
    // The only time you CAN'T take a weapon pickup is when you already have
    // the weapon AND you already have the maximum ammo allowed.
    if( IN_RANGE( INVEN_WEAPON_FIRST, Item, INVEN_WEAPON_LAST ) )
    {
        if( (Item == INVEN_WEAPON_SMP) &&
            (m_Inventory2.GetAmount( Item ) == 1.0f) )
        {
            Item = INVEN_WEAPON_DUAL_SMP;
        }
        else
        if( (Item == INVEN_WEAPON_SHOTGUN) &&
                (m_Inventory2.GetAmount( Item ) == 1.0f) )
        {
            Item = INVEN_WEAPON_DUAL_SHT;
        }
        /*** Uncomment this code to support dual desert eagles. 
        if( (Item == INVEN_WEAPON_DESERT_EAGLE) &&
            (m_Inventory2.GetAmount( Item ) == 1.0f) )
        {
            Item = INVEN_WEAPON_DUAL_EAGLE;
        }
        ***/

        xbool bFirstTimePickedup = FALSE;
        
        // Take the weapon if there is space.
        if( m_Inventory2.GetAmount( Item ) < 1.0f )
        {
            // don't have this item, it's the first time we've grabbed it.
            bFirstTimePickedup = TRUE;

            m_Inventory2.SetAmount( Item, 1.0f );

            // set flag for dual weapon
            xbool bIsDual = (Item == INVEN_WEAPON_DUAL_SMP) || (Item == INVEN_WEAPON_DUAL_SHT ); //|| (Item == INVEN_WEAPON_DUAL_EAGLE );

            // we currently don't have one of these in our inventory
            if( ShouldSwitchToWeapon2( Item, TRUE ) )
            {
                // don't do state change if this is a dual weapon, we'll do it below
                SetNextWeapon2( Item, FALSE, !bIsDual );

                // override animation for dual pickup
                if( bIsDual )
                {
                    SetAnimState( ANIM_STATE_PICKUP );
                }
            }
            else
            {
                // be sure we change the pre-mutation weapon if we pick up a dual
                if( IsMutated() )
                {
                     // set dual as premutation weapon
                     if( bIsDual )
                     {
                         m_PreMutationWeapon2 = Item; 
                     }
                }
            }
        }

        new_weapon* pWeapon = GetWeaponPtr( Item );

        // Take the clip of ammo in the weapon if possible.
        {
            // set up dual weapons specially.
            if( pWeapon )
            {
                inven_item ParentItem = new_weapon::GetParentIDForDualWeapon( Item );

                // make sure the clip is full!
                if( ParentItem != INVEN_NULL )
                {
                    // dual weapon's clip is full... give ammo to parent weapon
                    if( (pWeapon->GetAmmoCount() >= pWeapon->GetAmmoPerClip()) && !bFirstTimePickedup )
                    {
                        new_weapon *pParentWeapon = GetWeaponPtr( ParentItem );

                        // add the ammo to the weapon
                        s32 GunAmmo = pParentWeapon->GetAmmoPerClip( new_weapon::AMMO_PRIMARY );

                        // add ammo to parent item's reserves
                        AddAmmo2( ParentItem, GunAmmo );

                        // make counts match up
                        pWeapon->SetupDualAmmo( ParentItem );
                    }
                    else
                    {
                        // we ran over a dual capable weapon, put the ammo in the dual clip.
                        pWeapon->SetupDualAmmo( ParentItem );
                    }
                }               
                else
                {
                    // add the ammo to the weapon
                    s32 GunAmmo = pWeapon->GetAmmoPerClip( new_weapon::AMMO_PRIMARY );

#ifndef X_EDITOR
                    // KSS -- FIXME -- HACK FOR LAN PARTY
                    if( GameMgr.IsGameMultiplayer() )
                    {
                        GunAmmo = pWeapon->m_WeaponAmmo[new_weapon::AMMO_PRIMARY].m_AmmoMax;
                    }
#endif              
                    // we ran over a dual capable weapon, put the ammo in the dual clip.
                    pWeapon->AddAmmoToWeapon( GunAmmo, 0 );                    

                    // if you've died and picked up the weapon again, reload it
                    if( bFirstTimePickedup )
                    {
                        s32 count = pWeapon->GetAmmoCount();
                        if( count <= 0 )
                        {                            
                            // reload without anim                            
                            pWeapon->RefillClip(new_weapon::AMMO_PRIMARY);

                            return;
                        }
                    }
                }

                // be sure we play reload anim if gun is completely empty
                s32 count = pWeapon->GetAmmoCount();
                if( count <= 0 )
                {
                    // reload with anim
                    ReloadWeapon(new_weapon::AMMO_PRIMARY);
                }
            }
            else
            {
                ASSERTS(FALSE, xfs("Weapon %s missing from blueprint bag", GetInventory2().ItemToName(Item)) );
            }
        }

        return;
    }

    // **EVERYTHING ELSE**
    switch( Item )
    {
    case INVEN_HEALTH:
        m_Inventory2.SetAmount( Item, 0.0f );
        #ifndef X_EDITOR
        if( g_NetworkMgr.IsServer() )
        #endif
        {
            AddHealth( (f32)Amount );
        }
        break;

    case INVEN_MUTAGEN:
    case INVEN_MUTAGEN_CORPSE:
        m_Inventory2.SetAmount( Item, 0.0f );
        AddMutagen( (f32)Amount );
        break;

    case INVEN_AMMO_SMP:
        AddAmmo2( INVEN_WEAPON_SMP, Amount );
        break;
    case INVEN_AMMO_SHOTGUN:
        AddAmmo2( INVEN_WEAPON_SHOTGUN, Amount );
        break;
    case INVEN_AMMO_DESERT_EAGLE:
        AddAmmo2( INVEN_WEAPON_DESERT_EAGLE, Amount );
        break;
    case INVEN_AMMO_SNIPER_RIFLE:
        AddAmmo2( INVEN_WEAPON_SNIPER_RIFLE, Amount );
        break;
    case INVEN_AMMO_MESON:
        AddAmmo2( INVEN_WEAPON_MESON_CANNON, Amount );
        break;

    case INVEN_AMMO_BBG:
        AddAmmo2( INVEN_WEAPON_BBG, Amount );
        break;

    // TODO - Handle ammo boxes (which have multiple ammo types within).

    default:
        // Should this be here?  Should we just ASSERT( FALSE )?
        m_Inventory2.AddAmount( Item, (f32)Amount );
        break;
    }
}

//==============================================================================

xbool player::OnPickup( pickup& Pickup )
{
//  LOG_MESSAGE( "player::OnPickup", "" );

    // On the server:
    //  - First see if you want the pickup.
    //  - If so, take it.
    //
    // On a client:
    //  - First see if you want the pickup.
    //  - If so, tell the server "I want this pickup".
    //  - Server: If pickup not already taken, "You can have it".
    //  - Client: Take it.
    //  + This is further complicated by health pickups which must be handled
    //    by the ghost on the server side.

    if( !CanTakePickup( Pickup ) )
        return( FALSE );

#ifndef X_EDITOR
    if( g_NetworkMgr.IsServer() )
#endif
    {
        TakePickup( Pickup );
        return( TRUE );
    }
#ifndef X_EDITOR
    else
    {
        ASSERT( g_NetworkMgr.IsClient() );
        net_WantPickup( Pickup );
        return( TRUE );
    }
#endif

    // Should never be able to get here, but added the following line to keep
    // the compiler happy.  (An unhappy compiler is a bad thing!)
    return( FALSE );
}

//==============================================================================
s32 GetStartWeaponsForLevel( const char* pLevelName )
{
    // KSS -- TO ADD NEW WEAPON
    // Determine the weapons a player start with and the start item
    s32         Weapons     = WB_SMP | WB_DE | WB_SG | WB_SR | WB_MC | WB_MM | WB_MP | WB_FG | WB_BBG | WB_JBG | WB_SCN | WB_MS;
    for( s32 i=0; s_MapToWeaponTable[i].pLevelName ; i++ )
    {
        if( x_stristr( pLevelName, s_MapToWeaponTable[i].pLevelName ) )
        {
            Weapons     = s_MapToWeaponTable[i].StartWeapons;
            break;
        }
    }

    return Weapons;
}

//==============================================================================

#ifndef X_RETAIL

s32 GetMemoryBallastForLevel( const char* pLevelName )
{
    // KSS -- TO ADD NEW WEAPON
    // Determine the amount of memory to withold.
    s32 Ballast = 0;
    for( s32 i=0; s_MapToWeaponTable[i].pLevelName ; i++ )
    {
        if( x_stristr( pLevelName, s_MapToWeaponTable[i].pLevelName ) )
        {
            Ballast = s_MapToWeaponTable[i].MemoryBallast;
            break;
        }
    }

    return Ballast;
}

#endif

//==============================================================================
#if defined( X_EDITOR )
static s32 GetAvailableWeaponsForLevel( const char* pLevelName )
{
    // KSS -- TO ADD NEW WEAPON
    // Determine the weapons a player start with and the start item
    s32 Weapons = WB_SMP | WB_DE | WB_SG | WB_SR | WB_MC | WB_MM | WB_MP | WB_FG | WB_BBG | WB_JBG | WB_SCN;
    for( s32 i=0 ; s_MapToWeaponTable[i].pLevelName ; i++ )
    {
        if( x_stristr( pLevelName, s_MapToWeaponTable[i].pLevelName ) )
        {
            Weapons     = s_MapToWeaponTable[i].AvailableWeapons;
            break;
        }
    }

    return Weapons;
}
#endif
//==============================================================================

inven_item GetEquipedWeaponForLevel( const char* pLevelName )
{
    // Determine the weapons a player start with and the start item
    inven_item  StartItem   = INVEN_WEAPON_SMP;
    for( s32 i=0; s_MapToWeaponTable[i].pLevelName ; i++ )
    {
        if( x_stristr( pLevelName, s_MapToWeaponTable[i].pLevelName ) )
        {
            StartItem   = s_MapToWeaponTable[i].StartItem;
            break;
        }
    }

    return StartItem;
}

//==============================================================================

//==============================================================================
void player::DebugSetupInventory( const char* pLevelName )
{
    // KSS -- TO ADD NEW WEAPON
    s32 Weapons = GetStartWeaponsForLevel( pLevelName );

    // Give the player some weapons
    if( Weapons & WB_SMP )
        m_Inventory2.SetAmount( INVEN_WEAPON_SMP, 1.0f );
    if( Weapons & WB_SG )
        m_Inventory2.SetAmount( INVEN_WEAPON_SHOTGUN, 1.0f );
    if( Weapons & WB_DE )
        m_Inventory2.SetAmount( INVEN_WEAPON_DESERT_EAGLE, 1.0f );
    if( Weapons & WB_SR )
        m_Inventory2.SetAmount( INVEN_WEAPON_SNIPER_RIFLE, 1.0f );
    if( Weapons & WB_MC )
        m_Inventory2.SetAmount( INVEN_WEAPON_MESON_CANNON, 1.0f );
    if( Weapons & WB_BBG )
        m_Inventory2.SetAmount( INVEN_WEAPON_BBG, 1.0f );
    if( (Weapons & WB_MM) || (Weapons & WB_MP) )
        m_Inventory2.SetAmount( INVEN_WEAPON_MUTATION, 1.0f );
    if( Weapons & WB_FG )
        m_Inventory2.SetAmount( INVEN_GRENADE_FRAG, 5.0f );
    if( Weapons & WB_JBG )
        m_Inventory2.SetAmount( INVEN_GRENADE_JBEAN, 5.0f );
    if( Weapons & WB_SCN )
        m_Inventory2.SetAmount( INVEN_WEAPON_SCANNER, 1.0f );

    // mreed: You'll only have gloves until you mutate. The final solution is to make sure the player
    // has goves in the first level. I'm giving the player gloves all the time here so that we can 
    // see the effect of having them, then losing them from mutation.
    if( !(Weapons & WB_MM) && !(Weapons & WB_MP) )
        m_Inventory2.SetAmount( INVEN_GLOVES, 1.0f ); 

    m_CurrentWeaponItem = GetEquipedWeaponForLevel( pLevelName );
}
#ifdef X_EDITOR

void player::EditorPreGame( void )
{
    actor::EditorPreGame();

    CLOG_MESSAGE( LOGGING_ENABLED, xfs("player::EditorPreGame %08x",(u32)this), "Start creating templates" );

    // Clear weapon guids
    x_memset( &m_WeaponGuids, 0, sizeof(m_WeaponGuids) );

    // Clear inventory
    m_Inventory2.Clear();

    s32 WeaponsForLevel = GetAvailableWeaponsForLevel( (const char*)g_Project.m_DFSName );

    // Here is where we create the inventory items that are specified for each inventory in the editor.
    for( s32 i=0; i<INVEN_NUM_WEAPONS; i++ )
    {
        inven_item WeaponItem = inventory2::WeaponIndexToItem(i);

        xbool DoCreateWeapon = TRUE;

        switch( WeaponItem )
        {
        case INVEN_WEAPON_SMP:
            DoCreateWeapon = WeaponsForLevel & WB_SMP;
            break;
        case INVEN_WEAPON_SHOTGUN:
            DoCreateWeapon = WeaponsForLevel & WB_SG;
            break;
        case INVEN_WEAPON_SNIPER_RIFLE:
            DoCreateWeapon = WeaponsForLevel & WB_SR;
            break;
        case INVEN_WEAPON_DESERT_EAGLE:
            DoCreateWeapon = WeaponsForLevel & WB_DE;
            break;
        case INVEN_WEAPON_MESON_CANNON:
            DoCreateWeapon = WeaponsForLevel & WB_MC;
            break;
        case INVEN_WEAPON_DUAL_SMP:
            DoCreateWeapon = WeaponsForLevel & WB_SMP;
            break;

        case INVEN_WEAPON_DUAL_SHT:
            DoCreateWeapon = WeaponsForLevel & WB_SG;
            break;

        // KSS -- TO ADD NEW WEAPON
        case INVEN_WEAPON_BBG:
            DoCreateWeapon = WeaponsForLevel & WB_BBG;
            break;

        case INVEN_WEAPON_SCANNER:
            DoCreateWeapon = WeaponsForLevel & WB_SCN;
            break;

        case INVEN_WEAPON_DUAL_EAGLE:
            DoCreateWeapon = WeaponsForLevel & WB_DE;
            break;
        case INVEN_WEAPON_MUTATION:
            DoCreateWeapon = WeaponsForLevel & WB_MM;
            break;
        }

        if( DoCreateWeapon )
        {
            const char* pBlueprintName = inventory2::ItemToBlueprintName( WeaponItem );
            if( pBlueprintName )
            {
                guid Guid = g_TemplateMgr.EditorCreateSingleTemplateFromPath( pBlueprintName, vector3(0,0,0), radian3(0,0,0), -1, -1 ); 

                CLOG_MESSAGE( LOGGING_ENABLED, xfs("player::EditorPreGame %08x",(u32)this), "EditorCreateSingleTemplateFromPath '%s' %08x:%08x", pBlueprintName, Guid.GetHigh(), Guid.GetLow() );

                if( Guid )
                {
                    new_weapon* pWeapon = (new_weapon*)g_ObjMgr.GetObjectByGuid( Guid );
                    ASSERT( pWeapon );
                    pWeapon->InitWeapon( GetPosition(), new_weapon::RENDER_STATE_PLAYER, GetGuid() );
                    OnWeaponAnimInit2( WeaponItem, pWeapon );
                    pWeapon->SetupRenderInformation( );
                    pWeapon->SetAnimation( ANIM_STATE_IDLE, 0.0f );
                    pWeapon->BeginIdle();
                }

                m_WeaponGuids[i] = Guid;
            }
        }
        else
        {
            m_WeaponGuids[i] = 0;
        }
    }

    CLOG_MESSAGE( LOGGING_ENABLED, xfs("player::EditorPreGame %08x",(u32)this), "Done creating templates" );

    DebugEnableWeapons( (const char*)g_Project.m_DFSName );

    m_WeaponsCreated = TRUE;
}

#endif // X_EDITOR

//==============================================================================

void player:: ReInitInventory( void )
{
    ASSERT( m_WeaponsCreated );
    if( m_WeaponsCreated )
    {
        for( s32 i=0; i<INVEN_NUM_WEAPONS; i++ )
        {
            // Nuke the object, if it exists!
            new_weapon* pWeapon = (new_weapon*)g_ObjMgr.GetObjectByGuid( m_WeaponGuids[i] );
            if( pWeapon )
                g_ObjMgr.DestroyObjectEx( m_WeaponGuids[i], TRUE );
            m_WeaponGuids[i] = 0;
        }
        m_WeaponsCreated = FALSE;

        FXMgr.EndOfFrame(); // Call FXMgr EndOfFrame to flush any deferred deletes - must be after ObjMgr.Clear
        FXMgr.EndOfFrame();
        FXMgr.EndOfFrame();
        FXMgr.EndOfFrame();
    }

    // Now init the inventory.
    InitInventory();
}

//==============================================================================

void player::InitInventory( void )
{
    if( !m_WeaponsCreated )
    {
        CreateAllWeaponObjects();
        m_WeaponsCreated = TRUE;
#ifdef X_EDITOR
        DebugEnableWeapons( "<null>" );
#else
    #if (!CONFIG_IS_DEMO)
        if( g_StateMgr.UseDefaultLoadOut() )
    #endif
        {
            // Clear inventory
            m_Inventory2.Clear();
            DebugEnableWeapons( g_ActiveConfig.GetLevelPath() );
            g_StateMgr.DisableDefaultLoadOut();
        }
#endif
    }
}

//==============================================================================
//==============================================================================

view& player::GetView( s32 Player ) 
{ 
#ifndef X_EDITOR
    ASSERT( (Player < MAX_LOCAL_PLAYERS) && (Player >= 0) ); 
#endif
    return m_Views[Player]; 
}

//==============================================================================

view& player::GetView( void ) 
{ 
#ifdef X_EDITOR
    return GetView( 0 ); 
#else
    return GetView( GetLocalSlot() ); 
#endif
}

//==============================================================================

xbool player::IsAvatar( void )
{
    xbool bRenderAvatar = !m_bActivePlayer
                          || (m_CurrentAnimState == ANIM_STATE_FALLING_TO_DEATH)
                          || (m_LocalSlot == -1);

#if defined( TARGET_PS2 ) && !defined( CONFIG_RETAIL )
    bRenderAvatar = bRenderAvatar;
#endif

#if defined(X_EDITOR)
    const view* ActiveView = eng_GetView();
    if ( ActiveView && ((ActiveView->GetPosition() - GetView().GetPosition()).LengthSquared() > 0.5f) )
    {
        bRenderAvatar = TRUE;
    }
#endif // X_EDITOR

    return bRenderAvatar;
}

//==============================================================================

xbool player::UsingLoco( void )
{
#if defined( X_EDITOR )
    return( FALSE );
#else
    // SB: Use loco so skin geom is setup for bbox collision detection in MP
    return ( GameMgr.IsGameMultiplayer() );
#endif
}

//=========================================================================
void player::UpdateConvulsion( void )
{
    // mreed: no convulsions ever
    if ( FALSE && !m_bInMutationTutorial && m_bIsMutated )
    {
        m_ConvulsionInfo.m_TimeSinceLastConvulsion += m_DeltaTime;

        if ( m_ConvulsionInfo.m_bConvulsingNow )
        {
            m_ConvulsionInfo.m_TimeLeftInThisConvulsion -= m_DeltaTime;
            
            // see if we should end the convulsion
            if ( m_ConvulsionInfo.m_TimeLeftInThisConvulsion <= 0.0f )
            {
                // we're done
                m_ConvulsionInfo.m_bConvulsingNow = FALSE;
            }
        }
        else
        {
            if ( m_ConvulsionInfo.m_TimeSinceLastConvulsion >= m_ConvulsionInfo.m_ConvulseAtTime )
            {
                // Time to convulse
                m_ConvulsionInfo.m_bConvulsingNow           = TRUE;
                m_ConvulsionInfo.m_TimeSinceLastConvulsion  = 0.0f;

                // reset our time for our next convulsion
                m_ConvulsionInfo.m_ConvulseAtTime
                    = m_Mutagen * g_ConvulsionTweaks.m_MutagenConvulsionMultiplierPeriod;
                m_ConvulsionInfo.m_ConvulseAtTime
                    = MAX( g_ConvulsionTweaks.m_MinConvulsionPeriod, m_ConvulsionInfo.m_ConvulseAtTime );

                // Play audio
                g_AudioMgr.Play( "SNI_Fire", GetPosition(), GetZone1(), TRUE );

                // Start rumble
                DoFeedback( m_ConvulsionFeedbackDuration, m_ConvulsionFeedbackIntensity );

                // blur
                xcolor Color( 
                    Mutagen_Convulsion_Color_R.GetS32(), 
                    Mutagen_Convulsion_Color_G.GetS32(), 
                    Mutagen_Convulsion_Color_B.GetS32() );

                g_PostEffectMgr.StartPainBlur( GetLocalSlot(), 20.0f, Color );
            }
        }
    }
}

//=========================================================================

f32 ComputeSoftLean( f32 LeanAmount )
{
    LeanAmount = MAX( -1.0f, LeanAmount );
    LeanAmount = MIN(  1.0f, LeanAmount );

    const f32 Sign      = (LeanAmount > 0.0f) ? 1.0f : (LeanAmount < 0.0f) ? -1.0f : 0;
    const f32 Period    = x_abs( LeanAmount ) * PI;
    const f32 Phase     = PI / 2.0f;

    return( Sign * (1.0f - ((x_sin( Period + Phase ) + 1.0f) / 2.0f)) );
}

//=========================================================================

f32 ComputeLean( f32 SoftLeanAmount )
{
    SoftLeanAmount = MAX( -1.0f, SoftLeanAmount );
    SoftLeanAmount = MIN(  1.0f, SoftLeanAmount );

    const f32 LeanAmount = x_abs( (x_asin( ((x_abs( SoftLeanAmount ) - 1.0f) * -2.0f) - 1.0f ) - (PI / 2.0f)) / PI );
    const f32 Sign = (SoftLeanAmount >= 0.0f) ? 1.0f : -1.0f;

    return Sign * LeanAmount;
}

//=========================================================================

f32 ComputeLeanValueForPosition( const vector3& Start, const vector3& HitPoint, xbool bLeaningRight )
{
    vector3 Lean( HitPoint - Start );
    Lean.GetY() = 0.0f;

    f32 LeanAmount = (Lean / GetTweakF32( "LeanX" )).Length();
    if ( bLeaningRight )
    {
        LeanAmount *= -1.0f;
    }

    return LeanAmount;
}

//=========================================================================

void player::UpdateLean( f32 LeanValue )
{
    const lean_state OldLeanState      = m_LeanState;
    const f32        OldLeanAmount     = m_LeanAmount;
    const f32        OldSoftLeanAmount = m_SoftLeanAmount;

    if ( (LeanValue > GetTweakF32( "LeanThreshold" )) && (m_LeanAmount >= 0.0f) )
    {
        // Leaning left
        const f32 ElapsedTime       = m_LeanAmount * GetTweakF32( "LeanTime" );
        const f32 NewElapsedTime    = ElapsedTime + m_DeltaTime;
        m_LeanAmount                = NewElapsedTime / GetTweakF32( "LeanTime" );
        m_LeanState                 = LEAN_LEFT;
    }
    else if ( (LeanValue < -GetTweakF32( "LeanThreshold" )) && (m_LeanAmount <= 0.0f) )
    {
        // Leaning right
        const f32 ElapsedTime       = (-m_LeanAmount) * GetTweakF32( "LeanTime" );
        const f32 NewElapsedTime    = ElapsedTime + m_DeltaTime;
        m_LeanAmount                = -(NewElapsedTime / GetTweakF32( "LeanTime" ));
        m_LeanState                 = LEAN_RIGHT;
    }
    else
    {
        // Returning to upright
        if ( m_LeanAmount > 0.0f )
        {
            // from left
            const f32 ElapsedTime       = m_LeanAmount * GetTweakF32( "LeanTime" );
            const f32 NewElapsedTime    = ElapsedTime - m_DeltaTime;
            m_LeanAmount                = NewElapsedTime / GetTweakF32( "LeanTime" );
            m_LeanAmount                = MAX( 0.0f, m_LeanAmount );
            m_LeanState                 = LEAN_RETURN_FROM_LEFT;
        }
        else if ( m_LeanAmount < 0.0f )
        {
            // from right
            const f32 ElapsedTime       = (-m_LeanAmount) * GetTweakF32( "LeanTime" );
            const f32 NewElapsedTime    = ElapsedTime - m_DeltaTime;
            m_LeanAmount                = -(NewElapsedTime / GetTweakF32( "LeanTime" ));
            m_LeanAmount                = MIN( 0.0f, m_LeanAmount );
            m_LeanState                 = LEAN_RETURN_FROM_RIGHT;
        }
        else
        {
            m_LeanState = LEAN_NONE;
        }

    }
    m_LeanAmount = MAX( -1.0f, m_LeanAmount );
    m_LeanAmount = MIN( 1.0f, m_LeanAmount );

    m_SoftLeanAmount = ComputeSoftLean( m_LeanAmount );

    // check to make sure we aren't violating any collision
    if ( (m_LeanState == LEAN_RIGHT) || (m_LeanState == LEAN_LEFT) )
    {
        // New position
        vector3 StartPos( GetPosition() );
        vector3 Offset  ( GetAnimPlayerOffset() );
        Offset.GetY() = 0.0f;
        vector3 EndPos  ( GetPosition() + Offset );

        //
        // We need to elevate our collision check to account for uneven floors
        // that cause us to collide at our feet, when we're mostly just
        // concerned about the player's head and weapon
        //
        static f32 TweakLeanElevate = 10.0f;
        StartPos.GetY() += TweakLeanElevate;
        EndPos.GetY() += TweakLeanElevate;

        const vector3 Delta( EndPos - StartPos );

        if ( Delta.LengthSquared() > ZERO )
        {
            m_Physics.SetupPlayerCollisionCheck( StartPos, EndPos );
            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                                            object::ATTR_BLOCKS_PLAYER_LOS, 
                                            object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING );

            if ( g_CollisionMgr.m_nCollisions > 0 )
            {
                vector3 StopPos = StartPos + (Delta * g_CollisionMgr.m_Collisions[0].T);
                m_SoftLeanAmount = ComputeLeanValueForPosition( StartPos, StopPos, m_LeanState == LEAN_RIGHT );
                m_LeanAmount = ComputeLean( m_SoftLeanAmount );
            }
        }
    }

#ifndef X_EDITOR
    // Send across net?    
    if(     ( OldLeanState      != m_LeanState      )
        ||  ( OldLeanAmount     != m_LeanAmount     )
        ||  ( OldSoftLeanAmount != m_SoftLeanAmount ) )
    {
        m_NetDirtyBits |= LEAN_BIT;  // NETWORK
    }        
#endif // X_EDITOR
    
}

//=========================================================================

vector3 player::GetLeanOffset( void )
{
    vector3 Forward ( radian3( 0.0f, m_Yaw, 0.0f ) );
    vector3 Lean    ( 0.0f, 0.0f, 0.0f );

    if ( m_SoftLeanAmount != 0.0f )
    {
        Lean = Forward;
        Lean.RotateY( R_90 );
        Lean *= (m_SoftLeanAmount * GetTweakF32( "LeanX" )); // Scale according to our horiz offset

        f32 Vertical = (PI / 2) * x_abs( m_SoftLeanAmount) * GetTweakF32( "LeanY" );

        Lean += vector3( 0.0f, -Vertical, 0.0f );
    }

    return Lean;
}

//=========================================================================

void player::UpdateArmsOffsetForLean( void )
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if ( pWeapon )
    {
        //const bbox BBox( pWeapon->GetBBox() );
        const vector3 CenterPos( pWeapon->GetBBox().GetCenter() );
        radian        Pitch, Yaw;
        GetView().GetPitchYaw( Pitch, Yaw );
        const vector3 LookDir( radian3( Pitch, Yaw, 0.0f ) );
        const vector3 ViewPos( GetView().GetPosition() - (LookDir * 10000.0f) );
        const vector3 ViewEnd( ViewPos + (LookDir * 10000.0f) );
        vector3 Closest( CenterPos.GetClosestPToLSeg( ViewPos, ViewEnd ) );
        const f32 DistanceThisFrame = GetTweakF32( "LeanWeaponOffsetSpeed" ) * m_DeltaTime;

        switch ( m_LeanState )
        {
        case LEAN_LEFT:
        case LEAN_RIGHT:
            {
                // Move the rig under our look direction
                vector3 Dir = Closest - CenterPos;
                const f32 TotalDistanceSquared = Dir.LengthSquared();
                if ( x_sqr( DistanceThisFrame ) >= TotalDistanceSquared )
                {
                    m_LeanWeaponOffset += Dir; // Just go the remaining distance
                }
                else
                {
                    Dir.Normalize();
                    Dir *= (m_LeanWeaponOffset.Length() + DistanceThisFrame);
                    m_LeanWeaponOffset = Dir;
                }
            }
            break;

        case LEAN_NONE:
        case LEAN_RETURN_FROM_LEFT:
        case LEAN_RETURN_FROM_RIGHT:
            {
                vector3 Dir = -m_LeanWeaponOffset;
                // Move the rig back
                if ( x_sqr( DistanceThisFrame ) > Dir.LengthSquared() )
                {
                    m_LeanWeaponOffset.Zero();
                }
                else
                {
                    Dir.Normalize();
                    Dir *= DistanceThisFrame;
                    m_LeanWeaponOffset += Dir;
                }
            }
            break;
        default:
            ASSERTS( FALSE, xfs( "Invalid lean_state: %d", m_LeanState ) );
        }
    }
    else
    {
        m_LeanWeaponOffset.Zero();
    }
    
    ASSERT( m_LeanWeaponOffset.IsValid() );
}


//=========================================================================

vector3 player::GetAnimPlayerOffset( void )
{
    return m_ArmsOffset + GetLeanOffset() + m_LeanWeaponOffset;
}

//=========================================================================

// we can't "hide" objects, so we'll just push it down really far out of site
void HideObject( object* pObj )
{
    if ( pObj )
    {
        vector3 Pos( pObj->GetPosition() );
        Pos.GetY() -= 10000.0f;
        pObj->OnMove( Pos );
    }
}
void UnhideObject( object* pObj )
{
    if ( pObj )
    {
        vector3 Pos( pObj->GetPosition() );
        Pos.GetY() += 10000.0f;
        pObj->OnMove( Pos );
    }
}

//=========================================================================

void player::ManTurret( guid TurretGuid, 
                        guid Turret2Guid, 
                        guid Turret3Guid, 
                        guid AnchorGuid,
                        guid LeftBoundaryGuid,
                        guid RightBoundaryGuid,
                        guid UpperBoundaryGuid,
                        guid LowerBoundaryGuid )
{
    LOG_MESSAGE( "Turret", "Man Turret" );

    if ( m_bIsMutated )
    {
        SetupMutationChange( FALSE );
#ifdef mreed 
        LOG_MESSAGE( "Mutation", "m_bIsMutantVisionOn = FALSE (man turret)" );
#endif
        m_bIsMutantVisionOn = FALSE;
        ForceNextWeapon();
    }

    m_Turret.PreviousWeapon = m_CurrentWeaponItem;
    SetNextWeapon2( INVEN_WEAPON_TRA );

    m_Turret.TurretGuid         = TurretGuid;
    m_Turret.Turret2Guid        = Turret2Guid;
    m_Turret.Turret3Guid        = Turret3Guid;
    m_Turret.PreviousL2W        = GetL2W();
    m_Turret.PreviousWeapon     = m_CurrentWeaponItem;
    m_Turret.LeftBoundaryGuid   = LeftBoundaryGuid;
    m_Turret.RightBoundaryGuid  = RightBoundaryGuid;
    m_Turret.UpperBoundaryGuid  = UpperBoundaryGuid;
    m_Turret.LowerBoundaryGuid  = LowerBoundaryGuid;

    // Hide the turret and parts
    turret* pTurret = (turret*)g_ObjMgr.GetObjectByGuid( TurretGuid );
    ASSERT( pTurret->IsKindOf( turret::GetRTTI() ) );
    if ( pTurret )
    {
        pTurret->Hide();
    }
    HideObject( g_ObjMgr.GetObjectByGuid( Turret2Guid ) );
    HideObject( g_ObjMgr.GetObjectByGuid( Turret3Guid ) );

    object* pAnchor = g_ObjMgr.GetObjectByGuid( AnchorGuid );
    if ( pAnchor )
    {
        m_Turret.AnchorL2W = pAnchor->GetL2W();
        const radian3 Rotation( m_Turret.AnchorL2W.GetRotation() );
        Teleport( m_Turret.AnchorL2W.GetTranslation(), Rotation.Pitch, Rotation.Yaw );
    }
    else
    {
        m_Turret.AnchorL2W.Identity();
    }

    SetIsCrouching( FALSE );
    m_bInTurret  = TRUE;
}

//=========================================================================

void player::ExitTurret( void )
{
    if ( m_bInTurret )
    {
        LOG_MESSAGE( "Turret", "Exit Turret" );

        const radian3 Rotation( m_Turret.PreviousL2W.GetRotation() );
        Teleport( m_Turret.PreviousL2W.GetTranslation(), Rotation.Pitch, Rotation.Yaw  );
        loco* pLoco = GetLocoPointer();
        if( pLoco )
            pLoco->m_Physics.SetVelocity( vector3(0.0f,0.0f,0.0f) );
        SwitchWeapon2( m_Turret.PreviousWeapon );
        m_Inventory2.RemoveAmount( INVEN_WEAPON_TRA, 1 );
        m_bInTurret  = FALSE;

        // unhide the turret and parts
        turret* pTurret = (turret*)g_ObjMgr.GetObjectByGuid( m_Turret.TurretGuid );
        ASSERT( pTurret->IsKindOf( turret::GetRTTI() ) );
        if ( pTurret )
        {
            pTurret->Unhide();
        }
        UnhideObject( g_ObjMgr.GetObjectByGuid( m_Turret.Turret2Guid ) );
        UnhideObject( g_ObjMgr.GetObjectByGuid( m_Turret.Turret3Guid ) );

    }
}

//=========================================================================

void player::Teleport( const vector3& Position, xbool DoBlend, xbool DoEffect ) 
{ 
    if( !DoBlend )
    {
        extern void ActivateSoundEmitters( const vector3& Position );
        ActivateSoundEmitters( Position );
    }

    m_Physics.InitialGroundCheck( Position );
    actor::Teleport( Position, DoBlend, DoEffect );

    if( !DoBlend )
    {
        // Update ears
        radian Pitch = GetPitch();
        radian Yaw   = GetYaw();
        matrix4 W2V;
        W2V.Identity();
        W2V.RotateX( Pitch );
        W2V.RotateY( Yaw );
        W2V.Translate( Position + vector3(0,180,0) );
        W2V.InvertRT();
        g_AudioMgr.SetEar( m_AudioEarID, W2V, Position, GetZone1(), 1.0f );
    }
    
    // Make sure 1st person weapon is in sync
    OnMoveWeapon();
}

//=========================================================================

void player::Teleport( const vector3& Position, radian Pitch, radian Yaw, xbool DoBlend, xbool DoEffect )
{
    // Update velocity direction
    vector3 Velocity = m_Physics.GetVelocity();
    radian  DeltaYaw = x_MinAngleDiff( Yaw, GetYaw() );
    Velocity.RotateY( DeltaYaw );
    m_Physics.SetVelocity( Velocity );

    if( !DoBlend )
    {
        extern void ActivateSoundEmitters( const vector3& Position );
        ActivateSoundEmitters( Position );
    }

    // When the player (not actor or ghost) teleports and gets a new pitch/yaw,
    // we need to clear his network targeting information just to be safe.
    m_TargetNetSlot = -1;

    m_Physics.InitialGroundCheck( Position );
    actor::Teleport( Position, Pitch, Yaw, DoBlend, DoEffect );

    if( !DoBlend )
    {
        // Update ears
        matrix4 W2V;
        W2V.Identity();
        W2V.RotateX( Pitch );
        W2V.RotateY( Yaw );
        W2V.Translate( Position + vector3(0,180,0) );
        W2V.InvertRT();
        g_AudioMgr.SetEar( m_AudioEarID, W2V, Position, GetZone1(), 1.0f );
        
        // Make sure 1st person hands are in sync
        m_AnimPlayer.SetYaw( m_Yaw + m_CurrentHorozRigOffset - m_ShakeYaw );
        m_AnimPlayer.SetPitch( -m_Pitch - m_CurrentVertRigOffset - m_ShakePitch );
    }
    
    // Make sure 1st person weapon is in sync
    AttachWeapon();
}

//=========================================================================

void player::ForceNextWeapon( void )
{
    // Set previous weapon and current weapon, clear next weapon
    m_PrevWeaponItem     = m_CurrentWeaponItem;
    m_CurrentWeaponItem  = m_NextWeaponItem;
    m_NextWeaponItem     = INVEN_NULL;

    // zero out the reticle radius
    m_ReticleRadius             = 0.0f;
    m_ReticleGrowSpeed          = 0.0f;
    m_AimAssistData.bReticleOn  = FALSE;

    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon )
    {
        pWeapon->SetupRenderInformation( );
        pWeapon->SetRotation( m_AnimPlayer.GetPitch() , m_AnimPlayer.GetYaw() );
        OnMoveWeapon();

        // Bring the new weapon up.
        SetAnimState( ANIM_STATE_SWITCH_TO );

        #ifndef X_EDITOR
        m_NetDirtyBits |= WEAPON_BIT;  // NETWORK
        #endif // X_EDITOR
    }
    else
    {
        SetAnimState( ANIM_STATE_UNDEFINED );
    }
}

//=========================================================================

void player::ForceMutationChange( xbool bMutate )
{
//  LOG_MESSAGE( "player::ForceMutationChange", "%s", bMutate ? "TRUE" : "FALSE" );
    
    SetupMutationChange( bMutate );

    if( m_bIsMutated && (m_CurrentWeaponItem != INVEN_WEAPON_MUTATION) )
    {
        ForceNextWeapon();
    }
    m_bIsMutantVisionOn = m_bIsMutated;
#ifdef mreed 
    LOG_MESSAGE( "Mutation", "m_bIsMutantVisionOn = %s (ForceMutationChange)", m_bIsMutated ? "TRUE" : "FALSE" );
#endif
}

//=========================================================================

vector3 player::GetBonePos( s32 BoneIndex )
{
    // for NPCs or 3rd person view of avatar
    if( IsAvatar() )
    {
        return actor::GetBonePos(BoneIndex);
    }

    // First-person player arms
    return m_AnimPlayer.GetBonePosition(BoneIndex); 
}

//=========================================================================

void player::ContagionDOT( void )
{

#ifndef X_EDITOR

    // Now, if we are in a multiplayer game, and I am an actual physical player,
    // I should go out and smack anyone standing too close to my very ill self 
    // with whom I have a clear LOS.  (The LOS testing is done in actor.)

    ASSERT( GameMgr.IsGameMultiplayer() );
    ASSERT( m_bContagious );
    ASSERT( m_pMPContagion );

    /*
    LOG_MESSAGE( "player::ContagionDOT", 
                 "Attacking:%08X", 
                 m_pMPContagion->PlayerMask );
    */

    actor* pActor = NULL;
    guid   Source = NULL_GUID;

    if( m_pMPContagion->Origin != -1 )
    {
        pActor = (actor*)NetObjMgr.GetObjFromSlot( m_pMPContagion->Origin );
        if( pActor )
            Source = pActor->GetGuid();
        else
            m_pMPContagion->Origin = -1;
    }

    for( s32 i = 0; i < 32; i++ )
    {
        if( !(m_pMPContagion->PlayerMask & (1<<i)) )
            continue;

        pActor = (actor*)NetObjMgr.GetObjFromSlot( i );
        if( !pActor )
            continue;

        pain Pain;
        Pain.Setup( "CONTAGION_TICK_TO_OTHERS", 
                    Source, GetPosition() );
        Pain.SetDirectHitGuid( pActor->GetGuid() );
        Pain.ApplyToObject( pActor->GetGuid() );
    }

    // Don't forget to take a little pain for your self!
    {   
        pain Pain;
        Pain.Setup( "CONTAGION_TICK", Source, GetPosition() );
        Pain.SetDirectHitGuid( GetGuid() );
        Pain.ApplyToObject( GetGuid() );
    }

#endif
}

//=========================================================================
