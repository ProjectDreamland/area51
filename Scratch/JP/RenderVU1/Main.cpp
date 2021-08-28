//==============================================================================
//
//  VU1 Test
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "Render\SkinGeom.hpp"
#include "Render\RigidGeom.hpp"
#include "rawanim.hpp"
#include "RawMesh.hpp"
#include "Geometry\vu1\vu1.hpp"
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
s32             NumInst = 1;    //104;

xbitmap Bitmap;

#ifdef TARGET_PC
    const char*     DataPath = "Data\\";
#endif

xbool UseVU = FALSE;
vector3 LDir( 0.0f, 0.0f, 1.0f );

s32 List = 1;

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
        f32    S = 0.825f;
        f32    R = 0.005f;

        if( input_IsPressed( INPUT_KBD_ESCAPE  ) )  return( FALSE );

        if( input_IsPressed( INPUT_MOUSE_BTN_R ) )  S *= 4.0f;
        if( input_IsPressed( INPUT_KBD_W       ) )  View.Translate( vector3( 0, 0, S), view::VIEW );
        if( input_IsPressed( INPUT_KBD_S       ) )  View.Translate( vector3( 0, 0,-S), view::VIEW );
        if( input_IsPressed( INPUT_KBD_A       ) )  View.Translate( vector3( S, 0, 0), view::VIEW );
        if( input_IsPressed( INPUT_KBD_D       ) )  View.Translate( vector3(-S, 0, 0), view::VIEW );
        if( input_IsPressed( INPUT_KBD_R       ) )  View.Translate( vector3( 0, S, 0), view::VIEW );
        if( input_IsPressed( INPUT_KBD_F       ) )  View.Translate( vector3( 0,-S, 0), view::VIEW );
        
        if( input_WasPressed( INPUT_KBD_P ) ) List++;
        if( input_WasPressed( INPUT_KBD_O ) ) List--;

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
    eng_SetView         ( View, 0 );
    eng_ActivateView    ( 0 );

    {
        draw_ClearL2W();
        draw_Grid( vector3(  0,  0,  0), 
                   vector3(100,  0,  0), 
                   vector3(  0,  0,100), 
                   xcolor (  0,128,  0), 16 );
    }
}

//==============================================================================

