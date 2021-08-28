/*

	Image.cpp - Generic 24 bit image class functions

*/

#include "x_files.hpp"
#include "Math.h"

#include "Image.h"
#include "CodeBook4MMX.h"
#include "fCodeBook.h"
#include "Lloyd.h"
#include "MedianCut.h"

QuantMethodType Image::QuantMethod = QM_MedianCut;
bool Image::QuantDiffusion = false;


double Image::Diff(Image *pComp)
{
double Result;

	if(GetXSize() != pComp->GetXSize() || GetYSize() != pComp->GetYSize())
		return 1000000000000000.0;	// BIG number

	Result = 0;
	if(GetType() == Type_32Bit && pComp->GetType() == Type_32Bit)
	{
	Image32 *pImg1, *pImg2;
	u8	*pSrc1, *pSrc2;
	u32	Count, Count2, TempResult = 0;

		pImg1 = (Image32 *)this;
		pSrc1 = (u8 *)pImg1->GetPixels();
		pImg2 = (Image32 *)pComp;
		pSrc2 = (u8 *)pImg2->GetPixels();

		Count = XSize * YSize;
		while(Count)
		{
			Count2 = Count;
			if(Count2 > 0x7fffff)
				Count2 = 0x7fffff;

			Count -= Count2;

			__asm {
			mov			eax, dword ptr pSrc1;
			mov			ebx, dword ptr pSrc2;
			mov			ecx, [Count2]
$Loop:
			movd		mm0, [eax]
			movd		mm1, [ebx]
			pxor		mm2, mm2

			punpcklbw	mm0, mm2
			punpcklbw	mm1, mm2

			add			eax, 4
			psubsw		mm0, mm1

			add			ebx, 4
			pmaddwd		mm0, mm0

			movq		mm1, mm0
			psrlq		mm0, 32
			paddd		mm0, mm1
			movd		edx, mm0

			add			[TempResult], edx

			dec			ecx
			jg			$Loop
			emms
			}

			Result += TempResult;
			TempResult = 0;
		}

		/*for(i=0; i<Count; i++)
		{
			Val = abs((long)pSrc1[i] - (long)pSrc2[i]);
			Val *= Val;
			Result += Val;
		}*/
	}
	else if(GetType() == Type_8Bit && pComp->GetType() == Type_8Bit)
	{
	Image8	*pImg1, *pImg2;
	Color	*pPal1, *pPal2;
	u8	*pSrc1, *pSrc2;
	u32	i, Count, TempResult;
	Color	c1, c2;

		pImg1 = (Image8 *)this;
		pSrc1 = pImg1->GetPixels();
		pPal1 = pImg1->GetPalette();
		pImg2 = (Image8 *)pComp;
		pSrc2 = pImg2->GetPixels();
		pPal2 = pImg2->GetPalette();

		Count = XSize * YSize;

		for(i=0; i<Count; i++)
		{
			c1 = pPal1[pSrc1[i]];
			c2 = pPal2[pSrc2[i]];

			__asm {
			movd		mm0, [c1]
			movd		mm1, [c2]
			pxor		mm2, mm2

			punpcklbw	mm0, mm2
			punpcklbw	mm1, mm2

			add			eax, 4
			psubsw		mm0, mm1

			add			ebx, 4
			pmaddwd		mm0, mm0

			movq		mm1, mm0
			psrlq		mm0, 32
			paddd		mm0, mm1
			movd		[TempResult], mm0
			emms
			}

			/*
			Val = (long)c1.a - (long)c2.a;
			Result += (Val * Val);

			Val = (long)c1.r - (long)c2.r;
			Result += (Val * Val);

			Val = (long)c1.g - (long)c2.g;
			Result += (Val * Val);

			Val = (long)c1.b - (long)c2.b;
			Result += (Val * Val);
			*/
			Result += TempResult;
		}
	}
	else
	{
	Image8	*pImg1;
	Image32	*pImg2;
	Color	*pPal1, *pSrc2;
	u8	*pSrc1;
	u32	i, Count, TempResult;
	Color	c1, c2;

		if(GetType() == Type_8Bit)
		{
			pImg1 = (Image8 *)this;
			pImg2 = (Image32 *)pComp;
		}
		else
		{
			pImg1 = (Image8 *)pComp;
			pImg2 = (Image32 *)this;
		}

		pSrc1 = pImg1->GetPixels();
		pPal1 = pImg1->GetPalette();
		pSrc2 = pImg2->GetPixels();

		Count = XSize * YSize;

		for(i=0; i<Count; i++)
		{
			c1 = pPal1[pSrc1[i]];
			c2 = pSrc2[i];

			/*
			Val = (long)c1.a - (long)c2.a;
			Result += (Val * Val);

			Val = (long)c1.r - (long)c2.r;
			Result += (Val * Val);

			Val = (long)c1.g - (long)c2.g;
			Result += (Val * Val);

			Val = (long)c1.b - (long)c2.b;
			Result += (Val * Val);
			*/
			__asm {
			movd		mm0, [c1]
			movd		mm1, [c2]
			pxor		mm2, mm2

			punpcklbw	mm0, mm2
			punpcklbw	mm1, mm2

			add			eax, 4
			psubsw		mm0, mm1

			add			ebx, 4
			pmaddwd		mm0, mm0

			movq		mm1, mm0
			psrlq		mm0, 32
			paddd		mm0, mm1
			movd		[TempResult], mm0
			emms
			}
			Result += TempResult;
		}
	}
	return Result;
}

