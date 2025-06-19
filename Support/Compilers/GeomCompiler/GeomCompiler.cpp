//=============================================================================
//
//  Rigid and Skin Geom Compiler by JP and TA
//
//=============================================================================

#include "GeomCompiler.hpp"
#include "Render\RigidGeom.hpp"
#include "Render\SkinGeom.hpp"
#include "faststrip.hpp"
#include "PS2strip.hpp"
#include "ArmOptimizer.hpp"
#include "PS2SkinOptimizer.hpp"
#include "BMPUtil.hpp"
#include "Auxiliary\Bitmap\aux_Bitmap.hpp"
#include "RigidDesc.hpp"
#include "SkinDesc.hpp"
#include <io.h>
#include "RawSettings.hpp"


// Xbox specific
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <3rdParty/Xbox/Include/D3d8.h>
#include <3rdParty/Xbox/Include/XGraphics.h>
#include <3rdParty/Nvidia/NvTriStrip/NvTriStrip.h>

#define STRIPE_GEOMS 1

//=============================================================================

#define MAX_TEMP_PUSH_BUFFER 262144

#define MAX_VU_VERTS            80
#define BACKFACE_CULLING        TRUE
#define SORT_BONES              FALSE
#define ADCBIT                  15
#define CCWBIT                  5
#define PS2_SKIN_FRACTION_BITS  4
#define PS2_SKIN_FIXED_SCALE    ((f32)(1<<PS2_SKIN_FRACTION_BITS))
#define PS2_SKIN_RANGE          ((f32)(1<<(15-PS2_SKIN_FRACTION_BITS)))
#define MAX_UV_KEYS             10000

//=============================================================================

//
// Layout of Max material "constants"
//
enum
{
    MaxConst_DetailScale,
    MaxConst_EnvType,
    MaxConst_EnvBlend,
    MaxConst_FixedAlpha,
};

//
// Material parameter slots in 3DS Max
//
enum
{
    DETAIL_SCALE,
    ENV_TYPE,
    ENV_BLEND,
    FIXED_ALPHA,
    FORCE_ZFILL,
    USE_DIFFUSE,
    UNUSED2,
    UNUSED1,

    // these parameters are set by the compiler
    PUNCH_THRU,
    COMPILED_UNUSED3,
    COMPILED_UNUSED2,
    COMPILED_UNUSED1,

    MAX_PARAMS
};

//=============================================================================

//
// Name of Map slots in 3DS Max
//

char* MaxMapNames[ geom_compiler::NumMaps ] =
{
    "Diffuse Map",
    "Diffuse Map 2",
    "Diffuse Blend",
    "LightMap",
    "Opacity Map",
    "Environment Intensity",
    "Environment Map",
    "Self-Illumination Map",
    "Detail Map",
    "Punch-Through",
};

const s32 MaxMeshes        =  63;                       // Max number of sub-meshes per RawMesh
const s32 MaxMaterials     =  63;                       // Max number of materials per sub-mesh
const s32 MaxTextureWidth  = 512;
const s32 MaxTextureHeight = 512;


extern xbool g_Verbose;
extern xbool g_ColoredMips;
extern xbool g_DoCollision;
extern xbool g_ShrinkTextures;
extern struct _finddata_t g_ExeData;

//=============================================================================

xbool LoadBitmap( xbitmap& Bitmap, const char* pFileName )
{
    if( auxbmp_Load( Bitmap, pFileName) == FALSE )
        return FALSE;

    // Shrink if requested
    if( g_ShrinkTextures )
    {
        x_throw( "SHRINKING in deprecated." );
        /*
        x_printf("SHRINKING (%s)\n",pFileName);
        s32 W,H;
        W = Bitmap.GetWidth();
        H = Bitmap.GetHeight();
        if( (W>=32) && (H>=32) )
        {
            W /= 2;
            H /= 2;
            Bitmap.Resize(W,H,TRUE);
        }
        */
    }

    return TRUE;
}

//=============================================================================

geom_compiler::geom_compiler( void )
{
    m_FastCollision     [0] = 0;
    m_PhysicsMatx       [0] = 0;    
    m_SettingsFile      [0] = 0;
    m_TexturePath       [0] = 0 ;
    m_pGeomRscDesc          = NULL;
    x_strcpy( m_UserName, "Unknown user" );
    x_strcpy( m_ComputerName, "Unknown computer" );
    m_ExportDate        [0] = 0;
    m_ExportDate        [1] = 0;
    m_ExportDate        [2] = 0;
}

//=============================================================================

geom_compiler::~geom_compiler( void )
{
    // TODO: Are we creating a memory leak with the geom_rsc_desc??
    // should the resource loader clean it up, or do we do that manually?
}

//=============================================================================

void geom_compiler::SetUserInfo( const char* pUserName, const char* pComputerName )
{
    if( pUserName )
        x_strcpy( m_UserName, pUserName );
    else
        x_strcpy( m_UserName, "Unknown user" );
    
    if( pComputerName )
        x_strcpy( m_ComputerName, pComputerName );
    else
        x_strcpy( m_ComputerName, "Unknown computer" );
}

//=============================================================================

void geom_compiler::SetExportDate( s32 Month, s32 Day, s32 Year )
{
    m_ExportDate[0] = Month;
    m_ExportDate[1] = Day;
    m_ExportDate[2] = Year;
}

//=============================================================================

void geom_compiler::ReportWarning( const char* pWarning )
{
    x_printf( "WARNING: %s\n", pWarning );
    x_printf( "  Last exported by: %s, on: %d/%d/%d\n", m_UserName, m_ExportDate[0], m_ExportDate[1], m_ExportDate[2] );
}

//=============================================================================

void geom_compiler::ReportError( const char* pError )
{
    x_printf( "ERROR: %s\n", pError );
    x_printf( "  Last exported by: %s, on: %d/%d/%d\n", m_UserName, m_ExportDate[0], m_ExportDate[1], m_ExportDate[2] );
}

//=============================================================================

void geom_compiler::ThrowError( const char* pError )
{
    ReportError( pError );
    x_throw( "GeomCompiler Error Occured" );
}

//=============================================================================

void geom_compiler::AddPlatform( platform Platform, const char* pFileName )
{
    plat_info& P = m_PlatInfo.Append();
    x_strcpy( P.FileName, pFileName );
    P.Platform = Platform;
}

//=============================================================================

s32 geom_compiler::GetPlatformIndex( platform Platform )
{
    s32 Index = -1;

    for( s32 i=0; i<m_PlatInfo.GetCount(); i++ )
    {
        if( m_PlatInfo[i].Platform == Platform )
        {
            Index = i;
            break;
        }
    }
    
    if( Index == -1 )
        x_throw( "Unknown platform" );

    return( Index );
}

//=============================================================================

void geom_compiler::AddFastCollision( const char* pFileName )
{
    x_strcpy( m_FastCollision, pFileName );
}

//=============================================================================

void geom_compiler::SetPhysicsMatx( const char* pFileName )
{
    x_strcpy( m_PhysicsMatx, pFileName );
}

//=============================================================================

void geom_compiler::SetSettingsFile( const char* pFileName )
{
    x_strcpy( m_SettingsFile, pFileName );
}

//=============================================================================

void geom_compiler::LoadResource( const char* pFileName, xbool bSkin )
{
    // check the extension. are we loading a matx file or a resource file?
    char Ext[X_MAX_EXT];
    x_splitpath( pFileName, NULL, NULL, NULL, Ext );

    if( !x_stricmp( Ext, ".matx" ) )
    {
        // we need to create and fill in a resource description, making the
        // vmeshes match to the rawmesh submeshes

        // create a geom resource
        if( bSkin )
        {
            m_pGeomRscDesc = (geom_rsc_desc*)&g_RescDescMGR.CreateRscDesc( ".skingeom" );
            if( x_strcmp( m_pGeomRscDesc->GetType(), "SkinGeom" ) )
            {
                ThrowError( "Internal error - geom type mismatch" );
            }
        }
        else
        {
            m_pGeomRscDesc = (geom_rsc_desc*)&g_RescDescMGR.CreateRscDesc( ".rigidgeom" );
            if( x_strcmp( m_pGeomRscDesc->GetType(), "RigidGeom" ) )
            {
                ThrowError( "Internal error - geom type mismatch" );
            }
        }
        
        // copy in the filename
        m_pGeomRscDesc->SetMatxFileName( pFileName );
        
        // load the rawmesh so we can match up vmeshes
        rawmesh2 RawMesh;
        if( !RawMesh.Load( pFileName ) )
        {
            ThrowError( xfs( "Unable to load (%s)", pFileName ) );
        }

        // match up vmeshes to submeshes
        s32 i;
        for( i = 0; i < RawMesh.m_nSubMeshs; i++ )
        {
            geom_rsc_desc::virtual_mesh& VMesh = m_pGeomRscDesc->AppendVirtualMesh();
            x_strsavecpy( VMesh.Name, RawMesh.m_pSubMesh[i].Name, geom_rsc_desc::MAX_NAME_LENGTH );
            
            geom_rsc_desc::lod_info& LodInfo = VMesh.LODs.Append();
            LodInfo.nMeshes = 1;
            x_strsavecpy( LodInfo.MeshName[0], RawMesh.m_pSubMesh[i].Name, geom_rsc_desc::MAX_NAME_LENGTH );
            LodInfo.ScreenSize = 10000;
        }
    }
    else
    {
        // just load the resource file through the manager
        if( bSkin )
        {
            skingeom_rsc_desc& RscDesc = (skingeom_rsc_desc&)g_RescDescMGR.Load( pFileName );
            if( x_strcmp( RscDesc.GetType(), "SkinGeom" ) )
            {
                x_throw( xfs("Trying to compile non-skin geom resource as skingeom (%s)", RscDesc.GetName()) );
            }
            m_pGeomRscDesc = (geom_rsc_desc*)&RscDesc;
        }
        else
        {
            rigidgeom_rsc_desc& RscDesc = (rigidgeom_rsc_desc&)g_RescDescMGR.Load( pFileName );
            if( x_strcmp( RscDesc.GetType(), "RigidGeom" ) )
            {
                x_throw( xfs("Trying to compile non-rigid geom resource as rigidgeom (%s)", RscDesc.GetName()) );
            }
            m_pGeomRscDesc = (geom_rsc_desc*)&RscDesc;
        }
    }
}

//=============================================================================

struct bone_to_hit_mapping
{
    const char*                 pBoneNameFragment;
    geom::bone::hit_location    HitLocation;
};

static 
bone_to_hit_mapping g_BoneHitLocationMapping[] = 
{
    { "Leg",        geom::bone::HIT_LOCATION_LEGS           },

    { "Spine",      geom::bone::HIT_LOCATION_TORSO          },      

    { "Neck",       geom::bone::HIT_LOCATION_HEAD           },
    { "Head",       geom::bone::HIT_LOCATION_HEAD           },

    { "Arm_L",      geom::bone::HIT_LOCATION_SHOULDER_LEFT  },
    { "L_Forearm",  geom::bone::HIT_LOCATION_SHOULDER_LEFT  },
    { "L_Shoulder", geom::bone::HIT_LOCATION_SHOULDER_LEFT  },

    { "Arm_R",      geom::bone::HIT_LOCATION_SHOULDER_RIGHT },
    { "R_Forearm",  geom::bone::HIT_LOCATION_SHOULDER_RIGHT },
    { "R_Shoulder", geom::bone::HIT_LOCATION_SHOULDER_RIGHT },   

    // Dust Mites
    { "Body",       geom::bone::HIT_LOCATION_TORSO          },      
    { "Jaw",        geom::bone::HIT_LOCATION_HEAD           },      

};

//=============================================================================

struct object_to_object_mapping
{
    const char* pObjectA;
    const char* pObjectB;
};

// NOTE: This table is used to map Bone->RigidBody mapping and RigidBody->Bone mapping
static 
object_to_object_mapping g_BoneToRigidBodyMapping[] = 
{
    // Bone             // RigidBody
    
    // Extra theta bones
    { "Leg_L_Foot_Squash01",    "L_Foot"                },
    { "Leg_L_Foot_Squash02",    "L_Foot"                },
    { "Leg_L_Foot",             "L_Foot"                },
    { "Leg_L_Calf01",           "L_Calf01"              },
    { "Leg_L_Calf02",           "L_Calf02"              },
    { "Leg_L_Thigh",            "L_Thigh"               },

    { "ChstArm_L_UpperArm",     "L_ChstArm_UpperArm"    },
    { "ChstArm_L_ForeArm",      "L_ChstArm_LowerArm"    },
    { "ChstArm_L_Hand",         "L_ChstArm_Hand"        },
    { "ChstArm_L_Finger",       "L_ChstArm_Hand"        },
    { "ChstArm_L_Thumb",        "L_ChstArm_Hand"        },
    
    { "Leg_R_Foot_Squash01",    "R_Foot"                },
    { "Leg_R_Foot_Squash02",    "R_Foot"                },
    { "Leg_R_Foot",             "R_Foot"                },
    { "Leg_R_Calf01",           "R_Calf01"              },
    { "Leg_R_Calf02",           "R_Calf02"              },
    { "Leg_R_Thigh",            "R_Thigh"               },

    { "ChstArm_R_UpperArm",     "R_ChstArm_UpperArm"    },
    { "ChstArm_R_ForeArm",      "R_ChstArm_LowerArm"    },
    { "ChstArm_R_Hand",         "R_ChstArm_Hand"        },
    { "ChstArm_R_Finger",       "R_ChstArm_Hand"        },
    { "ChstArm_R_Thumb",        "R_ChstArm_Hand"        },

    { "Neck",                   "Neck"                  },
    { "spore",                  "Torso"                 },
    
    // Normal characters
    { "Root",                   "Pelvis"        },
    { "Spine01",                "Pelvis"        },
    { "Spine02",                "Torso"         },
    { "Head",                   "Head"          },
    { "Neck",                   "Torso"         },
    { "Face",                   "Head"          },
    { "Jaw",                    "Head"          },

    { "Arm_R_UpperArm",         "R_UpperArm"    },
    { "Arm_R_ForeArm",          "R_LowerArm"    },
    { "R_Forearm",              "R_LowerArm"    },
    { "Arm_R_Hand",             "R_Hand"        },
    { "Hand_R",                 "R_Hand"        },
    { "Attach_R",               "R_Hand"        },

    { "Arm_L_UpperArm",         "L_UpperArm"    },
    { "Arm_L_ForeArm",          "L_LowerArm"    },
    { "L_Forearm",              "L_LowerArm"    },
    { "Arm_L_Hand",             "L_Hand"        },
    { "Hand_L",                 "L_Hand"        },
    { "Attach_L",               "L_Hand"        },

    { "Leg_R_Thigh",            "R_Thigh"       },
    { "Leg_R_Calf",             "R_Calf"        },
    { "Leg_R_Foot",             "R_Foot"        },
    { "Leg_R_Toe",              "R_Foot"        },

    { "Leg_L_Thigh",            "L_Thigh"       },
    { "Leg_L_Calf",             "L_Calf"        },
    { "Leg_L_Foot",             "L_Foot"        },
    { "Leg_L_Toe",              "L_Foot"        },

    { "L_Shoulder",             "Torso"         },
    { "R_Shoulder",             "Torso"         },
    { "Body",                   "Torso"         },
};

//=============================================================================

void geom_compiler::BuildBone( geom::bone& Bone, const rawmesh2::bone& RawBone )
{
    // Grab bind info
    Bone.BindRotation = RawBone.Rotation;
    Bone.BindPosition = RawBone.Position;

    // Grab bbox
    Bone.BBox = RawBone.BBox;

    // Clear hit location and rigid body index
    Bone.HitLocation = geom::bone::HIT_LOCATION_UNKNOWN;
    Bone.iRigidBody  = -1;
}

//=============================================================================

void geom_compiler::BuildBones(       geom&     Geom, 
                                const rawmesh2& GeomRawMesh,
                                const rawmesh2& PhysicsRawMesh )
{
    s32 i, j, nMaps;

    // Get bone count
    s32 nBones = GeomRawMesh.m_nBones ;
    if(! nBones )
        return;

    // Create bones
    Geom.m_nBones = nBones ;
    Geom.m_pBone  = new geom::bone[nBones] ;
    if( Geom.m_pBone == NULL )
        x_throw( "Out of memory" );

    // Setup bones
    for( i = 0 ; i < nBones ; i++ )
        BuildBone( Geom.m_pBone[i], GeomRawMesh.m_pBone[i] );

    // Setup hit locations
    for( i = 0; i < nBones; i++ )
    {
        // Loop through all maps
        nMaps = sizeof(g_BoneHitLocationMapping) / sizeof(bone_to_hit_mapping);
        for( j = 0; j < nMaps; j++ )
        {
            // Found bone in mapping table?
            if( x_stristr( GeomRawMesh.m_pBone[i].Name, g_BoneHitLocationMapping[j].pBoneNameFragment ) )
            {
                // Map bone if not already assigned
                geom::bone& Bone = Geom.m_pBone[i];
                if( Bone.HitLocation == geom::bone::HIT_LOCATION_UNKNOWN )
                    Bone.HitLocation = g_BoneHitLocationMapping[j].HitLocation;
            }            
        }
    }
    
    // Setup bone -> rigid body mapping
    if( PhysicsRawMesh.m_nRigidBodies )
    {
        // Loop through bones
        for( i = 0; i < nBones; i++ )
        {
            // Loop through map table
            nMaps = sizeof(g_BoneToRigidBodyMapping) / sizeof(object_to_object_mapping);
            for( j = 0; j < nMaps; j++ )
            {
                // Found bone in mapping table?
                if( x_stristr( GeomRawMesh.m_pBone[i].Name, g_BoneToRigidBodyMapping[j].pObjectA ) )
                {
                    // Try map bone if not already assigned
                    geom::bone& Bone = Geom.m_pBone[i];
                    if( Bone.iRigidBody == -1 )
                        Bone.iRigidBody = PhysicsRawMesh.GetRigidBodyIDFromName( g_BoneToRigidBodyMapping[j].pObjectB, TRUE );
                }                    
            }
        }
            
        // Validate bone -> rigid body mapping
        for( i = 0; i < nBones; i++ )
        {
            // Lookup bone
            geom::bone& Bone = Geom.m_pBone[i];
        
            // Not mapped yet?
            if( Bone.iRigidBody == -1 )
            {
                // Try map bone to matching rigid body name
                const char* pBoneName = GeomRawMesh.m_pBone[i].Name;
                while( ( Bone.iRigidBody == -1 ) && ( pBoneName[0] ) )
                {
                    // Try lookup rigid body
                    Bone.iRigidBody = PhysicsRawMesh.GetRigidBodyIDFromName( pBoneName, TRUE );
                    
                    // Goto next character in bone name
                    pBoneName++;
                }
                
                // Still not found?
                if( Bone.iRigidBody == -1 )
                {
                    // Map to root rigid body
                    Bone.iRigidBody = 0;

                    // Show warning
                    ReportWarning( xfs("Bone [%s] -> RigidBody mapping not found, defaulting to root rigid body.", GeomRawMesh.m_pBone[i].Name ) );
                }
            }
        }
    }
    
    // Show bone->rigid body mappings?
    if( ( g_Verbose ) && ( PhysicsRawMesh.m_nRigidBodies ) )
    {
        x_printf( "\nBone->RigidBody mappings:\n" );
        for( i = 0; i < nBones; i++ )
        {
            // Lookup bone
            geom::bone& Bone = Geom.m_pBone[i];
            if( Bone.iRigidBody != -1 )
            {
                x_printf( "\"%s\" mapped to \"%s\"\n", 
                          GeomRawMesh.m_pBone[i].Name,
                          PhysicsRawMesh.m_pRigidBodies[Bone.iRigidBody].Name );
            }                          
            else                                
            {
                x_printf("\"%s\" mapped to NULL!\n", GeomRawMesh.m_pBone[i].Name );
            }
        }    
        x_printf( "\n\n" );
    }
}
    
//=============================================================================

void geom_compiler::BuildRigidBody( geom::rigid_body& RigidBody, const rawmesh2::rigid_body& RawRigidBody )
{
    // Grab properties
    RigidBody.BodyBindRotation  = RawRigidBody.BodyRotation;
    RigidBody.BodyBindPosition  = RawRigidBody.BodyPosition;
    RigidBody.PivotBindRotation = RawRigidBody.PivotRotation;
    RigidBody.PivotBindPosition = RawRigidBody.PivotPosition;
    RigidBody.NameOffset        = m_Dictionary.Add( RawRigidBody.Name );
    RigidBody.Mass              = RawRigidBody.Mass;  
    RigidBody.Radius            = RawRigidBody.Radius;
    RigidBody.Width             = RawRigidBody.Width;
    RigidBody.Height            = RawRigidBody.Height;
    RigidBody.Length            = RawRigidBody.Length;
    RigidBody.iParentBody       = RawRigidBody.iParent;    
    RigidBody.iBone             = -1;

    // Bake bind scale into properties so we don't have to export it
    RigidBody.Radius       *= RawRigidBody.BodyScale.GetX(); 
    RigidBody.Width        *= RawRigidBody.BodyScale.GetX();  
    RigidBody.Height       *= RawRigidBody.BodyScale.GetY(); 
    RigidBody.Length       *= RawRigidBody.BodyScale.GetZ(); 
    
    // Setup default flags
    RigidBody.Flags = geom::rigid_body::FLAG_WORLD_COLLISION;
    
    // Turn off collision for rope
    if( x_stristr( RawRigidBody.Name, "RB_Rope" ) == RawRigidBody.Name )
        RigidBody.Flags &= ~geom::rigid_body::FLAG_WORLD_COLLISION;
        
    // Setup type    
    if( x_stricmp( RawRigidBody.Type, "Sphere" ) == 0 )
    {
        RigidBody.Type = geom::rigid_body::TYPE_SPHERE;
    }                
    else if( x_stricmp( RawRigidBody.Type, "Cylinder" ) == 0 )
    {
        RigidBody.Type = geom::rigid_body::TYPE_CYLINDER;
    }        
    else if( x_stricmp( RawRigidBody.Type, "Box" ) == 0 )
    {    
        RigidBody.Type = geom::rigid_body::TYPE_BOX;
    }        
    else        
    {    
        RigidBody.Type = geom::rigid_body::TYPE_SPHERE;
    }        
    
    // Setup default collision mask to all on
    RigidBody.CollisionMask = 0xFFFFFFFF;
    
    // Setup DOFs
    for( s32 i = 0; i < 6; i++ )
    {
        geom::rigid_body::dof&           DOF    = RigidBody.DOF[i];
        const rawmesh2::rigid_body::dof& RawDOF = RawRigidBody.DOF[i];
        
        DOF.Flags = 0;
        
        if( RawDOF.bActive )
            DOF.Flags |= geom::rigid_body::dof::FLAG_ACTIVE;
        
        if( RawDOF.bLimited )
            DOF.Flags |= geom::rigid_body::dof::FLAG_LIMITED;
        
        DOF.Min = RawDOF.Min;
        DOF.Max = RawDOF.Max;
    }
    
}

//=============================================================================

static 
object_to_object_mapping g_RigidBodyDisableCollisionMapping[] = 
{
    { "Head",           "Pelvis"        },
    { "Head",           "L_Thigh"       },
    { "Head",           "R_Thigh"       },
    { "Head",           "L_Foot"        },
    { "Head",           "R_Foot"        },
    { "Head",           "L_Calf"        },
    { "Head",           "R_Calf"        },
                                   
    { "Torso",          "L_Thigh"       },
    { "Torso",          "R_Thigh"       },
    { "Torso",          "L_Calf"        },
    { "Torso",          "R_Calf"        },
    { "Torso",          "L_Foot"        },
    { "Torso",          "R_Foot"        },
                                   
    { "Pelvis",         "L_Calf"        },
    { "Pelvis",         "R_Calf"        },
    { "Pelvis",         "L_Foot"        },
    { "Pelvis",         "R_Foot"        },
                                   
    { "L_Thigh",        "L_Foot"        },
    { "R_Thigh",        "R_Foot"        },
};

//=============================================================================

void geom_compiler::BuildRigidBodies(       geom&     Geom, 
                                      const rawmesh2& GeomRawMesh,
                                      const rawmesh2& PhysicsRawMesh )
{
    s32 i, j, nMaps;
    
    // Lookup # to create
    s32 nRigidBodies = PhysicsRawMesh.m_nRigidBodies;
    if( !nRigidBodies )
        return;
    
    // Create rigid bodies
    Geom.m_nRigidBodies = nRigidBodies;
    Geom.m_pRigidBodies = new geom::rigid_body[nRigidBodies];
    if( Geom.m_pRigidBodies == NULL )
        x_throw( "Out of memory" );

    // Setup rigid bodies
    for( i = 0 ; i < nRigidBodies; i++ )
        BuildRigidBody( Geom.m_pRigidBodies[i], PhysicsRawMesh.m_pRigidBodies[i] );

    // Setup rigid body -> bone mapping
    for( i = 0 ; i < nRigidBodies; i++ )
    {
        // Loop through maps
        nMaps = sizeof(g_BoneToRigidBodyMapping) / sizeof(object_to_object_mapping);
        for( j = 0; j < nMaps; j++ )
        {
            // Found rigid body in mapping table?
            if( x_stristr( PhysicsRawMesh.m_pRigidBodies[i].Name, g_BoneToRigidBodyMapping[j].pObjectB ) )
            {
                // Lookup bone index if it hasn't been mapped yet
                geom::rigid_body& Body = Geom.m_pRigidBodies[i];
                if( Body.iBone == -1 )
                    Body.iBone = GeomRawMesh.GetBoneIDFromName( g_BoneToRigidBodyMapping[j].pObjectA, TRUE );
            }
        }
    }
        
    // Check all RigidBody -> Bone mappings
    for( i = 0; i < nRigidBodies; i++ )
    {
        // Lookup bone
        geom::rigid_body& Body = Geom.m_pRigidBodies[i];
        if( Body.iBone == -1 )
        {
            // Try map bone to matching rigid body name
            const char* pBodyName = PhysicsRawMesh.m_pRigidBodies[i].Name;
            while( ( Body.iBone == -1 ) && ( pBodyName[0] ) )
            {
                // Try lookup bone
                Body.iBone = GeomRawMesh.GetBoneIDFromName( pBodyName, TRUE );

                // Goto next character in rigid body name
                pBodyName++;
            }

            // Still not mapped?
            if( Body.iBone == -1 )            
            {
                // Map to root bone
                Body.iBone = 0;

                // Show warning
                ReportWarning( xfs("RigidBody [%s] -> Bone mapping not found, defaulting to root bone.", PhysicsRawMesh.m_pRigidBodies[i].Name ) );
            }
        }
    }
            
    // Turn off collision between parent and child rigid bodies
    for( i = 0; i < nRigidBodies; i++ )
    {
        // Does body have parent?
        s32 iParentBody = Geom.m_pRigidBodies[i].iParentBody;
        if( iParentBody != -1 )
        {
            // Exclude each body from each others collision mask
            Geom.m_pRigidBodies[i].CollisionMask           &= ~( 1 << iParentBody );
            Geom.m_pRigidBodies[iParentBody].CollisionMask &= ~( 1 << i );
        }
    }
    
    // Turn off collision between specified rigid bodies
    nMaps = sizeof(g_RigidBodyDisableCollisionMapping) / sizeof(object_to_object_mapping);
    for( i = 0; i < nMaps; i++ )
    {
        // Lookup rigid body indices
        s32 iBodyA = PhysicsRawMesh.GetRigidBodyIDFromName( g_RigidBodyDisableCollisionMapping[i].pObjectA, TRUE );
        s32 iBodyB = PhysicsRawMesh.GetRigidBodyIDFromName( g_RigidBodyDisableCollisionMapping[i].pObjectB, TRUE );
        
        // If both found, disable collision between them
        if( ( iBodyA != -1 ) && ( iBodyB != -1 ) )
        {
            Geom.m_pRigidBodies[ iBodyA ].CollisionMask &= ~( 1 << iBodyB );
            Geom.m_pRigidBodies[ iBodyB ].CollisionMask &= ~( 1 << iBodyA );
        }
    }
}

//=============================================================================

