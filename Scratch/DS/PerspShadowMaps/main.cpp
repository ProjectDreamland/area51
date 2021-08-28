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
#include "RenderMesh.hpp"

//==============================================================================
//  STORAGE
//==============================================================================

// window data
s32                 g_WindowResX = 800;
s32                 g_WindowResY = 800;

// camera and player views
f32                 g_NearZ = 40.0f;
f32                 g_FarZ  = 8000.0f;
view                g_CameraView;
view                g_PlayerView;

// mesh and bitmap data
xhandle             g_FloorHandle;
rawmesh             g_FloorMesh;
xbitmap             g_FloorBMP;
xhandle             g_HazmatHandle;
rawmesh             g_HazmatMesh;
xbitmap             g_HazmatBMP;
xhandle             g_LightIconHandle;
rawmesh             g_LightIconMesh;
xbitmap             g_LightIconBMP;
xhandle             g_GrayHandle;
rawmesh             g_GrayMesh;
xbitmap             g_GrayBMP;
bbox                g_GrayBBox;

// control modes
s32                 g_MoveMode;     // 0 = camera, 1 = player, 2 = light
s32                 g_CameraMode;   // 0 = camera, 1 = player, 2 = light
s32                 g_LightType;    // 0 = point, 1 = directional
s32                 g_ShadowMode;   // 0 = normal, 1 = perspective

// lights
f32                 g_AggressiveZNear;
f32                 g_AggressiveZFar;
f32                 g_LightPitch;
f32                 g_LightYaw;
f32                 g_LightDistance;
vector3             g_LightPosition;
vector3             g_LightDirection;
bbox                g_DirLightBBox;

// grays
struct gray
{
    vector3 Position;
    xbool   CastShadow;
};
static const s32 kNumGrays = 16;
gray    g_Grays[kNumGrays];

// shadow bitmaps
s32                 g_ShadowTexW = 512;
s32                 g_ShadowTexH = 512;
IDirect3DSurface8*  g_pShadowZBuffer     = NULL;
IDirect3DTexture8*  g_pNormalShadTexture = NULL; 
IDirect3DSurface8*  g_pNormalShadSurface = NULL;
IDirect3DTexture8*  g_pPerspShadTexture  = NULL; 
IDirect3DSurface8*  g_pPerspShadSurface  = NULL;
matrix4             g_NormalShadowMapW2V;
matrix4             g_NormalShadowMapV2C;
matrix4             g_PerspShadowMapW2V;
matrix4             g_PerspShadowMapV2C;

// canonical view volume view
IDirect3DTexture8*  g_pClipViewTexture = NULL;
IDirect3DSurface8*  g_pClipViewSurface = NULL;
matrix4             g_ClipViewW2V;
matrix4             g_ClipViewV2C;
radian              g_ClipViewPitch;
radian              g_ClipViewYaw;
f32                 g_ClipViewDistance;

//==============================================================================
// FUNCTIONS
//==============================================================================

void RenderPlayerMesh( xbool ToShadowMap )
{
    matrix4 L2W = g_PlayerView.GetV2W();
    vector3 Pos = L2W.GetTranslation();
    Pos.GetY() -= 180.0f;
    L2W.SetTranslation(Pos);
    radian3 Rot = L2W.GetRotation();
    Rot.Pitch = R_0;
    L2W.SetRotation(Rot);
    if ( ToShadowMap )
    {
        RenderMeshToShadowMap( g_HazmatHandle, g_HazmatBMP, L2W, 0xff808080 );
    }
    else
    {
        RenderMesh( g_HazmatHandle, g_HazmatBMP, L2W, 0xff808080 );
    }
}

//==============================================================================

void RenderGrays( xbool ToShadowMap )
{
    for ( s32 i = 0; i < kNumGrays; i++ )
    {
        matrix4 L2W;
        L2W.Identity();
        L2W.SetTranslation(g_Grays[i].Position);
        if ( ToShadowMap )
            RenderMeshToShadowMap( g_GrayHandle, g_GrayBMP, L2W, 0xffC0C0C0 );
        else
            RenderMesh( g_GrayHandle, g_GrayBMP, L2W, g_Grays[i].CastShadow ? 0xffC0ffC0 : 0xffffffff );
    }
}

//==============================================================================

void InitializeFrameTextures( void )
{
    dxerr Error;

    // make a depth buffer to go with the shadow textures
    Error = g_pd3dDevice->CreateDepthStencilSurface( g_ShadowTexW, g_ShadowTexH,
                                D3DFMT_D24X8, D3DMULTISAMPLE_NONE, 
                                &g_pShadowZBuffer );
    ASSERT( Error == 0 );

    // create the normal shadow texture
    Error = g_pd3dDevice->CreateTexture( g_ShadowTexW, g_ShadowTexH, 1, 
                                D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, 
                                D3DPOOL_DEFAULT, &g_pNormalShadTexture );
    ASSERT( Error == 0 );

    // Get the first and only mipmap 
    Error = g_pNormalShadTexture->GetSurfaceLevel(0, &g_pNormalShadSurface );
    ASSERT( Error == 0 );

    // create the perspective shadow texture
    Error = g_pd3dDevice->CreateTexture( g_ShadowTexW, g_ShadowTexH, 1, 
                                D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, 
                                D3DPOOL_DEFAULT, &g_pPerspShadTexture );
    ASSERT( Error == 0 );

    // Get the first and only mipmap 
    Error = g_pPerspShadTexture->GetSurfaceLevel(0, &g_pPerspShadSurface );
    ASSERT( Error == 0 );

    // create the clip view texture
    Error = g_pd3dDevice->CreateTexture( g_ShadowTexW, g_ShadowTexH, 1,
                                D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
                                D3DPOOL_DEFAULT, &g_pClipViewTexture );
    ASSERT( Error == 0 );

    // Get the first and only mipmap
    Error = g_pClipViewTexture->GetSurfaceLevel( 0, &g_pClipViewSurface );
    ASSERT( Error == 0 );
}

//==============================================================================

void KillFrameTextures( void )
{
    g_pClipViewSurface->Release();
    g_pClipViewTexture->Release();
    g_pPerspShadSurface->Release();
    g_pPerspShadTexture->Release();
    g_pNormalShadSurface->Release();
    g_pNormalShadTexture->Release();
    g_pShadowZBuffer->Release();
}

//==============================================================================

