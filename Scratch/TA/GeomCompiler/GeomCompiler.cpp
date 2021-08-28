
#include "GeomCompiler.hpp"
#include "geom.hpp"
#include "rawmesh.hpp"

// 
// TODO:
//=========================================================================
// Tomas :
// Right now the materials are not been factor-out so everything is replicated.
// Because of that it is possible that the mesh is been fracture more than it has to.
//=========================================================================
// Tomas :
// All this code should really be using a soft-skin single compiler. Rather than to solve
// this cases then solving the animated cases and the solving the semy-soft skin, and 
// finally the solf=skin.
//=========================================================================


//=========================================================================

void geom_compiler::AddPlatform( platform Platform, const char* pDirName )
{
    plat_info& P = m_PlatInfo.Append();
    x_strcpy( P.DirName, pDirName );
    P.Platform = Platform;
}

//=========================================================================

void geom_compiler::AddLOD( const char* pFileName, f32 MinDistance, xbool BuildCollision )
{
    info& Info = m_InfoList.Append();
    x_strcpy( Info.FileName, pFileName );
    Info.BuildCollision = BuildCollision;
    Info.MinDistance    = MinDistance;
}

//=========================================================================

void geom_compiler::Export( const char* pFileName )
{
    array( rawmesh, Rawmesh, 8 );
    geom    Geom;
    s32     i;

    x_memset( &Geom, 0, sizeof(Geom) );

    //
    // Begin protected zone
    //
    e_begin;

    if( m_InfoList.GetCount() > 8 )
        e_throw( "Too many meshes" );

    //
    // Load all the raw-meshes
    //

    // This is a hack for now
    Geom.m_nDLists = m_InfoList.GetCount(); 

    for( i=0; i<Geom.m_nDLists; i++ ) 
    {
        VERIFY( Rawmesh[i].Load( m_InfoList[i].FileName ) == TRUE );

        Geom.m_nVerts       += Rawmesh[i].m_nVertices; 
        Geom.m_nIndices     += Rawmesh[i].m_nFacets*3;
    }

    //
    // Allocate all the necesary memory
    //
    Geom.m_pVertex   = new geom::vertex  [ Geom.m_nVerts     ];
    Geom.m_pIndex    = new u16           [ Geom.m_nIndices   ];
    Geom.m_pDList    = new geom::dlist_pc[ Geom.m_nDLists  ];

    if( Geom.m_pVertex   == NULL || 
        Geom.m_pIndex    == NULL ||
        Geom.m_pDList    == NULL )
    {
        e_throw( "Out of memory" );
    }

    //
    // Start copying the hold thing into the structure
    //

    s32         iVert     = 0;
    s32         iIndex    = 0;
    for( i=0; i<Geom.m_nDLists; i++ ) 
    {
        s32         j;
        rawmesh&    Raw = Rawmesh[i];

        Geom.m_pDList[i].iIndices = iIndex;
        Geom.m_pDList[i].iVert    = iVert;

        for( j=0; j<Raw.m_nVertices; j++ )
        {           
            Geom.m_pVertex[ iVert ].Pos      =  Raw.m_pVertex[j].Position;
            Geom.m_pVertex[ iVert ].Normal   =  Raw.m_pVertex[j].Normal [0];
            Geom.m_pVertex[ iVert ].UV       =  Raw.m_pVertex[j].UV     [0];
            Geom.m_pVertex[ iVert ].Color    =  Raw.m_pVertex[j].Color  [0];

            iVert++;
            ASSERT( iVert <= Geom.m_nVerts );
        }

        for( j=0; j<Raw.m_nFacets; j++ )
        {           
            Geom.m_pIndex[ iIndex++ ] = (u16)Raw.m_pFacet[j].iVertex[0];
            Geom.m_pIndex[ iIndex++ ] = (u16)Raw.m_pFacet[j].iVertex[1];
            Geom.m_pIndex[ iIndex++ ] = (u16)Raw.m_pFacet[j].iVertex[2];
        }
    }

    //
    // Save the grom
    //
    {
        fileio File;
        File.Save( pFileName, Geom, FALSE );
    }

    //
    // Handle any errors
    //
    e_block;




    e_continue( NULL );

    e_block_end;
}