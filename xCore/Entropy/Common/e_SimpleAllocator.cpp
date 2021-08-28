#include "x_types.hpp"
#include "x_files.hpp"
#include "e_simpleallocator.hpp"

#define CAPACITY_GROW_SIZE 512

//==============================================================================
// Heap management control functions. This class is a generic heap management
// class. Each heap can be anywhere in any memory space. All this class deals with
// is a base and a length for the heap. Everything else is assumed to be dealt
// with by the owner application.
//==============================================================================
simple_allocator::simple_allocator(void)
{
}

//==============================================================================
simple_allocator::~simple_allocator(void)
{
	ASSERT(!m_Initialized);
}

//==============================================================================
void simple_allocator::Init(void* base,s32 size)
{
    m_TotalSize             = size;
    m_pBase                 = base;

    m_pMemoryBlock          = NULL;
    m_nMemoryBlocks         = 0;
    m_nMemoryBlocksAllocated= 0;

    m_MemFree               = m_TotalSize;
    m_MemUsed               = 0;
    m_AllocCount            = 0;

	m_Initialized           = TRUE;

    m_FreeCount             = 1;
    m_RootBlock.m_Status    = AF_AVAILABLE;
    m_RootBlock.m_Length    = m_TotalSize;
    m_RootBlock.m_Address   = m_pBase;
    m_RootBlock.m_pNext     = NULL;

}

//==============================================================================
void simple_allocator::Kill(void)
{
	ASSERT(m_Initialized);
	m_Initialized = FALSE;
    x_free(m_pMemoryBlock);
    m_pMemoryBlock = NULL;
    m_nMemoryBlocks = 0;
    m_nMemoryBlocksAllocated = 0;
}

//==============================================================================

void simple_allocator::GrowCapacity( void )
{
    s32 i;
    // Convert current next ptrs to indices
    for( i=0; i<m_nMemoryBlocks; i++ )
    {
        if( m_pMemoryBlock[i].m_pNext )
            m_pMemoryBlock[i].m_pNext = (alloc_node*)((u32)m_pMemoryBlock[i].m_pNext-(u32)m_pMemoryBlock);
        else 
            m_pMemoryBlock[i].m_pNext = (alloc_node*)0xFFFFFFFF;
    }
    m_RootBlock.m_pNext = (m_RootBlock.m_pNext)?((alloc_node*)((u32)m_RootBlock.m_pNext-(u32)m_pMemoryBlock)):((alloc_node*)0xFFFFFFFF);

    m_nMemoryBlocksAllocated += CAPACITY_GROW_SIZE;
    m_pMemoryBlock = (alloc_node*)x_realloc(m_pMemoryBlock,sizeof(alloc_node)*m_nMemoryBlocksAllocated);
    ASSERT( m_pMemoryBlock );
    //x_memset(m_pMemoryBlock+m_nMemoryBlocks,0xFFFFFFFF, sizeof(alloc_node)*(m_nMemoryBlocksAllocated-m_nMemoryBlocks));

    // Convert next ptrs back into ptrs
    for( i=0; i<m_nMemoryBlocks; i++ )
    {
        if( m_pMemoryBlock[i].m_pNext == (alloc_node*)0xFFFFFFFF)
            m_pMemoryBlock[i].m_pNext = NULL;
        else 
            m_pMemoryBlock[i].m_pNext = (alloc_node*)((u32)m_pMemoryBlock+(u32)m_pMemoryBlock[i].m_pNext);
    }
    m_RootBlock.m_pNext = ((u32)m_RootBlock.m_pNext==0xFFFFFFFF) ? (NULL) : ((alloc_node*)((u32)m_pMemoryBlock+(u32)m_RootBlock.m_pNext));

    // Clear new blocks
    for (i=m_nMemoryBlocks;i<m_nMemoryBlocksAllocated;i++)
    {
        m_pMemoryBlock[i].m_Status = AF_FREE;
        m_pMemoryBlock[i].m_pNext  = NULL;
        m_pMemoryBlock[i].m_Address = 0;
        m_pMemoryBlock[i].m_Length = 0;
    }

    m_LastBlockUsed = m_nMemoryBlocks;
    m_nMemoryBlocks = m_nMemoryBlocksAllocated;
}

//==============================================================================

