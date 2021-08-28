//=============================================================================
//
//  SkinInst.cpp
//
//=============================================================================

#include "Entropy.hpp"
#include "Objects\Render\SkinInst.hpp"

//=============================================================================

#ifdef TARGET_XBOX
void xbox_Unregister ( skin_geom* pGeom );
#endif

//=============================================================================
// LOADER FOR THE SKIN GEOM RESOURCE
//=============================================================================

static struct skin_loader : public rsc_loader
{
    //-------------------------------------------------------------------------
    
    skin_loader( void ) : rsc_loader( "SKIN GEOM", ".skingeom" ) {}

    //-------------------------------------------------------------------------
    
    virtual void* PreLoad ( X_FILE* FP )
    {
        MEMORY_OWNER( "SKIN GEOM DATA" );
        fileio File;
        return( File.PreLoad( FP ) );
    }

    //-------------------------------------------------------------------------
    
    virtual void* Resolve ( void* pData ) 
    {
        fileio     File;
        skin_geom* pSkinGeom = NULL;

        File.Resolved( (fileio::resolve*)pData, pSkinGeom );

        return( pSkinGeom );
    }

    //-------------------------------------------------------------------------
    
    virtual void Unload( void* pData )
    {
        skin_geom* pSkinGeom = (skin_geom*)pData;
        ASSERT( pSkinGeom );

        #ifdef TARGET_XBOX
        xbox_Unregister( pSkinGeom );
        #endif

        delete pSkinGeom;
    }

} s_Skin_Geom_Loader;

//=============================================================================
// FUNCTIONS
//=============================================================================

skin_inst::skin_inst( void ) :
    render_inst(),
    m_MinAmbient(0,0,0,255),
    m_OtherAmbientAmount(0.4f)
{
}

//=============================================================================

skin_inst::~skin_inst( void )
{
    if ( m_hInst.IsNonNull() )
    {
        render::UnregisterSkinInstance( m_hInst );
    }
}

//=============================================================================

const char* skin_inst::GetSkinGeomName( void ) const
{
    return( m_hSkinGeom.GetName() );
}

//=============================================================================

void skin_inst::Render( const matrix4* pL2W,
                        const matrix4* pBone,
                        s32            nBone,
                        u32            Flags,
                        u64            LODMask,
                        const xcolor&  Ambient )
{
    (void)nBone;
    (void)pL2W;

    // calculate ambient
    vector3 AmbientVec( Ambient.R*m_OtherAmbientAmount,
                        Ambient.G*m_OtherAmbientAmount,
                        Ambient.B*m_OtherAmbientAmount );
    AmbientVec += vector3( (f32)m_MinAmbient.R, (f32)m_MinAmbient.G, (f32)m_MinAmbient.B );
    AmbientVec.Min(255.0f);

    s32 Alpha = (s32)(Ambient.A * m_Alpha) / 255;

    // Add a Skin Instance
    render::AddSkinInstance( m_hInst,
                             pBone,
                             LODMask,
                             m_VTextureMask,
                             Flags,
                             xcolor( (u8)AmbientVec.GetX(), (u8)AmbientVec.GetY(), (u8)AmbientVec.GetZ(), (u8)Alpha ) );
}

//=============================================================================

void skin_inst::RenderDistortion( const matrix4* pL2W,
                                  const matrix4* pBone,
                                  s32            nBone,
                                  u32            Flags,
                                  u64            LODMask,
                                  const radian3& NormalRot,
                                  const xcolor&  Ambient )
{
    (void)nBone;
    (void)pL2W;

    // Add a Skin Instance
    render::AddSkinInstanceDistorted( m_hInst, pBone, LODMask, Flags, NormalRot, Ambient );
}

//=============================================================================

void skin_inst::RenderShadowCast( const matrix4* pL2W,
                                  const matrix4* pBone,
                                  s32            nBone,
                                  u32            Flags,
                                  u64            LODMask,
                                  u64            ProjMask )
{
    (void)pL2W;
    (void)nBone;
    (void)Flags;

    // add the shadow
    render::AddSkinCaster( m_hInst,
                           pBone,
                           LODMask,
                           ProjMask );
}

//=============================================================================

void skin_inst::OnEnumProp( prop_enum& List )
{
    // Important: The Header and External MUST be enumerated first!
    List.PropEnumHeader  ( "RenderInst", "Render Instance", 0 );
    List.PropEnumExternal( "RenderInst\\File", "Resource\0skingeom", "Resource File", PROP_TYPE_MUST_ENUM );
    List.PropEnumColor   ( "RenderInst\\MinAmbient",   "Minimum amount of ambient this instance will receive.", 0 );
    List.PropEnumFloat   ( "RenderInst\\OtherAmbient", "Percentage of floor color that will influence ambient (between 0 and 1).", 0 );

    // enumerate the vtexture list
    s32 HeaderId = List.PushPath( "RenderInst\\" );
    m_VTextureMask.OnEnumProp( List, GetGeom() );
    List.PopPath( HeaderId );

    render_inst::OnEnumProp( List );
}

