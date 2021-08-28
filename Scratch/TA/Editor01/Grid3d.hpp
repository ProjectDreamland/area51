
#ifndef GRID3D_HPP
#define GRID3D_HPP

#include "x_color.hpp"
#include "x_math.hpp"

//=========================================================================
// GRID3D
//=========================================================================
class grid3d
{
public:

            grid3d          ( void );
    void    Render          ( void );
    void    SetSeparation   ( f32 X, f32 Z );
    void    SetSize         ( f32 X, f32 Z );
    void    SetL2W          ( matrix4& L2W );
    void    SetTranslations ( vector3& Pos );

//=========================================================================
// END PUBLIC INTERFACE
//=========================================================================
protected:

    struct vertex
    {
        vector3    Pos;
    };

protected:

    xcolor  m_Color;
    f32     m_SeparationX; 
    f32     m_SeparationZ; 
    f32     m_SizeX;       
    f32     m_SizeZ;       
    f32     m_OffSetX;     
    f32     m_OffSetZ;
    f32     m_OffSetY;
};

//=========================================================================
// END
//=========================================================================
#endif