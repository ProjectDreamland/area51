//==============================================================================
//
//  PlayerOnline.cpp
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

static const f32 s_idle_timeout_min             = 5.f;
static const f32 s_ilde_timout_max              = 8.f;
static const f32 s_DebounceDuration             = 0.5f;

#ifdef ksaffel
static xbool g_ShowViewPosition = FALSE;
#endif

tweak_handle TapRefireRefreshSeconds_Tweak( "TapRefireRefreshSeconds" );

void player::BeginState( void )
{
    m_TimeInState = 0.0f;

    switch( m_CurrentAnimState )
    {
    case ANIM_STATE_SWITCH_TO:          BeginSwitchTo();        break;
    case ANIM_STATE_SWITCH_FROM:        BeginSwitchFrom();      break;
    case ANIM_STATE_IDLE:               BeginIdle();            break;
    case ANIM_STATE_RUN:                BeginRun();             break;
    case ANIM_STATE_PICKUP:             BeginPickup();          break;
    case ANIM_STATE_DISCARD:            BeginDiscard();         break;
        //------------------------------------------------------------------------------
    case ANIM_STATE_FIRE:               BeginFire();            break;
    case ANIM_STATE_ALT_FIRE:           BeginAltFire();         break;
    case ANIM_STATE_GRENADE:            BeginGrenade();         break;
    case ANIM_STATE_ALT_GRENADE:        BeginAltGrenade();      break;
    
    case ANIM_STATE_MELEE:              BeginMelee();           break;

    case ANIM_STATE_MUTATION_SPEAR:
    case ANIM_STATE_MELEE_FROM_CENTER:  
    case ANIM_STATE_MELEE_FROM_UP:
    case ANIM_STATE_MELEE_FROM_DOWN:
    case ANIM_STATE_MELEE_FROM_RIGHT:
    case ANIM_STATE_MELEE_FROM_LEFT:
        BeginMelee_Special(m_CurrentAnimState);   
        break;

    case ANIM_STATE_COMBO_BEGIN:        BeginCombo();           break;
    case ANIM_STATE_COMBO_HIT:          BeginCombo_Hit();       break;
    case ANIM_STATE_COMBO_END:          BeginCombo_End();       break;

        //------------------------------------------------------------------------------
    case ANIM_STATE_RELOAD:             BeginReload();          break;
    case ANIM_STATE_RELOAD_IN:          BeginReloadIn();        break;
    case ANIM_STATE_RELOAD_OUT:         BeginReloadOut();       break;
    case ANIM_STATE_RAMP_UP:            BeginRampUp();          break;
    case ANIM_STATE_RAMP_DOWN:          BeginRampDown();        break;
    case ANIM_STATE_ALT_RAMP_UP:        BeginAltRampUp();       break;
    case ANIM_STATE_ALT_RAMP_DOWN:      BeginAltRampDown();     break;
    case ANIM_STATE_HOLD:               BeginHold();            break;
    case ANIM_STATE_ALT_HOLD:           BeginAltHold();         break;
    case ANIM_STATE_ZOOM_IN:            BeginZoomIn();          break;
    case ANIM_STATE_ZOOM_OUT:           BeginZoomOut();         break;
    case ANIM_STATE_ZOOM_IDLE:          BeginZoomIdle();        break;
    case ANIM_STATE_ZOOM_RUN:           BeginZoomRun();         break;
    case ANIM_STATE_ZOOM_FIRE:          BeginZoomFire();        break;
        //------------------------------------------------------------------------------
    case ANIM_STATE_DEATH:              BeginDeath();           break;
    case ANIM_STATE_MISSION_FAILED:     BeginMissionFailed();   break;
    case ANIM_STATE_FALLING_TO_DEATH:   BeginFallingToDeath();  break;
    case ANIM_STATE_CINEMA:             BeginCinema();          break;
    case ANIM_STATE_CHANGE_MUTATION:    BeginMutationChange();  break;
    case ANIM_STATE_UNDEFINED:                                  break;
    default:
        x_throw( "Don't know how to begin this player state." );
        break;
    }

}

//------------------------------------------------------------------------------

void player::EndState( void )
{
    switch( m_CurrentAnimState )
    {
    case ANIM_STATE_SWITCH_TO:      EndSwitchTo();          break;
    case ANIM_STATE_SWITCH_FROM:    EndSwitchFrom();        break;
    case ANIM_STATE_IDLE:           EndIdle();              break;
    case ANIM_STATE_RUN:            EndRun();               break;
    case ANIM_STATE_PICKUP:         EndPickup();            break;
    case ANIM_STATE_DISCARD:        EndDiscard();           break;
        //------------------------------------------------------------------------------
    case ANIM_STATE_FIRE:           EndFire();              break;
    case ANIM_STATE_ALT_FIRE:       EndAltFire();           break;
    case ANIM_STATE_GRENADE:        EndGrenade();           break;
    case ANIM_STATE_ALT_GRENADE:    EndAltGrenade();        break;
    case ANIM_STATE_MELEE:          EndMelee();             break;

    case ANIM_STATE_MUTATION_SPEAR:
    case ANIM_STATE_MELEE_FROM_CENTER:  
    case ANIM_STATE_MELEE_FROM_UP:
    case ANIM_STATE_MELEE_FROM_DOWN:
    case ANIM_STATE_MELEE_FROM_RIGHT:
    case ANIM_STATE_MELEE_FROM_LEFT:
        EndMelee_Special(m_CurrentAnimState);   
        break;

    case ANIM_STATE_COMBO_BEGIN:        EndCombo();         break;
    case ANIM_STATE_COMBO_HIT:          EndCombo_Hit();     break;
    case ANIM_STATE_COMBO_END:          EndCombo_End();     break;

        //------------------------------------------------------------------------------
    case ANIM_STATE_RELOAD:         EndReload();            break;
    case ANIM_STATE_RELOAD_IN:      EndReloadIn();          break;
    case ANIM_STATE_RELOAD_OUT:     EndReloadOut();         break;
    case ANIM_STATE_RAMP_UP:        EndRampUp();            break;
    case ANIM_STATE_RAMP_DOWN:      EndRampDown();          break;
    case ANIM_STATE_ALT_RAMP_UP:    EndAltRampUp();         break;
    case ANIM_STATE_ALT_RAMP_DOWN:  EndAltRampDown();       break;
    case ANIM_STATE_HOLD:           EndHold();              break;
    case ANIM_STATE_ALT_HOLD:       EndAltHold();           break;
    case ANIM_STATE_ZOOM_IN:        EndZoomIn();            break;
    case ANIM_STATE_ZOOM_OUT:       EndZoomOut();           break;
    case ANIM_STATE_ZOOM_IDLE:      EndZoomIdle();          break;
    case ANIM_STATE_ZOOM_RUN:       EndZoomRun();           break;
    case ANIM_STATE_ZOOM_FIRE:      EndZoomFire();          break;
        //------------------------------------------------------------------------------
    case ANIM_STATE_DEATH:          EndDeath();             break;
    case ANIM_STATE_MISSION_FAILED: EndMissionFailed();     break;
    case ANIM_STATE_CHANGE_MUTATION:EndMutationChange();    break;
    case ANIM_STATE_FALLING_TO_DEATH:EndFallingToDeath();   break;
    case ANIM_STATE_CINEMA:         EndCinema();            break;
    case ANIM_STATE_UNDEFINED:                              break;
    default:
        x_throw( "Don't know how to End this player state." );
        break;
    } 
}

//------------------------------------------------------------------------------

void player::UpdateState( const f32& rDeltaTime )
{
#ifdef ksaffel
    //x_printfxy(2,0,"AnimState: %i",m_CurrentAnimState);
#endif
    m_TimeInState += rDeltaTime;

    switch( m_CurrentAnimState )
    {
    case ANIM_STATE_SWITCH_TO:      UpdateSwitchTo      (rDeltaTime);   break;
    case ANIM_STATE_SWITCH_FROM:    UpdateSwitchFrom    (rDeltaTime);   break;
    case ANIM_STATE_IDLE:           UpdateIdle          (rDeltaTime);   break;
    case ANIM_STATE_RUN:            UpdateRun           (rDeltaTime);   break;
    case ANIM_STATE_PICKUP:         UpdatePickup        (rDeltaTime);   break;
    case ANIM_STATE_DISCARD:        UpdateDiscard       (rDeltaTime);   break;
        //------------------------------------------------------------------------------
    case ANIM_STATE_FIRE:           UpdateFire          (rDeltaTime);   break;
    case ANIM_STATE_ALT_FIRE:       UpdateAltFire       (rDeltaTime);   break;
    case ANIM_STATE_GRENADE:        UpdateGrenade       (rDeltaTime);   break;
    case ANIM_STATE_ALT_GRENADE:    UpdateAltGrenade    (rDeltaTime);   break;
    case ANIM_STATE_MELEE:          UpdateMelee         (rDeltaTime);   break;

    case ANIM_STATE_MUTATION_SPEAR:
    case ANIM_STATE_MELEE_FROM_CENTER:  
    case ANIM_STATE_MELEE_FROM_UP:
    case ANIM_STATE_MELEE_FROM_DOWN:
    case ANIM_STATE_MELEE_FROM_RIGHT:
    case ANIM_STATE_MELEE_FROM_LEFT:
        UpdateMelee_Special(rDeltaTime, m_CurrentAnimState);   
        break;

    case ANIM_STATE_COMBO_BEGIN:        UpdateCombo(rDeltaTime);        break;
    case ANIM_STATE_COMBO_HIT:          UpdateCombo_Hit(rDeltaTime);    break;
    case ANIM_STATE_COMBO_END:          UpdateCombo_End(rDeltaTime);    break;

        //------------------------------------------------------------------------------
    case ANIM_STATE_RELOAD:         UpdateReload        (rDeltaTime);   break;
    case ANIM_STATE_RELOAD_IN:      UpdateReloadIn      (rDeltaTime);   break;
    case ANIM_STATE_RELOAD_OUT:     UpdateReloadOut     (rDeltaTime);   break;
    case ANIM_STATE_RAMP_UP:        UpdateRampUp        (rDeltaTime);   break;
    case ANIM_STATE_RAMP_DOWN:      UpdateRampDown      (rDeltaTime);   break;
    case ANIM_STATE_ALT_RAMP_UP:    UpdateAltRampUp     (rDeltaTime);   break;
    case ANIM_STATE_ALT_RAMP_DOWN:  UpdateAltRampDown   (rDeltaTime);   break;
    case ANIM_STATE_HOLD:           UpdateHold          (rDeltaTime);   break;
    case ANIM_STATE_ALT_HOLD:       UpdateAltHold       (rDeltaTime);   break;
    case ANIM_STATE_ZOOM_IN:        UpdateZoomIn        (rDeltaTime);   break;
    case ANIM_STATE_ZOOM_OUT:       UpdateZoomOut       (rDeltaTime);   break;
    case ANIM_STATE_ZOOM_IDLE:      UpdateZoomIdle      (rDeltaTime);   break;
    case ANIM_STATE_ZOOM_RUN:       UpdateZoomRun       (rDeltaTime);   break;
    case ANIM_STATE_ZOOM_FIRE:      UpdateZoomFire      (rDeltaTime);   break;
        //------------------------------------------------------------------------------
    case ANIM_STATE_DEATH:          UpdateDeath         (rDeltaTime);   break;
    case ANIM_STATE_MISSION_FAILED: UpdateMissionFailed (rDeltaTime);   break;
    case ANIM_STATE_CHANGE_MUTATION:UpdateMutationChange(rDeltaTime);   break;
    case ANIM_STATE_FALLING_TO_DEATH:UpdateFallingToDeath(rDeltaTime);  break;
    case ANIM_STATE_CINEMA:         UpdateCinema(rDeltaTime);           break;
    case ANIM_STATE_UNDEFINED:                                          break;
    default:
        x_throw( "Don't know how to Update this player state." );
        break;
    }
}

//==============================================================================
//==============================================================================
// ANIM_STATE_DEATH
//==============================================================================
//==============================================================================

void player::BeginDeath( void )
{
    m_DeathTime = 0.0f;

    if ( !GetThirdPersonCamera() )
    {
        m_AnimStage = 1;
        m_PosOverrideCamera = m_AnimPlayer.GetBonePosition( m_iCameraBone );


        //set the animation for the weapon
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        xbool bMutationWeapon = FALSE;
        if ( pWeapon )
        {
            if ( pWeapon->GetType() == object::TYPE_WEAPON_MUTATION )
            {
                bMutationWeapon = TRUE;
            }

            xbool bMultiplayer = FALSE;
#ifndef X_EDITOR
            bMultiplayer = GameMgr.IsGameMultiplayer();
#endif
            if( !bMultiplayer )
            {
                if ( bMutationWeapon )
                {
                    const anim_group& CurAnimGroup( pWeapon->GetCurrentAnimGroup() );
                    pWeapon->SetAnimation( CurAnimGroup.GetAnimIndex( "MUT_WPN_Death01" ), 0.0f , FALSE );
                }
                else
                {
                    state_anims& State = m_Anim[inventory2::ItemToWeaponIndex(m_CurrentWeaponItem)][ANIM_STATE_SWITCH_FROM];
                    pWeapon->SetAnimation( State.WeaponAnim[ANIM_PRIORITY_DEFAULT] , 0.0f , FALSE );
                }
                pWeapon->ResetWeapon();
            }
        }    
        else
        {
            LOG_ERROR( "player::BeginDeath", "Unable to resolve current weapon." );
        }

        if ( !bMutationWeapon )
        {
            //get gun off screen
            SetAnimation( ANIM_STATE_SWITCH_FROM , ANIM_PRIORITY_DEFAULT , 0.f );
        }
    }
}

