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
#include "Locomotion.hpp"
#include "MeshUtil\RawMesh2.hpp"
#include "LocoSpecialOps.hpp"
#include "Render\Render.hpp"
#include "Render\RigidGeom.hpp"
#include "Render\SkinGeom.hpp"

#include "Lighting.hpp"
#include <stdio.h>
//==============================================================================
//  STORAGE
//==============================================================================

view            View2;
view            View;
random          R;
s32             ScreenW;
s32             ScreenH;
lighting        Lighting;

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

//==============================================================================

void Initialize( void )
{
    eng_Init();
    render_Init() ;

    View.SetXFOV( R_60 );
    View.SetPosition( vector3(654.458f,458.855f,-408.020f) );
    View.LookAtPoint( vector3(  0,  0,  0) );
    View.SetZLimits ( 0.1f, 10000.0f );

    View2 = View;

    eng_GetRes( ScreenW, ScreenH );

    g_RscMgr.Init();
    
    g_RscMgr.SetRootDirectory( "C:\\GameData\\A51\\Release\\PC");
    //g_RscMgr.SetRootDirectory( "E:\\GameData\\A51\\Release\\PC");
    g_RscMgr.SetOnDemandLoading( TRUE );



    //
    // Read all the facets
    //
    FILE* Fp;
    s32   Count=0;

    Fp = fopen( "Big_TestLight.xxx", "rt" );
    while( feof( Fp ) == FALSE )
    {
        vector3 P[3];
        vector3 N[3];
        
        for( s32 i=0;i<3;i++)
        {
            fscanf( Fp, "%f%f%f%f%f%f", &P[i].X, &P[i].Y, &P[i].Z, &N[i].X, &N[i].Y, &N[i].Z );
        }

        Lighting.AddFacet( P[0], N[0], 
                           P[1], N[1], 
                           P[2], N[2], 
                             0, 0 );
    }    
    fclose( Fp );

    //
    // Read all the lights
    //
    Fp = fopen( "Big_TestLight_L.xxx", "rt" );
    while( feof( Fp ) == FALSE )
    {
        vector3 P;
        f32     R;
        s32     r1,g1,b1;
        s32     r2,g2,b2;

        fscanf( Fp, "%f%f%f%f%d%d%d%d%d%d", &P.X, &P.Y, &P.Z, &R, &r1,&g1,&b1,&r2,&g2,&b2 );
        
        Lighting.AddLight( P, R, xcolor(r1,g1,b1,255), xcolor(r2,g2,b2,255) );
    }    
    fclose( Fp );

    //
    // Compile The data
    //
    Lighting.CompileData();
    Lighting.ComputeLighting();
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
        f32    S = 16.125f;
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
    }

    return( TRUE );
}

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

    Lighting.Render();
    
    eng_End();    

    //==---------------------------------------------------
    //  Render Skel
    //==---------------------------------------------------
    eng_Begin( "ray" );

    draw_Line  ( s_RayPos, s_HitPoint, xcolor( 255,255,0,255) );
    draw_Line  ( s_RayPos, s_RayPos+s_RayDir, xcolor( 0,255,0,255) );
    draw_Marker( s_HitPoint, xcolor( 255,0,0,255) );
    eng_End();    

    Lighting.CheckCollision( s_RayPos, s_RayDir );
}

//==============================================================================

void Advance( f32 Seconds )
{
    if( input_IsPressed( INPUT_KBD_SPACE ) )  
    {
        s_RayDir = View.RayFromScreen( d3deng_GetABSMouseX(), d3deng_GetABSMouseY() );

        if( s_bFromCamera )
            s_RayPos = View.GetPosition();

        s_RayDir *= 10000.0f;
        if( Lighting.CheckCollision( s_RayPos, s_RayDir ) )
        {
            s_HitPoint = Lighting.GetHitPoint();
        }            
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
