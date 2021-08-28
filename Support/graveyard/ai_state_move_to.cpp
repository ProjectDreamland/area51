///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_move_to.cpp
//
//      - implements a curious state where the AI will search for something
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "ai_state_move_to.hpp"
#include "ai\brain.hpp"
#include "ai\sensory_record.hpp"

ai_state_move_to::ai_state_move_to(brain* myBrain ):
    ai_state(myBrain)
{



}


ai_state_move_to::~ai_state_move_to()
{



}



void ai_state_move_to::OnAdvanceLogic( f32 deltaTime )
{
    CONTEXT( "ai_state_move_to::OnAdvanceLogic" );

    ai_state::OnAdvanceLogic(deltaTime);

    if( m_Brain->IsNewContactHostile() )
    {
        m_Brain->RequestStateChange( m_ExitStateAgro );
               

    }
    else if(m_Brain->IsNavPlanComplete() )
    {
        m_Brain->RequestStateChange(m_ExitStateNormal );
    }




}



void ai_state_move_to::OnEnterState( void )
{
    ai_state::OnEnterState();

    m_Brain->SetNewDestination(m_TargetPoint);

}


 


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

void ai_state_move_to::OnEnumProp( prop_enum&  List )
{

    ai_state::OnEnumProp( List );

    
 

    List.AddVector3(   xfs("AI State - %s\\Move to destination",m_CustomName),
                        "Destination for this move to state" );





}


xbool ai_state_move_to::OnProperty( prop_query& I    )
{
    xbool returnVal = ai_state::OnProperty(I);
    if(returnVal)
        return true;

    if( I.IsVar( xfs("AI State - %s\\Move to destination",m_CustomName) ) )
    {
        if( I.IsRead() )
        {

            I.SetVarVector3( m_TargetPoint );

        }
        else
        {
            m_TargetPoint =  I.GetVarVector3();

        }
        return true;
    }



    return returnVal;

}


