///////////////////////////////////////////////////////////////////////////////
//
//  action_player_camera_shake.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_player_camera_shake_
#define _action_player_camera_shake_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"

//=========================== ==============================================
// action_player_camera_shake
//=========================================================================

class action_player_camera_shake : public actions_ex_base
{
public:
                    action_player_camera_shake          ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_ACTION_PLAYER_CAMERA_SHAKE;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Shake the player view."; }
    virtual         const char*         GetDescription  ( void );

    virtual         xbool               Execute         ( f32 DeltaTime );    
    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

protected:
    
	f32			m_ShakeTime;						    // How long to shake the view.
    f32         m_CameraShakeAmount;                    // How much to shake the camera.
    f32         m_CameraShakeSpeed;                     // How fast to shake the camera.
	f32			m_FeedbackDuration; 					// Amount of force 
	f32			m_FeedbackIntensity;					// Amount of force 

};


#endif
