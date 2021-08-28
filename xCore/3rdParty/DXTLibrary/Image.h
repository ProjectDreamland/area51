/*

    Image.h - Generic 24 bit image class

*/

#ifndef IMAGE_H_
#define IMAGE_H_

#ifndef  X_TYPES_HPP
#include<x_types.hpp>
#endif

inline long FtoL(float f)
{
long R;

    __asm {
        fld [f]
        fistp [R]
    }
    return R;
}

typedef enum
{
    Type_8Bit,
    Type_32Bit,
} ImageType;

typedef enum
{
    AT_None,            // No alpha used - All 0xff
    AT_Binary,          // 0x00 / 0xff
    AT_Constant,        // Single, constant non-0xff value used 
    AT_ConstantBinary,  // 0x00 / 0x??
    AT_DualConstant,    // 0x?? / 0x?? - Two constants
    AT_Modulated,       // Multiple values used
} AlphaType;

typedef enum
{
    QM_Lloyd = 1,
    QM_MedianCut = 2,
} QuantMethodType;


class Color
{
public:
    Color() {;}
    Color(u32 c) : Col(c) {;}
    Color(u8 R, u8 G, u8 B, u8 A) : a(A), r(R), g(G), b(B) {;}

    union {
        struct { u8 a, r, g, b; };
        u32 Col;
    };
};

// ----------------------------------------------------------------------------
// Base image class
// ----------------------------------------------------------------------------
class Image
{
private:
    long    XSize, YSize;

public:
    Image( void ){ }
    Image( long w,long h )
    {
        XSize = w;
        YSize = h;
    }
    virtual ~Image() {;}
    virtual ImageType GetType(void) = 0;

    virtual void SetSize(long x, long y) = 0;
    virtual bool Crop(long x1, long y1, long x2, long y2) = 0;
    virtual bool SizeCanvas(long NewX, long NewY) = 0; // Resize the "canvas" without sizing the image

    long    GetXSize(void) {return XSize;}
    long    GetYSize(void) {return YSize;}

    double  Diff(Image *pComp);
    double  MSE(Image *pComp);

    AlphaType   AlphaUsage(unsigned char *pAlpha1 = 0, unsigned char *pAlpha0 = 0); // Returns how the alpha channel is used
    void        AlphaToBinary(unsigned char Threshold = 128);                       // Force alpha to be 0x00 / 0xff

    static  QuantMethodType QuantMethod;
    static  bool QuantDiffusion;

    friend class Image8;
    friend class Image32;
};


// ----------------------------------------------------------------------------
// 8 Bit palettized image class
// ----------------------------------------------------------------------------
class Image8 : public Image
{
private:
    Color   *pPalette;
    long    NumCols;

    u8  *pPixels;

public:
    Image8();
    ~Image8();
    ImageType GetType(void) {return Type_8Bit;}

    void ReleaseAll(void);

    void    SetSize(long x, long y);
    u8  *GetPixels(void) {return pPixels;}

    void    SetNumColors(long Cols);
    Color   *GetPalette(void) {return pPalette;}
    long    GetNumColors(void) {return NumCols;}
    bool    Crop(long x1, long y1, long x2, long y2);
    bool    SizeCanvas(long NewX, long NewY);

    void    QuantizeFrom(Image32 *pSrcImage, Image32 *pPaletteImage = NULL, Color *pForceColor = NULL);
    void    QuantizeFrom(Image32 *pSrcImage, long NumCols);
    Image8  &operator=(Image &Src);
    Image8  &operator=(Image8 &Src) {return this->operator=(*(Image *)&Src);}
    Image8  &operator=(Image32 &Src) {return this->operator=(*(Image *)&Src);}

    friend class Image32;
};


// ----------------------------------------------------------------------------
// 32 Bit true color image class
// ----------------------------------------------------------------------------
class Image32 : public Image
{
private:
    Color   *pPixels;

public:
    Image32( long w,long h,Color* ptr ): Image(w,h)
    {
        pPixels = ptr;
    }
    Image32();
  ~ Image32();

    ImageType GetType(void) {return Type_32Bit;}

    Color& operator()( long x,long y )
    {
        return pPixels[x+y*XSize];
    }

    void ReleaseAll(void);

    void    SetSize(long x, long y);
    Color * GetPixels(void) {return pPixels;}

    Image32 &operator=(Image &Src);
    Image32 &operator=(Image8 &Src) {return this->operator=(*(Image *)&Src);}
    Image32 &operator=(Image32 &Src) {return this->operator=(*(Image *)&Src);}

    long    UniqueColors(void);             // Unique color count for the image

    float   AverageSlope(void);             // Compute the average slope between pixel neighbors

    void    DiffuseError(long aBits, long rBits, long gBits, long bBits);   // # of bits per gun
    void    DiffuseQuant(Image8 &DestImg);  // DestImg already contains the palette

    bool    Crop(long x1, long y1, long x2, long y2);
    bool    SizeCanvas(long NewX, long NewY);
    bool    Quarter(Image32 &Dest);         // Generate a quarter size image in Dest
    bool    HalfX(Image32 &Dest);
    bool    HalfY(Image32 &Dest);

    void    ResizeX(Image32 &Dest, long NewX);
    void    ScaleUpX(Image32 &Dest, long NewX);
    void    ScaleDownX(Image32 &Dest, long NewX);

    void    ResizeY(Image32 &Dest, long NewY);
    void    ScaleUpY(Image32 &Dest, long NewY);
    void    ScaleDownY(Image32 &Dest, long NewY);

    friend class Image8;
};

#endif
