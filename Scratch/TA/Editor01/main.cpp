
#include "Entropy.hpp"
#include "Geometry\geom.hpp"
#include "MeshUtil\MatxMisc.hpp"
#include "resource.h"
#include "Grid3d.hpp"
#include "Axis3d.hpp"
#include "LayoutEditor.hpp"
#include <commctrl.h>

#pragma comment( lib, "comctl32" )


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


enum edit_mode
{
    EDIT_MODE_NULL,
    EDIT_ADD_INSTANCE,
    EDIT_ADD_WITH_RAY,
    EDIT_SEL_INSTANCE,
    EDIT_UNSEL_INSTANCE,
    EDIT_DEL_INSTANCE,
    EDIT_ADD_ROTATE,
    EDIT_ADD_LIGHT,
    EDIT_ADD_SOUND,
    EDIT_ADD_PFX,
    EDIT_ADD_PICKUP,
    EDIT_ADD_PICKUP_GRID,
    EDIT_ADD_GLOW,
};


static view     View;
static s32      ScreenW;
static s32      ScreenH;

static char*            s_pDir = NULL;
static map              s_Map;
static HWND             s_hMainWindow;
static HWND             s_hDialog;
static HWND             s_hMatDialog;
static HWND             s_hPickupDialog;
static grid3d           s_Grid;
static axis3d           s_Axis;
static vector3          s_GridPos(0,0,0);
static xbool            s_OrbitCam;
static f32              s_tLastFrame=0;
static f32              s_SnapStep  = 100;
static f32              s_LightI    = 1;
static f32              s_GlowI    = 1;
static xcolor           s_LightC(0xffffffff);
static xcolor           s_GlowC(0xffffffff);
f32              s_LightR               = 1000;
f32              s_GlowR                = 100;
f32              s_SoundR               = 100;
s32              s_SoundT               = 0;
s32              s_EffectsContainerId   = -1;

static char             s_InitialDirPrj[256]  = { "C:\\GameData\\A51\\Source\\Prototype" };
static char             s_InitialDirExp[256]  = { "C:\\GameData\\A51\\Release\\PC" };
static char             s_ProjectName[256]    = { 0 };
static char             s_ExportName[256]     = { 0 };


static vector3          s_FocusPos(0,0,0);
vector3          s_NormalPos(0,0,0);
static s32              s_FocusOrient = 0;
static quaternion       s_FocusFreeOrient(0,0,0,1);
static s32              s_bFocusFreeOrient = FALSE;

static layout_editor    s_Editor;
static edit_mode        s_EditMode = EDIT_MODE_NULL;

static char             s_ObjectName[64];
static s32              s_ObjectType;       

//=========================================================================

void UpdatePickupList( void );

//=========================================================================

xbool HandleInput( void )
{
    xbool ones = FALSE;

    while( input_UpdateState() )
    {
        radian Pitch =0 ;
        radian Yaw =0;
        f32    S = 128.125f;
        f32    R = 0.005f;

        if( 0 )
        {
            //View.GetPitchYaw( Pitch, Yaw );       
            Pitch = input_GetValue( INPUT_MOUSE_Y_REL ) * R*10;
            Yaw   = input_GetValue( INPUT_MOUSE_X_REL ) * R*10;
            f32 r = input_GetValue( INPUT_MOUSE_WHEEL_REL ) * -R*10;

            View.Translate( vector3( 0, 0, -r*2000), view::VIEW );
            View.Translate( vector3( Yaw*S*2, 0, 0 ), view::VIEW );
            View.Translate( vector3( 0, Pitch*S*2, 0 ), view::VIEW );

            if( input_IsPressed( INPUT_KBD_W       ) )  s_FocusPos += vector3( 0, 0, 100);
            if( input_IsPressed( INPUT_KBD_S       ) )  s_FocusPos += vector3( 0, 0,-100);
            if( input_IsPressed( INPUT_KBD_A       ) )  s_FocusPos += vector3( 100, 0, 0);
            if( input_IsPressed( INPUT_KBD_D       ) )  s_FocusPos += vector3(-100, 0, 0);

        }
        else
        {
            S *= s_tLastFrame;

            if( input_IsPressed( INPUT_MOUSE_BTN_R ) )  S *= 4.0f;
            if( input_IsPressed( INPUT_KBD_W       ) )  View.Translate( vector3( 0, 0, S), view::VIEW );
            if( input_IsPressed( INPUT_KBD_S       ) )  View.Translate( vector3( 0, 0,-S), view::VIEW );
            if( input_IsPressed( INPUT_KBD_A       ) )  View.Translate( vector3( S, 0, 0), view::VIEW );
            if( input_IsPressed( INPUT_KBD_D       ) )  View.Translate( vector3(-S, 0, 0), view::VIEW );
            if( input_IsPressed( INPUT_KBD_R       ) )  View.Translate( vector3( 0, S, 0), view::VIEW );
            if( input_IsPressed( INPUT_KBD_F       ) )  View.Translate( vector3( 0,-S, 0), view::VIEW );

            if( input_IsPressed( INPUT_MOUSE_BTN_C ) )
            {
                f32 r = input_GetValue( INPUT_MOUSE_Y_REL ) * -R*100;
                View.Translate( vector3( 0, r, 0) );
            }
            else
            {
                f32 r = input_GetValue( INPUT_MOUSE_WHEEL_REL ) * R*10000;
                View.Translate( vector3( 0,r, 0) );

                View.GetPitchYaw( Pitch, Yaw );       
                Pitch += input_GetValue( INPUT_MOUSE_Y_REL ) * R;
                Yaw   -= input_GetValue( INPUT_MOUSE_X_REL ) * R;

                Pitch = fMax( Pitch, -(R_90-R_1) );
                Pitch = fMin( Pitch, R_90-R_1 );
                View.SetRotation( radian3(Pitch,Yaw,0) );
            }
        }

        if( input_IsPressed( INPUT_MSG_EXIT ) )
            return( FALSE );


        if( input_IsPressed( INPUT_MSG_EXIT ) )
 
        void    SetL2W          ( matrix4& L2W );

    }

    if( 0 )
    {
        View.LookAtPoint ( s_FocusPos );
    }


    return( TRUE );
}

//=========================================================================
static
void UpdatePieceList( void )
{
    HWND Item = GetDlgItem( s_hDialog, IDC_PIECES_LIST );

    for( s32 i=0; i<s_Editor.GetNDescs(); i++ )
    {
        layout_editor::piece_desc& Piece = s_Editor.GetDesc( i ); 
        SendMessage( Item, LB_ADDSTRING, 0, (u32)Piece.FileName );
    }
}

//=========================================================================
static 
void QuickView( void )
{
    static radian Rot = 0;
    e_begin;

    HWND Item       = GetDlgItem( s_hDialog, IDC_PIECES_LIST );
    s32  Index      = SendMessage( Item, LB_GETCURSEL, 0, 0 );

    if( Item == 0 || Index ==  LB_ERR ) 
    {
        // TODO: Clear the quick view window
        return;
    }

    char Name[256];
    view QuicView   = View;

    // Get the name of the piece
    SendMessage( Item, LB_GETTEXT, (u32)Index, (u32)Name );
    layout_editor::piece_desc& Desc = s_Editor.GetDesc( Name ); 

    Rot += s_tLastFrame * 1;

    // Setup the view 
    QuicView.OrbitPoint( vector3( 0, 0, 0 ), 1000, -R_45, Rot );

    d3deng_UpdateDisplayWindow( GetDlgItem( s_hDialog, IDC_QUICK_VIEW ));
    eng_MaximizeViewport( QuicView );
    eng_SetView         ( QuicView, 0 );
    eng_ActivateView    ( 0 );
    matrix4 L2W;

    L2W.Identity();

    gman_Begin( "QuickView" );

        //
        // Set the lighting
        //
        vector3 LightDir( 0.5, -0.5, 0.5f );
        vector3 LightDir2( -0.5, 0.5, -0.5f );

        LightDir.Normalize();
        LightDir2.Normalize();
        LightDir *= 0.5f;
        LightDir2 *= 0.5f;

	    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING,		TRUE);
        g_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );

        d3deng_SetAmbientLight( xcolor( 32, 32, 32, 255 ) );

        d3deng_SetLight( 0, LightDir, xcolor(255,255,255,255) );
        d3deng_SetLight( 1, LightDir2, xcolor(255,255,255,255) );

        g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, xcolor(64,64,64,255) );
        g_pd3dDevice->SetTransform( D3DTS_WORLD,      (D3DMATRIX*)&L2W );
        g_pd3dDevice->SetTransform( D3DTS_VIEW,       (D3DMATRIX*)&QuicView.GetW2V() );
        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE  );
        g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE    );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE    );

        g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_ONE );
        g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_ZERO );

        gman_Render( L2W, Desc.hGeom );

    gman_End();

    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING,		FALSE );

    eng_PageFlip();

    e_handle;
}

//=========================================================================
static
void ComputeFocusMatrix( matrix4& L2W )
{
    if( s_bFocusFreeOrient == FALSE )
    {
        switch( s_FocusOrient )
        {
        case 0:
            L2W.Identity();
            L2W.SetTranslation( s_FocusPos );
            break;

        case 1:
            L2W.Identity();
            L2W.SetColumns( vector3( 0,0,-1 ), vector3( 0,1,0 ), vector3( 1,0,0 ) );
            L2W.SetTranslation( s_FocusPos );
            break;

        case 2:
            L2W.Identity();
            L2W.SetColumns( vector3( -1,0,0 ), vector3( 0,1,0 ), vector3( 0,0,-1 ) );
            L2W.SetTranslation( s_FocusPos );
            break;

        case 3:
            L2W.Identity();
            L2W.SetColumns( vector3( 0,0,1 ), vector3( 0,1,0 ), vector3( -1,0,0 ) );
            L2W.SetTranslation( s_FocusPos );
            break;
        }
        
        // Set the quaternion to be roateted like the matrix
        s_FocusFreeOrient.Setup( L2W );
    }
    else
    {
        L2W.Setup( vector3( 1,1,1 ), s_FocusFreeOrient, s_FocusPos );
    }
}

