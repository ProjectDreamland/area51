//==============================================================================
//
//  Actor.hpp
//
//  Base class for net_ghost, player, character.
//
//==============================================================================

#ifndef ACTOR_HPP
#define ACTOR_HPP

//=========================================================================
// INCLUDES
//=========================================================================
 
#include "NetworkMgr\NetObj.hpp"
#include "NetworkMgr\UpdateMgr.hpp"

#include "Objects\CorpsePain.hpp"
#include "Objects\NewWeapon.hpp"
#include "Objects\Pickup.hpp"
#include "Characters\factions.hpp"
#include "ZoneMgr\ZoneMgr.hpp"
#include "Inventory\Inventory2.hpp"
#include "Characters\FloorProperties.hpp"
#include "Loco\LocoAnimController.hpp"
#include "Loco\Loco.hpp"
#include "Decals\DecalPackage.hpp"

//==============================================================================

class actor_effects;
class pain_queue;
struct net_pain; 
class corpse;
class check_point_mgr;

//==============================================================================
// NETWORK SPECIFIC
//==============================================================================
#ifndef X_EDITOR
//------------------------------------------------------------------------------

//
// The update structure is used to hold data transfered from the server to 
// clients over the network.  It is used by classes actor, net_ghost, player, 
// and character.
//

struct update
{
    u32             DirtyBits;      // From net_ghost::dirty_bits.
    s32             Slot;           // Only used for debugging.  Not sent on wire.

    vector3         Position;               // POSITION_BIT
    radian          Pitch;                  // ORIENTATION_BIT
    radian          Yaw;                    // ORIENTATION_BIT
    s32             TargetNetSlot;          // ORIENTATION_BIT
    vector3         AimOffset;              // ORIENTATION_BIT
    xbool           Crouch;                 // AIR_CROUCH_BIT
    xbool           Airborn;                // AIR_CROUCH_BIT
                                            // CONTAGION_OFF_BIT

    f32             IncHealth;              // HEALTH_BIT
    f32             DecHealth;              // HEALTH_BIT
    s32             LifeSeq;                // LIFE_BIT
    u32             WeaponBits;             // LIFE_BIT
    s32             AmmoSMP;                // LIFE_BIT
    s32             AmmoShotgun;            // LIFE_BIT
    s32             AmmoSniper;             // LIFE_BIT
    s32             AmmoEagle;              // LIFE_BIT
    s32             AmmoMSN;                // LIFE_BIT
    s32             AmmoGrensFrag;          // LIFE_BIT
    s32             AmmoGrensJB;            // LIFE_BIT
    u32             TeamBits;               // TEAM_BITS_BIT
    f32             Lean;                   // LEAN_BIT

    xbool           Mutated;                // MUTATE_BIT
    s32             Weapon;                 // WEAPON_BIT
                                            // RELOAD_BIT
    s32             CurrentAmmo;            // CURRENT_AMMO_BIT

    s32             CurrentFireMode;        // FIRE_BIT
    s32             CurrentFireWeapon;      // FIRE_BIT & FIRE_BEGIN_BIT
                                            // FIRE_END_BIT
    s32             CurrentFireSeq;         // FIRE_BEGIN_BIT & FIRE_END_BIT matching                                            
    xbool           Flashlight;             // FLASHLIGHT_BIT

    s32             CurrentFragGrenades;    // TOSS_BIT
    s32             CurrentJBeanGrenades;   // TOSS_BIT
                                            // MELEE_BIT
                                            // DROP_ITEM_BIT
    s32             iAnim;                  // PLAY_ANIM_BIT
    f32             AnimBlendTime;          // PLAY_ANIM_BIT
    u32             AnimFlags;              // PLAY_ANIM_BIT
    f32             AnimPlayTime;           // PLAY_ANIM_BIT

    s32             DesiredTeam;            // WANT_NEW_TEAM_BIT
    xbool           ForceHuman;             // MUTATION_FLAGS_BIT
    xbool           ForceMutant;            // MUTATION_FLAGS_BIT
    s32             MutagenBurnMode;        // MUTATION_FLAGS_BIT
    u32             WayPointFlags;          // WAYPOINT_BIT
    vector3         WayPoint[2];            // WAYPOINT_BIT
    u32             EffectsFlags;           // EFFECTS_BIT

    xbool           WantToSpawn;            // WANT_SPAWN_BIT
    xbool           VoteCanCast;            // VOTE_FLAGS_BIT
    u32             VoteCanStartKick;       // VOTE_FLAGS_BIT
    xbool           VoteCanStartMap;        // VOTE_FLAGS_BIT
    s32             VoteAction;             // VOTE_ACTION_BIT
    s32             VoteArgument;           // VOTE_ACTION_BIT
    xbool           Contagion;              // CONTAGION_ON_BIT 
    s32             ContagionOrigin;        // CONTAGION_ON_BIT 

    s32             Skin;                   // ACTIVATE_BIT
    s32             VoiceActor;             // ACTIVATE_BIT
    corpse_pain     CorpseDeathPain;        // ACTIVATE_BIT
    
                                            // DEACTIVATE_BIT

                    update  ( void );
    void            Read    ( const bitstream& BitStream );
    void            Write   (       bitstream& BitStream );
};
             
//------------------------------------------------------------------------------

enum dirty_bits
{
    POSITION_BIT        = 0x00000001,
    ORIENTATION_BIT     = 0x00000002,
    AIR_CROUCH_BIT      = 0x00000004,
    CONTAGION_OFF_BIT   = 0x00000008,

    HEALTH_BIT          = 0x00000010,
    LIFE_BIT            = 0x00000020,
    TEAM_BITS_BIT       = 0x00000040,
    LEAN_BIT            = 0x00000080,

    MUTATE_BIT          = 0x00000100,
    WEAPON_BIT          = 0x00000200,
    RELOAD_BIT          = 0x00000400,
    CURRENT_AMMO_BIT    = 0x00000800,

    FIRE_BIT            = 0x00001000,
    FIRE_BEGIN_BIT      = 0x00002000,
    FIRE_END_BIT        = 0x00004000,
    FLASHLIGHT_BIT      = 0x00008000,

