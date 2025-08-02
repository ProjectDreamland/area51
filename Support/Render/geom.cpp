
#include "geom.hpp"

//=========================================================================

geom::geom( void )
{    
    x_memset( this, 0, sizeof(geom) );
    m_Version = geom::VERSION;
    m_hGeom   = HNULL;
}

//=========================================================================

geom::geom( fileio& File )
{
    (void)File;
    m_RefCount = 0;
    m_hGeom    = HNULL;

}

//=========================================================================

geom::~geom( void )
{
#if !defined(APP_EDITOR) || !defined(CONFIG_OPTDEBUG)
    ASSERT( GetRefCount()==0 );
#endif
}

//=========================================================================

void geom::FileIO( fileio& File )
{
    File.Static( m_BBox              );
    File.Static( m_Platform          );
    File.Static( m_Version           );
    File.Static( m_nFaces            );
    File.Static( m_nVertices         );
    File.Static( m_nBones            );
    File.Static( m_nBoneMasks        );
    File.Static( m_nPropertySections );
    File.Static( m_nProperties       );
    File.Static( m_nRigidBodies      );
    File.Static( m_nMeshes           );
    File.Static( m_nSubMeshes        );
    File.Static( m_nMaterials        );
    File.Static( m_nTextures         );
    File.Static( m_nUVKeys           );
    File.Static( m_nLODs             );
    File.Static( m_nVirtualMeshes    );
    File.Static( m_nVirtualMaterials );
    File.Static( m_nVirtualTextures  );
    File.Static( m_StringDataSize    );

    File.Static( m_pBone,             m_nBones            );
    File.Static( m_pBoneMasks,        m_nBoneMasks        );
    File.Static( m_pPropertySections, m_nPropertySections );
    File.Static( m_pProperties,       m_nProperties       );
    File.Static( m_pRigidBodies,      m_nRigidBodies      );
    File.Static( m_pMesh,             m_nMeshes           );
    File.Static( m_pSubMesh,          m_nSubMeshes        );
    File.Static( m_pMaterial,         m_nMaterials        );
    File.Static( m_pTexture,          m_nTextures         );
    File.Static( m_pUVKey,            m_nUVKeys           );
    File.Static( m_pLODSizes,         m_nLODs             );
    File.Static( m_pLODMasks,         m_nLODs             );
    File.Static( m_pVirtualMeshes,    m_nVirtualMeshes    );
    File.Static( m_pVirtualMaterials, m_nVirtualMaterials );
    File.Static( m_pVirtualTextures,  m_nVirtualTextures  );
    File.Static( m_pStringData,       m_StringDataSize    );
}

//=========================================================================

void geom::texture::FileIO( fileio& File )
{
    File.Static( DescOffset     );
    File.Static( FileNameOffset );
}

//=========================================================================

void geom::material::FileIO( fileio& File )
{
    File.Static( UVAnim             );
    File.Static( DetailScale        );
    File.Static( FixedAlpha         );
    File.Static( Flags              );
    File.Static( Type               );
    File.Static( nTextures          );
    File.Static( iTexture           );
    File.Static( nVirtualMats       );
    File.Static( iVirtualMat        );
}

//=========================================================================

void geom::material::uvanim::FileIO( fileio& File )
{
    File.Static( Type       );
    File.Static( StartFrame );
    File.Static( FPS        );
    File.Static( nKeys      );
    File.Static( iKey       );
}

//=========================================================================

void geom::uvkey::FileIO( fileio& File )
{
    File.Static( OffsetU );
    File.Static( OffsetV );
}

//=========================================================================

void geom::virtual_mesh::FileIO( fileio& File )
{
    File.Static( NameOffset );
    File.Static( nLODs      );
    File.Static( iLOD       );
}

//=========================================================================

void geom::virtual_material::FileIO( fileio& File )
{
    (void)File;
}

//=========================================================================

void geom::virtual_texture::FileIO( fileio& File )
{
    File.Static( MaterialMask );
    File.Static( NameOffset   );
}

//=========================================================================

void geom::submesh::FileIO( fileio& File )
{
    File.Static( iDList         );
    File.Static( iMaterial      );
    File.Static( WorldPixelSize );
}

//=========================================================================

void geom::mesh::FileIO( fileio& File )
{
    File.Static( BBox       );
    File.Static( NameOffset );
    File.Static( nSubMeshes );
    File.Static( iSubMesh   );
    File.Static( nBones     );
    File.Static( nFaces     );
    File.Static( nVertices  );
}

//=========================================================================

void geom::bone::FileIO( fileio& File )
{
    File.Static( BindRotation );
    File.Static( BindPosition );
    File.Static( BBox         );
    File.Static( HitLocation  );
    File.Static( iRigidBody   );
}

//=========================================================================

void geom::bone_masks::FileIO( fileio& File )
{
    File.Static( NameOffset );
    File.Static( nBones     );
    for( s32 i = 0; i < MAX_ANIM_BONES; i++ )
        File.Static( Weights[i] );
};

//=========================================================================

void geom::property_section::FileIO( fileio& File )
{
    File.Static( NameOffset );
    File.Static( iProperty );
    File.Static( nProperties );
}

//=========================================================================

