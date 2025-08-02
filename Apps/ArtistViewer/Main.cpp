//==============================================================================
//
//  File:           ArtistViewer.cpp
//
//  Description:    Simple multi-platform rigid/skin viewer for A51
//
//  Author:         Stephen Broumley
//
//  Date:           Started July, 2003 (C) Inevitable Entertainment Inc.
//
//
//
//==============================================================================


//==============================================================================
//  INCLUDES
//==============================================================================
#include "Entropy.hpp"
#include "Auxiliary\Bitmap\aux_Bitmap.hpp"
#include "Config.hpp"
#include "ViewerObject.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "Music_Mgr\Music_Mgr.hpp"
#include "ConversationMgr\ConversationMgr.hpp"
#include "IOManager\io_mgr.hpp"
#include "Render\LightMgr.hpp"
#include "Objects\Render\PostEffectMgr.hpp"
#include "Objects\Player.hpp"
#include "PlaySurfaceMgr\PlaySurfaceMgr.hpp"
#include "Controls.hpp"
#include "Util.hpp"
#include "Main.hpp"
#include "PhysicsMgr\PhysicsMgr.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "EventMgr\EventMgr.hpp"
#include "TweakMgr\TweakMgr.hpp"
#include "Objects\ParticleEmiter.hpp"

#ifdef TARGET_PS2
#include <sifdev.h>         // For sceOpen
#include "PS2\PS2_Misc.hpp"
#endif


//==============================================================================
//  DEFINES
//==============================================================================

#define CONFIG_LINES    12  // # of config lines to display



// Edit mode strings
const char* pModeStrings[] =
{
    "Object",
    "Light",
    "ScreenShot",
    "FX",
} ;

// Pic mode strings
const char* pPicModeStrings[] =
{
    "Top-left",
    "Center",
    "Fit-to-screen",
} ;

//==============================================================================
//  DATA
//==============================================================================

// Render
view                        g_View ;                        // Main view
xbitmap                     g_PicBitmap ;                   // Background pic
s32                         g_PicWidth = 0 ;                // Actual width
s32                         g_PicHeight = 0 ;               // Actual height
pic_status                  g_PicStatus = PIC_STATUS_NONE ; // Pic status
xbool                       g_bMovieShotActive = FALSE ;    // TRUE if active
                                                        
// Object                                               
viewer_object*              g_pObjects = NULL ;             // List of pointers to objects
s32                         g_nObjects = 0 ;                // # of objects
s32                         g_iObject  = 0 ;                // Active object to render/advance
s32                         g_iLoadedObject = -1 ;          // Index of loaded object in memory
player*                     g_pPlayer  = NULL;              // Ptr to dummy player

// Fx
s32                         g_iFx = 0;                      // Current fx
view                        g_FxView;                       // View for particle fx

// Physics
physics_inst                g_PhysicsInst[MAX_PHYSICS_INSTS];   // List of physics instances
s32                         g_iPhysicsInst = 0;                 // Next physics instance
                                                                        
// Editables
s32                         g_iConfig = 0 ;                 // Index of current config file
s32                         g_iConfigStart = -1 ;           // Start index of config list (for scrolling)
s32                         g_Mode    = MODE_OBJECT ;       // Edit mode
xbool                       g_bPause  = FALSE ;             // Pause flag
s32                         g_ShowHelp  = 1 ;               // Shows help
stats_mode                  g_StatsMode = STATS_MODE_HIGH ; // Shows stats
config_options::light               g_Light ;                       // Current light
s32                         g_PicMode = PIC_MODE_DEFAULT ;  // Pic display mode


// Here because the PS2 gamelib needs them!
#if defined(TARGET_PC)
#define PLATFORM_PATH   "PC"
#elif defined(TARGET_PS2)
#define PLATFORM_PATH   "PS2"
#elif defined(TARGET_XBOX)
#define PLATFORM_PATH   "XBOX"
#else
#error Target not defined!
#endif
#define RELEASE_PATH            "C:\\GameData\\A51\\Release"

xbool       g_GameLogicDebug     = FALSE ;
xbool       g_game_running       = TRUE ;
s32         g_Changelist         = -1;
const char* g_pBuildDate         = "Hello!";
xbool       g_first_person       = FALSE ;
xbool       g_InvertYAxis        = FALSE ;
xbool       g_MirrorWeapon       = FALSE;
xbool       g_UnlimitedAmmo      = TRUE ;
xbool       g_LevelLoadRequested = FALSE ;
xbool       g_Pause              = FALSE ;
char        g_LevelToLoad[ 256 ] = { 0 };
char        g_FullPath [ 256 ]   = { 0 };
xbool       g_bInsideRTF        = FALSE;
xbool       g_FreeCam           = FALSE;

xbool       g_RenderBoneBBoxes  = FALSE;

xbool       SHOW_STREAM_INFO = FALSE;
xbool       SHOW_AUDIO_LEVELS = FALSE;
xbool       SHOW_AUDIO_CHANNELS = FALSE;

xbool       g_AimAssist_Render_Reticle      = FALSE;
xbool       g_AimAssist_Render_Bullet       = FALSE;
xbool       g_AimAssist_Render_Turn         = FALSE;
xbool       g_AimAssist_Render_Bullet_Angle = FALSE;
xbool       g_AimAssist_Render_Player_Pills = FALSE;

s32             g_Difficulty = 1;  // Start out on Medium difficulty
const char* DifficultyText[] = { "Easy", "Medium", "Hard" };
xtimer      g_GameTimer;
s32         g_nFramesAfterLoad = 0;

xbool       g_bBloodEnabled    = TRUE;
xbool       g_bRagdollsEnabled = TRUE;

s32         g_MemoryLowWater = 0;
xbool       g_bControllerCheck = FALSE;


// Gadgets array
struct gadget
{
    char m_Name[256] ;
} ;
gadget g_InputGadgets[INPUT_KBD__END+1] ;

// Timers
xtimer  g_LogicCPU ;
xtimer  g_RenderCPU ;

// Audio ears
s32 g_EarID = 0;

// Functions
void LoadLevel( void )
{
}

void AudioStats( f32 DeltaTime )
{
    (void)DeltaTime;
}


//==============================================================================
//  FORWARD DECLARATIONS
//==============================================================================

// App functions
void AppAdvance( f32 DeltaTime ) ;
void AppReset( void );

// Retrieves stats for PS2
#ifdef TARGET_PS2
void    eng_GetStats(s32 &Count, f32 &CPU, f32 &GS, f32 &INT, f32& VBL, f32 &FPS) ;
#endif

// Returns TRUE if file exists
xbool DoesFileExist( const char* pName ) ;

// Loads common.cfg file
void LoadCommonConfig( void ) ;



//==============================================================================
//  LIGHTING FUNCTIONS
//==============================================================================

xcolor ComputeAmbient( const vector3& Pos )
{
    // Compute ambient from light
    xcolor Ambient ;
    if (g_Light.m_State == config_options::light::STATE_OFF)
        Ambient.Set( XCOLOR_BLACK );
    else
    if (g_Light.m_State == config_options::light::STATE_FULL_BRIGHT)
        Ambient.Set( 128, 128, 128, 255 );
    else
    {
        // Compute dist to light
        vector3 Delta = g_Light.m_Position - Pos;
        f32     Dist  = Delta.Length() ;

        // Compute attenuation
        f32 Atten = 1.0f - x_min(1.0f, Dist / g_Light.m_Radius) ;

        // Compute ambient color
        f32 R = x_min(255.0f, Atten * g_Light.m_Ambient.R) ;
        f32 G = x_min(255.0f, Atten * g_Light.m_Ambient.G) ;
        f32 B = x_min(255.0f, Atten * g_Light.m_Ambient.B) ;
        f32 A = x_min(255.0f, Atten * g_Light.m_Ambient.A) ;
        Ambient.R = (u8)R ;
        Ambient.G = (u8)G ;
        Ambient.B = (u8)B ;
        Ambient.A = (u8)A ;
    }
    
    return Ambient;
}


//==============================================================================
//  PHYSICS FUNCTIONS
//==============================================================================

void ClearPhysicsInsts( void )
{   
    // Loop through all physics instances and clear
    for( s32 i = 0; i < MAX_PHYSICS_INSTS; i++ )
        g_PhysicsInst[i].Kill();
}

//==============================================================================

void AddPhysicsInsts( const char*               pGeomName,
                      u64                       Mask,
                      loco_char_anim_player&    AnimPlayer,
                      const vector3&            Vel,
                      xbool                     bClearAll, 
                      xbool                     bBlast )
{
    // Clear all current instances?
    if( bClearAll )
        ClearPhysicsInsts();
    
    // Get next physics inst to initialize
    physics_inst& PhysicsInst = g_PhysicsInst[ g_iPhysicsInst ];
    if( ++g_iPhysicsInst == MAX_PHYSICS_INSTS )
        g_iPhysicsInst = 0;
    
    // Initialize physics instance
    PhysicsInst.Init( pGeomName, TRUE );
    PhysicsInst.SetMatrices( AnimPlayer, Vel );

    // Setup skin instance to render correctly    
    skin_inst& SkinInst = PhysicsInst.GetSkinInst();
    SkinInst.SetMinAmbient( XCOLOR_BLACK );
    SkinInst.SetOtherAmbientAmount( 1.0f );
    
    // Setup vmesh mask
    u32 VMeshMask = 0;
    geom* pGeom = SkinInst.GetGeom();
    if( pGeom )
    {
        for( s32 i = 0; i < 64; i++ )
        {
            if( Mask & ( 1 << i ) )
            {
                const char* pMesh = pGeom->GetMeshName( i );
                s32 iVMesh = pGeom->GetVMeshIndex( pMesh );
                VMeshMask |= 1 << iVMesh;
            }
        }
    }
    SkinInst.SetVMeshMask( VMeshMask );

    // Apply blast?
    if( bBlast )
    {
        // Loop through all rigid bodies
        for( s32 i = 0; i < PhysicsInst.GetNRigidBodies(); i++ )    
        {
            // Lookup body
            rigid_body& RigidBody = PhysicsInst.GetRigidBody( i );    
            
            // Add random velocity upwards
            RigidBody.GetLinearVelocity() +=  vector3( x_frand( -100.0f, 100.0f ),
                                                       x_frand( 600.0f, 900.0f ),
                                                       x_frand( -100.0f, 100.0f ) );
        }
    }
}

