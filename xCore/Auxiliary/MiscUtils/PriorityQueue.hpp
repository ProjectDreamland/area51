
#ifndef PRIORITY_QUEUE_HPP
#define PRIORITY_QUEUE_HPP

//=========================================================================
// General Priority queue
//
// T - is the type of the key data that the heap needs to keep track off. EX: s16, node*
// V - is the type of the priority/weight/value. EX: f32, s16, s8, etc.
// N - is the maximun number of nodes that the heap can have. (Array is no allocated)
//
//=========================================================================
// NOTE:
//
// * The queue will be really happy if you make T and V = 16bits. For the node
//   will become 32bits which is perfect to move around in memory.
//
// * Priorities are set such the lower the priority the more important it is.
//   if you have something that works the oposite way just pass (MaxPriority - Node.Weight)
//
// EXAMPLE:
//
//  priority_queue<s16,s16,100> MyQueue;
//  
//  for( s32 i=0; i<100; i++ )
//      MyQueue.Push( i, Node[i].Weight );
//  
//  ID = MyQueue.GetMin();
//  
//  MyQueue.Pop();
//  
//=========================================================================
template< class T, class V, s32 N >
class priority_queue
{
public:

            priority_queue  ( void );
    void    Push            ( T Element, V Weight );
    T       Pop             ( void );
    T       GetMin          ( void );
    xbool   IsEmpty         ( void );
    void    Clear           ( void );

protected:

    struct node
    {
        V       Weight;
        T       Data;
    };

protected:

    void BuildHeap  ( s32 Position );

protected:

    node        m_Data[ N ];
    s32         m_HeapSize;
};

//=========================================================================

template< class T, class V, s32 N > inline
xbool priority_queue<T,V,N>::IsEmpty( void )
{
    return m_HeapSize == 0;
}

//=========================================================================

template< class T, class V, s32 N > inline
priority_queue<T,V,N>::priority_queue( void )
{
    Clear();
}

//=========================================================================

template< class T, class V, s32 N > inline
void priority_queue<T,V,N>::Clear( void )
{
    m_HeapSize = 0;
}

//=========================================================================

template< class T, class V, s32 N > inline
void priority_queue<T,V,N>::Push( T Element, V Weight )
{
    // add a new element to the heap
    // make sure there is room for new element
    ASSERT( m_HeapSize < N );

    // value starts out in last position
    s32 Position = m_HeapSize++;

    // inv: position <= 0 and < heapmax

    // now percolate up
    while( Position > 0 && Weight < m_Data[(Position-1)>>1].Weight )
    {
        m_Data[ Position ] = m_Data[(Position-1)>>1];

        // inv: tree rooted at "position" is a heap
        Position = (Position - 1)>>1;
    }

    // found location, add new element
    m_Data[ Position ].Data   = Element;
    m_Data[ Position ].Weight = Weight;
}

//=========================================================================

template< class T, class V, s32 N > inline
T priority_queue<T,V,N>::GetMin( void )
{
    ASSERT( m_HeapSize > 0 );
    return m_Data[0].Data;
}


//=========================================================================

template< class T, class V, s32 N > inline
T priority_queue<T,V,N>::Pop ( void )
{
    T Min = GetMin();

    // remove the smallest element from a heap
    // move the last element into the first position
    m_Data[0] = m_Data[ --m_HeapSize ];

    // then move into position
    BuildHeap( 0 );

    return Min;
}

//=========================================================================

template< class T, class V, s32 N > inline
void priority_queue<T,V,N>::BuildHeap( s32 Position )
{
    // rebuild the heap
    node Node = m_Data[ Position ];

    while( Position < m_HeapSize )
    {
        // replace position with the smaller of the
        // two children, or the last element
        s32 ChildPos = (Position << 1) + 1;

        if( ChildPos < m_HeapSize )
        {
            if( (ChildPos + 1 < m_HeapSize) && m_Data[ ChildPos + 1].Weight < m_Data[ ChildPos ].Weight )
            {
                ChildPos += 1;
            }

            // inv: childpos is smaller of two children
            if( Node.Weight < m_Data[ ChildPos ].Weight )
            {
                // found right location
                m_Data[ Position ] = Node;

                // Done.
                break; 
            }
            else
            {
                m_Data[ Position ] = m_Data[ ChildPos ];
                Position = ChildPos;
                // recur and keep moving down
            }
        }
        else
        {
            // no children
            m_Data[ Position ] = Node;

            // Done.
            break;
        }
    }
}

//=========================================================================
// END
//=========================================================================
#endif