//------------------------------------------------------------------------------

void player::UpdateDeath( const f32& DeltaTime )
{
    m_DeathTime += DeltaTime; 

    xbool PrimaryPressed = (xbool)g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_PRIMARY ).WasValue;

    if ( GetThirdPersonCamera() )
    {
        UpdateThirdPersonCamera();
        if( PrimaryPressed )
        {
            m_bWantToSpawn = TRUE;
#ifndef X_EDITOR
            m_NetDirtyBits |= WANT_SPAWN_BIT;  // NETWORK
#endif // X_EDITOR

            m_bRespawnButtonPressed = TRUE;
        }
    }
    else
    {

        //make sure override position need leaves player bounding box
        //used for falling so death cam keeps falling...
        m_PosOverrideCamera.GetY() = MIN( GetBBox().Max.GetY(), m_PosOverrideCamera.GetY() );
        m_PosOverrideCamera.GetY() = MAX( GetBBox().Min.GetY(), m_PosOverrideCamera.GetY() );

        //make sure camera is on ground
        m_AnimPlayer.SetPosition( m_PosOverrideCamera );

        //make sure anim pitch adjusts to correct position
        radian rPitch = m_AnimPlayer.GetPitch();
        if ( rPitch > 0 )
        {
            rPitch = MAX(0, (rPitch - (R_45*DeltaTime)));
        }
        else if ( rPitch < 0 )
        {
            rPitch = MIN(0, (rPitch + (R_45*DeltaTime)));
        }
        m_AnimPlayer.SetPitch(rPitch);

        if ((m_AnimStage == 1) || m_bIsMutated)
        {
            //rotate the weapon
            new_weapon* pWeapon = GetCurrentWeaponPtr();
            if (pWeapon)
            {
                pWeapon->SetRotation( m_AnimPlayer.GetPitch() , m_AnimPlayer.GetYaw() );
                OnMoveWeapon();
            }
        }

        if (m_AnimStage == 3)
        {
#if defined(TARGET_PC) && !defined(X_EDITOR)
            PrimaryPressed |= input_WasPressed( INPUT_MOUSE_BTN_L );
#endif
            if( PrimaryPressed )
            {
                m_bWantToSpawn = TRUE;
#ifndef X_EDITOR
                m_NetDirtyBits |= WANT_SPAWN_BIT;  // NETWORK
#endif // X_EDITOR

                m_bRespawnButtonPressed = TRUE;
            }
            return;
        }

        //advance death stage
        if ( m_AnimPlayer.IsAtEnd() )
        {
            m_AnimStage++;
            if (m_AnimStage == 2)
            {
                //start death anim
                SetAnimation( ANIM_STATE_DEATH , ANIM_PRIORITY_DEFAULT, 0.0f );
            }
        }
    }
}

//------------------------------------------------------------------------------

void player::EndDeath( void )
{
    corpse* pCorpse = (corpse*)g_ObjMgr.GetObjectByGuid( m_CorpseGuid );
    if( pCorpse )
        pCorpse->SetPermanent( FALSE );

    m_CorpseGuid  = 0;
    m_AnimStage     = 0;
    m_DeathType     = DEATH_BY_ANIM;

    if( GetThirdPersonCamera() )
    {
        g_ObjMgr.DestroyObjectEx( GetThirdPersonCameraGuid(), TRUE );
        m_ThirdPersonCameraGuid = 0;
    }
}

//==============================================================================
//==============================================================================
// ANIM_STATE_CHANGE_MUTATION
//==============================================================================
//==============================================================================

void player::BeginMutationChange( void )
{
#ifdef mreed 
    if ( m_bIsMutated )
        LOG_MESSAGE( "Mutation", "BeginMutationChange" );
#endif

    m_AnimStage = 1;

    //get gun off screen
    SetAnimation( ANIM_STATE_SWITCH_FROM , ANIM_PRIORITY_DEFAULT , 0.f );

    //set the animation for the weapon
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon )
    {
        state_anims& State = m_Anim[inventory2::ItemToWeaponIndex(m_CurrentWeaponItem)][ANIM_STATE_SWITCH_FROM];
        pWeapon->SetAnimation( State.WeaponAnim[ANIM_PRIORITY_DEFAULT] , 0.f , FALSE );
        pWeapon->ResetWeapon();
    }    
    else
    {
        LOG_ERROR( "player::BeginMutationChange", "Unable to resolve current weapon." );
    }
}

//------------------------------------------------------------------------------

void player::UpdateMutationChange( f32 DeltaTime )
{
    //update stun effect

    // Let's handle the weapon switch from stuff.
    if (m_AnimStage == 1 || m_AnimStage == 3 )
    {
        //rotate the weapon
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if( pWeapon )
        {
            pWeapon->SetRotation( m_AnimPlayer.GetPitch() , m_AnimPlayer.GetYaw() );
            OnMoveWeapon();
        }
    }

    //Now we have to upate the AnimStage.  Once reload ends, 
    if ( m_AnimPlayer.IsAtEnd() )
    {
        m_AnimStage++;
        if (m_AnimStage == 2)
        {
            //start now we want the change strain animation.
            SetAnimation( ANIM_STATE_CHANGE_MUTATION , ANIM_PRIORITY_DEFAULT, 0.0f );
#ifdef mreed 
            if ( m_bIsMutated )
                LOG_MESSAGE( "Mutation", "BeginMutationChange:m_AnimStage == 2" );
#endif
        }

        if ( m_AnimStage == 3 )
        {
#ifdef mreed 
            if ( m_bIsMutated )
                LOG_MESSAGE( "Mutation", "BeginMutationChange:m_AnimStage == 3" );
#endif
            SetCurrentStrain( );

            // Force a weapon switch back to the same spot.
            m_NextWeaponItem = GetNextAvailableWeapon2( CYCLE_LEFT );
            m_NextWeaponItem = GetNextAvailableWeapon2( CYCLE_RIGHT );

            m_PrevWeaponItem     = m_CurrentWeaponItem;
            m_CurrentWeaponItem  = m_NextWeaponItem;
            m_NextWeaponItem     = INVEN_NULL;

            new_weapon* pWeapon = GetCurrentWeaponPtr();
            ASSERT( pWeapon );
            pWeapon->SetupRenderInformation( );
            pWeapon->SetRotation( m_AnimPlayer.GetPitch() , m_AnimPlayer.GetYaw() );
            OnMoveWeapon();

            //start now we want the change strain animation.
            SetAnimation( ANIM_STATE_SWITCH_TO , ANIM_PRIORITY_DEFAULT, 0.0f );

            //set the animation for the weapon
            state_anims& State = m_Anim[inventory2::ItemToWeaponIndex(m_CurrentWeaponItem)][ANIM_STATE_SWITCH_TO];
            pWeapon->SetAnimation( State.WeaponAnim[ANIM_PRIORITY_DEFAULT] , 0.f , FALSE );
        }

        if ( m_AnimStage == 4 )
        {
#ifdef mreed 
            if ( m_bIsMutated )
                LOG_MESSAGE( "Mutation", "BeginMutationChange:m_AnimStage == 4" );
#endif
            SetAnimState( ANIM_STATE_IDLE );
        }
    }

    ( void ) DeltaTime;
}

//------------------------------------------------------------------------------

void player::EndMutationChange( void )
{
#ifdef mreed 
    if ( m_bIsMutated )
        LOG_MESSAGE( "Mutation", "EndMutationChange" );
#endif
    m_AnimStage = 0;
}

//==============================================================================
//==============================================================================
// ANIM_STATE_SWITCH_TO
//==============================================================================
//==============================================================================

void player::BeginSwitchTo( void )
{
    // Make sure the weapon we are switching to is visible
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon )
    {
        pWeapon->SetVisible(TRUE);

        // set up the proper render state
        pWeapon->SetRenderState(new_weapon::RENDER_STATE_PLAYER);        

        // update for things like the meson cannon that need to restart particles and such
        pWeapon->BeginSwitchTo();
    }

    // make sure if we've switched weapons manually or by picking one up that we clear our combo.
    ClearCombo();

    m_NextAnimState = ANIM_STATE_UNDEFINED;

    // Set appropriate 'switch to' animation
    SetAnimation( ANIM_STATE_SWITCH_TO , ANIM_PRIORITY_DEFAULT , 0.0f );
}

//------------------------------------------------------------------------------

void player::UpdateSwitchTo( const f32& DeltaTime )
{
    (void)DeltaTime;

    if ( m_AnimPlayer.IsAtEnd() )
    {
        // we don't reload, go ahead and play transition animation
        if( !ReloadWeapon( new_weapon::AMMO_PRIMARY ) )
        {   
            // Play the idle or run.
            SetAnimState( GetMotionTransitionAnimState() );
        }
    }
}

//------------------------------------------------------------------------------

void player::EndSwitchTo( void )
{
    LoadAimAssistTweakHandles();

    if ( m_bIsMutated )
    {
        // make sure we have lost our gloves
        const f32 Gloves = m_Inventory2.GetAmount( INVEN_GLOVES );
        if ( Gloves > 0.0f )
        {
            m_Inventory2.RemoveAmount( INVEN_GLOVES, Gloves );
        }
    }

    // Make sure the weapon knows we are done playing the switch anim
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon )
    {
        pWeapon->EndSwitchTo();
    }
}

//==============================================================================
//==============================================================================
// ANIM_STATE_SWITCH_FROM
//==============================================================================
//==============================================================================

void player::BeginSwitchFrom( void )
{
    // Make sure that the next weapon state has been set
    //ASSERT( m_NextVirtualWeapon != WEAPON_UNDEFINED );

    m_NextAnimState = ANIM_STATE_UNDEFINED;

    // Set appropriate 'switch from' animation
    if( !m_bDead )
    {
        SetAnimation( ANIM_STATE_SWITCH_FROM , ANIM_PRIORITY_DEFAULT , 0.f );
    }

    // see if we need to throw this weapon away (if it's dual)
    new_weapon *pNextWeapon = GetWeaponPtr(m_NextWeaponItem);

    // if this is a dual, we're out of ammo and we're switching weapons, get rid of it.
    // this means we probably switched weapons before the discard could play
    if( pNextWeapon && (pNextWeapon->GetAmmoCount( pNextWeapon->GetPrimaryAmmoPriority() ) <= 0) )
    {
        SetupDualWeaponDiscard(m_NextWeaponItem);
    }

    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon )
    {
        pWeapon->ClearZoom();

        // make sure we set weapon to idle.  For weapons that recharge even while not active weapon.
        pWeapon->BeginIdle();

        // notify the weapon of a switch from.  For anything that needs to kill effects outright or whatever.
        pWeapon->BeginSwitchFrom();
    }

    ResetStickSensitivity();
}

//------------------------------------------------------------------------------

void player::UpdateSwitchFrom( const f32& DeltaTime )
{
    (void)DeltaTime;

    if ( m_AnimPlayer.IsAtEnd() )
    {
        ForceNextWeapon();
    }
}

//------------------------------------------------------------------------------

void player::EndSwitchFrom( void )
{
}

//==============================================================================
//==============================================================================
// ANIM_STATE_IDLE
//==============================================================================
//==============================================================================

void player::BeginIdle( void )
{
    //Set the animation timers and 
    m_fAnimationTime = 0.f;
    m_fMaxAnimTime = x_frand( s_idle_timeout_min , s_ilde_timout_max );

    m_NextAnimState = ANIM_STATE_UNDEFINED;

    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon && !pWeapon->GetIdleMode() )
    {
        pWeapon->BeginIdle();

        // set up the proper render state
        pWeapon->SetRenderState( new_weapon::RENDER_STATE_PLAYER );
    }

    // Play the default idle animation for this state
    SetAnimation( ANIM_STATE_IDLE , ANIM_PRIORITY_DEFAULT );
}

//------------------------------------------------------------------------------

void player::HandleFireInput( xbool IsAlternateFire )
{   
    new_weapon* pWeapon = GetCurrentWeaponPtr();

    // if your arms are hidden, don't fire.
    if( !AllowedToFire() )
    {
        return;
    }

    if ( pWeapon )
    {
        if ( pWeapon->GetType() == TYPE_WEAPON_MUTATION )
        {
            if ( (IsAlternateFire && !m_bSecondaryMutationFireEnabled) ||
                (!IsAlternateFire && !m_bPrimaryMutationFireEnabled) )
            {
                // we can't use this shot now -- not allowed
                return;
            }

            // do we have enough juice to fire?
            if( !GetMutationMeleeWeapon()->CanFire(IsAlternateFire) )
            {
                return;
            }
        }
    }
    else
    {
        g_AudioMgr.Play( "NoAmmo" );
        return;
    }


    // KSS -- FIXME -- Disable secondary fire for Gamer's Day as per Daryl.
    if( pWeapon->GetType() == object::TYPE_WEAPON_MSN && IsAlternateFire )
    {
        return;
    }

    if( pWeapon->CanFire(IsAlternateFire) )
    {
        // if it's the gauss, ramp up
        animation_state RampUpState = IsAlternateFire ? ANIM_STATE_ALT_RAMP_UP : ANIM_STATE_RAMP_UP;

        if( IsAnimStateAvailable2( m_CurrentWeaponItem, RampUpState ) )
        {
            SetAnimState( RampUpState );
        }
        else
        {
            if ( m_CurrentWeaponItem == INVEN_WEAPON_TRA )
            {
                // we're ignoring input for turret alt fire (machine gun), because we want
                // it disabled
                if ( !IsAlternateFire )
                {
                    SetAnimState( ANIM_STATE_FIRE );
                }
            }
            else
            {
                SetAnimState( IsAlternateFire ? ANIM_STATE_ALT_FIRE : ANIM_STATE_FIRE );
            }
        }
    }
    else
    {
        if( pWeapon->CanReload( pWeapon->GetPrimaryAmmoPriority() ) )
        {
            SetAnimState( ANIM_STATE_RELOAD );
        }
        else
        {
            g_AudioMgr.Play( "NoAmmo" );
        }
    }
}

