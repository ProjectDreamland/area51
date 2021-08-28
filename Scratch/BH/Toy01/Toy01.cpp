//==============================================================================
//
//  Toy01.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "AsIs/MeshUtil/MeshViewer.hpp"
#include "Auxiliary/Bitmap/aux_Bitmap.hpp"
#include "Auxiliary/fx_Runtime/fx_Mgr.hpp"
#include "Auxiliary/fx_Runtime/fx_SPEmitter.hpp"

//==============================================================================
//  STORAGE
//==============================================================================

view        View;
random      R;
mesh_viewer MeshView;
fx_handle   FXHandle1;
fx_handle   FXHandle2;
s32         LineFeed = 0;

//==============================================================================
//  FUNCTIONS
//==============================================================================

void Initialize( void )
{
    eng_Init();

    View.SetXFOV( R_60 );
    View.SetPosition( vector3(1000,1000,2000) );
//  View.SetPosition( vector3( 100, 100, 200) );
//  View.SetPosition( vector3(   5,   5,  10) );
    View.LookAtPoint( vector3(   0,   0,   0) );
    View.SetZLimits ( 0.1f, 100000.0f );
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

        f32    S = 0.125f;
        f32    R = 0.005f;

        if( input_IsPressed( INPUT_KBD_ESCAPE  ) )  return( FALSE );

        if( input_IsPressed( INPUT_MOUSE_BTN_R ) )  S *= 25.0f;
        if( input_IsPressed( INPUT_KBD_W       ) )  View.Translate( vector3( 0, 0, S), view::VIEW );
        if( input_IsPressed( INPUT_KBD_S       ) )  View.Translate( vector3( 0, 0,-S), view::VIEW );
        if( input_IsPressed( INPUT_KBD_A       ) )  View.Translate( vector3( S, 0, 0), view::VIEW );
        if( input_IsPressed( INPUT_KBD_D       ) )  View.Translate( vector3(-S, 0, 0), view::VIEW );
        if( input_IsPressed( INPUT_KBD_R       ) )  View.Translate( vector3( 0, S, 0), view::VIEW );
        if( input_IsPressed( INPUT_KBD_F       ) )  View.Translate( vector3( 0,-S, 0), view::VIEW );

/*
        View.GetPitchYaw( Pitch, Yaw );
        Pitch += input_GetValue( INPUT_MOUSE_Y_REL ) * R;
        Yaw   -= input_GetValue( INPUT_MOUSE_X_REL ) * R;
        View.SetRotation( radian3(Pitch,Yaw,0) );
*/
        View.RotateY( -input_GetValue( INPUT_MOUSE_X_REL ) * R, view::WORLD );
        View.RotateX(  input_GetValue( INPUT_MOUSE_Y_REL ) * R, view::VIEW  );

        if( input_WasPressed( INPUT_KBD_C ) )   
            FXHandle1.SetColor( xcolor( x_irand(127,255),
                                        x_irand(127,255),                
                                        x_irand(127,255) ) );                

        if( input_WasPressed( INPUT_KBD_T ) )   
            FXHandle1.SetTranslation( vector3( x_frand(-100,100),
                                               x_frand(-100,100),                
                                               x_frand(-100,100) ) );                

        if( input_WasPressed( INPUT_KBD_Q ) )   
        {
            FXHandle1.SetColor( XCOLOR_WHITE );
            FXHandle1.SetTranslation( vector3(0,0,0) );
        }

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
    }

    return( TRUE );
}

//==============================================================================

void Render( void )
{
    LineFeed++;
    if( LineFeed >= 60 )
    {
        x_printf( "\n" );
        LineFeed = 0;
    }

    eng_MaximizeViewport( View );
    eng_SetView         ( View, 0 );
    eng_ActivateView    ( 0 );

    eng_Begin();
    draw_Rect( rect(0,0,640,640), xcolor(31,31,31), FALSE );
    eng_End();

    eng_Begin();
    {
        draw_ClearL2W();
        draw_Grid( vector3(  -10,    0,  -10 ), 
                   vector3(   20,    0,    0 ), 
                   vector3(    0,    0,   20 ), 
                   xcolor (    0,  128,    0 ), 20 );
        draw_Grid( vector3( -100,    0, -100 ), 
                   vector3(  200,    0,    0 ), 
                   vector3(    0,    0,  200 ), 
                   xcolor (    0,   64,    0 ), 20 );
        //draw_Marker( R.v3( 50,75,0,25,50,75 ), R.color() );
        draw_Point( vector3( 100, 100, 0 ), XCOLOR_GREEN );
        //draw_BBox( bbox(vector3(50,0,50),vector3(75,25,75)) );
    }
    eng_End();

    /*
    eng_Begin();
    {
        draw_Begin( DRAW_QUADS, DRAW_2D | DRAW_USE_ALPHA | DRAW_NO_ZBUFFER );
        draw_Color ( xcolor( 255,255,255,  0 ) ); draw_Vertex( vector3(  0,  0, 10) ); 
        draw_Color ( xcolor( 255,255,255,255 ) ); draw_Vertex( vector3(  0,255, 10) ); 
        draw_Color ( xcolor( 255,255,255,255 ) ); draw_Vertex( vector3(100,255, 10) ); 
        draw_Color ( xcolor( 255,255,255,  0 ) ); draw_Vertex( vector3(100,  0, 10) ); 
        draw_End();
    }
    eng_End();
    */

    eng_Begin();
    {
        matrix4 M;
        M.Identity();
        draw_ClearL2W();
        g_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&M );
        //MeshView.Render();
        //bbox BBox( MeshView.GetBBox() );
        //draw_Marker( BBox.Max, XCOLOR_BLUE );
        //draw_Marker( BBox.Min, XCOLOR_BLUE );
        //draw_BBox( BBox, XCOLOR_BLUE );
    }
    eng_End();

    eng_Begin();
    FXHandle1.Render();
