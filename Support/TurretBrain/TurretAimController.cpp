#include "TurretAimController.hpp"


//=========================================================================

void turret_aim_controller::SetL2W( const matrix4& L2W )
{
    m_L2W = L2W;
    m_W2L = L2W;
    m_W2L.Invert();
}

 //=========================================================================

turret_aim_controller::turret_aim_controller()
{
    m_PitchSpeed            = R_30;
    m_YawSpeed              = R_30;
    m_PitchUpLimit          = R_90;
    m_PitchDownLimit        = R_90;
    m_YawRightLimit         = R_180;
    m_YawLeftLimit          = R_180;
    m_Pitch                 = 0;
    m_Yaw                   = 0;
    m_L2W.Identity();
    m_W2L.Identity();
    m_LocalAimDir.Set(0,0,1);
}

//=========================================================================

void turret_aim_controller::SetSpeeds       ( f32  PitchSpeed,    f32  YawSpeed       )
{
    m_PitchSpeed = PitchSpeed;
    m_YawSpeed   = YawSpeed;
}

//=========================================================================

void turret_aim_controller::SetPitchSpeed( f32 P )
{
    m_PitchSpeed = P;
}

//=========================================================================

void turret_aim_controller::SetYawSpeed( f32 Y )
{
    m_YawSpeed = Y;
}

//=========================================================================

void turret_aim_controller::GetSpeeds       ( f32& PitchSpeed,    f32& YawSpeed       ) const
{
    PitchSpeed = m_PitchSpeed;  
    YawSpeed   = m_YawSpeed;
}

//=========================================================================

f32 turret_aim_controller::GetPitchSpeed( void ) const
{
    return m_PitchSpeed;      
}

//=========================================================================

f32 turret_aim_controller::GetYawSpeed( void ) const
{
    return m_YawSpeed;
}

//=========================================================================

void turret_aim_controller::SetYawLimits    ( radian  YawRightLimit, radian  YawLeftLimit   )
{
    YawRightLimit = MIN(R_360,MIN(0,YawRightLimit));
    YawLeftLimit  = MIN(R_360,MIN(0,YawLeftLimit ));
    m_YawRightLimit = YawRightLimit;
    m_YawLeftLimit  = YawLeftLimit;
}

//=========================================================================

void turret_aim_controller::SetYawRightLimit( radian  YawRightLimit )
{
    YawRightLimit = MIN(R_360,MIN(0,YawRightLimit));
    m_YawRightLimit = YawRightLimit;    
}

//=========================================================================

void turret_aim_controller::SetYawLeftLimit( radian  YawLeftLimit )
{
    YawLeftLimit = MIN(R_360,MIN(0,YawLeftLimit));
    m_YawLeftLimit = YawLeftLimit;    
}

//=========================================================================

void turret_aim_controller::GetYawLimits    ( radian& YawRightLimit, radian& YawLeftLimit   ) const
{
    YawRightLimit = m_YawRightLimit;
    YawLeftLimit  = m_YawLeftLimit;
}

//=========================================================================

radian turret_aim_controller::GetYawRightLimit( void ) const
{
    return m_YawRightLimit;    
}

//=========================================================================

radian turret_aim_controller::GetYawLeftLimit( void ) const
{
    return m_YawLeftLimit;
}

//=========================================================================

void turret_aim_controller::SetPitchLimits  ( radian  PitchUpLimit,  radian  PitchDownLimit )
{
    PitchUpLimit   = MIN(R_90,MIN(0,PitchUpLimit));
    PitchDownLimit = MIN(R_90,MIN(0,PitchDownLimit));

    m_PitchUpLimit   = PitchUpLimit;
    m_PitchDownLimit = PitchDownLimit;
}

//=========================================================================

void turret_aim_controller::SetPitchUpLimit( radian  PitchUpLimit )
{
    PitchUpLimit   = MIN(R_90,MIN(0,PitchUpLimit));
    
    m_PitchUpLimit   = PitchUpLimit;    
}

//=========================================================================

void turret_aim_controller::SetPitchDownLimit( radian  PitchDownLimit )
{
    PitchDownLimit = MIN(R_90,MIN(0,PitchDownLimit));

    m_PitchDownLimit = PitchDownLimit;
}


//=========================================================================

void turret_aim_controller::GetPitchLimits  ( radian& PitchUpLimit,  radian& PitchDownLimit ) const
{
    PitchUpLimit   = m_PitchUpLimit;
    PitchDownLimit = m_PitchDownLimit;
}

//=========================================================================

radian turret_aim_controller::GetPitchUpLimit  ( void ) const
{
    return m_PitchUpLimit;    
}

//=========================================================================

radian turret_aim_controller::GetPitchDownLimit( void ) const
{
    return m_PitchDownLimit;
}


