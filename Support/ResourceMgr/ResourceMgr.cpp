//==============================================================================
//
//  ResourceMgr.cpp
//
//==============================================================================


//==============================================================================
//  INCLUDES
//==============================================================================

#include "ResourceMgr.hpp"
#include "x_string.hpp"
#include "x_log.hpp"
#include "x_time.hpp"
#include "x_context.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

#if defined(X_LOGGING)
#define LOG_RESOURCES
#endif // defined(X_LOGGING)

//==============================================================================
//  TYPES
//==============================================================================

rsc_mgr g_RscMgr;

//==============================================================================
//  STORAGE
//==============================================================================

rsc_loader*   rsc_mgr::m_pLoader = NULL;
s32           rsc_mgr::m_NumLoaders = 0;

static char     FixupBuffer[ 1024 ];


//==============================================================================
//==============================================================================
//==============================================================================
// RESOURCE LOADER
//==============================================================================
//==============================================================================
//==============================================================================
void* rsc_loader::PreLoad( X_FILE*& Fp, const char* pFileName )
{
    MEMORY_OWNER( "PRELOAD" );

#ifdef TARGET_XBOX
    // We need to track the resource name for validation and sanity
    extern void xbox_SetAllocationName( const char* );
    xbox_SetAllocationName( pFileName );
    m_pFileName = (char*)pFileName;
#endif

    ASSERT( pFileName );
    Fp = x_fopen( pFileName, "rb" );
    if( Fp == NULL ) return NULL;
    return PreLoad( Fp );
}

//==============================================================================
//==============================================================================
//==============================================================================
//  RSC_MGR FUNCTIONS
//==============================================================================
//==============================================================================
//==============================================================================

rsc_mgr::rsc_mgr( void )
{
    m_pResource = NULL;
    Clear();
}

//==============================================================================

rsc_mgr::~rsc_mgr( void )
{
    // Assert that there are no resources left
    Clear();
}

//==============================================================================

void rsc_mgr::Kill( void )
{
    Clear();
}

//==============================================================================

void rsc_mgr::Init( void )
{
    Clear();
}

//==============================================================================

void rsc_mgr::Clear( void )
{
    x_free( m_pResource );
    m_pResource = NULL;
    m_nResources = 0;
    m_nResourcesAllocated = 0;
    m_FirstUsed = -1;
    m_FirstFree = -1;

    x_memset(m_HashTable,-1,sizeof(m_HashTable));
    x_memset(m_RootDir,0,sizeof(m_RootDir));
#ifdef RSC_MGR_COLLECT_STATS
    x_memset((void *)&Stats, 0, sizeof(Stats));
#endif // RSC_MGR_COLLECT_STATS
    m_OnDemandLoading = FALSE;
}

//==============================================================================

#ifdef RSC_MGR_COLLECT_STATS
void rsc_mgr::PrintStats(void)
{
    x_DebugMsg("\n RESOURCE MANAGER STATS                                        \n");
    x_DebugMsg("===============================================================\n");
    x_DebugMsg("%25s     %9d\n", "numLoadedResources", Stats.numLoadedResources);
    
    x_DebugMsg("\n%25s%14s%14s\n", "STATS PER TYPE", "LOADED", "LOCKED");
    x_DebugMsg("          ====================================================\n");
    rsc_loader* pLoader = m_pLoader;
    for(s32 i = 0; i < m_NumLoaders;i++)
    {
        x_DebugMsg("%25s%14d\n", pLoader->m_pExt, Stats.numLoadedPerType[i]);
        pLoader = pLoader->m_pNext;
    }
}
#endif // RSC_MGR_COLLECT_STATS

//==============================================================================

const char* rsc_mgr::GetRootDirectory( s32 i ) const
{
    ASSERT( (i>=0) && (i<4) );
    return &m_RootDir[i*256];
}
//==============================================================================

void rsc_mgr::SetRootDirectory( const char* pRootDir, s32 i )
{
    ASSERT( (i>=0) && (i<4) );
    x_strcpy( m_RootDir+i*256, pRootDir );
}

//==============================================================================

void rsc_mgr::LoadDFS( const char* pRootDir )
{
    (void)pRootDir;
}

