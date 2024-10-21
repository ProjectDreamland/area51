//==============================================================================
//
//  x_plus.hpp
//
//==============================================================================

#ifndef X_PLUS_HPP
#define X_PLUS_HPP

//==============================================================================
//  
//  There are a few categories of functionality that aren't covered in the other
//  files and which are not large enough to justify a new file.  So, all those
//  bits have been gathered into this file.  And thus, x_plus was created.
//
//  Categories of functionality:
//                                 
//    - Variable length argument list support.
//    - Integral value alignment.
//    - Endian swapping for 16 and 32 bit values.
//    - Mutex class for single entry critical sections.
//    - Quick sort and binary search.
//    - Pseudo random number generation.
//    - NULL terminated string functions, such as x_strcpy.
//    - Memory block functions, such as x_memcpy.
//    - Standard ASCII conversions, such as x_tolower and x_atof.
//    - Standard character classifiers, such as x_isspace.
//    - Path manipulation functions, such as x_splitpath.
//  
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

#ifndef X_MEMORY_HPP
#include "x_memory.hpp"
#endif

#if( defined TARGET_XBOX )||( defined TARGET_PC )
#include <string.h>
#endif

// For variable length argument list support.
#include <stdarg.h>

//==============================================================================
//  MACROS
//==============================================================================

//==============================================================================
//
//  XBIN  - Is use to conver from a constant binary number to a regular
//          decinal number. Example: XBIN( 111 ) == 7. XBIN can do a a max of 
//          16bits worth of binary number ( 1111111111111111 ). So it does the  
//          first 16bits of a dword.
// 
//  XBINH - Works just as XBIN but it does the upper 16bits of a dword.
//
//  _XBIN - If this looks strange it is because is a private macro. 
//          So it is not advisable to use it.
//
//==============================================================================

#define _XBIN( A, L ) ( (u32)(((((u64)0##A)>>(3*L)) & 1)<<L) )
#define XBIN( N ) ((u32)( _XBIN( N,  0 ) | _XBIN( N,  1 ) | _XBIN( N,  2 ) | _XBIN( N,  3 ) | \
                          _XBIN( N,  4 ) | _XBIN( N,  5 ) | _XBIN( N,  6 ) | _XBIN( N,  7 ) | \
                          _XBIN( N,  8 ) | _XBIN( N,  9 ) | _XBIN( N, 10 ) | _XBIN( N, 11 ) | \
                          _XBIN( N, 12 ) | _XBIN( N, 13 ) | _XBIN( N, 14 ) | _XBIN( N, 15 ) ))
#define XBINH( N ) ( XBIN( N ) << 16 )


//==============================================================================
//  
//  The ALIGN_xx(n) macros result in the next number greater than or equal to n 
//  which is a multiple of xx.  For example, ALIGN_16(57) is 64.
//  
//  The ENDIAN_SWAP_xx macros toggle the Endian of a 16 or 32 bit value.
//  
//  The LITTLE_ENDIAN_xx and BIG_ENDIAN_xx macros convert the Endian of 16 or 32
//  bit values between the system "native" Endian to the specified Endian.  Note
//  that these macros work for both reading and writing.  Or, said another way, 
//  these macros toggle when the system native Endian does NOT match the macro 
//  name, and do nothing when the system native does match the macro name.
//  
//==============================================================================

#define ALIGN_256(n)    ( (((s32)(n)) + 255) & (-256) )
#define ALIGN_128(n)    ( (((s32)(n)) + 127) & (-128) )
#define ALIGN_64(n)     ( (((s32)(n)) +  63) & ( -64) )
#define ALIGN_32(n)     ( (((s32)(n)) +  31) & ( -32) )
#define ALIGN_16(n)     ( (((s32)(n)) +  15) & ( -16) )
#define ALIGN_8(n)      ( (((s32)(n)) +   7) & (  -8) )
#define ALIGN_4(n)      ( (((s32)(n)) +   3) & (  -4) )
#define ALIGN_2(n)      ( (((s32)(n)) +   1) & (  -2) )

#define ENDIAN_SWAP_16(A)           ( (((u16)(A)) >> 8) |   \
                                      (((u16)(A)) << 8) )    
                                    
