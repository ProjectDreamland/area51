/*

	Table.h - Type-independant Table (Dynamic array) class

*/

#ifndef _TABLE_H
#define _TABLE_H

/*-------------------------------------------------------------------------------

 A Generic "Table" class.      

  (DSilva 9-13-94)

  This is a type-safe variable length array which also supports list-like
  operations of insertion, appending and deleting.  Two instance variables
  are maintained: "nalloc" is the number elements allocated in the
  array; "count" is the number actual used. (count<=nalloc).
  Allocation is performed automatically when Insert or Append operations
  are performed.  It can also be done manually by calling Resize or Shrink.
  Note: Delete does not resize the storage: to do this call Shrink().  
  If you are going to do a sequence of Appends, it's more efficient to 
  first call Resize to make room for them.  Beware of using the Addr 
  function: it returns a pointer which may be invalid after subsequent 
  Insert, Append, Delete, Resize, or Shrink operations.  


  The implementation minimizes the storage of empty Tables: they are
  represented by a single NULL pointer.  Also, the major part of the
  code is generic, shared by different Tabs for different types of elements.

------------------------------------------------------------------------------*/

#include <malloc.h>
#include <assert.h>
#include <stdlib.h>

typedef struct {
    long count;
    long nalloc;
} TableHeader;

////////////////////////////////////////////////////////////////////////////////
// Functions for internal use only: Clients should never call these.
//
long TableMakeSize(TableHeader **ppTabHdr, int num, int elsize); 
long TableInsertAt(TableHeader **ppTabHdr,int at, int num, void *el, int elsize, int extra); 
long TableCopy(TableHeader **ppTabHdr,int at, int num, void *el, int elsize); 
long TableDelete(TableHeader **ppTabHdr,int starting, int num, int elsize);
void TableSetCount(TableHeader **ppTabHdr,int n, int elsize);
////////////////////////////////////////////////////////////////////////////////


// 100 is an arbitrary number - these aren't alloc'd normally
// but used for casting instead

template <class Type> class TableHead
{
public:
    long count;
    long nalloc;
    Type data[100];
    TableHead() { count = nalloc = 0; }
};


// Type of function to pass to Sort.
// Note: Sort just uses the C lib qsort function. If we restricted
// all tlTable elements to have well defined <,>,== then we wouldn't need
// this callback function.
typedef int(__cdecl *CompareFnc) ( const void *elem1, const void *elem2 );


template <class Type> class Table
{
private:
	TableHead<Type> *th;

public:
	Table() { th = 0; }									// Constructor
	Table(const Table& tb);								// Copy constructor
	virtual ~Table() { if(th) { delete th; th = 0; } }	// Destructor

	Table& operator=(const Table& tb);					// Assignment operator

	int Count() const { return(th ? th->count:0); }		// return number of entries being used
	int Nalloc() const { return(th ? th->nalloc:0); }	// return number of entries allocated

	void ZeroCount() { if (th) th->count = 0; }
	void SetCount(int n) { TableSetCount((TableHeader **)&th, n, sizeof(Type)); }

	Type& operator[](const int i) const;	// Array operator
	Type* Addr(const int i) const;			// Address of the ith entry

	int Insert(int at, Type &el);			// Insert an element at position "at" 
	int Insert(int at, int num, Type *el);	// Insert "num" elements position "at" 

	// Append an element position at end of array
	// If need to enlarge the array, allocate "allocExtra" extra slots
	int Append(Type &el, int allocExtra = 0);

	// Append "num" elements position on end of array
	// If need to enlarge the array, allocate "allocExtra" extra slots
	int Append(int num, Type *el, int allocExtra = 0);

    int Delete(int start,int num);			// List-type delete of "num" elements starting with "start"
	int Resize(int num);					// Change number allocated to num
	void Shrink();							// Contract the table so there's no wasted space (nalloc == count)

	void Sort(CompareFnc cmp);				// Sort the elements using the compare function provided

	int Find(Type &el);						// Return the index of the first occurance of (el), or -1 if not found
	int AppendUnique(Type &el, int Grow=0);	// Append (el) if unique and return index, or return index of existing (el)
	int Remove(Type &el);					// Find and remove the first occurance of (el), return 1 if success, 0 if not
};


// Copy constructor
template <class Type>
Table<Type>::Table(const Table& tb)
{
	th = 0;
	TableCopy((TableHeader **)&th, 0, tb.Count(), &tb.th->data, sizeof(Type));
}


// Assignment operator
template <class Type>
Table<Type>& Table<Type>::operator=(const Table<Type>& tb)
{
	if(tb.Count())
		TableCopy((TableHeader **)&th, 0, tb.Count(), &tb.th->data, sizeof(Type));
	return *this;
}

// Array operator
template <class Type>
Type& Table<Type>::operator[](const int i) const
{
	assert(th && (unsigned)i < (unsigned)th->count);
	return(th->data[i]);
}

// Address operation (use carefully)
template <class Type>
Type* Table<Type>::Addr(const int i) const
{
	assert(th && ((unsigned)i < (unsigned)th->count)); return(&th->data[i]);
}

// Insert an element at position "at" 
template <class Type>
int Table<Type>::Insert(int at, Type &el)
{
	return(TableInsertAt((TableHeader **)&th, at, 1, (void *)&el, sizeof(Type), 0));
}

// Insert "num" elements position "at" 
template <class Type>
int Table<Type>::Insert(int at, int num, Type *el)
{
	return(TableInsertAt((TableHeader **)&th, at, num, (void *)el, sizeof(Type),0));
}

// Append an element position at end of array
// If need to enlarge the array, allocate "allocExtra" extra slots
template <class Type>
int Table<Type>::Append(Type &el, int allocExtra)
{
    return(TableInsertAt((TableHeader **)&th, th ? th->count:0, 1, (void *)&el, sizeof(Type), allocExtra));
}

// Append "num" elements position on end of array
// If need to enlarge the array, allocate "allocExtra" extra slots
template <class Type>
int Table<Type>::Append(int num, Type *el, int allocExtra)
{
    return(TableInsertAt((TableHeader **)&th, th ? th->count : 0, num, (void *)el, sizeof(Type),allocExtra));
}

// List-type delete of "num" elements starting with "start"
template <class Type>
int Table<Type>::Delete(int start,int num)
{
    return(TableDelete((TableHeader **)&th, start, num, sizeof(Type)));
}

// Change number of allocated items to num
template <class Type>
int Table<Type>::Resize(int num)
{
    return(TableMakeSize((TableHeader **)&th, num, sizeof(Type)));
}

// Reallocate so there is no wasted space (nalloc = count)
template <class Type>
void Table<Type>::Shrink()
{
    TableMakeSize((TableHeader **)&th, th ? th->count : 0, sizeof(Type));
}

template <class Type>
void Table<Type>::Sort(CompareFnc cmp)
{
	if (th)
		qsort(th->data, th->count, sizeof(Type), cmp);
}

template <class Type>
int Table<Type>::Find(Type &el)
{
	if(th)
	{
		for(int i=0; i<th->count; i++)
		{
			if(th->data[i] == el)
				return i;
		}
	}

	return -1;
}

template <class Type>
int Table<Type>::Remove(Type &el)
{
int Index = Find(el);

	if(Index >= 0)
	{
		Delete(Index, 1);
		return 1;
	}
	return 0;
}

template <class Type>
int Table<Type>::AppendUnique(Type &el, int Grow)
{
int Index = Find(el);

	if(Index >= 0)
		return Index;

	return Append(el, Grow);
}

#endif
