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

#ifndef __SMEM_MATRIX_CACHE_HPP__
#define __SMEM_MATRIX_CACHE_HPP__

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"


//=========================================================================
// SMEM_CACHE
//=========================================================================

// A simple scratch memory cache class
class smem_matrix_cache
{
// Public functions
public:
                    // Constructor
                    smem_matrix_cache() ;

        // Sets dirty status
        void        SetDirty        ( xbool bStatus ) ;
        
        // Returns dirty status
        xbool       IsDirty         ( void ) const ;
        
        // Returns TRUE if matrix data allocated and not dirty
        xbool       IsValid         ( s32 nMatrices ) const ;
        
        // Returns current matrix allocation (doesn't care if it's dirty of not)
        matrix4*    GetMatrices     ( s32 nMatrices = -1 ) ;

// Private functions
private:
        // Returns TRUE if matrix allocation is valid
        xbool       IsAllocValid    ( s32 nMatrices = -1 ) const ;

// Private data
private:
        xbool       m_bDirty ;      // TRUE if matrices are valid
        s32         m_ID ;          // Allocation ID
        s32         m_nMatrices ;   // Current number of matrices allocated
        matrix4*    m_pMatrices ;   // Allocated matrices
} ;

//=========================================================================

#endif  //#ifndef __SMEM_MATRIX_CACHE_HPP__
