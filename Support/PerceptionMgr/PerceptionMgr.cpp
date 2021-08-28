#include "PerceptionMgr.hpp"
#include "e_Audio.hpp"
#include "audio\audio_hardware.hpp"
#include "NetworkMgr\GameMgr.hpp"
#ifndef X_EDITOR
#include "NetworkMgr\NetworkMgr.hpp"
#endif

perception_mgr g_PerceptionMgr;

perception_mgr::perception_mgr( void )
{
    m_GlobalTimeDialation = 
    m_PlayerTimeDialation =
    m_AudioTimeDialation  =
    m_ForwardSpeedFactor  =
    m_TurnRateFactor      = 1.0f;

    m_State = STATE_NORMAL;

    m_MutantTargetGlobalTimeDialation = 0.65f;
    m_MutantTargetPlayerTimeDialation = 1.0f;
    m_MutantTargetAudioTimeDialation  = 0.65f;
    m_MutantTargetForwardSpeedFactor  = 1.0f;

    m_BeginMutantDelay  = 1.2f;
    m_BeginMutantLength = 2.5f;

    m_EndMutantDelay  = 0.0f;
    m_EndMutantLength = 2.5f;
}

perception_mgr::~perception_mgr( void )
{
}

void perception_mgr::Init( void )
{
    m_GlobalTimeDialation = 
    m_PlayerTimeDialation =
    m_AudioTimeDialation  =
    m_ForwardSpeedFactor  =
    m_TurnRateFactor      = 1.0f;

    m_State = STATE_NORMAL;

    g_AudioHardware.SetPitchFactor( m_AudioTimeDialation );
}

void perception_mgr::Kill( void )
{
}

void perception_mgr::NewState( perception_state TargetState )
{
#ifndef X_EDITOR
    // Ignore state change requests if this is a splitscreen game.
    if( g_NetworkMgr.GetLocalPlayerCount() > 1 )
    {
        TargetState = STATE_NORMAL;
    }
#endif

    m_State = TargetState;
}

