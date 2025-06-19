
#include "CollisionVolume.hpp"

//=========================================================================
// FUNCTION
//=========================================================================

void collision_data::mat_info::FileIO( fileio& FileIO )
{
    FileIO.Static( SoundType );
    FileIO.Static( Flags );
}

void collision_data::high_cluster::FileIO( fileio& FileIO )
{
    FileIO.Static( BBox );       
    FileIO.Static( nTris );     
    FileIO.Static( iMesh );     
    FileIO.Static( iBone );     
    FileIO.Static( iDList );     
    FileIO.Static( iOffset );
    FileIO.Static( MaterialInfo );     
}

//=========================================================================

void collision_data::low_cluster::FileIO( fileio& FileIO )
{
    FileIO.Static( BBox );
    FileIO.Static( iVectorOffset );
    FileIO.Static( nPoints );
    FileIO.Static( nNormals );
    FileIO.Static( iQuadOffset );
    FileIO.Static( nQuads );
    FileIO.Static( iMesh );
    FileIO.Static( iBone );
}

//=========================================================================

void collision_data::low_quad::FileIO( fileio& FileIO )
{
    FileIO.Static( iP[0] );
    FileIO.Static( iP[1] );
    FileIO.Static( iP[2] );
    FileIO.Static( iP[3] );
    FileIO.Static( iN    );
    FileIO.Static( Flags );
}

//=========================================================================

collision_data::collision_data( void )
{
    BBox.Clear();

    nHighClusters       = 0;
    pHighCluster        = NULL;
    nHighIndices        = 0;
    pHighIndexToVert0   = NULL;

    nLowClusters = 0;   pLowCluster = 0;
    nLowVectors = 0;    pLowVector = 0;
    nLowQuads = 0;      pLowQuad = 0;
}

//=========================================================================

collision_data::collision_data( fileio& FileIO )
{
    (void)FileIO;
}

//=========================================================================

void collision_data::FileIO( fileio& FileIO )
{
    FileIO.Static( BBox );

    FileIO.Static( nHighClusters );
    FileIO.Static( pHighCluster, nHighClusters );
    FileIO.Static( nHighIndices );
    FileIO.Static( pHighIndexToVert0, nHighIndices );

    FileIO.Static( nLowClusters );
    FileIO.Static( nLowVectors );
    FileIO.Static( nLowQuads );
    FileIO.Static( pLowCluster, nLowClusters );
    FileIO.Static( pLowVector,  nLowVectors );
    FileIO.Static( pLowQuad,    nLowQuads );
}

//=========================================================================
