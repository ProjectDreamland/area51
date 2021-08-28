//==============================================================================
//
//  PlayerPhysics.cpp
// 
//==============================================================================
#if defined(bwatson)
#define X_SUPPRESS_LOGS
#endif
#include "player.hpp"
#include "InputMgr\GamePad.hpp"
#include "objects\ParticleEmiter.hpp"
#include "objects\Render\PostEffectMgr.hpp"
#include "objects\SpawnPoint.hpp"
#include "Objects\Event.hpp"
#include "Sound\EventSoundEmitter.hpp"
#include "..\support\templatemgr\TemplateMgr.hpp"
#include "characters\Character.hpp"
#include "Characters\Conversation_Packet.hpp"
#include "GameLib\StatsMgr.hpp"
#include "GameLib\RenderContext.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "objects\WeaponSniper.hpp"
#include "objects\ThirdPersonCamera.hpp"
#include "objects\WeaponSMP.hpp"
#include "objects\Corpse.hpp"
#include "NetworkMgr/NetObjMgr.hpp"
#include "NetworkMgr/Voice/VoiceMgr.hpp"
#include "Objects\Ladders\Ladder_Field.hpp"
#include "Objects\GrenadeProjectile.hpp"
#include "Objects\GravChargeProjectile.hpp"
#include "Objects\JumpingBeanProjectile.hpp"
#include "render\LightMgr.hpp"
#include "Objects\Door.hpp"
#include "objects\Projector.hpp"
#include "objects\WeaponMutation.hpp"
#include "StateMgr\StateMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "objects\HudObject.hpp"
#include "Configuration/GameConfig.hpp"
#include "objects\turret.hpp"
#include "objects\WeaponShotgun.hpp"
#include "Gamelib/DebugCheats.hpp"
#include "objects\FocusObject.hpp"
#include "PerceptionMgr\PerceptionMgr.hpp"
#include "Objects\LoreObject.hpp"
#include "Objects\Camera.hpp"
#include "Objects\JumpPad.hpp"

static f32 SlowYawMultiplier = 2.0f;
static f32 FastYawMultiplier = 5.2f;
static f32 StickModeChange   = 0.99f;
static f32 InitialYawMultiplier = 0.05f; // the smaller the faster start with a big stick change
static const f32 s_DistanceAtR25 = 700.0f;

f32 HumanSpeedFactor  = 1.000f;
f32 MutantSpeedFactor = 1.000f;

tweak_handle LookSpring_ReturnSpeedTweak("LookSpring_ReturnSpeed");

// multi-player movement tweaks
tweak_handle MP_RunSpeedFactor_NormalTweak      ("MP_RunSpeedFactor_Normal");
tweak_handle MP_StrafeSpeedFactor_NormalTweak   ("MP_StrafeSpeedFactor_Normal");
tweak_handle MP_RunSpeedFactor_MutantTweak      ("MP_RunSpeedFactor_Mutant");
tweak_handle MP_StrafeSpeedFactor_MutantTweak   ("MP_StrafeSpeedFactor_Mutant");