void CalcLightPosition( void )
{
    vector3 LightDir( 0.0f, 0.0f, 1.0f );
    LightDir.RotateX( g_LightPitch );
    LightDir.RotateY( g_LightYaw );
    vector3 LightPos = -LightDir * g_LightDistance;

    // done
    g_LightPosition  = LightPos;
    g_LightDirection = LightDir;
}

//==============================================================================

void Initialize( void )
{
    d3deng_SetResolution(g_WindowResX,g_WindowResY);
    eng_Init();

    // misc. stuff
    g_MoveMode   = 0;
    g_CameraMode = 0;
    g_LightType  = 1;
    g_ShadowMode = 0;

    // initialize the views
    g_CameraView.SetXFOV( R_60 );
    g_CameraView.SetPosition( vector3(1258.0f,688.0f,-143.0f) );
    g_CameraView.LookAtPoint( vector3(  0,  0,  0) );
    g_CameraView.SetZLimits ( g_NearZ, g_FarZ );

    //g_PlayerView.SetPosition( vector3(0.0f, 180.0f, -1000.0f) );
    g_PlayerView.SetPosition( vector3(0.0f, 180.0f, 0.0f) );
    g_PlayerView.SetRotation( radian3(R_0,  R_315,  R_0 ) );
    g_PlayerView.SetXFOV( R_60 );
    g_PlayerView.SetZLimits ( g_NearZ, g_FarZ );

    // load the meshes and textures
    g_HazmatMesh.Load   ( "MIL_Hazmat_MESH.matx" );
    g_LightIconMesh.Load( "Light.matx"           );
    g_FloorMesh.Load    ( "floor.matx"           );
    g_GrayMesh.Load     ( "Gray_Edgar_MESH.matx" );
    auxbmp_LoadNative   ( g_HazmatBMP,    "hazmat_gear.tga"   );
    auxbmp_LoadNative   ( g_LightIconBMP, "light.tga"         );
    auxbmp_LoadNative   ( g_FloorBMP,     "floor.tga"         );
    auxbmp_LoadNative   ( g_GrayBMP,      "gray_diff_000.tga" );
    vram_Register       ( g_HazmatBMP    );
    vram_Register       ( g_LightIconBMP );
    vram_Register       ( g_FloorBMP     );
    vram_Register       ( g_GrayBMP      );
    g_HazmatHandle    = InitializeMeshData(g_HazmatMesh   );
    g_LightIconHandle = InitializeMeshData(g_LightIconMesh);
    g_FloorHandle     = InitializeMeshData(g_FloorMesh    );
    g_GrayHandle      = InitializeMeshData(g_GrayMesh     );
    g_GrayBBox = g_GrayMesh.GetBBox();

    // set up the gray positions
    for ( s32 z = 0; z < 4; z++ )
    {
        for ( s32 x = 0; x < 4; x++ )
        {
            g_Grays[z*4+x].Position.Set( -400.0f+((f32)x/3.0f)*800.0f,
                                        0.0f,
                                        -400.0f+((f32)z/3.0f)*800.0f );
            g_Grays[z*4+x].CastShadow = FALSE;
        }
    }
    for ( s32 iGray = 0; iGray < kNumGrays; iGray++ )
    {
        g_Grays[iGray].Position.Set( x_frand(-400.0f, 400.0f), 0.0f, x_frand(-400.0f, 400.0f) );
        g_Grays[iGray].CastShadow = FALSE;
    }


    // set up a light around the view
    g_AggressiveZNear = 200.0f;
    g_AggressiveZFar  = 1000.0f;
    g_LightPitch      = 1.431f;
    g_LightYaw        = R_180;
    //g_LightPitch      = R_90;
    //g_LightYaw        = R_0;
    //g_LightPitch      = 0.846f;
    //g_LightYaw        = 5.363f;
    g_LightDistance   = 1000.0f;
    CalcLightPosition();

    // initialize the shadow render surfaces
    InitializeFrameTextures();

    g_ClipViewPitch    = R_45;
    g_ClipViewYaw      = R_180;
    g_ClipViewDistance = 5.0f;
}

//=========================================================================

void Shutdown( void )
{
    // unload the meshes and textures
    vram_Unregister( g_GrayBMP      );
    vram_Unregister( g_FloorBMP     );
    vram_Unregister( g_LightIconBMP );
    vram_Unregister( g_HazmatBMP    );
    KillMeshData( g_GrayHandle      );
    KillMeshData( g_FloorHandle     );
    KillMeshData( g_LightIconHandle );
    KillMeshData( g_HazmatHandle    );
    g_GrayMesh.Kill();
    g_FloorMesh.Kill();
    g_LightIconMesh.Kill();
    g_HazmatMesh.Kill();

    // kill the shadow render surfaces
    KillFrameTextures();
}

//=========================================================================

