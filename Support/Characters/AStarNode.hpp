#ifndef ASTARNODE_HPP
#define ASTARNODE_HPP

#include "navigation\ng_node2.hpp"

struct astar_node
{
    astar_node( void );

    xbool                           IsOpen                  ( void );
    xbool                           IsClosed                ( void );

    f32                             GetCostAndEstimate      ( void );
    void                            UpdateCostAndEstimate   ( void );
    
    void                            SetOpen                 ( void );
    void                            SetClosed               ( void );
    void                            ResetNode               ( void );

    f32                             GetMovementCost         ( void );
    f32                             GetEstimate             ( void );

    void                            CalculateHeuristic      ( const vector3& GoalPoint );
    void                            CalculateMovementCost   ( const vector3& Position  );
    
    void                            Initialize              ( ng_connection2* pCurrentConnection,
                                                              const vector3&  CurrentPos,
                                                              ng_connection2* pPrevConnection,
                                                              const vector3&  PreviousPos,
                                                              ng_connection2* pEndConnection,
                                                              s32             ParentIndex,
                                                              const vector3&  DestPoint );

    ng_connection2*                 m_pNavConnection;         // actual node that this aStar Node represents
    vector3                         m_Position;

    ng_connection2*                 m_pParentConnection;      // parent of the node in the path.   
    vector3                         m_ParentPosition;

    s32                             m_ParentIndex;

    f32                             m_fMovementCost;
    f32                             m_fHeuristicEstimate;
    f32                             m_fCostAndEstimate;

};

#endif