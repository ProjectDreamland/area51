/*

	MedianCut.cpp

*/

#include "x_files.hpp"

#include "MedianCut.h"

MedianCut::MedianCut()
{
	pRoot = NULL;

	TreePool = new TreeNode[512];
	PoolAlloc = 512;
	PoolUsed = 0;
}

MedianCut::~MedianCut()
{
	ReleaseAll();
}

void MedianCut::ReleaseAll(void)
{
	LeafList.Allocate(0);
	pRoot = NULL;

	if(TreePool) delete [] TreePool;
	TreePool = NULL;
	PoolAlloc = PoolUsed = 0;
}

TreeNode *MedianCut::GetNewTreeNode(void)
{
	assert(PoolUsed < PoolAlloc);
	return TreePool + PoolUsed++;
}

void MedianCut::ResetTree(void)
{
	LeafList.Allocate(0);
	pRoot = NULL;

	for(int i=0; i<PoolUsed; i++)
	{
		TreePool[i].SplitAxis = -1;
		TreePool[i].pLessEqual = NULL;
		TreePool[i].pGreater = NULL;
	}
	PoolUsed = 0;
}

void MedianCut::BuildRootNode(CodeBook &Codes)
{
long i, Count;
VectPtr	*pList;

	Count = Codes.NumCodes();
	if(Count == 0) return;

	pRoot = GetNewTreeNode();
	pRoot->CodeList.SetCount(Count);
	pList = pRoot->CodeList.Addr(0);

	for(i=0; i<Count; i++)
	{
		pList[i].pVect = &Codes[i];
		pList[i].UsageCount = Codes.UsageCount(i);
	}

	pRoot->ComputeBounds();
	pRoot->ComputeError();

	LeafList.Allocate(256);
	LeafList.Insert(pRoot);
}

void MedianCut::BuildTree(CodeBook &Codes, long TreeSize)
{
xbool bFinished = FALSE;
TreeNode *pNode, *pLE, *pGT;
cbVector *pVect;
VectPtr *pList;
long Axis, Len, i, Split, Count, NumLeaves, Changed = 0;

	ResetTree();
	BuildRootNode(Codes);

	if(pRoot == NULL)
		bFinished = TRUE;

	NumLeaves = LeafList.Count();

	while(!bFinished)
	{
		pNode = GetFirstLeaf();

		Axis = pNode->LongAxis;
		Len = pNode->AxisLen;
		if(Len == 0) break;		// Couldn't find a split - Finished

		// Create two new tree nodes
		pLE = pNode->pLessEqual = GetNewTreeNode();
		pGT = pNode->pGreater = GetNewTreeNode();

		// Choose a split point for the parent node
		Split = pNode->SplitPoint;
		pNode->SplitAxis = (u8)Axis;
		pNode->SplitPoint = (u8)Split;

		// Move all nodes from the parent into the two children
		Count = pNode->CodeList.Count();
		pLE->CodeList.Resize(Count);
		pGT->CodeList.Resize(Count);

		pList = pNode->CodeList.Addr(0);
		for(i=0; i<Count; i++)
		{
			pVect = pList[i].pVect;

			if((*pVect)[Axis] <= Split)
				pLE->CodeList.Append(pList[i]);
			else
				pGT->CodeList.Append(pList[i]);
		}

		// Compute the min & max values of the two children
		pLE->ComputeBounds();
		pLE->ComputeError();

		pGT->ComputeBounds();
		pGT->ComputeError();

		pNode->CodeList.Resize(0);

		// Add the children to the leaf list and increment the leaf count
		LeafList.ExtractInsert( pLE );
		LeafList.Insert( pGT );
		NumLeaves++;

		if(NumLeaves == TreeSize)
			bFinished = TRUE;
	}

	// Index the nodes
	Count = LeafList.Count();
	for(i=0; i<Count; i++)
	{
		pNode = GetLeaf(i);
		pNode->Index = i;
	}
}


