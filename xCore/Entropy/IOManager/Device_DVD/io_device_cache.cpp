//==============================================================================
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================
//==============================================================================
//
// DVD caching to a background device.
//
//==============================================================================
#define X_SUPPRESS_LOGS
#include "x_files.hpp"

#include "io_device_dvd.hpp"
#include "io_device_cache.hpp"

#if defined(bwatson) && defined(X_DEBUG)
#define DEBUG_CACHE
#endif

// Show read error blue screen?
#if defined( TARGET_PS2 ) && ( defined( CONFIG_DEBUG ) || defined( CONFIG_VIEWER ) || defined( CONFIG_QA ) )
#define IO_DEBUG_SHOW_READ_ERROR
#endif


#if !defined(DEBUG_CACHE) && !defined(X_RETAIL)
#define x_DebugMsg if(0) x_DebugMsg
#endif

cache_handler g_IODeviceCache;

const char*             s_PartitionName = "BASLUS-20595";
const char*             s_Password      = "";


//==============================================================================
// Debug functions
//==============================================================================

#ifdef IO_DEBUG_SHOW_READ_ERROR

#ifdef CONFIG_RETAIL
#error io debug show read error not allowed in retail builds!!!
#endif

// Display read error blue screen
extern void x_DebugSetCause( const char* pCause );
void IO_DebugShowReadError( const char* pFileName )
{
    // Force blue screen crash with message
    x_DebugSetCause( xfs( "Disc read error on file: %s", pFileName ) );
    *(u32*)1 = 0;
}

#endif  //#ifdef IO_DEBUG_SHOW_READ_ERROR


//==============================================================================
#if defined(ENABLE_CACHING)
void s_StartCacheThread( s32 , char** argv )
{
    cache_handler* pHandler = (cache_handler*)argv;

    pHandler->CacheThread();
}
#endif

//-----------------------------------------------------------------------------
// 
void cache_handler::Init(void)
{
#if defined(ENABLE_HDD_SUPPORT) && defined(ENABLE_CACHING)
    s32 i;
    InvalidateCache();
    for( i=0; i<MAX_CACHE_REQUESTS; i++ )
    {
        m_CopyQueue[i].m_Status         = CACHE_COPY_IDLE;
        m_CopyQueue[i].m_DestHandle     = -1;
        m_CopyQueue[i].m_DestLength     = 0;
        m_CopyQueue[i].m_SourceHandle   = -1;
        m_CopyQueue[i].m_SourceLength   = 0;
    }
    m_CopyEntryIndex = 0;
    // Kick off a thread that will deal with initializing the HDD
    m_pCacheThread = new xthread(s_StartCacheThread, "Cache background reader", 8192, 4, 1, (char**)this);
#else
    m_Status = CACHE_NOT_PRESENT;
#endif
}

//-----------------------------------------------------------------------------
void cache_handler::Kill(void)
{
#if defined(ENABLE_HDD_SUPPORT) && defined(ENABLE_CACHING)
    delete m_pCacheThread;
    m_pCacheThread = NULL;
#endif
}

//-----------------------------------------------------------------------------
// NOTE: Any file i/o requests need to wait until the cache manager is ready
// prior to submitting a request to it. This is because the PS2 cache manager 
// takes a considerable period of time to start up and there's no good reason
// to block file i/o while it is.
//-----------------------------------------------------------------------------
// Device manager open interface function
s32  cache_handler::Open(const char* pFilename, cache_open_mode Mode, u32& Length)
{
    s32             Handle;
#if defined(ENABLE_HDD_SUPPORT) && defined(ENABLE_CACHING)
    cache_request   Request;
    xbool           Excluded = FALSE;
    const char*     AUDIOPKG_EXT = ".audiopkg";

    if( (x_strlen(pFilename) > x_strlen(AUDIOPKG_EXT)) &&
        (x_strcmp(pFilename-x_strlen(AUDIOPKG_EXT), AUDIOPKG_EXT)==0) )
    {
        Excluded = TRUE;
    }

    if( (Mode == CACHE_OPEN_WRITE) && (x_strncmp("HDD:", pFilename, 4 )!=0) )
    {
        Excluded = TRUE;
    }

    if( CacheIsReady() && !Excluded )
    {
        Request.SetType     ( CACHE_REQUEST_OPEN );
        Request.SetFilename ( pFilename );
        Request.SetMode     ( Mode );
        Request.SetFilename ( pFilename );

        m_qRequests.Send( &Request, MQ_BLOCK );
        if( Request.WaitDone() <= 0 )
        {
            return 0;
        }

        Length = Request.GetLength();
        ASSERT( (Request.GetHandle() & CACHED_FLAG)==0 );
        return Request.GetHandle() | CACHED_FLAG;
    }
#endif
    // No cache, just read it from the CD, or the host then.
    Handle = SystemOpen( pFilename, Mode );
    if( Handle <= 0 )
        return 0;

    ASSERT( (Handle & CACHED_FLAG) == 0 );
    Length = SystemGetLength( Handle );
    return Handle;
}

