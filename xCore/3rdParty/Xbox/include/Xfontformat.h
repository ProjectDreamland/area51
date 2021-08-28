//****************************************************************************
//
// XBox font file format description.
//
//****************************************************************************

#pragma once

#define FILE_VERSION    2 

//============================================================================
// Characters are processed in two separate parts, the character segment
// and the character offset.  The segment consists of the top 12 bits
// and are used as the index into the various tables.  The offset is the
// lower four bits.  We do this to take advanage of the properties of the 
// Kanji unicode character set which has a contigous run of segments but a 
// random distribution of offsets.
//
// This file format is optimized for fonts containing the Kanji unicode
// glyphs.  It is not as efficient for fonts that only contain western, 
// Hiragana and Katakana glyphs because of the extra overhead of the
// segment table which is not needed in those cases.  It should not be
// difficult fix.
//
// This file format does not support multiple styles in the same file
// (i.e. bold, italics, etc).  We can easily add that by having separate
// glyph tables for each included style.  This allows the segment run
// and segment tables to be shared (no big deal).
//
// The file is organized as follows:
//
//  Header
//  GlyphTable - array of offsets to each glyph
//  SegmentRun Table
//  Segment Table
//  Glyphs
//
// The header is 4 byte aligned, the glyph table is 4 byte aligned, 
// the segment run table and the segment table are 2 byte aligned.  The
// Glyphs are 1-byte aligned.  
//============================================================================

//============================================================================
// The following macros figure out these segment/offset for a character.
//============================================================================

#define CHAR_SEGMENT(x) (x >> 4)
#define CHAR_OFFSET(x) (x & 0xF)
#define MAKE_CHAR(x, y) (x << 4 | y)

#define CHAR_SEGMENT_MAX    0xFFF
#define CHAR_OFFSET_MAX     16

//============================================================================
// Initial header at offset 0 of the file, describes some of the global
// information about the font. 
//============================================================================

typedef struct _FontHeader
{
    WORD wSignature;        // Identifies this is actually a font, must be 'XFNT'
    WORD wVersion;          // The FILE_VERSION the font was written with

    // Descriptions of other structures in this file.

    WORD cGlyphs;           // The number of entries in the glyph offset
                            // table.  This actually has one extra entry so
                            // we can use this table to calculate the size
                            // of the glyph before we read it.
                            
    WORD cSegmentRunTable;  // The number of entries in the segment run table.

    // Font metrics, all of the following values are in pixels.

    WORD wDefaultChar;      // The character to use if the requested character
                            // does not have a glyph in the font.

    BYTE uCellHeight;       // The height of the character cell.
    BYTE uDescent;          // Distance from the bottom of the cell to the 
                            // baseline.

    BYTE uAntialiasLevel;   // Amount of antialias information (0, 2, 4)
    BYTE uRLEWidth;         // Width of each RLE 'packet', 2, 4, 8

    BYTE uMaxBitmapHeight;  // Height of the tallest bitmap
    BYTE uMaxBitmapWidth;   // Width of the widest bitmap

} FontHeader;

//============================================================================
// Keeps track of 'runs' of segment values for the character set in this
// font.  Fonts will typically have very few of these.
//============================================================================

typedef struct _SegmentRun
{
    WORD wFirstSegment;     // The segment that starts the run.
    WORD cSegments;         // The number of segments in the run.

    WORD iSegmentTable;     // Index into the segment table where the 
                            // descriptors for this run live.  That table
                            // is allocated immediately after the segment 
                            // run table.
} SegmentRun;

//============================================================================
// Describes the characters that are available in the current segment.
//============================================================================

typedef struct _SegmentDescriptor
{
    WORD iGlyph;        // The index into the glyph data array which is at
                        // offset zero in one of the glyph data areas.  This
                        // refers to the glyph for the first character in this
                        // segment that is defined in the font.

    WORD wCharMask;     // A bit-mask of the character offset for each character
                        // defined in this character segment.  The glyph data
                        // for a character will be at iGlyph + n where n is the
                        // number of characters defined in this segment with
                        // as smaller character offset.
} SegmentDescriptor;

//============================================================================
// Holds the spacing and drawing information for a glyph.
//============================================================================

