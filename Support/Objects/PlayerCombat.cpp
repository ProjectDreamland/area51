//==============================================================================
//
//  PlayerCombat.cpp
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
#include "Characters\MutantTank\Mutant_Tank.hpp"
#include "Objects\WeaponBBG.hpp"

#ifdef X_EDITOR
#include "../Apps/Editor/Project.hpp"
#else
#include "NetworkMgr\MsgMgr.hpp"
#include "Menu\DebugMenu2.hpp"
#endif

static       f32    LargeReticleSpeed           = 10.0f;
static       f32    SmallReticleSpeed           = 300.0f;
static       f32    WeaponCollisionRadius       = 5.0f;
static const radian s_AimYawOffsetRateOfChange  = R_180;

#define             MAX_AUTO_AIM_DISTANCE       4500.0f

//=========================================================================
// EXTERNALS
//=========================================================================

extern s32 g_Difficulty;
extern const char* DifficultyText[];

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

// tweak values
extern f32 AimAssist_LOF_Dist;

extern f32 AimAssist_Reticle_Near_Dist;
extern f32 AimAssist_Reticle_Far_Dist;
extern f32 AimAssist_Reticle_Near_Radius;
extern f32 AimAssist_Reticle_Far_Radius;

extern f32 AimAssist_Bullet_Inner_Near_Dist;
extern f32 AimAssist_Bullet_Inner_Far_Dist;
extern f32 AimAssist_Bullet_Inner_Near_Radius;
extern f32 AimAssist_Bullet_Inner_Far_Radius;

extern f32 AimAssist_Bullet_Outer_Near_Dist;
extern f32 AimAssist_Bullet_Outer_Far_Dist;
extern f32 AimAssist_Bullet_Outer_Near_Radius;
extern f32 AimAssist_Bullet_Outer_Far_Radius;

extern f32 AimAssist_Bullet_Angle;

extern f32 AimAssist_Turn_Inner_Near_Dist;
extern f32 AimAssist_Turn_Inner_Far_Dist;
extern f32 AimAssist_Turn_Inner_Near_Radius;
extern f32 AimAssist_Turn_Inner_Far_Radius;

extern f32 AimAssist_Turn_Outer_Near_Dist;
extern f32 AimAssist_Turn_Outer_Far_Dist;
extern f32 AimAssist_Turn_Outer_Near_Radius;
extern f32 AimAssist_Turn_Outer_Far_Radius;

extern f32 AimAssist_Turn_Damp_Near_Dist;
extern f32 AimAssist_Turn_Damp_Far_Dist;


// Tweak handles
extern tweak_handle AimAssist_LOF_Dist_Tweak;

extern tweak_handle AimAssist_Reticle_Near_Dist_Tweak ;
extern tweak_handle AimAssist_Reticle_Far_Dist_Tweak;
extern tweak_handle AimAssist_Reticle_Near_Radius_Tweak;
extern tweak_handle AimAssist_Reticle_Far_Radius_Tweak;

extern tweak_handle AimAssist_Bullet_Inner_Near_Dist_Tweak;
extern tweak_handle AimAssist_Bullet_Inner_Far_Dist_Tweak;
extern tweak_handle AimAssist_Bullet_Inner_Near_Radius_Tweak;
extern tweak_handle AimAssist_Bullet_Inner_Far_Radius_Tweak;

extern tweak_handle AimAssist_Bullet_Outer_Near_Dist_Tweak;
extern tweak_handle AimAssist_Bullet_Outer_Far_Dist_Tweak;
extern tweak_handle AimAssist_Bullet_Outer_Near_Radius_Tweak;
extern tweak_handle AimAssist_Bullet_Outer_Far_Radius_Tweak;

extern tweak_handle AimAssist_Bullet_Angle_Tweak;

extern tweak_handle AimAssist_Turn_Inner_Near_Dist_Tweak;
extern tweak_handle AimAssist_Turn_Inner_Far_Dist_Tweak;
extern tweak_handle AimAssist_Turn_Inner_Near_Radius_Tweak;
extern tweak_handle AimAssist_Turn_Inner_Far_Radius_Tweak;

extern tweak_handle AimAssist_Turn_Outer_Near_Dist_Tweak;
extern tweak_handle AimAssist_Turn_Outer_Far_Dist_Tweak;
extern tweak_handle AimAssist_Turn_Outer_Near_Radius_Tweak;
extern tweak_handle AimAssist_Turn_Outer_Far_Radius_Tweak;

extern tweak_handle AimAssist_Turn_Damp_Near_Dist_Tweak;
extern tweak_handle AimAssist_Turn_Damp_Far_Dist_Tweak;

xbool g_PlayerMeleeShakeView                    = TRUE;
xbool g_PlayerMeleeDoFeedback                   = TRUE;

////////////////////////////////////////////////////////
// KSS -- FIXME -- move these to tweak table
// Combo tweaks 
f32 s_MeleeShakeSpeed[MAX_COMBO_HITS]           = { 4.0f, 4.0f, 4.0f };
f32 s_MeleeShakeTime[MAX_COMBO_HITS]            = { 0.4f, 0.4f, 0.5f };
f32 s_MeleeShakeAmount[MAX_COMBO_HITS]          = { 3.0f, 3.0f, 4.0f };
f32 s_MeleeFeedbackShakeAmount[MAX_COMBO_HITS]  = { 1.0f, 1.0f, 1.0f};
f32 s_MeleeFeedbackShakeTime[MAX_COMBO_HITS]    = { 0.2f, 0.2f, 0.25f};
// END -- KSS -- FIXME -- move these to tweak table
////////////////////////////////////////////////////////

s32 GetStartWeaponsForLevel( const char* pLevelName );
inven_item GetEquipedWeaponForLevel( const char* pLevelName );


//INVEN_WEAPON_MUTATION

// KSS -- TO ADD NEW WEAPON
mtwt s_MapToWeaponTable[] =
{
    {   "dreamlnd",    1*1024, INVEN_NULL              , WB_DE | WB_SCN                                                                    , 0                                                                                                 , TRUE  },
    {   "undrgrnd",    1*1024, INVEN_NULL              , WB_DE | WB_SCN                                                                    , 0                                                                                                 , TRUE  },
    {   "hotzone",     1*1024, INVEN_WEAPON_SMP        , WB_DE | WB_SMP | WB_SCN                                                           , WB_DE | WB_SMP | WB_SCN                                                                           , TRUE  },
    {   "search",      1*1024, INVEN_WEAPON_SMP        , WB_DE | WB_SMP | WB_FG | WB_SG | WB_SCN                                           , WB_DE | WB_SMP | WB_FG | WB_SCN                                                                   , FALSE },
    {   "getbig",      1*1024, INVEN_WEAPON_SMP        , WB_DE | WB_SMP | WB_FG | WB_SG | WB_SCN                                           , WB_DE | WB_SMP | WB_FG | WB_SCN                                                                   , FALSE },
    {   "laststnd",    1*1024, INVEN_WEAPON_SMP        , WB_DE | WB_SMP | WB_FG | WB_SG | WB_SCN                                           , WB_DE | WB_SMP | WB_FG | WB_SG | WB_SCN                                                           , FALSE },
    {   "oneofthm",    1*1024, INVEN_WEAPON_SMP        , WB_MM | WB_SMP | WB_DE | WB_SCN                                                   , WB_MM | WB_SCN                                                                                    , FALSE },
    {   "mutation",    1*1024, INVEN_WEAPON_SMP        , WB_DE | WB_SMP | WB_MM | WB_SG | WB_MP | WB_SCN                                   , WB_DE | WB_SMP | WB_MM | WB_SCN                                                                   , FALSE },
    {   "sidetrck",    1*1024, INVEN_WEAPON_SMP        , WB_DE | WB_SMP | WB_SG | WB_MM | WB_MP | WB_SCN                                   , WB_DE | WB_SMP | WB_SG | WB_MM | WB_MP | WB_SCN                                                   , FALSE },
    {   "dr_cray",     1*1024, INVEN_WEAPON_SMP        , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_MS | WB_SCN                   , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_SCN                           , FALSE },
    {   "illumin",     1*1024, INVEN_WEAPON_SMP        , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_MS | WB_SCN                   , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_SCN                   | WB_MS , FALSE },
    {   "flynobjs",    1*1024, INVEN_WEAPON_SMP        , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_MS | WB_SCN                   , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_SCN                   | WB_MS , FALSE },
    {   "liespast",    1*1024, INVEN_WEAPON_SMP        , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_MS | WB_BBG | WB_SCN | WB_JBG , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_SCN | WB_JBG          | WB_MS , FALSE },
    {   "caves",       1*1024, INVEN_WEAPON_SMP        , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_MS | WB_BBG | WB_SCN          , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_BBG | WB_SCN          | WB_MS , FALSE },
    {   "boarding",    1*1024, INVEN_WEAPON_SMP        , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_MS | WB_BBG | WB_SCN          , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_BBG | WB_SCN          | WB_MS , FALSE },
    {   "ascend1",     1*1024, INVEN_WEAPON_SMP        , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_MS | WB_BBG | WB_MC | WB_SCN  , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_BBG | WB_SCN          | WB_MS , FALSE },
    {   "ascend2",     1*1024, INVEN_WEAPON_SMP        , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_MS | WB_BBG | WB_MC | WB_SCN  , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_BBG | WB_MC | WB_SCN  | WB_MS , FALSE },
    {   "ascend3",     1*1024, INVEN_WEAPON_SMP        , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_MS | WB_BBG | WB_MC | WB_SCN  , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_BBG | WB_MC | WB_SCN  | WB_MS , FALSE },
    {   "exit",        1*1024, INVEN_WEAPON_SMP        , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_MS | WB_BBG | WB_MC | WB_SCN  , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_BBG | WB_MC | WB_SCN  | WB_MS , FALSE },
    {     NULL,        1*1024, INVEN_WEAPON_SMP        , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_MS | WB_BBG | WB_MC | WB_SCN  , WB_DE | WB_SMP | WB_FG | WB_SG | WB_MM | WB_SR | WB_MP | WB_BBG | WB_MC | WB_SCN  | WB_MS , FALSE },
};



xbool player::IsAltFiring( void )
{
    return m_bIsMutated
        ? (xbool)(g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_FIRE_CONTAGION ).IsValue)
        : (xbool)(g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_SECONDARY ).IsValue);
}

//==============================================================================
xbool player::IsFiring( void )
{
    xbool PrimaryDown 
        = m_bIsMutated 
        ? (xbool)g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_FIRE_PARASITES ).IsValue
        : (xbool)g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_PRIMARY ).IsValue;
#if defined(TARGET_PC) && !defined(X_EDITOR)
    PrimaryDown |= input_IsPressed( INPUT_MOUSE_BTN_L );
#endif

    return( PrimaryDown && !m_bRespawnButtonPressed );
}
//==============================================================================

void player::GetProjectileHitLocation(vector3& EndPos, xbool bUseBulletAssist)
{   
    radian Pitch;
    radian Yaw;

    // the view's rotation
    view &View = GetView();

    if( bUseBulletAssist )
    {
        m_AimAssistData.BulletAssistDir.GetPitchYaw( Pitch, Yaw );
    }
    else
    {
        GetEyesPitchYaw( Pitch, Yaw );
    }

    vector3 Dest( radian3( Pitch, Yaw, 0.0f ) );
    const vector3 ViewPos = View.GetPosition();
    Dest *= MAX_AUTO_AIM_DISTANCE;
    EndPos = ViewPos + Dest;
    g_CollisionMgr.AddToIgnoreList( GetGuid() );
    g_CollisionMgr.RaySetup( GetGuid(), ViewPos, EndPos );
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_SMALL_PROJECTILES, object::ATTR_COLLISION_PERMEABLE);

    // default modifier to full distance in case the collision manager returns no collisions
    f32 DistModifier = 1.0f;

    // if we don't hit anything, T is undefined
    if( g_CollisionMgr.m_nCollisions > 0 )
    {
        DistModifier = g_CollisionMgr.m_Collisions[0].T;
    }

    // get our new end position
    EndPos = ViewPos + (DistModifier*(EndPos-ViewPos));
}

