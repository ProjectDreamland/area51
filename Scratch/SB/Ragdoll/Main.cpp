//==============================================================================
//
//  Main.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Render\Render.hpp"
#include "Animation\CharAnimPlayer.hpp"
#include "ResourceMgr\ResourceMgr.hpp"

#include "Loco\Loco.hpp"
#include "Ragdoll\Ragdoll.hpp"
#include "Ragdoll\Mutant1Ragdoll.hpp"
#include "Ragdoll\Mutant3Ragdoll.hpp"
#include "Ragdoll\MutantTankRagdoll.hpp"
#include "Ragdoll\HumanRagdoll.hpp"
#include "Ragdoll\GrayRagdoll.hpp"
#include "Ragdoll\ScientistRagdoll.hpp"
#include "Ragdoll\HazmatRagdoll.hpp"

int g_right_stick_swap_xy = 0;


static struct texture_loader : public rsc_loader
{
    texture_loader( void ) : rsc_loader( "TEXTURE", ".xbmp" ) {}

	//-------------------------------------------------------------------------

    virtual void* PreLoad( X_FILE* FP )
    {
        texture* pTexture = new texture;
        ASSERT( pTexture );

        pTexture->m_Bitmap.Load( FP );
        
        return( pTexture );
    }

	//-------------------------------------------------------------------------

    virtual void* Resolve( void* pData ) 
    {
        texture* pTexture = (texture*)pData;

        vram_Register( pTexture->m_Bitmap );

        return( pTexture );
    }

	//-------------------------------------------------------------------------

    virtual void Unload( void* pData )
    {
        texture* pTexture = (texture*)pData;

        vram_Unregister( pTexture->m_Bitmap );

        delete pTexture;
    }

} s_Texture_Loader;


static struct cubemap_loader : public rsc_loader
{
    cubemap_loader( void ) : rsc_loader( "CUBEMAP", ".envmap" ) {}

	//-------------------------------------------------------------------------

    virtual void* PreLoad( X_FILE* FP )
    {
        cubemap* pCubemap = new cubemap;
        ASSERT( pCubemap );

        for ( s32 i = 0; i < 6; i++ )
        {
            pCubemap->m_Bitmap[i].Load( FP );
        }

        return( pCubemap );
    }

	//-------------------------------------------------------------------------

    virtual void* Resolve( void* pData ) 
    {
        cubemap* pCubemap = (cubemap*)pData;

        for ( s32 i = 0; i < 6; i++ )
            vram_Register( pCubemap->m_Bitmap[i] );

        return( pCubemap );
    }

	//-------------------------------------------------------------------------

    virtual void Unload( void* pData )
    {
        cubemap* pCubemap = (cubemap*)pData;

        for ( s32 i = 0; i < 6; i++ )
        {
            vram_Unregister( pCubemap->m_Bitmap[i] );
        }

        delete pCubemap;
    }

} s_Cubemap_Loader;


static struct skin_loader : public rsc_loader
{
    //-------------------------------------------------------------------------
    
    skin_loader( void ) : rsc_loader( "SKIN GEOM", ".skingeom" ) {}

    //-------------------------------------------------------------------------
    
    virtual void* PreLoad ( X_FILE* FP )
    {
        fileio File;
        return( File.PreLoad( FP ) );
    }

    //-------------------------------------------------------------------------
    
    virtual void* Resolve ( void* pData ) 
    {
        fileio      File;
        skin_geom* pSkinGeom = NULL;

        File.Resolved( (fileio::resolve*)pData, pSkinGeom );

        return( pSkinGeom );
    }

    //-------------------------------------------------------------------------
    
    virtual void Unload( void* pData )
    {
        skin_geom* pSkinGeom = (skin_geom*)pData;
        ASSERT( pSkinGeom );
    
        delete pSkinGeom;
    }

} s_Skin_Geom_Loader;





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
//  STORAGE
//==============================================================================

f32             g_BlurOffset = 0 ;
xcolor          g_BlurColor  = XCOLOR_BLACK ;
view            View2;
view            View;
random          R;
s32             ScreenW;
s32             ScreenH;

#ifdef TARGET_PC
#pragma warning( disable : 4355 ) // warning 'this' used in base member initializer list
    const char*     DataPath = "";
#endif

#ifdef TARGET_GCN
    const char*     DataPath = "";
#endif

