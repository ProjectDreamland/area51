/*

	LoadImageFrom.h - Image loading functions

*/

#ifndef LOADIMAGEFROM_H_
#define LOADIMAGEFROM_H_

#include "Image.h"
#include <StdIO.h>

enum {
	LI_OK = 0,
	LI_FileNotFound,
	LI_UnknownType,
	LI_UnsupportedFormat,
	LI_FileError,
	LI_GenericError,
};

// Allocates a new Image object and stores the pointer in *pDest
// Returns one of the above enums to indicate success/failure

long LoadImageFrom(const char *pFilename, Image **pDest);


// Format-specific image loaders
long LoadFromTGA(FILE *pFile, Image **pDest);
long LoadFromBMP(FILE *pFile, Image **pDest);

#endif
