
///////////////////////////////////////////////////////////////////////////
// INCLUDES 
///////////////////////////////////////////////////////////////////////////

//#ifndef X_RETAIL
//#define D3DCOMPILE_NOTINLINE 1
//#endif


#include "xbox_private.hpp"

#if (!defined CONFIG_RETAIL) && (!defined _D3D8PERF_H_)
#include <D3d8perf.h>
#endif

#include "e_Engine.hpp"
#include "xbox_font.hpp"

static s32 eng_BeginCount = 0;

render::texture_stage_state g_TextureStageState;
render::render_state        g_RenderState;

///////////////////////////////////////////////////////////////////////////
// SWITCHES
///////////////////////////////////////////////////////////////////////////

#define SWITCH_ADAPTIVE_FRAMERATE   1
#define DEFAULT_ALIGNMENT           16

#ifndef X_RETAIL
#   define SWITCH_30_FPS            1
#   define SWITCH_60_FPS            0
#else
#    ifdef X_QA
#       define SWITCH_30_FPS        1
#       define SWITCH_60_FPS        0
#    else
#       define SWITCH_30_FPS        1
#       define SWITCH_60_FPS        0
#    endif
#endif

#define CONFIG_IS_DEMO 0

#if CONFIG_IS_DEMO
#   include "audio_hardware.hpp"
#   include "x_threads.hpp"
    s32 g_DemoMode            = 0;
    s32 g_DemoTimeout         = 4*60;
    s32 g_DemoInactiveTimeout = 1*60;
    LD_DEMO* s_pDemoData;
    LAUNCH_DATA s_ld;
#endif


///////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////

#define ENG_FONT_SIZEX      10
#define ENG_FONT_SIZEY      16
#ifndef CONFIG_RETAIL
#define SCRATCH_MEM_SIZE    (262144*8) // 1MB
#else
#define SCRATCH_MEM_SIZE    (262144*8) // 1MB
#endif
#define ENG_MAX_WINDOWS     8

///////////////////////////////////////////////////////////////////////////
// EXTERNAL DEPENDENCIES
///////////////////////////////////////////////////////////////////////////

void  vram_Init         ( void );
void  vram_Kill         ( void );

void  smem_Init         ( s32 NBytes );
void  smem_Kill         ( void );

void  draw_Init         ( void );
void  draw_Kill         ( void );

///////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
///////////////////////////////////////////////////////////////////////////

#if SWITCH_USE_DEFERRED_RENDERING
IDirect3DDevice8* g_pd3dInternal = NULL;
#else
#define g_pd3dInternal g_pd3dDevice
IDirect3DDevice8* g_pd3dDevice = NULL;
#endif

xtimer rst;
s32 rstct;
xbool g_EngWaitingForRetrace = FALSE;
volatile s32 g_VBlankCount = 0;
s32 s_FrameRateLock = 1;

static f32 s_MsPerFrame;
xbool g_bClearingViewport = TRUE;
xbool g_bPresentPending = FALSE;
xbool g_b480P           = FALSE;
xbool g_b720P           = FALSE;
xbool g_bFlippable      = TRUE;
xbool g_bPAL            = FALSE;

static s32 s_ShowSafeArea = 1;

u32 g_LEdge;
u32 g_TEdge;
u32 g_REdge;
u32 g_BEdge;

u32 g_PhysW;
u32 g_PhysH;
u32 g_PhysFPS;

texture_factory g_TextureFactory;
vert_factory g_VertFactory;
push_factory g_PushFactory;
texture_mgr g_Texture;

///////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
///////////////////////////////////////////////////////////////////////////

static struct eng_locals
{
    eng_locals( void ) { memset( this, 0, sizeof(eng_locals) ); }

    //
    // general variables
    //
    IDirect3D8*     pD3D;
    xcolor          TextColor;
    xcolor          BackColor;
    xbool           bBeginScene;
    xbool           bD3DBeginScene;

    //
    // View variables
    //
    view            View;

    //
    // Window manechment related varaibles
    //
    xbool           bActive;
    xbool           bReady;      
    RECT            rcWindowRect;
    xbool           bWindowed;   
    s32             MaxXRes;
    s32             MaxYRes;

    //
    // FPS variables
    //
    xtick           FPSFrameTime[8];
    xtick           FPSLastTime;
    s32             FPSIndex;
    xtimer          CPUTIMER;
    f32             CPUMS;
    f32             IMS;
} s;

///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////

//=========================================================================

void xbox_EntryPoint( void )
{
    #if CONFIG_IS_DEMO
    {
        DWORD dwLaunchDataType;
        DWORD Ret = XGetLaunchInfo( &dwLaunchDataType,&s_ld );
        if( Ret==ERROR_NOT_FOUND )
        {
            __asm int 3 // skipped by Debug|attach-process
            __asm int 3 // proper breakpoint
        }
        switch( dwLaunchDataType )
        {
            case LDT_FROM_DASHBOARD:
                s_pDemoData = NULL;
                break;

            case LDT_TITLE:
                s_pDemoData=( LD_DEMO* )s_ld.Data;
                g_DemoInactiveTimeout = s_pDemoData->dwTimeout/1000;
                break;

            default:
                x_DebugMsg( "Demo must have same title ID as caller\n" );
                ASSERT(0);
                break;
        }
    }
    #endif

    x_Init(0,NULL);

    // set territory
    x_SetTerritory( x_GetConsoleRegion() );

    s.FPSFrameTime[0] = 0;
    s.FPSFrameTime[1] = 0;
    s.FPSFrameTime[2] = 0;
    s.FPSFrameTime[3] = 0;
    s.FPSFrameTime[4] = 0;
    s.FPSFrameTime[5] = 0;
    s.FPSFrameTime[6] = 0;
    s.FPSFrameTime[7] = 0;

    s.FPSLastTime = x_GetTime();
}


//=========================================================================

s32 xbox_ExitPoint( void )
{
//  eng_Kill();

#if 0
    cdfs_Kill();
#endif

    x_Kill();
    return 0;
}

//=============================================================================
static
void UpdateFPS( void )
{
    xtick CurrentTime;

    CurrentTime                     = x_GetTime();
    s.FPSFrameTime[ s.FPSIndex ]    = CurrentTime - s.FPSLastTime;
    s.FPSLastTime                   = CurrentTime;
    s.FPSIndex                     += 1;
    s.FPSIndex                     &= 0x07;
}

//=============================================================================

f32 eng_GetFPS( void )
{
    xtick Sum = s.FPSFrameTime[0] +
                s.FPSFrameTime[1] +
                s.FPSFrameTime[2] +
                s.FPSFrameTime[3] +
                s.FPSFrameTime[4] +
                s.FPSFrameTime[5] +
                s.FPSFrameTime[6] +
                s.FPSFrameTime[7];

    return( (f32)(s32)(((8.0f / x_TicksToMs( Sum )) * 1000.0f) + 0.5f) );
}       

static D3DPRESENT_PARAMETERS s_d3dpp;

//=========================================================================
template<class T>
__forceinline VOID Clamp( T* pVal, T Min, T Max )
{
    if( *pVal < Min )      *pVal = Min;
    else if( *pVal > Max ) *pVal = Max;
}

//=========================================================================

