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
    void*   x_debug_malloc  ( s32 NBytes, const char* pFileName, s32 Line, const char* pFunction);
    void*   x_debug_realloc ( void* pMemory, s32 NewNBytes, const char* pFileName, s32 Line );
    void    x_debug_free    ( void* pMemory, const char* pFileName, s32 Line );
#if defined(__GNUC__)
    #define x_malloc(S)     x_debug_malloc ( (S), __FILE__, __LINE__, __PRETTY_FUNCTION__ )
    #define x_realloc(A,S)  x_debug_realloc( (A), (S), __FILE__, __LINE__ )
    #define x_free(A)       x_debug_free   ( (A), __FILE__, __LINE__ )
#else
    #define x_malloc(S)     x_debug_malloc ( (S), __FILE__, __LINE__, NULL )
    #define x_realloc(A,S)  x_debug_realloc( (A), (S), __FILE__, __LINE__ )
    #define x_free(A)       x_debug_free   ( (A), __FILE__, __LINE__ )
#endif
    #endif

//==============================================================================
//  SYSTEM SPECIFIC CALLS TO PERFORM THE ALLOC/DELETE
//==============================================================================

void* sys_mem_realloc( void* pBlock, u32 nBytes );
void* sys_mem_malloc ( u32 nBytes );
void  sys_mem_free   ( void* pBlock );
void  sys_mem_Init   ( void );

#define ZERO_SIZE_PTR   (-1)
//==============================================================================
