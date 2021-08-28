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

//==============================================================================
//  STORAGE
//==============================================================================

view            View2;
view            View;
random          R;
s32             ScreenW;
s32             ScreenH;
xbitmap         Bitmap;
xbitmap         Specular;
xbitmap         Bump[2];
rawmesh         RawMesh;
s32             SelectMethod=8;
xbool           AdditiveMode= TRUE;

//==============================================================================
// FUNCTIONS
//==============================================================================

//==============================================================================

void LoadWithAlpha( xbitmap& Bitmap, const char* pDiff, const char* pAlpha )
{
    xbitmap Alpha;
    auxbmp_Load         ( Bitmap, pDiff );    
    Bitmap.ConvertFormat( xbitmap::FMT_32_ARGB_8888 );
    auxbmp_Load         ( Alpha,  pAlpha );    

    for( s32 y=0; y<Bitmap.GetWidth(); y++ )
    for( s32 x=0; x<Bitmap.GetWidth(); x++ )
    {
        xcolor A, F;

        A = Alpha.GetPixelColor ( x,y );
        F = Bitmap.GetPixelColor( x,y );
        F.A = A.R;
        Bitmap.SetPixelColor(F, x,y);

    }
    auxbmp_ConvertToD3D( Bitmap );
    vram_Register( Bitmap );
}

//==============================================================================

void LoadWithAlphaBump( xbitmap& Bitmap1, xbitmap& Bitmap2, const char* pDiff, const char* pAlpha, const char* pBump )
{
    xbitmap Bump;
    xbitmap Alpha;

    auxbmp_Load         ( Bitmap1, pDiff );    
    auxbmp_Load         ( Bitmap2, pDiff );    
    Bitmap1.ConvertFormat( xbitmap::FMT_32_ARGB_8888 );
    Bitmap2.ConvertFormat( xbitmap::FMT_32_ARGB_8888 );

    auxbmp_Load         ( Alpha,  pAlpha );    
    auxbmp_Load         ( Bump,   pBump  );    

    for( s32 y=0; y<Bitmap.GetWidth(); y++ )
    for( s32 x=0; x<Bitmap.GetWidth(); x++ )
    {
        xcolor A, F, B;

        B = Bump.GetPixelColor ( x,y );
        A = Alpha.GetPixelColor ( x,y );
        A.R = 128;

        f32 I = fMin( 1, (A.R/255.0f) );

        F = Bitmap1.GetPixelColor( x,y );
        F.A = (u8)(I*255* (B.R*(1/255.0f)));
        Bitmap1.SetPixelColor(F, x,y);

        F = Bitmap2.GetPixelColor( x,y );
        F.A = (u8)(I*255* ((255-B.R)*(1/255.0f)));
        Bitmap2.SetPixelColor(F, x,y);
    }
    auxbmp_ConvertToD3D( Bitmap1 );
    auxbmp_ConvertToD3D( Bitmap2 );

    vram_Register( Bitmap1 );
    vram_Register( Bitmap2 );
}

//==============================================================================

void Initialize( void )
{
    eng_Init();

    View.SetXFOV( R_60 );
    View.SetPosition( vector3(654.458f,458.855f,-408.020f) );
    View.LookAtPoint( vector3(  0,  0,  0) );
    View.SetZLimits ( 0.1f, 10000.0f );

    View2 = View;

    eng_GetRes( ScreenW, ScreenH );

    LoadWithAlpha( Bitmap,      "A_cw_fl_Diff_001.tga", "A_cw_fl_Diff_001_alpha.tga" );
    LoadWithAlpha( Specular,    "Specular.tga", "Specular.tga" );
    LoadWithAlphaBump( Bump[0], Bump[1],  "A_cw_fl_Diff_001.tga", "A_cw_fl_Diff_001_alpha.tga", "A_cw_fl_Diff_001_bump.tga" );
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

        if( input_WasPressed( INPUT_KBD_RIGHT) )  SelectMethod = iMin( 8, ++SelectMethod );
        if( input_WasPressed( INPUT_KBD_LEFT ) )  SelectMethod = iMax( 0, --SelectMethod );
        if( input_WasPressed( INPUT_KBD_SPACE ) ) AdditiveMode = !AdditiveMode;

        
    }

    return( TRUE );
}