//==============================================================================
radian3 player::GetProjectileTrajectory( void )
{
    // the view's rotation
    radian Pitch;
    radian Yaw;
    vector3 StartPosition;
    xbool bUseWeaponPos = FALSE;

    new_weapon* pWeaponObj = GetCurrentWeaponPtr();

    // StartPosition will most likely be the "firepoint" of the weapon instead of coming out of your eyes.
    // This is for weapons like the Meson Cannon where you can see the projectile and the weapon.
    // GetFiringStartPosition() will return true for weapons that overload it.  Defaults to FALSE.
    bUseWeaponPos = pWeaponObj->GetFiringStartPosition(StartPosition);

    // if bUseWeaponPos == TRUE, for things like meson cannon and mutation weapon, fire projectile from firing point
    // Otherwise, use bullet assist direction
    if( !bUseWeaponPos )
    {
        m_AimAssistData.BulletAssistDir.GetPitchYaw( Pitch, Yaw );

        // apply aim degredation
        radian3 Rot = ApplyAimDegredation( Pitch, Yaw );

        return Rot;
    }

    vector3 EndPosition;
    GetProjectileHitLocation(EndPosition);

    // get the direction vector from the firing start position to where our trace hit
    vector3 ToTarget( EndPosition - StartPosition );

    // reload pitch and yaw from the Target vector
    ToTarget.GetPitchYaw( Pitch, Yaw );

    // We are firing the projectile visually from the weapon.
    radian3 ProjectileRot = ApplyAimDegredation( Pitch, Yaw );

    return ProjectileRot;
}

//==============================================================================

radian3 player::ApplyAimDegredation( radian Pitch, radian Yaw )
{
    vector3 Dir(0,0,1);
    Dir.RotateX( R_6*x_frand(-m_AimDegradation,m_AimDegradation) );   // Pitch Z-axis up or down by spread angle
    Dir.RotateZ( x_frand(0,R_360) );    // Roll dir around Z
    Dir.RotateX( Pitch );                  // Orient around original direction
    Dir.RotateY( Yaw );

    f32 P,Y;
    Dir.GetPitchYaw(P,Y);

    //x_DebugMsg("%f %f %f %f %f\n",m_AimDegradation,Pitch,Yaw,P,Y);

    return radian3(P,Y,0);
}

//==============================================================================

guid player::GetEnemyOnReticle( void )
{
    if( ReticleOnTarget() )
    {
        return m_AimAssistData.TargetGuid;
    }
    else
    {
        return 0;
    }
}

//===========================================================================
guid player::GetFriendlyOnReticle( void )
{
    if( ReticleOnTarget() )
    {
        return m_AimAssistData.OnlineFriendlyTargetGuid;
    }
    else
    {
        return 0;
    }
}

//===========================================================================

void player::UpdateAimOffset( f32 DeltaTime )
{
    // Largest value for AimOffset should be R_25.  Smallest value should be 0.
    if ( m_AimAssistData.TargetGuid != 0 )
    {
        m_YawAimOffset = MIN( R_25, m_YawAimOffset + DeltaTime * s_AimYawOffsetRateOfChange );
    }
    else
    {
        m_YawAimOffset = MAX( R_0, m_YawAimOffset - DeltaTime * s_AimYawOffsetRateOfChange );
    }
}

//===========================================================================

xbool player::AddAmmo2( inven_item WeaponItem, s32 Amount )
{
    // not sending in anything
    if( Amount == 0 )
        return FALSE;

    if( m_Inventory2.HasItem( WeaponItem ) )
    {
        new_weapon* pWeapon = GetWeaponPtr( WeaponItem );

        if( pWeapon )
        {
            s32 nCurrentAmmoCount = pWeapon->GetTotalPrimaryAmmo();
            s32 nMaxAmmoCount     = pWeapon->GetMaxPrimaryAmmo();

            if( nCurrentAmmoCount < nMaxAmmoCount )
            {
                f32 NewAmount = (f32)Amount;
#ifndef X_EDITOR
                if( GameMgr.GetGameType() == GAME_CAMPAIGN )
                {
                    tweak_handle AmmoScalarTweak( xfs( "AmmoAmount_%s", DifficultyText[g_Difficulty] ) );

                    f32 AmmoScalar = 0.0f;

                    if( AmmoScalarTweak.Exists() )
                    {
                        AmmoScalar = AmmoScalarTweak.GetF32();
                    }

                    // Scale ammo based on difficulty level
                    // the scalar could be +/- and is a whole percentage i.e. -20
                    NewAmount = Amount + ( Amount * (AmmoScalar/100.0f) );

                    // make sure we don't give less than 1, that's bad
                    if( NewAmount < 1.0f )
                    {
                        NewAmount = 1.0f;
                    }
                }
#endif 
                // actually put ammo into weapon
                pWeapon->AddAmmoToWeapon( (s32)NewAmount, 0 );

                // if this weapon was empty, reload it for us
                if( nCurrentAmmoCount == 0 )
                {
                    // if this is our current weapon, do all the animations and such
                    if( WeaponItem == m_CurrentWeaponItem )
                    {
                        ReloadWeapon(new_weapon::AMMO_PRIMARY);
                    }
                    else
                    {
                        // otherwise, just auto-reload it
                        pWeapon->Reload(new_weapon::AMMO_PRIMARY);
                    }
                }

                return TRUE;
            }
        }
    }
    return FALSE;
}

//----------------------------------------------------------------------------------------------------------------

void player::DegradeAim( f32 fAmountToDegradeBy )
{
    m_AimDegradation = MIN(1.0f, m_AimDegradation + fAmountToDegradeBy);
}

//===========================================================================


f32 MinYawModifier        =   0.42f;
f32 AimAssistDownVelocity = 100.00f;
f32 AimAssistUpVelocity   =   1.00f;

void player::UpdateAimAssistance( f32 DeltaTime )
{
    // find and set the targeted guid.
    UpdateCurrentAimTarget( DeltaTime );

    s32     LastNetSlot   = m_TargetNetSlot;
    vector3 LastAimOffset = m_AimOffset;

    m_TargetNetSlot = -1;

    if ( m_AimAssistData.TargetGuid != 0 )
    {
        m_fCurrentYawAimModifier = MAX( m_fCurrentYawAimModifier - AimAssistDownVelocity * DeltaTime, MinYawModifier );
        m_fCurrentPitchAimModifier = 0.5f;

        object* pObject = g_ObjMgr.GetObjectByGuid( m_AimAssistData.TargetGuid );

        // Update the aim offset if we're aiming at another human player.
        if( pObject )
        {  
            if( pObject->IsKindOf( actor::GetRTTI() ) )
            {
                actor& Actor = actor::GetSafeType( *pObject ); 
                s32 NetSlot = Actor.net_GetSlot();
                if( IN_RANGE( 0, NetSlot, 31 ) && !Actor.IsDead() )
                {
                    m_TargetNetSlot = NetSlot;
                    m_AimOffset     = m_AimAssistData.AimDelta;
                }
            }
        }
    }
    else
    {
        m_fCurrentYawAimModifier = MIN( m_fCurrentYawAimModifier + AimAssistUpVelocity * DeltaTime, 1.0f );
        m_fCurrentPitchAimModifier = 1.0f;
    }

#ifndef X_EDITOR
    if( (m_TargetNetSlot != LastNetSlot) || ((m_TargetNetSlot != -1) && (m_AimOffset != LastAimOffset)) )
    {
        m_NetDirtyBits |= ORIENTATION_BIT;
    }
#endif
}

//===========================================================================

struct targetData
{
    guid    Guid;
    f32     distance;
};

//===========================================================================

s32 DataListCompareFN( targetData* pItem1, targetData* pItem2 )
{
    if( pItem1->distance < pItem2->distance ) return -1;
    return pItem1->distance > pItem2->distance;
}

//=============================================================================

f32 g_AimYawConstraint   = R_5;
f32 g_AimPitchConstraint = R_3;

//=============================================================================

static inline
f32 ComputeInterpValue( f32 Dist, f32 NearDist, f32 FarDist, f32 NearValue, f32 FarValue )
{
    f32 T = (Dist - NearDist) / (FarDist - NearDist);            
    if( T<0 ) T = 0;
    if( T>1 ) T = 1;
    return NearValue + T*(FarValue - NearValue);
}

//=============================================================================
#ifdef ksaffel
#define AIM_LOGGING_ENABLED 1
#else
#define AIM_LOGGING_ENABLED 0
#endif

f32 AIMASSIST_PERP_SPEED_MAX      = 300;
f32 AIMASSIST_BULLET_ASSIST_SCALE = 2.5f;
f32 AIMASSIST_TURN_DAMPEN_SCALE   = 0.1f;
f32 AIMASSIST_MULTIPLAYER_SCALE   = 1.5f;


