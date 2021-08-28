//==============================================================================
//
//  VerletCollision.hpp
//
//  A collection of fast world collision functions for vertlet sphere based physics 
//
//==============================================================================

#ifndef __VERTLET_COLLISION_HPP__
#define __VERTLET_COLLISION_HPP__

//==============================================================================
// INCLUDES
//==============================================================================
#include "x_files.hpp"

//==============================================================================
// FORWARD DECLARATIONS
//==============================================================================
struct coll_object ;

//==============================================================================
// STRUCTURES
//==============================================================================

// Sphere casting structure, a reference of which is passed through all functions
struct sphere_cast
{
    // Output
    f32                 m_CollT ;           // Current closest collision T value (0=start, 1=end)
    plane               m_CollPlane ;       // Current closest collision plane
/*
    xbool               m_bCollLocal ;      // TRUE if current output is in local space
    coll_object*        m_pCollObject ;     // Current closest collided object
    const matrix4*      m_pCollL2W ;        // Current closest collided object L2W

    // Sphere input
    f32                 m_Radius ;          // Radius of moving sphere

    // Sphere movement world input
    vector3             m_WorldStart ;      // World space start position
    vector3             m_WorldEnd ;        // World space end position
    vector3             m_WorldDelta ;      // World space delta
    bbox                m_WorldBBox ;       // World space bbox around movement

    // Sphere movement local space input
    vector3             m_LocalStart ;      // Local space start position
    vector3             m_LocalEnd ;        // Local space end position
    vector3             m_LocalDelta ;      // Local space delta
    bbox                m_LocalBBox ;       // Local space bbox around movement

    // Object input
    coll_object*        m_pObject ;         // Current object to test collision with
    const matrix4*      m_pClusterL2W ;     // Current object cluster L2W being tested
*/
} ;


//==============================================================================
// FUNCTIONS
//==============================================================================

// Renders collision (useful for debugging)
void VerletCollision_Render( void ) ;

// Collects all the potential collidable objects within the world bbox
void VerletCollision_CollectObjects( const bbox& WorldBBox ) ;

// Fast sphere cast.
//  NOTE - You must call "VerletCollision_CollectObjects" before hand to setup
//         the possible collidable objects
xbool VerletCollision_SphereCast ( const vector3&       Start,
                                   const vector3&       End,
                                   const f32            Radius,
                                         sphere_cast&   Cast ) ;


#endif  // #ifndef __VERTLET_COLLISION_HPP__