//==============================================================================

#ifdef RSC_MGR_COLLECT_STATS
void rsc_mgr::SanityCheck( void )
{
    // Be sure there are not duplicate loaders for any extension
    // Be sure all free and used are accounted for
    // All used should have pData != NULL
    // All free should have pData == NULL
}
#endif // RSC_MGR_COLLECT_STATS

//==============================================================================

#ifdef RSC_MGR_COLLECT_STATS
void rsc_mgr::DumpStats( void )
{
#if !defined(bwatson) && !(defined bhapgood)
    x_DebugMsg("***********************************\n");
    x_DebugMsg("          RESOURCE MANAGER\n");
    x_DebugMsg("***********************************\n");

    // Dump all the loaders we have
    {
        x_DebugMsg("LOADERS AVAILABLE:\n");

        rsc_loader* pLoader = m_pLoader;
        s32 Count=0;
        while( pLoader )
        {
            x_DebugMsg("%2d] %16s %16s\n",Count,pLoader->m_pExt,pLoader->m_pType);
            pLoader = pLoader->m_pNext;
        }
    }

    // Dump resource references
    {
        x_DebugMsg("RESOURCES\n");

        s32 I = m_FirstUsed;
        while( I!=-1 )
        {
            switch( m_pResource[I].State )
            {
            case LOADED:        x_DebugMsg("(LOADED)      "); break;
            case NOT_LOADED:    x_DebugMsg("(NOT LOADED)  "); break;
            case FAILED_LOAD:   x_DebugMsg("(FAILED LOAD) "); break;
            }

            x_DebugMsg("%c NM:%s\n",
                m_pResource[I].bTagged ? ('T'):('_'),
                m_pResource[I].Name);

            I = m_pResource[I].Next;
        }
    }

#endif
}
#endif // RSC_MGR_COLLECT_STATS

//==============================================================================

#ifdef RSC_MGR_COLLECT_STATS
void rsc_mgr::DumpStatsToFile( const char* pFileName )
{
    // Count resources
    s32 nResources = 0;
    {
        s32 I = m_FirstUsed;
        while( I!=-1 )
        {
            if( m_pResource[I].pLoader )
                nResources++;
            I = m_pResource[I].Next;
        }
    }

    // Allocate structures
    struct rsc_struct
    {
        resource*   pR;
    };
    rsc_struct* pResc = (rsc_struct*)x_malloc(sizeof(rsc_struct)*nResources);
    if( !pResc )
        return;

    // Fill out structures
    {
        s32 C=0;
        s32 I = m_FirstUsed;
        while( I!=-1 )
        {
            if( m_pResource[I].pLoader )
            {
                pResc[C].pR = &m_pResource[I];
                C++;
            }
            I = m_pResource[I].Next;
        }
        ASSERT( C == nResources );
    }

    // Sort the resources by loader extension
    {
        s32 A,B;
        for( A=0; A<nResources; A++ )
        {
            s32 iBest = A;

            for( B=A+1; B<nResources; B++ )
            {
                if( x_stricmp(pResc[B].pR->pLoader->m_pExt,pResc[iBest].pR->pLoader->m_pExt) < 0 )
                {
                    iBest = B;
                }
                else
                if( x_stricmp(pResc[B].pR->pLoader->m_pExt,pResc[iBest].pR->pLoader->m_pExt) == 0 )
                {
                    if( pResc[B].pR->FileSize > pResc[iBest].pR->FileSize )
                        iBest = B;
                    else
                    if( (pResc[B].pR->FileSize == pResc[iBest].pR->FileSize) &&
                        (x_stricmp(pResc[B].pR->Name,pResc[iBest].pR->Name) < 0) )
                        iBest = B;
                }

            }

            resource* pT = pResc[A].pR;
            pResc[A].pR = pResc[iBest].pR;
            pResc[iBest].pR = pT;
        }
    }

    X_FILE* fp = x_fopen(pFileName,"wt");
    if( !fp )
        return;

    // Do summary
    {
        x_fprintf(fp,"Summary\n");
        rsc_loader* pLoader = NULL;
        s32 FileSizeTotal=0;
        f32 LoadTimeTotal=0;
        s32 Count=0;
        for( s32 i=0; i<nResources; i++ )
        {
            if( pResc[i].pR->pLoader != pLoader )
            {
                if( pLoader != NULL )
                {
                    x_fprintf(fp,"%d,%d,%8.3f,%s\n",Count,FileSizeTotal,LoadTimeTotal,pLoader->m_pExt);
                    pLoader = pResc[i].pR->pLoader;
                    FileSizeTotal = 0;
                    LoadTimeTotal = 0;
                    Count = 0;
                }
                pLoader = pResc[i].pR->pLoader;
            }
            FileSizeTotal += pResc[i].pR->FileSize;
            LoadTimeTotal += pResc[i].pR->LoadTime;
            Count++;
        }
        if( pLoader != NULL )
        {
            x_fprintf(fp,"%d,%d,%8.3f,%s\n",Count,FileSizeTotal,LoadTimeTotal,pLoader->m_pExt);
        }

        x_fflush(fp);
    }

    // Dump resource references
    {
        rsc_loader* pLoader = NULL;
        for( s32 i=0; i<nResources; i++ )
        {
            if( pResc[i].pR->pLoader != pLoader )
            {
                pLoader = pResc[i].pR->pLoader;
                x_fprintf(fp,"\n%s\n",pLoader->m_pExt);
            }
            x_fprintf(fp,"%d, %s\n",pResc[i].pR->FileSize,pResc[i].pR->Name);
            x_fflush(fp);
        }
    }

    x_fclose(fp);

    x_free(pResc);
}
#endif // RSC_MGR_COLLECT_STATS

