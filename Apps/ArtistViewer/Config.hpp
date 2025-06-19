//==============================================================================
//
//  File:           Config.hpp
//
//  Description:    Config class
//
//  Author:         Stephen Broumley
//
//  Date:           Started July29th, 2002 (C) Inevitable Entertainment Inc.
//
//==============================================================================

#ifndef __CONFIG_HPP__
#define __CONFIG_HPP__

//==============================================================================
//  INCLUDES
//==============================================================================
#include "Entropy.hpp"
#include "Animation/AnimData.hpp"

//==============================================================================
//  CLASSES
//==============================================================================

// Contains config info
class config_options
{
//==========================================================================
// DEFINES
//==========================================================================
public:

    // Compile platforms
    enum system
    {
        SYSTEM_PC,
        SYSTEM_PS2,
        SYSTEM_GCN,
        SYSTEM_XBOX,

        SYSTEM_TOTAL
    };

    // Types
    enum type
    {
        TYPE_NULL,
        TYPE_LOCO,
        TYPE_LIP_SYNC,
        TYPE_OBJECT
    };


//==========================================================================
// STRUCTURES
//==========================================================================
public:
    
    //======================================================================

    // Generic object read in from config files
    class object
    {
    public:
        
        // Simple geom structure
        class geom
        {
        public:
            // Data
            char    m_Type        [X_MAX_PATH];
            char    m_Matx        [X_MAX_PATH];
            char    m_PhysicsMatx [X_MAX_PATH];
            char    m_SettingsFile[X_MAX_PATH];

            // Functions
            geom();
        };

        //======================================================================

        // Simple anim structure
        class anim
        {
        public:
            // Data
            char    m_Name          [ X_MAX_PATH ];
            char    m_Matx          [ X_MAX_PATH ];
            char    m_Type          [ 32 ];
            xbool   m_Loop;
            s32     m_FPS;
            f32     m_BlendTime;
            f32     m_Weight;
            s32     m_LoopFrame;
            s32     m_EndFrameOffset;
            char    m_ChainAnim[ X_MAX_PATH ];
            s32     m_ChainFrame;
            f32     m_ChainCyclesMin;
            f32     m_ChainCyclesMax;
            xbool   m_ChainCyclesInteger;
            xbool   m_AccumHoriz;
            xbool   m_AccumVert;
            xbool   m_AccumYaw;
            xbool   m_Collision;
            xbool   m_Gravity;
            s32     m_Handle;

            // Functions
            anim();
            void AutoSetupFromName( void );
        };

        //======================================================================

        // Simple sound structure
        class sound
        {
        public:
            // Data
            char    m_Type  [X_MAX_PATH];
            char    m_Source[X_MAX_PATH];

            // Functions
            sound();
        };

        // Mesh
        struct mesh
        {
            // Data
            char    m_Name[64];

            // Functions
            mesh()
            {
                m_Name[0] = 0;
            }
        };

        // LOD
        struct lod
        {
            // Data
            f32             m_ScreenSize;
            xarray<mesh>    m_Meshes;            
            u64             m_Mask;

            // Functions
            lod()
            {
                m_ScreenSize = 100.0f;
                m_Mask       = 0;
                m_Meshes.SetGrowAmount( 4 );
            }
        };

        // Data
        char                m_Type          [X_MAX_PATH];       // Type of config file

        char                m_Name          [X_MAX_PATH];       // Name of object
        char                m_AttachObject  [X_MAX_PATH];       // Name of object attached to
        char                m_AttachBone    [X_MAX_PATH];       // Name of bone attached to

        char                m_CompiledGeom  [X_MAX_PATH];       // Name of geom package to compile
        char                m_CompiledAnim  [X_MAX_PATH];       // Name of anim package to compile
        char                m_CompiledAudio [X_MAX_PATH];       // Name of audio package to compile
        
        xarray<config_options::object::geom>        m_Geoms;                            // List of mesh .matx files
        xarray<config_options::object::anim>        m_Anims;                            // List of animation .matx files
        xarray<config_options::object::sound>       m_Sounds;                           // List of sound .wav files

        xarray<lod>         m_LODs;                             // List of lods
        xbool               m_bUseAimer;        // Should aimer/eye tracking etc be turned on

        // Functions
        object();

        // Returns type
        type GetType( void ) const;

        // Returns TRUE if object is soft skinned
        xbool IsSoftSkinned( void ) const;
    };