static ragdoll*                 s_pRagdoll = NULL ;
static xtimer                   s_LogicTime ;
static xbool                    s_bGeometry = TRUE ;
static xbool                    s_bSkeleton = TRUE ;
static xbool                    s_bBones    = FALSE;
static xbool                    s_Bone      = -1 ;
static loco_char_anim_player    s_AnimPlayer ;
static xbool                    s_bPlayAnim = TRUE ;
static s32                      s_AnimIndex = 0 ;
static vector3                  s_AnimVel ;

static f32                  TIME_STEP    = 1.0f / 30.0f ;
static f32                  BLAST_RADIUS = 800.0f;
static f32                  BLAST_AMOUNT = 1000.0f;

//==============================================================================

static void Initialize( void )
{
    eng_Init();
    render::Init() ;

    anim_event::Init() ;

    View.SetXFOV( R_60 );
    View.SetPosition( vector3(100, 250, -300) );
    View.LookAtPoint( vector3(  0,  0,  0) );
    View.SetZLimits ( 0.1f, 10000.0f );

    View2 = View;

    eng_GetRes( ScreenW, ScreenH );

    g_RscMgr.Init();

#ifdef TARGET_PC    
    g_RscMgr.SetRootDirectory( "C:\\GameData\\A51\\Release\\PC");
    g_RscMgr.SetOnDemandLoading( TRUE );

#endif
#ifdef TARGET_PS2    
    g_RscMgr.SetRootDirectory( "C:\\GameData\\A51\\Release\\PS2");
    g_RscMgr.SetOnDemandLoading( TRUE );
#endif

    // Create ragdoll
    s_pRagdoll = new ragdoll() ;
    ASSERT(s_pRagdoll) ;

    // TEST
 //   ragdoll RD ;
 //   RD.Init("Mut01_bindpose.skingeom", "Mut01.anim", HumanRagdoll, NULL) ;

    //s_pRagdoll->Init("Mut01_bindpose.skingeom", "Mut01.anim", Mutant1Ragdoll, NULL) ;
    //s_pRagdoll->Init("Mut03_bindpose.skingeom", "Mut03.anim", Mutant3Ragdoll, NULL) ;
    //s_pRagdoll->Init("hazmat_bindpose.skingeom", "hazmat.anim", HumanRagdoll, NULL) ;
    //s_pRagdoll->Init("gray_bindpose.skingeom", "gray.anim", GrayRagdoll, NULL) ;
    //s_pRagdoll->Init("Mut04_bindpose.skingeom", "Mut04.anim", MutantTankRagdoll, NULL) ;
    //s_pRagdoll->Init("Sci_bindpose.skingeom", "Sci.anim", SciRagdoll, NULL) ;
    //s_pRagdoll->Init("hazmat_bindpose.skingeom", "Spec4.anim", HazmatRagdoll, NULL) ;
    s_pRagdoll->Init("grunt_hazmat.skingeom", "strain_grunt.anim", Mutant1Ragdoll, NULL) ;

    // Setup animation
    s_AnimPlayer.SetAnimGroup(s_pRagdoll->GetAnimGroup()) ;
    //s_AnimIndex = s_AnimPlayer.GetAnimIndex("HSO_RUN_FORWARD") ;
    s_AnimIndex = 0;
    s_AnimPlayer.SetAnim(s_pRagdoll->GetAnimGroup(), s_AnimIndex, 0.25f) ;
    s_AnimPlayer.SetYaw(R_180) ;
    s_AnimVel.Zero() ;

    // Show sizes
    x_DebugMsg("Ragdoll    Size = %d bytes\n",    sizeof(ragdoll)) ;
    x_DebugMsg("Particles  Size = %d %d bytes\n", sizeof(particle),             sizeof(s_pRagdoll->m_Particles)) ;
    x_DebugMsg("Stickbones Size = %d %d bytes\n", sizeof(stick_bone),           sizeof(s_pRagdoll->m_StickBones)) ;
#ifdef X_DEBUG
    x_DebugMsg("DebugPlane Size = %d %d bytes\n", sizeof(ragdoll::debug_plane), sizeof(s_pRagdoll->m_DebugPlanes)) ;
#endif
    x_DebugMsg("\n\nStatic") ;
    x_DebugMsg("RagdollDef Size = %d bytes\n", sizeof(Mutant1Ragdoll)) ;
    x_DebugMsg("DistRules  Size = %d %d bytes\n", sizeof(dist_rule), sizeof(dist_rule) * Mutant1Ragdoll.m_NDistRules) ;
    x_DebugMsg("Geombones  Size = %d %d bytes\n", sizeof(geom_bone), sizeof(geom_bone) * Mutant1Ragdoll.m_NGeomBones) ;
}

