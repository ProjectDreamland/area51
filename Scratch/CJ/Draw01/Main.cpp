//==============================================================================
//
//  Toy01.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "aux_Bitmap.hpp"
extern "C" {
#include "lua/include/lua.h"
#include "lua/include/lualib.h"
#include "lua/include/lauxlib.h"
}

//==============================================================================

extern void World_Init( void );
extern void World_Render( void );
extern void World_AddBox( bbox& Box, xcolor Color );

extern vector3 g_PlayerCenter;

//==============================================================================
//  STORAGE
//==============================================================================

view            View;
random          R;
xbitmap         Tex4Bit;
xbitmap         Tex8Bit;
xbitmap         Tex32Bit;
xbitmap         Logo;
s32             ScreenW;
s32             ScreenH;

#ifdef TARGET_PC
    const char*     DataPath = "Data\\";
#endif

#ifdef TARGET_GCN
    const char*     DataPath = "";
#endif

#ifdef TARGET_PS2
    const char*     DataPath = "";
#endif


//==============================================================================
//  FUNCTIONS
//==============================================================================

void Initialize( void )
{
    eng_Init();

    View.SetXFOV( R_60 );
    View.SetPosition( vector3(100,100,-200) );
    View.LookAtPoint( vector3(100,  0,  0) );
    View.SetZLimits ( 0.1f, 100000.0f );

    VERIFY( auxbmp_LoadNative( Tex4Bit,  xfs("%sTex4Bit.bmp",        DataPath) ) );
    VERIFY( auxbmp_LoadNative( Tex8Bit,  xfs("%sTex8Bit.bmp",        DataPath) ) );
    VERIFY( auxbmp_LoadNative( Tex32Bit, xfs("%sTex32Bit.tga",       DataPath) ) );
    VERIFY( auxbmp_LoadNative( Logo,     xfs("%sInevitable_Logo.tga",DataPath) ) );

    vram_Register( Tex4Bit  );
    vram_Register( Tex8Bit  );
    vram_Register( Tex32Bit );
    vram_Register( Logo     );

    eng_GetRes( ScreenW, ScreenH );

    World_Init();
}

//=========================================================================

void Shutdown( void )
{
    vram_Unregister( Tex4Bit    );
    vram_Unregister( Tex8Bit    );
    vram_Unregister( Tex32Bit   );
    vram_Unregister( Logo       );

    Tex4Bit.Kill();
    Tex8Bit.Kill();
    Tex32Bit.Kill();
    Logo.Kill();
}

//=========================================================================

