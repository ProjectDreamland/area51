//==============================================================================
//
// LinkedList.hpp
//
//      Simple to use template linked linked list class that allows items
//      to be multiple linked lists.
//
//==============================================================================

#ifndef __LINKED_LIST_HPP__
#define __LINKED_LIST_HPP__



//==============================================================================
//
// EXAMPLE USAGE:
//
//==============================================================================
//
// Define class/structure with a node per list you want it in:
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//      struct particle
//      {
//          // Linked list nodes need by linked list
//          linked_list_node<particle>  m_ActiveListNode;
//          linked_list_node<particle>  m_FreeListNode;
//          linked_list_node<particle>  m_RenderListNode;
//
//          // Members
//          xcolor                      m_Color;
//          vector3                     m_Position;
//      };
//
//
// Define linked list type defs so code is more readable:
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//      typedef linked_list< particle, offsetof( particle, m_ActiveListNode  ) > particle_active_list;
//      typedef linked_list< particle, offsetof( particle, m_FreeListNode    ) > particle_free_list;
//      typedef linked_list< particle, offsetof( particle, m_RenderListNode  ) > particle_render_list;
//
// 
// Define the lists in a class/structure or as locals:
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//      class particle_system
//      {
//          particle_free_list    m_ParticleFreeList;  
//          particle_active_list  m_ParticleActiveList;  
//          particle_render_list  m_ParticleRenderList;  
//      };        
//
//
// Adding/Removing items to a list:
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  
//      particle* pParticle = m_ParticleFreeList.GetHead();
//      if( pParticle )
//      {
//          m_ParticleFreeList.Remove( pParticle );
//          m_ParticleActiveList.Append( pParticle );
//      }
//
//
// To trace the items in a list:
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//      particle* pParticle = m_ParticleRenderList.GetHead();
//      while( pParticle )
//      {
//          // Process
//          pParticle->Render();
//
//          // Get next
//          pParticle = m_ParticleRenderList.GetNext( pParticle );
//      }
//
//==============================================================================


//==============================================================================
// INCLUDES
//==============================================================================

#include <stddef.h>
#include "x_files.hpp"

//==============================================================================
// DEBUG DEFINES
//==============================================================================

#if defined(CONFIG_DEBUG) && !defined(X_RETAIL)

// Turn on all debug (slow!)
#define LINKED_LIST_ASSERT( __exp__ )   ASSERT( __exp__ )
#define LINKED_LIST_DEBUG

#else

// Turn off all debug
#define LINKED_LIST_ASSERT( __exp__ )   ((void)0)

#endif

//==============================================================================
// LINKED LIST NODE STRUCTURE
//==============================================================================

// T = Class of item that list will hold
template< class T >
struct linked_list_node
{
    // Functions
    linked_list_node()
    {
        // Clear pointers
        m_pPrev = NULL;
        m_pNext = NULL;
    }

    // Data
    T*  m_pPrev;    // Previous item in list
    T*  m_pNext;    // Next item in list
};


//==============================================================================
// LINKED LIST TEMPLATE CLASS
//==============================================================================

// T = Class of item that list will hold
// N = Offset in item of linked list node
template< class T, s32 N >
class linked_list
{
//==========================================================================
// Functions
//==========================================================================
public:

	// Constructor/destructor
    linked_list();
    ~linked_list();

    // Sets linked_list to empty
    void                    Clear           ( void );

    // Returns linked list node offset
    s32                     GetNodeOffset   ( void ) const;
    
    // Returns linked list node for item
    linked_list_node<T>&    GetNode         ( T* pItem ) const;

    // Returns head of linked_list
    T*                      GetHead         ( void ) const;

    // Returns tail of linked_list
    T*                      GetTail         ( void ) const;

    // Returns previous value of item
    T*                      GetPrev         ( T* pItem ) const;

    // Returns next value of item
    T*                      GetNext         ( T* pItem ) const;

    // Set previous value of item
    void                    SetPrev         ( T* pItem, T* pPrev ) const;

    // Set next value if item
    void                    SetNext         ( T* pItem, T* pNext ) const;

    // Add an item to the end of the linked_list
	s32                     Append          ( T* pItem );
    
    // Add an item to the front of the linked_list
    s32                     Prepend         ( T* pItem );

