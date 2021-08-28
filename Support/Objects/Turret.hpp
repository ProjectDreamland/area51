//==============================================================================
//
//  Turret.hpp
//
//==============================================================================

#ifndef TURRET_HPP
#define TURRET_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Objects\PlaySurface.hpp"
#include "Animation\AnimPlayer.hpp"
#include "Characters\Factions.hpp"
#include "Objects\Event.hpp"
#include "Objects\Actor\Actor.hpp"

//==============================================================================
//  NOTES
//==============================================================================
/*

  Supported animations

  * INACTIVE_IDLE
    ENGAGED_IDLE
    ACTIVE_IDLE
  
    ALERT

    ACTIVATE
    DEACTIVATE

    RAMP_UP
    RAMP_DOWN
  * SHOOT
    RELOAD

  * PITCH
  * YAW

  Anims marked with * are required for the turret to be functional.


*/
//==============================================================================
//  TYPES
//==============================================================================

class turret : public play_surface
{
public:

    CREATE_RTTI( turret, play_surface, object )


//------------------------------------------------------------------------------
//  Public Types

public:

    // Turret types
    enum type
    {
        TYPE_BULLET_LIGHT,
        TYPE_BULLET_MEDIUM,
        TYPE_BULLET_HEAVY,
    };

    enum state
    {
        STATE_UNDEFINED,
        STATE_IDLE,     
        STATE_PREACTIVATING,
        STATE_ACTIVATING,       
        STATE_AIMING,        
        STATE_RAMP_UP,
        STATE_FIRING,
        STATE_RAMP_DOWN,
        STATE_RELOADING,
        STATE_COOLING,
        STATE_UNAIMING,
        STATE_PREDEACTIVATING,
        STATE_DEACTIVATING,
        STATE_DESTROY_BEGIN,
        STATE_DESTROYED,
    };


//------------------------------------------------------------------------------
//  Public Functions

public:

                turret          ( void );
               ~turret          ( void );
    
    virtual void                OnEnumProp      ( prop_enum&    List );
    virtual xbool               OnProperty      ( prop_query&   I    );

    virtual void                OnMove          ( const vector3& NewPos   );      
    virtual void                OnTransform     ( const matrix4& L2W      );
    virtual void                OnAdvanceLogic  ( f32 DeltaTime );
    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );
    virtual bbox                GetLocalBBox    ( void ) const;
    virtual void                OnPain          ( const pain& Pain ) ;
    inline  xbool               IsFriendlyFaction( factions Faction );
    inline  factions            GetFaction      ( void );
    inline  void                SetFaction      ( factions NewFaction );

    virtual const char*         GetLogicalName  ( void );


    virtual void                GetBoneL2W      ( s32 iBone, matrix4& L2W );
            type                GetKind         ( void ) const;

            const char*         GetCurrentStateName( void );
    inline  state               GetCurrentState ( void );
            vector3             GetStandBonePosition( void );

            void                GetSensorInfo   ( vector3& Pos, radian3& Rot );
    const   char*               GetAnimGroupName( object::type characterType );
            guid                GetEntryMarker  ( void );
            void                SetGunner       ( guid newGunner );
    inline  guid                GetGunner       ( void );
    inline  void                SetController   ( guid newController );
    inline  guid                GetController   ( void );
    inline  xbool               IsIndestructable( void );
    inline  xbool               IsDestroyed     ( void );

            void                SetObjectiveGuid( guid ObjectiveGuid );
            xbool               OfferNewObjectiveGuid( guid NewGuid );            

    virtual void                OnPolyCacheGather( void );

            void                SetRadiusMultiplier( f32 Mult );
            f32                 GetRadiusMultiplier( void );
    
    virtual void                SetTargetGuid   ( guid Guid );

public: // ORB related
            xbool               OrbEnter        ( guid Orb );            
            xbool               IsReserved      ( void );
            xbool               CanBeReservedByMe( guid Orb );
            xbool               Reserve         ( guid Orb );
            xbool               Unreserve       ( guid Orb );
            xbool               RequiresOrb     ( void );           // TRUE if the "Orb Powered" property is set

