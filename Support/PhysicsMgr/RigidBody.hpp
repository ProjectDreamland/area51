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

#include "Physics.hpp"
#include "LinkedList.hpp"
#include "CollisionShape.hpp"


//==============================================================================
// FORWARD DECLARATIONS
//==============================================================================
class constraint;


//==============================================================================
// CLASSES
//==============================================================================
class rigid_body
{
//==============================================================================
// Defines
//==============================================================================
public:

    // Flags
    enum flags
    {
        FLAG_WORLD_COLLISION = ( 1 << 0 ),   // TRUE if body should collide with world    
        FLAG_IS_ACTIVE       = ( 1 << 1 ),   // TRUE if body is moving    
        FLAG_HAS_COLLIDED    = ( 1 << 2 ),   // TRUE if body has collided
    };

//==============================================================================
// Structures
//==============================================================================
public:

    // State of rigid body
    struct state
    {
        matrix4     m_L2W;
        vector3     m_LinearVelocity;
        vector3     m_AngularVelocity;
    };
    

//==============================================================================
// Functions
//==============================================================================
public:
             rigid_body();
            ~rigid_body();

            // Property functions
            f32             GetMass             ( void ) const X_SECTION(physics);   
            void            SetMass             ( f32 Mass ) X_SECTION(physics);
            void            SetInfiniteMass     ( void ) X_SECTION(physics);
            f32             GetInvMass          ( void ) const X_SECTION(physics);   

            f32             GetElasticity       ( void ) const X_SECTION(physics);
            f32             GetStaticFriction   ( void ) const X_SECTION(physics);
            f32             GetDynamicFriction  ( void ) const X_SECTION(physics);

            void            SetElasticity       ( f32 Elasticity ) X_SECTION(physics);
            void            SetStaticFriction   ( f32 StaticFriction ) X_SECTION(physics);
            void            SetDynamicFriction  ( f32 DynamicFriction ) X_SECTION(physics);

            // Transform functions
            const matrix4&  GetL2W              ( void ) const X_SECTION(physics);
            const matrix4&  GetPrevL2W          ( void ) const X_SECTION(physics);
            matrix4         GetW2L              ( void ) const X_SECTION(physics);
            void            SetPrevL2W          ( const matrix4& L2W ) X_SECTION(physics);
            void            SetL2W              ( const matrix4& L2W ) X_SECTION(physics);
            
            void            SetPosition         ( const vector3& Position ) X_SECTION(physics);
            vector3         GetPosition         ( void ) const X_SECTION(physics);
            
            void            ComputeWorldInvInertia( void ) X_SECTION(physics);

            // Bounding box functions
            void            ComputeWorldBBox    ( void ) X_SECTION(physics);
            const bbox&     GetWorldBBox        ( void ) const X_SECTION(physics);

            // Inertia tensor functions
            void            SetBodyInertia      ( const vector3& I );
            const matrix4&  GetWorldInvInertia  ( void ) const X_SECTION(physics);
            
            // Velocity functions
            void            ClampVelocities     ( void ) X_SECTION(physics);
            void            SetLinearVelocity   ( const vector3& Velocity ) X_SECTION(physics);
            const vector3&  GetLinearVelocity   ( void ) const X_SECTION(physics);
            vector3&        GetLinearVelocity   ( void ) X_SECTION(physics);
            
            void            SetAngularVelocity  ( const vector3& Velocity ) X_SECTION(physics);
            const vector3&  GetAngularVelocity  ( void ) const X_SECTION(physics);
            vector3&        GetAngularVelocity  ( void ) X_SECTION(physics);
            
            void            SetLinearDamping    ( f32 LinearDamping ) X_SECTION(physics);
            void            SetAngularDamping   ( f32 AngularDamping ) X_SECTION(physics);
            
            void            ZeroLinearVelocity  ( void ) X_SECTION(physics);
            void            ZeroAngularVelocity ( void ) X_SECTION(physics);
            void            ZeroVelocity        ( void ) X_SECTION(physics);

