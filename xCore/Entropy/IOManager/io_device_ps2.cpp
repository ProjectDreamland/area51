//==============================================================================
//
// CDROM Defines, Buffers, Etc
//
//==============================================================================
#include "ps2/filemanager.hpp"
#include "ps2/fileio.hpp"

#define CDROM_CACHE_SIZE    (32*1024)
#define CDROM_NUM_FILES     (16)                        // TODO: CJG - Increase this to accomodate multiple CDFS's
#define CDROM_INFO_SIZE     (32)
#define CDROM_CACHE         ((void*)s_CdromCache)
#define CDROM_FILES         ((void*)s_CdromFiles)
#define CDROM_BUFFER_ALIGN  (32)
#define CDROM_OFFSET_ALIGN  (0)
#define CDROM_LENGTH_ALIGN  (0)

static char           s_CdromCache[ CDROM_CACHE_SIZE ] PS2_ALIGNMENT(CDROM_BUFFER_ALIGN);
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


static void ps2_ConvertPath( char* pClean, char* pFilename )
{
    // Put the correct prefix in front of the filename depending on whether or not
    // we're running from dvd or the devkit
#if defined(TARGET_PS2_DVD)
    x_strcpy(pClean,"cdrom0:\\");
#else
    x_strcpy(pClean,"host:C:\\gamedata\\meridian\\resources\\ps2\\");
#endif
    pClean = pClean+x_strlen(pClean);

    // Now copy all characters, changing '/' or '\' in to '\'
    while( *pFilename )
    {
        if( (*pFilename == '\\') || (*pFilename == '/') )
        {
            *pClean++ = '\\';
            pFilename++;

            while( *pFilename && ((*pFilename == '\\') || (*pFilename == '/')) )
                pFilename++;
        }
        else
        {
#if defined(TARGET_PS2_DVD)
            *pClean++ = x_toupper(*pFilename++);
#else
            *pClean++ = *pFilename++;
#endif
        }
    }

#if defined(TARGET_PS2_DVD)
    *pClean++=';';
    *pClean++='1';
#endif
    *pClean++=0x0;
}

//==============================================================================

static xbool DeviceCDROMOpen( const char* pFilename, io_device_file* pFile )
{

    char CleanFile[256];
    ps2_ConvertPath( CleanFile, (char *)pFilename );

    // Now we open the file using the iop file manager
    pFile->pDeviceInfo = (void*)g_FileManager.Open(CleanFile,FIO_READ);
    if ((s32)pFile->pDeviceInfo <=0)
        return FALSE;

    pFile->Length = g_FileManager.Length((s32)pFile->pDeviceInfo);
    return TRUE;
}

//==============================================================================

static void DeviceCDROMClose( io_device_file* pFile )
{
    g_FileManager.Close((s32)pFile->pDeviceInfo);
}

//==============================================================================

static xbool DeviceCDROMRead( io_device_file* pFile, void* pBuffer, s32 Length, s32 Offset, s32 AddressSpace )
{
    s32 ReadLength;
    fileio_address_type addrtype;


    if (AddressSpace)
    {
        addrtype = FIO_ADDR_AUDIO;
    }
    else
    {
        addrtype = FIO_ADDR_MAIN;
    }

    ReadLength = g_FileManager.Read((s32)pFile->pDeviceInfo,pBuffer,Length,Offset,addrtype);
    s_CallbackActive++;
    if( ReadLength == Length )
    {
        // Its all good!
        ProcessEndOfRequest( io_mgr::CDROM, io_request::COMPLETED );
    }
    else
    {
        // Ack failed!
        ProcessEndOfRequest( io_mgr::CDROM, io_request::FAILED );
    }
    s_CallbackActive--;

    return ReadLength == Length;
}
