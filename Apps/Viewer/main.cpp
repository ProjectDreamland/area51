//=============================================================================
//
//  Geom Viewer
//
//=============================================================================

#include "Entropy.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Render\Render.hpp"
//#include "Render\RenderUpdate.hpp"
#include "Objects\Player.hpp"
#include "Objects\PlaySurface.hpp"
#include "Gamelib\Level.hpp"
#include "Gamelib\Link.hpp"
#include "Auxiliary\MiscUtils\fileio.hpp"
#include "Auxiliary\Bitmap\aux_Bitmap.hpp"

//=============================================================================
//  CONSTANTS
//=============================================================================

#define RELEASE_PATH            "C:\\GameData\\A51\\Release"
#define FILELIST                "filelist.txt"
#define AUTOLOAD                0

//=============================================================================

struct stats
{
    struct ps2
    {
        xbool   VU1Stats;
        xbool   MemoryUsage;
    };
    
    xbool   EngineStats;    
    xbool   RenderStats;
    xbool   RenderVerbose;
    ps2     PS2;
    s32     Interval;
};

//=============================================================================

class tweak
{
public:

    struct entry
    {
        char pName[32];
        f32* pValue;
        f32  Inc;
    };

            tweak       ( void );
    void    AddEntry    ( const char* pName, f32* pValue, f32 Inc );
    void    Update      ( void );
    void    Render      ( void );

private:
    
    xarray<entry>   m_lEntry;
    s32             m_Cursor;
    s32             m_Left;
    s32             m_Right;
};

//=============================================================================
//  GLOBALS
//=============================================================================

xbool       g_Exit        = FALSE;
xbool       g_FreeCam     = TRUE;
xbool       g_UseSelector = TRUE;

xbool       g_RenderBoneBBoxes  = FALSE;

view        g_View;
s32         g_FOV       =    60;
f32         g_NearZ     =    10.0f;
f32         g_FarZ      = 10000.0f;
u32         g_nFrame    =     0;
xtimer      g_GameTimer;

char        g_LevelName[ 256 ];
char        g_FullPath [ 256 ];

stats       g_Stats;

#ifdef TARGET_PS2
xbool   g_game_running = TRUE;
xbool   g_first_person = TRUE;
#endif

xbool        DoBlur = FALSE;

//=============================================================================
//  STATICS
//=============================================================================

static s32     s_Cursor      = 0;
static radian  s_Yaw         = R_0;
static radian  s_Pitch       = R_0;
static xbool   s_Hints       = FALSE;
static xbool   s_IsLoading   = FALSE;
static xbool   s_IsTweaking  = FALSE;
static f32     s_Intensity   = 1.0f;

static vector3 s_LDir( 0.0f, -1.0f, 0.0f );

static tweak   s_Tweak;

static vector4 s_EnvMult1( -1.4f,  0.2f, 2.0f, 800.0f );
static vector4 s_EnvMult2(  0.0f, -2.0f, 0,    800.0f );

//=============================================================================
//  PROTOTYPES
//=============================================================================

void    Render              ( void );
void    Update              ( f32 DeltaTime );
void    Stats               ( f32 DeltaTime );
void    LoadCamera          ( void );
void    SaveCamera          ( void );
xbool   FileSelector        ( char* pFileName );
void    LoadGeom            ( const char* pFileName );
void    LightObject         ( f32 Intensity );

//=============================================================================
//  PLATFORM SPECIFIC
//=============================================================================

#ifdef TARGET_PC
#include "main_pc.cpp"
#endif

#ifdef TARGET_PS2
#include "main_ps2.cpp"
#endif

//=============================================================================
//  MAIN
//=============================================================================

