
///////////////////////////////////////////////////////////////////////////
// INCLUDES 
///////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "e_Engine.hpp"
#include "x_threads.hpp"

//#define TEST_KILL_D3D

///////////////////////////////////////////////////////////////////////////
// INCLUDE ADITIONAL LIBRARIES
///////////////////////////////////////////////////////////////////////////

#pragma comment( lib, "winmm" )

// Auto include DirectX libs in a .NET build
#if _MSC_VER >= 1300
#ifdef CONFIG_DEBUG
#pragma comment( lib, "dsound.lib" )
#pragma comment( lib, "d3dx9.lib" )
#pragma comment( lib, "d3d9.lib" )
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "dinput8.lib" )
#else
#pragma comment( lib, "dsound.lib" )
#pragma comment( lib, "d3dx9.lib" )
#pragma comment( lib, "d3d9.lib" )
#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "dinput8.lib" )
#endif
#endif

///////////////////////////////////////////////////////////////////////////
// MAKE SUER THAT WE HAVE THE RIGHT VERSION
///////////////////////////////////////////////////////////////////////////

#ifndef DIRECT3D_VERSION
#error ------- COMPILING VERSION ? OF D3D ------- 
#endif

#if(DIRECT3D_VERSION < 0x0600)
#error ------- COMPILING VERSION 5 OF D3D ------- 
#endif

#if(DIRECT3D_VERSION < 0x0700)
#error ------- COMPILING VERSION 6 OF D3D ------- 
#endif

#if(DIRECT3D_VERSION < 0x0800)
#error ------- COMPILING VERSION 7 OF D3D ------- 
#endif

#if(DIRECT3D_VERSION < 0x0900)
#error ------- COMPILING VERSION 8 OF D3D ------- 
#endif

///////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////

#define ENG_FONT_SIZEX      7
#define ENG_FONT_SIZEY      12
#define SCRACH_MEM_SIZE     (2*1024*1024)

///////////////////////////////////////////////////////////////////////////
// EXTERNAL DEPENDENCIES
///////////////////////////////////////////////////////////////////////////

xbool d3deng_InitInput  ( HWND Window );

void  vram_Init         ( void );
void  vram_Kill         ( void );

void  smem_Init         ( s32 NBytes );
void  smem_Kill         ( void );

void  draw_Init         ( void );
void  draw_Kill         ( void );

///////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
///////////////////////////////////////////////////////////////////////////

IDirect3DDevice9*       g_pd3dDevice = NULL;
D3DPRESENT_PARAMETERS   g_d3dpp; 

s32 rstct;

///////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
///////////////////////////////////////////////////////////////////////////

static struct eng_locals
{
    eng_locals( void ) { memset( this, 0, sizeof(eng_locals) ); }

