///////////////////////////////////////////////////////////////////////////
//
//  destory_this_trigger.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\destory_this_trigger.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"

#include "Entropy.hpp"

//=========================================================================
// DESTORY_THIS_TRIGGER
//=========================================================================

destory_this_trigger::destory_this_trigger ( guid ParentGuid ) : actions_base( ParentGuid )
{
}

//=============================================================================

void destory_this_trigger::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * destory_this_trigger::Execute" );

    (void) pParent;
    
    ASSERT( pParent );

    pParent->KillTrigger();
}

//=============================================================================

void destory_this_trigger::OnRender ( void )
{
}

//=============================================================================

void destory_this_trigger::OnEnumProp ( prop_enum& rPropList )
{  
    actions_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool destory_this_trigger::OnProperty ( prop_query& rPropQuery )
{
    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;
    
    return FALSE;
}


