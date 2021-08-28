///////////////////////////////////////////////////////////////////////////
//
//  action_ai_pathto_guid.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_ai_pathto_guid.hpp"
#include "..\Support\Characters\Character.hpp"
#include "objects\group.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_ai_pathto_guid::action_ai_pathto_guid ( guid ParentGuid ) : action_ai_base( ParentGuid ),
m_Distance(50.0f),
m_YawThreshold(R_180),
m_MoveToStyle(loco::MOVE_STYLE_WALK),
m_NextAiState(character_state::STATE_IDLE),
m_Retreating(FALSE),
m_bAlignExact(FALSE),
m_bGotoExact(FALSE)
{
}

//=========================================================================

xbool action_ai_pathto_guid::Execute( f32 DeltaTime )
{
    return action_ai_base::Execute(DeltaTime);
}

//=============================================================================

void action_ai_pathto_guid::OnEnumProp	( prop_enum& rPropList )
{
    action_ai_base::OnEnumProp( rPropList );

    //guid specific fields
    m_LocationAffecter.OnEnumProp( rPropList, "Destination" );

    rPropList.PropEnumEnum ( "Move Style", loco::GetMoveStyleEnum(), "What style of movement to use to get to location.", PROP_TYPE_MUST_ENUM);
    rPropList.PropEnumBool ( "Retreating", "If this is true, we are trying to get away from the target.", PROP_TYPE_MUST_ENUM);
    
    rPropList.PropEnumBool ( "Goto Exact", "If this is true, the character hit the marker's position exactly.", PROP_TYPE_MUST_ENUM);
    if( !m_bGotoExact )
    {
        rPropList.PropEnumFloat( "Distance",   "Minimum distance to target pos to consider task complete. (can not be less than 1)", 0 );
    }

    rPropList.PropEnumBool ( "Align Exact", "If this is true, the character will align with the target guid's (hopefully a marker) orientation.", PROP_TYPE_MUST_ENUM);
    if( !m_bAlignExact )
    {    
        rPropList.PropEnumAngle( "Yaw Threshold", "Threshold for yaw, if we are more than this much off NPC will turn.", PROP_TYPE_MUST_ENUM);
    }

    if( m_TriggerGuid )
    {   
        rPropList.PropEnumEnum    ( "Next State" ,    character::GetStatesEnum(), "State to transition the AI to after the action is complete.", 0 );
    }
}

//=============================================================================

xbool action_ai_pathto_guid::OnProperty	( prop_query& rPropQuery )
{
    if( action_ai_base::OnProperty( rPropQuery ) )
        return TRUE;
  
    if ( rPropQuery.VarBool( "Retreating", m_Retreating) )
        return TRUE;

    if ( rPropQuery.VarBool( "Must Align", m_bGotoExact) )
        return TRUE;

    if ( rPropQuery.VarBool( "Align Exact", m_bAlignExact) )
        return TRUE;

    if ( rPropQuery.VarBool( "Goto Exact", m_bGotoExact) )
        return TRUE;

    if (rPropQuery.VarAngle("Yaw Threshold",m_YawThreshold))
        return TRUE;

    if ( rPropQuery.IsVar( "Move Style") )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarEnum( loco::GetMoveStyleName( m_MoveToStyle ) ); 
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            m_MoveToStyle = loco::GetMoveStyleByName  ( pString) ;
            return( TRUE );
        }
    }

    if ( rPropQuery.IsVar( "Next State" ) ) 
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarEnum( character::GetStateName(m_NextAiState) ); 
            return TRUE;
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            m_NextAiState = (s32) character::GetStateByName  ( pString) ;
            return TRUE;
        }
    }

    if (rPropQuery.VarFloat("Distance",m_Distance))
    {
        if ( m_Distance < 1.0f )
        {
            m_Distance = 1.0f;
        }
        return TRUE;
    }

    //guid specific fields
    if( m_LocationAffecter.OnProperty( rPropQuery, "Destination" ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char* action_ai_pathto_guid::GetDescription( void )
{
    static big_string   Info;
    static med_string   AIName;
    static med_string   MoveStyleName;
    static med_string   ObjectName;
    AIName.Set( GetAIName() );
    MoveStyleName.Set( loco::GetMoveStyleName( m_MoveToStyle ) );
    ObjectName.Set( m_LocationAffecter.GetObjectInfo() );
    Info.Set(xfs( "%s %s To %s", AIName.Get(), MoveStyleName.Get(), ObjectName.Get() ) );
    return Info.Get();
}

//=============================================================================

guid action_ai_pathto_guid::GetPathToGuid( void )
{
    guid Destination = 0;

    object* pObject = m_LocationAffecter.GetObjectPtr();
    if (pObject)
    {
        Destination = pObject->GetGuid();
    }

    return Destination;
}