void player::UpdateRotation( const f32& rDeltaTime )
{
    CONTEXT( "player::UpdateRotation" );

    radian OldPitch = m_Pitch;
    radian OldYaw   = m_Yaw;

    f32 fAimYawOffset = R_0;

    // make sure we load in tweaks in case they've changed
    LoadAimAssistTweaks();

    CalculatePitchLimits( rDeltaTime );
    CalculateRotationAccelerationFactors( rDeltaTime );
    UpdateAimAssistance ( rDeltaTime );
    UpdateAimOffset     ( rDeltaTime );

    //
    // If the current weapon had zoom enable, adjust the Yaw according to the X FOV.
    //
    new_weapon* pWeaponObj = GetCurrentWeaponPtr();

    if( pWeaponObj )
    {       
        if( pWeaponObj->IsZoomEnabled() )
        {
            m_fYawValue /= (m_OriginalViewInfo.XFOV / pWeaponObj->GetXFOV());
            m_fYawValue *= pWeaponObj->GetZoomMovementMod();
            m_fPitchValue *= pWeaponObj->GetZoomMovementMod();
        }
    }

    if( rDeltaTime < F32_MIN )
    {
        ASSERT(0);
    }

    if ( m_AimAssistData.TargetGuid != 0 )
    {
        static f32 s_AimAssistTune  = 1.06f;

        fAimYawOffset   = m_YawAimOffset
            * m_fStrafeValue
            * rDeltaTime 
            * ( -s_DistanceAtR25 / m_AimAssistData.LOFPtDist )
            //* ( -s_DistanceAtR25 / m_DistanceToAimAssistTarget ) 
            * m_AimAssistData.TurnDampeningT //m_AimAssistPct 
            * s_AimAssistTune;
    }

#if 0
    LOG_MESSAGE("player::UpdateRotation", "fAimYawOffset = %f", fAimYawOffset );
    LOG_MESSAGE("player::UpdateRotation", "m_YawAimOffset = %f", m_YawAimOffset );
    LOG_MESSAGE("player::UpdateRotation", "LOFPtDis = %f", m_AimAssistData.LOFPtDist );
    LOG_MESSAGE("player::UpdateRotation", "TurnDampeningT = %f", m_AimAssistData.TurnDampeningT );
    LOG_MESSAGE("player::UpdateRotation", "m_fStrafeValue = %f", m_fStrafeValue );
    LOG_MESSAGE("player::UpdateRotation", "m_fCurrentYawAimModifier = %f", m_fCurrentYawAimModifier);
#endif

    const f32 AbsYawValue = x_abs( m_fYawValue );
    const f32 YawChange = x_abs( m_fYawValue - m_fPreviousYawValue );

    static f32 MaxYawChange = 0.0f;
    if ( YawChange > MaxYawChange ) MaxYawChange = YawChange;

    if ( (AbsYawValue > 0.0f) && (AbsYawValue < StickModeChange) )
    {
        f32 P = m_fYawValue * SlowYawMultiplier * rDeltaTime;
        m_Yaw   += P;
        m_YawAccelFactor = P / (m_fYawValue * FastYawMultiplier * rDeltaTime * m_fCurrentYawAimModifier);
        m_YawAccelFactor = MIN( 1.0f, m_YawAccelFactor );
        m_YawAccelFactor = MAX( 0.0f, m_YawAccelFactor );
    }
    else
    {
        if ( YawChange >= StickModeChange )
        {
            // The stick just moved really far, really fast, give ourselves a nice kick start rather than a soft start
            m_YawAccelFactor = MAX( m_YawAccelFactor, rDeltaTime / (m_YawAccelTime * InitialYawMultiplier) );
            m_YawAccelFactor = MIN( 1.0f, m_YawAccelFactor );
            m_YawAccelFactor = MAX( 0.0f, m_YawAccelFactor );
        }

        f32 P = m_fYawValue * FastYawMultiplier * rDeltaTime * m_YawAccelFactor * m_fCurrentYawAimModifier;
        m_Yaw   += P;
    }

    m_Yaw += fAimYawOffset;

    m_Pitch += m_fPitchValue * rDeltaTime * m_fPitchStickSensitivity * m_PitchAccelFactor * m_fCurrentPitchAimModifier;
    m_Pitch  = MIN( m_PitchMax, MAX( m_PitchMin , m_Pitch ) );

    if ( m_bInTurret )
    {
        // Make sure we're within the turret's boundaries
        object* pObj;
        f32 MinAngleDiff = x_MinAngleDiff( m_Yaw, OldYaw );
        vector3 EyePos( m_Turret.AnchorL2W.GetTranslation() );
        EyePos.GetY() += 150.0f;

        // Left
        if (   (MinAngleDiff > 0.0f) // rotating left
            && (pObj = g_ObjMgr.GetObjectByGuid( m_Turret.LeftBoundaryGuid )) )
        {
            const vector3 Pos( pObj->GetPosition() );
            const vector3 ToPos( Pos - EyePos );
            const f32 LeftYawBound = ToPos.GetYaw();

            if ( (x_MinAngleDiff( m_Yaw, LeftYawBound )    >  0.0f)      // Yaw is to the left of the bound
                && (x_MinAngleDiff( LeftYawBound, OldYaw ) >= 0.0f) )    // OldYaw is to the right of the bound
            {
                // We've just crossed the boundary
                m_Yaw = LeftYawBound;
            }
        }

        // Right
        if (   (MinAngleDiff < 0.0f) // rotating right
            && (pObj = g_ObjMgr.GetObjectByGuid( m_Turret.RightBoundaryGuid )) )
        {
            const vector3 Pos( pObj->GetPosition() );
            const vector3 ToPos( Pos - EyePos );
            const f32 RightYawBound = ToPos.GetYaw();

            if ( (x_MinAngleDiff( m_Yaw, RightYawBound )   <  0.0f)      // Yaw is to the right of the bound
                && (x_MinAngleDiff( RightYawBound, OldYaw )<= 0.0f) )    // OldYaw is to the left of the bound
            {
                // We've just crossed the boundary
                m_Yaw = RightYawBound;
            }
        }

        MinAngleDiff = x_MinAngleDiff( m_Pitch, OldPitch );
        // Upper
        if (   (MinAngleDiff < 0.0f) // rotating up
            && (pObj = g_ObjMgr.GetObjectByGuid( m_Turret.UpperBoundaryGuid )) )
        {
            const vector3 Pos( pObj->GetPosition() );
            const vector3 ToPos( Pos - EyePos );
            const f32 UpperPitchBound = ToPos.GetPitch();

            if (   (x_MinAngleDiff( m_Pitch, UpperPitchBound )  <  0.0f)      // Pitch is above the bound
                && (x_MinAngleDiff( UpperPitchBound, OldPitch ) <= 0.0f) )    // OldPitch is below the bound
            {
                // We've just crossed the boundary
                m_Pitch = UpperPitchBound;
            }
        }

        // Lower
        if (   (MinAngleDiff > 0.0f) // rotating down
            && (pObj = g_ObjMgr.GetObjectByGuid( m_Turret.LowerBoundaryGuid )) )
        {
            const vector3 Pos( pObj->GetPosition() );
            const vector3 ToPos( Pos - EyePos );
            const f32 LowerPitchBound = ToPos.GetPitch();

            if (   (x_MinAngleDiff( m_Pitch, LowerPitchBound )  >  0.0f)      // Pitch is below the bound
                && (x_MinAngleDiff( LowerPitchBound, OldPitch ) >= 0.0f) )    // OldPitch is above the bound
            {
                // We've just crossed the boundary
                m_Pitch = LowerPitchBound;
            }
        }
    }

#ifndef X_EDITOR
    if( g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::LOOK_VERTICAL   ).IsValue )
    {
        // do stuff?
    }
    else
    {
        player_profile& p = g_StateMgr.GetActiveProfile(g_StateMgr.GetProfileListIndex(m_LocalSlot));

        // if lookspring is on, recenter vertically over time
        if( p.m_bLookspringOn )
        {
            f32 SpringSpeed = LookSpring_ReturnSpeedTweak.GetF32();
            m_Pitch -= (m_Pitch * SpringSpeed * rDeltaTime);
            m_Pitch  = MIN( m_PitchMax, MAX( m_PitchMin, m_Pitch ) );
        }
    }