double Image::MSE(Image *pComp)
{
double diff;

	diff = Diff(pComp);
	diff /= (double)(XSize * YSize * 4);
	return diff;
}

AlphaType Image::AlphaUsage(unsigned char *pAlpha1, unsigned char *pAlpha0)
{
long	x, y;
long	Usage[256];
long	Unique;
Color	*pPix;

	x_memset(Usage, 0, sizeof(Usage));		// Reset the usage array

	// Count all the different values used
	if(GetType() == Type_8Bit)
	{
		pPix = ((Image8 *)this)->GetPalette();
		for(x=0; x<((Image8 *)this)->GetNumColors(); x++)
			Usage[pPix[x].a]++;
	}
	else if(GetType() == Type_32Bit)
	{
		pPix = ((Image32 *)this)->GetPixels();
		for(y=0; y<YSize; y++)
		{
			for(x=0; x<XSize; x++)
				Usage[pPix[x].a]++;

			pPix += XSize;
		}
	}


	// Count the number of unique alpha values
	Unique = 0;
	for(x=0; x<256; x++)
		Unique += (Usage[x] != 0);

	if(pAlpha1) *pAlpha1 = 0xff;
	if(pAlpha0) *pAlpha0 = 0x00;

	// Based on the unique alphas, classify the image
	switch(Unique)
	{
	case 0:	// Only possible if the texture is of size 0 x 0
		return AT_None;

	case 1:
		if(Usage[0xff])
			return AT_None;
		else
		{
			if(pAlpha1)
			{
				if(GetType() == Type_32Bit)
					*pAlpha1 = ((Image32 *)this)->GetPixels()[0].a;
				else if(GetType() == Type_8Bit)
					*pAlpha1 = ((Image8 *)this)->GetPalette()[0].a;
			}
			return AT_Constant;
		}

	case 2:
		if(Usage[0] && Usage[0xff])
			return AT_Binary;
		else if(Usage[0])
		{
			if(pAlpha1)	// Find the alpha value
			{
				for(x=1; x<256; x++)
					if(Usage[x])
						*pAlpha1 = (unsigned char)x;
			}
			return AT_ConstantBinary;
		}
		else
		{
			if(pAlpha0 && pAlpha1)
			{
				x = 0;
				while(x<256 && Usage[x] == 0)
					x++;
				*pAlpha0 = (unsigned char)x;

				x++;
				while(x<256 && Usage[x] == 0)
					x++;
				*pAlpha1 = (unsigned char)x;
			}
			return AT_DualConstant;
		}
		break;

	default:
		return AT_Modulated;
	}
}

void Image::AlphaToBinary(unsigned char Threshold)
{
long	x, y;
Color	*pPix;
unsigned char NewAlpha[256];

	x_memset(NewAlpha, 0, Threshold);
	x_memset(NewAlpha+Threshold, 255, 256-Threshold);

	if(GetType() == Type_8Bit)
	{
		pPix = ((Image8 *)this)->GetPalette();
		for(x=0; x<((Image8 *)this)->GetNumColors(); x++)
			pPix[x].a = NewAlpha[ pPix[x].a ];
	}
	else if(GetType() == Type_32Bit)
	{
		pPix = ((Image32 *)this)->GetPixels();
		for(y=0; y<YSize; y++)
		{
			for(x=0; x<XSize; x++)
				pPix[x].a = NewAlpha[ pPix[x].a ];

			pPix += XSize;
		}
	}
}


Image8::Image8()
{
	pPixels = 0;
	pPalette = 0;
	XSize = YSize = 0;
	NumCols = 0;
}

Image8::~Image8()
{
	ReleaseAll();
}

void Image8::ReleaseAll(void)
{
	if(pPixels) delete [] pPixels;
	pPixels = 0;
	XSize = YSize = 0;
	if(pPalette) delete [] pPalette;
	NumCols = 0;
}

void Image8::SetSize(long x, long y)
{
	if(pPixels) delete [] pPixels;
	pPixels = 0;
	XSize = YSize = 0;

	XSize = x;
	YSize = y;
	if(x * y)
		pPixels = new u8[XSize * YSize];
}

void Image8::SetNumColors(long Cols)
{
	if(pPalette)
	{
		delete [] pPalette;
		pPalette = 0;
	}
	NumCols = Cols;
	if(NumCols)
		pPalette = new Color[NumCols];
}