//==============================================================================

s16 rsc_mgr::ComputeHashIndex( const char* pStr )
{
    u32 Hash = 5381;
    s32 C;

    // Process each character to generate the hash key
    while( (C = *pStr++) )
    {
        if( (C >= 'a') && (C <= 'z') ) C += ('A' - 'a');
        Hash = ((Hash<<5) + Hash) ^ C;
    }

    return (s16)(Hash%HASH_SIZE);
}

//==============================================================================

s16 rsc_mgr::AllocEntry(  const char* pStr )
{   
    MEMORY_OWNER( "RESOURCE MANAGER" );

    // Check if we need to grow resource list
    if( m_nResources == m_nResourcesAllocated )
    {
        m_nResourcesAllocated += 1024;
        m_pResource = (resource*)x_realloc( m_pResource, sizeof(resource)*m_nResourcesAllocated );
        ASSERT( m_pResource );

        // Add new resources to free list
        x_memset( m_pResource+m_nResources, 0, sizeof(resource)*(m_nResourcesAllocated-m_nResources) );
        for( s16 i=m_nResourcesAllocated-1; i>=m_nResources; i-- )
        {
            // Clear entry so we know it's not being used
            m_pResource[i].State = NOT_USED;

            m_pResource[i].Next = m_FirstFree;
            m_pResource[i].Prev = -1;
            if( m_FirstFree != -1 )
                m_pResource[ m_FirstFree ].Prev = i;
            m_FirstFree = i;
        }
    }

    // Increase number of resources
    m_nResources++;

    // Pull resource from free list
    ASSERT( m_FirstFree != -1 );
    s16 I = m_FirstFree;
    m_FirstFree = m_pResource[I].Next;
    if( m_FirstFree != -1 )
        m_pResource[m_FirstFree].Prev = -1;
    m_pResource[I].Next = -1;
    m_pResource[I].Prev = -1;

    // Add to used list
    if( m_FirstUsed != -1 )
        m_pResource[ m_FirstUsed ].Prev = I;
    m_pResource[I].Next = m_FirstUsed;
    m_FirstUsed = I;

    // reset the resource
    m_pResource[I].State     = NOT_LOADED;
    m_pResource[I].bTagged   = FALSE;
    m_pResource[I].NextHash  = -1;
    m_pResource[I].HashIndex = -1;

    // Add the resource to our hash table for quick lookups
    s16 HashIndex = ComputeHashIndex( pStr );
    m_pResource[I].HashIndex = HashIndex;
    m_pResource[I].NextHash  = m_HashTable[HashIndex];
    m_HashTable[HashIndex]  = I;

    return I;
}

