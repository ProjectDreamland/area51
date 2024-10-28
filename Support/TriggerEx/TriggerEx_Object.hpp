///////////////////////////////////////////////////////////////////////////////
//
//  TriggerEx_Object.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGEREX_OBJECT_
#define _TRIGGEREX_OBJECT_

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "..\Support\TriggerEx\TriggerEx_Conditionals.hpp"
#include "..\Support\TriggerEx\TriggerEx_Actions.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "Affecters\conditional_affecter.hpp"
#include "..\Support\Objects\focusobject.hpp"

class trigger_ex_mngr;
class check_point_mgr;

//=========================================================================
// TRIGGER_OBJECT
//  NOTE : trigger_ex_object are not contained in virtual space, only the 
//          derived class spatial_trigger_ex_object is contained within virutal space...
//=========================================================================

class trigger_ex_object : public object
{
public:
    
    CREATE_RTTI( trigger_ex_object, object, object )
                                    trigger_ex_object           ( void );
    virtual                        ~trigger_ex_object           ( void );
                            
    virtual         bbox            GetLocalBBox			    ( void ) const ;
    virtual         s32             GetMaterial				    ( void ) const { return MAT_TYPE_NULL; }
           
	virtual			void	        OnEnumProp				    ( prop_enum& rList );
	virtual			xbool	        OnProperty				    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         s32             OnValidateActions           ( xstring& ActionType, s32 nActions, actions_ex_base* Actions[], xstring& ErrorMsg );
    virtual         s32             OnValidateProperties        ( xstring& ErrorMsg );
#endif

	virtual			void	        OnColCheck				    ( void );
    virtual         void            OnColNotify                 ( object& Object );
    virtual         void            OnActivate                  ( xbool Flag );

#ifndef X_RETAIL
    virtual         void            OnDebugRender               ( void );
#endif // X_RETAIL

    virtual         void            OnMove                      ( const vector3& NewPos   );      
    
                    void            LogicCheckOnActivate        ( void );

    virtual const object_desc&      GetTypeDesc                 ( void ) const;
    static  const object_desc&      GetObjectType               ( void );

//=========================================================================
// Interface functions for the various condtions and actions..
//=========================================================================

    inline s32                      GetActionCount              ( void ) { return m_NumIfActions; }
    inline s32                      GetElseActionCount          ( void ) { return m_NumElseActions; }
    
    actions_ex_base*                GetAction                   ( s32 Index );
    actions_ex_base*                GetElseAction               ( s32 Index );
    
    xbool                           IsFirstInActionSet          ( actions_ex_base* pAction );
    s32                             GetActionIndex              ( actions_ex_base* pAction );
    void                            SetActionIndex              ( actions_ex_base* pAction, s32 Index );
    void                            RemoveAction                ( actions_ex_base* pAction, xbool bAndDelete = TRUE );

    xbool                           CanSwitchElse               ( actions_ex_base* pAction );
    void                            SwitchElse                  ( actions_ex_base* pAction );
    
    virtual const guid*             GetTriggerActor             ( void ) { return NULL; }
    void                            KillTrigger                 ( void );
    void                            ResetTrigger                ( void );

    inline f32                      GetUpdateRate               ( void ) { return m_UpdateRate; }
    void                            ReleaseBlocking             ( void );

    xbool                           HasDialogLine               ( void );
    
    //meta functions
    void                            GetValidLabels              ( xbool bElseAction, char* pLabels );
    xbool                           SetTriggerActionIndexToLabel( const char* pName );
    xbool                           RetryOnError                ( void );

    void                            ForceStartTrigger           ( void );

#ifndef CONFIG_RETAIL
//    void                            DumpData                    ( text_out& DataDumpFile );
    void                            DumpData                    ( X_FILE* pFile );
#endif

#ifdef X_EDITOR
    virtual void                    EditorPreGame               ( void );
    const char*                     GetTriggerName              ( void ) { return GetName(); }
    char*                           GetTriggerDescription       ( void ) { return m_Description.Get(); }
    char*                           GetTriggerTypeString        ( void );
    char*                           GetTriggerStatusString      ( void );
    char*                           GetTriggerActionString      ( void );
#endif // X_EDITOR
       
public:
    
//=========================================================================
// Various Enumerations..
//=========================================================================

    enum trigger_ex_state
    {
        TRIGGER_STATE_INVALID = -1,
        STATE_SLEEPING,
        STATE_CHECKING,
        STATE_DELAYING,
        STATE_RECOVERY,
        STATE_RESETTING,
        TRIGGER_STATE_END            
    };
    
