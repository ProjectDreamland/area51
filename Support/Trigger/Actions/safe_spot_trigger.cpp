///////////////////////////////////////////////////////////////////////////
//
//  safe_spot_trigger.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\safe_spot_trigger.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"
#include "..\Support\Characters\Character.hpp"
#include "objects\Player.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"

#include "Entropy.hpp"

static const xcolor s_AIColor               (0,255,255);

//=========================================================================
// safe_spot_trigger
//=========================================================================

safe_spot_trigger::safe_spot_trigger ( guid ParentGuid ) : actions_base( ParentGuid )
{
}

//=============================================================================

void safe_spot_trigger::Execute ( trigger_object* pParent )
{
    ( void ) pParent;

    TRIGGER_CONTEXT( "ACTION * safe_spot_trigger::Execute" );
      
    if ( pParent->GetTriggerActor() ==NULL )
        return;

    object_ptr<player> PlayerObj( *pParent->GetTriggerActor() );

    if ( !PlayerObj.IsValid( ) )
        return;

    ASSERT( PlayerObj.m_pObject );
    
    PlayerObj.m_pObject->BackUpCurrentState();
}

//=============================================================================

void safe_spot_trigger::OnRender ( void )
{
    actions_base::OnRender();
    
#ifdef TARGET_PC
#endif
}

//=============================================================================

void safe_spot_trigger::OnEnumProp ( prop_enum& rPropList )
{
    //object info
    
    actions_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool safe_spot_trigger::OnProperty ( prop_query& rPropQuery )
{   
    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}