    //
    // general variables
    //
    HINSTANCE       hInst; 
    HWND            Wnd;  
    HWND            WndParent;     
    HWND            WndDisplay;  
    IDirect3D9*     pD3D;
    LPD3DXFONT      pFont;
    xcolor          TextColor;
    u32             Mode;
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
    // Input related variables
    //
    MSG             Msg;
    s32             LastPressedKey;
    f32             ABSMouseX;
    f32             ABSMouseY;
    f32             MouseX;
    f32             MouseY;
    xbool           bMouseLeftButton;
    xbool           bMouseRightButton;
    xbool           bMouseMiddleButton;
    f32             MouseWheelAbs;
    f32             MouseWheelRel;
    xbool           bMouseDelta;
    mouse_mode      MouseMode;

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
static
void ConvertComandLine( s32* pargc, char* argv[], LPSTR lpCmdLine )
{
    s32 argc = 1;

    if( *lpCmdLine )
    {
        argv[1] = lpCmdLine;
        argc = 2;

        do
        {
            if( *lpCmdLine == ' ' )
            {
                do
                {
                    *lpCmdLine = 0;
                    lpCmdLine++;

                } while( *lpCmdLine == ' ' );

                if( *lpCmdLine == 0 ) break;
                argv[argc++] = lpCmdLine;
            }

            lpCmdLine++;

        } while( *lpCmdLine );
    }

    *pargc = argc;
}

//=========================================================================

void d3deng_EntryPoint( s32& argc, char**& argv, HINSTANCE h1, HINSTANCE h2, LPSTR str1, INT i1 )
{
    static char* ArgvBuff[256] = { NULL };
    argv = ArgvBuff;
    ConvertComandLine( &argc, argv, str1 );
    s.hInst = h1;

    s.FPSFrameTime[0] = 0;
    s.FPSFrameTime[1] = 0;
    s.FPSFrameTime[2] = 0;
    s.FPSFrameTime[3] = 0;
    s.FPSFrameTime[4] = 0;
    s.FPSFrameTime[5] = 0;
    s.FPSFrameTime[6] = 0;
    s.FPSFrameTime[7] = 0;

    s.FPSLastTime = x_GetTime();

    x_Init(argc,argv);
}

//=========================================================================

s32 eng_GetProductCode( void )
{
    return 0;
}

//=============================================================================

const char* eng_GetProductKey(void)
{
    return NULL;
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

    //
    // Stop the d3d engine
    //
    if( g_pd3dDevice != NULL) 
        g_pd3dDevice->Release();

    if( s.pD3D != NULL)
        s.pD3D->Release();
    
    if( s.pFont )
        s.pFont->Release();

    UnregisterClass( "Render Window", s.hInst );
}

//=========================================================================

s32 d3deng_ExitPoint( void )
{
//  eng_Kill();

    x_Kill();
    return 0;
}

//=========================================================================

void d3deng_SetPresets( u32 Mode )
{
    s.Mode = Mode;
}

//=============================================================================

u32 d3deng_GetMode( void )
{
    return s.Mode ;
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

//=========================================================================
static
void InitializeD3D( HWND hWnd, s32 XRes, s32 YRes )
{
    dxerr Error;

    //
    // Create the D3D object, which is needed to create the D3DDevice.
    //
    s.pD3D = Direct3DCreate9( D3D_SDK_VERSION );
    ASSERT( s.pD3D );

    //
    // Create our rendering device
    // 
    ZeroMemory( &g_d3dpp, sizeof(g_d3dpp) );

    if( (s.Mode & ENG_ACT_STENCILOFF) == 0 )
    {
        ASSERT( (s.Mode&ENG_ACT_16_BPP) == 0 );
        g_d3dpp.EnableAutoDepthStencil    = TRUE;
        g_d3dpp.AutoDepthStencilFormat    = D3DFMT_D24S8;
    }                

    s.bWindowed = (s.Mode & ENG_ACT_FULLSCREEN) != ENG_ACT_FULLSCREEN;
    if( s.bWindowed )
    {
        D3DDISPLAYMODE d3ddm;

        // TODO: CJ: We really need to enumerate the adapters at this point and choose the
        // appropriate one if one exists

        // Get the desktop window info, this will fail on Remote Desktop
        Error = s.pD3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm );
        if( Error != 0 )
            return;

        // we want alpha
        if( d3ddm.Format == D3DFMT_X8R8G8B8 )
            d3ddm.Format = D3DFMT_A8R8G8B8;
        else if( d3ddm.Format == D3DFMT_X4R4G4B4 )
            d3ddm.Format = D3DFMT_A4R4G4B4;

        // fill the struct
        g_d3dpp.Windowed                  = TRUE;
        g_d3dpp.SwapEffect                = D3DSWAPEFFECT_COPY; //DISCARD;
        g_d3dpp.BackBufferFormat          = d3ddm.Format;
        g_d3dpp.BackBufferCount           = 1;
        g_d3dpp.PresentationInterval      = D3DPRESENT_INTERVAL_DEFAULT;

        // Set the maximun allow size for a d3d window
        g_d3dpp.BackBufferWidth           = s.MaxXRes;
        g_d3dpp.BackBufferHeight          = s.MaxYRes;
    }
    else
    {
        g_d3dpp.Windowed                  = FALSE;
        g_d3dpp.BackBufferWidth           = s.MaxXRes = XRes;
        g_d3dpp.BackBufferHeight          = s.MaxYRes = YRes;
        g_d3dpp.BackBufferFormat          = ( s.Mode & ENG_ACT_16_BPP ) ? D3DFMT_X1R5G5B5:D3DFMT_A8R8G8B8;
        g_d3dpp.SwapEffect                = D3DSWAPEFFECT_COPY;
    }

    s.Mode |= ENG_ACT_BACKBUFFER_LOCK; //So far so

    if( s.Mode & ENG_ACT_BACKBUFFER_LOCK )
    {
        g_d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    }

    // Lookup device caps
    D3DCAPS9 Caps ;
    Error = s.pD3D->GetDeviceCaps(D3DADAPTER_DEFAULT,
                                  D3DDEVTYPE_HAL,
                                  &Caps) ;

    // Force software shaders if we can't do at least 1.1 vertex shader
    if ((Caps.VertexShaderVersion & 0xFFFF) < 0x0101)
        s.Mode |= ENG_ACT_SHADERS_IN_SOFTWARE ;

    // Force software shaders if using a software device
    if (s.Mode & s.Mode & ENG_ACT_SOFTWARE)
        s.Mode |= ENG_ACT_SHADERS_IN_SOFTWARE ;

    // Setup flags
    DWORD dwFlags = 0 ;
    if (s.Mode & ENG_ACT_SHADERS_IN_SOFTWARE)
        dwFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING ;
    else
        dwFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING ;

    // Create the device
    Error = s.pD3D->CreateDevice(   
        D3DADAPTER_DEFAULT,    
        //(s.Mode & ENG_ACT_SOFTWARE) ? D3DDEVTYPE_REF :
        //D3DDEVTYPE_REF, 
        D3DDEVTYPE_HAL, 
        hWnd,
        //((s.Mode & ENG_ACT_SHADERS_IN_SOFTWARE) || (s.Mode & ENG_ACT_SOFTWARE) ) ?
        //     D3DCREATE_SOFTWARE_VERTEXPROCESSING : D3DCREATE_HARDWARE_VERTEXPROCESSING,
        dwFlags,
        &g_d3dpp,
        &g_pd3dDevice );

    //
    //
    // 
    #ifdef TEST_KILL_D3D
        g_pd3dDevice = NULL;
    #endif
    //
    //
    //

    // If there was an error should we bail?
    if(Error != 0)
    {
        MessageBox(d3deng_GetWindowHandle(), xfs("Error creating device: %d", Error), "Device Error", MB_OK);
        g_pd3dDevice = NULL;
    }
    //ASSERT( Error == 0 );

    //TODO: DONT FORGET INITIALIZE SHADERS HERE!!!!!!!!!!!!

    if( g_pd3dDevice )
    {
        // Turn on software processing
        if (s.Mode & ENG_ACT_SHADERS_IN_SOFTWARE)
            g_pd3dDevice->SetSoftwareVertexProcessing( TRUE ) ;

        //
        // If we are doing a window mode set the viewport to show 
        // only the portion that we are interested
        //
        if( s.bWindowed )
        {
            D3DVIEWPORT9 vp = { 0, 0, XRes, YRes, 0.0f, 1.0f };
            g_pd3dDevice->SetViewport( &vp );         
            RECT Window;

            GetWindowRect   ( s.Wnd, &Window );
            GetClientRect   ( s.Wnd, &s.rcWindowRect         );
            s32 W = (Window.right   - Window.left)*2 - (s.rcWindowRect.right  - s.rcWindowRect.left) ;
            s32 H = (Window.bottom  - Window.top)*2  - (s.rcWindowRect.bottom - s.rcWindowRect.top ) ;
            MoveWindow      ( s.Wnd,  0, 0, W, H, true );

            GetClientRect   ( s.Wnd, &s.rcWindowRect );

            // Make sure that it becames the active window
            ShowWindow      ( s.Wnd, SW_SHOWNORMAL );
            UpdateWindow    ( s.Wnd );
            SetActiveWindow ( s.Wnd );
        }
    }    
}

//=========================================================================

void d3deng_UpdateDisplayWindow( HWND hWindow )
{
    s.WndDisplay = hWindow;

    //
    // Get the new size of the window
    //
    GetClientRect( s.WndDisplay, &s.rcWindowRect );

    //
    // Resize the view port
    //
    DWORD dwRenderWidth  = s.rcWindowRect.right - s.rcWindowRect.left;
    DWORD dwRenderHeight = s.rcWindowRect.bottom - s.rcWindowRect.top;

    dwRenderWidth  = MIN( (DWORD)s.MaxXRes, dwRenderWidth );
    dwRenderHeight = MIN( (DWORD)s.MaxYRes, dwRenderHeight );

    D3DVIEWPORT9 vp = { 0, 0, dwRenderWidth, dwRenderHeight, 0.0f, 1.0f };
    if( g_pd3dDevice )
        g_pd3dDevice->SetViewport( &vp );
}

//=============================================================================

LRESULT CALLBACK eng_D3DWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg )
    {
        case WM_ACTIVATE:
            {
                s.bActive = LOWORD(wParam) != WA_INACTIVE;
                if( s.bActive )
                {
                    d3deng_ComputeMousePos();
                    d3deng_ComputeMousePos();
                }
            }
            break;

        case WM_SIZE:
            // Check to see if we are losing or gaining our window. Set the
            // active flag to match.
            if( SIZE_MAXHIDE==wParam || SIZE_MINIMIZED==wParam )
                s.bActive = FALSE;
            else s.bActive = TRUE;

            // A new window size will require a new backbuffer size. The
            // easiest way to achieve this is to release and re-create
            // everything. Note: if the window gets too big, we may run out
            // of video memory and need to exit. This simple app exits
            // without displaying error messages, but a real app would behave
            // itself much better.
            if( s.bActive && s.bReady )
            {
                d3deng_UpdateDisplayWindow( hWnd );
                s.bReady = TRUE;
            }
            break;

        case WM_GETMINMAXINFO:
            ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 100;
            ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 100;
            break;

        case WM_SETCURSOR:
            // We turn the cursor off for fullscreen modes
            if( s.bActive && s.bReady && (!s.bWindowed) )
            {
                SetCursor(NULL);
                return TRUE;
            }
            break;

        case WM_CLOSE:
            DestroyWindow( hWnd );
            return 0;
        
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0L;

        case WM_MOUSEMOVE:
            s.ABSMouseX = s.MouseX = LOWORD(lParam);  // horizontal position of cursor 
            s.ABSMouseY = s.MouseY = HIWORD(lParam);  // vertical position of cursor 

            // Compute the abs position of the mouse
            {
                s32 w = (s.rcWindowRect.right-s.rcWindowRect.left);
                s32 h = (s.rcWindowRect.bottom-s.rcWindowRect.top);
                if( w > s.MaxXRes )
                {
                    s.ABSMouseX = (s.ABSMouseX*s.MaxXRes)/w;
                }

                if( h > s.MaxYRes )
                {
                    s.ABSMouseY = (s.ABSMouseY*s.MaxYRes)/h;
                }
            }
            return 0;
         case WM_LBUTTONDOWN:
            s.bMouseLeftButton = true;
            return 0;
         case WM_MBUTTONDOWN:
             s.bMouseMiddleButton = true;
            return 0;
         case WM_RBUTTONDOWN:
             s.bMouseRightButton = true;
            return 0;
         case WM_LBUTTONUP:
             s.bMouseLeftButton = false;
            return 0;
         case WM_MBUTTONUP:
             s.bMouseMiddleButton = false;
            return 0;
         case WM_RBUTTONUP:
             s.bMouseRightButton = false;
            return 0;
         case WM_MOUSEWHEEL:
            {
                 f32 Wheel = (f32)((s16)(wParam >> 16)) / 120.0f ;
                 s.MouseWheelAbs += Wheel ;
                 s.MouseWheelRel = Wheel ;
            }
            return 0;
         case WM_KEYDOWN:
             break;
    }

    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

