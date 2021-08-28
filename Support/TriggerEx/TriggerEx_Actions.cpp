///////////////////////////////////////////////////////////////////////////
//
//  TriggerEx_Actions.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\TriggerEx\TriggerEx_Actions.hpp"
#include "..\Support\TriggerEx\TriggerEx_Manager.hpp"
#include "..\Support\TriggerEx\TriggerEx_Object.hpp"
#include "..\Support\Characters\Character.hpp"
#include "..\Support\Characters\TaskSystem\character_task.hpp"

//include all actions
#include "Actions\action_set_property.hpp"
#include "Actions\action_object_destroy.hpp"
#include "Actions\action_object_activation.hpp"
#include "Actions\action_portal_activate.hpp"
#include "Actions\action_object_move.hpp"
#include "Actions\action_object_move_relative.hpp"
#include "Actions\action_object_damage.hpp"
#include "Actions\action_door_logic.hpp"
#include "Actions\action_player_inventory.hpp"
#include "Actions\action_player_camera_shake.hpp"
#include "Actions\action_create_template.hpp"
#include "Actions\action_affect_global.hpp"
#include "Actions\action_music_intensity.hpp"
#include "Actions\action_display_text.hpp"
#include "Actions\action_player_hud.hpp"
#include "Actions\action_save_game.hpp"
#include "Actions\action_fade_geometry.hpp"
#include "Actions\action_screen_fade.hpp"
#include "Actions\action_man_turret.hpp"
#include "Actions\action_exit_turret.hpp"
#include "Actions\action_play_2d_sound.hpp"
#include "Actions\action_mission_failed.hpp"

#include "Actions\action_ai_attack_guid.hpp"
#include "Actions\action_ai_pathto_guid.hpp"
#include "Actions\action_ai_lookat_guid.hpp"
#include "Actions\action_ai_play_anim.hpp"
#include "Actions\action_ai_dialog_line.hpp"
#include "Actions\action_ai_searchto_guid.hpp"
#include "Actions\action_ai_inventory.hpp"
#include "Actions\action_ai_nav_activation.hpp"
#include "Actions\action_ai_death.hpp"

#include "Meta\trigger_meta_label.hpp"
#include "Meta\trigger_meta_delay.hpp"
#include "Meta\trigger_meta_block.hpp"
#include "Meta\trigger_meta_goto.hpp"
#include "Meta\trigger_meta_breakpoint.hpp"
#include "Meta\trigger_meta_cinema_block.hpp"

#include "Actions\action_load_level.hpp"
#include "Actions\action_checkpoint.hpp"
#include "Actions\action_change_perception.hpp"

#include "AudioMgr\AudioMgr.hpp"
#include "..\Support\Templatemgr\TemplateMgr.hpp"
#include "Entropy.hpp"
#include "Music_mgr\Music_mgr.hpp"

//=========================================================================
// STATIC VARIABLES
//=========================================================================

static const xcolor s_ActivateColor         (0,255,0);
static const xcolor s_DeactivateColor       (255,0,0);
static const xcolor s_MoveColor             (255,0,255);
static const xcolor s_DestroyColor          (255,255,255);
static const xcolor s_AIColor               (0,255,255);
static const xcolor s_CreateTemplateColor   (255,255,255);

static const f32    s_SphereRadius  = 100.0f;

typedef enum_pair<actions_ex_base::action_ex_types> actions_ex_enum_pair;