    TOSS_BIT            = 0x00010000,
    MELEE_BIT           = 0x00020000,
    DROP_ITEM_BIT       = 0x00040000,
    PLAY_ANIM_BIT       = 0x00080000,

    WANT_NEW_TEAM_BIT   = 0x00100000,
    MUTATION_FLAGS_BIT  = 0x00200000,
    WAYPOINT_BIT        = 0x00400000,
    EFFECTS_BIT         = 0x00800000,

    WANT_SPAWN_BIT      = 0x01000000,
    VOTE_FLAGS_BIT      = 0x02000000,
    VOTE_ACTION_BIT     = 0x04000000,
    CONTAGION_ON_BIT    = 0x08000000,

//  ACTIVATE_BIT        = 0x80000000,
//  DEACTIVATE_BIT      = 0x40000000,
};

enum WAYPOINT_BITS
{
    WAYPOINT_TELEPORT       = 0x00000001,
    WAYPOINT_TELEPORT_FX    = 0x00000002,
    WAYPOINT_JUMP_PAD_FX    = 0x00000004,
};

//------------------------------------------------------------------------------
#endif // X_EDITOR
//==============================================================================

//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================

//=========================================================================
//  HEALTH CLASS
//=========================================================================

class actor_health
{
protected:

        f32     m_Health;
        f32     m_Inc;
        f32     m_Dec;

public:
        actor_health    ( void );        
       ~actor_health    ( void ) { };      
        
void    Reset      ( f32 Health = 100.0f );  // Set Health and Inc, clear Dec.
void    Dead       ( void );       // Set Dec to Inc, and thus Health to 0.
void    Add        ( f32 Delta, xbool bForce = TRUE );  // Add to health.
void    Sub        ( f32 Delta, xbool bForce = TRUE );  // Subtract from health.
void    SetInc     ( f32 Inc   );  // If greater, use new Inc, update Health.
void    SetDec     ( f32 Dec   );  // If greater, use new Dec, update Health.
f32     GetHealth  ( void ) { return( m_Health ); };  // Get current health.
f32     GetInc     ( void ) { return( m_Inc ); }; // Get ALL increases to health.
f32     GetDec     ( void ) { return( m_Dec ); }; // Get ALL reductions in health.
};

//=========================================================================

inline actor_health::actor_health( void )
{
    m_Health = -42.0f;   // Nonsense value.
    m_Inc    =   0.0f;
    m_Dec    = -42.0f;
}

//=========================================================================

inline void actor_health::Reset( f32 Health )
{
    m_Health = Health;
    m_Inc    = Health;
    m_Dec    = 0.0f;
}

//=========================================================================

inline void actor_health::Dead( void )
{
    m_Health = 0.0f;
    m_Inc    = m_Dec;
}

//=========================================================================

inline void actor_health::Add( f32 Delta, xbool bForce /* = TRUE */ )
{
    if( Delta > 0.0f )      m_Inc += Delta;
    else                    m_Dec -= Delta;
    m_Health = m_Inc - m_Dec;
    if( m_Health < 0.0f )   
        m_Health = 0.0f;
    else if ( !bForce && (m_Health > 100.0f) )
    {
        m_Health = 100.0f;
        m_Inc = m_Health + m_Dec;
    }
}

//=========================================================================

inline void actor_health::Sub( f32 Delta, xbool bForce /* = TRUE */ )
{
    if( Delta > 0.0f )      m_Dec += Delta;
    else                    m_Inc -= Delta;
    m_Health = m_Inc - m_Dec;
    if( m_Health < 0.0f )   
        m_Health = 0.0f;
    else if ( !bForce && (m_Health > 100.0f) )
    {
        m_Health = 100.0f;
        m_Dec = m_Inc - m_Health;
    }
}

//=========================================================================

inline void actor_health::SetInc( f32 Inc )
{
    if( Inc > m_Inc )
    {
        m_Inc    = Inc;
        m_Health = m_Inc - m_Dec;
    }
}

//=========================================================================

inline void actor_health::SetDec( f32 Dec )
{
    if( Dec > m_Dec )
    {
        m_Dec    = Dec;
        m_Health = m_Inc - m_Dec;
        if( m_Health < 0.0f )   
            m_Health = 0.0f;
    }
}

//=========================================================================
// ACTOR CLASS
//=========================================================================

class actor : public netobj
{
//=========================================================================
// DEFINES
//=========================================================================
public:

    // Dialog types
    enum eDialogType
    {
        DIALOG_NONE = 0,
        DIALOG_ALERT,
        DIALOG_RUSH,
        DIALOG_KILL,
        DIALOG_CLEAR,
        DIALOG_FLEE,
        DIALOG_FRIENDLY_HIT,
        DIALOG_FRIENDLY_WOUND,
        DIALOG_GRENADE_THROW,
        DIALOG_GRENADE_SPOT,
        DIALOG_COVER,
        DIALOG_COVER_REQ,
        DIALOG_MANDOWN,
        DIALOG_RELOAD,
        DIALOG_UNDER_FIRE,
        DIALOG_HIT,
        DIALOG_HIT_MELEE,
        DIALOG_FLINCH,
        DIALOG_DIE_MELEE,
        DIALOG_DIE_GUNFIRE,
        DIALOG_DIE_EXPLOSION,
        DIALOG_DIE_FALL,
        DIALOG_TYPE_COUNT = DIALOG_DIE_FALL,

        DIALOG_SCANNER_VO,
        DIALOG_ANIM_EVENT,
        DIALOG_GOAL_DIALOG,
        DIALOG_TRIGGER_NO_INTERRUPT,
    };

    // Deaths
    enum death_type
    {
        DEATH_BY_ANIM,
        DEATH_BY_EXPLOSION,
        DEATH_BY_RAGDOLL,
        DEATH_SIMPLE
    };

    // Locomotion character animation player controller mappings
    enum anim_flags
    {
        ANIM_FLAG_IMPACT_CONTROLLER = loco::ANIM_FLAG_CONTROLLER0,
        ANIM_FLAG_SHOOT_CONTROLLER  = loco::ANIM_FLAG_CONTROLLER1
    } ;
 
