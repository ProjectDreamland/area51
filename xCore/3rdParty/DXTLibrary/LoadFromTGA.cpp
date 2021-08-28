/*

	LoadFromTGA.cpp - TGA Image loader

*/

#include "StdAfx.h"
#include "LoadImageFrom.h"

const BYTE AlphaMask =  0x0f;
const BYTE OriginMask = 0x30;

#pragma pack(1)
typedef struct
{
	BYTE	bIDLength;			// Length of the ID field
	BYTE	bCMapType;			// 0/1
	BYTE	bImgType;			// 2 = uncompressed true color
	WORD	CMapStart;			// Index of the first palette entry
	WORD	CMapLength;			// Length of the color map
	BYTE	CMapBPP;			// Bits per pixel in the color map

	WORD	XOrigin, YOrigin;
	WORD	Width, Height;		// Image size
	BYTE	bPlanes;			// Bits per pixel
	BYTE	bImgDescription;	// Origin / Alpha channel bits
} TGAHeader;
#pragma pack()


static long Load8BitTGA(FILE *pFile, TGAHeader *pHead, Image **pDest);
static long Load24BitTGA(FILE *pFile, TGAHeader *pHead, Image **pDest);
static long Load32BitTGA(FILE *pFile, TGAHeader *pHead, Image **pDest);


long LoadFromTGA(FILE *pFile, Image **pDest)
{
TGAHeader	Head;

	if(fread(&Head, 1, sizeof(Head), pFile) != sizeof(Head))
		return LI_FileError;

	if(Head.bImgType & 0x08)		// Compressed image
		return LI_UnsupportedFormat;

	if(Head.bImgDescription & 0x10)	// Right-side origin
		return LI_UnsupportedFormat;


	switch(Head.bImgType)
	{
	case 0x01:	// Palettized image
		{
		if(Head.bPlanes == 8)
			return Load8BitTGA(pFile, &Head, pDest);
		else
			return LI_UnsupportedFormat;
		}
		break;

	case 0x02:	// True color image
		{
			switch(Head.bPlanes)
			{
			case 24: return Load24BitTGA(pFile, &Head, pDest);
			case 32: return Load32BitTGA(pFile, &Head, pDest);
			default: return LI_UnsupportedFormat;
			}
		}
		break;

	default:
		return LI_UnsupportedFormat;
	}

	return LI_GenericError;
}


static long Load8BitTGA(FILE *pFile, TGAHeader *pHead, Image **pDest)
{
Image8 *pImg;
long XSize, YSize, NCols, YStart, YDelta, y, c;
DWORD DataStart, DataSize;
BYTE *pBuf, *pSrc, *pPix;
Color *pPal;

	if(pHead->CMapBPP != 24 && pHead->CMapBPP != 32)
		return LI_UnsupportedFormat;

	XSize = pHead->Width;
	YSize = pHead->Height;
	NCols = pHead->CMapLength;

	DataStart = sizeof(TGAHeader) + pHead->bIDLength;
	fseek(pFile, DataStart, SEEK_SET);
	DataSize = (pHead->CMapBPP * NCols) / 8;
	pBuf = new BYTE[DataSize];
	if(fread(pBuf, 1, DataSize, pFile) != DataSize)
	{
		delete [] pBuf;
		return LI_FileError;
	}

	pImg = new Image8;
	pImg->SetNumColors(NCols);
	pPal = pImg->GetPalette();
	pSrc = pBuf;

	switch(pHead->CMapBPP)
	{
	case 24:
		for(c=0; c<NCols; c++)
		{
			pPal[c].a = 0xff;
			pPal[c].r = pSrc[2];
			pPal[c].g = pSrc[1];
			pPal[c].b = pSrc[0];
			pSrc += 3;
		}
		break;

	case 32:
		for(c=0; c<NCols; c++)
		{
			pPal[c].a = pSrc[3];
			pPal[c].r = pSrc[2];
			pPal[c].g = pSrc[1];
			pPal[c].b = pSrc[0];
			pSrc += 4;
		}
		break;
	}
	delete [] pBuf;

	pImg->SetSize(XSize, YSize);
	DataSize = XSize * YSize;
	pBuf = new BYTE[DataSize];

	if(fread(pBuf, 1, DataSize, pFile) != DataSize)
	{
		delete [] pBuf;
		delete pImg;
		return LI_FileError;
	}

	if(pHead->bImgDescription & OriginMask)
	{
		YStart = 0;
		YDelta = XSize;
	}
	else
	{
		YStart = pHead->Height-1;
		YDelta = -XSize;
	}

	pSrc = pBuf;
	pPix = pImg->GetPixels() + (XSize * YStart);
	for(y=0; y<YSize; y++)
	{
		memcpy(pPix, pSrc, XSize);
		pSrc += XSize;
		pPix += YDelta;
	}
	*pDest = pImg;
	delete [] pBuf;
	return LI_OK;
}

