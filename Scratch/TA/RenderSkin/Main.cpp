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


    eng_GetRes( ScreenW, ScreenH );
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
        
        draw_Color( xcolor( 255,255,255 ) );
        
        draw_UV    ( 0,0 );     draw_Vertex( vector3( (f32)ScreenW - 100, (f32)ScreenH - 100, 0) );
        draw_UV    ( 0,1 );     draw_Vertex( vector3( (f32)ScreenW - 100, (f32)ScreenH,       0) );
        draw_UV    ( 1,1 );     draw_Vertex( vector3( (f32)ScreenW,       (f32)ScreenH,       0) );
        draw_UV    ( 1,0 );     draw_Vertex( vector3( (f32)ScreenW,       (f32)ScreenH - 100, 0) );
        draw_End();
    }
    eng_End();
}

//==============================================================================
void RenderSkin( vector3& LightDirection )
{
    s32      i,j;
    s32      nStrips = 0;
    matrix4* pL2W    = (matrix4*)smem_BufferAlloc( sizeof(matrix4) * RawAnim.m_nBones );
    vector3  LDir    = -LightDirection;

    // Compute anim frame
    RawAnim.ComputeBonesL2W( pL2W, Frame );

    Frame += 1;

    // Render using the rawmesh
    if( 0 )
    {
        eng_Begin( "DrawDList(Raw)" );
        draw_Begin( DRAW_TRIANGLES );

        for( s32 i=0; i<RawMesh.m_nFacets; i++ )
        {
            draw_Color( x_rand() );
            for( s32 j=0; j<3; j++ )
            {
                vector3 V;
                rawmesh::vertex& Vert = RawMesh.m_pVertex[ RawMesh.m_pFacet[i].iVertex[j] ];

                V.Zero();
                for( s32 t=0;t<Vert.nWeights;t++ )
                    V += Vert.Weight[t].Weight * ( pL2W[ Vert.Weight[t].iBone ] * Vert.Position );

                vector3  NewNorm = pL2W[ Vert.Weight[0].iBone ].RotateVector( Vert.Normal[0] );

                s32   I       = (s32)(fMin( 1, 0.3f + fMax( 0, NewNorm.Dot( LDir ) )) * 255);

                draw_Color( xcolor( I, I, I, 255) ); 
                draw_Vertex( V );
            }
        }

        draw_End();
        eng_End();
        return;
    }


    // Compute the new position for the verts
    for( i=0; i<pSkinGeom->m_nDList; i++ )
    {
        skin_geom::dlist_ps2& DList         = pSkinGeom->m_DListPtr.pPS2[i];
        s32                   ReindexCur    = 0;

        // Allocate a new place for the vert position
        vector4*    pPos    = (vector4*)smem_BufferAlloc( sizeof(vector4) * DList.nUVs );
        xcolor*     pCol    = (xcolor*)smem_BufferAlloc ( sizeof(xcolor) * DList.nUVs );
        ASSERT(pCol && pPos);

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
                    pCol[Index] = xcolor( I, I, I, 255 );
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
        eng_Begin( "DrawDList" );

        for( j=0; j<DList.nUVs; j++ )
        {
            if( pPos[j].W )
            {
                if( j ) draw_End();
                draw_Begin( DRAW_TRIANGLE_STRIPS );
                g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

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
                draw_Color  ( pCol[j] );
                draw_UV     ( DList.pUV[j].U / (f32)(0xffff>>1), DList.pUV[j].V / (f32)(0xffff>>1) );
                draw_Vertex ( vector3( pPos[j].X, pPos[j].Y, pPos[j].Z) );
            }
        }

        draw_End();
        eng_End();
    }
}

//==============================================================================

void AppMain( s32, char** )
{
    Initialize();

    {
        fileio File;
        File.Load( "C:\\GameData\\A51\\Release\\PS2\\idle_no_gun.skingeom", pSkinGeom );
        RawMesh.Load( "C:\\GameData\\A51\\Source\\Prototype\\Characters\\Hazmat\\idle_no_gun.MATX");
        RawAnim.Load( "C:\\GameData\\A51\\Source\\Prototype\\Characters\\Hazmat\\idle_no_gun.MATX");


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
    

    while( TRUE )
    {
        if( !HandleInput() )
            break;
        Render();
        RenderSkin( vector3( 0,-1,0) );

        // DONE!
        eng_PageFlip();
    }

    Shutdown();
}

//==============================================================================
