///////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"
#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"
#include "..\Support\Characters\Character.hpp"

#include "..\Support\Trigger\Actions\activate_object.hpp"
#include "..\Support\Trigger\Actions\safe_spot_trigger.hpp"
#include "..\Support\Trigger\Actions\ai_modify_behavior.hpp"
#include "..\Support\Trigger\Actions\ai_modify_behavior_targeted.hpp"
#include "..\Support\Trigger\Actions\ai_move_to.hpp"
#include "..\Support\Trigger\Actions\change_player_health.hpp"
#include "..\Support\Trigger\Actions\change_state_vars.hpp"
#include "..\Support\Trigger\Actions\create_object_from_template.hpp"
#include "..\Support\Trigger\Actions\deactivate_object.hpp"
#include "..\Support\Trigger\Actions\destory_object.hpp"
#include "..\Support\Trigger\Actions\destory_this_trigger.hpp"
#include "..\Support\Trigger\Actions\game_music_intensity.hpp"
#include "..\Support\Trigger\Actions\give_player_item.hpp"
#include "..\Support\Trigger\Actions\move_object.hpp"
#include "..\Support\Trigger\Actions\play_script.hpp"
#include "..\Support\Trigger\Actions\play_sound.hpp"
#include "..\Support\Trigger\Actions\set_timer.hpp"
#include "..\Support\Trigger\Actions\change_object_health.hpp"
#include "..\Support\Trigger\Actions\lock_player_view.hpp"
#include "..\Support\Trigger\Actions\play_conversation.hpp"
#include "..\Support\Trigger\Actions\ai_activate_task.hpp"
#include "..\Support\Trigger\Actions\cause_damage.hpp"
#include "..\Support\Trigger\Actions\change_player_strain.hpp"
#include "..\Support\Trigger\Actions\Door_Logic.hpp"
#include "..\Support\Trigger\Actions\open_and_lock_door.hpp"
#include "..\Support\Trigger\Actions\close_and_lock_door.hpp"
#include "..\Support\Trigger\Actions\return_door_to_normal.hpp"
#include "..\Support\Trigger\Actions\set_actor_friends.hpp"
#include "..\Support\Trigger\Actions\set_actor_faction.hpp"

#include "AudioMgr\AudioMgr.hpp"
#include "..\Support\Templatemgr\TemplateMgr.hpp"
#include "Entropy.hpp"
#include "Music_mgr\Music_mgr.hpp"

//=========================================================================
// STATIC VARIABLES
//=========================================================================

const   f32   k_rand_draw_displace_amt = 1.0f;

static const xcolor s_ActivateColor         (0,255,0);
static const xcolor s_DeactivateColor       (255,0,0);
static const xcolor s_MoveColor             (255,0,255);
static const xcolor s_DestroyColor          (255,255,255);
static const xcolor s_AIColor               (0,255,255);
static const xcolor s_CreateTemplateColor   (255,255,255);

static const f32    s_SphereRadius  = 100.0f;

typedef enum_pair<actions_base::action_types> action_enum_pair;