void Image8::QuantizeFrom(Image32 *pSrcImg, long QuantCols)
{
	if(Image::QuantMethod == QM_Lloyd)
	{
	ImgCodeBook	Source, Dest;
	fCodebook	fSource, fDest;
	Lloyd		Quant;
	cbVector	*pVect;
	long		i, x, y;
	Color		*pSrcPix;
	u8		*pDestPix;

		Source.FromImage(pSrcImg);
		fSource = Source;
		Quant.Execute(fSource, fDest, QuantCols);

		Dest = fDest;
		Dest.GenerateDistanceTables();
		SetNumColors( Dest.NumCodes() );
		SetSize(pSrcImg->XSize, pSrcImg->YSize);

		for(i=0; i<NumCols; i++)
			pPalette[i].Col = *(long *)&Dest[i];

		pSrcPix = pSrcImg->GetPixels();
		pDestPix = pPixels;

		for(y=0; y<YSize; y++)
		{
			pVect = (cbVector *)pSrcPix;
			for(x=0; x<XSize; x++)
				pDestPix[x] = (u8)Dest.FindVector(pVect[x]);

			pSrcPix += XSize;
			pDestPix += XSize;
		}
	}
	else if(Image::QuantMethod == QM_MedianCut)
	{
	ImgCodeBook	Source;
	MedianCut	Quant;
	TreeNode	*pNode;
	cbVector	*pVect;
	long		i, x, y;
	Color		*pSrcPix;
	u8		*pDestPix;

		Source.FromImage(pSrcImg);
		Quant.BuildTree(Source, QuantCols);

		SetNumColors( Quant.GetCount() );
		SetSize(pSrcImg->XSize, pSrcImg->YSize);

		for(i=0; i<NumCols; i++)
		{
			pNode = Quant.GetLeaf(i);
			pPalette[i].Col = *(long *)(&pNode->Center);
		}

		pSrcPix = pSrcImg->GetPixels();
		pDestPix = pPixels;

		if(QuantDiffusion == false)
		{
			for(y=0; y<YSize; y++)
			{
				pVect = (cbVector *)pSrcPix;
				for(x=0; x<XSize; x++)
				{
					pNode = Quant.FindVectorBest(pVect[x]);
					pDestPix[x] = (u8)pNode->Index;
				}

				pSrcPix += XSize;
				pDestPix += XSize;
			}
		}
		else
			pSrcImg->DiffuseQuant(*this);
	}
}

void Image8::QuantizeFrom(Image32 *pSrcImg, Image32 *pPaletteImg, Color *pForceColor)
{
long ColorsWanted = 255 + (pForceColor == 0);

	if(pPaletteImg == 0)
		pPaletteImg = pSrcImg;

	if(Image::QuantMethod == QM_Lloyd)
	{
	ImgCodeBook	Source, Dest;
	fCodebook	fSource, fDest;
	Lloyd		Quant;
	cbVector	*pVect;
	long		i, x, y;
	Color		*pSrcPix;
	u8		*pDestPix;

		Source.FromImage(pPaletteImg);
		fSource = Source;
		Quant.Execute(fSource, fDest, ColorsWanted);

		Dest = fDest;
		Dest.GenerateDistanceTables();
		if(pForceColor)
			SetNumColors( 256 );
		else
			SetNumColors( Dest.NumCodes() );
		SetSize(pSrcImg->XSize, pSrcImg->YSize);

		for(i=0; i<Dest.NumCodes(); i++)
			pPalette[i].Col = *(long *)&Dest[i];

		if(pForceColor)
			pPalette[255] = *pForceColor;

		pSrcPix = pSrcImg->GetPixels();
		pDestPix = pPixels;

		for(y=0; y<YSize; y++)
		{
			pVect = (cbVector *)pSrcPix;
			for(x=0; x<XSize; x++)
				pDestPix[x] = (u8)Dest.FindVector(pVect[x]);

			pSrcPix += XSize;
			pDestPix += XSize;
		}
	}
	else if(Image::QuantMethod == QM_MedianCut)
	{
	ImgCodeBook	Source;
	MedianCut	Quant;
	TreeNode	*pNode;
	cbVector	*pVect;
	long		i, x, y;
	Color		*pSrcPix;
	u8		*pDestPix;

		Source.FromImage(pPaletteImg);
		Quant.BuildTree(Source, ColorsWanted);

		SetNumColors( Quant.GetCount() );
		SetSize(pSrcImg->XSize, pSrcImg->YSize);

		for(i=0; i<NumCols; i++)
		{
			pNode = Quant.GetLeaf(i);
			pPalette[i].Col = *(long *)(&pNode->Center);
		}

		pSrcPix = pSrcImg->GetPixels();
		pDestPix = pPixels;

		if(QuantDiffusion == false)
		{
			for(y=0; y<YSize; y++)
			{
				pVect = (cbVector *)pSrcPix;
				for(x=0; x<XSize; x++)
				{
					pNode = Quant.FindVectorBest(pVect[x]);
					pDestPix[x] = (u8)pNode->Index;
				}

				pSrcPix += XSize;
				pDestPix += XSize;
			}
		}
		else
			pSrcImg->DiffuseQuant(*this);

		if(pForceColor)
		{
			// Re-copy the palette, including the added color
			SetNumColors( 256 );
			SetSize(pSrcImg->XSize, pSrcImg->YSize);

			for(i=0; i<Quant.GetCount(); i++)
			{
				pNode = Quant.GetLeaf(i);
				pPalette[i].Col = *(long *)(&pNode->Center);
			}
			pPalette[255] = *pForceColor;
		}
	}
}


Image8 &Image8::operator=(Image &Src)
{
	if(Src.GetType() == Type_8Bit)
	{
	Image8 *pSrc = (Image8 *)&Src;

		SetSize(pSrc->XSize, pSrc->YSize);
		SetNumColors(pSrc->NumCols);
		x_memcpy(pPixels, pSrc->pPixels, XSize*YSize);
		x_memcpy(pPalette, pSrc->pPalette, NumCols*4);
	}
	else if(Src.GetType() == Type_32Bit)
	{
		QuantizeFrom( (Image32 *)&Src );
	}
	return *this;
}

