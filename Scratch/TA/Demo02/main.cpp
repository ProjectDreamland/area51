
#include "Entropy.hpp"
#include "Geometry\geom.hpp"
#include "MeshUtil\MatxMisc.hpp"
#include "Poly\Poly.hpp"

struct piece
{
    char    FileName[256];
    geom*   pGeom;
};

struct piece_instance
{
    s32         Index;
    matrix4     L2W;
};

struct map
{
    xarray<piece>           lPiece;
    xarray<piece_instance>  lPInst;
};


static view     View;
static s32      ScreenW;
static s32      ScreenH;

static char*    s_pDir = NULL;
static map      s_Map;

//=========================================================================
void LoadMap( const char* pMap )
{
    matx_misc   MapDesc;
    s32         i, j;

    // Load the main mesh
    if( MapDesc.Load( xfs("%s\\%s", s_pDir, pMap) ) == FALSE )
        e_throw( "Unable to find the map file" );

    // 
    // create the map
    //
    for( i=0; i<MapDesc.m_nXRefs; i++ )
    {
        char FileName[256];
        matx_misc::xref& XRef = MapDesc.m_pXRef[i];

        x_splitpath( XRef.FileName, NULL, NULL,  FileName, NULL );

        // Search to see if we already got this piece
        for( j=0; j<s_Map.lPiece.GetCount(); j++ )
        {
            if( x_stricmp( s_Map.lPiece[j].FileName, FileName ) == 0 )
                break;
        }

        // We could not find it so load the new piece
        if( j == s_Map.lPiece.GetCount() )
        {
            fileio File;
            piece& Piece = s_Map.lPiece.Append();

            x_strcpy( Piece.FileName, FileName );
            File.Load( xfs("%s\\%s.geom", s_pDir, Piece.FileName), Piece.pGeom );
        }

        XRef.Position.X = x_floor(XRef.Position.X / 10.0f + 0.5f ) * 10.0f;
        XRef.Position.Y = x_floor(XRef.Position.Y / 10.0f + 0.5f ) * 10.0f;
        XRef.Position.Z = x_floor(XRef.Position.Z / 10.0f + 0.5f ) * 10.0f;


        // Add the new instance
        piece_instance& Inst = s_Map.lPInst.Append();
        Inst.Index           = j;
        Inst.L2W.Setup( XRef.Scale, 
                        XRef.Rotation, 
                        XRef.Position );
    }



}


//=========================================================================
void Initialize( void )
{
    // Set the working directory
    s_pDir = "C:\\A51\\Level1\\Release\\PC" ;
    LoadMap( "Map.matx" );

    eng_Init();

    View.SetXFOV( R_60 );
    View.SetPosition( vector3(100,100,200) );
    View.LookAtPoint( vector3(  0,  0,  0) );
    View.SetZLimits ( 10, 50000 );

    eng_GetRes( ScreenW, ScreenH );

    #ifdef TARGET_PS2
    poly_Initialize();
    #endif
}
//=========================================================================

xbool HandleInput( void )
{
    s32 Count=0;
    while( Count++ < 20 && input_UpdateState() )
    {
        
    #ifdef TARGET_PC
        // Move view using keyboard and mouse
        // WASD - forward, left, back, right
        // RF   - up, down
        // MouseRightButton - 4X speed

        radian Pitch;
        radian Yaw;
        f32    S = 4.125f;
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
        f32    R   = 0.0025f;
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

//=========================================================================

void Render( void )
{
    eng_MaximizeViewport( View );
    eng_SetView         ( View, 0 );
    eng_ActivateView    ( 0 );

    vector3 LightDir( 0.5, -0.5, 0.5f );
    vector3 LightDir2( -0.5, 0.5, 0.5f );

    LightDir.Normalize();
    LightDir2.Normalize();
    LightDir *= 0.5f;
    LightDir2 *= 0.5f;

    //==---------------------------------------------------
    //  GRID, BOX AND MARKER
    //==---------------------------------------------------
    for( s32 i=0; i<s_Map.lPInst.GetCount(); i++ )
    {
        piece_instance& Inst  = s_Map.lPInst[i];
        geom*           pGeom = s_Map.lPiece[ Inst.Index ].pGeom;

        eng_Begin( "GridBoxMarker" );
        {
#ifdef TARGET_PC
            draw_Begin( DRAW_TRIANGLES );
            draw_SetL2W( Inst.L2W );
            for( s32 i=0; i<pGeom->m_nIndices; i+=3 )
            {
                for( s32 j=0; j<3; j++ )
                {
                    geom::vertex& V0 = pGeom->m_pVertex[ pGeom->m_pIndex[i + j ] ];
                    f32 I            = fMin( LightDir.Dot( V0.Normal ), 0.0f );
                        I           += fMin( LightDir2.Dot( V0.Normal ), 0.0f );
                        I = fMin( 1, I );

                    draw_Color ( xcolor( 255*I,255*I,255*I,  0 ) ); 
                    draw_Vertex( V0.Pos ); 
                }

            }
            draw_End();
#endif

#ifdef TARGET_PS2
            vector3*  pPos = (vector3*)smem_BufferAlloc( pGeom->m_nIndices * sizeof(vector3)  );
            ps2color* pCol = (ps2color*)smem_BufferAlloc( pGeom->m_nIndices * sizeof(ps2color) );
            vector2*  pUV  = (vector2*)smem_BufferAlloc( pGeom->m_nIndices * sizeof(vector2)  );

            ASSERT( pPos );
            ASSERT( pCol );
            ASSERT( pUV  );

            for( s32 i=0; i<pGeom->m_nIndices; i+=3 )
            {
                for( s32 j=0; j<3; j++ )
                {
                    geom::vertex& V0 = pGeom->m_pVertex[ pGeom->m_pIndex[i + j ] ];
                    f32 I            = fMin( LightDir.Dot( V0.Normal ), 0.0f );
                        I           += fMin( LightDir2.Dot( V0.Normal ), 0.0f );
                        I = fMin( 1, I );

                    xcolor Color( 255*I,255*I,255*I,  0 );
                    pPos[i+j] = Inst.L2W * V0.Pos;
                    pUV[i+j].Set( 0, 0 );
                    pCol[i+j].Set( Color.R, Color.G, Color.B, Color.A );
                }
            }

            poly_Begin( POLY_NO_TEXTURE );
            poly_Render( pPos, pUV, pCol, pGeom->m_nIndices );
            poly_End();

#endif
        }
        eng_End();


    }

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

    //exit( 0 );
}
