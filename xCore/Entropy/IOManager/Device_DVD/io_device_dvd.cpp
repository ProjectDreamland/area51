#include "..\io_mgr.hpp"
#include "..\io_filesystem.hpp"
#include "io_device_dvd.hpp"
#include "x_log.hpp"

//#define LOG_PHYSICAL_READ  "io_device_dvd::PhysicalRead(read)"
//#define LOG_PHYSICAL_SEEK  "io_device_dvd::PhysicalRead(seek)"
//#define LOG_PHYSICAL_WRITE "io_device_dvd::PhysicalWrite"

//==============================================================================

io_device_dvd g_IODeviceDVD;

//==============================================================================

io_device_dvd::io_device_dvd( void )
{
    m_pLastFile  = NULL;
    m_LastOffset = -1;
    m_LastLength = 0;
    m_nSeeks     = 0;           
}

//==============================================================================

io_device_dvd::~io_device_dvd( void )
{
}

//==============================================================================
void io_device_dvd::LogPhysRead( io_device_file* pFile, s32 Length, s32 Offset )
{
    (void)pFile;
    (void)Length;
    (void)Offset;

#ifdef LOG_PHYSICAL_SEEK
    // Same file?
    if( m_pLastFile != pFile )
    {
        LOG_MESSAGE( LOG_PHYSICAL_SEEK, "SEEK! Different File: %s", pFile->Filename );
        m_nSeeks++;
    }
    // Need to seek?
    else if( Offset != (m_LastOffset+m_LastLength) )
    {
        LOG_MESSAGE( LOG_PHYSICAL_SEEK, "SEEK! Prev Offset: %d, Len: %d, Curr Offset: %d, Len: %d, Delta %d", m_LastOffset, m_LastLength, Offset, Length, Offset-m_LastOffset );
        m_nSeeks++;
    }

    // Update 'em
    m_pLastFile  = pFile;
    m_LastOffset = Offset;
    m_LastLength = Length;
#endif // LOG_PHYSICAL_SEEK

#ifdef LOG_PHYSICAL_READ
    LOG_MESSAGE( LOG_PHYSICAL_READ, "READ! File: %s, Offset: %d, Length: %d", pFile->Filename, Offset, Length );
#endif // LOG_PHYSICAL_READ
}

//==============================================================================

void io_device_dvd::LogPhysWrite( io_device_file* pFile, s32 Length, s32 Offset )
{
    (void)pFile;
    (void)Length;
    (void)Offset;

#ifdef LOG_PHYSICAL_WRITE
    LOG_MESSAGE( LOG_PHYSICAL_WRITE, "WRITE! File: %s, Offset: %d, Length: %d", pFile->Filename, Offset, Length );
#endif
}


