///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_attack_aggressive.cpp
//
//      - implements an attack_aggressive state for an actor
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "ai_State_attack_aggressive.hpp"
#include "ai\brain.hpp"
#include "ai\sensory_record.hpp"


ai_state_attack_aggressive::ai_state_attack_aggressive(brain* myBrain ):
    ai_state_attack(myBrain)
{



}


ai_state_attack_aggressive::~ai_state_attack_aggressive()
{



}



void ai_state_attack_aggressive::OnAdvanceLogic( f32 deltaTime )
{
    CONTEXT( "ai_state_attack_aggressive::OnAdvanceLogic" );

    (void)deltaTime;


}



void ai_state_attack_aggressive::OnEnterState( void )
{



} 


xbool ai_state_attack_aggressive::OnAttemptExit( void )
{


    return true;
}


void ai_state_attack_aggressive::OnExitState( void )
{



}


    
void ai_state_attack_aggressive::OnInit( void )
{



}






///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

void ai_state_attack_aggressive::OnEnumProp( prop_enum&  List )
{
    ai_state_attack::OnEnumProp( List );


}


xbool ai_state_attack_aggressive::OnProperty( prop_query& I    )
{

    xbool returnVal = ai_state_attack::OnProperty(I);

    return returnVal;

}


