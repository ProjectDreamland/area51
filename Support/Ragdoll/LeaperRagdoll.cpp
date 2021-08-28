//==============================================================================
//
//  LeaperRagdoll.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "LeaperRagdoll.hpp"


//==============================================================================
// STORAGE
//==============================================================================

// Storage for the scientist ragdoll definition
// (rules and geom bones are filled in at runtime)
static dist_rule   LeaperDistRules[ragdoll::MAX_DIST_RULES] PS2_ALIGNMENT(64);
static geom_bone   LeaperGeomBones[ragdoll::MAX_GEOM_BONES] PS2_ALIGNMENT(64);


//==============================================================================
// MAX EXPORT DATA
//==============================================================================
static particle_def ParticleDefs[] =
{
    { "Particle R Foot",
      0.0f, 0.0f, 252.0f,
      -19.9182f, 1.27161f, 17.7417f,
      1.0f, 0.9f,
      true},

    { "Particle R Knee",
      0.0f, 0.0f, 252.0f,
      -15.9604f, -11.6625f, 69.5125f,
      1.0f, 1.0f,
      true},

    { "Particle L Foot",
      0.0f, 0.0f, 252.0f,
      19.9181f, 1.27166f, 17.7417f,
      1.0f, 0.9f,
      true},

    { "Particle L Knee",
      0.0f, 0.0f, 252.0f,
      15.9605f, -11.6625f, 69.5125f,
      1.0f, 1.0f,
      true},

    { "Particle R Hip",
      0.0f, 0.0f, 252.0f,
      -12.5722f, -2.22935e-005f, 115.717f,
      1.0f, 1.0f,
      true},

    { "Particle L Hip",
      0.0f, 0.0f, 252.0f,
      12.5722f, 1.21771e-005f, 115.717f,
      1.0f, 1.0f,
      true},

    { "Particle R Torso",
      0.0f, 0.0f, 252.0f,
      -15.0f, -0.3683f, 141.874f,
      1.0f, 1.1f,
      true},

    { "Particle L Torso",
      0.0f, 0.0f, 252.0f,
      15.0f, -0.368301f, 141.874f,
      1.0f, 1.1f,
      true},

    { "Particle Neck",
      0.0f, 0.0f, 252.0f,
      -4.88496e-007f, -1.10257f, 190.998f,
      1.0f, 1.0f,
      true},

    { "Particle R Shoulder",
      0.0f, 0.0f, 252.0f,
      -24.7545f, 4.18153f, 177.858f,
      1.0f, 1.2f,
      true},

    { "Particle L Shoulder",
      0.0f, 0.0f, 252.0f,
      24.7545f, 4.1816f, 177.858f,
      1.0f, 1.2f,
      true},

    { "Particle R Elbow",
      0.0f, 0.0f, 252.0f,
      -33.446f, 5.63265f, 143.556f,
      1.0f, 1.0f,
      true},

    { "Particle L Elbow",
      0.0f, 0.0f, 252.0f,
      33.4459f, 5.63274f, 143.556f,
      1.0f, 1.0f,
      true},

    { "Particle Head",
      0.0f, 0.0f, 252.0f,
      0.00710011f, -4.73599f, 218.281f,
      1.0f, 1.1f,
      true},

    { "Particle L Wrist",
      0.0f, 0.0f, 252.0f,
      42.5367f, 1.08175f, 108.095f,
      1.0f, 0.9f,
      true},

    { "Particle R Wrist",
      0.0f, 0.0f, 252.0f,
      -42.5367f, 1.08163f, 108.095f,
      1.0f, 0.9f,
      true},

} ;
static constraint_def ConstraintDefs[] =
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
      true, 400.0f,
      1.0f, false},

    { "Constraint33",
      0.0f, 252.0f, 252.0f,
      "Particle R Torso", "Particle R Elbow",
      false, 100.0f,
      false, 100.0f,
      true, 400.0f,
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

bone_match_def LeaperBoneMatchDefs[] = 
{
    // Specify hand bone matching because fingers in bind auto-connect to legs otherwise
    {"_Hand_L_",    "L Hand"},
    {"_Hand_R_",    "R Hand"},
} ;

//==============================================================================

ragdoll_def LeaperRagdoll =
{    
    // m_bInitialized
    FALSE,

    // Name
    "Leaper",

    // Particles
    ParticleDefs,
    sizeof(ParticleDefs) / sizeof(ParticleDefs[0]),

    // Distance constraints
    ConstraintDefs,
    sizeof(ConstraintDefs) / sizeof(ConstraintDefs[0]),

    // Bone matching
    LeaperBoneMatchDefs,
    sizeof(LeaperBoneMatchDefs) / sizeof(LeaperBoneMatchDefs[0]),

    // Distance rules (built at runtime)
    LeaperDistRules,
    sizeof(LeaperDistRules) / sizeof(LeaperDistRules[0]),
    0,

    // Geometry bones (built at runtime)
    LeaperGeomBones,
    sizeof(LeaperGeomBones) / sizeof(LeaperGeomBones[0]),
    0,

    NULL,
    NULL,
    NULL
} ;

//==============================================================================


