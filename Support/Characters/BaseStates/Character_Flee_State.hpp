#ifndef _CHARACTER_FLEE_STATE_HPP_
#define _CHARACTER_FLEE_STATE_HPP_

#include "..\CharacterState.hpp"

class character_flee_state : public character_state
{
// enums
public:
    enum base_flee_phases
    {
        PHASE_FLEE_FLEE_FROM_TARGET = PHASE_BASE_COUNT,
        PHASE_FLEE_TURN_TO_TARGET_RUNNING,
        PHASE_FLEE_RUNING_COWER,
    };

// Functions
public:
                    character_flee_state  ( character& ourCharacter, character_state::states State );
    virtual        ~character_flee_state  ( void );

    virtual void    OnInit                  ( void );
    virtual void    OnEnter                 ( void );
    virtual void    OnThink                 ( void );
    virtual void    OnEnumProp              ( prop_enum&    List ) ;
    virtual xbool   OnProperty              ( prop_query&   rPropQuery ) ;

    // character state specific virtual functions
    virtual s32     UpdatePhase             ( f32 DeltaTime );
    virtual void    ChangePhase             ( s32 newPhase );
    virtual character_state::states  UpdateState             ( f32 DeltaTime );

    // debugging information
    virtual const char*GetStateName ( void )    { return "CHARACTER_FLEE_FLEE"; }    
    virtual const char*GetPhaseName ( s32 thePhase = PHASE_NONE );

protected:
    f32         m_TimeTillBored;
    f32         m_PauseInPlace;
};

#endif
