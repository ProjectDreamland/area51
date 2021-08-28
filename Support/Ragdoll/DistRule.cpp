//==============================================================================
//
//  DistRule.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "DistRule.hpp"
#include "Entropy.hpp"


//==============================================================================
// CLASSES
//==============================================================================

dist_rule::dist_rule( void )
{
    m_Type              = DIST_EQUAL ;
    m_ParticleOffset[0] = -1;
    m_ParticleOffset[1] = -1;
    m_Dist              = 0 ;
    m_DistSqr           = 0 ;
    m_Damp              = 1.0f ;

#ifdef X_DEBUG    
    m_pName             = NULL ;
    m_bBone             = FALSE ;
    m_Color             = XCOLOR_WHITE ;
    m_Status            = 0 ;
#endif    
}

//==============================================================================

void dist_rule::Init(       dist_rule::type Type, 
                      const char*           pName, 
                            particle        Particles[],
                            s32             ParticleA,
                            s32             ParticleB,
                            f32             Dist, 
                            xbool           bBone,
                            f32             Damp, 
                            xcolor          Color )
{
    ASSERT(pName) ;

    // Lookup particles
    ASSERT(ParticleA != -1) ;
    ASSERT(ParticleB != -1) ;
    ASSERT(ParticleA != ParticleB) ;
    particle* pA = &Particles[ParticleA] ;
    particle* pB = &Particles[ParticleB] ;

    // Make sure min dist rule is valid
    f32 CurrDist = (pA->m_Pos - pB->m_Pos).Length() ;
    if ((Type == DIST_MIN) && (CurrDist < Dist))
        Dist = CurrDist ;
    ASSERT(pA   != pB) ;
    ASSERT(Dist != 0) ;

    // Record info
    m_Type              = Type ;
    m_ParticleOffset[0] = ParticleA * sizeof(particle);
    m_ParticleOffset[1] = ParticleB * sizeof(particle);
    m_Dist              = Dist ;
    m_DistSqr           = Dist*Dist ;
    m_Damp              = Damp ;

    // Setup constraint distances squared
    switch( Type )
    {
    case DIST_MIN:
        m_MinDistSqr = m_DistSqr;
        m_MaxDistSqr = -F32_MAX;
        break;

    case DIST_MAX:
        m_MinDistSqr = F32_MAX;
        m_MaxDistSqr = m_DistSqr;
        break;

    default:    
        ASSERTS( 0, "Passed invalid type!" );
        // Drop through to setup
    case DIST_EQUAL:
        m_MinDistSqr = F32_MAX;
        m_MaxDistSqr = -F32_MAX;
        break;
    }
    
#ifdef X_DEBUG    
    m_pName             = pName ;
    m_bBone             = bBone ;
    m_Color             = Color ;
    m_Status            = 0 ;
#else
    (void)pName;
    (void)bBone;
    (void)Color;
#endif
}

//==============================================================================

#ifdef X_DEBUG

void dist_rule::Render( particle Particles[], xbool bBonesOnly /*= TRUE*/ )
{
    // Is rule active?
    if ((!m_Status) && (!m_bBone))
        return ;

    // Only render bones?
    if ((bBonesOnly) && (!m_bBone))
        return ;

    // Are particles valid?
    ASSERT( m_ParticleOffset[0] != -1 );
    ASSERT( m_ParticleOffset[1] != -1 );
    ASSERT( m_ParticleOffset[0] != m_ParticleOffset[1] );
    
    // Lookup particles
    particle& P0 = *(particle*) ( (u32)&Particles[0] + m_ParticleOffset[0] );
    particle& P1 = *(particle*) ( (u32)&Particles[0] + m_ParticleOffset[1] );

    // Draw
    draw_ClearL2W() ;
    draw_Begin(DRAW_LINES, DRAW_NO_ZBUFFER) ;
    draw_Color( m_Color ) ;
    draw_Vertex( P0.m_Pos ) ;
    draw_Vertex( P1.m_Pos ) ;
    draw_End() ;
}

#endif

//==============================================================================