bool Image8::Crop(long x1, long y1, long x2, long y2)
{
u8 *pNewPix, *pSrc, *pDest;
long NewX, NewY, x, y;

	if(x2 < x1 || y2 < y1)
		return false;

	NewX = x2 - x1 + 1;
	NewY = y2 - y1 + 1;

	pNewPix = new u8[NewX * NewY];
	pSrc = pPixels + y1 * XSize;
	pDest = pNewPix;
	for(y=y1; y<y2; y++)
	{
		for(x=x1; x<x2; x++)
			*pDest++ = pSrc[x];

		pSrc += XSize;
	}
	delete [] pPixels;
	pPixels = pNewPix;
	XSize = NewX;
	YSize = NewY;
	return true;
}

bool Image8::SizeCanvas(long NewX, long NewY)
{
u8 *pNewPix, *pSrc, *pDest;
long x, y;
long XRun, YRun;

	pNewPix = new u8[NewX * NewY];
	x_memset(pNewPix, 0, NewX * NewY);

	XRun = __min(XSize, NewX);
	YRun = __min(YSize, NewY);

	pSrc = pPixels;
	pDest = pNewPix;
	for(y=0; y<YRun; y++)
	{
		for(x=0; x<XRun; x++)
			pDest[x] = pSrc[x];

		pDest += NewX;
		pSrc += XSize;
	}
	delete [] pPixels;
	pPixels = pNewPix;
	XSize = NewX;
	YSize = NewY;
	return true;
}



Image32::Image32()
{
	pPixels = 0;
	XSize = YSize = 0;
}

Image32::~Image32()
{
	ReleaseAll();
}

void Image32::ReleaseAll(void)
{
	if(pPixels) delete [] pPixels;
	pPixels = 0;
	XSize = YSize = 0;
}

void Image32::SetSize(long x, long y)
{
	ReleaseAll();

	XSize = x;
	YSize = y;
	if(x * y)
		pPixels = new Color[XSize * YSize];
}

long Image32::UniqueColors(void)
{
ImgCodeBook	Codes;

	Codes.FromImageUnique(this);
	return Codes.NumCodes();
}

float Image32::AverageSlope(void)
{
long	x, y, YMinus1, XMinus1, XPlus1;
Color	*pPix;
double	Slope = 0;
long	r, g, b, s, Count;

	XMinus1 = XSize-1;
	XPlus1 = XSize+1;
	YMinus1 = YSize-1;

	pPix = pPixels;
	Count = 0;
	for(y=0; y<YMinus1; y++)
	{
		for(x=0; x<XMinus1; x++)
		{
			r = abs((long)pPix[x].r - (long)pPix[x+XPlus1].r);
			g = abs((long)pPix[x].g - (long)pPix[x+XPlus1].g);
			b = abs((long)pPix[x].b - (long)pPix[x+XPlus1].b);

			s = r+g+b;
			if(s)
			{
				Slope += s;
				Count++;
			}
		}
		pPix += XSize;
	}

	Slope /= (double)Count;
	return (float)Slope;
}

Image32 &Image32::operator=(Image &Src)
{
	if(Src.GetType() == Type_32Bit)
	{
	Image32 *pSrc = (Image32 *)&Src;

		SetSize(pSrc->XSize, pSrc->YSize);
		x_memcpy(pPixels, pSrc->pPixels, XSize*YSize*4);
	}
	else if(Src.GetType() == Type_8Bit)
	{
	Image8	*pSrc = (Image8 *)&Src;
	Color	*pPal, *pDest;
	long	x, y, XSize, YSize;
	u8	*pSrcPix;

		XSize = pSrc->XSize;
		YSize = pSrc->YSize;
		SetSize(XSize, YSize);

		pPal = pSrc->GetPalette();
		pSrcPix = pSrc->GetPixels();
		pDest = pPixels;

		for(y=0; y<YSize; y++)
		{
			for(x=0; x<XSize; x++)
				pDest[x] = pPal[pSrcPix[x]];

			pDest += XSize;
			pSrcPix += XSize;
		}
	}
	return *this;
}

typedef struct
{
	short	r, g, b, a;
} ShortCol;

inline u8 Clamp(short Val)
{
	if(Val > 255) Val = 255;
	else if(Val < 0) Val = 0;
	return (u8)Val;
}

