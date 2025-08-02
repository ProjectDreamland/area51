//=========================================================================
// INCLUDES
//=========================================================================
#include <stdio.h>
#include <process.h>
#include <io.h>

#include "Config.hpp"
#include "Util.hpp"
#include "MeshUtil\RawAnim.hpp"


//=========================================================================
// DEFINES
//=========================================================================
//#define OUTPUT_PARAMS     // Outputs params to dos window for debugging

//=========================================================================
// FUNCTIONS
//=========================================================================

// This is here so it compiles without entropy
void eng_PageFlip(void)
{
}

//=========================================================================

// Output to DOS window
void OutputParams ( const xstring& S )
{

#ifdef OUTPUT_PARAMS

    x_printf(S) ;
    x_printf("\n") ;

#else

    (void)S ;
    
#endif

}

//=========================================================================

// Returns viewer compile path
const char* GetViewerDataPath( config_options::system System )
{
    ASSERT(System >= 0) ;
    ASSERT(System < config_options::SYSTEM_TOTAL) ;

    return g_Config.m_ViewerDataPaths[System].m_Name ;
}

//=========================================================================

// Returns texture source path
const char* GetTextureSourcePath( config_options::system System )
{
    ASSERT(System >= 0) ;
    ASSERT(System < config_options::SYSTEM_TOTAL) ;

    return g_Config.m_TextureSourcePaths[System].m_Name ;
}

//=========================================================================

// Returns system compile string
const char* GetSystemCompile( config_options::system System )
{
    // Final compile command
    switch(System)
    {
        case config_options::SYSTEM_PC:   return "PC" ;
        case config_options::SYSTEM_PS2:  return "PS2" ;
        case config_options::SYSTEM_XBOX: return "XBOX" ;
        case config_options::SYSTEM_GCN:  return "GCN" ;
    }

    ASSERT(0) ;
    return "" ;
}

//=========================================================================

