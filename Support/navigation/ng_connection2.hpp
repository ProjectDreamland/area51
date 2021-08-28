//============================================================================
//
//  ng_connection2.cpp
//
//      Nav graph connection object.  Holds temp data and pointer to
//      connection data stored in the nav_map.
//
//
//============================================================================
#ifndef NG_CONNECTION2_HPP
#define NG_CONNECTION2_HPP
#include "nav_map.hpp"

class nav_map;


//============================================================================


struct pathing_hints
{
    //  PATH TYPES:
    //  -----------

    //  The following values are used to control how favorable
    //  a given connection type is.
    //    0 = Disallow connections with this flag
    //    1 = Very poor   
    //  ...
    //
    //  128 = Normal
    //  ...
    //
    //  255 = Unbelievably optimal 
    //
    //
    //  Try not keep values close to 128.  Using 255 would make all
    //  connections of that type almost as fast as teleporting.
    //
    //  LEASHING
    //  --------
    //
    //  If LeashDist <= 0, leashing is ignored.
    //  If LeashDist >  0, construciton of a path will be terminated
    //                     as soon as a node outside the leash radius
    //                     is reached.
    //
    vector3     LeashPt;        // World pos of leash point
    f32         LeashDist;      // Distance(cm) of leash

    u8          Default;
    u8          Jump;
    u8          SmallNPC;
    
    // Flags
    u8          bUseNavMap : 1;     // Should NPC even use the nav map?

    pathing_hints()
    {
        Default  = 128;
        Jump     = 0;
        SmallNPC = 0;
        bUseNavMap = TRUE;

        LeashPt.Set(0,0,0);
        LeashDist = -1;
    };
};


//============================================================================


class ng_connection2
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
        HINT_JUMP       = BIT(5),       //  Connection is a jump
        HINT_ORIENTATION_CHANGE = BIT(6),// Connection requires orientation change
        HINT_ONE_WAY    = BIT(7),
        HINT_PATROL_ROUTE= BIT(8),       //  Is this part of a patrol route

        HINT_SMALL_NPC  = BIT(9),       //  "Small" NPC Path (.5m tall max)

        HINT_DISABLED   = BIT(10),      // This connection has been shut off


        FLAG_OPEN               = BIT( 29 ),    // For pathing
        FLAG_CLOSED             = BIT( 30 ),    // For pathing

        FLAG_ALL_PATHING        = (FLAG_OPEN | FLAG_CLOSED),

        HINT_ALL        = 0xFFFFFFFF
   
    };


                                    ng_connection2(void);
    virtual                        ~ng_connection2();
    

    //=========================================================================
    //
    //  Init                - Sets the connection data on creation
    //  CalculateLenth      - Called on creation to calculate length of connection
    //  GetDistanceToPoint  - Distance from this connection to a Point
    //  
    //=========================================================================
    void                            Init                    (   nav_map::connection2_data   *connectionData,
                                                                nav_map*                    owner,
                                                                nav_connection_slot_id      iSlot );
    void                            CalculateLength         ( void );
    f32                             GetDistanceToPoint      ( const vector3& pointToCheck );
    xbool                           IsPointInConnection     ( const vector3& pointToCheck, f32 bufferAmount = 10.0f );

    void                            GetCorners              ( vector3* pCorners );
    xbool                           ClipLine                ( vector3& LineStart, vector3& LineEnd );
    
    inline nav_connection_slot_id   GetSlotID               ( void ) const;
    
    //=========================================================================
    //  
    //  Inline accessor functions for data, name should say it all
    //
    //=========================================================================

    inline u8                       GetOverlapCount                 ( void ) const;
    inline nav_connection_slot_id   GetOverlapRemoteConnectionID    ( s32 iOverlap ) const;
    inline nav_node_slot_id         GetOverlapNodeID                ( s32 iOverlap ) const;
     
    inline u32                      GetAIHints              ( void ) const ;
/*
    inline nav_node_slot_id         GetStartNodeID          ( void );
    inline nav_node_slot_id         GetEndNodeID            ( void );
    inline nav_node_slot_id         GetOtherNodeID          ( nav_node_slot_id thisNode );

    inline ng_node&                 GetStartNode            ( void );
    inline ng_node&                 GetEndNode              ( void );
    inline ng_node&                 GetOtherNode            ( nav_node_slot_id thisNode );
  */  
    inline vector3                  GetStartPosition        ( void ) const;
    inline vector3                  GetEndPosition          ( void ) const;
    inline vector3                  GetAnchorPosition       ( s32 iAnchor_0_or_1) const;

    inline f32                      GetLength               ( void ) const;
    inline f32                      GetWidth                ( void ) const;  

    inline u32                      GetFlags                ( void ) const;

    inline u8                       GetGridID               ( void ) const;

    inline void                     SetEnabled              ( xbool bOnOff );
    inline xbool                    GetEnabled              ( void );

    //
    //  Pathing related
    //
    inline void                     SetOpen                 ( void );
    inline void                     SetClosed               ( void );
    inline xbool                    IsOpen                  ( void ) const;
    inline xbool                    IsClosed                ( void ) const;
    inline void                     ResetPathingFlags       ( void );

    inline xbool                    IsOneWay                ( void ) const;
    inline xbool                    IsSmall                 ( void ) const;
    
