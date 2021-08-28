#include "x_files.hpp"
#if defined(X_EDITOR)
#include "StdAfx.h"
#endif
#include "RigidDesc.hpp"
#include "..\Support\Render\RigidGeom.hpp"
#include "..\Editor\Project.hpp"

//=========================================================================
// RESOURCE DEFINITION
//=========================================================================

DEFINE_RSC_TYPE( s_RigidResDesc, rigidgeom_rsc_desc, "RigidGeom", "RIGIDGEOM Files (*.rigidgeom)|*.rigidgeom", "Geometry for Rigid Objects" );

//=========================================================================
// FUNCTIONS
//=========================================================================

rigidgeom_rsc_desc::rigidgeom_rsc_desc( void ) : geom_rsc_desc( s_RigidResDesc )
{
    x_memset( m_CollisionFileName, 0, sizeof( m_CollisionFileName ) );

    m_bDoCollision = TRUE;
}

//=========================================================================

void rigidgeom_rsc_desc::OnEnumProp( prop_enum& List )
{
    geom_rsc_desc::OnEnumProp( List );

    List.PropEnumFileName( "ResDesc\\Collision",
                            "MATX Files (*.matx)|*.matx||",
                            "Select an Exported 3DMax File", 0 );

    List.PropEnumBool    ( "ResDesc\\DoCollision",
                            "Set this to false if you don't want to collide with this geometry.", 0 );
}

//=========================================================================

xbool rigidgeom_rsc_desc::OnProperty( prop_query& I )
{
    if( geom_rsc_desc::OnProperty( I ) )
        return TRUE;
    
    if( I.IsVar( "ResDesc\\Collision" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFileName( m_CollisionFileName, 256 );
        }
        else
        {
            x_strsavecpy( m_CollisionFileName, I.GetVarFileName(), 256 );
        }
        
        return( TRUE );
    }

    if( I.VarBool( "ResDesc\\DoCollision", m_bDoCollision ) )
        return TRUE;
    
    return( FALSE );    
}

//=========================================================================

void rigidgeom_rsc_desc::OnGetCompilerDependencies( xarray<xstring>& List )
{
    geom_rsc_desc::OnGetCompilerDependencies( List );

    if( m_CollisionFileName[0] != '\0' )
        List.Append() = m_CollisionFileName;
}

//=========================================================================

void rigidgeom_rsc_desc::OnGetFinalDependencies( xarray<xstring>& List, platform Platform, const char* pDirectory )
{
    //
    // accumulate dependencies from the base class
    //
    geom_rsc_desc::OnGetFinalDependencies( List, Platform, pDirectory );

    //
    // get texture dependencies from the compiled geometry
    //

    // Load the resource
    rigid_geom*         pRigidGeom; 
    fileio              RigidGeomIO;
    RigidGeomIO.Load( xfs( "%s\\%s", pDirectory, GetName() ), pRigidGeom );
    if( pRigidGeom == NULL )
    {
        x_throw( xfs("Unable to open [%s] So I can't check for dependencies", xfs( "%s\\%s", pDirectory, GetName())) );
    }

    // Go through all its textures
    for( s32 i=0; i<pRigidGeom->m_nTextures; i++ )
    {
        List.Append() = pRigidGeom->GetTextureName( i );
    }

    // Cleanup
    delete pRigidGeom;
}

//=========================================================================

#if defined(X_EDITOR)
void rigidgeom_rsc_desc::OnGetCompilerRules( xstring& CompilerRules )
{
    CompilerRules.Clear();

    // name of the compiler
    CompilerRules += "GeomCompiler.exe ";

    // Verbose
    if( IsVerbose() )
        CompilerRules += "-LOG ";

    // colored mips for debugging
    if ( s_ColoredMips )
        CompilerRules += "-COLORMIPS ";

    // physics file?
    if( m_PhysicsMatx[0] != '\0' )
    {
        CompilerRules += "-PHYSICS \"";
        CompilerRules += m_PhysicsMatx;
        CompilerRules += "\" ";
    }

    // settings?
    if( m_SettingsFile[0] != '\0' )
    {
        CompilerRules += "-SETTINGS \"";
        CompilerRules += m_SettingsFile;
        CompilerRules += "\" ";
    }

    // collision file
    if( m_CollisionFileName[0] != '\0' )
    {
        CompilerRules += "-C \"";
        CompilerRules += m_CollisionFileName;
        CompilerRules += "\" ";
    }

    // rigid resource to compile
    char ResourceName[X_MAX_PATH];
    GetFullName( ResourceName );
    CompilerRules += "-RIGID -F \"";
    CompilerRules += ResourceName;
    CompilerRules += "\" ";

    // do collision or not?
    if( m_bDoCollision == FALSE )
    {
        CompilerRules += "-NO_COLLISION ";
    }    

    // output texture path
    CompilerRules += "-TEXTURE_PATH \"";
    CompilerRules += g_Settings.GetSourcePath() ;
    CompilerRules += "\"";
}
#else
void rigidgeom_rsc_desc::OnGetCompilerRules( xstring& CompilerRules )
{
    CompilerRules.Clear();
}
#endif

//=========================================================================

void rigidgeom_rsc_desc::OnCheckIntegrity( void )
{
    geom_rsc_desc::OnCheckIntegrity();

    if (x_stricmp(m_FileName, m_CollisionFileName) == 0)
        x_throw( "\nAre you trying to slow down the game or what!!\nThe collision matx CANNOT be the geometry matx!!\nCreate a low poly collision model you slacker!!\n") ;
}

//=========================================================================