xbool HandleInput( void )
{
    while( input_UpdateState() )
    {
        // Move view using keyboard and mouse
        // WASD - forward, left, back, right
        // RF   - up, down
        radian Pitch;
        radian Yaw;
        f32    S = 16.125f/4;
        f32    R = 0.010f;

        if( input_IsPressed( INPUT_KBD_ESCAPE  ) )  return( FALSE );

        if ( g_MoveMode == 0 )
        {
            if( input_IsPressed( INPUT_MOUSE_BTN_L ) )  S *= 4.0f;
            if( input_IsPressed( INPUT_KBD_W       ) )  g_CameraView.Translate( vector3( 0, 0, S), view::VIEW );
            if( input_IsPressed( INPUT_KBD_S       ) )  g_CameraView.Translate( vector3( 0, 0,-S), view::VIEW );
            if( input_IsPressed( INPUT_KBD_A       ) )  g_CameraView.Translate( vector3( S, 0, 0), view::VIEW );
            if( input_IsPressed( INPUT_KBD_D       ) )  g_CameraView.Translate( vector3(-S, 0, 0), view::VIEW );
            if( input_IsPressed( INPUT_KBD_R       ) )  g_CameraView.Translate( vector3( 0, S, 0), view::VIEW );
            if( input_IsPressed( INPUT_KBD_F       ) )  g_CameraView.Translate( vector3( 0,-S, 0), view::VIEW );

            g_CameraView.GetPitchYaw( Pitch, Yaw );       
            Pitch += input_GetValue( INPUT_MOUSE_Y_REL ) * R;
            Yaw   -= input_GetValue( INPUT_MOUSE_X_REL ) * R;
            g_CameraView.SetRotation( radian3(Pitch,Yaw,0) );
        }
        else if ( g_MoveMode == 1 )
        {
            if( input_IsPressed( INPUT_MOUSE_BTN_L ) )  S *= 4.0f;
            if( input_IsPressed( INPUT_KBD_W       ) )  g_PlayerView.Translate( vector3( 0, 0, S), view::VIEW );
            if( input_IsPressed( INPUT_KBD_S       ) )  g_PlayerView.Translate( vector3( 0, 0,-S), view::VIEW );
            if( input_IsPressed( INPUT_KBD_A       ) )  g_PlayerView.Translate( vector3( S, 0, 0), view::VIEW );
            if( input_IsPressed( INPUT_KBD_D       ) )  g_PlayerView.Translate( vector3(-S, 0, 0), view::VIEW );
            if( input_IsPressed( INPUT_KBD_R       ) )  g_PlayerView.Translate( vector3( 0, S, 0), view::VIEW );
            if( input_IsPressed( INPUT_KBD_F       ) )  g_PlayerView.Translate( vector3( 0,-S, 0), view::VIEW );
            g_PlayerView.GetPitchYaw( Pitch, Yaw );
            Pitch += input_GetValue( INPUT_MOUSE_Y_REL ) * R;
            Yaw   -= input_GetValue( INPUT_MOUSE_X_REL ) * R;
            g_PlayerView.SetRotation( radian3(Pitch,Yaw,0) );

            vector3 PlayerPos = g_PlayerView.GetPosition();
            PlayerPos.GetY() = MAX( PlayerPos.GetY(), 180.0f );
            g_PlayerView.SetPosition(PlayerPos);
        }
        else if ( g_MoveMode == 2 )
        {
            if( input_IsPressed( INPUT_MOUSE_BTN_L ) )  S *= 4.0f;
            if( input_IsPressed( INPUT_KBD_S       ) )  g_LightDistance += S;
            if( input_IsPressed( INPUT_KBD_W       ) )  g_LightDistance -= S;
            g_LightPitch += input_GetValue( INPUT_MOUSE_Y_REL ) * R;
            g_LightYaw   -= input_GetValue( INPUT_MOUSE_X_REL ) * R;

            g_LightPitch = MAX( g_LightPitch, -R_90 );
            g_LightPitch = MIN( g_LightPitch,  R_90 );
            if ( g_LightYaw >= R_360 )
                g_LightYaw -= R_360;
            if ( g_LightYaw < R_0 )
                g_LightYaw += R_360;
            g_LightDistance = MAX( g_LightDistance, 100.0f );
            g_LightDistance = MIN( g_LightDistance, 1000.0f );
        }
        else
        {
            if( input_IsPressed( INPUT_MOUSE_BTN_L ) )  S *= 4.0f;
            if( input_IsPressed( INPUT_KBD_S       ) )  g_ClipViewDistance += S*0.01f;
            if( input_IsPressed( INPUT_KBD_W       ) )  g_ClipViewDistance -= S*0.01f;
            g_ClipViewPitch += input_GetValue( INPUT_MOUSE_Y_REL ) * R;
            g_ClipViewYaw   -= input_GetValue( INPUT_MOUSE_X_REL ) * R;

            g_ClipViewPitch = MAX( g_ClipViewPitch, -R_90 );
            g_ClipViewPitch = MIN( g_ClipViewPitch,  R_90 );
            if ( g_ClipViewYaw >= R_360 )
                g_ClipViewYaw -= R_360;
            if ( g_ClipViewYaw < R_0 )
                g_ClipViewYaw += R_360;
            g_ClipViewDistance = MAX( g_ClipViewDistance, 1.0f );
            g_ClipViewDistance = MIN( g_ClipViewDistance, 20.0f );
        }

        if ( input_WasPressed( INPUT_KBD_SPACE ) )
        {
            g_MoveMode = (g_MoveMode+1)%4;
        }

        if ( input_WasPressed( INPUT_KBD_V ) )
        {
            g_CameraMode = (g_CameraMode+1)%3;
        }

        if ( input_WasPressed( INPUT_KBD_T ) )
            g_LightType = !g_LightType;

        if ( input_WasPressed( INPUT_KBD_Q ) )
            g_ShadowMode = !g_ShadowMode;

        if( input_IsPressed( INPUT_MSG_EXIT ) )
            return( FALSE );
    }

    return( TRUE );
}

//==============================================================================

void CalcOccluders( void )
{
    if ( g_LightType )
    {
        matrix4 W2Light;
        W2Light.Identity();
        W2Light.SetTranslation(g_LightPosition);
        W2Light.SetRotation(radian3(g_LightPitch,g_LightYaw,R_0));
        W2Light.InvertRT();

        /*
        // get the eight corners of the shadow view frustum in light space, and
        // figure out the min, max z values
        s32 X0, X1, Y0, Y1;
        g_PlayerView.GetViewport(X0,Y0,X1,Y1);
        vector3 ViewCorners[8];
        f32 MinZ =  F32_MAX;
        f32 MaxZ = -F32_MAX;
        ViewCorners[0] = g_PlayerView.RayFromScreen( (f32)X0, (f32)Y0, view::VIEW );
        ViewCorners[1] = g_PlayerView.RayFromScreen( (f32)X0, (f32)Y1, view::VIEW );
        ViewCorners[2] = g_PlayerView.RayFromScreen( (f32)X1, (f32)Y1, view::VIEW );
        ViewCorners[3] = g_PlayerView.RayFromScreen( (f32)X1, (f32)Y0, view::VIEW );
        ViewCorners[4] = ViewCorners[0];
        ViewCorners[5] = ViewCorners[1];
        ViewCorners[6] = ViewCorners[2];
        ViewCorners[7] = ViewCorners[3];
        for ( s32 i = 0; i < 8; i++ )
        {
            f32 Dist = (i<4) ? g_AggressiveZNear : g_AggressiveZFar;
            ViewCorners[i] *= Dist / ViewCorners[i].Z;
            ViewCorners[i]  = g_PlayerView.ConvertV2W( ViewCorners[i] );
            ViewCorners[i]  = W2Light * ViewCorners[i];
            MinZ = MIN(MinZ, ViewCorners[i].Z);
            MaxZ = MAX(MaxZ, ViewCorners[i].Z);
        }

        // give the min and max z just a little bit more to be safe
        MinZ -= g_AggressiveZNear;
        MaxZ += g_AggressiveZNear;
        */

/*
        // calculate visibility for each of the grays by extruding their bboxes
        for ( s32 iGray = 0; iGray < kNumGrays; iGray++ )
        {
            for ( s32 Corner = 0; Corner < 8; Corner++ )
            {
                static vector3 Scales[8] =
                {
                    vector3( 0.0f, 0.0f, 0.0f ),
                    vector3( 0.0f, 0.0f, 1.0f ),
                    vector3( 0.0f, 1.0f, 0.0f ),
                    vector3( 0.0f, 1.0f, 1.0f ),
                    vector3( 1.0f, 0.0f, 0.0f ),
                    vector3( 1.0f, 0.0f, 1.0f ),
                    vector3( 1.0f, 1.0f, 0.0f ),
                    vector3( 1.0f, 1.0f, 1.0f )
                }

                // move the corner into light space
                vector3 vCorner;
                vCorner.X = Scales[Corner].X*g_GrayBBox.Min.X + (1.0f-Scales[Corner].X)*g_GrayBBox.Max.X;
                vCorner.Y = Scales[Corner].Y*g_GrayBBox.Min.Y + (1.0f-Scales[Corner].Y)*g_GrayBBox.Max.Y;
                vCorner.Z = Scales[Corner].Z*g_GrayBBox.Min.Z + (1.0f-Scales[Corner].Z)*g_GrayBBox.Max.Z;
                vCorner = W2Light*(g_Grays[i].Position+vCorner);
            }
        }
        */
    }
}

