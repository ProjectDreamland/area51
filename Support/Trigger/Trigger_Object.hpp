///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Object.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_OBJECT_
#define _TRIGGER_OBJECT_

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "..\Support\Trigger\Trigger_Conditionals.hpp"
#include "..\Support\Trigger\Trigger_Actions.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"

class trigger_mngr;

//=========================================================================
// TRIGGER_OBJECT
//  NOTE : trigger_object are not contained in virtual space, only the 
//          derived class spatial_trigger_object is contained within virutal space...
//=========================================================================

class trigger_object : public object
{
public:
    
    CREATE_RTTI( trigger_object, object, object )
                                    trigger_object                      ( void );
                                   ~trigger_object                      ( void );
                            
    virtual         bbox            GetLocalBBox			            ( void ) const ;
    virtual         s32             GetMaterial				            ( void ) const { return MAT_TYPE_NULL; }
           
	virtual			void	        OnEnumProp				            ( prop_enum& rList );
	virtual			xbool	        OnProperty				            ( prop_query& rPropQuery );
    virtual         void            OnActivate                          ( xbool Flag );    

    virtual const object_desc&      GetTypeDesc                         ( void ) const;
    static  const object_desc&      GetObjectType                       ( void );
    
//=========================================================================
// Interface functions for the various condtions and actions..
//=========================================================================

    void                            RemoveCondition                     ( conditional_base* pCondition );
    void                            RemoveAction                        ( actions_base* pAction );
    virtual const guid*             GetTriggerActor                     ( void ) { return NULL; }
    void                            KillTrigger                         ( void );

#ifdef WIN32
    virtual void             EditorPreGame           ( void );
#endif
    
protected:
    
//=========================================================================
// Various Enumerations..
//=========================================================================

    enum trigger_state
    {
        TRIGGER_STATE_INVALID = -1,
          
            STATE_SLEEPING,
            STATE_CHECKING,
            STATE_DELAYING,
            STATE_RECOVERY,
            STATE_DYING,
            
            TRIGGER_STATE_END
            
    };
    
    enum trigger_type
    {
        TRIGGER_TYPE_INVALID = -1,
            
            TRIGGER_ONCE,
            TRIGGER_REPEATING,
            TRIGGER_REPEATING_COUNTED,
            
            TRIGGER_TYPE_END
    };

    enum { MAX_PTR_ARRAY_SIZE = 32 };

protected:
    
//=========================================================================
// Internal data structures and classes...
//=========================================================================

    class  trigger_selector : public prop_interface
    {
    public:
                                trigger_selector( void );
                        void    Init                                 ( trigger_object* pParent );
        virtual			void	OnEnumProp				             ( prop_enum& rList );
        virtual			xbool	OnProperty				             ( prop_query& rPropQuery );
        
        conditional_base::conditional_types     m_ConditionType;                            // Condtion type currently selected
        actions_base::action_types              m_ActionType;                               // Action type currently selected
        xbool                                   m_Active;                                   // State Flag to determine if the selector is active..
        trigger_object*                         m_Parent;                                   // Parent of the selector
        sml_string                              m_VariableName;                             // Name of the variable
    };
    
protected:
    
    virtual         void        OnInit					( void );
    virtual         void        OnKill                  ( void );

#ifndef X_RETAIL
    virtual         void        OnDebugRender			( void );
#endif // X_RETAIL

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
                    
                    void        AddCondition            ( conditional_base::conditional_types ConditionType , s32 Number );
                    void        AddAction               ( actions_base::action_types ActionType , s32 Number );


//=========================================================================
// Property interface functions...
// Functions with Dynamic in the name are used on loading to create the
//  various triggers before they can be initailized by the property system.
//=========================================================================

protected: 
                    
                    void        EnumPropDynamic         ( prop_enum& rList );
                    void        EnumPropConditions      ( prop_enum& rList );
                    void        EnumPropActions         ( prop_enum& rList );
                    void        EnumPropElseConditions  ( prop_enum& rList );
                    void        EnumPropElseActions     ( prop_enum& rList );