void Image32::DiffuseError(long aBits, long rBits, long gBits, long bBits)
{
long x, y;
ShortCol *pTempPix, *pDest;
short r, g, b, a, rErr, gErr, bErr, aErr;
short rMask, gMask, bMask, aMask;
Color *pSrc;

	rMask = (1 << (12-rBits)) - 1;
	gMask = (1 << (12-gBits)) - 1;
	bMask = (1 << (12-bBits)) - 1;
	aMask = (1 << (12-aBits)) - 1;

	pTempPix = new ShortCol[XSize * YSize];
	pDest = pTempPix;
	pSrc = pPixels;

	for(y=0; y<YSize; y++)
	{
		for(x=0; x<XSize; x++)
		{
			r = pSrc[x].r;
			g = pSrc[x].g;
			b = pSrc[x].b;
			a = pSrc[x].a;
			pDest[x].r = r << 4;
			pDest[x].g = g << 4;
			pDest[x].b = b << 4;
			pDest[x].a = a << 4;
		}
		pSrc += XSize;
		pDest += XSize;
	}

	pDest = pTempPix;
	for(y=0; y<YSize-1; y++)
	{
		for(x=0; x<XSize-1; x++)
		{
			r = pDest[x].r;
			g = pDest[x].g;
			b = pDest[x].b;
			a = pDest[x].a;

			rErr = r - ((r + rMask/2) & ~rMask);
			gErr = g - ((g + gMask/2) & ~gMask);
			bErr = b - ((b + bMask/2) & ~bMask);
			aErr = a - ((a + aMask/2) & ~aMask);

			r -= rErr;
			g -= gErr;
			b -= bErr;
			a -= aErr;

			pDest[x].r = r;
			pDest[x].g = g;
			pDest[x].b = b;
			pDest[x].a = a;

			pDest[x+1].r += rErr / 2;
			pDest[x+1].g += gErr / 2;
			pDest[x+1].b += bErr / 2;
			pDest[x+1].a += aErr / 2;

			pDest[x+XSize].r += rErr / 4;
			pDest[x+XSize].g += gErr / 4;
			pDest[x+XSize].b += bErr / 4;
			pDest[x+XSize].a += aErr / 4;

			if(x)
			{
				pDest[x+XSize-1].r += rErr / 8;
				pDest[x+XSize-1].g += gErr / 8;
				pDest[x+XSize-1].b += bErr / 8;
				pDest[x+XSize-1].a += aErr / 8;

				if(x > 2)
				{
					pDest[x+XSize-3].r += rErr / 8;
					pDest[x+XSize-3].g += gErr / 8;
					pDest[x+XSize-3].b += bErr / 8;
					pDest[x+XSize-3].a += aErr / 8;
				}
			}
		}

		r = pDest[x].r;
		g = pDest[x].g;
		b = pDest[x].b;
		a = pDest[x].a;

		rErr = r - ((r + rMask/2) & ~rMask);
		gErr = g - ((g + gMask/2) & ~gMask);
		bErr = b - ((b + bMask/2) & ~bMask);
		aErr = a - ((a + aMask/2) & ~aMask);

		r -= rErr;
		g -= gErr;
		b -= bErr;
		a -= aErr;

		pDest[x].r = r;
		pDest[x].g = g;
		pDest[x].b = b;
		pDest[x].a = a;

		pDest += XSize;
	}

	for(x=0; x<XSize; x++)
	{
		r = pDest[x].r;
		g = pDest[x].g;
		b = pDest[x].b;
		a = pDest[x].a;

		rErr = r - ((r + rMask/2) & ~rMask);
		gErr = g - ((g + gMask/2) & ~gMask);
		bErr = b - ((b + bMask/2) & ~bMask);
		aErr = a - ((a + aMask/2) & ~aMask);

		r -= rErr;
		g -= gErr;
		b -= bErr;
		a -= aErr;

		pDest[x].r = r;
		pDest[x].g = g;
		pDest[x].b = b;
		pDest[x].a = a;
	}

	pDest = pTempPix;
	pSrc = pPixels;
	rMask >>= 4;
	gMask >>= 4;
	bMask >>= 4;
	aMask >>= 4;
	for(y=0; y<YSize; y++)
	{
		for(x=0; x<XSize; x++)
		{
			pSrc[x].r = Clamp(pDest[x].r >> 4) & ~rMask;
			pSrc[x].g = Clamp(pDest[x].g >> 4) & ~gMask;
			pSrc[x].b = Clamp(pDest[x].b >> 4) & ~bMask;
			pSrc[x].a = Clamp(pDest[x].a >> 4) & ~aMask;
		}
		pSrc += XSize;
		pDest += XSize;
	}
	delete [] pTempPix;
}