//****s_ActionsAllEnumTable **** this table should not use the BuildString() as it exceeds the 255 char limit...
static action_enum_pair s_ActionsAllEnumTable[] = 
{
        action_enum_pair("PLAY_SOUND",                     actions_base::TYPE_ACTION_PLAY_SOUND),
        action_enum_pair("ACTIVATE_OBJECT",                actions_base::TYPE_ACTION_ACTIVATE_OBJECT),
        action_enum_pair("DEACTIVATE_OBJECT",              actions_base::TYPE_ACTION_DEACTIVATE_OBJECT),
        action_enum_pair("CREATE_OBJECT_FROM_TEMPLATE",    actions_base::TYPE_ACTION_CREATE_OBJECT_FROM_TEMPLATE),
        action_enum_pair("MOVE_OBJECT",                    actions_base::TYPE_ACTION_MOVE_OBJECT),
        action_enum_pair("CHANGE_PLAYER_HEALTH",           actions_base::TYPE_ACTION_CHANGE_PLAYER_HEALTH),
        action_enum_pair("CHANGE_STATE_VARIABLE",          actions_base::TYPE_ACTION_CHANGE_STATE_VARIABLE),
        action_enum_pair("GIVE_PLAYER_ITEM",               actions_base::TYPE_ACTION_GIVE_PLAYER_ITEM),
        action_enum_pair("PLAY_SCRIPT",                    actions_base::TYPE_ACTION_PLAY_SCRIPT),
        action_enum_pair("DESTORY_OBJECT",                 actions_base::TYPE_ACTION_DESTORY_OBJECT),
        action_enum_pair("AI_MOVE_TO",                     actions_base::TYPE_ACTION_AI_MOVE_TO),
        action_enum_pair("AI_MODIFY_BEHAVIOR",             actions_base::TYPE_ACTION_AI_MODIFY_BEHAVIOR),
        action_enum_pair("AI_MODIFY_BEHAVIOR_TARGETED",    actions_base::TYPE_ACTION_AI_MODIFY_BEHAVIOR_TARGETED), 
        action_enum_pair("SET_TIMER",                      actions_base::TYPE_ACTION_SET_TIMER),
        action_enum_pair("KILL_THIS_TRIGGER",              actions_base::TYPE_ACTION_DESTORY_THIS_TRIGGER),
        action_enum_pair("MUSIC_INTENSITY",                actions_base::TYPE_ACTION_MUSIC_INTENSITY),
        action_enum_pair("CHANGE_OBJECT_HEALTH",           actions_base::TYPE_ACTION_CHANGE_OBJECT_HEALTH),
        action_enum_pair("LOCK_PLAYER_VIEW",               actions_base::TYPE_ACTION_LOCK_PLAYER_VIEW),
        action_enum_pair("PLAY_CINEMATIC_SCRIPT",          actions_base::TYPE_ACTION_PLAY_CINEMATIC_SCRIPT),
        action_enum_pair("SAFE_SPOT_TRIGGER",              actions_base::TYPE_ACTION_SAFE_SPOT_TRIGGER ),
        action_enum_pair("PLAY_CONVERSATION",              actions_base::TYPE_ACTION_PLAY_CONVERSATION),
        action_enum_pair("ACTIVATE_AI_TASK",               actions_base::TYPE_ACTION_ACTIVATE_TASK),       
        action_enum_pair("CAUSE_DAMAGE",                   actions_base::TYPE_ACTION_CAUSE_DAMAGE),       
        action_enum_pair("CHANGE_STRAIN",                  actions_base::TYPE_ACTION_CHANGE_PLAYER_STRAIN),
        action_enum_pair("DOOR_LOGIC",                     actions_base::TYPE_ACTION_DOOR_LOGIC),

        action_enum_pair("OPEN_AND_LOCK_DOOR",             actions_base::TYPE_ACTION_OPEN_AND_LOCK_DOOR),
        action_enum_pair("CLOSE_AND_LOCK_DOOR",            actions_base::TYPE_ACTION_CLOSE_AND_LOCK_DOOR),
        action_enum_pair("RESTORE_DOOR",                   actions_base::TYPE_ACTION_RESTORE_DOOR),
        action_enum_pair("DOOR_LOGIC",                     actions_base::TYPE_ACTION_DOOR_LOGIC),

        action_enum_pair("SET_FRIENDS",                    actions_base::TYPE_ACTION_SET_ACTOR_FRIENDS ),
        action_enum_pair("SET_FACTION",                    actions_base::TYPE_ACTION_SET_ACTOR_FACTION ),

        action_enum_pair( k_EnumEndStringConst,            actions_base::INVALID_ACTION_TYPES) //**MUST BE LAST**//
};

