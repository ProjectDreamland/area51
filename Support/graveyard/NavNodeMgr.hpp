///////////////////////////////////////////////////////////////////////////////////////////////////
// NavNodeMgr.hpp
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef NAVNODEMGR_HPP
#define NAVNODEMGR_HPP

#include "BaseNodeMgr.hpp"
#include "nav_node.hpp"
#include "nav_connection.hpp"

const s32   kNAV_MAP_VERSION_NUMBER = 100;

class navnode_mgr : public basenode_mgr
{
public:
    

                            navnode_mgr            (void);
    virtual                 ~navnode_mgr           (void);

    virtual void            Init                    ( void );
    
    virtual base_node*      GetNode                 ( node_slot_id NodeIndex );
    virtual base_node*      GetNextNode             ( void );
    virtual base_node*      GetPrevNode             ( void );

    virtual base_connection* GetConnection           ( node_slot_id ConnectionIndex );
    virtual base_connection* GetNextConnection       ( void );
    virtual base_connection* GetPrevConnection       ( void );

    virtual node_slot_id    GetNewNode              ( void );
    virtual node_slot_id    GetNewConnection        ( void );

    virtual void            DeleteNode              ( slot_id  thisNavNode    );
    virtual void            DeleteConnection        ( slot_id  thisConnection );

    virtual void            TestPath                 (void);

    virtual void            LoadMap                 (const char* fileName );
    virtual void            SaveMap                 (const char* fileName ); 

    virtual void            ResortNodes             ( void );
    virtual node_slot_id    LocateNearestNode       ( vector3&  thisPoint );


    // Patrole helper functions
    virtual node_slot_id    GetNearestPatrolNode    ( vector3 thisPoint );

    // Chooses new patrol node. CurrPatrolNode = node that npc is at, bForward = path search direction
    virtual node_slot_id    GetNextPatrolNode       ( node_slot_id CurrPatrolNode, xbool bForward = TRUE ) ;




protected:
    nav_connection      m_Connections[ kMAX_CONNECTIONS ];
    nav_node            m_NavNodes[ kMAX_NAV_NODES ];

    node_slot_id        m_SortedList[kMAX_NAV_NODES ];       // keep one for X and one for Y

    s32                 m_HighestNode;

};

extern navnode_mgr g_NavMgr;

#endif