xbool g_bTestFriendly = TRUE;
void player::UpdateCurrentAimTarget( f32 DeltaTime )
{
    (void) DeltaTime;
    const view& View                    = GetView();
    vector3 Position                    = GetPosition();
    xbool   Final_bReticleOn            = FALSE;
    f32     TargetCullDot               = x_cos(R_20);

    m_AimAssistData.BulletAssistDir       = View.GetViewZ();
    m_AimAssistData.BulletAssistBestDist  = F32_MAX;
    m_AimAssistData.TurnDampeningT        = 0.0f;
    m_AimAssistData.TargetGuid            = 0;
    m_AimAssistData.OnlineFriendlyTargetGuid = 0;

    if( GetCurrentWeaponPtr() == NULL )
        return;

    if( AimAssist_LOF_Dist == 0.0f )
        return;

    // Decide if we should scale up radii based on multiplayer
    f32 MultiplayerRadiiScale = 1.0f;

#ifndef X_EDITOR
    // take no damage from friendly sources.
    if( GameMgr.IsGameMultiplayer() )
    {
        MultiplayerRadiiScale = AIMASSIST_MULTIPLAYER_SCALE;
    }
#endif


    // Decide if bullet assist should lead
    f32 BulletAssistLeadSpeed = 0.0f;
    {
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if( pWeapon->IsKindOf( weapon_bbg::GetRTTI() ) )
        {
            tweak_handle SpeedTweak( xfs("%s_SPEED", pWeapon->GetLogicalName()) );
            BulletAssistLeadSpeed = SpeedTweak.GetF32();
        }
    }

    // Get our velocity
    vector3 PlayerVelocity = m_Physics.GetVelocity();
//    x_printfxy(0,1,"LS: %7.3f",BulletAssistLeadSpeed);
//    x_printfxy(0,2,"%7.1f %7.1f %7.1f",PlayerVelocity.GetX(),PlayerVelocity.GetY(),PlayerVelocity.GetZ());

    //
    // Find LOF Collision distance
    //
    vector3 LOFStart;
    vector3 LOFEnd;
    vector3 LOFDir;    
    {
        LOFDir                              = View.GetViewZ();
        LOFStart                            = View.GetPosition();
        LOFEnd                              = LOFStart + LOFDir * AimAssist_LOF_Dist;
        g_CollisionMgr.LineOfSightSetup( GetGuid(), LOFStart, LOFEnd );
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_PLAYER_LOS, object::ATTR_COLLISION_PERMEABLE|object::ATTR_LIVING);

        // save off collision distance
        if( g_CollisionMgr.m_nCollisions > 0 )
        {
            m_AimAssistData.LOFCollisionDist = AimAssist_LOF_Dist * g_CollisionMgr.m_Collisions[0].T;
        }
    }

    //
    // Loop through all active players
    //
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
                //
                // Get target position info
                //
                vector3 TargetPos   = pActor->GetPosition();
                vector3 TargetDelta = TargetPos - Position;
                f32 TargetDist      = TargetDelta.Length();

                // throw out this guy if he's behind us
                if( LOFDir.Dot(TargetDelta) <= 0.0f )
                {
                    continue;
                }

                // If target is a decent distance away check if the angle
                // to his position is far away from the LOFDir
                vector3 TargetDeltaDir = TargetDelta;
                TargetDeltaDir.Normalize();
                if( TargetDist > 1000.0f )
                {
                    if( LOFDir.Dot(TargetDeltaDir) <= TargetCullDot )
                    {
                        continue;
                    }
                }
            
                //
                // Get Velocity of target
                //
                vector3 TargetVelocity(0,0,0);
                vector3 RelativeVelocity(0,0,0);
                f32     PerpSpeed=0;
                vector3 PerpVelocity;
                f32     PerpSpeedT=0;
                {
                    loco* pLoco = pActor->GetLocoPointer();
                    if( pLoco )
                    {
                        TargetVelocity = pLoco->m_Physics.GetVelocity();
                    }

                    RelativeVelocity = TargetVelocity - PlayerVelocity;
                    PerpVelocity = RelativeVelocity - (LOFDir.Dot(RelativeVelocity)*LOFDir);
                    PerpSpeed = PerpVelocity.Length();
                    PerpSpeedT = PerpSpeed / AIMASSIST_PERP_SPEED_MAX;
                    if( PerpSpeedT > 1 ) PerpSpeedT = 1;
                }

                // Compute scale for bullet assist and turn dampening 
                f32 BulletAssistPerpSpeedScale = 1.0f + PerpSpeedT * (AIMASSIST_BULLET_ASSIST_SCALE - 1.0f);
                f32 TurnDampenPerpSpeedScale = 1.0f + PerpSpeedT * (AIMASSIST_TURN_DAMPEN_SCALE - 1.0f);

                //
                // Get spine information
                //
                vector3 SpineTop;
                vector3 SpineBot;
                {
                    pActor->GetHeadAndRootPosition( SpineTop, SpineBot );

                    // Raise top of spine since head bone is at base of head
                    SpineTop += vector3(0,20.0f,0);
                }

                //
                // Get closest pt between LOF and Spine
                //
                vector3 SpinePt;
                vector3 LOFPt;
                vector3 SpineLOFOffset;
                f32     LOFPtT;
                f32     SpinePtT;
                f32     LOFPtDist;
                {
                    x_ClosestPtsOnLineSegs( LOFStart, LOFEnd, SpineTop, SpineBot, LOFPt, SpinePt, LOFPtT, SpinePtT );
                    LOFPtDist = AimAssist_LOF_Dist * LOFPtT;
                    SpineLOFOffset = (SpinePt - LOFPt);
                    m_AimAssistData.LOFSpineDist = SpineLOFOffset.Length();

                    m_AimAssistData.SpinePt   = SpinePt;
                    m_AimAssistData.SpinePtT  = SpinePtT;
                    m_AimAssistData.LOFPt     = LOFPt;
                    m_AimAssistData.LOFPtDist = LOFPtDist;

                    // don't let it go negative (behind us) or 0
                    m_AimAssistData.LOFPtDist = x_clamp(m_AimAssistData.LOFPtDist, 1.0f, F32_MAX);
                }

                //
                // Check distance to LOFPt against collision
                //
                xbool bLOFPtBlocked = (LOFPtDist > m_AimAssistData.LOFCollisionDist);


                //
                // Check if Reticle should be on
                //
                if( bLOFPtBlocked == FALSE )
                {
                    f32 Radius = ComputeInterpValue( TargetDist,
                        AimAssist_Reticle_Near_Dist,
                        AimAssist_Reticle_Far_Dist,
                        AimAssist_Reticle_Near_Radius*BulletAssistPerpSpeedScale,
                        AimAssist_Reticle_Far_Radius*BulletAssistPerpSpeedScale );

                    m_AimAssistData.ReticleRadius = Radius;

                    if( m_AimAssistData.LOFSpineDist <= Radius )
                    {
                        Final_bReticleOn = TRUE;
                    }
                }

                //
                // Compute a new BulletAssistDir if the LOF approaches closer than the previous target
                //
                if( (bLOFPtBlocked == FALSE) && (m_AimAssistData.LOFSpineDist < m_AimAssistData.BulletAssistBestDist) )
                {
                    // Compute what the new bullet assist direction would be
                    f32 InnerRadius = ComputeInterpValue( TargetDist,
                        AimAssist_Bullet_Inner_Near_Dist,
                        AimAssist_Bullet_Inner_Far_Dist,
                        AimAssist_Bullet_Inner_Near_Radius*BulletAssistPerpSpeedScale*MultiplayerRadiiScale,
                        AimAssist_Bullet_Inner_Far_Radius*BulletAssistPerpSpeedScale*MultiplayerRadiiScale );

                    f32 OuterRadius = ComputeInterpValue( TargetDist,
                        AimAssist_Bullet_Outer_Near_Dist,
                        AimAssist_Bullet_Outer_Far_Dist,
                        AimAssist_Bullet_Outer_Near_Radius*BulletAssistPerpSpeedScale*MultiplayerRadiiScale,
                        AimAssist_Bullet_Outer_Far_Radius*BulletAssistPerpSpeedScale*MultiplayerRadiiScale );

                    m_AimAssistData.BulletInnerRadius = InnerRadius;
                    m_AimAssistData.BulletOuterRadius = OuterRadius;

                    // interpolate between inner and outer radius (inverse)
                    f32 T = x_parametric(m_AimAssistData.LOFSpineDist, OuterRadius, InnerRadius, TRUE);
                    if( T > 0 )
                    {
                        radian AssistAngle = T*AimAssist_Bullet_Angle;

                        // Build AimAtPoint
                        vector3 AimAtPoint = SpinePt;

                        // Apply lead for slow projectiles
                        if( (BulletAssistLeadSpeed>0) && (PerpSpeed>0.0f) )
                        {
                            // Compute approx flight time
                            f32 FlightTime = TargetDist / BulletAssistLeadSpeed;
                            f32 LeadDist = PerpSpeed * FlightTime;
                            if( LeadDist > 100.0f ) LeadDist = 100.0f;
                            AimAtPoint += PerpVelocity*(LeadDist/PerpSpeed);
//                            x_printfxy(0,5,"LD: %6.2f",LeadDist);
                        }

                        // Rotate LOFDir toward AimAtPoint by the angle AssistAngle
                        vector3 ToSpine = AimAtPoint - View.GetPosition();
                        radian Angle = v3_AngleBetween( LOFDir, ToSpine );
                        if( Angle > AssistAngle ) Angle = AssistAngle;
                        vector3 Axis = LOFDir.Cross( ToSpine );
                        Axis.Normalize();
                        quaternion Q( Axis, Angle );

                        //
                        // Remember aim direction
                        //
                        vector3 NewBulletAssistDir = Q * LOFDir;

                        // Only replace best if LOF is clear.  Search for a clear LOF from full bullet
                        // assist direction back to LOF direction
                        s32 nSegs=4;
                        s32 i;
                        for( i=0; i<nSegs; i++ )
                        {
                            f32 T = (f32)i / (f32)(nSegs-1);
                            vector3 Dir = NewBulletAssistDir + T*(LOFDir-NewBulletAssistDir);
                            Dir.Normalize();

                            vector3 EndPos = LOFStart + Dir*m_AimAssistData.LOFPtDist;
                            g_CollisionMgr.LineOfSightSetup( GetGuid(), LOFStart, EndPos);
                            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_PLAYER_LOS, object::ATTR_COLLISION_PERMEABLE|object::ATTR_LIVING);

                            if( g_CollisionMgr.m_nCollisions==0 )
                            {
                                m_AimAssistData.BulletAssistBestDist = m_AimAssistData.LOFSpineDist;
                                m_AimAssistData.TargetGuid           = pActor->GetGuid();
                                m_AimAssistData.BulletAssistDir      = Dir;

                                // Get the aim delta relative to the players orientation.
                                vector3 AbsoluteDelta = LOFPt - pActor->GetPosition();

                                m_AimAssistData.AimDelta             = AbsoluteDelta.Length() *
                                    vector3( AbsoluteDelta.GetPitch(), 
                                    AbsoluteDelta.GetYaw() - pActor->GetYaw() );

                                break;
                            }
                        }
                    }
                }

                //
                // Compute turn dampening amount
                //
                if( bLOFPtBlocked == FALSE )
                {
                    f32 InnerRadius = ComputeInterpValue( TargetDist,
                        AimAssist_Turn_Inner_Near_Dist,
                        AimAssist_Turn_Inner_Far_Dist,
                        AimAssist_Turn_Inner_Near_Radius,
                        AimAssist_Turn_Inner_Far_Radius );

                    f32 OuterRadius = ComputeInterpValue( TargetDist,
                        AimAssist_Turn_Outer_Near_Dist,
                        AimAssist_Turn_Outer_Far_Dist,
                        AimAssist_Turn_Outer_Near_Radius,
                        AimAssist_Turn_Outer_Far_Radius );

                    m_AimAssistData.TurnInnerRadius = InnerRadius;
                    m_AimAssistData.TurnOuterRadius = OuterRadius;

                    // interpolate between inner and outer radius (inverse)
                    f32 T = x_parametric(m_AimAssistData.LOFSpineDist, OuterRadius, InnerRadius, TRUE);

                    T *= TurnDampenPerpSpeedScale;

                    // Keep the largest amount
                    if( T > m_AimAssistData.TurnDampeningT )
                        m_AimAssistData.TurnDampeningT = T;
                }
            }
            else  ////////////////////////////////////////// FRIENDLY TARGET //////////////////////////////////////////
                if(g_bTestFriendly)
                {
                    //
                    // Get target position info
                    //
                    vector3 TargetPos   = pActor->GetPosition();
                    vector3 TargetDelta = TargetPos - Position;
                    f32 TargetDist      = TargetDelta.Length();

                    // throw out this guy if he's behind us
                    if( LOFDir.Dot(TargetDelta) <= 0.0f )
                    {
                        continue;
                    }

                    // If target is a decent distance away check if the angle
                    // to his position is far away from the LOFDir
                    vector3 TargetDeltaDir = TargetDelta;
                    TargetDeltaDir.Normalize();
                    if( TargetDist > 1000.0f )
                    {
                        if( LOFDir.Dot(TargetDeltaDir) <= TargetCullDot )
                        {
                            continue;
                        }
                    }

                    //
                    // Get spine information
                    //
                    vector3 SpineTop;
                    vector3 SpineBot;
                    {
                        SpineBot = pActor->GetPosition();
                        SpineTop = SpineBot + vector3(0,150,0);
                        /* Commented out for performance around friendlies - AndyT
                        ((character*)pActor)->GetHeadAndRootPosition( SpineTop, SpineBot );
                        // Raise top of spine since head bone is at base of head
                        SpineTop += vector3(0,20.0f,0);
                        */
                    }

                    //
                    // Get closest pt between LOF and Spine
                    //
                    vector3 SpinePt;
                    vector3 LOFPt;
                    vector3 SpineLOFOffset;
                    f32     LOFPtT;
                    f32     SpinePtT;
                    f32     LOFPtDist;
                    f32     Dist;
                    {
                        x_ClosestPtsOnLineSegs( LOFStart, LOFEnd, SpineTop, SpineBot, LOFPt, SpinePt, LOFPtT, SpinePtT );
                        LOFPtDist = AimAssist_LOF_Dist * LOFPtT;
                        SpineLOFOffset = (SpinePt - LOFPt);
                    }

                    //
                    // Check distance to LOFPt against collision
                    //
                    xbool bLOFPtBlocked = (LOFPtDist > m_AimAssistData.LOFCollisionDist);

                    Dist = SpineLOFOffset.Length();

                    //
                    // Check if Reticle should be on
                    //
                    if( bLOFPtBlocked == FALSE )
                    {
                        f32 Radius = ComputeInterpValue( TargetDist,
                            AimAssist_Reticle_Near_Dist,
                            AimAssist_Reticle_Far_Dist,
                            AimAssist_Reticle_Near_Radius,
                            AimAssist_Reticle_Far_Radius );

                        // see if we're inside pill
                        if( Dist <= Radius )
                        {
                            Final_bReticleOn = TRUE;
                            m_AimAssistData.OnlineFriendlyTargetGuid = pActor->GetGuid();
                            break; // break out of while loop
                        }
                    }
                }
        }
    }

    //
    // Deaden the aim stick
    //
    if( m_AimAssistData.TurnDampeningT > 0 )
    {
        // kill aim assist stick dampening at AimAssist_Turn_Damp_Near_Dist
        f32 DampPct = x_parametric(m_AimAssistData.LOFPtDist, AimAssist_Turn_Damp_Near_Dist, AimAssist_Turn_Damp_Far_Dist, TRUE);

        // scale turn dampening
        m_AimAssistData.TurnDampeningT = m_AimAssistData.TurnDampeningT * DampPct;

        tweak_handle StickDampTweak("TurnDampeningT"); // 0=no turning, 1=normal turning
        f32 DampMax     = StickDampTweak.GetF32();
        f32 StickyMult  = 1.0f + (DampMax-1.0f)*m_AimAssistData.TurnDampeningT;
        m_fYawValue     *= StickyMult;
        m_fPitchValue   *= StickyMult;

        CLOG_MESSAGE( AIM_LOGGING_ENABLED, "player::UpdateCurrentAimTarget", "T:: %f", m_AimAssistData.TurnDampeningT );

    }
    else
    {
        CLOG_MESSAGE( AIM_LOGGING_ENABLED, "player::UpdateCurrentAimTarget *CHECK*", "T: %f", m_AimAssistData.TurnDampeningT );

        // Be sure if we aren't doing aim assist for enemies that we kill turn dampening
        m_AimAssistData.TurnDampeningT = 0.0f;
    }


    //
    // Remember reticle is on the target
    //
    {
        m_AimAssistData.bReticleOn = Final_bReticleOn;
    }

    UpdateReticleRadius( DeltaTime );

