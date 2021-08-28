#ifndef PLAYER_HPP
#define PLAYER_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "e_View.hpp"
#include "Obj_mgr\obj_mgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Animation\AnimPlayer.hpp"
#include "Locomotion\CharacterPhysics.hpp"

#include "Objects\Render\SkinInst.hpp"
#include "Animation\CharAnimPlayer.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "ZoneMgr\ZoneMgr.hpp"
#include "EventMgr\EventMgr.hpp"
#include "Objects\Actor\Actor.hpp"
#include "Objects\SpawnPoint.hpp"
#include "Objects\PlayerLoco.hpp"
#include "Objects\NewWeapon.hpp"
#include "Objects\ProjectileBullett.hpp"
#include "Objects\Pickup.hpp"

#include "..\Support\Trigger\Actions\lock_player_view.hpp"

#include "..\Support\NetworkMgr\NetObj.hpp"
#include "..\Support\NetworkMgr\Blender.hpp"

//=========================================================================
// Defines
//=========================================================================

#define STUN_PLAYER

#ifdef TARGET_PS2
#define MAX_LOCAL_PLAYERS   2
#else
#define MAX_LOCAL_PLAYERS   4
#endif

#define MAX_MELEE_STATES    5 // ANIM_STATE_MELEE to ANIM_STATE_MELEE_END.  Keep this in sync.
#define MAX_LORE_ITEMS      5

//==============================================================================
// KSS -- TO ADD NEW WEAPON
#define WB_SMP  (1<<0)       // SMP
#define WB_DE   (1<<1)       // Desert Eagle
#define WB_SG   (1<<2)       // Shotgun
#define WB_SR   (1<<3)       // Sniper Rifle
#define WB_MC   (1<<4)       // Meson Cannon
#define WB_MM   (1<<5)       // Mutant Melee
#define WB_MP   (1<<6)       // Mutant Parasites
#define WB_FG   (1<<7)       // Frag Grenades
#define WB_BBG  (1<<8)       // BBG (NAW)
#define WB_JBG  (1<<9)       // Jumping Bean Grenades
#define WB_SCN  (1<<10)      // Jumping Bean Grenades
#define WB_MS   (1<<11)      // Mutant secondary (contagion)

#define MAX_COMBO_HITS      3

// Forward Declarations
class weapon_mutation;
class collectable_anti_mutagen;
class ladder_field ;
class check_point_mgr;

//==============================================================================
//  TYPES
//==============================================================================

class corpse;
class third_person_camera;

//=============================================================================
//  Multiplayer tweaks.

struct mp_tweaks
{
    xbool   Active;
    f32     JumpSpeed;
    f32     Gravity;    
    f32     AirControl;
};

extern mp_tweaks g_MPTweaks;

class hud_object;

struct mtwt
{
    const char* pLevelName;
    s32         MemoryBallast;
    inven_item  StartItem;
    s32         AvailableWeapons;
    s32         StartWeapons;
    xbool       bLoadWarnsLowAmmo;
};


//=============================================================================
//------------------------------------------------------------------------------
// aim assist info
//------------------------------------------------------------------------------
struct AimAssistData
{
    AimAssistData( void );          // struct constructor

    vector3 BulletAssistDir;        // our aim assist direction for the bullet
    xbool   bReticleOn;             // is the reticle on a target
    f32     BulletAssistBestDist;   // the best distance to the target used for bullet assist
    f32     TurnDampeningT;         // what is the interpolation distance for turn dampening
    guid    TargetGuid;             // this is the best target for our aim assist
    f32     LOFCollisionDist;       // used to test if we hit something before we got to target
    f32     LOFSpineDist;           // distance to the closest point on the spine that we are firing at
    vector3 SpinePt;                // point on the spine
    vector3 LOFPt;                  // where the line of fire point ends
    f32     LOFPtT;                 // at what point along the line of fire is the spine point intersecting
    f32     SpinePtT;               // where along the spine is the line of fire intersecting
    f32     LOFPtDist;              // the actual distance along the line of fire point (AIMASSIST_LOF_DIST * LOFPtT)
    f32     ReticleRadius;          // used for the pill that tells when the reticle is over an enemy (turns it on).
    f32     BulletInnerRadius;      // what's the inner radius of the bullet assist
    f32     BulletOuterRadius;      // what's the outer radius of the bullet assist
    f32     TurnInnerRadius;        // what's the inner radius of the turn dampening
    f32     TurnOuterRadius;        // what's the outer radius of the turn dampening

    // online stuff
    guid    OnlineFriendlyTargetGuid;   // this is the friendly target for online
    vector3 AimDelta;                   // used for online to determine the offset of where we are aiming
};

//=============================================================================

//=========================================================================
// PLAYER
//=========================================================================

class player : public actor
{
public:
    CREATE_RTTI( player, actor, object )

public:

	//------------------------------------------------------------------------------
    enum cycle_direction
	{
		CYCLE_LEFT = 0,
		CYCLE_RIGHT
	};

    struct bullet_fly_by
    {
        vector3 Start;
        vector3 End;
        f32     Age;
        f32     Lifetime;
        s32     VoiceID;
        xbool   bIsActive;
    };

    struct convulsion_info
    {
        convulsion_info( void );
        f32   m_TimeSinceLastConvulsion;        // How long ago did we have a convulsion?
        f32   m_ConvulseAtTime;                 // At what m_TimeSinceLastConvulsion do we convulse again?
        xbool m_bConvulsingNow;                 // Are we convulsing now?
        f32   m_TimeLeftInThisConvulsion;       // If we are convulsing now, how much longer?
    };

    #define MAX_FLY_BYS (4)

    //------------------------------------------------------------------------------
    struct strain_control_modifiers
    {
        strain_control_modifiers();

        f32                     m_StrainProximityAlertRadius;
        f32                     m_StrainMaxFowardVelocity;            // Maximum foward/Backwards velocity
        f32                     m_StrainMaxStrafeVelocity;            // Maximum strafe velocity
        f32                     m_StrainJumpVelocity;                 // How much velocity he has at the take off of the jump
        f32                     m_StrainMaxHealth;                    // Player's maximum health   
        vector3                 m_StrainEyesOffset;                   // Offset for the strain
//      f32                     m_StrainStickSensitivity;             // Stick sensitivity
        f32                     m_StrainMinWalkSpeed;                 // Strain Min Walk Speed
        f32                     m_StrainMinRunSpeed;                  // Strain Min Run Speed
        f32                     m_StrainDecelerationFactor;           // Rate of deceleration for this strain.
        f32                     m_StrainCrouchChangeRate;             // Speed of crouch.
        f32                     m_StrainReticleMovementDegrade;       // How quickly does the reticle degrade
        f32                     m_fStrainForwardAccel;                // Forward acceleration
        f32                     m_fStrainStrafeAccel;                 // Strafe acceleration
        f32                     m_fStrainYawSensitivity;              // Yaw sensitivity for this strain
        f32                     m_fStrainPitchSensitivity;            // Pitch sensitivity for this strain
        f32                     m_StrainYawAccelTime;                 // How long it takes to get to max acceleration for yaw rotation
        f32                     m_StrainPitchAccelTime;               // How long it takes to get to max acceleration for pitch rotation
    };

    enum view_flags
    {
        VIEW_NULL           = 0,
        VIEW_SHAKE          = BIT( 0),
        VIEW_ALL            = 0xFFFFFFFF
    };

    enum non_exclusive_states
    {
        NE_STATE_NULL = 0,
        NE_STATE_STUNNED = BIT(0)
    };

    struct view_info
    {
        view_info( void )
        {
            XFOV = 0;
        }
        radian XFOV;
    };

    //------------------------------------------------------------------------------
    enum weapon_state
    {
        WEAPON_STATE_NONE = 0,
        WEAPON_STATE_FIRST_PERSON,
        WEAPON_STATE_AVATAR
    };

    //------------------------------------------------------------------------------
    enum anim_priority
    {
        ANIM_PRIORITY_DEFAULT   = 0,
        MAX_ANIM_PER_STATE      = 4
    };

    //------------------------------------------------------------------------------
    struct state_anims
    {
        u8 nPlayerAnims;
        u8 nWeaponAnims;
        u8 PlayerAnim[ MAX_ANIM_PER_STATE ];
        u8 WeaponAnim[ MAX_ANIM_PER_STATE ];
    };

    //------------------------------------------------------------------------------
    // List of the different animation states for player stage1 and 2.
    enum animation_state
    {
        ANIM_STATE_UNDEFINED = -1,

        ANIM_STATE_SWITCH_TO,
        ANIM_STATE_SWITCH_FROM,
        ANIM_STATE_IDLE,
        ANIM_STATE_RUN,
        ANIM_STATE_THROW,
        ANIM_STATE_PICKUP,
        ANIM_STATE_DISCARD,
        
