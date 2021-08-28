//############################################################################
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  MSSDBG.C: Runtime debugging facilities for MSS API calls              ##
//##                                                                        ##
//##  16-bit protected-mode source compatible with MSC 7.0                  ##
//##  32-bit protected-mode source compatible with MSC 9.0                  ##
//##                                                                        ##
//##  Version 1.00 of 15-Feb-95: Derived from WAILDBG V1.01                 ##
//##          1.01 of 11-Oct-95: Added MEM_alloc/free_lock() debug trace    ##
//##          1.02 of 20-Jan-96: jkr: added all the win32 stuff             ##
//##          1.03 of 20-Mar-98: jkr: merged DOS and windows                ##
//##                                                                        ##
//##  Author: John Miles & Jeff Roberts                                     ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################

#include "mss.h"
#include "imssapi.h"

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <math.h>

//############################################################################
//##                                                                        ##
//## Locked static data                                                     ##
//##                                                                        ##
//############################################################################

U16 AIL_debug=0;

U16 AIL_sys_debug=0;

U16 AIL_indent=0;

U32 AIL_starttime;

#if defined(IS_WINDOWS) || defined(IS_MAC)

FILE *AIL_debugfile;

#else

U32 AIL_debugfile;

#endif

#ifndef IS_WIN32

#define IN_AIL
#define OUT_AIL

#define IN_AIL_NM
#define OUT_AIL_NM


#else

HANDLE MilesMutex=0;
#ifdef DEBUGMUTEX
S32 MilesCount=0;
S32 MilesCurThread=0;
#endif

#define IN_AIL  InMilesMutex()
#define OUT_AIL OutMilesMutex()

#define IN_AIL_NM
#define OUT_AIL_NM

#endif

#ifdef IS_32

  int AIL_didaninit=NO;

  #define DidAnInit() (AIL_didaninit!=NO)

  #define SetInit(val) AIL_didaninit=(val);

#else

  #define MAXTASKS 32             // Only 32 16-bit apps can be used at once

  static HTASK inittasks[MAXTASKS];

  static int numtasks=0;

  static U8 DidAnInit()          // See if this task has previously called
  {                              //   AIL_startup
    int i;
    HTASK t=GetCurrentTask();
    for(i=0;i<numtasks;i++)
      if (inittasks[i]==t)
        return(1);
    return(0);
  }

  static void SetInit(int val)   // Clear or set the task's init status
  {
    int i;
    HTASK t;

    if (val==NO) {
      t=GetCurrentTask();
      for(i=0;i<numtasks;i++)
        if (inittasks[i]==t) {
          AIL_memcpy((void*)&inittasks[i],(void*)&inittasks[i+1],(numtasks-i-1)*sizeof(inittasks[0]));
          --numtasks;
          break;
        }
    } else {
      if (numtasks>=MAXTASKS)
        MessageBox(0,"Too many MSS applications loaded at once.","Error from MSS",MB_OK);
      else
        inittasks[numtasks++]=GetCurrentTask();
    }
  }

#endif


//############################################################################
//##                                                                        ##
//## Macros                                                                 ##
//##                                                                        ##
//############################################################################

#define START  if (AIL_indent++, (AIL_debug && \
                  ((AIL_indent==1) || AIL_sys_debug) && \
                  (!AIL_low_background() && AIL_time_write())))

#define END    AIL_indent--

#define RESULT if (AIL_debug && ((AIL_indent==1) || AIL_sys_debug) && (!AIL_low_background()))

#define INDENT AIL_fprintf(AIL_debugfile,"%s%s",(char FAR *)indentspace,(char FAR *)indentdot+33-AIL_indent);

static char indentspace[]="               ";
static char indentdot[]="תתתתתתתתתתתתתתתתתתתתתתתתתתתתתתתת";

#ifdef IS_DOS

//############################################################################
//##                                                                        ##
//## Locked code                                                            ##
//##                                                                        ##
//############################################################################

#ifdef __WATCOMC__
#pragma aux AIL_debug "_*";
#pragma aux AIL_sys_debug "_*";
#pragma aux AIL_indent "_*";
#pragma aux AIL_starttime "_*";
#pragma aux AIL_debugfile "_*";
#pragma aux AIL_didaninit "_*";
#endif

#define LOCK(x) AIL_vmm_lock((void *) &(x),sizeof(x))

static S32 locked = 0;

void __pascal AILDEBUG_end(void);

void __pascal AILDEBUG_start(void)
{
   if (!locked)
      {
      AIL_vmm_lock_range(AILDEBUG_start, AILDEBUG_end);

      LOCK (AIL_indent);
      LOCK (AIL_debug);
      LOCK (AIL_sys_debug);
      LOCK (AIL_debugfile);

      LOCK (AIL_didaninit);
      LOCK (AIL_starttime);

      locked = 1;

      }
}

#else

static void outresfloat(F32 val)
{
  INDENT;
  AIL_fprintf(AIL_debugfile,"Result = %f\r\n",val);
}

#endif

static void outresint(U32 val)
{
  INDENT;
  AIL_fprintf(AIL_debugfile,"Result = %d\r\n",val);
}

static void outreshex(U32 val)
{
  INDENT;
  AIL_fprintf(AIL_debugfile,"Result = 0x%X\r\n",val);
}

////////////////////////////////////////////////////////
// jkr: this next block of code is for Win32s support //
////////////////////////////////////////////////////////

#define WAIL32SVERSION 0x20002

#ifdef IS_WIN32