#ifdef X_EDITOR
           void                     EditorGetZoneAndCount   ( xbool* pZoneArray, s32& nConnectionsInList );
           void                     FindZonesSpanned        ( vector3& vPosStart, vector3& vPosEnd, xbool* pZoneArray, s32& nConnectionsInList );
           void                     FindZonesSpanned        ( xarray<u16>* pZoneXArray, s32 nIndex );
#endif // X_EDITOR

protected:

    inline void                     SetSlotID               ( nav_connection_slot_id SlotID );


    nav_map::connection2_data*      m_ConnectionData;   //  Pointer to the connection data for this object

    nav_map*                        m_Owner;            //  Pointer to the nav_map that contains this
                                                        //  connection.  If Map becomes singleton, make
                                                        //  this pointer a static to save 4 bytes per connection

    f32                             m_Length;           //  Precalculated length
    nav_connection_slot_id          m_SlotID;           //  What is my ID
    

    friend class nav_map;
};


//=============================================================================
//
//  Inline functions
//
//=============================================================================

//=============================================================================
inline 
u32 ng_connection2::GetAIHints ( void ) const
{
    ASSERT(m_ConnectionData);
    return m_ConnectionData->m_Flags ;
}
/*
//=============================================================================
inline 
nav_node_slot_id ng_connection2::    GetStartNodeID ( void )
{
    ASSERT(m_ConnectionData);
    return m_ConnectionData->m_Node[0];
}


//=============================================================================
inline 
nav_node_slot_id ng_connection2::    GetEndNodeID ( void )
{
    ASSERT(m_ConnectionData);
    return m_ConnectionData->m_Node[1];
}

//=============================================================================
inline 
nav_node_slot_id ng_connection2::    GetOtherNodeID ( nav_node_slot_id thisID  )
{
    ASSERT(m_ConnectionData);
    if (thisID != m_ConnectionData->m_Node[0] && thisID != m_ConnectionData->m_Node[1])
    {
        x_DebugMsg("Error in Nav-Pathing detected!\n");
    }

    if ( m_ConnectionData->m_Node[0] == thisID  )
    {
        return m_ConnectionData->m_Node[1];

    }
    else
    {
        return m_ConnectionData->m_Node[0];

    }
}
*/
//=============================================================================
inline 
void ng_connection2::SetSlotID( nav_connection_slot_id SlotID )
{
    m_SlotID = SlotID;

}

//=============================================================================
inline 
nav_connection_slot_id ng_connection2::GetSlotID( void ) const
{
    return m_SlotID;

}

//=============================================================================
inline 
f32  ng_connection2::                GetLength ( void ) const
{
    return m_Length;
}


//=============================================================================
inline 
f32  ng_connection2::                GetWidth ( void ) const
{
    ASSERT(m_ConnectionData);
    return m_ConnectionData->m_Width;
}

//=============================================================================

inline 
u8 ng_connection2::GetOverlapCount( void ) const
{
    ASSERT( m_ConnectionData );
    return m_ConnectionData->m_nOverlaps;
}

//=============================================================================

inline 
nav_connection_slot_id ng_connection2::GetOverlapRemoteConnectionID( s32 iOverlap ) const
{
    ASSERT( m_ConnectionData );
    ASSERT( (iOverlap>=0) && (iOverlap<m_ConnectionData->m_nOverlaps) );

    nav_map::connection2_connectivity_data& Connect = g_NavMap.GetConnectivityData( m_ConnectionData->m_iFirstConnectivity+iOverlap );
    return Connect.m_iRemoteConnection;
}

//=============================================================================

inline 
nav_node_slot_id ng_connection2::GetOverlapNodeID( s32 iOverlap ) const
{
    ASSERT( m_ConnectionData );
    ASSERT( (iOverlap>=0) && (iOverlap<m_ConnectionData->m_nOverlaps) );

    nav_map::connection2_connectivity_data& Connect = g_NavMap.GetConnectivityData( m_ConnectionData->m_iFirstConnectivity+iOverlap );
    return Connect.m_iOverlapData;
}