        ANIM_STATE_FIRE,
        ANIM_STATE_ALT_FIRE,
        ANIM_STATE_GRENADE,
        ANIM_STATE_ALT_GRENADE,
        ANIM_STATE_MUTATION_SPEAR,
        ANIM_STATE_MELEE,
        ANIM_STATE_MELEE_FROM_CENTER,
        ANIM_STATE_MELEE_FROM_DOWN,
        ANIM_STATE_MELEE_FROM_LEFT,
        ANIM_STATE_MELEE_FROM_RIGHT,
        ANIM_STATE_MELEE_FROM_UP,
        ANIM_STATE_MELEE_TRAVEL,
        ANIM_STATE_MELEE_END,

        // combo melee stuff
        ANIM_STATE_COMBO_BEGIN,
        ANIM_STATE_COMBO_HIT,
        ANIM_STATE_COMBO_END,

        ANIM_STATE_RELOAD,
        ANIM_STATE_RELOAD_IN,
        ANIM_STATE_RELOAD_OUT,
        ANIM_STATE_RAMP_UP,
        ANIM_STATE_RAMP_DOWN,
        ANIM_STATE_ALT_RAMP_UP,
        ANIM_STATE_ALT_RAMP_DOWN,
        ANIM_STATE_HOLD,
        ANIM_STATE_ALT_HOLD,
        
        ANIM_STATE_ZOOM_IN,
        ANIM_STATE_ZOOM_OUT,
        ANIM_STATE_ZOOM_IDLE,
        ANIM_STATE_ZOOM_RUN,
        ANIM_STATE_ZOOM_FIRE,

        ANIM_STATE_DEATH,
        ANIM_STATE_CHANGE_MUTATION,
        ANIM_STATE_FALLING_TO_DEATH,

        ANIM_STATE_CINEMA,
        ANIM_STATE_MISSION_FAILED,

        ANIM_STATE_MAX
    };

//=========================================================================
//
// GetActivePlayer  -   This is to get the active player.
// GetView          -   Gets the player view used for rendering.
// GetIsActiveView  -   Tells whether or not this player has the active view.
// ResetView        -   Resets the view to the original FOV
// GetEyesPosition  -   Gets the position of the eyes of the player. (The view position)
// GetPitch         -   Returns the pitch of the player
// GetYaw           -   Returns the Yaw of the player
// GetSpeed         -   Returns instantaneous velocity of the player
// SetMaxPitch      -   Sets the maximum pitch of the player
// SetMinPitch      -   Sets the minimum pitch of the player
// SetStickSensitivity- How sensitive is the stick?
// ResetStickSensitivity- Resets to original stick sensitivity.

// AddHealth        -   Nothing yet.  Will update player's health when a health object is collected
// AddDefence       -   Nothing yet.  Will update player's defensive stats when defensive item is collected
// AddBoost         -   Nothing yet.  Will update player's stats when boost item is collected

// GetMaxHealth     -   Returns maximum health for the player.
// OnDeath          -   Called when player dies.  Resets position, resets HUD.
// OnReset          -   Resets the player's zone, hud, and health and moves the player to respawn position
// GetPlayerObjectZone  -   Returns zone from m_PlayerTracker (ie. what the player feet are in)
// GetPlayerViewZone    -   Returns zone from cinema (if playing) or m_PlayerTracker if not
// ShakeView        -   Shakes the view of this player object.

// OnPain           -   Called when something tries to hurt the player
//
//
// SpottedByEnemy   -   Called by enemy when it spots the player.  Used to update time for figuring out when it's safe to save
//
//=========================================================================

public:
                            player              ( void );
            virtual         ~player             ( void );

            // Object description.
    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );


    static  player*         GetActivePlayer     ( void );
            void            SetAsActivePlayer   ( xbool bActive ) { m_bActivePlayer = bActive; }
            xbool           IsActivePlayer      ( void ) { return m_bActivePlayer; }

            void            SetLocalPlayer      ( s32 LocalIndex );

    static  view&           GetView             ( s32 Player );
    view&                   GetView             ( void );
    const   view_info&      GetViewInfo         ( void ) { return m_ViewInfo; }
            xbool           IsAvatar            ( void );
            xbool           IsMutated           ( void ) { return m_bIsMutated; }
            xbool           IsMutantVisionOn    ( void ) { return m_bIsMutantVisionOn; }
    virtual xbool           SetMutated          ( xbool bMutate );
    virtual void            SetupMutationChange ( xbool bMutate );
            void            UpdateMutagen       ( f32 DeltaTime );
    f32                     GetSpeed            ( void );
    const view_info&        GetOriginalViewInfo ( void ) { return m_OriginalViewInfo; }
    void                    ResetView           ( void );
    void                    SetStickSensitivity ( const f32& rMultiplier );//{ m_fStickSensitivity *= rMultiplier; }
    void                    ResetStickSensitivity( void );//{ m_fStickSensitivity = m_fOriginalStickSensitivity; }
    virtual void            OnKill              ( void );               
    virtual void            OnDeath             ( void );
    virtual void            OnReset             ( void );
    virtual void            OnSpawn             ( void );
            void            OnMissionFailed     ( s32 TableName, s32 ReasonName );

    hud_object*             GetHud              ( void );

    void                    ManTurret           ( guid TurretGuid, 
                                                  guid Turret2Guid, 
                                                  guid Turret3Guid, 
                                                  guid AnchorGuid, 
                                                  guid LeftBoundaryGuid,
                                                  guid RightBoundaryGuid,
                                                  guid UpperBoundaryGuid,
                                                  guid LowerBoundaryGuid );
    void                    ExitTurret          ( void );
    void                    Teleport            ( const vector3& Position, radian Pitch, radian Yaw, xbool DoBlend = TRUE, xbool DoEffect = FALSE );
    void                    Teleport            ( const vector3& Position, xbool DoBlend = TRUE, xbool DoEffect = FALSE );
    
            void            ParseOnPainForEffects( const pain& Pain );
            void            UpdateZoneTrack     ( void ); 
    zone_mgr::zone_id       GetPlayerObjectZone ( void ) const;
    zone_mgr::zone_id       GetPlayerViewZone   ( void ) const;
    void                    ShakeView           ( f32 Time, f32 Amount = 1.0f, f32 Speed = 1.0f );
    xbool                   InvalidSound        ( void );
    f32                     GetAimDegradation   ( void ){ return m_AimDegradation; };
            vector3         GetDefaultViewPos   ( void );

            void            ComputeView         ( view& View, view_flags Flags = player::VIEW_NULL );
    virtual void            Push                ( const vector3& PushVector );

      const xarray<pain>&   GetLastPainEvents   ( void ){ return m_LastPainEvent; }
            void            ClearPainEvent      ( void );
    virtual void            OnPain              ( const pain& Pain );
            void            RespondToPain       ( const pain& Pain );
            void            DoBasicPainFeedback ( f32 Force );
    virtual actor::eHitType OverrideFlinchType  ( actor::eHitType hitType );
    virtual xbool           AddHealth           ( f32 nDeltaHealth );
    virtual xbool           AddAmmo2            ( inven_item WeaponItem, s32 Amount );
            void            EmitMeleePain       ( void );
            void            DoTendrilCollision  ( void );

            void            HandleBulletFlyby   ( bullet_projectile& Bullet );
            xbool           RenderSniperZoom    ( void );

    virtual render_inst*        GetRenderInstPtr        ( void )            { return &m_Skin; }
    virtual anim_group::handle* GetAnimGroupHandlePtr   ( void )            { return &m_AnimGroup; }
    virtual vector3             GetBonePos              ( s32 BoneIndex );

    virtual const char*         GetLogicalName      ( void ) { return "PLAYER";}


//=========================================================================           
// SpottedByEnemy       -   Resets safe spot counter
// GetIsSafeSpot        -   Checks some tickers and tells if the place where the player is is now safe.
// SetCurrentSpotAsSafeSpot - Sets player's current position as the new safe spot.
// ResetToLastSafeSpot  -     Resets the player to the last safe spot
// GatherGameSpeakGuid  - Gathers possible candidate to speak to
// DegradeAim           - Slight degradation of the bullet's trajectory.
// SetAimRecoverSpeed   - How quickly does it take for the aim to recover after the player stops moving
// PushViewCinematic    - Sets view to cinematic
// IsStealthed          - Is this object stealthed
// OnPain               - Pain event handler
// BackUpCurrentState   - Saves current state of the player
// ResetPlayer          - SOMETHING TO DO WITH RIFT BOT.  DON'T KNOW.
// CanSeeObject         - Can we see the specified object.
// GetMaxVelocity       - Maximum velocity
// GetCurrentVelocity   - Current velocity.
// GetSightYaw          - direction head is facing
//=========================================================================
public:
    
    virtual void            GetProjectileHitLocation (vector3& EndPos, xbool bUseBulletAssist = TRUE);
            xbool           IsAltFiring         ( void );
            xbool           IsFiring            ( void );
    virtual void            SpottedByEnemy      ( void ) { m_LastTimeSeenByEnemy = (f32)x_GetTimeSec(); }
    virtual xbool           GetIsSafeSpot       ( void );
    virtual void            SetCurrentSpotAsSafeSpot ( void );
    virtual void            ResetToLastSafeSpot ( void );

    virtual void            GatherGameSpeakGuid ( void );

    virtual void            AddNewWeapon2           ( inven_item WeaponItem );
    virtual xbool           TryAddAmmo2             ( inven_item WeaponItem );
            void            UpdateVirtualWeapons2   ( void );

