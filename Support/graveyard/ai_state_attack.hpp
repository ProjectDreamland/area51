///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_attack.hpp
//
//      - attack contains all logic and data common to all combat states
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef AI_STATE_ATTACK_HPP
#define AI_STATE_ATTACK_HPP
#include "ai_state.hpp"

class ai_state_attack : public ai_state
{
public:

    enum firing_mode
    {
        FIRING_MODE_SINGLE_SHOT_SLOW =0,
        FIRING_MODE_SINGLE_SHOT_MEDIUM,
        FIRING_MODE_SINGLE_SHOT_FAST,
        FIRING_MODE_THREE_SHOT_BURSTS,
        FIRING_MODE_NONSTOP,
        FIRING_MODE_LAST,

        FIRING_MODE_FORCE_32BIT = 0xFFFFFFFF
    
    };



                                ai_state_attack(brain* myBrain = NULL);
    virtual                     ~ai_state_attack();

    virtual     void            OnAdvanceLogic( f32 deltaTime );

    virtual     void            OnEnterState( void ); 
    virtual     xbool           OnAttemptExit( void );
    virtual     void            OnExitState( void );
    
    virtual     void            OnInit( void );
    
    
    virtual     void            EvaluateTargets( void );

    virtual     void            UpdateDestination( f32 deltaTime );

    virtual     firing_mode     GetFiringMode( void );
    virtual     firing_mode     GetFiringModeFromString( const char* thisMode );
    virtual     const char*     GetFiringModeString( firing_mode thisMode );
    virtual     void            SetFiringMode( firing_mode thisMode );
    virtual     void            SetFiringMode( const char* thisMode );
    
    virtual     void            UpdateRangeToTarget( void );

    virtual     f32             GetRangeToTarget( void );

    virtual     void            AdvanceTowardsTarget(void);
    
    virtual     void            UpdateWeaponFiring(f32 deltaTime);
    virtual     xbool           AttemptShot( void );
    
    virtual     void            PickANewCombatSpot( f32 deltaTime );

    virtual     void            UpdateLookAtObject( f32 deltaTime );
    

    virtual     guid            GetCurrentTarget( void );



///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

    virtual     void            OnEnumProp( prop_enum&  List );
    virtual     xbool           OnProperty( prop_query& I    );


protected:
    
    guid        m_Target;

    f32         m_MinTimeBetweenTargetSwitch;
    f32         m_TimeSinceTargetSwitch;

    f32         m_MinRangeToTarget;
    f32         m_MaxRangeToTarget;
    f32         m_RangeToTarget;

    //  weapon firing
    f32         m_TimeSinceLastShot;
    s32         m_ShotsLeftToBurst;

    firing_mode m_FiringMode;

    emotion_controller::emotion_level m_EmotionLevelToSwitchToFear;
    emotion_controller::emotion_level m_EmotionLevelToSwitchToAngry;


    char        m_ExitStateTargetLost[32];
    char        m_ExitStateScared[32];
    char        m_ExitStateAngry[32];
    static char* m_FiringModeName[FIRING_MODE_LAST];
    
    f32         m_MoveWhileFiring;
    xbool       m_CombatMovement;
    f32         m_MaxDistanceToRunForEvade;
    
    xbool       m_TargetVisible;
    
    f32         m_FavorCover;

    f32         m_TimeToWatchTargetAfterLostLOS;
    
    f32         m_ChanceToMoveRelative;
    f32         m_MinTimeBetweenMoveChanges;
    f32         m_TimeSinceMoveChange;
    f32         m_TimeSinceTargetWasVisible;

};


#endif//AI_STATE_ATTACK_AGGRESSIVE_HPP