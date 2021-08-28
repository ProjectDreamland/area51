//==============================================================================
//
//  x_memory_private.hpp
//
//==============================================================================

#ifndef X_MEMORY_PRIVATE_HPP
#define X_MEMORY_PRIVATE_HPP
#else
#error "File " __FILE__ " has been included twice!"
#endif

//==============================================================================
//  FUNCTIONS
//==============================================================================
        
void    x_MemInit       ( void );
void    x_MemKill       ( void );

//==============================================================================

#ifdef X_MEM_DEBUG

void*   x_debug_malloc  ( s32 NBytes, const char* pFileName, s32 Line );
#define x_malloc(S)     x_debug_malloc( (S), __FILE__, __LINE__ )

#endif

//==============================================================================
