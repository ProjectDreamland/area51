//==============================================================================
//  
//  x_bitmap_cmap.cpp
//  
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_BITMAP_HPP
#include "..\x_bitmap.hpp"
#endif

#ifndef X_MEMORY_HPP
#include "..\x_memory.hpp"
#endif

#include <math.h>
#include <stdio.h>

//==============================================================================

#if !( defined( TARGET_PS2 ) && defined( CONFIG_RETAIL ) )

//==============================================================================


/* Print some performance stats. */
/* #define INSTRUMENT_IT    */
/* Track minimum and maximum in inv_cmap_2. */
#define MINMAX_TRACK

static s32 bcenter, gcenter, rcenter;
static s32 gdist, rdist, cdist;
static s32 cbinc, cginc, crinc;
static u32 *gdp, *rdp, *cdp;
static byte *grgbp, *rrgbp, *crgbp;
static s32 gstride, rstride;
static s32 x, xsqr, colormax;
static s32 cindex;

#ifdef INSTRUMENT_IT
static long outercount = 0, innercount = 0;
#endif


#define NBITS       6
#define NENTRIES    (1<<(3*NBITS))
static byte*   s_Index = NULL;
static u32*    s_Dist = NULL;
static s32     s_NColors;
static xcolor* s_Color;
static xbool   s_UseAlpha;

//==============================================================================

void cmap_Begin     ( const xcolor* pColor, s32 NColors, xbool UseAlpha );
s32  cmap_GetIndex  ( xcolor Color );
void cmap_End       ( void );

//==============================================================================

void maxfill( u32* buffer, s32 side );
s32 redloop( void );
s32 blueloop( s32 restart );
s32 greenloop( s32 restart );



/*****************************************************************
 * TAG( inv_cmap_2 )
 *
 * Compute an inverse colormap efficiently.
 * Inputs:
 *     colors:        Number of colors in the forward colormap.
 *     colormap:    The forward colormap.
 *     bits:        Number of quantization bits.  The inverse
 *             colormap will have (2^bits)^3 entries.
 *     dist_buf:    An array of (2^bits)^3 long integers to be
 *             used as scratch space.
 * Outputs:
 *     rgbmap:        The output inverse colormap.  The entry
 *             rgbmap[(r<<(2*bits)) + (g<<bits) + b]
 *             is the colormap entry that is closest to the
 *             (quantized) color (r,g,b).
 * Assumptions:
 *     Quantization is performed by right shift (low order bits are
 *     truncated).  Thus, the distance to a quantized color is
 *     actually measured to the color at the center of the cell
 *     (i.e., to r+.5, g+.5, b+.5, if (r,g,b) is a quantized color).
 * Algorithm:
 *     Uses a "distance buffer" algorithm:
 *     The distance from each representative in the forward color map
 *     to each point in the rgb space is computed.  If it is less
 *     than the distance currently stored in dist_buf, then the
 *     corresponding entry in rgbmap is replaced with the current
 *     representative (and the dist_buf entry is replaced with the
 *     new distance).
 *
 *     The distance computation uses an efficient incremental formulation.
 *
 *     Distances are computed "outward" from each color.  If the
 *     colors are evenly distributed in color space, the expected
 *     number of cells visited for color I is N^3/I.
 *     Thus, the complexity of the algorithm is O(log(K) N^3),
 *     where K = colors, and N = 2^bits.
 */