//#define USING_DS_API ((AIL_preference[DIG_USE_WAVEOUT]==NO) && (AIL_preference[DIG_USE_MSS_MIXER]==NO))

  #undef RAD_process
  #define RAD_process(header,func,paramdesc,numparams) Num##func ,

  enum {
    NumAIL_StartFunc,
    #include "mssdbg.inc"
    NumAIL_EndFunc,
  };

  #undef RAD_process
  #define RAD_process(header,func,paramdesc,numparams) paramdesc ,

  char* aildescs[]={
    #include "mssdbg.inc"
  };

  #undef RAD_process

  static U32 onwin32s=0;  // jkr: are we running win32s?

  static U32 doquery=0;   // jkr: use queryperformance instead of timegettime?

  static LONGLONG qpfreq; // frequency of the high-perf timer

  static LONGLONG qpfirst;// first read of queryperformance

  static U32 lasttime;
  static U32 lastrad;
  static U32 radadj=0;

  static U32 lasttimeu;
  static U32 lastradu;
  static U32 radadju=0;

  // jkr: macros to make Win32s calls less ugly than they really are

  #define ailcall0(header,func,type) \
    ((onwin32s)?(type)callthunk(((U32)Num##func)-((U32)NumAIL_StartFunc)-1,0):\
    header##_API_##func())

  #define dsailcall0(header,func,type) \
    ((onwin32s)?(type)callthunk(((U32)Num##func)-((U32)NumAIL_StartFunc)-1,0):\
     header##_API_##func())

//    ((!USING_DS_API)?header##_API_##func():
//    header##_DS_API_##func()))

  #define ailcall1(header,func,type,param1) \
    ((onwin32s)?(type)callthunk(((U32)Num##func)-((U32)NumAIL_StartFunc)-1,((U32*)&param1)):\
    header##_API_##func(param1))

  #define dsailcall1(header,func,type,param1) \
    ((onwin32s)?(type)callthunk(((U32)Num##func)-((U32)NumAIL_StartFunc)-1,((U32*)&param1)):\
     header##_API_##func(param1))

//    ((!USING_DS_API)?header##_API_##func(param1):
//    header##_DS_API_##func(param1)))

  #define ailcall2(header,func,type,param1,param2) \
    ((onwin32s)?(type)callthunk(((U32)Num##func)-((U32)NumAIL_StartFunc)-1,((U32*)&param1)):\
    header##_API_##func(param1,param2))

  #define dsailcall2(header,func,type,param1,param2) \
    ((onwin32s)?(type)callthunk(((U32)Num##func)-((U32)NumAIL_StartFunc)-1,((U32*)&param1)):\
     header##_API_##func(param1,param2))

//    ((!USING_DS_API)?header##_API_##func(param1,param2):
//    header##_DS_API_##func(param1,param2)))

  #define ailcall3(header,func,type,param1,param2,param3) \
    ((onwin32s)?(type)callthunk(((U32)Num##func)-((U32)NumAIL_StartFunc)-1,((U32*)&param1)):\
    header##_API_##func(param1,param2,param3))

  #define dsailcall3(header,func,type,param1,param2,param3) \
    ((onwin32s)?(type)callthunk(((U32)Num##func)-((U32)NumAIL_StartFunc)-1,((U32*)&param1)):\
     header##_API_##func(param1,param2,param3))

//    ((!USING_DS_API)?header##_API_##func(param1,param2,param3):
//    header##_DS_API_##func(param1,param2,param3)))

  #define ailcall4(header,func,type,param1,param2,param3,param4) \
    ((onwin32s)?(type)callthunk(((U32)Num##func)-((U32)NumAIL_StartFunc)-1,((U32*)&param1)):\
    header##_API_##func(param1,param2,param3,param4))

  #define dsailcall4(header,func,type,param1,param2,param3,param4) \
    ((onwin32s)?(type)callthunk(((U32)Num##func)-((U32)NumAIL_StartFunc)-1,((U32*)&param1)):\
     header##_API_##func(param1,param2,param3,param4))

//    ((!USING_DS_API)?header##_API_##func(param1,param2,param3,param4):
//    header##_DS_API_##func(param1,param2,param3,param4)))

  #define ailcall5(header,func,type,param1,param2,param3,param4,param5) \
    ((onwin32s)?(type)callthunk(((U32)Num##func)-((U32)NumAIL_StartFunc)-1,((U32*)&param1)):\
    header##_API_##func(param1,param2,param3,param4,param5))

  #define dsailcall5(header,func,type,param1,param2,param3,param4,param5) \
    ((onwin32s)?(type)callthunk(((U32)Num##func)-((U32)NumAIL_StartFunc)-1,((U32*)&param1)):\
     header##_API_##func(param1,param2,param3,param4,param5))

//    ((!USING_DS_API)?header##_API_##func(param1,param2,param3,param4,param5):
//    header##_DS_API_##func(param1,param2,param3,param4,param5)))

  #define ailcall6(header,func,type,param1,param2,param3,param4,param5,param6) \
    ((onwin32s)?(type)callthunk(((U32)Num##func)-((U32)NumAIL_StartFunc)-1,((U32*)&param1)):\
    header##_API_##func(param1,param2,param3,param4,param5,param6))

  #define ailcall7(header,func,type,param1,param2,param3,param4,param5,param6,param7) \
    ((onwin32s)?(type)callthunk(((U32)Num##func)-((U32)NumAIL_StartFunc)-1,((U32*)&param1)):\
    header##_API_##func(param1,param2,param3,param4,param5,param6,param7))


  // thunking typedefs

  typedef DWORD (WINAPI * UT32PROC) (LPVOID lpBuff,DWORD user,LPVOID*trans);

  typedef BOOL (WINAPI * UTRegType) (HANDLE mod,LPCSTR dll16name,LPCSTR InitName,
                                     LPCSTR CallName, UT32PROC * thunk, FARPROC cb,
                                     LPVOID buff);

  typedef VOID (WINAPI * UTUnRType) (HANDLE mod);

  static UTRegType UTRegister = NULL;     // Thunk setup call

  static UTUnRType UTUnRegister = NULL;   // Thunk shutdown call

  static UT32PROC lpThunk = NULL;         // stepdown thunk address

  typedef U32 (WINAPI **call32with1)(U32 param1);
  typedef U32 (WINAPI **call32with2)(U32 param1,U32 param2);
  typedef U32 (WINAPI **call32with3)(U32 param1,U32 param2,U32 param3);
  typedef U32 (WINAPI **call32with4)(U32 param1,U32 param2,U32 param3,U32 param4);
  typedef U32 (WINAPI **call32with5)(U32 param1,U32 param2,U32 param3,U32 param4,U32 param5);

  DWORD WINAPI Win16CallIn(LPVOID lpbuff, WIN32S_CALLBACKS* CBD)
  {
    U8 * sb=CBD->Buffer32+CBD->StartO;
    while (CBD->CurLeft<CBD->Size) {
      switch (*sb) {
        case 1:
          (**(call32with1)(sb+1))( *(U32*)(sb+5) );
          break;
        case 2:
          (**(call32with2)(sb+1))( *(U32*)(sb+5), *(U32*)(sb+9) );
          break;
        case 3:
          (**(call32with3)(sb+1))( *(U32*)(sb+5), *(U32*)(sb+9), *(U32*)(sb+13) );
          break;
        case 4:
          (**(call32with4)(sb+1))( *(U32*)(sb+5), *(U32*)(sb+9), *(U32*)(sb+13), *(U32*)(sb+17) );
          break;
        case 5:
          (**(call32with5)(sb+1))( *(U32*)(sb+5), *(U32*)(sb+9), *(U32*)(sb+13), *(U32*)(sb+17), *(U32*)(sb+21) );
          break;
      }
      CBD->CurLeft+=(*sb*4)+5;
      CBD->StartO+=(*sb*4)+5;
      sb+=(*sb*4)+5;
      if (CBD->StartO>=CBD->Size) {
        CBD->StartO=0;
        sb=CBD->Buffer32;
      }
    }
    return(0);
  }

  // jkr: this function uses a string lookup table to build a stack frame and
  //      jumps into the 16-bit code

  static U32 callthunk(U32 funcnumber,U32* stackstart)
  {
    DWORD  args[13];
    DWORD* xlat[14];
    U32 FAR* ptr;

    U8 numargs=0;
    U8 numxlats=0;
    U32* stk=stackstart;

    char* s=aildescs[funcnumber];

    while (*s) {
      switch (*s++) {
        case 't':                          // pointer type from MSS 16
          ptr=(U32 FAR*)*stk++;

          if ((ptr==0) || (ptr==(U32 FAR*)-1)) { // don't translate if NULL or -1
            args[numargs++]=(U32)ptr;
            break;
          }

          if (*(ptr-1)!=0x5753534d) {      // check for hidden marker
           error:
            if (*(ptr-2)!=0x5753534d) {    // see if offset by a dword
             error2:
              MessageBox(0,"Bad pointer passed to Miles Sound System for Win32s.","Error from MSS",MB_OK);
              return(0);
            }
            if (*(ptr-5)!=*(ptr-4))        // check for matching pointer
              goto error2;

            args[numargs++]=*(ptr-5)+4;    // get real 16:16 pointer
            break;
          }

          if (*(ptr-4)!=*(ptr-3))          // check for the same pointer twice
            goto error;

          args[numargs++]=*(ptr-4);        // get real 16:16 pointer (don't translate)
          break;
        case 'p':                          // pointer type (need to translate)
          xlat[numxlats++]=&args[numargs]; // note!: falls through to next case
        case 'd':                          // dword type
          args[numargs++]=*stk++;
          break;
      }
    }

    xlat[numxlats] = 0;

    return( (U32)((*lpThunk)(numargs?args:0,funcnumber,numxlats?xlat:0)) );
  }


  // jkr: this function is called when the 32 bit DLL is loaded

  BOOL Win32sInit(HINSTANCE hinstDll)
  {
    OSVERSIONINFO oi;
    BOOL b;
    DWORD version = WAIL32SVERSION;

    if (onwin32s) {
      onwin32s++;
      return(1);
    }

    oi.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
    GetVersionEx(&oi);

    if (oi.dwPlatformId!=VER_PLATFORM_WIN32s) {
      if (QueryPerformanceFrequency((LARGE_INTEGER*)&qpfreq)) {
        doquery=1;
        QueryPerformanceCounter((LARGE_INTEGER*)&qpfirst);
        lastrad=0;
        lasttime=timeGetTime();
        lastradu=0;
        lasttime=lasttime;
      }
      return TRUE;
    }

    UTRegister=(UTRegType)GetProcAddress(GetModuleHandle("KERNEL32"),"UTRegister");
    UTUnRegister=(UTUnRType)GetProcAddress(GetModuleHandle("KERNEL32"),"UTUnRegister");

    if ((UTRegister==0) || (UTUnRegister==0)) {
      MessageBox(0,"Unable to load the 32 bit thunking DLL (KERNEL32.DLL).","Error from MSS",MB_OK);
      return(0);
    }

    b=UTRegister(hinstDll,"MSS16.DLL","WAIL16INIT","WAIL16CALL",&lpThunk,(FARPROC)Win16CallIn,&version);
    if (b)
      onwin32s=1;

    return(b);
  }


  // jkr: this function is called when the 32 bit DLL is freed

  void Win32sDone(HINSTANCE hinstDll)
  {
    if (onwin32s) {
      if (--onwin32s==0)
        UTUnRegister(hinstDll);
    }
  }

  //
  // we don't display these calls in the debug file because they are
  // called so frequently
  //

  DXDEF U32 WINAPI AIL_ms_count()
  {
    LONGLONG count;
    if (doquery)
    {
      U32 ret, time;
      S32 deltatime,deltarad;

      // read the new times...
      QueryPerformanceCounter((LARGE_INTEGER*)&count);
      time = timeGetTime();
      ret = radadj + ( (U32)(((count-qpfirst)*((LONGLONG)1000))/qpfreq) );

      // see how much each has moved
      deltatime = time - lasttime;
      deltarad = ret - lastrad;

      // check the query against timegettime to make sure it hasn't
      //   jumped ahead...

      if ( abs( deltatime-deltarad ) > 200 )
      {
        deltatime -= deltarad;
        radadj += deltatime;
        ret += deltatime;
      }

      // now check to see if it went backwards?
      if ( ( ( U32 ) ( ret - lastrad ) ) > 0xc0000000 )
      {
        // yup, just return the old timer value then
        return( lastrad );
      }

      lasttime = time;
      lastrad = ret;
      return(ret);
    
    } else
      return( timeGetTime() );
  }

  DXDEF U32 WINAPI AIL_us_count()
  {
    LONGLONG count;
    if (doquery) {
      U32 ret, time;
      S32 deltatime,deltarad;

      // read the new times...
      QueryPerformanceCounter((LARGE_INTEGER*)&count);
      time = timeGetTime();
      ret = radadju + ( (U32)(((count-qpfirst)*((LONGLONG)1000000L))/qpfreq) );

      // see how much each has moved
      deltatime = (time - lasttimeu)*1000;
      deltarad = ret - lastradu;

      // check the query against timegettime to make sure it hasn't
      //   jumped ahead...

      if ( abs( deltatime-deltarad ) > 200000 )
      {
        deltatime -= deltarad;
        radadju += deltatime;
        ret += deltatime;
      }

      // now check to see if it went backwards?
      if ( ( ( U32 ) ( ret - lastradu ) ) > 0xc0000000 )
      {
        // yup, just return the old timer value then
        return( lastradu );
      }

      lasttimeu = time;
      lastradu = ret;
      return(ret);
    } else
      return( timeGetTime()*1000 );
  }

#else

#ifdef IS_WIN16

  #undef RAD_process
  #define RAD_process(header,func,paramdesc,numparams) (FARPROC)header##_API_##func ,

  FARPROC ailprocs[]={
    #include "mssdbg.inc"
  };

  #undef RAD_process
  #define RAD_process(header,func,paramdesc,numparams) numparams ,

  U8 ailparams[]={
    #include "mssdbg.inc"
  };


  #undef RAD_process

  // jkr: these macros expand to normal function calls on a 16-bit compile

  #define ailcall0(header,func,type)                                    \
            header##_API_##func()

  #define ailcall1(header,func,type,param1)                             \
            header##_API_##func(param1)

  #define ailcall2(header,func,type,param1,param2)                      \
            header##_API_##func(param1,param2)

  #define ailcall3(header,func,type,param1,param2,param3)               \
            header##_API_##func(param1,param2,param3)

  #define ailcall4(header,func,type,param1,param2,param3,param4)        \
            header##_API_##func(param1,param2,param3,param4)

  #define ailcall5(header,func,type,param1,param2,param3,param4,param5) \
            header##_API_##func(param1,param2,param3,param4,param5)

  #define ailcall6(header,func,type,param1,param2,param3,param4,param5,param6) \
            header##_API_##func(param1,param2,param3,param4,param5,param6)

  #define ailcall7(header,func,type,param1,param2,param3,param4,param5,param6,param7) \
            header##_API_##func(param1,param2,param3,param4,param5,param6,param7)

  #define dsailcall0 ailcall0
  #define dsailcall1 ailcall1
  #define dsailcall2 ailcall2
  #define dsailcall3 ailcall3
  #define dsailcall4 ailcall4
  #define dsailcall5 ailcall5


  typedef DWORD (FAR PASCAL  * UT16CBPROC)( LPVOID lpBuff,
                                          DWORD  dwUserDefined,
                                          LPVOID FAR *lpTranslationList
                                        );
  UT16CBPROC PollWin32s;

  // jkr: this function gets called when the Win32s side is initalized

  DWORD AILEXPORT WAIL16INIT(UT16CBPROC pfnCB, LPVOID data)
  {
    if ((*(LPDWORD)data)!=WAIL32SVERSION)  {
      MessageBox(0,"Mismatched Miles Sound System DLLs (check MSS16.DLL and MSS32.DLL).","Error from MSS",MB_ICONHAND|MB_OK);
      return FALSE;
    }
    PollWin32s=pfnCB;
    return TRUE;
  }

  #define getparam(x) ((U32)(((LPDWORD) data)[(x)]))

  typedef U32 AILCALL FAR call16with0();
  typedef U32 AILCALL FAR call16with1(U32 param1);
  typedef U32 AILCALL FAR call16with2(U32 param1,U32 param2);
  typedef U32 AILCALL FAR call16with3(U32 param1,U32 param2,U32 param3);
  typedef U32 AILCALL FAR call16with4(U32 param1,U32 param2,U32 param3,U32 param4);
  typedef U32 AILCALL FAR call16with5(U32 param1,U32 param2,U32 param3,U32 param4,U32 param5);
  typedef U32 AILCALL FAR call16with6(U32 param1,U32 param2,U32 param3,U32 param4,U32 param5,U32 param6);
  typedef U32 AILCALL FAR call16with7(U32 param1,U32 param2,U32 param3,U32 param4,U32 param5,U32 param6,U32 param7);

  typedef U32 (WINAPI FAR* UT16To32Type)(U32 seloff);

  UT16To32Type UT16To32=0;

  // jkr: this function dispatches the incoming Win32s calls

  U8 FromWin32sCB=0;

  DWORD AILEXPORT WAIL16CALL(LPVOID data, DWORD funccode)
  {
    U32 ret;

    if (UT16To32==0) {   // jkr: dynamically load the thunking DLLs
      UT16To32=(UT16To32Type)GetProcAddress(GetModuleHandle("WIN32S16"),"UTSELECTOROFFSETTOLINEAR");
      if (UT16To32==0) {
        MessageBox(0,"Unable to load thunking DLL (W32SYS.DLL).","Error from MSS",MB_OK);
        return(0);
      }
    }

    if (ailparams[funccode]&0x40)
      FromWin32sCB=1;

    switch( ailparams[funccode]&0xf ) {
      case 0:
        ret= (*(call16with0*)ailprocs[funccode])() ;
        break;
      case 1:
        ret= (*(call16with1*)ailprocs[funccode])(getparam(0)) ;
        break;
      case 2:
        ret= (*(call16with2*)ailprocs[funccode])(getparam(0),getparam(1)) ;
        break;
      case 3:
        ret= (*(call16with3*)ailprocs[funccode])(getparam(0),getparam(1),getparam(2)) ;
        break;
      case 4:
        ret= (*(call16with4*)ailprocs[funccode])(getparam(0),getparam(1),getparam(2),getparam(3)) ;
        break;
      case 5:
        ret= (*(call16with5*)ailprocs[funccode])(getparam(0),getparam(1),getparam(2),getparam(3),getparam(4)) ;
        break;
      case 6:
        ret= (*(call16with6*)ailprocs[funccode])(getparam(0),getparam(1),getparam(2),getparam(3),getparam(4),getparam(5)) ;
        break;
      case 7:
        ret= (*(call16with7*)ailprocs[funccode])(getparam(0),getparam(1),getparam(2),getparam(3),getparam(4),getparam(5),getparam(6)) ;
        break;
      default:
        MessageBox(0,"Bad function call made by MSS for Win32s.","Error from MSS",MB_OK);
        return(0);
    }

    FromWin32sCB=0;

    if ((ret) && (ailparams[funccode]&0x80))
      ret=(*UT16To32)(ret);

    return( ret );
  }

#else

  #ifdef IS_DOS

  //DOS

  #define ailcall0(header,func,type)                                    \
            header##_API_##func()

  #define ailcall1(header,func,type,param1)                             \
            header##_API_##func(param1)

  #define ailcall2(header,func,type,param1,param2)                      \
            header##_API_##func(param1,param2)

  #define ailcall3(header,func,type,param1,param2,param3)               \
            header##_API_##func(param1,param2,param3)

  #define ailcall4(header,func,type,param1,param2,param3,param4)        \
            header##_API_##func(param1,param2,param3,param4)

  #define ailcall5(header,func,type,param1,param2,param3,param4,param5) \
            header##_API_##func(param1,param2,param3,param4,param5)

  #define ailcall6(header,func,type,param1,param2,param3,param4,param5,param6) \
            header##_API_##func(param1,param2,param3,param4,param5,param6)

  #define ailcall7(header,func,type,param1,param2,param3,param4,param5,param6,param7) \
            header##_API_##func(param1,param2,param3,param4,param5,param6,param7)

  #define dsailcall0 ailcall0
  #define dsailcall1 ailcall1
  #define dsailcall2 ailcall2
  #define dsailcall3 ailcall3
  #define dsailcall4 ailcall4
  #define dsailcall5 ailcall5

  #else

  #ifdef IS_MAC

static void AIL_get_debug_file(char* fname)
{
  AIL_strcpy( fname, MSS_Directory);
  AIL_strcat(fname,"mssdebug.log");
}

  #define ailcall0(header,func,type)                                    \
            header##_API_##func()

  #define ailcall1(header,func,type,param1)                             \
            header##_API_##func(param1)

  #define ailcall2(header,func,type,param1,param2)                      \
            header##_API_##func(param1,param2)

  #define ailcall3(header,func,type,param1,param2,param3)               \
            header##_API_##func(param1,param2,param3)

  #define ailcall4(header,func,type,param1,param2,param3,param4)        \
            header##_API_##func(param1,param2,param3,param4)

  #define ailcall5(header,func,type,param1,param2,param3,param4,param5) \
            header##_API_##func(param1,param2,param3,param4,param5)

  #define ailcall6(header,func,type,param1,param2,param3,param4,param5,param6) \
            header##_API_##func(param1,param2,param3,param4,param5,param6)

  #define ailcall7(header,func,type,param1,param2,param3,param4,param5,param6,param7) \
            header##_API_##func(param1,param2,param3,param4,param5,param6,param7)

  #define dsailcall0 ailcall0
  #define dsailcall1 ailcall1
  #define dsailcall2 ailcall2
  #define dsailcall3 ailcall3
  #define dsailcall4 ailcall4
  #define dsailcall5 ailcall5

  #endif

  #endif

#endif

#endif


static S32    AIL_low_background                (void)
{
#ifdef IS_WIN32
   if (onwin32s)
     return(0);
   else
     return(AIL_API_background());
#else
  return(AIL_API_background());
#endif
}


//############################################################################
//##                                                                        ##
//## Write timestamp to output file or device                               ##
//##                                                                        ##
//############################################################################

S32 AILCALL AIL_time_write(void)
{
   U32 t;
   U16 h,m,s,c;

   t = AIL_ms_count()-AIL_starttime;

   h = (U16)(t/3600000L);
   m = (U16)((t/60000L)%60L);
   s = (U16)((t/1000L)%60L);
   c = (U16)(t%1000L);

   if (AIL_indent == 1)
      {
      AIL_fprintf(AIL_debugfile,"[%.02d:%.02d:%.02d.%.03d] %s",h,m,s,c,(char FAR *)indentdot+33-AIL_indent);
      }
   else
      AIL_fprintf(AIL_debugfile,"%s%s", (char FAR *)indentspace,(char FAR *)indentdot+33-AIL_indent);

   return 1;
}

//############################################################################
//##                                                                        ##
//## AIL_MMX_available()                                                    ##
//##                                                                        ##
//############################################################################

#ifdef IS_X86

DXDEF U32 AILEXPORT AIL_MMX_available(void)
{
   U32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_MMX_available()\r\n");
      }

   IN_AIL_NM;

   if (!AIL_get_preference(AIL_ENABLE_MMX_SUPPORT))
      {
      result = 0;
      }
   else
      {
      result = MSS_MMX_available();
      }

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

#endif

//############################################################################
//##                                                                        ##
//## AIL_debug_printf()                                                     ##
//##                                                                        ##
//############################################################################

DXDEF void
#ifndef IS_MAC
__cdecl
#endif
AIL_debug_printf(C8 const FAR *fmt, ...)
{
   static char format_string[2048];
   static char work_string[2048];

   S32 i;
   va_list ap;

   for (i=0; i < 2048; i++)
      {
      format_string[i] = fmt[i];

      if (!fmt[i])
         {
         break;
         }
      }

   format_string[2047] = 0;

   va_start(ap,
            fmt);

   vsprintf(work_string,
            format_string,
            ap);

   va_end  (ap);

#ifdef IS_WINDOWS
   OutputDebugString(work_string);
#else
#if SHOW_DEBUG_TRACE
   printf(work_string);
#endif
#endif

#if DEBUG_LOGFILE
   static S32 init = 0;

   if (init == 0)
      {
      _unlink("debug.log");
      init = 1;
      }

   FILE FAR*out = fopen("debug.log","a+t");
   fprintf(out,"%s",work_string);
   fclose(out);
#endif
}

#if defined(IS_WINDOWS) || defined(IS_MAC)

//############################################################################
//##                                                                        ##
//## Start of debugging functions                                           ##
//##                                                                        ##
//## Excluded: AIL_switch_stack()                                           ##
//##           AIL_restore_stack()                                          ##
//##           AIL_background()                                             ##
//##                                                                        ##
//############################################################################

DXDEF S32    AILEXPORT AIL_startup                   (void)
{
   static char filename[256];
   static S8 sysdebug[8];
   static S32 elapstime;
   static struct tm *adjtime;
   static char loctime[32];

   if (DidAnInit())
     return(0);

   SetInit(YES);

#ifdef IS_WIN32

   //
   // Get handle to application process thread that called AIL_startup(),
   // so that callback functions can suspend it
   //

   if (!onwin32s)  // jkr: don't do thread stuff under Win32s

     DuplicateHandle(GetCurrentProcess(),
                     GetCurrentThread(),
                     GetCurrentProcess(),
                    &hAppThread,
                     0,
                     FALSE,
                     DUPLICATE_SAME_ACCESS);
#endif

   //
   // Get profile string for debug script, and enable debug mode
   // if script filename valid
   //

   AIL_debug     = 0;
   AIL_sys_debug = 0;

#ifdef IS_MAC
   
   AIL_get_debug_file( filename );

#else

   GetProfileString("MSS", "MSSDebug", "None", filename, sizeof(filename));

   if (!AIL_strnicmp(filename, "None", 4))
      {
      ailcall0(AIL,startup,void);
      return(1);
      }

   GetProfileString("MSS", "MSSSysDebug", "0", sysdebug, sizeof(sysdebug));

   if (AIL_strnicmp(sysdebug, "0", 1))
      {
      AIL_sys_debug = 1;
      }

#endif

   //
   // Open script file and set "debug" flag
   //

#ifdef IS_MAC
  AIL_debugfile = fopen(filename,"r");
  if(AIL_debugfile)
  {
    fclose(AIL_debugfile);
    AIL_debugfile = fopen(filename,"wt");
    AIL_sys_debug = 0;
  }
#else
   AIL_debugfile = fopen(filename,"wt");
#endif

   if (AIL_debugfile == NULL)
      {
      ailcall0(AIL,startup,void);
      return(1);
      }

   //
   // Write header to script file
   //

   time(&elapstime);
   adjtime = localtime(&elapstime);
   AIL_strcpy(loctime,asctime(adjtime));
   loctime[24] = 0;

   AIL_fprintf(AIL_debugfile,
               "-------------------------------------------------------------------------------\r\n"
               "Miles Sound System usage script generated by MSS V"MSS_VERSION"\r\n"
               "Start time: %s\r\n"
               "-------------------------------------------------------------------------------\r\n\r\n"
               ,(U8 FAR *) loctime);

   //
   // Initialize API
   //

   IN_AIL;

   ailcall0(AIL,startup,void);

   OUT_AIL;

   AIL_starttime=AIL_ms_count();

   AIL_debug  = 1;

   AIL_indent = 1;
   AIL_time_write();
   AIL_indent = 0;

   AIL_fprintf(AIL_debugfile,"AIL_startup()\r\n");

   return(1);
}

#endif


//############################################################################

DXDEF
void    AILEXPORT  AIL_shutdown                  (void)
{
   if (DidAnInit())
     SetInit(NO);

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_shutdown()\r\n");
      }

   IN_AIL;

   ailcall0(AIL,shutdown,void);

   OUT_AIL;

   if (AIL_debug)
#if defined(IS_WINDOWS) || defined(IS_MAC)
      fclose(AIL_debugfile);
#else
      AIL_lowfclose(AIL_debugfile);
#endif

   END;

   #ifdef IS_WIN32

   if (!onwin32s) {  // jkr: don't do thread stuff under Win32s
     if (hAppThread) {
       CloseHandle(hAppThread);     //avoid debug kernel message
       hAppThread=0;
     }
   }

   #endif

}

//############################################################################

#ifdef IS_WIN32

DXDEF
HWND AILEXPORT AIL_HWND(void)
{
   HWND result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_HWND()\r\n");
      }

   IN_AIL_NM;

   result = AIL_API_HWND();

   OUT_AIL_NM;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

#else

#ifdef IS_MAC

DXDEF
ProcessSerialNumber AILEXPORT AIL_Process(void)
{
   ProcessSerialNumber result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_Process()\r\n");
      }

   IN_AIL;

   result = AIL_API_Process();

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result.highLongOfPSN);
      outreshex((U32)result.lowLongOfPSN);
      }

   END;

   return result;
}

#endif

#endif

//############################################################################

DXDEF
void FAR * AILEXPORT AIL_mem_alloc_lock(U32 size)
{
   void FAR *result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_mem_alloc_lock(%li)\r\n",size);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,mem_alloc_lock,void FAR*,size);

   OUT_AIL_NM;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
void AILEXPORT AIL_mem_free_lock(void FAR *ptr)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_mem_free_lock(0x%lX)\r\n",ptr);
      }

   IN_AIL_NM;

   ailcall1(AIL,mem_free_lock,void,ptr);

   OUT_AIL_NM;

   END;
}

//############################################################################

DXDEF
S32    AILEXPORT  AIL_set_preference            (U32       number, //)
                                              S32       value)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_preference(%ld,%ld)\r\n",number,value);
      }

   IN_AIL;

   result=ailcall2(AIL,set_preference,S32,number,value);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
char FAR * AILEXPORT  AIL_last_error               (void)
{
   char FAR *result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_last_error()\r\n");
      }

   IN_AIL_NM;

   result=ailcall0(AIL,last_error,char FAR*);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return (char FAR*) result;
}

//############################################################################

DXDEF
void AILEXPORT  AIL_set_error               (char const FAR* error_msg)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_error(%s)\r\n", error_msg);
      }

   IN_AIL_NM;

   ailcall1(AIL,set_error,void,error_msg);

   OUT_AIL_NM;

   END;
}

//############################################################################

DXDEF
void    AILEXPORT  AIL_delay                     (S32        intervals)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_delay(%ld)\r\n",intervals);
      }

   IN_AIL_NM;

#ifdef IS_WIN32

   Sleep(intervals * 100 / 6);

#else

   AIL_API_delay(intervals);

#endif

   OUT_AIL_NM;

   END;
}

//############################################################################

DXDEF
S32    AILEXPORT  AIL_background                (void)
{
   S32 result;

   IN_AIL_NM;

   result=AIL_low_background();

   OUT_AIL_NM;

   return result;
}

//############################################################################

DXDEF
HTIMER  AILEXPORT  AIL_register_timer            (AILTIMERCB    callback_fn)
{
   HTIMER result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_register_timer(0x%lX)\r\n",callback_fn);
      }

   IN_AIL;

   result=ailcall1(AIL,register_timer,HTIMER,callback_fn);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
U32   AILEXPORT  AIL_set_timer_user            (HTIMER      timer, //)
                                             U32         user)
{
   U32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_timer_user(%ld,%lu)\r\n",timer,user);
      }

   IN_AIL;

   result=ailcall2(AIL,set_timer_user,U32,timer,user);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
void    AILEXPORT  AIL_set_timer_period          (HTIMER      timer, //)
                                               U32         microseconds)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_timer_period(%ld,%lu)\r\n",timer,microseconds);
      }

   IN_AIL;

   ailcall2(AIL,set_timer_period,void,timer,microseconds);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void    AILEXPORT  AIL_set_timer_frequency       (HTIMER      timer, //)
                                               U32         hertz)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_timer_frequency(%ld,%lu)\r\n",timer,hertz);
      }

   IN_AIL;

   ailcall2(AIL,set_timer_frequency,void,timer,hertz);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void    AILEXPORT  AIL_set_timer_divisor         (HTIMER      timer, //)
                                               U32         PIT_divisor)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_timer_divisor(%ld,%lu)\r\n",timer,PIT_divisor);
      }

   IN_AIL;

   ailcall2(AIL,set_timer_divisor,void,timer,PIT_divisor);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void    AILEXPORT  AIL_start_timer               (HTIMER      timer)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_start_timer(%lu)\r\n",timer);
      }

   IN_AIL;

   ailcall1(AIL,start_timer,void,timer);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void    AILEXPORT  AIL_start_all_timers          (void)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_start_all_timers()\r\n");
      }

   IN_AIL;

   ailcall0(AIL,start_all_timers,void);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void    AILEXPORT  AIL_stop_timer                (HTIMER      timer)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_stop_timer(%lu)\r\n",timer);
      }

   IN_AIL;

   ailcall1(AIL,stop_timer,void,timer);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void    AILEXPORT  AIL_stop_all_timers           (void)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_stop_all_timers()\r\n");
      }

   IN_AIL;

   ailcall0(AIL,stop_all_timers,void);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void    AILEXPORT  AIL_release_timer_handle      (HTIMER      timer)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_release_timer_handle(%lu)\r\n",timer);
      }

   IN_AIL;

   ailcall1(AIL,release_timer_handle,void,timer);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void    AILEXPORT  AIL_release_all_timers        (void)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_release_all_timers()\r\n");
      }

   IN_AIL;

   ailcall0(AIL,release_all_timers,void);

   OUT_AIL;

   END;
}
//############################################################################

DXDEF HDIGDRIVER AILEXPORT AIL_open_digital_driver( U32 frequency,
                                                    S32 bits,
                                                    S32 channels,
                                                    U32 flags )
{
   HDIGDRIVER result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_open_digital_driver(%ld, %ld, %ld, 0x%lX)\r\n",
                                                             frequency,
                                                             bits,
                                                             channels,
                                                             flags);
      }

   IN_AIL;

   #ifdef IS_WINDOWS

   {
     PCMWAVEFORMAT wf;

     wf.wf.wFormatTag      = WAVE_FORMAT_PCM;
     wf.wf.nChannels       = (channels==1)?1:2;
     wf.wf.nSamplesPerSec  = (frequency)?frequency:22050;
     wf.wBitsPerSample     = (bits==8)?8:16;

     wf.wf.nBlockAlign     = (wf.wBitsPerSample / 8) * wf.wf.nChannels;
     wf.wf.nAvgBytesPerSec = (U32)wf.wf.nSamplesPerSec * (U32)wf.wf.nBlockAlign;

     if ((flags&AIL_OPEN_DIGITAL_FORCE_PREFERENCE)==0)
       AIL_set_preference(DIG_USE_WAVEOUT,FALSE);

     if (AIL_waveOutOpen(&result,0,WAVE_MAPPER,(LPWAVEFORMAT)&wf)==0)
     {
       #ifdef IS_WIN32
       if ((result->emulated_ds) && ((flags&AIL_OPEN_DIGITAL_FORCE_PREFERENCE)==0))
       {
         OutMilesMutex();
         AIL_waveOutClose(result);
         InMilesMutex();
         goto trywaveout;
       }
       #endif
     }
     else
     {
       #ifdef IS_WIN32
       if ((flags&AIL_OPEN_DIGITAL_FORCE_PREFERENCE)==0)
       {
        trywaveout:
         AIL_set_preference(DIG_USE_WAVEOUT,TRUE);
         if (AIL_waveOutOpen(&result,0,WAVE_MAPPER,(LPWAVEFORMAT)&wf))
           result=0;
       }
       else
       #endif
         result=0;
     }
   }

   #else

   #ifdef IS_DOS

   if ((flags&AIL_OPEN_DIGITAL_FORCE_PREFERENCE)==0)
   {
     if (frequency)
       AIL_set_preference(DIG_HARDWARE_SAMPLE_RATE, frequency );
     if (bits)
       AIL_set_preference(DIG_USE_16_BITS, (bits>8)?TRUE:FALSE );
     if (channels)
       AIL_set_preference(DIG_USE_STEREO, (channels>1)?TRUE:FALSE );
   }
   if (AIL_install_DIG_INI(&result)!=AIL_INIT_SUCCESS)
     result=0;
   
   #else

   result = AIL_API_open_digital_driver( frequency, bits, channels, flags );

   #endif

   #endif

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}


DXDEF void AILEXPORT AIL_close_digital_driver( HDIGDRIVER dig )
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_close_digital_driver(0x%lX)\r\n",dig);
      }

   IN_AIL_NM;

   #ifdef IS_WINDOWS
   dsailcall1(AIL,waveOutClose,void,dig);
   #else
   #ifdef IS_DOS
   AIL_API_uninstall_DIG_driver(dig)
   #else
   AIL_API_close_digital_driver( dig );
   #endif
   #endif

   OUT_AIL_NM;

   END;
}


#ifdef IS_WINDOWS

DXDEF
S32          AILEXPORT AIL_waveOutOpen        (HDIGDRIVER   FAR *drvr,  //)
                                               LPHWAVEOUT   FAR *lphWaveOut,
                                               S32             dwDeviceID,
                                               LPWAVEFORMAT      lpFormat)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_waveOutOpen(0x%lX, 0x%lX, %d, 0x%lX)\r\n",drvr,
                                                             lphWaveOut,
                                                             dwDeviceID,
                                                             lpFormat);
      }

   IN_AIL;

   result=dsailcall4(AIL,waveOutOpen,S32,drvr,lphWaveOut,(U32)dwDeviceID,lpFormat);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_waveOutClose      (HDIGDRIVER dig)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_waveOutClose(0x%lX)\r\n",dig);
      }

   IN_AIL_NM;

   dsailcall1(AIL,waveOutClose,void,dig);

   OUT_AIL_NM;

   END;
}

#endif

#if defined(IS_WINDOWS) || defined(IS_MAC)

//############################################################################

DXDEF
S32         AILEXPORT AIL_digital_handle_release      (HDIGDRIVER dig)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_digital_handle_release(0x%lX)\r\n",dig);
      }

   IN_AIL;

   result=dsailcall1(AIL,digital_handle_release,S32,dig);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
S32         AILEXPORT AIL_digital_handle_reacquire    (HDIGDRIVER dig)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_digital_handle_reacquire(0x%lX)\r\n",dig);
      }

   IN_AIL;

   result=dsailcall1(AIL,digital_handle_reacquire,S32,dig);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF HDIGINPUT AILEXPORT AIL_open_input          (AIL_INPUT_INFO FAR *info)
{
   HDIGINPUT result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_open_input(0x%lX)\r\n",info);
      }

   IN_AIL;

   result=ailcall1(AIL,open_input,HDIGINPUT,info);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF void      AILEXPORT AIL_close_input         (HDIGINPUT         dig)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_close_input(0x%lX)\r\n",dig);
      }

   IN_AIL;

   ailcall1(AIL,close_input,void,dig);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF AIL_INPUT_INFO FAR * AILEXPORT AIL_get_input_info (HDIGINPUT         dig)
{
   AIL_INPUT_INFO FAR *result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_get_input_info(0x%lX)\r\n",dig);
      }

   IN_AIL;

   result=ailcall1(AIL,get_input_info,AIL_INPUT_INFO FAR *,dig);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF S32       AILEXPORT AIL_set_input_state     (HDIGINPUT         dig, //)
                                                   S32               enable)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_input_state(0x%lX,%ld)\r\n",dig,enable);
      }

   IN_AIL;

   result=ailcall2(AIL,set_input_state,S32,dig,enable);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

#endif

//############################################################################

DXDEF
S32         AILEXPORT AIL_digital_CPU_percent(HDIGDRIVER dig)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_digital_CPU_percent(0x%lX)\r\n",dig);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,digital_CPU_percent,S32,dig);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
S32         AILEXPORT AIL_digital_latency(HDIGDRIVER dig)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_digital_latency(0x%lX)\r\n",dig);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,digital_latency,S32,dig);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
HSAMPLE      AILEXPORT AIL_allocate_sample_handle (HDIGDRIVER dig)

{
   HSAMPLE result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_allocate_sample_handle(0x%lX)\r\n",dig);
      }

   IN_AIL;

   result=dsailcall1(AIL,allocate_sample_handle,HSAMPLE,dig);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
HSAMPLE      AILEXPORT AIL_allocate_file_sample  (HDIGDRIVER dig,          //)
                                               void const FAR *file_image,
                                               S32        block)
{
   HSAMPLE result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_allocate_file_sample(0x%lX,0x%lX,%ld)\r\n",dig,
         file_image,block);
      }

   IN_AIL;

   result=ailcall3(AIL,allocate_file_sample,HSAMPLE,dig,file_image,block);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_release_sample_handle (HSAMPLE S)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_release_sample_handle(0x%lX)\r\n",S);
      }

   IN_AIL;

   dsailcall1(AIL,release_sample_handle,void,S);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_init_sample           (HSAMPLE S)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_init_sample(0x%lX)\r\n",S);
      }

   IN_AIL;

   dsailcall1(AIL,init_sample,void,S);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
S32        AILEXPORT AIL_set_sample_file        (HSAMPLE   S, //)
                                              void const FAR *file_image,
                                              S32       block)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_sample_file(0x%lX,0x%lX,%ld)\r\n",S,file_image,block);
      }

   IN_AIL;

   result=ailcall3(AIL,set_sample_file,S32,S,file_image,block);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
S32          AILEXPORT AIL_set_named_sample_file (HSAMPLE   S, //)
                                                  C8   const FAR *file_type_suffix,
                                                  void const FAR *file_image,
                                                  S32       file_size,
                                                  S32       block)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_named_sample_file(0x%lX,%s,0x%lX,%ld,%ld)\r\n",
         S,file_type_suffix,file_image,file_size,block);
      }

   IN_AIL;

   result=ailcall5(AIL,set_named_sample_file,U32,S,file_type_suffix,file_image,file_size,block);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
HPROVIDER    AILEXPORT AIL_set_sample_processor  (HSAMPLE     S, //)
                                                SAMPLESTAGE pipeline_stage,
                                                HPROVIDER   provider)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_sample_processor(0x%lX,%ld,0x%lX)\r\n",
         S,pipeline_stage,provider);
      }

   IN_AIL;

   result=ailcall3(AIL,set_sample_processor,HPROVIDER,S,pipeline_stage,provider);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
HPROVIDER    AILEXPORT AIL_set_stream_processor(HSTREAM     S, //)
                                                SAMPLESTAGE pipeline_stage,
                                                HPROVIDER   provider)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_stream_processor(0x%lX,%ld,0x%lX)\r\n",
         S,pipeline_stage,provider);
      }

   IN_AIL;

   result=ailcall3(AIL,set_sample_processor,HPROVIDER,S->samp,pipeline_stage,provider);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
