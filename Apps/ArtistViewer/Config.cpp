//==============================================================================
//
//  File:           Config.cpp
//
//  Description:    Config class
//
//  Author:         Stephen Broumley
//
//  Date:           Started July29th, 2002 (C) Inevitable Entertainment Inc.
//
//==============================================================================


//==============================================================================
//  INCLUDES
//==============================================================================
#include "Config.hpp"
#include "ConfigFile.hpp"
#include "Loco/Loco.hpp"
#include "Main.hpp"


//==============================================================================
// DATA
//==============================================================================

config_options  g_Config;

//==============================================================================
// GEOM CLASS FUNCTIONS
//==============================================================================

config_options::object::geom::geom()
{
    m_Type[0]         = 0;
    m_Matx[0]         = 0;
    m_PhysicsMatx[0]  = 0;
    m_SettingsFile[0] = 0;
}

//==============================================================================
// ANIM CLASS FUNCTIONS
//==============================================================================

config_options::object::anim::anim()
{
    m_Name[0] = 0;
    m_Matx[0] = 0;
    m_Type[0] = 0;
    m_Loop = FALSE;
    m_FPS = 30;
    m_BlendTime = -1.0f;
    m_Weight = 1.0f;
    m_LoopFrame = 0;
    m_EndFrameOffset = 0;
    m_ChainAnim[0] = 0;
    m_ChainFrame = -1;
    m_ChainCyclesMin = 1;
    m_ChainCyclesMax = 2;
    m_ChainCyclesInteger = TRUE;
    m_AccumHoriz = FALSE;
    m_AccumVert  = FALSE;
    m_AccumYaw   = FALSE;
    m_Collision  = TRUE;
    m_Gravity    = TRUE;
    m_Handle     = 180;
}

//==============================================================================

// Returns anim info to be used for compiling
void config_options::object::anim::AutoSetupFromName( void )
{
    // Classify the type of animation and set the flags appropriately
    if ( (x_stristr(m_Name, "idle")) && (x_stristr(m_Name, "turn")) )
    {
        // IDLE_TURN
        m_AccumYaw   = TRUE;
        m_AccumHoriz = TRUE;
        m_AccumVert  = FALSE;
        m_Loop       = FALSE;
    }
    else if ( (x_stristr(m_Name, "idle")) && (x_stristr(m_Name, "fidget")) )
    {
        // IDLE_FIDGET
        m_AccumYaw   = FALSE;
        m_AccumHoriz = FALSE;
        m_AccumVert  = FALSE;
        m_Loop       = FALSE;
    }
    else if (x_stristr(m_Name, "idle"))
    {
        // IDLE
        m_AccumYaw   = FALSE;
        m_AccumHoriz = FALSE;
        m_AccumVert  = FALSE;
        m_Loop       = TRUE;
    }
    else if (   (x_stristr(m_Name, "front"))    ||
                (x_stristr(m_Name, "back"))     ||
                (x_stristr(m_Name, "left"))     ||
                (x_stristr(m_Name, "right"))    ||
                (x_stristr(m_Name, "forward"))  ||
                (x_stristr(m_Name, "backward")) ||
                (x_stristr(m_Name, "strafe")) )
    {
        // FRONT, BACK, LEFT, RIGHT
        m_AccumYaw   = FALSE;
        m_AccumHoriz = TRUE;
        m_AccumVert  = FALSE;
        m_Loop       = TRUE;
    }
    else
    {
        // GENERIC
        m_AccumYaw   = FALSE;
        m_AccumHoriz = TRUE;
        m_AccumVert  = FALSE;
        m_Loop       = FALSE;
    }
}



//==============================================================================
// SOUND CLASS FUNCTIONS
//==============================================================================

config_options::object::sound::sound()
{
    m_Type[0]   = 0;
    m_Source[0] = 0;
};

//==============================================================================
// FX CLASS FUNCTIONS
//==============================================================================

config_options::fx::fx()
{
    m_Source[0] = 0;
};


//==============================================================================
// OBJECT CLASS FUNCTIONS
//==============================================================================

