//============================================================================
//
//  ng_node2
//
//      Holds temp data and manages data from the nav_map
//
//
//
//============================================================================
#ifndef NG_NODE2_HPP
#define NG_NODE2_HPP
#include "nav_map.hpp"
#include "ng_connection2.hpp"


class ng_node2
{
public:
    
    enum flags
    {
        FLAG_NULL               = 0,
        FLAG_HIDING_SPOT        = BIT( 0 ),     //  Is this a designated hiding spot
        FLAG_COVER_SPOT         = BIT( 1 ),     //

        BITMASK_GAME_FLAGS      = FLAG_HIDING_SPOT | FLAG_COVER_SPOT,

        FLAG_OPEN               = BIT( 29 ),
        FLAG_CLOSED             = BIT( 30 ),
        
        FLAG_ALL                = 0xFFFFFFFF
    
    };



                                    ng_node2(void);
    virtual                        ~ng_node2();
    
    //=========================================================================
    //  
    //  Init    - Sets up initial data for the node.  Called after data load
    //
    //=========================================================================

    void                            Init                    ( nav_map::overlap_data* pNodeData, 
                                                              nav_map*               pOwner,
                                                              nav_node_slot_id       iSlot );

    //=========================================================================
    //
    //  Inline data accessor functions, names should say it all
    //
    //=========================================================================
    inline vector3&                 GetPosition             ( void ) const;

    inline nav_connection_slot_id   GetOtherConnectionID    ( nav_connection_slot_id firstConnection ) const;
    inline nav_connection_slot_id   GetConnectionIDByID     ( u16 connectionIndex ) const;
    inline ng_connection2&          GetConnectionByID       ( u16 connectionIndex ) const;

    inline u32                      GetFlags                ( void ) const;
    inline void                     ResetGameFlags          ( void );
    
    inline u8                       GetGridID               ( void ) const;
    inline void                     SetGridID               ( u8 thisID );
    
    inline nav_node_slot_id         GetSlotID               ( void ) const;

    inline s32                          GetVertCount        ( void ) const;
    inline const nav_map::overlap_vert& GetVert             ( s32 iVert );
    inline vector3                  GetCenter               ( void ) const;

    inline xbool                    IsVertOutside           ( s32 iVert );
           xbool                    IsPointInside           ( const vector3& checkPoint );

protected:

    inline void                     SetSlotID( nav_node_slot_id iSlot );

    nav_map::overlap_data*          m_NodeData;     //  Pointer to the overlap data stored in the map
    nav_map*                        m_Owner;        //  Pointer to the nav map.  If map becomes a
                                                    //  singleton, make this a static to save 4 bytes
                                                    //  per node
    nav_node_slot_id                m_SlotID;       //  What is my ID

    friend class nav_map;
};







//=============================================================================
//
//  Inline functions
//
//=============================================================================
inline
vector3&  ng_node2::GetPosition ( void ) const
{
    ASSERT( m_NodeData );
    return m_NodeData->m_Center;

}


//=============================================================================

inline 
nav_connection_slot_id ng_node2::GetOtherConnectionID( nav_connection_slot_id firstConnection ) const
{
    ASSERT(m_NodeData);
    if( m_NodeData->m_iConnection[0] == firstConnection )
    {
        return m_NodeData->m_iConnection[1];
    }
    else
    {
        return m_NodeData->m_iConnection[0];
    }

}

//=============================================================================
inline 
nav_connection_slot_id ng_node2::GetConnectionIDByID ( u16 connectionIndex ) const
{
    ASSERT(m_NodeData);
    ASSERT( (connectionIndex == 0) || (connectionIndex == 1));
    
    connectionIndex = MINMAX(0,connectionIndex,1);

    return m_NodeData->m_iConnection[ connectionIndex ];    
}


//=============================================================================
inline 
u32 ng_node2::GetFlags ( void ) const
{
    ASSERT(m_NodeData);

    return m_NodeData->m_Flags;
}
    

//=============================================================================

inline 
ng_connection2& ng_node2::GetConnectionByID    ( u16 connectionIndex ) const
{
    ASSERT(m_NodeData);   
    ASSERT(m_Owner);

    ASSERT( (connectionIndex == 0) || (connectionIndex == 1));
    
    connectionIndex = MINMAX(0,connectionIndex,1);

    return m_Owner->GetConnectionByID( m_NodeData->m_iConnection[ connectionIndex ] );   
}

//=============================================================================

inline 
u8 ng_node2::GetGridID ( void ) const
{
    ASSERT( m_NodeData );
    ASSERT( m_Owner );

    return (m_Owner->GetConnectionByID( m_NodeData->m_iConnection[0] ).GetGridID() );;    
}

//=============================================================================

inline 
nav_node_slot_id ng_node2::GetSlotID( void ) const
{
    return m_SlotID;
}

//=============================================================================

inline 
void ng_node2::SetSlotID( nav_node_slot_id iSlot )
{
    m_SlotID = iSlot;
}

//=============================================================================

inline
s32 ng_node2::GetVertCount( void ) const
{
    ASSERT( m_NodeData );

    return m_NodeData->m_nOverlapPts;
}

//=============================================================================

inline 
const nav_map::overlap_vert& ng_node2::GetVert( s32 iVert )
{
    ASSERT( m_NodeData );
    ASSERT( iVert >= 0 );
    ASSERT( iVert < m_NodeData->m_nOverlapPts );

    return g_NavMap.GetOverlapVert( m_NodeData->m_iFirstOverlapPt + iVert );
}

//=============================================================================

inline 
xbool ng_node2::IsVertOutside( s32 iVert )
{
    ASSERT( m_NodeData );
    ASSERT( iVert >= 0 );
    ASSERT( iVert < m_NodeData->m_nOverlapPts );

    return !!(g_NavMap.GetOverlapVert( m_NodeData->m_iFirstOverlapPt + iVert ).m_Flags & nav_map::overlap_vert::FLAG_OUTSIDE);
}

//=============================================================================

inline
vector3 ng_node2::GetCenter( void ) const 
{
    ASSERT( m_NodeData );
    return m_NodeData->m_Center;
}

//=============================================================================

//=============================================================================
#endif//NG_NODE2_HPP
