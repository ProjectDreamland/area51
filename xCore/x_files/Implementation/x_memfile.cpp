//==============================================================================
//
//  x_memfile.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "../x_memfile.hpp"
#include "../x_files.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
//  MEMBER FUNCTIONS FOR CLASS 'xmemfile'
//==============================================================================

void xmemfile::EnsureCapacity( s32 Capacity )
{
    if( Capacity > m_Capacity )
    {
        s32 NewCapacity = m_Capacity;

        if( NewCapacity == 0 )
            NewCapacity = 1024;

        // Double capacity until it exceeds required
        while( NewCapacity < Capacity )
            NewCapacity *= 2;

        // Realloc the buffer
        m_pData = (byte*)x_realloc( m_pData, NewCapacity );
        ASSERT( m_pData );
        m_Capacity = NewCapacity;
    }
}

//==============================================================================

xmemfile::xmemfile( void )
{
    m_pData     = 0;
    m_Capacity  = 0;
    m_Position  = 0;
    m_EOF       = 0;
}

//==============================================================================

xmemfile::xmemfile( const xmemfile& MemFile )
{
    m_pData = (byte*)x_malloc( MemFile.m_Capacity );
    ASSERT( m_pData );
    m_Capacity  = MemFile.m_Capacity;
    m_Position  = MemFile.m_Position;
    m_EOF       = MemFile.m_EOF;
    x_memcpy( m_pData, MemFile.m_pData, m_Capacity );
}

//==============================================================================

xmemfile::~xmemfile( void )
{
    // Delete buffer
    if( m_pData )
        x_free( m_pData );
}

//==============================================================================

s32 xmemfile::GetLength( void ) const
{
    return m_EOF;
}

//==============================================================================

void xmemfile::Clear( void )
{
    m_EOF      = 0;
    m_Position = 0;
}

//==============================================================================

void xmemfile::FreeExtra( void )
{
    if( m_Capacity > m_EOF )
    {
        m_pData = (byte*)x_realloc( m_pData, m_EOF );
        m_Capacity = m_EOF;
    }
}

//==============================================================================

xbool xmemfile::IsEOF( void ) const
{
    return m_Position >= m_EOF;
}

//==============================================================================

s32 xmemfile::Tell( void ) const
{
    return m_Position;
}

//==============================================================================

s32 xmemfile::Seek( s32 Offset, s32 Origin )
{
    s32 NewPosition = Offset;

    // Calculate NewPosition
    switch( Origin )
    {
    case X_SEEK_SET:
        NewPosition = Offset;
        break;
    case X_SEEK_CUR:
        NewPosition = m_Position + Offset;
        break;
    case X_SEEK_END:
        NewPosition = m_EOF - Offset;
        break;
    default:
        ASSERTS( 0, "Illegal 'Origin' for Seek" );
    }

    // Clamp NewPosition to valid range
    if( NewPosition < 0 )
        NewPosition = 0;
    if( NewPosition > m_EOF )
        NewPosition = m_EOF;

    // Set Position from NewPosition
    m_Position = NewPosition;

    // Return new Position
    return m_Position;
}

//==============================================================================

s32 xmemfile::Read( byte* pBuffer, s32 Count )
{
    s32 BytesToRead = MIN( Count, (m_EOF-m_Position) );

    x_memcpy( pBuffer, m_pData + m_Position, BytesToRead );
    m_Position += BytesToRead;

    return BytesToRead;
}

//==============================================================================

s32 xmemfile::Write( byte* pBuffer, s32 Count )
{
    EnsureCapacity( m_Position + Count );
    x_memcpy( m_pData + m_Position, pBuffer, Count );
    m_Position += Count;

    return Count;
}

//==============================================================================

void xmemfile::Write_s8( s8 Value )
{
    *this += Value;
}

//==============================================================================

void xmemfile::Write_s16( s16 Value )
{
    *this += Value;
}

//==============================================================================

void xmemfile::Write_s32( s32 Value )
{
    *this += Value;
}

//==============================================================================

void xmemfile::Write_f32( f32 Value )
{
    *this += Value;
}

//==============================================================================

void xmemfile::Write_xstring( const xstring& String )
{
    *this += String;
}

//==============================================================================

void xmemfile::Write_xwstring( const xwstring& String )
{
    *this += String;
}

//==============================================================================

s8 xmemfile::Read_s8( void )
{
    s8 Value = 0;
    if( (m_Position+1) <= m_EOF )
    {
        Value = (s8)m_pData[m_Position++];
    }
    return Value;
}

//==============================================================================

s16 xmemfile::Read_s16( void )
{
    s16 Value = 0;
    if( (m_Position+2) <= m_EOF )
    {
        Value  = ((s16)m_pData[m_Position++]) <<   8;
        Value |= ((s16)m_pData[m_Position++]) & 0xff;
    }
    return Value;
}

//==============================================================================

s32 xmemfile::Read_s32( void )
{
    s32 Value = 0;
    if( (m_Position+4) <= m_EOF )
    {
        Value  = (((s32)m_pData[m_Position++]) << 24) & 0xff000000;
        Value |= (((s32)m_pData[m_Position++]) << 16) & 0x00ff0000;
        Value |= (((s32)m_pData[m_Position++]) <<  8) & 0x0000ff00;
        Value |= (((s32)m_pData[m_Position++]) <<  0) & 0x000000ff;
    }
    return Value;
}

//==============================================================================

