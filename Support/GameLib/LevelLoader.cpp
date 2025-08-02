#include "Entropy.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Render\Render.hpp"
#include "Objects\Player.hpp"
#include "Objects\Corpse.hpp"
#include "Objects\LevelSettings.hpp"
#include "Objects\PlaySurface.hpp"
#include "Objects\Render\PostEffectMgr.hpp"
#include "Gamelib\Level.hpp"
#include "Gamelib\binLevel.hpp"
#include "Gamelib\Link.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "inputmgr\inputmgr.hpp"
#include "IOManager\io_mgr.hpp"
#include "Audio\audio_stream_mgr.hpp"
#include "Audio\audio_hardware.hpp"
#include "x_files\x_context.hpp"
#include "Auxiliary\Bitmap\aux_Bitmap.hpp"
#include "..\Support\TriggerEx\TriggerEx_Manager.hpp"
#include "..\Support\Tracers\TracerMgr.hpp"
#include "..\Support\Render\LightMgr.hpp"
#include "navigation\nav_map.hpp"
#include "navigation\ng_connection2.hpp"
#include "navigation\ng_node2.hpp"
#include "ZoneMgr\ZoneMgr.hpp"
#include "PlaySurfaceMgr\PlaySurfaceMgr.hpp"
#include "gamelib\StatsMgr.hpp" 
#include "objects\ParticleEmiter.hpp"
#include "Menu\DebugMenu2.hpp"
#include "Music_Mgr\Music_mgr.hpp"
#include "MusicStateMgr\MusicStateMgr.hpp"
#include "ConversationMgr\ConversationMgr.hpp"
#include "StringMgr\StringMgr.hpp"
#include "GameTextMgr\GameTextMgr.hpp"
#include "Objects\SpawnPoint.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "Templatemgr\TemplateMgr.hpp"
#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "Decals\DecalMgr.hpp"
#include "GameLib\RenderContext.hpp"
#include "DataVault\DataVault.hpp"
#include "TweakMgr\TweakMgr.hpp"
#include "PainMgr\PainMgr.hpp"
#include "PhysicsMgr\PhysicsMgr.hpp"
#include "Objects\AlienGlob.hpp"
#include "NetworkMgr\MsgMgr.hpp"
#include "Debris\debris_mgr.hpp"
#include "Audio\audio_voice_mgr.hpp"
#include "PerceptionMgr\PerceptionMgr.hpp"
#include "LevelLoader.hpp"
#include "UI\ui_manager.hpp"
#include "UI\ui_Font.hpp"
#include "StateMgr\StateMgr.hpp"
#include "MemCardMgr/MemCardMgr.hpp"
#include "Configuration/GameConfig.hpp"
#include "OccluderMgr/OccluderMgr.hpp"
#include "Dialogs/dlg_download.hpp"
#include "Dialogs/dlg_LoadGame.hpp"

#ifndef X_RETAIL
#include "InputMgr/Monkey.hpp"
#endif

#include "e_Memcard.hpp"
#include "StateMgr/maplist.hpp"
#include "NetworkMgr/Downloader/Archive.hpp"

//=============================================================================

level_loader  g_LevelLoader;
xbool         g_level_loading  = FALSE;
extern char   g_FullPath [ 256 ];
extern xtimer g_GameTimer;
#ifndef CONFIG_VIEWER
extern u32    g_nLogicFramesAfterLoad;
#endif

#ifndef X_RETAIL
void* g_pBallast = NULL;
#endif

static archive* s_pArchive = NULL;
//=============================================================================

level_loader::level_loader( void )
{
}

//=============================================================================

level_loader::~level_loader( void )
{
}

//=============================================================================

void level_loader::LoadInfo( const char* pPath )
{
    if ( pPath )
    {
        text_in InfoTextIn;
        InfoTextIn.OpenFile( pPath );

        while ( InfoTextIn.ReadHeader() )
        {
            if ( x_stricmp( InfoTextIn.GetHeaderName(), "Info" ) == 0 )
            {
                bbox WorldBBox;
                WorldBBox.Clear();
                InfoTextIn.ReadFields();
                InfoTextIn.GetBBox( "WorldBBox", WorldBBox );
                g_ObjMgr.SetSafeBBox( WorldBBox );
            }

            if ( x_stricmp( InfoTextIn.GetHeaderName(), "PlayerInfo" ) == 0 )
            {
                vector3 Position;
                radian  Pitch;
                radian  Yaw;
                s32     Zone;
                guid    Guid;
                InfoTextIn.ReadFields();
                InfoTextIn.GetVector3(  "Position", Position    );
                InfoTextIn.GetF32(      "Pitch",    Pitch       );
                InfoTextIn.GetF32(      "Yaw",      Yaw         );
                InfoTextIn.GetS32(      "Zone",     Zone        );
                InfoTextIn.GetGuid(     "PlayerGuid", Guid      );

                pGameLogic->SetPlayerSpawnInfo( Position, Pitch, Yaw, Zone, Guid );
            }
        }
        InfoTextIn.CloseFile();               
    }
}