//=========================================================================

void RenderLights( void )
{
    eng_Begin( "RENDER MAP LIGHTS" );

    //g_pd3dDevice->SetRenderState( D3DRS_LIGHTING,	  FALSE );

    for( s32 i=0; i<s_Editor.GetLightCount(); i++ )
    {
        layout_editor::light& Light = s_Editor.GetLight( i );

        if( Light.Flags & layout_editor::INST_FLAGS_SELECTED ) 
        {
            bbox BBox( Light.Sphere.GetBBox() );

            draw_Sphere( Light.Sphere.Pos, 50, xcolor( 255,0, 0,255) );
            draw_BBox  ( BBox, Light.Color);
        }
        else
        {
            draw_Sphere( Light.Sphere.Pos, 50, xcolor(255,255,255,255) );
        }
    }

    eng_End();
}

//=========================================================================

void RenderGlow( void )
{
    eng_Begin( "RENDER MAP GLOWS" );

    //g_pd3dDevice->SetRenderState( D3DRS_LIGHTING,	  FALSE );

    for( s32 i=0; i<s_Editor.GetGlowCount(); i++ )
    {
        layout_editor::glow& Glow = s_Editor.GetGlow( i );

        if( Glow.Flags & layout_editor::INST_FLAGS_SELECTED ) 
        {
            bbox BBox( Glow.Sphere.GetBBox() );

            draw_Sphere( Glow.Sphere.Pos, 50, xcolor( 255,0, 0,255) );
            draw_BBox  ( BBox, Glow.Color);
        }
        else
        {
            draw_Sphere( Glow.Sphere.Pos, 50, xcolor(255,255,0,255) );
        }
    }

    eng_End();
}

//=========================================================================

void RenderSounds( void )
{
    s32 i;

    eng_Begin( "RENDER MAP SOUNDS" );
    
    draw_Begin( DRAW_SPRITES );
    draw_ClearL2W();

    for( i=0; i<s_Editor.GetSoundCount(); i++ )
    {
        layout_editor::sound_inst& Sound = s_Editor.GetSound( i );

        if( Sound.Flags & layout_editor::INST_FLAGS_SELECTED ) 
        {
            draw_Sprite( Sound.Sphere.Pos, vector2( 100,100 ), xcolor( 255,0,0,255 ) );
        }
        else
        {
            draw_Sprite( Sound.Sphere.Pos, vector2( 100,100 ), xcolor( 0,255,0,255 ) );
        }
    }

    draw_End();

    for( i=0; i<s_Editor.GetSoundCount(); i++ )
    {
        layout_editor::sound_inst& Sound = s_Editor.GetSound( i );

        if( Sound.Flags & layout_editor::INST_FLAGS_SELECTED ) 
        {
            draw_BBox  ( Sound.Sphere.GetBBox(), xcolor( 255,0,0,255 ) );
        }
    }
    eng_End();

}

//=========================================================================

void RenderPfx( void )
{
    eng_Begin( "RENDER MAP PFX" );

    for( s32 i=0; i<s_Editor.GetParticleCount(); i++ )
    {
        layout_editor::pfx_inst& Pfx = s_Editor.GetParticle( i );

        if( Pfx.Flags & layout_editor::INST_FLAGS_SELECTED ) 
        {
            bbox BBox( Pfx.Sphere.GetBBox() );

            draw_Sphere( Pfx.Sphere.Pos, PFX_SPHERE_RADIUS, xcolor( 255,0, 0,255) );
            draw_BBox  ( BBox, xcolor( 0,0, 255,255));
        }
        else
        {
            draw_Sphere( Pfx.Sphere.Pos, PFX_SPHERE_RADIUS, xcolor(0,0,255,255) );
        }
    }

    eng_End();
}

//=========================================================================

void RenderPickup( void )
{
    eng_Begin( "RENDER MAP PICKUP" );

    for( s32 i=0; i<s_Editor.GetPickupCount(); i++ )
    {
        layout_editor::pickup_inst& Pickup = s_Editor.GetPickup( i );

        if( Pickup.Flags & layout_editor::INST_FLAGS_SELECTED ) 
        {
            bbox BBox( Pickup.Sphere.GetBBox() );

            draw_Sphere( Pickup.Sphere.Pos, PICKUP_SPHERE_RADIUS, xcolor( 0,255, 0,255) );
            draw_BBox  ( BBox, xcolor( 0,255, 0,255));
        }
        else
        {
            draw_Sphere( Pickup.Sphere.Pos, PICKUP_SPHERE_RADIUS, xcolor(0,255,0,255) );
        }
    }

    eng_End();
}

//=========================================================================

void RenderAddLight( void )
{
    matrix4 L2W;
    ComputeFocusMatrix( L2W );
    eng_Begin( "RENDER LIGHT ADD" );
    vector3 Pos = L2W.GetTranslation(); 

    bbox BBox;
    BBox.Clear();
    BBox.Min = ( Pos - vector3( s_LightR, s_LightR, s_LightR ) );
    BBox.Max = ( Pos + vector3( s_LightR, s_LightR, s_LightR ) );

    draw_Sphere( Pos, LIGHT_SPHERE_RADIUS, xcolor( 255,0,0,255));
    draw_BBox  ( BBox,xcolor( 255,0,0,255));
    eng_End(  );

}

//=========================================================================

void RenderAddGlow( void )
{
    matrix4 L2W;
    ComputeFocusMatrix( L2W );
    eng_Begin( "RENDER GLOW ADD" );
    vector3 Pos = L2W.GetTranslation(); 

    bbox BBox;
    BBox.Clear();
    BBox.Min = ( Pos - vector3( s_GlowR, s_GlowR, s_GlowR ) );
    BBox.Max = ( Pos + vector3( s_GlowR, s_GlowR, s_GlowR ) );

    draw_Sphere( Pos, GLOW_SPHERE_RADIUS, xcolor( 255,0,0,255));
    draw_BBox  ( BBox,xcolor( 255,0,0,255));
    eng_End(  );

}

//=========================================================================

void RenderAddSound( void )
{
    matrix4 L2W;
    ComputeFocusMatrix( L2W );
    eng_Begin( "RENDER SOUND ADD" );
    vector3 Pos = L2W.GetTranslation(); 

    draw_Begin( DRAW_SPRITES );
    draw_ClearL2W();
    draw_Sprite( Pos, vector2( 100,100 ), xcolor( 255,0,0,255 ) );
    draw_End();

    bbox BBox;
    BBox.Clear();
    BBox.Min = ( Pos - vector3( s_SoundR, s_SoundR, s_SoundR ) );
    BBox.Max = ( Pos + vector3( s_SoundR, s_SoundR, s_SoundR ) );

    draw_BBox  ( BBox,xcolor( 255,0,0,255));
    eng_End(  );

}

//=========================================================================

void RenderAddPfx( void )
{
    matrix4 L2W;
    ComputeFocusMatrix( L2W );
    eng_Begin( "RENDER PFX ADD" );
    vector3 Pos = L2W.GetTranslation(); 

    bbox BBox;
    BBox.Clear();
    BBox.Min = ( Pos - vector3( PFX_SPHERE_RADIUS, PFX_SPHERE_RADIUS, PFX_SPHERE_RADIUS ) );
    BBox.Max = ( Pos + vector3( PFX_SPHERE_RADIUS, PFX_SPHERE_RADIUS, PFX_SPHERE_RADIUS ) );

    draw_Sphere( Pos, PFX_SPHERE_RADIUS, xcolor( 255,0,0,255));
    draw_BBox  ( BBox,xcolor( 255,0,0,255));
    eng_End(  );

}

//=========================================================================

void RenderAddPickup( void )
{
    matrix4 L2W;
    ComputeFocusMatrix( L2W );
    eng_Begin( "RENDER PICKUP ADD" );
    vector3 Pos = L2W.GetTranslation(); 

    bbox BBox;
    BBox.Clear();
    BBox.Min = ( Pos - vector3( PICKUP_SPHERE_RADIUS, PICKUP_SPHERE_RADIUS, PICKUP_SPHERE_RADIUS ) );
    BBox.Max = ( Pos + vector3( PICKUP_SPHERE_RADIUS, PICKUP_SPHERE_RADIUS, PICKUP_SPHERE_RADIUS ) );

    draw_Sphere( Pos, PICKUP_SPHERE_RADIUS, xcolor( 255,0,0,255));
    draw_BBox  ( BBox,xcolor( 255,0,0,255));
    eng_End(  );

}

//=========================================================================
void RenderAddMode( void )
{
    e_begin;

    char Name[256];
    HWND Item       = GetDlgItem( s_hDialog, IDC_PIECES_LIST );
    s32  Index      = SendMessage( Item, LB_GETCURSEL, 0, 0 );

    if( Item == 0 || Index ==  LB_ERR ) 
        return;

    // Get the name of the piece
    SendMessage( Item, LB_GETTEXT, (u32)Index, (u32)Name );
    layout_editor::piece_desc& Desc = s_Editor.GetDesc( Name ); 
    matrix4 L2W;
    
    ComputeFocusMatrix( L2W );

    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE    );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE    );

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE  );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE  );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR  );

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR     );


    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

    g_pd3dDevice->SetRenderState( D3DRS_ZBIAS, 10 );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_ONE );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_ONE );

    //g_pd3dDevice->SetRenderState( D3DRS_LIGHTING,	  TRUE );


    gman_Begin( "RenderAddMode" );
    g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, xcolor( 0,255,0,128) );
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,      D3DCULL_CW );
    gman_Render( L2W, Desc.hGeom );
    gman_End();

    gman_Begin( "RenderAddMode" );
    g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, xcolor( 0,0,255,128) );
    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,     D3DCULL_CCW );
    gman_Render( L2W, Desc.hGeom );
    gman_End();


    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,     D3DCULL_CW );

    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_ONE );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_ZERO );

    // Clean the shader
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE    );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE    );

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE  );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE  );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE  );

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE     );
    g_pd3dDevice->SetRenderState( D3DRS_ZBIAS, 0 );

    e_handle;
}

