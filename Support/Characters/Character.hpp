//============================================================================== 
// Character.hpp 
// 
// Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved. 
// 
// This is the base class for all NPCs
// 
//==============================================================================

#ifndef __CHARACTER_HPP__
#define __CHARACTER_HPP__ 

//=========================================================================
// INCLUDES
//=========================================================================
#include "navigation\ng_connection2.hpp"
#include "objects\actor\actor.hpp"
#include "BaseStates\Character_Trigger_State.hpp"
#include "BaseStates\Character_Follow_State.hpp"
#include "Objects\Event.hpp"
//=========================================================================
// SPECIAL COMPILER SWITCHES
//=========================================================================

#if ( defined TARGET_PC ) || ( defined TARGET_XBOX )
// Get rid of "warning C4355: 'this' used in base member initializer list"
#pragma warning( disable : 4355 ) 
#endif

//=========================================================================
// Defines
//=========================================================================

enum eNPCName
{
    NPC_GENERIC,
    NPC_BRIDGES,
    NPC_CARSON,
    NPC_CRISPY, 
    NPC_CRISPY_MUTATED,
    NPC_CHEW,   
    NPC_DRCRAY,
    NPC_FERRI,
    NPC_LEONARD,
    NPC_MCCANN, 
    NPC_MRWHITE,
    NPC_RAMIREZ,
    NPC_VICTOR,    
    NPC_SCIENTIST,
    NPC_TECHNICIAN,
    NPC_THETA,
    NPC_ALIENGLOB,    
    NPC_MUTANT,
    NPC_BLACKOPS,
    NPC_BLACKOP_LEADER,
    NPC_UNKNOWN,
    NPC_MAX,
};

#define MAX_ENTITIES_FOR_PROXIMITY_CHECK    6
#define MAX_NODES_IN_PATH_LIST              50
const s32 k_MaxSplinePoints = 6;

const s32 k_MaxAllies = 4;

//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================
class character ;
class character_state ;
class cover_node;
class character_hold_state ;
class action_ai_dialog_line;
class character_task_set;

//=========================================================================
// character_hold_state class
//  [12/12/2003]
//
//  This state is used when the NPC is not active. Does nothing and ignores all.
//=========================================================================

class character_hold_state : public character_state
{
public:
// Construction / destruction
    character_hold_state   ( character& Character, states State );
    
    inline virtual xbool        IgnorePain      ( const pain& Pain );
    inline virtual xbool        IgnoreAlerts    ( void ); 
    inline virtual xbool        IgnoreSight     ( void );
    inline virtual xbool        IgnoreSound     ( void );
    inline virtual xbool        IgnoreAttacks   ( void );

// Virtual functions    
    virtual const char*GetStateName ( void )            { return "CHARACTER_HOLD"; }    
    virtual const char*GetPhaseName ( s32 thePhase )    { (void)thePhase; return "PHASE NONE"; }
};

//=========================================================================
// CLASSES
// Base class character
class character : public actor
{
// Real time type information
public:
    CREATE_RTTI( character, actor, object )

//=========================================================================
// Friends of the family
//=========================================================================
friend class character_state ;
friend class character_death_state;
friend class god ;

//=========================================================================
// Defines
//=========================================================================
public:

    enum eCharacterSubtypes
    {
        SUBTYPE_NONE = -1
    };

    enum eDialogActors
    {
        INVALID_ACTION_TYPES = -1,        
        DIALOG_ACTOR_A,
        DIALOG_ACTOR_B,
        DIALOG_ACTOR_C,
        DIALOG_ACTOR_D,
        DIALOG_ACTOR_NONE,
    };

    enum eAwarenessLevel
    {
        AWARENESS_NONE,
        AWARENESS_COMBAT_READY,
        AWARENESS_ALERT,
        AWARENESS_SEARCHING,
        AWARENESS_ACQUIRING_TARGET,
        AWARENESS_TARGET_LOST,
        AWARENESS_TARGET_SPOTTED,
    };

    enum eLookatModes
    {
        LOOKAT_NONE,
        LOOKAT_FORWARD,
        LOOKAT_LAST_LOCATION_OF_INTEREST,
        LOOKAT_NAVIGATION,
        LOOKAT_NAV_FORWARD,
        LOOKAT_CURRENT_TARGET,
        LOOKAT_DOING_GOAL,
        LOOKAT_ENTERING_COVER,
        LOOKAT_MARKER_ALIGN,
        LOOKAT_INTEREST_OBJECT,
    };

    enum eGoalTypes
    {
        GOAL_NONE,
		GOAL_IDLE,
		GOAL_TURNTO_TARGET,
		GOAL_TURNTO_LOCATION,
		GOAL_LOOKAT_TARGET,
		GOAL_LOOKAT_LOCATION,
		GOAL_GOTO_TARGET,
		GOAL_GOTO_LOCATION,
		GOAL_RETREAT_FROM_TARGET,
		GOAL_RETREAT_FROM_LOCATION,
		GOAL_PLAY_ANIMATION,
		GOAL_PLAY_ANIMATION_SCALED_TO_TARGET,
		GOAL_SAY_DIALOG,
        GOAL_COUNT
    };

    enum eGoalFailureReasons
    {
        FAILED_GOAL_NONE,
        FAILED_GOAL_STUCK,
        FAILED_GOAL_CANT_REACH,
        FAILED_GOAL_BELOW_TARGET,
        FAILED_GOAL_ABOVE_TARGET,
        FAILED_GOAL_TIMED_OUT,
        FAILED_GOAL_PLAY_ANIM_ERROR,
        FAILED_GOAL_PLAY_DIALOG_ERROR,
        FAILED_GOAL_VOICE_TIMED_OUT,
        FAILED_GOAL_ACTOR_COLLISION,
        FAILED_GOAL_OBJECT_COLLISION,
        FAILED_GOAL_NOWHERE_TO_RETREAT,
        FAILED_GOAL_OUTSIDE_LEASH,
    };

    enum eAvoidCharacterAttempt
    {
        AVOID_NONE,
        AVOID_THROUGH,
        AVOID_WAIT,
    };

    enum eEscapeAttempt
    {
        ESCAPE_NONE = -1,
        ESCAPE_GO_LEFT,
        ESCAPE_GO_RIGHT,
        ESCAPE_GO_BACK_LEFT,
        ESCAPE_GO_BACK_RIGHT,
        ESCAPE_HEAD_EXPLODE,
        ESCAPE_SLIGHT_DELAY
    };

    enum eCoverPreference 
    {
        COVER_PREF_NONE,
        COVER_PREF_CLOSER_TO_TARGET,
        COVER_PREF_FURTHER_FROM_TARGET,
        COVER_PREF_CLOSER_TO_US,
    };

    struct dialog_info
    {        
        void            Clear();
        void            SetData( eDialogType dialogType, const char* dialogName , const char* animName , const char* animPkg, u32 Flags = 0, u8 DialogFlags = 0, f32 BlendOutTime = 0.25f );
        // data
        char            m_DialogName[64];
        char            m_AnimName[64];   
        char            m_AnimGroup[64];
        f32             m_BlendOutTime;
        eDialogType     m_DialogType;
        u32             m_AnimFlags;
        u8              m_DialogFlags;
    };

    struct goal_info
    {
        vector3             m_Offset;
        vector3             m_Location;       
        vector3             m_RetreatGoalDestination; 
        char                m_AnimName[64];   
        char                m_AnimGroup[64];
        char                m_DialogName[64];    
        guid                m_TargetGuid;
        radian              m_FOV;
        f32                 m_AnimPlayTime ;        
        f32                 m_DesiredDist;
        f32                 m_DialogAnimBlendOutTime;
        eGoalTypes          m_GoalType;
        loco::move_style    m_MoveStyle;                
        loco::anim_type     m_AnimType;
        f32                 m_AnimBlendTime;
        u32                 m_AnimFlags ;
        u32                 m_IsExact:1,
                            m_UseCurrentTarget:1,
                            m_DialogBlockOnDialog:1,
                            m_DialogKillAnim:1,
                            m_FaceTargetOnRetreat:1;
        u8                  m_DialogFlags;
    };
    
//=========================================================================
// Class functions
//=========================================================================
public:
                        character       ( void );
    virtual             ~character() ;


//=========================================================================
// Inherited virtual functions from base class
//=========================================================================
public:
    virtual void        OnInit                  ( void );
    virtual void        OnKill                  ( void );   
    virtual void        OnAdvanceLogic          ( f32 DeltaTime );      
    virtual void        OnRender                ( void );
            