//=============================================================================

void level_loader::LoadDFS( const char* pDFS )
{
    char* Ext[] = 
    {
        ".xbmp",
        ".rigidgeom",
        ".skingeom",
        ".anim",
        ".decalpkg",
        ".envmap",
        ".rigidcolor",
        ".stringbin",
        ".fxo",
        ".audiopkg",
        ".font"
    };

    s32 iFileSystem = g_IOFSMgr.GetFileSystemIndex( pDFS );
    s32 nFiles      = g_IOFSMgr.GetNFilesInFileSystem( iFileSystem );
    xtimer DeltaTime;

    DeltaTime.Start();

    for( s32 i=0; i<nFiles; i++ )
    {
        char FilePath[256];

        g_IOFSMgr.GetFileNameInFileSystem( iFileSystem, i, FilePath );

        char FExt[32];
        char FName[128];
        char RscName[128];
        x_splitpath(FilePath,NULL,NULL,FName,FExt);
        x_sprintf(RscName,"%s%s",FName,FExt);

        for( s32 j=0 ; (j<sizeof(Ext)/sizeof(char*)) ; j++ )
        {
            // Is it a supported type?
            if( x_stricmp( FExt, Ext[j] ) == 0 )
            {
                // Force resource mgr to load resource
                rhandle_base Handle;
                Handle.SetName( RscName );
                Handle.GetPointer();
                j = sizeof(Ext)/sizeof(char*);
            }
        }
    }
}

//=============================================================================

void RedirectTextureAllocator( void )
{
#ifdef TARGET_XBOX
    g_TextureFactory.GhostGeneralIntoTemp( true );
    extern void IncrementQHGuid(void);
    IncrementQHGuid();
#endif
}

//=============================================================================

void RestoreTextureAllocator( void )
{
#ifdef TARGET_XBOX
    g_TextureFactory.GhostGeneralIntoTemp( false );
    extern void IncrementQHGuid(void);
    extern void DeleteAllQHGuid(void);
    DeleteAllQHGuid(); // Kill all of the current guid
    IncrementQHGuid();
#endif
}

//=============================================================================

void level_loader::InitSlideShow( const char* pSlideShowScriptFile )
{
    // if we need to do a slide show for the campaign mode, handle that now
    if( g_StateMgr.GetState() == SM_SINGLE_PLAYER_LOAD_MISSION )
    {
        // grab a pointer to the load screen...we'll need that in a moment
        dlg_load_game* pLoadScreen = (dlg_load_game*)g_UiMgr->GetTopmostDialog( g_UiUserID );
        ASSERT( pLoadScreen );
        pLoadScreen->StartLoadingProcess();

        // load up the script and let the loading screen know about the
        // details of it
        text_in TextIn;
        TextIn.OpenFile( pSlideShowScriptFile );

        // load in the audio descriptor
        char AudioDescriptor[256];
        TextIn.ReadHeader();
        TextIn.ReadFields();
        TextIn.GetString( "descriptor", AudioDescriptor );

        // load in the text slide times
        f32 StartTextAnim;
        TextIn.ReadHeader();
        TextIn.ReadFields();
        TextIn.GetF32( "start_text_anim", StartTextAnim );
        pLoadScreen->SetTextAnimInfo( StartTextAnim );

        // load in the slide information
        TextIn.ReadHeader();
        s32 nSlides = TextIn.GetHeaderCount();

        RedirectTextureAllocator();
        {
            s32 i;
            pLoadScreen->SetNSlides( nSlides );
            for( i = 0; i < nSlides; i++ )
            {
                char   TextureName[256];
                f32    StartFadeIn;
                f32    EndFadeIn;
                f32    StartFadeOut;
                f32    EndFadeOut;
                xcolor SlideColor;
                
                TextIn.ReadFields();
                TextIn.GetString( "texture", TextureName );
                TextIn.GetF32( "start_fade_in", StartFadeIn );
                TextIn.GetF32( "end_fade_in", EndFadeIn );
                TextIn.GetF32( "start_fade_out", StartFadeOut );
                TextIn.GetF32( "end_fade_out", EndFadeOut );
                TextIn.GetColor( "slide_color", SlideColor );

                pLoadScreen->SetSlideInfo( i, TextureName, StartFadeIn, EndFadeIn, StartFadeOut, EndFadeOut, SlideColor );
            }
        }
        RestoreTextureAllocator();

        // make sure the loading audio package is loaded
        rhandle_base Handle;
        Handle.SetName( "DX_Loading.audiopkg" );
        Handle.GetPointer();
        m_VoiceID = 0;

        // kick off the audio
        g_AudioMgr.ReleaseAll();
        m_VoiceID = g_AudioMgr.Play( AudioDescriptor );

        global_settings& Settings = g_StateMgr.GetActiveSettings();
        g_AudioMgr.SetVoiceVolume( Settings.GetVolume( VOLUME_SPEECH ) / 100.0f );

        pLoadScreen->SetVoiceID( m_VoiceID );

        // NOTE FOR ROB: If I don't do this delay thread, the audio never
        // gets kicked off. Is it an IO priority issue, or something else?
        // NOTE FROM HAPGOOD: Because of the crappy Xbox scheduler, you
        // won't start new thread unless you yield the current context.
        // A sleep of 1 should suffice.
        g_AudioMgr.Update( 0.33f );
        x_DelayThread( 500 );
        g_AudioMgr.Update( 0.33f );

        // start the slide show
        pLoadScreen->StartSlideshow();
    }
    // start rendering the slide show or other loading screen in the background
    g_StateMgr.StartBackgroundRendering();
}

