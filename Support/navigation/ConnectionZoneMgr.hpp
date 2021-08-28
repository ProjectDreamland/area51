#ifndef __NAVIGATION_ZONE_MGR_HPP__
#define __NAVIGATION_ZONE_MGR_HPP__


#include "x_types.hpp"
#include "x_stdio.hpp"

class ng_connection2;
/*
 *

        Array of 
   'connection_zone_info'
    m_ConnectionZoneInfo
              ---  Index   ---
             |   |------->|   |         
              ---          ---     
             |   |\       |   |    ]     
              ---   \      ---     ]     m_ConnectionList
             |   |    \   |   |    ]     
              ---       \  ---          
               .         >|   |          
               .           ---           
               .          |   |          
               .           ---           
               .          |   |          
               .           ---           
              ---         |   |          
             |   |         ---           
              ---         |   |          
             |   |         ---           
              ---         |   |          
             |   |         ---           
              ---         |   |          
             |   |         ---           
              ---         |   |          
                           --- 

 *	
 */

class connection_zone_mgr
{
public:
    
    struct connection_zone_info
    {
        connection_zone_info();
        void Clear( void );
        u8  m_ZoneID;                   // Zone ID that this structure represents.
        u16 m_ConnectionZoneIndex;      // Index into the large array that stores the connection info.
        u16 m_Count;                    // Assumes less than 2^16 nodes per zone.
    };

public:
    connection_zone_mgr( void );
    virtual ~connection_zone_mgr( void );

            void                Reset                   ( void );

#ifdef X_EDITOR
            void                EditorPreComputeCounts  ( ng_connection2* pConnection, s32 nConnectionCount );
            void                WriteCountData          ( X_FILE* FilePointer );
            void                WriteListData           ( X_FILE* FilePointer );
            void                SetZoneMgrData          ( ng_connection2* pConnection, s32 nConnectionCount );
#endif // X_EDITOR

            void                LoadCountData           ( X_FILE* FilePointer );
            void                LoadListData            ( X_FILE* FilePointer );
            void                AllocateStorageForZoneMgr( s32  NumZonesWithConnections, s32 nConnectionsInList );
            u16*                GetConnectionListForZone( u8 ZoneID, u16& Count ); 

protected:

    s32                         m_ConnectionZoneInfoCount;
    connection_zone_info*       m_ConnectionZoneInfo;       // Keeps index into array and count of connections at that index.
    
    s32                         m_ConnectionIndexListCount;
    u16*                        m_ConnectionIndexList;      // What the above indexes into.
};


#endif