void Image32::DiffuseQuant(Image8 &Dest)	// Dest image should already contain the palette
{
CodeBook Pal;
cbVector dVect, sVect;
long x, y, i;
ShortCol *pTempPix, *pTemp;
short r, g, b, a, rErr, gErr, bErr, aErr;
Color *pSrc;
u8 *pDest;

	Pal.SetCount(Dest.GetNumColors());
	for(i=0; i<Dest.GetNumColors(); i++)
		Pal[i] = *(cbVector *)&(Dest.GetPalette()[i].Col);

	pTempPix = new ShortCol[XSize * YSize];
	pTemp = pTempPix;
	pSrc = pPixels;

	for(y=0; y<YSize; y++)
	{
		for(x=0; x<XSize; x++)
		{
			r = pSrc[x].r;
			g = pSrc[x].g;
			b = pSrc[x].b;
			a = pSrc[x].a;
			pTemp[x].r = r << 4;
			pTemp[x].g = g << 4;
			pTemp[x].b = b << 4;
			pTemp[x].a = a << 4;
		}
		pSrc += XSize;
		pTemp += XSize;
	}

	pTemp = pTempPix;
	pDest = Dest.GetPixels();
	for(y=0; y<YSize-1; y++)
	{
		for(x=0; x<XSize-1; x++)
		{
			a = pTemp[x].a;
			r = pTemp[x].r;
			g = pTemp[x].g;
			b = pTemp[x].b;

			sVect[0] = Clamp((a + 8) >> 4);
			sVect[1] = Clamp((r + 8) >> 4);
			sVect[2] = Clamp((g + 8) >> 4);
			sVect[3] = Clamp((b + 8) >> 4);

			i = Pal.FindVectorSlow(sVect);
			dVect = Pal[i];

			aErr = a - ((short)dVect[0] << 4);
			rErr = r - ((short)dVect[1] << 4);
			gErr = g - ((short)dVect[2] << 4);
			bErr = b - ((short)dVect[3] << 4);

			pDest[x] = (u8)i;

			pTemp[x+1].a += aErr / 2;
			pTemp[x+1].r += rErr / 2;
			pTemp[x+1].g += gErr / 2;
			pTemp[x+1].b += bErr / 2;

			pTemp[x+XSize].a += aErr / 4;
			pTemp[x+XSize].r += rErr / 4;
			pTemp[x+XSize].g += gErr / 4;
			pTemp[x+XSize].b += bErr / 4;

			if(x)
			{
				pTemp[x+XSize-1].a += aErr / 8;
				pTemp[x+XSize-1].r += rErr / 8;
				pTemp[x+XSize-1].g += gErr / 8;
				pTemp[x+XSize-1].b += bErr / 8;

				if(x > 2)
				{
					pTemp[x+XSize-3].a += aErr / 8;
					pTemp[x+XSize-3].r += rErr / 8;
					pTemp[x+XSize-3].g += gErr / 8;
					pTemp[x+XSize-3].b += bErr / 8;
				}
			}
		}

		a = pTemp[x].a;
		r = pTemp[x].r;
		g = pTemp[x].g;
		b = pTemp[x].b;

		sVect[0] = Clamp((a + 8) >> 4);
		sVect[1] = Clamp((r + 8) >> 4);
		sVect[2] = Clamp((g + 8) >> 4);
		sVect[3] = Clamp((b + 8) >> 4);

		i = Pal.FindVectorSlow(sVect);
		pDest[x] = (u8)i;

		pTemp += XSize;
		pDest += XSize;
	}

	for(x=0; x<XSize; x++)
	{
		a = pTemp[x].a;
		r = pTemp[x].r;
		g = pTemp[x].g;
		b = pTemp[x].b;

		sVect[0] = Clamp((a + 8) >> 4);
		sVect[1] = Clamp((r + 8) >> 4);
		sVect[2] = Clamp((g + 8) >> 4);
		sVect[3] = Clamp((b + 8) >> 4);

		i = Pal.FindVectorSlow(sVect);
		pDest[x] = (u8)i;
	}
	delete [] pTempPix;
}


bool Image32::Crop(long x1, long y1, long x2, long y2)
{
Color *pNewPix, *pSrc, *pDest;
long NewX, NewY, x, y;

	if(x2 < x1 || y2 < y1)
		return false;

	NewX = x2 - x1 + 1;
	NewY = y2 - y1 + 1;

	pNewPix = new Color[NewX * NewY];
	pSrc = pPixels + y1 * XSize;
	pDest = pNewPix;
	for(y=y1; y<y2; y++)
	{
		for(x=x1; x<x2; x++)
			*pDest++ = pSrc[x];

		pSrc += XSize;
	}
	delete [] pPixels;
	pPixels = pNewPix;
	XSize = NewX;
	YSize = NewY;
	return true;
}

bool Image32::SizeCanvas(long NewX, long NewY)
{
Color *pNewPix, *pSrc, *pDest;
long x, y;
long XRun, YRun;

	pNewPix = new Color[NewX * NewY];
	x_memset(pNewPix, 0, NewX * NewY * sizeof(Color));

	XRun = __min(XSize, NewX);
	YRun = __min(YSize, NewY);

	pSrc = pPixels;
	pDest = pNewPix;
	for(y=0; y<YRun; y++)
	{
		for(x=0; x<XRun; x++)
			pDest[x] = pSrc[x];

		pDest += NewX;
		pSrc += XSize;
	}
	delete [] pPixels;
	pPixels = pNewPix;
	XSize = NewX;
	YSize = NewY;
	return true;
}

bool Image32::Quarter(Image32 &Dest)
{
long	x, y, NewX, NewY;
Color	*pSrcPix, *pDestPix;
long	r, g, b, a;

	if((XSize | YSize) & 1)		// Not an even size - Can't quarter it
		return false;

	NewX = XSize / 2;
	NewY = YSize / 2;

	Dest.SetSize(NewX, NewY);
	pSrcPix = GetPixels();
	pDestPix = Dest.GetPixels();

	for(y=0; y<NewY; y++)
	{
		for(x=0; x<NewX; x++)
		{
			r = (long)pSrcPix[0].r + (long)pSrcPix[1].r + (long)pSrcPix[XSize].r + (long)pSrcPix[XSize+1].r;
			g = (long)pSrcPix[0].g + (long)pSrcPix[1].g + (long)pSrcPix[XSize].g + (long)pSrcPix[XSize+1].g;
			b = (long)pSrcPix[0].b + (long)pSrcPix[1].b + (long)pSrcPix[XSize].b + (long)pSrcPix[XSize+1].b;
			a = (long)pSrcPix[0].a + (long)pSrcPix[1].a + (long)pSrcPix[XSize].a + (long)pSrcPix[XSize+1].a;

			pDestPix[x].r = (unsigned char)((r+3) >> 2);
			pDestPix[x].g = (unsigned char)((g+3) >> 2);
			pDestPix[x].b = (unsigned char)((b+3) >> 2);
			pDestPix[x].a = (unsigned char)((a+3) >> 2);

			pSrcPix += 2;
		}
		pSrcPix += XSize;
		pDestPix += NewX;
	}
	return true;
}

