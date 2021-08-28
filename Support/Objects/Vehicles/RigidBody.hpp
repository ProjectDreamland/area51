//==============================================================================
//
//  RigidBody.hpp
//
//==============================================================================

#ifndef __RIGID_BODY_HPP__
#define __RIGID_BODY_HPP__

//==============================================================================
// INCLUDES
//==============================================================================
#include "x_files.hpp"
#include "x_math.hpp"
#include "Objects\Render\RigidInst.hpp"
#include "Animation\AnimData.hpp"

#define MAX_RIGID_BODY_VERTICES 2048
//==============================================================================
// CLASSES
//==============================================================================

// Class that represents a single rigid body
class rigid_body
{
// Structures
protected:

// Data
public:
            // Geometry vars
            guid                m_ObjectGuid ;              // Owner guid (or NULL if none)
            rigid_inst          m_RigidInst ;               // Rigid instance geometry
            vector3             m_RenderOffset ;            // Rigid geometry render offset
            s32                 m_nVertices ;               // # of vertices
            vector3             m_Vertex[MAX_RIGID_BODY_VERTICES] ;           // List of vertices
            bbox                m_LocalBBox ;               // BBox of all local verts                
            //s32                 m_NBones ;                  // # of bones in heirarchy
            //rhandle<anim_group> m_hAnimGroup ;              // Animation group handle

            // Definition physics
            vector3             m_Gravity ;                 // World space gravity
            matrix3             m_InvLocalInertiaTensor ;   // Local space inertia tensor
            f32                 m_Mass ;                    // Mass of body
            f32                 m_InvMass ;                 // (1.0f / Mass) of body
            f32                 m_LinearDamping ;           // Linear velocity damping
            f32                 m_AngularDamping ;          // Angular velcity damping
            f32                 m_Friction ;                // Friction (0 = none, 1 = full)
            f32                 m_Elasticity ;              // Bouncyness (0 = none, 1 = 100%)

            // Dynamic physics
            xbool               m_bActive ;                 // TRUE if active
            vector3             m_Position ;                // World position
            quaternion          m_Orientation ;             // World orientation
                                
            vector3             m_LinearVelocity ;          // Linear velocity in world space
            vector3             m_AngularVelocity ;         // Angular velocity in local space
                                
            vector3             m_LinearMomentum ;          // World space velocity * Mass
            vector3             m_AngularMomentum ;         // World space angular velocity * mass
                                
            vector3             m_Force ;                   // Total of forces on CM in world space
            vector3             m_Torque ;                  // Total moments on CM in world space
                                
            matrix3             m_InvWorldInertiaTensor ;   // Inverse of world space intertial tensor
                                
            matrix4             m_L2W ;                     // Local to world matrix
            f32                 m_DeltaTime ;               // Current delta time

// Functions
public:
            // Constructor/Destructor
            rigid_body() ;
    virtual ~rigid_body() ;

            // Returns index of vertex if found, or -1 if not found
            s32                 FindVertex          ( const vector3& V ) const ;
                                                    
            // Initialization                       
    virtual void                Init                ( const rigid_geom& RigidGeom,
                                                      const matrix4&    L2W,
                                                            f32         MassYScale = 1.0f,  
                                                            guid        ObjectGuid = 0) ;

    virtual void                Reset               ( const matrix4& L2W ) ;
                                                    
            // Adds force to body                   
            void                AddForce            ( const vector3& Position, const vector3& Force ) ;

            // Adds impulse (immediate force) to body
            void                AddImpulse          ( const vector3& Position, const vector3& Force ) ;

            // Checks collision and applies forces when needed
            void                ProcessCollision    ( const vector3& P, const plane& Plane ) ;
            void                CheckCollision      ( void ) ;
            xbool               CheckCollision      ( const vector3& P, plane& Plane, f32& Depth ) ;
            xbool               CheckCollision      ( const vector3& S, const vector3& E, plane& Plane, f32& T ) ;
            
            // Computes forces                      
    virtual void                ComputeForces       ( f32 DeltaTime ) ;
                                                    
            // Integrates movement                  
            void                Integrate           ( f32 DeltaTime ) ;
                                                    
            // Advances logic                       
    virtual void                AdvanceSimulation   ( f32 DeltaTime ) ;
            void                Advance             ( f32 DeltaTime ) ;
                                                    
            // Renders geometry                     
    virtual void                Render              ( void ) ;
                                            
            // Position functions
            const vector3&      GetPosition         ( void ) const ;
            const quaternion&   GetOrientation      ( void ) const ;
            const matrix4&      GetL2W              ( void ) const ;
            f32                 GetMass             ( void ) const ;
            const bbox&         GetLocalBBox        ( void ) const ;
            const vector3&      GetLinearMomentum   ( void ) const ;
            const vector3&      GetRenderOffset     ( void ) const ;
            const vector3&      GetGravity          ( void ) const ;

            void                SetPosition         ( const vector3& Pos ) ;
            void                SetOrientation      ( const radian3& Rot ) ;
            void                SetL2W              ( const matrix4& L2W ) ;
    virtual void                ZeroVelocities      ( void ) ;
            
            vector3             GetVelocity         ( const vector3& P ) const ;
            vector3             GetAngularVelocity  ( const vector3& P ) const ;
            vector3             GetVelocity         ( void ) const ;
                                                    
} ;

//==============================================================================
// UTIL FUNCTIONS
//==============================================================================

// Returns bounding box of rigid geom bone
bbox GetRigidGeomBoneBBox( const rigid_geom* pRigidGeom, s32 iBone ) ;



#endif  // #ifndef __RIGID_BODY_HPP__
