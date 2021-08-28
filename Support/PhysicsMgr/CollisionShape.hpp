//==============================================================================
//
//  CollisionShape.hpp
//
//==============================================================================

#ifndef __COLLISION_SHAPE_HPP__
#define __COLLISION_SHAPE_HPP__

//==============================================================================
// INCLUDES
//==============================================================================

#include "Physics.hpp"


//==============================================================================
// FORWARD DECLARATIONS
//==============================================================================
class shape;
class rigid_body;
struct collision_info;


//==============================================================================
// CLASSES
//==============================================================================
class collision_shape
{
//==============================================================================
// Defines
//==============================================================================
public:

    // Types
    enum type
    {
        TYPE_NULL = -1,     // Not defined
        
        TYPE_SPHERE,        // Sphere  (Pos, Radius)
        TYPE_BOX,           // 3D Box  (Width, Height, Length)
        TYPE_CAPSULE,       // Capsule (StartPos, EndPos, Radius)
        TYPE_WORLD,         // World (dummy)
        
        TYPE_COUNT          // Total 
    };

//==============================================================================
// Structures
//==============================================================================
public:
    // Internal swept sphere
    struct sphere
    {
        // Constant properties
        vector3 m_Offset;           // Local offset 
        
        // Computed members
        vector3 m_CollFreePos; // Last world collision free pos
        vector3 m_PrevPos;     // World start position
        vector3 m_CurrPos;     // World end position
        
#ifdef ENABLE_PHYSICS_DEBUG
        // Debug
        xbool   m_bCollision;       // Collision draw flag
    
        // Functions
        sphere();
        ~sphere();
#endif        

    };

//==============================================================================
// Functions
//==============================================================================
public:
            // Constructor/destructor
             collision_shape();
            ~collision_shape();

            // Type functions
            type            GetType             ( void ) const X_SECTION(physics);
            void            SetType             ( type Type ) X_SECTION(physics);

            // Owner functions
            void            SetOwner            ( rigid_body* pOwner ) X_SECTION(physics);
            rigid_body*     GetOwner            ( void ) const X_SECTION(physics);

            // Dimension functions
            void            SetRadius           ( f32 Radius ) X_SECTION(physics);
            f32             GetRadius           ( void ) X_SECTION(physics);

            // Query functions
            bbox            ComputeLocalBBox    ( void ) const X_SECTION(physics);
            bbox            ComputeWorldBBox    ( void ) const X_SECTION(physics);

            // Collision functions
            void            SetL2W              ( const matrix4& L2W ) X_SECTION(physics);
            void            SetL2W              ( const matrix4& PrevL2W, const matrix4& NextL2W ) X_SECTION(physics);
            
            // Sphere functions
            void            SetSphereCapacity   ( s32 Count ) X_SECTION(physics);
            void            AddSphere           ( const vector3& Offset ) X_SECTION(physics);
            s32             GetNSpheres         ( void ) const X_SECTION(physics);
            sphere&         GetSphere           ( s32 Index ) X_SECTION(physics);
      const sphere&         GetSphere           ( s32 Index ) const X_SECTION(physics);
          
            // Capsule functions  
      const vector3&        GetCapsulePrevStartPos  ( void ) const X_SECTION(physics);
      const vector3&        GetCapsulePrevEndPos    ( void ) const X_SECTION(physics);
      const vector3&        GetCapsuleCurrStartPos  ( void ) const X_SECTION(physics);
      const vector3&        GetCapsuleCurrEndPos    ( void ) const X_SECTION(physics);
            
#ifdef ENABLE_PHYSICS_DEBUG
            
            // Debug render functions    
            void            DebugClearCollision ( void ) X_SECTION(physics);
            void            DebugRender         ( const matrix4& L2W, xcolor Color ) X_SECTION(physics);
            
#endif

//==============================================================================
// Data
//==============================================================================
protected:
        type            m_Type;         // Type of collision
        rigid_body*     m_pOwner;       // Owner of collision_shape (or NULL)
        f32             m_Radius;       // Radius of spheres and/or capsule
        xarray<sphere>  m_Spheres;      // List of spheres
            
//==============================================================================
// Friends
//==============================================================================
friend class rigid_body;
friend class physics_mgr;
friend class physics_inst;
friend class collider;

};

//==============================================================================
// Constructor/destructor
//==============================================================================

#ifndef ENABLE_PHYSICS_DEBUG

inline
collision_shape::collision_shape() :
    m_Type      ( collision_shape::TYPE_NULL ),
    m_pOwner    ( NULL ),
    m_Radius    ( 0.0f )
{
}

//==============================================================================

inline
collision_shape::~collision_shape()
{
}

#endif  //#ifndef ENABLE_PHYSICS_DEBUG

//==============================================================================
// Type functions
//==============================================================================

inline
collision_shape::type collision_shape::GetType( void ) const
{
    return m_Type;
}

//==============================================================================

inline
void collision_shape::SetType( collision_shape::type Type )
{
    m_Type = Type;
}

//==============================================================================
// Owner functions
//==============================================================================

inline
void collision_shape::SetOwner( rigid_body* pOwner )
{
    m_pOwner = pOwner;
}

//==============================================================================

inline
rigid_body* collision_shape::GetOwner( void ) const
{
    return m_pOwner;
}

//==============================================================================
// Dimension functions
//==============================================================================

inline
void collision_shape::SetRadius ( f32 Radius )
{
    m_Radius = Radius;
}

//==============================================================================

inline
f32 collision_shape::GetRadius ( void )
{
    return m_Radius;
}

//==============================================================================
// Sphere functions
//==============================================================================

inline
void collision_shape::SetSphereCapacity( s32 Count )
{
    m_Spheres.SetCapacity( Count );
}

//==============================================================================

inline 
s32 collision_shape::GetNSpheres( void ) const
{
    return m_Spheres.GetCount();
}

//==============================================================================

inline
collision_shape::sphere& collision_shape::GetSphere( s32 Index )
{
    return m_Spheres[ Index ];
}

//==============================================================================

inline
const collision_shape::sphere& collision_shape::GetSphere( s32 Index ) const
{
    return m_Spheres[ Index ];
}

//==============================================================================
// Capsule functions  
//==============================================================================

inline
const vector3& collision_shape::GetCapsulePrevStartPos( void ) const
{
    ASSERT( m_Type == TYPE_CAPSULE );
    ASSERT( m_Spheres.GetCount() > 1 );

    return m_Spheres[0].m_PrevPos;
}

//==============================================================================

inline
const vector3& collision_shape::GetCapsulePrevEndPos( void ) const
{
    ASSERT( m_Type == TYPE_CAPSULE );
    ASSERT( m_Spheres.GetCount() > 1 );

    return m_Spheres[ m_Spheres.GetCount() - 1 ].m_PrevPos;
}

//==============================================================================

inline
const vector3& collision_shape::GetCapsuleCurrStartPos( void ) const
{
    ASSERT( m_Type == TYPE_CAPSULE );
    ASSERT( m_Spheres.GetCount() > 1 );

    return m_Spheres[0].m_CurrPos;
}

//==============================================================================

inline
const vector3& collision_shape::GetCapsuleCurrEndPos( void ) const
{
    ASSERT( m_Type == TYPE_CAPSULE );
    ASSERT( m_Spheres.GetCount() > 1 );

    return m_Spheres[ m_Spheres.GetCount() - 1 ].m_CurrPos;
}

//==============================================================================


#endif  //#define __COLLISION_SHAPE_HPP__
