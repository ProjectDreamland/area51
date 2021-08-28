//=========================================================================
//
// E_ENGINE.CPP
//
//=========================================================================

#ifdef VENDOR_MW
#include "mwUtils.h"
#endif

#include "e_engine.hpp"
#include "ps2_except.hpp"
#include "ps2_misc.hpp"
#include "x_threads.hpp"
#include "iopmanager.hpp"
#include "x_log.hpp"
#include "x_memory.hpp"
#include "audio_hardware.hpp"
#include "libscf.h"
#include "libmc2.h"


//#define INCLUDE_DEMO_CYCLE

#ifdef INCLUDE_DEMO_CYCLE
#include "libscedemo.h"
s32 g_DemoMode = 0;
s32 g_DemoTimeout = 10*60;
s32 g_DemoInactiveTimeout = 1*60;
#endif
//=========================================================================

#if (defined( CONFIG_VIEWER ) || defined( CONFIG_QA )) && (defined( autobuild ))
#define EXCEPTION_HANDLER
#endif

//#define BIT16
#define RESOLUTION_X    512
#define RESOLUTION_Y    448

#define WINDOW_LEFT    (2048-(512/2))
#define WINDOW_TOP     (2048-(512/2))

#define DLIST_SIZE     (1*1024*1024)

//=========================================================================

       xbool            DISPLAY_SMEM_STATS=FALSE;
static fsAABuff         s_aaBuff;
       fsAABuff*        s_pAAbuff;
       s32              s_FrameCount;
static s32              s_XRes=RESOLUTION_X;
static s32              s_YRes=RESOLUTION_Y;
static xbool            s_PalMode = FALSE;

       s32              s_SMem      = ( 277 * 1024); 

static s32              s_ProductCode = 20595;
static const char       s_ProductKey[] = "BASLUS";

static s32              s_BGColor[3] = {0,0,0};

static xbool            s_InsideTask = FALSE;
static xbool            s_FirstTask  = TRUE;
static xbool            s_PageFlipScreenClear = TRUE;

s32                     s_ContextStack[8] = {0,0,0,0,0,0,0,0};
s32                     s_ContextStackIndex = 0;
s32                     s_Context = 0;

#ifndef X_RETAIL
static xtimer           RoundTrip;
#endif // X_RETAIL

//
// View variables
//
static view             s_View;

// Screen shot variables
#if !defined( CONFIG_RETAIL )
static xbool            s_ScreenShotRequest  = FALSE ;
static xbool            s_ScreenShotActive   = FALSE ;
static s32              s_ScreenShotSize     = 1 ;
static s32              s_ScreenShotX        = 0 ;
static s32              s_ScreenShotY        = 0 ;
static s32              s_ScreenShotNumber   = 0 ;
static char             s_ScreenShotPath[X_MAX_PATH] = {0};
static char             s_ScreenShotName[X_MAX_PATH] = {0} ;

// Writes out next multi part screen shot
static void ProcessNextScreenShot( void ) ;

#endif // !defined( CONFIG_RETAIL )


#if !defined( CONFIG_RETAIL )
s32   ENG_DLIST_MAX_USED=0;
#endif // !defined( CONFIG_RETAIL )

//=========================================================================

#if !defined(X_RETAIL) || defined(X_QA)
static xtimer STAT_CPUTIMER;
f32 STAT_GSMS;
f32 STAT_CPUMS;
f32 STAT_IMS;
f32 STAT_VBL;
#endif //!defined(X_RETAIL) || defined(X_QA)

void draw_Init( void );
void draw_Kill( void );
extern s32 VRAM_BytesLoaded;
static xbool s_Initialized = FALSE;

//=========================================================================

void ps2_ResetHardware(void);

//=========================================================================

void ClearVram(void)
{
    sceGsLoadImage gsImage PS2_ALIGNMENT(64);
    s32 i;
    u8 *pData;

    pData = (u8 *)x_malloc(256 * 256 * 4);
    ASSERT(pData);
    x_memset(pData,0x00,256*256*4);
    for (i=0;i<16;i++)
    {
        sceGsSetDefLoadImage(&gsImage,i * 1024, 4 ,SCE_GS_PSMCT32,0,0,256,256);
        FlushCache(0);
        sceGsExecLoadImage(&gsImage,(u_long128 *)pData);
        sceGsSyncPath(0,0);
    }
    x_free(pData);
}

//=============================================================================

static void ClearVRAM( s16 DstBase, s32 Width, s32 Height, xcolor Color, u32 Mask )
{
    ASSERT( DstBase < 512 );

    // Draw 32 pixel wide columns at a time
    s32 ColumnWidth  = 32;
    s32 nColumns     = Width / ColumnWidth;
    
    gsreg_Begin( 4 + nColumns*2 );
    gsreg_Set( SCE_GS_FRAME_1,   SCE_GS_SET_FRAME_1  ( DstBase, Width / 64, SCE_GS_PSMCT32, ~Mask ) );
    gsreg_Set( SCE_GS_TEST_1,    SCE_GS_SET_TEST_1   ( 0, 0, 0, 0, 0, 0, 1, 1 ) );
    gsreg_Set( SCE_GS_PRIM,      SCE_GS_SET_PRIM     ( SCE_GS_PRIM_SPRITE, 0, 0, 0, 0, 0, 0, 0, 0 ) );
    gsreg_Set( SCE_GS_RGBAQ,     SCE_GS_SET_RGBAQ    ( Color.R, Color.G, Color.B, Color.A, 0x3F800000 ) );

    s32 X  = WINDOW_LEFT;
    s32 Y0 = WINDOW_TOP;
    s32 Y1 = Y0 + Height;
    
    for( s32 i=0; i<nColumns; i++ )
    {
        s32 X0 = X;
        s32 X1 = X0 + ColumnWidth;
    
        gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ( (X0 << 4), (Y0 << 4), 0 ) );
        gsreg_Set( SCE_GS_XYZ2, SCE_GS_SET_XYZ( (X1 << 4), (Y1 << 4), 0 ) );
        
        X += ColumnWidth;
    }
    
    gsreg_End();
}

//=============================================================================

//
// Copied from x_bitmap_io.cpp
//
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