/*
 * Here's the idea:  scan from the "center" of each cell "out"
 * until we hit the "edge" of the cell -- that is, the point
 * at which some other color is closer -- and stop.  In 1-D,
 * this is simple:
 *     for i := here to max do
 *         if closer then buffer[i] = this color
 *         else break
 *     repeat above loop with i := here-1 to min by -1
 *
 * In 2-D, it's trickier, because along a "scan-line", the
 * region might start "after" the "center" point.  A picture
 * might clarify:
 *         |    ...
 *               | ...    .
 *              ...        .
 *           ... |      .
 *          .    +         .
 *           .          .
 *            .         .
 *             .........
 *
 * The + marks the "center" of the above region.  On the top 2
 * lines, the region "begins" to the right of the "center".
 *
 * Thus, we need a loop like this:
 *     detect := false
 *     for i := here to max do
 *         if closer then
 *             buffer[..., i] := this color
 *             if !detect then
 *                 here = i
 *                 detect = true
 *         else
 *             if detect then
 *                 break
 *
 * Repeat the above loop with i := here-1 to min by -1.  Note that
 * the "detect" value should not be reinitialized.  If it was
 * "true", and center is not inside the cell, then none of the
 * cell lies to the left and this loop should exit
 * immediately.
 *
 * The outer loops are similar, except that the "closer" test
 * is replaced by a call to the "next in" loop; its "detect"
 * value serves as the test.  (No assignment to the buffer is
 * done, either.)
 *
 * Each time an outer loop starts, the "here", "min", and
 * "max" values of the next inner loop should be
 * re-initialized to the center of the cell, 0, and cube size,
 * respectively.  Otherwise, these values will carry over from
 * one "call" to the inner loop to the next.  This tracks the
 * edges of the cell and minimizes the number of
 * "unproductive" comparisons that must be made.
 *
 * Finally, the inner-most loop can have the "if !detect"
 * optimized out of it by splitting it into two loops: one
 * that finds the first color value on the scan line that is
 * in this cell, and a second that fills the cell until
 * another one is closer:
 *      if !detect then        {needed for "down" loop}
 *         for i := here to max do
 *         if closer then
 *             buffer[..., i] := this color
 *             detect := true
 *             break
 *    for i := i+1 to max do
 *        if closer then
 *             buffer[..., i] := this color
 *         else
 *             break
 *
 * In this implementation, each level will require the
 * following variables.  Variables labelled (l) are local to each
 * procedure.  The ? should be replaced with r, g, or b:
 *      cdist:            The distance at the starting point.
 *     ?center:    The value of this component of the color
 *      c?inc:            The initial increment at the ?center position.
 *     ?stride:    The amount to add to the buffer
 *             pointers (dp and rgbp) to get to the
 *             "next row".
 *     min(l):        The "low edge" of the cell, init to 0
 *     max(l):        The "high edge" of the cell, init to
 *             colormax-1
 *     detect(l):        True if this row has changed some
 *                     buffer entries.
 *      i(l):             The index for this row.
 *      ?xx:            The accumulated increment value.
 *
 *      here(l):        The starting index for this color.  The
 *                      following variables are associated with here,
 *                      in the sense that they must be updated if here
 *                      is changed.
 *      ?dist:            The current distance for this level.  The
 *                      value of dist from the previous level (g or r,
 *                      for level b or g) initializes dist on this
 *                      level.  Thus gdist is associated with here(b)).
 *      ?inc:            The initial increment for the row.

 *      ?dp:            Pointer into the distance buffer.  The value
 *                      from the previous level initializes this level.
 *      ?rgbp:            Pointer into the rgb buffer.  The value
 *                      from the previous level initializes this level.
 *
 * The blue and green levels modify 'here-associated' variables (dp,
 * rgbp, dist) on the green and red levels, respectively, when here is
 * changed.
 */

