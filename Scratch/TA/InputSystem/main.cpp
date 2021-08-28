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
#include "Animation\AnimPlayer.hpp"
#include "Animation\CharAnimPlayer.hpp"
#include "C:\Projects\A51\Support\InputMgr\inputmgr.hpp"

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


//=========================================================================

class ingame_pad : public input_pad
{
public:

    enum
    {
        MOVE_STRAFE,
        MOVE_FOWARD_BACKWARDS,
        LOOK_HORIZONTAL,
        LOOK_VERTICAL,
        ACTION_JUMP,
        ACTION_CROUCH,
        ACTION_PRIMARY,
        ACTION_SECONDARY,
        ACTION_RELOAD,
    };

public:

                        ingame_pad      ( void );
    virtual void        OnUpdate        ( f32 DeltaTime );
    virtual void        OnInitialize    ( void );

protected:
protected:

};

//=========================================================================

ingame_pad::ingame_pad( void )
{
}

//=========================================================================

void ingame_pad::OnInitialize( void )
{
    //
    // Set all my logical actions
    //
    SetLogical( MOVE_STRAFE,            "Strafe" );
    SetLogical( MOVE_FOWARD_BACKWARDS,  "Fowards/Backwards" );
    SetLogical( LOOK_HORIZONTAL,        "Horizontral Look" );
    SetLogical( LOOK_VERTICAL,          "Vertical Look" );

    SetLogical( ACTION_RELOAD,          "Reload" );
    SetLogical( ACTION_PRIMARY,         "Primary" );
    SetLogical( ACTION_SECONDARY,       "Secondary" );
    SetLogical( ACTION_JUMP,            "Jump" );
    SetLogical( ACTION_CROUCH,          "Crouch" );

    //
    // Set the default controler
    //
    AddMapping( MOVE_STRAFE,            INPUT_PS2_STICK_LEFT_X, FALSE );
    AddMapping( MOVE_FOWARD_BACKWARDS,  INPUT_PS2_STICK_LEFT_Y, FALSE );

    AddMapping( LOOK_HORIZONTAL,        INPUT_PS2_STICK_RIGHT_X, FALSE );
    AddMapping( LOOK_VERTICAL,          INPUT_PS2_STICK_RIGHT_Y, FALSE );

    AddMapping( ACTION_RELOAD,          INPUT_PS2_BTN_SQUARE,   TRUE );
    AddMapping( ACTION_PRIMARY,         INPUT_PS2_BTN_R1,       TRUE );
    AddMapping( ACTION_SECONDARY,       INPUT_PS2_BTN_R2,       TRUE );

    AddMapping( ACTION_JUMP,            INPUT_PS2_BTN_L1,       TRUE );
    AddMapping( ACTION_CROUCH,          INPUT_PS2_BTN_L2,       TRUE );
}

//=========================================================================

void ingame_pad::OnUpdate( f32 DeltaTime )
{
    //
    // First lets read all the input what we need
    //
    input_pad::OnUpdate( DeltaTime );

    // Nothing to do for now
    OnDebugRender();
}


//==============================================================================
static ingame_pad IngamePad;

void Initialize( void )
{
    eng_Init();

    View.SetXFOV( R_70 );
    View.SetPosition( vector3(654.458f,458.855f,-408.020f) );
    View.LookAtPoint( vector3(  0,  0,  0) );
    View.SetZLimits ( 0.1f, 10000.0f );

    View2 = View;

    g_InputMgr.RegisterPad( IngamePad );


    eng_GetRes( ScreenW, ScreenH );

    g_RscMgr.Init();
    
    g_RscMgr.SetRootDirectory( "C:\\GameData\\A51\\Release\\PC");
    //g_RscMgr.SetRootDirectory( "E:\\GameData\\A51\\Release\\PC");
    g_RscMgr.SetOnDemandLoading( TRUE );
}

//=========================================================================

void Shutdown( void )
{
}

//=========================================================================

xbool HandleInput( void )
{
    if( g_InputMgr.Update( 0 ) )
        return FALSE;

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


    JOYINFO Info;
    joyGetPos( 0, &Info );


    JOYINFOEX ExInfo;

    x_memset( &ExInfo, 0, sizeof(ExInfo) );
    ExInfo.dwSize = sizeof(ExInfo);
    ExInfo.dwFlags = JOY_RETURNALL ;
    joyGetPosEx( 0, &ExInfo );
    

    for( s32 i=0; i<32;i++)
    {
        x_printfxy(0,i," %3d %d", i, ExInfo.dwButtons&(1<<i) );
    }

    i = 0;
    x_printfxy(10,i++,"%d", ExInfo.dwXpos );
    x_printfxy(10,i++,"%d", ExInfo.dwYpos );
    x_printfxy(10,i++,"%d", ExInfo.dwZpos );

    x_printfxy(10,i++,"%d", ExInfo.dwRpos );
    x_printfxy(10,i++,"%d", ExInfo.dwUpos );
    x_printfxy(10,i++,"%d", ExInfo.dwVpos );



    x_printfxy(0,i,"%d", Info.wButtons&(1<<i) );

    x_DebugMsg( "joyGetNumDevs: %d\n", joyGetNumDevs() );

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
        eng_Begin( "Skin" );


        //IngamePad

        eng_End();
    }

}

//==============================================================================

void Advance( f32 Seconds )
{
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