static s32 ShowSplashGetPS2SwizzledIndex( s32 I )
{
    static u8 swizzle_lut[32] = 
    {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
            0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
            0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
            0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };

    return swizzle_lut[(I) & 31] + (I & ~31);
}

static sceGsDBuff       s_db;

void ShowSplash(void)
{
    s32				handle,length;
    io_buffer*		mem;
    u8*				pData;
    s32				i;
    s32				xoffset,yoffset;
    sceGsLoadImage	gsimage;
    u32*			pPalette;
    u32*			pLineBuffer;
    s32				Height;


    ClearVram();
    //
    // Initialize CD system
    //
    //sceSifInitRpc(0);
    //sceCdInit(SCECdINIT);
    //sceCdMmode(SCECdCD);

    scePrintf("EarlyStart: Initializing graphics environment\n");
    ps2_ResetHardware();
    //
    // GS
    //
    *(u_long *)&s_db.clear0.rgbaq = SCE_GS_SET_RGBAQ(0x00, 0x00, 0x00, 0x00, 0x3f800000);
    *(u_long *)&s_db.clear1.rgbaq = SCE_GS_SET_RGBAQ(0x00, 0x00, 0x00, 0x00, 0x3f800000);
    if( s_PalMode )
    {
        Height = 512;
    }
    else
    {
        Height = 448;
    }
    s_pAAbuff       = (fsAABuff*) (((u_int)(&s_aaBuff)) | 0x20000000);

#ifdef BIT16

    SetupFS_AA_buffer( s_pAAbuff, 
        512, 
        512, 
        SCE_GS_PSMCT16,  
        SCE_GS_ZGEQUAL,//SCE_GS_ZGREATER, 
        SCE_GS_PSMZ16,  
        SCE_GS_CLEAR);   
#else

    SetupFS_AA_buffer( s_pAAbuff, 
        512, 
        512, 
        SCE_GS_PSMCT32,  
        SCE_GS_ZGEQUAL,//SCE_GS_ZGREATER, 
        SCE_GS_PSMZ32,  
        SCE_GS_CLEAR);   
#endif
    //
    // VBLANK
    //
    scePrintf("EarlyStart: Waiting for vblank\n");
    FlushCache(0);
    while(sceGsSyncV(0) == 0){};  

    char Tail[X_MAX_PATH];
    char Path[X_MAX_PATH];

#ifdef TARGET_DEV
    x_strcpy( Path, "host:c:\\GameData\\A51\\Release\\PS2\\" );
    x_strcpy( Tail, "P" );
#elif defined(INCLUDE_DEMO_CYCLE)
    x_strcpy( Path, "cdrom0:\\AREA51\\" );
    x_strcpy( Tail, ";1" );
#else
    x_strcpy( Path, "cdrom0:\\" );
    x_strcpy( Tail, ";1" );
#endif

    x_strcat( Path, "LT" );

    if( s_PalMode )
    {
        s32 Language = sceScfGetLanguage();
        switch( Language )
        {
        case SCE_ENGLISH_LANGUAGE:	    x_strcat( Path, "ENG" ); break;
        case SCE_FRENCH_LANGUAGE:	    x_strcat( Path, "FRE" ); break;
        case SCE_SPANISH_LANGUAGE:	    x_strcat( Path, "SPA" ); break;
        case SCE_GERMAN_LANGUAGE:	    x_strcat( Path, "GER" ); break;
        case SCE_ITALIAN_LANGUAGE:      x_strcat( Path, "ITA" ); break;
        default:                        x_strcat( Path, "ENG" ); break;
        }
    }
    else
    {
        x_strcat( Path, "USA" );
    }

    x_strcat( Path, ".XBM" );
    x_strcat( Path, Tail );
    handle = sceOpen( Path, SCE_RDONLY );
    if( handle < 0 )
        return;

    length = sceLseek(handle,0,SCE_SEEK_END);
    sceLseek(handle,0,SCE_SEEK_SET);

    mem = (io_buffer*)x_malloc(length);
    ASSERT(mem);

    sceRead(handle,mem,length);
    sceClose(handle);

    // Now, download the image to display memory
    pData = (u8*)((u8*)mem+sizeof(io_buffer));
    pPalette = (u32*)(pData+mem->DataSize);

    yoffset = (Height - mem->Height)/2;
    xoffset = (512 - mem->Width)/2;

    ASSERT(yoffset>=0);
    ASSERT(xoffset>=0);

    pLineBuffer = (u32*)x_malloc(mem->Width * sizeof(u32));
    ASSERT(pLineBuffer);

    for( i=0; i<mem->Height; i++ )
    {
        s32 j;
        for( j=0; j<mem->Width; j++ )
        {
            pLineBuffer[j] = pPalette[ShowSplashGetPS2SwizzledIndex(*pData)];
            pData++;
        }
        sceGsSetDefLoadImage(&gsimage,  0,					// Base address of dest buffer
            mem->Width/64,		// Width of transfer
            SCE_GS_PSMCT32,		// Data format
            xoffset,			// Dest X
            i+yoffset,			// Dest Y
            mem->Width,			// Dest width
            1					// Dest height
            );
        FlushCache(0);
        sceGsExecLoadImage(&gsimage, (u_long128*)pLineBuffer);
        sceGsSyncPath(0, 0);
    }

    x_free(pLineBuffer);
    x_free(mem);
    while(sceGsSyncV(0) == 0){};  
    sceGsDispEnv disp;

    // ////////////////////////////
    // 
    //  Initilize display
    // 
    sceGsSetDefDispEnv(
        &disp,
        SCE_GS_PSMCT32,
        512,
        Height,
        0,
        0
        );
    sceGsPutDispEnv(&disp);
}

