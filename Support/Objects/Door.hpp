#ifndef DOOR_HPP
#define DOOR_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "AnimSurface.hpp"
#include "Circuit.hpp"

//=========================================================================
// CLASS
//=========================================================================



class door : public anim_surface
{
public:
    CREATE_RTTI( door, anim_surface, object )

    enum state
    {
        LOCKING,        // Playing closed -> locked animation.
        LOCKED,         // Has tight "Locked message" trigger.
        UNLOCKING,      // Playing locked -> closed animation.
        CLOSING,        // Playing open   -> closed animation.
        CLOSED,         // Has generous "open" trigger.
        PRECLOSE,       // Delay before closing.  Still has "open" trigger.
        OPENING,        // Playing closed -> open   animation.
        OPEN,           // Has generous "open" trigger.  (Closed when empty.)
  
        STAY_OPEN,      // Used to override normal behavior.  No trigger.
        STAY_CLOSED,    // Used to override normal behavior.  No trigger.

        MAX_STATE = STAY_CLOSED
    };

//=========================================================================

                                door            ( void );
   
	virtual			void	    OnEnumProp		( prop_enum& rList );
	virtual			xbool	    OnProperty		( prop_query& rPropQuery );
    
    virtual         bbox        GetLocalBBox    ( void ) const;
                    bbox        GetDoorBBox     ( void ) const;

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

                    void        SetTargetState  ( state TargetState );
                    void        OverRideLogic   ( xbool bRunLogic );

                    void        WakeUp          ( void );
                    state       GetState        ( void ) { return m_CurrentState; }

                    circuit&    GetCircuit      ( void ) { return m_Circuit; }

                    xbool       UsesProxBox     ( void ) { return m_bUseProximityBox; }

protected:   
//=========================================================================
       
 
    virtual void                OnRender		( void );
    virtual void                OnColNotify		( object& Object );
    virtual void                OnAdvanceLogic	( f32 DelaTime );
    virtual void                OnInit          ( void );     
    virtual void                OnColCheck      ( void );

#ifndef X_RETAIL
    virtual void                OnDebugRender   ( void );    
#endif // X_RETAIL

    void                        SetAnimIndex    ( void );

    void                        UpdateState     ( f32 DeltaTime );
    void                        SwitchState     ( state State );
    xbool                       CheckProximity  ( void );

protected:

    state                       m_InitialState;         // Initial state of the door.      
    state                       m_RestingState;         // Resting state of the door.      
    state                       m_CurrentState; 		// Current state that the door is in.
    state                       m_TargetState;          // The state that we want to get to.

    guid                        m_PortalGuid;           // Portal GUID
    s32                         m_AnimIndex[MAX_STATE]; // Anim group index for all the states.

    rhandle<char>               m_hAudioPackage;        // Audio resource associated with this object.
    char                        m_DoorClosedSfx[64];    // When the door is closed.
    f32                         m_PreCloseWaitTime;     // Time to wait before shutting down the emitter.
    u32                         m_Flags;                // Flags for the door.

    s32                         m_DoorSoundID;          // Sound ID currently playing..
    f32                         m_SoundOcclusion;       // The sound occlusion factor for this door.
//    bbox                        m_ProximityBBox;        // This will determine when to activate the emitter.
//    bbox                        m_CloseProximityBBox;   // Used for closed state.    
    vector3                     m_BBoxScale;            // Scale the Render bbox.
    bbox                        m_RenderBBox;           // The rendering bbox.
    f32                         m_CurrentWaitTime;

    xbool                       m_DoorBoneDebug;
    s32                         m_DoorCloseDelay;

    xbool                       m_bUseProximityBox;
    bbox                        m_ProximityBox;

    circuit                     m_Circuit;
};

//=========================================================================
// END
//=========================================================================
#endif
