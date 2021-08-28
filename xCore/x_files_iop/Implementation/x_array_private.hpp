//==============================================================================
//
//  x_array_private.hpp
//
//==============================================================================

#ifndef X_ARRAY_PRIVATE_HPP
#define X_ARRAY_PRIVATE_HPP
#else
#error "File " __FILE__ " has been included twice!"
#endif

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_PLUS_HPP
#include "x_plus.hpp"
#endif

#ifndef X_MEMORY_HPP
#include "x_memory.hpp"
#endif

//==============================================================================
//  DEFINES
//==============================================================================

#define STATUS_NORMAL  0
#define STATUS_LOCKED  1
#define STATUS_STATIC  2

//==============================================================================
//  FUNCTIONS
//==============================================================================
//  Inline functions first.
//==============================================================================

template< class T >
inline T& xarray<T>::operator [] ( s32 Index ) const
{
    ASSERT( Index >= 0 );
    ASSERT( Index <  m_Count );

    return( m_pData[ Index ] );
}

//==============================================================================

template< class T >
inline T& xarray<T>::GetAt( s32 Index ) const
{
    ASSERT( Index >= 0 );
    ASSERT( Index <  m_Count );

    return( m_pData[ Index ] );
}

//==============================================================================

template< class T >
inline void xarray<T>::SetAt( s32 Index, const T& Element )
{
    ASSERT( Index >= 0 );
    ASSERT( Index <  m_Count );

    m_pData[ Index ] = Element;
}

//==============================================================================

template< class T >
inline s32 xarray<T>::GetCount( void ) const
{
    return( m_Count );
}

//==============================================================================

template< class T >
inline s32 xarray<T>::GetCapacity( void ) const
{
    return( m_Capacity );
}

//==============================================================================

template< class T >
inline xarray<T>::operator T* ( void ) const
{
    return( m_pData );
}

//==============================================================================

template< class T >
inline void xarray<T>::SetLocked( xbool Locked )
{
    ASSERT( m_Status != STATUS_STATIC );
    m_Status = Locked ? STATUS_LOCKED : STATUS_NORMAL;
}

//==============================================================================

template< class T >
inline xbool xarray<T>::IsLocked( void ) const
{
    return( (m_Status == STATUS_LOCKED) || (m_Status == STATUS_STATIC) );
}

//==============================================================================

template< class T >
inline xbool xarray<T>::IsStatic( void ) const
{
    return( m_Status == STATUS_STATIC );
}

//==============================================================================

template< class T >
inline const xarray<T>& xarray<T>::operator += ( const xarray<T>& Array )
{
    Append( Array );
    return( *this );
}

//==============================================================================

template< class T >
inline const xarray<T>& xarray<T>::operator += ( const T& Element )
{
    Append( Element );
    return( *this );
}

//==============================================================================

template< class T >
xarray<T>::xarray( void )
{
    m_pData    = NULL;
    m_Count    = 0;
    m_Capacity = 0;
    m_Status   = STATUS_NORMAL;
}

//==============================================================================

template< class T >
xarray<T>::xarray( const xarray<T>& Array )
{
    s32 i;

    m_Status   = STATUS_NORMAL;
    m_Capacity = Array.GetCount();
    m_Count    = m_Capacity;
    m_pData    = new T[ m_Count ];

    for( i = 0; i < m_Count; i++ )
    {
        m_pData[i] = Array.m_pData[i];
    }
}

//==============================================================================

template< class T >
xarray<T>::xarray( T* Array, s32 Capacity, s32 Count )
{
    ASSERT( Array );
    ASSERT( Count    >= 0 );
    ASSERT( Capacity >= Count );
    m_pData    = Array;
    m_Count    = Count;
    m_Capacity = Capacity;
    m_Status   = STATUS_STATIC;
}

//==============================================================================

template< class T >
xarray<T>::~xarray( void )
{
    if( m_Status != STATUS_STATIC )
        delete [] m_pData;
}

//==============================================================================

