#ifndef __MUTANTTANK_ATTACK_STATE_HPP
#define __MUTANTTANK_ATTACK_STATE_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "characters\basestates\character_attack_state.hpp"



//=========================================================================
// ATTACK CLASS
//=========================================================================
class mutanttank_attack_state : public character_attack_state
{

//=========================================================================
// Defines
//=========================================================================
public:

    // Phases
    enum mutanttank_attack_phases
    {
        PHASE_MUTANTTANK_STAGE_RAGE = PHASE_BASE_COUNT,
        
        PHASE_MUTANTTANK_ATTACK_CHASE, 
        PHASE_MUTANTTANK_ATTACK_CHASE_EVADE, 
        
        PHASE_MUTANTTANK_ATTACK_CHARGE_ALIGN,
        PHASE_MUTANTTANK_ATTACK_CHARGE,
        PHASE_MUTANTTANK_ATTACK_CHARGE_SWING,
        PHASE_MUTANTTANK_ATTACK_CHARGE_MISS,
        
        PHASE_MUTANTTANK_ATTACK_MELEE_ALIGN,
        PHASE_MUTANTTANK_ATTACK_MELEE,
        
        PHASE_MUTANTTANK_ATTACK_LEAP_ALIGN,
        PHASE_MUTANTTANK_ATTACK_LEAP,
        
        PHASE_MUTANTTANK_ATTACK_STEP_BACK,
        PHASE_MUTANTTANK_ATTACK_PACE,
        
        PHASE_MUTANTTANK_ATTACK_RANGED_ALIGN,
        PHASE_MUTANTTANK_ATTACK_RANGED_ATTACK,
        
        PHASE_MUTANTTANK_ATTACK_BORED,
        PHASE_MUTANTTANK_ATTACK_SURPRISED,
        
        PHASE_MUTANTTANK_ATTACK_BUBBLE_ALIGN,
        PHASE_MUTANTTANK_ATTACK_BUBBLE,
        
        PHASE_MUTANTTANK_ATTACK_CANISTER_TO,
        PHASE_MUTANTTANK_ATTACK_CANISTER_IDLE,
        PHASE_MUTANTTANK_ATTACK_CANISTER_SMASH,
        PHASE_MUTANTTANK_ATTACK_CANISTER_FROM,
                
        PHASE_MUTANTTANK_ATTACK_CROUCH_PERCH_HOPPOINT,
        PHASE_MUTANTTANK_ATTACK_JUMP_PERCH_HOPPOINT,
        PHASE_MUTANTTANK_ATTACK_CROUCH_PERCH,
        PHASE_MUTANTTANK_ATTACK_JUMP_PERCH,
        PHASE_MUTANTTANK_ATTACK_PERCH_ROAR,
        PHASE_MUTANTTANK_ATTACK_FROM_PERCH,

        PHASE_MUTANTTANK_ATTACK_CROUCH_GRATE_HOPPOINT,
        PHASE_MUTANTTANK_ATTACK_JUMP_GRATE_HOPPOINT,
        PHASE_MUTANTTANK_ATTACK_CROUCH_GRATE,
        PHASE_MUTANTTANK_ATTACK_JUMP_GRATE,
        PHASE_MUTANTTANK_ATTACK_GRATE_SMASH,
        PHASE_MUTANTTANK_ATTACK_FROM_GRATE,

        PHASE_MUTANTTANK_ATTACK_REGEN_SHIELD,
        
        PHASE_MUTANTTANK_ATTACK_CONTAGION,
    };

    // Misc defines
    enum defines
    {
        MAX_STAGE_COUNT = 4,
    };
    
    // Type
    enum type
    {
        TYPE_A,
        TYPE_E3,    // Excavation 3
        TYPE_AB45,  // AlienBase 4-5
        
        TYPE_COUNT
    };
    
//=========================================================================
// Structures
//=========================================================================
public:

    // Stage structure
    struct stage
    {
        // Data
        f32 m_Health;                       // Health for this stage
        
        f32 m_MoveSpeed;                    // 0 = Walk, 1 = Run
        
        f32 m_ChaseMinTime;                 // Chase time before attacks are allowed
        
        f32 m_EvadeInterval;                // Time between evades ( -1 = none )
        
        f32 m_ChargeInterval;               // Time between charges ( -1 = none )
        f32 m_ChargeToMaxSpeedTime;         // Time to obtain max speed
        f32 m_ChargeMaxSpeed;               // 0 = Normal, 1 = Fast
        f32 m_ChargeMinDist;                // Closest distance for a charge
        f32 m_ChargeMaxDist;                // Furthest distance for a charge
        