#define ENDIAN_SWAP_32(A)           ( (((u32)(A)) >> 24) |  \
                                      (((u32)(A)) << 24) |  \
                                      ((((u32)(A)) & 0x00FF0000) >> 8) | \
                                      ((((u32)(A)) & 0x0000FF00) << 8) ) 

#ifdef TARGET_GCN
#define ENDIAN_SWAP_64(A)           ( (((u64)(A)) >> 56) |  \
                                      (((u64)(A)) << 56) |  \
                                      ((((u64)(A)) & 0x00FF000000000000ULL) >> 40)   | \
                                      ((((u64)(A)) & 0x000000000000FF00ULL) << 40)   | \
                                      ((((u64)(A)) & 0x0000FF0000000000ULL) >> 24)   | \
                                      ((((u64)(A)) & 0x0000000000FF0000ULL) << 24)   | \
                                      ((((u64)(A)) & 0x000000FF00000000ULL) >> 8)    | \
                                      ((((u64)(A)) & 0x00000000FF000000ULL) << 8) ) 
#else
#define ENDIAN_SWAP_64(A)           ( (((u64)(A)) >> 56) |  \
                                      (((u64)(A)) << 56) |  \
                                      ((((u64)(A)) & 0x00FF000000000000) >> 40)   | \
                                      ((((u64)(A)) & 0x000000000000FF00) << 40)   | \
                                      ((((u64)(A)) & 0x0000FF0000000000) >> 24)   | \
                                      ((((u64)(A)) & 0x0000000000FF0000) << 24)   | \
                                      ((((u64)(A)) & 0x000000FF00000000) >> 8)    | \
                                      ((((u64)(A)) & 0x00000000FF000000) << 8) ) 
#endif

#ifdef LITTLE_ENDIAN
    #define LITTLE_ENDIAN_16(A)     A 
    #define LITTLE_ENDIAN_32(A)     A
    #define BIG_ENDIAN_16(A)        ENDIAN_SWAP_16(A)
    #define BIG_ENDIAN_32(A)        ENDIAN_SWAP_32(A)
#endif

#ifdef BIG_ENDIAN
    #define LITTLE_ENDIAN_16(A)     ENDIAN_SWAP_16(A) 
    #define LITTLE_ENDIAN_32(A)     ENDIAN_SWAP_32(A)
    #define BIG_ENDIAN_16(A)        A
    #define BIG_ENDIAN_32(A)        A
#endif   
    
//==============================================================================
//  Variable length argument list support.
//==============================================================================

#define x_va_start(list,prev)   va_start(list,prev)
#define x_va_end(list)          va_end(list)
#define x_va_arg(list,mode)     va_arg(list,mode)

typedef va_list x_va_list;

//==============================================================================
//  TYPES
//==============================================================================

//==============================================================================
//  
//  Mutex class.
//
//==============================================================================
/*
class mutex
{
public:
            mutex   ( void );
           ~mutex   ( void );
    void    Enter   ( void );
    void    Exit    ( void );
};
*/
//==============================================================================
//
//  Define a "standard compare function" type.  Functions which match this 
//  signature take the addresses of two items (provided as void pointers) and 
//  return a value indicating the relationship between the two items.  The 
//  return values are interpretted as follows:
//  
//      < 0   indicates   Item1 < Item2
//      = 0   indicates   Item1 = Item2
//      > 0   indicates   Item1 > Item2
//  
//  Functions x_strcmp and x_stricmp fit the compare function pattern and can be
//  used as compare functions.
//
//  This type definition is used by the quick sort and binary search functions. 
//
//==============================================================================

typedef s32 compare_fn( const void* pItem1, const void* pItem2 );

//==============================================================================
//  FUNCTIONS
//==============================================================================

//==============================================================================
//  
//  x_qsort     - Optimized quick sort function.
//
//  x_bsearch   - Binary search function.  Return value is address of desired
//                item in the list, or NULL if item was not found.
//  
//  x_bsearch   - Has an extra parameter.  Like x_bsearch(), but, if not found, 
//                uses the extra parameter to report where the item would go if 
//                it were added to the list.
//  
//  The quick sort function is recursive.  It is guaranteed that it will not 
//  recurse more than "log base 2 of NItems".  Use of x_qsort in a critical 
//  program (such as a game) should be tested under the most extreme potential 
//  conditions to prevent stack overflows.
//  
//  The input to the binary search functions must be a list sorted in ascending 
//  order.  The standand ANSI bsearch function does not allow for duplicates in 
//  the list.  The x_files versions handle duplicates.  The first item which 
//  matches will be returned.
//
//==============================================================================
                                  
