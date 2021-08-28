//=============================================================================
//
// VU Skinning optimizer - original planning by TA and DS, coding by DS
//
//=============================================================================

#include "faststrip.hpp"
#include "PS2SkinOptimizer.hpp"

#define BACKFACE_CULLING    TRUE

//=============================================================================

ps2skin_optimizer::ps2skin_optimizer( void )
{
    Reset();
}

//=============================================================================

ps2skin_optimizer::~ps2skin_optimizer( void )
{
}

//=============================================================================

void ps2skin_optimizer::Reset( void )
{
    ClearMatrixCache();
    m_bOptimized = FALSE;
    m_lOrigTris.Clear();
    m_lTriBatches.Clear();
    m_lFinalBatches.Clear();
    m_lOrigTris.SetCapacity( 1000 );
    m_lTriBatches.SetCapacity( 20 );
    m_lFinalBatches.SetCapacity( 20 );
}

//=============================================================================

void ps2skin_optimizer::AddTri( const optimizer_tri& Tri )
{
    ASSERT( !m_bOptimized );

    optimizer_tri& NewTri = m_lOrigTris.Append();
    NewTri                = Tri;
    NewTri.Added          = FALSE;

    // sanity check--make sure all of the verts are weighted to something
    for ( s32 iVert = 0; iVert < 3; iVert++ )
    {
        const optimizer_vert& Vert = Tri.Verts[iVert];
        for ( s32 iVertWeight = 0; iVertWeight < Vert.nWeights; iVertWeight++ )
        {
            ASSERT( Vert.iOrigBones[iVertWeight] >= 0 );
        }
    }
}

//=============================================================================

