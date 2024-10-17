#ifdef TARGET_PC
#include <crtdbg.h>
#endif

#include "RawMaterial.hpp"
#include "TextIn.hpp"
#include "TextOut.hpp"

//=========================================================================
// FUNCTIONS
//=========================================================================

//#define RMESH_USE_SANITY

#ifdef RMESH_USE_SANITY
#define RMESH_SANITY    x_MemSanity(); 
#else
#define RMESH_SANITY
#endif

//=========================================================================

const char *    rawmaterial::m_CachedFile = 0;
s32 rawmaterial::m_nMaterials = 0;
s32 rawmaterial::m_nSubMeshs = 0;
s32 rawmaterial::m_nTextures = 0;
rawmaterial::texture* rawmaterial::m_pTexture = 0;
rawmaterial::material* rawmaterial::m_pMaterial = 0;
rawmaterial::sub_mesh* rawmaterial::m_pSubMesh = 0;



//=========================================================================

rawmaterial::rawmaterial( void )
{
    //x_memset( this, 0, sizeof(*this) );
}

//=========================================================================

rawmaterial::~rawmaterial( void )
{    
    Kill();
}

//=========================================================================

void rawmaterial::Kill( void )
{
    // is cache invalid
    if (!m_CachedFile)
    {
        if(m_pTexture)  delete[]m_pTexture;
        if(m_pMaterial) delete[]m_pMaterial;
        if(m_pSubMesh)  delete[]m_pSubMesh;
        x_memset( this, 0, sizeof(*this) );
        m_pTexture = 0;
        m_pMaterial = 0;
        m_pSubMesh = 0;
        m_nTextures = 0;
        m_nMaterials = 0;
        m_nSubMeshs = 0;
    }
}

//=========================================================================

