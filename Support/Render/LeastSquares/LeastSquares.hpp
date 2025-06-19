//==============================================================================
//  LeastSquares.hpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//
//  This class is used to create a least-squares polynomial approximation for
//  a given set of data. Check out least-squares polynomials from math.wolfram.com
//  or from efunda.com.
//
//  In a general form, an M-th degree polynomial looks like:
//  y= a[0] +a[1]*x + a[2]*x^2 + a[3]*x^3 + ... + a[m]*x^m
//
//  Given a set of data to approximate:
//  { (x[1],y[1]), (x[2],y[2]), ..., (x[n],y[n]) }
//  where n >= m+1, the best fitting curve f(x) has the least square error.
//
//  min = SUM( i=1,n, {y[i] - (a[0] + a[1]*x[i] + a[2]*x[i]^2 + ... + a[m]*x[i]^m)}^2 )
//      = SUM( i=1,n, {y[i] - f(x[i])}^2 )
//
//  We'll call this sum PI.
//
//  a[0],a[1], ..., a[m] are unknown coefficients while x[i] and y[i] come from
//  the sample data. To obtain the least square error, the unknown coefficients
//  a[0], a[1], ..., a[m] must yield zero first derivatives.
//
//  (We'll use the letter 'd' to denote partial derivatives)
//  (dPI/da[0]) = 2 * SUM(i=1,n, 1      * {y[i]-(a[0]+a[1]x[i]+a[2]x[i]^2+...+a[m]x[i]^m]} = 0
//  (dPI/da[1]) = 2 * SUM(i=1,n, x[i]^1 * {y[i]-(a[0]+a[1]x[i]+a[2]x[i]^2+...+a[m]x[i]^m]} = 0
//  (dPI/da[2]) = 2 * SUM(i=1,n, x[i]^2 * {y[i]-(a[0]+a[1]x[i]+a[2]x[i]^2+...+a[m]x[i]^m]} = 0
//  ...
//  (dPI/da[m]) = 2 * SUM(i=1,n, x[i]^m * {y[i]-(a[0]+a[1]x[i]+a[2]x[i]^2+...+a[m]x[i]^m]} = 0
//
//  Now if we expand and re-arrange the above equations we'll get this (clearly
//  doing this without being able to use math symbols is not very friendly on
//  the eyes, but if you jot it down on paper with the summation symbol it
//  becomes *slightly* easier to read!)
//  SUM(i=1,n,1*       y[i]) = a[0]*SUM(i=1,n,1)      + a[1]*SUM(i=1,n,x[i])       + a[2]*SUM(i=1,n,x[i]^2)     + ... + a[m]*SUM(i=1,n,x[i]^m)
//  SUM(i=1,n,(x[i]^1)*y[i]) = a[0]*SUM(i=1,n,x[i])   + a[1]*SUM(i=1,n,x[i]^2)     + a[2]*SUM(i=1,n,x[i]^3)     + ... + a[m]*SUM(i=1,n,x[i]^(m+1))
//  SUM(i=1,n,(x[i]^2)*y[i]) = a[0]*SUM(i=1,n,x[i]^2) + a[1]*SUM(i=1,n,x[i]^3)     + a[2]*SUM(i=1,n,x[i]^4)     + ... + a[m]*SUM(i=1,n,x[i]^(m+2))
//  ...
//  SUM(i=1,n,(x[i]^m)*y[i]) = a[0]*SUM(i=1,n,x[i]^m) + a[1]*SUM(i=1,n,x[i]^(m+1)) + a[2]*SUM(i=1,n,x[1]^(m+2)) + ... + a[m]*SUM(i=1,n,x[i]^(m+m))
//
//  So what we have here is a system of linear equations. We solve that system
//  of linear equations using gaussian elimination or something similar and
//  then we've got a proper solution.
//
//
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
// WARNING WARNING WARNING WARNING WARNING
// THIS CODE USES 64-BIT PRECISION FLOATS. DO NOT USE THIS CODE DURING
// GAMEPLAY. SETUP OR LOAD TIME ONLY!!!!!!
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

#ifndef __LEASTSQUARES_HPP__
#define __LEASTSQUARES_HPP__

#include "x_types.hpp"

//==============================================================================

class least_squares
{
public:
    least_squares( void );
    ~least_squares( void );

    enum    { MAX_POLYNOMIAL_DEGREE = 3 };

    void    Setup       ( s32 PolynomialDegree = MAX_POLYNOMIAL_DEGREE );
    void    AddSample   ( f32 X, f32 Y );
    xbool   Solve       ( void );
    f32     GetCoeff    ( s32 Index );
    void    SetCoeff    ( s32 Index, f32 Value );
    f32     Evaluate    ( f32 X );

protected:
    s32             m_nSamples;
    s32             m_Degree;
    xbool           m_bSolved;
    f64             m_Matrix[MAX_POLYNOMIAL_DEGREE+1][MAX_POLYNOMIAL_DEGREE+1];
    f64             m_Vector[MAX_POLYNOMIAL_DEGREE+1];
    f64             m_Coeffs[MAX_POLYNOMIAL_DEGREE+1];
};

#endif // __LEASTSQUARES_HPP__