//=========================================================================

static void Shutdown( void )
{
    // Kill ragdolls
    if (s_pRagdoll)
        delete s_pRagdoll ;

    // Kill systems
    render::Kill() ;
}

//=========================================================================

#ifdef TARGET_PC
#define INPUT_RESET INPUT_KBD_SPACE
#define INPUT_U     INPUT_KBD_R
#define INPUT_D     INPUT_KBD_F
#define INPUT_F     INPUT_KBD_W
#define INPUT_B     INPUT_KBD_S
#define INPUT_L     INPUT_KBD_A
#define INPUT_R     INPUT_KBD_D

#define INPUT_PITCH_SIGN    1
#define INPUT_PITCH INPUT_MOUSE_Y_REL

#define INPUT_YAW_SIGN  1
#define INPUT_YAW       INPUT_MOUSE_X_REL

#define INPUT_RESET         INPUT_KBD_SPACE
#define INPUT_RESET_BOMB    INPUT_KBD_B
#define INPUT_INFO          INPUT_KBD_C
#define INPUT_GEOMETRY      INPUT_KBD_G
#define INPUT_SKELETON      INPUT_KBD_H
#define INPUT_BONES         INPUT_KBD_J
#define INPUT_BONE_INC      INPUT_KBD_U
#define INPUT_BONE_DEC      INPUT_KBD_I

#define INPUT_ANIM          INPUT_KBD_RETURN
#define INPUT_NEXT_ANIM     INPUT_KBD_RBRACKET
#define INPUT_PREV_ANIM     INPUT_KBD_LBRACKET

#endif

#ifdef TARGET_PS2
#define INPUT_RESET INPUT_PS2_BTN_START     
#define INPUT_U     INPUT_PS2_BTN_R1
#define INPUT_D     INPUT_PS2_BTN_R2
#define INPUT_F     INPUT_PS2_BTN_L_UP
#define INPUT_B     INPUT_PS2_BTN_L_DOWN
#define INPUT_L     INPUT_PS2_BTN_L_LEFT
#define INPUT_R     INPUT_PS2_BTN_L_RIGHT

#define INPUT_PITCH_SIGN    10
#define INPUT_PITCH         INPUT_PS2_STICK_LEFT_Y

#define INPUT_YAW_SIGN      10
#define INPUT_YAW           INPUT_PS2_STICK_LEFT_X

#define INPUT_RESET_BOMB    INPUT_PS2_BTN_SELECT
#define INPUT_INFO          INPUT_PS2_BTN_TRIANGLE

#define INPUT_GEOMETRY      INPUT_PS2_BTN_SQUARE
#define INPUT_SKELETON      INPUT_PS2_BTN_CROSS
//#define INPUT_BONES         INPUT_PS2_BTN_CIRCLE
//#define INPUT_BONE_INC      INPUT_PS2_BTN_L1
//#define INPUT_BONE_DEC      INPUT_PS2_BTN_L2

#define INPUT_ANIM          INPUT_PS2_BTN_CIRCLE
#define INPUT_NEXT_ANIM     INPUT_PS2_BTN_L1
#define INPUT_PREV_ANIM     INPUT_PS2_BTN_L2




#endif