    // General hit location
    enum general_hit_location
    {
        HIT_INVALID = -1,
        HIT_HIGH,
        HIT_MIDDLE,
        HIT_LOW
    };

    // Hit type - used to decide which impact animations to play
    enum eHitType
    {
        HITTYPE_HARD,
        HITTYPE_LIGHT,
        HITTYPE_IDLE,
        HITTYPE_PLAYER_MELEE_1,
    };
    
    // Motion direction when dying
    enum death_motion_direction
    {
        DEATH_MOVE_INVALID = -1, 
        DEATH_MOVE_LEFT,
        DEATH_MOVE_RIGHT,
        DEATH_MOVE_FORWARD,
        DEATH_MOVE_BACK
    };

    // Offsets to use when returning position
    enum eOffsetPos
    {
        OFFSET_NONE,                // None
        OFFSET_CENTER,              // ?
        OFFSET_EYES,                // Between the eyes (if the actor has both of them)
        OFFSET_AIM_AT,              // Insert ammo here
        OFFSET_TOP_OF_BBOX,         // ?
    };

    // Avatar mutation state info
    enum avatar_mutation_state
    {
        AVATAR_NORMAL,
        AVATAR_MUTATING,
        AVATAR_MUTANT,
        AVATAR_NORMALING // mreed: heh heh
    };

    enum mutagen_burn_mode
    {
        MBM_NORMAL_CAMPAIGN,
        MBM_FORCED,
        MBM_AT_WILL,
        MBM_MAX
    };

    // cloaking state info
    enum cloak_state
    {
        CLOAKING_ON,
        CLOAKING_TURNING_OFF,
        CLOAKING_OFF,
        CLOAKING_TURNING_ON,
    };

    // Lean state
    enum lean_state 
    {
        LEAN_NONE,
        LEAN_LEFT,
        LEAN_RIGHT,
        LEAN_RETURN_FROM_LEFT,
        LEAN_RETURN_FROM_RIGHT,
        
        LEAN_COUNT
    };

//=========================================================================
// FUNCTIONS
//=========================================================================
public:
     CREATE_RTTI( actor, netobj, object );
    
    // Constructor/destructor
        actor();
virtual ~actor();

        // Property functions
virtual void            OnEnumProp              ( prop_enum&     List     );
virtual xbool           OnProperty              ( prop_query&    I        );

#ifdef X_EDITOR
virtual s32             OnValidateProperties    ( xstring&       ErrorMsg );
#endif

        // Object virtual functions
virtual void            OnInit                  ( void );
virtual void            OnKill                  ( void );
virtual void            OnAdvanceLogic          ( f32 DeltaTime );
virtual void            OnAliveLogic            ( f32 DeltaTime );
virtual void            OnDeathLogic            ( f32 DeltaTime );
virtual void            OnRender                ( void );
virtual void            OnRenderShadowCast      ( u64 ProjMask );
virtual void            OnRenderTransparent     ( void );
virtual void            OnRenderWeapon          ( void );
virtual void            OnColCheck              ( void );    
virtual void            OnMove                  ( const vector3& NewPos );
virtual void            OnTransform             ( const matrix4& L2W );
virtual bbox            GetColBBox              ( void );     
virtual bbox            GetLocalBBox            ( void ) const;
virtual vector3         GetVelocity             ( void ) const;          

        void            OnAdvanceGhostLogic     ( f32 DeltaTime );

#ifndef X_RETAIL
virtual void            OnColRender             ( xbool bRenderHigh );
#endif // X_RETAIL

        // Health functions
        xbool           AddHealth               ( f32 DeltaHealth );
virtual f32             GetHealth               ( void );
virtual void            ResetHealth             ( f32 Health );
virtual f32             GetMaxHealth            ( void );
virtual void            SetMaxHealth            ( f32 MaxHealth );
virtual f32             GetParametricHealth     ( void );
        xbool           DoAliveLogic            ( void );
virtual xbool           GetCanDie               ( void )          { return m_bCanDie;    }
        void            SetCanDie               ( xbool bCanDie ) { m_bCanDie = bCanDie; }

        // Faction/friend functions
        factions        GetFaction              ( void ) const;
        factions        GetFactionForGuid       ( guid Guid ) const;
        void            SetFaction              ( factions NewFaction );
        xbool           IsFriendlyFaction       ( factions Faction ) const;
        xbool           IsEnemyFaction          ( factions Faction ) const;
        void            SetFriendFlags          ( u32 NewFriends );
        u32             GetFriendFlags          ( void ) const;
        xbool           IsAlly                  ( const actor* pActor ) const;
        xbool           IsEnemy                 ( const actor* pActor ) const;
        
