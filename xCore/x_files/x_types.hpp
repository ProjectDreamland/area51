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
#define BIT(x)              ( 1<<(x) )

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
                               
#define U32_MAX               4294967295U
#define U32_MIN                        0

#ifdef TARGET_PS2
#define U64_MAX     18446744073709551615LL
#else
#define U64_MAX     18446744073709551615
#endif
#define U64_MIN                        0

#define S8_MAX                       127
#define S8_MIN                      -128
                            
#define S16_MAX                    32767
#define S16_MIN                   -32768
                            
#define S32_MAX         ((s32)2147483647)
#define S32_MIN        -((s32)2147483647)

#define S64_MAX      9223372036854775807
#define S64_MIN     -9223372036854775808

#define F32_MIN         1.175494351e-38f    // Smallest POSITIVE f32 value.
#define F32_MAX         3.402823466e+38f

#define F64_MIN  2.2250738585072014e-308    // Smallest POSITIVE f64 value.
#define F64_MAX  1.7976931348623158e+308

#define X_KILOBYTE(x) ((x) *    1024)
#define X_MEGABYTE(x) ((x) * 1048576)

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

#define X_SECTION(x)
  
#endif

//------------------------------------------------------------------------------

#ifdef TARGET_PS2

  // Types for PS2 targets under SN Systems' compiler.
  #ifdef __GNUC__
    typedef unsigned char       u8;
    typedef unsigned short      u16;
    typedef unsigned int        u32;
    typedef unsigned long       u64;
    typedef unsigned int		u128 __attribute__ (( mode(TI) )) __attribute__ ((aligned(16)));
    typedef   signed char       s8;
    typedef   signed short      s16;
    typedef   signed int        s32;
    typedef   signed long       s64;
    typedef          float      f32;
    typedef          double     f64;
    typedef u8                  byte;
    typedef s32                 xbool;
    typedef u16                 xwchar;
  #endif

#if defined(__GNUC__)
#define X_SECTION(x) __attribute__((section("."#x)))
#else
#define X_SECTION(x)
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
  #else
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

#if defined(__GNUC__)
#define X_SECTION(x) __attribute__((section("."#x)))
#else
#define X_SECTION(x)
#endif

#endif

//------------------------------------------------------------------------------

#ifdef TARGET_XBOX

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

    union __declspec(intrin_type)__declspec( align( 16 )) f128
    {
        struct
        {
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };
    };

#define X_SECTION(x)
#endif 

//------------------------------------------------------------------------------
// For C++ Only
//------------------------------------------------------------------------------
#ifdef __cplusplus

//------------------------------------------------------------------------------

#define NULL_GUID ((guid)0)
struct guid
{
    u64 Guid;
    guid(void)          { Guid = 0;         }
    guid(u64 NewGuid)   { Guid = NewGuid;   }
    inline operator const u64       ( void ) const { return Guid; }
    inline u32 GetLow ( void ) { return (u32)((Guid>> 0)&0xFFFFFFFF); }
    inline u32 GetHigh( void ) { return (u32)((Guid>>32)&0xFFFFFFFF); }
};

//==============================================================================
//==============================================================================
// inline function versions

inline s32 iMax( s32 a, s32 b )       { return ( (a) > (b) ?  (a) : (b) ); }
inline f32 fMax( f32 a, f32 b )       { return ( (a) > (b) ?  (a) : (b) ); }
inline s32 iMin( s32 a, s32 b )       { return ( (a) < (b) ?  (a) : (b) ); }
inline f32 fMin( f32 a, f32 b )       { return ( (a) < (b) ?  (a) : (b) ); }

template< class T > inline T x_abs ( T a )              { return ( (a) < (0) ? -(a) : (a) ); } 
template< class T > inline T x_min ( T a, T b )         { return ( (a) < (b) ?  (a) : (b) ); } 
template< class T > inline T x_min ( T a, T b, T c )    { return x_min( x_min( a, b ), c );  }
template< class T > inline T x_max ( T a, T b )         { return ( (a) > (b) ?  (a) : (b) ); } 
template< class T > inline T x_max ( T a, T b, T c )    { return x_max( x_max( a, b ), c );  }
template< class T > inline T x_sign( T a )              { if (a < 0) return -1 ; else if (a > 0) return 1 ; else return 0 ; }

//==============================================================================
// XHANDLE
//==============================================================================

#define HNULL -1

struct xhandle
{
    s32 Handle;

    inline xhandle                  ( void  )      {}
    inline xhandle                  ( s32 I )      { Handle = I;             }
    inline operator const s32       ( void ) const { return Handle;          }
    inline xbool          IsNonNull ( void ) const { return Handle != HNULL; }
    inline xbool          IsNull    ( void ) const { return Handle == HNULL; }    
};

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

#ifdef TARGET_GCN
#define xalloctype unsigned int
#else
#define xalloctype u32
#endif

//------------------------------------------------------------------------------

inline void* operator new       ( xalloctype Size, void* pData ) { (void)Size; return pData; }
inline void  operator delete    ( void* pMemory,   void* pData ) { (void)pMemory; (void)pData; }

template< class T >          inline void xConstruct( T* Ptr )         { (void) new(Ptr) T; }
template< class T, class S > inline T*   xConstruct( T* Ptr, S& Ref ) { return new(Ptr) T(Ref); }

//---------------------------
#ifndef USE_SYSTEM_NEW_DELETE
//---------------------------

void* operator new       ( xalloctype Size );
void* operator new    [] ( xalloctype Size );
void* operator new       ( xalloctype Size, const char* pFileName, s32 LineNumber, const char* pFunction );
void* operator new    [] ( xalloctype Size, const char* pFileName, s32 LineNumber, const char* pFunction );

void  operator delete    ( void* pMemory );
void  operator delete [] ( void* pMemory );
void  operator delete    ( void* pMemory, const char*, s32, const char* );
void  operator delete [] ( void* pMemory, const char*, s32, const char* );

#ifdef X_MEM_DEBUG
#if defined( __GNUC__ )
    #define new new( __FILE__, __LINE__, __PRETTY_FUNCTION__ )
//  #define delete delete( __FILE__, __LINE__, __PRETTY_FUNCTION__ )
#else
    #define new new( __FILE__, __LINE__, NULL )
//  #define delete delete( __FILE__, __LINE__, NULL )
#endif
#endif

//-----------------------------
#endif // USE_SYSTEM_NEW_DELETE
//-----------------------------

//------------------------------------------------------------------------------
// END of C++ Only
//------------------------------------------------------------------------------
#endif

//==============================================================================
#endif // X_TYPES_HPP
//==============================================================================
