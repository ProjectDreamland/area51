#include "x_files.hpp"
#if defined(X_EDITOR)
#include "StdAfx.h"
#endif
#include "SkinDesc.hpp"
#include "..\Support\Render\SkinGeom.hpp"
#include "..\Editor\Project.hpp"

//=========================================================================
// RESOURCE DEFINITION
//=========================================================================

DEFINE_RSC_TYPE( s_SkinResDesc, skingeom_rsc_desc, "SkinGeom", "SKINGEOM Files (*.skingeom)|*.skingeom", "Geometry for Skinned Objects" );

//=========================================================================
// FUNCTIONS
//=========================================================================

skingeom_rsc_desc::skingeom_rsc_desc( void ) : geom_rsc_desc( s_SkinResDesc )
{
}

//=========================================================================

void skingeom_rsc_desc::OnEnumProp( prop_enum& List )
{
    geom_rsc_desc::OnEnumProp( List );
}

//=========================================================================

xbool skingeom_rsc_desc::OnProperty( prop_query& I )
{
    if( geom_rsc_desc::OnProperty( I ) )
    {   
        // nothing to do
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//=========================================================================

void skingeom_rsc_desc::OnGetCompilerDependencies( xarray<xstring>& List )
{
    geom_rsc_desc::OnGetCompilerDependencies( List );
}

//=========================================================================

void skingeom_rsc_desc::OnGetFinalDependencies( xarray<xstring>& List, platform Platform, const char* pDirectory )
{
    //
    // accumulate dependencies from the base class
    //
    geom_rsc_desc::OnGetFinalDependencies( List, Platform, pDirectory );

    //
    // get texture dependencies from the compiled geometry
    //

    // Load the resource
    skin_geom*         pSkinGeom; 
    fileio             SkinGeomIO;
    SkinGeomIO.Load( xfs( "%s\\%s", pDirectory, GetName() ), pSkinGeom );
    if( pSkinGeom == NULL )
    {
        x_throw( xfs("Unable to open [%s] So I can't check for dependencies", xfs( "%s\\%s", pDirectory, GetName())) );
    }

    // Go through all its textures
    for( s32 i=0; i<pSkinGeom->m_nTextures; i++ )
    {
        List.Append() = pSkinGeom->GetTextureName( i );
    }

    // Cleanup
    delete pSkinGeom;
}

//=========================================================================

#if defined(X_EDITOR)
void skingeom_rsc_desc::OnGetCompilerRules( xstring& CompilerRules )
{
    CompilerRules.Clear();

    // Name of the compiler
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

    // skin resource to compile
    char ResourceName[X_MAX_PATH];
    GetFullName( ResourceName );
    CompilerRules += "-SKIN -F \"";
    CompilerRules += ResourceName;
    CompilerRules += "\" ";

    // the output texture path
    CompilerRules += "-TEXTURE_PATH \"";
    CompilerRules += g_Settings.GetSourcePath() ;
    CompilerRules += "\"";
}
#else
void skingeom_rsc_desc::OnGetCompilerRules( xstring& CompilerRules )
{
    CompilerRules.Clear();
}
#endif

//=========================================================================

void skingeom_rsc_desc::OnCheckIntegrity( void )
{
    geom_rsc_desc::OnCheckIntegrity();
}

//=========================================================================
