///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Spatial_Object.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_SPATIAL_OBJECT_
#define _TRIGGER_SPATIAL_OBJECT_

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"

//=========================================================================
// SPATIAL_TRIGGER_OBJECT
//  Note: a spatial trigger can be of four types : axis aligned box, sphere, plane or ray
//=========================================================================

class trigger_spatial_object : public trigger_object
{
public:

     CREATE_RTTI( trigger_spatial_object, trigger_object , object )

public:
    enum spatial_types
    {
        SPATIAL_TYPES_INVALID = -1,

        TYPE_RAY,
        TYPE_PLANE,
        TYPE_AXIS_CUBE,
        TYPE_CUBIC,
        TYPE_SPHERICAL,

        SPATIAL_TYPES_END
    };

    enum activation_types
    {
        ACTIVATION_TYPES_INVALID = -1,

        ACTIVATE_ON_BY_OVERRIDE_TYPE,
        ACTIVATE_ON_PLAYER,
        ACTIVATE_ON_NPC,
        ACTIVATE_ON_BULLET,
        ACTIVATE_ON_NPC_OR_PLAYER,
        ACTIVATE_ON_USE_EVENT,

        ACTIVATION_TYPES_END
    };
    
    enum activation_response_type
    {
        ACTIVATION_RESPONSE_TYPE_INVALID = -1,
            
            RESPONSE_IF_STILL_INSIDE,
            RESPONSE_RETURN_TO_SLEEP,
            RESPONSE_PERMANENT_ON,
            RESPONSE_PERMANENT_OFF,
            
            ACTIVATION_RESPONSE_TYPE_END
    };

public:





                             trigger_spatial_object     ( void );
                            ~trigger_spatial_object     ( void );
                            
    virtual         s32         GetMaterial             ( void ) const { return MAT_TYPE_NULL; }
           
    virtual         void        OnEnumProp              ( prop_enum& rList );
    virtual         xbool       OnProperty              ( prop_query& rPropQuery );
    
    virtual         bbox        GetLocalBBox            ( void ) const ;
   
    virtual         void        OnColCheck              ( void );
    virtual         void        OnColNotify             ( object& Object );

    virtual const object_desc&  GetTypeDesc             ( void ) const;
    static  const object_desc&  GetObjectType           ( void );
      
    virtual         void        OnActivate              ( xbool Flag );    
    virtual const guid*         GetTriggerActor         ( void ) { return &m_TriggerActor; }

                    void        SetResponseType         ( activation_response_type ResponseType );
                    void        SetUpdateRate           ( f32 UpdateRate );

protected:
                   


    enum { MAX_DIMENSIONS_PARAMS = 3 };

protected:

#ifndef X_RETAIL
    virtual         void        OnDebugRender               ( void );
#endif // X_RETAIL

    virtual         void        OnInit                      ( void );
    virtual         void        OnKill                      ( void );
                   
    virtual         void        ExecuteLogic                ( f32 DeltaTime ); 
                    xbool       ExecutePlayerPolling        ( f32 DeltaTime );
    virtual         void        ActivateTrigger             ( void );
    virtual         void        DeactivateTrigger           ( void );
                    void        StartPlayerPolling          ( void );
                         
                    xbool       QueryPlayerInVolume         ( void );
                    xbool       QueryNpcInVolume            ( void );
                    xbool       QueryBulletInVolume         ( void );
    virtual         xbool       QueryObjectInVolume         ( object* pObject );
                    xbool       CheckForUseEvent            ( void );

                    void        Setup                       ( activation_types            ActivateType, 
                                                              spatial_types               SpatialType,
                                                              activation_response_type    ResponseType,
                                                              const f32*                  Dimensions );

                    void        Setup                       ( const f32*  Dimensions );
                    void        LogicCheckOnActivate        ( void );


    f32             m_Dimensions[MAX_DIMENSIONS_PARAMS];        // Dimensions of various types of triggers

    xbool                       m_IsActivated;                  // Flag if the trigger has been activated...
    activation_types            m_ActivationType;               // Types of objects which will turn on this trigger
    spatial_types               m_SpatialType;                  // The type of the trigger
    activation_response_type    m_ResponseType;                 // How the trigger responds to activation
    guid                        m_TriggerActor;                 // Actor who triggered us..
    xbool                       m_EventPollingStarted;          // Event polling flag.
    xbool                       m_HasPressed;                   // Stores if the player has pressed the key

protected:
    
    typedef enum_pair<spatial_types>                spatial_pair;
    typedef enum_table<spatial_types>               spatial_table;
    typedef enum_pair<activation_types>             activation_pair;
    typedef enum_table<activation_types>            activation_table;
    typedef enum_pair<activation_response_type>     response_pair;
    typedef enum_table<activation_response_type>    response_table;
    
    static spatial_pair             s_SpatialPairTable[];
    static spatial_table            s_SpatialEnumTable;
    static activation_pair          s_ActivationPairTable[];
    static activation_table         s_ActivationEnumTable;
    static response_pair            s_ResponsePairTable[];
    static response_table           s_ResponseEnumTable;

};

//===========================================================================

inline
void trigger_spatial_object::SetResponseType( trigger_spatial_object::activation_response_type ResponseType )
{
    m_ResponseType = ResponseType;
}

//===========================================================================

inline
void trigger_spatial_object::SetUpdateRate( f32 UpdateRate )
{
    m_UpdateRate = UpdateRate;
}


//=========================================================================
// END
//=========================================================================

#endif





















