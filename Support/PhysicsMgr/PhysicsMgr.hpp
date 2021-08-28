//==============================================================================
//
//  PhysicsMgr.hpp
//
//==============================================================================

#ifndef __PHYSICS_MGR_HPP__
#define __PHYSICS_MGR_HPP__

//==============================================================================
// INCLUDES
//==============================================================================

#include "Physics.hpp"
#include "PhysicsInst.hpp"

//==============================================================================
// FORWARD DECLARATIONS
//==============================================================================
class actor;


//==============================================================================
// CLASSES
//==============================================================================

class physics_mgr
{
//==============================================================================
// Defines
//==============================================================================
public:


//==============================================================================
// Structures
//==============================================================================
public:

    struct collision
    {
        // Reported values
        vector3             m_Position;             // World position of collision
        vector3             m_Normal;               // Normal of collision into shape0
        f32                 m_Depth;                // Penetration depth of collision

        rigid_body*         m_pBody0;               // Rigid body0 or NULL
        rigid_body*         m_pBody1;               // Rigid body1 or NULL
        actor*              m_pActor1;              // Actor that body1 is attached to
        
        f32                 m_Elasticity;           // Bouncy-ness of collision
        f32                 m_StaticFriction;       // Friction when not moving
        f32                 m_DynamicFriction;      // Friction when moving
        
        // Computed values for each collision solve pass
        f32                 m_Denominator;          // Impulse denominator
        vector3             m_R0;                   // Position of collision relative to shape0
        vector3             m_R1;                   // Position of collision relative to shape1
        vector3             m_PenetrationExtra;     // Penetration penalty velocity
    };

//==============================================================================
// Functions
//==============================================================================
public:
             physics_mgr();
            ~physics_mgr();

            // Initialization
            void        Init                ( void );
            void        ClearDeltaTime      ( void );
            void        Kill                ( void );

            // Physics instance list management functions
            void        AddInstance             ( physics_inst* pInstance );
            void        RemoveInstance          ( physics_inst* pInstance );
            void        WakeupInstance          ( physics_inst* pInstance );
            void        PutToSleepInstance      ( physics_inst* pInstance );
            void        CollisionWakeupInstance   ( physics_inst* pInstance );
            s32         GetAwakeInstanceCount   ( void ) const;

            // Collision functions
            s32         GetNextCollisionGroup   ( void );
            collision&  GetCollision            ( s32 Index ) X_SECTION(physics);
            s32         GetNCollisions          ( void ) const X_SECTION(physics);

            s32         AddCollision        (       rigid_body* pBody0,
                                                    rigid_body* pBody1,
                                              const vector3&    Pos,
                                              const vector3&    Dir, 
                                              const f32&        Dist ) X_SECTION(physics);
           
            void        DetectWorldCollisions       ( physics_inst* pInst ) X_SECTION(physics);
            void        DetectSelfCollisions        ( physics_inst* pInst ) X_SECTION(physics);
            void        DetectInstInstCollisions    ( physics_inst* pInst ) X_SECTION(physics);
            void        DetectInstActorCollisions   ( physics_inst* pInst ) X_SECTION(physics);
            void        DetectCollisions            ( void ) X_SECTION(physics);
            
            void        PreApplyCollisions  ( f32 DeltaTime ) X_SECTION(physics);
            s32         SolveCollision      ( collision& Collision ) X_SECTION(physics);
                                              
            void        PreApplyConstraints (  f32 DeltaTime ) X_SECTION(physics);
            void        SolveCollisions     ( f32 DeltaTime, s32 nIterations ) X_SECTION(physics);
            void        SolveContacts       ( f32 DeltaTime, s32 nIterations ) X_SECTION(physics);
            void        ShockPropagation    ( void ) X_SECTION(physics);
                       
            // Logic functions
            void        PutInstancesToSleep             ( f32 DeltaTime ) X_SECTION(physics);
            void        BuildActiveBodyList             ( void ) X_SECTION(physics);
            void        BuildActiveBodyAndConstraintList( void ) X_SECTION(physics);
            void        Step                ( f32 DeltaTime ) X_SECTION(physics);
            void        Advance             ( f32 DeltaTime ) X_SECTION(physics);
            
#ifdef ENABLE_PHYSICS_DEBUG
            // Render functions
            void        DebugRender             ( void );
            void        DebugFindClosestInstanceInView( const view*         pView,
                                                        physics_inst_list&  InstList,
                                                        f32&                BestDist,
                                                        physics_inst*&      pBestInst );
            void        DebugRenderInstances    ( xbool bRenderConstraints = FALSE );
            void        DebugRenderCollisions   ( void );
            void        DebugText               ( s32 X, s32 Y, xcolor Color, const char* pFormatStr, ... );
            void        DebugShowStats          ( void );
#endif


//==============================================================================
// Structures
//==============================================================================
public:

    // Settings
    struct settings
    {
        f32     m_MaxTimeStep;                  // Maximum time step of simulation
        s32     m_nMaxTimeSteps;                // Maximum # of steps to take
        vector3 m_Gravity;                      // World gravity
        
        s32     m_nMaxCollisions;               // Maximum # of collisions
        s32     m_nCollisionIterations;         // # of iterations to solve collision
        f32     m_CollisionHitBackoffDist;      // Dist to backoff when a collision occurs
        xbool   m_bInstInstCollision;           // Collide physics instance with others?
        xbool   m_bSelfCollision;               // Collide bodies within physics instance?
        f32     m_PenetrationFix;               // Penetration fix scalar
        f32     m_MaxPenetrationFix;            // Max amount of penetration fix
        