    virtual void        InitLoco                ( void );
            void        InitTweaks              ( void );

#if !defined(X_RETAIL) || defined(X_QA)
    virtual void        OnDebugRender           ( void );
#endif // X_RETAIL || X_QA

    virtual void        OnRenderTransparent     ( void );
    virtual s32         GetMaterial             ( void ) const ;
    virtual bbox        GetLocalBBox            ( void ) const;
    virtual void        OnEvent                 ( const event& Event );
    virtual void        OnEnumProp              ( prop_enum&    List ) ;
    virtual xbool       OnProperty              ( prop_query&   I    ) ;
    virtual f32         GetYaw                  ( void );
#ifdef X_EDITOR
    virtual s32         OnValidateProperties    ( xstring&       ErrorMsg );
#endif

inline virtual xbool    IsCharacter             ( void );
    
    virtual void        OnKilledEnemy           ( guid deadEnemy );
    virtual void        OnKilledAlly            ( guid deadAlly );
    virtual void        OnDamagedEnemy          ( guid damagedEnemy );
    virtual const char* GetLogicalName          ( void );

//=========================================================================
// Virtual functions
//=========================================================================
public:
    virtual void        OnHitByFriendly         ( guid ShooterID  );
    virtual xbool       OnPlayFullBodyImpactAnim( loco::anim_type AnimType, f32 BlendTime, u32 Flags );
    virtual xbool       GetHasDrainableCorpse   ( void )            { return FALSE; }

    // Navigation helper functions
    virtual void        HandleNavMapUpdate      ( nav_map::update_info& Update );
    virtual xbool       AlwaysAllowMoveBackwards( void )            { return FALSE; }
            s32         GetSubtype              ( void )            { return m_Subtype; }    
            void        SetSubtype              ( s32 newSubtype)   { m_Subtype = newSubtype; }    
    
    inline  nav_connection_slot_id  GetCurrentConnection( void );
    inline  xbool       GetHasClearJumpAttack   ( void );
    inline  xbool       GetHasClearLOSFeet      ( void );
    inline  xbool       GetHasClearLOSEyes      ( void );
    inline  xbool       GetCanMoveLeft          ( void );
    inline  xbool       GetCanMoveRight         ( void );
    inline  xbool       GetCanMoveBack          ( void );
    inline  xbool       GetCanMoveForward       ( void );
    inline  xbool       GetInNavMap             ( void );
    inline  f32         GetTimeOutofNavMap      ( void );
    inline  f32         GetLeashDistance        ( void ) const;
    inline  guid        GetLeashGuid            ( void ) const;
    inline  void        RecalcPath              ( void )            { m_NeedsToRecalcPath = TRUE; }
            xbool       GetClearToMoveTo        ( const vector3& location, xbool doCollisionCheck = TRUE );
            f32         GetSqrHorzDistToNextNav ( void );
            xbool       GetIsAtDestination      ( void );

        // pathing
    inline  xbool       GetIsFollowingPath      ( void );
    inline  void        SetIsFollowingPath      ( xbool followingPath );
            const char* GetEscapeAttemptName    ( void );                        
            const char* GetAvoidAttemptName     ( void );                        
    inline  void        SetCollisionIgnoreGuid  ( guid ignoreGuid );
            xbool       GetIsStuck              ( void )            { return m_IsStuck; }

    // Collision functions
    virtual void        ApplyBoneBBoxCollision  ( void );

    // State functions
    static  s32         GetStateCount           ( void );            
    static  const char* GetStateName            ( s32 Index ); 
    static  const char* GetStatesEnum           ( void);
    static  character_state::states GetStateByName ( const char* pName );                                    
    inline  void        ClearInitialState       ( void );
            void        SetPostLipSyncState     ( character_state::states lipSyncState ) { m_PostLipsyncState = lipSyncState; }
    

    // Event functions
    virtual xbool       OnAnimEvent( const anim_event& Event, const vector3& WorldPos );

    // Alert functions                          
    virtual void        OnAlert                 ( alert_package& Package );

    // Pain/damage functions
    virtual void        OnPain                  ( const pain& Pain );
    virtual xbool       TakeDamage              ( const pain& Pain );
    virtual void        KillMe                  ( void );
            void        DropWeapon              ( void );
            void        DropAmmoForWeapon       ( inven_item weaponItem );
            void        DropGrenade             ( void );
            void        DropInventory           ( void );
            void        SetIgnorePain           ( xbool bShouldIgnorePain );

    // On event or state functions.
    virtual void        OnThink                 ( void ) ;    
    virtual void        OnBeingShotAt           ( object::type ProjectileType , guid ShooterID );
    virtual void        OnHitFriendly           ( guid FriendlyID );
    virtual void        OnGrenadeAlert          ( alert_package& Package );
    virtual void        OnActivate              ( xbool Flag );
    virtual void        OnDeath                 ( void );
    virtual void        OnSpawn                 ( void );
    virtual void        OnContagionDOT          ( void );

    // Trigger state functions
    virtual xbool       TriggerState            ( action_ai_base* pAction );
            void        SetTriggerStateData     ( action_ai_base* pContext );
            xbool       TriggerState            ( character_trigger_state::TriggerData pData );
            void        SetTriggerStateData     ( character_trigger_state::TriggerData pData );
    inline  character_trigger_state::TriggerData GetTriggerStateData( void );
        //task list functions
    inline  void        SetPendingTaskListGuid  ( guid newGuid );
            character_task_set* GetPendingTaskList( void );
            void        SetPostTriggerTarget    ( guid target );

    // Dialog functions
    virtual xbool       PlayDialog              ( eDialogType dialogType, const char* dialogName = NULL, xbool hotVoice = FALSE, const char* animName = NULL, const char* animPkg = NULL, u32 Flags = 0, u8 DialogFlags = 0, f32 BlendOutTime = 0.25f );
    virtual const char* GetDialogPrefix         ( void );
    virtual s32         GetNumberVoiceActors    ( void ) = 0;
    inline  s32         GetVoiceID              ( void );
    inline  void        SetVoiceID              ( s32 ID );

    // Sound/sight related functions
            // sight functions
    inline  f32         GetTargetNotSeenTimer   ( void );
    inline  vector3     GetLastKnownLocationOfTarget( void );
    inline  void        SetLastKnownLocationOfTarget( const vector3& Location );
    inline  vector3     GetLastSeenLocationOfTarget( void );
    inline  f32         GetSightRadius          ( void );
            // sound functions
    inline  vector3     GetLastSoundPosition    ( void );
    inline  f32         GetTimeSinceLastSound   ( void );
    inline  void        ClearSoundHeard         ( void );
    inline  xbool       GetSoundHeard           ( void );
    inline  virtual void Listen                 ( conversation_packet& rPacket );
    virtual xbool       SetPotentialListener    ( xbool bPotentialListener ) ;
    virtual void        SetActiveListener       ( xbool bActiveListener ) ;
            void        SendAlert               ( alert_package::alert_type alertType, guid CauseGuid = 0, xbool forceSend = FALSE );
            void        SendAlert               ( alert_package::alert_type alertType, const vector3& sourceLocation, guid CauseGuid = 0, xbool forceSend = FALSE );
            void        SetAllyAcquiredTarget   ( xbool targetAcquired )    { m_bAllyAcquiredTarget = targetAcquired; }
            xbool       GetAllyAcquiredTarget   ( void )                    { return m_bAllyAcquiredTarget; }

    // Awareness functions
    inline  vector3     GetLastLocationOfInterest( void );
    inline  void        SetLastLocationOfInterest( const vector3& Location );
    inline  eAwarenessLevel GetAwarenessLevel   ( void );
            void        SetAwarenessLevel       ( eAwarenessLevel newAwarenessLevel );
            const char* GetAwarenessLevelName   ( void );
            character_state::states GetStateFromAwareness( void );

    // cloak functions
    virtual void        UpdateCloak             ( f32 DeltaTime );
    virtual void        Cloak                   ( void );
    virtual void        Decloak                 ( void );