//-----------------------------------------------------------------------------
// Device manager read interface function
s32 cache_handler::Read(s32 Handle, void* Addr, s32 Length, s32 Offset )
{
#if defined(ENABLE_HDD_SUPPORT) && defined(ENABLE_CACHING)
    cache_request Request;

    ASSERT( Handle & ~ CACHED_FLAG );
    if( Handle & CACHED_FLAG )
    {
        Request.SetType     ( CACHE_REQUEST_READ );
        Request.SetHandle   ( Handle & ~CACHED_FLAG );
        Request.SetDataPtr  ( Addr );
        Request.SetLength   ( Length );
        Request.SetOffset   ( Offset );
        ASSERT( Request.GetHandle() );
        m_qRequests.Send( &Request, MQ_BLOCK );
        return Request.WaitDone();
    }
#endif
    return SystemRead( Handle, Offset, Addr, Length );
}

//-----------------------------------------------------------------------------
// Device manager write interface function

s32 cache_handler::Write(s32 Handle, void* Addr, s32 Length, s32 Offset )
{
#if defined(ENABLE_HDD_SUPPORT) && defined(ENABLE_CACHING)
    cache_request Request;

    ASSERT( Handle & ~ CACHED_FLAG );
    if( Handle & CACHED_FLAG )
    {
        Request.SetType     ( CACHE_REQUEST_WRITE );
        Request.SetHandle   ( Handle & ~CACHED_FLAG );
        Request.SetDataPtr  ( Addr );
        Request.SetLength   ( Length );
        Request.SetOffset   ( Offset );
        m_qRequests.Send    ( &Request, MQ_BLOCK );
        return Request.WaitDone();
    }
#endif
    return SystemWrite( Handle, Offset, Addr, Length );
}

//-----------------------------------------------------------------------------
// Device manager close interface function
void cache_handler::Close( s32 Handle )
{
#if defined(ENABLE_HDD_SUPPORT) && defined(ENABLE_CACHING)
    cache_request Request;

    ASSERT( Handle & ~ CACHED_FLAG );
    if( Handle & CACHED_FLAG )
    {
        Request.SetType     ( CACHE_REQUEST_CLOSE );
        Request.SetHandle   ( Handle & ~CACHED_FLAG);
        m_qRequests.Send    ( &Request, MQ_BLOCK );
        Request.WaitDone    ( );
        return;
    }
#endif
    SystemClose( Handle );
}

//-----------------------------------------------------------------------------
xbool cache_handler::CacheIsReady( void )
{
    return ( m_Status != CACHE_INIT );
}

#if defined(ENABLE_HDD_SUPPORT) && defined(ENABLE_CACHING)
//-----------------------------------------------------------------------------
void cache_handler::InvalidateCache( void )
{
    m_CacheOwner             = -1;
    m_CacheOffset            = -1;
    m_CacheDirty             = TRUE;
}

//-----------------------------------------------------------------------------
void cache_handler::CacheThread( void )
{
    xtimer      t;
    t.Start();
    m_Status = CACHE_INIT;
    m_Status = SystemInit();

    t.Stop();
    LOG_MESSAGE("cache_handler::CacheThread", "HDD Init took %2.02f seconds\n",(f32)t.ReadSec());

    m_CopyingFile = FALSE;
    while( TRUE )
    {
        cache_request* pRequest;

#if defined(ENABLE_CACHING)
        // If we're in the process of copying a file, then we just poll to see if there is a request waiting to
        // run.
        if( m_CopyingFile )
        {
            pRequest = (cache_request*)m_qRequests.Recv( MQ_NOBLOCK );
        }
        else
#endif
        {
            t.Reset();
            t.Start();
            pRequest = (cache_request*)m_qRequests.Recv( MQ_BLOCK );
            t.Stop();
        }

        if( pRequest )
        {
            switch( pRequest->GetType() )
            {
                //-----------------------------------
            case CACHE_REQUEST_OPEN:    ProcessOpenRequest  ( pRequest );   break;
            case CACHE_REQUEST_CLOSE:   ProcessCloseRequest ( pRequest );   break;
            case CACHE_REQUEST_READ:    ProcessReadRequest  ( pRequest );   break;
            case CACHE_REQUEST_WRITE:   ProcessWriteRequest ( pRequest );   break;
            default:
                ASSERT( FALSE );
            }
        }

#if defined(ENABLE_CACHING)
        UpdateCache();
#endif
    }
}