//==============================================================================

void CreateNormalShadowMap( void )
{
    eng_Begin( "NormalShadowMap" );

    // set up the view and projection matrices
    matrix4 OrigW2V;
    matrix4 W2V;
    matrix4 OrigV2C;
    matrix4 V2C;
    if ( g_LightType )
    {
        matrix4 W2Light;
        W2Light.Identity();
        W2Light.SetTranslation(g_LightPosition);
        W2Light.SetRotation(radian3(g_LightPitch,g_LightYaw,R_0));
        W2Light.InvertRT();

        // get the eight courners of the player frustum in "light" space, and
        // add them to our bbox
        s32 X0, X1, Y0, Y1;
        vector3 ViewCorners[8];
        g_PlayerView.GetViewport(X0,Y0,X1,Y1);
        g_DirLightBBox.Clear();
        ViewCorners[0] = g_PlayerView.RayFromScreen( (f32)X0, (f32)Y0, view::VIEW );
        ViewCorners[1] = g_PlayerView.RayFromScreen( (f32)X0, (f32)Y1, view::VIEW );
        ViewCorners[2] = g_PlayerView.RayFromScreen( (f32)X1, (f32)Y1, view::VIEW );
        ViewCorners[3] = g_PlayerView.RayFromScreen( (f32)X1, (f32)Y0, view::VIEW );
        ViewCorners[4] = ViewCorners[0];
        ViewCorners[5] = ViewCorners[1];
        ViewCorners[6] = ViewCorners[2];
        ViewCorners[7] = ViewCorners[3];
        for ( s32 i = 0; i < 8; i++ )
        {
            f32 Dist = (i<4) ? g_AggressiveZNear : g_AggressiveZFar;
            ViewCorners[i] *= Dist / ViewCorners[i].GetZ();
            ViewCorners[i]  = g_PlayerView.ConvertV2W( ViewCorners[i] );
            ViewCorners[i]  = W2Light * ViewCorners[i];
            g_DirLightBBox += ViewCorners[i];
        }

        // move the light so that it sits in the center of the bbox, and
        // the near and far planes for the light match up to the bbox min
        // and max z amounts
        vector3 TransAmount;
        TransAmount.GetX() = -0.5f*(g_DirLightBBox.Min.GetX()+g_DirLightBBox.Max.GetX());
        TransAmount.GetY() = -0.5f*(g_DirLightBBox.Min.GetY()+g_DirLightBBox.Max.GetY());
        TransAmount.GetZ() = -g_DirLightBBox.Min.GetZ() + 10.0f;
        g_DirLightBBox.Translate( TransAmount );
        matrix4 TransMat    = W2Light;
        TransMat.InvertRT();
        vector3 NewLightPos = -TransAmount;
        NewLightPos         = TransMat * NewLightPos;
        W2V.Identity();
        W2V.SetTranslation(NewLightPos);
        W2V.SetRotation(radian3(g_LightPitch,g_LightYaw,R_0));
        W2V.InvertRT();

        V2C.Identity();
        V2C(0,0) = 1.0f/g_DirLightBBox.Max.GetX();
        V2C(1,1) = -1.0f/g_DirLightBBox.Max.GetY();
        V2C(2,2) = 1.0f/(g_DirLightBBox.Max.GetZ()-g_DirLightBBox.Min.GetZ());
        V2C(3,2) = g_DirLightBBox.Min.GetZ()/(g_DirLightBBox.Max.GetZ()-g_DirLightBBox.Min.GetZ());
    }
    else
    {
        //#### TODO:
        ASSERT( FALSE );
        W2V.Identity();
        V2C.Identity();
    }

    // tell d3d about the new matrices
    g_NormalShadowMapW2V = W2V;
    g_pd3dDevice->GetTransform( D3DTS_VIEW, (D3DMATRIX*)&OrigW2V );
    g_pd3dDevice->SetTransform( D3DTS_VIEW, (D3DMATRIX*)&W2V );
    g_NormalShadowMapV2C = V2C;
    g_pd3dDevice->GetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&OrigV2C );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&V2C );
    
    // set the normal shadow map surface as the destination
    IDirect3DSurface8* pOrigBackBuffer;
    IDirect3DSurface8* pOrigZBuffer;
    g_pd3dDevice->GetRenderTarget( &pOrigBackBuffer );
    g_pd3dDevice->GetDepthStencilSurface( &pOrigZBuffer );
    HRESULT Error = g_pd3dDevice->SetRenderTarget( g_pNormalShadSurface, g_pShadowZBuffer );
    ASSERT( Error == 0 );

    // clear the texture
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         0xffffffff, 1.0f, 0L );

    // set up the viewport
    D3DVIEWPORT8 OldVP;
    D3DVIEWPORT8 NewVP = { 1, 1, g_ShadowTexW-2, g_ShadowTexH-2, 0.0f, 1.0f };
    g_pd3dDevice->GetViewport( &OldVP );
    g_pd3dDevice->SetViewport( &NewVP );

    // render the player
    RenderPlayerMesh(TRUE);
    RenderGrays(TRUE);

    // restore the stuff we've likely changed
    g_pd3dDevice->SetViewport( &OldVP );
    g_pd3dDevice->SetRenderTarget( pOrigBackBuffer, pOrigZBuffer );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&OrigV2C );
    g_pd3dDevice->SetTransform( D3DTS_VIEW,       (D3DMATRIX*)&OrigW2V );

    eng_End();
}