//==============================================================================

void rsc_mgr::FreeEntry( s16 I )
{
    // Decrease number of resources
    m_nResources--;

    // Remove from the hash table
    s16 HashIndex = m_pResource[I].HashIndex;
    s16 PrevHashEntry = -1;
    s16 CurrHashEntry = m_HashTable[HashIndex];
    while( (CurrHashEntry != I) && (CurrHashEntry != -1) )
    {
        PrevHashEntry = CurrHashEntry;
        CurrHashEntry = m_pResource[CurrHashEntry].NextHash;
    }
    ASSERT( CurrHashEntry != -1 );
    if( PrevHashEntry == -1 )
    {
        // This entry was the first one in the hash table, just reset the
        // hash table pointer
        m_HashTable[HashIndex] = m_pResource[I].NextHash;
    }
    else
    {
        // This entry was either in the middle or the last one in the
        // hash table list. Set Prev->Next to Curr->Next
        m_pResource[PrevHashEntry].NextHash = m_pResource[I].NextHash;
    }

    // Remove from used list
    if( m_pResource[I].Next != -1 )
        m_pResource[ m_pResource[I].Next ].Prev = m_pResource[I].Prev;
    if( m_pResource[I].Prev != -1 )
        m_pResource[ m_pResource[I].Prev ].Next = m_pResource[I].Next;
    if( m_FirstUsed == I )
        m_FirstUsed = m_pResource[I].Next;

    // Add to free list
    m_pResource[I].Next = m_FirstFree;
    m_pResource[I].Prev = -1;
    if( m_FirstFree != -1 )
        m_pResource[ m_FirstFree ].Prev = I;
    m_FirstFree = I;

    // Clear entry so we know it's not being used
    m_pResource[I].State     = NOT_USED;
    m_pResource[I].bTagged   = FALSE;
    m_pResource[I].NextHash  = -1;
    m_pResource[I].HashIndex = -1;
}

//==============================================================================

s16 rsc_mgr::FindEntry( const char* pResourceName )
{
    if( m_pResource == NULL )
        return -1;

    // use the hash table to do a quick lookup
    s16 HashIndex = ComputeHashIndex( pResourceName );
    
    // loop through the available resources doing the strcmp's
    s16 I = m_HashTable[HashIndex];
    while( I != -1 )
    {
        if (x_toupper(m_pResource[I].Name[0]) == x_toupper(pResourceName[0]) )
        {        
            if( x_stricmp( m_pResource[I].Name, pResourceName ) == 0 )
            {
                return I;
            }
        }

        I = m_pResource[I].NextHash;
    }

    return -1;
}

//==============================================================================

