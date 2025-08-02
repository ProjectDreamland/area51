
#ifndef COLLISION_VOLUME_HPP
#define COLLISION_VOLUME_HPP

#include "x_math.hpp"
#include "Auxiliary\MiscUtils\Fileio.hpp"

//=========================================================================
// class
//=========================================================================

struct collision_data
{
    void FileIO   ( fileio& FileIO );
    collision_data( fileio& FileIO );
    collision_data( void );

    struct mat_info
    {
        enum 
        {
            FLAG_DOUBLESIDED        = ( 1 <<  0 ),  // Material is double sided
            FLAG_TRANSPARENT        = ( 1 <<  1 ),  // Material is transparent
        };

        u16         SoundType;
        u16         Flags;

        void FileIO( fileio& FileIO );
    };

    struct high_cluster
    {
        bbox        BBox;
        s16         nTris;
        s16         iMesh;
        s16         iBone;
        s16         iDList;
        s32         iOffset;
        mat_info    MaterialInfo;

        void FileIO( fileio& FileIO );
    };

    struct low_quad
    {
        byte iP[4];
        byte iN;
        byte Flags;
        void FileIO( fileio& FileIO );
    };

    struct low_cluster
    {
        bbox    BBox;
        s16     iVectorOffset;
        s16     nPoints;
        s16     nNormals;
        s16     iQuadOffset;
        s16     nQuads;
        s16     iMesh;
        s16     iBone;
        void FileIO( fileio& FileIO );
    };

//=========================================================================

    bbox            BBox;           // Only valid for "zero pose".
    s32             nHighClusters;
    high_cluster*   pHighCluster;
    s32             nHighIndices;
    u16*            pHighIndexToVert0;
    s16             nLowClusters;
    s16             nLowVectors;
    s16             nLowQuads;
    low_cluster*    pLowCluster;
    vector3*        pLowVector;
    low_quad*       pLowQuad;
};


//=========================================================================
#endif
//=========================================================================