//==============================================================================

void RenderPhysicsInsts( void )
{
    // Render all instances
    for( s32 i = 0; i < MAX_PHYSICS_INSTS; i++ )
    {
        // Lookup instance
        physics_inst& PhysicsInst = g_PhysicsInst[i];
        
        // Compute ambient
        xcolor Ambient = ComputeAmbient( PhysicsInst.GetPosition() );
        
        // Render
        PhysicsInst.Render( render::CLIPPED, Ambient );
    }        
}


//==============================================================================
//  FUNCTIONS
//==============================================================================

// Prints asserts to the screen and continues
xbool ViewerRTF( const char* pFileName,
                 s32         LineNumber,
                 const char* pExprString,
                 const char* pMessageString )
{
    // Output to debugger
    x_DebugMsg("\n\nFile:%s\n", pFileName) ;
    x_DebugMsg("Line:%d\n", LineNumber) ;
    x_DebugMsg("Expr:%s\n", pExprString) ;

    // Shot info
    x_printf("\n\nFile:%s\n", pFileName) ;
    x_printf("Line:%d\n", LineNumber) ;
    x_printf("Expr:%s\n", pExprString) ;
    if (pMessageString)
        x_printf("Mesg:%s\n", pMessageString) ;

    // Continue
    return FALSE ;
}

//==============================================================================

// Thse functions aren't in the PC engine so here they are doing nothing...
#ifdef TARGET_PC

void eng_ScreenShot( const char* pFileName /*= NULL */, s32 Size /*= 1 */ )
{
    (void)pFileName ;
    (void)Size ;
}

//==============================================================================

s32 eng_ScreenShotSize( void )
{
    return 1 ;
}

//==============================================================================

s32 eng_ScreenShotX( void )
{
    return 1 ;
}

//==============================================================================

s32 eng_ScreenShotY( void )
{
    return 1 ;
}

#endif

//==============================================================================

// Display "select config" screen
void SelectConfig( void )
{
    x_DebugMsg("SelectConfig\n") ;

    // Skip first update
    input_UpdateState() ;

    // Keep going until we choose something
    while(1)
    {
        // Use black background
        eng_SetBackColor(XCOLOR_BLACK) ;

        // Show build info
#ifdef TARGET_PC
        x_printfxy(0,0, "A51 VIEWER BUILT %s", __TIMESTAMP__) ;
#else
        x_printfxy(0,0, "A51 VIEWER BUILT %s %s", __DATE__, __TIME__) ;
#endif

#if defined( CONFIG_VIEWER )
        x_printfxy(0,1, "(viewer build)") ;
#elif defined( CONFIG_DEBUG )
        x_printfxy(0,1, "(debug build)") ;
#elif defined( CONFIG_OPTDEBUG )
        x_printfxy(0,1, "(optdebug build)") ;
#else
#error add new config here!
#endif

        // Show memory info
        s32 Free, Largest, Fragments ;
        x_MemGetFree(Free, Largest, Fragments) ;
        x_printfxy(0,23,"Free:%.2fK Lrg:%.2fK Frags:%d", 
                  (f32)Free    / 1024,
                  (f32)Largest / 1024,
                  Fragments) ;

        // Show the options
        s32 x=1 ;
        s32 y=3 ;
        if (g_Config.m_ConfigFiles.GetCount())
        {
            x_printfxy(x,y, "Press X to select config file") ; 
            y += 3 ;
        }
        else
        {
            // Nothing to select
            x_printfxy(x,y++, "No config files specified") ;
            x_printfxy(x,y++, "Please update common.cfg") ;
            y += 2 ;
        }

        // Show config files and cursor
        x = 0 ;

        // Scroll up?
        while( g_iConfig < g_iConfigStart )                    
            g_iConfigStart--;

        // Scroll down?
        while( g_iConfig >= ( g_iConfigStart + CONFIG_LINES ) )
            g_iConfigStart++;

        // Show scroll up?
        if( g_iConfigStart != -1 )
            x_printfxy( x+11, y-1, "<<--- more --->>" );
        
        // Show config files
        s32 i;
        for( i = g_iConfigStart ; ( i < g_Config.m_ConfigFiles.GetCount() ) && ( i < ( g_iConfigStart + CONFIG_LINES ) ) ; i++)
        {
            // Show cursor?
            if( i == g_iConfig )
            {
                x_printfxy( x,    y, ">" );
                x_printfxy( x+37, y, "<" );
            }

            // Show release dir or config?
            if( i == -1 )
            {
                x_printfxy(x+2,y++,"%s*.*", g_Config.GetGameDataPath()) ; 
            }
            else
            {                        
                x_printfxy(x+2,y++, "%s", g_Config.m_ConfigFiles[i].m_Name) ;
            }                
        }
        
        // Show scroll down?
        if( i != g_Config.m_ConfigFiles.GetCount() )
            x_printfxy( x+11, y, "<<--- more --->>" );
        
        // Show error list?
        if (g_Config.m_MissingConfigFiles.GetCount())
        {
            x = 1 ;
            y += 2;
            x_printfxy(x,y++, "ERROR! - Config files not found!") ;
            x_printfxy(x,y++, "       - Please fix common.cfg") ; y += 1 ;
            for (s32 i = 0 ; i < g_Config.m_MissingConfigFiles.GetCount() ; i++)
                x_printfxy(x+2,y+i, "%s", g_Config.m_MissingConfigFiles[i].m_Name) ;
        }

        // Process input
        if (input_UpdateState())
        {
            // Move cursor up?
            if (        (input_WasPressed(JOY_CFG_SEL_UP))
                    ||  (input_WasPressed(KEY_CFG_SEL_UP)) )
            {
                g_iConfig-- ;
                if (g_iConfig < -1)
                    g_iConfig = g_Config.m_ConfigFiles.GetCount() - 1 ;
            }

            // Move cursor down?
            if (        (input_WasPressed(JOY_CFG_SEL_DOWN))
                    ||  (input_WasPressed(KEY_CFG_SEL_DOWN)) )
            {
                g_iConfig++ ;
                if (g_iConfig >= g_Config.m_ConfigFiles.GetCount())
                    g_iConfig = -1 ;
            }

            // Reload common.cfg?
            if (        (input_WasPressed(JOY_MAIN_RELOAD))
                    ||  (input_WasPressed(KEY_MAIN_RELOAD)) )
            {
                LoadCommonConfig() ;
            }

            // Done?
            if (        (input_WasPressed(JOY_CFG_SEL_SELECT))
                    ||  (input_WasPressed(KEY_CFG_SEL_SELECT)) )
                break ;

            // Quit?
            if (        (input_WasPressed(JOY_CFG_SEL_QUIT))
                    ||  (input_WasPressed(KEY_CFG_SEL_QUIT)) )
                break ;
        }

        // Toggle screen
        eng_PageFlip() ;
    }
}

//==============================================================================

// Resets lighting
void ResetLight( void )
{
    // Reset to config light
    g_Light = g_Config.m_Light ;

    // Re-light all objects
    for (s32 i = 0 ; i < g_nObjects ; i++)
        g_pObjects[i].Light() ;
}

//==============================================================================

// Returns TRUE if bitmap dimension is valid (power of 2 and less than 512)
xbool IsBitmapDimensionValid( s32 V )
{
    // Too big?
    if (V > 512)
        return FALSE ;

    // Check all values
    for (s32 i = 0 ; i < 32 ; i++)
    {
        // Power of 2 found?
        if (V == (1<<i))
            return TRUE ;
    }

    return FALSE ;
}

//==============================================================================

// Loads common.cfg file
void LoadCommonConfig( void )
{
    // Keep current copy of config settings to detect changes
    s32           ShowHelp  = g_Config.m_ShowHelp ;
    s32           ShowStats = g_Config.m_ShowStats ;
    config_options::light Light     = g_Config.m_Light ;

    // Reload common config
    ClearPhysicsInsts();
    g_Config.LoadCommon("Common.cfg") ;

    // Range check config
    if (g_Config.m_ConfigFiles.GetCount())
    {
        // Range check config?
        if (g_iConfig != -1)
        {
            // Range check config
            if (g_iConfig < 0)
                g_iConfig = 0 ;
            else
            if (g_iConfig >= g_Config.m_ConfigFiles.GetCount())
                g_iConfig = g_Config.m_ConfigFiles.GetCount()-1 ;
        }

        // Range check object
        if (g_iObject < 0)
            g_iObject = 0 ;
        else
        if (g_iObject >= g_Config.m_Objects.GetCount())
            g_iObject = g_Config.m_Objects.GetCount()-1 ;
    }
    else
    {
        // No config
        g_iConfig = -1 ;
        g_iObject = -1 ;
    }

    // Update help?
    if (g_Config.m_ShowHelp != ShowHelp)
        g_ShowHelp = g_Config.m_ShowHelp ;
    
    // Update stats?
    if (g_Config.m_ShowStats != ShowStats)
        g_StatsMode = (stats_mode)g_Config.m_ShowStats ;

    // Update light?
    if (g_Config.m_Light.m_Ambient != Light.m_Ambient)
        g_Light.m_Ambient = g_Config.m_Light.m_Ambient ;
    if (g_Config.m_Light.m_Color != Light.m_Color)
        g_Light.m_Color = g_Config.m_Light.m_Color ;
    if (g_Config.m_Light.m_Position != Light.m_Position)
        g_Light.m_Position = g_Config.m_Light.m_Position ;
    if (g_Config.m_Light.m_Intensity != Light.m_Intensity)
        g_Light.m_Intensity = g_Config.m_Light.m_Intensity ;
    if (g_Config.m_Light.m_Radius != Light.m_Radius)
        g_Light.m_Radius = g_Config.m_Light.m_Radius ;
}

//==============================================================================

