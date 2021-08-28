////////////////////////////////////////////////////////////////////////////
//  DEFINES
////////////////////////////////////////////////////////////////////////////
#ifndef __IOPTYPES_H
#define __IOPTYPES_H

#ifdef TRUE
#undef TRUE
#undef FALSE
#endif

#define FALSE           (0)
#define TRUE            (1)

#ifndef NULL
  #ifdef __cplusplus
    #define NULL            0
  #else
    #define NULL            ((void*)0)
  #endif
#endif

//
// Simple macros that the x_files provide free of charge.
//

#ifndef ABS
    #define ABS(a)          ( (a) < 0 ? -(a) : (a) )
#endif

#ifndef MAX
    #define MAX(a,b)        ( (a)>(b) ? (a) : (b) )
#endif

#ifndef MIN
    #define MIN(a,b)        ( (a)<(b) ? (a) : (b) )
#endif

#ifndef IN_RANGE
    #define IN_RANGE(a,v,b)     ( ((a) <= (v)) && ((v) <= (b)) )
#endif

////////////////////////////////////////////////////////////////////////////
//  TYPES
////////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------

#ifdef TARGET_PS2_IOP

#include <kernel.h>
  // Types for PS2 Targets under SN compiler.
  #ifdef __GNUC__
    typedef unsigned char       u8;
    typedef unsigned short      u16;
    typedef unsigned int        u32;
    typedef unsigned long long  u64;
    typedef   signed char       s8;
    typedef   signed short      s16;
    typedef   signed int        s32;
    typedef   signed long long  s64;
    typedef          float      f32;
    typedef          double     f64;
  #endif

  // Types for PS2 Targets under Developer Studio.  (iCL needs this.)
  #ifdef _MSC_VER
    typedef unsigned char       u8;
    typedef unsigned short      u16;
    typedef unsigned int        u32;
    typedef unsigned __int64    u64;
    typedef   signed char       s8;
    typedef   signed short      s16;
    typedef   signed int        s32;
    typedef   signed __int64    s64;
    typedef          float      f32;
    typedef          double     f64;
  #endif

  // Types for PS2 Targets under Metrowerks.  (iCL needs this.)
  #ifdef __MWERKS__
    typedef unsigned char       u8;
    typedef signed char         s8;
    typedef unsigned short      u16;
    typedef signed short        s16;
    typedef unsigned int        u32;
    typedef signed int          s32;
    typedef unsigned long long  u64;
    typedef signed long long    s64;
    typedef float               f32;
    typedef double              f64;
  #endif

#ifndef ASSERT
    #ifdef X_DEBUG
        #define BREAK asm("break 0x0001")
#define ASSERT(x) if (!(x)) { iop_DebugMsg("\nIOP-Assert fail at line %d of %s\n",__LINE__,__FILE__); BREAK;}
        #define ASSERTS(x,s) if (!(x)) { iop_DebugMsg("\nIOP-Assert:%s at line %d of %s\n",s,__LINE__,__FILE__); BREAK;}
    #else
        #define BREAK
        #define ASSERT(x)
        #define ASSERTS(x,s)
    #endif
#endif


#define malloc #error Malloc used
#define free    #error Free Used
////////////////////////////////////////////////////////////////////////////

typedef u8      byte;
typedef s32     xbool;

#else
    #include "x_files.hpp"
#endif      


#endif // __IOPTYPES_H