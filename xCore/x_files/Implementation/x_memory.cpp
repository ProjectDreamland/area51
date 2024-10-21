//==============================================================================
//
//  x_memory.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_MEMORY_HPP
#include "..\x_memory.hpp"
#endif

#ifndef X_FILES_PRIVATE_HPP
#include "x_files_private.hpp"
#endif

#ifndef X_STDIO_HPP
#include "..\x_stdio.hpp"
#endif

#ifndef X_STRING_HPP
#include "..\x_string.hpp"
#endif

#include "..\x_threads.hpp"

#include "..\x_log.hpp"

#include <stdio.h>

#ifdef TARGET_XBOX
#   ifdef CONFIG_RETAIL
#       define D3DCOMPILE_PUREDEVICE 1
#   endif
#   include <xtl.h>
#endif

//==============================================================================
//  DEFINES
//==============================================================================

#ifdef x_malloc
#undef x_malloc
#endif

#ifdef new
#undef new
#endif

#ifdef x_free
#undef x_free
#endif

#ifdef x_realloc
#undef x_realloc
#endif

#if defined(TARGET_PS2) && defined(X_DEBUG)
#define DO_MEMORY_FILLS
#endif

#define MEMORY_ALLOC_FILL_VALUE (0xFEEDC0DE)
#define MEMORY_FREE_FILL_VALUE  (0xDEADBEEF)

#if defined(X_MEM_DEBUG)
#define PAD_BYTES    16     // Pad mem_header to 48 bytes (multiple of 16).
#else
#define PAD_BYTES    0      // Pad mem_header to 16 bytes (multiple of 16).
#endif  

#define PAD_VALUE       (('~' << 24) | ('~' << 16) | ('~' << 8) | ('~' << 0))

#define MAX_MARKS 1024

#if defined(USE_OWNER_STACK)
static const    s32     OWNER_STRING_BUFFER_SIZE    = 16*1024;
static const    s32     MAX_OWNER_STRINGS           = 1024;
static const    s32     MAX_OWNER_STACK_SIZE        = 16*1024;
static          char    OwnerStringBuffer[OWNER_STRING_BUFFER_SIZE];
static          s32     OwnerStringBufferUsed       = 0;
static          char*   OwnerStrings[MAX_OWNER_STRINGS];
static          u8      OwnerInclude[MAX_OWNER_STRINGS];
static          u8      OwnerExclude[MAX_OWNER_STRINGS];
static          u8      OwnerRequire[MAX_OWNER_STRINGS];
static          xbool   OwnerRequirementExists=FALSE;
static          s32     OwnerStack[MAX_OWNER_STACK_SIZE];
static          s32     nOwnerStrings;
static          s32     OwnerStackSize;
#endif // defined(USE_OWNER_STACK)

//==============================================================================
//  TYPES
//==============================================================================

#ifndef X_RETAIL
#define USE_MEM_HEADERS
#endif

#ifdef USE_MEM_HEADERS

struct mem_header
{
    s32         Sequence;
    s32         RequestedSize;
    mem_header* pNext;
    mem_header* pPrev;

#ifdef X_MEM_DEBUG

    s16         Mark;
    s16         LineNumber;
    const char* pFilename;

    #if defined(USE_OWNER_STACK)
        u8          OwnerStack[8];
    #else
        const char* pFunction;
        s32         Dummy;
    #endif // defined(USE_OWNER_STACK)

#endif // X_MEM_DEBUG

    #if PAD_BYTES > 0
        u32     Pad[ PAD_BYTES / 4 ];
    #endif
};

#endif

#if !defined(USE_MEM_HEADERS) && defined(X_MEM_DEBUG)
#error Bad combo of USE_MEM_HEADERS and X_MEM_DEBUG
#endif

#if !defined(USE_MEM_HEADERS) && defined(USE_OWNER_STACK)
#error Bad combo of USE_MEM_HEADERS and USE_OWNER_STACK
#endif

//==============================================================================
//  STORAGE
//==============================================================================

#ifdef USE_MEM_HEADERS
static s32   Sequence = 1;
static xbool UsingNew = FALSE;
#endif

#ifdef X_MEM_DEBUG
    static s32  CurrentCount = 0;
    static s32  CurrentBytes = 0;
    static s32  TotalBytes   = 0;
    static s32  MaxBytes     = 0;

    static s32  Mark         = 0;
    static s32  MarkSequence[ MAX_MARKS ]      = { 0 };
    static char MarkComment [ MAX_MARKS * 28 ];
    static const char* LimitLength(const char* pString,s32 count);
#if defined(TARGET_DEV) && !defined(USE_OWNER_STACK)
    static const char* PrettyFunction(char* pOut, const char* pString,s32 count);
#endif

#endif

#ifdef USE_MEM_HEADERS
static mem_header   Anchor;
#endif

// Craig: Do not remove this without speaking with me!!!
#if defined( TARGET_PS2 ) && !defined( X_RETAIL )
byte g_MemoryBallast[1*1024];
#endif


#if defined(USE_OWNER_STACK)

void x_MemQueryExcludeAll( void )
{
    // Nuke the inclusion list
    x_memset( OwnerInclude, 0, sizeof(OwnerInclude) );
    x_memset( OwnerRequire, 0, sizeof(OwnerRequire) );
    x_memset( OwnerExclude, 0, sizeof(OwnerExclude) );
    OwnerRequirementExists = FALSE;
}

void x_MemQueryIncludeAll( void )
{
    // Nuke the inclusion list
    x_memset( OwnerInclude, 1, sizeof(OwnerInclude) );
    x_memset( OwnerRequire, 0, sizeof(OwnerRequire) );
    x_memset( OwnerExclude, 0, sizeof(OwnerExclude) );
    OwnerRequirementExists = FALSE;
}

void x_MemQueryInclude( const char* pString )
{
    for( s32 i=1 ; i<nOwnerStrings ; i++ )
    {
        if( x_strstr( OwnerStrings[i], pString ) != NULL )
        {
            OwnerInclude[i] = TRUE;
        }
    }
}

void x_MemQueryRequire( const char* pString )
{
    OwnerRequirementExists = TRUE;

    s32 count = 0;
    for( s32 i=1 ; i<nOwnerStrings ; i++ )
    {
        if( x_strstr( OwnerStrings[i], pString ) != NULL )
        {
            OwnerRequire[i] = TRUE;
            count++;
        }
    }

    ASSERT( count <= 1 );
}

void x_MemQueryExclude( const char *pString )
{
    for( s32 i=1 ; i<nOwnerStrings ; i++ )
    {
        if( x_strstr( OwnerStrings[i], pString ) != NULL )
            OwnerExclude[i] = TRUE;
    }
}