xbool rawmaterial::Load( const char* pFileName )
{
    s32 i,j;

    // Is cache valid
    if (m_CachedFile)
    {
        // Is file currently cached?
        if (!x_strcmp(this->m_CachedFile, pFileName))
            return TRUE;


        if(m_pTexture)  delete[]m_pTexture;
        if(m_pMaterial) delete[]m_pMaterial;
        if(m_pSubMesh)  delete[]m_pSubMesh;
        m_pTexture = 0;
        m_pMaterial = 0;
        m_pSubMesh = 0;
        m_nTextures = 0;
        m_nMaterials = 0;
        m_nSubMeshs = 0;
        x_free((void *)m_CachedFile);
        m_CachedFile = 0;
    }

    text_in File;
    byte* pMatUsed = (byte*)x_malloc(sizeof(byte)*4096*256);
    ASSERT( pMatUsed );
    x_memset(pMatUsed,0,4096*256);

    //
    // Open the file
    //
    File.OpenFile( pFileName );

    while( File.ReadHeader() == TRUE )
    {
        if( x_stricmp( File.GetHeaderName(), "Polygons"     ) == 0 )
        {
            // Allocate the vertex count
            s32 m_nFacets   = File.GetHeaderCount();

            // Read each of the fields in the file
            for( s32 i=0; i<m_nFacets; i++ )
            {
                s32     Index;
                s32     iMesh;
                s32     iMaterial;

                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                File.GetField( "Index:d",      &Index     );
                File.GetField( "iMesh:d",      &iMesh     );
                File.GetField( "iMaterial:d",  &iMaterial );
        
                ASSERT( (iMesh<4096) && (iMaterial<256) );

                pMatUsed[ iMesh*256 + iMaterial ] = 1;
            }            
        }
        else if( x_stricmp( File.GetHeaderName(), "Textures"     ) == 0 )
        {
            // Allocate the vertex count
            m_nTextures = File.GetHeaderCount();
            m_pTexture  = new texture[ m_nTextures ];
            ASSERT( m_pTexture );
            x_memset( m_pTexture, 0, sizeof(texture)*m_nTextures );

            for( s32 i=0; i<m_nTextures; i++ )
            {
                texture&    Texture = m_pTexture[i];
                s32         Index;

                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                File.GetField( "Index:d",      &Index            );
                File.GetField( "FileName:s",    Texture.FileName );
            }
        }
        else if( x_stricmp( File.GetHeaderName(), "Materials"    ) == 0 )
        {
            // Allocate the vertex count
            m_nMaterials = File.GetHeaderCount();
            m_pMaterial  = new material[ m_nMaterials ];
            ASSERT( m_pMaterial );
            x_memset( m_pMaterial, 0, sizeof(material)*m_nMaterials );

            for( s32 i=0; i<m_nMaterials; i++ )
            {
                material&   Material = m_pMaterial[i];
                s32         Index;
                char        MaterialType[256];
                char        IlluminationType[256];
                char        CompositionType[256];

                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                File.GetField( "Index:d",               &Index                  );
                File.GetField( "MaterialName:s",        Material.Name           );
                File.GetField( "MaterialType:s",        MaterialType            );
                File.GetField( "IlluminationType:s",    IlluminationType        );
                File.GetField( "CompositionType:s",     CompositionType         );
                File.GetField( "nTexMaterials:d",      &Material.nTexMaterials  );
                File.GetField( "bDoubleSide:d",        &Material.bDoubleSide    );
                File.GetField( "nConstants:d",         &Material.nConstans      );

                //
                // TODO: Conver from string to material type
                //

                //
                // Convert the enum for the illum.
                //
                if     ( x_stricmp( IlluminationType, "DYNAMIC_I" ) == 0 )   Material.IllumType = ILLUM_TYPE_DYNAMIC_VERTEX_MONOCROM;
                else if( x_stricmp( IlluminationType, "LIGHTMAP"  ) == 0 )   Material.IllumType = ILLUM_TYPE_LIGHTMAP;                    
                else if( x_stricmp( IlluminationType, "DYNAMIC"   ) == 0 )   Material.IllumType = ILLUM_TYPE_DYNAMIC_VERTEX;
                else                                                         Material.IllumType = ILLUM_TYPE_NONE;

                //
                // Convert the enum for the illum.
                //
                if     ( x_stricmp( CompositionType, "OVERWRITE" ) == 0 ) Material.CompType = COMPOSITION_TYPE_OVERWRITE;
                else if( x_stricmp( CompositionType, "ADD" )       == 0 ) Material.CompType = COMPOSITION_TYPE_ADD;                    
                else if( x_stricmp( CompositionType, "SUB" )       == 0 ) Material.CompType = COMPOSITION_TYPE_SUB;
                else if( x_stricmp( CompositionType, "MUL" )       == 0 ) Material.CompType = COMPOSITION_TYPE_MUL;
                else                                                      Material.CompType = COMPOSITION_TYPE_NONE;
            }
        }
        else if( x_stricmp( File.GetHeaderName(), "MatTexture"   ) == 0 )
        {
            //ASSERT( File.GetHeaderCount() >= m_nMaterials );

            for( s32 i=0; i<m_nMaterials; i++ )
            for( s32 j=0; j<m_pMaterial[i].nTexMaterials; j++ )
            {
                s32             Index, SubIndex;
                char            UAddress[128];
                char            VAddress[128];
                char            WAddress[128];
                char            ChanelType[128];
                char            TextureType[128];
                char            UVWAnimType[128];
                char            FilterType[128];
                char            AnimTexPlayType[128];
                char            TintingType[128];
                f32             R,G,B;

                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                File.GetField( "iMaterial:d",  &Index           );
                File.GetField( "Index:d",      &SubIndex        );

                ASSERT( Index == i );
                ASSERT( SubIndex == j );

                tex_material&   TexMat = m_pMaterial[i].TexMaterial[j];

                File.GetField( "ChanelType:s",          ChanelType       );
                File.GetField( "iChanel:d",            &TexMat.iChanel   );
                File.GetField( "TextureType:s",         TextureType      );
                File.GetField( "iTexture:d",           &TexMat.iTexture  );

                File.GetField( "UVWAddressMode:sss",    UAddress, VAddress, WAddress );

                File.GetField( "UVWAnimMode:s",         UVWAnimType             );
                File.GetField( "nUVWAnimFrames:d",     &TexMat.nUVWAnimFrames   );
                File.GetField( "iUVWAnimID:d",         &TexMat.iUVWAnimID       );

                File.GetField( "Filtering:s",           FilterType              );
                File.GetField( "TexBias:f",            &TexMat.Bias             );

                File.GetField( "AnimTexPlayType:s",     AnimTexPlayType         );
                File.GetField( "AnimTexPlaySpeed:f",   &TexMat.TexAnimPlaySpeed );

                File.GetField( "TintingType:s",         TintingType             );
                File.GetField( "Color:fff",             &R, &G, &B               );
                TexMat.Color.SetfRGBA(R, G, B, 1);

                //
                // Conver chanel type
                //

                if     ( x_stricmp( ChanelType, "CONSTANT" )        == 0 ) TexMat.ChanelType = TEX_MATERIAL_CONSTANT;
                else if( x_stricmp( ChanelType, "VERTEX"   )        == 0 ) TexMat.ChanelType = TEX_MATERIAL_VERTEX;
                else if( x_stricmp( ChanelType, "TEXTURE"  )        == 0 ) TexMat.ChanelType = TEX_MATERIAL_TEXTURE;
                else if( x_stricmp( ChanelType, "TEXTURE_AUTOGEN" ) == 0 ) TexMat.ChanelType = TEX_MATERIAL_TEXTURE_AUTOGEN;
                else                                                       TexMat.ChanelType = TEX_MATERIAL_NONE;


                //
                // Conver texture type
                //
                     if( x_stricmp( TextureType, "BASE1"        ) == 0 )   TexMat.TexType = TEX_TYPE_BASE1;
                else if( x_stricmp( TextureType, "BASE2"        ) == 0 )   TexMat.TexType = TEX_TYPE_BASE2;
                else if( x_stricmp( TextureType, "BASE3"        ) == 0 )   TexMat.TexType = TEX_TYPE_BASE3;
                else if( x_stricmp( TextureType, "BASE4"        ) == 0 )   TexMat.TexType = TEX_TYPE_BASE4;
                else if( x_stricmp( TextureType, "BLEND_RGB"    ) == 0 )   TexMat.TexType = TEX_TYPE_BLEND_RGB;
                else if( x_stricmp( TextureType, "BLEND_RG"     ) == 0 )   TexMat.TexType = TEX_TYPE_BLEND_RG;
                else if( x_stricmp( TextureType, "BLEND_R"      ) == 0 )   TexMat.TexType = TEX_TYPE_BLEND_R;
                else if( x_stricmp( TextureType, "OPACITY_ALPHA") == 0 )   TexMat.TexType = TEX_TYPE_OPACITY_ALPHA;
                else if( x_stricmp( TextureType, "OPACITY_PUNCH") == 0 )   TexMat.TexType = TEX_TYPE_OPACITY_PUNCH;
                else if( x_stricmp( TextureType, "ENVMAP"       ) == 0 )   TexMat.TexType = TEX_TYPE_ENVIROMENT;
                else if( x_stricmp( TextureType, "BUMP"         ) == 0 )   TexMat.TexType = TEX_TYPE_BUMP;
                else if( x_stricmp( TextureType, "SELF_ILLUM"   ) == 0 )   TexMat.TexType = TEX_TYPE_SELF_ILLUM;
                else if( x_stricmp( TextureType, "SPECULAR"     ) == 0 )   TexMat.TexType = TEX_TYPE_SPECULAR;
                else if( x_stricmp( TextureType, "INTENSITY"    ) == 0 )   TexMat.TexType = TEX_TYPE_INTENSITY;
                else if( x_stricmp( TextureType, "NORMAL"       ) == 0 )   TexMat.TexType = TEX_TYPE_NORMAL;
                else if( x_stricmp( TextureType, "LIGHTMAP"     ) == 0 )   TexMat.TexType = TEX_TYPE_LIGHTMAP;
                else if( x_stricmp( TextureType, "ANISOTROPIC"  ) == 0 )   TexMat.TexType = TEX_TYPE_ANISOTROPIC;
                else                                                       TexMat.TexType = TEX_TYPE_NONE;

                //
                // Conver the U adress
                //
                if     ( x_stricmp( UAddress, "MIRROR" ) == 0 )     TexMat.UAddress = TEXTURE_MIRROR;
                else if( x_stricmp( UAddress, "WRAP"   ) == 0 )     TexMat.UAddress = TEXTURE_WRAP;
                else if( x_stricmp( UAddress, "CLAMP"  ) == 0 )     TexMat.UAddress = TEXTURE_CLAMP;
                else                                                TexMat.UAddress = TEXTURE_NONE;

                //
                // Conver the V adress
                //
                if     ( x_stricmp( VAddress, "MIRROR" ) == 0 )     TexMat.VAddress = TEXTURE_MIRROR;
                else if( x_stricmp( VAddress, "WRAP"   ) == 0 )     TexMat.VAddress = TEXTURE_WRAP;
                else if( x_stricmp( VAddress, "CLAMP"  ) == 0 )     TexMat.VAddress = TEXTURE_CLAMP;
                else                                                TexMat.VAddress = TEXTURE_NONE;

                //
                // Conver the V adress
                //
                if     ( x_stricmp( WAddress, "MIRROR" ) == 0 )     TexMat.WAddress = TEXTURE_MIRROR;
                else if( x_stricmp( WAddress, "WRAP"   ) == 0 )     TexMat.WAddress = TEXTURE_WRAP;
                else if( x_stricmp( WAddress, "CLAMP"  ) == 0 )     TexMat.WAddress = TEXTURE_CLAMP;
                else                                                TexMat.WAddress = TEXTURE_NONE;

                //
                // Convert the UVW animation mode
                //
                if     ( x_stricmp( UVWAnimType, "LOOP"      ) == 0 )  TexMat.UVWAnimType = UVWANIM_LOOP;
                else if( x_stricmp( UVWAnimType, "RELATIVE"  ) == 0 )  TexMat.UVWAnimType = UVWANIM_RELATIVE;
                else                                                   TexMat.UVWAnimType = UVWANIM_NONE;


                //
                // Texture Filter type
                //
                if     ( x_stricmp( FilterType, "BILINEAR"      ) == 0 )  TexMat.FilterType = FILTER_BILINEAR;
                else if( x_stricmp( FilterType, "TRILINEAR"     ) == 0 )  TexMat.FilterType = FILTER_TRILINEAR;
                else if( x_stricmp( FilterType, "ANISOTROPIC"   ) == 0 )  TexMat.FilterType = FILTER_ANISOTROPIC;
                else                                                      TexMat.FilterType = FILTER_POINT;

                //
                // Texture animation type
                //
                if     ( x_stricmp( AnimTexPlayType, "FORWARD"   ) == 0 )  TexMat.TexAnimType = TEXANIM_FORWARD;
                else if( x_stricmp( AnimTexPlayType, "REVERSE"   ) == 0 )  TexMat.TexAnimType = TEXANIM_REVERSE;
                else if( x_stricmp( AnimTexPlayType, "PING_PONG" ) == 0 )  TexMat.TexAnimType = TEXANIM_PING_PONG;
                else if( x_stricmp( AnimTexPlayType, "RANDOM"    ) == 0 )  TexMat.TexAnimType = TEXANIM_RANDOM;
                else                                                       TexMat.TexAnimType = TEXANIM_NONE;

                //
                // Texture Tinting type
                //
                if     ( x_stricmp( TintingType, "ADD"      ) == 0 )  TexMat.TintingType = TINTING_ADD;
                else if( x_stricmp( TintingType, "MUL"      ) == 0 )  TexMat.TintingType = TINTING_MUL;
                else if( x_stricmp( TintingType, "REPL"     ) == 0 )  TexMat.TintingType = TINTING_REPL;
                else                                                  TexMat.TintingType = TINTING_NONE;
            }            
        }
        else if( x_stricmp( File.GetHeaderName(), "MatConstants" ) == 0 )
        {
            ASSERT( File.GetHeaderCount() >= m_nMaterials );
    
            for( s32 i=0; i<m_nMaterials; i++ )
            for( s32 j=0; j<m_pMaterial[i].nConstans; j++ )
            {
                s32     Index, SubIndex;
                f32     Value;

                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                File.GetField( "iMaterial:d",  &Index           );
                File.GetField( "Index:d",      &SubIndex        );

                ASSERT( Index == i );
                ASSERT( SubIndex == j );

                File.GetField( "K:f", &Value );

                // Convert to color format
                m_pMaterial[i].K[j] = Value;
            }            
        }
       else if( x_stricmp( File.GetHeaderName(), "Mesh" ) == 0 )
        {
            xarray<char *> MeshNames;
            xarray<u32>    MeshCounts;

            m_nSubMeshs = File.GetHeaderCount();
    
            m_pSubMesh  = new sub_mesh[ m_nSubMeshs ];
            if( m_pSubMesh == NULL )
                x_throw( "Out of memory" );

            x_memset( m_pSubMesh, 0, sizeof(sub_mesh)*m_nSubMeshs );

            // Read each of the fields in the file
            for( s32 i=0; i<m_nSubMeshs; i++ )
            {
                sub_mesh& SubMesh = m_pSubMesh[i];
                s32       Index;

                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                File.GetField( "Index:d",  &Index );
                if( Index != i ) x_throw( "Wrong index from a mesh field" );

                File.GetField( "Name:s", &SubMesh.Name );                
            }
        }
        else File.SkipToNextHeader();
    }

    //
    // Did we rich the end of file successfully?
    //
    if( File.IsEOF() == FALSE )
        return FALSE;

    // Setup submesh use bools
    for( i=0; i<m_nSubMeshs; i++ )
    {
        for( j=0; j<256; j++ )
            m_pSubMesh[i].UsesMat[j] = pMatUsed[ i*256+j ];
    }

    x_free(pMatUsed);
    m_CachedFile = x_strdup(pFileName);
    return TRUE;
}

//=========================================================================

rawmaterial::tex_material* rawmaterial::material::GetTexMap( texture_type TexType )
{
    for( s32 i=0; i<nTexMaterials; i++ )
    {
        if( TexMaterial[i].TexType == TexType ) return &TexMaterial[i];
    }

    return NULL;
}

//=========================================================================

s32 rawmaterial::material::GetUVChanelCount( void )
{
    s32 Max = -1;
    for( s32 i=0; i<nTexMaterials; i++ )
    {
        if( TexMaterial[i].ChanelType == rawmaterial::TEX_MATERIAL_TEXTURE )
        {
            Max = MAX( TexMaterial[i].iChanel, Max );
        }
    }

    return (Max==-1)?0:Max+1;
}

//=========================================================================
