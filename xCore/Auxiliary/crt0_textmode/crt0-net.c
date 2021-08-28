/***
*crt0.c - C runtime initialization routine
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This is the actual startup routine for apps.  It calls the user's main
*       routine [w]main() or [w]WinMain after performing C Run-Time Library
*       initialization.
*
*       With ifdefs, this source file also provides the source code for:
*       wcrt0.c     the startup routine for console apps with wide chars
*       wincrt0.c   the startup routine for Windows apps
*       wwincrt0.c  the startup routine for Windows apps with wide chars
*
*******************************************************************************/

#ifndef CRTDLL

#include <cruntime.h>
#include <dos.h>
#include <internal.h>
#include <process.h>
#include <stdlib.h>
#include <string.h>
#include <rterr.h>
#include <rtcapi.h>
#include <windows.h>
#include <awint.h>
#include <tchar.h>
#include <dbgint.h>

void    x_Init  ( void );
void    x_Kill  ( void );

/*
 * wWinMain is not yet defined in winbase.h. When it is, this should be
 * removed.
 */

int
WINAPI
wWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    int nShowCmd
    );

#ifdef WPRFLAG
_TUCHAR * __cdecl _wwincmdln(void);
#else  /* WPRFLAG */
_TUCHAR * __cdecl _wincmdln(void);
#endif  /* WPRFLAG */

/*
 * command line, environment, and a few other globals
 */

#ifdef WPRFLAG
wchar_t *_wcmdln;           /* points to wide command line */
#else  /* WPRFLAG */
char *_acmdln;              /* points to command line */
#endif  /* WPRFLAG */

char *_aenvptr = NULL;      /* points to environment block */
wchar_t *_wenvptr = NULL;   /* points to wide environment block */


void (__cdecl * _aexit_rtn)(int) = _exit;   /* RT message return procedure */

static void __cdecl fast_error_exit(int);   /* Error exit via ExitProcess */

static int __cdecl check_managed_app(void); /* Determine if a managed app */

/*
 * _error_mode and _apptype, together, determine how error messages are
 * written out.
 */
int __error_mode = _OUT_TO_DEFAULT;
#ifdef _WINMAIN_
int __app_type = _GUI_APP;
#else  /* _WINMAIN_ */
int __app_type = _CONSOLE_APP;
#endif  /* _WINMAIN_ */


/***
*mainCRTStartup(void)
*wmainCRTStartup(void)
*WinMainCRTStartup(void)
*wWinMainCRTStartup(void)
*
*Purpose:
*       These routines do the C runtime initialization, call the appropriate
*       user entry function, and handle termination cleanup.  For a managed
*       app, they then return the exit code back to the calling routine, which
*       is the managed startup code.  For an unmanaged app, they call exit and
*       never return.
*
*       Function:               User entry called:
*       mainCRTStartup          main
*       wmainCRTStartup         wmain
*       WinMainCRTStartup       WinMain
*       wWinMainCRTStartup      wWinMain
*
*Entry:
*
*Exit:
*       Managed app: return value from main() et al, or the exception code if
*                 execution was terminated by the __except guarding the call
*                 to main().
*       Unmanaged app: never return.
*
*******************************************************************************/

#ifdef _WINMAIN_

