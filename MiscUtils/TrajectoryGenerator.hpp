#include "x_math.hpp"


//==================================================================================================
//
// SolveForAngleOfElevation  -   Pass in a speed, a start position, a destination, and gravity.
//                               The two possible solutions are stored in First and Second solution.
//
//==================================================================================================

void  GAMEUTIL_SolveForAngleOfElevation( f32 Speed, const vector3& StartPos, const vector3& DestPos, 
                                         f32 GravAccel, radian& FirstSolution, radian& SecondSolution ) ;

//=======================================================================================================
//
// SolveForInitialVelocity  -   Pass in a start position, end position, gravity and the angle of elevation
//                              and this will give you back the initial speed you need to get from the start
//                              position to the destination.  Bear in mind that this expects a value from
//                              0 - PI/2 for the angle of elevation, so put your pitch into two dimensions.
//
//=======================================================================================================

f32 GAMEUTIL_SolveForInitialVelocity( );