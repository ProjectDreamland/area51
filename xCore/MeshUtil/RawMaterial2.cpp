#ifdef TARGET_PC
#include <crtdbg.h>
#endif

#include "RawMaterial2.hpp"
#include "RawMaterial.hpp"
#include "TextIn.hpp"
#include "TextOut.hpp"
#include "rawmesh.hpp"

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

rawmaterial2::rawmaterial2( void )
{
    x_memset( this, 0, sizeof(*this) );
}

//=========================================================================

rawmaterial2::~rawmaterial2( void )
{    
    Kill();
}

//=========================================================================

void rawmaterial2::Kill( void )
{
    if(m_pTexture)  delete[]m_pTexture;
    if(m_pMaterial) delete[]m_pMaterial;
    if(m_pSubMesh)  delete[]m_pSubMesh;
    if(m_pParamKey) delete[]m_pParamKey;

    x_memset( this, 0, sizeof(*this) );
}

//=========================================================================

xbool rawmaterial2::Load( const char* pFileName )
{
    Kill();

    rawmaterial     RM;
    RM.Load(pFileName);
    BuildFromRM(RM);
    RM.Kill();

    return TRUE;
}

//=========================================================================

void rawmaterial2::GetMangledName( rgba_source_type     iSource,            // Map that controls mangling
                                   const char*          pSourceName,        // Source filename
                                         char*          pDestBuffer,        // Dest buffer for mangled name
                                   s32                  nMaxChars )         // Max # chars in dest buffer
{
    char    Temp  [ 512         ];
    char    pDrive[ X_MAX_DRIVE ];
    char    pDir  [ X_MAX_DIR   ];
    char    pFName[ X_MAX_FNAME ];

    x_splitpath( pSourceName, pDrive, pDir, pFName, NULL );

    ASSERT(pDestBuffer);

    //==-------------------------------------
    // Determine what the mangled filename 
    // will be
    //==-------------------------------------
    const char* pNameSuffix = NULL;

    switch( iSource )
    {
    case rawmaterial2::RGBA_SOURCE_TYPE_RGB:
        pNameSuffix="[D]";
        break;
    case rawmaterial2::RGBA_SOURCE_TYPE_R:
        pNameSuffix="[R]";
        break;
    case rawmaterial2::RGBA_SOURCE_TYPE_G:
        pNameSuffix="[G]";
        break;
    case rawmaterial2::RGBA_SOURCE_TYPE_B:
        pNameSuffix="[B]";
        break;
    case rawmaterial2::RGBA_SOURCE_TYPE_A:
        pNameSuffix="[A]";
        break;
    default:
        ASSERTS(FALSE,"Found an unknown channel source while compiling maps");
        break;
    }

    x_sprintf( Temp, "%s%s%s%s.xbmp", pDrive, pDir, pFName, pNameSuffix );	

    x_strncpy( pDestBuffer, Temp, nMaxChars );

}

//=========================================================================
//=========================================================================
//=========================================================================
// CONVERT FROM RAWMESH TO RAWMESH2
//=========================================================================
//=========================================================================
//=========================================================================

