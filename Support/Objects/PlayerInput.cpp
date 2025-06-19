//==============================================================================
//
//  PlayerInput.cpp
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
#include "Characters\ActorEffects.hpp"
#include "Configuration/GameConfig.hpp"
#include "objects\turret.hpp"
#include "objects\WeaponShotgun.hpp"
#include "Gamelib/DebugCheats.hpp"
#include "objects\FocusObject.hpp"
#include "PerceptionMgr\PerceptionMgr.hpp"
#include "Objects\LoreObject.hpp"
#include "Objects\Camera.hpp"
#include "InputMgr\Monkey.hpp"

#ifdef X_EDITOR
#include "../Apps/Editor/Project.hpp"
#else
#include "NetworkMgr\MsgMgr.hpp"
#include "Menu\DebugMenu2.hpp"
#endif

f32 g_CrouchUpVelocity    = 80.0f;

static f32 MIN_TIME_BETWEEN_MUTATION_CHANGES = 0.5f;


void player::UpdateUserInput(  f32 DeltaTime )
{
    if (!m_bActivePlayer)
        return;

#ifndef X_EDITOR
    if ( g_StateMgr.IsPaused() )
    {
#if defined(TARGET_XBOX) && !defined(X_EDITOR)
        // Still allow voice chat when in menus for xbox only.
        if( g_NetworkMgr.IsOnline() )
        {
            if( g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_TALK_MODE_TOGGLE ).WasValue )
            {
                g_VoiceMgr.ToggleTalkMode();
            }

            // Voice chat
            {
                g_VoiceMgr.SetTalking( FALSE );

                if( g_VoiceMgr.IsHeadsetPresent() == TRUE )
                {
                    // check for voice chat
                    if( g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_CHAT ).IsValue )
                    {
                        // Test to make sure the player is allowed to chat
                        if( (g_VoiceMgr.IsVoiceBanned()  == FALSE) &&
                            (g_VoiceMgr.IsVoiceEnabled() ==  TRUE) )
                            g_VoiceMgr.SetTalking( TRUE );
                        return;
                    }
                }
            }
        }
#endif
        ClearStickInput(); 
        g_IngamePad[m_ActivePlayerPad].ClearAllLogical();
        return;
    }
#endif

    if ( m_ViewCinematicPlaying )
    {
        //Play the view cinematic
        UpdateViewCinematic( DeltaTime );
    }
    else if ( !m_Cinema.m_bCinemaOn 
        && (!m_bDead || !m_bCanDie) )
    {
        if( m_ActivePlayerPad != -1 )
        {
            //handles button press events
            OnButtonInput();
            UpdateStickInput();            
        }
    }
    else if ( m_Cinema.m_bCinemaOn )
    {
        ClearStickInput();    
        m_fMoveValue            = 0;
        m_fStrafeValue          = 0;
        m_fRawControllerYaw     = 0;
        m_fRawControllerPitch   = 0;
        m_fPreviousYawValue     = 0;
        m_fPreviousPitchValue   = 0;
        g_IngamePad[m_ActivePlayerPad].ClearAllLogical();
    }
}
//==============================================================================

void player::ClearStickInput( void )
{
    m_fMoveValue            = 0.0f;
    m_fStrafeValue          = 0.0f;
    m_fRawControllerPitch   = 0.0f;
    m_fRawControllerYaw     = 0.0f;
    m_fPitchValue           = 0.0f;
    m_fYawValue             = 0.0f;
    m_fPreviousPitchValue   = 0.0f;
    m_fPreviousYawValue     = 0.0f;
}

//===========================================================================

struct controller_scale_tweak
{
    vector2 Point0;
    vector2 Point1;
    vector2 Direction0;
    vector2 Direction1;
} ;

controller_scale_tweak g_ControllerScaleTweak 
    = { vector2( 0.0f, 0.0f ),      // Point0
        vector2( 1.0f, 1.0f ),      // Point1
        vector2( 1.0f, 0.1f ),      // Direction0
        vector2( 0.0f, 3.2f ) };    // Direction1

