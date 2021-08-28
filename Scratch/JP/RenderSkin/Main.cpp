//==============================================================================
//
//  Toy01.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "Geometry\SkinGeom.hpp"
#include "rawanim.hpp"
#include "Skin.hpp"
#include "vu0.hpp"
#include "vu1.hpp"
#include "Auxiliary\Bitmap\aux_Bitmap.hpp"

//==============================================================================
//  STORAGE
//==============================================================================

f32             Frame=0;
rawanim         RawAnim;
skin_geom*      pSkinGeom;
view            View;
random          R;
s32             ScreenW;
s32             ScreenH;

#ifdef TARGET_PC
    const char*     DataPath = "Data\\";
#endif

#ifdef TARGET_GCN
    const char*     DataPath = "";
#endif

xbitmap Bitmap;

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
    
    eng_GetRes( ScreenW, ScreenH );

    vu0_Init();
}

//=========================================================================

void Shutdown( void )
{
    vu0_Kill();
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
    if(0)
    {
    eng_Begin( "GridBoxMarker" );
    {
        draw_ClearL2W();
        draw_Grid( vector3(  0,  0,  0), 
                   vector3(100,  0,  0), 
                   vector3(  0,  0,100), 
                   xcolor (  0,128,  0), 16 );
    }
    eng_End();
    }
}

//==============================================================================

xbool UseVU1 = TRUE;
xbool clip   = TRUE;

