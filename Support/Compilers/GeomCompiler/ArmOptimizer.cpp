
#include "ArmOptimizer.hpp"

extern xbool g_Verbose;

///////////////////////////////////////////////////////////////////////////
// TYPES
///////////////////////////////////////////////////////////////////////////

struct weight_list
{
    s32 nWeights;
    s32 Weight[3*8];
};

///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////

//=========================================================================

void arm_optimizer::OpCommands( section& Section )
{    
    X_FILE*         Fp;
    list_command    lOptimized; 
    s32             From     = 0;
    s32             To       = 0;
    s32             Flash    = 0;
    s32*            FlushSys = m_pMatrixCacheScore;
    list_command&   lCommand = Section.lCommand;
    list_vertex     lNewVertex;

    //
    // Reset the matrix cache
    //
    // The matrix score values will mean the fallowing
    //  1 is lock
    //  2 old matrix
    // -1 is corently not been use
    //
    for( s32 i=0; i<m_nMatrices; i++ )
    {
        m_pMatrixCache[ i ]     = -1;
        FlushSys[ i ]           = -1;
    }

    for( i=0; i<lCommand.GetCount(); i++ )
    {
        command& Command = lCommand[i];

        if( Command.Type == UPLOAD_MATRIX )
        {
            m_pMatrixCache[ Command.Arg2 ]  = Command.Arg1;
            FlushSys[ Command.Arg2 ]        = 1;
        }

        if( Command.Type == NOP_MATRIX && (Flash == 1) )
        {
            FlushSys[ Command.Arg2 ] = -1;
        }

        if( Command.Type == DRAW_LIST )
        {
            To = Command.Arg2;
            s32 j;
            s32 nFreeNodes=0;

            //
            // Check whether we have a full cache
            //
            for( j=0; j<m_nMatrices; j++ )
            {
                if( FlushSys[ j ] == -1 ) nFreeNodes++;
            }

            //
            // Check howmany new matrices we are going to load next
            //
            {
                for( j=(i+1); j<lCommand.GetCount(); j++ )
                {
                    command& Command = lCommand[j];
                    if( Command.Type != UPLOAD_MATRIX ) break;
                    nFreeNodes--;
                }
            }

            // Disactivate cleaning mode
            Flash = 0;

            if( nFreeNodes >= 0  && ((i+1) < lCommand.GetCount()) ) continue;

            // Activate cleaning mode
            Flash = 1;

            //
            // Flash whatever matrices are left over if this is the last draw
            //
            //
            for( j=0; j<m_nMatrices; j++ )
            {
                if( FlushSys[ j ] >= 0 )
                {
                    if( FlushSys[ j ] != -1 )
                    {
                        if( FlushSys[ j ] == 1 )
                            lOptimized.Append() =  command( UPLOAD_MATRIX, m_pMatrixCache[ j ], j );

                        FlushSys[ j ] = 2;
                    }
                }
            }

            //
            // Flush the polys
            //
            {
                s32 OldFrom = From;
                lOptimized.Append() = command( DRAW_LIST, From, To );
                From  = To;

                //
                // Change the index of the vertex weighting to point at the 
                // right matrix cache entry.
                //
                {
                    list_triangle& lTriangle = Section.lTriangle;
                    list_vertex&   lVertex   = Section.lVertex;

                    for( s32 j=OldFrom; j<To; j++ )
                    {
                        for( s32 v=0; v<3; v++ )
                        {
                            rawmesh2::vertex V = lVertex[ lTriangle[j].iVertex[v] ];

                            //
                            // Use the weights in the cache
                            // 
                            for( s32 w = 0; w<V.nWeights; w++ )
                            {
                                for( s32 m=0; m<m_nMatrices; m++ )
                                {
                                    if( FlushSys[ m ] >= 0 )
                                    {
                                        if( V.Weight[w].iBone == m_pMatrixCache[ m ] )
                                        {
                                            V.Weight[w].iBone = m;
                                            break;
                                        }
                                    }
                                }

                                ASSERT( m != m_nMatrices );
                            }

                            //
                            // Add new vertices if we need to
                            // 
                            for( w=0; w<lNewVertex.GetCount(); w++ )
                            {
                                rawmesh2::vertex& V2 = lNewVertex[w];

                                if( V.Position == V2.Position &&
                                    V.nWeights == V2.nWeights )
                                {
                                    for( s32 h=0; h<V.nWeights; h++ )
                                    {
                                        if( V.Weight[h].iBone != V2.Weight[h].iBone )
                                            break;

                                        if( V.Weight[h].Weight != V2.Weight[h].Weight )
                                            break;
                                    }

                                    if( h != V.nWeights ) continue;

                                    if( V.nUVs != V2.nUVs ) continue;

                                    for( h=0; h < V.nUVs; h++)
                                    {
                                        if( V.UV[h].X != V2.UV[h].X ||
                                            V.UV[h].Y != V2.UV[h].Y ) break;                    
                                    }
                                    if( h != V.nUVs ) continue;


                                    // New vertex index
                                    lTriangle[j].iVertex[v] = w;
                                    break;
                                }                                    
                            }

                            if( w == lNewVertex.GetCount() )
                            {
                                lTriangle[j].iVertex[v] = lNewVertex.GetCount();
                                lNewVertex.Append();
                                lNewVertex[ lTriangle[j].iVertex[v] ] = V;
                            }
                        }
                    }
                }
            }
        }
    }

    //
    // Set the resolt in our list
    //
    lCommand        = lOptimized;
    Section.lVertex = lNewVertex;

    //
    // Ouput the log
    //
    Fp = x_fopen( "C:\\ArmOpLog.txt", "at" );
    ASSERT( Fp );

    x_fprintf (Fp, "\n\n" );
    x_fprintf (Fp, "=============================================\n" );
    x_fprintf (Fp, "Optimized Commands\n" );
    x_fprintf (Fp, "=============================================\n" );

    for( i=0; i<lCommand.GetCount(); i++ )
    {
        command& Command = lCommand[i];

        switch( Command.Type )
        {
        case DRAW_LIST:     x_fprintf( Fp, "DRAW_LIST    :  From (%d) to (%d) \n", Command.Arg1, Command.Arg2 ); 
            break;
        case UPLOAD_MATRIX: x_fprintf( Fp, "UPLOAD_MATRIX:  BoneID (%d) CacheID (%d) \n", Command.Arg1, Command.Arg2 ); 
            break;
        case NOP_MATRIX:    x_fprintf( Fp, "NOP_MATRIX   :  BoneID (%d) CacheID (%d) \n", Command.Arg1, Command.Arg2 ); 
            break;
        }
    }

    x_fprintf (Fp, "\n\n" );
    x_fclose( Fp );
}