static HRESULT CreateD3DDevice( s32 XRes, s32 YRes, u32 ExtraFlags,u32 FPS )
{
    // Set up the structure used to create the D3DDevice.
    ZeroMemory( &s_d3dpp, sizeof(s_d3dpp) );

    // Select 10x11 aspect
    //if( XRes==640 )
    //{
    //    ExtraFlags|=D3DPRESENTFLAG_10X11PIXELASPECTRATIO;
    //}
    s_d3dpp.Flags|=ExtraFlags;

    // Set fullscreen 640x480x32 mode
    s_d3dpp.BackBufferWidth        = XRes;
    s_d3dpp.BackBufferHeight       = YRes;
    s_d3dpp.BackBufferFormat       = D3DFMT_A8R8G8B8;

    s.rcWindowRect.top    = 0;
    s.rcWindowRect.left   = 0;
    s.rcWindowRect.right  = XRes;
    s.rcWindowRect.bottom = YRes;

    // Set the maximun allow size for a d3d window
    s.MaxXRes = MIN(1600, XRes);
    s.MaxYRes = MIN(1200, YRes);

    // Create one backbuffer and a zbuffer
    s_d3dpp.BackBufferCount                 = 1;
    s_d3dpp.EnableAutoDepthStencil          = FALSE;//TRUE;
  //s_d3dpp.AutoDepthStencilFormat          = D3DFMT_D24S8;

    DWORD Interval;
#if SWITCH_ADAPTIVE_FRAMERATE
    Interval = D3DPRESENT_INTERVAL_IMMEDIATE;
#else
    #if SWITCH_60_FPS
        Interval = D3DPRESENT_INTERVAL_ONE_OR_IMMEDIATE;
    #endif
    #if SWITCH_30_FPS
        Interval = D3DPRESENT_INTERVAL_TWO_OR_IMMEDIATE;
    #endif
#endif
    s_d3dpp.FullScreen_PresentationInterval = Interval;
    s_d3dpp.FullScreen_RefreshRateInHz      = FPS;
    // Set up how the backbuffer is "presented" to the frontbuffer each frame
    s_d3dpp.SwapEffect                      = D3DSWAPEFFECT_FLIP;

    // --------------------------------------------------------------------------

    f32 GapX = f32(XRes);
    f32 GapY = f32(YRes);
    {
        if( ExtraFlags & D3DPRESENTFLAG_PROGRESSIVE )
        {
            GapX = (GapX-GapX*0.85f)/2.0f;
            GapY = (GapY-GapY*0.90f)/2.0f;
        }
        else
        {
            GapX = (GapX-GapX*0.85f)/2.0f;
            GapY = (GapY-GapY*0.85f)/2.0f;
        }
    }
    g_LEdge = u32( GapX );
    g_TEdge = u32( GapY );
    g_REdge = XRes-g_LEdge;
    g_BEdge = YRes-g_TEdge;
    g_PhysW = XRes;
    g_PhysH = YRes;

    // --------------------------------------------------------------------------
    // Create the Direct3D device. Hardware vertex processing is specified 
    // since all vertex processing takes place on Xbox hardware.
    const u32 PushBufferSize = 1048576*4;
    s.pD3D->SetPushBufferSize( PushBufferSize,1048576 );
#if D3DCOMPILE_PUREDEVICE
    return s.pD3D->CreateDevice(
        0,
        D3DDEVTYPE_HAL,
        NULL,
        D3DCREATE_PUREDEVICE,
        &s_d3dpp,
        &g_pd3dInternal
    );
#else
    return s.pD3D->CreateDevice(
        0,
        D3DDEVTYPE_HAL,
        NULL,
        D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &s_d3dpp,
        &g_pd3dInternal
    );
#endif
}

//=============================================================================

namespace xbox
{
    //! Timer object
    /** This class defines a simple timing mechanism.
        */

    struct timer
    {
        //  -------------------------------------------------------------------------
        //                                                 Construction methodologies
        //  -------------------------------------------------------------------------

        /** Default constructor.
            This routine initialises the timer object.
            */

        timer( void )
        {
            reset( );
        }

        /** Destructor.
            This routine kills the timer.
            */

    ~   timer( void )
        {
        }

        //  -------------------------------------------------------------------------
        //                                                          Timing operations
        //  -------------------------------------------------------------------------

        /** Reset timer.
            This routine resets the timing mechanism.
            */

        void reset( void )
        {
            QueryPerformanceCounter((LARGE_INTEGER*)&m_Start );
            m_Stop = m_Start;
        }

        /** Start timer.
            This routine starts a timing session.
            */

        void start( void )
        {
            m_bRunning = 1;
            QueryPerformanceCounter((LARGE_INTEGER*)&m_Start );
        }

        /** Start timer.
            This routine starts a timing session.
            */

        void stop( void )
        {
            m_bRunning = 0;
            QueryPerformanceCounter((LARGE_INTEGER*)&m_Stop );
        }

        //  -------------------------------------------------------------------------
        //                                                Elapsed interval operations
        //  -------------------------------------------------------------------------

        /** Get time.
            Get time elapsed in milliseconds.
            */

        f32 asSec( void )const
        {
            return asMs()/1000;
        }

        /** Get time.
            Get time elapsed in milliseconds.
            */

        f32 asMs( void )const
        {
            u64 Time;
            if( m_bRunning ) 
                QueryPerformanceCounter((LARGE_INTEGER*)&Time );
            else
                Time = m_Stop;
            u64 Freq;
            QueryPerformanceFrequency((LARGE_INTEGER*)&Freq );
            f32 Answ = f32(u32(Time-m_Start))/f32(u32( Freq ))*1000;
            return Answ;
        }

        /** Get time.
            Returns the elapsed time in microseconds.
            */

        f32 asUs( void )const
        {
            return asMs()*1000;
        }

        /** Get time.
            Get time elapsed in milliseconds.
            */

        f32 asNs( void )const
        {
            return asUs( )*1000;
        }

        //  -------------------------------------------------------------------------
        //                                                         Private properties
        //  -------------------------------------------------------------------------

    private:

        //! Running flag
        /** This member keeps track of whether we're running or not.
            */

	    u32 m_bRunning : 1;

        //! Start time
        /** This is the start time in cycles.
            */

        u64 m_Start;

        //! Total time
        /** This is the start time in cycles.
            */

        u64 m_Stop;
    };
}

#if SWITCH_ADAPTIVE_FRAMERATE
static xbox::timer s_SmoothTimer;
#endif

//=========================================================================

static f32 s_BrightnessMax      = 1.4750000f;
static f32 s_BrightnessMin      = 0.5500001f;

 const f32 DEFAULT_BRIGHTNESS   = 1.115f; // DO NOT CHANGE THIS NUMBER LEST I HURT YOU!
static f32 s_Brightness         = DEFAULT_BRIGHTNESS;
static f32 s_Contrast           = 2.068f;// PS2 = 20.68f

void xbox_SetBrightness( f32 Brightness )
{
    ASSERT( (Brightness >= 0.0f) && (Brightness <= 1.0f) );

    f32 T = 1-Brightness;

    // Another way of doing a lerp
    s_Brightness = s_BrightnessMin + T * (s_BrightnessMax - s_BrightnessMin);
}

//=========================================================================

inline f32 xbox_ComputeBrightness( f32 NativeBrightness )
{
    return 1.0f-( (NativeBrightness - s_BrightnessMin)/(s_BrightnessMax-s_BrightnessMin) );
}

//=========================================================================

f32 xbox_GetBrightness( void )
{
    return xbox_ComputeBrightness( s_Brightness );
}

//=========================================================================

f32 xbox_GetDefaultBrightness( void )
{
    return xbox_ComputeBrightness( DEFAULT_BRIGHTNESS );
}

//=========================================================================

xbool g_BlackOut = FALSE;

void SetGamma( void )
{
    D3DGAMMARAMP Ramp;
    if( g_BlackOut )
        x_memset( &Ramp,0,sizeof(Ramp) );
    else
    {
        f32 Gamma;
        for( u32 i=0;i<256;i++ )
        {
            Gamma = x_pow( f32(i)/255.0f,s_Brightness )*255.0f;
            Gamma = x_max( 0.0f,Gamma * 1.0811f - s_Contrast );
            Gamma = Gamma/255.0f;

            f32 r = x_min( 1.0f,x_max( 0.0f,Gamma ));
            f32 g = x_min( 1.0f,x_max( 0.0f,Gamma ));
            f32 b = x_min( 1.0f,x_max( 0.0f,Gamma ));

            Ramp.green[i]= BYTE( g*255.0f );
            Ramp.blue [i]= BYTE( b*255.0f );
            Ramp.red  [i]= BYTE( r*255.0f );
        }
    }
    D3DDevice::SetGammaRamp( 0,&Ramp );
}

//=========================================================================
//
//  NOTE: Don't try and do any floating point stuff here.
//  NOTE: This is a DPC (Deferred Procedural Call)--a funky interrupt.
//
#if SWITCH_ADAPTIVE_FRAMERATE
static volatile xbool s_bWaitingForVBlank=FALSE;
static volatile HANDLE s_hVBlankEvent=NULL;
static volatile s32 s_VBlankCount=0;

static void VBlankCallback( D3DVBLANKDATA *pData )
{
    if( ++ s_VBlankCount >= s_FrameRateLock && s_bWaitingForVBlank )
    {
        SetEvent( s_hVBlankEvent );
        s_bWaitingForVBlank=FALSE;
        s_VBlankCount=0;
    }
}
#endif

//=============================================================================

static bool s_b60FPS = false;

void xbox_Set60FPS( bool b60FPS )
{
    s_b60FPS = b60FPS;
}

//=============================================================================

int eng_GetProductCode(void)
{
    return 0;
}

//=============================================================================

const char* eng_GetProductKey(void)
{
    return NULL;
}

//=============================================================================

static f32 s_TimeTaken = 0.0f;

f32 eng_GetTimings( void )
{
    return s_TimeTaken;
}

//=============================================================================