public:
    void                Hide                    ( void ) { m_IsHidden = TRUE; };
    void                Unhide                  ( void ) { m_IsHidden = FALSE; };
    void                GetMannedBoundaryGuids  ( guid& Left, guid& Right, guid& Upper, guid& Lower ) { Left = m_LeftBoundary; Right = m_RightBoundary; Upper = m_UpperBoundary; Lower = m_LowerBoundary; }

protected:

    enum tracking_status
    {
        TRACK_NO_TARGET     = 0,    // No target
        TRACK_AIMING,               // In process of aiming, can't fire yet
        TRACK_OUTSIDE_FOF,          // Target is outside field of fire
        TRACK_LOCKED,               // Locked on target, ready to fire
    };


            const char*         GetStateName    ( state State );

    virtual void                OnImport        ( text_in& TIn );
    virtual void                OnRender        ( void );
    virtual void                OnRenderTransparent ( void );

#ifndef X_RETAIL
    virtual void                OnDebugRender   ( void );
    virtual void                OnColRender     ( xbool bRenderHigh );
#endif // X_RETAIL

    virtual void                OnColCheck      ( void );
    virtual const matrix4*      GetBoneL2Ws     ( void ) ;
    
    virtual void                OnEvent         ( const event& Event );

    virtual void                OnEnterState    ( state NewState );
    
    virtual xbool               CanSenseTarget  ( void );               // Is target within sense radius
    virtual xbool               CanFireAtTarget ( void );               // Is target within fire radius
    virtual xbool               IsTargetInRange ( void );               // Is target within either radii
    
    virtual void                Fire            ( void );

    virtual vector3             GetTargetPos    ( void );

    virtual tracking_status     TrackTarget     ( f32 DeltaTime );

            void                UpdateTrackControllers  ( void );

            void                UpdateAnimList  ( void );

    virtual void                TakeDamage      ( const pain& Pain );

    virtual void                EnumAttachPoints      ( xstring& String   ) const;
    virtual s32                 GetAttachPointIDByName( const char* pName ) const;
    virtual xstring             GetAttachPointNameByID( s32 iAttachPt     ) const;
    virtual xbool               GetAttachPointData    ( s32      iAttachPt,
                                                        matrix4& L2W,
                                                        u32      Flags = 0 );
    virtual void                OnAttachedMove        ( s32             iAttachPt,
                                                        const matrix4&  L2W );

//------------------------------------------------------------------------------
//  Private Types

protected:

    
//------------------------------------------------------------------------------
//  Private Functions

protected:
    //
    //  Track Target            Handle tracking the current target and aimin the turret
    //
    //  LookForTarget           Scans for actors
    //
    //  UpdateAim               Updates the Aim to try and come to bear on specified P/Y
    //
    //
    //

    void        TrackTarget         ( const object* pObject, radian& Pitch, radian& Yaw );

    xbool       LookForTarget       ( void );
    void        UpdateAim           ( f32 DeltaTime, radian Pitch, radian Yaw );
    xbool       CheckLOS            ( const vector3& Pt,                                // This CheckLOS is NOT buffered
                                      u32 Attr = object::ATTR_BLOCKS_CHARACTER_LOS ); 
    xbool       CheckLOS            ( guid Guid,                                        // This CheckLOS IS buffered. It will only check 
                                      u32 Attr = object::ATTR_BLOCKS_CHARACTER_LOS );   // every second, if the Guid is the same as the
                                                                                        // last one passed in.
    f32         GetEffectiveRadiusSense( void );
    f32         GetEffectiveRadiusFire ( void );

    void        LimitPitchYaw       ( radian& Pitch, radian& Yaw );
    xbool       IsAimedAt           ( radian Pitch, radian Yaw );    
    void        Fire                ( f32 DeltaTime );
    xbool       IsTargetValid       ( void );
    vector3     GetObjectAimAt      ( guid Guid,                    // Gets position to aim at for guid
                                      actor::eOffsetPos = 
                                           actor::OFFSET_EYES ); 

    xbool       GetTargetSightYaw   ( guid Guid, radian& Yaw );     // Gets facing direction of target object
    f32         DistToObjectAimAt   ( guid Guid );                  // Dist from sensor to object aim-at pt
    xbool       CalculateLinearAimDirection( const vector3& TargetPos,
                                             const vector3& TargetVel,
                                             const vector3& SourcePos,
                                             const vector3& SourceVel,
                                             f32 VelInheritFactor,
                                             f32 LinearSpeed,
                                             f32 LifetimeS,
                                             vector3& AimDirection,
                                             vector3& AimPosition       );
    f32         GetAimScoreForPoint ( const vector3& Pt );
    
    void        ActivateObject      ( guid Guid );

    xbool       IsPowered           ( void );
    void        LaunchOrb           ( void );           // If an orb is inside, launch it

    void        StopAimingSound     ( void );