HPROVIDER    AILEXPORT AIL_set_DLS_processor   (HDLSDEVICE  dev, //)
                                                SAMPLESTAGE pipeline_stage,
                                                HPROVIDER   provider)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_DLS_processor(0x%lX,%ld,0x%lX)\r\n",
         dev,pipeline_stage,provider);
      }

   IN_AIL;

   result=ailcall3(AIL,set_sample_processor,HPROVIDER,dev->sample,pipeline_stage,provider);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
HPROVIDER    AILEXPORT AIL_set_digital_driver_processor  (HDIGDRIVER  dig, //)
                                                        DIGDRVSTAGE pipeline_stage,
                                                        HPROVIDER   provider)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_digital_driver_processor(0x%lX,%ld,0x%lX)\r\n",
         dig,pipeline_stage,provider);
      }

   IN_AIL;

   result=ailcall3(AIL,set_digital_driver_processor,HPROVIDER,dig,pipeline_stage,provider);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_set_sample_address    (HSAMPLE S, //)
                                               void const    FAR *start,
                                               U32     len)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_sample_address(0x%lX,0x%lX,%lu)\r\n",S,start,len);
      }

   IN_AIL;

   dsailcall3(AIL,set_sample_address,void,S,start,len);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_set_sample_type       (HSAMPLE S, //)
                                               S32     format, 
                                               U32     flags)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_sample_type(0x%lX,%ld,%lu)\r\n",S,format,flags);
      }

   IN_AIL;

   dsailcall3(AIL,set_sample_type,void,S,format,flags);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_set_sample_adpcm_block_size(HSAMPLE S, //)
                                                       U32 blocksize)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_sample_adpcm_block_size(0x%lX,%lu)\r\n",S,blocksize);
      }

   IN_AIL;

   dsailcall2(AIL,set_sample_adpcm_block_size,void,S,blocksize);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_start_sample          (HSAMPLE S)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_start_sample(0x%lX)\r\n",S);
      }

   IN_AIL;

   dsailcall1(AIL,start_sample,void,S);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_stop_sample           (HSAMPLE S)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_stop_sample(0x%lX)\r\n",S);
      }

   IN_AIL;

   dsailcall1(AIL,stop_sample,void,S);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_resume_sample         (HSAMPLE S)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_resume_sample(0x%lX)\r\n",S);
      }

   IN_AIL;

   dsailcall1(AIL,resume_sample,void,S);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_end_sample            (HSAMPLE S)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_end_sample(0x%lX)\r\n",S);
      }

   IN_AIL;

   dsailcall1(AIL,end_sample,void,S);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_set_sample_playback_rate//()
                                             (HSAMPLE S, 
                                              S32     playback_rate)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_sample_playback_rate(0x%lX,%ld)\r\n",S,playback_rate);
      }

   IN_AIL_NM;

   dsailcall2(AIL,set_sample_playback_rate,void,S,playback_rate);

   OUT_AIL_NM;

   END;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_set_sample_volume     (HSAMPLE S, //)
                                               S32     volume)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_sample_volume(0x%lX,%ld)\r\n",S,volume);
      }

   IN_AIL_NM;

   dsailcall2(AIL,set_sample_volume,void,S,volume);

   OUT_AIL_NM;

   END;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_set_sample_pan        (HSAMPLE S, //)
                                               S32     pan)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_sample_pan(0x%lX,%ld)\r\n",S,pan);
      }

   IN_AIL_NM;

   dsailcall2(AIL,set_sample_pan,void,S,pan);

   OUT_AIL_NM;

   END;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_set_sample_loop_count (HSAMPLE S, //)
                                               S32     loop_count)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_sample_loop_count(0x%lX,%ld)\r\n",S,loop_count);
      }

   IN_AIL_NM;

   dsailcall2(AIL,set_sample_loop_count,void,S,loop_count);

   OUT_AIL_NM;

   END;
}


//############################################################################

DXDEF
void         AILEXPORT AIL_set_sample_loop_block (HSAMPLE S,
                                                     S32     loop_start_offset,
                                                     S32     loop_end_offset)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_sample_loop_block(0x%lX,%ld,%ld)\r\n",S,loop_start_offset,loop_end_offset);
      }

   IN_AIL_NM;

   dsailcall3(AIL,set_sample_loop_block,void,S,loop_start_offset,loop_end_offset);

   OUT_AIL_NM;

   END;
}



//############################################################################

DXDEF
U32        AILEXPORT AIL_sample_status         (HSAMPLE S)
{
   U32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_sample_status(0x%lX)\r\n",S);
      }

   IN_AIL_NM;

   result=dsailcall1(AIL,sample_status,U32,S);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
S32         AILEXPORT AIL_sample_playback_rate  (HSAMPLE S)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_sample_playback_rate(0x%lX)\r\n",S);
      }

   IN_AIL_NM;

   result=dsailcall1(AIL,sample_playback_rate,S32,S);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
S32         AILEXPORT AIL_sample_volume         (HSAMPLE S)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_sample_volume(0x%lX)\r\n",S);
      }

   IN_AIL_NM;

   result=dsailcall1(AIL,sample_volume,S32,S);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
S32         AILEXPORT AIL_sample_pan            (HSAMPLE S)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_sample_pan(0x%lX)\r\n",S);
      }

   IN_AIL_NM;

   result=dsailcall1(AIL,sample_pan,S32,S);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
S32         AILEXPORT AIL_sample_loop_count     (HSAMPLE S)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_sample_loop_count(0x%lX)\r\n",S);
      }

   IN_AIL_NM;

   result=dsailcall1(AIL,sample_loop_count,S32,S);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
void   AILEXPORT AIL_set_sample_reverb(HSAMPLE S, //)
                                             F32     reverb_level,
                                             F32     reverb_reflect_time,
                                             F32     reverb_decay_time)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_sample_reverb(0x%X,%f,%f,%f)\r\n",S,reverb_level,reverb_reflect_time,reverb_decay_time);
      }

   IN_AIL;

   ailcall4(AIL,set_sample_reverb,void,S,reverb_level,reverb_reflect_time,reverb_decay_time);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void   AILEXPORT AIL_sample_reverb    (HSAMPLE  S, //)
                                             F32 FAR *reverb_level,
                                             F32 FAR *reverb_reflect_time,
                                             F32 FAR *reverb_decay_time)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_sample_reverb(0x%X,%X,%X,%X)\r\n",S,reverb_level,reverb_reflect_time,reverb_decay_time);
      }

   IN_AIL;

   ailcall4(AIL,sample_reverb,void,S,reverb_level,reverb_reflect_time,reverb_decay_time);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void     AILEXPORT AIL_set_digital_master_volume
                                             (HDIGDRIVER dig,
                                              S32        master_volume)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_digital_master_volume(0x%X,%d)\r\n",dig,master_volume);
      }

   IN_AIL;

   dsailcall2(AIL,set_digital_master_volume,void,dig,master_volume);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
S32     AILEXPORT AIL_digital_master_volume   (HDIGDRIVER dig)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_digital_master_volume(0x%X)\r\n",dig);
      }

   IN_AIL;

   result=dsailcall1(AIL,digital_master_volume,S32,dig);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
S32     AILEXPORT AIL_minimum_sample_buffer_size  (HDIGDRIVER dig, //)
                                                S32        playback_rate,
                                                S32        format)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_minimum_sample_buffer_size(0x%lX,%ld,%ld)\r\n",dig,
         playback_rate,format);
      }

   IN_AIL;

   result=dsailcall3(AIL,minimum_sample_buffer_size,S32,dig,playback_rate,format);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
S32     AILEXPORT AIL_sample_buffer_ready       (HSAMPLE S)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_sample_buffer_ready(0x%lX)\r\n",S);
      }

   IN_AIL_NM;

   result=dsailcall1(AIL,sample_buffer_ready,S32,S);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
void     AILEXPORT AIL_load_sample_buffer        (HSAMPLE S, //)
                                               U32     buff_num,
                                               void const FAR *buffer,
                                               U32     len)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_load_sample_buffer(0x%lX,%ld,0x%lX,%lu)\r\n",
         S,buff_num,buffer,len);
      }

   IN_AIL;

   dsailcall4(AIL,load_sample_buffer,void,S,buff_num,buffer,len);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
S32     AILEXPORT AIL_sample_buffer_info      (HSAMPLE S, //)
            U32     FAR *pos0,
            U32     FAR *len0,
            U32     FAR *pos1,
            U32     FAR *len1)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_sample_buffer_info(0x%lX,0x%lX,0x%lX,0x%lX,0x%lX)\r\n",
                 S,pos0,len0,pos1,len1);
      }

   IN_AIL_NM;

   result=dsailcall5(AIL,sample_buffer_info,S32,S,pos0,len0,pos1,len1);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}


//############################################################################

DXDEF
U32    AILEXPORT AIL_sample_granularity        (HSAMPLE S)
{
   U32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_sample_granularity(0x%lX)\r\n",S);
      }

   IN_AIL_NM;

   result=dsailcall1(AIL,sample_granularity,U32,S);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
void     AILEXPORT AIL_set_sample_position       (HSAMPLE S, //)
                                               U32     pos)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_sample_position(0x%lX,%lu)\r\n",S,pos);
      }

   IN_AIL;

   dsailcall2(AIL,set_sample_position,void,S,pos);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
U32    AILEXPORT AIL_sample_position           (HSAMPLE S)
{
   U32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_sample_position(0x%lX)\r\n",S);
      }

   IN_AIL_NM;

   result=dsailcall1(AIL,sample_position,U32,S);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
