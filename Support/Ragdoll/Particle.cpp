//==============================================================================
//
//  Particle.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "Particle.hpp"
#include "Entropy.hpp"


//==============================================================================
// DATA
//==============================================================================

// For fun testing!
const char* pFixedList[] =
{
    //"Particle Head",
    //"Particle L Wrist",
    //"Particle R Wrist",

    NULL
} ;

//==============================================================================
// CLASSES
//==============================================================================

particle::particle()
{
    m_pName = "NULL" ;
    m_Pos.Zero() ;
    m_LastPos.Zero() ;
    m_LastCollPos.Zero() ;
    m_Mass    = 1.0f ;
    m_InvMass = 1.0f ;
    m_GeomBoneIndex = -1 ;
    m_pParent = NULL ;
}

//==============================================================================

void particle::Init( const char* pName, const vector3& Pos, f32 Mass, xbool bCollision, xcolor Color )
{
    s32 i ;

    m_pName   = pName ;
    m_Pos     = m_BindPos = m_LastPos = m_LastCollPos = Pos ;
    m_Mass    = Mass ;
    if (Mass != 0)
        m_InvMass = 1.0f / Mass ;
    m_Color   = Color ;
    m_bCollision = bCollision ;

    // Set inverse mass to 0 if this particle is fixed
    i = 0 ;
    while(pFixedList[i])
    {
        if (x_stricmp(pName, pFixedList[i]) == 0)
            m_InvMass = 0 ;

        i++ ;
    }
}

//==============================================================================

void particle::SetBind( void )
{
    m_Pos = m_LastPos = m_LastCollPos = m_BindPos ;
}

//==============================================================================

// Sets up parent particle
void particle::SetParent( particle* pParent )
{
    m_pParent = pParent ;
}

//==============================================================================

#ifdef X_DEBUG

void particle::Render( f32 Radius )
{
    // Skip if no collision
    if (!m_bCollision)
        return ;

    // Draw current position
    draw_ClearL2W() ;
    draw_Sphere(m_Pos, Radius, m_Color) ;
    draw_Point(m_Pos, m_Color) ;

    // Draw last collision
    draw_Sphere(m_LastCollPos, Radius, XCOLOR_WHITE) ;
    draw_Point(m_LastCollPos, m_Color) ;
}

#endif

//==============================================================================

void particle::Reset( void )
{
    m_Pos = m_LastPos = m_LastCollPos = m_BindPos ;
}

//==============================================================================

