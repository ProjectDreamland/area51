

//==========================================================================
// Notes: All of the list insertion / removal functions should have
// some kind of semaphoring put around them, so that these lists may safely
// be used in a multitasking application.
//==========================================================================

// Debug levels:
//   0 = no debug stuff. Printing fully disabled.
//   1 = Errors printed.
//   2 = "Here I am"'s printed

//#define ANALYZE_SORT


#include "x_files.hpp"
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "ccList.h"

ccMinNode::ccMinNode()
{
	next = (ccMinNode *)LIST_MAGIC;
	prev = (ccMinNode *)LIST_MAGIC; 
}

ccMinNode::ccMinNode(const ccMinNode &)
{
{
	next = (ccMinNode *)LIST_MAGIC;
	prev = (ccMinNode *)LIST_MAGIC; 
}
}

ccMinNode::~ccMinNode()
{
}

ccNode::ccNode(void)
{
	name = 0;
	nameHash = 0;
}


ccNode::~ccNode(void)
{
	if(name)
		delete [] name;
}


//****************************************************************************
// The behaviour of this function is this: If NULL is passed, the current name
// (if any) is cleared, and nothing else happens.
// Otherwise, the old name is cleared and the new name is set.
//****************************************************************************

xbool ccNode::SetName(const char *theName)
{
xbool retval = FALSE;
long	Len;

	if(name)
	{
		delete [] name;	// Get rid of the old one first, if any.
		name = NULL;
	}

	if(theName)
	{
		Len = strlen(theName);
		name = new char[Len+1];

		if(name)
		{
			strcpy(name,theName);

			//--Compute a hash value for the name-------
			nameHash = CalcHash(theName);
			retval = TRUE;
		}
	}

	return(retval);
}

ccMinList::ccMinList()
{
	head = tail = 0;
	numElements = 0;
}

ccMinList::~ccMinList()
{
	Purge();
}

void ccMinList::AddNode(ccMinNode *insertpoint, ccMinNode *node)
{
	// Make sure that there is a node.
	assert(node != NULL);

	// Make sure that this node isn't already in a list.
	assert(node->next == (ccNode *)LIST_MAGIC && node->prev == (ccNode *)LIST_MAGIC);

	if(node)
	{
		if(insertpoint)
		{
			node->next = insertpoint->next;
			if( node->next )
				node->next->prev = node;

			node->prev = insertpoint;
			insertpoint->next = node;
		}
		else	// If no insert point is being passed, it is assumed that you want the head of the list.
		{
			node->next = head;
			if( node->next )
				node->next->prev = node;

			node->prev = NULL;
			head = node;
		}

		if(tail == insertpoint)
			tail = node;	// update the tail pointer.


		assert(head != (ccNode *)LIST_MAGIC && tail != (ccNode *)LIST_MAGIC);

		numElements++;
	}
}



xbool ccMinList::IsInList(ccMinNode *node) const
{
xbool isthere = FALSE;
ccMinNode *c;

	c = GetHead();

	while(c)
	{
		if(c == node)
		{
			isthere = TRUE;
			c = NULL;
		}
		else
			c = c->GetNext();
	}

	return(isthere);
}

long ccMinList::ElementNumber(ccMinNode *node)
{
long	Index = 0;
ccMinNode *c;

	c = GetHead();

	while(c)
	{
		if(c == node)
			return Index;
		else
			c = c->GetNext();

		Index++;
	}

	return -1;
}


// This routine should never really be called with a null 'node' pointer. However, it deals with this
// case properly.

ccMinNode *ccMinList::RemNode(ccMinNode *node)
{
	if(node)
	{

		assert( (node->next != (ccMinNode *)LIST_MAGIC) && (node->prev != (ccMinNode *)LIST_MAGIC) );

		// Make sure that the list thinks that there are elements.
		assert(numElements);

		assert( IsInList(node) );

		if(node == head)
			head = node->next;

		if(node == tail)
			tail = node->prev;

		if(node->prev)
			node->prev->next = node->next;

		if(node->next)
			node->next->prev = node->prev;

		node->next = (ccMinNode *)LIST_MAGIC;
		node->prev = (ccMinNode *)LIST_MAGIC;

		assert(head != (ccMinNode *)LIST_MAGIC && tail != (ccMinNode *)LIST_MAGIC);

		numElements--;
	}
	else
		node = NULL;

	return(node);
}


