///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Conditionals.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_CONDITIONALS_
#define _TRIGGER_CONDITIONALS_

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_types.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "..\Support\Globals\Global_Variables_Manager.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "Auxiliary\MiscUtils\PropertyEnum.hpp"
#include "Inventory\\InventoryItem.hpp"
#include "InputMgr\GamePad.hpp"
#include "..\Support\Objects\Player.hpp"
//=========================================================================
// CONDITIONAL_BASE
//=========================================================================
struct condition_create_function;
class  trigger_object;

class  conditional_base : public prop_interface
{  
    
public:
      
    //Note ** Do not remove depreciated enums from list. The trigger object saves out a list
    //of conditions it contains and uses the enum cast to a s32 to identify the action. Changing
    //the order of the enums or removing enums in the middle will break this listing..**//

    enum conditional_types
    {
        INVALID_CONDITIONAL_TYPES = -1,
            
            TYPE_CONDITION_PLAYER_HEALTH,               //0
            TYPE_CONDITION_PLAYER_HAS,                  //1
            TYPE_CONDITION_OBJECT_EXIST,                //2
            TYPE_CONDITION_CHECK_STATE_VARS,            //3
            TYPE_CONDITION_CHECK_SCRIPT,                //4
            TYPE_CONDITION_CHECK_TRIGGER_STATE,         //5
            TYPE_CONDITION_RANDOM_CHANCE,               //6
            TYPE_CONDITION_ONLY_ON_THIS_NPC,            //7
            TYPE_CONDITION_ONLY_ON_THIS_ITEM,           //8
            TYPE_CONDITION_COUNT_THINGS,                //9
            TYPE_CONDITION_CHECK_TIMMER,                //10
            TYPE_CONDITION_COUNTER,                     //11
            TYPE_CONDITION_OBJECT_HEALTH,               //12
            TYPE_CONDITION_ON_THIS_BUTTON,              //13
            TYPE_CONDITION_PLAYER_STRAIN,               //14
            
            CONDITIONAL_TYPES_END
    };
    
    enum conditional_flags
    {
        INVALID_CONDITIONAL_FLAGS = -1,
            
            FLAG_OR,
            FLAG_AND,
            
            CONDITIONAL_FLAGS_END
    };

    enum { MAX_CONDITION_ENUM_BUFFER = 255 };
    
                    conditional_base( guid ParentGuid );
    virtual        ~conditional_base();
    
    virtual         conditional_types   GetType             ( void )   { return INVALID_CONDITIONAL_TYPES;}
    virtual         const char*         GetTypeName         ( void )   { return "Condtion Base"; } 
    virtual         const char*         GetTypeInfo         ( void )   { return "Base condition class, null funtionality"; } 
    virtual         xbool               Execute             ( trigger_object* pParent ) = 0;    
    virtual			void	            OnEnumProp	        ( prop_enum& rList );
    virtual			xbool	            OnProperty	        ( prop_query& rPropQuery );
                    conditional_flags   GetFlag             ( void ) { return m_Flag; }
                    xbool               GetElse             ( void ) { return m_ElseFlag; }
                    void                SetElse             ( xbool ElseFalg ) { m_ElseFlag = ElseFalg; }

public:
    
    static          conditional_base*   CreateCondition     ( const conditional_types& rType , const guid& rParentGuid );
    virtual         const char*         GetInfo             ( void ){ return "\0"; }

public:
      
    static enum_table<conditional_types>     m_ConditionalAllEnum;                 // Enumeration of the condtion types..
    static enum_table<conditional_types>     m_ConditionalPlayerEnum;              // Enumeration of the condtion types..
    static enum_table<conditional_types>     m_ConditionalAIEnum;                  // Enumeration of the condtion types..
    static enum_table<conditional_types>     m_ConditionalMiscEnum;                // Enumeration of the condtion types..

    static          void                    RegisterCreationFunction( condition_create_function* pCreate );

protected:

    conditional_flags                       m_Flag;             // Logic conditional flags for this condition
    xbool                                   m_ElseFlag;         // Flag if this condition is an else condition
    guid                                    m_ParentGuid;       // Guid of the object which holds this action
    static condition_create_function*       m_CreateHead;
};

//=========================================================================
// ACTION_CREATE_FUNCTION : used for automatic registeration of creation function for actions..
//=========================================================================

struct condition_create_function
{
    condition_create_function(conditional_base::conditional_types Type, conditional_base* (*pCreateCondition)  (guid ParentGuid) )
    {
        m_Type              = Type;
        m_pCreateCondition  = pCreateCondition;
        m_Next              = NULL;

        conditional_base::RegisterCreationFunction( this );
    }

    conditional_base::conditional_types      m_Type;
    conditional_base*                       (*m_pCreateCondition)  (guid ParentGuid); 
    condition_create_function*               m_Next;
};

//=========================================================================
// PLAYER_HEALTH : checks on player health...
//=========================================================================

class  player_health : public conditional_base
{
public:
    
                    player_health                   ( guid ParentGuid );

    virtual         conditional_types   GetType     ( void )   { return TYPE_CONDITION_PLAYER_HEALTH;}
    virtual         const char*         GetTypeName ( void )   { return "Player Health"; } 
    virtual         const char*         GetTypeInfo ( void )   { return "Checks the player health against a value."; } 
    virtual         xbool               Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
    virtual         const char*         GetInfo     ( void );

protected:
    
    enum codes
    { 
        INVALID_CODES = -1,
            
            CODE_GREATER_INCLUSIVE,
            CODE_LESSER_INCLUSIVE,
            CODE_GREATER,
            CODE_LESSER,
            CODE_EQUAL,
            CODE_NOT_EQUAL,
            
            CODES_END
    };
    
protected:

    s32             m_Code;         // Used to determine what type of conditional check to perform
    f32             m_Value;        // Value of Health to check against..

protected:
  
    typedef enum_pair<codes>        code_pair;
    typedef enum_table<codes>       code_table;

    static conditional_base*                Create( guid ParentGuid ) { return new player_health(ParentGuid); }
    static condition_create_function        m_Register;

    static code_pair                        s_PairTable[];
    static code_table                       s_EnumTable;
};

//=========================================================================
// OBJECT_HEALTH : checks on object health...
//=========================================================================

class  object_health : public conditional_base
{
public:
    
                    object_health                   ( guid ParentGuid );

    virtual         conditional_types   GetType     ( void )   { return TYPE_CONDITION_OBJECT_HEALTH;}
    virtual         const char*         GetTypeName ( void )   { return "Object Health"; } 
    virtual         const char*         GetTypeInfo ( void )   { return "Checks the objects health against a value."; } 
    virtual         xbool               Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
    virtual         const char*         GetInfo     ( void );

protected:
    
    enum codes
    { 
        INVALID_CODES = -1,
            
            CODE_GREATER_INCLUSIVE,
            CODE_LESSER_INCLUSIVE,
            CODE_GREATER,
            CODE_LESSER,
            CODE_EQUAL,
            CODE_NOT_EQUAL,
            
            CODES_END
    };
    
protected:

    s32             m_Code;         // Used to determine what type of conditional check to perform
    f32             m_Value;        // Value of Health to check against..
    guid            m_ObjectGuid;   // Object guid to check.

protected:
  
    typedef enum_pair<codes>        code_pair;
    typedef enum_table<codes>       code_table;

    static conditional_base*                Create( guid ParentGuid ) { return new object_health(ParentGuid); }
    static condition_create_function        m_Register;

    static code_pair                        s_PairTable[];
    static code_table                       s_EnumTable;
};

//=========================================================================
// PLAYER_HAS  : checks if player has item...
//=========================================================================

class player_has : public conditional_base
{
public:
                    player_has                      ( guid ParentGuid );

