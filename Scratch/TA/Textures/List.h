//==============================================================================
//
// list.h
//
//==============================================================================
#ifndef __LIST_H__
#define __LIST_H__


//==============================================================================
// INCLUDES
//==============================================================================

#include "x_files\x_files.hpp"


//==============================================================================
// TEMPLATE CLASSES
//==============================================================================

// USE:
// list<color_node, offsetof(color_node, LevelNext), offsetof(color_node, LevelPrev)>  List ;



// Class template
template<class TYPE, s32 NEXT_OFFSET, s32 PREV_OFFSET>
class list
{
//==========================================================================
// Data
//==========================================================================
private:
    TYPE    *m_Head ;   // Head of list
    TYPE    *m_Tail ;   // Tail of list
    s32     m_Size ;    // Length of list

//==========================================================================
// Functions
//==========================================================================
public:

	// Construction
    inline list()
    {
        // Set to empty list
        Clear() ;
    }

    // Destruction
    inline ~list()
    {
    }
    
    //======================================================================

    // Sets list to empty
    inline void Clear( void )
    {
        m_Head = NULL ;
        m_Tail = NULL ;
        m_Size = 0 ;
    }

    //======================================================================

    // Returns next offset value
    inline s32 GetNextOffset() const
    {
        return NEXT_OFFSET ;
    }

    //======================================================================

    // Returns previous offset value
    inline s32 GetPrevOffset() const
    {
        return PREV_OFFSET ;
    }

    //======================================================================

    // Returns head of list
    inline TYPE* GetHead() const
    {
        return m_Head ;
    }

    //======================================================================

    // Returns tail of list
    inline TYPE* GetTail() const
    {
        return m_Tail ;
    }

    //======================================================================

    // Returns address of next pointer
    inline TYPE* *GetNextPointer(TYPE* Element) const
    {
        return (TYPE* *)((s32)Element + NEXT_OFFSET) ;
    }

    //======================================================================

    // Returns address of prev pointer
    inline TYPE* *GetPrevPointer(TYPE* Element) const
    {
        return (TYPE* *)((s32)Element + PREV_OFFSET) ;
    }

    //======================================================================

    // Returns next value if element
    inline TYPE* GetNext(TYPE* Element) const
    {
        TYPE* *NextPointer = GetNextPointer(Element) ;
        return *NextPointer ;
    }

    //======================================================================

    // Returns previous value of element
    inline TYPE* GetPrev(TYPE* Element) const
    {
        TYPE* *PrevPointer = GetPrevPointer(Element) ;
        return *PrevPointer ;
    }

    //======================================================================

    // Set next value if element
    inline void SetNext(TYPE* Element, TYPE* Next) const
    {
        TYPE* *NextPointer = GetNextPointer(Element) ;
        *NextPointer = Next ;
    }

    //======================================================================

    // Set previous value of element
    inline void SetPrev(TYPE* Element, TYPE* Prev) const
    {
        TYPE* *PrevPointer = GetPrevPointer(Element) ;
        *PrevPointer = Prev ;
    }

    //======================================================================

    // Add an element to the end of the list
	inline s32 Append(TYPE* Element)
    {
        // Should not already be in list!
        //ASSERT(Find(Element) == -1) ;

        // Empty list?
        if (m_Size == 0)
        {
            ASSERT(m_Head == NULL) ;
            ASSERT(m_Tail == NULL) ;

            m_Head = Element ;
            m_Tail = Element ;

            SetNext(Element, NULL) ;
            SetPrev(Element, NULL) ;
        }
        else
        {
            ASSERT(m_Head != NULL) ;
            ASSERT(m_Tail != NULL) ;

            // Add to tail
            SetNext(m_Tail, Element) ;
            SetPrev(Element, m_Tail) ;
            SetNext(Element, NULL) ;
            m_Tail = Element ;
        }

        //ASSERT(Find(Element) != -1) ;

        // Update size
        m_Size++ ;

        return m_Size ;
    }

    //======================================================================

    // Delete element from list
	inline s32 Delete(TYPE* Element)
    {
        // Should already be in list!
        //ASSERT(Find(Element) != -1) ;

        ASSERT(m_Size > 0) ;

        ASSERT(m_Head != NULL) ;
        ASSERT(m_Tail != NULL) ;

        TYPE* Next = GetNext(Element) ;
        TYPE* Prev = GetPrev(Element) ;

        // Update prev?
        if (Prev)
            SetNext(Prev, Next) ;
        else
        {
            ASSERT(m_Head == Element) ;
            m_Head = Next ;
        }

        // Update next?
        if (Next)
            SetPrev(Next, Prev) ;
        else
        {
            ASSERT(m_Tail == Element) ;
            m_Tail = Prev ;
        }

        //ASSERT(Find(Element) == -1) ;

        // Update size
        m_Size-- ;

        return m_Size ;
    }

    //======================================================================

    // Returns number of items
    inline s32 GetSize() const
    {
        return m_Size ;
    }

    //======================================================================

	// Searches for given element - returns -1 if not found, else the index
	inline s32 Find(TYPE* Element) const
    {
        // Perform slow linear search
        s32   Index = 0 ;
        TYPE* Search = m_Head ;
        while(Search)
        {
            if (Search == Element)
                return Index ;

            Index++ ;
            Search = GetNext(Search) ;
        }
    
        return -1 ;
    }

    //======================================================================

    // Quick check
    inline s32 Contains(TYPE* Element) const
    {
        // At head or tail of list?
        if ( (m_Head == Element) || (m_Tail == Element) )
            return TRUE ;

        // Is list?
        if ( (GetNext(Element)) || (GetPrev(Element)) )
            return TRUE ;

        // Can't be in list
        return FALSE ;
    }

    //======================================================================

	// Assignment operator
	inline const list<TYPE, NEXT_OFFSET, PREV_OFFSET>& operator=(const list<TYPE, NEXT_OFFSET, PREV_OFFSET>& SourceList)
    {
        // Lists cannot be using the same pointers!
        ASSERT(NEXT_OFFSET != SourceList.GetNextOffset()) ;
        ASSERT(PREV_OFFSET != SourceList.GetPrevOffset()) ;

        // Setup list vars
        m_Head = SourceList.GetHead() ;
        m_Tail = SourceList.GetTail() ;
        m_Size = SourceList.GetSize() ;

        // Setup element vars
        TYPE* Element = SourceList.GetHead() ;
        while(Element)
        {
            // Set next and previous pointers
            SetNext(Element, SourceList.GetNext(Element)) ;
            SetPrev(Element, SourceList.GetPrev(Element)) ;

            // Get next element in source list
            Element = SourceList.GetNext(Element) ;
        }

        return (*this) ;
	}

    //======================================================================

} ;


#endif  //#ifndef __LIST_H__