//=============================================================================

void level_loader::KillSlideShow( void )
{
    // Setting the state will cause us to get stuck in a while loop until
    // the text actually fades out. (Should only take a second.)
    g_StateMgr.StopBackgroundRendering();
}

//=============================================================================

void level_loader::LoadLevel( xbool bFullLoad )
{
    //g_MatchMgr.StartDelayedStatsRead();

    // On with the show .......................................................
    if (bFullLoad)
    {
        // Reset the static player member variable that indicates death state.
        // this is used by the state manager to determine if any pre-level
        // cinematics can be played.
        player::s_bPlayerDied = FALSE;
    }

    m_VoiceID   = 0;
    m_bFullLoad = bFullLoad;

    ASSERT( g_StateMgr.IsBackgroundThreadRunning() == FALSE );
    // Reset the perception mgr.
    g_PerceptionMgr.Init();

    g_level_loading = TRUE;

    MEMORY_OWNER( "LOADLEVEL" );
    char pPath  [ 256 ];
    char pPath2 [ 256 ];
    char pPath3 [ 256 ];
    slot_id SlotID;

    const map_entry* pMapEntry = g_MapList.Find( g_ActiveConfig.GetLevelID(), g_ActiveConfig.GetGameTypeID() );
    if( pMapEntry == NULL )
    {
        FetchManifest();
        pMapEntry = g_MapList.Find( g_ActiveConfig.GetLevelID(), g_ActiveConfig.GetGameTypeID() );
    }

    if( pMapEntry == NULL )
    {
        g_ActiveConfig.SetExitReason( GAME_EXIT_INVALID_MISSION );
        return;
    }

    GameMgr.SetZoneMinimum( pMapEntry->GetMinPlayers() );

    xfs LevelDFS( "levels/%s/level", g_ActiveConfig.GetLevelPath() );

#ifndef X_RETAIL
    if( bFullLoad )
    {
        extern s32 GetMemoryBallastForLevel( const char* pLevelName );
        g_pBallast = x_malloc( GetMemoryBallastForLevel( g_ActiveConfig.GetLevelPath() ) );
    }
#endif

    LOG_MESSAGE( "LoadLevel", "BEGIN! Level:%s, Memory Free:%d bytes",pMapEntry->GetDisplayName(),x_MemGetFree() );

    if( pMapEntry->GetFlags() & MF_DOWNLOAD_MAP )
    {
        // Ok, the map is on the memory card so we need to load it now. This WILL eventually have been 
        // pre-loaded by the front end as map loading really needs to have some sort of indication that
        // it is in progress. But for now, we just brute force load it and assume only the first memory 
        // card slot.
        if( LoadContent( *pMapEntry ) == FALSE )
        {
            g_ActiveConfig.SetExitReason( GAME_EXIT_INVALID_MISSION );
            return;
        }
    }
    else
    {
        // Mount the level.dfs file.
        g_IOFSMgr.MountFileSystem( (const char*)LevelDFS, 2 );
    }

    if( bFullLoad )
    {      
        // Initialize animation system
        // NOTE: This must be done BEFORE any data is loaded.
        anim_event::Init();

        // Multiplayer: hasa to be here because BeginSession is creating render targets
        // and clearing them. If another thread is active during the Clear() BOOM!
        render::BeginSession( g_NetworkMgr.GetLocalPlayerCount() );

        // Load the slide show script.
        InitSlideShow( xfs("%s\\%s", g_RscMgr.GetRootDirectory(), "SlideShowScript.txt") );

        // Open the load script.
        text_in TextIn;
        TextIn.OpenFile( xfs("%s\\%s", g_RscMgr.GetRootDirectory(), "LoadScript.txt") );
        TextIn.ReadHeader();

        // Execute the load script.
        s32  nCommands = TextIn.GetHeaderCount();
        xtimer DeltaTime;

        DeltaTime.Reset();
        DeltaTime.Start();

        for( s32 i=0; i < nCommands; i++ )
        {
            char command[256];
            char arguments[256];
            char hddargs[256];

            TextIn.ReadFields();
            TextIn.GetString( "command",   command );
            TextIn.GetString( "arguments", arguments );

            x_strcpy( hddargs, arguments );
            if( x_strncmp( (const char*)LevelDFS, "HDD:",4 ) == 0 )
            {
                x_sprintf( hddargs, "HDD:%s", arguments );
            }

            if( x_strcmp( command, "load_dfs" ) == 0 )
            {
                g_IOFSMgr.MountFileSystem( hddargs, 3 );
                LoadDFS( hddargs );
                g_IOFSMgr.UnmountFileSystem( hddargs );
            }
            else if( x_strcmp( command, "load_resource" ) == 0 )
            {
                rhandle_base Handle;
                Handle.SetName( arguments );
                Handle.GetPointer();
            }
            else if( x_strcmp( command, "mount_dfs" ) == 0 )
            {
                g_IOFSMgr.MountFileSystem( hddargs, 3 );
            }
            else if( x_strcmp( command, "unmount_dfs" ) == 0 )
            {
                g_IOFSMgr.UnmountFileSystem( hddargs );
            }

        }

        TextIn.CloseFile();

        g_DataVault.Init();
        LoadTweaks( g_FullPath );
        LoadPain( g_FullPath );

        // Create permanent objects
        g_ObjMgr.CreateObject("god") ;

        // Load the NavMap
        x_makepath( pPath, NULL, g_FullPath, "level_data", ".nmp" );
        g_NavMap.Load( pPath );

        // Load Globals Variables...
        x_makepath( pPath, NULL, g_FullPath, "level_data", ".glb" );
        {
            MEMORY_OWNER( "GLOBAL VARIABLE DATA" );
            g_VarMgr.LoadGlobals( pPath );
        }

        // Setup resource handles to rigid color table
        x_makepath( pPath, NULL, g_FullPath, "level_data", ".rigidcolor" );

        // Force the rigid color instance to be loaded prior to level init
        // since it requires a large allocation

        {
            rhandle_base Handle;
            Handle.SetName(pPath);
            Handle.GetPointer();
        }
    #if 0
        // Force the ordered files to load
        x_makepath( pPath, NULL, g_FullPath, "level_data", ".load" );
        ForceLoad(pPath);

        x_makepath( pPath, NULL, g_FullPath, "level_data", ".load_extra" );
        ForceLoad(pPath);
    #endif

        x_makepath( pPath, NULL, g_FullPath, "level_data", ".info" );
        LoadInfo( pPath );
    }

    // Create god, proxy play surface, load the globals.
    if( !bFullLoad )
    {
        g_ObjMgr.CreateObject("god");
        g_PlaySurfaceMgr.CreateProxyPlaySurfaceObject();

        // Load the NavMap
        x_makepath( pPath, NULL, g_FullPath, "level_data", ".nmp" );
        g_NavMap.Load( pPath );

        // Load Globals Variables...
        x_makepath( pPath, NULL, g_FullPath, "level_data", ".glb" );
        {
            MEMORY_OWNER( "GLOBAL VARIABLE DATA" );
            g_VarMgr.LoadGlobals( pPath );
        }
    }

    // Load the level
    x_makepath( pPath,      NULL, g_FullPath, "level_data", ".bin_level" );
    x_makepath( pPath2,     NULL, g_FullPath, "level_data", ".lev_dict" );
    x_makepath( pPath3,     NULL, g_FullPath, "level_data", ".load" );

    g_BinLevelMgr.LoadLevel( pPath, pPath2, pPath3 );

    if( bFullLoad )
    {
        {
            // load the rigid colors...they will be assigned to the
            // geometry in a moment
            rhandle<color_info> hRigidColor;
            x_makepath( pPath, NULL, g_FullPath, "level_data", ".rigidcolor" );
            hRigidColor.SetName( pPath );
            hRigidColor.GetPointer();
        }

        {   
            MEMORY_OWNER("TEMPLATE DATA");
            //load templates
            x_makepath( pPath, NULL, g_FullPath, "level_data", ".templates" );
            x_makepath( pPath2, NULL, g_FullPath, "level_data", ".tmpl_dct" );
            g_TemplateMgr.LoadData(pPath, pPath2);
        }

        {
            MEMORY_OWNER("ZONE DATA");
            //load portal/zone list
            x_makepath( pPath, NULL, g_FullPath, "level_data", ".zone" );
            g_ZoneMgr.Load(pPath);
        }

        {
            MEMORY_OWNER("PLAYSURFACE DATA");
            //load playsurfaces
            x_makepath( pPath, NULL, g_FullPath, "level_data", ".playsurface" );
            g_PlaySurfaceMgr.OpenFile(pPath, TRUE);
            g_PlaySurfaceMgr.LoadAllZones();
            g_PlaySurfaceMgr.CloseFile();
        }

        {
            MEMORY_OWNER("STATIC DECAL DATA");
            //load static decals
            x_makepath( pPath, NULL, g_FullPath, "level_data", ".decals" );
            g_DecalMgr.LoadStaticDecals( pPath );
        }
    }

    //initialize player tracker
    SlotID = g_ObjMgr.GetFirst( object::TYPE_PLAYER );

    while(SlotID != SLOT_NULL)
    {
        object_ptr<player> PlayerObj( g_ObjMgr.GetObjectBySlot( SlotID ) );

        if (PlayerObj.IsValid())
        {
            PlayerObj.m_pObject->InitZoneTracking();
        }

        SlotID = g_ObjMgr.GetNext( SlotID );
    }


    // Clear the polycache
    g_PolyCache.InvalidateAllCells();

    g_level_loading = FALSE;

#ifndef X_RETAIL
    // setup automonkey in non-retail builds
    g_Monkey.SetAutoMonkeyMode(g_Config.AutoMonkeyMode);
#endif

    // reset the rigid color pointers
    x_makepath( pPath, NULL, g_FullPath, "level_data", ".rigidcolor" );
    g_BinLevelMgr.SetRigidColor( pPath );

    // reset any screen fades we might've had on
    g_PostEffectMgr.StartScreenFade( xcolor(0,0,0,0), 0.0f );

    // reset the audio that may have been faded at some point
    g_AudioMgr.SetMasterVolume( 1.0f );

    MsgMgr.Init();
    g_MusicMgr.Init();

    // Setup OccluderMgr and search invis walls for occluders
    g_OccluderMgr.Init();
    g_OccluderMgr.GatherOccluders();

    // Reset DecalMgr
    g_DecalMgr.ResetDynamicDecals();

    // Setup the global game timer
    g_GameTimer.Reset();
    g_GameTimer.Start();

    // inflate the world bounds a bit
    g_ObjMgr.InflateSafeBBox( 1000.0f );

    #ifndef CONFIG_VIEWER
    // reset the frame count to 0
    g_nLogicFramesAfterLoad = 0;
	#endif

    if( bFullLoad )
    {
        KillSlideShow();
    }

    // Unmount the level.dfs file.
    if( pMapEntry->GetFlags() & MF_DOWNLOAD_MAP )
    {
        UnloadContent();
    }
    else
    {
        g_IOFSMgr.UnmountFileSystem( (const char*)LevelDFS );
    }
}

