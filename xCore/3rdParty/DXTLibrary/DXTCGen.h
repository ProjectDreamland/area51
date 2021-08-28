/*

	DXTCGen.h - Bounds & variance based codebook generator class

*/

#ifndef DXTCGEN_H_
#define DXTCGEN_H_

#include "CodeBook4MMX.h"

class DXTCGen
{
private:
	CodeBook	Vects;
	cbVector	*pVects;

	static void BuildCodes3(cbVector *pVects, cbVector &v1, cbVector &v2);
	static void BuildCodes4(cbVector *pVects, cbVector &v1, cbVector &v2);

	static void BuildCodes3(cbVector *pVects, long Channel, cbVector &v1, cbVector &v2);
	static void BuildCodes4(cbVector *pVects, long Channel, cbVector &v1, cbVector &v2);

	long ComputeError3(CodeBook &Pixels);
	long ComputeError4(CodeBook &Pixels);

public:
	DXTCGen();

	long Execute3(CodeBook &Source, CodeBook &Pixels, CodeBook &Dest);
	long Execute4(CodeBook &Source, CodeBook &Pixels, CodeBook &Dest);
};

#endif