s32 x_MemQuery( xbool bIncludeAll )
{
    s32 result = 0;
    mem_header* pHeader = Anchor.pNext;
    while( pHeader != &Anchor )
    {
        // Is it included, excluded, or satisfy the requirement?
        xbool bInclude      = bIncludeAll;
        xbool bExclude      = FALSE;
        xbool bHasRequired  = FALSE;

        // Loop through and determine the different bools
        for( s32 i=0 ; (i<8); i++ )
        if( pHeader->OwnerStack[i] )
        {
            if( OwnerInclude[ pHeader->OwnerStack[i] ] )
                bInclude = TRUE;

            if( OwnerExclude[ pHeader->OwnerStack[i] ] )
                bExclude = TRUE;

            if( OwnerRequire[ pHeader->OwnerStack[i] ] )
                bHasRequired = TRUE;
        }

        // Is it included and not excluded?
        if( bInclude && (bExclude==FALSE) )
        {
            // Do we satisfied required or there was no required?
            if( (OwnerRequirementExists==FALSE) || bHasRequired )
            {
                result += pHeader->RequestedSize;
            }
        }

        pHeader = pHeader->pNext;
    }

    return result;
}
#endif // USE_OWNER_STACK

//==============================================================================
//  FUNCTIONS
//==============================================================================
//  PLACEHOLDER IMPLEMENTATIONS!
//==============================================================================

void x_MemInit( void )
{
    sys_mem_Init();

#ifdef USE_MEM_HEADERS
    Anchor.Sequence      =  0;
    Anchor.RequestedSize = -1;
    Anchor.pNext         = &Anchor;
    Anchor.pPrev         = &Anchor;
#endif

#ifdef X_MEM_DEBUG
    Anchor.Mark         =  0;
    Anchor.LineNumber   = -1;
    Anchor.pFilename    = __FILE__;

    #if defined(USE_OWNER_STACK)
        OwnerStrings[0] = NULL;
        nOwnerStrings   = 1;
        OwnerStackSize  = 0;
    #else
        #ifdef TARGET_PC
            // Not supported by PC compiler
            Anchor.pFunction    = "unknown" ;
        #elif defined( TARGET_XBOX )
            Anchor.pFunction    = "unknown" ;
        #else
            Anchor.pFunction    = __PRETTY_FUNCTION__;
        #endif // TARGET_PC
    #endif // defined(USE_OWNER_STACK)

    #if PAD_BYTES > 0
        x_memset( Anchor.Pad, '*',  PAD_BYTES );
    #endif
    x_memset( MarkComment, 0, sizeof(MarkComment) );
    x_strcpy( MarkComment, "Default 1st Mark" );

#endif // X_MEM_DEBUG

}

//==============================================================================

void x_MemKill( void )
{
    #ifdef X_MEM_DEBUG
    x_MemSanity();
    // The current thread has its "globals" allocated, so the count can't be 0.
//    if( CurrentCount > 1 )
//        x_MemDump();
    #endif
}

//==============================================================================

//==============================================================================
#ifdef X_MEM_DEBUG
//==============================================================================

#if defined(USE_OWNER_STACK)

//==============================================================================
// x_MemGrabOwnerStack( u8* Buffer, s32 BufferSize )
// Grabs the top indices off the stack, up to BufferSize.
// If buffer is larger than stack, then end of buffer is
// padded with 0s. The 0s then refer to the 0th element
// of OwnerStrings, which points to NULL.
//==============================================================================

void x_MemGrabOwnerStack( u8* Buffer, s32 BufferSize )
{
#if defined(X_RETAIL)
    (void)Buffer;
    (void)BufferSize;
#else
    s32 i;
    const s32 Stop = MIN( BufferSize, OwnerStackSize );
    for ( i = 0; i < Stop; ++i )
    {
        Buffer[i] = OwnerStack[OwnerStackSize-i-1];        
    }

    // Pad the rest with 0s
    for ( i = OwnerStackSize; i < BufferSize; ++i )
    {
        Buffer[i] = 0;
    }
#endif
}

// mreed TODO: this is a linear search into an ordered array, could be faster
// rmb TODO: no, it cant be faster in its current implementation.
xbool FindOwner( const char* pOwnerName, u8& OwnerIdx )
{
    if ( !pOwnerName )
    {
        ASSERT( 0 ); // expecting valid string, but not fatal
        return FALSE;
    }

    s32 i;

    // start at 1 since 0 always holds the null string
    for ( i = 1; i < nOwnerStrings; ++i )
    {
        ASSERT( OwnerStrings[i] ); // expecting valid string, but not fatal
        if ( OwnerStrings[i] && (x_strcmp( OwnerStrings[i], pOwnerName ) == 0) )
        {
            OwnerIdx = i;
            return TRUE;
        }
    }

    return FALSE;
}

//==============================================================================

u8 InsertOwner( const char* pOwnerName )
{
    if ( !pOwnerName )
    {
        ASSERT( 0 ); // expecting valid string, but not fatal
        return 0; // return the null string index
    }

    // note: This is not an exclusive insertion. It appends the string to the
    //       end of the OwnerStringBuffer, and inserts it's pointer into the
    //       OwnerStrings
    const s32 Bytes = x_strlen( pOwnerName ) + 1;
    if ( ((OwnerStringBufferUsed + Bytes) <= OWNER_STRING_BUFFER_SIZE)
        && (nOwnerStrings < MAX_OWNER_STRINGS-1) )
    {
        //----------------------------------------------------
        // we have room, append the owner name to the buffer
        //----------------------------------------------------

        // store the string ptr
        char* pBufferOwnerName = (char *)(OwnerStringBuffer + OwnerStringBufferUsed); 

        x_strcpy( (char*)(OwnerStringBuffer + OwnerStringBufferUsed), pOwnerName );
        OwnerStringBufferUsed += Bytes;

        //Append the string pointer to OwnerStrings
        OwnerStrings[nOwnerStrings] = pBufferOwnerName;
        ++nOwnerStrings;
        return (nOwnerStrings-1);
    }
    else
    {
        ASSERT(0);

        // Dump out the stack
        s32 i;
        if ( ((OwnerStringBufferUsed + Bytes) >= OWNER_STRING_BUFFER_SIZE) )
        {
            x_DebugMsg( "x_MemPushOwner ran out of string buffer space\n" );
        }
        else if ( nOwnerStrings > MAX_OWNER_STRINGS-1 )
        {
            x_DebugMsg( "x_MemPushOwner ran out of strings\n" );
        }

        x_DebugMsg( "DUMP:\n*****\n\n" );
        for ( i = 0; i < OwnerStackSize; ++i )
        {
            x_DebugMsg( "%s\n", OwnerStack[i] );
        }
        ASSERT( FALSE ); // ran out of string buffer, or OwnerStrings
        // not a fatal problem though
        return 0;
    }
}

void x_MemPushOwner( const char* pOwnerName )
{
    (void)pOwnerName;

    // Check to see if this owner is in our table
    u8 OwnerIdx;
    if ( !FindOwner( pOwnerName, OwnerIdx ) )
    {
        // Add this owner
        OwnerIdx = InsertOwner( pOwnerName );
    }

    // Push it onto our stack
    if( OwnerStackSize < MAX_OWNER_STACK_SIZE )
    {
        OwnerStack[OwnerStackSize] = OwnerIdx;
        ++OwnerStackSize;
    }
    else
    {
        // Overflow is bad!!!!
        ASSERT( 0 );
    }
}