//==============================================================================
//==============================================================================
//==============================================================================
static IDirect3DTexture8*       s_pTempTexture      = NULL; 
static IDirect3DSurface8*       s_pTempZBuffer      = NULL;
static IDirect3DSurface8*       s_pTempSurface      = NULL;
const s32                       s_TextrueW          = 256;
const s32                       s_TextrueH          = 256;

//==============================================================================

void InitializeRenderToTexture( void )
{
    dxerr Error;

    // make a depth buffer to go with the first texture
    Error = g_pd3dDevice->CreateDepthStencilSurface( s_TextrueW, s_TextrueH,    
                                D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 
                                &s_pTempZBuffer );
    ASSERT( Error == 0 );

    // We sould not need mip mapping for this.
    Error = g_pd3dDevice->CreateTexture( s_TextrueW, s_TextrueH, 1, 
                                D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, 
                                D3DPOOL_DEFAULT, &s_pTempTexture );
    ASSERT( Error == 0 );

    // Get the first and only mipmap 
    Error = s_pTempTexture->GetSurfaceLevel(0, &s_pTempSurface );
    ASSERT( Error == 0 );
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

void CreateSpecularTexture( void )
{
    dxerr               Error;
    D3DVIEWPORT8        OldVP;
    IDirect3DSurface8*  pBackBuffer       = NULL; 
    IDirect3DSurface8*  pZBuffer          = NULL; 

    //
    // Get the main Zbuffer and Back buffer
    //
    g_pd3dDevice->GetRenderTarget       ( &pBackBuffer );
    g_pd3dDevice->GetDepthStencilSurface( &pZBuffer    );

    //
    // Tell the system to render to our texture
    //
    Error = g_pd3dDevice->SetRenderTarget( s_pTempSurface, s_pTempZBuffer);
    ASSERT( Error == 0 );

    //
    // A standard form of clearing the screen and Zbuffer. Here to see
    // what each of the parameters means.
    //
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         0xffffffff, 1.0f, 0L ); 

    //
    // Set the view view port 
    //
    {
        g_pd3dDevice->GetViewport( &OldVP ); 
        D3DVIEWPORT8 vp = { 0, 0, s_TextrueW, s_TextrueH, 0.0f, 1.0f };
        g_pd3dDevice->SetViewport( &vp );
    }

    //
    // Okay now move the cube to be 
    //
    matrix4 L2W;
    L2W.Identity();
    L2W.SetScale( vector3( 1000, 1000, 1000 ) );
    L2W.SetTranslation( View.GetPosition() );
    g_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&L2W );

    //
    // Activate the texture/tutures and set render modes
    //
    g_pd3dDevice->SetTexture( 0, vram_GetSurface( Specular ) );
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    // Make sure not to do anything funsy
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

    //
    // Render the cube
    //
    UTIL_RenderCube();

    //
    // Resotre all modes
    //
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE,  TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
    g_pd3dDevice->SetViewport( &OldVP );

    // Set the main frame back
    Error = g_pd3dDevice->SetRenderTarget( pBackBuffer, pZBuffer );
    ASSERT( Error == 0 );

    // Clear the L2W just in case
    L2W.Identity();
    g_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&L2W );
}


//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

//==============================================================================
struct vertex{ float x,y,z,nx,ny,nz,u,v; };

