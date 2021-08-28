#ifndef __GRAY_ATTACK_STATE_HPP
#define __GRAY_ATTACK_STATE_HPP

#include "characters\basestates\character_attack_state.hpp"


class gray_attack_state : public character_attack_state
{

public:

    enum gray_attack_phases
    {
        PHASE_GRAY_ATTACK_SURPRISE = PHASE_BASE_COUNT,
        PHASE_GRAY_ENGAGE,                                  // Close to firing distance
        PHASE_GRAY_FIGHT_CROUCHED,                          // Fight while crouched
        PHASE_GRAY_FIGHT,                                   // Fire at player
        PHASE_GRAY_FIGHT_EVADE,                             // Evading
        PHASE_GRAY_FIGHT_STRAFE,                            // Strafe around while fighting
        PHASE_GRAY_FALLBACK_FACE_TARGET,                    // Fallback to firing distance
        PHASE_GRAY_FALLBACK_FACE_AWAY,                      // Fallback to firing distance
        PHASE_GRAY_FALLBACK_STOP_AND_LOOKAT_TARGET,         // After fallback-face-away, we want to stop and turn
        PHASE_GRAY_REACQUIRE_TARGET_GOTO_LAST_VIEWABLE,     // Reacquire a lost target - goto last place it could be seen from
        PHASE_GRAY_REACQUIRE_TARGET_GOTO_LAST_KNOWN_POS,    // Reacquire a lost target - goto last place it was seen at
        PHASE_GRAY_REACQUIRE_TARGET_GOTO_TARGET,            // Reacquire a lost target - goto target (cheat)        
        PHASE_GRAY_ATTACK_PACE,                             // Pace when enemy is not reachable
        PHASE_GRAY_HOLD_NO_TARGET,                          // Hold position
        PHASE_GRAY_ATTACK_GRENADE_PREP,                     // Orient and prep for a toss
        PHASE_GRAY_ATTACK_GRENADE_THROW,                    // Toss a GC
    };

// Functions
public:
                    gray_attack_state       ( character& Gray, states State ) ;
    virtual         ~gray_attack_state      ( void );
    virtual void    OnInit                  ( void );
    virtual void    OnEnter                 ( void ) ;
    virtual xbool   OnExit                  ( void ) ;
    virtual void    OnThink                 ( void );           
    virtual void    OnEnumProp              ( prop_enum&    List ) ;
    virtual xbool   OnProperty              ( prop_query&   rPropQuery ) ;
/*
    virtual xbool   OnPain                  ( const pain& Pain);
    virtual void    OnBeingShotAt           ( object::type Type, guid ShooterID );
    virtual void    OnHitByFriendly         ( guid ShooterID  );
    virtual void    OnHitFriendly           ( guid FriendlyID );
*/

    // character state specific virtual functions
    virtual s32     UpdatePhase             ( f32 DeltaTime );
    virtual void    ChangePhase             ( s32 newPhase );
    
    virtual character_state::states  UpdateState             ( f32 DeltaTime );

    // debugging information
    virtual const char*GetStateName ( void )    { return "CHARACTER_GRAY_ATTACK"; }    
    virtual const char*GetPhaseName ( s32 thePhase = PHASE_NONE );

#ifndef X_RETAIL
    virtual void    OnDebugRender           ( void );
#endif // X_RETAIL

protected:
    gray_attack_phases      ChoosePhase_TargetHidden        ( void );
    s32                     ValidateNewPhase    ( s32 NewPhase );
    void                    AdvanceTimers       ( f32 DeltaTime );
    //void                    PostUpdateCleanup   ( void );
    s32                     CheckForRequiredStrafing( void );
    void                    HandleShooting      ( void );
    s32                     CheckForFallback    ( void );
// Data
public:
    
    // Timers
    f32     m_Max_No_See_Time;

    s32     m_LastGrayPhase;
    xbool   m_LastPaceLeft;
    f32     m_CheckForCrouchTimer;
    
      
    f32     m_MinComfortDistance;
    f32     m_MaxComfortDistance;
    
    vector3 m_LastPlaceWhereTargetWasViewable;   
    
    xbool   m_bDebugRender;
    xbool   m_bGiveUpAndSearch;

    //
    //  Shooting
    //
    s32     m_iRoundsLeftToFire;
    f32     m_fNextRoundTimer;
    f32     m_fNextVolleyTimer;

    //
    //  Dynamic settings
    //
    vector3 m_WhereLocoShouldGo;
    f32     m_CurMinComfortDistance;
    f32     m_CurMaxComfortDistance;

    radian  m_rTargetViewYaw;

    //
    //  Strafe
    //
    xbool   m_StrafingLeft;
    f32     m_StrafeForwardBackModifier;     // normalized vector to target is scaled by this
                                            // amt and applied to the strafe vector.
    f32     m_FailedStrafeTimer;
    xbool   m_FailedStrafeLeft;

    //
    //  Evade
    //
    f32     m_EvadeTimer;
    f32     m_EvadeTimeBetween;
    f32     m_EvadeProb;

    //
    //  Grenade
    //
    f32     m_GrenadeTimer; 
/*
    //
    //
    //
    u32     m_bShot:1,
            m_bShotAt:1,
            m_bHitFriendly:1,
            m_bHitByFriendly:1;   
            */
};

#endif