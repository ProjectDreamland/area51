/*

	fCodeBook.h - Float Codebook classes and prototypes for VQ

*/

#ifndef FCODEBOOK_H_
#define FCODEBOOK_H_

#include "cfVector.h"
#include "ccList.h"

class CodeBook;


class fVectNode : public ccMinNode
{
public:
	cfVector	V;
	long		usageCount, Hash;

	inline fVectNode *GetNext(void) {return (fVectNode *)ccMinNode::GetNext();}
	inline fVectNode *GetPrev(void) {return (fVectNode *)ccMinNode::GetNext();}
};


class fCodebook : public ccMinNode
{
private:
	ccMinList VectList;

public:
	long NumVectors(void) {return VectList.GetNumElements();}
	fCodebook &operator=(CodeBook &Src);

	fVectNode *GetHead(void) {return (fVectNode *)VectList.GetHead();}
	fVectNode *RemHead(void) {return (fVectNode *)VectList.RemHead();}

	void Purge(void) {VectList.Purge();}
	void AddTail(fVectNode *pNode) {VectList.AddTail(pNode);}

	fVectNode *ClosestVect(cfVector &Vect);
	long ClosestIndex(cfVector &Vect);

	long CalcCenter(cfVector &Vect);	// Returns the number of vectors summed

	inline fCodebook *GetNext(void) {return (fCodebook *)ccMinNode::GetNext();}
	inline fCodebook *GetPrev(void) {return (fCodebook *)ccMinNode::GetNext();}
	friend class CodeBook;
	friend class ImgCodeBook;
};

#endif
