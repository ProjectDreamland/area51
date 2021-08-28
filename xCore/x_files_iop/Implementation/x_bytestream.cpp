//==============================================================================
//
//  x_bytestream.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_BYTESTREAM_HPP
#include "../x_bytestream.hpp"
#endif

#ifndef X_STDIO_HPP
#include "../x_stdio.hpp"
#endif

#ifndef X_MEMORY_HPP
#include "../x_memory.hpp"
#endif

#ifndef X_DEBUG_HPP
#include "../x_debug.hpp"
#endif

#ifndef X_FILES_PRIVATE_HPP
#include "x_files_private.hpp"
#endif

//==============================================================================
//  DEFINES
//==============================================================================
//
#define BUFFER_SIZE    (*(((s32*)m_pData) - 2))
#define STREAM_LENGTH  (*(((s32*)m_pData) - 1))

//==============================================================================
//  IMPLEMENTATION NOTES NOTES FOR CLASS 'xbytestream'
//==============================================================================
//
//  In order to get an xbytestream to be as compatible as possible with byte*,
//  the only class member field is the data pointer.  The other information 
//  needed to make the class work, specifically the "buffer size" and the 
//  "stream length" values, have been moved down into the dynamic buffer.
//  
//      xstream
//      +-----------+
//      | m_pData---=-----------------------------+
//      | (byte*)   |                             |
//      +-----------+                             |
//                                                V
//                  +--------------------------------------------------------+
//                  | BufferSize | StreamLength | stream byte data...        |
//                  | (s32)      | (s32)        | (byte[])                   |
//                  +--------------------------------------------------------+
//  
//  The macros BUFFER_SIZE and STREAM_LENGTH fetch the appropriate values from
//  the m_pData pointer.
//  
//==============================================================================

//==============================================================================
//  MEMBER FUNCTIONS FOR CLASS 'xbytestream'
//==============================================================================

void xbytestream::EnsureCapacity( s32 Capacity )
{
    s32  NewBufferSize;

    // Since we store the buffer size and stream length in the buffer,
    // it must be at least 8 bytes larger than the size of the requested capacity.
    // Furthermore, in order to reduce heap activity, the allocation is rounded
    // up to a multiple of 16.

    NewBufferSize = ALIGN_16( Capacity + 8 );
    Capacity      = NewBufferSize - 8;

    if( (NewBufferSize > BUFFER_SIZE) || (Capacity > STREAM_LENGTH) )
    {
        // Careful!  We must back the pointer off by 8, and then advance by 8.
        m_pData = (byte*)x_realloc( m_pData - 8, NewBufferSize ) + 8;
        ASSERT( m_pData-8 );
        BUFFER_SIZE = NewBufferSize;
    }
}

//==============================================================================

xbytestream::xbytestream( void )
{
    m_pData = (byte*)x_malloc( 16 ) + 8;
    ASSERT( m_pData-8 );

    BUFFER_SIZE   = 16;
    STREAM_LENGTH =  0;
}

//==============================================================================

xbytestream::xbytestream( const byte* pData, s32 Count )
{
    ASSERT( pData );

    // See comments in EnsureCapacity for why we added 8 and aligned to 16.
    s32 BufferSize = ALIGN_16( Count + 8 );

    m_pData = (byte*)x_malloc( BufferSize ) + 8;
    ASSERT( m_pData-8 );

    BUFFER_SIZE   = BufferSize;
    STREAM_LENGTH = Count;
    x_memcpy( m_pData, pData, Count );
}

//==============================================================================

xbytestream::xbytestream( const xbytestream& Bytestream )
{
    // See comments in EnsureCapacity for why we added 8 and aligned to 16.
    s32 Count      = Bytestream.GetLength();
    s32 BufferSize = ALIGN_16( Count + 8 );

    m_pData = (byte*)x_malloc( BufferSize ) + 8;
    ASSERT( m_pData-8 );

    BUFFER_SIZE   = BufferSize;
    STREAM_LENGTH = Count;
    x_memcpy( m_pData, Bytestream.m_pData, Count );
}

//==============================================================================

