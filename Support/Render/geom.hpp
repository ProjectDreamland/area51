
#ifndef GEOM_HPP
#define GEOM_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_files.hpp"
#include "Auxiliary\MiscUtils\Fileio.hpp"
#include "Material_Prefs.hpp"
#include "Animation\AnimData.hpp"

// blech...this conditional include is because geomcompiler breaks when
// you get multiple versions of directx included (dx8 is needed for the
// xbox compilation, dx9 is needed for the editor)
#if !defined(TARGET_PC) || (defined(TARGET_PC) && defined(X_EDITOR))
#include "Entropy.hpp"
#endif

//=========================================================================
// GEOM
//=========================================================================

//
// A single "geom" can be made of multiple "meshs".
// This allows multiple objects to be stored in the same file.
// For example: all LOD levels, a single character object with different heads.
// 
// A "mesh" represents a complete object within a geom.
// For example: an LOD or a model of a head.
//    
// Each mesh is made from one or more "submeshs".
// There is a submesh for every material used by the mesh.
//

struct geom
{
    enum
    {
        VERSION = 41,
    };

    //-------------------------------------------------------------------------

    // Geometry bone
    struct bone
    {
        // Hit location enums 
        enum hit_location
        {
            HIT_LOCATION_START,

            HIT_LOCATION_HEAD               = HIT_LOCATION_START,
            HIT_LOCATION_SHOULDER_LEFT,
            HIT_LOCATION_SHOULDER_RIGHT,
            HIT_LOCATION_TORSO,
            HIT_LOCATION_LEGS,

            HIT_LOCATION_COUNT,

            HIT_LOCATION_UNKNOWN,
            HIT_LOCATION_UNKNOWN_WRONG_GUID,
        };

        // Data
        quaternion      BindRotation;   // Local space bind rotation
        vector3         BindPosition;   // Local space bind position
        bbox            BBox;           // Bounding box in local space
        s16             HitLocation;    // Hit location type
        s16             iRigidBody;     // Index of rigid body to attach to (or -1 if none)

        // Functions
        void FileIO( fileio& File );
    } ;

    // Bone masks
    struct bone_masks
    {
        s32     NameOffset;                 // Offset into string data for name
        s32     nBones;                     // # of bones referenced
        f32     Weights[MAX_ANIM_BONES];    // Weight ( 0 -> 1 ) for each bone
        
        // Functions
        void FileIO( fileio& File );
    };
    
    // Property section
    struct property_section
    {
        // Data
        s16     NameOffset;     // Offset into string data for name 
        s16     iProperty;      // Index of first property in section
        s16     nProperties;    // # of properties in section
        
        // Functions
        void FileIO( fileio& File );
    };
    
    // Property
    struct property
    {
        // Type defines
        enum type
        {
            TYPE_FLOAT,         // 0 = f32
            TYPE_INTEGER,       // 1 = s32
            TYPE_ANGLE,         // 2 = radian
            TYPE_STRING,        // 3 = char*
            
            TYPE_TOTAL          // Total count
        };
        
        // Data
        s16     NameOffset;     // Offset into string data for name
        s16     Type;           // Type of property
        union
        {
            f32     Float;          // Float value
            s32     Integer;        // Integer value
            radian  Angle;          // Angle value
            s32     StringOffset;   // Offset into string data of string
        } Value;
        
        // Functions
        void FileIO( fileio& File );
    };
    
    // Rigid body
    struct rigid_body
    {
        // IK Degrees of freedom info
        struct dof
        {
            // Axis
            enum axis
            {
                DOF_TX,     // X translation
                DOF_TY,     // Y translation
                DOF_TZ,     // Z translation

                DOF_RX,     // X rotation
                DOF_RY,     // Y rotation
                DOF_RZ,     // Z rotation
            };

            // Flags
            enum flags
            {
                FLAG_ACTIVE  = ( 1 << 0 ),      // Axis is active
                FLAG_LIMITED = ( 1 << 1 ),      // Axis is limited
            };
            
            // Data
            u32     Flags;      // Flags
            f32     Min;        // Minimum limit
            f32     Max;        // Maximum limit

            // Functions
            void FileIO( fileio& File );
        };
    
        // Type
        enum type
        {
            TYPE_SPHERE,
            TYPE_CYLINDER,
            TYPE_BOX
        };

        // Flags
        enum Flags
        {
            FLAG_WORLD_COLLISION = ( 1 << 0 ),  // Body has collision with world
        };