//****s_ActionsAllEnumTable **** this table should not use the BuildString() as it exceeds the 255 char limit...
static actions_ex_enum_pair s_ActionsAllEnumTable[] = 
{
        actions_ex_enum_pair("Affect Global",                  actions_ex_base::TYPE_ACTION_AFFECT_GLOBAL_VAR),
        actions_ex_enum_pair("Affect Property",                actions_ex_base::TYPE_ACTION_AFFECT_PROPERTY),
        actions_ex_enum_pair("Create Template",                actions_ex_base::TYPE_ACTION_CREATE_TEMPLATE),
        actions_ex_enum_pair("Damage Object",                  actions_ex_base::TYPE_ACTION_DAMAGE_OBJECT),
        actions_ex_enum_pair("Destroy Object",                 actions_ex_base::TYPE_ACTION_DESTORY_OBJECT),
        actions_ex_enum_pair("Door Logic",                     actions_ex_base::TYPE_ACTION_DOOR_LOGIC),
        actions_ex_enum_pair("Object Activation",              actions_ex_base::TYPE_ACTION_OBJECT_ACTIVATION),
        actions_ex_enum_pair("Player Inventory",               actions_ex_base::TYPE_ACTION_PLAYER_INVENTORY),
        actions_ex_enum_pair("Shake Player view",              actions_ex_base::TYPE_ACTION_PLAYER_CAMERA_SHAKE),
        actions_ex_enum_pair("Man Turret",                     actions_ex_base::TYPE_ACTION_MAN_TURRET),
        actions_ex_enum_pair("Exit Turret",                    actions_ex_base::TYPE_ACTION_EXIT_TURRET),
        actions_ex_enum_pair("Move Object",                    actions_ex_base::TYPE_ACTION_MOVE_OBJECT),
        actions_ex_enum_pair("Move Object Relative",           actions_ex_base::TYPE_ACTION_MOVE_OBJECT_RELATIVE),
        actions_ex_enum_pair("Portal Activation",              actions_ex_base::TYPE_ACTION_PORTAL_ACTIVATE),
        actions_ex_enum_pair("Player HUD",                     actions_ex_base::TYPE_ACTION_PLAYER_HUD),
        actions_ex_enum_pair("AI Attack",                      actions_ex_base::TYPE_ACTION_AI_ATTACK_GUID),
        actions_ex_enum_pair("AI Dialog Line",                 actions_ex_base::TYPE_ACTION_AI_DIALOG_LINE),
        actions_ex_enum_pair("AI Look At",                     actions_ex_base::TYPE_ACTION_AI_LOOKAT_GUID),
        actions_ex_enum_pair("AI Path To",                     actions_ex_base::TYPE_ACTION_AI_PATHTO_GUID),
        actions_ex_enum_pair("AI Play Anim",                   actions_ex_base::TYPE_ACTION_AI_PLAY_ANIM),
        actions_ex_enum_pair("AI Search To",                   actions_ex_base::TYPE_ACTION_AI_SEARCHTO_GUID),
        actions_ex_enum_pair("AI Inventory",                   actions_ex_base::TYPE_ACTION_AI_INVENTORY),
        actions_ex_enum_pair("AI Nav Connection Activation",   actions_ex_base::TYPE_ACTION_AI_NAV_ACTIVATION),
        actions_ex_enum_pair("AI Death",                       actions_ex_base::TYPE_ACTION_AI_DEATH),
        actions_ex_enum_pair("Block",                          actions_ex_base::TYPE_META_BLOCK),
        actions_ex_enum_pair("Breakpoint",                     actions_ex_base::TYPE_META_BREAKPOINT),
        actions_ex_enum_pair("Delay",                          actions_ex_base::TYPE_META_DELAY),
        actions_ex_enum_pair("Goto",                           actions_ex_base::TYPE_META_GOTO),
        actions_ex_enum_pair("Label",                          actions_ex_base::TYPE_META_LABEL),
        actions_ex_enum_pair("Music Intensity",                actions_ex_base::TYPE_ACTION_MUSIC_INTENSITY),
        actions_ex_enum_pair("Display Text",                   actions_ex_base::TYPE_ACTION_DISPLAY_TEXT),
        actions_ex_enum_pair("Load Level",                     actions_ex_base::TYPE_ACTION_LOAD_LEVEL),
        actions_ex_enum_pair("Save Game",                      actions_ex_base::TYPE_ACTION_SAVE_GAME),
        actions_ex_enum_pair("Fade Geometry",                  actions_ex_base::TYPE_ACTION_FADE_GEOMETRY),
        actions_ex_enum_pair("Screen Fade",                    actions_ex_base::TYPE_ACTION_SCREEN_FADE),
        actions_ex_enum_pair("Cinema Block",                   actions_ex_base::TYPE_META_CINEMA_BLOCK),
        actions_ex_enum_pair("Checkpoint",                     actions_ex_base::TYPE_ACTION_CHECKPOINT),
        actions_ex_enum_pair("Play 2D Sound",                  actions_ex_base::TYPE_ACTION_PLAY_2D_SOUND),
        actions_ex_enum_pair("Change Perception",              actions_ex_base::TYPE_ACTION_CHANGE_PERCEPTION),
        actions_ex_enum_pair("Mission Failed",                 actions_ex_base::TYPE_ACTION_MISSION_FAILED),
        actions_ex_enum_pair( k_EnumEndStringConst,            actions_ex_base::INVALID_ACTION_TYPES) //**MUST BE LAST**//
};