void
compute_cmap2( void )
{
    int nbits = 8 - NBITS;

    colormax = 1 << NBITS;
    x = 1 << nbits;
    xsqr = 1 << (2 * nbits);

    /* Compute "strides" for accessing the arrays. */
    gstride = colormax;
    rstride = colormax * colormax;

#ifdef INSTRUMENT_IT
    outercount = 0;
    innercount = 0;
#endif
    maxfill( s_Dist, colormax );

    for ( cindex = 0; cindex < s_NColors; cindex++ )
    {
    /*
     * Distance formula is
     * (red - map[0])^2 + (green - map[1])^2 + (blue - map[2])^2
     *
     * Because of quantization, we will measure from the center of
     * each quantized "cube", so blue distance is
     *     (blue + x/2 - map[2])^2,
     * where x = 2^(8 - bits).
     * The step size is x, so the blue increment is
     *     2*x*blue - 2*x*map[2] + 2*x^2
     *
     * Now, b in the code below is actually blue/x, so our
     * increment will be 2*(b*x^2 + x^2 - x*map[2]).  For
     * efficiency, we will maintain this quantity in a separate variable
     * that will be updated incrementally by adding 2*x^2 each time.
     */
    /* The initial position is the cell containing the colormap
     * entry.  We get this by quantizing the colormap values.
     */

    /* RG: Special case added when bits = 5. This code seems to achieve a
   * more visually pleasing mapping than Spencer's original code.  Don't
     * ask for a mathematical basis for the following, as these
     * modifications where stumbled upon experimentally.
     */

    if (nbits == 3)
    {
      int r, g, b;

    x = 2;
    xsqr = 4;

      r = rcenter = s_Color[cindex].R;
    g = gcenter = s_Color[cindex].G;
      b = bcenter = s_Color[cindex].B;

    r >>= 2;        /* 6-bits of precision for original entry */
    g >>= 2;
    b >>= 2;
    rcenter >>= 3;  /* 5-bits of precision for quantized entry */
    gcenter >>= 3;
    bcenter >>= 3;

    rdist = r - ((rcenter << 1) + 1);
    gdist = g - ((gcenter << 1) + 1);
    cdist = b - ((bcenter << 1) + 1);
    cdist = rdist * rdist + gdist * gdist + cdist * cdist;

    crinc = 4 - (4 * r) + (8 * rcenter);
    cginc = 4 - (4 * g) + (8 * gcenter);
    cbinc = 4 - (4 * b) + (8 * bcenter);
    }
    else
    {
      rcenter = s_Color[cindex].R >> nbits;
    gcenter = s_Color[cindex].G >> nbits;
      bcenter = s_Color[cindex].B >> nbits;

    rdist = s_Color[cindex].R - (rcenter * x + x/2);
      gdist = s_Color[cindex].G - (gcenter * x + x/2);
    cdist = s_Color[cindex].B - (bcenter * x + x/2);
      cdist = rdist*rdist + gdist*gdist + cdist*cdist;

    crinc = 2 * ((rcenter + 1) * xsqr - (s_Color[cindex].R * x));
      cginc = 2 * ((gcenter + 1) * xsqr - (s_Color[cindex].G * x));
    cbinc = 2 * ((bcenter + 1) * xsqr - (s_Color[cindex].B * x));
  }

    /* Array starting points. */
    cdp = s_Dist + rcenter * rstride + gcenter * gstride + bcenter;
    crgbp = s_Index + rcenter * rstride + gcenter * gstride + bcenter;

    (void)redloop();
    }
#ifdef INSTRUMENT_IT
    fprintf( stderr, "K = %d, N = %d, outer count = %ld, inner count = %ld\n",
         colors, colormax, outercount, innercount );
#endif
}

/* redloop -- loop up and down from red center. */
int
redloop()
{
    int detect;
    int r;//, i = cindex;
    int first;
    long txsqr = xsqr + xsqr;
    //static int here, min, max;
    static long rxx;

    detect = 0;

    /* Basic loop up. */
    for ( r = rcenter, rdist = cdist, rxx = crinc,
      rdp = cdp, rrgbp = crgbp, first = 1;
      r < colormax;
      r++, rdp += rstride, rrgbp += rstride,
      rdist += rxx, rxx += txsqr, first = 0 )
    {
    if ( greenloop( first ) )
        detect = 1;
    else if ( detect )
        break;
    }

    /* Basic loop down. */
    for ( r = rcenter - 1, rxx = crinc - txsqr, rdist = cdist - rxx,
      rdp = cdp - rstride, rrgbp = crgbp - rstride, first = 1;
      r >= 0;
      r--, rdp -= rstride, rrgbp -= rstride,
      rxx -= txsqr, rdist -= rxx, first = 0 )
    {
    if ( greenloop( first ) )
        detect = 1;
    else if ( detect )
        break;
    }

    return detect;
}