    enum trigger_ex_types
    {
        TRIGGER_TYPES_INVALID = -1,
        TRIGGER_TYPE_SIMPLE,
        TRIGGER_TYPE_SPATIAL,
        TRIGGER_TYPE_VIEWABLE,
        TRIGGER_TYPES_END
    };

    enum trigger_ex_occurs
    {
        TRIGGER_BEHAVIOR_INVALID = -1,
        TRIGGER_OCCURS_ONCE,
        TRIGGER_OCCURS_REPEATING,       
        TRIGGER_BEHAVIOR_END
    };

    enum trigger_ex_reset
    {
        TRIGGER_RESET_INVALID = -1,
        TRIGGER_RESET_DESTROY,
        TRIGGER_RESET_DEACTIVATE,       
        TRIGGER_RESET_END
    };

    //spatial specific enums

    enum trigger_ex_spatial_types
    {
        SPATIAL_TYPES_INVALID = -1,
        SPATIAL_TYPE_AXIS_CUBE,
        SPATIAL_TYPE_SPHERICAL,
        SPATIAL_TYPES_END
    };

    enum trigger_ex_spatial_activation_types
    {
        SPATIAL_ACTIVATION_TYPES_INVALID = -1,
        SPATIAL_ACTIVATE_ON_PLAYER,
        SPATIAL_ACTIVATE_ON_NPC,
        SPATIAL_ACTIVATE_ON_BULLET,
        SPATIAL_ACTIVATE_ON_NPC_OR_PLAYER,
        SPATIAL_ACTIVATE_ON_INTERNAL,
        SPATIAL_ACTIVATION_TYPES_END
    };

    enum trigger_ex_spatial_activation_bullet_types
    {
        SPATIAL_ACTIVATION_BULLET_TYPES_INVALID = -1,
        SPATIAL_ACTIVATE_BULLET_TYPES_ALL,
        SPATIAL_ACTIVATE_BULLET_TYPES_BULLET,
        SPATIAL_ACTIVATE_BULLET_TYPES_MES_ALT_FIRE,
        SPATIAL_ACTIVATE_BULLET_TYPES_NPC_BULLET,
        SPATIAL_ACTIVATE_BULLET_TYPES_PLAYER_BULLET,
        SPATIAL_ACTIVATION_BULLET_TYPES_END
    };
    
    enum trigger_ex_spatial_activation_response
    {
        SPATIAL_RESPONSE_TYPE_INVALID = -1,
        SPATIAL_RESPONSE_IF_STILL_INSIDE,
        SPATIAL_RESPONSE_RETURN_TO_SLEEP,
        SPATIAL_RESPONSE_TYPE_END
    };
    
    enum { MAX_ACTION_ARRAY_SIZE = 64 };
    enum { MAX_SPATIAL_DIMENSIONS_PARAMS = 3 };

    //specifically for description's debug rendering
    trigger_ex_types                GetTriggerType              ( void ) { return m_TriggerType; }
                    void            OnRenderSpatial             ( void );

    inline          xbool           IsActive                    ( void ) { return m_OnActivate; }

    trigger_ex_spatial_types        GetSpatialType              ( void ) { return m_SpatialType; };
    f32                             GetDimension                ( s32 DimensionIndex ) { if ( IN_RANGE( 0, DimensionIndex, 2 ) ) return m_Dimensions[DimensionIndex]; else return 0.0;};

    s32             m_DrawActivationIcon;           // Debug functionality
    xbool           m_DrawError;
    xcolor          m_CurrentColor;                 // Debug functionality

#ifdef X_EDITOR

protected:
    
//=========================================================================
// Internal data structures and classes...
//=========================================================================

    class  trigger_ex_selector : public prop_interface
    {
    public:
                                trigger_ex_selector( void );
                        void    Init                                 ( trigger_ex_object* pParent );
        virtual			void	OnEnumProp				             ( prop_enum& rList );
        virtual			xbool	OnProperty				             ( prop_query& rPropQuery );
        
        actions_ex_base::action_ex_types            m_ActionType;        // Action type currently selected
        xbool                                       m_Active;            // State Flag to determine if the selector is active..
        trigger_ex_object*                          m_Parent;            // Parent of the selector
    };
    
public:

    class triggex_ex_copy_data
    {
    public:
                                triggex_ex_copy_data( void );
        void                    Copy( actions_ex_base* pAction );
        void                    Paste( trigger_ex_object* pTrigger, xbool bAsIfAction, s32 iIndex );
        xbool                   HasActionToPaste(void);