void SetupMap( rawmaterial2::map& M, rawmaterial::tex_material& TM )
{
    x_memset( &M, 0, sizeof(rawmaterial2::map) );

    M.FilterType    = rawmaterial2::FILTER_TYPE_BILINEAR;
    M.RGBASource    = rawmaterial2::RGBA_SOURCE_TYPE_RGB;
    M.UAddress      = rawmaterial2::ADDRESS_TYPE_WRAP;
    M.VAddress      = rawmaterial2::ADDRESS_TYPE_WRAP;
    M.nTextures     = 1;
    M.TextureFPS    = 0;
    M.iTexture      = TM.iTexture;
    M.iUVChannel    = TM.iChanel;

    switch( TM.FilterType )
    {
        case rawmaterial::FILTER_POINT:  M.FilterType = rawmaterial2::FILTER_TYPE_POINT; break;
        case rawmaterial::FILTER_BILINEAR:  M.FilterType = rawmaterial2::FILTER_TYPE_BILINEAR; break;
        case rawmaterial::FILTER_TRILINEAR:  M.FilterType = rawmaterial2::FILTER_TYPE_TRILINEAR; break;
        case rawmaterial::FILTER_ANISOTROPIC:  M.FilterType = rawmaterial2::FILTER_TYPE_POINT; break;
    }
    
    switch( TM.UAddress )
    {
        case rawmaterial::TEXTURE_WRAP:  M.UAddress = rawmaterial2::ADDRESS_TYPE_WRAP; break;
        case rawmaterial::TEXTURE_CLAMP:  M.UAddress = rawmaterial2::ADDRESS_TYPE_CLAMP; break;
        case rawmaterial::TEXTURE_MIRROR:  M.UAddress = rawmaterial2::ADDRESS_TYPE_MIRROR; break;
    }

    switch( TM.VAddress )
    {
        case rawmaterial::TEXTURE_WRAP:  M.VAddress = rawmaterial2::ADDRESS_TYPE_WRAP; break;
        case rawmaterial::TEXTURE_CLAMP:  M.VAddress = rawmaterial2::ADDRESS_TYPE_CLAMP; break;
        case rawmaterial::TEXTURE_MIRROR:  M.VAddress = rawmaterial2::ADDRESS_TYPE_MIRROR; break;
    }
}

//=========================================================================

static
void SetupMat_Default( rawmaterial2::material& M2 )
{
    M2.Type                     = 0;
    M2.LightingType             = rawmaterial2::LIGHTING_TYPE_STATIC_AND_DYNAMIC;
    M2.BlendType                = rawmaterial2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmaterial2::TINT_TYPE_FINAL_OUTPUT;
    M2.bTwoSided                = FALSE;
    M2.bPunchthrough            = FALSE;
    M2.bRandomizeAnim           = FALSE;
    M2.TintColor.nKeys          = 1;
    M2.TintColor.nParamsPerKey  = 3;
    M2.TintColor.Current[0]     = 255.0f;
    M2.TintColor.Current[1]     = 255.0f;
    M2.TintColor.Current[2]     = 255.0f;
    M2.TintAlpha.nKeys          = 1;
    M2.TintAlpha.nParamsPerKey  = 1;
    M2.TintAlpha.Current[0]     = 255.0f;
}

//=========================================================================

static
void SetupMat_Default( rawmaterial2::material& M2, rawmaterial::material& M )
{
    (void)M;

    M2.Type                     = 0;
    M2.LightingType             = rawmaterial2::LIGHTING_TYPE_STATIC_AND_DYNAMIC;
    M2.BlendType                = rawmaterial2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmaterial2::TINT_TYPE_FINAL_OUTPUT;
    M2.bTwoSided                = FALSE;
    M2.bPunchthrough            = FALSE;
    M2.bRandomizeAnim           = FALSE;
    M2.TintColor.nKeys          = 1;
    M2.TintColor.nParamsPerKey  = 3;
    M2.TintColor.Current[0]     = x_frand(128,255);
    M2.TintColor.Current[1]     = x_frand(0,0);
    M2.TintColor.Current[2]     = x_frand(0,0);
    M2.TintAlpha.nKeys          = 1;
    M2.TintAlpha.nParamsPerKey  = 1;
    M2.TintAlpha.Current[0]     = 255.0f;
}

//=========================================================================

