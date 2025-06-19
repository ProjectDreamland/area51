#ifndef D3DENG_PRIVATE_HPP
#define D3DENG_PRIVATE_HPP

///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

// Included this header only one time
#pragma once
#ifndef STRICT
#define STRICT
#endif
#include "..\3rdParty\DirectX9\d3d9.h"
#include "..\3rdParty\DirectX9\d3dx9.h"
#include "..\3rdParty\DirectX9\d3dx9core.h"
#include "..\3rdParty\DirectX9\dplay8.h"
#include <windows.h>
#include <mmsystem.h>
#include "x_files.hpp"

///////////////////////////////////////////////////////////////////////////
// DEFINE AND ENUMS
///////////////////////////////////////////////////////////////////////////

#define WM_MOUSEWHEEL                   0x020A

// DX ERROR ENUM
#define DXERROR( A ) ENG_##A = (A<0) ? -A : A
enum dxerror_enum
{
    DXERROR( E_OUTOFMEMORY                                  ),
    DXERROR( D3DXERR_INVALIDDATA                            ),
    DXERROR( D3DERR_CONFLICTINGRENDERSTATE                  ),
    DXERROR( D3DERR_CONFLICTINGTEXTUREFILTER                ),
    DXERROR( D3DERR_CONFLICTINGTEXTUREPALETTE               ),
    DXERROR( D3DERR_DEVICELOST                              ),
    DXERROR( D3DERR_DEVICENOTRESET                          ),
    DXERROR( D3DERR_DRIVERINTERNALERROR                     ),
    DXERROR( D3DERR_DRIVERINVALIDCALL                       ),
    DXERROR( D3DERR_INVALIDCALL                             ),
    DXERROR( D3DERR_INVALIDDEVICE                           ),
    DXERROR( D3DERR_MOREDATA                                ),
    DXERROR( D3DERR_NOTAVAILABLE                            ),
    DXERROR( D3DERR_NOTFOUND                                ),
    DXERROR( D3DERR_OUTOFVIDEOMEMORY                        ),
    DXERROR( D3DERR_TOOMANYOPERATIONS                       ),
    DXERROR( D3DERR_UNSUPPORTEDALPHAARG                     ),
    DXERROR( D3DERR_UNSUPPORTEDALPHAOPERATION               ),
    DXERROR( D3DERR_UNSUPPORTEDCOLORARG                     ),
    DXERROR( D3DERR_UNSUPPORTEDCOLOROPERATION               ),
    DXERROR( D3DERR_UNSUPPORTEDFACTORVALUE                  ),
    DXERROR( D3DERR_UNSUPPORTEDTEXTUREFILTER                ),
    DXERROR( D3DERR_WRONGTEXTUREFORMAT                      ),
    DXERROR( D3DXERR_CANNOTATTRSORT                         ),
    DXERROR( D3DXERR_CANNOTMODIFYINDEXBUFFER                ),
    DXERROR( D3DXERR_INVALIDMESH                            ),
    DXERROR( D3DXERR_SKINNINGNOTSUPPORTED                   ),
    DXERROR( D3DXERR_TOOMANYINFLUENCES                      ),
    DXERROR( DPNERR_ABORTED                                 ),
    DXERROR( DPNERR_ADDRESSING                              ),
    //DXERROR( DPNERR_ABORTED                                 ),   
    //DXERROR( DPNERR_ADDRESSING                              ),   
    DXERROR( DPNERR_ALREADYCLOSING                          ),   
    DXERROR( DPNERR_ALREADYCONNECTED                        ),   
    DXERROR( DPNERR_ALREADYDISCONNECTING                    ),   
    DXERROR( DPNERR_ALREADYINITIALIZED                      ),   
    DXERROR( DPNERR_ALREADYREGISTERED                       ),   
    DXERROR( DPNERR_BUFFERTOOSMALL                          ),   
    DXERROR( DPNERR_CANNOTCANCEL                            ),   
    DXERROR( DPNERR_CANTCREATEGROUP                         ),   
    DXERROR( DPNERR_CANTCREATEPLAYER                        ),   
    DXERROR( DPNERR_CANTLAUNCHAPPLICATION                   ),   
    DXERROR( DPNERR_CONNECTING                              ),   
    DXERROR( DPNERR_CONNECTIONLOST                          ),   
    DXERROR( DPNERR_CONVERSION                              ),   
    DXERROR( DPNERR_DOESNOTEXIST                            ),   
    DXERROR( DPNERR_DUPLICATECOMMAND                        ),   
    DXERROR( DPNERR_ENDPOINTNOTRECEIVING                    ),   
    DXERROR( DPNERR_ENUMQUERYTOOLARGE                       ),   
    DXERROR( DPNERR_ENUMRESPONSETOOLARGE                    ),   
    DXERROR( DPNERR_EXCEPTION                               ),   
    DXERROR( DPNERR_GENERIC                                 ),   
    DXERROR( DPNERR_GROUPNOTEMPTY                           ),   
    DXERROR( DPNERR_HOSTING                                 ),   
    DXERROR( DPNERR_HOSTREJECTEDCONNECTION                  ),   
    DXERROR( DPNERR_HOSTTERMINATEDSESSION                   ),   
    DXERROR( DPNERR_INCOMPLETEADDRESS                       ),   
    DXERROR( DPNERR_INVALIDADDRESSFORMAT                    ),   
    DXERROR( DPNERR_INVALIDAPPLICATION                      ),   
    DXERROR( DPNERR_INVALIDCOMMAND                          ),   
    DXERROR( DPNERR_INVALIDDEVICEADDRESS                    ),   
    DXERROR( DPNERR_INVALIDENDPOINT                         ),   
    DXERROR( DPNERR_INVALIDFLAGS                            ),   
    DXERROR( DPNERR_INVALIDGROUP                            ),   
    DXERROR( DPNERR_INVALIDHANDLE                           ),   
    DXERROR( DPNERR_INVALIDHOSTADDRESS                      ),   
    DXERROR( DPNERR_INVALIDINSTANCE                         ),   
    DXERROR( DPNERR_INVALIDINTERFACE                        ),   
    DXERROR( DPNERR_INVALIDOBJECT                           ),   
    DXERROR( DPNERR_INVALIDPARAM                            ),   
    DXERROR( DPNERR_INVALIDPASSWORD                         ),   
    DXERROR( DPNERR_INVALIDPLAYER                           ),   
    DXERROR( DPNERR_INVALIDPOINTER                          ),   
    DXERROR( DPNERR_INVALIDPRIORITY                         ),   
    DXERROR( DPNERR_INVALIDSTRING                           ),   
    DXERROR( DPNERR_INVALIDURL                              ),   
    DXERROR( DPNERR_INVALIDVERSION                          ),   
    DXERROR( DPNERR_NOCAPS                                  ),   
    DXERROR( DPNERR_NOCONNECTION                            ),   
    DXERROR( DPNERR_NOHOSTPLAYER                            ),   
    DXERROR( DPNERR_NOINTERFACE                             ),   
    DXERROR( DPNERR_NOMOREADDRESSCOMPONENTS                 ),   
    DXERROR( DPNERR_NORESPONSE                              ),   
    DXERROR( DPNERR_NOTALLOWED                              ),   
    DXERROR( DPNERR_NOTHOST                                 ),   
    DXERROR( DPNERR_NOTREADY                                ),   
    DXERROR( DPNERR_NOTREGISTERED                           ),   
    DXERROR( DPNERR_OUTOFMEMORY                             ),   
    DXERROR( DPNERR_PENDING                                 ),   
    DXERROR( DPNERR_PLAYERALREADYINGROUP                    ),   
    DXERROR( DPNERR_PLAYERLOST                              ),   
    DXERROR( DPNERR_PLAYERNOTINGROUP                        ),   
    DXERROR( DPNERR_PLAYERNOTREACHABLE                      ),   
    DXERROR( DPNERR_SENDTOOLARGE                            ),   
    DXERROR( DPNERR_SESSIONFULL                             ),   
    DXERROR( DPNERR_TABLEFULL                               ),   
    DXERROR( DPNERR_TIMEDOUT                                ),   
    DXERROR( DPNERR_UNINITIALIZED                           ),   
    DXERROR( DPNERR_UNSUPPORTED                             ),   
    DXERROR( DPNERR_USERCANCEL                              ),   
    DXERROR( DXFILEERR_BADALLOC                             ),   
    DXERROR( DXFILEERR_BADARRAYSIZE                         ),   
    DXERROR( DXFILEERR_BADCACHEFILE                         ),   
    DXERROR( DXFILEERR_BADDATAREFERENCE                     ),   
    DXERROR( DXFILEERR_BADFILE                              ),   
    DXERROR( DXFILEERR_BADFILECOMPRESSIONTYPE               ),   
    DXERROR( DXFILEERR_BADFILEFLOATSIZE                     ),   
    DXERROR( DXFILEERR_BADFILETYPE                          ),   
    DXERROR( DXFILEERR_BADFILEVERSION                       ),   
    DXERROR( DXFILEERR_BADINTRINSICS                        ),   
    DXERROR( DXFILEERR_BADOBJECT                            ),   
    DXERROR( DXFILEERR_BADRESOURCE                          ),   
    DXERROR( DXFILEERR_BADSTREAMHANDLE                      ),   
    DXERROR( DXFILEERR_BADTYPE                              ),   
    DXERROR( DXFILEERR_BADVALUE                             ),   
    DXERROR( DXFILEERR_FILENOTFOUND                         ),   
    DXERROR( DXFILEERR_INTERNALERROR                        ),   
    DXERROR( DXFILEERR_NOINTERNET                           ),   
    DXERROR( DXFILEERR_NOMOREDATA                           ),   
    DXERROR( DXFILEERR_NOMOREOBJECTS                        ),   
    DXERROR( DXFILEERR_NOMORESTREAMHANDLES                  ),   
    DXERROR( DXFILEERR_NOTDONEYET                           ),   
    DXERROR( DXFILEERR_NOTEMPLATE                           ),   
    DXERROR( DXFILEERR_NOTFOUND                             ),   
    DXERROR( DXFILEERR_PARSEERROR                           ),   
    DXERROR( DXFILEERR_RESOURCENOTFOUND                     ),   
    DXERROR( DXFILEERR_URLNOTFOUND                          ),   