// Functions
config_options::object::object()
{
    // Clear
    m_Type         [0] = 0;

    m_Name         [0] = 0;
    m_AttachObject [0] = 0;
    m_AttachBone   [0] = 0;

    m_CompiledGeom[0]  = 0;
    m_CompiledAnim[0]  = 0;
    m_CompiledAudio[0] = 0;

    // Setup array grow counts    
    m_Geoms.SetCapacity( 1 );
    m_Geoms.SetGrowAmount( 2 );
    m_Anims.SetGrowAmount( 8 );
    m_Sounds.SetGrowAmount( 1 );
    m_LODs.SetGrowAmount( 4 );
    
    m_bUseAimer = TRUE;
}

//==============================================================================

// Returns type
config_options::type config_options::object::GetType( void ) const
{
    // Loco?
    if( x_stristr( m_Type, "LOCO" ) )
        return config_options::TYPE_LOCO;
    
    // Lip sync?
    if( x_stristr( m_Type, "LIP" ) )
        return config_options::TYPE_LIP_SYNC;

    // Normal object
    return config_options::TYPE_OBJECT;
}

//==============================================================================

// Returns TRUE if object is soft skinned
xbool config_options::object::IsSoftSkinned( void ) const
{
    // Compiled skinned resource?
    if (x_stristr(m_CompiledGeom, "skin"))
        return TRUE;

    // Check all raw meshes for "skin" in the type
    for (s32 i = 0; i < m_Geoms.GetCount(); i++)
    {
        // Skinned type?
        if (x_stristr(m_Geoms[i].m_Type, "skin"))
            return TRUE;
    }

    // Not soft skinned
    return FALSE;
}

//==============================================================================
// FILE CLASS FUNCTIONS
//==============================================================================

// Functions
config_options::file::file()
{
    m_Name[0] = 0;
}


//==============================================================================
// LIGHT CLASS FUNCTIONS
//==============================================================================

// Constructor
config_options::light::light()
{
    m_Color.Set(255,255,255,255);
    m_Ambient.Set(76,76,76,255);
    m_Position.Set(0,0,-1000);
    m_Intensity = 1;
    m_Radius    = 2000;
    m_State     = light::STATE_ON;
}

//==============================================================================
    
// Operators
xbool config_options::light::operator == (const config_options::light& L ) const
{
    return (x_memcmp(this, &L, sizeof(config_options::light)) == 0);
}

//==============================================================================

xbool config_options::light::operator != (const config_options::light& L ) const
{
    return (x_memcmp(this, &L, sizeof(config_options::light)) != 0);
}

//==============================================================================

// Returns state description
const char* config_options::light::GetStateString( void ) const
{
    switch(m_State)
    {
        case STATE_OFF:         return "OFF";
        case STATE_ON:          return "ON";
        case STATE_FULL_BRIGHT: return "FULL BRIGHT";
    }                        

    return NULL;
}


//==============================================================================
// CONFIG CLASS FUNCTIONS
//==============================================================================

// Constructor/destructor
config_options::config_options()
{
    Init();
}

//==============================================================================

config_options::~config_options()
{
    Kill();
}

//==============================================================================

