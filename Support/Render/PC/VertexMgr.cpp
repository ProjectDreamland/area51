//=========================================================================
//
// Vertex Manager for PC
//
//=========================================================================

#if !defined(TARGET_PC)
#error "This is only for the PC target platform. Please check build exclusion rules"
#endif

#include "VertexMgr.hpp"

//=========================================================================

s32 vertex_mgr::NextLog2( u32 n )
{
    if( n == 0 ) return 0;
    f32 f  = (f32)((n << 1) - 1);
    u32 rn = *((u32*)&f);
    return (s32)((rn >> 23) - 127);
}

//=========================================================================

s32 vertex_mgr::GetHashEntry( s32 nItems, xbool bVertex )  
{ 
    s32 iHash;

    (void)( bVertex );
    iHash = iMin( (NUM_HASH_ENTRIES-1), iMax( 0, (NextLog2(nItems) - START_HASH_ENTRY) ) );   
    
    ASSERT( iHash >= 0 );
    ASSERT( iHash < NUM_HASH_ENTRIES );
    return iHash;
}

//=========================================================================

void vertex_mgr::Init( DWORD FVF, s32 VStride )
{
    s32 i;

    // Clear all the vertices
    for( i=0; i<NUM_HASH_ENTRIES; i++ )
    {
        m_VertHash[i].Handle = HNULL;
    }

    // Clear all the Indices
    for( i=0; i<NUM_HASH_ENTRIES; i++ )
    {
        m_IndexHash[i].Handle = HNULL;
    }

    // Init fields
    m_Stride = VStride;
    m_FVF    = FVF;
}

//=========================================================================

void vertex_mgr::RemoveNodeFormHash( xhandle hNode, xbool bVertex )
{
    node& Node      = m_lNode( hNode );
    s32   HashEntry = GetHashEntry( Node.Count, bVertex );

    if( Node.hHashNext.IsNull() == FALSE ) 
        m_lNode( Node.hHashNext ).hHashPrev = Node.hHashPrev;

    if( Node.hHashPrev.IsNull() == FALSE )
    {
        m_lNode( Node.hHashPrev ).hHashNext = Node.hHashNext;
    }
    else if( bVertex )
    {
        ASSERT( m_VertHash[HashEntry] == hNode );
        m_VertHash[HashEntry]  = Node.hHashNext;
    }
    else
    {
        ASSERT( m_IndexHash[HashEntry] == hNode );
        m_IndexHash[HashEntry] = Node.hHashNext;
    }

    Node.hHashPrev.Handle = HNULL;
    Node.hHashNext.Handle = HNULL;
    Node.Flags           |= FLAGS_FULL;
}

//=========================================================================

void vertex_mgr::AddNodeToHash( xhandle hNode, xbool bVertex )
{
    node& Node      = m_lNode( hNode );
    s32   HashEntry = GetHashEntry( Node.Count, bVertex );

    Node.hHashPrev.Handle = HNULL;
    Node.Flags &= ~FLAGS_FULL;

    if( bVertex )
    {
        Node.hHashNext          = m_VertHash[ HashEntry ];
        m_VertHash[ HashEntry ] = hNode;
    }
    else
    {
        Node.hHashNext           = m_IndexHash[ HashEntry ];
        m_IndexHash[ HashEntry ] = hNode;
    }

    if( Node.hHashNext.IsNull() == FALSE )
        m_lNode( Node.hHashNext ).hHashPrev = hNode;
}


//=========================================================================

