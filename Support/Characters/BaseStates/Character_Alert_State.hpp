#ifndef _CHARACTER_ALERT_STATE_HPP_
#define _CHARACTER_ALERT_STATE_HPP_

#include "..\CharacterState.hpp"

class character_alert_state : public character_state
{
// enums
public:
    enum base_alert_phases
    {
        PHASE_ALERT_FACE_SOURCE = PHASE_BASE_COUNT,
        PHASE_ALERT_REACT_SOUND,
        PHASE_ALERT_IDLE,
    };

// Functions
public:
                    character_alert_state    ( character& ourCharacter, character_state::states State );
    virtual         ~character_alert_state   ( void );
    virtual void    OnInit                  ( void );
    virtual void    OnEnter                 ( void );
    virtual void    OnThink                 ( void );
    virtual void    OnEnumProp              ( prop_enum&    List ) ;
    virtual xbool   OnProperty              ( prop_query&   rPropQuery ) ;
    virtual xbool   IsAlert                 ( void )                        { return TRUE; }
    virtual void    OnBeingShotAt           ( object::type Type , guid ShooterID );
    virtual f32     GetAimerBlendScale      ( void )                      { return 0.6f; }

    // character state specific virtual functions
    virtual s32     UpdatePhase             ( f32 DeltaTime );
    virtual void    ChangePhase             ( s32 newPhase );
    virtual character_state::states  UpdateState             ( f32 DeltaTime );

    // debugging information
    virtual const char*GetStateName ( void )    { return "CHARACTER_ALERT"; }    
    virtual const char*GetPhaseName ( s32 thePhase = PHASE_NONE );

#ifndef X_RETAIL
    virtual void    OnDebugRender           ( void );
#endif // X_RETAIL

protected:
    f32                 m_TimeTillBored;
    xbool               m_bDebugRender;
};

#endif
