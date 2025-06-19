//==============================================================================
//  LeastSquares.cpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//==============================================================================

#include "x_plus.hpp"

#include "LeastSquares.hpp"

//==============================================================================
// CONSTANTS
//==============================================================================

static const f64 kEpsilon = 0.001;

//==============================================================================
// IMPLEMENTATION
//==============================================================================

least_squares::least_squares( void ) :
    m_nSamples  ( 0 ),
    m_Degree    ( 0 ),
    m_bSolved   ( FALSE )
{
    x_memset( m_Matrix, 0, sizeof(m_Matrix) );
    x_memset( m_Vector, 0, sizeof(m_Vector) );
    x_memset( m_Coeffs, 0, sizeof(m_Coeffs) );
}

//==============================================================================

least_squares::~least_squares( void )
{
}

//==============================================================================

void least_squares::Setup( s32 PolynomialDegree )
{
    ASSERT( (PolynomialDegree > 0) && (PolynomialDegree <= MAX_POLYNOMIAL_DEGREE) );
    m_nSamples = 0;
    m_Degree   = PolynomialDegree;
    m_bSolved  = FALSE;

    // clear out the matrix and vector in preparation for solving the approximation
    x_memset( m_Matrix, 0, sizeof(m_Matrix) );
    x_memset( m_Vector, 0, sizeof(m_Vector) );
    x_memset( m_Coeffs, 0, sizeof(m_Coeffs) );
}

//==============================================================================

void least_squares::AddSample( f32 X, f32 Y )
{
    ASSERT( m_bSolved == FALSE );
    ASSERT( m_Degree );

    f64 XPowers[(MAX_POLYNOMIAL_DEGREE*2)+1];

    // figure out the powers of x that we'll need
    s32 i;
    XPowers[0] = 1.0;
    XPowers[1] = (f64)X;
    for( i = 2; i <= m_Degree*2; i++ )
    {
        XPowers[i] = X * XPowers[i-1];
    }

    // now update the matrix sums
    s32 Row, Col;
    for( Row = 0; Row <= m_Degree; Row++ )
    {
        for( Col = 0; Col <= m_Degree; Col++ )
        {
            m_Matrix[Row][Col] += XPowers[Row+Col];
        }
    }

    // now update the vector sums
    for( i = 0; i <= m_Degree; i++ )
    {
        m_Vector[i] += XPowers[i] * (f64)Y;
    }

    // and mark that we've added one more sample
    m_nSamples++;
}

//==============================================================================

xbool least_squares::Solve( void )
{
    // we can only solve if we have one more sample than the degree of our
    // polynomial
    if( m_nSamples < (m_Degree+1) )
    {
        ASSERTS( FALSE, "More samples needed to get a decent result!" );
        return FALSE;
    }

    // perform gaussian elimination until we get an upper triangular matrix
    s32 i, j, k;
    for( i = 0; i < m_Degree; i++ )
    {
        // find the biggest element and pivot using that (it turns out that
        // in most cases the biggest element is the best for dealing with
        // accumulated rounding errors)
        s32 Biggest    = i;
        f64 AbsBiggest = x_abs( m_Matrix[Biggest][i] );
        for( j = i + 1; j <= m_Degree; j++ )
        {
            f64 AbsJ = x_abs( m_Matrix[j][i] );
            if( AbsJ > AbsBiggest )
            {
                Biggest    = j;
                AbsBiggest = AbsJ;
            }
        }

        // if the pivot element is zero, then we have a singular matrix,
        // which won't have a solution
        if( AbsBiggest < kEpsilon )
        {
            //ASSERTS( FALSE, "Singular matrix" );
            m_bSolved = TRUE;
            return FALSE;
        }

        // now swap the pivot row with row i
        if( Biggest != i )
        {
            f64 Temp;
            for( j = i; j <= m_Degree; j++ )
            {
                Temp                 = m_Matrix[i][j];
                m_Matrix[i][j]       = m_Matrix[Biggest][j];
                m_Matrix[Biggest][j] = Temp;
            }

            Temp              = m_Vector[i];
            m_Vector[i]       = m_Vector[Biggest];
            m_Vector[Biggest] = Temp;
        }

        // now do the elimination step for this pivot on each of the rows
        for( j = i + 1; j <= m_Degree; j++ )
        {
            f64 Scale = m_Matrix[j][i] / m_Matrix[i][i];
            for( k = i; k <= m_Degree; k++ )
            {
                m_Matrix[j][k] -= Scale * m_Matrix[i][k];
            }

            m_Vector[j] -= Scale * m_Vector[i];

            // Sanity check...make sure we maintain zeros to the left...if
            // we've implemented the gaussian elimination correctly, and if
            // we haven't run into horrendous rounding errors, this should
            // be the case.
            /*#ifdef X_DEBUG
            for( k = 0; k <= i; k++ )
            {
                ASSERT( x_abs(m_Matrix[j][k]) < kEpsilon );
            }
            #endif*/
        }
    }

    // now we should have an upper triangular matrix, we can use back
    // substitution to figure out the coefficients
    for( i = m_Degree; i >= 0; i-- )
    {
        f64 Total = 0.0f;
        for( j = i + 1; j <= m_Degree; j++ )
        {
            Total += m_Matrix[i][j] * m_Coeffs[j];
        }
        m_Coeffs[i] = (m_Vector[i] - Total) / m_Matrix[i][i];
    }

    m_bSolved = TRUE;
    return TRUE;
}

//==============================================================================

f32 least_squares::GetCoeff( s32 Index )
{
    ASSERT( m_bSolved );
    ASSERT( (Index>=0) && (Index<=m_Degree) );
    return (f32)m_Coeffs[Index];
}

//==============================================================================

void least_squares::SetCoeff( s32 Index, f32 Value )
{
    ASSERT( m_bSolved );
    ASSERT( (Index>=0) && (Index<=m_Degree) );

    m_Coeffs[Index] = Value;
}

//==============================================================================

f32 least_squares::Evaluate( f32 X )
{
    s32 i;

    // figure out the powers of x
    f64 XPowers[MAX_POLYNOMIAL_DEGREE+1];
    XPowers[0] = 1.0;
    XPowers[1] = (f64)X;
    for( i = 2; i <= m_Degree; i++ )
        XPowers[i] = (f64)X * XPowers[i-1];

    // evaluate the polynomial
    f64 Total = 0.0;
    for( i = 0; i <= m_Degree; i++ )
    {
        Total += XPowers[i] * m_Coeffs[i];
    }

/*
    // figure out the powers of x
    f32 XPowers[MAX_POLYNOMIAL_DEGREE+1];
    XPowers[0] = 1.0;
    XPowers[1] = (f32)X;
    for( i = 2; i <= m_Degree; i++ )
        XPowers[i] = (f32)X * XPowers[i-1];

    // evaluate the polynomial
    f32 Total = 0.0;
    for( i = 0; i <= m_Degree; i++ )
    {
        Total += XPowers[i] * m_Coeffs[i];
    }
*/

    return (f32)Total;
}

//==============================================================================
