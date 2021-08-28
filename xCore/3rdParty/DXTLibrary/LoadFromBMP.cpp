/*

	LoadFromBMP.cpp - BMP Image loader

*/

#include "StdAfx.h"
#include "LoadImageFrom.h"
#include <Assert.h>

typedef struct
{
    DWORD   Size;
    WORD    Reserved1;
    WORD    Reserved2;
    DWORD   OffBits;
} BMHeader;

typedef struct
{
    DWORD	Size;
    long	Width;
    long	Height;
    WORD	Planes;
    WORD	BitCount;
    DWORD	Compression;
    DWORD	SizeImage;
    long	XPelsPerMeter;
    long	YPelsPerMeter;
    DWORD	ClrUsed;
    DWORD	ClrImportant;
} BMInfo;

static long Load8BitBMP(FILE *pFile, BMHeader *pHead, BMInfo *pInfo, Image **pDest);
static long Load24BitBMP(FILE *pFile, BMHeader *pHead, BMInfo *pInfo, Image **pDest);


long LoadFromBMP(FILE *pFile, Image **pDest)
{
WORD ID;
BMHeader	Head;
BMInfo		Info;

	// Parse the header for the format info
	if(fread(&ID, 1, 2, pFile) != 2 || ID != 'MB')
		return LI_FileError;

	if(fread(&Head, 1, sizeof(Head), pFile) != sizeof(Head))
		return LI_FileError;

	if(fread(&Info, 1, sizeof(Info), pFile) != sizeof(Info))
		return LI_FileError;

	if(Info.Compression != 0)
		return LI_UnsupportedFormat;

	if(Info.SizeImage == 0)
		Info.SizeImage = Info.BitCount * Info.Width * Info.Height / 8;

	switch(Info.BitCount)
	{
	case 8:		return Load8BitBMP(pFile, &Head, &Info, pDest);
	case 24:	return Load24BitBMP(pFile, &Head, &Info, pDest);
	default:
		return LI_UnsupportedFormat;
	}
}


static long Load8BitBMP(FILE *pFile, BMHeader *pHead, BMInfo *pInfo, Image **pDest)
{
Image8	*pImg;
BYTE	*pBuf, *pSrc, *pPix;
Color	*pPal;
long	x, y, XSize, YSize, Cols;
DWORD	dwSize;

	pImg = new Image8;

	XSize = pInfo->Width;
	YSize = pInfo->Height;
	Cols = pInfo->ClrUsed;
	if(Cols == 0) Cols = 256;

	pImg->SetNumColors(Cols);
	pPal = pImg->GetPalette();
	pBuf = new BYTE[Cols*4];
	if(fread(pBuf, 1, Cols * 4, pFile) != (unsigned)Cols * 4)
	{
		delete [] pBuf;
		delete pImg;
		return LI_FileError;
	}

	// Convert the palette into ARGB
	pSrc = pBuf;
	for(y=0; y<Cols; y++)
	{
		pPal[y].a = 0xff;
		pPal[y].r = pSrc[2];
		pPal[y].g = pSrc[1];
		pPal[y].b = pSrc[0];
		pSrc += 4;
	}
	delete [] pBuf;

	dwSize = XSize * YSize;
	pBuf = new BYTE[dwSize];
	fseek(pFile, pHead->OffBits, SEEK_SET);
	if(fread(pBuf, 1, dwSize, pFile) != dwSize)
	{
		delete pImg;
		delete [] pBuf;
		return LI_FileError;
	}

	pImg->SetSize(XSize, YSize);
	pPix = pImg->GetPixels() + dwSize - XSize;
	pSrc = pBuf;

	for(y=0; y<YSize; y++)
	{
		for(x=0; x<XSize; x++)
			pPix[x] = pSrc[x];

		pSrc += XSize;
		if( (long)pSrc & 3 )
			pSrc += 4 - ((long)pSrc & 3);

		pPix -= XSize;
	}
	*pDest = pImg;
	delete [] pBuf;
	return LI_OK;
}


static long Load24BitBMP(FILE *pFile, BMHeader *pHead, BMInfo *pInfo, Image **pDest)
{
Image32 *pImg;
BYTE	*pBuf, *pSrc;
Color	*pPix;
long	x, y, XSize, YSize;

	pImg = new Image32;

	pBuf = new BYTE[pInfo->SizeImage];
	fseek(pFile, pHead->OffBits, SEEK_SET);
	if(fread(pBuf, 1, pInfo->SizeImage, pFile) != pInfo->SizeImage)
	{
		delete pImg;
		delete [] pBuf;
		return LI_FileError;
	}

	XSize = pInfo->Width;
	YSize = pInfo->Height;

	pImg->SetSize(XSize, YSize);
	pPix = pImg->GetPixels() + XSize * (YSize-1);
	pSrc = pBuf;

	for(y=0; y<YSize; y++)
	{
		for(x=0; x<XSize; x++)
		{
			pPix[x].a = 0xff;
			pPix[x].r = pSrc[2];
			pPix[x].g = pSrc[1];
			pPix[x].b = pSrc[0];
			pSrc += 3;
		}

		if((long)pSrc & 3)
			pSrc += 4 - ((long)pSrc & 3);

		pPix -= XSize;
	}
	*pDest = pImg;
	delete [] pBuf;
	return LI_OK;
}
