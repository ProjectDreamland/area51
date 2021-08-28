/*

	ccHeap.h - Heap class

*/

#ifndef CCHEAP_H
#define CCHEAP_H

class ccHeap;

class ccHeapNode
{
private:
	long	Key;

public:
	ccHeapNode() {Key=0;}
	ccHeapNode(long NewKey) {Key = NewKey;}

	virtual ~ccHeapNode() {;}

	inline long GetKey(void) {return Key;}
	inline void SetKey(long NewKey) {Key = NewKey;}

	friend class ccHeap;
};

class ccHeap
{
private:
	long		Size, Allocated;
	ccHeapNode	**pHeap;

	void SiftUp(void);

public:
	ccHeap();
	~ccHeap();

	void Allocate(long NumItems);

	void Insert(ccHeapNode *pNode);				// Simply insert a new node
	ccHeapNode *Extract(void);					// Remove the head and return it

	void ExtractInsert(ccHeapNode *pNode);		// Removes the head, and inserts the new node  (faster)

	inline long Count(void) {return Size;}
	inline ccHeapNode *GetNode(long Index) {return pHeap[Index];}
};


#endif