        // Inventory functions
virtual void            ReloadAllWeapons        ( void );
virtual xbool           AddItemToInventory2     ( inven_item Item );
virtual xbool           RemoveItemFromInventory2( inven_item Item, xbool bRemoveAll = false );
virtual void            AddAmmoToInventory2     ( inven_item Item, s32 Amount );
virtual void            ClearInventory2         ( void );
        inventory2&     GetInventory2           ( void ) { return m_Inventory2; }
virtual void            InitInventory           ( void );
        void            ReInitInventory         ( void );
        xbool           HasItemInInventory2     ( inven_item Item );
virtual xbool           OnPickup                ( pickup& Pickup );
virtual xbool           InTurret                ( void ) { return FALSE; }
        // Kill/damage functions
virtual void            OnKilledEnemy           ( guid deadEnemy )      { (void)deadEnemy;      };
virtual void            OnKilledAlly            ( guid deadAlly )       { (void)deadAlly;       };
virtual void            OnDamagedEnemy          ( guid damagedEnemy )   { (void)damagedEnemy;   };
virtual void            OnDamagedAlly           ( guid damagedAlly )    { (void)damagedAlly;    };
virtual void            OnBeingShotAt           ( object::type ProjectileType , guid ShooterID ) { (void)ProjectileType; (void)ShooterID; }
virtual void            OnHitFriendly           ( guid FriendlyID )     { (void)FriendlyID; } //do nothing
virtual void            OnHitByFriendly         ( guid ShooterID  )     { (void)ShooterID; }
virtual xbool           OnPlayFullBodyImpactAnim( loco::anim_type AnimType, f32 BlendTime, u32 Flags );
virtual void            OnDeath                 ( void );
virtual void            OnSpawn                 ( void );
virtual f32             GetSpawnFadeTime        ( void ) { return m_SpawnFadeTime; }
        corpse*         CreateCorpseObject      ( xbool BodyFades = TRUE );
        void            CreateCorpse            ( void );
        guid            GetCorpseGuid           ( void ) { return m_CorpseGuid; }
        xbool           IsAlive                 ( void ) const { return( !m_bDead ); };
        xbool           IsDead                  ( void ) const { return(  m_bDead ); };
        xbool           HasSpawnInvulnerability ( void ) const { return (m_SpawnNeutralTime > 0)?TRUE:FALSE; }
        void            SetSpawnNeutralTime     ( f32 Time ) { m_SpawnNeutralTime = Time; }

#if !defined(CONFIG_RETAIL)
        void            DebugSuicide            ( void );
#endif

// Type functions
virtual xbool           IsPlayer                ( void )    { return FALSE; }
virtual xbool           IsCharacter             ( void )    { return FALSE; }
virtual xbool           IsNetGhost              ( void )    { return FALSE; }

// Tweak stuff
virtual f32             ModifyDamageByDifficulty( f32 Damage );
virtual f32             ModifyDamageByTurret    ( f32 Damage );

// Locomotion functions
virtual void            InitLoco                ( void );
virtual xbool           IsRunning               ( void );
        xbool           IsMoving                ( void );
virtual xbool           IsStaggering            ( void );
        loco*           GetLocoPointer          ( void )  { return m_pLoco; }
virtual xbool           UsingLoco               ( void )  { return TRUE;    }
virtual f32             GetPitch                ( void );
virtual f32             GetYaw                  ( void );
virtual void            SetPitch                ( radian Pitch );
virtual void            SetYaw                  ( radian Yaw   );

// Animation functions
        xbool           HasAnim                 ( loco::anim_type animType );
        xbool           IsAnimInPackage         ( const char* pAnimGroup, const char* pName );
        xbool           IsPlayingFullBodyLipSync( void );
        xbool           IsPlayingFullBodyCinema ( void );             
        xbool           IsPlayingLipSync        ( void );
        xbool           IsPlayingCinema         ( void );

// Zone functions
virtual void            Teleport                ( const vector3& Position, xbool DoBlend = TRUE, xbool DoEffect = FALSE );
virtual void            Teleport                ( const vector3& Position, radian Pitch, radian Yaw, xbool DoBlend = TRUE, xbool DoEffect = FALSE );
virtual void            InitZoneTracking        ( void );
virtual void            UpdateZoneTrack         ( void );
        void            UpdateZone              ( u8 Zone );
        const zone_mgr::tracker& GetZoneTracker  ( void ) const { return m_ZoneTracker; }

// Weapon functions
virtual void            AddNewWeapon2           ( inven_item WeaponItem ) { (void)WeaponItem; }
virtual void            PickupWeapon2           ( inven_item WeaponItem ) { (void)WeaponItem; }
virtual void            SwitchWeapon2           ( inven_item WeaponItem ) { (void)WeaponItem; }
        new_weapon*     GetCurrentWeaponPtr     ( void );
        new_weapon*     GetWeaponPtr            ( inven_item WeaponItem );

virtual void            MoveWeapon              ( xbool UpdateWeaponRenderState );
        void            UpdateWeapon            ( f32 DeltaTime );        
//      guid            GetWeapon               ( void )                   { return m_WeaponGuid; } 
        xbool           HasWeaponEquiped        ( void )                   { return GetCurrentWeaponPtr() != NULL; }
virtual xbool           EquipWeapon2            ( inven_item WeaponItem );
virtual void            UnequipCurrentWeapon    ( void );
virtual s32             GetWeaponRenderState    ( void );
virtual xbool           IsFlashlightOn          ( void );
#ifndef X_EDITOR
        void            DirtyAmmo               ( void )                    { m_NetDirtyBits |= CURRENT_AMMO_BIT; }
#endif

// Collision functions
        void            SetCollidedActor        ( guid collidedActor )      { m_CollidedActor = collidedActor; }
virtual f32             GetActorCollisionRadius ( void );
virtual void            Push                    ( const vector3& PushVector );
virtual f32             GetCollisionHeight      ( void );
virtual f32             GetCollisionRadius      ( void );
        void            WakeUpDoors             ( void );
        void            ResetRidingPlatforms    ( void );

// Floor functions                
        floor_properties&   GetFloorProperties  ( void )        { return m_FloorProperties ; }
        xcolor              GetFloorColor       ( void )        { return m_FloorProperties.GetColor(); }
        u32                 GetFloorMaterial    ( void )        { return m_FloorProperties.GetMaterial(); }
        f32                 GetFloorIntensity   ( void );

// AI related functions
virtual radian          GetSightYaw             ( void ) const;   // Get direction head is facing    
virtual vector3         GetPositionWithOffset   ( eOffsetPos offset );
virtual f32             GetMovementNoiseLevel   ( void );   // return a value of how noisy we are (0.0f - 1.0f)
virtual void            GetHeadAndRootPosition  ( vector3& HeadPos, vector3& RootPos );

// Activate functions
virtual xbool           IsActive                ( void ) {return m_bIsActive;}
        void            SetIsActive             ( xbool bIsActive );