xhandle vertex_mgr::AllocNode( s32 nItems, xbool bVertex, s32 Stride, DWORD FVF )
{
    xhandle     hNode;

    hNode.Handle = HNULL;
    ASSERT( Stride != 0 );

    //
    // Do search throw the hash table and see if we find a good node.
    // 
    for( s32 HashEntry = GetHashEntry( nItems, bVertex ); HashEntry < NUM_HASH_ENTRIES; HashEntry++ )
    {
        if( bVertex )
        {
            hNode = m_VertHash[ HashEntry ];
        }
        else
        {
            hNode = m_IndexHash[ HashEntry ];
        }

        for( ; hNode.IsNull() == FALSE; hNode = m_lNode( hNode ).hHashNext )
        {
            node& Node = m_lNode( hNode );

            if( Node.Count > nItems )
                break;
        }

        if( hNode.IsNull() == FALSE )
            break;
    }

    //
    // Okay we didn't find any hole in any of the pools to put this vertex set.
    // Here we have two options. One try to clean all vertex buffers (compact them)
    // Hopping to get some hole big enoft and then try to find the hole ones again or
    // just allocate another vertex pool.
    // DONE: For now we will just allocate another vertex pool.
    //
    if( hNode.IsNull() ) 
    {
        //
        // Create the physical pool
        //
        dxerr           Error;
        xhandle         hPool;
        pool*           pPool;

        if( bVertex )
        {
            vertex_pool&    Pool = m_lVertexPool.Add( hPool );

            pPool           = &Pool;

            Pool.nItems     = MAX_VERTEX_POOL;
            Pool.FVF        = FVF;
            Pool.Stride     = Stride;
            Pool.pVertex    = NULL;

            if( g_pd3dDevice )
            {
                Error = g_pd3dDevice->CreateVertexBuffer( Pool.Stride * Pool.nItems, 
                                                          D3DUSAGE_WRITEONLY,
                                                          (Pool.FVF<100)?0:Pool.FVF,
                                                          D3DPOOL_MANAGED,
                                                          &Pool.pVertex,
                                                          NULL );

                if( Error )
                {
                    m_lVertexPool.DeleteByHandle( hPool );
                    x_throw( "Unable to create a new vetex buffer in D3D" );
                }
            }
            else
            {
                // LIE!!!!
                Pool.pVertex = (IDirect3DVertexBuffer9*)x_malloc( Pool.Stride * Pool.nItems );
            }
        }
        else
        {
            index_pool&    Pool = m_lIndexPool.Add( hPool );

            pPool           = &Pool;

            Pool.nItems     = MAX_INDEX_POOL;
            Pool.Stride     = Stride;
            Pool.pIndex     = NULL;

            if( g_pd3dDevice )
            {
                Error = g_pd3dDevice->CreateIndexBuffer( Pool.Stride * Pool.nItems, 
                                                         D3DUSAGE_WRITEONLY,
                                                         Pool.Stride==sizeof(u16)?D3DFMT_INDEX16:D3DFMT_INDEX32,
                                                         D3DPOOL_MANAGED,
                                                         &Pool.pIndex,
                                                         NULL );

                ASSERT( Pool.Stride == sizeof(u16) );
                if( Error )
                {
                    m_lIndexPool.DeleteByHandle( hPool );
                    x_throw( "Unable to create a new index buffer in D3D" );
                }
            }
            else
            {
                // LIE!!!!
                Pool.pIndex = (IDirect3DIndexBuffer9*)x_malloc( Pool.Stride * Pool.nItems );
            }
        }

        //
        // Add the empty node that represents the new pool
        //
        node&   EmptyNode  = m_lNode.Add( hNode );
        EmptyNode.Count    = pPool->nItems;
        EmptyNode.Offset   = 0;
        EmptyNode.hPool    = hPool;
        EmptyNode.Flags    = bVertex?FLAGS_VERTEX:0;
        EmptyNode.User     = -1;

        // Set the global next and prev
        EmptyNode.hGlobalNext.Handle = HNULL;
        EmptyNode.hGlobalPrev.Handle = HNULL;

        // Insert the node into the hash
        AddNodeToHash( hNode, bVertex );

        // Set in the pool's first node
        pPool->hFirstNode = hNode;
    }

    //
    // Now we need to alloc the node and possibly add the rest of the empty space
    // back into a node. 
    //
    ASSERT( hNode.IsNull() == FALSE );

    //
    // Do a quick sanity check
    //
    {
        node& Node      = m_lNode( hNode );
        ASSERT( Node.Count >= nItems );
        ASSERT( Node.hGlobalNext.Handle == -1 || Node.hGlobalNext.Handle >= 0 );
        ASSERT( Node.hGlobalPrev.Handle == -1 || Node.hGlobalPrev.Handle >= 0 );
        ASSERT( Node.hHashNext.Handle   == -1 || Node.hHashNext.Handle   >= 0 );
        ASSERT( Node.hHashPrev.Handle   == -1 || Node.hHashPrev.Handle   >= 0 );
        ASSERT( (Node.Flags & FLAGS_FULL) == 0 );
    }

    //
    // Remove the node from the hash table
    //
    RemoveNodeFormHash( hNode, bVertex );

    //
    // Check whether we need to create an empty node or not. If so we must insert an
    // empty node into the list
    //
    s32   Bias      = 100;
    if( ( m_lNode( hNode ).Count - Bias ) > nItems )
    {        
        xhandle hEmptyNode;
        node&   EmptyNode  = m_lNode.Add( hEmptyNode );
        node&   Node       = m_lNode( hNode );
        EmptyNode.Count    = Node.Count  - nItems;
        EmptyNode.Offset   = Node.Offset + nItems;
        EmptyNode.Flags    = bVertex?FLAGS_VERTEX:0;
        EmptyNode.User     = -1;
        EmptyNode.hPool    = Node.hPool;

        // Set the global next and prev
        EmptyNode.hGlobalNext = Node.hGlobalNext;
        EmptyNode.hGlobalPrev = hNode;
        Node.hGlobalNext = hEmptyNode;

        if( EmptyNode.hGlobalNext.IsNull() == FALSE )
            m_lNode( EmptyNode.hGlobalNext ).hGlobalPrev = hEmptyNode;

        // Insert the node into the hash
        AddNodeToHash( hEmptyNode, bVertex );

        // Now let's reset the node with the right count
        Node.Count = nItems;
    }

    //
    // Set the number of items that were actually allocated
    //
    m_lNode( hNode ).User   = nItems;

    return hNode;
}