typedef struct _Glyph
{
    // Describes the bitmap for the glyph.

    BYTE uBitmapHeight;
    BYTE uBitmapWidth;

    // Describes the metrics for the bitmap.  All of these values
    // are relative to the current cursor position.

    BYTE uAdvance;      // # of pixels to advance to get to the next character
    char iBearingX;     // horizontal offset to the left side of the bitmap, may be negative
    char iBearingY;     // vertical offset to the top of the bitmap, may be negative

    // The bitmap immediately follows this structure.
} Glyph;

//============================================================================
// Bitmap format
//
// Bitmaps are stored using a simple RLE algorithm which encodes the lengths
// of alternating off and on pixel runs.  The values of partially-on pixels
// for antialiasing are stored via a simple escape mechanism.
//
// The algorithm always starts at the top-left corner of the glyph and 
// treats the glyph as if it was a simple bitmap with a pitch that is exactly
// the same as the bitmap width.  The encoder will count pixels across row
// boundaries.
//
// A pixel can be in one of three states:
// 
//   off          - the pixel is not part of the glyph
//   on           - the color of the text should fully overwrite the 
//                  background on this pixel
//   partially on - the text color should be blended with the background
//                  color, used for antialiasing
// 
// The encoding algorithm in a human-understandable format.  See the
// PackBitmap method in truetype.cpp for the actual implementation.
//
//   loop
//
//     set the pixel count to 0
//
//     while not at the end of the bitmap and the pixel is off
//       increment the pixel count
//       move to the next pixel
//    
//     encode pixel count into an RLE packet
//
//     quit if at the end of the bitmap
//
//     while not at the end of the bitmap and the pixel is neither on nor off
//       encode pixel value into an RLE packet
//       move to next pixel
//
//     quit if at the end of the bitmap
//
//     set the pixel count to 0
//
//     while not at the end of the bitmap and the pixel is on
//       increment the pixel count
//       move to the next pixel
//
//     encode pixel count into an RLE packet
//
//     quit if at the end of the bitmap
//
//     while not at the end of the bitmap and the pixel is neither on nor off
//       encode pixel value into an RLE packet
//       move to next pixel
//
//     quit if at the end of the bitmap
//
//   end loop
//
// There are two kinds of RLE packets.  The first encodes a count of a run of
// either off or on pixels and consists of a set of integers of uRLEWidth
// bits, where uRLEWidth is set in the font header file and is the same for
// all glyphs in the font.  The second kind of packet encodes the value
// of a partially on pixel or a zero count of an on or off pixel and
// consists of an integer with uRLEWidth bits that is always zero and
// an integer of uAntialiaslevel bits, also set in the font headerfile,
// which contains either zero for a zero count or the value of the partially
// on pixel.
// 
// For the count encoding, if the count is equal to or greater than the
// maximum value that will fit in the integer then it needs to be
// encoded in multiple integers where all of the integers are added
// together to get the final value.
//
//    while count is greater than or equal to zero
//      write the lesser of the count or the maximum value
//      subtract the maximum value from the count
//
// For example, assuming 3 bit RLE width and 4 bit antialias width...
//
// Counts of either on or off pixels:
//
//   a count of 1 pixel    : 001
//   a count of 6 pixels   : 110
//   a count of 7 pixels   : 111 000
//   a count of 8 pixels   : 111 001
//   a count of 21 pixels  : 111 111 111 000
//   a count of zero pixels: 000 0000  <-- uses the paritially on format
// 
// The value of a partially on pixel:
//
//   a value of 1          : 000 0001
//   a value of 7          : 000 0111
//   a value of 15 (max)   : 000 1111
//
// NOTES:
//
//   * the value for a partially-on pixel must fit in the size of the value
//     portion of the partially on RLE packet.
//
//   * if the font does not contain any antialiasing information than
//     a count of zero pixels is encoded just as a count, zero is not used
//     as an escape sequence to the paritially on packet format because there
//     are no partially on packets
//
//   * the values we get for the antialias information is actually from 
//     0 to 2 ^ antialiaslevel _inclusive_ which means we have to encode
//     2^n + 1 discrete values to avoid losing any of the information.  This
//     format works because the count RLE packet can encode 2 of the values 
//     (on or off) and the partially on RLE packet can encode the 2^n - 1
//     other values (the value can hold 2^n minus the one value used to
//     represent a zero count.
//
//============================================================================