xbool HandleInput( f32 DeltaTime )
{
    while( input_UpdateState() )
    {
        
    #ifdef TARGET_PC
        // Move view using keyboard and mouse
        // WASD - forward, left, back, right
        // RF   - up, down
        // MouseRightButton - 4X speed

        radian Pitch;
        radian Yaw;
        f32    S = 12.5f * 10 * DeltaTime;
        f32    R = 0.005f * 40 * DeltaTime;

        if( input_IsPressed( INPUT_KBD_ESCAPE  ) )  return( FALSE );

        if( input_IsPressed( INPUT_MOUSE_BTN_R ) )  S *= 4.0f;
        if( input_IsPressed( INPUT_KBD_W       ) )  View.Translate( vector3( 0, 0, S), view::VIEW );
        if( input_IsPressed( INPUT_KBD_S       ) )  View.Translate( vector3( 0, 0,-S), view::VIEW );
        if( input_IsPressed( INPUT_KBD_A       ) )  View.Translate( vector3( S, 0, 0), view::VIEW );
        if( input_IsPressed( INPUT_KBD_D       ) )  View.Translate( vector3(-S, 0, 0), view::VIEW );
        if( input_IsPressed( INPUT_KBD_R       ) )  View.Translate( vector3( 0, S, 0), view::VIEW );
        if( input_IsPressed( INPUT_KBD_F       ) )  View.Translate( vector3( 0,-S, 0), view::VIEW );

        View.GetPitchYaw( Pitch, Yaw );       
        Pitch += input_GetValue( INPUT_MOUSE_Y_REL ) * R;
        Yaw   -= input_GetValue( INPUT_MOUSE_X_REL ) * R;
        View.SetRotation( radian3(Pitch,Yaw,0) );

//        View.LookAtPoint( g_PlayerCenter - vector3( 0, -400, 1000 ), g_PlayerCenter );


        if( input_GetValue( INPUT_MOUSE_BTN_R ) )
        {
        }

        if( input_IsPressed( INPUT_MSG_EXIT ) )
            return( FALSE );
    #endif

    #ifdef TARGET_PS2

        radian Pitch;
        radian Yaw;
        f32    S   = 0.50f;
        f32    R   = 0.025f;
        f32    Lateral;
        f32    Vertical;

        if( input_IsPressed( INPUT_PS2_BTN_L1 ) )  S *= 4.0f;
        if( input_IsPressed( INPUT_PS2_BTN_R1 ) )  View.Translate( vector3( 0, S, 0), view::VIEW );
        if( input_IsPressed( INPUT_PS2_BTN_R2 ) )  View.Translate( vector3( 0,-S, 0), view::VIEW );

        Lateral  = S * input_GetValue( INPUT_PS2_STICK_LEFT_X );
        Vertical = S * input_GetValue( INPUT_PS2_STICK_LEFT_Y );
        View.Translate( vector3(0,0,Vertical), view::VIEW );
        View.Translate( vector3(-Lateral,0,0), view::VIEW );

        View.GetPitchYaw( Pitch, Yaw );
        Pitch += input_GetValue( INPUT_PS2_STICK_RIGHT_Y ) * R;
        Yaw   -= input_GetValue( INPUT_PS2_STICK_RIGHT_X ) * R;
        View.SetRotation( radian3(Pitch,Yaw,0) );

    #endif

    }

    return( TRUE );
}

//==============================================================================