// Compiles geometry, animations, and audio for object
void CompileObject( config& Config, config_options::object& Object, config_options::system System, xbool bVerbose )
{
    s32     i,j ;
    X_FILE* pFile ;

    // Must have a mesh!
    if (Object.m_Geoms.GetCount() == 0)
        return ;

    // Geometry exist?
    if (Util_DoesFileExist(Object.m_Geoms[0].m_Matx, "ERROR: ") == FALSE)
        return ;

    //=====================================================================
    // Compile geometry
    //=====================================================================
    pFile = x_fopen("Geom.txt", "wt") ;
    if (pFile)
    {
        x_printf("\n") ;

        xstring Params, Line ;

        // Type
        if (Object.IsSoftSkinned())
            Params =" -SKIN" ;
        else
            Params = " -RIGID" ;

        // Verbose?
        if( bVerbose )
            Params +=" -LOG";
        
        // Source .matx
        Line.Format(" -F \"%s\"", Object.m_Geoms[0].m_Matx) ; Params += Line ;
        
        // Texture path
        Line.Format(" -TEXTURE_PATH \"%s\"", GetTextureSourcePath(System)) ; Params += Line ;
        
        // Physics matx?
        if( Object.m_Geoms[0].m_PhysicsMatx[0] )
        {         
            // Only add if present
            if (Util_DoesFileExist(Object.m_Geoms[0].m_PhysicsMatx, "ERROR: ") == TRUE)
            {
                Line.Format(" -PHYSICS \"%s\"", Object.m_Geoms[0].m_PhysicsMatx ); 
                Params += Line;
            }         
        }

        // Settings file?
        if( Object.m_Geoms[0].m_SettingsFile[0] )
        {         
            // Only add if present
            if (Util_DoesFileExist(Object.m_Geoms[0].m_SettingsFile, "ERROR: ") == TRUE)
            {
                Line.Format(" -SETTINGS \"%s\"", Object.m_Geoms[0].m_SettingsFile ); 
                Params += Line;
            }                
        }

        // Final compile command
        Line.Format(" -%s \"%s%s\"", 
                    GetSystemCompile(System), 
                    GetViewerDataPath(System), 
                    Object.m_CompiledGeom) ; 
        Params += Line ;

        // Print to screen and add to file
        OutputParams( Params ) ;
        x_fprintf(pFile, Params) ;

        // Done
        x_fclose(pFile) ;

        // Compile
        x_printf("Compiling %s \"%s\"\n", GetSystemCompile(System), Object.m_CompiledGeom) ;
        _flushall() ;
        if (_spawnl(_P_WAIT, "GeomCompiler.exe", "GeomCompiler.exe", "@Geom.txt", NULL) == -1)
            x_printf("ERROR: Geometry compile failed!\n") ;
    }
    else
    {
        // Show error
        x_printf("ERROR - Could not create Geom.txt") ;
    }

    //=====================================================================
    // Compile animations
    //=====================================================================
    if (Object.m_Anims.GetCount())
    {
        x_printf("\n") ;

        // Create compile animation script
        pFile = x_fopen("Anim.txt", "wt") ;
        if (pFile)
        {
            xstring Params ;

            // Package params
            Params.Format(" -BINDPOSE \"%s\" -KEEPBIND 0 ", Object.m_Geoms[0].m_Matx) ;

            // Verbose?
            if( bVerbose )
                Params +=" -LOG";
            
            // Print to screen and add to file
            OutputParams( Params ) ;
            x_fprintf(pFile, Params) ;

            // Add all animations to output
            s32 nAnimsAdded = 0 ;
            for (i = 0 ; i < Object.m_Anims.GetCount() ; i++)
            {
                // Lookup anim name and type
                config_options::object::anim& Anim = Object.m_Anims[i] ;

                // Skip if not there
                if (Util_DoesFileExist(Anim.m_Matx, "WARNING: ") == FALSE)
                    continue ;

                // Load anim?
                rawanim RawAnim;
                if( RawAnim.Load( Anim.m_Matx ) )
                {
                    // Search for particle super events
                    for( j = 0; j < RawAnim.m_nSuperEvents; j++ )
                    {
                        // Is this a particle event?
                        rawanim::super_event& SE = RawAnim.m_pSuperEvent[j];
                        if( SE.Type == EVENT_TYPE_PARTICLE )
                        {
                            // Compile full name
                            char Fx[ X_MAX_PATH ];                           
                            x_strcpy( Fx, "C:\\GameData\\A51\\Source\\Art\\Particles\\");
                            x_strcat( Fx, SE.Strings[0] );
                            x_strcat( Fx, ".fxs" );                        
                        
                            // Add to object if not already
                            Config.AddFx( Fx, TRUE );
                        }
                    }                        
                }            

                // Create params
                Params.Clear() ;

                // Add to package
                xstring Line ;
                Line.Format(" -NAME \"%s\"",                Anim.m_Name) ;               Params += Line ;
                Line.Format(" -LOOP %d",                    Anim.m_Loop) ;               Params += Line ;
                Line.Format(" -LOOP_FRAME %d",              Anim.m_LoopFrame) ;          Params += Line ;
                Line.Format(" -END_FRAME_OFFSET %d",        Anim.m_EndFrameOffset) ;     Params += Line ;
                Line.Format(" -FPS %d",                     Anim.m_FPS) ;                Params += Line ;
                
                Line.Format(" -ACCUM_HORIZ_MOTION %d",      Anim.m_AccumHoriz) ;         Params += Line ;
                Line.Format(" -ACCUM_VERT_MOTION %d",       Anim.m_AccumVert) ;          Params += Line ;
                Line.Format(" -ACCUM_YAW_MOTION %d",        Anim.m_AccumYaw) ;           Params += Line ;
                Line.Format(" -GRAVITY %d",                 Anim.m_Gravity) ;            Params += Line ;
                Line.Format(" -WORLD_COLLISION %d",         Anim.m_Collision) ;          Params += Line ;

                Line.Format(" -WEIGHT %f",                  Anim.m_Weight) ;             Params += Line ;
                Line.Format(" -BLEND_TIME %f",              Anim.m_BlendTime) ;          Params += Line ;
                Line.Format(" -HANDLE %d",                  Anim.m_Handle) ;             Params += Line ;
                
                Line.Format(" -CHAIN_ANIM \"%s\"",          Anim.m_ChainAnim) ;          Params += Line ;
                Line.Format(" -CHAIN_FRAME %d",             Anim.m_ChainFrame) ;         Params += Line ;
                Line.Format(" -CHAIN_CYCLES_MIN %f",        Anim.m_ChainCyclesMin) ;     Params += Line ;
                Line.Format(" -CHAIN_CYCLES_MAX %f",        Anim.m_ChainCyclesMax) ;     Params += Line ;
                Line.Format(" -CHAIN_CYCLES_INTEGER %d",    Anim.m_ChainCyclesInteger) ; Params += Line ;
                
                // THIS MUST BE LAST!!!!
                Line.Format(" -ANIM \"%s\"",                Anim.m_Matx) ;               Params += Line ;

                // Update count
                nAnimsAdded++ ;

                // Print to screen and add to file
                OutputParams( Params ) ;
                x_fprintf(pFile, Params) ;
            }

            // Platform
            Params.Clear() ;
            Params.Format(" -%s \"%s%s\"", 
                          GetSystemCompile(System), 
                          GetViewerDataPath(System), 
                          Object.m_CompiledAnim) ;


            // Print to screen and add to file
            OutputParams( Params ) ;
            x_fprintf(pFile, Params) ;

            // Done
            x_fclose(pFile) ;

            // Compile
            if (nAnimsAdded)
            {
                x_printf("Compiling %s \"%s\"\n", GetSystemCompile(System), Object.m_CompiledAnim) ;
                _flushall() ;
                if (_spawnl(_P_WAIT, "AnimCompiler.exe", "AnimCompiler.exe", "@Anim.txt", NULL) == -1)
                    x_printf("ERROR: Animation compile failed!\n") ;
            }
        }
        else
        {
            // Show error
            x_printf("ERROR - Could not create Anim.txt") ;
        }
    }

    //=====================================================================
    // Compile audio
    //=====================================================================
    if (Object.m_Sounds.GetCount())
    {
        x_printf("\n") ;

        // Create compile animation script
        pFile = x_fopen("Audio.txt", "wt") ;
        if (pFile)
        {
            // Get package name
            char PackageFName[X_MAX_FNAME] ;
            x_splitpath(Object.m_CompiledAudio, NULL, NULL, PackageFName, NULL) ;

            // Package params
            x_fprintf(pFile, "package:\n") ;
            x_fprintf(pFile, "%s\n", PackageFName) ;
            x_fprintf(pFile, "[effect=0]\n");
            x_fprintf(pFile, "\n") ;

            // Count the number of valid sounds
            s32 nSoundsAdded = 0 ;
            for (i = 0 ; i < Object.m_Sounds.GetCount() ; i++)
            {
                // Lookup sound
                config_options::object::sound& Sound = Object.m_Sounds[i] ;

                // Skip if not there
                if (Util_DoesFileExist(Sound.m_Source, "WARNING: ") == FALSE)
                     continue ;

                // Update total
                nSoundsAdded++ ;
            }

            // Add all sounds to output
            x_fprintf(pFile, "files:\n") ;
            for (i = 0 ; i < Object.m_Sounds.GetCount() ; i++)
            {
                // Lookup sound
                config_options::object::sound& Sound = Object.m_Sounds[i] ;

                // Skip if not there
                if (Util_DoesFileExist(Sound.m_Source, NULL) == FALSE)
                    continue ;

                // Get sound filename
                char SoundFName[X_MAX_FNAME] ;
                x_splitpath(Sound.m_Source, NULL, NULL, SoundFName, NULL) ;

                // Insert a dummy sound due to audio driver bug?
                if ((i == 0) && (nSoundsAdded == 1))
                {
                    // Add dummy sound to compiler script
                    x_fprintf(pFile, "f_%s_DUMMY\n", SoundFName) ;
                    x_fprintf(pFile, "COLD\n") ;
                    x_fprintf(pFile, "NoLipSync\n") ;
                    x_fprintf(pFile, "%s\n", Sound.m_Source) ;
                    x_fprintf(pFile, "\n") ;
                }

                // Add sound to compiler script
                x_fprintf(pFile, "f_%s\n", SoundFName) ;
                x_fprintf(pFile, "COLD\n") ;
                x_fprintf(pFile, "NoLipSync\n") ;
                x_fprintf(pFile, "%s\n", Sound.m_Source) ;
                x_fprintf(pFile, "\n") ;
            }
            x_fprintf(pFile, "\n") ;

            // Add descriptors
            x_fprintf(pFile, "descriptors:\n") ;
            for (i = 0 ; i < Object.m_Sounds.GetCount() ; i++)
            {
                // Lookup sound
                config_options::object::sound& Sound = Object.m_Sounds[i] ;

                // Skip if not there
                if (Util_DoesFileExist(Sound.m_Source, NULL) == FALSE)
                    continue ;

                // Get sound filename
                char SoundFName[X_MAX_FNAME] ;
                x_splitpath(Sound.m_Source, NULL, NULL, SoundFName, NULL) ;

                // Insert a dummy sound due to audio driver bug?
                if ((i == 0) && (nSoundsAdded == 1))
                {
                    // Add dummy sound to compiler script
                    x_fprintf(pFile, "%s_DUMMY\n", Sound.m_Type) ;
                    x_fprintf(pFile, "simple\n") ;
                    x_fprintf(pFile, "f_%s_DUMMY\n", SoundFName) ;
                    x_fprintf(pFile, "\n") ;
                }

                // Add sound to compiler script
                x_fprintf(pFile, "%s\n", Sound.m_Type) ;
                x_fprintf(pFile, "simple\n") ;
                x_fprintf(pFile, "f_%s\n", SoundFName) ;
                x_fprintf(pFile, "\n") ;
            }
            x_fprintf(pFile, "\n") ;

            // Final compile command
            x_fprintf(pFile, "output:\n") ; 
            x_fprintf(pFile, "%s%s\n", GetViewerDataPath(System), Object.m_CompiledAudio) ;

            // Done
            x_fclose(pFile) ;

            // Compile?
            if (nSoundsAdded)
            {
                x_printf("Compiling %s \"%s\"\n", GetSystemCompile(System), Object.m_CompiledAudio) ;
                _flushall() ;
                if (_spawnl(_P_WAIT, 
                            "SoundPackager.exe", 
                            "SoundPackager.exe", 
                            "-V",
                            xfs("-%s", GetSystemCompile(System)),
                            "Audio.txt",
                            NULL) == -1)
                {
                    x_printf("ERROR: Audio compile failed!\n") ;
                }
            }
        }
        else
        {
            // Show error
            x_printf("ERROR - Could not create Audio.txt") ;
        }
    }
}

