///////////////////////////////////////////////////////////////////////////
//
//  set_actor_friends.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\set_actor_friends.hpp"
#include "Entropy.hpp"
#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Objects\Actor\Actor.hpp"

static const xcolor s_ActivateColor         (115,0,115);

//=========================================================================
// SET_ACTOR_FRIENDS
//=========================================================================

set_actor_friends::set_actor_friends ( guid ParentGuid ) : 
    actions_base( ParentGuid ),
    m_TriggeredFriends( 0 )
{
}

//=============================================================================

void  set_actor_friends::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * set_actor_friends::Execute" );

    ( void ) pParent;

    // Let's grab the actor who we are setting.
    object_ptr< actor > ActorPtr( m_ActorGuid );
    if ( ActorPtr.IsValid() )
    {
        ActorPtr.m_pObject->SetFriendFlags( m_TriggeredFriends );
    }
}

//=============================================================================

void set_actor_friends::OnEnumProp ( prop_enum& rPropList )
{
    actions_base::OnEnumProp( rPropList );

    rPropList.AddGuid( "Actor", "The guid of the actor whose faction you want to change." );

    factions_manager::OnEnumFriends( rPropList );
}

//=============================================================================

xbool set_actor_friends::OnProperty ( prop_query& rPropQuery )
{
    if ( actions_base::OnProperty( rPropQuery ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarGUID( "Actor", m_ActorGuid ) )
    {
        return TRUE;
    }

    if ( factions_manager::OnPropertyFriends( rPropQuery, m_TriggeredFriends ) )
    {
        return TRUE;
    }

    return FALSE;
}

//=============================================================================

void set_actor_friends::OnRender ( void )
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