void geom_compiler::BuildSettings( geom& Geom, const char* pSettingsFile, const rawmesh2& GeomRawMesh )
{
    // Clear bone masks
    Geom.m_nBoneMasks = 0;
    Geom.m_pBoneMasks = NULL;

    // Clear properties
    Geom.m_nPropertySections = 0;
    Geom.m_pPropertySections = NULL;
    Geom.m_nProperties       = 0;
    Geom.m_pProperties       = NULL;
    
    // No file specified?
    if( !pSettingsFile[0] )
        return;

    // Show info
    if( g_Verbose )
        x_printf( "Parsing settings file [%s]\n", pSettingsFile );
    
    // Try load raw settings
    raw_settings RawSettings;
    if( !RawSettings.Load( pSettingsFile ) )
        return;

    // Compile bone masks?
    if( RawSettings.m_BoneMasks.GetCount() )
    {
        // Show info
        x_printf( "%30s - %10d Bone masks\n", "Compiling...", RawSettings.m_BoneMasks.GetCount() );
        
        // Allocate
        Geom.m_nBoneMasks = RawSettings.m_BoneMasks.GetCount(); 
        Geom.m_pBoneMasks = new geom::bone_masks[ Geom.m_nBoneMasks ];
        if( Geom.m_pBoneMasks == NULL )
            x_throw( "Out of memory" );

        // Compile bone masks
        for( s32 iBoneMasks = 0; iBoneMasks < RawSettings.m_BoneMasks.GetCount(); iBoneMasks++ )
        {
            // Lookup source + dest
            const raw_settings::bone_masks& SrcBoneMasks = RawSettings.m_BoneMasks[ iBoneMasks ];
            geom::bone_masks&     DstBoneMasks = Geom.m_pBoneMasks[ iBoneMasks ];

            // Setup bone masks
            DstBoneMasks.NameOffset = m_Dictionary.Add( SrcBoneMasks.Name );
            DstBoneMasks.nBones     = 0;
            for( s32 iWeight = 0; iWeight < MAX_ANIM_BONES; iWeight++ )
                DstBoneMasks.Weights[iWeight] = 0.0f;

            // Setup mask values for specified bones            
            for( s32 iMask = 0; iMask < SrcBoneMasks.Masks.GetCount(); iMask++ )
            {
                // Lookup source
                const raw_settings::bone_masks::mask& SrcMask = SrcBoneMasks.Masks[iMask];
                
                // Setup
                s32 iBone = GeomRawMesh.GetBoneIDFromName( (const char*)SrcMask.BoneName, TRUE );
                if( iBone != -1 )
                {
                    DstBoneMasks.Weights[iBone] = SrcMask.Weight; 
                    DstBoneMasks.nBones = x_max( DstBoneMasks.nBones, iBone+1 );
                }
                else
                {
                    x_printf( xfs( "Warning: Bone [%s] not found in geometry - mask wil be ignored.\nReferenced in settings file [%s]\n\n", (const char*)SrcMask.BoneName, pSettingsFile ) );
                }                    
            }
        }
        
        // Show compiled info?
        if( g_Verbose )
        {
            // Loop over all bone masks
            x_printf("\nBone Masks:%d\n", Geom.m_nBoneMasks );
            for( s32 i = 0; i < Geom.m_nBoneMasks; i++ )
            {
                // Show group info
                const geom::bone_masks& BoneMasks = Geom.m_pBoneMasks[ i ];
                x_printf("\n    GroupName:%s nBones:%d\n\n", m_Dictionary.GetString( BoneMasks.NameOffset ), BoneMasks.nBones );

                // Show bones and weights
                for( s32 j = 0; j < GeomRawMesh.m_nBones; j++ )
                    x_printf("        Bone:%s Weight:%f\n", GeomRawMesh.m_pBone[j].Name, BoneMasks.Weights[j] );

                x_printf("\n");
            }
            x_printf("\n");
        }            
    }

    // Compile properties?
    if( RawSettings.m_Properties.GetCount() )
    {
        // Show info
        x_printf( "%30s - %10d Properties\n", "Compiling...", RawSettings.m_Properties.GetCount() );
        
        // Allocate property sections
        Geom.m_nPropertySections = RawSettings.m_PropertySections.GetCount();
        Geom.m_pPropertySections = new geom::property_section[ Geom.m_nPropertySections ];
        if( Geom.m_pPropertySections == NULL )
            x_throw( "Out of memory" );

        // Allocate properties
        Geom.m_nProperties = RawSettings.m_Properties.GetCount();
        Geom.m_pProperties = new geom::property[ Geom.m_nProperties ];
        if( Geom.m_pProperties == NULL )
            x_throw( "Out of memory" );
            
        // Compile property sections
        for( s32 iSection = 0; iSection < RawSettings.m_PropertySections.GetCount(); iSection++ )
        {
            // Lookup source + dest
            const raw_settings::property_section& SrcSection = RawSettings.m_PropertySections[iSection];
            geom::property_section&               DstSection = Geom.m_pPropertySections[iSection];
            
            // Setup dest
            DstSection.NameOffset  = m_Dictionary.Add( SrcSection.Name );
            DstSection.iProperty   = SrcSection.iProperty;
            DstSection.nProperties = SrcSection.nProperties;
        }

        // Compile properties
        for( s32 iProperty = 0; iProperty < RawSettings.m_Properties.GetCount(); iProperty++ )
        {
            // Lookup source + dest
            const raw_settings::property& SrcProperty = RawSettings.m_Properties[iProperty];
            geom::property&               DstProperty = Geom.m_pProperties[iProperty];

            // Setup
            DstProperty.NameOffset = m_Dictionary.Add( SrcProperty.Name );
            if( SrcProperty.Type == "FLOAT" )
            {
                DstProperty.Type        = geom::property::TYPE_FLOAT;
                DstProperty.Value.Float = x_atof( SrcProperty.Value );
            }                
            else if( SrcProperty.Type == "INTEGER" )
            {
                DstProperty.Type          = geom::property::TYPE_INTEGER;
                DstProperty.Value.Integer = x_atoi( SrcProperty.Value );
            }                
            else if( SrcProperty.Type == "ANGLE" )
            {
                DstProperty.Type        = geom::property::TYPE_ANGLE;
                DstProperty.Value.Float = DEG_TO_RAD( x_atof( SrcProperty.Value ) );
            }                
            else if( SrcProperty.Type == "STRING" )
            {
                DstProperty.Type        = geom::property::TYPE_STRING;
                DstProperty.Value.StringOffset = m_Dictionary.Add( SrcProperty.Value );
            }
            else
            {
                x_throw( xfs( "Unknow property type %s\nReferenced in settings file [%s]\n\n", SrcProperty.Type, pSettingsFile ) );
            }
        }            
        
        // Show compiled info?
        if( g_Verbose )
        {
            x_printf("\nProperties:%d\n", Geom.m_nProperties );
            for( s32 iSection = 0; iSection < Geom.m_nPropertySections; iSection++ )
            {
                // Lookup section info
                geom::property_section& Section = Geom.m_pPropertySections[iSection];
                const char* pSection = m_Dictionary.GetString( Section.NameOffset );
                x_printf("\n   Section:%s nProperties:%d\n\n", m_Dictionary.GetString( Section.NameOffset ), Section.nProperties );
                
                // Loop through all properties
                for( iProperty = 0; iProperty < Section.nProperties; iProperty++ )
                {
                    // Lookup property info
                    const geom::property& Property = Geom.m_pProperties[ Section.iProperty + iProperty ];
                    const char* pName = m_Dictionary.GetString( Property.NameOffset );
                    xstring Type;
                    xstring Value;
                    switch( Property.Type )
                    {
                    case geom::property::TYPE_FLOAT:
                        Type = "FLOAT";
                        Value.Format( "%f", Property.Value.Float );
                        break;
                    case geom::property::TYPE_ANGLE:
                        Type = "ANGLE";
                        Value.Format( "%f", RAD_TO_DEG( Property.Value.Angle ) );
                        break;
                    case geom::property::TYPE_INTEGER:
                        Type = "INTEGER";
                        Value.Format( "%d", Property.Value.Integer );
                        break;
                    case geom::property::TYPE_STRING:
                        Type  = "STRING";
                        Value = m_Dictionary.GetString( Property.Value.StringOffset );
                        break;
                    default:
                        x_throw( xfs( "Unknow property type %s\nReferenced in settings file [%s]\n\n", Type, pSettingsFile ) );
                        break;
                    }
                    
                    // Show info
                    x_printf( "       Name:%30s    Type:%6s    Value:%8s\n", pName, (const char*)Type, (const char*)Value );
                }
                x_printf("\n");
            }                                              
        }
    }
}

//=============================================================================

xbool geom_compiler::IsSameMaterial( const rawmesh2::material& RawMatA,
                                     const rawmesh2::material& RawMatB,
                                     const rawmesh2&           RawMesh )
{
    // type check
    if( RawMatA.Type != RawMatB.Type )  return FALSE;

    // diffuse texture check
    s32 iDiffA = RawMatA.Map[Max_Diffuse1].iTexture;
    s32 iDiffB = RawMatB.Map[Max_Diffuse1].iTexture;
    s32 iEnvA  = RawMatA.Map[Max_Environment].iTexture;
    s32 iEnvB  = RawMatB.Map[Max_Environment].iTexture;
    s32 iDetA  = RawMatA.Map[Max_DetailMap].iTexture;
    s32 iDetB  = RawMatB.Map[Max_DetailMap].iTexture;
    s32 iPunA  = RawMatA.Map[Max_PunchThrough].iTexture;
    s32 iPunB  = RawMatB.Map[Max_PunchThrough].iTexture;
    const char* pDiffNameA = (iDiffA>=0) ? RawMesh.m_pTexture[iDiffA].FileName : "";
    const char* pDiffNameB = (iDiffB>=0) ? RawMesh.m_pTexture[iDiffB].FileName : "";
    const char* pEnvNameA  = (iEnvA >=0) ? RawMesh.m_pTexture[iEnvA].FileName  : "";
    const char* pEnvNameB  = (iEnvB >=0) ? RawMesh.m_pTexture[iEnvB].FileName  : "";
    const char* pDetNameA  = (iDetA >=0) ? RawMesh.m_pTexture[iDetA].FileName  : "";
    const char* pDetNameB  = (iDetB >=0) ? RawMesh.m_pTexture[iDetB].FileName  : "";
    const char* pPunNameA  = (iPunA >=0) ? RawMesh.m_pTexture[iPunA].FileName  : "";
    const char* pPunNameB  = (iPunB >=0) ? RawMesh.m_pTexture[iPunB].FileName  : "";
    if( x_strcmp( pDiffNameA, pDiffNameB ) ) return FALSE;
    if( x_strcmp( pEnvNameA,  pEnvNameB  ) ) return FALSE;
    if( x_strcmp( pDetNameA,  pDetNameB  ) ) return FALSE;
    if( x_strcmp( pPunNameA,  pPunNameB  ) ) return FALSE;
        
    // parameter check
    if( RawMatA.bTwoSided                          != RawMatB.bTwoSided                          ) return FALSE;
    if( RawMatA.Constants[DETAIL_SCALE].Current[0] != RawMatB.Constants[DETAIL_SCALE].Current[0] ) return FALSE;
    if( RawMatA.Constants[FIXED_ALPHA].Current[0]  != RawMatB.Constants[FIXED_ALPHA].Current[0]  ) return FALSE;
    if( RawMatA.Constants[ENV_TYPE].Current[0]     != RawMatB.Constants[ENV_TYPE].Current[0]     ) return FALSE;
    if( RawMatA.Constants[FORCE_ZFILL].Current[0]  != RawMatB.Constants[FORCE_ZFILL].Current[0]  ) return FALSE;
    if( RawMatA.Constants[USE_DIFFUSE].Current[0]  != RawMatB.Constants[USE_DIFFUSE].Current[0]  ) return FALSE;
    if( RawMatA.Constants[ENV_BLEND].Current[0]    != RawMatB.Constants[ENV_BLEND].Current[0]    ) return FALSE;

    // uv anim check
    const rawmesh2::param_pkg& ParamA = RawMatA.Map[Max_Diffuse1].UVTranslation;
    const rawmesh2::param_pkg& ParamB = RawMatB.Map[Max_Diffuse1].UVTranslation;
    if( ParamA.nKeys         != ParamB.nKeys         ) return FALSE;
    if( ParamA.nParamsPerKey != ParamB.nParamsPerKey ) return FALSE;
    for( s32 i = 0; i < ParamA.nKeys * ParamA.nParamsPerKey; i+= ParamA.nParamsPerKey )
    {
        s32 iKeyA = RawMatA.iFirstKey + i;
        s32 iKeyB = RawMatB.iFirstKey + i;
        if( RawMesh.m_pParamKey[iKeyA + 0] != RawMesh.m_pParamKey[iKeyB + 0] ) return FALSE;
        if( RawMesh.m_pParamKey[iKeyA + 1] != RawMesh.m_pParamKey[iKeyB + 1] ) return FALSE;
    }

    return TRUE;
}

//=============================================================================

void geom_compiler::BuildBasicStruct( geom& Geom, const rawmesh2& RawMesh, mesh& Mesh, xbool IsRigid )
{
    s32             i,j;
    xarray<s32>     MatMap;
    xarray<xhandle> hMatDList;

    // Load specified physics raw mesh?
    rawmesh2 PRawMesh; 
    xbool    bUsePhysicRawMesh = FALSE;
    if( m_PhysicsMatx[0] )
        bUsePhysicRawMesh = PRawMesh.Load( m_PhysicsMatx );
        
    // Lookup physics raw mesh to use
    const rawmesh2& PhysicsRawMesh = bUsePhysicRawMesh ? PRawMesh : RawMesh;        

    // Show info?
    if( g_Verbose )
        PhysicsRawMesh.PrintRigidBodies();

    // Output info    
    if( RawMesh.m_nBones )
        x_printf( "%30s - %10d Bones\n", "Compiling...", RawMesh.m_nBones );
    
    if( PhysicsRawMesh.m_nRigidBodies )        
        x_printf( "%30s - %10d Rigid bodies\n", "Compiling...", PhysicsRawMesh.m_nRigidBodies );

    // Build bones
    BuildBones( Geom, RawMesh, PhysicsRawMesh );

    // Build rigid bodies
    BuildRigidBodies( Geom, RawMesh, PhysicsRawMesh );    

    // Build bone mask tables etc
    BuildSettings( Geom, m_SettingsFile, RawMesh );
    
    // Usage map for the materials
    s8 MatUsed[ MaxMeshes ][ MaxMaterials ];

    x_memset( MatUsed, 0, sizeof( MatUsed ) );

    //
    // Determine the materials that each mesh uses
    //

    if( RawMesh.m_nMaterials > MaxMaterials )
    {
        ThrowError( "Too many materials in file" );
    }

    if( RawMesh.m_nMaterials <= 0 )
    {
        ThrowError( "No materials defined for this MATX... Compile will fail" );
    }
    
    s32 MaterialsUsed = 0;
    
    for( i=0; i<RawMesh.m_nFacets; i++ )
    {
        s32 iMesh     = RawMesh.m_pFacet[i].iMesh;
        s32 iMaterial = RawMesh.m_pFacet[i].iMaterial;

        if( (iMesh < 0) || (iMesh >= MaxMeshes) )
            x_throw( xfs( "Invalid iMesh %d", iMesh ) );
        
        if( (iMaterial < 0) || (iMaterial >= MaxMaterials) )
            x_throw( xfs( "Invalid iMaterial %d", iMaterial ) );
        
        MatUsed[ iMesh ][ iMaterial ] = 1;
        
        if( iMaterial > MaterialsUsed )
            MaterialsUsed = iMaterial;
    }

    MaterialsUsed++;
    if( MaterialsUsed > RawMesh.m_nMaterials )
    {
        ThrowError( xfs( "Invalid material was used %d (max %d)", MaterialsUsed, RawMesh.m_nMaterials ) );
    }

    //
    // Find which materials are used and add them to list
    //
    
    for( i=0; i<RawMesh.m_nMaterials; i++ )
    {
        // First lets make sure that this material is used in the mesh
        for( j=0; j<RawMesh.m_nFacets; j++ )
        {
            if( RawMesh.m_pFacet[j].iMaterial == i )
                break;
        }
        
        // If it is used then...
        if( j != RawMesh.m_nFacets )
        {            
            // Search to see if the material already exists
            for( j=0; j<Mesh.Material.GetCount(); j++ )
            {
                if( IsSameMaterial( Mesh.Material[j].pRawMesh->m_pMaterial[ Mesh.Material[j].iRawMaterial ],
                                    RawMesh.m_pMaterial[i],
                                    RawMesh ) )
                {
                    break;
                }
            }        

            // Add new material if we didn't find it
            if( j == Mesh.Material.GetCount() ) 
            {
                material& Mat    = Mesh.Material.Append();
                Mat.pRawMesh     = &RawMesh;
                Mat.iRawMaterial = i;

                // make sure we have a diffuse texture
                rawmesh2::material& RawMat = RawMesh.m_pMaterial[Mat.iRawMaterial];

                if( RawMat.Map[Max_Diffuse1].iTexture == -1 )
                    ThrowError( xfs( "Material does not have a texture <%s>", RawMat.Name ) );

                // make sure the diffuse texture is not a now-defunct ifl file
                rawmesh2::texture&  RawTex = RawMesh.m_pTexture[RawMat.Map[Max_Diffuse1].iTexture];
                char                Ext[X_MAX_EXT];
                x_splitpath( RawTex.FileName, NULL, NULL, NULL, Ext );
                if( !x_stricmp( Ext, ".ifl" ) )
                {
                    ThrowError( xfs( "IFL files are no longer supported <%s>", RawTex.FileName ) );
                }

                // load the texinfo file
                if( !Mat.TexInfo.Load( RawTex.FileName ) )
                {
                    ReportWarning( "Problem with texinfo" );
                }
            }
            
            // Add material map entry
            MatMap.Append() = j;
        }
        else
        {
            // Material is not used
            MatMap.Append() = -1;
        }
    }

    if( g_Verbose )
        x_DebugMsg( "%d Materials\n", Mesh.Material.GetCount() );

    //
    // Build the submeshes and create display lists for the materials
    //

    if( IsRigid == TRUE )
    {
        for( s32 n=0; n < RawMesh.m_nSubMeshs; n++ )
        {
            sub_mesh& SubMesh = Mesh.SubMesh.Add();
            
            SubMesh.pRawMesh    = &RawMesh;
            SubMesh.pRawSubMesh = &RawMesh.m_pSubMesh[n] ;
        
            char* pName = RawMesh.m_pSubMesh[n].Name;
            s32   Size  = sizeof( SubMesh.Name );
            
            if( x_strlen( pName ) > (Size-1) )
                ThrowError( xfs( "Mesh name [%s] is longer than %d characters", pName, Size-1 ) );
            
            x_strcpy( SubMesh.Name, pName );
        
            // Create a display list for each material used in the SubMesh
            for( i=0; i<RawMesh.m_nMaterials; i++ )
            {
                // Is this material used by the SubMesh?
                if( MatUsed[n][i] && (MatMap[i] != -1) )
                {
                    xharray<dlist> lDList;

                    // Create a new display list for each bone
                    for( j=0; j<RawMesh.m_nBones; j++ )
                    {
                        // Create new dlist
                        dlist& DList = lDList.Add() ;

                        // *INEV* *SB* - Pre-allocate tri indices
                        DList.lTri.SetCapacity(RawMesh.m_nFacets) ;
                    }
                    
                    // Assign facets to the correct display list
                    for( j=0; j<RawMesh.m_nFacets; j++ )
                    {
                        rawmesh2::facet&  Facet   = RawMesh.m_pFacet[j];
                        rawmesh2::vertex* pVertex = RawMesh.m_pVertex;
                        
                        // Get the bone used by the first vertex in the facet
                        s32 Vert0 = Facet.iVertex[0];
                        s32 iBone = pVertex[ Vert0 ].Weight[0].iBone;
                        
                        if( iBone >= lDList.GetCount() )
                            ThrowError( "Bone index is out of range" );
                        
                        /*
                        // Check all vertices in the facet use the same bone
                        for( s32 k=1; k<Facet.nVertices; k++ )
                        {
                            s32 VertI = Facet.iVertex[k];
                            s32 Index = pVertex[ VertI ].Weight[0].iBone;
                            
                            if( Index != iBone )
                            {
                                x_throw( "Vertices of the face do not use the same bone" );
                            }
                        }
                        */
                        
                        // Check if facet uses this Material and is in the same mesh
                        if( (RawMesh.m_pFacet[j].iMesh     == n) &&
                            (RawMesh.m_pFacet[j].iMaterial == i) )
                        {
                            // Add the facet to the correct bone DList
                            lDList[ iBone ].lTri.Append( j );
                        }
                    }
                    
                    // Copy the new display lists into the SubMesh (if they were used)
                    for( j=0; j<lDList.GetCount(); j++ )
                    {
                        dlist& BoneDList = lDList[j];
                    
                        // Check if any facets were added to this display list
                        if( BoneDList.lTri.GetCount() > 0 )
                        {
                            // Create a new display list in the SubMesh
                            dlist& DList    = SubMesh.lDList.Add();
                            DList.iMaterial = MatMap[i];
                            DList.iBone     = j;

                            // Copy the facet indices over to the SubMesh display list

                            // *INEV* *SB* - FAST COPY!!!
                            DList.lTri = BoneDList.lTri ;
                            
                            // Copy the facet indices over to the SubMesh display list
                            //for( s32 k=0; k<BoneDList.lTri.GetCount(); k++ )
                            //{
                                //DList.lTri.Append() = BoneDList.lTri[k];
                            //}
                        }
                    }
                }
            }
        }
    }
    else
    {
        for( s32 n=0; n < RawMesh.m_nSubMeshs; n++ )
        {
            xhandle   hSubMesh;
            sub_mesh& SubMesh = Mesh.SubMesh.Add( hSubMesh );
            
            SubMesh.pRawMesh    = &RawMesh;
            SubMesh.pRawSubMesh = &RawMesh.m_pSubMesh[n] ;
        
            char* pName = RawMesh.m_pSubMesh[n].Name;
            s32   Size  = sizeof( SubMesh.Name );
            
            if( x_strlen( pName ) > (Size-1) )
                ThrowError( xfs( "Mesh name [%s] is longer than %d characters", pName, Size-1 ) );
            
            x_strcpy( SubMesh.Name, pName );
        
            // Create a display list for each material used in the SubMesh
            for( i=0; i<RawMesh.m_nMaterials; i++ )
            {
                // Is this material used by the SubMesh?
                if( MatUsed[n][i] && (MatMap[i] != -1) )
                {
                    // Create a new dlist for this object
                    xhandle Handle;
                    dlist&  DList    = SubMesh.lDList.Add( Handle );

                    // *INEV* *SB* - Pre-allocate tris!
                    DList.lTri.SetCapacity(RawMesh.m_nFacets) ;

                    DList.iMaterial  = MatMap[ i ];
                    DList.iBone      = -1;

                    // Assign facets to the correct display list
                    for( j=0; j<RawMesh.m_nFacets; j++ )
                    {
                        rawmesh2::facet&  Facet   = RawMesh.m_pFacet[j];
                                                
                        // Check if facet uses this Material and is in the same mesh
                        if( (RawMesh.m_pFacet[j].iMesh     == n) &&
                            (RawMesh.m_pFacet[j].iMaterial == i) )
                        {
                            // Add the facet to the correct bone DList
                            DList.lTri.Append( j );
                        }
                    }
                    
                    if( DList.lTri.GetCount() == 0 )
                        SubMesh.lDList.DeleteByHandle( Handle );
                    
                }
            }

            if( SubMesh.lDList.GetCount() < 0 )
                Mesh.SubMesh.DeleteByHandle( hSubMesh );
        }
    }
    
    if( g_Verbose )
    {
        for( i=0; i<Mesh.SubMesh.GetCount(); i++ )
        {
            sub_mesh& SubMesh = Mesh.SubMesh[i];
        
            x_DebugMsg( "Model %s has %d dlists with materials:\n", SubMesh.Name, SubMesh.lDList.GetCount() );
            for( j=0; j<SubMesh.lDList.GetCount(); j++ )
            {
                x_DebugMsg( "    mat=%d tri=%d\n", SubMesh.lDList[j].iMaterial, SubMesh.lDList[j].lTri.GetCount() );
            }
        }
    }
}

//=============================================================================

void geom_compiler::Export( const char* pFileName, comp_type Type, const char* pTexturePath )
{
    // reset the dictionary
    m_Dictionary.Reset();

    // Keep source path
    if (pTexturePath)
        x_strcpy(m_TexturePath, pTexturePath) ;

    // 
    switch( Type )
    {
        case TYPE_RIGID:
            ExportRigidGeom( pFileName );
            break;
        
        case TYPE_SKIN:
            ExportSkinGeom( pFileName );
            break;

        default:
            x_throw( "Unknown compiler type" );
            break;
    }
}

//=============================================================================

void geom_compiler::RemoveUnusedVMeshes( rawmesh2& RawMesh )
{
    s32 i, j, k, t;

    //
    // nuke any submeshes that aren't contained in a virtual mesh
    //
    if( m_pGeomRscDesc->GetVirtualMeshCount() )
    {
        // figure out which meshes are included in the vmeshes
        xbool* pMeshUsed = new xbool[RawMesh.m_nSubMeshs];
        for( i = 0; i < RawMesh.m_nSubMeshs; i++ )
        {
            pMeshUsed[i] = FALSE;
            for( j = 0; j < m_pGeomRscDesc->GetVirtualMeshCount() && (pMeshUsed[i] == FALSE); j++ )
            {
                const geom_rsc_desc::virtual_mesh& VMesh = m_pGeomRscDesc->GetVirtualMesh( j );
                for( k = 0 ; k < VMesh.LODs.GetCount() && (pMeshUsed[i] == FALSE); k++ )
                {
                    const geom_rsc_desc::lod_info& LODInfo = VMesh.LODs[k];
                    for( t = 0; t < LODInfo.nMeshes && (pMeshUsed[i] == FALSE); t++ )
                    {
                        if( !x_strcmp( LODInfo.MeshName[t], RawMesh.m_pSubMesh[i].Name ) )
                        {
                            pMeshUsed[i] = TRUE;
                        }
                    }
                }
            }
        }

        // Nuke all the facets that represent those meshes
        for( i = 0; i < RawMesh.m_nSubMeshs; i++ )
        {
            if( pMeshUsed[i] )
                continue;

            for( j = 0; j < RawMesh.m_nFacets; j++ )
            {
                if( RawMesh.m_pFacet[j].iMesh == i )
                {
                    RawMesh.m_pFacet[j] = RawMesh.m_pFacet[ RawMesh.m_nFacets-1 ];
                    j--;
                    RawMesh.m_nFacets--;
                }
            }
        }

        // clean up
        delete []pMeshUsed;
    }
}

//=============================================================================

void geom_compiler::ExportRigidGeom( const char* pFileName )
{
    // load the resource
    LoadResource( pFileName, FALSE );

    //
    // Load the raw-mesh
    //

    rigid_geom  RigidGeom;
    rawmesh2    RawMesh;
    mesh        Mesh;
    s32         i, j;
    rawmesh2    RawMeshFC;

    RawMesh.Load( m_pGeomRscDesc->GetMatxFileName() );
    SetUserInfo( RawMesh.m_UserName, RawMesh.m_ComputerName );
    SetExportDate( RawMesh.m_ExportDate[0], RawMesh.m_ExportDate[1], RawMesh.m_ExportDate[2] );
    RemoveUnusedVMeshes( RawMesh );

    if( RawMesh.m_nBones > 1 )
    {
        rawanim     RawAnim;
        xarray<s32> BadSubMesh;
        
        RawAnim.Load( m_pGeomRscDesc->GetMatxFileName() );

        //
        // Nuke all the submeshes that are bones.
        //
        BadSubMesh.Clear();
        for( j=0; j<RawMesh.m_nSubMeshs; j++ )
        {            
            if( x_stristr( RawMesh.m_pSubMesh[j].Name, "Human_" ) )
                BadSubMesh.Append() = j;
        }
        
        // Nuke all the facets that represent those bones.
        for( j=0; j<RawMesh.m_nFacets; j++ )
        {
            for( s32 t=0; t<BadSubMesh.GetCount(); t++ )
            {
                if( BadSubMesh[t] == RawMesh.m_pFacet[j].iMesh )
                    break;
            }
        
            if( t < BadSubMesh.GetCount() )
            {
                RawMesh.m_pFacet[j] = RawMesh.m_pFacet[ RawMesh.m_nFacets-1 ];
                j--;
                RawMesh.m_nFacets--;
            }
        }            
    
        RawMesh.CleanWeights( 1, 0.00001f );
        RawMesh.CleanMesh();
        RawMesh.SortFacetsByMaterialAndBone();
        
        // Do remapping of the skeleton
        RawAnim.DeleteDummyBones();
        RawMesh.ApplyNewSkeleton( RawAnim );
        
        // Show hierarchy?
        if( g_Verbose )
            RawAnim.PrintHierarchy();
    }
    else
    {
        RawMesh.CleanMesh();
        RawMesh.SortFacetsByMaterial();
    }


    BuildBasicStruct( RigidGeom, RawMesh, Mesh, TRUE );

    //
    // Check whether we have to load the fast collision data
    //
    if( m_FastCollision[0] )
    {
        RawMeshFC.Load( m_FastCollision );

        // Make sure to simplify the mesh
        for( s32 j=0; j<RawMeshFC.m_nVertices; j++ )
        {
            RawMeshFC.m_pVertex[j].nColors  = 0;
            RawMeshFC.m_pVertex[j].nUVs     = 0;
            RawMeshFC.m_pVertex[j].nNormals = 0;
        }

        RawMeshFC.CleanWeights( 1, 0.00001f );
        RawMeshFC.CleanMesh();

        CompileLowCollision( RigidGeom, RawMeshFC, RawMesh, m_FastCollision );
    }
    else
    {
        CompileLowCollisionFromBBox( RigidGeom, RawMesh );
    }

    // Export to the correct platform
    for( i=0; i<m_PlatInfo.GetCount(); i++ )
    {
        // Make sure directionary is clear so we don't get dupes
        m_Dictionary.Reset();
        
        switch( m_PlatInfo[i].Platform )
        {
            case PLATFORM_XBOX:
                ExportRigidGeomXbox( Mesh, RigidGeom, pFileName );
                break;

            case PLATFORM_PS2 :
                ExportRigidGeomPS2( Mesh, RigidGeom, pFileName );
                break;
            
            case PLATFORM_PC :
                ExportRigidGeomPC( Mesh, RigidGeom, pFileName );
                break;
        
            default :
                ASSERT( 0 );
                break;
        }
    }

    // print out a material and texture listing
    if( g_Verbose )
    {
        PrintSummary( RigidGeom );

        // print out a summary of the bboxes
        bbox BBox = RigidGeom.m_Collision.BBox;
        x_printf( "\n--BBox sizes-----------------------------------\n" );
        x_printf( "Collision:\n" );
        x_printf( "(%3.3f, %3.3f, %3.3f)-(%3.3f, %3.3f, %3.3f)\n",
            BBox.Min.GetX(), BBox.Min.GetY(), BBox.Min.GetZ(),
            BBox.Max.GetX(), BBox.Max.GetY(), BBox.Max.GetZ() );
        x_printf( "Extent: (%3.3f, %3.3f, %3.3f)\n",
            BBox.Max.GetX() - BBox.Min.GetX(),
            BBox.Max.GetY() - BBox.Min.GetY(),
            BBox.Max.GetZ() - BBox.Min.GetZ() );

        BBox = RigidGeom.m_BBox;
        x_printf( "Geom:\n" );
        x_printf( "(%3.3f, %3.3f, %3.3f)-(%3.3f, %3.3f, %3.3f)\n",
            BBox.Min.GetX(), BBox.Min.GetY(), BBox.Min.GetZ(),
            BBox.Max.GetX(), BBox.Max.GetY(), BBox.Max.GetZ() );
        x_printf( "Extent: (%3.3f, %3.3f, %3.3f)\n",
            BBox.Max.GetX() - BBox.Min.GetX(),
            BBox.Max.GetY() - BBox.Min.GetY(),
            BBox.Max.GetZ() - BBox.Min.GetZ() );
    }
}

//=============================================================================

f32 ComputeWorldPixelSize( const vector4* pPos, s16* pUV, s32 nVerts, const xbitmap& Bitmap )
{
    f32 WorldPixelSize = 0.0f;
    s32 nTri           = 0;

    for( s32 i=0; i<nVerts; i++, pPos += 1, pUV += 2 )
    {
        f32 ADC = pPos->GetW();
        if( (reinterpret_cast<u32 &>(ADC) & 0x8000) == 0 )
        {
            vector2 Tex[3];
            vector3 V1;
            vector3 V2;

            // Convert UV's from 12:4 fixed point to floating point
            Tex[0].Set( pUV[-4] / (f32)(1 << 12), pUV[-3] / (f32)(1 << 12) );
            Tex[1].Set( pUV[-2] / (f32)(1 << 12), pUV[-1] / (f32)(1 << 12) );
            Tex[2].Set( pUV[ 0] / (f32)(1 << 12), pUV[ 1] / (f32)(1 << 12) );

            // Compute 2 edge vectors on the tri in texel space
            V1.Set( (Tex[2].X - Tex[0].X) * (f32)Bitmap.GetWidth(),
                    (Tex[2].Y - Tex[0].Y) * (f32)Bitmap.GetHeight(),
                    0.0f );
            V2.Set( (Tex[1].X - Tex[0].X) * (f32)Bitmap.GetWidth(),
                    (Tex[1].Y - Tex[0].Y) * (f32)Bitmap.GetHeight(),
                    0.0f );

            // Compute area of texture space used by tri
            f32 TexArea = v3_Cross( V1, V2 ).Length() * 0.5f;

            // Ensure we have at least 1 pixel square of texture
            if( TexArea < 1.0f )
                continue;

            // Compute 2 edge vectors on the tri
            V1 = pPos[ 0] - pPos[-2];
            V2 = pPos[-1] - pPos[-2];
            
            // Compute area of tri
            f32 TriArea = v3_Cross( V1, V2 ).Length() * 0.5f;

            if( (x_isvalid( TriArea ) == FALSE) ||
                (x_isvalid( TexArea ) == FALSE) )
                continue;

            WorldPixelSize += x_sqrt( TriArea ) / x_sqrt( TexArea );
            nTri++;
        }
    }

    f32 Average = 0.001f;

    if( nTri > 0 )
    {
        Average = WorldPixelSize / nTri;
    }

    return( Average );
}