//=========================================================================
static
HWND CreateWin( s32 Width, s32 Height )
{
    // Register the window class
    WNDCLASS wndClass = { CS_HREDRAW | CS_VREDRAW, eng_D3DWndProc, 0, 0, s.hInst,
                          NULL,
                          LoadCursor(NULL, IDC_ARROW), 
                          (HBRUSH)GetStockObject(WHITE_BRUSH), NULL,
                          TEXT("Render Window") };

    RegisterClass( &wndClass );

    // Create our main window
    return CreateWindow( TEXT("Render Window"),
                         TEXT("Area 51"),
                         WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                         CW_USEDEFAULT, Width, Height, 0L, 0L, s.hInst, 0L );
}

//=========================================================================

#ifndef X_RETAIL

void text_BeginRender( void )
{
    //
    // Make sure that the direct 3d has started
    //
    if( s.bD3DBeginScene == FALSE )
    {
        if( eng_Begin() )
        {
            eng_End();
        }
    }
}

#endif // X_RETAIL

//=========================================================================

#ifndef X_RETAIL

void text_RenderStr( char* pStr, s32 NChars, xcolor Color, s32 PixelX, s32 PixelY )
{
    dxerr                   Error;
    RECT                    Rect;

    if( !g_pd3dDevice || !s.pFont )
        return;

    Rect.left   = PixelX;
    Rect.top    = PixelY;
    Rect.bottom = PixelY + ENG_FONT_SIZEY;
    Rect.right  = Rect.left + (NChars*ENG_FONT_SIZEX);
                    
    //rstct+=NChars;
    Error = s.pFont->DrawText( NULL, pStr, NChars, &Rect, DT_NOCLIP, Color );//s.TextColor );
    if(Error != D3D_OK) rstct = Error;
}