    virtual         conditional_types   GetType     ( void )   { return TYPE_CONDITION_PLAYER_HAS;}
    virtual         const char*         GetTypeName ( void )   { return "Player Has"; } 
    virtual         const char*         GetTypeInfo ( void )   { return "Checks if the player has an item."; }
    virtual         xbool               Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
    virtual         const char*         GetInfo     ( void );
    
protected:
    
    enum codes
    { 
        INVALID_CODES = -1,
            
            CODE_HAS_ITEM,
            CODE_NOT_HAVE_ITEM,
            
            CODES_END
    };

protected:

    inventory_item::inv_type            m_ItemType; // ID of item to see if player has
    s32                                 m_Code;         // Used to determine what type of conditional check to perform

protected:

    static conditional_base*                Create( guid ParentGuid ) { return new player_has(ParentGuid); }
    static condition_create_function        m_Register;
};

//=========================================================================
// PLAYER_STRAIN  : checks if player is or is not a certain strain.
//=========================================================================

class player_strain : public conditional_base
{
public:
                    player_strain                   ( guid ParentGuid );

    virtual         conditional_types   GetType     ( void )   { return TYPE_CONDITION_PLAYER_STRAIN;}
    virtual         const char*         GetTypeName ( void )   { return "Player Strain"; } 
    virtual         const char*         GetTypeInfo ( void )   { return "Checks if player is or is not a certain strain."; }
    virtual         xbool               Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
    virtual         const char*         GetInfo     ( void );
    
protected:
    
    enum codes
    { 
        INVALID_CODES = -1,
            
            CODE_IS_STRAIN,
            CODE_IS_NOT_STRAIN,
            
            CODES_END
    };

protected:

    player::player_mutation_strain  m_StrainToCheck; // Strain we are checking with this condition
    s32                                         m_Code;          // Used to determine what type of conditional check to perform

protected:

    static conditional_base*                Create( guid ParentGuid ) { return new player_strain(ParentGuid); }
    static condition_create_function        m_Register;
};


//=========================================================================
// OBJECT_EXIST : checks if an object exist...
//=========================================================================

class object_exist : public conditional_base
{
public:
                    object_exist                    ( guid ParentGuid );

    virtual         conditional_types   GetType     ( void )   { return TYPE_CONDITION_OBJECT_EXIST;}
    virtual         const char*         GetTypeName ( void )   { return "Object Exist"; } 
    virtual         const char*         GetTypeInfo ( void )   { return "Checks the existance state of an object."; }
    virtual         xbool               Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
    virtual         const char*         GetInfo     ( void );

protected:
    
    enum codes
    { 
        INVALID_CODES = -1,
            
            CODE_DOES_OBJECT_EXIST,
            CODE_DOES_OBJECT_NOT_EXIST,
            
            CODES_END
    };

protected:
        
    guid            m_ObjectGuid;       // Object ID to see if an object exist...
    s32             m_Code;             // Used to determine what type of conditional check to perform

protected:

    static conditional_base*                Create( guid ParentGuid ) { return new object_exist(ParentGuid); }
    static condition_create_function        m_Register;
};

//=========================================================================
// CHECK_STATE_VARS : checks the global variable is..
//=========================================================================

class check_state_vars : public conditional_base
{
    
public:
                   check_state_vars                 ( guid ParentGuid );

    virtual         conditional_types   GetType     ( void )   { return TYPE_CONDITION_CHECK_STATE_VARS;}
    virtual         const char*         GetTypeName ( void )   { return "Check a Global Variable"; }
    virtual         const char*         GetTypeInfo ( void )   { return "Checks a global variable state."; }
    virtual         xbool               Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
    virtual         const char*         GetInfo     ( void );
        
protected:
    
    enum codes
    { 
        INVALID_CODES = -1,
            
            CODE_GREATER_INCLUSIVE,
            CODE_LESSER_INCLUSIVE,
            CODE_GREATER,
            CODE_LESSER,
            CODE_EQUAL,
            CODE_NOT_EQUAL,
            
            CODES_END
    };
    
protected:

    s32                         m_Code;                                     // Used to determine what type of conditional check to perform
    sml_string                  m_VariableName;                             // Name of the variable
    xhandle                     m_VarHandle;                                // Gloabal variable handle..
    var_mngr::variable_types    m_Type;                                     // Type of variable...
    s32                         m_VarRaw;                                   // Raw data of variable is cast into proper type (f32, u32, s32, xbool) at runtime...

protected:
   
    typedef enum_pair<codes>        code_pair;
    typedef enum_table<codes>       code_table;

    static conditional_base*                Create( guid ParentGuid ) { return new check_state_vars(ParentGuid); }
    static condition_create_function        m_Register;

    static code_pair                        s_PairTable[];
    static code_table                       s_EnumTable;

    static code_pair                        s_BoolPairTable[];
    static code_table                       s_BoolEnumTable;
};

//=========================================================================
// CHECK_SCRIPT : checks on the state of a script
//=========================================================================

class check_script : public conditional_base
{
public:
                    check_script                    ( guid ParentGuid );

    virtual         conditional_types   GetType     ( void )   { return TYPE_CONDITION_CHECK_SCRIPT;}
    virtual         const char*         GetTypeName ( void )   { return "Check a Script"; } 
    virtual         const char*         GetTypeInfo ( void )   { return "Checks the return value of an executed script."; }
    virtual         xbool               Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
    virtual         const char*         GetInfo     ( void );
    
   
protected:
    
    enum codes
    { 
        INVALID_CODES = -1,
            
            CODE_SCRIPT_EVAULATE,
            
            CODES_END
    };
     
protected:

    s32             m_ScriptID;     // Script ID to check
    s32             m_Code;         // Used to determine what type of conditional check to perform

protected:

    static conditional_base*                Create( guid ParentGuid ) { return new check_script(ParentGuid); }
    static condition_create_function        m_Register;
};

//=========================================================================
// CHECK_TRIGGER_STATE : checks in the state of an existing trigger
//=========================================================================

class check_trigger_state : public conditional_base
{
public:
                    check_trigger_state             ( guid ParentGuid );

    virtual         conditional_types   GetType     ( void )   { return TYPE_CONDITION_CHECK_TRIGGER_STATE;}
    virtual         const char*         GetTypeName ( void )   { return "Check a Trigger"; } 
    virtual         const char*         GetTypeInfo ( void )   { return "Checks the return value of an executed trigger."; }
    virtual         xbool               Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
    virtual         const char*         GetInfo     ( void );
  
protected:
    
    enum codes
    { 
        INVALID_CODES = -1,
            
            CODE_TRIGGER_EVAULTATE,
            
            CODES_END
    };

protected:
    
    guid            m_TriggerGuid;      // GUID of trigger to watch..
    s32             m_Code;             // Used to determine what type of conditional check to perform

protected:

    static conditional_base*                Create( guid ParentGuid ) { return new check_trigger_state(ParentGuid); }
    static condition_create_function        m_Register;
};

//=========================================================================
// RANDOM_CHANCE : True based on a random chance..
//=========================================================================

class random_chance : public conditional_base
{
public:
                    random_chance                   ( guid ParentGuid );

    virtual         conditional_types   GetType     ( void )   { return TYPE_CONDITION_RANDOM_CHANCE;}
    virtual         const char*         GetTypeName ( void )   { return "Random Chance"; } 
    virtual         const char*         GetTypeInfo ( void )   { return "Rolls a dice to see if this comes out TRUE."; }
    virtual         xbool               Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
    virtual         const char*         GetInfo     ( void );
  
protected:
    
    enum codes
    { 
        INVALID_CODES = -1,
            
            CODE_RANDOM_CHANCE,
            
            CODES_END
    };

protected:
    
    f32             m_RandomPercent;        // Random Chance of a True event...
    s32             m_Code;                 // Used to determine what type of conditional check to perform};

protected:

    static conditional_base*                Create( guid ParentGuid ) { return new random_chance(ParentGuid); }
    static condition_create_function        m_Register;
};

