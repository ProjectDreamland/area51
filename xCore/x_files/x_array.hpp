//==============================================================================
//  
//  x_array.hpp
//  
//==============================================================================

#ifndef X_ARRAY_HPP
#define X_ARRAY_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

#ifndef X_STDIO_HPP
#include "x_stdio.hpp"
#endif

//==============================================================================
//  TYPES
//==============================================================================

//==============================================================================
//  TEMPLATIZED DYNAMIC ARRAY
//==============================================================================
//  
//  Yet another templatized dynamic array!  This one has one advantage over all 
//  the others, though... it's in the x_files.
//  
//  The class T which is used to instantiate the template must satisfy a few
//  requirements:
//      - Void constructor, "T( void )".
//      - Assignment, "operator = ( const T& )".
//      - Equality test, "operator == ( const T& )", if Find is used.
//  
//  The Count represents how many elements are currently in the xarray.  The 
//  Capacity indicates how many elements can be held without the need for the 
//  xarray to re-allocate its internal array.
//  
//  The xarray class tries to minimize the number of dynamic memory operations
//  performed.  However, it relies on information provided by the SetCapacity 
//  function.
//  
//  An xarray can be prevented from automatically adjusting its capacity by 
//  "locking" it with SetLocked( TRUE ).  The xarray will behave normally as 
//  long as no alterations to the capacity are required.  The xarray can be 
//  unlocked at any time.
//  
//  An xarray can be constructed "around" any pre-existing normal array via the
//  "xarray( T*, s32, s32 )" constructor.  The resulting xarray will consider 
//  itself "static" and permanently locked.  All functions will perform as 
//  expected as long as no capacity changes are required.  Upon destruction, a
//  static xarray will not attempt to delete the pointer to the data array.
//  
//  When the run-time validation macros are enabled, that is when X_ASSERT is
//  defined, all of the functions perform usage validations.  Of particular note
//  is that operator [] has index range checks.
//
//==============================================================================
//  
//  xarray( T*, s32, s32 )  - Construct in "static" mode using given array 
//                            pointer.  Adds the xarray interface to any 
//                            pre-existing normal array.
//  
//  operator T*             - Cast to "T*" function.  Returns the address of the 
//                            internal data array.
//  
//  Append( void )          - Append a void constructed element to the array and
//                            return a reference to this newly added element.
//                            This provides the opportunity to "set up" an 
//                            object "in place" rather than setting it up in an 
//                            external instance and then copying the object into
//                            the xarray.
//  
//  SetCapacity - Attempts to set the Capacity to the given value.  Only valid 
//                if not Locked, not Static, and the given Capacity is not less
//                than the current Count.  This can cause the Capacity to be
//                increased or decreased.
//  
//  Clear       - Sets the Count to zero.  If not Static and not Locked, this 
//                will release the allocation thus setting the Capacity to zero.
//  
//  FreeExtra   - Only valid when not Static and not Locked.  If the Capacity
//                exceeds the Count, then the dynamic allocation is reduced to
//                the minimum necessary size, and Capacity is set to Count.
//  
//==============================================================================

template <class T>
class xarray
{
public:                         
                    xarray          ( void );
                    xarray          ( const xarray<T>& Array   );
                    xarray          ( T* Array, s32 Capacity, s32 Count=0 );
                   ~xarray          ( void );

                    operator T*     ( void ) const; 
        T&          operator []     ( s32 Index ) const;

        T&          GetAt           ( s32 Index ) const;
        void        SetAt           ( s32 Index, const T& Element );
        
        T&          Insert          ( s32 Index );
        void        Insert          ( s32 Index, const T& Element );
        void        Insert          ( s32 Index, const xarray<T>& Array );
        void        Delete          ( s32 Index, s32 Count=1 );
        
        T&          Append          ( void );
        void        Append          ( const T& Element );
        void        Append          ( const xarray<T>& Array );
        
        s32         GetCount        ( void ) const;
        void        SetCount        ( s32 NewCount);    //Be careful with this one!
        s32         GetCapacity     ( void ) const;
        void        SetCapacity     ( s32 Capacity );

        void        SetGrowAmount   ( s32 Amt );

        void        Clear           ( void );
        void        FreeExtra       ( void );
        
        s32         Find            ( const T& Element, s32 StartIndex=0 ) const;

        void        SetLocked       ( xbool Locked );
        xbool       IsLocked        ( void ) const;
        xbool       IsStatic        ( void ) const;

        xbool       Save            ( X_FILE* pFile ) const;
        xbool       Load            ( X_FILE* pFile );