static actions_ex_enum_pair s_ActionsGeneralEnumTable[] = 
{
        actions_ex_enum_pair("Create Template",                actions_ex_base::TYPE_ACTION_CREATE_TEMPLATE),
        actions_ex_enum_pair("Damage Object",                  actions_ex_base::TYPE_ACTION_DAMAGE_OBJECT),
        actions_ex_enum_pair("Destroy Object",                 actions_ex_base::TYPE_ACTION_DESTORY_OBJECT),
        actions_ex_enum_pair("Fade Geometry",                  actions_ex_base::TYPE_ACTION_FADE_GEOMETRY),
        actions_ex_enum_pair("Move Object",                    actions_ex_base::TYPE_ACTION_MOVE_OBJECT),
        actions_ex_enum_pair("Move Object Relative",           actions_ex_base::TYPE_ACTION_MOVE_OBJECT_RELATIVE),
        actions_ex_enum_pair("Object Activation",              actions_ex_base::TYPE_ACTION_OBJECT_ACTIVATION),
        actions_ex_enum_pair("Portal Activation",              actions_ex_base::TYPE_ACTION_PORTAL_ACTIVATE),
        actions_ex_enum_pair( k_EnumEndStringConst,            actions_ex_base::INVALID_ACTION_TYPES) //**MUST BE LAST**//
};

static actions_ex_enum_pair s_ActionsInterfaceEnum[] = 
{   
        actions_ex_enum_pair("Change Perception",              actions_ex_base::TYPE_ACTION_CHANGE_PERCEPTION),
        actions_ex_enum_pair("Checkpoint",                     actions_ex_base::TYPE_ACTION_CHECKPOINT),
        actions_ex_enum_pair("Display Text",                   actions_ex_base::TYPE_ACTION_DISPLAY_TEXT),
        actions_ex_enum_pair("Load Level",                     actions_ex_base::TYPE_ACTION_LOAD_LEVEL),
        actions_ex_enum_pair("Music Intensity",                actions_ex_base::TYPE_ACTION_MUSIC_INTENSITY),
        actions_ex_enum_pair("Save Game",                      actions_ex_base::TYPE_ACTION_SAVE_GAME),
        actions_ex_enum_pair("Screen Fade",                    actions_ex_base::TYPE_ACTION_SCREEN_FADE),
        actions_ex_enum_pair( k_EnumEndStringConst,            actions_ex_base::INVALID_ACTION_TYPES) //**MUST BE LAST**//
};

static actions_ex_enum_pair s_ActionsSpecificEnumTable[] = 
{
        actions_ex_enum_pair("Affect Global",                  actions_ex_base::TYPE_ACTION_AFFECT_GLOBAL_VAR),
        actions_ex_enum_pair("Affect Property",                actions_ex_base::TYPE_ACTION_AFFECT_PROPERTY),
        actions_ex_enum_pair("Door Logic",                     actions_ex_base::TYPE_ACTION_DOOR_LOGIC),
        actions_ex_enum_pair("Play 2D Sound",                  actions_ex_base::TYPE_ACTION_PLAY_2D_SOUND),
        actions_ex_enum_pair( k_EnumEndStringConst,            actions_ex_base::INVALID_ACTION_TYPES) //**MUST BE LAST**//
};

