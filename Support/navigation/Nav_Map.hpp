//////////////////////////////////////////////////////////////////////////////
//
// nav_map.hpp
//
//      nav_map holds all of the nav data.  There are 5 significant chunks of 
//      data it creates, they are:
//
//          node data
//          connection data
//          nodes
//          connections
//          connection index          
//
//      The node data and connecction data are what is loaded from binary file
//      dumped by the editor.  They contain only the permanent data for the
//      map.  The nodes and connections are the object wrappers that make use
//      of that data and after the data is loaded, the data for each node and
//      connection is associated with an node or connection object.  The nodes
//      and connections hold the temporary data that is used for searches and
//      values that can be calculated at load time.
//
//      The connection index is used to store the connections used by they nodes.
//      Instead of allocating a fixed size array in each node and limiting
//      the number of connections they can have and also wasting memory and
//      bloating file size when they use fewer than the fixed array size,
//      a single array of indexes is created.  The nodes then hold a u16 that
//      is the starting point of it's connection indices an then a u8 for the
//      count.  This lets the nodes point to up to 255 connections with only
//      3 bytes of overhead and the connection list wastes no space and does
//      not have to do a dynamic allocation per node.
//
//      The only limits to the system currently are memory, no more than
//      32k conections, and no more than 255 connections per node.
//
//      Each node uses up 19 bytes, each connection 12 bytes + 4 for connection
//      indexing.  That is the on disk version, the in memory version works
//      out to 27 bytes per node and 28 bytes per connection.
//
//      CDS- 2/19
//      Added concept of grids.  A grid is a set of nodes and connections that
//      are all connected.  This allows you to immediately look at 2 nodes and
//      tell if a path can be found from one to the other.
//
//  
//////////////////////////////////////////////////////////////////////////////
#ifndef NAV_MAP_HPP
#define NAV_MAP_HPP

#include "x_files.hpp"
#include "Auxiliary\MiscUtils\Fileio.hpp"
#include "ConnectionZoneMgr.hpp"
#include "MiscUtils\guid.hpp"
#include "x_bitstream.hpp"

typedef u16 nav_node_slot_id;
typedef u16 nav_connection_slot_id;

#define NULL_NAV_SLOT (u16)         0xffff
#define MAX_EDGES_IN_LIST           50
#define MAX_CLIP_LINE_CONNECTIONS   50

#define CONNECTION_QUERY_CACHE_SIZE             16      // size of the connection cache used by getnearest... calls

class ng_connection2;
class ng_node2;

struct step_data
{
    step_data();
    void Clear( void );

    nav_connection_slot_id  m_CurrentConnection;        // Where you should be
    nav_connection_slot_id  m_DestConnection;           // Where you should go
    nav_node_slot_id        m_NodeToPassThrough;        // What you need to walk through to get there
};

//===========================================================================

struct lbox
{
    lbox();
    ~lbox();

    bbox            BBox;
    xarray<s32>     Index;
    s32             iGrid;
    f32             Score;
    xbool           bFinished;
};

//===========================================================================

struct path_find_struct
{
    // Functions
    path_find_struct();
    
    void        Clear( void );
    
    // Data
    step_data               m_StepData[MAX_EDGES_IN_LIST];
    u16                     m_nSteps;
    
    u16                     m_bStartPointOnConnection : 1,
                            m_bEndPointOnConnection : 1,
                            m_bStraightPath;
                            
    nav_connection_slot_id  m_StartConnectionSlotID;    
    nav_connection_slot_id  m_EndConnectionSlotID;    
    vector3                 m_vStartPoint;
    vector3                 m_vEndPoint;
    
#ifndef X_RETAIL
    // Debug functions
    void        RenderPath( f32 Radius );

    //  Debug data
    s32         m_nClipLineConnections;        
    vector3     m_ClipLineStart;
    vector3     m_ClipLineEnd;
#endif    
};

//===========================================================================

class nav_map 
{
public:

    enum ClipPlanes
    {
        CLIP_LEFT = 0,
        CLIP_RIGHT,
        CLIP_TOP,
        CLIP_BOTTOM,
        CLIP_MAX
    };
    