static
void SetupMat_SolidColor( rawmaterial2::material& M2, rawmaterial::material& M )
{
    SetupMat_Default( M2 );

    M2.Type                     = 0;
    M2.LightingType             = rawmaterial2::LIGHTING_TYPE_STATIC_AND_DYNAMIC;
    M2.BlendType                = rawmaterial2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmaterial2::TINT_TYPE_FINAL_OUTPUT;
    M2.bTwoSided                = M.bDoubleSide;
    M2.bPunchthrough            = FALSE;
    M2.bRandomizeAnim           = FALSE;
    M2.TintColor.nKeys          = 1;
    M2.TintColor.nParamsPerKey  = 3;
    M2.TintColor.Current[0]     = 255; //pBase->Color.R;
    M2.TintColor.Current[1]     = 255; //pBase->Color.G;
    M2.TintColor.Current[2]     = 255; //pBase->Color.B;
    M2.TintAlpha.nKeys          = 1;
    M2.TintAlpha.nParamsPerKey  = 1;
    M2.TintAlpha.Current[0]     = 255.0f;
}

//=========================================================================

static
void SetupMat_1Texture( rawmaterial2::material& M2, rawmaterial::material& M )
{
    rawmaterial::tex_material* pBase = M.GetTexMap( rawmaterial::TEX_TYPE_BASE1     );

    SetupMat_Default( M2 );

    M2.Type                     = 1;
    M2.LightingType             = rawmaterial2::LIGHTING_TYPE_STATIC_AND_DYNAMIC;
    M2.BlendType                = rawmaterial2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmaterial2::TINT_TYPE_NONE;
    M2.bTwoSided                = M.bDoubleSide;
    M2.bPunchthrough            = FALSE;
    M2.bRandomizeAnim           = FALSE;

    M2.nMaps = 1;
    SetupMap( M2.Map[0], *pBase );
}

//=========================================================================

static
void SetupMat_1TextureLightmap( rawmaterial2::material& M2, rawmaterial::material& M )
{
    rawmaterial::tex_material* pBase = M.GetTexMap( rawmaterial::TEX_TYPE_BASE1 );
    rawmaterial::tex_material* pLM   = M.GetTexMap( rawmaterial::TEX_TYPE_SELF_ILLUM );

    SetupMat_Default( M2 );

    M2.Type                     = 7;
    M2.LightingType             = rawmaterial2::LIGHTING_TYPE_STATIC_AND_DYNAMIC;
    M2.BlendType                = rawmaterial2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmaterial2::TINT_TYPE_NONE;
    M2.bTwoSided                = M.bDoubleSide;
    M2.bPunchthrough            = FALSE;
    M2.bRandomizeAnim           = FALSE;

    M2.nMaps = 2;
    SetupMap( M2.Map[0], *pBase );
    SetupMap( M2.Map[3], *pLM );
}

//=========================================================================

static
void SetupMat_1TextureOpacity( rawmaterial2::material& M2, rawmaterial::material& M )
{
    rawmaterial::tex_material* pBase   = M.GetTexMap( rawmaterial::TEX_TYPE_BASE1 );
    rawmaterial::tex_material* pAlpha  = M.GetTexMap( rawmaterial::TEX_TYPE_OPACITY_ALPHA );
    rawmaterial::tex_material* pPunch  = M.GetTexMap( rawmaterial::TEX_TYPE_OPACITY_PUNCH );
    ASSERT( pAlpha || pPunch );

    SetupMat_Default( M2 );

    M2.Type                     = 2;
    M2.LightingType             = rawmaterial2::LIGHTING_TYPE_STATIC_AND_DYNAMIC;
    M2.BlendType                = rawmaterial2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmaterial2::TINT_TYPE_NONE;
    M2.bTwoSided                = M.bDoubleSide;
    M2.bPunchthrough            = (pPunch) ? (TRUE):(FALSE);
    M2.bRandomizeAnim           = FALSE;

    M2.nMaps = 2;
    SetupMap( M2.Map[0], *pBase );

    if( pPunch )
        SetupMap( M2.Map[4], *pPunch );
    else
        SetupMap( M2.Map[4], *pAlpha );

    M2.Map[4].RGBASource = rawmaterial2::RGBA_SOURCE_TYPE_A;
}

//=========================================================================