virtual anim_group::handle* GetAnimGroupHandlePtr ( void ) { return &m_hAnimGroup; }

protected:
    virtual void                TryToFireAtTarget( void );

//------------------------------------------------------------------------------
//  Private Data

protected:

    anim_group::handle          m_hAnimGroup;
    rhandle<char>               m_hAudioPackage;
    simple_anim_player          m_AnimPlayer;
    anim_track_controller       m_TrackController[2];
    s32                         m_ProjectileTemplateID;
    s32                         m_AimingSoundID;
    s32                         m_StopAimingSoundID;
    voice_id                    m_AimingSoundVoice;

    s32             m_LogicalName;
    
    factions        m_Faction ;             //  which faction this object belongs to
    u32             m_FriendFaction;

    tracking_status m_LastAimedTrackingStatus;

    state           m_State;
    state           m_PreChangeState;
    guid            m_TargetGuid;
    s32             m_ShotCount;
    f32             m_ActionTimer;
    f32             m_ScanTimer;
    type            m_Type;
    radian          m_Pitch;
    radian          m_Yaw;   
    vector3         m_LocalAimDir;
    vector3         m_LocalTargetDir;
    f32             m_Health;
    f32             m_MaxHealth;
    s32             m_LastAnimPainID;
    vector3         m_TargetVel;
    vector3         m_TargetPos;
    s32             m_IdleCycle;

    guid            m_ActivateOnDestruction;
    guid            m_ActivateOnActivation;
    guid            m_ActivateOnDeactivation;
    guid            m_ActivateOnTargetLoss;
    guid            m_ActivateOnTargetAcquisition;
    guid            m_ActivateOnFire;
    guid            m_ActivateOnReload;
    guid            m_ActivateOnScan;

    guid            m_TrackingGuid;         // If valid, turret will track this (not firing)
                                            // Whenever m_TargetGuid == NULL
    guid            m_ObjectiveGuid;        // If valid, turret will track and attack this
                                            // until it is destroyed.

    f32             m_HitPercentage;
    
    slot_id         m_ObjSlotForLastLOSCheck;   // Slot of object last passed to CheckLOS
    f32             m_LOSCheckTimer;    


    f32             m_RadiusSense;  
    f32             m_RadiusFire;
    f32             m_RadiusMultiplier;
    radian          m_FiringConeFOV;        // FOV of cone around aim dir.
    f32             m_TimeBetweenScans;     // Seconds between checking for target attempts
    f32             m_TimeBetweenShots;     // Seconds between shots in a volley
    s32             m_nShotsBetweenReloads; // # of shots between reloading
    s32             m_nMinShots;            // Minimum # of shots at a time
    f32             m_TimeCool;             // Seconds waited with no target before deactivating
    f32             m_Velocity;
    radian          m_PitchSpeed;           // Speed for pitching the turret 
    radian          m_YawSpeed;             // Speed for yawing the turret
       
    radian          m_RestPitch;
    radian          m_RestYaw;

    radian          m_YawRightLimit;
    radian          m_YawLeftLimit;
    radian          m_PitchUpLimit;
    radian          m_PitchDownLimit;

    // Orb
    guid            m_ReservedBy;               //  Guid of orb 
    s32             m_OrbTemplateID;            //  ID of string relating to orb template
    vector3         m_LaunchDelta;              //  Offset from bbox root to where the orb should launch to
    

    // Additional audio
    f32             m_TimeSinceLastAimCorrection;   // Seconds since last time pitch/yaw was modified
    f32             m_IdleTimeUntilStopAim;         // How long to wait until the "stop aiming" sound is played

    // Caps
    u32             m_bIsFunctional:1,          // Has minimum set of animations
                    m_bHasAlert:1,              // Has ALERT
                    m_bHasActiveIdle:1,         // Has IDLE_ACTIVE_TRACKING
                    m_bHasEngagedIdle:1,        // Has IDLE_ACTIVE_COMBAT
                    m_bHasActivate:1,           // Has ACTIVATE
                    m_bHasDeactivate:1,         // Has DEACTIVATE
                    m_bHasReload:1,             // Has RELOAD
                    m_bHasRampUp:1,             // Has RAMP_UP
                    m_bHasRampDown:1,           // Has RAMP_DOWN
                    m_bHasActiveDeath:1,        // Has ACTIVE_DEATH
                    m_bHasInactiveDeath:1,      // Has INACTIVE_DEATH
                    m_bHasDestroyed:1,          // Has DESTROYED
                    m_bYawLimited:1,            // Yaw limits do not sum to 360deg
                    m_bPitchLimited:1,          // Pitch limits do not sum to 360deg
                    m_bIndestructable:1,        // Indestructible 100% of the time
                    m_bTurretActive:1,          // Indicates turret active/inactive state
                    // 16 bits
                    m_bLastLOSPassed:1,         // 
                    m_bHasFire:1,               // Has FIRE
                    m_bPlayIdleCompletely:1,    // 
                    m_bIgnoresDamageable:1,     // Ignores damageable objects
                    m_bRequiesOrbPower:1,       // Turret needs an orb inside to be powered
                    m_bOrbInside:1,             // An orb is inside
                    m_bInactiveIndestructable:1,// Indestructible only when it is in the inactive state
                    m_bDestroySelfWhenDead:1,   // When the turret hits the death state, destroy it.
                    m_bAimStartLoopPlaying:1,   // Is the aiming sound currently being played

                    m_IsHidden:1;               // Turret is hidden

