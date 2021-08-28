//==============================================================================
//
//  Toy01.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "Auxiliary\Bitmap\aux_Bitmap.hpp"
#include "RawMesh.hpp"
#include "Quantizer.h"
#include "Compress.hpp"
#include "NewQuantizer.hpp"
#include "TGALoader.hpp"


// Profecy: www.twilight3d.com

//==============================================================================
//  STORAGE
//==============================================================================

view            View2;
view            View;
random          R;
s32             ScreenW;
s32             ScreenH;
xbitmap         Specular;
xbitmap         SpecularAlpha;
rawmesh         RawMesh;
s32             SelectMethod=7;
xbool           AdditiveMode= TRUE;

xbitmap         BmpXFilesAlpha;
xbitmap         BmpPropAlpha;

f32             XFilesSeconds;
f32             XPropSeconds;

xbitmap         BmpXFiles;
xbitmap         BmpSteve;
xbitmap         BmpProp;
xbitmap         BmpCompress[2];
xbitmap         BmpCompressV2[2];
s32             Page = 0;

//==============================================================================
// FUNCTIONS
//==============================================================================

//==============================================================================

void LoadWithAlpha( xbitmap& Bitmap, const char* pDiff, const char* pAlpha = NULL )
{
    xbitmap Alpha;

    auxbmp_Load         ( Bitmap, pDiff );    
    //TGA_Load            ( Bitmap, pDiff );    

    Bitmap.ConvertFormat( xbitmap::FMT_32_ARGB_8888 );

    if( pAlpha )
    {
        auxbmp_Load         ( Alpha,  pAlpha );    
        //TGA_Load            ( Alpha,  pAlpha );    

        for( s32 y=0; y<Bitmap.GetHeight(); y++ )
        for( s32 x=0; x<Bitmap.GetWidth(); x++ )
        {
            xcolor A, F;

            A = Alpha.GetPixelColor ( x,y );
            F = Bitmap.GetPixelColor( x,y );
            F.A = A.R;
            Bitmap.SetPixelColor(F, x,y);
        }
    }

    auxbmp_ConvertToD3D( Bitmap );
    vram_Register( Bitmap );
}

//==============================================================================

void PaletticeSteve( xbitmap& DestBitmap, xbitmap& Bitmap )
{
    // Declare a color tree
    color_tree Tree;
    s32        x,y,i;

    // Loop through all the pixels of your source bitmap and call .....
    for( x=0; x<Bitmap.GetWidth();  x++ )
    for( y=0; y<Bitmap.GetHeight(); y++ )
        Tree.AddColor( Bitmap.GetPixelColor(x,y) );

    // Call the quantizer to setup as many colors as you want
    Tree.Quantize(256);

    // Create a new bitmap to store the palettized version
    byte*       pPixelData = new byte[ Bitmap.GetWidth() * Bitmap.GetHeight() ];
    xcolor*     pClutData  = new xcolor[ 256 ];

    // Use something like this to grab the palette:
    for( i = 0 ; i < 256 ; i++ )
	    pClutData[i] = Tree.GetClutColor(i);

    // Then remap the source pixels into the palettized format    
    for( x=0; x<Bitmap.GetWidth();  x++ )
    for( y=0; y<Bitmap.GetHeight(); y++ )
        pPixelData[x + y*Bitmap.GetWidth()] = Tree.GetClutIndex( Bitmap.GetPixelColor(x,y) );

    // Set the new bitmap
    DestBitmap.Setup( xbitmap::FMT_P8_ARGB_8888,    
                      Bitmap.GetWidth(),
                      Bitmap.GetHeight(),
                      TRUE,
                      pPixelData,
                      TRUE,
                      (byte*)pClutData );
} 

