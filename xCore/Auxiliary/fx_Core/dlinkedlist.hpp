// ============================================================================
// Data Structures For Game Programmers
// Ron Penton
// DLinkedList.h
// This is the Doubly-Linked List class
// ============================================================================
#ifndef DLINKEDLIST_H
#define DLINKEDLIST_H




// forward declarations of all the classes in this file
template<class Datatype> class DListNode;
template<class Datatype> class DLinkedList;
template<class Datatype> class DListIterator;



// -------------------------------------------------------
// Name:        DListNode
// Description: This is the Doubly-linked list node class.
// -------------------------------------------------------
template<class Datatype>
class DListNode
{
public:


// ----------------------------------------------------------------
//  Name:           m_data
//  Description:    This is the data in the node.
// ----------------------------------------------------------------
    Datatype m_data;

// ----------------------------------------------------------------
//  Name:           m_next
//  Description:    This is a pointer to the next node in the list
// ----------------------------------------------------------------
    DListNode<Datatype>* m_next;

// ----------------------------------------------------------------
//  Name:           m_previous
//  Description:    This is a pointer to the last node in the list
// ----------------------------------------------------------------
    DListNode<Datatype>* m_previous;


// ----------------------------------------------------------------
//  Name:           DeLink
//  Description:    This delinks this node from the list it is in.
//  Arguments:      None.
//  Return Value:   None.
// ----------------------------------------------------------------
    void Delink()
    {
        // if a previous node exists, then make the previous
        // node point to the next node.
        if( m_previous != 0 )
            m_previous->m_next = m_next;

        // if the next node exists, then make the next node
        // point to the previous node.
        if( m_next != 0 )
            m_next->m_previous = m_previous;
    }


// ----------------------------------------------------------------
//  Name:           InsertAfter
//  Description:    This adds a node after the current node.
//  Arguments:      p_data - The data to store in the new node.
//  Return Value:   None.
// ----------------------------------------------------------------
    void InsertAfter( Datatype p_data )
    {
        // create the new node.
        DListNode<Datatype>* newnode = new DListNode<Datatype>;
        newnode->m_data = p_data;

        // set up newnode's pointers.
        newnode->m_next     = m_next;
        newnode->m_previous = this;

        // if there is a node after this one, make it point to
        // newnode
        if( m_next != 0 )
            m_next->m_previous = newnode;

        // make the current node point to newnode.
        m_next = newnode;
    }


// ----------------------------------------------------------------
//  Name:           InsertBefore
//  Description:    This adds a node before the current node.
//  Arguments:      p_data - The data to store in the new node.
//  Return Value:   None.
// ----------------------------------------------------------------
    void InsertBefore( Datatype p_data )
    {
        // create the new node.
        DListNode<Datatype>* newnode = new DListNode<Datatype>;
        newnode->m_data = p_data;

        // set up newnode's pointers.
        newnode->m_next     = this;
        newnode->m_previous = m_previous;

        // if there is a node before this one, make it point to
        // newnode
        if( m_previous != 0 )
            m_previous->m_next = newnode;

        // make the current node point to newnode.
        m_previous = newnode;
    }


};