void eng_SyncVBlank( void )
{
    #if SWITCH_ADAPTIVE_FRAMERATE
    {
        f32 TimeTaken = s_SmoothTimer.asMs();
        {
            if( s_b60FPS )
            {
                f32 Skipped = 1.0f-TimeTaken/s_MsPerFrame;
                if( Skipped < 0.0f )
                    s_FrameRateLock = 0;
                else
                    s_FrameRateLock = 1;
                s_TimeTaken = TimeTaken;
            }
            else
            {
                f32 Skipped = 2.0f-TimeTaken/s_MsPerFrame;
                if( Skipped < 0.0f )
                    s_FrameRateLock = 0;
                else
                    s_FrameRateLock = 2;
                s_TimeTaken = TimeTaken;
            }
        }

        if( s_FrameRateLock )
        {
            s_bWaitingForVBlank= TRUE;
            WaitForSingleObject( s_hVBlankEvent,INFINITE );
        }
        s_SmoothTimer.stop ();
        s_SmoothTimer.start();
    }
    #endif
}

//=============================================================================

void(*g_VSyncCB)(void)=NULL;

void PresentFrame( void )
{
    if( g_bPresentPending )
    {
        /* smooth frame rate */

        g_pd3dDevice->BlockUntilIdle();
        if( g_VSyncCB )
            g_VSyncCB();
        eng_SyncVBlank();
        SetGamma();

        /* must wait for GPU */

        x_BeginAtomic();
        {
            g_pd3dInternal->Present( 0,0,0,0 );
            g_bPresentPending = FALSE;
            if( g_bClearingViewport )
            {
                D3DVIEWPORT8 Viewport={ 0,0,s.MaxXRes,s.MaxYRes,0.0f,1.0f };
                g_pd3dInternal->SetViewport( &Viewport );
                g_pd3dInternal->Clear(
                    0,
                    0,
                    D3DCLEAR_TARGET
                        |   D3DCLEAR_STENCIL
                        |   D3DCLEAR_ZBUFFER,
                    s.BackColor,
                    1.0f,
                    0
                );
            }
        }
        x_EndAtomic();
    }
}

//=============================================================================

xbool eng_Begin( const char* pTaskName )
{
    static const char* pLastTask = NULL;

    eng_BeginCount++;

    ASSERT( s.bBeginScene == FALSE );

    pLastTask = pTaskName;

    // this is saying to present the frame: page flip (really)

    if( g_bPresentPending && g_bFlippable )
    {
        if( s.bD3DBeginScene )
        {
            g_pd3dInternal->EndScene();
            s.bD3DBeginScene = FALSE;
        }
        PresentFrame();
    }

    if( s.bD3DBeginScene == FALSE )
    {
        // Begin the scene
        if( FAILED( g_pd3dInternal->BeginScene() ) ) 
            return FALSE;

        // Mark the d3dbeginscene as active
        s.bD3DBeginScene = TRUE;
    }

    s.bBeginScene = TRUE;

    // Clear draw's L2W
    draw_ClearL2W();

    return TRUE;
}

//=========================================================================
struct HSL{ f32 h,s,l; };
struct RGB{ f32 r,g,b; };
/*
   Calculate HSL from RGB
   Hue is in degrees
   Lightness is between 0 and 1
   Saturation is between 0 and 1
*/
HSL RGBtoHSL(RGB& c1)
{
   f32 themin,themax,delta;
   HSL c2;

   themin = MIN(c1.r,MIN(c1.g,c1.b));
   themax = MAX(c1.r,MAX(c1.g,c1.b));
   delta = themax - themin;
   c2.l = (themin + themax) / 2;
   c2.s = 0;
   if (c2.l > 0 && c2.l < 1)
      c2.s = delta / (c2.l < 0.5f ? (2*c2.l) : (2-2*c2.l));
   c2.h = 0;
   if (delta > 0) {
      if (themax == c1.r && themax != c1.g)
         c2.h += (c1.g - c1.b) / delta;
      if (themax == c1.g && themax != c1.b)
         c2.h += (2 + (c1.b - c1.r) / delta);
      if (themax == c1.b && themax != c1.r)
         c2.h += (4 + (c1.r - c1.g) / delta);
      c2.h *= 60;
   }
   return(c2);
}

//=========================================================================
/*
   Calculate RGB from HSL, reverse of RGB2HSL()
   Hue is in degrees
   Lightness is between 0 and 1
   Saturation is between 0 and 1
*/
RGB HSLtoRGB(HSL& c1)
{
    RGB c2,sat,ctmp;

    while (c1.h < 0)
        c1.h += 360;
    while (c1.h > 360)
        c1.h -= 360;

    if (c1.h < 120)
    {
        sat.r = (120 - c1.h) / 60.0f;
        sat.g = c1.h / 60.0f;
        sat.b = 0;
    }
    else if (c1.h < 240)
    {
        sat.r = 0;
        sat.g = (240 - c1.h) / 60.0f;
        sat.b = (c1.h - 120) / 60.0f;
    }
    else
    {
        sat.r = (c1.h - 240) / 60.0f;
        sat.g = 0;
        sat.b = (360 - c1.h) / 60.0f;
    }
    sat.r = MIN(sat.r,1);
    sat.g = MIN(sat.g,1);
    sat.b = MIN(sat.b,1);

    ctmp.r = 2 * c1.s * sat.r + (1 - c1.s);
    ctmp.g = 2 * c1.s * sat.g + (1 - c1.s);
    ctmp.b = 2 * c1.s * sat.b + (1 - c1.s);

    if (c1.l < 0.5f)
    {
        c2.r = c1.l * ctmp.r;
        c2.g = c1.l * ctmp.g;
        c2.b = c1.l * ctmp.b;
    }
    else
    {
        c2.r = (1 - c1.l) * ctmp.r + 2 * c1.l - 1;
        c2.g = (1 - c1.l) * ctmp.g + 2 * c1.l - 1;
        c2.b = (1 - c1.l) * ctmp.b + 2 * c1.l - 1;
   }

   return(c2);
}

//=========================================================================

struct io_buffer
{
    s32             DataSize;
    s32             ClutSize;
    s32             Width;
    s32             Height;
    s32             PW;
    u32             Flags;
    s32             NMips;
    xbitmap::format Format;
};

//=========================================================================

static void DrawIoBuffer( io_buffer* pBuffer )
{
    // Draw xbitmap to screen .................................................

    IDirect3DSurface8* pBack;
    if( !g_pd3dDevice->GetBackBuffer( 0,D3DBACKBUFFER_TYPE_MONO,&pBack ))
    {
        D3DLOCKED_RECT LockedRect;
        pBack->LockRect( &LockedRect,NULL,D3DLOCK_TILED );

        u8* pDstBits = (u8*)LockedRect.pBits;
        u8* pSrcBits = (u8*)(pBuffer+1);

        s32 Ex = (g_PhysW-pBuffer->Width )/2;
        s32 Ey = (g_PhysH-pBuffer->Height)/2;

        // Byte swapping
        for( u32 y=Ey;y<g_PhysH-Ey;y++ )
        {
            for( u32 x=Ex;x<g_PhysW-Ex;x++ )
            {
                u8* pDst = (pDstBits+y*LockedRect.Pitch)+(x*4);
                {
                    // Pixel = RGBA
                    f32 b = f32(pSrcBits[0])/255.0f;
                    f32 g = f32(pSrcBits[1])/255.0f;
                    f32 r = f32(pSrcBits[2])/255.0f;
                    f32 a = f32(pSrcBits[3])/255.0f;

                    if(0)
                    {
                        RGB rgb;
                        rgb.r = r;
                        rgb.g = g;
                        rgb.b = b;

                        HSL hsl = RGBtoHSL( rgb );

                        static f32 Tweak = 1.115f;

                        hsl.s = hsl.s*Tweak;
                        hsl.s = x_min( 1.0f,x_max( 0.0f,hsl.s ));   
                        hsl.l = ((hsl.l*255.0f)*1.0811f-20.68f)/255.0f;
                        hsl.l = x_min( 1.0f,x_max( 0.0f,hsl.l ));

                        rgb = HSLtoRGB( hsl );
                        r = rgb.r;
                        g = rgb.g;
                        b = rgb.b;
                    }

                    // BGRA = RGBA
                    pDst[0]=u8(b*255.0f);
                    pDst[1]=u8(g*255.0f);
                    pDst[2]=u8(r*255.0f);
                    pDst[3]=u8(a*255.0f);
                }
                pSrcBits += 4;
            }
        }

        pBack->UnlockRect( );
        pBack->Release( );

        SetGamma();

    __asm wbinvd

        g_pd3dInternal->Present( 0,0,0,0 );
    }
}

//=========================================================================

void xbox_DrawXbmpFromSection( const char* pName )
{
    g_pd3dInternal->Clear( 0,NULL,D3DCLEAR_TARGET,0,0.0f,0 );

    // Load section data ......................................................

    xstring SectionName;
    SectionName.Format( "%s_%dx%d",pName,g_PhysW,g_PhysH );
    io_buffer* pBuffer = (io_buffer*)XLoadSection( SectionName );
    ASSERT( pBuffer );
    if( pBuffer )
    {
        DrawIoBuffer( pBuffer );
        XFreeSection( SectionName );
    }
}

