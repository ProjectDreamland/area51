//==============================================================================
//
//  SMemMatrixCache.hpp
//
//  A simple utility class that can will allocate matrices using smem so that
//  they can be referenced by the main display list (since smem is double buffered).
//
//  Allocations will only re-occur within the same frame if a page flip has happened
//  or the current allocation is too small.
//
//  There is also support for flagging the data as dirty.
//  Eg. An animation player may use the flag to decide when to re-compute matrices   
//
//==============================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "SMemMatrixCache.hpp"


//=========================================================================
// PUBLIC FUNCTIONS
//=========================================================================

smem_matrix_cache::smem_matrix_cache()
{
    // Clear cache
    m_bDirty    = FALSE ;   // TRUE if data is valid
    m_ID        =  -1 ;     // Allocation ID
    m_nMatrices = 0 ;       // Current number of matrices allocated
    m_pMatrices = NULL ;    // Allocated matrices
}

//=========================================================================

// Sets dirty status
void smem_matrix_cache::SetDirty( xbool bStatus )
{
    // Keep status
    m_bDirty = bStatus ;

    // If setting status to valid, then the allocation must be valid!
    if (m_bDirty == FALSE)
    {
        ASSERTS((IsAllocValid() == TRUE), "Smem toggle occured during a main thread function - another thread (frontend) must still be running!") ;
    }
}

//=========================================================================

// Returns dirty status
xbool smem_matrix_cache::IsDirty( void ) const
{
    // Allocation invalid?
    if (IsAllocValid() == FALSE)
        return TRUE ;

    // Is data dirty?
    return (m_bDirty) ;
}

//=========================================================================

// Returns TRUE if matrix data allocated and not dirty
xbool smem_matrix_cache::IsValid( s32 nMatrices ) const
{
    // Is data dirty?
    if (m_bDirty == TRUE)
        return FALSE ;

    // Is allocation bad?
    if (IsAllocValid(nMatrices) == FALSE)
        return FALSE ;
    
    // All is good
    return TRUE ;
}

//=========================================================================

// Returns current matrix allocation (doesn't care if it's dirty of not)
matrix4* smem_matrix_cache::GetMatrices( s32 nMatrices )
{
    // Is current allocation okay?
    if (IsAllocValid(nMatrices))
        return m_pMatrices ;

    // Allocate new cache and flag data is dirty
    m_ID        = smem_GetActiveID() ;
    ASSERT( nMatrices > 0 );
    m_nMatrices = nMatrices ;
    m_pMatrices = (matrix4*)smem_BufferAlloc(nMatrices * sizeof(matrix4)) ;
    ASSERT( m_pMatrices );
    m_bDirty    = TRUE ;

    #ifdef TARGET_PS2
    // SMEM allocations are uncached as a safety precaution for MFIFO.
    // Since we know characters are a special case and there will be
    // a FlushCache before any deferred rendering happens, we can go
    // ahead and make them cached for better performance.
    m_pMatrices = (matrix4*)((u32)m_pMatrices & 0x0fffffff );
    #endif

    return m_pMatrices ;
}

//=========================================================================
// PRIVATE FUNCTIONS
//=========================================================================

// Returns TRUE if matrix allocation is valid
xbool smem_matrix_cache::IsAllocValid( s32 nMatrices ) const
{
    // Allocation must be present
    if (m_pMatrices == NULL)
        return FALSE ;

    // Allocation must be big enough
    if (m_nMatrices < nMatrices)
        return FALSE ;

    // Allocation must be on the correct smem frame
    if (m_ID != smem_GetActiveID())
        return FALSE ;

    // All is good
    return TRUE ;
}

//=========================================================================