//==============================================================================

void x_MemPopOwner( void )
{
    if( OwnerStackSize > 0 )
    {
        --OwnerStackSize;
        OwnerStack[OwnerStackSize] = 0;
    }
}

#endif // defined(USE_OWNER_STACK)

//==============================================================================

#ifdef DO_MEMORY_FILLS
void FillMemory( void* pMemory, u32 Value, s32 nBytes )
{
    u32* pM = (u32*)pMemory;
    s32  C  = nBytes / 4;
    while( C-- )
    {
        *pM = Value;
        pM++;
    }
}
#endif

//==============================================================================

s32 XMEMORY_WATCH_SIZE = -1;

static s32 s_Free,s_Largest,s_Fragments;
void* x_debug_malloc( s32         NBytes,
                      const char* pFileName, 
                      s32         LineNumber,
                      const char* pFunction)
{
    ASSERT( NBytes >= 0 );
    ASSERT( pFileName );

    if( XMEMORY_WATCH_SIZE != -1 )
    {
        if( XMEMORY_WATCH_SIZE == NBytes )
        {
            BREAK;
        }
    }

    x_BeginAtomic();

    mem_header* pHeader = (mem_header*)sys_mem_malloc( NBytes + sizeof(mem_header) );
   
    if( pHeader == NULL )
    {
        x_EndAtomic();

        #ifdef CONFIG_VIEWER
        ((u32*)1)[0] = 0xBAADC0DE;
        #endif

        x_MemDump( "AllocationFailure.txt", TRUE );
        x_MemSanity();
    }

    // Make sure we haven't run out of heap space.
    ASSERTS( pHeader, 
             (const char*)xfs( "Heap exhuasted.  Malloc %d bytes from %s(%d).", 
                               NBytes, LimitLength(pFileName,32), LineNumber ) );

    // Fill memory
    #ifdef DO_MEMORY_FILLS
    FillMemory( pHeader, MEMORY_ALLOC_FILL_VALUE, NBytes + sizeof(mem_header) );
    #endif

    // Insert into double linked list BEFORE the anchor.
    pHeader->pNext          = &Anchor;
    pHeader->pPrev          = Anchor.pPrev;
    Anchor.pPrev            = pHeader;
    pHeader->pPrev->pNext   = pHeader;

    // Set up header fields.
    pHeader->RequestedSize  = NBytes;
    pHeader->Mark           = Mark;
    pHeader->Sequence       = UsingNew ? Sequence : -Sequence;
    pHeader->LineNumber     = LineNumber;    
    pHeader->pFilename      = pFileName;
#if defined(USE_OWNER_STACK)
    (void)pFunction;
    x_MemGrabOwnerStack( pHeader->OwnerStack, 8 );
#else
    pHeader->pFunction      = pFunction;
#endif // defined(USE_OWNER_STACK)

    #if PAD_BYTES > 0
        for( s32 i = 0; i < (PAD_BYTES/4); i++ )
            pHeader->Pad[i] = PAD_VALUE;
    #endif
    Sequence = MAX( Sequence+1, 1 );

    // Update stats.
    CurrentCount += 1;
    CurrentBytes += NBytes;
    TotalBytes   += NBytes;
    MaxBytes      = MAX( MaxBytes, CurrentBytes );
    x_EndAtomic();
#if !defined(TARGET_PC)
    LOG_MALLOC( pHeader, NBytes+sizeof(mem_header), pFileName, LineNumber );
#endif
    static s32 count=0;
    if (count==0)
    {
        x_MemGetFree(s_Free,s_Largest,s_Fragments);
        count=50;
    }
    count--;
    // Done!
    return( pHeader + 1 );
}

void* x_debug_realloc( void* pMemory, s32 NewNBytes, const char* pFileName, s32 Line )
{
    ASSERT( NewNBytes >= 0 );

    if( pMemory == NULL )
    {
        #ifdef X_MEM_DEBUG
            return( x_debug_malloc( NewNBytes, pFileName, Line, NULL ) );
        #else   
            return( x_malloc( NewNBytes ) );
        #endif
    }

    x_BeginAtomic();

    mem_header* pHeader = ((mem_header*)pMemory) - 1;

    // Remove from double link list.
    pHeader->pNext->pPrev = pHeader->pPrev;
    pHeader->pPrev->pNext = pHeader->pNext;

    // Update stats.
    #ifdef X_MEM_DEBUG
        CurrentBytes -= pHeader->RequestedSize;
        CurrentBytes += NewNBytes;
        TotalBytes   += MAX( 0, NewNBytes - pHeader->RequestedSize );
        MaxBytes      = MAX( MaxBytes, CurrentBytes );
    #endif

    // Reallocate.
    pHeader = (mem_header*)sys_mem_realloc( pHeader, NewNBytes + sizeof(mem_header) );

    if( pHeader == NULL )
    {
        byte* pBuff = (byte*)sys_mem_malloc( NewNBytes + sizeof(mem_header) );
        if( pBuff )
        {
            // Fill memory
            #ifdef DO_MEMORY_FILLS
            FillMemory( pBuff, MEMORY_ALLOC_FILL_VALUE, NewNBytes + sizeof(mem_header));
            #endif

            x_memcpy( pBuff + sizeof(mem_header), pMemory, NewNBytes );
            pHeader = (mem_header*)pBuff;
            sys_mem_free(pMemory);
        }
    }

    // Make sure we haven't run out of heap space.
    ASSERTS( pHeader, 
             (const char*)xfs( "Heap exhuasted.  Realloc %d bytes.", NewNBytes ) );

    // Insert into double linked list BEFORE the anchor.
    pHeader->pNext        = &Anchor;
    pHeader->pPrev        = Anchor.pPrev;
    Anchor.pPrev          = pHeader;
    pHeader->pPrev->pNext = pHeader;

    // Update header fields.
    #ifdef X_MEM_DEBUG
    pHeader->Mark          = Mark;
    #endif
    pHeader->RequestedSize = NewNBytes;
    pHeader->Sequence      = UsingNew ? Sequence : -Sequence;
    Sequence = MAX( Sequence+1, 1 );

    x_EndAtomic();
#if !defined(TARGET_PC)
    LOG_REALLOC( pHeader, (void*)((u32)pMemory - sizeof(mem_header)), NewNBytes + sizeof(mem_header), pFileName, Line );
#endif
    // Done!
    return( pHeader + 1 );
}

