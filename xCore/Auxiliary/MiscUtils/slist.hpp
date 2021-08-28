#ifndef SLISTS_HPP
#define SLISTS_HPP

#include "x_files.hpp"

//=========================================================================
// SLISTS
//=========================================================================

//=========================================================================
// HELPERS
//=========================================================================
struct empty_struct {};

struct slist_hnode
{
    inline xbool          IsNull    ( void ) const { return Handle >= 0xffff; }    
    xhandle Handle;
};

//=========================================================================
// CLASS
//=========================================================================
template< class H, class T >
class slist
{
public:
                    slist           ( void );
                   ~slist           ( void );

    H&              AddList         ( xhandle& hList );
    H&              AddList         ( void );
    T&              AddNode         ( xhandle hList, slist_hnode& hNode );
    T&              AddNode         ( xhandle hList );
    T&              AppendNode      ( xhandle hList, slist_hnode& hNode );
    T&              AppendNode      ( xhandle hList );
    void            CopyList        ( xhandle hDest, xhandle hSrc );
    s32             GetCount        ( xhandle hList ) const; 

    H&              GetHeader       ( xhandle       hList ) const;
    T&              GetNode         ( slist_hnode   hNode ) const;
    H&              GetList         ( xhandle       hlist ) const;

    xhandle         GetFirstList    ( void ) const;
    xhandle         GetNextList     ( xhandle hList ) const;

    slist_hnode     GetFirstNode    ( xhandle hList );
    slist_hnode     GetNextNode     ( xhandle hList, slist_hnode hNode ) const;

    void            DelNode         ( xhandle hList, slist_hnode hNode );
    void            DelList         ( xhandle hList );
    void            Empty           ( xhandle hList );
    void            Clear           ( void );
    void            SanityCheck     ( void ) const;

protected:

    struct header
    {
        u16 iNode;
        u16 iHeader;
        H   User;
    };

    void    Grow( s32 Count );

    u16*    m_pNode;
    header* m_pHeader;
    T*      m_pList;
    u16     m_iFree;
    u16     m_iFreeHeader;
    u16     m_iHeader;
    s32     m_nNodes;
    s32     m_nHeaders;
};

//=========================================================================

template< class H, class T > inline
slist<H,T>::slist( void )
{
    m_pNode         = NULL;
    m_pHeader       = NULL;
    m_pList         = NULL;
    m_iFree         = 0xffff;
    m_iFreeHeader   = 0xffff;
    m_iHeader       = 0xffff;
    m_nNodes        = 0;
    m_nHeaders      = 0;
}

//=========================================================================

template< class H, class T > inline
slist<H, T>::~slist( void )
{
    SanityCheck();

    if( m_pNode   ) free( m_pNode   );
    if( m_pHeader ) free( m_pHeader );
    if( m_pList   ) free( m_pList   );
}

//=========================================================================

template< class H, class T >
void slist<H, T>::Clear( void )
{
    s32 i;

    // Do we have something todo?
    if( m_pNode == NULL )
        return;

    // Initialize variables
    m_iFree         = 0xffff;
    m_iFreeHeader   = 0xffff;
    m_iHeader       = 0xffff;

    // Assign all nodes into the empty chain
    for( i=0; i<m_nNodes; i++ )
    {
        m_pNode[i] = i+1;
    }

    // make sure that the last one is set to null
    m_pNode[i-1] = m_iFree;
    m_iFree      = 0;

    // Clear all the headers
    for( i=0; i<m_nHeaders; i++ )
    {
        m_pHeader[i].iHeader = i+1;
    }

    // make sure that the last one is set to null
    m_pHeader[i-1].iHeader = m_iFreeHeader;
    m_iFreeHeader          = 0;
}

//=========================================================================