static
void SetupMat_2TexturesBlendedTexture( rawmaterial2::material& M2, rawmaterial::material& M )
{
    rawmaterial::tex_material* pBaseA  = M.GetTexMap( rawmaterial::TEX_TYPE_BASE1 );
    rawmaterial::tex_material* pBaseB  = M.GetTexMap( rawmaterial::TEX_TYPE_BASE2 );
    rawmaterial::tex_material* pMix    = M.GetTexMap( rawmaterial::TEX_TYPE_BLEND_R );

    SetupMat_Default( M2 );

    M2.Type                     = 3;
    M2.LightingType             = rawmaterial2::LIGHTING_TYPE_STATIC_AND_DYNAMIC;
    M2.BlendType                = rawmaterial2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmaterial2::TINT_TYPE_NONE;
    M2.bTwoSided                = M.bDoubleSide;
    M2.bPunchthrough            = FALSE;
    M2.bRandomizeAnim           = FALSE;

    M2.nMaps = 3;
    SetupMap( M2.Map[0], *pBaseA );
    SetupMap( M2.Map[1], *pBaseB );
    SetupMap( M2.Map[2], *pMix );

    M2.Map[2].RGBASource = rawmaterial2::RGBA_SOURCE_TYPE_R;
}

//=========================================================================

static
void SetupMat_2TexturesBlendedTextureLightmap( rawmaterial2::material& M2, rawmaterial::material& M )
{
    rawmaterial::tex_material* pBaseA  = M.GetTexMap( rawmaterial::TEX_TYPE_BASE1 );
    rawmaterial::tex_material* pBaseB  = M.GetTexMap( rawmaterial::TEX_TYPE_BASE2 );
    rawmaterial::tex_material* pMix    = M.GetTexMap( rawmaterial::TEX_TYPE_BLEND_R );
    rawmaterial::tex_material* pLM     = M.GetTexMap( rawmaterial::TEX_TYPE_SELF_ILLUM );

    SetupMat_Default( M2 );

    M2.Type                     = 5;
    M2.LightingType             = rawmaterial2::LIGHTING_TYPE_STATIC_AND_DYNAMIC;
    M2.BlendType                = rawmaterial2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmaterial2::TINT_TYPE_NONE;
    M2.bTwoSided                = M.bDoubleSide;
    M2.bPunchthrough            = FALSE;
    M2.bRandomizeAnim           = FALSE;

    M2.nMaps = 3;
    SetupMap( M2.Map[0], *pBaseA );
    SetupMap( M2.Map[1], *pBaseB );
    SetupMap( M2.Map[2], *pMix );
    SetupMap( M2.Map[3], *pLM );

    M2.Map[2].RGBASource = rawmaterial2::RGBA_SOURCE_TYPE_R;
}

//=========================================================================

static
void SetupMat_2TexturesBlendedVertex( rawmaterial2::material& M2, rawmaterial::material& M )
{
    rawmaterial::tex_material* pBaseA  = M.GetTexMap( rawmaterial::TEX_TYPE_BASE1 );
    rawmaterial::tex_material* pBaseB  = M.GetTexMap( rawmaterial::TEX_TYPE_BASE2 );

    SetupMat_Default( M2 );

    M2.Type                     = 4;
    M2.LightingType             = rawmaterial2::LIGHTING_TYPE_STATIC_AND_DYNAMIC;
    M2.BlendType                = rawmaterial2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmaterial2::TINT_TYPE_NONE;
    M2.bTwoSided                = M.bDoubleSide;
    M2.bPunchthrough            = FALSE;
    M2.bRandomizeAnim           = FALSE;

    M2.nMaps = 2;
    SetupMap( M2.Map[0], *pBaseA );
    SetupMap( M2.Map[1], *pBaseB );
}

//=========================================================================