AILSAMPLECB AILEXPORT AIL_register_SOB_callback     (HSAMPLE S, //) 
                                                    AILSAMPLECB SOB)
{
   AILSAMPLECB result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_register_SOB_callback(0x%lX,0x%lX)\r\n",S,SOB);
      }

   IN_AIL;

   result=dsailcall2(AIL,register_SOB_callback,AILSAMPLECB,S,SOB);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
AILSAMPLECB AILEXPORT AIL_register_EOB_callback     (HSAMPLE S, //) 
                                                    AILSAMPLECB EOB)
{
   AILSAMPLECB result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_register_EOB_callback(0x%lX,0x%lX)\r\n",S,EOB);
      }

   IN_AIL;

   result=dsailcall2(AIL,register_EOB_callback,AILSAMPLECB,S,EOB);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
AILSAMPLECB AILEXPORT AIL_register_EOS_callback     (HSAMPLE S, //) 
                                                    AILSAMPLECB EOS)
{
   AILSAMPLECB result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_register_EOS_callback(0x%lX,0x%lX)\r\n",S,EOS);
      }

   IN_AIL;

   result=dsailcall2(AIL,register_EOS_callback,AILSAMPLECB,S,EOS);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
AILSAMPLECB AILEXPORT AIL_register_EOF_callback     (HSAMPLE S, //)
                                                 AILSAMPLECB EOFILE)
{
   AILSAMPLECB result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_register_EOF_callback(0x%lX,0x%lX)\r\n",S, EOFILE);
      }

   IN_AIL;

   result=ailcall2(AIL,register_EOF_callback,AILSAMPLECB,S,EOFILE);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
void     AILEXPORT AIL_set_sample_user_data      (HSAMPLE S, //)
                                               U32     index,
                                               S32     value)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_sample_user_data(0x%lX,%ld,%ld)\r\n",S,index,value);
      }

   IN_AIL_NM;

   dsailcall3(AIL,set_sample_user_data,void,S,index,value);

   OUT_AIL_NM;

   END;
}

//############################################################################

DXDEF
S32     AILEXPORT AIL_sample_user_data          (HSAMPLE S, //)
                                              U32     index)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_sample_user_data(0x%lX,%lu)\r\n",S,index);
      }

   IN_AIL_NM;

   result=dsailcall2(AIL,sample_user_data,S32,S,index);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
S32     AILEXPORT AIL_active_sample_count       (HDIGDRIVER dig)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_active_sample_count(0x%lX)\r\n",dig);
      }

   IN_AIL;

   result=dsailcall1(AIL,active_sample_count,S32,dig);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

void     AILEXPORT AIL_digital_configuration     (HDIGDRIVER dig,
                                               S32   FAR *rate,
                                               S32   FAR *format,
                                               char  FAR *config)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_digital_configuration(0x%lX,0x%lX,0x%lX,0xlX)\r\n",
         dig,rate,format,config);
      }

   IN_AIL;

   dsailcall4(AIL,digital_configuration,void,dig,rate,format,config);

   OUT_AIL;

   END;
}


#ifdef IS_WIN32

//############################################################################

DXDEF
void     AILEXPORT AIL_get_DirectSound_info (HSAMPLE              S,
                                          LPVOID               *lplpDS,
                                          LPVOID               *lplpDSB)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_get_DirectSound_info(0x%lX,0x%lX,0x%lX)\r\n",
         S,lplpDS,lplpDSB);
      }

   IN_AIL;

   if (AIL_preference[DIG_USE_WAVEOUT]==NO)
      {
      AIL_API_get_DirectSound_info(S,lplpDS,lplpDSB);
      }

   OUT_AIL;

   END;
}


//############################################################################

DXDEF
S32      AILEXPORT  AIL_set_DirectSound_HWND(HDIGDRIVER dig, HWND wnd)
{
   S32 result=0;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_DirectSound_HWND(0x%lX, 0x%lX)\r\n",dig,wnd);
      }

   IN_AIL;

   if (AIL_preference[DIG_USE_WAVEOUT]==NO)
      {
      result=AIL_API_set_DirectSound_HWND(dig,wnd);
      }

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

#endif


//############################################################################

DXDEF HMDIDRIVER AILEXPORT AIL_open_XMIDI_driver( U32 flags )
{
   HMDIDRIVER result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_open_XMIDI_driver(0x%lX)\r\n",
                                                       flags);
      }

   IN_AIL;

   #ifdef IS_WINDOWS

   if (AIL_midiOutOpen(&result,0,(flags&AIL_OPEN_XMIDI_NULL_DRIVER)?(U32)MIDI_NULL_DRIVER:(U32)MIDI_MAPPER))
     result=0;

   #else

   #ifdef IS_DOS

   if (flags&AIL_OPEN_XMIDI_NULL_DRIVER)
   {
     result=AIL_install_MDI_driver_file("NULL.MDI",0);
   }
   else
   {
     if (AIL_install_MDI_INI(&result)!=AIL_INIT_SUCCESS)
       result=0;
   }

   #else

   result = AIL_API_open_XMIDI_driver( flags );

   #endif

   #endif

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}


DXDEF void AILEXPORT AIL_close_XMIDI_driver( HMDIDRIVER mdi )
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_close_XMIDI_driver(0x%lX)\r\n",mdi);
      }

   IN_AIL;

   #ifdef IS_WINDOWS
   dsailcall1(AIL,midiOutClose,void,mdi);
   #else
   #ifdef IS_DOS
   AIL_API_uninstall_MDI_driver(mdi)
   #else
   AIL_API_close_XMIDI_driver( mdi );
   #endif
   #endif

   OUT_AIL;

   END;
}


#ifdef IS_WINDOWS

DXDEF
S32          AILEXPORT AIL_midiOutOpen      (HMDIDRIVER FAR *drvr,   //)
                                             LPHMIDIOUT FAR *lphMidiOut,
                                             S32         dwDeviceID)

{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_midiOutOpen(0x%lX, 0x%lX, %d)\r\n",drvr,lphMidiOut,dwDeviceID);
      }

   IN_AIL;

   result=ailcall3(AIL,midiOutOpen,S32,drvr,lphMidiOut,(U32)dwDeviceID);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_midiOutClose      (HMDIDRIVER mdi)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_midiOutClose(0x%lX)\r\n",mdi);
      }

   IN_AIL;

   ailcall1(AIL,midiOutClose,void,mdi);

   OUT_AIL;

   END;
}

#endif

//############################################################################

#if defined(IS_WINDOWS) || defined(IS_MAC)

DXDEF
S32         AILEXPORT AIL_MIDI_handle_release     (HMDIDRIVER mdi)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_MIDI_handle_release(0x%lX)\r\n",mdi);
      }

   IN_AIL;

   result=ailcall1(AIL,MIDI_handle_release,S32,mdi);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
S32         AILEXPORT AIL_MIDI_handle_reacquire     (HMDIDRIVER mdi)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_MIDI_handle_reacquire(0x%lX)\r\n",mdi);
      }

   IN_AIL;

   result=ailcall1(AIL,MIDI_handle_reacquire,S32,mdi);

   OUT_AIL;

   RESULT
      {
      INDENT;

      AIL_fprintf(AIL_debugfile,"Result = %lu\r\n",result);
      }

   END;

   return result;
}

#endif

//############################################################################

DXDEF
HSEQUENCE    AILEXPORT AIL_allocate_sequence_handle//()
                                             (HMDIDRIVER mdi)
{
   HSEQUENCE result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_allocate_sequence_handle(0x%lX)\r\n",mdi);
      }

   IN_AIL;

   result=ailcall1(AIL,allocate_sequence_handle,HSEQUENCE,mdi);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_release_sequence_handle//()
                                             (HSEQUENCE S)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_release_sequence_handle(0x%lX)\r\n",S);
      }

   IN_AIL;

   ailcall1(AIL,release_sequence_handle,void,S);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
S32         AILEXPORT AIL_init_sequence         (HSEQUENCE S, //)
                                              void const FAR *start,
                                              S32       sequence_num)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_init_sequence(0x%lX,0x%lX,%ld)\r\n",S,start,sequence_num);
      }

   IN_AIL;

   result=ailcall3(AIL,init_sequence,S32,S,start,sequence_num);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_start_sequence        (HSEQUENCE S)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_start_sequence(0x%lX)\r\n",S);
      }

   IN_AIL;

   ailcall1(AIL,start_sequence,void,S);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_stop_sequence         (HSEQUENCE S)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_stop_sequence(0x%lX)\r\n",S);
      }

   IN_AIL;

   ailcall1(AIL,stop_sequence,void,S);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_resume_sequence       (HSEQUENCE S)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_resume_sequence(0x%lX)\r\n",S);
      }

   IN_AIL;

   ailcall1(AIL,resume_sequence,void,S);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_end_sequence          (HSEQUENCE S)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_end_sequence(0x%lX)\r\n",S);
      }

   IN_AIL;

   ailcall1(AIL,end_sequence,void,S);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_set_sequence_tempo    (HSEQUENCE S, //)
                                               S32       tempo,
                                               S32       milliseconds)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_sequence_tempo(0x%lX,%ld,%ld)\r\n",S,tempo,
         milliseconds);
      }

   IN_AIL;

   ailcall3(AIL,set_sequence_tempo,void,S,tempo,milliseconds);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_set_sequence_volume   (HSEQUENCE S, //)
                                               S32       volume,
                                               S32       milliseconds)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_sequence_volume(0x%lX,%ld,%ld)\r\n",S,volume,
         milliseconds);
      }

   IN_AIL_NM;

   ailcall3(AIL,set_sequence_volume,void,S,volume,milliseconds);

   OUT_AIL_NM;

   END;
}

//############################################################################

DXDEF
void         AILEXPORT AIL_set_sequence_loop_count (HSEQUENCE S, //)
                                                 S32       loop_count)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_sequence_loop_count(0x%lX,%ld)\r\n",S,loop_count);
      }

   IN_AIL_NM;

   ailcall2(AIL,set_sequence_loop_count,void,S,loop_count);

   OUT_AIL_NM;

   END;
}

//############################################################################

DXDEF
U32        AILEXPORT AIL_sequence_status       (HSEQUENCE S)
{
   U32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_sequence_status(0x%lX)\r\n",S);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,sequence_status,U32,S);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
S32         AILEXPORT AIL_sequence_tempo        (HSEQUENCE S)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_sequence_tempo(0x%lX)\r\n",S);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,sequence_tempo,S32,S);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
S32         AILEXPORT AIL_sequence_volume       (HSEQUENCE S)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_sequence_volume(0x%lX)\r\n",S);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,sequence_volume,S32,S);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
S32         AILEXPORT AIL_sequence_loop_count   (HSEQUENCE S)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_sequence_loop_count(0x%lX)\r\n",S);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,sequence_loop_count,S32,S);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
void     AILEXPORT AIL_set_XMIDI_master_volume
                                             (HMDIDRIVER mdi,
                                              S32        master_volume)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_XMIDI_master_volume(0x%X,%d)\r\n",mdi,master_volume);
      }

   IN_AIL;

   ailcall2(AIL,set_XMIDI_master_volume,void,mdi,master_volume);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
S32     AILEXPORT AIL_XMIDI_master_volume   (HMDIDRIVER mdi)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_XMIDI_master_volume(0x%X)\r\n",mdi);
      }

   IN_AIL;

   result=ailcall1(AIL,XMIDI_master_volume,S32,mdi);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
S32     AILEXPORT AIL_active_sequence_count     (HMDIDRIVER mdi)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_active_sequence_count(0x%lX)\r\n",mdi);
      }

   IN_AIL;

   result=ailcall1(AIL,active_sequence_count,S32,mdi);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
S32     AILEXPORT AIL_controller_value          (HSEQUENCE S, //)
                                              S32       channel,
                                              S32       controller_num)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_controller_value(0x%lX,%ld,%ld)\r\n",S,channel,
         controller_num);
      }

   IN_AIL_NM;

   result=ailcall3(AIL,controller_value,S32,S,channel,controller_num);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
S32     AILEXPORT AIL_channel_notes             (HSEQUENCE S, //)
                                              S32       channel)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_channel_notes(0x%lX,%ld)\r\n",S,channel);
      }

   IN_AIL;

   result=ailcall2(AIL,channel_notes,S32,S,channel);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
void     AILEXPORT AIL_sequence_position         (HSEQUENCE S, //)
                                                  S32       FAR *beat,
                                                  S32       FAR *measure)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_sequence_position(0x%lX,0x%lX,0x%lX)\r\n",S,beat,measure);
      }

   IN_AIL_NM;

   ailcall3(AIL,sequence_position,void,S,beat,measure);

   OUT_AIL_NM;

   END;
}

//############################################################################

DXDEF
void     AILEXPORT AIL_branch_index              (HSEQUENCE S, //)
                                               U32       marker)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_branch_index(0x%lX,%lu)\r\n",S,marker);
      }

   IN_AIL;

   ailcall2(AIL,branch_index,void,S,marker);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
AILPREFIXCB AILEXPORT AIL_register_prefix_callback  (HSEQUENCE S, //)
                                               AILPREFIXCB   callback)
{
   AILPREFIXCB result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_register_prefix_callback(0x%lX,0x%lX)\r\n",
         S,callback);
      }

   IN_AIL;

   result=ailcall2(AIL,register_prefix_callback,AILPREFIXCB,S,callback);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
AILTRIGGERCB AILEXPORT AIL_register_trigger_callback (HSEQUENCE S, //)
                                              AILTRIGGERCB   callback)
{
   AILTRIGGERCB result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_register_trigger_callback(0x%lX,0x%lX)\r\n",
         S,callback);
      }

   IN_AIL;

   result=ailcall2(AIL,register_trigger_callback,AILTRIGGERCB,S,callback);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
AILSEQUENCECB AILEXPORT AIL_register_sequence_callback(HSEQUENCE S, //)
                                              AILSEQUENCECB   callback)
{
   AILSEQUENCECB result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_register_sequence_callback(0x%lX,0x%lX)\r\n",
         S,callback);
      }

   IN_AIL;

   result=ailcall2(AIL,register_sequence_callback,AILSEQUENCECB,S,callback);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
AILBEATCB AILEXPORT AIL_register_beat_callback(HSEQUENCE S, //)
                                              AILBEATCB   callback)
{
   AILBEATCB result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_register_beat_callback(0x%lX,0x%lX)\r\n",
         S,callback);
      }

   IN_AIL;

   result=ailcall2(AIL,register_beat_callback,AILBEATCB,S,callback);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
AILEVENTCB AILEXPORT AIL_register_event_callback   (HMDIDRIVER mdi, //)
                                                    AILEVENTCB callback)
{
   AILEVENTCB result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_register_event_callback(0x%lX,0x%lX)\r\n",
         mdi,callback);
      }

   IN_AIL;

   result=ailcall2(AIL,register_event_callback,AILEVENTCB,mdi,callback);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
AILTIMBRECB AILEXPORT AIL_register_timbre_callback  (HMDIDRIVER mdi, //)
                                                     AILTIMBRECB callback)
{
   AILTIMBRECB result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_register_timbre_callback(0x%lX,0x%lX)\r\n",
         mdi, callback);
      }

   IN_AIL;

   result=ailcall2(AIL,register_timbre_callback,AILTIMBRECB,mdi,callback);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
void     AILEXPORT AIL_set_sequence_user_data    (HSEQUENCE S, //)
                                               U32       index,
                                               S32       value)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_sequence_user_data(0x%lX,%ld,%ld)\r\n",S,index,value);
      }

   IN_AIL_NM;

   ailcall3(AIL,set_sequence_user_data,void,S,index,value);

   OUT_AIL_NM;

   END;
}

//############################################################################

DXDEF
S32     AILEXPORT AIL_sequence_user_data        (HSEQUENCE S, //)
                                              U32       index)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_sequence_user_data(0x%lX,%lu)\r\n",S,index);
      }

   IN_AIL_NM;

   result=ailcall2(AIL,sequence_user_data,S32,S,index);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
void     AILEXPORT AIL_register_ICA_array        (HSEQUENCE   S,  //)
                                               U8     FAR *array)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_register_ICA_array(0x%lX,0x%lX)\r\n",S,array);
      }

   IN_AIL;

   ailcall2(AIL,register_ICA_array,void,S,array);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
S32     AILEXPORT AIL_lock_channel              (HMDIDRIVER mdi)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_lock_channel(0x%lX)\r\n",mdi);
      }

   IN_AIL;

   result=ailcall1(AIL,lock_channel,S32,mdi);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
void     AILEXPORT AIL_release_channel           (HMDIDRIVER mdi, //)
                                               S32        channel)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_release_channel(0x%lX,%ld)\r\n",mdi,channel);
      }

   IN_AIL;

   ailcall2(AIL,release_channel,void,mdi,channel);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void     AILEXPORT AIL_map_sequence_channel      (HSEQUENCE S, //)
                                               S32       seq_channel,
                                               S32       new_channel)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_map_sequence_channel(0x%lX,%ld,%ld)\r\n",
         S,seq_channel,new_channel);
      }

   IN_AIL;

   ailcall3(AIL,map_sequence_channel,void,S,seq_channel,new_channel);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
S32     AILEXPORT AIL_true_sequence_channel     (HSEQUENCE S, //)
                                              S32       seq_channel)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_true_sequence_channel(0x%lX,%ld)\r\n",
         S,seq_channel);
      }

   IN_AIL;

   result=ailcall2(AIL,true_sequence_channel,S32,S,seq_channel);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
void     AILEXPORT AIL_send_channel_voice_message(HMDIDRIVER mdi, //)
                                               HSEQUENCE  S,
                                               S32        status,
                                               S32        data_1,
                                               S32        data_2)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_send_channel_voice_message(0x%lX,0x%lX,0x%lX,0x%lX,0x%lX)\r\n",
         mdi,S,status,data_1,data_2);
      }

   IN_AIL;

   ailcall5(AIL,send_channel_voice_message,void,mdi,S,status,data_1,data_2);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void     AILEXPORT AIL_send_sysex_message        (HMDIDRIVER mdi, //)
                                               void const FAR *buffer)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_send_sysex_message(0x%lX,0x%lX)\r\n",mdi,buffer);
      }

   IN_AIL;

   ailcall2(AIL,send_sysex_message,void,mdi,buffer);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
HWAVESYNTH    AILEXPORT AIL_create_wave_synthesizer   (HDIGDRIVER dig,  //)
                                                    HMDIDRIVER mdi,
                                                    void const FAR *wave_lib,
                                                    S32        polyphony)
{
   HWAVESYNTH result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_create_wave_synthesizer(0x%lX,0x%lX,0x%lX,%ld)\r\n",
         dig,mdi,wave_lib,polyphony);
      }

   IN_AIL;

   result=ailcall4(AIL,create_wave_synthesizer,HWAVESYNTH,dig,mdi,wave_lib,polyphony);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
void     AILEXPORT AIL_destroy_wave_synthesizer  (HWAVESYNTH W)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_destroy_wave_synthesizer(0x%lX)\r\n",W);
      }

   IN_AIL;

   ailcall1(AIL,destroy_wave_synthesizer,void,W);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
S32        AILEXPORT AIL_file_error  (void)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_file_error()\r\n");
      }

   IN_AIL_NM;

   result=ailcall0(AIL,file_error,S32);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