    ENG_END =(unsigned int)0xffffffff
};
#undef DXERROR

//
// ENGINE INITIALIZATIONS
//
enum d3deng_mode                            // Must OR this modes to get what you want
{
    ENG_ACT_DEFAULT             = (0),      // Activates nothing
    ENG_ACT_FULLSCREEN          = (1<<0),   // Default is Window
    ENG_ACT_SOFTWARE            = (1<<1),   // Default is Hardware
    ENG_ACT_BACKBUFFER_LOCK     = (1<<2),   // Default is you can't lock back buffer
    ENG_ACT_STENCILOFF          = (1<<3),   // Default is you will use stencil
    ENG_ACT_16_BPP              = (1<<4),   // Default is you will use stencil
    ENG_ACT_SHADERS_IN_SOFTWARE = (1<<5),   // Default is that the shaders are done in hardware
    ENG_ACT_LOCK_WINDOW_SIZE    = (1<<6)
};

///////////////////////////////////////////////////////////////////////////
// TYPES
///////////////////////////////////////////////////////////////////////////

struct dxerr
{
    dxerror_enum    Desc;
    xbool           bSign;

    inline operator HRESULT() { return (HRESULT) ((bSign) ? -Desc : Desc) ; }
    inline const dxerr& operator = ( HRESULT hRes ) 
    {         
        if( hRes < 0 ){ bSign=1; Desc = (dxerror_enum)-hRes; }  
        else          { bSign=0; Desc = (dxerror_enum)hRes;  } 

        return *this;
    }
};