xbytestream::~xbytestream( void )
{
    // Careful!  We must back the pointer off by 8.
    if( m_pData )   x_free( m_pData - 8 );
}

//==============================================================================

s32 xbytestream::GetLength( void ) const
{
    return STREAM_LENGTH;
}

//==============================================================================

void xbytestream::Clear( void )
{
    STREAM_LENGTH = 0;
}

//==============================================================================

void xbytestream::FreeExtra( void )
{
    s32 Size = ALIGN_16( STREAM_LENGTH+9 );
    if( Size > BUFFER_SIZE )
    {
        // Careful!  We must back the pointer off by 8, and then advance by 8.
        m_pData = (byte*)x_realloc( m_pData - 8, Size ) + 8;
        ASSERT( m_pData-8 );
        BUFFER_SIZE = Size;
    }
}

//==============================================================================

byte xbytestream::GetAt( s32 Index ) const
{
    ASSERT( Index >= 0 );
    ASSERT( Index <  STREAM_LENGTH );

    return m_pData[Index];
}

//==============================================================================

void xbytestream::SetAt( s32 Index, byte Data )
{
    ASSERT( Index >= 0 );
    ASSERT( Index <  STREAM_LENGTH );

    m_pData[Index] = Data;
}

//==============================================================================

byte* xbytestream::GetBuffer( void ) const
{
    return m_pData;
}

//==============================================================================

void xbytestream::Insert( s32 Index, byte Data )
{
    ASSERT( Index >= 0 );
    ASSERT( Index <= STREAM_LENGTH );

    // Make sure there is enough buffer capacity.  We need room for the current 
    // value of the stream and the new byte
    EnsureCapacity( STREAM_LENGTH + 1 );

    // Make space for insertion.
    x_memmove( &m_pData[ Index + 1], 
               &m_pData[ Index ], 
               (STREAM_LENGTH - Index) );

    // Increase length appropriately.
    STREAM_LENGTH += 1;

    // Insert the byte
    m_pData[Index] = Data;
}

//==============================================================================

void xbytestream::Insert( s32 Index, const byte* pData, s32 Count )
{
    ASSERT( Index >= 0 );
    ASSERT( Index <= STREAM_LENGTH );
    ASSERT( pData );

    // Make sure there is enough buffer capacity.  We need room for the current 
    // value of the stream and the new stream.
    EnsureCapacity( STREAM_LENGTH + Count );

    // Make space for insertion.
    x_memmove( &m_pData[ Index + Count ], 
               &m_pData[ Index ], 
               (STREAM_LENGTH - Index) );

    // Increase length appropriately.
    STREAM_LENGTH += Count;

    // Insert the stream.
    x_memcpy( &m_pData[Index], pData, Count );
}

//==============================================================================

void xbytestream::Insert( s32 Index, const xbytestream& Bytestream )
{
    ASSERT( Index >= 0 );
    ASSERT( Index <= STREAM_LENGTH );
    ASSERT( this  != &Bytestream );     // Cannot insert an xbytestream into itself!

    s32 Count = Bytestream.GetLength();

    // Make sure there is enough buffer capacity.  We need room for the current 
    // value of the stream and the new stream.
    EnsureCapacity( STREAM_LENGTH + Count );

    // Make space for insertion.
    x_memmove( &m_pData[ Index + Count ], 
               &m_pData[ Index ], 
               (STREAM_LENGTH - Index) );

    // Increase length appropriately.
    STREAM_LENGTH += Count;

    // Insert the stream.
    x_memcpy( m_pData+Index, Bytestream.m_pData, Count );
}

//==============================================================================

void xbytestream::Delete( s32 Index, s32 Count )
{
    ASSERT( Count >= 0 );
    ASSERT( Index >= 0 );
    ASSERT( Index <= STREAM_LENGTH );
    ASSERT( (Index+Count) <= STREAM_LENGTH );

    // Move the "end" of the stream "over" the stuff to be deleted.
    x_memmove( &m_pData[ Index ], 
               &m_pData[ Index + Count ], 
               (STREAM_LENGTH - (Index + Count) ) );

    // Adjust length.
    STREAM_LENGTH -= Count;
}

//==============================================================================