//Break down sub enum tables to get around the 255 char limit for enum strings..
static action_enum_pair s_ActionsDoorEnumTable[] = 
{
        action_enum_pair("OPEN_AND_LOCK_DOOR",             actions_base::TYPE_ACTION_OPEN_AND_LOCK_DOOR),
        action_enum_pair("CLOSE_AND_LOCK_DOOR",            actions_base::TYPE_ACTION_CLOSE_AND_LOCK_DOOR),
        action_enum_pair("RESTORE_DOOR",                   actions_base::TYPE_ACTION_RESTORE_DOOR),
        action_enum_pair("DOOR_LOGIC",                     actions_base::TYPE_ACTION_DOOR_LOGIC),

        action_enum_pair( k_EnumEndStringConst,            actions_base::INVALID_ACTION_TYPES) //**MUST BE LAST**//
};

static action_enum_pair s_ActionsMiscEnumTable[] = 
{
        action_enum_pair("PLAY_SOUND",                     actions_base::TYPE_ACTION_PLAY_SOUND),
        action_enum_pair("PLAY_CONVERSATION",              actions_base::TYPE_ACTION_PLAY_CONVERSATION),
        action_enum_pair("ACTIVATE_OBJECT",                actions_base::TYPE_ACTION_ACTIVATE_OBJECT),
        action_enum_pair("DEACTIVATE_OBJECT",              actions_base::TYPE_ACTION_DEACTIVATE_OBJECT),
        action_enum_pair("CREATE_OBJECT_FROM_TEMPLATE",    actions_base::TYPE_ACTION_CREATE_OBJECT_FROM_TEMPLATE),
        action_enum_pair("MOVE_OBJECT",                    actions_base::TYPE_ACTION_MOVE_OBJECT),
        action_enum_pair("PLAY_SCRIPT",                    actions_base::TYPE_ACTION_PLAY_SCRIPT),
        action_enum_pair("DESTORY_OBJECT",                 actions_base::TYPE_ACTION_DESTORY_OBJECT),
        action_enum_pair("KILL_THIS_TRIGGER",              actions_base::TYPE_ACTION_DESTORY_THIS_TRIGGER),
        action_enum_pair("MUSIC_INTENSITY",                actions_base::TYPE_ACTION_MUSIC_INTENSITY),    
        action_enum_pair("CHANGE_OBJECT_HEALTH",           actions_base::TYPE_ACTION_CHANGE_OBJECT_HEALTH),
        action_enum_pair("PLAY_CINEMATIC_SCRIPT",          actions_base::TYPE_ACTION_PLAY_CINEMATIC_SCRIPT),
        action_enum_pair("CAUSE_DAMAGE",                   actions_base::TYPE_ACTION_CAUSE_DAMAGE),       

        action_enum_pair( k_EnumEndStringConst,            actions_base::INVALID_ACTION_TYPES) //**MUST BE LAST**//
};

static action_enum_pair s_ActionsAIEnumTable[] = 
{
        action_enum_pair("AI_MOVE_TO",                     actions_base::TYPE_ACTION_AI_MOVE_TO),
        action_enum_pair("AI_MODIFY_BEHAVIOR",             actions_base::TYPE_ACTION_AI_MODIFY_BEHAVIOR),
        action_enum_pair("AI_MODIFY_BEHAVIOR_TARGETED",    actions_base::TYPE_ACTION_AI_MODIFY_BEHAVIOR_TARGETED),
        action_enum_pair("ACTIVATE_AI_TASK",               actions_base::TYPE_ACTION_ACTIVATE_TASK),       
        
        action_enum_pair( k_EnumEndStringConst,            actions_base::INVALID_ACTION_TYPES) //**MUST BE LAST**//
};

