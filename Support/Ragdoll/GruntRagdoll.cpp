//==============================================================================
//
//  GruntRagdoll.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "GruntRagdoll.hpp"


//==============================================================================
// STORAGE
//==============================================================================

// Storage for the Grunt ragdoll definition
// (rules and geom bones are filled in at runtime)
static dist_rule   GruntDistRules[ragdoll::MAX_DIST_RULES] PS2_ALIGNMENT(64);
static geom_bone   GruntGeomBones[ragdoll::MAX_GEOM_BONES] PS2_ALIGNMENT(64);


//==============================================================================
// MAX EXPORT DATA
//==============================================================================
static particle_def ParticleDefs[] =
{
    { "Particle R Foot",
      0.0f, 0.0f, 252.0f,
      -11.6203f, 6.68197f, 16.0698f,
      1.0f, 0.9f,
      true},

    { "Particle R Knee",
      0.0f, 0.0f, 252.0f,
      -11.6203f, -2.0387f, 66.1522f,
      1.0f, 1.0f,
      true},

    { "Particle L Foot",
      0.0f, 0.0f, 252.0f,
      11.6203f, 6.682f, 16.0698f,
      1.0f, 0.9f,
      true},

    { "Particle L Knee",
      0.0f, 0.0f, 252.0f,
      11.6203f, -2.03867f, 66.1522f,
      1.0f, 1.0f,
      true},

    { "Particle R Hip",
      0.0f, 0.0f, 252.0f,
      -11.6203f, 2.29215f, 121.682f,
      1.0f, 1.0f,
      true},

    { "Particle L Hip",
      0.0f, 0.0f, 252.0f,
      11.6203f, 2.29218f, 121.682f,
      1.0f, 1.0f,
      true},

    { "Particle R Torso",
      0.0f, 0.0f, 252.0f,
      -14.1888f, -6.13456e-006f, 140.342f,
      1.0f, 1.1f,
      true},

    { "Particle L Torso",
      0.0f, 0.0f, 252.0f,
      14.1995f, -5.96046e-006f, 140.159f,
      1.0f, 1.1f,
      true},

    { "Particle Neck",
      0.0f, 0.0f, 252.0f,
      1.35253e-005f, -9.03266f, 184.446f,
      1.0f, 1.1f,
      true},

    { "Particle R Shoulder",
      0.0, 0.0, 252.0,
      -21.8693f, -5.06357f, 175.948f,
      1.0f, 1.2f,
      true},

    { "Particle L Shoulder",
      0.0, 0.0, 252.0,
      21.8693f, -5.06351f, 175.948f,
      1.0f, 1.2f,
      true},

    { "Particle R Elbow",
      0.0, 0.0, 252.0,
      -41.6209f, -4.05996f, 145.81f,
      1.0f, 1.0f,
      true},

    { "Particle L Elbow",
      0.0, 0.0, 252.0,
      41.6209f, -4.05985f, 145.81f,
      1.0f, 1.0f,
      true},

    { "Particle Head",
      0.0, 0.0, 252.0,
      1.85081e-005f, -20.5621f, 203.391f,
      1.0f, 1.0f,
      true},

    { "Particle L Wrist",
      0.0, 0.0, 252.0,
      60.5473f, -5.45715f, 116.962f,
      1.0f, 0.9f,
      true},

    { "Particle R Wrist",
      0.0, 0.0, 252.0,
      -60.5472f, -5.45733f, 116.962f,
      1.0f, 0.9f,
      true},

} ;
static constraint_def ConstraintDefs[] =
{
    { "Constraint L Calf",
      252.0, 0.0, 0.0,
      "Particle L Knee", "Particle L Foot",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint L Thigh",
      252.0, 0.0, 0.0,
      "Particle L Hip", "Particle L Knee",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint R Calf",
      252.0, 0.0, 0.0,
      "Particle R Knee", "Particle R Foot",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint R Thigh",
      252.0, 0.0, 0.0,
      "Particle R Hip", "Particle R Knee",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint07",
      252.0, 0.0, 0.0,
      "Particle L Hip", "Particle R Torso",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint08",
      252.0, 0.0, 0.0,
      "Particle L Hip", "Particle L Torso",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint09",
      252.0, 0.0, 0.0,
      "Particle R Torso", "Particle L Torso",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint Hips",
      252.0, 0.0, 0.0,
      "Particle R Hip", "Particle L Hip",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint11",
      252.0, 0.0, 0.0,
      "Particle R Hip", "Particle R Torso",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint12",
      252.0, 0.0, 0.0,
      "Particle R Hip", "Particle L Torso",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint13",
      252.0, 0.0, 0.0,
      "Particle R Torso", "Particle Neck",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint14",
      252.0, 0.0, 0.0,
      "Particle R Torso", "Particle R Shoulder",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint15",
      252.0, 0.0, 0.0,
      "Particle R Torso", "Particle L Shoulder",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint16",
      252.0, 0.0, 0.0,
      "Particle L Torso", "Particle Neck",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint17",
      252.0, 0.0, 0.0,
      "Particle L Torso", "Particle R Shoulder",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint18",
      252.0, 0.0, 0.0,
      "Particle L Torso", "Particle L Shoulder",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint19",
      252.0, 0.0, 0.0,
      "Particle Neck", "Particle R Shoulder",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint20",
      252.0, 0.0, 0.0,
      "Particle Neck", "Particle L Shoulder",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint21",
      252.0, 0.0, 0.0,
      "Particle R Shoulder", "Particle L Shoulder",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint22",
      252.0, 0.0, 0.0,
      "Particle L Shoulder", "Particle L Elbow",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint24",
      252.0, 0.0, 0.0,
      "Particle R Shoulder", "Particle R Elbow",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint27",
      252.0, 0.0, 0.0,
      "Particle Neck", "Particle Head",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint36",
      0.0, 0.0, 252.0,
      "Particle R Knee", "Particle R Torso",
      false, 100.0,
      true, 95.0,
      false, 100.0,
      1.0f, false},

    { "Constraint37",
      0.0, 0.0, 252.0,
      "Particle L Knee", "Particle L Torso",
      false, 100.0,
      true, 95.0,
      false, 100.0,
      1.0f, false},

    { "Constraint01",
      0.0, 252.0, 0.0,
      "Particle R Shoulder", "Particle Head",
      false, 100.0,
      true, 100.0,
      false, 100.0,
      0.9f, false},

    { "Constraint04",
      0.0, 252.0, 0.0,
      "Particle L Shoulder", "Particle Head",
      false, 100.0,
      true, 100.0,
      false, 100.0,
      0.9f, false},

    { "Constraint26",
      0.0, 252.0, 0.0,
      "Particle R Torso", "Particle Head",
      false, 100.0,
      true, 100.0,
      false, 100.0,
      0.9f, false},

    { "Constraint30",
      0.0, 252.0, 0.0,
      "Particle L Torso", "Particle Head",
      false, 100.0,
      true, 100.0,
      false, 100.0,
      0.9f, false},

    { "Constraint23",
      252.0, 0.0, 0.0,
      "Particle L Elbow", "Particle L Wrist",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint31",
      252.0, 0.0, 0.0,
      "Particle R Elbow", "Particle R Wrist",
      true, 100.0,
      false, 100.0,
      false, 100.0,
      1.0f, true},

    { "Constraint43",
      252.0, 0.0, 252.0,
      "Particle L Hip", "Particle L Shoulder",
      false, 100.0,
      true, 90.0,
      false, 100.0,
      1.0f, false},

    { "Constraint44",
      252.0, 0.0, 252.0,
      "Particle R Hip", "Particle R Shoulder",
      false, 100.0,
      true, 90.0,
      false, 100.0,
      1.0f, false},

    { "Constraint28",
      0.0, 252.0, 0.0,
      "Particle R Foot", "Particle R Hip",
      false, 100.0,
      true, 50.0,
      false, 100.0,
      1.0f, false},

    { "Constraint29",
      0.0, 252.0, 0.0,
      "Particle L Foot", "Particle L Hip",
      false, 100.0,
      true, 50.0,
      false, 100.0,
      1.0f, false},

    { "Constraint106",
      252.0, 0.0, 184.0,
      "Particle R Shoulder", "Particle R Wrist",
      false, 100.0,
      true, 30.0,
      false, 100.0,
      1.0f, false},

    { "Constraint107",
      252.0, 0.0, 184.0,
      "Particle L Shoulder", "Particle L Wrist",
      false, 100.0,
      true, 30.0,
      false, 100.0,
      1.0f, false},

    { "Constraint108",
      0.0, 252.0, 252.0,
      "Particle L Knee", "Particle R Hip",
      false, 100.0,
      true, 97.0,
      false, 115.0,
      1.0f, false},

    { "Constraint109",
      0.0, 252.0, 252.0,
      "Particle R Knee", "Particle L Hip",
      false, 100.0,
      true, 97.0,
      false, 115.0,
      1.0f, false},

    { "Constraint32",
      0.0, 252.0, 252.0,
      "Particle L Torso", "Particle L Elbow",
      false, 100.0,
      false, 100.0,
      true, 125.0,
      1.0f, false},

    { "Constraint33",
      0.0, 252.0, 252.0,
      "Particle R Torso", "Particle R Elbow",
      false, 100.0,
      false, 100.0,
      true, 125.0,
      1.0f, false},

    { "Constraint34",
      249.0, 233.0, 57.0,
      "Particle R Foot", "Particle R Torso",
      false, 100.0,
      true, 50.0,
      false, 100.0,
      1.0f, false},

    { "Constraint35",
      249.0, 233.0, 57.0,
      "Particle L Foot", "Particle L Torso",
      false, 100.0,
      true, 50.0,
      false, 100.0,
      1.0f, false},

    { "Constraint68",
      255.0, 255.0, 169.0,
      "Particle R Knee", "Particle L Knee",
      false, 100.0,
      true, 60.0,
      false, 300.0,
      1.0f, false},

} ;

//==============================================================================

//bone_match_def GruntBoneMatchDefs[] = 
//{
    //{"Bip01 R Clavicle",    "Chest"},
    //{"Bip01 L Clavicle",    "Chest"},
//} ;

//==============================================================================

ragdoll_def GruntRagdoll =
{    
    // m_bInitialized
    FALSE,

    // Name
    "Grunt",

    // Particles
    ParticleDefs,
    sizeof(ParticleDefs) / sizeof(ParticleDefs[0]),

    // Distance constraints
    ConstraintDefs,
    sizeof(ConstraintDefs) / sizeof(ConstraintDefs[0]),

    // Bone matching
    0,
    NULL,
    //GruntBoneMatchDefs,
    //sizeof(GruntBoneMatchDefs) / sizeof(GruntBoneMatchDefs[0]),

    // Distance rules (built at runtime)
    GruntDistRules,
    sizeof(GruntDistRules) / sizeof(GruntDistRules[0]),
    0,

    // Geometry bones (built at runtime)
    GruntGeomBones,
    sizeof(GruntGeomBones) / sizeof(GruntGeomBones[0]),
    0,

    NULL,
    NULL,
    NULL
} ;

//==============================================================================