//=========================================================================

void arm_optimizer::ResetMatrixCache( void )
{
    for( s32 i=0; i<m_nMatrices; i++ )
    {
        m_pMatrixCache[i]       = -1;
        m_pMatrixCacheScore[i]  = -1;
    }
}

//=========================================================================

void arm_optimizer::ResetVertexCache( void )
{
    for( s32 i=0; i<m_CacheSize; i++ )
    {
        m_pVertexCache[i]       = -1;
        m_pVertexCacheScore[i]  = -1;
    }
}


//=========================================================================

void arm_optimizer::OpSection( section& Section ) 
{
    xarray<weight_list>     WList;
    list_triangle           OptimizeFacet;//     ( Section.lTriangle.GetCount() );
    s32                     NTrisTotal  = Section.lTriangle.GetCount();
    s32                     CurentCount = 0;
    s32                     BrauniPoints[3]={-1,-1,-1};
    X_FILE*                 Fp;
    s32                     nVertsStalls  = 0;
    s32                     nMatrixStalls = 0;
    s32                     LastPolyFlush = 0;

    //
    // Something to help reduce the matrix stalls
    //
    s64* pMatrixAge = new s64[ m_nMatrices ];
    ASSERT( pMatrixAge );
    x_memset( pMatrixAge, 0, sizeof(s64)*m_nMatrices );

    //
    // Create the log file of the optimicer
    //
    Fp = x_fopen( "c:\\ArmOpLog.txt", "at" );
    ASSERT( Fp );

    //
    // Create the weight list for each facet
    //
    {
        s32 nFacets = Section.lTriangle.GetCount();
        for( s32 i=0; i<nFacets;i++)
        {
            triangle&       Facet = Section.lTriangle[i];
            WList.Append();
            weight_list&    WL    = WList[ WList.GetCount()-1 ];

            WL.nWeights = 0;
            for( s32 j=0; j<3;j++)
            {
                vertex& V = Section.lVertex[ Facet.iVertex[j] ];

                for( s32 w=0; w<V.nWeights; w++)
                {
                    for( s32 c=0; c<WL.nWeights; c++ )
                    {
                        if( V.Weight[w].iBone == WL.Weight[c] ) break;
                    }

                    if( c == WL.nWeights )
                    {
                        WL.Weight[ WL.nWeights ] = V.Weight[w].iBone;
                        WL.nWeights++;
                    }
                }
            }

            if( WL.nWeights > m_nMatrices )
            {
                x_throw( "The matrix cache is too small.\n" );
            }
        }
    }

    //
    // Use the first facet in the list for the seek of the algorithum
    // Then Initialize all the caches and move the facet to the optimice grop
    //
    {
        s32 MatrixCount;
        s32 i;

        // Set all the matrices into the cache
        MatrixCount = 0;
        for( i=0; i<WList[0].nWeights; i++ )
        {
            m_pMatrixCache[ i ] = WList[0].Weight[i];
            Section.lCommand.Append() = command( UPLOAD_MATRIX, m_pMatrixCache[ i ], i );
            MatrixCount++;
        }

        // invalidate the rest of the matrices
        for( i=MatrixCount; i<m_nMatrices; i++ )
        {
            m_pMatrixCache[i]      = -1;
            Section.lCommand.Append() = command( NOP_MATRIX, -1, i );                            
        }

        // Copy all the verts into the cache
        for( i=0; i<3; i++ )
        {
            m_pVertexCache[i] =  Section.lTriangle[0].iVertex[i];
        }
        
        // invalidate the rest
        for( ; i<m_CacheSize; i++ )
        {
            m_pVertexCache[i] = -1 ;
        }

        // Move facet zero from the list
        OptimizeFacet.Append() = Section.lTriangle[0];
        Section.lTriangle.Delete( 0 );
        WList.Delete( 0 );
    }

    //
    // Now we must find from our source list of facet which facet is best suted
    // base on what we already have in the cache
    //
    while( Section.lTriangle.GetCount() )
    {
        s32 nTrisLeft       = Section.lTriangle.GetCount();
        s32 iBestFacet      = 0;
        f32 BestScore       = -100000000.0f;
        s32 nWeightNotFound = 0;
        s32 nVertsFound     = 0;

        //
        // Reset the score of the verts and matrices
        //
        CurentCount++;
        for( s32 f=0; f< m_nMatrices; f++ )
        {
            m_pMatrixCacheScore[f] = -(m_pMatrixCache[f] == -1);
        }

        for( f=0; f< m_CacheSize; f++ )
        {
            m_pVertexCacheScore[f] = -(m_pVertexCache[f] == -1);
        }

        if( g_Verbose && ((CurentCount%100) == 0) )
        {
            x_DebugMsg( "%d out of %d there are left %d\n", CurentCount, NTrisTotal, nTrisLeft );
        }

        //
        // Find the next best facet
        //
        for( f=0; f< nTrisLeft; f++ )
        {
            s32             i,c=0,k;
            triangle&       Facet = Section.lTriangle[f];
            weight_list&    WL    = WList[f];
            f32             Score = 0;

            // get the score base on the matrix cache
            for( c=i=0; i<WL.nWeights; i++ )
            {
                for( s32 j=0; j<m_nMatrices; j++ )
                {
                    if( m_pMatrixCache[j] == WL.Weight[i] ) 
                    {
                        m_pMatrixCacheScore[j]++;
                        Score += 100;
                        break;
                    }
                }

                if( j == m_nMatrices )
                {
                    c++; 
                }
            }

            // For every matrix it miss we are going to punish it 
            Score -= c * 50;

            // IF we have found all the bones for this facet we like it
            // allot.
            if( c == 0 ) Score = 10000; 

            // Don't bather checking anything else if we alredy lost
            if( BestScore > (Score+3) ) continue;

            // Get score base on the vertex cache
            for( k=i=0; i<3; i++)
            {
                const s32 v = Facet.iVertex[i];

                for( s32 j=0; j<m_CacheSize; j++ )
                {
                    if( m_pVertexCache[j] == v ) 
                    {
                        m_pVertexCacheScore[j]++;
                        Score += 1;
                        k++;
                        break;
                    }
                }
            }

            // Give brauni points from keeping facets to jump all over the place
            if( k == 3 )
            {
                for( k=0; k<3; k++)
                for( s32 l=0; l<3; l++)
                {
                    if( BrauniPoints[k] == Facet.iVertex[l] )
                    {
                        Score += 1;
                        break;
                    }
                }
            }

            // Now chech if we got a winer so far
            if( Score > BestScore )
            {
                iBestFacet      = f;
                BestScore       = Score;
                nWeightNotFound = c;
                nVertsFound     = k;

                const s32 MaxScore = 10000 + 3*5*100 + 3;

                if( Score == MaxScore ) break;
            }
        }

        //
        // Check whether we have to invalidate any of the caches
        //
        {
            triangle&       Facet = Section.lTriangle[ iBestFacet ];
            weight_list&    WL    = WList[ iBestFacet ];

            //
            // Check whether we have to invalidate any of the matrices
            //
            if( nWeightNotFound )
            {
                //
                // Flush polys
                //
                if( OptimizeFacet.GetCount() != LastPolyFlush )
                {
                    Section.lCommand.Append() = command( DRAW_LIST, LastPolyFlush, OptimizeFacet.GetCount() );                            
                    LastPolyFlush = OptimizeFacet.GetCount();
                }

                //
                // The hardware probably invalidates the verts we should do the same
                //
                if( 0 )
                {
                    ResetVertexCache();
                    nVertsFound=0;
                }

                //
                // Find the best score and upload the matrices
                //
                for( s32 v=0; v<WL.nWeights; v++)
                {
                    s32 iMatrix = 0;
                    BestScore   = 999999999.0f;

                    // Find the best score               
                    for( s32 i=0; i<m_nMatrices; i++ )
                    {
                        if( WL.Weight[v] == m_pMatrixCache[i] ) break;

                        // We want to take the place of a matrix that most
                        // polygons agree that it sucks
                        if( m_pMatrixCacheScore[i] == BestScore )
                        {
                            if( pMatrixAge[i] > pMatrixAge[ iMatrix ] )
                            {
                                BestScore = (f32)m_pMatrixCacheScore[i];
                                iMatrix   = i;
                            }
                        }
                        else
                        if( m_pMatrixCacheScore[i] < BestScore )
                        {
                            BestScore = (f32)m_pMatrixCacheScore[i];
                            iMatrix   = i;
                        }
                    }

                    // We found that matrix in the cache
                    if( i != m_nMatrices ) continue;

                    // Upload one of the matrixces that we need
                    ASSERT( m_pMatrixCacheScore[ iMatrix ] < (10000000-1) ); 
                    
                    if( m_pMatrixCacheScore[ iMatrix ] != -1 ) nMatrixStalls++;

                    // Add the command to add the matrices
                    Section.lCommand.Append() = command( UPLOAD_MATRIX, WL.Weight[v], iMatrix );

                    // Add and update the score
                    m_pMatrixCache[ iMatrix ]         = WL.Weight[v];
                    m_pMatrixCacheScore[ iMatrix ]   += 10000000;
                    pMatrixAge[ iMatrix ]             = 0;
                }

                //
                // Set the Null commands for the matrices
                //
                {
                    for( s32 i=0; i<m_nMatrices; i++ )
                    {
                        if( m_pMatrixCacheScore[i] <= 0 )
                        {
                            Section.lCommand.Append() = command( NOP_MATRIX, m_pMatrixCache[ i ], i );                            

                            // make older matrix suck more
                            pMatrixAge[ i ]++;
                        }
                    }
                }
            }

            //
            // Check whether we have to invalidate any of the verts
            //
            if( nVertsFound < 3 )
            {
                for( s32 v=0; v<3; v++ )
                {
                    s32 iVertex = 0;
                    BestScore   = 999999999.0f;

                    for( s32 i=0; i<m_CacheSize; i++ )
                    {
                        // Make sure that is none of the matrices what we
                        // need for the new facet
                        if( Facet.iVertex[v] == m_pVertexCache[i] ) 
                        {
                            break;
                        }

                        // We want to take the place of a matrix that most
                        // polygons agree that it sucks
                        if( m_pVertexCacheScore[i] < BestScore )
                        {
                            BestScore = (f32)m_pVertexCacheScore[i];
                            iVertex   = i;
                        }
                    }

                    // We have found the vertex in the cache
                    if( i < m_CacheSize ) continue;
                    m_pVertexCache[ iVertex ]       = Facet.iVertex[v];

                    ASSERT( m_pVertexCacheScore[ iVertex ] < (10000000-1) );
                    if( m_pVertexCacheScore[ iVertex ] != -1 ) nVertsStalls++;
                    m_pVertexCacheScore[ iVertex ] += 10000000;
                }
            }
        }

        //
        // Do some sanity checks
        //
        {
            weight_list& WL = WList[ iBestFacet ];
            triangle& Facet = Section.lTriangle[ iBestFacet ];
            for( s32 v=0; v<WL.nWeights; v++ )
            {
                for( s32 i=0;i<m_nMatrices; i++ )
                {
                    if( m_pMatrixCache[i] == WL.Weight[v] ) break; 
                }

                ASSERT( i < m_nMatrices );
            }


        }

        //
        // Do the log file
        //
        {
            //
            // Print the vertex and matrix caches
            //
            x_fprintf( Fp,"MC[ ");
            for( s32 i=0;i<m_nMatrices; i++ )
            {
                x_fprintf( Fp,"%4d ", m_pMatrixCache[i]);
            }
            x_fprintf( Fp,"]\n");

            x_fprintf( Fp,"MS[ ");
            for( i=0;i<m_nMatrices; i++ )
            {
                if( m_pMatrixCacheScore[i] >= (10000000-1) )
                {
                    x_fprintf( Fp,"#%2d# ", m_pMatrixCacheScore[i] - 10000000 );
                }
                else
                {
                    x_fprintf( Fp,"%4d ", m_pMatrixCacheScore[i]);
                }
            }
            x_fprintf( Fp,"]\n");

            x_fprintf( Fp,"VC[ ");
            for( i=0;i<m_CacheSize; i++ )
            {
                x_fprintf( Fp,"%4d ", m_pVertexCache[i]);
            }
            x_fprintf( Fp,"]\n");

            x_fprintf( Fp,"VS[ ");
            for( i=0;i<m_CacheSize; i++ )
            {
                if( m_pVertexCacheScore[i] >= (10000000-1) )
                {
                    x_fprintf( Fp,"X%2dX ", m_pVertexCacheScore[i] - 10000000 );
                }
                else
                {
                    x_fprintf( Fp,"%4d ", m_pVertexCacheScore[i]);
                }
            }
            x_fprintf( Fp,"]\n");

            //
            // Print the vertex
            //
            {
                weight_list& WL = WList[ iBestFacet ];

                triangle& Facet = Section.lTriangle[ iBestFacet ];

                x_fprintf( Fp, "V[%4d %4d %4d] ",
                    Section.lTriangle[iBestFacet].iVertex[0],
                    Section.lTriangle[iBestFacet].iVertex[1],
                    Section.lTriangle[iBestFacet].iVertex[2] );

                x_fprintf( Fp, "M[");
                for( s32 v=0; v<WL.nWeights; v++ )
                {
                    x_fprintf( Fp, " %4d",WL.Weight[v] ); 
                }
                x_fprintf( Fp, " ]\n\n\n");
            }
        }

        //
        // Set the brauni point hehehe...
        //
        BrauniPoints[0] = Section.lTriangle[iBestFacet].iVertex[0];
        BrauniPoints[1] = Section.lTriangle[iBestFacet].iVertex[1];
        BrauniPoints[2] = Section.lTriangle[iBestFacet].iVertex[2];

        //
        // we can remove the facet from the source list to the dest list
        //
        OptimizeFacet.Append() = Section.lTriangle[iBestFacet];
        Section.lTriangle.Delete( iBestFacet );
        WList.Delete( iBestFacet );
    }

    //
    // Flush what ever remaining polygos
    //
    if( OptimizeFacet.GetCount() != LastPolyFlush )
    {
        Section.lCommand.Append() = command( DRAW_LIST, LastPolyFlush, OptimizeFacet.GetCount() );                            
        LastPolyFlush = OptimizeFacet.GetCount();
    }

    //
    // Copy to the section all the facets
    //
    Section.lTriangle = OptimizeFacet;

    //
    // Delete the matrix age
    //
    delete []pMatrixAge;

    //
    // Printout the commands
    //
    {
        s32 nComands = Section.lCommand.GetCount();

        x_fprintf (Fp, "\n\n" );
        x_fprintf (Fp, "=============================================\n" );
        x_fprintf (Fp, "Preoptimized Commands\n" );
        x_fprintf (Fp, "=============================================\n" );

        for( s32 i=0; i<nComands; i++ )
        {
            command& Command = Section.lCommand[i];

            switch( Command.Type )
            {
            case DRAW_LIST:     x_fprintf( Fp, "DRAW_LIST    :  From (%d) to (%d) \n", Command.Arg1, Command.Arg2 ); 
                break;
            case UPLOAD_MATRIX: x_fprintf( Fp, "UPLOAD_MATRIX:  BoneID (%d) CacheID (%d) \n", Command.Arg1, Command.Arg2 ); 
                break;
            case NOP_MATRIX:    x_fprintf( Fp, "NOP_MATRIX   :  BoneID (%d) CacheID (%d) \n", Command.Arg1, Command.Arg2 ); 
                break;
            }
        }
        x_fprintf (Fp, "\n\n" );
    }

    //
    // Close the log file
    //
    x_fprintf( Fp, "\n\n------------------------------------------\n");
    x_fprintf( Fp, "NVerts  Stalls per triangle: %f\n", nVertsStalls/(f32)OptimizeFacet.GetCount() );
    x_fprintf( Fp, "NMatrix Stalls: %d\n", nMatrixStalls );
    x_fprintf( Fp, "------------------------------------------\n");
    x_fclose( Fp );
}

