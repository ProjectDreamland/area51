///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_curious.cpp
//
//      - implements a curious state where the AI will search for something
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "ai_state_curious.hpp"
#include "ai\brain.hpp"
#include "ai\sensory_record.hpp"

ai_state_curious::ai_state_curious(brain* myBrain ):
    ai_state(myBrain)
{



}


ai_state_curious::~ai_state_curious()
{



}



void ai_state_curious::OnAdvanceLogic( f32 deltaTime )
{
    CONTEXT( "ai_state_curious::OnAdvanceLogic" );

    ai_state::OnAdvanceLogic(deltaTime);


}



void ai_state_curious::OnEnterState( void )
{
    ai_state::OnEnterState();


}


xbool ai_state_curious::OnAttemptExit( void )
{
    

    return ai_state::OnAttemptExit();
}


void ai_state_curious::OnExitState( void )
{
    ai_state::OnExitState();


}


    
void ai_state_curious::OnInit( void )
{
    ai_state::OnInit();

    x_strcpy( m_ExitStateTargetFound, "None");


}


 


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

void ai_state_curious::OnEnumProp( prop_enum&  List )
{

    ai_state::OnEnumProp( List );

    List.AddEnum(   xfs("AI State - %s\\Exit States\\EXIT_TARGET_FOUND", m_CustomName),
                    m_Brain->GetAIStateEnumText(),
                    "Exit States for this AI",
                    PROP_TYPE_MUST_ENUM   );


}


xbool ai_state_curious::OnProperty( prop_query& I    )
{
    xbool returnVal = ai_state::OnProperty(I);
    if(returnVal)
        return true;

    if( I.IsVar( xfs("AI State - %s\\Exit States\\EXIT_TARGET_FOUND",m_CustomName) ) )
    {
        if( I.IsRead() )
        {

            I.SetVarEnum( m_ExitStateTargetFound );

        }
        else
        {
            x_strcpy(m_ExitStateTargetFound, I.GetVarEnum() );

        }
        return true;
    }
    




    return returnVal;

}