void x_qsort    ( const void*     pBase,          // Address of first item in array.
                  s32             NItems,         // Number of items in array.
                  s32             ItemSize,       // Size of one item.
                  compare_fn*     pCompare );     // Compare function.

void* x_bsearch ( const void*     pKey,           // Reference item to search for.
                  const void*     pBase,          // First item in the array.
                  s32             NItems,         // Number of items in array.
                  s32             ItemSize,       // Size of one item.
                  compare_fn*     pCompare );     // Compare function.
                  
void* x_bsearch ( const void*     pKey,           // Reference item to search for.
                  const void*     pBase,          // First item in the array.
                  s32             NItems,         // Number of items in array.
                  s32             ItemSize,       // Size of one item.
                  compare_fn*     pCompare,       // Compare function.
                  void*&          pLocation );    // Where item should be in list.
                  
//==============================================================================
//  Pseudo random number generation functions.
//==============================================================================
//                  
//  x_srand     - Set seed for the random number generator.
//  x_rand      - Random integer in [   0, X_RAND_MAX ].
//  x_irand     - Random integer in [ Min, Max        ].
//  x_frand     - Random float   in [ Min, Max        ].
//
//  X_RAND_MAX is the largest value which can be returned by x_rand().
//  
//  Note that all threads use the above four functions.  You can not rely on
//  these function for "reproducible" random numbers since other threads may
//  interfere.
//
//  If you need reproducible random numbers, then use an instance of class
//  random.  Each instance of random is independent from all others.  The 
//  interface for class random is the same as the four global functions (minus 
//  the "x_" prefix).  
//  
//  random::srand   - Same as x_srand.
//  random::rand    - Same as x_rand.
//  random::irand   - Same as x_irand.
//  random::frand   - Same as x_frand.
//  
//==============================================================================

//==============================================================================
//  Need some announcements before we can proceed.
//==============================================================================

struct vector2;
#ifdef TARGET_PS2
struct vector3;
#else
union vector3;
#endif
struct xcolor;

//==============================================================================

#define X_RAND_MAX   0x7FFF 

//==============================================================================

void    x_srand     ( s32 Seed );
s32     x_rand      ( void );               // Result in [   0, X_RAND_MAX ]
s32     x_irand     ( s32 Min, s32 Max );   // Result in [ Min, Max        ]
f32     x_frand     ( f32 Min, f32 Max );   // Result in [ Min, Max        ]

//==============================================================================

class random
{
public:
            random  ( void );
            random  ( s32 Seed );

    void    srand   ( s32 Seed );
    s32     rand    ( void );               // Result in [   0, X_RAND_MAX ]
    s32     irand   ( s32 Min, s32 Max );   // Result in [ Min, Max        ]
    f32     frand   ( f32 Min, f32 Max );   // Result in [ Min, Max        ]

    vector2 v2      ( f32 MinX, f32 MaxX, f32 MinY, f32 MaxY );
    vector3 v3      ( f32 MinX, f32 MaxX, f32 MinY, f32 MaxY, f32 MinZ, f32 MaxZ );
    xcolor  color   ( u8 A = 255 );

private:
    s32     m_Seed;
};

