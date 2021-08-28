//==============================================================================
//
//  SubMesh Test
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "rawanim.hpp"
#include "RawMesh.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Render\Render.hpp"
#include "Render\RigidGeom.hpp"
#include "Render\SkinGeom.hpp"
#include "Render\SkinMgr.hpp"
#include "Render\Texture.hpp"
#include "Render\vu1\vu1.hpp"
#include "Auxiliary\Bitmap\aux_Bitmap.hpp"

//==============================================================================
//  STORAGE
//==============================================================================

f32             Frame=0;
rawanim         RawAnim;
rawmesh         RawMesh;
skin_geom*      pSkinGeom  = NULL;
rigid_geom*     pRigidGeom = NULL;
view            View;
random          R;
s32             ScreenW;
s32             ScreenH;

s8              Normals[16384]              PS2_ALIGNMENT(16);
matrix4         L2W[ VU1_MAX_INSTANCES ]    PS2_ALIGNMENT(16);
s32             NumInst = 1;

#ifdef TARGET_PC
    const char*     DataPath = "Data\\";
#endif

xbool UseVU = FALSE;
vector3 LDir( 0.0f, 0.0f, 1.0f );

s32 mask=-1;

xhandle hGeom;

hinst hInst[128];

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
    //eng_SetBackColor( XCOLOR_GREEN );

    eng_GetRes( ScreenW, ScreenH );

    s32 i;

    for( i=0; i<128; i++ ) hInst[i].Handle = HNULL;

    for( i=0; i<16384; i++ ) Normals[i] = -1;
    
    for( i=0; i<VU1_MAX_INSTANCES; i++ )
    {
        const f32 Dist = 200.0f;
    
        vector3 P;
        
        P.X = ((f32)x_rand() / X_RAND_MAX) * Dist;
        P.Z = ((f32)x_rand() / X_RAND_MAX) * Dist;
        P.Y = 0.0f;
        
        f32 S = 0.5f + (((f32)x_rand() / X_RAND_MAX) * 0.5f);
        S = 0.05f;

        radian3 R( R_0, R_0, R_0 );
        R.Yaw = ((f32)x_rand() / X_RAND_MAX) * R_360;

        P.Zero();
        //P.Z += i * 2.0f;
        P.X += i * 2.0f;
    
        L2W[i].Identity();
        L2W[i].Scale( S );
        //L2W[i].Rotate( R );
        L2W[i].Translate( P );
    }
}

//=========================================================================

void Shutdown( void )
{
}

//=========================================================================

void SaveCamera( void )
{
    matrix4 M = View.GetV2W();
    X_FILE* fp;
    
    if( !(fp = x_fopen( "camera.dat", "wb" ))) ASSERT( FALSE );
    x_fwrite( &M, sizeof( M ), 1, fp );
    x_fclose( fp );
    x_DebugMsg( "Camera saved\n" );
}

void LoadCamera( void )
{
    X_FILE* fp;

    matrix4 ViewMat;
    ViewMat.Identity();
    
    if( !(fp = x_fopen( "camera.dat", "rb" ))) return;
    x_fread( &ViewMat, sizeof( ViewMat ), 1, fp );
    x_fclose( fp );

    View.SetV2W( ViewMat );
    x_DebugMsg( "Camera loaded\n" );
}

//=========================================================================