void eng_Init(void)
{
    MEMORY_OWNER( "eng_Init()" );

#ifdef TARGET_PS2_CLIENT
    if (s_Initialized)
        return;
#endif
    ASSERT(!s_Initialized);
    s_Initialized = TRUE;

    s_Context = 0;
    s_ContextStackIndex = 0;
    s_ContextStack[0] = 0;

#if !defined( CONFIG_RETAIL )
    ENG_DLIST_MAX_USED = 0;
#endif // !defined( CONFIG_RETAIL )

    //
    // GS
    //
    printf("ENGINE: Initializing graphics environment\n");
#if 0       // def TARGET_DVD This is not allowed. Violation of TRC.
            // See ps2eng_Begin() - we get the territory and pal mode from the
            // executable name via command line.

    // In their infinite wisdom, sony wants to hide whether or not
    // the game is working on PAL or NTSC hardware so there is no
    // actual mechanism for determining this. The only solution Sony
    // support gave was to check the SYSTEM.CNF file on the CDROM.
    // Lame.
    {
        s32 file;
        s32 length;
        char *pByte;

        char Temp[512];

        file = sceOpen("cdrom0:\\SYSTEM.CNF;1",SCE_RDONLY);
        if (file >= 0)
        {
            length = sceRead(file,Temp,512);

            pByte = Temp;
            s_PalMode = FALSE;
            while (length)
            {
                if (x_strncmp(pByte,"VMODE",5)==0)
                {
                    pByte+=5;
                    while (*pByte==' ') pByte++;
                    ASSERT(*pByte == '=');
                    pByte++;
                    while (*pByte==' ') pByte++;

                    if (x_strncmp(pByte,"PAL",3)==0)
                    {
                        s_PalMode=TRUE;
                    }
                    break;
                }
                length--;
                pByte++;
            }
            sceClose(file);
        }
    }
#endif
    //
    // INPUT
    //
    printf("ENGINE: Initializing input\n");
    input_Init();

    //
    // USB
    //
    printf("ENGINE: Initializing usb\n");
    //USB_Init();

    //
    // AA
    //
    printf("ENGINE: Initializing Anti-aliasing settings\n");
    s_pAAbuff       = (fsAABuff*) (((u_int)(&s_aaBuff)) | 0x20000000);

#ifdef BIT16

    SetupFS_AA_buffer( s_pAAbuff, 
                       512, 
                       512, 
                       SCE_GS_PSMCT16,  
                       SCE_GS_ZGEQUAL,//SCE_GS_ZGREATER, 
                       SCE_GS_PSMZ16,  
                       SCE_GS_CLEAR);   
#else

    SetupFS_AA_buffer( s_pAAbuff, 
                       512, 
                       512, 
                       SCE_GS_PSMCT32,  
                       SCE_GS_ZGEQUAL,//SCE_GS_ZGREATER, 
                       SCE_GS_PSMZ24,  
                       SCE_GS_CLEAR);   
#endif


    //
    // VRAM
    //
    printf("ENGINE: Initializing vram module\n");
    vram_Init();

#if !defined( X_RETAIL ) || defined( X_QA )
    //
    // FONT
    //
    printf("ENGINE: Initializing font module\n");
    font_Init();

    //
    // TEXT
    //
    printf("ENGINE: Initializing text module\n");
    text_Init();
    text_SetParams( 512, 448, 8, 8,
                    13, 
                    18,
                    8 );
    text_ClearBuffers();
#endif //  !defined( X_RETAIL ) || defined( X_QA )

    //
    // DRAW
    //
    draw_Init();

    //
    // DMAS
    //
    printf("ENGINE: Initializing dmas\n");
    {
        sceDmaEnv denv;
	    sceDmaReset(1);
	    sceDmaGetEnv(&denv);
	    denv.notify = 0x0100; // enable Ch.8 CPCOND 
	    sceDmaPutEnv(&denv);
    }

    //
    // SMEM
    //
    printf("ENGINE: Initializing scratch memory\n");
    smem_Init(s_SMem);

    //
    // DLIST
    //
    printf("ENGINE: Initializing MFIFO dlist\n");
    DLIST.Init( DLIST_SIZE );
    extern s32 x_GetMainThreadID( void );
    DLIST.SetThreadID( x_GetMainThreadID() );
    DLIST.Enable();
    DLIST.BeginFrame();
    s_FirstTask = TRUE;

    //
    // TIME
    //
    printf("ENGINE: Initializing time\n");
#ifndef X_RETAIL
    RoundTrip.Start();
#endif // X_RETAIL
}

//=========================================================================

void eng_Kill(void)
{
    DLIST.Disable();
    DLIST.Kill();
    smem_Kill();
    draw_Kill();
    text_Kill();
}

//=========================================================================

void eng_GetRes( s32& XRes, s32& YRes )
{
    XRes = s_XRes;
    YRes = s_YRes;
}

//=========================================================================
    
void eng_GetPALMode( xbool& PALMode )
{
    PALMode = s_PalMode;
}
//=========================================================================

void eng_MaximizeViewport( view& View )
{
    View.SetViewport( 0, 0, s_XRes, s_YRes );
}

//=========================================================================

void eng_SetBackColor( xcolor Color )
{
    s_BGColor[0] = Color.R;
    s_BGColor[1] = Color.G;
    s_BGColor[2] = Color.B;
    FB_SetBackgroundColor(s_pAAbuff,Color.R,Color.G,Color.B);
}

//=========================================================================

void eng_Sync( void )
{
    ASSERTS( FALSE, "Not supported on PS2!" );
}

//=========================================================================
void ps2_ResetHardware(void)
{
	s32 pmode;
	s32 omode;

	sceDmaReset( 1 );
	sceDevVif0Reset();
	sceDevVu0Reset();
	sceDevVif1Reset();
	sceDevVu1Reset();
	sceDevGifReset();
	sceDevVu1PutDBit(1);
	sceGsResetPath();

#if 0
	if ( s_ProgressiveScan && !s_PalMode)
	{
		pmode = SCE_GS_NOINTERLACE;
		omode = SCE_GS_DTV480P;
	}
	else
#endif
	{
		pmode = SCE_GS_INTERLACE;
		if (s_PalMode)
		{
			omode = SCE_GS_PAL;
			s_YRes = 512;
		}
		else
		{
			omode = SCE_GS_NTSC;
		}
	}

    sceGsResetGraph(0,pmode,omode,SCE_GS_FIELD);
	sceGsResetPath();

    // reinitialize the dlist
    DLIST.Kill();
    DLIST.Init( DLIST_SIZE );
    s_FrameCount = 0;
}

void eng_PresentFrame( s32 Frame )
{
    PutDispBuffer( s_pAAbuff, Frame, TRUE );     // toggle 2 circuits
}

static volatile xbool s_InsidePageFlip = FALSE;