//-----------------------------------------------------------------------------
void cache_handler::ProcessOpenRequest( cache_request* pRequest )
{
    char*           pName;
    const char*     pFilename;
    s32             Handle;
    s32             Length=0;
    cache_open_mode OpenMode=CACHE_OPEN_READ;

    switch( pRequest->GetMode() )
    {
    case CACHE_OPEN_READ:  OpenMode = CACHE_OPEN_READ;  break;
    case CACHE_OPEN_WRITE: OpenMode = CACHE_OPEN_WRITE; break;
    default:        ASSERT(FALSE);
    }

    if( m_CacheDeviceIsPresent == FALSE )
    {
        // if we have no cache device, then all we'll be doing is double buffering I/O requests, so we just
        // open the file as if it were a cache file.
        Handle = SystemOpen( pRequest->GetFilename(), OpenMode );
        if( Handle > 0 )
        {
            ASSERT( (Handle & CACHED_FLAG) == 0 );
            pRequest->SetLength( SystemGetLength( Handle ) );
        }
        pRequest->SetHandle     ( Handle );
        pRequest->RequestDone   ( Handle );
        return;
    }

    // First, see if the file exists on the HD. So we just grab the filename portion
    // with pfs0: first.
    pName = (char*)m_CacheBuffer;
#if defined(TARGET_PS2)
    x_strcpy( pName, "pfs0:" );
#else
    x_strcpy( pName, "Z:\\" );
#endif
    pFilename = pRequest->GetFilename();
    pFilename = x_strchr( pFilename, ':' );
    ASSERT( pFilename );
    x_strcat( pName, pFilename+1 );
    // Open the target file (on the caching device)
    Handle = SystemOpen( pName, OpenMode );
    if( Handle > 0 )
    {
        ASSERT( (Handle & CACHED_FLAG) == 0 );
        //
        // If it was there, then this means we've already copied that file. 
        // **NOTE TBD** need to verify source & dest files are the same length. If not, 
        // either restart the copy process from the current point or re-do everything 
        // from the beginning.
        //
        Length = SystemGetLength( Handle );
        pRequest->SetLength     ( Length );
        pRequest->SetHandle     ( Handle );
        pRequest->RequestDone   ( Handle );
        LOG_MESSAGE("cache_handler::ProcessOpenRequest", "Open: File:%s, handle:%d\n", pName, pRequest->GetHandle() );
        return;
    }

#if defined(ENABLE_CACHING)
    // Find a cache entry to queue up a file copy request. If we can't find one then we have
    // too many requests pending so all we do is update the cache until we can find one.
    s32     Index;
    xbool   HasBeenReported;

    Index = m_CopyEntryIndex;
    HasBeenReported = FALSE;

    while( TRUE )
    {
        if( m_CopyQueue[Index].m_Status == CACHE_COPY_IDLE )
        {
            // Found one!
            break;
        }
        Index++;
        if( Index >= MAX_CACHE_REQUESTS )
        {
            Index = 0;
        }
        // We looped completely, so this means no entries are available. In this case, we stick
        // in this loop doing a cache update
        if( Index == m_CopyEntryIndex )
        {
            if( !HasBeenReported )
            {
                HasBeenReported = TRUE;
                LOG_WARNING("cache_handler::ProcessOpenRequest","Could not get copy queue entry. Waiting for copy to complete.");
            }
            UpdateCache();
        }
    }
    cache_pending& Cache = m_CopyQueue[Index];

    // Ok, the file is NOT on the HDD, so we need to start a copy process from the CD. If the
    // file does not exist on the source, then we don't have the file at all so we give up.
    Cache.m_SourceHandle = SystemOpen( pRequest->GetFilename(), OpenMode );
    if( Cache.m_SourceHandle <= 0 )
    {
        pRequest->SetHandle     ( Cache.m_SourceHandle );
        pRequest->RequestDone   ( Cache.m_SourceHandle );
        return;
    }
    ASSERT( (Cache.m_SourceHandle & CACHED_FLAG) == 0 );

    // It could be perfectly legit for us to not be able to create the target file. This
    // would be because we've run out of space. We'll deal with that later. Right now,
    // we just assume it's ok. The other, device, layer should detect this error then
    // open a CD based file.
    Cache.m_DestHandle      = SystemOpen( pName, CACHE_OPEN_WRITE );
    Cache.m_DestLength      = 0;
    Cache.m_SourceLength    = SystemGetLength( Cache.m_SourceHandle );

    ASSERT( Cache.m_DestHandle > 0 );
    ASSERT( (Cache.m_DestHandle & CACHED_FLAG) == 0 );
    pRequest->SetLength     ( Cache.m_SourceLength );
    pRequest->SetHandle     ( Cache.m_DestHandle );
    pRequest->RequestDone   ( Cache.m_DestHandle );
    Cache.m_Status = CACHE_COPY_WAITING;

    LOG_MESSAGE("cache_handler::ProcessOpenRequest", "Open cache: File:%s, cache handle:%d, handle:%d, queue entry:%d\n", pName, Cache.m_SourceHandle, Cache.m_DestHandle, Index );
#else
    // if we have caching disabled then we just
    // open the file as if it were a cache file.
    Handle = SystemOpen( pRequest->GetFilename(), OpenMode );
    if( Handle > 0 )
    {
        ASSERT( (Handle & CACHED_FLAG) == 0 );
        pRequest->SetLength( SystemGetLength( Handle ) );
    }
    pRequest->SetHandle     ( Handle );
    pRequest->RequestDone   ( Handle );
    return;
#endif
}