//=========================================================================
static 
void PuchHole( wall_geom::hole& Hole, matrix4& L2W )
{
    // Set the rendering stuff
    g_pd3dDevice->SetVertexShader( D3DFVF_XYZ );
    g_pd3dDevice->SetTransform   ( D3DTS_WORLD, (D3DMATRIX*)&L2W );

    if( 1 )
    {
        g_pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0 ); 
        g_pd3dDevice->SetRenderState( D3DRS_ZFUNC,            D3DCMP_LESS );
        g_pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    TRUE );
        g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,     FALSE );        

        // prapare the stencil stuff
        // (StencilRef & StencilMask) CompFunc (StencilBufferValue & StencilMask)
        // The current pixel is written to the target surface if the stencil test 
        // passes, and is ignored otherwise.
        g_pd3dDevice->SetRenderState( D3DRS_STENCILMASK,  ~0 );
        g_pd3dDevice->SetRenderState( D3DRS_STENCILREF,    0 );
        g_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,   D3DCMP_ALWAYS );

        for( s32 i=0; i<3;i++ )
        {
            if( i == 0 )
            {
                g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,     D3DCULL_CCW );
                g_pd3dDevice->SetRenderState( D3DRS_STENCILFAIL,  D3DSTENCILOP_KEEP );
                g_pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_INCR );
                g_pd3dDevice->SetRenderState( D3DRS_STENCILPASS,  D3DSTENCILOP_KEEP );
            }
            else if( i == 1 )
            {
                g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,       D3DCULL_CW );
                g_pd3dDevice->SetRenderState( D3DRS_STENCILFAIL,    D3DSTENCILOP_KEEP );
                g_pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL,   D3DSTENCILOP_DECR );
                g_pd3dDevice->SetRenderState( D3DRS_STENCILPASS,    D3DSTENCILOP_KEEP );
            }
            else if( i == 2 )
            {
                g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,       D3DCULL_CW );
                g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,   TRUE );
                g_pd3dDevice->SetRenderState( D3DRS_ZFUNC,          D3DCMP_ALWAYS );
                g_pd3dDevice->SetRenderState( D3DRS_STENCILFAIL,    D3DSTENCILOP_ZERO );
                g_pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL,   D3DSTENCILOP_ZERO );
                g_pd3dDevice->SetRenderState( D3DRS_STENCILPASS,    D3DSTENCILOP_ZERO );
                g_pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,    D3DCMP_LESS );

                vector4     Pixel[4];
                const f32   Size = 4;
    
                Pixel[0].Set(    0,    0, 1, 1.0f );
                Pixel[1].Set(    0, 1024, 1, 1.0f );
                Pixel[2].Set( 1280,    0, 1, 1.0f );
                Pixel[3].Set( 1280, 1024, 1, 1.0f );

                g_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW  );
                g_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, Pixel, sizeof(vector4) );

                break;
            }

           
            g_pd3dDevice->DrawIndexedPrimitiveUP(
                D3DPT_TRIANGLELIST,
                0, 
                Hole.nVerts,
                Hole.nIndices/3,
                Hole.pIndex,
                D3DFMT_INDEX16,
                Hole.pVert,
                sizeof(vector3) );
        }

        g_pd3dDevice->SetRenderState ( D3DRS_ZWRITEENABLE ,     TRUE );
        g_pd3dDevice->SetRenderState ( D3DRS_CULLMODE,          D3DCULL_CW );
        g_pd3dDevice->SetRenderState ( D3DRS_STENCILENABLE,     FALSE );
        g_pd3dDevice->SetRenderState ( D3DRS_ZFUNC,             D3DCMP_LESSEQUAL   );
        g_pd3dDevice->SetRenderState ( D3DRS_COLORWRITEENABLE,  D3DCOLORWRITEENABLE_ALPHA |
                                                                D3DCOLORWRITEENABLE_BLUE  | 
                                                                D3DCOLORWRITEENABLE_GREEN |
                                                                D3DCOLORWRITEENABLE_RED  );
    }
    else
    {
        g_pd3dDevice->DrawIndexedPrimitiveUP(
            D3DPT_TRIANGLELIST,
            0, 
            Hole.nVerts,
            Hole.nIndices/3,
            Hole.pIndex,
            D3DFMT_INDEX16,
            Hole.pVert,
            sizeof(vector3) );
    }
}

//=========================================================================
struct sort
{
    s32         iInst;
    wall_geom*  pWallGeom;
    f32         Distance;
};

s32 Sort_Geom( sort* pItem1, sort* pItem2 )
{
    if( pItem1->Distance < pItem2->Distance ) return -1;
    return pItem1->Distance > pItem2->Distance;
}

void RenderMap( void )
{
    s32 i;
    s32 TotalPolys = 0;
    s32 TotalVerts = 0;
    f32 TotalSurfArea = 0;

    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING,		FALSE );
    g_pd3dDevice->SetRenderState(D3DRS_SPECULARENABLE,  FALSE );
    g_pd3dDevice->SetRenderState(D3DRS_AMBIENT, 0 );
    g_pd3dDevice->SetRenderState(D3DRS_COLORVERTEX, TRUE );

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE  );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE    );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_DISABLE    );

    g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,         D3DBLEND_ONE );
    g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,        D3DBLEND_ZERO );

    smem_StackPushMarker();

    vector3 CamPos;
    sort*   pSort = (sort*)smem_StackAlloc( sizeof(sort)*s_Editor.GetNInsts() );
    ASSERT( pSort != NULL );

    CamPos = View.GetPosition();

    for( i=0; i<s_Editor.GetNInsts(); i++ )
    {
        layout_editor::piece_inst& Inst  = s_Editor.GetInst( i );
        layout_editor::piece_desc& Desc  = s_Editor.GetDesc( Inst.hDesc );
        
        
        pSort[i].pWallGeom = Desc.pWallGeom;
        vector3      Pos   = pSort[i].pWallGeom->m_Volumen.m_Prim.pVol->BBox.GetCenter();
        
        Pos                = Inst.L2W * Pos;        
        pSort[i].Distance  = (CamPos-Pos).LengthSquared();
        pSort[i].iInst     = i;
    }

    x_qsort( pSort, s_Editor.GetNInsts(), sizeof(sort), (compare_fn*)Sort_Geom );

    gman_Begin( "RENDER MAP" );
    for( i=0; i<s_Editor.GetNInsts(); i++ )
    {
        layout_editor::piece_inst& Inst  = s_Editor.GetInst( pSort[i].iInst );
        layout_editor::piece_desc& Desc  = s_Editor.GetDesc( Inst.hDesc );
/*
        if( pSort[i].pWallGeom->m_Info.nHoles )
        {
            for( s32 j=0; j<pSort[i].pWallGeom->m_Info.nHoles; j++ )
            {
                PuchHole( pSort[i].pWallGeom->m_Info.pHole[j], Inst.L2W );
            }

            gman_ResetStreams();
        }
*/
        if( Inst.Flags & layout_editor::INST_FLAGS_SELECTED )
            continue;

        TotalSurfArea   += Desc.SurfaceArea;
        TotalPolys      += (pSort[i].pWallGeom->m_System.pPC->nIndices /3 );
        TotalVerts      += (pSort[i].pWallGeom->m_System.pPC->nVerts );

        //g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,     D3DCULL_NONE );
        gman_Render( Inst.L2W, Inst.hGeom );
    }
    gman_End();

    //
    // Render selected pieces
    //

    gman_Begin( "SELECTED PIECES MAP" );

	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING,		TRUE);
    g_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
    d3deng_SetAmbientLight( xcolor( 255, 0, 0, 255 ) );
    
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE    );
    g_pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE    );

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_ADD  );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE  );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR  );
    g_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, xcolor( 128,0,0,255) );

    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE     );

    for( i=0; i<s_Editor.GetNInsts(); i++ )
    {
        layout_editor::piece_inst& Inst  = s_Editor.GetInst( pSort[i].iInst );
        layout_editor::piece_desc& Desc  = s_Editor.GetDesc( Inst.hDesc );


        if( !(Inst.Flags & layout_editor::INST_FLAGS_SELECTED) )
            continue;

        TotalSurfArea   += Desc.SurfaceArea;
        TotalPolys      += (pSort[i].pWallGeom->m_System.pPC->nIndices /3 );
        TotalVerts      += (pSort[i].pWallGeom->m_System.pPC->nVerts );

        gman_Render( Inst.L2W, Inst.hGeom );
    }
    gman_End();
    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING,		FALSE);


    smem_StackPopToMarker();

    x_printfxy( 0,2, "Polys      : %d", TotalPolys );
    x_printfxy( 0,3, "Verts      : %d", TotalVerts );
    x_printfxy( 0,4, "SurfArea   : %f", TotalSurfArea );
    x_printfxy( 0,5, "nInstances : %d", s_Editor.GetNInsts() );

    //
    // Render bbox from the collision
    //
    if( 0 )
    for( i=0; i<s_Editor.GetNInsts(); i++ )
    {
        layout_editor::piece_inst& Inst  = s_Editor.GetInst( i );
        layout_editor::piece_desc& Desc  = s_Editor.GetDesc( Inst.hDesc );
        
        bbox BBox = Desc.pWallGeom->m_Volumen.m_Prim.pVol->BBox;        
        BBox.Transform( Inst.L2W );
        eng_Begin();
        draw_BBox( BBox );
        eng_End();
    }


}