void* simple_allocator::Alloc(s32 size)
{
    // Check if doing a zero-size allocation
    if( size == 0 )
        return (void*)0xFFFFFFFF;

    // Be sure enough headers have been allocated.  We are doing it here so systems
    // can initialize the simple allocator before malloc is available.
    if( (m_AllocCount+m_FreeCount+1) >= m_nMemoryBlocksAllocated )
        GrowCapacity();

    // Go through the heap and find the smallest block that
    // will fit what we're looking for
    alloc_node* pHeader;
    alloc_node* pBlockToUse;
    s32         LastSize,i;

    //
    // This can get very slow
    //
    size = (size+3)&~3;
    ASSERT(m_Initialized);
    pHeader = &m_RootBlock;
    LastSize = 1 << 30;
    pBlockToUse = NULL;
    while (pHeader)
    {
        if (pHeader->m_Status == AF_AVAILABLE)
        {
            if ( (pHeader->m_Length >= size) &&
                 (pHeader->m_Length < LastSize) )
            {
                LastSize = pHeader->m_Length;
                pBlockToUse = pHeader;
            }
        }
        pHeader = pHeader->m_pNext;
    }

    // No available blocks to fit the size we're looking for, just
    // fail!
    if (!pBlockToUse)
        return 0;

    ASSERT( (pBlockToUse==&m_RootBlock) || ((pBlockToUse>=m_pMemoryBlock) && (pBlockToUse<(m_pMemoryBlock+m_nMemoryBlocksAllocated))));

    pBlockToUse->m_Status = AF_ALLOCATED;
    m_AllocCount ++;

    // If we didn't consume the entire block, we create a new header
    // which has the remainder of what is available within it
    if (pBlockToUse->m_Length != size)
    {
        // Now we grab any available header out of the header pool
        pHeader = NULL;
        for (i=0;i<m_nMemoryBlocks;i++)
        {
            if (m_pMemoryBlock[m_LastBlockUsed].m_Status == AF_FREE)
            {
                pHeader = &m_pMemoryBlock[m_LastBlockUsed];
                break;
            }
            m_LastBlockUsed++;
            if (m_LastBlockUsed >= m_nMemoryBlocks)
            {
                m_LastBlockUsed = 0;
            }
        }

        ASSERTS(pHeader,"Ran out of memory blocks for simple allocator");

        //
        // Allocate this block and add it in to the used list
        //

        m_FreeCount++;

        pHeader->m_Status       = AF_AVAILABLE;
        pHeader->m_Address      = (void*)((u32)pBlockToUse->m_Address + size);
        pHeader->m_Length       = pBlockToUse->m_Length - size;
        pHeader->m_pNext        = pBlockToUse->m_pNext;

        pBlockToUse->m_Length   = size;
        pBlockToUse->m_pNext    = pHeader;
    }
    m_MemFree -= pBlockToUse->m_Length;
    m_MemUsed += pBlockToUse->m_Length;

    return (pBlockToUse->m_Address);
}

//==============================================================================
xbool simple_allocator::IsValid(void* base)
{
    alloc_node* pHeader;
    pHeader = &m_RootBlock;
    while (pHeader)
    {
        if (pHeader->m_Address == base)
            break;
        pHeader = pHeader->m_pNext;
    }

	return ( pHeader && 
		    (pHeader->m_Status == AF_ALLOCATED) );
}

//==============================================================================
void simple_allocator::Validate(void)
{
	// In debug mode, this function will walk the memory allocation list and make
	// sure that it's state is consistent. Things that will be checked:
	// 1. No memory block overruns (only if memory is physical and accessable)
	// 2. Memory blocks have been coalesced where they could
	// 3. There are no holes
	// 4. All memory headers point to valid locations of "memory"
}

//==============================================================================
s32 simple_allocator::Free(void* base)
{
    // Check if freeing a zero-size allocation
    if( (u32)base == 0xFFFFFFFF )
        return 0;

    // Check if freeing a NULL ptr
    if( base == NULL )
        return 0;

    alloc_node* pHeader;
    alloc_node* pPrev;
    alloc_node* pNext;
    s32         Length;

	ASSERT(m_Initialized);
    // First, find the block that this memory is in
    pPrev = NULL;

    pHeader = &m_RootBlock;
    while (pHeader)
    {
        if (pHeader->m_Address == base)
            break;
        pPrev = pHeader;
        pHeader = pHeader->m_pNext;
    }
    ASSERTS(pHeader, "The memory block freed was not valid");
    ASSERTS(pHeader->m_Status == AF_ALLOCATED,"The memory block has already been freed");

    pHeader->m_Status = AF_AVAILABLE;
    pNext = pHeader->m_pNext;

    Length = pHeader->m_Length;

    if (pNext)
    {
        // Verify that we should, indeed, be coalescing properly
        ASSERT(pHeader->m_pNext == pNext);
        ASSERT(pHeader->m_Length + (s32)pHeader->m_Address == (s32)pNext->m_Address);

        // Can we coalesce this with the following memory block?
        if (pNext->m_Status == AF_AVAILABLE)
        {
            m_FreeCount--;

            pHeader->m_Length += pNext->m_Length;
            pHeader->m_pNext = pNext->m_pNext;
            pNext->m_Status = AF_FREE;
        }
    }

    if (pPrev)
    {
        // Verify that we should, indeed, be coalescing properly
        ASSERT(pPrev->m_pNext == pHeader);
        ASSERT(pPrev->m_Length + (s32)pPrev->m_Address == (s32)pHeader->m_Address);
        // Can we coalesce the previous memory block with the
        // current?
        if (pPrev->m_Status == AF_AVAILABLE)
        {
            m_FreeCount--;
            pPrev->m_Length += pHeader->m_Length;
            pPrev->m_pNext = pHeader->m_pNext;
            pHeader->m_Status = AF_FREE;
        }
    }

    m_AllocCount--;
    m_MemFree += Length;
    m_MemUsed -= Length;

    return Length;
}

