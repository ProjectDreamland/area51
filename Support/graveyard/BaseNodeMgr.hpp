///////////////////////////////////////////////////////////////////////////////////////////////////
// BaseNodeMgr.hpp
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef BASENODEMGR_HPP
#define BASENODEMGR_HPP

#include "base_node.hpp"
#include "base_connection.hpp"

class base_plan;

const s32 kMAX_STEPS = 32;
const s32 k_MAX_SEARCH_NODES = 128;
const s32 kMAX_CONNECTIONS = 2048;
const s32 kMAX_NAV_NODES   = 2048;

typedef s32 node_slot_id;

struct PathNode
{
    node_slot_id m_Node;
    node_slot_id m_Connection;
    f32     m_TotalCost;
    s32     m_SearchStep;
}; 


class basenode_mgr
{
public:

                            basenode_mgr            (void);
    virtual                 ~basenode_mgr           (void);

    xbool                   UpdatePlan              ( vector3& thisPoint, base_plan* thisPlan );

    node_slot_id            LocateNearestNode       ( vector3&  thisPoint );
    void				    LocateAllNodesInArea    ( bbox& Area, xarray<base_node*>& Nodes, u32 flags );
    virtual void            Init                    ( void );

    node_slot_id            GetFirstOpenConnection  ( void );
    
    node_slot_id            GetFirstOpenNode        ( void );
    
    void                    InsertNodeIntoList      ( node_slot_id thisNode, node_slot_id parent, node_slot_id connectionID  );
    
    virtual base_node*      GetNode                 ( node_slot_id NodeIndex );
    virtual base_node*      GetNextNode             ( void ) { return NULL; }
    virtual base_node*      GetPrevNode             ( void ) { return NULL; }

    virtual base_connection*GetConnection           ( node_slot_id ConnectionIndex );
    virtual base_connection*GetNextConnection       ( void ) { return NULL; }
    virtual base_connection*GetPrevConnection       ( void ) { return NULL; }
    
    virtual node_slot_id    GetNewNode              ( void ) = 0;
    virtual node_slot_id    GetNewConnection        ( void ) = 0;
    virtual void            DeleteNode              ( slot_id  thisNavNode    ) = 0;
    virtual void            DeleteConnection        ( slot_id  thisConnection ) = 0;

    void                    SetStartNodeIndex       ( node_slot_id StartNodeIndex ){ m_CurrentNode = StartNodeIndex; }
    void                    SetStartConnectionIndex ( node_slot_id StartConnectionIndex ){ m_CurrentConnection = StartConnectionIndex; }

    void                    FileIO                  ( fileio& File );
    void                    LoadMap                 (const char* fileName );
    void                    SaveMap                 (const char* fileName ); 

    void                    SanityCheck             (void);

//=========================================================================
//  Editor functions
//=========================================================================

    void                    EnableConnectionMode    ( void ) { m_NavConnectionModeEnabled = true;    }
    void                    DisableConnectionMode   ( void ) { m_NavConnectionModeEnabled = false;   }
    xbool                   GetConnectionMode       ( void ) { return m_NavConnectionModeEnabled; }

    void                    StartConnectionCreation ( node_slot_id firstPoint );
    void                    CompleteConnection      ( node_slot_id secondPoint );

   
    void                    AddConnectedNodesToOpenList( node_slot_id nearestNode );
    void                    AddGuidToConnection     ( guid aGuid, node_slot_id thisConnection);

    void                    SetTestStart            ( guid thisNode );
    void                    SetTestEnd              ( guid thisNode );

    virtual void            TestPath                 (void) = 0;

protected:
    node_slot_id        m_CurrentPath[kMAX_STEPS];
    vector3             m_Position;
    vector3             m_Destination;
    f32                 m_HorizontalNodeSearchRange ,
                        m_VerticalNodeSearchRange   ;
    s32                 m_SearchCount;                      // Holds the current search number
    s32                 m_SearchStepNumber;
    f32                 m_CheapestSolution;
    xbool               m_NavConnectionModeEnabled;
    PathNode            m_PathNodes[k_MAX_SEARCH_NODES];
    node_slot_id        m_CheapestNode;
    node_slot_id        m_CurrentNode;
    node_slot_id        m_CurrentConnection;

    node_slot_id             m_FirstNavNodeForConnection;
    node_slot_id             m_FirstSelectedObject;
    node_slot_id             m_TestStart;
    node_slot_id             m_TestEnd;
};


#endif