//    x_printfxy(0,3,"%7.1f %7.1f %7.1f",BestVelocity.GetX(),BestVelocity.GetY(),BestVelocity.GetZ());
//    x_printfxy(0,4,"TD: %4.2f", m_AimAssistData.TurnDampeningT);

}

//=============================================================================

radian player::CalculateNecessaryAimAssistYaw( object* pObject )
{
    // Get the vector from the view to the center of the object's bbox.
    vector3 vViewToBoxCenter = pObject->GetColBBox().GetCenter() - m_EyesPosition;

    // Calculate the length and width that I'm going to need.
    // We may need to optimize this, because it's expensive.
    // ALSO: need to check vs. width here, not just radius.
    f32 fToCenterLength = vViewToBoxCenter.Length();
    //    f32 fBoxWidth = pObject->GetColBBox().GetRadius();
    f32 fBoxWidth = pObject->GetColBBox().Max.GetX() - pObject->GetColBBox().Min.GetX();

    radian HalfAngle = x_atan( fBoxWidth / fToCenterLength );

    return HalfAngle;
}

//===========================================================================

radian  player::CalculateNecessaryAimAssistPitch( object* pObject )
{
    // Get the vector from the view to the center of the object's bbox.
    vector3 vViewToBoxCenter = pObject->GetColBBox().GetCenter() - m_EyesPosition;

    // Calculate the length and width that I'm going to need.
    // We may need to optimize this, because it's expensive.
    // ALSO: need to check vs. width here, not just radius.
    f32 fToCenterLength = vViewToBoxCenter.Length();
    f32 fBoxHeight = pObject->GetColBBox().Max.GetY() - pObject->GetColBBox().Min.GetY();

    radian HalfAngle = x_atan( fBoxHeight / fToCenterLength );

    return HalfAngle;
}

//===========================================================================

radian player::CalculateActualYawToTarget( object* pObject )
{
    vector3 vViewToBoxCenter = pObject->GetColBBox().GetCenter() - m_EyesPosition;

    // Calculate the differences in pitch
    radian YawBox;//, WorldViewYaw;
    YawBox = vViewToBoxCenter.GetYaw();

    return x_MinAngleDiff( YawBox, m_EyesYaw );
}

//===========================================================================

radian player::CalculateActualPitchToTarget( object* pObject )
{
    vector3 vViewToBoxCenter = pObject->GetColBBox().GetCenter() - m_EyesPosition;

    // Calculate the differences in pitch
    radian PitchBox;
    PitchBox = vViewToBoxCenter.GetPitch();

    return x_MinAngleDiff( PitchBox, m_EyesPitch );
}

//===========================================================================

radian player::GetSightYaw( void ) const
{
    return m_EyesYaw;
}

//===========================================================================

player::animation_state player::SetupMutationMeleeWeapon( void )
{
    // Do not create a 3rd person camera for network ghosts.
    if( m_LocalSlot == -1 )
        return ANIM_STATE_MELEE;

    // set a default
    animation_state AnimState = ANIM_STATE_MELEE;

    // make sure we have our "mutant melee weapon" out
    if( !GetMutationMeleeWeapon() )
    {
        return AnimState;
    }

    /*  KSS -- FIXME -- melee tendrils removed for OPM Demo
    // setup defaults
    xbool bCanFireTendrils = TRUE;    
    guid DirectHitGuid = 0;
    actor *pActor = NULL;
    
#ifndef X_EDITOR
    if( GameMgr.GetGameType() != GAME_CAMPAIGN )
    {
        bCanFireTendrils = FALSE;
    }
#endif

    // we're in the editor or in campaign mode... we can check other tendril rules
    // If we're not in campaign mode, no need to check all this stuff.
    if( bCanFireTendrils )
    {
        // Fire a sphere out from the eye the correct distance and 
        // determine if we hit anything.
        vector3 HitPosition;
        {
            DoTendrilCollision();
            if( g_CollisionMgr.m_nCollisions )
            {
                DirectHitGuid = g_CollisionMgr.m_Collisions[0].ObjectHitGuid;
                HitPosition = g_CollisionMgr.m_Collisions[0].Point;
            }
        }
        
        // If there was no direct hit then there's nothing left to do
        if( DirectHitGuid==0 )
        {
            bCanFireTendrils = FALSE;
        }

        pain_handle PainHandle( xfs("%s_ExtremeMelee", GetLogicalName(), GetLogicalName()) );

        pain Pain;
        Pain.Setup( PainHandle, GetGuid(), GetColBBox().GetCenter() );

        // get the actor we hit
        pActor = (actor*)g_ObjMgr.GetObjectByGuid(DirectHitGuid);

        // don't have a target, they can't die or they ignore pain from us, get out of here
        if( !pActor || (!pActor->GetCanDie()) || pActor->IgnorePain(Pain) )
        {
            bCanFireTendrils = FALSE;
        }
    }

    // if we hit a living something, set the mutation weapon's target
    // if this is a mutant tank, just use hands
    if( pActor && DirectHitGuid && bCanFireTendrils && (!pActor->IsKindOf(mutant_tank::GetRTTI())) )
    {
    GetMutationMeleeWeapon()->SetTarget(DirectHitGuid);
    AnimState = ANIM_STATE_MUTATION_SPEAR;
    }
    else
    */

    // KSS -- FIXME -- mutation tendrils removed for OPM Demo
    {
        // only do this the first time
        g_AudioMgr.Play( "Mut_Melee" );

        // get the swat anim
        AnimState = GetNextMeleeState();
    }

    m_NextAnimState = ANIM_STATE_UNDEFINED;

    SetAnimation( AnimState, ANIM_PRIORITY_DEFAULT );
    GetMutationMeleeWeapon()->Setup( GetGuid(), AnimState );    

    return AnimState;
}

//===========================================================================
void player::DoTendrilCollision( void )
{
    tweak_handle ReachDistanceTweak("PLAYER_TendrilReachDistance");
    tweak_handle SphereRadiusTweak("PLAYER_TendrilCheckRadius");
    f32 MeleeReachDistance = ReachDistanceTweak.GetF32();
    f32 MeleeSphereRadius  = SphereRadiusTweak.GetF32();

    vector3 StartPos = GetView().GetPosition();
    vector3 EndPos   = StartPos + GetView().GetViewZ() * MeleeReachDistance;

    g_CollisionMgr.SphereSetup( GetGuid(), StartPos, EndPos, MeleeSphereRadius );
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_LIVING, object::ATTR_COLLISION_PERMEABLE );
}

//===========================================================================
weapon_mutation* player::GetMutationMeleeWeapon( void )
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();

    // make sure this is a mutation weapon
    if( pWeapon && pWeapon->IsKindOf( weapon_mutation::GetRTTI()) )
    {
        return (weapon_mutation*)pWeapon;
    }

    return NULL;
}

//===========================================================================

// ---------------------------------------------

new_weapon::reticle_radius_parameters player::GetReticleParams( void )
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();

    if( pWeapon )
    {
        if (m_CurrentAnimState == ANIM_STATE_ZOOM_IDLE ||
            m_CurrentAnimState == ANIM_STATE_ZOOM_RUN ||
            m_CurrentAnimState == ANIM_STATE_ZOOM_FIRE)
        {
            //alt fire
            return pWeapon->GetAltReticleRadiusParameters();
        }
        else
        {
            //standard fire
            return pWeapon->GetReticleRadiusParameters();
        }
    }
    else
    {
        new_weapon::reticle_radius_parameters Params;
        Params.m_CrouchBonus            = 0.0f;
        Params.m_GrowAccel              = 0.0f;
        Params.m_MaxMovementPenalty     = 0.0f;
        Params.m_MaxRadius              = 0.0f;
        Params.m_MoveShrinkAccel        = 0.0f;
        Params.m_PenaltyForShot         = 0.0f;
        Params.m_ShotPenaltyDegradeRate = 0.0f;
        Params.m_ShotShrinkAccel        = 0.0f;

        return Params;
    }
}

// ---------------------------------------------