bool Image32::HalfX(Image32 &Dest)
{
long	x, y, NewX;
Color	*pSrcPix, *pDestPix;
long	r, g, b, a;

	if(XSize & 1)		// Not an even size - Can't half it
		return false;

	NewX = XSize / 2;

	Dest.SetSize(NewX, YSize);
	pSrcPix = GetPixels();
	pDestPix = Dest.GetPixels();

	for(y=0; y<YSize; y++)
	{
		for(x=0; x<NewX; x++)
		{
			r = (long)pSrcPix[0].r + (long)pSrcPix[1].r;
			g = (long)pSrcPix[0].g + (long)pSrcPix[1].g;
			b = (long)pSrcPix[0].b + (long)pSrcPix[1].b;
			a = (long)pSrcPix[0].a + (long)pSrcPix[1].a;

			pDestPix[x].r = (unsigned char)((r+1) >> 1);
			pDestPix[x].g = (unsigned char)((g+1) >> 1);
			pDestPix[x].b = (unsigned char)((b+1) >> 1);
			pDestPix[x].a = (unsigned char)((a+1) >> 1);

			pSrcPix += 2;
		}
		pDestPix += NewX;
	}
	return true;
}

bool Image32::HalfY(Image32 &Dest)
{
long	x, y, NewY;
Color	*pSrcPix, *pDestPix;
long	r, g, b, a;

	if(YSize & 1)		// Not an even size - Can't half it
		return false;

	NewY = YSize / 2;

	Dest.SetSize(XSize, NewY);
	pSrcPix = GetPixels();
	pDestPix = Dest.GetPixels();

	for(y=0; y<NewY; y++)
	{
		for(x=0; x<XSize; x++)
		{
			r = (long)pSrcPix[0].r + (long)pSrcPix[XSize].r;
			g = (long)pSrcPix[0].g + (long)pSrcPix[XSize].g;
			b = (long)pSrcPix[0].b + (long)pSrcPix[XSize].b;
			a = (long)pSrcPix[0].a + (long)pSrcPix[XSize].a;

			pDestPix[x].r = (unsigned char)((r+1) >> 1);
			pDestPix[x].g = (unsigned char)((g+1) >> 1);
			pDestPix[x].b = (unsigned char)((b+1) >> 1);
			pDestPix[x].a = (unsigned char)((a+1) >> 1);

			pSrcPix++;
		}
		pSrcPix += XSize;
		pDestPix += XSize;
	}
	return true;
}


void Image32::ResizeX(Image32 &Dest, long NewX)
{
	if(NewX < XSize)
		ScaleDownX(Dest, NewX);
	else if(NewX > XSize)
		ScaleUpX(Dest, NewX);
	else
		Dest = *this;
}

void Image32::ScaleUpX(Image32 &Dest, long NewX)
{
float XPos, XStep;
float Scale, r, g, b, a;
long x, y, xs;
Color *pPix, *pDest;

	XStep = (float)XSize / (float)NewX;
	Dest.SetSize( NewX, YSize );

	pPix = pPixels;
	pDest = Dest.pPixels;

	for(y=0; y<YSize; y++)
	{
		XPos = 0;
		for(x=0; x<NewX; x++)
		{
			xs = (long)XPos;
			Scale = XPos - (float)xs;
			if((Scale > 0.001f) && ((xs+1) < XSize))
			{
				a = (float)pPix[xs].a * (1.0f-Scale) + (float)pPix[xs+1].a * Scale;
				r = (float)pPix[xs].r * (1.0f-Scale) + (float)pPix[xs+1].r * Scale;
				g = (float)pPix[xs].g * (1.0f-Scale) + (float)pPix[xs+1].g * Scale;
				b = (float)pPix[xs].b * (1.0f-Scale) + (float)pPix[xs+1].b * Scale;

				pDest[x].a = (u8)FtoL(a);
				pDest[x].r = (u8)FtoL(r);
				pDest[x].g = (u8)FtoL(g);
				pDest[x].b = (u8)FtoL(b);
			}
			else
				pDest[x] = pPix[xs];

			XPos += XStep;
		}
		pDest += NewX;
		pPix += XSize;
	}
}

