#ifndef VERTEX_MANAGER_HPP
#define VERTEX_MANAGER_HPP

#if !defined(TARGET_PC)
#error "This is only for the PC target platform. Please check build exclusion rules"
#endif

//=========================================================================
// INCLUDES
//=========================================================================
#include "Entropy.hpp"
#include "x_array.hpp"

//=========================================================================
// CLASS
//=========================================================================

class vertex_mgr
{    
//=========================================================================
public:

    void        Init                ( DWORD FVF, s32 VStride );
    void        Kill                ( void );
    xhandle     AddDList            ( void* pVertex, s32 nVertices, u16* pIndex, s32 nIndices, s32 nPrims );
    void        DelDList            ( xhandle hDList );
    void        BeginRender         ( void );
    void        DrawDList           ( xhandle hDList, D3DPRIMITIVETYPE PrimType );
    void*       LockDListVerts      ( xhandle hDList );
    void        UnlockDListVerts    ( xhandle hDList );
    void*       LockDListIndices    ( xhandle hDList, s32& Index );
    void        UnlockDListIndices  ( xhandle hDList );
    void        InvalidateCache     ( void );

//=========================================================================
protected:

    enum
    {
        START_HASH_ENTRY = 8,           // 2^8 = 256 This is where the hash starts
        NUM_HASH_ENTRIES = 8,           // 2^( MIN_HASH_ENTRY + NUM_HASH_ENTRIES )

        MAX_VERTEX_POOL  = 0xffff,      // This has to be less or equal to 0xffff
        MAX_INDEX_POOL   = 0xffff,      // This can be anything really.

        FLAGS_FULL       = (1<<0),      // Full/Empty
        FLAGS_VERTEX     = (1<<1),      // Vertex/Index
    };

    struct dlist
    {   
        s32                         nPrims;           // Number of primitives to render
        xhandle                     hVertexNode;      // Vertex node contains all the info for vertices
        xhandle                     hIndexNode;       // Index node contains all the info for the indices
    };

    struct pool
    {
        s32                         nItems;           // Number of items in the list
        xhandle                     hFirstNode;       // Fist node in the list  
        s32                         Stride;           // The stride of an item
    };

    struct index_pool : public pool
    {
        IDirect3DIndexBuffer9*      pIndex;           // Pointer to the buffer 
    };

    struct vertex_pool : public pool
    {
        IDirect3DVertexBuffer9*     pVertex;          // Pointer to the buffer 
        DWORD                       FVF;              // Description of the type of vertices
    };

    struct node
    {
        s32                         User;             // Number of nodes allocated by the user
        s32                         Count;            // Number of actual items in the node
        s32                         Offset;           // Where in the buffer are our items
        xhandle                     hPool;            // Which pool from a pool type we are talking about
        u32                         Flags;            // Flags for the node
        xhandle                     hGlobalNext;      // Global link list for a given pool
        xhandle                     hGlobalPrev;      // Global link list for a given pool
        xhandle                     hHashNext;        // Next in Hash 
        xhandle                     hHashPrev;        // Prev in Hash
    };

//=========================================================================
protected:

    xhandle AllocIndexSet           ( s32 nIndices );
    xhandle AllocVertexSet          ( s32 nVertices, s32 Stride, DWORD FVF );

    xhandle AllocNode               ( s32 nItems, xbool bVertex, s32 Stride=0, DWORD FVF=0 );
    void    FreeNode                ( xhandle hNode, xbool bVertex );
    s32     NextLog2                ( u32 n );
    s32     GetHashEntry            ( s32 nIndices, xbool bVertex );
    void    RemoveNodeFormHash      ( xhandle hNode, xbool bVertex );
    void    AddNodeToHash           ( xhandle hNode, xbool bVertex );

    void    ActivateStreams         ( xhandle hDList );


//=========================================================================
protected:

    xharray<node>           m_lNode;                          // List of allocated/Free nodes for vertices
    xharray<dlist>          m_lDList;                         // List of display lists that have the rendering info

    xharray<vertex_pool>    m_lVertexPool;                    // Pool for vertices
    xharray<index_pool>     m_lIndexPool;                     // Pool for vertices

    xhandle                 m_VertHash [ NUM_HASH_ENTRIES ];  // Hash entri containing empty nodes
    xhandle                 m_IndexHash[ NUM_HASH_ENTRIES ];  // Hash entri containing empty nodes

    s32                     m_Stride;                         // This is temporary
    DWORD                   m_FVF;                            // This is temporary

    xhandle                 m_LastVertexPool;
    xhandle                 m_LastIndexPool;
};

//=========================================================================
// END
//=========================================================================
#endif