        // Data
        quaternion      BodyBindRotation;   // World space body bind rotation
        vector3         BodyBindPosition;   // World space body bind position
        quaternion      PivotBindRotation;  // World space pivot bind rotation
        vector3         PivotBindPosition;  // World space pivot bind position
        s32             NameOffset;         // Offset into string data for name
        f32             Mass;               // Mass of rigid body
        f32             Radius;             // Radius of rigid body
        f32             Width;              // Width of rigid body
        f32             Height;             // Height of rigid body
        f32             Length;             // Length of rigid body
        s16             Type;               // Type of rigid body
        u16             Flags;              // Various flags
        s16             iParentBody;        // Index of parent rigid body (or -1)
        s16             iBone;              // Index of best bone to attach to
        u32             CollisionMask;      // Describes collision with other bodies
        dof             DOF[6];             // Degrees of freedom info

        // Functions
        void FileIO( fileio& File );
    };
    
    struct mesh
    {
        bbox            BBox;
        s16             NameOffset;     // into string data
        s16             nSubMeshes;
        s16             iSubMesh;
        s16             nBones;         // Number of bones used
        s16             nFaces;
        s16             nVertices;

        void FileIO( fileio& File );
    };

    struct submesh
    {
        s16             iDList;             // Index into list of display lists
        s16             iMaterial;          // Index of the Material that this SubMesh uses
        f32             WorldPixelSize;     // Average World Pixel size for this SubMesh
        u32             BaseSortKey;        // used internally by the rendering system

        void FileIO( fileio& File );
    };

    struct material
    {
        struct uvanim
        {
            enum type
            {
                FIXED = 0,
                LOOPED,
                PINGPONG,
                ONESHOT,
            };

            s8  Type;
            s8  StartFrame;
            s8  FPS;
            s8  nKeys;
            s16 iKey;
        
            void FileIO( fileio& File );
        };
    
        enum
        {
            MAX_PARAMS          = 12,
        }; 

        enum
        {
            FLAG_DOUBLE_SIDED       = 0x0001,
            FLAG_HAS_ENV_MAP        = 0x0002,
            FLAG_HAS_DETAIL_MAP     = 0x0004,
            FLAG_ENV_WORLD_SPACE    = 0x0008,
            FLAG_ENV_VIEW_SPACE     = 0x0010,
            FLAG_ENV_CUBE_MAP       = 0x0020,
            FLAG_FORCE_ZFILL        = 0x0040,
            FLAG_ILLUM_USES_DIFFUSE = 0x0080,
            FLAG_IS_PUNCH_THRU      = 0x0100,
            FLAG_IS_ADDITIVE        = 0x0200,
            FLAG_IS_SUBTRACTIVE     = 0x0400
        };

        uvanim          UVAnim;                         // UV Animation data
        f32             DetailScale;
        f32             FixedAlpha;
        u16             Flags;                          // flags
        s8              Type;
        s8              nTextures;                      // Total number of textures used in the material
        s8              iTexture;                       // Index into global texture list for the Geom
        s8              nVirtualMats;                   // Number of registered mats based on this material (1 unless there is a virtual texture present)
        s8              iVirtualMat;                    // Offset to the bitmaps

        void FileIO( fileio& File );
    };

    struct texture
    {
        s16 DescOffset;
        s16 FileNameOffset;
        
        void FileIO( fileio& File );
    };

    struct uvkey
    {
        u8 OffsetU;
        u8 OffsetV;
        
        void FileIO( fileio& File );
    };

    struct virtual_mesh
    {
        s16     NameOffset;
        s16     nLODs;
        s16     iLOD;

        void FileIO( fileio& File );
    };

    struct virtual_material
    {
        xhandle MatHandle;

        void FileIO( fileio& File );
    };

    struct virtual_texture
    {
        s16     NameOffset;         // name of the virtual texture
        u32     MaterialMask;       // mask of which materials it will effect

        void FileIO( fileio& File );
    };

    //-------------------------------------------------------------------------
            
                geom            ( void );
                geom            ( fileio& File );
                ~geom           ( void );
    void        FileIO          ( fileio& File );
    s32         GetNFaces       ( void )          const;
    s32         GetNVerts       ( void )          const;
    xbool       HasUVAnim       ( s32 iMaterial ) const;
    
    s32         AddRef          ( void );
    s32         Release         ( void );
    s32         GetRefCount     ( void ) const;

    s32         GetVMeshIndex   ( const char* pName )        const;
    s32         GetMeshIndex    ( const char* pName )        const;
    s32         GetVTextureIndex( const char* pName )        const;
    s32         GetSubMeshIndex ( s32 iMesh, s32 iMaterial ) const;
    xbool       HasUVAnim       ( s32 iMesh, s32 iMaterial ) const;
    const char* GetVMeshName    ( s32 iVMesh               ) const;
    const char* GetMeshName     ( s32 iMesh                ) const;
    const char* GetVTextureName ( s32 iVTexture            ) const;
    const char* GetTextureDesc  ( s32 iTexture             ) const;
    const char* GetTextureName  ( s32 iTexture             ) const;
    u64         GetLODMask      ( u32 VMeshMask,
                                  u16 ScreenSize           ) const;
    
