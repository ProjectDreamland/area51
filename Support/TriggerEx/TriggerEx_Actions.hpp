///////////////////////////////////////////////////////////////////////////////
//
//  TriggerEx_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGEREX_ACTIONS_
#define _TRIGGEREX_ACTIONS_

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_types.hpp"
#include "..\Support\Globals\Global_Variables_Manager.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "Auxiliary\MiscUtils\PropertyEnum.hpp"

class trigger_ex_object;
class character_task;
class object_affecter;

//=========================================================================
// ACTIONS_BASE
//=========================================================================

struct action_ex_create_function;

class actions_ex_base : public prop_interface
{  

public:

    CREATE_RTTI_BASE( actions_ex_base );

    enum action_ex_flags
    {
        NULL_ACTION_FLAG            = 0,
        ELSE_ACTION_FLAG            = BIT(1),
        PRE_ACTIVATE_ACTION_FLAG    = BIT(2),
        POST_ACTIVATE_ACTION_FLAG   = BIT(3)
    };
    
    //Note ** Do not remove depreciated enums from list. The trigger object saves out a list
    //of actions it contains and uses the enum cast to a s32 to identify the action. Changing
    //the order of the enums or removing enums in the middle will break this listing..**//

    enum action_ex_types
    {
        INVALID_ACTION_TYPES = -1,
 
        TYPE_ACTION_DAMAGE_OBJECT,                   //0                
        TYPE_ACTION_DESTORY_OBJECT,                  //1
        TYPE_ACTION_OBJECT_ACTIVATION,               //2
        TYPE_ACTION_DOOR_LOGIC,                      //3
        TYPE_ACTION_CREATE_TEMPLATE,                 //4
        TYPE_ACTION_MOVE_OBJECT,                     //5
        TYPE_ACTION_PLAYER_INVENTORY,                //6
        TYPE_ACTION_PLAYER_CAMERA_SHAKE,             //7
        TYPE_ACTION_MUSIC_INTENSITY,                 //8
        TYPE_ACTION_DISPLAY_TEXT,                    //9
        TYPE_ACTION_MOVE_OBJECT_RELATIVE,            //10
        TYPE_ACTION_PORTAL_ACTIVATE,                 //11
        TYPE_ACTION_PLAYER_HUD,                      //12
        TYPE_ACTION_SAVE_GAME,                       //13
        TYPE_ACTION_FADE_GEOMETRY,                   //14
        TYPE_ACTION_SCREEN_FADE,                     //15
        TYPE_ACTION_MAN_TURRET,                      //16
        TYPE_ACTION_EXIT_TURRET,                     //17
        TYPE_ACTION_MISSION_FAILED,                  //18
 
        TYPE_ACTION_AI_ATTACK_GUID = 20,             //20
        TYPE_ACTION_AI_PATHTO_GUID,                  //21
        TYPE_ACTION_AI_LOOKAT_GUID,                  //22
        TYPE_ACTION_AI_DIALOG_LINE,                  //23
        TYPE_ACTION_AI_PLAY_ANIM,                    //24
        TYPE_ACTION_AI_SEARCHTO_GUID,                //25
        TYPE_ACTION_AI_INVENTORY,                    //26
        TYPE_ACTION_AI_NAV_ACTIVATION,               //27
        TYPE_ACTION_AI_DEATH,                        //28

        TYPE_ACTION_AFFECT_GLOBAL_VAR = 50,          //50
        TYPE_ACTION_AFFECT_PROPERTY,                 //51

        TYPE_META_LABEL = 100,                       //100
        TYPE_META_DELAY,                             //101
        TYPE_META_BLOCK,                             //102
        TYPE_META_GOTO,                              //103
        TYPE_META_BREAKPOINT,                        //104
        TYPE_META_CINEMA_BLOCK,                      //105

        TYPE_ACTION_LOAD_LEVEL = 120,                //120
        TYPE_ACTION_CHECKPOINT,                      //121

        TYPE_ACTION_PLAY_2D_SOUND = 130,             //130
        TYPE_ACTION_CHANGE_PERCEPTION,               //131

        ACTION_TYPES_END
    };
    
                    actions_ex_base                         ( guid ParentGuid );
    virtual        ~actions_ex_base                         ( void );