f32 g_BlurOffset;
xcolor g_BlurColor;
void AppMain( s32 argc, char* argv[] )
{
    (void)argc;
    (void)argv;

    ForceLink();

    // Initialize general systems
    eng_Init();
    guid_Init();
    g_ObjMgr.Init();
    g_SpatialDBase.Init( 400.0f );

    // Init stats
    x_memset( &g_Stats, 0, sizeof( g_Stats ) );
    g_Stats.Interval = 1;

    // Setup the full path to the platform specific release data
    x_strcpy( g_FullPath, xfs( "%s\\%s", RELEASE_PATH, PLATFORM_PATH ) );

    // Initialize the resource system
    g_RscMgr.Init();
    g_RscMgr.SetRootDirectory( g_FullPath );
    g_RscMgr.SetOnDemandLoading( TRUE );
    
    // Initialize the render system
    render_Init();

    // Setup render update
    render_update RenderUpdate;

    // Add Tweak items
    {
        s_Tweak.AddEntry( "X1", &s_EnvMult1.X,  0.01f );
        s_Tweak.AddEntry( "Y1", &s_EnvMult1.Y,  0.01f );
        s_Tweak.AddEntry( "Z1", &s_EnvMult1.Z,  0.01f );
        s_Tweak.AddEntry( "X2", &s_EnvMult2.X,  0.01f );
        s_Tweak.AddEntry( "Y2", &s_EnvMult2.Y,  0.01f );
        s_Tweak.AddEntry( "Z2", &s_EnvMult2.Z,  0.01f );
        s_Tweak.AddEntry( "D1", &s_EnvMult1.W, 20.00f );
    }

    // Load the debug camera
    LoadCamera();

    // Setup the global game timer
    g_GameTimer.Reset();
    g_GameTimer.Start();
    
    //
    // Main Loop
    //
    
    while( g_Exit == FALSE )
    {
		g_nFrame++;
        
        // Compute the duration of the last frame
        f32 DeltaTime = MAX( g_GameTimer.ReadSec(), 0.001f );

		g_GameTimer.Reset();
		g_GameTimer.Start();

        RenderUpdate.OnUpdate( DeltaTime );
        
        Update( DeltaTime );
        Render();
        
        Stats( DeltaTime );
    }
}

//=============================================================================

object* GetFirstObject( void )
{
    slot_id SlotID  = g_ObjMgr.GetFirst( object::TYPE_PLAY_SURFACE );
    object* pObject = NULL;
        
    if( SlotID != SLOT_NULL )
    {
        pObject = g_ObjMgr.GetObjectBySlot( SlotID );
        DEMAND( pObject );
    }
    
    return( pObject );
}

//=============================================================================

xbool HandleInput( f32 DeltaTime )
{
    (void)DeltaTime;
    input_UpdateState();
    
    // Check for exit message
    if( input_IsPressed( INPUT_MSG_EXIT ) )
        return( FALSE );

    // Show tweak menu
    if( s_IsLoading == FALSE )
    {
        if( input_WasPressed( INPUT_PS2_BTN_START ) )
            s_IsTweaking ^= 1;
    }

    // Show file selector
    if( input_WasPressed( INPUT_PS2_BTN_SELECT ) ||
        input_WasPressed( INPUT_KBD_TAB ) )
    {
        g_UseSelector = s_IsLoading = TRUE;
        s_IsTweaking  = FALSE;
    }

    // PS2 Free Camera on/off
    if( input_IsPressed ( INPUT_PS2_BTN_L_STICK )  &&
        input_WasPressed( INPUT_PS2_BTN_R_STICK ) )
        g_FreeCam ^= 1;

    static s32 FreeCamDebounce = 0;

    // PC Free Camera on/off
    if( input_WasPressed( INPUT_KBD_GRAVE ) )
    {
        FreeCamDebounce++;
        if( FreeCamDebounce == 1 )
            g_FreeCam ^= 1;
    }
    else
    {
        FreeCamDebounce = 0;
    }

    if( input_WasPressed( INPUT_PS2_BTN_L2 ) )
    {
        DoBlur = !DoBlur;
    }

    // Rotate object
    if( input_IsPressed( INPUT_PS2_BTN_CROSS ) )
    {
        radian Move = R_90 * DeltaTime;
    
        s_Yaw   -= input_GetValue( INPUT_PS2_STICK_LEFT_X ) * Move;
        s_Pitch -= input_GetValue( INPUT_PS2_STICK_LEFT_Y ) * Move;
        
        object* pObject = GetFirstObject();                
        
        if( pObject )
        {
            matrix4 L2W;
            L2W.Identity();
            L2W.Rotate( radian3( s_Pitch, s_Yaw, R_0 ) );
        
            pObject->OnTransform( L2W );
        }
    }
    
    // Hints
    if( input_WasPressed( INPUT_PS2_BTN_SQUARE ) )
    {
        //DoBlur ^= 1;
        //s_Hints ^= 1;
        SaveCamera();
    }

    // Lighting intensity
    if( s_IsTweaking == FALSE )
    {
        if( input_IsPressed( INPUT_PS2_BTN_L_UP ) )
        {
            s_Intensity += 0.025f;
            s_Intensity  = MIN( s_Intensity, 1.0f );
            LightObject( s_Intensity );
        }
        
        if( input_IsPressed( INPUT_PS2_BTN_L_DOWN ) )
        {
            s_Intensity -= 0.025f;
            s_Intensity  = MAX( s_Intensity, 0.0f );
            LightObject( s_Intensity );
        }
    }
    
    // Handle any platform specific input requirements
    if( HandleInputPlatform( DeltaTime ) == FALSE )
        return( FALSE );
 
    return( TRUE );
}

