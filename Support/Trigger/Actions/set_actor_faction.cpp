///////////////////////////////////////////////////////////////////////////
//
//  set_actor_faction.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\set_actor_faction.hpp"
#include "Entropy.hpp"
#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Objects\Actor\Actor.hpp"

static const xcolor s_ActivateColor         (115,115,0);

//=========================================================================
// SET_ACTOR_FACTION
//=========================================================================

set_actor_faction::set_actor_faction( guid ParentGuid ) : 
    actions_base( ParentGuid ),
    m_TriggeredFaction( FACTION_NONE )
{
}

//=============================================================================

void  set_actor_faction::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * set_actor_faction::Execute" );

    ( void ) pParent;

    // Let's grab the actor who we are setting.
    object_ptr< actor > ActorPtr( m_ActorGuid );
    if ( ActorPtr.IsValid() )
    {
        ActorPtr.m_pObject->SetFaction( m_TriggeredFaction );
    }
}

//=============================================================================

void set_actor_faction::OnEnumProp ( prop_enum& rPropList )
{
    actions_base::OnEnumProp( rPropList );

    rPropList.AddGuid( "Actor", "The guid of the actor whose faction you want to change." );

    factions_manager::OnEnumFaction( rPropList );
}

//=============================================================================

xbool set_actor_faction::OnProperty ( prop_query& rPropQuery )
{
    if ( actions_base::OnProperty( rPropQuery ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarGUID( "Actor", m_ActorGuid ) )
    {
        return TRUE;
    }

    if ( factions_manager::OnPropertyFaction( rPropQuery, m_TriggeredFaction ) )
    {
        return TRUE;
    }

    return FALSE;
}

//=============================================================================

void set_actor_faction::OnRender ( void )
{
    object_ptr<object> ActorPtr( m_ActorGuid );

    if ( !ActorPtr.IsValid() )
        return;
#ifdef TARGET_PC
    vector3 MyPosition = GetPositionOwner() + SMP_UTIL_RandomVector(k_rand_draw_displace_amt);
    draw_Line( MyPosition, ActorPtr.m_pObject->GetPosition(), s_ActivateColor );
    draw_BBox( ActorPtr.m_pObject->GetBBox(), s_ActivateColor );
    draw_Label( ActorPtr.m_pObject->GetPosition(), s_ActivateColor, GetTypeName() );
#endif
}