            // State backup functions
            void            GetState            ( state& State ) const X_SECTION(physics);
            void            SetPosition         ( const state& State ) X_SECTION(physics);
            void            SetVelocity         ( const state& State ) X_SECTION(physics);
                            
            // Force/torque functions                            
            void            ClearForces         ( void ) X_SECTION(physics);
            void            ComputeForces       ( f32 DeltaTime ) X_SECTION(physics);                

            void            AddWorldForce       ( const vector3& Force ) X_SECTION(physics);
            void            AddWorldForce       ( const vector3& Force,  const vector3& Position ) X_SECTION(physics);
                                                
            void            AddWorldTorque      ( const vector3& Torque ) X_SECTION(physics);
            void            AddWorldTorque      ( const vector3& Torque, const vector3& Position ) X_SECTION(physics);
                            
            void            ApplyWorldImpulse   ( const vector3& Impulse ) X_SECTION(physics);
            void            ApplyWorldImpulse   ( const vector3& Impulse, const vector3& Position ) X_SECTION(physics);
            void            ApplyLocalImpulse   ( const vector3& Impulse, const vector3& Position ) X_SECTION(physics);
                            
            // Integration functions                            
            void            IntegratePosition   ( f32 DeltaTime ) X_SECTION(physics);
            void            IntegrateVelocity   ( f32 DeltaTime ) X_SECTION(physics);
                                    
            // Active functions                                     
            void            UpdateActiveState   ( f32 DeltaTime ) X_SECTION(physics);
            xbool           HasActiveEnergy     ( void ) const X_SECTION(physics);
            xbool           IsActive            ( void ) const X_SECTION(physics);
            f32             GetActiveBlend      ( void ) const X_SECTION(physics);
            void            Deactivate          ( void ) X_SECTION(physics);
            void            Activate            ( void ) X_SECTION(physics);

            // Collision functions
            void             SetCollisionShape  ( collision_shape* pCollisionShape, f32 Mass, f32 InertiaMax );
            collision_shape* GetCollisionShape  ( void ) const;
            void             SetCollisionInfo   ( s32 Group, s32 ID, u32 Mask );
            s32              GetCollisionID     ( void ) const X_SECTION(physics);
            xbool            IsCollisionEnabled ( rigid_body* pBody ) X_SECTION(physics);
            void             SetWorldCollision  ( xbool bEnable ) X_SECTION(physics);
            void             CollisionWakeup    ( void ) X_SECTION(physics);
            
            // Collision impact query functions
            void             ClearCollision     ( void );
            xbool            HasCollided        ( void ) const;
            f32              GetCollisionSpeedSqr( void ) const;

            // Constraint functions
            void            SetPivotConstraint  ( constraint* pPivotConstraint );
            constraint*     GetPivotConstraint  ( void ) const;
            
#ifdef ENABLE_PHYSICS_DEBUG
            // Debug render functions
            void            SetDebugColor       ( xcolor DebugColor );
            xcolor          GetDebugColor       ( void ) const;
            void            DebugRender         ( void );
#endif

//==============================================================================
// Data
//==============================================================================
public:

        // Linked list nodes
        linked_list_node<rigid_body>    m_ActiveListNode;   // Physics mgr active list node

protected:

        // Constant properties
        f32                 m_Mass;                 // Mass
        f32                 m_InvMass;              // 1/Mass
        vector3             m_BodyInvInertia;       // Body space inverse inertia (diagonal of matrix)
        f32				    m_Elasticity;           // Bouncyness
        f32				    m_StaticFriction;       // Friction when not moving
        f32				    m_DynamicFriction;      // Friction when moving
        f32                 m_LinearDamping;        // Linear velocity damping
        f32                 m_AngularDamping;       // Angular velocity damping
        collision_shape*    m_pCollisionShape;      // Pointer to collision shape

        // Status information
mutable u32                 m_Flags;                // General flags
        f32                 m_InactiveTime;         // Time body has had below active energy

        // State variables
mutable matrix4             m_L2W;                  // Local to world transform
mutable bbox                m_WorldBBox;            // World space bounding box

