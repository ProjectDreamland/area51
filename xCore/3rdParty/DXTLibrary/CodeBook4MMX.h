/*

	CodeBook4MMX.h - 4 byte MMX optimized Codebook class

*/

#ifndef CODEBOOK_H
#define CODEBOOK_H

#include "Table.h"

const long CodeSize = 4;
const long CodeDWORDSize = CodeSize/4;
const long HashSize = 2048;

class Image32;
class fCodebook;
class Color;


class cbVector
{
private:
	u8	pData[CodeSize];

public:
	cbVector() {;}
	~cbVector() {;}

	inline u8 *GetPtr(void) {return pData;}

	inline u8 &operator[](int i) {return pData[i];}
	cbVector &operator=(cbVector &Vect) {*(long *)pData = *(long *)Vect.pData; return *this;}
	int operator==(cbVector &Vect) {return *(long *)pData == *(long *)Vect.pData;}

	void Min(cbVector &Test1);
	void Max(cbVector &Test1);
	void MinMax(cbVector &Min, cbVector &Max);

	long Sum(void);			// Summation of the vector componants
	long Mag(void);			// Magnitude of the vector
	long InvMag(void);		// Magnitude of the vector ^ ffffffffff...

	void Diff(cbVector &Test1, cbVector &Test2);
	long __fastcall DiffMag(cbVector &Vect);	// Magnitude of the difference between this and Vect (Dist ^ 2)

	friend class CodeBook;
	friend class ImgCodeBook;
};


typedef struct
{
	long	Origin;
	long	AntiOrigin;
} DualDist;


class CodeBook
{
private:
	Table<cbVector>		VectList;
	Table<long>			usageCount;

public:
	virtual ~CodeBook() {;}

	void AddVector(cbVector &Vect);
	long FindVectorSlow(cbVector &Vect);
	long FindVectorSlow(cbVector &Vect, long &Error);
	long ClosestError(cbVector &Vect);
	long ClosestError(long UseCount, cbVector &Vect);

	CodeBook &operator=(fCodebook &Src);
	cbVector &operator[](int i) {return VectList[i];}
	long UsageCount(int i) {return usageCount[i];}

	inline long NumCodes(void) {return VectList.Count();}
	inline void SetSize(long Size) {VectList.Resize(Size); usageCount.Resize(Size);}
	inline void SetCount(long Count) {VectList.SetCount(Count); usageCount.SetCount(Count);}

	friend class ImgCodeBook;
	friend class fCodebook;
};


class ImgCodeBook : public CodeBook
{
private:
	Table<long>			HashValues;
	Table<long>			HashList[HashSize];
	Table<DualDist>		DistList;
	Table<long>			BrightList;

	void SortCodes(void);

public:
	ImgCodeBook();
	~ImgCodeBook();

	inline void SetSize(long Size) {VectList.Resize(Size); usageCount.Resize(Size); HashValues.Resize(Size);}
	void ReleaseAll(void);

	ImgCodeBook &operator=(fCodebook &Src) {CodeBook::operator=(Src); return *this;}

	void AddVector(cbVector &Vect);
	void AddVectorUnique(cbVector &Vect) {AddVector(Vect);}

	void FromImage(Image32 *pImg);
	void FromImageUnique(Image32 *pImg);

	void GenerateDistanceTables(void);
	long FindVector(cbVector &Vect);

	friend class fCodebook;
};

#endif