void ps2skin_optimizer::Optimize( void )
{
    m_TotalTime.Reset();
    m_TotalTime.Start();

    ASSERT( !m_bOptimized );
    m_bOptimized = TRUE;

    // early out
    if ( m_lOrigTris.GetCount() == 0 )
    {
        m_TotalTime.Stop();
        return;
    }

    // Determine the original vertex index range
    s32 MaxOrigIndex = 0;
    {
        for( s32 i=0; i<m_lOrigTris.GetCount(); i++ )
        {
            MaxOrigIndex = MAX(MaxOrigIndex,m_lOrigTris[i].Verts[0].iOrigIndex);
            MaxOrigIndex = MAX(MaxOrigIndex,m_lOrigTris[i].Verts[1].iOrigIndex);
            MaxOrigIndex = MAX(MaxOrigIndex,m_lOrigTris[i].Verts[2].iOrigIndex);
        }
        m_pVertInBatch = (byte*)x_malloc(sizeof(byte)*(MaxOrigIndex+1));
        x_memset(m_pVertInBatch,0,sizeof(byte)*(MaxOrigIndex+1));
    }

    // set up for the optimization loop...note that we must have at least
    // one tri batch
    m_CacheBuildTime.Reset();
    m_CacheBuildTime.Start();
    tri_batch* pCurrentBatch = &m_lTriBatches.Append();

    // the optimization loop
    xbool bDone = FALSE;
    while ( !bDone )
    {
        s32 BestTriScore = 100000;
        s32 BestTri      = -1;

        // find the best candidate for adding to the tri batch
        for ( s32 i = 0; i < m_lOrigTris.GetCount(); i++ )
        {
            s32            TriScore = 0;
            optimizer_tri& Tri      = m_lOrigTris[i];
            
            // can't add a tri that's already been added to a list
            if ( Tri.Added )
                continue;

            // if the tri doesn't require any matrix loads, don't even
            // bother with scoring the rest...we should add it to this
            // batch
            s32 BonesNotInCache = FindMatrixCacheIndices( Tri );
            if ( BonesNotInCache == 0 )
            {
                BestTri = i;
                break;
            }

            // otherwise, penalize the tri for how many matrices it
            // needs to load
            TriScore += BonesNotInCache * 1000;

            // Give the tri a bonus for sharing verts with tris already in the batch
            //s32 NSharedVerts = CountSharedVerts( *pCurrentBatch, Tri );
            s32 NSharedVerts = 0;
            if( m_pVertInBatch[ Tri.Verts[0].iOrigIndex ] ) NSharedVerts++;
            if( m_pVertInBatch[ Tri.Verts[1].iOrigIndex ] ) NSharedVerts++;
            if( m_pVertInBatch[ Tri.Verts[2].iOrigIndex ] ) NSharedVerts++;

            TriScore -= NSharedVerts * 300;

            // was this the best tri?
            if ( TriScore < BestTriScore )
            {
                BestTriScore = TriScore;
                BestTri      = i;
            }
        }

        // add the best tri to our batch
        if ( BestTri == -1 )
        {
            bDone = TRUE;

            // sanity check
            for ( s32 i = 0; i < m_lOrigTris.GetCount(); i++ )
            {
                ASSERT( m_lOrigTris[i].Added == TRUE );
            }
        }
        else
        {
            optimizer_tri& TriToAdd = m_lOrigTris[BestTri];

            // add matrices to the cache if necessary
            s32 BonesNotInCache = FindMatrixCacheIndices( TriToAdd );
            if ( BonesNotInCache )
            {
                xbool Success = AddMatricesToCache( TriToAdd, *pCurrentBatch );
                if ( !Success )
                {
                    // sanity check...make sure we have tris before making a new batch
                    ASSERT( pCurrentBatch->Tris.GetCount() );

                    // the matrix cache must be full, create a new batch, flush the
                    // matrix cache, and start again.
                    pCurrentBatch = &m_lTriBatches.Append();
                    FlushMatrixCache();
                    Success = AddMatricesToCache( TriToAdd, *pCurrentBatch );
                    if ( !Success )
                    {
                        // this should never (well, almost never) happen. Basically, this
                        // means our caching algorithm didn't do a very good job and we've
                        // got a bunch of matrices in there that we can't make any more
                        // tris from, yet there's still tris to be rendered from the set.
                        // Empty the cache completely and start again.
                        ClearMatrixCache();
                        Success = AddMatricesToCache( TriToAdd, *pCurrentBatch );
                        ASSERT( Success );
                    }

                    // Clear list of verts in batch
                    x_memset(m_pVertInBatch,0,sizeof(byte)*(MaxOrigIndex+1));
                }
            }

            // now the matrices are guaranteed to be in the cache, add the triangle to our batch
            TriToAdd.Added = TRUE;
            pCurrentBatch->Tris.Append() = TriToAdd;
            m_pVertInBatch[ TriToAdd.Verts[0].iOrigIndex ] = 1;
            m_pVertInBatch[ TriToAdd.Verts[1].iOrigIndex ] = 1;
            m_pVertInBatch[ TriToAdd.Verts[2].iOrigIndex ] = 1;
        }
    }

    // sanity check time...
    s32 NTotalTris = 0;
    for ( s32 iBatch = 0; iBatch < m_lTriBatches.GetCount(); iBatch++ )
    {
        tri_batch& Batch = m_lTriBatches[iBatch];
        ASSERT( m_lTriBatches[iBatch].Tris.GetCount() );
        NTotalTris += m_lTriBatches[iBatch].Tris.GetCount();
    }
    ASSERT( NTotalTris == m_lOrigTris.GetCount() );
    m_CacheBuildTime.Stop();

    x_free(m_pVertInBatch);

    // finally we can strip it and build the final batches
    m_StripBuildTime.Reset();
    m_StripBuildTime.Start();
    BuildFinalBatches();
    m_StripBuildTime.Stop();
    m_TotalTime.Stop();
}

//=============================================================================

