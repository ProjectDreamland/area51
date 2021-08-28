///////////////////////////////////////////////////////////////////////////////
//
//  base_node.cpp
//
//      base_node is the base class that other node inherit from.
//
//
/////////////////////////////////////////////////////////////////////////////// 
#ifndef BASE_NODE_HPP
#define BASE_NODE_HPP

#include "x_math.hpp"
#include "x_debug.hpp"
#include "MiscUtils\RTTI.hpp"
#include "obj_mgr\obj_mgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "miscutils\fileio.hpp"

const s32 k_MAX_NODE_CONNECTIONS = 8;

typedef s32 node_slot_id;


class base_node 
{
public:

    enum flags
    {

    };



                            base_node(void);
    virtual                 ~base_node();

    virtual     void        OnRender( void );

#ifndef X_RETAIL
    virtual     void        OnDebugRender(void);
#endif // X_RETAIL

    virtual void            AddConnection               ( node_slot_id newConnection );    
    virtual void            Reset                       ( void );
    virtual void            Update                      ( void );               



    virtual void            DeleteConnection            ( node_slot_id thisConnection );

    virtual node_slot_id    GetConnectionNearestPoint   ( vector3 thisPoint );
    virtual node_slot_id    GetConnectionByIndex        ( s32  thisConnection ) ;
    virtual s32             GetConnectionCount          ( void );

    virtual xbool           IsInUse     ( void )        { return m_InUse; }
    virtual void            SetIsInUse  ( void )        { m_InUse = true; }
    
    virtual void            SetPlaceHolder( guid newPlaceHolder  ) { m_PlaceHolder = newPlaceHolder;     }
    virtual guid            GetPlaceHolder( void )      { return m_PlaceHolder;    }

    virtual void            SetPosition( vector3& newPosition ) { m_Position = newPosition;    }
    virtual vector3&        GetPosition( void )                 { return m_Position; }

    virtual node_slot_id    GetSlotID(void )  { return m_SlotID; }
    virtual void            SetSlotID( node_slot_id newSlotID ) { m_SlotID = newSlotID; }



    

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Reset           - Resets it to a base state
//
//  Update          - should be called to force recalculation and verification of members
//
//  AddConnection   - Adds a new connection to this node
//
//  DeleteConnection- Removes a connection
//
//  GetConnectionNearestPoint- Returns the connection that ends nearest a given point
//
//  GetConnectionByIndex - just returns the connection by index for fast steping through list
//
//////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void            SetSearchNumber             ( s32 searchNumber ) { m_SearchNumber = searchNumber; }
    virtual s32             GetSearchNumber             ( void ) { return m_SearchNumber; }

    virtual void            SetSearchStepNumber         ( s32 searchStepNumber ) { m_SearchStepNumber = searchStepNumber; }
    virtual s32             GetSearchStepNumber         ( void ) { return m_SearchStepNumber; }

    virtual f32             GetCostToNode               ( void ) { return m_CostToNode; }
    virtual node_slot_id    GetNextInList               ( void ) { return m_NextInSearch; }
    virtual void            SetNextInList               ( node_slot_id nextID ) { m_NextInSearch = nextID; }

    virtual node_slot_id    GetPred                     ( void ) { return m_PredNode;   }

    virtual void            SetCost                     ( f32 cost ) { m_CostToNode = cost; }
    virtual void            SetConnectionUsedInSearch   ( node_slot_id thisSlot) { m_ConnectionUsedInSearch = thisSlot;  }
    virtual node_slot_id    GetConnectionUsedInSearch   ( void ) { return m_ConnectionUsedInSearch;   }

    virtual void            SetSearchInfo               ( node_slot_id nextInSearch, node_slot_id predNode, s32 searchNumber,s32 searchStep, f32 costToNode );

    virtual u32             GetAttributes               ( void ) { return 0; }
    
    virtual void            FileIO                      ( fileio& File );
    virtual void            Save                        ( X_FILE *outFile );
    virtual void            Load                        ( X_FILE *inFile  );
//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Editor function 
//
//////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void        OnEnumProp        ( prop_enum& List );
    virtual xbool       OnProperty        ( prop_query&        I    );

    virtual void        SetIsInPath       ( xbool isinPath) { m_IsInPath = isinPath;     }
    virtual xbool       GetIsInPath       ( void ) { return m_IsInPath; }


    

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Editor function
//
//////////////////////////////////////////////////////////////////////////////////////////////////
//    virtual void    OnEnumProp        ( xarray<prop_enum>& List );
//    virtual xbool   OnProperty        ( prop_query&        I    );


protected: 

    vector3         m_Position;
    s32             m_ConnectionCount;
    node_slot_id    m_Connections[k_MAX_NODE_CONNECTIONS ];
    xbool           m_InUse;

    guid            m_PlaceHolder;

    node_slot_id    m_SlotID;

//  Search variables
    s32             m_SearchNumber;         //  This is the number of the search this is/was being used in
    s32             m_SearchStepNumber;
    node_slot_id    m_PredNode;             //  previous node
    f32             m_CostToNode;           //  total cost to this node
    node_slot_id    m_NextInSearch;         //  Next node in search
    node_slot_id    m_ConnectionUsedInSearch;

    xbool           m_IsInPath;
    


private:
    



};


#endif//BASE_NODE_HPP
