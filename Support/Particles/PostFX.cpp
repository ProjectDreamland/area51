//==============================================================================
//
//  PostFX.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================
#include "entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Fx.hpp"
#include "ParticleEffect.hpp"
#include "ParticleEmitter.hpp"

#include "PostFX.hpp"

extern view View;

static xbitmap*  s_Glow = NULL ;
static xbool CheckPointVis( const vector3& Pt );

f32 GetDeltaTime( void );

//==============================================================================
// Add an effect element to be rendered later
void postfx::AddFX   ( const vector3& Pt, const vector2& Size, const xcolor& Color, f32 Life, bool CheckVis )
{
    // skip the living
    while ( ( m_Idx < MAX_POST_FX ) && ( m_FX[ m_Idx ].m_Life > 0.0f ) )
        m_Idx++;

    if ( m_Idx < MAX_POST_FX )
    {
        m_FX[ m_Idx   ].m_Pt[0] =       Pt;
        m_FX[ m_Idx   ].m_Size =        Size;
        m_FX[ m_Idx   ].m_Color =       Color;
        m_FX[ m_Idx   ].m_Type =        postfx_element::SPOT;
        m_FX[ m_Idx   ].m_Age =         0.0f;
        m_FX[ m_Idx   ].m_Life =        Life;
        m_FX[ m_Idx   ].m_IsVis =       true;
        m_FX[ m_Idx++ ].m_CheckVis =    CheckVis;
    }   
}


//==============================================================================
// Add an oriented quad
void postfx::AddFX   ( const vector3& Pt1, const vector3& Pt2, f32 Size, const xcolor& Color, bool CheckVis )
{
    // skip the living
    while ( ( m_Idx < MAX_POST_FX ) && ( m_FX[ m_Idx ].m_Life > 0.0f ) )
        m_Idx++;

    if ( m_Idx < MAX_POST_FX )
    {
        m_FX[ m_Idx   ].m_Pt[0] =       Pt1;
        m_FX[ m_Idx   ].m_Pt[1] =       Pt2;
        m_FX[ m_Idx   ].m_Size.X =      Size;
        m_FX[ m_Idx   ].m_Color =       Color;
        m_FX[ m_Idx   ].m_Life =        0.01f;
        m_FX[ m_Idx   ].m_Age =         0.0f;
        m_FX[ m_Idx   ].m_Type =        postfx_element::QUAD;
        m_FX[ m_Idx   ].m_IsVis =       true;
        m_FX[ m_Idx++ ].m_CheckVis =    CheckVis;
    }
}


//==============================================================================
/*
    // Pass1 - clear alpha only in back buffer - use draw to draw a colord rectangle
    xcolor(0,0,0,0) ;
    gsreg_SetZBufferTest(ZBUFFER_TEST_ALWAYS);
    gsreg_SetZBufferUpdate( FALSE ) ;
    gsreg_SetAlphaBlend( ALPHA_BLEND_MODE_OFF ) ;   //finalpixel=((ColA - ColB)*AlphaC) + ColD ;

    // Pass2 - set alpha to 255 with z test
    gsreg_SetZBufferTest(ZBUFFER_TEST_GEQUAL) ;
    gsreg_SetZBufferUpdate( FALSE ) ;
    gsreg_SetAlphaBlend( ALPHA_BLEND_MODE_OFF ) ;   //finalpixel=((ColA - ColB)*AlphaC) + ColD ;

    // Pass2.5 - copy from 1x1 pixel to size of sprite
    gsreg_SetZBufferTest(ZBUFFER_TEST_ALWAYS);
    gsreg_SetZBufferUpdate( FALSE ) ;
    gsreg_SetAlphaBlend( ALPHA_BLEND_MODE_OFF ) ;   //finalpixel=((ColA - ColB)*AlphaC) + ColD ;

    // Pass3 - render glow texture alpha * dest alpha into dest alpha channel only
    gsreg_SetZBufferTest(ZBUFFER_TEST_ALWAYS);
    gsreg_SetZBufferUpdate( FALSE ) ;
    gsreg_SetAlphaBlend( ALPHA_BLEND_MODE(C_SRC,C_ZERO,A_DST,C_ZERO) );   //finalpixel=((SrcCol - 0)*DstA) + 0 ;

    // Pass4 - render final image into rgba
    gsreg_SetZBufferTest(ZBUFFER_TEST_ALWAYS);
    gsreg_SetZBufferUpdate( FALSE ) ;
    gsreg_SetAlphaBlend( ALPHA_BLEND_MODE(C_SRC,C_ZERO,A_DST,C_DST) );   //finalpixel=((SrcCol - 0)*DstA) + DstC ;
*/