void x_debug_free( void* pMemory, const char* pFileName, s32 Line )
{
    (void)pFileName;
    (void)Line;

    if( pMemory == NULL )
        return;

    x_BeginAtomic();

    mem_header* pHeader = ((mem_header*)pMemory) - 1;

    // Look for trouble!
#ifdef X_MEM_DEBUG
    #if PAD_BYTES > 0
    for( s32 i = 0; i < (PAD_BYTES/4); i++ )
        if( pHeader->Pad[i] != PAD_VALUE ) // Corruption!
        {
            x_EndAtomic();
            ASSERTS( FALSE, "FATAL! Memory Corrupt! Bad Pad!" );
        }
    #endif // PAD_BYTES
    if( (pHeader->pNext->pPrev != pHeader) || (pHeader->pPrev->pNext != pHeader) ) // Corruption!
    {
        x_EndAtomic();
        ASSERTS( FALSE, "FATAL! Memory Corrupt! Bad Links!" );
    }
#endif // X_MEM_DEBUG

    // Update stats.
    #ifdef X_MEM_DEBUG
        CurrentCount -= 1;
        CurrentBytes -= pHeader->RequestedSize;
    #endif

    // Remove from double link list.
    pHeader->pNext->pPrev = pHeader->pPrev;
    pHeader->pPrev->pNext = pHeader->pNext;

    #ifdef DO_MEMORY_FILL
    FillMemory( pHeader, MEMORY_FREE_FILL_VALUE, pHeader->RequestedSize + siezof(mem_header) );
    #endif

    // Done!
    sys_mem_free( pHeader );
    x_EndAtomic();
#if !defined(TARGET_PC)
    LOG_FREE( pHeader, pFileName, Line );
#endif
}

//==============================================================================
#else // X_MEM_DEBUG
//==============================================================================

extern void x_DebugSetCause( const char* pCause );

#ifdef USE_MEM_HEADERS
static s32 s_Free,s_Largest,s_Fragments;
#endif

void* x_malloc( s32 NBytes )
{
    ASSERT( NBytes >= 0 );

#ifdef USE_MEM_HEADERS

    x_BeginAtomic();
    mem_header* pHeader = (mem_header*)sys_mem_malloc( NBytes + sizeof(mem_header) );

    if (!pHeader)
    {
        s32 Free,Largest,Fragments;

        x_MemGetFree(Free,Largest,Fragments);
        printf("Failed allocation, size=%d, free=%d, largest=%d, fragments=%d\n",NBytes,Free,Largest,Fragments);
        x_EndAtomic();
        LOG_FLUSH();
        // Make sure we haven't run out of heap space.
        ASSERTS( pHeader, 
                 (const char*)xfs( "Heap exhuasted.  Malloc %d bytes.", NBytes ) );

#if defined( CONFIG_VIEWER ) || (defined( CONFIG_QA )&& !defined TARGET_XBOX)
        x_DebugSetCause( xfs( "OUT OF MEMORY - need: %d\n"
                              "                       free: %d\n"
                              "                    largest: %d\n"
                              "                  fragments: %d", NBytes, Free, Largest, Fragments ) );
        *(u32*)1 = 0;
#endif

        return NULL;
    }

    // Fill memory
    #ifdef DO_MEMORY_FILLS
    FillMemory( pHeader, MEMORY_ALLOC_FILL_VALUE, NBytes + sizeof(mem_header) );
    #endif

    // Insert into double linked list BEFORE the anchor.
    pHeader->pNext        = &Anchor;
    pHeader->pPrev        = Anchor.pPrev;
    Anchor.pPrev          = pHeader;
    pHeader->pPrev->pNext = pHeader;

    // Set up header fields.
    pHeader->RequestedSize = NBytes;
    pHeader->Sequence      = UsingNew ? Sequence : -Sequence;
    Sequence = MAX( Sequence+1, 1 );
    x_EndAtomic();
#if !defined(TARGET_PC)
    LOG_MALLOC( pHeader, NBytes+sizeof(mem_header), "", 0 );
#endif
    static s32 count=0;
    if (count==0)
    {
        x_MemGetFree(s_Free,s_Largest,s_Fragments);
        count=50;
    }
    count--;
    // Done!
    return( pHeader + 1 );

#else // USE_MEM_HEADERS

    x_BeginAtomic();
    void* pMemory = sys_mem_malloc( NBytes );
    x_EndAtomic();
    return pMemory;

#endif // USE_MEM_HEADERS

}

void* x_realloc( void* pMemory, s32 NewNBytes )
{

    ASSERT( NewNBytes >= 0 );

#ifdef USE_MEM_HEADERS

    if( pMemory == NULL )
    {
        #ifdef X_MEM_DEBUG
            return( x_debug_malloc( NewNBytes, "<realloc>", -1, NULL ) );
        #else   
            return( x_malloc( NewNBytes ) );
        #endif
    }

    x_BeginAtomic();

    mem_header* pHeader = ((mem_header*)pMemory) - 1;

    // Remove from double link list.
    pHeader->pNext->pPrev = pHeader->pPrev;
    pHeader->pPrev->pNext = pHeader->pNext;

    // Update stats.
    #ifdef X_MEM_DEBUG
        CurrentBytes -= pHeader->RequestedSize;
        CurrentBytes += NewNBytes;
        TotalBytes   += MAX( 0, NewNBytes - pHeader->RequestedSize );
        MaxBytes      = MAX( MaxBytes, CurrentBytes );
    #endif

    // Reallocate.
    pHeader = (mem_header*)sys_mem_realloc( pHeader, NewNBytes + sizeof(mem_header) );

    if( pHeader == NULL )
    {
        byte* pBuff = (byte*)sys_mem_malloc( NewNBytes + sizeof(mem_header) );
        if( pBuff )
        {
            // Fill memory
            #ifdef DO_MEMORY_FILLS
            FillMemory( pBuff, MEMORY_ALLOC_FILL_VALUE, NewNBytes + sizeof(mem_header));
            #endif

            x_memcpy( pBuff + sizeof(mem_header), pMemory, NewNBytes );
            pHeader = (mem_header*)pBuff;
            sys_mem_free(pMemory);
        }
    }

    // Make sure we haven't run out of heap space.
    ASSERTS( pHeader, 
             (const char*)xfs( "Heap exhuasted.  Realloc %d bytes.", NewNBytes ) );

    // Insert into double linked list BEFORE the anchor.
    pHeader->pNext        = &Anchor;
    pHeader->pPrev        = Anchor.pPrev;
    Anchor.pPrev          = pHeader;
    pHeader->pPrev->pNext = pHeader;

    // Update header fields.
    #ifdef X_MEM_DEBUG
    pHeader->Mark          = Mark;
    #endif
    pHeader->RequestedSize = NewNBytes;
    pHeader->Sequence      = UsingNew ? Sequence : -Sequence;
    Sequence = MAX( Sequence+1, 1 );

    x_EndAtomic();
#if !defined(TARGET_PC)
    LOG_REALLOC( pHeader, pMemory, NewNBytes + sizeof(mem_header), "", 0 );
#endif
    // Done!
    return( pHeader + 1 );

#else // USE_MEM_HEADERS

    x_BeginAtomic();
    pMemory = sys_mem_realloc( pMemory, NewNBytes );
    x_EndAtomic();
    return pMemory;

#endif // USE_MEM_HEADERS
}