    // Rendering functions
    inline  anim_group::handle& GetAnimGroupHandle( void );

    // Target functions
            void        UpdateTarget            ( void ) ;
            void        SetTargetGuid           ( guid Guid, xbool shoutAboutTarget = TRUE ); 
    inline  guid        GetTargetGuid           ( void );
            xbool       IsTargetEnemy           ( guid Guid = 0 );
            xbool       IsValidTarget           ( guid targetGuid );
            void        SetOverrideTarget       ( guid OverrideGuid );
            xbool       OfferOverrideTarget     ( guid OverrideGuid );
    virtual xbool       CanTargetGlobs          ( void )                { return FALSE; }
            xbool       GetIsKungFuTime         ( void );
            

            // aiming
    inline  xbool       CanSeeTarget            ( void );
    inline  xbool       CanShootAtTarget        ( void );
    inline  radian      GetAimToTargetYaw       ( void );
            xbool       IsTargetInFiringCone    ( const vector3& Target );//, vector3& FiringPoint, vector3& AimPoint, radian Threshold = R_20 );
            
            xbool       IsFacingTarget          ( const vector3& TargetPos, radian SightFOV ) ;
            xbool       IsFacingTarget          ( radian SightFOV );
            xbool       IsTargetFacingUs        ( radian SightFOV );

            xbool       IsTargetInSightCone     ( f32 SightRadius, radian SightFOV );   // Uses m_SightDist and m_FOV
            xbool       IsTargetInSightCone     ( const vector3& TargetPos, f32 SightRadius, radian SightFOV ) ;
            xbool       IsTargetInSightCone     ( const vector3& TargetPos, u8 brightness ) ;   // Uses m_SightDist and m_FOV
            xbool       IsTargetInSightCone     ( void ) ;                             // Uses m_SightDist and m_FOV and current target
        // navigation
            vector3     GetToTargetWeighted     ( guid target = 0, eOffsetPos offset = OFFSET_NONE );     // return the distance to either the passed in target, or our target if NULL passed.
            vector3     GetToTarget             ( guid target = 0, eOffsetPos offset = OFFSET_NONE );     // return the distance to either the passed in target, or our target if NULL passed.
            xbool       IsNewTargetCloser       ( guid Guid );
            xbool       UpdateCanReachTarget    ( const vector3& targetPosition, f32 reachRadius );
    inline  xbool       CanReachTarget          ( void );
    inline  xbool       CanPathToTarget         ( void );
    inline  xbool       GetCanReachGoalTarget   ( void );
            vector3     GetTargetPosWithOffset  ( guid target = 0, eOffsetPos offset = OFFSET_NONE );
            void        ForceTargetSeen         ( void )    { m_bTargetSeen = TRUE; }

    // info tracking
    inline  xbool       IsCombatReady           ( void );
    inline  void        SetCombatReady          ( xbool isReady );
    inline  xbool       HasAllies               ( void );
            xbool       HasAlliesWithin         ( f32 distSqr );
            void        UpdateHasAllies         ( void );
            f32         GetTimeSinceLastCombat  ( void );

    // Weapon and inventory related functions           
            xbool       SelectWeapon            ( void );
            void        SelectGrenade           ( void );
            void        CheckShooting           ( void );
            void        AllowShootingNow        ( void );
            xbool       HasFullClip             ( void );
            void        CreateWeaponObject      ( void );
            void        InitInventory           ( void );
    inline  xbool       GetFriendlyBlocksTarget ( void );
            xbool       WeaponReady             ( void );
            inven_item  GetWeaponItem           ( void )    { return m_WeaponItem; }

            // Editor special case functions
#ifdef X_EDITOR
    virtual void        EditorPreGame           ( void );
#endif // X_EDITOR


        // grenade functions
    inline  xtick       GetLastGrenadeThrowTime ( void );
            xbool       HasClearGrenadeThrow    ( xbool useTestLocation = FALSE, const vector3& testLocation = vector3(0.0f,0.0f,0.0f) ); //quick determination if character has correct arc to
            xbool       HasClearFragThrow       ( xbool useTestLocation = FALSE, const vector3& testLocation = vector3(0.0f,0.0f,0.0f) ); //quick determination if character has correct arc to
            xbool       HasClearGravThrow       ( xbool useTestLocation = FALSE, const vector3& testLocation = vector3(0.0f,0.0f,0.0f) ); //quick determination if character has correct arc to
            xbool       DetermineGrenadeTrajectory( xbool useTestLocation = FALSE, const vector3& testLocation = vector3(0.0f,0.0f,0.0f) );    // Creates the trajectory of the grenade
            xbool       DetermineGrenadeStraightLine( xbool useTestLocation = FALSE, const vector3& testLocation = vector3(0.0f,0.0f,0.0f) );    // Creates the trajectory of the grenade
            xbool       IsTrajectoryValid       ( void );      // Checks the path of the grenade to make sure that there are no collisions.
            void        ThrowGrenade            ( const vector3& grenadePosition );
            void        ThrowSecondaryGrenade   ( const vector3& grenadePosition );

            xbool       RequestAutofire         ( void );       // help me!
            xbool       ProvideAutofire         ( void );
            void        AutofireRequestOver     ( void );
            void        CancelAutofire          ( void );

        // firing functions
            void        UpdateFiring            ( void );        
    inline  void        SetWantsToFirePrimary   ( xbool wantsToFirePrimary );
    inline  void        SetWantsToFireSecondary ( xbool wantsToFireSecondary );
    inline  xbool       GetWantsToFirePrimary   ( void );
    inline  xbool       GetWantsToFireSecondary ( void );
            void        FireWeapon              ( xbool firingPrimary = TRUE );
            void        CinemaFireWeapon        ( );
            vector3     CalculateShotDestination( xbool& isHit, s32 accuracyModifier = 100 );
            vector3     GetHitDestination       ( void );
            vector3     GetMissDestination      ( void );
            xbool       IsPlayingIdleFiring     ( void );

            void        PlayPrimaryFiringAnimation  ( void );
            void        PlaySecondaryFiringAnimation( void );
            void        EmitMeleePain               ( const pain_event& PainEvent );
            void        GetMeleeInfo                ( vector3& StartPos, vector3& EndPos, f32& MeleeSphereRadius );
            void        InitMeleeValues         ( void );

        // reload functions
            void        UpdateReloading         ( void );
    inline  void        ReloadWeapon            ( void );
    inline  xbool       IsReloading             ( void );
            xbool       IsFiring                ( void );
            void        PlayReloadAnimation     ( void );
    inline  f32         GetShortMeleeRange      ( void );
    inline  f32         GetLongMeleeRange       ( void );
    inline  f32         GetTimeSinceLastReload  ( void );
            
        // aiming functions
    inline  void        SetWantsToAim           ( xbool wantsToAim );
    inline  xbool       GetWantsToAim           ( void );
            vector3     GetShootAtPosition      ( void );                   // what are we shooting at?

    // Cover Functions
//            cover_node* GetAndReserveNearestCoverNode( void ) ;
            guid        FindNearestValidCover   ( eCoverPreference coverPref, xbool checkCurrent = FALSE );
    inline  guid        GetStickyCoverNode      ( void );
    inline  void        SetStickCoverNode       ( guid newStickyNode );
    inline  guid        GetCurrentCover         ( void );
    inline  cover_node* GetCurrentCoverNodePtr  ( void );
    inline  xbool       GetCoverChanged         ( void );
    inline  xbool       GetIsCoverHopper        ( void );
            anim_group::handle GetCoverAnimGroupHandle ( void );
            ng_node2&   GetClosestNode          ( void );
            void        SetCurrentCover         ( guid CoverNode );
            void        UpdateCoverNode         ( void );
    inline  xbool       GetCoverIsValid         ( void );
            xbool       GetIsInCover            ( void );
            xbool       GetIsCoverSticky        ( guid coverGuid );
    virtual xbool       CoverRetreatWhenDamaged ( void )    { return TRUE; }            

    // Alarm Functions
            guid        FindNearestAlarm        ( void );
    inline  guid        GetCurrentAlarm         ( void );
            void        UpdateAlarmNode         ( void );
            xbool       IsAlarmNodeValid        ( guid testNodeGuid );

