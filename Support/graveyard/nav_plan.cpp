///////////////////////////////////////////////////////////////////////////////
//
//  nav_plan.cpp
//
//
//
///////////////////////////////////////////////////////////////////////////////
#include "nav_plan.hpp"
#include "NavNodeMgr.hpp"

//=========================================================================

nav_plan::nav_plan ( void ) : base_plan()
{
   
}

//=========================================================================
 
nav_plan::~nav_plan( void )
{

}
   
//=========================================================================

void nav_plan::SetDestination ( vector3 newDestination )
{
    if( (newDestination - m_Destination ).LengthSquared() < 2500.0f )
    {
//      ASSERT(tempDestinationNode != -1 );
        return;
    }

    Reset();
    m_CompletePath = false;
    m_CurrentGoal = 0;
    m_Destination = newDestination;

    node_slot_id tempDestinationNode = g_NavMgr.LocateNearestNode( m_Destination );
    if(tempDestinationNode == SLOT_NULL )
    {
//        x_try("No nearest node found.  No nav map created possibly?");
        return;
    }
//    ASSERT(tempDestinationNode != SLOT_NULL );
    m_DestinationNode = tempDestinationNode;

}

//=========================================================================

vector3 nav_plan::GetCurrentGoalPoint ( void )
{

    if(m_NodePath[m_CurrentGoal] == SLOT_NULL )
    {
        return m_Destination;
    }
    else
    {
    
        return ((g_NavMgr.GetNode( m_NodePath[m_CurrentGoal] ))->GetPosition());
    }

}

//=========================================================================

vector3 nav_plan::GetLastNodePoint    ( void )
{
    ASSERT(m_NodePath[0] != SLOT_NULL );

    s32 count =0;
    while( count < (kMAX_NAV_PLAN_CONNECTIONS-1) && m_NodePath[count] != SLOT_NULL )
    {
        ++count;
    }
    //  found the one after the last one, now back up one
    count--;

    ASSERT(count >= 0) ;
    ASSERT(count < kMAX_NAV_PLAN_CONNECTIONS) ;

    base_node* pthisNode = g_NavMgr.GetNode( m_NodePath[count] );
    vector3 aVector;
    aVector = pthisNode->GetPosition();

    return aVector;


}

//=========================================================================
