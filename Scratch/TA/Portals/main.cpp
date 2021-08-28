//==============================================================================
//
//  Toy01.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "MeshUtil\RawMesh2.hpp"
#include "Render\Render.hpp"
#include "Render\RigidGeom.hpp"
#include "Render\SkinGeom.hpp"
#include <stdio.h>
#include "ZoneMgr.hpp"

//==============================================================================
//  STORAGE
//==============================================================================

view            View;
view            View2;
random          R;
s32             ScreenW;
s32             ScreenH;

vector3         s_RayDir(0,1,0);
vector3         s_RayPos(0,0,0);
vector3         s_HitPoint(0,0,0);
xbool           s_bFromCamera = TRUE;

#pragma warning( disable : 4355 ) // warning 'this' used in base member initializer list

#ifdef TARGET_PC
    const char*     DataPath = "Data\\";
#endif

#ifdef TARGET_GCN
    const char*     DataPath = "";
#endif



//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
// MAIN
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

vector3 PortalEdges[]= 
{  
    vector3( -100, 200, 0 ),
    vector3( -100,   0, 0 ),
    vector3(  100,   0, 0 ),
    vector3(  100, 200, 0 ),
};


//==============================================================================

void Initialize( void )
{
    eng_Init();
    render_Init() ;

    View.SetXFOV( R_60 );
    View.SetPosition( vector3( 0, 100, -300 ) );//vector3(654.458f,458.855f,-408.020f) );
    View.LookAtPoint( vector3(  200, 300,  0) );
    View.SetZLimits ( 10, 10000.0f );

    eng_GetRes( ScreenW, ScreenH );

    g_RscMgr.Init();
    
    g_RscMgr.SetRootDirectory( "C:\\GameData\\A51\\Release\\PC");
    //g_RscMgr.SetRootDirectory( "E:\\GameData\\A51\\Release\\PC");
    g_RscMgr.SetOnDemandLoading( TRUE );

    View2 = View;

    //
    //
    //
    bbox BBox;
    BBox.Clear();

    vector3 Edges[8];
    matrix4 L2W;
    s32 i;

    g_ZoneMgr.AddStart( 3, 2 );

    // Add portal 1
    L2W.Identity();
    for( i=0; i<4; i++ ) Edges[i] = L2W * PortalEdges[i];
    g_ZoneMgr.AddPortal( BBox, Edges, 0, 1 );

    // Add portal 1
    L2W.SetTranslation( vector3(20,0,400) );
    for( i=0; i<4; i++ ) Edges[i] = L2W * PortalEdges[i];
    g_ZoneMgr.AddPortal( BBox, Edges, 1, 2 );


    // Add zones
    g_ZoneMgr.AddZone( BBox, 0 );
    g_ZoneMgr.AddZone( BBox, 1 );
    g_ZoneMgr.AddZone( BBox, 2 );

    g_ZoneMgr.AddEnd();
}

//=========================================================================

void Shutdown( void )
{
    // Cleanup geometry
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
        f32    S = 1.125f;
        f32    R = 0.005f;

        if( input_IsPressed( INPUT_KBD_ESCAPE  ) )  return( FALSE );

        if( input_IsPressed( INPUT_KBD_LSHIFT ) )  S *= 4.0f;
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
    }

    return( TRUE );
}

//==============================================================================
/*
void Debug( void )
{
    s32     i;
    s32     Count[2];
    vector3 Edges[2][16];

    //
    // Get ready to start the process
    //
    Count[1] = 4;
    for( i=0; i<4; i++ )
    {
        Edges[1][i] = PortalEdges[i];
    }

    //
    // Start clipping away
    //
    for( i=0; i<g_ZoneMgr.m_Frustum[0].nPlanes; i++)
    {
        const s32 iNew = i&1;
        const s32 iOld = 1 - (i&1);

        g_ZoneMgr.m_Frustum[0].Plane[i].ClipNGon( Edges[iNew], Count[iNew], Edges[iOld], Count[iOld] );
        ASSERT( Count[iNew] < 16 );

        // Did we clip all away?
        if( Count[iNew] == 0 )
            return;
    }

    //
    // Copy the final edges 
    //
    const s32 iNew = 1-(i&1);

    x_srand( 12312 );
    draw_Begin( DRAW_LINES );
    draw_Color( xcolor( 255,0,0,255 ) );

    for( s32 k=0; k<Count[iNew]; k++ )
    {
        draw_Vertex( Edges[iNew][k] );
        draw_Vertex( Edges[iNew][(k+1)%Count[iNew]] );
    }
    draw_End();

}
*/
//==============================================================================

void Render( void )
{
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
    //  Render Skel
    //==---------------------------------------------------
    if( 1 )
    {
        eng_Begin( "Skeleoton" );


        eng_End();
    }

    //==---------------------------------------------------
    //  Render Skel
    //==---------------------------------------------------
    if( 1 )
    {
        //eng_Begin( "Skin" );


        //eng_End();
    }

    //==---------------------------------------------------
    //  Render Skel
    //==---------------------------------------------------
    eng_Begin( "Rect" );

    
    eng_End();    

    //==---------------------------------------------------
    //  Render Skel
    //==---------------------------------------------------
    eng_Begin( "Render Portals" );

    g_ZoneMgr.Render();
    //draw_Frustum( View2 );

    //Debug();
    eng_End();

}

//==============================================================================

void Advance( f32 Seconds )
{
    if( input_IsPressed( INPUT_KBD_SPACE ) )  
    {
        View2 = View;
        g_ZoneMgr.PortalWalk( View, 0 );
    }

    if( input_WasPressed( INPUT_KBD_C ) )  
        s_bFromCamera = !s_bFromCamera;
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
