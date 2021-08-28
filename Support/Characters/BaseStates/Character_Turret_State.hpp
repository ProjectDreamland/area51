#ifndef _CHARACTER_TURRET_STATE_HPP_
#define _CHARACTER_TURRET_STATE_HPP_

#include "..\CharacterState.hpp"

class character_turret_state : public character_state
{
// enums
public:
    enum base_turret_phases
    {
        PHASE_TURRET_GOTO_TURRET = PHASE_BASE_COUNT,
        PHASE_TURRET_ALIGN_TO_MARKER,
        PHASE_TURRET_ENTER_TURRET,
        PHASE_TURRET_IDLE,
        PHASE_TURRET_FIRE,
        PHASE_TURRET_RELOAD,
        PHASE_TURRET_EXIT_TURRET,
    };

// Functions
public:
                    character_turret_state    ( character& ourCharacter, character_state::states State );
    virtual         ~character_turret_state   ( void );
    virtual void    OnInit                  ( void );
    virtual void    OnEnter                 ( void );
    virtual xbool   OnExit                  ( void );
    virtual void    OnThink                 ( void );
    virtual void    OnEnumProp              ( prop_enum&    List ) ;
    virtual xbool   OnProperty              ( prop_query&   rPropQuery ) ;
    virtual xbool   OnPain                  ( const pain& Pain );
    virtual xbool   IgnoreAlerts            ( void )                { return TRUE; }    
    virtual xbool   IgnoreSight             ( void )                { return TRUE; }
    virtual xbool   IgnoreSound             ( void )                { return TRUE; }    
    virtual xbool   IgnoreAttacks           ( void )                { return TRUE; }    
    virtual xbool   IgnoreFlinches          ( void )                { return TRUE; }    
    virtual xbool   IgnoreFalling           ( void )                { return TRUE; }    

    // character state specific virtual functions
    virtual s32     UpdatePhase             ( f32 DeltaTime );
    virtual void    ChangePhase             ( s32 newPhase );
    virtual character_state::states  UpdateState             ( f32 DeltaTime );

    // debugging information
    virtual const char*GetStateName ( void )    { return "CHARACTER_TURRET"; }    
    virtual const char*GetPhaseName ( s32 thePhase = PHASE_NONE );

            void    SetCurrentTurret( guid ourTurret );
protected:
    anim_group::handle m_TurretAnimGroupHandle;
    xbool           m_EndState;
    guid            m_CurrentTurret;
};

#endif