xbool HandleInput( void )
{
    if( input_UpdateState() )
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
        
        if( input_WasPressed( INPUT_KBD_P ) ) mask++;
        if( input_WasPressed( INPUT_KBD_O ) ) mask--;

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

        if( input_IsPressed( INPUT_PS2_BTN_CIRCLE ) ) SaveCamera();

        if( input_WasPressed( INPUT_PS2_BTN_L_RIGHT ) ) mask++;
        if( input_WasPressed( INPUT_PS2_BTN_L_LEFT  ) ) mask--;

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

void StartRender( void )
{
    eng_Begin( "Render" );
    eng_MaximizeViewport( View );
    eng_SetView         ( View, 0 );
    eng_ActivateView    ( 0 );

    {
        draw_ClearL2W();
        draw_Grid( vector3(  0,  0,  0), 
                   vector3(100,  0,  0), 
                   vector3(  0,  0,100), 
                   xcolor (  0,128,  0), 16 );
    }
    eng_End();
}

//==============================================================================

void RenderStrip( vector4* pPos, u16* pCol, s16* pUV, s32 nVerts )
{
    vector3 PosQ[3];
    xcolor  ColQ[3];
    vector2 TexQ[3];
    
    xbool Kick = FALSE;

    draw_Begin( DRAW_TRIANGLES, DRAW_TEXTURED );

    for( s32 j=0; j<nVerts; j++ )
    {
        Kick = !(*(u32*)&pPos[j].W & 0x8000);

        PosQ[0] = PosQ[1];
        PosQ[1] = PosQ[2];
        ColQ[0] = ColQ[1];
        ColQ[1] = ColQ[2];
        TexQ[0] = TexQ[1];
        TexQ[1] = TexQ[2];

        PosQ[2].X = pPos[j].X;
        PosQ[2].Y = pPos[j].Y;
        PosQ[2].Z = pPos[j].Z;

        ColQ[2].R = ((pCol[j] >>  0) & 0x1F) << 3;
        ColQ[2].G = ((pCol[j] >>  5) & 0x1F) << 3;
        ColQ[2].B = ((pCol[j] >> 10) & 0x1F) << 3;
        ColQ[2].A = ((pCol[j] >> 15) & 0x01) << 7;
        
        TexQ[2].X = pUV[j*2+0] / (f32)(1<<12);
        TexQ[2].Y = pUV[j*2+1] / (f32)(1<<12);

        if( Kick )
        {
            draw_Color ( ColQ[0] );
            draw_UV    ( TexQ[0] );
            draw_Vertex( PosQ[0] );
            draw_Color ( ColQ[1] );
            draw_UV    ( TexQ[1] );
            draw_Vertex( PosQ[1] );
            draw_Color ( ColQ[2] );
            draw_UV    ( TexQ[2] );
            draw_Vertex( PosQ[2] );
        }
    }

    draw_End();
}

//==============================================================================

u16 pCol[ 65536 ];

radian xang = R_0;
radian yang = R_0;
radian zang = R_0;

radian xspd = R_90;
radian yspd = R_30;
radian zspd = R_70;

u32 g_Mask = 0xFFFFFFFF;

//==============================================================================

void eng_GetStats(s32 &Count, f32 &CPU, f32 &GS, f32 &IMS, f32 &FPS);

void AppMain( s32, char** )
{
    Initialize();

    for( s32 i=0; i<63356; i++ )
    {
        pCol[i] = x_rand() & 0xFFFF;
        //pCol[i] = 31 | (31<<5) | (31<<10);
        //apCol[i] = 0;
    }

    {
        if( 0 )
        {
            texture* a = new texture;
            a->Include();
        }

        #ifdef TARGET_PS2
        char* Path = "C:\\GameData\\A51\\Release\\PS2";
        #else
        char* Path = "C:\\GameData\\A51\\Release\\PC";
        #endif
        
        fileio File;
        File.Load( xfs( "%s\\JP_Material_Test.rigidgeom", Path ), pRigidGeom );
        //File.Load( xfs( "%s\\Test.rigidgeom", Path ), pRigidGeom );
          
        g_RscMgr.Init();
        g_RscMgr.SetRootDirectory( Path );
        g_RscMgr.SetOnDemandLoading( TRUE );
        
        render_Init();
        
        render_Register( *pRigidGeom, geom_node::RIGID );
        
        for( s32 i=0; i<10; i++ )
        {
            hInst[i] = render_RegisterInstance( *pRigidGeom, pCol + (i*1024) );
        }
    }
    
    LoadCamera();    

    xtimer T;
    T.Start();
    
    #ifdef TARGET_PS2
    s32 Count = 0;
    f32 CPU = 0.0f, IMS = 0.0f, FPS = 0.0f, GS = 0.0f;
    #endif

    L2W[0].Identity();
    L2W[1].Identity();
    L2W[1].Translate( vector3( 50, 0, 0 ) );

    while( TRUE )
    {
        if( !HandleInput() )
            break;
        
        StartRender();

        //=====================================================================
        
        if( TRUE )
        {
            render_Begin( "Geom" );
        
            if( 0 )
            {
                for( s32 i=0; i<9; i+=3 )
                {
                    L2W[i].Identity();
                    L2W[i].Scale( vector3( 0.2f, 0.2f, 0.2f ) );
                    L2W[i].Translate( vector3( i*30.0f, 0.0f, i*10.0f ) );
                
                    u32 Mask = (mask < 0) ? mask : (1 << mask);
                
                    render_AddInstance( *pRigidGeom, hInst[0], &L2W[i], TRUE, Mask, render::WireFrame | render::Normal );
                }
            }
            else
            {
                for( s32 i=0; i<9; i+=3 )
                {
                    L2W[i+0].Identity();
                    L2W[i+1].Identity();
                    L2W[i+2].Identity();
                    
                    L2W[i+0].Scale( vector3( 0.2f, 0.2f, 0.2f ) );
                    L2W[i+1].Scale( vector3( 0.2f, 0.2f, 0.2f ) );
                    L2W[i+2].Scale( vector3( 0.2f, 0.2f, 0.2f ) );
                    
                    L2W[i+0].RotateX( xang * (i * 0.2f) );
                    L2W[i+1].RotateY( yang * (i * 0.2f) );
                    L2W[i+2].RotateZ( zang * (i * 0.2f) );
                    
                    L2W[i+0].Translate( vector3( i*10.0f, 0.0f, i*10.0f ) );
                    L2W[i+1].Translate( vector3( i*10.0f, 0.0f, i*10.0f ) );
                    L2W[i+2].Translate( vector3( i*10.0f, 0.0f, i*10.0f ) );

                    u64 Mask = (mask < 0) ? mask : (1 << mask);
                    
                    render_AddInstance( *pRigidGeom, hInst[i], &L2W[i], TRUE, Mask, render::Normal );
                }
                
                f32 DeltaTime = 1.0f / 300.0f;
                
                xang += xspd * DeltaTime;
                yang += yspd * DeltaTime;
                zang += zspd * DeltaTime;
            }

            render_End();
        }
        
        //=====================================================================

        eng_PageFlip();

        #ifdef TARGET_PS2
        if( T.ReadSec() >= 10.0f )
        {
            T.Reset();
            T.Start();
            eng_GetStats( Count, CPU, GS, IMS, FPS );
            eng_PrintStats();
        }
        #endif
    }

    Shutdown();
}

//==============================================================================