#endif

    UpdateCameraShake( rDeltaTime );

    if( (m_Pitch != OldPitch) || 
        (m_Yaw   != OldYaw  ) )
    {
#ifndef X_EDITOR
        m_NetDirtyBits |= ORIENTATION_BIT;
#endif
    }
}

//===========================================================================
void player::CalculateRigOffset( f32 DeltaTime )
{
    ( void )DeltaTime;

    CalculateStrafeRigOffset( DeltaTime );
    CalculateMoveRigOffset( DeltaTime );

    // Figure out where to put the rig in relation to the camera.
    vector3 vDesiredOffsetStrafe( m_fCurrentStrafeRigOffset, 0.0f, 0.0f );
    vector3 vDesiredOffsetMove( 0.0f, 0.0f, m_fCurrentMoveRigOffset );

    vDesiredOffsetStrafe.RotateY( m_EyesYaw );
    vDesiredOffsetMove.RotateY( m_EyesYaw );

    m_vRigOffset = vDesiredOffsetStrafe + vDesiredOffsetMove;
}   

//===========================================================================

void player::CalculateStrafeRigOffset( f32 DeltaTime )
{
    xbool bMovingLeft = FALSE;
    xbool bMovingRight = FALSE;
    f32 PreviousStrafeOffset = m_fCurrentStrafeRigOffset;

    // The desired offsets for where the controller is currently placed.
    f32 DesiredStrafeOffset = m_fRigMaxStrafeOffset * m_fStrafeValue;

    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon && pWeapon->IsZoomEnabled() )
        DesiredStrafeOffset = 0.0f;

    // Update the offsets.
    if ( DesiredStrafeOffset < m_fCurrentStrafeRigOffset )
    {
        m_fCurrentStrafeRigOffset -= ( m_fRigStrafeOffsetVelocity * DeltaTime );
        bMovingLeft = TRUE;
    }
    else
        if ( DesiredStrafeOffset > m_fCurrentStrafeRigOffset )
        {
            m_fCurrentStrafeRigOffset += ( m_fRigStrafeOffsetVelocity * DeltaTime );
            bMovingRight = TRUE;
        }


        if ( bMovingLeft )
        {
            m_fCurrentStrafeRigOffset = MAX( m_fCurrentStrafeRigOffset, DesiredStrafeOffset );
        }
        if ( bMovingRight )
        {
            m_fCurrentStrafeRigOffset = MIN( m_fCurrentStrafeRigOffset, DesiredStrafeOffset );
        }

        if ( DesiredStrafeOffset == 0.0f )
        {
            if ( PreviousStrafeOffset <= 0.0f && m_fCurrentStrafeRigOffset >= 0.0f )
            {
                m_fCurrentStrafeRigOffset = 0.0f;
            }
            if ( PreviousStrafeOffset >= 0.0f && m_fCurrentStrafeRigOffset <= 0.0f )
            {
                m_fCurrentStrafeRigOffset = 0.0f;
            }

        }

}

//===========================================================================

void player::CalculateMoveRigOffset( f32 DeltaTime )
{
    xbool bMovingForward = FALSE;
    xbool bMovingBackward = FALSE;
    f32 PreviousMoveOffset = m_fCurrentMoveRigOffset;

    // The desired offsets for where the controller is currently placed.
    f32 DesiredMoveOffset = m_fRigMaxMoveOffset * m_fMoveValue;

    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon && pWeapon->IsZoomEnabled() )
        DesiredMoveOffset = 0.0f;

    // Update the offsets.
    if ( DesiredMoveOffset < m_fCurrentMoveRigOffset )
    {
        m_fCurrentMoveRigOffset -= ( m_fRigMoveOffsetVelocity * DeltaTime );
        bMovingForward = TRUE;
    }
    else
        if ( DesiredMoveOffset > m_fCurrentMoveRigOffset )
        {
            m_fCurrentMoveRigOffset += ( m_fRigMoveOffsetVelocity * DeltaTime );
            bMovingBackward = TRUE;
        }

        if ( bMovingForward )
        {
            m_fCurrentMoveRigOffset = MAX( m_fCurrentMoveRigOffset, DesiredMoveOffset );
        }
        else
            if ( bMovingBackward )
            {
                m_fCurrentMoveRigOffset = MIN( m_fCurrentMoveRigOffset, DesiredMoveOffset );
            }

            if ( DesiredMoveOffset == 0.0f )
            {
                if ( PreviousMoveOffset <= 0.0f && m_fCurrentMoveRigOffset >= 0.0f )
                {
                    m_fCurrentMoveRigOffset = 0.0f;
                }
            }
}