//=========================================================================

static
void InitializeD3D( s32 XRes, s32 YRes )
{
    // Create the D3D object, which is used to create the D3DDevice.
    s.pD3D = Direct3DCreate8( D3D_SDK_VERSION );
    ASSERT( s.pD3D );

    // --------------------------------------------------------------------------

    // Process video flags

    u32 dwVideoFlags = XGetVideoFlags( );
    switch( XGetVideoStandard( ))
    {
        case XC_VIDEO_STANDARD_NTSC_M:
            g_PhysFPS = 60;
            g_bPAL = FALSE;
            break;

        case XC_VIDEO_STANDARD_NTSC_J:
            g_PhysFPS = 60;
            g_bPAL = FALSE;
            break;

        case XC_VIDEO_STANDARD_PAL_I:
            if( dwVideoFlags & XC_VIDEO_FLAGS_PAL_60Hz )
                g_PhysFPS = 60;
            else
                g_PhysFPS = 50;
            g_bPAL = TRUE;
            break;

        default:
            ASSERT(0);
    }

    // --------------------------------------------------------------------------

    #define ALLOW_HD_1080i 0
    #define ALLOW_HD_720p  0
    #define ALLOW_HD_480p  1

    // setup 1080i ************************************************************

    u32 ExtraFlags = 0;

    #if ALLOW_HD_1080i
    if( dwVideoFlags & XC_VIDEO_FLAGS_HDTV_1080i )
    {
        ExtraFlags = D3DPRESENTFLAG_INTERLACED | D3DPRESENTFLAG_WIDESCREEN;
        XRes = 1920;
        YRes = 1080;
    }
    else
    #endif

    // setup 720p *************************************************************

    #if ALLOW_HD_720p
    if( dwVideoFlags & XC_VIDEO_FLAGS_HDTV_720p )
    {
        ExtraFlags = D3DPRESENTFLAG_PROGRESSIVE | D3DPRESENTFLAG_WIDESCREEN;
        g_b720P = TRUE;
        XRes = 1280;
        YRes = 720;
    }
    else
    #endif

    // setup 480p *************************************************************

    #if ALLOW_HD_480p
    if( dwVideoFlags & XC_VIDEO_FLAGS_HDTV_480p )
    {
        ExtraFlags = D3DPRESENTFLAG_PROGRESSIVE;
        g_b480P = TRUE;
        XRes = 640;//720;
        YRes = 480;
    }
    #endif

    // --------------------------------------------------------------------------

    if( CreateD3DDevice( XRes,YRes,ExtraFlags,g_PhysFPS ))
    {
        __asm int 3
        __asm int 3
    }

    s_MsPerFrame = 1000.0f/g_PhysFPS;

    // --------------------------------------------------------------------------
    // Activate all sub-allocators - damned push buffers!

    const u32 Sizes[5]=
    {
        (u32( 1048576.0f*12.00f+0.5f )+(D3DTILE_ALIGNMENT      -1)) & ~(D3DTILE_ALIGNMENT      -1), // Texture buffer quota
        (u32( 1048576.0f* 5.26f+0.5f )+(D3DPUSHBUFFER_ALIGNMENT-1)) & ~(D3DPUSHBUFFER_ALIGNMENT-1), // Tiled memory used by renderer
        (u32( 1048576.0f* 0.01f+0.5f )+(D3DPUSHBUFFER_ALIGNMENT-1)) & ~(D3DPUSHBUFFER_ALIGNMENT-1), // Recording quota
        (u32( 1048576.0f* 2.00f+0.5f )+(DEFAULT_ALIGNMENT      -1)) & ~(DEFAULT_ALIGNMENT      -1), // Push buffers
        (u32( 1048576.0f* 6.00f+0.5f )+(DEFAULT_ALIGNMENT      -1)) & ~(DEFAULT_ALIGNMENT      -1), // Vertex  buffer quota
    };

    const heap::basic* Heaps[5]=
    {
        &texture_factory::m_Allocator[kPOOL_GENERAL], // Texture buffer quota
        &texture_factory::m_Allocator[kPOOL_TILED  ], // Tiled memory used by renderer
        &push_factory   ::m_Allocator[kPOOL_RECORD ], // Recording quota
        &push_factory   ::m_Allocator[kPOOL_PUSH   ], // Push buffers
        &vert_factory   ::m_Allocator                 // Vertex  buffer quota
    };

    const u32 Protections[5]=
    {
        PAGE_READWRITE | PAGE_WRITECOMBINE | PAGE_NOCACHE, // Texture buffer quota
        PAGE_READWRITE | PAGE_WRITECOMBINE | PAGE_NOCACHE, // Tiled memory used by renderer
        PAGE_READWRITE | PAGE_WRITECOMBINE               , // Recording quota
        PAGE_READWRITE | PAGE_WRITECOMBINE               , // Push buffers
        PAGE_READWRITE | PAGE_WRITECOMBINE | PAGE_NOCACHE, // Vertex  buffer quota
    };

    const u32 Alignments[5]=
    {
        D3DTEXTURE_ALIGNMENT   , // Texture buffer quota
        D3DTILE_ALIGNMENT      , // Tiled memory used by renderer
        D3DPUSHBUFFER_ALIGNMENT, // Recording quota
        D3DPUSHBUFFER_ALIGNMENT, // Push buffers
        DEFAULT_ALIGNMENT      , // Vertex  buffer quota
    };

    u32 Length = 0;
    for( s32 i = 0; i < 5; i++ )
    {
        Length += Sizes[i];
    }
    u8* Ptr = (u8*)XPhysicalAlloc(
        Length,
        MAXULONG_PTR,
        D3DTEXTURE_ALIGNMENT,
        PAGE_READWRITE
    );
    for( s32 i=0; i < 5; i++ )
    {
        Heaps[i]->heap::basic::basic( Ptr,Alignments[i],Sizes[i] );
        XPhysicalProtect(
            (void*)Heaps[i]->GetBase(),
            (DWORD)Heaps[i]->GetSize(),
            Protections [i]
        );
    }

    // --------------------------------------------------------------------

    g_TextureFactory.Init();
    g_PushFactory   .Init();
    g_VertFactory   .Init();

    // --------------------------------------------------------------------

    extern void xbox_InitShaders( void );
    xbox_InitShaders();
}

//=========================================================================

void ENG_TextColor( const xcolor& Color )
{
    s.TextColor = Color;
}

//=============================================================================

void ENG_SetLight( s32 LightID, const vector3& Dir, const xcolor& Color )
{
    D3DLIGHT8 Light;

    //
    // Here we fill up the structure for D3D. The first thing we say 
    // is the type of light that we want. In this case DIRECTIONAL. 
    // Which basically means that it doesn't have an origin. The 
    // second thing that we fill is the diffuse lighting. Basically 
    // is the color of the light. Finally we fill up the Direction. 
    // Note how we negate to compensate for the D3D way of thinking.
    //

    ZeroMemory( &Light, sizeof(Light) );   // Clear the hold structure
    Light.Type          = D3DLIGHT_DIRECTIONAL;

    Light.Diffuse.r   = (Color>>16)*(1/255.0f); 
    Light.Diffuse.g   = (Color>> 8)*(1/255.0f);
    Light.Diffuse.b   = (Color>> 0)*(1/255.0f);
    Light.Diffuse.a   = (Color>>24)*(1/255.0f);

    Light.Specular    = Light.Diffuse;

    Light.Direction.x = -Dir.GetX();   // Set the direction of
    Light.Direction.y = -Dir.GetY();   // the light and compensate
    Light.Direction.z = -Dir.GetZ();   // for DX way of thinking.

    //
    // Here we set the light number zero to be the light which we just
    // describe. What is light 0? Light 0 is one of the register that 
    // D3D have for lighting. You can overwrite registers at any time. 
    // Only lights that are set in registers are use in the rendering 
    // of the scene.
    //
    g_pd3dDevice->SetLight( LightID, &Light );

    //
    // Here we enable out register 0. That way what ever we render 
    // from now on it will use register 0. The other registers are by 
    // default turn off.
    //
    g_pd3dDevice->LightEnable( 0, TRUE );
}

//=============================================================================

