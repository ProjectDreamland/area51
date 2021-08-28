///////////////////////////////////////////////////////////////////////////
//
//  ai_modify_behavior.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\ai_modify_behavior.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"
#include "..\Support\Characters\Character.hpp"

#include "Entropy.hpp"

//=========================================================================
// AI_MODIFY_BEHAVIOR
//=========================================================================

ai_modify_behavior::ai_modify_behavior ( guid ParentGuid ) : actions_base( ParentGuid ),
m_DesiredBehavior(character_state::STATE_IDLE)
{
}

//=========================================================================

void ai_modify_behavior::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * ai_modify_behavior::Execute" );
    
    (void) pParent;

    ASSERT( pParent );

    const guid* pGuidActor = pParent->GetTriggerActor();

    if (pGuidActor == NULL)
        return;

    object_ptr<character> CharPtr(*pGuidActor);

    if (!CharPtr.IsValid())
        return;

    ((character*)CharPtr.m_pObject)->SetupState( m_DesiredBehavior );
}

//=========================================================================

void ai_modify_behavior::OnRender ( void )
{
}

//=========================================================================

void ai_modify_behavior::OnEnumProp ( prop_enum& rPropList )
{   
    rPropList.AddEnum ( "Desired Behavior" , character::GetStatesEnum(), "The desired behavior for the AI to exhbit." );

    actions_base::OnEnumProp( rPropList );
}

//=========================================================================

xbool ai_modify_behavior::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.IsVar( "Desired Behavior" ) ) 
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarEnum( character::GetStateName(m_DesiredBehavior) ); 
          
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();

            m_DesiredBehavior = character::GetStateByName  ( pString) ;

            return( TRUE );
        }
    }

    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;
    
    return FALSE;
}