template< class H, class T >
void slist<H, T>::SanityCheck( void ) const
{
    s32 nFullHeaders  = 0;
    s32 nEmptyHeaders = 0;
    s32 nEmptyNodes   = 0;
    s32 nFullNodes    = 0;
    u16 iNode, iH;

    //
    // First let count how many headers are in used
    //    
    iNode = m_iHeader; 
    while( iNode != 0xffff )
    {
        ASSERT( iNode < m_nHeaders );
        iNode = m_pHeader[ iNode ].iHeader;
        nFullHeaders++;
        ASSERT( nFullHeaders <= m_nHeaders );
    }

    //
    // Find how many headers are empty
    //
    iNode = m_iFreeHeader; 
    while( iNode != 0xffff )
    {
        ASSERT( iNode < m_nHeaders );
        iNode = m_pHeader[ iNode ].iHeader;
        nEmptyHeaders++;
        ASSERT( (nFullHeaders + nEmptyHeaders) <= m_nHeaders );
    }
    ASSERT( (nFullHeaders + nEmptyHeaders) == m_nHeaders );

    //
    // Get the number of empty nodes
    //
    iNode = m_iFree;
    while( iNode != 0xffff )
    {
        ASSERT( iNode < m_nNodes );
        iNode = m_pNode[ iNode ];
        nEmptyNodes++;
        ASSERT( nEmptyNodes <= m_nNodes );
    }

    //
    // Headers must be okay. Lets check nodes now.
    //
    iH = m_iHeader; 
    while( iH != 0xffff )
    {
        s32 SubCount;
        iNode = m_pHeader[ iH ].iNode;
        if( iNode != 0xffff )
        {
            // Lets make sure that it is circular 
            SubCount = 0;
            
            do
            {
                ASSERT( iNode < m_nNodes );
                iNode = m_pNode[ iNode ];
                SubCount++;
                ASSERT( (SubCount + nEmptyNodes + nFullNodes) <= m_nNodes );

            } while( iNode != m_pHeader[ iH ].iNode );
            nFullNodes += SubCount;
        }

        // Move to the next header
        iH = m_pHeader[ iH ].iHeader;
    }
    ASSERT( (nEmptyNodes + nFullNodes) == m_nNodes );
}

//=========================================================================

template< class H, class T >
void slist<H, T>::Grow( s32 Count )
{
    m_nNodes += Count;

    if( m_pNode == NULL )
    {    
        m_pNode     = (u16*)malloc( sizeof(u16) * m_nNodes );
        m_pList     = (T*)  malloc( sizeof(T)   * m_nNodes );
    }
    else
    {
        m_pNode = (u16*)realloc( m_pNode, sizeof(u16) * m_nNodes );
        m_pList = (T*)  realloc( m_pList, sizeof(T)   * m_nNodes );
    }

    // Check to make sure that we are okay
    if( m_pNode == NULL )
        e_throw( "Out of memory" );

    if( m_pList == NULL )
    {
        free( m_pNode );
        e_throw( "Out of memory" );
    }

    // Assign all the empty nodes in the list
    s32 i;
    for( i=m_nNodes-Count; i<m_nNodes; i++ )
    {
        m_pNode[i] = i+1;
    }

    // make sure that the last one is set to null
    m_pNode[i-1] = m_iFree;
    m_iFree      = m_nNodes - Count;
}

//=========================================================================

template< class H, class T >
H& slist<H, T>::AddList( xhandle& hHandle )
{
    // Allocate more headers if we need to
    if( m_iFreeHeader == 0xffff )
    {
        s32 NewCount = (m_nHeaders>>1); 
            NewCount = (NewCount < 10 ) ? 10 : NewCount;
        m_nHeaders  += NewCount;

        if( m_pHeader == NULL ) 
            m_pHeader = (header*)malloc( sizeof(header) * m_nHeaders );
        else 
            m_pHeader = (header*)realloc( m_pHeader, sizeof(header) * m_nHeaders );

        if( m_pHeader == NULL )
            e_throw( "Out of memory" );

        for( s32 i=m_nHeaders-NewCount; i<m_nHeaders; i++ )
        {
            m_pHeader[i].iHeader = i+1;
        }

        // make sure that the last one is set to null
        m_pHeader[i-1].iHeader = m_iFreeHeader;
        m_iFreeHeader          = m_nHeaders-NewCount;
    }

    // Get a free header
    s32 iHeader   = m_iFreeHeader;
    m_iFreeHeader = m_pHeader[ m_iFreeHeader ].iHeader;

    // Initialize the header and add it into the header list
    m_pHeader[ iHeader ].iNode   = 0xffff;
    m_pHeader[ iHeader ].iHeader = m_iHeader;
    m_iHeader                    = iHeader;

    // Return handle for our new list
    hHandle.Handle = iHeader ;

    return m_pHeader[ iHeader ].User;
}                           