//===========================================================================
// YAW
void player::CalculateLookHorozOffset( f32 DeltaTime )
{
    xbool bLookingLeft = FALSE;
    xbool bLookingRight = FALSE;
    f32 PreviousLookOffset = m_CurrentHorozRigOffset;

    // The desired offsets for where the controller is currently placed.
    f32 DesiredHorozOffset = m_RigLookMaxHorozOffset * m_fYawValue;

    // Update the offsets.
    if ( DesiredHorozOffset < m_CurrentHorozRigOffset )
    {
        m_CurrentHorozRigOffset -= ( m_RigLookHorozVelocity * DeltaTime );
        bLookingLeft = TRUE;
    }
    else if ( DesiredHorozOffset > m_CurrentHorozRigOffset )
    {
        m_CurrentHorozRigOffset += ( m_RigLookHorozVelocity * DeltaTime );
        bLookingRight = TRUE;
    }

    if ( bLookingLeft )
    {
        m_CurrentHorozRigOffset = MAX( m_CurrentHorozRigOffset, DesiredHorozOffset );
    }
    else if ( bLookingRight )
    {
        m_CurrentHorozRigOffset = MIN( m_CurrentHorozRigOffset, DesiredHorozOffset );
    }

    if ( DesiredHorozOffset == 0.0f )
    {
        if ( PreviousLookOffset <= 0.0f && m_CurrentHorozRigOffset >= 0.0f )
        {
            m_CurrentHorozRigOffset = 0.0f;
        }
    }
}

//===========================================================================
// PITCH
void player::CalculateLookVertOffset( f32 DeltaTime )
{
    xbool bLookingUp = FALSE;
    xbool bLookingDown = FALSE;
    f32 PreviousLookOffset = m_CurrentVertRigOffset;

    // The desired offsets for where the controller is currently placed.
    f32 DesiredVertOffset = m_RigLookMaxVertOffset * m_fPitchValue;

    // Update the offsets.
    if ( DesiredVertOffset < m_CurrentVertRigOffset )
    {
        m_CurrentVertRigOffset -= ( m_RigLookVertVelocity * DeltaTime );
        bLookingUp = TRUE;
    }
    else
        if ( DesiredVertOffset > m_CurrentVertRigOffset )
        {
            m_CurrentVertRigOffset += ( m_RigLookVertVelocity * DeltaTime );
            bLookingDown = TRUE;
        }

        if ( bLookingUp )
        {
            m_CurrentVertRigOffset = MAX( m_CurrentVertRigOffset, DesiredVertOffset );
        }
        else
            if ( bLookingDown )
            {
                m_CurrentVertRigOffset = MIN( m_CurrentVertRigOffset, DesiredVertOffset );
            }

            if ( DesiredVertOffset == 0.0f )
            {
                if ( PreviousLookOffset <= 0.0f && m_CurrentVertRigOffset >= 0.0f )
                {
                    m_CurrentVertRigOffset = 0.0f;
                }
            }
}

//===========================================================================

void player::CalculateRotationAccelerationFactors( f32 DeltaTime )
{
    f32 AccelerationTime = m_YawAccelTime;

    // Update the yaw acceleration factor
    if ( x_abs( m_fYawValue ) >= F32_MIN )
    {
        m_YawAccelFactor += ( DeltaTime / AccelerationTime );
        m_YawAccelFactor = MIN( 1.0f, m_YawAccelFactor );
    }
    else
    {
        m_YawAccelFactor = 0.0f;
    }
    // Update the pitch acceleration factor
    if ( x_abs( m_fPitchValue ) >= F32_MIN )
    {
        m_PitchAccelFactor += ( DeltaTime / m_PitchAccelTime );
        m_PitchAccelFactor = MIN( 1.0f, m_PitchAccelFactor );
    }
    else
    {
        m_PitchAccelFactor = 0.0f;
    }

}

//===========================================================================
// The stick position determines the maximum turn rate
// The stick position also determines the turn acceleration

void player::UpdateRotationRates( f32 DeltaTime )
{
    (void) DeltaTime;
}

//===========================================================================


