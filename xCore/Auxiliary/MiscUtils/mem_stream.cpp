
#include "mem_stream.hpp"
#include "x_plus.hpp"

//=========================================================================

mem_stream::mem_stream( void )
{
    x_memset( this, 0, sizeof(*this) );
    m_GrowSize = 100*1024;    // 100K grow as a default
}

//=========================================================================

mem_stream::~mem_stream( void )
{
    x_free( m_pPtr );
}

//=========================================================================

void mem_stream::GrowBy( s32 Count )
{
    ASSERT( Count > 0 );
    m_GrowSize = Count;
}

//=========================================================================

s32 mem_stream::Tell( void )
{
    ASSERT( m_pPtr );
    return m_Pos;
}

//=========================================================================

void mem_stream::SeekPos( s32 Pos )
{
    ASSERT( m_pPtr );
    ASSERT( Pos <= m_EndPos );        
    m_Pos = Pos;
}

//=========================================================================

void mem_stream::Grow( s32 Count )
{
    // Always grow by the max
    if( Count < m_GrowSize ) 
        Count = m_GrowSize;
    
    // New size
    m_CurrentSize += Count;
    if( m_pPtr ) 
    {
        m_pPtr = (byte*)x_realloc( m_pPtr, m_CurrentSize );
        ASSERT( m_pPtr );
    }
    else
    {
        m_pPtr = (byte*)x_malloc( m_CurrentSize );
        ASSERT( m_pPtr );
    }
}

//=========================================================================

void mem_stream::Preallocate( s32 Count, xbool bUpdatePos )
{
    s32 NewPos = m_Pos+Count;

    // Make sure that we can fit in memory
    if( NewPos > m_CurrentSize ) 
        Grow( Count );

    ASSERT( m_pPtr );

    // Keep track of the end of the file
    if( NewPos > m_EndPos ) 
        m_EndPos = NewPos;

    // Set the new pos if requested
    if( bUpdatePos ) 
        m_Pos = NewPos;
}

//=========================================================================

void mem_stream::Preallocate32( s32 Count, xbool bUpdatePos )
{
    // Make sure to align the current position to 32bytes
    s32 Aligment = ALIGN_32(m_Pos) - m_Pos;
    m_Pos = ALIGN_32(m_Pos);

    s32 NewPos = m_Pos+Count;

    // Make sure that we can fit in memory
    if( NewPos > m_CurrentSize ) 
        Grow( Count + Aligment );

    ASSERT( NewPos <= m_CurrentSize );
    ASSERT( m_pPtr );

    // Keep track of the end of the file
    if( NewPos > m_EndPos ) 
        m_EndPos = NewPos;

    // Set the new pos if requested
    if( bUpdatePos ) 
        m_Pos = NewPos;
}


//=========================================================================

void mem_stream::Write( const void* pData, s32 Count )
{
    // Reserve the space
    Preallocate( Count );
    ASSERT( m_pPtr );

    // Now Write the data
    // TODO: Change Indian here
    x_memcpy( &m_pPtr[m_Pos], pData, Count );
    m_Pos += Count;
}

//=========================================================================

void mem_stream::WriteAt( s32 Pos, const void* pData, s32 Count )
{
    ASSERT( m_pPtr );
    SeekPos( Pos );
    Write( pData, Count );
}

//=========================================================================

void mem_stream::GotoEnd( void )
{
//  hopefully this doesn't mask any bugs. Shouldn't.
//  ASSERT( m_pPtr );
    m_Pos = m_EndPos;
}

//=========================================================================

s32 mem_stream::GetLength( void )
{
    return m_EndPos;
}

//=========================================================================

void mem_stream::Save( X_FILE* Fp )
{
    if( m_pPtr == NULL )
        return;
    x_fwrite( m_pPtr, m_EndPos, 1, Fp );
}