#ifdef X_EDITOR
    virtual void            OnEditorRender          ( void );
#endif // X_EDITOR

    //=========================================================================
    // FLASHLIGHT STUFF
    //=========================================================================
            void            InitFlashlight      ( const vector3& rInitPos );
    virtual xbool           IsFlashlightOn      ( void )            { return m_bUsingFlashlight; }
            xbool           IsFlashlightActive  ( void );            
            void            SetFlashlightActive ( xbool bOn );
            void            MoveFlashlight      ( void );
            void            UpdateFlashlightBattery ( f32 nDeltaTime );
    virtual xbool           AddBattery              ( const f32& nDeltaBattery );
    virtual f32             GetBattery              ( void ) { return m_Battery; }
    virtual f32             GetMaxBattery           ( void ) { return m_MaxBattery; }

    virtual void            DegradeAim                      ( f32 fAmountToDegradeBy );
    virtual void            SetAimRecoverSpeed              ( f32 fRecover ) { m_AimRecoverSpeed = fRecover; }    
      const vector3&        GetCurrentWeaponCollisionOffset ( void ) { return m_WeaponCollisionOffset; }

    virtual xbool           IsPlayer            ( void )                    { return TRUE; }

            void            PushViewCinematic   ( lock_view_node* pLockViewBuffer );
            guid            GetCinemaCameraGuid ( void ) const { return m_Cinema.m_CinemaCameraGuid; }

    virtual void            BackUpCurrentState  ( void );
    virtual void            RestoreState        ( void );

            void            ResetPlayer         ( const vector3& rPos, const radian3& rLookAt );
    virtual xbool           CanSeeObject        (object* pObject);

            f32             GetMaxVelocity          ( void );
            f32             GetCurrentVelocity      ( void );
    virtual f32             GetCollisionHeight      ( void );
    virtual f32             GetCollisionRadius      ( void );
    virtual void            AddImpulse              ( vector3& rImpulse );
            void            OnWeaponSwitch2         ( const cycle_direction& CycleDirection );
			void			SetNextWeapon2          ( inven_item WeaponItem, xbool ForceSwitch = FALSE, xbool StateChange = TRUE );
			xbool			ShouldSwitchToWeapon2   ( inven_item WeaponItem, xbool bFirstPickup );

    virtual void            OnMove              ( const vector3& NewPos );
    virtual void            OnMoveFreeCam       ( view& View );
    virtual void            OnExitFreeCam       ( vector3& NewPos );
            s32             GetActivePlayerPad  ( void ) { return m_ActivePlayerPad; }
            s32             GetLocalSlot        ( void ) const { return m_LocalSlot; }
            void            SetLocalSlot        ( s32 Slot) { m_LocalSlot = Slot; }
    virtual void            Jump                ( void );
            void            HitJumpPad          ( const vector3& Velocity, 
                                                        f32      DeltaTime, 
                                                        f32      AirControl, 
                                                        xbool    BoostOnly, 
                                                        xbool    ReboostOnly, 
                                                        xbool    Instantaneous, 
                                                        guid     JumpPadGuid );
            void            DoFeedback          ( f32 Duration, f32 Intensity );
            void            UpdateLean          ( f32 LeanValue );
            void            UpdateArmsOffsetForLean( void );                         
            f32             GetLastTimeWeaponFired          ( void );

    //---------------------------------------------------------------------
    // Mutation weapon melee stuff
    //---------------------------------------------------------------------
  animation_state           SetupMutationMeleeWeapon    ( void );            
  weapon_mutation*          GetMutationMeleeWeapon      ( void );
            void            RandomizeMeleeAnimStateList ( void );
            void            InitializeMeleeAnimStateList( void );
  animation_state           GetNextMeleeState           ( void );

    virtual void            SetMeleeState       ( animation_state MeleeState );

    virtual void            OnEvent             ( const event& Event );
    virtual f32             GetMovementNoiseLevel ( void );
    virtual radian          GetSightYaw         ( void ) const;   

    virtual xbool           AddItemToInventory2     ( inven_item Item );
    virtual xbool           RemoveItemFromInventory2( inven_item Item, xbool bRemoveAll = false );
            
            void            AcquireAllLoreObjects   ( void );
            void            LoadAllLoreObjects      ( void );
            xbool           GetJBGLoreAcquired      ( void )            { return m_bJBGLoreAcquired; }
            void            SetJBGLoreAcquired      ( xbool bAcquired ) { m_bJBGLoreAcquired = bAcquired; }
            xbool           GetClosestLoreObjectDist( f32 &ClosestDist ); // returns FALSE if no objects are available or none are close enough

            guid            GetEnemyOnReticle   ( void );
            guid            GetFriendlyOnReticle( void );

            guid            GetBestTargetGuid( void )
            {
                return m_AimAssistData.TargetGuid;
            }

            vector3         GetAimDelta( void )
            {
                return m_AimAssistData.AimDelta;
            }
    
            f32             GetReticleRadius    ( void ) const { return m_ReticleRadius; }
            xbool           ReticleOnTarget     ( void ) const { return m_AimAssistData.bReticleOn; }
            void            UpdateWeaponPullback( void );
            void            MoveAnimPlayer      ( const vector3& Pos ); // this adds in the arms offset and keeps the weapon
                                                                        // out of geometry
            vector3         GetAnimPlayerOffset ( void );

            void            VoteCast            ( s32 Vote );
            void            VoteStartKick       ( s32 Kick );
            void            VoteStartMap        ( s32 Map  );
            xbool           CanVote             ( void ) { return m_VoteMode; }

#ifdef jmchugh
    // This is here so I can stun the player with a keypress / controller combo.
            virtual void        StunPlayer                  ( void ){};
#endif
    virtual void        OnTransform                 ( const matrix4& L2W );    

//=========================================================================
// 
// GetLocalBBox         -   Gets the bbox of the player in local space
// GetMaterial          -   Legacy code. (Will get removed with time)
// OnMove               -   Any time of the player needs to change its position
//                          we must call this function
// OnEnumProp           -   Enumerates all the properties related to the player
// OnProperty           -   Set/Gets properties for the player
// OnInit               -   Gets call when initializing the object.
// OnColRender          -   Render the collision of the player. (Bounding spheres)
// OnColCheck           -   Gets call when an object wants to check the collision agains the player
// OnRender             -   Gets call when ever we need to render the player
// OnRenderShadowCast   -   Gets call when ever we render a player shadow
// OnAdvanceLogic       -   This function gets call ones per game loop. Used to 
//                          advance the logic of the player.
// OnAnimationInit      -   This function get call when ever an animation is 
//                          assign to the player.
// GetHealth            -   Returns health. 0=dead, 100=full
// UseFocusObject       -   Checks for focus objects and uses one if appropriate.  Returns true if used.
// OnButtonInput        -   Handles button press events
// OnGameSpeak          -   Handles speaking related button press events.
// ProcessAnimEvents    -   Handles events from animations
// UpdateRotation       -   Handles rotating the player (right thumbstick / other things that would effect view)
// CalculateStrafeVelocity- Calculates the appropriate strafe velocity
// CalculateForwardVelocity-Calculates appropriate forward velocity
// ReversingStrafeDirection-Returns true if trying to strafe opposite direction of current strafe
// ReversingMoveDirection - Returns true if trying to move opposite direction of current motion
// ParseOnPainForEffects
//
// GetIsSafeSpot        -   Call to figure out if the current spot is safe enough to consider a save point
//
//=========================================================================

protected:

    virtual void        OnEnumProp                  ( prop_enum&  List );
    virtual xbool       OnProperty                  ( prop_query& I    );
    virtual void        OnInit                      ( void );
    virtual void        OnRender                    ( void );
    virtual void        OnRenderTransparent         ( void );
    virtual void        OnRenderShadowCast          ( u64 ProjMask );
    virtual void        OnAdvanceLogic              ( f32 DeltaTime );

    virtual void        OnAnimEvents                ( void );
            xbool       UseFocusObject              ( void );
    virtual void        OnButtonInput               ( void );
            void        OnGameSpeak                 ( void );
            void        UpdateStickInput            ( void );
            void        ClearStickInput             ( void );
    virtual void        ProcessSfxEvents            ( void );
            xbool       NearMutagenReservoir        ( void );

            xbool       DoFootfallCollisions        ( void );
            void        PlayFootfall                ( f32 DeltaTime );

            // Static foot fall sfx lookup (so net ghost can play these sounds)
public:    
static      char*       GetFootfallHeel             ( s32 Material );
static      char*       GetFootfallToe              ( s32 Material );
static      char*       GetFootfallSlide            ( s32 Material );
static      char*       GetFootfallLandSweetner     ( s32 Material );