//=========================================================================

xbool turret_aim_controller::UpdateAim       ( f32 DeltaTime, radian   DesiredPitch, radian DesiredYaw )
{
    radian DeltaP, DeltaY;
    radian TurnP,  TurnY;
      
    DeltaP = x_MinAngleDiff( DesiredPitch, m_Pitch );
    DeltaY = x_MinAngleDiff( DesiredYaw,   m_Yaw   );
    TurnP  = m_PitchSpeed * DeltaTime;
    TurnY  = m_YawSpeed * DeltaTime;

    if (m_bYawLimited)
    {
        if ((DesiredYaw < (R_360-m_YawLeftLimit) ) &&
            (DesiredYaw > m_YawRightLimit))
        {
            // Desired yaw is outside turret limits
            radian DeltaLeft  = x_MinAngleDiff( m_YawLeftLimit,  DesiredYaw );
            radian DeltaRight = x_MinAngleDiff( R_360-m_YawRightLimit, DesiredYaw );

            if (DeltaLeft < DeltaRight)
                DeltaY = TurnY;
            else
                DeltaY = -TurnY;
        }             
    }
/*
    if (m_bPitchLimited)
    {
        if ((Pitch < R_360-m_YawLeftLimit) ||
            (Pitch > m_YawRightLimit))
        {
            // Desired yaw is outside turret limits
            radian DeltaLeft  = x_MinAngleDiff( m_YawLeftLimit,  Yaw );
            radian DeltaRight = x_MinAngleDiff( m_YawRightLimit, Yaw );

            DeltaY = MIN( DeltaLeft, DeltaRight );
        }     
    }
  */  
    if( x_abs( DeltaP ) < TurnP )
    {
        m_Pitch = DesiredPitch;
    }
    else
    {
        if( DeltaP > R_0 )  
            m_Pitch += TurnP;
        else                
            m_Pitch -= TurnP;
    }

    if( x_abs( DeltaY ) < TurnY )
    {
        m_Yaw = DesiredYaw;     
    }
    else
    {
        if( DeltaY > R_0 )  
            m_Yaw += TurnY;
        else
            m_Yaw -= TurnY;
    }

    m_Yaw   = x_ModAngle( m_Yaw   );
    m_Pitch = x_ModAngle( m_Pitch );

    LimitPitchYaw( m_Pitch, m_Yaw );


    // Are we locked on?
    m_LocalAimDir.Set(0,0,-1);
    m_LocalAimDir.Rotate( radian3( -m_Pitch, m_Yaw, 0 ));

    return FALSE; //TODO: return true when closely aligned
}

//=========================================================================

xbool turret_aim_controller::UpdateAim       ( f32 DeltaTime, const radian3& DesiredRotation )
{
    return UpdateAim( DeltaTime, DesiredRotation.Pitch, DesiredRotation.Yaw );
}

//=========================================================================

void turret_aim_controller::GetAim          ( radian& CurrentPitch,  radian& CurrentYaw ) const
{
    CurrentPitch = m_Pitch;
    CurrentYaw   = m_Yaw;
}

//=========================================================================

f32 turret_aim_controller::GetAimScoreForPoint ( const vector3& Pt, const vector3& aSensorPos )
{
    vector3     TargetPos = Pt;
    vector3     SensorPos = aSensorPos;
    
    TargetPos = m_W2L.Transform( TargetPos );
    SensorPos = m_W2L.Transform( SensorPos );

    vector3 Delta = TargetPos - SensorPos;

    radian  Yaw,Pitch;
    Delta.GetPitchYaw(Pitch,Yaw);
    Delta.Normalize();

    Pitch = x_ModAngle2( Pitch );
    Yaw   = x_ModAngle2( Yaw+R_180 );

    xbool bInYaw = FALSE;
    xbool bInPitch = FALSE;

    if ((Yaw < m_YawLeftLimit) && (Yaw > -m_YawRightLimit))
        bInYaw = TRUE;
    
    if ((Pitch < m_PitchDownLimit) && (Pitch > -m_PitchUpLimit))
        bInPitch = TRUE;

    f32 Score;

    if (bInPitch && bInYaw)
    {
        // Inside FOF
        // Score will be positive       
        f32 Dot = Delta.Dot( m_LocalAimDir );

        // Massage from [-1,1] to [0,1]
        Score = (Dot+1)/2;
    }
    else
    {
        // Outside FOF
        // Score will be negative
        f32 Dot = Delta.Dot( m_LocalAimDir );

        // Massage from [-1,1] to [-1,0]
        Score = (Dot-1)/2;
    }

    return Score;
}

//=========================================================================

void turret_aim_controller::LimitPitchYaw   ( radian& Pitch, radian& Yaw )
{
    (void)Pitch;
    (void)Yaw;
}