void player::CalculatePitchLimits( const f32& rDeltaTime )
{
    m_PitchMin += 2.f;
    m_PitchMax += 2.f;
    m_DesiredPitchMin += 2.f;
    m_DesiredPitchMax += 2.f;

    //Handle changes to m_DesiredPitchMin
    if ( m_PitchMin < m_DesiredPitchMin )
    {
        m_PitchMin = MIN( m_PitchMin + m_fPitchChangeSpeed * rDeltaTime , m_DesiredPitchMin ); 
    }
    else
        if ( m_PitchMin > m_DesiredPitchMin )
        {
            m_PitchMin = MAX( m_PitchMin - m_fPitchChangeSpeed * rDeltaTime , m_DesiredPitchMin );         
        }

        //Handle changes to m_DesiredPitchMax
        if ( m_PitchMax < m_DesiredPitchMax )
        {
            m_PitchMax = MIN( m_PitchMax + m_fPitchChangeSpeed * rDeltaTime , m_DesiredPitchMax ); 
        }
        else
            if ( m_PitchMax > m_DesiredPitchMax )
            {
                m_PitchMax = MAX( m_PitchMax - m_fPitchChangeSpeed * rDeltaTime , m_DesiredPitchMax );         
            }

            m_PitchMin -= 2.f;
            m_PitchMax -= 2.f;

            m_DesiredPitchMin -= 2.f;
            m_DesiredPitchMax -= 2.f;
}

//=========================================================================

// SB:
// Scales velocity and speed with respect to plane normal:
// NOTE: Velocity and speed are handled separately because Velocity has DeltaTime
// baked into it and speed does not - don't ask me why...
static
void ScaleVelocityComponent( const vector3& PlaneNormal, 
                             f32            PerpScale, 
                             f32            ParaScale, 
                             vector3&       Velocity, 
                             f32&           Speed )
{
    // Compute current speed squared, and exit if not moving
    f32 PrevSpeedSqr = Velocity.LengthSquared();
    if( PrevSpeedSqr < 0.0001f )
        return;

    // Compute components into and along collision plane
    vector3 PerpVel = PlaneNormal * v3_Dot( PlaneNormal, Velocity );
    vector3 ParaVel = Velocity - PerpVel;

    // Compute new velocity, taking scaling into account
    Velocity = ( PerpScale * PerpVel ) + ( ParaScale * ParaVel);

    // Scale speed in proportion with velocity change
    f32 CurrSpeedSqr = Velocity.LengthSquared();
    if( CurrSpeedSqr >= 0.0001f )
    {
        f32 PrevSpeed = x_sqrt( PrevSpeedSqr );
        f32 CurrSpeed = x_sqrt( CurrSpeedSqr );
        Speed *= CurrSpeed / PrevSpeed;
    }    
    else
    {
        Speed = 0.0f;    
    }        
}

//=========================================================================

void player::ScaleVelocity( const vector3& PlaneNormal, f32 PerpScale, f32 ParaScale )
{
    // Apply to forward and side components
    ScaleVelocityComponent( PlaneNormal, PerpScale, ParaScale, m_ForwardVelocity, m_fForwardSpeed );
    ScaleVelocityComponent( PlaneNormal, PerpScale, ParaScale, m_StrafeVelocity,  m_fStrafeSpeed );
}

//==============================================================================

void player::CalculateForwardVelocity( const vector3& rViewZ , const f32& rDeltaTime )
{
    CONTEXT("player::CalculateForwardVelocity");
    //maximum velocity for current controller input
    f32 fCurMaxForwardSpeed = m_MaxFowardVelocity * m_fMoveValue;
    m_fPrevForwardSpeed = m_fForwardSpeed;

    //trying not to move or just switched directions
    if ( fCurMaxForwardSpeed == 0.0f || ReversingMoveDirection( fCurMaxForwardSpeed ) )
    {
        if ( m_fForwardSpeed > 0.0f )
        {
            m_fForwardSpeed = MAX( 0.0f , -m_fForwardAccel * m_fDecelerationFactor * rDeltaTime + m_fForwardSpeed );
        }

        else
            if ( m_fForwardSpeed < 0.0f )
            {
                m_fForwardSpeed = MIN( 0.0f , m_fDecelerationFactor * m_fForwardAccel * rDeltaTime + m_fForwardSpeed );
            }
    }

    else
        if ( fCurMaxForwardSpeed > 0.0f )
        {
            if ( fCurMaxForwardSpeed >= m_fForwardSpeed )
            {
                m_fForwardSpeed = MIN( fCurMaxForwardSpeed , m_fForwardAccel * rDeltaTime + m_fForwardSpeed );
            }
            else
            {
                m_fForwardSpeed = MAX( fCurMaxForwardSpeed , -m_fForwardAccel * m_fDecelerationFactor * rDeltaTime + m_fForwardSpeed );
            }

        }

        else
        {
            if ( fCurMaxForwardSpeed <= m_fForwardSpeed )
            {
                m_fForwardSpeed = MAX( fCurMaxForwardSpeed , -m_fForwardAccel * rDeltaTime + m_fForwardSpeed );
            }
            else
            {
                m_fForwardSpeed = MIN( fCurMaxForwardSpeed , m_fForwardAccel * m_fDecelerationFactor * rDeltaTime + m_fForwardSpeed );
            }
        }

        m_ForwardVelocity = rViewZ * (rDeltaTime * m_fForwardSpeed);

        if( g_MPTweaks.Active )
        {
            if( IsMutated() )
            {
                m_ForwardVelocity *= MP_RunSpeedFactor_MutantTweak.GetF32();
                m_ForwardVelocity *= MutantSpeedFactor;
            }
            else
            {
                m_ForwardVelocity *= MP_RunSpeedFactor_NormalTweak.GetF32();
                m_ForwardVelocity *= HumanSpeedFactor;
            }
        }

        m_ForwardVelocity *= g_PerceptionMgr.GetForwardSpeedFactor();
}

