#ifndef _GRUNT_COVER_STATE_HPP_
#define _GRUNT_COVER_STATE_HPP_

#include "Characters\BaseStates\Character_Cover_State.hpp"

class grunt_cover_state : public character_cover_state
{
// enums
public:
    enum grunt_cover_phases
    {
        PHASE_GRUNT_COVER_GOTO_COVER = PHASE_BASE_COUNT,
        PHASE_GRUNT_COVER_FACE_EXACT,
        PHASE_GRUNT_COVER_ENTER_COVER,
        PHASE_GRUNT_COVER_IDLE,
        PHASE_GRUNT_COVER_EXIT_COVER,
    };

// Functions
public:
                    grunt_cover_state  ( character& ourCharacter, character_state::states State );
    virtual        ~grunt_cover_state  ( void );

    virtual void    OnEnumProp              ( prop_enum&    List ) ;
    virtual xbool   OnProperty              ( prop_query&   rPropQuery ) ;

    // character state specific virtual functions
    virtual xbool   UseRelativeMode         ( void );
    virtual s32     UpdatePhase             ( f32 DeltaTime );
    virtual void    ChangePhase             ( s32 newPhase );
    virtual character_state::states  UpdateState( f32 DeltaTime );
    virtual xbool   GetIsAtCover            ( void )        { return ( m_CurrentPhase != PHASE_GRUNT_COVER_GOTO_COVER ); }
    virtual xbool   ProvideAutofire         ( void );
    virtual xbool   IgnoreFullBodyFlinches  ( void );
            
    // debugging information
    virtual const char*GetStateName ( void )    { return "CHARACTER_GRUNT_COVER"; }    
    virtual const char*GetPhaseName ( s32 thePhase = PHASE_NONE );

protected:

    virtual xbool   GetAnimNameFromPhase( s32 nextPhase, char* pBufferSize32 );
    virtual s32     GetFacePhase();

protected:
};

#endif