void player::ScaleYawAndPitchValues( void )
{
    //
    // We need to scale the raw controller values on a spline curve
    // The following code computes the multiplication factors for the
    // raw controller values (pitch and yaw), so we can multiply and get the new values.
    // The shape of the curve is tuned with Direction0.Y and Direction1.Y 
    // in g_ControllerScaleTweak
    //
    f32 Sign = (m_fRawControllerYaw < 0.0f) ? -1.0f : 1.0f;
    f32 s = x_abs( m_fRawControllerYaw );
    f32 s2 = s * s;
    f32 s3 = s * s * s;
    f32 h1 = (2.0f * s3) - (3 * s2) + 1;
    f32 h2 = (-2.0f * s3) + (3 * s2);
    f32 h3 = s3 - (2.0f * s2) + s;
    f32 h4 = s3 - s2;

    m_fYawValue = (h1 * g_ControllerScaleTweak.Point0.Y)
                + (h2 * g_ControllerScaleTweak.Point1.Y)
                + (h3 * g_ControllerScaleTweak.Direction0.Y)
                + (h4 * g_ControllerScaleTweak.Direction1.Y);
    m_fYawValue *= Sign;

    Sign = (m_fRawControllerPitch < 0.0f) ? -1.0f : 1.0f;
    s = x_abs( m_fRawControllerPitch );
    s2 = s * s;
    s3 = s * s * s;
    h1 = (2.0f * s3) - (3 * s2) + 1;
    h2 = (-2.0f * s3) + (3 * s2);
    h3 = s3 - (2.0f * s2) + s;
    h4 = s3 - s2;

    m_fPitchValue 
        = (h1 * g_ControllerScaleTweak.Point0.Y)
        + (h2 * g_ControllerScaleTweak.Point1.Y)
        + (h3 * g_ControllerScaleTweak.Direction0.Y)
        + (h4 * g_ControllerScaleTweak.Direction1.Y);
    m_fPitchValue *= Sign;

#ifndef X_EDITOR
    // do input sensitivity
    {
        player_profile& p = g_StateMgr.GetActiveProfile(g_StateMgr.GetProfileListIndex(m_LocalSlot));

        u32 sensitivity_H = p.GetSensitivity(SM_X_SENSITIVITY);
        u32 sensitivity_V = p.GetSensitivity(SM_Y_SENSITIVITY);

        // Sensitivity setting range
        // |___________I__________|
        // 0           50        100
        // 
        // 0   = -50%
        // 50  = normal
        // 100 = +50%
        //

        f32 Scalar_H = (f32)((sensitivity_H - 50.0f)/100.0f);
        f32 Scalar_V = (f32)((sensitivity_V - 50.0f)/100.0f);

        m_fYawValue     = m_fYawValue + (m_fYawValue * Scalar_H); 
        m_fPitchValue   = m_fPitchValue + (m_fPitchValue * Scalar_V);
    }
#endif
    //DrawLabelInFront( xfs( "YawValue: %f\nRawYaw%f\nPitchValue: %f\nRawPitch: %f\nSlopeYaw: %f\nSlopePitch: %f\n", m_fYawValue, m_fRawControllerYaw, m_fPitchValue, m_fRawControllerPitch, fCurrentSlopeYaw, fCurrentSlopePitch ) );
}

//==============================================================================

