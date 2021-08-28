///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_attack_passive.cpp
//
//      - implements an attack_passive state for an actor
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "ai_state_attack_passive.hpp"
#include "ai\brain.hpp"
#include "ai\sensory_record.hpp"

ai_state_attack_passive::ai_state_attack_passive(brain* myBrain ) :
    ai_state_attack(myBrain)
{



}


ai_state_attack_passive::~ai_state_attack_passive()
{



}



void ai_state_attack_passive::OnAdvanceLogic( f32 deltaTime )
{
    CONTEXT( "ai_state_attack_passive::OnAdvanceLogic" );

    (void)deltaTime;


}



void ai_state_attack_passive::OnEnterState( void )
{
 


}


xbool ai_state_attack_passive::OnAttemptExit( void )
{


    return true;
}


void ai_state_attack_passive::OnExitState( void )
{



}


    
void ai_state_attack_passive::OnInit( void )
{



}


    




///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

void ai_state_attack_passive::OnEnumProp( prop_enum&  List )
{

    ai_state_attack::OnEnumProp( List );


}


xbool ai_state_attack_passive::OnProperty( prop_query& I    )
{
    xbool returnVal = ai_state_attack::OnProperty(I);

    return returnVal;

}