                    xbool       OnPropertyDynamic       ( prop_query& rPropQuery );
                    xbool       OnPropertyConditions    ( prop_query& rPropQuery );
                    xbool       OnPropertyActions       ( prop_query& rPropQuery );

                    void        CalculateAndFlags       ( void );
                    void        CalculateOrFlags        ( void );

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

                    xbool       EvaulateCondtions       ( void );
                    xbool       EvaulateMainCondtions   ( void );
                    xbool       EvaulateElseCondtions   ( void );
                    void        ExecuteAllActions       ( void );

                    xbool       CheckElseState          ( void );

//=========================================================================
// Useful interface functions for derived classes and others..
//=========================================================================
                    
protected:  

                    void            SetTriggerState     ( const trigger_state State );
                    trigger_state   GetTriggerState     ( void ) { return m_State; }
                    void            ForceNextUpdate     ( void );
                    xbool           CanUpdate           ( f32 DeltaTime );  
                    
protected:
    
    conditional_base*   m_Conditions[MAX_PTR_ARRAY_SIZE];   // Ptr array for condtions
    actions_base*       m_Actions[MAX_PTR_ARRAY_SIZE];      // Ptr array for actions

    f32                 m_UpdateRate;                   // How often this trigger updates if at all
    f32                 m_RecoveryRate;                 // Amount of time from executing action to returning to checking, only applies to repeating triggers..
    f32                 m_DelayRate;                    // Amount of time from triggering of condition to execution of action to wait

    u32                 m_AndFlags;                     // And flags, all these bits must be true.
    u32                 m_OrFlags;                      // OR  flags, any of these bits can be true..
    u32                 m_ElseAndFlags;                 // And flags for Else block, all these bits must be true.
    u32                 m_ElseOrFlags;                  // OR  flags for Else block, any of these bits can be true..
 
#ifdef TARGET_PC
    trigger_selector    m_Selector;                     // Selector object used to actually determine the type of condition and actions to create..
#endif

    s32                 m_NumConditons;                 // Number of total conditions..
    s32                 m_NumActions;                   // Number of total actions...
    
    trigger_type        m_Type;                         // The trigger type
   
    xbool               m_DrawActivationSphere;         // Debug functionality
    xcolor              m_CurrentColor;                 // Debug functionality

    xbool               m_OnActivate;                   // Flag which controls whether an object should update..
    s32                 m_RepeatCount;                  // Max times a trigger is allowed to repeat..
    s32                 m_ActivateCount;                // Number of times a trigger has been activated..
    xbool               m_UseElse;                      // Flag is on if there  are any else conditions or actions..
    xbool               m_ExecuteElseActions;           // If the condtions of the else block resolve to true this flag will be true...
    
private:
    
    //the reason for making these private is because these functions affect variables
    //which the base trigger class fundementally depend upon to update correctly and should not be 
    //changed directly but instead through the interface functions in order for the state machine
    //to correcly operate..

    void        UpdateNextTime          ( f32 Time ); 
    xbool       CheckNextTime           ( f32 DeltaTime ); 

    trigger_state       m_State;
    f32                 m_NextUpdateTime;               // Next time for this trigger to update...
      
    //Trigger manager vars..
    guid                m_Next;                         // Link list for the triggers only to be used by TriggerManager
    guid                m_Prev;                         // Link list for the triggers only to be used by TriggerManager   
    s32                 m_TriggerSlot;                  // Slot the trigger is sorted in the TriggerManager

    //State flags used to init correctly when we enter those states..
    xbool               m_EnteringDelay;                // Flags which get set upon entering Delay state used to keep update time correct       
    xbool               m_EnteringRecovery;             // Flags which get set upon entering Recovery state used to keep update time correct            

    //////////////////////////////////////////////////////////////////////////////////////////////
    //FRIEND CLASSES
    
    friend trigger_selector;
    friend trigger_mngr;
};

//=========================================================================
// END
//=========================================================================

#endif
