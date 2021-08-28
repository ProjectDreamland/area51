/*

	CodeBook.cpp - CodeBook class functions for VQLib

*/

#include "x_files.hpp"

#include "CodeBook.h"
#include "fCodeBook.h"
#include <String.h>
#include <StdIO.h>
#include <Math.h>
#include "Image.h"


unsigned long isqrt(unsigned long x)
{
unsigned long r, s, t; 

	r = 0;
	for(t=0x40000000; t; t >>= 2)
	{
		s = r + t;
		if(s <= x) 
		{
			x -= s;
			r = s + t;
		}
		r >>= 1;
	}

	return r;
}


cbVector &cbVector::operator=(cbVector &Vect)
{
u32 *pSrc, *pDest;

	pSrc = (u32 *)Vect.pData;
	pDest = (u32 *)pData;

	for(int i=0; i<CodeDWORDSize; i++)
		pDest[i] = pSrc[i];

	return *this;
}

int cbVector::operator==(cbVector &Vect)
{
u32 *lpSrc, *lpDest;

	lpSrc = (u32 *)pData;
	lpDest = (u32 *)Vect.pData;

	for(int i=0; i<CodeDWORDSize; i++)
		if(lpSrc[i] != lpDest[i])
			return 0;
	return 1;
}

void cbVector::Min(cbVector &Test1)
{
long i;

	for(i=0; i<CodeSize; i++)
	{
		if(Test1.pData[i] < pData[i])
			pData[i] = Test1.pData[i];
	}
}

void cbVector::Max(cbVector &Test1)
{
long i;

	for(i=0; i<CodeSize; i++)
	{
		if(Test1.pData[i] > pData[i])
			pData[i] = Test1.pData[i];
	}
}


void cbVector::MinMax(cbVector &Min, cbVector &Max)
{
long i;

	for(i=0; i<CodeSize; i++)
	{
		if(pData[i] > Max[i]) Max[i] = pData[i];
		if(pData[i] < Min[i]) Min[i] = pData[i];
	}
}


void cbVector::Diff(cbVector &Test1, cbVector &Test2)
{
	for(int i=0; i<CodeSize; i++)
	{
		if(Test1[i] >= Test2[i])
			pData[i] = Test1[i] - Test2[i];
		else
			pData[i] = Test2[i] - Test1[i];
	}
}

long cbVector::DiffMag(cbVector &Vect)
{
long Val, Result = 0;

	for(int i=0; i<CodeSize; i++)
	{
		Val = (long)pData[i] - (long)Vect[i];
		Result += Val * Val;
	}
	return Result;
}

long cbVector::Sum(void)
{
long i, Result = 0;

	for(i=0; i<CodeSize; i++)
		Result += pData[i];

	return Result;
}

long cbVector::Mag(void)
{
long i, Val, Result = 0;

	for(i=0; i<CodeSize; i++)
	{
		Val = pData[i];
		Result += Val * Val;
	}
	return Result;
}

long cbVector::InvMag(void)
{
long i, Val, Result = 0;

	for(i=0; i<CodeSize; i++)
	{
		Val = (pData[i] ^ 0xff);
		Result += Val * Val;
	}
	return Result;
}


long CodeBook::FindVectorSlow(cbVector &Vect)
{
long		i, Count, Closest, ClosestIndex, TestMag;

	Count = VectList.Count();

	Closest = Vect.DiffMag(VectList[0]);
	ClosestIndex = 0;

	for(i=1; i<Count; i++)
	{
		TestMag = Vect.DiffMag(VectList[i]);
		if(TestMag < Closest)
		{
			Closest = TestMag;
			ClosestIndex = i;
			if(Closest == 0) break;
		}
	}

	return ClosestIndex;
}

void CodeBook::AddVector(cbVector &Vect)
{
long i, Count;

	Count = VectList.Count();
	for(i=0; i<Count; i++)
	{
		if(Vect == VectList[i])
		{
			usageCount[i]++;
			return;
		}
	}
	i = 1;
	VectList.Append(Vect, Count*2+1);
	usageCount.Append(i, Count*2+1);
}