void geom::property::FileIO( fileio& File )
{
    File.Static( NameOffset );
    File.Static( Type );
    switch( Type )
    {
    case geom::property::TYPE_FLOAT:
        File.Static( Value.Float );
        break;
    case geom::property::TYPE_INTEGER:
        File.Static( Value.Integer );
        break;
    case geom::property::TYPE_ANGLE:
        File.Static( Value.Angle );
        break;
    case geom::property::TYPE_STRING:
        File.Static( Value.StringOffset );
        break;
    default:
        ASSERTS( 0, "You need to add this new type!!" );        
    }
}

//=========================================================================

void geom::rigid_body::dof::FileIO( fileio& File )
{
    File.Static( Flags );
    File.Static( Min   );
    File.Static( Max   );
}

//=========================================================================

void geom::rigid_body::FileIO( fileio& File )
{
    File.Static( BodyBindRotation   );
    File.Static( BodyBindPosition   );
    File.Static( PivotBindRotation  );
    File.Static( PivotBindPosition  );
    File.Static( NameOffset         );
    File.Static( Mass               );
    File.Static( Radius             );
    File.Static( Width              );
    File.Static( Height             );
    File.Static( Length             );
    File.Static( Type               );    
    File.Static( Flags              );    
    File.Static( iParentBody        );    
    File.Static( iBone              );    
    File.Static( CollisionMask      );
    File.Static( DOF, 6             );
}

//=========================================================================
// Bone mask functions
//=========================================================================

const geom::bone_masks* geom::FindBoneMasks( const char* pName )const
{
    // Loop through all bone masks
    for( s32 i = 0; i < m_nBoneMasks; i++ )
    {
        // Lookup name of bone masks entry
        const char* pBoneMasksName = &m_pStringData[m_pBoneMasks[i].NameOffset];

        // Match?
        if( x_strcmp( pBoneMasksName, pName ) == 0 )
            return &m_pBoneMasks[i];
    }

    // Not found            
    return NULL;
}

//=========================================================================
// Property functions
//=========================================================================

const geom::property_section* geom::FindPropertySection ( const char* pSection ) const
{
    ASSERT( pSection );

    // Search all property sections
    for( s32 iSection = 0; iSection < m_nPropertySections; iSection++ )
    {
        // Lookup section info
        const geom::property_section& Section = m_pPropertySections[iSection];
        const char* pSectionName = &m_pStringData[ Section.NameOffset ];

        // Matching section?
        if( x_strcmp( pSectionName, pSection ) == 0 )
            return &Section;
    }
    
    // Not found
    return NULL;
}

//=========================================================================

const geom::property* geom::FindProperty( const property_section* pSection, const char* pName, geom::property::type Type ) const
{
    // Make sure section is valid
    ASSERT( pSection );
    ASSERT( pSection >= &m_pPropertySections[ 0 ] );
    ASSERT( pSection <  &m_pPropertySections[ m_nPropertySections ] );
    
    // Search all properties in the section
    const geom::property* Properties = &m_pProperties[ pSection->iProperty ];
    for( s32 iProperty = 0; iProperty < pSection->nProperties; iProperty++ )
    {
        // Lookup property
        const geom::property& Property = Properties[ iProperty ];
        
        // Matching type?
        if( Type == (geom::property::type)Property.Type ) 
        {
            // Matching name?
            const char* pPropName = &m_pStringData[ Property.NameOffset ];
            if( x_strcmp( pPropName, pName ) == 0 )
                return &Property;
        }                
    }
    
    // Not found
    return NULL;
}

//=========================================================================

xbool geom::GetPropertyFloat( const geom::property_section* pSection, const char* pName, f32* pValue ) const
{
    ASSERT( pSection );
    ASSERT( pName );
    ASSERT( pValue );

    // Lookup property and fail if not found
    const geom::property* pProp = FindProperty( pSection, pName, geom::property::TYPE_FLOAT );
    if( !pProp )
        return FALSE;
        
    // Grab value and return success
    *pValue = pProp->Value.Float;
    return TRUE;
}

//=========================================================================

xbool geom::GetPropertyInteger( const geom::property_section* pSection, const char* pName, s32* pValue ) const
{
    ASSERT( pSection );
    ASSERT( pName );
    ASSERT( pValue );

    // Lookup property and fail if not found
    const geom::property* pProp = FindProperty( pSection, pName, geom::property::TYPE_INTEGER );
    if( !pProp )
        return FALSE;

    // Grab value and return success
    *pValue = pProp->Value.Integer;
    return TRUE;
}

//=========================================================================

xbool geom::GetPropertyAngle( const geom::property_section* pSection, const char* pName, radian* pValue ) const
{
    ASSERT( pSection );
    ASSERT( pName );
    ASSERT( pValue );
    
    // Lookup property and fail if not found
    const geom::property* pProp = FindProperty( pSection, pName, geom::property::TYPE_ANGLE );
    if( !pProp )
        return FALSE;

    // Grab value and return success
    *pValue = pProp->Value.Angle;
    return TRUE;
}

//=========================================================================

xbool geom::GetPropertyString ( const geom::property_section* pSection, const char* pName, char* pValue, s32 nChars ) const
{
    ASSERT( pSection );
    ASSERT( pName );
    ASSERT( pValue );

    // Lookup property and fail if not found
    const geom::property* pProp = FindProperty( pSection, pName, geom::property::TYPE_STRING );
    if( !pProp )
        return FALSE;

    // Grab value and return success
    x_strsavecpy( pValue, &m_pStringData[ pProp->Value.StringOffset ], nChars );
    return TRUE;
}

//=========================================================================