    //=========================================================================
    //
    //  node_data/connection_data
    //
    //      Data structure used by the game to load the permanent data and by
    //      the editor to save the data.  
    //
    //
    //=========================================
    //  NEW structures for navigation v2
    //

    struct overlap_data
    {
        overlap_data()
        {
            m_Flags           = 0;
            m_iConnection[0]  = 0;
            m_iConnection[1]  = 0;            
            m_iFirstOverlapPt = 0;            

            m_Center.Set(0,0,0);
        }

        vector3     m_Center;               // centroid of overlapped region
        u32         m_Flags;        
        s16         m_iConnection[2];       // index into an array of connection2_data indices
                                            // ->  an overlap always consumes 2 connections
        s32         m_nOverlapPts;
        s32         m_iFirstOverlapPt;      // index into overlap point storage               
    };

    struct overlap_vert
    {
        enum    
        {
            FLAG_OUTSIDE            = (1<<0),
        };

        overlap_vert() :
            m_Pos(0,0,0),
            m_Flags(0)
        {
        }

        vector3p    m_Pos;
        u32         m_Flags;
    };

    struct connection2_connectivity_data
    {
        connection2_connectivity_data()
        {
            m_iRemoteConnection = 0;
            m_iOverlapData      = 0;
        }
        
        u16         m_iRemoteConnection;
        u16         m_iOverlapData;
    };

    struct connection2_data 
    {
        connection2_data()
        {
            m_BBox.Clear();
            m_Width         = 5;
            m_Flags         = 0;
            m_nOverlaps     = 0;
            m_iFirstConnectivity = 0;            
            m_iGrid         = 255;
            m_iNextInGrid   = NULL_NAV_SLOT;
        }

        bbox        m_BBox;
        vector3     m_StartPt;
        vector3     m_EndPt;
        f32         m_Width;
        u32         m_Flags;
        s16         m_iFirstConnectivity;   // index into the static array of connection2_connectivity_data
        u16         m_iNextInGrid;          // index of next connection in the grid       
        u8          m_nOverlaps;            // number of overlaps        
        u8          m_iGrid;                // Grid this connection belongs to
        
    };

    struct connection_lookup_box
    {
        bbox    m_BBox;             // BBox containing all of the connections
        s32     m_iGrid;            // Grid that all connections are within
        s32     m_iConnection;      // Index to first connection in main array
        s32     m_nConnections;     // Number of connections
    };

    struct update_info
    {
        nav_connection_slot_id      iUpdatedConnection;     // Which connection was updated
    };

    //
    //  end of new structures for navigation v2
    //=========================================
                                nav_map( void );
    virtual                     ~nav_map();
    virtual void                Reset( void );

    //virtual void                Init ( s32 nodeCount, s32 connectionCount, s32 connectionIndexCount );
    virtual void                Init   ( s32 nConnections, 
                                         s32 nRequiredOverlaps, 
                                         s32 nRequiredOverlapInfos, 
                                         s32 nRequiredOverlapVerts );
    virtual void                SetData( void );
    //=========================================================================
    //
    //  Navigation functions
    //
    //=========================================================================
            xbool               DoOverlap                   ( nav_connection_slot_id A,             // IN
                                                              nav_connection_slot_id B,             // IN
                                                              nav_node_slot_id*      pOverlap );    // OUT (optional)
    virtual nav_node_slot_id    GetNearestNode              ( const vector3 &thisPoint );
            vector3             GetNearestPointOnConnection ( nav_connection_slot_id iConnection, const vector3& vPointToCheck );
            void                GetPointOfIntersection      ( const vector3& vLineStartPoint, const vector3& vLineEndPoint, const vector3& vTestPoint, vector3& vIntersectionPoint );
    virtual nav_node_slot_id    GetNearestWithIgnore        ( const vector3& Position, nav_node_slot_id* pNavNodes, s32 nNodes );
            xbool               IsPointInGrid               ( const vector3& Point, u8 Grid );
            xbool               IsPointInMap                ( const vector3& Point );            

    virtual nav_connection_slot_id  GetNearestConnection    ( const vector3 &thisPoint );
            nav_connection_slot_id  GetNearestConnectionInGrid ( const vector3 &thisPoint, u8 withGridID );
            nav_connection_slot_id  GetNearestConnectionNoZoneCheck( const vector3 &thisPoint );
            xbool                   GetConnectionContainingPoint  ( nav_connection_slot_id& connectionList, const vector3& testPoint );
            
