//==============================================================================
//
//  AlienPlatformDock.hpp
//
//==============================================================================
#ifndef __ALIEN_PLATFORM_DOCK_HPP__
#define __ALIEN_PLATFORM_DOCK_HPP__


#define DOCK_MAX_DESTINATIONS             8
//==============================================================================
//  INCLUDES
//==============================================================================

#include "Objects\AnimSurface.hpp"
#include "Animation\AnimPlayer.hpp"
#include "ZoneMgr\ZoneMgr.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "TriggerEx\Affecters\object_affecter.hpp"

//==============================================================================
//  NOTES
//==============================================================================
class alien_platform_dock : public anim_surface
{
public:
    CREATE_RTTI( alien_platform_dock, anim_surface, object )


public:
    alien_platform_dock();
    ~alien_platform_dock();


    virtual void                OnEnumProp      ( prop_enum&    List );
    virtual xbool               OnProperty      ( prop_query&   I    );

    virtual void                OnAdvanceLogic  ( f32 DeltaTime );
    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

#if !defined( CONFIG_RETAIL )
    virtual void                OnRender        ( void );
#endif // !defined( CONFIG_RETAIL )

#ifndef X_RETAIL
    virtual void                OnDebugRender   ( void );
#endif // X_RETAIL

    virtual void                OnColCheck      ( void );

public:
            s32                 GetDestinationCount( void      );
            xbool               GetDestination     ( s32 iDest, guid& OutDock, guid& OutPath );
            
            void                Highlight       ( void );
            void                Unhighlight     ( void );

            void                ActivateDock    ( xbool bActive );            

#ifdef X_EDITOR
    virtual s32                 OnValidateProperties( xstring& ErrorMsg );
#endif

protected:
        enum state
        {
            STATE_IDLE,
            STATE_ACTIVE,
            STATE_HIGHLIGHTED,
        };

        enum effect
        {
            EFFECT_IDLE         = 0,
            EFFECT_ACTIVE       = 1,
            EFFECT_HIGHLIGHTED  = 2,

            EFFECT_COUNT        = 3,
            EFFECT_UNKNOWN,
        };
            
            xbool               IsPlayerOn      ( void );
            void                SwitchState     ( state     NewState );
            void                ActivateEffect  ( effect    Effect   );

protected:

    
    struct destination
    {
        object_affecter         m_Dock;
        object_affecter         m_Path;
    };

    


    guid                m_DockedPlatform;
    destination         m_Destination[ DOCK_MAX_DESTINATIONS ];
    s32                 m_nDestinations;
    state               m_State;
    guid                m_Effect[ EFFECT_COUNT ];
    effect              m_CurrentEffect;


    u8                  m_bPlayerOn:1,          // Player is standing on this dock
                        m_bHighlighted:1,       // Hightlighted by another dock
                        m_bActive:1;            // Active could indicate the player is in the network

};


#endif //__ALIEN_PLATFORM_DOCK_HPP__