void eng_PageFlip(void)
{
    ASSERT( !s_InsidePageFlip );
    s_InsidePageFlip = TRUE;

    CONTEXT( "eng_PageFlip" );
    LOG_FLUSH();

    // It is possible that we can do a page flip after drawing nothing.
    // In this case, the screen wouldn't have been cleared, because that
    // happens on the first call to eng_Begin for performance reasons. In
    // that rare case, let's force the issue by having an empty eng_Begin/End
    // pair.
    if( s_FirstTask )
    {
        if( eng_Begin( "ForcedScreenWipe" ) )
        {
            eng_End();
        }
    }

    //
    // COLLECT STATS
    //
#if !defined(X_RETAIL) || defined(X_QA)
    // grab the CPU timer stats
    STAT_CPUTIMER.Stop();
    STAT_CPUMS = STAT_CPUTIMER.ReadMs();
#endif  // !defined( X_RETAIL ) || defined( X_QA )

    //
    // ADD TEXT RENDER
    //
#if !defined(X_RETAIL) || defined(X_QA)
    #ifndef X_RETAIL
    if( !eng_ScreenShotActive() )
    #endif
    {
        eng_Begin("Text");
        text_Render();
        eng_End();
    }
#endif  // !defined( X_RETAIL ) || defined( X_QA )

    //
    // TOGGLE BUFFERS
    //
#if !defined( X_RETAIL ) || defined( X_QA )
    text_ClearBuffers();
#endif // X_RETAIL
    smem_Toggle();

    //
    // End the current frame
    // NOTE: We delay the beginning of the next frame until the first
    // eng_Begin call. This gives a chance for the MFIFO to catch up
    // while doing logic, AI, etc.
    //
    if( !DLIST.InsideFrame() )
    {
        // We shouldn't normally come inside here, but it is possible
        // to do so if somebody called multiple page flips without a
        // Begin/End pair.
        DLIST.BeginFrame();
    }
    s_FirstTask = TRUE;
    DLIST.EndFrame();
    s_FrameCount++;

    //
    // CHECK ON GS CONTEXT
    //
    ASSERT( s_ContextStackIndex == 0 );
    s_Context = 0;
    s_ContextStackIndex = 0;
    s_ContextStack[0] = 0;

    //
    // HANDLE SCREEN SHOTS
    //
#if !defined( CONFIG_RETAIL )
    // Write out next multi-part screen shot?
    if ( s_ScreenShotRequest || s_ScreenShotActive )
        ProcessNextScreenShot() ;
#endif // !defined( CONFIG_RETAIL )

    //
    // RESTART THE DLIST
    //
    DLIST.BeginFrame();

    //
    // RETURN TO USER
    //
#if !defined(X_RETAIL) || defined(X_QA)
    STAT_CPUTIMER.Reset();
    STAT_CPUTIMER.Start();
#endif // !defined(X_RETAIL) || defined(X_QA)

    s_InsidePageFlip = FALSE;
}

//=========================================================================

s32 eng_GetFrameIndex( void )
{
    return(s_FrameCount&0x01);
}

//=========================================================================

xbool eng_InBeginEnd(void)
{
    return s_InsideTask;
}

//=========================================================================

xbool eng_Begin( const char* pTaskName )
{
    ASSERT( !eng_InBeginEnd() );
	
    s_InsideTask = TRUE;
    if( pTaskName == NULL )
        pTaskName = "unknown";

    // If this is the first task of the frame, then we need to wait for
    // the previous frame to finish and start the new one. (We've deferred
    // the actual page flip to this point so that the MFIFO has a chance
    // to run while logic is happening!)
    if( s_FirstTask )
    {
        s_FirstTask = FALSE;

        // set up the default registers and clear the frame buffer
        DLIST.BeginTask( "GSReset" );
        gsreg_Begin( 2 );
        gsreg_Set( SCE_GS_ZBUF_1, SCE_GS_SET_ZBUF( VRAM_ZBUFFER_START / 8192, SCE_GS_PSMZ24, 0 ) );
        gsreg_Set( SCE_GS_ZBUF_2, SCE_GS_SET_ZBUF( VRAM_ZBUFFER_START / 8192, SCE_GS_PSMZ24, 0 ) );
        gsreg_End();
        if( s_PageFlipScreenClear )
            ClearVRAM( eng_GetFrameBufferAddr(0) / 2048, s_XRes, s_YRes, xcolor(s_BGColor[0], s_BGColor[1], s_BGColor[2], 0), 0xffffffff );
        gsreg_Begin( 6*2 + 3);
        for ( s32 context = 0; context < 2; context++ )
        {
            eng_PushGSContext(context);
            gsreg_SetAlphaBlend          ( ALPHA_BLEND_OFF );
            gsreg_SetClamping            ( FALSE );
            gsreg_SetFBMASK              ( 0x00000000 ); // will also set the FRAME register
            gsreg_Set                    ( SCE_GS_XYOFFSET_1 + context,
                SCE_GS_SET_XYOFFSET( WINDOW_LEFT<<4, WINDOW_TOP<<4 ) );
            gsreg_SetScissor             ( 0, 0, s_XRes, s_YRes );
            gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 0, ALPHA_TEST_FAIL_KEEP,
                FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_ALWAYS );
            eng_PopGSContext();
        }
        gsreg_Set( SCE_GS_PRMODECONT, SCE_GS_SET_PRMODECONT(1) );
        gsreg_Set( SCE_GS_COLCLAMP,   SCE_GS_SET_COLCLAMP(1) );
        gsreg_Set( SCE_GS_DTHE,       SCE_GS_SET_DTHE(0) );
        gsreg_End();
        vram_SetMipK( -10.0f );
        vram_Flush();
        DLIST.EndTask();
    }

    // start up the new task
    DLIST.BeginTask( pTaskName );

    return TRUE;
}

//=========================================================================

void eng_End(void)
{
    ASSERT( eng_InBeginEnd() );
    DLIST.EndTask();
    s_InsideTask = FALSE;
}

//=========================================================================

#if !defined( CONFIG_RETAIL )

// Writes out current frame buffer into file
#define LINES_PER_CHUNK 32

