#ifndef __GRUNT_ATTACK_STATE_HPP
#define __GRUNT_ATTACK_STATE_HPP

#include "characters\basestates\character_attack_state.hpp"


class grunt_attack_state : public character_attack_state
{

public:

    enum grunt_attack_phases
    {
        PHASE_GRUNT_ATTACK_CLOSE_TO_MELEE = PHASE_BASE_COUNT,
        PHASE_GRUNT_ATTACK_RETREAT,
        PHASE_GRUNT_ATTACK_STAND_SHOOT,
        PHASE_GRUNT_ATTACK_STAND_SHOOT_PACE_LEFT,
        PHASE_GRUNT_ATTACK_STAND_SHOOT_PACE_RIGHT,
        PHASE_GRUNT_ATTACK_EVADE,
        PHASE_GRUNT_ATTACK_LEAP,
        PHASE_GRUNT_ATTACK_SHORT_MELEE,
        PHASE_GRUNT_ATTACK_LONG_MELEE,
        PHASE_GRUNT_ATTACK_MELEE_BACK_LEFT,
        PHASE_GRUNT_ATTACK_MELEE_BACK_RIGHT,
        PHASE_GRUNT_ATTACK_MELEE_STRAFE_LEFT,
        PHASE_GRUNT_ATTACK_MELEE_STRAFE_RIGHT,
        PHASE_GRUNT_ATTACK_PACE_LEFT,
        PHASE_GRUNT_ATTACK_PACE_RIGHT,
        PHASE_GRUNT_ATTACK_PACE_IDLE,
        PHASE_GRUNT_ATTACK_NO_TARGET,
        PHASE_GRUNT_ATTACK_GROWL,
        PHASE_GRUNT_ATTACK_SURPRISED,
        PHASE_GRUNT_ATTACK_SHUFFLE_LEFT,
        PHASE_GRUNT_ATTACK_SHUFFLE_RIGHT,
    };

// Functions
public:
                    grunt_attack_state      ( character& Mutant, states State ) ;
    virtual         ~grunt_attack_state     ( void );
    virtual void    OnEnter                 ( void ) ;
    virtual xbool   OnExit                  ( void ) ;

    // character state specific virtual functions
    virtual s32     UpdatePhase             ( f32 DeltaTime );
    virtual void    ChangePhase             ( s32 newPhase );
    virtual character_state::states  UpdateState             ( f32 DeltaTime );

    virtual void    OnBeingShotAt   ( object::type Type , guid ShooterID );
    virtual xbool   IsMeleeingPlayer( void );
            
    // debugging information
    virtual const char*GetStateName ( void )    { return "CHARACTER_GRUNT_ATTACK"; }    
    virtual const char*GetPhaseName ( s32 thePhase );

protected:
            s32     GetChasePhase       ( void );

            s32     GetMeleePhase       ( void );
            s32     GetShortMeleePhase  ( void );
            s32     GetLongMeleePhase   ( void );
            s32     GetShufflePhase      ( void );

            s32     GetPacePhase        ( void );
            s32     GetStandPacePhase   ( void );
            s32     GetMeleeStrafePhase ( void );
            s32     GetKungFuPhase      ( void );

            xbool   CanDoShortMelee     ( void );
            xbool   CanDoLongMelee      ( void );
// Data
public:

    // Different variables needed for the states between MELEE_BEGIN and MELEE_END
        xbool   m_LastPaceLeft;
        f32     m_EvadeTimer;
        f32     m_LeapTimer;   
        f32     m_MeleeTimer;   
        f32     m_Max_No_See_Time;
        f32     m_SinceLastShuffle;
        f32     m_SinceLastStrafe;
        xbool   m_bWantToEvade;
        xbool   m_bSwitchOutOfAttack;
        xbool   m_bHasClearJumpAttack;
        xbool   m_bDoLongMelee;
        xbool   m_bLastShuffleLeft;

        f32     m_CantReachTimer;
        f32     m_TimeInPaceIdle;
static  s32     s_LastChasePhase;
} ;

#endif