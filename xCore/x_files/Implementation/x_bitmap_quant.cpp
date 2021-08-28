//==============================================================================
//  
//  x_bitmap_quant.cpp
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

#ifndef X_TIME_HPP
#include "..\x_time.hpp"
#endif

#ifndef X_STRING_HPP
#include "..\x_string.hpp"
#endif

//==============================================================================

#if !( defined( TARGET_PS2 ) && defined( CONFIG_RETAIL ) )

//==============================================================================

void quant_Begin            ( void );
void quant_SetPixels        ( const xcolor* pColor, s32 NColors );
void quant_End              ( xcolor* pPalette, s32 NColors, xbool UseAlpha );

//==============================================================================

#define HIST_BIT   (6)
#define HIST_MAX   (1 << HIST_BIT)
#define R_STRIDE   (HIST_MAX * HIST_MAX)
#define G_STRIDE   (HIST_MAX)
#define HIST_SHIFT (8 - HIST_BIT)
#define MAX_BOXES   256

struct qbox
{
  s32  variance;         // weighted variance 
  s32  total_weight;     // total weight 
  s32  tt_sum;           // tt_sum += r*r+g*g+b*b*weight over entire box 
  s32  t_ur;             // t_ur += r*weight over entire box 
  s32  t_ug;             // t_ug += g*weight over entire box 
  s32  t_ub;             // t_ub += b*weight over entire box 
  s32  ir, ig, ib;       // upper and lower bounds 
  s32  jr, jg, jb;
};

static qbox     s_QBox[MAX_BOXES];
static s32      s_NQBoxes;
static s32*     s_pQuantHist;
static xcolor*  s_Pixel;
static s32      s_NPixels;

//==============================================================================

static void ComputeSum( s32 ir, s32 ig, s32 ib,
                        s32 jr, s32 jg, s32 jb,
                        s32& total_weight, s32& tt_sum, 
                        s32& t_ur, s32& t_ug, s32& t_ub)
{
  s32 i, j, r, g, b;
  s32 rs, ts;
  s32 w, tr, tg, tb;
  s32 *rp, *gp, *bp;

  j = 0;

  tr = tg = tb = i = 0;

  rp = s_pQuantHist + ((ir * R_STRIDE) + (ig * G_STRIDE) + ib);

  for (r = ir; r <= jr; r++)
  {
    rs = r * r;
    gp = rp;

    for (g = ig; g <= jg; g++)
    {
      ts = rs + (g * g);
      bp = gp;

      for (b = ib; b <= jb; b++)
        if (*bp++)                // was this cell used at all?
        {
          w   = *(bp - 1);        // update statistics
          j  += w;
          tr += r * w;
          tg += g * w;
          tb += b * w;
          i  += (ts + b * b) * w;
        }

      gp += G_STRIDE;
    }

    rp += R_STRIDE;
  }

  total_weight = j;
  tt_sum       = i;
  t_ur         = tr;
  t_ug         = tg;
  t_ub         = tb;
}

//==============================================================================