static void DumpFrameBuffer( const char* pFileName )
{
    ASSERT(pFileName) ;
    s32     XRes = s_XRes;
    s32     YRes = s_YRes;
    X_FILE* fp;
    s32     i;
    //YRes = 512*3;
    // Try open file
    fp = x_fopen(pFileName,"wb");
    if (!fp)
        return;

    // Build tga header
    byte Header[18];
    x_memset(&Header,0,18);
    Header[16] = 32;
    Header[12] = (XRes>>0)&0xFF;
    Header[13] = (XRes>>8)&0xFF;
    Header[14] = (YRes>>0)&0xFF;
    Header[15] = (YRes>>8)&0xFF;
    Header[2]  = 2;
    Header[17] = 32;    // NOT flipped vertically.

    // Write out header data
    x_fwrite( Header, 18, 1, fp );

    // Steal space from the display list to store our screen shot so we don't
    // have to do a malloc. A bit dodgy, but this allows us to get screenshots
    // when using ProView.
    s32 Size = s_XRes*LINES_PER_CHUNK*sizeof(xcolor);
    x_DebugMsg( "Screenshot mem needed: %d\nAvailable:%d\n\n", Size, (u32)DLIST_SIZE );
    ASSERT( s_XRes*LINES_PER_CHUNK*sizeof(xcolor) < (u32)DLIST_SIZE );
    xcolor* pBuffer = (xcolor*)DLIST.GetMFIFOStart();
    ASSERT(pBuffer);

    
    //
    // KILL THE MIFO
    //
	DLIST.Disable();

    // Build and execute image store
    sceGsStoreImage gsimage;
    s32 NLines = YRes;
    s32 Offset = 512 * ((s_FrameCount&0x1) ^ 0x01) ;
    while( NLines )
    {
        s32 N = MIN(NLines,LINES_PER_CHUNK);
        sceGsSetDefStoreImage(&gsimage, Offset*XRes/64, XRes / 64, SCE_GS_PSMCT32, 0, 0, XRes, N);

	    FlushCache(0);
	    sceGsExecStoreImage(&gsimage, (u_long128*)(pBuffer));
	    sceGsSyncPath(0, 0);

        NLines -= N;
        Offset += N;

        for (i=0;i<XRes*N;i++)
        {
            xcolor C = pBuffer[i];
            pBuffer[i].R = C.B;
            pBuffer[i].G = C.G;
            pBuffer[i].B = C.R;
            pBuffer[i].A = C.A;
        }
        x_fwrite(pBuffer,XRes*sizeof(xcolor)*N,1,fp);
    }

    x_fclose(fp);

    //
    // RE-ENABLE THE MFIFO
    //
    DLIST.Enable();
}

//=========================================================================

// Returns current shot name
static void GetScreenShotSubPicFName( char* ShotName )
{
    // Locals
    char    Drive   [X_MAX_DRIVE];
    char    Dir     [X_MAX_DIR];
    char    FName   [X_MAX_FNAME];
    char    Ext     [X_MAX_EXT];

    // Create current shot name in the form:  ps2shot000_4x4_000.tga
    x_splitpath(s_ScreenShotName, Drive, Dir, FName, Ext) ;
    x_sprintf(ShotName, "%s%s%s_%dx%d_%03d%s", 
              Drive, Dir, FName,                        // ps2shot000
              s_ScreenShotSize, s_ScreenShotSize,       // 4x4
              s_ScreenShotNumber,                       // 000
              Ext) ;                                    // .tga
}

// Writes out next multi part screen shot
static void ProcessNextScreenShot( void )
{
    // Locals
    char    ShotName[X_MAX_PATH];

    // If just requested screen shot, then start the sequence -
    // next time round we'll be ready to capture
    if (s_ScreenShotRequest)
    {
        // Flag we are now active
        s_ScreenShotRequest = FALSE ;
        s_ScreenShotActive  = TRUE ;
        return ;
    }

    // Should only come here if active
    ASSERT(s_ScreenShotActive) ;

    // Create current shot name in the form:  ps2shot000_4x4_000.tga
    GetScreenShotSubPicFName(ShotName) ;
    
    // Goto next screen shot
    s_ScreenShotNumber++ ;
    
    // Write out current shot
    DumpFrameBuffer(ShotName) ;

    // Next screen across?
    if (++s_ScreenShotX == s_ScreenShotSize)
    {
        // Next screen down
        s_ScreenShotX = 0 ;

        // Done?
        if (++s_ScreenShotY == s_ScreenShotSize)
        {
            // End the screen shot
            s_ScreenShotActive = FALSE ;
            s_ScreenShotSize   = 1 ;
            s_ScreenShotX      = 0 ;
            s_ScreenShotY      = 0 ;

            // Auto-join the pics (PicJoinTool.exe should be in same dir as .elf)
            #ifndef X_RETAIL            
                s32 File = sceOpen( xfs( "host0:EXEC:%sPicJoinTool.exe %s*.tga", s_ScreenShotPath, s_ScreenShotPath ), 1 );
                if (File > 0)
                    sceClose(File) ;
            #endif
        }
    }
}

//=========================================================================

void eng_ScreenShot( const char* pFileName /*= NULL*/, s32 Size /*= 1*/  )
{
    // If already shooting, exit so that screen number does not increase!
    if ((s_ScreenShotRequest) || (s_ScreenShotActive))
        return ;

    // Request a screen shot now - this syncs with the page flip
    s_ScreenShotRequest = TRUE ;
    
    // Start at top left
    s_ScreenShotSize    = Size ;
    s_ScreenShotNumber  = 0 ;
    s_ScreenShotX       = 0 ;
    s_ScreenShotY       = 0 ;

    // Get file info
    char Drive[X_MAX_DRIVE] = {0} ;
    char Dir  [X_MAX_DIR  ] = {0} ;
    char FName[X_MAX_FNAME] = {0} ;
    char Ext  [X_MAX_EXT  ] = {0} ;

    // Clear path and filename
    s_ScreenShotPath[0] = 0;
    s_ScreenShotName[0] = 0;
    
    // Filename/dir specified?
    if (pFileName)
    {
        x_strcpy(s_ScreenShotName, pFileName) ;
        x_splitpath(pFileName, Drive, Dir, FName, Ext) ;
        x_makepath( s_ScreenShotPath, Drive, Dir, NULL, NULL );
    }

    // If pic name wasn't specified, auto search for next one
    if (x_strlen(Ext) == 0)
    {
        // Auto name
        s32     AutoName = 0 ;
        X_FILE* pFile    = NULL ;
        char    SubPicName[X_MAX_PATH] ;
        do
        {
            // Goto next auto name
            x_sprintf(FName, "ps2sshot%03d", AutoName++) ;
            x_makepath(s_ScreenShotName, Drive, Dir, FName, ".tga") ;

            // Try big file / single screen shot name
            pFile = x_fopen(s_ScreenShotName, "rb") ;
            if (!pFile)
            {
                // Try sub pic name
                GetScreenShotSubPicFName( SubPicName ) ;
                pFile = x_fopen(SubPicName, "rb") ;
            }

            // Just close the file if was found
            if (pFile)
                x_fclose(pFile) ;
        }
        while(pFile) ;  // Keep going until we get to a file that does not exist
    }
}

