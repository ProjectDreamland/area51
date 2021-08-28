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

//==============================================================================
//  STORAGE
//==============================================================================

view            View2;
view            View;
random          R;
s32             ScreenW;
s32             ScreenH;
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
static spec_ops_loco s_SpecOpsLoco;

static skin_geom*    s_pGeom ;       // Pointer to skinned geometry
static hinst         s_hInst ;       // Pointer to instance handle

static vector3       s_DesPoint(0,0,0);
static vector3       s_AimPoint(0,0,100);

static vector3       s_DPoint[4];
static s32           s_Cur = 0;



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
}

//=========================================================================

void Shutdown( void )
{
    // Cleanup geometry
    if (s_pGeom)
    {
        render_UnregisterInstance( *s_pGeom, s_hInst ) ;
        delete s_pGeom ;
    }

    render_Kill() ;
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

void RenderSoftSkin( void )
{
    // Create null bitmap anim
    hbitmap_anim hBitmapAnim ;
    hBitmapAnim.Handle = HNULL ;
    
    // Create null uv anim
    huv_anim hUVAnim ;
    hUVAnim.Handle = HNULL ;
    
    // Call render system
    matrix4 L2W ;
    L2W.Identity() ;
    render_Begin("Instances") ;
    render_AddSkinInstance( *s_pGeom,                   // Geometry
                            &L2W,                       // L2W
                            s_SpecOpsLoco.m_Player.GetBoneL2Ws(),   // pBone matrices
                            s_SpecOpsLoco.m_Player.GetNBones(),     // nBone
                            -1,                         // Mask
                            render::NORMAL,             // Flags
                            hBitmapAnim,                // hBitmapAnim
                            hUVAnim) ;                  // hUVAnim
    render_End() ;

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

        s_SpecOpsLoco.RenderSkeleton(TRUE);

        eng_End();
    }

    //==---------------------------------------------------
    //  Render Skel
    //==---------------------------------------------------
    if( 1 )
    {
        //eng_Begin( "Skin" );

        RenderSoftSkin();

        //eng_End();
    }

    //==---------------------------------------------------
    //  Render Skel
    //==---------------------------------------------------
    eng_Begin( "Rect" );

    //rect Rect ( d3deng_GetABSMouseX()-32, d3deng_GetABSMouseY()-32, d3deng_GetABSMouseX()+32, d3deng_GetABSMouseY()+32);
    //draw_Rect( Rect );

    bbox BBox;

    BBox.Min = s_DesPoint - vector3(50,0,50);
    BBox.Max = s_DesPoint + vector3(50,0,50);

    draw_BBox       ( BBox );
    draw_Marker( s_DesPoint );

    BBox.Min = s_AimPoint - vector3(50,0,50);
    BBox.Max = s_AimPoint + vector3(50,0,50);

    draw_BBox       ( BBox, xcolor(0,0,255,255));
    draw_Marker( s_AimPoint, xcolor(0,0,255,255) );
    draw_Marker( s_AimPoint*vector3(1,0,1), xcolor(0,255,0,255) );

    
    eng_End();    
}

//==============================================================================

