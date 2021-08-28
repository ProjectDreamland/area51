#include "ConnectionZoneMgr.hpp"
#include "ng_connection2.hpp"
#include "..\ZoneMgr\ZoneMgr.hpp"
//===========================================================================
//==============================================================================
// SimpleCompare
//==============================================================================
s32 SimpleCompare(
    const void* pItem1,
    const void* pItem2 )
{
    // Sort by lowest pixel count to biggest pixel count
    if ( (u16)pItem1 > (u16) pItem2 )
        return 1 ;
    else
    if ( (u16)pItem1 < (u16) pItem2 )
        return -1 ;
    else
        return 0 ;
}

//===========================================================================

connection_zone_mgr::connection_zone_mgr() :
    m_ConnectionZoneInfoCount(0),
    m_ConnectionZoneInfo( NULL ),
    m_ConnectionIndexListCount(0),
    m_ConnectionIndexList(NULL)    
{
}

//===========================================================================

connection_zone_mgr::connection_zone_info::connection_zone_info()
{
    Clear();
}

//===========================================================================

void connection_zone_mgr::connection_zone_info::Clear( void )
{
    m_ConnectionZoneIndex = 0xffff;
    m_Count = 0xffff;
    m_ZoneID = 0xff;
}

//===========================================================================

connection_zone_mgr::~connection_zone_mgr( void )
{
    Reset();
}

//===========================================================================

void connection_zone_mgr::Reset( void )
{
    if ( m_ConnectionZoneInfo )
    {
        delete[] m_ConnectionZoneInfo;
        m_ConnectionZoneInfo = NULL;
    }

    if ( m_ConnectionIndexList )
    {
        delete[] m_ConnectionIndexList;
        m_ConnectionIndexList = NULL;
    }
}

#ifdef X_EDITOR

//===========================================================================

void connection_zone_mgr::EditorPreComputeCounts( ng_connection2* pConnection, s32 nConnectionCount )
{
    s32 i = 0;
    xbool* bZonesWithConnections = NULL;
    s32 NumZonesWithConnections = 0;
        
    if ( g_ZoneMgr.GetZoneCount() > 0 )
    {
        bZonesWithConnections = new xbool[ g_ZoneMgr.GetZoneCount() ];
        x_memset( bZonesWithConnections, 0, sizeof( xbool ) * g_ZoneMgr.GetZoneCount() );
    }
    else
    {
        bZonesWithConnections = new xbool[ 2 ];
        NumZonesWithConnections = 1;

        // No zones, we're done.
 //       return;
    }
    
    s32    nConnectionsInList = 0;      // This is the total number of connections in the list.  There can be repeats,
                                        // these are counted as many times as they are repeated.

    for ( i = 0; i < nConnectionCount; i++ )
    {
        // Fill bZonesWithConnections and get nConnectionsInList
        pConnection[i].EditorGetZoneAndCount( bZonesWithConnections, nConnectionsInList );
    }

    // Now determine how many zones are actually used.
    for ( i = 0; i < g_ZoneMgr.GetZoneCount(); i++ )
    {
        if( bZonesWithConnections[i] )
        {
            NumZonesWithConnections++;
        }
    }

    // We have the counts, now allocate the memory.
    m_ConnectionZoneInfoCount = NumZonesWithConnections;
    m_ConnectionIndexListCount = nConnectionsInList;

    AllocateStorageForZoneMgr( m_ConnectionZoneInfoCount, m_ConnectionIndexListCount );

    // Counts have been computed, memory has been allocated.  We're done.  Cleanup.

    if ( bZonesWithConnections )
    {
        delete[] bZonesWithConnections;
    }
}

//===========================================================================

void connection_zone_mgr::WriteCountData( X_FILE* filePointer )
{
    s32 writeSize = 0;
    ASSERT(filePointer);

    writeSize = x_fwrite( &m_ConnectionZoneInfoCount, sizeof(m_ConnectionZoneInfoCount), 1,  filePointer );
    ASSERT(writeSize == 1 );

    writeSize = x_fwrite( &m_ConnectionIndexListCount, sizeof(m_ConnectionIndexListCount), 1, filePointer );
    ASSERT( writeSize == 1 );
}

//===========================================================================

void connection_zone_mgr::WriteListData( X_FILE* filePointer )
{
    s32 writeSize = 0;
    ASSERT(filePointer);

    writeSize = x_fwrite( &( m_ConnectionZoneInfo[0] ), sizeof(connection_zone_info), m_ConnectionZoneInfoCount, filePointer );
    ASSERT( writeSize == m_ConnectionZoneInfoCount );

    writeSize = x_fwrite( &(m_ConnectionIndexList[0]),   sizeof(u16), m_ConnectionIndexListCount, filePointer );
    ASSERT( writeSize == m_ConnectionIndexListCount );
}

//===========================================================================

