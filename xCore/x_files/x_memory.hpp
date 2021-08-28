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
void    x_free              ( void* pMemory );
void*   x_realloc           ( void* pMemory, s32 NewNBytes );

void    x_MemDump           ( void );
void    x_MemDump           ( const char* pFileName, xbool bCommaSeperated );

void    x_MemAddMark        ( const char* pComment );
void    x_MemClearMarks     ( void );

void    x_MemSanity         ( void );
/*
void    x_MemDonateBlock    ( byte* pBlock, s32 NBytes );
*/
void    x_MemGetFree        ( s32& Free, s32& Largest, s32& Fragments);
s32     x_MemGetFree        ( void );
s32     x_MemGetUsed        ( void );
s32     x_MemGetPtrSize     ( void* pMemory );

#if !defined(X_RETAIL) && defined(USE_OWNER_STACK)
    void    x_MemPushOwner      ( const char* pOwnerName );
    void    x_MemPopOwner       ( void );
#else
    #if defined(VENDOR_SN) && !defined(_MSC_VER)
        // This is the way to make the calls disappear with GNU
        #define x_MemPushOwner(...) ((void)0)
        #define x_MemPopOwner(...)  ((void)0)
    #else 
        inline void x_MemPushOwner(...) {};
        inline void x_MemPopOwner(...) {};
    #endif // defined(VENDOR_SN) && !defined(_MSC_VER)
#endif // !defined(X_RETAIL)

class xmemowner
{
public:
inline  xmemowner   ( const char* pName ) { (void)pName; x_MemPushOwner( pName ); }
inline ~xmemowner   ( void )              { x_MemPopOwner(); }
};

#if defined(USE_OWNER_STACK)
#define MEMORY_OWNER( a ) xmemowner __owner__(a)
#else
#define MEMORY_OWNER( a )
#endif // defined(USE_OWNER_STACK)

//#define MEMORY_OWNER_DETAIL xmemowner __owner__(a)
#define MEMORY_OWNER_DETAIL( a )

#if defined(USE_OWNER_STACK)
void x_MemQueryExcludeAll( void );
void x_MemQueryIncludeAll( void );
void x_MemQueryInclude( const char* pString );
void x_MemQueryRequire( const char* pString );
void x_MemQueryExclude( const char *pString );
s32  x_MemQuery( xbool bIncludeAll = FALSE );
#endif // USE_OWNER_STACK

//==============================================================================
//  FUNCTIONS

//==============================================================================
//  PRIVATE
//==============================================================================

#include "Implementation/x_memory_private.hpp"

//==============================================================================
#endif // X_MEMORY_HPP
//==============================================================================