void x_free( void* pMemory )
{

#ifdef USE_MEM_HEADERS

    if( pMemory == NULL )
        return;

    x_BeginAtomic();

    mem_header* pHeader = ((mem_header*)pMemory) - 1;

    // Look for trouble!
    #ifdef X_MEM_DEBUG
        #if PAD_BYTES > 0
            for( s32 i = 0; i < (PAD_BYTES/4); i++ )
                ASSERT( pHeader->Pad[i] == PAD_VALUE );     // Corruption!
        #endif
        ASSERT( pHeader->pNext->pPrev == pHeader );         // Corruption!
        ASSERT( pHeader->pPrev->pNext == pHeader );         // Corruption!
    #endif

    // Update stats.
    #ifdef X_MEM_DEBUG
        CurrentCount -= 1;
        CurrentBytes -= pHeader->RequestedSize;
    #endif

    // Remove from double link list.
    pHeader->pNext->pPrev = pHeader->pPrev;
    pHeader->pPrev->pNext = pHeader->pNext;

    #ifdef DO_MEMORY_FILL
    FillMemory( pHeader, MEMORY_FREE_FILL_VALUE, pHeader->RequestedSize + sizeof(mem_header) );
    #endif

    // Done!
    sys_mem_free( pHeader );
    x_EndAtomic();
#if !defined(TARGET_PC)
    LOG_FREE( pHeader, "", 0 );
#endif

#else // USE_MEM_HEADERS

    x_BeginAtomic();
    sys_mem_free( pMemory );
    x_EndAtomic();

#endif // USE_MEM_HEADERS

}

//==============================================================================
#endif // X_MEM_DEBUG
//==============================================================================

//==============================================================================
//  C++ OPERATORS 'new' AND 'delete'
//==============================================================================

//==============================================================================
#ifndef USE_SYSTEM_NEW_DELETE
//==============================================================================

void* operator new( xalloctype Size )
{
    void* pResult;
    #ifdef USE_MEM_HEADERS
    UsingNew = TRUE;
    #endif
    #ifdef X_MEM_DEBUG
    pResult  = x_debug_malloc( (s32)Size, "<unknown>", -1, NULL );
    #else
    pResult  = x_malloc( (s32)Size );
    #endif
    #ifdef USE_MEM_HEADERS
    UsingNew = FALSE;
    #endif
    return( pResult );
}

//==============================================================================

void* operator new [] ( xalloctype Size )
{
    void* pResult;
    #ifdef USE_MEM_HEADERS
    UsingNew = TRUE;
    #endif
    #ifdef X_MEM_DEBUG
    pResult  = x_debug_malloc( (s32)Size, "<unknown>", -1, NULL );
    #else
    pResult  = x_malloc( (s32)Size );
    #endif
    #ifdef USE_MEM_HEADERS
    UsingNew = FALSE;
    #endif
    return( pResult );
}

//==============================================================================

void operator delete( void* pMemory )
{
    if( pMemory == NULL )
        return;
    #ifdef USE_MEM_HEADERS
    UsingNew = TRUE;
    #endif
    #ifdef X_MEM_DEBUG
        x_debug_free( pMemory, "<unknown>", -1 );
    #else
        x_free( pMemory );
    #endif
    #ifdef USE_MEM_HEADERS
    UsingNew = FALSE;
    #endif
}

//==============================================================================

void operator delete [] ( void* pMemory )
{
    if( pMemory == NULL )
        return;
    #ifdef USE_MEM_HEADERS
    UsingNew = TRUE;
    #endif
    #ifdef X_MEM_DEBUG
        x_debug_free( pMemory, "<unknown>", -1 );
    #else
        x_free( pMemory );
    #endif
    #ifdef USE_MEM_HEADERS
    UsingNew = FALSE;
    #endif
}

//==============================================================================
#ifdef X_MEM_DEBUG
//==============================================================================

void* operator new( xalloctype Size, const char* pFileName, s32 LineNumber, const char* pFunction )
{
    void* pResult;
    #ifdef USE_MEM_HEADERS
    UsingNew = TRUE;
    #endif
    pResult  = x_debug_malloc( (s32)Size, pFileName, LineNumber, pFunction );
    #ifdef USE_MEM_HEADERS
    UsingNew = FALSE;
    #endif
    return( pResult );
}

//==============================================================================

void* operator new [] ( xalloctype Size, const char* pFileName, s32 LineNumber, const char* pFunction )
{
    (void)pFunction;
    void* pResult;
    #ifdef USE_MEM_HEADERS
    UsingNew = TRUE;
    #endif
    pResult  = x_debug_malloc( (s32)Size, pFileName, LineNumber, pFunction );
    #ifdef USE_MEM_HEADERS
    UsingNew = FALSE;
    #endif
    return( pResult );
}

//==============================================================================

void operator delete( void* pMemory, const char* pFileName, s32 LineNumber, const char* pFunction )
{
    (void)pFunction;
    if( pMemory == NULL )
        return;
    #ifdef USE_MEM_HEADERS
    UsingNew = TRUE;
    #endif
    x_debug_free( pMemory, pFileName, LineNumber );
    #ifdef USE_MEM_HEADERS
    UsingNew = FALSE;
    #endif
}

//==============================================================================

void operator delete [] ( void* pMemory, const char* pFileName, s32 LineNumber, const char* pFunction )
{
    (void)pFunction;
    if( pMemory == NULL )
        return;
    #ifdef USE_MEM_HEADERS
    UsingNew = TRUE;
    #endif
    x_debug_free( pMemory, pFileName, LineNumber );
    #ifdef USE_MEM_HEADERS
    UsingNew = FALSE;
    #endif
}

//==============================================================================
#endif // X_MEM_DEBUG
//==============================================================================
#endif // !USE_SYSTEM_NEW_DELETE
//==============================================================================

void x_MemAddMark( const char* pComment )
{
    #ifdef X_MEM_DEBUG
    Mark += 1;

    if( Mark == MAX_MARKS )
    {
        mem_header* pHeader = Anchor.pNext;
        while( pHeader != &Anchor )
        {
            pHeader->Mark = 0;
            pHeader = pHeader->pNext;
        }
        Mark = 0;
    }

    x_strncpy( &MarkComment[ Mark * 28 ], pComment, 27 );
    MarkSequence[Mark] = Sequence;
    Sequence = MAX( Sequence+1, 1 );
    #else
    (void)pComment;
    #endif
}

//==============================================================================

void x_MemClearMarks( void )
{
    #ifdef X_MEM_DEBUG
    Mark = 0;
    x_strcpy( MarkComment, "Default 1st Mark" );
    MarkSequence[0] = 0;
    #endif
}

//==============================================================================

void x_MemDump( void )
{
    x_MemDump( "MemDump.txt", TRUE );
}

#if defined(X_MEM_DEBUG)
static const char* LimitLength(const char* pString,s32 count) 
{
    if (!pString)
        return "<none>";

    if (x_strlen(pString) > count)
        return pString+x_strlen(pString)-count;

    return pString;
}

