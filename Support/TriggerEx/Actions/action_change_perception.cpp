///////////////////////////////////////////////////////////////////////////
//
//  action_change_perception.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_change_perception.hpp"

#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "PerceptionMgr\PerceptionMgr.hpp"

//=========================================================================
//=========================================================================

action_change_perception::action_change_perception ( guid ParentGuid ) : actions_ex_base( ParentGuid )
{
    m_TimeDelayingSoFar     = 0.0f;
    m_bHasBegun             = FALSE;
    m_bIsBeginChange        = TRUE;
    m_GlobalTimeDialation   = 1.0f;
    m_AudioTimeDialation    = 1.0f;
    m_ForwardSpeedFactor    = 1.0f;
    m_TurnRateFactor        = 1.0f;
    m_TimeRange             = 1.0f;
    m_bBlock                = FALSE;
}

//=============================================================================

void action_change_perception::OnActivate ( xbool Flag )
{
    (void)Flag;

    m_TimeDelayingSoFar     = 0.0f;
    m_bHasBegun             = FALSE;
}

//=============================================================================

xbool action_change_perception::Execute ( f32 DeltaTime )
{
    if (!m_bHasBegun)
    {
        g_PerceptionMgr.SetTriggerTargetGlobalTimeDialation ( m_GlobalTimeDialation );
        g_PerceptionMgr.SetTriggerTargetAudioTimeDialation  ( m_AudioTimeDialation );
        g_PerceptionMgr.SetTriggerTargetForwardSpeedFactor  ( m_ForwardSpeedFactor );
        g_PerceptionMgr.SetTriggerTargetTurnRateFactor      ( m_TurnRateFactor );

        if (m_bIsBeginChange)
        {
            g_PerceptionMgr.SetTriggerBeginLength           ( m_TimeRange );
            g_PerceptionMgr.BeginTriggerPerception          (  );
        }
        else
        {
            g_PerceptionMgr.SetTriggerEndLength             ( m_TimeRange );
            g_PerceptionMgr.EndTriggerPerception            (  );
        }

        if (m_bBlock)
        {
            m_bHasBegun = TRUE;
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }

    //we are blocking
    m_TimeDelayingSoFar += DeltaTime;

    if (m_TimeDelayingSoFar > m_TimeRange)
    {
        //done with delay
        m_TimeDelayingSoFar = 0.0f;
        m_bHasBegun         = FALSE;
        return TRUE;
    }

    //delay some more
    return FALSE;
}

//=============================================================================

void action_change_perception::OnEnumProp ( prop_enum& List )
{
    List.PropEnumEnum( "PerceptionChange" ,     "Change Perception\0Restore Perception\0", "Are we beginning or ending a perception change?", PROP_TYPE_MUST_ENUM );

    if (m_bIsBeginChange)
    {
        List.PropEnumFloat	 ( "GameTimeDilation", "1.0 is standard, less is slower, more is greater", 0 );
        List.PropEnumFloat	 ( "AudioTimeDilation", "1.0 is standard, less is slower, more is greater", 0 );
        List.PropEnumFloat	 ( "ForwardSpeedFactor", "1.0 is standard, less is slower, more is greater", 0 );
        List.PropEnumFloat	 ( "TurnRateFactor", "1.0 is standard, less is slower, more is greater", 0 );
    }

    List.PropEnumFloat	 ( "TimeLength", "Over what amount of time do these changes take place (in seconds).", 0 );
    List.PropEnumBool 	 ( "Block", "Should we block for the specified time length?", 0 );

    //object info
    actions_ex_base::OnEnumProp( List );
}

//=============================================================================

xbool action_change_perception::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.IsVar  ( "PerceptionChange" ) )
    {
        if( rPropQuery.IsRead() )
        {
            if (m_bIsBeginChange)
                rPropQuery.SetVarEnum( "Change Perception" );
            else
                rPropQuery.SetVarEnum( "Restore Perception" );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            if( x_stricmp( pString, "Change Perception" )==0)   { m_bIsBeginChange = TRUE; }
            if( x_stricmp( pString, "Restore Perception" )==0)  { m_bIsBeginChange = FALSE;}
        }
        return TRUE;
    }

    if ( rPropQuery.VarFloat("GameTimeDilation",m_GlobalTimeDialation))
        return TRUE;

    if ( rPropQuery.VarFloat("AudioTimeDilation",m_AudioTimeDialation))
        return TRUE;

    if ( rPropQuery.VarFloat("ForwardSpeedFactor",m_ForwardSpeedFactor))
        return TRUE;

    if ( rPropQuery.VarFloat("TurnRateFactor",m_TurnRateFactor))
        return TRUE;

    if ( rPropQuery.VarFloat("TimeLength",m_TimeRange))
        return TRUE;

    if ( rPropQuery.VarBool("Block",m_bBlock))
        return TRUE;

    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char* action_change_perception::GetDescription( void )
{
    if (m_bBlock)
    {
        if (m_bIsBeginChange)
            return "* Begin Perception Change";
        else
            return "* End Perception Change";
    }
    else
    {
        if (m_bIsBeginChange)
            return "Begin Perception Change";
        else
            return "End Perception Change";
    }
}

//=============================================================================