    // Editor special case functions
#ifdef X_EDITOR
virtual void            EditorPreGame           ( void );
#endif // X_EDITOR

// Hit location functions
virtual geom::bone::hit_location    GetHitLocation          ( const pain& Pain );   
        const char*                 GetHitLocationName      ( geom::bone::hit_location Loc );

// Animation functions
virtual void            PlayImpactAnim          ( const pain& Pain, eHitType hitType );
virtual xbool           HandleSpecialImpactAnim ( const eHitType hitType ) { (void)hitType; return FALSE; }
        loco::anim_type GetDeathAnim            ( const pain& painThatKilledUs );

// Event functions
virtual void            SendAnimEvents          ( void ) ;
virtual xbool           OnAnimEvent             ( const anim_event& Event, const vector3& WorldPos );

// Pain related functions
//virtual void            OnApplyPain             ( pain& Pain );
virtual void            OnPain                  ( const pain& Pain )=0;
virtual xbool           TakeDamage              ( const pain& Pain );

#ifndef X_EDITOR
virtual void            DoMultiplayerPainEffects( const pain& Pain );
#endif

virtual void            PlayFlinch              ( const pain& Pain );
        void            PlayPainSfx             ( void );
        void            UpdatePainSfx           ( f32 DeltaTime );
virtual eHitType        OverrideFlinchType      ( eHitType hitType )        { return hitType; }
        const pain&     GetPainThatKilledUs     ( void )                    { return m_PainThatKilledUs; }
        corpse_pain&    GetCorpseDeathPain      ( void )                    { return m_CorpseDeathPain; }
        eHitType        GetHitType              ( const pain& Pain );
virtual void            UpdateFellFromAltitude  ( void ) {}
virtual void            TakeFallPain            ( void ) {}
virtual xbool           IgnorePain              ( const pain& Pain ) { (void)Pain; return FALSE; }
virtual xbool           IgnoreFlinches          ( void ) { return FALSE; }
virtual xbool           IgnoreFullBodyFlinches  ( void ) { return FALSE; }
virtual void            BroadcastActorDeath     ( guid actorKiller ) { (void)actorKiller; }

// cloaking functions
virtual void            UpdateCloak             ( f32 DeltaTime );
virtual void            Cloak                   ( void );
virtual void            Decloak                 ( void );

// contagion functions
virtual xbool           IsContagious            ( void )                { return m_bContagious; }
virtual void            InitContagion           ( s32 Origin = -1 );
virtual void            KillMPContagion         ( void );
virtual void            ContagionLogic          ( f32 DeltaTime );
virtual void            ContagionDOT            ( void );
        void            RenderContagion         ( void );

// Dialog functions
virtual xbool           PlayDialog              (       eDialogType dialogType, 
                                                  const char*       dialogName  = NULL, 
                                                        xbool       hotVoice    = FALSE, 
                                                  const char*       animName    = NULL, 
                                                  const char*       animPkg     = NULL, 
                                                        u32         Flags       = 0, 
                                                        u8          DialogFlags = 0,
                                                        f32         BlendOutTime = 0.25f );

// Effects functions
        xbool           IsBloodEnabled          ( void ) const;
virtual void            CreateDamageEffects     ( const pain& Pain, xbool bDoLargeEffects=TRUE, xbool bDoDebris=TRUE );
        void            CreateSplatDecalOnGround( void );
        void            CreateSplatDecalOnWall  ( const pain& rPain );

// Attach point functions
virtual void            EnumAttachPoints        ( xstring& String   ) const;
virtual s32             GetAttachPointIDByName  ( const char* pName ) const;
virtual xstring         GetAttachPointNameByID  ( s32 iAttachPt     ) const;
virtual xbool           GetAttachPointData      ( s32 iAttachPt, matrix4& L2W, u32 Flags = 0 );
virtual void            OnAttachedMove          ( s32 iAttachPt, const matrix4& L2W );

// Decal functions
        const char*     GetBloodDecalPackage    ( void ) const;
        s32             GetBloodDecalGroup      ( void ) const;

// Rendering functions
const   matrix4*        GetBonesForRender       ( u64 LODMask, s32& nActiveBones );
        void            RenderHitLocations      ( void );
        f32             TimeSinceLastRender     ( void );
virtual skin_inst&      GetSkinInst             ( void ) { return m_SkinInst; }
        anim_group::handle& GetAnimGroupHandle  ( void );
virtual void            SetSkinVMesh            ( xbool bMutant );

virtual xbool           SetMutated              ( xbool bMutate );
virtual void            SetupMutationChange     ( xbool bMutate );
virtual void            ForceMutationChange     ( xbool bMutate );
        void            PrepPlayerAvatar        ( void );
        void            SetCanToggleMutation    ( xbool bCanToggleMutation );
        mutagen_burn_mode GetMutagenBurnMode    ( void ) const { return m_MutagenBurnMode; }
        void            SetMutagenBurnMode      ( mutagen_burn_mode MutagenBurnMode );

        void                   SetAvatarMutationState            ( avatar_mutation_state State );
        avatar_mutation_state  GetAvatarMutationState            ( void ) const { return m_AvatarMutationState; }
        f32                    GetTimeLeftInAvatarMutationState  ( void ) const { return m_TimeLeftInAvatarMutationState; }
        void                   UpdateAvatarMutation              ( f32 DeltaTime );

virtual render_inst*        GetRenderInstPtr      ( void ) { return &m_SkinInst; }
virtual anim_group::handle* GetAnimGroupHandlePtr ( void ) { return &m_hAnimGroup; }
virtual vector3             GetBonePos            ( s32 BoneIndex );

virtual const char*         GetLogicalName        ( void ) { return "ACTOR";}

        actor_effects*  GetActorEffects( xbool bCreate=FALSE );

// Flag functions
        xbool   IsCrouching     ( void ) const;
        void    SetIsCrouching  ( xbool bIsCrouching );
        
        xbool   IsAirborn       ( void ) const;
        void    SetIsAirborn    ( xbool bIsAirborn );

        f32         GetLeanAmount   ( void ) const;
        void        SetLeanAmount   ( f32 LeanAmount );
        lean_state  GetLeanState    ( void ) const;
        void        SetLeanState    ( lean_state State );

        void    SetVoteCanCast      ( xbool CanCast     );
        void    SetVoteCanStartKick ( u32   KickMask    );
        void    SetVoteCanStartMap  ( xbool CanStartMap );

        xbool   GetVoteCanCast      ( void ) const;
        u32     GetVoteCanStartKick ( void ) const;
        xbool   GetVoteCanStartMap  ( void ) const;

        xbool   IsDumbAndFast       ( void ) const { return m_bDumbAndFast; };

        void        SetWayPoint( s32 Index, const vector3& V ) { m_WayPoint[Index] = V; };
const   vector3&    GetWayPoint( s32 Index ) { return m_WayPoint[Index]; };

