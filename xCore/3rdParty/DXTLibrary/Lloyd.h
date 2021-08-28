/*

	Lloyd.h - Lloyd codebook generator class

*/

#ifndef LLOYD_H_
#define LLOYD_H_

#include "fCodeBook.h"


class Lloyd
{
private:
	ccMinList	Codes;	// List of codebooks (the centroid of each will ultimately be our result codes)

public:
	long Execute(fCodebook &Source, fCodebook &Dest, long Target);
};

#endif