void player::UpdateStickInput(void)
{
    ASSERT( m_ActivePlayerPad != -1 );

    m_fPreviousYawValue = m_fYawValue;
    m_fPreviousPitchValue = m_fPitchValue;

#if defined(TARGET_PC) //GS: Experimental PC mouse controls.
    const f32 BaseMouseSensitivity = 64.0f;
    f32 Rot = R_10 * m_DeltaTime;
    
    m_fRawControllerYaw = -g_IngamePad[m_ActivePlayerPad].GetLogical(ingame_pad::LOOK_HORIZONTAL).IsValue * Rot;
    m_fRawControllerPitch = g_IngamePad[m_ActivePlayerPad].GetLogical(ingame_pad::LOOK_VERTICAL).IsValue * Rot;
    
    #if !defined(X_EDITOR)    
    player_profile& p = g_StateMgr.GetActiveProfile(g_StateMgr.GetProfileListIndex(m_LocalSlot));
	
    //MAB: removed invert Y global var - only check profile now
    if( p.m_bInvertY )
    {
        m_fRawControllerPitch = -m_fRawControllerPitch;
    }
    #else
    // invert Y
    extern xbool g_EditorInvertY;
    if( g_EditorInvertY )
    {
        m_fRawControllerPitch   = -m_fRawControllerPitch;
    }
    
    // Mirror weapon?
    extern xbool g_MirrorWeapon;
    if( g_MirrorWeapon )        
    {
        // Turn on mirror for hands
        m_AnimPlayer.SetMirrorBone( 0 );

        // Turn on mirror for weapon
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if( pWeapon )
            pWeapon->GetCurrentAnimPlayer().SetMirrorBone( 0 );
    }
    else
    {
        // Turn off mirror for hands
        m_AnimPlayer.SetMirrorBone( -1 );

        // Turn off mirror for weapon
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if( pWeapon )
            pWeapon->GetCurrentAnimPlayer().SetMirrorBone( -1 );
    }
    #endif
    
    m_fYawValue = m_fRawControllerYaw * BaseMouseSensitivity;
    m_fPitchValue = m_fRawControllerPitch * BaseMouseSensitivity;
    
    #if !defined(X_EDITOR)    
    u32 sensitivity_H = p.GetSensitivity(SM_X_SENSITIVITY);
    u32 sensitivity_V = p.GetSensitivity(SM_Y_SENSITIVITY);
    #else
    u32 sensitivity_H = 32; //HACK HACK HACK!!!
    u32 sensitivity_V = 32; //HACK HACK HACK!!!
    #endif
    
    f32 scaleH = 0.5f + (sensitivity_H / 100.0f);
    f32 scaleV = 0.5f + (sensitivity_V / 100.0f);
    
    m_fYawValue   *= scaleH; //R_0
    m_fPitchValue *= scaleV; //R_0
#else
    m_fRawControllerYaw = -g_IngamePad[m_ActivePlayerPad].GetLogical(ingame_pad::LOOK_HORIZONTAL).IsValue;
    m_fRawControllerPitch = g_IngamePad[m_ActivePlayerPad].GetLogical(ingame_pad::LOOK_VERTICAL).IsValue;

    player_profile& p = g_StateMgr.GetActiveProfile(g_StateMgr.GetProfileListIndex(m_LocalSlot));
	
    //MAB: removed invert Y global var - only check profile now
    if( p.m_bInvertY )
    {
        m_fRawControllerPitch = -m_fRawControllerPitch;
    }
#endif

    if ( !m_bInTurret )
    {
        m_fMoveValue   = +g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::MOVE_FOWARD_BACKWARDS ).IsValue;
        m_fStrafeValue = -g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::MOVE_STRAFE ).IsValue;

#if defined(TARGET_PC) && !defined(X_EDITOR)
        {
            xbool MoveForwardKeyIsPressed  = (xbool)g_IngamePad[m_ActivePlayerPad].GetLogical(ingame_pad::MOVE_FORWARD).WasValue;
            xbool MoveBackwardKeyIsPressed = (xbool)g_IngamePad[m_ActivePlayerPad].GetLogical(ingame_pad::MOVE_BACKWARD).WasValue;
            xbool StrafeLeftKeyIsPressed   = (xbool)g_IngamePad[m_ActivePlayerPad].GetLogical(ingame_pad::STRAFE_LEFT).WasValue;
            xbool StrafeRightKeyIsPressed  = (xbool)g_IngamePad[m_ActivePlayerPad].GetLogical(ingame_pad::STRAFE_RIGHT).WasValue;

            f32 MoveValue   = 0.0f;
            f32 StrafeValue = 0.0f;

            if (MoveForwardKeyIsPressed) 
            {
                MoveValue = 1.0f;
            } 
            else if (MoveBackwardKeyIsPressed) 
            {
                MoveValue = -1.0f;
            }
            
            if (StrafeLeftKeyIsPressed) 
            {
                StrafeValue = 1.0f;
            } 
            else if (StrafeRightKeyIsPressed) 
            {
                StrafeValue = -1.0f;
            }

            m_fMoveValue   += MoveValue;
            m_fStrafeValue += StrafeValue;
        }