CodeBook &CodeBook::operator=(fCodebook &Src)
{
fVectNode	*pNode;
cbVector	*pVect;
long		*pUsage;
long		i, Count;

	Count = Src.NumVectors();
	VectList.SetCount(Count);
	usageCount.SetCount(Count);

	if(Count)
	{
		pVect = VectList.Addr(0);
		pUsage = usageCount.Addr(0);
		pNode = Src.GetHead();
		while(pNode)
		{
			for(i=0; i<fCodeSize; i++)
				pVect->GetPtr()[i] = (u8)FtoL(pNode->V[i]);
			*pUsage = pNode->usageCount;

			pNode = pNode->GetNext();
			pVect++;
			pUsage++;
		}
	}
	return *this;
}



ImgCodeBook::ImgCodeBook()
{
}

ImgCodeBook::~ImgCodeBook()
{
	ReleaseAll();
}

void ImgCodeBook::ReleaseAll(void)
{
long i;

	VectList.Resize(0);
	usageCount.Resize(0);

	for(i=0; i<HashSize; i++)
		HashList[i].Resize(0);

	DistList.Resize(0);
	BrightList.Resize(0);
}


void ImgCodeBook::AddVector(cbVector &Vect)
{
long i, Count;
long Hash = 0, HashIndex;

	Hash = 0;
	for(i=0; i<CodeDWORDSize; i++)
		Hash ^= ((long *)Vect.pData)[i] << (i*2);

	HashIndex = (Hash % 49157) & (HashSize-1);

	Count = HashList[HashIndex].Count();
	if(Count)
	{
	long *pHashes, hi;

		pHashes = HashList[HashIndex].Addr(0);
		for(i=0; i<Count; i++)
		{
			hi = pHashes[i];
			if( HashValues[hi] == Hash && Vect == VectList[hi] )
			{
				usageCount[hi]++;
				return;
			}
		}
	}

	i = VectList.Append(Vect, 8192);
	Count = 1;
	usageCount.Append(Count, 8192);
	HashValues.Append(Hash, 8192);
	HashList[HashIndex].Append(i, 8);
}


void ImgCodeBook::FromImage(Image32 *pImg, Color *pForceColor)
{
long		Size;
Color		*pPix, *pDest;
long		x, y;
long		Width, Height;
cbVector	V;

	assert(CodeSize == 4);

	Width = pImg->GetXSize();
	Height = pImg->GetYSize();
	Size = Width * Height;
	SetSize(Size);

	pPix = pImg->GetPixels();
	pDest = (Color *)V.GetPtr();	// Aim a color pointer at our vector
	for(y=0; y<Height; y++)
	{
		for(x=0; x<Width; x++)
		{
			*pDest = pPix[x];		// Copy the color into the vector
			AddVector(V);
		}
		pPix += Width;
	}

	if(pForceColor != 0)
	{
		*pDest = *pForceColor;		// Copy the force color into the vector
		for(x=0; x<Size; x++)
			AddVector(V);			// add the force color as many times as there are pixels in the image, to make it important
	}
}

void ImgCodeBook::FromImageUnique(Image32 *pImg, Color* pForceColor)
{
long		Size;
Color		*pPix, *pDest;
long		x, y;
long		Width, Height;
cbVector	V, f;

	assert(CodeSize == 4);

	Width = pImg->GetXSize();
	Height = pImg->GetYSize();
	Size = Width * Height;
	SetSize(Size);

	pPix = pImg->GetPixels();
	for(y=0; y<Height; y++)
	{
		pDest = (Color *)V.GetPtr();
		for(x=0; x<Width; x++)
		{
			*pDest = pPix[x];
			AddVectorUnique(V);
		}
		pPix += Width;
	}

	if(pForceColor != 0)
	{
		*pDest = *pForceColor;		// Copy the force color into the vector
		for(x=0; x<Size; x++)
			AddVector(V);			// add the force color as many times as there are pixels in the image, to make it important
	}

//      char szText[512];
//	    wsprintf(szText, "Built %d unique codes\n", VectList.Count());
//	    OutputDebugString(szText);
}


