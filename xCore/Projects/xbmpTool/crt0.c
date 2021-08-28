/***
*crt0.c - C runtime initialization routine
*
*       Copyright (c) 1989-1997, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This the actual startup routine for apps.  It calls the user's main
*       routine [w]main() or [w]WinMain after performing C Run-Time Library
*       initialization.
*
*       (With ifdef's, this source file also provides the source code for
*       wcrt0.c, the startup routine for console apps with wide characters,
*       wincrt0.c, the startup routine for Windows apps, and wwincrt0.c,
*       the startup routine for Windows apps with wide characters.)
*
*******************************************************************************/

#ifdef _WIN32

#ifndef CRTDLL

#include <cruntime.h>
#include <dos.h>
#include <internal.h>
#include <stdlib.h>
#include <string.h>
#include <rterr.h>
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
*BaseProcessStartup(PVOID Peb)
*
*Purpose:
*       This routine does the C runtime initialization, calls main(), and
*       then exits.  It never returns.
*
*Entry:
*       PVOID Peb - pointer to Win32 Process Environment Block (not used)
*
*Exit:
*       This function never returns.
*
*******************************************************************************/

#ifdef _WINMAIN_

#ifdef WPRFLAG
void wWinMainCRTStartup(
#else  /* WPRFLAG */
void WinMainCRTStartup(
#endif  /* WPRFLAG */

#else  /* _WINMAIN_ */

#ifdef WPRFLAG
void wmainCRTStartup(
#else  /* WPRFLAG */
void mainCRTStartup(
#endif  /* WPRFLAG */

#endif  /* _WINMAIN_ */
        void
        )

{
        int mainret;

#ifdef _WINMAIN_
        _TUCHAR *lpszCommandLine;
        STARTUPINFO StartupInfo;
#endif  /* _WINMAIN_ */

        /*
         * Get the full Win32 version
         */
        _osver = GetVersion();

        _winminor = (_osver >> 8) & 0x00FF ;
        _winmajor = _osver & 0x00FF ;
        _winver = (_winmajor << 8) + _winminor;
        _osver = (_osver >> 16) & 0x00FFFF ;

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
         * Guard the remainder of the initialization code and the call
         * to user's main, or WinMain, function in a __try/__except
         * statement.
         */

        __try {

            _ioinit();                      /* initialize lowio */

#ifdef WPRFLAG
            /* get wide cmd line info */
            _wcmdln = (wchar_t *)__crtGetCommandLineW();

            /* get wide environ info */
            _wenvptr = (wchar_t *)__crtGetEnvironmentStringsW();

            _wsetargv();
            _wsetenvp();
#else  /* WPRFLAG */
            /* get cmd line info */
            _acmdln = (char *)GetCommandLineA();

            /* get environ info */
            _aenvptr = (char *)__crtGetEnvironmentStringsA();

            _setargv();
            _setenvp();
#endif  /* WPRFLAG */

            x_Init();

            _cinit();                       /* do C data initialize */

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
            exit(mainret);
        }
        __except ( _XcptFilter(GetExceptionCode(), GetExceptionInformation()) )
        {
            /*
             * Should never reach here
             */
            _exit( GetExceptionCode() );

        } /* end of try - except */

        x_Kill();
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
*       Calls ExitProcess.
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
        ExitProcess(255);       /* normally _exit(255) */
}

#ifndef WPRFLAG


#endif  /* WPRFLAG */

#endif  /* CRTDLL */

#else  /* _WIN32 */

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <msdos.h>
#include <string.h>
#include <setjmp.h>
#include <dbgint.h>
#include <macos\types.h>
#include <macos\segload.h>
#include <macos\gestalte.h>
#include <macos\osutils.h>
#include <macos\traps.h>
#include <mpw.h>

static void __cdecl Inherit(void);  /* local function */

int __cdecl main(int, char **, char **);             /*generated by compiler*/

unsigned long _GetShellStack(void);

static char * __cdecl _p2cstr_internal ( unsigned char * str );

extern MPWBLOCK * _pMPWBlock;
extern int __argc;
extern char **__argv;

/***
*__crt0()
*
*Purpose:
*       This routine does the C runtime initialization, calls main(), and
*       then exits.  It never returns.
*
*Entry:
*
*Exit:
*       This function never returns.
*
*******************************************************************************/

void __cdecl __crt0 (
        )
{
        int mainret;
        char szPgmName[32];
        char *pArg;
        char *argv[2];

#ifndef _M_MPPC
        void *pv;

        /* This is the magic stuff that MPW tools do to get info from MPW*/

        pv = (void *)*(int *)0x316;
        if (pv != NULL && !((int)pv & 1) && *(int *)pv == 'MPGM') {
            pv = (void *)*++(int *)pv;
            if (pv != NULL && *(short *)pv == 'SH') {
                _pMPWBlock = (MPWBLOCK *)pv;
            }
        }

#endif  /* _M_MPPC */

        _environ = NULL;
        if (_pMPWBlock == NULL) {
            __argc = 1;
            memcpy(szPgmName, (char *)0x910, sizeof(szPgmName));
            pArg = _p2cstr_internal(szPgmName);
            argv[0] = pArg;
            argv[1] = NULL;
            __argv = argv;

#ifndef _M_MPPC
            _shellStack = 0;                        /* force ExitToShell */
#endif  /* _M_MPPC */
        }
#ifndef _M_MPPC
        else {
            _shellStack = _GetShellStack();        //return current a6, or first a6
            _shellStack += 4;                      //a6 + 4 is the stack pointer we want
            __argc = _pMPWBlock->argc;
            __argv = _pMPWBlock->argv;

            Inherit();       /* Inherit file handles - env is set up by _envinit if needed */
        }
#endif  /* _M_MPPC */

        /*
         * call run time initializer
         */
        __cinit();

        mainret = main(__argc, __argv, _environ);
        exit(mainret);
}


#ifndef _M_MPPC
/***
*Inherit() - obtain and process info on inherited file handles.
*
*Purpose:
*
*       Locates and interprets MPW std files.  For files we just save the
*       file handles.   For the console we save the device table address so
*       we can do console I/O.  In the latter case, FDEV is set in the _osfile
*       array.
*
*Entry:
*       Address of MPW param table
*
*Exit:
*       No return value.
*
*Exceptions:
*
*******************************************************************************/

static void __cdecl Inherit (
        void
        )
{
        MPWFILE *pFile;
        int i;
        pFile = _pMPWBlock->pFile;
        if (pFile == NULL) {
            return;
        }
        for (i = 0; i < 3; i++) {
            switch ((pFile->pDevice)->name) {
                case 'ECON':
                    _osfile[i] |= FDEV | FOPEN;
                    _osfhnd[i] = (int)pFile;
                    break;

                case 'FSYS':
                    _osfile[i] |= FOPEN;
                    _osfhnd[i] = (*(pFile->ppFInfo))->ioRefNum;
                    break;
            }
            pFile++;
        }
}

#endif  /* _M_MPPC */



static char * __cdecl _p2cstr_internal (
        unsigned char * str
        )
{
        unsigned char *pchSrc;
        unsigned char *pchDst;
        int  cch;

        if ( str && *str ) {
            pchDst = str;
            pchSrc = str + 1;

            for ( cch=*pchDst; cch; --cch ) {
                *pchDst++ = *pchSrc++;
            }

            *pchDst = '\0';
        }

        return( str );
}

#endif  /* _WIN32 */