    // Removed item from linked_list
	s32                     Remove          ( T* pItem );

    // Returns number of items
    s32                     GetCount        ( void ) const;

	// Searches for given item - returns -1 if not found, else the index
	s32                     Find            ( T* pItem ) const;
    
	// Assignment operator
	const linked_list< T, N >& operator= ( const linked_list< T, N >& SourceList );
    
    //==========================================================================
    // Data
    //==========================================================================
private:
        T*      m_pHead;   // Pointer to first item in list
        T*      m_pTail;   // Pointer to last item in list
        s32     m_Count;   // # of items in list
};


//==============================================================================
// FUNCTIONS
//==============================================================================

// Constructor
template< class T, s32 N > inline
linked_list< T, N >::linked_list()
{
    // Set to empty linked list
    m_pHead = NULL;
    m_pTail = NULL;
    m_Count = 0;
}

//==============================================================================

// Destruction
template< class T, s32 N > inline
linked_list< T, N >::~linked_list()
{
// Trash prev/next pointers of all items currently in list if debug is defined
#ifdef LINKED_LIST_DEBUG
    Clear();
#endif
    
}

//==============================================================================

// Clears list
template< class T, s32 N > inline
void linked_list< T, N >::Clear( void )
{
// Trash prev/next pointers of all items currently in list if debug is defined
#ifdef LINKED_LIST_DEBUG
    // Process all list
    while( GetHead() )
    {
        // Get head item
        T* pItem = GetHead();
        ASSERT( pItem );
        
        // Remove from list
        Remove( pItem );
    }
#endif

    // Clear list
    m_pHead = NULL;
    m_pTail = NULL;
    m_Count = 0;
}

//==============================================================================

// Returns node offset value
template< class T, s32 N > inline
s32 linked_list< T, N >::GetNodeOffset( void ) const
{
    return N;
}

//==============================================================================

// Returns linked list node for item
template< class T, s32 N > inline
linked_list_node<T>& linked_list< T, N >::GetNode( T* pItem ) const
{
    ASSERT( pItem );
    return *(linked_list_node<T>*) ( (u32)pItem + GetNodeOffset() );
}

//==============================================================================

// Returns head of linked_list
template< class T, s32 N > inline
T* linked_list< T, N >::GetHead( void ) const
{
    return m_pHead;
}

//==============================================================================

// Returns tail of linked_list
template< class T, s32 N > inline
T* linked_list< T, N >::GetTail( void ) const
{
    return m_pTail;
}

//==============================================================================

// Returns previous value of item
template< class T, s32 N > inline
T* linked_list< T, N >::GetPrev( T* pItem ) const
{
    linked_list_node<T>& Node = GetNode( pItem );
    return Node.m_pPrev;
}

//==============================================================================

// Returns next value of item
template< class T, s32 N > inline
T* linked_list< T, N >::GetNext( T* pItem ) const
{
    linked_list_node<T>& Node = GetNode( pItem );
    return Node.m_pNext;
}

//==============================================================================

// Set previous value of item
template< class T, s32 N > inline
void linked_list< T, N >::SetPrev( T* pItem, T* pPrev ) const
{
    linked_list_node<T>& Node = GetNode( pItem );
    Node.m_pPrev = pPrev;
}

//==============================================================================

// Set next value of item
template< class T, s32 N > inline
void linked_list< T, N >::SetNext( T* pItem, T* pNext ) const
{
    linked_list_node<T>& Node = GetNode( pItem );
    Node.m_pNext = pNext;
}

//==============================================================================

// Add an item to the end of the linked_list
template< class T, s32 N > inline
s32 linked_list< T, N >::Append( T* pItem )
{
    // Should not already be in list
    LINKED_LIST_ASSERT( Find( pItem ) == -1 );

    // Appending to empty list?
    if ( m_Count == 0 )
    {
        // List should be empty
        ASSERT( m_pHead == NULL );
        ASSERT( m_pTail == NULL );

        // Connect item into list
        m_pHead = pItem;
        m_pTail = pItem;

        // Clear item pointers
        SetNext( pItem, NULL );
        SetPrev( pItem, NULL );
    }
    else
    {
        // List should not be empty
        ASSERT( m_pHead != NULL );
        ASSERT( m_pTail != NULL );

        // Add to tail
        SetNext( m_pTail, pItem );
        SetPrev( pItem, m_pTail );
        SetNext( pItem, NULL );
        m_pTail = pItem;
    }

    // Should now be in list
    LINKED_LIST_ASSERT( Find( pItem ) != -1 );

    // Update count
    m_Count++;

    return m_Count;
}