// Sets up defaults
void config_options::Init( void )
{
    x_DebugMsg("config_options::Init\n");

    // General
    m_ShowHelp  = 0;
    m_ShowStats = 1;

    // System directories
    strcpy(m_SystemDirs[SYSTEM_PC].m_Name,   "PC"  );
    strcpy(m_SystemDirs[SYSTEM_PS2].m_Name,  "PS2" );
    strcpy(m_SystemDirs[SYSTEM_GCN].m_Name,  "GCN" );
    strcpy(m_SystemDirs[SYSTEM_XBOX].m_Name, "XBOX");

    // Data paths
    strcpy(m_ViewerDataPaths[SYSTEM_PC].m_Name,   "C:\\GameData\\A51\\Apps\\ArtistViewer\\PC\\");
    strcpy(m_ViewerDataPaths[SYSTEM_PS2].m_Name,  "C:\\GameData\\A51\\Apps\\ArtistViewer\\PS2\\");
    strcpy(m_ViewerDataPaths[SYSTEM_GCN].m_Name,  "C:\\GameData\\A51\\Apps\\ArtistViewer\\GCN\\");
    strcpy(m_ViewerDataPaths[SYSTEM_XBOX].m_Name, "C:\\GameData\\A51\\Apps\\ArtistViewer\\XBOX\\");

    // Game paths
    strcpy(m_GameDataPaths[SYSTEM_PC].m_Name,   "C:\\GameData\\A51\\Release\\PC\\");
    strcpy(m_GameDataPaths[SYSTEM_PS2].m_Name,  "C:\\GameData\\A51\\Release\\PS2\\");
    strcpy(m_GameDataPaths[SYSTEM_GCN].m_Name,  "C:\\GameData\\A51\\Release\\GCN\\");
    strcpy(m_GameDataPaths[SYSTEM_XBOX].m_Name, "C:\\GameData\\A51\\Release\\XBOX\\");
   
    // Texture source paths
    strcpy(m_TextureSourcePaths[SYSTEM_PC].m_Name,   "C:\\GameData\\A51\\Source\\");
    strcpy(m_TextureSourcePaths[SYSTEM_PS2].m_Name,  "C:\\GameData\\A51\\Source\\");
    strcpy(m_TextureSourcePaths[SYSTEM_GCN].m_Name,  "C:\\GameData\\A51\\Source\\");
    strcpy(m_TextureSourcePaths[SYSTEM_XBOX].m_Name, "C:\\GameData\\A51\\Source\\");

    // Background                                       
    m_BackPic[0] = 0;
    m_BackColor = XCOLOR_BLACK;
    m_GridColor = XCOLOR_GREEN;

    // Setup array grow counts
    m_ConfigFiles.SetCapacity( 16 );
    m_ConfigFiles.SetGrowAmount( 16 );
    m_MissingConfigFiles.SetCapacity( 16 );
    m_MissingConfigFiles.SetGrowAmount( 16 );
    m_Objects.SetCapacity( 10 );
    m_Objects.SetGrowAmount( 100 );

    // Config files             
    m_ConfigFiles.Clear();
    m_MissingConfigFiles.Clear();

    // Screen shot
    m_ScreenShotPath.m_Name[0] = 0;

    // Clear objects
    m_Objects.Clear();

    // Fxs
    m_Fxs.Clear();    
    m_Fxs.SetGrowAmount(10);
}

//==============================================================================

// Kills all data
void config_options::Kill( void )
{
    // Delete objects
    m_Objects.Clear();
}

//==============================================================================

