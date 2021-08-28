/*

	ImageDXTC.h

*/

#ifndef IMAGEDXTC_H_
#define IMAGEDXTC_H_

#include "Image.h"

class CodeBook;

typedef enum
{
	DC_None,
	DC_DXT1,
	DC_DXT3,
	DC_DXT5,
} DXTCMethod;


class ImageDXTC
{
private:
	long		XSize, YSize;
	u16		    *pBlocks;
	DXTCMethod	Method;
	u8		    AlphaValue;

	void		DXT1to32(Image32 *pDest);
	void		DXT3to32(Image32 *pDest);

	void		EmitTransparentBlock(u16 *pDest);
	void		Emit1ColorBlock(u16 *pDest, Color c);
	void		Emit1ColorBlockTrans(u16 *pDest, Color c, Color *pSrc);
	void		Emit2ColorBlock(u16 *pDest, Color c1, Color c2, Color *pSrc);
	void		Emit2ColorBlockTrans(u16 *pDest, Color c1, Color c2, Color *pSrc);
	void		EmitMultiColorBlock3(u16 *pDest, CodeBook &cb, Color *pSrc);
	void		EmitMultiColorBlock4(u16 *pDest, CodeBook &cb, Color *pSrc);
	void		EmitMultiColorBlockTrans(u16 *pDest, CodeBook &cb, Color *pSrc);
	void		Emit4BitAlphaBlock(u16 *pDest, Color *pSrc);


public:
	ImageDXTC();
~   ImageDXTC();

	void	ReleaseAll(void);

	void	SetMethod(DXTCMethod NewMethod);	// MUST be called before setsize
	void	SetSize(long x, long y);

	long		GetXSize (void) {return XSize;}
	long		GetYSize (void) {return YSize;}
	DXTCMethod	GetMethod(void) {return Method;}
	u16		   *GetBlocks(void) {return pBlocks;}

	void	FromImage32(Image32 *pSrc, DXTCMethod = DC_None);
	void	ToImage32(Image32 *pDest);

	void	CompressDXT1(Image32 *pSrcImg);		// Potentially called by FromImage32
	void	CompressDXT3(Image32 *pSrcImg);		// Potentially called by FromImage32
};

void PlotDXT3Alpha(u16 *pSrc, Color *pDest, long Pitch);
void PlotDXT1     (u16 *pSrc, Color *pDest, long Pitch);

#endif