#endif // X_RETAIL
//=========================================================================

#ifndef X_RETAIL

void text_EndRender( void )
{
}

#endif // X_RETAIL

//=========================================================================

void ENG_TextColor( const xcolor& Color )
{
    s.TextColor = Color;
}

//=========================================================================

void d3deng_SetWindowHandle( HWND hWindow )
{
    s.Wnd = hWindow;
}

//=========================================================================

void d3deng_SetParentHandle( HWND hWindow )
{
    s.WndParent = hWindow;
}

//=========================================================================

void d3deng_SetResolution( s32 Width,  s32 Height )
{
    ASSERT( Width  > 0 );
    ASSERT( Width  < 40000 );
    ASSERT( Height > 0 );
    ASSERT( Height < 40000 );

    s.MaxXRes  = Width;
    s.MaxYRes  = Height;
}

//=========================================================================

void d3deng_SetDefaultStates( void )
{
    if( g_pd3dDevice )
    {
        // Set dither in case the use selects 16bits
        g_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE, TRUE );

        // Make sure that we are dealing with the Z buffer
        g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );
        g_pd3dDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );

        // Set some color conbiner functions
        g_pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );

        // Right handed mode
        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );

        // We can do alpha from the begining
        g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
        g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
        g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

        // Turn off the stupid specular lighting
        g_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );

        // Set bilinear mode
        g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR  );
        g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR  );

        g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR  );
        g_pd3dDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR  );

        g_pd3dDevice->SetSamplerState( 2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR  );
        g_pd3dDevice->SetSamplerState( 2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR  );

        g_pd3dDevice->SetSamplerState( 3, D3DSAMP_MINFILTER, D3DTEXF_LINEAR  );
        g_pd3dDevice->SetSamplerState( 3, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR  );

        // Turn on the trilinear blending
        g_pd3dDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR  );

        // Set the Initial wraping behavier
        g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
        g_pd3dDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );

        // Set the ambient light
        d3deng_SetAmbientLight( xcolor( 0xfff8f8f8 ) );

        // Make sure that we turng off the lighting as the default
        g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
    }
}