xbool DoPostFX = TRUE;

// Render all collected effects
#define VIS_CHKS        5
void postfx::Render  ( void )
{
    if( !DoPostFX ) return;   //!!!!
	
	
static s32 vischk = 0;
    s32 vischk_end = vischk + VIS_CHKS; 
    //xtimer FXTime;

    //FXTime.Start();

    s32 i ;
    f32 DeltaTime = GetDeltaTime();//0.033f;//tgl.DeltaLogicTime; //(tgl.LogicTimeMs % 1000) / 1000.0f ;

//    x_printfxy( 2, 2, "%f", DeltaTime );

    // setup the hardware stuff
    if( eng_Begin("POSTFX_RENDER") )
    {
        draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_USE_ALPHA );
#ifdef TARGET_PS2
        gsreg_Begin();
        //gsreg_SetAlphaBlend( ALPHA_BLEND_MODE(C_SRC,C_ZERO,A_DST,C_DST) );   //finalpixel=((ColA - ColB)*AlphaC) + ColD ;
        gsreg_SetAlphaBlend( ALPHA_BLEND_MODE(C_SRC,C_ZERO,A_SRC,C_DST) );
        gsreg_SetZBufferUpdate(FALSE);
        gsreg_SetZBufferTest(ZBUFFER_TEST_ALWAYS); //gsreg_SetZBufferTest(ZBUFFER_TEST_GEQUAL);
        gsreg_End();
#endif
        
        // initialize the texture if necessary
        if ( s_Glow == NULL )
            s_Glow = &g_FxTextures[FXTEX_GLOW];

        // make it the current texture    
        draw_SetTexture( *s_Glow );

        // render each point sprite
        for ( i = 0; i < MAX_POST_FX; i++ )
        {
           
            //if ( m_FX[ i ].m_Type == postfx_element::SPOT )
            {
                xcolor Col = m_FX[ i ].m_Color;

                // ensure a minimum color brightness (excluding alpha) ... can't have a dark glow, can we?
                while ( ( Col.R + Col.G + Col.B ) < 384 )
                {
                    Col.R += ( ( 255 - Col.R ) >> 1 );
                    Col.G += ( ( 255 - Col.G ) >> 1 );
                    Col.B += ( ( 255 - Col.B ) >> 1 );
                }

                // if it's one of them weird persistent glows...
                if ( m_FX[ i ].m_Life > 0.0f )
                {
                    if ( m_FX[ i ].m_Age < m_FX[ i ].m_Life )
                    {
                        Col.A = (u8)( (1.0f - (m_FX[ i ].m_Age / m_FX[ i ].m_Life )) * m_FX[ i ].m_Color.A );
                        m_FX[ i ].m_Age += DeltaTime;

                        // kill it if it's now too old
                        if ( m_FX[ i ].m_Age > m_FX[ i ].m_Life )
                            m_FX[ i ].m_Life = 0.0f;
                    }
                    else
                        // it's dead
                        m_FX[ i ].m_Life = 0.0f;

                    // draw it
                    // check visibility first 
                    if ( m_FX[ i ].m_CheckVis )
                    {
                        // only check visibility on our current set of glowies
                        // if ( ( i >= vischk ) && ( i < vischk_end ) )
                        {
                            m_FX[i].m_IsVis = CheckPointVis( m_FX[ i ].m_Pt[0] );
                        }

                        if ( m_FX[i].m_IsVis )
                            draw_Sprite( m_FX[ i ].m_Pt[0], m_FX[ i ].m_Size, Col );
                    }
                    else
                        draw_Sprite( m_FX[ i ].m_Pt[0], m_FX[ i ].m_Size, Col );


                    // RenderGlow ( m_FX[ i ].m_Pt[0], m_FX[ i ].m_Size.X, Col );       <-- R.I.P. for now
                }    
            }
        }

        vischk += VIS_CHKS;

        if ( vischk >= MAX_POST_FX )
            vischk = 0;

        // shut down sprites...
        draw_End();

        // get ready for quads...
        /*
        draw_Begin( DRAW_TRIANGLES, DRAW_TEXTURED | DRAW_USE_ALPHA );

        // TODO: Set appropriate texture
        for ( i = 0; i < MAX_POST_FX; i++ )
        {
            if ( m_FX[ i ].m_Type == postfx_element::QUAD )
            {
                if ( m_FX[ i ].m_Life > 0.0f )
                {
                    xcolor Col = m_FX[ i ].m_Color;

                    // ensure a minimum color brightness (excluding alpha) ... can't have a dark glow, can we?
                    while ( ( Col.R + Col.G + Col.B ) < 384 )
                    {
                        Col.R += ( ( 255 - Col.R ) >> 1 );
                        Col.G += ( ( 255 - Col.G ) >> 1 );
                        Col.B += ( ( 255 - Col.B ) >> 1 );
                    }      
        
                    draw_OrientedQuad( m_FX[ i ].m_Pt[0], m_FX[ i ].m_Pt[1], vector2(0.4f,0.0f), vector2(0.6f,1.0f),
                                        m_FX[ i ].m_Color, m_FX[ i ].m_Color, m_FX[ i ].m_Size.X );

                    m_FX[ i ].m_Life = 0.0f;
                }
            }
        }

        // shut down quads...
        draw_End();
        */


        // reset the hardware
#ifdef TARGET_PS2
        gsreg_Begin();
        gsreg_SetZBufferUpdate(TRUE);
        gsreg_SetZBufferTest(ZBUFFER_TEST_GEQUAL);
        gsreg_End();
#endif

        eng_End();
    }
    //FXTime.Stop();

    //x_printfxy(2, 2, "%f\n", FXTime.ReadMs() );

}