        f32     GetInactiveTime     ( void ) const { return m_InactiveTime; };
        void    ClearInactiveTime   ( void ) { m_InactiveTime = 0.0f; }
        void    UpdateInactive      ( f32 DeltaTime );

//=========================================================================
// DATA
//=========================================================================
public:    
        
        // Actor active list
static  actor*                  m_pFirstActive;
static  s32                     m_nActive;
        actor*                  m_pNextActive;
        actor*                  m_pPrevActive;

protected:

        // Active info
        xbool                   m_bIsActive ;           // TRUE if character is within active area

        // Faction/team info
        factions                m_Faction ;             // which faction this object belongs to
        u32                     m_FriendFlags ;         // Flags of our friends.
        
        // Health 
        actor_health            m_Health;               // Amount of health that the actor has.
        f32                     m_MaxHealth;            // Actors's maximum health
        xbool                   m_bDead;                // If TRUE, then you are DEAD!
        xbool                   m_bCanDie;              // Defaults to TRUE
        death_type              m_DeathType;            // Type of death
        guid                    m_CorpseGuid;           // Guid of dead body
        xbool                   m_bWantToSpawn;         // Network related
        f32                     m_SpawnFadeTime;        // Time left for spawn fade
        f32                     m_SpawnNeutralTime;     // Time left for neutral spawn behavior (invuln./can't shoot)

        // Zone tracking
        zone_mgr::tracker       m_ZoneTracker;          // Tracks the zones.

        // Inventory
        inventory2              m_Inventory2;           // Inventory
//      inventory               m_Inventory;            // Inventory items
//      inventory               m_WeaponInventory;      // Inventory weapons
//      guid                    m_WeaponGuid;	        // Guid to current weapon
        guid                    m_WeaponGuids[INVEN_NUM_WEAPONS];
        xbool                   m_WeaponsCreated;               // TRUE when weapon objects have been created
        inven_item              m_CurrentWeaponItem;            // Current weapon item, see inventory2.hpp for mapping

        // Pain
        s32                     m_LastAnimPainID;
        s32                     m_LastMeleeEventID;
        s32                     m_CurrentPainEventID;   // the id of our current pain events.
        f32                     m_BigPainTakenTime;
        pain                    m_PainThatKilledUs;     // info from the pain that killed us.
        f32                     m_TimeSinceLastPain;
        corpse_pain             m_CorpseDeathPain;      // Last pain applied to corpse
        f32                     m_SafeFallAltitude;
        f32                     m_DeathFallAltitude;
        f32                     m_FellFromAltitude;     // where did we start falling?
        
        // Conversation/audio properties
        rhandle<char>           m_hAudioPackage;        // Audio resource for this object.
        voice_id                m_VoiceID;              // The voice id used for dialog, pain grunts etc.
        s32                     m_PreferedVoiceActor;   // 0-3 the voice that says our dialog a,b,c,d
        f32                     m_PainSfxInterval;      // Time to wait before playing another pain grunt
        f32                     m_PainSfxDelay;         // Delay time before playing impact sound ( 0 = none queued )

        // cloaking properties
        voice_id                m_CloakVoiceID;         // voice id for cloaking transition sound
        f32                     m_CloakShieldPainTimer; // time just after taking pain where the cloak shield is messed up
        f32                     m_CloakTransitionTime;  // how long we have been in a transition state (going into or out of cloak)
        cloak_state             m_CloakState;           // are we cloaked, uncloaked, or transitioning between the two?

        // Collision
        guid                    m_CollidedActor;        // guid of the actor we ran into
        f32                     m_TimeActorColliding;   // time we have spent actor colliding
        f32                     m_TimeObjectColliding;  // time we have spent colliding with non-actors

        // Decals
        rhandle<decal_package>  m_hBloodDecalPackage;   // the blood package
        s32                     m_BloodDecalGroup;      // the blood group (within the package)

        // Locomotion
        xbool                   m_LocoAllocated;        // If true, must delete m_pLoco pointer.
        loco*                   m_pLoco;
        xbool                   m_bIgnoreLocoInTransform; // annoying bool so we ignore the loco on transform in the editor.
        f32                     m_LastTimeStaggered;

        // Rendering
        floor_properties        m_FloorProperties;
        skin_inst               m_SkinInst ;            // Render instance
        anim_group::handle      m_hAnimGroup ;          // Animation group handle
        f32                     m_TimeSinceLastRender;  // Last time character was rendered
        f32                     m_LeanAmount;           // -1.0f to 1.0f indicates leaning all the way left
                                                        //       to all the way right, respectively
        lean_state              m_LeanState;            // Current lean state

        // Mutation vision glow properties
        xcolor                  m_FriendlyGlowColor;
        xcolor                  m_EnemyGlowColor;
        f32                     m_PulseGlowDelta;       // rate of pulse based character state
        f32                     m_CurrentGlowPulse;     // glow can pulse based on character state
        xbool                   m_bAllowedToGlow;       // is this guy allowed to glow?

        // Contagion timers
        f32                     m_ContagionTimer;       // Time left as contagious.
        f32                     m_ContagionDOTTimer;    // Time to next DOT.

        // Flags
        u32                     m_CanCloak:1,
                                m_MustCloak:1,
                                m_bIsCrouching:1,               // Is the actor crouching
                                m_bIsAirborn:1,                 // Is the in the air
                                m_VoteCanCast:1,
                                m_VoteCanStartMap:1,
                                m_bIsMutated:1,                 // Are we mutated?
                                m_bCanToggleMutation:1,
                                m_bDumbAndFast:1,
                                m_bPrimaryFired:1,              // Flags primary has been fired (used in online)
                                m_bEndPrimaryFire:1,            // Used to stop primary fire (used in online)
                                m_bContagious:1;

        s32                     m_FireState;

        u32                     m_VoteCanStartKick;
        s32                     m_VoteAction;       // 1=Cast 2=CallKick 3=CallMap
        s32                     m_VoteArgument;     // Payload for VoteAction

        radian                  m_Pitch;                        // Head Logical orientation. Used for movement
        radian                  m_Yaw;                          // Head Logical orientation. Used for movement