//=========================================================================
// ON_THIS_NPC : Only activates if a particular NPC is in the volume
//=========================================================================

class on_this_npc : public conditional_base
{
public:
                    on_this_npc                    ( guid ParentGuid );

    virtual         conditional_types   GetType     ( void )   { return TYPE_CONDITION_ONLY_ON_THIS_NPC;}
    virtual         const char*         GetTypeName ( void )   { return "On This Npc"; } 
    virtual         const char*         GetTypeInfo ( void )   { return "Checks if the triggering object is this NPC."; }
    virtual         xbool               Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
    virtual         const char*         GetInfo     ( void );
  
protected:
    
    enum codes
    { 
        INVALID_CODES = -1,
            
            CODE_ON_THIS_NPC,
            
            CODES_END
    };

protected:
    
    guid            m_NPC_Guid;             // Guid of the npc of intrest
    s32             m_Code;                 // Used to determine what type of conditional check to perform};

protected:

    static conditional_base*                Create( guid ParentGuid ) { return new on_this_npc(ParentGuid); }
    static condition_create_function        m_Register;
};

//=========================================================================
// ON_THIS_ITEM : Only activates if a particular item is in the volume
//=========================================================================

class on_this_item : public conditional_base
{
public:
                    on_this_item                    ( guid ParentGuid );

    virtual         conditional_types   GetType     ( void )   { return TYPE_CONDITION_ONLY_ON_THIS_ITEM;}
    virtual         const char*         GetTypeName ( void )   { return "On This Item"; } 
    virtual         const char*         GetTypeInfo ( void )   { return "Checks if the triggering actor has a particular item."; }
    virtual         xbool               Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
    virtual         const char*         GetInfo     ( void );
  
protected:
    
    enum codes
    { 
        INVALID_CODES = -1,
            
            CODE_ON_THIS_ITEM,
            
            CODES_END
    };

protected:
    
    s32             m_ItemID;               // ID of item of intrest
    s32             m_Code;                 // Used to determine what type of conditional check to perform};

protected:

    static conditional_base*                Create( guid ParentGuid ) { return new on_this_item(ParentGuid); }
    static condition_create_function        m_Register;
};

//=========================================================================
// COUNT_THINGS : Count various things 
//=========================================================================

class count_things : public conditional_base
{
public:
                    count_things                    ( guid ParentGuid );

    virtual         conditional_types   GetType     ( void )   { return TYPE_CONDITION_COUNT_THINGS;}
    virtual         const char*         GetTypeName ( void )   { return "Count Things"; }
    virtual         const char*         GetTypeInfo ( void )   { return "Count various things in the world, NPC, or Player."; }
    virtual         xbool               Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
    virtual         const char*         GetInfo     ( void );
  
protected:
    
    enum codes
    { 
        INVALID_CODES = -1,
            
            CODE_COUNT_ITEM_ON_PLAYER,
            CODE_COUNT_ITEM_IN_WORLD,
            CODE_COUNT_ITEM_IN_VOLUME,
            CODE_COUNT_NPCS_IN_VOLUME,
            
            CODES_END
    };
 
    enum logic_codes
    { 
        INVALID_LOGIC_CODES = -1,
            
            CODE_GREATER_INCLUSIVE,
            CODE_LESSER_INCLUSIVE,
            CODE_GREATER,
            CODE_LESSER,
            CODE_EQUAL,
            CODE_NOT_EQUAL,
            
            LOGIC_CODES_END
    };

protected:
    
    s32             m_Value;                // The value to test the count against
    s32             m_Code;                 // Used to determine what type of conditional check to perform};
    s32             m_LogicCode;

protected:
   
    typedef enum_pair<codes>        code_pair;
    typedef enum_table<codes>       code_table;
 
    typedef enum_pair<logic_codes>        logic_code_pair;
    typedef enum_table<logic_codes>       logic_code_table;

    static conditional_base*                Create( guid ParentGuid ) { return new count_things(ParentGuid); }
    static condition_create_function        m_Register;