//  FXHandle2.Render();
    eng_End();

    eng_PageFlip();
}

//==============================================================================

xbool Toggle = TRUE;

void AppMain( s32, char** )
{
    Initialize();
//  CreateEffectFile();

//  MeshView.Load( "TrollholeWaterfall.MATX" );
    // MeshView.PlayAnimation();

    x_MemAddMark( "Before Loading" );

    FXDebug.ElementWire   = 1;
//  FXDebug.EffectAxis    = 1;
    FXDebug.EffectBounds  = 1;
    FXDebug.ElementBounds = 1;

    FXMgr.LoadEffect( "Test", "C:/GameData/A51/Release/PC/Muzzleflash_1stperson_SMP_000.fxo" );
//  FXMgr.LoadEffect( "Test", "C:/GameData/Meridian/Resources/PC/Common/Effects/LT_Tikitorch02.fxo" );
//  FXMgr.LoadEffect( "Test", "C:/Projects/Tools/Apps/fx_Editor/Samples/Export/JN-05.fxo" );
//  FXMgr.LoadEffect( "Test", "C:/Projects/Tools/Apps/fx_Editor/Samples/Export/Untitled.fxo" );
//  FXMgr.LoadEffect( "Test", "C:/Projects/Tools/Apps/fx_Editor/Samples/Export/JV_Shockwaves.fxo" );
//  FXMgr.LoadEffect( "Test", "C:/Projects/Tools/Apps/fx_Editor/Samples/Export/JV_ShockMania.fxo" );

    FXHandle1.InitInstance( "Test" );
//  FXHandle2.InitInstance( "Test" );

//  FXHandle1.SetScale( vector3( 0.2f, 0.2f, 0.2f ) );

    FXHandle1.SetTranslation( vector3(   100, 1000, 0 ) );
//  FXHandle2.SetTranslation( vector3( -1000, 0, 0 ) );

    x_MemAddMark( "After Loading" );

    xtimer Timer;
    xtimer Rotate;

    Rotate.Start();

    //x_MemDump( "0.txt" );

//  FXHandle1.SetTranslation( vector3(50,0,20) );

    while( TRUE )
    {
        Render();

        f32 DeltaTime = Timer.TripSec();

        FXHandle1.AdvanceLogic( DeltaTime );
//      FXHandle2.AdvanceLogic( DeltaTime );
//      FXHandle1.AdvanceLogic( 1.0f / 50.0f );
//      FXHandle1.SetRotation( radian3( Rotate.ReadSec() * 0.25f, Rotate.ReadSec(), R_0 ) );
//      FXHandle1.SetScale   ( vector3( 1.1f + x_sin( Rotate.ReadSec() ),
//                                     1.1f + x_sin( Rotate.ReadSec() ),
//                                     1.1f + x_sin( Rotate.ReadSec() ) ) );
//      FXHandle1.SetTranslation( vector3( 0.0f, x_sin( Rotate.ReadSec() * 0.5f ) * 50.0f, 0.0f ) );


        /*
        if( input_IsPressed( INPUT_KBD_SPACE, 0 ) )
        {
            FXHandle1.AdvanceLogic( DeltaTime );
//          FXHandle2.AdvanceLogic( DeltaTime / 2.0f );

            FXHandle1.SetRotation( radian3( Rotate.ReadSec() * 0.25f, Rotate.ReadSec(), R_0 ) );
//          FXHandle1.SetScale   ( vector3( 2.0f + x_sin( Rotate.ReadSec() ), 1.0f, 1.0f ) );
        }
        */

        if( !HandleInput() )
            break;

        /*
        if( input_WasPressed( INPUT_KBD_SPACE, 0 ) )
        {
            Toggle = !Toggle;
            FXHandle1.SetSuspended( Toggle );
        }
        */

        if( input_WasPressed( INPUT_KBD_Q, 0 ) )
        {
            FXHandle1.Restart();
            FXHandle1.SetTranslation( vector3(   100, 100, 0 ) );
//          FXHandle2.Restart();
        }

        /*
        if( input_WasPressed( INPUT_KBD_SPACE, 0 ) )
        {
            FXHandle1.AdvanceLogic( 1.0f / 30.0f );
        }
        */

        if( FXHandle1.IsFinished() )
            x_printfxy( 1, 1, "DONE" );

        FXMgr.EndOfFrame();
    }

    //x_MemDump( "1.txt" );

    FXHandle1.KillInstance();
//  FXHandle2.KillInstance();
    FXMgr.UnloadEffect( "Test" );

    //x_MemDump( "2.txt" );
}

//==============================================================================