static void ShrinkBox( s32  ir, s32  ig, s32  ib,
                       s32  jr, s32  jg, s32  jb,
                       s32& lr, s32& lg, s32& lb,
                       s32& hr, s32& hg, s32& hb )
{
  s32 r, g, b;
  s32 *rp, *gp, *bp, *s;
  s32 newlr,newlg,newlb;
  s32 newhr,newhg,newhb;

  newlr=newlg=newlb=-1;
  newhr=newhg=newhb=-1;

  s = s_pQuantHist + (ir * R_STRIDE + ig * G_STRIDE + ib);

  rp = s;

  for (r = ir; r <= jr; r++)
  {
    gp = rp;

    for (g = ig; g <= jg; g++)
    {
      bp = gp;

      for (b = ib; b <= jb; b++)
        if (*bp++) { newlr = r; goto lr_done; }

      gp += G_STRIDE;
    }

    rp += R_STRIDE;
  }

lr_done:

  gp = s;

  for (g = ig; g <= jg; g++)
  {
    rp = gp;

    for (r = ir; r <= jr; r++)
    {
      bp = rp;

      for (b = ib; b <= jb; b++)
        if (*bp++) { newlg = g; goto lg_done; }

      rp += R_STRIDE;
    }

    gp += G_STRIDE;
  }

lg_done:

  bp = s;

  for (b = ib; b <= jb; b++)
  {
    rp = bp;

    for (r = ir; r <= jr; r++)
    {
      gp = rp;

      for (g = ig; g <= jg; g++, gp += G_STRIDE)
        if (*gp) { newlb = b; goto lb_done; }

      rp += R_STRIDE;
    }

    bp++;
  }

lb_done:

  s = s_pQuantHist + (jr * R_STRIDE + jg * G_STRIDE + jb);

  rp = s;

  for (r = jr; r >= ir; r--)
  {
    gp = rp;

    for (g = jg; g >= ig; g--)
    {
      bp = gp;

      for (b = jb; b >= ib; b--)
        if (*bp--) { newhr = r; goto hr_done; }

      gp -= G_STRIDE;
    }

    rp -= R_STRIDE;
  }

hr_done:

  gp = s;

  for (g = jg; g >= ig; g--)
  {
    rp = gp;

    for (r = jr; r >= ir; r--)
    {
      bp = rp;

      for (b = jb; b >= ib; b--)
        if (*bp--) { newhg = g; goto hg_done; }

      rp -= R_STRIDE;
    }

    gp -= G_STRIDE;
  }

hg_done:

  bp = s;

  for (b = jb; b >= ib; b--)
  {
    gp = bp;

    for (g = jg; g >= ig; g--)
    {
      rp = gp;

      for (r = jr; r >= ir; r--, rp -= R_STRIDE)
        if (*rp) { newhb = b; goto hb_done; }

      gp -= G_STRIDE;
    }

    bp--;
  }

hb_done:
  ;

  ASSERT( (newlr>=0) && (newlr<HIST_MAX) );
  ASSERT( (newlg>=0) && (newlg<HIST_MAX) );
  ASSERT( (newlb>=0) && (newlb<HIST_MAX) );
  ASSERT( (newhr>=0) && (newhr<HIST_MAX) );
  ASSERT( (newhg>=0) && (newhg<HIST_MAX) );
  ASSERT( (newhb>=0) && (newhb<HIST_MAX) );

    lr=newlr;
    lg=newlg;
    lb=newlb;
    hr=newhr;
    hg=newhg;
    hb=newhb;
 
}

//==============================================================================

static s32 ComputeVariance( s32 tw,   s32 tt_sum,
                            s32 t_ur, s32 t_ug, s32 t_ub )
{
  f32 temp;

  // The following calculations can be performed in fixed point if needed.
  // Just be sure to preserve enough precision!

  temp  = (f32)t_ur * (f32)t_ur;
  temp += (f32)t_ug * (f32)t_ug;
  temp += (f32)t_ub * (f32)t_ub;
  temp /= (f32)tw;

  return( (s32)((f32)tt_sum - temp) );
}

//==============================================================================