//-----------------------------------------------------------------------------
void cache_handler::ProcessCloseRequest( cache_request* pRequest )
{
#if defined(ENABLE_CACHING)
    if( m_CacheOwner == pRequest->GetHandle() )
    {
        InvalidateCache();
    }
#endif
    LOG_MESSAGE("cache_handler::ProcessCloseRequest", "Close file handle: %d\n", pRequest->GetHandle() );
    // Closes are ALWAYS straight forward.
    SystemClose( pRequest->GetHandle() );
    pRequest->RequestDone( 0 );
}

//-----------------------------------------------------------------------------
void cache_handler::ProcessReadRequest( cache_request* pRequest )
{
    s32 Length;
    s32 RequestedLength;
    s32 ProcessedLength;

    ASSERT( pRequest->GetHandle() );
    //
    // If we're currently copying the file we're interested in, then
    // we make sure what we're trying to read has been copied. If it
    // hasn't, then we send the request back to the queue so it will
    // be processed next time round.
    //
    // Note: Sending the requests back to the end of the request queue will
    // mean that any additional requests sent will be processed if they can
    // be. This will be just after a cache copy is complete. The cache copying
    // is the mechanism that will force this thread to sleep.
    //
    ProcessedLength = 0;
    RequestedLength = pRequest->GetLength();

#if defined(ENABLE_CACHING)
    static xbool NotifyWaitingOnData=TRUE;

    cache_pending& Cache = m_CopyQueue[ m_CopyEntryIndex ];
    if( Cache.m_Status == CACHE_COPY_PROCESSING )
    {
        if( (pRequest->GetHandle() == Cache.m_DestHandle) &&
            ((pRequest->GetOffset() + pRequest->GetLength()) > Cache.m_DestLength) )
        {
            if( NotifyWaitingOnData )
            {
                LOG_WARNING("cache_handler::ProcessReadRequest","Read request delayed waiting on copy, handle:%d, Offset:0x%08x, Copy Offset:0x%08x", Cache.m_DestHandle, pRequest->GetOffset(), Cache.m_DestLength );
            }
            m_qRequests.Send( pRequest, MQ_BLOCK );
            return;
        }
    }

    if( !NotifyWaitingOnData )
    {
        LOG_MESSAGE("cache_handler::ProcessReadRequest","Read request resumed. handle:%d", Cache.m_DestHandle );
        NotifyWaitingOnData = TRUE;
    }
    //
    // If the file we're interested in is waiting to be copied, but not in
    // the process of being copied, then we just delay the read request 
    // until it's copied.
    //
    static xbool NotifyWaitingOnCopy = TRUE;
    s32 i;
    for( i=0; i < MAX_CACHE_REQUESTS; i++ )
    {
        if( (m_CopyQueue[i].m_Status == CACHE_COPY_WAITING) &&
            (m_CopyQueue[i].m_DestHandle == pRequest->GetHandle()) )
        {
            if( NotifyWaitingOnCopy )
            {
                LOG_WARNING("cache_handler::ProcessReadRequest","Read request delayed waiting on copy, handle:%d, Offset:0x%08x, Copy Offset:0x%08x", Cache.m_DestHandle, pRequest->GetOffset(), Cache.m_DestLength );
                NotifyWaitingOnCopy = FALSE;
            }

            m_qRequests.Send( pRequest, MQ_BLOCK );
            return;
        }
    }

    if( !NotifyWaitingOnCopy )
    {
        LOG_MESSAGE("cache_handler::ProcessReadRequest","Read request resumed. handle:%d", Cache.m_DestHandle );
        NotifyWaitingOnCopy = TRUE;
    }
    // We're assuming all sorts of stuff about cache reads. Due to the IO manager, it *should*
    // always be 32K aligned, and of 32K in size unless it's the last block.
    Length = 0;
    xbool ThisIsOwner       = (m_CacheOwner == pRequest->GetHandle());
    xbool CacheBaseTheSame  = (m_CacheOffset == pRequest->GetOffset());
    xbool CacheIsValid      = (m_CacheDirty == FALSE);
    if( ThisIsOwner && CacheBaseTheSame && CacheIsValid )
    {
        Length = MIN( pRequest->GetLength(), (s32)sizeof(m_CacheBuffer) );
        LOG_MESSAGE("cache_handler - Cache", "Readahead copy: handle:%d, dest:0x%08x, offset:0x%08x, length:%d\n",pRequest->GetHandle(), pRequest->GetDataPtr(), pRequest->GetOffset(), Length );
        x_memcpy( pRequest->GetDataPtr(), m_CacheBuffer, Length );
        // Advance all the pointers we need to 
        pRequest->SetDataPtr( (byte*)pRequest->GetDataPtr() + Length );
        pRequest->SetLength( pRequest->GetLength() - Length );
        pRequest->SetOffset( pRequest->GetOffset() + Length );
        ProcessedLength = Length;
    }
#endif // ENABLE_CACHING

    // Anything left, we just read the data.
    if( pRequest->GetLength() )
    {
        LOG_MESSAGE("cache_handler::ProcessReadRequest", "Read handle:%d, dest:0x%08x, offset:0x%08x, length:%d\n",pRequest->GetHandle(), pRequest->GetDataPtr(), pRequest->GetOffset(), pRequest->GetLength() );
        Length = SystemRead( pRequest->GetHandle(), pRequest->GetOffset(), pRequest->GetDataPtr(), pRequest->GetLength() );
        ProcessedLength+= Length;
        pRequest->SetOffset( pRequest->GetOffset() + Length );
    }

#if defined(ENABLE_CACHING)
    if( m_CacheOwner != pRequest->GetHandle() )
    {
        m_CacheOwner            = pRequest->GetHandle();
        m_CacheOffset           = pRequest->GetOffset();
        m_CacheDirty            = TRUE;        // Cache has been used, so it's ready for a refill.
        m_CacheFileLength       = SystemGetLength( m_CacheOwner );
    }
#endif

    // Tell the caller we're done and start to set up for the next pre-read.
    ASSERT( ProcessedLength == RequestedLength );
    pRequest->RequestDone( ProcessedLength );
}