// Loads objects from current config file
void LoadObjects( xbool bReset )
{
    s32 i, j ;


    x_DebugMsg("LoadObjects\n") ;

    // Reload common config
    LoadCommonConfig() ;

    // Unregister current bitmap
    if (g_PicStatus == PIC_STATUS_VALID)
    {
        vram_Unregister(g_PicBitmap) ;
        g_PicStatus = PIC_STATUS_NONE ;
    }

    // Background pic specified?
    if (g_Config.m_BackPic[0])
    {
        // Show loading message
        x_printf("%s\n", g_Config.m_BackPic) ;
        eng_PageFlip() ;
        x_printfxy(0,0,"Loading...") ;
        eng_PageFlip() ;
        x_printfxy(0,0,"Loading...") ;
        eng_PageFlip() ;

        // Try load
        xbitmap SrcPic ;
        if (Util_DoesFileExist(g_Config.m_BackPic) == FALSE)
        {
            // Not there...
            g_PicStatus = PIC_STATUS_FILE_NOT_FOUND ;
        }
        else
        if (auxbmp_Load(SrcPic, g_Config.m_BackPic))
        {
            // Now copy into the pic that will be used for rendering
            g_PicWidth  = SrcPic.GetWidth() ;
            g_PicHeight = SrcPic.GetHeight() ;

            // Compute scale incase picture is bigger than 512x512
            f32 ScaleX = x_min(1.0f, (f32)PIC_WIDTH  / g_PicWidth ) ;
            f32 ScaleY = x_min(1.0f, (f32)PIC_HEIGHT / g_PicHeight) ;
            f32 Scale  = x_min(ScaleX, ScaleY) ;

            // Use bi-linear to fit bitmap into our 512x512?
            if (Scale < 1.0f)
            {
                // Scale pixel co-ord by this
                ScaleX = ScaleY = 1.0f / Scale ;

                // UVs for bilinear are specified from 0->1
                ScaleX /= g_PicWidth ;
                ScaleY /= g_PicHeight ;

                // Just blit
                for (s32 y = 0 ; y < PIC_HEIGHT ; y++)
                {
                    for (s32 x = 0 ; x < PIC_WIDTH ; x++)
                    {
                        // Compute source uv
                        f32 u = (f32)x * ScaleX ;
                        f32 v = (f32)y * ScaleY ;

                        // If valid, use src color, otherwise just use transparent
                        if ((u < 1) && (v < 1))
                            g_PicBitmap.SetPixelColor(SrcPic.GetBilinearColor(u,v,TRUE), x,y) ;
                        else
                            g_PicBitmap.SetPixelColor(xcolor(0,0,0,0), x,y) ;
                    }
                }
            }
            else
            {
                // Just blit
                for (s32 y = 0 ; y < PIC_HEIGHT ; y++)
                {
                    for (s32 x = 0 ; x < PIC_WIDTH ; x++)
                    {
                        // If valid, use src color, otherwise just use transparent
                        if ((x < g_PicWidth) && (y < g_PicHeight))
                            g_PicBitmap.SetPixelColor(SrcPic.GetPixelColor(x,y), x,y) ;
                        else
                            g_PicBitmap.SetPixelColor(xcolor(0,0,0,0), x,y) ;
                    }
                }
            }

            // Setup valid size
            g_PicWidth  = (s32)(Scale * SrcPic.GetWidth()) ;
            g_PicHeight = (s32)(Scale * SrcPic.GetHeight()) ;
            ASSERT(g_PicWidth  <= PIC_WIDTH) ;
            ASSERT(g_PicHeight <= PIC_HEIGHT) ;

            // Put into native format so alpha works etc
            auxbmp_ConvertToNative(g_PicBitmap) ;

            // Register with vram
            vram_Register(g_PicBitmap) ;

            // It's all good!
            g_PicStatus = PIC_STATUS_VALID ;
        }
        else
        {
            // Wrong format
            g_PicStatus = PIC_STATUS_INVALID_FORMAT ;
        }
    }

    // No configs?
    if ((g_iConfig != -1) && (g_Config.m_ConfigFiles.GetCount() == 0))
    {
        g_iConfig = -1 ;
        g_iObject = -1 ;
        return ;
    }

    // Clear output
    for (i = 0 ; i < 32 ; i++)
        x_printf("\n") ;

    // Show loading message
    eng_PageFlip() ;
    x_printfxy(0,0,"Loading...") ;
    eng_PageFlip() ;
    x_printfxy(0,0,"Loading...") ;
    eng_PageFlip() ;

    // Load objects from config file?
    xbool bSuccess = TRUE;
    if (g_iConfig != -1)
    {
        g_RscMgr.SetRootDirectory( g_Config.GetViewerDataPath() ) ;
        g_RscMgr.SetOnDemandLoading(FALSE) ;
        bSuccess = g_Config.LoadObjectsFromConfig( g_Config.m_ConfigFiles[g_iConfig].m_Name, FALSE ) ;
    }
    else
    {
#ifdef TARGET_PS2
        // Create directory file
        //s32 File = sceOpen(xfs("host0:EXEC:cmd /c dir %s /s /b >dir.txt\n", g_Config.GetGameDataPath()),1) ;
        sceOpen(xfs("host0:EXEC:cmd /c dir %s /s /b >dir.txt\n", g_Config.GetGameDataPath()),1) ;
#endif

#ifdef TARGET_PC
        // Create directory file 
        system(xfs("dir %s /s /b >dir.txt\n", g_Config.GetGameDataPath())) ;
#endif

        // Load objects
        g_RscMgr.SetRootDirectory( g_Config.GetGameDataPath() ) ;
        g_RscMgr.SetOnDemandLoading( FALSE ) ;
        g_Config.LoadObjectsFromDirectory("dir.txt") ;

        // Flag no objects loaded
        g_iLoadedObject = -1 ;

#ifdef TARGET_PS2
        // Close directory file
        //if (File > 0)
            //sceClose(File) ;
#endif
    }
    
    // Range check object
    if (g_iObject < 0)
        g_iObject = 0 ;
    else
    if (g_iObject >= g_Config.m_Objects.GetCount())
        g_iObject = g_Config.m_Objects.GetCount()-1 ;

    // Store object states?
    s32                   nObjectStates = g_nObjects ;
    viewer_object::state* pObjectStates = NULL ;
    if( (!bReset) && ( g_iConfig != -1 ) )
    {
        // Try allocate
        pObjectStates = new viewer_object::state[nObjectStates] ;
        if (pObjectStates)
        {
            // Grab states
            for (i = 0 ; i < nObjectStates ; i++)
                g_pObjects[i].GetState(pObjectStates[i]) ;
        }
    }

    // Clear object list
    delete [] g_pObjects ;
    g_pObjects = NULL ;
    g_nObjects = 0 ;

    // Destroy all resources from the resource manager so they
    // are forced to be reloaded!
    g_RscMgr.UnloadAll() ;
    g_RscMgr.SetOnDemandLoading( FALSE ) ;

    // Allocate objects
    g_nObjects = g_Config.m_Objects.GetCount() ;
    if (g_nObjects)
        g_pObjects = new viewer_object[g_nObjects] ;

    // Clear audio
    g_AudioMgr.UnloadAllPackages() ;

    // Can't fit all objects in memory at once!
    if (g_iConfig != -1)
    {
        // Create viewer objects
        for (i = 0 ; i < g_nObjects ; i++)
        {
            // Show info
            for (j = 0 ; j <= i ; j++)
            {
                // Lookup object
                config_options::object& ConfigObject = g_Config.m_Objects[j] ;

                if (ConfigObject.m_CompiledGeom[0])
                    x_printf("\n%s\n", g_Config.m_Objects[j].m_CompiledGeom) ;

                if (ConfigObject.m_CompiledAnim[0])
                    x_printf("%s\n",   g_Config.m_Objects[j].m_CompiledAnim) ;

                if (ConfigObject.m_CompiledAudio[0])
                    x_printf("%s\n",   g_Config.m_Objects[j].m_CompiledAudio) ;
            }
            x_printfxy(0,0,"Loading...") ;
            eng_PageFlip() ;
            x_printfxy(0,0,"Loading...") ;
            eng_PageFlip() ;

            // Load object
            if( !g_pObjects[i].Init(g_Config.m_Objects[i]) )
                bSuccess = FALSE;
        }
    }
    else
    {
        // Just copy compiled geom names so reset code below works...
        for (i = 0 ; i < g_nObjects ; i++)
        {
            // Copy geom name
            x_strcpy(g_pObjects[i].m_CompiledGeom, g_Config.m_Objects[i].m_CompiledGeom) ;
        }
    }

    // Restore states?
    if (pObjectStates)
    {
        // Set states
        for (i = 0 ; i < x_min(g_nObjects, nObjectStates) ; i++)
        {
            // Only restore if names are the same, otherwise, force a complete reset
            if (x_strcmp(g_pObjects[i].m_CompiledGeom, pObjectStates[i].m_CompiledGeom) == 0)
                g_pObjects[i].SetState(pObjectStates[i]) ;
            else
                bReset = TRUE ;
        }

        // Cleanup
        delete [] pObjectStates ;
    }

    // Reset?
    if (bReset)
    {
        ResetLight() ;
    }
    
    // Create objects that are needed to run the game
    AppReset();

    // If no objects, the go to fx mode
    if( ( g_nObjects == 0 ) && ( g_Config.m_Fxs.GetCount() ) )
        g_Mode = MODE_FX; 

    // Clear output
    if( bSuccess )
    {
        for (i = 0 ; i < 32 ; i++)
            x_printf("\n") ;
    }
    else
    {
        for (i = 0 ; i < 4 ; i++)
            x_printf("\n") ;
    }
}

//==============================================================================

// Sets gadget entry name
void SetGadgetName( s32 Index, const char* pName )
{
    // Remove "INPUT_PS2_"?
    if (x_strncmp(pName, "INPUT_PS2_", 10) == 0)
        x_strcpy(g_InputGadgets[Index].m_Name, &pName[10]) ;
    else
        x_strcpy(g_InputGadgets[Index].m_Name, pName) ;
}

//==============================================================================

void ResetFxView( void )
{
    // Setup default view
    g_FxView.SetXFOV( R_60 );
    g_FxView.SetPosition( vector3(0, 400, -10*100) );
    g_FxView.LookAtPoint( vector3(0, 0,   0) );
    g_FxView.SetZLimits ( 0.1f, 10000.0f );
}

//==============================================================================

