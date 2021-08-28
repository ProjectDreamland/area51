///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_patrol.cpp
//
//      - implements a curious state where the AI will search for something
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "ai_state_patrol.hpp"
#include "ai\brain.hpp"
#include "ai\sensory_record.hpp"
#include "navigation\NavNodeMgr.hpp"
ai_state_patrol::ai_state_patrol(brain* myBrain ):
    ai_state(myBrain),
    m_bForward(true)
{

}


ai_state_patrol::~ai_state_patrol()
{



}



void ai_state_patrol::OnAdvanceLogic( f32 deltaTime )
{
    CONTEXT( "ai_state_patrol::OnAdvanceLogic" );

    ai_state::OnAdvanceLogic(deltaTime);

    if( m_Brain->IsNewContactHostile() )
    {
        m_Brain->RequestStateChange( m_ExitStateAgro );
               

    }
    else if(m_Brain->IsNavPlanComplete() )
    {        
        m_Brain->GetNextPatrolPointSlot( m_CurrentPatrolNode, m_bForward  );
        m_Brain->SetNewDestination( g_NavMgr.GetNode(m_CurrentPatrolNode)->GetPosition() );
    }
}



void ai_state_patrol::OnEnterState( void )
{
    ai_state::OnEnterState();

    m_CurrentPatrolNode = m_Brain->GetNearestPatrolNode();

    base_node* thisNode = g_NavMgr.GetNode( m_CurrentPatrolNode );

    ASSERT(thisNode);

    vector3 aVector = thisNode->GetPosition();

    m_Brain->SetNewDestination( aVector );

}