    // Turret Functions
    inline  guid        GetStickyTurret         ( void );
    
    // Goal functions
            xbool       UpdateGoal              ( f32 DeltaTime );
            void        ChangeGoal              ( void );
    inline  xbool       GetGoalCompleted        ( void );
    inline  xbool       GetGoalSucceeded        ( void );
    inline  goal_info   GetCurrentGoalInfo      ( void );
            const char* GetGoalFailedReasonName ( void ) const;
    inline eGoalFailureReasons GetGoalFailedReason ( void ) const;
            xbool       DoingMovementGoal       (void );

            void        ClearGoalData           ( void );
            void        SetIdleGoal             ( void );
            void        SetTurnToTargetGoal     ( guid target, const vector3& offset = vector3(0.0f,0.0f,0.0f), radian sightFOV = 0.0f, xbool turnExact = FALSE );
            void        SetTurnToLocationGoal   ( const vector3& location, radian sightFOV = 0.0f, xbool turnExact = FALSE );
            void        SetLookAtTargetGoal     ( guid target, const vector3& offset = vector3(0.0f,0.0f,0.0f), f32 distance = 0.0f, radian sightFOV = 0.0f, xbool lookatHead = FALSE );
            void        SetLookAtLocationGoal   ( const vector3& location, f32 distance = 0.0f, radian sightFOV = 0.0f );
            void        SetGotoTargetGoal       ( guid target, const vector3& offset = vector3(0.0f,0.0f,0.0f), loco::move_style moveStyle = loco::MOVE_STYLE_NULL, f32 arriveDist = 25.0f, xbool gotoExact = FALSE );
            void        SetGotoLocationGoal     ( const vector3& location, loco::move_style moveStyle = loco::MOVE_STYLE_NULL, f32 arriveDist = 25.0f, xbool gotoExact = FALSE );
            void        SetRetreatFromTargetGoal( guid target, const vector3& offset = vector3(0.0f,0.0f,0.0f), loco::move_style moveStyle = loco::MOVE_STYLE_NULL, f32 minDistance = 100.0f, xbool faceTargetOnRetreat = TRUE );
            void        SetRetreatFromLocationGoal( const vector3& location, loco::move_style moveStyle = loco::MOVE_STYLE_NULL, f32 minDistance = 100.0f, xbool faceTargetOnRetreat = TRUE );
            void        SetPlayAnimationGoal    ( loco::anim_type AnimType, f32 BlendTime = DEFAULT_BLEND_TIME, u32 AnimFlags = 0, f32 PlayTime = 0.0f );
            void        SetScaledPlayAnimationGoal( loco::anim_type newAnim, f32 BlendTime = DEFAULT_BLEND_TIME, u32 AnimFlags = 0, const vector3& location = vector3(0.0f,0.0f,0.0f) );
            void        SetPlayAnimationGoal    ( const char* animName , const char* animPkg = NULL, f32 BlendTime = DEFAULT_BLEND_TIME, u32 AnimFlags = 0, f32 PlayTime = 0.0f );
            void        SetScaledPlayAnimationGoal( const char* animName , const char* animPkg = NULL, f32 BlendTime = DEFAULT_BLEND_TIME, u32 AnimFlags = 0, f32 PlayTime = 0.0f, const vector3& location = vector3(0.0f,0.0f,0.0f) );
            void        SetSayDialogGoal        ( const char* dialogName, const char* animName = NULL, const char* animPkg = NULL, f32 BlendTime = DEFAULT_BLEND_TIME, u32 Flags = 0, u8 DialogFlags = 0, xbool BlockOnDialog = TRUE, xbool KillAnim = TRUE, f32 BlendOutTime = 0.25f );
            void        UpdateGoalLocation      ( const vector3& location );
            vector3     GetGoalTrueLocation     ( void );
            vector3     GetGoalTargetsLocation  ( void );
inline virtual xbool       GetLogStateChanges   ( void );
            const char* GetGoalTypeName         ( void );

            void        SetAnimRate             ( f32 animRate );
            f32         GetAnimRate             ( void );
            void        SetAnimYawDelta         ( radian yawDelta )     { m_AnimYawDelta = yawDelta; }

    // State functions
    virtual void        AdvanceState            ( f32 DeltaTime );
    virtual xbool       SetupState              ( character_state::states State, void* pContext = NULL );
            xbool       ChangeState             ( void );
            void        SetMinStateTime         ( f32 minStateTime )            { m_MinStateTime = minStateTime; }
    virtual void        SetRecoverState         ( character_state::states NewState ) ;        
            character_state::states  GetActiveState( void );
            xbool       HasState                ( character_state::states stateType );
            character_state *GetStateByType     ( character_state::states stateType );
    inline  character_state::states GetLastState( void );
            character_state::states GetCurrentStateType( void );
            xbool       IgnoreAttacks           ( void );
inline virtual xbool    AlertDetected           ( f32 ThreatLevel, const vector3& ThreatPos );
    virtual void        SetYawFacingTarget      ( radian TargetYaw, radian MaxDeltaYaw );
    virtual radian      GetCurrentFacingYaw     ( void );
            xbool       IsMeleeingPlayer        ( void );

            void        SetProjectileAttached   ( void )    { m_bProjectileAttached = TRUE; }
            xbool       GetProjectileAttached   ( void )    { return m_bProjectileAttached; }

    // Loco functions
        //animations
            f32         GetLipSyncEventStartFrame( const char* pAnimGroup, const char* pName );
    virtual xbool       PlayAnimation           ( const char* pAnimGroup, const char* pName, f32 BlendTime = DEFAULT_BLEND_TIME, u32 Flags = 0, f32 PlayTime = 0.0f ) ;
    virtual void        StopAnimation           ( void );
    virtual xbool       IsAnimAtEnd             ( void );
inline virtual void     ControlUpperBody        ( xbool bControl, f32 BlendTime = DEFAULT_BLEND_TIME );
        // lookat
    inline  eLookatModes GetLookatMode          ( void );
    inline  void        SetHeadLookat           ( guid lookatGuid, f32 lookatTime = -1.0f , f32 lookatDistance = 400.0f, radian lookatAngle = R_60 );
    inline  void        SetOverrideLookatMode   ( eLookatModes newMode );
    inline  void        SetOverrideLookatInterest( guid newInterest );
    inline  xbool       GetRootWhenIdle         ( void );


    // Misc data functions
//            void        BroadcastPlayerTurned   ( void );
            void        TurnAgainstPlayer       ( void );
            const char* GetLookatStateName      ( void );
    inline  vector3     GetInitialPosition      ( void );
    inline  void        SetInitialPosition      ( const vector3& newPosition);
    inline  guid        GetFollowTargetGuid     ( void );
    virtual void        OverridePhysics         ( xbool bOverrride );
    inline  xbool       GetAutoRagdoll          ( void );
            void        SetNotifyOnDeathGuid    ( guid NotifyOnDeath );
    inline  xbool       GetBodyFades            ( void );
    inline  void        SetCrazyFire            ( xbool crazyFire );
//    inline  xbool       GetInPlayersLOF         ( void );
//            void        UpdatePlayerLOF         ( void ); 
    inline  xbool       GetDoRunLogic           ( void );
    inline  xbool       GetStartStunAnim        ( void );
    inline  void        StunAnimStarted         ( void );
    inline  f32         GetStunTimer            ( void );
            void        EnableStun              ( f32 Duration );

    // scanner stuff
            eNPCName    NameToEnum              ( const char* pName );
            xstring     GetEnumStringNPCs       ( void );
            const char* EnumToName              ( eNPCName NPC );
            const char* GetScanIdentifier       ( void );
            xbool       WasRecentlyInCombat     ( void );
            eNPCName    GetIDFromSubtype        ( void );

    virtual void        NotifyScanBegin         ( void );
    virtual void        NotifyScanEnd           ( void );


        
protected:
    