//=========================================================================

template< class H, class T > inline
H& slist<H, T>::AddList( void )
{
    xhandle hHandle;
    return AddList( hHandle );
}

//=========================================================================

template< class H, class T >
T& slist<H, T>::AddNode( xhandle hList, slist_hnode& hNode )
{
    ASSERT( !hList.IsNull() );

    u16 iNode;

    // If not more then time to grow
    if( m_iFree == 0xffff )
        Grow( (m_nNodes>>1) < 100 ? 100 : (m_nNodes>>1) );

    // Get an node from the empty list
    iNode   = m_iFree;
    m_iFree = m_pNode[ iNode ];

    // Add it to the header
    if( m_pHeader[ hList ].iNode != 0xffff )
    {
        m_pNode[ iNode ]                    = m_pNode[ m_pHeader[ hList ].iNode ];
        m_pNode[ m_pHeader[ hList ].iNode ] = iNode;
    }
    else
    {
        // Create a circular link list
        m_pNode[ iNode ]         = iNode;
        m_pHeader[ hList ].iNode = iNode; 
    }

    // buidl the handle
    hNode.Handle = iNode;

    // Now we can return the data to the user
    return m_pList[ iNode ];
}

//=========================================================================

template< class H, class T >
T& slist<H, T>::AppendNode( xhandle hList, slist_hnode& hNode )
{
    ASSERT( !hList.IsNull() );

    u16 iNode;

    // If not more then time to grow
    if( m_iFree == 0xffff )
        Grow( (m_nNodes>>1) < 100 ? 100 : (m_nNodes>>1) );

    // Get an node from the empty list
    iNode   = m_iFree;
    m_iFree = m_pNode[ iNode ];

    // Add it to the header
    if( m_pHeader[ hList ].iNode != 0xffff )
    {
        m_pNode[ iNode ]                    = m_pNode[ m_pHeader[ hList ].iNode ];
        m_pNode[ m_pHeader[ hList ].iNode ] = iNode;
        m_pHeader[ hList ].iNode            = iNode;
    }
    else
    {
        // Create a circular link list
        m_pNode[ iNode ]         = iNode;
        m_pHeader[ hList ].iNode = iNode; 
    }

    // buidl the handle
    hNode.Handle = iNode;

    // Now we can return the data to the user
    return m_pList[ iNode ];
}

//=========================================================================

template< class H, class T > inline
T& slist<H, T>::AppendNode( xhandle hList )
{
    slist_hnode hNode;
    return AppendNode( hList, hNode );
}

//=========================================================================

template< class H, class T > inline
T& slist<H, T>::AddNode( xhandle hList )
{
    slist_hnode hNode;
    return AddNode( hList, hNode );
}

//=========================================================================

template< class H, class T >
void slist<H, T>::DelNode( xhandle hList, slist_hnode hNode )
{
    ASSERT( !hNode.IsNull() );
    ASSERT( !hList.IsNull() );

    // okay this is worse case
    if( m_pHeader[ hList ].iNode == hNode.Handle )
    {
        u16 iNode = m_pHeader[ hList ].iNode;
        
        while( m_pNode[ iNode ] != hNode.Handle )
        {
            iNode = m_pNode[ iNode ];
        };

        // nuke it from the list
        if( m_pNode[ iNode ] == iNode )
        {
            m_pHeader[ hList ].iNode = 0xffff;
        }
        else
        {
            m_pNode[ iNode ]            = m_pNode[ hNode.Handle ];
            m_pHeader[ hList ].iNode    = iNode;
        }
    }
    else
    {
        u16 iNode = m_pHeader[ hList ].iNode;
        
        while( m_pNode[ iNode ] != hNode.Handle )
        {
            iNode = m_pNode[ iNode ];
            ASSERT( (iNode != m_pHeader[ hList ].iNode) && "Count not find it");
        }

        // nuke it from the list
        ASSERT( m_pNode[ iNode ] == hNode.Handle );
        m_pNode[ iNode ] = m_pNode[ hNode.Handle ];
    }

    // add it to the empty chain
    m_pNode[ hNode.Handle ] = m_iFree;
    m_iFree                 = (u16)hNode.Handle;
}

//=========================================================================

