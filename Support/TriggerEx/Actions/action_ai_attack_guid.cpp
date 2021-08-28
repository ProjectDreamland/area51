///////////////////////////////////////////////////////////////////////////
//
//  action_ai_attack_guid.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_ai_attack_guid.hpp"
#include "..\Support\Characters\Character.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_ai_attack_guid::action_ai_attack_guid ( guid ParentGuid ) : action_ai_base( ParentGuid )
{
}

//=============================================================================

void action_ai_attack_guid::OnEnumProp	( prop_enum& rPropList )
{
    action_ai_base::OnEnumProp( rPropList );

    //guid specific fields
    m_TargetAffecter.OnEnumProp( rPropList, "Target" );
}

//=============================================================================

xbool action_ai_attack_guid::OnProperty	( prop_query& rPropQuery )
{
    if( action_ai_base::OnProperty( rPropQuery ) )
        return TRUE;
  
    //guid specific fields
    if( m_TargetAffecter.OnProperty( rPropQuery, "Target" ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char* action_ai_attack_guid::GetDescription( void )
{
    static big_string   Info;
    static med_string   AIName;
    static med_string   ObjectName;
    AIName.Set( GetAIName() );
    ObjectName.Set( m_TargetAffecter.GetObjectInfo() );
    Info.Set(xfs("%s Attack %s", AIName.Get(), ObjectName.Get()));          
    return Info.Get();
}

//=============================================================================

guid action_ai_attack_guid::GetTargetGuid( void )
{
    guid Target = 0;

    object* pObject = m_TargetAffecter.GetObjectPtr();
    if (pObject)
    {
        Target = pObject->GetGuid();
    }

    return Target;
}
