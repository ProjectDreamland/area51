/*

	Usage.cpp

*/

#include "LoadImageFrom.h"
#include "ImageDXTC.h"


bool ConvertImage(char *pFilename)
{
long		lResult;
bool		bResult = true;
Image		*pImg;
Image32		Img32, *pImg32;


	// Load the source image and return it via pImg
	lResult = LoadImageFrom(pFilename, &pImg);
	if(lResult != 0)
	{
		if(lResult == LI_UnsupportedFormat || lResult == LI_UnknownType)
			ErrVal = Err_UnsupportedFormat;
		else
			ErrVal = Err_ErrorLoadingImage;
		return false;
	}

	// Optional - You can just make this a multiple of 4 check
	if( (~(pImg->GetXSize() - 1) & pImg->GetXSize()) != pImg->GetXSize() ||
		(~(pImg->GetYSize() - 1) & pImg->GetYSize()) != pImg->GetYSize() )
	{
		ErrVal = Err_NotPowerOfTwo;
		return false;
	}

	// Convert the input image to a 32 bit image if it isn't already
	if(pImg->GetType() == Type_32Bit)	// Straight 32 bit
		pImg32 = (Image32 *)pImg;
	else
	{
		Img32 = *pImg;
		pImg32 = &Img32;				// Convert to 32 bit
	}


	// Floyd/Steinberg (modified) error diffusion
	pImg32->DiffuseError(8, 5, 6, 5);

	// Build the DXTC texture
	dxtc.FromImage32( pImg32, DC_None );	// Auto-format (DXT1 / DXT3)

	XSize = pImg32->GetXSize();
	YSize = pImg32->GetYSize();

	// Save it to a file
	dwSize = XSize * YSize / 2;
	if(WriteFile(hFile, dxtc.GetBlocks(), dwSize, &dwWritten, 0) == FALSE || dwWritten != dwSize)
	{
		ErrVal = Err_ErrorWritingOutputFile;
		bResult = false;
	}

	return bResult;
}