void Render( void )
{
    eng_MaximizeViewport( View );
    eng_SetView         ( View );

    //==---------------------------------------------------
    //  BOX
    //==---------------------------------------------------
    eng_Begin( "Box" );
    {
        draw_BBox( bbox(vector3(-50,-50,-50),vector3(50,50,50)) );
    }
    eng_End();

#if 0
    //==---------------------------------------------------
    //  2D ALPHA QUAD
    //==---------------------------------------------------
    eng_Begin( "AlphaQuad" );
    {
        draw_Begin( DRAW_QUADS, DRAW_2D | DRAW_USE_ALPHA | DRAW_NO_ZBUFFER );
        draw_Color ( xcolor( 255,255,255,  0 ) ); draw_Vertex( vector3(  0,  0, 10) ); 
        draw_Color ( xcolor( 255,255,255,255 ) ); draw_Vertex( vector3(  0,255, 10) ); 
        draw_Color ( xcolor( 255,255,255,255 ) ); draw_Vertex( vector3(100,255, 10) ); 
        draw_Color ( xcolor( 255,255,255,  0 ) ); draw_Vertex( vector3(100,  0, 10) ); 
        draw_End();
    }
    eng_End();


    //==---------------------------------------------------
    //  TEXTURE SAMPLES
    //==---------------------------------------------------
    f32 StartX = -50;

    eng_Begin( "TextureSamples" );
    {
        //==-------------------------------------
        //  4 bit texture
        //
        f32     NextX = StartX + 50;


        draw_Begin( DRAW_QUADS, DRAW_TEXTURED );

        //  Set the current texture
        draw_SetTexture( Tex4Bit );

        //  We only need to set the color once if it is the same
        //  for every vert.  Draw will cache it internally and
        //  apply it to all verts that don't have an explicit
        //  vertex color provided.
        draw_Color( xcolor( 255,255,255 ) );

        // Send UV/Vertex pairs       
        draw_UV    ( 0,0 );     draw_Vertex( vector3( StartX, 50,0) );
        draw_UV    ( 0,1 );     draw_Vertex( vector3( StartX, 0, 0) );
        draw_UV    ( 1,1 );     draw_Vertex( vector3( NextX,  0, 0) );
        draw_UV    ( 1,0 );     draw_Vertex( vector3( NextX,  50,0) );

        draw_End();


        //==-------------------------------------
        //  8 bit texture
        //
        StartX = NextX  + 10;
        NextX  = StartX + 50;

        draw_Begin( DRAW_QUADS, DRAW_TEXTURED );
        draw_SetTexture( Tex8Bit );

        draw_Color( xcolor( 255,255,255 ) );
        
        draw_UV    ( 0,0 );     draw_Vertex( vector3( StartX, 50,0) );
        draw_UV    ( 0,1 );     draw_Vertex( vector3( StartX, 0, 0) );
        draw_UV    ( 1,1 );     draw_Vertex( vector3( NextX,  0, 0) );
        draw_UV    ( 1,0 );     draw_Vertex( vector3( NextX,  50,0) );
        draw_End();

        //==-------------------------------------
        //  32 bit texture
        //
        StartX = NextX  + 10;
        NextX  = StartX + 50;

        draw_Begin( DRAW_QUADS, DRAW_TEXTURED );
        draw_SetTexture( Tex32Bit );

        draw_Color( xcolor( 255,255,255 ) );

        draw_UV    ( 0,0 );     draw_Vertex( vector3( StartX, 50,0) );
        draw_UV    ( 0,1 );     draw_Vertex( vector3( StartX, 0, 0) );
        draw_UV    ( 1,1 );     draw_Vertex( vector3( NextX,  0, 0) );
        draw_UV    ( 1,0 );     draw_Vertex( vector3( NextX,  50,0) );
        draw_End();

        //==-------------------------------------
        //  32 bit texture with alpha
        //
        StartX = NextX  + 10;
        NextX  = StartX + 50;
        
        draw_Begin( DRAW_QUADS, DRAW_TEXTURED | DRAW_USE_ALPHA );
        draw_SetTexture( Tex32Bit );
        
        draw_Color( xcolor( 255,255,255 ) );

        draw_UV    ( 0,0 );     draw_Vertex( vector3( StartX, 50,0) );
        draw_UV    ( 0,1 );     draw_Vertex( vector3( StartX, 0, 0) );
        draw_UV    ( 1,1 );     draw_Vertex( vector3( NextX,  0, 0) );
        draw_UV    ( 1,0 );     draw_Vertex( vector3( NextX,  50,0) );
        draw_End();
    }
    eng_End();


    //==---------------------------------------------------
    //  2D SPRITE
    //==---------------------------------------------------
    eng_Begin( "2D Sprite" );
    {
        draw_Begin( DRAW_QUADS, DRAW_2D | DRAW_TEXTURED | DRAW_USE_ALPHA);
        draw_SetTexture( Logo );
        
        draw_Color( xcolor( 255,255,255 ) );
        
        draw_UV    ( 0,0 );     draw_Vertex( vector3( (f32)ScreenW - 100, (f32)ScreenH - 100, 0) );
        draw_UV    ( 0,1 );     draw_Vertex( vector3( (f32)ScreenW - 100, (f32)ScreenH,       0) );
        draw_UV    ( 1,1 );     draw_Vertex( vector3( (f32)ScreenW,       (f32)ScreenH,       0) );
        draw_UV    ( 1,0 );     draw_Vertex( vector3( (f32)ScreenW,       (f32)ScreenH - 100, 0) );
        draw_End();
    }
    eng_End();
#endif // 0

    // Render the world
    World_Render();
}

//==============================================================================

static int l_sin( lua_State *L )
{
    float d = lua_tonumber( L, 1 );         // get argument
    lua_pushnumber( L, (float)sin(d) );     // push result
    return 1;                               // number of results
}

static int l_print (lua_State *L)
{
    int n = lua_gettop(L);  /* number of arguments */
    int i;
    lua_getglobal(L, "tostring");
    for (i=1; i<=n; i++) {
        const char *s;
        lua_pushvalue(L, -1);  /* function to be called */
        lua_pushvalue(L, i);   /* value to print */
        lua_call(L, 1, 1);
        s = lua_tostring(L, -1);  /* get result */
        if (s == NULL)
            return luaL_error(L, "`tostring' must return a string to `print'");
        if (i>1) x_DebugMsg( "\t" );
        x_DebugMsg( s );
        lua_pop(L, 1);  /* pop result */
    }
    x_DebugMsg( "\n" );
    return 0;
}

