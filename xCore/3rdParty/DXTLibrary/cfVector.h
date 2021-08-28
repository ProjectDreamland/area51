/*

	cbVector.h - Codebook Vector classes and prototypes for VQ

*/

#ifndef CBVECTOR_H_
#define CBVECTOR_H_

const long fCodeSize = 4;

class cfVector
{
private:
	float pData[fCodeSize];

public:
	cfVector() {;}
	~cfVector() {;}

	inline float *GetPtr(void) {return pData;}

	inline float &operator[](int i) {return pData[i];}
	cfVector &operator=(cfVector &Vect);

	cfVector &operator+=(cfVector &Vect);
	cfVector &operator-=(cfVector &Vect);
	cfVector &operator*=(cfVector &Vect);
	cfVector &operator/=(cfVector &Vect);
	cfVector &operator*=(float f);
	cfVector &operator/=(float f);

	void Sum(cfVector &Vect1, cfVector &Vect2);
	void Diff(cfVector &Vect1, cfVector &Vect2);

	cfVector &operator=(float f);

	float DiffMag(cfVector &Vect);
	float Distance(cfVector &Vect);

	int operator==(cfVector &Vect);

	void Min(cfVector &Test1);
	void Max(cfVector &Test1);
	void MinMax(cfVector &Min, cfVector &Max);

	float Mag(void);			// Magnitude of the vector
	float InvMag(void);			// Inverse Magnitude of the vector (Dist from 255.0 .... )
};


#endif
