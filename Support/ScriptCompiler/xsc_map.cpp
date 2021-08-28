//==============================================================================
//
//  xsc_map.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "xsc_map.hpp"
#include "x_files.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
//  MEMBER FUNCTIONS FOR CLASS 'xsc_map'
//==============================================================================

void xsc_map::EnsureCapacity( s32 Capacity )
{
    if( Capacity > m_Capacity )
    {
        s32 NewCapacity = m_Capacity*2;

        // Start with a minimum of 16 entries
        if( NewCapacity == 0 )
            NewCapacity = 16;

        // Double capacity until it exceeds required
        while( NewCapacity < Capacity )
            NewCapacity *= 2;

        // Realloc the buffer
        m_pData = (entry*)x_realloc( m_pData, NewCapacity*sizeof(entry) );
        ASSERT( m_pData );
        m_Capacity = NewCapacity;
    }
}

//==============================================================================

xsc_map::xsc_map( void )
{
    m_pData     = 0;
    m_Capacity  = 0;
    m_Count     = 0;
}

//==============================================================================

xsc_map::xsc_map( const xsc_map& Map )
{
    m_pData = (entry*)x_malloc( Map.m_Count * sizeof(entry) );
    ASSERT( m_pData );
    m_Capacity  = Map.m_Count;
    m_Count     = Map.m_Count;
    x_memcpy( m_pData, Map.m_pData, m_Capacity );
}

//==============================================================================

xsc_map::~xsc_map( void )
{
    // Delete buffer
    if( m_pData )
        x_free( m_pData );
}

//==============================================================================

void xsc_map::Clear( void )
{
    m_Count = 0;
}

//==============================================================================

s32 xsc_map::GetCount( void ) const
{
    return m_Count;
}

//==============================================================================

void xsc_map::Add( s32 Key, s32 Value )
{
    EnsureCapacity( m_Count+1 );

    m_pData[m_Count].Key   = Key;
    m_pData[m_Count].Value = Value;
    m_Count++;
}

//==============================================================================

xsc_map::entry* xsc_map::GetEntry( s32 Index )
{
    ASSERT( (Index >= 0) && (Index < m_Count) );

    return &m_pData[Index];
}

//==============================================================================

xsc_map::entry* xsc_map::FindByKey( s32 Key )
{
    for( s32 i=0 ; i<m_Count ; i++ )
    {
        if( m_pData[i].Key == Key )
            return &m_pData[i];
    }

    return NULL;
}

//==============================================================================

xsc_map::entry* xsc_map::FindByValue( s32 Value )
{
    for( s32 i=0 ; i<m_Count ; i++ )
    {
        if( m_pData[i].Value == Value )
            return &m_pData[i];
    }

    return NULL;
}

//==============================================================================

xsc_map::entry* xsc_map::FindByPair( s32 Key, s32 Value )
{
    for( s32 i=0 ; i<m_Count ; i++ )
    {
        if( ( m_pData[i].Key == Key ) &&
            ( m_pData[i].Key == Key ) )
            return &m_pData[i];
    }

    return NULL;
}

//==============================================================================

s32 xsc_map::ValueFromKey( s32 Key )
{
    s32 Value = 0;
    entry* pEntry = FindByKey( Key );
    ASSERT( pEntry );
    if( pEntry )
        Value = pEntry->Value;
    return Value;
}

//==============================================================================

s32 xsc_map::KeyFromValue( s32 Value )
{
    s32 Key = 0;
    entry* pEntry = FindByValue( Value );
    ASSERT( pEntry );
    if( pEntry )
        Key = pEntry->Key;
    return Key;
}

//==============================================================================

const xsc_map& xsc_map::operator = ( const xsc_map& Map )
{
    if( this != &Map )
    {
        EnsureCapacity( Map.m_Count );
        x_memcpy( m_pData, Map.m_pData, Map.m_Count );
        m_Count = Map.m_Count;
    }

    return( *this );
}

//==============================================================================

const xsc_map& xsc_map::operator += ( const entry& Entry )
{
    Add( Entry.Key, Entry.Value );

    return( *this );
}

//==============================================================================
