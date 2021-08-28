#ifndef _BLACKOP_COVER_STATE_HPP_
#define _BLACKOP_COVER_STATE_HPP_

#include "Characters\BaseStates\Character_Cover_State.hpp"

class blackOp_cover_state : public character_cover_state
{
// enums
public:
    enum bo_cover_phases
    {
        PHASE_BO_COVER_GOTO_COVER = PHASE_BASE_COUNT,
        PHASE_BO_COVER_FACE_EXACT,
        PHASE_BO_COVER_ENTER_COVER,
        PHASE_BO_COVER_IDLE,
        PHASE_BO_COVER_ROLL_OUT,
        PHASE_BO_COVER_OUT_IDLE,
        PHASE_BO_COVER_OUT_SCAN,
        PHASE_BO_COVER_OUT_GRENADE,
        PHASE_BO_COVER_FULL_AUTO,
        PHASE_BO_COVER_ROLL_IN,
        PHASE_BO_COVER_EXIT_COVER,
        PHASE_BO_COVER_ALIGN_FOR_MELEE,
        PHASE_BO_COVER_MELEE,
        PHASE_BO_COVER_GOTO_CORPSE,
        PHASE_BO_COVER_DRAIN_CORPSE,
        PHASE_BO_COVER_SUMMON_ALLIES,
        PHASE_BO_COVER_STAND_AND_SHOOT,
        PHASE_BO_COVER_COVER_ME,
        PHASE_BO_COVER_REQUEST_ATTACK,
    };

// Functions
public:
                    blackOp_cover_state  ( character& ourCharacter, character_state::states State );
    virtual        ~blackOp_cover_state  ( void );

    // character state specific virtual functions
    virtual xbool   UseRelativeMode         ( void );
    virtual s32     UpdatePhase             ( f32 DeltaTime );
    virtual void    ChangePhase             ( s32 newPhase );
    virtual character_state::states  UpdateState( f32 DeltaTime );
    virtual xbool   ProvideAutofire         ( void );
    virtual xbool   IgnoreFullBodyFlinches  ( void );
    virtual void    AutofireRequestSent     ( void ) { m_AutofireRequestSent = TRUE; }
    virtual xbool   CanReload               ( void )            { return GetCurrentPhase() != PHASE_BO_COVER_COVER_ME; }
    virtual xbool   OnPain                  ( const pain& Pain );
    virtual void    OnEnter                 ( void );
            
    // debugging information
    virtual xbool   GetIsAtCover            ( void )        { return ( m_CurrentPhase != PHASE_BO_COVER_GOTO_COVER ); }
    virtual const char*GetStateName ( void )    { return "BLACKOP_COVER_COVER"; }    
    virtual const char*GetPhaseName ( s32 thePhase = PHASE_NONE );

protected:

    virtual xbool   GetAnimNameFromPhase( s32 nextPhase, char* pBufferSize32 );
    virtual s32     GetNextPhaseRolledOut();    
    virtual s32     GetNextPhaseRolledIn();
    virtual s32     GetNextPhaseOutOfCover();
    virtual s32     GetFacePhase();

    xbool m_AutofireRequestSent;
    xbool m_WantsToRollin;
    f32   m_MinTimeRolledin;
};

#endif
