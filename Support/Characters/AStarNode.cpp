#include "AStarNode.hpp"

//===========================================================================

astar_node::astar_node( void ) :
    m_pNavConnection( NULL ),
    m_pParentConnection( NULL ),
    m_ParentIndex( -1 ),
    m_fMovementCost( 0.f ),
    m_fHeuristicEstimate( 0.f ),
    m_fCostAndEstimate( 0.f )
{
}

//===========================================================================

void astar_node::Initialize( ng_connection2* pCurrentConnection, 
                             const vector3&  CurrentPos,
                             ng_connection2* pPrevConnection,
                             const vector3&  PrevPos,
                             ng_connection2* pEndConnection,
                             s32             ParentIndex,
                             const vector3&  DestPoint )
{
    //set the navigation node pointers
    m_pNavConnection    = pCurrentConnection;
    m_Position          = CurrentPos;
    m_pParentConnection = pPrevConnection;
    m_ParentPosition    = PrevPos;
    m_ParentIndex       = ParentIndex;
    (void)pEndConnection;

    //calculate the movement cost from the current node to the previous node
    CalculateMovementCost( CurrentPos );
    CalculateHeuristic   ( DestPoint  );
    UpdateCostAndEstimate();
}

//===========================================================================

void astar_node::ResetNode( void )
{
    m_pNavConnection->ResetPathingFlags();
}

//===========================================================================

void astar_node::SetOpen( void )
{
    ASSERT( m_pNavConnection );
    m_pNavConnection->SetOpen();
}

//===========================================================================

void astar_node::SetClosed( void )
{
    ASSERT( m_pNavConnection );
    m_pNavConnection->SetClosed();
}

//===========================================================================

xbool astar_node::IsOpen( void )
{
    ASSERT( m_pNavConnection );
    return m_pNavConnection->IsOpen();
}

//===========================================================================

xbool astar_node::IsClosed( void )
{
    ASSERT( m_pNavConnection );
    return m_pNavConnection->IsClosed();
}


//===========================================================================

f32 astar_node::GetCostAndEstimate( void )
{
    return m_fCostAndEstimate;
}

//===========================================================================

void astar_node::UpdateCostAndEstimate( void )
{
    m_fCostAndEstimate =  m_fMovementCost + m_fHeuristicEstimate;
}

//===========================================================================

void astar_node::CalculateHeuristic( const vector3& GoalPoint )
{
    //for now, the heuristic will be the square of the distance from the current node to the goal node
    m_fHeuristicEstimate = ( m_Position - GoalPoint ).LengthSquared();
}

//===========================================================================

void astar_node::CalculateMovementCost( const vector3& Position )
{
    //cost for now will be the square of the distance between the nodes.
    m_fMovementCost = ( Position - m_ParentPosition ).LengthSquared();   
}


