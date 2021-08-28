//==============================================================================
//
//  PhysicsInst.hpp
//
//==============================================================================

#ifndef __PHYSICS_INST_HPP__
#define __PHYSICS_INST_HPP__

//==============================================================================
// INCLUDES
//==============================================================================

#include "Animation\AnimData.hpp"
#include "Objects\Render\SkinInst.hpp"
#include "Animation\SMemMatrixCache.hpp"
#include "CollisionMgr\CollisionMgr.hpp"

#include "Physics.hpp"
#include "RigidBody.hpp"
#include "CollisionShape.hpp"
#include "Constraint.hpp"


//==============================================================================
// FORWARD DECLARATIONS
//==============================================================================
class rigid_body;
class constraint;
class collision_shape;
class loco_char_anim_player;


//==============================================================================
// CLASSES
//==============================================================================
class physics_inst
{
//==============================================================================
// Defines
//==============================================================================
public:
    
//==============================================================================
// Structures
//==============================================================================
public:
        
//==============================================================================
// Functions
//==============================================================================
public:

    // Constructor/destructor
             physics_inst();
            ~physics_inst();

    // Initialization functions
private:    
            xbool               Init                    ( xbool bPopFix, f32 ConstraintBlendTime );
            
public:            
            xbool               Init                    ( const char*      pGeomName, xbool bPopFix, f32 ConstraintBlendTime = 0.0f );
            xbool               Init                    ( const skin_inst& SkinInst,  xbool bPopFix, f32 ConstraintBlendTime = 0.0f );
            void                Kill                    ( void );
            
    // Render functions
            void                Render                  ( u32 Flags, xcolor Ambient );
            
            #ifdef ENABLE_PHYSICS_DEBUG
            void                DebugRender             ( void );
            void                RenderCollision         ( void );
            #endif
            
    // Position functions
            vector3             GetPosition             ( void )const X_SECTION(physics);
            
    // Active functions
            xbool               IsActive                ( void ) const X_SECTION(physics);
            f32                 GetSpeedSqr             ( void ) const X_SECTION(physics);    
            xbool               HasActiveEnergy         ( void ) const X_SECTION(physics);
            void                Deactivate              ( void ) X_SECTION(physics);
            void                Activate                ( void ) X_SECTION(physics);
            void                SetActiveWhenVisible    ( xbool bActive );
            void                SetKeepActiveTime       ( f32 Time );

    // Matrix functions
            void                DirtyMatrices           ( void ) X_SECTION(physics);
            const matrix4*      GetBoneL2Ws             ( u64& LODMask, s32& nActiveBones );
            vector3             GetBoneWorldPosition    ( s32 iBone );  // NOTE: Bind has been removed!
            matrix4             GetBoneWorldTransform   ( s32 iBone );  // NOTE: Bind has been removed!
            void                SetMatrices             ( loco_char_anim_player& AnimPlayer, const vector3& Vel );
            void                SetMatrices             ( const matrix4* pMatrices, s32 nBones, xbool bInheritVel );

            
    // Blast/force functions
            void                ApplyBlast              ( const vector3& Pos, f32 Radius, f32 Amount );
            void                ApplyBlast              ( const vector3& Pos, const vector3& Dir, f32 Radius, f32 Amount );
            void                ApplyVectorForce        ( const vector3& Dir, f32 Amount );
            
    // Collision functions
            void                OnColCheck              ( guid OwnerObject );
            void                ComputeWorldBBox        ( void ) X_SECTION(physics);
            const bbox&         GetWorldBBox            ( void ) const X_SECTION(physics);
            
    // Query functions
            anim_group::handle& GetAnimGroupHandle      ( void );
            skin_inst&          GetSkinInst             ( void );
            const skin_inst&    GetSkinInst             ( void ) const;
            const char*         GetAnimName             ( void ) const;
            const char*         GetSkinGeomName         ( void ) const;
            
    // Rigid body functions
            s32                 GetNRigidBodies         ( void ) const X_SECTION(physics);
            rigid_body&         GetRigidBody            ( s32 iRigidBody ) X_SECTION(physics);
            const char*         GetRigidBodyName        ( s32 iRigidBody ) const;
            