protected:
            void        ZoomOutAndReload            ( void );

    //------------------------------------------------------------------------------
    // Animation initialization routines
    //------------------------------------------------------------------------------
    virtual void        OnAnimationInit             ( void );
            void        OnWeaponAnimInit2           ( inven_item WeaponItem, new_weapon* pWeapon );
            void        ResetAnimationTable         ( void );
            void        ResetWeaponAnimTable2       ( inven_item WeaponItem );

    virtual radian3     GetProjectileTrajectory     ( void );
    virtual radian3     ApplyAimDegredation         ( radian Pitch, radian Yaw );

    virtual void        UpdateRotation              ( const f32& rDeltaTime );
    virtual void        UpdateCameraShake           ( f32 DeltaTime );
    virtual	void        UpdateCharacterRotation     ( const f32& DeltaTime );
            void        UpdateBulletSounds          ( f32 DeltaTime );

            // Used for determing various movement properties.
            void        CalculatePitchLimits        ( const f32& rDeltaTime );
            void        CalculateRotationAccelerationFactors( f32 DeltaTime );
            void        UpdateRotationRates         ( f32 DeltaTime );
    virtual void        CalculateStrafeVelocity     ( const vector3& rViewX , const f32& rDeltaTime );
    virtual void        CalculateForwardVelocity    ( const vector3& rViewZ , const f32& rDeltaTime );
            xbool       ReversingStrafeDirection    ( const f32& rMaxStrafe );
            xbool       ReversingMoveDirection      ( const f32& rMaxForward );
            xbool       HasSpeedReversed            ( void );

            // Used for determining necessary rig offsets.
            void        CalculateRigOffset          ( f32 DeltaTime );
            void        CalculateStrafeRigOffset    ( f32 DeltaTime );
            void        CalculateMoveRigOffset      ( f32 DeltaTime );
            void        CalculateLookHorozOffset    ( f32 DeltaTime );
            void        CalculateLookVertOffset     ( f32 DeltaTime );
            vector3     GetLeanOffset               ( void );

            // Used for determing and applying aim assistance.
            void        UpdateAimAssistance             ( f32 DeltaTime );
            void        UpdateAimOffset                 ( f32 DeltaTime );
            void        UpdateCurrentAimTarget          ( f32 DeltaTime );
            radian      CalculateNecessaryAimAssistYaw  ( object* pObject );
            radian      CalculateNecessaryAimAssistPitch( object* pObject );
            radian      CalculateActualYawToTarget      ( object* pObject );
            radian      CalculateActualPitchToTarget    ( object* pObject );

            // Used for scaling raw controller input to allow 'fine tuning'
            void        ScaleYawAndPitchValues      ( void );


            void        UpdateViewCinematic         ( const f32& rDeltaTime ); 
            
            void        UpdateSafeSpot              ( f32 DeltaTime );
            void        UpdateUserInput             ( f32 DeltaTime );
    virtual void        SwitchWeapon2               ( inven_item WeaponItem );
    virtual void        OnMoveWeapon                ( void );
    virtual void        OnTransformWeapon           ( const matrix4& L2W );

            void        DrawLabelInFront            ( const char* pLabel );

            // Non exclusive states.
            // Stunned
            void        ComputeStunnedPitchYawOffset( radian PitchOffset, radian YawOffset );

            guid                    GetThirdPersonCameraGuid    ( void ) const;
            third_person_camera*    GetThirdPersonCamera        ( void ) const;
            void                    SetupThirdPersonCamera      ( void );
            void                    UpdateThirdPersonCamera     ( void );

            void        UpdateReticleRadius         ( f32 DeltaTime );
            void        AttachWeapon                ( void );
            vector3     GetWeaponCollisionOffset    ( guid WeaponGuid, const vector3& FirePos );
            void        SetCurrentStrain            ( void );
            void        RenderAimAssistDebugInfo    ( void );

            //------------------------------------------------------------------------------
            // Animation initialization routines
            //------------------------------------------------------------------------------
            void        LoadAimAssistTweakHandles   ( void );
            void        LoadAimAssistTweaks         ( void );

    virtual xbool       UsingLoco                   ( void );

//==============================================================================
//  OLD PLAYER_ALL_STRAINS STUFF
//==============================================================================
public:
    // Pickups and inventory
    virtual xbool               OnPickup                    ( pickup& Pickup );
            xbool               CanTakePickup               ( pickup& Pickup );
            void                ItemAcquiredMessage         ( inven_item Item );
            void                ItemFullMessage             ( inven_item Item );
            void                NoWeapon_NoAmmoPickupMessage( inven_item Item );
            void                TakePickup                  ( pickup& Pickup );
            void                RemoveAllWeaponInventory    ( void );
            s32                 GetAmmoFromWeaponType       ( inven_item Item );
            xbool               CheckForDualWeaponSetup     ( void );            
            xbool               LoadWarnsLowAmmo            ( void );

            void                OnMoveViewPosition          ( const vector3& rNewPos );
            xbool               OnStrainProperty            ( prop_query& rPropQuery );
            void                EnumrateStrainControls      ( prop_enum& List );
            xbool               OnStrainControlProperty     ( prop_query& I );
            animation_state     GetCurrentAnimState         ( void )    { return m_CurrentAnimState; }            

    //------------------------------------------------------------------------------
    // Actor overloads.
    //------------------------------------------------------------------------------

            void                DebugSetupInventory         ( const char* pLevelName );
            void                DebugEnableWeapons          ( const char* pLevelName );
    virtual void                InitInventory               ( void );
            void                ReInitInventory             ( void );
    virtual xbool               IsRunning                   ( void )    { return m_IsRunning; }        
    virtual xbool               InTurret                    ( void ) { return m_bInTurret; }
    //------------------------------------------------------------------------------
    // Name checking routines.
    //------------------------------------------------------------------------------
    inven_item                  GetWeaponFromAnimName       ( const char* pAnimName );
    animation_state             GetAnimStateFromName        ( const char* pAnimName );

    //------------------------------------------------------------------------------
    // Type checking routines.
    //------------------------------------------------------------------------------
    xbool                       IsAnimStateAvailable2       ( inven_item WeaponItem, animation_state AnimState );
    inven_item                  GetCurrentGrenadeType2      ( void ){ return m_CurrentGrenadeType2; }

    //------------------------------------------------------------------------------
    // Every frame, player specific.
    //------------------------------------------------------------------------------
    virtual void                OnAliveLogic                ( f32 DeltaTime );
    virtual void                OnDeathLogic                ( f32 DeltaTime );
            void                UpdateAudio                 ( f32 DeltaTime );
            guid                IsInLadderField             ( void ) ;
    virtual xbool               UpdateLadderMovement        ( f32 DeltaTime ) ;
    virtual void                UpdateMovement              ( f32 DeltaTime );
            void                UpdateCrouchHeight          ( const f32& rDeltaTime );

    //------------------------------------------------------------------------------
    // Input handlers
    //------------------------------------------------------------------------------
            xbool                   ShouldSkipWeaponCycle       ( const inven_item& CurrentWeaponItem, const inven_item& NextWeapon );
			inven_item              GetNextAvailableWeapon2 	( const cycle_direction& CycleDirection );
            inven_item              GetCurrentWeapon2           ( void );
            guid                    GetCurrentWeaponGuid2       ( void );

            void                    CameraFall                  ( f32 fPercentHeight );

    //------------------------------------------------------------------------------
    // Strain related methods
    //------------------------------------------------------------------------------
        xbool                   UseAntiMu                       ( collectable_anti_mutagen* pAntiMu );
        void                    SetupStrain                     ( void );
        
#ifdef STUN_PLAYER
    // This is here so I can stun the player with a keypress / controller combo.
    virtual void            StunPlayer                      ( void );
#endif

protected:
            xbool           IsChangingMutation              ( void ) const;
            void            HandleFireInput                 ( xbool IsAlternateFire );
            xbool           SetupDualWeaponDiscard          ( inven_item &WeaponItem );

            // grenade throwin
            void            GetThrowPoints                  ( vector3 &Point1, vector3 &Point2, vector3 &Point3 );
            vector3         SetupGrenadeThrow               ( const vector3 &EventPos );

