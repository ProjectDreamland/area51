///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_attack_normal.cpp
//
//      - implements an attack_normal state for an actor
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "ai_state_attack_normal.hpp"
#include "ai\brain.hpp"
#include "ai\sensory_record.hpp"


ai_state_attack_normal::ai_state_attack_normal(brain* myBrain ):
    ai_state_attack(myBrain)
{



}


ai_state_attack_normal::~ai_state_attack_normal()
{



}



void ai_state_attack_normal::OnAdvanceLogic( f32 deltaTime )
{
    CONTEXT( "ai_state_attack_normal::OnAdvanceLogic" );

    (void)deltaTime;

    ai_state_attack::OnAdvanceLogic( deltaTime );
}



void ai_state_attack_normal::OnEnterState( void )
{
    

    ai_state_attack::OnEnterState();
}


xbool ai_state_attack_normal::OnAttemptExit( void )
{


    return ai_state_attack::OnAttemptExit();;
}


void ai_state_attack_normal::OnExitState( void )
{

    ai_state_attack::OnExitState();

}

 
    
void ai_state_attack_normal::OnInit( void )
{

    ai_state_attack::OnInit();

}


 


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

void ai_state_attack_normal::OnEnumProp( prop_enum&  List )
{

    ai_state_attack::OnEnumProp( List );

  

}


xbool ai_state_attack_normal::OnProperty( prop_query& I    )
{
    xbool returnVal = ai_state_attack::OnProperty(I);
    if(returnVal)
        return true;




    return returnVal;

}