        // Derived quantities (auxiliary variables)
        matrix4             m_WorldInvInertia;      // World space inverse inertia tensor
        vector3             m_LinearVelocity;       // Linear (translation) velocity
        vector3             m_AngularVelocity;      // Angular (rotation) velocity

        // Computed quantities about center of mass
        vector3             m_Force;                // Accumulated force to apply
        vector3             m_Torque;               // Accumulated torque to apply

        // Backup state
        state               m_BackupState;          // Back of world transform and velocity

        // Collision info
        s32                 m_CollisionGroup;       // Group ID (bodies in same instance have same)
        s32                 m_CollisionID;          // Collision index
        u32                 m_CollisionBit;         // Collision bit mask ( 1 << m_CollisionID )
        u32                 m_CollisionMask;        // Collision mask (Mask & Bit) = Collision?
        f32                 m_CollisionSpeedSqr;    // Biggest collision speed squared (if collision happened)
        
        // Constraint info
        constraint*         m_pPivotConstraint;     // Ptr to pivot constraint (or NULL)
        
#ifdef ENABLE_PHYSICS_DEBUG
        // Render info
        xcolor              m_DebugColor;           // Debug render color
#endif

//==============================================================================
// Friends
//==============================================================================
friend class physics_mgr;
friend class physics_inst;
friend class collider;

};

//==============================================================================
// TYPES
//==============================================================================

typedef linked_list< rigid_body, offsetof( rigid_body, m_ActiveListNode ) > rigid_body_active_list;


//==============================================================================
// FUNCTIONS
//==============================================================================

#ifndef ENABLE_PHYSICS_DEBUG

inline
rigid_body::~rigid_body()
{
}

#endif

//==============================================================================
// Property functions
//==============================================================================

inline
f32 rigid_body::GetMass( void ) const
{
    return m_Mass;
}

//==============================================================================

inline
void rigid_body::SetMass( f32 Mass )
{
    ASSERT( Mass > 0.0f );

    // Record mass and inverse mass
    m_Mass    = Mass;
    m_InvMass = 1.0f / Mass;
}

//==============================================================================

inline
void rigid_body::SetInfiniteMass( void )
{
    // Set to non-movable and infinite mass
    m_Mass    = 0.0f;
    m_InvMass = 0.0f;
    m_BodyInvInertia.Zero();
    m_WorldInvInertia.Zero();
}

//==============================================================================

inline
f32 rigid_body::GetInvMass( void ) const
{
    return m_InvMass;
}

//==============================================================================

inline
f32 rigid_body::GetElasticity( void ) const
{
    return m_Elasticity;
}

//==============================================================================

inline
f32 rigid_body::GetStaticFriction( void ) const
{
    return m_StaticFriction;
}

//==============================================================================

inline
void rigid_body::SetElasticity( f32 Elasticity )
{
    m_Elasticity = Elasticity;
}

//==============================================================================

inline
void rigid_body::SetStaticFriction( f32 StaticFriction )
{
    m_StaticFriction = StaticFriction;   
}

//==============================================================================

inline
void rigid_body::SetDynamicFriction( f32 DynamicFriction )
{
    m_DynamicFriction = DynamicFriction;
}

//==============================================================================
// Transform functions
//==============================================================================

inline
const matrix4& rigid_body::GetL2W( void ) const
{
    return m_L2W;
}

//==============================================================================

inline
const matrix4& rigid_body::GetPrevL2W( void ) const
{
    return m_BackupState.m_L2W;
}

//==============================================================================

inline
matrix4 rigid_body::GetW2L( void ) const
{
    return m4_InvertRT( GetL2W() );
}

//==============================================================================

inline
void rigid_body::ComputeWorldBBox( void )
{
    // Grab from collision shape?
    if( m_pCollisionShape )
    {
        // Use world bbox of collision shape
        m_WorldBBox = m_pCollisionShape->ComputeWorldBBox();
    }
    else
    {    
        // Setup default
        m_WorldBBox.Set( GetPosition(), 1.0f );
    }        
}

//==============================================================================