    static code_pair                        s_PairTable[];
    static code_table                       s_EnumTable;

    static logic_code_pair                  s_LogicPairTable[];
    static logic_code_table                 s_LogicEnumTable;
};

//=========================================================================
// CHECK_TIMER : Checks the timmer against some value
//=========================================================================

class check_timer : public conditional_base
{
public:
                    check_timer                     ( guid ParentGuid );

    virtual         conditional_types   GetType     ( void )   { return TYPE_CONDITION_CHECK_TIMMER;}
    virtual         const char*         GetTypeName ( void )   { return "Check Timer"; } 
    virtual         const char*         GetTypeInfo ( void )   { return "Checks the current time of a global timer."; }
    virtual         xbool               Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
    virtual         const char*         GetInfo     ( void );
  
protected:
    
    enum codes
    { 
        INVALID_CODES = -1,
            
            CODE_GREATER_INCLUSIVE,
            CODE_LESSER_INCLUSIVE,
            CODE_GREATER,
            CODE_LESSER,
            CODE_EQUAL,
            CODE_NOT_EQUAL,
            
            CODES_END
    };

protected:
    
    sml_string        m_TimerName;          //Name of Timer to operate upon
    xhandle           m_TimerHandle;        //Handle to the timer
    f32               m_Value;              //Value to test against
    s32               m_Code;               //Action code
    

protected:

    typedef enum_pair<codes>        code_pair;
    typedef enum_table<codes>       code_table;

    static conditional_base*                Create( guid ParentGuid ) { return new check_timer(ParentGuid); }
    static condition_create_function        m_Register;

    static code_pair                        s_PairTable[];
    static code_table                       s_EnumTable;
};

//=========================================================================
// COUNTER : Counts the number of times its been called
//=========================================================================

class check_counter : public conditional_base
{
public:
                    check_counter                   ( guid ParentGuid );

    virtual         conditional_types   GetType     ( void )   { return TYPE_CONDITION_COUNTER;}
    virtual         const char*         GetTypeName ( void )   { return "Counter"; } 
    virtual         const char*         GetTypeInfo ( void )   { return "Checks a internal counter of the number of times this condition has been executed."; }
    virtual         xbool               Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
    virtual         const char*         GetInfo     ( void );
  
protected:
    
    enum codes
    { 
        INVALID_CODES = -1,
            
            CODE_GREATER_INCLUSIVE,
            CODE_LESSER_INCLUSIVE,
            CODE_GREATER,
            CODE_LESSER,
            CODE_EQUAL,
            CODE_NOT_EQUAL,
            
            CODES_END
    };

protected:
    
    s32               m_Value;              //Value to test against
    s32               m_Count;              //Execution Count
    s32               m_Code;               //Action code
    
protected:

    typedef enum_pair<codes>        code_pair;
    typedef enum_table<codes>       code_table;

    static conditional_base*                Create( guid ParentGuid ) { return new check_counter(ParentGuid); }
    static condition_create_function        m_Register;

    static code_pair                        s_PairTable[];
    static code_table                       s_EnumTable;
};

//=========================================================================
// ON_THIS_BUTTON : Only activates if a particular item is in the volume
//  NOTE:  This trigger will only work in a spatial trigger.
//=========================================================================

class on_this_button : public conditional_base
{
public:
                    on_this_button                  ( guid ParentGuid );

    virtual         conditional_types   GetType     ( void )   { return TYPE_CONDITION_ON_THIS_BUTTON; }
    virtual         const char*         GetTypeName ( void )   { return "On This Button"; } 
    virtual         const char*         GetTypeInfo ( void )   { return "Checks if a certain button has been pressed."; }
    virtual         xbool               Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
    virtual         const char*         GetInfo     ( void );
  
protected:

    static conditional_base*                Create( guid ParentGuid ) { return new on_this_button(ParentGuid); }
    static condition_create_function        m_Register;

    ingame_pad::locgical_id                 m_ButtonID;
};


#endif