//==============================================================================

void CreateLookAtMatrix( matrix4& Dst, const vector3& Eye, const vector3& LookAt, const vector3& UpVector )
{
    // get the direction we are looking (this will be the z-axis)
    vector3 F = LookAt - Eye;
    F.Normalize();

    // find the other axes
    vector3 Up = UpVector;
    Up.Normalize();
    vector3 S  = Up.Cross(F);
    S.Normalize();
    vector3 U  = F.Cross(S);
    U.Normalize();

    // now we can fill in the matrix
    Dst.Identity();
    Dst.SetRows(S,U,F);
    Dst.PreTranslate(-Eye);
}

//==============================================================================

void CreatePerspShadowMap( void )
{
    eng_Begin( "PerspShadowMap" );

    // set up the view and projection matrices
    matrix4 OrigW2V;
    matrix4 W2V;
    matrix4 OrigV2C;
    matrix4 V2C;
    if ( g_LightType )
    {
        // build a view that matches the normal player view, but has much
        // more aggressive z limits so we won't have to try shadowing out
        // too far
        view    PlayerView = g_PlayerView;
        PlayerView.SetZLimits( g_AggressiveZNear, g_AggressiveZFar );

        // build a light position that gives your light direction if you
        // were looking at the player
        vector3 PlayerPos = PlayerView.GetV2W().GetTranslation();
        vector3 LightPos  = PlayerPos - g_LightDirection;
        
        // transform the light into the player's camera space
        LightPos = PlayerView.GetW2V() * LightPos;

        // Transform the light into canonical [-1,1] view. This should give us
        // a point light on the infinity plane. Or said differently, this should
        // give us the vanishing point of the parallel light lines. Normally,
        // you pass in (x,y,z,1) to this matrix, but since we're wanting the
        // this point on the infinity plane, we use (x,y,z,0)
        matrix4 P2C;
        P2C.Identity();
        f32 W = (f32)(1.0f / x_tan(PlayerView.GetXFOV()*0.5f));
        f32 H = (f32)(1.0f / x_tan(PlayerView.GetYFOV()*0.5f));
        P2C(0,0) = -W;
        P2C(1,1) =  H;
        P2C(2,2) = (g_AggressiveZFar+g_AggressiveZNear)/(g_AggressiveZFar-g_AggressiveZNear);
        P2C(3,2) = (-2.0f*g_AggressiveZNear*g_AggressiveZFar)/(g_AggressiveZFar-g_AggressiveZNear);
        P2C(2,3) = 1.0f;
        vector4 LightPos4( LightPos.GetX(), LightPos.GetY(), LightPos.GetZ(), 0.0f );
        if ( (LightPos4.GetZ()>-0.001f) && (LightPos4.GetZ()<0.001f) )
        {
            // the light direction lies in the XY plane which doesn't have
            // a vanishing point in the canonical view space (i.e. it doesn't
            // have a point on the infinity plane)
            // we'll make a point that is roughly in the right direction, and
            // is very far away. this should be "good enough"
            f32 InfZ    = (g_AggressiveZFar+g_AggressiveZNear)/(g_AggressiveZFar-g_AggressiveZNear);
            //f32 InfZ    = g_AggressiveZFar/(g_AggressiveZFar-g_AggressiveZNear);
            LightPos4   = P2C * LightPos4;
            LightPos4.GetZ() = 0.0f;
            LightPos4.GetW() = 0.0f;
            LightPos4.Normalize();
            LightPos4.GetX() = LightPos4.GetX() * 1000.0f;
            LightPos4.GetY() = LightPos4.GetY() * 1000.0f;
            LightPos4.GetZ() = InfZ;
        }
        else
        {
            LightPos4     = P2C * LightPos4;
            f32 OneOverW  = 1.0f / LightPos4.GetW();
            LightPos4.GetX()  *= OneOverW;
            LightPos4.GetY()  *= OneOverW;
            LightPos4.GetZ()  *= OneOverW;
            LightPos4.GetW()  *= OneOverW;
        }
        LightPos.Set(LightPos4.GetX(),LightPos4.GetY(),LightPos4.GetZ());
    
        // now create a view that goes from the player's canonical view volume
        // to our new point light
        view    LightView;
        matrix4 PlayerClipToLight;
        vector3 Center(0.0f,0.0f,0.0f);
        CreateLookAtMatrix(PlayerClipToLight, LightPos, Center, vector3(0.0f,0.0f,1.0f) );
        matrix4 PlayerLightToClip = PlayerClipToLight;
        PlayerLightToClip.Invert();
        LightView.SetV2W(PlayerLightToClip);

        // figure out some limits for our point lights and set those into the view
        static const f32 kRadius = 1.0f;
        f32 Dist      = (LightPos-Center).Length();
        f32 FOV       = 2.0f * asinf(kRadius/Dist);
        f32 NearPlane = Dist-kRadius*2.0f;
        f32 FarPlane  = Dist+kRadius*2.0f;
        if ( NearPlane < 0.001f )
            NearPlane = 0.001f;
        LightView.SetViewport(0,0,g_ShadowTexW,g_ShadowTexH);
        LightView.SetXFOV(FOV);
        LightView.SetZLimits(NearPlane,FarPlane);

        x_printfxy( 0, 10, "near=%3.3f far=%3.3f fov=%3.3f", NearPlane, FarPlane, RAD_TO_DEG(FOV) );

        // now we can build the final matrices
        W2V = PlayerView.GetW2V();
        V2C = LightView.GetV2C() * LightView.GetW2V() * P2C;
    }
    else
    {
        //#### TODO:
        ASSERT( FALSE );
        W2V.Identity();
        V2C.Identity();
    }

    // tell d3d about the new matrices
    g_PerspShadowMapW2V = W2V;
    g_pd3dDevice->GetTransform( D3DTS_VIEW, (D3DMATRIX*)&OrigW2V );
    g_pd3dDevice->SetTransform( D3DTS_VIEW, (D3DMATRIX*)&W2V );
    g_PerspShadowMapV2C = V2C;
    g_pd3dDevice->GetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&OrigV2C );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&V2C );
    
    // set the persp shadow map surface as the destination
    IDirect3DSurface8* pOrigBackBuffer;
    IDirect3DSurface8* pOrigZBuffer;
    g_pd3dDevice->GetRenderTarget( &pOrigBackBuffer );
    g_pd3dDevice->GetDepthStencilSurface( &pOrigZBuffer );
    HRESULT Error = g_pd3dDevice->SetRenderTarget( g_pPerspShadSurface, g_pShadowZBuffer );
    ASSERT( Error == 0 );

    // clear the texture
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         0xffffffff, 1.0f, 0L );

    // set up the viewport
    D3DVIEWPORT8 OldVP;
    D3DVIEWPORT8 NewVP = { 1, 1, g_ShadowTexW-2, g_ShadowTexH-2, 0.0f, 1.0f };
    g_pd3dDevice->GetViewport( &OldVP );
    g_pd3dDevice->SetViewport( &NewVP );

    // render the player
    //####
    //RenderPlayerMesh(TRUE);
    RenderGrays(TRUE);

    // restore the stuff we've likely changed
    g_pd3dDevice->SetViewport( &OldVP );
    g_pd3dDevice->SetRenderTarget( pOrigBackBuffer, pOrigZBuffer );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&OrigV2C );
    g_pd3dDevice->SetTransform( D3DTS_VIEW,       (D3DMATRIX*)&OrigW2V );

    eng_End();
}