#if defined(TARGET_DEV) && !defined(USE_OWNER_STACK)
static const char* PrettyFunction(char* pOut, const char* pString, s32 Count)
{
    const char* pSrc;
    // If we have no string, it's a realloc so we just bail right now.
    if (!pString)
    {
        return "<realloc>";
    }

    pOut[Count]=0x0;
    // Look for the first occurance of the '(' for the function list
    pSrc = x_strchr(pString,'(');
    // If we didn't find one, or our string is less than 32 characters,
    // just return the size limited string
    if ( (!pSrc) || (x_strlen(pString)<=Count) )
    {
        x_strncpy(pOut,pString,Count);
        return pOut;
    }

    // Now search back from the '(' to find the first white space which will
    // mark the end of the return value prototype
    s32 Length;

    Length=0;
    while ( (pSrc != pString) && (*pSrc != ' ') && (Length < Count) )
    {
        pSrc--;
        Length++;
    }
    if (*pSrc==' ')
    {
        Length--;
        pSrc++;
    }


    if ( x_strlen(pSrc) < Count )
    {
        return pString+x_strlen(pString)-Count;
    }
    else
    {
        x_strncpy(pOut,pSrc,Count);
    }
    return pOut;
}
#endif
#endif
//==============================================================================

void x_MemDump( const char* pFileName, xbool bCommaSeperated )
{
    (void)pFileName;
    (void)bCommaSeperated;

#ifdef USE_MEM_HEADERS

#if defined(X_MEM_DEBUG)
    (void)LimitLength;
#endif
#if defined(TARGET_DEV)

#if defined(X_MEM_DEBUG) && !defined(USE_OWNER_STACK)
    char NameBuffer[80];
#endif
    X_FILE* pFile = x_fopen( pFileName, "wt" );
    
    ASSERT( pFile );

    x_DebugMsg("Starting memory dump.....\n");

#if !defined(USE_OWNER_STACK)
    x_fprintf( pFile, "Mark Sequence Malloc     Size       Addr Line File                              Function\n" );
    x_fprintf( pFile, "==== ======== ====== ======== ========== ==== ================================  ================================\n" );
#endif // !defined(USE_OWNER_STACK)

    s32 CurrentTotal=0;
    mem_header* pHeader = Anchor.pNext;
    while( pHeader != &Anchor )
    {
        #ifdef X_MEM_DEBUG
        if( pHeader->Mark > pHeader->pPrev->Mark )
        {
            x_fprintf( pFile, "%4d %8d ------ -------- ---------- ---- ----%s\n",
                               pHeader->Mark, 
                               MarkSequence[ pHeader->Mark ], 
                               &MarkComment[ pHeader->Mark * 28 ] );
        }
        #endif

        #if defined(X_MEM_DEBUG)
            #if defined(USE_OWNER_STACK)
                if( bCommaSeperated )
                {
                    x_fprintf( pFile, "%4d, %8d, %s, %8d, 0x%08X, %4d, %-32s,  ", 
                        pHeader->Mark,
                        ABS( pHeader->Sequence ), 
                        (pHeader->Sequence > 0) ? "new   " : "malloc",
                        pHeader->RequestedSize, 
                        (u32)(pHeader+1),
                        pHeader->LineNumber, 
                        LimitLength(pHeader->pFilename,32) );
                }
                else
                {
                    x_fprintf( pFile, "%4d %8d %s %8d 0x%08X %4d %-32s  ", 
                                    pHeader->Mark,
                                    ABS( pHeader->Sequence ), 
                                    (pHeader->Sequence > 0) ? "new   " : "malloc",
                                    pHeader->RequestedSize, 
                                    (u32)(pHeader+1),
                                    pHeader->LineNumber, 
                                    LimitLength(pHeader->pFilename,32) );
                }
                for( s32 i=7 ; i>=0  ; i-- )
                {
                    if( pHeader->OwnerStack[i] )
                        x_fprintf( pFile, "\\ %s ", OwnerStrings[pHeader->OwnerStack[i]] );
                }
                x_fprintf( pFile, "\n" );
                                
            #else
                x_fprintf( pFile, "%4d %8d %s %8d 0x%08X %4d %-32s  %-32s\n", 
                    pHeader->Mark,
                    ABS( pHeader->Sequence ), 
                    (pHeader->Sequence > 0) ? "new   " : "malloc",
                    pHeader->RequestedSize, 
                    (u32)(pHeader+1),
                    pHeader->LineNumber, 
                    LimitLength(pHeader->pFilename,32),
                    PrettyFunction(NameBuffer,pHeader->pFunction,32) );
            #endif // defined(USE_OWNER_STACK)
        #else
        x_fprintf( pFile, ".... %8d %s %8d 0x%08X .... ....\n", 
                          ABS( pHeader->Sequence ), 
                          (pHeader->Sequence > 0) ? "new   " : "malloc",
                          pHeader->RequestedSize,
                          (u32)(pHeader+1) );
        #endif // X_MEM_DEBUG

        CurrentTotal += pHeader->RequestedSize;

        pHeader = pHeader->pNext;
    }

#if !defined(USE_OWNER_STACK)
    x_fprintf( pFile, "==== ======== ====== ======== ========== ==== ================================  ================================\n" );
    x_fprintf( pFile, "Mark Sequence Malloc     Size       Addr Line File                              Function\n" );
    #ifndef X_MEM_DEBUG
    x_fprintf( pFile, ".... ........ ...... %8d ........ .... ....\n", CurrentTotal);
    #endif

    #ifdef X_MEM_DEBUG
    if( Mark > 0 )
    {
        s32         i;
        s32         Allocs;
        s32         Bytes;
        mem_header* pHeader;

        x_fprintf( pFile, "\n\n\n" );
        x_fprintf( pFile, "Mark   Allocs                 TotalBytes      Comment\n" );
        x_fprintf( pFile, "==== ========                 ==========      ================================\n" );

        for( i = 0; i <= Mark; i++ )
        {
            pHeader = Anchor.pNext;
            Allocs  = 0;
            Bytes   = 0;

            while( pHeader != &Anchor )
            {
                if( pHeader->Mark == i )
                {
                    Allocs += 1;
                    Bytes  += pHeader->RequestedSize;
                }
                pHeader = pHeader->pNext;
            }

            x_fprintf( pFile, "%4d %8d ...... ........ %10d .... %s\n",
                              i, Allocs, Bytes, &MarkComment[i*28] );
        }

        x_fprintf( pFile, "==== ========                 ==========      ================================\n" );
        x_fprintf( pFile, "Mark   Allocs                 TotalBytes      Comment\n" );
    }
    #endif

    #ifdef X_MEM_DEBUG
    {
        mem_header* pA = Anchor.pNext;
        mem_header* pB = Anchor.pNext;

        x_fprintf( pFile, "\n\n\n" );
        x_fprintf( pFile, "       Allocs                 TotalBytes      File\n" );
        x_fprintf( pFile, "     ========                 ==========      ================================\n" );

        pA = Anchor.pNext;

        while( pA != &Anchor )
        {
            // Check to see if we've seen allocations from this file before.
            pB = Anchor.pNext;
            while( pB != &Anchor )
            {
                if( pA->pFilename == pB->pFilename )
                    break;
                pB = pB->pNext;
            }

            // If we haven't seen anything from this file before, add to the 
            // total.
            if( pB == pA )
            {
                s32 Count = 0;
                s32 Sum   = 0;

                while( pB != &Anchor )
                {
                    if( pA->pFilename == pB->pFilename )
                    {
                        Count++;
                        Sum += pB->RequestedSize;
                    }
                    pB = pB->pNext;
                }

                // Display values.
                x_fprintf( pFile, "     %8d ...... ........ %10d .... %s\n", Count, Sum, LimitLength(pA->pFilename,64) );
            }

            pA = pA->pNext;
        }

        x_fprintf( pFile, "     ========                 ==========      ================================\n" );
        x_fprintf( pFile, "       Allocs                 TotalBytes      File\n" );
    }
    #endif

    #if defined(X_MEM_DEBUG) && !defined(USE_OWNER_STACK)
    {
        mem_header* pA = Anchor.pNext;
        mem_header* pB = Anchor.pNext;

        x_fprintf( pFile, "\n\n\n" );
        x_fprintf( pFile, "       Allocs                 TotalBytes      Function\n" );
        x_fprintf( pFile, "     ========                 ==========      ================================\n" );

        pA = Anchor.pNext;

        while( pA != &Anchor )
        {
            // Check to see if we've seen allocations from this function before.
            pB = Anchor.pNext;
            while( pB != &Anchor )
            {
                if( pA->pFunction == pB->pFunction )
                    break;
                pB = pB->pNext;
            }

            // If we haven't seen anything from this function before, add to the 
            // total.
            if( pB == pA )
            {
                s32 Count = 0;
                s32 Sum   = 0;

                while( pB != &Anchor )
                {
                    if( pA->pFunction == pB->pFunction )
                    {
                        Count++;
                        Sum += pB->RequestedSize;
                    }
                    pB = pB->pNext;
                }

                // Display values.
                x_fprintf( pFile, "     %8d ...... ........ %10d .... %s\n", Count, Sum, PrettyFunction(NameBuffer,pA->pFunction,64) );
            }

            pA = pA->pNext;
        }

        x_fprintf( pFile, "     ========                 ==========      ================================\n" );
        x_fprintf( pFile, "       Allocs                 TotalBytes      Function\n" );
    }
    #endif
#endif // !defined(USE_OWNER_STACK)
    #ifdef X_MEM_DEBUG  
    {
        x_fprintf( pFile, "\n\n\n" );
        x_fprintf( pFile, "                                   Value      Summary\n" );
        x_fprintf( pFile, "                              ==========      ================================\n" );
        x_fprintf( pFile, "                              %10d .... Sequence\n",        Sequence     );
        x_fprintf( pFile, "                              %10d .... Current Count\n\n", CurrentCount );
        x_fprintf( pFile, "                              %10d .... Total Bytes\n\n",   TotalBytes   );
        x_fprintf( pFile, "                              %10d .... Maximum Bytes\n",   MaxBytes     );
        x_fprintf( pFile, "                              %10d .... Current Bytes\n",   CurrentBytes );
        x_fprintf( pFile, "\n" );
    }
    #endif  

    {
        s32 FreeBytes;
        s32 LargestFree;
        s32 Fragments;

        x_MemGetFree(FreeBytes,LargestFree,Fragments);

        x_fprintf( pFile, "                              %10d .... Free Bytes\n",      FreeBytes    );
        x_fprintf( pFile, "                              %10d .... Largest block\n",   LargestFree  );
        x_fprintf( pFile, "                              %10d .... Block Fragments\n", Fragments    );
        x_fprintf( pFile, "\n" );
    }

    x_fclose( pFile );
    x_DebugMsg("End of memory dump....\n");
#endif

#endif // USE_MEM_HEADERS
}