S32        AILEXPORT AIL_file_size   (char const FAR   *filename)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_file_size(%s)\r\n",filename);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,file_size,S32,filename);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
void FAR * AILEXPORT AIL_file_read   (char const FAR   *filename,
                                          void FAR *dest)
{
   void FAR * result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_file_read(%s,0x%lX)\r\n",filename,dest);
      }

   IN_AIL_NM;

   result=ailcall2(AIL,file_read,void FAR*,filename,dest);

   OUT_AIL_NM;

   RESULT
      {
      outreshex((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
S32        AILEXPORT AIL_file_write  (char const FAR   *filename,
                                          void const FAR *buf,
                                          U32       len)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_file_write(%s,0x%lX,%ld)\r\n",filename,buf,len);
      }

   IN_AIL_NM;

   result=ailcall3(AIL,file_write,S32,filename,buf,len);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
S32        AILEXPORT AIL_WAV_file_write  (char const FAR   *filename,
                                          void const FAR *buf,
                                          U32       len,
                                          S32       rate,
                                          S32       format)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_WAV_file_write(%s,0x%lX,%ld,%ld,%ld)\r\n",filename,buf,len,rate,format);
      }

   IN_AIL_NM;

   result=ailcall5(AIL,WAV_file_write,S32,filename,buf,len,rate,format);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


#ifdef IS_MAC
//############################################################################

DXDEF
S32        AILEXPORT AIL_file_fss_size   (FSSpec const *spec)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_file_fss_size(0x%lX)\r\n",spec);
      }

   IN_AIL;

   result=ailcall1(AIL,file_fss_size,S32,spec);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
void FAR * AILEXPORT AIL_file_fss_read   (FSSpec const *spec,
                                          void FAR *dest)
{
   void FAR * result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_file_fss_read(0x%lX,0x%lX)\r\n",spec,dest);
      }

   IN_AIL;

   result=ailcall2(AIL,file_fss_read,void FAR*,spec,dest);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
S32        AILEXPORT AIL_file_fss_write  (FSSpec const *spec,
                                          void const FAR *buf,
                                          U32       len)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_file_fss_write(0x%lX,0x%lX,%ld)\r\n",spec,buf,len);
      }

   IN_AIL;

   result=ailcall3(AIL,file_fss_write,S32,spec,buf,len);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}

//############################################################################

DXDEF
S32        AILEXPORT AIL_file_fss_attrib_write  (FSSpec const *spec,
                                          void FAR const *buf,
                                          U32       len, 
                                          U32 creator, 
                                          U32 type )
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_file_fss_attrib_write(0x%lX,0x%lX,%ld,%ld,%ld)\r\n",spec,buf,len,creator,type);
      }

   IN_AIL;

   result=ailcall5(AIL,file_fss_attrib_write,S32,spec,buf,len,creator,type);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
S32        AILEXPORT AIL_WAV_file_fss_write  (FSSpec const *spec,
                                          void const FAR *buf,
                                          U32       len,
                                          S32       rate,
                                          S32       format)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_WAV_file_fss_write(0x%lX,0x%lX,%ld,%ld,%ld)\r\n",spec,buf,len,rate,format);
      }

   IN_AIL;

   result=ailcall5(AIL,WAV_file_fss_write,S32,spec,buf,len,rate,format);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}
#endif

//############################################################################

#if defined(IS_WINDOWS) || defined(IS_MAC)

DXDEF
void       AILEXPORT AIL_serve()
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_serve()\r\n");
      }

   IN_AIL_NM;

   ailcall0(AIL,serve,void);

   OUT_AIL_NM;

   END;
}

#endif

//############################################################################

DXDEF
HREDBOOK AILEXPORT AIL_redbook_open(U32 which)
{
   HREDBOOK result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_redbook_open(%ld)\r\n",which);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,redbook_open,HREDBOOK,which);

   OUT_AIL_NM;

   RESULT
      {
      outreshex((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
HREDBOOK AILEXPORT 
#ifdef IS_MAC
AIL_redbook_open_volume(char const* drive)
#else
AIL_redbook_open_drive(S32 drive)
#endif
{
   HREDBOOK result;

   START
      {
      AIL_fprintf(AIL_debugfile,
#ifdef IS_MAC
      "AIL_redbook_open_volume(%s:)\r\n"
#else
      "AIL_redbook_open_drive(%c:)\r\n"
#endif
      ,drive);
      }

   IN_AIL_NM;

#ifdef IS_MAC
   result=ailcall1(AIL,redbook_open_volume,HREDBOOK,drive);
#else
   result=ailcall1(AIL,redbook_open_drive,HREDBOOK,drive);
#endif

   OUT_AIL_NM;

   RESULT
      {
      outreshex((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
void AILEXPORT AIL_redbook_close(HREDBOOK hand)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_redbook_close(0x%lX)\r\n",hand);
      }

   IN_AIL_NM;

   ailcall1(AIL,redbook_close,void,hand);

   OUT_AIL_NM;

   END;
}


//############################################################################

DXDEF
void AILEXPORT AIL_redbook_eject(HREDBOOK hand)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_redbook_eject(0x%lX)\r\n",hand);
      }

   IN_AIL_NM;

   ailcall1(AIL,redbook_eject,void,hand);

   OUT_AIL_NM;

   END;
}


//############################################################################

DXDEF
void AILEXPORT AIL_redbook_retract(HREDBOOK hand)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_redbook_retract(0x%lX)\r\n",hand);
      }

   IN_AIL_NM;

   ailcall1(AIL,redbook_retract,void,hand);

   OUT_AIL_NM;

   END;
}


//############################################################################

DXDEF
U32 AILEXPORT AIL_redbook_status(HREDBOOK hand)
{
   U32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_redbook_status(0x%lX)\r\n",hand);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,redbook_status,U32,hand);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
U32 AILEXPORT AIL_redbook_tracks(HREDBOOK hand)
{
   U32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_redbook_tracks(0x%lX)\r\n",hand);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,redbook_tracks,U32,hand);

   OUT_AIL_NM;

   RESULT
      {
      INDENT;

      AIL_fprintf(AIL_debugfile,"Result = %ld\r\n",result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
U32 AILEXPORT AIL_redbook_track(HREDBOOK hand)
{
   U32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_redbook_track(0x%lX)\r\n",hand);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,redbook_track,U32,hand);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
void AILEXPORT AIL_redbook_track_info(HREDBOOK hand,U32 tracknum,U32 FAR* startmsec,U32 FAR* endmsec)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_redbook_track_info(0x%lX, %ld, 0x%lX, 0x%lX)\r\n",hand,tracknum,startmsec,endmsec);
      }

   IN_AIL_NM;

   ailcall4(AIL,redbook_track_info,void,hand,tracknum,startmsec,endmsec);

   OUT_AIL_NM;

   END;
}


//############################################################################

DXDEF
U32 AILEXPORT AIL_redbook_id(HREDBOOK hand)
{
   U32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_redbook_id(0x%lX)\r\n",hand);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,redbook_id,U32,hand);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
U32 AILEXPORT AIL_redbook_position(HREDBOOK hand)
{
   U32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_redbook_position(0x%lX)\r\n",hand);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,redbook_position,U32,hand);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
U32 AILEXPORT AIL_redbook_play(HREDBOOK hand,U32 startmsec, U32 endmsec)
{
   U32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_redbook_play(0x%lX, %ld, %ld)\r\n",hand,startmsec,endmsec);
      }

   IN_AIL_NM;

   result=ailcall3(AIL,redbook_play,U32,hand,startmsec,endmsec);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
U32 AILEXPORT AIL_redbook_stop(HREDBOOK hand)
{
   U32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_redbook_stop(0x%lX)\r\n",hand);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,redbook_stop,U32,hand);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
U32 AILEXPORT AIL_redbook_pause(HREDBOOK hand)
{
   U32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_redbook_pause(0x%lX)\r\n",hand);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,redbook_pause,U32,hand);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
U32 AILEXPORT AIL_redbook_resume(HREDBOOK hand)
{
   U32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_redbook_resume(0x%lX)\r\n",hand);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,redbook_resume,U32,hand);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
S32 AILEXPORT AIL_redbook_volume(HREDBOOK hand)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_redbook_volume(0x%lX)\r\n",hand);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,redbook_volume,S32,hand);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
S32 AILEXPORT AIL_redbook_set_volume(HREDBOOK hand, S32 volume)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_redbook_set_volume(0x%lX,%ld)\r\n",hand,volume);
      }

   IN_AIL_NM;

   result=ailcall2(AIL,redbook_set_volume,S32,hand,volume);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

#if defined(IS_WINDOWS) || defined(IS_MAC)

DXDEF
S32   AILEXPORT AIL_quick_startup             (S32         use_digital,
                                               S32         use_MIDI,
                                               U32         output_rate,
                                               S32         output_bits,
                                               S32         output_channels)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_startup(%ld, %ld, %ld, %ld, %ld )\r\n",use_digital,use_MIDI,output_rate,output_bits,output_channels);
      }

   IN_AIL_NM;

   result=ailcall5(AIL,quick_startup,S32,use_digital,use_MIDI,output_rate,output_bits,output_channels);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}

#endif

//############################################################################

DXDEF
void   AILEXPORT AIL_quick_shutdown            (void)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_shutdown()\r\n");
      }

   IN_AIL_NM;

   ailcall0(AIL,quick_shutdown,void);

   OUT_AIL_NM;

   END;
}


//############################################################################

DXDEF
void   AILEXPORT AIL_quick_handles ( HDIGDRIVER FAR* pdig, HMDIDRIVER FAR* pmdi, HDLSDEVICE FAR* pdls )
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_handles( 0x%lX, 0x%lX , 0x%lX )\r\n",pdig,pmdi,pdls);
      }

   IN_AIL_NM;

   ailcall3(AIL,quick_handles,void,pdig,pmdi,pdls);

   OUT_AIL_NM;

   END;
}


//############################################################################

DXDEF
HAUDIO  AILEXPORT AIL_quick_load                (char const   FAR *filename)
{
   HAUDIO result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_load(%s)\r\n",filename);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,quick_load,HAUDIO,filename);

   OUT_AIL_NM;

   RESULT
      {
      outreshex((U32)result);
      }
   END;

   return result;
}


//############################################################################

#ifdef IS_MAC

DXDEF
HAUDIO  AILEXPORT AIL_quick_fss_load            (FSSpec const   FAR *filename)
{
   HAUDIO result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_fss_load(0x%lX)\r\n",filename);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,quick_fss_load,HAUDIO,filename);

   OUT_AIL_NM;

   RESULT
      {
      outreshex((U32)result);
      }
   END;

   return result;
}

#endif

//############################################################################

DXDEF
HAUDIO  AILEXPORT AIL_quick_load_mem            (void const   FAR *mem,
                                                 U32    size)
{
   HAUDIO result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_load_mem(0x%lX,%lu)\r\n",mem,size);
      }

   IN_AIL_NM;

   result=ailcall2(AIL,quick_load_mem,HAUDIO,mem,size);

   OUT_AIL_NM;

   RESULT
      {
      outreshex((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
HAUDIO  AILEXPORT AIL_quick_copy                (HAUDIO audio)
{
   HAUDIO result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_copy(0x%lX)\r\n",audio);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,quick_copy,HAUDIO,audio);

   OUT_AIL_NM;

   RESULT
      {
      outreshex((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
void    AILEXPORT AIL_quick_unload              (HAUDIO      audio)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_unload(0x%lX)\r\n",audio);
      }

   IN_AIL_NM;

   ailcall1(AIL,quick_unload,void,audio);

   OUT_AIL_NM;

   END;
}


//############################################################################

DXDEF
S32    AILEXPORT AIL_quick_play                (HAUDIO      audio,
                                                U32         loop_count)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_play(0x%lX, %ld)\r\n",audio,loop_count);
      }

   IN_AIL_NM;

   result=ailcall2(AIL,quick_play,S32,audio,loop_count);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
void   AILEXPORT AIL_quick_halt                (HAUDIO      audio)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_halt(0x%lX)\r\n",audio);
      }

   IN_AIL_NM;

   ailcall1(AIL,quick_halt,void,audio);

   OUT_AIL_NM;

   END;
}


//############################################################################

DXDEF
S32     AILEXPORT AIL_quick_status              (HAUDIO      audio)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_status(0x%lX)\r\n",audio);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,quick_status,S32,audio);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
HAUDIO  AILEXPORT AIL_quick_load_and_play       (char const  FAR *filename,
                                                 U32         loop_count,
                                                 S32         wait_request)
{
   HAUDIO result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_load_and_play(%s, %ld, %ld)\r\n",filename,loop_count,wait_request);
      }

   IN_AIL_NM;

   result=ailcall3(AIL,quick_load_and_play,HAUDIO,filename,loop_count,wait_request);

   OUT_AIL_NM;

   RESULT
      {
      outreshex((U32)result);
      }
   END;

   return result;
}

//############################################################################

#ifdef IS_MAC

DXDEF
HAUDIO  AILEXPORT AIL_quick_fss_load_and_play   (FSSpec const  FAR *filename,
                                                 U32         loop_count,
                                                 S32         wait_request)
{
   HAUDIO result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_fss_load_and_play(0x%lX, %ld, %ld)\r\n",filename,loop_count,wait_request);
      }

   IN_AIL_NM;

   result=ailcall3(AIL,quick_fss_load_and_play,HAUDIO,filename,loop_count,wait_request);

   OUT_AIL_NM;

   RESULT
      {
      outreshex((U32)result);
      }
   END;

   return result;
}

#endif

//############################################################################

DXDEF
void   AILEXPORT AIL_quick_set_speed     (HAUDIO      audio, S32 speed)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_set_speed(0x%lX, %ld)\r\n",audio,speed);
      }

   IN_AIL_NM;

   ailcall2(AIL,quick_set_speed,void,audio,speed);

   OUT_AIL_NM;

   END;
}


//############################################################################

DXDEF
void   AILEXPORT AIL_quick_set_volume (HAUDIO audio, S32 volume, S32 extravol)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_set_volume(0x%lX, %ld, %ld)\r\n",audio,volume,extravol);
      }

   IN_AIL_NM;

   ailcall3(AIL,quick_set_volume,void,audio,volume,extravol);

   OUT_AIL_NM;

   END;
}


//############################################################################

DXDEF
void   AILEXPORT AIL_quick_set_reverb (HAUDIO audio,
                                           F32     reverb_level,
                                           F32     reverb_reflect_time,
                                           F32     reverb_decay_time)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_set_reverb(0x%lX, %f, %f, %f)\r\n",audio,reverb_level,reverb_reflect_time,reverb_decay_time);
      }

   IN_AIL_NM;

   ailcall4(AIL,quick_set_reverb,void,audio,reverb_level,reverb_reflect_time,reverb_decay_time);

   OUT_AIL_NM;

   END;
}


//############################################################################

DXDEF
HSTREAM AILEXPORT AIL_open_stream(HDIGDRIVER dig, char const FAR* filename, S32 stream_mem)
{
   HSTREAM result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_open_stream(0x%lX, %s, %ld)\r\n",dig,filename,stream_mem);
      }

   IN_AIL;

   result=ailcall3(AIL,open_stream,HSTREAM,dig,filename,stream_mem);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
void AILEXPORT AIL_close_stream(HSTREAM stream)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_close_stream(0x%lX)\r\n",stream);
      }

   IN_AIL;

   ailcall1(AIL,close_stream,void,stream);

   OUT_AIL;

   END;
}


//############################################################################

DXDEF
S32 AILEXPORT AIL_service_stream(HSTREAM stream, S32 fillup)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_service_stream(0x%lX, %ld)\r\n",stream,fillup);
      }

   IN_AIL_NM;

   result=ailcall2(AIL,service_stream,S32,stream,fillup);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
void AILEXPORT AIL_start_stream(HSTREAM stream)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_start_stream(0x%lX)\r\n",stream);
      }

   IN_AIL;

   ailcall1(AIL,start_stream,void,stream);

   OUT_AIL;

   END;
}


//############################################################################

DXDEF
void AILEXPORT AIL_pause_stream(HSTREAM stream, S32 onoff)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_pause_stream(0x%lX, %ld)\r\n",stream,onoff);
      }

   IN_AIL;

   ailcall2(AIL,pause_stream,void,stream,onoff);

   OUT_AIL;

   END;
}


//############################################################################

DXDEF
void AILEXPORT AIL_set_stream_volume(HSTREAM stream,S32 volume)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_stream_volume(0x%lX, %ld)\r\n",stream,volume);
      }

   IN_AIL_NM;

   ailcall2(AIL,set_stream_volume,void,stream,volume);

   OUT_AIL_NM;

   END;
}


//############################################################################

DXDEF
void AILEXPORT AIL_set_stream_pan(HSTREAM stream,S32 pan)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_stream_pan(0x%lX, %ld)\r\n",stream,pan);
      }

   IN_AIL_NM;

   ailcall2(AIL,set_stream_pan,void,stream,pan);

   OUT_AIL_NM;

   END;
}


//############################################################################

DXDEF
S32 AILEXPORT AIL_stream_volume(HSTREAM stream)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_stream_volume(0x%lX)\r\n",stream);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,stream_volume,S32,stream);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
S32 AILEXPORT AIL_stream_pan(HSTREAM stream)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_stream_pan(0x%lX)\r\n",stream);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,stream_pan,S32,stream);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
void AILEXPORT AIL_set_stream_playback_rate(HSTREAM stream, S32 rate)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_stream_playback_rate(0x%lX, %ld)\r\n",stream,rate);
      }

   IN_AIL_NM;

   ailcall2(AIL,set_stream_playback_rate,void,stream,rate);

   OUT_AIL_NM;

   END;
}


//############################################################################

DXDEF
S32 AILEXPORT AIL_stream_playback_rate(HSTREAM stream)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_stream_playback_rate(0x%lX)\r\n",stream);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,stream_playback_rate,S32,stream);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
S32 AILEXPORT AIL_stream_loop_count(HSTREAM stream)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_stream_loop_count(0x%lX)\r\n",stream);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,stream_loop_count,S32,stream);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
void         AILEXPORT AIL_set_stream_loop_block (HSTREAM S,
                                                     S32     loop_start_offset,
                                                     S32     loop_end_offset)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_stream_loop_block(0x%lX,%ld,%ld)\r\n",S,loop_start_offset,loop_end_offset);
      }

   IN_AIL_NM;

   ailcall3(AIL,set_stream_loop_block,void,S,loop_start_offset,loop_end_offset);

   OUT_AIL_NM;

   END;
}



//############################################################################

DXDEF
void AILEXPORT AIL_set_stream_loop_count(HSTREAM stream, S32 count)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_stream_loop_count(0x%lX, %ld)\r\n",stream,count);
      }

   IN_AIL_NM;

   ailcall2(AIL,set_stream_loop_count,void,stream,count);

   OUT_AIL_NM;

   END;
}

//############################################################################

DXDEF
S32 AILEXPORT AIL_stream_status(HSTREAM stream)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_stream_status(0x%lX)\r\n",stream);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,stream_status,S32,stream);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
void AILEXPORT AIL_set_stream_position(HSTREAM stream,S32 offset)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_stream_position(0x%lX, %ld)\r\n",stream,offset);
      }

   IN_AIL;

   ailcall2(AIL,set_stream_position,void,stream,offset);

   OUT_AIL;

   END;
}


//############################################################################