//==============================================================================

vector2 postfx::CalcUV( u32 x, u32 y )
{
    return vector2( (f32)x / 512.0f, (f32)y / 512.0f );
}

vector2 CalcUV2( f32 x, f32 y )
{
    static s32 UV=1 ;
    if (UV)
        return vector2( x / 512.0f, y / 512.0f );
    else
        return vector2( x / 512.0f, y / 448.0f );
}

//==============================================================================
#ifdef TARGET_PS2

void postfx::SetTextureSource (s32 FrameIndex)
{
    ASSERT((FrameIndex >= 0) && (FrameIndex <= 1)) ;

    u64 Data ;

    gsreg_Begin() ;

    // Setup flush texture register
    gsreg_Set( SCE_GS_TEXFLUSH, 0) ;

    // Set TEX1 (TURN OFF MIPS)
    gsreg_Set( (SCE_GS_TEX1_1+eng_GetGSContext()), 0 ) ;

    // Set TEX0
    sceGsTex0* Tex0 = (sceGsTex0*)&Data ;
    *(u64*)Tex0 = 0 ;

    u32 FBAddr = eng_GetFrameBufferAddr(FrameIndex) ;
    s32 Width, Height ;
    eng_GetRes(Width, Height) ;

    ASSERT((FBAddr & 63) == 0) ;

    static s32 TEX=64 ;

    Tex0->TBP0 = FBAddr / TEX ;
    ASSERT((Width & 63) == 0) ;
    Tex0->TBW  = Width/64 ;
    Tex0->PSM  = 0 ;    // PSMCT32
    Tex0->TW   = 9 ;   // actual width=2^pw=512
    Tex0->TH   = 9 ;   // actual height=2^ph=512
    Tex0->TCC  = 1 ;    // RGBA
    Tex0->TFX  = 1 ;    // Decal (ignore vertex colors)
    gsreg_Set( (SCE_GS_TEX0_1+eng_GetGSContext()), Data );

    // Set MIPTBP1
    gsreg_Set( (SCE_GS_MIPTBP1_1+eng_GetGSContext()), 0 ) ;

    // Set MIPTBP2
    gsreg_Set( (SCE_GS_MIPTBP2_1+eng_GetGSContext()), 0 ) ;

    gsreg_End() ;
}

