///////////////////////////////////////////////////////////////////////////////
//
//  TriggerEx_Conditionals.cpp
//
///////////////////////////////////////////////////////////////////////////////

#include "..\Support\TriggerEx\TriggerEx_Conditionals.hpp"
#include "..\Support\TriggerEx\TriggerEx_Manager.hpp"
#include "..\Support\Objects\Player.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"

//include all conditions
#include "Conditions\condition_check_property.hpp"
#include "Conditions\condition_object_exists.hpp"
#include "Conditions\condition_player_button_state.hpp"
#include "Conditions\condition_player_has_item.hpp"
#include "Conditions\condition_random_chance.hpp"
#include "Conditions\condition_check_global.hpp"
#include "Conditions\condition_within_range.hpp"
#include "Conditions\condition_line_of_sight.hpp"
#include "Conditions\condition_check_focus_object.hpp"
#include "Conditions\condition_check_health.hpp"
#include "Conditions\condition_is_censored.hpp"

//=========================================================================
// STATIC FUNCTIONS
//=========================================================================

static sml_string s_OrStr ("OR ");
static sml_string s_AndStr("AND");

//=============================================================================

typedef enum_pair<conditional_ex_base::conditional_ex_types> conditional_enum_pair;

//=============================================================================

static conditional_enum_pair s_ConditonalAllEnumTable[] = 
{
        conditional_enum_pair("Check Focus Object",     conditional_ex_base::TYPE_CONDITION_CHECK_FOCUS_OBJECT),
        conditional_enum_pair("Check Global Var",       conditional_ex_base::TYPE_CONDITION_CHECK_GLOBAL),
        conditional_enum_pair("Check Property",         conditional_ex_base::TYPE_CONDITION_CHECK_PROPERTY),
        conditional_enum_pair("Object Exists",          conditional_ex_base::TYPE_CONDITION_OBJECT_EXISTS),
        conditional_enum_pair("Random Chance",          conditional_ex_base::TYPE_CONDITION_RANDOM_CHANCE),
        conditional_enum_pair("Button State",           conditional_ex_base::TYPE_CONDITION_BUTTON_STATE),
        conditional_enum_pair("Player Has Item",        conditional_ex_base::TYPE_CONDITION_PLAYER_HAS),
        conditional_enum_pair("Object In Range",        conditional_ex_base::TYPE_CONDITION_WITHIN_RANGE),
        conditional_enum_pair("Line Of Sight",          conditional_ex_base::TYPE_CONDITION_LINE_OF_SIGHT),
        conditional_enum_pair("Check Health",           conditional_ex_base::TYPE_CONDITION_CHECK_HEALTH),
        conditional_enum_pair("Content Censored",       conditional_ex_base::TYPE_CONDITION_IS_CENSORED),
        conditional_enum_pair( k_EnumEndStringConst,    conditional_ex_base::INVALID_CONDITIONAL_TYPES),  //**MUST BE LAST**//
};

static conditional_enum_pair s_ConditonalGeneralEnumTable[] = 
{
        conditional_enum_pair("Check Focus Object",     conditional_ex_base::TYPE_CONDITION_CHECK_FOCUS_OBJECT),
        conditional_enum_pair("Check Global Var",       conditional_ex_base::TYPE_CONDITION_CHECK_GLOBAL),
        conditional_enum_pair("Check Property",         conditional_ex_base::TYPE_CONDITION_CHECK_PROPERTY),
        conditional_enum_pair("Object Exists",          conditional_ex_base::TYPE_CONDITION_OBJECT_EXISTS),
        conditional_enum_pair("Random Chance",          conditional_ex_base::TYPE_CONDITION_RANDOM_CHANCE),
        conditional_enum_pair("Object In Range",        conditional_ex_base::TYPE_CONDITION_WITHIN_RANGE),
        conditional_enum_pair("Line Of Sight",          conditional_ex_base::TYPE_CONDITION_LINE_OF_SIGHT),
        conditional_enum_pair("Check Health",           conditional_ex_base::TYPE_CONDITION_CHECK_HEALTH),
        conditional_enum_pair("Content Censored",       conditional_ex_base::TYPE_CONDITION_IS_CENSORED),
        conditional_enum_pair( k_EnumEndStringConst,    conditional_ex_base::INVALID_CONDITIONAL_TYPES),  //**MUST BE LAST**//
};