#ifdef X_DEBUG
    vector3         m_LastFireBarrelPos;
    vector3         m_LastFireTargetPos;
    vector3         m_LastFireTargetVel;
#endif
    //char            m_GreyAnimPackageFileName[128];
    //char            m_SoldierAnimPackageFileName[128];
    //char            m_CivilianAnimPackageFileName[128];

    anim_group::handle          m_hGreyAnimGroup;
    anim_group::handle          m_hSoldierAnimGroup;
    anim_group::handle          m_hCivilianAnimGroup;

    s32             m_EntryBone;           
    s32             m_StandBone;           
    guid            m_EntryMarker;
    guid            m_Controller;
    guid            m_Gunner;



    //
    // Manned turret stuff (manned by a player)
    //

    // guids of manned rotation limit objects
    guid            m_LeftBoundary;
    guid            m_RightBoundary;
    guid            m_UpperBoundary;
    guid            m_LowerBoundary;
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

object*     TurretCreator( void );


inline guid turret::GetGunner()
{
    return m_Gunner;
}

inline xbool turret::IsIndestructable( void )
{
    return m_bIndestructable;
}

inline xbool turret::IsDestroyed( void )
{
    return ( m_Health <= 0 || m_State == STATE_DESTROYED);
}

inline guid turret::GetController()
{
    return m_Controller;
}

inline void turret::SetController( guid newController )
{
    m_Controller = newController;
}

inline turret::state turret::GetCurrentState()
{
    return m_State;
}

inline
xbool turret::IsFriendlyFaction( factions Faction )
{
    if ( Faction == FACTION_NOT_SET)
        return FALSE;

    return( (m_FriendFaction & Faction) ||
            (Faction = FACTION_NEUTRAL) || 
            (m_FriendFaction = FACTION_NEUTRAL) );
}

inline
factions turret::GetFaction( void )
{
    ASSERT(m_Faction!=FACTION_NOT_SET);
    return m_Faction;
}

inline
void turret::SetFaction( factions NewFaction )
{
    m_Faction = NewFaction;
}

inline
xbool turret::RequiresOrb( void )
{
    return m_bRequiesOrbPower;
}
//==============================================================================
#endif // TURRET_HPP
//==============================================================================