//=============================================================================

inline
void ng_connection2::SetOpen( void )
{
    ASSERT(m_ConnectionData);
    ResetPathingFlags();
    m_ConnectionData->m_Flags |=  FLAG_OPEN;
}

//=============================================================================

inline
void ng_connection2::SetClosed( void )
{
    ASSERT(m_ConnectionData);
    ResetPathingFlags();
    m_ConnectionData->m_Flags |=  FLAG_CLOSED;
}

//=============================================================================

inline
xbool ng_connection2::IsOpen( void ) const
{
    ASSERT(m_ConnectionData);
    return !!(m_ConnectionData->m_Flags & FLAG_OPEN);
}

//=============================================================================

inline
xbool ng_connection2::IsClosed( void ) const
{
    ASSERT(m_ConnectionData);
    return !!(m_ConnectionData->m_Flags & FLAG_CLOSED);
}

//=============================================================================

inline
void ng_connection2::ResetPathingFlags( void )
{
    ASSERT(m_ConnectionData);
    m_ConnectionData->m_Flags =  GetFlags() & (~FLAG_ALL_PATHING);
}

//=============================================================================

inline 
u32 ng_connection2::GetFlags ( void ) const
{
    ASSERT(m_ConnectionData);

    return m_ConnectionData->m_Flags;
}

//============================================================================= 

inline 
vector3 ng_connection2::GetStartPosition( void ) const
{
    ASSERT( m_ConnectionData );
    return m_ConnectionData->m_StartPt;
}

//============================================================================= 

inline 
vector3 ng_connection2::GetEndPosition( void ) const
{
    ASSERT( m_ConnectionData );
    return m_ConnectionData->m_EndPt;
}

//============================================================================= 

inline 
vector3 ng_connection2::GetAnchorPosition( s32 iAnchor_0_or_1 ) const
{
    ASSERT( m_ConnectionData );
    iAnchor_0_or_1 = MINMAX(0,iAnchor_0_or_1,1);
    if (iAnchor_0_or_1==0)
        return m_ConnectionData->m_StartPt;
    return m_ConnectionData->m_EndPt;
}

//=============================================================================

inline 
u8 ng_connection2::GetGridID( void ) const
{
    ASSERT( m_ConnectionData );
    return m_ConnectionData->m_iGrid;
}

//=============================================================================

inline 
void ng_connection2::SetEnabled( xbool bOnOff )
{
    ASSERT( m_ConnectionData );
    if (bOnOff)
        m_ConnectionData->m_Flags &= (~HINT_DISABLED);        
    else
        m_ConnectionData->m_Flags |= HINT_DISABLED;

}

//=============================================================================

inline 
xbool ng_connection2::GetEnabled( void )
{
    ASSERT( m_ConnectionData );
    return !(m_ConnectionData->m_Flags & HINT_DISABLED);
}

//=============================================================================

inline 
xbool ng_connection2::IsOneWay( void ) const
{
    ASSERT( m_ConnectionData );
    return !!(m_ConnectionData->m_Flags & ng_connection2::HINT_ONE_WAY);
}

//=============================================================================

inline 
xbool ng_connection2::IsSmall( void ) const
{
    ASSERT( m_ConnectionData );
    return !!(m_ConnectionData->m_Flags & ng_connection2::HINT_SMALL_NPC);
}

//=============================================================================

//=============================================================================

//=============================================================================

//=============================================================================

//=============================================================================

/*
//=============================================================================
inline 
ng_node& ng_connection2::            GetStartNode ( void )
{
    
    nav_node_slot_id StartSlot = m_ConnectionData->m_Node[0];
    return m_Owner->GetNodeByIndex(StartSlot );

    
}

//=============================================================================
inline 
ng_node& ng_connection2::            GetEndNode   ( void )
{
    return m_Owner->GetNodeByIndex(m_ConnectionData->m_Node[1] );

}

//=============================================================================
inline 
ng_node& ng_connection2::            GetOtherNode   ( nav_node_slot_id thisID  )
{
    if (thisID != m_ConnectionData->m_Node[0] && thisID != m_ConnectionData->m_Node[1])
    {
        x_DebugMsg("Error in Nav-Pathing detected!\n");
    }

    if(thisID == m_ConnectionData->m_Node[0] )
    {
        return m_Owner->GetNodeByIndex(m_ConnectionData->m_Node[1] );
    }
    else 
    {
        return m_Owner->GetNodeByIndex(m_ConnectionData->m_Node[0] );
    }
    
}
*/

#endif//NG_CONNECTION2_HPP