        f32 m_RangedInterval;               // Timer between mason cannon ( -1 = none )
        f32 m_RangedMinDist;                // Closest distance ranged can be performed
        f32 m_RangedMaxDist;                // Furthest distance ranged can be performed
        
        f32 m_LeapInterval;                 // Timer between leaping ( -1 = none )
        f32 m_LeapMinDist;                  // Closest distance for a leap
        f32 m_LeapMaxDist;                  // Furthest distance for a leap
        
        f32 m_MeleeInterval;                // Timer between melee ( -1 = none )
        
        f32 m_BubbleHealthInterval;         // Health interval between bubble attacks ( -1 = none )
        
        f32 m_CanisterHealthInterval;       // Health interval between canister attacks ( -1 = none )
        f32 m_CanisterIdleTime;             // Idle time on canister before smashing it
        
        f32 m_ParasiteShieldRegenInterval;  // Time between parasite shield regenerations

        f32 m_ContagionHealthInterval;      // Health interval between contagion attacks ( -1 = none )

        f32 m_JumpToPerch;                  // Do we jump to the perch? -1 no 1 yes.
        f32 m_JumpToGrate;                  // Do we jump to the grating? -1 no 1 yes.
        
        // Functions
                stage();
        void    LoadTweaks  ( const xstring& Prefix );
    };
    
    
//=========================================================================
// Functions
//=========================================================================
public:
        // Constructor/destructor                
                    mutanttank_attack_state     ( character& Mutant, states State ) ;
virtual             ~mutanttank_attack_state    ( void );

        // Stage setup functions
        void        LoadTweaks  ( void );

        // State functions
virtual void        OnEnter      ( void ) ;
virtual xbool       OnExit       ( void ) ;
virtual states      UpdateState  ( f32 DeltaTime );
virtual const char* GetStateName ( void )    { return "CHARACTER_MUTANTTANK_ATTACK"; }    
virtual void        OnBeingShotAt( object::type Type , guid ShooterID );
virtual xbool       OnPain       ( const pain& Pain );
virtual xbool       IgnorePain   ( const pain& Pain );
        // Phase functions
virtual s32         UpdatePhase  ( f32 DeltaTime );
virtual void        ChangePhase  ( s32 newPhase );
virtual const char* GetPhaseName ( s32 thePhase = PHASE_NONE );

        // Property functions
virtual void        OnEnumProp   ( prop_enum&    List ) ;
virtual xbool       OnProperty   ( prop_query&   rPropQuery ) ;

        // Debug functions
#ifndef X_RETAIL
virtual void        OnDebugRender   ( void );
#endif // X_RETAIL


protected:
        // Internal functions
        s32         ChooseNextPhase();
        xbool       IsCurrentPhaseOverride();
//=========================================================================
// Data
//=========================================================================
public:

        // Flags
        xbool       m_LastPaceLeft;                 // Should theta pace left or right?
        xbool       m_bSwitchOutOfAttack;           // Should theta switch out of attack state
        xbool       m_bWantToEvade;                 // Should theta play an evade anim?
        xbool       m_PerchJumped;
        xbool       m_GrateJumped;

        // Timers
        f32         m_ChaseTimer;                   // Time theta has been running
        f32         m_EvadeTimer;                   // Time since last evade
        f32         m_LeapTimer;                    // Time since last leap
        f32         m_MeleeTimer;                   // Time since last melee
        f32         m_RangedTimer;                  // Time since last ranged attack
        f32         m_ChargeTimer;                  // Time since last charge
        f32         m_RegenShieldTimer;             // Time since last parasite shield regeneration
        
        // Misc
        f32         m_ChargeSpeed;                  // Current charge speed
        vector3     m_ChargeLineStart;              // Start of charge position
        vector3     m_ChargeLineEnd;                // End of charge position
        f32         m_BubbleHealth;                 // Health before next bubble attack
        f32         m_StageRageHealth;              // Health before next stage rage
        f32         m_CanisterHealth;               // Health before next canister attack
        s32         m_iDestCanister;                // Index of canister to jump to
        f32         m_GroundYPos;                   // Y position of ground
        f32         m_ContagionHealth;              // Health before next contagion attack
        
        // Stage vars
        type        m_Type;                         // Type of boss
        stage       m_Stages[ MAX_STAGE_COUNT ];    // List of stages
        s32         m_nStages;                      // # of stages
        s32         m_CurrentStage;                 // Current stage

        guid        m_CurrentHopPoint;              // marker we are going to hop to.
        
//=========================================================================
// Friends
//=========================================================================
friend class mutant_tank;
        
} ;

#endif