/* greenloop -- loop up and down from green center. */
s32 greenloop( s32 restart )
{
    int detect;
    int g;//, i = cindex;
    int first;
    long txsqr = xsqr + xsqr;
    static int here, min, max;
#ifdef MINMAX_TRACK
    static int prevmax, prevmin;
    int thismax, thismin;
#endif
    static long ginc, gxx, gcdist;    /* "gc" variables maintain correct */
    static u32 *gcdp;        /*  values for bcenter position, */
    static byte *gcrgbp;    /*  despite modifications by blueloop */
                    /*  to gdist, gdp, grgbp. */

    if ( restart )
    {
    here = gcenter;
    min = 0;
    max = colormax - 1;
    ginc = cginc;
#ifdef MINMAX_TRACK
    prevmax = 0;
    prevmin = colormax;
#endif
    }

#ifdef MINMAX_TRACK
    thismin = min;
    thismax = max;
#endif
    detect = 0;

    /* Basic loop up. */
    for ( g = here, gcdist = gdist = rdist, gxx = ginc,
      gcdp = gdp = rdp, gcrgbp = grgbp = rrgbp, first = 1;
      g <= max;
      g++, gdp += gstride, gcdp += gstride, grgbp += gstride, gcrgbp += gstride,
      gdist += gxx, gcdist += gxx, gxx += txsqr, first = 0 )
    {
    if ( blueloop( first ) )
    {
        if ( !detect )
        {
        /* Remember here and associated data! */
        if ( g > here )
        {
            here = g;
            rdp = gcdp;
            rrgbp = gcrgbp;
            rdist = gcdist;
            ginc = gxx;
#ifdef MINMAX_TRACK
            thismin = here;
#endif
        }
        detect = 1;
        }
    }
    else if ( detect )
    {
#ifdef MINMAX_TRACK
        thismax = g - 1;
#endif
        break;
    }
    }

    /* Basic loop down. */
    for ( g = here - 1, gxx = ginc - txsqr, gcdist = gdist = rdist - gxx,
      gcdp = gdp = rdp - gstride, gcrgbp = grgbp = rrgbp - gstride,
      first = 1;
      g >= min;
      g--, gdp -= gstride, gcdp -= gstride, grgbp -= gstride, gcrgbp -= gstride,
      gxx -= txsqr, gdist -= gxx, gcdist -= gxx, first = 0 )
    {
    if ( blueloop( first ) )
    {
        if ( !detect )
        {
        /* Remember here! */
        here = g;
        rdp = gcdp;
        rrgbp = gcrgbp;
        rdist = gcdist;
        ginc = gxx;
#ifdef MINMAX_TRACK
        thismax = here;
#endif
        detect = 1;
        }
    }
    else if ( detect )
    {
#ifdef MINMAX_TRACK
        thismin = g + 1;
#endif
        break;
    }
    }

#ifdef MINMAX_TRACK
    /* If we saw something, update the edge trackers.  For now, only
     * tracks edges that are "shrinking" (min increasing, max
     * decreasing.
     */
    if ( detect )
    {
    if ( thismax < prevmax )
        max = thismax;

    prevmax = thismax;

    if ( thismin > prevmin )
        min = thismin;

    prevmin = thismin;
    }
#endif

    return detect;
}

/* blueloop -- loop up and down from blue center. */
s32 blueloop( s32 restart )
{
    int detect;
    register u32 *dp;
    register byte *rgbp;
    register long bdist, bxx;
    register int b, i = cindex;
    register long txsqr = xsqr + xsqr;
    register int lim;
    static int here, min, max;
#ifdef MINMAX_TRACK
    static int prevmin, prevmax;
    int thismin, thismax;
#endif /* MINMAX_TRACK */
    static long binc;

    if ( restart )
    {
    here = bcenter;
    min = 0;
    max = colormax - 1;
    binc = cbinc;
#ifdef MINMAX_TRACK
    prevmin = colormax;
    prevmax = 0;
#endif /* MINMAX_TRACK */
    }

    detect = 0;
#ifdef MINMAX_TRACK
    thismin = min;
    thismax = max;
#endif

    /* Basic loop up. */
    /* First loop just finds first applicable cell. */
    for ( b = here, bdist = gdist, bxx = binc, dp = gdp, rgbp = grgbp, lim = max;
      b <= lim;
      b++, dp++, rgbp++,
      bdist += bxx, bxx += txsqr )
    {
#ifdef INSTRUMENT_IT
    outercount++;
#endif
    if ( *dp > (u32)bdist )
    {
        /* Remember new 'here' and associated data! */
        if ( b > here )
        {
        here = b;
        gdp = dp;
        grgbp = rgbp;
        gdist = bdist;
        binc = bxx;
#ifdef MINMAX_TRACK
        thismin = here;
#endif
        }
        detect = 1;
#ifdef INSTRUMENT_IT
        outercount--;
#endif
        break;
    }
    }
    /* Second loop fills in a run of closer cells. */
    for ( ;
      b <= lim;
      b++, dp++, rgbp++,
      bdist += bxx, bxx += txsqr )
    {
#ifdef INSTRUMENT_IT
    outercount++;
#endif
    if ( *dp > (u32)bdist )
    {
#ifdef INSTRUMENT_IT
        innercount++;
#endif
        *dp = bdist;
        *rgbp = i;
    }
    else
    {
#ifdef MINMAX_TRACK
        thismax = b - 1;
#endif
        break;
    }
    }

    /* Basic loop down. */
    /* Do initializations here, since the 'find' loop might not get
     * executed.
     */
    lim = min;
    b = here - 1;
    bxx = binc - txsqr;
    bdist = gdist - bxx;
    dp = gdp - 1;
    rgbp = grgbp - 1;
    /* The 'find' loop is executed only if we didn't already find
     * something.
     */
    if ( !detect )
    for ( ;
          b >= lim;
          b--, dp--, rgbp--,
          bxx -= txsqr, bdist -= bxx )
    {
#ifdef INSTRUMENT_IT
        outercount++;
#endif
        if ( *dp > (u32)bdist )
        {
        /* Remember here! */
        /* No test for b against here necessary because b <
         * here by definition.
         */
        here = b;
        gdp = dp;
        grgbp = rgbp;
        gdist = bdist;
        binc = bxx;
#ifdef MINMAX_TRACK
        thismax = here;
#endif
        detect = 1;
#ifdef INSTRUMENT_IT
        outercount--;
#endif
        break;
        }
    }
    /* The 'update' loop. */
    for ( ;
      b >= lim;
      b--, dp--, rgbp--,
      bxx -= txsqr, bdist -= bxx )
    {
#ifdef INSTRUMENT_IT
    outercount++;
#endif
    if ( *dp > (u32)bdist )
    {
#ifdef INSTRUMENT_IT
        innercount++;
#endif
        *dp = bdist;
        *rgbp = i;
    }
    else
    {
#ifdef MINMAX_TRACK
        thismin = b + 1;
#endif
        break;
    }
    }


    /* If we saw something, update the edge trackers. */
#ifdef MINMAX_TRACK
    if ( detect )
    {
    /* Only tracks edges that are "shrinking" (min increasing, max
     * decreasing.
     */
    if ( thismax < prevmax )
        max = thismax;

    if ( thismin > prevmin )
        min = thismin;

    /* Remember the min and max values. */
    prevmax = thismax;
    prevmin = thismin;
    }
#endif /* MINMAX_TRACK */

    return detect;
}