void player::UpdateReticleRadius( f32 DeltaTime )
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();

    if( !pWeapon )
        return;

    new_weapon::reticle_radius_parameters ReticleParams = GetReticleParams();   

    ASSERT( SmallReticleSpeed > LargeReticleSpeed );
    ASSERT( ReticleParams.m_MaxRadius > ReticleParams.m_MinRadius );

    //
    // First, figure out what the movement penalty is
    //
    f32 Speed = m_Physics.GetVelocity().Length();
    f32 DesiredRadius = ReticleParams.m_MaxRadius; // we'll apply penalties for movement and shooting to this

    if ( Speed >= SmallReticleSpeed )
    {
        DesiredRadius -= ReticleParams.m_MaxMovementPenalty;
    }
    else if ( Speed <= LargeReticleSpeed )
    {
        DesiredRadius = ReticleParams.m_MaxRadius; // no penalty from movement
    }
    else
    {
        const f32 RelativeSpeed     = Speed - LargeReticleSpeed;                // Speed relative to reticle speed range
        const f32 SpeedRange        = SmallReticleSpeed - LargeReticleSpeed;

        ASSERT( RelativeSpeed > 0.0f );
        ASSERT( RelativeSpeed <= SpeedRange );

        f32 Penalty = MIN( ReticleParams.m_MaxMovementPenalty, ((RelativeSpeed / SpeedRange) * ReticleParams.m_MaxMovementPenalty) );
        DesiredRadius = ReticleParams.m_MaxRadius - Penalty;
    }


    //
    // Next, figure out what the shooting penalty is
    //

    // Degrade shot penalty
    m_ReticleShotPenalty -= ReticleParams.m_ShotPenaltyDegradeRate * DeltaTime;
    m_ReticleShotPenalty = MAX( 0.0f, m_ReticleShotPenalty );
    DesiredRadius -= m_ReticleShotPenalty;
    DesiredRadius = MAX( ReticleParams.m_MinRadius, DesiredRadius );

    //
    // Add in the crouch bonus as needed
    //
    if ( m_bIsCrouching )
    {
        DesiredRadius += ReticleParams.m_CrouchBonus;
    }


    //
    // Now update radius speed and current radius
    //
    if ( m_ReticleRadius > DesiredRadius)
    {
        if ( m_ReticleGrowSpeed > 0.0f )
        {
            // if growing, stop
            m_ReticleGrowSpeed = 0.0f;
        }

        // we need to shrink
        const f32 ShrinkAccel = (m_ReticleShotPenalty > 0.0f) ? ReticleParams.m_ShotShrinkAccel : ReticleParams.m_MoveShrinkAccel;
        m_ReticleGrowSpeed -= (ShrinkAccel * DeltaTime);
    }
    else if ( m_ReticleRadius < DesiredRadius )
    {
        if ( m_ReticleGrowSpeed < 0.0f )
        {
            // if shrinking, stop
            m_ReticleGrowSpeed = 0.0f;
        }

        // we need to grow
        m_ReticleGrowSpeed += (ReticleParams.m_GrowAccel * DeltaTime);
    }

    f32 GrowAmount = m_ReticleGrowSpeed * DeltaTime;
    f32 NewRadius = m_ReticleRadius + GrowAmount;

    if (   ((m_ReticleRadius < DesiredRadius) && (NewRadius > DesiredRadius))
        || ((m_ReticleRadius > DesiredRadius) && (NewRadius < DesiredRadius)) )
    {
        // we overshot our goal, just end up there
        NewRadius = DesiredRadius;
        m_ReticleGrowSpeed = 0.0f;
    }

    m_ReticleRadius = NewRadius;

    // Clamp
    const f32 Max = ReticleParams.m_MaxRadius + ReticleParams.m_CrouchBonus;
    if ( m_ReticleRadius > Max )
    { 
        m_ReticleRadius = Max;
        m_ReticleGrowSpeed = 0.0f;
    }
    else if ( (m_ReticleRadius < ReticleParams.m_MinRadius) && (GrowAmount < 0.0f) )
    {
        m_ReticleRadius = ReticleParams.m_MinRadius;
        m_ReticleGrowSpeed = 0.0f;
    }
}

//=============================================================================

vector3 player::GetWeaponCollisionOffset( guid WeaponGuid, const vector3& FirePos )
{
    const vector3 Dir      ( m_Pitch, m_Yaw );

    //
    // Come up with the point to use for our collision start.
    // It will be along the gun's aim axis
    //
    static const f32 Dist = 100.0f;
    vector3 WeaponStalk( FirePos - (Dir * Dist) );
    vector3 PtOnA;
    vector3 PtOnB;
    x_ClosestPtsOnLineSegs( WeaponStalk, FirePos, GetPosition(), GetPosition() + vector3( 0.0f, 200.0f, 0.0f ), PtOnA, PtOnB );

    vector3 Start( PtOnA );
    vector3 End( FirePos - (Dir * WeaponCollisionRadius) ); // back off by our radius

    g_CollisionMgr.SphereSetup( WeaponGuid, Start, End, WeaponCollisionRadius );   
    g_CollisionMgr.UseLowPoly();
    g_CollisionMgr.SetMaxCollisions( 1 );
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_PLAYER, object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING );

    // If we hit anything, figure out where we want to be
    f32 DesiredCollisionOffsetScalar = 0.0f;
    if ( g_CollisionMgr.m_nCollisions > 0 )
    {
        DesiredCollisionOffsetScalar = (Start - End).Length();
        DesiredCollisionOffsetScalar *= (1.01f - g_CollisionMgr.m_Collisions[0].T);
    }

    // Move towards our goal
    static f32 s_WeaponSlideSpeed = 150.0f;

    f32 Delta = DesiredCollisionOffsetScalar - m_WeaponCollisionOffsetScalar;
    if ( x_abs( Delta ) > 0.01f )
    {
        if ( m_LastWeaponCollisionOffsetScalar > DesiredCollisionOffsetScalar )
        {
            // We're moving the weapon forward, towards neutral
            // move this direction smoothly
            const f32 Distance = s_WeaponSlideSpeed * m_DeltaTime;
            if ( x_abs( Delta ) < Distance )
            {
                // we'd overshoot it, so just go there
                m_WeaponCollisionOffsetScalar = DesiredCollisionOffsetScalar;
            }
            else
            {
                if ( DesiredCollisionOffsetScalar > m_WeaponCollisionOffsetScalar )
                {
                    Delta = Distance;
                }
                else
                {
                    Delta = -Distance;
                }

                m_WeaponCollisionOffsetScalar += Delta;
            }
        }
        else
        {
            // We're moving the weapon back out of a wall, do this instantly
            m_WeaponCollisionOffsetScalar = DesiredCollisionOffsetScalar;
        }
    }
    else
    {
        m_WeaponCollisionOffsetScalar = DesiredCollisionOffsetScalar;
    }

    vector3 Pullback( -Dir );
    Pullback *= m_WeaponCollisionOffsetScalar;
    return Pullback;
}

//===========================================================================


void player::SetMeleeState( animation_state MeleeState )
{
    // Get a reference to the state that we are considering
    state_anims& State = m_Anim[inventory2::ItemToWeaponIndex(m_CurrentWeaponItem)][MeleeState];

    // Can we fire the secondary weapon?
    if( State.nPlayerAnims> 0 )
    {
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if( pWeapon )
            pWeapon->ClearZoom();

        SetAnimState( MeleeState );

#ifndef X_EDITOR
        // Play melee on 3rd person avatar
        net_Melee();
#endif // X_EDITOR

    }
}

//==============================================================================

xbool player::AllowedToFire( void )
{
    if( m_bHidePlayerArms )
    {
        return FALSE;
    }

    if( m_Cinema.m_bCinemaOn )
    {
        return FALSE;
    }

    return TRUE;
}

//==============================================================================

void player::OnWeaponSwitch2( const cycle_direction& CycleDirection )
{
    if( (m_CurrentAnimState == ANIM_STATE_GRENADE) ||
        (m_CurrentAnimState == ANIM_STATE_ALT_GRENADE) ||
        (m_CurrentAnimState == ANIM_STATE_DISCARD) )
        return;

    if( IsMutated() )
    {
        return;
    }

    //player is attempting to switch weapons, for now, we'll just cycle through the weapons that are in the inventory
    SetNextWeapon2( GetNextAvailableWeapon2( CycleDirection ) );
}

//==============================================================================

void player::SetNextWeapon2( inven_item WeaponItem, xbool ForceSwitch, xbool StateChange )
{
    if( (WeaponItem == m_CurrentWeaponItem) && !ForceSwitch )
    {
        // No need to switch to the current weapon
        return;
    }

    m_NextWeaponItem = WeaponItem;

    // turn off the flashlight
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon )
    {
        pWeapon->ClearZoom();

        // make sure we clear anything the weapon is doing
        pWeapon->BeginSwitchFrom();
    }
    else
    {
        // weapon is invalid?  Turn off flashlight then
        SetFlashlightActive( FALSE );
    }

    // Check weapon that's about to be the current weapon.
    pWeapon = GetWeaponPtr( WeaponItem );
    if( pWeapon )
    {
        // this new weapon doesn't have a flashlight, turn it off
        if( !pWeapon->HasFlashlight() )
        {
            SetFlashlightActive( FALSE );
        }
    }

    if( StateChange && !m_bDead )
    {
        // Switch to the next weapon.
        SetAnimState( ANIM_STATE_SWITCH_FROM );
    }

#ifndef X_EDITOR
    m_NetDirtyBits |= WEAPON_BIT;  // NETWORK
#endif // X_EDITOR 
}

//==============================================================================

xbool player::ShouldSkipWeaponCycle( const inven_item& CurrentWeaponItem, const inven_item& NextWeapon )
{
    switch( CurrentWeaponItem )
    {
    case INVEN_WEAPON_SMP:
        {
            if( NextWeapon == INVEN_WEAPON_DUAL_SMP )
            {
                return TRUE;
            }            
        }
        break;
    case INVEN_WEAPON_DUAL_SMP:
        {
            if( NextWeapon == INVEN_WEAPON_SMP )
            {
                return TRUE;
            }
        }
        break;

    case INVEN_WEAPON_SHOTGUN:
        {
            if( NextWeapon == INVEN_WEAPON_DUAL_SHT )
            {
                return TRUE;
            }            
        }
        break;
    case INVEN_WEAPON_DUAL_SHT:
        {
            if( NextWeapon == INVEN_WEAPON_SHOTGUN )
            {
                return TRUE;
            }
        }
        break;
    default:
        {
            return FALSE;
        }
        break;
    }

    return FALSE;
}

//=============================================================================

inven_item player::GetNextAvailableWeapon2( const cycle_direction& CycleDirection )
{
    inven_item CurrentWeaponItem = m_CurrentWeaponItem;

    //determine if we need to add or subtract from m_CurrentWeapon
    s32 Direction = 1;
    if ( CycleDirection == CYCLE_LEFT )
    {
        Direction = -1;
    }
    //select the next weapon in the list to test.
    u32 NextWeapon = (inven_item)(( (s32)CurrentWeaponItem + Direction ) % INVEN_NUM_WEAPONS);
    NextWeapon = ( NextWeapon>INVEN_NUM_WEAPONS ) ? INVEN_NUM_WEAPONS-1: NextWeapon;

    // TODO: CJ: WEAPONS: Check for infinite loop?
    while( (inven_item)NextWeapon != CurrentWeaponItem )
    {
        // should we skip this weapon (i.e. dual SMPs switching to/from SMP)
        if( ! ShouldSkipWeaponCycle(CurrentWeaponItem, (inven_item)NextWeapon) )
        {
            //if the weapon is in our inventory, that's is for the while loop. can't cycle to mutation weapon
            if( (NextWeapon != INVEN_WEAPON_MUTATION) && (m_Inventory2.HasItem( (inven_item)NextWeapon )) )
                break;
        }

        NextWeapon = (((s32)NextWeapon + Direction ) % INVEN_NUM_WEAPONS);
        NextWeapon = ( NextWeapon>INVEN_NUM_WEAPONS ) ? INVEN_NUM_WEAPONS-1: NextWeapon;
    }

    // Check for Dual SMP
    if( ((inven_item)NextWeapon == INVEN_WEAPON_SMP) && m_Inventory2.HasItem(INVEN_WEAPON_DUAL_SMP) )
    {
        NextWeapon = INVEN_WEAPON_DUAL_SMP;
    }
    else
    // Check for Dual Shotguns
    if( ((inven_item)NextWeapon == INVEN_WEAPON_SHOTGUN) && m_Inventory2.HasItem(INVEN_WEAPON_DUAL_SHT) )
    {
        NextWeapon = INVEN_WEAPON_DUAL_SHT;
    }

    return (inven_item)NextWeapon;
}

//=============================================================================

#if !defined( CONFIG_RETAIL )