//-----------------------------------------------------------------------------
void cache_handler::ProcessWriteRequest( cache_request* pRequest )
{
    s32 Length;

    Length = SystemWrite( pRequest->GetHandle(), pRequest->GetOffset(), pRequest->GetDataPtr(), pRequest->GetLength() );
    pRequest->RequestDone( Length );
}

//-----------------------------------------------------------------------------
void cache_handler::UpdateCache( void )
{
#if defined(ENABLE_CACHING)
    s32 Length;

    if( !m_CopyingFile )
    {
        // Scan through for potentially pending copy requests.
        s32 Index;
        s32 i;
        Index = m_CopyEntryIndex;

        for( i = 0; i < MAX_CACHE_REQUESTS; i++ )
        {
            cache_pending& Cache = m_CopyQueue[Index];

            ASSERT( Cache.m_Status != CACHE_COPY_PROCESSING );
            // Found one! Set up the copy process.
            if( Cache.m_Status == CACHE_COPY_WAITING )
            {
                ASSERT( Cache.m_SourceHandle != -1 );
                ASSERT( Cache.m_DestHandle != -1 );
                ASSERT( Cache.m_DestLength == 0 );
                Cache.m_Status      = CACHE_COPY_PROCESSING;
                m_CopyEntryIndex    = Index;
                m_CopyingFile       = TRUE;
                Cache.m_StartTime.Reset();
                Cache.m_StartTime.Start();
                LOG_MESSAGE("cache_handler::UpdateCache", "Started new copy. Index:%d, SourceHandle:%d, DestHandle:%d", Index, Cache.m_SourceHandle, Cache.m_DestHandle );
                break;
            }
            Index++;
            if( Index >= MAX_CACHE_REQUESTS )
            {
                Index = 0;
            }
        }
    }

    if( m_CopyingFile )
    {
        cache_pending& Cache = m_CopyQueue[ m_CopyEntryIndex ];
        ASSERT( Cache.m_Status == CACHE_COPY_PROCESSING );

        s32 ReadLength;
        s32 WriteLength;

        Length = MIN( Cache.m_SourceLength - Cache.m_DestLength, (s32)sizeof(m_CacheBuffer) );
        ReadLength = SystemRead( Cache.m_SourceHandle, Cache.m_DestLength, m_CacheBuffer, Length );
        ASSERT( ReadLength == Length );
        WriteLength = SystemWrite( Cache.m_DestHandle, Cache.m_DestLength, m_CacheBuffer, ReadLength );
        ASSERT( WriteLength >= 0 );
        //
        //*** WILL NEED ERROR RECOVERY FOR VOLUME FULL
        //
        ASSERTS( WriteLength==ReadLength, "Write Failed. Is cache full?");
        LOG_MESSAGE("cache_handler::UpdateCache", "Cache copy: SrcHandle:%d, DestHandle:%d, Offset:0x%08x", Cache.m_SourceHandle, Cache.m_DestHandle, Cache.m_DestLength);
        //
        // Cache contains clean data. Let's not bother invalidating it as there *may* be a chance
        // the app is requesting this block on the next read.
        //
        Cache.m_DestLength  = Cache.m_DestLength+WriteLength;

        // Are we done copying now?
        if( Cache.m_DestLength == Cache.m_SourceLength )
        {
            f32 CopyTime;
            f32 KBytesCopied;
            KBytesCopied = Cache.m_DestLength / 1024.0f;
            Cache.m_StartTime.Stop();
            CopyTime = Cache.m_StartTime.ReadSec();
            SystemClose( Cache.m_SourceHandle );
            LOG_MESSAGE("cache_handler::UpdateCache", "Close cache: handle:%d, %2.02f sec to copy %2.02f KB, %2.02f KB/sec\n", Cache.m_SourceHandle, CopyTime, KBytesCopied, KBytesCopied/CopyTime);
            x_DebugMsg("Close cache: handle:%d, %2.02f sec to copy %2.02f KB, %2.02f KB/sec\n", Cache.m_SourceHandle, CopyTime, KBytesCopied, KBytesCopied/CopyTime);
            m_CopyingFile = FALSE;
            Cache.m_SourceHandle    = -1;
            Cache.m_DestHandle      = -1;
            Cache.m_Status          = CACHE_COPY_IDLE;
        }
        m_CacheOwner        = Cache.m_DestHandle;
        m_CacheOffset       = Cache.m_DestLength;
        m_CacheDirty        = FALSE;
        m_CacheFileLength   = Cache.m_DestLength;
    }

    // Due to the logic above, the cache will never be flagged dirty or owned if we're in the process
    // of copying a file to the caching device, so this should not be entered if we're copying.

    // Now, for a little niftyness, let's pre-read the next block as we've most likely just consumed 
    // the last one and are about to consume the next one. So, why not be ready for it?
    //* IMPLEMENTATION NOTE *
    // Due to the way this sub-level of caching works, we *should* see a marked improvement in performance from
    // dvd if there is no HDD present. So, we managed to get a benefit from the additional data usage after
    // all! Double buffered caching at no additional cost.

    if( m_CacheDirty && (m_CacheOwner != -1) )
    {
        xtimer  t;
        f32     t1;
        s32     Result;
        ASSERT(!m_CopyingFile);
        t.Start();

        t1 = t.TripMs();
        Length = MIN( m_CacheFileLength - m_CacheOffset, (s32)sizeof(m_CacheBuffer) );
        Result = SystemRead( m_CacheOwner, m_CacheOffset, m_CacheBuffer, Length );
        t.Stop();
        LOG_MESSAGE("cache_handler - Cache", "Cache readahead: Handle:%d, Offset:0x%08x, %2.02fms(%2.02fms)\n", m_CacheOwner, m_CacheOffset, t.ReadMs(), t1 );
        m_CacheDirty = FALSE;
    }
#else
    ASSERT(FALSE);
#endif
}

