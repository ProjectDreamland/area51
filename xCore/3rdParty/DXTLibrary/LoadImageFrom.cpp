/*

	LoadImageFrom.cpp - Image loading functions

*/

#include "StdAfx.h"
#include "LoadImageFrom.h"

enum {
	LI_Unknown,
	LI_TGA,
	LI_BMP,
};

long LoadImageFrom(const char *pFilename, Image **pDest)
{
char *pExt;
long ImgType = LI_Unknown, Result;
FILE *pFile;

	pExt = strrchr(pFilename, '.');
	if(pExt == 0)
		return LI_UnknownType;

	if(stricmp(pExt, ".tga") == 0)
		ImgType = LI_TGA;
	else if(stricmp(pExt, ".bmp") == 0)
		ImgType = LI_BMP;
	else
		return LI_UnknownType;

	pFile = fopen(pFilename, "rb");
	if(pFile == 0)
		return LI_FileNotFound;

	switch(ImgType)
	{
	case LI_TGA:
		Result = LoadFromTGA(pFile, pDest);
		break;

	case LI_BMP:
		Result = LoadFromBMP(pFile, pDest);
		break;
	}

	fclose(pFile);
	return Result;
}
