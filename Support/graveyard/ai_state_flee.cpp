///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_flee.cpp
//
//      - implements a curious state where the AI will search for something
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "ai_state_flee.hpp"
#include "ai\brain.hpp"
#include "ai\sensory_record.hpp"

ai_state_flee::ai_state_flee(brain* myBrain ):
    ai_state(myBrain)
{



}


ai_state_flee::~ai_state_flee()
{



}



void ai_state_flee::OnAdvanceLogic( f32 deltaTime )
{
    CONTEXT( "ai_state_flee::OnAdvanceLogic" );

    ai_state::OnAdvanceLogic(deltaTime);
}



void ai_state_flee::OnEnterState( void )
{

    ai_state::OnEnterState();

}


xbool ai_state_flee::OnAttemptExit( void )
{
    

    return ai_state::OnAttemptExit();;
}


void ai_state_flee::OnExitState( void )
{
    ai_state::OnExitState();


}


    
void ai_state_flee::OnInit( void )
{
    ai_state::OnInit();

    x_strcpy(m_ExitStateTargetLost,"None");


}


 


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

void ai_state_flee::OnEnumProp( prop_enum&  List )
{

    ai_state::OnEnumProp( List );

    List.AddEnum(   xfs("AI State - %s\\Exit States\\EXIT_TARGET_LOST", m_CustomName),
                    m_Brain->GetAIStateEnumText(),
                    "Exit States for this AI",
                    PROP_TYPE_MUST_ENUM   );


}


xbool ai_state_flee::OnProperty( prop_query& I    )
{
    xbool returnVal = ai_state::OnProperty(I);
    if(returnVal)
        return true;

    if( I.IsVar( xfs("AI State - %s\\Exit States\\EXIT_TARGET_LOST",m_CustomName) ) )
    {
        if( I.IsRead() )
        {

            I.SetVarEnum( m_ExitStateTargetLost );

        }
        else
        {
            x_strcpy(m_ExitStateTargetLost, I.GetVarEnum() );

        }
        return true;
    }

    return returnVal;

}


