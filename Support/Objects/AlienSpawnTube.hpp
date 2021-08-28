//==============================================================================
//
//  AlienSpawnTube.hpp
//
//==============================================================================
#ifndef __ALIEN_SPAWN_TUBE_HPP__
#define __ALIEN_SPAWN_TUBE_HPP__

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Objects\AnimSurface.hpp"
#include "Animation\AnimPlayer.hpp"
#include "ZoneMgr\ZoneMgr.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Objects\Event.hpp"
#include "TriggerEx\Affecters\object_affecter.hpp"

//==============================================================================
//  NOTES
//==============================================================================

class alien_spawn_tube : public anim_surface
{
public:
    CREATE_RTTI( alien_spawn_tube, anim_surface, object )


public:
    enum state
    {
        STATE_UNKNOWN,
        STATE_IDLE,
        STATE_SPAWNING_PREWAIT,        
        STATE_SPAWNING_POSTWAIT,
    };

public:
    alien_spawn_tube();
    ~alien_spawn_tube();


    virtual void                OnEnumProp      ( prop_enum&    List );
    virtual xbool               OnProperty      ( prop_query&   I    );

    virtual void                OnAdvanceLogic  ( f32 DeltaTime );
    virtual void                OnEvent         ( const event& Event );

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

#ifdef X_EDITOR
    virtual s32                 OnValidateProperties( xstring& ErrorMsg );
#endif


public:
            xbool               CanBeReservedByMe( guid Orb );
            xbool               Reserve         ( guid Orb );
            xbool               Unreserve       ( guid Orb );

            xbool               OrbEnter        ( guid Orb );
    

public:
    void                DeactivateOrb   ( void );
    void                ActivateOrb     ( void );

    void                SetSpawner      ( guid Spawner );

protected:

    void                SwitchState             ( state NewState );

protected:
    state               m_State;
    guid                m_ReservedBy;
    guid                m_SpawnGroup;           // Group object to insert guid into
    s32                 m_TemplateIDToSpawn;    
    s32                 m_iSpawnAnimName;       // index into dictionary    
    object_affecter     m_TargetAffecter;       // What to attack when spawned
    object_affecter     m_ActivateOnSpawn;      // What to notify on spawn
    xhandle             m_SpawnedVarHandle;     // Global variable handle
    s32                 m_SpawnedVarName;       // Name of the variable to store spawned object guid in
    guid                m_gLastSpawnedNPC;      // GUID of last object to be spawned
    
    // For spawned object anim 
    s32                 m_AnimName;         // Name of animation to play
    s32                 m_AnimGroupName;    // Name of animation group
    f32                 m_AnimPlayTime;     // Number of cycles/seconds to play
    u32                 m_AnimFlags;        // Animation control flags

};

#endif //__ALIEN_ORB_HPP__