#endif
    }
    else
    {
        m_fMoveValue   = 0.0f;
        m_fStrafeValue = 0.0f;
    }   
    //DrawLabelInFront( xfs( "RawYaw: %f\nRawPitch: %f\nRawMove: %f\nRawStrafe: %f\n", m_fRawControllerYaw, m_fRawControllerPitch, m_fMoveValue, m_fStrafeValue ) );
#ifndef TARGET_PC
    ScaleYawAndPitchValues();    
#endif 
}

//===========================================================================

void player::OnButtonInput( void )
{
    // don't allow player to switch weapons, zoom in, attack, etc.
    if( m_bHidePlayerArms )
    {
        return;
    }
    // This is to make sure you don't lean or fire on the same button press as 
    // voting or respawning, respectively.
    {   
        xbool PrimaryDown = (xbool)g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_PRIMARY ).IsValue;
        if( !PrimaryDown )
        {
            m_bRespawnButtonPressed = FALSE;
        }

        if( !g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_VOTE_YES ).IsValue && 
            !g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_VOTE_NO  ).IsValue )
        {
            m_bVoteButtonPressed = FALSE;
        }
    }

    //
    // Handle voting menu input
    //
    if( m_VoteCanCast )
    {
        if( !m_VoteMode )
        {
            // Activate vote mode / menu.
            if( g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_VOTE_MENU_ON ).WasValue )
            {
                m_VoteMode = TRUE;
                // Activate the vote menu.
                LOG_MESSAGE( "player::OnButtonInput", "Vote menu activated." );
                return;
            }
        }
        else
        {
            // Deactivate vote mode / menu.
            if( g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_VOTE_MENU_OFF ).WasValue )
            {
                m_VoteMode      = FALSE;
                // Deactivate the vote menu.
                LOG_MESSAGE( "player::OnButtonInput", "Vote menu deactivated." );
                return;
            }

            // Vote YES.
            if( g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_VOTE_YES ).WasValue )
            {
                m_VoteMode      = FALSE;
                m_VoteCanCast   = FALSE;
                m_bVoteButtonPressed = TRUE;
                // Deactivate the vote menu.
                VoteCast( +1 );
                LOG_MESSAGE( "player::OnButtonInput", "Vote YES." );
                return;
            }

            // Vote NO.
            if( g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_VOTE_NO ).WasValue )
            {
                m_VoteMode      = FALSE;
                m_VoteCanCast   = FALSE;
                m_bVoteButtonPressed = TRUE;
                // Deactivate the vote menu.
                VoteCast( -1 );
                LOG_MESSAGE( "player::OnButtonInput", "Vote NO." );
                return;
            }

            // Vote ABSTAIN.
            if( g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_VOTE_ABSTAIN ).WasValue )
            {
                m_VoteMode      = FALSE;
                m_VoteCanCast   = FALSE;
                // Deactivate the vote menu.
                VoteCast( 0 );
                LOG_MESSAGE( "player::OnButtonInput", "Vote ABSTAIN." );
                return;
            }
        }
    }
    else 
    {
        // So that the vote key doesn't show up if the vote expires while it's showing.
        m_VoteMode = FALSE;

        if( g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_DROP_FLAG ).IsValue )
        {
            #ifndef X_EDITOR
            m_NetDirtyBits |= DROP_ITEM_BIT;
            if( g_NetworkMgr.IsServer() )
                pGameLogic->DropFlag( m_NetSlot );
            #endif
        }
    }

    //
    // Do nothing if stunned.
    //
    if ( m_NonExclusiveStateBitFlag & NE_STATE_STUNNED )
    {
        return;
    }

    //update base class button input
    ASSERT( m_ActivePlayerPad != -1 );

    xbool bStopCrouching = FALSE;


