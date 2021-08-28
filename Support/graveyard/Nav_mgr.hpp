///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Nav_mgr.hpp
//
//  The nav_mgr is a singleton class that takes care of pullling all the navigation classes
//  together.  It also manages
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef NAV_MGR_HPP
#define NAV_MGR_HPP

#include "navigation\nav_node.hpp"
#include "navigation\nav_connection.hpp"
class nav_plan;
const s32 kMAX_STEPS = 16;
const s32 k_MAX_SEARCH_NODES = 128;
const s32 kMAX_CONNECTIONS = 200;
const s32 kMAX_NAV_NODES   = 200;

struct PathNode
{
    slot_id m_Node;
    slot_id m_Connection;
    f32     m_TotalCost;
    s32     m_SearchStep;
}; 


class nav_mgr
{
public:

                        nav_mgr(void);
    virtual             ~nav_mgr();

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  UpdateNavPlan       - Make a nav plan for reaching a destination
//
//  LocateNearestNode   - Get's the nearest node to this point.  used for picking a starting point
//
//  Init                - Initializes all the data structures for nav_mgr
//
//  GetConnection       - Gets a connection from the nav_mgr's internal list of connections
//
///////////////////////////////////////////////////////////////////////////////////////////////////


    xbool               UpdateNavPlan       ( vector3& thisPoint, nav_plan& thisPlan );

    slot_id             LocateNearestNode   ( vector3&  thisPoint );

    void				LocateAllNodesInArea ( bbox& Area, xarray<base_node*>& Nodes, u32 flags = nav_node::FLAG_ALL );
    virtual void        Init                ( void );

    nav_connection&     GetConnection       ( slot_id   thisConnection );
    slot_id             GetFirstOpenConnection ( void );
    
    nav_node&           GetNavNode          ( slot_id   thisNavNode );
    slot_id             GetFirstOpenNavNode ( void );

    void                DeleteNavNode        ( slot_id  thisNavNode    );
    void                DeleteConnection     ( slot_id  thisConnection );
    
    slot_id             GetNewNavNode       ( void );
    slot_id             GetNewConnection    ( void );

    void                InsertNodeIntoList( slot_id thisNode, slot_id parent, slot_id connectionID  );


    void                FileIO( fileio& File );
    void                LoadNavMap(const char* fileName );
    void                SaveNavMap(const char* fileName );

    void                SanityCheck(void);

///////////////////////////////////////////////////////////////////////////////////////////////////
    
    static nav_mgr*     GetNavMgr(void)     { if(!m_sThis) new nav_mgr; return m_sThis;  }


 

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor functions
///////////////////////////////////////////////////////////////////////////////////////////////////

    void                EnableNavConnectionMode ( void ) { m_NavConnectionModeEnabled = true;    }
    void                DisableNavConnectionMode( void ) { m_NavConnectionModeEnabled = false;   }
    xbool               GetNavConnectionMode ( void ) { return m_NavConnectionModeEnabled; }

    void                StartNavConnectionCreation ( slot_id firstPoint );
    void                CompleteNavConnection( slot_id secondPoint );

   
    void                AddConnectedNodesToOpenList( slot_id nearestNode );
    void                AddGuidToNavConnection( guid aGuid, slot_id thisConnection);

    void                SetNavTestStart( guid thisNode );
    void                SetNavTestEnd  ( guid thisNode );

    void                TestNav(void);

protected:
    slot_id             m_CurrentPath[kMAX_STEPS];
    vector3             m_Position;
    vector3             m_Destination;
    
    f32                 m_HorizontalNodeSearchRange ,
                        m_VerticalNodeSearchRange   ;

    static  nav_mgr*    m_sThis;

    nav_connection      m_Connections[ kMAX_CONNECTIONS ];
    nav_node            m_NavNodes[ kMAX_NAV_NODES ];

    s32                 m_SearchCount;                      // Holds the current search number
    s32                 m_SearchStepNumber;
    f32                 m_CheapestSolution;

    xbool               m_NavConnectionModeEnabled;



    PathNode            m_PathNodes[k_MAX_SEARCH_NODES];

    rhandle<nav_connection> m_hConnections;
    rhandle<nav_node>   m_hNavNodes;

    slot_id             m_CheapestNode;


//  Editor functionality


    slot_id             m_FirstNavNodeForConnection;
    slot_id             m_FirstSelectedObject;


    slot_id             m_NavTestStart;
    slot_id             m_NavTestEnd;



};


#endif//NAV_MGR_HPP