//=========================================================================

void turret_aim_controller::OnEnumProp( prop_enum& List )
{
    List.AddFloat   ( "Pitch Speed", "Number of degrees the turret can pitch per second" );
    List.AddFloat   ( "Yaw Speed", "Number of degrees the turret can yaw per second" );

    List.AddAngle   ( "Pitch Up Limit", "Max angle the turret can rotate up to.  If 'Pitch Up Limit' and 'Pitch Down Limit' sum to 180 degrees, pitch limiting is turned off" );
    List.AddAngle   ( "Pitch Down Limit", "Max angle the turret can rotate down to.  If 'Pitch Up Limit' and 'Pitch Down Limit' sum to 180 degrees, pitch limiting is turned off" );

    List.AddAngle   ( "Yaw Right Limit", "Max angle the turret can rotate to the right.  If 'Yaw Right Limit' and 'Yaw Left Limit' sum to 360 degrees, yaw limiting is turned off" );
    List.AddAngle   ( "Yaw Left Limit", "Max angle the turret can rotate to the left.  If 'Yaw Right Limit' and 'Yaw Left Limit' sum to 360 degrees, yaw limiting is turned off" );        
}

//=============================================================================

xbool turret_aim_controller::OnProperty( prop_query& I )
{
    if( I.IsVar( "Pitch Speed" ))
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( RAD_TO_DEG( m_PitchSpeed ) );
        }
        else
        {
            m_PitchSpeed = DEG_TO_RAD( I.GetVarFloat() );
        }
        return TRUE;
    }
    else
    if( I.IsVar( "Yaw Speed" ))
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( RAD_TO_DEG( m_YawSpeed ) );
        }
        else
        {
            m_YawSpeed = DEG_TO_RAD( I.GetVarFloat() );
        }
        return TRUE;
    }
    else
    if( I.VarAngle( "Pitch Up Limit", m_PitchUpLimit) )
    {
        m_PitchUpLimit = MIN(R_90,MAX(R_0,m_PitchUpLimit));
        
        if ((m_PitchDownLimit + m_PitchUpLimit) < (R_360 * 0.995))
            m_bPitchLimited = TRUE;
        else 
            m_bPitchLimited = FALSE;
        return TRUE;
    }
    else
    if( I.VarAngle( "Pitch Down Limit", m_PitchDownLimit ) )
    {
        m_PitchDownLimit = MIN(R_90,MAX(R_0,m_PitchDownLimit));

        if ((m_PitchDownLimit + m_PitchUpLimit) < (R_360 * 0.995))
            m_bPitchLimited = TRUE;
        else 
            m_bPitchLimited = FALSE;
        return TRUE;
    }
    else
    if( I.VarAngle( "Yaw Right Limit", m_YawRightLimit) )
    {
        if (m_YawRightLimit + m_YawLeftLimit > R_360)
            m_YawLeftLimit = R_360 - m_YawRightLimit;

        if ((m_YawLeftLimit + m_YawRightLimit) < (R_360 * 0.995))
            m_bYawLimited = TRUE;
        else 
            m_bYawLimited = FALSE;
        return TRUE;
    }
    else
    if( I.VarAngle( "Yaw Left Limit", m_YawLeftLimit ) )
    {
        if (m_YawRightLimit + m_YawLeftLimit > R_360)
            m_YawRightLimit = R_360 - m_YawLeftLimit;

        if ((m_YawLeftLimit + m_YawRightLimit) < (R_360 * 0.995))
            m_bYawLimited = TRUE;
        else 
            m_bYawLimited = FALSE;
        return TRUE;
    }
    
    return( FALSE );
}

//=========================================================================

xbool turret_aim_controller::IsYawLimited( void ) const
{
    return m_bYawLimited;
}

//=========================================================================

xbool turret_aim_controller::IsPitchLimited( void ) const
{
    return m_bPitchLimited;
}

//=========================================================================

xbool turret_aim_controller::IsAimedAt       ( radian Pitch, radian Yaw, radian WithinFOV )
{
    vector3 TargetDir(0,0,-1);
    TargetDir.Rotate( radian3( -Pitch, Yaw, 0 ));

    f32 FiringCone = m_LocalAimDir.Dot( TargetDir );
    if (FiringCone > 0.9999f)
        return TRUE;

    f32 Angle = x_acos( FiringCone );

    if (Angle <= WithinFOV)
        return TRUE;
    
    return FALSE;

}

//=========================================================================

vector3 turret_aim_controller::GetLocalAimDir  ( void )
{
    return m_LocalAimDir;
}

//=========================================================================

//=========================================================================

//=========================================================================

//=========================================================================

//=========================================================================

//=========================================================================

//=========================================================================