void Image32::ScaleDownX(Image32 &Dest, long NewX)
{
float r, g, b, a;
float XStart, XEnd, XStep, InvXStep, Scale;
long x, y, xs, xe;
Color *pPix, *pDest;

	Dest.SetSize( NewX, YSize );
	pPix = pPixels;
	pDest = Dest.pPixels;

	XStep = (float)XSize / (float)NewX;
	InvXStep = 1.0f / XStep;

	for(y=0; y<YSize; y++)
	{
		XStart = 0.0f;
		XEnd = XStart + XStep;
		if(XEnd > XSize)
			XEnd = (float)XSize;

		for(x=0; x<NewX; x++)
		{
			xs = (long)XStart;
			Scale = 1.0f - (XStart - (float)xs);

			a = (float)(pPix[xs].a) * Scale;
			r = (float)(pPix[xs].r) * Scale;
			g = (float)(pPix[xs].g) * Scale;
			b = (float)(pPix[xs].b) * Scale;
			xs++;

			xe = (long)XEnd;
			while(xs < xe)
			{
				a += (float)pPix[xs].a;
				r += (float)pPix[xs].r;
				g += (float)pPix[xs].g;
				b += (float)pPix[xs].b;
				xs++;
			}

			Scale = XEnd - (float)xe;
			if(Scale > 0.001f)
			{
				a += (float)(pPix[xs].a) * Scale;
				r += (float)(pPix[xs].r) * Scale;
				g += (float)(pPix[xs].g) * Scale;
				b += (float)(pPix[xs].b) * Scale;
			}

			a *= InvXStep;
			r *= InvXStep;
			g *= InvXStep;
			b *= InvXStep;

			pDest[x].a = (u8)FtoL(a);
			pDest[x].r = (u8)FtoL(r);
			pDest[x].g = (u8)FtoL(g);
			pDest[x].b = (u8)FtoL(b);

			XStart += XStep;
			XEnd += XStep;
		}
		pPix += XSize;
		pDest += NewX;
	}
}

void Image32::ResizeY(Image32 &Dest, long NewY)
{
	if(NewY < YSize)
		ScaleDownY(Dest, NewY);
	else if(NewY > YSize)
		ScaleUpY(Dest, NewY);
	else
		Dest = *this;
}

void Image32::ScaleUpY(Image32 &Dest, long NewY)
{
float YPos, YStep;
float Scale, r, g, b, a;
long x, y, ys;
Color *pPix, *pPix2, *pDest;

	YStep = (float)YSize / (float)NewY;
	Dest.SetSize( XSize, NewY );

	pDest = Dest.pPixels;

	YPos = 0;
	for(y=0; y<NewY; y++)
	{
		ys = (long)YPos;
		pPix = pPixels + XSize * ys;
		pPix2 = pPix + XSize;
		Scale = YPos - (float)ys;

		if(Scale > 0.001f && (ys+1) < YSize)
		{
			for(x=0; x<XSize; x++)
			{
				a = (float)pPix[x].a * (1.0f-Scale) + (float)pPix2[x].a * Scale;
				r = (float)pPix[x].r * (1.0f-Scale) + (float)pPix2[x].r * Scale;
				g = (float)pPix[x].g * (1.0f-Scale) + (float)pPix2[x].g * Scale;
				b = (float)pPix[x].b * (1.0f-Scale) + (float)pPix2[x].b * Scale;

				pDest[x].a = (u8)FtoL(a);
				pDest[x].r = (u8)FtoL(r);
				pDest[x].g = (u8)FtoL(g);
				pDest[x].b = (u8)FtoL(b);
			}
		}
		else
		{
			for(x=0; x<XSize; x++)
				pDest[x] = pPix[x];
		}
		YPos += YStep;
		pDest += XSize;
	}
}

void Image32::ScaleDownY(Image32 &Dest, long NewY)
{
float r, g, b, a;
float YStart, YEnd, YStep, InvYStep, Scale;
long x, y, ys, ye;
Color *pPix, *pPixStart, *pDest;

	Dest.SetSize(XSize, NewY);
	pDest = Dest.pPixels;

	YStep = (float)YSize / (float)NewY;
	InvYStep = 1.0f / YStep;

	YStart = 0.0f;
	YEnd = YStart + YStep;
	for(y=0; y<NewY; y++)
	{
		pPixStart = pPixels + (long)YStart * XSize;
		for(x=0; x<XSize; x++)
		{
			ys = (long)YStart;
			pPix = pPixStart + x;
			Scale = 1.0f - (YStart - (float)ys);

			a = (float)(pPix->a) * Scale;
			r = (float)(pPix->r) * Scale;
			g = (float)(pPix->g) * Scale;
			b = (float)(pPix->b) * Scale;

			ys++;
			pPix += XSize;

			ye = (long)YEnd;
			while(ys < ye)
			{
				a += (float)pPix->a;
				r += (float)pPix->r;
				g += (float)pPix->g;
				b += (float)pPix->b;

				ys++;
				pPix += XSize;
			}

			Scale = YEnd - (float)ye;
			if(Scale > 0.001f)
			{
				a += (float)(pPix->a) * Scale;
				r += (float)(pPix->r) * Scale;
				g += (float)(pPix->g) * Scale;
				b += (float)(pPix->b) * Scale;
			}

			a *= InvYStep;
			r *= InvYStep;
			g *= InvYStep;
			b *= InvYStep;

			pDest[x].a = (u8)FtoL(a);
			pDest[x].r = (u8)FtoL(r);
			pDest[x].g = (u8)FtoL(g);
			pDest[x].b = (u8)FtoL(b);
		}

		pDest += XSize;
		YStart += YStep;
		YEnd += YStep;
		if(YEnd > YSize)
			YEnd = (float)YSize;
	}
}
