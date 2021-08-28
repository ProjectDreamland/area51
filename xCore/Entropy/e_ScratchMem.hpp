//==============================================================================
//  
//  e_ScratchMem.hpp
//
//==============================================================================

#ifndef E_SCRATCHMEM_HPP
#define E_SCRATCHMEM_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_types.hpp"
#include "x_debug.hpp"
#include "x_plus.hpp"

//==============================================================================
//  FUNCTIONS
//==============================================================================

//------------------------------------------------------------------------------
//  Public functions
//------------------------------------------------------------------------------

void    smem_ChangeSize         ( s32 NBytes );
byte*   smem_BufferAlloc        ( s32 NBytes ); // Can return NULL
byte*   smem_StackAlloc         ( s32 NBytes ); // Can return NULL
void    smem_StackPushMarker    ( void );
void    smem_StackPopToMarker   ( void );
s32     smem_GetMaxUsed         ( void );
s32     smem_GetBufferSize      ( void );
void    smem_SetThreadId        ( s32 ThreadId );

//------------------------------------------------------------------------------
//  Private functions
//------------------------------------------------------------------------------

void    smem_Init               ( s32 NBytes );
void    smem_Kill               ( void );
void    smem_Toggle             ( void );
void    smem_ResetAfterException( void );

//==============================================================================
//==============================================================================
#ifdef X_DEBUG
#define smem_BufferAlloc(S)     smem_dfunc_BufferAlloc( (S), __FILE__, __LINE__ )
#define smem_StackAlloc(S)      smem_dfunc_StackAlloc( (S), __FILE__, __LINE__ )
#define smem_StackPushMarker()  smem_dfunc_StackPushMarker( __FILE__, __LINE__ )
#define smem_StackPopToMarker() smem_dfunc_StackPopToMarker( __FILE__, __LINE__ )
byte*   smem_dfunc_BufferAlloc        ( s32 NBytes, const char* pFileName, s32 Line );
byte*   smem_dfunc_StackAlloc         ( s32 NBytes, const char* pFileName, s32 Line );
void    smem_dfunc_StackPushMarker    ( const char* pFileName, s32 Line );
void    smem_dfunc_StackPopToMarker   ( const char* pFileName, s32 Line );
#else
#define smem_BufferAlloc(S)     smem_rfunc_BufferAlloc( (S) )
#define smem_StackAlloc(S)      smem_rfunc_StackAlloc( (S) )
#define smem_StackPushMarker()  smem_rfunc_StackPushMarker()
#define smem_StackPopToMarker() smem_rfunc_StackPopToMarker()
byte*   smem_rfunc_BufferAlloc        ( s32 NBytes );
byte*   smem_rfunc_StackAlloc         ( s32 NBytes );
void    smem_rfunc_StackPushMarker    ( void );
void    smem_rfunc_StackPopToMarker   ( void );
#endif

// These functions can be used to make sure you have a valid scratch allocation
s32     smem_GetActiveID        ( void ) ;
byte*   smem_GetActiveStartAddr ( void ) ;
byte*   smem_GetActiveEndAddr   ( void ) ;
void    smem_Validate           ( void ) ;


//==============================================================================
//==============================================================================
// PRIVATE VARIABLES -- DON'T ACCESS DIRECTLY!!!!
#ifdef TARGET_PS2
// PS2 needs 64-byte alignment to guarantee that all uncached accesses are
// aligned to the size of a cache line. (We return uncached pointers, but we
// can't guarantee the game will not make them cached.)
#define SMEM_ALIGN      ALIGN_64
#else
#define SMEM_ALIGN      ALIGN_16            // Set to ALIGN_8, ALIGN_16, etc.
#endif
#define MAX_MARKERS     16

extern xbool   g_SMemInitialized;
extern byte*   g_pSMemBufferTop;
extern byte*   g_pSMemStackTop;
extern byte*   g_pSMemMarker[ MAX_MARKERS ];
extern s32     g_SMemNextMarker;
extern s32     g_SMemThreadId;

//==============================================================================
//==============================================================================

//==============================================================================
//==============================================================================
//==============================================================================
#ifndef X_DEBUG // RELEASE
//==============================================================================
//==============================================================================
//==============================================================================

inline byte* smem_rfunc_BufferAlloc( s32 NBytes )
{
    ASSERT( (g_SMemThreadId == -1) || (g_SMemThreadId == x_GetThreadID()) );
    ASSERT( g_SMemInitialized );
    ASSERT( NBytes >= 0 );

    // Allocate memory for the request.
    byte* pAllocation = g_pSMemBufferTop;
    g_pSMemBufferTop += SMEM_ALIGN( NBytes );

    // Make sure we haven't exhausted this section.
    //
    // NOTE - If you got an ASSERT failure here, then you have consumed all 
    //        available scratch memory.  Increase the amount of memory dedicated
    //        to the scratch memory system.
    //

    //if( g_pSMemBufferTop > g_pSMemStackTop )
    //{
    //    x_DebugMsg("smem_BufferAlloc Failed!!!\n");
    //    ASSERT( FALSE );
    //}

    // Return the allocated memory.
    return( pAllocation );
}

//==============================================================================

inline byte* smem_rfunc_StackAlloc( s32 NBytes )
{
    ASSERT( (g_SMemThreadId == -1) || (g_SMemThreadId == x_GetThreadID()) );
    ASSERT( g_SMemInitialized );
    ASSERT( NBytes >= 0 );

    // Allocate memory for the request.
    g_pSMemStackTop -= SMEM_ALIGN( NBytes );
    byte* pAllocation = g_pSMemStackTop;

    // Make sure we haven't exhausted this section.
    //
    // NOTE - If you got an ASSERT failure here, then you have consumed all 
    //        available scratch memory.  Increase the amount of memory dedicated
    //        to the scratch memory system.
    //
    if( g_pSMemBufferTop > g_pSMemStackTop )
    {
        x_DebugMsg("smem_StackAlloc Failed!!!\n");
        ASSERT( FALSE );
    }

    // Return the allocated memory.
    return( pAllocation );
}

//==============================================================================

inline void smem_rfunc_StackPushMarker( void )
{
    ASSERT( (g_SMemThreadId == -1) || (g_SMemThreadId == x_GetThreadID()) );
    ASSERT( g_SMemInitialized );
    ASSERT( g_SMemNextMarker < MAX_MARKERS );   // Too many markers!

    g_pSMemMarker[g_SMemNextMarker] = g_pSMemStackTop;
    g_SMemNextMarker++;
}

//==============================================================================

inline void smem_rfunc_StackPopToMarker( void )
{
    ASSERT( (g_SMemThreadId == -1) || (g_SMemThreadId == x_GetThreadID()) );
    ASSERT( g_SMemInitialized );
    ASSERT( g_SMemNextMarker > 0 );     // Too many pops!

    g_SMemNextMarker--;
    g_pSMemStackTop = g_pSMemMarker[g_SMemNextMarker];
}

//==============================================================================
//==============================================================================
//==============================================================================
#endif // RELEASE
//==============================================================================
//==============================================================================
//==============================================================================

//==============================================================================
#endif // E_SCRATCHMEM_HPP
//==============================================================================
