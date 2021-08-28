//==============================================================================
#ifndef __TURRET_AIMCONTROLLER_HPP__
#define __TURRET_AIMCONTROLLER_HPP__
//==============================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "Animation\AnimPlayer.hpp"

//=========================================================================
// CLASS
//=========================================================================
//
//  *+*+*+*+*  m_CurrentRot stores values in local space.  *+*+*+*+*
//
//  GetAim returns local space values.  You will need to add in the world
//  space rotation values for the object running the aim controller
//
//=========================================================================

class turret_aim_controller
{
public:
                    turret_aim_controller();

    virtual void    SetL2W          ( const matrix4& L2W );

    void            SetSpeeds       ( f32     PitchSpeed,    f32     YawSpeed       );
    void            SetPitchSpeed   ( f32     PitchSpeed );
    void            SetYawSpeed     ( f32     YawSpeed   );
    
    void            GetSpeeds       ( f32&    PitchSpeed,    f32&    YawSpeed       ) const;
    f32             GetPitchSpeed   ( void ) const;
    f32             GetYawSpeed     ( void ) const;
    

    void            SetYawLimits      ( radian  YawRightLimit, radian  YawLeftLimit   );
    void            SetYawLeftLimit   ( radian  YawLeftLimit  );
    void            SetYawRightLimit  ( radian  YawRightLimit );

    void            GetYawLimits      ( radian& YawRightLimit, radian& YawLeftLimit   ) const;
    radian          GetYawLeftLimit   ( void ) const;
    radian          GetYawRightLimit  ( void ) const;

    void            SetPitchLimits    ( radian  PitchUpLimit,  radian  PitchDownLimit );
    void            SetPitchUpLimit   ( radian  PitchUpLimit );
    void            SetPitchDownLimit ( radian  PitchDownLimit );

    void            GetPitchLimits    ( radian& PitchUpLimit,  radian& PitchDownLimit ) const;
    radian          GetPitchUpLimit   ( void ) const;
    radian          GetPitchDownLimit ( void ) const;

    xbool           UpdateAim       ( f32 DeltaTime, radian   DesiredPitch, radian DesiredYaw );
    xbool           UpdateAim       ( f32 DeltaTime, const radian3& DesiredRotation );

    void            GetAim          ( radian& CurrentPitch,  radian& CurretnYaw ) const;

    f32             GetAimScoreForPoint ( const vector3& Pt, const vector3& aSensorPos );

    xbool           IsYawLimited    ( void ) const;
    xbool           IsPitchLimited  ( void ) const;
    //xbool           IsAimedAt       ( radian Pitch, radian Yaw ) const;    


    void            OnEnumProp      ( prop_enum& List );
    xbool           OnProperty      ( prop_query& I );

    radian          GetAimDelta     ( vector3   TargetDir );

    xbool           IsAimedAt       ( radian Pitch, radian Yaw, radian WithinFOV );
    vector3         GetLocalAimDir  ( void );


protected:
    void            LimitPitchYaw   ( radian& Pitch, radian& Yaw );

    
protected:

    // Current status
    radian          m_Pitch;
    radian          m_Yaw;
    vector3         m_LocalAimDir;

    matrix4         m_L2W;
    matrix4         m_W2L;
    
    // Speeds
    f32             m_PitchSpeed;           // Speed for pitching the turret 
    f32             m_YawSpeed;             // Speed for yawing the turret

    // Limits
    radian          m_YawRightLimit;
    radian          m_YawLeftLimit;
    radian          m_PitchUpLimit;
    radian          m_PitchDownLimit;

    // Flags
    u16             m_bYawLimited:1,            // Yaw limits do not sum to 360deg
                    m_bPitchLimited:1;          // Pitch limits do not sum to 360deg


};

//==============================================================================
#endif // __TURRET_BRAIN_HPP__
//==============================================================================