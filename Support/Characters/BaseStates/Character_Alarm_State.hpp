#ifndef _CHARACTER_ALARM_STATE_HPP_
#define _CHARACTER_ALARM_STATE_HPP_

#include "..\CharacterState.hpp"

class character_alarm_state : public character_state
{
// enums
public:
    enum base_alarm_phases
    {
        PHASE_ALARM_GOTO_ALARM = PHASE_BASE_COUNT,
        PHASE_ALARM_FACE_EXACT,
        PHASE_ALARM_USE_ALARM,
    };

// Functions
public:
                    character_alarm_state  ( character& ourCharacter, character_state::states State );
    virtual        ~character_alarm_state  ( void );

    virtual void    OnInit                  ( void );
    virtual void    OnEnter                 ( void );
    virtual void    OnThink                 ( void );
    virtual void    OnEnumProp              ( prop_enum&    List ) ;
    virtual xbool   OnProperty              ( prop_query&   rPropQuery ) ;

    // character state specific virtual functions
    virtual s32     UpdatePhase             ( f32 DeltaTime );
    virtual void    ChangePhase             ( s32 newPhase );
    virtual character_state::states  UpdateState( f32 DeltaTime );
            
    // debugging information
    virtual const char*GetStateName ( void )    { return "CHARACTER_ALARM"; }    
    virtual const char*GetPhaseName ( s32 thePhase = PHASE_NONE );

protected:

    xbool       m_StateDone;
    xbool       m_HasValidTarget;
};

#endif