void perception_mgr::Update( f32 DeltaTime )
{
#ifdef X_EDITOR
    xbool bIsMultiplayer = FALSE;
#else
    xbool bIsMultiplayer = GameMgr.IsGameMultiplayer();
#endif

    switch( m_State )
    {
        case STATE_NORMAL:
            break;

        case STATE_BEGIN_MUTATE:
            // Reset timer.
            m_Timer = 0.0f;

            // Handle pre-delay.
            NewState( STATE_BEGIN_MUTATE_DELAY );
            break;

        case STATE_BEGIN_MUTATE_DELAY:
            // Tick tock...
            m_Timer += DeltaTime;

            // Done?
            if( m_Timer >= m_BeginMutantDelay )
            {
                // Reset timer.
                m_Timer = 0.0f;

                // Start mutating!
                NewState( STATE_TO_MUTATE );
            }
            break;

        case STATE_TO_MUTATE:
            // Tick tock...
            m_Timer += DeltaTime;

            // All done?
            if( m_Timer >= m_BeginMutantLength )
            {
                // Force 'em to the target.
                m_GlobalTimeDialation = m_MutantTargetGlobalTimeDialation;
                m_AudioTimeDialation  = m_MutantTargetAudioTimeDialation;
                m_PlayerTimeDialation = m_MutantTargetPlayerTimeDialation;
                m_ForwardSpeedFactor  = m_MutantTargetForwardSpeedFactor;
                
                // We are now mutated!
                NewState( STATE_MUTATED );
            }
            else
            {
                // Move to the target values!
                f32 Delta;
                f32 Scale = 1.0f - (m_Timer / m_BeginMutantLength);

                Delta = 1.0f - m_MutantTargetGlobalTimeDialation;
                m_GlobalTimeDialation = m_MutantTargetGlobalTimeDialation + Delta * Scale;

                Delta = 1.0f - m_MutantTargetAudioTimeDialation;
                m_AudioTimeDialation = m_MutantTargetAudioTimeDialation + Delta * Scale;

                Delta = 1.0f - m_MutantTargetPlayerTimeDialation;
                m_PlayerTimeDialation = m_MutantTargetPlayerTimeDialation + Delta * Scale;

                Delta = 1.0f - m_MutantTargetForwardSpeedFactor;
                m_ForwardSpeedFactor = m_MutantTargetForwardSpeedFactor + Delta * Scale;
            }


            if( bIsMultiplayer )
            {
                m_GlobalTimeDialation = 1.0f;
                m_PlayerTimeDialation = 1.0f;
            }
            else
            {
                // Factor out the global time dialation.
                m_PlayerTimeDialation /= m_GlobalTimeDialation;
            }

            g_AudioHardware.SetPitchFactor( m_AudioTimeDialation );
            break;

        case STATE_MUTATED:
            break;

        case STATE_END_MUTATE:
            // Reset timer.
            m_Timer = 0.0f;

            // Do the pre-delay.
            NewState( STATE_END_MUTATE_DELAY );
            break;

        case STATE_END_MUTATE_DELAY:
            // Tick tock...
            m_Timer += DeltaTime;

            // Done?
            if( m_Timer >= m_EndMutantDelay )
            {
                // Reset timer.
                m_Timer = 0.0f;

                // Go back to normal!
                NewState( STATE_FROM_MUTATE );
            }
            break;

        case STATE_FROM_MUTATE:
            // Tick tock...
            m_Timer += DeltaTime;

            // Done?
            if( m_Timer >= m_EndMutantLength )
            {
                // Force 'em normal
                m_GlobalTimeDialation = 1.0f;
                m_PlayerTimeDialation = 1.0f;
                m_AudioTimeDialation  = 1.0f;
                m_ForwardSpeedFactor  = 1.0f;

                // All better now!
                NewState( STATE_NORMAL );
            }
            else
            {
                // Move to normal values!
                f32 Delta;
                f32 Scale = 1.0f - (m_Timer / m_BeginMutantLength);

                Delta = m_MutantTargetGlobalTimeDialation - 1.0f;
                m_GlobalTimeDialation = 1.0f + Delta * Scale;

                Delta = m_MutantTargetAudioTimeDialation - 1.0f;
                m_AudioTimeDialation = 1.0f + Delta * Scale;

                Delta = m_MutantTargetPlayerTimeDialation - 1.0f;
                m_PlayerTimeDialation = 1.0f + Delta * Scale;

                Delta = m_MutantTargetForwardSpeedFactor - 1.0f;
                m_ForwardSpeedFactor = 1.0f + Delta * Scale;
            }

            if( bIsMultiplayer )
            {
                m_GlobalTimeDialation = 1.0f;
                m_PlayerTimeDialation = 1.0f;
            }
            else
            {
                // Factor out the global time dialation.
                m_PlayerTimeDialation /= m_GlobalTimeDialation;
            }

            g_AudioHardware.SetPitchFactor( m_AudioTimeDialation );
            break;

        case STATE_BEGIN_SHELLSHOCK:
            break;

        case STATE_TO_SHELLSHOCK:
            break;

        case STATE_SHELLSHOCK:
            break;

        case STATE_END_SHELLSHOCK:
            break;

        case STATE_FROM_SHELLSHOCK:
            break;
    }
}

void perception_mgr::BeginMutate( void )
{
    NewState( STATE_BEGIN_MUTATE );
}

void perception_mgr::EndMutate( void )
{
    NewState( STATE_END_MUTATE );
}

void perception_mgr::BeginShellShock( f32 Severity )
{
    (void)Severity;
    NewState( STATE_BEGIN_SHELLSHOCK );
}

void perception_mgr::EndShellShock( void )
{
    NewState( STATE_END_SHELLSHOCK );
}