TreeNode *MedianCut::FindVector(cbVector &Vect)
{
TreeNode *pNode;
long	Axis, Point;

	pNode = pRoot;
	Axis = pNode->SplitAxis;
	while(Axis >= 0)
	{
		Point = pNode->SplitPoint;
		if(Vect[Axis] > Point)
			pNode = pNode->pGreater;
		else
			pNode = pNode->pLessEqual;

		Axis = pNode->SplitAxis;
	}

	return pNode;
}

TreeNode *MedianCut::FindVectorBest(cbVector &Vect)
{
TreeNode *pNode;
long i, Diff, BestIndex = 0, BestDiff = 0x7fffffff;

	for(i=0; i<GetCount(); i++)
	{
		pNode = GetLeaf(i);
		Diff = Vect.DiffMag(pNode->Center);
		if(Diff < BestDiff)
		{
			BestDiff = Diff;
			BestIndex = i;
		}
	}
	return GetLeaf(BestIndex);
}


TreeNode::TreeNode()
{
	SplitAxis = -1;
	pLessEqual = pGreater = NULL;
}

TreeNode::~TreeNode()
{
}

void TreeNode::ComputeBounds(void)
{
long	i, Count;
VectPtr	*pList;

	Count = CodeList.Count();
	if(Count)
	{
		pList = CodeList.Addr(0);
		Max = Min = *pList[0].pVect;

		for(i=0; i<Count; i++)
			pList[i].pVect->MinMax(Min, Max);

		Diff.Diff(Max, Min);
	}
	else
	{
		for(i=0; i<CodeSize; i++)
			Diff[i] = 0;
		__asm int 3;
	}
}

static int CompareBytes(const void *p1, const void *p2)
{
int b1, b2;

	b1 = *(u8 *)p1;
	b2 = *(u8 *)p2;

	return b1 - b2;
}

void TreeNode::ComputeError(void)
{
u32	Avg[CodeSize];
double	AxError[CodeSize], Error, LargestError;
long	i, j, Count, usageCount, Val, Total;
cbVector	*pVect;
VectPtr		*pList;

	Error = 0;
	LongAxis = 0;
	Count = CodeList.Count();
	if(Count)
	{
		if(Count > 1)
		{
			// Reset the error amounts & averages
			for(j=0; j<CodeSize; j++)
			{
				AxError[j] = 0;
				Avg[j] = 0;
			}
			Total = 0;

			pList = CodeList.Addr(0);
			for(i=0; i<Count; i++)
			{
				pVect = pList[i].pVect;
				usageCount = pList[i].UsageCount;
				Total += usageCount;

				for(j=0; j<CodeSize; j++)
					Avg[j] += (*pVect)[j] * usageCount;
			}

			for(j=0; j<CodeSize; j++)
			{
				Avg[j] = Avg[j] / Total;
				Center[j] = (u8)Avg[j];
			}

			for(i=0; i<Count; i++)
			{
			u8 *pByte;

				pVect = pList[i].pVect;
				pByte = pVect->GetPtr();
				usageCount = pList[i].UsageCount;

				for(j=0; j<CodeSize; j++)
				{
					Val = (long)pByte[j] - Avg[j];
					AxError[j] += Val * Val * usageCount;
				}
			}

			Error = LargestError = AxError[0];
			for(j=1; j<CodeSize; j++)
			{
				Error += AxError[j];
				if(AxError[j] > LargestError)
				{
					LongAxis = (u8)j;
					LargestError = AxError[j];
				}
			}
		}
		else
			Center = *CodeList[0].pVect;
	}

	SetKey(Error);
	AxisLen = Diff[LongAxis];
	SplitPoint = Center[LongAxis];
}

char TreeNode::LongestAxis(void)
{
u8 D, Axis = 0;
long i;

	D = Diff[0];
	for(i=1; i<CodeSize; i++)
	{
		if(Diff[i] > D)
		{
			D = Diff[i];
			Axis = (u8)i;
		}
	}

	return Axis;
}

xbool TreeNode::Encloses(cbVector &Vect)
{
long i;

	for(i=0; i<CodeSize; i++)
	{
		if( Vect[i] < Min[i] || Vect[i] > Max[i] )
			return FALSE;
	}

	return TRUE;
}
