//==============================================================================
//
//  x_math_misc.hpp
//
//==============================================================================

#ifndef X_MATH_MISC_HPP
#define X_MATH_MISC_HPP


//==============================================================================
//  LINE/LINESEG/RAY  RELATED MATH FUNCTIONS
//
//  x_IntersectLineSegLineSeg   Calculates intersection pt between 2 line segments
//                              Returns TRUE if they intersect, otherwise FALSE.
//                          TA and TB are the parametric time values along A and B. 
//                          The test only checks for intersection on the 
//                          interval of the line segment, not the infinite line.
//
//  x_ClosestPtsOnLineSegs      Calculates closest point on two line segs to each
//                          other.
//
//==============================================================================


xbool   x_IntersectLineSegLineSeg( const vector3& StartA, const vector3& EndA,    // IN
                                   const vector3& StartB, const vector3& EndB,    // IN
                                         vector3& ISectPt                     );  // OUT

xbool   x_IntersectLineSegLineSeg( const vector3& StartA, const vector3& EndA,    // IN
                                   const vector3& StartB, const vector3& EndB,    // IN
                                         vector3& ISectPt,                        // OUT
                                         f32&     TA,                             // OUT optional
                                         f32&     TB                          );  // OUT optional


void    x_ClosestPtsOnLineSegs   ( const vector3& StartA, const vector3& EndA,    // IN
                                   const vector3& StartB, const vector3& EndB,    // IN
                                         vector3& PtOnA,                          // OUT
                                         vector3& PtOnB                       );  // OUT

void    x_ClosestPtsOnLineSegs   ( const vector3& StartA, const vector3& EndA,    // IN
                                   const vector3& StartB, const vector3& EndB,    // IN
                                         vector3& PtOnA,                          // OUT
                                         vector3& PtOnB,                          // OUT
                                         f32&     TA,                             // OUT
                                         f32&     TB                          );  // OUT


//==============================================================================
#endif  // X_MATH_MISC
//==============================================================================