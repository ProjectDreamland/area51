
#include "SkinGeom.hpp"

#ifdef TARGET_XBOX
#include "Entropy/XBox/xbox_private.hpp"

void xbox_PreRegister( const char*,skin_geom* pGeom );
#endif

//=========================================================================

skin_geom::skin_geom( void ) : geom()
{
}

//=========================================================================

skin_geom::skin_geom( fileio& File ) : geom( File )
{
    (void)File;

    // Must resolve here because dynamic data is deleted upon exiting
    // this function. Note that we will register it here, but unregister
    // from the fileio because rigid_geom's don't have destructors. (PS2
    // and PC optimize virtual destructors differently, making the struct
    // size change.)
    #ifdef TARGET_XBOX
    xbox_PreRegister( "Skin geom",this );
    #endif
}

//=========================================================================

void skin_geom::uv_ps2::FileIO( fileio& File )
{
    File.Static( U );
    File.Static( V );
}

//=========================================================================

void skin_geom::pos_ps2::FileIO( fileio& File )
{
    File.Static( Pos, 3 );
    File.Static( ADC );
}

//=========================================================================

void skin_geom::boneindex_ps2::FileIO( fileio& File )
{
    File.Static( B0 );
    File.Static( B1 );
    File.Static( W0 );
    File.Static( W1 );
}

//=========================================================================

void skin_geom::normal_ps2::FileIO( fileio& File )
{
    File.Static( Normal[0] );
    File.Static( Normal[1] );
    File.Static( Normal[2] );
    File.Static( Pad );
}

//=========================================================================

void skin_geom::command_ps2::FileIO( fileio& File )
{
    File.StaticEnum ( Cmd );
    File.Static     ( Arg1 );
    File.Static     ( Arg2 );
}

//=========================================================================

void skin_geom::dlist_ps2::FileIO( fileio& File )
{
    File.Static( nCmds );
    File.Static( pCmd, nCmds );

    File.Static( nUVs );
    File.Static( pUV, nUVs );
    
    File.Static( nPos );
    File.Static( pPos, nPos );

    File.Static( nBoneIndices );
    File.Static( pBoneIndex, nBoneIndices );

    File.Static( nNormals );
    File.Static( pNormal, nNormals );
}

//=========================================================================

void skin_geom::vertex_pc::FileIO( fileio& File )
{
    File.Static( Position );
    File.Static( Normal );
    File.Static( UVWeights );
}

//=========================================================================

void skin_geom::command_pc::FileIO( fileio& File )
{
    File.StaticEnum ( Cmd );
    File.Static     ( Arg1 );
    File.Static     ( Arg2 );
}

//=========================================================================

void skin_geom::dlist_pc::FileIO( fileio& File )
{
    File.Static( nIndices );
    File.Static( pIndex, nIndices );

    File.Static( nVertices );
    File.Static( pVertex, nVertices );

    File.Static( nCommands );
    File.Static( pCmd, nCommands );
}

//=========================================================================

void skin_geom::command_xbox::FileIO( fileio& File )
{
    File.StaticEnum ( Cmd );
    File.Static     ( Arg1 );
    File.Static     ( Arg2 );
}

//=========================================================================

void skin_geom::vertex_xbox::FileIO( fileio& File )
{
    File.Static( Pos          );
    File.Static( PackedNormal );
    File.Static( UV           );
    File.Static( Weights      );
    File.Static( Bones        );
}

//=========================================================================

void skin_geom::dlist_xbox::FileIO( fileio& File )
{
    File.Static ( nIndices );
    File.Static ( pIndex, nIndices );
    File.Static ( nPushSize );
    File.Static ( pPushBuffer,nPushSize );
    File.Static ( nVerts );
    File.Dynamic( pVert, nVerts );
    File.Static ( nCommands );
    File.Static ( pCmd, nCommands );
}

//=========================================================================

void skin_geom::FileIO( fileio& File )
{
    geom::FileIO( File );

    File.Static( m_Version );
    File.Static( m_nDList  );
    File.Static( m_nBones  );

    switch( m_Platform )
    {
        case PLATFORM_XBOX :
            File.Static( m_System.pXbox, m_nDList );
            break;

        case PLATFORM_PS2 :
            File.Static( m_System.pPS2, m_nDList );
            break;

        case PLATFORM_PC :
            File.Static( m_System.pPC, m_nDList );
            break;

        default :
            ASSERT( 0 );
            break;
    }
}

//=========================================================================