            void        SetAnimPackage          ( const char* animPackageName );
    // naviagation functions
            xbool       CanPathTo               ( const vector3& vDestination );
            void        UpdateHasClearJumpAttack( void );
            void        UpdateHasClearLOS       ( void );
            xbool       CheckClearLOS( eOffsetPos offset );
            xbool       UpdateCanMoveLeft       ( f32 distance = 200.0f, xbool doCollisionCheck = TRUE );
            xbool       UpdateCanMoveRight      ( f32 distance = 200.0f, xbool doCollisionCheck = TRUE );
            xbool       UpdateCanMoveBack       ( f32 distance = 200.0f, xbool doCollisionCheck = TRUE );
            xbool       UpdateCanMoveForward    ( f32 distance = 200.0f, xbool doCollisionCheck = TRUE );
            void        UpdateLookahead         ( void );
            void        SwitchLookatMode        ( void );
            void        UpdateInNavMap          ( void ); 
            void        UpdatePathing           ( void );
            void        UpdateIsStuck           ( f32 DeltaTime );
            void        UpdateEscape            ( void );
            void        UpdateLocoMoveStyle     ( void );
            xbool       UpdateEscapeAttempt     ( xbool& success );
            void        AttemptToAvoidActorCollision( f32 DeltaTime );
            void        ChangeEscapeAttempt     ( void );
    virtual void        InitPathingHints        ( void );
            xbool       GetNewPath              ( const vector3& vDestination );
            // jumping
    virtual void        CalculateJumpInfo       ( void );
    virtual void        HandleJumpLogic         ( f32 DeltaTime );

            void        AdvanceAIState          ( f32 DeltaTime );
    virtual void        AdvanceLoco             ( f32 DeltaTime );
            void        UpdateAllowMotion       ( void );

            void        UpdateVoice             ( f32 DeltaTime);
            xbool       ListenForSounds         ( void );
            void        ListenForAlerts         ( void );
            void        UpdateAwarenessLevel    ( void );                    

            xbool       IsCoverNodeValid        ( guid testNodeGuid );
            // Returns TRUE if the character can see another object
            xbool       CanSeeObject            ( guid Guid, xbool bIgnoreSightCone = FALSE );
            xbool       UpdateCanShootAtTarget  ( void );
            nav_connection_slot_id GetNewRetreatPath( const vector3& vRetreatFrom, f32 DistToAchieve );
            xbool       SetupRetreatInsideConnection( const vector3& vRetreatFrom );    // Called by GetNewRetreatPath
            void        UpdateLookAt            ( void );
            void        UpdateMoveTo            ( f32 DeltaTime );
            void        UpdateCurrentConnection ( void );
            void        UpdateAimToTarget       ( void );
//virtual     void        BroadcastActorDeath     ( guid actorKiller );
//            void        BroadcastCoverRequestDialogDone();
inline virtual xbool    SetupShoulderLight      ( void );
inline virtual void     UpdateFellFromAltitude  ( void );
inline virtual void     TakeFallPain            ( void );
       virtual xbool    IgnorePain              ( const pain& Pain );
       virtual xbool    IgnoreFlinches          ( void );
       virtual xbool    IgnoreFullBodyFlinches  ( void );
       virtual actor::eHitType OverrideFlinchType( actor::eHitType hitType );


//=========================================================================
// Data
//=========================================================================

public:
#if !defined(X_RETAIL) || defined(X_QA)
    // Debug booleans
    static  xbool         s_bDebugInGame;       // Debugs on PS2,XBOX etc
    static  xbool         s_bDebugLoco;         // Loco lookat, moveat
    static  xbool         s_bDebugAI;           // AI
    static  xbool         s_bDebugPath;         // Pathfinding
    static  xbool         s_bDebugStats;        // Shows # of meshes, verts, faces, bones etc

    void    AddToStateChangeList(const char *newText);
    void    AddToPhaseChangeList(const char *newText);
    void    AddToGoalChangeList(const char *newText);
#endif

protected:
    
#if !defined(X_RETAIL) || defined(X_QA)
        char    m_StateChangeList[4][128];
        char    m_PhaseChangeList[4][128];
        char    m_GoalChangeList[4][128];
#endif

    // update target helper functions
        void        UpdateTargetData        ( void );
        void        ChoseBestTarget         ( void );
        s32         GetHitChance            ( void );
        void        CalculateSplinePoints   ( void );
        void        UpdateSpline            ( void );
virtual xbool       UseSplines              ( void ) { return TRUE; }

    //===========================================================================
    // Here are the states that belong to all characters
    //===========================================================================
        character_state*        m_pStateList ;          // List of connected states
        character_state*        m_pActiveState ;        // Currently active state
        character_trigger_state m_TriggerState;
        character_hold_state	m_HoldState;
        character_follow_state	m_FollowState;    
        character_trigger_state::TriggerData m_TriggerStateData;       

    // Navigation data and collision data
        pathing_hints       m_PathingHints;
        path_find_struct    m_PathFindStruct;
        eAvoidCharacterAttempt m_AvoidActorAttempt;
        vector3             m_SplinePointList[k_MaxSplinePoints];
        vector3             m_InitialPosition;
        vector3             m_DesiredLocation;              // this is where we want to go (it's our loco_move to when we are moving)
        vector3             m_NextPathPoint;
        vector3             m_PreviousPathPoint;
        vector3             m_JumpStartPos;
        vector3             m_JumpLandPos;
        vector3             m_JumpApexPos;
        vector3             m_LastMovingPosition;
        vector3             m_UnstuckVector;
        guid                m_CollisionIgnoreGuid;
        guid                m_LeashGuid;    
        f32                 m_AvoidAttemptTime;
        f32                 m_TimeSinceLastRetreatJig;                                            
        f32                 m_RethinkRetreatTimer;
        f32                 m_JumpVelocity;
        f32                 m_JumpTimeRemaining;
        f32                 m_TotalJumpTime;
        f32                 m_JumpTimeToApex;
        f32                 m_SinceLastGetPath;
        f32                 m_LeashDistance;
        f32                 m_TimeInEscapeAttempt;
        f32                 m_TimeStuck;
        f32                 m_TimeOutofNavMap;
        f32                 m_PreviousActorCollisionRadius;
        f32                 m_ModifiedActorCollisionRadius;
        f32                 m_CloakDecloakTimer;
        s32                 m_CurrentPathStructIndex;
        s32                 m_CurrentEscapeAttempt;
        s32                 m_EscapeAttempts;
        s32                 m_LastEscapeAttempt;
        s32                 m_PointsPerSpline;
        s32                 m_CurrentSplinePoint;
        s32                 m_Subtype;
        u32                 m_NeedsToRecalcPath:1,            // should we recalculate our path?
                            m_CurrentConnectionSlotChanged:1,
                            m_bFollowingPath:1,
                            m_PathReset:1,
                            m_InNavMap:1,
                            m_EscapeDoneMovingBack:1,
                            m_IsStuck:1,
                            m_CanMoveForward:1,
                            m_CanMoveLeft:1,
                            m_CanMoveRight:1,
                            m_CanMoveBack:1,
                            m_bHasClearJumpAttack:1,
                            m_bHasClearLOSFeet:1,
                            m_bHasClearLOSEyes:1,
                            m_bCanReachPathDest:1,
                            m_bCrazyFire:1,
                            m_bForceWeaponFire:1,
                            m_bCanReload:1,
                            m_bNeverPlayReloadAnim:1,
                            m_bWantsToDropWeapon:1,
                            m_bCanAlwaysBackpeddle:1;

        
/*static  dialog_actor_pair   s_DialogActorTypePairTable[];
static  dialog_actor_table  s_DialogActorTypeEnumTable;

typedef enum_pair<eDialogActors>   dialog_actor_pair;
typedef enum_table<eDialogActors>  dialog_actor_table;*/
static  enum_table<eDialogActors>m_DialogActorsEnum;        // Enumeration of the dialog actors..
        nav_connection_slot_id  m_GoalsConnectionSlot;      // what is the nearest connection slot to our current goals location?
        nav_connection_slot_id  m_CurrentConnectionSlot;    // what is the nearest connection slot to us?
        nav_connection_slot_id  m_GoalRetreatToConnectionSlot; // what slot are we attempting to retreat to?