//=========================================================================

void Render( void )
{
    d3deng_UpdateDisplayWindow( d3deng_GetWindowHandle() );
    eng_MaximizeViewport( View );
    eng_SetView         ( View, 0 );
    eng_ActivateView    ( 0 );

    // make sure d3d has the latest matrices
    g_pd3dDevice->SetTransform( D3DTS_VIEW,       (D3DMATRIX*)&View.GetW2V() );
    g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&View.GetV2C() );

    // Render a marker
    eng_Begin( "Render Marker");
    draw_Marker( vector3( 0,0,0 ), xcolor(255,0,0,255) );
    eng_End();

    // Render Lights
    RenderLights();

    // Render Glows
    RenderGlow();

    // Render Sound
    RenderSounds();
    
    // Render Pfx.
    RenderPfx();

    // Render Pickup.
    RenderPickup();

    // Render the map        
    eng_SetView         ( View, 0 );
    eng_ActivateView    ( 0 );
    RenderMap();


    eng_Begin( "Grid" );
    {
        s_Grid.SetTranslations( s_GridPos );
        s_Grid.Render();
    }
    eng_End();

    eng_Begin( "Axis" );
    {
        if( s_EditMode == EDIT_ADD_INSTANCE || s_EditMode == EDIT_ADD_LIGHT || s_EditMode == EDIT_ADD_SOUND )
            s_FocusPos.Y = s_GridPos.Y;

        s_Axis.SetPosition( s_FocusPos );
        s_Axis.Render();
    }
    eng_End();


    // Render the selected light
    if( s_EditMode == EDIT_ADD_LIGHT ) 
        RenderAddLight();

    // Render the selected sound.
    if( s_EditMode == EDIT_ADD_SOUND )
        RenderAddSound();

    // Render the selected piece
    if( s_EditMode == EDIT_ADD_INSTANCE ) 
        RenderAddMode();

    // Render the selected piece
    if( s_EditMode == EDIT_ADD_WITH_RAY ) 
        RenderAddMode();

    // Render the selected piece
    if( s_EditMode == EDIT_ADD_ROTATE ) 
        RenderAddMode();

    // Render the selected particle effect.
    if( s_EditMode == EDIT_ADD_PFX )
        RenderAddPfx();

    // Render the selected pickup item.
    if( s_EditMode == EDIT_ADD_PICKUP || s_EditMode == EDIT_ADD_PICKUP_GRID)
        RenderAddPickup();

    // Render the selected light
    if( s_EditMode == EDIT_ADD_GLOW ) 
        RenderAddGlow();
}

//==============================================================================
static
void AddNewPiece( void )
{
    e_begin;

    HWND Item       = GetDlgItem( s_hDialog, IDC_PIECES_LIST );
    s32  Index      = SendMessage( Item, LB_GETCURSEL, 0, 0 );

    if( Item == 0 )
        return;

    if( Index ==  LB_ERR ) 
        e_throw( "You must select a piece first" );

    // Get the name of the piece
    char Name[256];
    matrix4 L2W;
    SendMessage( Item, LB_GETTEXT, (u32)Index, (u32)Name );
    ComputeFocusMatrix( L2W );

    s32 Index2 = SendMessage( GetDlgItem( s_hDialog, IDC_PICKUP_TYPE ), CB_GETCURSEL, 0, 0 );
    s_ObjectType = SendMessage( GetDlgItem( s_hDialog, IDC_PICKUP_TYPE ), CB_GETITEMDATA, Index2, 0 );
    SendMessage( GetDlgItem( s_hDialog, IDC_OBJECT_NAME ), WM_GETTEXT, (WPARAM)64, (LPARAM)s_ObjectName );
    SetWindowText( GetDlgItem( s_hDialog, IDC_OBJECT_NAME ), "");


    // Now Lets add it
    s_Editor.AddInst( Name, L2W, s_ObjectType, s_ObjectName );

    e_handle;
}

//==============================================================================
static
void AddNewLight( void )
{
    e_begin;

    // Compute the fovus pos
    matrix4 L2W;
    ComputeFocusMatrix( L2W );

    // Now Lets add it
    s_Editor.AddLight( sphere( L2W.GetTranslation(), s_LightR), s_LightC, s_LightI );

    s_Editor.UpdateLighting();

    e_handle;
}

//==============================================================================
static
void AddNewGlow( void )
{
    e_begin;

    // Compute the fovus pos
    matrix4 L2W;
    ComputeFocusMatrix( L2W );

    // Now Lets add it
    s_Editor.AddGlow( sphere( L2W.GetTranslation(), s_GlowR), s_GlowC, s_GlowI );

    s_Editor.UpdateLighting();

    e_handle;
}

//==============================================================================
static void AddNewSound( void )
{
    e_begin;

    // Compute the fovus pos
    matrix4 L2W;
    ComputeFocusMatrix( L2W );

    // Now Lets add it
    s_Editor.AddSound( sphere( L2W.GetTranslation(), s_SoundR), s_SoundT);

    e_handle;
 
}

//==============================================================================
static void AddNewPfx( void )
{
    e_begin;

    // Compute the fovus pos
    matrix4 L2W;
    ComputeFocusMatrix( L2W );

    s32 Type = SendMessage( GetDlgItem( s_hDialog, IDC_PFXTYPE ), CB_GETCURSEL, 0, 0 );
    
    // Now Lets add it
    s_Editor.AddEffect( L2W.GetTranslation(), s_NormalPos, Type );

    e_handle;
 
}

//==============================================================================
static void AddNewPickup( void )
{
    e_begin;

    // Compute the fovus pos
    matrix4 L2W;
    ComputeFocusMatrix( L2W );

    s32 Type = SendMessage( GetDlgItem( s_hDialog, IDC_PICKUP_TYPE ), CB_GETCURSEL, 0, 0 );
    
    // Now Lets add it
    s_Editor.AddPickup( sphere( L2W.GetTranslation(), PICKUP_SPHERE_RADIUS), Type );

    e_handle;
 
}

//==============================================================================
static 
void RotatePiece( s32 I )
{
    s_FocusOrient += I;            // 0 = 0, 1 = 90, 2 = 180, 3 = 270

    if( s_FocusOrient > 3 ) 
        s_FocusOrient = 0;

    if( s_FocusOrient < 0 ) 
        s_FocusOrient = 3;

    SetDlgItemText( s_hDialog, IDC_ROTATE_INFO, xfs("%d", s_FocusOrient*90) );
}

//==============================================================================

xbool SaveProject( char* pFileName, char* pInitialDir, char* pExtDesc, char* pExt, char* pTitle, HWND hWnd )
{
    OPENFILENAME OpenStr;

    x_memset( &OpenStr, 0, sizeof(OpenStr )); 
    OpenStr.lStructSize         = sizeof(OpenStr );
    OpenStr.hwndOwner           = hWnd;
    OpenStr.hInstance           = d3deng_GetInstace();
    OpenStr.lpstrFilter         = pExtDesc;
    OpenStr.lpstrFile           = pFileName;
    OpenStr.nMaxFile            = 256;
    OpenStr.lpstrInitialDir     = pInitialDir;
    OpenStr.lpstrTitle          = pTitle;
    OpenStr.Flags               = OFN_ENABLESIZING | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;

    if( GetSaveFileName( &OpenStr ) )
    {
        char Drive[256];
        char Dir[256];
        char FileName[256];

        x_splitpath( pFileName,   Drive, Dir, FileName, NULL ); 
        x_makepath ( pInitialDir, Drive, Dir, NULL, NULL );
        x_makepath ( pFileName,   Drive, Dir, FileName, pExt );

        return TRUE;
    }

    return FALSE;
}

//==============================================================================

xbool LoadProject( char* pFileName, char* pInitialDir, char* pExtDesc, char* pExt, char* pTitle, HWND hWnd )
{
    OPENFILENAME OpenStr;

    x_memset( &OpenStr, 0, sizeof(OpenStr )); 
    OpenStr.lStructSize         = sizeof(OpenStr );
    OpenStr.hwndOwner           = hWnd;
    OpenStr.hInstance           = d3deng_GetInstace();
    OpenStr.lpstrFilter         = pExtDesc;
    OpenStr.lpstrFile           = pFileName;
    OpenStr.nMaxFile            = 256;
    OpenStr.lpstrFileTitle      = NULL;
    OpenStr.nMaxFileTitle       = 0;
    OpenStr.lpstrInitialDir     = pInitialDir;
    OpenStr.lpstrTitle          = pTitle;
    OpenStr.Flags               = OFN_ENABLESIZING | OFN_PATHMUSTEXIST;

    if( GetOpenFileName( &OpenStr ) )
    {
        char Drive[256];
        char Dir[256];
        char FileName[256];

        x_splitpath( pFileName,   Drive, Dir, FileName, NULL ); 
        x_makepath ( pInitialDir, Drive, Dir, NULL, NULL );
        x_makepath ( pFileName,   Drive, Dir, FileName, pExt );

        return TRUE;
    }

    return FALSE;
}


//==============================================================================