//------------------------------------------------------------------------------

void player::UpdateIdle( const f32& DeltaTime )
{
    (void)DeltaTime;

    // don't allow player to switch weapons, zoom in, attack, etc.
    if( !m_bHidePlayerArms )
    {
        // Check if we need to go back to a firing state
        if( IsFiring() )
        {
            HandleFireInput( FALSE );
        }    
        else if( IsAltFiring() )
        {
            // Get a reference to the state that we are considering
            state_anims& State = m_Anim[inventory2::ItemToWeaponIndex(m_CurrentWeaponItem)][ANIM_STATE_ALT_FIRE];

            // Can we fire the secondary weapon?
            if( State.nWeaponAnims > 0 )
            {
                HandleFireInput( TRUE );
            }
            else
            {
                if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_ZOOM_IN ) )            
                {
                    SetAnimState( ANIM_STATE_ZOOM_IN );
                }
            }
        }

        xbool bUsedFocusObject = UseFocusObject();
        xbool bPressedReload   = !bUsedFocusObject && (g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_RELOAD ).WasValue > 0.0f); 

        // Reloading?
        if( bPressedReload && !NearMutagenReservoir() )
        {
            ReloadWeapon(new_weapon::AMMO_SECONDARY, FALSE);
        }
    }

    // Check if we need to change to another motion animation.
    if ( m_Physics.GetVelocity().LengthSquared() > m_fMinRunSpeed )
    {
        SetAnimState( GetMotionTransitionAnimState() );
        return;
    }

    // Watch the animation timers and player's motion.  Switch the animation if necessary.
    m_fAnimationTime += DeltaTime;

    // Wait until the current animtion ends, then switch the animation
    if( m_AnimPlayer.IsAtEnd() )
    {
        state_anims& State = m_Anim[inventory2::ItemToWeaponIndex(m_CurrentWeaponItem)][ANIM_STATE_IDLE];

        // Are we allowed to switch to a different Idle animation?
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if( pWeapon && pWeapon->CanSwitchIdleAnim() &&
            ( m_fAnimationTime > m_fMaxAnimTime ) )
        {
            // If we're currently playing the default animation, check if different idle animations
            // exist if so then switch to one of them, otherwise restart the idle state.
            if( (m_AnimPlayer.GetAnimIndex() == State.PlayerAnim[ANIM_PRIORITY_DEFAULT]) && 
                (State.nPlayerAnims > 1) )
            {
                s32 nRandAnimIndex = x_irand( 1 , State.nPlayerAnims - 1 );

                if( m_CurrentAnimStateIndex == nRandAnimIndex )
                    nRandAnimIndex = 0;

                SetAnimation( ANIM_STATE_IDLE , nRandAnimIndex );

                //Set the animation timers and 
                m_fAnimationTime = 0.f;
                m_fMaxAnimTime = x_frand( s_idle_timeout_min , s_ilde_timout_max );
            }
            else
            {
                SetAnimation( ANIM_STATE_IDLE , ANIM_PRIORITY_DEFAULT );
            }
        }
        else
        {
            SetAnimation( ANIM_STATE_IDLE , ANIM_PRIORITY_DEFAULT );
        }
    }
}

//------------------------------------------------------------------------------

void player::EndIdle( void )
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( (m_NextAnimState != ANIM_STATE_RUN) && pWeapon )
        pWeapon->EndIdle();
}

//==============================================================================
//==============================================================================
// ANIM_STATE_RUN
//==============================================================================

void player::BeginRun( void )
{
    // If the previous state is walking, we need to blend into the run animation
    f32 fBlendTime = 0.0f;
    if( m_PreviousAnimState == ANIM_STATE_IDLE )
    {
        fBlendTime = DEFAULT_BLEND_TIME;
    }

    m_NextAnimState = ANIM_STATE_UNDEFINED;

    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon && !pWeapon->GetIdleMode() )
        pWeapon->BeginIdle();

    // Player is running, set the animation accordingly
    SetAnimation( ANIM_STATE_RUN , 0 , fBlendTime );    
}

//------------------------------------------------------------------------------

void player::UpdateRun( const f32& DeltaTime )
{
    (void)DeltaTime;

    // don't allow player to switch weapons, zoom in, attack, etc.
    if( !m_bHidePlayerArms )
    {
        // Check if we need to go back to a firing state
        if( IsFiring() )
        {
            HandleFireInput( FALSE );
        }
        else if( IsAltFiring() )
        {
            // Get a reference to the state that we are considering
            state_anims& State = m_Anim[inventory2::ItemToWeaponIndex(m_CurrentWeaponItem)][ANIM_STATE_ALT_FIRE];

            // Can we fire the secondary weapon?
            if( State.nWeaponAnims > 0 )
            {
                HandleFireInput( TRUE );
            }
            else
            {
                if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_ZOOM_IN ) )
                {
                    SetAnimState( ANIM_STATE_ZOOM_IN );
                }
            }
        }

        xbool bUsedFocusObject = UseFocusObject();
        xbool bPressedReload   = !bUsedFocusObject && (g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_RELOAD ).WasValue > 0.0f); 

        // Reloading?
        if( bPressedReload && !NearMutagenReservoir() )
        {
            ReloadWeapon(new_weapon::AMMO_PRIMARY, FALSE);
        }
    }

    // Check if we need to change to another motion animation.
    if ( m_Physics.GetVelocity().LengthSquared() < m_fMinRunSpeed )
    {
        SetAnimState( GetMotionTransitionAnimState() );
    }
}

//------------------------------------------------------------------------------

void player::EndRun( void )
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( (m_NextAnimState != ANIM_STATE_IDLE) && pWeapon )
        pWeapon->EndIdle();
}

//==============================================================================
//==============================================================================
// ANIM_STATE_PICKUP
//==============================================================================
//==============================================================================

void player::BeginPickup( void )
{
    // Make sure that the next weapon state has been set
    //ASSERT( m_NextVirtualWeapon != WEAPON_UNDEFINED );

    // Set previous weapon and current weapon, clear next weapon
    m_PrevWeaponItem     = m_CurrentWeaponItem;
    m_CurrentWeaponItem  = m_NextWeaponItem;
    m_NextWeaponItem     = INVEN_NULL;

    // zero out the reticle radius
    m_ReticleRadius             = 0.0f;
    m_ReticleGrowSpeed          = 0.0f;
    m_AimAssistData.bReticleOn  = FALSE;

    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon )
    {
        pWeapon->SetupRenderInformation( );
        pWeapon->SetRotation( m_AnimPlayer.GetPitch() , m_AnimPlayer.GetYaw() );
        OnMoveWeapon();

#ifndef X_EDITOR
        m_NetDirtyBits |= WEAPON_BIT;  // NETWORK
#endif // X_EDITOR
    }
    else
    {
        SetAnimState( ANIM_STATE_UNDEFINED );
    }

    m_NextAnimState = ANIM_STATE_UNDEFINED;

    // Set appropriate 'switch from' animation
    SetAnimation( ANIM_STATE_PICKUP , ANIM_PRIORITY_DEFAULT , 0.f );

    if ( pWeapon )
    {
        pWeapon->SetVisible( TRUE );
        pWeapon->ClearZoom();
    }
}

//------------------------------------------------------------------------------

void player::UpdatePickup( const f32& DeltaTime )
{
    (void)DeltaTime;

    if ( m_AnimPlayer.IsAtEnd() )
    {
        // Bring the new weapon up.
        SetAnimState( GetMotionTransitionAnimState() );

    }
}

//------------------------------------------------------------------------------

void player::EndPickup( void )
{
}

//==============================================================================
//==============================================================================
// ANIM_STATE_DISCARD
//==============================================================================
//==============================================================================

void player::BeginDiscard( void )
{
    m_NextAnimState = ANIM_STATE_UNDEFINED;
    SetAnimation( ANIM_STATE_DISCARD , ANIM_PRIORITY_DEFAULT );
}

//------------------------------------------------------------------------------

void player::UpdateDiscard( const f32& DeltaTime )
{
    (void)DeltaTime;

    if ( m_AnimPlayer.IsAtEnd() )
    {
        // not a dual weapon, discard normal weapon
        if( !SetupDualWeaponDiscard(m_CurrentWeaponItem) )
        {
            if( m_Inventory2.HasItem( m_CurrentWeaponItem ) )
            {
                m_Inventory2.RemoveAmount( m_CurrentWeaponItem, 1.0f );
            }

            // Set previous weapon and current weapon, clear next weapon
            m_PrevWeaponItem     = m_CurrentWeaponItem;
            m_CurrentWeaponItem  = m_NextWeaponItem;
            m_NextWeaponItem     = INVEN_NULL;
        }

        // zero out the reticle radius
        m_ReticleRadius             = 0.0f;
        m_ReticleGrowSpeed          = 0.0f;
        m_AimAssistData.bReticleOn  = FALSE;

        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if( pWeapon )
        {
            pWeapon->SetupRenderInformation( );
            pWeapon->SetRotation( m_AnimPlayer.GetPitch() , m_AnimPlayer.GetYaw() );
            OnMoveWeapon();

            // see if we need to reload after discarding other weapon
            if( !ReloadWeapon(new_weapon::AMMO_PRIMARY) )
            {
                // Bring the new weapon up.
                SetAnimState( ANIM_STATE_IDLE );
            }

#ifndef X_EDITOR
            m_NetDirtyBits |= WEAPON_BIT;  // NETWORK
#endif // X_EDITOR
        }
        else
        {
            SetAnimState( ANIM_STATE_UNDEFINED );
        }
    }
}

//------------------------------------------------------------------------------
void player::EndDiscard( void )
{
    LoadAimAssistTweakHandles();
}

//==============================================================================
//==============================================================================
//  ANIM_STATE_FIRE
//==============================================================================
//==============================================================================

void player::BeginFire( void )
{
    // Generate the percentages used to determine what firing animations are being played.
    GenerateFiringAnimPercentages();

    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if ( pWeapon  )
    {
        pWeapon->BeginPrimaryFire();
#ifndef X_EDITOR

        // SMP has looping anims, so handle that separately
        if( ( pWeapon->GetType() != TYPE_WEAPON_SMP ) && ( pWeapon->GetType() != TYPE_WEAPON_DUAL_SMP ) )
        {
            // Play fire on 3rd person avatar
            net_FirePrimary();        
        }
        else
        {
            // Begin looping fire on 3rd person avatar
            net_BeginFirePrimary();
        }
#endif
    }

    // we can't tap fire again until release
    m_bCanTapFire = FALSE;

    // reset timer
    m_TapRefireTime = 0.0f;

    m_NextAnimState = ANIM_STATE_UNDEFINED;

    // If we have more than one fire anim then play a different one each time we fire.    
    state_anims& WeaponState    = m_Anim[inventory2::ItemToWeaponIndex(m_CurrentWeaponItem)][ANIM_STATE_FIRE];

    // TODO: CJ: WEAPONS: The SMP was failing to fire because it had 2 fire animations, why?
    if(0)
        //    if( WeaponState.nWeaponAnims > 1 )
    {   
        if( m_LastFireAnimStateIndex >= WeaponState.nWeaponAnims )
            m_LastFireAnimStateIndex = 0;

        SetAnimation( ANIM_STATE_FIRE, m_LastFireAnimStateIndex, 0.0f );
        m_LastFireAnimStateIndex++;
    }
    else
    {
        SetAnimation( ANIM_STATE_FIRE, ANIM_PRIORITY_DEFAULT, 0.0f );
    }
}

