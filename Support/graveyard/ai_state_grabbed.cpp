///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_grabbed.cpp
//
//      - implements a curious state where the AI will search for something
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "ai_state_grabbed.hpp"
#include "ai\brain.hpp"
#include "ai\sensory_record.hpp"
#include "objects\npc.hpp"

ai_state_grabbed::ai_state_grabbed(brain* myBrain ):
    ai_state(myBrain)
{



}


ai_state_grabbed::~ai_state_grabbed()
{



}



void ai_state_grabbed::OnAdvanceLogic( f32 deltaTime )
{
    CONTEXT( "ai_state_grabbed::OnAdvanceLogic" );

    (void)deltaTime;
}


    
void ai_state_grabbed::OnEnterState( void )
{
    m_Brain->DisableMovement();
//    m_Brain->GetOwner()->GetLocomotion()->PlayAnimState( base_loco::AT_DEATH );

}


xbool ai_state_grabbed::OnAttemptExit( void )
{


    return true;
}


void ai_state_grabbed::OnExitState( void )
{



}


    
void ai_state_grabbed::OnInit( void )
{



}


 


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

void ai_state_grabbed::OnEnumProp( prop_enum&  List )
{

    ai_state::OnEnumProp( List );





}


xbool ai_state_grabbed::OnProperty( prop_query& I    )
{
    xbool returnVal = ai_state::OnProperty(I);
    if(returnVal)
        return true;

 


    return returnVal;

}