LRESULT CALLBACK MainWindowProc(
  HWND hwnd,      // handle to window
  UINT uMsg,      // message identifier
  WPARAM wParam,  // first message parameter
  LPARAM lParam   // second message parameter
)
{
    static s16 OldPosX;
    static s16 OldPosY;
    e_begin;

    switch (uMsg) 
    { 
            // Initialize the window. 
        case WM_ERASEBKGND: 
            return 0; 

        case WM_CREATE: 
            // Initialize the window.
            return 0; 
 
        case WM_SIZE: 
            {
                if( s_hDialog==0 || d3deng_GetWindowHandle()==0) 
                    break;

                // Set the size and position of the window. 
                RECT Window;
                GetClientRect   ( hwnd, &Window );
                s32 DialogW  = 150;
                MoveWindow( s_hDialog, Window.right - DialogW, 0, DialogW, Window.bottom - Window.top, TRUE );
                s32 D3DW = Window.right - Window.left - DialogW; 
                MoveWindow( d3deng_GetWindowHandle(), 0, 0, D3DW, Window.bottom - Window.top, TRUE );
                return 0; 
            }
 
        case WM_ACTIVATE:
            {
                if( (wParam & WA_ACTIVE) && d3deng_GetWindowHandle() )
                    SetActiveWindow(d3deng_GetWindowHandle() );

                return 0;
            }

        case WM_COMMAND:
            {
                switch( LOWORD(wParam) )
                {
                case AFX_ID_NEW_PROJECT:
                    {
                        if( s_Editor.IsModified() )
                        {
                            int Answer = MessageBox( hwnd, 
                                        "Are you sure you want to lose your changes?", 
                                        "WARNING", 
                                        MB_OKCANCEL | MB_ICONWARNING | MB_TASKMODAL );

                            if( Answer == IDCANCEL )
                                return 0;
                        }

                        // Okay lets start a new project
                        s_Editor.NewProject();

                        // Clear the name of the project
                        SetWindowText( s_hMainWindow, "Unnamed" );

                        // Set the project name
                        s_ProjectName[0] = 0;

                        return 0;
                    }
                case AFX_ID_SAVE:

                    if( s_ProjectName[0] )
                    {
                        s_Editor.Save( s_ProjectName );
                        return 0;
                    }

                case AFX_ID_SAVEAS:                    
                    if( SaveProject( s_ProjectName, 
                                     s_InitialDirPrj, 
                                     "ProjectFile (*.Proj)\0*.proj\0All (*.*)\0*.*\0", 
                                     ".proj",
                                     "Save Project", hwnd ) )
                    {
                        s_Editor.Save( s_ProjectName );
                        SetWindowText( s_hMainWindow, s_ProjectName );
                    }
                    return 0;

                case AFX_ID_LOAD:

                    if( LoadProject( s_ProjectName, 
                                     s_InitialDirPrj, 
                                     "ProjectFile (*.Proj)\0*.proj\0All (*.*)\0*.*\0", 
                                     ".proj",
                                     "Load Project", hwnd ) ) 
                    {
                        s_Editor.Load( s_ProjectName );
                    }
                    return 0;

                case AFX_ID_EXPORT:
                    {
                        if( SaveProject( s_ExportName, 
                                         s_InitialDirExp, 
                                         "LevelFile (*.level)\0*.level\0All (*.*)\0*.*\0", 
                                         ".level",
                                         "Export Level", hwnd ) ) 
                        {
                            s_Editor.Export( s_ExportName );
                        }
                    }
                    return 0;
                }

                return 0;
            }

        case WM_CLOSE:
            if( s_Editor.IsModified() )
            {
                int Answer = MessageBox( hwnd, 
                            "Are you sure you want to exit and lose your changes?", 
                            "WARNING", 
                            MB_OKCANCEL | MB_ICONWARNING | MB_TASKMODAL );

                if( Answer == IDCANCEL )
                    return 0;
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0L;

        case WM_KEYDOWN:
            switch( wParam )
            {
            case VK_PRIOR:
                {
                    HWND Item = GetDlgItem( s_hDialog, IDC_GRID_YPOS );
                    s_GridPos.Y += 100; 
                    SetWindowText( Item, xfs("%3.2f", s_GridPos.Y/100.0f) );
                    break;
                }
            case VK_NEXT: 
                {
                    HWND Item = GetDlgItem( s_hDialog, IDC_GRID_YPOS );
                    s_GridPos.Y -= 100; 
                    SetWindowText( Item, xfs("%3.2f", s_GridPos.Y/100.0f) );
                    break;
                }

            case VK_DELETE:                
                {
                RotatePiece(1);
                break;
                }

            case VK_END:                
                {
                RotatePiece(1);
                break;
                }

            case VK_UP:
                {
                   vector3 V = View.GetViewZ();
                   V.Y = 0;
                   V.Normalize();
                   s_FocusPos += V * s_SnapStep;         
                   s_FocusPos.Snap( s_SnapStep,s_SnapStep,s_SnapStep );
                   break;
                }

            case VK_DOWN:
                {
                   vector3 V = View.GetViewZ();
                   V.Y = 0;
                   V.Normalize();
                   s_FocusPos -= V * s_SnapStep;
                   s_FocusPos.Snap( s_SnapStep,s_SnapStep,s_SnapStep );
                   break;
                }

            case VK_LEFT:
                {
                   vector3 V = View.GetViewX();
                   V.Y = 0;
                   V.Normalize();
                   s_FocusPos += V * s_SnapStep;
                   s_FocusPos.Snap( s_SnapStep,s_SnapStep,s_SnapStep );
                   break;
                }

            case VK_RIGHT:
                {
                   vector3 V = View.GetViewX();
                   V.Y = 0;
                   V.Normalize();
                   s_FocusPos -= V * s_SnapStep;
                   s_FocusPos.Snap( s_SnapStep,s_SnapStep,s_SnapStep );
                   break;
                }

            case VK_SPACE:
                {
                    if( s_EditMode == EDIT_ADD_LIGHT )
                        AddNewLight();
                    
                    if( s_EditMode == EDIT_ADD_SOUND )
                        AddNewSound();

                    if( s_EditMode == EDIT_ADD_INSTANCE )
                        AddNewPiece();

                    if( s_EditMode == EDIT_ADD_WITH_RAY )
                        AddNewPiece();

                    if( s_EditMode == EDIT_ADD_ROTATE )
                        AddNewPiece();

                    if( s_EditMode == EDIT_ADD_PFX )
                        AddNewPfx();

                    if( s_EditMode == EDIT_ADD_PICKUP || s_EditMode == EDIT_ADD_PICKUP_GRID )
                        AddNewPickup();

                    if( s_EditMode == EDIT_ADD_GLOW )
                        AddNewGlow();

                    break;
                }
            }
            return 0;

        case WM_MOUSEWHEEL:
            eng_D3DWndProc( hwnd, uMsg, wParam, lParam );
            return 0;

        case WM_LBUTTONDOWN:
            {
                s16     xPos = (lParam>>0)&0xffff; 
                s16     yPos = (lParam>>16)&0xffff; 
                vector3 Ray  = View.RayFromScreen( xPos, yPos );
                plane   Plane( vector3( 0,1,0 ), -s_GridPos.Y );
                vector3 P0   = View.GetPosition();
                vector3 P1   = P0 + Ray*10000.0f;
                f32     t;

                if( s_EditMode == EDIT_ADD_INSTANCE || s_EditMode == EDIT_ADD_LIGHT || s_EditMode == EDIT_ADD_SOUND 
                    || s_EditMode == EDIT_ADD_PICKUP_GRID )
                {
                    if( Plane.Intersect( t, P0, P1 ) == TRUE )
                    {
                        s_FocusPos = P0 + t*( P1 - P0 );
                        s_FocusPos.Snap( s_SnapStep, s_SnapStep, s_SnapStep );
                    }
                }

                if( s_EditMode == EDIT_SEL_INSTANCE )
                {
                    s_Editor.Select( P0, P1, TRUE );
                }

                if( s_EditMode == EDIT_UNSEL_INSTANCE )
                {
                    s_Editor.Select( P0, P1, FALSE );
                }

                if( s_EditMode == EDIT_ADD_WITH_RAY || s_EditMode == EDIT_ADD_PFX || s_EditMode == EDIT_ADD_GLOW 
                    || s_EditMode == EDIT_ADD_PICKUP )
                {
                    t = s_Editor.GetHitPos( P0, P1, s_NormalPos );
                    if( t != -1)
                        s_FocusPos = P0 + t*( P1 - P0 );
                }                    

                OldPosX = xPos;
                OldPosY = yPos;
                return 0;
            }

        case WM_MOUSEMOVE:
            {
                if( (s_EditMode == EDIT_ADD_INSTANCE || s_EditMode == EDIT_ADD_LIGHT || s_EditMode == EDIT_ADD_SOUND 
                    || s_EditMode == EDIT_ADD_PICKUP_GRID) && (wParam&MK_LBUTTON) )
                {
                    plane   Plane( vector3( 0,1,0 ), -s_GridPos.Y );
                    s16     xPos = (lParam>>0)&0xffff; 
                    s16     yPos = (lParam>>16)&0xffff; 
                    vector3 Ray  = View.RayFromScreen( xPos, yPos );
                    vector3 P0   = View.GetPosition();
                    vector3 P1   = P0 + Ray*10000.0f;
                    f32     t;

                    if( Plane.Intersect( t, P0, P1 ) == TRUE )
                    {
                        s_FocusPos = P0 + t*( P1 - P0 );
                        s_FocusPos.Snap( s_SnapStep, s_SnapStep, s_SnapStep );
                    }
                }                    

                if( ( s_EditMode == EDIT_ADD_WITH_RAY || s_EditMode == EDIT_ADD_PFX || s_EditMode == EDIT_ADD_GLOW 
                    || s_EditMode == EDIT_ADD_PICKUP ) && (wParam&MK_LBUTTON) )
                {
                    s16     xPos = (lParam>>0)&0xffff; 
                    s16     yPos = (lParam>>16)&0xffff; 
                    vector3 Ray  = View.RayFromScreen( xPos, yPos );
                    vector3 P0   = View.GetPosition();
                    vector3 P1   = P0 + Ray*10000.0f;
                    f32     t;

                    t = s_Editor.GetHitPos( P0, P1 , s_NormalPos);
                    if( t != -1)
                        s_FocusPos = P0 + t*( P1 - P0 );
                }                    

                if( s_EditMode == EDIT_ADD_ROTATE && (wParam&MK_LBUTTON) )
                {
                    s16     xPos = (lParam>>0)&0xffff; 
                    s16     yPos = (lParam>>16)&0xffff; 
                    s16 DeltaX = xPos - OldPosX;
                    s16 DeltaY = yPos - OldPosY;

                    s_FocusFreeOrient.RotateY( DeltaX * 0.05f );
                    radian3 Rot = s_FocusFreeOrient.GetRotation();

                    
                    SetDlgItemText( s_hDialog, IDC_ROTATE_INFO, xfs("%4.3f", RAD_TO_DEG( Rot.Yaw )) );


                    OldPosX = xPos;
                    OldPosY = yPos;
                }

                return 0;
            }
        // 
        // Process other messages. 
        // 
     } 

e_handle;

    return DefWindowProc(hwnd, uMsg, wParam, lParam); 
}

//==============================================================================
static
LRESULT CALLBACK D3DWindowProc(
  HWND hwnd,      // handle to window
  UINT uMsg,      // message identifier
  WPARAM wParam,  // first message parameter
  LPARAM lParam   // second message parameter
)
{
    switch (uMsg) 
    { 
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_KEYDOWN:
        case WM_MOUSEWHEEL:
            MainWindowProc( hwnd, uMsg, wParam, lParam );
            return 0;

    }

    return eng_D3DWndProc( hwnd, uMsg, wParam, lParam );
}

//==============================================================================

int CALLBACK MatDialogProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
    e_begin;

    switch (uMsg) 
    {
        case WM_INITDIALOG:
        {

        }
        break;
        case WM_COMMAND:
        {
            switch( LOWORD( wParam ) )
            {
                case IDC_SOUND_TYPE_LIST:
                {
                
                }
                break;
                case IDC_SOUND_TYPE_MAT:
                {
                    
                }
                break;
                case ID_SOUND_TYPE_APPLY:
                {
                    HWND Item = GetDlgItem( s_hMatDialog, IDC_SOUND_TYPE_LIST );
                    s32  Index      = SendMessage( Item, LB_GETCURSEL, 0, 0 );

                    if( Index ==  LB_ERR ) 
                        e_throw( "You must select a piece first" );

                    layout_editor::piece_desc& Piece = s_Editor.GetDesc( Index );
                    Piece.MatType = (layout_editor::material_type)SendMessage( GetDlgItem( hwndDlg, IDC_SOUND_TYPE_MAT ), CB_GETCURSEL, 0, 0 );

                    
                    char Buff[256] = {0};
                    char PieceType[16] = {0};
                    switch( Piece.MatType )
                    {
                        case layout_editor::MAT_TYPE_NULL:{  x_strcpy( PieceType,"NULL"); }break;
                        case layout_editor::MAT_TYPE_WOOD:{  x_strcpy( PieceType,"WOOD"); }break;
                        case layout_editor::MAT_TYPE_METAL:{ x_strcpy( PieceType,"METAL"); }break;
                        case layout_editor::MAT_TYPE_FLESH:{ x_strcpy( PieceType,"FLESH"); }break;
                        case layout_editor::MAT_TYPE_STONE:{ x_strcpy( PieceType,"STONE"); }break;
                    
                    };
                    //x_strcpy( Buff, Piece.FileName, sizeof( Piece.FileName ) );
            
                    x_sprintf( Buff,"%s ----- %s", Piece.FileName, PieceType );
                    SendMessage( Item, LB_DELETESTRING, Index, 0 );
                    SendMessage( Item, LB_INSERTSTRING, Index, (u32)Buff );

                                
                }
                break;
                case ID_SOUND_TYPE_DONE:
                {
                    EndDialog( s_hMatDialog, NULL );
                }
                break;
            }
            return 0;
        }
        break;
        case WM_PAINT:

        break;
        case WM_CLOSE:
        {
            EndDialog( s_hMatDialog, NULL );
        }
        break;

    }

    e_handle;

    return 0;
}

