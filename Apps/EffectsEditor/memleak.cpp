#include <stdafx.h>

#include "memleak.h"



CMemDebug   __MemDebug;
FILE*       LogFile;

//============================================================================
// AllocHook prototype
int __cdecl AllocHook(
                        int      nAllocType,
                        void   * pvData,
                        size_t   nSize,
                        int      nBlockUse,
                        long     lRequest,
                        const unsigned char * szFileName,
                        int      nLine
                     );



//============================================================================
// Install the hook
CMemDebug::CMemDebug()
{
#ifdef __DEBUG_LEAKS__

    LogFile = fopen( "leaks.txt", "wt" );
    
    if ( LogFile )
        _CrtSetAllocHook( AllocHook );

#endif 
}


//============================================================================
// Install the hook
CMemDebug::~CMemDebug()
{
#ifdef __DEBUG_LEAKS__

    if ( LogFile )
        fclose( LogFile );
#endif
}


//============================================================================
// The Hook
int __cdecl AllocHook(
                        int      nAllocType,
                        void   * pvData,
                        size_t   nSize,
                        int      nBlockUse,
                        long     lRequest,
                        const unsigned char * szFileName,
                        int      nLine
                     )
{
#ifdef __DEBUG_LEAKS__

    char *operation[] = { "", "allocating", "re-allocating", "freeing" };
    char *blockType[] = { "Free", "Normal", "CRT", "Ignore", "Client" };

    if ( nBlockUse == _CRT_BLOCK )   // Ignore internal C runtime library allocations
        return( TRUE );

    if ( szFileName )
    {
        if ( strcmpi( (char*)szFileName, "plex.cpp" ) == 0 )
            return TRUE;
        if ( strcmpi( (char*)szFileName, "strcore.cpp" ) == 0 )
            return TRUE;
        if ( strcmpi( (char*)szFileName, "map_pp.cpp" ) == 0 )
            return TRUE;
    }

    if ( lRequest > 833 )
        nSize = 0;

    _ASSERT( ( nAllocType > 0 ) && ( nAllocType < 4 ) );
    _ASSERT( ( nBlockUse >= 0 ) && ( nBlockUse < 5 ) );

    fprintf(    LogFile, 
                "Memory operation in %s, line %d: %s a %d-byte '%s' block (# %ld)\n",
                szFileName, nLine, operation[nAllocType], nSize, 
                blockType[nBlockUse], lRequest );
    
    if ( pvData != NULL )
        fprintf( LogFile, " at %X", pvData );

    fflush(LogFile);

#endif // __DEBUG_LEAKS__
    return( TRUE );         // Allow the memory operation to proceed

}