static void SplitBox( s32 ID )
{
  s32   i;
  qbox* new_box;
  qbox* old_box = s_QBox+ID;

  ASSERT( old_box->variance > 0 );

  s32 total_weight;
  s32 tt_sum, t_ur, t_ug, t_ub;
  s32 ir, ig, ib, jr, jg, jb;

  s32 total_weight1;
  s32 tt_sum1, t_ur1, t_ug1, t_ub1;
  s32 ir1, ig1, ib1, jr1, jg1, jb1;

  s32 total_weight2;
  s32 tt_sum2, t_ur2, t_ug2, t_ub2;
  s32 ir2, ig2, ib2, jr2, jg2, jb2;

  s32 total_weight3;
  s32 tt_sum3, t_ur3, t_ug3, t_ub3;

  s32 lowest_variance, variance_r, variance_g, variance_b;
  s32 pick_r, pick_g, pick_b;

  new_box = s_QBox + s_NQBoxes;
  s_NQBoxes++;

  total_weight          = old_box->total_weight;
  tt_sum                = old_box->tt_sum;
  t_ur                  = old_box->t_ur;
  t_ug                  = old_box->t_ug;
  t_ub                  = old_box->t_ub;
  ir                    = old_box->ir;
  ig                    = old_box->ig;
  ib                    = old_box->ib;
  jr                    = old_box->jr;
  jg                    = old_box->jg;
  jb                    = old_box->jb;

  // left box's initial statistics 

  total_weight1         = 0;
  tt_sum1               = 0;
  t_ur1                 = 0;
  t_ug1                 = 0;
  t_ub1                 = 0;

  // right box's initial statistics 

  total_weight2         = total_weight;
  tt_sum2               = tt_sum;
  t_ur2                 = t_ur;
  t_ug2                 = t_ug;
  t_ub2                 = t_ub;

  // Note: One useful optimization has been purposefully omitted from the
  // following loops. The variance function is always called twice per
  // iteration to calculate the new total variance. This is a waste of time
  // in the possibly common case when the new split pos32 did not shift any
  // new points from one box into the other. A simple test can be added to
  // remove this inefficiency.

  // locate optimum split pos32 on red axis 

  variance_r = 0x7FFFFFFF;

  pick_r = ir;
  for (i = ir; i < jr; i++)
  {
    s32 total_variance;

    // calculate the statistics for the area being taken
    // away from the right box and given to the left box

    ComputeSum(i, ig, ib, i, jg, jb,
        total_weight3, tt_sum3, t_ur3, t_ug3, t_ub3);

    ASSERT(total_weight3 <= total_weight);

    // update left and right box's statistics 

    total_weight1 += total_weight3;
    tt_sum1       += tt_sum3;
    t_ur1         += t_ur3;
    t_ug1         += t_ug3;
    t_ub1         += t_ub3;

    total_weight2 -= total_weight3;
    tt_sum2       -= tt_sum3;
    t_ur2         -= t_ur3;
    t_ug2         -= t_ug3;
    t_ub2         -= t_ub3;

    ASSERT((total_weight1 + total_weight2) == total_weight);

    // calculate left and right box's overall variance 

    total_variance = ComputeVariance(total_weight1, tt_sum1, t_ur1, t_ug1, t_ub1) +
                     ComputeVariance(total_weight2, tt_sum2, t_ur2, t_ug2, t_ub2);

    // found better split point? if so, remember it 

    if (total_variance < variance_r)
    {
      variance_r = total_variance;
      pick_r = i;
    }
  }

  // left box's initial statistics 

  total_weight1         = 0;
  tt_sum1               = 0;
  t_ur1                 = 0;
  t_ug1                 = 0;
  t_ub1                 = 0;

  // right box's initial statistics

  total_weight2         = total_weight;
  tt_sum2               = tt_sum;
  t_ur2                 = t_ur;
  t_ug2                 = t_ug;
  t_ub2                 = t_ub;

  // locate optimum split pos32 on green axis 

  variance_g = 0x7FFFFFFF;
  pick_g = ig;
  for (i = ig; i < jg; i++)
  {
    s32 total_variance;

    // calculate the statistics for the area being taken
    // away from the right box and given to the left box

    ComputeSum(ir, i, ib, jr, i, jb,
        total_weight3, tt_sum3, t_ur3, t_ug3, t_ub3);

    ASSERT(total_weight3 <= total_weight);

    // update left and right box's statistics 

    total_weight1 += total_weight3;
    tt_sum1       += tt_sum3;
    t_ur1         += t_ur3;
    t_ug1         += t_ug3;
    t_ub1         += t_ub3;

    total_weight2 -= total_weight3;
    tt_sum2       -= tt_sum3;
    t_ur2         -= t_ur3;
    t_ug2         -= t_ug3;
    t_ub2         -= t_ub3;

    ASSERT((total_weight1 + total_weight2) == total_weight);

    // calculate left and right box's overall variance 

    total_variance = ComputeVariance(total_weight1, tt_sum1, t_ur1, t_ug1, t_ub1) +
                     ComputeVariance(total_weight2, tt_sum2, t_ur2, t_ug2, t_ub2);

    // found better split point? if so, remember it 

    if (total_variance < variance_g)
    {
      variance_g = total_variance;
      pick_g = i;
    }
  }

  // left box's initial statistics 

  total_weight1         = 0;
  tt_sum1               = 0;
  t_ur1                 = 0;
  t_ug1                 = 0;
  t_ub1                 = 0;

  // right box's initial statistics 

  total_weight2         = total_weight;
  tt_sum2               = tt_sum;
  t_ur2                 = t_ur;
  t_ug2                 = t_ug;
  t_ub2                 = t_ub;

  variance_b = 0x7FFFFFFF;
    pick_b = ib;
  // locate optimum split pos32 on blue axis 

  for (i = ib; i < jb; i++)
  {
    s32 total_variance;

    // calculate the statistics for the area being taken
    // away from the right box and given to the left box

    ComputeSum(ir, ig, i, jr, jg, i,
        total_weight3, tt_sum3, t_ur3, t_ug3, t_ub3);

    ASSERT(total_weight3 <= total_weight);

    // update left and right box's statistics 

    total_weight1 += total_weight3;
    tt_sum1       += tt_sum3;
    t_ur1         += t_ur3;
    t_ug1         += t_ug3;
    t_ub1         += t_ub3;

    total_weight2 -= total_weight3;
    tt_sum2       -= tt_sum3;
    t_ur2         -= t_ur3;
    t_ug2         -= t_ug3;
    t_ub2         -= t_ub3;

    ASSERT((total_weight1 + total_weight2) == total_weight);

    // calculate left and right box's overall variance 

    total_variance = ComputeVariance(total_weight1, tt_sum1, t_ur1, t_ug1, t_ub1) +
                     ComputeVariance(total_weight2, tt_sum2, t_ur2, t_ug2, t_ub2);

    // found better split point? if so, remember it 

    if (total_variance < variance_b)
    {
      variance_b = total_variance;
      pick_b = i;
    }
  }

  // now find out which axis should be split 

  lowest_variance = variance_r;
  i = 0;

  if (variance_g < lowest_variance)
  {
    lowest_variance = variance_g;
    i = 1;
  }

  if (variance_b < lowest_variance)
  {
    lowest_variance = variance_b;
    i = 2;
  }

  // split box on the selected axis 

  ir1 = ir; ig1 = ig; ib1 = ib;
  jr2 = jr; jg2 = jg; jb2 = jb;

  switch (i)
  {
    case 0:
    {
      jr1 = pick_r + 0; jg1 = jg; jb1 = jb;
      ir2 = pick_r + 1; ig2 = ig; ib2 = ib;
      break;
    }
    case 1:
    {
      jr1 = jr; jg1 = pick_g + 0; jb1 = jb;
      ir2 = ir; ig2 = pick_g + 1; ib2 = ib;
      break;
    }
    case 2:
    {
      jr1 = jr; jg1 = jg; jb1 = pick_b + 0;
      ir2 = ir; ig2 = ig; ib2 = pick_b + 1;
      break;
    }
  }

  // shrink the new boxes to their minimum possible sizes 

  ShrinkBox(ir1, ig1, ib1, jr1, jg1, jb1,
            ir1, ig1, ib1, jr1, jg1, jb1);

  ShrinkBox(ir2, ig2, ib2, jr2, jg2, jb2,
            ir2, ig2, ib2, jr2, jg2, jb2);

  // update statistics 

  ComputeSum(ir1, ig1, ib1, jr1, jg1, jb1,
      total_weight1, tt_sum1, t_ur1, t_ug1, t_ub1);

  total_weight2         = total_weight - total_weight1;
  tt_sum2               = tt_sum - tt_sum1;
  t_ur2                 = t_ur - t_ur1;
  t_ug2                 = t_ug - t_ug1;
  t_ub2                 = t_ub - t_ub1;

  // create the new boxes 

  old_box->variance     = ComputeVariance(total_weight1, tt_sum1, t_ur1, t_ug1, t_ub1);
  old_box->total_weight = total_weight1;
  old_box->tt_sum       = tt_sum1;
  old_box->t_ur         = t_ur1;
  old_box->t_ug         = t_ug1;
  old_box->t_ub         = t_ub1;
  old_box->ir           = ir1;
  old_box->ig           = ig1;
  old_box->ib           = ib1;
  old_box->jr           = jr1;
  old_box->jg           = jg1;
  old_box->jb           = jb1;

  new_box->variance     = ComputeVariance(total_weight2, tt_sum2, t_ur2, t_ug2, t_ub2);
  new_box->total_weight = total_weight2;
  new_box->tt_sum       = tt_sum2;
  new_box->t_ur         = t_ur2;
  new_box->t_ug         = t_ug2;
  new_box->t_ub         = t_ub2;
  new_box->ir           = ir2;
  new_box->ig           = ig2;
  new_box->ib           = ib2;
  new_box->jr           = jr2;
  new_box->jg           = jg2;
  new_box->jb           = jb2;

}