    virtual         action_ex_types     GetType             ( void ) { return INVALID_ACTION_TYPES;}
    virtual         const char*         GetTypeInfo         ( void ) { return "Base action class, null funtionality"; } 
    virtual         const char*         GetDescription      ( void ) { return "\0"; }
    virtual         xbool               Execute             ( f32 DeltaTime ) = 0;    
    virtual			void	            OnEnumProp	        ( prop_enum& rList );
    virtual			xbool	            OnProperty	        ( prop_query& rPropQuery );
    virtual         void                OnActivate          ( xbool Flag ) { (void)Flag; }

#ifdef X_EDITOR
    // Implement any of these functions in your derived classes for property validation
    virtual         object_affecter*    GetObjectRef0 ( xstring& Desc )                    { (void)Desc; return NULL; }
    virtual         object_affecter*    GetObjectRef1 ( xstring& Desc )                    { (void)Desc; return NULL; }
    virtual         s32*                GetAnimRef    ( xstring& Desc, s32& AnimName )     { (void)Desc; (void)AnimName; return NULL; }
    virtual         rhandle<char>*      GetSoundRef   ( xstring& Desc, s32& SoundName )    { (void)Desc; (void)SoundName; return NULL; }
    virtual         s32*                GetGlobalRef  ( xstring& Desc )                    { (void)Desc; return NULL; }
    virtual         s32*                GetPropertyRef( xstring& Desc, s32& PropertyType ) { (void)Desc; (void)PropertyType; return NULL; }
    virtual         s32*                GetTemplateRef( xstring& Desc )                    { (void)Desc;  return NULL; }
    virtual         guid*               GetGuidRef0   ( xstring& Desc )                    { (void)Desc;  return NULL; }
    virtual         guid*               GetGuidRef1   ( xstring& Desc )                    { (void)Desc;  return NULL; }
#endif

#ifndef X_RETAIL
    virtual         void                OnDebugRender       ( s32 Index ) { (void) Index; /*no-op*/ }
#endif // X_RETAIL
    
    inline          xbool               GetElse             ( void ) { return m_ElseFlag; }
    inline          void                SetElse             ( xbool ElseFlag ) { m_ElseFlag = ElseFlag; }
    inline          guid                GetTriggerGuid      ( void ) { return m_TriggerGuid; }
    inline          void                SetTaskOwner        ( character_task* pTask ) { m_pTaskOwner = pTask; }

    inline          xbool               WasExecuteAnError   ( void ) { return m_bErrorInExecute; }
    inline          void                ResetErrorRegister  ( void ) { m_bErrorInExecute = FALSE; }
    
                    trigger_ex_object*  GetTriggerPtr       ( void );
                    xbool               RetryOnError        ( void );

#ifdef X_EDITOR
    virtual         void                EditorPreGame       ( void ) {};
#endif // X_EDITOR

public:
    
    static          actions_ex_base*        CreateAction    ( const action_ex_types& rType , const guid& rParentGuid );
    
public:
 
    static enum_table<action_ex_types>      m_ActionsAllEnum;               // Enumeration of the action types..
    static enum_table<action_ex_types>      m_ActionsGeneralEnum;           // Enumeration of the action types..
    static enum_table<action_ex_types>      m_ActionsAIEnum;                // Enumeration of the action types..
    static enum_table<action_ex_types>      m_ActionsPlayerEnum;            // Enumeration of the action types..
    static enum_table<action_ex_types>      m_ActionsSpecificEnum;          // Enumeration of the action types..
    static enum_table<action_ex_types>      m_ActionsMetaEnum;              // Enumeration of the action types..
    static enum_table<action_ex_types>      m_ActionsInterfaceEnum;         // Enumeration of the action types..
    static enum_table<action_ex_types>      m_ActionsTaskEnum;              // Enumeration of the action types..

    static          void                    RegisterCreationFunction( action_ex_create_function* pCreate );

protected:

                    vector3                 GetPositionOwner( void );
               
protected:
    
   guid                                     m_TriggerGuid;      // Guid of the object which holds this action
   character_task*                          m_pTaskOwner;       // pointer to the object which holds this action
   xbool                                    m_ElseFlag;         // Flag if this action is an else action
   action_ex_flags                          m_Flags;            // Flags which stores the various properties of this action
   xbool                                    m_bErrorInExecute;  // was an error detected?

protected:
    
   static action_ex_create_function*        m_CreateHead;       // Autotmatic registration list head node

};

//=========================================================================
// ACTION_CREATE_FUNCTION : used for automatic registeration of creation function for actions..
//=========================================================================

typedef actions_ex_base* create_action_ex_fn ( guid ParentGuid );

struct action_ex_create_function
{
    action_ex_create_function( actions_ex_base::action_ex_types Type, create_action_ex_fn* pCreateAction )
    {
        m_Type          = Type;
        m_pCreateAction = pCreateAction;
        m_Next          = NULL;
    }

    actions_ex_base::action_ex_types    m_Type;
    create_action_ex_fn*                m_pCreateAction;
    action_ex_create_function*          m_Next;
};

template < class ActionExClass > 
    struct automatic_action_ex_registeration
{
    automatic_action_ex_registeration( void )
    {
        static action_ex_create_function m_CreationObject( ActionExClass::GetTypeStatic(), Create );

        actions_ex_base::RegisterCreationFunction( &m_CreationObject );
    }
    
    static actions_ex_base*  Create( guid ParentGuid ) { return new ActionExClass(ParentGuid); }
};

#endif





