        T*          GetPtr          ( void ) const;

const   xarray<T>&  operator =      ( const xarray<T>& Array   );
const   xarray<T>&  operator +=     ( const xarray<T>& Array   );
const   xarray<T>&  operator +=     ( const T&         Element );
const   xarray<T>   operator +      ( const xarray<T>& Array   ) const; 

protected:
        s32         CalcGrowth      ( void );           // Utility function to factor out growth algorithms

        T*          m_pData; 
        s32         m_Count;   
        s32         m_Capacity;
        s32         m_Status;
        s32         m_GrowAmount;
};

//==============================================================================
//  TEMPLATIZED HANDLE-BASE ARRAY 
//==============================================================================
//
//
//
//==============================================================================
template< class T >
class xharray
{
public:
                xharray             ( void );
               ~xharray             ( void );
    s32         GetCount            ( void ) const;

    s32         GetCapacity         ( void ) const;
    void        Clear               ( xbool bReorder = FALSE );
    T&          Add                 ( void );
    T&          Add                 ( xhandle&  hHandle  );

    T&          operator[]          ( s32       Index    );
    const T&    operator[]          ( s32       Index    ) const;
    T&          operator()          ( xhandle   hHandle  );
    const T&    operator()          ( xhandle   hHandle  ) const;

    void        DeleteByIndex       ( s32       Index    );
    void        DeleteByHandle      ( xhandle   hHandle  );
    xhandle     GetHandleByIndex    ( s32       Index    ) const;
    s32         GetIndexByHandle    ( xhandle   hHandle  ) const;

    s32         CalcGrowth          ( void );
    void        GrowListBy          ( s32       nNodes   );

    xbool       Save                ( X_FILE*   Fp       );
    xbool       Load                ( X_FILE*   Fp       );

    const   xharray<T>&  operator = ( const xharray<T>& Array   );

protected:
    s32         m_Capacity;
    s32         m_nNodes;
    xhandle*    m_pHandle;
    T*          m_pList;    
};

///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////

//=========================================================================

template< class T >
inline s32 xharray<T>::CalcGrowth( void )
{
    s32 GrowBy = 1;

#ifdef TARGET_PC
    GrowBy = MAX( 100, (m_Capacity/2) );
#else
    GrowBy = 1;
#endif

    return GrowBy;
}

//=========================================================================

template< class T > inline
xharray<T>::xharray( void )
{
    m_Capacity    = 0;
    m_nNodes      = 0;
    m_pHandle     = NULL;
    m_pList       = NULL;    
}

//=========================================================================

template< class T > inline
xharray<T>::~xharray( void )
{
    Clear( FALSE );

    delete[] m_pHandle;
    delete[] m_pList;
}

//=========================================================================

template< class T > inline
const xharray<T>& xharray<T>::operator = ( const xharray<T>& Array )
{
    //
    // delete all the nodes
    //
    Clear( FALSE );
    
    delete[] m_pHandle;
    delete[] m_pList;
    m_pHandle = NULL;
    m_pList = NULL;

    //
    // Reset all the variables
    //
    m_Capacity    = 0;
    m_nNodes      = 0;
    m_pHandle     = NULL;
    m_pList       = NULL;    

    //
    // Allocate the buffers
    //
    GrowListBy( Array.GetCount() );

    //
    // Copy all the nodes
    //
    for( s32 i=0; i<Array.GetCount(); i++ )
    {
        Add() = Array[i];
    }

    return *this;
}

//=========================================================================

template< class T >
void xharray<T>::GrowListBy( s32 nNodes )
{
    xhandle*    pNewHandle;
    T*          pNewList;    
    s32         i;

    ASSERT( nNodes > 0 );

    //
    // Increase the capacity
    //
    m_Capacity += nNodes;

    //
    // Allocate the new arrays
    //
    pNewList   = new T[m_Capacity];
    ASSERT( pNewList );

    pNewHandle = new xhandle[m_Capacity];
    ASSERT( pNewHandle );

    //
    // Copy all the previous nodes to the new arrays
    //
    for( i=0; i<m_nNodes; i++ )
    {
        pNewHandle[i] = m_pHandle[i];
        pNewList[i] = m_pList[i];
    }

    //
    // Fill in the rest of the hash entries
    //
    for( i = m_nNodes; i<m_Capacity; i++ )
    {
        pNewHandle[ i ].Handle = i;
    }

    //
    // Update the class with the new lists
    //
    delete[] m_pHandle;
    delete[] m_pList;

    m_pHandle = pNewHandle;
    m_pList   = pNewList;
}

//=========================================================================

template< class T > inline
T& xharray<T>::operator[]( s32 Index )
{
    ASSERT( Index >= 0 );
    ASSERT( Index < m_nNodes );

    return m_pList[ m_pHandle[ Index ].Handle ];
}

//=========================================================================