// This function legally returns NULL if the list is empty.
ccMinNode * ccMinList::RemHead(void)
{
ccMinNode *n = head;

	if(n)
		RemNode(n);

	return(n);
}


// This function legally returns NULL if the list is empty.
ccMinNode * ccMinList::RemTail(void)
{
ccMinNode *n = tail;

	if(n)
		RemNode(n);

	return(n);
}

ccMinNode *ccMinList::FindNode(unsigned long ordinalnumber) const
{
unsigned short i = 0;
ccMinNode *n;
xbool running = TRUE;

	assert(ordinalnumber < numElements);

	n = GetHead();
	if( n )
	{
		while(running)
		{
			if(i == ordinalnumber || n == NULL)
				running = FALSE;
			else
			{
				n = n->GetNext();
				i++;
			}
		}
	}

	return(n);
}

void ccMinList::Purge(void)
{
ccMinNode *cmn;

	cmn = RemHead();
	while( cmn )
	{
		delete cmn;
		cmn = RemHead();
	}
}

void ccMinList::Sort( NodeCheckFlipFunc CheckFlip )
{
ccMinNode *n,*nn;
ccMinNode *first = NULL, *last = NULL;
xbool did_sort = TRUE,flip,running;

	n = GetHead();

	if (!n) return; //can't sort a list with 0 elems!

	do
	{
		// This is the forward direction pass

		if(did_sort)
		{
			did_sort = FALSE;
			running = TRUE;
			while( running )
			{

				nn = n->GetNext();

				if( nn == last || nn == NULL )
				{
					running = FALSE;
					last = n;
				}
				else
				{
					flip =  CheckFlip(n,nn);

					if( flip )
					{
						RemNode(n);
						AddNode(nn,n);
						did_sort = TRUE;
					}
					else
						n = nn;
				}
			}
		}

		// Backward pass

		if(did_sort)
		{
			did_sort = FALSE;

			n = last;

			running = TRUE;
			while( running )
			{

				nn = n->GetPrev();

				if( nn == first || nn == NULL )
				{
					running = FALSE;
					first = n;
				}
				else
				{
					flip =  CheckFlip(nn,n);

					if( flip )
					{
						RemNode(nn);
						AddNode(n,nn);
						did_sort = TRUE;
					}
					else
						n = nn;
				}
			}
			n = first;
		}

	} while ( did_sort );
}

ccList::ccList()
{
}

ccList::~ccList()
{
}


ccNode *ccList::FindNode(const char *name, ccNode *node) const
{
ccNode *retn = NULL;
unsigned long Hash;

	assert(name);

	Hash = CalcHash(name);

	while(node)
	{
		if(node->name)
		{
			if((Hash == node->nameHash) && (!(stricmp(node->name, name))) )
			{
				retn = node;
				break;		// Found it!
			}
		}
		node = node->GetNext();
	}

	return(retn);
}


ccNode *ccList::FindNode(const char *name) const
{
ccNode *n = GetHead();

	return(FindNode(name,n));
}

static xbool CheckAlpha(ccNode *n, ccNode *nn)
{
	return( (stricmp(n->GetName(), nn->GetName() )) > 0);
}

void ccList::SortAlpha()
{

	Sort(&(NodeCheckFlipFunc)CheckAlpha);
}


//-------------------------------------------------------------------
// ccHashNode stuff follows - new functionality for faster searches
//-------------------------------------------------------------------

unsigned long CalcHash(const char *String)
{
unsigned long Hash = 0;
unsigned long g;

	while(*String != 0)
	{
		Hash = (Hash << 4) + toupper(*String++);
		if ((g = Hash & 0xf0000000) != 0)
			Hash = (Hash ^ (g >> 24)) ^ g;
	}
	return Hash;
}