template< class H, class T > inline
void slist<H, T>::Empty( xhandle hList )
{
    ASSERT( !hList.IsNull() );

    // Delete all the nodes
    u16 iNode = m_pHeader[ hList ].iNode;
    if( iNode != 0xffff )
    {
        do
        {
            u16 iNext = m_pNode[ iNode ];
            m_pNode[ iNode ] = m_iFree;
            m_iFree          = iNode;
            iNode            = iNext;

        } while( iNode != m_pHeader[ hList ].iNode );
    }
}

//=========================================================================

template< class H, class T >
void slist<H, T>::DelList( xhandle hList )
{
    ASSERT( !hList.IsNull() );

    // First empty the list
    Empty( hList );
    
    // Find the prevoius header
    u16* piHeader = &m_iHeader;
    while( *piHeader != hList )
    {
        ASSERT( *piHeader != 0xffff );
        piHeader = &m_pHeader[ *piHeader ].iHeader;
    }

    // Remove the header from the active list
    *piHeader = m_pHeader[ hList ].iHeader;

    // add the empty header int he empty list
    m_pHeader[ hList ].iHeader = m_iFreeHeader;
    m_iFreeHeader              = (u16)hList;
}

//=========================================================================

template< class H, class T > inline
slist_hnode slist<H, T>::GetFirstNode( xhandle hList )
{
    ASSERT( !hList.IsNull() );

    slist_hnode Node;
    if( m_pHeader[ hList ].iNode == 0xffff )
        Node.Handle.Handle = 0xffff;
    else
        Node.Handle.Handle = m_pNode[ m_pHeader[ hList ].iNode ];

    return Node;
}

//=========================================================================

template< class H, class T > inline
slist_hnode slist<H, T>::GetNextNode( xhandle hList, slist_hnode hNode ) const
{
    ASSERT( !hNode.IsNull() );
    ASSERT( !hList.IsNull() );

    slist_hnode Node;

    if( hNode.Handle == m_pHeader[ hList ].iNode )
        Node.Handle.Handle = 0xffff;
    else
        Node.Handle.Handle = m_pNode[ hNode.Handle ];

    return Node;
}

//=========================================================================

template< class H, class T > inline
T& slist<H, T>::GetNode( slist_hnode hNode ) const
{
    ASSERT( !hNode.IsNull() );
    return m_pList[ hNode.Handle ];
}

//=========================================================================

template< class H, class T > inline
void slist<H, T>::CopyList( xhandle hDest, xhandle hSrc )
{
    ASSERT( !hDest.IsNull() );
    ASSERT( !hSrc.IsNull() );

    slist_hnode hNode = GetFirst( hSrc );

    while( hNode.Handle != 0xffff )
    {
        AddNode( hDest ) = GetNode( hNode );        
        hNode            = GetNext( hNode );
    }    
}

//=========================================================================

template< class H, class T > inline
H& slist<H, T>::GetHeader( xhandle hList ) const
{
    ASSERT( !hList.IsNull() );
    return m_pHeader[ hList ].User;
}

//=========================================================================

template< class H, class T > inline
s32 slist<H, T>::GetCount( xhandle hList ) const
{
    ASSERT( !hList.IsNull() );

    s32 Count = 0;
    u16 iNode = m_pHeader[ hList ].iNode;
    if( iNode != 0xffff )
    {
        do
        {
            iNode = m_pNode[ iNode ];
            Count++;

        } while( iNode != m_pHeader[ hList ].iNode );
    }

    return Count;
}

//=========================================================================

template< class H, class T > inline
xhandle slist<H, T>::GetFirstList( void ) const
{
    if( m_iHeader == 0xffff )
        return xhandle( HNULL );
    return xhandle( m_iHeader );
}

//=========================================================================

template< class H, class T > inline
xhandle slist<H, T>::GetNextList( xhandle hList ) const
{
    ASSERT( hList.Handle < 0xffff );
    ASSERT( hList.Handle >= 0 );
    if( m_pHeader[ hList ].iHeader == 0xffff )
        return xhandle( HNULL );
    return xhandle( m_pHeader[ hList ].iHeader );
}

//=========================================================================

template< class H, class T > inline
H& slist<H, T>::GetList( xhandle hList ) const
{
    return m_pHeader[ hList ].User;
}

//=========================================================================
// END
//=========================================================================
#endif