//=========================================================================

s32 eng_ScreenShotActive( void )
{
    return s_ScreenShotActive ;
}

//=========================================================================

s32 eng_ScreenShotSize( void )
{
    return s_ScreenShotSize ;
}

//=========================================================================

s32 eng_ScreenShotX( void )
{
    return s_ScreenShotX ;
}

//=========================================================================

s32 eng_ScreenShotY( void )
{
    return s_ScreenShotY ;
}

#endif // !defined( CONFIG_RETAIL )

//=============================================================================
//=============================================================================
//=============================================================================
//  VIEW MANAGEMENT
//=============================================================================
//=============================================================================
//=============================================================================

void eng_SetView ( const view& View )
{
    s_View = View;

#if !defined( CONFIG_RETAIL )
    // Tell the view about a multi-part screen shot!
    s_View.SetSubShot( s_ScreenShotX, s_ScreenShotY, s_ScreenShotSize ) ;
#endif // !defined( CONFIG_RETAIL )
}

//=============================================================================

const view* eng_GetView( void )
{
    return &s_View;
}

//=============================================================================
//=============================================================================
//=========================================================================
// MISC
//=============================================================================
//=============================================================================
//=============================================================================

#if !defined(X_RETAIL) || defined(X_QA)
void    eng_GetStats(s32 &Count, f32 &CPU, f32 &GS, f32 &INT, f32& VBL, f32 &FPS)
{
    Count=0;
    CPU   = STAT_CPUMS;
    GS    = STAT_GSMS;
    INT   = STAT_IMS;
    VBL   = STAT_VBL;
    FPS   = 1000.0f / (STAT_CPUMS+STAT_IMS);
}
#endif // !defined(X_RETAIL) || defined(X_QA)

//=========================================================================
// TODO: Do this properly

#if !defined(X_RETAIL) || defined(X_QA)

f32 eng_GetFPS()
{
    return 1000.0f / (STAT_CPUMS+STAT_IMS);
}
#endif // X_RETAIL

//=========================================================================

#ifndef X_RETAIL
void    eng_PrintStats      ( void )
{
    s32 Count ;
    f32 CPU, GS, INT, VBL, FPS ;
    
    eng_GetStats(Count, CPU, GS, INT, VBL, FPS) ;

    //x_DebugMsg("================ Engine Stats ================\n");
	//x_DebugMsg("%1d CPU:%4.1f  GS:%4.1f  INT:%4.1f  VBL:%4.1f  FPS:%4.1f\n", Count, CPU, GS, INT, VBL, FPS) ;
    //dmaman_PrintTaskStats();

    {
        vector3 Pos;
        radian Pitch;
        radian Yaw;

        Pos = s_View.GetPosition();
        s_View.GetPitchYaw(Pitch,Yaw);

        x_DebugMsg("View0 Location: (%1.4f,%1.4f,%1.4f) P:%1.4f Y:%1.4f\n",
            Pos.GetX(),Pos.GetY(),Pos.GetZ(),Pitch,Yaw);
    }


    //for( s32 i=0; i<5; i++ )
        //printf("%4.1f ",STAT_INTERNAL[i]);
    //printf("\n");
}
#endif // X_RETAIL

//=========================================================================

xbool   eng_InsideTask      ( void )
{
    return s_InsideTask;
}

//=========================================================================

u64     eng_GetFRAMEReg     ( void )
{
    return FB_GetFRAMEReg(s_pAAbuff,  ((s_FrameCount)&0x01));
}

//=========================================================================

// 0=back buffer, 1=front buffer
u64     eng_GetFRAMEReg     ( s32 Buffer )
{
    ASSERT((Buffer == 0) || (Buffer == 1)) ;
    return FB_GetFRAMEReg(s_pAAbuff,  ((s_FrameCount+Buffer)&0x01));
}

//=========================================================================

u32     eng_GetFrameBufferAddr ( s32 Buffer )
{
    ASSERT((Buffer == 0) || (Buffer == 1)) ;
    return FB_GetFrameBufferAddr(s_pAAbuff,  (s_FrameCount + Buffer) & 0x01) ;
}

//=========================================================================

#if !defined( X_RETAIL ) || defined( X_QA )

void text_BeginRender( void )
{
    font_BeginRender();
}

#endif // !defined( X_RETAIL ) || defined( X_QA )

//=========================================================================

#if !defined( X_RETAIL ) || defined( X_QA )

void text_RenderStr( char* pStr, s32 NChars, xcolor Color, s32 PixelX, s32 PixelY )
{
    font_Render( pStr, NChars, Color, PixelX, PixelY );
}

#endif // !defined( X_RETAIL ) || defined( X_QA )

//=========================================================================

#if !defined( X_RETAIL ) || defined( X_QA )

void text_EndRender( void )
{
    font_EndRender();
}

#endif // !defined( X_RETAIL ) || defined( X_QA )

//=============================================================================
//=============================================================================
//=========================================================================
// STARTUP
//=============================================================================
//=============================================================================
//=============================================================================

void PS2_SetFileMode( s32 Mode, char* pPrefix );
char s_CopyOfBootFilename[256];
void ps2_GetFreeMem(s32 *pFree,s32 *pLargest,s32 *pFragments);

//=============================================================================

