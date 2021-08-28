#ifndef __LEAPER_ATTACK_STATE_HPP
#define __LEAPER_ATTACK_STATE_HPP

#include "characters\basestates\character_attack_state.hpp"


class leaper_attack_state : public character_attack_state
{

public:

    enum leaper_attack_phases
    {
        PHASE_LEAPER_ATTACK_CLOSE_TO_LEAP = PHASE_BASE_COUNT,
        PHASE_LEAPER_ATTACK_CLOSE_TO_MELEE,
        PHASE_LEAPER_ATTACK_STRAFE_RIGHT,
        PHASE_LEAPER_ATTACK_STRAFE_LEFT,
        PHASE_LEAPER_ATTACK_EVADE,
        PHASE_LEAPER_ATTACK_RETREAT_EVADE,
        PHASE_LEAPER_ATTACK_LEAP,
        PHASE_LEAPER_ATTACK_SHORT_MELEE,
        PHASE_LEAPER_ATTACK_LONG_MELEE,
        PHASE_LEAPER_ATTACK_GROWL,
        PHASE_LEAPER_ATTACK_SURPRISED,
        PHASE_LEAPER_ATTACK_RETREAT
    };

// Functions
public:
                    leaper_attack_state      ( character& Mutant, states State ) ;
    virtual         ~leaper_attack_state     ( void );
    virtual void    OnEnter                 ( void ) ;
    virtual xbool   OnExit                  ( void ) ;

    // character state specific virtual functions
    virtual s32     UpdatePhase             ( f32 DeltaTime );
    virtual void    ChangePhase             ( s32 newPhase );
    virtual character_state::states  UpdateState             ( f32 DeltaTime );

    virtual void    OnBeingShotAt   ( object::type Type , guid ShooterID );
    virtual xbool   IsMeleeingPlayer( void );
            
    // debugging information
    virtual const char*GetStateName ( void )    { return "CHARACTER_LEAPER_ATTACK"; }    
    virtual const char*GetPhaseName ( s32 thePhase );

protected:
            s32     GetChasePhase   ( void );
            s32     GetAttackPhase  ( void );
            s32     GetStrafePhase  ( void );
// Data
public:

    // Different variables needed for the states between MELEE_BEGIN and MELEE_END
        xbool   m_LastPaceLeft;
        f32     m_EvadeTimer;
        f32     m_LeapTimer;   
        f32     m_Max_No_See_Time;
        xbool   m_bWantToEvade;
        xbool   m_bSwitchOutOfAttack;
        xbool   m_bHasClearJumpAttack;

        f32     m_CantReachTimer;
static  s32     s_LastChasePhase;
} ;

#endif