    // Target, Awareness, and Sound/Sight related data
        guid                m_AllyGuids[k_MaxAllies];
        dialog_info         m_CurrentDialogInfo;
        object_affecter     m_OverrideTargetAffector; //affector for the override target.
        vector3             m_LastLocationOfInterest;
        vector3             m_LastKnownLocationOfTarget;
        vector3             m_LastSeenLocationOfTarget;
        vector3             m_LastSoundPos;         // posistion of last threatining sound.
        guid                m_TargetGuid;           // Object of interest
        guid                m_OverrideTargetGuid;   // We MUST target this guid as long as it exists.
        guid                m_AimAtGuid;            // if no targets and we get a fire event, we'll aim it at this.
        xtick               m_LastListenTime;       // The last time the character listened for sound.
        xtick               m_LastAlertTime;        // The last time the character listened for alerts.
        xtick               m_LastAlertSentTime;    // Last time we sent out an alert;
        radian              m_IdleSightFOV ;        // Sight field of view
        radian              m_AlertSightFOV ;       // Sight field of view
        f32                 m_LightSightRadius ;    // Distance that can be seen
        f32                 m_DarkSightRadius ;     // Distance that can be seen
        f32                 m_TargetSightedTimer;
        f32                 m_TimeAtAwarenessLevel;
        f32                 m_TimeSinceLastSound;   // time since we last heard a sound.
        f32                 m_SoundRange;           // How far the characters can listen;
        f32                 m_TimeSinceLastDialog;  // how long since we last played a dialog?
        f32                 m_TargetNotSeenTimer;   // Time target has not been seen
        f32                 m_NoTargetTimer;
        f32                 m_TimeSinceLastPathToTargetCheck; // how long since we tried to update to see if we can path to target?
        f32                 m_TimeSinceLastTargetUpdate;
        f32                 m_TimeWaitingForVoiceToStart;
        f32                 m_TimeSincePlayerDamagedMeLast;
        s32                 m_TimesPlayerHasDamagedMe;
        eDialogActors       m_OverrridePreferedVoiceActor;   // -1 to 3 the voice that says our dialog a,b,c,d, -1 means leave it random.
        eAwarenessLevel     m_AwarenessLevel;
        eDialogType         m_CurrentDialogType;
        u32                 m_SoundHeard:1,             // did we hear a sound?
                            m_VoiceStarted:1,           // has the voice started?
                            m_AllowDialog:1,            // does this NPC allow dialog to be played?
                            m_CanSeeTarget:1,           // Object of interest
                            m_CanShootAtTarget:1,       // do we have a straight LOS to target?
                            m_FriendlyBlocksTarget:1,
                            m_CanReachTarget:1,         // can we reach our current target?
                            m_CanReachGoalTarget:1,     // can we reach the target of our goal?
                            m_CanPathToTarget:1,        // can we path to our current target?
                            m_bTargetPlayer :1,         // Is the character targeting a player.        
                            m_CombatReady:1,            // Is this NPC ready/expecting combat?
                            m_bNeverCombatReady:1,      // If true we never become combat ready.
                            m_HasAllies:1,              // do we have friends nearby?
                            m_bTargetSeen:1,            // have we ever seen our current target?
                            m_bAllyAcquiredTarget:1,
                            m_bPlayAllClearDialog:1;
//                            m_bMovingIntoPlayersLOF:1,  // are we heading into the player's line of fire?  
//                            m_bInPlayersLOF:1;          // are we in the player's line of fire? 

    // NPC Name
        eNPCName            m_NPCName;                  // what index for the NPCs name is it?
        s32                 m_ScannerVOIndex;           // keep track of the last VO said so we can walk the list.

    // Weapon and Pain data
        inven_item          m_WeaponItem;               // The weapon this character is carrying
        inven_item          m_GrenadeItem;              // The grenade this character is carrying
        vector3             m_GrenadeDestination ;                  // Where the grenade will land
        vector3             m_GrenadeThrowStart;
        xtick               m_LastGrenadeThrowTime; // last time we threw a grenade.
        guid                m_NotifyOnDeath;        // Guid of an object to notify when this object is killed.
        radian              m_WeaponFireAngle;
        radian3             m_GrenadeTrajectory ;                   // Trajectory of the grenade
        f32                 m_SinceLastShot;        // time since last shot.
        f32                 m_ShootDelay;           // min time between shots.
        f32                 m_ShortMeleeRange;      // range of our short melee attack 
        f32                 m_LongMeleeRange;       // range of our long melee attack
        f32                 m_TimeSinceLastReload;  // how long since we reloaded last?
        f32                 m_WeaponDropPercent;    // chance we will drop our weapon
        f32                 m_AmmoDropPercent;      // chance we will drop an ammo clip
        f32                 m_GrenadeDropPercent;   // chance we will drop a grenade
        f32                 m_InventoryDropPercent; // chance we will drop other inventory
        s32                 m_Accuracy;             // how accurate are we?
        s32                 m_MovingTargetAccuracy; // how accurate when our target is moving?
        f32                 m_AccuracyDifficultyScaler; // accuracy tweak due to difficulty
        xbool               m_bPassOnGuidToCorpse;
#if !defined(X_RETAIL) || defined(X_QA)
        vector3             m_CurrentPainCenter;
        f32                 m_CurrentPainRadius;
#endif
        u32                 m_Reloading:1,
                            m_WantsToAim:1,
                            m_WantsToFirePrimary:1,
                            m_WantsToFireSecondary:1,
                            m_NextShotMustHit:1,// if true then the next fired shot WILL be a hit.
                            m_bIgnorePain:1,
                            m_bIgnoreFlinches:1,
                            m_bAutoRagdoll:1,
                            m_MeleeInited:1,
                            m_bProjectileAttached:1,
                            m_bDropWeapons:1,
                            m_bDropAmmo:1,
                            m_bDropGrenades:1,
                            m_bDropInventory:1;


    // State, Goal, and loco data
        goal_info           m_GoalInfo;             // our current goal
        eGoalFailureReasons m_FailedReason;         // why did we fail? 
        char                m_WeaponlessAnimPackage[64]; // anim package to switch to when we lose our gun.
        f32                 m_TimeInGoal;           // how long have we been processing the current goal...
        f32                 m_StateTime;            // Time in current state
        f32                 m_MinFallHeight;        // min height till we start taking damage
        f32                 m_MaxFallHeight;        // min height till we start taking damage
        f32                 m_MinStateTime;         // min time we must remain in our current state.
        radian              m_AimToTargetYaw;       // Difference between our aim and totarget angle
        radian              m_AnimYawDelta;         // this is feed as the yaw delta into the next animation we play.          
        character_state::states m_InitialState;     // Startup state of character
        character_state::states m_LastState;
        character_state::states m_DesiredState;     // the state we wish to change to.
        character_state::states m_PostLipsyncState; // go to this state when you finish your cinema lipsync.
        void *              m_StateContext;         // the conetext for this new state;
        u32                 m_IgnoreLocoInTransform:1, // annoying bool so we ignore the loco on transform in the editor.
                            m_LocoFallingOverride:1,
                            m_NewGoal:1,            // do we have a new goal
                            m_GoalCompleted:1,      // have we completed our current goal?
                            m_GoalSucceeded:1,      // were we sucessful in completing our goal?
                            m_LogStateChanges:1,
                            m_bWasPlayingFullBodyLipSync:1;    // should we log AI state and phase changes?