//=============================================================================

void geom_compiler::ExportRigidGeomPS2( mesh& Mesh, rigid_geom& RigidGeom, const char* pFileName )
{
    s32                         iDList;
    s32                         i,j,k;
    faststrip                   Strip;
    s32                         PlatformID = GetPlatformIndex( PLATFORM_PS2 );
    collision_data::mat_info*   pMatList;

    //
    // Allocate memory for the PS2 display list
    //
    
    s32 nPS2DList = 0;
    for( i=0; i<Mesh.SubMesh.GetCount(); i++ )
    {
        const sub_mesh& SubMesh = Mesh.SubMesh[i];
        nPS2DList += SubMesh.lDList.GetCount();
    }

    rigid_geom::dlist_ps2*   pPS2DList = new rigid_geom::dlist_ps2[ nPS2DList ];
    if( pPS2DList == NULL )
        x_throw( "Out of memory" );

    //
    // Allocate memory for a temporary material list.
    //

    pMatList = new collision_data::mat_info[ nPS2DList ];
    if( !pMatList )
        x_throw( "Out of memory." );

    //
    // Build the display lists
    //   
    for( iDList=i=0; i<Mesh.SubMesh.GetCount(); i++ )
    {
        const sub_mesh& SubMesh = Mesh.SubMesh[i];
        for( j=0; j<SubMesh.lDList.GetCount(); j++ )
        {
            const dlist& DList = SubMesh.lDList[j];

            //
            // First lets get the indices to be ordered properly
            //
            
            Strip.Open( 0, TRUE ); 
            
            for( k=0; k<DList.lTri.GetCount(); k++ )
            {
                Strip.AddTri(
                    SubMesh.pRawMesh->m_pFacet[ DList.lTri[k] ].iVertex[0], 
                    SubMesh.pRawMesh->m_pFacet[ DList.lTri[k] ].iVertex[1], 
                    SubMesh.pRawMesh->m_pFacet[ DList.lTri[k] ].iVertex[2] );
            }
            
            Strip.Close();

            //
            // Note the material.
            //
            material&           Mat     = Mesh.Material[DList.iMaterial];
            ASSERT(Mat.pRawMesh);
            const rawmesh2&     RawMesh = *Mat.pRawMesh;
            rawmesh2::material& RawMat  = RawMesh.m_pMaterial[ Mat.iRawMaterial ];

            pMatList[iDList].SoundType = Mesh.Material[DList.iMaterial].TexInfo.SoundMat;
            pMatList[iDList].Flags     = 0;
            if (RawMat.bTwoSided)
                pMatList[iDList].Flags     |= collision_data::mat_info::FLAG_DOUBLESIDED;

            //
            // Now build the actual display list
            //
            
            rigid_geom::dlist_ps2& PS2DList = pPS2DList[ iDList++ ];

            s32  nIndices = Strip.GetMaxNumIndices( MAX_VU_VERTS );
            s32* pI = (s32*)x_malloc( nIndices * sizeof( s32 ) );
            ASSERT( pI );
            
            nIndices = Strip.GetIndicesPS2( pI, MAX_VU_VERTS );

            PS2DList.nVerts    = nIndices;
            PS2DList.iBone     = DList.iBone;
            PS2DList.pUV       = new s16    [ PS2DList.nVerts * 2 ];
            PS2DList.pNormal   = new s8     [ PS2DList.nVerts * 3 ];
            PS2DList.pPosition = new vector4[ PS2DList.nVerts * 1 ];
            
            if( PS2DList.pUV       == NULL ||
                PS2DList.pNormal   == NULL ||
                PS2DList.pPosition == NULL )
                x_throw( "Out of memory" );

            for( k=0; k<nIndices; k++ )
            {
                s32             Index   = Strip.GetIndex( pI[ k ] );
                rawmesh2::vertex Vertex = SubMesh.pRawMesh->m_pVertex[ Index ];

                PS2DList.pUV[ k*2+0 ]     = (s16)(Vertex.UV[0].X * (1<<12));
                PS2DList.pUV[ k*2+1 ]     = (s16)(Vertex.UV[0].Y * (1<<12));

                PS2DList.pNormal[ k*3+0 ] = (s8)(Vertex.Normal[0].GetX() * (0xff>>1));
                PS2DList.pNormal[ k*3+1 ] = (s8)(Vertex.Normal[0].GetY() * (0xff>>1));
                PS2DList.pNormal[ k*3+2 ] = (s8)(Vertex.Normal[0].GetZ() * (0xff>>1));

                PS2DList.pPosition[ k ]   = Vertex.Position;

                s32 ExtraBits = 0;
                if( Strip.IsIndexNewStrip( pI[k] ) )   ExtraBits |= (1 << ADCBIT);
                if( Strip.IsIndexCCWTri  ( pI[k] ) )   ExtraBits |= (1 << CCWBIT);

                PS2DList.pPosition[k].GetIW() = ExtraBits;
            }

            RecenterUVs( PS2DList.pUV, PS2DList.nVerts );
        }
    }

    //
    // Setup the Geom
    //
    
    geom& Geom       = RigidGeom;

    Geom.m_nTextures = 0;
    Geom.m_pTexture  = NULL;
    Geom.m_Platform  = PLATFORM_PS2;
    
    // Allocate space for all the meshes in the geom
    Geom.m_nMeshes   = Mesh.SubMesh.GetCount();
    Geom.m_pMesh     = new geom::mesh[ Geom.m_nMeshes ];
    
    // Allocate space for all the submeshes in the geom
    Geom.m_nSubMeshes = nPS2DList;
    Geom.m_pSubMesh   = new geom::submesh[ Geom.m_nSubMeshes ];
    
    if( (Geom.m_pMesh == NULL) || (Geom.m_pSubMesh == NULL) )
        x_throw( "Out of memory" );

    //
    // Setup mesh and submesh structures
    //
 
    s32 iColor = 0;

    for( iDList=i=0; i<Mesh.SubMesh.GetCount(); i++ )
    {
        // Setup runtime "geom::mesh" name and bone count
        Geom.m_pMesh[i].NameOffset = (s16)m_Dictionary.Add( Mesh.SubMesh[i].Name );
        Geom.m_pMesh[i].nBones     = Mesh.SubMesh[i].pRawSubMesh->nBones ;


        // 1 submesh per display list
        Geom.m_pMesh[i].nSubMeshes = Mesh.SubMesh[i].lDList.GetCount();
        Geom.m_pMesh[i].iSubMesh   = iDList;
        Geom.m_pMesh[i].nVertices  = 0;
        Geom.m_pMesh[i].nFaces     = 0;
        
        // Set the material and display list index into the submeshes
        for( k=0; k<Mesh.SubMesh[i].lDList.GetCount(); k++, iDList++ )
        {
            // tally up the vert and face counts...the face count
            // is before stripping, and the vert count is after stripping
            // has occured
            const dlist& DList = Mesh.SubMesh[i].lDList[k];
            Geom.m_pMesh[i].nFaces    += DList.lTri.GetCount();
            Geom.m_pMesh[i].nVertices += pPS2DList[iDList].nVerts;

            geom::submesh& SubMesh = Geom.m_pSubMesh[iDList];
            
            SubMesh.iDList    = iDList;
            SubMesh.iMaterial = Mesh.SubMesh[i].lDList[k].iMaterial;

            pPS2DList[ iDList ].iColor = iColor;

            // Keep each Display Lists color table aligned
            s32 nColor = pPS2DList[ iDList ].nVerts;
            iColor += ALIGN_16( sizeof( u16 ) * nColor ) / sizeof( u16 );
        }
    }        

    ASSERT( iDList == nPS2DList );

    // Count up the faces and vertices
    Geom.m_nFaces    = 0;
    Geom.m_nVertices = 0;
    for ( i = 0; i < Geom.m_nMeshes; i++ )
    {
        Geom.m_nFaces    += Geom.m_pMesh[i].nFaces;
        Geom.m_nVertices += Geom.m_pMesh[i].nVertices;
    }

    // Setup the RigidGeom
    RigidGeom.m_nDList      = nPS2DList;
    RigidGeom.m_System.pPS2 = pPS2DList;

    //
    // Save the Materials
    //
    ExportMaterial( Mesh, RigidGeom, PlatformID );

    //
    // Save the virtual meshes
    //
    ExportVirtualMeshes( Mesh, RigidGeom, PlatformID );

    //
    // Compute BBox's and Average World Pixel Size
    //

    RigidGeom.m_BBox.Clear();
    
    char FileName[256];
    x_memset( FileName, 0, sizeof( FileName ) );

    xbitmap Bitmap;
    s32 TotalVertices = 0;        
    for( i=0; i<RigidGeom.m_nMeshes; i++ )
    {
        s32         nVerts = 0;
        geom::mesh& Mesh   = RigidGeom.m_pMesh[i];
        Mesh.BBox.Clear();
        
        for( s32 j=0; j<Mesh.nSubMeshes; j++ )
        {
            s32            iSubMesh = Mesh.iSubMesh + j;
            geom::submesh& SubMesh  = RigidGeom.m_pSubMesh[ iSubMesh ];
            
            rigid_geom::dlist_ps2& DList = RigidGeom.m_System.pPS2[ SubMesh.iDList ];

            nVerts += DList.nVerts;

            for( s32 k=0; k<DList.nVerts; k++ )
            {
                vector3 Pos;
                Pos = DList.pPosition[k];
                Mesh.BBox += Pos;
            }
            
            // Get the first diffuse bitmap
            geom::material& Material  = RigidGeom.m_pMaterial[ SubMesh.iMaterial ];
            const char*     pFileName = m_Dictionary.GetString( RigidGeom.m_pTexture[Material.iTexture].FileNameOffset );

            char Drive[256];
            char Dir[256];
            char Path[256];

            x_splitpath( m_PlatInfo[ PlatformID ].FileName, Drive, Dir, NULL, NULL );
            x_makepath ( Path, Drive, Dir, NULL, NULL );

            
            if( x_strcmp( pFileName, FileName ) != 0 )
            {
                x_strcpy( FileName, pFileName );

                if( LoadBitmap( Bitmap, xfs("%s\\%s", Path, pFileName) ) == FALSE )
                    ThrowError( xfs( "Unable to load bitmap [%s]", pFileName ) );
            }

            SubMesh.WorldPixelSize = ComputeWorldPixelSize( DList.pPosition, DList.pUV, DList.nVerts, Bitmap );
        }
        
        RigidGeom.m_BBox += Mesh.BBox;

        x_printf( "%30s - %10d Vertices\n", m_Dictionary.GetString( Mesh.NameOffset ), nVerts );
        TotalVertices += nVerts;
    }
    x_printf( "%30s - %10d Total Vertices\n", "", TotalVertices );

    //
    // Build the "hi res" collision data.  AndyT
    //
    {
        CompileHighCollisionPS2( RigidGeom, pMatList, pFileName );
    } 

    // make sure the rigid geom collision box incorporates the low-poly collision
    for( i = 0; i < RigidGeom.m_Collision.nLowClusters; i++ )
    {
        RigidGeom.m_BBox += RigidGeom.m_Collision.pLowCluster[i].BBox;
    }

    delete [] pMatList;

    //
    // Compile the dictionary
    //
    CompileDictionary( RigidGeom );

    //
    // Save the data
    //
    fileio File;
    File.Save( m_PlatInfo[ PlatformID ].FileName, RigidGeom, FALSE );
}

//=============================================================================

void geom_compiler::ExportRigidGeomXbox( mesh& Mesh, rigid_geom& RigidGeom, const char* pFileName )
{
    s32                         i,j,k;
    s32                         iDList;
    s32                         nIndices;
    s32                         nXboxDList;
    s32                         PlatformID = GetPlatformIndex( PLATFORM_XBOX );
    collision_data::mat_info*   pMatList;

    //
    // Allocate memory for the Xbox display list
    //
    
    nXboxDList = 0;
    for( i=0; i<Mesh.SubMesh.GetCount(); i++ )
    {
        const sub_mesh& SubMesh = Mesh.SubMesh[i];
        nXboxDList += SubMesh.lDList.GetCount();
    }

    //
    // Allocate memory for a temporary material list.
    //

    pMatList = new collision_data::mat_info[ nXboxDList ];
    if( !pMatList )
        x_throw( "Out of memory." );

    //
    // Build the display lists
    //

    const rawmesh2&   RawMesh = *Mesh.SubMesh[0].pRawMesh;
    rawmesh2::vertex* pVertex = RawMesh.m_pVertex;

    rigid_geom::dlist_xbox* pXboxDList     = new rigid_geom::dlist_xbox[ nXboxDList ];
    s32*                  pVertexUsage = new s32[ RawMesh.m_nVertices ];
    
    if( (pXboxDList == NULL) || (pVertexUsage == NULL) )
        x_throw( "Out of memory" );

    s32 TotalVerts   = 0;
    s32 TotalIndices = 0;
 
    // Allocate temporary push buffer
    void* pPushBuffer = x_malloc(( MAX_TEMP_PUSH_BUFFER+D3DPUSHBUFFER_ALIGNMENT )&~D3DPUSHBUFFER_ALIGNMENT );

    s32 iColor = 0;

    // Loop through all SubMeshes
    for( iDList=i=0; i<Mesh.SubMesh.GetCount(); i++ )
    {
        const sub_mesh& SubMesh = Mesh.SubMesh[i];
        
        // Loop through all Display lists in the SubMesh
        for( j=0; j<SubMesh.lDList.GetCount(); j++ )
        {
            const dlist& DList = SubMesh.lDList[j];

            //
            // Note the material.
            //
            material&           Mat     = Mesh.Material[DList.iMaterial];
            ASSERT(Mat.pRawMesh);
            const rawmesh2&     RawMesh = *Mat.pRawMesh;
            rawmesh2::material& RawMat  = RawMesh.m_pMaterial[ Mat.iRawMaterial ];

            pMatList[iDList].SoundType = Mesh.Material[DList.iMaterial].TexInfo.SoundMat;
            pMatList[iDList].Flags     = 0;
            if (RawMat.bTwoSided)
                pMatList[iDList].Flags     |= collision_data::mat_info::FLAG_DOUBLESIDED;


            rigid_geom::dlist_xbox& XboxDList = pXboxDList[ iDList++ ];
            
            s32 nFacets      = SubMesh.lDList[j].lTri.GetCount();
            nIndices         = nFacets * 3;
            XboxDList.nIndices = nIndices;
            XboxDList.pIndices = new u16[ nIndices ];
            XboxDList.pVert    = new rigid_geom::vertex_xbox[ nIndices ];    // Allocate more than we need
            XboxDList.iBone    = DList.iBone;
            XboxDList.iColor   = -1;

            if( (XboxDList.pIndices == NULL) || (XboxDList.pVert == NULL) )
                x_throw( "Out of memory" );

            TotalIndices += nIndices;

            // Clear vertex usage table
            for( k=0; k<RawMesh.m_nVertices; k++ )
                pVertexUsage[k] = -1;

            s32 nVerts = 0;
            s32 nIndex = 0;
            
            // Loop through all the Tri's in the Display list
            for( k=0; k<DList.lTri.GetCount(); k++ )
            {
                s32 Tri = DList.lTri[k];

                ASSERT( (Tri>=0) && (Tri<RawMesh.m_nFacets) );
                rawmesh2::facet& Facet = RawMesh.m_pFacet[ Tri ];

                // Loop through all verts in the Tri
                for( s32 n=0; n<3; n++ )
                {
                    // Get the index into the global vertex pool.
                    // Convert the index to a DList relative index.
                    s32 GlobalIndex = Facet.iVertex[n];
                    ASSERT( (GlobalIndex>=0) && (GlobalIndex<RawMesh.m_nVertices) );

                    s32 VertexIndex = pVertexUsage[ GlobalIndex ];
                    ASSERT( (VertexIndex==-1) || ((VertexIndex>=0) && (VertexIndex<nVerts)));

                    // Check if we have this vertex already
                    if( VertexIndex == -1 )
                    {
                        // Create the new vertex
                        rigid_geom::vertex_xbox& Vertex = XboxDList.pVert[ nVerts ];

                        Vertex.Pos    = pVertex[ GlobalIndex ].Position;
                        Vertex.UV     = pVertex[ GlobalIndex ].UV[0];

                        // Pack normal
                        vector3& Norm = pVertex[ GlobalIndex ].Normal[0];
                        Vertex.PackedNormal =
                            (((( u32 )( Norm.GetZ() *  511.0f )) & 0x3ff ) << 22L ) |
                            (((( u32 )( Norm.GetY() * 1023.0f )) & 0x7ff ) << 11L ) |
                            (((( u32 )( Norm.GetX() * 1023.0f )) & 0x7ff ) <<  0L );

                        // Store an index to the new vertex
                        pVertexUsage[ GlobalIndex ] = nVerts;
                        VertexIndex = nVerts;
                        nVerts++;
                    }

                    ASSERT( VertexIndex <= 0xFFFF );
                    ASSERT( nIndex < XboxDList.nIndices );
                    
                    // Store the index to the vertex
                    XboxDList.pIndices[ nIndex ] = VertexIndex;
                    nIndex++;
                }
            }

            XboxDList.iColor = iColor;
            XboxDList.nVerts = nVerts;

            iColor     += nVerts;
            TotalVerts += nVerts;

            ASSERT( nIndex == nIndices );

            // Convert to triangle strips
            #if STRIPE_GEOMS
            {
                u16 nGroups;
                PrimitiveGroup* pGroups;

                GenerateStrips( XboxDList.pIndices,XboxDList.nIndices,&pGroups,&nGroups );

                #if _MSC_VER < 1300
                ASSERTS( 0,"Xbox GeomCompiler MUST be built with Visual Studio.NET 2003\n" ):
                #else
                {
                    if( nGroups > 1 )
                        x_printf( "Warning: %d strips ignored\n",nGroups-1 );
                    u32 TotalSize = 0;
                    if( pPushBuffer )
                    {
                        // Optimise all strips
                        for( u16 i=0;i<nGroups;i++ )
                        {
                            if( pGroups[i].type != PT_STRIP )
                            {
                                x_printf( "WARNING: Triangle list group ignored!\n" );
                                continue;
                            }
                            DWORD dwPushSize = MAX_TEMP_PUSH_BUFFER;
                            XGCompileDrawIndexedVertices(
                                ((u8*)pPushBuffer)+TotalSize,
                                & dwPushSize,
                                D3DPT_TRIANGLESTRIP,
                                pGroups[i].numIndices,
                                pGroups[i].indices );
                            TotalSize += dwPushSize;
                        }

                        // Save strips
                        XboxDList.pPushBuffer=( u8*)x_malloc( TotalSize );
                        XboxDList.nPushSize = TotalSize;
                        XboxDList.hPushBuffer = NULL;
                        XboxDList.hVert = NULL;
                        x_memcpy(
                            XboxDList.pPushBuffer,
                            pPushBuffer,
                            TotalSize
                        );
                    }
                }
                #endif
                delete[]pGroups;
            }
            #elif _MSC_VER >= 1300 // Optimise small lists( .NET only )
            if( pPushBuffer )
            {
                DWORD dwPushSize = MAX_TEMP_PUSH_BUFFER;
                XGCompileDrawIndexedVertices(
                    pPushBuffer,
                    & dwPushSize,
                    D3DPT_TRIANGLELIST,
                    XboxDList.nIndices,
                    XboxDList.pIndices );
                XboxDList.pPushBuffer=( u8*)x_malloc( dwPushSize );
                ASSERT( XboxDList.pPushBuffer );
                XboxDList.nPushSize = dwPushSize;
                XboxDList.hPushBuffer = NULL;
                XboxDList.hVert = NULL;
                x_memcpy(
                    XboxDList.pPushBuffer,
                    pPushBuffer,
                    dwPushSize
                );
            }
            #endif
        }
    }
    x_free( pPushBuffer );

/*
    if( TotalVerts < RawMesh.m_nVertices )
    {
        x_printf("GeomComp TotalVerts:%5d  RawMeshVerts:%5d   FN:%s\n",TotalVerts,RawMesh.m_nVertices,pFileName);
        x_printf("AAAAAAAAAAAAAAAAAAAAAAAHHHHHHHHHHHHHHHHHHHHHHHHHHHH\n");
    }
    ASSERT( TotalVerts >= RawMesh.m_nVertices );
*/
    delete pVertexUsage;

    //
    // Setup the Geom
    //
    
    geom& Geom       = RigidGeom;

    Geom.m_nTextures = 0;
    Geom.m_pTexture  = NULL;
    Geom.m_Platform  = PLATFORM_XBOX;
    
    // Allocate space for all the meshes in the geom
    Geom.m_nMeshes   = Mesh.SubMesh.GetCount();
    Geom.m_pMesh     = new geom::mesh[ Geom.m_nMeshes ];
    
    // Allocate space for all the submeshes in the geom
    Geom.m_nSubMeshes = nXboxDList;
    Geom.m_pSubMesh   = new geom::submesh[ Geom.m_nSubMeshes ];
    
    if( (Geom.m_pMesh == NULL) || (Geom.m_pSubMesh == NULL) )
        x_throw( "Out of memory" );

    //
    // Setup mesh and submesh structures
    //

    for( iDList=i=0; i<Mesh.SubMesh.GetCount(); i++ )
    {
        // Setup runtime "geom::mesh" name and bone count
        Geom.m_pMesh[i].NameOffset = (s16)m_Dictionary.Add( Mesh.SubMesh[i].Name );
        Geom.m_pMesh[i].nBones     = Mesh.SubMesh[i].pRawSubMesh->nBones ;

        // 1 submesh per display list
        Geom.m_pMesh[i].nSubMeshes = Mesh.SubMesh[i].lDList.GetCount();
        Geom.m_pMesh[i].iSubMesh   = iDList;
        Geom.m_pMesh[i].nVertices  = 0;
        Geom.m_pMesh[i].nFaces     = 0;
        
        // Set the material and display list index into the submeshes
        for( k=0; k<Mesh.SubMesh[i].lDList.GetCount(); k++, iDList++ )
        {
            // tally up the vert and face counts...the face count
            // is before indexing, and the vert count is after indexing
            // has occured
            const dlist& DList = Mesh.SubMesh[i].lDList[k];
            Geom.m_pMesh[i].nFaces    += DList.lTri.GetCount();
            Geom.m_pMesh[i].nVertices += pXboxDList[ iDList ].nVerts;

            geom::submesh& SubMesh = Geom.m_pSubMesh[ iDList ];
            
            SubMesh.iDList    = iDList;
            SubMesh.iMaterial = Mesh.SubMesh[i].lDList[k].iMaterial;
        }
    }        

    ASSERT( iDList == nXboxDList );

    // Count up the faces and vertices
    Geom.m_nFaces    = 0;
    Geom.m_nVertices = 0;
    for ( i = 0; i < Geom.m_nMeshes; i++ )
    {
        Geom.m_nFaces    += Geom.m_pMesh[i].nFaces;
        Geom.m_nVertices += Geom.m_pMesh[i].nVertices;
    }

    // Setup the RigidGeom
    RigidGeom.m_nDList     = nXboxDList;
    RigidGeom.m_System.pXbox = pXboxDList;

    //
    // Compute BBox's
    //

    RigidGeom.m_BBox.Clear();

    s32 TotalVertices = 0;        

    for( i=0; i<RigidGeom.m_nMeshes; i++ )
    {
        geom::mesh& Mesh = RigidGeom.m_pMesh[i];
        Mesh.BBox.Clear();
        
        s32 nVerts = 0;
        for( s32 j=0; j<Mesh.nSubMeshes; j++ )
        {
            s32            iSubMesh = Mesh.iSubMesh + j;
            geom::submesh& SubMesh  = RigidGeom.m_pSubMesh[ iSubMesh ];
            
            rigid_geom::dlist_xbox& DList = RigidGeom.m_System.pXbox[ SubMesh.iDList ];
            
            for( s32 k=0; k<DList.nVerts; k++ )
            {
                Mesh.BBox += DList.pVert[k].Pos;
            }

            nVerts += DList.nVerts;
        }

        x_printf( "%30s - %10d Vertices\n", m_Dictionary.GetString(Mesh.NameOffset), nVerts );
        TotalVertices    += nVerts;
        RigidGeom.m_BBox += Mesh.BBox;
    }
    x_printf( "%30s - %10d Total Vertices\n", "", TotalVertices );

    //
    // Save the materials
    //
    ExportMaterial( Mesh, RigidGeom, PlatformID );

    //
    // Save the virtual meshes
    //
    ExportVirtualMeshes( Mesh, RigidGeom, PlatformID );

    //
    // Build the "hi res" collision data.  AndyT
    //
    {
        CompileHighCollisionXBOX( RigidGeom, pMatList, pFileName );
    } 

    // make sure the rigid geom collision box incorporates the low-poly collision
    for( i = 0; i < RigidGeom.m_Collision.nLowClusters; i++ )
    {
        RigidGeom.m_BBox += RigidGeom.m_Collision.pLowCluster[i].BBox;
    }

    delete [] pMatList;

    //
    // Compile the dictionary
    //
    CompileDictionary( RigidGeom );

    //
    // Save the data
    //

    fileio File;
    File.Save( m_PlatInfo[ PlatformID ].FileName , RigidGeom, FALSE );
}

//=============================================================================

void geom_compiler::ExportRigidGeomPC( mesh& Mesh, rigid_geom& RigidGeom, const char* pFileName )
{
    s32                         i,j,k;
    s32                         iDList;
    s32                         nIndices;
    s32                         nPCDList;
    s32                         PlatformID = GetPlatformIndex( PLATFORM_PC );
    collision_data::mat_info*   pMatList;

    //
    // Allocate memory for the PC display list
    //
    
    nPCDList = 0;
    for( i=0; i<Mesh.SubMesh.GetCount(); i++ )
    {
        const sub_mesh& SubMesh = Mesh.SubMesh[i];
        nPCDList += SubMesh.lDList.GetCount();
    }

    //
    // Allocate memory for a temporary material list.
    //

    pMatList = new collision_data::mat_info[ nPCDList ];
    if( !pMatList )
        x_throw( "Out of memory." );

    //
    // Build the display lists
    //

    const rawmesh2&   RawMesh = *Mesh.SubMesh[0].pRawMesh;
    rawmesh2::vertex* pVertex = RawMesh.m_pVertex;

    rigid_geom::dlist_pc* pPCDList     = new rigid_geom::dlist_pc[ nPCDList ];
    s32*                  pVertexUsage = new s32[ RawMesh.m_nVertices ];
    
    if( (pPCDList == NULL) || (pVertexUsage == NULL) )
        x_throw( "Out of memory" );

    s32 TotalVerts   = 0;
    s32 TotalIndices = 0;
 
    // Loop through all SubMeshes
    for( iDList=i=0; i<Mesh.SubMesh.GetCount(); i++ )
    {
        const sub_mesh& SubMesh = Mesh.SubMesh[i];
        
        // Loop through all Display lists in the SubMesh
        for( j=0; j<SubMesh.lDList.GetCount(); j++ )
        {
            const dlist& DList = SubMesh.lDList[j];

            //
            // Note the material.
            //
            material&           Mat     = Mesh.Material[DList.iMaterial];
            ASSERT(Mat.pRawMesh);
            const rawmesh2&     RawMesh = *Mat.pRawMesh;
            rawmesh2::material& RawMat  = RawMesh.m_pMaterial[ Mat.iRawMaterial ];

            pMatList[iDList].SoundType = Mesh.Material[DList.iMaterial].TexInfo.SoundMat;
            pMatList[iDList].Flags     = 0;
            if (RawMat.bTwoSided)
                pMatList[iDList].Flags     |= collision_data::mat_info::FLAG_DOUBLESIDED;
            
            // SB: Is this a transparent material?
            //     (the editor uses this for double sided selection)
            switch( RawMat.Type )
            {
                default:
                    ASSERTS( 0, "Add new material type here or below if it's transparent!" );
                case Material_Not_Used:
                case Material_Diff:
                case Material_Diff_PerPixelEnv:
                case Material_Diff_PerPixelIllum:
                case Material_Distortion:
                case Material_Distortion_PerPolyEnv:
                    break;                
                case Material_Alpha:
                case Material_Alpha_PerPolyEnv:
                case Material_Alpha_PerPixelIllum:
                case Material_Alpha_PerPolyIllum:
                    pMatList[iDList].Flags |= collision_data::mat_info::FLAG_TRANSPARENT;
                    break;
            }                    

            rigid_geom::dlist_pc& PCDList = pPCDList[ iDList++ ];
            
            s32 nFacets      = SubMesh.lDList[j].lTri.GetCount();
            nIndices         = nFacets * 3;
            PCDList.nIndices = nIndices;
            PCDList.pIndices = new u16[ nIndices ];
            PCDList.pVert    = new rigid_geom::vertex_pc[ nIndices ];    // Allocate more than we need
            PCDList.iBone    = DList.iBone;
            
            if( (PCDList.pIndices == NULL) || (PCDList.pVert == NULL) )
                x_throw( "Out of memory" );

            TotalIndices += nIndices;

            // Clear vertex usage table
            for( k=0; k<RawMesh.m_nVertices; k++ )
                pVertexUsage[k] = -1;

            s32 nVerts = 0;
            s32 nIndex = 0;
            
            // Loop through all the Tri's in the Display list
            for( k=0; k<DList.lTri.GetCount(); k++ )
            {
                s32 Tri = DList.lTri[k];

                ASSERT( (Tri>=0) && (Tri<RawMesh.m_nFacets) );
                rawmesh2::facet& Facet = RawMesh.m_pFacet[ Tri ];

                // Loop through all verts in the Tri
                for( s32 n=0; n<3; n++ )
                {
                    // Get the index into the global vertex pool.
                    // Convert the index to a DList relative index.
                    s32 GlobalIndex = Facet.iVertex[n];
                    ASSERT( (GlobalIndex>=0) && (GlobalIndex<RawMesh.m_nVertices) );

                    s32 VertexIndex = pVertexUsage[ GlobalIndex ];
                    ASSERT( (VertexIndex==-1) || ((VertexIndex>=0) && (VertexIndex<nVerts)));
                    
                    // Check if we have this vertex already
                    if( VertexIndex == -1 )
                    {
                        // Create the new vertex
                        rigid_geom::vertex_pc& Vertex = PCDList.pVert[ nVerts ];
                        
                        Vertex.Pos    = pVertex[ GlobalIndex ].Position;
                        Vertex.UV     = pVertex[ GlobalIndex ].UV[0];
                        Vertex.Color  = pVertex[ GlobalIndex ].Color[0];
                        Vertex.Normal = pVertex[ GlobalIndex ].Normal[0]; 
                        
                        // Store an index to the new vertex
                        pVertexUsage[ GlobalIndex ] = nVerts;
                        VertexIndex = nVerts;
                        nVerts++;
                    }

                    ASSERT( VertexIndex <= 0xFFFF );
                    ASSERT( nIndex < PCDList.nIndices );
                    
                    // Store the index to the vertex
                    PCDList.pIndices[ nIndex ] = VertexIndex;
                    nIndex++;
                }
            }

            PCDList.nVerts = nVerts;
            TotalVerts    += nVerts;

            ASSERT( nIndex == nIndices );
        }
    }

/*
    if( TotalVerts < RawMesh.m_nVertices )
    {
        x_printf("GeomComp TotalVerts:%5d  RawMeshVerts:%5d   FN:%s\n",TotalVerts,RawMesh.m_nVertices,pFileName);
        x_printf("AAAAAAAAAAAAAAAAAAAAAAAHHHHHHHHHHHHHHHHHHHHHHHHHHHH\n");
    }
    ASSERT( TotalVerts >= RawMesh.m_nVertices );
*/
    delete pVertexUsage;

    //
    // Setup the Geom
    //
    
    geom& Geom       = RigidGeom;

    Geom.m_nTextures = 0;
    Geom.m_pTexture  = NULL;
    Geom.m_Platform  = PLATFORM_PC;
    
    // Allocate space for all the meshes in the geom
    Geom.m_nMeshes   = Mesh.SubMesh.GetCount();
    Geom.m_pMesh     = new geom::mesh[ Geom.m_nMeshes ];
    
    // Allocate space for all the submeshes in the geom
    Geom.m_nSubMeshes = nPCDList;
    Geom.m_pSubMesh   = new geom::submesh[ Geom.m_nSubMeshes ];
    
    if( (Geom.m_pMesh == NULL) || (Geom.m_pSubMesh == NULL) )
        x_throw( "Out of memory" );

    //
    // Setup mesh and submesh structures
    //

    for( iDList=i=0; i<Mesh.SubMesh.GetCount(); i++ )
    {
        // Setup runtime "geom::mesh" name and bone count
        Geom.m_pMesh[i].NameOffset = (s16)m_Dictionary.Add( Mesh.SubMesh[i].Name );
        Geom.m_pMesh[i].nBones     = Mesh.SubMesh[i].pRawSubMesh->nBones ;

        // 1 submesh per display list
        Geom.m_pMesh[i].nSubMeshes = Mesh.SubMesh[i].lDList.GetCount();
        Geom.m_pMesh[i].iSubMesh   = iDList;
        Geom.m_pMesh[i].nVertices  = 0;
        Geom.m_pMesh[i].nFaces     = 0;
        
        // Set the material and display list index into the submeshes
        for( k=0; k<Mesh.SubMesh[i].lDList.GetCount(); k++, iDList++ )
        {
            // tally up the vert and face counts...the face count
            // is before indexing, and the vert count is after indexing
            // has occured
            const dlist& DList = Mesh.SubMesh[i].lDList[k];
            Geom.m_pMesh[i].nFaces    += DList.lTri.GetCount();
            Geom.m_pMesh[i].nVertices += pPCDList[ iDList ].nVerts;

            geom::submesh& SubMesh = Geom.m_pSubMesh[ iDList ];
            
            SubMesh.iDList    = iDList;
            SubMesh.iMaterial = Mesh.SubMesh[i].lDList[k].iMaterial;
        }
    }        

    ASSERT( iDList == nPCDList );

    // Count up the faces and vertices
    Geom.m_nFaces    = 0;
    Geom.m_nVertices = 0;
    for ( i = 0; i < Geom.m_nMeshes; i++ )
    {
        Geom.m_nFaces    += Geom.m_pMesh[i].nFaces;
        Geom.m_nVertices += Geom.m_pMesh[i].nVertices;
    }

    // Setup the RigidGeom
    RigidGeom.m_nDList     = nPCDList;
    RigidGeom.m_System.pPC = pPCDList;

    //
    // Compute BBox's
    //

    RigidGeom.m_BBox.Clear();

    s32 TotalVertices = 0;        

    for( i=0; i<RigidGeom.m_nMeshes; i++ )
    {
        geom::mesh& Mesh = RigidGeom.m_pMesh[i];
        Mesh.BBox.Clear();
        
        s32 nVerts = 0;
        for( s32 j=0; j<Mesh.nSubMeshes; j++ )
        {
            s32            iSubMesh = Mesh.iSubMesh + j;
            geom::submesh& SubMesh  = RigidGeom.m_pSubMesh[ iSubMesh ];
            
            rigid_geom::dlist_pc& DList = RigidGeom.m_System.pPC[ SubMesh.iDList ];
            
            for( s32 k=0; k<DList.nVerts; k++ )
            {
                Mesh.BBox += DList.pVert[k].Pos;
            }

            nVerts        += DList.nVerts;
        }

        x_printf( "%30s - %10d Vertices\n", m_Dictionary.GetString(Mesh.NameOffset), nVerts );
        TotalVertices    += nVerts;
        RigidGeom.m_BBox += Mesh.BBox;
    }
    x_printf( "%30s - %10d Total Vertices\n", "", TotalVertices );

    //
    // Save the materials
    //
    ExportMaterial( Mesh, RigidGeom, PlatformID );

    //
    // Save the virtual meshes
    //
    ExportVirtualMeshes( Mesh, RigidGeom, PlatformID );

    //
    // Build the "hi res" collision data.  AndyT
    //
    {
        CompileHighCollisionPC( RigidGeom, pMatList, pFileName );
    } 

    // make sure the rigid geom collision box incorporates the low-poly collision
    for( i = 0; i < RigidGeom.m_Collision.nLowClusters; i++ )
    {
        RigidGeom.m_BBox += RigidGeom.m_Collision.pLowCluster[i].BBox;
    }

    delete [] pMatList;

    //
    // Compile the dictionary
    //
    CompileDictionary( RigidGeom );

    //
    // Save the data
    //

    fileio File;
    File.Save( m_PlatInfo[ PlatformID ].FileName , RigidGeom, FALSE );
}

