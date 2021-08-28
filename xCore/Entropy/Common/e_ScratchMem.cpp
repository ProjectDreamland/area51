//==============================================================================
//  
//  e_ScratchMem.cpp
//
//==============================================================================

//==============================================================================
//  
//  
//              +---BufferTop                             StackTop---+
//              |                                                    |
//              |                            StackMarker[]--+--+--+  |
//              |                                           |  |  |  |
//              V                                       ... v  v  v  V
//  Storage---> +----------------------------------------------------+
//              |                                                    |
//              |    Buffer -->                         <-- Stack    |
//              |                                                    |
//              +----------------------------------------------------+
//  
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "..\e_ScratchMem.hpp"
#include "x_memory.hpp"
#include "x_stdio.hpp"
#include "x_threads.hpp"

#ifdef TARGET_PS2
#include "eekernel.h"
#endif

//==============================================================================
//  DEFINES
//==============================================================================

#define SMEM_ALIGNMENT  SMEM_ALIGN(1)       // Do not change this.

//==============================================================================
//  STORAGE
//==============================================================================

xbool   g_SMemInitialized = FALSE;
byte*   g_pSMemBufferTop;
byte*   g_pSMemStackTop;
byte*   g_pSMemMarker[ MAX_MARKERS ];
s32     g_SMemNextMarker;
s32     g_SMemThreadId = -1;

static  s32     s_ActiveID = 0 ;    
                
static  s32     s_Active;
static  byte*   s_pStorage[2];
static  byte*   s_pAllocedStorage[2];
static  s32     s_CurrentSize[2];
static  s32     s_RequestedSize;
                
static  byte    s_Signature[ SMEM_ALIGNMENT ];
        s32     SCRATCH_MEM_MAX_USED = 0;

enum calltype
{
    BUFFER_ALLOC = 0,
    STACK_ALLOC,
    STACK_PUSH_MARKER,
    STACK_POP_MARKER,
};

//==============================================================================
//==============================================================================
//==============================================================================
#ifdef USE_LOG // IF LOGGING
//==============================================================================
//==============================================================================
//==============================================================================
//#define USE_LOG


//==============================================================================

struct smem_log
{
    calltype Type;
    char     FileName[32];
    s32      FileLine;
    s32      Amount;
};

#define MAX_LOG_ENTRIES 1024
static smem_log s_Log[MAX_LOG_ENTRIES];
static s32      s_NLogEntries;

//==============================================================================

static
void smem_AddLogEntry( calltype Type, const char* pFileName, s32 Line, s32 Amount=0 )
{
    ASSERT( s_NLogEntries < MAX_LOG_ENTRIES );

    s32 Len = x_strlen( pFileName ) + 1;
    if( Len > 32 )  pFileName += (Len-32);
    x_strncpy( s_Log[s_NLogEntries].FileName, pFileName, 32 );

    s_Log[s_NLogEntries].Type = Type;
    s_Log[s_NLogEntries].FileLine = Line;
    s_Log[s_NLogEntries].Amount = Amount;

    s_NLogEntries++;
    (void)Type;
    (void)pFileName;
    (void)Line;
    (void)Amount;
}

//==============================================================================

static
void smem_DumpLog( void )
{
    x_DebugMsg("====== SCRATCH MEM LOG DUMP ======\n");

    s32 I = s_NLogEntries-1;
    s32 N = 0;
    while( (I>=0) && (N<10) )
    {
        switch( s_Log[I].Type )
        {
        case BUFFER_ALLOC: x_DebugMsg("%3d] BUFFER_ALLOC %5d LINE:%4d <%s>\n",I,s_Log[I].Amount,s_Log[I].FileLine,s_Log[I].FileName);
            break;
        case STACK_ALLOC:  x_DebugMsg("%3d] STACK_ALLOC  %5d LINE:%4d <%s>\n",I,s_Log[I].Amount,s_Log[I].FileLine,s_Log[I].FileName);
            break;
        case STACK_PUSH_MARKER: x_DebugMsg("%3d] STACK_PUSH_M       LINE:%4d <%s>\n",I,s_Log[I].FileLine,s_Log[I].FileName);
            break;
        case STACK_POP_MARKER: x_DebugMsg("%3d] STACK_POP_M        LINE:%4d <%s>\n",I,s_Log[I].FileLine,s_Log[I].FileName);
            break;
        }
    
        I--;
        N++;
    }
}

//==============================================================================
//==============================================================================
//==============================================================================
#endif // END LOGGING
//==============================================================================
//==============================================================================
//==============================================================================

//==============================================================================

s32 smem_GetBufferSize( void )
{
    return s_RequestedSize;
}

//==============================================================================