//==============================================================================
void Initialize( void )
{
    d3deng_SetResolution  ( 1024, 1024 );
    eng_Init();

    View.SetXFOV( R_60 );
    View.SetPosition( vector3(654.458f,458.855f,-408.020f) );
    View.LookAtPoint( vector3(  0,  0,  0) );
    View.SetZLimits ( 0.1f, 10000.0f );

    View2 = View;

    eng_GetRes( ScreenW, ScreenH );

    //
    // Load textures
    //
    LoadWithAlpha( Specular, "test1.tga", NULL );
    LoadWithAlpha( SpecularAlpha, "test1.tga", "test1Alpha.tga" );

    //
    // First lets convert the x files version
    //
    if( 1 )
    {
        xtimer Timer;

        BmpXFilesAlpha = SpecularAlpha;

        Timer.Start();
        BmpXFilesAlpha.ConvertFormat( xbitmap::FMT_P8_ARGB_8888 );
        XFilesSeconds = Timer.ReadSec();

        auxbmp_ConvertToD3D( BmpXFilesAlpha );
        vram_Register( BmpXFilesAlpha );
    }
    
    //
    // Using the profecy engine
    //
    if( 1 )
    {
        xtimer Timer;

        BmpPropAlpha = SpecularAlpha;

        Timer.Start();
        QuanticeImage( BmpPropAlpha, 0.99999f);
        XPropSeconds = Timer.ReadSec();

        auxbmp_ConvertToD3D( BmpPropAlpha );
        vram_Register( BmpPropAlpha );   
    }

    //
    // First lets convert the x files version
    //
    if( 1 )
    {
        BmpXFiles = Specular;
        BmpXFiles.ConvertFormat( xbitmap::FMT_P8_ARGB_8888 );
        auxbmp_ConvertToD3D( BmpXFiles );
        vram_Register( BmpXFiles );
    }

    //
    // Now lets convert with  Steve paleticer
    //
    if( 1 )
    {
        PaletticeSteve( BmpSteve, Specular );
        auxbmp_ConvertToD3D( BmpSteve );
        vram_Register( BmpSteve );    
    }

    //
    // Using the profecy engine
    //
    if( 1 )
    {
        BmpProp = Specular;
        QuanticeImage( BmpProp, 0.99999f);
        auxbmp_ConvertToD3D( BmpProp );
        vram_Register( BmpProp );   
    }

    //
    // Compress PS2
    //
    if( 1 )
    {
        auxbmp_CompressPS2( BmpXFiles, BmpCompress[0], BmpCompress[1], FALSE ) ;
        auxbmp_ConvertToD3D( BmpCompress[0] );
        auxbmp_ConvertToD3D( BmpCompress[1] );
        vram_Register( BmpCompress[0] );   
        vram_Register( BmpCompress[1] );   
    }

    //
    // Compress textures
    //
    if( 1 )
    {
        CompressPS2( Specular, BmpCompressV2[0], BmpCompressV2[1] );
        auxbmp_ConvertToD3D( BmpCompressV2[0] );
        auxbmp_ConvertToD3D( BmpCompressV2[1] );
        vram_Register( BmpCompressV2[0] );   
        vram_Register( BmpCompressV2[1] );   
    }    
}

//=========================================================================

void Shutdown( void )
{
}

//=========================================================================

xbool HandleInput( void )
{
    while( input_UpdateState() )
    {
        // Move view using keyboard and mouse
        // WASD - forward, left, back, right
        // RF   - up, down
        // MouseRightButton - 4X speed

        radian Pitch;
        radian Yaw;
        f32    S = 16.125f/8;
        f32    R = 0.005f;

        if( input_IsPressed( INPUT_KBD_ESCAPE  ) )  return( FALSE );

        if( input_IsPressed( INPUT_MOUSE_BTN_L ) )  S *= 4.0f;
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

        if( input_WasPressed( INPUT_KBD_RIGHT) )  Page = iMin( 3, ++Page );
        if( input_WasPressed( INPUT_KBD_LEFT ) )  Page = iMax( 0, --Page );
    }

    return( TRUE );
}


//==============================================================================

void UTIL_RenderSprite( f32 X, f32 Y, f32 W, f32 H )
{
    struct vertex{ float x,y,z,rhw,u,v; };
    vertex Vertex[]={
    { X,   Y+H,  0.5f, 1.0f, 0.0f, 1.0f }, //  2
    { X+W, Y+H,  0.5f, 1.0f, 1.0f, 1.0f }, //  3 -- 3
    { X,   Y,    0.5f, 1.0f, 0.0f, 0.0f }, //  1
    { X+W, Y,    0.5f, 1.0f, 1.0f, 0.0f }, //  4 -- 4                                    
    };

    g_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW | D3DFVF_TEX1 );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, Vertex, sizeof(vertex) );
}


//==============================================================================