//==============================================================================

void quant_ClearHistogram( void )
{
    ASSERT( s_pQuantHist );
    x_memset(s_pQuantHist,0,sizeof(s32)*HIST_MAX*HIST_MAX*HIST_MAX);
}

//==============================================================================

void quant_Begin( void )
{
    s_pQuantHist = (s32*)x_malloc(sizeof(s32)*HIST_MAX*HIST_MAX*HIST_MAX);
    ASSERT(s_pQuantHist);
    quant_ClearHistogram();
}

//==============================================================================

void quant_SetPixels( const xcolor* pColor, s32 NColors )
{
    s_Pixel = (xcolor*)pColor;
    s_NPixels = NColors;
}

//==============================================================================

static
s32 quant_BuildPaletteFromHistogram( xcolor* pPalette, s32 NColors )
{
    s32 i;

    //
    // Init box list
    //
    {
        s32 total_weight;
        s32 tt_sum, t_ur, t_ug, t_ub;
        s32 ir, ig, ib, jr, jg, jb;

        // Init box list
        s_NQBoxes = 1;
  
        // Shrink initial box
        ShrinkBox( 0, 0, 0, HIST_MAX-1, HIST_MAX-1, HIST_MAX-1, ir, ig, ib, jr, jg, jb );

        // Compute initial sum
        ComputeSum( ir, ig, ib, jr, jg, jb, total_weight, tt_sum, t_ur, t_ug, t_ub );

        // Init box list
        s_QBox[0].total_weight = total_weight;
        s_QBox[0].variance     = 1;
        s_QBox[0].tt_sum       = tt_sum;
        s_QBox[0].t_ur         = t_ur;
        s_QBox[0].t_ug         = t_ug;
        s_QBox[0].t_ub         = t_ub;
        s_QBox[0].ir           = ir;
        s_QBox[0].ig           = ig;
        s_QBox[0].ib           = ib;
        s_QBox[0].jr           = jr;
        s_QBox[0].jg           = jg;
        s_QBox[0].jb           = jb;
    }

    //
    // Build boxes
    //
    while( s_NQBoxes < NColors )
    {
        // Find worst box
        s32 WorstID    = 0;
        s32 WorstScore = 0;
        for( i=0; i<s_NQBoxes; i++ )
        if( s_QBox[i].variance > WorstScore )
        {
            WorstID = i;
            WorstScore = s_QBox[i].variance;
        }

        if( WorstScore==0 )
            break;

        if( (s_QBox[WorstID].ir == s_QBox[WorstID].jr) &&
            (s_QBox[WorstID].ig == s_QBox[WorstID].jg) &&
            (s_QBox[WorstID].ib == s_QBox[WorstID].jb) )
            break;

        // Split this box
        SplitBox( WorstID );
    }

    // Make colors from boxes
    s32 UsableColors=0;
    for( i=0; i<NColors; i++ )
    {
        xcolor C;
        s32 tw = s_QBox[i].total_weight;
        if( tw==0 )
        {
            C.R = 0;
            C.G = 255;
            C.B = 0;
            C.A = 255;
        }
        else
        {
            C.R = (((s_QBox[i].t_ur << HIST_SHIFT) + (tw>>1)) / tw);
            C.G = (((s_QBox[i].t_ug << HIST_SHIFT) + (tw>>1)) / tw);
            C.B = (((s_QBox[i].t_ub << HIST_SHIFT) + (tw>>1)) / tw);
            C.A = 255;
            UsableColors++;
        }
        pPalette[i] = C;
    }

    return UsableColors;
}

