///////////////////////////////////////////////////////////////////////////////
//
//  nav_connection.hpp
//
//
///////////////////////////////////////////////////////////////////////////////
#ifndef NAV_CONNECTION_HPP
#define NAV_CONNECTION_HPP

#include "navigation\nav_node.hpp"
#include "navigation\base_connection.hpp"

class nav_connection : public base_connection
{
public:
    enum AI_hints
    {
        HINT_NONE       = 0x00000000,
        HINT_DARK       = BIT(0),       //  Connection is mostly concealed in Darkness
        HINT_COVER      = BIT(1),       //  Path has significant cover
        HINT_WALL       = BIT(2),       //  connection is on a wall
        HINT_CEILING    = BIT(3),       //  Connection is on the ceiling
        HINT_SNEAKY     = BIT(4),       //  Connection is considered a sneaky path
        HINT_JUMPER     = BIT(5),       //  Connection is a jump
        HINT_ORIENTATION_CHANGE = BIT(6),// Connection requires orientation change
        HINT_ONE_WAY    = BIT(7),
        HINT_PATROL_ROUTE= BIT(8),       //  Is this part of a patrol route

        HINT_ALL        = 0xFFFFFFFF

    
    };

    
    
                    nav_connection(void);
    virtual         ~nav_connection();

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
    virtual s32         GetHints    ( void )    { return m_AIHints;     }
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

    virtual s32         GetDataSize ( void );
    virtual void        AddData     ( byte* thisBuffer);
    virtual void        SetData     ( byte* thisBuffer);
    virtual void        OnRender    ( void );


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
    s32             m_AIHints;      //      

    //  custom anim data here...  Not sure how the anims work yet so not defining it    

private:



};





#endif//NAV_CONNECTION_HPP