//=========================================================================

void d3deng_CreateFont( void )
{
    // Get height in appropriate units
    HDC hDC = GetDC( NULL );
    int nHeight = -( MulDiv( ENG_FONT_SIZEY, GetDeviceCaps(hDC, LOGPIXELSY), 72 ) );
    ReleaseDC( NULL, hDC );

    HRESULT Error = D3DXCreateFont( g_pd3dDevice, nHeight * 3 / 4, 0, FW_NORMAL, 0, FALSE, 
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                DEFAULT_PITCH | FF_DONTCARE, TEXT("Lucida Console"), 
                                &s.pFont );

    ASSERT( Error == 0 );
}

//=========================================================================

void eng_Init( void )
{
    if( s.MaxXRes == 0 )
    {
        s.MaxXRes = 1024; //512;
        s.MaxYRes = 768; //512;
    }

    //
    // Initialize the engine
    //
    if( s.Wnd == 0 )
        s.Wnd = CreateWin( s.MaxXRes, s.MaxYRes );

    // The default window to display will be the current window
    s.WndDisplay = s.Wnd;

    InitializeD3D( s.Wnd, s.MaxXRes, s.MaxYRes );

    //
    // Show the window
    //
    ShowWindow( s.Wnd, SW_SHOWDEFAULT );
    UpdateWindow( s.Wnd );

    //
    // Init text
    //
    text_Init();
    text_SetParams( s.MaxXRes, s.MaxYRes, 0, 0,
                    ENG_FONT_SIZEX, ENG_FONT_SIZEY,
                    8 );
    text_ClearBuffers();


    //
    // load the font
    //
    if( g_pd3dDevice )
    {
        d3deng_CreateFont();

        ENG_TextColor( 0xffffffff ); // xcolor(255,255,125,255)
    }

    //
    // Initialize dinput
    //
    if( s.WndParent )
    {
        VERIFY( d3deng_InitInput( s.WndParent ) == TRUE );
    }
    else
    {
        VERIFY( d3deng_InitInput( s.Wnd ) == TRUE );
    }

    //
    // Initialize vram
    //
    vram_Init();

    //
    // Set the scrach memory
    //
    smem_Init(SCRACH_MEM_SIZE);

    //
    // Initialize draw
    //
    draw_Init();

    //
    // Indicate teh the engine is ready
    //
    s.bReady = TRUE;

    //
    // Set default modes
    //
    d3deng_SetDefaultStates();
}