void ClearParticles( void )
{
    // Delete all particles
    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_PARTICLE );
    while( SlotID != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
        g_ObjMgr.DestroyObject( pObject->GetGuid() );
        SlotID = g_ObjMgr.GetFirst( object::TYPE_PARTICLE );
    }

    // Delete all particles emitters
    SlotID = g_ObjMgr.GetFirst( object::TYPE_PARTICLE_EVENT_EMITTER );
    while( SlotID != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
        g_ObjMgr.DestroyObject( pObject->GetGuid() );
        SlotID = g_ObjMgr.GetFirst( object::TYPE_PARTICLE_EVENT_EMITTER );
    }

    // Unload all particles?
    if( g_iConfig == -1 )    
    {
        // Unload resources of current object
        if( g_iLoadedObject != -1 )
        {
            // Kill object but keep type so that it isn't reset next time it's initialized
            config_options::type Type = g_pObjects[g_iLoadedObject].m_Type;
            g_pObjects[g_iLoadedObject].Kill() ;
            g_pObjects[g_iLoadedObject].m_Type = Type;
        }

        // Delete all resources from memory
        g_RscMgr.UnloadAll() ;
        g_RscMgr.SetOnDemandLoading( FALSE ) ;

        // Re-load current object
        if( g_iObject != -1 )
        {
            g_pObjects[g_iObject].Init( g_Config.m_Objects[g_iObject] );

            // Flag it's loaded
            g_iLoadedObject = g_iObject ;
        }        
        else
            g_iLoadedObject = -1;
    }
}

//==============================================================================

void AppReset( void )
{
    // Stop player trying to load stuff!
    g_RscMgr.SetOnDemandLoading( TRUE );

    // Flush object manager
    g_ObjMgr.Clear();
    FXMgr.EndOfFrame();
    FXMgr.EndOfFrame();
    FXMgr.EndOfFrame();
    FXMgr.EndOfFrame();
    FXMgr.EndOfFrame();
    
    // Create permanent objects
    g_pPlayer = (player*)g_ObjMgr.GetObjectByGuid( g_ObjMgr.CreateObject("Player") );
    if( g_pPlayer )
    {
        // Turn off rendering and logic
        g_pPlayer->SetAttrBits( g_pPlayer->GetAttrBits() & ~object::ATTR_NEEDS_LOGIC_TIME );
        g_pPlayer->SetAttrBits( g_pPlayer->GetAttrBits() & ~object::ATTR_RENDERABLE );

        // This sets the controller index and enable rumbles        
        g_pPlayer->SetLocalPlayer( 0 );
    }
    
    g_RscMgr.SetOnDemandLoading( FALSE );
    
    // Add fxs from object anims to?
    if( g_iConfig != -1 )
    {
        // Build fx list
        for( s32 iObject = 0; iObject < g_nObjects; iObject++ )
        {
            // Build from anim events
            const anim_group* pAnimGroup = g_pObjects[iObject].m_hAnimGroup.GetPointer();
            if( pAnimGroup )
            {
                // Check all anims
                for( s32 iAnim = 0; iAnim < pAnimGroup->GetNAnims(); iAnim++ )
                {
                    // Get anim
                    const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( iAnim );
                    
                    // Check all events
                    for( s32 iEvent = 0; iEvent < AnimInfo.GetNEvents(); iEvent++ )
                    {
                        // Get event info
                        const anim_event& AnimEvent = AnimInfo.GetEvent( iEvent );
                        const char* AnimEventType = AnimEvent.GetType();
                        
                        // Is this a particle event?
                        if( x_stricmp( AnimEventType, "Particle" ) == 0 )
                        {
                            // Build name
                            char FxName[ 64 ];
                            x_strcpy( FxName, AnimEvent.GetString( anim_event::STRING_IDX_PARTICLE_TYPE ) );
                            x_strcat( FxName, ".fxo" );
                        
                            // Add fx
                            g_Config.AddFx( FxName, FALSE );
                        }
                    }
                }
            }
        }
    }
    
    // Clamp current fx selection
    if( g_iFx >= g_Config.m_Fxs.GetCount() )
        g_iFx = 0;
}

//==============================================================================

// Initialize all systems
void AppInit( void )
{
    // Setup the full path to the platform specific release data
    x_strcpy( g_FullPath, xfs( "%s\\%s", RELEASE_PATH, PLATFORM_PATH ) );

    x_DebugMsg("Initializing app...\n") ;

    // Editables
    g_ShowHelp  = g_Config.m_ShowHelp ;
    g_StatsMode = (stats_mode)g_Config.m_ShowStats ;
    g_Light     = g_Config.m_Light ;

    // Override assert handler
    x_SetRTFHandler(ViewerRTF) ;

    // Init systems
    x_DebugMsg("..Engine\n") ;
    eng_Init();
    
    // Setup text for PS2
    text_SetParams( 512,    // ScreenW
                    448,    // ScreenH
                    8,      // X Border W
                    8,      // Y Border W
                    13,     // CharW
                    18,     // CharH
                    23 );   // NScrollLines
    text_ClearBuffers();
    
    x_DebugMsg("..guid_Init()\n") ;
    guid_Init();

    x_DebugMsg("..anim_event::Init\n") ;
    anim_event::Init();

    x_DebugMsg("..g_SpatialDBase.Init( 400.0f )\n") ;
    g_SpatialDBase.Init( 400.0f );

    x_DebugMsg("..g_PostEffectMgr.Init()\n") ;
    g_PostEffectMgr.Init();

    x_DebugMsg("..g_PlaySurfaceMgr.Init()\n") ;
    g_PlaySurfaceMgr.Init();

    x_DebugMsg("..anim_event::Init\n") ;
    anim_event::Init() ;

    x_DebugMsg( "..AudioManager\n" );
    g_AudioManager.Init( 512 * 1024 );

    g_EarID = g_AudioMgr.CreateEar();

    x_DebugMsg("..MusicMgr\n") ;
    g_MusicMgr.Init();
    
    x_DebugMsg("..ConverseMgr\n") ;
    g_ConverseMgr.Init();

    x_DebugMsg( "..IO Mgr\n" );
    g_IoMgr.Init();

    x_DebugMsg("..render\n") ;
    render::Init() ;

    x_DebugMsg("..ResourceMgr\n") ;
    g_RscMgr.Init();
    g_RscMgr.SetOnDemandLoading( FALSE ) ;

    x_DebugMsg("..PhysicsMgr\n") ;
    g_PhysicsMgr.Init();
    g_PhysicsMgr.m_Settings.m_bUsePolycache = FALSE;

    x_DebugMsg("..ObjMgr\n") ;
    g_ObjMgr.Init();

    x_DebugMsg("..Data vault\n") ;
    g_DataVault.Init();

    // Clear Fx view
    ResetFxView();

    // Create objects that are needed to run the game
    AppReset();

    // Cunningly fill out the gadget name table
    x_DebugMsg("..Gadget table\n") ;
    s32 Index = 0 ;
    #define BEGIN_GADGETS
    #define DEFINE_GADGET(__gadget__)                   SetGadgetName(Index++, #__gadget__) ;
    #define DEFINE_GADGET_VALUE(__gadget__, __value__)  { Index = __value__ ; SetGadgetName(Index++, #__gadget__ ) ; }
    #define END_GADGETS
    #include "e_input_gadget_defines.hpp"

    // Allocate 32bit background pic in native platform format
    byte* pData = (byte*)x_malloc(PIC_WIDTH*PIC_HEIGHT*4) ;
    ASSERT(pData) ;
    g_PicBitmap.Setup(xbitmap::FMT_32_ABGR_8888, PIC_WIDTH, PIC_HEIGHT, TRUE, pData) ;
    auxbmp_ConvertToNative(g_PicBitmap) ;

    // Timers
    g_LogicCPU.Reset() ;
    g_RenderCPU.Reset() ;

    // Load the config
    x_DebugMsg("..Config\n") ;
    g_Config.Init() ;
    LoadCommonConfig() ;

    // Setup default view
    g_View.SetXFOV( R_60 );
    g_View.SetPosition( vector3(0, 100, -200) );
    g_View.LookAtPoint( vector3(0, 100,   0) );
    g_View.SetZLimits ( 0.1f, 10000.0f );

    // Choose config and load the objects
    SelectConfig() ;
    LoadObjects(TRUE) ;

    x_DebugMsg("AppInit successful...\n") ;
}

//=========================================================================

// Kill all systems
void AppKill( void )
{
    x_DebugMsg("AppKill\n") ;

    // Clean data
    delete [] g_pObjects ;
    g_Config.Kill() ;

    // Delete background pic
    if (g_PicStatus == PIC_STATUS_VALID)
    {
        vram_Unregister(g_PicBitmap) ;
        g_PicStatus = PIC_STATUS_NONE ;
    }
    g_PicBitmap.Kill() ;

    // Kill systems
    g_ObjMgr.Kill();
    g_PhysicsMgr.Kill();
    g_PlaySurfaceMgr.Kill();
    g_SpatialDBase.Kill() ;
    g_RscMgr.UnloadAll() ;
    g_RscMgr.Kill() ;
    render::Kill() ;
    g_MusicMgr.Kill() ;
    g_ConverseMgr.Kill() ;
    g_AudioMgr.Kill() ;
    g_IoMgr.Kill();
    eng_Kill() ;

    x_DebugMsg("AppKill successful...\n") ;
}

//=========================================================================

// Handles object mode input
void HandleObjectModeInput( f32 DeltaTime )
{
    // Select prev/next object?
    if ( (g_nObjects) && (input_IsPressed(JOY_OBJECT_SHIFT)) )
    {
        // Previous object?
        if (input_WasPressed(JOY_OBJECT_PREV))
        {
            if (--g_iObject < 0)
                g_iObject = g_nObjects-1 ;
        }

        // Next object?
        if (input_WasPressed(JOY_OBJECT_NEXT))
        {
            if (++g_iObject >= g_nObjects)
                g_iObject = 0 ;
        }
    }

    // Load the object for all game assets?
    if ((g_iConfig == -1) && (g_iLoadedObject != g_iObject))
    {
        // Unload resources of current object
        if (g_iLoadedObject != -1)
        {
            // Kill object but keep type so that it isn't reset next time it's initialized
            config_options::type Type = g_pObjects[g_iLoadedObject].m_Type;
            g_pObjects[g_iLoadedObject].Kill() ;
            g_pObjects[g_iLoadedObject].m_Type = Type;
        }
        
        // Delete all resources from memory
        g_RscMgr.UnloadAll() ;
        g_RscMgr.SetOnDemandLoading( FALSE ) ;

        // Re-load current object
        g_pObjects[g_iObject].Init( g_Config.m_Objects[g_iObject] );

        // Flag it's loaded
        g_iLoadedObject = g_iObject ;
    }

    // Apply input to selected object
    if (g_iObject != -1)
        g_pObjects[g_iObject].HandleInput(DeltaTime) ;
}

