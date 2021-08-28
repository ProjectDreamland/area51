///////////////////////////////////////////////////////////////////////////////////////////////////
//  base_plan.hpp
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef BASE_PLAN_HPP
#define BASE_PLAN_HPP

#include "BaseNodeMgr.hpp"
#include "Obj_mgr\obj_mgr.hpp"


const s32   kMAX_NAV_PLAN_CONNECTIONS = 128;    // SB - If it's not this big - it crashes!!

class base_plan
{
public:
                base_plan ( void );
    virtual     ~base_plan ();
    
    void        Reset( void );

    node_slot_id     GetStartingPoint    ( void );               // Gets the first point in the path
    void             SetStartingPoint    ( node_slot_id thisPoint );
    node_slot_id     GetNextPoint        ( node_slot_id thisPoint );  // Let's you step through the list of points
    node_slot_id     GetNextConnection   ( node_slot_id thisPoint );  // Let's you step through the list of connections
    node_slot_id     GetLastPoint        ( void );               // Fast way to get the last point

    void             AddPoint            ( node_slot_id thisConnection, node_slot_id thisPoint, s32 atThisIndex = -1 );
    xbool            ReachedPoint        ( node_slot_id thisPoint );  
    node_slot_id     GetCurrentGoal      ( void ) { return m_CurrentGoal;  }
    virtual vector3  GetCurrentGoalPoint ( void ) = 0;
    
    virtual void     SetDestination      ( vector3 newDestination ) = 0;
    vector3&         GetDestination      ( void ) ;

    node_slot_id     GetDestinationNode  ( void );  
    node_slot_id     GetOwner            ( void );  

    void             SetCompletePath     ( xbool complete ) ; 
    xbool            GetCompletePath     ( void ); 

    virtual vector3  GetLastNodePoint    ( void ) = 0;



protected:
    

    void        PathSanityCheck(void);          //  Checks the current path to make sure it's

///////////////////////////////////////////////////////////////////////////////////////////////////

    xbool            m_CompletePath;                 //  Do we believe this path is complete?
    vector3          m_Destination;                  //  What is the actual destination
    node_slot_id     m_DestinationNode;              //  The node that is nearest the destination point
    node_slot_id     m_NodePath[kMAX_NAV_PLAN_CONNECTIONS ];   //  What nodes do we have to take to get there
    node_slot_id     m_ConnectionPath[kMAX_NAV_PLAN_CONNECTIONS ];// What connections do we take to get there
    node_slot_id     m_Owner;                        //  Who owns this nav plan    
    node_slot_id     m_CurrentGoal;                  //  Where we're headed next
    
};



inline
vector3& base_plan::GetDestination      ( void )  
{ 
    return m_Destination; 
}

inline
node_slot_id     base_plan::GetDestinationNode  ( void )  
{ 
    return m_DestinationNode;  
}

inline
node_slot_id     base_plan::GetOwner            ( void )  
{ 
    return m_Owner;  
}

inline
void        base_plan::SetCompletePath     ( xbool complete )  
{ 
    m_CompletePath = complete;    
}

inline
xbool       base_plan::GetCompletePath     ( void ) 
{ 
    return m_CompletePath;    
}





#endif//BASE_PLAN_HPP