//=========================================================================

void CompileFxs( config& Config, config_options::system System )
{
    s32 i;
    
    //=====================================================================
    // Compile fx
    //=====================================================================
    x_printf("\n") ;
    for( i = 0 ; i < Config.m_Fxs.GetCount(); i++ )
    {
        xstring Params, Line ;

        // Eg.
        // C:\GameData\A51\Apps\Compilers_Dev\fx_Export.exe 
        // "C:\GameData\A51\Source\Art\Particles\AH_barrelleak_000.fxs"
        // -PS2 "C:\GameData\A51\Release\PS2\AH_Barrelleak_000.fxo" 

        // Split up source name
        char Drive [ X_MAX_DRIVE ];
        char Dir   [ X_MAX_DIR   ];
        char FName [ X_MAX_FNAME ];
        char Ext   [ X_MAX_EXT   ];
        x_splitpath( Config.m_Fxs[i].m_Source, Drive, Dir, FName, Ext );

        // Make output file
        char OutputFile[ X_MAX_PATH ];
        x_makepath( OutputFile, NULL, NULL, FName, ".fxo" );

        // Source .fxs
        Line.Format("\"%s\"", Config.m_Fxs[i].m_Source ); Params += Line ;

        // Final compile command
        Line.Format(" -%s \"%s%s\"", 
            GetSystemCompile(System), 
            GetViewerDataPath(System), 
            OutputFile );
        Params += Line ;

        // Print to screen
        OutputParams( Params ) ;

        // Compile
        x_printf("Compiling %s \"%s\"\n", GetSystemCompile(System), OutputFile) ;
        _flushall() ;
        if (_spawnl(_P_WAIT, "fx_Export.exe", "fx_Export.exe", (const char*)Params, NULL) == -1)
            x_printf("ERROR: Fx compile failed!\n") ;
    }
}

