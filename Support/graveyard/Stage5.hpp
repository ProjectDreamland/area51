//=========================================================================
//
//  Stage5.cpp
//
//=========================================================================

#ifndef __STAGE5_HPP__
#define __STAGE5_HPP__


//=========================================================================
// INCLUDES
//=========================================================================

#include "CharacterState.hpp"
#include "Character.hpp"
#include "Stage5Loco.hpp"

//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================

class stage5 ;
class stage5_state ;

//=========================================================================
// STAGE5 STATES
//=========================================================================

// Base state class
class stage5_state : public character_state
{
// Functions
public:
                 stage5_state   ( stage5& Stage5, states State ) ;

// Data
public:
    stage5&  m_Base ;  // Stage5 owner
} ;

//=========================================================================

// Idle
class stage5_idle_state : public stage5_state
{
// Functions
public:
                 stage5_idle_state  ( stage5& Stage5, states State ) ;
    virtual void OnInit             ( void ) ;
    virtual void OnEnter            ( void ) ;
    virtual void OnAdvance          ( f32 DeltaTime ) ;
    virtual void OnThink            ( void ) ;

// Data
public:
    f32     m_FidgetTime ;

} ;

//=========================================================================

// Patrol
class stage5_patrol_state : public stage5_state
{
// Functions
public:
                 stage5_patrol_state  ( stage5& Stage5, states State ) ;
    virtual void OnInit                ( void ) ;
    virtual void OnEnter               ( void ) ;
    virtual void OnAdvance             ( f32 DeltaTime ) ;
    virtual void OnThink               ( void ) ;
    virtual void OnRender              ( void ) ;

// Data
public:
    f32     m_FidgetTime ;
} ;

//=========================================================================

// Attack
class stage5_attack_state : public stage5_state
{
// Functions
public:
                 stage5_attack_state   ( stage5& Stage5, states State ) ;
    virtual void OnEnter               ( void ) ;
    virtual void OnAdvance             ( f32 DeltaTime ) ;
    virtual void OnThink               ( void ) ;
    virtual void OnRender              ( void ) ;

// Data
public:
    f32     m_OffsetTimer ;
    f32     m_ShootTimer ;
    f32     m_MeleeTimer ;
    f32     m_HowlTimer ;
    f32     m_BulletTimer ;
    s32     m_BulletSide ;
    s32     m_BulletCount ;
    vector3 m_Offset ;
} ;

//=========================================================================

// Flee
class stage5_flee_state : public stage5_state
{
// Functions
public:
                 stage5_flee_state   ( stage5& Stage5, states State ) ;
    virtual void OnEnter               ( void ) ;
    virtual void OnAdvance             ( f32 DeltaTime ) ;
    virtual void OnThink               ( void ) ;

// Data
public:
} ;

//=========================================================================

// Death
class stage5_death_state : public stage5_state
{
// Functions
public:
                 stage5_death_state   ( stage5& Stage5, states State ) ;
    virtual void OnEnter               ( void ) ;
    virtual void OnAdvance             ( f32 DeltaTime ) ;

// Data
public:

    f32         m_DeathCountdown;  
} ;


//=========================================================================
// STAGE5 CLASS
//=========================================================================

class stage5 : public character
{
// Real time type information
public:
    CREATE_RTTI( stage5, character, character )

friend class stage5_idle_state ;
friend class stage5_patrol_state ;
friend class stage5_attack_state ;
friend class stage5_flee_state ;
friend class stage5_death_state ;

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

//=========================================================================
// Defines
//=========================================================================
public:

//=========================================================================
// Class functions
//=========================================================================
public:
                    stage5 () ;
    virtual         ~stage5() ;

//=========================================================================
// Inherited virtual functions from base class
//=========================================================================
public:
    virtual void    OnInit          ( void ) ;
    virtual void    OnKill          ( void ) ;   
    virtual void    OnAdvanceLogic  ( f32 DeltaTime ) ;
    virtual void    OnRender        ( void ) ;
    virtual void    OnColCheck      ( void ) ;
    virtual xbool   OnEvent         ( const anim_event& Event, const vector3& WorldPos ) ;
    virtual void    OnPain          ( const pain& Pain ) ;

//=========================================================================
// Data
//=========================================================================
protected:

    // Locomotion
    stage5_loco             m_Loco ;

    // States
    stage5_idle_state       m_Idle ;
    stage5_patrol_state     m_Patrol ;
    stage5_attack_state     m_Attack ;
    stage5_flee_state       m_Flee ;
    stage5_death_state      m_Death ;

//=========================================================================
// Editor
//=========================================================================
protected:
    virtual void    OnEnumProp      ( prop_enum&    List ) ;
    virtual xbool   OnProperty      ( prop_query&   I    ) ;

};



//=========================================================================


#endif
