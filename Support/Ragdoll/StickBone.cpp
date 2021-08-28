//==============================================================================
//
//  StickBone.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "StickBone.hpp"
#include "Entropy.hpp"


//==============================================================================
// CLASSES
//==============================================================================

stick_bone::stick_bone()
{
    m_pName = NULL ;
    m_Color = XCOLOR_BLACK ;
    m_L2W.Identity() ;
    m_Start.Zero() ;
    m_End.Zero() ;
}

//==============================================================================

void stick_bone::Init( const char* pName, xcolor Color )
{
    m_pName = pName ;
    m_Color = Color ;
}

//==============================================================================

#ifdef X_DEBUG

void stick_bone::Render( void )
{
    // Lookup matrix values
    vector3& AxisX = *(vector3*)(&m_L2W(0,0)) ;
    vector3& AxisY = *(vector3*)(&m_L2W(1,0)) ;
    vector3& AxisZ = *(vector3*)(&m_L2W(2,0)) ;

    // Draw
    draw_ClearL2W() ;
    
    draw_Begin(DRAW_LINES, DRAW_NO_ZBUFFER) ;
    
    // Draw bone
    draw_Color(m_Color) ;
    draw_Vertex(m_Start) ;
    draw_Vertex(m_End) ;

    // Draw an axis at the center
    vector3 P = (m_Start + m_End) / 2 ;
    draw_Color(XCOLOR_RED) ;
    draw_Vertex(P) ;
    draw_Vertex(P + (AxisX * 2.5f)) ;

    draw_Color(XCOLOR_GREEN) ;
    draw_Vertex(P) ;
    draw_Vertex(P + (AxisY * 2.5f)) ;

    draw_Color(XCOLOR_BLUE) ;
    draw_Vertex(P) ;
    draw_Vertex(P + (AxisZ * 2.5f)) ;

    draw_End() ;
}

#endif

//==============================================================================
