#ifndef __SKIN_SHADER_H__
#define __SKIN_SHADER_H__


//==============================================================================
// Includes
//==============================================================================


#ifdef __cplusplus

#include "Entropy.hpp"

//==============================================================================
// C++ STRUCTURES
//==============================================================================

// Lighting 
struct d3d_skin_lighting
{
    // Just 1 directional light currently supported
    vector3 Dir ;
    vector4 DirCol ;
    vector4 AmbCol ;
} ;

// Vertex shader constants
struct d3d_vs_consts
{
    matrix4 W2C ;               // c0,c1,c2,c3
    f32     Zero ;              // c4.x
    f32     One ;               // c4.y
    f32     MinusOne ;          // c4.z
    f32     Fog ;               // c4.w
    vector4 LightDirCol ;       // c5
    vector4 LightAmbCol ;       // c6
} ;

struct d3d_vs_bone
{
    vector4 L2W0 ;
    vector4 L2W1 ;
    vector4 L2W2 ;
    vector3 LightDir ;
} ;

#endif

//==============================================================================
// SHADER CONSTANTS
//==============================================================================

// Input are:

//      x   y   z   w
// v0 = PX, PY, PZ, I1
// v1 = NX, NY, NZ, I2
// v2 = U,  V,  W1, W2


// Vertex inputs
#define VS_VERT_POS             v0
#define VS_VERT_NORMAL          v1
#define VS_VERT_UVS             v2.xy
#define VS_VERT_INDEX1          v0.w
#define VS_VERT_INDEX2          v1.w
#define VS_VERT_WEIGHT1         v2.z
#define VS_VERT_WEIGHT2         v2.w

// Bone struct
#define VS_BONE_L2W0            0
#define VS_BONE_L2W1            1
#define VS_BONE_L2W2            2
#define VS_BONE_LIGHT_DIR       3
#define VS_BONE_SIZE            4

// World to clip
#define VS_W2C0                 c0
#define VS_W2C1                 c1
#define VS_W2C2                 c2
#define VS_W2C3                 c3

// Constants
#define VS_ZERO                 c4.x
#define VS_ONE                  c4.y
#define VS_MINUS_ONE            c4.z
#define VS_FOG                  c4.w

// Light constants
#define VS_LIGHT_DIR_COL        c5
#define VS_LIGHT_AMB_COL        c6

// Here is where the bones start
#define VS_BONE_REG_OFFSET      7


#endif  // #define __SKIN_SHADER_H__