static
void SetupMat_2TexturesBlendedVertexWithOpacity( rawmaterial2::material& M2, rawmaterial::material& M )
{
    rawmaterial::tex_material* pBaseA  = M.GetTexMap( rawmaterial::TEX_TYPE_BASE1 );
    rawmaterial::tex_material* pBaseB  = M.GetTexMap( rawmaterial::TEX_TYPE_BASE2 );
    rawmaterial::tex_material* pAlpha  = M.GetTexMap( rawmaterial::TEX_TYPE_OPACITY_ALPHA );
    rawmaterial::tex_material* pPunch  = M.GetTexMap( rawmaterial::TEX_TYPE_OPACITY_PUNCH );
    ASSERT( pAlpha || pPunch );


    SetupMat_Default( M2 );

    M2.Type                     = 8;
    M2.LightingType             = rawmaterial2::LIGHTING_TYPE_STATIC_AND_DYNAMIC;
    M2.BlendType                = rawmaterial2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmaterial2::TINT_TYPE_NONE;
    M2.bTwoSided                = M.bDoubleSide;
    M2.bPunchthrough            = (pPunch) ? (TRUE):(FALSE);
    M2.bRandomizeAnim           = FALSE;

    M2.nMaps = 3;

    SetupMap( M2.Map[0], *pBaseA );
    SetupMap( M2.Map[1], *pBaseB );

    if( pPunch )
        SetupMap( M2.Map[4], *pPunch );
    else
        SetupMap( M2.Map[4], *pAlpha );

    M2.Map[4].RGBASource = rawmaterial2::RGBA_SOURCE_TYPE_A;
}

//=========================================================================