#ifndef X_EDITOR
    if( g_NetworkMgr.IsOnline() )
    {
        if( g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_TALK_MODE_TOGGLE ).WasValue )
        {
            g_VoiceMgr.ToggleTalkMode();
        }

        // Voice chat
        {
            g_VoiceMgr.SetTalking( FALSE );

            if( g_VoiceMgr.IsHeadsetPresent() == TRUE )
            {
                // check for voice chat
                if( g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_CHAT ).IsValue )
                {
                    // Test to make sure the player is allowed to chat
                    if( (g_VoiceMgr.IsVoiceBanned()  == FALSE) &&
                        (g_VoiceMgr.IsVoiceEnabled() ==  TRUE) )
                         g_VoiceMgr.SetTalking( TRUE );
                }
            }
        }
    }

    player_profile& p = g_StateMgr.GetActiveProfile(g_StateMgr.GetProfileListIndex(m_LocalSlot));
    if( p.m_bCrouchOn )
    {
        xbool CrouchKeyPressed = (xbool)g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_CROUCH ).WasValue;
        if( CrouchKeyPressed )
        {
            // crouch is a toggle and we're crouching so turn it off
            if( IsCrouching() )
            {
                bStopCrouching = TRUE;
            }
            else if ( !m_bInTurret )
            {
                // move the arms a little
                m_ArmsVelocity += vector3( 0.0f, -g_CrouchUpVelocity, 0.0f );

                // Start crouching
                SetIsCrouching( TRUE );
            }
        }
    }
    else
#endif
    {
        xbool CrouchKeyIsPressed = (xbool)g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_CROUCH ).IsValue;
        if( CrouchKeyIsPressed )
        {
            // only do this if we weren't crouching previously
            if( !IsCrouching() && !m_bInTurret )
            {
                // move the arms a little
                m_ArmsVelocity += vector3( 0.0f, -g_CrouchUpVelocity, 0.0f );

                // Start crouching
                SetIsCrouching( TRUE );
            }
        }
        else
        {
            // Only do this if we are already crouching
            if( IsCrouching() )
            {
                bStopCrouching = TRUE;
            }
        }
    }

    // for whatever reason, we need to quit crouching
    if( bStopCrouching )
    {
        // Stop crouching
        SetIsCrouching( FALSE );

        // move the arms a little
        m_ArmsVelocity += vector3( 0.0f, g_CrouchUpVelocity, 0.0f );
    }

    // Jump button
    xbool JumpPressed = (xbool)g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_JUMP ).WasValue;
    if( !m_bInTurret && JumpPressed && m_bCanJump )
    {
        Jump();

        // Stop crouching when you jump
        SetIsCrouching( FALSE );
    }

    // Look for 'game speak' buttons.
    OnGameSpeak() ;

    //
    // Toggle mutation
    //
    xbool Multiplayer = FALSE;
#ifndef X_EDITOR
    Multiplayer = GameMgr.IsGameMultiplayer();