inline
void rigid_body::SetBodyInertia( const vector3& I )
{
    // Set inverse body space inertia
    m_BodyInvInertia.Set( 1.0f / I.GetX(), 
                          1.0f / I.GetY(), 
                          1.0f / I.GetZ() );
}

//==============================================================================

inline
void rigid_body::SetPosition( const vector3& Position )
{
    m_L2W.SetTranslation( Position );
}

//==============================================================================

inline
vector3 rigid_body::GetPosition( void ) const
{
    return m_L2W.GetTranslation();
}

//==============================================================================

inline
void rigid_body::ComputeWorldInvInertia( void )
{
    // Compute orientation and inverse orientation (no translation)
    matrix4 Orient = m_L2W;
    Orient.ClearTranslation();
    matrix4 InvOrient = m4_Transpose( Orient );
    
    // Compute world space inverse as follows (operations read from right->left):
    // m_WorldInvInertia = InvOrient * BodyInvInertia * Orient
    
    // Since the "BodyInvInertia" only has it's diagonal elements set,
    // the computation can be optimized to this:
    Orient.Scale( m_BodyInvInertia );   // ie. BodyInvInertia * Orient
    m_WorldInvInertia = InvOrient * Orient;
}

//==============================================================================
// Bounding box functions
//==============================================================================

inline
const bbox& rigid_body::GetWorldBBox( void ) const
{
    return m_WorldBBox;
}

//==============================================================================

inline
f32 rigid_body::GetDynamicFriction  ( void ) const
{
    return m_DynamicFriction;
}


//==============================================================================
// Inertia tensor functions
//==============================================================================

inline
const matrix4& rigid_body::GetWorldInvInertia ( void ) const
{
    return m_WorldInvInertia;
}

//==============================================================================
// Velocity functions
//==============================================================================

inline
void  rigid_body::SetLinearVelocity( const vector3& Velocity )
{
    m_LinearVelocity = Velocity;
}

//==============================================================================

inline
const vector3& rigid_body::GetLinearVelocity( void ) const
{
    return m_LinearVelocity;
}

//==============================================================================

inline
vector3& rigid_body::GetLinearVelocity( void )
{
    return m_LinearVelocity;
}

//==============================================================================

inline
void  rigid_body::SetAngularVelocity( const vector3& Velocity )
{
    m_AngularVelocity = Velocity;
}

//==============================================================================

inline
const vector3& rigid_body::GetAngularVelocity( void ) const
{
    return m_AngularVelocity;
}

//==============================================================================

inline
vector3& rigid_body::GetAngularVelocity( void )
{
    return m_AngularVelocity;
}

//==============================================================================

inline
void rigid_body::SetLinearDamping( f32 LinearDamping )
{
    m_LinearDamping = LinearDamping;
}

//==============================================================================

inline
void rigid_body::SetAngularDamping( f32 AngularDamping )
{
    m_AngularDamping = AngularDamping;
}

//==============================================================================

inline
void rigid_body::ZeroLinearVelocity( void )
{
    m_LinearVelocity.Zero();
}

//==============================================================================

inline
void rigid_body::ZeroAngularVelocity( void )
{
    m_AngularVelocity.Zero();
}

//==============================================================================

inline
void rigid_body::ZeroVelocity( void )
{
    m_LinearVelocity.Zero();
    m_AngularVelocity.Zero();
}


//==============================================================================
// State backup functions
//==============================================================================

inline
void rigid_body::GetState( rigid_body::state& State ) const
{
    // Store
    State.m_L2W             = GetL2W();
    State.m_LinearVelocity  = m_LinearVelocity;
    State.m_AngularVelocity = m_AngularVelocity;
}

//==============================================================================

inline
void rigid_body::SetPosition( const rigid_body::state& State )
{
    // Restore position
    SetL2W( State.m_L2W );
}

//==============================================================================

inline
void rigid_body::SetVelocity( const rigid_body::state& State )
{
    // Restore
    m_LinearVelocity  = State.m_LinearVelocity;
    m_AngularVelocity = State.m_AngularVelocity;
}