void Advance( f32 Seconds )
{
    //TEMP!!
    //Seconds = 1.0f/300.0f ;

    //
    // Compute the new fnial destination
    //
    if( input_IsPressed( INPUT_KBD_SPACE ) )
    {
        vector3 RayDir = View.RayFromScreen( d3deng_GetABSMouseX(), d3deng_GetABSMouseY() ) * 100000.0f;
        vector3 RayPos = View.GetPosition();
        plane   Plane( vector3(0,1,0), 0 );
        f32     t;
        if( Plane.Intersect( t, RayPos, RayDir ) )
        {
            s_DesPoint =  RayPos + RayDir*t;
        }
    }

    if( input_IsPressed( INPUT_KBD_C ) )
    {
        vector3 RayDir = View.RayFromScreen( d3deng_GetABSMouseX(), d3deng_GetABSMouseY() ) * 100000.0f;
        vector3 RayPos = View.GetPosition();
        plane   Plane( vector3(0,1,0), 0 );
        f32     t;
        if( Plane.Intersect( t, RayPos, RayDir ) )
        {
            vector3 ColPos = RayPos + RayDir*t;
            s_AimPoint.Set( ColPos.X, s_AimPoint.Y, ColPos.Z );
        }
    }

    if( input_IsPressed( INPUT_KBD_T       ) )  s_AimPoint.Y += 10;
    if( input_IsPressed( INPUT_KBD_G       ) )  s_AimPoint.Y -= 10;
    if( input_WasPressed( INPUT_KBD_U      ) )  s_DesPoint = s_DPoint[s_Cur];

    

    s_SpecOpsLoco.SetLootAt( s_AimPoint );
    s_SpecOpsLoco.SetMoveAt( s_DesPoint );

    //
    // Advance the Logic 
    //

    static f32 SlowMo = 1;
    static f32 Pause=1;
    if( input_WasPressed( INPUT_KBD_P ) )
    {
        if( Pause == 1 ) Pause = 0;
        else Pause = 1;
    }

    if( input_WasPressed( INPUT_KBD_O ) )
    {
        if( SlowMo == 1 ) SlowMo = 0.25f;
        else SlowMo = 1;
    }

    f32 SingleStep = 0;
    if( input_WasPressed( INPUT_KBD_L ) )
    {
        SingleStep += 1/60.0f;
    }

//    x_printfxy( 50, 0, "Pause : %f ", Pause );
//    x_printfxy( 50, 1, "Pause : %f ", SlowMo );

    f32 V = Seconds * Pause * SlowMo + SingleStep;
    if( V > 0 )
        s_SpecOpsLoco.Advance( V );
}


//==============================================================================

void AppMain( s32, char** )
{
    Initialize();
    xtimer Timer;

    // Load geometry
    fileio File;
    File.Load( "C:\\GameData\\A51\\Release\\PC\\Hazmat_bindpose.skingeom", s_pGeom );
    ASSERT(s_pGeom) ;

    // Register with render lib
    render_RegisterGeom( *s_pGeom, geom_node::SKIN );

    // Create instance
    s_hInst = render_RegisterInstance( *s_pGeom ) ;


    s_SpecOpsLoco.Initialize( "SpecOps_rel.anim" );
    s_SpecOpsLoco.m_Player.SetRemoveTurnYaw(TRUE) ;

    s_SpecOpsLoco.SetState( locomotion::STATE_IDLE );
    
    s_DPoint[0].Set( -400, 0 ,0 );
    s_DPoint[1].Set( 300, 0 ,0 );
    s_DPoint[2].Set( 0, 0 ,400 );

    s_DesPoint = s_DPoint[0];

    while( TRUE )
    {
        if( !HandleInput() )
            break;

        eng_MaximizeViewport( View );
        eng_SetView         ( View, 0 );
        eng_ActivateView    ( 0 );

        Advance( Timer.TripSec() );

        Render();

        if( 0 )
        {
            irect Rect( 0,0, 128,128 );

            View2.SetPosition( s_AimPoint + vector3( 0,100,0) );//s_SpecOpsLoco.GetPosition() + vector3( 0, 100, 300 ) );
            View2.LookAtPoint( s_SpecOpsLoco.GetPosition() + vector3( 0, 100,   0 ) );
            View2.SetViewport( Rect.l, Rect.t, Rect.r, Rect.b );
            eng_SetView         ( View2, 0 );
            eng_ActivateView    ( 0 );

            eng_Begin( "Clsar");
            draw_GouraudRect( Rect,XCOLOR_BLACK,XCOLOR_BLACK,XCOLOR_BLACK,XCOLOR_BLACK, FALSE );
            draw_ClearZBuffer( Rect );
            eng_End();

            Render();
        }

        if( (s_SpecOpsLoco.GetPosition()-s_DPoint[s_Cur]).Length() < 100 ) 
        {
            s_Cur = (s_Cur+1)%3;
                
            s_DesPoint = s_DPoint[s_Cur];
        }

        // DONE!
        eng_PageFlip();
    }

    Shutdown();
}

//==============================================================================