            xbool               GetClosestPointInOverlap            ( nav_connection_slot_id SrcSlot,       // IN
                                                                      nav_connection_slot_id DestSlot,      // IN
                                                                      const vector3&         thisPoint,     // IN
                                                                      const vector3&         destPoint,     // IN
                                                                      f32                    characterWidth,// IN
                                                                      vector3&               ClosestPoint );// OUT

            vector3             GetNearestPointInNavMap     ( const vector3& thisPoint );
            vector3             GetInfoForNearestPointInNavmap  ( const vector3&          thisPoint,
                                                                  nav_connection_slot_id& ClosestConnection,
                                                                  nav_node_slot_id&       ClosestNode );

            void                ClipConnections         ( vector3* pDstPos, const vector3* pSrcPos, s32& NDstVerts, s32 NSrcVerts, f32* pClipEdgeValues );
            vector3             GetClosestPointToLine   ( overlap_vert* pVerts, s32 nVerts, const vector3& vPos, const vector3& vEnd );

            nav_node_slot_id    FindOverlapAtDistanceFromPoint( f32 desiredDistance, const vector3& fleeFromLocation, const vector3& startLocation, nav_connection_slot_id startConnection );
            
            void                ClipLine                ( ng_connection2& Con, vector3& LineStart, vector3& LineEnd );
            
            xbool               DoesStraightPathExist   ( path_find_struct& PathFindStruct );
            
            vector3             GetBestGuessDestination ( nav_connection_slot_id StartConnectionSlotID,
                                                          nav_connection_slot_id EndConnectionSlotID,
                                                          const vector3&         StartPosition,
                                                          const vector3&         FinalDestination );
            
            nav_connection_slot_id GetNearestConnectionInGrid( nav_connection_slot_id KnownConnection,  // IN: This connectin's grid is the limit for the search 
                                                               const vector3&         Point );          // IN: Target point
                                                          

            u8      GetConnectionGridID( nav_connection_slot_id iConnection );


    //=========================================================================
    //
    //  Nav manipulation
    //
    //=========================================================================
            void        SetConnectionStatus     ( guid ConnectionGuid, xbool bOnOff );
            xbool       GetConnectionStatus     ( guid ConnectionGuid );

    //=========================================================================
    //
    //  File reading/writing 
    //
    //=========================================================================
    virtual void                Load ( X_FILE *filePointer );
    virtual void                Load ( const char* fileName );

#ifdef X_EDITOR
    virtual void                Save                    ( X_FILE *filePointer );
            void                ScoreLBox               ( lbox& LBox );
            xbool               SplitLBox               ( lbox& LBox, lbox& NewLBox );
            void                CompileLookupBoxes      ( void );
            void                InitConnectionZoneMgr   ( void );
#endif // X_EDITOR
    
    //=========================================================================
    //
    //  Save game functions
    //
    //=========================================================================
    void                        StoreDynamicData        ( bitstream& BitStream );
    void                        RestoreDynamicData      ( bitstream& BitStream );
    void                        DumpDynamicData         ( bitstream& BitStream );

    //=========================================================================
    //
    //  Data access functions
    //
    //=========================================================================
    
    inline overlap_data*                    GetOverlapDataPtr       ( void );
    inline connection2_data*                GetConnectionDataPtr    ( void );
    inline connection2_connectivity_data*   GetConnectivityDataPtr  ( void );
    inline overlap_vert*                    GetOverlapVertDataPtr   ( void );   
    
    virtual connection2_data&               GetConnectionData       ( s32 iConnection   );
    virtual overlap_data&                   GetOverlapData          ( s32 iOverlap      );
    virtual connection2_connectivity_data&  GetConnectivityData     ( s32 iConnectivity );
    
    inline void                             AddConnectionGuid       ( guid  Guid,
                                                                      s32   iConnection );

    inline s32                  GetNodeCount        ( void );
    inline s32                  GetConnectionCount  ( void );    

    virtual ng_connection2&     GetConnectionByID   ( nav_connection_slot_id iConnectoin );
    virtual ng_node2&           GetNodeByID         ( nav_node_slot_id       iNode       );