struct d3dvertex 
{
    vector3     Pos;
    vector3     Normal;
    vector2     UV;

    inline d3dvertex(void){}
    inline d3dvertex( const vector3& P, const vector3& N, const vector2& TUV ) : Pos( P ), Normal( N ), UV( TUV ) {}
    inline void Set( const vector3& P, const vector3& N, const vector2& TUV ) 
    {
        Pos     = P;
        Normal  = N;
        UV      = TUV;
    }
};

struct d3dtlvertex 
{
    vector3    Screen;
    f32        Rhw;
    xcolor     Color;
    f32        Specular;
    vector2    UV;

    inline void Set( const vector3& P, xcolor C, const vector2& TUV  ) 
    {
        Screen      = P;
        UV          = TUV;
        Color       = C;
        Specular    = 1;
        Rhw         = 1;
    }
};

struct d3dlvertex 
{
    vector3    Pos;
    xcolor     Color;
    vector2    UV;
    f32        Specular;

    inline void Set( const vector3& P, xcolor C, const vector2& TUV  ) 
    {
        Pos         = P;
        UV          = TUV;
        Color       = C;
        Specular    = 1;
    }
};

///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////

void d3deng_SetLight        ( s32 LightID, vector3& Dir, xcolor& Color );
void d3deng_SetAmbientLight ( xcolor& Color );


