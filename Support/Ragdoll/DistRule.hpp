//==============================================================================
//
//  DistRule.hpp
//
//==============================================================================

#ifndef __DIST_RULE_HPP__
#define __DIST_RULE_HPP__

//==============================================================================
// INCLUDES
//==============================================================================
#include "x_files.hpp"
#include "x_math.hpp"
#include "Particle.hpp"

//==============================================================================
// CLASSES
//==============================================================================

struct dist_rule
{
// Defines
public:
    enum type
    {
        DIST_EQUAL,                 // Keeps particle equal distance apart
        DIST_MIN,                   // Does not let particles get closest than distance
        DIST_MAX                    // Does not let particles get further than distance
    } ;

// Data
public:
    s32         m_ParticleOffset[2] ;   // Particle offset (bytes)
    f32         m_Dist ;                // Distance of rule
    f32         m_DistSqr ;             // Distance squared
    f32         m_MaxDistSqr;           // Max distance constraint
    f32         m_MinDistSqr;           // Min distance constraint
    f32         m_Damp ;                // Dampen amount
    s32         m_Type ;                // Type of distance rule
    
#ifdef X_DEBUG
    const char* m_pName ;               // Name of rule
    xbool       m_bBone ;               // TRUE if it's a bone
    xcolor      m_Color ;               // Color
    s32         m_Status ;              // Runtime status
#endif

// Functions
public:

    // Constructor
         dist_rule( void ) ;

    // Initialization
    void Init   (        type      Type, 
                   const char*     pName, 
                         particle  Particles[],
                         s32       ParticleA,
                         s32       ParticleB,
                         f32       Dist, 
                         xbool     bBone,
                         f32       Damp = 1, 
                         xcolor    Color = XCOLOR_WHITE ) ;

    // Applies rules to particles
    void Apply  ( particle Particles[] );

#ifdef X_DEBUG
    // Renders rule
    void Render ( particle Particles[], xbool bBonesOnly = TRUE ) ;
#endif

} PS2_ALIGNMENT(16);

//==============================================================================

inline
void dist_rule::Apply( particle Particles[] )
{
    // Lookup particles
    particle& P0 = *(particle*) ( (u32)&Particles[0] + m_ParticleOffset[0] );
    particle& P1 = *(particle*) ( (u32)&Particles[0] + m_ParticleOffset[1] );

#ifdef X_DEBUG
    // Update status
    if (m_Status)
        m_Status-- ;
#endif

#if USE_VU0
    // Vectors
    u128 DELTA;
    u128 DOT;
    u128 DIFF;
    u128 DELTA0;
    u128 DELTA1;

    // Floats
    f32  DistSqr;

    // Compute Delta = PosB - PosA
    asm( "vsub.xyz  VOUT, VIN1, VIN0"  : "=j VOUT" (DELTA)   : "j VIN0" (P0.m_Pos.GetU128() ), "j VIN1" (P1.m_Pos.GetU128()) );

    // Lookup inverse masses and compute total
    f32 InvMass0     = P0.m_InvMass;
    f32 InvMass1     = P1.m_InvMass;
    f32 TotalInvMass = InvMass0 + InvMass1;

    // Compute distance squared
    asm( "vmul.xyz	VOUT, VIN,  VIN"   : "=j VOUT" (DOT)     : "j VIN" (DELTA) );
    asm( "vaddy.x	VOUT, VOUT, VINy"  : "=j VOUT" (DOT)     : "j VIN" (DOT) );
    asm( "vaddz.x	VOUT, VIN,  VINz"  : "=j VOUT" (DOT)     : "j VIN" (DOT) );
    asm( "qmfc2     FOUT, VIN"         : "=r FOUT" (DistSqr) : "j VIN" (DOT) );

    // If both particles are immovable, then exit
    if( TotalInvMass == 0.0f )
        return;

    // Min distance constraint already satisfied?
    if ( DistSqr >= m_MinDistSqr )
        return;

    // Max distance constraint already satisfied?
    if ( DistSqr <= m_MaxDistSqr )
        return;

    // Compute correction dist and dampen
    f32 Diff = ( m_Damp * -2.0f * ( ( m_DistSqr / ( DistSqr + m_DistSqr ) ) - 0.5f ) ) / TotalInvMass ;

    // Get ready to compute position deltas
    asm( "qmtc2 FIN, VOUT"    : "=j VOUT" (DIFF)   : "r FIN" (Diff) );
    asm( "qmtc2 FIN, VOUT"    : "=j VOUT" (DELTA0) : "r FIN" (InvMass0) );
    asm( "qmtc2 FIN, VOUT"    : "=j VOUT" (DELTA1) : "r FIN" (InvMass1) );

    // Compute deltas
    asm( "vmulx.xyz  VOUT, VIN0, VIN1x" : "=j VOUT" (DELTA)  : "j VIN0" (DELTA), "j VIN1" (DIFF) );
    asm( "vmulx.xyz  VOUT, VIN0, VIN1x" : "=j VOUT" (DELTA0) : "j VIN0" (DELTA), "j VIN1" (DELTA0) );
    asm( "vmulx.xyz  VOUT, VIN0, VIN1x" : "=j VOUT" (DELTA1) : "j VIN0" (DELTA), "j VIN1" (DELTA1) );

    // Apply deltas to particles
    asm( "vadd.xyz  VOUT, VIN0, VIN1"  : "=j VOUT" (DELTA0) : "j VIN0" (P0.m_Pos.GetU128() ), "j VIN1" (DELTA0) );
    asm( "vsub.xyz  VOUT, VIN0, VIN1"  : "=j VOUT" (DELTA1) : "j VIN0" (P1.m_Pos.GetU128() ), "j VIN1" (DELTA1) );

    // Store results
    P0.m_Pos.Set( DELTA0 );
    P1.m_Pos.Set( DELTA1 );

#else   // #if USE_VU0

    // Lookup inverse masses and compute total
    f32 InvMass0     = P0.m_InvMass;
    f32 InvMass1     = P1.m_InvMass;
    f32 TotalInvMass = InvMass0 + InvMass1;

    // Get distance between particles
    vector3 Delta   = P1.m_Pos - P0.m_Pos ;
    f32     DistSqr = Delta.LengthSquared() ;

    // If both particles are immovable, then exit
    if( TotalInvMass == 0.0f )
        return;

    // Min distance constraint already satisfied?
    if ( DistSqr >= m_MinDistSqr )
        return;

    // Max distance constraint already satisfied?
    if ( DistSqr <= m_MaxDistSqr )
        return;

    // Move to target dist
    //f32 Dist = x_sqrt(DistSqr) ;
    //f32 Diff = (Dist - m_Dist) / (Dist * TotalInvMass) ;

    // Using sqrt approx
    f32 Diff = m_Damp * -2.0f * ((m_DistSqr / (DistSqr + m_DistSqr)) - 0.5f) / TotalInvMass ;

    // Scale
    Delta *= Diff;

    // Apply deltas
    P0.m_Pos += InvMass0 * Delta ;
    P1.m_Pos -= InvMass1 * Delta ;
    
#endif  // #if USE_VU0

#ifdef X_DEBUG
    // Draw
    m_Status = 1 ;
#endif
}

//==============================================================================

#endif  // #ifndef __DIST_RULE_HPP__
