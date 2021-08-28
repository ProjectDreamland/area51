///////////////////////////////////////////////////////////////////////////////
//
//  base_plan.cpp
//
//
//
///////////////////////////////////////////////////////////////////////////////
#include "base_plan.hpp"
#include "base_node.hpp"
#include "base_connection.hpp"

//=========================================================================

base_plan::base_plan ( void )
{
    Reset();
}

//=========================================================================

base_plan::~base_plan ()
{

}
    
//=========================================================================

void base_plan::Reset( void )
{
    m_CompletePath = false;
    m_DestinationNode = SLOT_NULL;
//    m_Destination.Zero();
    s32 count;
    for(count = 0 ; count < kMAX_NAV_PLAN_CONNECTIONS ; count++)
    {
        m_NodePath[count] = SLOT_NULL;
        m_ConnectionPath[count] = SLOT_NULL;
    }
    m_CurrentGoal = 0;
}

//=========================================================================

node_slot_id  base_plan::GetStartingPoint    ( void )
{
    return m_NodePath[0];
}

//=========================================================================

void base_plan::SetStartingPoint ( node_slot_id thisPoint )
{
    Reset();
    m_NodePath[0] = thisPoint;
}

//=========================================================================

node_slot_id  base_plan::GetNextPoint        ( node_slot_id thisPoint )
{
    return m_NodePath[thisPoint+1];
    
    
//    s32 count;
//    for( count =0; count < kMAX_NAV_PLAN_CONNECTIONS -1; count++)
//    {
//        if( m_NodePath[count] == thisPoint)
//            return m_NodePath[count+1];
//        else if( m_NodePath[count] == SLOT_NULL )
//            return SLOT_NULL;
//    }

    return SLOT_NULL;

}

//=========================================================================

node_slot_id  base_plan::GetNextConnection   ( node_slot_id thisPoint )
{
    s32 count;
    for( count =0; count < kMAX_NAV_PLAN_CONNECTIONS -1; count++)
    {
        if( m_ConnectionPath[count] == thisPoint )
            return m_ConnectionPath[ count + 1 ];
        else if( m_ConnectionPath[count] == SLOT_NULL )
            return SLOT_NULL;
    }

    return SLOT_NULL;

}

//=========================================================================

node_slot_id  base_plan::GetLastPoint        ( void )
{
    return SLOT_NULL;
}

//=========================================================================

void base_plan::PathSanityCheck(void)          //  Checks the current path to make sure it's
{
    // Validate the base_plan here to verify all links connect, no doubling back and such
    
}

//=========================================================================

void  base_plan::AddPoint ( node_slot_id thisConnection, node_slot_id thisPoint, s32 atThisIndex)
{

    s32 count =0;

    if(atThisIndex == -1)    
    {
    

        while( count < (kMAX_NAV_PLAN_CONNECTIONS-1) && m_ConnectionPath[count] != SLOT_NULL )
        {
            count++;
        }
    }
    else
    {
        count = atThisIndex;
    }

    ASSERT(count >= 0) ;
    ASSERT(count < kMAX_NAV_PLAN_CONNECTIONS) ;

    m_ConnectionPath[count] = thisConnection;
    m_NodePath[count] = thisPoint;
}


//=========================================================================

xbool base_plan::ReachedPoint ( node_slot_id thisPoint )
{
    (void)thisPoint;

//    s32 count;
//    for( count =0; count < kMAX_NAV_PLAN_CONNECTIONS ; count++)
//    {
//        if( this->m_NodePath[count] == thisPoint )
//        {
///            m_CurrentGoal = count +1;
            m_CurrentGoal++;     
//        }
//    }

    if ( m_NodePath[m_CurrentGoal] == SLOT_NULL )
    {
        return true;
    }
    else
    {
        return false;
    }

}

