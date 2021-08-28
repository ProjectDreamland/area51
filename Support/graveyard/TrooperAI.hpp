///////////////////////////////////////////////////////////////////////////////
//
//
//
//
///////////////////////////////////////////////////////////////////////////////
#include "AIStateMachine.hpp"

#ifndef TROOPERAI_HPP
#define TROOPERAI_HPP

class trooper_ai : public ai_state_machine
{

public:
                        trooper_ai(slot_id thisActor);
    virtual             ~trooper_ai();

    virtual const char* GetAIName( void ) { return ("trooper"); }

    virtual void        OnAdvanceLogic(f32 TimeDelta);
    virtual void        SetState(AI_state newState);

    virtual void        GotShot( slot_id thisSlot, s32 Damage );
    
protected:
    virtual         void    AIStateAttack          ( f32 DeltaTime );
    virtual         void    AIStateAware           ( f32 DeltaTime );
    virtual         void    AIStateFlee            ( f32 DeltaTime );
    virtual         void    AIStateIdle            ( f32 DeltaTime );
    virtual         void    AIStateDead            ( f32 DeltaTime );




};


#endif//TROOPERAI_HPP