void ps2skin_optimizer::PrintStats( void )
{
    ASSERT( m_bOptimized );
    
    // count the number of unique matrices and duplicate matrix loads
    xarray<s32> UniqueMatrices;
    s32 NDupeMatrixUploads = 0;
    s32 iBatch;
    for ( iBatch = 0; iBatch < m_lFinalBatches.GetCount(); iBatch++ )
    {
        final_batch& FinalBatch = m_lFinalBatches[iBatch];
        for ( s32 iMatrix = 0; iMatrix < FinalBatch.nBonesToLoad; iMatrix++ )
        {
            if ( UniqueMatrices.Find( FinalBatch.iOrigBones[iMatrix] ) == -1 )
                UniqueMatrices.Append( FinalBatch.iOrigBones[iMatrix] );
            else
                NDupeMatrixUploads++;
        }
    }

    // figure out the theoretical best verts/tri ratio
    xarray<s32> OrigIndices;
    for ( s32 iTri = 0; iTri < m_lOrigTris.GetCount(); iTri++ )
    {
        optimizer_tri& OrigTri = m_lOrigTris[iTri];
        if ( OrigIndices.Find(OrigTri.Verts[0].iOrigIndex) == -1 )
            OrigIndices.Append(OrigTri.Verts[0].iOrigIndex);
        if ( OrigIndices.Find(OrigTri.Verts[1].iOrigIndex) == -1 )
            OrigIndices.Append(OrigTri.Verts[1].iOrigIndex);
        if ( OrigIndices.Find(OrigTri.Verts[2].iOrigIndex) == -1 )
            OrigIndices.Append(OrigTri.Verts[2].iOrigIndex);
    }
    f32 fBestVertsPerTri;
    if ( m_lOrigTris.GetCount() )
        fBestVertsPerTri = (f32)OrigIndices.GetCount() / (f32)m_lOrigTris.GetCount();
    else
        fBestVertsPerTri = 0.0f;

    // figure out what our verts/tri ratio really is
    s32 NFinalVerts = 0;
    s32 NVertBuffers = 0;
    for ( iBatch = 0; iBatch < m_lFinalBatches.GetCount(); iBatch++ )
    {
        final_batch& Batch = m_lFinalBatches[iBatch];
        ASSERT( Batch.Verts.GetCount() );
        s32 Count = Batch.Verts.GetCount();
        NVertBuffers += (Count / kSkinVertBufferSize) + ((Count%kSkinVertBufferSize)?1:0);
        NFinalVerts  += Count;
    }
    f32 fVertsPerTri;
    if ( m_lOrigTris.GetCount() )
        fVertsPerTri = (f32)NFinalVerts / (f32)m_lOrigTris.GetCount();
    else
        fVertsPerTri = 0.0f;

    // figure out if we're soft skinning or rigid skinning each batch
    s32 NOneInfluenceVerts = 0;
    s32 NTwoInfluenceVerts = 0;
    s32 NSkinnedBatches    = 0;
    s32 NRigidBatches      = 0;
    for ( iBatch = 0; iBatch < m_lFinalBatches.GetCount(); iBatch++ )
    {
        final_batch& Batch  = m_lFinalBatches[iBatch];
        xbool        bRigid = TRUE;
        for ( s32 iVert = 0; iVert < Batch.Verts.GetCount(); iVert++ )
        {
            optimizer_vert& Vert = Batch.Verts[iVert];
            if ( Vert.nWeights > 1 )
            {
                NTwoInfluenceVerts++;
                bRigid = FALSE;
            }
            else
            {
                NOneInfluenceVerts++;
            }
        }

        if ( bRigid )
            NRigidBatches++;
        else
            NSkinnedBatches++;
    }
    ASSERT( (NOneInfluenceVerts + NTwoInfluenceVerts) == NFinalVerts );
    ASSERT( (NSkinnedBatches+NRigidBatches) == m_lFinalBatches.GetCount() );

    x_printf( "                  NUniqueVerts = %d\n",    OrigIndices.GetCount()     );
    x_printf( "                    NInputTris = %d\n",    m_lOrigTris.GetCount()     );
    x_printf( "            NDifferentMatrices = %d\n",    UniqueMatrices.GetCount()  );
    x_printf( "               NSkinnedBatches = %d\n",    NSkinnedBatches            );
    x_printf( "                 NRigidBatches = %d\n",    NRigidBatches              );
    x_printf( "                NMatrixPackets = %d\n",    m_lFinalBatches.GetCount() );
    x_printf( "       NDuplicateMatrixUploads = %d\n",    NDupeMatrixUploads         );
    x_printf( "            NOneInfluenceVerts = %d\n",    NOneInfluenceVerts         );
    x_printf( "            NTwoInfluenceVerts = %d\n",    NTwoInfluenceVerts         );
    x_printf( "                  NVertBuffers = %d\n",    NVertBuffers               );
    x_printf( "                 Verts per Tri = %3.3f\n", fVertsPerTri               );
    x_printf( "Theoretical best verts per tri = %3.3f\n", fBestVertsPerTri           );
    x_printf( "       Total time to build (s) = %3.3f\n", m_TotalTime.ReadSec()      );
    x_printf( "       Time to build cache (s) = %3.3f\n", m_CacheBuildTime.ReadSec() );
    x_printf( "      Time to build strips (s) = %3.3f\n", m_StripBuildTime.ReadSec() );
}