//==============================================================================

// Add an item to the front of the linked_list
template< class T, s32 N > inline
s32 linked_list< T, N >::Prepend( T* pItem )
{
    // Should not already be in list
    LINKED_LIST_ASSERT( Find( pItem ) == -1 );

    // Appending to empty list?
    if ( m_Count == 0 )
    {
        // List should be empty
        ASSERT( m_pHead == NULL );
        ASSERT( m_pTail == NULL );

        // Connect item into list
        m_pHead = pItem;
        m_pTail = pItem;

        // Clear item pointers
        SetNext( pItem, NULL );
        SetPrev( pItem, NULL );
    }
    else
    {
        // List should not be empty
        ASSERT( m_pHead != NULL );
        ASSERT( m_pTail != NULL );

        // Add to head
        SetPrev( m_pHead, pItem );
        SetNext( pItem, m_pHead );
        SetPrev( pItem, NULL );
        m_pHead = pItem;
    }

    // Should now be in list
    LINKED_LIST_ASSERT( Find( pItem ) != -1 );

    // Update count
    m_Count++;

    return m_Count;
}

//==============================================================================

// Removed item from linked_list
template< class T, s32 N > inline
s32 linked_list< T, N >::Remove( T* pItem )
{
    // Should already be in linked list!
    LINKED_LIST_ASSERT( Find( pItem ) != -1 );

    // List should not be empty
    ASSERT( m_Count > 0 );
    ASSERT( m_pHead != NULL );
    ASSERT( m_pTail != NULL );

    // Get previous and next items
    T* pPrev = GetPrev( pItem );
    T* pNext = GetNext( pItem );

    // Update previous?
    if ( pPrev )
    {
        SetNext( pPrev, pNext );
    }        
    else
    {
        ASSERT( m_pHead == pItem );
        m_pHead = pNext;
    }

    // Update next?
    if ( pNext )
    {
        SetPrev( pNext, pPrev );
    }        
    else
    {
        ASSERT( m_pTail == pItem );
        m_pTail = pPrev;
    }

    // Should now be in list
    LINKED_LIST_ASSERT( Find( pItem ) == -1 );

    // Update count
    m_Count--;

#ifdef LINKED_LIST_DEBUG
    // Setup trash pointers to catch problems
    SetPrev( pItem, (T*)0xDEADBEEF );
    SetNext( pItem, (T*)0xDEADBEEF );
#endif

    return m_Count;
}

//==============================================================================

// Returns number of items
template< class T, s32 N > inline
s32 linked_list< T, N >::GetCount( void ) const
{
    return m_Count;
}

//==============================================================================

// Searches for given item - returns -1 if not found, else the index
template< class T, s32 N > inline
s32 linked_list< T, N >::Find( T* pItem ) const
{
    // Perform slow linear search
    s32 Index   = 0;
    T*  pSearch = GetHead();
    while( pSearch )
    {
        // Found?
        if( pSearch == pItem )
            return Index;

        // Check next item
        Index++;
        pSearch = GetNext( pSearch );
    }

    // Not found
    return -1;
}

//==============================================================================

// Assignment operator
template< class T, s32 N > inline
const linked_list< T, N >& linked_list< T, N >::operator= ( const linked_list< T, N >& SourceList )
{
    // Lists cannot be using the same pointers!
    ASSERT( GetNodeOffset() != SourceList.GetNodeOffset() );

    // Setup linked_list vars
    m_pHead = SourceList.GetHead();
    m_pTail = SourceList.GetTail();
    m_Count = SourceList.GetCount();

    // Loop through all items of source list
    T* pItem = SourceList.GetHead();
    while( pItem )
    {
        // Set next and previous pointers
        SetNext( pItem, SourceList.GetNext( pItem ));
        SetPrev( pItem, SourceList.GetPrev( pItem ));

        // Get next item in source linked_list
        pItem = SourceList.GetNext( pItem );
    }

    return ( *this );
}

//==============================================================================


#endif  //#ifndef __LINKED_LIST_HPP__