// Loads common config file
xbool config_options::LoadCommon( const char* FileName )
{
    x_DebugMsg("config_options::LoadCommon\n");

    // Try open
    config_file CfgFile;
    if (!CfgFile.OpenFile(FileName))
    {
        x_DebugMsg("\n\nERROR!\nCould not find \"%s\"\n", FileName);
        x_DebugMsg("Make sure your target manager file server root\n");
        x_DebugMsg("and home dir are set to \"C:\\GameData\\A51\\Apps\\ArtistViewer\"\n\n\n");
        return FALSE;
    }

    // Clear config files             
    m_ConfigFiles.Clear();
    m_MissingConfigFiles.Clear();

    // Clear pic
    m_BackPic[0] = 0;

    // Keep looping until end of file is reached
    while(!CfgFile.IsEOF())
    {
        // Read next token
        CfgFile.Read();

        // Search for variable
        file File;
        path Path;

        // Misc
        CfgFile.Read("ShowHelp",              m_ShowHelp );
        CfgFile.Read("ShowStats",             m_ShowStats );
        CfgFile.Read("LightColor",            m_Light.m_Color );
        CfgFile.Read("LightAmbient",          m_Light.m_Ambient );
        CfgFile.Read("LightPosition",         m_Light.m_Position );
        CfgFile.Read("LightIntensity",        m_Light.m_Intensity );
        CfgFile.Read("LightRadius",           m_Light.m_Radius );
        CfgFile.Read("BackPic",               m_BackPic );
        CfgFile.Read("BackCol",               m_BackColor );
        CfgFile.Read("GridCol",               m_GridColor );
        CfgFile.Read("PlayStation2Directory", m_SystemDirs[SYSTEM_PS2] );
        CfgFile.Read("PCDirectory",           m_SystemDirs[SYSTEM_PC] );
        CfgFile.Read("XboxDirectory",         m_SystemDirs[SYSTEM_XBOX] );
        CfgFile.Read("GamecubeDirectory",     m_SystemDirs[SYSTEM_GCN] );

        // ViewerDataPath?
        if ( CfgFile.Read( "ViewerDataPath", Path ) )
        {
            // Read path and append system dirs to setup full paths
            for (s32 i = 0; i < SYSTEM_TOTAL; i++)
            {
                m_ViewerDataPaths[i] = Path;
                x_strcat(m_ViewerDataPaths[i].m_Name, m_SystemDirs[i].m_Name);
                Util_FixPath(m_ViewerDataPaths[i].m_Name);
            }
        }
        
        // GameDataPath?
        if ( CfgFile.Read( "GameDataPath", Path ) )
        {
            // Read path and append system dirs to setup full paths
            for (s32 i = 0; i < SYSTEM_TOTAL; i++)
            {
                m_GameDataPaths[i] = Path;
                x_strcat(m_GameDataPaths[i].m_Name, m_SystemDirs[i].m_Name);
                Util_FixPath(m_GameDataPaths[i].m_Name);
            }
        }

        // SourceDataPath?
        if ( CfgFile.Read( "SourceDataPath", Path ) )
        {
            // Read path and set texture source
            for (s32 i = 0; i < SYSTEM_TOTAL; i++)
                m_TextureSourcePaths[i] = Path;
        }

        // ConfigFile?
        if ( CfgFile.Read( "Config", File.m_Name ) )
        {
            // Add to appropriate config list
            if (Util_DoesFileExist(File.m_Name))
                m_ConfigFiles.Append(File);
            else
                m_MissingConfigFiles.Append(File);
        }

        // ScreenShotPath?
        CfgFile.Read( "ScreenShotPath", m_ScreenShotPath );
    }

    // Done
    CfgFile.CloseFile();

    // Success!
    return TRUE;
}

//==============================================================================

