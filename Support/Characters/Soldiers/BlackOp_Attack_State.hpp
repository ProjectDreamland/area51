#ifndef _BLACKOP_ATTACK_STATE_HPP_
#define _BLACKOP_ATTACK_STATE_HPP_

#include "Characters\BaseStates\Character_Attack_State.hpp"

class blackOp_attack_state : public character_attack_state
{
// enums
public:

    enum blackOp_attack_phases
    {
        PHASE_BO_ATTACK_STRAFE_LEFT = PHASE_BASE_COUNT,
        PHASE_BO_ATTACK_STRAFE_RIGHT,
        PHASE_BO_ATTACK_STAND_SHOOT,
        PHASE_BO_ATTACK_ENTER_CROUCH_SHOOT,
        PHASE_BO_ATTACK_CROUCH_SHOOT,
        PHASE_BO_ATTACK_EXIT_CROUCH_SHOOT,
        PHASE_BO_ATTACK_ADVANCE,
        PHASE_BO_ATTACK_BACKUP,
        PHASE_BO_ATTACK_ALIGN_FOR_GRENADE_THROW,
        PHASE_BO_ATTACK_THROW_GRENADE,
        PHASE_BO_ATTACK_CLOSE_FOR_MELEE,
        PHASE_BO_ATTACK_ALIGN_FOR_MELEE,
        PHASE_BO_ATTACK_MELEE,
        PHASE_BO_ATTACK_MELEE_BACK_RIGHT,
        PHASE_BO_ATTACK_MELEE_BACK_LEFT,
        PHASE_BO_ATTACK_GOTO_CORPSE,
        PHASE_BO_ATTACK_DRAIN_CORPSE,
        PHASE_BO_ATTACK_COVER_ME,
        PHASE_BO_ATTACK_REQUEST_ATTACK,
        PHASE_BO_ATTACK_COUNT 
    };

    enum eTargetRangeBrackets
    {
        TARGET_RANGE_FURTHER_THAN_OPTIMAL,
        TARGET_RANGE_OPTIMAL,
        TARGET_RANGE_CLOSER_THAN_OPTIMAL,
        TARGET_RANGE_WITHIN_MELEE_POSSIBILITY,
    };

    // Functions
public:
                    blackOp_attack_state    ( character& ourCharacter, character_state::states State );
    virtual        ~blackOp_attack_state    ( void );
    virtual void    OnInit                  ( void );
    virtual void    OnEnter                 ( void );
    virtual xbool   OnExit                  ( void );
    virtual void    OnThink                 ( void );
    virtual void    OnEnumProp              ( prop_enum&    List ) ;
    virtual xbool   OnProperty              ( prop_query&   rPropQuery ) ;
    virtual xbool   OnPain                  ( const pain& Pain);
    virtual void    OnBeingShotAt           ( object::type Type, guid ShooterID );
    virtual void    OnHitByFriendly         ( guid ShooterID  );
    virtual void    OnHitFriendly           ( guid FriendlyID );
    virtual void    AutofireRequestSent     ( void )            { m_AutofireRequestSent = TRUE; }
    virtual xbool   CanReload               ( void )            { return GetCurrentPhase() != PHASE_BO_ATTACK_COVER_ME; }       

    // character state specific virtual functions
    virtual s32     UpdatePhase             ( f32 DeltaTime );
    virtual void    ChangePhase             ( s32 newPhase );
    virtual character_state::states  UpdateState             ( f32 DeltaTime );

    // debugging information
    virtual const char*GetStateName ( void )    { return "BLACKOPPS_ATTACK"; }    
    virtual const char*GetPhaseName ( s32 thePhase = PHASE_NONE );

protected:
    s32             GetNextPhaseNormal      ( void );

    s32             GetTargetVisiblePhase   ( void );
    s32             GetTargetHiddenPhase    ( void );
    s32             GetForcedRethinkPhase   ( void );
    s32             GetAnyMovementPhase     ( void );
    s32             GetEvadePhase           ( void );
    s32             GetStrafePhase          ( void );

    eTargetRangeBrackets GetTargetRangeBracket( void );

    // this is the distance we wish to maintain to our target
    f32 m_OptimalFiringDistance;

    f32 m_MinPhaseTime;

    f32 m_TimeSinceLastRethink;
    f32 m_TimeSinceLastMove;

    xbool m_ForceRethink;
    xbool m_ForceRangeRethink;
    xbool m_CouldSeeTarget;
    xbool m_AutofireRequestSent;

    eTargetRangeBrackets m_TargetRangeBracket;

    s32 m_EvadePercent;
    s32 m_StandShootPercent;
    s32 m_PainStrafePercent;
    s32 m_AfterExitPhase;

};

#endif // 