// -------------------------------------------------------
// Name:        DLinkedList
// Description: This is the Doubly-linked list container.
// -------------------------------------------------------
template<class Datatype>
class DLinkedList
{
public:

// ----------------------------------------------------------------
//  Name:           DLinkedList
//  Description:    Constructor; creates an empty list
//  Arguments:      None.
//  Return Value:   None.
// ----------------------------------------------------------------
    DLinkedList()
    {
        m_head = 0;
        m_tail = 0;
        m_count = 0;
    }

    
// ----------------------------------------------------------------
//  Name:           DLinkedList
//  Description:    Destructor; destroys every node
//  Arguments:      None.
//  Return Value:   None.
// ----------------------------------------------------------------
    ~DLinkedList()
    {
        // temporary node pointers.
        DListNode<Datatype>* node = m_head;
        DListNode<Datatype>* next;

        while( node != 0 )
        {
            // save the pointer to the next node.
            next = node->m_next;

            // delete the current node.
            delete node;

            // make the next node the current node.
            node = next;
        }
    }


// ----------------------------------------------------------------
//  Name:           Append
//  Description:    Adds a new node to the end of a list
//  Arguments:      p_data - the data to be added.
//  Return Value:   None.
// ----------------------------------------------------------------
    void Append( Datatype p_data )
    {
        // if there is no head node (ie: list is empty)
        if( m_head == 0 )
        {
            // create a new head node.
            m_head = m_tail = new DListNode<Datatype>;
            m_head->m_data = p_data;
            m_head->m_next = 0;
            m_head->m_previous = 0;
        }
        else
        {
            // insert a new node after the tail, and reset the tail.
            m_tail->InsertAfter( p_data );
            m_tail = m_tail->m_next;
        }
        m_count++;
    }


// ----------------------------------------------------------------
//  Name:           Prepend
//  Description:    Addss a new node to the beginning of a list
//  Arguments:      p_data - the data to be added.
//  Return Value:   None.
// ----------------------------------------------------------------
    void Prepend( Datatype p_data )
    {
        // if there is no head node (ie: list is empty)
        if( m_head == 0 )
        {
            // create a new head node.
            m_head = m_tail = new DListNode<Datatype>;
            m_head->m_data = p_data;
            m_head->m_next = 0;
            m_head->m_previous = 0;
        }
        else
        {
            // insert a new node before the head, and reset the head.
            m_head->InsertBefore( p_data );
            m_head = m_head->m_previous;
        }
        m_count++;
    }


// ----------------------------------------------------------------
//  Name:           RemoveHead
//  Description:    This removes the very first node in the list.
//  Arguments:      None.
//  Return Value:   None.
// ----------------------------------------------------------------
    void RemoveHead()
    {
        DListNode<Datatype>* node = 0;

        if( m_head != 0 )
        {
            // make node point to the next node.
            node = m_head->m_next;

            // then delete the head, and make the pointer
            // point to node.
            delete m_head;
            m_head = node;

            // if the head is null, then we've just deleted the only node
            // in the list. set the tail to 0.
            // if not, set the previous pointer to 0.
            if( m_head == 0 )
                m_tail = 0;
            else
                m_head->m_previous = 0;

            m_count--;
        }
    }


// ----------------------------------------------------------------
//  Name:           RemoveTail
//  Description:    This removes the very last node in the list.
//  Arguments:      None.
//  Return Value:   None.
// ----------------------------------------------------------------
    void RemoveTail()
    {
        DListNode<Datatype>* node = 0;

        if( m_tail != 0 )
        {
            // make node point to the next node.
            node = m_tail->m_previous;

            // then delete the head, and make the pointer
            // point to node.
            delete m_tail;
            m_tail = node;

            // if the tail is null, then we've just deleted the only node
            // in the list. set the head to 0.
            // if not, set the next pointer to 0.
            if( m_tail == 0 )
                m_head = 0;
            else
                m_tail->m_next = 0;

            m_count--;
        }
    }



// ----------------------------------------------------------------
//  Name:           InsertAfter
//  Description:    Inserts data after the iterator, or at the end
//                  of the list if iterator is invalid.
//  Arguments:      p_iterator: The iterator to insert after
//                  p_data: the data to insert
//  Return Value:   None.
// ----------------------------------------------------------------
    void InsertAfter( DListIterator<Datatype>& p_iterator, Datatype p_data )
    {
        if( p_iterator.m_node != 0 )
        {
            // insert the data after the iterator
            p_iterator.m_node->InsertAfter( p_data );

            // if the iterator was the tail of the list,
            // reset the tail pointer
            if( p_iterator.m_node == m_tail )
                m_tail = m_tail->m_next;

            // increment the count
            m_count++;
        }
        else
        {
            Append( p_data );
        }
    }


// ----------------------------------------------------------------
//  Name:           InsertBefore
//  Description:    inserts data before the iterator, or prepends
//                  it to the beginning of the list if invalid.
//  Arguments:      p_iterator: The iterator to insert after
//                  p_data: the data to insert
//  Return Value:   None.
// ----------------------------------------------------------------
    void InsertBefore( DListIterator<Datatype>& p_iterator, Datatype p_data )
    {
        if( p_iterator.m_node != 0 )
        {
            // insert the data before the iterator
            p_iterator.m_node->InsertBefore( p_data );

            // if the iterator was the head of the list,
            // reset the head pointer.
            if( p_iterator.m_node == m_head )
                m_head = m_head->m_previous;

            // increment the count
            m_count++;
        }
        else
        {
            Prepend( p_data );
        }
    }


// ----------------------------------------------------------------
//  Name:           Remove
//  Description:    Removes the node that the iterator points to.
//                  moves iterator forward to the next node.
//  Arguments:      p_iterator: The iterator to remove
//  Return Value:   None.
// ----------------------------------------------------------------
    void Remove( DListIterator<Datatype>& p_iterator )
    {
        // temporary node pointer.
        DListNode<Datatype>* node;

        // if node is invalid, do nothing.
        if( p_iterator.m_node == 0 )
            return;


        // save the pointer to the node we want to delete.
        node = p_iterator.m_node;

        // if the node we want to remove is the head or the tail
        // nodes, then move the head or tail to the next or
        // previous node.
        if( node == m_head )
        {
            m_head = m_head->m_next;
        }
        else if( node == m_tail )
        {
            m_tail = m_tail->m_previous;
        }

        // move the iterator forward to the next valid node
        p_iterator.Forth();

        // delink and delete the node.
        node->Delink();
        delete node;

        // if the head is 0, then set the tail to 0 as well.
        if( m_head == 0 )
            m_tail = 0;
        
        m_count--;
    }



// ----------------------------------------------------------------
//  Name:           GetIterator
//  Description:    Gets an iterator pointing to the beginning
//                  of the list.
//  Arguments:      None.
//  Return Value:   iterator pointing to the beginning of the list.
// ----------------------------------------------------------------
    DListIterator<Datatype> GetIterator()
    {
        return DListIterator<Datatype>( this, m_head );
    }


// ----------------------------------------------------------------
//  Name:           Size
//  Description:    Gets the size of the list
//  Arguments:      None.
//  Return Value:   size of the list.
// ----------------------------------------------------------------
    int Size()
    {
        return m_count;
    }

// ----------------------------------------------------------------
//  Name:           m_head
//  Description:    The first node in the list
// ----------------------------------------------------------------
    DListNode<Datatype>* m_head;

// ----------------------------------------------------------------
//  Name:           m_tail
//  Description:    The last node in the list
// ----------------------------------------------------------------
    DListNode<Datatype>* m_tail;

// ----------------------------------------------------------------
//  Name:           m_count
//  Description:    The number of nodes in the list
// ----------------------------------------------------------------
    int m_count;
};




