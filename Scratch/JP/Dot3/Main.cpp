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
#include "RawMesh.hpp"
#include "Geometry\vu1\vu1.hpp"
#include "Auxiliary\Bitmap\aux_Bitmap.hpp"

//==============================================================================
//  STORAGE
//==============================================================================

f32             Frame=0;
rawanim         RawAnim;
rawmesh         RawMesh;
skin_geom*      pSkinGeom;
view            View;
random          R;
s32             ScreenW;
s32             ScreenH;

s8              Normals[16384]              PS2_ALIGNMENT(16);
matrix4         g_L2W[ VU1_MAX_INSTANCES ]  PS2_ALIGNMENT(16);
s32             NumInst = 1;    //104;

xbitmap Bitmap;
xbitmap BumpMap;
xbitmap NoBump;
xbitmap Dot3Map;

#ifdef TARGET_PC
    const char*     DataPath = "Data\\";
#endif

xbool UseVU = FALSE;

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
    
        g_L2W[i].Identity();
        g_L2W[i].Scale( S );
        //g_L2W[i].Rotate( R );
        g_L2W[i].Translate( P );
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
        if( input_IsPressed( INPUT_PS2_BTN_CROSS  ) ) eng_ScreenShot();

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

void RenderStrip( vector4* pPos, const skin_geom::uv_ps2* pUV, xcolor* pCol, s32 nVerts )
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

        ColQ[2]   = pCol[j];
        
        TexQ[2].X = pUV[j].U / (f32)(1<<12);
        TexQ[2].Y = pUV[j].V / (f32)(1<<12);

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

radian Ang = R_0;

radian3 Rot( R_0, R_0, R_0 );
radian3 Spd( R_90 * 5, R_0, R_0 );
radian3 Amp( R_45, R_45, R_0 );

f32 lmin=0.0f;
f32 lmax=1.0f;

s32 alpha=0;
s32 blend=1;
u32 Mask=0x00FFFFFF;
f32 Ambient=0.5f;
f32 MaxI=1.0f;

s32 rotobj=0;
s32 rotlight=1;

s32 tfx = 0;
s32 tcc = 1;
s32 Lum = 128;
s32 lighting=0;