DXDEF
S32 AILEXPORT AIL_stream_position(HSTREAM stream)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_stream_position(0x%lX)\r\n",stream);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,stream_position,S32,stream);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


//############################################################################

DXDEF
void AILEXPORT AIL_stream_info(HSTREAM stream, S32 FAR* datarate, S32 FAR* sndtype, S32 FAR* length, S32 FAR* memory)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_stream_info(0x%lX, 0x%lX, 0x%lX, 0x%lX, 0x%lX)\r\n",stream,datarate,sndtype,length,memory);
      }

   IN_AIL_NM;

   ailcall5(AIL,stream_info,void,stream,datarate,sndtype,length,memory);

   OUT_AIL_NM;

   END;
}


//############################################################################

DXDEF
AILSTREAMCB AILEXPORT AIL_register_stream_callback(HSTREAM stream, AILSTREAMCB callback)
{
   AILSTREAMCB result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_register_stream_callback(0x%lX, 0x%lX)\r\n",stream,callback);
      }

   IN_AIL;

   result=ailcall2(AIL,register_stream_callback,AILSTREAMCB,stream,callback);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }
   END;

   return result;
}


#if defined(IS_WINDOWS) || defined(IS_MAC)

//############################################################################

DXDEF
void AILEXPORT AIL_auto_service_stream(HSTREAM stream, S32 onoff)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_auto_service_stream(0x%lX, %ld)\r\n",stream,onoff);
      }

   IN_AIL;

   ailcall2(AIL,auto_service_stream,void,stream,onoff);

   OUT_AIL;

   END;
}


#endif

//############################################################################

DXDEF
void     AILEXPORT AIL_set_stream_user_data      (HSTREAM S, //)
                                               U32     index,
                                               S32     value)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_stream_user_data(0x%lX,%ld,%ld)\r\n",S,index,value);
      }

   IN_AIL_NM;

   ailcall3(AIL,set_stream_user_data,void,S,index,value);

   OUT_AIL_NM;

   END;
}


//############################################################################

DXDEF
S32     AILEXPORT AIL_stream_user_data          (HSTREAM S, //)
                                              U32     index)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_stream_user_data(0x%lX,%lu)\r\n",S,index);
      }

   IN_AIL_NM;

   result=ailcall2(AIL,stream_user_data,S32,S,index);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}


//############################################################################

DXDEF
void   AILEXPORT AIL_set_stream_reverb(HSTREAM S, //)
                                             F32     reverb_level,
                                             F32     reverb_reflect_time,
                                             F32     reverb_decay_time)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_stream_reverb(0x%X,%f,%f,%f)\r\n",S,reverb_level,reverb_reflect_time,reverb_decay_time);
      }

   IN_AIL;

   ailcall4(AIL,set_stream_reverb,void,S,reverb_level,reverb_reflect_time,reverb_decay_time);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void   AILEXPORT AIL_stream_reverb    (HSTREAM  S, //)
                                             F32 FAR *reverb_level,
                                             F32 FAR *reverb_reflect_time,
                                             F32 FAR *reverb_decay_time)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_stream_reverb(0x%X,%X,%X,%X)\r\n",S,reverb_level,reverb_reflect_time,reverb_decay_time);
      }

   IN_AIL;

   ailcall4(AIL,stream_reverb,void,S,reverb_level,reverb_reflect_time,reverb_decay_time);

   OUT_AIL;

   END;
}


//############################################################################

DXDEF
HDLSDEVICE AILEXPORT AIL_DLS_open(HMDIDRIVER mdi, HDIGDRIVER dig,
#if defined(IS_WINDOWS) || defined(IS_MAC)
                                  char const FAR* dls,
#else
                                  AILDOSDLS const FAR* dls,
#endif
                                  U32 flags, U32 rate, S32 bits, S32 channels)
{
   HDLSDEVICE result;

   START
      {
#if defined(IS_WINDOWS) || defined(IS_MAC)
      AIL_fprintf(AIL_debugfile,"AIL_DLS_open(0x%lX,0x%lX,%s,0x%lX,%lu,%lu,%lu)\r\n",mdi,dig,dls,flags,rate,bits,channels);
#else
      AIL_fprintf(AIL_debugfile,"AIL_DLS_open(0x%lX,0x%lX,0x%lX,0x%lX,%lu,%lu,%lu)\r\n",mdi,dig,dls,flags,rate,bits,channels);
#endif
      }

   IN_AIL;

   result=ailcall7(AIL,DLS_open,HDLSDEVICE,mdi,dig,dls,flags,rate,bits,channels);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}


//############################################################################

DXDEF
void   AILEXPORT  AIL_DLS_close(HDLSDEVICE dls, U32 flags)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_DLS_close(0x%lX,%ld)\r\n",dls,flags);
      }

   IN_AIL;

   ailcall2(AIL,DLS_close,void,dls,flags);

   OUT_AIL;

   END;
}


//############################################################################

DXDEF
HDLSFILEID AILEXPORT AIL_DLS_load_file(HDLSDEVICE dls, char const FAR* filename, U32 flags)
{
   HDLSFILEID result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_DLS_load_file(0x%lX,%s,%lu)\r\n",dls,filename,flags);
      }

   IN_AIL;

   result=ailcall3(AIL,DLS_load_file,HDLSFILEID,dls,filename,flags);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}


//############################################################################

DXDEF
HDLSFILEID AILEXPORT AIL_DLS_load_memory(HDLSDEVICE dls, void const FAR* memfile, U32 flags)
{
   HDLSFILEID result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_DLS_load_memory(0x%lX,0x%lX,%lu)\r\n",dls,memfile,flags);
      }

   IN_AIL;

   result=ailcall3(AIL,DLS_load_memory,HDLSFILEID,dls,memfile,flags);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}


//############################################################################

DXDEF
void   AILEXPORT AIL_DLS_unload(HDLSDEVICE dls, HDLSFILEID dlsid)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_DLS_unload(0x%lX,0x%lX)\r\n",dls,dlsid);
      }

   IN_AIL;

   ailcall2(AIL,DLS_unload,void,dls,dlsid);

   OUT_AIL;

   END;
}


//############################################################################

DXDEF
void   AILEXPORT AIL_DLS_compact(HDLSDEVICE dls)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_DLS_compact(0x%lX)\r\n",dls);
      }

   IN_AIL;

   ailcall1(AIL,DLS_compact,void,dls);

   OUT_AIL;

   END;
}


//############################################################################

DXDEF
void   AILEXPORT AIL_DLS_set_reverb(HDLSDEVICE dls,
                                         F32     reverb_level,
                                         F32     reverb_reflect_time,
                                         F32     reverb_decay_time)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_DLS_set_reverb(0x%lX, %f, %f, %f)\r\n",dls,reverb_level,reverb_reflect_time,reverb_decay_time);
      }

   IN_AIL;

   ailcall4(AIL,DLS_set_reverb,void,dls,reverb_level,reverb_reflect_time,reverb_decay_time);

   OUT_AIL;

   END;
}


//############################################################################

DXDEF
void   AILEXPORT AIL_DLS_get_reverb(HDLSDEVICE dls,
                                         F32 FAR*    reverb_level,
                                         F32 FAR*    reverb_reflect_time,
                                         F32 FAR*    reverb_decay_time)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_DLS_get_reverb(0x%lX, 0x%lX, 0x%lX, 0x%lX)\r\n",dls,reverb_level,reverb_reflect_time,reverb_decay_time);
      }

   IN_AIL;

   ailcall4(AIL,DLS_get_reverb,void,dls,reverb_level,reverb_reflect_time,reverb_decay_time);

   OUT_AIL;

   END;
}


//############################################################################

DXDEF
void   AILEXPORT AIL_DLS_get_info(HDLSDEVICE dls, AILDLSINFO FAR* info, S32 FAR* PercentCPU)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_DLS_get_info(0x%lX,0x%lX,0x%lX)\r\n",dls,info,PercentCPU);
      }

   IN_AIL_NM;

   ailcall3(AIL,DLS_get_info,void,dls,info,PercentCPU);

   OUT_AIL_NM;

   END;
}

//############################################################################

DXDEF
void AILEXPORT    AIL_set_sequence_ms_position  (HSEQUENCE S, //)
                                                 S32       milliseconds)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_sequence_ms_position(0x%lX,0x%lu)\r\n",S,milliseconds);
      }

   IN_AIL;

   ailcall2(AIL,set_sequence_ms_position,void,S,milliseconds);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void AILEXPORT AIL_sequence_ms_position(HSEQUENCE S, //)
                                  S32 FAR    *total_milliseconds,
                                  S32 FAR    *current_milliseconds)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_sequence_ms_position(0x%lX,0x%lX,0x%lX)\r\n",S,total_milliseconds,current_milliseconds);
      }

   IN_AIL_NM;

   ailcall3(AIL,sequence_ms_position,void,S,total_milliseconds,current_milliseconds);

   OUT_AIL_NM;

   END;
}

//############################################################################

DXDEF
void AILEXPORT AIL_sample_ms_position(HSAMPLE S, //)
                                  S32 FAR    *total_milliseconds,
                                  S32 FAR    *current_milliseconds)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_sample_ms_position(0x%lX,0x%lX,0x%lX)\r\n",S,total_milliseconds,current_milliseconds);
      }

   IN_AIL_NM;

   dsailcall3(AIL,sample_ms_position,void,S,total_milliseconds,current_milliseconds);

   OUT_AIL_NM;

   END;
}

//############################################################################

DXDEF
void AILEXPORT    AIL_set_sample_ms_position  (HSAMPLE S, //)
                                               S32       milliseconds)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_sample_ms_position(0x%lX,0x%lu)\r\n",S,milliseconds);
      }

   IN_AIL;

   dsailcall2(AIL,set_sample_ms_position,void,S,milliseconds);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void AILEXPORT AIL_stream_ms_position(HSTREAM S, //)
                                  S32 FAR    *total_milliseconds,
                                  S32 FAR    *current_milliseconds)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_stream_ms_position(0x%lX,0x%lX,0x%lX)\r\n",S,total_milliseconds,current_milliseconds);
      }

   IN_AIL_NM;

   ailcall3(AIL,stream_ms_position,void,S,total_milliseconds,current_milliseconds);

   OUT_AIL_NM;

   END;
}

//############################################################################

DXDEF
void AILEXPORT    AIL_set_stream_ms_position  (HSTREAM S, //)
                                               S32       milliseconds)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_stream_ms_position(0x%lX,0x%lu)\r\n",S,milliseconds);
      }

   IN_AIL;

   ailcall2(AIL,set_stream_ms_position,void,S,milliseconds);

   OUT_AIL;

   END;
}

//############################################################################

DXDEF
void AILEXPORT AIL_quick_set_ms_position(HAUDIO audio,S32 milliseconds)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_set_ms_position(0x%lX,0x%lu)\r\n",audio,milliseconds);
      }

   IN_AIL;

   ailcall2(AIL,quick_set_ms_position,void,audio,milliseconds);

   OUT_AIL;

   END;
}


//############################################################################

DXDEF
S32 AILEXPORT AIL_quick_ms_position(HAUDIO audio)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_ms_position(0x%lX)\r\n",audio);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,quick_ms_position,S32,audio);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
S32 AILEXPORT AIL_quick_ms_length(HAUDIO audio)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_ms_length(0x%lX)\r\n",audio);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,quick_ms_length,S32,audio);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}


//############################################################################

DXDEF
S32 AILEXPORT AIL_quick_type(HAUDIO audio)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_quick_type(0x%lX)\r\n",audio);
      }

   IN_AIL_NM;

   result=ailcall1(AIL,quick_type,S32,audio);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

DXDEF
S32  AILEXPORT      AIL_MIDI_to_XMI       (void const FAR* MIDI,
                                           U32       MIDI_size,
                                           void FAR* FAR*XMIDI,
                                           U32  FAR* XMIDI_size,
                                           S32       flags)

{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_MIDI_to_XMI(0x%lX,%lu,0x%lX,0x%lX,0x%lX)\r\n",
         MIDI, MIDI_size,XMIDI,XMIDI_size,flags);
      }

   IN_AIL_NM;

   result=AIL_API_MIDI_to_XMI(MIDI, MIDI_size,XMIDI,XMIDI_size,flags);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

  return( result);

}


DXDEF
S32 AILEXPORT AIL_compress_ADPCM(AILSOUNDINFO const FAR* info,
                              void FAR* FAR* outdata, U32 FAR* outsize)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_compress_ADPCM(0x%lX,0x%lX,0x%lX)\r\n",
         info,outdata,outsize);
      }

   IN_AIL_NM;

   result=AIL_API_compress_ADPCM(info,outdata,outsize);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

  return( result);

}

DXDEF
S32 AILEXPORT AIL_decompress_ADPCM(AILSOUNDINFO const FAR* info,
                                void FAR* FAR* outdata, U32 FAR* outsize)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_decompress_ADPCM(0x%lX,0x%lX,0x%lX)\r\n",
         info,outdata,outsize);
      }

   IN_AIL_NM;

   result=AIL_API_decompress_ADPCM(info,outdata,outsize);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

  return( result);

}


DXDEF
S32 AILEXPORT AIL_WAV_info(void const FAR* data, AILSOUNDINFO FAR* info)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_WAV_info(0x%lX,0x%lX)\r\n",
         data,info);
      }

   IN_AIL_NM;

   result=AIL_API_WAV_info(data,info);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

  return( result);

}

DXDEF
S32 AILEXPORT AIL_file_type(void const FAR* data, U32 size)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_file_type(0x%lX,%lu)\r\n",
         data,size);
      }

   IN_AIL_NM;

   result=AIL_API_file_type(data,size);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

  return( result);

}

DXDEF
S32 AILEXPORT AIL_find_DLS    (void const FAR* data, U32 size,
                               void FAR* FAR*xmi, U32 FAR* xmisize,
                               void FAR* FAR*dls, U32 FAR* dlssize)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_find_DLS_in_XMI(0x%lX,%lu,0x%lX,0x%lX,0x%lX,0x%lX)\r\n",
         data,size,xmi,xmisize,dls,dlssize);
      }

   IN_AIL_NM;

   result=AIL_API_find_DLS(data,size,xmi,xmisize,dls,dlssize);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

  return( result);

}

//############################################################################

DXDEF
S32 AILEXPORT AIL_extract_DLS      (void const FAR       *source_image, //)
                                     U32             source_size,
                                     void FAR * FAR *XMI_output_data,
                                     U32  FAR       *XMI_output_size,
                                     void FAR * FAR *DLS_output_data,
                                     U32  FAR       *DLS_output_size,
                                     AILLENGTHYCB    callback)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_DLS_extract_image(0x%lX,0x%lu,0x%lX,0x%lX,0x%lX,0x%lX,0x%lX)\r\n",
         source_image,source_size,XMI_output_data,XMI_output_size,DLS_output_data,DLS_output_size,callback);
      }

   IN_AIL_NM;

   result=AIL_API_extract_DLS(source_image,source_size,XMI_output_data,XMI_output_size,DLS_output_data,DLS_output_size,callback);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;

}

#if defined(IS_WIN32) || defined(IS_MAC)

extern HDIGDRIVER primary_digital_driver;   // from msswo.cpp

//############################################################################
DXDEF HDIGDRIVER AILEXPORT AIL_primary_digital_driver  (HDIGDRIVER new_primary)
{
   HDIGDRIVER result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_primary_digital_driver(%lX)\r\n",
         new_primary);
      }

   IN_AIL;

   result = primary_digital_driver;

   if (new_primary != NULL)
      {
      primary_digital_driver = new_primary;
      }

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF S32 AILEXPORT AIL_enumerate_filters (HPROENUM  FAR *next, //)
                                           HPROVIDER FAR *dest,
                                           C8  FAR * FAR *name)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_enumerate_filters(%lX,%lX,%lX)\r\n",
         next,dest,name);
      }

   IN_AIL_NM;

   result = AIL_API_enumerate_filters(next,dest,name);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF HDRIVERSTATE AILEXPORT AIL_open_filter        (HPROVIDER  provider, //)
                                                     HDIGDRIVER dig)
{
   FLTRESULT result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_open_filter(%lX,%lX)\r\n",
         provider,dig);
      }

   IN_AIL;

   result = AIL_API_open_filter(provider,dig);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF void  AILEXPORT    AIL_close_filter       (HDRIVERSTATE filter)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_close_filter(%lX)\r\n",
         filter);
      }

   IN_AIL;

   AIL_API_close_filter(filter);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF S32 AILEXPORT AIL_enumerate_filter_attributes(HPROVIDER                  lib,   //)
                                                    HINTENUM FAR *             next,
                                                    RIB_INTERFACE_ENTRY FAR *  dest)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_enumerate_filter_attributes(%lX,%lX,%lX)\r\n",
         lib,next,dest);
      }

   IN_AIL_NM;

   result = RIB_enumerate_interface(lib,
                                   "MSS pipeline filter",
                                    RIB_ATTRIBUTE,
                                    next,
                                    dest);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF S32 AILEXPORT AIL_enumerate_filter_sample_attributes(HPROVIDER                  lib,   //)
                                                           HINTENUM FAR *             next,
                                                           RIB_INTERFACE_ENTRY FAR *  dest)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_enumerate_filter_sample_attributes(%lX,%lX,%lX)\r\n",
         lib,next,dest);
      }

   IN_AIL_NM;

   result = RIB_enumerate_interface(lib,
                                   "Pipeline filter sample services",
                                    RIB_ATTRIBUTE,
                                    next,
                                    dest);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF void AILEXPORT AIL_filter_attribute(HPROVIDER   lib,  //)
                                          C8 const FAR *    name,
                                          void FAR *  val)
{
   U32 token;
   PROVIDER_QUERY_ATTRIBUTE query_attribute = NULL;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_filter_attribute(%lX,%s,%lX)\r\n",
         lib,name,val);
      }

   IN_AIL_NM;

   *(S32 FAR *) val = -1;

   RIB_request_interface_entry(lib,
                               "MSS pipeline filter",
                               RIB_FUNCTION,
                               "PROVIDER_query_attribute",
                   (U32 FAR *) &query_attribute);

   if (RIB_request_interface_entry(lib,
                                  "MSS pipeline filter",
                                   RIB_ATTRIBUTE,
                                   name,
                                  &token) == RIB_NOERR)
      {
      *(U32 FAR *) val = query_attribute(token);
      }

   OUT_AIL_NM;

   END;
}

//############################################################################
DXDEF void AILEXPORT AIL_filter_sample_attribute(HSAMPLE     samp, //)
                                                 C8 const FAR *    name,
                                                 void FAR *  val)
{
   U32 token;
   HPROVIDER lib;
   FLTSMP_SAMPLE_ATTRIBUTE sample_attribute = NULL;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_filter_sample_attribute(%lX,%s,%lX)\r\n",
         samp,name,val);
      }

   IN_AIL_NM;

   *(S32 FAR *) val = -1;

   if (samp != NULL)
      {
      lib = samp->pipeline[DP_FILTER].provider;

      RIB_request_interface_entry(lib,
                                 "Pipeline filter sample services",
                                  RIB_FUNCTION,
                                 "FLTSMP_sample_attribute",
                     (U32 FAR *) &sample_attribute);

      if (sample_attribute != NULL)
         {
         if (RIB_request_interface_entry(lib,
                                        "Pipeline filter sample services",
                                         RIB_ATTRIBUTE,
                                         name,
                                        &token) == RIB_NOERR)
            {
            *(U32 FAR *) val = sample_attribute(samp->pipeline[DP_FILTER].TYPE.FLT.sample_state,
                                                token);
            }
         }
      }

   OUT_AIL_NM;

   END;
}

