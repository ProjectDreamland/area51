//=================================================================================================
//
//  ng_connection.hpp
//
//      
//
//=================================================================================================

#ifndef NG_CONNECTION_HPP
#define NG_CONNECTION_HPP

//=================================================================================================
//  INCLUDES
//=================================================================================================

#include "obj_mgr\obj_mgr.hpp"


//=================================================================================================
//  CLASS DEFINITION
//=================================================================================================

//  If we're in the editor then connections are objects, PS2 they are not objects
#ifdef  WIN32
class ng_connection : public object
#else
class ng_connection
#endif//WIN32
{

public:

//------------------------------------ WIN32---------------------------------------
#ifdef  WIN32
    // Standard RTTI definition
    CREATE_RTTI( ng_connection, object, object )
#endif//WIN32

//------------------------------------- ALL Platforms -----------------------------

                                ng_connection      ( void );
                                ~ng_connection     ( void );


    virtual         void        Reset       ( void );       //  Resets data
    virtual         void        Update      ( void );       //  Updates calculated values    
    virtual         f32         GetLength   ( void );       //  Returns the length
    virtual         f32         GetWidth    ( void );       //  Returns the Width
    virtual         ng_node_id  GetStartNode( void );       //  Returns the first/starting node
    virtual         ng_node_id  GetEndNode  ( void );       //  Returns the second/end node
    virtual         ng_node_id  GetOtherEnd ( ng_node_id thisEnd ); // Pass it one end and it gives you the other
    virtual         void        SetNodes    ( ng_node_id startNode, ng_node_id endNode );   // Sets the nodes
    virtual         vector3     GetPosition ( void );       //  Get's the position
    virtual         f32         GetCost     ( void );       //  Returns the cost for this connection


//------------------------------------ WIN32---------------------------------------
// Editor specific functions
#ifdef  WIN32

    virtual         type        GetType         ( void ) const { return TYPE_NG_CONNECTION; } 
    virtual         bbox        GetLocalBBox    ( void ) const { return m_Sphere.GetBBox(); }
    virtual         s32         GetMaterial     ( void ) const { return MAT_TYPE_NULL; }
                                
    virtual         void        OnEnumProp      ( prop_enum&    List );
    virtual         xbool       OnProperty      ( prop_query&   I    );
    virtual         void        OnRender        ( void ) ;
                                
#endif//WIN32                   
    
                        
//------------------------------------- ALL Platforms -----------------------------
    
protected:
    ng_connection_id            m_ConnectionIndex;  //  Index to the base connection data held in ng_mgr
    f32                         m_Length;           //  Precalculated length so 
    f32                         m_TotalCost;        //  Total cost up to this point
    s32                         m_SearchCount;      //  Which search this information is relative too
                                
                                
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

inline
f32 ng_connection::GetLength   ( void )            
{ 
    return m_Length;      
}
