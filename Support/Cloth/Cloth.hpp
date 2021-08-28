//==============================================================================
//
//  Cloth.hpp
//
//==============================================================================

#ifndef __CLOTH_HPP__
#define __CLOTH_HPP__

//==============================================================================
// INCLUDES
//==============================================================================
#include "x_files.hpp"
#include "Objects\Render\RigidInst.hpp"
#include "CollisionMgr\CollisionMgr.hpp"


//==============================================================================
// FORWARD DECLARATIONS
//==============================================================================
class pain;


//==============================================================================
// CLASSES
//==============================================================================


// Particle
class cloth_particle
{
// Functions
public:
             cloth_particle();

    // Mass functions
    f32     GetMass( void ) const;
    void    SetMass( f32 Mass );
        
// Data        
public:
    vector3 m_BindPos ;     // Bind position
    vector3 m_Pos ;         // Current position
    vector3 m_LastPos ;     // Last frame position
    vector3 m_Normal ;      // Normal
    vector2 m_UV ;          // Texture UV
    f32     m_InvMass ;     // 1.0f / Mass
    xcolor  m_Color ;       // Render color
} ;

//==============================================================================

// Connection between 2 particles
class cloth_connection
{
public:
    s16     m_ParticleA ;   // Particle A
    s16     m_ParticleB ;   // Particle B
    f32     m_RestDistSqr ; // Rest distance squared between particles
} ;

//==============================================================================

// Triangle made out of 3 particles
class cloth_triangle
{
public:
    s16     m_Particles[3] ;    // Indices into 3 particles
} ;

//===========================================================================

// Cloth
class cloth
{
// Defines
public:

// Structures
private:

// Data
public:
    
            // Physics components
            xarray<cloth_particle>      m_Particles;            // List of particles
            xarray<cloth_connection>    m_Connections;          // List of connections
            xarray<cloth_triangle>      m_Triangles;            // List of triangles
            f32                         m_MinDist;              // Min distance squared between particles
            bbox                        m_LocalCollBBox ;       // Local space axis aligned fast collision box
            bbox                        m_WorldCollBBox ;       // World space axis aligned fast collision box

            // Physics properties
            vector3                     m_Gravity;              // Gravity to add to cloth
            f32                         m_Dampen;               // Dampen amount
            f32                         m_Stretch;              // Stretch amount
            s32                         m_nIterations;          // # of constraint iterations
            f32                         m_ImpactScale;          // Impact scale of bullets
            f32                         m_SimulationTime;       // Time to simulate before level starts
            
            // Rendering components
            rigid_inst                  m_RigidInst ;           // Rigid instance
            u64                         m_RenderMask ;          // Render instance mask
            s32                         m_MaterialIndex ;       // Index of cloth material in rigid inst
            xbitmap                     m_DamageBMP ;           // Damage bitmap
            f32                         m_DeltaTime ;           // Accumulated delta time
            guid                        m_ObjectGuid ;          // Owner guid (or NULL if none)
            matrix4                     m_L2W ;                 // Local to world matrix
            matrix4                     m_W2L ;                 // World to local matrix
            bbox                        m_LocalBBox ;           // Local space bbox
            bbox                        m_WorldBBox ;           // World space bbox
            xcolor                      m_LightAmbColor;        // Ambient color
            f32                         m_LightDirAmount;       // Amount of directional light

            // Wind vars
            xbool                       m_bWind;                // Wind on or off?
            f32                         m_WindTimer;            // Time of on/off
            vector3                     m_WindDir;              // Direction of wind
            radian3                     m_WindRot;              // Rotation of wind
            f32                         m_WindMinOnTime;        // Min amount of time wind is on
            f32                         m_WindMaxOnTime;        // Max amount of time wind is on
            f32                         m_WindMinOffTime;       // Min amount of time wind is off
            f32                         m_WindMaxOffTime;       // Max amount of time wind is off
            f32                         m_WindMinStrength;      // Min strength of wind
            f32                         m_WindMaxStrength;      // Max strength of wind

            // Static vars
static      byte                        m_DamageClut[16*4] ;    // 16 colors, 4 bytes each for RGBA

// Functions
public:
         cloth() ;
         ~cloth() ;

// Functions

    // Private initialization functions
private:

            s32         FindParticle            ( const vector3& P ) ;
            s32         AddParticle             ( const vector3& P, const vector2& UV ) ;
            void        PegParticles            ( s32 iAxis, s32 Dir );
            void        ClearAllInvMasses       ( void );
            