void ENG_SetAmbientLight( const xcolor& Color )
{
    D3DMATERIAL8 mtrl;

    //
    // What we do here is to create a material that will be use for 
    // all the render objects. why we need to do this? We need to do 
    // this to describe to D3D how we want the light to reflected 
    // from our objects. Here you can fin more info about materials. 
    // We set the color base of the material in this case just white. 
    // Then we set the contribution for the ambient lighting, in this 
    // case 0.3f. 
    //
    ZeroMemory( &mtrl, sizeof(mtrl) );

    // Color of the material
    mtrl.Diffuse.r  = mtrl.Diffuse.g = mtrl.Diffuse.b   = 1.0f; 
    mtrl.Specular.r = mtrl.Specular.g = mtrl.Specular.b = 0.5f;
    mtrl.Power      = 50;


    // ambient light
    mtrl.Ambient.r   = (Color>>16)*(1/255.0f); 
    mtrl.Ambient.g   = (Color>> 8)*(1/255.0f);
    mtrl.Ambient.b   = (Color>> 0)*(1/255.0f);
    mtrl.Ambient.a   = (Color>>24)*(1/255.0f);

    //
    // Finally we activate the material
    //
    g_pd3dDevice->SetMaterial( &mtrl );

    //
    // This function will set the ambient color. In this case white.
    // R=G=B=A=255. which is like saying 0xffffffff. Because the color
    // is describe in 32bits. One each of the bytes in those 32bits
    // describe a color component. You can also use a macro provided 
    // by d3d to build the color.
    //
    g_RenderState.Set( D3DRS_AMBIENT, Color );
}

//=========================================================================