void maxfill( u32* buffer, s32 side )
{
    (void)side;
    register unsigned long maxv = ~0L;
    register long i;
    register u32 *bp;

    for ( i = colormax * colormax * colormax, bp = buffer;
      i > 0;
      i--, bp++ )
    *bp = maxv;
}

//==============================================================================

/*****************************************************************
 * TAG( inv_cmap_1 )
 *
 * Compute an inverse colormap efficiently.
 * Inputs:

 *     colors:        Number of colors in the forward colormap.
 *     colormap:    The forward colormap.
 *     bits:        Number of quantization bits.  The inverse
 *             colormap will have (2^bits)^3 entries.
 *     dist_buf:    An array of (2^bits)^3 long integers to be
 *             used as scratch space.
 * Outputs:
 *     rgbmap:        The output inverse colormap.  The entry
 *             rgbmap[(r<<(2*bits)) + (g<<bits) + b]
 *             is the colormap entry that is closest to the
 *             (quantized) color (r,g,b).
 * Assumptions:
 *     Quantization is performed by right shift (low order bits are
 *     truncated).  Thus, the distance to a quantized color is
 *     actually measured to the color at the center of the cell
 *     (i.e., to r+.5, g+.5, b+.5, if (r,g,b) is a quantized color).
 * Algorithm:
 *     Uses a "distance buffer" algorithm:
 *     The distance from each representative in the forward color map
 *     to each point in the rgb space is computed.  If it is less
 *     than the distance currently stored in dist_buf, then the
 *     corresponding entry in rgbmap is replaced with the current
 *     representative (and the dist_buf entry is replaced with the
 *     new distance).
 *
 *     The distance computation uses an efficient incremental formulation.
 *
 *     Right now, distances are computed for all entries in the rgb
 *     space.  Thus, the complexity of the algorithm is O(K N^3),
 *     where K = colors, and N = 2^bits.
 */