//=========================================================================

void* vertex_mgr::LockDListVerts( xhandle hDList )
{
    BYTE*           pDest = NULL;
    node&           Node  = m_lNode( m_lDList(hDList).hVertexNode );
    vertex_pool&    Pool  = m_lVertexPool( Node.hPool );
    dxerr           Error;
    ASSERT( (Node.Flags&FLAGS_VERTEX) == FLAGS_VERTEX );

    if( g_pd3dDevice )
    {
        Error = Pool.pVertex->Lock( Pool.Stride*Node.Offset, Pool.Stride*Node.User, (void**)&pDest, 0 );
        if( Error != 0 )
        {
            x_throw( "Unable to lock the dest vertex buffer in D3D" );
        }

        return pDest;
    }
    else
    {   
        pDest = ((BYTE*)Pool.pVertex) + (Pool.Stride*Node.Offset);
        return pDest;
    }
}

//=========================================================================

void vertex_mgr::UnlockDListVerts( xhandle hDList )
{
    node&           Node  = m_lNode( m_lDList(hDList).hVertexNode );
    vertex_pool&    Pool  = m_lVertexPool( Node.hPool );
    dxerr           Error;
    ASSERT( (Node.Flags&FLAGS_VERTEX) == FLAGS_VERTEX );

    if( g_pd3dDevice )
    {
        Error = Pool.pVertex->Unlock();
        if( Error != 0 )
        {
            x_throw( "Unable to unlock the Index buffer in D3D" );
        }
    }
    else
    {
        // Don't need to do anything
    }
}

//=========================================================================

