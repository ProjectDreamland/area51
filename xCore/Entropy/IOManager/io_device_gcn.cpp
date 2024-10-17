#include <dolphin.h>
#include <libsn.h>

//==============================================================================
//
// CDROM Defines, Buffers, Etc
//
//==============================================================================

#define CDROM_CACHE_SIZE    (32*1024)
#define CDROM_NUM_FILES     (32)                        
#define CDROM_INFO_SIZE     (sizeof(DVDFileInfo))
#define CDROM_CACHE         ((void*)s_CdromCache)
#define CDROM_FILES         ((void*)s_CdromFiles)
#define CDROM_BUFFER_ALIGN  (32)
#define CDROM_OFFSET_ALIGN  (4)
#define CDROM_LENGTH_ALIGN  (32)

static char           s_CdromCache[ CDROM_CACHE_SIZE ] GCN_ALIGNMENT(CDROM_BUFFER_ALIGN);
static io_device_file s_CdromFiles[ CDROM_NUM_FILES ];

//==============================================================================
//
// Device definitions.
//
//==============================================================================

io_device::device_data s_DeviceData[ io_mgr::NUM_DEVICES ] = {
//   Name               Id IsSupported IsReadable IsWriteable         CacheSize         BufferAlign         OffsetAlign         LengthAlign         NumFiles         InfoSize          pCache    pFilesBuffer
//======= ================ =========== ========== =========== ================= =================== =================== =================== ================ ================ =============== ===============
{ "CDROM",   io_mgr::CDROM,       TRUE,      TRUE,      FALSE, CDROM_CACHE_SIZE, CDROM_BUFFER_ALIGN, CDROM_OFFSET_ALIGN, CDROM_LENGTH_ALIGN, CDROM_NUM_FILES, CDROM_INFO_SIZE,    CDROM_CACHE,    CDROM_FILES },
};

//==============================================================================

static void gcn_DVDCallback( s32 Result, DVDFileInfo* pFileInfo )
{
    (void)pFileInfo;

    // We are in the callback
    s_CallbackActive++;

    // Success?
    if( Result >= 0 )
    {
        // Its all good!
        ProcessEndOfRequest( io_mgr::CDROM, io_request::COMPLETED );
    }
    else
    {
        // Ack failed!
        ProcessEndOfRequest( io_mgr::CDROM, io_request::FAILED );
    }

    // Done with callback
    s_CallbackActive--;
}

//==============================================================================

static void gcn_ConvertPath( char* pClean, char* pFilename )
{
    while( *pFilename )
    {
        if( (*pFilename == '\\') || (*pFilename == '/') )
        {
            *pClean++ = '/';
            pFilename++;

            while( *pFilename && ((*pFilename == '\\') || (*pFilename == '/')) )
                pFilename++;
        }
        else
        {
            *pClean++ = *pFilename++;
        }
    }

    *pClean = 0;
}

#ifdef TARGET_GCN_DEV

// TODO: Set FileSystemOffset!

//==============================================================================

static xbool DeviceCDROMOpen( const char* pFilename, io_device_file* pFile )
{
    if( IsDevLink() )
    {
        // Open the file
        pFile->Handle=(void*)(PCopen( (char *)pFilename, 0, 0 ) + 1);

        if( pFile->Handle )
        {
            pFile->Length = PClseek((int)pFile->Handle-1,0,X_SEEK_END);
            PClseek((int)pFile->Handle-1,0,X_SEEK_SET);
            return TRUE;
        }
        else
        {
            // Oops!
            return FALSE;
        }
    }
    else
    {
        char CleanFile[256];
        gcn_ConvertPath( CleanFile, (char *)pFilename );
        if( DVDOpen( (char *)CleanFile, (DVDFileInfo*)pFile->pDeviceInfo ) )
        {
            // Set the handle.
            pFile->Handle = pFile->pDeviceInfo;

            // Get the length of the file.
            pFile->Length = DVDGetLength( (DVDFileInfo*)pFile->pDeviceInfo );

            // It's all good!
            return TRUE;
        }
        else
        {
            // Oops!
            return FALSE;
        }
    }
}

