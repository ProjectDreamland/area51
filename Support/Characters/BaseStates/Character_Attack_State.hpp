#ifndef _CHARACTER_ATTACK_STATE_HPP_
#define _CHARACTER_ATTACK_STATE_HPP_

#include "..\CharacterState.hpp"

class character_attack_state : public character_state
{
// enums
public:
    enum base_attack_phases
    {
        PHASE_ATTACK_CLOSE_TO_OPTIMAL = character_state::PHASE_BASE_COUNT,
        PHASE_ATTACK_EVADE_LEFT,
        PHASE_ATTACK_EVADE_RIGHT,
        PHASE_ATTACK_IDLE,
        PHASE_ATTACK_CLOSE_FOR_MELEE,
        PHASE_ATTACK_ALIGN_FOR_SHORT_MELEE,
        PHASE_ATTACK_SHORT_MELEE,
        PHASE_ATTACK_ALIGN_FOR_LONG_MELEE,
        PHASE_ATTACK_LONG_MELEE,
        PHASE_ATTACK_CLOSE_FOR_JUMP_ATTACK,
        PHASE_ATTACK_ALIGN_FOR_JUMP_ATTACK,
        PHASE_ATTACK_JUMP_ATTACK,
        PHASE_ATTACK_RETREAT_TO_OPTIMAL,
        PHASE_ATTACK_SURPRISED,
        PHASE_ATTACK_COUNT
    };

    enum eCoverDesire
    {
        SEEK_COVER_INVALID,
        SEEK_COVER_ALWAYS,
        SEEK_COVER_SHOTAT,
        SEEK_COVER_DAMAGED,
        SEEK_COVER_NEVER,
    };
// Functions
public:
                    character_attack_state    ( character& ourCharacter, character_state::states State );
    virtual         ~character_attack_state   ( void );
    virtual void    OnInit                  ( void );
    virtual void    OnEnter                 ( void );
    virtual xbool   OnExit                  ( void );
    virtual void    OnThink                 ( void );
    virtual void    OnEnumProp              ( prop_enum&    List ) ;
    virtual xbool   OnProperty              ( prop_query&   rPropQuery ) ;

    virtual xbool   IgnoreSound             ( void )                        { return TRUE; }
        
    // character state specific virtual functions
    virtual s32     UpdatePhase             ( f32 DeltaTime );
    virtual void    ChangePhase             ( s32 newPhase );
    virtual character_state::states  UpdateState             ( f32 DeltaTime );

    // debugging information
    virtual const char*GetStateName ( void )    { return "CHARACTER_ATTACK"; }    
    virtual const char*GetPhaseName ( s32 thePhase = PHASE_NONE );

protected:
            s32     ChooseNextPhase         ( void );
            s32     ChooseEvade             ( void );
            s32     ChooseTargetHiddenPhase ( void );
            s32     ChooseTargetVisiblePhase( void );

    static enum_table<eCoverDesire> m_SeekCoverEnum; 
    xtick   m_ScanTime;
    f32     m_MeleeAttackDelay;
    f32     m_OptimalDistance;
    f32     m_SinceLastMeleeAttack;
    f32     m_SinceLastEvade;
    f32     m_SinceLastJumpAttack;
    f32     m_SinceLastShotAt;
    f32     m_JumpAttackMinDistance;
    f32     m_JumpAttackMaxDistance;
    s32     m_JumpAttackPercent; 
    f32     m_MinTimeBetweenJumpAttacks;
    f32     m_MaxTimeClosingToAttack;       // Max time to spend in a "CLOSE_FOR_..." state
    xbool   m_CanMelee;
    xbool   m_CanJumpAttack;
    eCoverDesire m_OurCoverDesire;
};

#endif
