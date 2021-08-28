#ifndef ARM_OPTIMIZER_HPP
#define ARM_OPTIMIZER_HPP

///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

#include "x_array.hpp"
#include "RawMesh2.hpp"

///////////////////////////////////////////////////////////////////////////
// TYPES
///////////////////////////////////////////////////////////////////////////

struct arm_optimizer
{
///////////////////////////////////////////////////////////////////////////
                   
    enum commnad_type
    {
        DRAW_LIST,       // Arg1 = Start,  Arg2 = End
        UPLOAD_MATRIX,   // Arg1 = BoneID, Arg2 = CacheID
        NOP_MATRIX       // Arg1 = BoneID, Arg2 = CacheID
    };

    struct command
    {
        inline command( void ){}
        inline command( commnad_type T, s32 a1, s32 a2 ) { Type = T; Arg1 = a1; Arg2 = a2; }

        commnad_type Type;
        s32          Arg1;
        s32          Arg2;
    };

    typedef rawmesh2::vertex            vertex;
    typedef rawmesh2::weight            weight;
    typedef rawmesh2::facet             triangle;
    typedef xarray<vertex>              list_vertex;
    typedef xarray<triangle>            list_triangle;
    typedef xarray<command>             list_command;

    struct section
    {
        s32                 iMaterial;
        list_vertex         lVertex;
        list_triangle       lTriangle;
        list_command        lCommand;
    };

///////////////////////////////////////////////////////////////////////////

    void Build( const rawmesh2& RawMesh, const xarray<s32>& lTri, s32 nMatrices = 1, s32 CacheSize = 17 );
    void OpSection( section& Section );
    void OpCommands( section& Section );
    void ResetMatrixCache( void );
    void ResetVertexCache( void );

///////////////////////////////////////////////////////////////////////////

    section                 m_Section;
    s32                     m_nMatrices;
    s32                     m_CacheSize;
    s32                     m_MaxVertsSection;
    s32*                    m_pMatrixCache;
    s32*                    m_pMatrixCacheScore;
    s32*                    m_pVertexCache;
    s32*                    m_pVertexCacheScore;
};

///////////////////////////////////////////////////////////////////////////
// END
///////////////////////////////////////////////////////////////////////////
#endif 