//=========================================================================

// Compiles all objects in config file
void CompileConfig( const char* pConfig, xbool bVerbose )
{
    s32 i;
    
    // Show info
    x_printf("\n") ;
    x_printf("***************************************************************************\n") ;
    x_printf("Compiling \"%s\"\n", pConfig) ;
    x_printf("***************************************************************************\n") ;
    
    // File read successfully?
    if (g_Config.LoadObjectsFromConfig(pConfig, TRUE))
    {
        // Compile all objects
        for( i = 0; i < g_Config.m_Objects.GetCount(); i++ )
        {
            //CompileObject(g_Config.m_Objects[i], config_options::SYSTEM_PC) ;
            CompileObject(g_Config, g_Config.m_Objects[i], config_options::SYSTEM_PS2, bVerbose ) ;
        }
        CompileFxs( g_Config, config_options::SYSTEM_PS2 );
    }
    else
    {
        // Show error
        x_printf("ERROR - Could not open \"%s\"\n", pConfig) ;        
    }
}

//=========================================================================

// Main entry point
void main( s32 argc, char* argv[] )
{
    // Show info
#ifdef TARGET_PC
    x_printf("A51 VIEWER COMPILER BUILT %s\n\n", __TIMESTAMP__) ;
#else
    x_printf("A51 VIEWER COMPILER BUILT %s %s\n\n", __DATE__, __TIME__) ;
#endif

    // Try load common config file
    if (g_Config.LoadCommon("common.cfg") == 0)
    {
        // Show error
        x_printf("Error - Common.cfg not found!\n") ;

        // Exit
        exit(EXIT_FAILURE) ;
    }
    
    //TEMP: Comment this in to compile a certain config file...
    //CompileConfig("ParticleFx.cfg", TRUE);
    //argc = 0;

    // If there is only 1 parameter (the .exe), then auto search and compile all config files
    if (argc == 1)
    {
        x_printf("Auto searching for config files...\n") ;

	    // Find all matching files in this directory
    	_finddata_t FileData;
	    long        FindData ;
	    FindData = _findfirst("*.cfg", &FileData) ;
	    if (FindData != -1)
        {
		    do
            {
                // No file?
                if (!FileData.name)
                    continue ;

                // Skip hidden/system/directories
                if (FileData.attrib & (_A_HIDDEN | _A_SYSTEM | _A_SUBDIR))
                    continue ;

                // Skip "common.cfg"
                if (x_stricmp(FileData.name, "common.cfg") == 0)
                    continue ;

                // Compile
                CompileConfig( FileData.name, TRUE ) ;

            } while(_findnext(FindData, &FileData) == 0) ;
            _findclose( FindData );
        }
    }
    else
    {
        // Loop through and compile all config files
        for (s32 i = 0 ; i < (argc-1) ; i++)
            CompileConfig(argv[i+1], FALSE) ;
    }

    // Close down
    g_Config.Kill() ;

    // HACK BECAUSE X FILES SHUT DOWN CRASHES IN THREAD CODE!!!
    _exit(0) ;
}

//=========================================================================