//==============================================================================

void x_MemSanity( void )
{
#ifdef X_MEM_DEBUG

    x_BeginAtomic();
    mem_header* pHeader = Anchor.pNext;
    while( pHeader != &Anchor )
    {
        ASSERT( pHeader->pNext->pPrev == pHeader );         // Corruption!
        ASSERT( pHeader->pPrev->pNext == pHeader );         // Corruption!
        ASSERT( pHeader->RequestedSize >= 0 );              // Corruption!
        #if PAD_BYTES > 0
            for( s32 i = 0; i < (PAD_BYTES/4); i++ )
                ASSERT( pHeader->Pad[i] == PAD_VALUE );     // Corruption!
        #endif
        pHeader = pHeader->pNext;
    }
    x_EndAtomic();

#endif // X_MEM_DEBUG
}

//============================================================================

#ifdef X_MEM_DEBUG

s32 x_MemGetPtrSize( void* pMemory )
{
    if( pMemory == NULL || pMemory == (void*)ZERO_SIZE_PTR)
        return 0;

    mem_header* pHeader = ((mem_header*)pMemory) - 1;
    return pHeader->RequestedSize;
}

#endif // X_MEM_DEBUG

//==============================================================================
// SYSTEM SPECIFIC MEMORY ALLOCATION FUNCTIONS
//==============================================================================
//
//  The default heap has no limits on how large it can grow so I force a limit
//  using a heap of my own. The malloc() function will still go to the old
//  place.
//
#if defined(TARGET_XBOX) || defined(TARGET_PS2) || defined(TARGET_GCN)
//==============================================================================
// BW - This is a copy of the structure from the x_malloc.cpp file. This will be placed
// in a header file properly when we clean up x_malloc.cpp.

    struct mallinfo 
    {
      int arena;    /* non-mmapped space allocated from system */
      int ordblks;  /* number of free chunks */
      int smblks;   /* number of fastbin blocks */
      int hblks;    /* number of mmapped regions */
      int hblkhd;   /* space in mmapped regions */
      int usmblks;  /* maximum total allocated space */
      int fsmblks;  /* space available in freed fastbin blocks */
      int uordblks; /* total allocated space */
      int fordblks; /* total free space */
      int keepcost; /* top-most, releasable (via malloc_trim) space */
      int flargest; /* Largest free block */
    };

    extern "C"
    {
        void*           dlmalloc(int, size_t);
        void            dlfree(int, void*);
        void*           dlrealloc(int, void*,size_t);
        struct mallinfo dlmallinfo(int);
    }

    void* sys_mem_realloc( void* pBlock, u32 nBytes )
	{
        void *pNew = dlrealloc(0,pBlock,nBytes);
		return pNew;
	}

    void* sys_mem_malloc( u32 nBytes )
	{
#if defined(X_DEBUG)
        static xbool s_ForceMemDump = FALSE;
        if (s_ForceMemDump)
        {
            x_EndAtomic();
            s_ForceMemDump = FALSE;
            x_MemDump();
            x_BeginAtomic();
        }
#endif
        void* pBlock = dlmalloc(0, nBytes );

        #ifdef TARGET_XBOX
        if( ! pBlock )
        {
        static u32 Req  = nBytes;
        static u32 Size = x_MemGetFree();
            D3D__pDevice->Clear( 0,0,D3DCLEAR_TARGET,D3DCOLOR_RGBA( 0,127,0,0),0.0f,0 );
            D3D__pDevice->Present( 0,0,0,0 );
        __asm int 3
        __asm int 3
        }
        #endif

        return pBlock;
	}

    void sys_mem_free( void* pBlock )
	{
		dlfree(0,pBlock);
	}

    void x_MemGetFree( s32& Free, s32& Largest, s32& Fragments)
    {
        // the dlmallinfo needs to happen in an atomic state, but
        // x_MemGetFree may get called from various points, so
        // do a check and put us into atomic if necessary
        x_BeginAtomic();
        mallinfo m = dlmallinfo(0);
        Free      = m.fordblks;
        Largest   = m.flargest;
        Fragments = m.ordblks;
        x_EndAtomic();
    }

