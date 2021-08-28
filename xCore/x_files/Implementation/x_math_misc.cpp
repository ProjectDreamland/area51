//==============================================================================
//
//  x_math_misc.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_MATH_HPP
#include "..\x_math.hpp"
#endif

//==============================================================================
//  FUNCTIONS
//==============================================================================

xbool x_IntersectLineSegLineSeg( const vector3& StartA, const vector3& EndA,    // IN
                                 const vector3& StartB, const vector3& EndB,    // IN
                                       vector3& ISectPt,                        // OUT
                                       f32&     TA,                             // OUT optional
                                       f32&     TB )                            // OUT optional
{
    // Test for intersection
    //
    //  Ref: http://www.exaflop.org/docs/cgafaq/cga1.html 
    //         Subject 1.03
    //
    //  A     D
    //    \ /
    //     X
    //   /   \
    // C      B

    f32 denom = (EndA.GetX()-StartA.GetX())*(EndB.GetZ()-StartB.GetZ())-
                (EndA.GetZ()-StartA.GetZ())*(EndB.GetX()-StartB.GetX());
    
    // if denom == 0, lines are parallel
    if (denom != 0)
    {
        // Not parallel
        f32 numA = (StartA.GetZ()-StartB.GetZ())*(EndB.GetX()-StartB.GetX())-
                   (StartA.GetX()-StartB.GetX())*(EndB.GetZ()-StartB.GetZ());
        f32 numB = (StartA.GetZ()-StartB.GetZ())*(EndA.GetX()-StartA.GetX())-
                   (StartA.GetX()-StartB.GetX())*(EndA.GetZ()-StartA.GetZ());

        if (x_abs(numA) < 0.0001f )
        {
            //lines are coincident, so let's just return a point on the line
            ISectPt = StartA;

            TA = 0;
            TB = 0;

            return TRUE;
        }

        f32 rA = numA / denom;
        f32 rB = numB / denom;

        if ((rA>=0) && (rA<=1) && (rB>=0) && (rB<=1))
        {
            // We have an intersection
            vector3 Pt = EndA-StartA;
            Pt.Scale( rA );
            Pt += StartA;

            ISectPt = Pt;

            TA = rA;
            TB = rB;

            return TRUE;
        }
    }

    TA = 0;
    TB = 0;

    return FALSE;
}

//==============================================================================

xbool x_IntersectLineSegLineSeg( const vector3& StartA, const vector3& EndA,    // IN
                                 const vector3& StartB, const vector3& EndB,    // IN
                                       vector3& ISectPt )                        // OUT
{
    f32 T1,T2;  // unused
    return x_IntersectLineSegLineSeg( StartA, EndA, StartB, EndB, ISectPt, T1, T2 );
}

//==============================================================================

void x_ClosestPtsOnLineSegs( const vector3& StartA, const vector3& EndA,    // IN
                             const vector3& StartB, const vector3& EndB,    // IN
                                   vector3& PtOnA,                          // OUT
                                   vector3& PtOnB,                          // OUT
                                   f32&     TA,                             // OUT
                                   f32&     TB                          )   // OUT
{
    vector3  u = EndA - StartA;
    vector3  v = EndB - StartB;
    vector3  w = StartA - StartB;
    f32      a = u.Dot(u);        // always >= 0
    f32      b = u.Dot(v);
    f32      c = v.Dot(v);        // always >= 0
    f32      d = u.Dot(w);
    f32      e = v.Dot(w);
    f32      D = a*c - b*b;       // always >= 0

    //
    // compute the line parameters of the two closest points
    //
    if( D < 0.000001f ) 
    {   
        TA = 0.0f;
        TB = (b>c ? d/b : e/c);   // use the largest denominator
    }
    else 
    {
        TA = (b*e - c*d) / D;
        TB = (a*e - b*d) / D;
    }

    if( TA < 0 ) TA = 0;
    if( TA > 1 ) TA = 1;
    if( TB < 0 ) TB = 0;
    if( TB > 1 ) TB = 1;


    PtOnA = StartA + TA*u;
    PtOnB = StartB + TB*v;
}

//==============================================================================

void x_ClosestPtsOnLineSegs( const vector3& StartA, const vector3& EndA,    // IN
                             const vector3& StartB, const vector3& EndB,    // IN
                                   vector3& PtOnA,                          // OUT
                                   vector3& PtOnB )                         // OUT
{
    f32 TA,TB;

    x_ClosestPtsOnLineSegs( StartA, EndA, StartB, EndB, PtOnA, PtOnB, TA, TB );
}
