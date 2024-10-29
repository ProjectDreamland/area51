#ifndef ASTAR_HPP
#define ASTAR_HPP

#include "Navigation\ng_node2.hpp"
#include "Navigation\ng_connection2.hpp"
#include "..\MiscUtils\PriorityQueue.hpp"
#include "AStarNode.hpp"


class astar_path_finder
{
    
public:
            astar_path_finder ( void );
    
//=========================================================================
//
// GeneratePath     -   Generates a path between two nodes.
// AddToOpen        -   Adds a node to the open list.
// CloseNode		-	Closes the specified node.
// RenderPath       -   Debug for Jim. Renders the path, open, and closed nodes.
//
//=========================================================================
            
    xbool   GeneratePath        ( ng_connection2*       pStartConnection,   // IN
                                  ng_connection2*       pEndConnection,     // IN
                                  const vector3&        DestPoint,          // IN
                                  guid                  NPCGuid,            // IN
                                  const pathing_hints&  Hints,              // IN
                                  s32*                  PathList,           // IN
                                  s32                   PathCount,          // IN
                                  s32&                  nStepsInPath );     // OUT
    
    void    AddToOpen           ( ng_connection2* pCurrentConnection, 
                                  const vector3&  CurrentPos,
                                  ng_connection2* pPrevConnection, 
                                  const vector3&  PrevPos,
                                  ng_connection2* pEndConnection, 
                                  s32             ParentIndex,
                                  f32             Multiplier );
    
    void    CloseNode           ( ng_node2* pNodeToClose ) ;
    void    ResetNumNodes       ( void ) ;

#ifdef X_EDITOR
    void    RenderPath      ( void );
#endif // X_EDITOR

protected:

    enum
    {
        MAX_OPEN_NODES = 100,       //maximum number of open nodes.
        MAX_NODES      = 1000,      //maximum number of nodes considered.
    };

    typedef priority_queue< nav_connection_slot_id, f32, MAX_OPEN_NODES > myqueue;    //priority queue typedef

protected:
    
//=========================================================================
//
// ResetNodeList     -   Resets the counters for the node list.
//
//=========================================================================
    
    f32     CalculateMultiplierForConnection( const ng_connection2&     Conn, 
                                              const pathing_hints&      Hints );

    void    ResetNodeList   ( void ) ;
    s32     CreatePath      ( s32* PathList, s32 PathCount );
protected:

    guid                m_CurrentNPCGuid;       //Guid of object that requested the path
    nav_node_slot_id    m_GoalNodeID;           //The ID of the node that is the end of the path.
    s32                 m_PathIndex;            //The index of the final node in the node list. Used for tracing the path back.
    vector3             m_DestPoint;

    myqueue             m_OpenList;             //The open list.
    astar_node          m_NodeList[MAX_NODES];  //The list of all nodes considered for the path.
    s32                 m_NumConnections;       //Number of connections that has been considered so far.
    
};

#endif