#else

    void* sys_mem_realloc( void* pBlock, u32 nBytes )
    {
        return realloc( pBlock, size_t(nBytes) );
    }

    void* sys_mem_malloc( u32 nBytes )
    {

        return malloc( nBytes );
    }

    void sys_mem_free( void* pBlock )
    {
        free( pBlock );
    }

    void    x_MemGetFree( s32& Free, s32& Largest, s32& Fragments)
    {
        Free      = -1;
        Largest   = -1;
        Fragments = -1;
    }

#endif

//---------------------------------------------------------
// System specific memory alloc init routines
#if defined(TARGET_PS2)

void* __heap_start;
void* __heap_end;

extern byte _stack_size[];
extern byte _memory_size[];
extern byte _end[];
extern s32  AdditionalMemoryForDebug;

void    sys_mem_Init(void)
{
    // End of code
    __heap_start = _end;

#if defined(CONFIGURATION_DEBUG) || defined(CONFIGURATION_RELEASE) || defined(X_DEBUG)

    volatile s32 testvar;
    if ( (u32)&testvar > X_MEGABYTE(32) )
    {
        __heap_end   = (void*)( X_MEGABYTE(32) - (u32)_stack_size + AdditionalMemoryForDebug);
    }
    else

#endif
    {
        __heap_end   = (void*)( (u32)_memory_size - (u32)_stack_size );
    }
}

#elif defined(TARGET_GCN)

void* __heap_start;
void* __heap_end;

void    sys_mem_Init(void)
{
    __heap_start = OSGetArenaLo();
    __heap_end   = OSGetArenaHi();
    OSSetArenaLo(__heap_end);
}

#elif defined(TARGET_XBOX)

//void* __heap_start;
//void* __heap_end;
void    sys_mem_Init(void)
{
}

#else
void    sys_mem_Init(void)
{
}
#endif


//============================================================================
s32 x_MemGetFree(void)
{
    s32 Free,Largest,Fragments;

    x_MemGetFree(Free,Largest,Fragments);
    return Free;
}

//============================================================================
s32 x_MemGetUsed(void)
{
#if defined(TARGET_PC)
    return 0;
#else
    mallinfo m = dlmallinfo(0);

    return m.uordblks;
#endif
}

#ifdef TARGET_PS2

#ifdef X_DEBUG
s32 AdditionalMemoryForDebug = 8*1048576;
#else
s32 AdditionalMemoryForDebug = (3*1024+512)*1024;
#endif  // X_DEBUG

#endif      // TARGET_PS2

//============================================================================
//**** FUNCTIONS REQUIRED FOR the Doug Lea Malloc code

//////////////////////////////////////////////////////////////////////////////
//
// FUNCTION     PS2_GetHeapMem().
// DESCRIPTION  the MORECORE function
//
//////////////////////////////////////////////////////////////////////////////

#if defined(TARGET_PS2) || defined(TARGET_GCN)
extern void* __heap_start;
extern void* __heap_end;

void* sys_mem_sbrk(int pool, int iSize)
{
    static char* pcEndOfMem = 0;
    (void)pool;

    if (0 == pcEndOfMem)
    {
        u32 HeapStart =  (u32)__heap_start;
        u32 HeapEnd = (u32)__heap_end;

        pcEndOfMem = (char*) (HeapEnd & ~4095 );

        if (iSize > 0)
        {
            // align prog end to page size (4096)
            unsigned int uHeapStart = HeapStart;
            uHeapStart = (uHeapStart + 4095) & ~4095;
            return (void*)uHeapStart;
        }    
    }

    if (iSize == 0)
    {
        // return end of mem
        return pcEndOfMem;
    }
    else
    {
        return (void*)-1;
    }
}
// End function PS2_GetHeapMem().

void sys_mem_oom(void)
{
}
#endif


#if defined(TARGET_XBOX)

#define XBOX_MEM_SIZE u32(1048576.0f*22.75f+0.5f)

void* __heap_start=NULL;
void* __heap_end  =NULL;



#if defined(X_MEM_DEBUG)

static byte* DebugMainHeapStart = NULL;
static byte* DebugMainHeapEnd   = NULL;

void SetLoggerBounds()
{
    LOG_MALLOC( DebugMainHeapStart, 1, "FALSE--TELLING LOGGER OUR LOW ADDRESS", 0 );
    LOG_FREE( DebugMainHeapStart, "FALSE--TELLING LOGGER OUR LOW ADDRESS", 0 );

    LOG_MALLOC( DebugMainHeapEnd - 1, 1, "FALSE--TELLING LOGGER OUR HIGH ADDRESS", 0 );
    LOG_FREE( DebugMainHeapEnd + XBOX_MEM_SIZE - 1, "FALSE--TELLING LOGGER OUR HIGH ADDRESS", 0 );
}
#endif //X_MEM_DEBUG


void* sys_mem_sbrk(int pool, int iSize)
{
    static char* pcEndOfMem = 0;
    (void)pool;

    ASSERT( !pool );

    static byte* s_MainHeapStart = NULL;
    static byte* s_MainHeapEnd   = NULL;

    if( !s_MainHeapStart )
    {
        s_MainHeapStart=( byte* )GlobalAlloc(GMEM_FIXED,XBOX_MEM_SIZE);

        if( !s_MainHeapStart )
        {
            __asm int 3 // cannot use assert
        }

        s_MainHeapEnd = s_MainHeapStart + XBOX_MEM_SIZE;
#if defined(X_MEM_DEBUG)
        DebugMainHeapStart = s_MainHeapStart;
        DebugMainHeapEnd = s_MainHeapEnd;
#endif

        ASSERTS(s_MainHeapStart,"Unable to allocate heap 1 memory");
        if (iSize > 0)
        {
            // align prog end to page size (4096)
            unsigned int uHeapStart = (unsigned int)s_MainHeapStart;
            uHeapStart= (uHeapStart + 4095) & ~4095;
            return (void*)uHeapStart;
        }    
    }

    if (iSize == 0)
    {
        // return end of mem
        return s_MainHeapEnd;
    }
    else
    {
        return (void*)-1;
    }
}

void sys_mem_oom(void)
{
    mallinfo m0 = dlmallinfo(0);
    mallinfo m1 = dlmallinfo(1);

}

#endif