void RenderPill( const vector3& Top, const vector3& Bot, f32 Radius, xcolor C )
{
    (void)Radius;
    const view* View = eng_GetView();
    //vector3 RightDir = -View->GetViewX() * Radius;
    vector3 Axis     = -View->GetViewZ();
    vector3 SpineDir = Top - Bot;
    plane EyePlane;
    EyePlane.Setup( View->GetPosition(), View->GetViewZ() );
    vector3 Par,Perp;
    EyePlane.GetComponents(SpineDir,Par,Perp);
    SpineDir = Par;
    SpineDir.Normalize();
    SpineDir *= Radius;

    s32 nSegs=16;
    s32 i;
    vector3 TP0,TP1;
    vector3 BP0,BP1;
    for( i=0; i<=nSegs; i++ )
    {
        TP0 = TP1;
        BP0 = BP1;
        radian R = (f32)i * (R_180 / (f32)nSegs);
        quaternion TQ( Axis, +R-R_90 );
        quaternion BQ( Axis, -R-R_90 );
        TP1 = (TQ * SpineDir) + Top;
        BP1 = (BQ * SpineDir) + Bot;

        if( (i==0) || (i==nSegs) )
            draw_Line(TP1,BP1,C);

        if( i>0 )
        {
            draw_Line(TP0,TP1,C);
            draw_Line(BP0,BP1,C);
        }
    }
}

#endif // !defined( CONFIG_RETAIL )

//===========================================================================

#if !defined( CONFIG_RETAIL ) && !defined( CONFIG_PROFILE )
#define RENDER_AIM_ASSIST 1
#else
#define RENDER_AIM_ASSIST 0
#endif

void player::RenderAimAssistDebugInfo( void )
{
#if( RENDER_AIM_ASSIST )

    // Render AimAssist
    const view* View = eng_GetView();

    // technically this extra if doesn't need to be here, but the "GetHeadAndRootPosition"
    // call can be UBER-expensive, and we should just avoid it completely if at all possible
    if( g_AimAssist_Render_Reticle || g_AimAssist_Render_Bullet || g_AimAssist_Render_Turn )
    {
        actor* pActor = actor::m_pFirstActive;
        while( pActor )
        {
            // only draw player pills if requested
            if( g_AimAssist_Render_Player_Pills || (pActor->GetGuid() != GetGuid()) )
            {
                //
                // Get spine information
                //
                vector3 SpineTop;
                vector3 SpineBot;
                {
                    ((character*)pActor)->GetHeadAndRootPosition( SpineTop, SpineBot );
                }

                //
                // Get closest pt between LOF and Spine
                //
                // m_AimAssistData.LOFSpineDist;
                //draw_Marker( m_AimAssistData.SpinePt, XCOLOR_WHITE );
                //draw_Marker( m_AimAssistData.LOFPt, XCOLOR_WHITE );
                if( g_AimAssist_Render_Reticle )
                {
                    f32 Radius = m_AimAssistData.ReticleRadius;
                    RenderPill( SpineTop, SpineBot, Radius, XCOLOR_RED );
                }

                if( g_AimAssist_Render_Bullet )
                {                        
                    f32 InnerRadius = m_AimAssistData.BulletInnerRadius;
                    f32 OuterRadius = m_AimAssistData.BulletOuterRadius;

                    RenderPill( SpineTop, SpineBot, InnerRadius, XCOLOR_BLUE );
                    RenderPill( SpineTop, SpineBot, OuterRadius, XCOLOR_BLUE );
                }

                if( g_AimAssist_Render_Turn )
                {
                    f32 InnerRadius = m_AimAssistData.TurnInnerRadius;
                    f32 OuterRadius = m_AimAssistData.TurnOuterRadius;

                    RenderPill( SpineTop, SpineBot, InnerRadius, XCOLOR_GREEN );
                    RenderPill( SpineTop, SpineBot, OuterRadius, XCOLOR_GREEN );
                }
            }

            pActor = pActor->m_pNextActive;
        }
    }

    if( g_AimAssist_Render_Bullet )
    {
        vector3 Pos;
        Pos = View->GetPosition() + View->GetViewZ()*150.0f;
        draw_Marker(Pos, XCOLOR_WHITE);
        Pos = View->GetPosition() + m_AimAssistData.BulletAssistDir*150.0f;
        draw_Marker(Pos, XCOLOR_WHITE);
    }

    if( g_AimAssist_Render_Bullet_Angle )
    {
        quaternion Q( View->GetViewY(), -AimAssist_Bullet_Angle );
        vector3 Dir = Q * View->GetViewZ() * 200.0f;
        s32 nSegs=32;
        s32 i;
        vector3 TP0,TP1;
        for( i=0; i<=nSegs; i++ )
        {
            TP0 = TP1;
            radian R = (f32)i * (R_360 / (f32)nSegs);
            quaternion Q(View->GetViewZ(),R);
            TP1 = (Q * Dir) + View->GetPosition();
            if( i>0 )
            {
                draw_Line(TP0,TP1,XCOLOR_YELLOW);
            }
        }
    }

#endif // #if( RENDER_AIM_ASSIST )
}

//===========================================================================

void player::AttachWeapon( void )
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if ( pWeapon )
    {
        radian3 Rot( m_AnimPlayer.GetPitch() , m_AnimPlayer.GetYaw(), m_AnimPlayer.GetRoll() );
        vector3 Pos( m_AnimPlayer.GetPosition() );

        matrix4 L2W;
        L2W.Identity();
        L2W.SetRotation( Rot );
        L2W.SetTranslation( Pos );

        //( (new_weapon*)pObject )->SetRotation
        //( (new_weapon*)pObject )->OnMove
        OnTransformWeapon( L2W );
        pWeapon->SetZone1( GetZone1() );
    }
}

//===========================================================================


void player::AddNewWeapon2( inven_item WeaponItem )
{
    // Already have the weapon?
    if( m_Inventory2.HasItem( WeaponItem ) )
    {
        return;
    }

    // Add the weapon
    m_Inventory2.AddAmount( WeaponItem, 1.0f );

    if( (m_CurrentWeaponItem == INVEN_NULL) && (WeaponItem != INVEN_WEAPON_MUTATION) )
    {
        m_CurrentWeaponItem = WeaponItem;

        // Set the render index.
        new_weapon* pWeapon = GetWeaponPtr( WeaponItem );
        if( pWeapon )
        {
            pWeapon->SetupRenderInformation( );
        }
    }
}

//===========================================================================

xbool player::TryAddAmmo2( inven_item Item )
{
    (void)Item;
    // TODO: CJ: WEAPONS: This code needs revisting to get ammo back into the weapons

    /*
    // lookup the weapon we are using.    
    player_virtual_weapon WeaponType = GetWeaponStateFromType( pWeapon->GetType() );

    // Couldn't find the weapon?
    if( WeaponType == WEAPON_UNDEFINED )
    {
    return FALSE;
    }

    //
    // mreed: ??? what's this code trying to do, and what does it have to do with dual? 5/8/2004
    //
    // If we have the weapon, check if we are maxed out on ammo.
    if( m_bWeaponInInventory[WeaponType] )
    {
    player_weapon_obj PlayerWeaponObj = GetWeaponObjFromVirtual( WeaponType );
    object_ptr<new_weapon> WeaponObj( m_GuidWeaponArray[PlayerWeaponObj] );

    s32 nCurrentAmmoCount = WeaponObj.m_pObject->GetTotalPrimaryAmmo();
    s32 nMaxAmmoCount     = WeaponObj.m_pObject->GetMaxPrimaryAmmo();

    // refill ammo if we are low
    if( nCurrentAmmoCount < nMaxAmmoCount )
    {
    ((new_weapon*)WeaponObj.m_pObject)->SetupDualAmmo();
    return TRUE;
    }
    }
    */
    return FALSE;
}

//===========================================================================

xbool player::ShouldSwitchToWeapon2( inven_item WeaponItem, xbool bFirstPickup )
{
    if( m_bDead )
    {
        return FALSE;
    }

    if( m_CurrentWeaponItem == INVEN_NULL )
    {
        // Player has no weapon -> Probably a good idea to switch
        return TRUE;
    }

    // make sure that if we are mutated, we don't switch weapons, AT ALL!
    if( IsMutated() )
    {
        return FALSE;
    }

    // TODO: CJ: WEAPONS: Revisit this to put the rules in place
    // mreed: temporary hack until we get all the rules defined and in place
    xbool RetVal = FALSE;

    // this is a defaulted value for the editor, in the game it will check a profile setting.
    xbool bShouldCheckRating = TRUE;
    
#ifndef X_EDITOR
    // get player profile
    player_profile& p = g_StateMgr.GetActiveProfile(g_StateMgr.GetProfileListIndex(m_LocalSlot));

    bShouldCheckRating = p.GetWeaponAutoSwitch();
#endif

    inven_item ParentItem = new_weapon::GetParentIDForDualWeapon( WeaponItem );

    // see if this is a dual weapon
    xbool bSwitchDualWeapon = FALSE;

    if( ParentItem != INVEN_NULL )
    {
        // is our current weapon the parent of this new weapon?
        // if so, this means we need to switch to the dual version
        bSwitchDualWeapon = ( m_CurrentWeaponItem == ParentItem );
    }

    // if weapon auto-switch is set, check weapon auto-switch ratings if this is the first time we picked it up.
    if( (bShouldCheckRating && bFirstPickup) || bSwitchDualWeapon )
    {
        new_weapon *pCurrentWeapon  = GetCurrentWeaponPtr();
        new_weapon *pNewWeapon      = GetWeaponPtr(WeaponItem);

        // if our new weapon is rated higher than current, switch to it.
        if(    pNewWeapon 
            && pCurrentWeapon 
            && (pNewWeapon->GetAutoSwitchRating() > pCurrentWeapon->GetAutoSwitchRating()) )
        {
            RetVal = TRUE;
        }
    }

    return RetVal;
}

//==============================================================================

void player::OnWeaponAnimInit2( inven_item WeaponItem, new_weapon* pWeapon )
{
    LOG_MESSAGE( "player::OnWeaponAnimInit", "" );

    // If the weapons has already been initialized with the player don't do it again.
    if( m_WeaponGuids[inventory2::ItemToWeaponIndex(WeaponItem)] != 0 )
        return;

    //reset this weapon's animation table
    ResetWeaponAnimTable2( WeaponItem );

    if ( pWeapon->IsInited() )
    {
        pWeapon->SetupRenderInformation( );

        if( pWeapon->HasAnimGroup() == FALSE )
            return;

        // Get the weapon's anim group that we're initializing.
        const anim_group& WeaponAnimGroup = pWeapon->GetCurrentAnimGroup();
        animation_state AnimIndex = ANIM_STATE_UNDEFINED;

        for ( s32 i = 0; i < WeaponAnimGroup.GetNAnims(); i++ )
        {
            AnimIndex = ANIM_STATE_UNDEFINED;

            const anim_info& AnimInfo   = WeaponAnimGroup.GetAnimInfo( i );
            const char*      pAnimName  = AnimInfo.GetName();

            // This animation is always there...
            if ( x_strcmp( pAnimName, "BIND_POSE" ) == 0 )
                continue;

            AnimIndex = GetAnimStateFromName( pAnimName );

            if ( AnimIndex == ANIM_STATE_UNDEFINED )
            {
                continue;
            }

            //We have valid index to the animation table, now set the values
            state_anims& State =  m_Anim[inventory2::ItemToWeaponIndex(WeaponItem)][AnimIndex];

            if( State.nWeaponAnims >= MAX_ANIM_PER_STATE )
            {
                x_try;
                x_throw( xfs( "WARNING: Too many animations of this type %s for weapon" , pAnimName ));
                x_catch_display;
                continue;
            }
            else
            {
                // TODO:  Eventually, weapons will need to contain animation sets for all
                //        player strains.  This sets all weapon animations to the human set for now.
                // Set the index of the animation
                ASSERT(State.nWeaponAnims < MAX_ANIM_PER_STATE);
                State.WeaponAnim[State.nWeaponAnims] = i;
                State.nWeaponAnims++;
            }
        }

        pWeapon->SetupRenderInformation( );
    }   
}