void rsc_mgr::Load( const char* pResourceName )
{
#ifdef RSC_MGR_COLLECT_STATS
    xtimer Timer;
    Timer.Start();
#endif // RSC_MGR_COLLECT_STATS

    //
    // Check if resource is already loaded
    //
    s16 I = FindEntry( pResourceName );
    if( (I != -1) && ((m_pResource[I].State == LOADED) || (m_pResource[I].State == FAILED_LOAD)) )
    {
        return;
    }

    //
    // Create a new resource and init as a failed load
    //
    if( I == -1 )
    {
        I = AllocEntry( pResourceName );
        m_pResource[I].pData            = NULL;
        m_pResource[I].pLoader          = NULL;
        m_pResource[I].State            = NOT_LOADED;
#ifdef RSC_MGR_COLLECT_STATS
        m_pResource[I].FileSize         = -1;
        m_pResource[I].LoadTime         = 0;
#endif // RSC_MGR_COLLECT_STATS
        x_strncpy(m_pResource[I].Name,pResourceName,RESOURCE_NAME_SIZE);
    }

    // Get extension which begins at last period
    char* pExt = NULL;
    {
        char* C = (char*)pResourceName;
        while( *C )
        {
            if( (*C) == '.' ) pExt = C;
            C++;
        }
    }
    if( pExt == NULL )
    {
#ifdef RSC_MGR_COLLECT_STATS
        if(m_pResource[I].State != FAILED_LOAD)
            Stats.numFailedLoadResources++;
#endif // RSC_MGR_COLLECT_STATS

        m_pResource[I].State = FAILED_LOAD;
        return;
    }

    // Find loader
    rsc_loader* pLoader = m_pLoader;

#ifdef RSC_MGR_COLLECT_STATS
    s32 LoaderIndex = 0;
#endif // RSC_MGR_COLLECT_STATS
    while( pLoader )
    {        
        if( x_stricmp( pLoader->m_pExt, pExt ) == 0 )
        {   
            break;
        }        
#ifdef RSC_MGR_COLLECT_STATS
        LoaderIndex++;
#endif // RSC_MGR_COLLECT_STATS
        pLoader = pLoader->m_pNext;
    }
    if( pLoader == NULL )
    {
#ifdef RSC_MGR_COLLECT_STATS
        if(m_pResource[I].State != FAILED_LOAD)
            Stats.numFailedLoadResources++;
#endif // RSC_MGR_COLLECT_STATS

        m_pResource[I].State = FAILED_LOAD;
        return;
    }

    //
    // Create full path
    //
    char    Path[256];
    
#if defined(LOG_RESOURCES)
    s32 nBytesBefore = x_MemGetFree();
#endif // defined(LOG_RESOURCES)

    // Attempt load
    X_FILE* fp=NULL;
    void*   pData = NULL;
    x_try;
    {
        for( s32 i=0; i<4; i++ )
        {
            if( m_RootDir[i*256] )
                x_strcpy( Path, xfs("%s/%s",&m_RootDir[i*256],pResourceName) );
            else
                x_strcpy( Path, pResourceName );
            pData = pLoader->PreLoad( fp, Path );
            if( fp )
            {
                break;
            }
        }
    }
    x_catch_display;

    // Check for a failed resource loader ->PreLoad and return accordingly
    if( !fp )
    {
#ifdef RSC_MGR_COLLECT_STATS
        if(m_pResource[I].State != FAILED_LOAD)
            Stats.numFailedLoadResources++;
#endif // RSC_MGR_COLLECT_STATS
        
        x_DebugMsg( "RSCMGR: Could not open for reading (%s)\n", pResourceName );
        m_pResource[I].State = FAILED_LOAD;
        return;
    }


    // Load data
    x_try;
    {
        pData = pLoader->Resolve( pData ); 
    }
    x_catch_begin;


    // Display the exception dialog
#ifdef RSC_MGR_COLLECT_STATS
    static xbool bSkipDialog = FALSE;
    if( xExceptionCatchHandler( NULL, __LINE__, NULL, bSkipDialog) )
    {
        BREAK;
    }
#endif // RSC_MGR_COLLECT_STATS
    
    // Close the resource file and NULL the handle so the code below kicks in
    x_fclose( fp );
    fp = NULL;

    x_catch_end;

    // Check for a failed resource loader ->Resolve and return accordingly
    if( !fp )
    {
#ifdef RSC_MGR_COLLECT_STATS
        if(m_pResource[I].State != FAILED_LOAD)
            Stats.numFailedLoadResources++;
#endif // RSC_MGR_COLLECT_STATS
        x_DebugMsg( "RSCMGR: Could not open for reading (%s)\n", pResourceName );
        m_pResource[I].State = FAILED_LOAD;
        return;
    }

#ifdef RSC_MGR_COLLECT_STATS
    // Copy file size
    m_pResource[I].FileSize = x_flength(fp);

    // Copy in load time
    Timer.Stop();
    m_pResource[I].LoadTime = Timer.ReadSec();
#endif // RSC_MGR_COLLECT_STATS

    // Close file
    x_fclose(fp);

    // Check if any data was returned
    if( !pData )
    {
#ifdef RSC_MGR_COLLECT_STATS
        if(m_pResource[I].State != FAILED_LOAD)
            Stats.numFailedLoadResources++;
#endif // RSC_MGR_COLLECT_STATS
        //x_DebugMsg("RSCMGR: No data loaded\n");
        m_pResource[I].State = FAILED_LOAD;
        return;
    }

#if defined(LOG_RESOURCES)
    s32 nBytesAfter = x_MemGetFree();
    if( nBytesAfter != nBytesBefore )
    {
        // DO *not* change the output format - we have tools that parse this data!
        //LOG_MESSAGE( "rsc_mgr::Load", "%d, %s", nBytesBefore-nBytesAfter, Path );
        /*
        LOG_MESSAGE( "rsc_mgr::Load", 
                     "%s (path: [%s], size: %6d)", 
                     m_pResource[I].Name,
                     Path,
                     nBytesBefore-nBytesAfter ); 
        */
    }
#endif // defined(LOG_RESOURCES)

    m_pResource[I].pData            = pData;
    m_pResource[I].pLoader          = pLoader;
    m_pResource[I].State            = LOADED;
#ifdef RSC_MGR_COLLECT_STATS
    Stats.numLoadedPerType[LoaderIndex]++;
    Stats.numLoadedResources++;
#endif // RSC_MGR_COLLECT_STATS
}