#endif // ENABLE_HDD_SUPPORT
//******************************************************************************
// TARGET SPECIFIC I/O Routines.
//******************************************************************************
//

// We only need a few functions to actually implement the caching. We need 
// synchronous I/O functions even if the underlying system is asynchronous.
// We must be careful since these functions can be called from either the
// cache thread or the main application thread, this will depend on whether
// or not the cache initialized by the time the file was opened. It prevents any
// stalls due to cache writing.
//
//******************************************************************************
#if defined(TARGET_PS2)

#include "ps2/iopmanager.hpp"

#include "sifrpc.h"
#include "eekernel.h"

//-----------------------------------------------------------------------------
cache_status cache_handler::SystemInit( void )
{
#if defined(ENABLE_CACHING) && defined(ENABLE_HDD_SUPPORT)
    s32 status;
    const char hddarg[] = "-o" "\0" "8" "\0" "-n" "\0" "16";

    m_CacheDeviceIsPresent = FALSE;

    m_ModuleHandle = g_IopManager.LoadModule( "hdd.irx", hddarg, sizeof(hddarg), TRUE );
    //
    // If the module doesn't load, then we definitely do NOT have an hdd installed. However, if it
    // does load, then we may still not have the hdd installed, so we need to check further.
    //

    if( m_ModuleHandle < 0 )
    {
        return CACHE_NOT_PRESENT;
    }

    status = sceDevctl("hdd0:", HDIOC_STATUS, NULL, 0, NULL, 0);

    switch( status )
    {
        //-----------------------------------
    case 0:                             // All is OK so far.
        break;
        //-----------------------------------
    case 2:                             // Locked?
        return CACHE_NOT_PRESENT;
        break;
        //-----------------------------------
    case 1:                             // Not formatted.
        return CACHE_NOT_FORMATTED;
        break;
        //-----------------------------------
    default:                            // Any other error and it's not connected.
        return CACHE_NOT_PRESENT;
        break;
    }


#if defined(X_DEBUG)
    // On a debug build, we format the drive all the time.
    sceFormat( "hdd0:", NULL, NULL, 0 );
#endif

    // Check to see if our partition exists. If not, try and create one. Any error would
    // mean that there is insufficient space. We may need to elaborate on the error condition
    // here.

    status = sceOpen( xfs("hdd0:%s,%s,%s",s_PartitionName,s_Password,s_Password), SCE_RDWR );
    if( status < 0 )
    {
        status = sceOpen( xfs("hdd0:%s,%s,%s,256M,PFS",s_PartitionName, s_Password, s_Password) ,SCE_CREAT|SCE_RDWR);
        LOG_MESSAGE("cache_handler::InitDevice","sceOpen (create) returned status %d\n", status );
        if( status >= 0 )
        {
            s32 zonesize = 8192;
            sceClose( status );

            status = sceFormat("pfs:", xfs("hdd0:%s,%s", s_PartitionName, s_Password) , &zonesize, sizeof(zonesize) );
            LOG_MESSAGE("cache_handler::InitDevice", "sceFormat returned status %d\n", status );
            if( status < 0 )
            {
                return CACHE_ERROR;
            }
        }
        else
        {
            return CACHE_FULL;
        }
    }
    else
    {
        sceClose( status );
    }
    //
    // If any of the above operations failed, then it means that the drive is full or there is a specific
    // error with the drive, such as I/O error. We need to check for this at a later stage.
    //

    // The partition should now exist, so mount should not fail if we're going to use this part.
    status = sceMount("pfs0:",xfs("hdd0:%s,%s",s_PartitionName, s_Password),0,NULL,0);
    LOG_MESSAGE( "cache_handler::InitDevice", "sceMount returned status %d\n",status );
    if( status < 0 )
    {
        ASSERT(FALSE);
        // Partition corrupt? Should I recreate it?
        return CACHE_ERROR;
    }
    m_CacheDeviceIsPresent = TRUE;
    return CACHE_READY;
#else
    return CACHE_NOT_PRESENT;
#endif
}