//==============================================================================

xbool player::ReversingMoveDirection( const f32& fMaxForward )
{
    return ( ( m_fForwardSpeed > 0.0f && fMaxForward < 0.0f ) || ( m_fForwardSpeed < 0.0f && fMaxForward > 0.0f ) );
}

//==============================================================================

xbool   player::HasSpeedReversed( void )
{
    return ( ( m_fPrevForwardSpeed >= 0.0f && m_fForwardSpeed < 0.0f ) || ( m_fPrevForwardSpeed <= 0.0f && m_fForwardSpeed > 0.0f ) );
}

//==============================================================================

void player::CalculateStrafeVelocity( const vector3& rViewX , const f32& rDeltaTime )
{
    //maximum velocity for current controller input
    f32 fCurMaxStrafeSpeed = m_MaxStrafeVelocity * m_fStrafeValue;


    //trying not to move or just switched directions
    if ( fCurMaxStrafeSpeed == 0.0f || ReversingStrafeDirection( fCurMaxStrafeSpeed ) )
    {
        if ( m_fStrafeSpeed > 0.0f )
        {
            m_fStrafeSpeed = MAX( 0.0f , -m_fStrafeAccel * m_fDecelerationFactor * rDeltaTime + m_fStrafeSpeed );
        }

        else
            if ( m_fStrafeSpeed < 0.0f )
            {
                m_fStrafeSpeed = MIN( 0.0f , m_fDecelerationFactor * m_fStrafeAccel * rDeltaTime + m_fStrafeSpeed );
            }
    }

    //trying to move right
    else
        if ( fCurMaxStrafeSpeed < 0.0f )
        {
            //speeding up - growing negative
            if ( fCurMaxStrafeSpeed <= m_fStrafeSpeed )
            {
                m_fStrafeSpeed = MAX( fCurMaxStrafeSpeed , -m_fStrafeAccel * rDeltaTime + m_fStrafeSpeed );
            }
            //slowing down - growing positive
            else
            {
                m_fStrafeSpeed = MIN( fCurMaxStrafeSpeed , m_fDecelerationFactor * m_fStrafeAccel * rDeltaTime + m_fStrafeSpeed );
            }
        }

        //trying to move left
        else
        {
            //speeding up - growing positive
            if ( fCurMaxStrafeSpeed >= m_fStrafeSpeed )
            {
                m_fStrafeSpeed = MIN( fCurMaxStrafeSpeed , m_fStrafeAccel * rDeltaTime + m_fStrafeSpeed );
            }
            //slowing down - growing negative : 
            else
            {
                m_fStrafeSpeed = MAX( fCurMaxStrafeSpeed , -m_fStrafeAccel * m_fDecelerationFactor * rDeltaTime + m_fStrafeSpeed );
            }
        }

        m_StrafeVelocity = rViewX * (rDeltaTime * m_fStrafeSpeed);

        if( g_MPTweaks.Active )
        {
            if( IsMutated() )
            {
                m_StrafeVelocity *= MP_StrafeSpeedFactor_MutantTweak.GetF32();
                m_StrafeVelocity *= MutantSpeedFactor;
            }
            else
            {
                m_StrafeVelocity *= MP_StrafeSpeedFactor_NormalTweak.GetF32();
                m_StrafeVelocity *= HumanSpeedFactor;
            }
        }
}

//==============================================================================

xbool player::ReversingStrafeDirection( const f32& fMaxStrafe )
{
    return ( ( m_fStrafeSpeed > 0.0f && fMaxStrafe < 0.0f ) || ( m_fStrafeSpeed < 0.0f && fMaxStrafe > 0.0f ) );
}

//==============================================================================

f32 player::GetSpeed( void )
{
    return m_fStrafeSpeed + m_fForwardSpeed;
}

//==============================================================================

f32 player::GetCurrentVelocity( void )
{
    //return highest of these two
    return MAX(x_abs(m_fStrafeSpeed), x_abs(m_fForwardSpeed));
}

//==============================================================================

f32 player::GetMaxVelocity( void )
{ 
    //return highest of these two
    return MAX(m_MaxFowardVelocity, m_MaxStrafeVelocity); 
}

//===========================================================================