void* vertex_mgr::LockDListIndices( xhandle hDList, s32& Index )
{
    BYTE*        pDest = NULL;
    node&        IndexNode   = m_lNode( m_lDList(hDList).hIndexNode );
    node&        VertexNode  = m_lNode( m_lDList(hDList).hVertexNode );
    index_pool&  Pool  = m_lIndexPool( IndexNode.hPool );
    dxerr        Error;
    ASSERT( (IndexNode.Flags&FLAGS_VERTEX) == FALSE );

    if( g_pd3dDevice )
    {
        Error = Pool.pIndex->Lock( Pool.Stride*IndexNode.Offset, Pool.Stride*IndexNode.User, (void**)&pDest, 0 );
        if( Error != 0 )
        {
            x_throw( "Unable to lock the Index buffer in D3D" );
        }

        Index = VertexNode.Offset;
        return pDest;
    }
    else
    {
        Index = VertexNode.Offset;
        pDest = ((BYTE*)Pool.pIndex) + (Pool.Stride*IndexNode.Offset);
        return pDest;
    }

}

//=========================================================================

void vertex_mgr::UnlockDListIndices( xhandle hDList )
{
    node&           Node  = m_lNode( m_lDList(hDList).hIndexNode );
    index_pool&     Pool  = m_lIndexPool( Node.hPool );
    dxerr           Error;
    ASSERT( (Node.Flags&FLAGS_VERTEX) == FALSE );

    if( g_pd3dDevice )
    {
        Error = Pool.pIndex->Unlock();
        if( Error != 0 )
        {
            x_throw( "Unable to unlock the Index buffer in D3D" );
        }
    }
    else
    {
        // Don't need to do anything
    }
}

//=========================================================================

xhandle vertex_mgr::AllocVertexSet( s32 nVertices, s32 Stride, DWORD FVF )
{
    ASSERT( MAX_VERTEX_POOL == 0xffff );
    if( nVertices > 0xffff )
        x_throw( "Unable to allocated object that have more than 65,000 vertices" );

    return AllocNode( nVertices, TRUE, Stride, FVF );
}

//=========================================================================

xhandle vertex_mgr::AllocIndexSet( s32 nIndices )
{
    if( nIndices > (MAX_INDEX_POOL/3) )
        x_throw( xfs( "Unable to allocated object that have more than %d facets", (MAX_INDEX_POOL/3) ) );

    return AllocNode( nIndices, FALSE, sizeof(u16) );
}

//=========================================================================

void vertex_mgr::FreeNode( xhandle hNode, xbool bVertex )
{
    //
    // First thing lets do any merging
    //
    {
        node&   Node  = m_lNode( hNode );

        //
        // Can we merge on the right?
        //
        if( Node.hGlobalNext.IsNull() == FALSE && (m_lNode( Node.hGlobalNext ).Flags & FLAGS_FULL) == 0 ) 
        {
            xhandle hDelNode = Node.hGlobalNext;
            node&   DelNode  = m_lNode( Node.hGlobalNext );

            // First lets remove the node from the hash
            RemoveNodeFormHash( hDelNode, bVertex );

            // Now remove the node from the global list
            Node.Count += DelNode.Count;
            Node.hGlobalNext = DelNode.hGlobalNext;

            if( Node.hGlobalNext.IsNull() == FALSE )
                m_lNode( Node.hGlobalNext ).hGlobalPrev = hNode;

            m_lNode.DeleteByHandle( hDelNode );        
        }

        //
        // Can we merge Left
        //
        if( Node.hGlobalPrev.IsNull() == FALSE && (m_lNode( Node.hGlobalPrev ).Flags & FLAGS_FULL) == 0 ) 
        {
            xhandle hNewNode = Node.hGlobalPrev;
            node&   NewNode  = m_lNode( Node.hGlobalPrev );

            // First lets remove the new node from the hash
            RemoveNodeFormHash( hNewNode, bVertex );

            // Now lets merge
            NewNode.Count += Node.Count;

            NewNode.hGlobalNext = Node.hGlobalNext;

            if( Node.hGlobalNext.IsNull() == FALSE )
                m_lNode( NewNode.hGlobalNext ).hGlobalPrev = hNewNode;

            m_lNode.DeleteByHandle( hNode );        

            // Set the new handle
            hNode = hNewNode;
        }
    }

    //
    // Now we must add our node to the hash
    //
    AddNodeToHash( hNode, bVertex );
}

