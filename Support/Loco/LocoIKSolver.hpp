//=========================================================================
//
//  LocoIKSolver.hpp
//
//=========================================================================
#ifndef __LOCO_IK_SOLVER_HPP__
#define __LOCO_IK_SOLVER_HPP__

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_math.hpp"
#include "Animation\AnimData.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Render\Geom.hpp"

//=========================================================================
// DEFINES
//=========================================================================

//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================

//=========================================================================
// CLASS LOCO IK SOLVER - Used to re-position bones after animation pipeline
//=========================================================================

class loco_ik_solver
{
//=========================================================================
// DEFINES
//=========================================================================
public:

//=========================================================================
// STRUCTURES
//=========================================================================
public:

    // Bone info
    struct bone_mapping
    {
        // Data
        s32         m_iBone;        // Index of geometry bone
        matrix4     m_B2S;          // Bone space to solver space mapping
        matrix4     m_S2B;          // Solver to bone space mapping
    };
    
    // Body to body constraint
    struct constraint
    {
        // Data
        s32         m_iBone0;       // Index of bone0
        s32         m_iBone1;       // Index of bone1
        vector3     m_LocalPos0;    // Position in local space of bone0
        vector3     m_LocalPos1;    // Position in local space of bone1
        f32         m_MinDist;      // Minimum distance points should be
        f32         m_MaxDist;      // Maximum distance points should be
        f32         m_MassRatio0;   // Mass ratio of bone0
        f32         m_MassRatio1;   // Mass ratio of bone 1
        f32         m_Inertia0;     // Rotation inertia of bone0
        f32         m_Inertia1;     // Rotation inertia of bone1
        
        // Functions
        void SolveBone(       matrix4& M, 
                        const vector3& Pos, 
                        const vector3& Dir, 
                              f32      Amount,
                              f32      MassRatio,
                              f32      Inertia );
                    
        void Solve( matrix4* pMatrices, s32 nActiveBones, f32 Weight );
    };

//=========================================================================
// PUBLIC FUNCTIONS
//=========================================================================
public:
                loco_ik_solver();
virtual        ~loco_ik_solver();

        // Initializes solver
        void    Init    ( bone_mapping* pBoneMappings, s32 nBoneMappings,
                          constraint*   pConstraints,  s32 nConstraints,
                          s32           nIterations                       );

private:
        // Solves all constraints
        void    SolveConstraints( matrix4* pMatrices, s32 nActiveBones );
        
public:                          
        // Applies all constraints and solves matrices
        void    Solve           ( matrix4* pMatrices, s32 nActiveBones );
                              
        // Sets the IK solving weight amount
        void    SetWeight       ( f32 Weight );
        
        // Retrieves the current weight
        f32     GetWeight       ( void ) const;
        
                                                  
//=========================================================================
// PRIVATE DATA
//=========================================================================
protected:

        bone_mapping*   m_pBoneMappings;    // Pointer to list of bone mappings
        s32             m_nBoneMappings;    // # if bones to solve
        
        constraint*     m_pConstraints;     // Pointer to list of constraints
        s32             m_nConstraints;     // # of constraints        

        s32             m_nIterations;      // # of iterations to do when solving
        f32             m_Weight;           // Weight of IK. 0 = none, 1= full

//=========================================================================
// FRIENDS
//=========================================================================
friend class loco_char_anim_player;
friend class loco;
};

//=========================================================================
// INLINE FUNCTIONS
//=========================================================================

// Sets the IK solving weight amount
inline
void loco_ik_solver::SetWeight( f32 Weight )
{
    ASSERT( Weight >= 0.0f );
    ASSERT( Weight <= 1.0f );
    
    m_Weight = Weight;
}

//=========================================================================

// Retrieves the current weight
inline
f32 loco_ik_solver::GetWeight( void ) const
{
    return m_Weight;
}


//=========================================================================
#endif // END __LOCO_IK_SOLVER_HPP__
//=========================================================================
