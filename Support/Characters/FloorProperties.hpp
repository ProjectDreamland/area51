//==============================================================================
//  
//  FloorProperties.hpp
//
//==============================================================================

#ifndef FLOOR_PROPERTIES_HPP
#define FLOOR_PROPERTIES_HPP

//==============================================================================

#include "x_math.hpp"
#include "x_color.hpp"

//==============================================================================

class floor_properties
{
public:

             floor_properties    ( void );
            ~floor_properties    ( void );

            void    Init            ( f32 Radius, f32 ColorFadeTime );
            void    Update          ( const vector3& Position, f32 DeltaTime, xbool bIgnorePosition = FALSE );
            void    ForceUpdate     ( const vector3& NewPosition );

            xcolor  GetColor        ( void ) { return m_CurrentColor; }
            u32     GetMaterial     ( void ) { return m_FloorMat; }
private:

            xbool   GrabFloorProperties( const vector3& Position, xcolor& Color, u32& Mat );

private:

    vector3 m_LastPosition;
    f32     m_RadiusSquared;

    vector3 m_StartColor;
    vector3 m_EndColor;
    xcolor  m_CurrentColor;
    f32     m_ColorFadeTime;
    f32     m_ColorFadeT;
    
    u32     m_FloorMat;
};

//==============================================================================
#endif // FLOOR_PROPERTIES_HPP
//==============================================================================
