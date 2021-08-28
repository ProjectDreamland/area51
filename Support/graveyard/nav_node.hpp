///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Nav_node.hpp
//
//      nav_node is a connection point for nav_connections n the movement graph.
//      nav_node are stored in object manager and spatial database so they can be
//      search for based on location.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef NAV_NODE_HPP
#define NAV_NODE_HPP


#include "navigation\base_node.hpp"
//#include "miscutils\fileio.hpp"


const f32 k_NavNodeBboxSize = 10.0f;

class nav_node : public base_node
{
public:


    enum flags
    {
        FLAG_NULL               = 0,
        FLAG_HIDING_SPOT        = BIT( 0 ),     //  Is this a designated hiding spot
        FLAG_COVER_SPOT         = BIT( 1 ),     //
        
        FLAG_ALL                = 0xFFFFFFFF
    
    };


                        nav_node(void);
    virtual             ~nav_node();


    virtual     void    OnRender( void );

#ifndef X_RETAIL
    virtual     void    OnDebugRender(void);
#endif // X_RETAIL

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Reset           - Resets it to a base state
//
//  Update          - should be called to force recalculation and verification of members
//
//////////////////////////////////////////////////////////////////////////////////////////////////


    virtual void            Reset                       ( void );
    virtual void            Update                      ( void );               

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
    virtual u32             GetAttributes               ( void ) { return m_Attributes; }
    

    virtual void            FileIO                      ( fileio& File );
    virtual void            Save                        ( X_FILE *outFile );
    virtual void            Load                        ( X_FILE *inFile  );
    
    virtual s32             GetDataSize                 ( void );
    virtual void            AddData                     ( byte* thisBuffer);
    virtual void            SetData                     ( byte* thisBuffer);

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Editor function 
//
//////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void            OnEnumProp        ( prop_enum& List );
    virtual xbool           OnProperty        ( prop_query&        I    );

    virtual void            SetIsInPath       ( xbool isinPath) { m_IsInPath = isinPath;     }
    virtual xbool           GetIsInPath       ( void ) { return m_IsInPath; }
 

protected:

    void                    SanityCheck(void);

///////////////////////////////////////////////////////////////////////////////////////////////////

    
    u32         m_Attributes;
/*
    s32         m_SearchNumber;         //  This is the number of the search this is/was being used in
    s32         m_SearchStepNumber;
    slot_id     m_PredNode;             //  previous node
    f32         m_CostToNode;           //  total cost to this node
    slot_id     m_NextInSearch;         //  Next node in search
    slot_id     m_ConnectionUsedInSearch;

    xbool       m_IsInPath;
*/

};


#endif//NAV_NODE_HPP