//=============================================================================
//
//  Audio Manager Stress Test
//
//=============================================================================

#include "Entropy.hpp"
#include "Gamelib\Link.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "Entropy\Audio\io_mgr.hpp"

//=============================================================================
//  CONSTANTS
//=============================================================================

#define RELEASE_PATH            "C:\\GameData\\A51\\Release"
#define CONFIG_FILENAME         "config.txt"
#define AUTOLOAD                0                               // Skip level selector
#define ENABLE_PROJTEXTURES     0                               // Enable Projected Textures

//=============================================================================

//=============================================================================
//  GLOBALS
//=============================================================================

char        g_FullPath [ 256 ];
xbool       g_Exit      = FALSE;
xtimer      g_GameTimer;
s32         MusicID[4];
s32         SfxID[48];

//=============================================================================

void UpdateAudio( f32 DeltaTime )
{    
    static s32 id;
    g_AudioManager.Update( DeltaTime );
    
#if 0
    for( s32 i = 0; i < 2; i++ )
    {
        if( !(g_AudioManager.IsPlaying( MusicID[i] ) || g_AudioManager.IsStarting( MusicID[i] )) )
        {                
            MusicID[i] = g_AudioManager.Play( "Music_Demo" );
            x_DebugMsg("New music started, id %d\n",MusicID[i]);
        }    
    }
#endif
    
    if (!g_AudioManager.IsPlaying(id))
    {
        id = g_AudioManager.Play( "StereoMusic"  );
//        id = g_AudioManager.Play( "MonoMusic"  );
    }
#if 0
    for( s32 i = 0; i < 1; i++ )
    {
        if( !(g_AudioManager.IsPlaying( SfxID[i] ) || g_AudioManager.IsStarting( SfxID[i] )) )
        {                
            vector3 Pos( 0.0f, 0.0f, 0.0f );
            
            switch( x_irand( 1, 3 ) )
            {
                case 1:
                    Pos.X = x_frand(-1500.0f, 1500.0f);
                break;
                case 2:
                    Pos.Y = x_frand(-1500.0f, 1500.0f);
                break;
                case 3:
                    Pos.Z = x_frand(-1500.0f, 1500.0f);
                break;
                default:
                    ASSERT( 0 );
                break;
            }
            
            char  SoundName[64];
            
            switch( x_irand( 1, 4 ) )
            {
                // Simple
                case 1:
                    x_sprintf( (char*)SoundName, "Run" );
                break;
                // Random.
                case 2:
                    x_sprintf( (char*)SoundName, "Attack" );
                break;
                // Complex.
                case 3:
                    x_sprintf( (char*)SoundName, "Descriptor3" );
                break;
                // Weighted.
                case 4:
                    x_sprintf( (char*)SoundName, "AttackSwooshA06" );
                break;
                default:
                    ASSERT( 0 );
                break;
            }
            SfxID[i] = g_AudioManager.Play( (const char*)SoundName, Pos  );
        }    
    }
#endif
}

//=============================================================================
//
//  Game Update Function
//
//=============================================================================

void Update( f32 DeltaTime )
{

    UpdateAudio( DeltaTime );
}


//=============================================================================

void LoadAudioPackage( void )
{
    VERIFY(g_AudioManager.LoadPackage("NewTest.audiopkg"));
}

//=============================================================================
//  MAIN
//=============================================================================

void AppMain( s32 argc, char* argv[] )
{
    (void)argc;
    (void)argv;

    // Initialize general systems
    x_Init();
    x_DebugMsg("Entered app\n");
    eng_Init();
    
    x_DebugMsg( "Initialize audio system\n" );
    g_AudioManager.Init();

    x_DebugMsg( "Initialize io system\n" );
    g_IoMgr.Init();

    // Init stats
    // Setup the full path to the platform specific release data
    x_strcpy( g_FullPath, xfs( "%s\\PS2", RELEASE_PATH ) );

    x_DebugMsg( "Starting to initialize resource manager\n" );

    // Setup the global game timer
    g_GameTimer.Reset();
    g_GameTimer.Start();

    LoadAudioPackage();
    
    for( s32 i = 0; i < 4; i++ )
        MusicID[i] = 0;    

    for( s32 i = 0; i < 48; i++ )
        SfxID[i] = 0;    

    //
    // Main Loop
    //

    while( g_Exit == FALSE )
    {
        
        // Compute the duration of the last frame
        f32 DeltaTime = MAX( g_GameTimer.ReadSec(), 0.001f );

		g_GameTimer.Reset();
		g_GameTimer.Start();
                
        Update( DeltaTime );

    }


    g_AudioManager.UnloadAllPackages();
    g_AudioManager.Kill();
    g_IoMgr.Kill();
}

//=============================================================================