//=============================================================================
	
void Update( f32 DeltaTime )
{
    g_Exit = !HandleInput( DeltaTime );

    if( g_UseSelector == TRUE )
    {
        g_UseSelector = FALSE;
    
        char pFileName[ 256 ];
        x_memset( pFileName, 0, sizeof( pFileName ) );
        
        // Let user choose a file
        if( FileSelector( pFileName ) == TRUE )
        {
            char pExt[256];
        
            x_splitpath( pFileName, NULL, NULL, NULL, pExt );

            if( x_stricmp( pExt, ".rigidgeom" ) == 0 )
            {
                LoadGeom( pFileName );
            }
        }
    }
    
    if( s_IsTweaking == TRUE )
        s_Tweak.Update();
    
    if( g_FreeCam == FALSE )
    {
        g_ObjMgr.AdvanceAllLogic( DeltaTime );
    }
}

//=============================================================================

void RenderHints( void )
{
    if( eng_Begin( "Hints" ) )
    {
        draw_ClearL2W();
        draw_Grid( vector3( -3200.0f, 0.0f, -3200.0f ),
                vector3(  6400.0f, 0.0f,     0.0f ),
                vector3(     0.0f, 0.0f,  6400.0f ),
                xcolor ( 0, 128, 0 ),
                16 );

        object* pObject = GetFirstObject();
        
        if( pObject )
        {
            draw_SetL2W( pObject->GetL2W() );
            draw_Axis( 100.0f );
        }
        
        eng_End();
    }
}

//=============================================================================

void Render( void )
{
    // Perform any platform specific render initialization
    InitRenderPlatform();

    // Set background clear color
    eng_SetBackColor( XCOLOR_BLACK );
    eng_SetBackColor( xcolor( 64, 64, 64 ) );

    if( g_FreeCam == FALSE )
    {
        // Get View from player
        slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_PLAYER );
        
        if( SlotID != SLOT_NULL )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
            
            if( pObject )
            {
                player& Player = player::GetSafeType( *pObject );
                g_View = Player.GetView();
            }
        }
    }

    g_View.SetXFOV( DEG_TO_RAD( g_FOV ) );
    g_View.SetZLimits( g_NearZ, g_FarZ );

    // Set the viewport
    eng_MaximizeViewport( g_View );
    eng_SetView         ( g_View, 0 );
    eng_ActivateView    ( 0 );

    render_SetEnvMult( s_EnvMult1, s_EnvMult2 );

    if( s_IsTweaking == TRUE )
        s_Tweak.Render();

    if( s_IsLoading == FALSE )
    {
        if( s_Hints == TRUE )
            RenderHints();
        
        // create the fake specular/env. map
        render_CreateEnvTexture();

        // Render all objects
        g_ObjMgr.Render();
    }

    render_ApplySelfIllumGlows();

    EndRenderPlatform();
    
    eng_PageFlip();
}