// Loads objects config file
xbool config_options::LoadObjectsFromConfig( const char* FileName, xbool bFxFullPath )
{
    x_DebugMsg("config_options::LoadObjectsFromConfig\n");

    // Try open
    config_file CfgFile;
    if (!CfgFile.OpenFile(FileName))
        return FALSE;

    // Delete the current objects
    m_Objects.Clear();

    // Clear fx list
    m_Fxs.SetCount( 0 );
    m_Fxs.SetGrowAmount( 10 );

    // Keep looping until end of file is reached
    xbool bSuccess = TRUE;
    while( 1 )
    {
        // Read next token
        CfgFile.Read();

        // Fx?
        if ( CfgFile.Is( "Fx" ) )
        {
            char Fx[ X_MAX_PATH ];
            if( CfgFile.Read( "Fx", Fx ) )
                AddFx( Fx, bFxFullPath );
        }

        //---------------------------------------------------------------------------------------------
        // BeginObject?
        //---------------------------------------------------------------------------------------------
        if ( CfgFile.Is( "BeginObject" ) )
        {
            // Create and add a new object
            object& Object = m_Objects.Append();

            // Setup the object
            while( 1 )
            {
                // Read next token
                CfgFile.Read();

                // Read vars?
                CfgFile.Read( "Type",           Object.m_Type );
                CfgFile.Read( "Name",           Object.m_Name );
                CfgFile.Read( "Attach",         Object.m_AttachObject, Object.m_AttachBone );
                CfgFile.Read( "CompiledGeom",   Object.m_CompiledGeom );
                CfgFile.Read( "CompiledAnim",   Object.m_CompiledAnim );
                CfgFile.Read( "CompiledAudio",  Object.m_CompiledAudio );
                CfgFile.Read( "UseAimer",       Object.m_bUseAimer );

                // Old raw mesh support
                if ( CfgFile.Is( "RawMesh" ) )
                {
                    object::geom& Geom = Object.m_Geoms.Append();
                    CfgFile.Read( "RawMesh", Geom.m_Type, Geom.m_Matx );
                }

                // Old raw anim support
                if ( CfgFile.Is( "RawAnim" ) )
                {
                    object::anim& Anim = Object.m_Anims.Append();
                    CfgFile.Read( "RawAnim", Anim.m_Name, Anim.m_Matx );
                    Anim.AutoSetupFromName();
                }

                // Old raw audio support
                if ( CfgFile.Is( "RawAudio" ) )
                {
                    object::sound& Sound = Object.m_Sounds.Append();
                    CfgFile.Read( "RawAudio", Sound.m_Type, Sound.m_Source );
                }

                //---------------------------------------------------------------------------------------------
                // BeginGeom?
                //---------------------------------------------------------------------------------------------
                if ( CfgFile.Is( "BeginGeom" ) )
                {
                    // Create new geometry
                    object::geom& Geom = Object.m_Geoms.Append();

                    // Setup the style
                    while( 1 )
                    {
                        // Read next token
                        CfgFile.Read();

                        // Read vars
                        CfgFile.Read( "Type",         Geom.m_Type );
                        CfgFile.Read( "Matx",         Geom.m_Matx );
                        CfgFile.Read( "PhysicsMatx",  Geom.m_PhysicsMatx );
                        CfgFile.Read( "SettingsFile", Geom.m_SettingsFile );

                        // End?
                        if( CfgFile.Is( "EndGeom" ) )
                            break;

                        // Error?
                        if( CfgFile.IsEOF() )
                        {
                            x_printf("ERROR: EndGeom not found in \n%s\n\n", FileName );
                            bSuccess = FALSE;
                            break;
                        }
                    }
                }

                //---------------------------------------------------------------------------------------------
                // BeginAnim?
                //---------------------------------------------------------------------------------------------
                if ( CfgFile.Is( "BeginAnim" ) )
                {
                    // Create new anim
                    object::anim& Anim = Object.m_Anims.Append();

                    // Setup the style
                    while( 1 )
                    {
                        // Read next token
                        CfgFile.Read();

                        // Name?
                        if ( CfgFile.Read( "Name", Anim.m_Name ) )
                            Anim.AutoSetupFromName();

                        // Read vars
                        CfgFile.Read( "Matx",               Anim.m_Matx );
                        CfgFile.Read( "Type",               Anim.m_Type );
                        CfgFile.Read( "FPS",                Anim.m_FPS );
                        CfgFile.Read( "Loop",               Anim.m_Loop );
                        CfgFile.Read( "LoopFrame",          Anim.m_LoopFrame );
                        CfgFile.Read( "EndFrameOffset",     Anim.m_EndFrameOffset );
                        CfgFile.Read( "Weight",             Anim.m_Weight );
                        CfgFile.Read( "BlendTime",          Anim.m_BlendTime );
                        CfgFile.Read( "AccumHoriz",         Anim.m_AccumHoriz );
                        CfgFile.Read( "AccumVert",          Anim.m_AccumVert );
                        CfgFile.Read( "AccumYaw",           Anim.m_AccumYaw );
                        CfgFile.Read( "ChainAnim",          Anim.m_ChainAnim );
                        CfgFile.Read( "ChainFrame",         Anim.m_ChainFrame );
                        CfgFile.Read( "ChainCyclesMin",     Anim.m_ChainCyclesMin );
                        CfgFile.Read( "ChainCyclesMax",     Anim.m_ChainCyclesMax );
                        CfgFile.Read( "ChainCyclesInteger", Anim.m_ChainCyclesInteger );

                        // End?
                        if( CfgFile.Is( "EndAnim") )
                            break;

                        // Error?
                        if( CfgFile.IsEOF() )
                        {
                            x_printf("ERROR: EndAnim not found in \n%s\n\n", FileName );
                            bSuccess = FALSE;
                            break;
                        }
                    }
                }

                //---------------------------------------------------------------------------------------------
                // BeginLOD?
                //---------------------------------------------------------------------------------------------
                if ( CfgFile.Is( "BeginLOD" ) )
                {
                    // Create new lod
                    object::lod& LOD = Object.m_LODs.Append();

                    // Setup the style
                    while( 1 )
                    {
                        // Read next token
                        CfgFile.Read();

                        // Read vars
                        CfgFile.Read( "ScreenSize", LOD.m_ScreenSize );
                        
                        // Mesh?
                        if( CfgFile.Is( "Mesh" ) )
                        {
                            object::mesh& Mesh = LOD.m_Meshes.Append();
                            x_strcpy( Mesh.m_Name, CfgFile.ReadString() );
                        }

                        // End?
                        if ( CfgFile.Is( "EndLOD") )
                            break;

                        // Error?
                        if( CfgFile.IsEOF() )
                        {
                            x_printf("ERROR: EndLOD not found in \n%s\n\n", FileName );
                            bSuccess = FALSE;
                            break;
                        }
                    }
                }

                // End?
                if (CfgFile.Is( "EndObject") )
                    break;

                // Error?
                if( CfgFile.IsEOF() )
                {
                    x_printf("ERROR: EndObject not found in \n%s\n\n", FileName );
                    bSuccess = FALSE;
                    break;
                }
            }
        }

        // End?
        if ( CfgFile.IsEOF() )
            break;
    }

    // Done
    CfgFile.CloseFile();

    return bSuccess;
}