void RenderSkin( vector3& LightDirection )
{
    s32      i,j;
    //s32      nStrips = 0;
    matrix4* pL2W    = (matrix4*)smem_BufferAlloc( sizeof(matrix4) * RawAnim.m_nBones );
    vector3  LDir    = -LightDirection;

    // Compute anim frame
    //RawAnim.ComputeBonesL2W( pL2W, Frame );
    pL2W->Identity();

    matrix4 Rot;
    Rot.Identity();
    if( rotobj ) Rot.RotateY( Ang );
    
    for( s32 n=0; n<RawAnim.m_nBones; n++ )
    {
        pL2W[n] = Rot * pL2W[n];
    }

    if( anim )
        Frame += 1;

    // Render using the rawmesh
    if( 1 )
    {
        vector3 PTS[4096][3];
        xcolor  RGB[4096][3];
        vector2 UVS[4096][3];
    
        for( s32 i=0; i<RawMesh.m_nFacets; i++ )
        {
            for( s32 j=0; j<3; j++ )
            {
                vector3 V;
                rawmesh::vertex& Vert = RawMesh.m_pVertex[ RawMesh.m_pFacet[i].iVertex[j] ];

                V.Zero();
                for( s32 t=0;t<Vert.nWeights;t++ )
                    V += Vert.Weight[t].Weight * ( pL2W[ Vert.Weight[t].iBone ] * Vert.Position );

                vector3  NewNorm = pL2W[ Vert.Weight[0].iBone ].RotateVector( Vert.Normal[0] );

                s32   I       = (s32)(fMin( MaxI, Ambient + fMax( 0, NewNorm.Dot( LDir ) )) * 255);

                if( !lighting ) I = Lum;
                
                PTS[i][j] = V;
                RGB[i][j] = xcolor( I, I, I, I );
                UVS[i][j] = Vert.UV[0];
            }
        }

        {
            matrix4 W2L( Rot );
            W2L.Invert();
            vector3 L = W2L.RotateVector( LDir );
            L.Normalize();
        
            xcolor* pClut = (xcolor*)Dot3Map.GetClutData();
            ASSERT( pClut );
            
            for( s32 c=0; c<256; c++ )
            {
                f32 X = MINMAX( -1.0f, (pClut[c].R / 128.0f) - 1.0f, 1.0f );
                f32 Y = MINMAX( -1.0f, (pClut[c].G / 128.0f) - 1.0f, 1.0f );
                f32 Z = MINMAX( -1.0f, (pClut[c].B / 128.0f) - 1.0f, 1.0f );
            
                f32 Dot = MINMAX( -1.0f, (X * L.X) + (Y * L.Y) + (Z * L.Z), 1.0f );
                
                pClut[c].A = (u8)MINMAX( 0.0f, ( 255.0f * ((Dot + 1.0f) * 0.5f) ), 255.0f );
            }
        
            draw_Begin( DRAW_TRIANGLES, DRAW_TEXTURED | DRAW_USE_ALPHA );

            //
            // Pass 1
            //

            vram_Flush();

            vram_SetTextureFunction( Dot3Map, tfx, tcc );
            draw_SetTexture( Dot3Map );
            
            gsreg_Begin();
            gsreg_SetFBMASK( Mask );
            gsreg_SetAlphaBlend( ALPHA_BLEND_OFF );
            gsreg_End();

            {
                for( s32 i=0; i<RawMesh.m_nFacets; i++ )
                {
                    for( s32 j=0; j<3; j++ )
                    {
                        draw_Color ( RGB[i][j] );
                        draw_UV    ( UVS[i][j] );
                        draw_Vertex( PTS[i][j] );
                    }
                }
            }

            //
            // Pass 2
            //

            draw_SetTexture( Bitmap );

            gsreg_Begin();
            gsreg_SetFBMASK( 0x00000000 );
            gsreg_SetAlphaBlend( ALPHA_BLEND_MODE( C_SRC, C_ZERO, A_DST, C_ZERO ) );
            
            if( !blend ) gsreg_SetAlphaBlend( ALPHA_BLEND_OFF );
            gsreg_End();

            {
                for( s32 i=0; i<RawMesh.m_nFacets; i++ )
                {
                    for( s32 j=0; j<3; j++ )
                    {
                        draw_Color ( RGB[i][j] );
                        draw_UV    ( UVS[i][j] );
                        draw_Vertex( PTS[i][j] );
                    }
                }
            }

            draw_End();
        }
        
        return;
    }

    vram_Activate( Bitmap );
    vu1_Init( View.GetW2S(), View.GetW2C(), View.GetC2S() );
    vu1_Begin();

    // Compute the new position for the verts
    for( i=0; i<pSkinGeom->m_nDList; i++ )
    {
        skin_geom::dlist_ps2& DList         = pSkinGeom->m_DListPtr.pPS2[i];
        s32                   ReindexCur    = 0;

        // Allocate a new place for the vert position
        vector4*    pPos    = (vector4*)smem_BufferAlloc( sizeof( vector4 ) * DList.nUVs );
        u16*        pCol    = (u16*)    smem_BufferAlloc( sizeof( xcolor  ) * DList.nUVs );
        xcolor*     pRGB    = (xcolor*) smem_BufferAlloc( sizeof( xcolor  ) * DList.nUVs );
        ASSERT( pCol && pPos && pRGB );

        // Start processing verts
        for( j=0; j<DList.nPosNorm; j++ )
        {
            skin_geom::posnom_ps2& PN      = DList.pPosNorm[j];

            //
            // VU0 Code
            // 
            if( 0 )
            {
                xbool Found = FALSE;
                f32 Bestd=100000.0f;
                s32 Ind=-1;
                for( s32 t=0; t<RawMesh.m_nVertices; t++ )
                {
                    f32 d = (PN.Pos - RawMesh.m_pVertex[t].Position).Length();

                    if( d < Bestd )
                    {
                        Bestd = d;
                        Ind   = t;
                    }
                    if( PN.Pos.X != RawMesh.m_pVertex[t].Position.X ) continue;
                    if( PN.Pos.Y != RawMesh.m_pVertex[t].Position.Y ) continue;
                    if( PN.Pos.Z != RawMesh.m_pVertex[t].Position.Z ) continue;
                    if( DList.pBone[j*2+0] != RawMesh.m_pVertex[t].Weight[0].iBone ) continue;
                    if( PN.W1              != RawMesh.m_pVertex[t].Weight[0].Weight ) continue;

                    Found = TRUE;
                }

                //ASSERT( Found );
            }

            vector3                NewPos  = PN.W1 * ( pL2W[ DList.pBone[j*2+0] ] * PN.Pos ) +
                                             PN.W2 * ( pL2W[ DList.pBone[j*2+1] ] * PN.Pos );
            vector3                NewNorm = pL2W[ DList.pBone[j*2+0] ].RotateVector( PN.Normal );
            s32                    I       = (s32)(fMin( 1, Ambient + fMax( 0.0f, NewNorm.Dot( LDir ) )) * 255);
            
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

                I = Lum;                    
                    pRGB[Index].Set( I, I, I, I );
                }
                else
                {
                    s32 Index = DList.pReIndex[ReindexCur];

                    pPos[Index] = vector4( NewPos.X, NewPos.Y, NewPos.Z, 1 ); // NO ADC bit here
                    pCol[Index] = (I>>3) | ((I>>3)<<5) | ((I>>3)<<10);
                    pCol[Index] = 16 | (16<<5) | (16<<10);
                    
                I = Lum;                    
                    pRGB[Index].Set( I, I, I, I );
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
                    vu1_AddInstance( pCol, g_L2W[i], (clipped ? VU1_INSTANCE_CLIPPED : 0) );
                }
                
                vu1_RenderInstances();
                vu1_EndInstance();
            }
            else
            {
                for( s32 i=0; i<NumInst; i++ )
                {
                    //vu1_Render( pPos, pCol, (s16*)DList.pUV, (s8*)Normals, DList.nUVs, g_L2W[i] );
                }
            }
        }
        else
        {
            matrix4 W2L( Rot );
            W2L.Invert();
            vector3 L = W2L.RotateVector( LDir );
        
            xcolor* pClut = (xcolor*)Dot3Map.GetClutData();
            ASSERT( pClut );
            
            for( s32 c=0; c<256; c++ )
            {
                f32 X = MINMAX( -1.0f, (pClut[c].R / 128.0f) - 1.0f, 1.0f );
                f32 Y = MINMAX( -1.0f, (pClut[c].G / 128.0f) - 1.0f, 1.0f );
                f32 Z = MINMAX( -1.0f, (pClut[c].B / 128.0f) - 1.0f, 1.0f );
            
                f32 Dot = MINMAX( -1.0f, (X * L.X) + (Y * L.Y) + (Z * L.Z), 1.0f );
                
                pClut[c].A = (u8)MINMAX( 0.0f, ( 255.0f * ((Dot + 1.0f) * 0.5f) ), 255.0f );
            }
            
            draw_Begin( DRAW_TRIANGLES, DRAW_TEXTURED | DRAW_USE_ALPHA );

            //
            // Pass 1
            //

            vram_Flush();

            vram_SetTextureFunction( Dot3Map, 0, 1 );
            draw_SetTexture( Dot3Map );
            
            gsreg_Begin();
            gsreg_SetFBMASK( Mask );
            gsreg_SetAlphaBlend( ALPHA_BLEND_OFF );
            gsreg_End();

            RenderStrip( pPos, DList.pUV, pRGB, DList.nUVs );

            //
            // Pass 2
            //

            draw_SetTexture( Bitmap );

            gsreg_Begin();
            gsreg_SetFBMASK( 0x00000000 );
            gsreg_SetAlphaBlend( ALPHA_BLEND_MODE( C_SRC, C_ZERO, A_DST, C_ZERO ) );
            
            if( !blend ) gsreg_SetAlphaBlend( ALPHA_BLEND_OFF );
            gsreg_End();
            
            RenderStrip( pPos, DList.pUV, pRGB, DList.nUVs );

            draw_End();
        }
    }
    
    vu1_End();
}