    // Rigid body functions
    const char*             GetRigidBodyName    ( s32 iRigidBody ) const;
    s32                     GetRigidBodyIndex   ( const char* pName ) const;
                             
    // Bone mask functions                                  
    const bone_masks*       FindBoneMasks       ( const char* pName )const;                                  
    
    // Property search: Returns address of section/property if present
    const property_section* FindPropertySection ( const char* pSection ) const;
    const property*         FindProperty        ( const property_section* pSection, const char* pName, geom::property::type Type ) const;
    
    // Property query: Returns TRUE and sets up the value if present, else just returns FALSE and leaves value
    xbool                   GetPropertyFloat    ( const geom::property_section* pSection, const char* pName, f32*    pValue ) const;
    xbool                   GetPropertyInteger  ( const geom::property_section* pSection, const char* pName, s32*    pValue ) const;
    xbool                   GetPropertyAngle    ( const geom::property_section* pSection, const char* pName, radian* pValue ) const;
    xbool                   GetPropertyString   ( const geom::property_section* pSection, const char* pName, char*   pValue, s32 nChars ) const;
    
    //-------------------------------------------------------------------------

    bbox            m_BBox;
    s16             m_Platform;
    s16             m_RefCount;
    s16             m_Version;
    s16             m_nFaces;       // including all meshes/lods
    s16             m_nVertices;    // including all meshes/lods
    s16             m_nBones;
    s16             m_nBoneMasks;
    s16             m_nPropertySections;
    s16             m_nProperties;
    s16             m_nRigidBodies;
    s16             m_nMeshes;
    s16             m_nSubMeshes;
    s16             m_nMaterials;
    s16             m_nTextures;
    s16             m_nUVKeys;
    s16             m_nLODs;
    s16             m_nVirtualMeshes;
    s16             m_nVirtualMaterials;
    s16             m_nVirtualTextures;
    s16             m_StringDataSize;

    bone*               m_pBone;
    bone_masks*         m_pBoneMasks;
    property_section*   m_pPropertySections;
    property*           m_pProperties;
    rigid_body*         m_pRigidBodies;
    mesh*               m_pMesh;
    submesh*            m_pSubMesh;
    material*           m_pMaterial;
    texture*            m_pTexture;        
    uvkey*              m_pUVKey;
    u16*                m_pLODSizes;
    u64*                m_pLODMasks;
    virtual_mesh*       m_pVirtualMeshes;
    virtual_material*   m_pVirtualMaterials;
    virtual_texture*    m_pVirtualTextures;
    char*               m_pStringData;
    xhandle             m_hGeom;            // handle to the registered geom
};

//=========================================================================

inline s32 geom::AddRef( void )
{
    return (++m_RefCount);
}

//=========================================================================

inline s32 geom::Release( void )
{
    ASSERT( m_RefCount > 0 );
    return (--m_RefCount);
}

//=========================================================================

inline s32 geom::GetRefCount( void ) const
{
    return m_RefCount;
}

//=========================================================================

inline s32 geom::GetNFaces( void ) const
{
    s32 Total = 0;
    for ( s32 i = 0; i < m_nMeshes; i++ )
    {
        Total += m_pMesh[i].nFaces;
    }

    return Total;
}

//=========================================================================

inline s32 geom::GetNVerts( void ) const
{
    s32 Total = 0;
    for ( s32 i = 0; i < m_nMeshes; i++ )
    {
        Total += m_pMesh[i].nVertices;
    }

    return Total;
}

//=========================================================================

inline xbool geom::HasUVAnim( s32 iMaterial ) const
{
    ASSERT( (iMaterial >= 0) && (iMaterial < m_nMaterials) );
    return (m_pMaterial[iMaterial].UVAnim.nKeys > 0);
}

//=========================================================================
// Rigid body functions
//=========================================================================

inline const char* geom::GetRigidBodyName( s32 iRigidBody ) const
{
    ASSERT( ( iRigidBody >= 0 ) && ( iRigidBody < m_nRigidBodies ) );
    return &m_pStringData[ m_pRigidBodies[ iRigidBody ].NameOffset ];
}

//=========================================================================

inline
s32 geom::GetRigidBodyIndex( const char* pName ) const
{
    // Check all rigid bodies
    for( s32 i = 0; i < m_nRigidBodies; i++ )
    {
        // Found?
        if( x_strcmp( GetRigidBodyName( i ), pName ) == 0 )
            return i;
    }
    
    // Not found
    return -1;
}