//==============================================================================

static int l_draw_Marker( lua_State *L )
{
    float x = lua_tonumber( L, 1 );         // get argument
    float y = lua_tonumber( L, 2 );         // get argument
    float z = lua_tonumber( L, 3 );         // get argument

    draw_Marker( vector3(x,y,z) );

    return 0;                               // number of results
}

static int l_draw_Begin( lua_State *L )
{
    draw_Begin( DRAW_TRIANGLES, 0 );
    return 0;                               // number of results
}

static int l_draw_End( lua_State *L )
{
    draw_End();
    return 0;                               // number of results
}

static int l_draw_Vertex( lua_State *L )
{
    float x = lua_tonumber( L, 1 );         // get argument
    float y = lua_tonumber( L, 2 );         // get argument
    float z = lua_tonumber( L, 3 );         // get argument

    draw_Vertex( x, y, z );

    return 0;                               // number of results
}

static int l_draw_Color( lua_State *L )
{
    float r = lua_tonumber( L, 1 );         // get argument
    float g = lua_tonumber( L, 2 );         // get argument
    float b = lua_tonumber( L, 3 );         // get argument
    float a = lua_tonumber( L, 4 );         // get argument

    draw_Color( r, g, b, a );

    return 0;                               // number of results
}

static const luaL_reg drawlib[] =
{
    {"Marker",  l_draw_Marker},

    {"Begin",   l_draw_Begin},
    {"End",     l_draw_End},

    {"Vertex",  l_draw_Vertex},
    {"Color",   l_draw_Color},

    {NULL, NULL}
};

#define LUA_DRAWLIBNAME "draw"

// Open draw library
LUALIB_API int luaopen_draw (lua_State *L) {
    luaL_openlib(L, LUA_DRAWLIBNAME, drawlib, 0);
    return 1;
}

//==============================================================================

static vector3* l_checkvector3( lua_State *L )
{
    void *pData = luaL_checkudata( L, 1, "vector3" );
    luaL_argcheck( L, pData != NULL, 1, "`vector3` expected" );
    return (vector3*)pData;
}

static int l_vector3_new( lua_State *L )
{
//    int n = luaL_checkint(L, 1);
    size_t nbytes = sizeof( vector3 );
    vector3* pVector = (vector3*)lua_newuserdata( L, nbytes );
    luaL_getmetatable( L, "vector3" );
    lua_setmetatable( L, -2 );

    if( lua_gettop( L ) > 1 )
    {
        pVector->GetX() = luaL_check_number( L, 1 );
        pVector->GetY() = luaL_check_number( L, 2 );
        pVector->GetZ() = luaL_check_number( L, 3 );
    }

    return 1;  // new userdatum is already on the stack
}

static int l_vector3_tostring( lua_State *L )
{
    vector3* pVector = l_checkvector3( L );
    lua_pushstring( L, xfs("(%.3f,%.3f,%.3f)", pVector->GetX(), pVector->GetY(), pVector->GetZ()) );
    return 1;
}

static const struct luaL_reg vector3lib_f [] =
{
    {"new", l_vector3_new},
    {NULL, NULL}
};

static const struct luaL_reg vector3lib_m [] =
{
    {"__tostring", l_vector3_tostring},
    {NULL, NULL}
};

// Open vector3 library
int luaopen_vector3( lua_State *L )
{
    luaL_newmetatable( L, "vector3" );

    lua_pushstring( L, "__index" );
    lua_pushvalue( L, -2 );  /* pushes the metatable */
    lua_settable( L, -3 );  /* metatable.__index = metatable */

    luaL_openlib( L, NULL, vector3lib_m, 0 );
    luaL_openlib( L, "vector3", vector3lib_f, 0 );
    return 1;
}

//==============================================================================
// Lua compiled chunk loader
//==============================================================================