//==============================================================================

int CALLBACK PickupDialogProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
    e_begin;

    switch (uMsg) 
    {
        case WM_INITDIALOG:
        {

        }
        break;
        case WM_COMMAND:
        {
            switch( LOWORD( wParam ) )
            {
            /*
                case IDC_ADD_PICKUP:
                {
                    s_bFocusFreeOrient = TRUE;
                    s_EditMode = EDIT_ADD_PICKUP; 
                    SetDlgItemText( hwndDlg, IDC_EDIT_MODE, "Add Pickup" );
                    SetFocus( s_hMainWindow );                
                }
                break;
                case IDC_ADD_PICKUP_GRID:
                {
                    s_bFocusFreeOrient = FALSE;
                    s_EditMode = EDIT_ADD_PICKUP_GRID; 
                    SetDlgItemText( hwndDlg, IDC_EDIT_MODE, "Add Pickup" );
                    SetFocus( s_hMainWindow );                
                }
                */
                break;
                case IDC_OBJECT_NAME:
                {
                    s_bFocusFreeOrient = TRUE;
                    s_EditMode = EDIT_ADD_PICKUP; 
                    SetDlgItemText( hwndDlg, IDC_EDIT_MODE, "Add Pickup" );
                    SetFocus( s_hMainWindow );                
                }
                break;

            }
            return 0;
        }
        break;
        case WM_PAINT:

        break;
        case WM_CLOSE:
        {
            EndDialog( s_hPickupDialog, NULL );
            s_hPickupDialog = NULL;
        }
        break;

    }

    e_handle;

    return 0;
}


//==============================================================================


