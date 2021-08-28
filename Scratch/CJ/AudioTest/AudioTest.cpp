#include "x_files.hpp"
#include "Entropy.hpp"
#include "io_mgr.hpp"
#include "x_log.hpp"

#ifdef TARGET_XBOX
    static
#else
    inline
#endif
void AppBegin(void*)
{
    s32 VoiceId=0;
    s32 nTries = 0;
    s32 PlayCount = 0;

    // Initialize Entropy + Audio + I/O
    eng_Init();
    g_AudioMgr.Init(0);
    g_IoMgr.Init();

    // Set the speaker configuration
    g_AudioMgr.SetSpeakerConfig( SPEAKERS_STEREO );

	g_AudioMgr.SetClip( 0, 10000 );

    ear_id EarID = g_AudioMgr.CreateEar();
    matrix4 m;
    m.Identity();
    vector3 Position( 10.0f, 0.0f, 10.0f );

    g_AudioMgr.SetEar( EarID, m, vector3(0,0,0), 0, 1.0f );

    // Load a test package
    g_AudioMgr.LoadPackage( "Weapon_DEagle.audiopkg" );
    g_AudioMgr.LoadPackage( "test.audiopkg" );

	g_AudioMgr.Play( "test_it" );

    // Loop through the tests
    s32 Index = 0;
    while( 1 )
    {
        // Simulate update of audio manager
        x_DelayThread( 33 );

        g_AudioMgr.Update( 0.033f );

        // Start a sound if one isn't playing
//        if( !g_AudioMgr.IsValidVoiceId( VoiceId ) )
//        {
//            VoiceId = g_AudioMgr.Play( "EGL_FIRE" );
//        }
//        else

/*
        {
            static f32 Time = 0.0f;
            static f32 SwitchTime = 1.0f;

            Time += 0.033f;

            if( Time > SwitchTime )
            {
                Time = 0.0f;
                g_AudioMgr.Release( VoiceId, 2.0f );
                VoiceId = g_AudioMgr.Play( "EGL_FIRE", Position, 0, TRUE );
            }
        }
*/
        // Flush the log
        LOG_FLUSH();
    }
}

void AppMain( s32 argc, char* argv[] )
{
    (void)argc;
    (void)argv;

    //  Need to formalize this somewhere in the headers
    #ifdef TARGET_XBOX
        #define STACK_SPACE 65536*4 // 256k for XBOX (fixes crash in smaug loader)
    #else
        #define STACK_SPACE 65536   // 64k everyone else
    #endif

    //  Create application thread and set appropriate stack
    #if defined(TARGET_XBOX) || defined(TARGET_GCN)
        xthread*pThread;
        pThread = new xthread(AppBegin,"App main test",STACK_SPACE,0);
        for( ;; )
        {
            x_DelayThread(1000);
        }
    #else
        AppBegin(NULL);
    #endif
}

void pc_PreResetCubeMap( void )
{
}

void pc_PostResetCubeMap( void )
{
}

struct s_AudioData
{
    char   Desc[64];
    f32    fVolume;
    f32    fNearClip;
    f32    fFarClip;
};

xbool AUDIO_TWEAK = FALSE;

s_AudioData *AudioTweakData;