        actor_effects*          m_pEffects;

        actor::mutagen_burn_mode        m_MutagenBurnMode;         // what percentage of mutagen we burn every seconde
        actor::avatar_mutation_state    m_AvatarMutationState;
        f32                             m_TimeLeftInAvatarMutationState;

        // Aiming stuff.
        s32                     m_TargetNetSlot;
        vector3                 m_AimOffset;

        u32                     m_WayPointFlags;
        s32                     m_WayPointTimeOut;
        vector3                 m_WayPoint[2];

        xbool                   m_bLockedDoors;

    //--------------------------------------------------------------------------
    //  Inactivity tracking
    //--------------------------------------------------------------------------

    f32         m_InactiveTime;
    vector3     m_RecentPosition;

//==============================================================================
//  NETWORKING STUFF
//==============================================================================
#ifndef X_EDITOR
//------------------------------------------------------------------------------

public:

    virtual u32             net_GetUpdateMask   ( s32 TargetClient ) const;

    virtual void            net_Activate        ( void );
    virtual void            net_SetTeamBits     ( u32 TeamBits );
    virtual void            net_SetSkin         ( s32 Skin );
    virtual void            net_SetVoiceActor   ( s32 VoiceActor );
    
    virtual void            net_AcceptUpdate    ( const bitstream& BitStream );
    virtual void            net_AcceptUpdate    ( const update&    Update    );
    virtual void            net_ProvideUpdate   (       bitstream& BitStream, 
                                                        u32&       DirtyBits );
    virtual void            net_ProvideUpdate   (       update&    Update,
                                                        u32&       DirtyBits );

            void            net_DropWeapon      ( void );
    virtual xbool           net_EquipWeapon2    ( inven_item WeaponItem );

            void            net_ReportNetPain   (       s32          Victim,
                                                        s32          Origin,
                                                        s32          PainType,
                                                        s32          VictimLifeSeq,
                                                        xbool        Kill,
                                                        f32          Damage );

            void            net_ApplyNetPain    ( net_pain& NetPain );
            void            net_UpdateHealth    ( f32 DeltaHealth );

            void            net_WantPickup      ( pickup& Pickup );
            void            net_RequestTeam     ( s32 NewTeam );
            s32             net_GetLifeSeq      ( void ) { return( m_Net.LifeSeq ); };
            
            // Call on server to trigger anims to play on client
            // TO DO - Add to move so client controlled players also send?
    virtual void            net_PlayAnim            ( s32 iAnim, 
                                                      f32 BlendTime, 
                                                      u32 Flags, 
                                                      f32 PlayTime );
                                                          
            // Avatar animation playback functions (call on server or client)
            void            net_Reload              ( void );
            void            net_Melee               ( void );
            void            net_Grenade             ( void );
            void            net_FirePrimary         ( void );
            void            net_FireSecondary       ( void );
            void            net_BeginFirePrimary    ( void );
            void            net_EndFirePrimary      ( void );

protected:

    //--------------------------------------------------------------------------
    // Common networking members.

    struct net
    {
        enum 
        {
            FIRE_PRIMARY,
            FIRE_SECONDARY
        };

        // Functions
        net( void );

        // Data
        s32         Skin;           // Net skin (defines in GameMgr.hpp)
        
        s32         iAnim;          // Play animation info
        f32         AnimBlendTime;
        u32         AnimFlags;
        f32         AnimPlayTime;

        s32         FireMode;       // FIRE_PRIMARY, FIRE_SECONDARY, see enum above...
        s32         FireSeq;        // FIRE_BEGIN_BIT & FIRE_END_BIT matching                                            
        s32         DesiredTeam;    // Used to request a team change from server.
        s32         LifeSeq;        // When 'even', alive.  When 'odd', dead!
        pain_queue* pPainQueue;      
    };

    //--------------------------------------------------------------------------
    
    net     m_Net;  // <<---- All actor specific networking variables!

    //--------------------------------------------------------------------------
    //  Contagion
    //--------------------------------------------------------------------------
    struct mp_contagion
    {
        u32         PlayerMask;
        fx_handle   Arc[32];
        s32         Origin;
    };

    mp_contagion*   m_pMPContagion;

//------------------------------------------------------------------------------
#endif // X_EDITOR
//==============================================================================

friend class save_mgr;
friend class check_point_mgr;
friend class state_mgr;
};

//=========================================================================

inline
vector3 actor::GetVelocity( void ) const
{
    // Use loco physics?
    if( m_pLoco )
        return m_pLoco->m_Physics.GetVelocity();
        
    // Just use base class
    return object::GetVelocity();        
}

//=========================================================================

inline 
f32 actor::GetHealth()
{
    return m_Health.GetHealth();
}

//=========================================================================

inline 
void actor::ResetHealth( f32 Health )
{
    return m_Health.Reset( Health );
}

//=========================================================================

inline 
f32 actor::GetMaxHealth()
{
    return m_MaxHealth;
}

//=========================================================================

inline 
void actor::SetMaxHealth( f32 MaxHealth )
{
    m_MaxHealth = MaxHealth;
}

//=========================================================================

inline
f32 actor::GetParametricHealth()
{
    return( GetHealth() / m_MaxHealth );
}

//=========================================================================

inline
xbool actor::DoAliveLogic( void )
{
    xbool bNormalLogic = ( (m_Health.GetHealth() > 0.f) || (!m_bCanDie) );
    return bNormalLogic;
}

//===========================================================================
// FACTION FUNCTIONS
//===========================================================================

inline
factions actor::GetFaction( void ) const
{
    ASSERT( m_Faction != FACTION_NOT_SET );

    // Special case when contagious...
    if( m_bContagious )
    {
        return FACTION_DEATHMATCH;
    }
    else
    {    
        return m_Faction;
    }
}

//===========================================================================

inline
void actor::SetFaction( factions NewFaction )
{
    m_Faction = NewFaction;
}

//===========================================================================

inline
void actor::SetFriendFlags( u32 NewFriends )
{
    m_FriendFlags = NewFriends | FACTION_NEUTRAL;
    m_FriendFlags = m_FriendFlags & !FACTION_DEATHMATCH;
}