//==============================================================================

void player::GenerateFiringAnimPercentages( void )
{
    //get the current state.
    state_anims& State = m_Anim[inventory2::ItemToWeaponIndex(m_CurrentWeaponItem)][m_CurrentAnimState];

    //set up the percentages.  if there's 4 animations: .70 / .15 / .10 / .05
    if ( State.nPlayerAnims == MAX_ANIM_PER_STATE )
    {
        m_fAnimPriorityPercentage[0] = 0.70f;
        m_fAnimPriorityPercentage[1] = 0.85f;
        m_fAnimPriorityPercentage[2] = 0.95f;
        m_fAnimPriorityPercentage[3] = 1.00f;
    }

    //if there's 3 animations: .75 / .15 / .10 / 0
    else
        if ( State.nPlayerAnims == 3 )
        {
            m_fAnimPriorityPercentage[0] = 0.7f;
            m_fAnimPriorityPercentage[1] = 0.9f;
            m_fAnimPriorityPercentage[2] = 1.0f;
            m_fAnimPriorityPercentage[3] = 0.0f;
        }

        //if there's 2 animations: .75 / .25 / 0 / 0
        else
            if ( State.nPlayerAnims == 2 )
            {
                m_fAnimPriorityPercentage[0] = 0.75f;
                m_fAnimPriorityPercentage[1] = 1.00f;
                m_fAnimPriorityPercentage[2] = 0.00f;
                m_fAnimPriorityPercentage[3] = 0.00f;
            }

            //if there's 1 animations: 1.0 / 0 / 0 / 0
            else
                if ( State.nPlayerAnims == 1 )
                {
                    m_fAnimPriorityPercentage[0] = 1.0f;
                    m_fAnimPriorityPercentage[1] = 0.0f;
                    m_fAnimPriorityPercentage[2] = 0.0f;
                    m_fAnimPriorityPercentage[3] = 0.0f;
                }
                else
                {
                    ASSERT( FALSE );
                }
}

//==============================================================================

s32 player::GetNextFiringAnimIndex( void )
{
    //generate a random number between 0 and 1
    f32 fRand = x_frand( 0.0f, 1.0f );

    for ( s32 i = 0; i < MAX_ANIM_PER_STATE; i++ )
    {
        if ( fRand < m_fAnimPriorityPercentage[i] )
        {
            return i;
        }
    }

    ASSERT( FALSE );
    return 0;
}

//==============================================================================

void player::ResetWeaponAnimTable2( inven_item WeaponItem )
{
//  LOG_MESSAGE( "player::ResetWeaponAnimTable", "" );

    //clear the weapon animation array only.
    for ( s32 j = 0; j < ANIM_STATE_DEATH; j++ )
    {
        m_Anim[inventory2::ItemToWeaponIndex(WeaponItem)][j].nWeaponAnims = 0;
    }
}

//==============================================================================


void player::EmitMeleePain( void )
{
    tweak_handle ReachDistanceTweak("PLAYER_MeleeReachDistance");
    tweak_handle SphereRadiusTweak("PLAYER_MeleeSphereRadius");
    f32 MeleeReachDistance = ReachDistanceTweak.GetF32();
    f32 MeleeSphereRadius  = SphereRadiusTweak.GetF32();

    //
    // Fire a sphere out from the eye the correct distance and 
    // determine if we hit anything.
    //
    guid DirectHitGuid=0;
    vector3 HitPosition;
    {
        vector3 StartPos = GetView().GetPosition();
        vector3 EndPos   = StartPos + GetView().GetViewZ() * MeleeReachDistance;

        g_CollisionMgr.SphereSetup( GetGuid(), StartPos, EndPos, MeleeSphereRadius );
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_LARGE_PROJECTILES, object::ATTR_COLLISION_PERMEABLE );

        if( g_CollisionMgr.m_nCollisions )
        {
            DirectHitGuid = g_CollisionMgr.m_Collisions[0].ObjectHitGuid;
            HitPosition = g_CollisionMgr.m_Collisions[0].Point;
        }
    }

    // If there was no direct hit then there's nothing left to do
    if( DirectHitGuid==0 )
        return;

    // We hit something! Play a sound!
    if( m_PlayMeleeSound  && DirectHitGuid )
    {
        m_PlayMeleeSound = FALSE;

        // Create an event sound emitter.
        guid Guid = g_ObjMgr.CreateObject( event_sound_emitter::GetObjectType() );
        object* pSndEventObj = g_ObjMgr.GetObjectByGuid( Guid );

        event_sound_emitter& EventEmitter = event_sound_emitter::GetSafeType( *pSndEventObj );

        char DescName[64];
        x_sprintf( DescName, "Melee_%s", EventEmitter.GetMaterialName(g_CollisionMgr.m_Collisions[0].Flags) );

        EventEmitter.PlayEmitter(   DescName, 
            HitPosition, 
            GetZone1(), 
            event_sound_emitter::SINGLE_SHOT, 
            m_WeaponGuids[inventory2::ItemToWeaponIndex(m_CurrentWeaponItem)] );
        // hit something, set flag
        m_bLastMeleeHit = TRUE;
    }
    else
    {
        // hit nothing, set flag
        m_bLastMeleeHit = FALSE;
    }


    // Do shakes and feedback (skip if cloth is hit otherwise this looks weird)
    object* pHitObject = g_ObjMgr.GetObjectByGuid( DirectHitGuid );
    if(     ( pHitObject ) 
        &&  ( pHitObject->GetType() != object::TYPE_CLOTH_OBJECT )
        &&  ( pHitObject->GetType() != object::TYPE_FLAG ) )
    {
        if( g_PlayerMeleeShakeView )
        {
            ShakeView( s_MeleeShakeTime[m_ComboCount], s_MeleeShakeAmount[m_ComboCount], s_MeleeShakeSpeed[m_ComboCount] );
        }

        if( g_PlayerMeleeDoFeedback )
        {
            DoFeedback((s_MeleeShakeTime[m_ComboCount]/2.5f), s_MeleeFeedbackShakeAmount[m_ComboCount]);
        }
    }

    // Build pain
    pain Pain;
    {
        Pain.Setup(xfs("%s_MELEE_%d",GetLogicalName(),m_ComboCount),GetGuid(),HitPosition);
        Pain.SetDirection( GetView().GetViewZ() );
        Pain.SetDirectHitGuid( DirectHitGuid );
        Pain.SetCollisionInfo( g_CollisionMgr.m_Collisions[0] );
        Pain.ApplyToObject( DirectHitGuid );

        // melee impact FX
        particle_emitter::CreateProjectileCollisionEffect( g_CollisionMgr.m_Collisions[0], GetGuid() );
    }
}

//===========================================================================

const char* player::GetCurrentWeaponName( void )
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();

    if( pWeapon == NULL )
        return "";

    switch( pWeapon->GetType() )
    {
    case object::TYPE_WEAPON_SMP:           return "SMP";
        // KSS -- TO ADD NEW WEAPON
    case object::TYPE_WEAPON_SHOTGUN:       return "SHT";
    case object::TYPE_WEAPON_SCANNER:       return "SCN";
    case object::TYPE_WEAPON_SNIPER:        return "SNI";
    case object::TYPE_WEAPON_DESERT_EAGLE:  return "EGL";
    case object::TYPE_WEAPON_MSN:           return "MSN";
    case object::TYPE_WEAPON_BBG:           return "BBG";
    case object:: TYPE_WEAPON_TRA:          return "TRA";
    case object::TYPE_WEAPON_MUTATION:      return "MUT";
    default:                                return "";
    }
}

//=============================================================================

f32 player::GetLastTimeWeaponFired()
{
    return m_LastTimeWeaponFired;
}


void player::ResetWeaponFlags( void )
{

}

//===========================================================================

guid player::GetCurrentWeaponGuid2( void )
{
    return m_WeaponGuids[inventory2::ItemToWeaponIndex(m_CurrentWeaponItem)];
}

//===========================================================================


void player::SwitchWeapon2( inven_item WeaponItem )
{
    //==========================================================================================
    // Begin code from ghost.cpp
    //==========================================================================================

    if( WeaponItem != m_CurrentWeaponItem )
    {
        m_NextWeaponItem = WeaponItem;
    }

    //==========================================================================================
    // End code from ghost.cpp
    //==========================================================================================

    if( WeaponItem != m_CurrentWeaponItem )
    {
        //set the state to ANIM_STATE_SWITCH_FROM
        SetAnimState( ANIM_STATE_SWITCH_FROM );

        // turn off the flashlight
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if( !pWeapon || (!pWeapon->HasFlashlight()) )
        {
            // weapon is invalid?  Turn off flashlight then
            SetFlashlightActive(FALSE);            
        }
        else if( pWeapon->HasFlashlight() )
        {
            SetFlashlightActive(TRUE);
        }
    }
}

//===========================================================================

void player::OnMoveWeapon( void )
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();

    // update weapon position
    if( pWeapon )
    {
        // call weapon's OnMove
        pWeapon->OnMove( m_AnimPlayer.GetPosition());

        // update zones
        pWeapon->SetZone1( GetZone1() );

        // move the flashlight if it's active
        MoveFlashlight();        
    }
}

void player::OnTransformWeapon( const matrix4& L2W )
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon )
    {
        // call weapon's OnTransform
        pWeapon->OnTransform( L2W );

        // Update 3rd person weapon also so split screen works
        actor::MoveWeapon( FALSE );

        // move the flashlight if it's active
        MoveFlashlight();        
    }
}

//===========================================================================

void player::RemoveAllWeaponInventory( void )
{
    // Remove all the weapons from the inventory.
    for( s32 i=0 ; i<INVEN_NUM_WEAPONS ; i++ )
    {
        m_Inventory2.SetAmount( inventory2::WeaponIndexToItem(i), 0.0f );
    }

    // Set the next weapon type and switch from the current weapon.
    m_NextWeaponItem = INVEN_NULL;
    SetAnimState( ANIM_STATE_SWITCH_FROM );
}

//==============================================================================

s32 player::GetWeaponRenderState( void )
{
    return new_weapon::RENDER_STATE_PLAYER;
}

//==============================================================================
//=============================================================================

void player::HandleBulletFlyby( bullet_projectile& Bullet )
{
    vector3 ClosestPoint;
    vector3 End         = Bullet.GetCurrentPos();
    vector3 Start       = Bullet.GetInitialPos();
    vector3 Velocity    = Bullet.GetVelocity();
    vector3 EarPosition = GetPosition() + vector3( 0, 100, 0 );

    // Find closest point.
    ClosestPoint = EarPosition.GetClosestPToLSeg( Start, End );
    Velocity.Normalize();

    vector3 Delta = ClosestPoint - End;
    if( Delta.Length() > 1.0f )
    {
        // Should we play the fly by? 5 meter limit for now...
        vector3 EarToBullet    = ClosestPoint - EarPosition;
        f32     BulletDistance = EarToBullet.Length();
        if( BulletDistance < 500.0f )
        {
            // Look for an unused fly by...
            for( s32 i=0 ; i<MAX_FLY_BYS ; i++ )
            {
                // Is it active?
                if( !m_BulletFlyBy[i].bIsActive )
                {
                    // movement over 6 meters at current time...
                    m_BulletFlyBy[i].Start     = ClosestPoint - (Velocity * 400);
                    m_BulletFlyBy[i].End       = ClosestPoint + (Velocity * 1200);
                    m_BulletFlyBy[i].VoiceID   = g_AudioMgr.PlayVolumeClipped( "BulletFlyBy", m_BulletFlyBy[i].Start, GetZone1(), TRUE );
                    m_BulletFlyBy[i].Age       = 0.0f;
                    m_BulletFlyBy[i].Lifetime  = g_AudioMgr.GetLengthSeconds( m_BulletFlyBy[i].VoiceID );
                    m_BulletFlyBy[i].bIsActive = TRUE;
                    break;
                }
            }
        }
    }
}