s32 smem_GetMaxUsed( void )
{
    return SCRATCH_MEM_MAX_USED;
}

//==============================================================================

void smem_SetThreadId( s32 ThreadId )
{
    g_SMemThreadId = ThreadId;
}


//==============================================================================
//  FUNCTIONS
//==============================================================================

//==============================================================================
//  Internal functions
//==============================================================================

static
void smem_ActivateSection( void )
{
    g_pSMemBufferTop = s_pStorage[ s_Active ];
    g_pSMemStackTop  = g_pSMemBufferTop + s_CurrentSize[ s_Active ];
    
    // SMEM should come from uncached memory if we are to use MFIFO. This is
    // because with MFIFO, there is not a good location to flush the cache, and
    // that has to be done before any render data that is referenced from
    // SMEM can be DMA'ed. Using uncached writes means we don't have to flush
    // the cache, but we will sacrifices some CPU performance. Specific areas
    // that need to be hand-optimized can do that themselves...
    #ifdef TARGET_PS2
    g_pSMemBufferTop = (byte*)((u32)g_pSMemBufferTop | 0x20000000);
    g_pSMemStackTop  = (byte*)((u32)g_pSMemStackTop  | 0x20000000);
    #endif
    
    // Place a signature below the buffer region for overrun checking.
    x_memcpy( g_pSMemBufferTop, s_Signature, SMEM_ALIGNMENT );
    g_pSMemBufferTop += SMEM_ALIGNMENT;

    // We also place a signature above the stack region.
    g_pSMemStackTop -= SMEM_ALIGNMENT;
    x_memcpy( g_pSMemStackTop, s_Signature, SMEM_ALIGNMENT );

    // The next available marker is 0.
    g_SMemNextMarker = 0;
}

//==============================================================================
//  Exposed functions
//==============================================================================

void smem_Init( s32 NBytes )
{
    ASSERT( !g_SMemInitialized );
    ASSERT( NBytes >= 0 );
    
    MEMORY_OWNER( "smem_Init()" );

    // Build the signature.
    x_memset( s_Signature, '~', SMEM_ALIGNMENT );
    x_memcpy( s_Signature + (SMEM_ALIGNMENT>>1) - 2, "SMEM", 4 );

    s_RequestedSize = SMEM_ALIGN( NBytes );

    // add 64 for cache line alignment purposes (necessary for the
    // PS2, and shouldn't hurt the other platforms)
    s_pAllocedStorage[0] = (byte*)x_malloc( s_RequestedSize + 64 );
    s_pAllocedStorage[1] = (byte*)x_malloc( s_RequestedSize + 64 );
    s_pStorage[0] = (byte*)ALIGN_64( s_pAllocedStorage[0] );
    s_pStorage[1] = (byte*)ALIGN_64( s_pAllocedStorage[1] );
    s_CurrentSize[1] = s_RequestedSize;
    s_CurrentSize[0] = s_RequestedSize;

    s_Active = 0;
    smem_ActivateSection();

    g_SMemInitialized = TRUE;
    SCRATCH_MEM_MAX_USED = 0;

    #ifdef USE_LOG
    s_NLogEntries = 0;
    #endif

    // Since we'll be accessing the memory as uncached on the PS2, we need to
    // make sure that any debugging memory fillers (i.e. 0xFEEDC0DE) are flushed
    #ifdef TARGET_PS2
    FlushCache( WRITEBACK_DCACHE );
    #endif
}

//==============================================================================

void smem_Kill( void )
{
    ASSERT( g_SMemInitialized );
    ASSERT( g_SMemNextMarker == 0 );

    x_free( s_pAllocedStorage[0] );
    x_free( s_pAllocedStorage[1] );
    s_pAllocedStorage[0] = NULL;
    s_pAllocedStorage[1] = NULL;
    s_pStorage[0]        = NULL;
    s_pStorage[1]        = NULL;
    s_CurrentSize[0] = 0;
    s_CurrentSize[1] = 0;

    #ifdef USE_LOG
    s_NLogEntries = 0;
    #endif

    g_SMemInitialized = FALSE;
}

//==============================================================================

extern xbool g_bInsideRTF;

