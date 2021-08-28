
#ifndef _GRID_HELPER_HPP
#define _GRID_HELPER_HPP

#include "x_math.hpp"

struct grid_helper
{
    grid_helper() :
        m_fVerticalGridOffset( 0.f ),
        m_fHorizontalGridOffset( 500.f ),
        m_fSquareHoroz( 100.f ),
        m_fSquareVert( 100.f )
    {
    }

    vector3                     GetRandomAimPos( f32 LookYOffset ) ;

    f32                         m_fVerticalGridOffset ;   // Height offset of grid
    f32                         m_fHorizontalGridOffset ; // Horizontal distance to the aim grid
    f32                         m_fSquareHoroz ;          // Horizontal distance of aim grid
    f32                         m_fSquareVert ;           // Vertical distance of aim grid

};

//=========================================================================

struct frustum_helper
{
    enum flags
    {
        FLAGS_DIRTY_PLANES = (1<<0),
        FLAGS_OCCUPIED     = (1<<1),
        FLAGS_DEBUG_RENDER = (1<<2)
    };

    frustum_helper() :
        m_Width ( 300 ),
        m_Height( 300 ),
        m_Distance( 500.f ),
        m_Flags( FLAGS_DIRTY_PLANES )
    {
    }

    inline void     ComputeVerts( vector3* pVert ) const ;

    f32             m_Width;                // Width of the window use for the frustum
    f32             m_Height;               // Height of the window use for the frustum
    f32             m_Distance;             // Distance for the view
    s32             m_MaxIndices[3*5];      // Indices to check bbox agains the different planes of the view
    plane           m_Plane[5];
    u32             m_Flags;
    
};

//===============================================================================

inline
void frustum_helper::ComputeVerts( vector3* pVert ) const
{
    f32 W = m_Width/2;
    f32 H = m_Height/2;

    pVert[0].Set( -W,  H, m_Distance );
    pVert[1].Set( -W, -H, m_Distance );
    pVert[2].Set(  W, -H, m_Distance );
    pVert[3].Set(  W,  H, m_Distance );
}

#endif