f32 xmemfile::Read_f32( void )
{
    s32 Value = 0;
    if( (m_Position+4) <= m_EOF )
    {
        Value  = (((s32)m_pData[m_Position++]) << 24) & 0xff000000;
        Value |= (((s32)m_pData[m_Position++]) << 16) & 0x00ff0000;
        Value |= (((s32)m_pData[m_Position++]) <<  8) & 0x0000ff00;
        Value |= (((s32)m_pData[m_Position++]) <<  0) & 0x000000ff;
    }
    return *((f32*)&Value);
}

//==============================================================================

xstring xmemfile::Read_xstring( void )
{
    xstring String;

    char Value = 1;
    while( ((m_Position+1) <= m_EOF) && (Value != 0) )
    {
        Value = m_pData[m_Position++];
        if( Value )
            String += Value;
    }
    
    return String;
}

//==============================================================================

xwstring xmemfile::Read_xwstring( void )
{
    xwstring String;

    xwchar Value = 1;
    while( ((m_Position+1) <= m_EOF) && (Value != 0) )
    {
        Value = ((((s32)m_pData[m_Position  ]) << 8) & 0xff00) |
                ((((s32)m_pData[m_Position+1]) << 0) & 0x00ff);
        m_Position += 2;
        if( Value )
            String += Value;
    }
    
    return String;
}

//==============================================================================

const xmemfile& xmemfile::operator = ( const xmemfile& MemFile )
{
    if( this != &MemFile )
    {
        EnsureCapacity( MemFile.m_EOF );
        x_memcpy( m_pData, MemFile.m_pData, MemFile.m_EOF );
        m_EOF = MemFile.m_EOF;
        m_Position = MemFile.m_Position;
    }

    return( *this );
}

//==============================================================================

const xmemfile& xmemfile::operator += ( const xmemfile& MemFile )
{
    EnsureCapacity( m_Position + MemFile.m_EOF );
    x_memcpy( m_pData + m_Position, MemFile.m_pData, MemFile.m_EOF );
    m_Position += MemFile.m_EOF;

    return( *this );
}

//==============================================================================

const xmemfile& xmemfile::operator += ( s8 Data )
{
    EnsureCapacity( m_Position + 1 );
    m_pData[m_Position++] = Data;
    if( m_Position > m_EOF )
        m_EOF = m_Position;

    return( *this );
}

//==============================================================================

const xmemfile& xmemfile::operator += ( s16 Data )
{
    EnsureCapacity( m_Position + 2 );
    m_pData[m_Position++] = (byte)(Data >>   8);
    m_pData[m_Position++] = (byte)(Data & 0xff);
    if( m_Position > m_EOF )
        m_EOF = m_Position;

    return( *this );
}

//==============================================================================

const xmemfile& xmemfile::operator += ( s32 Data )
{
    EnsureCapacity( m_Position + 4 );
    m_pData[m_Position++] = (byte)(Data >>  24);
    m_pData[m_Position++] = (byte)(Data >>  16);
    m_pData[m_Position++] = (byte)(Data >>   8);
    m_pData[m_Position++] = (byte)(Data & 0xff);
    if( m_Position > m_EOF )
        m_EOF = m_Position;

    return( *this );
}

//==============================================================================

const xmemfile& xmemfile::operator += ( f32 Data )
{
    s32 t = *((s32*)&Data);
    EnsureCapacity( m_Position + 4 );
    m_pData[m_Position++] = (byte)(t >>  24);
    m_pData[m_Position++] = (byte)(t >>  16);
    m_pData[m_Position++] = (byte)(t >>   8);
    m_pData[m_Position++] = (byte)(t & 0xff);
    if( m_Position > m_EOF )
        m_EOF = m_Position;

    return( *this );
}

//==============================================================================

const xmemfile& xmemfile::operator += ( const xstring& String )
{
    EnsureCapacity( m_Position + String.GetLength() + 1 );
    for( s32 i=0 ; i<String.GetLength() ; i++ )
        m_pData[m_Position++] = (byte)String[i];
    m_pData[m_Position++] = (byte)0;
    if( m_Position > m_EOF )
        m_EOF = m_Position;

    return( *this );
}

//==============================================================================

const xmemfile& xmemfile::operator += ( const xwstring& String )
{
    EnsureCapacity( m_Position + String.GetLength()*2 + 1 );
    for( s32 i=0 ; i<String.GetLength() ; i++ )
    {
        m_pData[m_Position++] = (byte)String[i] >>   8;
        m_pData[m_Position++] = (byte)String[i] & 0xff;
    }
    m_pData[m_Position++] = (byte)0;
    m_pData[m_Position++] = (byte)0;
    if( m_Position > m_EOF )
        m_EOF = m_Position;

    return( *this );
}

//==============================================================================

xbool xmemfile::LoadFile( const char* pFileName )
{
    s32     Size;
    s32     Read;

    X_FILE* pFile = x_fopen( pFileName, "rb" );

    if( !pFile )
        return( FALSE );

    Size = x_flength( pFile );
    EnsureCapacity( Size );
    Read = x_fread( m_pData, 1, Size, pFile );
    m_EOF = Read;
    m_Position = 0;

    x_fclose( pFile );

    return( Read == Size );
}

//==============================================================================

xbool xmemfile::SaveFile( const char* pFileName ) const
{
    s32     Written;

    X_FILE* pFile = x_fopen( pFileName, "wb" );

    if( !pFile )
        return( FALSE );

    Written = x_fwrite( m_pData, 1, m_EOF, pFile );
    x_fclose( pFile );

    return( Written == m_EOF );
}

//==============================================================================