//------------------------------------------------------------------------------
void player::UpdateFire( const f32& DeltaTime )
{
    (void)DeltaTime;

    m_TapRefireTime += DeltaTime;

    m_LastTimeWeaponFired = (f32)x_GetTimeSec();   
    
    // If the current weapon needs to be reloaded, set the next state to reload
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon && (pWeapon->GetAmmoCount( pWeapon->GetPrimaryAmmoPriority() ) <= 0) )
    {
        if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_DISCARD ) )
        {
            // Discard
            SetAnimState( ANIM_STATE_DISCARD );
            m_NextAnimState = ANIM_STATE_DISCARD;
        }
        else
        {
            m_NextAnimState =  GetMotionTransitionAnimState();
        }
    }

    // Is the player still holding the fire button?
    xbool   bFiring = IsFiring();

    // Do we need to reload?  If the weapon has a ramp down play that first.
    // If we run out of ammo and can't reload go to the Idle state.
    if( m_NextAnimState == ANIM_STATE_RELOAD )
    {
        if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_ALT_RAMP_DOWN ) )
        {
            SetAnimState( ANIM_STATE_RAMP_DOWN );
        }
        else
        {
            SetAnimState( m_NextAnimState );
        }
        return;
    }
    else if( (m_NextAnimState == ANIM_STATE_IDLE) || (m_NextAnimState == ANIM_STATE_RUN) )
    {
        if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_RAMP_DOWN ) )
        {
            SetAnimState( ANIM_STATE_RAMP_DOWN );
        }
        else
        {
            SetAnimState( m_NextAnimState );
        }

        // see if we need to reload
        ReloadWeapon(new_weapon::AMMO_PRIMARY);

        return;
    }

    if ( m_NextAnimState != ANIM_STATE_DISCARD )
    {
        // When the currently playing animation ends, need to set another animation
        if( bFiring )
        {
            // Still firing away.
            if( m_AnimPlayer.IsAtEnd() || 
                (pWeapon->CanFastTapFire() && m_bCanTapFire && (m_TapRefireTime >= TapRefireRefreshSeconds_Tweak.GetF32())) )
            {
                // Start fire animation
                state_anims& WeaponState = m_Anim[inventory2::ItemToWeaponIndex(m_CurrentWeaponItem)][ANIM_STATE_FIRE];
                if( WeaponState.nWeaponAnims > 1 )
                {
                    if( m_LastFireAnimStateIndex >= WeaponState.nWeaponAnims )
                        m_LastFireAnimStateIndex = 0;

                    SetAnimation( ANIM_STATE_FIRE, m_LastFireAnimStateIndex, 0.0f );
                    m_LastFireAnimStateIndex++;
                }
                else
                {
                    SetAnimation( ANIM_STATE_FIRE, ANIM_PRIORITY_DEFAULT, 0.0f );
                }
                
#ifndef X_EDITOR            
                // SMP has looping anims, so handle that separately
                if( ( pWeapon->GetType() != TYPE_WEAPON_SMP ) && ( pWeapon->GetType() != TYPE_WEAPON_DUAL_SMP ) )
                {
                    // Play fire on 3rd person avatar
                    net_FirePrimary();        
                }
#endif
                // we can't tap fire again until release
                m_bCanTapFire = FALSE;

                // reset timer
                m_TapRefireTime = 0.0f;
            }
        }
        else
        {
            // we've relased the button, signal a tap fire.
            m_bCanTapFire = TRUE;
        
            // can we interrupt primary fire?
            if( pWeapon && pWeapon->CanIntereptPrimaryFire( m_CurrentAnimIndex ) )
            {
                // We stopped firing, set appropriate animation state
                if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_RAMP_DOWN ) ) 
                {
                    SetAnimState( ANIM_STATE_RAMP_DOWN );
                }
                else
                {
                    SetAnimState( GetMotionTransitionAnimState() );
                }
            }
        }
    }
}

//------------------------------------------------------------------------------

void player::EndFire( void )
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon )
    {
        pWeapon->EndPrimaryFire();

#ifndef X_EDITOR
        // Stop looping fire for SMP
        if( ( pWeapon->GetType() == TYPE_WEAPON_SMP ) || ( pWeapon->GetType() == TYPE_WEAPON_DUAL_SMP ) )
        {
            net_EndFirePrimary();
        }
#endif
    }

    m_NextAnimState = ANIM_STATE_UNDEFINED;
}

//==============================================================================
//==============================================================================
//  ANIM_STATE_ALT_FIRE
//==============================================================================
//==============================================================================

void player::BeginAltFire( void )
{
    // Generate the percentages used to determine what firing animations are being played.
    GenerateFiringAnimPercentages();

    m_NextAnimState = ANIM_STATE_UNDEFINED;

    new_weapon* pWeapon = GetCurrentWeaponPtr();

    // tell the weapon we are starting to fire (currently only for meson cannon).
    if( pWeapon )
    {
        pWeapon->BeginAltFire();
    }

    // Set appropriate idle firing animation
    SetAnimation( ANIM_STATE_ALT_FIRE , ANIM_PRIORITY_DEFAULT, 0.0f );
    
#ifndef X_EDITOR            
    // Play fire on 3rd person avatar
    net_FireSecondary();
#endif
    
}

//------------------------------------------------------------------------------

void player::UpdateAltFire( const f32& DeltaTime )
{
    (void)DeltaTime;

    // Time to reload or go to fire empty.
    // If the current weapon needs to be reloaded, set the next state to reload
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon 
        && (   pWeapon->GetAmmoCount( pWeapon->GetSecondaryAmmoPriority() ) <= 0) 
        && (m_AnimPlayer.IsAtEnd()) )
    {
        if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_DISCARD ) )
        {
            // Discard
            SetAnimState( ANIM_STATE_DISCARD );
            m_NextAnimState = ANIM_STATE_DISCARD;
        }
        else
        {
            m_NextAnimState =  GetMotionTransitionAnimState();
        }
    }
    else if ( pWeapon 
        && ((pWeapon->GetAmmoCount( pWeapon->GetPrimaryAmmoPriority() ) <= 0) 
        && (m_AnimPlayer.IsAtEnd())) )
    {
        if ( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_DISCARD ) )
        {
            // Discard
            SetAnimState( ANIM_STATE_DISCARD );
            m_NextAnimState = ANIM_STATE_DISCARD;
        }
        else
        {
            // If we can reload, do so.
            // Otherwise, go immediately to alt fire empty
            if ( pWeapon->CanReload( pWeapon->GetPrimaryAmmoPriority() ) )
                m_NextAnimState = ANIM_STATE_RELOAD;
            else
                m_NextAnimState =  GetMotionTransitionAnimState();
        }
    }

    // When the currently playing animation ends, need to set another animation
    xbool bFiring = FALSE;
    if( IsAltFiring() )
        bFiring = TRUE;

    // Do we need to reload?  If the weapon has a ramp down play that first.
    // If we run out of ammo and can't reload go to the Idle state.
    if( m_NextAnimState == ANIM_STATE_RELOAD )
    {
        if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_ALT_RAMP_DOWN ) )
        {
            SetAnimState( ANIM_STATE_ALT_RAMP_DOWN );
        }
        else
        {
            SetAnimState( m_NextAnimState );
        }
        return;
    }
    else if( (m_NextAnimState == ANIM_STATE_IDLE) || (m_NextAnimState == ANIM_STATE_RUN) )
    {
        if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_ALT_RAMP_DOWN ) )
        {
            SetAnimState( ANIM_STATE_ALT_RAMP_DOWN );
        }
        else
        {
            SetAnimState( m_NextAnimState );
        }

        // see if we need to reload
        ReloadWeapon(new_weapon::AMMO_SECONDARY);

        return;
    }

    if ( m_NextAnimState != ANIM_STATE_DISCARD )
    {
        xbool bCheckRampDown = FALSE;
        // When the currently playing animation ends, need to set another animation
        if( bFiring )
        {
            // Still firing away.
            if( m_AnimPlayer.IsAtEnd() )
            {
                if( pWeapon->CanAltChainFire() )
                {
                    //s Sitch to running / firing state?
                    s32 nAnimIndex = GetNextFiringAnimIndex();
                    SetAnimation( ANIM_STATE_ALT_FIRE, nAnimIndex , 0.f );
                    
#ifndef X_EDITOR            
                    // Play fire on 3rd person avatar
                    net_FireSecondary();
#endif
                }
                else
                {
                    bCheckRampDown = TRUE;
                }
            }
        }
        else
        {
            bCheckRampDown = TRUE;
        }

        // check if we need to ramp down or transition
        if( bCheckRampDown )
        {
            if( pWeapon && pWeapon->CanIntereptSecondaryFire( m_CurrentAnimIndex ) )
            {
                // We stopped firing, set appropriate animation state
                if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_ALT_RAMP_DOWN ) ) 
                {
                    SetAnimState( ANIM_STATE_ALT_RAMP_DOWN );
                }
                else// if( m_AnimPlayer.IsAtEnd() )
                {
                    SetAnimState( GetMotionTransitionAnimState() );
                }
            }
        }
    }
}

//------------------------------------------------------------------------------

void player::EndAltFire( void )
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon )
    {
        pWeapon->EndAltFire();
    }        

    m_NextAnimState = ANIM_STATE_UNDEFINED;
}

//==============================================================================
//==============================================================================
//  ANIM_STATE_GRENADE
//==============================================================================
//==============================================================================

void player::BeginGrenade( void )
{
    m_NextAnimState = ANIM_STATE_UNDEFINED;

    // Set appropriate grenade throwing animation.
    SetAnimation( ANIM_STATE_GRENADE , ANIM_PRIORITY_DEFAULT );

    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if ( pWeapon )
    {
        pWeapon->ClearZoom();
    }

#ifndef X_EDITOR
    // Play toss grenade anim on 3rd person avatar
    net_Grenade();
#endif // X_EDITOR

}

//------------------------------------------------------------------------------

void player::UpdateGrenade( const f32& DeltaTime )
{
    (void)DeltaTime;

    // When the currently playing animation ends, need to set another animation
    if ( m_AnimPlayer.IsAtEnd() )
    {
        // see if we need to throw this weapon away (if it's dual)
        new_weapon *pWeapon = GetWeaponPtr(m_CurrentWeaponItem);

        // if this is a dual, we're out of ammo and we've thrown a grenade, get rid of it.        
        if( pWeapon && (pWeapon->GetAmmoCount( pWeapon->GetPrimaryAmmoPriority() ) <= 0) )
        {
            if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_DISCARD ) )
            {
                // Discard
                SetAnimState( ANIM_STATE_DISCARD );
                m_NextAnimState = ANIM_STATE_DISCARD;
                return;
            }
        }
        
        // see if we interrupted a reload
        if( !ReloadWeapon(new_weapon::AMMO_PRIMARY) )
        {
            SetAnimState( GetMotionTransitionAnimState() );
        }
    }
}

//------------------------------------------------------------------------------

void player::EndGrenade( void )
{
}

//==============================================================================
//==============================================================================
//  ANIM_STATE_ALT_GRENADE
//==============================================================================
//==============================================================================

void player::BeginAltGrenade( void )
{
    m_NextAnimState = ANIM_STATE_UNDEFINED;

    // Set appropriate grenade throwing animation.
    SetAnimation( ANIM_STATE_ALT_GRENADE , ANIM_PRIORITY_DEFAULT );

    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if ( pWeapon )
    {
        pWeapon->ClearZoom();
    }

#ifndef X_EDITOR
    // Play toss grenade anim on 3rd person avatar
    net_Grenade();
#endif // X_EDITOR
}

//------------------------------------------------------------------------------

void player::UpdateAltGrenade( const f32& DeltaTime )
{
    (void)DeltaTime;

    // When the currently playing animation ends, need to set another animation
    if ( m_AnimPlayer.IsAtEnd() )
    {
        // see if we need to throw this weapon away (if it's dual)
        new_weapon *pWeapon = GetWeaponPtr(m_CurrentWeaponItem);

        // if this is a dual, we're out of ammo and we've thrown a grenade, get rid of it.        
        if( pWeapon && (pWeapon->GetAmmoCount( pWeapon->GetPrimaryAmmoPriority() ) <= 0) )
        {
            if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_DISCARD ) )
            {
                // Discard
                SetAnimState( ANIM_STATE_DISCARD );
                m_NextAnimState = ANIM_STATE_DISCARD;

                return;
            }            
        }

        // see if we interrupted a reload
        if( !ReloadWeapon(new_weapon::AMMO_PRIMARY) )
        {
            SetAnimState( GetMotionTransitionAnimState() );
        }
    }
}

//------------------------------------------------------------------------------

void player::EndAltGrenade( void )
{

}

//==============================================================================
//==============================================================================
//  ANIM_STATE_MELEE
//==============================================================================
//==============================================================================

void player::BeginMelee( void )
{
    m_NextAnimState = ANIM_STATE_UNDEFINED;

    SetAnimation( ANIM_STATE_MELEE, ANIM_PRIORITY_DEFAULT );

    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if ( pWeapon )
    {
        pWeapon->ClearZoom();
    }

    // just make sure we have a clean slate
    ClearCombo();

#ifdef ksaffel
    //    x_printfxy(0,0, "BeginMelee" );
#endif
}

//------------------------------------------------------------------------------

void player::UpdateMelee( const f32& DeltaTime )
{
    (void)DeltaTime;

#ifdef ksaffel
    //x_printfxy(0,0, "UpdateMelee" );
#endif

    // When the currently playing animation ends, need to set another animation
    if ( m_AnimPlayer.IsAtEnd() )
    {
        // try to reload the weapon (just in case we interrupted it)
        if( !ReloadWeapon(new_weapon::AMMO_PRIMARY) )
        {
            SetAnimState( GetMotionTransitionAnimState() );
        }
    }
}
//------------------------------------------------------------------------------

void player::EndMelee( void )
{
    m_PlayMeleeSound = TRUE;
    
    // clear combo stuff
    if( !m_bHitCombo || !m_bLastMeleeHit )
    {
        ClearCombo();
    }
    // clear flag
    m_bHitCombo = FALSE;
}

//------------------------------------------------------------------------------
void player::BeginCombo( void )
{
    m_NextAnimState = ANIM_STATE_UNDEFINED;

    m_ComboCount++;

    // clear flags
    m_bCanRequestCombo = FALSE;
    m_bHitCombo = FALSE;

    SetAnimation( ANIM_STATE_COMBO_BEGIN, ANIM_PRIORITY_DEFAULT );

    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if ( pWeapon )
    {
        pWeapon->ClearZoom();
    }
}

//------------------------------------------------------------------------------
void player::UpdateCombo( const f32& rDeltaTime )
{
    (void)rDeltaTime;

#ifdef ksaffel    
    //x_printfxy(0,0, "UpdateMelee_Special" );
#endif

    // When the currently playing animation ends, need to set another animation
    if( m_AnimPlayer.IsAtEnd() )
    {
        if( !m_bHitCombo || !m_bLastMeleeHit )
        {
            SetAnimState(ANIM_STATE_COMBO_END);
        }
        else
        {
            SetAnimState(ANIM_STATE_COMBO_HIT);
        }
    }
}

//------------------------------------------------------------------------------
void player::EndCombo( void )
{
    // clear combo stuff
    if( !m_bHitCombo || !m_bLastMeleeHit )
    {
        ClearCombo();
    }

    m_PlayMeleeSound = TRUE;

    // clear flag
    m_bHitCombo = FALSE;
}

