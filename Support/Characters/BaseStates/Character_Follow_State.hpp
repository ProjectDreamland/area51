#ifndef _CHARACTER_FOLLOW_STATE_HPP_
#define _CHARACTER_FOLLOW_STATE_HPP_

#include "Characters\CharacterState.hpp"
#include "Characters\ResponseList.hpp"

class character_follow_state : public character_state
{
// enums
public:
    enum base_follow_phases
    {
        PHASE_FOLLOW_IDLE = PHASE_BASE_COUNT,
        PHASE_FOLLOW_RUN_TO_TARGET,
        PHASE_FOLLOW_WALK_TO_TARGET,
    };

// Functions
public:
                    character_follow_state    ( character& ourCharacter, character_state::states State );
    virtual         ~character_follow_state   ( void );
    virtual void    OnInit                  ( void );
    virtual void    OnEnter                 ( void );
    virtual void    OnThink                 ( void );
    virtual void    OnEnumProp              ( prop_enum&    List ) ;
    virtual xbool   OnProperty              ( prop_query&   rPropQuery ) ;
    virtual void    OnBeingShotAt           ( object::type Type , guid ShooterID );

    virtual xbool       IgnorePain          ( const pain& Pain )        { (void)Pain; return m_ResponseList.HasFlags(response_list::RF_INVINCIBLE); }
    virtual xbool       IgnoreAlerts        ( void )                    { return m_ResponseList.HasFlags(response_list::RF_IGNORE_ALERTS); }    
    virtual xbool       IgnoreSight         ( void )                    { return m_ResponseList.HasFlags(response_list::RF_IGNORE_SIGHT); }    
    virtual xbool       IgnoreSound         ( void )                    { return m_ResponseList.HasFlags(response_list::RF_IGNORE_SOUND); }    
    virtual xbool       IgnoreAttacks       ( void )                    { return m_ResponseList.HasFlags(response_list::RF_IGNORE_ATTACKS); }    

    // character state specific virtual functions
    virtual s32     UpdatePhase             ( f32 DeltaTime );
    virtual void    ChangePhase             ( s32 newPhase );
    virtual character_state::states  UpdateState             ( f32 DeltaTime );

    // debugging information
    virtual const char*GetStateName ( void )    { return "CHARACTER_FOLLOW"; }    
    virtual const char*GetPhaseName ( s32 thePhase = PHASE_NONE );


protected:

    f32     m_FollowDistance;
    response_list m_ResponseList;
    xbool   m_SpottedSurpriseDone;
};

#endif