xbool HandleInput( f32 DeltaTime )
{
#ifdef TARGET_PC
    while( input_UpdateState() )
#else
    input_UpdateState() ;
#endif
    {
        // Move view using keyboard and mouse
        // WASD - forward, left, back, right
        // RF   - up, down
        // MouseRightButton - 4X speed

        radian Pitch;
        radian Yaw;
        f32    S = 300.0f * DeltaTime ;
        f32    R = 0.005f ;

        if (input_IsPressed(INPUT_KBD_ESCAPE  ))  return( FALSE );

        if (input_IsPressed(INPUT_MOUSE_BTN_L ))  S *= 4.0f;
        if (input_IsPressed(INPUT_F           ))  View.Translate( vector3( 0, 0, S), view::VIEW );
        if (input_IsPressed(INPUT_B           ))  View.Translate( vector3( 0, 0,-S), view::VIEW );
        if (input_IsPressed(INPUT_L           ))  View.Translate( vector3( S, 0, 0), view::VIEW );
        if (input_IsPressed(INPUT_R           ))  View.Translate( vector3(-S, 0, 0), view::VIEW );
        if (input_IsPressed(INPUT_U           ))  View.Translate( vector3( 0, S, 0), view::VIEW );
        if (input_IsPressed(INPUT_D           ))  View.Translate( vector3( 0,-S, 0), view::VIEW );

        View.GetPitchYaw( Pitch, Yaw );       
        Pitch += INPUT_PITCH_SIGN * input_GetValue( INPUT_PITCH ) * R;
        Yaw   -= INPUT_YAW_SIGN   * input_GetValue( INPUT_YAW ) * R;
        View.SetRotation( radian3(Pitch,Yaw,0) );

        if( input_IsPressed( INPUT_MSG_EXIT ) )
            return( FALSE );

        if( input_WasPressed( INPUT_RESET ) )
        {
            //vector3 P = s_AnimPlayer.GetPosition() ;
            s_AnimPlayer.SetPosition(vector3(0,0,0)) ;
            //vector3 V = View.GetPosition() ;
            //V -= P ;
            //View.SetPosition(V) ;
            s_AnimPlayer.GetCurrAnim().SetFrame(0) ;
            s_pRagdoll->Reset() ;
        }

        if( input_WasPressed( INPUT_RESET_BOMB ) )
        {
            // Turn off anim
            s_bPlayAnim = FALSE ;

            // Pick random bomb spot near feet
            vector3 P = s_pRagdoll->GetFeetPos() ;
            P.X += x_frand(-100,100) ;
            P.Y = 0 ;
            P.Z += x_frand(-100,100) ;

            // Apply directional blast to make the ragdoll fly away from the grenade
            //m_Ragdoll.ApplyBlast( Pain.Center, Pain.RadiusR1, MIN(1000, Pain.ForceR0*10.0f) );

            // Apply blast upwards so the ragdoll always fly up in the air
            //m_Ragdoll.ApplyBlast( Pain.Center, vector3(0,1,0), Pain.RadiusR1, MIN(2000, Pain.ForceR0*20.0f) ) ;

            // Blast!
            s_pRagdoll->ApplyBlast(P, BLAST_RADIUS, BLAST_AMOUNT) ;
            s_pRagdoll->ApplyBlast(P, vector3(0,1,0), BLAST_RADIUS, BLAST_AMOUNT*2) ;
        }
        
        if( input_WasPressed( INPUT_GEOMETRY ) )
            s_bGeometry ^= TRUE ;

        if( input_WasPressed( INPUT_SKELETON ) )
            s_bSkeleton ^= TRUE ;

#ifdef TARGET_PC
        if( input_WasPressed( INPUT_BONES ) )
            s_bBones ^= TRUE ;

        if( input_WasPressed( INPUT_BONE_INC ) )
            s_Bone++ ;

        if( input_WasPressed( INPUT_BONE_DEC ) )
        {
            if (s_Bone < -1)
                s_Bone = -1 ; 
            s_Bone-- ;
        }
#endif

        if( input_WasPressed( INPUT_ANIM ) )
        {
            // Give ragdoll vels from anim
            if (s_bPlayAnim)
            {
                vector3 DeltaPos ;
                s_AnimPlayer.Advance(1.0f / 30.0f, DeltaPos) ;
                s_pRagdoll->SetMatrices(s_AnimPlayer, DeltaPos) ;
            }

            s_bPlayAnim ^= TRUE ;
            s_AnimPlayer.SetPosition(vector3(0,0,0)) ;
            s_AnimPlayer.GetCurrAnim().SetFrame(0) ;
        }

        if( input_WasPressed( INPUT_NEXT_ANIM ) )
        {
            if (!s_bPlayAnim)
                s_AnimPlayer.SetPosition(vector3(0,0,0)) ;

            s_bPlayAnim = TRUE ;

            if (++s_AnimIndex == s_AnimPlayer.GetNAnims())
                s_AnimIndex = 0 ;

            s_AnimPlayer.SetAnim(s_pRagdoll->GetAnimGroup(), s_AnimIndex, 0.25f) ;
        }

        if( input_WasPressed( INPUT_PREV_ANIM ) )
        {
            if (!s_bPlayAnim)
                s_AnimPlayer.SetPosition(vector3(0,0,0)) ;

            s_bPlayAnim = TRUE ;

            if (--s_AnimIndex < 0)
                s_AnimIndex = s_AnimPlayer.GetNAnims() -1 ;

            s_AnimPlayer.SetAnim(s_pRagdoll->GetAnimGroup(), s_AnimIndex, 0.25f) ;
        }
    }

    return( TRUE );
}



//==============================================================================