#endif

    //
    // mreed:
    // This is a little strange, but we need more pressure to toggle mutation when we're 
    // leaning. This means it's less likely to succeed with a "WasValue" since you only get
    // one shot at it. So, when leaning, we will use "IsValue" so that we have more than
    // one chance to get the pressure needed. The side-effect of this is that we are no longer
    // debounced for mutation when leaning.
    // Luckily (?), there is a mutation frequency timer that prevents rapid mutation/demutation
    // so we'll always get through the mutation cycle before we come back. Plus, this only matters
    // when intentionally trying to mutate while leaning.
    //
    // So, this is where we get the button press values
    //
    ingame_pad::logical& Logical   = g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_MUTATION   );
    ingame_pad::logical& MPLogical = g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_MP_MUTATE  );
    
    const xbool IsLeaning = x_abs( m_SoftLeanAmount ) > 0.1f;
    const f32 ActionMutation   = IsLeaning ? Logical.IsValue   : Logical.WasValue;
    const f32 ActionMPMutation = IsLeaning ? MPLogical.IsValue : MPLogical.WasValue;

    xbool MutationPressed = Multiplayer
                          ? (ActionMutation > 0.0f)
                          : (ActionMPMutation > 0.0f);

    static f32 MinTimeSinceUseToThrowGrenade = 1.0f;
    static f32 MinTimeSinceUseToMutate = 1.0f;

    if (   !m_bInTurret 
        &&  m_bCanToggleMutation 
        && !IsChangingMutation()
        &&  MutationPressed
        && (m_MutationChangeTime > MIN_TIME_BETWEEN_MUTATION_CHANGES)
        && (m_UseTime > MinTimeSinceUseToMutate) )
    {

        //
        // If we're leaning, we need to press harder to toggle mutation
        //
        xbool HaveButtonPressureToToggleMutation = TRUE;

        f32 MutationValue = 1.0f;

        if ( IsLeaning )
        {
#if    defined( TARGET_PS2  )
            MutationValue = input_GetValue( INPUT_PS2_BTN_L_UP, m_ActivePlayerPad );
#elif  defined( TARGET_XBOX )
            MutationValue = input_GetValue( INPUT_XBOX_BTN_UP,  m_ActivePlayerPad );
#endif
        }

        ASSERTS( g_MonkeyOptions.Enabled || (MutationValue > 0.0f), "Dpad Up is zero, when toggling mutation, the mapping has changed, find Mike Reed" );

        static f32 MinValueToToggleMutationWhileLeaning = 0.3f;
        HaveButtonPressureToToggleMutation = MutationValue > MinValueToToggleMutationWhileLeaning;

        if ( HaveButtonPressureToToggleMutation )
        {
            // OK, our input is in order, see what we need to do
            if( m_Inventory2.HasItem( INVEN_WEAPON_MUTATION ) && !IsMutated() )
            {
                SetupMutationChange(TRUE);
            }
            else if( IsMutated() )
            {
                SetupMutationChange(FALSE);
            }        
        }
    }

    if ( !m_bInTurret && g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_CYCLE_RIGHT ).WasValue )
    {
        OnWeaponSwitch2( CYCLE_RIGHT );
    }
    if ( !m_bInTurret && g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_CYCLE_LEFT ).WasValue )
    {
        OnWeaponSwitch2( CYCLE_LEFT );
    }

    //
    // NOTE: TWEEK THIS, WE MIGHT WANT TO MAKE THE RAMP DOWN FIRST BEFORE DOING MELEE.
    //
    xbool MeleePressed = m_bIsMutated
                        ? g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_MUTANT_MELEE ).IsValue > 0.0f
                        : g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_MELEE_ATTACK ).IsValue > 0.0f;
    if( !m_bInTurret && MeleePressed )
    {
        animation_state MeleeState = ANIM_STATE_MELEE;

        // make sure we are completely mutated
        if( IsMutated() )
        {
            switch( m_CurrentAnimState )
            {
                // we don't want melee kicking off when we are switching to while we are mutated.
                // This will cause the meshes to get hosed
            case ANIM_STATE_SWITCH_TO:
            case ANIM_STATE_SWITCH_FROM:
                {}
                break;

                //////////////////////////////////////////////////////////////////////////
                // put any other mutation special cases here
                //////////////////////////////////////////////////////////////////////////

            default:
                {                    

                    // if we are mutated, do extreme melee attack.                
                    if( m_bMutationMeleeEnabled )
                    {
                        // if we are already attacking, return.
                        // NOTE: we can't expect SetAnimState to check if the anims are the same because we have 5 different ones here.
                        if( !m_bMeleeLunging )
                        {
                            MeleeState = SetupMutationMeleeWeapon();
                            SetMeleeState(MeleeState);
                        }
                    }                    
                }
                break;
            }
        }
        else // we aren't mutated, do normal melee stuff
        {   
            if( m_ComboCount >= MAX_COMBO_HITS )
            {
                m_ComboCount = MAX_COMBO_HITS-1;
                ASSERT(0);
                return;
            }  

            if( m_ComboCount == 0 )
            {
                // if you don't do this, when you demutate you will swing your wittle human arms and it looks dumb :)
                if( m_CurrentAnimState != ANIM_STATE_SWITCH_FROM && m_CurrentAnimState != ANIM_STATE_DISCARD )
                {
                    if( m_bCanRequestCombo && m_bLastMeleeHit )
                    {
                        // Still stage 0 and we're requesting to start a combo
                        SetMeleeState(ANIM_STATE_COMBO_BEGIN);
                    }
                    else
                    {
                        // we aren't requesting a combo yet, do initial melee
                        SetMeleeState(MeleeState);
                    }
                }
            }            

            // if you can request a combo, set the flag
            if( m_bCanRequestCombo )
            {
                m_bHitCombo = TRUE;
            }
        }
    }

    // don't throw a grenade if we're just exiting fly mode