//==============================================================================

void rsc_mgr::Unload( const char* pResourceName )
{
    // Find resource in list
    s16 I = FindEntry( pResourceName );

    if( I == -1 )
    {
        LOG_WARNING( "rsc_mgr::Unload", "RSCMGR: Could not find [%s] for unload\n", pResourceName );
        return;
    }

    if( m_pResource[I].State == LOADED )
    {
        // Unload data
        m_pResource[I].pLoader->Unload( m_pResource[I].pData );
        m_pResource[I].pData = NULL;
        m_pResource[I].State = NOT_LOADED;

        // Remove entry
        FreeEntry( I );
    }
}

//==============================================================================

void rsc_mgr::Refresh( const char* pResourceName )
{
    CONTEXT( "rsc_mgr::Refresh" );

    s16 I = FindEntry( pResourceName );
    
    if( I == -1 )
    {
        Load( pResourceName );
        return;
    }
    
    if( m_pResource[I].State == FAILED_LOAD )
        m_pResource[I].State = NOT_LOADED;

    // Unload current resource 
    if( m_pResource[I].State == LOADED )
    {
        // Unload data
        m_pResource[I].pLoader->Unload( m_pResource[I].pData );
        m_pResource[I].pData = NULL;
        m_pResource[I].State = NOT_LOADED;
    }

    // Load new resource
    Load( pResourceName );
}

//==============================================================================

void rsc_mgr::UnloadAll( xbool bIgnoreTagged )
{
    // Loop through all resources and unload
    s16 I = m_FirstUsed;
    while( I!=-1 )
    {
        // Lookup next because this resource will be deleted!
        s16 NextI = m_pResource[I].Next;

        // Find?
        s16 Info = FindEntry( m_pResource[I].Name );
        ASSERT( Info == I );
        if( Info != -1 )
        {
            if( !((bIgnoreTagged==TRUE) && (m_pResource[Info].bTagged==TRUE)) )
                Unload( m_pResource[I].Name );
        }

        // Goto next
        I = NextI;
    }

    if( bIgnoreTagged==FALSE )
    {
        x_free( m_pResource );
        m_pResource = NULL;
        m_nResources = 0;
        m_nResourcesAllocated = 0;
        m_FirstUsed = -1;
        m_FirstFree = -1;

        x_memset( m_HashTable, -1, sizeof(m_HashTable) );
#ifdef RSC_MGR_COLLECT_STATS
        x_memset((void *)&Stats, 0, sizeof(Stats));
#endif
        m_OnDemandLoading = TRUE;
    }
}

//==============================================================================

void rsc_mgr::TagResources( void )
{
    // Loop through all resources and call refresh
    s16 I = m_FirstUsed;
    while( I!=-1 )
    {
        m_pResource[I].bTagged = TRUE;
        I = m_pResource[I].Next;
    }
}

//==============================================================================

void rsc_mgr::RefreshAll( void )
{
    // Loop through all resources and call refresh
    s16 I = m_FirstUsed;
    while( I!=-1 )
    {
        Refresh( m_pResource[I].Name );
        I = m_pResource[I].Next;
    }
}

//==============================================================================