void Render( void )
{
     // Look at ragdoll, keeping camera at least 300 away
    vector3 F = s_pRagdoll->GetCenterPos() ;
    vector3 E = View.GetPosition() ;
    vector3 D = F - E ;
    f32     Dist = D.Length() ;
    if (Dist > 700)
    {
        D.Normalize() ;
        E += D * (Dist - 700) ;
    }        
    View.LookAtPoint(E, F) ;

    eng_MaximizeViewport( View );
    eng_SetView         ( View, 0 );
    eng_ActivateView    ( 0 );

    //==---------------------------------------------------
    //  GRID, BOX AND MARKER
    //==---------------------------------------------------
    eng_Begin( "GridBoxMarker" );
    {
        draw_ClearL2W();

        /*
        draw_Begin(DRAW_QUADS, DRAW_CULL_NONE) ;
        draw_Color(xcolor(0,0,255,255)) ;
        draw_Vertex(-5000,0,-5000) ;
        draw_Vertex(5000,0,-5000) ;
        draw_Vertex(5000,0,5000) ;
        draw_Vertex(-5000,0,5000) ;
        draw_End() ;
        */

        draw_Grid( vector3(  -5000,   0,    -5000), 
                   vector3(10000,  0,    0), 
                   vector3(  0,   0, 10000), 
                   xcolor (  0,32,  0), 32 );

    }
    eng_End();


    //==---------------------------------------------------
    //  Render rag doll
    //==---------------------------------------------------
    if( 1 )
    {
        eng_Begin("ragdoll") ;
        render::Begin() ;

        if (s_bGeometry)
            s_pRagdoll->RenderGeometry() ;
        
        if (s_bSkeleton)
            s_pRagdoll->RenderSkeleton() ;

        if (s_bBones)
            s_pRagdoll->RenderBones(s_Bone) ;

        render::End() ;
        eng_End();
    }
}

//==============================================================================

void Advance( f32 Seconds )
{
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
    static f32 DT = 0 ;
    DT += V ;
    if (V == 0)
        s_pRagdoll->Advance(0) ;
    while(DT > 0)
    {
        f32 Step = MAX(DT, TIME_STEP) ;
        s_pRagdoll->Advance(Step) ;
        DT -= Step ;
    }

    // Advance anim
    vector3 Pos = s_AnimPlayer.GetPosition() ;
    s_AnimPlayer.Advance(V, s_AnimVel) ;
    Pos += s_AnimVel ;
    s_AnimPlayer.SetPosition(Pos) ;
    s_AnimPlayer.SetYaw(s_AnimPlayer.GetAnimInfo().GetHandleAngle()) ;

    // Move camera
    if (s_bPlayAnim)
    {
        Pos = View.GetPosition() ;
        Pos += s_AnimVel ;
        View.SetPosition(Pos) ;
    }

    // Setup particles from animation
    if (s_bPlayAnim)
    {
        loco_char_anim_player AnimRestore = s_AnimPlayer ;
        s_pRagdoll->SetMatrices(s_AnimPlayer, s_AnimVel) ;
        s_AnimPlayer = AnimRestore ;
    }
}

//==============================================================================

void AppMain( s32, char** )
{
    Initialize();
    xtimer Timer;
   
    Timer.Start() ;

    while( TRUE )
    {
        f32 DeltaTime = Timer.TripSec() ;
        // Cap incase we go in the debugger
        if (DeltaTime > (1.0f / 60.0f))
            DeltaTime = 1.0f / 60.0f ;

        if( !HandleInput(DeltaTime) )
            break;


        s_LogicTime.Reset() ;
        s_LogicTime.Start() ;
        Advance(DeltaTime) ;
        s_LogicTime.Stop() ;

        Render();

        // Show debug info
        #ifdef TARGET_PS2
            s32 X = 0 ;
            s32 Y = 23 ;
        #endif

        #ifdef TARGET_PC
            s32 X = 0 ;
            s32 Y = 41 ;
        #endif

        x_printfxy(X,   Y,"Logic:%.2f",   s_LogicTime.ReadMs()) ;
        x_printfxy(X+11,Y,"Parts:%d",     s_pRagdoll->m_NParticles) ;
        x_printfxy(X+20,Y,"DistRules:%d", s_pRagdoll->m_pDef->m_NDistRules) ;
        x_printfxy(X+0,Y-1,"%s",          s_AnimPlayer.GetAnimInfo().GetName()) ;
        x_printfxy(X, Y-2, "Kinetic Energy %f", s_pRagdoll->GetKineticEnergy()) ;

        // DONE!
        eng_PageFlip();
    }

    Shutdown();
}

//==============================================================================
