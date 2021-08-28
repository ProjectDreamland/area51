///////////////////////////////////////////////////////////////////////////
//
//  ai_modify_behavior_targeted.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\ai_modify_behavior_targeted.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"
#include "..\Support\Characters\Character.hpp"

#include "Entropy.hpp"

static const xcolor s_AIColor               (0,255,255);

//=========================================================================
// AI_MODIFY_BEHAVIOR_TARGETED
//=========================================================================

ai_modify_behavior_targeted::ai_modify_behavior_targeted ( guid ParentGuid ) : actions_base( ParentGuid ),
m_AIGuid(NULL),
m_DesiredBehavior(character_state::STATE_IDLE)  
{
}

//=============================================================================

void ai_modify_behavior_targeted::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * ai_modify_behavior_targeted::Execute" );

    (void) pParent;
    
    ASSERT( pParent );

    if (m_AIGuid == 0)
        return;

    object_ptr<character>   CharPtr(m_AIGuid);

    if ( !CharPtr.IsValid() )
        return;

    ((character*)CharPtr.m_pObject)->SetupState( m_DesiredBehavior );
}

//=============================================================================

void ai_modify_behavior_targeted::OnRender ( void )
{
    object_ptr<object> ObjectPtr(m_AIGuid);
    
    if ( !ObjectPtr.IsValid() )
        return;

#ifdef TARGET_PC
    vector3 MyPosition =  GetPositionOwner() + SMP_UTIL_RandomVector(k_rand_draw_displace_amt);
    draw_Line( MyPosition, ObjectPtr.m_pObject->GetPosition(), s_AIColor );
    draw_BBox( ObjectPtr.m_pObject->GetBBox(), s_AIColor );
    draw_Label( ObjectPtr.m_pObject->GetPosition(), s_AIColor, GetTypeName() );
#endif
}

//=============================================================================

void ai_modify_behavior_targeted::OnEnumProp ( prop_enum& rPropList )
{  
    rPropList.AddGuid ( "AI Guid" , "Guid of the AI state to modify." );
     
    rPropList.AddEnum ( "Desired Behavior" , character::GetStatesEnum(), "The desired behavior for the AI to exhbit." );

    actions_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool ai_modify_behavior_targeted::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.VarGUID ( "AI Guid"  , m_AIGuid ) )
    {
        return TRUE;
    }
    
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