void UTIL_RenderCube( void )
{
    struct vertex{ float x,y,z,nx,ny,nz,u,v; };
    static vertex Vertex[]={
    { -1.0f, -1.0f,  1.0f, -0.577350f, -0.577350f,  0.577350f,  1.0f, 0.0f }, //  1
    { -1.0f, -1.0f, -1.0f, -0.577350f, -0.577350f, -0.577350f,  0.0f, 0.0f }, //  2
    {  1.0f, -1.0f,  1.0f,  0.577350f, -0.577350f,  0.577350f,  1.0f, 1.0f }, //  3 -- 3
    {  1.0f, -1.0f, -1.0f,  0.577350f, -0.577350f, -0.577350f,  0.0f, 1.0f }, //  4 -- 4                                    
    {  1.0f,  1.0f, -1.0f,  0.577350f,  0.577350f, -0.577350f,  0.0f, 0.0f }, //  5 -- 5
    { -1.0f, -1.0f, -1.0f, -0.577350f, -0.577350f, -0.577350f,  1.0f, 1.0f }, //  6
    { -1.0f,  1.0f, -1.0f, -0.577350f,  0.577350f, -0.577350f,  1.0f, 0.0f }, //  7
    { -1.0f, -1.0f,  1.0f, -0.577350f, -0.577350f,  0.577350f,  0.0f, 1.0f }, //  8
    { -1.0f,  1.0f,  1.0f, -0.577350f,  0.577350f,  0.577350f,  0.0f, 0.0f }, //  9
    {  1.0f, -1.0f,  1.0f,  0.577350f, -0.577350f,  0.577350f,  1.0f, 1.0f }, // 10 -- 3
    {  1.0f,  1.0f,  1.0f,  0.577350f,  0.577350f,  0.577350f,  1.0f, 0.0f }, // 11 -- oposite 4                                    
    {  1.0f,  1.0f, -1.0f,  0.577350f,  0.577350f, -0.577350f,  0.0f, 0.0f }, // 12 -- 5
    { -1.0f,  1.0f,  1.0f, -0.577350f,  0.577350f,  0.577350f,  1.0f, 1.0f }, // 13 -- 9
    { -1.0f,  1.0f, -1.0f, -0.577350f,  0.577350f, -0.577350f,  0.0f, 1.0f } }; // 14 -- 7

    g_pd3dDevice->SetVertexShader( (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1) );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 12, Vertex, sizeof(vertex) );
}

//==============================================================================

void Page1( void )
{
    x_printfxy( 0,0, "Page 1 - Compare paletticers" );

    //
    // Render the original texture
    //
    {
        draw_Begin( DRAW_SPRITES, DRAW_2D );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        

        draw_SetTexture( Specular );
        draw_Sprite     ( vector3(0,0,0),
                          vector2(512,512),
                          xcolor(0xffffffff) );

        draw_End();
    }

    //
    // Render the profecy texture
    //
    {
        draw_Begin( DRAW_SPRITES, DRAW_2D );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT  );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        

        draw_SetTexture( BmpProp     );
        draw_Sprite     ( vector3(512,0,0),
                          vector2(512,512),
                          xcolor(0xffffffff) );

        draw_End();
    }

    //
    // Render the xfiles version 
    //
    {
        draw_Begin( DRAW_SPRITES, DRAW_2D );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT  );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        

        draw_SetTexture( BmpXFiles );
        draw_Sprite     ( vector3(0,512,0),
                          vector2(512,512),
                          xcolor(0xffffffff) );

        draw_End();
    }

    //
    // Render the Steve version 
    //
    {
        draw_Begin( DRAW_SPRITES, DRAW_2D );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT  );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        

        draw_SetTexture( BmpSteve );
        draw_Sprite     ( vector3(512,512,0),
                          vector2(512,512),
                          xcolor(0xffffffff) );

        draw_End();
    }
}

//==============================================================================

void Page2( void )
{
    x_printfxy( 0,0, "Page 2 - New Compression PS2" );

    //
    // Render the original texture
    //
    {
        draw_Begin( DRAW_SPRITES, DRAW_2D );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        

        draw_SetTexture( Specular );
        draw_Sprite     ( vector3(0,0,0),
                          vector2(512,512),
                          xcolor(0xffffffff) );

        draw_End();
    }

    //
    // Render the original texture
    //
    {
        // FILTER
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR  );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR  );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR  );

        // Set the texture coor
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0  );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0  );
        
        // COLOR
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );

        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE  );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );

        g_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP,   D3DTOP_ADD );
        g_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT );
        g_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );

        // ALPHA
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

        // END
        g_pd3dDevice->SetTextureStageState( 3, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 3, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        

        g_pd3dDevice->SetTexture( 0, vram_GetSurface( BmpCompressV2[0] ) );
        g_pd3dDevice->SetTexture( 1, vram_GetSurface( BmpCompressV2[1] ) );

        UTIL_RenderSprite( 512, 0, 512, 512 );


        // Base        
        draw_Begin( DRAW_SPRITES, DRAW_2D );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT  );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        

        draw_SetTexture( BmpCompressV2[0]  );
        draw_Sprite     ( vector3(0,512,0),
                          vector2(512,512),
                          xcolor(0xffffffff) );

        draw_End();

        // Luminance
        draw_Begin( DRAW_SPRITES, DRAW_2D );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT  );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        

        draw_SetTexture( BmpCompressV2[1]  );
        draw_Sprite     ( vector3(512,512,0),
                          vector2(512,512),
                          xcolor(0xffffffff) );

        draw_End();
    }
}

//==============================================================================