void RenderStrip( vector4* pPos, u16* pCol, s16* pUV, s32 nVerts )
{
    vector3 PosQ[3];
    xcolor  ColQ[3];
    vector2 TexQ[3];
    
    xbool Kick = FALSE;

    draw_Begin( DRAW_TRIANGLES, DRAW_TEXTURED );
    draw_SetTexture( Bitmap );

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

xbool instanced = TRUE;
xbool anim      = FALSE;
xbool clipped   = TRUE;

#ifdef TARGET_PS2

void RenderSkinPS2( vector3& LightDirection )
{
    s32      i,j;
    //s32      nStrips = 0;
    matrix4* pL2W    = (matrix4*)smem_BufferAlloc( sizeof(matrix4) * RawAnim.m_nBones );
    vector3  LDir    = -LightDirection;

    // Compute anim frame
    RawAnim.ComputeBonesL2W( pL2W, Frame );

    if( anim )
        Frame += 1;

    vram_Activate( Bitmap );
    vu1_Init( View.GetW2S(), View.GetW2C(), View.GetC2S() );
    vu1_Begin();

    // Compute the new position for the verts
    for( i=0; i<pSkinGeom->m_nDList; i++ )
    {
        skin_geom::dlist_ps2& DList         = pSkinGeom->m_DListPtr.pPS2[i];
        s32                   ReindexCur    = 0;

        // Allocate a new place for the vert position
        vector4*    pPos    = (vector4*)smem_BufferAlloc( sizeof(vector4) * DList.nUVs );
        u16*        pCol    = (u16*)    smem_BufferAlloc ( sizeof(xcolor) * DList.nUVs );
        ASSERT(pCol && pPos);

        // Start processing verts
        for( j=0; j<DList.nPosNorm; j++ )
        {
            skin_geom::posnom_ps2& PN      = DList.pPosNorm[j];

            vector3                NewPos  = PN.W1 * ( pL2W[ DList.pBone[j*2+0] ] * PN.Pos ) +
                                             PN.W2 * ( pL2W[ DList.pBone[j*2+1] ] * PN.Pos );
            vector3                NewNorm = pL2W[ DList.pBone[j*2+0] ].RotateVector( PN.Normal );
            s32                    I       = (s32)(fMin( 1, 0.3f + fMax( 0, NewNorm.Dot( LDir ) )) * 255);
            
            //
            // CPU Code ( Copy verts to final location )
            //
            for( s32 k=DList.pReIndex[ReindexCur++]; k>0; k--, ReindexCur++ )
            {
                ASSERT( ReindexCur <= DList.nReIndex );

                // This means that we must set the ADC bits here
                if( DList.pReIndex[ReindexCur] & (1<<15) )
                {
                    s32 Index = DList.pReIndex[ReindexCur] & (u16)~(1<<15);

                    pPos[Index] = vector4( NewPos.X, NewPos.Y, NewPos.Z, 1 ); // | ADC bit here
                    pCol[Index] = (I>>3) | ((I>>3)<<5) | ((I>>3)<<10);
                    pCol[Index] = 16 | (16<<5) | (16<<10);
                    *(u32*)&pPos[Index].W |= 0x8000;
                }
                else
                {
                    s32 Index = DList.pReIndex[ReindexCur];

                    pPos[Index] = vector4( NewPos.X, NewPos.Y, NewPos.Z, 1 ); // NO ADC bit here
                    pCol[Index] = (I>>3) | ((I>>3)<<5) | ((I>>3)<<10);
                    pCol[Index] = 16 | (16<<5) | (16<<10);
                }
            }
        }

        //
        // Render display list ( KICK IT OFF )
        //

        if( UseVU )
        {
            if( instanced )
            {
                vu1_BeginInstance( pPos, (s16*)DList.pUV, (s8*)Normals, DList.nUVs );
                
                for( s32 i=0; i<NumInst; i++ )
                {
                    vu1_AddInstance( pCol, L2W[i], (clipped ? VU1_INSTANCE_CLIPPED : 0) );
                }
                
                vu1_RenderInstances();
                vu1_EndInstance();
            }
            else
            {
                for( s32 i=0; i<NumInst; i++ )
                {
                    //vu1_Render( pPos, pCol, (s16*)DList.pUV, (s8*)Normals, DList.nUVs, L2W[i] );
                }
            }
        }
        else
        {
        
            RenderStrip( pPos, pCol, (s16*)DList.pUV, DList.nUVs );
        }
    }
    
    vu1_End();
}

#endif

//==============================================================================

#ifdef TARGET_PC

void RenderSkinPC( vector3& LightDirection )
{
    s32      i;
    matrix4* pL2W    = (matrix4*)smem_BufferAlloc( sizeof(matrix4) * RawAnim.m_nBones );
    vector3  LDir    = -LightDirection;

    // Compute anim frame
    RawAnim.ComputeBonesL2W( pL2W, Frame );

    if( anim )
        Frame += 1;

    vram_Activate( Bitmap );

    // Compute the new position for the verts
    for( i=0; i<pSkinGeom->m_nDList; i++ )
    {
        skin_geom::dlist_pc& DList         = pSkinGeom->m_DListPtr.pPC[i];
        s32                  ReindexCur    = 0;
/*
        // Allocate a new place for the vert position
        vector4*    pPos    = (vector4*)smem_BufferAlloc( sizeof(vector4) * DList.nUVs );
        u16*        pCol    = (u16*)    smem_BufferAlloc ( sizeof(xcolor) * DList.nUVs );
        ASSERT(pCol && pPos);

        // Start processing verts
        for( j=0; j<DList.nPosNorm; j++ )
        {
            skin_geom::posnom_ps2& PN      = DList.pPosNorm[j];

            vector3                NewPos  = PN.W1 * ( pL2W[ DList.pBone[j*2+0] ] * PN.Pos ) +
                                             PN.W2 * ( pL2W[ DList.pBone[j*2+1] ] * PN.Pos );
            vector3                NewNorm = pL2W[ DList.pBone[j*2+0] ].RotateVector( PN.Normal );
            s32                    I       = (s32)(fMin( 1, 0.3f + fMax( 0, NewNorm.Dot( LDir ) )) * 255);
            
            //
            // CPU Code ( Copy verts to final location )
            //
            for( s32 k=DList.pReIndex[ReindexCur++]; k>0; k--, ReindexCur++ )
            {
                ASSERT( ReindexCur <= DList.nReIndex );

                // This means that we must set the ADC bits here
                if( DList.pReIndex[ReindexCur] & (1<<15) )
                {
                    s32 Index = DList.pReIndex[ReindexCur] & (u16)~(1<<15);

                    pPos[Index] = vector4( NewPos.X, NewPos.Y, NewPos.Z, 1 ); // | ADC bit here
                    pCol[Index] = (I>>3) | ((I>>3)<<5) | ((I>>3)<<10);
                    pCol[Index] = 16 | (16<<5) | (16<<10);
                    *(u32*)&pPos[Index].W |= 0x8000;
                }
                else
                {
                    s32 Index = DList.pReIndex[ReindexCur];

                    pPos[Index] = vector4( NewPos.X, NewPos.Y, NewPos.Z, 1 ); // NO ADC bit here
                    pCol[Index] = (I>>3) | ((I>>3)<<5) | ((I>>3)<<10);
                    pCol[Index] = 16 | (16<<5) | (16<<10);
                }
            }
        }

        RenderStrip( pPos, pCol, (s16*)DList.pUV, DList.nUVs );
*/
    }
}

#endif

//==============================================================================

#ifdef TARGET_PS2

void RenderRigidPS2( void )
{
    List = MINMAX( 0, List, pRigidGeom->m_nSubMeshs-1 );

    rigid_geom::dlist_ps2* pDList = &pRigidGeom->m_System.pPS2->pDList[List];

    vector4* pPos = pDList->pPosition;
    s16*     pUV  = pDList->pUV;
    u16*     pCol = (u16*)pUV;
    
    if( UseVU )
    {
        vram_Activate( Bitmap );
        vu1_Init( View.GetW2S(), View.GetW2C(), View.GetC2S() );
        vu1_Begin();
        vu1_Render( pPos, pCol, pUV, Normals, pDList->nVerts, 0, L2W[0] );
        vu1_End();
    }
    else
    {
        RenderStrip( pPos, pCol, pUV, pDList->nVerts );
    }
}

#endif

//==============================================================================

#ifdef TARGET_PC

void RenderRigidPC( void )
{
    List = MINMAX( 0, List, pRigidGeom->m_nSubMeshs-1 );
    
    rigid_geom::dlist_pc*  pDList  = &pRigidGeom->m_System.pPC->pDList[List];
    rigid_geom::vertex_pc* pVertex =  pRigidGeom->m_System.pPC->pVertex;

    draw_Begin( DRAW_TRIANGLES, DRAW_TEXTURED );
    draw_SetTexture( Bitmap );

    for( s32 i=0; i<pDList->nIndices; i++ )
    {
        s32 I = pRigidGeom->m_System.pPC->pIndex[ pDList->iIndices + i ];
        
        xcolor C;
        u16 RGB = *(u16*)&pVertex[I].UV;
        C.R = ((RGB >>  0) & 0x1F) << 3;
        C.G = ((RGB >>  5) & 0x1F) << 3;
        C.B = ((RGB >> 10) & 0x1F) << 3;
        C.A = 255;
        
        draw_Color ( C );
        draw_UV    ( pVertex[I].UV  );
        draw_Vertex( pVertex[I].Pos );
    }
    
    draw_End();
}

#endif

//==============================================================================

void eng_GetStats(s32 &Count, f32 &CPU, f32 &GS, f32 &IMS, f32 &FPS);

void AppMain( s32, char** )
{
    Initialize();

    {
        auxbmp_Load( Bitmap, "C:\\GameData\\A51\\test.bmp" );
        
        #ifdef TARGET_PS2
        
        auxbmp_ConvertToPS2( Bitmap );
        char* Path = "C:\\GameData\\A51\\Release\\PS2";
        
        #else
        
        auxbmp_ConvertToD3D( Bitmap );
        char* Path = "C:\\GameData\\A51\\Release\\PC";
        
        #endif
        
        vram_Register( Bitmap );
    
        fileio File;
        //File.Load( xfs( "%s\\Hazmat_00.skingeom",       Path ), pSkinGeom );
        //File.Load( xfs( "%s\\room_ceiling_00.rigidgeom", Path ), pRigidGeom );
        //File.Load( xfs( "%s\\gray_TOTAL.skingeom",      Path ), pSkinGeom );
        //File.Load( xfs( "%s\\gray_TOTAL3.rigidgeom",     Path ), pRigidGeom );
          File.Load( xfs( "%s\\gray_LOD_test.rigidgeom",   Path ), pRigidGeom );
    }
    
    LoadCamera();    

    xtimer T;
    T.Start();
    
    #ifdef TARGET_PS2
    s32 Count = 0;
    f32 CPU = 0.0f, IMS = 0.0f, FPS = 0.0f, GS = 0.0f;
    vu1_stats Stats;
    #endif

    L2W[0].Identity();

    while( TRUE )
    {
        if( !HandleInput() )
            break;
        
        eng_Begin( "Render" );
        
        Render();

        #ifdef TARGET_PS2
        
        vu1_InitStats();
        
        if( pSkinGeom  ) RenderSkinPS2( LDir );
        if( pRigidGeom ) RenderRigidPS2();
        
        vu1_GetStats( Stats );
        x_printfxy( 2, 2, "Verts: %d", Stats.nTotalVerts );
        x_printfxy( 2, 3, "CPU  : %f", Stats.CPUTime );
        x_printfxy( 2, 4, "GS/VU: %f", GS );
        x_printfxy( 2, 5, "Bytes: %d", Stats.nBytesUploaded );

        #else
        
        if( pSkinGeom  ) RenderSkinPC( LDir );
        if( pRigidGeom ) RenderRigidPC();
        
        #endif
        
        eng_End();
        eng_PageFlip();

        #ifdef TARGET_PS2
        if( T.ReadSec() >= 1.0f )
        {
            T.Reset();
            T.Start();
            eng_GetStats( Count, CPU, GS, IMS, FPS );
            //eng_PrintStats();
        }
        #endif
    }

    vram_Unregister( Bitmap );
    Bitmap.Kill();

    Shutdown();
}

//==============================================================================