//------------------------------------------------------------------------------
void player::ClearCombo( void )
{
    m_ComboCount = 0;
    m_bHitCombo = FALSE;
    m_bLastMeleeHit = FALSE;
}

//------------------------------------------------------------------------------

void player::BeginCombo_Hit( void )
{
    m_NextAnimState = ANIM_STATE_UNDEFINED;

    m_ComboCount++;

    // clear flags
    m_bCanRequestCombo = FALSE;
    m_bHitCombo = FALSE;

    SetAnimation( ANIM_STATE_COMBO_HIT, ANIM_PRIORITY_DEFAULT );

    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if ( pWeapon )
    {
        pWeapon->ClearZoom();
    }
}

//------------------------------------------------------------------------------
void player::UpdateCombo_Hit( const f32& rDeltaTime )
{
    (void)rDeltaTime;

#ifdef ksaffel    
    //x_printfxy(0,0, "UpdateMelee_Special" );
#endif

    // When the currently playing animation ends, need to set another animation
    if( m_AnimPlayer.IsAtEnd() )
    {
        SetAnimState(ANIM_STATE_COMBO_END);
    }
}

//------------------------------------------------------------------------------
void player::EndCombo_Hit( void )
{
    // clear combo stuff
    if( !m_bHitCombo || !m_bLastMeleeHit )
    {
        ClearCombo();
    }

    m_PlayMeleeSound = TRUE;
}

//------------------------------------------------------------------------------

void player::BeginCombo_End( void )
{
    m_NextAnimState = ANIM_STATE_UNDEFINED;

    SetAnimation( ANIM_STATE_COMBO_END, ANIM_PRIORITY_DEFAULT );

    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if ( pWeapon )
    {
        pWeapon->ClearZoom();
    }
}

//------------------------------------------------------------------------------
void player::UpdateCombo_End( const f32& rDeltaTime )
{
    (void)rDeltaTime;

#ifdef ksaffel    
    //x_printfxy(0,0, "UpdateMelee_Special" );
#endif

    // When the currently playing animation ends, need to set another animation
    if( m_AnimPlayer.IsAtEnd() )
    {
        // try to reload the weapon (just in case we interrupted it)
        if( !ReloadWeapon(new_weapon::AMMO_PRIMARY) )
        {
            SetAnimState( GetMotionTransitionAnimState() );
        }
    }
}

//------------------------------------------------------------------------------
void player::EndCombo_End( void )
{
    ClearCombo();
}

//==============================================================================
//==============================================================================
//  ANIM_STATE_MELEE_XXX
//==============================================================================
//==============================================================================

void player::BeginMelee_Special( const animation_state& AnimState )
{
    (void)AnimState;
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if ( pWeapon )
    {
        pWeapon->ClearZoom();
    }

    m_bMeleeLunging = TRUE;

#ifdef ksaffel
    //x_printfxy(0,0, "BeginMelee_Special" );
#endif

}

//------------------------------------------------------------------------------

void player::UpdateMelee_Special( const f32& DeltaTime, const animation_state& AnimState )
{
    (void)DeltaTime;
    (void)AnimState;
#ifdef ksaffel    
    //x_printfxy(0,0, "UpdateMelee_Special" );
#endif

    // When the currently playing animation ends, need to set another animation
    if ( m_AnimPlayer.IsAtEnd() )
    {
        SetAnimState( GetMotionTransitionAnimState() );
    }
}

//------------------------------------------------------------------------------

void player::EndMelee_Special( const animation_state& AnimState )
{
    (void)AnimState;
#ifdef ksaffel
    //x_printfxy(0,0, "EndMelee_Special" );
#endif

    // clear FOV effect
    m_ViewInfo.XFOV = m_OriginalViewInfo.XFOV;

    // tell the weapon we are done
    GetMutationMeleeWeapon()->SetMeleeComplete(TRUE);

    m_bMeleeLunging = FALSE;
}

//------------------------------------------------------------------------------

//==============================================================================
//==============================================================================
//  ANIM_STATE_RELOAD
//==============================================================================
//==============================================================================

void player::BeginReload( void )
{
    // lookup the weapon we are using.
    new_weapon* pWeapon = GetCurrentWeaponPtr();

    // Make sure the weapon we are switching to is visible
    if( pWeapon )
    {
        pWeapon->SetVisible(TRUE);

        // started reload, initialize flag to false so that we know the reload hasn't finished
        pWeapon->SetReloadCompleted(FALSE);
    }   

    if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_RELOAD_IN ) && m_NeedRelaodIn )
    {
        SetAnimState( ANIM_STATE_RELOAD_IN );
    }
    else
    {
        m_NextAnimState = ANIM_STATE_UNDEFINED;

        // Set appropriate RELOAD animation
        SetAnimation( ANIM_STATE_RELOAD , ANIM_PRIORITY_DEFAULT );

#ifndef X_EDITOR
        // Play reload anim on 3rd person avatar
        net_Reload();
#endif
    }
}

//------------------------------------------------------------------------------

void player::UpdateReload( const f32& DeltaTime )
{
    // Updating the ammo count triggered by an animation event and handled in
    (void)DeltaTime;

    // don't allow player to switch weapons, zoom in, attack, etc.
    if( m_bHidePlayerArms )
    {
        return;
    }

    //check if we need to go back to a firing state
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( IsFiring() && 
        (pWeapon && pWeapon->IsWeaponReady( pWeapon->GetPrimaryAmmoPriority() )) )
    {
        // if it's the gauss, ramp up
        if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_RAMP_UP ) &&
             m_CurrentWeaponItem != INVEN_WEAPON_MESON_CANNON )
        {
            SetAnimState( ANIM_STATE_RAMP_UP );
        }
        else
        {
            SetAnimState( ANIM_STATE_FIRE );
        }
    }
    else if ( IsAltFiring() &&
        (pWeapon && pWeapon->IsWeaponReady( pWeapon->GetSecondaryAmmoPriority() )) &&
         m_CurrentWeaponItem != INVEN_WEAPON_MESON_CANNON)
    {
        // We have to go through the reload outs.
        if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_ALT_RAMP_UP ) )
        {
            SetAnimState( ANIM_STATE_ALT_RAMP_UP );
        }
        else if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_ALT_FIRE ) )
        {
            SetAnimState( ANIM_STATE_ALT_FIRE );
        }
        else if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_ZOOM_IN ) )
        {
            SetAnimState( ANIM_STATE_ZOOM_IN );
        }
    }

    xbool bReloaded = FALSE;

    if( pWeapon && pWeapon->IsReloadCompleted() && 
        !pWeapon->IsKindOf(weapon_shotgun::GetRTTI()) ) // ignore this for shotgun, it behaves differently 
    {
        // we completed the reload, we can fire without worrying about 
        // interrupting the reload just because the "look good" part of the anim is still playing
        pWeapon->Reload( new_weapon::AMMO_PRIMARY );
        pWeapon->Reload( new_weapon::AMMO_SECONDARY );
        bReloaded = TRUE;

    }

    if( pWeapon && m_AnimPlayer.IsAtEnd() )
    {
        // if we haven't reloaded yet, go for it
        if( !bReloaded )
        {
            pWeapon->Reload( new_weapon::AMMO_PRIMARY );
            pWeapon->Reload( new_weapon::AMMO_SECONDARY );
        }

        if( pWeapon->ContinueReload() )
        {
            //  x_DebugMsg( "Playing Reload Anim\n" );
            SetAnimation( ANIM_STATE_RELOAD , ANIM_PRIORITY_DEFAULT );
        }
        else
        {
            // Set to new animation state.
            if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_RELOAD_OUT ) )
                SetAnimState( ANIM_STATE_RELOAD_OUT );
            else
                SetAnimState( GetMotionTransitionAnimState() );
        }
    }

    UseFocusObject();
}

//------------------------------------------------------------------------------

void player::EndReload( void )
{
    m_NeedRelaodIn = TRUE;
}

//==============================================================================
//==============================================================================
//  ANIM_STATE_RELOAD_IN
//==============================================================================
//==============================================================================

void player::BeginReloadIn( void )
{
    // Set appropriate animation
    SetAnimation( ANIM_STATE_RELOAD_IN , ANIM_PRIORITY_DEFAULT );
}

//------------------------------------------------------------------------------

void player::UpdateReloadIn( const f32& DeltaTime )
{
    (void)DeltaTime;

    if ( m_AnimPlayer.IsAtEnd() )
    {
        SetAnimState( ANIM_STATE_RELOAD );
    }
}

//------------------------------------------------------------------------------

void player::EndReloadIn( void )
{
    m_NeedRelaodIn = FALSE;
}

//==============================================================================
//==============================================================================
//  ANIM_STATE_RELOAD_OUT
//==============================================================================
//==============================================================================

void player::BeginReloadOut( void )
{
    // Set appropriate animation
    SetAnimation( ANIM_STATE_RELOAD_OUT , ANIM_PRIORITY_DEFAULT );
}

//------------------------------------------------------------------------------

void player::UpdateReloadOut( const f32& DeltaTime )
{
    (void)DeltaTime;

    if ( m_AnimPlayer.IsAtEnd() )
    {
        SetAnimState( GetMotionTransitionAnimState() );
    }
}

//------------------------------------------------------------------------------

void player::EndReloadOut( void )
{

}

//==============================================================================
//==============================================================================
//  ANIM_STATE_RAMP_UP
//==============================================================================
//==============================================================================

void player::BeginRampUp( void )
{
    m_NextAnimState = ANIM_STATE_UNDEFINED;

    SetAnimation( ANIM_STATE_RAMP_UP , ANIM_PRIORITY_DEFAULT );
}

//------------------------------------------------------------------------------

void player::UpdateRampUp( const f32& DeltaTime )
{
    (void)DeltaTime;

    // If the 'fire' button is being pressed, we go to ANIM_STATE_FIRE,
    // otherwise, we to to AMIM_STATE_RAMP_DOWN
    if ( IsFiring() )
    {        
        if ( m_AnimPlayer.IsAtEnd() )
        {
            if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_HOLD ) )
            {
                // Switch to hold anim
                SetAnimState( ANIM_STATE_HOLD );
            }
            else
            {
                SetAnimState( ANIM_STATE_FIRE );
            }
        }
    }
    else
    {

        // Get the hand and the weapon parametric values.
        f32 RampPercent = m_AnimPlayer.GetFrameParametric();
        f32 WeaponRampPer = 1.0f;
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if ( pWeapon )
            WeaponRampPer = pWeapon->GetFrameParametric();

        SetAnimState( ANIM_STATE_RAMP_DOWN );
        x_DebugMsg( "Ramp Down percent [%f] Weapon [%f]\n", RampPercent, WeaponRampPer );

        // Set the weapons and the guns start frame.
        m_AnimPlayer.SetFrameParametric( (1.0f-RampPercent) );
        if ( pWeapon )
            pWeapon->SetFrameParametric( (1.0f-WeaponRampPer) );
    }
}

//------------------------------------------------------------------------------

void player::EndRampUp( void )
{
}

//==============================================================================
//==============================================================================
//  ANIM_STATE_RAMP_DOWN
//==============================================================================
//==============================================================================

void player::BeginRampDown( void )
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if ( pWeapon )
    {
        pWeapon->SetVisible(TRUE);
    }   

    m_NextAnimState = ANIM_STATE_UNDEFINED;
    SetAnimation( ANIM_STATE_RAMP_DOWN , ANIM_PRIORITY_DEFAULT );
}

//------------------------------------------------------------------------------

void player::UpdateRampDown( const f32& DeltaTime )
{
    (void)DeltaTime;

    new_weapon* pWeapon = GetCurrentWeaponPtr();

    if( m_AnimPlayer.IsAtEnd() )
    {
        xbool bUsedFocusObject = UseFocusObject();
        xbool bPressedReload   = !bUsedFocusObject && (g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_RELOAD ).WasValue > 0.0f); 

        if( bPressedReload && !NearMutagenReservoir() )
        {
            // If we can reload, do so.
            // Otherwise, play the idle animation.
            if ( pWeapon && pWeapon->CanReload( new_weapon::AMMO_PRIMARY ) )
            {
                m_NextAnimState = ANIM_STATE_RELOAD;
            }
            else
            {
                m_NextAnimState = GetMotionTransitionAnimState();
            }
        }
        else if( pWeapon && IsFiring() )
        {
            if( pWeapon->GetAmmoCount( pWeapon->GetPrimaryAmmoPriority() ) <= 0 )
            {
                // If we can reload, do so.
                // Otherwise, play the idle animation.
                if ( pWeapon->CanReload( new_weapon::AMMO_PRIMARY ) )
                {
                    m_NextAnimState = ANIM_STATE_RELOAD;
                }
                else
                {
                    m_NextAnimState = GetMotionTransitionAnimState();
                }
            }
            else
            {
                // Do we need to handle any ramping?
                if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_RAMP_UP ) )
                {
                    m_NextAnimState = ANIM_STATE_RAMP_UP;
                }
                else
                {
                    m_NextAnimState = ANIM_STATE_FIRE;
                }
            }
        }
        else
        {
            m_NextAnimState = GetMotionTransitionAnimState();
        }

        SetAnimState( m_NextAnimState );
    }
}

//------------------------------------------------------------------------------

void player::EndRampDown( void )
{
}

//==============================================================================
//==============================================================================
//  ANIM_STATE_ALT_RAMP_UP
//==============================================================================
//==============================================================================