//==============================================================================
//  Standard NULL terminated string manipulation functions.
//==============================================================================
//  
//  x_strlen        - Return length of string.
//  x_strcpy        - Copy string.
//  x_strdup        - Duplicate string into new memory.
//  x_strcat        - Concatenate one string onto end of another.
//  x_strncat       - Concatenate, but with character limit.
//  x_strcmp        - String comparison.
//  x_stricmp       - String comparison, but ignore upper/lower case.
//  x_strncmp       - String comparison, but with character limit.
//  x_strstr        - Find substring within string.
//  x_stristr       - Find substring within string case insensitive.
//  x_strchr        - Find first occurrence of given character in string.
//  x_strrchr       - Find last  occurrence of given character in string.
//  x_strncpy       - Copies string up to a certain count.
//                    If too many characters, a null character is NOT appended.
//  x_strsavecpy    - Copies string up to a certain count.
//                    If too many characters, a null character is appended.
//  
//  Additional functions:
//  
//  x_strtoupper    - Upper case entire string.
//  x_strtolower    - Lower case entire string.
//  
//  Wide string functions:
//  
//  x_wstrlen       - Return length of string.
//  x_wstrcpy       - Copy string.
//  x_wstrdup       - Duplicate string into new memory.
//  x_wstrcat       - Concatenate one string onto end of another.
//  x_wstrncat      - Concatenate, but with character limit.
//  x_wstrcmp       - String comparison.
//  x_wstricmp      - String comparison, but ignore upper/lower case.
//  x_wstrncmp      - String comparison, but with character limit.
//  x_wstrstr       - Find substring within string.
//  x_wstrchr       - Find first occurrence of given wide character in string.
//  x_wstrrchr      - Find last  occurrence of given wide character in string.
//  
//  Mixed width string function:
//  
//  x_mstrcpy       - Copy wide   string into narrow buffer.
//                  - Copy narrow string into wide   buffer.
//  x_mstrncpy      - Copy wide   to narrow, but with character limit.
//                  - Copy narrow to wide  , but with character limit.
//  x_mstrcat       - Concatenate wide   string onto narrow buffer.
//                  - Concatenate narrow string onto wide   buffer.
//  
//==============================================================================

s32     x_strlen    ( const char* pStr );
char*   x_strcpy    (       char* pDest,    const char* pSrc );
char*	x_strdup	( const char* pStr );
char*   x_strcat    (       char* pFront,   const char* pBack );
s32     x_strcmp    ( const char* pStr1,    const char* pStr2 );
char*   x_strncpy   (       char* pDest,    const char* pSrc,   s32 Count );
char*   x_strsavecpy(       char* pDest,    const char* pSrc,   s32 Count );
char*   x_strncat   (       char* pFront,   const char* pBack,  s32 Count );
s32     x_stricmp   ( const char* pStr1,    const char* pStr2 );
s32     x_strncmp   ( const char* pStr1,    const char* pStr2,  s32 Count );
char*   x_strstr    ( const char* pMainStr, const char* pSubStr );
char*   x_stristr   ( const char* pMainStr, const char* pSubStr );
char*   x_strchr    ( const char* pStr, s32 C );
char*   x_strrchr   ( const char* pStr, s32 C );

char*   x_strtoupper( char* pStr );
char*   x_strtolower( char* pStr );

s32     x_wstrlen   ( const xwchar* pWideStr );
xwchar* x_wstrcpy   (       xwchar* pWideDest,    const xwchar* pWideSrc );
xwchar*	x_strdup	( const xwchar* pWideStr );
xwchar* x_wstrcat   (       xwchar* pWideFront,   const xwchar* pWideBack );
s32     x_wstrcmp   ( const xwchar* pWideStr1,    const xwchar* pWideStr2 );
s32     x_wstricmp  ( const xwchar* pWideStr1,    const xwchar* pWideStr2 );
xwchar* x_wstrncpy  (       xwchar* pWideDest,    const xwchar* pWideSrc,   s32 Count );
xwchar* x_wstrncat  (       xwchar* pWideFront,   const xwchar* pWideBack,  s32 Count );
s32     x_wstrncmp  ( const xwchar* pWideStr1,    const xwchar* pWideStr2,  s32 Count );
xwchar* x_wstrstr   ( const xwchar* pWideMainStr, const xwchar* pWideSubStr );
xwchar* x_wstrchr   ( const xwchar* pWideStr, s32 WideChar );
xwchar* x_wstrrchr  ( const xwchar* pWideStr, s32 WideChar );

char*   x_mstrcpy   (   char* pDest, const xwchar* pSrc );
xwchar* x_mstrcpy   ( xwchar* pDest, const   char* pSrc );
char*   x_mstrncpy  (   char* pDest, const xwchar* pSrc, s32 Count );
xwchar* x_mstrncpy  ( xwchar* pDest, const   char* pSrc, s32 Count );
char*   x_mstrcat   (   char* pDest, const xwchar* pSrc );
xwchar* x_mstrcat   ( xwchar* pDest, const   char* pSrc );
xwchar* x_mstristr  ( const xwchar* pMainStr, const   char* pSubStr );
char*   x_mstristr  ( const   char* pMainStr, const xwchar* pSubStr );