//==============================================================================

void postfx::RenderGlow ( const vector3& Pt, f32 Size, xcolor Color )
{
    // HEY JOHN - THESE SETTINGS GET IT WORKING (ALTHOUGH THE COLOR ALPHA DOES NOT SEEM TO BE COMING THROUGH)
    // THE FLICKERING CAUSES THE WHOLE SPRITE TO FLICKER THOUGH 8(
    // STEVE.

    static xcolor COL = xcolor(255,0,0,128);
    static s32 PASS1 = 1 ;
    static s32 PASS2 = 1 ;
    static s32 PASS3 = 1 ;
    static s32 PASS4 = 1 ;
    static s32 GLOWSIZE=20 ;
    static s32 PIXELSIZE = 20 ;
    static s32 PIXELSIZE2 = 30 ;


    Size = GLOWSIZE ;
    //Color = COL;

    // skip if not visible
    //if ( tgl.GetView().PointInView( Pt ) == FALSE )
    if ( eng_GetView()->PointInView( Pt ) == FALSE )
        return;

    eng_PushGSContext(0);


    // Convert world space to screen space
    vector4 WPt = vector4(Pt.X, Pt.Y, Pt.Z, 1.0f) ;        
    //vector4 SPt = tgl.GetView().GetW2S() * WPt ;
    vector4 SPt = eng_GetView()->GetW2S() * WPt ;

    static f32 ZBIAS=0 ;
    SPt.Z += ZBIAS ;

    // Project onto screen
    f32 OneOverW = 1.0f / SPt.W ;
    SPt *= OneOverW ;

#define OFFSET_X    (2048-(512/2))
#define OFFSET_Y    (2048-(512/2))
    SPt.X -= OFFSET_X ;
    SPt.Y -= OFFSET_Y ;

    // Copy into 2d point
    vector3 Pt2D ;
    Pt2D.X = SPt.X ;
    Pt2D.Y = SPt.Y ;
    Pt2D.Z = SPt.Z * 16.0f ;

    vector3 SprPt2D = Pt2D ;
    SprPt2D.X -= Size/2 ;
    SprPt2D.Y -= Size/2 ;

    static s32 UVMULT=16;
    vector2 SprUV2D ;
    SprUV2D.X = Pt2D.X * UVMULT ;
    SprUV2D.Y = Pt2D.Y * UVMULT ;

    vector2 uv = CalcUV2( Pt2D.X, Pt2D.Y );
    SprUV2D = uv ;


    if ( PASS1 )
    {
        // Step 1: Render rectangle to clear backbuffer
        //vram_Sync() ;
        //draw_Begin( DRAW_QUADS, DRAW_2D | DRAW_2D_KEEP_Z);
        draw_Begin( DRAW_TRIANGLES, DRAW_2D );//| DRAW_NO_ZBUFFER | DRAW_2D_KEEP_Z);
        gsreg_Begin();
        gsreg_SetZBuffer( FALSE ) ;
        gsreg_SetZBufferUpdate( FALSE ) ;
        gsreg_SetAlphaBlend( ALPHA_BLEND_OFF ) ;
        gsreg_SetFBMASK ( 0x00);//FFFFFF );
        gsreg_End();

        draw_Color ( xcolor(0,0,0,0) );
        /*
        draw_Vertex( Pt2D.X - PIXELSIZE, Pt2D.Y - PIXELSIZE, Pt2D.Z ); 
        draw_Vertex( Pt2D.X - PIXELSIZE, Pt2D.Y + PIXELSIZE, Pt2D.Z ); 
        draw_Vertex( Pt2D.X + PIXELSIZE, Pt2D.Y + PIXELSIZE, Pt2D.Z ); 
        draw_Vertex( Pt2D.X + PIXELSIZE, Pt2D.Y - PIXELSIZE, Pt2D.Z ); */

        draw_Vertex( Pt2D.X - PIXELSIZE2, Pt2D.Y - PIXELSIZE2, Pt2D.Z ); 
        draw_Vertex( Pt2D.X - PIXELSIZE2, Pt2D.Y + PIXELSIZE2, Pt2D.Z ); 
        draw_Vertex( Pt2D.X + PIXELSIZE2, Pt2D.Y + PIXELSIZE2, Pt2D.Z ); 

        draw_Vertex( Pt2D.X + PIXELSIZE2, Pt2D.Y + PIXELSIZE2, Pt2D.Z ); 
        draw_Vertex( Pt2D.X + PIXELSIZE2, Pt2D.Y - PIXELSIZE2, Pt2D.Z ); 
        draw_Vertex( Pt2D.X - PIXELSIZE2, Pt2D.Y - PIXELSIZE2, Pt2D.Z ); 

        /*draw_Vertex( 0, 0, 0 );
        draw_Vertex( 0, 447, 0 );
        draw_Vertex( 511, 447, 0);

        draw_Vertex( 511, 447, 0);
        draw_Vertex( 511, 0, 0 );
        draw_Vertex( 0,0,0 );*/

        draw_End();
    }

    if ( PASS2 )
    {
        // Step 2: Render rectangle to set backbuffer using z buffer
        //vram_Sync() ;
        //draw_Begin( DRAW_QUADS, DRAW_2D | DRAW_2D_KEEP_Z);
        draw_Begin( DRAW_TRIANGLES, DRAW_2D );//| DRAW_2D_KEEP_Z);
        gsreg_Begin();
        gsreg_SetZBuffer( TRUE ) ;
        gsreg_SetZBufferUpdate( FALSE ) ;
        gsreg_SetAlphaBlend( ALPHA_BLEND_OFF ) ;
        //gsreg_SetFBMASK ( 0x00 );
        gsreg_SetFBMASK ( 0x00);//FFFFFF );
        gsreg_End();

        draw_Color ( Color ); //Color.A) );
        /*draw_Vertex( Pt2D.X - PIXELSIZE, Pt2D.Y - PIXELSIZE, Pt2D.Z ); 
        draw_Vertex( Pt2D.X - PIXELSIZE, Pt2D.Y + PIXELSIZE, Pt2D.Z ); 
        draw_Vertex( Pt2D.X + PIXELSIZE, Pt2D.Y + PIXELSIZE, Pt2D.Z ); 
        draw_Vertex( Pt2D.X + PIXELSIZE, Pt2D.Y - PIXELSIZE, Pt2D.Z ); */

        draw_Vertex( Pt2D.X - PIXELSIZE, Pt2D.Y - PIXELSIZE, Pt2D.Z ); 
        draw_Vertex( Pt2D.X - PIXELSIZE, Pt2D.Y + PIXELSIZE, Pt2D.Z ); 
        draw_Vertex( Pt2D.X + PIXELSIZE, Pt2D.Y + PIXELSIZE, Pt2D.Z ); 

        draw_Vertex( Pt2D.X + PIXELSIZE, Pt2D.Y + PIXELSIZE, Pt2D.Z ); 
        draw_Vertex( Pt2D.X + PIXELSIZE, Pt2D.Y - PIXELSIZE, Pt2D.Z ); 
        draw_Vertex( Pt2D.X - PIXELSIZE, Pt2D.Y - PIXELSIZE, Pt2D.Z ); 

        draw_End();
    }

    static s32 IN2D = 0 ;

    if ( PASS3 )
    {
        // Step 3: render a full-size rectangle (in 3D) over the pixel using the alpha of the pixel
        // Note: yes, it's going to overwrite itself, but that is ok, because basically it's
        // simply replicating itself, so the referenced pixel will remain unchanged
        vram_Sync() ;
        if (IN2D)
            draw_Begin( DRAW_SPRITES, DRAW_2D | DRAW_2D_KEEP_Z | DRAW_TEXTURED | DRAW_NO_ZBUFFER );
        else
            draw_Begin( DRAW_SPRITES, DRAW_TEXTURED | DRAW_NO_ZBUFFER );
    
        gsreg_Begin();
        //gsreg_SetZBufferTest(ZBUFFER_TEST_ALWAYS);
        //gsreg_SetZBufferUpdate( FALSE ) ;
        gsreg_SetZBuffer( FALSE ) ;
        gsreg_SetAlphaBlend( ALPHA_BLEND_OFF ) ;
        gsreg_SetFBMASK ( 0x00FFFFFF );    
        gsreg_End();
   
        SetTextureSource( 0 );
        if (IN2D)
            draw_SpriteUV( SprPt2D, vector2(Size,Size), SprUV2D, SprUV2D, XCOLOR_WHITE );
        else
            draw_SpriteUV( Pt, vector2(Size,Size), uv, uv, XCOLOR_WHITE );


        draw_End();
    }


    if ( PASS4 )
    {
        // Step 4: render final image into rgba
        vram_Sync() ;
        if (IN2D)
            draw_Begin( DRAW_SPRITES, DRAW_2D | DRAW_2D_KEEP_Z | DRAW_USE_ALPHA | DRAW_TEXTURED | DRAW_NO_ZBUFFER );
        else
            draw_Begin( DRAW_SPRITES, DRAW_USE_ALPHA | DRAW_TEXTURED | DRAW_NO_ZBUFFER );

        gsreg_Begin();
        //gsreg_SetZBufferTest(ZBUFFER_TEST_ALWAYS);
        //gsreg_SetZBufferUpdate( FALSE ) ;
        gsreg_SetZBuffer( FALSE ) ;
        gsreg_SetAlphaBlend( ALPHA_BLEND_MODE(C_SRC,C_ZERO,A_DST,C_DST) );   //finalpixel=((SrcCol - 0)*DstA) + DstC ;
        gsreg_SetFBMASK ( 0x00 );
        gsreg_End();

        // make it the current texture    
        draw_SetTexture( *s_Glow );

        // draw the sprite
        if (IN2D)
            draw_Sprite( SprPt2D, vector2(Size,Size), Color );
        else
            draw_Sprite( Pt, vector2(Size,Size), Color );

        draw_End();
    }
    vram_Sync() ;
    
    eng_PopGSContext();
}

