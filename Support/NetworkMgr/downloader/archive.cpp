//==============================================================================
//
//  Archive.cpp
//
//==============================================================================
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//  Not to be used without express permission of Inevitable Entertainment, Inc.
//
//==============================================================================

#include "x_files.hpp"
#include "archive.hpp"
#include "zlib/zlib.h"
#include "NetworkMgr/NetworkMgr.hpp"

archive::archive( void )
{
    m_Initialized = FALSE;
}

archive::~archive( void )
{
    ASSERT( m_Initialized == FALSE );
}

//----------------------------------------------------------
//----------------------------------------------------------
struct gzip_header
{
    u8  ID[2];
    u8  Method;
    u8  Flags;
    u8  Time[4];
    u8  ExtendedFlags;
    u8  OS;
};

static void* my_Alloc( voidpf Opaque, unsigned int items, unsigned int size )
{
    (void)Opaque;
    return x_malloc( items*size );
}

static void my_Free( voidpf Opaque, voidpf Ptr )
{
    (void)Opaque;
    x_free( Ptr );
}


//----------------------------------------------------------
xbool archive::Init( const byte* pData, s32 Length )
{
    (void)pData;
    (void)Length;
    ASSERT( m_Initialized == FALSE );
    archive_member Member;
    // Decompress archive.

    m_Initialized = TRUE;
    // Go through the archive, one entry at a time and expand it in to memory. This
    // will soon go directly to HDD.
    while( Length )
    {
        s32 err;

        const gzip_header& Header=(const gzip_header&)*pData;
        u16     Crc16;
        u32     OriginalSize;
        u32     CRC32;

        pData  += sizeof(Header);
        Length -= sizeof(Header);

        ASSERT( (Header.ID[0]==0x1f) && (Header.ID[1]==0x8b) );
        if( (Header.ID[0]!=0x1f) || (Header.ID[1]!=0x8b) )
        {
            Kill();
            return FALSE;
        }

        if( Header.Method!=8 )
        {
            Kill();
            return FALSE;
        }

        if( Length <=0 )
        {
            Kill();
            return FALSE;
        }

        x_memset(&Member,0,sizeof(Member));

        // Extra data present?
        if( Header.Flags & 0x04 )
        {
            u16 ExtraLength;
            ExtraLength = (*pData++ << 8) | (*pData++);

            pData+= ExtraLength;
            Length-= ExtraLength;
            ASSERT( Length>0 );
        }

        // Filename present?
        if( Header.Flags & 0x08 )
        {
            char* pFilename;

            pFilename = Member.Filename;

            while( *pData )
            {
                *pFilename++ = *pData++;
                Length--;
            }
            pData++;
            Length--;
            *pFilename = 0x0;
        }
        else
        {
            x_strcpy(Member.Filename,"Undefined.txt");
        }

        if( Header.Flags & 0x10 )
        {
            while( *pData++ )
            {
                Length--;
            }
        }

        // CRC16 present?
        if( Header.Flags & 0x02 )
        {
            Crc16 = (*pData++ << 8) | (*pData++);
            Length = Length-2;
        }
        else
        {
            Crc16 = 0;
        }

        if( Length < 0 )
        {
            Kill();
            return FALSE;
        }

        m_Stream.total_in   = 0;
        m_Stream.total_out  = 0;
        m_Stream.next_in    = (byte*)pData;
        m_Stream.avail_in   = Length;
        m_Stream.next_out   = NULL;
        m_Stream.avail_out  = 0;
        m_Stream.zalloc     = my_Alloc;
        m_Stream.zfree      = my_Free;
        m_Stream.opaque     = NULL;

        inflateInit2( &m_Stream,-MAX_WBITS );

        // Create file headers and data pointers
        xtimer t;

        t.Start();

        while( TRUE )
        {
            s32 BytesDecoded;
            m_Stream.avail_out = sizeof(m_Buffer);
            m_Stream.next_out  = m_Buffer;
            err = inflate( &m_Stream, Z_NO_FLUSH );

            BytesDecoded = sizeof(m_Buffer) - m_Stream.avail_out;

            if( BytesDecoded )
            {
                // x_fwrite( m_Buffer, m_Stream.avail_out - sizeof(m_Buffer), 1, m_Handle );
                Member.pData = (byte*)x_realloc( Member.pData, Member.Length+BytesDecoded );
                x_memcpy( Member.pData+Member.Length, m_Buffer, BytesDecoded );
                Member.Length += BytesDecoded;
            }
            if( err == Z_STREAM_END )
                break;
            ASSERT( err == Z_OK );
            if( err != Z_OK )
            {
                Kill();
                return FALSE;
            }
            if( t.ReadMs() > 100.0f )
            {
                LOG_MESSAGE( "archive::Init", "NetworkMgr update needed during decompress." );
                g_NetworkMgr.Update( t.ReadSec() );
                t.Reset();
                t.Start();
            }
        }
        err = inflateEnd( &m_Stream );
        ASSERT( err == Z_OK );
        pData       += m_Stream.total_in;
        Length      -= m_Stream.total_in;


        CRC32        = (pData[3]<<24)|(pData[2]<<16)|(pData[1]<<8)|pData[0];
        Length      -= 4;
        pData       += 4;

        OriginalSize = (pData[3]<<24)|(pData[2]<<16)|(pData[1]<<8)|pData[0];
        Length      -= 4;
        pData       += 4;

        ASSERT( OriginalSize == m_Stream.total_out );
        if( OriginalSize != m_Stream.total_out )
        {
            Kill();
            return FALSE;
        }
        m_Members.Append(Member);
    }

    return TRUE;
}

//----------------------------------------------------------
//----------------------------------------------------------
void archive::Kill( void )
{
    s32 i;

    ASSERT( m_Initialized );
    for( i=0; i< m_Members.GetCount(); i++ )
    {
        x_free(m_Members[i].pData);
    }
    m_Members.Clear();
    m_Initialized = FALSE;
}

//----------------------------------------------------------
s32 archive::GetMemberCount( void )
{
    ASSERT( m_Initialized );
    return m_Members.GetCount();
}

//----------------------------------------------------------
const char* archive::GetMemberFilename( s32 Index )
{
    ASSERT( m_Initialized );
    return m_Members[Index].Filename;
}

//----------------------------------------------------------
const byte* archive::GetMemberData( s32 Index )
{
    ASSERT( m_Initialized );
    return m_Members[Index].pData;
}

//----------------------------------------------------------
s32 archive::GetMemberLength( s32 Index )
{
    ASSERT( m_Initialized );
    return m_Members[Index].Length;
}