    //======================================================================

    // Simple filename class
    class file
    {
        // Data
    public:
        char    m_Name[X_MAX_PATH];

        // Functions
        file();
    };

    //======================================================================

    // Light
    struct light
    {
        // Defines
        enum state
        {
            STATE_OFF,
            STATE_ON,
            STATE_FULL_BRIGHT,

            STATE_COUNT
        };

        // Data
        xcolor  m_Color;       // Color
        xcolor  m_Ambient;     // Ambient
        vector3 m_Position;    // Position
        f32     m_Intensity;   // Intensity
        f32     m_Radius;      // Radius
        s32     m_State;       // State

        // Constructor
        light();

        // Operators
        xbool operator == (const light& L ) const;
        xbool operator != (const light& L ) const;

        // Returns state description
        const char* GetStateString( void ) const;
    };

    //======================================================================

    struct path
    {
        char m_Name[X_MAX_PATH];
    };

    //======================================================================

    // Simple fx
    class fx
    {
    public:
        // Data
        char    m_Source[X_MAX_PATH];

        // Functions
        fx();
    };

    //======================================================================

//==========================================================================
// DATA
//==========================================================================

public:
    //HACK HACK HACK HACK
    xbool       DemoBuild;
    xbool       AutoCampaign;
    xbool       AutoSplitScreen;
    xbool       AutoServer;
    xbool       AutoClient;
    s32         AutoLevel;              // If AutoCampaign or AutoServer.
    s32         AutoServerType;         // Only used if AutoServer.  GameMgr.hpp.
    s32         AutoMutateMode;         // Only used if AutoServer.  GameMgr.hpp.
    s32         AutoMonkeyMode;         // Used for specifying monkey settings on startup
    char        AutoServerName[32];     // Only used if AutoServer/Client.

    // General
    s32                 m_ShowHelp;                    // Help
    s32                 m_ShowStats;                   // Frame rate etc

    // Paths
    path                m_SystemDirs        [SYSTEM_TOTAL]; // System directories
    path                m_ViewerDataPaths   [SYSTEM_TOTAL]; // Viewer compiled data path
    path                m_GameDataPaths     [SYSTEM_TOTAL]; // Game compiled data path
    path                m_TextureSourcePaths[SYSTEM_TOTAL]; // Source of texture

    // Lighting    
    light               m_Light;                       // Scene light
                                                        
    // Background                                       
    char                m_BackPic[X_MAX_PATH];         // Picture
    xcolor              m_BackColor;                   // Background color
    xcolor              m_GridColor;                   // Grid color
                                
    // Config files            
    xarray<file>        m_ConfigFiles;                 // List of config files    
    xarray<file>        m_MissingConfigFiles;          // List of missing config files                                  

    // Screen shot
    path                m_ScreenShotPath;               // Screen shot folder

    // Objects                  
    xarray<object>      m_Objects;                     // List of objects

    // Fx    
    xarray<fx>          m_Fxs;                              // List of fx files

//==========================================================================
// FUNCTIONS
//==========================================================================

public:
    // Constructor/destructor
    config_options();
    ~config_options();

    // Sets up defaults
    void    Init    ( void );

    // Kills all data
    void    Kill    ( void );

    // Loads common config file - return TRUE if successful, else FALSE
    xbool   LoadCommon  ( const char* FileName );
    
    // Loads object config file - return TRUE if successful, else FALSE
    xbool   LoadObjectsFromConfig ( const char* FileName, xbool bFxFullPath );
    
    // Loads objects from game data directory
    xbool   LoadObjectsFromDirectory( const char* FileName );

    // Returns native viewer data path
    const char* GetViewerDataPath( void ) const;

    // Returns native game data path
    const char* GetGameDataPath  ( void ) const;
    
    // Adds fx if not already present
    void AddFx( const char* pFx, xbool bFullPath );
};

//==============================================================================
// DATA
//==============================================================================

extern config_options g_Config;

//==============================================================================
// HACKOTRON
//==============================================================================

#define CONFIG_IS_DEMO              0
#define CONFIG_IS_AUTOCAMPAIGN      0
#define CONFIG_IS_AUTOSPLITSCREEN   0
#define CONFIG_IS_AUTOSERVER        0
#define CONFIG_IS_AUTOCLIENT        0

//==============================================================================


#endif  //#ifndef __CONFIG_HPP__
