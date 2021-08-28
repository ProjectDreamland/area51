#include "gridhelper.hpp"

vector3 grid_helper::GetRandomAimPos( f32 LookYOffset )
{
    s32 nHoroz = x_irand( -1, 1 ) ;
    s32 nVert  = x_irand( -1, 1 ) ;
    // Need these vectors to find the next position.
    vector3 vZ( 0.f, 0.f, -m_fHorizontalGridOffset ) ;
    vector3 vYDiff( 0.f, m_fVerticalGridOffset + LookYOffset, 0.f ) ;
    vector3 vX( m_fSquareHoroz, 0.f, 0.f ) ;
    vector3 vCenter = vZ + vYDiff ;

    // X offset
    vector3 vXOffset = (f32)nHoroz * vX ;
    vector3 vPos = vCenter + vXOffset ;

    // Y offset
    f32 fPosYOffset = (f32) nVert * m_fSquareVert ;
    vPos.GetY() += fPosYOffset ;

    return vPos ;
}