typedef struct
{
	long	Key, OrigIndex;
} SortKey;

int CompareKeys(const void *pk1, const void *pk2)
{
	return ((SortKey *)pk1)->Key - ((SortKey *)pk2)->Key;
}


void ImgCodeBook::SortCodes(void)
{
SortKey		*pSortList;
cbVector	*pTempVects;
long		*pTempCounts;
long		i, Count, Bright;

	Count = VectList.Count();
	pSortList = new SortKey[Count];
	pTempVects = new cbVector[Count];
	pTempCounts = new long[Count];

	for(i=0; i<Count; i++)
	{
		Bright = VectList[i].Sum();
		pSortList[i].OrigIndex = i;
		pSortList[i].Key = Bright;
	}

	qsort(pSortList, Count, sizeof(SortKey), CompareKeys);
	memcpy(pTempVects, VectList.Addr(0), sizeof(cbVector) * Count);

	BrightList.SetCount(Count);
	for(i=0; i<Count; i++)
	{
		VectList[i] = pTempVects[ pSortList[i].OrigIndex ];
		usageCount[i] = pTempCounts[ pSortList[i].OrigIndex ];
		BrightList[i] = pSortList[i].Key;
	}

	delete [] pSortList;
	delete [] pTempVects;
	delete [] pTempCounts;
}

void ImgCodeBook::GenerateDistanceTables(void)
{
DualDist	*pDist;
long		i, Count;

	Count = VectList.Count();
	if(Count == 0)
		return;

	SortCodes();

	DistList.Resize(Count);
	pDist = DistList.Addr(0);
	for(i=0; i<Count; i++)
	{
		pDist[i].Origin = isqrt(VectList[i].Mag());
		pDist[i].AntiOrigin = isqrt(VectList[i].InvMag());
	}
}

long ImgCodeBook::FindVector(cbVector &Vect)
{
long	i, Count, Tested = 0, Closest, ClosestIndex, TestMag;
u32  	Dist, InvDist, Root;
long	OriginMin, OriginMax, AntiMin, AntiMax, Bright, bDiff, Scope;
DualDist	*pDist;
cbVector	Diff;

	Count = VectList.Count();

	Bright = Vect.Sum();
	i = Count >> 1;
	Scope = Count >> 2;

	while(Scope)
	{
		bDiff = Bright - BrightList[i];
		if(bDiff > 0)
			i += Scope;
		else if(bDiff < 0)
			i -= Scope;
		else
			break;

		Scope >>= 1;
	}

	Dist = isqrt(Vect.Mag());
	InvDist = isqrt(Vect.InvMag());

	Diff.Diff(Vect, VectList[0]);
	Closest = Diff.Mag();
	ClosestIndex = 0;

	Root = isqrt(Closest);
	OriginMin = Dist - Root;
	OriginMax = Dist + Root;
	AntiMin = InvDist - Root;
	AntiMax = InvDist + Root;

	pDist = DistList.Addr(0);
	for(i=1; i<Count; i++)
	{
		if(	pDist->AntiOrigin >= AntiMin && pDist->AntiOrigin <= AntiMax &&
			pDist->Origin >= OriginMin && pDist->Origin <= OriginMax )
		{
			Tested++;
			Diff.Diff(Vect, VectList[i]);
			TestMag = Diff.Mag();
			if(TestMag < Closest)
			{
				Closest = TestMag;
				ClosestIndex = i;
				if(Closest == 0) break;

				Root = isqrt(Closest);
				OriginMin = Dist - Root;
				OriginMax = Dist + Root;
				AntiMin = InvDist - Root;
				AntiMax = InvDist + Root;
			}
		}
		pDist++;
	}

	return ClosestIndex;
}
