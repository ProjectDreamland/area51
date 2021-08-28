///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  nav_plan.hpp
//
//      Description:    
//          The nav_plan contains the current navigation plan for an actor.  Each AI should have
//          a nav_plan which should contain his current path.  
//                  
//          
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef NAV_PLAN_HPP
#define NAV_PLAN_HPP

#include "Obj_mgr\obj_mgr.hpp"
#include "base_plan.hpp"


class nav_plan : public base_plan
{
public:
                nav_plan ( void );
    virtual     ~nav_plan ();
    
    virtual vector3     GetCurrentGoalPoint ( void ); 
    
    virtual void        SetDestination      ( vector3 newDestination );    
    virtual vector3     GetLastNodePoint    ( void );

};


#endif//NAV_PLAN_HPP
 