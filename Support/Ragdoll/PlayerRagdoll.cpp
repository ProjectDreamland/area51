//==============================================================================
//
//  PlayerRagdoll.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "PlayerRagdoll.hpp"

//==============================================================================
// STORAGE
//==============================================================================

// Storage for the Player ragdoll definition
// (rules and geom bones are filled in at runtime)
static dist_rule   PlayerDistRules[ragdoll::MAX_DIST_RULES] PS2_ALIGNMENT(64);
static geom_bone   PlayerGeomBones[ragdoll::MAX_GEOM_BONES] PS2_ALIGNMENT(64);


//==============================================================================
// MAX EXPORT DATA
//==============================================================================
static particle_def ParticleDefs[] =
{
    { "Particle R Foot",
      0.0f, 0.0f, 252.0f,
      -17.0399f, 6.24967f, 14.9338f,
      1.0f, 0.9f,
      true},

    { "Particle R Knee",
      0.0f, 0.0f, 252.0f,
      -13.5002f, 3.36193f, 55.0521f,
      1.0f, 1.0f,
      true},

    { "Particle L Foot",
      0.0f, 0.0f, 252.0f,
      17.0399f, 6.24972f, 14.9338f,
      1.0f, 0.9f,
      true},

    { "Particle L Knee",
      0.0f, 0.0f, 252.0f,
      13.5002f, 3.36196f, 55.0521f,
      1.0f, 1.0f,
      true},

    { "Particle R Hip",
      0.0f, 0.0f, 252.0f,
      -9.31529f, -1.72484e-005f, 102.444f,
      1.0f, 1.0f,
      true},

    { "Particle L Hip",
      0.0f, 0.0f, 252.0f,
      9.31529f, 8.29242e-006f, 102.444f,
      1.0f, 1.0f,
      true},

    { "Particle R Torso",
      0.0f, 0.0f, 252.0f,
      -13.493f, -5.54545e-006f, 126.865f,
      1.0f, 1.1f,
      true},

    { "Particle L Torso",
      0.0f, 0.0f, 252.0f,
      13.1464f, -5.53761e-006f, 126.686f,
      1.0f, 1.1f,
      true},

    { "Particle Neck",
      0.0f, 0.0f, 252.0f,
      5.51343e-007f, 0.328126f, 167.306f,
      1.0f, 1.0f,
      true},

    { "Particle R Shoulder",
      0.0f, 0.0f, 252.0f,
      -18.7696f, 3.29455f, 155.864f,
      1.0f, 1.2f,
      true},

    { "Particle L Shoulder",
      0.0f, 0.0f, 252.0f,
      18.7696f, 3.29461f, 155.864f,
      1.0f, 1.2f,
      true},

    { "Particle R Elbow",
      0.0f, 0.0f, 252.0f,
      -28.2235f, 2.92555f, 123.779f,
      1.0f, 1.0f,
      true},

    { "Particle L Elbow",
      0.0f, 0.0f, 252.0f,
      28.2235f, 2.92563f, 123.779f,
      1.0f, 1.0f,
      true},

    { "Particle Head",
      0.0f, 0.0f, 252.0f,
      -0.0862226f, -8.15201e-006f, 186.496f,
      1.0f, 1.1f,
      true},

    { "Particle L Wrist",
      0.0f, 0.0f, 252.0f,
      35.9561f, 1.39711f, 97.3971f,
      1.0f, 0.9f,
      true},

    { "Particle R Wrist",
      0.0f, 0.0f, 252.0f,
      -35.9561f, 1.397f, 97.3971f,
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

ragdoll_def PlayerRagdoll =
{    
    // m_bInitialized
    FALSE,

    // Name
    "Player",

    // Particles
    ParticleDefs,
    sizeof(ParticleDefs) / sizeof(ParticleDefs[0]),

    // Distance constraints
    ConstraintDefs,
    sizeof(ConstraintDefs) / sizeof(ConstraintDefs[0]),

    // Bone matching
    0,
    NULL,

    // Distance rules (built at runtime)
    PlayerDistRules,
    sizeof(PlayerDistRules) / sizeof(PlayerDistRules[0]),
    0,

    // Geometry bones (built at runtime)
    PlayerGeomBones,
    sizeof(PlayerGeomBones) / sizeof(PlayerGeomBones[0]),
    0,

    NULL,
    NULL,
    NULL
} ;

//==============================================================================


