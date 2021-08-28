#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP
//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_stdio.hpp"

#define PRELOAD_FILE( filename )      filename
#define PRELOAD_PS2_FILE( filename )  filename
#define PRELOAD_XBOX_FILE( filename ) filename
#define PRELOAD_MP_FILE( filename )   filename

#if defined(X_DEBUG)
// *** IF YOU NEED THIS DEFINED FOR DEBUGGING, DO NOT CHECK IT IN THAT WAY
//#define RSC_MGR_COLLECT_STATS
#endif  // X_RETAIL

//==============================================================================
//  TYPES
//==============================================================================

class rsc_loader;
class rsc_mgr;

//==============================================================================
//==============================================================================
//==============================================================================
// RHANDLE
//==============================================================================
//==============================================================================
//==============================================================================
class rhandle_base
{
public:
                    rhandle_base    ( void );
                    rhandle_base    ( const char* pResourceName );
                   ~rhandle_base    ( void );

    void            SetName         ( const char* pResourceName );
    const char*     GetName         ( void ) const;

    void*           GetPointer      ( void ) const;
    xbool           IsLoaded        ( void ) const;

    s16             GetIndex        ( void ) const;

    void            Destroy         ( void );
    xbool           IsNull          ( void ) const { return m_Data == -1; }

protected:

    void            SetIndex        ( s16 I );

    s32             m_Data;

    friend rsc_mgr;
};

//==============================================================================

template< class T >
struct rhandle : public rhandle_base
{
    T*  GetPointer  ( void ) const;
};

//==============================================================================
//==============================================================================
//==============================================================================
// RSC_MGR
//==============================================================================
//==============================================================================
//==============================================================================
class rsc_mgr
{

//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------

public:

                    rsc_mgr         ( void );
                   ~rsc_mgr         ( void );

        void        Init            ( void );
        void        Kill            ( void );

#ifdef RSC_MGR_COLLECT_STATS
        void        SanityCheck     ( void );
        void        DumpStats       ( void );
        void        DumpStatsToFile ( const char* pFileName );
#endif // RSC_MGR_COLLECT_STATS

        void        SetRootDirectory( const char* pRootDir, s32 i=0 );
        void        LoadDFS         ( const char* pDFSFile );
        const char* GetRootDirectory( s32 i=0 ) const;
        void        SetOnDemandLoading( xbool OnDemand );

        const char* FixupFilename   ( const char* Filename );

        void        Load            ( const char* pResourceName );
        void        Unload          ( const char* pResourceName );
        void        Refresh         ( const char* pResourceName );
        void        RefreshAll      ( void );
        void        UnloadAll       ( xbool bIgnoreTagged = FALSE );
        void        TagResources    ( void );

#ifdef RSC_MGR_COLLECT_STATS
        struct
        {
            s32         numFailedLoadResources;
            s32         numLoadedResources;
            s32         numLoadedPerType[16];
        } Stats;

        void        PrintStats(void);
#endif // RSC_MGR_COLLECT_STATS

//------------------------------------------------------------------------------
//  Private Functions
//------------------------------------------------------------------------------
private:
        void        Clear           ( void );
        s16         ComputeHashIndex( const char* pStr );
        s16         AllocEntry      ( const char* pStr );
        void        FreeEntry       ( s16 I );
        s16         FindEntry       ( const char* pResourceName );

        void        AddRHandle      ( rhandle_base& RHandle, const char* pResourceName );
        const char* GetRHandleName  ( const rhandle_base& RHandle ) const;
        void*       GetPointer      ( const rhandle_base& RHandle );
        void*       GetPointerSlow  ( const rhandle_base& RHandle );
        xbool       IsRHandleLoaded ( const rhandle_base& RHandle );

//------------------------------------------------------------------------------
//  Private Data
//------------------------------------------------------------------------------
private:

    #define RESOURCE_NAME_SIZE  128

    enum state
    {
        NOT_USED,
        NOT_LOADED,
        LOADED,
        FAILED_LOAD,
    };

    struct resource
    {
        char        Name[RESOURCE_NAME_SIZE];
        state       State;
        void*       pData;
        rsc_loader* pLoader;
        s16         bTagged;
        s16         Next;       // in the used or free list
        s16         Prev;       // in the used or free list
        s16         HashIndex;  // index back into the hash table
        s16         NextHash;   // in the list for hashing collisions
#ifdef RSC_MGR_COLLECT_STATS
        s32         FileSize;
        f32         LoadTime;
#endif
    };

    enum { HASH_SIZE = 1327 };  // keep this a prime number for best results (use prime-numbers.org)
    s16             m_HashTable[HASH_SIZE];

    s16             m_FirstUsed;
    s16             m_FirstFree;

    resource*       m_pResource;
    s32             m_nResources;
    s32             m_nResourcesAllocated;

    char            m_RootDir[256*4];

    xbool           m_OnDemandLoading;


    static rsc_loader*     m_pLoader;
    static s32      m_NumLoaders;
    friend rsc_loader;
    friend rhandle_base;
};

//==============================================================================
// RSC_LOADER
//==============================================================================
//
// rsc_loader - User must call this constructor 
//
// PreLoad  -   Use this function to get raw data from the file to the system.
//              Note that this will be sone in a different tread.
//
// Resolve  -   This is call to convert from the raw data into game ready data.
//              This is call by the main tread so it should be okay to call 
//              Game system calls.
//
// Unload   -   This function is call by the main tread to release the pdata.
//              Make sure you cast pData to the right type of the destructors 
//              to be call.
//
//==============================================================================
class rsc_loader
{
//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------

virtual void*   PreLoad         ( X_FILE* pFP   ) = 0;
virtual void*   Resolve         ( void*   pData ) = 0;
virtual void    Unload          ( void*   pData ) = 0;
virtual void*   PreLoad         ( X_FILE*& Fp, const char* pFileName );

//------------------------------------------------------------------------------
// For the user to call
//------------------------------------------------------------------------------
protected:
                rsc_loader      ( const char* pType, const char* pExt );

#ifdef TARGET_XBOX
    char* m_pFileName;
#endif

//------------------------------------------------------------------------------
//  Private Data
//------------------------------------------------------------------------------
private:

    const char*     m_pType;
    const char*     m_pExt;
    rsc_loader*     m_pNext;

    friend rsc_mgr;
};

//==============================================================================
// GLOBALS
//==============================================================================

extern rsc_mgr g_RscMgr;

//==============================================================================
// INLINE
//==============================================================================
#ifndef INLINE_RESOURCE_MANAGER_HPP
#include "inline_ResourceMgr.hpp"
#endif

//==============================================================================
// END
//==============================================================================
#endif 