//==============================================================================

struct alpha_info
{
    s32 HistCount;
    s32 QAlpha;
};

struct alpha_box
{
    s32 MinA;
    s32 MaxA;
    s32 Error;
    s32 Alpha;
    s32 NSrcColors;
    s32 NPalColors;
    s32 TotalColors;
};

struct alpha_pal
{
    xcolor C[256];
    s32    NColors;
};

void ComputeCenterAlpha( alpha_info* pAI, alpha_box* pAB )
{
    // Compute weighted alpha
    s32 WA=0;
    s32 C=0;
    s32 i;

    // Compute weighted alpha
    for( i=pAB->MinA; i<=pAB->MaxA; i++ )
    {
        WA += i*pAI[i].HistCount;
        C  += pAI[i].HistCount;
    }

    pAB->Alpha = (C)?(WA/C):(255);
    pAB->TotalColors = C;

    // Compute error
    pAB->Error = 0;
    for( i=pAB->MinA; i<=pAB->MaxA; i++ )
    {
        s32 DE = ABS( i-pAB->Alpha );
        pAB->Error += DE*pAI[i].HistCount;
    }
}

xbool FindAlphaSplit( alpha_info* pAI, 
                     alpha_box* pAB,
                     alpha_box* pLB,
                     alpha_box* pRB )
{
    s32 LA = pAB->MinA;
    s32 RA = pAB->MaxA;
    s32 i;
    s32 BestE=0x7FFFFFFF;
    alpha_box BestLB;
    alpha_box BestRB;
    alpha_box LB;
    alpha_box RB;


    // Find smallest error
    for( i=LA+1; i<=RA; i++ )
    {
        LB.MinA = LA;
        LB.MaxA = i-1;
        RB.MinA = i;
        RB.MaxA = RA;
        ComputeCenterAlpha( pAI, &LB );
        ComputeCenterAlpha( pAI, &RB );

        if( (LB.TotalColors>0) && (RB.TotalColors>0) && ((LB.Error+RB.Error) < BestE) )
        {
            BestE = LB.Error+RB.Error;
            BestLB = LB;
            BestRB = RB;
        }
    }

    if( BestE==0x7FFFFFFF )
        return FALSE;

    *pLB = BestLB;
    *pRB = BestRB;
/*
    // Confirm there are pixels in each box
    s32 nPixelsInLBox=0;
    s32 nPixelsInRBox=0;
    {
        s32 NColors = s_NPixels;
        xcolor* pColor = s_Pixel;
        while( NColors-- )
        {
            if( (pColor->A >= pLB->MinA) &&
                (pColor->A <= pLB->MaxA) )
                nPixelsInLBox++;

            if( (pColor->A >= pRB->MinA) &&
                (pColor->A <= pRB->MaxA) )
                nPixelsInRBox++;

            pColor++;
        }
    }

    ASSERT( (nPixelsInLBox>0) && (nPixelsInRBox>0) );
*/
    return TRUE;
}