void rsc_mgr::SetOnDemandLoading( xbool OnDemand )
{
    m_OnDemandLoading = OnDemand;
}

//==============================================================================
//==============================================================================
//==============================================================================
// RHANDLE FUNCTIONS
//==============================================================================
//==============================================================================
//==============================================================================

void rsc_mgr::AddRHandle( rhandle_base& RHandle, const char* pResourceName )
{
    s16 I;

    if( pResourceName )
    {
        I = FindEntry( pResourceName );

        if( I == -1 )
        {
            // Create a new resource entry
            I = AllocEntry( pResourceName );
            m_pResource[I].pData    = NULL;
            m_pResource[I].pLoader  = NULL;
            m_pResource[I].State    = NOT_LOADED;
            x_strncpy(m_pResource[I].Name,pResourceName,RESOURCE_NAME_SIZE);

            if (!m_OnDemandLoading)
            {
               Load(m_pResource[I].Name);
            }
            else
            {
/*
                // Throw an exception for ondemand loads!
                s32   nBytes = x_strlen( m_pResource[I].Name );
                char* pName  = m_pResource[I].Name;
                if( nBytes > 40 )
                    pName += nBytes - 40;
                extern void x_DebugSetCause( const char* pCause );
                x_DebugSetCause( xfs( "OnDemand Load! File:\n" 
                    "%s", pName ) );

                *(u32*)1 = 0;
*/
            }
        }
    }
    else
    {
        I = RHandle.GetIndex();

        // Handle null handle
        if( I == -1 )
            return;

        ASSERT( (I>=0) && (I<m_nResourcesAllocated) );
    }

    // Confirm resource is being used
    ASSERT( m_pResource[I].State != NOT_USED );

    // Fill out handle information
    RHandle.SetIndex( I );
}

//==============================================================================

const char* rsc_mgr::GetRHandleName( const rhandle_base& RHandle ) const
{
    s32 I = RHandle.GetIndex();

    if( I == -1 )
        return "";

    ASSERT( (I>=0) && (I<m_nResourcesAllocated) );
    ASSERT( m_pResource[I].State != NOT_USED );

    return m_pResource[I].Name;
}

//==============================================================================

void* rsc_mgr::GetPointerSlow( const rhandle_base& RHandle )
{
    s32 I = RHandle.GetIndex();

    // Handle null handle
    if( I == -1 )
        return NULL;

    // Confirm range 
    ASSERT( (I>=0) && (I<m_nResourcesAllocated) );

    ASSERT( m_pResource[I].State != NOT_USED );

    // Check for on demand loading
    if( !m_pResource[I].pData )
    {
        if( m_OnDemandLoading )
        {
            Load( m_pResource[I].Name );
        }
        else
        {
/*
            // Throw an exception for on-demand load.
            s32   nBytes = x_strlen( m_pResource[I].Name );
            char* pName  = m_pResource[I].Name;
            if( nBytes > 40 )
                pName += nBytes - 40;
            extern void x_DebugSetCause( const char* pCause );
            x_DebugSetCause( xfs( "OnDemand Load! File:\n" 
                                  "%s", pName ) );

            *(u32*)1 = 0;
*/
        }
    }

    // Return data or null
    return m_pResource[I].pData;
}

//==============================================================================

xbool rsc_mgr::IsRHandleLoaded ( const rhandle_base& RHandle )
{
    s16 I = RHandle.GetIndex();

    // Handle null handle
    if( I == -1 )
        return FALSE;

    // Confirm range 
    ASSERT( (I>=0) && (I<m_nResourcesAllocated) );
    ASSERT( m_pResource[I].State != NOT_USED );

    if( m_pResource[I].State == LOADED )
        return TRUE;

    return FALSE;
}

//==============================================================================


const char* rsc_mgr::FixupFilename( const char* Filename )
{
    if ( (x_strlen(Filename)+x_strlen(m_RootDir)) > (s32)sizeof(FixupBuffer)-4 )
        x_throw("Rsc_mgr: Fixed up path exceeds size of internal buffer" );

    x_sprintf(FixupBuffer, "%s/%s",m_RootDir,Filename);

    return FixupBuffer;
}

