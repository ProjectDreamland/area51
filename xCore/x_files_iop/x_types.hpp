//==============================================================================
//  
//  x_types.hpp
//  
//==============================================================================

#ifndef X_TYPES_HPP
#define X_TYPES_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TARGET_HPP
#include "x_target.hpp"
#endif

//
// For each platform, certain include files are essentially ubiquitous.  
// Include them here.
//

#ifdef TARGET_PS2
#endif

#ifdef TARGET_GCN
#endif

#ifdef TARGET_XBOX
#endif

//==============================================================================
//  DEFINES
//==============================================================================

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#define FALSE       (0)
#define TRUE        (1)

#ifndef NULL
  #ifdef __cplusplus
    #define NULL        0
  #else
    #define NULL        ((void*)0)
  #endif
#endif

//
// Simple macros provided by the x_files free of charge.
//

#ifdef ABS
#undef ABS
#endif

#ifdef MIN
#undef MIN
#endif

#ifdef MAX
#undef MAX
#endif

#ifdef IN_RANGE
#undef IN_RANGE
#endif

#define ABS(a)              ( (a) < (0) ? -(a) : (a) )
#define MIN(a,b)            ( (a) < (b) ?  (a) : (b) )
#define MAX(a,b)            ( (a) > (b) ?  (a) : (b) )
#define MINMAX(a,v,b)       ( MAX( (a), MIN((v),(b)) ) )

#define IN_RANGE(a,v,b)     ( ((a) <= (v)) && ((v) <= (b)) )

//==============================================================================
//  TYPES
//==============================================================================
//  
//  The x_files provide the following standard types:
//  
//      u8      s8      Unsigned/signed  8 bit integer.
//      u16     s16     Unsigned/signed 16 bit integer.
//      u32     s32     Unsigned/signed 32 bit integer.
//      u64     s64     Unsigned/signed 64 bit integer.
//      f32             32 bit floating point value.
//      f64             64 bit floating point value.
//      byte            Single byte.  Use "byte*" for "pointer to raw memory".
//      xbool           Boolean value.  Use with TRUE and FALSE.
//      xwchar          Wide character (16 bit).
//      xcolor          Standard color form.  Defined in x_color.hpp.
//  
//  Various math types are defined in x_math.hpp.
//  
//==============================================================================

#define U8_MAX                       255
#define U8_MIN                         0
                                
#define U16_MAX                    65535
#define U16_MIN                        0
                               
#define U32_MAX               4294967295
#define U32_MIN                        0

#define U64_MAX     18446744073709551615            
#define U64_MIN                        0

#define S8_MAX                       127
#define S8_MIN                      -128
                            
#define S16_MAX                    32767
#define S16_MIN                   -32768
                            
#define S32_MAX               2147483647
#define S32_MIN              -2147483648

#define S64_MAX      9223372036854775807
#define S64_MIN     -9223372036854775808

#define F32_MIN         1.175494351e-38f    // Smallest POSITIVE f32 value
#define F32_MAX         3.402823466e+38f

#define F64_MIN  2.2250738585072014e-308    // Smallest POSITIVE f64 value
#define F64_MAX  1.7976931348623158e+308

//==============================================================================

#ifdef TARGET_PC

  // Types for PC targets under Microsoft's Visual Studio.
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
    typedef u8                  byte;
    typedef s32                 xbool;
    typedef u16                 xwchar;
  #endif
  
#endif

//------------------------------------------------------------------------------

#ifdef TARGET_PS2

  // Types for PS2 targets under SN Systems' compiler.
  #ifdef __GNUC__
    typedef unsigned char       u8;
    typedef unsigned short      u16;
    typedef unsigned int        u32;
    typedef unsigned long long  u64;
    typedef   signed char       s8;
    typedef   signed short      s16;
    typedef   signed int        s32;
    typedef   signed long long  s64;
    typedef u8                  byte;
    typedef s32                 xbool;
    typedef u16                 xwchar;
//#if defined(TARGET_PS2_IOP)
#if 1
	#include "implementation/x_math_soft_float.hpp"
#else
    typedef          float      f32;
    typedef          double     f64;
#endif
  #endif

#endif      

//------------------------------------------------------------------------------

#ifdef TARGET_GCN
  #ifdef __SN__               // for SN
    typedef   signed char       s8;
    typedef   signed short      s16;
    typedef   signed long       s32;
    typedef   signed long long  s64;
    typedef unsigned char       u8;
    typedef unsigned short      u16;
    typedef unsigned long       u32;
    typedef unsigned long long  u64;
    typedef          float      f32;
    typedef          double     f64;
    typedef u8                  byte;
    typedef s32                 xbool;
    typedef u16                 xwchar;
  #endif
#endif

//------------------------------------------------------------------------------

#ifdef TARGET_XBOX
#error Types not created for X-Box.
#endif

//==============================================================================
//  
//  Global redefinitions of the C++ heap management functions.  Specifically:
//      operator new
//      operator new []
//      operator delete
//      operator delete []
//  
//  Since these operators are part of the language, they have the broadest 
//  possible scope.  This file will generally be included by every file of every
//  project which utilizes the x_files.  For this reason, this is a good place
//  for these declarations.  These functions are defined in x_memory.hpp.
//  
//  Some compilers require the definition of versions of operator delete which
//  accept the same expanded parameters as the redefined operator new.  Hence
//  the presence of the extra versions of operator delete.
//  
//  Targets which use MFC do not use the x_files versions of operators new and 
//  delete.
//  
//  Declarations for functions x_malloc and x_free are in x_memory.hpp.
//  
//==============================================================================

#ifdef X_DEBUG
#define X_MEM_DEBUG
#endif

//---------------------------
#ifndef USE_SYSTEM_NEW_DELETE
//---------------------------
#ifdef TARGET_GCN
#define xalloctype unsigned int
#else
#define xalloctype u32
#endif

void* operator new       ( xalloctype Size );
void* operator new    [] ( xalloctype Size );
void* operator new       ( xalloctype Size, char* pFileName, s32 LineNumber );
void* operator new    [] ( xalloctype Size, char* pFileName, s32 LineNumber );

void  operator delete    ( void* pMemory );
void  operator delete [] ( void* pMemory );
void  operator delete    ( void* pMemory, char*, s32 );
void  operator delete [] ( void* pMemory, char*, s32 );


#ifdef X_MEM_DEBUG
    #define new new( __FILE__, __LINE__ )
#endif              

//-----------------------------
#endif // USE_SYSTEM_NEW_DELETE
//-----------------------------

//==============================================================================
#endif // X_TYPES_HPP
//==============================================================================