//===========================================================================
// Returns guid of ladder if player is intersecting a ladder
guid player::IsInLadderField( void )
{
    // Lookup character physics
    const character_physics& Physics = m_Physics ;

    // Search for being inside a ladder
    g_ObjMgr.SelectBBox( object::ATTR_COLLIDABLE, GetBBox(), object::TYPE_LADDER_FIELD) ;
    slot_id SlotID = g_ObjMgr.StartLoop();
    while(SlotID != SLOT_NULL)
    {
        // Lookup object and quit loop
        ladder_field* pLadder = (ladder_field*)g_ObjMgr.GetObjectBySlot(SlotID) ;
        ASSERT(pLadder) ;

        // Overlapping this ladder?
        if (  (pLadder->GetGuid() != m_JumpedOffLadderGuid)         // did we just jump off this ladder?
            && pLadder->DoesCylinderIntersect(Physics.GetPosition(),// are we in the field? 
            Physics.GetColHeight(), 
            Physics.GetColRadius()))
        {
            g_ObjMgr.EndLoop();
            return pLadder->GetGuid() ;
        }

        // Check next object
        SlotID = g_ObjMgr.GetNextResult(SlotID) ;
    }
    g_ObjMgr.EndLoop();

    // No ladder found
    return 0 ;
}

//===========================================================================

// Ladder tweakables
static f32 LADDER_CLIMB_SPEED       = 300.0f ;  // Vertical speed
static f32 LADDER_STRAFE_SPEED      = 100.0f ;  // Horizontal speed
static f32 LADDER_FLIP_UP_ANGLE     = 25.0f ;   // Angle at which to swap up/down when facing ladder
static f32 LADDER_AT_TOP_OFFSET     = 10.0f ;   // Dismount distance from top
static f32 LADDER_AT_BOT_OFFSET     = 10.0f ;   // Dismount distance from bottom
static f32 LADDER_JUMP_OFF_VEL      = 150.0f ;  // Push away vel when jumping
//static f32 LADDER_MOVING_AWAY_VEL   = 300.0f ;   // Vel threshold for detecting moving away from ladder

xbool player::UpdateLadderMovement( f32 DeltaTime )
{
    // Lookup physics to use
    character_physics& Physics = m_Physics ;

    // Default to not being on a ladder
    m_bOnLadder = FALSE ;
    m_LadderOutDir.Zero() ;
    Physics.SetUseGravity(TRUE) ;

    // On a ladder?
    guid LadderGuid = IsInLadderField() ;
    if (!LadderGuid)
        return FALSE ;

    // Get ladder object
    const ladder_field* pLadder = (ladder_field*)g_ObjMgr.GetObjectByGuid(LadderGuid) ;
    ASSERT(pLadder) ;

    // Lookup ladder object info
    const matrix4& LadderL2W = pLadder->GetL2W() ;

    // Setup local direction vectors
    vector3 Out (0,0,1) ;
    vector3 Up  (0,1,0) ;
    vector3 Side(1,0,0) ;

    // Compute ladder world direction vectors
    vector3 LadderOut  = LadderL2W.RotateVector(Out) ;
    vector3 LadderUp   = LadderL2W.RotateVector(Up) ;
    vector3 LadderSide = LadderL2W.RotateVector(Side) ;

    // Compute ladder climb plane
    plane LadderOutPlane ;
    LadderOutPlane.Setup(pLadder->GetPosition(), LadderOut) ;
    f32 Dist = LadderOutPlane.Distance(GetPosition()) ;

    // Behind the ladder? (eg. when entering the ladder from the top of a ledge)
    if (Dist < 0)
        return FALSE ;

    // Compute player facing forward direction
    vector3 PlayerForward(Out) ;
    PlayerForward.RotateX(m_Pitch) ;
    PlayerForward.RotateY(m_Yaw) ;

    // Compute player running direction after getting off ladder
    vector3 PlayerRun(Out * m_fMoveValue) ;
    PlayerRun.RotateY(m_Yaw) ;

    // Flip up direction if player is looking down the ladder
    xbool bLookingDownLadder = (LadderUp.Dot(PlayerForward) < x_cos(DEG_TO_RAD(90.0f + LADDER_FLIP_UP_ANGLE))) ;
    if (bLookingDownLadder)
        LadderUp = -LadderUp ;

    // Flip side direction if player is looking out from ladder
    if (LadderOut.Dot(PlayerForward) < 0)
        LadderSide  = -LadderSide ;

    // Compute up/down/side movement from input
    vector3 UpDownVel = LadderUp   * m_fMoveValue   * DeltaTime * LADDER_CLIMB_SPEED ;
    vector3 SideVel   = LadderSide * m_fStrafeValue * DeltaTime * LADDER_STRAFE_SPEED  ;

    // Get ladder and feet info
    f32 Top    = pLadder->GetTop() ;
    f32 Bottom = pLadder->GetBottom() ;
    f32 Feet   = GetPosition().GetY() ;

    // If moving down and past bottom of ladder, push the player off ready for dismount
    if ((UpDownVel.GetY() < 0) && (Feet < (Bottom + LADDER_AT_BOT_OFFSET)))
        SideVel += PlayerRun * DeltaTime * LADDER_CLIMB_SPEED ;

    // Running into the ladder?
    if (LadderOut.Dot(PlayerRun) < 0)
    {
        // If near top and moving up, push the player off ready dismount
        if ( (UpDownVel.GetY() > 0) && (Feet > (Top - LADDER_AT_TOP_OFFSET)) )
            SideVel += PlayerRun * DeltaTime * LADDER_CLIMB_SPEED ;
    }
    else
    {
        // If pulling away from the ladder, pulling back on the stick, and looking down, then let go!
        if ((PlayerRun.Dot(LadderOut) > 0) && (m_fMoveValue < 0) && (bLookingDownLadder))
            return FALSE ;

        // If the player is NOT facing the ladder and moving up, then stop from going off the top
        if ( (UpDownVel.GetY() > 0) && (Feet > (Top - LADDER_AT_TOP_OFFSET)) )
            UpDownVel.Zero() ;
    }

    // Compute final vel
    vector3 FinalVel = UpDownVel + SideVel ;

    // Clear velocity so player doesn't shoot up/down if no input
    Physics.ZeroVelocity() ;

    // Turn off gravity
    Physics.SetUseGravity(FALSE) ;

    // Update
    Physics.Advance( Physics.GetPosition() + FinalVel, DeltaTime );
    OnMove( Physics.GetPosition() );

    // Record player is on a ladder
    m_bOnLadder         = TRUE ;
    m_Physics.SetGroundTracking( FALSE );
    m_LastLadderGuid    = LadderGuid;
    m_LadderOutDir      = LadderOut ;

    return TRUE ;
}