//=========================================================================

// Handles input for light mode
void HandleLightModeInput( f32 DeltaTime )
{
    // Make a copy of the light
    config_options::light Light = g_Light ;

    // Rotate around origin
    radian Pitch, Yaw ;
#ifdef TARGET_PC
    Pitch = -DeltaTime * R_180 * input_GetValue(JOY_LIGHT_YAW) ;
    Yaw   = -DeltaTime * R_180 * input_GetValue(JOY_LIGHT_PITCH) ;
#else
    Pitch = -DeltaTime * R_180 * input_GetValue(JOY_LIGHT_PITCH) ;
    Yaw   = -DeltaTime * R_180 * input_GetValue(JOY_LIGHT_YAW) ;
#endif

    if (Pitch != 0)
        Light.m_Position.RotateX(Pitch) ;
    if (Yaw != 0)
        Light.m_Position.RotateY(Yaw) ;

    // Modify intensity
    f32 T = DeltaTime * 0.5f ;
    if (input_IsPressed(JOY_LIGHT_INC_INTENSITY))
        Light.m_Intensity = x_min(1.0f, Light.m_Intensity + T) ;
    if (input_IsPressed(JOY_LIGHT_DEC_INTENSITY))
        Light.m_Intensity = x_max(0.0f, Light.m_Intensity - T) ;

    // Modify radius
    T = DeltaTime * 1000.0f ;
    if (input_IsPressed(JOY_LIGHT_INC_RADIUS))
        Light.m_Radius = x_min(1000000.0f, Light.m_Radius + T) ;
    if (input_IsPressed(JOY_LIGHT_DEC_RADIUS))
        Light.m_Radius = x_max(0.0f,     Light.m_Radius - T) ;

    // Toggle state
    if (input_WasPressed(JOY_LIGHT_TOGGLE_TYPE))
    {
        if (++Light.m_State == config_options::light::STATE_COUNT)
            Light.m_State = 0 ;
    }

    // Reset?
    if (input_WasPressed(JOY_LIGHT_RESET))
    {
        // Reset light
        Light = g_Config.m_Light ;
    }

    // Update?
    if (g_Light != Light)
    {
        // Keep new light
        g_Light = Light ;

        // Re-light all objects
        for (s32 i = 0 ; i < g_nObjects ; i++)
            g_pObjects[i].Light() ;
    }
}

//=========================================================================

// Handles screen shot mode input
void HandleScreenShotModeInput( f32 DeltaTime )
{
    (void)DeltaTime ;

    // Inc size?
    if (input_WasPressed(JOY_SCREEN_SHOT_INC_SIZE))
    {
        if (g_ScreenShotSize < 4)
            g_ScreenShotSize++ ;
    }

    // Dec size?
    if (input_WasPressed(JOY_SCREEN_SHOT_DEC_SIZE))
    {
        if (g_ScreenShotSize > 1)
            g_ScreenShotSize-- ;
    }

    // Take it?
    if (input_WasPressed(JOY_SCREEN_SHOT_TAKE))
    {
        // Auto name filename, but use our screen size
        eng_ScreenShot(g_Config.m_ScreenShotPath.m_Name, g_ScreenShotSize) ;
    }
    
    // Make movie?
    if( !g_bMovieShotActive && input_WasPressed(JOY_SCREEN_SHOT_MOVIE1) && input_WasPressed(JOY_SCREEN_SHOT_MOVIE2) )
    {
        // Start the movie shot
        g_bMovieShotActive = TRUE;
        
        // Start the anim from the start and output the first shot
        if (g_iObject != -1)
        {
            g_pObjects[g_iObject].PlayCurrentAnim( TRUE );
            g_pObjects[g_iObject].DoCurrentAnimFrameScreenShot();
        }            
    }

    // Mode?
    if (input_WasPressed(JOY_SCREEN_SHOT_PIC_MODE))
    {
        if (++g_PicMode == PIC_MODE_COUNT)
            g_PicMode = 0 ;
    }

    // Reset?
    if (input_WasPressed(JOY_SCREEN_SHOT_RESET))
    {
        // Put back to default
        g_ScreenShotSize = 1 ;
        g_PicMode        = PIC_MODE_DEFAULT ;
    }
}

//=========================================================================

// Handles fx mode input
void HandleFxModeInput( f32 DeltaTime )
{
    // Prev fx?
    if( input_WasPressed( JOY_FX_PREV ) )
    {
        if( --g_iFx < 0 )
            g_iFx = g_Config.m_Fxs.GetCount()-1;
    }

    // Next fx?
    if( input_WasPressed( JOY_FX_NEXT ) )
    {
        if( ++g_iFx >= g_Config.m_Fxs.GetCount() )
            g_iFx = 0;
    }

    // Hit play?
    if( ( g_Config.m_Fxs.GetCount() ) && ( input_WasPressed( JOY_FX_PLAY ) ) )
    {
        // Kill current particles
        ClearParticles();
        
        // Start particle
        particle_emitter::CreateParticleEmitter( g_Config.m_Fxs[ g_iFx ].m_Source, 
                                                 particle_emitter::PLAY_FOREVER );
    }

    // Rotate camera?
    {
        radian Pitch, Yaw ;
        f32    S,R,T ;

        // Joy controller input
        S  = DeltaTime ;

        if ( input_IsPressed( JOY_OBJECT_SPEEDUP  ) )  S *= 2.0f;

        g_FxView.GetPitchYaw( Pitch, Yaw );
        R = S * R_90 ;
#ifdef TARGET_PC
        Pitch -= input_GetValue( JOY_OBJECT_YAW ) * R ;
        Yaw   += input_GetValue( JOY_OBJECT_PITCH ) * R ;
#else
        Pitch += input_GetValue( JOY_OBJECT_PITCH ) * R ;
        Yaw   -= input_GetValue( JOY_OBJECT_YAW ) * R ;
#endif
        g_FxView.SetRotation( radian3(Pitch,Yaw,0) );

        // Mouse input
        if ( input_IsPressed( MSE_OBJECT_SPEEDUP  ) )  S *= 2.0f;

        T = S * 200.0f ;
        if ( input_IsPressed( MSE_OBJECT_MOVE ) )
        {
        }
        else
        {
            R = S * R_45 ;
            g_FxView.GetPitchYaw( Pitch, Yaw );       
            Pitch += input_GetValue( MSE_OBJECT_PITCH ) * R ;
            Yaw   -= input_GetValue( MSE_OBJECT_YAW  ) * R ;
            g_FxView.SetRotation( radian3(Pitch,Yaw,0) );
        }
    }

    // Move camera?
    {
        f32    T ;

        // Joy controller input
        f32 S = 1.0f * DeltaTime ;

        if( input_IsPressed( JOY_OBJECT_SPEEDUP   ) )  S *= 2.0f;

        T = S*200 ;
        if( input_IsPressed( JOY_OBJECT_MOVE_UP   ) )  g_FxView.Translate( vector3( 0, T,0 ), view::VIEW );
        if( input_IsPressed( JOY_OBJECT_MOVE_DOWN ) )  g_FxView.Translate( vector3( 0,-T,0 ), view::VIEW );

        T = S*500 ;
#ifdef TARGET_PC    
        g_FxView.Translate( vector3(0, 0, T * -input_GetValue( JOY_OBJECT_MOVE_Z ) ), view::VIEW );
        g_FxView.Translate( vector3(T * input_GetValue( JOY_OBJECT_MOVE_X ),0,0), view::VIEW );
#else
        g_FxView.Translate( vector3(0, 0, T * input_GetValue( JOY_OBJECT_MOVE_Z ) ), view::VIEW );
        g_FxView.Translate( vector3(T * -input_GetValue( JOY_OBJECT_MOVE_X ), 0, 0 ), view::VIEW );
#endif

        // Keyboard input
        T = S * 350 ;
        if( input_IsPressed( KEY_OBJECT_FORWARD ) )  g_FxView.Translate( vector3( 0, 0, T), view::VIEW );
        if( input_IsPressed( KEY_OBJECT_BACK    ) )  g_FxView.Translate( vector3( 0, 0,-T), view::VIEW );
        if( input_IsPressed( KEY_OBJECT_LEFT    ) )  g_FxView.Translate( vector3( T, 0, 0), view::VIEW );
        if( input_IsPressed( KEY_OBJECT_RIGHT   ) )  g_FxView.Translate( vector3(-T, 0, 0), view::VIEW );
        if( input_IsPressed( KEY_OBJECT_UP      ) )  g_FxView.Translate( vector3( 0, T, 0), view::VIEW );
        if( input_IsPressed( KEY_OBJECT_DOWN    ) )  g_FxView.Translate( vector3( 0,-T, 0), view::VIEW );

        // Mouse input
        if ( input_IsPressed( MSE_OBJECT_SPEEDUP  ) )  S *= 2.0f;

        T = S * 200.0f ;
        if ( input_IsPressed( MSE_OBJECT_MOVE ) )
        {
            g_FxView.Translate( vector3( input_GetValue(MSE_OBJECT_MOVE_HORIZ)   * T, 0, 0), view::VIEW );
            g_FxView.Translate( vector3( 0, input_GetValue(MSE_OBJECT_MOVE_VERT) * T, 0), view::VIEW );
        }
        g_FxView.Translate( vector3( 0, 0, input_GetValue(MSE_OBJECT_ZOOM) * S * 10000.0f), view::VIEW );
    }
    
    // Clear
    if( input_WasPressed( JOY_FX_CLEAR ) )
    {
        // Delete all the particles
        ClearParticles();
    }
    
    // Reset
    if( input_WasPressed( JOY_FX_RESET ) )
    {
        // Reset the view
        ResetFxView();

        // Delete all the particles
        ClearParticles();
    }
}

