//==============================================================================
//
//  AlienOrb.hpp
//
//==============================================================================
#ifndef __ALIEN_SPOTTER_HPP__
#define __ALIEN_SPOTTER_HPP__

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

class alien_spotter : public turret
{
public:
    CREATE_RTTI( alien_spotter, turret, object )

     alien_spotter();
    ~alien_spotter();

    enum
    {
        MAX_ACTIVE_ORBS     = 8,
    };
public:

    virtual void                OnEnumProp      ( prop_enum&    List );
    virtual xbool               OnProperty      ( prop_query&   I    );
    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    virtual void                OnAdvanceLogic  ( f32 DeltaTime );
    virtual void                SetTargetGuid   ( guid Guid );

    virtual turret::tracking_status TrackTarget( f32 DeltaTime );

protected:
    virtual xbool               LaunchOrb       ( void );
    virtual xbool               GetLaunchPoint  ( vector3& Pos, vector3& Dir, radian3& Rot );
    virtual void                OnEnterState    ( state NewState );
    virtual void                StartAiming     ( void );
    virtual void                StopAiming      ( void );
            void                DeathSpawnOrbs  ( void );

protected:
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

    rhandle<char>       m_hSpotterAudioPackage;
    s32                 m_AimStartLoopSoundID;
    s32                 m_AimStopSoundID;
    voice_id            m_AimStartLoopVoiceID;

    u8                  m_bTrackingLocked:1;
};


#endif // __ALIEN_SPOTTER_HPP__
