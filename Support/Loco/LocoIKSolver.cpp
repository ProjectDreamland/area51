//=========================================================================
//
//  LocoIKSolver.cpp
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "LocoIKSolver.hpp"


//=========================================================================
// CONSTRAINT FUNCTIONS
//=========================================================================

// Applies corrective force to bone
void loco_ik_solver::constraint::SolveBone(       matrix4& M, 
                                            const vector3& Pos, 
                                            const vector3& Dir, 
                                                  f32      Amount,
                                                  f32      MassRatio,
                                                  f32      Inertia )
{
    // Apply to rotation?
    if( Inertia != 0.0f )
    {
        // Compute world space axis to rotate around for correction
        vector3 T      = M.GetTranslation();
        vector3 RelPos = Pos - T;
        vector3 Axis   = v3_Cross( RelPos, Dir );
        if( Axis.SafeNormalize() )
        {
            // Rotate around pivot
            matrix4 DeltaRot;
            DeltaRot.Setup( Axis, Amount * Inertia );
            M.SetTranslation( vector3(0,0,0) );
            M = DeltaRot * M;
            M.SetTranslation( T );
        }
    }

    // Apply to translation?
    if( MassRatio != 0.0f )
        M.Translate( Dir * Amount * MassRatio );
}

//=========================================================================

void loco_ik_solver::constraint::Solve( matrix4* pMatrices, s32 nActiveBones, f32 Weight )
{
    ASSERT( pMatrices );
    ASSERT( m_iBone0 >= 0 );
    ASSERT( m_iBone1 >= 0 );

    // Bones not present?
    if( m_iBone0 >= nActiveBones )
        return;
    if( m_iBone1 >= nActiveBones )
        return;

    // Lookup bone matrices
    matrix4& M0 = pMatrices[m_iBone0];
    matrix4& M1 = pMatrices[m_iBone1];
    
    // Compute world space positions
    vector3 WorldPos0 = M0 * m_LocalPos0;
    vector3 WorldPos1 = M1 * m_LocalPos1;

    // Compute distance squared between points
    vector3 Dir     = WorldPos1 - WorldPos0;
    f32     DistSqr = Dir.LengthSquared();

    // Already satisfied?
    if( ( DistSqr >= x_sqr( m_MinDist ) ) && ( DistSqr <= x_sqr( m_MaxDist ) ) )
        return;

    // Compute distance between points
    f32 Dist = 0.0f;
    if( DistSqr > 0.00001f )
        Dist = x_sqrt( DistSqr );    
    
    // Normalize direction
    if( Dist > 0.00001f )
        Dir *= 1.0f / Dist;

    // Compute deviation distance from constraint
    if( Dist < m_MinDist )
        Dist -= m_MinDist;
    else if( Dist > m_MaxDist )
        Dist -= m_MaxDist;

    // Take solver weight into account
    Dist *= Weight;
    
    // Solve constraint on each bone matrix
    vector3 MidPos = ( WorldPos0 + WorldPos1 ) * 0.5f;
    SolveBone( M0, MidPos,  Dir, Dist, m_MassRatio0, m_Inertia0 );
    SolveBone( M1, MidPos, -Dir, Dist, m_MassRatio1, m_Inertia1 );
}

//=========================================================================
// SOLVER FUNCTIONS
//=========================================================================

loco_ik_solver::loco_ik_solver() :
    m_pBoneMappings ( NULL  ),  // Pointer to list of bone mappings
    m_nBoneMappings ( 0     ),  // # if bones to solve

    m_pConstraints  ( NULL  ),  // Pointer to list of constraints
    m_nConstraints  ( 0     ),  // # of constraints        

    m_nIterations   ( 2     ),  // # of iterations to do when solving
    m_Weight        ( 1.0f  )   // Weight of IK. 0 = none, 1= full
{
}

//=========================================================================

loco_ik_solver::~loco_ik_solver()
{
}

//=========================================================================

// Initializes solver
void loco_ik_solver::Init( bone_mapping* pBoneMappings, s32 nBoneMappings,
                           constraint*   pConstraints,  s32 nConstraints,
                           s32           nIterations                       )
{
    // This solver is pretty useless if we have nothing to solve!
    ASSERT( pConstraints );
    ASSERT( nConstraints > 0 );
    
    // Keep the info
    m_pBoneMappings = pBoneMappings;
    m_nBoneMappings = nBoneMappings;
    m_pConstraints  = pConstraints;
    m_nConstraints  = nConstraints;
    m_nIterations   = nIterations;
}

//=========================================================================

// Solves all constraints
void loco_ik_solver::SolveConstraints( matrix4* pMatrices, s32 nActiveBones )
{
    // Loop over all constraints
    for( s32 i = 0; i < m_nConstraints; i++ )
        m_pConstraints[i].Solve( pMatrices, nActiveBones, m_Weight );
}
    
//=========================================================================

// Applies all constraints and solves matrices
void loco_ik_solver::Solve( matrix4* pMatrices, s32 nActiveBones )
{
    s32 i;
    
    // Nothing to do?
    if( m_Weight == 0.0f )
        return;
        
    // Step1: Convert from bone space to solver space
    for( i = 0; i < m_nBoneMappings; i++ )
    {
        // Lookup mapping info
        bone_mapping& Mapping    = m_pBoneMappings[i];
        if( Mapping.m_iBone >= nActiveBones )
            continue;

        // Convert bone space to solver space
        matrix4& BoneMatrix = pMatrices[ Mapping.m_iBone ];
        BoneMatrix = BoneMatrix * Mapping.m_B2S; 
    }

    // Step2: Iterate over and solve constraints
    for( i = 0; i < m_nIterations; i++ )
        SolveConstraints( pMatrices, nActiveBones );
    
    // Step3: Finally convert from solver space to bone space
    for( i = 0; i < m_nBoneMappings; i++ )
    {
        // Lookup mapping info
        bone_mapping& Mapping = m_pBoneMappings[i];
        if( Mapping.m_iBone >= nActiveBones )
            continue;

        // Convert solver space to bone space
        matrix4& BoneMatrix = pMatrices[ Mapping.m_iBone ];
        BoneMatrix = BoneMatrix * Mapping.m_S2B; 
    }
}

//=========================================================================