void Render( void )
{
    struct lovert
    {
        vector3 P;
        vector3 N;
        vector2 BaseUV;
        vector2 UV;
    };

    s32     I=0;
    lovert* pVert = new lovert[ RawMesh.m_nFacets * 3 ];
    ASSERT( pVert );

    //==---------------------------------------------------
    //  GRID, BOX AND MARKER
    //==---------------------------------------------------
    eng_Begin( "GridBoxMarker" );
    {
        draw_ClearL2W();
        draw_Grid( vector3(  -5000,   0,    -5000), 
                   vector3(10000,  0,    0), 
                   vector3(  0,   0, 10000), 
                   xcolor (  0,128,  0), 32 );
    }
    eng_End();

    //==---------------------------------------------------
    // Lets always compute the specular texture for the new method
    //==---------------------------------------------------
    CreateSpecularTexture();  

    //==---------------------------------------------------
    //  Render Skel
    //==---------------------------------------------------
    eng_Begin( "Rect" );

    matrix4 C2T;
    matrix4 ProJM = View.GetW2C();
    matrix4 V2W = View.GetV2W();

    C2T.Identity();
    C2T.SetScale( vector3(0.5, -0.5, 0.5) );
    C2T.SetTranslation( vector3(0.5, 0.5, 1) );
    
    ProJM = C2T * ProJM;

    {
        const char* pTitle = "";

        x_printfxy( 30,0, "%f %f %f\n", V2W.GetTranslation().X, V2W.GetTranslation().Y, V2W.GetTranslation().Z );        

        for( s32 i=0; i<RawMesh.m_nFacets; i++ )
        {
            for( s32 j=0; j<3; j++ )
            {
                lovert&  V = pVert[ I++ ];

                V.BaseUV.X = RawMesh.m_pVertex[ RawMesh.m_pFacet[i].iVertex[j] ].UV[0].X; 
                V.BaseUV.Y = RawMesh.m_pVertex[ RawMesh.m_pFacet[i].iVertex[j] ].UV[0].Y;  
                V.P    = RawMesh.m_pVertex[ RawMesh.m_pFacet[i].iVertex[j] ].Position;
                V.N    = RawMesh.m_pVertex[ RawMesh.m_pFacet[i].iVertex[j] ].Normal[0];
                V.P   *= 10;

                vector3 E =  V2W.GetTranslation() - V.P;

                switch( SelectMethod )
                {
                case 0: 
                    {
                        pTitle = "Faster method";
                        V.UV.X = ( E.X*0.5f  + E.Y      + E.Z*0.2f )/2000.0f + 0.5f;
                        V.UV.Y = ( E.X*0.01f + E.Y*0.5f - E.Z*0.3f )/2000.0f + 0.5f;
                        break;
                    }
                case 1:  
                    {
                        pTitle = "Second chepest method";
                        f32 L = E.LengthSquared();
                        V.UV.X = (E.X*E.X)/L;
                        V.UV.Y = (E.Y*E.Y)/L;
                        break;
                    }
                case 2:  
                    {
                        pTitle = "Variation of Second chepest method";
                        f32 L = E.LengthSquared();

                        V.UV.X =  (E.X*E.Z)/L;
                        V.UV.Y = -(E.Y*E.Z)/L;

                        V.UV.X = V.UV.X + 0.5f;
                        V.UV.Y = V.UV.Y + 0.5f;

                        break;
                    }
                case 3: 
                    {
                        pTitle = "Best of the hack ways";
                        E.Normalize();
                        V.UV.X = (E.Z      + E.X*0.5f + E.Y*0.5f )/2 + 0.5f;
                        V.UV.Y = (E.Y*0.5f + E.X      + E.Z*0.2f )/2 + 0.5f;
                        break;
                    }
                case 4: 
                    {
                        pTitle = "Sphererical mapping";
                        E.Normalize();
                        vector3 R = E.Reflect( V.N );

                        f32 m = 2 * x_sqrt( R.X*R.X + R.Y*R.Y + R.Z*R.Z );
                        V.UV.X = R.X / m + 1/2.0f;
                        V.UV.Y = R.Y / m + 1/2.0f;
                        break;
                    }
                case 5: 
                    {
                        pTitle = "Paravola maping";
                        E.Normalize();
                        vector3 R = E.Reflect( V.N );
                        
                        V.UV.X = (R.X / (2*(1+R.Z))) + 0.5f ;
                        V.UV.Y = (R.Y / (2*(1+R.Z))) + 0.5f ;
                        break;
                    }
                case 6: 
                    {
                        pTitle = "Testing projective method doesn't work very well";
                        V.UV.X = ((E.X+E.Z) / (2*(1+E.Z))) + 0.5f ;
                        V.UV.Y = ((E.Y+E.Z) / (2*(1+E.Z))) + 0.5f ;
                        break;
                    }

                case 7: 
                    {
                        vector4 Vert;

                        pTitle = "Projected CubeMap. A good candidate.";
                        Vert.X = V.P.X;
                        Vert.Y = V.P.Y;
                        Vert.Z = V.P.Z;
                        Vert.W = 1;

                        Vert = ProJM * Vert;

                       // if( Vert.W < 0.001f ) Vert.W = 0.001f;
                        V.UV.X = Vert.X/Vert.W;
                        V.UV.Y = Vert.Y/Vert.W;

                        break;
                    }

                case 8: 
                    {
                        vector4 Vert;

                        pTitle = "Projected CubeMap with faked bump";
                        Vert.X = V.P.X;
                        Vert.Y = V.P.Y;
                        Vert.Z = V.P.Z;
                        Vert.W = 1;

                        Vert = ProJM * Vert;

                       // if( Vert.W < 0.001f ) Vert.W = 0.001f;
                        V.UV.X = Vert.X/Vert.W;
                        V.UV.Y = Vert.Y/Vert.W;
                        break;
                    }

                }
            }
        }

        x_printfxy( 0,8, pTitle );
        x_printfxy( 0,9, (AdditiveMode)?"Additive Mode":"Blended Mode" );
        
    }

    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_ONE );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_ZERO );

    s32 State = 0;
    if( 1 )
    {
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_TEXCOORDINDEX, State );

        if( SelectMethod == 8 )
            g_pd3dDevice->SetTexture          ( State, vram_GetSurface( Bump[0] ) );
        else
            g_pd3dDevice->SetTexture          ( State, vram_GetSurface( Bitmap ) );

        g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );//D3DTOP_MODULATE );
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

        g_pd3dDevice->SetTextureStageState( State, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

//g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
//g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_ALPHAREPLICATE  );

        g_pd3dDevice->SetTextureStageState( State+1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( State+1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
        State++;
    }

    if( 1 )
    {
        if( SelectMethod == 7 || SelectMethod == 8 )
        {
            g_pd3dDevice->SetTexture          ( State, s_pTempTexture );
            g_pd3dDevice->SetTextureStageState( State, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
            g_pd3dDevice->SetTextureStageState( State, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );
        }
        else
        {
            g_pd3dDevice->SetTexture          ( State, vram_GetSurface( Specular ) );
            g_pd3dDevice->SetTextureStageState( State, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP );
            g_pd3dDevice->SetTextureStageState( State, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP );
        }

        g_pd3dDevice->SetTextureStageState( State, D3DTSS_TEXCOORDINDEX, State );

        if( AdditiveMode )
        {
            g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLOROP,   D3DTOP_MODULATEALPHA_ADDCOLOR   );
            g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLORARG1, D3DTA_CURRENT);
            g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        }
        else
        {
            g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLOROP,   D3DTOP_BLENDCURRENTALPHA  );
            g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLORARG2, D3DTA_CURRENT);
        }

        g_pd3dDevice->SetTextureStageState( State, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_ALPHAARG2, D3DTA_CURRENT );