template< class T >
void xarray<T>::Insert( s32 Index, const T& Element )
{
    s32 i;
    ASSERT( Index >= 0 );
    ASSERT( Index <= m_Count ); // Allow "insertion" at the end of the array.
    
    if( m_Capacity < m_Count+1 )
    {
        // Need to grow the buffer.
        ASSERT( m_Status != STATUS_LOCKED );
        DEMAND( m_Status != STATUS_STATIC );

        s32 NewCapacity = m_Count+1;
        T*  pNewData    = new T[ NewCapacity ];

        // Copy over elements below insertion point.
        for( i = 0; i < Index; i++ )
        {
            pNewData[i] = m_pData[i];
        }

        // Copy over elements above insertion point.
        for( i = Index; i < m_Count; i++ )
        {
            pNewData[i+1] = m_pData[i];
        }

        // Copy in the inserted element.
        pNewData[Index] = Element;

        // Ditch the old allocation, and install the new one.
        delete [] m_pData;
        m_pData    = pNewData;
        m_Capacity = NewCapacity;

        // Update count.
        m_Count += 1;
    }
    else
    {
        // We don't need to grow.

        // Shift elements over.
        for( i = m_Count; i > Index; i-- )
        {
            m_pData[i] = m_pData[i-1];
        }

        // Assign new element.
        m_pData[Index] = Element;

        // Update count.
        m_Count += 1;
    }   
}

//==============================================================================

template< class T >
void xarray<T>::Insert( s32 Index, const xarray<T>& Array )
{
    s32 i;
    ASSERT( Index >= 0 );
    ASSERT( Index <= m_Count ); // Allow "insertion" at the end of the array.
    ASSERT( *this != Array );   // Can't insert self into self!  (Ouch!)

    if( m_Capacity < (m_Count + Array.m_Count) )
    {
        // Need to grow the buffer.
        ASSERT( m_Status != STATUS_LOCKED );
        DEMAND( m_Status != STATUS_STATIC );

        s32 NewCapacity = m_Count + Array.m_Count;
        T*  pNewData    = new T[ NewCapacity ];

        // Copy over elements below insertion point.
        for( i = 0; i < Index; i++ )
        {
            pNewData[i] = m_pData[i];
        }

        // Copy over elements above insertion point.
        for( i = Index; i < m_Count; i++ )
        {
            pNewData[ i + Array.m_Count ] = m_pData[i];
        }

        // Copy in the inserted elements.
        for( i = 0; i < Array.m_Count; i++ )
        {
            pNewData[ i + Index ] = Array.m_pData[i];
        }

        // Ditch the old allocation, and install the new one.
        delete [] m_pData;
        m_pData    = pNewData;
        m_Capacity = NewCapacity;

        // Update count.
        m_Count += Array.m_Count;
    }
    else
    {
        // We don't need to grow.

        // Shift elements over.
        for( i = m_Count + Array.m_Count - 1; i >= Index + Array.m_Count; i-- )
        {
            m_pData[i] = m_pData[ i - Array.m_Count ];
        }

        // Bring in new elements.
        for( i = 0; i < Array.m_Count; i++ )
        {
            m_pData[ i + Index ] = Array.m_pData[i];
        }

        // Update count.
        m_Count += Array.m_Count;
    }   
}

//==============================================================================

template< class T >
void xarray<T>::Delete( s32 Index, s32 Count )
{
    s32 Shift;
    ASSERT( Index >= 0 );
    ASSERT( Count >= 0 );
    ASSERT( Index + Count <= m_Count );

    // Shift elements down.
    Shift = m_Count - (Index + Count);
    while( Shift > 0 )
    {
        m_pData[ Index ] = m_pData[ Index + Count ];
        Shift--;
        Index++;
    }
     
    // Update count.
    m_Count -= Count;
}

//==============================================================================

template< class T >
T& xarray<T>::Append( void )
{
    if( m_Capacity < m_Count+1 )
    {
        // Need to grow the buffer.
        ASSERT( m_Status != STATUS_LOCKED );
        DEMAND( m_Status != STATUS_STATIC );

        s32 i;
        T*  pNewData = new T[ m_Count + 1 ];

        // Copy over original elements.
        for( i = 0; i < m_Count; i++ )
        {
            pNewData[i] = m_pData[i];
        }

        // Ditch the old allocation, and install the new one.
        delete [] m_pData;
        m_pData    = pNewData;
        m_Capacity = m_Count + 1;
    }

    // Update count.
    m_Count++;

    // Return reference to "new" element.
    return( m_pData[ m_Count-1 ] );
}

//==============================================================================

template< class T >
void xarray<T>::Append( const T& Element )
{       
    if( m_Capacity < m_Count+1 )
    {
        // Need to grow the buffer.
        ASSERT( m_Status != STATUS_LOCKED );
        DEMAND( m_Status != STATUS_STATIC );

        s32 i;
        T*  pNewData = new T[ m_Count + 1 ];

        // Copy over original elements.
        for( i = 0; i < m_Count; i++ )
        {
            pNewData[i] = m_pData[i];
        }

        // Ditch the old allocation, and install the new one.
        delete [] m_pData;
        m_pData    = pNewData;
        m_Capacity = m_Count + 1;
    }

    // Bring in new element.
    m_pData[ m_Count ] = Element;

    // Update count.
    m_Count++;
}