    // Collision functions
            s32                 GetNCollisionShapes     ( void ) const X_SECTION(physics);
            collision_shape&    GetCollisionShape       ( s32 Index ) X_SECTION(physics);
            void                SetActorCollision       ( xbool bEnable );
            xbool               GetActorCollision       ( void ) const X_SECTION(physics);
            void                SetWorldCollision       ( xbool bEnable );
            xbool               GetWorldCollision       ( void ) const X_SECTION(physics);
            void                SetInstCollision        ( xbool bEnable );
            xbool               GetInstCollision        ( void ) const X_SECTION(physics);
            
    // Constraint functions
            s32                 GetNBodyBodyConstraints     ( void ) const X_SECTION(physics);
            constraint&         GetBodyBodyConstraint       ( s32 Index ) X_SECTION(physics);
            
            s32                 GetNBodyWorldConstraints    ( void ) const X_SECTION(physics);
            constraint&         GetBodyWorldConstraint      ( s32 Index ) X_SECTION(physics);
            
            s32                 AddBodyWorldConstraint      ( s32            iRigidBody, 
                                                              const vector3& WorldPos,
                                                              f32            MaxDist );
                                                              
            void                SetBodyWorldConstraintWorldPos( s32 iConstraint, const vector3& WorldPos );                                                              
                                                              
            void                DeleteAllBodyWorldConstraints( void );                                                                  
                
            void                SetConstraintWeight     ( f32 Weight );                
            f32                 GetConstraintWeight     ( void ) const;
                            
    // Zone functions
            void                SetZone                 ( u8 Zone );                
            u8                  GetZone                 ( void ) const;

    // Logic functions            
            void                Advance                 ( f32 DeltaTime );
            
//==============================================================================
// Data
//==============================================================================

private:

    // Flags
    u32                             m_bInitialized           : 1;   // Initialized
    u32                             m_bInAwakeList           : 1;   // In physics mgr awake list
    u32                             m_bInSleepingList        : 1;   // In physics mgr sleeping list
    u32                             m_bInCollisionWakeupList : 1;   // In physics mgr collision wake up list
    u32                             m_bPopFix                : 1;   // Use pop fix matrices?
    u32                             m_bActorCollision        : 1;   // Should collision with actors occur?
    u32                             m_bWorldCollision        : 1;   // Should instance collides with the world?
    u32                             m_bInstCollision         : 1;   // Should instance collide with other instances?     
    u32                             m_bActiveWhenVisible     : 1;   // Bodies are always active when visible
    
public:

    // Linked list nodes
    linked_list_node<physics_inst>  m_ListNode;         // Physics mgr list node

protected:

    // Physics components
    xarray<rigid_body>              m_RigidBodies;              // List of rigid bodies
    xarray<collision_shape>         m_CollisionShapes;          // List of collision shapes
    xarray<constraint>              m_BodyBodyContraints;       // List of body -> body constraints
    xarray<constraint>              m_BodyWorldContraints;      // List of body -> world constraints
    xarray<matrix4>                 m_PopFixMatrices;           // Matrices to fix pop
    bbox                            m_WorldBBox;                // World bounding box
    f32                             m_ConstraintWeight;         // Current constraint strength
    f32                             m_ConstraintWeightDelta;    // Use to blend in constraints
    f32                             m_KeepActiveTime;           // Timer to force bodies to be active
    u8                              m_Zone;                     // Game zone (used in col detection)
    
    // Rendering components
    anim_group::handle              m_hAnimGroup;               // Animation group handle
    skin_inst                       m_SkinInst;                 // Skinned instance
    smem_matrix_cache               m_MatrixCache;              // Matrix allocation class

            
//==============================================================================
// Friends
//==============================================================================
friend class rigid_body;
friend class physics_mgr;
friend class collider;

};

//==============================================================================
// TYPES
//==============================================================================

typedef linked_list< physics_inst, offsetof( physics_inst, m_ListNode )       > physics_inst_list;

//==============================================================================
// INLINE FUNCTIONS
//==============================================================================

inline
const bbox& physics_inst::GetWorldBBox( void ) const
{
    return m_WorldBBox;
}

//==============================================================================

inline
anim_group::handle& physics_inst::GetAnimGroupHandle( void )
{
    return m_hAnimGroup;
}

//==============================================================================

inline
skin_inst& physics_inst::GetSkinInst( void )
{
    return m_SkinInst;
}

//==============================================================================