void level_loader::LoadLevelFinish( void )
{
#if defined(TARGET_PS2) && defined( TARGET_DEV )
    xtimer CorpseTimer ;
#endif;

    // kill the slideshow audio
    if( m_bFullLoad && g_AudioMgr.IsValidVoiceId( m_VoiceID ) )
    {
        g_AudioMgr.Release( m_VoiceID, 0.0f );
        g_AudioMgr.Update( 0.033f );
        x_DelayThread( 100 );
        g_AudioMgr.Update( 0.033f );
    }

    // Initialize ragdolls now that all geometry and collision is loaded
    // NOTE: This must be done AFTER the slide show has been killed because
    // that will render on a separate thread and both the collision code
    // and page flip will use scratchmem, which is NOT thread-safe.
#if defined(TARGET_PS2) && defined( TARGET_DEV )
    x_DebugMsg("\n\nInitializing %d dead bodies...\n", g_ObjMgr.GetNumInstances(object::TYPE_CORPSE)) ;
    CorpseTimer.Reset() ;
    CorpseTimer.Start() ;
#endif

    slot_id SlotID = g_ObjMgr.GetFirst(object::TYPE_CORPSE);
    while (SlotID != SLOT_NULL)
    {
        // Lookup corpse
        object* pObj    = g_ObjMgr.GetObjectBySlot(SlotID);
        corpse* pCorpse = &corpse::GetSafeType( *pObj );

        // Initialize
        pCorpse->InitializeEditorPlaced() ;

        // Get next corpse
        SlotID = g_ObjMgr.GetNext(SlotID);
    }

#if defined(TARGET_PS2) && defined(TARGET_DEV)
    CorpseTimer.Stop() ;
    x_DebugMsg("Took %f secs\n\n", CorpseTimer.ReadSec()) ;
#endif

    // initialize audio volumes from global settings
    global_settings& Settings = g_StateMgr.GetActiveSettings();
    Settings.CommitAudio();

    LOG_MESSAGE( "LoadLevel", "Done!" );

#ifdef TARGET_XBOX
__asm wbinvd
#endif
}