//-----------------------------------------------------------------------------
s32 cache_handler::SystemOpen( const char* pFilename, cache_open_mode Mode )
{
    s32         Handle = -1;
    char        Path[128];
    char*       pName;
    const char* pOriginal;

    switch( Mode )
    {
    case CACHE_OPEN_READ:
        Handle = sceOpen( pFilename, SCE_RDONLY, 0 );
        break;
    case CACHE_OPEN_WRITE:

        // Deal with creating a directory for this save.
        // Go through each section of the filename and create the directory if
        // it doesn't exist.
        pName = Path;
        pOriginal = pFilename;

        while( *pOriginal )
        {
            // If we're at the end of the original string, then we just
            // passed the filename component so we skip trying to check
            // for a directory.
            if( (*pOriginal == '\\') || (*pOriginal=='/') )
            {
                *pName = 0x0;
                Handle = sceDopen( Path );
                LOG_MESSAGE( "cache_handler::SystemOpen","Open directory %s returned %d", Path, Handle );
                if( Handle < 0 )
                {
                    Handle = sceMkdir( Path, SCE_STM_RWXU | SCE_STM_RWXG | SCE_STM_RWXO );
                    LOG_MESSAGE("cache_handler::SystemOpen","Make directory %s returned %d", Path, Handle );
                    ASSERT( Handle >= 0 );
                }
                else
                {
                    sceDclose( Handle );
                }
                *pName++='\\';
                pOriginal++;
            }
            else
            {
                *pName++ = *pOriginal++;
            }
        }
        *pName = 0;
        Handle = sceOpen( Path, SCE_RDWR|SCE_TRUNC|SCE_CREAT, SCE_STM_RWXU | SCE_STM_RWXG | SCE_STM_RWXO );
        break;
    default:
        ASSERT( FALSE );
    }
    //
    // PS2 can have a handle value of 0 be valid.
    //
    if( Handle < 0 )
        return 0;

    return Handle+1;
}

//-----------------------------------------------------------------------------
s32 cache_handler::SystemGetLength( s32 Handle )
{
    return sceLseek( Handle-1, 0, SCE_SEEK_END );
}

//-----------------------------------------------------------------------------
void cache_handler::SystemClose( s32 Handle )
{
    sceClose( Handle-1 );
#if defined(ENABLE_CACHING)
    sceSync("pfs0:",0);
#endif

}

