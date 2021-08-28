///////////////////////////////////////////////////////////////////////////
//
//  action_ai_lookat_guid.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_ai_lookat_guid.hpp"
#include "..\Support\Characters\Character.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_ai_lookat_guid::action_ai_lookat_guid ( guid ParentGuid ) : 
    action_ai_base( ParentGuid ),
    m_HeadLookat( FALSE ),
    m_SightDistance(3000.0f),
    m_SightFOV(45.0f),
    m_NextAiState(character_state::STATE_IDLE)
{
}

//=============================================================================

void action_ai_lookat_guid::OnEnumProp	( prop_enum& rPropList )
{
    action_ai_base::OnEnumProp( rPropList );

    //guid specific fields
    m_TargetAffecter.OnEnumProp( rPropList, "LookAt" );

    rPropList.PropEnumBool ( "HeadLookat", "If true this sets what we lookat with our head (won't play turn anims to look at it)", 0);
    rPropList.PropEnumFloat( "SightDist", "Sight distance that target must be within", 0);
    rPropList.PropEnumFloat( "SightFOV",  "Sight FOV that target must be within", 0 );

    if( m_TriggerGuid )
    {   
        rPropList.PropEnumEnum    ( "Next State" ,    character::GetStatesEnum(), "State to transition the AI to after the action is complete.", 0 );
    }
}

//=============================================================================

xbool action_ai_lookat_guid::OnProperty	( prop_query& rPropQuery )
{
    if( action_ai_base::OnProperty( rPropQuery ) )
        return TRUE;

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

    if (rPropQuery.VarBool("HeadLookat",m_HeadLookat))
        return TRUE;

    if (rPropQuery.VarFloat("SightDist",m_SightDistance))
        return TRUE;

    if (rPropQuery.VarFloat("SightFOV",m_SightFOV))
        return TRUE;

    //guid specific fields
    if( m_TargetAffecter.OnProperty( rPropQuery, "LookAt" ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char* action_ai_lookat_guid::GetDescription( void )
{
    static big_string   Info;

    // Guid specified?
    if( m_TargetAffecter.GetGuid() != 0 )
    {
        static med_string   AIName;
        static med_string   ObjectName;
        AIName.Set( GetAIName() );
        ObjectName.Set( m_TargetAffecter.GetObjectInfo() );
        if( m_HeadLookat )
        {
            Info.Set(xfs("%s Head Look at %s", AIName.Get(), ObjectName.Get()));          
        }
        else
        {        
            Info.Set(xfs("%s Look at %s", AIName.Get(), ObjectName.Get()));          
        }
    }
    else
    {
        if( m_HeadLookat )
        {
            Info.Set( xfs("%s Clear Head look at", GetAIName() ) );
        }
        else
        {
            Info.Set( xfs("%s Clear look at", GetAIName() ) );
        }
    }
    
    return Info.Get();
}

//=============================================================================

guid action_ai_lookat_guid::GetLookAtGuid( void )
{
    guid LookAt = 0;

    object* pObject = m_TargetAffecter.GetObjectPtr();
    //ASSERT(pObject);
    if (pObject)
    {
        LookAt = pObject->GetGuid();
    }

    return LookAt;
}

//=============================================================================

#ifdef X_EDITOR

object_affecter* action_ai_lookat_guid::GetObjectRef1( xstring& Desc )
{
    // Only validate guid if specified
    if( m_TargetAffecter.GetGuid() != 0 )
    {
        Desc = "Target object error: "; 
        return &m_TargetAffecter; 
    }
    else
    {
        return NULL;
    }
}

#endif

//=============================================================================