//===========================================================================


void player::Jump( void )
{
#if defined(aharp) && !defined(X_EDITOR)
    /*
    static xbool bTurnOnCinema = FALSE;
    MsgMgr.Message( MSG_STRING, 0, (s32)((const xwchar*)xwstring("ABC\nDEFGH\nIJKLMN")) );

    slot_id SlotID  = g_ObjMgr.GetFirst( object::TYPE_HUD_OBJECT );

    if( SlotID != SLOT_NULL )
    {
    object* pObj    = g_ObjMgr.GetObjectBySlot( SlotID );
    hud_object& Hud = hud_object::GetSafeType( *pObj );

    Hud.SetupLetterBox( bTurnOnCinema, 0.8f );
    bTurnOnCinema = !bTurnOnCinema;
    }
    */
#endif

    // Lookup physics
    character_physics& Physics = m_Physics ;

    // Make sure we're not able to jump as high when ascending a hill
    f32 VerticalVelocity = Physics.GetVelocity().GetY();
    f32 JumpVelocity     = m_JumpVelocity;

    if( g_MPTweaks.Active )
    {
        VerticalVelocity = 0.0f;
        JumpVelocity    *= g_MPTweaks.JumpSpeed;
    }

    if ( VerticalVelocity > 0.0f )
    {
        JumpVelocity -= VerticalVelocity;
        JumpVelocity  = MAX( 0.0f, JumpVelocity );
    }

    // Jump off a ladder?
    if( m_bOnLadder )
    {
        m_bOnLadder = FALSE ;
        m_JumpedOffLadderGuid = m_LastLadderGuid;
        vector3 Vel = Physics.GetVelocity();
        Vel += m_LadderOutDir * LADDER_JUMP_OFF_VEL;
        Physics.SetVelocity(Vel);
    }
    else
    {
        // Jump vertically
        Physics.Jump( JumpVelocity );
    }             
}

//===========================================================================

void player::HitJumpPad( const vector3& Velocity, 
                               f32      DeltaTime, 
                               f32      AirControl, 
                               xbool    BoostOnly,
                               xbool    ReboostOnly,
                               xbool    Instantaneous,
                               guid     JumpPadGuid )
{
    m_Physics.Fling( Velocity, DeltaTime, AirControl, 
                     BoostOnly, ReboostOnly, 
                     Instantaneous, JumpPadGuid );

    if( Instantaneous && !ReboostOnly )
    {
        #ifndef X_EDITOR
        if( !(m_WayPointFlags & WAYPOINT_TELEPORT_FX) )
        {
            slot_id Slot = g_ObjMgr.GetFirst( TYPE_JUMP_PAD );
            while( Slot != SLOT_NULL )
            {
                object* pObject = g_ObjMgr.GetObjectBySlot( Slot );
                ASSERT( pObject );
                ASSERT( pObject->GetType() == TYPE_JUMP_PAD );
                vector3 Gap = pObject->GetPosition() - GetPosition();
                if( Gap.LengthSquared() < 250.0f )
                {
                    jump_pad* pJumpPad = (jump_pad*)pObject;
                    pJumpPad->PlayJump();
                    break;
                }
                Slot = g_ObjMgr.GetNext( Slot );
            }

            m_NetDirtyBits    |= WAYPOINT_BIT;
            m_WayPointFlags   |= WAYPOINT_JUMP_PAD_FX;
            m_WayPointTimeOut  = 0;
        }
        #endif
    }
}

//===========================================================================

void player::UpdateFellFromAltitude( void )
{
    // Use the current position if we have moved upwards or are not falling
    f32 Altitude = GetPosition().GetY();
    if( (Altitude > m_FellFromAltitude) || !m_bFalling )
    {
        m_FellFromAltitude = Altitude;
    }
}

//==============================================================================