void        d3deng_EntryPoint     ( s32& argc, char**& argv, HINSTANCE h1, HINSTANCE h2, LPSTR str1, INT i1 );
s32         d3deng_ExitPoint      ( void );
void        d3deng_SetPresets     ( u32 Mode = ENG_ACT_DEFAULT );
u32         d3deng_GetMode        ( void ) ;

void        DebugMessage          ( const char* FormatStr, ... );

HWND        d3deng_GetWindowHandle      ( void );
void        d3deng_UpdateDisplayWindow  ( HWND hWindow );
void        d3deng_SetWindowHandle      ( HWND hWindow );
void        d3deng_SetParentHandle      ( HWND hWindow );

void        d3deng_SetResolution  ( s32 Width, s32 Height );
HINSTANCE   d3deng_GetInstace     ( void );
LRESULT CALLBACK eng_D3DWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );


///////////////////////////////////////////////////////////////////////////
// HACK FUNCTIONS
///////////////////////////////////////////////////////////////////////////

enum mouse_mode
{
    MOUSE_MODE_BUTTONS,
    MOUSE_MODE_NEVER,
    MOUSE_MODE_ALWAYS,
	MOUSE_MODE_ABSOLUTE
};

void  d3deng_SetMouseMode    ( mouse_mode Mode );
void  d3deng_ComputeMousePos ( void );
f32   d3deng_GetABSMouseX    ( void ); //Legacy code, used ONLY on ArtistViewer.
f32   d3deng_GetABSMouseY    ( void ); //Legacy code, used ONLY on ArtistViewer.

///////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
///////////////////////////////////////////////////////////////////////////

extern IDirect3DDevice9* g_pd3dDevice;

///////////////////////////////////////////////////////////////////////////
// Magical macro which defines the entry point of the app. Make sure that 
// the use has the entry point: void AppMain( s32 argc, char* argv[] ) 
// define somewhere. MFC apps don't need the user entry point.
///////////////////////////////////////////////////////////////////////////
#define AppMain AppMain( s32 argc, char* argv[] );                          \
s32 __stdcall WinMain( HINSTANCE h1, HINSTANCE h2, LPSTR str1, INT i1 )     \
{ s32 argc; char** argv; d3deng_EntryPoint( argc, argv, h1, h2, str1, i1 ); \
  x_StartMain(AppMain,argc,argv);                                           \
  return d3deng_ExitPoint(); } void AppMain                                                              


///////////////////////////////////////////////////////////////////////////
// END
///////////////////////////////////////////////////////////////////////////
#endif