void player::BeginAltRampUp( void )
{
    m_NextAnimState = ANIM_STATE_UNDEFINED;
    SetAnimation( ANIM_STATE_ALT_RAMP_UP , ANIM_PRIORITY_DEFAULT );

    new_weapon* pWeapon = GetCurrentWeaponPtr();

    // tell the weapon we are ramping up (currently only for meson cannon).
    if( pWeapon )
    {
        pWeapon->BeginAltRampUp();
    }
}

//------------------------------------------------------------------------------

void player::UpdateAltRampUp( const f32& DeltaTime )
{
    (void)DeltaTime;

    // If the 'fire' button is being pressed, we go to ANIM_STATE_FIRE,
    // otherwise, we to to AMIM_STATE_RAMP_DOWN
    if ( IsAltFiring() )
    {
        if ( m_AnimPlayer.IsAtEnd() )
        {
            if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_ALT_HOLD ) )
            {
                // Switch to hold anim
                SetAnimState( ANIM_STATE_ALT_HOLD );
            }
            else
            {
                SetAnimState( ANIM_STATE_ALT_FIRE);
            }
        }
    }
    else
    {
        SetAnimState(ANIM_STATE_ALT_FIRE);
        /* KSS -- Aaron said we weren't using this and it was affecting how the meson cannon animations work, so, taking it out for now.
        // Get the hand and the weapon parametric values.
        f32 RampPercent = m_AnimPlayer.GetFrameParametric();
        f32 WeaponRampPer = 1.0f;
        object* pObject = g_ObjMgr.GetObjectByGuid( m_GuidWeaponArray[m_CurrentWeaponObj] );
        if ( pObject )
        WeaponRampPer = ((new_weapon*)pObject )->GetFrameParametric();

        SetAnimState( ANIM_STATE_ALT_RAMP_DOWN );
        x_DebugMsg( "Alt Ramp Down percent [%f] Weapon [%f]\n", RampPercent, WeaponRampPer );

        // Set the weapons and the guns start frame.
        m_AnimPlayer.SetFrameParametric( (1.0f-RampPercent) );
        if ( pObject )
        ((new_weapon*)pObject )->SetFrameParametric( (1.0f-WeaponRampPer) );
        */
    }
}

//------------------------------------------------------------------------------

void player::EndAltRampUp( void )
{
    xbool bGoingIntoHold   = (m_NextAnimState == ANIM_STATE_ALT_HOLD);
    xbool bSwitchingWeapon = (m_NextAnimState == ANIM_STATE_SWITCH_FROM);

    new_weapon* pWeapon = GetCurrentWeaponPtr();

    // tell the weapon we are finished ramping up (currently only for meson cannon).
    if( pWeapon )
    {
        pWeapon->EndAltRampUp(bGoingIntoHold, bSwitchingWeapon);
    }
}

//==============================================================================
//==============================================================================
//  ANIM_STATE_ALT_RAMP_DOWN
//==============================================================================
//==============================================================================

void player::BeginAltRampDown( void )
{
    f32 fBlendTime;
    if ( m_PreviousAnimState == ANIM_STATE_ALT_RAMP_UP )
    {
        fBlendTime = DEFAULT_BLEND_TIME;
    }
    else
    {
        fBlendTime = 0.f;
    }

    m_NextAnimState = ANIM_STATE_UNDEFINED;
    SetAnimation( ANIM_STATE_ALT_RAMP_DOWN, ANIM_PRIORITY_DEFAULT, fBlendTime );
}

//------------------------------------------------------------------------------

void player::UpdateAltRampDown( const f32& DeltaTime )
{
    ( void ) DeltaTime;

    new_weapon* pWeapon = GetCurrentWeaponPtr();

    if( m_AnimPlayer.IsAtEnd() )
    {
        xbool bUsedFocusObject = UseFocusObject();
        xbool bPressedReload   = !bUsedFocusObject && (g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_RELOAD ).WasValue > 0.0f); 

        if( bPressedReload && !NearMutagenReservoir() )
        {
            // Time to reload or go to fire empty.
            // If the current weapon needs to be reloaded, set the next state to reload
            // We are out of ammo, check if we have a clip left.  If not then go to idle.
            if( pWeapon && pWeapon->CanReload( new_weapon::AMMO_SECONDARY ) )
            {
                m_NextAnimState = ANIM_STATE_RELOAD;
            }
            else
            {
                m_NextAnimState = GetMotionTransitionAnimState();
            }
        }
        else if( IsAltFiring() )
        {
            if( pWeapon && (pWeapon->GetAmmoCount( pWeapon->GetSecondaryAmmoPriority() ) <= 0) )
            {
                // We are out of ammo, check if we have a clip left.  If not then go to idle.
                if( pWeapon->CanReload( new_weapon::AMMO_SECONDARY ) )
                {
                    m_NextAnimState = ANIM_STATE_RELOAD;
                }
                else
                {
                    m_NextAnimState = GetMotionTransitionAnimState();
                }
            }
            else
            {
                // Do we need to handle any ramping?
                if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_ALT_RAMP_UP ) )
                {
                    m_NextAnimState = ANIM_STATE_ALT_RAMP_UP;
                }
                else
                {
                    m_NextAnimState = ANIM_STATE_ALT_FIRE;
                }
            }

        }
        else
        {
            m_NextAnimState = GetMotionTransitionAnimState();
        }

        SetAnimState( m_NextAnimState );
    }
}

//------------------------------------------------------------------------------

void player::EndAltRampDown( void )
{
}

//==============================================================================
//==============================================================================
//  ANIM_STATE_HOLD
//==============================================================================
//==============================================================================

void player::BeginHold( void )
{
    m_AnimStage = 1;
    m_WpnHoldTime = 0.0f;

    m_NextAnimState = ANIM_STATE_UNDEFINED;
    SetAnimation( ANIM_STATE_HOLD , ANIM_PRIORITY_DEFAULT );
}

//------------------------------------------------------------------------------

void player::UpdateHold( const f32& DeltaTime )
{
    m_WpnHoldTime += DeltaTime;

    if (m_AnimStage == 1)
    {
        // Isthe player still holding the fire button?
        xbool bFiring = IsFiring();

        if ( !bFiring )
        {
            m_AnimStage++;
            SetAnimState( ANIM_STATE_FIRE );
        }
        else
        {
            // Wait until the current animtion ends, then switch the animation
            if( m_AnimPlayer.IsAtEnd() )
            {
                state_anims& State  = m_Anim[inventory2::ItemToWeaponIndex(m_CurrentWeaponItem)][ANIM_STATE_HOLD];
                s32 nAnimIndex      = ANIM_PRIORITY_DEFAULT;

                // If we have multiple hold animation, increase the anim index as time increases.
                if ( m_AnimPlayer.GetAnimIndex() == State.PlayerAnim[ANIM_PRIORITY_DEFAULT] && State.nPlayerAnims > 1 )
                {
                    nAnimIndex = (s32)((m_WpnHoldTime/m_MaxAnimWeaponHoldTime) * (f32)(State.nPlayerAnims - 1));
                }

                SetAnimation( ANIM_STATE_HOLD, nAnimIndex );
            }
        }
    }
    else
    {
        if ( m_AnimPlayer.IsAtEnd() )
        {
            SetAnimState( ANIM_STATE_RAMP_DOWN );
        }
    }
}

//------------------------------------------------------------------------------

void player::EndHold( void )
{
    m_WpnHoldTime = 0.0f;
}

//==============================================================================
//==============================================================================
//  ANIM_STATE_ALT_HOLD
//==============================================================================
//==============================================================================

void player::BeginAltHold( void )
{
    m_AnimStage = 1;
    m_WpnHoldTime = 0.0f;

    m_NextAnimState = ANIM_STATE_UNDEFINED;
    SetAnimation( ANIM_STATE_ALT_HOLD, ANIM_PRIORITY_DEFAULT );
}

//------------------------------------------------------------------------------

void player::UpdateAltHold( const f32& DeltaTime )
{
    m_WpnHoldTime += DeltaTime;

    if (m_AnimStage == 1)
    {
        // Is the player still holding the fire button?
        xbool bFiring = IsAltFiring();

        if ( !bFiring )
        {
            SetAnimState( ANIM_STATE_ALT_FIRE );
        }
        else
        {
            // Wait until the current animtion ends, then switch the animation
            if( m_AnimPlayer.IsAtEnd() )
            {
                state_anims& State  = m_Anim[inventory2::ItemToWeaponIndex(m_CurrentWeaponItem)][ANIM_STATE_ALT_HOLD];
                s32 nAnimIndex      = ANIM_PRIORITY_DEFAULT;

                // If we have multiple hold animation, increase the anim index as time increases.
                if ( m_AnimPlayer.GetAnimIndex() == State.PlayerAnim[ANIM_PRIORITY_DEFAULT] && State.nPlayerAnims > 1 )
                {
                    nAnimIndex = (s32)((m_WpnHoldTime/m_MaxAnimWeaponHoldTime) * (f32)(State.nPlayerAnims - 1));
                }

                SetAnimation( ANIM_STATE_ALT_HOLD, nAnimIndex );
            }
        }
    }
    else
    {
        if ( m_AnimPlayer.IsAtEnd() )
        {
            SetAnimState( ANIM_STATE_ALT_RAMP_DOWN );
        }
    }
}

//------------------------------------------------------------------------------

void player::EndAltHold( void )
{
    m_WpnHoldTime = 0.0f;
    new_weapon* pWeapon = GetCurrentWeaponPtr();

    // tell the weapon we are ending the hold (currently only for meson cannon).
    if( pWeapon )
    {
        xbool bSwitchingWeapon = (m_NextAnimState == ANIM_STATE_SWITCH_FROM);

        if( !bSwitchingWeapon )
        {
            BeginAltFire();
        }

        pWeapon->EndAltHold( bSwitchingWeapon );
    }
}

//==============================================================================
//==============================================================================
//  ANIM_STATE_FALLING_TO_DEATH
//==============================================================================
//==============================================================================
void player::BeginFallingToDeath( void )
{
    m_NextAnimState = ANIM_STATE_UNDEFINED;
    SetupThirdPersonCamera();

    if ( m_pLoco && UsingLoco() )
    {
        m_pLoco->SetState( loco::STATE_IDLE );
        m_pLoco->PlayDeathAnim( loco::ANIM_DEATH_EXPLOSION );
    }
}

//------------------------------------------------------------------------------

void player::UpdateFallingToDeath( f32 DeltaTime )
{
    (void)DeltaTime;

    if ( GetThirdPersonCamera() )
    {
        UpdateThirdPersonCamera();
        GetThirdPersonCamera()->MoveTowardsPitch( -m_Physics.GetVelocity().GetPitch() );
    }
}

//------------------------------------------------------------------------------

void player::EndFallingToDeath( void )
{
}

//==============================================================================
//==============================================================================
//  ANIM_STATE_ZOOM_IN
//==============================================================================
//==============================================================================

void player::BeginZoomIn( void )
{
    m_NextAnimState = ANIM_STATE_UNDEFINED;
    // Play the zoom in animation for this state
    SetAnimation( ANIM_STATE_ZOOM_IN , ANIM_PRIORITY_DEFAULT );

    new_weapon* pWeapon = GetCurrentWeaponPtr();

    if( pWeapon )
    {
        pWeapon->IncrementZoom();
        pWeapon->ZoomInComplete(FALSE);
    }
}

//------------------------------------------------------------------------------

void player::UpdateZoomIn( const f32& DeltaTime )
{
    (void)DeltaTime;

    // Wait until the current animtion ends, then switch the animation
    if( m_AnimPlayer.IsAtEnd() )
    {
        f32     VelocitySquared = m_Physics.GetVelocity().LengthSquared();
        if( VelocitySquared < m_fMinRunSpeed )
            SetAnimState( ANIM_STATE_ZOOM_IDLE );
        else
            SetAnimState( ANIM_STATE_ZOOM_RUN );
    }
}

//------------------------------------------------------------------------------

void player::EndZoomIn( void )
{   
    LoadAimAssistTweakHandles();
    new_weapon* pWeapon = GetCurrentWeaponPtr();

    if( pWeapon )
    {
        pWeapon->ZoomInComplete(TRUE);
    }
}

//------------------------------------------------------------------------------

void player::EndZoomState( void )
{
    SetAnimState(ANIM_STATE_ZOOM_OUT);
    m_NextAnimState = ANIM_STATE_IDLE;
}

//==============================================================================
//==============================================================================
//  ANIM_STATE_ZOOM_OUT
//==============================================================================
//==============================================================================

void player::BeginZoomOut( void )
{
    ASSERT( m_NextAnimState != ANIM_STATE_UNDEFINED );

    // Play the zoom in animation for this state
    SetAnimation( ANIM_STATE_ZOOM_OUT , ANIM_PRIORITY_DEFAULT );

    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if ( pWeapon )
    {
        while ( pWeapon->IncrementZoom() > 0 )
            ;
    }
}

//------------------------------------------------------------------------------

void player::UpdateZoomOut( const f32& DeltaTime )
{
    (void)DeltaTime;
    ASSERT( m_NextAnimState != ANIM_STATE_UNDEFINED );

    // Wait until the current animtion ends, then switch the animation
    if( m_AnimPlayer.IsAtEnd() )
    {
        SetAnimState( m_NextAnimState );    
    }
}

//------------------------------------------------------------------------------

void player::EndZoomOut( void )
{
    LoadAimAssistTweakHandles();
}

//==============================================================================
//==============================================================================
//  ANIM_STATE_ZOOM_IDLE
//==============================================================================
//==============================================================================

void player::BeginZoomIdle( void )
{
    // Play the zoom in animation for this state
    SetAnimation( ANIM_STATE_ZOOM_IDLE , ANIM_PRIORITY_DEFAULT );

    // this is a zoom idle, but, we're still idling
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon && !pWeapon->GetIdleMode() )
        pWeapon->BeginIdle();
}

