///////////////////////////////////////////////////////////////////////////
//
//  action_player_camera_shake.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_player_camera_shake.hpp"
#include "Objects\Player.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_player_camera_shake::action_player_camera_shake ( guid ParentGuid ) : actions_ex_base( ParentGuid ),
m_ShakeTime( 3.f ),
m_CameraShakeAmount(1.0f),
m_CameraShakeSpeed(1.0f),
m_FeedbackDuration( 0.5f ),
m_FeedbackIntensity( 0.8f )
{
}

//=============================================================================

xbool action_player_camera_shake::Execute ( f32 DeltaTime )
{
    (void) DeltaTime;

    player* pPlayerObj = SMP_UTIL_GetActivePlayer();
    if ( pPlayerObj )
    {
        pPlayerObj->ShakeView(m_ShakeTime, m_CameraShakeAmount, m_CameraShakeSpeed );

        //force feedback
        pPlayerObj->DoFeedback(m_FeedbackDuration, m_FeedbackIntensity );

        return TRUE;
    }

    m_bErrorInExecute = TRUE;
    return (!RetryOnError());
}

//=============================================================================

void action_player_camera_shake::OnEnumProp	( prop_enum& rPropList )
{
	rPropList.PropEnumFloat( "Shake Time",         "How long to shake the view", 0 );
    rPropList.PropEnumFloat( "Camera Shake Amount","How much to shake the camera", 0 );
    rPropList.PropEnumFloat( "Camera Shake Speed", "How fast to shake the camera", 0 );
	rPropList.PropEnumFloat( "Feedback Intensity", "How much force feedback in controller", 0 );
	rPropList.PropEnumFloat( "Feedback Duration",  "How long should force feedback last", 0 );

    actions_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool action_player_camera_shake::OnProperty	( prop_query& rPropQuery )
{
	if ( rPropQuery.VarFloat( "Shake Time" , m_ShakeTime) )
		return TRUE;
    
	if ( rPropQuery.VarFloat( "Feedback Intensity", m_FeedbackIntensity ) )
		return TRUE;

	if ( rPropQuery.VarFloat( "Feedback Duration", m_FeedbackDuration ) )
		return TRUE;

	if ( rPropQuery.VarFloat( "Camera Shake Amount", m_CameraShakeAmount ) )
    {
		m_CameraShakeAmount = MAX( 0.0f, m_CameraShakeAmount );
        m_CameraShakeAmount = MIN( 1.0f, m_CameraShakeAmount );
        return TRUE;
    }

	if ( rPropQuery.VarFloat( "Camera Shake Speed", m_CameraShakeSpeed ) )
    {
		m_CameraShakeSpeed = MAX( 0.0f, m_CameraShakeSpeed );
        m_CameraShakeSpeed = MIN( 1.0f, m_CameraShakeSpeed );
		return TRUE;
    }

    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;
  
    return FALSE;
}

//=============================================================================

const char* action_player_camera_shake::GetDescription( void )
{
    static big_string   Info;
    Info.Set("Shake Player View");
    return Info.Get();
}