void rawmaterial2::BuildFromRM( rawmaterial& RM )
{
    Kill();

    m_nTextures = RM.m_nTextures;
    if (m_nTextures > 0)
    {
        m_pTexture = new texture[ RM.m_nTextures ];
        x_memcpy( m_pTexture, RM.m_pTexture, sizeof(texture)*m_nTextures );
    }
    else
        m_pTexture = NULL;

    m_nSubMeshs = RM.m_nSubMeshs;
    if (m_nSubMeshs > 0)
    {
        m_pSubMesh = new sub_mesh[ RM.m_nSubMeshs ];
        x_memcpy( m_pSubMesh, RM.m_pSubMesh, sizeof(sub_mesh)*m_nSubMeshs );
    }
    else
        m_pSubMesh = NULL;

    //
    // Build materials & maps
    //

    // Allocate materials
    m_pMaterial = new material[ RM.m_nMaterials ];
    m_nMaterials = RM.m_nMaterials;
    x_memset( m_pMaterial, 0, sizeof(material)*m_nMaterials );
    for( s32 m=0; m<m_nMaterials; m++ )
    {
        // Get easy access to the two materials
        rawmaterial2::material& M2 = m_pMaterial[m];
        rawmaterial::material& M   = RM.m_pMaterial[m];

        // Clear new material and copy over name
        x_memset( &M2, 0, sizeof(rawmaterial2::material) );
        x_strcpy( M2.Name, M.Name );

        //
        // Try to recognize original material intent and call correct
        // conversion
        //

        rawmaterial::tex_material* pBase[4];
        rawmaterial::tex_material* pMix[3];
        rawmaterial::tex_material* pSelfIllum;
        rawmaterial::tex_material* pOpacity[2];

        pBase[0] = M.GetTexMap( rawmaterial::TEX_TYPE_BASE1     );
        pBase[1] = M.GetTexMap( rawmaterial::TEX_TYPE_BASE2     );
        pBase[2] = M.GetTexMap( rawmaterial::TEX_TYPE_BASE3     );
        pBase[3] = M.GetTexMap( rawmaterial::TEX_TYPE_BASE4     );
        pMix[0]  = M.GetTexMap( rawmaterial::TEX_TYPE_BLEND_R   );
        pMix[1]  = M.GetTexMap( rawmaterial::TEX_TYPE_BLEND_RG  );
        pMix[2]  = M.GetTexMap( rawmaterial::TEX_TYPE_BLEND_RGB );
        pOpacity[0]  = M.GetTexMap( rawmaterial::TEX_TYPE_OPACITY_ALPHA );
        pOpacity[1]  = M.GetTexMap( rawmaterial::TEX_TYPE_OPACITY_PUNCH );
        pSelfIllum = M.GetTexMap( rawmaterial::TEX_TYPE_SELF_ILLUM );

        xbool bOpacity = (pOpacity[0] || pOpacity[1]);

        if( pMix[0] && pBase[0] && pBase[1] && (pMix[0]->ChanelType == rawmaterial::TEX_MATERIAL_VERTEX) && bOpacity )
        {
            // Two base textures blended using vertex alpha with an opacity map
            x_DebugMsg("MAT: 2 Base blended using vert alpha with opacity map <%s>\n",M.Name);
            SetupMat_2TexturesBlendedVertexWithOpacity(M2,M); //
        }
        else
        if( (pBase[0]==NULL) && (!bOpacity) )
        {
            // No material, make a solid random color
            x_DebugMsg("MAT: No material <%s>\n",M.Name);
            SetupMat_Default(M2,M); //
        }
        else
        if( pMix[0] && pBase[0] && pBase[1] && (pMix[0]->ChanelType == rawmaterial::TEX_MATERIAL_VERTEX) && (!bOpacity) )
        {
            // Two base textures blended using vertex alpha
            x_DebugMsg("MAT: 2 Base blended using vert alpha <%s>\n",M.Name);
            SetupMat_2TexturesBlendedVertex(M2,M); //
        }
        else
        if( pMix[0] && pBase[0] && pBase[1] && (pMix[0]->ChanelType == rawmaterial::TEX_MATERIAL_TEXTURE) && (!bOpacity) && (!pSelfIllum) )
        {
            // Two base textures blended using blend texture
            x_DebugMsg("MAT: 2 Base blended using blend texture <%s>\n",M.Name);
            SetupMat_2TexturesBlendedTexture(M2,M); //
        }
        else 
        if( pMix[0] && pBase[0] && pBase[1] && (pMix[0]->ChanelType == rawmaterial::TEX_MATERIAL_TEXTURE) && (!bOpacity) && (pSelfIllum) )
        {
            // Two base textures blended using blend texture with final lightmap (in self illum slot of old material)
            x_DebugMsg("MAT: 2 Base blended using blend texture, * lightmap<%s>\n",M.Name);
            SetupMat_2TexturesBlendedTextureLightmap(M2,M); //
        }
        else
        if( pBase[0] && (pBase[0]->ChanelType == rawmaterial::TEX_MATERIAL_TEXTURE) && bOpacity  )
        {
            // Single texture with opacity
            x_DebugMsg("MAT: 1 Base with Opacity <%s>\n",M.Name);
            SetupMat_1TextureOpacity(M2,M); //
        }
        else
        if( pBase[0] && (pBase[0]->ChanelType == rawmaterial::TEX_MATERIAL_TEXTURE) && pSelfIllum && (!bOpacity)  )
        {
            // Single texture with lightmap applied
            x_DebugMsg("MAT: 1 Base with Lightmap <%s>\n",M.Name);
            SetupMat_1TextureLightmap(M2,M); //
        }
        else
        if( pBase[0] && (pBase[0]->ChanelType == rawmaterial::TEX_MATERIAL_TEXTURE) && (!bOpacity) )
        {
            // Plain old single-texture
            x_DebugMsg("MAT: 1 Base <%s>\n",M.Name);
            SetupMat_1Texture(M2,M); //
        }
        else
        if( pBase[0] && (pBase[0]->ChanelType == rawmaterial::TEX_MATERIAL_VERTEX) && (!bOpacity)  )
        {
            // Solid color, no textures
            x_DebugMsg("MAT: Solid color (%d,%d,%d) <%s>\n",pBase[0]->Color.R,pBase[0]->Color.G,pBase[0]->Color.B,M.Name);
            SetupMat_SolidColor(M2,M); //
        }
        else        
        {
            x_DebugMsg("MAT: DON'T KNOW <%s>\n",M.Name);
            SetupMat_Default(M2,M);//
        }
    }
}

//=========================================================================