//g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
//g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_ALPHAREPLICATE  );


        g_pd3dDevice->SetTextureStageState( State+1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( State+1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
        State++;
    }

    if( SelectMethod == 7 || SelectMethod == 8 )
    {
        g_pd3dDevice->SetTextureStageState( State-1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3 | D3DTTFF_PROJECTED );
        g_pd3dDevice->SetTextureStageState( State-1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION );

        matrix4 V2W;        
        V2W = View.GetV2C();
        ProJM = C2T * V2W;

        g_pd3dDevice->SetTransform( D3DTS_TEXTURE1, (D3DXMATRIX*)&ProJM );
        //g_pd3dDevice->SetTransform( D3DTS_TEXTURE0, (D3DXMATRIX*)&ProJM );
    }

    if( SelectMethod == 8 )
    {
        //State = 0;
        // Put the alpha in the current register
        g_pd3dDevice->SetTexture          ( State, vram_GetSurface( Bump[1] ) );
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_TEXCOORDINDEX, 0 );

        g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLOROP,   D3DTOP_SELECTARG1   );
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLORARG1, D3DTA_CURRENT);
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLORARG2, D3DTA_TEXTURE);

        g_pd3dDevice->SetTextureStageState( State, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );

        g_pd3dDevice->SetTextureStageState( State+1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( State+1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

        State++;

        // Do the final computation
        g_pd3dDevice->SetTexture          ( State, s_pTempTexture );
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLOROP,   D3DTOP_SELECTARG2 );//D3DTOP_MODULATEALPHA_ADDCOLOR    );
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLORARG1, D3DTA_CURRENT);
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLORARG2, D3DTA_TEXTURE);