    // Misc data 
        bbox                m_BBox;                 // Rendering BBox
        object_affecter     m_FollowTargetAffector;
        guid                m_PendingTaskListGuid;         // the guid of our task list.       
//        guid                m_TriggerGuid;          // the guid of our triggerEX.       
        guid                m_FollowTargetGuid;
        guid                m_CurrentAlarm;         // The cover we are currently using.
        guid                m_StickyCoverNode;      // If this is set, then we will stay in cover state at this node.
        guid                m_CurrentCover;         // The cover we are currently using.
        guid                m_StickyTurret;         // If this is set, then we will use this turret.
        guid                m_ActivateOnDeath;
        guid                m_OverrideLookatInterest; // target we were told to lookat as an override. 
        guid                m_pNoBlockDialogTrigger;
        guid                m_HeadLookat;           // object for our head to lookat
        f32                 m_HeadLookatTimer;      // we will look at the object this long, negative means never turn off.
        f32                 m_OutofLookatTimer;     // this gets incremented when we are outside the lookat area... 
        f32                 m_HeadLookatDistance;   // distance we must be within in order to look at the thinghy. 
        radian              m_HeadLookatAngle;      // max angle we will look around to see the object. 

//        anim_group::handle  m_CoverAnimGroupHandle; // the animgrouphandle for our current cover.
        f32                 m_TimeActive;
        f32                 m_TimeSinceLastCoverCheck;
        f32                 m_DamageInCover;
        f32                 m_TimeSinceLastLookatSwitch;
        f32                 m_MaxDistToCover;
        f32                 m_LipSyncStartAnimFrame;
        f32                 m_StunTimer;
        s32                 m_DialogPrefixString;
        s32                 m_LogicalName;
        s32                 m_GroundMaterial ;      // Material under feet
        eLookatModes        m_LastLookatMode;           // Our current lookat mode.
        eLookatModes        m_LookatMode;           // Our current lookat mode.
        eLookatModes        m_OverrideLookatMode;   // an override mode provided by the AI.
        u32                 m_bThinking:1,          // TRUE if character is thinking
                            m_CoverChanged:1,         // set to true for one tick when our sticky cover node changes.
                            m_CoverIsValid:1,         // true if cover is valid.            
                            m_IgnoreInterest:1,       // we will ignore interest objects.
                            m_BodiesFade:1,
                            m_bCanUseAlarms:1,
                            m_bRootToPositionWhenIdle:1,
                            m_bOnlyUsesCoverAhead:1,
                            m_bCoverHopper:1,
                            m_bDoRunLogic:1,
                            m_bDoRender:1,
                            m_bIsInHeadTracking:1;

                            
    
//=========================================================================
// Debugging
//=========================================================================
#ifdef X_EDITOR
public:
    //
    //  TEXT BLOCK DEBUG OUTPUT
    //
    void    SetupDebugTextBlock         ( s32 nLines, s32 nCharsWide, xcolor Color );
    void    PrintToTextBlock            ( xcolor Color, const char* pFormatString, ... );

protected:
    vector2 m_ScreenBlockCursor;
    s32     m_iScreenBlockLine;
    s32     m_nScreenBlockMaxLines;
    s32     m_nPreviousRequestedLines;
#endif // X_EDITOR

};

//=========================================================================

inline xbool character_hold_state::IgnorePain( const pain& Pain ) 
{ 
    (void)Pain; 
    return TRUE; 
}

//=========================================================================

inline xbool character_hold_state::IgnoreAlerts( void ) 
{ 
    return TRUE; 
}    

//=========================================================================

inline xbool character_hold_state::IgnoreSight( void )
{ 
    return FALSE; 
}

//=========================================================================

inline xbool character_hold_state::IgnoreSound( void )
{ 
    return FALSE; 
}    

//=========================================================================

inline xbool character_hold_state::IgnoreAttacks( void )
{ 
    return FALSE; 
}    

//=========================================================================

inline xbool character::IsCharacter( void )               
{ 
    return TRUE; 
}

//=========================================================================

inline  nav_connection_slot_id character::GetCurrentConnection( void )
{ 
    return m_CurrentConnectionSlot; 
}

//=========================================================================

inline  xbool character::GetCanMoveLeft( )
{
    return m_CanMoveLeft;
}

//=========================================================================

inline  xbool character::GetCanMoveRight( )
{
    return m_CanMoveRight;
}

//=========================================================================

inline  xbool character::GetCanMoveBack( )
{
    return m_CanMoveBack;
}

//=========================================================================

inline  xbool character::GetCanMoveForward( )
{
    return m_CanMoveForward;
}

//===========================================================================

inline xbool character::GetIsFollowingPath( void )
{
    return m_bFollowingPath;
}

//===========================================================================

inline void character::SetIsFollowingPath( xbool followingPath )
{
    m_bFollowingPath = followingPath;
}

//=========================================================================

inline void character::ClearInitialState( void )
{ 
    m_InitialState = character_state::STATE_NULL; 
}

//=========================================================================

inline character_trigger_state::TriggerData character::GetTriggerStateData( void )
{ 
    return m_TriggerStateData; 
}

//=============================================================================

inline void character::SetPendingTaskListGuid( guid newGuid )
{
    m_PendingTaskListGuid = newGuid;
}

//=========================================================================

inline s32 character::GetVoiceID( void )
{ 
    return m_VoiceID; 
}

//=========================================================================

inline void character::SetVoiceID( s32 ID )      
{ 
    m_VoiceID = ID; 
}

//=========================================================================

inline xbool character::GetHasClearJumpAttack( void )
{
    return m_bHasClearJumpAttack;
}

//=========================================================================

inline xbool character::GetHasClearLOSFeet( void )
{
    return m_bHasClearLOSFeet;
}

//=========================================================================

inline xbool character::GetHasClearLOSEyes( void )
{
    return m_bHasClearLOSEyes;
}

//=========================================================================

inline f32 character::GetTargetNotSeenTimer( void )        
{ 
    return m_TargetNotSeenTimer; 
}

//=========================================================================

inline vector3 character::GetLastKnownLocationOfTarget( void )   
{ 
    return m_LastKnownLocationOfTarget; 
}

//=========================================================================

inline void character::SetLastKnownLocationOfTarget( const vector3& Location )   
{ 
    m_LastKnownLocationOfTarget = Location; 
}

//=========================================================================

inline vector3 character::GetLastSeenLocationOfTarget( void )    
{ 
    return m_LastSeenLocationOfTarget;  
}

//=========================================================================

inline f32 character::GetSightRadius( void )        
{ 
    return m_LightSightRadius; 
}

//=========================================================================

inline vector3 character::GetLastLocationOfInterest( void )       
{ 
    return m_LastLocationOfInterest; 
}

//=========================================================================

inline void character::SetLastLocationOfInterest( const vector3& Location ) 
{ 
    m_LastLocationOfInterest = Location; 
}

//=========================================================================

inline character::eAwarenessLevel character::GetAwarenessLevel( ) 
{ 
    return m_AwarenessLevel;
}

//=========================================================================

inline vector3 character::GetLastSoundPosition( void )        
{ 
    return m_LastSoundPos;        
}

//=========================================================================

inline f32 character::GetTimeSinceLastSound( void )        
{ 
    return m_TimeSinceLastSound;  
}

//=========================================================================

inline void character::ClearSoundHeard( void )            
{ 
    m_SoundHeard = FALSE; 
}   

//=========================================================================

inline xbool character::GetSoundHeard( void )            
{ 
    return m_SoundHeard; 
}           

//=========================================================================

inline void character::Listen( conversation_packet& rPacket )
{ 
    (void)rPacket ; 
}

//=========================================================================

inline
void character::SetNotifyOnDeathGuid( guid NotifyOnDeath )
{
    m_NotifyOnDeath = NotifyOnDeath;
}

//=========================================================================

inline guid character::GetTargetGuid( void )            
{ 
    return m_TargetGuid; 
}

//=========================================================================

inline xbool character::CanSeeTarget( void )        
{ 
    return m_CanSeeTarget; 
}           

//=========================================================================

inline xbool character::CanShootAtTarget( void )        
{ 
    return m_CanShootAtTarget; 
}           

//=========================================================================

inline radian character::GetAimToTargetYaw( void )        
{ 
    return m_AimToTargetYaw; 
}                            

//=========================================================================

inline xbool character::CanReachTarget( void )
{ 
    return m_CanReachTarget; 
}            

//=========================================================================

inline xbool character::GetCanReachGoalTarget( void )
{ 
    return m_CanReachGoalTarget; 
}            

//=========================================================================

inline xbool character::CanPathToTarget( void )
{ 
    return m_CanPathToTarget; 
}            

//=========================================================================

inline xtick character::GetLastGrenadeThrowTime( void )                    
{ 
    return m_LastGrenadeThrowTime; 
}            

//=========================================================================

inline void character::SetWantsToFirePrimary( xbool wantsToFirePrimary )    
{ 
    m_WantsToFirePrimary = wantsToFirePrimary;    
}

//=========================================================================

inline void character::SetWantsToFireSecondary( xbool wantsToFireSecondary )  
{ 
    m_WantsToFireSecondary = wantsToFireSecondary;
}

//=========================================================================

inline xbool character::GetWantsToFirePrimary( void )                    
{ 
    return m_WantsToFirePrimary;  
}

//=========================================================================

