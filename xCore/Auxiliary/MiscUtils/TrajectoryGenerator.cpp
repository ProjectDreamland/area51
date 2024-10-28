#include "TrajectoryGenerator.hpp"

//===========================================================================================

void GAMEUTIL_SolveForAngleOfElevation( f32 Speed, const vector3& StartPos, 
                                                   const vector3& DestPos, f32 GravAccel,
                                                   radian& FirstSolution,
                                                   radian& SecondSolution )
{
    f32    dY = DestPos.GetY() - StartPos.GetY();
    f32    dX = ( vector3( DestPos.GetX(), 0, DestPos.GetZ() ) - vector3( StartPos.GetX() , 0 , StartPos.GetZ() ) ).Length();

    //quadratic equation:  sqrt( b^2 - 4ac ) becomes the following.
    f32 QuadFirst   = dX*dX;
    f32 QuadSecond  = - ( (GravAccel*GravAccel *dX*dX*dX*dX)  / ( Speed*Speed*Speed*Speed ) );
    f32 QuadThird   = (2.f * GravAccel*dX*dX*dY) / (Speed*Speed);

    f32    TestQuadNumber = x_sqrt( QuadFirst + QuadSecond + QuadThird );

//    FirstSolution  = x_atan( Speed*Speed* ( ( -dX + QuadNumber ) / ( GravAccel * dX *dX ) ) );
//    SecondSolution = x_atan( Speed*Speed* ( ( -dX - QuadNumber ) / ( GravAccel * dX *dX ) ) );
    FirstSolution  = x_atan( Speed*Speed* ( ( -dX + TestQuadNumber ) / ( GravAccel * dX *dX ) ) );
    SecondSolution = x_atan( Speed*Speed* ( ( -dX - TestQuadNumber ) / ( GravAccel * dX *dX ) ) );
}


//===========================================================================================