class lua_chunk
{
protected:
    lua_chunk();

public:
    lua_chunk( const char* pFileName ) { String.LoadFile( pFileName ); BytesRead = 0; }

    xstring String;
    s32     BytesRead;
};

static const char* LuaLoader( lua_State* L, void* pData, size_t* pSize )
{
    lua_chunk* pChunk = (lua_chunk*)pData;
    *pSize = pChunk->String.GetLength() - pChunk->BytesRead;
    return *pSize ? &pChunk->String[0] : NULL;
}

//==============================================================================

class Sorter : public x_compare_functor<const xstring&>
{
public:
    s32 operator()( const xstring& A, const xstring& B )
    {
        xstring strItem1 = A;
        xstring strItem2 = B;
        strItem1.MakeUpper();
        strItem2.MakeUpper();
        if( strItem1 < strItem2 )   return( -1 );
        if( strItem1 > strItem2 )   return(  1 );
        else return(  0 );
    }
};


void AppMain( s32, char** )
{
    Initialize();


    xarray<xstring> a;

    a.Append() = "3";
    a.Append() = "2";
    a.Append() = "1";

    {
        for( s32 i=0; i<a.GetCount(); i++ )
        {
            x_DebugMsg( "%s\n", (const char*)a[i] );
        }
    }
    x_qsort( &a[0], a.GetCount(), Sorter() );
    {
        for( s32 i=0; i<a.GetCount(); i++ )
        {
            x_DebugMsg( "%s\n", (const char*)a[i] );
        }
    }



    World_AddBox( bbox( vector3( -10000, -1, -10000 ), vector3( 10000, 0, 10000 ) ), XCOLOR_WHITE );

    bbox b( vector3( 0, 0, 0 ), vector3( 200, 10, 15 ) );
    for( s32 i=0 ; i<20 ; i++ )
    {
        World_AddBox( b, XCOLOR_WHITE );
        b.Translate( vector3( 0, 10, 15 ) );
    }

    World_AddBox( bbox( vector3( -10, 0, 0 ), vector3( 0, 20*20, 20*20 ) ), XCOLOR_WHITE );

    // Init Lua
    lua_State* L = lua_open();

    luaopen_base( L );
    luaopen_table( L );
    luaopen_string( L );
    luaopen_math( L );
    luaopen_debug( L );

    luaopen_vector3( L );
    luaopen_draw( L );

    lua_pushcfunction( L, l_sin );
    lua_setglobal( L, "mysin" );
    lua_pushcfunction( L, l_print );
    lua_setglobal( L, "print" );

    // Load our file & Execute
    lua_chunk Chunk( "luac.out" );
//    int Result = lua_load( L, LuaLoader, &Chunk, "luac.out" ); // This loads binary
    int Result = luaL_loadfile( L, "t.lua" ); // This loads source & compiles
    Result =  lua_pcall( L, 0, 0, 0 );
    if( Result != 0 )
    {
        const char* pError = lua_tostring(L, -1);
        x_DebugMsg( pError );
    }

    // Start the logic timer
    xtimer Timer;
    Timer.Start();

    while( TRUE )
    {
        // Get DeltaTime
        x_DelayThread( 33 );
        f32 DeltaTime = Timer.ReadSec();
        Timer.Reset();
        Timer.Start();

        // Input
        if( !HandleInput( DeltaTime ) )
            break;

        // Render
        eng_MaximizeViewport( View );
        eng_SetView         ( View );
        World_Render();

        // Render markers through lua
        eng_Begin( "lua" );
        lua_getglobal( L, "DrawMarkers" );
        lua_pushnumber( L, 100 );
        lua_pushnumber( L, 100 );
        lua_pushnumber( L, 100 );
        lua_pushnumber( L, 100 );
        Result = lua_pcall( L, 4, 0, 0 );
        if( Result != 0 )
        {
            const char* pError = lua_tostring(L, -1);
            x_DebugMsg( pError );
        }
        eng_End();

        eng_PageFlip();
    }

    Shutdown();
}

//==============================================================================

void pc_PreResetCubeMap( void ) {}
void pc_PostResetCubeMap( void ) {}