//=============================================================================

void Stats( f32 DeltaTime )
{
    static f32 s_StatTimer = 0.0f;
    s_StatTimer += DeltaTime;
    
    if( s_StatTimer > g_Stats.Interval )
    {
        s_StatTimer = 0.0f;
        
        if( g_Stats.EngineStats == TRUE )
            eng_PrintStats();

        if( g_Stats.RenderStats == TRUE )
        {
            s32 Mode = render::stats::TO_DEBUG;
            
            if( g_Stats.RenderVerbose == TRUE )
                Mode |= render::stats::VERBOSE;

            render_GetStats().Print( Mode );
        }
        
        PrintStatsPlatform();
    }
}

//=============================================================================

void SaveCamera( void )
{
    matrix4 M = g_View.GetV2W();
    X_FILE* fp;
    
    if( !(fp = x_fopen( xfs( "%s\\camera.dat", g_FullPath ), "wb" ))) ASSERT( FALSE );
    x_fwrite( &M, sizeof( M ), 1, fp );
    x_fclose( fp );
    x_DebugMsg( "Camera saved\n" );
}

//=============================================================================

void LoadCamera( void )
{
    X_FILE* fp;

    matrix4 ViewMat;
    ViewMat.Identity();
    
    if( !(fp = x_fopen( xfs( "%s\\camera.dat", g_FullPath ), "rb" ))) return;
    x_fread( &ViewMat, sizeof( ViewMat ), 1, fp );
    x_fclose( fp );

    g_View.SetV2W( ViewMat );
    x_DebugMsg( "Camera loaded\n" );
}

//=============================================================================

xbool InitSelector( xarray< char[256] >& List )
{
    char    pPath[ 256 ];
    X_FILE* pFile;
    
    // Build full path to filelist
    x_makepath( pPath, NULL, g_FullPath, FILELIST, NULL );

    pFile = x_fopen( pPath, "rt" );
    
    if( pFile == NULL )
        return( FALSE );

    s32 Size = x_flength( pFile ) + 1;
    
    // Load filelist into buffer
    char* pBuffer = new char[ Size ];
    char* pMemory = pBuffer;
    x_memset( pBuffer, 0, Size );
    x_fread ( pBuffer, 1, Size, pFile );
    x_fclose( pFile );

    // Build the list
    while( *pBuffer )
    {
        // Skip over whitespace
        while( x_isspace( *pBuffer ) == TRUE )
        {
            if( *pBuffer == 0 )
                break;

            pBuffer++;
        }
        
        if( *pBuffer != '\0' )
        {
            char* pString = List.Append();
        
            // Copy string
            while( TRUE )
            {
                char C = *pBuffer;
                
                if( (C == '\0') || (C == 0x0A) || (C == 0x0D) )
                    break;
            
                *pString++ = *pBuffer++;
            }
            
            // Remove any trailing whitespace
            while( x_isspace( *(--pString) ) );
            
            pString[1] = '\0';
        }
    }
    
    delete pMemory;

    return( TRUE );
}

//=============================================================================

void DisplaySelector( const xarray< char[256] >& List )
{
    s32 MaxLines  =  20;
    s32 MaxCursor =  10;
    s32 Arrow     = s_Cursor;
    s32 Count     = MIN( List.GetCount(), MaxLines );
    
    if( s_Cursor < MaxCursor )
    {
        for( s32 i=0; i<Count; i++ )
            x_printfxy( 2, 3+i, List[i] );
    }
    else
    if( s_Cursor < (List.GetCount() - MaxCursor) )
    {
        s32 Start = s_Cursor - MaxCursor + 1;
        Arrow     = MaxCursor - 1;
    
        for( s32 i=0; i<Count; i++ )
        {
            x_printfxy( 2, 3+i, List[ Start + i ] );
        }
    }
    else
    {
        s32 Start = List.GetCount() - MaxLines;
        Arrow     = s_Cursor - Start;
    
        for( s32 i=0; i<Count; i++ )
        {
            x_printfxy( 2, 3+i, List[ Start + i ] );
        }
    }

    x_printfxy( 2, 1, "Select a file" );
    x_printfxy( 0, 3+Arrow, xfs( ">>" ) );
}