//=============================================================================

void geom_compiler::ExportSkinGeom( const char* pFileName )
{
    // load the resource
    LoadResource( pFileName, TRUE );

    //
    // Load the raw-mesh
    //

    rawmesh2    RawMesh;
    s32         i;

    rawanim RawAnim;
    RawMesh.Load( m_pGeomRscDesc->GetMatxFileName() );
    RawAnim.Load( m_pGeomRscDesc->GetMatxFileName() );
    SetUserInfo( RawMesh.m_UserName, RawMesh.m_ComputerName );
    SetExportDate( RawMesh.m_ExportDate[0], RawMesh.m_ExportDate[1], RawMesh.m_ExportDate[2] );
    RemoveUnusedVMeshes( RawMesh );

    RawMesh.CleanMesh();
    RawMesh.CleanWeights( 2, 0.00001f );
    RawMesh.SortFacetsByMaterialAndBone();

    // Do remapping of the skeleton
    RawAnim.DeleteDummyBones();
    RawMesh.ApplyNewSkeleton( RawAnim );
    
    // Show hierarchy?
    if( g_Verbose )
        RawAnim.PrintHierarchy();

    // Export to the correct platform
    for( i=0; i<m_PlatInfo.GetCount(); i++ )
    {
        // Need this here so dictionary is cleaned up
        m_Dictionary.Reset();
        
        skin_geom SkinGeom;
        mesh      Mesh;
        BuildBasicStruct( SkinGeom, RawMesh, Mesh, FALSE );
    
        // Build for platform
        switch( m_PlatInfo[i].Platform )
        {
            case PLATFORM_XBOX :
                ExportSkinGeomXbox( Mesh, SkinGeom, pFileName );
                break;

            case PLATFORM_PS2 :
                ExportSkinGeomPS2( Mesh, SkinGeom, pFileName );
                break;
            
            case PLATFORM_PC :
                ExportSkinGeomPC( Mesh, SkinGeom, pFileName );
                break;

            default :
                ASSERT( 0 );
                break;
        }
        
        // print out a material and texture listing
        if( g_Verbose )
            PrintSummary( SkinGeom );
    }
}

//=============================================================================

void geom_compiler::RecenterUVs( s16* pUV, s32 nVerts )
{
    s32 i;
    s16 UMin = S16_MAX;
    s16 UMax = S16_MIN;
    s16 VMin = S16_MAX;
    s16 VMax = S16_MIN;

    // calculate the range uv's that we have
    for ( i = 0; i < nVerts; i++ )
    {
        s16 U = pUV[i*2+0] / 4096;
        s16 V = pUV[i*2+1] / 4096;

        UMin = MIN( U, UMin );
        UMax = MAX( U, UMax );
        VMin = MIN( V, VMin );
        VMax = MAX( V, VMax );
    }

    // recenter the uv's around zero to minimize ps2 precision oddities for
    // tiled uv's
    s16 URange = UMax-UMin;
    s16 VRange = VMax-VMin;
    s16 UOffset = (-(URange/2) - UMin)*4096;
    s16 VOffset = (-(VRange/2) - VMin)*4096;
    for ( i = 0; i < nVerts; i++ )
    {
        pUV[i*2+0] += UOffset;
        pUV[i*2+1] += VOffset;
    }
}

//=============================================================================

f32 ComputeWorldPixelSize( skin_geom::dlist_ps2& DList, const xbitmap& Bitmap )
{
    // allocate space for verts to be used by the standard ComputeWorldPixelSize function
    vector4* pVert = new vector4[ DList.nPos ];
    s16*     pUV   = new s16[ DList.nPos * 2 ];

    for ( s32 i = 0; i < DList.nPos; i++ )
    {
        pVert[i].GetX() = (f32)DList.pPos[i].Pos[0] / PS2_SKIN_FIXED_SCALE;
        pVert[i].GetY() = (f32)DList.pPos[i].Pos[1] / PS2_SKIN_FIXED_SCALE;
        pVert[i].GetZ() = (f32)DList.pPos[i].Pos[2] / PS2_SKIN_FIXED_SCALE;
        pVert[i].GetIW() = DList.pPos[i].ADC;
        pUV[i*2+0]       = DList.pUV[i].U;
        pUV[i*2+1]       = DList.pUV[i].V;
    }

    // calculate the world pixel size
    f32 Result = ComputeWorldPixelSize( pVert, pUV, DList.nPos, Bitmap );

    // clean up and return
    delete []pVert;
    delete []pUV;
    
    return Result;
}

//=============================================================================

void geom_compiler::ExportSkinGeomPS2( mesh& Mesh, skin_geom& SkinGeom, const char* pFileName )
{
    s32         iDList;
    s32         i,j,k,l;
    faststrip   Strip;
    s32         PlatformID = GetPlatformIndex( PLATFORM_PS2 );

    xbool       bOutOfFixedRange = FALSE;

    //
    // Allocate memory for the PS2 display list
    //
    
    s32 nPS2DList = 0;
    for( i=0; i<Mesh.SubMesh.GetCount(); i++ )
    {
        const sub_mesh& SubMesh = Mesh.SubMesh[i];
        nPS2DList += SubMesh.lDList.GetCount();
    }

    skin_geom::dlist_ps2* pPS2DList = new skin_geom::dlist_ps2[ nPS2DList ];
    if( pPS2DList == NULL )
        x_throw( "Out of memory" );

    ps2skin_optimizer SkinOptimizer;
    for( iDList=i=0; i<Mesh.SubMesh.GetCount(); i++ )
    {
        const sub_mesh& SubMesh = Mesh.SubMesh[i];
        for( j=0; j<SubMesh.lDList.GetCount(); j++ )
        {
            const dlist& DList = SubMesh.lDList[j];

            //
            // Add the vertices to the optimizer
            //
            SkinOptimizer.Reset();
            for ( k = 0; k < DList.lTri.GetCount(); k++ )
            {
                ps2skin_optimizer::optimizer_tri Tri;

                for ( s32 iVert = 0; iVert < 3; iVert++ )
                {
                    s32               OrigIndex = SubMesh.pRawMesh->m_pFacet[ DList.lTri[k] ].iVertex[iVert];
                    rawmesh2::vertex* pVert = &SubMesh.pRawMesh->m_pVertex[OrigIndex];

                    Tri.Verts[iVert].iOrigIndex = OrigIndex;
                    Tri.Verts[iVert].nWeights   = pVert->nWeights;
                    for ( s32 iWeight = 0; iWeight < pVert->nWeights; iWeight++ )
                    {
                        Tri.Verts[iVert].iOrigBones[iWeight] = pVert->Weight[iWeight].iBone;
                        Tri.Verts[iVert].fWeights[iWeight]   = pVert->Weight[iWeight].Weight;
                    }
                }
                SkinOptimizer.AddTri( Tri );
            }

            //
            // Optimize for the matrix cache
            //
            SkinOptimizer.Optimize();

            if ( 0 )
            {
                x_printf( "=================================================================\n"  );
                x_printf( "Stats for mesh %d:%d (%s:material #%d)\n",
                          i, j, SubMesh.Name, j );
                SkinOptimizer.PrintStats();
                x_printf( "=================================================================\n"  );
            }

            //
            // Build the final display list
            //
            s32 nMatrixUploads = 0;
            s32 nVertRenders = SkinOptimizer.GetNFinalBatches();
            s32 nVerts = 0;
            for ( k = 0; k < SkinOptimizer.GetNFinalBatches(); k++ )
            {
                const ps2skin_optimizer::final_batch& Batch = SkinOptimizer.GetFinalBatch(k);
                nMatrixUploads += Batch.nBonesToLoad;
                nVerts         += Batch.Verts.GetCount();
            }

            // allocate space for the various display list things
            s32 WorstCasePad     = nVertRenders*4;      // at max 4 extra verts per "render" packet for alignment
            s32 nCmds            = nMatrixUploads + nVertRenders + 1;
            s32 nUVs             = nVerts;
            s32 nPos             = nVerts;
            s32 nBoneIndices     = nVerts;
            s32 nOrigBoneIndices = nVerts;
            s32 nNormals         = nVerts;
            skin_geom::command_ps2*   pCmd       = new skin_geom::command_ps2[nCmds+WorstCasePad];
            skin_geom::uv_ps2*        pUV        = new skin_geom::uv_ps2[nUVs+WorstCasePad];
            skin_geom::pos_ps2*       pPos       = new skin_geom::pos_ps2[nPos+WorstCasePad];
            skin_geom::boneindex_ps2* pBoneIndex = new skin_geom::boneindex_ps2[nBoneIndices+WorstCasePad];
            skin_geom::normal_ps2*    pNormal    = new skin_geom::normal_ps2[nNormals+WorstCasePad];
            if ( !pCmd || !pUV || !pPos || !pBoneIndex || !pNormal )
                x_throw( "Out of Memory" );

            // build the components of the display list
            skin_geom::command_ps2* pCurrCmd  = pCmd;
            s32                     iCurrVert = 0;
            for ( k = 0; k < SkinOptimizer.GetNFinalBatches(); k++ )
            {
                // set up the matrix loads for this batch
                const ps2skin_optimizer::final_batch& Batch = SkinOptimizer.GetFinalBatch(k);
                for ( l = 0; l < Batch.nBonesToLoad; l++ )
                {
                    pCurrCmd->Cmd  = skin_geom::PS2_CMD_UPLOAD_MATRIX;
                    pCurrCmd->Arg1 = Batch.iOrigBones[l];
                    pCurrCmd->Arg2 = Batch.iCacheBones[l];
                    pCurrCmd++;
                }

                // what are the details of this batch?
                skin_geom::command_types_ps2 Cmd  = skin_geom::PS2_CMD_RENDER_VERTS_RIGID;
                s32                          Arg1 = iCurrVert;
                s32                          Arg2 = Batch.Verts.GetCount();

                // add the verts
                for ( l = 0; l < Batch.Verts.GetCount(); l++ )
                {
                    ps2skin_optimizer::optimizer_vert& OptVert = Batch.Verts[l];
                    rawmesh2::vertex*                  pRMVert = &SubMesh.pRawMesh->m_pVertex[OptVert.iOrigIndex];
                    
                    // figure out the weights...
                    ASSERT( (OptVert.nWeights == 1) || (OptVert.nWeights == 2) );
                    ASSERT( OptVert.fWeights[0] >= 0.5f );
                    f32 W0 = OptVert.fWeights[0];
                    f32 W1 = (OptVert.nWeights == 2) ? OptVert.fWeights[1] : 0.0f;
                    ASSERT( W0 >= W1 );
                    ASSERT( (0.0f<=W0) && (W0<=1.0f) );
                    ASSERT( (0.0f<=W1) && (W1<=1.0f) );

                    // if there are two weights, then we need to soft-skin this batch
                    if ( W1 > 0.0f )
                        Cmd = skin_geom::PS2_CMD_RENDER_VERTS_SOFT;

                    // set up the uvs and weights (fixed-point 4.12)
                    pUV[iCurrVert].U  = (s16)(pRMVert->UV[0].X * (1 << 12));
                    pUV[iCurrVert].V  = (s16)(pRMVert->UV[0].Y * (1 << 12));

                    // fill in the positions, making sure they are within the
                    // fixed-point range
                    f32 Pos[3] = { pRMVert->Position.GetX(),
                                   pRMVert->Position.GetY(),
                                   pRMVert->Position.GetZ() };
                    for( s32 iPos = 0; iPos < 3; iPos++ )
                    {
                        if( Pos[iPos] > PS2_SKIN_RANGE )
                        {
                            bOutOfFixedRange = TRUE;
                            Pos[iPos] = PS2_SKIN_RANGE;
                        }
                        if( Pos[iPos] < -PS2_SKIN_RANGE )
                        {
                            bOutOfFixedRange = TRUE;
                            Pos[iPos] = -PS2_SKIN_RANGE;
                        }

                        pPos[iCurrVert].Pos[iPos] = (s16)(Pos[iPos] * PS2_SKIN_FIXED_SCALE);
                    }

                    // fill in the adc and winding bits
                    pPos[iCurrVert].ADC   = (OptVert.ADC ? (1<<ADCBIT) : 0x0000);
                    pPos[iCurrVert].ADC  |= (OptVert.CCW ? (1<<CCWBIT) : 0x0000);

                    // set up the bone indices (note the bone indices are pre-multiplied by
                    // 4 to avoid calculating the vector offset in microcode)
                    u32 B0, B1;
                    B0 = OptVert.iCacheBones[0];
                    B1 = ((OptVert.nWeights == 2) ? OptVert.iCacheBones[1] : B0);
                    ASSERT( (B0*4 < 256) && (B1*4 < 256) );
                    W0 *= 255.0f;
                    W1 *= 255.0f;
                    W0 = x_round(W0, 1.0f);
                    W1 = x_round(W1, 1.0f);
                    W0 = MIN(255.0f, W0);
                    W1 = MIN(255.0f, W1);
                    W0 = MAX(0.0f, W0);
                    W1 = MAX(0.0f, W1);
                    pBoneIndex[iCurrVert].B0 = (u8)(B0*4);
                    pBoneIndex[iCurrVert].B1 = (u8)(B1*4);
                    pBoneIndex[iCurrVert].W0 = (u8)(W0);
                    pBoneIndex[iCurrVert].W1 = (u8)(W1);
                    s32 Sum = (s32)pBoneIndex[iCurrVert].W0 + (s32)pBoneIndex[iCurrVert].W1;
                    if ( Sum != 255 )
                    {
                        // Sum should normally equal 255, but may be slightly off if there was
                        // any rounding accumulation errors going on above. This safety check
                        // should fix it up.
                        if ( Sum > 255 )
                        {
                            if ( pBoneIndex[iCurrVert].W0 > pBoneIndex[iCurrVert].W1 )
                                pBoneIndex[iCurrVert].W0 -= (Sum-255);
                            else
                                pBoneIndex[iCurrVert].W1 -= (Sum-255);
                        }
                        else if ( Sum < 255 )
                        {
                            if ( pBoneIndex[iCurrVert].W0 > pBoneIndex[iCurrVert].W1 )
                                pBoneIndex[iCurrVert].W1 += (255-Sum);
                            else
                                pBoneIndex[iCurrVert].W0 += (255-Sum);
                        }

                        ASSERT( 255 == ((s32)pBoneIndex[iCurrVert].W0 + (s32)pBoneIndex[iCurrVert].W1) );
                    }

                    // set up the normals (signed fixed-point 1.0.7)
                    pNormal[iCurrVert].Normal[0] = (s8)(pRMVert->Normal[0].GetX() * (0xff>>1));
                    pNormal[iCurrVert].Normal[1] = (s8)(pRMVert->Normal[0].GetY() * (0xff>>1));
                    pNormal[iCurrVert].Normal[2] = (s8)(pRMVert->Normal[0].GetZ() * (0xff>>1));
                    pNormal[iCurrVert].Pad       = 0;

                    // everything is added now...move along to to the next vert
                    iCurrVert++;
                }

                // verts must be a multiple of 4 to avoid alignment issues, so if its less than that
                // pad the data out...
                while ( (iCurrVert % 4) != 0 )
                {
                    pUV[iCurrVert]          = pUV[iCurrVert-1];
                    pPos[iCurrVert]         = pPos[iCurrVert-1];
                    pBoneIndex[iCurrVert]   = pBoneIndex[iCurrVert-1];
                    pNormal[iCurrVert]      = pNormal[iCurrVert-1];

                    // vu1 should never receive this vert or try to render it since its only here for
                    // padding, but just as an added safety measure, we'll set the adc bit
                    pPos[iCurrVert].ADC     = (1<<ADCBIT);

                    iCurrVert++;
                }

                // add the command for this vertex batch
                pCurrCmd->Cmd  = Cmd;
                pCurrCmd->Arg1 = Arg1;
                pCurrCmd->Arg2 = Arg2;
                pCurrCmd++;
            }
            s32 nVertsWithPadding = iCurrVert;

            // looks like we're done
            pCurrCmd->Cmd  = skin_geom::PS2_CMD_END;
            pCurrCmd->Arg1 = 0;
            pCurrCmd->Arg2 = 0;
            pCurrCmd++;

            // sanity check
            ASSERT( pCurrCmd == (pCmd+nCmds) );

            //
            // Setup PS2 Display List
            //
            
            skin_geom::dlist_ps2& PS2DList = pPS2DList[ iDList++ ];
            
            PS2DList.nCmds        = nCmds;
            PS2DList.pCmd         = pCmd;
            PS2DList.nUVs         = nVertsWithPadding;
            PS2DList.pUV          = pUV;
            PS2DList.nPos         = nVertsWithPadding;
            PS2DList.pPos         = pPos;
            PS2DList.nBoneIndices = nVertsWithPadding;
            PS2DList.pBoneIndex   = pBoneIndex;
            PS2DList.nNormals     = nVertsWithPadding;
            PS2DList.pNormal      = pNormal;

            RecenterUVs( (s16*)PS2DList.pUV, nVertsWithPadding );
        }
    }

    ASSERT( iDList == nPS2DList );

    // report a warning if we had to clamp some verts for the fixed-point conversion
    if( bOutOfFixedRange )
    {
        ReportWarning( xfs( "Verts go outside of allowed range (%3.2f cm), so some clamping will occur.", PS2_SKIN_RANGE ) );
    }

    //
    // Setup the Geom
    //
    
    geom& Geom       = SkinGeom;

    Geom.m_nTextures = 0;
    Geom.m_pTexture  = NULL;
    Geom.m_Platform  = PLATFORM_PS2;
    
    // Allocate space for all the meshes in the geom
    Geom.m_nMeshes   = Mesh.SubMesh.GetCount();
    Geom.m_pMesh     = new geom::mesh[ Geom.m_nMeshes ];

    // Allocate space for all the submeshes in the geom
    Geom.m_nSubMeshes = nPS2DList;
    Geom.m_pSubMesh   = new geom::submesh[ Geom.m_nSubMeshes ];
    
    if( (Geom.m_pMesh == NULL) || (Geom.m_pSubMesh == NULL) )
        x_throw( "Out of memory" );

    //
    // Setup mesh and submesh structures
    //

    for( iDList=i=0; i<Mesh.SubMesh.GetCount(); i++ )
    {
        // Setup runtime "geom::mesh" name and bone count
        Geom.m_pMesh[i].NameOffset = (s16)m_Dictionary.Add( Mesh.SubMesh[i].Name );
        Geom.m_pMesh[i].nBones     = Mesh.SubMesh[i].pRawSubMesh->nBones ;

        // 1 submesh per display list
        Geom.m_pMesh[i].nSubMeshes = Mesh.SubMesh[i].lDList.GetCount();
        Geom.m_pMesh[i].iSubMesh   = iDList;
        Geom.m_pMesh[i].nVertices  = 0;
        Geom.m_pMesh[i].nFaces     = 0;
        
        // Set the material and display list index into the submeshes
        for( k=0; k<Mesh.SubMesh[i].lDList.GetCount(); k++, iDList++ )
        {
            // tally up the vert and face counts...the face count
            // is before optimization, and the vert count is after optimization
            // has occured
            const dlist& DList = Mesh.SubMesh[i].lDList[k];
            Geom.m_pMesh[i].nFaces    += DList.lTri.GetCount();
            Geom.m_pMesh[i].nVertices += pPS2DList[iDList].nPos;

            geom::submesh& SubMesh = Geom.m_pSubMesh[iDList];
            
            SubMesh.iDList    = iDList;
            SubMesh.iMaterial = Mesh.SubMesh[i].lDList[k].iMaterial;
        }
    }        

    ASSERT( iDList == nPS2DList );

    // Count up the faces and vertices
    Geom.m_nFaces    = 0;
    Geom.m_nVertices = 0;
    for ( i = 0; i < Geom.m_nMeshes; i++ )
    {
        Geom.m_nFaces    += Geom.m_pMesh[i].nFaces;
        Geom.m_nVertices += Geom.m_pMesh[i].nVertices;
    }
    
    // Setup the SkinGeom
    SkinGeom.m_nBones      = Mesh.SubMesh[0].pRawMesh->m_nBones;
    SkinGeom.m_nDList      = nPS2DList;
    SkinGeom.m_System.pPS2 = pPS2DList;

    //
    // Save the Materials
    //
    ExportMaterial( Mesh, SkinGeom, PlatformID );

    //
    // Save the virtual meshes
    //
    ExportVirtualMeshes( Mesh, SkinGeom, PlatformID );

    //
    // Compute BBox's
    //

    SkinGeom.m_BBox.Clear();

    char FileName[256];
    x_memset( FileName, 0, sizeof( FileName ) );

    xbitmap Bitmap;
    s32 TotalVertices = 0;
    for( i=0; i<SkinGeom.m_nMeshes; i++ )
    {
        s32         nVerts = 0;
        geom::mesh& Mesh   = SkinGeom.m_pMesh[i];
        Mesh.BBox.Clear();
        
        for( s32 j=0; j<Mesh.nSubMeshes; j++ )
        {
            s32            iSubMesh = Mesh.iSubMesh + j;
            geom::submesh& SubMesh  = SkinGeom.m_pSubMesh[ iSubMesh ];
            
            skin_geom::dlist_ps2& DList = SkinGeom.m_System.pPS2[ SubMesh.iDList ];
            nVerts += DList.nPos;

            for( s32 k=0; k<DList.nPos; k++ )
            {
                vector3 Pos( (f32)DList.pPos[k].Pos[0] / PS2_SKIN_FIXED_SCALE,
                             (f32)DList.pPos[k].Pos[1] / PS2_SKIN_FIXED_SCALE,
                             (f32)DList.pPos[k].Pos[2] / PS2_SKIN_FIXED_SCALE );
                Mesh.BBox += Pos;
            }

            // Get the first diffuse bitmap
            ASSERT( (SubMesh.iMaterial>=0) && (SubMesh.iMaterial<SkinGeom.m_nMaterials) );
            geom::material& Material  = SkinGeom.m_pMaterial[ SubMesh.iMaterial ];
            ASSERT( (Material.iTexture>=0) && (Material.iTexture<SkinGeom.m_nTextures) );
            const char*     pFileName = m_Dictionary.GetString( SkinGeom.m_pTexture[Material.iTexture].FileNameOffset );

            char Drive[256];
            char Dir[256];
            char Path[256];

            x_splitpath( m_PlatInfo[ PlatformID ].FileName, Drive, Dir, NULL, NULL );
            x_makepath ( Path, Drive, Dir, NULL, NULL );
            
            if( x_strcmp( pFileName, FileName ) != 0 )
            {
                x_strcpy( FileName, pFileName );

                if( LoadBitmap( Bitmap, xfs("%s\\%s", Path, pFileName)) == FALSE )
                    ThrowError( xfs( "Unable to load bitmap [%s]", pFileName ) );
            }

            SubMesh.WorldPixelSize = ComputeWorldPixelSize( DList, Bitmap );
        }
        
        SkinGeom.m_BBox += Mesh.BBox;
        x_printf( "%30s - %10d Vertices\n", m_Dictionary.GetString(Mesh.NameOffset), nVerts );

        TotalVertices += nVerts;
    }
    x_printf( "%30s - %10d Total Vertices\n", "", TotalVertices );

    //
    // Compile the dictionary
    //
    CompileDictionary( SkinGeom );

    //
    // Save the data
    //
    
    fileio File;
    File.Save( m_PlatInfo[ PlatformID ].FileName , SkinGeom, FALSE );
}

//=============================================================================