//==============================================================================

// Loads objects directory file
xbool config_options::LoadObjectsFromDirectory( const char* FileName )
{
    x_DebugMsg("config_options::LoadObjectsFromDirectory\n");

    // Try open over a second (file may not have been finished written yet)
    config_file CfgFile;
    xtimer TryTimer;
    TryTimer.Reset();
    TryTimer.Start();
    xbool bOpened = FALSE;
    do
    {
        // Try open
        bOpened = CfgFile.OpenFile(FileName);
    }
    while((!bOpened) && (TryTimer.ReadSec() < 6));

    // Failed?
    if (!bOpened)
        return FALSE;

    // Count # of geometry files
    s32 nGeoms = 0;
    s32 nFxs   = 0;
    while(!CfgFile.IsEOF())
    {
        // Read next line
        CfgFile.ReadLine();
        if (CfgFile.IsEOF())
            break;

        // Split file
        char Ext  [X_MAX_EXT  ];
        x_splitpath(CfgFile.String(), NULL, NULL, NULL, Ext);

        // Is this a valid .rigidgeom or .skingeom resource?
        if(     ( x_stricmp( Ext, ".rigidgeom" ) == 0 )
             || ( x_stricmp( Ext, ".skingeom"  ) == 0 ) )
        {
            nGeoms++;
        }
        
        // Is this a valid .fxo resource?
        if( x_stricmp(Ext, ".fxo") == 0 )
        {
            nFxs++;
        }
    }

    // Allocate config objects
    m_Objects.Clear();
    m_Objects.SetCapacity( nGeoms );
    m_Objects.SetGrowAmount( 100 );

    // Pre-allocate fx list
    m_Fxs.SetCount( 0 );
    m_Fxs.SetCapacity( nFxs );
    
    // Add geometry files
    s32 UpdateCount = 0;
    CfgFile.Rewind();
    while(!CfgFile.IsEOF())
    {
        // Read next line
        CfgFile.ReadLine();
        if (CfgFile.IsEOF())
            break;

        // Split file
        char Drive[X_MAX_DRIVE];
        char Dir  [X_MAX_DIR  ];
        char FName[X_MAX_FNAME];
        char Ext  [X_MAX_EXT  ];
        x_splitpath(CfgFile.String(), Drive, Dir, FName, Ext);

        // Is this a valid .rigidgeom or .skingeom resource?
        if(     ( x_stricmp( Ext, ".rigidgeom" ) == 0 )
             || ( x_stricmp( Ext, ".skingeom"  ) == 0 ) )
        {
            // Create and add a new object
            object& Object = m_Objects.Append();

            // Setup the object compiled geom name
            x_makepath(Object.m_CompiledGeom, NULL, NULL, FName, Ext);

            // Show info
            x_printf( "Found %s\n", Object.m_CompiledGeom );
            if( ( (++UpdateCount) & 31 ) == 0 )
                eng_PageFlip() ;
        }
        
        // Is this a valid .fxo resource?
        if( x_stricmp( Ext, ".fxo") == 0 )
        {
            config_options::fx& Fx = m_Fxs.Append();
            x_makepath( Fx.m_Source, NULL, NULL, FName, Ext );
            
            // Show info
            x_printf( "Found %s\n", Fx.m_Source );
            if( ( (++UpdateCount) & 31 ) == 0 )
                eng_PageFlip() ;
        }
    }

    // Now look for animation files that match the geometry files
    CfgFile.Rewind();
    while(!CfgFile.IsEOF())
    {
        // Read next line
        CfgFile.ReadLine();
        if (CfgFile.IsEOF())
            break;

        // Split file
        char Drive[X_MAX_DRIVE];
        char Dir  [X_MAX_DIR  ];
        char FName[X_MAX_FNAME];
        char Ext  [X_MAX_EXT  ];
        x_splitpath(CfgFile.String(), Drive, Dir, FName, Ext);

        // Is this a valid animation resource?
        if (x_stricmp(Ext, ".anim") == 0)
        {
            // Check all objects for geometry
            for (s32 i = 0; i < m_Objects.GetCount(); i++)
            {
                // Lookup object
                object& Object = m_Objects[i];

                // Grab geom file name
                char GeomFName[X_MAX_FNAME];
                x_splitpath(Object.m_CompiledGeom, NULL, NULL, GeomFName, NULL);

                // Is anim name at front of geometry name?
                if (x_stristr(GeomFName, FName) == &GeomFName[0])
                {
                    // Setup animation name
                    x_makepath(Object.m_CompiledAnim, NULL, NULL, FName, Ext);
                    
                    // Show info
                    x_printf( "Found %s\n", Object.m_CompiledAnim );
                    if( ( (++UpdateCount) & 7 ) == 0 )
                        eng_PageFlip() ;
                }
            }
        }
    }

    eng_PageFlip() ;

    // Done
    CfgFile.CloseFile();

    // Success!
    return TRUE;
}

