///////////////////////////////////////////////////////////////////////////
//
//  play_script.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\play_script.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"

#include "Entropy.hpp"

//=========================================================================
// PLAY_SCRIPT
//=========================================================================

play_script::play_script ( guid ParentGuid ) : actions_base( ParentGuid ),
m_ScriptID(0)
{
}

//=============================================================================

void play_script::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * play_script::Execute" );

    (void) pParent;
}

//=============================================================================

void play_script::OnEnumProp	( prop_enum& rPropList )
{
    //object info
    rPropList.AddInt ( "Script ID" , "ID of the script to play." );
    
    actions_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool play_script::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.VarInt ( "Script ID"  , m_ScriptID ) )
        return TRUE;
   
    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}