int CALLBACK MainMenuDialogProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
    e_begin;
    static HWND hTrackWnd1;
    static HWND hTrackWnd2;

    switch (uMsg) 
    { 

        case WM_INITDIALOG :
        {
            //SendMessage
            hTrackWnd1 = CreateWindow( TRACKBAR_CLASS, "Track Bar", 
                        WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS, 70, 640, 60, 20, hwndDlg, NULL, d3deng_GetInstace(), NULL );
            SendMessage(hTrackWnd1, TBM_SETRANGE, 1, (LPARAM)MAKELONG( 50, 2000 ) );
            SendMessage(hTrackWnd1, TBM_SETPOS, 1, 1000 );

            hTrackWnd2 = CreateWindow( TRACKBAR_CLASS, "Track Bar", 
                        WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS, 70, 680, 60, 20, hwndDlg, NULL, d3deng_GetInstace(), NULL );
            SendMessage(hTrackWnd2, TBM_SETRANGE, 1, (LPARAM)MAKELONG( 50,1000 ) );
            SendMessage(hTrackWnd2, TBM_SETPOS, 100, 5000 );

        }
        break;
        case WM_HSCROLL:
        {            
            
            
            if( hTrackWnd1 == (HWND)lParam )
                s_LightR = (f32)SendMessage( hTrackWnd1, TBM_GETPOS, 0, 0 );
            
            if( hTrackWnd2 == (HWND)lParam )
                s_SoundR = (f32)SendMessage( hTrackWnd2, TBM_GETPOS, 0, 0 );

            HWND Glow = GetDlgItem( hwndDlg, IDC_GLOW_SLIDER );
            if( Glow == (HWND)lParam )
                s_GlowR = (f32)SendMessage( Glow, TBM_GETPOS, 0, 0 );
        }
        break;

        case WM_COMMAND:
            {
                switch( LOWORD( wParam ) )
                {
                case IDC_LIGHT_COLOR:                    
                    {
                        static COLORREF acrCustClr[16]; 
                        CHOOSECOLOR CC;

                        x_memset( &CC, 0, sizeof(CC) );
                        CC.lStructSize      = sizeof(CHOOSECOLOR);
                        CC.hwndOwner        = s_hMainWindow;
                        CC.rgbResult        = s_LightC;
                        CC.lpCustColors     = (LPDWORD) acrCustClr;
                        CC.Flags            = CC_ANYCOLOR | CC_RGBINIT | CC_FULLOPEN;

                        ChooseColor( &CC );
                        CommDlgExtendedError();

                        s_LightC.Set( GetRValue(CC.rgbResult),
                                      GetGValue(CC.rgbResult),
                                      GetBValue(CC.rgbResult),
                                      255 );
                        MainMenuDialogProc( hwndDlg, WM_PAINT, wParam, lParam );
                        SetFocus( s_hMainWindow );
                        break;
                    }
                case IDC_ADD_LIGHT:
                    s_bFocusFreeOrient = TRUE;
                    s_EditMode = EDIT_ADD_LIGHT; 
                    SetDlgItemText( hwndDlg, IDC_EDIT_MODE, "Add Light" );
                    SetFocus( s_hMainWindow );
                    break;

                case IDC_ADD_ROTATE:
                    s_bFocusFreeOrient = TRUE;
                    s_EditMode = EDIT_ADD_ROTATE; 
                    SetDlgItemText( hwndDlg, IDC_EDIT_MODE, "Add Rotate" );
                    SetFocus( s_hMainWindow );
                    break;

                case IDC_ADD_WITH_RAY:
                    s_bFocusFreeOrient = TRUE;
                    s_EditMode = EDIT_ADD_WITH_RAY; 
                    SetDlgItemText( hwndDlg, IDC_EDIT_MODE, "Add With Ray" );
                    SetFocus( s_hMainWindow );
                    break;

                case IDC_ADD_MODE: 
                    s_bFocusFreeOrient = FALSE;
                    s_EditMode = EDIT_ADD_INSTANCE; 
                    SetDlgItemText( hwndDlg, IDC_EDIT_MODE, "Add Wall" );
                    SetFocus( s_hMainWindow );
                    break;

                case IDC_DEL_MODE:
                    s_Editor.DeleteSelection();
                    SetFocus( s_hMainWindow );
                    break;

                case IDC_SEL_MODE:
                    s_EditMode = EDIT_SEL_INSTANCE; 
                    SetDlgItemText( hwndDlg, IDC_EDIT_MODE, "Select" );
                    SetFocus( s_hMainWindow );
                    break;

                case IDC_UNSEL_MODE:
                    s_EditMode = EDIT_UNSEL_INSTANCE; 
                    SetDlgItemText( hwndDlg, IDC_EDIT_MODE, "UnSelect" );
                    SetFocus( s_hMainWindow );
                    break;

                case IDC_UNSEL_ALL:
                    s_Editor.UnselectAll();
                    SetFocus( s_hMainWindow );
                    break;

                case IDC_UNDO: 
                    s_Editor.Undo();
                    SetFocus( s_hMainWindow );
                    break;

                case IDC_REDO: 
                    s_Editor.Redo();
                    SetFocus( s_hMainWindow );
                    break;

                case IDC_ROTATE_P90: 
                    RotatePiece(1);
                    SetFocus( s_hMainWindow );
                    break;

                case IDC_ROTATE_N90: 
                    RotatePiece(-1);
                    SetFocus( s_hMainWindow );
                    break;
                case IDC_ADD_SOUND:
                {
                    s_bFocusFreeOrient = TRUE;
                    s_EditMode = EDIT_ADD_SOUND; 
                    SetDlgItemText( hwndDlg, IDC_EDIT_MODE, "Add Sound" );                    
                    SetFocus( s_hMainWindow );
                }
                break;
                case IDC_ADD_PFX:
                {
                    s_bFocusFreeOrient = TRUE;
                    s_EditMode = EDIT_ADD_PFX; 
                    SetDlgItemText( hwndDlg, IDC_EDIT_MODE, "Add PFX" );                    
                    SetFocus( s_hMainWindow );
                }
                break;
                case IDC_SOUNDTYPE:
                {
//                    MainMenuDialogProc( hwndDlg, WM_PAINT, wParam, lParam );
//                    s_SoundT = SendMessage( s_hMainWindow, LB_GETCURSEL, 0, 0 );
                
                    s_SoundT = SendMessage( GetDlgItem( hwndDlg, IDC_SOUNDTYPE ), CB_GETCURSEL, 0, 0 );
                   
                }
                break;
                case IDC_MATERIAL_TYPE:
                {
                    s_hMatDialog = CreateDialog(   d3deng_GetInstace(), 
                                                MAKEINTRESOURCE(IDD_MATERIAL_TYPE ),
                                                s_hMainWindow,
                                                MatDialogProc );
                    {
                        HWND Item = GetDlgItem( s_hMatDialog, IDC_SOUND_TYPE_LIST );
                        DWORD err = GetLastError();
                        

                        for( s32 i=0; i<s_Editor.GetNDescs(); i++ )
                        {
                            layout_editor::piece_desc& Piece = s_Editor.GetDesc( i );
                            char Buff[256] = {0};
                            char PieceType[16] = {0};
                            switch( Piece.MatType )
                            {
                                case layout_editor::MAT_TYPE_NULL:{  x_strcpy( PieceType,"NULL"); }break;
                                case layout_editor::MAT_TYPE_WOOD:{  x_strcpy( PieceType,"WOOD"); }break;
                                case layout_editor::MAT_TYPE_METAL:{ x_strcpy( PieceType,"METAL"); }break;
                                case layout_editor::MAT_TYPE_FLESH:{ x_strcpy( PieceType,"FLESH"); }break;
                                case layout_editor::MAT_TYPE_STONE:{ x_strcpy( PieceType,"STONE"); }break;
                        
                            };
                            //x_strcpy( Buff, Piece.FileName, sizeof( Piece.FileName ) );
                
                            x_sprintf( Buff,"%s ----- %s", Piece.FileName, PieceType );
                            SendMessage( Item, LB_ADDSTRING, 0, (u32)Buff );
                        }

                        SendMessage( Item, LB_SETCURSEL, 0, 0 );
                        layout_editor::piece_desc& Piece = s_Editor.GetDesc( 0 );
                        HWND ComboBox = GetDlgItem( s_hMatDialog, IDC_SOUND_TYPE_MAT );
            
                        SendMessage( ComboBox, CB_ADDSTRING, 0, (LPARAM)"MAT_TYPE_NULL" );
                        SendMessage( ComboBox, CB_ADDSTRING, 0, (LPARAM)"MAT_TYPE_WOOD" );
                        SendMessage( ComboBox, CB_ADDSTRING, 0, (LPARAM)"MAT_TYPE_METAL" );
                        SendMessage( ComboBox, CB_ADDSTRING, 0, (LPARAM)"MAT_TYPE_FLESH" );
                        SendMessage( ComboBox, CB_ADDSTRING, 0, (LPARAM)"MAT_TYPE_STONE" );

                        SendMessage( ComboBox, CB_SETCURSEL, 0, Piece.MatType );
            
                    }

                    ShowWindow( s_hMatDialog, SW_SHOWDEFAULT );
                    UpdateWindow( s_hMatDialog );
                }
                break;
/*                case IDC_PICKUP:
                {
                    if( !s_hPickupDialog )
                    {                                            
                        s_hPickupDialog = CreateDialog(   d3deng_GetInstace(), 
                                                    MAKEINTRESOURCE(IDD_PICKUP ),
                                                    s_hMainWindow,
                                                    PickupDialogProc );                
                        UpdatePickupList();
                        ShowWindow( s_hPickupDialog, SW_SHOWDEFAULT );
                        UpdateWindow( s_hPickupDialog );
                    }
                }
                break;
*/
                case IDC_PICKUP_TYPE:
                {
                    s32 Index = SendMessage( GetDlgItem( s_hMainWindow, IDC_PICKUP_TYPE ), CB_GETCURSEL, 0, 0 );
                    s_ObjectType = SendMessage( GetDlgItem( s_hMainWindow, IDC_PICKUP_TYPE ), CB_GETITEMDATA, Index, 0 );
                    
                }
                break;
                case IDC_OBJECT_NAME:
                {
                   SendMessage( GetDlgItem( s_hMainWindow, IDC_OBJECT_NAME ), EM_GETLINE, (WPARAM)0, (LPARAM)s_ObjectName );
                    
                }
                break;
                case IDC_LOOP_SOUND:
                {
                                            
                    HWND Loop = GetDlgItem( s_hDialog, IDC_LOOP_SOUND );
                    layout_editor::sound_desc& SoundDesc =  s_Editor.GetSoundDesc( s_SoundT );
                    
                    s32 Message = SendMessage( Loop, BM_GETSTATE, 0, 0 );
                    if(  Message & BST_CHECKED )
                        SoundDesc.Looped = TRUE;
                    else
                        SoundDesc.Looped = FALSE;
                    
                    SetFocus( s_hMainWindow );

                }
                break;
                case IDC_CLIP_SOUND:
                {
                                            
                    HWND Loop = GetDlgItem( s_hDialog, IDC_CLIP_SOUND );
                    layout_editor::sound_desc& SoundDesc =  s_Editor.GetSoundDesc( s_SoundT );
                    
                    s32 Message = SendMessage( Loop, BM_GETSTATE, 0, 0 );
                    if(  Message & BST_CHECKED )
                        SoundDesc.Clipped = TRUE;
                    else
                        SoundDesc.Clipped = FALSE;
                    
                    SetFocus( s_hMainWindow );

                }
                break;
                case IDC_GLOW_COLOR:                    
                {
                    static COLORREF acrCustClr[16]; 
                    CHOOSECOLOR CC;

                    x_memset( &CC, 0, sizeof(CC) );
                    CC.lStructSize      = sizeof(CHOOSECOLOR);
                    CC.hwndOwner        = s_hMainWindow;
                    CC.rgbResult        = s_GlowC;
                    CC.lpCustColors     = (LPDWORD) acrCustClr;
                    CC.Flags            = CC_ANYCOLOR | CC_RGBINIT | CC_FULLOPEN;

                    ChooseColor( &CC );
                    CommDlgExtendedError();

                    s_GlowC.Set( GetRValue(CC.rgbResult),
                                  GetGValue(CC.rgbResult),
                                 GetBValue(CC.rgbResult),
                                  255 );

                    MainMenuDialogProc( hwndDlg, WM_PAINT, wParam, lParam );
                    SetFocus( s_hMainWindow );
                    break;
                }
                break;
                case IDC_ADD_GLOW:
                {
                    s_bFocusFreeOrient = TRUE;
                    s_EditMode = EDIT_ADD_GLOW; 
                    SetDlgItemText( hwndDlg, IDC_EDIT_MODE, "Add Glow" );
                    SetFocus( s_hMainWindow );                
                }
                break;
                        /*
                case IDC_PIECES_LIST:
                    {
                        switch( HIWORD(wParam) )
                        {
                        case LBN_SETFOCUS:
                            {
                                SetFocus( s_hMainWindow );
                                break;
                            };
                        }
                        int a;
                        a = 0;
                        break;
                    }
                    */
                }
                return 0;
            }

        case WM_PAINT:
            {
                RECT r;
                HDC      p = GetDC( GetDlgItem( s_hDialog, IDC_COLOR_STATIC ) );
                HBRUSH a,b;
                b = CreateSolidBrush( RGB( s_LightC.R, s_LightC.G, s_LightC.B ) );
                a=(HBRUSH)SelectObject( p, b );
                GetWindowRect( GetDlgItem( s_hDialog, IDC_COLOR_STATIC ), &r );
                Rectangle( p, 0, 0, r.right - r.left, r.bottom - r.top );
                SelectObject( p, a );
                DeleteObject( b );
                ReleaseDC( GetDlgItem( s_hDialog, IDC_COLOR_STATIC ), p );
                break;
            }
        case WM_LBUTTONDOWN:
        case WM_KEYDOWN:
        case WM_MOUSEWHEEL:
            MainWindowProc( hwndDlg, uMsg, wParam, lParam );
            return 0;

    }

    e_handle;

    
    return 0;
}