void connection_zone_mgr::SetZoneMgrData( ng_connection2* pConnection, s32 nConnectionCount  )
{
    // Memory had better be allocated by this point.
    ASSERT( m_ConnectionZoneInfo != NULL );
    ASSERT( m_ConnectionIndexList != NULL );
    
    if ( m_ConnectionZoneInfo == NULL || m_ConnectionIndexList == NULL )
        return;
    
    s32 i = 0;

    xarray<u16> TempZoneList[256];
    for ( i = 0; i < nConnectionCount; i++ )
    {
        pConnection[i].FindZonesSpanned( TempZoneList, i );
    }

    // Now I have to sort my xarrays and eliminate duplicates.
    for ( i = 0; i < 256; i++ )
    {
        x_qsort( &TempZoneList[i], TempZoneList[i].GetCount(), sizeof( u16 ), SimpleCompare );

        for ( s32 j = 1; j < TempZoneList[i].GetCount(); j++ )
        {
            if ( TempZoneList[i][j-1] == TempZoneList[i][j] )
            {
                TempZoneList[i].Delete( j );
            }
        }
    }

    // OK.  My xarrays are ready to copy into my already allocated arrays.
    s32 k = 0;
    s32 CurrentZoneIndex = 0;
    s32 CurrentListIndex = 0;
    for ( i = 0; i < 256; i++ )
    {
        if ( TempZoneList[i].GetCount() > 0 )
        {
            m_ConnectionZoneInfo[CurrentZoneIndex].m_ZoneID = i;
            m_ConnectionZoneInfo[CurrentZoneIndex].m_Count = TempZoneList[i].GetCount();
            m_ConnectionZoneInfo[CurrentZoneIndex].m_ConnectionZoneIndex = CurrentListIndex;
            CurrentZoneIndex++;


            for ( k = 0; k < TempZoneList[i].GetCount(); k++ )
            {
                m_ConnectionIndexList[ CurrentListIndex ] = TempZoneList[i][k];
                ASSERT( CurrentListIndex < m_ConnectionIndexListCount );
                CurrentListIndex++;
            }
        }
    }
}


#endif // X_EDITOR

//===========================================================================

void connection_zone_mgr::LoadCountData( X_FILE* filePointer )
{
    ASSERT(filePointer);
    s32 ConnectionZoneInfoCount, ConnectionIndexListCount;

    s32 readSize;
    readSize = x_fread( &ConnectionZoneInfoCount, sizeof(ConnectionZoneInfoCount), 1, filePointer );
    ASSERT(readSize == 1);

    readSize = x_fread( &ConnectionIndexListCount, sizeof(ConnectionIndexListCount), 1, filePointer );
    ASSERT( readSize == 1 );

    AllocateStorageForZoneMgr( ConnectionZoneInfoCount, ConnectionIndexListCount );
}

//===========================================================================

void connection_zone_mgr::LoadListData( X_FILE* filePointer )
{
    ASSERT(filePointer);
    s32 readSize;

    //  Time to read in the actual data in 3 big chunks
    readSize = x_fread( &(m_ConnectionZoneInfo[0]), sizeof(connection_zone_info), m_ConnectionZoneInfoCount, filePointer );
    ASSERT( readSize == m_ConnectionZoneInfoCount );

    readSize = x_fread( &(m_ConnectionIndexList[0]), sizeof(u16), m_ConnectionIndexListCount, filePointer );
    ASSERT( readSize == m_ConnectionIndexListCount );

}

//===========================================================================

void connection_zone_mgr::AllocateStorageForZoneMgr( s32 ZonesWithConnections, s32 ConnectionsInList )
{
    ASSERT( m_ConnectionZoneInfo == NULL );
    ASSERT( m_ConnectionIndexList == NULL );

    m_ConnectionIndexListCount = ConnectionsInList;
    m_ConnectionZoneInfoCount = ZonesWithConnections;
    
    // Allocate the space.
    m_ConnectionZoneInfo = new connection_zone_info[ m_ConnectionZoneInfoCount ];
    m_ConnectionIndexList = new u16[ m_ConnectionIndexListCount ];
}

//===========================================================================

u16* connection_zone_mgr::GetConnectionListForZone( u8 ZoneID, u16& Count )
{
    u16* retVal = NULL;
    for ( s32 i = 0; i < m_ConnectionZoneInfoCount; i++ )
    {
        if ( m_ConnectionZoneInfo[i].m_ZoneID == ZoneID )
        {
            retVal = &m_ConnectionIndexList[m_ConnectionZoneInfo[i].m_ConnectionZoneIndex];
//            pList = &m_ConnectionIndexList[m_ConnectionZoneInfo[i].m_ConnectionZoneIndex];
            Count = m_ConnectionZoneInfo[i].m_Count;            
//            return TRUE;
        }
    }
    return retVal;

//    return FALSE;
}