//==============================================================================

void simple_allocator::DumpList(void)
{
    alloc_node* pHeader;

    static char* s_Status[]=
    {
        "FREE",
        "AVAILABLE",
        "ALLOCATED",
    };

    pHeader = &m_RootBlock;
    while (pHeader)
    {
        x_DebugMsg("START: 0x%08x  END: 0x%08x  LENGTH: %8d  STATUS: %s\n",
                    pHeader->m_Address,
                    (s32)pHeader->m_Address+pHeader->m_Length,
                    pHeader->m_Length,
                    s_Status[pHeader->m_Status]);
        pHeader = pHeader->m_pNext;
    }
}

//==============================================================================

void simple_allocator::SanityCheck(void)
{
    s32 i=0;
    s32 nUsedBlocks=0;
    s32 nUnusedBlocks=0;
    s32 nAllocated=0;
    s32 nFreeBlocks=0;

    if( m_RootBlock.m_Status == AF_FREE )
        nUnusedBlocks++;
    else
    if( m_RootBlock.m_Status == AF_AVAILABLE )
    {
        nFreeBlocks++;
        nUsedBlocks++;
    }
    else
    if( m_RootBlock.m_Status == AF_ALLOCATED )
    {
        nUsedBlocks++;
        nAllocated++;
    }

    for( i=0; i<m_nMemoryBlocksAllocated; i++ )
    {
        if( m_pMemoryBlock[i].m_Status == AF_FREE )
        {
            nUnusedBlocks++;
            continue;
        }

        if( m_pMemoryBlock[i].m_pNext )
        {
            ASSERT( (m_pMemoryBlock[i].m_pNext->m_Status==AF_AVAILABLE) || (m_pMemoryBlock[i].m_pNext->m_Status==AF_ALLOCATED));
            ASSERT( ((u32)m_pMemoryBlock[i].m_Address + m_pMemoryBlock[i].m_Length) == (u32)m_pMemoryBlock[i].m_pNext->m_Address );
        }

        if( m_pMemoryBlock[i].m_Status == AF_AVAILABLE )
        {
            nUsedBlocks++;
            nFreeBlocks++;
            if( m_pMemoryBlock[i].m_pNext )
            {
                ASSERT( m_pMemoryBlock[i].m_pNext->m_Status != AF_AVAILABLE );
            }
            continue;
        }

        if( m_pMemoryBlock[i].m_Status == AF_ALLOCATED )
        {
            nAllocated++;
            nUsedBlocks++;
            continue;
        }
    }

    ASSERT( x_abs((nUnusedBlocks + nUsedBlocks) - m_nMemoryBlocks) <= 1 );
    ASSERT( nAllocated == m_AllocCount );
}

//==============================================================================
/*
#define NUMBER_PTRS 8192
#define NUM_LOOPS (NUMBER_PTRS*32)
void*   PTR[NUMBER_PTRS];

void SimpleAllocatorStressTest( void )
{
    s32 i;
    

    simple_allocator Pool;
    void* Base = x_malloc(1048576);
    Pool.Init( Base, 1048576 );


    xtimer Timer;
    Timer.Reset();
    Timer.Start();
    for( i=0; i<NUMBER_PTRS; i++ )
        PTR[i] = Pool.Alloc(16);
    Timer.Stop();
    x_DebugMsg("TIME: %f\n",Timer.ReadMs());


    random Rand;
    Timer.Reset();
    Timer.Start();
    for( i=0; i<NUM_LOOPS; i++ )
    {
        s32 Index = Rand.irand(0,NUMBER_PTRS-1);

        if( PTR[Index] )
        {
            Pool.Free(PTR[Index]);
            PTR[Index] = NULL;
        }
        else
        {
            PTR[Index] = Pool.Alloc(16);
        }
    }
    Timer.Stop();
    x_DebugMsg("TIME: %f\n",Timer.ReadMs());


    Timer.Reset();
    Timer.Start();
    for( i=0; i<NUMBER_PTRS; i++ )
        Pool.Free( PTR[i] );
    Timer.Stop();
    x_DebugMsg("TIME: %f\n",Timer.ReadMs());


    Pool.Kill();
    x_free(Base);
}
*/
//==============================================================================