//g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
//g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLORARG1, D3DTA_CURRENT | D3DTA_ALPHAREPLICATE  );

        g_pd3dDevice->SetTextureStageState( State, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_ALPHAARG2, D3DTA_CURRENT );

        g_pd3dDevice->SetTextureStageState( State+1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( State+1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

        State++;

        g_pd3dDevice->SetTextureStageState( State-1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3 | D3DTTFF_PROJECTED );
        g_pd3dDevice->SetTextureStageState( State-1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION );


        f32 Factor = 0.95f;
        C2T.Identity();
        C2T.SetScale( vector3(0.5f, -0.5f, 0.45f) );
        C2T.SetTranslation( vector3(0.5f, 0.5f, Factor) );

        matrix4 V2W;        
        V2W = View.GetV2C();
        ProJM = C2T * V2W;

        g_pd3dDevice->SetTransform( D3DTS_TEXTURE3, (D3DXMATRIX*)&ProJM );
        //g_pd3dDevice->SetTransform( D3DTS_TEXTURE0, (D3DXMATRIX*)&ProJM );
        
    }

    if( 0 )
    {   
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_TEXCOORDINDEX, 0 );
        g_pd3dDevice->SetTexture          ( State, vram_GetSurface( Bitmap ) );

        g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLOROP,   D3DTOP_ADD );//MODULATEALPHA_ADDCOLOR   );
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLORARG2, D3DTA_TEXTURE );


//g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
//g_pd3dDevice->SetTextureStageState( State, D3DTSS_COLORARG1, D3DTA_CURRENT | D3DTA_ALPHAREPLICATE  );

        g_pd3dDevice->SetTextureStageState( State, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( State, D3DTSS_ALPHAARG1, D3DTA_CURRENT );

        g_pd3dDevice->SetTextureStageState( State+1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( State+1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
        State++;
    }


    g_pd3dDevice->SetVertexShader( (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX2) );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, RawMesh.m_nFacets, pVert, sizeof(lovert) );


  
    if( 0 )
    {
        draw_Begin( DRAW_SPRITES, DRAW_2D );

        if( SelectMethod == 7 )
        {
            g_pd3dDevice->SetTexture          ( 0, s_pTempTexture );
        }
        else
        {
            draw_SetTexture(Specular);
        }

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        

        draw_Sprite     ( vector3(0,0,0),
                          vector2(100,100),
                          xcolor(0xffffffff) );

        draw_End();
        


        draw_Begin( DRAW_LINES, DRAW_2D | DRAW_NO_ZBUFFER );

        for( s32 i=0; i<((RawMesh.m_nFacets * 3)-1); i++ )
        {
            draw_Vertex( vector3( pVert[i].UV.X*100,   pVert[i].UV.Y*100, 1 ) );
            draw_Vertex( vector3( pVert[i+1].UV.X*100, pVert[i+1].UV.Y*100, 1 ) );
        }

        draw_End();
    }

    eng_End();      

    delete []pVert;
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
    eng_SetBackColor( xcolor(0x98,0x98,0x98,0xff) );

    InitializeRenderToTexture();

    RawMesh.Load( "testObjects.MATX" );
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
