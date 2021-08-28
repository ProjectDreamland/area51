#ifndef _CHARACTER_SEARCH_STATE_HPP_
#define _CHARACTER_SEARCH_STATE_HPP_

#include "..\CharacterState.hpp"

class character_search_state : public character_state
{
// enums
public:
    enum base_search_phases
    {
        PHASE_SEARCH_GOTO_INTEREST = PHASE_BASE_COUNT,
        PHASE_SEARCH_LOOK_AROUND,
        PHASE_SEARCH_IDLE_MOMENTARILY,
        PHASE_SEARCH_WALK_AROUND,
        PHASE_SEARCH_SPOTTED_SURPRISE,
    };

// Functions
public:
                    character_search_state  ( character& ourCharacter, character_state::states State );
    virtual        ~character_search_state  ( void );

    virtual void    OnInit                  ( void );
    virtual void    OnEnter                 ( void );
    virtual void    OnEnumProp              ( prop_enum&    List ) ;
    virtual xbool   OnProperty              ( prop_query&   rPropQuery ) ;

    // character state specific virtual functions
    virtual s32     UpdatePhase             ( f32 DeltaTime );
    virtual void    ChangePhase             ( s32 newPhase );
    virtual character_state::states  UpdateState             ( f32 DeltaTime );
    virtual f32     GetAimerBlendScale      ( void )                      { return 0.6f; }

    // debugging information
    virtual const char*GetStateName ( void )    { return "CHARACTER_SEARCH_SEARCH"; }    
    virtual const char*GetPhaseName ( s32 thePhase = PHASE_NONE );

protected:
    f32     m_TimeTillBored;
    f32     m_LookAroundTime;
    f32     m_TimeAtLocationOfInterest;

    xbool   m_SpottedSurpriseDone;
};

#endif
