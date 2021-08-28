/*

	CodeBook.cpp - Codebook maintenance & generation routines

*/

#include "x_files.hpp"
#include <StdIO.h>
#include <Math.h>

#include "fCodeBook.h"
#include "CodeBook4MMX.h"

long fCodebook::CalcCenter(cfVector &Vect)
{
cfVector	Center;
fVectNode	*pNode;
long 		i, Count, Total = 0;
float		fCount;

	for(i=0; i<fCodeSize; i++)
		Center[i] = 0.0f;

	pNode = GetHead();
	while(pNode)
	{
		Count = pNode->usageCount;
		fCount = (float)Count;
		Total += Count;

		for(i=0; i<fCodeSize; i++)
			Center[i] += pNode->V[i] * fCount;

		pNode = pNode->GetNext();
	}

	Center /= (float)Total;
	Vect = Center;
	return Total;
}

long fCodebook::ClosestIndex(cfVector &Vect)
{
long		i, Result, Tested = 0;
float		CloseDist, TempDist;
cfVector	DistVect;
fVectNode	*pNode;

	if(NumVectors() == 0)
		return -1;

	pNode = GetHead();
	Result = 0;
	DistVect.Diff(Vect, pNode->V);
	CloseDist = DistVect.Mag();

	i = 1;
	pNode = pNode->GetNext();
	while(pNode)
	{
		DistVect.Diff(Vect, pNode->V);
		TempDist = DistVect.Mag();
		Tested++;
		if(TempDist < CloseDist)
		{
			CloseDist = TempDist;
			Result = i;
		}
		pNode = pNode->GetNext();
		i++;
	}

	return Result;
}

fCodebook &fCodebook::operator=(CodeBook &Src)
{
fVectNode	*pNode;
cbVector	*pVect;
long		i, Count;

	VectList.Purge();
	Count = Src.NumCodes();
	if(Count)
	{
		pVect = Src.VectList.Addr(0);
		for(i=0; i<Count; i++)
		{
			pNode = new fVectNode;
			for(int j=0; j<fCodeSize; j++)
				pNode->V[j] = pVect->GetPtr()[j];
			AddTail(pNode);
			pVect++;
		}
	}
	return *this;
}