//=============================================================================

s32 ps2skin_optimizer::GetNFinalBatches( void )
{
    ASSERT( m_bOptimized );
    return m_lFinalBatches.GetCount();
}

//=============================================================================

const ps2skin_optimizer::final_batch& ps2skin_optimizer::GetFinalBatch( s32 Index )
{
    ASSERT( m_bOptimized );
    ASSERT( (Index >= 0) && (Index < m_lFinalBatches.GetCount()) );
    return m_lFinalBatches[Index];
}

//=============================================================================

ps2skin_optimizer::tri_batch::tri_batch()
{
    nBonesToLoad = 0;
    Tris.Clear();
    Tris.SetCapacity( 50 );
};

//=============================================================================

xbool ps2skin_optimizer::optimizer_vert::operator== ( const ps2skin_optimizer::optimizer_vert& V ) const
{
    return ( iOrigIndex == V.iOrigIndex );
}

//=============================================================================

s32 ps2skin_optimizer::FindMatrixCacheIndices  ( optimizer_tri& Tri )
{
    s32 nNewBones = 0;
    s32 iNewBones[kMaxWeights*3];

    for ( s32 iVert = 0; iVert < 3; iVert++ )
    {
        optimizer_vert& Vert = Tri.Verts[iVert];
        for ( s32 iVertWeight = 0; iVertWeight < Vert.nWeights; iVertWeight++ )
        {
            // is the weight already in the cache?
            Vert.iCacheBones[iVertWeight] = -1;
            for ( s32 iCacheEntry = 0; iCacheEntry < kMatrixCacheSize; iCacheEntry++ )
            {
                if ( m_MatrixCache[iCacheEntry] == Vert.iOrigBones[iVertWeight] )
                {
                    Vert.iCacheBones[iVertWeight] = iCacheEntry;
                    break;
                }
            }

            // no? have we already put it in our bone add to-do list?
            if ( Vert.iCacheBones[iVertWeight] == -1 )
            {
                s32 iNewBone;
                for ( iNewBone = 0; iNewBone < nNewBones; iNewBone++ )
                {
                    if ( iNewBones[iNewBone] == Vert.iOrigBones[iVertWeight] )
                        break;
                }

                // no? then its one we would have to add
                if ( iNewBone == nNewBones )
                {
                    iNewBones[nNewBones] = Vert.iOrigBones[iVertWeight];
                    nNewBones++;
                }
            }
        }
    }

    return nNewBones;
}

//=============================================================================