static actions_ex_enum_pair s_ActionsAIEnumTable[] = 
{
        actions_ex_enum_pair("AI Attack",                      actions_ex_base::TYPE_ACTION_AI_ATTACK_GUID),
        actions_ex_enum_pair("AI Dialog Line",                 actions_ex_base::TYPE_ACTION_AI_DIALOG_LINE),
        actions_ex_enum_pair("AI Look At",                     actions_ex_base::TYPE_ACTION_AI_LOOKAT_GUID),
        actions_ex_enum_pair("AI Path To",                     actions_ex_base::TYPE_ACTION_AI_PATHTO_GUID),
        actions_ex_enum_pair("AI Play Anim",                   actions_ex_base::TYPE_ACTION_AI_PLAY_ANIM),
        actions_ex_enum_pair("AI Search To",                   actions_ex_base::TYPE_ACTION_AI_SEARCHTO_GUID),
        actions_ex_enum_pair("AI Inventory",                   actions_ex_base::TYPE_ACTION_AI_INVENTORY),
        actions_ex_enum_pair("AI Nav Connection Activation",   actions_ex_base::TYPE_ACTION_AI_NAV_ACTIVATION),
        actions_ex_enum_pair("AI Death",                       actions_ex_base::TYPE_ACTION_AI_DEATH),
        actions_ex_enum_pair( k_EnumEndStringConst,            actions_ex_base::INVALID_ACTION_TYPES) //**MUST BE LAST**//
};

static actions_ex_enum_pair s_ActionsPlayerEnumTable[] = 
{
        actions_ex_enum_pair("Player Inventory",               actions_ex_base::TYPE_ACTION_PLAYER_INVENTORY),
        actions_ex_enum_pair("Shake Player view",              actions_ex_base::TYPE_ACTION_PLAYER_CAMERA_SHAKE),
        actions_ex_enum_pair("Player HUD",                     actions_ex_base::TYPE_ACTION_PLAYER_HUD),
        actions_ex_enum_pair("Man Turret",                     actions_ex_base::TYPE_ACTION_MAN_TURRET),
        actions_ex_enum_pair("Exit Turret",                    actions_ex_base::TYPE_ACTION_EXIT_TURRET),
        actions_ex_enum_pair("Mission Failed",                 actions_ex_base::TYPE_ACTION_MISSION_FAILED),
        actions_ex_enum_pair( k_EnumEndStringConst,            actions_ex_base::INVALID_ACTION_TYPES) //**MUST BE LAST**//
};

static actions_ex_enum_pair s_ActionsMetaEnumTable[] = 
{
        actions_ex_enum_pair("Block",                          actions_ex_base::TYPE_META_BLOCK),
        actions_ex_enum_pair("Breakpoint",                     actions_ex_base::TYPE_META_BREAKPOINT),
        actions_ex_enum_pair("Cinema Block",                   actions_ex_base::TYPE_META_CINEMA_BLOCK),
        actions_ex_enum_pair("Delay",                          actions_ex_base::TYPE_META_DELAY),
        actions_ex_enum_pair("Goto",                           actions_ex_base::TYPE_META_GOTO),
        actions_ex_enum_pair("Label",                          actions_ex_base::TYPE_META_LABEL),
        actions_ex_enum_pair( k_EnumEndStringConst,            actions_ex_base::INVALID_ACTION_TYPES) //**MUST BE LAST**//
};

static actions_ex_enum_pair s_ActionsTaskEnumTable[] = 
{
        actions_ex_enum_pair("Block",                          actions_ex_base::TYPE_META_BLOCK),
        actions_ex_enum_pair("Breakpoint",                     actions_ex_base::TYPE_META_BREAKPOINT),
        actions_ex_enum_pair("Delay",                          actions_ex_base::TYPE_META_DELAY),
        actions_ex_enum_pair("Object Activation",              actions_ex_base::TYPE_ACTION_OBJECT_ACTIVATION),
        actions_ex_enum_pair( k_EnumEndStringConst,            actions_ex_base::INVALID_ACTION_TYPES) //**MUST BE LAST**//
};

