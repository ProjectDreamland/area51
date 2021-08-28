//==============================================================================
//
//  x_memory.hpp
//
//==============================================================================

#ifndef X_MEMORY_HPP
#define X_MEMORY_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_PLUS_HPP
#include "x_plus.hpp"
#endif

#ifndef X_DEBUG_HPP
#include "x_debug.hpp"
#endif

//==============================================================================
//  FUNCTIONS
//==============================================================================

void*   x_malloc            ( s32   NBytes  );
void*	x_malloctop			( s32   NBytes  );
void    x_free              ( void* pMemory );
void*   x_realloc           ( void* pMemory, s32 NewNBytes );
s32     x_msize             ( void* pMemory ) ;

void    x_MemDump           ( void );
void    x_MemDump           ( const char* pFileName );

void    x_MemAddMark        ( const char* pComment );
void    x_MemClearMarks     ( void );

void    x_MemSanity         ( void );

/*
void    x_MemDonateBlock    ( byte* pBlock, s32 NBytes );
*/

//==============================================================================
//  PRIVATE
//==============================================================================

#include "Implementation/x_memory_private.hpp"

//==============================================================================
#endif // X_MEMORY_HPP
//==============================================================================
