ImageLib w/DXTC Compression
---------------------------

Written by Jason Dorie, Black Box Games
Jason.Dorie@BlackBoxGames.com

Notes regarding DXTC / S3TC
---------------------------

  The guts of the DXTC/S3TC compressor is in DXTCGen.cpp - That's the class
that does the color selection given a block of pixels.  The DXTCImage.cpp
class builds a DXTC texture from a source image, using the DXTCGen class to
map individual blocks, and check error amounts.  (The two block encoding
routines return the amount of error introduced in the block, and the image
encoder attempts a 3 color and a four color block to see which is better.

  The DXTCGen encoder has been updated to use MMX instructions to improve
performance.  The original code is preserved in nearby comment blocks.
The code is written to be readable, so it may not be as fast as some other
implementations, and it also uses an almost brute force algorithm to encode
the color blocks.  This means excellent quality, but it sacrifices speed.
You can affect the speed by either removing the brute force search or by
reducing the radius through which this search happens.  The variable
"ScanRange" at the top of DXTCGen.cpp controls this.


  DXTC works really well if you use textures that are double the size of the
originals and compress those, because you get higher detail at the cost of
slight color loss.  It also works well with textures that don't have a lot
of smooth gradients in them, as that's where the artifacts are the most
noticeable.


The other code in the zip is as follows:

  Usage.cpp - An example of how to load, convert, and save a DXTC texture
using all this stuff.

Image.cpp - Image class library, used to store and manipulate 8 bit
palettized images, and 32 bit ARGB images.  It will convert between formats
with an assignment operator, and has routines for cropping, scaling
(bilinear upsample, summed-area downsample), alpha channel classification,
and a whole lot of other stuff.

Median.cpp - Variance-based median-cut vector quantizer.  Used for
palettizing (so is Lloyd)

Lloyd.cpp - Lloyd algorithm for vector quantization.  Lots slower than
median cut, but much better.

CodeBook4MMX.cpp - MMX optimized 4-byte vector class.  Used for manipulation
of 32 bit pixel data during palettizing.

CodeBook.cpp - Generic optimized 4-byte multiple vector class.  Used for
manipulation of 32 bit pixel data during palettizing.  This is the more
generic version of the above code, and will work with vectors that are
any multiple of four bytes.  The MedianCut and LLoyd classes will work
for any multiple-of-4 byte vectors as well, so this can be used for doing
larger vector quantization jobs, like the VQ textures supported by PowerVR
chipsets.

cfVector.cpp, fCodeBook.cpp - A floating point version of the codebook class,
used by Lloyd.cpp

ccDoubleHeap.cpp - Heap class which uses doubles as key values.  Used by
Median.cpp during quantization for color cluster sorting.

Table.cpp - Generic resizable array class.  Note that this class is NOT class
safe - It does not call constructors or destructors if the objects in the
table are classes.  This is a known deficiency for the sake of raw speed.
There are lots of class-safe vector classes out there.  :-)


LoadFrom???, LoadImageFrom.cpp - Image loading functions.  Use
LoadImageFrom() and give it a filename.  It will auto-load 32/24/8 bit
TGA's, and 24/8 bit BMP's.