//=========================================================================

void vertex_mgr::DelDList( xhandle hDList )
{
    dlist&  DList = m_lDList( hDList );

    if( DList.hVertexNode.IsNull() == FALSE )
        FreeNode( DList.hVertexNode, TRUE );

    if( DList.hIndexNode.IsNull() == FALSE )
        FreeNode( DList.hIndexNode, FALSE );

    m_lDList.DeleteByHandle( hDList );
}

//=========================================================================

xhandle vertex_mgr::AddDList( void* pVertex, s32 nVertices, u16* pIndex, s32 nIndices, s32 nPrims )
{
    xhandle hDList;
    dlist&  DList = m_lDList.Add( hDList );

    ASSERT( m_Stride > 0 );
    ASSERT( nPrims*3 == nIndices );
    DList.hVertexNode.Handle = HNULL;
    DList.hIndexNode.Handle  = HNULL;
    DList.nPrims             = nPrims;

    x_try;

    DList.hIndexNode  = AllocIndexSet   ( nIndices );
    DList.hVertexNode = AllocVertexSet  ( nVertices, m_Stride, m_FVF );

    // Copy the verts
    {
        void* pNewVert;
        pNewVert = LockDListVerts( hDList );
        x_memcpy( pNewVert, pVertex, m_Stride*nVertices );
        UnlockDListVerts( hDList );
    }

    // Copy the Indices
    static s32 b=1;
    //if( b )
    {
        b=0;
        u16* pNewIndex;
        node& VNode = m_lNode( DList.hVertexNode );
        node& INode = m_lNode( DList.hIndexNode );
        s32   Index;

        pNewIndex = (u16*)LockDListIndices( hDList, Index );
        for( s32 i=0; i<nIndices; i++ )
        {
            ASSERT( ((s32)pIndex[i] + VNode.Offset) < MAX_VERTEX_POOL ) ;
            ASSERT( pIndex[i] < VNode.User );
            pNewIndex[i] = pIndex[i] + VNode.Offset;
        }
        UnlockDListIndices( hDList );
    }

    x_catch_begin;
    
    DelDList( hDList );

    x_catch_end_ret;

    return hDList;
}

//=========================================================================

void vertex_mgr::InvalidateCache( void )
{
    m_LastVertexPool.Handle = HNULL;
    m_LastIndexPool.Handle  = HNULL;
}

//=========================================================================

void vertex_mgr::BeginRender( void )
{        
    InvalidateCache();
}

//=========================================================================

void vertex_mgr::ActivateStreams( xhandle hDList )
{
    dlist& DList  = m_lDList( hDList );
    node&  Vertex = m_lNode ( DList.hVertexNode );
    node&  Index  = m_lNode ( DList.hIndexNode  );

    //
    // Handle mini cash
    //
    if( Vertex.hPool != m_LastVertexPool  )
    {
        vertex_pool& Pool = m_lVertexPool( Vertex.hPool );
        if( g_pd3dDevice )
        {
            g_pd3dDevice->SetFVF( Pool.FVF );
            g_pd3dDevice->SetStreamSource( 0, Pool.pVertex, 0, Pool.Stride );
        }
        m_LastVertexPool = Vertex.hPool;
    }

    if( Index.hPool != m_LastIndexPool  )
    {
        index_pool& Pool = m_lIndexPool( Index.hPool );
        if( g_pd3dDevice )
        {
            g_pd3dDevice->SetIndices( Pool.pIndex );
        }
        m_LastIndexPool = Index.hPool;
    }
}

