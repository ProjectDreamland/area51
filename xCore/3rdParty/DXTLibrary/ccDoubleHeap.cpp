/*

	ccDoubleHeap.cpp - double precision Heap class functions

*/

#include "x_files.hpp"

#include "ccDoubleHeap.h"
#include <Assert.h>

ccDoubleHeap::ccDoubleHeap()
{
	pHeap = 0;
	Size = Allocated = 0;
}

ccDoubleHeap::~ccDoubleHeap()
{
	if(pHeap) delete [] pHeap;
}

void ccDoubleHeap::Allocate(long NumItems)
{
	if(pHeap) delete [] pHeap;
	pHeap = 0;

	if(NumItems)
		pHeap = new ccDoubleHeapNode *[NumItems+1];

	Allocated = NumItems;
	Size = 0;
}

void ccDoubleHeap::SiftUp(void)
{
ccDoubleHeapNode	*pTemp;
long				j, Index;

	Index = 1;
	j = 2;
	while(j <= Size)
	{
		if(j < Size)
		{
			if(pHeap[j]->Key < pHeap[j+1]->Key)
				j++;
		}

		if(pHeap[Index]->Key < pHeap[j]->Key)
		{
			pTemp = pHeap[j];
			pHeap[j] = pHeap[Index];
			pHeap[Index] = pTemp;
			Index = j;
		}
		else
			Index = Size+1;

		j = Index*2;
	}
}

void ccDoubleHeap::Insert(ccDoubleHeapNode *pNode)
{
long	i, j;
double	KeyTest = pNode->Key;

	assert(Size <= Allocated);

	Size++;
	j = Size;

	while(j > 1)
	{
		i = j/2;
		if(pHeap[i]->Key >= KeyTest)
			break;

		pHeap[j] = pHeap[i];
		j = i;
	}
	pHeap[j] = pNode;
}

ccDoubleHeapNode *ccDoubleHeap::Extract(void)
{
ccDoubleHeapNode *pResult;

	assert(Size > 0);

	pResult = pHeap[1];
	pHeap[1] = pHeap[Size];
	Size--;
	SiftUp();

	return pResult;
}

void ccDoubleHeap::ExtractInsert(ccDoubleHeapNode *pNode)
{
	pHeap[1] = pNode;
	SiftUp();
}
