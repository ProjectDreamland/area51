///////////////////////////////////////////////////////////////////////////
//
//  change_player_health.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\change_player_health.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"

#include "Objects\Player.hpp"
#include "Entropy.hpp"

//=========================================================================
// CHANGE_PLAYER_HEALTH
//=========================================================================


change_player_health::change_player_health ( guid ParentGuid ) : actions_base( ParentGuid ),
m_Health(0.0f),
m_ShakeView(FALSE),
m_ShakeTime( 3.f ),
m_ShakeForce( 100.f )
{
}

//=============================================================================

void change_player_health::Execute ( trigger_object* pParent )
{ 
    TRIGGER_CONTEXT( "ACTION * change_player_health::Execute" );

/*
    if (pParent->GetTriggerActor()==NULL)
        return;

    object_ptr<player> PlayerObj( *pParent->GetTriggerActor() );

    if (!PlayerObj.IsValid())
        return;
*/
    ( void )pParent;
    object_ptr<player> PlayerObj = SMP_UTIL_GetActivePlayer();
    if ( ! PlayerObj.IsValid() )
    {
        return;
    }


    pain PainEvent;

    PainEvent.Type      = pain::PAIN_ON_TRIGGER;
    PainEvent.Center    = PlayerObj.m_pObject->GetPosition();
    PainEvent.Origin    = pParent->GetGuid();
    PainEvent.PtOfImpact= PlayerObj.m_pObject->GetPosition();

    PainEvent.DamageR0  = -m_Health; 
    PainEvent.DamageR1  = -m_Health; 

    PainEvent.RadiusR0  = 100.0f ;
    PainEvent.RadiusR1  = 100.0f ;

    if (m_ShakeView)
	{
		PainEvent.ForceR0 = PainEvent.ForceR1 = m_ShakeForce;
		PlayerObj.m_pObject->OnPain( PainEvent );
		PlayerObj.m_pObject->ShakeView( m_ShakeTime );		
	}
	else
	{
		PlayerObj.m_pObject->OnPain( PainEvent );
	}
}

//=============================================================================

void change_player_health::OnEnumProp ( prop_enum& rPropList )
{
    //object info
    rPropList.AddFloat ( "Health Amount" ,  "Amount of health to add to the player." );
     
    rPropList.AddBool ( "Shake View" ,  "Flag to determine whether to shake the players view." );

	rPropList.AddFloat( "Shake Time", "How long to shake the view" );

	rPropList.AddFloat( "Shake Force", "How much force to shake the view with" );

    actions_base::OnEnumProp( rPropList );
    
}

//=============================================================================

xbool change_player_health::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.VarFloat ( "Health Amount"  , m_Health ) )
        return TRUE;
    
    if ( rPropQuery.VarBool ( "Shake View"   , m_ShakeView ) )
        return TRUE;

	if ( rPropQuery.VarFloat( "Shake Time" , m_ShakeTime) )
	{
		return TRUE;
	}
    
	if ( rPropQuery.VarFloat( "Shake Force", m_ShakeForce ) )
	{
		return TRUE;
	}

    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;
    
    return FALSE;
}



