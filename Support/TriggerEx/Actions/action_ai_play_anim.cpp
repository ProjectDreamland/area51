///////////////////////////////////////////////////////////////////////////
//
//  action_ai_play_anim.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_ai_play_anim.hpp"
#include "Characters\Character.hpp"
#include "Loco\LocoUtil.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_ai_play_anim::action_ai_play_anim ( guid ParentGuid ) : action_ai_base( ParentGuid ),
m_InterruptAI(TRUE),
m_AnimGroupName(-1),
m_AnimName(-1),
m_AnimBlendTime( DEFAULT_BLEND_TIME ),
m_AnimPlayTime(1.0f),
m_AnimFlags(0),
m_NextAiState(character_state::STATE_IDLE)
{
}

//=============================================================================

void action_ai_play_anim::OnEnumProp	( prop_enum& rPropList )
{
    action_ai_base::OnEnumProp( rPropList );

    // Add loco animation properties
    LocoUtil_OnEnumPropAnimFlags( rPropList,
                                  loco::ANIM_FLAG_PLAY_TYPE_ALL        | 
                                  loco::ANIM_FLAG_END_STATE_ALL        |
                                  loco::ANIM_FLAG_INTERRUPT_BLEND      |
                                  loco::ANIM_FLAG_TURN_OFF_AIMER       |
                                  loco::ANIM_FLAG_MASK_TYPE_ALL        |
                                  loco::ANIM_FLAG_RESTART_IF_SAME_ANIM,
                                  m_AnimFlags ) ;

    if( !m_bIsBlockingAction )
    {
        rPropList.PropEnumBool      ( "Interrupt AI",  "Are we interrupting the flow of the AI to play this anim, NOTE can't block on this action if FALSE?", 0 ) ;
    }
    rPropList.PropEnumFloat     ( "AnimBlendTime", "Time to blend into the anim", 0 ) ;

    m_TargetAffecter.OnEnumProp( rPropList, "Scaled To Target" );

    if( m_TriggerGuid )
    {   
        rPropList.PropEnumEnum    ( "Next State" ,    character::GetStatesEnum(), "State to transition the AI to after the action is complete.", 0 );
    }
}

//=============================================================================

xbool action_ai_play_anim::OnProperty	( prop_query& rPropQuery )
{
    if( action_ai_base::OnProperty( rPropQuery ) )
        return TRUE;

    // Check for loco animation property
    if (LocoUtil_OnPropertyAnimFlags(rPropQuery, 
                                     m_AnimGroupName, 
                                     m_AnimName, 
                                     m_AnimFlags, 
                                     m_AnimPlayTime))
    {
        return TRUE ;
    }

    if( rPropQuery.VarFloat( "AnimBlendTime", m_AnimBlendTime ) )
        return TRUE;
    else if( rPropQuery.VarBool("Interrupt AI", m_InterruptAI) )
    {    
        return TRUE;
    }
    else if( m_TargetAffecter.OnProperty( rPropQuery, "Scaled To Target" ) )
    {
        return TRUE;
    }
    else if ( rPropQuery.IsVar( "Next State" ) ) 
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

    return FALSE;
}

//=============================================================================

const char* action_ai_play_anim::GetDescription( void )
{
    static big_string   Info;
    static med_string   AnimText;

    if (m_AnimName == -1 )
    {
        AnimText.Set("Unknown");
    }
    else
    {
        AnimText.Set(g_StringMgr.GetString(m_AnimName));
    }

    Info.Set(xfs("%s Play Anim %s", GetAIName(), AnimText.Get()));          
    return Info.Get();
}

//=============================================================================

guid action_ai_play_anim::GetScaledTargetGuid( void )
{
    guid Target = 0;

    object* pObject = m_TargetAffecter.GetObjectPtr();
    if (pObject)
    {
        Target = pObject->GetGuid();
    }
    return Target;
}
