/*

	ccDoubleHeap.h - Double-precision Heap class

*/

#ifndef CCDOUBLEHEAP_H
#define CCDOUBLEHEAP_H

class ccDoubleHeap;


class ccDoubleHeapNode
{
private:
	double	Key;

public:
	ccDoubleHeapNode() {Key=0.0;}
	ccDoubleHeapNode(double NewKey) {Key = NewKey;}

	virtual ~ccDoubleHeapNode() {;}

	inline double GetKey(void) {return Key;}
	inline void SetKey(double NewKey) {Key = NewKey;}

	friend class ccDoubleHeap;
};


class ccDoubleHeap
{
private:
	long				Size, Allocated;
	ccDoubleHeapNode	**pHeap;

	void SiftUp(void);

public:
	ccDoubleHeap();
	~ccDoubleHeap();

	void Allocate(long NumItems);

	void Insert(ccDoubleHeapNode *pNode);			// Simply insert a new node
	ccDoubleHeapNode *Extract(void);				// Remove the head and return it

	void ExtractInsert(ccDoubleHeapNode *pNode);	// Removes the head, and inserts the new node  (faster)

	inline long Count(void) {return Size;}
	inline ccDoubleHeapNode *GetNode(long Index) {return pHeap[Index];}
};

#endif