//==============================================================================

void DrawBBox( const bbox& BBox, xcolor C )
{
    struct bbox_vert
    {
        vector3 P;
        DWORD   C;
    };

    static bbox_vert BBoxVerts[24];

    for ( s32 i = 0; i < 24; i++ )
        BBoxVerts[i].C = C;

    BBoxVerts[ 0].P.Set(BBox.Min.GetX(), BBox.Min.GetY(), BBox.Min.GetZ());
    BBoxVerts[ 1].P.Set(BBox.Min.GetX(), BBox.Min.GetY(), BBox.Max.GetZ());
    BBoxVerts[ 2].P.Set(BBox.Min.GetX(), BBox.Min.GetY(), BBox.Min.GetZ());
    BBoxVerts[ 3].P.Set(BBox.Min.GetX(), BBox.Max.GetY(), BBox.Min.GetZ());
    BBoxVerts[ 4].P.Set(BBox.Min.GetX(), BBox.Min.GetY(), BBox.Min.GetZ());
    BBoxVerts[ 5].P.Set(BBox.Max.GetX(), BBox.Min.GetY(), BBox.Min.GetZ());
    
    BBoxVerts[ 6].P.Set(BBox.Max.GetX(), BBox.Max.GetY(), BBox.Min.GetZ());
    BBoxVerts[ 7].P.Set(BBox.Min.GetX(), BBox.Max.GetY(), BBox.Min.GetZ());
    BBoxVerts[ 8].P.Set(BBox.Max.GetX(), BBox.Max.GetY(), BBox.Min.GetZ());
    BBoxVerts[ 9].P.Set(BBox.Max.GetX(), BBox.Min.GetY(), BBox.Min.GetZ());
    BBoxVerts[10].P.Set(BBox.Max.GetX(), BBox.Max.GetY(), BBox.Min.GetZ());
    BBoxVerts[11].P.Set(BBox.Max.GetX(), BBox.Max.GetY(), BBox.Max.GetZ());
    
    BBoxVerts[12].P.Set(BBox.Max.GetX(), BBox.Min.GetY(), BBox.Max.GetZ());
    BBoxVerts[13].P.Set(BBox.Max.GetX(), BBox.Max.GetY(), BBox.Max.GetZ());
    BBoxVerts[14].P.Set(BBox.Max.GetX(), BBox.Min.GetY(), BBox.Max.GetZ());
    BBoxVerts[15].P.Set(BBox.Max.GetX(), BBox.Min.GetY(), BBox.Min.GetZ());
    BBoxVerts[16].P.Set(BBox.Max.GetX(), BBox.Min.GetY(), BBox.Max.GetZ());
    BBoxVerts[17].P.Set(BBox.Min.GetX(), BBox.Min.GetY(), BBox.Max.GetZ());
    
    BBoxVerts[18].P.Set(BBox.Min.GetX(), BBox.Max.GetY(), BBox.Max.GetZ());
    BBoxVerts[19].P.Set(BBox.Min.GetX(), BBox.Max.GetY(), BBox.Min.GetZ());
    BBoxVerts[20].P.Set(BBox.Min.GetX(), BBox.Max.GetY(), BBox.Max.GetZ());
    BBoxVerts[21].P.Set(BBox.Min.GetX(), BBox.Min.GetY(), BBox.Max.GetZ());
    BBoxVerts[22].P.Set(BBox.Min.GetX(), BBox.Max.GetY(), BBox.Max.GetZ());
    BBoxVerts[23].P.Set(BBox.Max.GetX(), BBox.Max.GetY(), BBox.Max.GetZ());

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );

    g_pd3dDevice->SetVertexShader( (D3DFVF_XYZ|D3DFVF_DIFFUSE) );
    g_pd3dDevice->DrawPrimitiveUP( D3DPT_LINELIST, 12, BBoxVerts, sizeof(bbox_vert) );
}

//==============================================================================