//==============================================================================
// ANIMATION STATES
//==============================================================================
    virtual void            SetAnimState                    ( animation_state AnimState );
    virtual void            SetAnimation                    ( const animation_state& AnimState , const s32& nAnimIndex , const f32& fBlendTime = DEFAULT_BLEND_TIME );

    virtual void            BeginState                      ( void );
    virtual void            UpdateState                     ( const f32& rDeltaTime );
    virtual void            EndState                        ( void );
            
            // ANIM_STATE_SWITCH_TO
    virtual void            BeginSwitchTo                   ( void );
    virtual void            UpdateSwitchTo                  ( const f32& DeltaTime );
    virtual void            EndSwitchTo                     ( void );
            
            // ANIM_STATE_SWITCH_FROM
    virtual void            BeginSwitchFrom                 ( void );
    virtual void            UpdateSwitchFrom                ( const f32& DeltaTime );
    virtual void            EndSwitchFrom                   ( void );
            
            // ANIM_STATE_IDLE
    virtual void            BeginIdle                       ( void );
    virtual void            UpdateIdle                      ( const f32& DeltaTime );
    virtual void            EndIdle                         ( void );

            // ANIM_STATE_RUN
    virtual void            BeginRun                        ( void );
    virtual void            UpdateRun                       ( const f32& DeltaTime );
    virtual void            EndRun                          ( void );

            // ANIM_STATE_PICKUP
    virtual void            BeginPickup                     ( void );
    virtual void            UpdatePickup                    ( const f32& DeltaTime );
    virtual void            EndPickup                       ( void );

            // ANIM_STATE_DISCARD
    virtual void            BeginDiscard                    ( void );
    virtual void            UpdateDiscard                   ( const f32& DeltaTime );
    virtual void            EndDiscard                      ( void );

            //  ANIM_STATE_FIRE
    virtual void            BeginFire                       ( void );
    virtual void            UpdateFire                      ( const f32& DeltaTime );
    virtual void            EndFire                         ( void );

            //  ANIM_STATE_ALT_FIRE
    virtual void            BeginAltFire                    ( void );
    virtual void            UpdateAltFire                   ( const f32& DeltaTime );
    virtual void            EndAltFire                      ( void );

            //  ANIM_STATE_GRENADE
    virtual void            BeginGrenade                    ( void );
    virtual void            UpdateGrenade                   ( const f32& DeltaTime );
    virtual void            EndGrenade                      ( void );

            //  ANIM_STATE_ALT_GRENADE
    virtual void            BeginAltGrenade                 ( void );
    virtual void            UpdateAltGrenade                ( const f32& DeltaTime );
    virtual void            EndAltGrenade                   ( void );

            // ANIM_STATE_MELEE_XXX -- Mutant Extreme Melee stuff
    virtual void            BeginMelee_Special              ( const animation_state& AnimState );
    virtual void            UpdateMelee_Special             ( const f32& rDeltaTime, const animation_state& AnimState );
    virtual void            EndMelee_Special                ( const animation_state& AnimState );

            //  ANIM_STATE_MELEE
    virtual void            BeginMelee                      ( void );
    virtual void            UpdateMelee                     ( const f32& DeltaTime );
    virtual void            EndMelee                        ( void );

            // ANIM_STATE_COMBO_BEGIN
    virtual void            BeginCombo                      ( void );
    virtual void            UpdateCombo                     ( const f32& rDeltaTime );
    virtual void            EndCombo                        ( void );
    virtual void            ClearCombo                      ( void );

            // ANIM_STATE_COMBO_HIT
    virtual void            BeginCombo_Hit                  ( void );
    virtual void            UpdateCombo_Hit                 ( const f32& rDeltaTime );
    virtual void            EndCombo_Hit                    ( void );

    // ANIM_STATE_COMBO_HIT
    virtual void            BeginCombo_End                  ( void );
    virtual void            UpdateCombo_End                 ( const f32& rDeltaTime );
    virtual void            EndCombo_End                    ( void );

            //  ANIM_STATE_RELOAD
    virtual void            BeginReload                     ( void );
    virtual void            UpdateReload                    ( const f32& DeltaTime );
    virtual void            EndReload                       ( void );

            //  ANIM_STATE_RELOAD_IN
    virtual void            BeginReloadIn                   ( void );
    virtual void            UpdateReloadIn                  ( const f32& DeltaTime );
    virtual void            EndReloadIn                     ( void );

            //  ANIM_STATE_RELOAD_OUT
    virtual void            BeginReloadOut                  ( void );
    virtual void            UpdateReloadOut                 ( const f32& DeltaTime );
    virtual void            EndReloadOut                    ( void );

            //  ANIM_STATE_RAMP_UP
    virtual void            BeginRampUp                     ( void );
    virtual void            UpdateRampUp                    ( const f32& DeltaTime );
    virtual void            EndRampUp                       ( void );

            //  ANIM_STATE_RAMP_DOWN
    virtual void            BeginRampDown                   ( void );
    virtual void            UpdateRampDown                  ( const f32& DeltaTime );
    virtual void            EndRampDown                     ( void );

            //  ANIM_STATE_ALT_RAMP_UP
    virtual void            BeginAltRampUp                  ( void ) ;
    virtual void            UpdateAltRampUp                 ( const f32& DeltaTime ) ;
    virtual void            EndAltRampUp                    ( void ) ;

            //  ANIM_STATE_ALT_RAMP_DOWN
    virtual void            BeginAltRampDown                ( void ) ;
    virtual void            UpdateAltRampDown               ( const f32& DeltaTime ) ;
    virtual void            EndAltRampDown                  ( void ) ;

            //  ANIM_STATE_HOLD
    virtual void            BeginHold                       ( void );
    virtual void            UpdateHold                      ( const f32& DeltaTime );
    virtual void            EndHold                         ( void );

            //  ANIM_STATE_ALT_HOLD
    virtual void            BeginAltHold                    ( void );
    virtual void            UpdateAltHold                   ( const f32& DeltaTime );
    virtual void            EndAltHold                      ( void );

            //  ANIM_STATE_ZOOM_IN
    virtual void            BeginZoomIn                     ( void );
    virtual void            UpdateZoomIn                    ( const f32& DeltaTime );
    virtual void            EndZoomIn                       ( void );

            //  ANIM_STATE_ZOOM_OUT
    virtual void            BeginZoomOut                    ( void );
    virtual void            UpdateZoomOut                   ( const f32& DeltaTime );
    virtual void            EndZoomOut                      ( void );

            //  ANIM_STATE_ZOOM_IDLE
    virtual void            BeginZoomIdle                   ( void );
    virtual void            UpdateZoomIdle                  ( const f32& DeltaTime );
    virtual void            EndZoomIdle                     ( void );

            //  ANIM_STATE_ZOOM_RUN
    virtual void            BeginZoomRun                    ( void );
    virtual void            UpdateZoomRun                   ( const f32& DeltaTime );
    virtual void            EndZoomRun                      ( void );

            //  ANIM_STATE_ZOOM_FIRE
    virtual void            BeginZoomFire                   ( void );
    virtual void            UpdateZoomFire                  ( const f32& DeltaTime );
    virtual void            EndZoomFire                     ( void );

            //  ANIM_STATE_DEAD
    virtual void            BeginDeath                      ( void );
    virtual void            UpdateDeath                     ( const f32& DeltaTime );
    virtual void            EndDeath                        ( void );

            //  ANIM_STATE_CHANGE_MUTATION,
    virtual void            BeginMutationChange             ( void );
    virtual void            UpdateMutationChange            ( f32 DeltaTime );
    virtual void            EndMutationChange               ( void );

            //  ANIM_STATE_FALLING_TO_DEATH
    virtual void            BeginFallingToDeath             ( void );
    virtual void            UpdateFallingToDeath            ( f32 DeltaTime );
    virtual void            EndFallingToDeath               ( void );

            //  ANIM_STATE_CINEMA
    virtual void            BeginCinema                     ( void );
    virtual void            UpdateCinema                    ( f32 DeltaTime );
    virtual void            EndCinema                       ( void );

            //  ANIM_STATE_MISSION_FAILED
    virtual void            BeginMissionFailed              ( void );
    virtual void            UpdateMissionFailed             ( f32 DeltaTime );
    virtual void            EndMissionFailed                ( void );

            //------------------------------------------------------------------------------
            // Generic state helper routines.
            //------------------------------------------------------------------------------
			animation_state	GetMotionTransitionAnimState	( void );
			void			GenerateFiringAnimPercentages	( void );
			s32				GetNextFiringAnimIndex			( void );
            void            ForceNextWeapon                 ( void );
            void            ForceMutationChange             ( xbool bMutate );

            //------------------------------------------------------------------------------
            // Non exclusive state machine.
            //------------------------------------------------------------------------------
            void            SetNonExclusiveState            ( non_exclusive_states nStateBit );
            void            ClearNonExclusiveState          ( non_exclusive_states nStateBit );
            void            ClearAllNonExclusiveStates      ( void );

            void            BeginNonExclusiveState          ( non_exclusive_states nStateBit );
            void            UpdateActiveNonExclusiveStates  ( f32 DeltaTime );
            void            EndNonExclusiveState            ( non_exclusive_states nStateBit );

            // Non exclusive states.
            // Stunned
            void            BeginStunnedNE                  ( void );
            void            UpdateStunnedNE                 ( f32 DeltaTime );
            void            EndStunnedNE                    ( void );
    virtual void            ProcessStunnedPain              ( const pain& Pain );

    const char*             GetCurrentWeaponName            ( void );
   
            void            ResetWeaponFlags                ( void );
    new_weapon::reticle_radius_parameters GetReticleParams  ( void );
    virtual s32             GetWeaponRenderState            ( void );