inline
const skin_inst& physics_inst::GetSkinInst( void ) const
{
    return m_SkinInst;
}

//==============================================================================

inline
const char* physics_inst::GetAnimName( void ) const
{
    return m_hAnimGroup.GetName();
}

//==============================================================================

inline
const char* physics_inst::GetSkinGeomName( void ) const
{
    return m_SkinInst.GetSkinGeomName();
}

//==============================================================================
// Rigid body functions
//==============================================================================

inline
s32 physics_inst::GetNRigidBodies( void ) const
{
    return m_RigidBodies.GetCount();
}

//==============================================================================

inline
rigid_body& physics_inst::GetRigidBody( s32 iRigidBody )
{
    return m_RigidBodies[ iRigidBody ];
}

//==============================================================================
// Collision functions
//==============================================================================

inline
s32 physics_inst::GetNCollisionShapes( void ) const
{
    return m_CollisionShapes.GetCount();
}

//==============================================================================

inline
collision_shape& physics_inst::GetCollisionShape( s32 Index )
{
    return m_CollisionShapes[ Index ];
}

//==============================================================================

inline
void physics_inst::SetActorCollision( xbool bEnable )
{
    m_bActorCollision = bEnable;
}

//==============================================================================

inline
xbool physics_inst::GetActorCollision( void ) const
{
    return m_bActorCollision;
}

//==============================================================================

inline
void physics_inst::SetWorldCollision( xbool bEnable )
{
    m_bWorldCollision = bEnable;
}

//==============================================================================

inline
xbool physics_inst::GetWorldCollision( void ) const
{
    return m_bWorldCollision;
}

//==============================================================================

inline
void physics_inst::SetInstCollision( xbool bEnable )
{
    m_bInstCollision = bEnable;
}

//==============================================================================

inline
xbool physics_inst::GetInstCollision( void ) const
{
    return m_bInstCollision;
}

//==============================================================================
// Constraint functions
//==============================================================================

inline
s32 physics_inst::GetNBodyBodyConstraints( void ) const
{
    return m_BodyBodyContraints.GetCount();
}

//==============================================================================

inline
constraint& physics_inst::GetBodyBodyConstraint( s32 Index )
{
    return m_BodyBodyContraints[ Index ];
}

//==============================================================================

inline
s32 physics_inst::GetNBodyWorldConstraints( void ) const
{
    return m_BodyWorldContraints.GetCount();
}

//==============================================================================

inline
constraint& physics_inst::GetBodyWorldConstraint( s32 Index )
{
    return m_BodyWorldContraints[ Index ];
}

//==============================================================================

inline
void physics_inst::SetBodyWorldConstraintWorldPos( s32 iConstraint, const vector3& WorldPos )
{
    // Lookup constraint
    constraint& Constraint = GetBodyWorldConstraint( iConstraint) ;
    
    // Update world position
    // (since body1 is the world rigid body which is at the origin (0,0,0) and has zero rotation (0,0,0) 
    //  - we can just set the body position directly)
    Constraint.SetBodyPos( 1, WorldPos );
}

//==============================================================================

inline
void physics_inst::DeleteAllBodyWorldConstraints( void )
{
    // Clear list
    m_BodyWorldContraints.SetCount( 0 );
}

//==============================================================================

inline
void physics_inst::SetConstraintWeight( f32 Weight )
{
    m_ConstraintWeight = Weight;
}

//==============================================================================

inline
f32 physics_inst::GetConstraintWeight( void ) const
{
    return m_ConstraintWeight;
}

//==============================================================================
// Active functions
//==============================================================================

inline
xbool physics_inst::IsActive( void ) const
{
    return m_bInAwakeList;
}

//==============================================================================

inline
void physics_inst::SetActiveWhenVisible( xbool bActive )
{
    m_bActiveWhenVisible = bActive;
}

//==============================================================================

inline
void physics_inst::SetKeepActiveTime( f32 Time )
{
    m_KeepActiveTime = Time;
}    

//==============================================================================
// Zone functions
//==============================================================================

inline
void physics_inst::SetZone( u8 Zone )
{
    m_Zone = Zone;
}

//==============================================================================

inline
u8 physics_inst::GetZone( void ) const
{
    return m_Zone;
}

//==============================================================================


#endif  //#define __PHYSICS_INST_HPP__