void smem_Toggle( void )
{
    MEMORY_OWNER( "smem_Toggle()" );

    ASSERT( g_SMemInitialized );

    //x_printfxy(0,3,"MAXSMEM %1d",SCRATCH_MEM_MAX_USED);

    // Make sure all markers which were pushed, have been popped.
#if defined( TARGET_PS2 ) && !defined( TARGET_DVD )
    ASSERT( (g_bInsideRTF) || (g_SMemNextMarker == 0) );
#else
    ASSERT( g_SMemNextMarker == 0 );
#endif

    // Check to see if the signature under the buffer was violated.
    //
    // NOTE - There is a "signature" under the buffer area of scratch memory.
    //        This signature is no longer intact.  If you got an ASSERT failure
    //        here, then some code is trashing memory.
    //
    ASSERT( x_memcmp( s_pStorage[s_Active], 
                      s_Signature, SMEM_ALIGNMENT ) == 0 );

    // Make sure the stack wasn't blown.
    //
    // NOTE - If you got an ASSERT failure here, then you probably overran a 
    //        scratch memory stack allocation.
    //
    ASSERT( x_memcmp( s_pStorage[s_Active] + 
                        s_CurrentSize[s_Active] - 
                        SMEM_ALIGNMENT, 
                      s_Signature, SMEM_ALIGNMENT ) == 0 );

    // Remember most used
#ifdef TARGET_PS2
    SCRATCH_MEM_MAX_USED = MAX( (((u32)g_pSMemBufferTop&0x0FFFFFFF)-(u32)s_pStorage[ s_Active ]), (u32)SCRATCH_MEM_MAX_USED );
#else
    SCRATCH_MEM_MAX_USED = MAX( (g_pSMemBufferTop-s_pStorage[ s_Active ]), SCRATCH_MEM_MAX_USED );
#endif


    // To make life a little easier, get the index to the non-active section.
    s32 Other = 1 - s_Active;

    // Right now, "s_Active" is active.  The "other" section must no longer be
    // needed, otherwise we wouldn't be here.  If the program has requested a 
    // change in scratch memory size, we can go ahead and resize the "other" 
    // section now.

    if( s_CurrentSize[Other] != s_RequestedSize )
    {
        x_free( s_pAllocedStorage[Other] );
        s_pAllocedStorage[Other] = (byte*)x_malloc( s_RequestedSize + 64 );
        s_pStorage[Other] = (byte*)ALIGN_64( s_pAllocedStorage[Other] );
        s_CurrentSize[Other] = s_RequestedSize;
    }

    // Toggle the sections.

    s_Active = Other;
    smem_ActivateSection();

    // Next ID
    s_ActiveID++ ;

    // Clear log
    #ifdef USE_LOG
    s_NLogEntries = 0;
    #endif
}

//==============================================================================

void smem_ResetAfterException( void )
{
    g_SMemNextMarker = 0;
    smem_Toggle();
}

//==============================================================================

void smem_ChangeSize( s32 NBytes )
{
    ASSERT( g_SMemInitialized );
    ASSERT( NBytes >= 0 );
    s_RequestedSize = SMEM_ALIGN( NBytes );
}

//==============================================================================
//==============================================================================
//==============================================================================
#ifdef X_DEBUG // DEBUG
//==============================================================================
//==============================================================================
//==============================================================================

byte* smem_dfunc_BufferAlloc( s32 NBytes, const char* pFileName, s32 FileLine )
{
    (void)pFileName;
    (void)FileLine;

    ASSERT( (g_SMemThreadId == -1) || (g_SMemThreadId == x_GetThreadID()) );
    ASSERT( g_SMemInitialized );
    ASSERT( NBytes >= 0 );

    #ifdef USE_LOG
    smem_AddLogEntry( BUFFER_ALLOC, pFileName, FileLine, NBytes );
    #endif

    // Make sure the previous buffer allocation did not overrun.
    //
    // NOTE - If you got an ASSERT failure here, then you probably overran a 
    //        scratch memory buffer allocation.
    //
    if( x_memcmp( g_pSMemBufferTop - SMEM_ALIGNMENT, 
                    s_Signature, SMEM_ALIGNMENT ) != 0 )
    {
#ifdef USE_LOG
        smem_DumpLog();
#endif
        ASSERT(FALSE);
    }

    // Allocate memory for the request.
    byte* pAllocation = g_pSMemBufferTop;
    g_pSMemBufferTop += SMEM_ALIGN( NBytes );

    // Stick in a new overrun detection signature.
    x_memcpy( g_pSMemBufferTop, s_Signature, SMEM_ALIGNMENT );
    g_pSMemBufferTop += SMEM_ALIGNMENT;

    // Make sure we haven't exhausted this section.
    //
    // NOTE - If you got an ASSERT failure here, then you have consumed all 
    //        available scratch memory.  Increase the amount of memory dedicated
    //        to the scratch memory system.
    //
    if( g_pSMemBufferTop > g_pSMemStackTop )
    {
        #ifdef USE_LOG
        smem_DumpLog();
        #endif

        x_DebugMsg("smem_BufferAlloc Failed!!!\n");
        ASSERT( FALSE );
    }

    // Return the allocated memory.
    return( pAllocation );
}