// -------------------------------------------------------
// Name:        DListIterator
// Description: This is the basic linked list 
//              iterator class.
// -------------------------------------------------------
template<class Datatype>
class DListIterator
{
public:


// ----------------------------------------------------------------
//  Name:           DListIterator
//  Description:    Constructor, creates an iterator that points
//                  to the given list and node. 
//  Arguments:      p_list: pointer to the list the iterator belongs
//                          to.
//                  p_node: pointer to the current node.
//  Return Value:   None.
// ----------------------------------------------------------------
    DListIterator( DLinkedList<Datatype>* p_list = 0,
                   DListNode<Datatype>* p_node = 0 )
    {
        m_list = p_list;
        m_node = p_node;
    }


// ----------------------------------------------------------------
//  Name:           Start
//  Description:    Resets the iterator to the beginning of the 
//                  list
//  Arguments:      None.
//  Return Value:   None.
// ----------------------------------------------------------------
    void Start()
    {
        if( m_list != 0 )
            m_node = m_list->m_head;
    }

// ----------------------------------------------------------------
//  Name:           End
//  Description:    Resets the iterator to the end of the list
//  Arguments:      None.
//  Return Value:   None.
// ----------------------------------------------------------------
    void End()
    {
        if( m_list != 0 )
            m_node = m_list->m_tail;
    }


// ----------------------------------------------------------------
//  Name:           Forth
//  Description:    Moves the iterator forward by one position
//  Arguments:      None.
//  Return Value:   None.
// ----------------------------------------------------------------
    void Forth()
    {
        if( m_node != 0 )
            m_node = m_node->m_next;
    }


// ----------------------------------------------------------------
//  Name:           Back
//  Description:    Moves the iterator backward by one position.
//  Arguments:      None.
//  Return Value:   None.
// ----------------------------------------------------------------
    void Back()
    {
        if( m_node != 0 )
            m_node = m_node->m_previous;
    }


// ----------------------------------------------------------------
//  Name:           Item
//  Description:    Gets the item that the iterator is pointing to.
//  Arguments:      None.
//  Return Value:   Reference to the data in the node.
// ----------------------------------------------------------------
    Datatype& Item()
    {
        return m_node->m_data;
    }


// ----------------------------------------------------------------
//  Name:           Valid
//  Description:    Determines if the node is valid.
//  Arguments:      None.
//  Return Value:   true if valid
// ----------------------------------------------------------------
    bool Valid()
    {
        return (m_node != 0);
    }


// ----------------------------------------------------------------
//  Name:           operator==
//  Description:    Determines if two iterators point to the same
//                  node.
//  Arguments:      None.
//  Return Value:   true if they point to the same node.
// ----------------------------------------------------------------
    bool operator==( DListIterator<Datatype>& p_rhs )
    {
        if( m_node == p_rhs.m_node && m_list == p_rhs.m_list )
        {
            return true;
        }
        return false;
    }



// ----------------------------------------------------------------
//  Name:           m_node
//  Description:    pointer to the current node
// ----------------------------------------------------------------
    DListNode<Datatype>* m_node;


// ----------------------------------------------------------------
//  Name:           m_list
//  Description:    pointer to the current list.
// ----------------------------------------------------------------
    DLinkedList<Datatype>* m_list;
};



#endif
