/*
 *    LAME tools library include file
 *    Simple context free functions
 *      - no references to gfc, to gfp, or to any other function not defined
 *        in this module
 * 
 *    Copyright (c) 2000 Frank Klemm
 *    Copyright (c) 2001 John Dahlstrom
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* $Id: tools.h,v 1.2 2001/05/04 01:07:37 jd- Exp $ */

#ifndef LAME_TOOLS_H
#define LAME_TOOLS_H

#include "machine.h"



/***********************************************************************
*  Function Prototype Declarations
***********************************************************************/

static inline void qinterp_cf_42( const FLOAT y[4], FLOAT c[3] );
static inline void qinterp_cf_3( const FLOAT y[3], FLOAT c[3] );
static inline FLOAT qinterp_eval( const FLOAT c[3], FLOAT x, 
                                  FLOAT xtrans, FLOAT xratio );



/***********************************************************************
*  Macros and Static Inline Function Definitions
***********************************************************************/

/* qinterp_cf_42 - Given 4 points, find the coefficients for a quadratic
                   that connects the 2 center points.  -jd
  in: y    coordinate values, paired with constant x coordinates, -1, 0, 1, 2
 out: c    coefficients ordered for quadratic, (c[2] * x*x + c[1] * x + c[0])
design note:
  Utilize the inverse of two constant 3x3 matrices to compute two quadratics,
  one from the points at (-1,0,1), and the other from the points at (0,1,2).
  The mean of the two yields a quadratic between the points at 0 and 1.
*/
static inline void
qinterp_cf_42( const FLOAT y[4], FLOAT c[3] )
{
  c[2] = ( y[0] - y[1] - y[2] + y[3]) * 0.25; /* ([1 -1 -1 1] .* Y) * 0.25 */
  c[1] = y[2] - y[1] - c[2];    /* ([-1 -3 5 -1] .* Y) * 0.25 */
  c[0] = y[1];
}

/* qinterp_cf_3 - Given 3 points, find the coefficients for a quadratic
                  that connects the 3 points.  -jd
  in: y    coordinate values, paired with constant x coordinates, 0, 1, 2
 out: c    coefficients ordered for quadratic, (c[2] * x*x + c[1] * x + c[0])
*/
static inline void
qinterp_cf_3( const FLOAT y[3], FLOAT c[3] )
{
  c[2] = ( y[0] + y[2]) * 0.5 - y[1]; /* ([1 -2 1] .* Y) * 0.5 */
  c[1] = y[1] - y[0] - c[2];    /* ([-3 4 -1] .* Y) * 0.5 */
  c[0] = y[0];
}



/* qinterp_eval - Evaluate a quadratic at a point, given polynomial 
                  coefficients, and an x coordinate with translation and scale
                  ratio values.  This function evaluates the quadratic at the
                  transformed x coordinate ((x - xtrans) * xratio)).  -jd
 in: c       quadratic coefficients, for (c[2] * x * x + c[1] * x + c[0])
     x
     xtrans
     xratio
returns: y coordinate (the quadratic evaluated)
*/
static inline FLOAT
qinterp_eval( const FLOAT c[3], FLOAT x, FLOAT xtrans, FLOAT xratio )
{
  x = (x - xtrans) * xratio;
  return( (c[2] * x + c[1]) * x + c[0] );
}



#endif /* LAME_TOOLS_H */