#endif

static xbool CheckPointVis( const vector3& Pt )
{
    
    // make a final check to see if the point is visible
    vector3 seg = Pt - View.GetPosition();

    if ( seg.LengthSquared() > ( 3000.0f * 3000.0f ) )
        return false;

    g_CollisionMgr.RaySetup( 0, View.GetPosition(), Pt );
    g_CollisionMgr.CheckCollisions( );
    
    if ( g_CollisionMgr.m_nCollisions )
    {
        if ( g_CollisionMgr.m_Collisions[0].T > 0.90f )
            return true;
        else
            return false;
    }

    /*
    collider                Collider;
    
    Collider.RaySetup( NULL, eng_GetView()->GetPosition(), Pt, -1, TRUE );

    ObjMgr.Collide( object::ATTR_SOLID_STATIC, Collider );

    return !(Collider.HasCollided());*/

    return true;
}

f32 GetDeltaTime( void )
{
    f32         DeltaTime;      //  <--- change this for later
    static f32  CurrentTime = 0.0f;
    static f32  LastTime = 0.0f;
    static xtimer      Timer;

    if ( CurrentTime == 0.0f )
    {
        Timer.Start();
        CurrentTime = Timer.ReadSec();
        LastTime = CurrentTime;
        DeltaTime = 0.033f;
    }
    else
    {
        CurrentTime = Timer.ReadSec();
        DeltaTime = CurrentTime - LastTime;
        LastTime = CurrentTime;
    }

    return DeltaTime;
}
