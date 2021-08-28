//==============================================================================
//
//  MutantTankRagdoll.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "MutantTankRagdoll.hpp"


//==============================================================================
// STORAGE
//==============================================================================

// Storage for the mutant3 ragdoll definition
// (rules and geom bones are filled in at runtime)
static dist_rule   MutantTankDistRules[ragdoll::MAX_DIST_RULES] PS2_ALIGNMENT(64);
static geom_bone   MutantTankGeomBones[ragdoll::MAX_GEOM_BONES] PS2_ALIGNMENT(64);


//==============================================================================
// MAX EXPORT DATA
//==============================================================================
particle_def MutantTankParticleDefs[] =
{
    { "Particle R Foot",
      0.0f, 0.0f, 252.0f,
      -19.5888f, -3.15553f, 13.0808f,
      1.0f, 0.9f,
      true},

    { "Particle R Knee",
      0.0f, 0.0f, 252.0f,
      -19.5888f, -19.2116f, 117.351f,
      1.0f, 1.0f,
      true},

    { "Particle L Foot",
      0.0f, 0.0f, 252.0f,
      19.5888f, -3.15553f, 13.0808f,
      1.0f, 0.9f,
      true},

    { "Particle L Knee",
      0.0f, 0.0f, 252.0f,
      19.5888f, -19.2116f, 117.351f,
      1.0f, 1.0f,
      true},

    { "Particle R Hip",
      0.0f, 0.0f, 252.0f,
      -19.5888f, -10.9204f, 192.755f,
      1.0f, 1.0f,
      true},

    { "Particle L Hip",
      0.0f, 0.0f, 252.0f,
      19.5888f, -10.9204f, 192.755f,
      1.0f, 1.0f,
      true},

    { "Particle R Torso",
      0.0f, 0.0f, 252.0f,
      -23.0542f, -13.2098f, 240.358f,
      1.0f, 1.1f,
      true},

    { "Particle L Torso",
      0.0f, 0.0f, 252.0f,
      21.8838f, -13.2098f, 240.358f,
      1.0f, 1.1f,
      true},

    { "Particle Neck",
      0.0f, 0.0f, 252.0f,
      2.45124e-006f, -42.826f, 298.177f,
      1.0f, 1.0f,
      true},

    { "Particle R Shoulder",
      0.0f, 0.0f, 252.0f,
      -41.836f, -23.739f, 284.808f,
      1.0f, 1.2f,
      true},

    { "Particle L Shoulder",
      0.0f, 0.0f, 252.0f,
      41.836f, -23.739f, 284.808f,
      1.0f, 1.2f,
      true},

    { "Particle R Elbow",
      0.0f, 0.0f, 252.0f,
      -106.578f, -22.5531f, 284.802f,
      1.0f, 1.0f,
      true},

    { "Particle L Elbow",
      0.0f, 0.0f, 252.0f,
      106.578f, -22.5707f, 284.804f,
      1.0f, 1.0f,
      true},

    { "Particle Head",
      0.0f, 0.0f, 252.0f,
      -0.0862264f, -85.7392f, 295.738f,
      1.0f, 1.1f,
      true},

    { "Particle L Wrist",
      0.0f, 0.0f, 252.0f,
      162.847f, -29.2575f, 284.796f,
      1.0f, 0.9f,
      true},

    { "Particle R Wrist",
      0.0f, 0.0f, 252.0f,
      -162.847f, -29.2575f, 284.796f,
      1.0f, 0.9f,
      true},

} ;
constraint_def MutantTankConstraintDefs[] =
{
    { "Constraint L Calf",
      252.0f, 0.0f, 0.0f,
      "Particle L Knee", "Particle L Foot",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint L Thigh",
      252.0f, 0.0f, 0.0f,
      "Particle L Hip", "Particle L Knee",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint R Calf",
      252.0f, 0.0f, 0.0f,
      "Particle R Knee", "Particle R Foot",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint R Thigh",
      252.0f, 0.0f, 0.0f,
      "Particle R Hip", "Particle R Knee",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint07",
      252.0f, 0.0f, 0.0f,
      "Particle L Hip", "Particle R Torso",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint08",
      252.0f, 0.0f, 0.0f,
      "Particle L Hip", "Particle L Torso",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint09",
      252.0f, 0.0f, 0.0f,
      "Particle R Torso", "Particle L Torso",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint Hips",
      252.0f, 0.0f, 0.0f,
      "Particle R Hip", "Particle L Hip",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint11",
      252.0f, 0.0f, 0.0f,
      "Particle R Hip", "Particle R Torso",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint12",
      252.0f, 0.0f, 0.0f,
      "Particle R Hip", "Particle L Torso",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint13",
      252.0f, 0.0f, 0.0f,
      "Particle R Torso", "Particle Neck",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint14",
      252.0f, 0.0f, 0.0f,
      "Particle R Torso", "Particle R Shoulder",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint15",
      252.0f, 0.0f, 0.0f,
      "Particle R Torso", "Particle L Shoulder",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint16",
      252.0f, 0.0f, 0.0f,
      "Particle L Torso", "Particle Neck",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint17",
      252.0f, 0.0f, 0.0f,
      "Particle L Torso", "Particle R Shoulder",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint18",
      252.0f, 0.0f, 0.0f,
      "Particle L Torso", "Particle L Shoulder",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint19",
      252.0f, 0.0f, 0.0f,
      "Particle Neck", "Particle R Shoulder",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint20",
      252.0f, 0.0f, 0.0f,
      "Particle Neck", "Particle L Shoulder",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint21",
      252.0f, 0.0f, 0.0f,
      "Particle R Shoulder", "Particle L Shoulder",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint22",
      252.0f, 0.0f, 0.0f,
      "Particle L Shoulder", "Particle L Elbow",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint24",
      252.0f, 0.0f, 0.0f,
      "Particle R Shoulder", "Particle R Elbow",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint27",
      252.0f, 0.0f, 0.0f,
      "Particle Neck", "Particle Head",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint36",
      0.0f, 0.0f, 252.0f,
      "Particle R Knee", "Particle R Torso",
      false, 100.0f,
      true, 95.0f,
      false, 100.0f,
      1.0f, false},

    { "Constraint37",
      0.0f, 0.0f, 252.0f,
      "Particle L Knee", "Particle L Torso",
      false, 100.0f,
      true, 95.0f,
      false, 100.0f,
      1.0f, false},

    { "Constraint01",
      0.0f, 252.0f, 0.0f,
      "Particle R Shoulder", "Particle Head",
      false, 100.0f,
      true, 100.0f,
      false, 100.0f,
      0.9f, false},

    { "Constraint04",
      0.0f, 252.0f, 0.0f,
      "Particle L Shoulder", "Particle Head",
      false, 100.0f,
      true, 100.0f,
      false, 100.0f,
      0.9f, false},

    { "Constraint26",
      0.0f, 252.0f, 0.0f,
      "Particle R Torso", "Particle Head",
      false, 100.0f,
      true, 100.0f,
      false, 100.0f,
      0.9f, false},

    { "Constraint30",
      0.0f, 252.0f, 0.0f,
      "Particle L Torso", "Particle Head",
      false, 100.0f,
      true, 100.0f,
      false, 100.0f,
      0.9f, false},

    { "Constraint23",
      252.0f, 0.0f, 0.0f,
      "Particle L Elbow", "Particle L Wrist",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint31",
      252.0f, 0.0f, 0.0f,
      "Particle R Elbow", "Particle R Wrist",
      true, 100.0f,
      false, 100.0f,
      false, 100.0f,
      1.0f, true},

    { "Constraint43",
      252.0f, 0.0f, 252.0f,
      "Particle L Hip", "Particle L Shoulder",
      false, 100.0f,
      true, 90.0f,
      false, 100.0f,
      1.0f, false},

    { "Constraint44",
      252.0f, 0.0f, 252.0f,
      "Particle R Hip", "Particle R Shoulder",
      false, 100.0f,
      true, 90.0f,
      false, 100.0f,
      1.0f, false},

    { "Constraint28",
      0.0f, 252.0f, 0.0f,
      "Particle R Foot", "Particle R Hip",
      false, 100.0f,
      true, 50.0f,
      false, 100.0f,
      1.0f, false},

    { "Constraint29",
      0.0f, 252.0f, 0.0f,
      "Particle L Foot", "Particle L Hip",
      false, 100.0f,
      true, 50.0f,
      false, 100.0f,
      1.0f, false},

    { "Constraint106",
      252.0f, 0.0f, 184.0f,
      "Particle R Shoulder", "Particle R Wrist",
      false, 100.0f,
      true, 30.0f,
      false, 100.0f,
      1.0f, false},

    { "Constraint107",
      252.0f, 0.0f, 184.0f,
      "Particle L Shoulder", "Particle L Wrist",
      false, 100.0f,
      true, 30.0f,
      false, 100.0f,
      1.0f, false},

    { "Constraint108",
      0.0f, 252.0f, 252.0f,
      "Particle L Knee", "Particle R Hip",
      false, 100.0f,
      true, 97.0f,
      false, 115.0f,
      1.0f, false},

    { "Constraint109",
      0.0f, 252.0f, 252.0f,
      "Particle R Knee", "Particle L Hip",
      false, 100.0f,
      true, 97.0f,
      false, 115.0f,
      1.0f, false},

    { "Constraint32",
      0.0f, 252.0f, 252.0f,
      "Particle L Torso", "Particle L Elbow",
      false, 100.0f,
      false, 100.0f,
      true, 125.0f,
      1.0f, false},

    { "Constraint33",
      0.0f, 252.0f, 252.0f,
      "Particle R Torso", "Particle R Elbow",
      false, 100.0f,
      false, 100.0f,
      true, 125.0f,
      1.0f, false},

    { "Constraint34",
      249.0f, 233.0f, 57.0f,
      "Particle R Foot", "Particle R Torso",
      false, 100.0f,
      true, 50.0f,
      false, 100.0f,
      1.0f, false},

    { "Constraint35",
      249.0f, 233.0f, 57.0f,
      "Particle L Foot", "Particle L Torso",
      false, 100.0f,
      true, 50.0f,
      false, 100.0f,
      1.0f, false},

    { "Constraint68",
      255.0f, 255.0f, 169.0f,
      "Particle R Knee", "Particle L Knee",
      false, 100.0f,
      true, 60.0f,
      false, 300.0f,
      1.0f, false},

} ;

//==============================================================================

//bone_match_def MutantTankBoneMatchDefs[] = 
//{
//} ;

//==============================================================================

ragdoll_def MutantTankRagdoll =
{    
    // m_bInitialized
    FALSE,

    // Name
    "MutantTank",

    // Particles
    MutantTankParticleDefs,
    sizeof(MutantTankParticleDefs) / sizeof(MutantTankParticleDefs[0]),

    // Distance constraints
    MutantTankConstraintDefs,
    sizeof(MutantTankConstraintDefs) / sizeof(MutantTankConstraintDefs[0]),

    // Bone matching
    NULL,0,
    //MutantTankBoneMatchDefs,
    //sizeof(MutantTankBoneMatchDefs) / sizeof(MutantTankBoneMatchDefs[0]),

    // Distance rules (built at runtime)
    MutantTankDistRules,
    sizeof(MutantTankDistRules) / sizeof(MutantTankDistRules[0]),
    0,

    // Geometry bones (built at runtime)
    MutantTankGeomBones,
    sizeof(MutantTankGeomBones) / sizeof(MutantTankGeomBones[0]),
    0,

    NULL,
    NULL,
    NULL
} ;

//==============================================================================
