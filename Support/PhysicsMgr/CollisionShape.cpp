//==============================================================================
//
//  CollisionShape.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "CollisionShape.hpp"
#include "PhysicsMgr.hpp"


//==============================================================================
// SPHERE FUNCTIONS
//==============================================================================

#ifdef ENABLE_PHYSICS_DEBUG

collision_shape::sphere::sphere()
{
    PHYSICS_DEBUG_DYNAMIC_MEM_ALLOC( sizeof( collision_shape::sphere ) );
}            

//==============================================================================

collision_shape::sphere::~sphere()
{
    PHYSICS_DEBUG_DYNAMIC_MEM_FREE( sizeof( collision_shape::sphere ) );
}            

#endif  //#ifdef ENABLE_PHYSICS_DEBUG


//==============================================================================
// SHAPE FUNCTIONS
//==============================================================================

#ifdef ENABLE_PHYSICS_DEBUG

collision_shape::collision_shape() :
    m_Type      ( collision_shape::TYPE_NULL ),
    m_pOwner    ( NULL ),
    m_Radius    ( 0.0f )
{
    PHYSICS_DEBUG_DYNAMIC_MEM_ALLOC( sizeof( collision_shape ) );
}

//==============================================================================

collision_shape::~collision_shape()
{
    PHYSICS_DEBUG_DYNAMIC_MEM_FREE( sizeof( collision_shape ) );
}

#endif

//==============================================================================

void collision_shape::AddSphere( const vector3& Offset )
{
    // Create new sphere
    sphere& Sphere = m_Spheres.Append();

    // Init sphere
    Sphere.m_Offset    = Offset;
    Sphere.m_CollFreePos.Zero();
    Sphere.m_PrevPos.Zero();
    Sphere.m_CurrPos.Zero();

#ifdef ENABLE_PHYSICS_DEBUG
    Sphere.m_bCollision = FALSE;
#endif        
}

//==============================================================================
// BBox functions
//==============================================================================

bbox collision_shape::ComputeLocalBBox( void ) const
{
    // Compute local bbox
    bbox LocalBBox;
    LocalBBox.Clear();

    // Add all spheres to bounds
    for( s32 i = 0; i < m_Spheres.GetCount(); i++ )
    {
        // Get sphere
        sphere& Sphere = m_Spheres[i];

        // Add local sphere center to local bbox        
        LocalBBox += Sphere.m_Offset;
    }

    // Take sphere radius and float error into account
    f32 Inflate = m_Radius + 1.0f;
    LocalBBox.Inflate( Inflate, Inflate, Inflate );

    return LocalBBox;
}

//==============================================================================

bbox collision_shape::ComputeWorldBBox( void ) const
{
    // Clear world bbox
    bbox WorldBBox;
    WorldBBox.Clear();

    // Add all spheres to bounds
    f32 Radius = m_Radius + 1.0f;
    for( s32 i = 0; i < m_Spheres.GetCount(); i++ )
    {
        // Lookup sphere info
        sphere& Sphere = m_Spheres[i];

        // Compute movement bbox
        bbox    MoveBBox( Sphere.m_PrevPos, Sphere.m_CurrPos );
        MoveBBox.Inflate( Radius, Radius, Radius );

        // Compute movement sphere bounds
        bbox    MoveSphereBBox( MoveBBox.GetCenter(), MoveBBox.GetRadius() );

        // Compute collision bbox
        bbox    CollBBox( Sphere.m_CollFreePos, Radius );

        // Update world bounds
        WorldBBox += MoveSphereBBox;
        WorldBBox += CollBBox;
    }

    // For float error    
    WorldBBox.Inflate( 1.0f, 1.0f, 1.0f );

    return WorldBBox;
}

//==============================================================================

void collision_shape::SetL2W( const matrix4& L2W )
{
    ASSERT( L2W.IsValid() );

    // Put spheres and collision pos into world space
    for( s32 i = 0; i < m_Spheres.GetCount(); i++ )
    {
        // Look up sphere
        sphere& Sphere = m_Spheres[i];

        // Compute world position
        vector3 WorldPos = L2W * Sphere.m_Offset;
        Sphere.m_CollFreePos = WorldPos;
        Sphere.m_PrevPos     = WorldPos;
        Sphere.m_CurrPos     = WorldPos;
    }
}

//==============================================================================

void collision_shape::SetL2W( const matrix4& PrevL2W, const matrix4& NextL2W )
{
    ASSERT( PrevL2W.IsValid() );
    ASSERT( NextL2W.IsValid() );

    // Put spheres into world space
    for( s32 i = 0; i < m_Spheres.GetCount(); i++ )
    {
        // Look up sphere
        sphere& Sphere = m_Spheres[i];

        // Compute world positions
        Sphere.m_PrevPos = PrevL2W * Sphere.m_Offset;
        Sphere.m_CurrPos = NextL2W * Sphere.m_Offset;
    }
}

//==============================================================================
// Render functions
//==============================================================================

#ifdef ENABLE_PHYSICS_DEBUG

void collision_shape::DebugClearCollision( void )
{
    // Clear sphere collision rendering
    s32 nSpheres = GetNSpheres();
    for( s32 i = 0; i < nSpheres; i++ )
    {
        collision_shape::sphere& Sphere = GetSphere( i );
        Sphere.m_bCollision = FALSE;
    }
}

//==============================================================================

void collision_shape::DebugRender( const matrix4& L2W, xcolor Color )
{
    // Render spheres
    draw_ClearL2W();
    s32 nSpheres = GetNSpheres();
    for( s32 i = 0; i < nSpheres; i++ )
    {
        collision_shape::sphere& Sphere = GetSphere( i );

        if( Sphere.m_bCollision )
            draw_Sphere( Sphere.m_CollFreePos, m_Radius, XCOLOR_RED );
        else
            draw_Sphere( Sphere.m_CollFreePos, m_Radius, XCOLOR_YELLOW );

        draw_Sphere( Sphere.m_CurrPos, m_Radius, XCOLOR_GREEN );
    }

    // Render world bbox
    draw_SetL2W( L2W );
    draw_BBox( ComputeLocalBBox(), Color );
    draw_ClearL2W();
}

#endif  //#ifdef ENABLE_PHYSICS_DEBUG

//==============================================================================
