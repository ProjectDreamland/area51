///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  brain_stage_five.hpp
//
//      - brain specialization for a Stage 5 mutant
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef BRAIN_STAGE_FIVE__HPP
#define BRAIN_STAGE_FIVE__HPP

#include "brain.hpp"

class brain_stage_five : public brain
{
// Defines
public:
    enum state
    {
        STATE_PATROL,
        STATE_ALERT,
        STATE_ATTACK,
        STATE_SHOOT,
        STATE_DEATH,
    } ;


// Functions
public:

    // Constructor/destructor
                        brain_stage_five( npc* newOwner );
    virtual             ~brain_stage_five();

    // Virtual functions
    virtual void        Init                ( void );
    virtual void        OnRender            ( void ) ;
    virtual void        OnAdvanceLogic      ( f32 DeltaTime );
    virtual void        OnPain              ( const pain& Pain ) ;
    
private:
            // Helper functions
            void        ProcessAnimEvents   ( void ) ;
            void        UpdateTarget        ( f32 DeltaTime ) ;
            void        PathFind            ( f32 DeltaTime ) ;

            // State functions
            void        SetState            ( state State ) ;
            void        AdvanceState        ( f32 DeltaTime ) ;
                              
            // STATE_PATROL
            void        PatrolEnter         ( void ) ;
            void        PatrolExit          ( void ) ;
            void        PatrolAdvance       ( f32 DeltaTime ) ;
                           
            // STATE_ALERT
            void        AlertEnter          ( void ) ;
            void        AlertAdvance        ( f32 DeltaTime ) ;
            void        AlertExit           ( void ) ;
 
            // STATE_ATTACK
            void        AttackEnter         ( void ) ;
            void        AttackAdvance       ( f32 DeltaTime ) ;
            void        AttackExit          ( void ) ;
                           
            // STATE_SHOOT
            void        ShootEnter          ( void ) ;
            void        ShootAdvance        ( f32 DeltaTime ) ;
            void        ShootExit           ( void ) ;
                          
            // STATE_DEATH
            void        DeathEnter          ( void ) ;
            void        DeathAdvance        ( f32 DeltaTime ) ;
            void        DeathExit           ( void ) ;


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void        OnEnumProp( prop_enum&  List );
    virtual xbool       OnProperty( prop_query& I    );

// Data
protected:
        state           m_State ;               // Current state 
        f32             m_StateTime ;           // Time in current state
                                                
        node_slot_id    m_PatrolNode ;          // Current patrol node
                                                
        slot_id         m_TargetObject ;        // Object to attack
        vector3         m_TargetPos ;           // Target position to run to
        vector3         m_LookAtPos ;           // Position to look at
        xbool           m_FacingTarget ;        // TRUE if facing target
        f32             m_TargetMemoryTime ;    // How long enemy remembers target for    
                                                
        vector3         m_AlertPos ;            // Pos to go for when alerted
        f32             m_AlertTime ;           // Time to be alerted for*
                                                
        f32             m_ShootInterval ;       // Min time between next fire*
        f32             m_ShootTime ;           // Time before next fire
                                                
        s32             m_ShotCount ;           // Total shots to fire*
        f32             m_ShotInterval ;        // Interval between shots*
        f32             m_ShotTime ;            // Time before next fire
        s32             m_ShotRemain ;          // Shots remaining
        s32             m_ShotSide ;            // Current side shooting from
        
        f32             m_LeashRadius ;         // Limit from starting position*
        vector3         m_LeashPos ;            // Start position
                                                
        f32             m_SightRadius ;         // Sight distance*
        radian          m_SightFOV ;            // FOV in degrees*

        f32             m_TimeSinceLastUpdateTarget;    // Time since we looked for a new target
};



#endif//BRAIN_STAGE_FIVE__HPP 