static long Load24BitTGA(FILE *pFile, TGAHeader *pHead, Image **pDest)
{
Image32 *pImg;
long XSize, YSize, YStart, YDelta, x, y;
DWORD DataStart, DataSize;
BYTE *pBuf, *pSrc;
Color *pPix;

	DataStart = sizeof(TGAHeader) + pHead->bIDLength;
	fseek(pFile, DataStart, SEEK_SET);

	XSize = pHead->Width;
	YSize = pHead->Height;
	DataSize = XSize * YSize * 3;
	pBuf = new BYTE[DataSize];

	if(fread(pBuf, 1, DataSize, pFile) != DataSize)
	{
		delete [] pBuf;
		return LI_FileError;
	}

	if(pHead->bImgDescription & OriginMask)
	{
		YStart = 0;
		YDelta = XSize;
	}
	else
	{
		YStart = pHead->Height-1;
		YDelta = -XSize;
	}

	pImg = new Image32;
	pImg->SetSize(XSize, YSize);

	pSrc = pBuf;
	pPix = pImg->GetPixels() + (XSize * YStart);
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
		pPix += YDelta;
	}
	*pDest = pImg;
	delete [] pBuf;
	return LI_OK;
}

static long Load32BitTGA(FILE *pFile, TGAHeader *pHead, Image **pDest)
{
Image32 *pImg;
bool bTopDown;
long XSize, YSize, YStart, YDelta, x, y;
DWORD DataStart, DataSize;
BYTE *pBuf, *pSrc;
Color *pPix;
long AlphaBits;

	AlphaBits = pHead->bImgDescription & AlphaMask;

	if(AlphaBits != 8 && AlphaBits != 0)
		return LI_UnsupportedFormat;

	DataStart = sizeof(TGAHeader) + pHead->bIDLength;
	fseek(pFile, DataStart, SEEK_SET);

	XSize = pHead->Width;
	YSize = pHead->Height;
	DataSize = XSize * YSize * 4;
	pBuf = new BYTE[DataSize];

	if(fread(pBuf, 1, DataSize, pFile) != DataSize)
	{
		delete [] pBuf;
		return LI_FileError;
	}

	if(pHead->bImgDescription & OriginMask)
	{
		YStart = 0;
		YDelta = XSize;
		bTopDown = true;
	}
	else
	{
		YStart = pHead->Height-1;
		YDelta = -XSize;
		bTopDown = false;
	}

	pImg = new Image32;
	pImg->SetSize(XSize, YSize);

	pSrc = pBuf;
	pPix = pImg->GetPixels() + (XSize * YStart);
	for(y=0; y<YSize; y++)
	{
		for(x=0; x<XSize; x++)
		{
			pPix[x].a = pSrc[3];
			pPix[x].r = pSrc[2];
			pPix[x].g = pSrc[1];
			pPix[x].b = pSrc[0];
			pSrc += 4;
		}

		if(AlphaBits == 0)
		{
			for(x=0; x<XSize; x++)
				pPix[x].a = 0xff;
		}

		pPix += YDelta;
	}
	*pDest = pImg;
	delete [] pBuf;
	return LI_OK;
}