static conditional_enum_pair s_ConditonalPlayerEnumTable[] = 
{
        conditional_enum_pair("Button State",           conditional_ex_base::TYPE_CONDITION_BUTTON_STATE),
        conditional_enum_pair("Player Has Item",        conditional_ex_base::TYPE_CONDITION_PLAYER_HAS),
        conditional_enum_pair( k_EnumEndStringConst,    conditional_ex_base::INVALID_CONDITIONAL_TYPES),  //**MUST BE LAST**//
};

enum_table<conditional_ex_base::conditional_ex_types> conditional_ex_base::m_ConditionalAllEnum(     s_ConditonalAllEnumTable       );        
enum_table<conditional_ex_base::conditional_ex_types> conditional_ex_base::m_ConditionalGeneralEnum( s_ConditonalGeneralEnumTable   );      
enum_table<conditional_ex_base::conditional_ex_types> conditional_ex_base::m_ConditionalPlayerEnum(  s_ConditonalPlayerEnumTable    );      

//=============================================================================

condition_ex_create_function* conditional_ex_base::m_CreateHead = NULL;

//=========================================================================
// AUTOMATIC REGISTERATION..
//=========================================================================

automatic_condition_ex_registeration <condition_check_property>         Register_condition_check_property;
automatic_condition_ex_registeration <condition_object_exists>          Register_condition_object_exists;
automatic_condition_ex_registeration <condition_player_has_item>        Register_condition_player_has_item;
automatic_condition_ex_registeration <condition_player_button_state>    Register_condition_player_button_state;
automatic_condition_ex_registeration <condition_random_chance>          Register_condition_random_chance;
automatic_condition_ex_registeration <condition_check_global>           Register_condition_check_global;
automatic_condition_ex_registeration <condition_within_range>           Register_condition_within_range;
automatic_condition_ex_registeration <condition_line_of_sight>          Register_condition_line_of_sight;
automatic_condition_ex_registeration <condition_check_focus_object>     Register_condition_check_focus_object;
automatic_condition_ex_registeration <condition_check_health>           Register_condition_check_health;
automatic_condition_ex_registeration <condition_is_censored>            Register_condition_is_censored;

//=========================================================================
// STATIC FUNCTIONS
//=========================================================================

//Using the automatic registeration, walk through the action list..
    
conditional_ex_base* conditional_ex_base::CreateCondition ( const conditional_ex_types& rType , conditional_affecter* pParent )
{
    if ( rType ==  conditional_ex_base::INVALID_CONDITIONAL_TYPES )
        return NULL;
    
    if (conditional_ex_base::m_CreateHead == NULL)
        return NULL;
    
    conditional_ex_base* pNewCondition = NULL;
    
    condition_ex_create_function* pCurrent = conditional_ex_base::m_CreateHead;
    condition_ex_create_function* pMatched = NULL;
    
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
    
    pNewCondition = pMatched->m_pCreateCondition( pParent );
    
    return pNewCondition;
}

//=========================================================================
// CONDITIONAL_BASE
//=========================================================================

conditional_ex_base::conditional_ex_base(  conditional_affecter* pParent ) : 
m_LogicFlag(FLAG_AND), 
m_ElseFlag(FALSE), 
m_pParent(pParent)
{
}

//=============================================================================

conditional_ex_base::~conditional_ex_base() 
{
}

//=============================================================================