//==============================================================================

// Returns native viewer data path
const char* config_options::GetViewerDataPath( void ) const
{
#ifdef TARGET_PC
    return m_ViewerDataPaths[SYSTEM_PC].m_Name;
#endif

#ifdef TARGET_PS2
    return m_ViewerDataPaths[SYSTEM_PS2].m_Name;
#endif

#ifdef TARGET_GCN
    return m_ViewerDataPaths[SYSTEM_GCN].m_Name;
#endif

#ifdef TARGET_XBOX
    return m_ViewerDataPaths[SYSTEM_XBOX].m_Name;
#endif
}

//==============================================================================

// Returns native game data path
const char* config_options::GetGameDataPath  ( void ) const
{
#ifdef TARGET_PC
    return m_GameDataPaths[SYSTEM_PC].m_Name;
#endif

#ifdef TARGET_PS2
    return m_GameDataPaths[SYSTEM_PS2].m_Name;
#endif

#ifdef TARGET_GCN
    return m_GameDataPaths[SYSTEM_GCN].m_Name;
#endif

#ifdef TARGET_XBOX
    return m_GameDataPaths[SYSTEM_XBOX].m_Name;
#endif
}

//==============================================================================

// Adds fx if not already present
void config_options::AddFx( const char* pFx, xbool bFullPath )
{
    char Name[ X_MAX_PATH ];
    
    // Use full path?
    if( bFullPath )
    {
        x_strcpy( Name, pFx );
    }
    else
    {
        // Just keep filename and binary extension
        char FName[ X_MAX_FNAME ];
        char Ext  [ X_MAX_EXT   ];
        x_splitpath( pFx, NULL, NULL, FName, Ext );
        x_makepath( Name, NULL, NULL, FName, ".fxo" );
    }

    // Check all objects
    for( s32 i = 0; i < m_Fxs.GetCount(); i++ )
    {
        // Already found?
        if( x_stricmp( m_Fxs[i].m_Source, Name ) == 0 )
            return;
    }

    // Add and setup new fx
    fx& Fx = m_Fxs.Append();
    x_strcpy( Fx.m_Source, Name );
}

//==============================================================================