//=========================================================================

// Handle all input
xbool AppHandleInput( f32 DeltaTime )
{
    // Skip if during screen shot
    if( ( g_bMovieShotActive ) || ( eng_ScreenShotActive() ) )
        return TRUE ;

    // Wait for valid input
    if (input_UpdateState())
    {
        // Toggle mode?
        if (        (input_WasPressed(JOY_MAIN_TOGGLE_MODE))
                ||  (input_WasPressed(KEY_MAIN_TOGGLE_MODE)) )
        {
            // Moving from fx mode?
            if( g_Mode == MODE_FX )
                ClearParticles();

            // Next mode        
            if (++g_Mode > MODE_END)
                g_Mode = MODE_START ;
                
            // Skip Fx mode if no particles
            if( ( g_Mode == MODE_FX ) && ( g_Config.m_Fxs.GetCount() == 0 ) )
            {
                // Next mode        
                if (++g_Mode > MODE_END)
                    g_Mode = MODE_START ;
            }
                            
            // Moving from to mode?
            if( g_Mode == MODE_FX )
                ClearParticles();
        }

        // Handle mode input
        switch(g_Mode)
        {
            case MODE_OBJECT:   
                HandleObjectModeInput(DeltaTime) ;
                break ;

            case MODE_LIGHT:
                HandleLightModeInput(DeltaTime) ;
                break ;

            case MODE_SCREEN_SHOT:
                HandleScreenShotModeInput(DeltaTime) ;
                break ;
                
            case MODE_FX:
                HandleFxModeInput(DeltaTime) ;
                break ;
        }

        // Show config menu again?
        if (        (input_WasPressed(JOY_MAIN_SELECT_CONFIG))
                ||  (input_WasPressed(KEY_MAIN_SELECT_CONFIG)) )
        {
            s32 iConfig = g_iConfig ;
            SelectConfig() ;
            LoadObjects(iConfig != g_iConfig) ;
        }

        // Reload objects?
        if (        (input_WasPressed(JOY_MAIN_RELOAD))
                ||  (input_WasPressed(KEY_MAIN_RELOAD)) )
            LoadObjects(FALSE) ;

        // Toggle pause?
        if (        (input_WasPressed(JOY_MAIN_TOGGLE_PAUSE))
                ||  (input_WasPressed(KEY_MAIN_TOGGLE_PAUSE)) )
            g_bPause ^= TRUE ;
        
        // Single step?
        if (        (input_WasPressed(JOY_MAIN_SINGLE_STEP))
                ||  (input_WasPressed(KEY_MAIN_SINGLE_STEP)) )
        {
            AppAdvance(TIME_STEP) ;
        }

        // Toggle stats?
        if (        (input_WasPressed(JOY_MAIN_TOGGLE_STATS))
                ||  (input_WasPressed(KEY_MAIN_TOGGLE_STATS)) )
        {
            s32 iStatsMode = g_StatsMode;
            iStatsMode++ ;
            if (iStatsMode == 4)
                iStatsMode = 0 ;

            g_StatsMode = (stats_mode)iStatsMode ;
        }

        // Toggle help?
        if (        (input_WasPressed(JOY_MAIN_TOGGLE_HELP))
                ||  (input_WasPressed(KEY_MAIN_TOGGLE_HELP)) )
        {
            g_ShowHelp ^= TRUE ;
            
            // Clear debug text
            for( s32 i = 0 ; i < 40 ; i++ )
                x_printf("\n") ;
        }

        // Exit?
        if (        (input_WasPressed(KEY_MAIN_QUIT))
                ||  (input_IsPressed(INPUT_MSG_EXIT)) )
        {
            return( FALSE) ;
        }
    }

    return( TRUE );
}

//==============================================================================