static action_enum_pair s_ActionsPlayerEnumTable[] = 
{
        action_enum_pair("CHANGE_PLAYER_HEALTH",           actions_base::TYPE_ACTION_CHANGE_PLAYER_HEALTH),
        action_enum_pair("GIVE_PLAYER_ITEM",               actions_base::TYPE_ACTION_GIVE_PLAYER_ITEM),
        action_enum_pair("LOCK_PLAYER_VIEW",               actions_base::TYPE_ACTION_LOCK_PLAYER_VIEW),
        action_enum_pair("SAFE_SPOT_TRIGGER",              actions_base::TYPE_ACTION_SAFE_SPOT_TRIGGER ),
        action_enum_pair("CHANGE_STRAIN",                  actions_base::TYPE_ACTION_CHANGE_PLAYER_STRAIN),       
 
        action_enum_pair( k_EnumEndStringConst,            actions_base::INVALID_ACTION_TYPES) //**MUST BE LAST**//
};

static action_enum_pair s_ActionsVariablesEnumTable[] = 
{
        action_enum_pair("CHANGE_STATE_VARIABLE",          actions_base::TYPE_ACTION_CHANGE_STATE_VARIABLE),
        action_enum_pair("SET_TIMER",                      actions_base::TYPE_ACTION_SET_TIMER),
    
        action_enum_pair( k_EnumEndStringConst,            actions_base::INVALID_ACTION_TYPES) //**MUST BE LAST**//
};

static action_enum_pair s_ActionsFactionEnumTable[] = 
{
        action_enum_pair("SET_FRIENDS",           actions_base::TYPE_ACTION_SET_ACTOR_FRIENDS ),
        action_enum_pair("SET_FACTION",           actions_base::TYPE_ACTION_SET_ACTOR_FACTION ),

        action_enum_pair( k_EnumEndStringConst,            actions_base::INVALID_ACTION_TYPES) //**MUST BE LAST**//
};

enum_table<actions_base::action_types>  actions_base::m_ActionsAllEnum(         s_ActionsAllEnumTable       ); 
enum_table<actions_base::action_types>  actions_base::m_ActionsMiscEnum(        s_ActionsMiscEnumTable      );              
enum_table<actions_base::action_types>  actions_base::m_ActionsAIEnum(          s_ActionsAIEnumTable        );               
enum_table<actions_base::action_types>  actions_base::m_ActionsPlayerEnum(      s_ActionsPlayerEnumTable    );           
enum_table<actions_base::action_types>  actions_base::m_ActionsVariablesEnum(   s_ActionsVariablesEnumTable );      
enum_table<actions_base::action_types>  actions_base::m_ActionsDoorEnum(        s_ActionsDoorEnumTable );      
enum_table<actions_base::action_types>  actions_base::m_ActionsFactionsEnum(    s_ActionsFactionEnumTable );      
       
action_create_function*  actions_base::m_CreateHead = NULL;               

//=========================================================================
// AUTOMATIC REGISTERATION..
//=========================================================================

automatic_action_registeration <activate_object>                Register_activate_object;
automatic_action_registeration <ai_modify_behavior>             Register_ai_modify_behavior;
automatic_action_registeration <ai_modify_behavior_targeted>    Register_ai_modify_behavior_targeted;
automatic_action_registeration <ai_move_to>                     Register_ai_move_to;
automatic_action_registeration <change_object_health>           Register_change_object_health;
automatic_action_registeration <change_player_health>           Register_change_player_health;
automatic_action_registeration <change_state_vars>              Register_change_state_vars;
automatic_action_registeration <create_object_from_template>    Register_create_object_from_template;
automatic_action_registeration <deactivate_object>              Register_deactivate_object;
automatic_action_registeration <destory_object>                 Register_destory_object;
automatic_action_registeration <destory_this_trigger>           Register_destory_this_trigger;
automatic_action_registeration <game_music_intensity>           Register_game_music_intensity;
automatic_action_registeration <give_player_item>               Register_give_player_item;
automatic_action_registeration <lock_player_view>               Register_lock_player_view;
automatic_action_registeration <move_object>                    Register_move_object;
automatic_action_registeration <play_script>                    Register_play_script;
automatic_action_registeration <play_sound>                     Register_play_sound;
automatic_action_registeration <set_timer>                      Register_set_timer;
automatic_action_registeration <safe_spot_trigger>              Register_safe_spot_trigger;
automatic_action_registeration <play_conversation>              Register_play_conversation;
automatic_action_registeration <ai_activate_task>               Register_ai_activate_task;
automatic_action_registeration <cause_damage>                   Register_cause_damage;
automatic_action_registeration <change_player_strain>           Register_change_strain;
automatic_action_registeration <open_and_lock_door>             Register_open_and_lock_door;
automatic_action_registeration <close_and_lock_door>            Register_close_and_lock_door;
automatic_action_registeration <return_door_to_normal>          Register_return_door_to_normal;
automatic_action_registeration <door_logic>                     Register_door_logic;
automatic_action_registeration <set_actor_friends>              Register_set_actor_friends;
automatic_action_registeration <set_actor_faction>              Register_set_actor_faction;