//=============================================================================

void level_loader::UnloadLevel( xbool bFullUnload )
{
    g_MusicStateMgr.Init();
    g_MusicMgr.Kill();

    #ifdef TARGET_XBOX
    {
        for( s32 i=0;i<2;i++ )
        {
            g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,0,0.0f,0 );
            g_pd3dDevice->Present( 0,0,0,0 );
        }

        g_pd3dDevice->BlockUntilIdle();
        g_pd3dDevice->SetStreamSource( 0,NULL,0 );
        g_pd3dDevice->SetStreamSource( 1,NULL,0 );

        for( s32 i=0;i<4;i++ )
            input_EnableFeedback( false,i );
    }
    #endif

    // The show must go on ....................................................

    LOG_MESSAGE( "level_loader::UnloadLevel", "Unload level started." );

    // Reset the perception mgr.
    g_PerceptionMgr.Init();

    xtimer t;

    t.Start();

    if( bFullUnload )
    {
        render::EndSession();

        UnloadTweaks();
        UnloadPain();

        g_DataVault.Kill();
    }
    else
    {
#ifndef X_EDITOR
        // clear all the network object pointers
        NetObjMgr.Clear();
#endif
        g_DecalMgr.ResetDynamicDecals();
        g_AudioMgr.ReleaseAll();
        g_VarMgr.ClearData(); // Clear the global variables.
    }

    // This should shutdown all systems that are inited in LoadLevel
    g_ObjMgr.Clear();
    FXMgr.EndOfFrame(); // Call FXMgr EndOfFrame to flush any deferred deletes - must be after ObjMgr.Clear
    FXMgr.EndOfFrame();
    FXMgr.EndOfFrame();
    FXMgr.EndOfFrame();

    if( bFullUnload )
    {
        g_DecalMgr.UnloadStaticDecals();
        g_DecalMgr.ResetDynamicDecals();
        g_PlaySurfaceMgr.Reset();
#ifdef RSC_MGR_COLLECT_STATS
        g_RscMgr.DumpStats();
#endif // RSC_MGR_COLLECT_STATS
        g_RscMgr.UnloadAll(TRUE);
#ifdef RSC_MGR_COLLECT_STATS
        g_RscMgr.DumpStats();
#endif // RSC_MGR_COLLECT_STATS
        g_AudioMgr.DisplayPackages();
        g_StringMgr.Reset();
        g_TemplateStringMgr.Reset();
        g_ZoneMgr.Reset();
        anim_event::ResetByteStreams();

        MsgMgr.Reset();
        g_BinLevelMgr.ClearData( TRUE );
        g_TemplateMgr.ClearData();
        g_VarMgr.ClearData();
        debris_mgr::ClearData();
        g_SpatialDBase.Clear();

        const char* LevelDFS = g_ActiveConfig.GetLevelPath();
        g_IOFSMgr.UnmountFileSystem( LevelDFS );

        // Notify checkpoint manager that the next checkpoint should be at a level start.
        g_CheckPointMgr.SetCheckPointIndex( 0 );
    }

    g_PolyCache.InvalidateAllCells();

    if( bFullUnload )
    {
        extern xstring g_SkinPropSurfaceStringList;
        extern xstring g_AlienSpawnTubeStringList;
        extern xstring g_AnimSurfaceStringList;
        extern xstring g_ReactiveSurfaceStringList;
        extern xstring g_SuperDestructibleStringList;
        extern xstring g_SuperDestructiblePlayAnimStringList;

        g_SkinPropSurfaceStringList.Clear();
        g_AlienSpawnTubeStringList.Clear();
        g_AnimSurfaceStringList.Clear();
        g_ReactiveSurfaceStringList.Clear();
        g_SuperDestructibleStringList.Clear();
        g_SuperDestructiblePlayAnimStringList.Clear();

        g_SkinPropSurfaceStringList.FreeExtra();
        g_AlienSpawnTubeStringList.FreeExtra();
        g_AnimSurfaceStringList.FreeExtra();
        g_ReactiveSurfaceStringList.FreeExtra();
        g_SuperDestructibleStringList.FreeExtra();
        g_SuperDestructiblePlayAnimStringList.FreeExtra();

        // This is last because it does an x_free followed by x_malloc
        g_NavMap.Reset();

#ifndef X_RETAIL
        if( g_pBallast )
        {
            x_free( g_pBallast );
            g_pBallast = NULL;
        }
#endif
    }
    else
    {
        g_NavMap.Reset();
    }

    // Clear OccluderMgr, even on partial unload
    g_OccluderMgr.Kill();

    // Move the identifier tables down low in memory.
    g_AudioMgr.ReMergeIdentifierTables();