        actions_ex_base::action_ex_types         m_ActionType;
        xarray<prop_container>                   m_Properties;
    };

    s32                         m_iPasteIndex;

    void                        CopyAction                          ( actions_ex_base* pAction );

#endif //X_EDITOR


protected:
    
    virtual         void        OnInit					( void );
    virtual         void        OnKill                  ( void );
    virtual         void        OnRender        		( void );
    virtual         void        OnAdvanceLogic          ( f32 DeltaTime );      

//=========================================================================
// Trigger Manager Interface functions...
// The ExecuteLogic function does all the logic for this trigger, should be only called by the g_TriggerMngr.
//=========================================================================

    virtual         void        ExecuteLogic            ( f32 DeltaTime );
                    xbool       IsAwake                 ( void );

//=========================================================================
// Editor functionality..
//=========================================================================

protected: 
                    
                    xbool       AddAction               ( actions_ex_base::action_ex_types ActionType , s32 Number, xbool bAsElse );


//=========================================================================
// Property interface functions...
// Functions with Dynamic in the name are used on loading to create the
//  various triggers before they can be initailized by the property system.
//=========================================================================

protected: 
                    
                    void        EnumPropDynamic         ( prop_enum& rList );
                    void        EnumPropActions         ( prop_enum& rList );
                    void        EnumPropElseActions     ( prop_enum& rList );
                    void        EnumPropSpatial         ( prop_enum& rPropList );

                    xbool       OnPropertyDynamic       ( prop_query& rPropQuery );
                    xbool       OnPropertyActions       ( prop_query& rPropQuery );
                    xbool       OnPropertySpatial       ( prop_query& rPropQuery );

//=========================================================================
// Debug rendering function
//=========================================================================

protected:  
                    
                    void        OnRenderActions         ( void );

//=========================================================================
// Various state execution functions and related..    
//=========================================================================

protected: 
                    
                    void        ExecuteSleeping         ( f32 DeltaTime );
                    void        ExecuteChecking         ( f32 DeltaTime );
                    void        ExecuteRecovery         ( f32 DeltaTime );
                    void        ExecuteDelaying         ( f32 DeltaTime );

                    void        ExecuteAllActions       ( void );
                    xbool       ExecuteIndividualAction ( actions_ex_base* pAction, s32 ActionIndex, f32 DeltaTime );

//=========================================================================
// Useful interface functions for derived classes and others..
//=========================================================================
                    
protected:  

                    void                SetTriggerState     ( const trigger_ex_state State );
                    trigger_ex_state    GetTriggerState     ( void ) { return m_State; }
                    void                ForceNextUpdate     ( void );
                    xbool               CanUpdate           ( f32 DeltaTime );

                    void                PreUpdateState      ( void );
                    
//spatial trigger functions

                    void                ActivateSpatialTrigger  ( void );
                    void                DeactivateSpatialTrigger( void );    

                    xbool               QueryPlayerInVolume     ( void );
                    xbool               QueryNpcInVolume        ( void );
                    xbool               QueryBulletInVolume     ( void );
    virtual         xbool               QueryObjectInVolume     ( object* pObject );

protected:
    
    actions_ex_base*        m_IfActions[MAX_ACTION_ARRAY_SIZE];      // Ptr array for actions
    actions_ex_base*        m_ElseActions[MAX_ACTION_ARRAY_SIZE];    // Ptr array for actions

    conditional_affecter    m_ConditionAffecter;

    trigger_ex_types        m_TriggerType;                  // What kind of Trigger is this, simple, spatial, viewable

    f32                     m_UpdateRate;                   // How often this trigger updates if at all
    f32                     m_RecoveryRate;                 // Amount of time from executing action to returning to checking, only applies to repeating triggers..
    f32                     m_DelayRate;                    // Amount of time from triggering of condition to execution of action to wait
 
#ifdef X_EDITOR
    trigger_ex_selector     m_Selector;                     // Selector object used to actually determine the type of actions to create..
    big_string              m_Description;
#endif // X_EDITOR

    s32                     m_NumIfActions;                 // Number of if actions...
    s32                     m_NumElseActions;               // Number of else actions...
        
    trigger_ex_occurs       m_Behavior;                     // The trigger behavior type
    trigger_ex_reset        m_ResetType;                    // What to do once the trigger has completed
   
    xbool                   m_OnActivate;                   // Flag which controls whether an object should update..
    s32                     m_RepeatCount;                  // Max times a trigger is allowed to repeat..
    s32                     m_ActivateCount;                // Number of times a trigger has been activated..
    xbool                   m_bTriggerFired;