#if defined( ENABLE_DEBUG_MENU )
    xbool GrenadePressed = g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_THROW_GRENADE ).IsValue && 
                          !(g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_TALK_MODE_TOGGLE ).IsValue);
#else
    xbool GrenadePressed = g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_THROW_GRENADE ).IsValue;
#endif


    if( GrenadePressed )
    {
        if (   (m_Inventory2.GetAmount( m_CurrentGrenadeType2 ) > 0)
            && ( !IsMutated() )
            && AllowedToFire() 
            && !m_bInTurret
            && (m_UseTime > MinTimeSinceUseToThrowGrenade) )
        {
            // Get a reference to the state that we are considering
            s32 GrenadeState = (m_CurrentGrenadeType2==INVEN_GRENADE_FRAG) ? ANIM_STATE_GRENADE : ANIM_STATE_ALT_GRENADE;

            // if we are already throwing a grenade of any type, don't any other grenade be thrown
            if( m_CurrentAnimState != ANIM_STATE_GRENADE && m_CurrentAnimState != ANIM_STATE_ALT_GRENADE )
            {
                state_anims& State = m_Anim[inventory2::ItemToWeaponIndex(m_CurrentWeaponItem)][GrenadeState];

                // Can we fire the secondary weapon?
                if( State.nPlayerAnims > 0 )
                {
                    new_weapon* pWeapon = GetCurrentWeaponPtr();
                    if( pWeapon )
                        pWeapon->ClearZoom();

                    SetAnimState( (animation_state)GrenadeState );
                }
            }
        }
    }

    // flashlight button
    xbool FlashlightPressed = Multiplayer
                             ? (g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_MP_FLASHLIGHT ).IsValue > 0.0f)
                             : (g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_FLASHLIGHT ).WasValue > 0.0f);

    if ( FlashlightPressed && !IsMutated() )
    {
        new_weapon* pWeapon = GetCurrentWeaponPtr();

        if( pWeapon && pWeapon->HasFlashlight() )
        {
            SetFlashlightActive( !IsFlashlightActive() );
        }
        else
        {
            // weapon is invalid?  Turn off flashlight then
            SetFlashlightActive( FALSE );
        }
    }

    // Use on a mutagen reservoir
    xbool UseKeyPressed = g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_USE ).IsValue;
    
    if ( UseKeyPressed && NearMutagenReservoir() )
    {
        static const f32 ChangeRate = 10.0f;
        AddMutagen( ChangeRate * m_DeltaTime );

        // play the sucking sound when refilling mutagen from a super-contagious dead body.
        if( m_SuckingMutagenLoopID == 0 )
        {
            m_SuckingMutagenLoopID = g_AudioMgr.Play( "SCDB_Suck_Mutagen_Loop", GetPosition(), GetZone1(), TRUE );
        }
    }
    else
    {
        if( m_SuckingMutagenLoopID != 0 )
        {
            // not refilling anymore, release ID
            g_AudioMgr.Release( m_SuckingMutagenLoopID, 1.0f );
            m_SuckingMutagenLoopID = 0;
        }
    }

    xbool LeanLeftPressed = g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::LEAN_LEFT ).IsValue && !m_bVoteButtonPressed;
    xbool LeanRightPressed = g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::LEAN_RIGHT ).IsValue && !m_bVoteButtonPressed;

    if ( !m_bInTurret && LeanLeftPressed )
    {
        UpdateLean( 1.0f );
    }
    else if ( !m_bInTurret && LeanRightPressed )
    {
        UpdateLean( -1.0f );
    }
    else
    {
        UpdateLean( 0.0f );
    }
}

//==============================================================================