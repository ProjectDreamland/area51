/*

	Lloyd.cpp - Implementation of the Lloyd algorithm for codebook generation

*/

#include "x_files.hpp"
#include <StdIO.h>
#include <StdLib.h>
#include <Math.h>
#include <Assert.h>

#include "Lloyd.h"

const long IterationLimit = 3;


long Lloyd::Execute(fCodebook &Source, fCodebook &Dest, long Target)
{
long		i, Count, CurCode, NumCodes, Iteration;
bool		bCanSplit;
fCodebook	*pList, *pDestList1, *pDestList2;
cfVector	Center, Center1, Center2, Jitter;
float		Dist1, Dist2;
fVectNode	*pNode, *pDestNode;
//    char		szText[128];

	srand(0);
	Count = Source.NumVectors();
	if(Count <= Target)
	{
		pNode = Source.GetHead();
		while(pNode)
		{
			pDestNode = new fVectNode;
			pDestNode->V = pNode->V;
			Dest.AddTail(pDestNode);
			pNode = pNode->GetNext();
		}
		return Count;
	}

	pList = new fCodebook;
	pNode = Source.GetHead();
	while(pNode)
	{
		pDestNode = new fVectNode;
		pDestNode->V = pNode->V;
		pList->AddTail(pDestNode);
		pNode = pNode->GetNext();
	}
	Codes.AddTail(pList);

	NumCodes = Codes.GetNumElements();
	CurCode = 0;
	while(Codes.GetNumElements() < (unsigned)Target)
	{
		pList = (fCodebook *)Codes.RemHead();
		if(pList->NumVectors() > 1)
		{
			if(pList->NumVectors() == 2)
			{
				pDestList1 = new fCodebook;
				pDestList2 = new fCodebook;

				pNode = pList->RemHead();
				pDestList1->AddTail( pNode );

				pNode = pList->RemHead();
				pDestList2->AddTail( pNode );

				delete pList;
				Codes.AddTail(pDestList1);
				Codes.AddTail(pDestList2);
			}
			else
			{
				pList->CalcCenter(Center);						// Compute the centroid of pList

				for(i=0; i<fCodeSize; i++)
					Jitter[i] = (float)rand() / (float)RAND_MAX;	// Compute a jitter amount

				// Create the two new centers
				Center1 = Center;	Center1 += Jitter;
				Center2 = Center;	Center2 -= Jitter;

				pDestList1 = new fCodebook;
				pDestList2 = new fCodebook;

				for(Iteration=0; Iteration<IterationLimit; Iteration++)
				{
					// Compute membership in the new lists based on the distance to centers
					pNode = pList->RemHead();
					while(pNode)
					{
						Dist1 = Center1.DiffMag(pNode->V);
						Dist2 = Center2.DiffMag(pNode->V);

						if(Dist1 < Dist2)
							pDestList1->AddTail(pNode);
						else
							pDestList2->AddTail(pNode);

						pNode = pList->RemHead();
					}

					if(Iteration < (IterationLimit-1))
					{
						// Compute centers for the new lists
						pDestList1->CalcCenter(Center1);
						pDestList2->CalcCenter(Center2);

						// Move the members back into the source list & repeat
						pNode = pDestList1->RemHead();
						while(pNode)
						{
							pList->AddTail(pNode);
							pNode = pDestList1->RemHead();
						}

						pNode = pDestList2->RemHead();
						while(pNode)
						{
							pList->AddTail(pNode);
							pNode = pDestList2->RemHead();
						}
					}
				}

				if(pDestList1->NumVectors())
					Codes.AddTail( pDestList1 );
				else
					delete pDestList1;

				if(pDestList2->NumVectors())
					Codes.AddTail( pDestList2 );
				else
					delete pDestList2;

				delete pList;
			}
		}
		else
			Codes.AddTail(pList);

		CurCode++;
		if(CurCode == NumCodes)
		{
			bCanSplit = false;
			pList = (fCodebook *)Codes.GetHead();
			while(pList)
			{
				if(pList->NumVectors() > 1)
				{
					bCanSplit = true;
					break;
				}
				pList = pList->GetNext();
			}

			if(bCanSplit == false)
				break;

			NumCodes = Codes.GetNumElements();
			CurCode = 0;

//			    wsprintf(szText, "%d codes\n", NumCodes);
//			    OutputDebugString(szText);
		}
	}

	NumCodes = Codes.GetNumElements();
//	    wsprintf(szText, "%d codes\n", NumCodes);
//	    OutputDebugString(szText);

	// Copy the Codes centroids into the Dest list
	pList = (fCodebook *)Codes.GetHead();
	while(pList)
	{
		pNode = new fVectNode;
		pNode->usageCount = pList->CalcCenter(pNode->V);	// Compute the centroid of pList
		Dest.AddTail(pNode);
		pList = pList->GetNext();
	}

	Codes.Purge();
	return Dest.NumVectors();
}