//==============================================================================
static
void CreateWindows( void )
{
    //
    // Create the main window
    //
    {
        // Register the window class
        WNDCLASS wndClass = { CS_HREDRAW | CS_VREDRAW, MainWindowProc, 0, 0, d3deng_GetInstace(),
                              NULL,
                              LoadCursor(NULL, IDC_ARROW), 
                              (HBRUSH)GetStockObject(WHITE_BRUSH), NULL,
                              TEXT("Editor WinCLASS") };

        RegisterClass( &wndClass );

        // Load the menu
        HMENU hMenu = LoadMenu( d3deng_GetInstace(), MAKEINTRESOURCE( IDR_MENU1 ) );

        // Create our main window
        s_hMainWindow = CreateWindow( TEXT("Editor WinCLASS"),
                                      TEXT("Editor Window"),
                                      WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, 
                                      CW_USEDEFAULT, CW_USEDEFAULT, 
                                      CW_USEDEFAULT, CW_USEDEFAULT,
                                      0L,
                                      hMenu,
                                      d3deng_GetInstace(), 0L );

        ShowWindow( s_hMainWindow, SW_SHOWDEFAULT );
        UpdateWindow( s_hMainWindow );
    }

    //
    // Create the dialog menu
    //
    {
        s_hDialog = CreateDialog(   d3deng_GetInstace(), 
                                    MAKEINTRESOURCE(IDD_DIALOG1 ),
                                    s_hMainWindow,
                                    MainMenuDialogProc );


        ShowWindow( s_hDialog, SW_SHOWDEFAULT );
        UpdateWindow( s_hDialog );
    }

    //
    // Create the D3D window
    //
    {
        // Register the window class
        WNDCLASS wndClass = { CS_HREDRAW | CS_VREDRAW, D3DWindowProc, 0, 0, d3deng_GetInstace(),
                              NULL,
                              LoadCursor(NULL, IDC_ARROW), 
                              (HBRUSH)GetStockObject(WHITE_BRUSH), NULL,
                              TEXT("D3D WinCLASS") };

        RegisterClass( &wndClass );

        RECT Window;
        GetClientRect   ( s_hMainWindow, &Window );

        // Create our main window
        HWND D3DWin = CreateWindow( TEXT("D3D WinCLASS"),
                      TEXT("D3D Window"),
                      WS_VISIBLE | WS_CHILDWINDOW, CW_USEDEFAULT,
                      CW_USEDEFAULT, 
                      Window.right - Window.left, 
                      Window.bottom - Window.top, 
                      s_hMainWindow, 
                      0L, d3deng_GetInstace(), 0L );

        d3deng_SetWindowHandle( D3DWin );
        d3deng_SetParentHandle( s_hMainWindow );
    }

    //
    // Set the initial position of the window
    //
    {
        RECT Window;
        GetWindowRect( GetDesktopWindow(), &Window );
        d3deng_SetResolution( Window.right - Window.left, Window.bottom - Window.top );
        MoveWindow( s_hMainWindow, 100,0, 800, 600, TRUE );
    }
}

//=========================================================================

void UpdateSoundDescList( void )
{
    HWND ComboBox = GetDlgItem( s_hDialog, IDC_SOUNDTYPE );
    s32 Count = s_Editor.GetSoundNDescs();

    for( s32 i = 0; i < Count; i++ )
    {
        layout_editor::sound_desc desc = s_Editor.GetSoundDesc( i );

        char Name[256];
        s32 Size  = x_strlen( desc.Name ) ;
        s32 Start = iMax( 0, Size-16 );
        

        for( s32 j=Start; j<Size+1; j++ )
        {
            Name[j-Start] = desc.Name[j];
        }

        SendMessage( ComboBox, CB_ADDSTRING, 0, (LPARAM)Name );
    }

    SendMessage( ComboBox, CB_SETCURSEL, 0, 0 );
    
    layout_editor::sound_desc desc = s_Editor.GetSoundDesc( 0 );
    HWND Loop = GetDlgItem( s_hDialog, IDC_LOOP_SOUND );

    if( desc.Looped )
        SendMessage( Loop, BM_SETCHECK, BST_CHECKED, 0 );
    else
        SendMessage( Loop, BM_SETCHECK, BST_UNCHECKED, 0 );

    //SendMessage( ComboBox, CB_SETHORIZONTALEXTENT, 256,  0);
    //SendMessage( ComboBox, CB_SETDROPPEDWIDTH, 256,  0);
}

//=========================================================================

void UpdatePFXList( void )
{
    s32 Count = s_Editor.GetNumFXTypes();
    HWND PfxList;

    PfxList = GetDlgItem( s_hDialog, IDC_PFXTYPE );

    for ( s32 i = 0; i < Count; i++ )
    {
        char Name[256];
        s32  ID;

        s_Editor.GetFXType( i, Name, ID );
        s32 Idx = SendMessage( PfxList, CB_ADDSTRING, 0, (LPARAM)Name );
        SendMessage( PfxList, CB_SETITEMDATA, (WPARAM)Idx, (LPARAM)ID );
    }

    SendMessage( PfxList, CB_SETCURSEL, 0, 0 );
}

//=========================================================================

void UpdatePickupList( void )
{
    HWND PickupList;
    PickupList = GetDlgItem( s_hDialog, IDC_PICKUP_TYPE );

    SendMessage( PickupList, CB_ADDSTRING, 0, (LPARAM)"Wall" );
    SendMessage( PickupList, CB_SETITEMDATA, (WPARAM)0, (LPARAM)layout_editor::INST_FLAGS_WALL );

    SendMessage( PickupList, CB_ADDSTRING, 0, (LPARAM)"Ammo" );
    SendMessage( PickupList, CB_SETITEMDATA, (WPARAM)1, (LPARAM)layout_editor::INST_FLAGS_AMMO );

    SendMessage( PickupList, CB_ADDSTRING, 0, (LPARAM)"Health" );
    SendMessage( PickupList, CB_SETITEMDATA, (WPARAM)2, (LPARAM)layout_editor::INST_FLAGS_HEALTH );

    SendMessage( PickupList, CB_ADDSTRING, 0, (LPARAM)"Item" );
    SendMessage( PickupList, CB_SETITEMDATA, (WPARAM)3, (LPARAM)layout_editor::INST_FLAGS_ITEM );

    SendMessage( PickupList, CB_ADDSTRING, 0, (LPARAM)"Object" );
    SendMessage( PickupList, CB_SETITEMDATA, (WPARAM)4, (LPARAM)layout_editor::INST_FLAGS_OBJECT );

    SendMessage( PickupList, CB_SETCURSEL, 0, 0 );
    s_ObjectType = SendMessage( GetDlgItem( s_hMainWindow, IDC_PICKUP_TYPE ), CB_GETITEMDATA, 0, 0 );
/*    
    s32 Count = s_Editor.GetNumPickupTypes();
    HWND PickupList;

    PickupList = GetDlgItem( s_hMainWindow, IDC_PICKUP_TYPE );

    for ( s32 i = 0; i < Count; i++ )
    {
        char Name[256];
        s32  Type;

        s_Editor.GetPickupType( i, Name, Type );
        s32 Idx = SendMessage( PickupList, CB_ADDSTRING, 0, (LPARAM)Name );
        SendMessage( PickupList, CB_SETITEMDATA, (WPARAM)Idx, (LPARAM)Type );
    }

    SendMessage( PickupList, CB_SETCURSEL, 0, 0 );
*/
}

//=========================================================================

void Initialize( void )
{
    //
    // Create the windows and initialize the engine
    //
    CreateWindows();
    eng_Init();

    //
    // Initialize the geom_manager
    // - Support systems
    //
    gman_Init();

    //
    // Initialize the editor
    //
    // Set the working directory
    s_pDir = "C:\\GameData\\A51\\Release\\PC" ;
    s_Editor.SetWorkingDir( s_pDir );
    s_Editor.RefreshAllFiles();

    // Initialize the render helpers
    s_Grid.SetSeparation( 100, 100 );
    s_Grid.SetSize      ( 5000, 5000 );

    // Update the files in the editor
    UpdatePieceList();
    UpdateSoundDescList();
    UpdatePFXList();
    UpdatePickupList();

    // Initialize the view
    View.SetXFOV( R_60 );
    View.SetPosition( vector3(100,100,200) );
    View.LookAtPoint( vector3(  0,  0,  0) );
    View.SetZLimits ( 10, 50000 );
}

//==============================================================================
static
void Kill( void )
{
    gman_Kill();
}

//==============================================================================

void AppMain( s32, char** )
{
    xtimer Timer;

    // Now debug info for exceptions
    g_bErrorBreak = FALSE;
    
    
    Initialize();
//    s_EffectsContainerId = audio_LoadContainer(xfs("%s\\%s", g_RscMgr.GetRootDirectory(), "effects.pkg"));
    
    InitCommonControls();

    matrix4 L2W;
    L2W.Identity();

    L2W.RotateY( R_270 );

    eng_SetBackColor( xcolor( 32,32,32,255 ));

    while( TRUE )
    {
        // Get the time per frame
        s_tLastFrame = (f32)x_TicksToSec( Timer.Trip() );

        ///
        ///
        ///

 //       SetFocus( s_hMainWindow );
        if( !HandleInput() )
            break;

        //
        // Set the shader
        //

        eng_Begin();
        vram_Activate();
        eng_End();

        //
        // Start the rendering
        //
        QuickView();

        Render();

        x_printfxy( 0, 0, "%f", eng_GetFPS( ));
        // DONE!
        eng_PageFlip();

    }
    
//    audio_UnloadContainer( s_EffectsContainerId );
    Kill();
}
