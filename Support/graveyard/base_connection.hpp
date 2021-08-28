///////////////////////////////////////////////////////////////////////////////
//
//  base_connection.hpp
//
//
///////////////////////////////////////////////////////////////////////////////
#ifndef BASE_CONNECTION_HPP
#define BASE_CONNECTION_HPP

#include "navigation\nav_node.hpp"

class base_connection
{
public:
    
                    base_connection(void);
    virtual         ~base_connection();

//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Reset           -   Resets the object to a cleared state
//
//  Update          -   Should be called to recalculate values if something changes
//
//  GetHints        -   Returns the AI hints for this connection
//
//  GetLength       -   Returns the length of the connection, precalculated since it's used often
//
//  GetWidth        -   Returns the Width of this connection
//
//  GetStartNode    -   What is the start node.  Start/End matter since could be one way
//
//  GetEndNode      -   What is the End node
//
//  GetOtherEnd     -   small helper function I made since I was doing this so much
//
//  GetPosiiton     -   connections don't really have a position.  This calcs it from it's nodes
//
//  GetCost         -   Calcs the cost based on the current AI searching
//
//////////////////////////////////////////////////////////////////////////////////////////////////


    virtual void        Reset       ( void );
    virtual void        Update      ( void );
    virtual f32         GetLength   ( void )    { return m_Length;      }
    virtual f32         GetWidth    ( void )    { return m_Width;       }
    virtual node_slot_id GetStartNode( void )    { return m_StartNode;   }
    virtual node_slot_id GetEndNode  ( void )    { return m_EndNode;     }

    virtual node_slot_id GetOtherEnd ( node_slot_id thisEnd ); 

    virtual void        SetNodes    ( node_slot_id startNode, node_slot_id endNode );

    virtual vector3     GetPosition ( void );
    
    virtual f32         GetCost     ( void );
    
    virtual void        FileIO      ( fileio& File );
    virtual void        Save        ( X_FILE *outFile );
    virtual void        Load        ( X_FILE *inFile  );
    virtual void        OnRender    ( void ) {};


//////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Editor function
//
//////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void        OnEnumProp        ( prop_enum& List );
    virtual xbool       OnProperty        ( prop_query&        I    );

    virtual void        SetPlaceHolder( guid newPlaceHolder  ) { m_PlaceHolder = newPlaceHolder;     }
    virtual guid        GetPlaceHolder( void )      { return m_PlaceHolder;    }


    
protected:
    slot_id         m_StartNode,    //  
                    m_EndNode;      //  
    guid            m_PlaceHolder;
    f32             m_Length;       //  Precalculated length so 
    f32             m_Width;        //  Width of the path
    

    //  Values used by nav_mgr to do searches
    f32             m_TotalCost;    //  Total cost up to this point
    s32             m_SearchCount;  //  Which search this information is relative too
    

    //  custom anim data here...  Not sure how the anims work yet so not defining it    

private:



};



#endif//BASE_CONNECTION_HPP