inline xbool character::GetWantsToFireSecondary( void )                    
{ 
    return m_WantsToFireSecondary;
}

//=========================================================================

inline void character::ReloadWeapon( void )                    
{ 
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon )
    {    
        // do we play the reload anim? if not just do it now.
        if( m_bNeverPlayReloadAnim || 
            IsPlayingFullBodyCinema() )
        {        
            pWeapon->Reload( new_weapon::AMMO_PRIMARY );
            m_Reloading = FALSE;
            m_TimeSinceLastReload = 0.0f;
            AutofireRequestOver();
        }
        else
        {        
            m_Reloading = TRUE; 
            PlayReloadAnimation();
            if( GetAwarenessLevel() >= AWARENESS_TARGET_SPOTTED &&
                HasAllies() )
            {        
                if( RequestAutofire() )
                {            
                    PlayDialog(DIALOG_RELOAD);
                }
            }
        }
    }
}

//=========================================================================

inline f32 character::GetTimeSinceLastReload( void )                    
{ 
    return m_TimeSinceLastReload;
}

//=========================================================================

inline xbool character::IsReloading( void )                    
{ 
    return m_Reloading;
}

//=========================================================================

inline f32 character::GetShortMeleeRange( void )
{
    return m_ShortMeleeRange;
}

//=========================================================================

inline f32 character::GetLongMeleeRange( void )
{
    return m_LongMeleeRange;
}

//=========================================================================

inline void character::SetWantsToAim( xbool wantsToAim )        
{ 
    m_WantsToAim = wantsToAim;    
}

//=========================================================================

inline xbool character::GetWantsToAim( void )                    
{ 
    return m_WantsToAim;          
}

//=========================================================================

inline  xbool character::GetInNavMap( void )
{
    return m_InNavMap;
}

//=========================================================================

inline  f32 character::GetTimeOutofNavMap( void )
{
    return m_TimeOutofNavMap;
}

//=========================================================================

inline  void character::SetCollisionIgnoreGuid( guid ignoreGuid )
{
    m_CollisionIgnoreGuid = ignoreGuid;
}

//=========================================================================

inline xbool character::GetCoverIsValid( void )
{ 
    return m_CoverIsValid; 
}

//=========================================================================

inline guid character::GetStickyCoverNode( void )
{ 
    return m_StickyCoverNode; 
}

//=========================================================================

inline xbool character::GetCoverChanged( void )
{ 
    return m_CoverChanged; 
}

//=========================================================================

inline xbool character::GetIsCoverHopper( void )
{ 
    return m_bCoverHopper; 
}

//=========================================================================

/*inline anim_group::handle character::GetCoverAnimGroupHandle( void )
{
    return m_CoverAnimGroupHandle;
}*/

//=========================================================================

inline guid character::GetCurrentCover( void )
{ 
    return m_CurrentCover; 
}

//=========================================================================

inline
cover_node* character::GetCurrentCoverNodePtr( void )
{
    cover_node* pCoverNode = (cover_node*)g_ObjMgr.GetObjectByGuid( m_CurrentCover );
    return pCoverNode;
}

//=========================================================================

inline guid character::GetCurrentAlarm( void )
{ 
    return m_CurrentAlarm; 
}

//=========================================================================

inline guid character::GetStickyTurret( void )
{ 
    return m_StickyTurret; 
}

//=========================================================================

inline void character::SetStickCoverNode( guid newStickyNode )  
{ 
    if( m_StickyCoverNode != newStickyNode )
    {    
        m_StickyCoverNode = newStickyNode; 
    }
}            

//=========================================================================

inline xbool character::GetGoalCompleted( void )                    
{ 
    return m_GoalCompleted;   
}    

//=========================================================================

inline xbool character::GetGoalSucceeded( void )                    
{ 
    return m_GoalSucceeded;   
}    

//=========================================================================

inline character::goal_info character::GetCurrentGoalInfo( void )                    
{ 
    return m_GoalInfo;   
}    

//=========================================================================

inline xbool character::GetLogStateChanges( void )    
{ 
    return m_LogStateChanges; 
}

//=========================================================================

inline xbool character::AlertDetected( f32 ThreatLevel, const vector3& ThreatPos ) 
{ 
    (void) ThreatLevel; 
    (void) ThreatPos; 
    return FALSE; 
}

//=========================================================================

inline void character::ControlUpperBody( xbool bControl, f32 BlendTime ) 
{ 
    (void)bControl; 
    (void)BlendTime; 
}

//=========================================================================

inline character::eLookatModes character::GetLookatMode( void ) 
{ 
    return m_LookatMode; 
}

//=========================================================================

inline void character::SetOverrideLookatMode( eLookatModes newMode )    
{ 
    m_OverrideLookatMode = newMode; 
}

//=========================================================================

inline void character::SetHeadLookat( guid lookatGuid, f32 lookatTime, f32 lookatDistance , radian lookatAngle )
{
    m_HeadLookat = lookatGuid;
    m_HeadLookatTimer = lookatTime;
    m_HeadLookatDistance = lookatDistance;
    m_HeadLookatAngle = lookatAngle;
}

//=========================================================================

inline void character::SetOverrideLookatInterest( guid newInterest )        
{ 
    m_OverrideLookatInterest = newInterest; 
}            

//=========================================================================

inline xbool character::GetRootWhenIdle( void )
{
    return m_bRootToPositionWhenIdle;
}

//=========================================================================

inline vector3 character::GetInitialPosition( void )
{ 
    return m_InitialPosition; 
}

//=========================================================================

inline void character::SetInitialPosition( const vector3& newPosition)
{ 
    m_InitialPosition = newPosition; 
}

//=========================================================================

inline guid character::GetFollowTargetGuid( void )    
{ 
    return m_FollowTargetGuid;
}            

//=========================================================================

inline xbool character::GetAutoRagdoll( void )
{ 
    return m_bAutoRagdoll; 
}

//=========================================================================

inline xbool character::SetupShoulderLight( void )    
{ 
    return FALSE; 
}

//=========================================================================

inline void character::UpdateFellFromAltitude( void )    
{ ; /*Do nothing for now*/ }

//=========================================================================

inline void character::TakeFallPain( void )    
{ ; /*Do nothing for now*/ }

//=========================================================================

inline  
f32 character::GetLeashDistance( void ) const
{
    return m_LeashDistance;
}

//=========================================================================

inline 
guid character::GetLeashGuid( void ) const
{
    return m_LeashGuid;
}

//=========================================================================

inline 
xbool character::GetFriendlyBlocksTarget( void )
{
    return m_FriendlyBlocksTarget;
}

//=========================================================================

inline 
void character::SetCombatReady( xbool isReady ) 
{
    m_CombatReady = isReady;
    if( m_bNeverCombatReady )
    {
        m_CombatReady = FALSE;
    }
}

//=========================================================================

inline 
xbool character::IsCombatReady( void ) 
{
    return m_CombatReady;
}

//=========================================================================

inline 
xbool character::HasAllies( void ) 
{
    return m_HasAllies;
}

//=========================================================================

inline 
character::eGoalFailureReasons character::GetGoalFailedReason ( void ) const
{
    return m_FailedReason;
}

//=========================================================================

inline xbool character::GetBodyFades( ) 
{ 
    return m_BodiesFade; 
}

//=========================================================================

inline void character::SetCrazyFire( xbool crazyFire )
{
    m_bCrazyFire = crazyFire;
}

//=========================================================================

/*inline xbool character::GetInPlayersLOF( )
{
    return m_bInPlayersLOF;
}*/

//=========================================================================

inline xbool character::GetDoRunLogic( )
{
    return m_bDoRunLogic;
}

//=========================================================================

inline  character_state::states character::GetLastState( void )
{
    return m_LastState;
}

//=========================================================================

inline void character::SetIgnorePain( xbool bShouldIgnorePain )
{
    m_bIgnorePain = bShouldIgnorePain;
}

//=========================================================================

inline xbool character::GetStartStunAnim( void )
{
    return ( m_StunTimer > 0.0f );
}

//=========================================================================

inline void character::StunAnimStarted( void )
{
    m_StunTimer = 0.0f;
}

//=========================================================================

inline f32 character::GetStunTimer( void )
{
    return m_StunTimer;
}

//=========================================================================

#endif//__CHARACTER_HPP__