//=============================================================================

xbool skin_inst::OnProperty( prop_query& I )
{
    CONTEXT( "skin_inst::OnProperty" );

    if( render_inst::OnProperty( I ) )
        return( TRUE );

     // External
    if( I.IsVar( "RenderInst\\File" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hSkinGeom.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();
            ASSERT( pString );
            
            // Clear?
            if( x_strcmp( pString, "<null>" ) == 0 )
            {
                SetUpSkinGeom( "" );
            }
            else if( pString[0] )
            {
                // Setup
                SetUpSkinGeom( pString );
            }

            // if the filename has changed, this means we need to reset the vmesh mask
#if defined(X_EDITOR)
            m_VMeshMask.nVMeshes      = 0;
            m_VTextureMask.nVTextures = 0;
#endif
            m_VMeshMask.VMeshMask       = 0xffffffff;
            m_VTextureMask.VTextureMask = 0;
        }
        return( TRUE );
    }

    if ( I.VarColor( "RenderInst\\MinAmbient", m_MinAmbient ) )
    {
        return TRUE;
    }

    if ( I.VarFloat( "RenderInst\\OtherAmbient", m_OtherAmbientAmount ) )
    {
        return TRUE;
    }

    // handle the vtexture list
    s32 HeaderId = I.PushPath( "RenderInst\\" );
    if( m_VTextureMask.OnProperty( I, GetGeom() ) )
    {
        I.PopPath( HeaderId );
        return TRUE;
    }
    I.PopPath( HeaderId );

    return( FALSE );
}

//=============================================================================

void skin_inst::SetUpSkinGeom( const char* fileName)
{
    if ( m_hInst.IsNonNull() )
    {
        render::UnregisterSkinInstance( m_hInst );
        m_hInst = HNULL;
    }

    m_hSkinGeom.SetName( fileName );
    skin_geom* pSkinGeom = m_hSkinGeom.GetPointer();
    if( pSkinGeom )
    {
        // Register the instance with the Render Manager
        m_hInst = render::RegisterSkinInstance( *pSkinGeom );
    }
}

//=============================================================================

const skin_inst& skin_inst::operator = ( const skin_inst& Skin )
{
    m_hSkinGeom = Skin.m_hSkinGeom;
    SetUpSkinGeom( m_hSkinGeom.GetName() );

    m_VMeshMask    = Skin.m_VMeshMask;
    m_VTextureMask = Skin.m_VTextureMask;

    return *this;
}

//=============================================================================

#ifdef X_EDITOR
void skin_inst::UnregiserInst( void )
{
    if ( m_hInst.IsNonNull() )
    {
        render::UnregisterSkinInstance( m_hInst );
        m_hInst = HNULL;
    }
}
#endif // X_EDITOR

//=============================================================================

#ifdef X_EDITOR
void skin_inst::RegisterInst( void )
{
    skin_geom* pGeom = m_hSkinGeom.GetPointer();
    if ( pGeom )
    {
        m_hInst = render::RegisterSkinInstance( *pGeom );
    }
}
#endif // X_EDITOR

//=========================================================================

void skin_inst::SetVirtualTexture( const char* VTextureName, const char* DiffuseTextureDesc )
{
    geom* pGeom = GetGeom();
    if( !pGeom )
        return;

    // get a reference to the virtual texture we are talking about
    s32 VTextureIndex = pGeom->GetVTextureIndex( VTextureName );
    if( VTextureIndex < 0 )
        return;
    geom::virtual_texture& GeomVTex = pGeom->m_pVirtualTextures[VTextureIndex];

    // Figure out any material that this vtexture effects...this is a bit
    // nasty, but necessary due to the way vtextures are structured by the
    // geometry system.
    s32 i;
    for( i = 0; i < pGeom->m_nMaterials; i++ )
    {
        if( GeomVTex.MaterialMask & (1<<i) )
            break;
    }
    if( i == pGeom->m_nMaterials )
        return;
    geom::material& GeomMat = pGeom->m_pMaterial[i];

    // now from the material, see if we can find a texture description that matches
    // the diffuse texture description
    u32 Mask = 0x0f<<(VTextureIndex*4);
    for( i = GeomMat.iTexture; i < (GeomMat.iTexture+GeomMat.nTextures); i++ )
    {
        if( !x_stricmp( DiffuseTextureDesc, pGeom->GetTextureDesc( i ) ) )
        {
            m_VTextureMask.VTextureMask &= ~Mask;
            m_VTextureMask.VTextureMask |= (i-GeomMat.iTexture)<<(VTextureIndex*4);
            break;
        }
    }
}

//=============================================================================

void skin_inst::SetVirtualTexture( s32 VTexture )
{
    m_VTextureMask.VTextureMask = VTexture;
}

//=============================================================================
