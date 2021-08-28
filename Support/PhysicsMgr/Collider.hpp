//==============================================================================
//
//  Collider.hpp
//
//==============================================================================

#ifndef __COLLIDER_HPP__
#define __COLLIDER_HPP__

//==============================================================================
// INCLUDES
//==============================================================================

#include "Physics.hpp"
#include "CollisionMgr\PolyCache.hpp"


//==============================================================================
// FORWARD DECLARATIONS
//==============================================================================
class rigid_body;
class collision_shape;


//==============================================================================
// CLASS
//==============================================================================
class collider
{
//==============================================================================
// Defines
//==============================================================================
public:

    enum defines
    {
        MAX_COLLISIONS = 5,
        MAX_CLUSTERS   = 512,
    };

//==============================================================================
// Structures
//==============================================================================

    // Collision
    struct collision
    {
        // These must be filled out
        f32     m_Depth;      // Penetration depth
        vector3 m_Point;      // World position
        vector3 m_Normal;     // World normal
    };


//==============================================================================
// Public functions
//==============================================================================
public:

        // Constructor/destructor
        collider();
        ~collider();

        // Clears internal collisions
        void    ClearCollisions     ( void ) X_SECTION(physics);
        
        // Reports collisions to physics manager
        s32     ReportCollisions    ( collision_shape* pColl0, collision_shape* pColl1 ) X_SECTION(physics);
        
        // Adds to collision list
        void    RecordCollision     ( const collision& Collision ) X_SECTION(physics);
        
        // Checks collision between 2 moving shapes
        s32     Check               ( collision_shape* pColl0, collision_shape* pColl1 ) X_SECTION(physics);
        
        // Collects world polys of bbox and returns # of clusters
        s32    CollectWorldPolys   ( const bbox& WorldBBox ) X_SECTION(physics);
        
//==============================================================================
// Private functions
//==============================================================================
private:

        // Returns TRUE if sphere intersects NGon
        xbool   SphereNGonIntersection  ( const plane&   Plane,
                                          const vector3* Verts, 
                                          const s32      nVerts, 
                                          const vector3& SpherePos, 
                                          const f32      SphereRadius );

        // Returns TRUE if sphere intersects any of world
        xbool   SphereWorldIntersection ( const vector3& SpherePos, const f32 SphereRadius );

        // Check sweeping sphere V NGon collision
        xbool   SphereNGonCollision     ( const plane&      Plane,
                                          const vector3*    Verts,
                                          const s32         nVerts,
                                          const vector3&    SphereStart,
                                          const vector3&    SphereEnd,
                                          const f32         SphereRadius,
                                                f32&        CollT,
                                                plane&      CollPlane ) X_SECTION(physics);
                                  
        // Checks sweeping sphere V world collision        
        xbool   SphereWorldCollision    ( const vector3&    SphereStartPos,
                                          const vector3&    SphereEndPos,
                                          const f32         SphereRadius,
                                                f32&        CollT,
                                                plane&      CollPlane ) X_SECTION(physics);

        // Checks static sphere V sphere collision
        void    SphereSphereCollision   ( const vector3&    SpherePos0,
                                          const f32         SphereRadius0,
                                          const vector3&    SpherePos1,
                                          const f32         SphereRadius1 ) X_SECTION(physics);

        // Checks static sphere V sphere collision
        void    SphereCapsuleCollision  ( const vector3&    SpherePos,
                                          const f32         SphereRadius,
                                          const vector3&    CapsuleStartPos,
                                          const vector3&    CapsuleEndPos,
                                          const f32         CapsuleRadius ) X_SECTION(physics);

        // Checks static capsule V capsule collision
        void    CapsuleCapsuleCollision  ( const vector3&    CapsuleStartPos0,
                                           const vector3&    CapsuleEndPos0,
                                           const f32         CapsuleRadius0,
                                           const vector3&    CapsuleStartPos1,
                                           const vector3&    CapsuleEndPos1,
                                           const f32         CapsuleRadius1 ) X_SECTION(physics);

public:
        // Checks collision between swept spheres and world
        void    CheckSpheresWorld       ( collision_shape* pColl0, collision_shape* pColl1 ) X_SECTION(physics);

        // Checks collision between 2 spheres lists
        void    CheckSpheresSpheres     ( collision_shape* pColl0, collision_shape* pColl1 ) X_SECTION(physics);
        
        // Checks collision between spheres and capsule
        void    CheckSpheresCapsule     ( collision_shape* pColl0, collision_shape* pColl1 ) X_SECTION(physics);

        // Checks collision between 2 capsules
        void    CheckCapsuleCapsule     ( collision_shape* pColl0, collision_shape* pColl1 ) X_SECTION(physics);

//==============================================================================
// Data
//==============================================================================
protected:

        // Deepest collisions
        collision_shape*    m_pMovingColl;
        collision           m_Collisions[MAX_COLLISIONS];
        s32                 m_nMaxCollisions; // Dynamic max collisions

        // Poly cache info        
        poly_cache::cluster* m_pClusters[MAX_CLUSTERS];
        s32                  m_nClusters;
        bbox                 m_ClusterListBBox;
        
};

//==============================================================================

//==============================================================================
// DATA
//==============================================================================

extern collider g_Collider;


#endif  //#define __COLLIDER_HPP__
