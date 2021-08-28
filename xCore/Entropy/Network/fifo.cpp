
#include "x_files.hpp"
#include "fifo.hpp"

//------------------------------------------------------------------------------
fifo::fifo( void )
{
    m_Initialized = FALSE;
}

//------------------------------------------------------------------------------
fifo::~fifo( void )
{
#ifndef TARGET_PC
    ASSERT( m_Initialized==FALSE );
#endif
}

//------------------------------------------------------------------------------
void fifo::Init( void* pBuffer, s32 Length )
{
    ASSERT( !m_Initialized );
    m_Initialized   = TRUE;
    m_pData         = (byte*)pBuffer;
    m_Length        = Length;
    m_WriteIndex    = 0;
    m_ReadIndex     = 0;
    m_ValidBytes    = 0;
}

//------------------------------------------------------------------------------
void fifo::Kill(void)
{
    ASSERT( m_Initialized );
    m_Initialized = FALSE;
}

//------------------------------------------------------------------------------
void fifo::Delete( s32 Amount )
{
    ASSERT( m_ValidBytes >= Amount );
    m_ReadIndex += Amount;
    if( m_ReadIndex > m_Length )
    {
        m_ReadIndex -= m_Length;
    }
    m_ValidBytes -= Amount;
}

//------------------------------------------------------------------------------
xbool fifo::Remove( void* pBuffer, s32 nBytes, s32 Modulo )
{
    (void)Modulo;

    s32 BytesToCopy;
    byte* pDest;

    (void)Modulo;
    
    ASSERT( Modulo > 0 );

    pDest = (byte*)pBuffer;

    if( m_ValidBytes < nBytes )
        return FALSE;

    while( nBytes )
    {
        BytesToCopy = MIN( nBytes,m_Length - m_ReadIndex );

        x_memcpy( pDest, m_pData+m_ReadIndex, BytesToCopy );
        pDest       += BytesToCopy;
        m_ReadIndex += BytesToCopy;
        m_ValidBytes-= BytesToCopy;
        nBytes      -= BytesToCopy;
        if( m_ReadIndex >= m_Length )
        {
            m_ReadIndex = 0;
        }

    }
    return TRUE;
}

//------------------------------------------------------------------------------
xbool fifo::Insert( const void* pBuffer, s32 Length, s32 Modulo )
{
    s32 BytesToCopy;
    const byte* pSource;

    ASSERT( Modulo > 0 );
    while( GetBytesFree() < Length )
    {
        Delete( Modulo );
    }
    pSource = (const byte*)pBuffer;

    while( Length )
    {
        BytesToCopy = MIN(Length,m_Length - m_WriteIndex);

        x_memcpy(m_pData+m_WriteIndex,pSource,BytesToCopy);
        pSource     += BytesToCopy;
        m_WriteIndex+= BytesToCopy;
        m_ValidBytes+= BytesToCopy;
        Length      -= BytesToCopy;
        if( m_WriteIndex >= m_Length )
        {
            m_WriteIndex = 0;
        }
    }
    return TRUE;
}

//------------------------------------------------------------------------------
s32 fifo::GetBytesFree(void)
{
    return m_Length - m_ValidBytes;
}

//------------------------------------------------------------------------------
s32     fifo::GetBytesUsed(void)
{
    return m_ValidBytes;
}

//------------------------------------------------------------------------------
void    fifo::Clear(void)
{
    m_WriteIndex = 0;
    m_ReadIndex  = 0;
    m_ValidBytes = 0;

}

//------------------------------------------------------------------------------
byte*   fifo::GetData(void)
{
    return m_pData;
}

//------------------------------------------------------------------------------
void fifo::ProvideUpdate( netstream& BitStream, s32 MaxLength, s32 Modulo )
{
    char    Buffer[256];
    s32     BytesAvailableInStream;
    s32     BytesAvailableInFifo;
    s32     Length;

    BitStream.WriteMarker();

    MaxLength = MIN( MaxLength, (s32)sizeof(Buffer) );

    MaxLength = MaxLength - ( MaxLength % Modulo );
    BytesAvailableInStream = MIN(BitStream.GetNBytesFree(), MaxLength );
    BytesAvailableInFifo   = GetBytesUsed();

    // Get maximum space we'd want in the buffer
    Length = MIN( BytesAvailableInFifo, BytesAvailableInStream );
    BitStream.WriteRangedS32(Length,0,255);

#if defined(TARGET_XBOX)
    s32     Cursor;
    Cursor = BitStream.GetCursor();
    // Align the data fields to a 16 bit boundary
    if( Cursor & 7 )
    {
        s32 Zero=0;

        BitStream.WriteBits( &Zero, 8-(Cursor&7) );
    }

    Cursor = BitStream.GetCursor();
    s32 ByteCursor = (Cursor/8)-2;

#endif

    if( Length )
    {
        LOG_MESSAGE("fifo::ProvideUpdate","Removed %d bytes from fifo",Length);
        Remove(Buffer, Length, Modulo);
        BitStream.WriteBits(Buffer,Length*8);
    }

    BitStream.WriteMarker();
#if defined(TARGET_XBOX)
    // The xbox requires the first two bytes within a VDP stream to contain the amount of data that is to be
    // encrypted. This has to be in little-endian format. The bitstream code deals with everything big-endian.
    // The way it *should* be. Screw this little-endian crap.
    ASSERT( ByteCursor >= 0 );
    ASSERT( (Cursor & 7)==0 );
    BitStream.SetCursor(0);
    BitStream.WriteU32( ByteCursor & 0xff, 8 );
    BitStream.WriteU32( (ByteCursor>>8) & 0xff, 8 );
    BitStream.SetCursor( Cursor );
#endif
}

//------------------------------------------------------------------------------
void fifo::AcceptUpdate( netstream& BitStream, s32 Modulo )
{
    char    Buffer[256];
    s32     Length;

    BitStream.ReadMarker();

    BitStream.ReadRangedS32(Length, 0, 255);
    if( Length > 0 )
    {
        LOG_MESSAGE("fifo::AcceptUpdate","Received %d bytes of data.",Length);
    }
#if defined(TARGET_XBOX)
    BitStream.SetCursor( (BitStream.GetCursor()+7)&~7 );
#endif
    BitStream.ReadBits(Buffer,Length*8);
    Insert( Buffer, Length, Modulo );

    BitStream.ReadMarker();

}