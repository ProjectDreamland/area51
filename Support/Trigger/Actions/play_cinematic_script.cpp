///////////////////////////////////////////////////////////////////////////
//
//  play_cinematic_script.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\play_cinematic_script.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"

#include "Entropy.hpp"

static const xcolor s_ActivateColor         (0,255,0);

//=========================================================================
// play_cinematic_script
//=========================================================================

play_cinematic_script::play_cinematic_script ( guid ParentGuid ) : actions_base( ParentGuid )
{}

//=============================================================================

void play_cinematic_script::Execute ( trigger_object* pParent )
{
    (void) pParent;

    TRIGGER_CONTEXT( "ACTION * play_cinematic_script::Execute" );
}

//=============================================================================

void play_cinematic_script::OnRender ( void )
{

}

//=============================================================================

void play_cinematic_script::OnEnumProp	( prop_enum& rPropList )
{
    //object info
 
    actions_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool play_cinematic_script::OnProperty	( prop_query& rPropQuery )
{ 
    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;
  
    return FALSE;
}