public:
    vector3         GetEyesPosition         ( void ) const;
    void            GetEyesPitchYaw         ( radian& Pitch, radian& Yaw ) const;
    virtual         vector3         GetPositionWithOffset   ( eOffsetPos offset );

    const   matrix4&        GetL2W                  ( void ) const;          


                    void            EndZoomState            ( void );

    virtual         xbool           AddMutagen              ( const f32& nDeltaMutagen );
    virtual         f32             GetMutagen              ( void ) { return m_Mutagen; }
    virtual         f32             GetMaxMutagen           ( void ) { return m_MaxMutagen; }
    virtual         void            SetMutagen              ( f32 mutagenAmount ){ m_Mutagen = mutagenAmount; }
                    void            UpdateConvulsion        ( void );

//  virtual         void            ContagionTick           ( void );
    virtual         void            ContagionDOT            ( void );

                    skin_inst&      GetThirdPersonInst      ( void ) { return m_SkinInst; }
                anim_group::handle& GetAnimGroupHandle      ( void ) { return m_hAnimGroup; }

    virtual         void            UpdateFellFromAltitude  ( void );
    virtual         void            TakeFallPain            ( void );
   
                    xbool           ReloadWeapon            ( const new_weapon::ammo_priority& Priority, xbool bCheckAmmo = TRUE );
                    void            LogWeapons              ( void );

protected:

    virtual         s32             GetMaterial             ( void ) const { return MAT_TYPE_FLESH; }

#if !defined(X_RETAIL) && !defined(X_EDITOR)
                    void            AddAllWeaponsToInventory( void );
                    xbool           ShouldMonkeyAddAllWeapons( void );
#endif

public:
            void                    CreateAllWeaponObjects  ( void );            

    virtual void                    OnColCheck              ( void );    
            bbox                    GetLocalBBox            ( void ) const;
            bbox                    GetColBBox              ( void );
    virtual vector3                 GetVelocity             ( void ) const;          
    virtual void                    ScaleVelocity           ( const vector3& PlaneNormal, f32 PerpScale, f32 ParaScale );

            xbool                   IsCinemaRunning         ( void ) { return m_Cinema.m_bCinemaOn; }

#ifndef X_RETAIL
    virtual void                    OnColRender             ( xbool bRenderHigh );
#endif // X_RETAIL

    //--------------------------------------------------------------------------
    // Input handlers
    //--------------------------------------------------------------------------
    void                    OnPrimaryFire               ( void );
    void                    OnSecondaryFire             ( void );
    void                    OnReload                    ( void );
    xbool                   AllowedToFire               ( void );

    // Editor special case functions
#ifdef X_EDITOR
    virtual void            EditorPreGame           ( void );
#endif // X_EDITOR
//==============================================================================
//  NETWORKING STUFF
//==============================================================================
#ifndef X_EDITOR
//------------------------------------------------------------------------------

public:

    virtual void            net_Activate        ( void );
    virtual void            net_Logic           ( f32 DeltaTime );
    virtual xbool           net_EquipWeapon2    ( inven_item WeaponItem );

//------------------------------------------------------------------------------
#endif // X_EDITOR
//==============================================================================

protected:
    static view             m_Views[MAX_LOCAL_PLAYERS];     // Views for all the players
    view_info               m_ViewInfo;                     // persistent information for the player's view
    view_info               m_OriginalViewInfo;             // original persistent information for the player's view
    vector3                 m_RespawnPosition;              // Position where the player re-spawns after dying
    u8                      m_RespawnZone;                  // Zone to respawn in
    guid                    m_ThirdPersonCameraGuid;        // GUID of third person camera if we're using one

    f32                     m_PitchAccelTime;               // How long does it take to get to full pitch turning speed?
    f32                     m_YawAccelTime;                 // How long does it take to get to full yaw turning speed?
    radian                  m_PitchRate;                    // How fast pitch is changing, positive or negative, radians/second
    radian                  m_YawRate;                      // How fast yaw   is changing, positive or negative, radians/second
    f32                     m_PitchAccelFactor;             // Ranges from 0 - 1 over the course of m_PitchAccelTime.
    f32                     m_YawAccelFactor;               // Ranges from 0 - 1 over the course of m_YawAccelTime.
    f32                     m_PitchMax;                     // Maximum pitch of the player
    f32                     m_PitchMin;                     // Minimum pitch of the player
    f32                     m_DesiredPitchMax;              // Desired maximum pitch for the player
    f32                     m_DesiredPitchMin;              // Desired minimum pitch for the player
    f32                     m_fYawStickSensitivity;         // How responsive is the right stick ( 0.f - Any.f )
    f32                     m_fPitchStickSensitivity;       // How responsive is the right stick ( 0.f - Any.f )
    f32                     m_fOriginalYawStickSensitivity; // Original responsiveness of the right stick.
    f32                     m_fOriginalPitchStickSensitivity; // Original responsiveness of the right stick.

    vector3                 m_EyesOffset;                   // This offset is from the top of his head.
    f32                     m_ShakeTime;                    // Shake time remaining
    f32                     m_ShakeAngle;                   // Shake sine angle
    f32                     m_ShakeAmount;                  // How much to shake the camera.
    f32                     m_ShakeSpeed;                   // How fast to shake the camera.

    s32                     m_ActivePlayerPad;              // The pad that this player is currently using.
    s32                     m_LocalSlot;
    f32                     m_MaxFowardVelocity;            // Maximum foward/Backwards velocity
    f32                     m_MaxStrafeVelocity;            // Maximum strafe velocity
    f32                     m_JumpVelocity;                 // How much velocity he has at the take off of the jump

    vector3                 m_ForwardVelocity;              // Actual forward velocity
    vector3                 m_StrafeVelocity;               // Actual strafe velocity
    f32                     m_fForwardAccel;                // Scalar rate of forward acceleration
    f32                     m_fStrafeAccel;                 // Scalar rate of strafe acceleration
    f32                     m_fForwardSpeed;                // Scalar forward speed
    f32                     m_fCurrentYawOffset;            // Current yaw offset for character
    f32                     m_fCurrentPitchOffset;          // Current pitch offset for character
    f32                     m_fPitchChangeSpeed;            // Rate of change for the pitch
    f32                     m_fPrevForwardSpeed;            // Forward speed last frame
    f32                     m_fStrafeSpeed;                 // Scalar strafe speed
    f32                     m_fDecelerationFactor;          // Multiply acceleration by this when slowing down so the player slows down quicker than he speeds up.
    f32                     m_SoftLeanAmount;               // Lean amount softened by a sine wave
    vector3                 m_LeanWeaponOffset;             // Keeps the weapon in the center when leaning

    f32                     m_PitchArmsScalerPositive;      // Scales the pitch>0 before is used for the arms
    f32                     m_PitchArmsScalerNegative;      // Scales the pitch<0 before is used for the arms

    f32                     m_fCurrentCrouchFactor;
    f32                     m_fCrouchChangeRate;
    xbool                   m_bInTurret;

    char*                   m_pPlayerTitle;                 // The title of the player such (Stage 5)

    f32                     m_fMinWalkSpeed;                // Minimum speed needed to walk
    f32                     m_fMinRunSpeed;                 // Minimum speed needed to run

    s32                     m_iCameraBone;                  // Which Bone in the skeleton is the camera bone

    s32                     m_NonExclusiveStateBitFlag;     // Bits representing which non-exclusive states are active.

    // Stunned state variables.
    radian                      m_PreStunPitch;                         // What was the pitch going into the stun.
    radian                      m_PreStunYaw;                           // What was the yaw going into the stun.
    radian                      m_PreStunRoll;                          // What was the roll going into the stun.
    f32                         m_fStunnedTime;                         // How long have we been stunned.
    f32                         m_fMaxStunTime;                         // How long do we stay stunned for?
    radian                      m_MaxStunPitchOffset;                   // Maximum pitch offset while stunned.
    radian                      m_MaxStunYawOffset;                     // Maximum yaw offset while stunned.
    radian                      m_MaxStunRollOffset;                    // Maximum roll offset while stunned.
    f32                         m_fStunYawChangeSpeed;
    f32                         m_fStunPitchChangeSpeed;
    f32                         m_fStunRollChangeSpeed;


    bullet_fly_by           m_BulletFlyBy[MAX_FLY_BYS];

    f32                     m_fShakeAmpScalar;
    f32                     m_fShakeFreqScalar;
    f32                     m_fShakeMaxPitch;
    f32                     m_fShakeMaxYaw;
    f32                     m_PeakLandVelocity;             // The maximum vertival velocity the player reached while landing.
    f32                     m_PeakJumpVelocity;             // The maximum vertival velocity the player reached while jumping.

    guid                    m_LoreObjectGuids[MAX_LORE_ITEMS];      // The lore objects for this level
    xbool                   m_bAllLoreObjectsCollected;             // have we collected all the lore objects
    xbool                   m_bJBGLoreAcquired;

    f32                     m_ReticleMovementDegrade;

    f32                     m_InvalidSoundTimer;