//------------------------------------------------------------------------------

void player::UpdateZoomIdle( const f32& DeltaTime )
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();

    // Check if we need to go back to a firing state
    if( IsFiring() )
    {
        if( pWeapon && (pWeapon->GetAmmoCount( pWeapon->GetPrimaryAmmoPriority() ) > 0) )
        {
            SetAnimState( ANIM_STATE_ZOOM_FIRE );
        }
        else
        {
            if( pWeapon && pWeapon->CanReload( pWeapon->GetPrimaryAmmoPriority() ) )
            {
                SetAnimState( ANIM_STATE_ZOOM_OUT );
                m_NextAnimState = ANIM_STATE_RELOAD;
            }
            else
            {
                g_AudioMgr.Play( "NoAmmo" );
            }
        }
    }
    else if ( IsAltFiring() && (m_DebounceTime > s_DebounceDuration) )
    {
        // If we're at our max zoom, then zoom out, otherwise zoom some more
        if ( pWeapon->GetZoomStep() == pWeapon->GetnZoomSteps() )
        {
            SetAnimState( ANIM_STATE_ZOOM_OUT );
            m_NextAnimState = ANIM_STATE_IDLE;
        }
        else
        {
            if ( pWeapon )
            {
                pWeapon->IncrementZoom();
                m_DebounceTime = 0.0f;

                // zoomed, update tweaks
                LoadAimAssistTweakHandles();
            }
        }
    }

    xbool bUsedFocusObject = UseFocusObject();
    xbool bPressedReload   = !bUsedFocusObject && (g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_RELOAD ).WasValue > 0.0f); 

    // Reloading?
    if( bPressedReload && !NearMutagenReservoir() )
    {
        // Time to reload or go to fire empty.
        // If the current weapon needs to be reloaded, set the next state to reload
        if( pWeapon 
            && (   (pWeapon->CanReload( pWeapon->GetSecondaryAmmoPriority() )) 
            || (pWeapon->CanReload( pWeapon->GetPrimaryAmmoPriority()   ))) )
        {
            SetAnimState( ANIM_STATE_ZOOM_OUT );
            m_NextAnimState = ANIM_STATE_RELOAD;
        }
    }

    // Check if we need to change to another motion animation.
    if ( m_Physics.GetVelocity().LengthSquared() > m_fMinRunSpeed )
    {
        SetAnimState( ANIM_STATE_ZOOM_RUN );
        return;
    }

    // Watch the animation timers and player's motion.  Switch the animation if necessary.
    m_fAnimationTime += DeltaTime;

    if ( m_fAnimationTime > m_fMaxAnimTime )
    {
        // Wait until the current animtion ends, then switch the animation
        if( m_AnimPlayer.IsAtEnd() )
        {
            SetAnimation( ANIM_STATE_ZOOM_IDLE , ANIM_PRIORITY_DEFAULT );
        }
    }

    ReloadWeapon(new_weapon::AMMO_PRIMARY);
}

//------------------------------------------------------------------------------

void player::EndZoomIdle( void )
{
    // this is a zoom idle, but, we're still idling
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( (m_NextAnimState != ANIM_STATE_RUN) && pWeapon )
        pWeapon->EndIdle();
}

//==============================================================================
//==============================================================================
//  ANIM_STATE_ZOOM_RUN
//==============================================================================
//==============================================================================

void player::BeginZoomRun( void )
{
    // Play the zoom in animation for this state
    SetAnimation( ANIM_STATE_ZOOM_RUN , ANIM_PRIORITY_DEFAULT );

    // this is a zoom idle, but, we're still idling
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon && !pWeapon->GetIdleMode() )
        pWeapon->BeginIdle();
}

//------------------------------------------------------------------------------

void player::UpdateZoomRun( const f32& DeltaTime )
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();

    // Check if we need to go back to a firing state
    if( IsFiring() )
    {
        if( pWeapon && (pWeapon->GetAmmoCount( pWeapon->GetPrimaryAmmoPriority() ) > 0) )
        {
            SetAnimState( ANIM_STATE_ZOOM_FIRE );
        }
        else
        {
            if( pWeapon && pWeapon->CanReload( pWeapon->GetPrimaryAmmoPriority() ) )
            {
                SetAnimState( ANIM_STATE_ZOOM_OUT );
                m_NextAnimState = ANIM_STATE_RELOAD;
            }
            else
            {
                g_AudioMgr.Play( "NoAmmo" );
            }
        }
    }
    else if ( IsAltFiring() && (m_DebounceTime > s_DebounceDuration) )
    {
        // If we're at our max zoom, then zoom out, otherwise zoom some more
        if ( pWeapon->GetZoomStep() == pWeapon->GetnZoomSteps() )
        {
            SetAnimState( ANIM_STATE_ZOOM_OUT );
            m_NextAnimState = ANIM_STATE_IDLE;
        }
        else
        {
            if ( pWeapon )
            {
                pWeapon->IncrementZoom();
                m_DebounceTime = 0.0f;

                // zoomed, update tweaks
                LoadAimAssistTweakHandles();
            }
        }
    }

    xbool bUsedFocusObject = UseFocusObject();
    xbool bPressedReload   = !bUsedFocusObject && (g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_RELOAD ).WasValue > 0.0f); 

    // Reloading?
    if( bPressedReload && !NearMutagenReservoir() )
    {
        // Time to reload or go to fire empty.
        // If the current weapon needs to be reloaded, set the next state to reload
        if( pWeapon 
            && (   (pWeapon->CanReload( pWeapon->GetSecondaryAmmoPriority() )) 
            || (pWeapon->CanReload( pWeapon->GetPrimaryAmmoPriority()   ))) )
        {
            SetAnimState( ANIM_STATE_ZOOM_OUT );
            m_NextAnimState = ANIM_STATE_RELOAD;
        }
    }

    // Check if we need to change to another motion animation.
    if ( m_Physics.GetVelocity().LengthSquared() < m_fMinRunSpeed )
    {
        SetAnimState( ANIM_STATE_ZOOM_IDLE );
        return;
    }

    // Watch the animation timers and player's motion.  Switch the animation if necessary.
    m_fAnimationTime += DeltaTime;

    if ( m_fAnimationTime > m_fMaxAnimTime )
    {
        // Wait until the current animtion ends, then switch the animation
        if( m_AnimPlayer.IsAtEnd() )
        {
            SetAnimation( ANIM_STATE_ZOOM_RUN , ANIM_PRIORITY_DEFAULT );
        }
    }

    ReloadWeapon(new_weapon::AMMO_PRIMARY);
}

//------------------------------------------------------------------------------

void player::EndZoomRun( void )
{
    // this is a zoom idle, but, we're still idling
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( (m_NextAnimState != ANIM_STATE_RUN) && pWeapon )
        pWeapon->EndIdle();
}

//==============================================================================
//==============================================================================
//  ANIM_STATE_ZOOM_FIRE
//==============================================================================
//==============================================================================

void player::BeginZoomFire( void )
{
    // Generate the percentages used to determine what firing animations are being played.
    GenerateFiringAnimPercentages();

    // we can't tap fire again until release
    m_bCanTapFire = FALSE;

    // reset timer
    m_TapRefireTime = 0.0f;

    new_weapon* pWeapon = GetCurrentWeaponPtr();

    if ( pWeapon )
    {
        pWeapon->BeginPrimaryFire();   

#ifndef X_EDITOR        
        
        // SMP has looping anims, so handle that separately
        if( ( pWeapon->GetType() != TYPE_WEAPON_SMP ) && ( pWeapon->GetType() != TYPE_WEAPON_DUAL_SMP ) )
        {
            // Play fire on 3rd person avatar
            net_FirePrimary();        
        }
        else
        {
            // Begin looping fire on 3rd person avatar
            net_BeginFirePrimary();
        }
#endif
        
    }

    m_NextAnimState = ANIM_STATE_UNDEFINED;
    SetAnimation( ANIM_STATE_ZOOM_FIRE, ANIM_PRIORITY_DEFAULT, 0.0f );
}

//------------------------------------------------------------------------------

void player::UpdateZoomFire( const f32& DeltaTime )
{
    (void)DeltaTime;

    m_TapRefireTime += DeltaTime;

    // If the current weapon needs to be reloaded, set the next state to reload
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon && (pWeapon->GetAmmoCount( pWeapon->GetPrimaryAmmoPriority() ) <= 0) )
    {
        if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_DISCARD ) )
        {
            // Discard
            SetAnimState( ANIM_STATE_DISCARD );
            m_NextAnimState = ANIM_STATE_DISCARD;
        }
        else
        {
            m_NextAnimState =  GetMotionTransitionAnimState();
        }
    }

    // Is the player still holding the fire button?
    xbool bFiring = IsFiring();

    // Do we need to reload?  If the weapon has a ramp down play that first.
    // If we run out of ammo and can't reload go to the Idle state.
    if( m_NextAnimState == ANIM_STATE_RELOAD )
    {
        if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_ALT_RAMP_DOWN ) )
        {
            SetAnimState( ANIM_STATE_RAMP_DOWN );
        }
        else
        {
            SetAnimState( m_NextAnimState );
        }
        return;
    }
    else if( (m_NextAnimState == ANIM_STATE_IDLE) || (m_NextAnimState == ANIM_STATE_RUN) )
    {
        if( IsAnimStateAvailable2( m_CurrentWeaponItem, ANIM_STATE_RAMP_DOWN ) )
        {
            SetAnimState( ANIM_STATE_RAMP_DOWN );
        }
        else
        {
            SetAnimState( m_NextAnimState );
        }

        // see if we need to reload
        ReloadWeapon(new_weapon::AMMO_PRIMARY);

        // Special case:  We are zoomed in and out of ammo.
        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if( pWeapon )
        {
            // Clear zoom ourselves because no reload is going to occur.
            if( ( pWeapon->GetAmmoAmount( new_weapon::AMMO_PRIMARY ) == 0 ) )
            {
                pWeapon->ClearZoom();
            }
        }

        return;
    }

    // When the currently playing animation ends, need to set another animation
    if( bFiring )
    {
        // Still firing away.
        if( m_AnimPlayer.IsAtEnd() || 
            (pWeapon->CanFastTapFire() && m_bCanTapFire && (m_TapRefireTime >= TapRefireRefreshSeconds_Tweak.GetF32())) )
        {
            state_anims& WeaponState = m_Anim[inventory2::ItemToWeaponIndex(m_CurrentWeaponItem)][ANIM_STATE_ZOOM_FIRE];
            if( WeaponState.nWeaponAnims > 1 )
            {
                if( m_LastFireAnimStateIndex >= WeaponState.nWeaponAnims )
                    m_LastFireAnimStateIndex = 0;

                SetAnimation( ANIM_STATE_ZOOM_FIRE, m_LastFireAnimStateIndex, 0.0f );
                m_LastFireAnimStateIndex++;
            }
            else
            {
                SetAnimation( ANIM_STATE_ZOOM_FIRE, ANIM_PRIORITY_DEFAULT, 0.0f );
            }

            // we can't tap fire again until release
            m_bCanTapFire = FALSE;

            // reset timer
            m_TapRefireTime = 0.0f;
        }        
    }
    else
    {
        // we've relased the button, signal a tap fire.
        m_bCanTapFire = TRUE; 
    
        if( pWeapon && pWeapon->CanIntereptPrimaryFire( m_CurrentAnimIndex ) )
        {
            f32     VelocitySquared = m_Physics.GetVelocity().LengthSquared();
            if( VelocitySquared < m_fMinRunSpeed )
                SetAnimState( ANIM_STATE_ZOOM_IDLE );
            else
                SetAnimState( ANIM_STATE_ZOOM_RUN );
        }
    }
}

//------------------------------------------------------------------------------

void player::EndZoomFire( void )
{
    new_weapon* pWeapon = GetCurrentWeaponPtr();
    if( pWeapon )
    {
        pWeapon->EndPrimaryFire();
        
#ifndef X_EDITOR
        // Stop looping fire for SMP
        if( ( pWeapon->GetType() == TYPE_WEAPON_SMP ) || ( pWeapon->GetType() == TYPE_WEAPON_DUAL_SMP ) )
        {
            net_EndFirePrimary();
        }
#endif

    }        
}

//------------------------------------------------------------------------------

void player::SetAnimState( animation_state AnimState )
{
    // don't set the state if we're already in it
    if ( m_CurrentAnimState == AnimState )
        return;

#ifdef mreed 
    if ( m_bIsMutated )
        LOG_MESSAGE( "Mutation", "SetAnimState( %i )", AnimState );
#endif

    if ( m_bDead && (AnimState != ANIM_STATE_MISSION_FAILED) )
    {
#ifdef mreed 
        if ( AnimState != ANIM_STATE_DEATH )
        {
            if ( m_bIsMutated )
                LOG_MESSAGE( "Mutation", "SetAnimState( %i ), overriding w/ANIM_STATE_DEATH", AnimState );
        }
#endif        
        AnimState = ANIM_STATE_DEATH;
    }

    if (   (m_CurrentWeaponItem == INVEN_WEAPON_MUTATION    ) 
        && (m_CurrentAnimState  == ANIM_STATE_SWITCH_TO     )
        && (AnimState           != ANIM_STATE_SWITCH_FROM   )
        && (m_Health.GetHealth() > 0.0f)
        && (!m_bIsMutated       || !m_bIsMutantVisionOn     ) )
    {
        // This is a catch-all for the rare instances
        // when the change to mutation is inturrupted
        // by some other code. When this happens, we
        // need to force the rest of the mutation 
        // change.
        m_bIsMutated        = TRUE;
        m_bIsMutantVisionOn = TRUE;
        
        // Send over net
        SetMutated( TRUE );
    }

    m_NextAnimState = AnimState;

    //now end the state that we're in.
    EndState();

    //Set new state
    m_PreviousAnimState = m_CurrentAnimState;
    m_CurrentAnimState = AnimState;

    BeginState();
}