//=============================================================================

xbool FileSelector( char* pFileName )
{
    enum
    {
        LOAD_FILELIST,
        SELECT_FILE,
        DELAY = 60,
    };

    xarray<char[256]> List;

    s32 State = LOAD_FILELIST;

    while( (g_Exit != TRUE) && (State < DELAY) )
    {
        input_UpdateState();
    
        // Check for exit message
        if( input_IsPressed( INPUT_MSG_EXIT ) )
            g_Exit = TRUE;
        
        if( input_IsPressed ( INPUT_PS2_BTN_TRIANGLE ) )
        {
            s_IsLoading = FALSE;
            return( FALSE );
        }
        
        if( input_WasPressed( INPUT_PS2_BTN_SELECT   ) ) State = LOAD_FILELIST;
        
        // State machine
        switch( State )
        {
            case LOAD_FILELIST :
            {
                List.Clear();
            
                if( InitSelector( List ) == TRUE )
                {
                    State = SELECT_FILE;
                }
                else
                {
                    x_printfxy( 1, 1, xfs( "Unable to load %s", FILELIST ) );
                    s_Cursor = 0;
                }
            
                break;
            }
        
            case SELECT_FILE :
            {
                static s32 Down = 0, Up = 0;
                
                if( input_IsPressed ( INPUT_PS2_BTN_L_DOWN ) ||
                    input_WasPressed( INPUT_KBD_DOWN       ) )  Down++; else Down = 0;
        
                if( input_IsPressed ( INPUT_PS2_BTN_L_UP   ) ||
                    input_WasPressed( INPUT_KBD_UP         ) )  Up++;   else Up   = 0;
        
                if( input_IsPressed ( INPUT_PS2_BTN_CROSS  ) ||
                    input_WasPressed( INPUT_KBD_SPACE      ) )  State++;

                // Debounce button and check for repeat
                if( (Down == 1) || (Down > 20) ) s_Cursor++;
                if( (Up   == 1) || (Up   > 20) ) s_Cursor--;
                
                // Keep the cursor in limits
                s_Cursor = MINMAX( 0, s_Cursor, List.GetCount()-1 );
        
                DisplaySelector( List );
            
                #if AUTOLOAD
                State++;
                #endif
                break;
            }
            
            default :
            {
                x_printfxy( 1, 1, xfs( "Loading \"%s\"", List[ s_Cursor ] ) );
                x_printfxy( 1, 3, "Please Wait..." );
                State++;
                break;
            }
        }
        
        Render();
    }
    
    // Copy the name of the file we want to load
    x_strcpy( pFileName, List[ s_Cursor ] );

    return( TRUE );
}

//=============================================================================

void InitLoad( void )
{
    g_ObjMgr.Init();
    g_SpatialDBase.Init( 400.0f );

    // Flush all the loaded resources
    g_RscMgr.UnloadAll();

    // Initialize the render system
    render_Init();
}

//=============================================================================

void LoadLevel( const char* pLevel )
{
    char pPath[ 256 ];
    
    // Load the NavMap
    x_makepath( pPath, NULL, g_FullPath, pLevel, ".nmp" );
    
    // Load the level
    x_makepath( pPath, NULL, g_FullPath, pLevel, ".level" );

    level Level;
    Level.Open( pPath );
    Level.Load();
    Level.Close();

    // Setup resource handles to rigid color table
    x_makepath( pPath, NULL, g_FullPath, pLevel, ".rigidcolor" );
    Level.SetRigidColor( pPath );
}

//=============================================================================