xbool ps2skin_optimizer::AddMatricesToCache( optimizer_tri& Tri, tri_batch& Batch )
{
    for ( s32 iVert = 0; iVert < 3; iVert++ )
    {
        optimizer_vert& Vert = Tri.Verts[iVert];
        for ( s32 iVertWeight = 0; iVertWeight < Vert.nWeights; iVertWeight++ )
        {
            // is the weight already in the cache?
            s32 iCacheEntry;
            Vert.iCacheBones[iVertWeight] = -1;
            for ( iCacheEntry = 0; iCacheEntry < kMatrixCacheSize; iCacheEntry++ )
            {
                if ( m_MatrixCache[iCacheEntry] == Vert.iOrigBones[iVertWeight] )
                {
                    Vert.iCacheBones[iVertWeight] = iCacheEntry;
                    break;
                }
            }

            if ( iCacheEntry == kMatrixCacheSize )
            {
                // no? then we should add it
                for ( iCacheEntry = 0; iCacheEntry < kMatrixCacheSize; iCacheEntry++ )
                {
                    if ( m_MatrixCache[iCacheEntry] == -1 )
                    {
                        Vert.iCacheBones[iVertWeight]           = iCacheEntry;
                        m_MatrixCache[iCacheEntry]              = Vert.iOrigBones[iVertWeight];
                        Batch.iOrigBones[Batch.nBonesToLoad]    = Vert.iOrigBones[iVertWeight];
                        Batch.iCacheBones[Batch.nBonesToLoad]   = iCacheEntry;
                        Batch.nBonesToLoad++;
                        break;
                    }
                }

                // no room? then we have failed in our task
                if ( iCacheEntry == kMatrixCacheSize )
                {
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}

//=============================================================================

void ps2skin_optimizer::FlushMatrixCache( void )
{
    //
    // find any matrices that are in the cache, but aren't necessary any more,
    // and flush them out
    //
    for ( s32 iCacheEntry = 0; iCacheEntry < kMatrixCacheSize; iCacheEntry++ )
    {
        xbool bStillNeeded = FALSE;

        // is the slot already empty?
        if ( m_MatrixCache[iCacheEntry] == -1 )
            continue;

        // if not, is it used?
        for ( s32 iTri = 0; (iTri < m_lOrigTris.GetCount()) && !bStillNeeded; iTri++ )
        {
            const optimizer_tri& Tri = m_lOrigTris[iTri];
            if ( Tri.Added )
                continue;

            for ( s32 iVert = 0; (iVert < 3) && !bStillNeeded; iVert++ )
            {
                const optimizer_vert& Vert = Tri.Verts[iVert];
                for ( s32 iVertWeight = 0; (iVertWeight < Vert.nWeights) && !bStillNeeded; iVertWeight++ )
                {
                    if ( m_MatrixCache[iCacheEntry] == Vert.iOrigBones[iVertWeight] )
                        bStillNeeded = TRUE;
                }
            }
        }

        // if its not used, clear out the cache entry
        if ( !bStillNeeded )
        {
            m_MatrixCache[iCacheEntry] = -1;
        }
    }
}

//=============================================================================

void ps2skin_optimizer::ClearMatrixCache( void )
{
    for ( s32 i = 0; i < kMatrixCacheSize; i++ )
        m_MatrixCache[i] = -1;
}

//=============================================================================

void ps2skin_optimizer::BuildFinalBatches( void )
{
    faststrip Strip;

    //
    // Strip each triangle batch into a "final" batch
    //
    for ( s32 i = 0; i < m_lTriBatches.GetCount(); i++ )
    {
        tri_batch& TriBatch = m_lTriBatches[i];

        // copy the easy parts of the final batch
        final_batch& FinalBatch = m_lFinalBatches.Append();
        FinalBatch.nBonesToLoad = TriBatch.nBonesToLoad;
        for ( s32 iBone = 0; iBone < FinalBatch.nBonesToLoad; iBone++ )
        {
            FinalBatch.iOrigBones[iBone]  = TriBatch.iOrigBones[iBone];
            FinalBatch.iCacheBones[iBone] = TriBatch.iCacheBones[iBone];
        }

        // create a new vert pool for the stripper to use...the stripper needs
        // indices based on position, etc. but we need results to be in
        // optimizer_verts.
        xarray<optimizer_vert> StripperVertPool;
        StripperVertPool.SetCapacity( 30 );
        ASSERT( TriBatch.Tris.GetCount() );
        Strip.Open( 0, BACKFACE_CULLING );
        for ( s32 iTri = 0; iTri < TriBatch.Tris.GetCount(); iTri++ )
        {
            // get the tri indices in our vert pool
            s32 VertIndices[3];
            VertIndices[0] = StripperVertPool.Find( TriBatch.Tris[iTri].Verts[0] );
            VertIndices[1] = StripperVertPool.Find( TriBatch.Tris[iTri].Verts[1] );
            VertIndices[2] = StripperVertPool.Find( TriBatch.Tris[iTri].Verts[2] );
            if ( VertIndices[0] == -1 )
            {
                VertIndices[0] = StripperVertPool.GetCount();
                StripperVertPool.Append( TriBatch.Tris[iTri].Verts[0] );
            }
            if ( VertIndices[1] == -1 )
            {
                VertIndices[1] = StripperVertPool.GetCount();
                StripperVertPool.Append( TriBatch.Tris[iTri].Verts[1] );
            }
            if ( VertIndices[2] == -1 )
            {
                VertIndices[2] = StripperVertPool.GetCount();
                StripperVertPool.Append( TriBatch.Tris[iTri].Verts[2] );
            }

            // now add the tri to the stripper
            Strip.AddTri( VertIndices[0], VertIndices[1], VertIndices[2] );
        }
        Strip.Close();

        // now we know how many verts the final batch will have
        s32 nIndices = Strip.GetMaxNumIndices( kSkinVertBufferSize );
        s32* pI = (s32*)x_malloc( nIndices * sizeof(s32) );
        ASSERT( pI );
        nIndices = Strip.GetIndicesPS2( pI, kSkinVertBufferSize );

        // fill in the final batch verts
        FinalBatch.Verts.SetCapacity( nIndices );
        for ( s32 iFinalVert = 0; iFinalVert < nIndices; iFinalVert++ )
        {
            s32 Index = Strip.GetIndex( pI[iFinalVert] );
            optimizer_vert& FinalVert = StripperVertPool[Index];
            FinalVert.ADC = Strip.IsIndexNewStrip( pI[iFinalVert] );
            FinalVert.CCW = Strip.IsIndexCCWTri( pI[iFinalVert] );
            FinalBatch.Verts.Append( FinalVert );
        }
        ASSERT( FinalBatch.Verts.GetCount() == nIndices );

        // clean up
        x_free( pI );
    }
}

//=============================================================================

s32 ps2skin_optimizer::CountSharedVerts( tri_batch& Batch, optimizer_tri& Tri )
{

    s32 iTri;
    s32 nShared = 0;
    for ( iTri = 0; iTri < Batch.Tris.GetCount(); iTri++ )
    {
        optimizer_tri& BatchTri = Batch.Tris[iTri];
        if ( Tri.Verts[0].iOrigIndex == BatchTri.Verts[0].iOrigIndex )  nShared++;
        if ( Tri.Verts[0].iOrigIndex == BatchTri.Verts[1].iOrigIndex )  nShared++;
        if ( Tri.Verts[0].iOrigIndex == BatchTri.Verts[2].iOrigIndex )  nShared++;
        if ( Tri.Verts[1].iOrigIndex == BatchTri.Verts[0].iOrigIndex )  nShared++;
        if ( Tri.Verts[1].iOrigIndex == BatchTri.Verts[1].iOrigIndex )  nShared++;
        if ( Tri.Verts[1].iOrigIndex == BatchTri.Verts[2].iOrigIndex )  nShared++;
        if ( Tri.Verts[2].iOrigIndex == BatchTri.Verts[0].iOrigIndex )  nShared++;
        if ( Tri.Verts[2].iOrigIndex == BatchTri.Verts[1].iOrigIndex )  nShared++;
        if ( Tri.Verts[2].iOrigIndex == BatchTri.Verts[2].iOrigIndex )  nShared++;
    }

    return nShared;
}