            s32         FindConnection          ( s32 ParticleA, s32 ParticleB ) ;
            s32         AddConnection           ( s32 ParticleA, s32 ParticleB ) ;
            
            s32         FindTriangleVert        ( s32 Triangle, s32 Vertex ) ;
            s32         FindTriangle            ( s32 Verts[3] ) ;
            void        AddTriangle             ( const vector3& P0, const vector2& UV0,
                                                  const vector3& P1, const vector2& UV1,
                                                  const vector3& P2, const vector2& UV2 ) ;

    // Public initialization functions
public:
            void        SetObjectGuid           ( guid Guid ) { m_ObjectGuid = Guid; }
            void        Init                    ( void );
            void        Reset                   ( void ) ;
            void        Kill                    ( void ) ;


            void        OnEnumProp              ( prop_enum&    rList  );
            xbool       OnProperty              ( prop_query&   rQuery );
private:

    // Physics functions
public:
            void        ApplyCappedCylinderColl ( const vector3& Bottom, const vector3& Top, f32 Radius ) ;
            void        OnColCheck              ( guid ObjectHitGuid, u32 Flags ) ;


private:
            void        ApplyDistConstraints    ( void ) ;
            void        ApplyCollConstraints    ( void ) ;
            void        ApplyConstraints        ( void ) ;
            void        Integrate               ( f32 DeltaTime ) ;
            void        ComputeLighting         ( void ) ;
            void        ComputeBounds           ( void ) ;
            void        ComputeWorldCollBBox    ( void ) ;

public:

    // Applies blast force to particles
            void        ApplyBlast              ( const vector3& Pos, f32 Radius, f32 Amount ) ;
            void        ApplyBlast              ( const vector3& Pos, const vector3& Dir, f32 Radius, f32 Amount );

    // Applies pain blast to particles
            void        OnPain                  ( const pain& Pain );
            
    // Punch a hole in the damage texture
            s32         PunchDamage             ( f32 U, f32 V ) ;
            s32         CheckDamage             ( f32 U, f32 V ) ;      // Don't punch out the hole
            void        OnProjectileImpact      ( const object&          ProjectileObject,
                                                  const vector3&         ProjectileVel,
                                                        u32              CollPrimKey, 
                                                  const vector3&         CollPoint,
                                                  xbool                  PunchDamageHole,       // TRUE to punch out a hole
                                                  f32                    ManualImpactForce );    // <= 0 : normal behaviour for bullets
                                                                                                // >  0 : scale force to this length


    // Logic functions
            void        Advance                 ( f32 DeltaTime) ;
            
    // Set functions
            void        SetL2W                  ( const matrix4& L2W, xbool bReset = TRUE, f32 AirResistance = 0.0f ) ;
            void        SetLocalCollBBox        ( const bbox& BBox ) ;

    // Render functions
            void        RenderClothGeometry     ( s32 VTexture = 0 ) ;
            void        RenderRigidGeometry     ( u32 Flags = render::CLIPPED );

#if !defined( CONFIG_RETAIL )
            void        RenderSkeleton          ( void ) ;
#endif // !defined( CONFIG_RETAIL )

    // Query functions
          rigid_inst&   GetRigidInst            ( void ) ;
          u64           GetRenderMask           ( void ) const;
    const bbox&         GetLocalCollBBox        ( void ) ;
    const bbox&         GetLocalBBox            ( void ) const ;
    const bbox&         GetWorldBBox            ( void ) const ;

    // Xbox specific rendering functions
private:
    #ifdef TARGET_XBOX
    void        RenderPrimingPass       ( texture* pTexture );
    void        RenderNormalPass        ( texture* pTexture );
    #endif
} ;

//==============================================================================
// Query functions
//==============================================================================

inline
rigid_inst& cloth::GetRigidInst( void )
{
    return m_RigidInst;
}

//==============================================================================

inline
u64 cloth::GetRenderMask( void ) const
{
    return m_RenderMask;
}

//==============================================================================

inline
const bbox& cloth::GetLocalCollBBox( void )
{
    return m_LocalCollBBox;
}

//==============================================================================

inline
const bbox& cloth::GetLocalBBox( void ) const
{
    return m_LocalBBox;
}

//==============================================================================

inline
const bbox& cloth::GetWorldBBox( void ) const
{
    return m_WorldBBox;
}

//==============================================================================


#endif  // #ifndef __CLOTH_HPP__