//=============================================================================

static const char* pPrevName = NULL;

xbool eng_Begin( const char* pTaskName )
{
    ASSERT( s.bBeginScene == FALSE );
    pPrevName = pTaskName;

    // Exit if no device
    if( !g_pd3dDevice )
        return FALSE;

    if( s.bD3DBeginScene == FALSE )
    {
        // Check the cooperative level before rendering
        if( FAILED( g_pd3dDevice->TestCooperativeLevel() ) )
            return FALSE;

        if( FAILED( g_pd3dDevice->BeginScene() ) ) 
            return FALSE;

        // the user may have set the viewport before beginning rendering. in that
        // case we need to save the original viewport out
        dxerr Error;
        D3DVIEWPORT9 OldVP;
        Error = g_pd3dDevice->GetViewport( &OldVP );
        ASSERT( Error == 0 );

        //
        // Clear the backgournd
        // This must be here rather than in the page flip because of editors.
        // Plus it should be allot faster as well. Right thing to do. 
        //
        s32 Width  = MIN( s.MaxXRes, s.rcWindowRect.right-s.rcWindowRect.left );
        s32 Height = MIN( s.MaxYRes, s.rcWindowRect.bottom-s.rcWindowRect.top );
        D3DVIEWPORT9 vp = { 0, 0, Width, Height, 0.0f, 1.0f };
        Error = g_pd3dDevice->SetViewport( &vp );
//        ASSERT( Error == 0 );

        // Clear the back-ground
        Error = g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_STENCIL | D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, s.BackColor, 1.0f, 0L ); 
        ASSERT( Error == 0 );

        // Restore the original viewport
        Error = g_pd3dDevice->SetViewport( &OldVP );
        ASSERT( Error == 0 );
    }

    //
    // Mark the d3dbeginscene as active
    //
    s.bD3DBeginScene = TRUE;
    s.bBeginScene = TRUE;

    // Clear draw's L2W
    draw_ClearL2W();

    // Begun!
    return TRUE;
}

//=============================================================================

void eng_End( void )
{
//    ASSERT( s.bBeginScene );
    s.bBeginScene = FALSE;
}

//=============================================================================

xbool eng_InBeginEnd( void )
{
    return s.bBeginScene;
}

//=============================================================================

void eng_PrintStats( void )
{
    x_DebugMsg("CPU:%1.1f  Pageflip:%1.1f  FPS:%1.1f\n",s.CPUMS,s.IMS,1000.0f/(s.CPUMS+s.IMS));
}

//=============================================================================

bool g_bDeviceLost = false;

