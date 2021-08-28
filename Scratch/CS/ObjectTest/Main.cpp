//==============================================================================
//
//  Toy01.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================
#include "header.hpp"
#include "Entropy.hpp"
#include "aux_Bitmap.hpp"

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


//==============================================================================
//  FUNCTIONS
//==============================================================================

void Initialize( void )
{
    eng_Init();

    View.SetXFOV( R_60 );
    View.SetPosition( vector3(100,100,200) );
    View.LookAtPoint( vector3(  0,  0,  0) );
    View.SetZLimits ( 0.1f, 1000.0f );

    VERIFY( auxbmp_LoadNative( Tex4Bit,  xfs("%sTex4Bit.bmp",        DataPath) ) );
    VERIFY( auxbmp_LoadNative( Tex8Bit,  xfs("%sTex8Bit.bmp",        DataPath) ) );
    VERIFY( auxbmp_LoadNative( Tex32Bit, xfs("%sTex32Bit.tga",       DataPath) ) );
    VERIFY( auxbmp_LoadNative( Logo,     xfs("%sInevitable_Logo.tga",DataPath) ) );

    vram_Register( Tex4Bit  );
    vram_Register( Tex8Bit  );
    vram_Register( Tex32Bit );
    vram_Register( Logo     );

    eng_GetRes( ScreenW, ScreenH );


    
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

xbool HandleInput( void )
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
        f32    S = 0.125f;
        f32    R = 0.005f;

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

    #ifdef TARGET_GCN

        radian Pitch;
        radian Yaw;
        f32    S   = 0.50f;
        f32    R   = 0.025f;
        f32    Lateral;
        f32    Vertical;

        if( input_IsPressed( INPUT_GCN_BTN_Z ) )  S *= 4.0f;
        if( input_IsPressed( INPUT_GCN_BTN_Y ) )  View.Translate( vector3( 0, S, 0), view::VIEW );
        if( input_IsPressed( INPUT_GCN_BTN_X ) )  View.Translate( vector3( 0,-S, 0), view::VIEW );

        Lateral  = S * input_GetValue( INPUT_GCN_STICK_LEFT_X );
        Vertical = S * input_GetValue( INPUT_GCN_STICK_LEFT_Y );
        View.Translate( vector3(0,0,Vertical), view::VIEW );
        View.Translate( vector3(-Lateral,0,0), view::VIEW );

        View.GetPitchYaw( Pitch, Yaw );
        Pitch += input_GetValue( INPUT_GCN_STICK_RIGHT_Y ) * R;
        Yaw   -= input_GetValue( INPUT_GCN_STICK_RIGHT_X ) * R;
        View.SetRotation( radian3(Pitch,Yaw,0) );

    #endif

    }

    return( TRUE );
}

//==============================================================================

void Render( void )
{
    eng_MaximizeViewport( View );
    eng_SetView         ( View, 0 );
    eng_ActivateView    ( 0 );

    //==---------------------------------------------------
    //  GRID, BOX AND MARKER
    //==---------------------------------------------------
    eng_Begin( "GridBoxMarker" );
    {
        draw_ClearL2W();
        draw_Grid( vector3(  0,  0,  0), 
                   vector3(100,  0,  0), 
                   vector3(  0,  0,100), 
                   xcolor (  0,128,  0), 16 );
        draw_Marker( R.v3( 50,75,0,25,50,75 ), R.color() );
        draw_BBox( bbox(vector3(50,0,50),vector3(75,25,75)) );
    }
    eng_End();

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

    // DONE!
    eng_PageFlip();
}

//==============================================================================

void AppMain( s32, char** )
{
    Initialize();

    while( TRUE )
    {
        if( !HandleInput() )
            break;
        Render();
    }

    Shutdown();
}

//==============================================================================