//==============================================================================

template< class T >
void xarray<T>::Append( const xarray<T>& Array )
{
    s32 i;
    ASSERT( *this != Array );   // Can't append self onto self!

    if( m_Capacity < (m_Count + Array.m_Count) )
    {
        // Need to grow the buffer.
        ASSERT( m_Status != STATUS_LOCKED );
        DEMAND( m_Status != STATUS_STATIC );

        T*  pNewData = new T[ m_Count + Array.m_Count ];

        // Copy over original elements.
        for( i = 0; i < m_Count; i++ )
        {
            pNewData[i] = m_pData[i];
        }

        // Ditch the old allocation, and install the new one.
        delete [] m_pData;
        m_pData    = pNewData;
        m_Capacity = m_Count + Array.m_Count;
    }

    // Bring in new elements.
    for( i = 0; i < Array.m_Count; i++ )
    {
        m_pData[ m_Count + i ] = Array.m_pData[i];
    }    

    // Update count.
    m_Count += Array.m_Count;
}

//==============================================================================

template< class T >
void xarray<T>::SetCapacity( s32 Capacity )
{
    ASSERT( Capacity >= m_Count );
    ASSERT( m_Status == STATUS_NORMAL );

    if( Capacity != m_Capacity )
    {
        s32 i;
        T*  pNewData = new T[ Capacity ];

        // Copy over existing elements.
        for( i = 0; i < m_Count; i++ )
        {
            pNewData[i] = m_pData[i];
        }

        // Ditch the old allocation, and install the new one.
        delete [] m_pData;
        m_pData    = pNewData;
        m_Capacity = Capacity;        
    }
}

//==============================================================================

template< class T >
void xarray<T>::Clear( void )
{
    if( m_Status == STATUS_NORMAL )
    {
        delete [] m_pData;
        m_pData    = NULL;
        m_Count    = 0;
        m_Capacity = 0;
    }
    else
    {
        m_Count = 0;
    }
}

//==============================================================================

template< class T >
void xarray<T>::FreeExtra( void )
{
    ASSERT( m_Status == STATUS_NORMAL );

    if( m_Capacity > m_Count )
    {
        s32 i;
        T*  pNewData = new T[ m_Count ];

        // Bring in existing elements.
        for( i = 0; i < m_Count; i++ )
        {
            pNewData[i] = m_pData[i];
        }

        // Ditch the old allocation, and install the new one.
        delete [] m_pData;
        m_pData    = pNewData;
        m_Capacity = m_Count;        
    }
}

//==============================================================================

template< class T >
s32 xarray<T>::Find( const T& Element, s32 StartIndex ) const
{
    s32 i;

    ASSERT( StartIndex <= m_Count );

    // Linear search.
    for( i = StartIndex; i < m_Count; i++ )
        if( m_pData[i] == Element )
            return( i );

    // Not found!
    return( -1 );
}

//==============================================================================

template< class T >
const xarray<T>& xarray<T>::operator = ( const xarray<T>& Array )
{
    s32 i;

    if( this != &Array )
    {
        if( m_Capacity < Array.m_Count )
        {
            // Need to grow the buffer.
            ASSERT( m_Status != STATUS_LOCKED );
            DEMAND( m_Status != STATUS_STATIC );

            // Out with the old!
            delete [] m_pData;

            // Prepare for the new!
            m_pData    = new T[ Array.m_Count ];
            m_Count    = 0;
            m_Capacity = Array.m_Count;
        }

        // Copy in elements.
        for( i = 0; i < Array.m_Count; i++ )
        {
            m_pData[i] = Array.m_pData[i];
        }

        // Update count.
        m_Count = Array.m_Count;
    }

    return( *this );
}

//==============================================================================

template< class T >
const xarray<T> xarray<T>::operator + ( const xarray<T>& Array ) const
{
    xarray<T> Result;

    Result.SetCapacity( m_Count + Array.m_Count );
    Result.Append( *this );
    Result.Append( Array );

    return( Result );
}

//==============================================================================
//  CLEAR DEFINES
//==============================================================================

#undef STATUS_NORMAL
#undef STATUS_LOCKED
#undef STATUS_STATIC

//==============================================================================