void conditional_ex_base::OnEnumProp ( prop_enum& rPropList )
{
    rPropList.PropEnumInt   ( "LogicFlag" , "" , PROP_TYPE_DONT_SHOW  );
    rPropList.PropEnumBool  ( "ElseFlag"  , "" , PROP_TYPE_DONT_SHOW );

    ASSERT( m_pParent );    
    if (!m_pParent->IsFirstInConditionSet(this) )
    {
        rPropList.PropEnumEnum( "Logic",    "AND\0OR\0", "Logical condition to apply to this check.", PROP_TYPE_MUST_ENUM|PROP_TYPE_DONT_SAVE  );
    }

    rPropList.PropEnumHeader  ( "OrderOptions", "List of all re-ordering options for this instance", 0 );

    rPropList.PropEnumInt     ( "OrderOptions\\Index",    "Index between 0 and 7, where this condition exists", PROP_TYPE_MUST_ENUM | PROP_TYPE_DONT_SAVE );
    if (m_bAllowElse && m_pParent->CanSwitchElse(this))
    {
        rPropList.PropEnumButton  ( "OrderOptions\\If,Else",  "Mark this condition as an else.  If the initial \"if\" condition is not met, this condition will be handled in the else.", PROP_TYPE_MUST_ENUM );
    }
    rPropList.PropEnumButton  ( "OrderOptions\\Remove" ,  "Removes this condtion.", PROP_TYPE_MUST_ENUM  );
}

//=============================================================================

xbool conditional_ex_base::OnProperty ( prop_query& rPropQuery )
{  
    if( rPropQuery.IsVar( "OrderOptions\\Index" ) )
    {
        ASSERT( m_pParent );    
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarInt(m_pParent->GetConditionIndex(this));
        }
        else
        {
            m_pParent->SetConditionIndex(this, rPropQuery.GetVarInt());
        }
        return TRUE;
    }

    if ( rPropQuery.VarBool  ( "ElseFlag", m_ElseFlag ) )
    {
        return TRUE;
    }

    if( rPropQuery.IsVar( "OrderOptions\\If,Else" ) )
    {
        if( rPropQuery.IsRead() )
        {
            if (m_ElseFlag)
            {
                rPropQuery.SetVarButton( "Set as If Condition" );
            }
            else
            {
                rPropQuery.SetVarButton( "Set as Else Condition" );
            }
        }
        else
        {
            m_pParent->SwitchElse(this);
        }
        return TRUE;
    }
 
    if ( rPropQuery.VarInt  ( "LogicFlag", *((s32*) &m_LogicFlag )) )
    {
        return TRUE;
    }

    if ( rPropQuery.IsVar( "Logic"  ) )
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_LogicFlag )
            {
            case FLAG_AND:  rPropQuery.SetVarEnum( "AND" );  break;
            case FLAG_OR:   rPropQuery.SetVarEnum( "OR" );   break;
              
            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            if( x_stricmp( pString, "AND" )==0) { m_LogicFlag = FLAG_AND;}
            if( x_stricmp( pString, "OR" )==0)  { m_LogicFlag = FLAG_OR; }
            
            return( TRUE );
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
            ASSERT( m_pParent );
            //NOTE :: RemoveCondition destroys the condition immedeaitely, so do not perform any operation 
            //      on the condition after calling that function..
            m_pParent->RemoveCondition( this );

            return TRUE;
        }
        return TRUE;
    }

    return FALSE;
}

//=========================================================================

void conditional_ex_base::RegisterCreationFunction( condition_ex_create_function* pCreate )
{
    if (conditional_ex_base::m_CreateHead == NULL)
        conditional_ex_base::m_CreateHead = pCreate;
    else
    {
        condition_ex_create_function* pCurrent = conditional_ex_base::m_CreateHead;
        
        while( pCurrent->m_Next != NULL )
        {
            pCurrent = pCurrent->m_Next;
        }

        pCurrent->m_Next = pCreate;
    }
}

//=============================================================================

const char* conditional_ex_base::GetFlagStr( void )
{
    if (m_LogicFlag == conditional_ex_base::FLAG_OR)
        return s_OrStr.Get();
    else
        return s_AndStr.Get();
}