void Page3( void )
{
    x_printfxy( 0,0, "Page 3 - Compare Old Compress, New Compress, Palettice and Original" );

    //
    // Render the original texture
    //
    {
        draw_Begin( DRAW_SPRITES, DRAW_2D );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        

        draw_SetTexture( Specular );
        draw_Sprite     ( vector3(0,0,0),
                          vector2(512,512),
                          xcolor(0xffffffff) );

        draw_End();
    }

    //
    // Render the original texture
    //
    {
        // FILTER
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR  );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR  );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR  );

        // Set the texture coor
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0  );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0  );
        
        // COLOR
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );

        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );

        g_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP,   D3DTOP_ADD );
        g_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT );
        g_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );

        // ALPHA
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

        // END
        g_pd3dDevice->SetTextureStageState( 3, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 3, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        

        g_pd3dDevice->SetTexture( 0, vram_GetSurface( BmpCompress[0] ) );
        g_pd3dDevice->SetTexture( 1, vram_GetSurface( BmpCompress[1] ) );

        UTIL_RenderSprite( 512, 0, 512, 512 );
    }

    //
    // Render the profecy texture
    //
    {
        draw_Begin( DRAW_SPRITES, DRAW_2D );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT  );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        

        draw_SetTexture( BmpProp     );
        draw_Sprite     ( vector3(0,512,0),
                          vector2(512,512),
                          xcolor(0xffffffff) );

        draw_End();
    }

    //
    // New Compress Method
    //
    {
        // FILTER
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR  );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_MINFILTER, D3DTEXF_LINEAR  );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR  );

        // Set the texture coor
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0  );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0  );
        
        // COLOR
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );

        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );

        g_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLOROP,   D3DTOP_ADD );
        g_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT );
        g_pd3dDevice->SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT );

        // ALPHA
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_CURRENT );

        // END
        g_pd3dDevice->SetTextureStageState( 3, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 3, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        

        g_pd3dDevice->SetTexture( 0, vram_GetSurface( BmpCompressV2[0] ) );
        g_pd3dDevice->SetTexture( 1, vram_GetSurface( BmpCompressV2[1] ) );

        UTIL_RenderSprite( 512, 512, 512, 512 );
    }
}

//==============================================================================

void Page4( void )
{
    x_printfxy( 0,0, "Page 4 - XFiles VS New Method" );
    x_printfxy( 0,1, "XBMP: %f New: %f", XFilesSeconds, XPropSeconds );

    //
    // Render the original texture
    //
    {
        draw_Begin( DRAW_SPRITES, DRAW_2D );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        

        draw_SetTexture( BmpXFilesAlpha );
        draw_Sprite     ( vector3(0,0,0),
                          vector2(512,512),
                          xcolor(0xffffffff) );

        draw_End();
    }

    //
    // Render the profecy texture
    //
    {
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT  );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATECOLOR_ADDALPHA );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR  );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        

        g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
        g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_ONE );
        g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_ZERO );
        g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, 0 );

        g_pd3dDevice->SetTexture( 0, vram_GetSurface( BmpXFilesAlpha ) );
        
        UTIL_RenderSprite( 512, 0, 512, 512 );
    }

    //
    // Render the xfiles version 
    //
    {
        draw_Begin( DRAW_SPRITES, DRAW_2D );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT  );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        

        draw_SetTexture( BmpPropAlpha );
        draw_Sprite     ( vector3(0,512,0),
                          vector2(512,512),
                          xcolor(0xffffffff) );

        draw_End();
    }

    //
    // Render the Steve version 
    //
    {
        draw_Begin( DRAW_SPRITES, DRAW_2D );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_POINT  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_POINT  );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATECOLOR_ADDALPHA );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE  );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR  );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        

        g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
        g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_ONE );
        g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_ZERO );
        g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, 0 );

        draw_SetTexture( BmpPropAlpha );
        draw_Sprite     ( vector3(512,512,0),
                          vector2(512,512),
                          xcolor(0xffffffff) );

        draw_End();
    }
}



//==============================================================================

void Render( void )
{
    eng_Begin( "Textures" );

    if( Page == 0 ) Page1();
    if( Page == 1 ) Page2();
    if( Page == 2 ) Page3();
    if( Page == 3 ) Page4();

    eng_End();      
}

//==============================================================================

void Advance( f32 Seconds )
{
}


//==============================================================================

void AppMain( s32, char** )
{
    Initialize();

    xtimer Timer;

    while( TRUE )
    {
        if( !HandleInput() )
            break;

        eng_MaximizeViewport( View );
        eng_SetView         ( View, 0 );
        eng_ActivateView    ( 0 );

        Advance( Timer.TripSec() );

        Render();

        // DONE!
        eng_PageFlip();
    }

    Shutdown();
}

//==============================================================================
