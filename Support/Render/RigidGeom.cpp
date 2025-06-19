
#include "RigidGeom.hpp"

#ifdef TARGET_XBOX
#include "Entropy/XBox/xbox_private.hpp"

void xbox_PreRegister( const char*,rigid_geom* pGeom );
#endif

//=========================================================================

rigid_geom::rigid_geom( void ) :
    geom()
{
}

//=========================================================================

rigid_geom::rigid_geom( fileio& File ) 
:   geom( File ), 
    m_Collision( File )
{
    (void)File;

    // Must resolve here because dynamic data is deleted upon exiting
    // this function. Note that we will register it here, but unregister
    // from the fileio because rigid_geom's don't have destructors. (PS2
    // and PC optimize virtual destructors differently, making the struct
    // size change.)
    #ifdef TARGET_XBOX
    xbox_PreRegister( "Rigid geom",this );
    #endif
}

//=========================================================================

void rigid_geom::FileIO( fileio& File )
{
    geom::FileIO( File );

    File.Static( m_Collision );
    File.Static( m_nDList );

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

void rigid_geom::dlist_xbox::FileIO( fileio& File )
{
    File.Static ( nIndices );
    File.Static ( pIndices, nIndices );
    File.Static ( nPushSize );
    File.Static ( pPushBuffer,nPushSize );
    File.Static ( nVerts );
    File.Dynamic( pVert,nVerts );
    File.Static ( iBone );
    File.Static ( iColor );
}

//=========================================================================

void rigid_geom::dlist_ps2::FileIO( fileio& File )
{
    File.Static( nVerts );
    File.Static( pUV,       nVerts*2 );
    File.Static( pNormal,   nVerts*3 );
    File.Static( pPosition, nVerts*1 );
    File.Static( iBone  );
    File.Static( iColor );
}

//=========================================================================

void rigid_geom::dlist_pc::FileIO( fileio& File )
{
    File.Static( nIndices );
    File.Static( pIndices, nIndices );
    File.Static( nVerts );
    File.Static( pVert, nVerts );
    File.Static( iBone );
    File.Static( Pad );
}

//=========================================================================

void rigid_geom::vertex_xbox::FileIO( fileio& File )
{
    File.Static( Pos          );
    File.Static( PackedNormal );
    File.Static( UV           );
}

//=========================================================================

void rigid_geom::vertex_pc::FileIO( fileio& File )
{
    File.Static( Pos     );
    File.Static( Normal  );
    File.Static( Color   );
    File.Static( UV      );
}

//=========================================================================

extern
xbool RigidGeom_GetTriangle( const rigid_geom*          pRigidGeom,
                             s32                   Key,
                             vector3&              P0,
                             vector3&              P1,
                             vector3&              P2);

xbool rigid_geom::GetGeoTri( s32 Key, vector3& V0, vector3& V1, vector3& V2 ) const
{
    if( Key == -1 )
        return( FALSE );

    return RigidGeom_GetTriangle( this, Key, V0, V1, V2 );

}

//=========================================================================