ccHashNode::ccHashNode()
{
	Name = NULL;
	myPtr = NULL;
	Hash = 0;
}

ccHashNode::ccHashNode( ccHashNode& hashnode )
{
	Name = NULL;
	myPtr = NULL;
	Hash = 0;

	this->SetName( hashnode.GetName() );
}


ccHashNode::~ccHashNode()
{
	if(Name) delete [] Name;
	if(myPtr) delete myPtr;
}


xbool ccHashNode::SetName(const char *newName)
{
xbool Retval = FALSE;
long	Len;

	if(Name)
	{
		delete [] Name;	// Get rid of the old one first, if any.
		Name = NULL;
		Hash = 0;
	}

	if(newName)
	{
		Len = strlen(newName);
		if(Len)
			Name = new char[Len+1];

		if(Name)
		{
			strcpy(Name,newName);

			//--Compute a hash value for the name-------
			Hash = CalcHash(Name);
			Retval = TRUE;
		}
	}

	return(Retval);
}

ccHashList::ccHashList()
{
	TableSize = 0;
	HashTable = NULL;
}

ccHashList::ccHashList(long NewTableSize)
{
	TableSize = NewTableSize;
	HashTable = new ccMinList[TableSize];
}

ccHashList::~ccHashList()
{
ccHashNode	*n;

	n = GetHead();
	while(n)
	{
		RemNode(n);
		delete n;

		n = GetHead();
	}

	if(HashTable) delete [] HashTable;
	HashTable = NULL;
}

void ccHashList::Purge(void)
{
ccHashNode	*n;

	n = GetHead();
	while(n)
	{
		RemNode(n);
		delete n;
		n = GetHead();
	}

	if(HashTable)
		delete [] HashTable;

	if(TableSize)
		HashTable = new ccMinList[TableSize];
	else
		HashTable = NULL;
}

void ccHashList::SetTableSize(long NewTableSize)
{
	if(HashTable)
		delete [] HashTable;

	TableSize = NewTableSize;

	if(NewTableSize)
		HashTable = new ccMinList[TableSize];
	else
		HashTable = NULL;
}

void ccHashList::AddHashEntry(ccHashNode *n)
{
unsigned long Bucket;

	n->myPtr = new ccHashNodePtr(n);
	Bucket = n->Hash % TableSize;

	HashTable[Bucket].AddTail(n->myPtr);
}

void ccHashList::RemHashEntry(ccHashNode *n)
{
unsigned long Bucket;

	if(n->myPtr)
	{
		Bucket = n->Hash % TableSize;

		HashTable[Bucket].RemNode(n->myPtr);
		delete n->myPtr;
		n->myPtr = NULL;
	}
}

void ccHashList::AddHead(ccHashNode *n)
{
	NodeList.AddHead(n);
	AddHashEntry(n);
}

void ccHashList::AddTail(ccHashNode *n)
{
	NodeList.AddTail(n);
	AddHashEntry(n);
}

void ccHashList::AddNode(ccHashNode *insertPoint, ccHashNode *n)
{
	NodeList.AddNode(insertPoint, n);
	AddHashEntry(n);
}

void ccHashList::RemNode(ccHashNode *n)
{
	if(NodeList.RemNode(n))
		RemHashEntry(n);
}

ccHashNode *ccHashList::Find(const char *FindName)
{
ccHashNode		*Node;
ccHashNodePtr	*Ptr;

	if( !HashTable )
		return NULL;

	Ptr = (ccHashNodePtr *)HashTable[ CalcHash(FindName) % TableSize ].GetHead();

	while(Ptr)
	{
		Node = Ptr->GetNodePtr();
		if( !(stricmp(Node->Name, FindName)) )
			return Node;
		else
			Ptr = (ccHashNodePtr *)Ptr->GetNext();
	}
	return NULL;
}

void ccHashList::Rename(ccHashNode *n, const char *NewName)
{
	if(n->myPtr)
	{
		RemHashEntry(n);
		n->SetName(NewName);
		AddHashEntry(n);
	}
	else
		n->SetName(NewName);
}