void RenderSkin( vector3& LightDirection )
{
    s32      i,j;
    //s32      nStrips = 0;
    matrix4* pL2W    = (matrix4*)smem_BufferAlloc( sizeof(matrix4) * RawAnim.m_nBones );
    vector3  LDir    = -LightDirection;
    
    vector4 PS2_ALIGNMENT( 16 ) Output[2];

    skin_Init();
    
    eng_Begin( "VU1Init" );
    vu1_Init( View.GetC2S() );
    eng_End();

    // Compute anim frame
    RawAnim.ComputeBonesL2W( pL2W, Frame );

    skin_UploadMatrices( pL2W, RawAnim.m_nBones );

    Frame += 1;
    
    // Compute the new position for the verts
    for( i=0; i<pSkinGeom->m_nDList; i++ )
    {
        skin_geom::dlist_ps2& DList         = pSkinGeom->m_DListPtr.pPS2[i];
        s32                   ReindexCur    = 0;

        // Allocate a new place for the vert position
        vector4*    pPos    = (vector4*)smem_BufferAlloc( sizeof(vector4) * DList.nUVs );
        xcolor*     pCol    = (xcolor*)smem_BufferAlloc ( sizeof(xcolor) * DList.nUVs );
        ASSERT(pCol && pPos);

        skin_geom::posnom_ps2& PN = DList.pPosNorm[0];
        
        skin_LoadVertex0( &PN, &DList.pBone[0] );
        vu0_ExecuteSync();
        vu0_ExecuteMicrocode( 0 );

        // Start processing verts
        for( j=1; j<=DList.nPosNorm; j++ )
        {
            skin_geom::posnom_ps2& PN      = DList.pPosNorm[j];

            //
            // VU0 Code
            // 
            
            if( j & 1 )
            {
                skin_LoadVertex1( &PN, &DList.pBone[ j << 1 ] );
                vu0_ExecuteSync();
                vu0_ExecuteMicrocode( 1 << 1 );
                skin_StoreVertex0( Output );
            }
            else
            {
                skin_LoadVertex0( &PN, &DList.pBone[ j << 1 ] );
                vu0_ExecuteSync();
                vu0_ExecuteMicrocode( 0 );
                skin_StoreVertex1( Output );
            }

            vector3 NewPos ( Output[0].X, Output[0].Y, Output[0].Z );
            vector3 NewNorm( Output[1].X, Output[1].Y, Output[1].Z );
            
            //vector3                NewPos  = PN.W[0] * ( pL2W[ PN.B[0] ] * PN.Pos ) +
            //                                 PN.W[1] * ( pL2W[ PN.B[1] ] * PN.Pos );
            //vector3                NewNorm = pL2W[ PN.B[0] ].RotateVector( PN.Normal );
            
            s32 I   = (s32)(fMin( 1, 0.3f + fMax( 0, NewNorm.Dot( LDir ) )) * 255);
            
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
                    pCol[Index] = xcolor( I, I, I, 255 );
                    *(u32*)&pPos[Index].W = 0x8000;
                }
                else
                {
                    s32 Index = DList.pReIndex[ReindexCur];

                    pPos[Index] = vector4( NewPos.X, NewPos.Y, NewPos.Z, 0 ); // NO ADC bit here
                    pCol[Index] = xcolor( I, I, I, 255 );
                }
            }
        }

        //
        // Render display list ( KICK IT OFF )
        //
        
        if( UseVU1 == TRUE )
        {
            eng_Begin( "VU1" );

            matrix4 L2W;
            matrix4 L2S;
            matrix4 L2C;
            L2W.Identity();
            L2S = View.GetW2S() * L2W;
            L2C = View.GetW2C() * L2W;

            vram_Activate( Bitmap );
            
            vu1_Begin( VU1_NO_TEXTURE );
            //vu1_Begin();
            vu1_SetL2S( L2S );
            vu1_SetL2C( L2C );
            vu1_Render( pPos, (s16*)DList.pUV, pCol, DList.nUVs, clip );
            vu1_End();
            
            eng_End();
        }
        else
        {
            eng_Begin( "DrawDList" );
            draw_Begin( DRAW_TRIANGLES );

            for( j=0; j<DList.nUVs; j++ )
            {
                if( pPos[j].W )
                {
                    draw_Color  ( pCol[j] );
                    draw_UV     ( DList.pUV[j].U / (f32)(0xffff>>1), DList.pUV[j].V / (f32)(0xffff>>1) );
                    draw_Vertex ( vector3( pPos[j].X, pPos[j].Y, pPos[j].Z) );

                    j++;
                    draw_Color  ( pCol[j] );
                    draw_UV     ( DList.pUV[j].U / (f32)(0xffff>>1), DList.pUV[j].V / (f32)(0xffff>>1) );
                    draw_Vertex ( vector3( pPos[j].X, pPos[j].Y, pPos[j].Z) );

                    j++;
                    draw_Color  ( pCol[j] );
                    draw_UV     ( DList.pUV[j].U / (f32)(0xffff>>1), DList.pUV[j].V / (f32)(0xffff>>1) );
                    draw_Vertex ( vector3( pPos[j].X, pPos[j].Y, pPos[j].Z) );
                }
                else
                {
                    draw_Color  ( pCol[j-2] );
                    draw_UV     ( DList.pUV[j-2].U / (f32)(0xffff>>1), DList.pUV[j-2].V / (f32)(0xffff>>1) );
                    draw_Vertex ( vector3( pPos[j-2].X, pPos[j-2].Y, pPos[j-2].Z) );

                    draw_Color  ( pCol[j-1] );
                    draw_UV     ( DList.pUV[j-1].U / (f32)(0xffff>>1), DList.pUV[j-1].V / (f32)(0xffff>>1) );
                    draw_Vertex ( vector3( pPos[j-1].X, pPos[j-1].Y, pPos[j-1].Z) );

                    draw_Color  ( pCol[j] );
                    draw_UV     ( DList.pUV[j].U / (f32)(0xffff>>1), DList.pUV[j].V / (f32)(0xffff>>1) );
                    draw_Vertex ( vector3( pPos[j].X, pPos[j].Y, pPos[j].Z) );
                }
            }

            draw_End();
            eng_End();
        }
    }
}

//==============================================================================

void AppMain( s32, char** )
{
    Initialize();

    {
        fileio File;
        File.Load( "C:\\GameData\\A51\\Release\\PS2\\Hazmat_00.skingeom", pSkinGeom );
        RawAnim.Load( "C:\\GameData\\A51\\Source\\Prototype\\Character\\Hazmat\\Hazmat_00.MATX");
        
        auxbmp_Load( Bitmap, "test.bmp" );
        auxbmp_ConvertToPS2( Bitmap );
        vram_Register( Bitmap );
    }

    LoadCamera();    

    while( TRUE )
    {
        if( !HandleInput() )
            break;
            
        vector3 V( 0.0f, -1.0f, 0.0f );
        Render();
        RenderSkin( V );

        // DONE!
        eng_PageFlip();
    }

    vram_Unregister( Bitmap );
    Bitmap.Kill();

    Shutdown();
}

//==============================================================================