protected:
    
    // Values for adjusting the rig's position relative to the camera to cause 'gun movement'
    vector3                 m_vRigOffset;
    f32                     m_fRigMaxStrafeOffset;
    f32                     m_fRigMaxMoveOffset;
    f32                     m_fRigMoveOffsetVelocity;
    f32                     m_fRigStrafeOffsetVelocity;
    f32                     m_fCurrentMoveRigOffset;
    f32                     m_fCurrentStrafeRigOffset;

    // Values for adjusting the rig's rotation relative to the camera to cause 'gun movement'
    radian3                 m_RigLookOffset;
    radian                  m_RigLookMaxVertOffset;
    radian                  m_RigLookMaxHorozOffset;
    radian                  m_RigLookVertVelocity;
    radian                  m_RigLookHorozVelocity;
    radian                  m_CurrentVertRigOffset;
    radian                  m_CurrentHorozRigOffset;

    // Values for augmenting the control input relating to aim assistance
    f32                     m_fCurrentPitchAimModifier;
    f32                     m_fCurrentYawAimModifier;

    // Values for scaling raw controller input to allow 'fine tuning'
    f32                     m_fFineTuneThreshold;
    f32                     m_fYawValueAtFineTuneThreshold;
    f32                     m_fPitchValueAtFineTuneThreshold;

    f32                     m_fMidRangeThreshold;
    f32                     m_fYawValueAtMidrangeThreshold;
    f32                     m_fPitchValueAtMidrangeThreshold;

    radian                  m_YawAimOffset;

    f32                     m_TimeSinceLastZonePain;

    xarray<pain>            m_LastPainEvent;


    s32                     m_AudioEarID;
    
protected:

    static guid             s_ActivePlayerGuid;             // The guid of the active player

    //controller stick input for this frame.
    f32                     m_fMoveValue;
    f32                     m_fStrafeValue;
    f32                     m_fYawValue;
    f32                     m_fPitchValue;
    f32                     m_fRawControllerYaw;
    f32                     m_fRawControllerPitch;

    xbool                   m_bVoteButtonPressed;
    xbool                   m_bRespawnButtonPressed;

    // Controller input from last frame.
    f32                     m_fPreviousYawValue;
    f32                     m_fPreviousPitchValue;

    //  Values used to help determine if the player is currently safe
    f32                     m_LastTimeSeenByEnemy;          // Last time a unit fired at us
    f32                     m_LastTimeTookDamage;           // Last time player took damage

    vector3                 m_PositionOfLastSafeSpot;       // Position of last safe spot
    zone_mgr::zone_id       m_ZoneIDOfLastSafeSpot;         // Zone ID of last safe spot

    vector3                 m_NextPositionOfLastSafeSpot;   // Spot that will become the new safe spot
    zone_mgr::zone_id       m_NextZoneIDOfLastSafeSpot;     // ID that will become Zone ID of last safe spot


    f32                     m_AimDegradation;               // How bad aim is (0 best, 1 worst)
    f32                     m_AimRecoverSpeed;              // How quickly to recover aim (0.1 = slow, 1 = fast)

    f32                     m_YawMod;                       // Altered view based on pain impacts;
    f32                     m_PitchMod;                     
    f32                     m_RollMod;
    f32                     m_ShakePitch;
    f32                     m_ShakeYaw;
    xtick                   m_NearbyObjectCounter;
    xtick                   m_GameSpeakCounter;
    guid                    m_SpeakToGuid;                  // Guid of person who the player MAY speak to.
                                                            // Updated every .25 seconds.
    guid                    m_GameSpeakEmitterGuid;
   
    f32                     m_ProximityAlertRadius;

    guid                    m_CurrentTargetingAssistTarget;  // guid of the object that we are targeting.
    f32                     m_DistanceToAimAssistTarget;
    radian3                 m_CurrentTargetingModifation;
    radian3                 m_OffsetToTarget;
    f32                     m_AimAssistPct;
    xarray<prop_container>  m_SaveSpotProperties;  // backup of properties for save spots


protected:
    
    //Quick implementation of the lock view, needs to be replaced by the cinematic engine...

    lock_view_node          m_LockViewTable[lock_player_view::MAX_TABLE_SIZE];  
    xtick                   m_TimeStartTick;
    xbool                   m_ViewCinematicPlaying;
    s32                     m_CurrentViewNode;
    quaternion              m_StartView;
    quaternion              m_DesiredView;

    f32                     m_CScale;
    f32                     m_CTimeSum;

protected:
    
    xbool                   m_bActivePlayer;
    xbool                   m_bSpeaking;

    guid                    m_LastLadderGuid;
    guid                    m_JumpedOffLadderGuid;

    weapon_state            m_WeaponState;
    f32                     m_ReticleRadius;
    f32                     m_ReticleGrowSpeed;
    f32                     m_ReticleShotPenalty;

    vector3                 m_ArmsOffset;
    vector3                 m_ArmsVelocity;
    vector3                 m_WeaponCollisionOffset;
    f32                     m_LastWeaponCollisionOffsetScalar;
    f32                     m_WeaponCollisionOffsetScalar; // this is the one we use for springing pullback
    AimAssistData           m_AimAssistData;

    // Hide player arms for things like the decontamination sequence
    xbool                   m_bHidePlayerArms;
    xbool                   m_bArmsWereHidden; // used so we can tell if we need to play the switch to anim
    xbool                   m_bPlaySwitchTo;   // if this is true, this will play the "switchto" animation after arms re-appear from m_bHidePlyerArms
    
    // Can we do a tap fire?  Resets when the player releases the button (or maybe in the future after a timer?).
    xbool                   m_bCanTapFire;
    f32                   m_TapRefireTime;
    
    //------------------------------------------------------------------------------
    // Flashlight info
    //------------------------------------------------------------------------------
    xbool                   m_bUsingFlashlight;
    xbool                   m_bUsingFlashlightBeforeCinema;
    guid                    m_FlashlightGuid;                   // guid to a projected texture (attached to the gun)
    f32                     m_BatteryChangeTime;                // what is the accumulated time for changing battery value
    f32                     m_Battery;
    f32                     m_MaxBattery;
    f32                     m_FlashlightTimeout;

    f32                     m_fLastItemFullTime;
    f32                     m_fLastItemAcquiredTime;
    //------------------------------------------------------------------------------

//===================================================================
// CINEMATIC MEMBERS
//===================================================================
protected:

    struct cinema
    {
        xbool       m_bCinemaOn;                // Is cinema mode on?
        xbool       m_bPlayerZoneInitialized;   // Has player zone been initialized to cameras?
        guid        m_LookAtTargetGuid;         // Look at a target during the cinema
        vector3     m_CurrentLookDir;           // Current looking direction
        vector3     m_DesiredLookDir;           // Desired looking direction
        f32         m_BlendInTime;              // Cinema view blend in time
        f32         m_CurrentBlendInTime;       // Current view blend in time
        guid        m_CinemaCameraGuid;         // Cinema camera
        vector3     m_ViewCorrectionDelta;      // Used to put view back to the player
        xbool       m_bUseViewCorrection;       // Use the view correction blending?
        matrix4     m_CameraV2W;                // Camera view to world matrix for final view
        radian      m_CameraXFOV;               // Field of view to use for final view
    };

    cinema                      m_Cinema;

//===================================================================
// OLD PLAYER_ALL_STRAINS MEMBERS
//===================================================================
protected:
    
    strain_control_modifiers    m_StrainControls;
    xbool                       m_bStrainInitialized;
    xbool                       m_PlayMeleeSound;
    xbool                       m_IsRunning;
    s32                         m_ZoomLevelAfterReload;

    //animation tables / players for all of the strains.
    state_anims				    m_Anim[ INVEN_NUM_WEAPONS ][ ANIM_STATE_MAX ];
    anim_group::handle          m_AnimGroup;                        // Animation file for the player
    rhandle<char>               m_hAudioPackage;                    // Audio resource for the player.
    skin_inst                   m_Skin;                             // Geometry use to render the player
    s32                         m_iCameraTargetBone;                // Which bone in the skeleton is the target bone
    u32                         m_StrainFriendFlags;                //  Flags of our friends.
    
    char_anim_player            m_AnimPlayer;                       // Animation player used to play the arms animations

    animation_state             m_CurrentAnimState;                 //current animation state of the player
    animation_state             m_PreviousAnimState;                //previous animation state of the player
    animation_state             m_NextAnimState;                    //next animation state of the player
    s32                         m_AnimStage;                        //used to track where in a series of anims, that we are
                                                                    //valid until next state change    

    animation_state             m_MeleeAnimStates[MAX_MELEE_STATES];// randomized list to make sure we don't play the same attack consecutively
    s32                         m_MeleeAnimStateIndex;              // Save off the index we are using so that we can walk the list.

    inven_item                  m_CurrentGrenadeType2;              // The Current grenade that is equiped.

	s32					        m_CurrentAnimIndex;                 // Current animation index that is playing
	s32					        m_PreviousAnimIndex;                // Previous animation index that was playing.
    s32                         m_CurrentAnimStateIndex;            // Current animation state index that is playing
    s32                         m_PreviousAnimStateIndex;           // Previous animation state index that is playing

    f32                         m_fAnimationTime;                   // Time animation has been playing
	f32                         m_fMaxAnimTime;                     // Max amount of time an animation can play 

	f32					        m_fAnimPriorityPercentage[MAX_ANIM_PER_STATE];	//Values used to determine which animations we are going to play

    vector3                     m_PosOverrideCamera;                // Override camera position (used for death rather than using physic position)
    f32                         m_WpnHoldTime;                      // How long the weapon has been held    
    f32                         m_LastTimeWeaponFired;              // Last time player fired his weapon
    
    // Ladder variables
    xbool                       m_bOnLadder ;                       // TRUE if player is on a ladder
    vector3                     m_LadderOutDir ;                    // Ladder out direction (only valid if on a ladder)

    f32                         m_MaxAnimWeaponHoldTime;            // How long till be get to "jerkiest" animation when in weapon hold state;
    s32                         m_MutationAudioLoopSfx;
    xbool                       m_NeedRelaodIn;
    s32                         m_LastFireAnimStateIndex;
    s32                         m_nLoreDiscoveries;                 // 1st pass at unlocking secret galleries
    f32                         m_DebounceTime;
    xbool                       m_bWasMutated;
    xbool                       m_bIsMutantVisionOn;                // toggles based on super-event in weapon animation
    inven_item                  m_PreMutationWeapon2;               // what was our weapon before we mutated