#if defined(TARGET_XBOX) || defined(TARGET_PS2)
    // If its an online game, then save out the stats.
    if( GameMgr.IsGameOnline() )
    {
        // This is the last chance the server has to update his stats for submission.
        if( g_NetworkMgr.IsServer() )
        {
            GameMgr.UpdatePlayerStats();
        }
        
        // Set the match managers stats.
        g_MatchMgr.SetAllGameStats( GameMgr.GetPlayerStats() );

        // Reset stats so they don't get sent again on clients.
        GameMgr.ResetPlayerStats();

        #ifdef TARGET_XBOX
        g_MatchMgr.PostGameStatsToLive();
        #else
        g_MatchMgr.UpdateCareerStatsWithGameStats();
        g_MatchMgr.InitiateCareerStatsWrite();
        #endif // TARGET_XBOX
    }
#endif // defined(TARGET_XBOX) || defined(TARGET_PS2)

    LOG_MESSAGE( "level_loader::UnloadLevel", "Unload complete. Memory Free:%d bytes, took %2.02fms",x_MemGetFree(), t.ReadMs() );
    x_DebugMsg("Unload level tool %2.02fms", t.ReadMs() );
}

//=============================================================================
// Please make sure that you add the unmount call if you add another filesystem
// to the list. This will break the IOP reboot if it is not performed.