void LoadGeom( const char* pFileName )
{
    x_DebugMsg( "Loading Geom: %s\n", pFileName );

    InitLoad();
    
    // Create a new object
    guid Guid = g_ObjMgr.CreateObject( "Play Surface" );
    
    object* pObject = g_ObjMgr.GetObjectByGuid( Guid );
    
    play_surface& PlaySurface = play_surface::GetSafeType( *pObject );

    // Load the geom
    prop_query I;
    I.WQueryExternal( "RenderInst\\File", pFileName );
    PlaySurface.OnProperty( I );

    // Create dummy color table
    rigid_inst& RigidInst = PlaySurface.GetRigidInst();
    geom*       pGeom     = RigidInst.GetGeom();
    
    if( pGeom )
    {
        rigid_color_info RigidColor;
        
        RigidColor.nColors = pGeom->m_TotalVertices * 2;
        RigidColor.pColor  = new u16[ RigidColor.nColors ];
        
        for( s32 i=0; i < RigidColor.nColors; i++ )
        {
            s32 I = 16;
            RigidColor.pColor[i] = 0x8000 | (I << 10) | (I << 5) | I;
        }

        fileio File;
        File.Save( xfs( "%s\\test.rigidcolor", g_FullPath ), RigidColor, FALSE );
        
        delete RigidColor.pColor;
    }

    x_DebugMsg( "Loading Color Table\n" );
    PlaySurface.LoadColorTable( "test.rigidcolor" );

    LightObject( s_Intensity );

    const bbox& BBox   = PlaySurface.GetBBox();
    f32         Radius = BBox.GetRadius() * 1.3f;
    
    // Calculate a good starting position for the camera
    vector3 Org( PlaySurface.GetPosition() );
    vector3 Pos( Org + vector3( Radius, Radius, Radius ) );
    
    g_View.SetPosition( Pos );
    g_View.LookAtPoint( Org );
    
    // Load the debug camera
    LoadCamera();
    
    s_Pitch     = R_0;
    s_Yaw       = R_0;
    s_IsLoading = FALSE;
}

//=============================================================================

void LightObject( f32 Intensity )
{
    object* pObject = GetFirstObject();
    
    if( pObject == NULL )
        return;
        
    play_surface& PlaySurface = play_surface::GetSafeType( *pObject );
    rigid_inst&   RigidInst   = PlaySurface.GetRigidInst();

    geom* pGeom = RigidInst.GetGeom();
    
    if( pGeom == NULL )
        return;
    
    rigid_geom& RigidGeom = *(rigid_geom*)pGeom;
    LightObjectPlatform( RigidInst.GetColorTable(), RigidGeom, Intensity );
}

//=============================================================================

tweak::tweak( void )
{
    m_Cursor = 0;
    m_Left   = 0;
    m_Right  = 0;
}

//=============================================================================

void tweak::AddEntry( const char* pName, f32* pValue, f32 Inc )
{
    entry& Entry = m_lEntry.Append();
    
    x_strcpy( Entry.pName, pName );
    Entry.pValue = pValue;
    Entry.Inc    = Inc;
}

//=============================================================================

void tweak::Update( void )
{
    if( input_WasPressed( INPUT_PS2_BTN_L_UP   ) ) m_Cursor--;
    if( input_WasPressed( INPUT_PS2_BTN_L_DOWN ) ) m_Cursor++;

    m_Cursor = MINMAX( 0, m_Cursor, m_lEntry.GetCount() - 1 );
    
    if( input_IsPressed( INPUT_PS2_BTN_L_LEFT  ) ) m_Left++;  else m_Left  = 0;
    if( input_IsPressed( INPUT_PS2_BTN_L_RIGHT ) ) m_Right++; else m_Right = 0;

    f32 Add = 0.0f;
    f32 Inc = m_lEntry[ m_Cursor ].Inc;
    
    if( (m_Left  == 1) || (m_Left  > 10) ) Add =  Inc;
    if( (m_Right == 1) || (m_Right > 10) ) Add = -Inc;

    *(m_lEntry[ m_Cursor ].pValue) += Add;
}

//=============================================================================

void tweak::Render( void )
{
    s32 OX = 1;
    s32 OY = 1;

    for( s32 i=0; i < m_lEntry.GetCount(); i++ )
    {
        entry& Entry = m_lEntry[i];
    
        x_printfxy( OX+1, OY + i, xfs( "%s : %1.2f", Entry.pName, *Entry.pValue ) );
    }
    
    x_printfxy( OX, OY + m_Cursor, ">" );
}

//=============================================================================