void ps2eng_Begin( s32 argc, char* argv[] )
{
#ifdef TARGET_DEV
    xbool CDROM=FALSE;
#else
    xbool CDROM=TRUE;
#endif

    #ifdef VENDOR_MW
	mwInit();    /* To initialize the C++ runtime */
    #endif

    //
    // Watch for booting from a cdrom on a non-cd build
    //
#ifndef TARGET_DVD
    if( argc && 
        (argv[0][0] == 'c') &&
        (argv[0][1] == 'd') &&
        (argv[0][2] == 'r') &&
        (argv[0][3] == 'o') &&
        (argv[0][4] == 'm') )
#endif
    {
        CDROM = TRUE;
    }

    sceSifInitRpc(0);
    sceCdInit(SCECdINIT);

#if defined(INCLUDE_DEMO_CYCLE)
    u16 language,aspect,playmode,inactive,gameplay,mediatype;

    mediatype = SCE_DEMO_MEDIATYPE_CD;
    language=aspect=playmode=inactive=gameplay= 0;

    sceDemoStart(argc, argv, &language, &aspect, &playmode, &inactive, &gameplay, NULL, NULL, NULL);
    scePrintf("Language=%d,Aspect=%d,Playmode=%d,Inactive=%d, Gameplay=%d\n",language,aspect,playmode,inactive,gameplay);
    if( gameplay == 0 )
    {
        gameplay = 4*60;
        inactive = 1*60;
    }
    g_DemoTimeout = gameplay;
    g_DemoInactiveTimeout = inactive;
    if( playmode == SCE_DEMO_PLAYMODE_ATTRACT )
    {
        g_DemoMode = 1;
    }
    else
    {
        g_DemoMode = 2;
    }
#endif
    sceCdMmode(SCECdDVD);

    // Do low-level init
    // 02/14/02 - x_Init now performed by the threading system
    //x_Init();
    //TIME_Init();

    // Update PS2 fileio with "correct" prefix and mode

    //
    // Copy argv[0] for future reference
    //
    if (argc)
    {
#ifdef TARGET_DEV
        // Dev target will have the elf name from the local path.
        // if you want to test a PAL build, you can enter the expected name on the 
        // command line param edit box in the Load Elf File dialog of the debugger.
        // Ex: "cdrom0:\SLES_525.70;1" is a European territory title.
        if (argc >= 2)
        {
            x_strcpy( s_CopyOfBootFilename, argv[1] );
        }
        else
#endif
        {
            x_strcpy( s_CopyOfBootFilename, argv[0] );
        }
        // Determine the Territory code based on the start of the name.
        const char* ptr = x_strstr(s_CopyOfBootFilename, "SLUS");

        if ( ptr )
        {
            x_strcpy((char *)s_ProductKey, "BASLUS" );
            x_SetTerritory( XL_TERRITORY_AMERICA );
            s_PalMode = FALSE;
        }
        else
        {
            ptr = x_strstr(s_CopyOfBootFilename, "SLES");
            if ( ptr )
            {
                x_strcpy((char *)s_ProductKey, "BESLES" );
                x_SetTerritory( XL_TERRITORY_EUROPE );
                s_PalMode = TRUE;
            }
        }

        // Now, scan through the name to find the beginning of the product
        // code. It *should* be the first digit after the '_'
        // The name is usually in the form "cdrom0:\SLUS_205.95;1"
        ptr = x_strchr(s_CopyOfBootFilename,'_');

        // Do we even have a product code?
        s_ProductCode = 0;
        if( ptr )
        {
            ptr++; // Skip '_'
            while( *ptr )
            {
                if( *ptr=='.' )
                {
                    ptr++;
                }
                if( x_isdigit(*ptr) == FALSE )
                {
                    break;
                }
                s_ProductCode = s_ProductCode * 10 + (*ptr-'0');
                ptr++;
            }
        }
        if( s_ProductCode == 0 )
        {
            s_ProductCode = 20595;  
        }

    }
    else
    {
        x_strcpy( s_CopyOfBootFilename,"<Undefined>");
    }


    // Init iop
	g_IopManager.Init();

#if defined( EXCEPTION_HANDLER )
    except_Init();
#endif


    //
    // Let us know if we are running on cdrom
    //
    //if( CDROM )
//        PS2_SetFileMode(2,NULL);
}

//=============================================================================

void ps2eng_End( void )
{
	g_IopManager.Kill();
    // Do low-level kill
    //TIME_Kill();
    x_Kill();
    
    #ifdef VENDOR_MW
	mwExit();    /* Clean up, destroy constructed global objects */
    #endif
#ifdef INCLUDE_DEMO_CYCLE
    sceDemoEnd(SCE_DEMO_ENDREASON_PLAYABLE_QUIT);
#endif
}

//=============================================================================

void eng_SetViewport( const view& View )
{
    (void)View;
}

//=============================================================================

void eng_EnableScreenClear( xbool Enabled )
{
    s_PageFlipScreenClear = Enabled;
}

//=============================================================================

void eng_ClearFrontBuffer( u32 Mask )
{
    s16 FrontBuffer = !(s_FrameCount & 1) ? s_pAAbuff->dispFBP1 : s_pAAbuff->dispFBP0;
    
    ClearVRAM( FrontBuffer,
               VRAM_FRAME_BUFFER_WIDTH,
               VRAM_FRAME_BUFFER_HEIGHT,
               xcolor( 0, 0, 0, 0 ),
               Mask );
}

//==============================================================================

void eng_WriteToBackBuffer( u32 Mask )
{
    s16 BackBuffer = (s_FrameCount & 1) ? s_pAAbuff->dispFBP1 : s_pAAbuff->dispFBP0;
    
    gsreg_Begin( 1 );
    gsreg_Set( SCE_GS_FRAME_1, SCE_GS_SET_FRAME( BackBuffer, VRAM_FRAME_BUFFER_WIDTH / 64, SCE_GS_PSMCT32, ~Mask ) );
    gsreg_End();
}

//==============================================================================

s32 eng_GetProductCode( void )
{
    return s_ProductCode;
}

//==============================================================================

const char* eng_GetProductKey( void )
{
   return s_ProductKey;
}