//------------------------------------------------------------------------------
//  Xbox intrinsic versions.
//------------------------------------------------------------------------------
#if defined(TARGET_XBOX) && !defined(X_DEBUG)
    #define x_strlen strlen
    #define x_strcpy strcpy
    #define x_strcat strcat
    #define x_strcmp strcmp
#endif

//==============================================================================
//  Standard block memory functions.
//==============================================================================
//  
//  x_memcpy    - Copy memory block.  Blocks must not overlap.
//  x_memmove   - Copy memory block.  Blocks may overlap.
//  x_memset    - Fill memory block with given byte (or character) value.
//  x_memchr    - Find first occurrence of character within memory block.
//  x_memcmp    - Compare memory blocks.
//  
//  Additional functions:
//  
//  x_chksum    - Create a check sum value from a memory block.
//  
//==============================================================================

void*   x_memcpy    ( void* pDest, const void* pSrc, s32 Count );
void*   x_memset    ( void* pBuf, s32 C, s32 Count );
s32     x_memcmp    ( const void* pBuf1, const void* pBuf2, s32 Count );
void*   x_memmove   ( void* pDest, const void* pSrc, s32 Count );
void*   x_memchr    ( void* pBuf, s32 C, s32 Count );
u32     x_chksum    ( const void* pBuf, s32 Count );

//------------------------------------------------------------------------------
//  Xbox intrinsic versions.
//------------------------------------------------------------------------------
#if defined(TARGET_XBOX) && !defined(X_DEBUG)
    #define x_memcpy memcpy
    #define x_memset memset
    #define x_memcmp memcmp
#endif

//==============================================================================
//  Standard ASCII conversion functions.
//==============================================================================
//  
//  x_toupper   - Return upper case version of given character.
//  x_tolower   - Return lower case version of given character.
//  x_atoi      - Return s32 value based on string such as "123".
//  x_atof      - Return f32 value based on string such as "1.5".
//  
//==============================================================================

s32     x_toupper   ( s32 C );
s32     x_tolower   ( s32 C );
s32     x_atoi      ( const char* pStr );
f32     x_atof      ( const char* pStr );

//==============================================================================
//  Standard character classifiers.
//==============================================================================
//  
//  x_isspace   - White space?  (Space, tab, cariage return, line feed.)
//  x_isdigit   - '0' thru '9'.
//  x_isalpha   - 'A' thru 'Z' or 'a' thru 'z'.
//  
//==============================================================================

inline xbool x_isspace( s32 C ) { return( (C == 0x09) || (C == 0x0A) || (C == 0x0D) || (C == ' ') ); }
inline xbool x_isdigit( s32 C ) { return( (C >=  '0') && (C <= '9') ); }
inline xbool x_isalpha( s32 C ) { return( ((C >= 'A') && (C <= 'Z')) || ((C >= 'a') && (C <= 'z')) ); }
inline xbool x_ishex  ( s32 C ) { return( ((C >= 'A') && (C <= 'F')) || ((C >= 'a') && (C <= 'f')) || ((C >=  '0') && (C <= '9')) ); }

//==============================================================================
//  Path manipulation functions.
//==============================================================================
//  
//  x_splitpath - Break a fully or partially qualified path/filename into its
//                components.
//  
//  x_makepath  - Assemble a path/filename from given parts.
//  
//  Except for the pPath pointer, NULL may be used for any parameter which is 
//  not needed.  For example, if you have a file name which may or may not have 
//  path information and you just need the base file name, then use something 
//  like...
//  
//      char* pFileName;
//      char  FName[ X_MAX_FNAME ];
//      ...
//      x_splitpath( pFileName, NULL, NULL, FName, NULL );
//  
//  The X_MAX_... values represent the largest sizes of their respective
//  portions of a fully qualified path and file name.
//  
//==============================================================================

#define X_MAX_PATH   260
#define X_MAX_DRIVE    3
#define X_MAX_DIR    256
#define X_MAX_FNAME  256
#define X_MAX_EXT    256