//==============================================================================

byte* smem_dfunc_StackAlloc( s32 NBytes, const char* pFileName, s32 FileLine )
{
    (void)pFileName;
    (void)FileLine;

    ASSERT( (g_SMemThreadId == -1) || (g_SMemThreadId == x_GetThreadID()) );
    ASSERT( g_SMemInitialized );
    ASSERT( NBytes >= 0 );

    #ifdef USE_LOG
    smem_AddLogEntry( STACK_ALLOC, pFileName, FileLine, NBytes );
    #endif

    // Allocate memory for the request.
    g_pSMemStackTop -= SMEM_ALIGN( NBytes );
    byte* pAllocation = g_pSMemStackTop;

    // Stick in a new overrun detection signature.
    g_pSMemStackTop -= SMEM_ALIGNMENT;
    x_memcpy( g_pSMemStackTop, s_Signature, SMEM_ALIGNMENT );

    // Make sure we haven't exhausted this section.
    //
    // NOTE - If you got an ASSERT failure here, then you have consumed all 
    //        available scratch memory.  Increase the amount of memory dedicated
    //        to the scratch memory system.
    //
    if( g_pSMemBufferTop > g_pSMemStackTop )
    {
        #ifdef USE_LOG
        smem_DumpLog();
        #endif

        x_DebugMsg("smem_StackAlloc Failed!!!\n");
        ASSERT( FALSE );
    }

    // Return the allocated memory.
    return( pAllocation );
}

//==============================================================================

void smem_dfunc_StackPushMarker( const char* pFileName, s32 FileLine )
{
    (void)pFileName;
    (void)FileLine;

    ASSERT( (g_SMemThreadId == -1) || (g_SMemThreadId == x_GetThreadID()) );
    ASSERT( g_SMemInitialized );
    ASSERT( g_SMemNextMarker < MAX_MARKERS );   // Too many markers!

    g_pSMemMarker[g_SMemNextMarker] = g_pSMemStackTop;
    g_SMemNextMarker++;

    #ifdef USE_LOG
    smem_AddLogEntry( STACK_PUSH_MARKER, pFileName, FileLine );
    #endif
}

//==============================================================================

void smem_dfunc_StackPopToMarker( const char* pFileName, s32 FileLine )
{
    (void)pFileName;
    (void)FileLine;

    ASSERT( (g_SMemThreadId == -1) || (g_SMemThreadId == x_GetThreadID()) );
    ASSERT( g_SMemInitialized );
    ASSERT( g_SMemNextMarker > 0 );     // Too many pops!

    g_SMemNextMarker--;
    g_pSMemStackTop = g_pSMemMarker[g_SMemNextMarker];

    #ifdef USE_LOG
    smem_AddLogEntry( STACK_POP_MARKER, pFileName, FileLine );
    #endif

    // Make sure we didn't violate the overrun signature at the previous marker.
    //
    // NOTE - If you got an ASSERT failure here, then you probably overran a 
    //        scratch memory stack allocation.
    //
    if( x_memcmp( g_pSMemMarker[g_SMemNextMarker], 
                   s_Signature, SMEM_ALIGNMENT ) != 0 )
    {
#ifdef USE_LOG
        smem_DumpLog();
#endif
        ASSERT(FALSE);
    }
}

//==============================================================================
//==============================================================================
//==============================================================================
#endif // DEBUG
//==============================================================================
//==============================================================================
//==============================================================================


//==============================================================================

// These functions can be used to make sure you have a valid scratch allocation

//==============================================================================

s32 smem_GetActiveID( void )
{
    return s_ActiveID ;    
}

//==============================================================================

byte* smem_GetActiveStartAddr( void ) 
{
    // Get start of current buffer
    byte* Addr = s_pStorage[ s_Active ];

    // Skip signature
    Addr += SMEM_ALIGNMENT ;

    return Addr ;
}

//==============================================================================

byte* smem_GetActiveEndAddr( void )
{
    // Just return current active buffer position
    return g_pSMemBufferTop ;
}

//==============================================================================

void smem_Validate( void )
{
#ifdef X_DEBUG
    // Make sure the previous buffer allocation did not overrun.
    //
    // NOTE - If you got an ASSERT failure here, then you probably overran a 
    //        scratch memory buffer allocation.
    //
    if( x_memcmp( g_pSMemBufferTop - SMEM_ALIGNMENT, 
                    s_Signature, SMEM_ALIGNMENT ) != 0 )
    {
        #ifdef USE_LOG
            smem_DumpLog();
        #endif

        ASSERT(FALSE);
    }

#endif
}

//==============================================================================