enum_table<actions_ex_base::action_ex_types>  actions_ex_base::m_ActionsTaskEnum     ( s_ActionsTaskEnumTable      ); 
enum_table<actions_ex_base::action_ex_types>  actions_ex_base::m_ActionsAllEnum      ( s_ActionsAllEnumTable       ); 
enum_table<actions_ex_base::action_ex_types>  actions_ex_base::m_ActionsGeneralEnum  ( s_ActionsGeneralEnumTable   );              
enum_table<actions_ex_base::action_ex_types>  actions_ex_base::m_ActionsSpecificEnum ( s_ActionsSpecificEnumTable  ); 
enum_table<actions_ex_base::action_ex_types>  actions_ex_base::m_ActionsInterfaceEnum( s_ActionsInterfaceEnum      ); 
enum_table<actions_ex_base::action_ex_types>  actions_ex_base::m_ActionsPlayerEnum   ( s_ActionsPlayerEnumTable    ); 
enum_table<actions_ex_base::action_ex_types>  actions_ex_base::m_ActionsAIEnum       ( s_ActionsAIEnumTable        ); 
enum_table<actions_ex_base::action_ex_types>  actions_ex_base::m_ActionsMetaEnum     ( s_ActionsMetaEnumTable      ); 
       
action_ex_create_function*  actions_ex_base::m_CreateHead = NULL;               

//=========================================================================
// AUTOMATIC REGISTERATION..
//=========================================================================

//actions
automatic_action_ex_registeration <action_set_property>          Register_action_set_property;
automatic_action_ex_registeration <action_object_destroy>        Register_action_object_destroy;
automatic_action_ex_registeration <action_object_damage>         Register_action_object_damage;
automatic_action_ex_registeration <action_object_activation>     Register_action_object_activation;
automatic_action_ex_registeration <action_portal_activate>       Register_action_portal_activation;
automatic_action_ex_registeration <action_object_move>           Register_action_object_move;
automatic_action_ex_registeration <action_object_move_relative>  Register_action_object_move_relative;
automatic_action_ex_registeration <action_door_logic>            Register_action_door_logic;
automatic_action_ex_registeration <action_player_camera_shake>   Register_action_player_camera_shake;
automatic_action_ex_registeration <action_man_turret>            Register_action_man_turret;
automatic_action_ex_registeration <action_exit_turret>           Register_action_exit_turret;
automatic_action_ex_registeration <action_player_inventory>      Register_action_player_inventory;
automatic_action_ex_registeration <action_create_template>       Register_action_create_template;
automatic_action_ex_registeration <action_player_hud>            Register_action_player_hud;
automatic_action_ex_registeration <action_mission_failed>        Register_action_mission_failed;

automatic_action_ex_registeration <action_music_intensity>       Register_action_music_intensity;
automatic_action_ex_registeration <action_display_text>          Register_action_display_text;
automatic_action_ex_registeration <action_load_level>            Register_action_load_level;
automatic_action_ex_registeration <action_save_game>             Register_action_save_game;
automatic_action_ex_registeration <action_fade_geometry>         Register_action_fade_geometry;
automatic_action_ex_registeration <action_screen_fade>           Register_action_screen_fade;
automatic_action_ex_registeration <action_checkpoint>            Register_action_checkpoint;
automatic_action_ex_registeration <action_affect_global>         Register_action_affect_global;
     
automatic_action_ex_registeration <action_play_2d_sound>         Register_action_play_2d_sound;
automatic_action_ex_registeration <action_change_perception>     Register_action_change_perception;

automatic_action_ex_registeration <action_ai_attack_guid>        Register_action_ai_attack_guid;
automatic_action_ex_registeration <action_ai_lookat_guid>        Register_action_ai_lookat_guid;
automatic_action_ex_registeration <action_ai_pathto_guid>        Register_action_ai_pathto_guid;
automatic_action_ex_registeration <action_ai_dialog_line>        Register_action_ai_dialog_line;
automatic_action_ex_registeration <action_ai_play_anim>          Register_action_ai_play_anim;
automatic_action_ex_registeration <action_ai_searchto_guid>      Register_action_ai_searchto_guid;
automatic_action_ex_registeration <action_ai_inventory>          Register_action_ai_inventory;
automatic_action_ex_registeration <action_ai_nav_activation>     Register_action_ai_nav_activation;
automatic_action_ex_registeration <action_ai_death>              Register_action_ai_death;

