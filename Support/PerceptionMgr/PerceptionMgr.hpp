#ifndef PERCEPTION_MGR_HPP
#define PERCEPTION_MGR_HPP

#include "x_types.hpp"

class perception_mgr 
{
public:

    enum perception_state
    {
        STATE_NORMAL = 0,
        STATE_BEGIN_MUTATE,
        STATE_BEGIN_MUTATE_DELAY,
        STATE_TO_MUTATE,
        STATE_MUTATED,
        STATE_END_MUTATE,
        STATE_END_MUTATE_DELAY,
        STATE_FROM_MUTATE,
        STATE_BEGIN_SHELLSHOCK,
        STATE_TO_SHELLSHOCK,
        STATE_SHELLSHOCK,
        STATE_END_SHELLSHOCK,
        STATE_FROM_SHELLSHOCK
    };
                        perception_mgr                      ( void );
                       ~perception_mgr                      ( void );
    void                Init                                ( void );
    void                Kill                                ( void );
    void                Update                              ( f32               DeltaTime );
    void                NewState                            ( perception_state  TargetState );
    void                BeginMutate                         ( void );
    void                EndMutate                           ( void );
    void                BeginShellShock                     ( f32               Severity );
    void                EndShellShock                       ( void );
    f32                 GetGlobalTimeDialation              ( void )            { return m_GlobalTimeDialation; }
    f32                 GetPlayerTimeDialation              ( void )            { return m_PlayerTimeDialation; }
    f32                 GetAudioTimeDialation               ( void )            { return m_AudioTimeDialation; }
    f32                 GetForwardSpeedFactor               ( void )            { return m_ForwardSpeedFactor; }
    f32                 GetTurnRateFactor                   ( void )            { return m_TurnRateFactor; }
    void                SetMutantTargetGlobalTimeDialation  ( f32 Target )      { m_MutantTargetGlobalTimeDialation = Target; }
    void                SetMutantTargetPlayerTimeDialation  ( f32 Target )      { m_MutantTargetPlayerTimeDialation = Target; }
    void                SetMutantTargetAudioTimeDialation   ( f32 Target )      { m_MutantTargetAudioTimeDialation = Target; }
    void                SetMutantTargetForwardSpeedFactor   ( f32 Target )      { m_MutantTargetForwardSpeedFactor = Target; }
    void                SetMutantBeginDelay                 ( f32 Target )      { m_BeginMutantDelay = Target; }
    void                SetMutantBeginLength                ( f32 Target )      { m_BeginMutantLength = Target; }
    void                SetMutantEndDelay                   ( f32 Target )      { m_EndMutantDelay = Target; }
    void                SetMutantEndLength                  ( f32 Target )      { m_EndMutantLength = Target; }

    void                SetTriggerTargetGlobalTimeDialation ( f32 Target )      { m_TriggerTargetGlobalTimeDialation = Target; }
    void                SetTriggerTargetAudioTimeDialation  ( f32 Target )      { m_TriggerTargetAudioTimeDialation = Target; }
    void                SetTriggerTargetForwardSpeedFactor  ( f32 Target )      { m_TriggerTargetForwardSpeedFactor = Target; }
    void                SetTriggerTargetTurnRateFactor      ( f32 Target )      { m_TriggerTargetTurnRateFactor = Target; }
    void                SetTriggerBeginLength               ( f32 Target )      { m_TriggerBeginLength = Target; }
    void                SetTriggerEndLength                 ( f32 Target )      { m_TriggerEndLength = Target; }
    void                BeginTriggerPerception              ( void )            {}
    void                EndTriggerPerception                ( void )            {}

private:

    perception_state    m_State;

    f32                 m_GlobalTimeDialation;
    f32                 m_PlayerTimeDialation;
    f32                 m_AudioTimeDialation;
    f32                 m_ForwardSpeedFactor;
    f32                 m_TurnRateFactor;

    f32                 m_MutantTargetGlobalTimeDialation;
    f32                 m_MutantTargetPlayerTimeDialation;
    f32                 m_MutantTargetAudioTimeDialation;
    f32                 m_MutantTargetForwardSpeedFactor;

    f32                 m_TriggerTargetGlobalTimeDialation;
    f32                 m_TriggerTargetAudioTimeDialation;
    f32                 m_TriggerTargetForwardSpeedFactor;
    f32                 m_TriggerTargetTurnRateFactor;
    f32                 m_TriggerBeginLength;
    f32                 m_TriggerEndLength;

    f32                 m_Timer;
    f32                 m_BeginMutantDelay;
    f32                 m_BeginMutantLength;
    f32                 m_EndMutantDelay;
    f32                 m_EndMutantLength;
};

extern perception_mgr g_PerceptionMgr;

#endif // PERCEPTION_MGR_HPP