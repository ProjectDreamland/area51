///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  node_base.cpp
//
//      - implementation for the base node class
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef NODE_BASE_HPP
#define NODE_BASE_HPP

#include "Obj_mgr\obj_mgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "x_types.hpp"
#include "navigation\NavNodeMgr.hpp"


typedef s16 node_id;
typedef s16 connection_id;

const s32 MAX_CONNECTIONS = 8;


///////////////////////////////////////////////////////////////////////////////////////////////////
//  -- Begin Win32 specific
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
    class node_base : public object
#else
///////////////////////////////////////////////////////////////////////////////////////////////////
//  -- End Win32 specific
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
//  -- Beging PS2 specific
///////////////////////////////////////////////////////////////////////////////////////////////////
    class node_base 
#endif//WIN32
///////////////////////////////////////////////////////////////////////////////////////////////////
//  -- End PS2 specific
///////////////////////////////////////////////////////////////////////////////////////////////////

{
public:

///////////////////////////////////////////////////////////////////////////////////////////////////
//  -- Begin Win32 specific
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
    CREATE_RTTI( node_base, object, object )
    virtual bbox            GetLocalBBox    ( void ) const;
    virtual s32             GetMaterial     ( void ) const { return MAT_TYPE_NULL; }

    virtual void            OnMove          ( const vector3& newPos );


#endif//WIN32
///////////////////////////////////////////////////////////////////////////////////////////////////
//  -- End Win32 specific
///////////////////////////////////////////////////////////////////////////////////////////////////

    virtual void            OnEnumProp      ( prop_enum& List );
    virtual xbool           OnProperty      ( prop_query&        I    );

    virtual void            AddConnection               ( node_slot_id newConnection );    
    virtual void            Reset                       ( void );
    virtual void            Update                      ( void );               



    virtual void            DeleteConnection            ( node_slot_id thisConnection );

    virtual node_slot_id    GetConnectionNearestPoint   ( vector3 thisPoint );
    virtual node_slot_id    GetConnectionByIndex        ( s32  thisConnection ) ;
    virtual s32             GetConnectionCount          ( void );

    virtual xbool           IsInUse                     ( void )       ;
    virtual void            SetIsInUse                  ( void ) ;
    
    virtual void            SetPlaceHolder              ( guid newPlaceHolder  );
    virtual guid            GetPlaceHolder              ( void ) ;

    virtual void            SetPosition                 ( vector3& newPosition );
    virtual vector3&        GetPosition                 ( void ) ;
    virtual node_slot_id    GetSlotID                   ( void )  ;
    virtual void            SetSlotID                   ( node_slot_id newSlotID ) ;


    virtual void            SetSearchNumber             ( s32 searchNumber );
    virtual s32             GetSearchNumber             ( void );

    virtual void            SetSearchStepNumber         ( s32 searchStepNumber );
    virtual s32             GetSearchStepNumber         ( void );

    virtual f32             GetCostToNode               ( void );
    virtual node_slot_id    GetNextInList               ( void );
    virtual void            SetNextInList               ( node_slot_id nextID );

    virtual node_slot_id    GetPred                     ( void );

    virtual void            SetCost                     ( f32 cost ) ;
    virtual void            SetConnectionUsedInSearch   ( node_slot_id thisSlot) ;
    virtual node_slot_id    GetConnectionUsedInSearch   ( void ) ;
    virtual void            SetSearchInfo               (   node_slot_id    nextInSearch, 
                                                            node_slot_id    predNode, 
                                                            s32             searchNumber,
                                                            s32             searchStep, 
                                                            f32             costToNode   );

    virtual u32             GetAttributes               ( void );
    
    virtual void            Save                        ( X_FILE *outFile );
    virtual void            Load                        ( X_FILE *inFile  );


protected:

#ifdef WIN32

    virtual void            OnColCheck      ( void );

#endif//WIN32

    virtual void            OnInit          ( void );
    virtual void            OnRender        ( void );
    virtual void            OnAdvanceLogic  ( f32 DeltaTime );      


///////////////////////////////////////////////////////////////////////////////////////////////////


    vector3                 m_Position;         
    connection_id           m_Connections[MAX_CONNECTIONS];

//  Search variables
    u16                     m_SearchNumber;         //  This is the number of the search this is/was being used in
    s32                     m_SearchStepNumber;
    node_slot_id            m_PredNode;             //  previous node
    f32                     m_CostToNode;           //  total cost to this node
    node_slot_id            m_NextInSearch;         //  Next node in search
    node_slot_id            m_ConnectionUsedInSearch;

    xbool                   m_IsInPath;







        

}

xbool           node_base::IsInUse     ( void )        
{ 
    return m_InUse; 
}

void            node_base::SetIsInUse  ( void )        
{ 
    m_InUse = true; 
}
    
void            node_base::SetPlaceHolder( guid newPlaceHolder  ) 
{ 
    m_PlaceHolder = newPlaceHolder;     
}

guid            node_base::GetPlaceHolder( void )      
{ 
    return m_PlaceHolder;    
}

void            node_base::SetPosition( vector3& newPosition ) 
{ 
    m_Position = newPosition;    
}

vector3&        node_base::GetPosition( void )                 
{ 
    return m_Position; 
}

node_slot_id    node_base::GetSlotID(void )  
{ 
    return m_SlotID; 
}

void            node_base::SetSlotID( node_slot_id newSlotID ) 
{ 
    m_SlotID = newSlotID; 
}


void            node_base::SetSearchNumber             ( s32 searchNumber ) 
{ 
    m_SearchNumber = searchNumber; 
}

s32             node_base::GetSearchNumber             ( void ) 
{ 
    return m_SearchNumber; 
}

void            node_base::SetSearchStepNumber         ( s32 searchStepNumber ) 
{ 
    m_SearchStepNumber = searchStepNumber; 
}

s32             node_base::GetSearchStepNumber         ( void ) 
{ 
    return m_SearchStepNumber; 
}

f32             node_base::GetCostToNode               ( void ) 
{ 
    return m_CostToNode; 
}

node_slot_id    GetNextInList               ( void ) 
{ 
    return m_NextInSearch; 
}

void            node_base::SetNextInList               ( node_slot_id nextID ) 
{ 
    m_NextInSearch = nextID; 
}

node_slot_id    node_base::GetPred                     ( void ) 
{ 
    return m_PredNode;   
}

void            node_base::SetCost                     ( f32 cost ) 
{ 
    m_CostToNode = cost; 
}

void            node_base::SetConnectionUsedInSearch   ( node_slot_id thisSlot) 
{ 
    m_ConnectionUsedInSearch = thisSlot;  
}

node_slot_id    node_base::GetConnectionUsedInSearch   ( void ) 
{ 
    return m_ConnectionUsedInSearch;   
}

u32             node_base::GetAttributes               ( void ) 
{ 
    return 0; 
}






#endif//NODE_BASE_HPP