void level_loader::MountDefaultFilesystems( void )
{
    g_IOFSMgr.MountFileSystem( "BOOT",            1 ); 
    g_IOFSMgr.MountFileSystem( "PRELOAD",         1 ); 
    g_IOFSMgr.MountFileSystem( "AUDIO\\MUSIC",   10 ); 
    g_IOFSMgr.MountFileSystem( "AUDIO\\VOICE",   11 ); 
    g_IOFSMgr.MountFileSystem( "AUDIO\\HOT",     12 );
    g_IOFSMgr.MountFileSystem( "AUDIO\\AMBIENT", 12 );
    g_IOFSMgr.MountFileSystem( "STRINGS",        13 );
    g_IOFSMgr.MountFileSystem( "COMMON",         14 );
}

//=============================================================================

void level_loader::UnmountDefaultFilesystems( void )
{
    g_IOFSMgr.UnmountFileSystem( "COMMON" );
    g_IOFSMgr.UnmountFileSystem( "STRINGS" );
    g_IOFSMgr.UnmountFileSystem( "AUDIO\\AMBIENT" );
    g_IOFSMgr.UnmountFileSystem( "AUDIO\\HOT" );
    g_IOFSMgr.UnmountFileSystem( "AUDIO\\VOICE" );
    g_IOFSMgr.UnmountFileSystem( "AUDIO\\MUSIC" );
    g_IOFSMgr.UnmountFileSystem( "PRELOAD" );
    g_IOFSMgr.UnmountFileSystem( "BOOT" );
}