    enum action_status 
    {
        ACTION_ONHOLD,
        ACTION_RUN_IF,
        ACTION_RUN_ELSE
    };

    action_status           m_CurrentActionStatus;       
    s32                     m_iCurrentActionIndex;          // Are we doing advance logic's?
    
//Spatial Variables
protected:

    f32                                         m_Dimensions[MAX_SPATIAL_DIMENSIONS_PARAMS];// Dimensions of various types of triggers

    xbool                                       m_IsSpatialActivated;    // Flag if the trigger has been activated...
    trigger_ex_spatial_activation_types         m_ActivationType;        // Types of objects which will turn on this trigger
    trigger_ex_spatial_activation_bullet_types  m_ActivationBulletType;  // Type of bullet that will trigger a bulleted spatial trigger
    trigger_ex_spatial_types                    m_SpatialType;           // The type of the trigger
    trigger_ex_spatial_activation_response      m_ResponseType;          // How the trigger responds to activation
    s32                                         m_ActorGuidVarName;      // Name of the variable to store actor guid in
    xhandle                                     m_ActorGuidVarHandle;    // Global variable handle..
    xbool                                       m_bRequiresLineOfSight;  // Requires a ray test to validate view check
    u32                                         m_bLOSVerified;          // TRUE if a viewable trigger that requires LOS 
                                                                         //   has been activated via a clear line of sight
        
    typedef enum_pair<trigger_ex_spatial_types>                 spatial_pair;
    typedef enum_table<trigger_ex_spatial_types>                spatial_table;
    
    static spatial_pair                                         s_SpatialPairTable[];
    static spatial_table                                        s_SpatialEnumTable;
    
    typedef enum_pair<trigger_ex_spatial_activation_types>      activation_pair;
    typedef enum_table<trigger_ex_spatial_activation_types>     activation_table;
    
    static activation_pair                                      s_ActivationPairTable[];
    static activation_table                                     s_ActivationEnumTable;

    typedef enum_pair<trigger_ex_spatial_activation_bullet_types> activation_bullet_pair;
    typedef enum_table<trigger_ex_spatial_activation_bullet_types> activation_bullet_table;

    static activation_bullet_pair                               s_ActivationBulletTypePairTable[];
    static activation_bullet_table                              s_ActivationBulletTypeEnumTable;
    
    typedef enum_pair<trigger_ex_spatial_activation_response>   response_pair;
    typedef enum_table<trigger_ex_spatial_activation_response>  response_table;
    
    static response_pair                                        s_ResponsePairTable[];
    static response_table                                       s_ResponseEnumTable;

private:
    
    //the reason for making these private is because these functions affect variables
    //which the base trigger class fundementally depend upon to update correctly and should not be 
    //changed directly but instead through the interface functions in order for the state machine
    //to correcly operate..

    void        UpdateNextTime          ( f32 Time ); 
    xbool       CheckNextTime           ( f32 DeltaTime ); 

    trigger_ex_state    m_State;
    f32                 m_NextUpdateTime;               // Next time for this trigger to update...
      
    //Trigger manager vars..
    guid                m_Next;                         // Link list for the triggers only to be used by TriggerManager
    guid                m_Prev;                         // Link list for the triggers only to be used by TriggerManager   
    s32                 m_TriggerSlot;                  // Slot the trigger is sorted in the TriggerManager

    //State flags used to init correctly when we enter those states..
    xbool               m_EnteringDelay;                // Flags which get set upon entering Delay state used to keep update time correct       
    xbool               m_EnteringRecovery;             // Flags which get set upon entering Recovery state used to keep update time correct            
    xbool               m_bQuitRecovery;

    //////////////////////////////////////////////////////////////////////////////////////////////
    //FRIEND CLASSES
#ifdef X_EDITOR    
    friend trigger_ex_selector;
    friend triggex_ex_copy_data;
#endif //X_EDITOR
    friend trigger_ex_mngr;
    friend focus_object;
    friend check_point_mgr;
    friend void RunGame(void);
};

//=========================================================================

inline
actions_ex_base* trigger_ex_object::GetAction( s32 Index ) 
{ 
    ASSERT( Index >= 0 );
    ASSERT( Index < m_NumIfActions );
    
    return m_IfActions[Index];   
}

//=========================================================================

inline
actions_ex_base* trigger_ex_object::GetElseAction( s32 Index ) 
{ 
    ASSERT( Index >= 0 );
    ASSERT( Index < m_NumElseActions );
    
    return m_ElseActions[Index]; 
}

//=========================================================================



//=========================================================================
// END
//=========================================================================

#endif