//=========================================================================

void vertex_mgr::DrawDList( xhandle hDList, D3DPRIMITIVETYPE PrimType )
{
    ASSERT( hDList.Handle >= 0 );

    dlist& DList  = m_lDList( hDList );
    node&  Vertex = m_lNode ( DList.hVertexNode );
    node&  Index  = m_lNode ( DList.hIndexNode  );

    //
    // Activate streams
    //
    ActivateStreams( hDList );

    //
    // Sanity check
    //

#ifdef INTERVELOP
    s32 IndexOffset;

    struct vertex 
    {
        vector3p Pos;
        float GetX() const { return Pos.X; }
        float GetY() const { return Pos.Y; }
        float GetZ() const { return Pos.Z; }
    };
    
    vertex* pVert = (vertex*)LockDListVerts( hDList );
    u16* pIndx = (u16*)LockDListIndices( hDList, IndexOffset );

    for (s32 i = 0; i < DList.nPrims; i++)
    {
        x_DebugMsg( "%f %f %f -- ", 
            pVert[ pIndx[ i * 3 + 0 ] - Vertex.Offset ].GetX(),
            pVert[ pIndx[ i * 3 + 0 ] - Vertex.Offset ].GetY(),
            pVert[ pIndx[ i * 3 + 0 ] - Vertex.Offset ].GetZ());

        x_DebugMsg( "%f %f %f -- ", 
            pVert[ pIndx[ i * 3 + 1 ] - Vertex.Offset ].GetX(),
            pVert[ pIndx[ i * 3 + 1 ] - Vertex.Offset ].GetY(),
            pVert[ pIndx[ i * 3 + 1 ] - Vertex.Offset ].GetZ());

        x_DebugMsg( "%f %f %f\n ", 
            pVert[ pIndx[ i * 3 + 2 ] - Vertex.Offset ].GetX(),
            pVert[ pIndx[ i * 3 + 2 ] - Vertex.Offset ].GetY(),
            pVert[ pIndx[ i * 3 + 2 ] - Vertex.Offset ].GetZ());
    }
    UnlockDListVerts(hDList);
    UnlockDListIndices(hDList);
#endif        


    //
    // Get ready to render now
    //
    if( g_pd3dDevice )
    {
        ASSERT( DList.nPrims*3 == Index.User );
        ASSERT( (Index.Offset + Index.User) < MAX_INDEX_POOL );
        g_pd3dDevice->DrawIndexedPrimitive( PrimType, 
                                            0,
                                            Vertex.Offset, 
                                            Vertex.User, 
                                            Index.Offset, 
                                            DList.nPrims );
    }
}

//=========================================================================
void vertex_mgr::Kill( void )
{
    s32 i;

    for( i=0; i<m_lIndexPool.GetCount(); i++ )
    {
        if( m_lIndexPool[i].pIndex ) 
        {
            if( g_pd3dDevice )
            {
                m_lIndexPool[i].pIndex->Release();
                m_lIndexPool[i].pIndex = NULL;
            }
            else
            {
                x_free( m_lIndexPool[i].pIndex );
                m_lIndexPool[i].pIndex = NULL;
            }
        }
    }

    for( i=0; i<m_lVertexPool.GetCount(); i++ )
    {
        if( m_lVertexPool[i].pVertex ) 
        {
            if( g_pd3dDevice )
            {
                m_lVertexPool[i].pVertex->Release();
                m_lVertexPool[i].pVertex = NULL;
            }
            else
            {
                x_free(m_lVertexPool[i].pVertex);
                m_lVertexPool[i].pVertex = NULL;
            }
        }
    }

    m_lIndexPool.Clear();
    m_lVertexPool.Clear();
}