//==============================================================================

inline
void rigid_body::SetPrevL2W( const matrix4& L2W )
{
    m_BackupState.m_L2W = L2W;
}

//==============================================================================
// Force/torque functions                            
//==============================================================================

inline
void rigid_body::ClearForces( void )
{
    m_Force.Zero();
    m_Torque.Zero();
}

//==============================================================================

inline
void rigid_body::AddWorldForce( const vector3& Force )
{
    m_Force  += Force;
}

//==============================================================================

inline
void rigid_body::AddWorldForce( const vector3& Force, const vector3& Position )
{
    m_Force  += Force;
    m_Torque += v3_Cross( Position - GetPosition(),  Force );
}

//==============================================================================

inline
void rigid_body::AddWorldTorque( const vector3& Torque )
{
    m_Torque += Torque;
}

//==============================================================================

inline
void rigid_body::AddWorldTorque( const vector3& Torque, const vector3& Position )
{
    m_Torque += Torque;
    m_Force  += v3_Cross( Position - GetPosition(), Torque);
}

//==============================================================================

inline
void rigid_body::ApplyWorldImpulse( const vector3& Impulse )
{
    m_LinearVelocity += Impulse * m_InvMass;
}

//==============================================================================

inline
void rigid_body::ApplyWorldImpulse( const vector3& Impulse, const vector3& Position )
{
    m_LinearVelocity  += Impulse * m_InvMass;
    m_AngularVelocity += m_WorldInvInertia.RotateVector( v3_Cross( Position - GetPosition(), Impulse ) );
}

//==============================================================================

inline
void rigid_body::ApplyLocalImpulse( const vector3& Impulse, const vector3& Position )
{
#ifdef TARGET_PS2

    // ANGULAR-STEP1:   Cross = v3_Cross( Position, Impulse );
    u128 Cross;
    asm( "vopmula.xyz   acc,    VEC0,   VEC1
          vopmsub.xyz   RES,    VEC1,   VEC0" :
        "=j RES"  (Cross) :
        "j  VEC0" (Position.GetU128()),
        "j  VEC1" (Impulse.GetU128()) );

    // LINEAR-STEP1:    DeltaLinearVelocity = Impulse * m_InvMass;
    u128 DeltaLinearVelocity;
    asm( "qmtc2         FSCL, VSCL"       : "=j VSCL" (DeltaLinearVelocity) : "r FSCL" (m_InvMass) );
    asm( "vmulx.xyzw    VOUT, VIN, VSCLx" : "=j VOUT" (DeltaLinearVelocity) : "j VIN"  (Impulse.GetU128()), "j VSCL" (DeltaLinearVelocity) );
    
    // ANGULAR-STEP2:   DeltaAngularVelocity = m_WorldInvInertia.RotateVector( Cross )
    u128 DeltaAngularVelocity;
    asm( "vmulaz.xyzw acc,   COL2, VINz
          vmadday.xyzw acc,  COL1, VINy
          vmaddx.xyzw  VOUT, COL0, VINx" :
        "=j VOUT" ( DeltaAngularVelocity ) :
        "j  COL0" ( m_WorldInvInertia.GetCol0_U128() ),
        "j  COL1" ( m_WorldInvInertia.GetCol1_U128() ),
        "j  COL2" ( m_WorldInvInertia.GetCol2_U128() ),
        "j  VIN"  ( Cross ) );

    // LINEAR-STEP2:    m_LinearVelocity += DeltaLinearVelocity;
    u128 LinearVelocity;
    asm( "vadd.xyzw     VOUT, VEC0, VEC1" : "=j VOUT" (LinearVelocity) : "j VEC0" (m_LinearVelocity.GetU128()), "j VEC1" (DeltaLinearVelocity) );

    // ANGULAR-STEP3:   m_AngularVelocity += DeltaAngularVelocity
    u128 AngularVelocity;
    asm( "vadd.xyzw     VOUT, VEC0, VEC1" : "=j VOUT" (AngularVelocity) : "j VEC0" (m_AngularVelocity.GetU128()), "j VEC1" (DeltaAngularVelocity) );
    
    // LINEAR-STEP3    
    m_LinearVelocity.Set( LinearVelocity );
    
    // ANGULAR-STEP3    
    m_AngularVelocity.Set( AngularVelocity );

#else    
    
    m_LinearVelocity  += Impulse * m_InvMass;
    m_AngularVelocity += m_WorldInvInertia.RotateVector( v3_Cross( Position, Impulse ) );

#endif    
}