//==============================================================================

void x_splitpath    ( const char* pPath, char* pDrive,
                                         char* pDir,
                                         char* pFName,
                                         char* pExt );

void x_makepath     ( char* pPath, const char* pDrive,
                                   const char* pDir,  
                                   const char* pFName,
                                   const char* pExt );


//==============================================================================
void x_encrypt      ( void* pData,  s32   Length,   u32   Key[4] );
void x_decrypt      ( void* pData,  s32   Length,   u32   Key[4] );
//==============================================================================
template <class T>
class x_compare_functor
{
public:
    s32 operator()( T, T );
};

//------------------------------------------------------------------------------
//
//  What the hell is a PseudoQuickSort?  Simple!  Its "sorta quick sort".
//
//  Find a partition element and put it in the first position of the list.  The 
//  partition value is the median value of the first, middle, and last items.  
//  (Using the median value of these three items rather than just using the 
//  first item is a big win.)
//
//  Then, the usual partitioning and swapping, followed by swapping the 
//  partition element into place.
//
//  Then, of the two portions of the list which remain on either side of the 
//  partition, sort the smaller portion recursively, and sort the larger 
//  portion via another iteration of the same code.
//
//  Do not bother "quick sorting" lists (or sub lists) which have fewer than
//  RECURSION_THRESH items.  All final sorting is handled with an insertion sort
//  which is executed by the caller (x_qsort).  This is another huge win.  This 
//  means that this function does not actually completely sort the input list.  
//  It mostly sorts it.  That is, each item will be within RECURSION_THRESH 
//  positions of its correct position.  Thus, an insert sort will be able to 
//  finish off the sort process without a serious performance hit.
//
//  All data swaps are done in-line, which trades some code space for better
//  performance.  There are only three swap points, anyway.
//
//------------------------------------------------------------------------------

template <class T, class Cmp>
static
void PseudoQuickSort( T*      pBase,
                      T*      pMax,           
                      s32     RecursionThresh,
                      s32     PartitionThresh,
                      Cmp     Compare )
{
    T*  i;
    T*  j;
    T*  jj;
    T*  mid;
    T*  tmp;
    s32 lo;
    s32 hi;

    lo = (s32)(pMax - pBase);   // Total data to sort in objects.

    // Deep breath now...
    do
    {
        //
        // At this point, lo is the number of objects in the items in the current
        // partition.  We want to find the median value item of the first, 
        // middle, and last items.  This median item will become the middle 
        // item.  Set j to the "greater of first and middle".  If last is larger
        // than j, then j is the median.  Otherwise, compare the last item to 
        // "the lesser of the first and middle" and take the larger.  The code 
        // is biased to prefer the middle over the first in the event of a tie.
        //

        mid = i = pBase + ((u32)(lo) >> 1);

        if( lo >= PartitionThresh )
        {
            j = (Compare( *(jj = pBase), *i ) > 0)  ?  jj : i;

            if( Compare( *j, *(tmp = pMax - 1) ) > 0 )
            {
                // Use lesser of first and middle.  (First loser.)
                j = (j == jj ? i : jj);
                if( Compare( *j, *tmp ) < 0 )
                    j = tmp;
            }

            if( j != i )
            {
                // Swap!
                T c  = *i;
                *i++ = *j;
                *j++ = c;
            }
        }

        //
        // Semi-standard quicksort partitioning / swapping...
        //

        for( i = pBase, j = pMax - 1;  ;  )
        {
            while( (i < mid) && (Compare( *i, *mid ) <= 0) )
            {
                i += 1;
            }

            while( j > mid )
            {
                if( Compare( *mid, *j ) <= 0 )
                {
                    j -= 1;
                    continue;
                }

                tmp = i + 1;     // Value of i after swap.

                if( i == mid )
                {
                    // j <-> mid, new mid is j.
                    mid = jj = j;       
                }
                else
                {
                    // i <-> j
                    jj = j;
                    j -= 1;
                }

                goto SWAP;
            }

            if( i == mid )
            {
                break;
            }
            else
            {
                jj  = mid;
                tmp = mid = i;
                j  -= 1;
            }
SWAP:
            {
                T c   = *i;
                *i++  = *jj;
                *jj++ = c;
            }

            i = tmp;
        }

        //
        // Consider the sizes of the two partitions.  Process the smaller
        // partition first via recursion.  Then process the larger partition by
        // iterating through the above code again.  (Update variables as needed
        // to make it work.)
        //
        // NOTE:  Do not bother sorting a given partition, either recursively or
        // by another iteration, if the size of the partition is less than 
        // RECURSION_THRESH items.
        //

        j = mid;
        i = mid + 1;

        if( (lo = (j-pBase)) <= (hi = (pMax-i)) )
        {
            if( lo >= RecursionThresh )
                PseudoQuickSort( pBase, j, 
                                 RecursionThresh,
                                 PartitionThresh,
                                 Compare );
            pBase = i;
            lo    = hi;
        }
        else
        {
            if( hi >= RecursionThresh )
                PseudoQuickSort( i, pMax,
                                 RecursionThresh,
                                 PartitionThresh,
                                 Compare );
            pMax = j;
        }

    } while( lo >= RecursionThresh );
}