void geom_compiler::ExportSkinGeomXbox( mesh& Mesh, skin_geom& SkinGeom, const char* pFileName )
{
    s32             i,j,k;
    s32             iDList;
    s32             PlatformID = GetPlatformIndex( PLATFORM_XBOX );

    //
    // Get the total count of facet and indices and dlists
    //

    s32 nXboxDList = 0;
    s32 SubMeshCount = Mesh.SubMesh.GetCount();
    for( i=0; i<SubMeshCount; i++ )
    {
        const sub_mesh& SubMesh = Mesh.SubMesh[i];
        nXboxDList += SubMesh.lDList.GetCount();
    }

    skin_geom::dlist_xbox* pXboxDList = new skin_geom::dlist_xbox[ nXboxDList ];
    if( pXboxDList == NULL )
        x_throw( "Out of memory" );

    ASSERT( Mesh.SubMesh[0].pRawMesh );
    const rawmesh2& RawMesh = *Mesh.SubMesh[0].pRawMesh;

    // Allocate temporary push buffer
    void* pPushBuffer = x_malloc(( MAX_TEMP_PUSH_BUFFER+D3DPUSHBUFFER_ALIGNMENT )&~D3DPUSHBUFFER_ALIGNMENT );

    // Loop through all sub meshes
    for( iDList=i=0; i<SubMeshCount; i++ )
    {
        const sub_mesh& SubMesh = Mesh.SubMesh[i];
        for( j=0; j<SubMesh.lDList.GetCount(); j++ )
        {
            const dlist& DList = SubMesh.lDList[j];

            arm_optimizer Optimizer;
            Optimizer.Build( RawMesh, DList.lTri, (96-16)/4 ); // Leave room for 12 general registers, and 4 reg per bone

            arm_optimizer::section& Section = Optimizer.m_Section;

            //
            //  Copy all the indices
            //

            skin_geom::dlist_xbox& XboxDList = pXboxDList[ iDList++ ];

            XboxDList.nIndices = Section.lTriangle.GetCount() * 3;
            XboxDList.pIndex   = new u16[ XboxDList.nIndices ];
            if( XboxDList.pIndex == NULL )
                x_throw( "Out of memory" );

            for( k=0; k<Section.lTriangle.GetCount(); k++ )
            {
                XboxDList.pIndex[ (k*3) + 0 ] = Section.lTriangle[k].iVertex[0];
                XboxDList.pIndex[ (k*3) + 1 ] = Section.lTriangle[k].iVertex[1];
                XboxDList.pIndex[ (k*3) + 2 ] = Section.lTriangle[k].iVertex[2];
            }

            //
            //  Copy down all the verts
            //  

            XboxDList.nVerts = Section.lVertex.GetCount();
            XboxDList.pVert  = new skin_geom::vertex_xbox[ XboxDList.nVerts ];
            if( !XboxDList.pVert )
                x_throw( "Out of memory");

            for( k=0; k<XboxDList.nVerts; k++ )
            {
                skin_geom::vertex_xbox& Vert = XboxDList.pVert[k];

                // Position
                Vert.Pos = Section.lVertex[k].Position;

                // Bones
                Vert.Bones.X = (f32)Section.lVertex[k].Weight[0].iBone;
                if( Section.lVertex[k].nWeights > 1 )
                    Vert.Bones.Y = (f32)Section.lVertex[k].Weight[1].iBone;
                else
                    Vert.Bones.Y = 0.0f;
                Vert.Bones.X *= 4;
                Vert.Bones.Y *= 4;

                // Pack normal
                vector3& Norm = Section.lVertex[k].Normal[0];
                Vert.PackedNormal =
                    (((( u32 )( Norm.GetZ() *  511.0f )) & 0x3ff ) << 22L ) |
                    (((( u32 )( Norm.GetY() * 1023.0f )) & 0x7ff ) << 11L ) |
                    (((( u32 )( Norm.GetX() * 1023.0f )) & 0x7ff ) <<  0L );

                // UVs
                Vert.UV.X = Section.lVertex[k].UV[0].X;
                Vert.UV.Y = Section.lVertex[k].UV[0].Y;

                // Weights
                Vert.Weights.X = Section.lVertex[k].Weight[0].Weight;
                if( Section.lVertex[k].nWeights > 1 )
                    Vert.Weights.Y = Section.lVertex[k].Weight[1].Weight;
                else
                    Vert.Weights.Y = 0.0f;
            }

            //
            // Copy down all the commands
            //

            XboxDList.nCommands = Section.lCommand.GetCount();
            XboxDList.pCmd      = new skin_geom::command_xbox[ XboxDList.nCommands ];
            if( XboxDList.pCmd == NULL )
                x_throw( "Out of memory" );

            u32 nWritten=0;
            for( k=0; k<XboxDList.nCommands; k++ )
            {
                // Copy the command type
                switch( Section.lCommand[k].Type )
                {
                    // Add matrix to palette
                    case arm_optimizer::UPLOAD_MATRIX: 
                        XboxDList.pCmd[k].Cmd  = skin_geom::XBOX_CMD_UPLOAD_MATRIX;
                        XboxDList.pCmd[k].Arg2 = (u32)Section.lCommand[k].Arg2; // CacheID
                        XboxDList.pCmd[k].Arg1 = (u32)Section.lCommand[k].Arg1; // BoneID
                        break;

                    // Render push buffer
                    case arm_optimizer::DRAW_LIST:

                        #if _MSC_VER < 1300
                            ASSERTS( 0,"Xbox GeomCompiler must be built with Visual Studio.NET 2003\n" );
                        #else
                        {
                            DWORD Start = Section.lCommand[k].Arg1;
                            DWORD End   = Section.lCommand[k].Arg2;

                            #if STRIPE_GEOMS
                            {
                                // Create triangle strips
                                u16 nGroups;
                                PrimitiveGroup* pGroups;
                                GenerateStrips(
                                    XboxDList.pIndex+Start*3,
                                    (End-Start)*3,
                                    &pGroups,
                                    &nGroups );

                                // Increase command buffer if necessary
                                DWORD dwPushSize;
                                if( nGroups > 1 )
                                {
                                    s32 nCmds = XboxDList.nCommands;
                                    XboxDList.nCommands+= nGroups-1;
                                    skin_geom::command_xbox* pNew = new skin_geom::command_xbox[ XboxDList.nCommands ];
                                    x_memcpy(
                                        pNew,
                                        XboxDList.pCmd,
                                        nCmds*sizeof( skin_geom::command_xbox ));
                                    delete[ ]XboxDList.pCmd;
                                    XboxDList.pCmd = pNew;
                                }

                                // Compile all strips
                                for( u16 i=0;i<nGroups;i++ )
                                {
                                    dwPushSize = MAX_TEMP_PUSH_BUFFER-nWritten;
                                    XGCompileDrawIndexedVertices(
                                        ((u8*)pPushBuffer)+nWritten,
                                        &dwPushSize,
                                        D3DPT_TRIANGLESTRIP,
                                        pGroups[i].numIndices,
                                        pGroups[i].indices );

                                    // Save off 
                                    XboxDList.pCmd[k].Cmd = skin_geom::XBOX_CMD_DRAW_SECTION;
                                    XboxDList.pCmd[k].Arg1 = dwPushSize; // NBytes
                                    XboxDList.pCmd[k].Arg2 = nWritten; // Start
                                    nWritten += dwPushSize;
                                }
                                delete[]pGroups;
                            }
                            #else
                            {
                                XGCompileDrawIndexedVertices(
                                    ((u8*)pPushBuffer)+nWritten,
                                    &dwPushSize,
                                    D3DPT_TRIANGLELIST,
                                    (End-Start)*3,
                                    XboxDList.pIndex + Start*3 );
                                XboxDList.pCmd[k].Cmd = skin_geom::XBOX_CMD_DRAW_SECTION;
                                XboxDList.pCmd[k].Arg1 = dwPushSize; // NBytes
                                XboxDList.pCmd[k].Arg2 = nWritten; // Start
                                nWritten += dwPushSize;
                            }
                            #endif

                            // Save strips
                            XboxDList.pPushBuffer=( u8*)x_malloc( nWritten );
                            XboxDList.nPushSize = nWritten;
                            XboxDList.hPushBuffer = NULL;
                            XboxDList.hVert = NULL;
                            x_memcpy(
                                XboxDList.pPushBuffer,
                                pPushBuffer,
                                nWritten
                            );
                        }
                        #endif
                        break;

                    default:
                        x_throw( "Unknown command type while compiling a soft-skin for Xbox" );
                        break;
                }
            }

            //
            //  Copy push buffers into dlist
            //

            if( nWritten )
            {
                XboxDList.pPushBuffer=( u8*)x_malloc( nWritten );
                XboxDList.nPushSize = nWritten;
                XboxDList.hPushBuffer = NULL;
                XboxDList.hVert = NULL;
                x_memcpy(
                    XboxDList.pPushBuffer,
                    pPushBuffer,
                    nWritten
                );
                #ifdef X_DEBUG
                {
                    s32 i;
                    s32 n = XboxDList.nCommands;
                    for( i=0;i<n;i++ )
                    {
                        skin_geom::command_xbox& Cmd = XboxDList.pCmd[i];
                        if( Cmd.Cmd != skin_geom::XBOX_CMD_DRAW_SECTION )
                            continue;
                        u32 Len = Cmd.Arg1; // Push length
                        u32 Off = Cmd.Arg2; // Push offset
                        ASSERT( Off+Len<=XboxDList.nPushSize );
                    }
                }
                #endif
            }
        }
    }
    x_free( pPushBuffer );

    ASSERT( iDList == nXboxDList );

    //
    // Setup the Geom
    //
    
    geom& Geom       = SkinGeom;

    Geom.m_nTextures = 0;
    Geom.m_pTexture  = NULL;
    Geom.m_Platform  = PLATFORM_XBOX;
    
    // Allocate space for all the meshes in the geom
    Geom.m_nMeshes   = SubMeshCount;
    Geom.m_pMesh     = new geom::mesh[ Geom.m_nMeshes ];

    // Allocate space for all the submeshes in the geom
    Geom.m_nSubMeshes = nXboxDList;
    Geom.m_pSubMesh   = new geom::submesh[ Geom.m_nSubMeshes ];
    
    if( (Geom.m_pMesh == NULL) || (Geom.m_pSubMesh == NULL) )
        x_throw( "Out of memory" );

    //
    // Setup mesh and submesh structures
    //

    for( iDList=i=0; i<SubMeshCount; i++ )
    {
        // Setup runtime "geom::mesh" name and bone count
        Geom.m_pMesh[i].NameOffset = (s32)m_Dictionary.Add( Mesh.SubMesh[i].Name );
        Geom.m_pMesh[i].nBones     = Mesh.SubMesh[i].pRawSubMesh->nBones ;

        // 1 submesh per display list
        Geom.m_pMesh[i].nSubMeshes = Mesh.SubMesh[i].lDList.GetCount();
        Geom.m_pMesh[i].iSubMesh   = iDList;
        Geom.m_pMesh[i].nVertices  = 0;
        Geom.m_pMesh[i].nFaces     = 0;
        
        // Set the material and display list index into the submeshes
        for( k=0; k<Mesh.SubMesh[i].lDList.GetCount(); k++, iDList++ )
        {
            // tally up the vert and face counts...the face count
            // is before indexing, and the vert count is after indexing
            // has occured
            const dlist& DList = Mesh.SubMesh[i].lDList[k];
            Geom.m_pMesh[i].nFaces    += DList.lTri.GetCount();
            Geom.m_pMesh[i].nVertices += pXboxDList[iDList].nVerts;

            geom::submesh& SubMesh = Geom.m_pSubMesh[ iDList ];
            
            SubMesh.iDList    = iDList;
            SubMesh.iMaterial = Mesh.SubMesh[i].lDList[k].iMaterial;
        }
    }        

    ASSERT( iDList == nXboxDList );
    
    // Count up the faces and vertices
    Geom.m_nFaces    = 0;
    Geom.m_nVertices = 0;
    for ( i = 0; i < Geom.m_nMeshes; i++ )
    {
        Geom.m_nFaces    += Geom.m_pMesh[i].nFaces;
        Geom.m_nVertices += Geom.m_pMesh[i].nVertices;
    }

    // Setup the SkinGeom
    SkinGeom.m_nBones       = RawMesh.m_nBones;
    SkinGeom.m_nDList       = nXboxDList;
    SkinGeom.m_System.pXbox = pXboxDList;

    //
    // Compute BBox's
    //

    SkinGeom.m_BBox.Clear();
    
    for( i=0; i<SkinGeom.m_nMeshes; i++ )
    {
        geom::mesh& Mesh = SkinGeom.m_pMesh[i];
        Mesh.BBox.Clear();
        
        for( s32 j=0; j<Mesh.nSubMeshes; j++ )
        {
            s32            iSubMesh = Mesh.iSubMesh + j;
            geom::submesh& SubMesh  = SkinGeom.m_pSubMesh[ iSubMesh ];
            
            skin_geom::dlist_xbox& DList = SkinGeom.m_System.pXbox[ SubMesh.iDList ];

            for( k=0; k<DList.nVerts; k++ )
            {
                vector3 Pos = DList.pVert[k].Pos;
                Mesh.BBox += Pos;
            }
        }
        
        SkinGeom.m_BBox += Mesh.BBox;
    }

    //
    // Save the Materials
    //    
    ExportMaterial( Mesh, SkinGeom, PlatformID );

    //
    // Save the virtual meshes
    //
    ExportVirtualMeshes( Mesh, SkinGeom, PlatformID );

    //
    // Compile the dictionary
    //
    CompileDictionary( SkinGeom );

    //
    // Save the data
    //
    fileio File;
    File.Save( m_PlatInfo[ PlatformID ].FileName , SkinGeom, FALSE );
}

//=============================================================================

void geom_compiler::ExportSkinGeomPC( mesh& Mesh, skin_geom& SkinGeom, const char* pFileName )
{
    s32             i,j,k;
    s32             iDList;
    s32             PlatformID = GetPlatformIndex( PLATFORM_PC );

    //
    // Get the total count of facet and indices and dlists
    //
    
    s32 nPCDList = 0;
    for( i=0; i<Mesh.SubMesh.GetCount(); i++ )
    {
        const sub_mesh& SubMesh = Mesh.SubMesh[i];
        nPCDList += SubMesh.lDList.GetCount();
    }

    skin_geom::dlist_pc* pPCDList = new skin_geom::dlist_pc[ nPCDList ];
    if( pPCDList == NULL )
        x_throw( "Out of memory" );

    ASSERT( Mesh.SubMesh[0].pRawMesh );
    const rawmesh2& RawMesh = *Mesh.SubMesh[0].pRawMesh;

    for( iDList=i=0; i<Mesh.SubMesh.GetCount(); i++ )
    {
        const sub_mesh& SubMesh = Mesh.SubMesh[i];
        for( j=0; j<SubMesh.lDList.GetCount(); j++ )
        {
            const dlist& DList = SubMesh.lDList[j];

            arm_optimizer Optimizer;
            Optimizer.Build( RawMesh, DList.lTri, (96-10)/4 ); // Leave room for 10 general registers, and 4 reg per bone

            arm_optimizer::section& Section = Optimizer.m_Section;
            
            //
            // Copy all the indices
            //

            skin_geom::dlist_pc& PCDList = pPCDList[ iDList++ ];
            
            PCDList.nIndices = Section.lTriangle.GetCount() * 3;
            PCDList.pIndex   = new s16[ PCDList.nIndices ];
            if( PCDList.pIndex == NULL )
                x_throw( "Out of memory" );
            
            for( k=0; k<Section.lTriangle.GetCount(); k++ )
            {
                PCDList.pIndex[ (k*3) + 0 ] = Section.lTriangle[k].iVertex[0];
                PCDList.pIndex[ (k*3) + 1 ] = Section.lTriangle[k].iVertex[1];
                PCDList.pIndex[ (k*3) + 2 ] = Section.lTriangle[k].iVertex[2];
            }
            
            //
            // Copy down all the verts
            //
            
            PCDList.nVertices = Section.lVertex.GetCount();
            PCDList.pVertex   = new skin_geom::vertex_pc[ PCDList.nVertices ];
            if( PCDList.pVertex == NULL )
                x_throw( "Out of memory");

            for( k=0; k<PCDList.nVertices; k++ )
            {
                PCDList.pVertex[k].Position        = Section.lVertex[k].Position;
                PCDList.pVertex[k].Position.GetW() = (f32)Section.lVertex[k].Weight[0].iBone;
            
                PCDList.pVertex[k].Normal = Section.lVertex[k].Normal[0];
            
                if( Section.lVertex[k].nWeights > 1 )
                    PCDList.pVertex[k].Normal.GetW() = (f32)Section.lVertex[k].Weight[1].iBone;
                else
                    PCDList.pVertex[k].Normal.GetW() = 0.0f;
            
                PCDList.pVertex[k].UVWeights.GetX() = Section.lVertex[k].UV[0].X;
                PCDList.pVertex[k].UVWeights.GetY() = Section.lVertex[k].UV[0].Y;
            
                PCDList.pVertex[k].UVWeights.GetZ() = Section.lVertex[k].Weight[0].Weight;
                if( Section.lVertex[k].nWeights > 1 )
                    PCDList.pVertex[k].UVWeights.GetW() = Section.lVertex[k].Weight[1].Weight;
                else
                    PCDList.pVertex[k].UVWeights.GetW() = 0.0f;
            }
            
            //
            // Copy down all the commands
            //
            
            PCDList.nCommands = Section.lCommand.GetCount();
            PCDList.pCmd      = new skin_geom::command_pc[ PCDList.nCommands ];
            if( PCDList.pCmd == NULL )
                x_throw( "Out of memory" );
            
            for( k=0; k<PCDList.nCommands; k++ )
            {
                // Copy the command type
                switch( Section.lCommand[k].Type )
                {
                case arm_optimizer::DRAW_LIST: 
                    PCDList.pCmd[k].Cmd =  skin_geom::PC_CMD_DRAW_SECTION;
                    break;
            
                case arm_optimizer::UPLOAD_MATRIX: 
                    PCDList.pCmd[k].Cmd =  skin_geom::PC_CMD_UPLOAD_MATRIX;
                    break;
            
                default:
                    x_throw( "Unknown command type while compiling a soft-skin for PC" );
                }
            
                // copy the arguments
                PCDList.pCmd[k].Arg1 = (s16)Section.lCommand[k].Arg1;
                PCDList.pCmd[k].Arg2 = (s16)Section.lCommand[k].Arg2;
            }
        }
    }

    ASSERT( iDList == nPCDList );

    //
    // Setup the Geom
    //
    
    geom& Geom       = SkinGeom;

    Geom.m_nTextures = 0;
    Geom.m_pTexture  = NULL;
    Geom.m_Platform  = PLATFORM_PC;
    
    // Allocate space for all the meshes in the geom
    Geom.m_nMeshes   = Mesh.SubMesh.GetCount();
    Geom.m_pMesh     = new geom::mesh[ Geom.m_nMeshes ];

    // Allocate space for all the submeshes in the geom
    Geom.m_nSubMeshes = nPCDList;
    Geom.m_pSubMesh   = new geom::submesh[ Geom.m_nSubMeshes ];
    
    if( (Geom.m_pMesh == NULL) || (Geom.m_pSubMesh == NULL) )
        x_throw( "Out of memory" );

    //
    // Setup mesh and submesh structures
    //

    for( iDList=i=0; i<Mesh.SubMesh.GetCount(); i++ )
    {
        // Setup runtime "geom::mesh" name and bone count
        Geom.m_pMesh[i].NameOffset = m_Dictionary.Add( Mesh.SubMesh[i].Name );
        Geom.m_pMesh[i].nBones = Mesh.SubMesh[i].pRawSubMesh->nBones ;

        // 1 submesh per display list
        Geom.m_pMesh[i].nSubMeshes = Mesh.SubMesh[i].lDList.GetCount();
        Geom.m_pMesh[i].iSubMesh   = iDList;
        Geom.m_pMesh[i].nVertices  = 0;
        Geom.m_pMesh[i].nFaces     = 0;
        
        // Set the material and display list index into the submeshes
        for( k=0; k<Mesh.SubMesh[i].lDList.GetCount(); k++, iDList++ )
        {
            // tally up the vert and face counts...the face count
            // is before indexing, and the vert count is after indexing
            // has occured
            const dlist& DList = Mesh.SubMesh[i].lDList[k];
            Geom.m_pMesh[i].nFaces    += DList.lTri.GetCount();
            Geom.m_pMesh[i].nVertices += pPCDList[iDList].nVertices;

            geom::submesh& SubMesh = Geom.m_pSubMesh[ iDList ];
            
            SubMesh.iDList    = iDList;
            SubMesh.iMaterial = Mesh.SubMesh[i].lDList[k].iMaterial;
        }
    }        

    ASSERT( iDList == nPCDList );
    
    // Count up the faces and vertices
    Geom.m_nFaces    = 0;
    Geom.m_nVertices = 0;
    for ( i = 0; i < Geom.m_nMeshes; i++ )
    {
        Geom.m_nFaces    += Geom.m_pMesh[i].nFaces;
        Geom.m_nVertices += Geom.m_pMesh[i].nVertices;
    }

    // Setup the SkinGeom
    SkinGeom.m_nBones     = RawMesh.m_nBones;
    SkinGeom.m_nDList     = nPCDList;
    SkinGeom.m_System.pPC = pPCDList;

    //
    // Compute BBox's
    //

    SkinGeom.m_BBox.Clear();
    
    for( i=0; i<SkinGeom.m_nMeshes; i++ )
    {
        geom::mesh& Mesh = SkinGeom.m_pMesh[i];
        Mesh.BBox.Clear();
        
        for( s32 j=0; j<Mesh.nSubMeshes; j++ )
        {
            s32            iSubMesh = Mesh.iSubMesh + j;
            geom::submesh& SubMesh  = SkinGeom.m_pSubMesh[ iSubMesh ];
            
            skin_geom::dlist_pc& DList = SkinGeom.m_System.pPC[ SubMesh.iDList ];

            for( k=0; k<DList.nVertices; k++ )
            {
                vector3 Pos;
                Pos = DList.pVertex[k].Position;
                Mesh.BBox += Pos;
            }
        }
        
        SkinGeom.m_BBox += Mesh.BBox;
    }

    //
    // Save the Materials
    //    
    ExportMaterial( Mesh, SkinGeom, PlatformID );

    //
    // Save the virtual meshes
    //
    ExportVirtualMeshes( Mesh, SkinGeom, PlatformID );

    //
    // Compile the dictionary
    //
    CompileDictionary( SkinGeom );

    //
    // Save the data
    //
    
    fileio File;
    File.Save( m_PlatInfo[ PlatformID ].FileName , SkinGeom, FALSE );
}

//=============================================================================

void geom_compiler::BuildTexturePath( char* pPath, const char* pName, s32 PlatformID )
{
    char pFilename[256];
    char pDrive[256];
    char pDir  [256];
    x_splitpath( pName, NULL, NULL, pFilename, NULL );
    x_splitpath( m_PlatInfo[ PlatformID ].FileName, pDrive, pDir, NULL, NULL );
    x_makepath ( pPath, pDrive, pDir, pFilename, ".xbmp" );
}

//=============================================================================

void geom_compiler::ExportDiffuse( const xbitmap& Bitmap, const char* pName, pref_bpp PrefBPP, s32 nMips, s32 PlatformID )
{
    // Make a copy of the bitmap since we are going to modify it
    xbitmap BMPToSave = Bitmap;

    // Convert the texture to the correct format
    switch( m_PlatInfo[PlatformID].Platform )
    {
        case PLATFORM_XBOX:
            auxbmp_Compress( BMPToSave,pName,nMips );
            break;

    case PLATFORM_PC:
        auxbmp_ConvertToD3D( BMPToSave );
        break;

    case PLATFORM_PS2:
        switch ( PrefBPP )
        {
        default:
            ASSERT( FALSE );

        case PREF_BPP_DEFAULT:
        case PREF_BPP_8:
            BMPToSave.ConvertFormat( xbitmap::FMT_P8_ABGR_8888 );
            break;

        case PREF_BPP_32:
            BMPToSave.ConvertFormat( xbitmap::FMT_32_ABGR_8888 );
            break;

        case PREF_BPP_16:
            BMPToSave.ConvertFormat( xbitmap::FMT_16_ABGR_1555 );
            break;

        case PREF_BPP_4:
            BMPToSave.ConvertFormat( xbitmap::FMT_P4_ABGR_8888 );
            break;
        }
        if ( !BMPToSave.GetNMips()  )
            BMPToSave.BuildMips( nMips );

        if ( g_ColoredMips )
            bmp_util::ConvertToColoredMips( BMPToSave );

        bmp_util::ConvertToPS2( BMPToSave, TRUE );
        break;
    }

    // Save the bitmap
    char pPath[256];
    BuildTexturePath( pPath, pName, PlatformID );
    BMPToSave.Save( pPath );
}

//=============================================================================

void geom_compiler::ExportEnvironment( const xbitmap& Bitmap, const char* pName, s32 PlatformID )
{
    // Make a copy of the bitmap since we are going to modify it
    xbitmap BMPToSave = Bitmap;

    // Convert the texture to the correct format
    switch( m_PlatInfo[PlatformID].Platform )
    {
    case PLATFORM_XBOX:
        auxbmp_Compress( BMPToSave,pName,4 );
        break;

    case PLATFORM_PC:
        auxbmp_ConvertToD3D( BMPToSave );
        break;

    case PLATFORM_PS2:
        if ( BMPToSave.GetBPP() > 8 )
            bmp_util::ConvertToPalettized(BMPToSave, 8);
        BMPToSave.ConvertFormat( xbitmap::FMT_P8_ABGR_8888 );
        bmp_util::ConvertToPS2( BMPToSave, TRUE );
        break;
    }

    // Save the bitmap
    char pPath[256];
    BuildTexturePath( pPath, pName, PlatformID );
    BMPToSave.Save( pPath );
}

//=============================================================================

void geom_compiler::ExportDetail( const xbitmap& Bitmap, const char* pName, s32 PlatformID )
{
    // Make a copy of the bitmap since we are going to modify it
    xbitmap BMPToSave = Bitmap;

    // Convert the texture to the correct format
    switch( m_PlatInfo[PlatformID].Platform )
    {
    case PLATFORM_XBOX:
        bmp_util::ProcessDetailMap( BMPToSave, FALSE );
        BMPToSave = auxbmp_ConvertRGBToA8( BMPToSave );
        break;

    case PLATFORM_PC:
        bmp_util::ProcessDetailMap( BMPToSave, FALSE );
        auxbmp_ConvertToD3D( BMPToSave );
        break;

    case PLATFORM_PS2:
        bmp_util::ProcessDetailMap( BMPToSave, TRUE );
        ASSERT( BMPToSave.GetBPP() <= 8 );
        BMPToSave.ConvertFormat( xbitmap::FMT_P4_ABGR_8888 );
        bmp_util::ConvertToPS2( BMPToSave, FALSE );
        break;
    }

    // Save the bitmap
    char pPath[256];
    BuildTexturePath( pPath, pName, PlatformID );
    BMPToSave.Save( pPath );
}

//=============================================================================

void geom_compiler::ProcessPunchThruMap( map_info& DiffuseMap, const char* pPunchThruMap )
{
    // load the punch through map
    xbitmap PunchMap;
    if( LoadBitmap( PunchMap, pPunchThruMap ) == FALSE )
    {
        ThrowError( xfs( "Unable to load punch-through map (%s)", pPunchThruMap ) );
    }

    PunchMap.ConvertFormat( xbitmap::FMT_32_ARGB_8888 );
    if( bmp_util::SetPunchThrough( DiffuseMap.Bitmap, PunchMap ) == FALSE )
    {
        ThrowError( xfs( "Diffuse [%s] and PunchThru texture [%s] have different dimensions",
                    (const char*)DiffuseMap.InputBitmapName,
                    pPunchThruMap ) );
    }
}

//=============================================================================

void geom_compiler::ExportUVAnimation( const rawmesh2::material& RawMat,
                                       const f32*                pParamKey,
                                       geom::material&           Material,
                                       geom&                     Geom )
{
    const rawmesh2::param_pkg& Param  = RawMat.Map[ Max_Diffuse1 ].UVTranslation;
    geom::material::uvanim&    UVAnim = Material.UVAnim;

    x_memset( &UVAnim, 0, sizeof( UVAnim ) );

    // Check if we have any animation data
    if( Param.nKeys == 0 )
        return;
    
    // For now we will only handle U and V pairs
    if( Param.nParamsPerKey != 2 )
        ThrowError( xfs( "Number of UV Animation Params per Key is not 2! [%s]\n", RawMat.Name ) );
    
    x_DebugMsg( "nKeys=%d iFirstKey=%d nParamsPerKey=%d FPS=%d\n",
        Param.nKeys,
        Param.iFirstKey,
        Param.nParamsPerKey,
        Param.FPS );

    // try to match up the uv animation from the resource description
    const geom_rsc_desc::uv_animation* pRscUVAnim = m_pGeomRscDesc->GetUVAnimation( RawMat.Name );
    if( pRscUVAnim )
    {
        UVAnim.Type       = (s8)pRscUVAnim->AnimType;
        UVAnim.StartFrame = (s8)pRscUVAnim->StartFrame;
        UVAnim.FPS        = (s8)pRscUVAnim->FPS;
        UVAnim.iKey       = Geom.m_nUVKeys;
        UVAnim.nKeys      = Param.nKeys;
    }
    else
    {
        UVAnim.Type       = geom::material::uvanim::LOOPED;
        UVAnim.StartFrame = 0;
        UVAnim.FPS        = 30;
        UVAnim.iKey       = Geom.m_nUVKeys;
        UVAnim.nKeys      = Param.nKeys;
    }
    
    // Copy the UV key pairs into the Geom
    for( s32 i=0; i<Param.nKeys * Param.nParamsPerKey; i += Param.nParamsPerKey )
    {
        s32 iKey = RawMat.iFirstKey + i;
        
        Geom.m_pUVKey[ Geom.m_nUVKeys ].OffsetU = (u8)(x_fmod( pParamKey[ iKey + 0 ], 1.0f ) * 255.0f);
        Geom.m_pUVKey[ Geom.m_nUVKeys ].OffsetV = (u8)(x_fmod( pParamKey[ iKey + 1 ], 1.0f ) * 255.0f);
        
        Geom.m_nUVKeys++;
    }
}

//=============================================================================

void geom_compiler::ReadBitmap( map_info& MapInfo )
{
    // Load the bitmap
    if( LoadBitmap( MapInfo.Bitmap, MapInfo.InputBitmapName ) == FALSE )
        ThrowError( xfs( "Unable to load texture [%s]", (const char*)MapInfo.InputBitmapName ) );

    // Convert to ARGB and perform sanity checks on loaded bitmaps
    if( MapInfo.bOutOfDate )
    {
        MapInfo.Bitmap.ConvertFormat( xbitmap::FMT_32_ARGB_8888 );

        // check the overall dimension size
        s32 W = MapInfo.Bitmap.GetWidth();
        s32 H = MapInfo.Bitmap.GetHeight();
        if( (W > MaxTextureWidth) || (H > MaxTextureHeight) )
        {
            ThrowError( xfs( "Texture [%s] (%d x %d) is too big. Max size is (%d x %d)",
                        (const char*)MapInfo.InputBitmapName,
                        W, H, MaxTextureWidth, MaxTextureHeight ) );
        }

        // check for a power of 2
        if( (((W-1) & W) != 0) ||
            (((H-1) & H) != 0) )
        {
            ThrowError( xfs( "Texture [%s] (%d x %d) must have a power of 2 width and height",
                        (const char*)MapInfo.InputBitmapName,
                        W, H, MaxTextureWidth, MaxTextureHeight ) );
        }
    }
}

//=============================================================================

