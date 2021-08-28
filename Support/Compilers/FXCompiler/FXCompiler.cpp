#include "x_files.hpp"
#include "Entropy.hpp"
#include "effect.hpp"
#include "element_spemitter.hpp"
#include "element_sprite.hpp"
#include "element_mesh.hpp"
#include "element_plane.hpp"
#include "element_shockwave.hpp"
#include "element_cylinder.hpp"
#include "element_sphere.hpp"
#include "TextureMgr.hpp"
#include "errorlog.hpp"

using namespace fx_core;

error_log g_ErrorLog;

//=============================================================================
// Damn linker.

element_spemitter   DummySpemitter;
element_sprite      DummySprite;
element_mesh        DummyMesh;
element_shockwave   DummyShockwave;
element_plane       DummyPlane;
element_sphere      DummySphere;
element_cylinder    DummyCylinder;



char* pExportPC       = NULL;
char* pExportPS2      = NULL;
char* pExportGCN      = NULL;
char* pExportXBOX     = NULL;

//=============================================================================
// Forward declaration

void ExportFile( const char* pFileName );

//=============================================================================
// Main

int main( int argc, char** argv )
{
    g_pTextureMgr->DontUseVRAM();

//    eng_Init();
//    eng_Begin();

    // check params
    if ( argc < 2 )
    {
        x_printf( "\n" );
        x_printf( "Usage:  fx_Export  FileName.fxs  [-TARGET FileNameForTARGET.fxo]\n\n" );
        x_printf( "Where TARGET is PC, PS2, GCN, or XBOX.\n" );
        x_printf( "TARGET option overides export settings within the effect source.\n" );
        x_printf( "Multiple TARGETs can be specified on a single command line.\n\n" );
        return -1;
    }

    if ( argc > 2 )
    {
        s32 i;

        for ( i = 2; i < argc; i++ )
        {
            if ( !(i%2) )
            {
                if ( x_stricmp(argv[i], "-PC") == 0 )
                    pExportPC = argv[i+1];
                else
                if ( x_stricmp(argv[i], "-PS2") == 0 )
                    pExportPS2 = argv[i+1];
                else
                if ( x_stricmp(argv[i], "-GCN") == 0 )
                    pExportGCN = argv[i+1];
                else
                if ( x_stricmp(argv[i], "-XBOX") == 0 )
                    pExportXBOX = argv[i+1];
                else
                {
                    x_printf( "Unknown flag %s\n", argv[i] );
                }
            }
        }
    }
    
    // variables required by findfirst / findnext
    /*
    struct _finddata_t fx_file;
    long hFile;

    // Find matching files
    if( (hFile = _findfirst( argv[1], &fx_file )) == -1L )
    {
        printf( "No matching files found!\n\n" );
        return -1;
    }
    else
    {
        ExportFile( fx_file.name );

        while ( _findnext( hFile, &fx_file ) == 0 )
        {
            ExportFile( fx_file.name );
        }
    }
    */

    ExportFile( argv[1] );

    return 0;
}

// so we don't have to include render library
void pc_PostResetCubeMap( void ){}
void pc_PreResetCubeMap ( void ){}

void ExportFile( const char* pFileName )
{
    effect Effect;
    igfmgr FxFile;

    if( FxFile.Load( pFileName ) == FALSE )
        return;

    Effect.m_IgnoreMeshes = TRUE;
    Effect.Load( FxFile );

    // activate all bitmaps
    //g_pTextureMgr->DontLoad();
    Effect.ActivateAllTextures();

    // Clear the log of errors
    g_ErrorLog.Clear();

//  printf( "Processing %s\n", pFileName );

    // Apply export override values.
    if( pExportPC   )        Effect.m_ExportPC   = pExportPC;
    if( pExportPS2  )        Effect.m_ExportPS2  = pExportPS2; 
    if( pExportGCN  )        Effect.m_ExportGCN  = pExportGCN;
    if( pExportXBOX )        Effect.m_ExportXBOX = pExportXBOX;

    // Export to PC
    if( !Effect.m_ExportPC.IsEmpty() )
    {
        export Export;
        Export.ConstructData( &Effect, EXPORT_TARGET_PC );
        Export.SaveData( (const char*)Effect.m_ExportPC, EXPORT_TARGET_PC );
    //  printf( " - PC  : %s\n", Effect.m_ExportPC );
    }

    // Export to PS2
    if( !Effect.m_ExportPS2.IsEmpty() )
    {
        export Export;
        Export.ConstructData( &Effect, EXPORT_TARGET_PS2 );
        Export.SaveData( (const char*)Effect.m_ExportPS2, EXPORT_TARGET_PS2 );
    //  printf( " - PS2 : %s\n", Effect.m_ExportPS2 );
    }

    // Export to GCN
    if( !Effect.m_ExportGCN.IsEmpty() )
    {
        /*
        export Export;
        Export.ConstructData( &Effect, EXPORT_TARGET_GCN );
        Export.SaveData( (const char*)Effect.m_ExportGCN, EXPORT_TARGET_GCN );
    //  printf( " - GCN : %s\n", Effect.m_ExportGCN );
        */
    }

    // Export to XBOX
    if( !Effect.m_ExportXBOX.IsEmpty() )
    {
        export Export;
        Export.ConstructData( &Effect, EXPORT_TARGET_XBOX );
        Export.SaveData( (const char*)Effect.m_ExportXBOX, EXPORT_TARGET_XBOX );
    //  printf( " - XBOX: %s\n", Effect.m_ExportXBOX );
    } 
}