// Renders the scene
void AppRender( void )
{
    static f32 PIXEL_SCALE = DEFAULT_PIXEL_SCALE ;

    // Setup aspect ratio
    f32 PixelScale = PIXEL_SCALE ;
    if (eng_ScreenShotActive())
        PixelScale = 1 ;

    // Setup view
    if( g_Mode == MODE_FX )
        g_View = g_FxView;
    else if (g_iObject != -1)
        g_View = g_pObjects[g_iObject].m_View ;
        
    g_View.SetPixelScale(PixelScale) ;
    eng_MaximizeViewport( g_View );
    eng_SetView         ( g_View );

    // Setup audio
    f32 Near, Far ;
    g_View.GetZLimits(Near, Far) ;

    g_AudioMgr.SetEar( g_EarID, g_View.GetW2V(), g_View.GetPosition(), 0, 1.0f );

    // Setup background color
    eng_SetBackColor(g_Config.m_BackColor) ;

#ifdef TARGET_PS2
    s32 Width, Height;
    eng_GetRes( Width, Height);

    eng_Begin("GS Settings");
    
    // Clear FrontBuffer Alpha Channel
    eng_ClearFrontBuffer();
    eng_WriteToBackBuffer();

    gsreg_Begin(4);
    gsreg_SetScissor( 0, 0, Width, Height );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
    eng_PushGSContext( 1 );
    gsreg_SetScissor( 0, 0, Width, Height );
    gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                   FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
    eng_PopGSContext();
    
    gsreg_End();
    eng_End();
#endif  // #ifdef TARGET_PS2

    // Render background pic
    if (g_PicStatus == PIC_STATUS_VALID)
    {
        // Need to use all of vram (1meg for textures) to hold this 1 meg 512x512x32bit texture!
        vram_Flush();

        // Lookup res
        s32 XRes, YRes ;
        eng_GetRes(XRes, YRes) ;

        // Get center of screen
        f32 HXRes = (f32)XRes * 0.5f ;
        f32 HYRes = (f32)YRes * 0.5f ;

        // Setup quad position relative to center of screen (0,0),
        vector2 XY0, XY1 ;
        switch(g_PicMode)
        {
            default:
            case PIC_MODE_TOP_LEFT:
                XY0.Set(-HXRes,-HYRes) ;
                XY1.X = XY0.X + g_PicWidth ;
                XY1.Y = XY0.Y + g_PicHeight ;
                break ;

            case PIC_MODE_CENTER:
                XY0.Set(-g_PicWidth*0.5f, -g_PicHeight*0.5f) ;
                XY1 = -XY0 ;
                break ;
            
            case PIC_MODE_FIT:
                XY0.Set(-HXRes,-HYRes) ;
                XY1.Set(HXRes, HYRes) ;
                break ;
        }

        // Fix for aspect ratio so that screen shots are WYSIWYG
        f32 Ratio = 1.0f / PixelScale ;
        XY0.Y *= Ratio ;
        XY1.Y *= Ratio ;

        // Convert to pos and size
        vector3 Pos(XY0.X + HXRes, XY0.Y + HYRes, 0) ;
        vector2 Size(XY1.X - XY0.X, XY1.Y - XY0.Y) ;
        vector2 UV0(0,0) ;
        vector2 UV1((f32)g_PicWidth / PIC_WIDTH, (f32)g_PicHeight / PIC_HEIGHT) ;

        // Take big screen shot into account?
        if (eng_ScreenShotActive())
        {
            // Scale
            //Pos    *= eng_ScreenShotSize() ;
            //Size.X *= eng_ScreenShotSize() ;
            //Size.Y *= eng_ScreenShotSize() ;

            // Offset
            //Pos -= vector3( XRes * eng_ScreenShotX(), YRes * eng_ScreenShotY(), 0.0f ) ;
        }

        // Draw quad
        eng_Begin("Pic") ;
        draw_Begin(DRAW_SPRITES, DRAW_2D | DRAW_TEXTURED | DRAW_NO_ZBUFFER | DRAW_NO_ZWRITE | DRAW_USE_ALPHA) ;
        draw_SetTexture(g_PicBitmap) ;
        draw_SpriteUV(Pos, Size, UV0, UV1, XCOLOR_WHITE) ;
        draw_End();
        eng_End() ;
    }

    // split vram into banks for the different texture types
    vram_Flush();
#ifdef TARGET_PS2
    vram_AllocateBank( 25 );     // detail maps
    vram_AllocateBank( 25 );     // specular maps
    vram_AllocateBank( 25 );     // spotlight/shadow maps
#endif

    // Set up the environment map
    cubemap::handle Handle;
    Handle.SetName( "DefaultEnvMap.envmap" );
    render::SetAreaCubeMap( Handle );

    // Setup skinned lighting
    g_LightMgr.ClearLights() ;
    g_LightMgr.BeginLightCollection();
    if (g_Light.m_State == config_options::light::STATE_ON)
    {
        g_LightMgr.AddDynamicLight(g_Light.m_Position, 
                                   g_Light.m_Color,
                                   g_Light.m_Radius,
                                   g_Light.m_Intensity*0.5f,
                                   TRUE) ;
    }

    // Render light
    if (g_Mode == MODE_LIGHT)
    {
        eng_Begin("Light") ;
        xcolor Color = g_Light.m_Color ;
        Color.R = (u8)((f32)Color.R * g_Light.m_Intensity) ;
        Color.G = (u8)((f32)Color.G * g_Light.m_Intensity) ;
        Color.B = (u8)((f32)Color.B * g_Light.m_Intensity) ;
        Color.A = (u8)((f32)Color.A * g_Light.m_Intensity) ;
        Util_DrawMarker(g_Light.m_Position, Color) ;

        if ((g_Mode == MODE_LIGHT) && (g_Light.m_State == config_options::light::STATE_ON))
        {
            draw_ClearL2W() ;
            draw_Sphere(g_Light.m_Position, g_Light.m_Radius, Color) ;
        }
        eng_End() ;
    }

    // Render object?
    if( ( g_Mode != MODE_FX ) && ( g_iObject != -1 ) )
    {
        eng_Begin("Object") ;
        render::BeginNormalRender() ;
        g_RenderCPU.Reset() ;
        g_RenderCPU.Start() ;
        g_pObjects[g_iObject].Render(g_View, g_LogicCPU, g_RenderCPU) ;
        RenderPhysicsInsts();
        g_RenderCPU.Stop() ;
        g_LightMgr.EndLightCollection();
        render::EndNormalRender() ;
        render::BeginCustomRender() ;
        render::EndCustomRender() ;
        eng_End() ;
    }        

    // Render fx grid?
    if(     ( g_Mode == MODE_FX ) && ( !g_ShowHelp ) 
        &&  ( ( g_StatsMode == STATS_MODE_HIGH ) || ( g_StatsMode == STATS_MODE_MEDIUM ) ) )
    {
        // Render grid
        eng_Begin( "Grid" );
        draw_ClearL2W();
        f32 S = 100*5*4 ;
        xcolor C = g_Config.m_GridColor;
        draw_Grid( vector3(-S,  0, -S), 
            vector3( S*2,  0,  0), 
            vector3( 0,  0,  S*2), 
            C, 5*4 );

        // Draw origin axis
        C.R >>= 2 ;                
        C.G >>= 2 ;                
        C.B >>= 2 ;                
        draw_Line( vector3( -S, 0, 0  ), vector3( S, 0, 0 ), C );
        draw_Line( vector3(  0, 0, -S ), vector3( 0, 0, S ), C );
        eng_End();                
    }

    // Render particle objects etc
    eng_Begin("ObjMgr::RenderArtistViewer") ;
    g_ObjMgr.RenderArtistViewer( g_View );
    eng_End() ;

    // Show fx info
    if( ( g_Mode == MODE_FX ) && ( g_Config.m_Fxs.GetCount() ) && ( !g_ShowHelp ) )
    {
        // Render info
        s32 x=0,y=1;
        x_printfxy( x,y++, g_Config.m_Fxs[ g_iFx ].m_Source );
        x_printfxy( x, y++, "Sprites:%d", FXMgr.GetSpriteCount() );
    }

    // Update fx mgr
    FXMgr.EndOfFrame();

    // Apply post render effects
    eng_Begin("PostRenderEffects") ;
    render::BeginPostEffects();
    render::ApplySelfIllumGlows();
    render::EndPostEffects();
    eng_End() ;

    // Show debug info
    s32 x=0,y=0;

    // Show stats?
    if ((!eng_ScreenShotActive()) && (!g_ShowHelp) && (g_StatsMode != STATS_MODE_OFF))
    {
        // Show mode info?
        if (g_StatsMode == STATS_MODE_HIGH)
        {
            // Show current mode and pause flag
            if( g_Mode == MODE_FX )
            {
                x_printfxy(x,y++,"MODE:%s PAUSE:%d FX:%d-%d", 
                    pModeStrings[g_Mode], 
                    g_bPause,
                    g_iFx+1, g_Config.m_Fxs.GetCount() ) ;
            }
            else
            if (g_iConfig != -1)
            {
                x_printfxy(x,y++,"%s MODE:%s PAUSE:%d",  
                           g_Config.m_ConfigFiles[g_iConfig].m_Name, 
                           pModeStrings[g_Mode],
                           g_bPause) ;
            }
            else
            {
                x_printfxy(x,y++,"MODE:%s PAUSE:%d OBJ:%d-%d", 
                    pModeStrings[g_Mode], 
                    g_bPause,
                    g_iObject+1, g_nObjects) ;
            }
        }

        // Show background pic error
        switch(g_PicStatus)
        {
            case PIC_STATUS_FILE_NOT_FOUND:
                y++ ;
                x_printfxy(x,y++,"ERROR! PIC FILE NOT FOUND:") ;
                x_printfxy(x,y++,"%s", g_Config.m_BackPic) ;
                y++ ;
                break ;

            case PIC_STATUS_INVALID_FORMAT:
                y++ ;
                x_printfxy(x,y++,"ERROR! PIC FILE INVALID FORMAT:") ;
                x_printfxy(x,y++,"%s", g_Config.m_BackPic) ;
                x_printfxy(x,y++,"Please use bmp, png, psd, or tga") ;
                y++ ;
                break ;
        }

        // Show object info?
        if ((g_Mode == MODE_OBJECT) && (g_iObject != -1))
        {
            if (g_StatsMode != STATS_MODE_OFF)
                g_pObjects[g_iObject].ShowInfo(x,y) ;

            if (g_StatsMode == STATS_MODE_HIGH)
                g_pObjects[g_iObject].ShowStats(x,22) ;
        }

        // Show light info?
        if (g_Mode == MODE_LIGHT)
        {
            x_printfxy(x,y++,"Color (%d,%d,%d)", 
                       g_Light.m_Color.R,
                       g_Light.m_Color.G,
                       g_Light.m_Color.B) ;

            x_printfxy(x,y++,"Ambient (%d,%d,%d)", 
                       g_Light.m_Ambient.R,
                       g_Light.m_Ambient.G,
                       g_Light.m_Ambient.B) ;

            x_printfxy(x,y++,"BackCol (%d,%d,%d)", 
                       g_Config.m_BackColor.R,
                       g_Config.m_BackColor.G,
                       g_Config.m_BackColor.B) ;

            x_printfxy(x,y++,"Intensity:%.2f", g_Light.m_Intensity) ;
            x_printfxy(x,y++,"Radius:   %.2f", g_Light.m_Radius) ;
            x_printfxy(x,y++,"State:    %s",   g_Light.GetStateString()) ;
        }

        // Screen shot info?
        if (g_Mode == MODE_SCREEN_SHOT)
        {
            // Lookup res
            s32 XRes, YRes ;
            eng_GetRes(XRes, YRes) ;
            
            x_printfxy(x,y++,"ShotSize: %d (%dx%d)", 
                       g_ScreenShotSize,
                       g_ScreenShotSize * XRes,
                       g_ScreenShotSize * YRes) ;

            x_printfxy(x,y++,"PicMode:  %s", pPicModeStrings[g_PicMode]) ;
        }

    // PS2 stats?
    #ifdef TARGET_PS2
        if (g_StatsMode == STATS_MODE_HIGH)
        {
            s32 Count ;
            f32 CPU, GS, INT, VBL, FPS ;
            eng_GetStats(Count, CPU, GS, INT, VBL, FPS) ;
            x_printfxy(0,23, "LOG:%.2f REN:%.2f TOT:%.2f GS/VU:%.2f",  
                       g_LogicCPU.ReadMs(),
                       g_RenderCPU.ReadMs(),
                       g_LogicCPU.ReadMs() + g_RenderCPU.ReadMs(),
                       GS) ;
        }
    #endif
    }

    // Show help?
    if ( (!eng_ScreenShotActive()) && (g_ShowHelp) )
    {
        // Main keys
        x_printfxy(x,y++, "MAIN KEYS") ;
        x_printfxy(x,y++, "ToggleHelp:   %s", g_InputGadgets[JOY_MAIN_TOGGLE_HELP].m_Name) ;
        x_printfxy(x,y++, "ToggleStats:  %s", g_InputGadgets[JOY_MAIN_TOGGLE_STATS].m_Name) ;
        x_printfxy(x,y++, "Reload:       %s", g_InputGadgets[JOY_MAIN_RELOAD].m_Name) ;
        x_printfxy(x,y++, "SelectConfig: %s", g_InputGadgets[JOY_MAIN_SELECT_CONFIG].m_Name) ;
        x_printfxy(x,y++, "ToggleMode:   %s", g_InputGadgets[JOY_MAIN_TOGGLE_MODE].m_Name) ;
        x_printfxy(x,y++, "TogglePause:  %s", g_InputGadgets[JOY_MAIN_TOGGLE_PAUSE].m_Name) ;
        x_printfxy(x,y++, "SingleStep:   %s", g_InputGadgets[JOY_MAIN_SINGLE_STEP].m_Name) ; y++ ;

        // Object keys
        if (g_Mode == MODE_OBJECT)
        {
            x_printfxy(x,y++, "OBJECT MODE KEYS") ;

            // Loco object?
            xbool bLoco = FALSE ;
            if (g_iObject != -1)
                bLoco = (g_pObjects[g_iObject].m_Type == config_options::TYPE_LOCO) ;

            // Non-loco keys
            if (!bLoco)
            {
                x_printfxy(x,y++, "Control camera/object: %s", g_InputGadgets[JOY_OBJECT_SHIFT].m_Name) ;
                x_printfxy(x,y++, "Pitch:                 %s", g_InputGadgets[JOY_OBJECT_PITCH].m_Name) ;
                x_printfxy(x,y++, "Yaw:                   %s", g_InputGadgets[JOY_OBJECT_YAW].m_Name) ;
                x_printfxy(x,y++, "MoveX:                 %s", g_InputGadgets[JOY_OBJECT_MOVE_X].m_Name) ;
                x_printfxy(x,y++, "MoveZ:                 %s", g_InputGadgets[JOY_OBJECT_MOVE_Z].m_Name) ;
                x_printfxy(x,y++, "MoveUp:                %s", g_InputGadgets[JOY_OBJECT_MOVE_UP].m_Name) ;
                x_printfxy(x,y++, "MoveDown:              %s", g_InputGadgets[JOY_OBJECT_MOVE_DOWN].m_Name) ;
            }
            else
            {
                x_printfxy(x,y++, "ToggleMoveStyle:       %s", g_InputGadgets[JOY_OBJECT_MOVE_STYLE].m_Name) ;
                x_printfxy(x,y++, "MoveX:                 %s", g_InputGadgets[JOY_OBJECT_MOVE_X].m_Name) ;
                x_printfxy(x,y++, "MoveZ:                 %s", g_InputGadgets[JOY_OBJECT_MOVE_Z].m_Name) ;
                x_printfxy(x,y++, "LookX:                 %s", g_InputGadgets[JOY_OBJECT_LOOK_X].m_Name) ;
                x_printfxy(x,y++, "LookZ:                 %s", g_InputGadgets[JOY_OBJECT_LOOK_Z].m_Name) ;
                x_printfxy(x,y++, "LookUp:                %s", g_InputGadgets[JOY_OBJECT_LOOK_UP].m_Name) ;
                x_printfxy(x,y++, "LookDown:              %s", g_InputGadgets[JOY_OBJECT_LOOK_DOWN].m_Name) ;
                if( bLoco )
                {
                    x_printfxy(x,y++, "RagdollMode:           %s", g_InputGadgets[JOY_OBJECT_RAGDOLL_SHIFT].m_Name) ;
                    x_printfxy(x,y++, "BlastRagdoll:          %s", g_InputGadgets[JOY_OBJECT_BLAST_RAGDOLL].m_Name) ;
                    x_printfxy(x,y++, "DropRagdoll:           %s", g_InputGadgets[JOY_OBJECT_DROP_RAGDOLL].m_Name) ;
                }
            }
            
            x_printfxy(x,y++, "Speedup:               %s", g_InputGadgets[JOY_OBJECT_SPEEDUP].m_Name) ;

            x_printfxy(x,y++, "PlayAnim:              %s", g_InputGadgets[JOY_OBJECT_PLAY_ANIM].m_Name) ;
            if (bLoco)
                x_printfxy(x,y++, "Anim/Object/Camera:    %s", g_InputGadgets[JOY_OBJECT_SHIFT].m_Name) ;
            else
                x_printfxy(x,y++, "Anim/Object:           %s", g_InputGadgets[JOY_OBJECT_SHIFT].m_Name) ;
            x_printfxy(x,y++, "PrevAnim/PrevObject    %s", g_InputGadgets[JOY_OBJECT_PREV_ANIM].m_Name) ;
            x_printfxy(x,y++, "NextAnim/NextObject    %s", g_InputGadgets[JOY_OBJECT_NEXT_ANIM].m_Name) ;

            x_printfxy(x,y++, "Reset:                 %s", g_InputGadgets[JOY_OBJECT_RESET].m_Name) ;
        }

        // Light keys
        if (g_Mode == MODE_LIGHT)
        {
            x_printfxy(x,y++, "LIGHT MODE KEYS") ;
            x_printfxy(x,y++, "Pitch:             %s", g_InputGadgets[JOY_LIGHT_PITCH].m_Name) ;
            x_printfxy(x,y++, "Yaw:               %s", g_InputGadgets[JOY_LIGHT_YAW].m_Name) ;
            x_printfxy(x,y++, "Speedup:           %s", g_InputGadgets[JOY_LIGHT_SPEEDUP].m_Name) ;
            x_printfxy(x,y++, "IncIntensity:      %s", g_InputGadgets[JOY_LIGHT_INC_INTENSITY].m_Name) ;
            x_printfxy(x,y++, "DecIntensity:      %s", g_InputGadgets[JOY_LIGHT_DEC_INTENSITY].m_Name) ;
            x_printfxy(x,y++, "IncRadius:         %s", g_InputGadgets[JOY_LIGHT_INC_RADIUS].m_Name) ;
            x_printfxy(x,y++, "DecRadius:         %s", g_InputGadgets[JOY_LIGHT_DEC_RADIUS].m_Name) ;
            x_printfxy(x,y++, "ToggleOn/Off/Full: %s", g_InputGadgets[JOY_LIGHT_TOGGLE_TYPE].m_Name) ;
            x_printfxy(x,y++, "Reset:             %s", g_InputGadgets[JOY_LIGHT_RESET].m_Name) ;
        }

        // Screen shot keys
        if (g_Mode == MODE_SCREEN_SHOT)
        {
            x_printfxy(x,y++, "SCREEN SHOT KEYS") ;
            x_printfxy(x,y++, "IncSize:    %s", g_InputGadgets[JOY_SCREEN_SHOT_INC_SIZE].m_Name) ;
            x_printfxy(x,y++, "DecSize:    %s", g_InputGadgets[JOY_SCREEN_SHOT_DEC_SIZE].m_Name) ;
            x_printfxy(x,y++, "PicMode:    %s", g_InputGadgets[JOY_SCREEN_SHOT_PIC_MODE].m_Name) ;
            x_printfxy(x,y++, "TakeShot:   %s", g_InputGadgets[JOY_SCREEN_SHOT_TAKE].m_Name) ;
            x_printfxy(x,y++, "TakeMovie1: %s", g_InputGadgets[JOY_SCREEN_SHOT_MOVIE1].m_Name) ;
            x_printfxy(x,y++, "TakeMovie2: %s", g_InputGadgets[JOY_SCREEN_SHOT_MOVIE2].m_Name) ;
            x_printfxy(x,y++, "Reset:      %s", g_InputGadgets[JOY_SCREEN_SHOT_RESET].m_Name) ;
        }
        
        // Fx keys
        if( g_Mode == MODE_FX )
        {
            x_printfxy(x,y++, "FX MODE KEYS") ;
            
            x_printfxy(x,y++, "Control camera: %s", g_InputGadgets[JOY_OBJECT_SHIFT].m_Name) ;
            x_printfxy(x,y++, "Pitch:          %s", g_InputGadgets[JOY_OBJECT_PITCH].m_Name) ;
            x_printfxy(x,y++, "Yaw:            %s", g_InputGadgets[JOY_OBJECT_YAW].m_Name) ;
            x_printfxy(x,y++, "MoveX:          %s", g_InputGadgets[JOY_OBJECT_MOVE_X].m_Name) ;
            x_printfxy(x,y++, "MoveZ:          %s", g_InputGadgets[JOY_OBJECT_MOVE_Z].m_Name) ;
            x_printfxy(x,y++, "MoveUp:         %s", g_InputGadgets[JOY_OBJECT_MOVE_UP].m_Name) ;
            x_printfxy(x,y++, "MoveDown:       %s", g_InputGadgets[JOY_OBJECT_MOVE_DOWN].m_Name) ; y++;
            
            x_printfxy(x,y++, "Next Fx:        %s", g_InputGadgets[JOY_FX_NEXT].m_Name) ;
            x_printfxy(x,y++, "Prev Fx:        %s", g_InputGadgets[JOY_FX_PREV].m_Name) ;
            x_printfxy(x,y++, "Play Fx:        %s", g_InputGadgets[JOY_FX_PLAY].m_Name) ;
            x_printfxy(x,y++, "Clear:          %s", g_InputGadgets[JOY_FX_CLEAR].m_Name) ;
            x_printfxy(x,y++, "Reset:          %s", g_InputGadgets[JOY_FX_RESET].m_Name) ;
        }
        
    }

    // DONE!
    eng_PageFlip();
}