    virtual const overlap_vert& GetOverlapVert      ( s32 iVert ) const;

            void                RenderNavigationSpine( void );
            void                RenderConnectionsBright( void );

#if !defined(X_RETAIL) || defined(X_QA)
		    void                RenderNavNearView    ( void );
#endif
    /*
		   void					RenderExportedMap	( void );
		   void					RenderAllNodes		( void );
		   void					RenderAllConnections( void );
    */

public:
    static xbool   s_bDebugNavigation;


#ifdef X_EDITOR
public:
    xbool                       m_bIsLoaded;            // just to make sure that we're not using an old graph

protected:
    guid*                       m_pConnectionGuids;      
#endif // X_EDITOR


protected:
    //=======================================================================
    //
    overlap_data*                   m_pOverlapData;         // This stores the information about an overlap (connection id, centroid, index to first vert)
    s32                             m_nOverlapData;

    connection2_connectivity_data*  m_pConnectivityData;    // This stores information about an overlap belonging to a connection
    s32                             m_nConnectivityData;    // (1 for every overlap a connection has) * nConnections
    
    connection2_data*               m_pConnectionData;      // This stores data for connections (1 per connection)
    s32                             m_nConnectionData;

    connection_lookup_box*          m_pLookupBox;           // This stores the lookup boxes for fast queries
    s32                             m_nLookupBoxes;
    s32*                            m_pLookupIndex;         // This stores the lookup indices for fast queries
    s32                             m_nLookupIndices;

    overlap_vert*                   m_pOverlapVerts;        // This is the array of verts where overlap hulls are stored
    s32                             m_nOverlapVerts;        // (the overlap_data's m_iFirstOverlapPt refers to this array)

    guid_lookup                     m_GuidLookup;           // Maps GUIDs to indices into m_pConnection
    //
    //=======================================================================

    ng_node2*               m_pNode;            // Size = m_nOverlapData
    ng_connection2*         m_pConnection;      // Size = m_nConnectionData

    //=======================================================================
    //  NAV CONNECTION CACHE
    //  - for speeding up GetNearest... queries
    //=======================================================================
protected:
            void        AddToConnectionCache( nav_connection_slot_id NewID );
            void        OptimizeConnectionCache( s32 iEntry );

    nav_connection_slot_id          m_RecentConnections[ CONNECTION_QUERY_CACHE_SIZE ];
    u8                              m_iNextRecentConnection;                                // Next to be written to
    s32                             m_RecentCacheHits;
    s32                             m_RecentCacheMisses;

    //=======================================================================
    //  Clip line data
    //=======================================================================
    nav_connection_slot_id  m_ClipLineConnections[ MAX_CLIP_LINE_CONNECTIONS ];
    s32                     m_nClipLineConnections;        
};

extern nav_map g_NavMap;

//===========================================================================
//  Inline functions
//===========================================================================

inline 
nav_map::overlap_data* nav_map::GetOverlapDataPtr( void )
{
    return m_pOverlapData;
}

//===========================================================================

inline 
nav_map::connection2_data* nav_map::GetConnectionDataPtr( void )
{
    return m_pConnectionData;
}

//===========================================================================

inline 
nav_map::connection2_connectivity_data* nav_map::GetConnectivityDataPtr( void )
{
    return m_pConnectivityData;
}

//===========================================================================

inline 
nav_map::overlap_vert* nav_map::GetOverlapVertDataPtr( void )
{
    return m_pOverlapVerts;
}

//===========================================================================

inline 
s32 nav_map::GetNodeCount( void )
{
    return m_nOverlapData;
}

//===========================================================================

inline 
s32 nav_map::GetConnectionCount( void )
{
    return m_nConnectionData;
}

//===========================================================================

inline 
void nav_map::AddConnectionGuid( guid Guid, s32 iConnection )
{
    ASSERT( (iConnection >= 0) && (iConnection < m_nConnectionData));

    m_GuidLookup.Add( Guid, iConnection );
#ifdef X_EDITOR
    m_pConnectionGuids[ iConnection ] = Guid;
#endif // X_EDITOR
}

//===========================================================================

#endif//NAV_MAP_HPP