//=========================================================================
void level_loader::OnPollReturn( void )
{
    s32 card;
    s32 i;
    g_MapList.RemoveByFlags( MF_DOWNLOAD_MAP );
    for( card=0; card<2; card++ )
    {
        map_list& Manifest = g_UIMemCardMgr.GetManifest(card);
        for( i=0; i< Manifest.GetCount(); i++ )
        {
            g_MapList.Append( Manifest[i], &Manifest );
        }
    }
}

//=========================================================================
void level_loader::FetchManifest( void )
{

    g_UIMemCardMgr.PollContent( TRUE, this, &level_loader::OnPollReturn );
    g_StateMgr.StartBackgroundRendering();
    do 
    {
        x_DelayThread(32);
    } while( g_UIMemCardMgr.IsActionDone() == FALSE );
    g_StateMgr.StopBackgroundRendering();
}

//=============================================================================
void level_loader::LoadContentComplete( void )
{
    m_LoadInProgress = FALSE;
}

//=============================================================================
xbool level_loader::LoadContent( const map_entry& MapEntry )
{
#if defined(TARGET_PS2)
    s32     i;

    m_LoadInProgress = TRUE;
    g_UIMemCardMgr.LoadContent( MapEntry, this, &level_loader::LoadContentComplete );
    g_StateMgr.StartBackgroundRendering();
    //
    // Note: We can whirl around in this loop as the background renderer should currently
    // be running and that will deal with updating the display and slideshow.
    while( m_LoadInProgress && (g_ActiveConfig.GetExitReason() == GAME_EXIT_CONTINUE) )
    {
        x_DelayThread( 32 );
    }
    g_StateMgr.StopBackgroundRendering();

    s32 Checksum;
    if( g_ActiveConfig.GetExitReason() != GAME_EXIT_CONTINUE )
    {
        return FALSE;
    }

    Checksum = x_chksum( g_UIMemCardMgr.GetBuffer(), (g_UIMemCardMgr.GetBufferLength() &~3) );
    g_ActiveConfig.SetLevelChecksum( Checksum );

    ASSERT( s_pArchive == NULL );
    s_pArchive = new archive;

    x_MemSanity();
    ASSERT( s_pArchive );
    s_pArchive->Init( (byte*)g_UIMemCardMgr.GetBuffer(), g_UIMemCardMgr.GetBufferLength() );

    x_MemSanity();
    for( i=0; i<s_pArchive->GetMemberCount(); i++ )
    {
        LOG_MESSAGE( "PerformMemoryCardLoad", "Archive member %s, length:%d", s_pArchive->GetMemberFilename(i), s_pArchive->GetMemberLength(i) );
    }
    LOG_FLUSH();

    x_MemSanity();
    g_UIMemCardMgr.FreeBuffer();
    x_MemSanity();
    // Should always have an even number of archive members.
    ASSERT( (s_pArchive->GetMemberCount() & 1)==0 );
    s32 DfsCount = s_pArchive->GetMemberCount() / 2;
    for( i=0; i<DfsCount; i++ )
    {
        // Make sure we have the files in the order we expect. The first should be the .dfs file, the
        // second should have the .000 extension.
        ASSERT( x_stristr( s_pArchive->GetMemberFilename(i*2  ), ".dfs" ) != NULL );
        ASSERT( x_stristr( s_pArchive->GetMemberFilename(i*2+1), ".000" ) != NULL );
        xbool Result;

        Result = g_IOFSMgr.MountFileSystemRAM( s_pArchive->GetMemberFilename(i*2), (void*)s_pArchive->GetMemberData(i*2), (void*)s_pArchive->GetMemberData(i*2+1) );
        ASSERT( Result );
    }
    x_MemSanity();
    return TRUE;
#else
    (void)MapEntry;
    return FALSE;
#endif
}

//=============================================================================
void level_loader::UnloadContent( void )
{
    s32 DfsCount;
    s32 i;

    ASSERT( s_pArchive );
    DfsCount = s_pArchive->GetMemberCount() / 2;
    x_MemSanity();

    for( i=0; i<DfsCount; i++ )
    {
        xbool Result;

        Result = g_IOFSMgr.UnmountFileSystem( s_pArchive->GetMemberFilename(i*2) );
        x_MemSanity();
        ASSERT( Result );
    }
    x_MemSanity();
    s_pArchive->Kill();
    x_MemSanity();
    delete s_pArchive;
    x_MemSanity();
    s_pArchive = NULL;
}