        xbool   m_bSolveContacts;               // Solve contacts?
        s32     m_nContactIterations;           // # of iterations to solve contact
        
        xbool   m_bShock;                       // Do shock propagation?
        
        s32     m_nMaxActiveConstraints;        // Maximum # of active constraints
        f32     m_ConstraintFix;                // Constraint fix scalar
        f32     m_MaxConstraintFix;             // Max amount of constraint
        
        f32     m_ActiveLinearSpeed;            // Translation speed at which body is considered active
        f32     m_ActiveAngularSpeed;           // Rotation speed at which body is considered active
        xbool   m_bDeactivate;                  // Allow rigid bodies to deactivate
        f32     m_DeactivateStartTime;          // Time to start blending to deactivate
        f32     m_DeactivateEndTime;            // Time before being fully deactivate
        
        f32     m_MaxLinearVel;                 // Max linear velocity
        f32     m_MaxAngularVel;                // Max angular velocity
        f32     m_MaxForce;                     // Max allowed force
        
        s32     m_nRenderIterations;            // # of render jitter fix iterations
        
        xbool   m_bShowStats;                   // Show stats info?
        xbool   m_bDebugRender;                 // Render debug info?
        xbool   m_bUsePolycache;                // Use world collision or ground plane?
    };
    
#ifdef ENABLE_PHYSICS_DEBUG
    
    // Profile info
    struct profile
    {
        // Stats
        s32     m_nActiveInsts;
        s32     m_nActiveBodies;
        s32     m_nActiveSpheres;
        s32     m_nActiveConstraints;
        s32     m_nCollisions;
        s32     m_nPrepConstraints;
        s32     m_nSolveConstraints;
        s32     m_nBodyBodyCollTests;
        s32     m_nSphereSphereCollTests;
        s32     m_nCapsuleCapsuleCollTests;
        s32     m_nSphereCapsuleCollTests;
        s32     m_nClusters;
        s32     m_nNGons;
        s32     m_nSphereNGonCollTests;
        s32     m_nSphereNGonIntersecTests;
        f32     m_MaxLinearVel;
        f32     m_MaxAngularVel;
        f32     m_MaxForce;
        s32     m_StaticMemoryUsed;
        s32     m_DynamicMemoryUsed;
        
        // Timers
        xtimer  m_Advance;  
        xtimer  m_BuildLists;
        xtimer  m_PredictNew;
        xtimer  m_DetectCollision;
        xtimer  m_Restore;
        xtimer  m_SolveCollisions;
        xtimer  m_IntegrateVel;
        xtimer  m_SolveContacts;
        xtimer  m_PrepConstraints;
        xtimer  m_SolveConstraints;
        xtimer  m_ShockPropagation;
        xtimer  m_IntegratePos;
        xtimer  m_Render;
    };

#endif

//==============================================================================
// Data
//==============================================================================
public:

#ifdef ENABLE_PHYSICS_DEBUG
    // Profiling
    profile                     m_Profile;              // Profile info
#endif
    
    // Settings
    settings                    m_Settings;             // Settings
    
private:    
    
    // Physics instance list
    physics_inst_list           m_AwakeInstances;           // List of active instances
    physics_inst_list           m_SleepingInstances;        // List of inactive instances
    physics_inst_list           m_CollisionWakeupInstances; // List of instances woken up inside collision loop
    
    // Rigid body lists
    rigid_body_active_list      m_ActiveBodies;         // List of active bodies

    // Constraint lists
    xarray<active_constraint>   m_ActiveConstraints;    // List of active constraints
    constraint_solve_list       m_SolveConstraints;     // List of constraints to solve

    // Collision
    s32                         m_NextCollisionGroup;   // Next collision group to allocate
    xarray<collision>           m_Collisions;           // List of collisions

    // Logic
    f32                         m_DeltaTime;            // Current delta time
};

//==============================================================================

inline
void physics_mgr::ClearDeltaTime( void )
{
    m_DeltaTime = 0.0f;
}

//==============================================================================
// Physics instance list management functions
//==============================================================================

inline
s32 physics_mgr::GetAwakeInstanceCount( void ) const
{
    return m_AwakeInstances.GetCount();
}


//==============================================================================
// Collision functions
//==============================================================================

inline
s32 physics_mgr::GetNextCollisionGroup( void )
{
    return m_NextCollisionGroup++;
}

//==============================================================================

inline
physics_mgr::collision& physics_mgr::GetCollision( s32 Index )
{
    return m_Collisions[Index];
}

//==============================================================================

inline
s32 physics_mgr::GetNCollisions( void ) const
{
    return m_Collisions.GetCount();
}

//==============================================================================
// DATA
//==============================================================================

extern collision_shape   g_WorldColl;           // World collision shape
extern rigid_body        g_WorldBody;           // World rigid body

extern collision_shape   g_ActorColl;           // Actor collision shape
extern rigid_body        g_ActorBody;           // Actor rigid body
extern actor*            g_pCollisionActor;     // Current actor being tested against

extern physics_mgr      g_PhysicsMgr;           // Global physics mgr


//==============================================================================

#endif  //#define __PHYSICS_MGR_HPP__