void geom_compiler::ComputeMapNames( geom_compiler::map_slot& Map,
                                     s32                      MapType,
                                     u32                      CheckSum,
                                     pref_bpp                 PrefBPP,
                                     const char*              pInputFile,
                                     platform                 PlatformId )
{
    // calculate the output path...we'll need that later
    char OutputDrive[X_MAX_DRIVE];
    char OutputDir[X_MAX_DIR];
    x_splitpath( m_PlatInfo[PlatformId].FileName, OutputDrive, OutputDir, NULL, NULL );

    // calculate the filename with the path stripped off for virtual
    // texture comparisons
    char InputDrive[X_MAX_DRIVE];
    char InputDir[X_MAX_DIR];
    char InputFName[X_MAX_FNAME];
    char InputExt[X_MAX_EXT];
    char InputFNameExt[X_MAX_PATH];
    x_splitpath( pInputFile, InputDrive, InputDir, InputFName, InputExt );
    x_makepath( InputFNameExt, NULL, NULL, InputFName, InputExt );

    // check if this map will correspond to a virtual texture (note that
    // only diffuse slots get the virtual texture)
    s32 iVTexture = -1;
    if( MapType == Max_Diffuse1 )
    {
        s32 i;
        for( i = 0; i < m_pGeomRscDesc->GetVirtualTextureCount(); i++ )
        {
            const geom_rsc_desc::virtual_texture& VTexture = m_pGeomRscDesc->GetVirtualTexture(i);

            // is this even a valid virtual texture?
            if( VTexture.Textures.GetCount() == 0 )
                continue;

            // does this virtual texture match with our input filename?
            if( !x_stricmp( VTexture.Textures[0].FileName, InputFNameExt ) )
            {
                iVTexture = i;
                break;
            }
        }
    }

    // the first texture always comes directly from the geometry
    map_info& MapInfo = Map.MapList.Append();

    // the input name is just a straight copy
    MapInfo.InputBitmapName = pInputFile;

    // figure out what the bitmap name will be decorated with, based on its
    // checksum and map usage
    char NameDecoration[X_MAX_PATH];
    x_strcpy( NameDecoration, "[" );
    if( MapType == Max_DetailMap )
    {
        x_strcat( NameDecoration, "D" );
    }
    else if( MapType == Max_Environment )
    {
        x_strcat( NameDecoration, "E" );
    }
    else
    {
        switch( PrefBPP )
        {
        default:
        case PREF_BPP_32:       x_strcat( NameDecoration, "32_" );  break;
        case PREF_BPP_16:       x_strcat( NameDecoration, "16_" );  break;
        case PREF_BPP_8:        x_strcat( NameDecoration, "8_" );   break;
        case PREF_BPP_4:        x_strcat( NameDecoration, "4_" );   break;
        case PREF_BPP_DEFAULT:  x_strcat( NameDecoration, "8_" );   break;
        }
    }
    x_strcat( NameDecoration, xfs("%04X", CheckSum&0xffff) );
    x_strcat( NameDecoration, "]" );

    // the compiled name is decorated with the checksum, is an xbmp, and has
    // had the path stripped off
    char CompiledName[X_MAX_PATH];
    x_makepath( CompiledName, NULL, NULL, xfs("%s%s", InputFName, NameDecoration), "xbmp" );
    MapInfo.CompiledBitmapName = CompiledName;

    // and the output name contains the full path to the output file
    char OutputName[X_MAX_PATH];
    x_makepath( OutputName, OutputDrive, OutputDir, xfs("%s%s", InputFName, NameDecoration), "xbmp" );
    MapInfo.OutputBitmapName = OutputName;

    // assume the time stamp is good to start with (we'll be checking those later!)
    MapInfo.bOutOfDate = FALSE;

    // additional textures will come from the virtual texture
    if( iVTexture != -1 )
    {
        char Drive[X_MAX_DRIVE];
        char Dir[X_MAX_DIR];
        char FName[X_MAX_FNAME];
        char Ext[X_MAX_EXT];

        // If we've overridden the default texture, we need to take care of that now.
        // An artist might choose to do this to save memory. For example, if the
        // default bitmap is used in 9 levels, but the 10th level only uses the
        // other virtual texture positions, then they would do this to avoid
        // the need for custom max objects just for that 10th level.
        const geom_rsc_desc::virtual_texture& VTexture = m_pGeomRscDesc->GetVirtualTexture(iVTexture);
        if( VTexture.OverrideDefault )
        {
            if( x_strlen( VTexture.OverrideFileName ) == 0 )
            {
                ThrowError( xfs("Diffuse override filename is invalid for virtual texture[%d] - [%s]", iVTexture, VTexture.Name) );
            }

            // the input name is just a straight copy from the override
            MapInfo.InputBitmapName = VTexture.OverrideFileName;

            // the compiled name is decorated with the checksum, is an xbmp, and has
            // had the path stripped off
            x_splitpath( MapInfo.InputBitmapName, Drive, Dir, FName, Ext );
            x_makepath( CompiledName, NULL, NULL, xfs("%s%d", FName, CheckSum), "xbmp" );
            MapInfo.CompiledBitmapName = CompiledName;

            // and the output name contains the full path to the output file
            x_makepath( OutputName, OutputDrive, OutputDir, xfs("%s%d", FName, CheckSum), "xbmp" );
            MapInfo.OutputBitmapName = OutputName;

            // assume the time stamp is good to start with (we'll be checking those later!)
            MapInfo.bOutOfDate = FALSE;
        }

        // and now handle the rest of the virtual textures
        s32 i;
        for( i = 1; i < VTexture.Textures.GetCount(); i++ )
        {
            const geom_rsc_desc::texture_info& TexInfo = VTexture.Textures[i];
            
            // add this virtual texture
            map_info& VMapInfo = Map.MapList.Append();

            // the input name is just a straight copy
            VMapInfo.InputBitmapName = TexInfo.FileName;

            if( x_strlen( VMapInfo.InputBitmapName ) == 0 )
            {
                ThrowError( xfs("Invalid diffuse map for virtual texture[%d,%d] - [%s]", iVTexture, i, VTexture.Name) );
            }

            // the compiled name is decorated with the checksum, is an xbmp, and has
            // had the path stripped off
            x_splitpath( TexInfo.FileName, Drive, Dir, FName, Ext );
            x_makepath( CompiledName, NULL, NULL, xfs("%s%d", FName, CheckSum), "xbmp" );
            VMapInfo.CompiledBitmapName = CompiledName;

            // and the output name contains the full path to the output file
            x_makepath( OutputName, OutputDrive, OutputDir, xfs("%s%d", FName, CheckSum), "xbmp" );
            VMapInfo.OutputBitmapName = OutputName;

            // assume the time stamp is good to start with (we'll be checking those later!)
            VMapInfo.bOutOfDate = FALSE;
        }
    }
}

//=============================================================================

void geom_compiler::CheckTimeStamps( map_slot& Map, const char* pMixMap )
{
    s32     TimeBias      = 10;   // This is use to help for the multiprocessing. 
                                  // What happens is that while one process is saving the file the other one
                                  // is trying to load it! ;-(

    s32 i;
    for( i = 0; i < Map.MapList.GetCount(); i++ )
    {
        struct _finddata_t  SourceData;
        struct _finddata_t  MixData;
        struct _finddata_t  DestData;
        long                hSubFile;

        // assume we are up-to-date
        map_info& MapInfo = Map.MapList[i];
        MapInfo.bOutOfDate = FALSE;

        // get the source data information
        hSubFile = _findfirst( MapInfo.InputBitmapName, &SourceData );
        if( hSubFile == -1 ) 
        {
            ThrowError( xfs("Unable to load [%s]",MapInfo.InputBitmapName) );
        }
        _findclose( hSubFile );

        // attempt to get the dest data information
        hSubFile = _findfirst( MapInfo.OutputBitmapName, &DestData );
        if( hSubFile == -1 )
        {
            // output file doesn't exist
            MapInfo.bOutOfDate = TRUE;
        }
        else
        {
            _findclose( hSubFile );

            // compare all the timestamps to see whether this bitmap needs
            // to be recompiled
            if( (SourceData.time_write > (DestData.time_write-TimeBias)) ||
                (g_ExeData.time_write > (DestData.time_write-TimeBias)) )
            {
                MapInfo.bOutOfDate = TRUE;
            }
            
            // check the timestamp of the mixed map if necessary
            if( pMixMap )
            {
                hSubFile = _findfirst( pMixMap, &MixData );
                if( hSubFile == -1 )
                {
                    ThrowError( xfs("Unable to load [%s]", pMixMap ) );
                }
                _findclose( hSubFile );

                if( MixData.time_write > (DestData.time_write-TimeBias) )
                {
                    MapInfo.bOutOfDate = TRUE;
                }
            }
        }
    }
}

//=============================================================================

void geom_compiler::FillMapSlots( mesh& Mesh, platform PlatformId )
{
    s32 i, j;
    for( i = 0; i <  Mesh.Material.GetCount(); i++ )
    {
        // Get the material from the RawMesh
        material&           Mat     = Mesh.Material[i];
        const rawmesh2&     RawMesh = *Mat.pRawMesh;
        rawmesh2::material& RawMat  = RawMesh.m_pMaterial[Mat.iRawMaterial];

        //
        // Determine whether or not we need an environment map
        //        
        xbool   bNeedsEnviroment = FALSE;

        switch( RawMat.Type )
        {
        case Material_Diff : 
        case Material_Alpha :
        case Material_Diff_PerPixelIllum :
        case Material_Alpha_PerPixelIllum :
        case Material_Alpha_PerPolyIllum :
        case Material_Distortion :
            break;

        case Material_Distortion_PerPolyEnv :
            bNeedsEnviroment = TRUE;
            break;

        case Material_Diff_PerPixelEnv :
        case Material_Alpha_PerPolyEnv :
            if ( RawMat.Constants[MaxConst_EnvType].Current[0] != 0.0f )
            {
                bNeedsEnviroment = TRUE;
            }
            break;

        default :
            ThrowError( xfs( "Unknown Material Type %d", RawMat.Type ) );
            break;
        }

        // Set whether we have a punch-through image
        xbool bHasPunchThrough = (RawMat.Map[Max_PunchThrough].iTexture) != -1;

        // Set up the texture names and compile them if necessary
        for( j = 0; j < NumMaps; j++ )
        {
            // some maps don't need to be compiled at all or are not supported
            if ( (j == Max_Diffuse2)         ||
                 (j == Max_Blend)            ||
                 (j == Max_LightMap)         ||
                 (j == Max_Opacity)          ||
                 (j == Max_Intensity)        ||
                 (j == Max_SelfIllumination) ||
                 (j == Max_PunchThrough) )
            {
                continue;
            }

            // Get the texture index from the material
            s32 iTexture = RawMat.Map[j].iTexture;

            // check for a valid texture index
            if( iTexture >= RawMesh.m_nTextures )
                ThrowError( xfs( "Invalid texture index %d (max %d)\n", iTexture, RawMesh.m_nTextures ) );

            // Is this map used?
            if( iTexture < 0 )
                continue;

            // make sure this map is really necessary
            if( !bNeedsEnviroment && (j == Max_Environment) )
            {
                Mat.Maps[j].MapList.Clear();
                continue;
            }

            // check validity of texture path
            const char* pFileName = RawMesh.m_pTexture[iTexture].FileName;
            if( x_stristr( pFileName, m_TexturePath ) == 0 )
            {
                ThrowError( xfs("Texture [%s] is not in path [%s]", pFileName, m_TexturePath) );
            }

            // If we're baking in a punch-through map, we'll need to append all the compiled
            // files with a checksum. Calculate that now.
            u32 CheckSum = 0;
            if( (j == Max_Diffuse1) && bHasPunchThrough )
            {
                s32 Index = RawMat.Map[Max_PunchThrough].iTexture;
                char PathName[256];
                x_strcpy( PathName, RawMesh.m_pTexture[Index].FileName );
                x_strtoupper( PathName );
                CheckSum += x_chksum( PathName, x_strlen( PathName ) );
            }

            // fill in the map names and check the timestamps
            Mat.Maps[j].MapList.Clear();
            if( j == Max_Diffuse1 )
            {
                ComputeMapNames( Mat.Maps[j], j, CheckSum, Mat.TexInfo.PreferredBPP, RawMesh.m_pTexture[iTexture].FileName, PlatformId );
                if( bHasPunchThrough )
                {
                    s32 Index = RawMat.Map[Max_PunchThrough].iTexture;
                    CheckTimeStamps( Mat.Maps[j], RawMesh.m_pTexture[Index].FileName );
                }
                else
                {
                    CheckTimeStamps( Mat.Maps[j], NULL );
                }
            }
            else
            {
                ComputeMapNames( Mat.Maps[j], j, CheckSum, Mat.TexInfo.PreferredBPP, RawMesh.m_pTexture[iTexture].FileName, PlatformId );
                CheckTimeStamps( Mat.Maps[j], NULL );
            }
        }

        // process the diffuse map(s)
        if( Mat.Maps[Max_Diffuse1].MapList.GetCount() == 0 )
        {
            ThrowError( "No diffuse map specified!" );
        }
        else
        {
            for( j = 0; j < Mat.Maps[Max_Diffuse1].MapList.GetCount(); j++ )
            {
                map_info& MapInfo = Mat.Maps[Max_Diffuse1].MapList[j];
                ReadBitmap( MapInfo );
                if( MapInfo.bOutOfDate )
                {
                    if( bHasPunchThrough )
                    {
                        s32 Index = RawMat.Map[Max_PunchThrough].iTexture;
                        ProcessPunchThruMap( MapInfo, RawMesh.m_pTexture[Index].FileName );
                    }

                    ExportDiffuse( MapInfo.Bitmap, MapInfo.OutputBitmapName, Mat.TexInfo.PreferredBPP, Mat.TexInfo.nMipsToBuild, PlatformId );
                }
            }
        }

        // process the environment map
        if( bNeedsEnviroment )
        {
            if( Mat.Maps[Max_Environment].MapList.GetCount() == 0 )
            {
                ThrowError( "No environment map specified!" );
            }
            else
            {
                map_info& MapInfo = Mat.Maps[Max_Environment].MapList[0];
                ReadBitmap( MapInfo );
                if( MapInfo.bOutOfDate )
                {
                    ExportEnvironment( MapInfo.Bitmap, MapInfo.OutputBitmapName, PlatformId );
                }
            }
        }

        // process the detail map
        if( Mat.Maps[Max_DetailMap].MapList.GetCount() )
        {
            map_info& MapInfo = Mat.Maps[Max_DetailMap].MapList[0];
            ReadBitmap( MapInfo );
            if( MapInfo.bOutOfDate )
            {
                ExportDetail( MapInfo.Bitmap, MapInfo.OutputBitmapName, PlatformId );
            }
        }
    }
}

//=============================================================================

void geom_compiler::ExportMaterial( mesh& Mesh, geom& Geom, s32 PlatformID )
{
    s32 i, j, k;

    // generate the compiled bitmaps and fill in the mat slots appropriately
    FillMapSlots( Mesh, (platform)PlatformID );

    // allocate space for the materials
    Geom.m_nMaterials = Mesh.Material.GetCount();
    Geom.m_pMaterial  = new geom::material[Geom.m_nMaterials];

    // allocate space for the uv animations
    Geom.m_nUVKeys    = 0;
    Geom.m_pUVKey     = new geom::uvkey[ MAX_UV_KEYS ];

    // build up the materials
    s32 nTotalTextures = 0;
    s32 nTotalVMats    = 0;
    for( i = 0; i < Mesh.Material.GetCount(); i++ )
    {
        material&           MeshMat = Mesh.Material[i];
        rawmesh2::material& RawMat  = MeshMat.pRawMesh->m_pMaterial[MeshMat.iRawMaterial];
        geom::material&     Mat     = Geom.m_pMaterial[i];

        // fill in the basic material info
        Mat.DetailScale  = RawMat.Constants[DETAIL_SCALE].Current[0];
        Mat.FixedAlpha   = RawMat.Constants[FIXED_ALPHA].Current[0];
        Mat.Flags        = 0;
        Mat.Type         = RawMat.Type;
        Mat.nTextures    = MeshMat.Maps[Max_Diffuse1].MapList.GetCount() +
                           MeshMat.Maps[Max_Environment].MapList.GetCount() +
                           MeshMat.Maps[Max_DetailMap].MapList.GetCount();
        Mat.iTexture     = nTotalTextures;
        Mat.nVirtualMats = MeshMat.Maps[Max_Diffuse1].MapList.GetCount();
        Mat.iVirtualMat  = nTotalVMats;
        if( RawMat.bTwoSided )
            Mat.Flags |= geom::material::FLAG_DOUBLE_SIDED;
        if( MeshMat.Maps[Max_Environment].MapList.GetCount() )
            Mat.Flags |= geom::material::FLAG_HAS_ENV_MAP;
        if( MeshMat.Maps[Max_DetailMap].MapList.GetCount() )
            Mat.Flags |= geom::material::FLAG_HAS_DETAIL_MAP;
        if( RawMat.Constants[ENV_TYPE].Current[0] == 0.0f )
            Mat.Flags |= geom::material::FLAG_ENV_CUBE_MAP;
        else if( RawMat.Constants[ENV_TYPE].Current[0] == 1.0f )
            Mat.Flags |= geom::material::FLAG_ENV_VIEW_SPACE;
        else
            Mat.Flags |= geom::material::FLAG_ENV_WORLD_SPACE;
        if( RawMat.Constants[FORCE_ZFILL].Current[0] )
            Mat.Flags |= geom::material::FLAG_FORCE_ZFILL;
        if( RawMat.Constants[USE_DIFFUSE].Current[0] )
            Mat.Flags |= geom::material::FLAG_ILLUM_USES_DIFFUSE;
        if( RawMat.Map[Max_PunchThrough].iTexture >= 0 )
            Mat.Flags |= geom::material::FLAG_IS_PUNCH_THRU;
        if( RawMat.Constants[ENV_BLEND].Current[0] == 1.0f )
            Mat.Flags |= geom::material::FLAG_IS_ADDITIVE;
        else if( RawMat.Constants[ENV_BLEND].Current[0] == 2.0f )
            Mat.Flags |= geom::material::FLAG_IS_SUBTRACTIVE;

        // copy out the uv animation data
        ExportUVAnimation( RawMat, MeshMat.pRawMesh->m_pParamKey, Mat, Geom );

        // and increment our counters
        nTotalTextures += Mat.nTextures;
        nTotalVMats    += Mat.nVirtualMats;
    }

    // create space for the virtual materials
    Geom.m_nVirtualMaterials = nTotalVMats;
    Geom.m_pVirtualMaterials = new geom::virtual_material[nTotalVMats];

    // allocate space for the textures
    Geom.m_nTextures = nTotalTextures;
    Geom.m_pTexture  = new geom::texture[nTotalTextures];

    // set up the texture file names
    nTotalTextures = 0;
    for( i = 0; i < Mesh.Material.GetCount(); i++ )
    {
        material&           MeshMat = Mesh.Material[i];
        geom::material&     Mat     = Geom.m_pMaterial[i];

        // set up the diffuse texture
        for( j = 0; j < Mat.nVirtualMats; j++ )
        {
            Geom.m_pTexture[nTotalTextures].DescOffset     = m_Dictionary.Add( "" );
            Geom.m_pTexture[nTotalTextures].FileNameOffset = m_Dictionary.Add( MeshMat.Maps[Max_Diffuse1].MapList[j].CompiledBitmapName );
            nTotalTextures++;
        }

        // set up the environment map
        if( MeshMat.Maps[Max_Environment].MapList.GetCount() )
        {
            Geom.m_pTexture[nTotalTextures].DescOffset     = m_Dictionary.Add( "" );
            Geom.m_pTexture[nTotalTextures].FileNameOffset = m_Dictionary.Add( MeshMat.Maps[Max_Environment].MapList[0].CompiledBitmapName );
            nTotalTextures++;
        }

        // set up the detail map
        if( MeshMat.Maps[Max_DetailMap].MapList.GetCount() )
        {
            Geom.m_pTexture[nTotalTextures].DescOffset     = m_Dictionary.Add( "" );
            Geom.m_pTexture[nTotalTextures].FileNameOffset = m_Dictionary.Add( MeshMat.Maps[Max_DetailMap].MapList[0].CompiledBitmapName );
            nTotalTextures++;
        }
    }
    ASSERT( nTotalTextures == Geom.m_nTextures );

    // create a list of virtual textures based on materials
    xarray<geom::virtual_texture> VirtualTextures;
    VirtualTextures.Clear();
    for( i = 0; i < m_pGeomRscDesc->GetVirtualTextureCount(); i++ )
    {
        u32 MatMask = 0;

        // grab a handy reference to the resource virtual texture
        const geom_rsc_desc::virtual_texture& RscVTex = m_pGeomRscDesc->GetVirtualTexture( i );
        
        // grab the file name, sans dir, drive, and ext
        char RscFileName[X_MAX_FNAME];
        x_splitpath( RscVTex.Textures[0].FileName, NULL, NULL, RscFileName, NULL );

        // loop through all the materials, and see if we have a match for this virtual texture    
        for( j = 0; j < Geom.m_nMaterials; j++ )
        {
            material& MeshMat = Mesh.Material[j];
            if( (MeshMat.Maps[Max_Diffuse1].MapList.GetCount() > 1) &&
                (RscVTex.Textures.GetCount() > 1) )
            {
                char MapFileName[X_MAX_FNAME];
                x_splitpath( MeshMat.Maps[Max_Diffuse1].MapList[0].InputBitmapName, NULL, NULL, MapFileName, NULL );
                if( !x_stricmp( RscFileName, MapFileName ) )
                {
                    // we've got a material match!
                    MatMask |= (1<<j);

                    // sanity check
                    if( RscVTex.Textures.GetCount() != MeshMat.Maps[Max_Diffuse1].MapList.GetCount() )
                    {
                        ThrowError( "Virtual texture count mismatch. Two virtual textures effecting the same diffuse?" );
                    }

                    // fill in the more descriptive texture names
                    for( k = 0; k < RscVTex.Textures.GetCount(); k++ )
                    {
                        Geom.m_pTexture[Geom.m_pMaterial[j].iTexture+k].DescOffset = m_Dictionary.Add( RscVTex.Textures[k].Name );
                    }
                }
            }
        }

        // if this virtual texture is used, then mark it to be added to
        // the geom data
        if( MatMask != 0 )
        {
            geom::virtual_texture& GeomVTex = VirtualTextures.Append();
            GeomVTex.MaterialMask = MatMask;
            GeomVTex.NameOffset   = m_Dictionary.Add( RscVTex.Name );
        }
    }

    // now we can set up a proper virtual texture array for the geom
    Geom.m_nVirtualTextures = VirtualTextures.GetCount();
    Geom.m_pVirtualTextures = new geom::virtual_texture[Geom.m_nVirtualTextures];
    for( i = 0; i < Geom.m_nVirtualTextures; i++ )
    {
        Geom.m_pVirtualTextures[i] = VirtualTextures[i];
    }
    
#ifdef X_DEBUG
    for( i = 0; i < Geom.m_nMaterials; i++ )
    {
        geom::material& GeomMat = Geom.m_pMaterial[i];
        ASSERT( (GeomMat.iTexture>=0) && (GeomMat.iTexture<Geom.m_nTextures) );
    }
#endif
}

//=============================================================================

void geom_compiler::ExportVirtualMeshes( mesh& Mesh, geom& Geom, s32 PlatformID )
{
    Geom.m_nVirtualMeshes = m_pGeomRscDesc->GetVirtualMeshCount();
    Geom.m_pVirtualMeshes = NULL;
    Geom.m_nLODs          = 0;
    Geom.m_pLODSizes      = NULL;
    Geom.m_pLODMasks      = 0;
    
    if( Geom.m_nVirtualMeshes )
    {
        // count up how many lods we will need
        s32 i, j, k;
        for( i = 0; i < Geom.m_nVirtualMeshes; i++ )
        {
            const geom_rsc_desc::virtual_mesh& VMesh = m_pGeomRscDesc->GetVirtualMesh( i );
            Geom.m_nLODs += VMesh.LODs.GetCount();
        }

        // allocate space for the new data
        Geom.m_pLODSizes      = new u16[Geom.m_nLODs];
        Geom.m_pLODMasks      = new u64[Geom.m_nLODs];
        Geom.m_pVirtualMeshes = new geom::virtual_mesh[Geom.m_nVirtualMeshes];

        // fill in the virtual mesh data
        s32 nLODsAdded = 0;
        for( i = 0; i < Geom.m_nVirtualMeshes; i++ )
        {
            const geom_rsc_desc::virtual_mesh& VMesh   = m_pGeomRscDesc->GetVirtualMesh( i );
            geom::virtual_mesh&                DstMesh = Geom.m_pVirtualMeshes[i];
            DstMesh.iLOD       = nLODsAdded;
            DstMesh.nLODs      = VMesh.LODs.GetCount();
            DstMesh.NameOffset = m_Dictionary.Add( VMesh.Name );
            nLODsAdded += VMesh.LODs.GetCount();

            // add the LODs
            for( j = 0; j < VMesh.LODs.GetCount(); j++ )
            {
                geom_rsc_desc::lod_info& SrcLOD = VMesh.LODs[j];
                Geom.m_pLODSizes[DstMesh.iLOD + j] = (u16)SrcLOD.ScreenSize;
                Geom.m_pLODMasks[DstMesh.iLOD + j] = 0;
                for( k = 0; k < SrcLOD.nMeshes; k++ )
                {
                    // note that we can't use the normal GetMeshIndex function
                    // because we're still working inside the dictionary!
                    for( s32 MeshId = 0; MeshId < Geom.m_nMeshes; MeshId++ )
                    {
                        if( !x_strcmp( m_Dictionary.GetString( Geom.m_pMesh[MeshId].NameOffset ),
                                       SrcLOD.MeshName[k] ) )
                        {
                            Geom.m_pLODMasks[DstMesh.iLOD + j] |= ((u64)1<<MeshId);
                            break;
                        }
                    }

                    if( MeshId == Geom.m_nMeshes )
                    {
                        ReportWarning( xfs( "VMesh (%s) refers to mesh (%s) which isn't present in the geometry",
                                            VMesh.Name, SrcLOD.MeshName[k] ) );
                    }
                }
            }
        }
    }
}

//=============================================================================
//=============================================================================
//=============================================================================
// BUILDING LOW POLY COLLISION GEOMETRY -A51
//=============================================================================
//=============================================================================
//=============================================================================

s32 DetermineMeshBone( rawmesh2& Mesh, s32 iMesh )
{
    s32 iBone=-1;

    s32 I;
    for( I=0; I<Mesh.m_nFacets; I++ )
    if( Mesh.m_pFacet[I].iMesh == iMesh )
    {
        iBone = Mesh.m_pVertex[ Mesh.m_pFacet[I].iVertex[0] ].Weight[0].iBone;
        break;
    }

    return iBone;
}
 
//=============================================================================

f32 ComputeBBoxArea( const bbox& BBox )
{
    //vector3 Size = BBox.GetSize();
    //f32 Area = (Size.GetX()*Size.GetY() + Size.GetX()*Size.GetZ() + Size.GetY()*Size.GetZ())*2;
    //return Area;

    return BBox.GetSize().Length();
}

//=============================================================================