//==============================================================================
// PS2 reboot, for now, just goes to the network configuration tool. Eventually,
// this may be used to launch additional external tools. The XBOX version uses
// this mechanism to launch various aspects of the dashboard.
//
void eng_Reboot( reboot_reason Reason )
{
    static char* s_ProgramName[]={ s_CopyOfBootFilename,"",""};

    (void)Reason;
    //
    // Reset the graphics system.
    //
#if defined(INCLUDE_DEMO_CYCLE)
    s32     Handle;
    char    Buffer[512];
    char*   pName;
    char    Filename[64];
    char*   pFilename;

    Handle = sceOpen("cdrom0:\\SYSTEM.CNF;1",SCE_RDONLY);
    sceRead(Handle,Buffer,sizeof(Buffer));
    pName = x_strstr(Buffer,"cdrom0");
    ASSERT(pName);
    pFilename = Filename;
    while( TRUE )
    {
        if( *pName <' ' )
        {
            break;
        }
        *pFilename++ = *pName++;
    }
    *pFilename++=0x0;
#endif

    ps2_ResetHardware();                // Kill graphics

    g_IopManager.Kill();                // Kill the IOP manager

#if defined(TARGET_DEV)
    // On a devkit, let's set our directory so the configuration tool can
    // find all it's files.
    sceOpen(xfs("host:SETROOT:%s/3rdparty/ps2/Sony/Bin",XCORE_PATH),0,0);
#endif
    //
    // Reset initial FS stuff
    //
    sceSifInitRpc(0);
    sceCdInit(SCECdINIT);
    sceFsReset();
    sceCdDiskReady(0);
    //
    // Launch external tool
    //

    if( Reason == REBOOT_HALT )
    {
        while(TRUE);
    }
#if defined(INCLUDE_DEMO_CYCLE)
    sceDemoEnd( SCE_DEMO_ENDREASON_PLAYABLE_GAMEPLAY_TIMEOUT );
#endif
#if defined(TARGET_DEV)
//    LoadExecPS2("host:netgui/host_uc.elf",sizeof(s_ProgramName)/sizeof(char*),s_ProgramName);
    LoadExecPS2("host:NETGUI/CDVD_UC.ELF",sizeof(s_ProgramName)/sizeof(char*),s_ProgramName);
#else
    // U.S. product code
    if( eng_GetProductCode()==20595 )
    {
        LoadExecPS2("cdrom0:\\NETGUI\\CDVD_UC.ELF",sizeof(s_ProgramName)/sizeof(char*),s_ProgramName);
    }
    else
    {
        LoadExecPS2("cdrom0:\\NETGUI\\CDVD_EU.ELF",sizeof(s_ProgramName)/sizeof(char*),s_ProgramName);
    }
#endif
    ASSERT(FALSE);
}


//==============================================================================

static inline u8 PS2_BCDToDec( u8 Src )
{
    return ((Src & 0xF0) >> 4) * 10 + (Src & 0x0F);
}

//==============================================================================

static inline u8 PS2_DecToBCD( u8 Src )
{
    return ((Src/10) << 4) + (Src % 10);
}

//==============================================================================

datestamp eng_GetDate( void )
{
    sceCdCLOCK      rtc;
    SceMc2DateParam MCFormat;
    datestamp       DateStamp;

    ASSERT( sizeof(DateStamp) == sizeof(rtc) );

    // check for error
    if( sceCdReadClock( &rtc ) == 0 )
        return 0;

    // check for dead battery
    if( rtc.stat )
        return 0;

    sceScfGetLocalTimefromRTC( &rtc );

    MCFormat.year   = PS2_BCDToDec(rtc.year);
    MCFormat.month  = PS2_BCDToDec(rtc.month);
    MCFormat.hour   = PS2_BCDToDec(rtc.hour);
    MCFormat.min    = PS2_BCDToDec(rtc.minute);
    MCFormat.sec    = PS2_BCDToDec(rtc.second);
    MCFormat.day    = PS2_BCDToDec(rtc.day);

    ASSERT( sizeof(DateStamp) == sizeof(MCFormat) );
    x_memcpy( &DateStamp, &MCFormat, sizeof(MCFormat) );

    return DateStamp;
}

//==============================================================================

split_date eng_SplitDate( datestamp DateStamp )
{
    SceMc2DateParam Time;
    split_date  SplitDate;

    ASSERT( sizeof(DateStamp) == sizeof(Time) );
    x_memcpy( &Time, &DateStamp, sizeof(DateStamp) );

    SplitDate.Year      = Time.year;
    SplitDate.Month     = Time.month;
    SplitDate.Day       = Time.day;
    SplitDate.Hour      = Time.hour;
    SplitDate.Minute    = Time.min;
    SplitDate.Second    = Time.sec;
    return SplitDate;

}

//==============================================================================

split_date eng_SplitJSTDate( u64 JSTStamp )
{
    SceMc2DateParam MCFormat;
    sceCdCLOCK      rtc;
    split_date      SplitDate;

    ASSERT( sizeof(JSTStamp) == sizeof(SceMc2DateParam) );
    x_memcpy( &MCFormat, &JSTStamp, sizeof(JSTStamp) );

    // convert the sceMc2DateParam back into BCD clock param format
    rtc.day     = PS2_DecToBCD( MCFormat.day   );
    rtc.month   = PS2_DecToBCD( MCFormat.month );
    rtc.year    = PS2_DecToBCD( MCFormat.year % 100 );  // only use last two digits
    rtc.hour    = PS2_DecToBCD( MCFormat.hour  );
    rtc.minute  = PS2_DecToBCD( MCFormat.min   );
    rtc.second  = PS2_DecToBCD( MCFormat.sec   );

    // convert to local time
    sceScfGetLocalTimefromRTC( &rtc );

    // convert clock format back to decimal and split date 
    SplitDate.Year   = PS2_BCDToDec( rtc.year   );
    SplitDate.Month  = PS2_BCDToDec( rtc.month  );
    SplitDate.Day    = PS2_BCDToDec( rtc.day    );
    SplitDate.Hour   = PS2_BCDToDec( rtc.hour   );
    SplitDate.Minute = PS2_BCDToDec( rtc.minute );
    SplitDate.Second = PS2_BCDToDec( rtc.second );

    return SplitDate;
}