void eng_PageFlip()
{
    HRESULT hr;

    CONTEXT( "eng_PageFlip" );

    // Tickle screensaver
    SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, FALSE, 0, FALSE );
    SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, TRUE, 0, FALSE );

    // Check for lost device
    if( g_bDeviceLost == true )
    {
        // Force a redraw of the D3D window
        InvalidateRect( s.WndDisplay, NULL, FALSE );

        // Yield some CPU time to other processes
        Sleep( 100 ); // 100 milliseconds

        // Test the cooperative level to see if it's okay to render.
        if( FAILED( hr = g_pd3dDevice->TestCooperativeLevel() ) )
        {
            // The device has been lost but cannot be reset at this time. 
            // Therefore, rendering is not possible and we'll have to return 
            // and try again at a later time.
            if( hr == D3DERR_DEVICELOST )
                return;

            // The device has been lost but it can be reset at this time. 
            if( hr == D3DERR_DEVICENOTRESET )
            {
                extern void pc_PreResetCubeMap( void );
                pc_PreResetCubeMap();

                // Free the font
                if( s.pFont )
                {
                    try
                    {
                        s.pFont->Release();
                    }
                    catch(...)
                    {
                        // Ignore release errors - device is already lost
                    }
                    s.pFont = NULL;
                }

                // Reset the device
                hr = g_pd3dDevice->Reset( &g_d3dpp );
                if( FAILED(hr ) )
                    return;
            }

            return;
        }

        // Recovered - It's a miracle
        g_bDeviceLost = false;

        // Get our default states back
        d3deng_SetDefaultStates();

        // Create the cube map texture again
        extern void pc_PostResetCubeMap( void );
        pc_PostResetCubeMap();

        // Create the font again
        if(s.pFont == NULL)
        {
            d3deng_CreateFont();
        }
    }


    xtimer ARHTimer;
    ARHTimer.Reset();
    s.CPUTIMER.Stop();
    s.CPUMS = s.CPUTIMER.ReadMs();
    xtimer InternalTime;
    InternalTime.Start();

    const xbool bPrintFPS = FALSE;

    //
    // If the user hasn't called eng_Begin then force a call, we need at least 1 Begin End pair
    //
    if( s.bD3DBeginScene == FALSE )
    {
        if( eng_Begin() )
        {
            eng_End();
        }
    }

    //
    // Handle all the buffered text
    //
    rstct=0;
    ARHTimer.Start();
    
    // Check if we have a valid font before rendering text
    if(g_pd3dDevice && s.pFont)
    {
        text_Render();
    }
    
    ARHTimer.Stop();
    text_ClearBuffers();
    //x_printfxy(0, 43, "A1: %7.3f", ARHTimer.ReadMs());
    //x_printfxy(0, 44, "rst: %7.3f %d", rst.ReadMs(),rstct);
    //
    // Handle the FPS
    //
    if( bPrintFPS ) 
        x_printfxy( 0,0, "FPS: %4.2f", eng_GetFPS() );

    //
    // End the scene. and page flip
    //
    if( g_pd3dDevice && s.bD3DBeginScene )
    {
        g_pd3dDevice->EndScene();
        s.bD3DBeginScene = FALSE;
    }

    if( s.bWindowed )
    {
        RECT SrcRect;

        SrcRect.top    = 0;
        SrcRect.left   = 0;
        SrcRect.right  = s.rcWindowRect.right  - s.rcWindowRect.left;
        SrcRect.bottom = s.rcWindowRect.bottom - s.rcWindowRect.top;

        SrcRect.right  = MIN( s.MaxXRes, SrcRect.right  );
        SrcRect.bottom = MIN( s.MaxYRes, SrcRect.bottom );

        if( g_pd3dDevice )
        {
            hr = g_pd3dDevice->Present( &SrcRect, NULL, s.WndDisplay, NULL );
            if( hr == D3DERR_DEVICELOST )
                g_bDeviceLost = true;
        }
    }
    else        
    {
        if( g_pd3dDevice )
        {
            hr = g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
            if( hr == D3DERR_DEVICELOST )
                g_bDeviceLost = true;
        }
    }

    //
    // Taggle the scrach memory
    //
    smem_Toggle();

    //
    // Update fps
    //
    UpdateFPS();

    InternalTime.Stop();
    s.IMS = InternalTime.ReadMs();
    s.CPUTIMER.Reset();
    s.CPUTIMER.Start();
}

//=============================================================================

void eng_ResetAfterException( void )
{
    // Clear the in scene flag
    s.bBeginScene = FALSE;

    if( s.bD3DBeginScene )
    {
        if( g_pd3dDevice )
        {
            ASSERT( s.bD3DBeginScene );
            g_pd3dDevice->EndScene();
            s.bD3DBeginScene = FALSE;
        }

        smem_ResetAfterException();
    }
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
    s.View = View;

    if( g_pd3dDevice )
    {
        g_pd3dDevice->SetTransform( D3DTS_VIEW,       (D3DMATRIX*)&s.View.GetW2V() );
        g_pd3dDevice->SetTransform( D3DTS_PROJECTION, (D3DMATRIX*)&s.View.GetV2C() );
    }
}

//=============================================================================

const view* eng_GetView( void )
{
    return &s.View;
}

//=============================================================================

void eng_ScreenShot( const char* pFileName )
{
    ASSERTS( 0, "Press Print screen for now" );
}

//=============================================================================

#if !defined(X_RETAIL) || defined(X_QA)
xbool eng_ScreenShotActive( void )
{
    return FALSE;
}
#endif  // !defined( X_RETAIL ) || defined( X_QA )

//=============================================================================

void eng_Sync ( void )
{
}

//=============================================================================

void DebugMessage( const char* FormatStr, ... )
{
    va_list Args; 
    va_start( Args, FormatStr );

    OutputDebugString( xvfs( FormatStr, Args) );

    va_end( Args );
}

//=============================================================================

void eng_SetViewport( const view& View )
{
    if( !g_pd3dDevice || g_bDeviceLost )
        return;

    dxerr           Error;
    D3DVIEWPORT9    vp;
    s32             L, T, R, B;

    View.GetViewport( L, T, R, B );

    vp.X      = L;
    vp.Y      = T;
    vp.Width  = R-L-1;
    vp.Height = B-T-1;
    vp.MinZ   = 0.0f;
    vp.MaxZ   = 1.0f;

    vp.Width  = MIN( (u32)s.MaxXRes, vp.Width );
    vp.Height = MIN( (u32)s.MaxYRes, vp.Height );

    Error = g_pd3dDevice->SetViewport( &vp );
    ASSERT( Error == 0 );
}