public:
    xbool                       m_bMutationMeleeEnabled;            // Can we use the mutation melee attack?
    xbool                       m_bPrimaryMutationFireEnabled;      // Can we fire our primary mutation ammo?
    xbool                       m_bSecondaryMutationFireEnabled;    // Can we fire our secondary mutation ammo?
protected:
    xbool                       m_bMeleeLunging;                    // are we in the middle of a lunge?
    xbool                       m_bHolsterWeapon;                   // is our weapon in its holster?
    f32                         m_MeleeDamage;                      // How much damage per melee (not mutant melee)
    f32                         m_MeleeForce;                       // How much force from our melee (not mutant melee)
    xbool                       m_bInMutationTutorial;              // is the mutation tutorial running?
    convulsion_info             m_ConvulsionInfo;                   // stores info for our mutation tutorial convulsions
    f32                         m_ConvulsionFeedbackDuration;
    f32                         m_ConvulsionFeedbackIntensity;
    xbool                       m_bHitCombo;                        // did we hit the melee button and activate another combo within the threshold?
    xbool                       m_bCanRequestCombo;                 // are we in the request threshold?
    xbool                       m_bLastMeleeHit;                    // flag to make sure we hit something during a weapon melee
    s8                          m_ComboCount;                       // how many times have we combo'd?

    xbool                       m_bTweakHandlesLoaded;              // have the tweak handles been loaded?

    //------------------------------------------------------------------------------
    // Begin members from ghost.cpp/hpp
    //------------------------------------------------------------------------------

protected:
    f32                     m_Mutagen;
    f32                     m_MaxMutagen;                   // Player's maximum mutagen level
    vector3                 m_EyesPosition;                 // Where the player's eyes are after the last ComputeView() call
    radian                  m_EyesPitch;                    // Eye's pitch after the last ComputeView() call
    radian                  m_EyesYaw;                      // Eye's yaw after the last ComputeView() call

    voice_id                m_SuckingMutagenLoopID;         // the sound getting mutagen from a SCDB makes.

#if !defined( CONFIG_RETAIL )
    xbool                   m_bRenderSkeleton;              // Whether to render the skeleton or not
    xbool                   m_bRenderSkeletonNames;         // When ever the skeleton gets render whther to also print the name of the bones
    xbool                   m_bRenderBBox;                  // Renders the bbox of the player this is use mainly in the editor
#endif // !defined( CONFIG_RETAIL )

public:
    inven_item              m_PrevWeaponItem;
    inven_item              m_NextWeaponItem;

protected:
    // Locomotion                               
    character_physics       m_Physics;                      // Physics to drive the motion of the player
    player_loco             m_Loco;
    f32                     m_DeathTime;

    xbool                   m_bFalling;                     // used for pain when we land
    xbool                   m_bJustLanded;
    vector3                 m_DeltaPos;                     // current velocity / delta time
    xbool                   m_bCanJump;


    f32                     m_DeltaTime;
    f32                     m_TimeInState;

    s32                     m_MissionFailedTableName;
    s32                     m_MissionFailedReasonName;
    rhandle<xbitmap>        m_MissionFailedBmp;
  
    //------------------------------------------------------------------------------
    // End members from ghost.cpp/hpp
    //------------------------------------------------------------------------------

    xbool                   m_VoteMode;

    //------------------------------------------------------------------------------
    // Footfall members
    //------------------------------------------------------------------------------

    f32                     m_DelayTillNextStep;
    f32                     m_DistanceTraveled;
    f32                     m_DelayCountDown;
    s32                     m_HeelID;
    s32                     m_SlideID;
    s32                     m_ToeID;
    s32                     m_TrailStep;

    //------------------------------------------------------------------------------
    // Manned turret members
    //------------------------------------------------------------------------------
    struct manned_turret
    {
        guid                TurretGuid;         // guid of the turret we're manning -- this will be hidden
        guid                Turret2Guid;        // guid of a part of the turret we're manning -- this will be hidden
        guid                Turret3Guid;        // guid of a part of the turret we're manning -- this will be hidden
        guid                LeftBoundaryGuid;   // guid of the object whose position is to be used for left aim boundary
        guid                RightBoundaryGuid;  // guid of the object whose position is to be used for right aim boundary
        guid                UpperBoundaryGuid;  // guid of the object whose position is to be used for upper aim boundary
        guid                LowerBoundaryGuid;  // guid of the object whose position is to be used for lower aim boundary
        inven_item          PreviousWeapon;     // weapon before we manned the turret
        matrix4             PreviousL2W;        // L2W before we manned the turret
        matrix4             AnchorL2W;
    };
    manned_turret           m_Turret;
    f32                     m_MutationChangeTime;
    f32                     m_UseTime;              // how long since we last used an item

    //------------------------------------------------------------------------------
    //  State manager movie control
    //------------------------------------------------------------------------------
public:
    static xbool            s_bPlayerDied;      // Set in player::OnDeath.  Cleared by level_loader::LoadLevel
                                                // when level initially loads.  Checked by
                                                // state mgr in state_mgr::EnterSinglePlayerLoadMission
protected:


    friend class check_point_mgr;
    friend class state_mgr;
}; 

//=========================================================================

inline
vector3 player::GetVelocity( void ) const
{
    // Use physics
    return m_Physics.GetVelocity();
}

//=========================================================================

inline
void player::ResetStickSensitivity( void )
{
    m_fPitchStickSensitivity = m_fOriginalPitchStickSensitivity;
    m_fYawStickSensitivity   = m_fOriginalYawStickSensitivity;
}

//===========================================================================

inline
void player::SetStickSensitivity( const f32& rMultiplier )
{
    m_fPitchStickSensitivity *= rMultiplier;
    m_fYawStickSensitivity   *= rMultiplier;
}

//===========================================================================

inline
f32 player::GetCollisionHeight( void )
{
    return m_Physics.GetColHeight();
}

//===========================================================================

inline
f32 player::GetCollisionRadius( void )
{
    return m_Physics.GetColRadius();
}

//===========================================================================

inline
void player::AddImpulse( vector3& rImpulse )
{
    m_Physics.AddVelocity( rImpulse );
}

//===========================================================================

inline
inven_item player::GetCurrentWeapon2( void )
{
    return m_CurrentWeaponItem;
}

//===========================================================================
inline
guid player::GetThirdPersonCameraGuid( void ) const
{
    return m_ThirdPersonCameraGuid;
}

//==============================================================================

inline
vector3 player::GetEyesPosition( void ) const
{
    return m_EyesPosition;
}

//==============================================================================

inline
void player::GetEyesPitchYaw( radian& Pitch, radian& Yaw ) const
{
    Pitch   = m_EyesPitch;
    Yaw     = m_EyesYaw;
}

//==============================================================================
inline
player::convulsion_info::convulsion_info( void ) :
m_TimeSinceLastConvulsion       ( 0.0f ),
m_ConvulseAtTime                ( 0.0f ),
m_bConvulsingNow                ( FALSE ),
m_TimeLeftInThisConvulsion      ( 0.0f )
{
}

inline
xbool player::IsChangingMutation( void ) const
{
    return ((m_CurrentAnimState  == ANIM_STATE_SWITCH_TO ) || (m_CurrentAnimState == ANIM_STATE_SWITCH_FROM))
         && (m_CurrentWeaponItem == INVEN_WEAPON_MUTATION);
}


#endif // PLAYER_HPP
//=========================================================================