template< class T > inline
const T& xharray<T>::operator[]( s32 Index ) const
{
    ASSERT( Index >= 0 );
    ASSERT( Index < m_nNodes );

    return m_pList[ m_pHandle[ Index ].Handle ];
}

//=========================================================================

template< class T > inline
T& xharray<T>::operator()( xhandle hHandle )
{
    ASSERT( hHandle >= 0 );
    ASSERT( hHandle < m_Capacity );

    return m_pList[ hHandle.Handle ];
}

//=========================================================================

template< class T > inline
const T& xharray<T>::operator()( xhandle hHandle ) const
{
    ASSERT( hHandle >= 0 );
    ASSERT( hHandle < m_Capacity );

    return m_pList[ hHandle.Handle ];
}

//=========================================================================

template< class T > inline
s32 xharray<T>::GetCount( void ) const
{
    return m_nNodes;
}

//=========================================================================

template< class T > inline
s32 xharray<T>::GetCapacity( void ) const
{
    return m_Capacity;
}
    
//=========================================================================

template< class T > inline
xhandle xharray<T>::GetHandleByIndex( s32 Index ) const
{
    ASSERT( Index >= 0 );
    ASSERT( Index < m_nNodes );
    ASSERT( m_pHandle[ Index ] != HNULL );

    return m_pHandle[ Index ];
}

//=========================================================================

template< class T > inline
s32 xharray<T>::GetIndexByHandle( xhandle hHandle ) const
{
    ASSERT( hHandle != HNULL );

    for( s32 i=0; i<m_nNodes; i++ )
    {
        if( m_pHandle[ i ] == hHandle ) return i;
    }

    ASSERT( 0 && "Not hash entry contain that ID" );
    return 0;
}

//=========================================================================

template< class T > inline
void xharray<T>::DeleteByIndex( s32 Index )
{
    ASSERT( Index >= 0 );
    ASSERT( Index < m_nNodes );

    xhandle  HandleID = m_pHandle[ Index ];

    m_nNodes--;

    // Copy the last node into the deleted node. We don't care if it is the same.
    m_pHandle[ Index ]    = m_pHandle[ m_nNodes ];
    m_pHandle[ m_nNodes ] = HandleID;    
}

//=========================================================================

template< class T > inline
void xharray<T>::DeleteByHandle( xhandle hHandle )
{
    ASSERT( hHandle != HNULL );
    DeleteByIndex( GetIndexByHandle( hHandle ) );
}

//=========================================================================

template< class T > inline
T& xharray<T>::Add( xhandle& hHandle )
{
    //
    // Grow if need it
    //
    if( m_nNodes >= m_Capacity )
    {
        GrowListBy( CalcGrowth() );
    }

    hHandle = m_pHandle[ m_nNodes ];
    m_nNodes++;

    ASSERT( hHandle != HNULL );

    // return the node
    return m_pList[ hHandle.Handle ];
}

//=========================================================================

template< class T > inline
T& xharray<T>::Add( void )
{
    xhandle hHandle;
    return Add( hHandle );
}

//=========================================================================

template< class T > inline
void xharray<T>::Clear( xbool bReorder )
{
    s32 i;

    // reorder if the user request it
    if( bReorder )
    {
        for( i=0; i<m_Capacity; i++ )
            m_pHandle[i].Handle = i;
    }

    m_Capacity = 0;
    m_nNodes = 0;

    delete[] m_pHandle;
    delete[] m_pList;
    m_pHandle = NULL;
    m_pList = NULL;
}

//=========================================================================

template< class T > inline
xbool xharray<T>::Save( X_FILE* Fp )
{
    x_fwrite( this,         sizeof(*this),          1,              Fp );
    x_fwrite( m_pHandle,    sizeof(*m_pHandle),     m_Capacity,     Fp );
    x_fwrite( m_pList,      sizeof(*m_pList),       m_Capacity,     Fp );

    return TRUE;
}

//=========================================================================

template< class T > inline
xbool xharray<T>::Load( X_FILE* Fp )
{
    x_fread( this,         sizeof(*this),          1,          Fp );

    m_pList   = (T*)x_malloc( (sizeof(xhandle)+sizeof(T)) * m_Capacity );
    ASSERT( m_pList );

    m_pHandle = (xhandle*)(m_pList+m_Capacity);

    x_fread( m_pHandle,    sizeof(*m_pHandle),     m_Capacity, Fp );
    x_fread( m_pList,      sizeof(*m_pList),       m_Capacity, Fp );

    return TRUE;
}

//==============================================================================
//  PRIVATE
//==============================================================================

#include "Implementation/x_array_private.hpp"

//==============================================================================
#endif // X_ARRAY_HPP
//==============================================================================