#ifdef WPRFLAG
int wWinMainCRTStartup(
#else  /* WPRFLAG */
int WinMainCRTStartup(
#endif  /* WPRFLAG */

#else  /* _WINMAIN_ */

#ifdef WPRFLAG
int wmainCRTStartup(
#else  /* WPRFLAG */
int mainCRTStartup(
#endif  /* WPRFLAG */

#endif  /* _WINMAIN_ */

        void
        )

{
        int initret;
        int mainret;
        OSVERSIONINFOA *posvi;
        int managedapp;
#ifdef _WINMAIN_
        _TUCHAR *lpszCommandLine;
        STARTUPINFO StartupInfo;
#endif  /* _WINMAIN_ */
        /*
         * Dynamically allocate the OSVERSIONINFOA buffer, so we avoid
         * triggering the /GS buffer overrun detection.  That can't be
         * used here, since the guard cookie isn't available until we
         * initialize it from here!
         */
        posvi = (OSVERSIONINFOA *)_alloca(sizeof(OSVERSIONINFOA));

        /*
         * Get the full Win32 version
         */
        posvi->dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
        (void)GetVersionExA(posvi);

        _osplatform = posvi->dwPlatformId;
        _winmajor = posvi->dwMajorVersion;
        _winminor = posvi->dwMinorVersion;

        /*
         * The somewhat bizarre calculations of _osver and _winver are
         * required for backward compatibility (used to use GetVersion)
         */
        _osver = (posvi->dwBuildNumber) & 0x07fff;
        if ( _osplatform != VER_PLATFORM_WIN32_NT )
            _osver |= 0x08000;
        _winver = (_winmajor << 8) + _winminor;

        /*
         * Determine if this is a managed application
         */
        managedapp = check_managed_app();

#ifdef _MT
        if ( !_heap_init(1) )               /* initialize heap */
#else  /* _MT */
        if ( !_heap_init(0) )               /* initialize heap */
#endif  /* _MT */
            fast_error_exit(_RT_HEAPINIT);  /* write message and die */

#ifdef _MT
        if( !_mtinit() )                    /* initialize multi-thread */
            fast_error_exit(_RT_THREAD);    /* write message and die */
#endif  /* _MT */

        /*
         * Initialize the Runtime Checks stuff
         */
#ifdef _RTC
        _RTC_Initialize();
#endif  /* _RTC */
        /*
         * Guard the remainder of the initialization code and the call
         * to user's main, or WinMain, function in a __try/__except
         * statement.
         */

        __try {

            if ( _ioinit() < 0 )            /* initialize lowio */
                _amsg_exit(_RT_LOWIOINIT);

#ifdef WPRFLAG
            /* get wide cmd line info */
            _wcmdln = (wchar_t *)__crtGetCommandLineW();

            /* get wide environ info */
            _wenvptr = (wchar_t *)__crtGetEnvironmentStringsW();

            if ( _wsetargv() < 0 )
                _amsg_exit(_RT_SPACEARG);
            if ( _wsetenvp() < 0 )
                _amsg_exit(_RT_SPACEENV);
#else  /* WPRFLAG */
            /* get cmd line info */
            _acmdln = (char *)GetCommandLineA();

            /* get environ info */
            _aenvptr = (char *)__crtGetEnvironmentStringsA();

            if ( _setargv() < 0 )
                _amsg_exit(_RT_SPACEARG);
            if ( _setenvp() < 0 )
                _amsg_exit(_RT_SPACEENV);
#endif  /* WPRFLAG */

            x_Init();

            initret = _cinit(TRUE);                  /* do C data initialize */
            if (initret != 0)
                _amsg_exit(initret);

#ifdef _WINMAIN_

            StartupInfo.dwFlags = 0;
            GetStartupInfo( &StartupInfo );

#ifdef WPRFLAG
            lpszCommandLine = _wwincmdln();
            mainret = wWinMain(
#else  /* WPRFLAG */
            lpszCommandLine = _wincmdln();
            mainret = WinMain(
#endif  /* WPRFLAG */
                               GetModuleHandleA(NULL),
                               NULL,
                               lpszCommandLine,
                               StartupInfo.dwFlags & STARTF_USESHOWWINDOW
                                    ? StartupInfo.wShowWindow
                                    : SW_SHOWDEFAULT
                             );
#else  /* _WINMAIN_ */

#ifdef WPRFLAG
            __winitenv = _wenviron;
            mainret = wmain(__argc, __wargv, _wenviron);
#else  /* WPRFLAG */
            __initenv = _environ;
            mainret = main(__argc, __argv, _environ);
#endif  /* WPRFLAG */

#endif  /* _WINMAIN_ */

            if ( !managedapp )
                exit(mainret);

            _cexit();

        }
        __except ( _XcptFilter(GetExceptionCode(), GetExceptionInformation()) )
        {
            /*
             * Should never reach here
             */

            mainret = GetExceptionCode();

            if ( !managedapp )
                _exit(mainret);

            _c_exit();

        } /* end of try - except */

        x_Kill();

        return mainret;
}



/***
*_amsg_exit(rterrnum) - Fast exit fatal errors
*
*Purpose:
*       Exit the program with error code of 255 and appropriate error
*       message.
*
*Entry:
*       int rterrnum - error message number (amsg_exit only).
*
*Exit:
*       Calls exit() (for integer divide-by-0) or _exit() indirectly
*       through _aexit_rtn [amsg_exit].
*       For multi-thread: calls _exit() function
*
*Exceptions:
*
*******************************************************************************/

void __cdecl _amsg_exit (
        int rterrnum
        )
{
#ifdef _WINMAIN_
        if ( __error_mode == _OUT_TO_STDERR )
#else  /* _WINMAIN_ */
        if ( __error_mode != _OUT_TO_MSGBOX )
#endif  /* _WINMAIN_ */
            _FF_MSGBANNER();    /* write run-time error banner */

        _NMSG_WRITE(rterrnum);  /* write message */
        _aexit_rtn(255);        /* normally _exit(255) */
}

/***
*fast_error_exit(rterrnum) - Faster exit fatal errors
*
*Purpose:
*       Exit the process with error code of 255 and appropriate error
*       message.
*
*Entry:
*       int rterrnum - error message number (amsg_exit only).
*
*Exit:
*       Calls ExitProcess (through __crtExitProcess).
*
*Exceptions:
*
*******************************************************************************/

static void __cdecl fast_error_exit (
        int rterrnum
        )
{
#ifdef _WINMAIN_
        if ( __error_mode == _OUT_TO_STDERR )
#else  /* _WINMAIN_ */
        if ( __error_mode != _OUT_TO_MSGBOX )
#endif  /* _WINMAIN_ */
            _FF_MSGBANNER();    /* write run-time error banner */

        _NMSG_WRITE(rterrnum);  /* write message */
        __crtExitProcess(255);  /* normally _exit(255) */
}

/***
*check_managed_app() - Check for a managed executable
*
*Purpose:
*       Determine if the EXE the startup code is linked into is a managed app
*       by looking for the COM Runtime Descriptor in the Image Data Directory
*       of the PE or PE+ header.
*
*Entry:
*       None
*
*Exit:
*       1 if managed app, 0 if not.
*
*Exceptions:
*
*******************************************************************************/

static int __cdecl check_managed_app (
        void
        )
{
        PIMAGE_DOS_HEADER pDOSHeader;
        PIMAGE_NT_HEADERS pPEHeader;
        PIMAGE_OPTIONAL_HEADER32 pNTHeader32;
        PIMAGE_OPTIONAL_HEADER64 pNTHeader64;

        pDOSHeader = (PIMAGE_DOS_HEADER)GetModuleHandleA(NULL);
        if ( pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE )
            return 0;

        pPEHeader = (PIMAGE_NT_HEADERS)((char *)pDOSHeader +
                                        pDOSHeader->e_lfanew);
        if ( pPEHeader->Signature != IMAGE_NT_SIGNATURE )
            return 0;

        pNTHeader32 = (PIMAGE_OPTIONAL_HEADER32)&pPEHeader->OptionalHeader;
        switch ( pNTHeader32->Magic ) {
        case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
            /* PE header */
            if ( pNTHeader32->NumberOfRvaAndSizes <=
                    IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR )
                return 0;
            return !! pNTHeader32 ->
                      DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR] .
                      VirtualAddress;
        case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
            /* PE+ header */
            pNTHeader64 = (PIMAGE_OPTIONAL_HEADER64)pNTHeader32;
            if ( pNTHeader64->NumberOfRvaAndSizes <=
                    IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR )
                return 0;
            return !! pNTHeader64 ->
                      DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR] .
                      VirtualAddress;
        }

        /* Not PE or PE+, so not managed */
        return 0;
}

#ifndef WPRFLAG


#endif  /* WPRFLAG */

#endif  /* CRTDLL */