//==============================================================================


//==============================================================================
//==============================================================================
//  ANIM_STATE_CINEMA
//==============================================================================
//==============================================================================
void player::BeginCinema( void )
{
    // KSS -- new cinema code
    {
        // clear out correction delta
        m_Cinema.m_ViewCorrectionDelta.Zero();        
    }

    m_NextAnimState = ANIM_STATE_UNDEFINED;

    hud_object* Hud = GetHud();

    if( Hud )
    {
        Hud->SetupLetterBox( TRUE, 0.8f );
    }

    // Get gun off screen
    //SetAnimation( ANIM_STATE_SWITCH_FROM , ANIM_PRIORITY_DEFAULT , 0.f );

    // Setup current look direction
    m_Cinema.m_CurrentLookDir.Set(0,0,1);
    m_Cinema.m_CurrentLookDir.RotateX( GetPitch() );
    m_Cinema.m_CurrentLookDir.RotateY( GetYaw() );

    m_Cinema.m_CurrentBlendInTime = 0.0f;

    m_Physics.SetSolveActorCollisions( FALSE );
    m_Physics.SetLocoCollisionOn( FALSE );
    m_Physics.SetLocoGravityOn(FALSE) ;
    // Kick the weapon into idle
    {
        //Set the animation timers and 
        m_fAnimationTime = 0.f;
        m_fMaxAnimTime = x_frand( s_idle_timeout_min , s_ilde_timout_max );

        m_NextAnimState = ANIM_STATE_UNDEFINED;

        new_weapon* pWeapon = GetCurrentWeaponPtr();
        if( pWeapon && !pWeapon->GetIdleMode() )
        {
            // begin idle, but, tell the weapon this isn't a "normal" idle (i.e. a cinematic)
            pWeapon->BeginIdle(FALSE);
            pWeapon->ClearZoom();
        }

        // Play the default idle animation for this state
        SetAnimation( ANIM_STATE_IDLE , ANIM_PRIORITY_DEFAULT );
    }
    // set our zone to the camera thinghy.
    object* pCamera = g_ObjMgr.GetObjectByGuid( m_Cinema.m_CinemaCameraGuid );
    if( pCamera )
    {
        SetZone1(pCamera->GetZone1());
        SetZone2(pCamera->GetZone2());
        InitZoneTracking();
    }
}

//------------------------------------------------------------------------------

// This is roughly the offset height from the players feet to his eye
vector3 g_PlayerViewOffset( 0.0f, -198.6f, 0.0f );

void player::UpdateCinema( f32 DeltaTime )
{
    //
    // Note: we may want to add acceleration into this.
    //

    if( m_Cinema.m_bCinemaOn==FALSE )
    {
        // Make sure zone flag is cleared ready for next cinema
        m_Cinema.m_bPlayerZoneInitialized = FALSE;

        // Goto idle
        SetAnimState( ANIM_STATE_IDLE );       
        return;
    }

    // KSS -- new cinema code
    
    // Get the camera position and yaw and keep player near
    object* pObject = g_ObjMgr.GetObjectByGuid( m_Cinema.m_CinemaCameraGuid );
    if( pObject )
    {
        // SB: Keep camera info for "player::ComputeView"
        m_Cinema.m_CameraV2W  = pObject->GetL2W();
        m_Cinema.m_CameraXFOV = m_ViewInfo.XFOV;
        
        // SB: If this is a real camera object, then get the field of view also
        object_ptr<camera> pCamera = g_ObjMgr.GetObjectByGuid( m_Cinema.m_CinemaCameraGuid );
        if( pCamera )
        {
            m_Cinema.m_CameraXFOV = pCamera->GetView().GetXFOV();
        }
        
        // Get info from camera V2W
        vector3 CameraPos = m_Cinema.m_CameraV2W.GetTranslation();
        radian3 CameraRot = m_Cinema.m_CameraV2W.GetRotation();
        
        // Setup player rotation from camera
        SetYaw( CameraRot.Yaw );
        SetPitch( CameraRot.Pitch );

        matrix4 L2W;
        L2W.Identity();
        L2W.RotateY( CameraRot.Yaw );
        L2W.Translate( CameraPos + g_PlayerViewOffset );

        OnTransform( L2W );
        m_Yaw = L2W.GetRotation().Yaw;
        m_AnimPlayer.SetYaw( m_fCurrentYawOffset + m_Yaw );
        OnMoveViewPosition( L2W.GetTranslation() );
        
        // Update the eyes so that zone tracking works
        m_EyesPosition  = CameraPos;
        
        // Teleport player's zone to camera zone?
        if( !m_Cinema.m_bPlayerZoneInitialized )
        {
            // Initialize all zone related members to be where the camera is
            m_ZoneTracker.SetMainZone( (u8)pObject->GetZone1() );
            m_ZoneTracker.SetZone2   ( (u8)pObject->GetZone2() );
            m_ZoneTracker.SetPosition( pObject->GetPosition() );
            SetZone1( (u8)pObject->GetZone1() );
            SetZone2( (u8)pObject->GetZone2() );
            
            // Flag as initialized
            m_Cinema.m_bPlayerZoneInitialized = TRUE;
        }

        // Update zone tracking to follow the camera        
        UpdateZoneTrack();
    }

    // Make camera look at a specific object?
    {
        // Get position of target object to look at
        object* pObj = g_ObjMgr.GetObjectByGuid( m_Cinema.m_LookAtTargetGuid );
        if( pObj )
        {
            // Setup current look direction
            m_Cinema.m_CurrentLookDir.Set(0,0,1);
            m_Cinema.m_CurrentLookDir.RotateX( GetPitch() );
            m_Cinema.m_CurrentLookDir.RotateY( GetYaw() );

            // Compute a new desired look direction
            vector3 TargetPos = pObj->GetPosition();
            m_Cinema.m_DesiredLookDir = TargetPos - GetEyesPosition();
            m_Cinema.m_DesiredLookDir.Normalize();

            // Interpolate current direction toward desired direction

            // Get angle difference between the vectors
            radian Angle = v3_AngleBetween( m_Cinema.m_CurrentLookDir, m_Cinema.m_DesiredLookDir );
            ASSERT( Angle >= 0 );

            // Compute axis to rotate around.
            // If we are 180 degrees apart then rotate around Y.
            vector3 Axis = m_Cinema.m_CurrentLookDir.Cross(m_Cinema.m_DesiredLookDir);
            if( Angle > R_179 )
            {
                Axis.Set(0,1,0);
            }

            Axis.Normalize();

            // set up blend interpolation
            f32 AngleModifierT = 1.0f;

            // no divides by zero or ridiculously small blendin times
            if( m_Cinema.m_BlendInTime >= F32_MIN )
            {
                AngleModifierT = m_Cinema.m_CurrentBlendInTime / m_Cinema.m_BlendInTime;

                // The closer we get to BlendInTime, the more of the angle difference we want to take.        
                AngleModifierT = x_clamp(AngleModifierT, 0.0f, 1.0f);

                m_Cinema.m_CurrentBlendInTime += DeltaTime;

                // make sure we don't get a weird number
                m_Cinema.m_CurrentBlendInTime = x_clamp(m_Cinema.m_CurrentBlendInTime, 0.0f, m_Cinema.m_BlendInTime);
            }

            // Rotate current towards desired but use the AngleModifierT to ramp speed 
            // (i.e. if m_CurrentBlenInTime = m_BlendInTime then AngleModifierT = 1)
            Angle =  (AngleModifierT * Angle);
            quaternion Q(Axis,Angle);
            m_Cinema.m_CurrentLookDir = Q * m_Cinema.m_CurrentLookDir;

            // Pull out pitch and yaw from new look direction and feed to player
            radian Pitch;
            radian Yaw;
            m_Cinema.m_CurrentLookDir.GetPitchYaw( Pitch, Yaw );
            
            // SB: Keep camera info for "player::ComputeView"
            vector3 CameraPos = m_Cinema.m_CameraV2W.GetTranslation();
            radian3 CameraRot = m_Cinema.m_CameraV2W.GetRotation();
            CameraRot.Pitch = Pitch;
            CameraRot.Yaw   = Yaw;
            m_Cinema.m_CameraV2W.Setup( vector3( 1.0f, 1.0f, 1.0f ),
                                        CameraRot,
                                        CameraPos );
            
            // Tell player the new pitch and yaw
            SetPitch( Pitch );
            SetYaw( Yaw );
        }
    }
#ifdef ksaffel    

    if( g_ShowViewPosition )
    {
        vector3 ViewPos = GetView().GetPosition();
        vector3 Pos = GetPosition();

        vector3 Diff = Pos - ViewPos;

        x_printfxy( 1, 2, "Player( %7.1f, %7.1f, %7.1f )", Pos.GetX(), Pos.GetY(), Pos.GetZ() );
        x_printfxy( 1, 3, "View  ( %7.1f, %7.1f, %7.1f )", ViewPos.GetX(), ViewPos.GetY(), ViewPos.GetZ() );
        x_printfxy( 1, 4, "Diff  ( %7.1f, %7.1f, %7.1f )", Diff.GetX(), Diff.GetY(), Diff.GetZ() );    
        x_printfxy( 1, 5, "DiffY ( %7.1f", Diff.GetY() );
    }
#endif
}

//------------------------------------------------------------------------------

void player::EndCinema( void )
{
    // Get gun back on screen
    //SetAnimation( ANIM_STATE_SWITCH_TO , ANIM_PRIORITY_DEFAULT , 0.f );

    // Clear cinema values
    m_Cinema.m_LookAtTargetGuid = 0;

    // KSS -- new cinema code
    {
        // Snap player to ground
        {
            vector3 Pos = GetPosition();

            vector3 S = Pos + vector3(0,150,0);
            vector3 E = Pos + vector3(0,-150,0);
            g_CollisionMgr.SphereSetup( GetGuid(), S, E, 5.0f );    // Use sphere in-case of cracks!
            g_CollisionMgr.UseLowPoly();
            g_CollisionMgr.CheckCollisions( 
                object::TYPE_ALL_TYPES,                                 // these types
                object::ATTR_COLLIDABLE,                                // these attributes
                object::ATTR_PLAYER | object::ATTR_CHARACTER_OBJECT );  // not these attributes

            if( g_CollisionMgr.m_nCollisions > 0 )
            {
                // Put player slightly above the collision point
                Pos = g_CollisionMgr.m_Collisions[0].Point + vector3( 0.0f, 0.5f, 0.0f );
            }

            // Move player to final safe pos
            {
                matrix4 L2W;
                L2W.Identity();
                L2W.RotateY(m_Yaw);
                L2W.Translate(Pos);

                OnTransform( L2W );
                OnMoveViewPosition( L2W.GetTranslation() );
                UpdateZoneTrack();
            }
        }

        // are we using view correction?
        if( m_Cinema.m_bUseViewCorrection )
        {
            // Setup offset to blend from cinema eye position back to player eye position
            m_Cinema.m_ViewCorrectionDelta = m_EyesPosition - GetDefaultViewPos();
        }
        else
        {
            // just make sure
            m_Cinema.m_ViewCorrectionDelta.Zero();
        }
    }

    hud_object* Hud = GetHud();
    m_Physics.SetSolveActorCollisions( TRUE );
    m_Physics.SetLocoCollisionOn( TRUE );
    m_Physics.SetLocoGravityOn(TRUE) ;

    if( Hud )
    {
        Hud->SetupLetterBox( FALSE, 0.8f );
    }

    //
    // Do we need to turn the flashlight back on?
    //
    if ( m_bUsingFlashlightBeforeCinema )
    {
        SetFlashlightActive( TRUE );
        m_bUsingFlashlightBeforeCinema = FALSE;
    }
}

//==============================================================================

void player::BeginMissionFailed( void )
{
}

//==============================================================================

f32 DisplayTextTime = 3.0f;
f32 FadeOutTime = 2.0f;

void player::UpdateMissionFailed( f32 DeltaTime )
{
    (void)DeltaTime;
    const f32 LastTimeInState = m_TimeInState - DeltaTime;
    if ( IN_RANGE( LastTimeInState, DisplayTextTime, m_TimeInState ) )
    {
        g_PostEffectMgr.StartScreenFade( xcolor(0,0,0,255), FadeOutTime );
    }
    else if ( IN_RANGE( LastTimeInState, DisplayTextTime + FadeOutTime, m_TimeInState ) )
    {
#if !defined( X_EDITOR )
        pGameLogic->PlayerDied( m_NetSlot, m_NetSlot, 0 );
#endif
        xbool PrimaryPressed = (xbool)g_IngamePad[m_ActivePlayerPad].GetLogical( ingame_pad::ACTION_PRIMARY ).WasValue;
#if defined(TARGET_PC) && !defined(X_EDITOR)
        PrimaryPressed |= input_WasPressed( INPUT_MOUSE_BTN_L );
#endif
        if( PrimaryPressed )
        {
            m_bWantToSpawn = TRUE;
#ifndef X_EDITOR
            m_NetDirtyBits |= WANT_SPAWN_BIT;  // NETWORK
#endif // X_EDITOR

            m_bRespawnButtonPressed = TRUE;
        }
    }
}

//==============================================================================

void player::EndMissionFailed( void )
{
    m_CorpseGuid    = 0;
    m_AnimStage     = 0;
    m_DeathType     = DEATH_BY_ANIM;
}

//==============================================================================
