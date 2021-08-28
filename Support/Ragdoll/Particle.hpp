//==============================================================================
//
//  Particle.hpp
//
//==============================================================================

#ifndef __PARTICLE_HPP__
#define __PARTICLE_HPP__

//==============================================================================
// INCLUDES
//==============================================================================
#include "x_files.hpp"
#include "x_math.hpp"


//==============================================================================
// CLASSES
//==============================================================================

// Class to track particle position
struct particle
{
// Data
public:
            vector3         m_Pos ;             // Current position of particle
            f32             m_InvMass ;         // Reciprocal mass of particle (1.0f / Mass)
            f32             m_Mass ;            // Mass of particle
            s32             m_bCollision ;      // Collision flag
            s32             m_GeomBoneIndex ;   // Bind geometry bone index
            vector3         m_LastPos ;         // Last frame position of particle
            vector3         m_BindPos ;         // Object space bind position of particle
            vector3         m_LastCollPos ;     // Last collision free position of particle
            particle*       m_pParent ;         // Parent particle for angular dampening
            
            const char*     m_pName ;           // Name of particle
            xcolor          m_Color ;           // Debug color of particle

// Functions
public:
    // Constructor
            particle () ;

    // Initialization
    void    Init  ( const char*     pName, 
                    const vector3&  Pos, 
                          f32       Mass, 
                          xbool     bCollision = FALSE, 
                          xcolor    Color = XCOLOR_WHITE) ;

    // Sets the bind to the current position
    void    SetBind( void ) ;

    // Sets up parent particle
    void    SetParent( particle* pParent ) ;

#ifdef X_DEBUG

    // Renders the particle
    void    Render( f32 Radius ) ;

#endif
    
    // Clears velocity, and resets the position to the bind position
    void    Reset ( void ) ;

    // Sets the velocity of the particle
    void    SetVelocity( const vector3& Vel ) ;

    // Returns velocity of the particle
    vector3 GetVelocity( void ) const ;

    // Returns (1.0f / Mass) of particle
    f32     GetInvMass ( void ) const ;

} PS2_ALIGNMENT(16);


//==============================================================================

inline
void particle::SetVelocity( const vector3& Vel )
{
    ASSERT(Vel.IsValid()) ;

    m_LastPos = m_Pos - Vel ;
}

//==============================================================================

inline
vector3 particle::GetVelocity( void ) const
{
    return m_Pos - m_LastPos ;
}

//==============================================================================

inline
f32 particle::GetInvMass ( void ) const
{
    return m_InvMass ;
}

//==============================================================================

#endif  // #ifndef __PARTICLE_HPP__