void compute_cmap( void )
{
    register u32 *dp;
    register unsigned char *rgbp;
    register long bdist, bxx;
    register int b, i;
    int nbits = 8 - NBITS;
    register int colormax = 1 << NBITS;
    register long xsqr = 1 << (2 * nbits);
    int x = 1 << nbits;
    int rinc, ginc, binc, r, g;
    long rdist, gdist, rxx, gxx;

    for ( i = 0; i < s_NColors; i++ )
    {
    /*
     * Distance formula is
     * (red - map[0])^2 + (green - map[1])^2 + (blue - map[2])^2
     *
     * Because of quantization, we will measure from the center of
     * each quantized "cube", so blue distance is
     *     (blue + x/2 - map[2])^2,
     * where x = 2^(8 - bits).
     * The step size is x, so the blue increment is
     *     2*x*blue - 2*x*map[2] + 2*x^2
     *
     * Now, b in the code below is actually blue/x, so our
     * increment will be 2*x*x*b + (2*x^2 - 2*x*map[2]).  For
     * efficiency, we will maintain this quantity in a separate variable
     * that will be updated incrementally by adding 2*x^2 each time.
     */
    rdist = s_Color[i].R - x/2;
    gdist = s_Color[i].G - x/2;
    bdist = s_Color[i].B - x/2;
    rdist = rdist*rdist + gdist*gdist + bdist*bdist;

    rinc = 2 * (xsqr - (s_Color[i].R << nbits));
    ginc = 2 * (xsqr - (s_Color[i].G << nbits));
    binc = 2 * (xsqr - (s_Color[i].B << nbits));
    dp = s_Dist;
    rgbp = s_Index;
    for ( r = 0, rxx = rinc;
          r < colormax;
          rdist += rxx, r++, rxx += xsqr + xsqr )
        for ( g = 0, gdist = rdist, gxx = ginc;
          g < colormax;
          gdist += gxx, g++, gxx += xsqr + xsqr )
        for ( b = 0, bdist = gdist, bxx = binc;
              b < colormax;
              bdist += bxx, b++, dp++, rgbp++,
              bxx += xsqr + xsqr )
        {
#ifdef INSTRUMENT_IT
            outercount++;
#endif
            if ( i == 0 || *dp > (u32)bdist )
            {
#ifdef INSTRUMENT_IT
            innercount++;
#endif
            *dp = bdist;
            *rgbp = i;
            }
        }
    }
#ifdef INSTRUMENT_IT
    x_DebugMsg( "K = %d, N = %d, outer count = %ld, inner count = %ld\n",
         s_NColors, colormax, outercount, innercount );
#endif
}

//==============================================================================

void cmap_Begin( const xcolor* pColor, s32 NColors, xbool UseAlpha )
{
    ASSERT( s_Index==NULL );

    // Stop at first color from the right that is not pure green
    s32 i = NColors-1;
    while( (i>0) && (pColor[i].R==0) && (pColor[i].G==255) && (pColor[i].B==0) )
        i--;

    s_UseAlpha = UseAlpha;
    s_NColors = i+1;//NColors;
    s_Color   = (xcolor*)pColor;

    if( !s_UseAlpha )
    {
        s_Dist = (u32*)x_malloc(sizeof(u32) * NENTRIES);
        s_Index = (byte*)x_malloc(NENTRIES);
        ASSERT(s_Dist);
        ASSERT(s_Index);

        compute_cmap2();

        x_free(s_Dist);
        s_Dist = NULL;
    }
}

//==============================================================================

s32 cmap_GetIndex( xcolor Color )
{
    if( !s_UseAlpha )
    {
        s32 Entry = ((Color.R>>(8-NBITS))<<(2*NBITS)) |
                    ((Color.G>>(8-NBITS))<<(1*NBITS)) |
                    ((Color.B>>(8-NBITS))<<(0*NBITS));

        ASSERT( (Entry>=0) && (Entry<NENTRIES) );
        return s_Index[ Entry ];
    }

    // Search manually
    s32 BE=0x7FFFFFFF;
    s32 BI=0;
    s32 CV[4];
    s32 PV[4];
    CV[0] = Color.R;
    CV[1] = Color.G;
    CV[2] = Color.B;
    CV[3] = Color.A;

    for( s32 i=0; i<s_NColors; i++ )
    {
        PV[0] = s_Color[i].R;
        PV[1] = s_Color[i].G;
        PV[2] = s_Color[i].B;
        PV[3] = s_Color[i].A;

        s32 E = (PV[0]-CV[0])*(PV[0]-CV[0]) +
                (PV[1]-CV[1])*(PV[1]-CV[1]) +
                (PV[2]-CV[2])*(PV[2]-CV[2]) +
                ((PV[3]-CV[3])*(PV[3]-CV[3])*1);

        if( E < BE )
        {
            BE = E;
            BI = i;
        }
    }

    return BI;
}

//==============================================================================

void cmap_End( void )
{
    if( !s_UseAlpha )
    {
        ASSERT( s_Index );
        x_free(s_Index);
        s_Index=NULL;
    }
}

//==============================================================================

#endif // !( defined( TARGET_PS2 ) && defined( CONFIG_RETAIL ) )
