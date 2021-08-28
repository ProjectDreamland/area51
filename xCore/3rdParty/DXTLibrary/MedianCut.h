/*

	MedianCut.h - Median Cut quantization class

*/

#ifndef MEDIANCUT_H_
#define MEDIANCUT_H_

#include "CodeBook4MMX.h"
#include "ccDoubleHeap.h"

typedef struct
{
	cbVector	*pVect;
	long		UsageCount;
} VectPtr;

class TreeNode : public ccDoubleHeapNode
{
public:
	TreeNode();
	~TreeNode();

	long	Index;
	long	AxisLen;

	u8	    SplitPoint, Pad;
	char	SplitAxis , LongAxis;

	TreeNode	*pLessEqual;
	TreeNode	*pGreater;

	cbVector	Min, Max, Diff, Center;

	Table<VectPtr>	CodeList;

	void ComputeBounds(void);
	void ComputeError(void);

	char  LongestAxis(void);
	xbool Encloses(cbVector &Vect);
};


class MedianCut
{
private:
	TreeNode			*pRoot;
	ccDoubleHeap		LeafList;

	TreeNode			*TreePool;
	long				PoolAlloc, PoolUsed;

	void ReleaseAll(void);
	TreeNode *GetNewTreeNode(void);

	void BuildRootNode(CodeBook &Codes);

public:
	MedianCut();
~   MedianCut();

	inline TreeNode *GetFirstLeaf(void) {return (TreeNode *)LeafList.GetNode(1);}
	inline TreeNode *GetLeaf(long Index) {return (TreeNode *)LeafList.GetNode(Index+1);}

	inline long GetCount(void) {return LeafList.Count();}

	void ResetTree(void);
	void BuildTree(CodeBook &Codes, long TreeSize);

	TreeNode *FindVector(cbVector &Vect);
	TreeNode *FindVectorBest(cbVector &Vect);
};


#endif