//=============================================================================

void player::UpdateBulletSounds( f32 DeltaTime )
{
    // Look for active fly bys...
    for( s32 i=0 ; i<MAX_FLY_BYS ; i++ )
    {
        // Only if its active...
        if( m_BulletFlyBy[i].bIsActive )
        {
            // Update lifetime...
            m_BulletFlyBy[i].Age += DeltaTime;

            // Still alive?
            if( (m_BulletFlyBy[i].Age < m_BulletFlyBy[i].Lifetime) && g_AudioMgr.IsValidVoiceId( m_BulletFlyBy[i].VoiceID ) )
            {
                f32 Scale = m_BulletFlyBy[i].Age / m_BulletFlyBy[i].Lifetime; 
                vector3 Pos = m_BulletFlyBy[i].Start + Scale * (m_BulletFlyBy[i].End - m_BulletFlyBy[i].Start);
                g_AudioMgr.SetPosition( m_BulletFlyBy[i].VoiceID, Pos, GetZone1() );
            }
            else
            {
                // Kill it!
                m_BulletFlyBy[i].bIsActive = FALSE;
            }
        }
    }
}

//=============================================================================

xbool player::SetupDualWeaponDiscard( inven_item &WeaponItem )
{
    switch( WeaponItem )
    {
    case INVEN_WEAPON_DUAL_SMP:
        {
            // clear dual
            m_Inventory2.SetAmount( INVEN_WEAPON_DUAL_SMP, 0.0f );

            // Set next weapon to the SMP
            WeaponItem = INVEN_WEAPON_SMP;

            new_weapon* pWeapon = GetWeaponPtr( WeaponItem );

            // clear current weapon's clip ammo so that we'll have to reload once we dump the dual.            
            if( pWeapon )
            {                
                pWeapon->ClearClipAmmo();
            }
        }
        break;
    case INVEN_WEAPON_DUAL_SHT:
        {
            // clear dual
            m_Inventory2.SetAmount( INVEN_WEAPON_DUAL_SHT, 0.0f );

            // Set next weapon to the shotgun
            WeaponItem = INVEN_WEAPON_SHOTGUN;

            new_weapon* pWeapon = GetWeaponPtr( WeaponItem );

            // clear current weapon's clip ammo so that we'll have to reload once we dump the dual.            
            if( pWeapon )
            {                
                pWeapon->ClearClipAmmo();
            }
        }
        break;
        /*
    case INVEN_WEAPON_DUAL_EAGLE:
        {
            // clear dual
            m_Inventory2.SetAmount( INVEN_WEAPON_DUAL_SMP, 0.0f );

            // Set next weapon to the DE
            m_CurrentWeaponItem = INVEN_WEAPON_DESERT_EAGLE;

            new_weapon* pWeapon = GetWeaponPtr( WeaponItem );

            // clear current weapon's clip ammo so that we'll have to reload once we dump the dual.            
            if( pWeapon )
            {                
                pWeapon->ClearClipAmmo();
            }
        }
        break;
        */
    default:
        {
            // no dual discard
            return FALSE;
        }
        break;
    }

    return TRUE;
}

//=============================================================================

xbool player::CheckForDualWeaponSetup( void )
{
    xbool RetVal = FALSE;

    // set up dual SMPs
    if( m_Inventory2.HasItem(INVEN_WEAPON_SMP) && !m_Inventory2.HasItem(INVEN_WEAPON_DUAL_SMP))
    {
        m_Inventory2.SetAmount( INVEN_WEAPON_DUAL_SMP, 1.0f );
        SetNextWeapon2( INVEN_WEAPON_DUAL_SMP );

        new_weapon* pWeapon = (new_weapon*)g_ObjMgr.GetObjectByGuid( m_WeaponGuids[inventory2::ItemToWeaponIndex(INVEN_WEAPON_DUAL_SMP)] );
        if( pWeapon )
        {
            // make sure this weapon's clip is full
            pWeapon->RefillClip(new_weapon::AMMO_PRIMARY);
        }

        // pick this SMP up, will dual wield it.
        RetVal = TRUE;
    }
    else
    {
        if( m_Inventory2.HasItem(INVEN_WEAPON_DUAL_SMP) )
        {
            // already have one, don't pick it up
            RetVal = FALSE;    
        }
        else
        {
            m_Inventory2.SetAmount( INVEN_WEAPON_SMP, 1.0f );

            // don't have one at all yet, pick it up
            RetVal = TRUE;
        }
    }

    return RetVal;
}
//==============================================================================

s32 player::GetAmmoFromWeaponType(inven_item Item)
{
    s32 Amount = 0;

    switch( Item )
    {
    case INVEN_WEAPON_SMP:
    case INVEN_WEAPON_SHOTGUN:
    case INVEN_WEAPON_DESERT_EAGLE:
    case INVEN_WEAPON_SNIPER_RIFLE:
        {
            // Find the weapon from item type and get the amount of max ammo per clip and return it
            new_weapon* pWeapon = (new_weapon*)g_ObjMgr.GetObjectByGuid( m_WeaponGuids[inventory2::ItemToWeaponIndex(Item)] );
            if( pWeapon )
            {
                Amount = pWeapon->GetAmmoPerClip(new_weapon::AMMO_PRIMARY);
            }
        }
        break;
    default:
    case INVEN_WEAPON_DUAL_SMP:
    case INVEN_WEAPON_DUAL_SHT:
        //case INVEN_WEAPON_MESON_CANNON: // don't do anything for meson cannon?
    case INVEN_WEAPON_BBG: // don't do anything for BBG?
        {
            Amount = 0; // make sure
        }
        break;
    }

    return Amount;
}

//==============================================================================

#ifdef cgalley
#define LOGGING_ENABLED 1
#else
#define LOGGING_ENABLED 0
#endif

//==============================================================================

void player::CreateAllWeaponObjects( void )
{
    LOG_MESSAGE( "player::CreateAllWeaponObjects", "Creating weapons" );

    // Create the weapons for the player
    for( s32 i=0; i<INVEN_NUM_WEAPONS; i++ )
    {
        inven_item  WeaponItem      = inventory2::WeaponIndexToItem(i);
        const char* pBlueprintName  = inventory2::ItemToBlueprintName( WeaponItem );

        if( pBlueprintName )
        {
            // Get the weapon or create the weapon
            guid WeaponGUID = m_WeaponGuids[i];
            new_weapon* pWeapon = (new_weapon*)g_ObjMgr.GetObjectByGuid( WeaponGUID );
            if( pWeapon == NULL )
            {
                m_WeaponGuids[i] = 0;
                WeaponGUID = g_TemplateMgr.CreateSingleTemplate( pBlueprintName, vector3(0,0,0), radian3( 0,0,0 ), 0, 0 );
            }

            if( WeaponGUID )
            {
                new_weapon* pWeapon = (new_weapon*)g_ObjMgr.GetObjectByGuid( WeaponGUID );
                ASSERT( pWeapon );

                pWeapon->InitWeapon( GetPosition(), new_weapon::RENDER_STATE_PLAYER, GetGuid() );
                OnWeaponAnimInit2( WeaponItem, pWeapon );
                pWeapon->SetupRenderInformation( );
                pWeapon->SetAnimation( ANIM_STATE_IDLE, 0.0f );
                pWeapon->BeginIdle();

                // make sure m_AmmoInCurrentClip and such are set up properly
                pWeapon->Reload(new_weapon::AMMO_PRIMARY);

                m_WeaponGuids[i] = WeaponGUID;

                CLOG_MESSAGE( LOGGING_ENABLED, "player::CreateAllWeaponObjects", "Created: %s", inventory2::ItemToName( WeaponItem ) );
            }
        }
    }

    //----------------------------------------------------------
    // HACK: mreed since the flashlight doesn't save/restore,
    // we're killing it and setting our flashlight guid to
    // null. Then we call InitFlashlight, and everything works.
    //----------------------------------------------------------
    if ( m_FlashlightGuid )
    {
        if ( g_ObjMgr.GetObjectByGuid( m_FlashlightGuid ) )
        {
            g_ObjMgr.DestroyObject( m_FlashlightGuid );
        }
        m_FlashlightGuid = 0;
    }
    InitFlashlight( GetPosition() );
}

//------------------------------------------------------------------------------
xbool player::ReloadWeapon( const new_weapon::ammo_priority& Priority, xbool bCheckAmmo )
{
    s32 ammoCount = 0;
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( !pWeapon )
        return FALSE;

    // do NOT reload the mutation weapon
    if( m_CurrentWeaponItem == INVEN_WEAPON_MUTATION )
    {
        return FALSE;
    }

    // if we don't want to check the ammo (i.e. player pushed "reload button") then ignore count
    if( bCheckAmmo )
    {   
        if( Priority == new_weapon::AMMO_PRIMARY )
        {
            ammoCount = pWeapon->GetAmmoCount( pWeapon->GetPrimaryAmmoPriority() ); 
        }
        else
        {
            ammoCount = pWeapon->GetAmmoCount( pWeapon->GetSecondaryAmmoPriority() ); 
        }
    }

    // If our clip is out of ammo, reload
    // KSS -- always use AMMO_PRIMARY when checking reload?
    // if we don't want to check the ammo (i.e. player pushed "reload button") then ignore count
    if ( ( (ammoCount <= 0) || (!bCheckAmmo) ) && pWeapon->CanReload( new_weapon::AMMO_PRIMARY ) )
    {
        // CJ: Reset the zoom state in case of dry fire on sniper rile, etc.
        pWeapon->ClearZoom();

        SetAnimState( ANIM_STATE_RELOAD );

        // succeeded
        return TRUE;
    }

    // didn't reload
    return FALSE;
}

//==============================================================================

xbool player::LoadWarnsLowAmmo( void )
{
    // Determine if this level is supposed to warn the player that they are low on clip ammo with a message
    xbool bShouldWarn = FALSE;

    for( s32 i=0; s_MapToWeaponTable[i].pLevelName ; i++ )
    {
        if( x_stristr( g_ActiveConfig.GetLevelPath(), s_MapToWeaponTable[i].pLevelName ) )
        {
            bShouldWarn   = s_MapToWeaponTable[i].bLoadWarnsLowAmmo;
            break;
        }
    }

    return bShouldWarn;
}

//==============================================================================


void player::DebugEnableWeapons( const char* pLevelName )
{
    DebugSetupInventory( pLevelName );

    inven_item  StartItem   = GetEquipedWeaponForLevel( pLevelName );
    s32         Weapons     = GetStartWeaponsForLevel ( pLevelName );

    // Setup start item
    m_PreMutationWeapon2 = INVEN_NULL;
    m_CurrentWeaponItem = StartItem;
    if( m_CurrentWeaponItem == INVEN_WEAPON_MUTATION )
    {
        // Force the player to mutate
        m_PreMutationWeapon2 = INVEN_WEAPON_SMP;
        //        m_CurrentWeaponItem = INVEN_WEAPON_SMP;
        SetNextWeapon2( INVEN_WEAPON_MUTATION, TRUE );

        //        SetMutated( TRUE );
        //        m_bIsMutantVisionOn = TRUE;
    }

    // Enable mutation abilities
    if( Weapons & WB_MM )
    {
        m_bMutationMeleeEnabled         = TRUE;
    }

    if( Weapons & WB_MP )
    {
        m_bPrimaryMutationFireEnabled   = TRUE;
    }

    if ( Weapons & WB_MS )
    {
        m_bSecondaryMutationFireEnabled = TRUE;
    }

    // initialize flashlight
    vector3 rInitPos = m_AnimPlayer.GetPosition();
    InitFlashlight(rInitPos);

    // Load tweaks
    LoadAimAssistTweakHandles();
}

//==============================================================================