//metas
automatic_action_ex_registeration <trigger_meta_label>           Register_trigger_meta_label;
automatic_action_ex_registeration <trigger_meta_delay>           Register_trigger_meta_delay;
automatic_action_ex_registeration <trigger_meta_block>           Register_trigger_meta_block;
automatic_action_ex_registeration <trigger_meta_breakpoint>      Register_trigger_meta_breakpoint;
automatic_action_ex_registeration <trigger_meta_goto>            Register_trigger_meta_goto;
automatic_action_ex_registeration <trigger_meta_cinema_block>    Register_trigger_meta_cinema_block;


//=========================================================================
// STATIC FUNCTIONS
//=========================================================================

//Using the automatic registeration, walk through the action list..

actions_ex_base* actions_ex_base::CreateAction ( const action_ex_types& rType , const guid& rParentGuid )
{
    if ( rType ==  actions_ex_base::INVALID_ACTION_TYPES )
        return NULL;
        
    if (actions_ex_base::m_CreateHead == NULL)
        return NULL;
    
    actions_ex_base* pNewAction = NULL;

    action_ex_create_function* pCurrent = actions_ex_base::m_CreateHead;
    action_ex_create_function* pMatched = NULL;

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
    {
        ASSERT(FALSE);
        return NULL;
    }

    pNewAction = pMatched->m_pCreateAction( rParentGuid );
    
    return pNewAction;
}

//=========================================================================
// ACTIONS_BASE
//=========================================================================

actions_ex_base:: actions_ex_base ( guid ParentGuid ) : m_TriggerGuid ( ParentGuid ), 
m_pTaskOwner(NULL),
m_ElseFlag( FALSE ),
m_bErrorInExecute(FALSE)
{
}
 
//=============================================================================

actions_ex_base::~actions_ex_base ( void )
{
}

//=============================================================================

void actions_ex_base::OnEnumProp ( prop_enum& rPropList )
{
    if( m_TriggerGuid )
    {
        rPropList.PropEnumBool  ( "ElseFlag","", PROP_TYPE_DONT_SHOW );
    }

#ifdef X_EDITOR
    rPropList.PropEnumHeader( "OrderOptions",            "List of all re-ordering options for this instance", 0 );
    rPropList.PropEnumInt   ( "OrderOptions\\Index",     "Index between 0 and 47, where this action exists", PROP_TYPE_MUST_ENUM | PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_COPY);
    if( m_TriggerGuid )
    {
        object* pObject = NULL;
        if ( SMP_UTIL_IsGuidOfType ( &pObject, m_TriggerGuid , trigger_ex_object::GetRTTI() ) == TRUE )
        {
            rPropList.PropEnumButton( "OrderOptions\\Copy" ,     "Copy this action for pasting in any trigger.", PROP_TYPE_MUST_ENUM  );
            if (((trigger_ex_object*)pObject)->CanSwitchElse(this))
            {
                rPropList.PropEnumButton( "OrderOptions\\If,Else",   "Mark this action as an else action.  If the initial \"if\" condition is not met, this action will happen if the else \"condition\" is met.", PROP_TYPE_MUST_ENUM );
            }
        }    
    }
    rPropList.PropEnumButton( "OrderOptions\\Remove" ,   "Removes this action.", PROP_TYPE_MUST_ENUM  );
#endif
}

//=============================================================================