//=========================================================================

void arm_optimizer::Build( const rawmesh2& RawMesh, const xarray<s32>& lDListTri, s32 nMatrices, s32 CacheSize )
{
    //
    // Set the optimize parameters
    //
    m_nMatrices         = nMatrices;
    m_CacheSize         = CacheSize;
    m_MaxVertsSection   = 100000;

    for( s32 i=0; i<lDListTri.GetCount(); i++ )
    {
        m_Section.lTriangle.Append() = RawMesh.m_pFacet[ lDListTri[i] ];
    }

    //
    // Copy each vertex and fix up the index
    //
    
    if( g_Verbose )
        x_DebugMsg("Coping Vertex into the right section...\n");
    list_vertex&    lVert   = m_Section.lVertex;
    list_triangle&  lTri    = m_Section.lTriangle;
    s32             nTris   = lTri.GetCount();
    
    for( s32 j=0; j<nTris; j++ )
    {
        triangle& Triangle = lTri[j];

        if( g_Verbose && ((j%1000)==0) ) x_DebugMsg("Facet %d of %d\n", j,nTris );  
        for( s32 k=0; k<3; k++ )
        {
            vertex&   Vertex = RawMesh.m_pVertex[ Triangle.iVertex[k] ];
            s32       nVerts = lVert.GetCount();

            // Linear search for the particular vertex
            for( s32 l=0; l<nVerts; l++ )
            {
                s32     s;
                vertex& V = lVert[l];

                if( Vertex.nWeights != V.nWeights ) continue;
                if( Vertex.nUVs     != V.nUVs     ) continue;

                // Check normals
                if( V.Normal[0] != Vertex.Normal[0] )
                    continue;

                // Check Position
                if( V.Position != Vertex.Position )
                    continue;

                // Check UVs
                for( s=0; s < V.nUVs; s++)
                {
                    if( V.UV[s].X != Vertex.UV[s].X ||
                        V.UV[s].Y != Vertex.UV[s].Y ) break;                    
                }
                if( s != V.nUVs ) continue;

                // Check Weights
                for( s=0; s < V.nWeights; s++)
                {
                    if( V.Weight[s].iBone  != Vertex.Weight[s].iBone ||
                        V.Weight[s].Weight != Vertex.Weight[s].Weight ) break;                    
                }
                if( s != V.nWeights ) continue;

                // All is the same so we already have this vertex
                break;
            }

            // if we haven't found it the add it
            if( l == nVerts ) 
            {
                l = lVert.GetCount();
                lVert.Append() = Vertex;
            }

            // Set the index for this facet
            Triangle.iVertex[k] = l;
        }
    }

    //
    // Allocate all the cache system 
    //
    m_pMatrixCache        = new s32[ m_nMatrices ];
    ASSERT( m_pMatrixCache );

    m_pMatrixCacheScore   = new s32[ m_nMatrices ];
    ASSERT( m_pMatrixCacheScore );

    m_pVertexCache        = new s32[ m_CacheSize ];
    ASSERT( m_pVertexCache );

    m_pVertexCacheScore   = new s32[ m_CacheSize ];
    ASSERT( m_pVertexCacheScore );

    //
    // Create the file
    //
    x_fclose( x_fopen( "c:\\ArmOpLog.txt", "wt" ) );

    s32 FinalVertexCount=0;
    s32 FinalFacetCount=0;
    OpSection( m_Section );
    OpCommands( m_Section );
    FinalVertexCount += m_Section.lVertex.GetCount();
    FinalFacetCount  += m_Section.lTriangle.GetCount();

    //
    // Print some more stats
    //
    {
        X_FILE* Fp = x_fopen( "c:\\ArmOpLog.txt", "at" );

        x_fprintf( Fp, "Original Vertex Count: %d\n", RawMesh.m_nVertices );
        x_fprintf( Fp, "Final Vertex Count   : %d\n", FinalVertexCount );

        x_fprintf( Fp, "Original Facet Count: %d\n", RawMesh.m_nFacets );
        x_fprintf( Fp, "Final Facet Count   : %d\n", FinalFacetCount );

        x_fclose( Fp );
    }


    //
    // Clean up
    //
    delete[] m_pMatrixCache      ;
    delete[] m_pMatrixCacheScore ;
    delete[] m_pVertexCache      ;
    delete[] m_pVertexCacheScore ;
}
