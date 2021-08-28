///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_goal_list.cpp
//
//      - implements an goal_list state for an actor
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "ai_state_goal_list.hpp"
#include "brain.hpp"
#include "sensory_record.hpp"

ai_state_goal_list::ai_state_goal_list(brain* myBrain ):
    ai_state(myBrain)
{



}


ai_state_goal_list::~ai_state_goal_list()
{



}



void ai_state_goal_list::OnAdvanceLogic( f32 deltaTime )
{
    CONTEXT( "ai_state_goal_list::OnAdvanceLogic" );

    (void)deltaTime;
}



void ai_state_goal_list::OnEnterState( void )
{



}


xbool ai_state_goal_list::OnAttemptExit( void )
{


    return true;
}


void ai_state_goal_list::OnExitState( void )
{



}


    
void ai_state_goal_list::OnInit( void )
{



}


    


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

void ai_state_goal_list::OnEnumProp( prop_enum&  List )
{

    ai_state::OnEnumProp( List );



}


xbool ai_state_goal_list::OnProperty( prop_query& I    )
{
    xbool returnVal = ai_state::OnProperty(I);

    return returnVal;

}

