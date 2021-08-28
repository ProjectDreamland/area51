//=================================================================================================
//
//  ng_connection.hpp
//
//      
//
//=================================================================================================

#ifndef NG_NODE_HPP
#define NG_NODE_HPP

//=================================================================================================
//  INCLUDES
//=================================================================================================

#include "obj_mgr\obj_mgr.hpp"


//=================================================================================================
//  CLASS DEFINITION
//=================================================================================================

//  If we're in the editor then connections are objects, PS2 they are not objects
#ifdef  WIN32
class ng_node : public object
#else
class ng_node
#endif//WIN32
{

public:

//------------------------------------ WIN32---------------------------------------
#ifdef  WIN32
    // Standard RTTI definition
    CREATE_RTTI( ng_node, object, object )
#endif//WIN32

//------------------------------------- ALL Platforms -----------------------------

                                ng_node                     ( void );
                                ~ng_node                    ( void );
//

    virtual void                AddConnection               ( ng_node_id newConnection );    
    virtual void                Reset                       ( void );
    virtual void                Update                      ( void );               

    virtual void                DeleteConnection            ( node_slot_id thisConnection );

    virtual node_slot_id        GetConnectionNearestPoint   ( vector3 thisPoint );
    virtual node_slot_id        GetConnectionByIndex        ( s32  thisConnection ) ;
    virtual s32                 GetConnectionCount          ( void );

    virtual void                SetPosition                 ( vector3& newPosition ); 
    virtual vector3&            GetPosition                 ( void ); 

    virtual void                SetSearchNumber             ( s32 searchNumber ); 
    virtual s32                 GetSearchNumber             ( void ); 

    virtual void                SetSearchStepNumber         ( s32 searchStepNumber ); 
    virtual s32                 GetSearchStepNumber         ( void ); 

    virtual f32                 GetCostToNode               ( void ); 
    virtual node_slot_id        GetNextInList               ( void ); 
    virtual void                SetNextInList               ( node_slot_id nextID ); 

    virtual node_slot_id        GetPred                     ( void ); 

    virtual void                SetCost                     ( f32 cost ); 
    virtual void                SetConnectionUsedInSearch   ( node_slot_id thisSlot); 
    virtual node_slot_id        GetConnectionUsedInSearch   ( void ); 

    virtual void                SetSearchInfo               ( node_slot_id nextInSearch, 
                                                              node_slot_id predNode, 
                                                              s32 searchNumber,
                                                              s32 searchStep, 
                                                              f32 costToNode               );

    virtual u32                 GetAttributes               ( void );


    virtual void                SetIsInPath                 ( xbool isinPath);
    virtual xbool               GetIsInPath                 ( void );



//------------------------------------ WIN32---------------------------------------
// Editor specific functions
#ifdef  WIN32

    virtual type                GetType                     ( void ) const { return TYPE_NG_NODE;       } 
    virtual bbox                GetLocalBBox                ( void ) const { return m_Sphere.GetBBox(); }
    virtual s32                 GetMaterial                 ( void ) const { return MAT_TYPE_NULL;      }
                                                            
    virtual void                OnEnumProp                  ( prop_enum&    List );
    virtual xbool               OnProperty                  ( prop_query&   I    );
    virtual void                OnRender                    ( void               );
                                
#endif//WIN32                   
    
                        
//------------------------------------- ALL Platforms -----------------------------
    
protected:
    ng_node_id                  m_ConnectionIndex;  //  Index to the base connection data held in ng_mgr


//-----------------------------------  Search variables ---------------------------
    s16             m_SearchNumber;             //  This is the number of the search this is/was being used in
    s16             m_SearchStepNumber;         //  
    ng_node_id      m_PredNode;                 //  previous node
    ng_node_id      m_NextInSearch;             //  Next node in search
    ng_node_id      m_ConnectionUsedInSearch;   //  
    f32             m_CostToNode;               //  total cost to this node
    xbool           m_IsInPath;
                              
                                
//------------------------------------ WIN32---------------------------------------
// Editor specific data
#ifdef  WIN32

    sphere                      m_Sphere;           //  for collision and bbox info

#endif//WIN32


private:
    

};







//-------------------------------------------------------------------------------------------------
//      Inline functions
//-------------------------------------------------------------------------------------------------


void ng_node::SetPosition( vector3& newPosition )
{ 
    m_Position = newPosition;    
}

vector3& ng_node::GetPosition( void )
{ 
    return m_Position; 
}

void ng_node::SetSearchNumber( s32 searchNumber ) 
{ 
    m_SearchNumber = searchNumber; 
}

s32 ng_node::GetSearchNumber( void )
{ 
    return m_SearchNumber; 
}

void ng_node::SetSearchStepNumber( s32 searchStepNumber ) 
{ 
    m_SearchStepNumber = searchStepNumber; 
}

s32 ng_node::GetSearchStepNumber( void )
{ 
    return m_SearchStepNumber; 
}

f32 ng_node::GetCostToNode ( void )
{ 
    return m_CostToNode; 
}

node_slot_id ng_node::GetNextInList ( void )
{ 
    return m_NextInSearch; 
}

void ng_node::SetNextInList ( node_slot_id nextID )
{ 
    m_NextInSearch = nextID; 
}

node_slot_id ng_node::GetPred ( void )
{ 
    return m_PredNode;   
}

void ng_node::SetCost ( f32 cost )
{ 
    m_CostToNode = cost; 
}

void ng_node::SetConnectionUsedInSearch ( node_slot_id thisSlot)
{ 
    m_ConnectionUsedInSearch = thisSlot;  
}

node_slot_id ng_node::GetConnectionUsedInSearch ( void )
{
    return m_ConnectionUsedInSearch;   
}


void ng_node::SetIsInPath ( xbool isinPath)
{ 
    m_IsInPath = isinPath;     
}

xbool ng_node::GetIsInPath ( void )
{ 
    return m_IsInPath; 
}