//-----------------------------------------------------------------------------
s32 cache_handler::SystemWrite( s32 Handle, s32 Offset, void* Data, s32 Length )
{
    s32 LengthWritten;
    s32 Position;

    Position = sceLseek( Handle-1, Offset, SCE_SEEK_SET );
    ASSERT( Position == Offset );
    LengthWritten = sceWrite( Handle-1, Data, Length );
    ASSERT( Length >= 0 );
    ASSERT( LengthWritten == Length );
    return LengthWritten;
}

//-----------------------------------------------------------------------------
s32 cache_handler::SystemRead( s32 Handle, s32 Offset, void* Data, s32 Length )
{
#ifdef IO_DEBUG_SHOW_READ_ERROR
    s32 SeekRetries = 0;
    s32 ReadRetries = 0;
#endif

    s32 LengthRead = -1;
    s32 Position = -1;

    while( Position != Offset )
    {
        Position = sceLseek( Handle-1, Offset, SCE_SEEK_SET );
        
#ifdef IO_DEBUG_SHOW_READ_ERROR
        if( ++SeekRetries > 20 )
        {
            IO_DebugShowReadError( "<unknown>" );
        }
#endif
    }
    ASSERT( Position == Offset );
    
    while( LengthRead != Length )
    {
        LengthRead = sceRead( Handle-1, Data, Length );
        
#ifdef IO_DEBUG_SHOW_READ_ERROR
        if( ++ReadRetries > 20 )
        {
            IO_DebugShowReadError( "<unknown>" );
        }
#endif
    }
    
    ASSERT( LengthRead >= 0 );
    ASSERT( LengthRead == Length );
    return LengthRead;
}

#elif defined(TARGET_XBOX)

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#ifdef TARGET_XBOX
#   include "xbox/xbox_private.hpp"
#endif

//-----------------------------------------------------------------------------
cache_status cache_handler::SystemInit( void )
{
    // We probably should clean the partition at this point, if we need to.
    //m_CacheDeviceIsPresent = XFormatUtilityDrive();
    m_CacheDeviceIsPresent = TRUE;
    return CACHE_READY;
}

//-----------------------------------------------------------------------------
s32 cache_handler::SystemOpen( const char* pFilename, cache_open_mode Mode )
{
    HANDLE  Handle = INVALID_HANDLE_VALUE;

    switch( Mode )
    {
    case CACHE_OPEN_READ:
        Handle = CreateFile( pFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        break;

    case CACHE_OPEN_WRITE:
        Handle = CreateFile( pFilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
        break;
    }

    if( Handle == INVALID_HANDLE_VALUE )
        return FALSE;

    return (s32)Handle;
}

//-----------------------------------------------------------------------------
void cache_handler::SystemClose( s32 Handle )
{
    CloseHandle( (HANDLE)Handle );
}

//==============================================================================
static void CALLBACK CompletionCallback(
  DWORD dwErrorCode,                // completion code
  DWORD dwNumberOfBytesTransfered,  // number of bytes transferred
  LPOVERLAPPED lpOverlapped         // I/O information buffer
)
{
    (void)dwErrorCode;
    (void)dwNumberOfBytesTransfered;
    (void)lpOverlapped;
}

//-----------------------------------------------------------------------------
s32 cache_handler::SystemRead( s32 Handle, s32 Offset, void* Data, s32 Length )
{
    xbool        Ok;
    DWORD        ReadBytes;

    ASSERT( Handle );

    SetFilePointer( (HANDLE)Handle, Offset, 0, FILE_BEGIN );
    Ok=ReadFile( (HANDLE)Handle, Data, Length, &ReadBytes, NULL );
    // This will sleep until the request has been completed.
    if( Ok )
    {
        return ReadBytes;
    }
    return 0;
}

//-----------------------------------------------------------------------------
s32 cache_handler::SystemWrite( s32 Handle, s32 Offset, void* Data, s32 Length )
{
    xbool       Ok;
    OVERLAPPED  Request;
    DWORD       BytesWritten;

    x_memset(&Request, 0, sizeof(Request) );
    Request.Offset = Offset;

    SetFilePointer( (HANDLE)Handle, Offset, 0, FILE_BEGIN );
    Ok = WriteFile( (HANDLE)Handle, Data, Length, &BytesWritten, NULL );
    ASSERT( BytesWritten == Length );
    if( Ok )
    {
        return BytesWritten;
    }
    return 0;
}

//-----------------------------------------------------------------------------
s32 cache_handler::SystemGetLength( s32 Handle )
{
    DWORD dwFileSize=GetFileSize( (HANDLE)Handle, NULL );

    return (s32)dwFileSize;
}


#else
#error Designed for XBOX & PS2. Need system I/O calls for this platform
#endif