void xbox_EngInit( s32 maxXRes, s32 maxYRes, s32 XRes, s32 YRes )
{
    // Initialize the engine
    InitializeD3D( maxXRes, maxYRes);

    // Clear the background (first frame after reboot isn't cleared)
    g_pd3dInternal->Clear( 0, NULL, D3DCLEAR_STENCIL | D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0L ); 

    #if SWITCH_ADAPTIVE_FRAMERATE
    {
        s_VBlankCount=0;
        s_bWaitingForVBlank=FALSE;
        s_hVBlankEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
        g_pd3dInternal->SetVerticalBlankCallback( VBlankCallback );
    }
    #endif

    // Initialize imput
    input_Init();

    // Initialize vram
    vram_Init();

    // Set the scrach memory
    smem_Init( SCRATCH_MEM_SIZE);

    // Initialize draw
    draw_Init();

    // FONT
    font_Init();

    // Init text
    text_Init();
    text_SetParams( maxXRes, maxYRes, 0, 0, ENG_FONT_SIZEX, ENG_FONT_SIZEY, 8 );
    text_ClearBuffers();

    // Indicate the engine is ready
    s.bReady = TRUE;

    // Change CPU precision
    //_controlfp(_PC_24,_MCW_PC);

    // Occlusion culling
    g_RenderState.Set( D3DRS_OCCLUSIONCULLENABLE, FALSE );
    // Make sure that we are dealing with the Z buffer
    g_RenderState.Set( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
    g_RenderState.Set( D3DRS_ZENABLE, D3DZB_TRUE );

    // Set some color conbiner functions
    g_RenderState.Set( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );

    // Right handed mode
    g_RenderState.Set( D3DRS_CULLMODE, D3DCULL_CW );

    // We can do alpha from the begining
    g_RenderState.Set( D3DRS_ALPHABLENDENABLE, TRUE );
    g_RenderState.Set( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    g_RenderState.Set( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

    // Turn off the stupid specular lighting
    g_RenderState.Set( D3DRS_SPECULARENABLE, FALSE );

    // Set bilinear mode
    g_TextureStageState.Set( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR  );
    g_TextureStageState.Set( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR  );

    // Turn on the trilinear blending
    g_TextureStageState.Set( 0, D3DTSS_MIPFILTER, D3DTEXF_LINEAR  );

    // Set the Initial wraping behavier
    g_TextureStageState.Set( 0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP );
    g_TextureStageState.Set( 0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP );

    // Set the ambient light
    ENG_SetAmbientLight( 0xfff8f8f8 );

    // Make sure that we turn off the lighting as the default
    g_RenderState.Set( D3DRS_LIGHTING, FALSE );
    
    // Start pageflip timer
    s.CPUTIMER.Start();
}

void eng_Init( )
{
    #if defined X_DEBUG && defined bhapgood && 0
    D3D__SingleStepPusher=1;
    DmEnableGPUCounter(FALSE);
    D3DPERF_SetShowFrameRateInterval(5000);
    #endif

    xbox_EngInit( 640, 480, 640, 480 );

    static view s_View;
    {
        eng_MaximizeViewport( s_View );
        eng_SetView         ( s_View );
    }
}

//=========================================================================

void eng_Kill( void )
{
    //
    // Free sub modules
    //
    draw_Kill();
    vram_Kill();
    smem_Kill();
    text_Kill();

    #if SWITCH_ADAPTIVE_FRAMERATE
    g_pd3dInternal->SetVerticalBlankCallback( NULL );
    CloseHandle( s_hVBlankEvent );
    #endif

    //
    // Stop the d3d engine
    //
    if( g_pd3dInternal != NULL) 
        g_pd3dInternal->Release();

    if( s.pD3D != NULL)
        s.pD3D->Release();
}

//=============================================================================

xbool eng_InBeginEnd( void )
{
    return s.bBeginScene;
}

//=============================================================================

void eng_End( void )
{
    ASSERT( s.bBeginScene );
    s.bBeginScene = FALSE;

    g_pd3dDevice->KickPushBuffer();

    #ifndef X_RETAIL
    D3DPERF_EndEvent( );
    #endif
}

//=============================================================================

void eng_PrintStats( void )
{
    x_DebugMsg("CPU:%1.1f  Pageflip:%1.1f  FPS:%1.1f\n",s.CPUMS,s.IMS,1000.0f/(s.CPUMS+s.IMS));
}

//=============================================================================

void eng_PageFlip()
{
    CONTEXT( "PageFlip" );

    if( !eng_BeginCount )
        g_pd3dDevice->Clear( 0,0,D3DCLEAR_TARGET,0,0.0f,0 );
    eng_BeginCount = 0;

    xtimer ARHTimer;
    ARHTimer.Reset();
    s.CPUMS = s.CPUTIMER.ReadMs();
    xtimer InternalTime;
    InternalTime.Start();

    const xbool bPrintFPS = TRUE;

    #ifndef X_RETAIL
    if( s_ShowSafeArea )
    {
        eng_Begin( "Draw safe area" );
            irect rc;
            rc.Set( g_LEdge,g_TEdge,g_REdge,g_BEdge );
            draw_Rect( rc,xcolor( 127,127,127,255 ),TRUE );
        eng_End();
    }
    #endif

    //-------------------------------------------------------------------------
    //
    // Handle all the buffered text
    //
    rst.Reset();
    rstct=0;
    ARHTimer.Start();
    text_Render();
    ARHTimer.Stop();
    text_ClearBuffers();

    //-------------------------------------------------------------------------
    //
    //  Lock frame rate to 60fps (or at least try to)
    //
    #if SWITCH_USE_DEFERRED_RENDERING
    PresentFrame();
    #else
    g_bPresentPending = TRUE;
    #endif

    //-------------------------------------------------------------------------
    //
    //  Toggle the scrach memory
    //
    {   CONTEXT( "smem_Toggle" );
        smem_Toggle();
    }


    UpdateFPS();

    InternalTime.Stop();
    s.IMS = InternalTime.ReadMs();

    s.CPUTIMER.Reset();
    s.CPUTIMER.Start();
}


//=============================================================================

void eng_GetRes( s32& XRes, s32& YRes )
{
    XRes = s.rcWindowRect.right  - s.rcWindowRect.left;
    YRes = s.rcWindowRect.bottom - s.rcWindowRect.top;
}

//=============================================================================

void eng_MaximizeViewport( view& View )
{
    s32 XRes, YRes;
    eng_GetRes( XRes, YRes );

    s32 Width  = MIN( s.MaxXRes, s.rcWindowRect.right-s.rcWindowRect.left );
    s32 Height = MIN( s.MaxYRes, s.rcWindowRect.bottom-s.rcWindowRect.top );

    View.SetViewport( 0, 0, Width, Height );
}

//=============================================================================

void eng_SetBackColor( xcolor Color )
{
    s.BackColor = Color;
}

//=============================================================================

void eng_SetView ( const view& View )
{
    x_BeginAtomic();
    {
        s.View = View;

        if( g_pd3dDevice )
        {
            matrix4 v = s.View.GetW2V();
            g_pd3dDevice->SetTransform( D3DTS_VIEW,(D3DMATRIX*)&v );

            matrix4 p = s.View.GetV2C();
            g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&p );
        }
    }
    x_EndAtomic();
}

//=============================================================================

const view* eng_GetView( void )
{
    return &s.View;
}

//=============================================================================

void eng_ScreenShot( const char* pFileName, s32 Size )
{
#ifdef X_DEBUG
#if 0
    {
    u32 i=0;
    char pString[1024];
    for(;;)
    {
        x_sprintf( pString,"ScreenShot%d.bmp",i++ );
        if( DmScreenShot( pString ) != XBDM_NOERR )
            continue;
        break;
    }
#else
    static s32 NShots=0;
    char    AutoName[64];
    s32     XRes = s.MaxXRes;
    s32     YRes = s.MaxYRes;

    // Decide on a file name
    if( pFileName == NULL )
    {
        NShots++;
        x_strcpy(AutoName,xfs("PCScreenshot%03d.tga",NShots));
        pFileName = AutoName;
    }

    IDirect3DSurface8*      pBackBuffer = NULL;
    IDirect3DTexture8*      pBlitTarget = NULL;
    IDirect3DSurface8*      pDestSurf   = NULL;

    HRESULT Res = g_pd3dDevice->GetBackBuffer( 0,D3DBACKBUFFER_TYPE_MONO,&pBackBuffer );

    Res = g_pd3dDevice->CreateTexture( XRes, YRes, 1,
                                 0,
                                 D3DFMT_X8R8G8B8,
                                 D3DPOOL_MANAGED,
                                 &pBlitTarget );

    Res = pBlitTarget->GetSurfaceLevel( 0, &pDestSurf );

    Res = g_pd3dDevice->CopyRects( pBackBuffer, NULL, 0, pDestSurf, NULL );

    Res = pBackBuffer->Release();
    Res = pDestSurf->Release();

    D3DLOCKED_RECT      LockedInfo;
    
    HRESULT Result = pBlitTarget->LockRect( 0, &LockedInfo, NULL, 0 );

    byte* pData = new byte[XRes*YRes*4];
    ASSERT(pData);

    x_memcpy(pData, LockedInfo.pBits, XRes*YRes*4 );

    pBlitTarget->UnlockRect( 0 );

    xbitmap TempO;
    TempO.Setup( xbitmap::FMT_32_ARGB_8888, XRes, YRes, FALSE, pData );
    TempO.SaveTGA( pFileName );
    TempO.Kill();
    
    delete[] pData;
#endif
#endif
}

//=========================================================================

#ifndef CONFIG_RETAIL
s32 eng_ScreenShotActive( void )
{
    return 0 ;
}
#endif

//=========================================================================

s32 eng_ScreenShotSize( void )
{
    return 0;
}

//=========================================================================

s32 eng_ScreenShotX( void )
{
    return 0;
}

//=========================================================================

s32 eng_ScreenShotY( void )
{
    return 0;
}

//=============================================================================

void eng_Sync ( void )
{

}

//=============================================================================

void DebugMessage( const char* FormatStr, ... )
{
#ifndef X_RETAIL
    va_list Args; 
    va_start( Args, FormatStr );

    OutputDebugString( xvfs( FormatStr, Args) );

    va_end( Args );
#endif
}

//=============================================================================

static xbool s_bScaleActive = FALSE;
static f32   s_ScaleW;
static f32   s_ScaleH;

void eng_SetViewport( const view& View )
{
    HRESULT         Error;
    D3DVIEWPORT8    vp;

    s32 L,T,R,B;
    if( s_bScaleActive )
    {
        rect Rect;
        View.GetViewport( Rect );

        L = Rect.Min.X ? s32((f32(Rect.Min.X)/f32(s.MaxXRes))*s_ScaleW) : 0;
        T = Rect.Min.Y ? s32((f32(Rect.Min.Y)/f32(s.MaxYRes))*s_ScaleH) : 0;
        R = Rect.Max.X ? s32((f32(Rect.Max.X)/f32(s.MaxXRes))*s_ScaleW) : 0;
        B = Rect.Max.Y ? s32((f32(Rect.Max.Y)/f32(s.MaxYRes))*s_ScaleH) : 0;
    }
    else
    {
        View.GetViewport( L,T,R,B );
    }

    if( s.bD3DBeginScene )
    {
        VERIFY( !g_pd3dDevice->EndScene( ));
        VERIFY( !g_pd3dDevice->BeginScene( ));
    }

    vp.X      = L;
    vp.Y      = T;
    vp.Width  = R-L;
    vp.Height = B-T;

    vp.MinZ   = 0.0f;
    vp.MaxZ   = 1.0f;

    vp.Width  = MIN( (u32)s.MaxXRes, vp.Width );
    vp.Height = MIN( (u32)s.MaxYRes, vp.Height );

    Error = g_pd3dDevice->SetViewport( &vp );
    ASSERT( Error == D3D_OK );
}

//=============================================================================

void eng_ResetViewport( void )
{
    HRESULT         Error;
    D3DVIEWPORT8    vp;

    if( s.bD3DBeginScene )
    {
        VERIFY( !g_pd3dDevice->EndScene( ));
        VERIFY( !g_pd3dDevice->BeginScene( ));
    }

    vp.X      = 0;
    vp.Y      = 0;
    vp.Width  = s.rcWindowRect.right -s.rcWindowRect.left;
    vp.Height = s.rcWindowRect.bottom-s.rcWindowRect.top ;

    vp.MinZ   = 0.0f;
    vp.MaxZ   = 1.0f;

    vp.Width  = MIN( (u32)s.MaxXRes, vp.Width );
    vp.Height = MIN( (u32)s.MaxYRes, vp.Height );

    Error = g_pd3dDevice->SetViewport( &vp );
    ASSERT( Error == D3D_OK );
}

//=============================================================================

void ScaleRGBA( byte* pSrc, s32 wS, s32 hS, rect& Rect, byte* pDst, s32 wD, s32 hD )
{
    f32 dX = (f32)Rect.GetWidth()  / (f32)wD;
    f32 dY = (f32)Rect.GetHeight() / (f32)hD;

    s32 x,y;
    f32 X,Y;

    byte* pData = pDst;

    for (y=0,Y=0;y<hD;y++,Y+=dY)
    {
        for (x=0,X=0;x<wD;x++,X+=dX)
        {
            s32 sX,sY;
            sX = (s32)X;
            sY = (s32)Y;

            s32 Idx = sY * wS * 4 + sX * 4;

            *pData++ = pSrc[ Idx + 0 ];
            *pData++ = pSrc[ Idx + 1 ];
            *pData++ = pSrc[ Idx + 2 ];
            *pData++ = pSrc[ Idx + 3 ];
        }
    }
}

//=============================================================================

void xbox_BuildXBitmapFromScreen( xbitmap& Dst, s32 W, s32 H, rect& Rect )
{
    s32     XRes = s.MaxXRes;
    s32     YRes = s.MaxYRes;

    IDirect3DSurface8* pBackBuffer = NULL;
    IDirect3DTexture8* pBlitTarget = NULL;
    IDirect3DSurface8* pDestSurf   = NULL;

    g_pd3dDevice->GetBackBuffer( 0,D3DBACKBUFFER_TYPE_MONO,&pBackBuffer );

    g_pd3dDevice->CreateTexture( XRes, YRes, 1,
                                 0,
                                 D3DFMT_X8R8G8B8,
                                 D3DPOOL_MANAGED,
                                 &pBlitTarget );

    pBlitTarget->GetSurfaceLevel( 0, &pDestSurf );

    g_pd3dDevice->CopyRects( pBackBuffer, NULL, 0, pDestSurf, NULL );

    pBackBuffer->Release();
    pDestSurf->Release();

    D3DLOCKED_RECT      LockedInfo;
    
    HRESULT Result = pBlitTarget->LockRect( 0, &LockedInfo, NULL, 0 );

    byte* pData = new byte[XRes*YRes*4];
    ASSERT(pData);

    x_memcpy(pData, LockedInfo.pBits, XRes*YRes*4 );

    pBlitTarget->UnlockRect( 0 );


    byte*   pDestData = new byte[W*H*4];
    ASSERT(pDestData);

    ScaleRGBA( pData, XRes, YRes, Rect, pDestData, W, H );

    delete[] pData;

    xbitmap TempO;
    Dst.Setup( xbitmap::FMT_32_ARGB_8888, W, H, TRUE, pDestData );
    Dst.SaveTGA( "ResizedScreenie.tga" );
}

//-----------------------------------------------------

volatile s32 s_PausePageFlip = 0;

//-----------------------------------------------------

void eng_PageFlipPause( void )
{
    s_PausePageFlip++;
}

//-----------------------------------------------------

void eng_PageFlipResume( void )
{
    s_PausePageFlip--;    
}

//-----------------------------------------------------
xbool eng_IsPaused(void)
{
    return (s_PausePageFlip > 0);
}

//-----------------------------------------------------
#ifndef X_RETAIL
void eng_ShowSafeArea( xbool Param )
{
    s_ShowSafeArea = Param;
}
#endif















//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------

#define CACHE_PACKET_SIZE 32768
#define CACHE_READ_AHEAD 16

static byte Buffer[2][CACHE_PACKET_SIZE];
static xmesgq Pending(CACHE_READ_AHEAD);
static xmesgq Ready  (CACHE_READ_AHEAD);
static xthread* Producer = NULL;
static xthread* Consumer = NULL;

static volatile xbool s_bTerminator = FALSE;
static volatile HANDLE s_hSspEvent;
static volatile xbool s_bSuspended;
static volatile xbool s_bBusy;

static char** s_pFileList;
static s32 s_FileLength;
static s32 s_NFiles;

enum PacketType
{
    kOPEN, // begin transmission
    kDATA , // receive data
    kEND  , // done
};

struct Packet
{
    PacketType id;
    void * pData;
    u32    nData;
    s32    Index;
};


//-----------------------------------------------------

static void s_Producer( void )
{
#ifndef X_RETAIL
    DWORD dwTotal=0;
    xtimer timer;
    timer.Start( );
#endif
    char Temp[32]="?:\\";
    Packet* pNewPacket;
    HANDLE fpOut;
    HANDLE fpIn;
    DWORD len;

   for( s32 i=0;i<s_NFiles;i++ )
   {    //
        //  Open file for writing
        //
        Temp[0]='Z';
        x_strcpy( Temp+3,s_pFileList[i] );
        fpOut = CreateFile( Temp,GENERIC_WRITE,0,0,CREATE_NEW,0,0 );
        if( fpOut==INVALID_HANDLE_VALUE )
            continue;
        //
        //  Open file for reading
        //
        Temp[0]='D';
        fpIn = CreateFile( Temp,GENERIC_READ,0,0,OPEN_EXISTING,0,0 );
        if( fpIn==INVALID_HANDLE_VALUE )
            continue;
        len = GetFileSize( fpIn,NULL );
        if( !len ){
            CloseHandle( fpIn );
            continue;
        }
        //
        //  Transmit ok2go
        //
        pNewPacket = new Packet;
        pNewPacket->pData=fpOut;
        pNewPacket->nData=len;
        pNewPacket->id=kOPEN;
        pNewPacket->Index=i;
        Ready.Send(
            pNewPacket,
            MQ_NOBLOCK
        );
        //
        //  Start reading file blocks
        //
        for(;;)
        {   //
            //  Read packet
            //
            Packet* pPacket = ( Packet* )Pending.Recv( MQ_BLOCK );
            DWORD Retry=0;
            BOOL bResult;
            for(;;)
            {   //
                //  Read file
                //
                bResult = ReadFile( fpIn,pPacket->pData,CACHE_PACKET_SIZE,LPDWORD( &pPacket->nData ),0 );
                if( bResult )
                    break;
                else
                {
                #ifndef X_RETAIL
                    DWORD Err = GetLastError( );
                    BREAK
                #endif
                }
                //
                //  Die on excessive retries
                //
                if( ++Retry>10 )
                {
                    BREAK
                }
            }
        #ifndef X_RETAIL
            dwTotal += pPacket->nData;
        #endif
            //
            //  Safely suspend
            //
            WaitForSingleObject( s_hSspEvent,INFINITE );
            Ready.Send( pPacket,MQ_NOBLOCK );
            len-=pPacket->nData;
            ASSERT( len>=0 );
            if( len )
                continue;
            //
            //  Next file please
            //
            break;
        }
        CloseHandle( fpIn );
        fpIn = NULL;
        //
        //  Calculate throughput
        //
    #ifndef X_RETAIL
        f32 fThroughput = f32(dwTotal)/1024/1024/timer.ReadSec( );
        x_DebugMsg( "Total producer throughput %f MB/sec\n",fThroughput );
    #endif
        //
        //  Transmit end
        //
        pNewPacket = new Packet;
        pNewPacket->pData=NULL;
        pNewPacket->nData=0;
        pNewPacket->id=kEND;
        Ready.Send(
            pNewPacket,
            MQ_NOBLOCK
        );
    }
    //
    //  All files cached
    //
    CloseHandle( s_hSspEvent );
    s_hSspEvent = NULL;
    Producer = NULL;
    s_bBusy = FALSE;
#ifndef X_RETAIL
    timer.Stop( );
#endif
}

//-----------------------------------------------------

static void s_Consumer(void)
{
#ifndef X_RETAIL
    DWORD dwTotal=0;
    xtimer timer;
    timer.Start( );
#endif
    DWORD Result;
    s32 iIndex;
    HANDLE fp;
    s32 flen;

    while( s_bBusy )
    {
        Packet* pPacket = ( Packet* )Ready.Recv(MQ_BLOCK);
        switch( pPacket->id )
        {
            case kOPEN:
                iIndex= pPacket->Index;
                flen  = pPacket->nData;
                fp    = pPacket->pData;
                break;

            case kDATA:
            {
                DWORD Retry=0;
                BOOL bResult;
                for(;;)
                {   //
                    //  Write file (errors possibly injected)
                    //
                    bResult = WriteFile( fp,pPacket->pData,pPacket->nData,&Result,NULL );
                    if( bResult )
                        break;
                    else
                    {
                    #ifndef X_RETAIL
                        DWORD Err = GetLastError( );
                        BREAK
                    #endif
                    }
                    //
                    //  If retries failed, delete the file and die.
                    //
                    if( ++Retry>10 )
                    {
                        CloseHandle( fp );
                        DeleteFile( s_pFileList[iIndex] );
                        BREAK
                    }
                }
            #ifndef X_RETAIL
                dwTotal += Result;
            #endif
                Pending.Send( pPacket,MQ_NOBLOCK );
                flen -= Result;
                continue;
            }
            case kEND:
            {
            #ifndef X_RETAIL
                f32 fThroughput = f32(dwTotal)/1024/1024/timer.ReadSec( );
                x_DebugMsg( "Total consumer throughput %f MB/sec\n",fThroughput );
            #endif
                CloseHandle(fp);
                fp = NULL;
                break;
            }
        }
        WaitForSingleObject( s_hSspEvent,INFINITE );
        delete pPacket;
    }
    Consumer = NULL;
#ifndef X_RETAIL
    timer.Stop( );
#endif
}

//-----------------------------------------------------

void e_CacheFilesAsync( s32 NFiles,const char** pFileList )
{
    if( s_bBusy )
        return;
    s_bBusy = TRUE;
    //
    //  Number of files to read/write
    //
    s_pFileList = (char**)pFileList;
    s_NFiles = NFiles;
    s_FileLength = 0;
    //
    //  Create producer
    //
    if(!Producer )
        Producer = new xthread( s_Producer,"producer",8192,-7 );
    else
    {   //
        //  Setup message buffers
        //
        static Packet DataGrams[2]=
        {
            { kDATA,Buffer+0,266144 },
            { kDATA,Buffer+1,266144 }
        };
        Pending.Send( DataGrams+0,MQ_NOBLOCK);
        Pending.Send( DataGrams+1,MQ_NOBLOCK);
    }
    //
    //  Create consumer
    //
    if(!Consumer )
        Consumer = new xthread( s_Consumer,"consumer",8192,-7 );
    if(!s_hSspEvent )
        s_hSspEvent = CreateEvent( 0,1,1,0 );
}

//-----------------------------------------------------

void e_SuspendCaching( xbool bSuspend )
{
    if( bSuspend )
        ResetEvent( s_hSspEvent );
    else
        SetEvent( s_hSspEvent );
    s_bSuspended = bSuspend;
}

//-----------------------------------------------------

xbool e_IsCaching( void )
{
    return s_bSuspended && s_bBusy;
}




//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------
//-----------------------------------------------------



//-----------------------------------------------------

u32 render::texture_stage_state::Set( u32 iStage,u32 iIndex,u32 iValue )
{
    ASSERT( iStage < D3DTSS_MAXSTAGES );
    ASSERT( iIndex < D3DTSS_MAX );

    u32 Value = m_iData[ iStage ][ iIndex ];
    if( Value!=iValue )
    {
        VERIFY( !g_pd3dDevice->SetTextureStageState( iStage, D3DTEXTURESTAGESTATETYPE( iIndex ),DWORD( iValue )));
        m_iData[ iStage ][ iIndex ] = iValue;
    }

    return Value;
}

//-----------------------------------------------------

u32 render::texture_stage_state::Get( u32 iStage,u32 iIndex )
{
    ASSERT ( iStage < D3DTSS_MAXSTAGES );
    ASSERT ( iIndex < D3DTSS_MAX );
    return m_iData[iStage][iIndex];
}

//-----------------------------------------------------

#ifdef X_DEBUG
static u32 TrapState = 0;
#endif

u32 render::render_state::Set( u32 iIndex,u32 iValue )
{
    ASSERT ( iIndex < D3DRS_MAX );
    u32 Value = m_iData[ iIndex ];
    if( Value != iValue )
    {
        // Record old state if we're between push and pops

        if( m_bPush )
        {
            ASSERT( m_RecallCount < TOTAL_RECALL );
            push_record& Rec = m_Recall[m_RecallCount++];
            Rec.Index = iIndex;
            Rec.Value = Value;
        }

        // Set the render state
        VERIFY( !g_pd3dDevice->SetRenderState( D3DRENDERSTATETYPE( iIndex ),DWORD( iValue )));
    #ifdef X_DEBUG
        if( iIndex == TrapState )__asm int 3
    #endif
        m_iData[ iIndex ] = iValue;
    }
    return Value;
}

//-----------------------------------------------------

render::render_state::render_state( void )
{
    x_memset( m_iData,-1,D3DRS_MAX*sizeof( u32 ));
    m_RecallCount = 0;
    m_bPush = FALSE;
}

//-----------------------------------------------------

render::render_state::~render_state( void )
{
}

//-----------------------------------------------------

u32 render::render_state::Get( u32 iIndex )
{
    return m_iData[ iIndex ];
}

//-----------------------------------------------------

void render::render_state::Push( void )
{
    ASSERT( !m_bPush );
    m_bPush = 1;
}

//-----------------------------------------------------

void render::render_state::Pop( void )
{
    m_bPush = 0;

    for( u32 i=0;i<m_RecallCount;i++ )
    {
        push_record& Rec = m_Recall[i];
        Set( Rec.Index,Rec.Value );
    }
    m_RecallCount = 0;
}



//-----------------------------------------------------

void xbox_ScaleViewport( f32 Width, f32 Height, xbool bEnable )
{
    s_bScaleActive = bEnable;
    s_ScaleH      = Height;
    s_ScaleW      = Width;
}

//-----------------------------------------------------

void xbox_BeginFrameSmoothing( void )
{
    #if SWITCH_ADAPTIVE_FRAMERATE
    s_SmoothTimer.start();
    #endif
}

//-----------------------------------------------------

void xbox_EndFrameSmoothing( void )
{
    #if SWITCH_ADAPTIVE_FRAMERATE
    s_SmoothTimer.stop();
    #endif
}

//-----------------------------------------------------

void eng_Reboot( reboot_reason Reason )
{
    #if CONFIG_IS_DEMO
    {
        extern HANDLE s_hPads[4];
        XINPUT_FEEDBACK Feedback[4];
        x_memset( &Feedback[i], 0, sizeof(XINPUT_FEEDBACK) );
        for( s32 i=0; i<4; i++ )
            if( s_hPads[i] )
                XInputSetState( s_hPads[i], &Feedback[i] );

        ShowLegalScreen( "EndScreen",4000 );

        g_pd3dDevice->PersistDisplay();
        XLaunchNewImage( s_pDemoData->szLauncherXBE,&s_ld );
    }
    #else
    {
	    LD_LAUNCH_DASHBOARD DashConfig;

	    x_memset( &DashConfig, 0, sizeof(DashConfig) );
	    //eng_Kill();
	    switch( Reason )
	    {
	    case REBOOT_QUIT:
		    DashConfig.dwReason = XLD_LAUNCH_DASHBOARD_MAIN_MENU;
		    break;
	    case REBOOT_MANAGE:
		    DashConfig.dwReason = XLD_LAUNCH_DASHBOARD_NETWORK_CONFIGURATION;
		    break;
	    case REBOOT_NEWUSER:
		    DashConfig.dwReason = XLD_LAUNCH_DASHBOARD_NEW_ACCOUNT_SIGNUP;
		    break;
	    case REBOOT_MESSAGE:
		    DashConfig.dwReason = XLD_LAUNCH_DASHBOARD_ACCOUNT_MANAGEMENT;
		    break;
        case REBOOT_UPDATE:
            XOnlineTitleUpdate(NULL);
            // Deliberate fallthrough just in case this routine returns.
	    default:
		    ASSERT( FALSE );
		    DashConfig.dwReason = XLD_LAUNCH_DASHBOARD_MAIN_MENU;
		    break;
	    }
	    XLaunchNewImage( NULL, (LAUNCH_DATA*)&DashConfig );
    }
    #endif
}

//-----------------------------------------------------------------------------

xcolor xbox_UnswizzlePoint( const xbitmap* pBitmap,s32 X,s32 Y,s32 Mip )
{
    static u8* s_pPixels = NULL;
    static s32 s_VRAMID = 0;
    static s32 s_Mip = 0;

    if( !pBitmap )
    {
        x_free( s_pPixels );
        s_pPixels = NULL;

        return XCOLOR_BLACK;
    }

    xcolor Result;

    s32 H = pBitmap->GetHeight(Mip);
    s32 W = pBitmap->GetWidth (Mip);
    s32 B = pBitmap->GetBPP   ()>>3;
    s32 I = pBitmap->GetVRAMID();

    if( (s_VRAMID != I) || (Mip != s_Mip) )
    {
        // if different pixels free and realloc ...............................

        if( s_pPixels )
            x_free( s_pPixels );
        s_pPixels = (u8*)x_malloc( W*H*B );
        s_VRAMID = I;
        s_Mip = Mip;

        // unswizzle image into temporary buffer ..............................

        texture_factory::handle Handle=(texture_factory::handle)I;
        void* pSource = Handle->Lock(Mip);
        XGUnswizzleRect(
            pSource,
            W,
            H,
            NULL,
            s_pPixels,
            W*B,
            NULL,
            B
        );
        Handle->Unlock();
    }

    // define mask templates ..................................................

    struct mask1555{ u16 A:1;u16 R:5;u16 G:5;u16 B:5; };
    struct mask4444{ u16 A:4;u16 R:4;u16 G:4;u16 B:4; };
    struct mask565 { u16 R:5;u16 G:6;u16 B:5;         };

    // index into the desired pixel ...........................................

    switch( pBitmap->GetFormat() )
    {
        case xbitmap::FMT_32_ARGB_8888:
        case xbitmap::FMT_32_URGB_8888:
            Result = *(xcolor*)( s_pPixels+(X+Y*W)*B );
            break;

        case xbitmap::FMT_16_ARGB_4444:
        {
            mask4444& Mask = *(mask4444*)( s_pPixels+(X+Y*W)*B );
            Result.R = Mask.R;
            Result.G = Mask.G;
            Result.B = Mask.B;
            Result.A = Mask.A;
            break;
        }

        case xbitmap::FMT_16_ARGB_1555:
        case xbitmap::FMT_16_URGB_1555:
        {
            mask1555& Mask = *(mask1555*)( s_pPixels+(X+Y*W)*B );
            Result.R = Mask.R;
            Result.G = Mask.G;
            Result.B = Mask.B;
            Result.A = Mask.A;
            break;
        }

        case xbitmap::FMT_16_RGB_565:
        {
            mask565& Mask = *(mask565*)( s_pPixels+(X+Y*W)*B );
            Result.R = Mask.R;
            Result.G = Mask.G;
            Result.B = Mask.B;
            Result.A = 255;
            break;
        }

        default:
            BREAK;
    }

    return Result;
}

//-----------------------------------------------------------------------------

datestamp eng_GetDate( void )
{
    SYSTEMTIME  Time;
    datestamp   DateStamp;

    ASSERT( sizeof(DateStamp) == sizeof(FILETIME) );

    GetLocalTime( &Time );
    SystemTimeToFileTime( &Time, (FILETIME*)&DateStamp );
    return DateStamp;
}

//-----------------------------------------------------------------------------

split_date eng_SplitDate( datestamp DateStamp )
{
    SYSTEMTIME Time;
    split_date SplitDate;

    FileTimeToSystemTime( (FILETIME*)&DateStamp, &Time );
    SplitDate.Year          = Time.wYear;
    SplitDate.Month         = (u8)Time.wMonth;
    SplitDate.Day           = (u8)Time.wDay;
    SplitDate.Hour          = (u8)Time.wHour;
    SplitDate.Minute        = (u8)Time.wMinute;
    SplitDate.Second        = (u8)Time.wSecond;
    SplitDate.CentiSecond   = (u8)(Time.wMilliseconds/100);
    return SplitDate;
}

//-----------------------------------------------------------------------------

datestamp eng_JoinDate( const split_date& SplitDate )
{
    SYSTEMTIME  Time;
    datestamp   DateStamp;

    ASSERT( sizeof(datestamp) == sizeof(FILETIME) );
    Time.wYear          = SplitDate.Year;
    Time.wMonth         = SplitDate.Month;
    Time.wDay           = SplitDate.Day;
    Time.wHour          = SplitDate.Hour;
    Time.wMinute        = SplitDate.Minute;
    Time.wSecond        = SplitDate.Second;
    Time.wMilliseconds  = SplitDate.CentiSecond*100;
    SystemTimeToFileTime( &Time, (FILETIME*)&DateStamp );
    return DateStamp;
}

