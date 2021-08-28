///////////////////////////////////////////////////////////////////////////////
//
//
//  MutantAI.hpp
//
//
///////////////////////////////////////////////////////////////////////////////
#include "AIStateMachine.hpp"

#ifndef MUTANTAI_HPP
#define MUTANTAI_HPP

class mutant_ai : public ai_state_machine
{
public:
    enum mutant_base_activity
    {
        MUTANT_FIRST =0,
        MUTANT_GUARD,
        MUTANT_PATROL,
        MUTANT_WANDER,
        MUTANT_LAST,
        MUTANT_FORCE32BIT = 0xFFFFFFFF
    };



                            mutant_ai(slot_id thisActor);
    virtual                 ~mutant_ai();
                            
    virtual const char*     GetAIName               ( void ) { return ("mutant"); }
                            
    virtual void            OnAdvanceLogic          (f32 TimeDelta);
    virtual void            SetState                (AI_state newState);
                            
    virtual void            SetBaseActivity         ( mutant_base_activity thisActivity ) { m_BaseActivity = thisActivity; }
    
    virtual void            GotShot                 ( slot_id thisSlot, s32 Damage );

    virtual xbool           CheckForDamageUpdate    ( s32 thisAnim  , f32 thisFrame, f32 distanceToPlayer );


    
protected:
    virtual         void    AIStateAttack   ( f32 TimeDelta );       
    virtual         void    AIStateAware    ( f32 TimeDelta );       
    virtual         void    AIStateFlee     ( f32 TimeDelta );       
    virtual         void    AIStateIdle     ( f32 TimeDelta );       
    virtual         void    AIStateDead     ( f32 TimeDelta );

    mutant_base_activity    m_BaseActivity;
    xbool                   m_AppliedDamageThisAttack;
    xbool                   m_TimeToFlinch;

};

#endif