//==============================================================================

// Advances the scene
void AppAdvance( f32 DeltaTime )
{
    // Add to accumulated time
    static f32 AccumTime = 0 ;
    AccumTime += DeltaTime ;

    // Run at time step rate
    while(AccumTime >= TIME_STEP)
    {
        // Advance object?
        g_LogicCPU.Reset() ;
        g_LogicCPU.Start() ;
        
        // Update object?
        if( g_Mode != MODE_FX )
        {
            if (g_iObject != -1)
                g_pObjects[g_iObject].Advance(TIME_STEP) ;
            g_PhysicsMgr.Advance( TIME_STEP );
        }
                    
        g_ObjMgr.AdvanceAllLogic( DeltaTime );
        g_LogicCPU.Stop() ;            
        
        // Advance render system
        render::Update(DeltaTime) ;

        // Update accumed time
        AccumTime -= TIME_STEP ;
    }

    // Make sure matrices are setup when paused
    if (DeltaTime == 0)
    {
        // Advance object?
        g_LogicCPU.Reset() ;
        g_LogicCPU.Start() ;
        if (g_iObject != -1)
            g_pObjects[g_iObject].Advance(0) ;
        g_LogicCPU.Stop() ;            
    }

    // Update audio
    g_ConverseMgr.Update( DeltaTime );
    g_MusicMgr.Update( DeltaTime );
    g_AudioMgr.Update( DeltaTime );
}

//==============================================================================

// Main program entry point
void AppMain( s32, char** )
{
#ifdef TARGET_PC
        x_DebugMsg("A51 VIEWER BUILT %s\n\n", __TIMESTAMP__) ;
#else
        x_DebugMsg("A51 VIEWER BUILT %s %s\n\n", __DATE__, __TIME__) ;
#endif

#ifdef TARGET_PS2
    //TEMP UNTIL TRAUB FIXES XCL TO USE THE PROJECT SETTINGS WORKING FOLDER
    sceOpen("host0:SETROOT:C:/GameData/A51/Apps/ArtistViewer",0,0);
    //TEMP UNTIL TRAUB FIXES XCL TO USE THE PROJECT SETTINGS WORKING FOLDER
#endif

    AppInit();

    xtick Time[4] = {0} ;
    s32   TimeIndex   = 0 ;

    while( TRUE )
    {
        // Record time
        xtick StartTime = x_GetTime() ;

        // Compute delta time
        xtick SmoothTime = (Time[0] + Time[1] + Time[2] + Time[3]) / 4 ;
        f32   DeltaTime  = (f32)x_TicksToSec(SmoothTime) ;

        // Playing movie?
        if( g_bMovieShotActive )
        {   
            // Cancel movie?
            if (input_IsPressed(JOY_SCREEN_SHOT_CANCEL_MOVIE) )
            {
                g_bMovieShotActive = FALSE;    
            }
        
            // End of movie?
            if( ( g_iObject == -1 ) || ( g_pObjects[g_iObject].HasCurrentAnimFinished() ) )
            {
                g_bMovieShotActive = FALSE;
            }
            else
            {
                // Start another shot?
                if( eng_ScreenShotActive() == FALSE )
                {
                    // Advance to next frame
                    DeltaTime = 1.0f / 30.0f;
                }                    
            }                
        }

        // Quit pressed?
        if( !AppHandleInput(DeltaTime) )
            break;

        // Advance using average time
        if ( (g_bPause) || (eng_ScreenShotActive()) )
            AppAdvance(0) ; // Must call to update render matrices!
        else
            AppAdvance(DeltaTime) ; // Use average time

        // Render
        AppRender();

        // Playing movie?
        if( g_bMovieShotActive )
        {   
            // Start another shot?
            if( ( g_iObject != -1 ) && ( eng_ScreenShotActive() == FALSE ) )
            {
                // Do shot
                g_pObjects[g_iObject].DoCurrentAnimFrameScreenShot();
            }                    
        }

        // Update time table
        xtick EndTime = x_GetTime() ;
        Time[(TimeIndex++) & 3] = EndTime - StartTime ;
    }

    AppKill();
}

//==============================================================================
