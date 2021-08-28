//==============================================================================
//
//  AlienOrbSpawner.hpp
//
//==============================================================================
#ifndef __ALIEN_ORB_SPAWNER_HPP__
#define __ALIEN_ORB_SPAWNER_HPP__

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Objects\Turret.hpp"
#include "Animation\AnimPlayer.hpp"
#include "ZoneMgr\ZoneMgr.hpp"
#include "CollisionMgr\CollisionMgr.hpp"

//==============================================================================
//  NOTES
//==============================================================================

class alien_orb_spawner : public turret
{
public:
    CREATE_RTTI( alien_orb_spawner, turret, object )

        alien_orb_spawner();
    ~alien_orb_spawner();

    enum
    {
        MAX_ACTIVE_ORBS     = 8,
    };

    enum aosstate
    {
        AOS_STATE_IDLE,                 // Idle
        AOS_STATE_DEFENSIVE,            // Hide
        AOS_STATE_OFFENSIVE,            // Charging the player
        AOS_STATE_FIRING,               // Hold still and detonate
        AOS_STATE_SCRIPTED_MOVEMENT,    // Forced Destination is set
    };

public:

    virtual void                OnEnumProp      ( prop_enum&    List );
    virtual xbool               OnProperty      ( prop_query&   I    );
    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    virtual void                OnAdvanceLogic  ( f32 DeltaTime );
    virtual void                SetTargetGuid   ( guid Guid );

#ifdef X_EDITOR
    virtual s32                 OnValidateProperties( xstring& ErrorMsg );
#endif

#ifndef X_RETAIL
    virtual void                OnDebugRender   ( void );
#endif // X_RETAIL

    virtual void                OnPain          ( const pain& Pain );

    virtual turret::tracking_status TrackTarget( f32 DeltaTime );

protected:
    virtual xbool               LaunchOrb       ( void );
    virtual xbool               GetLaunchPoint  ( vector3& Pos, vector3& Dir, radian3& Rot );
    virtual void                OnEnterState    ( state NewState );
    virtual void                StartAiming     ( void );
    virtual void                StopAiming      ( void );
    virtual void                TryToFireAtTarget( void );
    
            void                DeathSpawnOrbs  ( void );
            void                FindCoverFromTarget( guid InvalidCover );
            void                UpdateCover     ( guid InvalidCover );
            void                Shout           ( guid About );
            void                AlertTo         ( guid Who );
            void                HandleMove      ( const vector3& DesiredPos, f32 DeltaTime );
            void                HandleFiring    ( f32 DeltaTime );
            void                HandleOffensive ( f32 DeltaTime );
            void                EnterAOSState   ( aosstate State );            
            void                TransmitPain    ( const bbox& Box );
            void                MoveDependants  ( void );
        
            xbool               IsInMyZone      ( const object* pObj ) const;
            xbool               IsInMyZone      ( const guid&   Guid ) const;

            xbool               AreValidTargets( void );

protected:
    f32                 m_TimeInAOSState;
    radian              m_SightConeAngle;
    s32                 m_nOrbsInPool;
    s32                 m_nMaxOrbsActive;
    f32                 m_OrbSpawnDelay;
    f32                 m_TimeSinceLastOrbLaunch;
    f32                 m_TimeSinceLastSecurityUpdate;
    guid                m_OrbSpawnObj;
    s32                 m_nActiveOrbs;
    s32                 m_nOrbsToSpawnOnDeath;
    guid                m_ActiveOrbs[ MAX_ACTIVE_ORBS ];
    vector3             m_LastTargetLockDir;
    s32                 m_OrbTemplateID;    
    aosstate            m_AOSState;
    guid                m_LastTarget;
    guid                m_PatrolTarget;
    guid                m_ForcedDestination;                // Spawner MUST go here no matter what
    guid                m_ActivateWhenEmpty;                // Activate when out of orbs

    f32                 m_MinVelocity;
    f32                 m_MaxVelocity;    
    guid                m_CoverObject;
    guid                m_CoverGroup;                       // Group of cover nodes the spawner can use
    vector3             m_CoverPosition;        
    vector3             m_Velocity;
    f32                 m_CurT;
    radian3             m_SteeringSpeed;   
    f32                 m_MaxCoverDist;
    f32                 m_MinCoverDist;
    f32                 m_TimeBetweenCoverChanges;

    guid                m_SpawnTubeLimitVolume;
    guid                m_TurretLimitVolume;
    guid                m_HostGroup;
    u8                  m_SpawnTubeProb;            // 0-100 probability of choosing spawntube over turret

    vector3             m_LastIdlePos;

    f32                 m_TimeSinceLastShout;

    f32                 m_TimeSinceTargetLost;    

    rhandle<char>       m_hSpotterAudioPackage;
    s32                 m_AimStartLoopSoundID;
    s32                 m_AimStopSoundID;
    voice_id            m_AimStartLoopVoiceID;
    
    // offense
    f32                 m_TimeSinceLastAttack;
    guid                m_AttackGuid;
    f32                 m_TimeBetweenAttackChecks;    
    f32                 m_AttackProb;
    f32                 m_EmptyAttackProb;  
    f32                 m_AttackDamage;
    f32                 m_AttackForce;
    f32                 m_AttackMaxTime;
    f32                 m_AttackDistance;
    voice_id            m_ChargeupVoiceID;
    f32                 m_AttackCountdownTimer;
    f32                 m_TimeToStandStillWhileFiring;

    // Temp stuff until firing is working via animation
    guid                m_AttackChargeParticleGuid;
    guid                m_AttackParticleGuid;

    //END TEMP


    u8                  m_bTrackingLocked:1,
                        m_bAOSStateLocked:1;
};


#endif // __ALIEN_SPOTTER_HPP__