//############################################################################
DXDEF void AILEXPORT AIL_filter_stream_attribute(HSTREAM     strm, //)
                                                 C8 const FAR *    name,
                                                 void FAR *  val)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_filter_stream_attribute(%lX,%s,%lX)\r\n",
         strm,name,val);
      }

   IN_AIL_NM;

   AIL_filter_sample_attribute( strm->samp, name, val);

   OUT_AIL_NM;

   END;
}

//############################################################################
DXDEF void AILEXPORT AIL_filter_DLS_attribute   (HDLSDEVICE     dls, //)
                                                 C8 const FAR *    name,
                                                 void FAR *  val)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_filter_DLS_attribute(%lX,%s,%lX)\r\n",
         dls,name,val);
      }

   IN_AIL_NM;

   AIL_filter_sample_attribute( dls->sample, name, val);

   OUT_AIL_NM;

   END;
}

//############################################################################
DXDEF void AILEXPORT AIL_set_filter_preference(HPROVIDER   lib, //)
                                               C8 const FAR *    name,
                                               void const FAR *  val)
{
   U32 token;
   FLT_SET_PROVIDER_PREFERENCE set_preference = NULL;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_filter_preference(%lX,%s,%lX)\r\n",
         lib,name,val);
      }

   IN_AIL;

   RIB_request_interface_entry(lib,
                              "MSS pipeline filter",
                               RIB_FUNCTION,
                              "FLT_set_provider_preference",
                  (U32 FAR *) &set_preference);

   if (set_preference != NULL)
      {
      if (RIB_request_interface_entry(lib,
                                     "MSS pipeline filter",
                                      RIB_PREFERENCE,
                                      name,
                                     &token) == RIB_NOERR)
         {
         set_preference(token,
                        val);
         }
      }

   OUT_AIL;

   END;
}

//############################################################################
DXDEF void AILEXPORT AIL_set_filter_sample_preference(HSAMPLE   samp, //)
                                                      C8 const FAR *    name,
                                                      void const FAR *  val)
{
   U32       token;
   HPROVIDER lib;
   FLTSMP_SET_SAMPLE_PREFERENCE set_preference = NULL;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_filter_sample_preference(%lX,%s,%lX)\r\n",
         samp,name,val);
      }

   IN_AIL;

   if (samp != NULL)
      {
      lib = samp->pipeline[DP_FILTER].provider;

      RIB_request_interface_entry(lib,
                                 "Pipeline filter sample services",
                                  RIB_FUNCTION,
                                 "FLTSMP_set_sample_preference",
                     (U32 FAR *) &set_preference);

      if (set_preference != NULL)
         {
         if (RIB_request_interface_entry(lib,
                                        "Pipeline filter sample services",
                                         RIB_PREFERENCE,
                                         name,
                                        &token) == RIB_NOERR)
            {
            set_preference(samp->pipeline[DP_FILTER].TYPE.FLT.sample_state,
                           token,
                           val);
            }
         }
      }

   OUT_AIL;

   END;
}

//############################################################################
DXDEF void AILEXPORT AIL_set_filter_stream_preference(HSTREAM     strm, //)
                                                 C8 const FAR *    name,
                                                 void const FAR *  val)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_filter_stream_preference(%lX,%s,%lX)\r\n",
         strm,name,val);
      }

   IN_AIL;

   AIL_set_filter_sample_preference( strm->samp, name, val);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF void AILEXPORT AIL_set_filter_DLS_preference(HDLSDEVICE     dls, //)
                                                 C8 const FAR *    name,
                                                 void const FAR *  val)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_filter_DLS_preference(%lX,%s,%lX)\r\n",
         dls,name,val);
      }

   IN_AIL;

   AIL_set_filter_sample_preference( dls->sample, name, val);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF S32 AILEXPORT AIL_enumerate_3D_providers (HPROENUM  FAR *next, //)
                                                HPROVIDER FAR *dest,
                                                C8  FAR * FAR *name)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_enumerate_3D_providers(%lX,%lX,%lX)\r\n",
         next,dest,name);
      }

   IN_AIL_NM;

   result = AIL_API_enumerate_3D_providers(next,dest,name);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF M3DRESULT AILEXPORT AIL_open_3D_provider        (HPROVIDER provider)
{
   M3DRESULT result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_open_3D_provider(%lX)\r\n",
         provider);
      }

   IN_AIL;

   result = AIL_API_open_3D_provider(provider);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF void  AILEXPORT    AIL_close_3D_provider       (HPROVIDER    lib)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_close_3D_provider(%lX)\r\n",
         lib);
      }

   IN_AIL;

   AIL_API_close_3D_provider(lib);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF S32 AILEXPORT AIL_enumerate_3D_provider_attributes(HPROVIDER                  lib,   //)
                                                         HINTENUM FAR *             next,
                                                         RIB_INTERFACE_ENTRY FAR *  dest)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_enumerate_3D_provider_attributes(%lX,%lX,%lX)\r\n",
         lib,next,dest);
      }

   IN_AIL_NM;

   result = RIB_enumerate_interface(lib,
                                   "MSS 3D audio services",
                                    RIB_ATTRIBUTE,
                                    next,
                                    dest);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF S32 AILEXPORT AIL_enumerate_3D_sample_attributes(HPROVIDER                  lib,   //)
                                                       HINTENUM FAR *             next,
                                                       RIB_INTERFACE_ENTRY FAR *  dest)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_enumerate_3D_sample_attributes(%lX,%lX,%lX)\r\n",
         lib,next,dest);
      }

   IN_AIL_NM;

   result = RIB_enumerate_interface(lib,
                                   "MSS 3D sample services",
                                    RIB_ATTRIBUTE,
                                    next,
                                    dest);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF void AILEXPORT AIL_3D_provider_attribute(HPROVIDER   lib,  //)
                                               C8 const FAR *    name,
                                               void FAR *  val)
{
   U32 token;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_provider_attribute(%lX,%s,%lX)\r\n",
         lib,name,val);
      }

   IN_AIL_NM;

   if (val)
     {

     *(S32 FAR *) val = -1;

     if (RIB_request_interface_entry(lib,
                                    "MSS 3D audio services",
                                     RIB_ATTRIBUTE,
                                     name,
                                    &token) == RIB_NOERR)
        {
        PROVIDER_QUERY_ATTRIBUTE getattr;

        if (RIB_request_interface_entry(lib,
                                       "MSS 3D audio services",
                                        RIB_FUNCTION,
                                        "PROVIDER_query_attribute",
                                        (U32 FAR*)&getattr) == RIB_NOERR)
           {

           *(U32 FAR *) val =getattr(token);

           }

        }
     }

   OUT_AIL_NM;

   END;
}

//############################################################################
DXDEF void AILEXPORT AIL_3D_sample_attribute(H3DSAMPLE   samp, //)
                                             C8 const FAR *    name,
                                             void FAR *  val)
{
   U32 token;
   HPROVIDER lib;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_sample_attribute(%lX,%s,%lX)\r\n",
         samp,name,val);
      }

   IN_AIL_NM;

   *(S32 FAR *) val = -1;

   lib = ((struct H3D FAR *) samp)->owner;

   if (RIB_request_interface_entry(lib,
                                  "MSS 3D sample services",
                                   RIB_ATTRIBUTE,
                                   name,
                                  &token) == RIB_NOERR)
      {
      *(U32 FAR *) val = ((M3DPROVIDER FAR *)
         RIB_provider_system_data(lib, 0))->sample_query_attribute(((struct H3D FAR *) samp)->actual,token);
      }

   OUT_AIL_NM;

   END;
}

//############################################################################
DXDEF void AILEXPORT AIL_set_3D_provider_preference(HPROVIDER   lib, //)
                                                    C8 const FAR *    name,
                                                    void const FAR *  val)
{
   U32 token;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_provider_preference(%lX,%s,%lX)\r\n",
         lib,name,val);
      }

   IN_AIL;

   if (RIB_request_interface_entry(lib,
                                  "MSS 3D audio services",
                                   RIB_PREFERENCE,
                                   name,
                                  &token) == RIB_NOERR)
      {
      M3D_SET_PROVIDER_PREFERENCE setpref;

      if (RIB_request_interface_entry(lib,
                                     "MSS 3D audio services",
                                      RIB_FUNCTION,
                                      "M3D_set_provider_preference",
                                      (U32 FAR*)&setpref) == RIB_NOERR)
         {

           setpref(token,val);

         }

      }

   OUT_AIL;

   END;
}

//############################################################################
DXDEF void AILEXPORT AIL_set_3D_sample_preference(H3DSAMPLE   samp, //)
                                                  C8 const FAR *    name,
                                                  void const FAR *  val)
{
   U32       token;
   HPROVIDER lib;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_provider_preference(%lX,%s,%lX)\r\n",
         samp,name,val);
      }

   IN_AIL;

   lib = ((struct H3D FAR *) samp)->owner;

   if (RIB_request_interface_entry(lib,
                                  "MSS 3D sample services",
                                   RIB_PREFERENCE,
                                   name,
                                  &token) == RIB_NOERR)
      {
      ((M3DPROVIDER FAR *) RIB_provider_system_data(lib, 0))->
         set_sample_preference(((struct H3D FAR *) samp)->actual,token, val);
      }

   OUT_AIL;

   END;
}

//############################################################################
DXDEF H3DSAMPLE AILEXPORT AIL_allocate_3D_sample_handle (HPROVIDER    lib)
{
   H3DSAMPLE result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_allocate_3D_sample_handle(%lX)\r\n",
         lib);
      }

   IN_AIL;

   result = AIL_API_allocate_3D_sample_handle(lib);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}


//############################################################################
DXDEF void AILEXPORT     AIL_release_3D_sample_handle  (H3DSAMPLE samp)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_release_3D_sample_handle(%lX)\r\n",
          samp);
      }

   IN_AIL;

   AIL_API_release_3D_sample_handle(samp);

   OUT_AIL;

   END;
}


//############################################################################
DXDEF void AILEXPORT     AIL_start_3D_sample         (H3DSAMPLE samp)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_start_3D_sample(%lX)\r\n",
          samp);
      }

   IN_AIL;

   AIL_API_start_3D_sample(samp);

   OUT_AIL;

   END;
}


//############################################################################
DXDEF void AILEXPORT     AIL_stop_3D_sample          (H3DSAMPLE samp)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_stop_3D_sample(%lX)\r\n",
          samp);
      }

   IN_AIL;

   AIL_API_stop_3D_sample(samp);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF void  AILEXPORT    AIL_resume_3D_sample        (H3DSAMPLE samp)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_resume_3D_sample(%lX)\r\n",
          samp);
      }

   IN_AIL;

   AIL_API_resume_3D_sample(samp);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF void  AILEXPORT    AIL_end_3D_sample        (H3DSAMPLE samp)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_end_3D_sample(%lX)\r\n",
          samp);
      }

   IN_AIL;

   AIL_API_end_3D_sample(samp);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF S32  AILEXPORT     AIL_set_3D_sample_info      (H3DSAMPLE samp, //)
                                                      AILSOUNDINFO const FAR *info)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_sample_info(%lX,%lX)\r\n",
          samp,info);
      }

   IN_AIL;

   result = AIL_API_set_3D_sample_info(samp, info);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF S32  AILEXPORT     AIL_set_3D_sample_file      (H3DSAMPLE samp, //)
                                                      void const FAR *file_image)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_sample_file(%lX,%lX)\r\n",
          samp,file_image);
      }

   IN_AIL;

   result = AIL_API_set_3D_sample_file(samp, file_image);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF void AILEXPORT     AIL_set_3D_sample_volume    (H3DSAMPLE samp, //)
                                                      S32       volume)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_sample_volume(%lX,%ld)\r\n",
          samp,volume);
      }

   IN_AIL_NM;

   AIL_API_set_3D_sample_volume(samp, volume);

   OUT_AIL_NM;

   END;
}

//############################################################################
DXDEF void AILEXPORT     AIL_set_3D_sample_playback_rate    (H3DSAMPLE samp, //)
                                                             S32       playback_rate)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_sample_playback_rate(%lX,%ld)\r\n",
          samp,playback_rate);
      }

   IN_AIL_NM;

   AIL_API_set_3D_sample_playback_rate(samp, playback_rate);

   OUT_AIL_NM;

   END;
}

//############################################################################
DXDEF void AILEXPORT     AIL_set_3D_sample_offset    (H3DSAMPLE samp, //)
                                                      U32       offset)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_sample_offset(%lX,%lu)\r\n",
          samp,offset);
      }

   IN_AIL;

   AIL_API_set_3D_sample_offset(samp, offset);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF void AILEXPORT     AIL_set_3D_sample_loop_count(H3DSAMPLE samp, //)
                                                      U32       loops)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_sample_loop_count(%lX,%lu)\r\n",
          samp,loops);
      }

   IN_AIL_NM;

   AIL_API_set_3D_sample_loop_count(samp, loops);

   OUT_AIL_NM;

   END;
}

//############################################################################
DXDEF void AILEXPORT     AIL_set_3D_sample_loop_block(H3DSAMPLE samp, //)
                                                      S32       loop_start_offset,
                                                      S32       loop_end_offset)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_sample_loop_block(%lX,%ld,%ld)\r\n",
          samp,loop_start_offset,loop_end_offset);
      }

   IN_AIL_NM;

   AIL_API_set_3D_sample_loop_block(samp, loop_start_offset, loop_end_offset);

   OUT_AIL_NM;

   END;
}

//############################################################################
DXDEF
void       AILEXPORT AIL_set_3D_sample_obstruction (H3DSAMPLE S, //)
                                                    F32       obstruction)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_sample_obstruction(%lX,%f)\r\n",
          S, obstruction);
      }

   IN_AIL;

   AIL_API_set_3D_sample_obstruction(S, obstruction);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF
void       AILEXPORT AIL_set_3D_sample_occlusion   (H3DSAMPLE S, //)
                                                    F32       occlusion)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_sample_occlusion(%lX,%f)\r\n",
          S, occlusion);
      }

   IN_AIL;

   AIL_API_set_3D_sample_occlusion(S, occlusion);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF
void       AILEXPORT AIL_set_3D_sample_cone        (H3DSAMPLE S, //)
                                                    F32       inner_angle,
                                                    F32       outer_angle,
                                                    S32       outer_volume)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_sample_cone(%lX,%f,%s,%u)\r\n",
          S, inner_angle,outer_angle,outer_volume);
      }

   IN_AIL;

   AIL_API_set_3D_sample_cone(S, inner_angle,outer_angle,outer_volume);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF
void       AILEXPORT AIL_set_3D_sample_effects_level   (H3DSAMPLE S, //)
                                                        F32       effects_level)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_sample_effects_level(%lX,%f)\r\n",
          S, effects_level);
      }

   IN_AIL;

   AIL_API_set_3D_sample_effects_level(S, effects_level);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF
AIL3DSAMPLECB AILEXPORT AIL_register_3D_EOS_callback   (H3DSAMPLE S, //)
                                                        AIL3DSAMPLECB cb)
{
   AIL3DSAMPLECB result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_register_3D_EOS_callback(%lX,%lX)\r\n",
          S, cb);
      }

   IN_AIL;

   result = AIL_API_register_3D_EOS_callback(S, cb);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF
F32        AILEXPORT AIL_3D_sample_obstruction (H3DSAMPLE S)
{
   F32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_sample_obstruction(%lX)\r\n",
          S);
      }

   IN_AIL_NM;

   result = AIL_API_3D_sample_obstruction(S);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF
F32        AILEXPORT AIL_3D_sample_occlusion   (H3DSAMPLE S)
{
   F32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_sample_occlusion(%lX)\r\n",
          S);
      }

   IN_AIL_NM;

   result = AIL_API_3D_sample_occlusion(S);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF
void       AILEXPORT AIL_3D_sample_cone        (H3DSAMPLE S,
                                                F32 FAR* inner_angle,
                                                F32 FAR* outer_angle,
                                                S32 FAR* outer_volume)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_sample_cone(%lX,%lX,%lX,%lX)\r\n",
          S,inner_angle,outer_angle,outer_volume);
      }

   IN_AIL_NM;

   AIL_API_3D_sample_cone(S,inner_angle,outer_angle,outer_volume);

   OUT_AIL_NM;

   END;
}

//############################################################################
DXDEF
F32        AILEXPORT AIL_3D_sample_effects_level   (H3DSAMPLE S)
{
   F32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_sample_effects_level(%lX)\r\n",
          S);
      }

   IN_AIL_NM;

   result = AIL_API_3D_sample_effects_level(S);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################

DXDEF
U32        AILEXPORT AIL_3D_sample_status         (H3DSAMPLE samp)
{
   U32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_sample_status(0x%lX)\r\n",samp);
      }

   IN_AIL_NM;

   result = AIL_API_3D_sample_status(samp);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF S32 AILEXPORT      AIL_3D_sample_volume        (H3DSAMPLE samp)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_sample_volume(%lX)\r\n",
          samp);
      }

   IN_AIL_NM;

   result = AIL_API_3D_sample_volume(samp);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF S32 AILEXPORT      AIL_3D_sample_playback_rate        (H3DSAMPLE samp)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_sample_playback_rate(%lX)\r\n",
          samp);
      }

   IN_AIL_NM;

   result = AIL_API_3D_sample_playback_rate(samp);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF U32 AILEXPORT      AIL_3D_sample_offset        (H3DSAMPLE     samp)
{
   U32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_sample_offset(%lX)\r\n",
          samp);
      }

   IN_AIL_NM;

   result = AIL_API_3D_sample_offset(samp);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF U32 AILEXPORT      AIL_3D_sample_length        (H3DSAMPLE     samp)
{
   U32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_sample_length(%lX)\r\n",
          samp);
      }

   IN_AIL_NM;

   result = AIL_API_3D_sample_length(samp);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF U32 AILEXPORT      AIL_3D_sample_loop_count    (H3DSAMPLE samp)
{
   U32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_sample_loop_count(%lX)\r\n",
          samp);
      }

   IN_AIL_NM;

   result = AIL_API_3D_sample_loop_count(samp);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF void AILEXPORT     AIL_set_3D_sample_distances (H3DSAMPLE samp, //)
                                                      F32       max_dist,
                                                      F32       min_dist)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_sample_distances(%lX,%f,%f)\r\n",
          samp,max_dist,min_dist);
      }

   IN_AIL;

   AIL_API_set_3D_sample_distances(samp,max_dist,min_dist);

   OUT_AIL;

   END;
}


//############################################################################
DXDEF void AILEXPORT     AIL_3D_sample_distances     (H3DSAMPLE samp, //)
                                                      F32 FAR * max_dist,
                                                      F32 FAR * min_dist)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_sample_distances(%lX,%lX,%lX)\r\n",
          samp,max_dist,min_dist);
      }

   IN_AIL_NM;

   AIL_API_3D_sample_distances(samp,max_dist,min_dist);

   OUT_AIL_NM;

   END;
}