//===========================================================================

inline
u32 actor::GetFriendFlags( void ) const
{
    return m_FriendFlags;
}

//=========================================================================

inline
xbool actor::IsFriendlyFaction( factions Faction ) const
{
    if( Faction == FACTION_NOT_SET )
        return FALSE;

    // deathmatch special case
    if( Faction == FACTION_DEATHMATCH ||
        GetFaction() == FACTION_DEATHMATCH )
        return FALSE;

    return( (m_FriendFlags & Faction) ||
            (Faction == FACTION_NEUTRAL) || 
            (GetFaction() == FACTION_NEUTRAL) );
}

//=========================================================================

inline
xbool actor::IsEnemyFaction( factions Faction ) const
{
    if ( Faction == FACTION_NOT_SET )
        return FALSE;

    if( ((Faction == FACTION_TEAM_ONE) && (GetFaction() == FACTION_TEAM_ONE)) ||
        ((Faction == FACTION_TEAM_TWO) && (GetFaction() == FACTION_TEAM_TWO)) )
    {
        return FALSE;
    }

    // deathmatch special case
    if( Faction == FACTION_DEATHMATCH ||
        GetFaction() == FACTION_DEATHMATCH )
        return TRUE;

    return( ((~m_FriendFlags) & Faction)      && 
            (Faction != FACTION_NEUTRAL)      && 
            (GetFaction() != FACTION_NEUTRAL) );
}

//===========================================================================

inline
const char* actor::GetBloodDecalPackage( void ) const
{
    return m_hBloodDecalPackage.GetName();
}

//===========================================================================

inline
s32 actor::GetBloodDecalGroup( void ) const
{
    return m_BloodDecalGroup;
}

//===========================================================================

inline 
anim_group::handle& actor::GetAnimGroupHandle( void )    
{ 
    return m_hAnimGroup; 
}

//=========================================================================

inline 
f32 actor::TimeSinceLastRender( void )
{ 
    return m_TimeSinceLastRender; 
}

//===========================================================================
// Dialog functions

inline
xbool actor::PlayDialog(       eDialogType dialogType, 
                         const char*       dialogName,
                               xbool       hotVoice,
                         const char*       animName,
                         const char*       animPkg,
                               u32         Flags,
                               u8          DialogFlags,
                               f32         BlendOutTime )
{
    (void)dialogType; 
    (void)dialogName;
    (void)hotVoice;
    (void)animName;
    (void)animPkg;
    (void)Flags;
    (void)DialogFlags;
    (void)BlendOutTime;
    return FALSE;
}

//===========================================================================
// Flag functions

inline
xbool actor::IsCrouching( void ) const
{
    return ( m_bIsCrouching != 0 );
}

//===========================================================================

inline
void actor::SetIsCrouching( xbool bIsCrouching )
{
    // State change?
    if( IsCrouching() != bIsCrouching )
    {
        // Record new state.
        m_bIsCrouching = bIsCrouching;
        
        // Send new crouch state across net.
        #ifndef X_EDITOR
        m_NetDirtyBits |= AIR_CROUCH_BIT;  // NETWORK
        #endif // X_EDITOR
    }
}

//===========================================================================

inline
xbool actor::IsAirborn( void ) const
{
    return ( m_bIsAirborn != 0 );
}

//===========================================================================

inline
void actor::SetIsAirborn( xbool bIsAirborn )
{
    // State change?
    if( IsAirborn() != bIsAirborn )
    {
        // Record new state.
        m_bIsAirborn = bIsAirborn;

        // Send new crouch state across net.
        #ifndef X_EDITOR
        m_NetDirtyBits |= AIR_CROUCH_BIT;  // NETWORK
        #endif // X_EDITOR
    }
}

//===========================================================================

inline
f32 actor::GetLeanAmount( void ) const
{
    return m_LeanAmount;
}

//===========================================================================

inline
void actor::SetLeanAmount( f32 LeanAmount )
{
    // Update?
    if( m_LeanAmount != LeanAmount )
    {
        m_LeanAmount = LeanAmount;
        
        // Send new lean state across net.
        #ifndef X_EDITOR
        m_NetDirtyBits |= LEAN_BIT;  // NETWORK
        #endif // X_EDITOR
    }
}

//===========================================================================

inline
actor::lean_state actor::GetLeanState( void ) const
{
    return m_LeanState;
}

//===========================================================================

inline
void actor::SetLeanState( lean_state State )
{
    // State change?
    if( m_LeanState != State )
    {
        // Record new state.
        m_LeanState = State;

        // Send new lean state across net.
        #ifndef X_EDITOR
        m_NetDirtyBits |= LEAN_BIT;  // NETWORK
        #endif // X_EDITOR
    }
}

//===========================================================================

inline
void actor::SetVoteCanCast( xbool CanCast )
{
    #ifndef X_EDITOR
    if( (xbool)m_VoteCanCast != CanCast )
        m_NetDirtyBits |= VOTE_FLAGS_BIT;
    #endif

    m_VoteCanCast = CanCast;
}

//===========================================================================

inline
xbool actor::GetVoteCanCast( void ) const
{
    return( m_VoteCanCast );
}

//===========================================================================

inline
void actor::SetVoteCanStartKick( u32 KickMask )
{
    #ifndef X_EDITOR
    if( m_VoteCanStartKick != KickMask )
        m_NetDirtyBits |= VOTE_FLAGS_BIT;
    #endif

    m_VoteCanStartKick = KickMask;
}

//===========================================================================

inline
u32 actor::GetVoteCanStartKick( void ) const
{
    return( m_VoteCanStartKick );
}

//===========================================================================

inline
void actor::SetVoteCanStartMap( xbool CanStartMap )
{
    #ifndef X_EDITOR
    if( (xbool)m_VoteCanStartMap != CanStartMap )
        m_NetDirtyBits |= VOTE_FLAGS_BIT;
    #endif

    m_VoteCanStartMap = CanStartMap;
}

//===========================================================================

inline
xbool actor::GetVoteCanStartMap( void ) const
{
    return( m_VoteCanStartMap );
}

//===========================================================================
#endif // ACTOR_HPP
//===========================================================================
