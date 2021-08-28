#ifndef _CHARACTER_IDLE_STATE_HPP_
#define _CHARACTER_IDLE_STATE_HPP_

#include "..\CharacterState.hpp"

class character_idle_state : public character_state
{
// enums
public:
    enum base_idle_phases
    {
        PHASE_IDLE_DEFAULT = PHASE_BASE_COUNT,
        PHASE_IDLE_WANDER,
    };

// Functions
public:
                    character_idle_state    ( character& ourCharacter, character_state::states State );
    virtual         ~character_idle_state   ( void );
    virtual void    OnInit                  ( void );
    virtual void    OnEnter                 ( void );
    virtual void    OnThink                 ( void );
    virtual void    OnEnumProp              ( prop_enum&    List ) ;
    virtual xbool   OnProperty              ( prop_query&   rPropQuery ) ;
    virtual void    OnBeingShotAt           ( object::type Type , guid ShooterID );
    virtual f32     GetAimerBlendScale      ( void )                      { return 0.3f; }

    // character state specific virtual functions
    virtual s32     UpdatePhase             ( f32 DeltaTime );
    virtual void    ChangePhase             ( s32 newPhase );
    virtual character_state::states  UpdateState             ( f32 DeltaTime );

    // debugging information
    virtual const char*GetStateName ( void )    { return "CHARACTER_IDLE"; }    
    virtual const char*GetPhaseName ( s32 thePhase = PHASE_NONE );


protected:

    f32     m_TimeTillWander;
    f32     m_AverageTimeTillWander;
    f32     m_WanderLeashRadius;
    xtick   m_LookatTime;
};

#endif