//==============================================================================

void eng_GetStats(s32 &Count, f32 &CPU, f32 &GS, f32 &INT, f32 &FPS);

vector3 LDir( 0.0f, 0.0f, 1.0f );

//==============================================================================

void RGB2XYZ( xcolor C, vector3& V )
{
    V.X = (C.R / 128.0f) - 1.0f;
    V.Y = (C.G / 128.0f) - 1.0f;
    V.Z = (C.B / 128.0f) - 1.0f;
    return;
    
    f32 R = C.R / 255.0f;
    f32 G = C.G / 255.0f;
    f32 B = C.B / 255.0f;
  
    V.X = MINMAX( -1.0f, B - (G + R), 1.0f );
    V.Y = MINMAX( -1.0f, G - (B + R), 1.0f );
    V.Z = MINMAX( -1.0f, R - (G + B), 1.0f );
}

//==============================================================================

void AppMain( s32, char** )
{
    Initialize();

    {
        auxbmp_Load( Bitmap,  "test.bmp"   );
        auxbmp_Load( BumpMap, "bump.bmp"   );
        auxbmp_Load( NoBump,  "nobump.bmp" );
        auxbmp_Load( Dot3Map, "dot3.tga"   );
        
        //Dot3Map = BumpMap;

        if( 0 )
        {
            ASSERT( Dot3Map.GetWidth()  == BumpMap.GetWidth()  );
            ASSERT( Dot3Map.GetHeight() == BumpMap.GetHeight() );
            ASSERT( Dot3Map.GetWidth()  == NoBump.GetWidth()   );
            ASSERT( Dot3Map.GetHeight() == NoBump.GetHeight()  );
        
            Dot3Map.ConvertFormat( xbitmap::FMT_32_ARGB_8888 );
            
            for( s32 Y=0; Y<Dot3Map.GetHeight(); Y++ )
            {
                for( s32 X=0; X<Dot3Map.GetWidth(); X++ )
                {
                    xcolor BC = BumpMap.GetPixelColor( X, Y );
                    xcolor NC = NoBump.GetPixelColor ( X, Y );
                    xcolor O;
                    
                    vector3 B, N, F;
                    
                    RGB2XYZ( BC, B );
                    RGB2XYZ( NC, N );

                    F.X = MINMAX( -1.0f, B.X - N.X, 1.0f );
                    F.Y = MINMAX( -1.0f, B.Y - N.Y, 1.0f );
                    F.Z = MINMAX( -1.0f, B.Z - N.Z, 1.0f );
                    
                    O.R = (u8)( 255.0f * ((F.X + 1.0f) * 0.5f) );
                    O.G = (u8)( 255.0f * ((F.Y + 1.0f) * 0.5f) );
                    O.B = (u8)( 255.0f * ((F.Z + 1.0f) * 0.5f) );
                    
                    Dot3Map.SetPixelColor( O, X, Y );
                }
            }
        }

        Dot3Map.SaveTGA( "dot3RGB.tga" );
        Dot3Map.ConvertFormat( xbitmap::FMT_P8_ABGR_8888 );
        Dot3Map.SaveTGA( "dot3.tga" );

        Bitmap.ConvertFormat( xbitmap::FMT_P8_ABGR_8888 );

        auxbmp_ConvertToPS2( Bitmap  );
        auxbmp_ConvertToPS2( Dot3Map );

        vram_Register( Bitmap  );
        vram_Register( Dot3Map );

        char Filename[256];
        x_strcpy( Filename, "test.MATX" );

        if( 0 )
        {
            X_FILE* fp;
            char Buffer[256];
            
            if( !(fp = x_fopen( "file.txt", "rb" ))) DEMAND( 0 );
            x_fread( Buffer, 1, sizeof( Filename ), fp );
            x_fclose( fp );
            
            s32 i=0;
            while( (Buffer[i] != 0x0D) && (Buffer[i] != 0x00) )
            {
                Filename[i] = Buffer[i];
                i++;
            }
            Filename[i] = 0;
        }
    
        //fileio File;
        //File.Load   ( "C:\\GameData\\A51\\Release\\PS2\\Hazmat_00.skingeom", pSkinGeom );
        //RawMesh.Load( "C:\\GameData\\A51\\Source\\Prototype\\Character\\Hazmat\\Hazmat_00.MATX");
        //RawAnim.Load( "C:\\GameData\\A51\\Source\\Prototype\\Character\\Hazmat\\Hazmat_00.MATX");

        RawMesh.Load( Filename );
        RawAnim.Load( Filename );

        s32 j;
        // Make sure to clean color information ( No color allow for now )
        for( j=0; j<RawMesh.m_nVertices; j++ )
        {
            RawMesh.m_pVertex[j].nColors = 0;
        }

        for(  j=0; j<RawMesh.m_nFacets; j++ )
        {
            RawMesh.m_pFacet[j].iMaterial = 0;
        }

        // Do remapping of the skeleton
        RawAnim.DeleteDummyBones();
        RawMesh.ApplyNewSkeleton( RawAnim );

        RawMesh.CleanWeights( 2, 0.00001f );
        RawMesh.SortFacetsByMaterialAndBone();
        RawMesh.CleanMesh();
    }
    
    LoadCamera();    

    xtimer T;
    T.Start();
    s32 Count = 0;
    f32 CPU, INT, FPS, GS = 0.0f;
    vu1_stats Stats;

    while( TRUE )
    {
        if( !HandleInput() )
            break;
        
        eng_Begin( "Render" );
        vu1_InitStats();
        
        Render();

        vector3 L( LDir );
        L.Normalize();
        if( rotlight ) L.Rotate( radian3( x_sin( Rot.Pitch ) * Amp.Pitch, x_sin( Rot.Yaw ) * Amp.Yaw, x_sin( Rot.Roll ) * Amp.Roll ) );
        
        RenderSkin( L );
        
        vu1_GetStats( Stats );
        x_printfxy( 2, 2, "Verts: %d", Stats.nTotalVerts );
        x_printfxy( 2, 3, "CPU  : %f", Stats.CPUTime );
        x_printfxy( 2, 4, "GS/VU: %f", GS );
        x_printfxy( 2, 5, "Bytes: %d", Stats.nBytesUploaded );

        eng_End();
        eng_PageFlip();

        eng_GetStats( Count, CPU, GS, INT, FPS );
        
        if( T.ReadSec() >= 1.0f )
        {
            T.Reset();
            T.Start();
            //eng_PrintStats();
        }

        f32 DeltaTime = 1.0f / 60.0f;
        
        Ang       += R_60      * DeltaTime;
        Rot.Pitch += Spd.Pitch * DeltaTime;
        Rot.Yaw   += Spd.Yaw   * DeltaTime;
        Rot.Roll  += Spd.Roll  * DeltaTime;
    }

    vram_Unregister( Bitmap  );
    vram_Unregister( BumpMap );
    vram_Unregister( NoBump  );
    vram_Unregister( Dot3Map );
    
    Bitmap.Kill();
    BumpMap.Kill();
    NoBump.Kill();
    Dot3Map.Kill();

    Shutdown();
}

//==============================================================================