void CreateClipViewMap( void )
{
    eng_Begin( "Clip View Map" );

    // set up the view and projection matrices
    matrix4 OrigW2V;
    matrix4 W2V;
    matrix4 OrigV2C;
    matrix4 V2C;

    // build a view that matches the normal player view, but uses
    // the much more aggressive shadow z limits
    view    PlayerView = g_PlayerView;
    PlayerView.SetZLimits( g_AggressiveZNear, g_AggressiveZFar );

    // create a [-1..1] volume that is the player's view volume
    matrix4 P2C;
    P2C.Identity();
    f32 W = (f32)(1.0f / x_tan(PlayerView.GetXFOV()*0.5f));
    f32 H = (f32)(1.0f / x_tan(PlayerView.GetYFOV()*0.5f));
    P2C(0,0) = -W;
    P2C(1,1) =  H;
    P2C(2,2) = (g_AggressiveZFar+g_AggressiveZNear)/(g_AggressiveZFar-g_AggressiveZNear);
    P2C(3,2) = (-2.0f*g_AggressiveZNear*g_AggressiveZFar)/(g_AggressiveZFar-g_AggressiveZNear);
    P2C(2,3) = 1.0f;
    //P2C(3,3) = 0.0f;

    // now create a view that goes from the player's canonical view volume to
    // our viewing position
    view    FinalView;
    matrix4 PlayerClipToView;
    vector3 Center(0.0f,0.0f,0.0f);
    vector3 ViewPos(0.0f,0.0f,g_ClipViewDistance);
    ViewPos.RotateX(g_ClipViewPitch);
    ViewPos.RotateY(g_ClipViewYaw);
    CreateLookAtMatrix(PlayerClipToView,ViewPos,Center,vector3(0.0f,1.0f,0.0f));
    matrix4 PlayerViewToClip = PlayerClipToView;
    PlayerViewToClip.Invert();
    FinalView.SetV2W(PlayerViewToClip);
    FinalView.SetViewport(0,0,g_ShadowTexW,g_ShadowTexH);
    FinalView.SetXFOV(R_45);
    FinalView.SetZLimits( 0.2f, 20.0f );
    matrix4 FinalViewV2C = FinalView.GetV2C();
    matrix4 FinalViewW2V = FinalView.GetW2V();
    W2V = PlayerView.GetW2V();
    V2C = FinalViewV2C * FinalViewW2V* P2C;

    // tell d3d about the new matrices
    g_ClipViewW2V = W2V;
    g_pd3dDevice->GetTransform( D3DTS_VIEW, (D3DMATRIX*)&OrigW2V );
    g_pd3dDevice->SetTransform( D3DTS_VIEW, (D3DMATRIX*)&W2V );
    g_ClipViewV2C = V2C;
    g_pd3dDevice->GetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&OrigV2C );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&V2C );
    
    // set the clip view map surface as the destination
    IDirect3DSurface8* pOrigBackBuffer;
    IDirect3DSurface8* pOrigZBuffer;
    g_pd3dDevice->GetRenderTarget( &pOrigBackBuffer );
    g_pd3dDevice->GetDepthStencilSurface( &pOrigZBuffer );
    HRESULT Error = g_pd3dDevice->SetRenderTarget( g_pClipViewSurface, g_pShadowZBuffer );
    ASSERT( Error == 0 );

    // clear the texture
    g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         0x00000000, 1.0f, 0L );

    // set up the viewport
    D3DVIEWPORT8 OldVP;
    D3DVIEWPORT8 NewVP = { 1, 1, g_ShadowTexW-2, g_ShadowTexH-2, 0.0f, 1.0f };
    g_pd3dDevice->GetViewport( &OldVP );
    g_pd3dDevice->SetViewport( &NewVP );

    // render everything
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
    matrix4 FloorL2W;
    FloorL2W.Identity();
    RenderMesh( g_FloorHandle, g_FloorBMP, FloorL2W, 0xff808080 );
    //RenderPlayerMesh(FALSE);
    RenderGrays(FALSE);

    // draw markers along the player's z pos
    matrix4 Ident;
    Ident.Identity();
    g_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&Ident);
    vector3 ZAxis( PlayerView.GetV2W()(2,0),
                   PlayerView.GetV2W()(2,1),
                   PlayerView.GetV2W()(2,2) );
    ZAxis.Normalize();
    for ( s32 i = 0; i < 50; i++ )
    {
        f32 T     = ((f32)i/19.0f);
        f32 ZDist = g_AggressiveZNear+T*(g_AggressiveZFar-g_AggressiveZNear);
        vector3 MarkerPoint = PlayerView.GetV2W().GetTranslation() + ZDist*ZAxis;
        bbox    MarkerBBox;
        MarkerBBox.Min = MarkerPoint-vector3(5.0f,5.0f,5.0f);
        MarkerBBox.Max = MarkerPoint+vector3(5.0f,5.0f,5.0f);
        DrawBBox(MarkerBBox,XCOLOR_WHITE);
    }

    // render a bbox showing the view volume
    W2V = FinalViewW2V;
    V2C = FinalViewV2C;
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&V2C );
    g_pd3dDevice->SetTransform( D3DTS_VIEW,       (D3DMATRIX*)&W2V );
    bbox CubeBBox;
    CubeBBox.Min.Set(-1.0f,-1.0f,-1.0f);
    CubeBBox.Max.Set( 1.0f, 1.0f, 1.0f);
    g_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&Ident);
    DrawBBox(CubeBBox,XCOLOR_GREEN);

    // restore the stuff we've likely changed
    g_pd3dDevice->SetViewport( &OldVP );
    g_pd3dDevice->SetRenderTarget( pOrigBackBuffer, pOrigZBuffer );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&OrigV2C );
    g_pd3dDevice->SetTransform( D3DTS_VIEW,       (D3DMATRIX*)&OrigW2V );

    eng_End();
}

//==============================================================================

