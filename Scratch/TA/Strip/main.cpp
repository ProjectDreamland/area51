//==============================================================================
//
//  Toy01.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "rawmesh.hpp"
#include "Ps2Strip.hpp"
#include "NvTriStrip.h"
#include <stdio.h>
#include "tc.h"

//==============================================================================
//  STORAGE
//==============================================================================

PrimitiveGroup* primGroups;
unsigned short  nGroups;

ps2_strip       Strip;
rawmesh         Rawmesh;
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

void RenderMesh( void )
{
    eng_Begin( "2D Sprite" );
        
    {
        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE  );
        draw_Begin( DRAW_TRIANGLES );
        draw_ClearL2W();

        s32 i,j;
        xcolor C;

        x_srand( 0 );
        for( i=0; i<Strip.GetIndexCount(); )
        {
            s32 iPrev[2];

            C.Set( x_rand(), x_rand(), x_rand() );

            for( j=0; j<2; j++, i++)
            {
                iPrev[j] = Strip.GetIndex(i);
                ASSERT( Strip.IsIndexNewStrip(i) );
                if( (i%Strip.GetCacheSize()) == 0 )
                {
                    ASSERT( Strip.IsIndexNewStrip(i) );
                    ASSERT( Strip.IsIndexNewStrip(i+1) );
                }
            }

            for( ;i<Strip.GetIndexCount() && Strip.IsIndexNewStrip(i) == FALSE; i++ )
            {
                for( j=0; j<2; j++)
                {
                    vector3 P = Rawmesh.m_pVertex[ iPrev[j] ].Position;

                    draw_Color( C );
                    draw_Vertex( P );
                }

                if( (i%Strip.GetCacheSize()) == 0 )
                {
                    ASSERT( Strip.IsIndexNewStrip(i) );
                    ASSERT( Strip.IsIndexNewStrip(i+1) );
                }

                iPrev[0] = iPrev[1]; 
                iPrev[1] = Strip.GetIndex(i);

                vector3 P = Rawmesh.m_pVertex[ iPrev[1] ].Position;
                draw_Color( C );
                draw_Vertex( P );
            }
        }

        draw_End();
    }

    {
        draw_Begin( DRAW_TRIANGLES, DRAW_WIRE_FRAME );
        draw_ClearL2W();

        for( s32 i=0; i<Rawmesh.m_nFacets; i++ )
        for( s32 j=0; j<3; j++ )
        {
            vector3 N = Rawmesh.m_pVertex[ Rawmesh.m_pFacet[i].iVertex[j] ].Normal[0];
            vector3 P = Rawmesh.m_pVertex[ Rawmesh.m_pFacet[i].iVertex[j] ].Position;
            xcolor C;

            C.Set( (s32)((N.X+1)*128), (s32)((N.Y+1)*128), (s32)((N.Z+1)*128), 255 );
            C.Set( 255,255,255,255 );

            draw_Color( C );
            draw_Vertex( P );
        }

        draw_End();
    }


    eng_End();

}

//==============================================================================

void AppMain( s32, char** )
{
    s32 i;

    Initialize();

    //
    // Load and clean the mesh
    //
    Rawmesh.Load( "Hazmat_test.MATX" );
    for( i=0; i<Rawmesh.m_nVertices; i++ )
    {
        Rawmesh.m_pVertex[i].nColors = 0;
        Rawmesh.m_pVertex[i].nUVs = 0;
    }
    Rawmesh.CleanMesh();

    //
    // Build using the nvidia strip
    //
    s32 VNIndexCount;
    unsigned short* pIndex = new unsigned short[ Rawmesh.m_nFacets*3 ];
    ASSERT( pIndex );

    for( i=0; i<Rawmesh.m_nFacets; i++ )
    {
        pIndex[i*3+0] = Rawmesh.m_pFacet[i].iVertex[0];
        pIndex[i*3+1] = Rawmesh.m_pFacet[i].iVertex[1];
        pIndex[i*3+2] = Rawmesh.m_pFacet[i].iVertex[2];
    }

    SetStitchStrips( FALSE );
    GenerateStrips( pIndex, Rawmesh.m_nFacets*3, &primGroups, &nGroups );

    VNIndexCount = 0;
    for( i=0; i<nGroups; i++ )
    {
        VNIndexCount += primGroups[i].numIndices;
    }
    

    //
    // Build the ps2 strip
    //
    Strip.Begin( Rawmesh.m_nVertices, 320 );
    for( i=0; i<Rawmesh.m_nFacets; i++ )
    {
        Strip.AddTri(   Rawmesh.m_pFacet[i].iVertex[0], 
                        Rawmesh.m_pFacet[i].iVertex[1],
                        Rawmesh.m_pFacet[i].iVertex[2] );
    }
    Strip.End();

    //
    // Do the defender strip
    //
    s32 nIndices=0;
    ACTCData* pActc = actcNew();

    actcBeginInput( pActc );
    for( i=0; i<Rawmesh.m_nFacets; i++ )
    {
        actcAddTriangle( pActc, Rawmesh.m_pFacet[i].iVertex[0],
                                Rawmesh.m_pFacet[i].iVertex[1], 
                                Rawmesh.m_pFacet[i].iVertex[2] );
    }
    actcEndInput( pActc ); 

    actcBeginOutput( pActc );
    while(1)
    {
    	unsigned v1=-1, v2=-1, v3=-1, vnext=-1, vpivot=-1 ;
        unsigned prim = actcStartNextPrim(pActc, &v2, &v3 );
        if (prim == ACTC_DATABASE_EMPTY)
            break ;

        nIndices += 2;
        if(prim == ACTC_PRIM_STRIP )
        {
            // Write out rest of verts
            while( actcGetNextVert(pActc, &vnext) != ACTC_PRIM_COMPLETE )
            {
                v1 = v2 ;
                v2 = v3 ;
                v3 = vnext ;
                nIndices++;
            }
        }
        else
        {
            ASSERT( 0 );
        }
    }
    actcEndOutput( pActc );
    


    while( TRUE )
    {
        if( !HandleInput() )
            break;
        Render();
        RenderMesh();

        x_printfxy(0,0,"Inv Score: %f", Strip.GetFinalScore() );
        x_printfxy(0,1,"NV  Score: %f", VNIndexCount /(f32)Rawmesh.m_nFacets  );
        x_printfxy(0,2,"DF  Score: %f", nIndices /(f32)Rawmesh.m_nFacets  );
    // DONE!
    eng_PageFlip();

    }

    Shutdown();
}

//==============================================================================