//=========================================================================

inline s32 geom::GetVMeshIndex( const char* pName ) const
{
    for( s32 i = 0; i < m_nVirtualMeshes; i++ )
    {
        if( !x_strcmp(&m_pStringData[m_pVirtualMeshes[i].NameOffset], pName) )
        {
            return i;
        }
    }

    return -1;
}

//=========================================================================

inline s32 geom::GetMeshIndex( const char* pName ) const
{
    for ( s32 i = 0; i < m_nMeshes; i++ )
    {
        if ( !x_strcmp(&m_pStringData[m_pMesh[i].NameOffset], pName) )
        {
            return i;
        }
    }

    return -1;
}

//=========================================================================

inline s32 geom::GetVTextureIndex( const char* pName ) const
{
    for( s32 i = 0; i < m_nVirtualTextures; i++ )
    {
        if( !x_strcmp(&m_pStringData[m_pVirtualTextures[i].NameOffset], pName) )
        {
            return i;
        }
    }

    return -1;
}

//=========================================================================

inline s32 geom::GetSubMeshIndex( s32 iMesh, s32 iMaterial ) const
{
    ASSERT( (iMesh>=0) && (iMesh<m_nMeshes) );
    for ( s32 i = m_pMesh[iMesh].iSubMesh;
          i < m_pMesh[iMesh].iSubMesh+m_pMesh[iMesh].nSubMeshes;
          i++ )
    {
        if ( m_pSubMesh[i].iMaterial == iMaterial )
            return i;
    }

    return -1;
}

//=========================================================================

inline xbool geom::HasUVAnim( s32 iMesh, s32 iMaterial ) const
{
    s32 iSubMesh = GetSubMeshIndex( iMesh, iMaterial );
    if ( iSubMesh < 0 )
        return FALSE;
    else
        return HasUVAnim( m_pSubMesh[iSubMesh].iMaterial );
}

//=========================================================================

inline const char* geom::GetVMeshName( s32 iVMesh ) const
{
    ASSERT( (iVMesh>=0) && (iVMesh < m_nVirtualMeshes) );
    s32 StringDataOffset = m_pVirtualMeshes[iVMesh].NameOffset;
    return &m_pStringData[StringDataOffset];
}

//=========================================================================

inline const char* geom::GetMeshName( s32 iMesh ) const
{
    ASSERT( (iMesh >= 0) && (iMesh < m_nMeshes) );
    s32 StringDataOffset = m_pMesh[iMesh].NameOffset;
    return &m_pStringData[StringDataOffset];
}

//=========================================================================

inline const char* geom::GetVTextureName( s32 iVTexture ) const
{
    ASSERT( (iVTexture >= 0) && (iVTexture < m_nVirtualTextures) );
    s32 StringDataOffset = m_pVirtualTextures[iVTexture].NameOffset;
    return &m_pStringData[StringDataOffset];
}

//=========================================================================

inline const char* geom::GetTextureDesc( s32 iTexture ) const
{
    ASSERT( (iTexture >= 0) && (iTexture < m_nTextures) );
    s32 StringDataOffset = m_pTexture[iTexture].DescOffset;
    return &m_pStringData[StringDataOffset];
}

//=========================================================================

inline const char* geom::GetTextureName( s32 iTexture ) const
{
    ASSERT( (iTexture >= 0) && (iTexture < m_nTextures) );
    s32 StringDataOffset = m_pTexture[iTexture].FileNameOffset;
    return &m_pStringData[StringDataOffset];
}

//=========================================================================

inline u64 geom::GetLODMask( u32 VMeshMask,
                             u16 ScreenSize ) const
{
    if( m_nVirtualMeshes == 0 )
    {
        return (u64)-1;
    }

    u64 LODMask = 0;
    for( s32 i = 0; i < m_nVirtualMeshes; i++ )
    {
        // check the vmesh mask
        if( (VMeshMask & (1<<i)) &&
            (m_pVirtualMeshes[i].nLODs) )
        {
            // okay, this vmesh is on, which LOD?
            s32 Choice = 0;
            #if !defined( X_RETAIL ) && !defined( TARGET_PC )
            if( !eng_ScreenShotActive() )
            #endif
            {
                for( s32 j = 1; j < m_pVirtualMeshes[i].nLODs; j++ )
                {
                    if( ScreenSize < m_pLODSizes[j+m_pVirtualMeshes[i].iLOD] )
                        Choice = j;
                }
            }

            // or in this LOD into the total mask
            LODMask |= m_pLODMasks[Choice+m_pVirtualMeshes[i].iLOD];
        }
    }

    return LODMask;
}

//=========================================================================
#endif
//=========================================================================