//############################################################################
DXDEF  void AILEXPORT   AIL_set_3D_user_data         (H3DPOBJECT obj, //)
                                                      U32        index,
                                                      S32        value)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_user_data(%lX,%ld,%lX)\r\n",
          obj,index,value);
      }

   IN_AIL_NM;

   AIL_API_set_3D_user_data(obj,index,value);

   OUT_AIL_NM;

   END;
}

//############################################################################
DXDEF  S32 AILEXPORT    AIL_3D_user_data             (H3DPOBJECT obj, //)
                                                      U32        index)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_user_data(%lX,%ld)\r\n",
          obj,index);
      }

   IN_AIL_NM;

   result = AIL_API_3D_user_data(obj,index);

   OUT_AIL_NM;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF  S32 AILEXPORT    AIL_active_3D_sample_count   (HPROVIDER lib)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_active_3D_sample_count(%lX)\r\n",lib);
      }

   IN_AIL;

   result = AIL_API_active_3D_sample_count(lib);

   OUT_AIL;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF  S32 AILEXPORT    AIL_3D_room_type   (HPROVIDER lib)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_room_type(%lX)\r\n",lib);
      }

   IN_AIL_NM;

   result = AIL_API_3D_room_type(lib);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF  void AILEXPORT   AIL_set_3D_room_type   (HPROVIDER lib, //)
                                                S32       EAX_room_type)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_room_type(%lX,%d)\r\n",lib,EAX_room_type);
      }

   IN_AIL;

   AIL_API_set_3D_room_type(lib, EAX_room_type);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF  S32 AILEXPORT    AIL_3D_speaker_type   (HPROVIDER lib)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_speaker_type(%lX)\r\n",lib);
      }

   IN_AIL_NM;

   result = AIL_API_3D_speaker_type(lib);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF  void AILEXPORT   AIL_set_3D_speaker_type   (HPROVIDER lib, //)
                                                   S32       speaker_type)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_speaker_type(%lX,%d)\r\n",lib,speaker_type);
      }

   IN_AIL;

   AIL_API_set_3D_speaker_type(lib, speaker_type);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF  F32 AILEXPORT    AIL_3D_rolloff_factor  (HPROVIDER lib)
{
   F32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_rolloff_factor(%lX)\r\n",lib);
      }

   IN_AIL_NM;

   result = AIL_API_3D_rolloff_factor(lib);

   OUT_AIL_NM;

   RESULT
      {
      outresfloat((F32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF  void AILEXPORT   AIL_set_3D_rolloff_factor (HPROVIDER lib, //)
                                                   F32       factor )
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_rolloff_factor(%lX,%f)\r\n",lib,factor);
      }

   IN_AIL;

   AIL_API_set_3D_rolloff_factor(lib, factor);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF  F32 AILEXPORT    AIL_3D_doppler_factor  (HPROVIDER lib)
{
   F32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_doppler_factor(%lX)\r\n",lib);
      }

   IN_AIL_NM;

   result = AIL_API_3D_doppler_factor(lib);

   OUT_AIL_NM;

   RESULT
      {
      outresfloat((F32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF  void AILEXPORT   AIL_set_3D_doppler_factor (HPROVIDER lib, //)
                                                   F32       factor )
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_doppler_factor(%lX,%f)\r\n",lib,factor);
      }

   IN_AIL;

   AIL_API_set_3D_doppler_factor(lib, factor);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF  F32 AILEXPORT    AIL_3D_distance_factor  (HPROVIDER lib)
{
   F32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_distance_factor(%lX)\r\n",lib);
      }

   IN_AIL_NM;

   result = AIL_API_3D_distance_factor(lib);

   OUT_AIL_NM;

   RESULT
      {
      outresfloat((F32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF  void AILEXPORT   AIL_set_3D_distance_factor (HPROVIDER lib, //)
                                                    F32       factor )
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_distance_factor(%lX,%f)\r\n",lib,factor);
      }

   IN_AIL;

   AIL_API_set_3D_distance_factor(lib, factor);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF H3DPOBJECT AILEXPORT AIL_open_3D_listener        (HPROVIDER lib)
{
   H3DPOBJECT result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_open_3D_listener(%lX)\r\n",lib);
      }

   IN_AIL;

   result = AIL_API_open_3D_listener(lib);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF void AILEXPORT     AIL_close_3D_listener       (H3DPOBJECT listener)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_close_3D_listener(%lX)\r\n",
          listener);
      }

   IN_AIL;

   AIL_API_close_3D_listener(listener);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF H3DPOBJECT AILEXPORT AIL_open_3D_object          (HPROVIDER lib)
{
   H3DPOBJECT result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_open_3D_object(%lX)\r\n",lib);
      }

   IN_AIL;

   result = AIL_API_open_3D_object(lib);

   OUT_AIL;

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//############################################################################
DXDEF void  AILEXPORT AIL_close_3D_object         (H3DPOBJECT object)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_close_3D_object(%lX)\r\n",
         object);
      }

   IN_AIL;

   AIL_API_close_3D_object(object);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF void  AILEXPORT    AIL_set_3D_position         (H3DPOBJECT obj, //)
                                                      F32     X,
                                                      F32     Y,
                                                      F32     Z)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_position(%lX,%f,%f,%f)\r\n",
          obj,X,Y,Z);
      }

   IN_AIL;

   AIL_API_set_3D_position(obj,X,Y,Z);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF void AILEXPORT     AIL_set_3D_velocity         (H3DPOBJECT obj, //)
                                                      F32     dX_per_ms,
                                                      F32     dY_per_ms,
                                                      F32     dZ_per_ms,
                                                      F32     magnitude)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_velocity(%lX,%f,%f,%f,%f)\r\n",
          obj,dX_per_ms,dY_per_ms,dZ_per_ms,magnitude);
      }

   IN_AIL;

   AIL_API_set_3D_velocity(obj,dX_per_ms,dY_per_ms,dZ_per_ms,magnitude);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF void AILEXPORT     AIL_set_3D_velocity_vector  (H3DPOBJECT obj, //)
                                                      F32     dX_per_ms,
                                                      F32     dY_per_ms,
                                                      F32     dZ_per_ms)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_velocity_vector(%lX,%f,%f,%f)\r\n",
          obj,dX_per_ms,dY_per_ms,dZ_per_ms);
      }

   IN_AIL;

   AIL_API_set_3D_velocity_vector(obj,dX_per_ms,dY_per_ms,dZ_per_ms);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF void AILEXPORT     AIL_set_3D_orientation      (H3DPOBJECT obj, //)
                                                      F32     X_face,
                                                      F32     Y_face,
                                                      F32     Z_face,
                                                      F32     X_up,
                                                      F32     Y_up,
                                                      F32     Z_up)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_3D_orientation(%lX,%f,%f,%f,%f,%f,%f)\r\n",
          obj,X_face,Y_face,Z_face,X_up,Y_up,Z_up);
      }

   IN_AIL;

   AIL_API_set_3D_orientation(obj,X_face,Y_face,Z_face,X_up,Y_up,Z_up);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF void AILEXPORT     AIL_3D_position             (H3DPOBJECT  obj, //)
                                                      F32 FAR *X,
                                                      F32 FAR *Y,
                                                      F32 FAR *Z)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_position(%lX,%lX,%lX,%lX)\r\n",
          obj,X,Y,Z);
      }

   IN_AIL_NM;

   AIL_API_3D_position(obj,X,Y,Z);

   OUT_AIL_NM;

   END;
}

//############################################################################
DXDEF void AILEXPORT     AIL_3D_velocity             (H3DPOBJECT  obj, //)
                                                      F32 FAR *dX_per_ms,
                                                      F32 FAR *dY_per_ms,
                                                      F32 FAR *dZ_per_ms)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_velocity(%lX,%lX,%lX,%lX)\r\n",
          obj,dX_per_ms,dY_per_ms,dZ_per_ms);
      }

   IN_AIL_NM;

   AIL_API_3D_velocity(obj,dX_per_ms,dY_per_ms,dZ_per_ms);

   OUT_AIL_NM;

   END;
}

//############################################################################
DXDEF void AILEXPORT     AIL_3D_orientation          (H3DPOBJECT  obj, //)
                                                      F32 FAR *X_face,
                                                      F32 FAR *Y_face,
                                                      F32 FAR *Z_face,
                                                      F32 FAR *X_up,
                                                      F32 FAR *Y_up,
                                                      F32 FAR *Z_up)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_3D_orientation(%lX,%lX,%lX,%lX,%lX,%lX,%lX)\r\n",
          obj,X_face,Y_face,Z_face,X_up,Y_up,Z_up);
      }

   IN_AIL_NM;

   AIL_API_3D_orientation(obj,X_face,Y_face,Z_face,X_up,Y_up,Z_up);

   OUT_AIL_NM;

   END;
}

//############################################################################
DXDEF void AILEXPORT     AIL_update_3D_position      (H3DPOBJECT obj, //)
                                                      F32     dt_milliseconds)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_update_3D_position(%lX,%f)\r\n",
          obj,dt_milliseconds);
      }

   IN_AIL;

   AIL_API_update_3D_position(obj, dt_milliseconds);

   OUT_AIL;

   END;
}

//############################################################################
DXDEF void AILEXPORT     AIL_auto_update_3D_position (H3DPOBJECT obj, //)
                                                      S32        enable)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_auto_update_3D_position(%lX,%ld)\r\n",
          obj,enable);
      }

   IN_AIL;

   AIL_API_auto_update_3D_position(obj,enable);

   OUT_AIL;

   END;
}

DXDEF
S32 AILEXPORT AIL_size_processed_digital_audio(
                                 U32             dest_rate,
                                 U32             dest_format,
                                 S32             num_srcs,
                                 AILMIXINFO const FAR* src)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_size_processed_digital_audio(%li,%li,%li,%lX)\r\n",
          dest_rate,dest_format,num_srcs,src);
      }

   IN_AIL_NM;

   result = AIL_API_size_processed_digital_audio(dest_rate,dest_format,num_srcs,src);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

DXDEF
S32 AILEXPORT AIL_process_digital_audio(
                                 void FAR       *dest_buffer,
                                 S32             dest_buffer_size,
                                 U32             dest_rate,
                                 U32             dest_format,
                                 S32             num_srcs,
                                 AILMIXINFO FAR* src)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_process_digital_audio(%lX,%li,%li,%li,%li,%lX)\r\n",
          dest_buffer,dest_buffer_size,dest_rate,dest_format,num_srcs,src);
      }

   IN_AIL_NM;

   result = AIL_API_process_digital_audio(dest_buffer,dest_buffer_size,dest_rate,dest_format,num_srcs,src);

   OUT_AIL_NM;

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}


#endif

#ifdef IS_DOS

//#############################################################################
REALFAR AILEXPORT  AIL_get_real_vect             (U32       vectnum)
{
   REALFAR result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_get_real_vect(0x%X)\r\n",vectnum);
      }

   result = AIL_API_get_real_vect(vectnum);

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}
//#############################################################################
void    AILEXPORT  AIL_set_real_vect             (U32       vectnum,
                                              REALFAR     real_ptr)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_real_vect(0x%X,0x%X)\r\n",vectnum,real_ptr);
      }

   AIL_API_set_real_vect(vectnum,real_ptr);

   END;
}

//#############################################################################
void    AILEXPORT  AIL_set_USE16_ISR             (S32        IRQ,
                                              REALFAR     real_base,
                                              U32       ISR_offset)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_USE16_ISR(%d,0x%X,%u)\r\n",IRQ,real_base,ISR_offset);
      }

   AIL_API_set_USE16_ISR(IRQ,real_base,ISR_offset);

   END;
}

//#############################################################################
void    AILEXPORT  AIL_restore_USE16_ISR         (S32        IRQ)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_restore_USE16_ISR(%d)\r\n",IRQ);
      }

   AIL_API_restore_USE16_ISR(IRQ);

   END;
}

//#############################################################################
U32   AILEXPORT  AIL_disable_interrupts        (void)
{
   U32 result;

   result = AIL_API_disable_interrupts();

   return result;
}

//#############################################################################
void    AILEXPORT  AIL_restore_interrupts        (U32       FD_register)
{
   AIL_API_restore_interrupts(FD_register);
}

//#############################################################################
S32    AILEXPORT  AIL_call_driver               (AIL_DRIVER *drvr,
                                              S32        fn,
                                              VDI_CALL   *in,
                                              VDI_CALL   *out)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_call_driver(0x%X,0x%X,0x%X,0x%X)\r\n",
         drvr,fn,in,out);
      }

   result = AIL_API_call_driver(drvr,fn,in,out);

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//#############################################################################
S32    AILEXPORT  AIL_read_INI                  (AIL_INI    *INI,
                                              char      *filename)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_API_read_INI(0x%X,%s)\r\n",INI,filename);
      }

   result = AIL_API_read_INI(INI,filename);

   if (result)
      {
      RESULT
         {
         INDENT;
         AIL_fprintf(AIL_debugfile,"Driver = %s\r\n",INI->driver_name);
         INDENT;
         AIL_fprintf(AIL_debugfile,"Device = %s\r\n",INI->device_name);
         INDENT;
         AIL_fprintf(AIL_debugfile,"IO     = %X\r\n",INI->IO.IO);
         INDENT;
         AIL_fprintf(AIL_debugfile,"IRQ    = %d\r\n",INI->IO.IRQ);
         INDENT;
         AIL_fprintf(AIL_debugfile,"DMA_8  = %d\r\n",INI->IO.DMA_8_bit);
         INDENT;
         AIL_fprintf(AIL_debugfile,"DMA_16 = %d\r\n",INI->IO.DMA_16_bit);
         }
      }
   else
      {
      RESULT
         {
         outresint((U32)result);
         }
      }

   END;

   return result;
}

//#############################################################################
IO_PARMS *   AILEXPORT AIL_get_IO_environment    (AIL_DRIVER *drvr)
{
   IO_PARMS *result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_get_IO_environment(0x%X)\r\n",drvr);
      }

   result = AIL_API_get_IO_environment(drvr);

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//#############################################################################
AIL_DRIVER * AILEXPORT AIL_install_driver        (U8 const     *driver_image,
                                              U32       n_bytes)
{
   AIL_DRIVER *result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_install_driver(0x%X,%u)\r\n",driver_image,n_bytes);
      }

   result = AIL_API_install_driver(driver_image,n_bytes);

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//#############################################################################
void         AILEXPORT AIL_uninstall_driver      (AIL_DRIVER *drvr)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_uninstall_driver(0x%X)\r\n",drvr);
      }

   AIL_API_uninstall_driver(drvr);

   END;
}

//#############################################################################
S32 AILEXPORT AIL_install_DIG_INI(HDIGDRIVER *dig)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_install_DIG_INI(0x%X)\r\n",dig);
      }

   result = AIL_API_install_DIG_INI(dig);

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//#############################################################################
HDIGDRIVER  AILEXPORT AIL_install_DIG_driver_file
                                             (char const *filename,
                                              IO_PARMS *IO)
{
   HDIGDRIVER result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_install_DIG_driver_file(%s,0x%X)\r\n",filename,IO);
      }

   result = AIL_API_install_DIG_driver_file(filename,IO);

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//#############################################################################
void         AILEXPORT AIL_uninstall_DIG_driver  (HDIGDRIVER dig)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_uninstall_DIG_driver(0x%X)\r\n",dig);
      }

   AIL_API_uninstall_DIG_driver(dig);

   END;
}

//#############################################################################
HDIGDRIVER  AILEXPORT AIL_install_DIG_driver_image
                                             (void const *driver_image,
                                              U32     size,
                                              IO_PARMS *IO)
{
   HDIGDRIVER result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_install_DIG_driver_image(0x%X,%u,0x%X)\r\n",
         driver_image,size,IO);
      }

   result = AIL_API_install_DIG_driver_image(driver_image,size,IO);

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//#############################################################################
S32 AILEXPORT AIL_install_MDI_INI(HMDIDRIVER *mdi)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_install_MDI_INI(0x%X)\r\n",mdi);
      }

   result = AIL_API_install_MDI_INI(mdi);

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//#############################################################################
HMDIDRIVER  AILEXPORT AIL_install_MDI_driver_file
                                             (char const *filename,
                                              IO_PARMS   *IO)
{
   HMDIDRIVER result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_install_MDI_driver_file(%s,0x%X)\r\n",filename,IO);
      }

   result = AIL_API_install_MDI_driver_file(filename,IO);

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//#############################################################################
void         AILEXPORT AIL_uninstall_MDI_driver  (HMDIDRIVER  mdi)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_uninstall_MDI_driver(0x%X)\r\n",mdi);
      }

   AIL_API_uninstall_MDI_driver(mdi);

   END;
}

//#############################################################################
HMDIDRIVER  AILEXPORT AIL_install_MDI_driver_image
                                             (void const *driver_image,
                                              U32       size,
                                              IO_PARMS   *IO)
{
   HMDIDRIVER result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_install_MDI_driver_image(0x%X,%u,0x%X)\r\n",
         driver_image,size,IO);
      }

   result = AIL_API_install_MDI_driver_image(driver_image,size,IO);

   RESULT
      {
      outreshex((U32)result);
      }

   END;

   return result;
}

//#############################################################################
S32     AILEXPORT AIL_MDI_driver_type       (HMDIDRIVER mdi)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_MDI_driver_type(0x%X)\r\n",mdi);
      }

   result = AIL_API_MDI_driver_type(mdi);

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//#############################################################################
void     AILEXPORT AIL_set_GTL_filename_prefix   (char const       *prefix)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_set_GTL_filename_prefix(%s)\r\n",prefix);
      }

   AIL_API_set_GTL_filename_prefix(prefix);

   END;
}

//#############################################################################
S32     AILEXPORT AIL_timbre_status             (HMDIDRIVER  mdi,
                                              S32        bank,
                                              S32        patch)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_timbre_status(0x%X,%d,%d)\r\n",mdi,bank,patch);
      }

   result = AIL_API_timbre_status(mdi,bank,patch);

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//#############################################################################
S32     AILEXPORT AIL_install_timbre            (HMDIDRIVER  mdi,
                                              S32        bank,
                                              S32        patch)
{
   S32 result;

   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_install_timbre(0x%X,%d,%d)\r\n",mdi,bank,patch);
      }

   result = AIL_API_install_timbre(mdi,bank,patch);

   RESULT
      {
      outresint((U32)result);
      }

   END;

   return result;
}

//#############################################################################
void     AILEXPORT AIL_protect_timbre            (HMDIDRIVER  mdi,
                                              S32        bank,
                                              S32        patch)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_protect_timbre(0x%X,%d,%d)\r\n",mdi,bank,patch);
      }

   AIL_API_protect_timbre(mdi,bank,patch);

   END;
}

//#############################################################################
void  AILEXPORT AIL_unprotect_timbre          (HMDIDRIVER  mdi,
                                              S32        bank,
                                              S32        patch)
{
   START
      {
      AIL_fprintf(AIL_debugfile,"AIL_unprotect_timbre(0x%X,%d,%d)\r\n",mdi,bank,patch);
      }

   AIL_API_unprotect_timbre(mdi,bank,patch);

   END;
}


//############################################################################
//##                                                                        ##
//## End of locked code                                                     ##
//##                                                                        ##
//############################################################################

void __pascal AILDEBUG_end(void)
{
   if (locked)
      {
      AIL_vmm_unlock_range(AILDEBUG_start, AILDEBUG_end);

      locked = 0;

      }
}

#endif