//=========================================================================
void  d3deng_SetMouseMode( mouse_mode Mode )
{
    s.MouseMode = Mode;
}

//=========================================================================

void d3deng_ComputeMousePos( void )
{
    xbool CheckMouse = FALSE;
    static xbool WasActive = FALSE;
    xbool  LastTimeActive;
    s.MouseX = 0;
    s.MouseY = 0;

    LastTimeActive = WasActive;
    WasActive      = FALSE;

    if( s.bActive == FALSE )
        return;

    if( s.MouseMode == MOUSE_MODE_NEVER )
        return;

    if( s.MouseMode == MOUSE_MODE_BUTTONS )
    {
        if( s.bMouseLeftButton == 0 && s.bMouseRightButton == 0 && s.bMouseMiddleButton == 0 )
            return;
    }

    
    WasActive = TRUE;


    RECT    Rect;
    POINT   MousePos;
    GetCursorPos ( &MousePos );
    GetWindowRect( s.Wnd, &Rect );

    s32 CenterX = (Rect.right + Rect.left) >> 1;
    s32 CenterY = (Rect.bottom + Rect.top) >> 1;

    if(s.MouseMode != MOUSE_MODE_ABSOLUTE)
    {
        SetCursorPos( CenterX, CenterY );
    }

    if( LastTimeActive == FALSE )        
        return;
    if(s.MouseMode != MOUSE_MODE_ABSOLUTE)
    {
        s.MouseX = (f32)(MousePos.x - CenterX);
        s.MouseY = (f32)(MousePos.y - CenterY);
    }
    else
    {
        s.MouseX = (f32)(MousePos.x - Rect.left);
        s.MouseY = (f32)(MousePos.y - Rect.top);
    }
}

//=========================================================================

f32 d3deng_GetABSMouseX( void ) //Legacy code, used ONLY on ArtistViewer.
{
    return s.ABSMouseX;
}

//=========================================================================

f32 d3deng_GetABSMouseY( void )  //Legacy code, used ONLY on ArtistViewer.
{
    return s.ABSMouseY;
}

//=============================================================================

HINSTANCE d3deng_GetInstace( void )
{
    return s.hInst;
}

//=============================================================================
HWND d3deng_GetWindowHandle( void )
{
    return s.Wnd;
}

//=============================================================================

void d3deng_SetLight( s32 LightID, vector3& Dir, xcolor& Color )
{
    D3DLIGHT9 Light;

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

    Color.GetfRGBA(  Light.Diffuse.r, 
                     Light.Diffuse.g,
                     Light.Diffuse.b,
                     Light.Diffuse.a );

    Light.Specular = Light.Diffuse;

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
    // Here we enable out register LightID. That way what ever we render 
    // from now on it will use register LightID. The other registers are by 
    // default turn off.
    //
    g_pd3dDevice->LightEnable( LightID, TRUE );
}

//=============================================================================

void d3deng_SetAmbientLight( xcolor& Color )
{
    D3DMATERIAL9 mtrl;

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
    mtrl.Diffuse.r  = mtrl.Diffuse.g  = mtrl.Diffuse.b  = 1.0f; 
    mtrl.Specular.r = mtrl.Specular.g = mtrl.Specular.b = 0.5f;
    mtrl.Power      = 50;


    // ambient light
    Color.GetfRGBA ( mtrl.Ambient.r,
                     mtrl.Ambient.g,
                     mtrl.Ambient.b,
                     mtrl.Ambient.a );

    if( g_pd3dDevice )
    {
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
        g_pd3dDevice->SetRenderState( D3DRS_AMBIENT, Color );
    }
}

//=============================================================================

void eng_Reboot( reboot_reason Reason )
{
    exit(Reason);
}

//=============================================================================

datestamp eng_GetDate( void )
{
    SYSTEMTIME  Time;
    datestamp   DateStamp;

    ASSERT( sizeof(DateStamp) == sizeof(FILETIME) );

    GetLocalTime( &Time );
    SystemTimeToFileTime( &Time, (FILETIME*)&DateStamp );
    return DateStamp;
}

//=============================================================================

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

//=============================================================================

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
