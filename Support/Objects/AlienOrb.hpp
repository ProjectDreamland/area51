//==============================================================================
//
//  AlienOrb.hpp
//
//==============================================================================
#ifndef __ALIEN_ORB_HPP__
#define __ALIEN_ORB_HPP__

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Objects\AnimSurface.hpp"
#include "Animation\AnimPlayer.hpp"
#include "ZoneMgr\ZoneMgr.hpp"
#include "CollisionMgr\CollisionMgr.hpp"

//==============================================================================
//  NOTES
//==============================================================================
class alien_orb : public anim_surface
{
public:
    CREATE_RTTI( alien_orb, anim_surface, object )


public:
    enum state
    {
        STATE_UNKNOWN,
        STATE_LAUNCH,
        STATE_LOOK_FOR_TARGET,
        STATE_MOVE_TO_TARGET,
        STATE_AT_TARGET,        
        STATE_DIEING,    
        STATE_SCRIPTED_MOTION,
    };

public:
    alien_orb();
    ~alien_orb();


    virtual void                OnEnumProp      ( prop_enum&    List );
    virtual xbool               OnProperty      ( prop_query&   I    );

    virtual void                OnAdvanceLogic  ( f32 DeltaTime );
    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    virtual void                OnTransform     ( const matrix4& L2W      );

    virtual void                OnRender        ( void );

#ifndef X_RETAIL
#ifdef TARGET_EDITOR
    virtual void                OnDebugRender   ( void );
#endif
#endif // X_RETAIL

    virtual void                OnColCheck      ( void );
    virtual void                OnPain          ( const pain& Pain );  

public:
            void                Launch          ( const vector3& InitialPos,
                                                  const vector3& DestPos );

            void                DeactivateOrb   ( void );
            void                ActivateOrb     ( void );

            void                SetSpawner      ( guid Spawner );

            void                SetTurretLimit  ( guid Volume );
            void                SetSpawnTubeLimit( guid Volume );
            void                SetSpawnTubeProb( u8 Prob );
            void                SetHostGroup    ( guid Group );
    inline  void                SetSearchTime   ( f32 SearchTime );

    static  guid                SelectTargetInVolume    ( object::type Type, const bbox& Volume, guid Guid, guid HostGroup );

            void                KillOrb         ( void );

            void                SetOrbitRadius      ( f32 OrbitRadius );
            void                SetOrbitOffset      ( f32 OrbitOffset );
            xbool               HasScriptedPosition ( void );
            void                SetScriptedPosition ( guid NewScriptedPosition );
#ifdef X_EDITOR
    virtual s32                 OnValidateProperties( xstring& ErrorMsg );
#endif

protected:

            void                SwitchState             ( state NewState );
            f32                 GetDistToTarget         ( void );
            xbool               IsTargetValid           ( void );
            xbool               FindTarget              ( void );
            void                MoveToTarget            ( f32 DeltaTime );
            void                LaunchToTarget          ( f32 DeltaTime );
            void                DoCollisionHit          ( const collision_mgr::collision& C );
            void                SetDestroyed            ( void );
            void                SetDeath                ( void );
            void                HandleCommonDeath       ( void );
            const char*         GetStateName            ( void );
            const char*         GetStateName            ( state State );
            void                HandleRenderableStates  ( void );
            void                StartMoving             ( void );
            void                StopMoving              ( void );
            xbool               EnterTargetObject       ( void );
            xbool               ReserveTarget           ( void );
            void                MoveGeomAndParticles    ( void );
            


protected:
    f32                 m_Health;
    f32                 m_AliveTime;
    f32                 m_TimeInState;  
    f32                 m_PersonalOffset;               // Used to randomize the vertical bobbing.
    guid                m_ScanCenterObject;
    guid                m_SpawnTubeLimitVolume;
    guid                m_TurretLimitVolume;    
    guid                m_HostGroup;
    guid                m_TargetGuid;
    guid                m_ScriptedPosition;
    f32                 m_OrbitRadius;                  // How far out to orbit the scripted position
    radian              m_OrbitOffset;                  // Offset around the orbit circle
    f32                 m_OrbitVerticalAmt;             // How much to wobble up and down
    guid                m_NextInActivationChain;        // Next orb in the activation chain.
                                                        // When the current orb is destroyed, it will
                                                        //    set the scripted position of the next orb
                                                        //    to NULL, thereby releasing it into the
                                                        //    world.  This is very specialized behaviour,
                                                        //    but the orb mechanics are pretty much solidified
                                                        //    at this point, and this will save us from 
                                                        //    implementing a ton of scripting.

    vector3             m_Velocity;    
    state               m_State;
    f32                 m_MoveSpeed;
    vector3             m_TargetPos;
    f32                 m_SearchTime;   
    vector3             m_LastGeometryPosition;     
    vector3             m_CurrentGeometryPosition;
    rhandle<char>       m_hCollisionParticles;
    f32                 m_CollisionParticleScale;
    f32                 m_WanderRadius;

    rhandle<char>       m_hAudioPackage;
    s32                 m_CreationSoundID;
    s32                 m_CollisionSoundID;
    s32                 m_FlyingSoundID;
    s32                 m_ScriptedFlyingSoundID;
    s32                 m_DeathSoundID;
    s32                 m_DestructionSoundID;
    voice_id            m_FlyingSoundVoice;

    rhandle<char>       m_hNormalParticles;
    f32                 m_NormalParticleScale;
    guid                m_NormalParticleGuid;       // Only valid while running
    rhandle<char>       m_hMutantVisionParticles;
    f32                 m_MutantParticleScale;
    guid                m_MutantParticleGuid;       // Only valid while running

    rhandle<char>       m_hDeathParticles;          // Death occurs when the orb
    f32                 m_DeathParticleScale;       // cannot find targets

    rhandle<char>       m_hDestructionParticles;    // Destruction occurs when the orb
    f32                 m_DestructionParticleScale; // dies due to excessive pain

    guid                m_ActivateWhenDead;         // orb dies or is destroyed
    guid                m_ActivateWhenArrived;      // orb arrives at it's target
    guid                m_Spawner;

    f32                 m_CollisionSphereRadius;

    f32                 m_AttackDamage;
    f32                 m_AttackForce;            

    u8                  m_SpawnTubeProb;            // 0-100 probability of choosing spawntube over turret

    u8                  m_bDestroyed:1,
                        m_bLostTarget:1,                        
                        m_bOrbActive:1,             // Is orb running logic/rendering, or is it in a host
                        m_bTrackedPositionsInvalid:1;


};


inline void alien_orb::SetSearchTime( f32 SearchTime )
{
    m_SearchTime = SearchTime;
}

inline void alien_orb::SetOrbitRadius( f32 OrbitRadius )
{
    m_OrbitRadius = MAX(0,OrbitRadius);
}

inline void alien_orb::SetOrbitOffset( f32 OrbitRadius )
{
    m_OrbitOffset = MAX(0,OrbitRadius);
}

inline xbool alien_orb::HasScriptedPosition( void )
{
    return !!m_ScriptedPosition;
}

#endif //__ALIEN_ORB_HPP__