//=========================================================================
// STATIC FUNCTIONS
//=========================================================================

//Using the automatic registeration, walk through the action list..

actions_base* actions_base::CreateAction ( const action_types& rType , const guid& rParentGuid )
{
    if ( rType ==  actions_base::INVALID_ACTION_TYPES )
        return NULL;
        
    if (actions_base::m_CreateHead == NULL)
        return NULL;
    
    actions_base* pNewAction = NULL;

    action_create_function* pCurrent = actions_base::m_CreateHead;
    action_create_function* pMatched = NULL;

    while( pCurrent != NULL )
    {
        if (pCurrent->m_Type == rType)
        {
            pMatched = pCurrent;
            break;
        }

        pCurrent = pCurrent->m_Next;
    }

    if (pMatched == NULL)
        return NULL;

    pNewAction = pMatched->m_pCreateAction( rParentGuid );
    
    return pNewAction;
}

//=========================================================================
// ACTIONS_BASE
//=========================================================================

actions_base:: actions_base ( guid ParentGuid ) : m_ParentGuid ( ParentGuid ), m_ElseFlag( FALSE )
{
}
 
//=============================================================================

actions_base::~actions_base ( void )
{
}

//=============================================================================

void actions_base::OnEnumProp ( prop_enum& rPropList )
{
    rPropList.AddBool  ( "Else",    "Action is used in the else block if TRUE.", 0 );
    rPropList.AddButton( "Remove" , "Removes this action.", PROP_TYPE_MUST_ENUM  );
}

//=============================================================================

xbool actions_base::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.VarBool  ( "Else", m_ElseFlag ) )
    {
        return TRUE;
    }
    
    if( rPropQuery.IsVar( "Remove" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarButton( "Remove" );
        }
        else
        {
            ASSERT( m_ParentGuid );
            
            object_ptr<trigger_object> TrgPtr ( m_ParentGuid );
 
            //NOTE :: RemoveAction destroys the action immedeaitely, so do not perform any operation 
            //          on the action after calling that function..
                
            if (TrgPtr.IsValid())
            {
                TrgPtr.m_pObject->RemoveAction( this );
                return TRUE;
            }
        }
        return TRUE;
    }
    
    return FALSE;
}
 
//=========================================================================

vector3 actions_base::GetPositionOwner( void )
{
    vector3 ZeroPos;

    ZeroPos.Zero();

    if (m_ParentGuid == NULL)
        return ZeroPos;

    object_ptr<object> ObjectPtr( m_ParentGuid );
    
    if (!ObjectPtr.IsValid()) 
        return ZeroPos;
    
    return ObjectPtr.m_pObject->GetPosition();
}

//=========================================================================

void actions_base::RegisterCreationFunction( action_create_function* pCreate )
{
    if (actions_base::m_CreateHead == NULL)
        actions_base::m_CreateHead = pCreate;
    else
    {
        action_create_function* pCurrent = actions_base::m_CreateHead;
        
        while( pCurrent->m_Next != NULL )
        {
            pCurrent = pCurrent->m_Next;
        }

        pCurrent->m_Next = pCreate;
    }
}






