//==============================================================================
// Active functions
//==============================================================================

inline
xbool rigid_body::IsActive( void ) const
{
    return ( m_Flags & FLAG_IS_ACTIVE ) != 0;
}


//==============================================================================
// Collision functions
//==============================================================================

inline
collision_shape* rigid_body::GetCollisionShape( void ) const
{
    return m_pCollisionShape;
}

//==============================================================================

inline
void rigid_body::SetCollisionInfo( s32 Group, s32 ID, u32 Mask )
{
    m_CollisionGroup = Group;
    m_CollisionID    = ID;
    m_CollisionBit   = 1 << ID;
    m_CollisionMask  = Mask;
}

//==============================================================================

inline
s32 rigid_body::GetCollisionID( void ) const
{
    return m_CollisionID;
}

//==============================================================================

inline
xbool rigid_body::IsCollisionEnabled( rigid_body* pBody )
{
    // Should only call for rigid bodies in the same group
    ASSERT( this != pBody );
    ASSERT( m_CollisionGroup == pBody->m_CollisionGroup );

    // Check masks        
    if( m_CollisionMask & pBody->m_CollisionBit )
    {
        ASSERT( pBody->m_CollisionMask & m_CollisionBit );
        return TRUE;
    }        
    else
    {
        ASSERT(!( pBody->m_CollisionMask & m_CollisionBit ));
        return FALSE;
    }        
}

//==============================================================================

inline
void rigid_body::SetWorldCollision( xbool bEnable )
{
    if( bEnable )
        m_Flags |= FLAG_WORLD_COLLISION;
    else        
        m_Flags &= ~FLAG_WORLD_COLLISION;
}

//==============================================================================

inline
void rigid_body::CollisionWakeup( void )
{
    // Sleeping?
    if( ( m_Flags & rigid_body::FLAG_IS_ACTIVE ) == 0 )
    {
        // Grab state ready for physics solving
        GetState( m_BackupState );

        // Make as active (leave velocities so it can be put back to sleep ASAP)
        m_Flags |= rigid_body::FLAG_IS_ACTIVE;
    }
}

//==============================================================================

inline
void  rigid_body::ClearCollision( void )
{
    m_Flags &= ~FLAG_HAS_COLLIDED;
    m_CollisionSpeedSqr = 0.0f;
}

//==============================================================================

inline
xbool rigid_body::HasCollided( void ) const
{
    return ( m_Flags & ( FLAG_IS_ACTIVE | FLAG_HAS_COLLIDED ) ) == ( FLAG_IS_ACTIVE | FLAG_HAS_COLLIDED );
}

//==============================================================================

inline
f32 rigid_body::GetCollisionSpeedSqr( void ) const
{
    return m_CollisionSpeedSqr;
}

//==============================================================================
// Constraint functions
//==============================================================================

inline
void rigid_body::SetPivotConstraint( constraint* pPivotConstraint )
{
    m_pPivotConstraint = pPivotConstraint;
}

//==============================================================================

inline
constraint* rigid_body::GetPivotConstraint( void ) const
{
    return m_pPivotConstraint;
}

//==============================================================================
// Render functions
//==============================================================================

#ifdef ENABLE_PHYSICS_DEBUG

inline
void rigid_body::SetDebugColor( xcolor DebugColor )
{
    m_DebugColor = DebugColor;
}

//==============================================================================

inline
xcolor rigid_body::GetDebugColor( void ) const
{
    return m_DebugColor;
}

#endif  //#ifdef ENABLE_PHYSICS_DEBUG

//==============================================================================



#endif  //#define __RIGID_BODY_HPP__
