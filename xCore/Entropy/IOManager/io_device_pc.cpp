//==============================================================================
//
// CDROM Defines, Buffers, Etc
//
//==============================================================================

#define CDROM_CACHE_SIZE    (32*1024)
#define CDROM_NUM_FILES     (16)                        // TODO: CJG - Increase this to accomodate multiple CDFS's
#define CDROM_INFO_SIZE     (32)
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

static xbool DeviceCDROMOpen( const char* pFilename, io_device_file* pFile )
{
    // Oops!
    return FALSE;
}

//==============================================================================

static void DeviceCDROMClose( io_device_file* pFile )
{
}

//==============================================================================

static xbool DeviceCDROMRead( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset, s32 AddressSpace )
{
	(void)AddressSpace;
    return FALSE;
}