//==============================================================================

void quant_End( xcolor* pPalette, s32 aNColors, xbool UseAlpha )
{
    xtimer T;
    T.Start();
    ASSERT( s_Pixel != NULL );

    if( UseAlpha == FALSE )
    {
        quant_ClearHistogram();

        // Enter pixels into histogram
        s32 NColors = s_NPixels;
        xcolor* pColor = s_Pixel;
        while( NColors-- )
        {
            s32 R = pColor->R >> HIST_SHIFT;
            s32 G = pColor->G >> HIST_SHIFT;
            s32 B = pColor->B >> HIST_SHIFT;
            s_pQuantHist[ (R*R_STRIDE) + (G*G_STRIDE) + B ]++;
            pColor++;
        }

        // Build palette
        quant_BuildPaletteFromHistogram( pPalette, aNColors );
    }
    else
    {

        alpha_info* AI; //[256];
        alpha_box*  AB; //[256];
        alpha_pal*  AP; //AP[256];
        
        s32 NAlphaBoxes;
        s32 NColors;
        xcolor* pColor;
        s32 i;

        AI = new alpha_info[256];
        AB = new alpha_box[256];
        AP = new alpha_pal[256];
        ASSERT(AI && AB && AP);
        // Build alpha histogram
        x_memset(AB,0,sizeof(alpha_box)*256);
        x_memset(AI,0,sizeof(alpha_info)*256);
        NColors = s_NPixels;
        pColor = s_Pixel;
        while( NColors-- )
        {
            AI[ pColor->A ].HistCount++;
            pColor++;
        }

        // Write out histogram
/*
        static s32 C=0;
        X_FILE* fp = x_fopen(xfs("hist%03d.txt",C),"wt");
        C++;
        ASSERT(fp);
        for( i=0; i<256; i++ )
            x_fprintf(fp,"%3d] %5d\n",i,AI[i].HistCount);
*/

        // Setup initial alpha boxes
        NAlphaBoxes = 1;
        AB[0].MinA  = 0;
        AB[0].MaxA  = 255;
        ComputeCenterAlpha(AI,&AB[0]);

        // Generate the alpha boxes
        while( NAlphaBoxes < MIN(aNColors/2,32) )
        {
            // Find worst box
            s32 WE=0;
            s32 WI=0;
            for( i=0; i<NAlphaBoxes; i++ )
            if( AB[i].Error > WE )
            {
                WE = AB[i].Error;
                WI = i;
            }

            // Check if we are done...
            if( WE==0 )
                break;

            // Split box.  Will return FALSE if we can't split without
            // generating boxes with no alphas in range
            alpha_box LB;
            alpha_box RB;
            if( !FindAlphaSplit( AI, &AB[WI], &LB, &RB ) )
                break;

            AB[WI] = LB;
            AB[NAlphaBoxes] = RB;
            NAlphaBoxes++;
        }

        // Decide on N colors per box
        {
            // Build sub-palettes for each box
            for( i=0; i<NAlphaBoxes; i++ )
            {
                // Load pixels into histogram
                quant_ClearHistogram();
                s32 NColors = s_NPixels;
                xcolor* pColor = s_Pixel;
                s32 nColorsInBox=0;
                while( NColors-- )
                {
                    if( (pColor->A >= AB[i].MinA) &&
                        (pColor->A <= AB[i].MaxA) )
                    {
                        s32 R = pColor->R >> HIST_SHIFT;
                        s32 G = pColor->G >> HIST_SHIFT;
                        s32 B = pColor->B >> HIST_SHIFT;
                        s_pQuantHist[ (R*R_STRIDE) + (G*G_STRIDE) + B ]++;
                        nColorsInBox = 0;
                    }
                    pColor++;
                }

                AP[i].NColors = quant_BuildPaletteFromHistogram( AP[i].C, aNColors );
            }

            for( i=0; i<NAlphaBoxes; i++ )
            {
                AB[i].NSrcColors = 0;

                for( s32 j=AB[i].MinA; j<=AB[i].MaxA; j++ )
                {
                    AB[i].NSrcColors += AI[j].HistCount;
                }

                AB[i].NPalColors = (aNColors*AB[i].NSrcColors)/(s_NPixels);
                
                if( AP[i].NColors < AB[i].NPalColors )
                    AB[i].NPalColors = AP[i].NColors;

                if( AB[i].NPalColors < 1 )
                    AB[i].NPalColors = 1;
            }

            // Check if we've already used too many?
            {
                s32 TotalC=0;

                for( i=0; i<NAlphaBoxes; i++ )
                    TotalC += AB[i].NPalColors;

                while( TotalC > aNColors )
                {
                    // Find largest
                    s32 BestI = 0;
                    for( i=0; i<NAlphaBoxes; i++ )
                    if( AB[i].NPalColors > AB[BestI].NPalColors )
                        BestI = i;

                    // Remove from largest
                    ASSERT( AB[BestI].NPalColors > 1 );

                    AB[BestI].NPalColors--;
                    TotalC--;
                }
            }

            // Be sure we fit
            while(1)
            {
                // Get total colors and get entry that needs extra colors
                // the most
                s32 TotalC=0;
                s32 BC=0;
                s32 BI=-1;
                for( i=0; i<NAlphaBoxes; i++ )
                {
                    TotalC += AB[i].NPalColors;

                    if( AP[i].NColors > 0 )
                    {
                        s32 Diff = (1000*(AP[i].NColors - AB[i].NPalColors))/AP[i].NColors;
                        if( Diff >= BC )
                        {
                            BC = Diff;
                            BI = i;
                        }
                    }
                }

                if( TotalC == aNColors )
                    break;

                ASSERT( TotalC <= aNColors );

                if( BI==-1 )
                    break;

                AB[BI].NPalColors++;
            }
        }

        // Dump alpha boxes
/*
        x_fprintf(fp,"------------------------------\n");
        for( i=0; i<NAlphaBoxes; i++ )
            x_fprintf(fp,"%3d] %3d <-> %3d  (%3d,%3d)  %3d   %5d  %5d  %5d\n",
                i, AB[i].MinA, AB[i].MaxA, AB[i].NPalColors,AP[i].NColors, AB[i].Alpha, AB[i].Error, AB[i].NSrcColors, AB[i].TotalColors );
*/

        // Build palettes
        {
            s32 PalI=0;
            xcolor Pal[256];
            for( i=0; i<NAlphaBoxes; i++ )
            {
                s32 NUsedColors;

                if( AB[i].NPalColors == AP[i].NColors )
                {
                    x_memcpy(Pal,AP[i].C,sizeof(xcolor)*AP[i].NColors);
                    NUsedColors = AP[i].NColors;
                }
                else
                {
                    // Load pixels into histogram
                    quant_ClearHistogram();
                    s32 NColors = s_NPixels;
                    xcolor* pColor = s_Pixel;
                    s32 nColorsInBox=0;
                    while( NColors-- )
                    {
                        if( (pColor->A >= AB[i].MinA) &&
                            (pColor->A <= AB[i].MaxA) )
                        {
                            s32 R = pColor->R >> HIST_SHIFT;
                            s32 G = pColor->G >> HIST_SHIFT;
                            s32 B = pColor->B >> HIST_SHIFT;
                            s_pQuantHist[ (R*R_STRIDE) + (G*G_STRIDE) + B ]++;
                            nColorsInBox++;
                        }
                        pColor++;
                    }

                    // Compute sub-palette
                    if( nColorsInBox > 0 )
                    {
                        NUsedColors = quant_BuildPaletteFromHistogram(Pal,AB[i].NPalColors);
                    }
                    else
                    {
                        NUsedColors = 0;
                    }
                }

                // Copy into main palette
                for( s32 j=0; j<NUsedColors; j++ )
                {
                    pPalette[PalI+j]    = Pal[j];
                    pPalette[PalI+j].A  = (byte)AB[i].Alpha;
                }
                PalI += NUsedColors;
            }

            // Clear unused colors
            while( PalI < aNColors )
            {
                pPalette[PalI].R = 0;
                pPalette[PalI].G = 255;
                pPalette[PalI].B = 0;
                pPalette[PalI].A = 255;
                PalI++;
            }
            
        }

        delete[] AI;
        delete[] AB;
        delete[] AP;

//        x_fclose(fp);
    }

    x_free(s_pQuantHist);
    s_pQuantHist = NULL;
    T.Stop();
//    x_DebugMsg("quant_End time : %1.2f\n",T.ReadMs());
}

//==============================================================================

#endif // !( defined( TARGET_PS2 ) && defined( CONFIG_RETAIL ) )
