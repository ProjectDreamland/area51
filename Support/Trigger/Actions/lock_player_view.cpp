///////////////////////////////////////////////////////////////////////////
//
//  lock_player_view.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\lock_player_view.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"

#include "Entropy.hpp"
#include "Objects\Player.hpp"

static const xcolor s_ActivateColor         (0,255,0);

//=========================================================================
// lock_player_view
//=========================================================================

lock_player_view::lock_player_view ( guid ParentGuid ) : actions_base( ParentGuid )
{}

//=============================================================================

void lock_player_view::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * lock_player_view::Execute" );

    (void) pParent;

    if (pParent->GetTriggerActor()==NULL)
        return;

    object_ptr<player> PlayerObj( *pParent->GetTriggerActor() );

    if (!PlayerObj.IsValid())
        return;

    PlayerObj.m_pObject->PushViewCinematic( m_LockViewTable );
}

//=============================================================================

void lock_player_view::OnRender ( void )
{

}

//=============================================================================

void lock_player_view::OnEnumProp	( prop_enum& rPropList )
{
    //object info

    for (s32 i = 0; i < MAX_TABLE_SIZE; i++)
    { 
        med_string Header( xfs( "Node[%i]", i) );

        rPropList.AddHeader ( Header.Get(), "A node in the look table" );

        rPropList.AddFloat  (  xfs("%s\\TimeTo",   Header.Get()) , 
            "Amount of time to go from this node to the next.(in seconds, -1 means end of script.)" );
        rPropList.AddFloat  (  xfs("%s\\Linger",   Header.Get()) , 
            "Amount of time to linger at this look position( in seconds ).)" );
        rPropList.AddVector3(  xfs("%s\\LookAt", Header.Get()) , 
            "Look position.(in world space)" );
    }

    actions_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool lock_player_view::OnProperty	( prop_query& rPropQuery )
{
    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;
   
    if( rPropQuery.IsSimilarPath( "Node[]" ) )
    {
        s32 iIndex = rPropQuery.GetIndex(1);
        
        ASSERT( iIndex < MAX_TABLE_SIZE && iIndex >= 0);
        
        if ( rPropQuery.VarFloat(   "Node[]\\TimeTo", m_LockViewTable[iIndex].m_TimeTo ) )
        { 
            return TRUE;
        }
         
        if ( rPropQuery.VarFloat(   "Node[]\\Linger", m_LockViewTable[iIndex].m_Linger ) )
        { 
            return TRUE;
        }

        if ( rPropQuery.VarVector3( "Node[]\\LookAt" ,m_LockViewTable[iIndex].m_LookAt ) )
        {
            return TRUE;
        }   
    }
    
    return FALSE;
}