//==============================================================================

static void DeviceCDROMClose( io_device_file* pFile )
{
    if( IsDevLink() )
    {
        PCclose( (int)pFile->Handle-1 );
    }
    else
    {
        DVDClose( (DVDFileInfo*)pFile->pDeviceInfo );
    }
}

//==============================================================================

#ifdef IO_FS_LOGGING_ENABLED
io_device_file* g_pCurrentFile      = NULL;
s32             g_CurrentSeekOffset = -1;
s32             g_LastSeekLength    = 0;
s32             g_nSeekTrack        = 0;           
#endif
                
//==============================================================================

static xbool DeviceCDROMRead( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset,s32 AddressSpace )
{
    xbool Success;
    (void)AddressSpace;

    // Just to make for certain we are NOT trying to transfer directly to vm space.
    ASSERTS( ((u32)pFile->pBuffer >> 24) == 0x80,"Attempt to do a device read direct to virtual memory");

#ifdef IO_FS_LOGGING_ENABLED    
    // Seek log enabled?
    if( g_IOFSMgr.LogEnabled( IO_FS_LOG_SEEK ) )
    {
        // Same file?
        if( g_pCurrentFile != pFile )
        {
            g_IOFSMgr.Log( IO_FS_LOG_SEEK, xfs( "Different File: %s", pFile->Filename ) );
            g_nSeekTrack++;
        }
        else if( Offset != (g_CurrentSeekOffset+g_LastSeekLength) )
        {
            g_IOFSMgr.Log( IO_FS_LOG_SEEK, xfs("Prev Offset: %d, Len: %d, Curr Offset: %d, Len: %d, Delta %d", g_CurrentSeekOffset, g_LastSeekLength, Offset, Length, Offset-g_CurrentSeekOffset) );
            g_nSeekTrack++;
        }

        g_pCurrentFile      = pFile;
        g_CurrentSeekOffset = Offset;
        g_LastSeekLength    = Length;
    }

    // Read log enabled?
    if( g_IOFSMgr.LogEnabled( IO_FS_LOG_READ ) )
    {
        g_IOFSMgr.Log( IO_FS_LOG_READ, xfs( "File:%s, Offset=%d, Length=%d", pFile->Filename, Offset, Length ) );
    }
#endif

    if( IsDevLink() )
    {
        Success = TRUE;
        PClseek( (int)pFile->Handle-1, Offset, 0 );
        if( Length != PCread( (int)pFile->Handle-1, (char*)pBuffer, Length ) )
        {
            Success = FALSE;
        }
        else
        {
            Success = TRUE;
        }
        gcn_DVDCallback( 1, (DVDFileInfo*)pFile->pDeviceInfo );
    }
    else
    {
        Success = DVDReadAsync( (DVDFileInfo*)pFile->pDeviceInfo, pBuffer, Length, Offset, gcn_DVDCallback );
    }

    return Success;
}

#endif // TARGET_GCN_DEV

#ifdef TARGET_GCN_DVD

//==============================================================================

static xbool DeviceCDROMOpen( const char* pFilename, io_device_file* pFile )
{
    // Get the system file handle
    char CleanFile[256];
    gcn_ConvertPath( CleanFile, (char *)pFilename );
    if( DVDOpen( (char *)CleanFile, (DVDFileInfo*)pFile->pDeviceInfo ) )
    {
        // Set the handle.
        pFile->Handle = pFile->pDeviceInfo;

        // Get the length of the file.
        pFile->Length = DVDGetLength( (DVDFileInfo*)pFile->pDeviceInfo );
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//==============================================================================

static void DeviceCDROMClose( io_device_file* pFile )
{
    (void)pFile;
}

//==============================================================================

static xbool DeviceCDROMRead( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset, s32 AddressSpace )
{
    xbool Success;
    (void)AddressSpace;

    Success = DVDReadAsync( (DVDFileInfo*)pFile->pDeviceInfo, pBuffer, Length, Offset, gcn_DVDCallback );

    return Success;
}

#endif // TARGET_GCN_DVD

