//==============================================================================
//
//  Constraint.hpp
//
//==============================================================================

#ifndef __CONSTRAINT_HPP__
#define __CONSTRAINT_HPP__

//==============================================================================
// INCLUDES
//==============================================================================

#include "Physics.hpp"
#include "RigidBody.hpp"
#include "LinkedList.hpp"

//==============================================================================
// FORWARD DECLARATIONS
//==============================================================================
class constraint;


//==============================================================================
// ACTIVE CONSTRAINT STRUCT
//==============================================================================
class active_constraint
{
    //==============================================================================
    // Data
    //==============================================================================
public:

    // Constraint vars
    constraint* m_pOwner;               // Owner constraint

    // Linked list nodes
    linked_list_node<active_constraint> m_SolveListNode;    // In solve list

    // Derived quantities
    xbool       m_bSatisfied;           // Does this constraint need
    vector3     m_RelMidPos0;           // Mid points relative to bodies space
    vector3     m_RelMidPos1;           // Mid points relative to bodies space
    vector3     m_RelPos0;              // Points relative to bodies space
    vector3     m_RelPos1;              // Points relative to bodies space
    vector3     m_CorrectionVel;        // Velocity correction based on distance
    f32         m_Weight;               // Weight of constraint ( 0 = no effect, 1 = full effect )
};


//==============================================================================
// TYPES
//==============================================================================

typedef linked_list< active_constraint, offsetof( active_constraint, m_SolveListNode ) >  constraint_solve_list;



//==============================================================================
// CONSTRAINT CLASS
//==============================================================================
class constraint
{
//==============================================================================
// Defines
//==============================================================================
public:
    
    // Logic flags
    enum flags
    {
        FLAG_BLEND_IN = ( 1 << 0 ),    // Should constraint be blended in?
    };
    
//==============================================================================
// Functions
//==============================================================================
public:
             constraint();
            ~constraint();

    // Query functions
            rigid_body* GetRigidBody ( s32 Index ) const;
    const   vector3&    GetBodyPos   ( s32 Index ) const;
            vector3&    GetBodyPos   ( s32 Index );
            void        SetBodyPos   ( s32 Index, const vector3& BodyPos );
            
            vector3     GetWorldPos  ( s32 Index ) const;
            void        SetWorldPos  ( s32 Index, const vector3& WorldPos );

            f32         GetMaxDist   ( void ) const;
            u32         GetFlags     ( void ) const;

    // Initialization with world position
            void        Init        ( rigid_body*      pBody0,
                                      rigid_body*      pBody1,
                                      const vector3&   WorldPos,
                                      f32              MaxDist,
                                      u32              Flags,
                                      xcolor           DebugColor ) X_SECTION(physics);

    // Initialization with local position for each body
            void        Init        ( rigid_body*      pBody0,
                                      rigid_body*      pBody1,
                                      const vector3&   Body0Pos,
                                      const vector3&   Body1Pos,
                                      f32              MaxDist,
                                      u32              Flags,
                                      xcolor           DebugColor ) X_SECTION(physics);

    // Logic functions
            xbool       IsActive        ( void ) const;
            xbool       PreApply        ( f32 DeltaTime, active_constraint& Active ) X_SECTION(physics);         
            xbool       Apply           ( active_constraint& Active ) X_SECTION(physics);         

#ifdef ENABLE_PHYSICS_DEBUG
            // Render functions
            xcolor      GetDebugColor   ( void ) const;
            void        DebugRender     ( void ) X_SECTION(physics);
#endif    

//==============================================================================
// Data
//==============================================================================

protected:

    // Body space positions and max world space distance allowed
    vector3         m_BodyPos0;             // Points in body0 space
    vector3         m_BodyPos1;             // Points in body1 space
    u32             m_Flags;                // Logic flags
    f32             m_MaxDist;              // Max distance allowed between points

    // Rigid bodies
    rigid_body*     m_pBody0;               // Body0 to constrain
    rigid_body*     m_pBody1;               // Body1 to constrain

    // Debug
#ifdef ENABLE_PHYSICS_DEBUG
    xcolor          m_DebugColor;   // Color of constraint for debugging
#endif    


//==============================================================================
// Friends
//==============================================================================
friend class physics_mgr;
};


//==============================================================================
// FUNCTIONS
//==============================================================================

#ifndef ENABLE_PHYSICS_DEBUG

inline
constraint::constraint() :
    m_BodyPos0( 0.0f, 0.0f, 0.0f ),
    m_BodyPos1( 0.0f, 0.0f, 0.0f ),
    m_MaxDist ( 0.0f ),
    m_pBody0( NULL ),
    m_pBody1( NULL )
{
}

//==============================================================================

inline
constraint::~constraint()
{
}

#endif

//==============================================================================

#ifdef ENABLE_PHYSICS_DEBUG

inline
xcolor constraint::GetDebugColor( void ) const
{ 
    return m_DebugColor;
}

#endif

//==============================================================================

inline
rigid_body* constraint::GetRigidBody( s32 Index ) const
{
    ASSERT( Index >= 0 );
    ASSERT( Index < 2 );
    return (&m_pBody0)[Index];
}

//==============================================================================

inline
const vector3& constraint::GetBodyPos( s32 Index ) const
{
    ASSERT( Index >= 0 );
    ASSERT( Index < 2 );
    return (&m_BodyPos0)[Index];
}

//==============================================================================

inline
vector3& constraint::GetBodyPos( s32 Index )
{
    ASSERT( Index >= 0 );
    ASSERT( Index < 2 );
    return (&m_BodyPos0)[Index];
}

//==============================================================================

inline
void constraint::SetBodyPos( s32 Index, const vector3& BodyPos )
{
    GetBodyPos( Index ) = BodyPos;
}

//==============================================================================

inline
vector3 constraint::GetWorldPos( s32 Index ) const
{
    // Lookup rigid body
    rigid_body* pBody = GetRigidBody( Index );
    ASSERT( pBody );
    
    // Lookup body pos
    const vector3& BodyPos = GetBodyPos( Index );
    
    // Convert to world space
    return pBody->GetL2W() * BodyPos;
}

//==============================================================================

inline
void constraint::SetWorldPos( s32 Index, const vector3& WorldPos )
{
    // Lookup rigid body
    rigid_body* pBody = GetRigidBody( Index );
    ASSERT( pBody );

    // Convert to body space
    SetBodyPos( Index, pBody->GetW2L() * WorldPos );
}

//==============================================================================

inline
f32 constraint::GetMaxDist( void ) const
{ 
    return m_MaxDist;
}

//==============================================================================

inline
u32 constraint::GetFlags( void ) const
{
    return m_Flags;
}


//==============================================================================
// Logic functions
//==============================================================================

inline
xbool constraint::IsActive( void ) const
{
    ASSERT( m_pBody0 );
    ASSERT( m_pBody1 );
    
    // Body0 active?
    if( m_pBody0->IsActive() )
        return TRUE;
        
    // Body1 active?
    if( m_pBody1->IsActive() )
        return TRUE;
        
    // Both inactive so constraint can be inactive
    return FALSE;
}

//==============================================================================

#endif  //#define __CONSTRAINT_HPP__