void geom_compiler::CompileLowCollision( rigid_geom&    RigidGeom, 
                                         rawmesh2&      LowMesh, 
                                         rawmesh2&      HighMesh, 
                                         const char*    pFileName )
{
    s32 i,j;

    collision_data& Coll = RigidGeom.m_Collision;

    rawmesh2& Raw                      = LowMesh;
    rawmesh2& HighRaw                  = HighMesh;
    s32* pHighMeshIndex                = new s32[ HighRaw.m_nSubMeshs ];

    x_memset( pHighMeshIndex, -1, sizeof(s32)*HighRaw.m_nSubMeshs );
    if( Raw.m_nSubMeshs > HighRaw.m_nSubMeshs )
    {
        ThrowError( xfs("Collision model mesh count exceeds the high model mesh count, File: [%s]", m_FastCollision ) );
    }

    //
    // Check if the collision mesh references the high mesh correctly
    //
    for( i = 0; i < Raw.m_nSubMeshs; i++ )
    {
        s32 iMatch = -1;
        s32 nMatches=0;
        s32 nTies=0;

        char LowName[256];
        x_strcpy(LowName,Raw.m_pSubMesh[i].Name);
        s32 Len = x_strlen(LowName);

        // Be sure _c is present
        if( ((LowName[Len-1] != 'c') && (LowName[Len-1] != 'C')) ||(LowName[Len-2] != '_') )
        {
            ThrowError( xfs("Collision mesh needs '_c' [%s], File [%s]", Raw.m_pSubMesh[i].Name, m_FastCollision) );
        }

        LowName[Len-2] = 0;

        for( j = 0; j < HighRaw.m_nSubMeshs; j++ )
        {
            if( x_stricmp( HighRaw.m_pSubMesh[j].Name, LowName )==0 )
                break;
        }

        if( j == HighRaw.m_nSubMeshs )
        {
            ThrowError( xfs("Unreferenced collision mesh [%s], File [%s]", Raw.m_pSubMesh[i].Name, m_FastCollision) );
        }

        if( pHighMeshIndex[i] != -1 )
        {
            ThrowError( xfs("Multiple references from the collision mesh [%s] to the High mesh [%s], File [%s]", HighRaw.m_pSubMesh[j].Name, Raw.m_pSubMesh[i].Name, m_FastCollision) );
        }
                
        // Set the mesh index.
        pHighMeshIndex[i] = j;
    }


    // Reset the mesh indexes and bones indices inside the facets to match
    // the high geometry mesh
    {
        s32 iHighMesh = -1;
        s32 iHighMeshBone = -1;
        for( i = 0; i < Raw.m_nFacets; i++ )
        {
            if( pHighMeshIndex[ Raw.m_pFacet[i].iMesh ] != iHighMesh )
            {
                iHighMesh = pHighMeshIndex[ Raw.m_pFacet[i].iMesh ];
                iHighMeshBone = DetermineMeshBone( HighRaw, iHighMesh );
            }

            // Copy new iMesh into facet
            Raw.m_pFacet[i].iMesh = iHighMesh;

            // Copy new bone index into verts
            s32 iLowBone = Raw.m_pVertex[Raw.m_pFacet[i].iVertex[0]].Weight[0].iBone;
            Raw.m_pVertex[Raw.m_pFacet[i].iVertex[0]].Weight[0].iBone = iHighMeshBone;
            Raw.m_pVertex[Raw.m_pFacet[i].iVertex[1]].Weight[0].iBone = iHighMeshBone;
            Raw.m_pVertex[Raw.m_pFacet[i].iVertex[2]].Weight[0].iBone = iHighMeshBone;

            if ( iHighMeshBone == -1 )
            {
                ThrowError( xfs("Mesh (%s) isn't attached to a bone.", HighRaw.m_pSubMesh[iHighMesh].Name) );
            }
            //x_DebugMsg("LowBone(%2d,%s)  HighBone(%2d,%s)\n",
            //   iLowBone,Raw.m_pBone[iLowBone].Name,iHighMeshBone,HighRaw.m_pBone[iHighMeshBone].Name);
        }
    }
    
    // This will sort the facets by meshes then material.
    Raw.SortFacetsByMaterial();

    //
    //*********************************************************************
    //
    {

        //
        // Report results!!!
        //
        {
            X_FILE* fp = NULL;//x_fopen("c:/temp/quadlists.txt","at");
            if( fp )
            {
                x_fprintf(fp,"%s\n",pFileName);
                x_fflush(fp);
                x_fclose(fp);
            }
        }

        #define NUM_NORMAL_PER_QFACET 1
        struct qfacet
        {
            xbool   bIsQuad;
            xbool   bIsMerged;
            xbool   bInCluster;

            s32     iMesh;
            s32     iBone;
            s32     iBBoxGroup;
            //s32     iMaterial;

            vector3 Point[4];
            bbox    BBox;
            plane   Plane;
            vector3 Normal[NUM_NORMAL_PER_QFACET];

            s32     iPoint[4];
            s32     iNormal[NUM_NORMAL_PER_QFACET];
            xbool   bNegateNormal[NUM_NORMAL_PER_QFACET];

        };

        qfacet* pQF = (qfacet*)x_malloc(sizeof(qfacet)*Raw.m_nFacets);
        s32 nQFs = 0;

        // Build a QFacet out of every facet
        for( i = 0; i < Raw.m_nFacets; i++ )
        {
            // See if we already have a cluster for this facet.
            s32 iMesh       = Raw.m_pFacet[i].iMesh;
            s32 iBone       = Raw.m_pVertex[ Raw.m_pFacet[i].iVertex[0] ].Weight[0].iBone;
            u32 iMaterial   = Raw.m_pFacet[i].iMaterial;
        
            //
            // Check if this facet is some dump sliver
            //
            vector3 P0 = Raw.m_pVertex[ Raw.m_pFacet[i].iVertex[0] ].Position;
            vector3 P1 = Raw.m_pVertex[ Raw.m_pFacet[i].iVertex[1] ].Position;
            vector3 P2 = Raw.m_pVertex[ Raw.m_pFacet[i].iVertex[2] ].Position;
            if( ((P0-P1).LengthSquared() < x_sqr(0.5f)) ||
                ((P1-P2).LengthSquared() < x_sqr(0.5f)) ||
                ((P2-P0).LengthSquared() < x_sqr(0.5f)) )
            {
                /*
                static X_FILE* fp = NULL;
                if( !fp ) fp = x_fopen("c:/temp/slivers.txt","at");
                if( fp )
                {
                    x_fprintf(fp,"SLIVER: (%4d) (%10.5f,%10.5f,%10.5f) (%10.5f,%10.5f,%10.5f) (%10.5f,%10.5f,%10.5f)\n",
                        i,
                        P0.X,P0.Y,P0.Z,
                        P1.X,P1.Y,P1.Z,
                        P2.X,P2.Y,P2.Z
                        );
                    x_fflush(fp);
                }
                */
                continue;
            }

            // Check for collapse by colinear
            vector3 Normal = v3_Cross( P1-P0, P2-P0 );
            if( Normal.Length() < 0.01f )
                continue;

            qfacet& QF = pQF[nQFs];
            nQFs++;

            QF.bIsQuad      = FALSE;
            QF.bIsMerged    = FALSE;
            QF.bInCluster   = FALSE;
            QF.iMesh        = Raw.m_pFacet[i].iMesh;
            QF.iBone        = Raw.m_pVertex[ Raw.m_pFacet[i].iVertex[0] ].Weight[0].iBone;
            //QF.iMaterial    = Raw.m_pFacet[i].iMaterial;
            QF.iBBoxGroup   = -1;

            QF.Point[0] = P0;
            QF.Point[1] = P1;
            QF.Point[2] = P2;
            QF.Point[3] = P0;

            QF.Normal[0] = v3_Cross( P1-P0, P2-P0 );
            QF.Normal[0].SafeNormalize();

            QF.Plane.Normal = QF.Normal[0];
            QF.Plane.D      = -QF.Normal[0].Dot( P1 );

            // BBox and EdgeNormals are computed AFTER merging quads
        }

        //
        // Merge qfacets into quads
        //
        for( s32 A=0; A<nQFs; A++ )
        {
            qfacet& QA = pQF[A];
            if( QA.bIsMerged ) continue;

            for( s32 B=A+1; B<nQFs; B++ )
            {
                qfacet& QB = pQF[B];

                // Already merged into a quad
                if( QB.bIsMerged ) continue;

                // Normals don't match up
                if( QB.Normal[0].Dot(QA.Normal[0]) < 0.999999f ) continue;

                // Be sure mesh, bone, and material match
                if( QB.iMesh != QA.iMesh ) continue;
                if( QB.iBone != QA.iBone ) continue;
                //if( QB.iMaterial != QA.iMaterial ) continue;

                vector3 BPoint[3] = {QB.Point[0],QB.Point[1],QB.Point[2]};
                vector3 QPoint[4]; 
 
                // Check if verts match
                s32 S=0;
                for( S=0; S<3; S++ )
                {
                    if( (QA.Point[0] == BPoint[0]) )
                    {
                        if( QA.Point[1] == BPoint[2] )
                        {
                            QPoint[0] = QA.Point[1];
                            QPoint[1] = QA.Point[2];
                            QPoint[2] = QA.Point[0];
                            QPoint[3] = BPoint[1];
                            break;
                        }

                        if( QA.Point[2] == BPoint[1] )
                        {
                            QPoint[0] = QA.Point[0];
                            QPoint[1] = QA.Point[1];
                            QPoint[2] = QA.Point[2];
                            QPoint[3] = BPoint[2];
                            break;
                        }
                    }

                    if( (QA.Point[1] == BPoint[0]) && (QA.Point[2] == BPoint[2]) )
                    {
                        QPoint[0] = QA.Point[2];
                        QPoint[1] = QA.Point[0];
                        QPoint[2] = QA.Point[1];
                        QPoint[3] = BPoint[1];
                        break;
                    }

                    vector3 TP = BPoint[0];
                    BPoint[0] = BPoint[1];
                    BPoint[1] = BPoint[2];
                    BPoint[2] = TP;
                }

                if( S!=3 )
                {
                    //
                    // We can convert this poly into a quad!!!
                    //
                    QA.bIsQuad = TRUE;
                    QA.Point[0] = QPoint[0];
                    QA.Point[1] = QPoint[1];
                    QA.Point[2] = QPoint[2];
                    QA.Point[3] = QPoint[3];
                    QB.bIsMerged = TRUE;

                    // Stop looping through B
                    break;
                }

            }
        }

        //
        // Clear out merged qfacets and finish computing bbox, etc.
        //
        s32 nQuads=0;
        s32 nTris=0;
        s32 i=0;
        for( j=0; j<nQFs; j++ )
        {
            if( pQF[j].bIsMerged ) continue;

            qfacet& QF = pQF[i];
            QF = pQF[j];
 
            i++;

            if( QF.bIsQuad )
                nQuads++;
            else
                nTris++;

            QF.BBox.Clear();
            QF.BBox.AddVerts( QF.Point, 4 );

            /*
            for( s32 k=0; k<4; k++ )
            {
                QF.Normal[k+1] = QF.Normal[0].Cross(QF.Point[(k+1)%4]-QF.Point[k]);
                QF.Normal[k+1].Normalize();
            }
            */
        }
        nQFs = i;

        // 
        // Solve BBOX Groups
        //
        {
            s32 MIN_FACETS = 16;
            f32 MIN_VALUE = 300.0f;//(600.0f*600.0f)*3*2;
            s32 NUM_SPLITS_TO_TRY = 8;

            struct group
            {
                bbox    BBox;
                f32     Value;
                s32     nFacets;
            };

            // Setup initial groups
            s32 i;
            s32 nGroups = 1;
            s32 MaxGroups = (nQFs / 32);
            if( MaxGroups < 1 ) MaxGroups = 1;
            group* pGroup = (group*)x_malloc(sizeof(group)*MaxGroups);
            pGroup[0].BBox.Clear();
            for( i=0; i<nQFs; i++ )
            {
                pGroup[0].BBox += pQF[i].BBox;
                pQF[i].iBBoxGroup = 0;
            }
            pGroup[0].Value = ComputeBBoxArea(pGroup[0].BBox);
            pGroup[0].nFacets = nQFs;

            // Keep splitting groups until we are happy
            while( 1 )
            {
                // Have we used up all the boxes
                if( nGroups == MaxGroups )
                    break;

                // Find largest bbox
                s32 j=0;
                for( i=1; i<nGroups; i++ )
                if( ( pGroup[i].Value > pGroup[j].Value ) && ( pGroup[i].nFacets > MIN_FACETS ) )
                    j = i;

                // If largest bbox is small enough then we're finished
                group& G = pGroup[j];
                if( G.Value < MIN_VALUE )
                    break;

                // Find best dimension to split on
                s32 BestDim=0;
                f32 BestT=0;
                f32 BestValue=F32_MAX;
                for( s32 iDim=0; iDim<3; iDim++ )
                for( f32 T=0; T<1.0f; T+=(1.0f/NUM_SPLITS_TO_TRY) )
                {
                    
                    s32 nLeft=0,nRight=0;
                    bbox BLeft;
                    bbox BRight;
                    BLeft.Clear();
                    BRight.Clear();
                    f32 SplitValue = G.BBox.Min[iDim] + T*( G.BBox.Max[iDim] - G.BBox.Min[iDim] );
                    for( i=0; i<nQFs; i++ )
                    if( pQF[i].iBBoxGroup == j )
                    {
                        vector3 BBoxCenter = pQF[i].BBox.GetCenter();
                        if( BBoxCenter[iDim] < SplitValue )
                        {
                            BLeft += pQF[i].BBox;
                            nLeft++;
                        }
                        else
                        {
                            BRight += pQF[i].BBox;
                            nRight++;
                        }
                    }

                    f32 V = MAX(ComputeBBoxArea(BLeft),ComputeBBoxArea(BRight));
                    if( V < BestValue )
                    {
                        BestValue = V;
                        BestT = T;
                        BestDim = iDim;
                    }
                }

                //
                // Do final splitting of triangles based on best split found
                //
                {
                    s32 iDim = BestDim;
                    f32 T = BestT;
                    s32 nLeft=0,nRight=0;
                    bbox BLeft;
                    bbox BRight;
                    BLeft.Clear();
                    BRight.Clear();
                    f32 SplitValue = G.BBox.Min[iDim] + T*( G.BBox.Max[iDim] - G.BBox.Min[iDim] );
                    for( i=0; i<nQFs; i++ )
                    if( pQF[i].iBBoxGroup == j )
                    {
                        vector3 BBoxCenter = pQF[i].BBox.GetCenter();
                        if( BBoxCenter[iDim] < SplitValue )
                        {
                            BLeft += pQF[i].BBox;
                            nLeft++;
                            pQF[i].iBBoxGroup = j;
                        }
                        else
                        {
                            BRight += pQF[i].BBox;
                            nRight++;
                            pQF[i].iBBoxGroup = nGroups;
                        }
                    }

                    pGroup[j].BBox = BLeft;
                    pGroup[j].Value = ComputeBBoxArea(BLeft);
                    pGroup[j].nFacets = nLeft;

                    pGroup[nGroups].BBox = BRight;
                    pGroup[nGroups].Value = ComputeBBoxArea(BRight);
                    pGroup[nGroups].nFacets = nRight;

                    nGroups++;
                }
            }

            for( i=0; i<nGroups; i++ )
            {
                vector3 Size = pGroup[i].BBox.GetSize();
                x_DebugMsg("%3d  (%7.0f,%7.0f,%7.0f) %7.0f %d\n",
                    i,Size.GetX(),Size.GetY(),Size.GetZ(),pGroup[i].Value,pGroup[i].nFacets);
            }

            x_free(pGroup);
        }

        #define MAX_QFACETS_PER_CLUSTER   32
        #define MAX_POINTS_PER_CLUSTER    32
        #define MAX_NORMALS_PER_CLUSTER   32

        struct qcluster
        {
            bbox    BBox;
            qfacet  QF[MAX_QFACETS_PER_CLUSTER];
            vector3 Point[MAX_POINTS_PER_CLUSTER];
            vector3 Normal[MAX_NORMALS_PER_CLUSTER];
            s32     nQFs;
            s32     nPoints;
            s32     nNormals;
            s32     iMesh;
            s32     iBone;
            //s32     iMaterial;
            s32     iBBoxGroup;
        };
        qcluster* pQC  = NULL;
        s32       nQCs = 0;

        while( 1 )
        {
            // Find a QF to open a new cluster for
            f32 BestStartingCorner = F32_MAX;
            s32 BestStartingQF = -1;
            {
                for( s32 i=0; i<nQFs; i++ )
                if( !pQF[i].bInCluster )
                {
                    f32 Corner = pQF[i].BBox.Min.GetX() + 
                                 pQF[i].BBox.Min.GetY() + 
                                 pQF[i].BBox.Min.GetZ();

                    if( Corner < BestStartingCorner )
                    {
                        BestStartingCorner = Corner;
                        BestStartingQF = i;
                    }
                }
            }

            // If we couldn't find any unclustered facets then we're finished
            if( BestStartingQF==-1 )
                break;

            // Open a new cluster
            nQCs++;
            pQC = (qcluster*)x_realloc( pQC, sizeof(qcluster)*nQCs );
            qcluster& QC = pQC[nQCs-1];

            // Init cluster with first quad
            {
                QC.iMesh        = pQF[BestStartingQF].iMesh;
                QC.iBone        = pQF[BestStartingQF].iBone;
                //QC.iMaterial    = pQF[BestStartingQF].iMaterial;
                QC.BBox         = pQF[BestStartingQF].BBox;

                QC.QF[0]        = pQF[BestStartingQF];
                QC.nQFs         = 1;

                QC.nPoints      = 4;
                for( s32 i=0; i<4; i++ )
                {
                    QC.Point[i] = QC.QF[0].Point[i];
                    QC.QF[0].iPoint[i] = i;
                }

                QC.iBBoxGroup       = QC.QF[0].iBBoxGroup;
                QC.nNormals         = 1;
                QC.Normal[0]        = QC.QF[0].Normal[0];
                QC.QF[0].iNormal[0] = 0;
                QC.QF[0].bNegateNormal[0] = FALSE;
                pQF[BestStartingQF].bInCluster = TRUE;
                
            }

            // Loop through QFs and fill cluster
            while( QC.nQFs < MAX_QFACETS_PER_CLUSTER )
            {
                s32 BestQF           = -1;

                static const f32 kNewPointPenalty  = 10.0f;
                static const f32 kNewNormalPenalty = 5.0f;
                static const f32 kBBoxSizePenalty  = 200.0f;

                f32 BestScore = F32_MAX;
                s32 BestPointI[4]    = {-1,-1,-1,-1};
                s32 BestNormalI[NUM_NORMAL_PER_QFACET] = {-1};

                //
                // Loop through all QFs picking the one that gets the best
                // score according to the heuristic
                // (nPointsAdded*PointPenalty) +
                // (nNormalsAdded*NormalPenalty) +
                // (bbox size increase*BBoxSizePenalty)
                //
                for( s32 iFacet=0; iFacet<nQFs; iFacet++ )
                if( (pQF[iFacet].bInCluster == FALSE) &&
                    (pQF[iFacet].iMesh      == QC.iMesh) &&
                    (pQF[iFacet].iBone      == QC.iBone) &&
                    (pQF[iFacet].iBBoxGroup == QC.iBBoxGroup)// &&
                    //(pQF[iFacet].iMaterial  == QC.iMaterial)
                    )
                {
                    f32 QFScore = 0.0f;

                    // Get short reference to the facet
                    qfacet& QF = pQF[iFacet];

                    //
                    // Count the number of points we'd need to add, and
                    // make sure they'd fit
                    //
                    xbool bPointsFit = TRUE;
                    s32 nPointsAdded = 0;
                    s32 NewNPoints   = QC.nPoints;
                    s32 PointI[4] = {-1,-1,-1,-1};
                    for( s32 iQFPoint = 0; iQFPoint < 4; iQFPoint++ )
                    {
                        s32 iQCPoint;
                        vector3 P = QF.Point[iQFPoint];

                        for( iQCPoint = 0; iQCPoint < NewNPoints; iQCPoint++ )
                        if( QC.Point[iQCPoint] == P )
                            break;

                        if( iQCPoint == NewNPoints )
                        {
                            // could we even add a point? If not, no sense continuing...
                            if ( NewNPoints == MAX_POINTS_PER_CLUSTER )
                            {
                                bPointsFit = FALSE;
                                break;
                            }

                            // We can add a new one
                            PointI[iQFPoint] = iQCPoint;
                            QC.Point[NewNPoints] = P;
                            NewNPoints++;
                            nPointsAdded++;
                        }
                        else
                        {
                            PointI[iQFPoint] = iQCPoint;
                        }
                    }

                    // penalize the score
                    QFScore += nPointsAdded * kNewPointPenalty;

                    // early out optimization...if we're already worse than the current
                    // best score or if the points don't fit, don't continue
                    if( (QFScore > BestScore) || (bPointsFit == FALSE) )
                        continue;

                    //
                    // calculate the delta size and penalize the facet
                    //
                    f32 OldSize = (QC.BBox).GetSize().Length();//QC.BBox.GetSize();
                    f32 NewSize = (QC.BBox+QF.BBox).GetSize().Length();//(QC.BBox + QF.BBox).GetSize();
                    f32 DeltaBBoxSize = NewSize - OldSize;
                    QFScore += kBBoxSizePenalty * DeltaBBoxSize;

                    // another early out based on score
                    if( QFScore > BestScore )
                        continue;

                    //
                    // Count the number of normals we'd need to add, and make sure they'd fit
                    //
                    xbool bNormalsFit = TRUE;
                    s32 nNormalsAdded = 0;
                    s32 NewNNormals   = QC.nNormals;
                    s32 NormalI[NUM_NORMAL_PER_QFACET] = {-1};
                    for( s32 iQFNormal = 0; iQFNormal < NUM_NORMAL_PER_QFACET; iQFNormal++ )
                    {
                        s32 iQCNormal;
                        vector3 N = QF.Normal[iQFNormal];
                        for( iQCNormal = 0; iQCNormal < NewNNormals; iQCNormal++ )
                        if( (QC.Normal[iQCNormal].Dot(N)) > 0.999999f )
                            break;

                        if ( iQCNormal == NewNNormals )
                        {
                            // could we even add a normal? If not, no sense continuing...
                            if ( NewNNormals == MAX_NORMALS_PER_CLUSTER )
                            {
                                bNormalsFit = FALSE;
                                break;
                            }

                            // We can add a new one
                            NormalI[iQFNormal] = iQCNormal;
                            QC.Normal[NewNNormals] = N;
                            NewNNormals++;
                            nNormalsAdded++;
                        }
                        else
                        {
                            NormalI[iQFNormal] = iQCNormal;
                        }
                    }

                    // penalize the score
                    QFScore += nNormalsAdded * kNewNormalPenalty;

                    // once more, should we continue on?
                    if( (QFScore > BestScore) || (bNormalsFit == FALSE) )
                        continue;

                    //
                    // so far, this is the best qfacet we've got, so mark it as such
                    //
                    BestQF    = iFacet;
                    BestScore = QFScore;
                    for( j=0; j<4; j++ )
                        BestPointI[j] = PointI[j];

                    for( j=0; j<NUM_NORMAL_PER_QFACET; j++ )
                        BestNormalI[j] = NormalI[j];
                }

                // no more qfacets will go in this cluster?
                if( BestQF == -1 )
                    break;


                //
                // add the best qfacet to the cluster
                //
                {
                    qfacet& QF = QC.QF[QC.nQFs];

                    pQF[BestQF].bInCluster = TRUE;
                    QF = pQF[BestQF];
                    QC.nQFs++;
                    QC.BBox += QF.BBox;

                    // Add points
                    s32 i;
                    for( i=0; i<4; i++ )
                    {
                        ASSERT( BestPointI[i] != -1 );
                        ASSERT( BestPointI[i] < MAX_POINTS_PER_CLUSTER );
                        QC.Point[ BestPointI[i] ] = QF.Point[i];
                        QC.nPoints = MAX( QC.nPoints, (BestPointI[i]+1) );
                        QF.iPoint[i] = BestPointI[i];
                    }

                    // Add normals
                    for( i=0; i<NUM_NORMAL_PER_QFACET; i++ )
                    {
                        ASSERT( BestNormalI[i] != -1 );
                        ASSERT( BestNormalI[i] < MAX_NORMALS_PER_CLUSTER );
                        QC.Normal[ BestNormalI[i] ] = QF.Normal[i];
                        QC.nNormals = MAX( QC.nNormals, (BestNormalI[i]+1) );
                        QF.iNormal[i] = BestNormalI[i];
                    }

                    // Verify normals and points match up
                    #ifdef X_ASSERT
                    {
                        vector3 N1 = QF.Normal[0];
                        vector3 N2 = QC.Normal[ QF.iNormal[0] ];
                        vector3 N3;
                        plane Plane;
                        Plane.Setup( QF.Point[0], QF.Point[1], QF.Point[2] );
                        N3 = Plane.Normal;

                        ASSERT( N1.Dot(N2) >= 0.999f );
                        ASSERT( N2.Dot(N3) >= 0.999f );
                        ASSERT( N3.Dot(N1) >= 0.999f );


                        ASSERT( QC.Point[ QF.iPoint[0] ] == QF.Point[0] );
                        ASSERT( QC.Point[ QF.iPoint[1] ] == QF.Point[1] );
                        ASSERT( QC.Point[ QF.iPoint[2] ] == QF.Point[2] );
                    }
                    #endif
                }
            }
        }

        //
        // Sort the clusters by iBone
        //
        {
            for( s32 A=0; A<nQCs; A++ )
            {
                s32 Best = A;
                for( s32 B=A+1; B<nQCs; B++ )
                {
                    if( pQC[B].iBone < pQC[Best].iBone )
                        Best = B;
                }

                if( Best != A )
                {
                    qcluster TQC = pQC[Best];
                    pQC[Best]     = pQC[A];
                    pQC[A]        = TQC;
                }
            }
        }

        //
        // Report results!!!
        //
        #if 0
        {
            X_FILE* fp = x_fopen("c:/temp/quadlists.txt","at");
            if( fp )
            {
                s32 i;
                vector3 MaxSize(0,0,0);
                vector3 AvgSize(0,0,0);

                s32 TotalPoints = 0;
                s32 TotalNormals = 0;
                for( i=0; i<nQCs; i++ )
                {
                    TotalPoints += pQC[i].nPoints;
                    TotalNormals += pQC[i].nNormals;
                }

                s32 TotalSize = 0;
                TotalSize += nQCs * sizeof(collision_data::low_cluster);
                TotalSize += sizeof(vector4)*TotalPoints;
                TotalSize += sizeof(vector4)*TotalNormals;
                TotalSize += nQFs * sizeof(collision_data::low_quad);

                x_fprintf(fp,"%s\n",pFileName);
                x_fprintf(fp,"<<%5d>> %8d QF:%d Q:%d T:%d P:%d N:%d\n",
                    TotalSize,
                    Raw.m_nFacets,
                    nQFs,nQuads,nTris,
                    TotalPoints,TotalNormals);

                for( i=0; i<nQCs; i++ )
                {
                    vector3 Size = pQC[i].BBox.GetSize();
                    AvgSize += Size;
                    x_fprintf(fp,"[%4d]  Q(%3d) P(%3d) N(%3d) MS(%3d) BN(%3d) (%7.0f,%7.0f,%7.0f)\n",
                        i, 
                        pQC[i].nQFs, 
                        pQC[i].nPoints,
                        pQC[i].nNormals,
                        pQC[i].iMesh, 
                        pQC[i].iBone, 
                        //pQC[i].iMaterial,
                        Size.GetX(), Size.GetY(), Size.GetZ() );

                    if( Size.LengthSquared() > MaxSize.LengthSquared() )
                        MaxSize = Size;
                }
                x_fprintf(fp,"SumSize (%7.0f,%7.0f,%7.0f) %7.0f\n",AvgSize.GetX(),AvgSize.GetY(),AvgSize.GetZ(),AvgSize.Length());
                AvgSize *= 1.0f / nQCs;
                x_fprintf(fp,"AvgSize (%7.0f,%7.0f,%7.0f) %7.0f\n",AvgSize.GetX(),AvgSize.GetY(),AvgSize.GetZ(),AvgSize.Length());
                x_fprintf(fp,"MaxSize (%7.0f,%7.0f,%7.0f) %7.0f\n",MaxSize.GetX(),MaxSize.GetY(),MaxSize.GetZ(),MaxSize.Length());

                x_fflush(fp);
                x_fclose(fp);
            }
        }
        #endif

        //
        // Build final structures
        //
        {
            s32 i;

            s32 TotalPoints     = 0;
            s32 TotalNormals    = 0;
            s32 TotalQuads      = 0;
            s32 TotalVectors    = 0;
            s32 TotalClusters   = nQCs;

            for( i=0; i<TotalClusters; i++ )
            {
                TotalPoints     += pQC[i].nPoints;
                TotalNormals    += pQC[i].nNormals;
                TotalQuads      += pQC[i].nQFs;
                TotalVectors    += pQC[i].nPoints;
                TotalVectors    += pQC[i].nNormals;
            }

            vector3* pVector                      = (vector3*)x_malloc(sizeof(vector3)*TotalVectors);
            collision_data::low_quad* pQuad       = (collision_data::low_quad*)x_malloc(sizeof(collision_data::low_quad)*TotalQuads);
            collision_data::low_cluster* pCluster = (collision_data::low_cluster*)x_malloc(sizeof(collision_data::low_cluster)*TotalClusters);

            if( (!pQuad) || (!pVector) )
                x_throw("Out of memory");

            //
            // Do we need to exlude
            //
            if( g_DoCollision )
            {
                s32 QuadOffset = 0;
                s32 VectorOffset = 0;
                for( i=0; i<TotalClusters; i++ )
                {
                    qcluster& QC = pQC[i];
                    collision_data::low_cluster& CL = pCluster[i];

                    CL.iBone            = QC.iBone;
                    CL.iMesh            = QC.iMesh;
                    //CL.iMaterial        = QC.iMaterial;
                    CL.BBox             = QC.BBox;
                    CL.iQuadOffset      = QuadOffset;
                    CL.iVectorOffset    = VectorOffset;
                    CL.nNormals         = QC.nNormals;
                    CL.nPoints          = QC.nPoints;
                    CL.nQuads           = QC.nQFs;

                    QuadOffset   += CL.nQuads;
                    VectorOffset += (CL.nPoints + CL.nNormals);

                    // Fill out points
                    for( j=0; j<CL.nPoints; j++ )
                    {
                        vector3& V = pVector[ CL.iVectorOffset+j ];
                        V        = QC.Point[j];
                    }

                    // Fill out normals
                    for( j=0; j<CL.nNormals; j++ )
                    {
                        vector3& V = pVector[ CL.iVectorOffset+CL.nPoints+j ];
                        V        = QC.Normal[j];
                    }

                    // Fill out quads
                    for( j=0; j<CL.nQuads; j++ )
                    {
                        collision_data::low_quad& QD = pQuad[ CL.iQuadOffset+j ];
                        QD.iP[0] = QC.QF[j].iPoint[0];
                        QD.iP[1] = QC.QF[j].iPoint[1];
                        QD.iP[2] = QC.QF[j].iPoint[2];
                        QD.iP[3] = QC.QF[j].iPoint[3];
                        QD.iN    = QC.QF[j].iNormal[0];
                        QD.Flags = (QC.QF[j].Point[3]==QC.QF[j].Point[0]) ? (0):(1);
                    }
                }

                Coll.nLowClusters   = TotalClusters;
                Coll.nLowQuads      = TotalQuads;
                Coll.nLowVectors    = TotalVectors;
                Coll.pLowCluster    = pCluster;
                Coll.pLowVector     = pVector;
                Coll.pLowQuad       = pQuad;
            }
            else
            {
                // Well the user does want us to have the low collision geometry
                Coll.nLowClusters   = 0;
                Coll.nLowQuads      = 0;
                Coll.nLowVectors    = 0;
                Coll.pLowCluster    = NULL;
                Coll.pLowVector     = NULL;
                Coll.pLowQuad       = NULL;
            }
        }

        // Cleanup
        x_free(pQF);
        x_free(pQC);

    }
    //
    //*********************************************************************
    //

}

//=============================================================================

void geom_compiler::CompileLowCollisionFromBBox( rigid_geom& RigidGeom, 
                                                rawmesh2&   HighMesh )
{
    rawmesh2 BBoxMesh;
    BBoxMesh.m_nBones = 1;
    BBoxMesh.m_pBone = new rawmesh2::bone;
    BBoxMesh.m_nVFrames = 0;
    BBoxMesh.m_nTextures = 0;
    BBoxMesh.m_pTexture = NULL;
    BBoxMesh.m_nMaterials = 1;
    BBoxMesh.m_pMaterial = new rawmesh2::material;
    BBoxMesh.m_nParamKeys = 0;
    BBoxMesh.m_pParamKey = NULL;


    BBoxMesh.m_nVertices = 8;
    BBoxMesh.m_pVertex   = new rawmesh2::vertex[8];
    BBoxMesh.m_nFacets   = 0;
    BBoxMesh.m_pFacet    = new rawmesh2::facet[12];
    BBoxMesh.m_nSubMeshs = 1;
    BBoxMesh.m_pSubMesh  = new rawmesh2::sub_mesh;

    BBoxMesh.m_pBone[0] = HighMesh.m_pBone[0];
    BBoxMesh.m_pMaterial[0] = HighMesh.m_pMaterial[0];
    BBoxMesh.m_pSubMesh[0] = HighMesh.m_pSubMesh[0];
    x_strcpy(BBoxMesh.m_pSubMesh[0].Name,xfs("%s_c",HighMesh.m_pSubMesh[0].Name));

    bbox HighBBox = HighMesh.GetBBox();

    // If Size is too thin along a certain dimension then inflate the bbox.
    {
        f32 MinSize = 1.0f;
        for( s32 i=0; i<3; i++ )
        {
            if( (HighBBox.Max[i]-HighBBox.Min[i]) < MinSize )
            {
                f32 Center = (HighBBox.Max[i]+HighBBox.Min[i])*0.5f;
                HighBBox.Min[i] = Center - MinSize*0.5f;
                HighBBox.Max[i] = Center + MinSize*0.5f;
            }
        }
    }

    // Indices used to convert min + max of bbox into 8 corners
    static byte CornerIndices[8*3]   = { 0,1,2,     // 0: MinX, MinY, MinZ
                                         4,1,2,     // 1: MaxX, MinY, MinZ
                                         0,5,2,     // 2: MinX, MaxY, MinZ
                                         4,5,2,     // 3: MaxX, MaxY, MinZ
                                         0,1,6,     // 4: MinX, MinY, MaxZ
                                         4,1,6,     // 5: MaxX, MinY, MaxZ
                                         0,5,6,     // 6: MinX, MaxY, MaxZ
                                         4,5,6 };   // 7: MaxX, MaxY, MaxZ;

    // Indices used to convert 8 corners into a 4 sided NGon
    static byte SideIndices[6*4]     = { 4,6,2,0,   // X -ve
                                         1,3,7,5,   // X +ve
                                         4,0,1,5,   // Y -ve
                                         2,6,7,3,   // Y +ve
                                         0,2,3,1,   // Z -ve
                                         5,7,6,4 }; // Z +ve

    // Build the corners
    vector3     Corner[8];
    {
        const byte*  pI = CornerIndices;
        const f32*   pBBoxF = &HighBBox.Min.GetX();
        for( s32 i=0; i<8; i++ )
        {
            BBoxMesh.m_pVertex[i].Position.Set( pBBoxF[pI[0]],
                                                pBBoxF[pI[1]],
                                                pBBoxF[pI[2]] );
            BBoxMesh.m_pVertex[i].nColors          = 0;
            BBoxMesh.m_pVertex[i].nNormals         = 0;
            BBoxMesh.m_pVertex[i].nUVs             = 0;
            BBoxMesh.m_pVertex[i].nWeights         = 1;
            BBoxMesh.m_pVertex[i].Weight[0].iBone  = 0;
            BBoxMesh.m_pVertex[i].Weight[0].Weight = 1.0f;
            pI += 3;
        }
    }

    //
    // We need to build 2 triangles for each side
    //
    {
        const byte*  pI = SideIndices;
        s32 iFacet = 0;
        for( s32 i=0; i<6; i++, pI += 4 )
        {
            // Build normal of bbox side
            plane BBoxPlane( BBoxMesh.m_pVertex[ pI[0] ].Position,
                             BBoxMesh.m_pVertex[ pI[1] ].Position,
                             BBoxMesh.m_pVertex[ pI[2] ].Position );             

            // Check high mesh faces to see if any faces point this direction
            s32 f;
            for( f = 0 ; f < HighMesh.m_nFacets; f++ )
            {
                // Lookup facet
                rawmesh2::facet& Facet = HighMesh.m_pFacet[f];
                    
                // Build facet normal
                plane FacetPlane( HighMesh.m_pVertex[ Facet.iVertex[0] ].Position,
                                  HighMesh.m_pVertex[ Facet.iVertex[1] ].Position,
                                  HighMesh.m_pVertex[ Facet.iVertex[2] ].Position );                
                
                // If bbox side and face normal are pointing the same direction, then keep this side
                if( v3_Dot( BBoxPlane.Normal, FacetPlane.Normal ) > 0.0f )
                    break;
                    
                // Can material be seen from both sides?
                rawmesh2::material& Mat = HighMesh.m_pMaterial[ Facet.iMaterial ];
                if(     ( Mat.bTwoSided )   // Two sided?
                    ||  ( Mat.Map[Max_PunchThrough].iTexture != -1 )   // Punch thru alpha?
                    ||  ( Mat.Type == Material_Alpha )                  // Alpha blended?
                    ||  ( Mat.Type == Material_Alpha_PerPolyEnv )
                    ||  ( Mat.Type == Material_Alpha_PerPixelIllum )
                    ||  ( Mat.Type == Material_Alpha_PerPolyIllum ) )
                {                    
                    // Keep if bbox side and face normal are pointing the opposite direction
                    if( v3_Dot( BBoxPlane.Normal, FacetPlane.Normal ) < 0.0f )
                        break;
                }                    
            }

            // Skip if no faces point same direction as the bbox side
            // (will happen for single sided floor/ceiling pieces as required by the physics)
            if( f == HighMesh.m_nFacets )
                continue;

            // Add facets                
            BBoxMesh.m_pFacet[iFacet].iMaterial = 0;
            BBoxMesh.m_pFacet[iFacet].iMesh     = 0;
            BBoxMesh.m_pFacet[iFacet].iVertex[0] = pI[0];
            BBoxMesh.m_pFacet[iFacet].iVertex[1] = pI[1];
            BBoxMesh.m_pFacet[iFacet].iVertex[2] = pI[2];
            BBoxMesh.m_pFacet[iFacet].Plane.Setup(1,0,0,0);
            iFacet++;

            BBoxMesh.m_pFacet[iFacet].iMaterial = 0;
            BBoxMesh.m_pFacet[iFacet].iMesh     = 0;
            BBoxMesh.m_pFacet[iFacet].iVertex[0] = pI[0];
            BBoxMesh.m_pFacet[iFacet].iVertex[1] = pI[2];
            BBoxMesh.m_pFacet[iFacet].iVertex[2] = pI[3];
            BBoxMesh.m_pFacet[iFacet].Plane.Setup(1,0,0,0);
            iFacet++;
        }
        BBoxMesh.m_nFacets = iFacet;

    }

    CompileLowCollision( RigidGeom, BBoxMesh, HighMesh, "CONSTRUCTING BBOX" );
}