//==============================================================================

#define X_QSORT_RECURSION_THRESH 4
#define X_QSORT_PARTITION_THRESH 6

template <class T, class Cmp>
void x_qsort( T* apBase, s32 NItems, Cmp Compare )
{
    T*  i;
    T*  j;
    T*  lo;
    T*  hi;
    T*  min;
    T*  pMax;
    T*  pBase = apBase;
    s32 RecursionThresh;          // Recursion threshold in objects
    s32 PartitionThresh;          // Partition threshold in objects

    ASSERT( pBase    );
    ASSERT( NItems   >= 0 );

    // Easy out?
    if( NItems <= 1 )
        return;

    // Set up some useful values.
    RecursionThresh = X_QSORT_RECURSION_THRESH;
    PartitionThresh = X_QSORT_PARTITION_THRESH;
    pMax            = pBase + NItems;

    //
    // Set the 'hi' value.
    // Also, if there are enough values, call the PseudoQuickSort function.
    //
    if( NItems >= X_QSORT_RECURSION_THRESH )
    {
        PseudoQuickSort( pBase,
                         pMax, 
                         RecursionThresh, 
                         PartitionThresh, 
                         Compare );
        hi = pBase + RecursionThresh;
    }
    else
    {
        hi = pMax;
    }

    //
    // Find the smallest element in the first "MIN(RECURSION_THRESH,NItems)"
    // items.  At this point, the smallest element in the entire list is 
    // guaranteed to be present in this sublist.
    //
    for( j = lo = pBase; (lo += 1) < hi;  )
    {
        if( Compare( *j, *lo ) > 0 )
            j = lo;
    }

    // 
    // Now put the smallest item in the first position to prime the next 
    // loop.
    //
    if( j != pBase )
    {
        i  = pBase;
        hi = pBase + 1;

        T c  = *j;
        *j++ = *i;
        *i++ = c;
    }

    //
    // Smallest item is in place.  Now we run the following hyper-fast
    // insertion sort.  For each remaining element, min, from [1] to [n-1],
    // set hi to the index of the element AFTER which this one goes.  Then,
    // do the standard insertion sort shift on a byte at a time basis.
    //
    for( min = pBase; (hi = min += 1) < pMax;  )
    {
        while( Compare( *(hi -= 1), *min ) > 0 )
        {
            // No body in this loop.
        }

        if( (hi += 1) != min )
        {
            lo = min;
            T c = *lo;
            for( i = j = lo; (j -= 1) >= hi; i = j )
            {
                *i = *j;
            }
            *i = c;
        }
    }
} 

//==============================================================================
// Assumption: Letters A-Z and a-z are contiguous in the character set.
// This is true for ASCII and UniCode, but not for EBCDIC!

inline s32 x_toupper( s32 C )
{
    if( (C >= 'a') && (C <= 'z') )
        C += ('A' - 'a');
    return( C );
}

//==============================================================================
// Assumption: Letters A-Z and a-z are contiguous in the character set.
// This is true for ASCII and UniCode, but not for EBCDIC!

inline s32 x_tolower( s32 C )
{
    if( (C >= 'A') && (C <= 'Z') )
        C += ('a' - 'A');
    return( C );
}

//==============================================================================
#endif // X_PLUS_HPP
//==============================================================================
