//==============================================================================
//
//  AlienPlatform.hpp
//
//==============================================================================
#ifndef __ALIEN_PLATFORM_HPP__
#define __ALIEN_PLATFORM_HPP__

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
class alien_platform : public anim_surface
{
public:
    CREATE_RTTI( alien_platform, anim_surface, object )


public:
    alien_platform();
    ~alien_platform();


    virtual void                OnEnumProp      ( prop_enum&    List );
    virtual xbool               OnProperty      ( prop_query&   I    );

    virtual void                OnAdvanceLogic  ( f32 DeltaTime );
    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    virtual void                OnRender        ( void );

    virtual void                OnMove              ( const vector3& NewPos   );      
    virtual void                OnMoveRel           ( const vector3& DeltaPos );    
    virtual void                OnTransform         ( const matrix4& L2W      );


#ifndef X_RETAIL
    virtual void                OnDebugRender   ( void );
#endif // X_RETAIL

    virtual void                OnColCheck      ( void );

#ifdef X_EDITOR
    virtual s32                 OnValidateProperties( xstring& ErrorMsg );
#endif


protected:
            enum platform_state
            {
                PLATFORM_STATE_IDLE,
                PLATFORM_STATE_ACTIVATING,
                PLATFORM_STATE_ACTIVE,
                PLATFORM_STATE_DEACTIVATING,
            };

            struct pathing
            {
                guid Source;        // Dock adjoined to platform's current dock
                guid This;          // Dock in question.
            };

            void                UpdateAttachedObject ( void );

            xbool               SwitchState     ( platform_state State );

            xbool               IsPlayerOn      ( void );
            xbool               IsUsePressed    ( void );
            xbool               GetNearestDockInCone ( const vector3& Pos,
                                                       const vector3& Dir,
                                                       guid&          OutDock,
                                                       guid&          OutPath );

            void                Highlight       ( guid Guid );
            void                Unhighlight     ( guid Guid );

            void                ConfigureTrackerAndLaunch( void );
            void                HandleTransit   ( void );

            void                HandleIdleLogic         ( f32 DeltaTime );
            void                HandleActivatingLogic   ( f32 DeltaTime );
            void                HandleActiveLogic       ( f32 DeltaTime );
            void                HandleDeactivatingLogic ( f32 DeltaTime );
            guid                SelectNextDestinationOnWayToGoal( guid GoalDock );
            s32                 AppendGuidsFromDock     ( pathing&          Dock, 
                                                          guid              WatchForThis,
                                                          xarray<pathing>&  Array,                                                          
                                                          s32&              WatchLocatedHere );
            void                ResolveQueuedDestination( void );
            void                ActivateDocks           ( xbool bActive );

protected:

    

    guid            m_Tracker;          // Tracker that the platform is coupled to.
    guid            m_CurrentDock;      // Dock we are currently attached to.
    guid            m_DestinationDock;  // Dock we are travelling to.
    guid            m_DestinationPath;  // Path to the destination
    guid            m_HighlightedDock;  // Dock we are highlighting, if we are not in transit and the player
                                        // is both standing on me and looking at a valid destination dock.
    guid            m_QueuedDestinationDock;    // For scripting: where should the dock go when it finishes
    guid            m_ActivateOnArrival;
    guid            m_ActivateOnDeparture; 

    guid            m_DragThisAroundWithMe; // Whatever this object is, drag it around.

    platform_state  m_State;
    f32             m_TrackerTargetTime;
    u32             m_bInTransit:1,
                    m_bPlayerOnMe:1;

#if (defined X_EDITOR) && (defined X_DEBUG)
    
    vector3         m_DbgEyePos;
    vector3         m_DbgEyeDir;

#endif  
};


#endif //__ALIEN_PLATFORM_HPP__