void Render( void )
{
    // text
    if ( g_MoveMode == 0   )        x_printfxy( 0, 0, "MoveMode=Camera"        );
    else if ( g_MoveMode == 1 )     x_printfxy( 0, 0, "MoveMode=Player"        );
    else if ( g_MoveMode == 2 )     x_printfxy( 0, 0, "MoveMode=Light"         );
    else                            x_printfxy( 0, 0, "MoveMode=ClipView"      );
    if ( g_CameraMode == 0 )        x_printfxy( 0, 1, "CameraMode=Camera"      );
    else if ( g_CameraMode == 1 )   x_printfxy( 0, 1, "CameraMode=Player"      );
    else                            x_printfxy( 0, 1, "CameraMode=Light"       );
    if ( g_LightType == 0 )         x_printfxy( 0, 2, "LightType=Point"        );
    else                            x_printfxy( 0, 2, "LightType=Directional"  );
    if ( g_ShadowMode == 0 )        x_printfxy( 0, 3, "ShadowMode=Normal"      );
    else                            x_printfxy( 0, 3, "ShadowMode=Perspective" );
    vector3 CameraPos = g_CameraView.GetV2W().GetTranslation();
    vector3 PlayerPos = g_PlayerView.GetV2W().GetTranslation();
    x_printfxy( 0, 4, "CameraPos=%3.3f,%3.3f,%3.3f", CameraPos.GetX(), CameraPos.GetY(), CameraPos.GetZ() );
    x_printfxy( 0, 5, "PlayerPos=%3.3f,%3.3f,%3.3f", PlayerPos.GetX(), PlayerPos.GetY(), PlayerPos.GetZ() );
    x_printfxy( 0, 6, "LightPitch=%3.3f", g_LightPitch );
    x_printfxy( 0, 7, "LightYaw=%3.3f", g_LightYaw );

    // re-calculate which meshes need to cast shadows
    CalcOccluders();

    // create the shadow textures
    CreateNormalShadowMap();
    CreatePerspShadowMap();
    CreateClipViewMap();

    // draw the floor
    {
        eng_Begin( "Floor" );
        matrix4 FloorL2W;
        FloorL2W.Identity();
        matrix4 ShadowProj;
        if ( g_ShadowMode == 0 )
        {
            ShadowProj  = g_NormalShadowMapV2C;
            ShadowProj *= g_NormalShadowMapW2V;
            ShadowProj *= eng_GetView()->GetV2W();
            ShadowProj.Scale(vector3( 0.5f, -0.5f, 1.0f ) );
            ShadowProj.Translate(vector3( 0.5f, 0.5f, 0.0f) );        
            RenderMeshWithShadow( g_FloorHandle, g_FloorBMP, g_pNormalShadTexture, FloorL2W, ShadowProj, FALSE, g_LightType?FALSE:TRUE );
        }
        else
        {
            matrix4 ScaleTrans;
            ScaleTrans.Identity();
            ScaleTrans.SetScale( vector3(0.5f,-0.5f,1.0f) );
            ScaleTrans.SetTranslation( vector3(0.5f,0.5f,0.0f) );

            ShadowProj = ScaleTrans *
                         g_PerspShadowMapV2C *
                         g_PerspShadowMapW2V *
                         eng_GetView()->GetV2W();
            RenderMeshWithShadow( g_FloorHandle, g_FloorBMP, g_pPerspShadTexture, FloorL2W, ShadowProj, TRUE, TRUE );
        }
        eng_End();
    }

    // render the lights
    {
        if ( g_CameraMode != 2 )
        {
            eng_Begin( "Lights" );

            // draw a light icon where the light is
            matrix4 LightL2W;
            LightL2W.Identity();
            LightL2W.SetTranslation(g_LightPosition);
            RenderMesh( g_LightIconHandle, g_LightIconBMP, LightL2W, 0xffffffff );

            // draw a line indicating the light's direction
            vector3 EndPoint = g_LightPosition + (100.0f*g_LightDirection);
            draw_ClearL2W();
            draw_Line( g_LightPosition, EndPoint, xcolor(255,255,255) );

            // if it's a dir light, draw the light's bbox
            if ( g_LightType )
            {
                matrix4 L2W = g_NormalShadowMapW2V;
                L2W.InvertRT();
                draw_SetL2W(L2W);
                draw_BBox( g_DirLightBBox );
                draw_Marker(L2W.GetTranslation(), xcolor(128,255,128));
            }

            eng_End();
        }
    }

    // draw the player's frustum and mesh
    {
        if ( (g_CameraMode == 0) ||
             (g_CameraMode == 2) )
        {
            eng_Begin( "Player" );
            draw_ClearL2W();
            draw_Frustum( g_PlayerView, xcolor(128,0,0), g_AggressiveZFar );
            RenderPlayerMesh(FALSE);
            eng_End();
        }
    }

    // if we're in the light view, then look-at marker that the light is rotating around
    {
        if ( g_CameraMode == 2 )
        {
            vector3 LookAtPos = g_PlayerView.GetV2W().GetTranslation();

            eng_Begin( "LookAtMarker" );
            draw_Marker( LookAtPos, xcolor(128,255,128) );
            eng_End();
        }
    }

    // draw the grays
    {
        eng_Begin("Grays");
        RenderGrays(FALSE);
        eng_End();
    }

    // render the shadow maps to show what we've created
    {
        eng_Begin("ShadowMapDisplay");
        draw_Begin( DRAW_SPRITES, DRAW_2D );
        g_pd3dDevice->SetTexture( 0, g_pNormalShadTexture );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        
        draw_Sprite     ( vector3(0.0f,g_WindowResY-200.0f,0.0f),
                          vector2(200.0f,200.0f),
                          xcolor(0xffffffff) );
        draw_End();
        
        draw_Begin( DRAW_SPRITES, DRAW_2D );
        g_pd3dDevice->SetTexture( 0, g_pPerspShadTexture );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        
        draw_Sprite     ( vector3(g_WindowResX-200.0f,g_WindowResY-200.0f,0.0f),
                          vector2(200.0f,200.0f),
                          xcolor(0xffffffff) );
        draw_End();

        draw_Begin( DRAW_SPRITES, DRAW_2D );
        g_pd3dDevice->SetTexture( 0, g_pClipViewTexture );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );        
        draw_Sprite     ( vector3(g_WindowResX-300.0f,0.0f,0.0f),
                          vector2(300.0f,300.0f),
                          xcolor(0xffffffff) );
        draw_End();

        eng_End();
    }
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

    while( TRUE )
    {
        if( !HandleInput() )
            break;

        CalcLightPosition();

        eng_MaximizeViewport( g_PlayerView );
        eng_MaximizeViewport( g_CameraView );
        if ( g_CameraMode == 0 )
        {
            eng_SetView         ( g_CameraView );
        }
        else if ( g_CameraMode == 1 )
        {
            eng_SetView         ( g_PlayerView );
        }
        else
        {
            view LightView;
            eng_MaximizeViewport( LightView );
            LightView.SetZLimits( g_NearZ, g_FarZ );
            LightView.SetXFOV( R_60 );
            LightView.SetPosition( g_LightPosition );
            LightView.LookAtPoint( g_LightPosition, g_LightPosition+g_LightDirection );
            eng_SetView         ( LightView );
        }

        g_pd3dDevice->SetTransform( D3DTS_VIEW, (D3DMATRIX*)&eng_GetView()->GetW2V() );
        g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&eng_GetView()->GetV2C() );

        Advance( Timer.TripSec() );

        Render();

    
        // DONE!
        eng_PageFlip();
    }

    Shutdown();
}

//==============================================================================
