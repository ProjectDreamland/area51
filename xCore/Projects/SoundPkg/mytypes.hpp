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
typedef unsigned short      u16;

    ////////////////////////////////////////////////////////////////////////////

typedef u8      byte;
typedef s32     xbool;

#endif // __IOPTYPES_H