//=============================================================================
//=============================================================================
//=============================================================================
// BUILDING HIGH POLY COLLISION GEOMETRY
//=============================================================================
//=============================================================================
//=============================================================================

inline
s32 CompareHighTriangles( const void* pT1, const void* pT2 )
{
    ASSERT( pT1 != NULL );
    ASSERT( pT2 != NULL );

    geom_compiler::high_tri& T1 = *((geom_compiler::high_tri*)pT1);
    geom_compiler::high_tri& T2 = *((geom_compiler::high_tri*)pT2);

    // Compare basic properties
    if( T1.iDList< T2.iDList ) return -1;
    if( T1.iDList> T2.iDList ) return +1;
    if( T1.iBone < T2.iBone ) return -1;
    if( T1.iBone > T2.iBone ) return +1;
    if( T1.iMesh < T2.iMesh ) return -1;
    if( T1.iMesh > T2.iMesh ) return +1;
    if( T1.MatInfo.SoundType < T2.MatInfo.SoundType ) return -1;
    if( T1.MatInfo.SoundType > T2.MatInfo.SoundType ) return +1;
    if( T1.MatInfo.Flags     < T2.MatInfo.Flags ) return -1;
    if( T1.MatInfo.Flags     > T2.MatInfo.Flags ) return +1;

    // Compare normals of plane
    vector3 N1 = T1.Plane.Normal;
    vector3 N2 = T2.Plane.Normal;
    if( N1.GetX() + 0.001f < N2.GetX() ) return -1;
    if( N1.GetX() - 0.001f > N2.GetX() ) return +1;
    if( N1.GetY() + 0.001f < N2.GetY() ) return -1;
    if( N1.GetY() - 0.001f > N2.GetY() ) return +1;
    if( N1.GetZ() + 0.001f < N2.GetZ() ) return -1;
    if( N1.GetZ() - 0.001f > N2.GetZ() ) return +1;
    
    // Compare actual plane D values
    if( T1.Plane.D + 0.001f < T2.Plane.D ) return -1;
    if( T1.Plane.D - 0.001f > T2.Plane.D ) return +1;

    // Consider these triangles identical
    return 0;
}


//=============================================================================

void geom_compiler::CompileHighCollision( rigid_geom&       RigidGeom, 
                                          high_tri*         pHighTri,
                                          s32               nHighTris,
                                          const char*       pFileName,
                                          platform          PlatformID )
{
    // early out if we're not doing collision
    // Note that the PC must always have collision, because that's how the
    // editor does selection. What a pain...
    if( !g_DoCollision && (PLATFORM_PC != PlatformID) )
    {
        RigidGeom.m_Collision.BBox.Clear();
        RigidGeom.m_Collision.nHighClusters     = 0;
        RigidGeom.m_Collision.nHighIndices      = 0;
        RigidGeom.m_Collision.pHighCluster      = NULL;
        RigidGeom.m_Collision.pHighIndexToVert0 = NULL;

        s32 iTri;
        for( iTri = 0; iTri < nHighTris; iTri++ )
        {
            RigidGeom.m_Collision.BBox.AddVerts( pHighTri[iTri].P, 3 );
        }

        return;
    }

    s32 i;
    xtimer Timer[5];
    Timer[0].Start();

    static X_FILE* fp = NULL;//x_fopen("c:/temp/highcoll.txt","at");
    if( fp )
    {
        x_fprintf(fp,"FILE: %s\n",pFileName);
        x_fprintf(fp,"nTris %d\n",nHighTris);
    }

    (void)pFileName;

    struct cluster
    {
        bbox        BBox;
        high_tri*   pFirstTri;
        s32         nTris;
        xbool       bFinished;
    };

    // Fill out plane information
    for( i=0; i<nHighTris; i++ )
    {
        pHighTri[i].Plane.Setup( pHighTri[i].P[0], pHighTri[i].P[1], pHighTri[i].P[2] );
        pHighTri[i].BBox.Clear();
        pHighTri[i].BBox.AddVerts( pHighTri[i].P, 3 );
        pHighTri[i].BBox.Inflate(1,1,1);
    }

    // Sort the triangles by similar properties
    qsort( pHighTri, nHighTris, sizeof(high_tri), CompareHighTriangles );

    // Build initial set of clusters
    cluster* pCluster  = (cluster*)x_malloc(sizeof(cluster)*1024);
    s32      nClusters = 0;
    for( i=0; i<1024; i++ )
    {
        pCluster[i].BBox.Clear();
        pCluster[i].nTris = 0;
        pCluster[i].pFirstTri = NULL;
        pCluster[i].bFinished = FALSE;
    }

    // Setup initial values for cluster boundaries
    s32                         ClusterBone = -1;
    s32                         ClusterMesh = -1;
    s32                         ClusterDList = -1;
    collision_data::mat_info    ClusterMatInfo;

    ClusterMatInfo.SoundType = 0xFFFF;
    ClusterMatInfo.Flags     = 0xFFFF;

    // Build initial set of clusters
    for( i=0; i<nHighTris; i++ )
    {
        high_tri& T = pHighTri[i];

        // Do we need to start a new cluster?
        if( (T.iDList   != ClusterDList) ||
            (T.iBone    != ClusterBone) ||
            (T.iMesh    != ClusterMesh) ||
            (T.MatInfo.SoundType  != ClusterMatInfo.SoundType ) ||
            (T.MatInfo.Flags      != ClusterMatInfo.Flags     ))
        {
            ASSERT(nClusters < 1024);
            ClusterDList    = T.iDList;
            ClusterBone     = T.iBone;
            ClusterMesh     = T.iMesh;
            ClusterMatInfo  = T.MatInfo;
            nClusters++;
        }

        // Add triangle to cluster
        T.pNext = pCluster[nClusters-1].pFirstTri;
        pCluster[nClusters-1].pFirstTri = &T;
        pCluster[nClusters-1].nTris++;
        pCluster[nClusters-1].BBox.AddVerts( T.P, 3 );
    }

    for( i=0; i<nClusters; i++ )
    {
        ASSERT( pCluster[i].pFirstTri );
        ASSERT( pCluster[i].nTris );
    }

    if( fp )
    {
        for( i=0; i<nClusters; i++ )
        {
            x_fprintf(fp,"%3d] ++ %4d %10.0f\n",i,pCluster[i].nTris,pCluster[i].BBox.GetSurfaceArea());
        }
    }

    //
    // Start loop to break clusters into smaller clusters
    //
    #define MAX_CLUSTERS                (128)
    #define MIN_TRIS_PER_CLUSTER        (32)
    #define NUM_BBOX_DIVISIONS          (5)
    #define MIN_TRIS_PER_SIDE_IN_SPLIT  (8)
    while( 1 )
    {
        // If we've hit an upper limit on clusters then bail
        if( nClusters >= MAX_CLUSTERS )
            break;

Timer[1].Start();
        //
        // Look for the largest cluster
        //
        s32 iBestCluster    = -1;
        f32 BestClusterSize = 0;
        for( i=0; i<nClusters; i++ )
        if( pCluster[i].bFinished == FALSE )
        {
            // Should this cluster be broken or should it be considered finished
            if( pCluster[i].nTris <= MIN_TRIS_PER_CLUSTER )
            {
                pCluster[i].bFinished = TRUE;
                continue;
            }

            // Decide if this cluster is a better break candidate
            f32 SurfaceArea = pCluster[i].BBox.GetSurfaceArea();
            if( SurfaceArea > BestClusterSize )
            {
                iBestCluster = i;
                BestClusterSize = SurfaceArea;
            }
        }
Timer[1].Stop();

        // If we didn't find one then we are finished.
        if( iBestCluster == -1 )
            break;


        // We can break this cluster
        {
Timer[2].Start();
            cluster& CL = pCluster[iBestCluster];

            s32  nBBoxes=0;
            bbox BBoxL[NUM_BBOX_DIVISIONS*3];
            bbox BBoxR[NUM_BBOX_DIVISIONS*3];
            
            // Build new bbox candidates
            for( i=0; i<NUM_BBOX_DIVISIONS; i++ )
            {
                f32 T = (f32)(i+1) / (f32)(NUM_BBOX_DIVISIONS+1);
                f32 XDiv = CL.BBox.Min.GetX() + T*(CL.BBox.Max.GetX() - CL.BBox.Min.GetX());
                f32 YDiv = CL.BBox.Min.GetY() + T*(CL.BBox.Max.GetY() - CL.BBox.Min.GetY());
                f32 ZDiv = CL.BBox.Min.GetZ() + T*(CL.BBox.Max.GetZ() - CL.BBox.Min.GetZ());
                BBoxL[nBBoxes+0] = CL.BBox;     BBoxL[nBBoxes+0].Max.GetX() = XDiv;
                BBoxR[nBBoxes+0] = CL.BBox;     BBoxR[nBBoxes+0].Min.GetX() = XDiv;
                BBoxL[nBBoxes+1] = CL.BBox;     BBoxL[nBBoxes+1].Max.GetY() = YDiv;
                BBoxR[nBBoxes+1] = CL.BBox;     BBoxR[nBBoxes+1].Min.GetY() = YDiv;
                BBoxL[nBBoxes+2] = CL.BBox;     BBoxL[nBBoxes+2].Max.GetZ() = ZDiv;
                BBoxR[nBBoxes+2] = CL.BBox;     BBoxR[nBBoxes+2].Min.GetZ() = ZDiv;
                nBBoxes += 3;
            }

            // Loop through candidates and choose best splitter
            f32 BestSplitterScore = F32_MAX;
            s32 iBestSplitter = -1;
            for( s32 iBB=0; iBB<NUM_BBOX_DIVISIONS*3; iBB++ )
            {
                const bbox& BBL         = BBoxL[iBB];
                const bbox& BBR         = BBoxR[iBB];
                s32         nLeft       = 0;
                s32         nRight      = 0;
                bbox        TightBBoxL;
                bbox        TightBBoxR;

                TightBBoxL.Clear();
                TightBBoxR.Clear();

                // Split triangles into the two camps
                high_tri* pTri = CL.pFirstTri;
                while( pTri )
                {
                    // Should the tri go on the right or left?
                    if( BBL.Intersect(pTri->BBox) )
                    {
                        // LEFT!
                        TightBBoxL += pTri->BBox;
                        nLeft++;
                    }
                    else
                    {
                        // RIGHT!
                        ASSERT( BBR.Intersect(pTri->BBox) );
                        TightBBoxR += pTri->BBox;
                        nRight++;
                    }

                    pTri = pTri->pNext;
                }
                ASSERT( (nLeft+nRight) == CL.nTris );

                // Check if this beats the best score so far
                if( ( nLeft  >= MIN_TRIS_PER_SIDE_IN_SPLIT ) && 
                    ( nRight >= MIN_TRIS_PER_SIDE_IN_SPLIT ) )
                {
                    f32 SurfaceAreaL = TightBBoxL.GetSurfaceArea();
                    f32 SurfaceAreaR = TightBBoxL.GetSurfaceArea();
                    f32 Score = SurfaceAreaL + SurfaceAreaR;

                    if( Score < BestSplitterScore )
                    {
                        BestSplitterScore = Score;
                        iBestSplitter = iBB;
                    }
                }
            }
Timer[2].Stop();

            // If no good splitter was found then mark this cluster as finished
            // we've done the best we can.
            if( iBestSplitter == -1 )
            {
                CL.bFinished = TRUE;
                continue;
            }
Timer[3].Start();

            //
            // Create a new cluster and split the triangles
            //
            {
                // Build new triangle lists
                const bbox& BBL         = BBoxL[iBestSplitter];
                const bbox& BBR         = BBoxR[iBestSplitter];
                high_tri*   pLeft       = NULL;
                high_tri*   pRight      = NULL;
                s32         nLeft       = 0;
                s32         nRight      = 0;
                bbox        TightBBoxL;
                bbox        TightBBoxR;

                TightBBoxL.Clear();
                TightBBoxR.Clear();

                // Split triangles into the two camps
                high_tri* pTri = CL.pFirstTri;
                while( pTri )
                {
                    high_tri* pNext = pTri->pNext;

                    // Should the tri go on the right or left?
                    if( BBL.Intersect(pTri->BBox) )
                    {
                        // LEFT!
                        TightBBoxL += pTri->BBox;
                        pTri->pNext = pLeft;
                        pLeft = pTri;
                        nLeft++;
                    }
                    else
                    {
                        // RIGHT!
                        ASSERT( BBR.Intersect(pTri->BBox) );
                        TightBBoxR += pTri->BBox;
                        pTri->pNext = pRight;
                        pRight = pTri;
                        nRight++;
                    }

                    pTri = pNext;
                }

                // Fill out clusters
                ASSERT( nClusters < 1024 );
                cluster& NCL = pCluster[nClusters];
                nClusters++;

                ASSERT( pLeft && pRight && nLeft && nRight );

                CL.BBox         = TightBBoxL;
                CL.pFirstTri    = pLeft;
                CL.nTris        = nLeft;
                CL.bFinished    = FALSE;
                
                NCL.BBox        = TightBBoxR;
                NCL.pFirstTri   = pRight;
                NCL.nTris       = nRight;
                NCL.bFinished   = FALSE;
            }
Timer[3].Stop();

        }
    }

Timer[4].Start();

    //
    // Sort the clusters by iBone
    //
    {
        for( s32 A=0; A<nClusters; A++ )
        {
            s32 Best = A;
            for( s32 B=A+1; B<nClusters; B++ )
            {
                if( pCluster[B].pFirstTri->iBone < pCluster[Best].pFirstTri->iBone )
                    Best = B;
            }

            if( Best != A )
            {
                cluster TQC        = pCluster[Best];
                pCluster[Best]     = pCluster[A];
                pCluster[A]        = TQC;
            }
        }
    }

    if( fp )
    {
        x_fprintf(fp,"Initial nClusters %d\n",nClusters);
        for( i=0; i<nClusters; i++ )
        {
            x_fprintf(fp,"%3d] ** %4d %10.0f %3d\n",i,pCluster[i].nTris,pCluster[i].BBox.GetSurfaceArea(),pCluster[i].pFirstTri->iBone);
        }
    }

    //
    // Fill out collision geometry
    //
    {
        RigidGeom.m_Collision.nHighClusters = nClusters;
        RigidGeom.m_Collision.nHighIndices  = nHighTris;
        RigidGeom.m_Collision.pHighCluster  = new collision_data::high_cluster[ nClusters ];
        RigidGeom.m_Collision.pHighIndexToVert0 = new u16[ nHighTris ];
    
        s32 iOffset = 0;
        for( i=0; i<nClusters; i++ )
        {
            collision_data::high_cluster& HCL = RigidGeom.m_Collision.pHighCluster[i];
            cluster& CL = pCluster[i];

            HCL.BBox         = CL.BBox;
            HCL.iBone        = CL.pFirstTri->iBone;
            HCL.iMesh        = CL.pFirstTri->iMesh;
            HCL.iOffset      = iOffset;
            HCL.iDList       = CL.pFirstTri->iDList;
            HCL.nTris        = CL.nTris;
            HCL.MaterialInfo = CL.pFirstTri->MatInfo;

            // Loop through triangles and add offsets to list
            high_tri* pTri = CL.pFirstTri;
            while( pTri )
            {
                u16 Index = pTri->I;
                if( pTri->bFlipOrient ) Index |= 0x8000;

                RigidGeom.m_Collision.pHighIndexToVert0[iOffset] = Index;
                iOffset++;

                ASSERT( pTri->iBone == HCL.iBone );
                ASSERT( pTri->iMesh == HCL.iMesh );
                ASSERT( pTri->MatInfo.Flags == HCL.MaterialInfo.Flags );
                ASSERT( pTri->MatInfo.SoundType == HCL.MaterialInfo.SoundType );
                ASSERT( pTri->iDList == HCL.iDList );

                pTri = pTri->pNext;
            }
        }
        ASSERT( iOffset == nHighTris );
    }

    //
    // Fill out main bbox
    //
    {
        RigidGeom.m_Collision.BBox.Clear();
        for( i=0; i<nClusters; i++ )
        {
            RigidGeom.m_Collision.BBox += pCluster[i].BBox;
        }
    }

    // Free allocated memory
    x_free(pCluster);
Timer[4].Stop();

    Timer[0].Stop();
    if( fp )
    {
        x_fprintf(fp,"TIME: %7.3f %7.3f %7.3f %7.3f %7.3f\n",
            Timer[0].ReadSec(),
            Timer[1].ReadSec(),
            Timer[2].ReadSec(),
            Timer[3].ReadSec(),
            Timer[4].ReadSec());
        x_fprintf(fp,"-----------------------------------------------------\n");
    }

}

//=============================================================================

void geom_compiler::CompileHighCollisionPS2( rigid_geom&                RigidGeom, 
                                             collision_data::mat_info*  pMatList,
                                             const char*                pFileName )
{
    (void)pFileName;

    s32         nTris = RigidGeom.GetNFaces();
    high_tri*   pTri  = new high_tri[ nTris ];

    //
    // Build the triangles.
    //

    s32 c = 0;
    s32 I = 0;

    // Loop thru meshes.
    for( s32 i = 0; i < RigidGeom.m_nMeshes; i++ )
    {
        geom::mesh& Mesh = RigidGeom.m_pMesh[i];

        // Loop thru all the display lists.
        for( s32 j = 0; j < Mesh.nSubMeshes; j++ )
        {
            s32                      iSubMesh = Mesh.iSubMesh + j;
            geom::submesh&           SubMesh  = RigidGeom.m_pSubMesh[ iSubMesh ];
            rigid_geom::dlist_ps2&   DList    = RigidGeom.m_System.pPS2[ SubMesh.iDList ];

            // Loop thru all the tris.
            for( s32 k = 0; k < DList.nVerts; k++ )
            {
                s32 A = DList.pPosition[k].GetIW();
                if( !(A & (1 << 15)) )
                {
                    pTri[I].I    = k-2;
                    pTri[I].P[0] = *((vector3*)(DList.pPosition + pTri[I].I + 0));
                    pTri[I].P[1] = *((vector3*)(DList.pPosition + pTri[I].I + 1));
                    pTri[I].P[2] = *((vector3*)(DList.pPosition + pTri[I].I + 2));
                    pTri[I].iBone = DList.iBone;
                    pTri[I].iMesh = i;
                    pTri[I].iDList = SubMesh.iDList;
                    pTri[I].MatInfo = pMatList[c];
                    pTri[I].bFlipOrient = (A & (1<<CCWBIT)) ? (FALSE) : (TRUE);
                    I++;
                    if( k < 2 )  x_throw( "Bad ADC." );                        
                }
            }

            c++;
        }
    }

    //
    // Build the collision geometry
    //
    CompileHighCollision( RigidGeom, pTri, nTris, pFileName, PLATFORM_PS2 );

    delete pTri;
}

//=============================================================================

void geom_compiler::CompileHighCollisionXBOX(   rigid_geom&                 RigidGeom, 
                                                collision_data::mat_info*   pMatList,
                                                const char*                 pFileName )
{
    (void)pFileName;

    s32         nTris = RigidGeom.GetNFaces();
    high_tri*   pTri  = new high_tri[ nTris ];

    //
    // Build the triangles.
    //

    s32 c = 0;
    s32 I = 0;

    // Loop thru meshes.
    for( s32 i = 0; i < RigidGeom.m_nMeshes; i++ )
    {
        geom::mesh& Mesh = RigidGeom.m_pMesh[i];

        // Loop thru all the display lists.
        for( s32 j = 0; j < Mesh.nSubMeshes; j++ )
        {
            s32                      iSubMesh = Mesh.iSubMesh + j;
            geom::submesh&            SubMesh = RigidGeom.m_pSubMesh[ iSubMesh ];
            rigid_geom::dlist_xbox&     DList = RigidGeom.m_System.pXbox[ SubMesh.iDList ];

            // Loop thru all the tris.
            for( s32 k = 0; k < DList.nIndices/3; k++ )
            {
                pTri[I].I    = (k * 3);
                pTri[I].P[0] = DList.pVert[DList.pIndices[(k*3)+0]].Pos;
                pTri[I].P[1] = DList.pVert[DList.pIndices[(k*3)+1]].Pos;
                pTri[I].P[2] = DList.pVert[DList.pIndices[(k*3)+2]].Pos;
                pTri[I].iBone = DList.iBone;
                pTri[I].iMesh = i;
                pTri[I].iDList = SubMesh.iDList;
                pTri[I].MatInfo = pMatList[c];
                pTri[I].bFlipOrient = FALSE;
                I++;
            }

            c++;
        }
    }

    //
    // Build the collision geometry
    //
    CompileHighCollision( RigidGeom, pTri, nTris, pFileName, PLATFORM_XBOX );

    delete pTri;
}

//=============================================================================

void geom_compiler::CompileHighCollisionPC( rigid_geom&                 RigidGeom, 
                                            collision_data::mat_info*   pMatList,
                                            const char*                 pFileName )
{
    (void)pFileName;

    s32         nTris = RigidGeom.GetNFaces();
    high_tri*   pTri  = new high_tri[ nTris ];

    //
    // Build the triangles.
    //

    s32 c = 0;
    s32 I = 0;

    // Loop thru meshes.
    for( s32 i = 0; i < RigidGeom.m_nMeshes; i++ )
    {
        geom::mesh& Mesh = RigidGeom.m_pMesh[i];

        // Loop thru all the display lists.
        for( s32 j = 0; j < Mesh.nSubMeshes; j++ )
        {
            s32                      iSubMesh = Mesh.iSubMesh + j;
            geom::submesh&           SubMesh  = RigidGeom.m_pSubMesh[ iSubMesh ];
            rigid_geom::dlist_pc&    DList    = RigidGeom.m_System.pPC[ SubMesh.iDList ];

            // Loop thru all the tris.
            for( s32 k = 0; k < DList.nIndices/3; k++ )
            {
                pTri[I].I    = (k * 3);
                pTri[I].P[0] = DList.pVert[DList.pIndices[(k*3)+0]].Pos;
                pTri[I].P[1] = DList.pVert[DList.pIndices[(k*3)+1]].Pos;
                pTri[I].P[2] = DList.pVert[DList.pIndices[(k*3)+2]].Pos;
                pTri[I].iBone = DList.iBone;
                pTri[I].iMesh = i;
                pTri[I].iDList = SubMesh.iDList;
                pTri[I].MatInfo = pMatList[c];
                pTri[I].bFlipOrient = FALSE;
                I++;
            }

            c++;
        }
    }

    //
    // Build the collision geometry
    //
    CompileHighCollision( RigidGeom, pTri, nTris, pFileName, PLATFORM_PC );

    delete pTri;
}

//=============================================================================

xbool RigidGeom_GetTriangle( const rigid_geom*          pRigidGeom,
                             s32                   Key,
                             vector3&              P0,
                             vector3&              P1,
                             vector3&              P2)
{
    // This is here to get the RigidGeom to shutup about needing this function!
    return FALSE;
}

//=============================================================================

void geom_compiler::CompileDictionary( geom& Geom )
{
    s32 i;

    // create a map of where the strings will end up
    s16* StringRemap = new s16[m_Dictionary.GetCount()];
    s16  Offset = 0;
    for( i = 0; i < m_Dictionary.GetCount(); i++ )
    {
        StringRemap[i] = Offset;
        Offset += 1 + x_strlen(m_Dictionary.GetString(i));
    }

    // copy the strings out
    Geom.m_StringDataSize = Offset;
    Geom.m_pStringData = new char[Geom.m_StringDataSize];
    for( i = 0; i < m_Dictionary.GetCount(); i++ )
    {
        x_strcpy( &Geom.m_pStringData[StringRemap[i]],
                  m_Dictionary.GetString(i) );
    }

    // remap the texture names
    for( i = 0; i < Geom.m_nTextures; i++ )
    {
        Geom.m_pTexture[i].DescOffset     = StringRemap[Geom.m_pTexture[i].DescOffset];
        Geom.m_pTexture[i].FileNameOffset = StringRemap[Geom.m_pTexture[i].FileNameOffset];
    }

    // remap the geom names
    for( i = 0; i < Geom.m_nMeshes; i++ )
    {
        Geom.m_pMesh[i].NameOffset = StringRemap[Geom.m_pMesh[i].NameOffset];
    }

    // remap the virtual mesh names
    for( i = 0; i < Geom.m_nVirtualMeshes; i++ )
    {
        Geom.m_pVirtualMeshes[i].NameOffset = StringRemap[Geom.m_pVirtualMeshes[i].NameOffset];
    }

    // remap the virtual texture names
    for( i = 0; i < Geom.m_nVirtualTextures; i++ )
    {
        Geom.m_pVirtualTextures[i].NameOffset = StringRemap[Geom.m_pVirtualTextures[i].NameOffset];
    }

    // remap the bone masks names
    for( i = 0; i < Geom.m_nBoneMasks; i++ )
    {
        Geom.m_pBoneMasks[i].NameOffset = StringRemap[Geom.m_pBoneMasks[i].NameOffset];
    }

    // remap the rigid body names
    for( i = 0; i < Geom.m_nRigidBodies; i++ )
    {
        Geom.m_pRigidBodies[i].NameOffset = StringRemap[Geom.m_pRigidBodies[i].NameOffset];
    }

    // remap the property section strings
    for( i = 0; i < Geom.m_nPropertySections; i++ )
    {
        geom::property_section& Section = Geom.m_pPropertySections[i];
        Section.NameOffset = StringRemap[Section.NameOffset];
    }

    // remap the property strings
    for( i = 0; i < Geom.m_nProperties; i++ )
    {
        geom::property& Prop = Geom.m_pProperties[i];
        Prop.NameOffset    = StringRemap[Prop.NameOffset];
        if( Prop.Type == geom::property::TYPE_STRING )
            Prop.Value.StringOffset = StringRemap[Prop.Value.StringOffset];
    }

    // clean up
    delete []StringRemap;
}

//=============================================================================

void geom_compiler::PrintSummary( geom& Geom )
{
    s32 i, j, k;

    // print out an overall summary
    x_printf( "\n--Mesh Summary--------------------------------\n" );
    x_printf( "%d VMeshes\n", Geom.m_nVirtualMeshes );
    x_printf( "%d Meshes\n", Geom.m_nMeshes );
    x_printf( "%d Verts\n", Geom.m_nVertices );
    x_printf( "%d Textures\n", Geom.m_nTextures );
    x_printf( "%d Materials\n", Geom.m_nMaterials );
    
    // print out a breakdown of the vmeshes
    x_printf( "\n--VMesh Breakdown-----------------------------\n" );
    for( i = 0; i < Geom.m_nVirtualMeshes; i++ )
    {
        geom::virtual_mesh& VMesh = Geom.m_pVirtualMeshes[i];
        x_printf( "%s\n", Geom.GetVMeshName( i) );
        for( j = VMesh.iLOD; j < VMesh.iLOD + VMesh.nLODs; j++ )
        {
            u64 LODMask = Geom.m_pLODMasks[j];
            for( k = 0; k < Geom.m_nMeshes; k++ )
            {
                if( LODMask & ((u64)1<<k) )
                {
                    x_printf( "    %s\n", Geom.GetMeshName(k) );
                }
            }
        }
    }

    // print out a breakdown of the materials
    x_printf( "\n--Material Breakdown--------------------------\n" );
    for( i = 0; i < Geom.m_nMaterials; i++ )
    {
        geom::material& Mat = Geom.m_pMaterial[i];
        x_printf( "Material #%d\n", i );
        x_printf( "  nTextures: %d\n", Mat.nTextures );

        s32 iDiffuse     = Mat.iTexture;
        s32 iEnvironment = iDiffuse + Mat.nVirtualMats;
        s32 iDetail      = iEnvironment + ((Mat.Flags & geom::material::FLAG_HAS_ENV_MAP) ? 1 : 0);
        for( j = 0; j < Mat.nVirtualMats; j++ )
        {
            x_printf( "  Diffuse: %s\n", Geom.GetTextureName( Mat.iTexture + j ) );
        }
        if( Mat.Flags & geom::material::FLAG_HAS_ENV_MAP )
        {
            x_printf( "  EnvMap: %s\n", Geom.GetTextureName( iEnvironment ) );
        }
        if( Mat.Flags & geom::material::FLAG_HAS_DETAIL_MAP )
        {
            x_printf( "  DetailMap: %s\n", Geom.GetTextureName( iDetail ) );
        }
    }

    x_printf( "\n--Texture Breakdown----------------------------\n" );
    for( i = 0; i < Geom.m_nTextures; i++ )
    {
        geom::texture& Tex   = Geom.m_pTexture[i];
        const char*    pName = Geom.GetTextureName( i );
        const char*    pDesc = Geom.GetTextureDesc( i );
        if( pDesc[0] == '\0' )
            pDesc = "NO DESC";
        x_printf( "Texture %d: Name(%s) Desc(%s)\n",
            i, pName, pDesc );
    }
}

//=============================================================================