void xbytestream::Append( byte Data )
{
    Insert( STREAM_LENGTH, Data );
}

//==============================================================================

void xbytestream::Append( const byte* pData, s32 Count )
{
    Insert( STREAM_LENGTH, pData, Count );
}

//==============================================================================

void xbytestream::Append( const xbytestream& Bytestream )
{
    Insert( STREAM_LENGTH, Bytestream );
}

//==============================================================================

s32 xbytestream::Find( byte Data, s32 StartIndex ) const
{
    ASSERT( StartIndex >= 0 );
    ASSERT( StartIndex <= STREAM_LENGTH );

    byte* pScan = m_pData + StartIndex;
    byte* pEnd  = m_pData + STREAM_LENGTH;

    while( pScan != pEnd )
    {
        if( *pScan == Data )
            return( pScan - m_pData );
        pScan++;
    }

    return( -1 );
}

//==============================================================================

s32 xbytestream::Find( const xbytestream& Pattern, s32 StartIndex ) const
{
    ASSERT( StartIndex >= 0 );
    ASSERT( StartIndex <= STREAM_LENGTH );

    // If the pattern is larger than the bytestream to be searched, then there is
    // no chance to succeed.

    s32 PatternLen = Pattern.GetLength();

    if( PatternLen > STREAM_LENGTH )
        return( -1 );

    byte* pScan = m_pData + StartIndex;
    byte* pEnd  = m_pData + STREAM_LENGTH - PatternLen + 1;

    while( pScan != pEnd )
    {
        // We must compare the pattern against the current scan point.

        s32   i;
        byte* p1 = pScan;
        byte* p2 = Pattern.m_pData;

        for( i = 0; i < PatternLen; i++ )
        {
            if( *p1 != *p2 )
                break;

            p1++;
            p2++;
        }

        // See how far we got on that last comparison run.  If we made it the 
        // full length of the pattern, then we have a winner!

        if( i == PatternLen )
        {
            return( pScan - m_pData );
        }

        pScan++;
    }

    return( -1 );
}

//==============================================================================

const xbytestream& xbytestream::operator = ( byte Data )
{
    EnsureCapacity( 1 );
    m_pData[0]    = Data;
    STREAM_LENGTH = 1;

    return( *this );
}

//==============================================================================

const xbytestream& xbytestream::operator = ( const xbytestream& Bytestream )
{
    if( this != &Bytestream )
    {
        s32 Len = Bytestream.GetLength();
        EnsureCapacity( Len );
        x_memcpy( m_pData, Bytestream.m_pData, Len );
        STREAM_LENGTH = Len;
    }

    return( *this );
}

//==============================================================================

const xbytestream& xbytestream::operator += ( const xbytestream& Bytestream )
{
    s32 Len = Bytestream.GetLength();
    EnsureCapacity( STREAM_LENGTH + Len );
    x_memcpy( m_pData + STREAM_LENGTH, Bytestream.m_pData, Len );
    STREAM_LENGTH += Len;

    return( *this );
}

//==============================================================================

const xbytestream& xbytestream::operator += ( byte Data )
{
    EnsureCapacity( STREAM_LENGTH + 1 );
    m_pData[STREAM_LENGTH] = Data;
    STREAM_LENGTH += 1;

    return( *this );
}

//==============================================================================

xbool xbytestream::LoadFile( const char* pFileName )
{
    s32     Size;
    s32     Read;

    X_FILE* pFile = x_fopen( pFileName, "rb" );

    if( !pFile )
        return( FALSE );

    Size = x_flength( pFile );
    EnsureCapacity( Size );
    Read = x_fread( m_pData, 1, Size, pFile );
    STREAM_LENGTH = Read;

    x_fclose( pFile );

    return( Read == Size );
}

//==============================================================================

xbool xbytestream::SaveFile( const char* pFileName ) const
{
    s32     Written;

    X_FILE* pFile = x_fopen( pFileName, "wb" );

    if( !pFile )
        return( FALSE );

    Written = x_fwrite( m_pData, 1, STREAM_LENGTH, pFile );
    x_fclose( pFile );

    return( Written == STREAM_LENGTH );
}

//==============================================================================