xbool actions_ex_base::OnProperty ( prop_query& rPropQuery )
{
    if( m_TriggerGuid )
    {
        if ( rPropQuery.VarBool  ( "ElseFlag", m_ElseFlag ) )
        {
            return TRUE;
        }
    }

#ifdef X_EDITOR
    if( rPropQuery.IsVar( "OrderOptions\\Index" ) )
    {
        if ( m_TriggerGuid )
        {
            object* pObject = NULL;            
            if ( SMP_UTIL_IsGuidOfType ( &pObject, m_TriggerGuid , trigger_ex_object::GetRTTI() ) == TRUE )
            {
                if( rPropQuery.IsRead() )
                {
                    rPropQuery.SetVarInt(((trigger_ex_object*)pObject)->GetActionIndex(this));
                }
                else
                {
                    ((trigger_ex_object*)pObject)->SetActionIndex(this, rPropQuery.GetVarInt());
                }
            }
        }
        else if ( m_pTaskOwner )
        {
            if( rPropQuery.IsRead() )
            {
                rPropQuery.SetVarInt(m_pTaskOwner->GetSubTaskIndex((action_ai_base*)this));
            }
            else
            {
                m_pTaskOwner->SetSubTaskIndex((action_ai_base*)this, rPropQuery.GetVarInt());
            }
        }
        return TRUE;
    }

    if( m_TriggerGuid )
    {
        if( rPropQuery.IsVar( "OrderOptions\\Copy" ) )
        {
            if( rPropQuery.IsRead() )
            {
                rPropQuery.SetVarButton( "Copy" );
            }
            else
            {
                object_ptr<trigger_ex_object> TrgPtr ( m_TriggerGuid );
                if (TrgPtr.IsValid())
                {
                    TrgPtr.m_pObject->CopyAction( this );
                    return TRUE;
                }
            }
            return TRUE;
        }

        if( rPropQuery.IsVar( "OrderOptions\\If,Else" ) )
        {
            if( rPropQuery.IsRead() )
            {
                if (m_ElseFlag)
                {
                    rPropQuery.SetVarButton( "Set as Primary Action" );
                }
                else
                {
                    rPropQuery.SetVarButton( "Set as Else Action" );
                }
            }
            else
            {
                object* pObject = NULL;
                if ( SMP_UTIL_IsGuidOfType ( &pObject, m_TriggerGuid , trigger_ex_object::GetRTTI() ) == TRUE )
                {
                    ((trigger_ex_object*)pObject)->SwitchElse(this);
                }        
            }
            return TRUE;
        }
    }
    
    if( rPropQuery.IsVar( "OrderOptions\\Remove" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarButton( "Remove" );
        }
        else
        {
            if ( m_TriggerGuid )
            {
                object_ptr<trigger_ex_object> TrgPtr ( m_TriggerGuid );
 
                //NOTE :: RemoveAction destroys the action immedeaitely, so do not perform any operation 
                //          on the action after calling that function..                
                if (TrgPtr.IsValid())
                {
                    TrgPtr.m_pObject->RemoveAction( this );
                    return TRUE;
                }
            }
            else if ( m_pTaskOwner )
            {
                m_pTaskOwner->RemoveSubTask( (action_ai_base*) this ); 
            }
        }
        return TRUE;
    }
#endif //X_EDITOR

    return FALSE;
}
 
//=========================================================================

vector3 actions_ex_base::GetPositionOwner( void )
{
    vector3 ZeroPos;

    ZeroPos.Zero();

    if (m_TriggerGuid)
    {
        object_ptr<object> ObjectPtr( m_TriggerGuid );
    
        if (ObjectPtr.IsValid()) 
            return ObjectPtr.m_pObject->GetPosition();
    }
    else if (m_pTaskOwner)
    {
        guid TaskSet = m_pTaskOwner->GetSetGuid();
        if (TaskSet)
        {
            object_ptr<object> ObjectPtr( TaskSet );
    
            if (!ObjectPtr.IsValid()) 
                return ObjectPtr.m_pObject->GetPosition();
        }
    }

    return ZeroPos;
}

//=========================================================================

void actions_ex_base::RegisterCreationFunction( action_ex_create_function* pCreate )
{
    if (actions_ex_base::m_CreateHead == NULL)
        actions_ex_base::m_CreateHead = pCreate;
    else
    {
        action_ex_create_function* pCurrent = actions_ex_base::m_CreateHead;
        
        while( pCurrent->m_Next != NULL )
        {
            pCurrent = pCurrent->m_Next;
        }

        pCurrent->m_Next = pCreate;
    }
}

//=========================================================================

trigger_ex_object* actions_ex_base::GetTriggerPtr( void )
{
    return (trigger_ex_object*)g_ObjMgr.GetObjectByGuid(m_TriggerGuid);
}

//===========================================================================

xbool actions_ex_base::RetryOnError( void )
{
    if ( GetTriggerPtr() )
    {
        return GetTriggerPtr()->RetryOnError();
    }
    return FALSE;
}

















