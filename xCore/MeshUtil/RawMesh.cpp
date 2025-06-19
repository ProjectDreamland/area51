#ifdef TARGET_PC
#include <crtdbg.h>
#endif

#include "RawMesh.hpp"
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

static xbool s_RawMeshRenamesDuplicates = FALSE;


void rawmesh::SetRenameDuplicates ( xbool b )
{
     s_RawMeshRenamesDuplicates = b;
}
//=========================================================================

rawmesh::rawmesh( void )
{
    x_memset( this, 0, sizeof(*this) );
}

//=========================================================================

rawmesh::~rawmesh( void )
{    
    Kill();
}

void rawmesh::Kill( void )
{
    if(m_pBone)     delete[]m_pBone;
    if(m_pVertex)   delete[]m_pVertex;
    if(m_pFacet)    delete[]m_pFacet;
    if(m_pTexture)  delete[]m_pTexture;
    if(m_pMaterial) delete[]m_pMaterial;
    if(m_pSubMesh)  delete[]m_pSubMesh;

    x_memset( this, 0, sizeof(*this) );
}

//=========================================================================

xbool rawmesh::Load( const char* pFileName )
{
    text_in *File;

    File = new text_in;

    Kill();

    //
    // Open the file
    //
    x_try;

    File->OpenFile( pFileName );

    x_catch_begin;

        delete File;
        File = NULL;

    x_catch_end_ret;

    while( File->ReadHeader() == TRUE )
    {
        if( x_stricmp( File->GetHeaderName(), "Hierarchy" ) == 0 )
        {
            // Allocate the bone count
            m_nBones = File->GetHeaderCount();
            m_pBone  = new bone[ m_nBones ];
            ASSERT( m_pBone );
            x_memset( m_pBone, 0, sizeof(bone)*m_nBones );

            // Read each of the fields in the file
            for( s32 i=0; i<File->GetHeaderCount(); i++ )
            {
                bone& Bone = m_pBone[i];
                s32   Index;

                if( File->ReadFields() == FALSE )                    
                {
                    delete File;
                    File = NULL;
                    return FALSE;
                }

                File->GetField( "Index:d",      &Index          );
                File->GetField( "Name:s",       Bone.Name       );
                File->GetField( "nChildren:d",  &Bone.nChildren );
                File->GetField( "iParent:d",    &Bone.iParent   );
                File->GetField( "Scale:fff",    &Bone.Scale.GetX(), &Bone.Scale.GetY(), &Bone.Scale.GetZ() );
                File->GetField( "Rotate:ffff",  &Bone.Rotation.X, &Bone.Rotation.Y, &Bone.Rotation.Z, &Bone.Rotation.W );
                File->GetField( "Pos:fff",      &Bone.Position.GetX(), &Bone.Position.GetY(), &Bone.Position.GetZ() );
            }
        }
        else if( x_stricmp( File->GetHeaderName(), "Vertices"     ) == 0 )
        {
            // Allocate the vertex count
            m_nVertices = File->GetHeaderCount();
            m_pVertex   = new vertex[ m_nVertices ];
            ASSERT( m_pVertex );
            x_memset( m_pVertex, 0, sizeof(vertex)*m_nVertices );

            // Read each of the fields in the file
            for( s32 i=0; i<File->GetHeaderCount(); i++ )
            {
                vertex& Vertex = m_pVertex[i];
                s32     Index;

                if( File->ReadFields() == FALSE )                    
                {
                    delete File;
                    File = NULL;
                    return FALSE;
                }

                File->GetField( "Index:d",      &Index          );
                File->GetField( "Pos:fff",      &Vertex.Position.GetX(), &Vertex.Position.GetY(), &Vertex.Position.GetZ() );
                File->GetField( "nNormals:d",   &Vertex.nNormals );
                File->GetField( "nUVSets:d",    &Vertex.nUVs     );
                File->GetField( "nColors:d",    &Vertex.nColors  );
                File->GetField( "nWeights:d",   &Vertex.nWeights );

                Vertex.nWeights = MIN(Vertex.nWeights,VERTEX_MAX_WEIGHT);
            }
        }
        else if( x_stricmp( File->GetHeaderName(), "Colors"     ) == 0 )
        {
            // Allocate the vertex count
            s32 nColors = File->GetHeaderCount();

            // Read each of the fields in the file
            for( s32 i=0; i<nColors; i++ )
            {
                s32     iVertex, SubIndex;
                f32     R,G,B,A;

                if( File->ReadFields() == FALSE )                    
                {
                    delete File;
                    File = NULL;
                    return FALSE;
                }

                File->GetField( "iVertex:d",    &iVertex          );
                vertex& Vertex = m_pVertex[iVertex];

                // Stevan: Added this to remove a warning.  
                //         Does this need to be here?
                (void)Vertex;

                File->GetField( "Index:d",      &SubIndex          );
                xcolor& C = m_pVertex[iVertex].Color[SubIndex];

                File->GetField( "Color:ffff",   &R, &G, &B, &A );
                C.SetfRGBA( R, G, B, A );
            }
        }
        else if( x_stricmp( File->GetHeaderName(), "Normals"      )  == 0 )
        {
            ASSERT( File->GetHeaderCount() >= m_nVertices );
    
            for( s32 i=0; i<m_nVertices; i++ )
            for( s32 j=0; j<m_pVertex[i].nNormals; j++ )
            {
                s32     Index, SubIndex;

                if( File->ReadFields() == FALSE )                    
                {
                    delete File;
                    File = NULL;
                    return FALSE;
                }

                File->GetField( "iVertex:d",    &Index           );
                File->GetField( "Index:d",      &SubIndex        );

                ASSERT( Index == i );
                ASSERT( SubIndex == j );

                File->GetField( "Normal:fff", &m_pVertex[i].Normal[j].GetX(), &m_pVertex[i].Normal[j].GetY(), &m_pVertex[i].Normal[j].GetZ() );
            }
        }
        else if( x_stricmp( File->GetHeaderName(), "UVSet"        ) == 0 )
        {
            //ASSERT( File->GetHeaderCount() >= m_nVertices );
    
            for( s32 i=0; i<m_nVertices; i++ )
            for( s32 j=0; j<m_pVertex[i].nUVs; j++ )
            {
                s32     Index, SubIndex;

                if( File->ReadFields() == FALSE )                    
                {
                    delete File;
                    File = NULL;
                    return FALSE;
                }

                File->GetField( "iVertex:d",    &Index           );
                File->GetField( "Index:d",      &SubIndex        );

                ASSERT( Index == i );
                ASSERT( SubIndex == j );

                File->GetField( "UV:ff", &m_pVertex[i].UV[j].X, &m_pVertex[i].UV[j].Y );
            }            
        }
        else if( x_stricmp( File->GetHeaderName(), "Skin"         ) == 0 )
        {
            s32 m_nSkin = File->GetHeaderCount();
            //ASSERT( File->GetHeaderCount() >= m_nVertices );
            if(m_nSkin > m_nVertices)
                m_nSkin = m_nVertices;
            for( s32 i=0; i<m_nSkin; i++ )
            for( s32 j=0; j<m_pVertex[i].nWeights; j++ )
            {
                s32     Index, SubIndex;

                if( File->ReadFields() == FALSE )                    
                {
                    delete File;
                    File = NULL;
                    return FALSE;
                }

                File->GetField( "iVertex:d",    &Index           );
                File->GetField( "Index:d",      &SubIndex        );

                ASSERT( Index == i );
                ASSERT( SubIndex == j );

                File->GetField( "iBone:d",  &m_pVertex[i].Weight[j].iBone );
                File->GetField( "Weight:f", &m_pVertex[i].Weight[j].Weight );
            }        
            for( s32 k=m_nSkin; k < m_nVertices; k++ )
            {
                m_pVertex[k].Weight[0].iBone  = 0;
                m_pVertex[k].Weight[0].Weight = 1.0f;
            }
        }        
        else if( x_stricmp( File->GetHeaderName(), "Polygons"     ) == 0 )
        {
            // Allocate the vertex count
            m_nFacets   = File->GetHeaderCount();
            m_pFacet    = new facet[ m_nFacets ];
            ASSERT( m_pFacet );
            x_memset( m_pFacet, 0, sizeof(facet)*m_nFacets );

            // Read each of the fields in the file
            for( s32 i=0; i<m_nFacets; i++ )
            {
                facet&  Facet = m_pFacet[i];
                s32     Index;

                if( File->ReadFields() == FALSE )                    
                {
                    delete File;
                    File = NULL;
                    return FALSE;
                }

                File->GetField( "Index:d",      &Index           );
                File->GetField( "iMesh:d",      &Facet.iMesh     );
                File->GetField( "nVerts:d",     &Facet.nVertices );
                File->GetField( "Normal:fff",   &Facet.Plane.Normal.GetX(), &Facet.Plane.Normal.GetY(), &Facet.Plane.Normal.GetZ() );
                File->GetField( "iMaterial:d",  &Facet.iMaterial );
            }            
        }
        else if( x_stricmp( File->GetHeaderName(), "FacetIndex"   )  == 0 )
        {
            ASSERT( File->GetHeaderCount() >= m_nFacets );
    
            for( s32 i=0; i<m_nFacets; i++ )
            for( s32 j=0; j<m_pFacet[i].nVertices; j++ )
            {
                s32     Index, SubIndex;

                if( File->ReadFields() == FALSE )                    
                {
                    delete File;
                    File = NULL;
                    return FALSE;
                }

                File->GetField( "iFacet:d",     &Index           );
                File->GetField( "Index:d",      &SubIndex        );

                ASSERT( Index == i );
                ASSERT( SubIndex == j );

                File->GetField( "iVertex:d",  &m_pFacet[i].iVertex[j] );
            }            
        }
        else if( x_stricmp( File->GetHeaderName(), "Textures"     ) == 0 )
        {
            // Allocate the vertex count
            m_nTextures = File->GetHeaderCount();
            m_pTexture  = new texture[ m_nTextures ];
            ASSERT( m_pTexture );
            x_memset( m_pTexture, 0, sizeof(texture)*m_nTextures );

            for( s32 i=0; i<m_nTextures; i++ )
            {
                texture&    Texture = m_pTexture[i];
                s32         Index;

                if( File->ReadFields() == FALSE )                    
                {
                    delete File;
                    File = NULL;
                    return FALSE;
                }

                File->GetField( "Index:d",      &Index            );
                File->GetField( "FileName:s",    Texture.FileName );
            }
        }
        else if( x_stricmp( File->GetHeaderName(), "Materials"    ) == 0 )
        {
            // Allocate the vertex count
            m_nMaterials = File->GetHeaderCount();
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

                if( File->ReadFields() == FALSE )                    
                {
                    delete File;
                    File = NULL;
                    return FALSE;
                }

                File->GetField( "Index:d",               &Index                  );
                File->GetField( "MaterialName:s",        Material.Name           );
                File->GetField( "MaterialType:s",        MaterialType            );
                File->GetField( "IlluminationType:s",    IlluminationType        );
                File->GetField( "CompositionType:s",     CompositionType         );
                File->GetField( "nTexMaterials:d",      &Material.nTexMaterials  );
                File->GetField( "bDoubleSide:d",        &Material.bDoubleSide    );
                File->GetField( "nConstants:d",         &Material.nConstans      );

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
        else if( x_stricmp( File->GetHeaderName(), "MatTexture"   ) == 0 )
        {
            //ASSERT( File->GetHeaderCount() >= m_nMaterials );

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

                if( File->ReadFields() == FALSE )                    
                {
                    delete File;
                    File = NULL;
                    return FALSE;
                }

                File->GetField( "iMaterial:d",  &Index           );
                File->GetField( "Index:d",      &SubIndex        );

                ASSERT( Index == i );
                ASSERT( SubIndex == j );

                tex_material&   TexMat = m_pMaterial[i].TexMaterial[j];

                File->GetField( "ChanelType:s",          ChanelType       );
                File->GetField( "iChanel:d",            &TexMat.iChanel   );
                File->GetField( "TextureType:s",         TextureType      );
                File->GetField( "iTexture:d",           &TexMat.iTexture  );

                File->GetField( "UVWAddressMode:sss",    UAddress, VAddress, WAddress );

                File->GetField( "UVWAnimMode:s",         UVWAnimType             );
                File->GetField( "nUVWAnimFrames:d",     &TexMat.nUVWAnimFrames   );
                File->GetField( "iUVWAnimID:d",         &TexMat.iUVWAnimID       );

                File->GetField( "Filtering:s",           FilterType              );
                File->GetField( "TexBias:f",            &TexMat.Bias             );

                File->GetField( "AnimTexPlayType:s",     AnimTexPlayType         );
                File->GetField( "AnimTexPlaySpeed:f",   &TexMat.TexAnimPlaySpeed );

                File->GetField( "TintingType:s",         TintingType             );
                File->GetField( "Color:fff",             &R, &G, &B               );
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
        else if( x_stricmp( File->GetHeaderName(), "MatConstants" ) == 0 )
        {
            ASSERT( File->GetHeaderCount() >= m_nMaterials );
    
            for( s32 i=0; i<m_nMaterials; i++ )
            for( s32 j=0; j<m_pMaterial[i].nConstans; j++ )
            {
                s32     Index, SubIndex;
                f32     Value;

                if( File->ReadFields() == FALSE )                    
                {
                    delete File;
                    File = NULL;
                    return FALSE;
                }

                File->GetField( "iMaterial:d",  &Index           );
                File->GetField( "Index:d",      &SubIndex        );

                ASSERT( Index == i );
                ASSERT( SubIndex == j );

                File->GetField( "K:f", &Value );

                // Convert to color format
                m_pMaterial[i].K[j] = Value;
            }            
        }
        else if( x_stricmp( File->GetHeaderName(), "UVAnimation" ) == 0 )
        {
            m_nUVWAnimFrames = File->GetHeaderCount();

            m_pUVWAnimFrame  = new uvanim_frame[ m_nUVWAnimFrames ];
            ASSERT( m_pUVWAnimFrame );
            x_memset( m_pUVWAnimFrame, 0, sizeof(uvanim_frame)*m_nUVWAnimFrames );

            // Read each of the fields in the file
            for( s32 i=0; i<m_nUVWAnimFrames; i++ )
            {
                uvanim_frame& Frame       = m_pUVWAnimFrame[i];

                if( File->ReadFields() == FALSE )                    
                {
                    delete File;
                    File = NULL;
                    return FALSE;
                }

                File->GetField( "iAnimID:d",  &Frame.ID         );
                File->GetField( "Index:d",    &Frame.iFrame     );
                File->GetField( "Scale:fff",  &Frame.Scale.GetX(), &Frame.Scale.GetY(), &Frame.Scale.GetZ() );
                File->GetField( "Rotate:fff", &Frame.Rotation.Roll, &Frame.Rotation.Pitch, &Frame.Rotation.Yaw );
                File->GetField( "Scale:fff",  &Frame.Position.GetX(), &Frame.Position.GetY(), &Frame.Position.GetZ() );
            }
        }
        else if( x_stricmp( File->GetHeaderName(), "Mesh" ) == 0 )
        {
            xarray<char *> MeshNames;
            xarray<u32>    MeshCounts;
            char           MeshNameTmp[512];

            m_nSubMeshs = File->GetHeaderCount();
    
            m_pSubMesh  = new sub_mesh[ m_nSubMeshs ];
            if( m_pSubMesh == NULL )
                x_throw( "Out of memory" );

            x_memset( m_pSubMesh, 0, sizeof(sub_mesh)*m_nSubMeshs );

            // Read each of the fields in the file
            for( s32 i=0; i<m_nSubMeshs; i++ )
            {
                sub_mesh& SubMesh = m_pSubMesh[i];
                s32       Index;

                if( File->ReadFields() == FALSE )                    
                {
                    delete File;
                    File = NULL;
                    return FALSE;
                }

                File->GetField( "Index:d",  &Index );
                if( Index != i ) x_throw( "Wrong index from a mesh field" );

                File->GetField( "Name:s", MeshNameTmp );                
                if(s_RawMeshRenamesDuplicates)
                {
                    xbool Match = FALSE;
                    for(s32 count = 0; (count < MeshNames.GetCount()) && !Match; count++)
                    {
                        if(x_strcmp(MeshNames.GetAt(count), MeshNameTmp) == 0)
                        {
                            MeshCounts.GetAt(count)++;
                            x_sprintf(SubMesh.Name, "%s%d", MeshNameTmp, MeshCounts.GetAt(count));
                            Match = TRUE;
                        }
                    }
                    if(!Match)
                    {
                        x_strcpy(SubMesh.Name, MeshNameTmp);
                        MeshNames.Append(SubMesh.Name);
                        MeshCounts.Append(0);
                    }
                }
                else
                {
                    File->GetField( "Name:s",  &SubMesh.Name );
                }
            

            }
        }
        else File->SkipToNextHeader();
    }

    //
    // Did we reach the end of file successfully?
    //
    if( File->IsEOF() == FALSE )
    {
        delete File;
        File = NULL;
        return FALSE;
    }

    delete File;
    File = NULL;
    return TRUE;
}

//=========================================================================

void rawmesh::Save( const char* pFileName )
{
    s32      nColors   = 0;
    s32      nNormals  = 0;
    s32      nUVs      = 0;
    s32      nWeights  = 0;
    s32      nFIndices = 0;
    s32      nMatTexs  = 0;
    s32      i;
    text_out File;
    ASSERT( pFileName );

    x_try;

    File.OpenFile( pFileName );

    x_catch_append( xfs( "Unable to open the file %s, for saving", pFileName) );


    File.AddHeader( "Hierarchy", m_nBones );
    for(  i=0; i<m_nBones; i++ )
    {
        bone& Bone = m_pBone[i];

        File.AddField( "Index:d",      i               );
        File.AddField( "Name:s",       Bone.Name       );
        File.AddField( "nChildren:d",  Bone.nChildren  );
        File.AddField( "iParent:d",    Bone.iParent    );
        File.AddField( "Scale:fff",    Bone.Scale.GetX(), Bone.Scale.GetY(), Bone.Scale.GetZ() );
        File.AddField( "Rotate:ffff",  Bone.Rotation.X, Bone.Rotation.Y, Bone.Rotation.Z, Bone.Rotation.W );
        File.AddField( "Pos:fff",      Bone.Position.GetX(), Bone.Position.GetY(), Bone.Position.GetZ() );
        File.AddEndLine();
    }

    File.AddHeader( "Vertices", m_nVertices );
    for(  i=0; i<m_nVertices; i++ )
    {
        vertex& Vertex = m_pVertex[i];

        File.AddField( "Index:d",      i               );
        File.AddField( "Pos:fff",      Vertex.Position.GetX(), Vertex.Position.GetY(), Vertex.Position.GetZ() );
        File.AddField( "nNormals:d",   Vertex.nNormals );
        File.AddField( "nUVSets:d",    Vertex.nUVs     );
        File.AddField( "nColors:d",    Vertex.nColors  );
        File.AddField( "nWeights:d",   Vertex.nWeights );
        File.AddEndLine();

        nColors  += Vertex.nColors;
        nNormals += Vertex.nNormals;
        nUVs     += Vertex.nUVs;
        nWeights += Vertex.nWeights;
    }

    File.AddHeader( "Colors", nColors );
    for( i=0; i<m_nVertices; i++ )
    {
        vertex& V = m_pVertex[i];
        for( s32 j=0; j<V.nColors; j++ )
        {
            xcolor& C = V.Color[j];
            f32     R = C.R * (1/255.0f);
            f32     G = C.G * (1/255.0f);
            f32     B = C.B * (1/255.0f);
            f32     A = C.A * (1/255.0f);

            File.AddField( "iVertex:d",  i         );
            File.AddField( "Index:d",    j          );
            File.AddField( "Color:ffff", R, G, B, A );
            File.AddEndLine();
        }
    }

    File.AddHeader( "Normals", nNormals );
    for( i=0; i<m_nVertices; i++ )
    {
        vertex& V = m_pVertex[i];
        for( s32 j=0; j<V.nNormals; j++ )
        {
            vector3 N = V.Normal[j];

            File.AddField( "iVertex:d",  i             );
            File.AddField( "Index:d",    j             );
            File.AddField( "Normal:fff", N.GetX(), N.GetY(), N.GetZ() );
            File.AddEndLine();
        }
    }

    File.AddHeader( "UVSet", nUVs );
    for( i=0; i<m_nVertices; i++ )
    {
        vertex& V = m_pVertex[i];
        for( s32 j=0; j<V.nUVs; j++ )
        {
            vector2 UV = V.UV[j];

            File.AddField( "iVertex:d", i           );
            File.AddField( "Index:d",   j           );
            File.AddField( "UV:ff",     UV.X, UV.Y  );
            File.AddEndLine();
        }
    }

    File.AddHeader( "Skin", nWeights );
    for( i=0; i<m_nVertices; i++ )
    {
        vertex& V = m_pVertex[i];
        for( s32 j=0; j<V.nWeights; j++ )
        {
            weight W = V.Weight[j];

            File.AddField( "iVertex:d", i        );
            File.AddField( "Index:d",   j        );
            File.AddField( "iBone:d",   W.iBone  );
            File.AddField( "Weight:f",  W.Weight );
            File.AddEndLine();
        }
    }

    File.AddHeader( "Polygons", m_nFacets );
    for( i=0; i<m_nFacets; i++ )
    {
        facet&  Facet = m_pFacet[i];

        File.AddField( "Index:d",      i               );
        File.AddField( "iMesh:d",      Facet.iMesh     );
        File.AddField( "nVerts:d",     Facet.nVertices );
        File.AddField( "Normal:fff",   Facet.Plane.Normal.GetX(), Facet.Plane.Normal.GetY(), Facet.Plane.Normal.GetZ() );
        File.AddField( "iMaterial:d",  Facet.iMaterial );
        File.AddEndLine();

        nFIndices += Facet.nVertices;
    }

    File.AddHeader( "FacetIndex", nFIndices );
    for( i=0; i<m_nFacets; i++ )
    {
        facet& F = m_pFacet[i];
        for( s32 j=0; j<F.nVertices; j++ )
        {
            s32 Index = F.iVertex[j];

            File.AddField( "iFacet:d",  i     );
            File.AddField( "Index:d",   j     );
            File.AddField( "iVertex:d", Index );
            File.AddEndLine();
        }
    }

    File.AddHeader( "Mesh", m_nSubMeshs );
    for( i=0; i<m_nSubMeshs; i++ )
    {
        sub_mesh& SubMesh = m_pSubMesh[i];

        File.AddField( "Index:d", i );
        File.AddField( "Name:s",  SubMesh.Name );
        File.AddEndLine();
    }

    File.AddHeader( "Textures", m_nTextures );
    for( i=0; i<m_nTextures; i++ )
    {
        texture& Texture = m_pTexture[i];

        File.AddField( "Index:d",    i                );
        File.AddField( "FileName:s", Texture.FileName );
        File.AddEndLine();
    }

    File.AddHeader( "Materials", m_nMaterials );
    for( i=0; i<m_nMaterials; i++ )
    {
        char        IlluminationType[256];
        char        CompositionType[256];
        material&   Material = m_pMaterial[i];

        //
        // Convert the enum for the illum.
        //
        switch( Material.IllumType )
        {
        case ILLUM_TYPE_DYNAMIC_VERTEX_MONOCROM: x_strcpy( IlluminationType, "DYNAMIC_I" ); break;
        case ILLUM_TYPE_LIGHTMAP:                x_strcpy( IlluminationType, "LIGHTMAP"  ); break;
        case ILLUM_TYPE_DYNAMIC_VERTEX:          x_strcpy( IlluminationType, "DYNAMIC"   ); break;
        default:
        case ILLUM_TYPE_NONE:                    x_strcpy( IlluminationType, "NONE"      ); break;
        }

        //
        // Convert the enum for the illum.
        //
        switch( Material.CompType )
        {
        case COMPOSITION_TYPE_OVERWRITE: x_strcpy( CompositionType, "OVERWRITE" ); break;
        case COMPOSITION_TYPE_ADD:       x_strcpy( CompositionType, "ADD" )      ; break;
        case COMPOSITION_TYPE_SUB:       x_strcpy( CompositionType, "SUB" )      ; break;
        case COMPOSITION_TYPE_MUL:       x_strcpy( CompositionType, "MUL" )      ; break;
        default:
        case COMPOSITION_TYPE_NONE:      x_strcpy( CompositionType, "NONE" )     ; break;
        }


        File.AddField( "Index:d",               i                       );
        File.AddField( "MaterialName:s",        Material.Name           );
        File.AddField( "MaterialType:s",        "DEFAULT"               );
        File.AddField( "IlluminationType:s",    IlluminationType        );
        File.AddField( "CompositionType:s",     CompositionType         );
        File.AddField( "nTexMaterials:d",       Material.nTexMaterials  );
        File.AddField( "bDoubleSide:d",         Material.bDoubleSide    );
        File.AddField( "nConstants:d",          Material.nConstans      );
        File.AddEndLine();

        nMatTexs += Material.nTexMaterials;
    }

    File.AddHeader( "MatTexture", nMatTexs );
    for( i=0; i<m_nMaterials; i++ )
    {
        material& M = m_pMaterial[i];
        for( s32 j=0; j<M.nTexMaterials; j++ )
        {
            tex_material& TexMat = M.TexMaterial[j];
            char    UAddress[128];
            char    VAddress[128];
            char    WAddress[128];
            char    ChanelType[128];
            char    TextureType[128];
            char    UVWAnimType[128];
            char    FilterType[128];
            char    AnimTexPlayType[128];
            char    TintingType[128];
            f32     R = TexMat.Color.R*(1/255.0f);
            f32     G = TexMat.Color.G*(1/255.0f);
            f32     B = TexMat.Color.B*(1/255.0f);

            //
            // Conver chanel type
            //
            switch( TexMat.ChanelType )
            {   
            case TEX_MATERIAL_CONSTANT:         x_strcpy( ChanelType, "CONSTANT" )       ; break; 
            case TEX_MATERIAL_VERTEX:           x_strcpy( ChanelType, "VERTEX"   )       ; break; 
            case TEX_MATERIAL_TEXTURE:          x_strcpy( ChanelType, "TEXTURE"  )       ; break; 
            case TEX_MATERIAL_TEXTURE_AUTOGEN:  x_strcpy( ChanelType, "TEXTURE_AUTOGEN" ); break; 
            default:
            case TEX_MATERIAL_NONE:             x_strcpy( ChanelType, "NONE" );            break; 
            }

            //
            // Convert texture type
            //
            switch( TexMat.TexType ) 
            {
            case TEX_TYPE_BASE1:            x_strcpy( TextureType, "BASE1"        ); break;
            case TEX_TYPE_BASE2:            x_strcpy( TextureType, "BASE2"        ); break;
            case TEX_TYPE_BASE3:            x_strcpy( TextureType, "BASE3"        ); break;
            case TEX_TYPE_BASE4:            x_strcpy( TextureType, "BASE4"        ); break;
            case TEX_TYPE_BLEND_RGB:        x_strcpy( TextureType, "BLEND_RGB"    ); break;
            case TEX_TYPE_BLEND_RG:         x_strcpy( TextureType, "BLEND_RG"     ); break;
            case TEX_TYPE_BLEND_R:          x_strcpy( TextureType, "BLEND_R"      ); break;
            case TEX_TYPE_OPACITY_ALPHA:    x_strcpy( TextureType, "OPACITY_ALPHA"); break;
            case TEX_TYPE_OPACITY_PUNCH:    x_strcpy( TextureType, "OPACITY_PUNCH"); break;
            case TEX_TYPE_ENVIROMENT:       x_strcpy( TextureType, "ENVMAP"       ); break;
            case TEX_TYPE_BUMP:             x_strcpy( TextureType, "BUMP"         ); break;
            case TEX_TYPE_SELF_ILLUM:       x_strcpy( TextureType, "SELF_ILLUM"   ); break;
            case TEX_TYPE_SPECULAR:         x_strcpy( TextureType, "SPECULAR"     ); break;
            case TEX_TYPE_INTENSITY:        x_strcpy( TextureType, "INTENSITY"    ); break;
            case TEX_TYPE_NORMAL:           x_strcpy( TextureType, "NORMAL"       ); break;
            case TEX_TYPE_LIGHTMAP:         x_strcpy( TextureType, "LIGHTMAP"     ); break;   
            case TEX_TYPE_ANISOTROPIC:      x_strcpy( TextureType, "ANISOTROPIC"  ); break;
            default:
            case TEX_TYPE_NONE:             x_strcpy( TextureType, "NONE"         ); break;
            }

            //
            // Conver the UV adress
            //
            switch( TexMat.UAddress )
            {
            case TEXTURE_MIRROR:    x_strcpy( UAddress, "MIRROR" ); break;
            case TEXTURE_WRAP:      x_strcpy( UAddress, "WRAP"   ); break;
            case TEXTURE_CLAMP:     x_strcpy( UAddress, "CLAMP"  ); break;
            default:
            case TEXTURE_NONE:      x_strcpy( UAddress, "NONE"   ); break;
            }

            switch( TexMat.VAddress )
            {
            case TEXTURE_MIRROR:    x_strcpy( VAddress, "MIRROR" ); break;
            case TEXTURE_WRAP:      x_strcpy( VAddress, "WRAP"   ); break;
            case TEXTURE_CLAMP:     x_strcpy( VAddress, "CLAMP"  ); break;
            default:
            case TEXTURE_NONE:      x_strcpy( VAddress, "NONE"   ); break;
            }

            switch( TexMat.WAddress )
            {
            case TEXTURE_MIRROR:    x_strcpy( WAddress, "MIRROR" ); break;
            case TEXTURE_WRAP:      x_strcpy( WAddress, "WRAP"   ); break;
            case TEXTURE_CLAMP:     x_strcpy( WAddress, "CLAMP"  ); break;
            default:
            case TEXTURE_NONE:      x_strcpy( WAddress, "NONE"   ); break;
            }

            //
            // Convert the UVW animation mode
            //
            switch( TexMat.UVWAnimType )
            {
            case UVWANIM_LOOP:      x_strcpy( UVWAnimType, "LOOP"      ); break;
            case UVWANIM_RELATIVE:  x_strcpy( UVWAnimType, "RELATIVE"  ); break;
            default:
            case UVWANIM_NONE:      x_strcpy( UVWAnimType, "NONE"      ); break;
            }

            //
            // Texture Filter type
            //
            switch( TexMat.FilterType )
            {
            case FILTER_BILINEAR:       x_strcpy( FilterType, "BILINEAR"      ); break;
            case FILTER_TRILINEAR:      x_strcpy( FilterType, "TRILINEAR"     ); break;
            case FILTER_ANISOTROPIC:    x_strcpy( FilterType, "ANISOTROPIC"   ); break;
            case FILTER_POINT:          x_strcpy( FilterType, "POINT"         ); break;
            }

            //
            // Texture animation type
            //
            switch( TexMat.TexAnimType )
            {
            case TEXANIM_FORWARD:   x_strcpy( AnimTexPlayType, "FORWARD"   ); break;
            case TEXANIM_REVERSE:   x_strcpy( AnimTexPlayType, "REVERSE"   ); break;
            case TEXANIM_PING_PONG: x_strcpy( AnimTexPlayType, "PING_PONG" ); break;
            case TEXANIM_RANDOM:    x_strcpy( AnimTexPlayType, "RANDOM"    ); break;
            default:
            case TEXANIM_NONE:      x_strcpy( AnimTexPlayType, "NONE"      ); break;
            }


            //
            // Texture Tinting type
            //
            switch( TexMat.TintingType )
            {
            case TINTING_ADD:   x_strcpy( TintingType, "ADD"      ); break;
            case TINTING_MUL:   x_strcpy( TintingType, "MUL"      ); break;
            case TINTING_REPL:  x_strcpy( TintingType, "REPL"     ); break;
            default:
            case TINTING_NONE:  x_strcpy( TintingType, "NONE"     ); break;
            }


            File.AddField( "iMaterial:d",           i                            );
            File.AddField( "Index:d",               j                            );
            File.AddField( "ChanelType:s",          ChanelType                   );
            File.AddField( "iChanel:d",             TexMat.iChanel               );
            File.AddField( "TextureType:s",         TextureType                  );
            File.AddField( "iTexture:d",            TexMat.iTexture              );
            File.AddField( "UVWAddressMode:sss",    UAddress, VAddress, WAddress );
            File.AddField( "UVWAnimMode:s",         UVWAnimType                  );
            File.AddField( "nUVWAnimFrames:d",      TexMat.nUVWAnimFrames        );
            File.AddField( "iUVWAnimID:d",          TexMat.iUVWAnimID            );
            File.AddField( "Filtering:s",           FilterType                   );
            File.AddField( "TexBias:f",             TexMat.Bias                  );
            File.AddField( "AnimTexPlayType:s",     AnimTexPlayType              );
            File.AddField( "AnimTexPlaySpeed:f",    TexMat.TexAnimPlaySpeed      );
            File.AddField( "TintingType:s",         TintingType                  );
            File.AddField( "Color:fff",             R, G, B                      );
            File.AddEndLine();
        }
    }

    File.AddHeader( "UVAnimation", m_nUVWAnimFrames );
    for( i=0; i<m_nUVWAnimFrames; i++ )
    {
        uvanim_frame& Frame       = m_pUVWAnimFrame[i];

        File.AddField( "iAnimID:d",  Frame.ID         );
        File.AddField( "Index:d",    Frame.iFrame     );
        File.AddField( "Scale:fff",  Frame.Scale.GetX(), Frame.Scale.GetY(), Frame.Scale.GetZ() );
        File.AddField( "Rotate:fff", Frame.Rotation.Roll, Frame.Rotation.Pitch, Frame.Rotation.Yaw );
        File.AddField( "Position:fff",  Frame.Position.GetX(), Frame.Position.GetY(), Frame.Position.GetZ() );
        File.AddEndLine();
    }

    // Done!
    File.CloseFile();
}

//=========================================================================

rawmesh::tex_material* rawmesh::material::GetTexMap( texture_type TexType )
{
    for( s32 i=0; i<nTexMaterials; i++ )
    {
        if( TexMaterial[i].TexType == TexType ) return &TexMaterial[i];
    }

    return NULL;
}

//=========================================================================

s32 rawmesh::material::GetUVChanelCount( void )
{
    s32 Max = -1;
    for( s32 i=0; i<nTexMaterials; i++ )
    {
        if( TexMaterial[i].ChanelType == rawmesh::TEX_MATERIAL_TEXTURE )
        {
            Max = MAX( TexMaterial[i].iChanel, Max );
        }
    }

    return (Max==-1)?0:Max+1;
}

//=========================================================================

bbox rawmesh::GetBBox( void )
{
    bbox BBox;
    BBox.Clear();
    for( s32 i=0; i<m_nVertices; i++ )
    {
        BBox += m_pVertex[i].Position;
    }
    return BBox;
}

//=========================================================================
static 
s32 CompareFacetByMaterial( const rawmesh::facet* pA, const rawmesh::facet* pB )
{
    if( pA->iMesh < pB->iMesh ) return -1;
    if( pA->iMesh > pB->iMesh ) return  1;

    if( pA->iMaterial < pB->iMaterial ) return -1;
    return pA->iMaterial > pB->iMaterial;
}

//=========================================================================

void rawmesh::SortFacetsByMaterial( void )
{
    x_qsort( m_pFacet, m_nFacets, sizeof(facet),
            (compare_fn*)CompareFacetByMaterial );
}

//=========================================================================
static rawmesh *g_pCompare;
s32 CompareFacetByMaterialAndBone( const rawmesh::facet* pA, const rawmesh::facet* pB )
{
    if( pA->iMesh < pB->iMesh ) return -1;
    if( pA->iMesh > pB->iMesh ) return  1;

    if( pA->iMaterial < pB->iMaterial ) return -1;

    if( pA->iMaterial > pB->iMaterial ) return 1;

    if( pA->iMaterial == pB->iMaterial )
    {
        if( g_pCompare->m_pVertex[pA->iVertex[0]].Weight[0].iBone >
            g_pCompare->m_pVertex[pB->iVertex[0]].Weight[0].iBone )
            return 1;
        if( g_pCompare->m_pVertex[pA->iVertex[0]].Weight[0].iBone <
            g_pCompare->m_pVertex[pB->iVertex[0]].Weight[0].iBone )
            return -1;
    }
    return 0;
}

//=========================================================================

void rawmesh::SortFacetsByMaterialAndBone( void )
{
    g_pCompare = this;
    x_qsort( m_pFacet, m_nFacets, sizeof(facet),
            (compare_fn*)CompareFacetByMaterialAndBone );
}

//=========================================================================

static xbool TempVCompare( rawmesh::vertex& A, rawmesh::vertex& B )
{
    s32 i;

    // Check position first
    {
        static const f32 PEpsilon = 0.001f; 
        vector3 T;
        T = A.Position - B.Position;
        f32 d = T.Dot( T );
        if( d > PEpsilon ) return FALSE;
    }
    
    if( A.nWeights != B.nWeights ) return FALSE;
    for( i=0; i<A.nWeights; i++ )
    {
        static const f32 WEpsilon = 0.001f;
        f32 d = (A.Weight[i].Weight*A.Weight[i].Weight) - (B.Weight[i].Weight*B.Weight[i].Weight);
        if( d > WEpsilon ) return FALSE;
        if( A.Weight[i].iBone != B.Weight[i].iBone ) return FALSE;
    }

    if( A.nNormals != B.nNormals ) return FALSE;
    for( i=0; i<A.nNormals; i++ )
    {
        static const f32 NEpsilon = 0.001f;
        vector3 T;
        T = B.Normal[i] - A.Normal[i];
        f32 d = T.Dot( T );
        if( d > NEpsilon ) return FALSE;
    }

    if( A.nUVs != B.nUVs ) return FALSE;
    for( i=0; i<A.nUVs; i++ )
    {
        static const f32 UVEpsilon = 0.001f;
        vector2 T;
        T = B.UV[i] - A.UV[i];
        f32 d = T.Dot( T );
        if( d > UVEpsilon ) return FALSE;
    }

    if( A.nColors != B.nColors ) return FALSE;
    for( i=0; i<A.nColors; i++ )
    {
        static const f32 CEpsilon = 3.0f;
        vector4 C1( A.Color[i].R, A.Color[i].G, A.Color[i].B, A.Color[i].A );
        vector4 C2( B.Color[i].R, B.Color[i].G, B.Color[i].B, B.Color[i].A );

        vector4 T = C1 - C2;
        f32 d = T.Dot( T );
        if( d > CEpsilon ) return FALSE;
    }

    return TRUE;
}

//=========================================================================

static xbool CompareFaces( rawmesh::facet& A, rawmesh::facet& B )
{
    s32 i;

    if( A.iMesh     != B.iMesh     ) return FALSE;
    if( A.nVertices != B.nVertices ) return FALSE;
    if( A.iMaterial != B.iMaterial ) return FALSE;

    for( i=0; i<A.nVertices; i++ )
    {
        if( A.iVertex[i] == B.iVertex[0] ) 
            break;
    }
    if( i == A.nVertices ) return FALSE;

    s32 Index = i;
    for( i=0; i<B.nVertices; i++ )
    {
        if( B.iVertex[i] != A.iVertex[(i+Index)%A.nVertices] ) 
            return FALSE;
    }

    return TRUE;
}

//=========================================================================

void rawmesh::CleanMesh( s32 iSubMesh /* = -1 */) // Remove this submesh
{
    s32 TotalFacetsRemoved    = 0;
    s32 TotalVerticesRemoved  = 0;
    s32 TotalMaterialsRemoved = 0;
    s32 TotalTexturesRemoved  = 0;

    RMESH_SANITY

    //
    // Sort weights from largest to smallest
    //
    {
        s32 i,j,k;
        for( i=0; i<m_nVertices; i++ )
        {
            for( j=0; j<m_pVertex[i].nWeights; j++ )
            {
                s32 BestW = j;
                for( k=j+1; k<m_pVertex[i].nWeights; k++ )
                {
                    if( m_pVertex[i].Weight[k].Weight > m_pVertex[i].Weight[BestW].Weight )
                        BestW = k;
                }

                weight TW = m_pVertex[i].Weight[j];
                m_pVertex[i].Weight[j] = m_pVertex[i].Weight[BestW];
                m_pVertex[i].Weight[BestW] = TW;
            }
        }
    }

    RMESH_SANITY
    //
    // Elliminate any digenerated facets
    //
    {
        s32 i;
        s32 nFacets=0;

        for( i=0; i<m_nFacets; i ++ )
        {
            vector3 Normal = v3_Cross( m_pVertex[m_pFacet[i].iVertex[1]].Position-m_pVertex[m_pFacet[i].iVertex[0]].Position,
                                       m_pVertex[m_pFacet[i].iVertex[2]].Position-m_pVertex[m_pFacet[i].iVertex[0]].Position );
            // Remove this facet if we're dumping out this submesh.
            
            if( ( iSubMesh != -1 && m_pFacet[i].iMesh == iSubMesh )
                || Normal.Length() < 0.00001f )
            {
                // Skip Facet
                //x_DebugMsg("Removing face %1d, (%1d,%1d,%1d)\n",i,m_pFacet[i].iVertex[0],m_pFacet[i].iVertex[1],m_pFacet[i].iVertex[2]);
            }
            else
            {
                m_pFacet[nFacets] = m_pFacet[i];
                nFacets++;
            }
        }

        // Set the new count
        TotalFacetsRemoved += m_nFacets - nFacets;
        m_nFacets = nFacets;

        // No facets left!
        if( m_nFacets <= 0 )
            x_throw( "RawMesh has not facets" );
    }

    RMESH_SANITY

    //
    // Elliminate any unuse vertices
    //
    {
        s32     i,j;
        s32*    pVRemap   = NULL;

        if( m_nVertices <= 0 )
            x_throw( "RawMesh has no vertices" );

        // begin the error section
        x_try;

        // Allocat the remap table
        pVRemap = new s32[ m_nVertices ];
        if( pVRemap == NULL )
            x_throw ("Out of memory" );

        // Fill the remap table 
        for( i=0; i<m_nVertices; i++) 
        {
            pVRemap[i]      = -1;
        }

        // Mark all the used vertices
        for( i=0; i<m_nFacets; i++ )
        for( s32 j=0; j<m_pFacet[i].nVertices; j++ )
        {
            if( m_pFacet[i].iVertex[j] < 0 || 
                m_pFacet[i].iVertex[j] >= m_nVertices )
                x_throw( xfs( "Found a facet that was indexing a vertex out of range! FaceID = %d VertexID = %d",
                i, m_pFacet[i].iVertex[j]) );

            pVRemap[ m_pFacet[i].iVertex[j] ] = -2;
        }

        // Create the remap table
        // and compact the vertices to the new location
        for( j=i=0; i<m_nVertices; i++) 
        {
            s32 Value  = pVRemap[i];

            pVRemap[i]       = j;
            m_pVertex[j]     = m_pVertex[i];

            if( Value == -2 ) j++;
        }

        // Set the final vertex count
        TotalVerticesRemoved += m_nVertices - j;
        m_nVertices = j;

        // Remap all the faces to point to the new location of verts
        for( i=0; i<m_nFacets; i++) 
        for( s32 j=0; j<m_pFacet[i].nVertices; j++ )
        {
            m_pFacet[i].iVertex[j] = pVRemap[ m_pFacet[i].iVertex[j] ];
        }

        // Clean up
        delete []pVRemap;

        // handle the errors
        x_catch_begin;
        
        if( pVRemap ) delete []pVRemap;

        x_catch_end_ret;
    }

    RMESH_SANITY

    //
    // Collapse vertices that are too close from each other and have 
    // the same properties.
    //
    {
        struct hash
        {
            s32 iVRemap;
            s32 iNext;
        };

        struct tempv
        {
            s32     RemapIndex;     // Which vertex Index it shold now use. 
            s32     Index;          // Inde to the original vertex
            s32     iNext;          // next node in the has 
        };

        if( m_nVertices <= 0 )
            x_throw( "RawMesh has not vertices" );

        s32         i;
        hash*       pHash     = NULL;
        tempv*      pTempV    = NULL;
        vertex*     pVertex   = NULL;
        s32         HashSize  = MAX( 20, m_nVertices*10 );
        f32         MaxX, MinX, Shift;

        // begin the error block
        x_try;

        // Allocate memory
        pHash   = new hash  [ HashSize ];
        pTempV  = new tempv [ m_nVertices ];

        if( pTempV == NULL || pHash == NULL )
            x_throw( "Out of memory" );

        // Initialize the hash with terminators
        for( i=0; i<HashSize; i++) 
        {
            pHash[i].iNext = -1;
        }

        // Fill the nodes for each of the dimensions
        MaxX = m_pVertex[0].Position.GetX();
        MinX = MaxX;
        for( i=0; i<m_nVertices; i++) 
        {
            pTempV[i].Index         =  i;
            pTempV[i].iNext         = -1;
            pTempV[i].RemapIndex    =  i;
           
            MaxX = MAX( MaxX, m_pVertex[i].Position.GetX() );
            MinX = MIN( MinX, m_pVertex[i].Position.GetX() );
        }

        // Hash all the vertices into the hash table
        Shift = HashSize/(MaxX-MinX+1);
        for( i=0; i<m_nVertices; i++) 
        {
            s32 OffSet = (s32)(( m_pVertex[i].Position.GetX() - MinX ) * Shift);

            ASSERT(OffSet >= 0 );
            ASSERT(OffSet < HashSize );

            pTempV[i].iNext  = pHash[ OffSet ].iNext;
            pHash[ OffSet ].iNext = i;
        }

        // Now do a seach for each vertex
        for( i=0; i<HashSize; i++ )
        {
            for( s32 k = pHash[i].iNext;  k != -1; k = pTempV[k].iNext )
            {
                s32 j;

                // This vertex has been remap
                if( pTempV[k].RemapIndex != pTempV[k].Index )
                    continue;

                // Seach in the current hash 
                for( j = pTempV[k].iNext; j != -1; j = pTempV[j].iNext )
                {                
                    // This vertex has been remap
                    if( pTempV[j].RemapIndex != pTempV[j].Index )
                        continue;

                    // If both vertices are close then remap vertex
                    if( TempVCompare( m_pVertex[ k ], m_pVertex[ j ] ))
                        pTempV[j].RemapIndex    = k;
                }

                // Searchin the hash on the left
                if( (i+1)< HashSize )
                {
                    for( j = pHash[i+1].iNext; j != -1; j = pTempV[j].iNext )
                    {                
                        // This vertex has been remap
                        if( pTempV[j].RemapIndex != pTempV[j].Index )
                            continue;

                        // If both vertices are close then remap vertex
                        if( TempVCompare( m_pVertex[ k ], m_pVertex[ j ] ))
                            pTempV[j].RemapIndex    = k;
                    }
                }
            }
        }

        RMESH_SANITY

        // Okay now we must collapse all the unuse vertices
        s32 nVerts=0;
        for( i=0; i<m_nVertices; i++ )
        {
            if( pTempV[i].RemapIndex == pTempV[i].Index )
            {
                pTempV[i].RemapIndex = nVerts;
                pTempV[i].Index      = -1;      // Mark as we have cranch it
                nVerts++;
            }
        }

        RMESH_SANITY

        // OKay now get all the facets and remap their indices
        for( i=0; i<m_nFacets; i++ )
        for( s32 j=0; j<m_pFacet[i].nVertices; j++ )
        {
            s32 iRemap = pTempV[ m_pFacet[i].iVertex[j] ].RemapIndex;

            if( pTempV[ m_pFacet[i].iVertex[j] ].Index == -1 )
            {
                m_pFacet[i].iVertex[j] = iRemap;
            }
            else
            {
                m_pFacet[i].iVertex[j] = pTempV[ iRemap ].RemapIndex;
            }
        }

        RMESH_SANITY

        // Now copy the vertices to their final location
        pVertex = new vertex[nVerts];
        if( pVertex == NULL ) 
            x_throw( "Out of memory" );

        for( i=0; i<m_nVertices; i++ )
        {
            s32 iRemap = pTempV[ i ].RemapIndex;

            if( pTempV[ i ].Index == -1 )
            {
                pVertex[ iRemap ] = m_pVertex[i];
            }
            else
            {
                pVertex[ pTempV[ iRemap ].RemapIndex ] = m_pVertex[i];
            }
        }

        RMESH_SANITY

        // Finally set the new count and 
        delete []m_pVertex;
        TotalVerticesRemoved += m_nVertices - nVerts;
        m_pVertex   = pVertex;
        m_nVertices = nVerts;

        // clean up
        delete []pHash;
        delete []pTempV;

        // Handle the errors
        x_catch_begin;

        if( pHash   ) delete []pHash;
        if( pTempV  ) delete []pTempV;
        if( pVertex ) delete []pVertex;

        x_catch_end_ret;

        RMESH_SANITY
    }

    RMESH_SANITY

    //
    // Nuke any facets that has the same vert indices and properties
    //
    {
        struct fref
        {
            s32 iFacet;
            s32 iNext;
        };

        s32     i;
        s32     nFacets;
        s32*    pVNode = NULL;
        fref*   pFRef  = NULL;
        s32     nRefs;
        s32     iRef;

        // Make sure that we have vertices
        if( m_nVertices <= 0 )
            x_throw( "RawMesh has not vertices" );

        // Protect our block
        x_try;

        // Get how many ref we should have
        nRefs = 0;
        for( i=0; i<m_nFacets; i++ )
        {
            nRefs += m_pFacet[i].nVertices;
        }

        // Allocate hash, and refs
        pVNode = new s32 [m_nVertices];
        pFRef  = new fref[ nRefs     ];
        if( pVNode == NULL || pFRef == NULL )
            x_throw( "Out of memory" );

        // Initalize the hash entries to null
        for( i=0; i<m_nVertices; i++ )
        {
            pVNode[i] = -1;            
        }

        // Insert all the face references into the hash
        iRef = 0;
        for(     i=0; i<m_nFacets; i++ )
        for( s32 j=0; j<m_pFacet[i].nVertices; j++ )
        {
            ASSERT( iRef < nRefs );
            pFRef[ iRef ].iFacet             = i;
            pFRef[ iRef ].iNext              = pVNode[ m_pFacet[i].iVertex[j] ];
            pVNode[ m_pFacet[i].iVertex[j] ] = iRef;
            iRef++;
        }

        // Find duplicate facets
        for( i=0; i<m_nVertices; i++ )
        for( s32 j = pVNode[i];      j != -1; j = pFRef[j].iNext )
        {
            facet& A = m_pFacet[ pFRef[j].iFacet ];

            // This facet has been removed
            if( A.nVertices < 0 )
                continue;

            for( s32 k = pFRef[j].iNext; k != -1; k = pFRef[k].iNext )
            {
                facet& B = m_pFacet[ pFRef[k].iFacet ];

                // This facet has been removed
                if( B.nVertices < 0 )
                    continue;

                // Check whether the two facets are the same
                if( CompareFaces( A, B ) )
                {
                    // Mark for removal
                    B.nVertices = 0;
                }
            }
        }

        // Remove any unwanted facets
        nFacets = 0;
        for( i=0; i<m_nFacets; i ++ )
        {
            if( m_pFacet[i].nVertices == 0 )
            {
                // Skip Facet
            }
            else
            {
                m_pFacet[nFacets] = m_pFacet[i];
                nFacets++;
            }
        }

        // Set the new count
        TotalFacetsRemoved += m_nFacets - nFacets;
        m_nFacets = nFacets;

        // Clean up
        delete []pVNode;
        delete []pFRef;

        // handle the error block
        x_catch_begin;

        if( pVNode ) delete []pVNode;
        if( pFRef  ) delete []pFRef;

        x_catch_end_ret;

        // No facets left!
        if( m_nFacets <= 0 )
            x_throw( "RawMesh has not facets" );
    }

    RMESH_SANITY

    //
    // Remove materials that are not been use
    //
    if( m_nMaterials > 0 )
    {
        s32     i;
        
        struct mat
        {
            xbool Used;
            s32   ReIndex;
        };

        mat*  pUsed = NULL;

        x_try;

        pUsed = new mat[ m_nMaterials ];
        if( pUsed == NULL )
            x_throw( "Out of memory" );

        // Assue that none of the materials are used
        for( i=0; i<m_nMaterials; i++ )
        {
            pUsed[i].Used       = FALSE;
            pUsed[i].ReIndex    = -1;
        }

        // Go throw the facets and mark all the used materials
        for( i=0; i<m_nFacets; i++ )
        {
            if( m_pFacet[i].iMaterial < 0 || 
                m_pFacet[i].iMaterial > m_nMaterials )  
                x_throw( xfs("Found a face which was using an unknow material FaceID=%d MaterialID =%d",i, m_pFacet[i].iMaterial) );

            pUsed[ m_pFacet[i].iMaterial ].Used = TRUE;
        }

        // Collapse all the material
        s32 nMaterials = 0;
        for( i=0; i<m_nMaterials; i++ )
        {
            if( pUsed[i].Used )
            {
                m_pMaterial[ nMaterials ] = m_pMaterial[ i ];
                pUsed[i].ReIndex          = nMaterials;
                nMaterials++;
            }
        }

        // Update the material indices for the facets
        for( i=0; i<m_nFacets; i++ )
        {
            if( pUsed[ m_pFacet[i].iMaterial ].ReIndex < 0 ||
                pUsed[ m_pFacet[i].iMaterial ].ReIndex >= nMaterials )
                x_throw( "Error while cleaning the materials in the rawmesh" );

            m_pFacet[i].iMaterial = pUsed[ m_pFacet[i].iMaterial ].ReIndex;
        }


        // Set the new material count
        TotalMaterialsRemoved += m_nMaterials - nMaterials;
        m_nMaterials = nMaterials;

        // clean up
        delete []pUsed;

        // handle any errors
        x_catch_begin;

        if( pUsed ) delete []pUsed;

        x_catch_end_ret;
    }

    RMESH_SANITY

    //
    // Remove unreferece textures
    //
    if( m_nTextures > 0 )
    {
        s32     i;

        struct tex
        {
            xbool Used;
            s32   ReIndex;
        };

        tex*  pUsed = FALSE;

        x_try;

        pUsed = new tex[ m_nTextures ];
        if( pUsed == NULL )
            x_throw( "Out of memory" );

        // Assue that none of the textures are used
        for( i=0; i<m_nTextures; i++ )
        {
            pUsed[i].Used    = FALSE;
            pUsed[i].ReIndex = -1;
        }
        
        // Go throw the materials and mark all the used textures
        for( i=0; i<m_nMaterials; i++ )
        for( s32 j=0; j<m_pMaterial[i].nTexMaterials; j++ )
        {
            tex_material& TexMaterial = m_pMaterial[i].TexMaterial[j];

            if( TexMaterial.TexType != TEX_TYPE_NONE )
            {
                if( TexMaterial.iTexture < 0 || 
                    TexMaterial.iTexture > m_nTextures )  
                    x_throw( xfs("Found a material which was using an unknow texmaterial\nMaterialID=%d TexMaterialID =%d TextureID=%d",
                    i, j, TexMaterial.iTexture ) );

                pUsed[ TexMaterial.iTexture ].Used = TRUE;
            }                
        }

        // Collapse all the textures
        s32 nTextures = 0;
        for( i=0; i<m_nTextures; i++ )
        {
            if( pUsed[i].Used )
            {
                m_pTexture[ nTextures ] = m_pTexture[ i ];
                pUsed[i].ReIndex        = nTextures;
                nTextures++;
            }
        }

        // Go throw all the materials and update their index
        for( i=0; i<m_nMaterials; i++ )
        for( s32 j=0; j<m_pMaterial[i].nTexMaterials; j++ )
        {
            tex_material& TexMaterial = m_pMaterial[i].TexMaterial[j];

            if( TexMaterial.TexType != TEX_TYPE_NONE )
            {
                if( pUsed[ TexMaterial.iTexture ].ReIndex < 0 || 
                    pUsed[ TexMaterial.iTexture ].ReIndex > nTextures )  
                    x_throw( xfs("Found a problem material\nMaterialID=%d TexMaterialID =%d TextureID=%d",
                    i, j, TexMaterial.iTexture ) );

                TexMaterial.iTexture = pUsed[ TexMaterial.iTexture ].ReIndex;
            }
        }


        // Set the new material count
        TotalTexturesRemoved = m_nTextures - nTextures;
        m_nTextures = nTextures;

        // clean up
        delete []pUsed;

        // handle any errors
        x_catch_begin;

        if( pUsed ) delete []pUsed;

        x_catch_end_ret;
    }

    RMESH_SANITY


    //
    // Rermove unwanted meshes
    //
    {
        s32 i;
        s32* pSMesh = new s32[ m_nSubMeshs ];
        if( pSMesh == NULL ) x_throw( "Out of memory" );

        x_memset( pSMesh, ~0, sizeof( s32 ) *m_nSubMeshs );

        for( i=0; i<m_nFacets; i++ )
        {
            pSMesh[ m_pFacet[i].iMesh ] = 1;
        }

        s32 nSubs = 0;
        for( i=0; i<m_nSubMeshs; i++ )
        {
            if( pSMesh[ i ] == 1 )
            {
                pSMesh[i] = nSubs;
                m_pSubMesh[nSubs++] = m_pSubMesh[i];
            }
            else
            {
                pSMesh[i] = -1;
            }
        }

        // Set the new count
        m_nSubMeshs = nSubs;

        for( i=0; i<m_nFacets; i++ )
        {
            ASSERT( pSMesh[ m_pFacet[i].iMesh ] >= 0 );
            m_pFacet[i].iMesh = pSMesh[ m_pFacet[i].iMesh ];
        }

        delete []pSMesh;
    }
    
}

//=========================================================================

void rawmesh::SanityCheck( void )
{

    //
    // Check that we have a valid number of materials
    //
    if( m_nMaterials < 0 ) 
        x_throw( "The Rawmesh it saying that it has a negative number of materials!");

    if( m_nMaterials > 1000 ) 
        x_throw( "The rawmesh has more than 1000 materials right now that is a sign of a problem" );
    
    if( m_nVertices < 0 )
        x_throw( "THe rawmesh has a negative number of vertices!" );

    if( m_nVertices > 100000000 )
        x_throw( "The rawmesh seems to have more that 100 million vertices that is consider bad" );

    if( m_nFacets < 0 )
        x_throw( "We have a negative number of facets" );

    if( m_nFacets > 100000000 )
        x_throw( "THe rawmesh has more thatn 100 million facets that is consider worng" );

    if( m_nBones < 0 ) 
        x_throw ("We have a negative count for bones" );
    
    if( m_nBones > 100000 )
        x_throw ("We have more than 100,000 bones this may be a problem" );

    if( m_nTextures < 0 )
        x_throw( "Found a negative number of textures" );

    if( m_nTextures > MATERIAL_MAX_TEXTURES )
        x_throw(xfs( "Found more than %d textures this is corrently not suported", MATERIAL_MAX_TEXTURES) );

    if( m_nSubMeshs < 0 )
        x_throw( "We have a negative number of meshes this is not good" );

    if( m_nSubMeshs > 100000000 )
        x_throw( "We have more than 100 million meshes this may be a problem" );

    if( m_nUVWAnimFrames < 0 )
        x_throw ("We have a negative number of animated UVs this is not good" );

    if( m_nUVWAnimFrames > 100000000 )
        x_throw ("We have more thatn 100 million number of anim UVs" );

    //
    // Check the good been of facets.
    //
    {
        s32 i;
        for( i=0; i<m_nFacets; i++ )
        {
            facet& Facet = m_pFacet[i];

            if( Facet.nVertices < 0 )
                x_throw( xfs( "I found a facet that had a negative number of indices to vertices Face#:%d", i) );

            if( Facet.nVertices < 3 )
                x_throw( xfs( "I found a facet with less than 3 vertices Face#:%d", i) );

            if( Facet.nVertices > 16 )
                x_throw( xfs( "I found a facet with more thatn 16 indices to vertices. The max is 16 this means memory overwun Face#:%d", i) );

            if( Facet.iMaterial < 0 )
                x_throw( xfs( "I found a facet with a negative material. That is not allow. Facet#:%d",i));

            if( Facet.iMaterial >= m_nMaterials ) 
                x_throw( xfs( "I found a facet that had an index to a material which is bad. Facet#:%d",i));

            if( Facet.iMesh < 0 )
                x_throw( xfs("I found a facet indexing to a negative offset for the meshID Facet#:%d",i));

            if( Facet.iMesh >= m_nSubMeshs )
                x_throw( xfs("I found a facet indexing to a not exiting mesh Facet#:%d",i));

            for( s32 j=0; j<Facet.nVertices; j++ )
            {
                if( Facet.iVertex[j] < 0 ) 
                    x_throw( xfs("I found a facet with a negative index to vertices. Facet#:%d",i));

                if( Facet.iVertex[j] >= m_nVertices ) 
                    x_throw( xfs("I found a facet with a index to a non-exiting vertex. Facet#:%d",i));
            }
        }
    }

    //
    // Check the good been of vertices.
    //
    {
        s32 i,j;
        for( i=0; i<m_nFacets; i++ )
        {
            facet& Facet = m_pFacet[i];
            for( j=0; j<Facet.nVertices; j++ )
            {
                vertex& V = m_pVertex[ Facet.iVertex[j] ];

                if( x_isvalid( V.Position.GetX() ) == FALSE || 
                    x_isvalid( V.Position.GetY() ) == FALSE ||
                    x_isvalid( V.Position.GetZ() ) == FALSE )
                    x_throw( xfs("Just got a infinete vertex position: Vertex#:%d",j));

                if( V.nWeights < 0 )
                    x_throw( xfs("We have a negative count of weights for one of the vertices. V#:%d",j));

                if( V.nWeights >= VERTEX_MAX_WEIGHT )
                    x_throw( xfs("Found a vertex with way too many weights. V#:%d",j));

                if( V.nWeights > m_nBones )
                    x_throw( xfs("Found a vertex pointing to a non-exiting bone: V#:d",j));

                if( V.nNormals < 0 ) 
                    x_throw( xfs("Found a vertex with a negative number of Normals. V#:%d",j));

                if( V.nNormals >= VERTEX_MAX_NORMAL )
                    x_throw( xfs("Found a vertex with way too many normals. V3:%d", j));

                if( V.nColors < 0 )
                    x_throw( xfs("I found a vertex with a negative number of colors. V#:%d", j));

                if( V.nColors >= VERTEX_MAX_COLOR )
                    x_throw( xfs("I found a vertex with way too many colors. V#:%d", j));

                if( V.nUVs < 0 )
                    x_throw( xfs("I found a vertex with a negative count for UVs. V#:%d",j));

                if( V.nUVs > VERTEX_MAX_UV )
                    x_throw( xfs("I found a vertex with way too many UVs. V#:%d", j));

                if( V.nUVs != m_pMaterial[ Facet.iMaterial ].GetUVChanelCount() )
                    x_throw( xfs("I found a vertex that didn't have the right number of UVs for the material been used. V#:%d, F#:%d",j,i));

            }
        }
    }
}

//=========================================================================

void rawmesh::CleanWeights( s32 MaxNumWeights, f32 MinWeightValue )
{
    s32 i,j,k;

    //
    // Sort weights from largest to smallest
    //
    for( i=0; i<m_nVertices; i++ )
    {
        for( j=0; j<m_pVertex[i].nWeights; j++ )
        {
            s32 BestW = j;
            for( k=j+1; k<m_pVertex[i].nWeights; k++ )
            {
                if( m_pVertex[i].Weight[k].Weight > m_pVertex[i].Weight[BestW].Weight )
                    BestW = k;
            }

            weight TW = m_pVertex[i].Weight[j];
            m_pVertex[i].Weight[j] = m_pVertex[i].Weight[BestW];
            m_pVertex[i].Weight[BestW] = TW;
        }
    }

    //
    // Cull any extra weights
    //
    for( i=0; i<m_nVertices; i++ )
    if( m_pVertex[i].nWeights > MaxNumWeights )
    {
        m_pVertex[i].nWeights = MaxNumWeights;

        // Normalize weights
        f32 TotalW=0.0f;
        for( j=0; j<m_pVertex[i].nWeights; j++ )
            TotalW += m_pVertex[i].Weight[j].Weight;
        for( j=0; j<m_pVertex[i].nWeights; j++ )
            m_pVertex[i].Weight[j].Weight /= TotalW;
    }

    //
    // Throw out all weights below MinWeightValue
    //
    for( i=0; i<m_nVertices; i++ )
    {
        // Keep weights above MinWeight
        s32 nWeights = 0;
        for( j=0; j<m_pVertex[i].nWeights; j++ )
        {
            if( m_pVertex[i].Weight[j].Weight >= MinWeightValue )
            {
                m_pVertex[i].Weight[nWeights] = m_pVertex[i].Weight[j];
                nWeights++;
            }
        }
        nWeights = MAX(1,nWeights);
        m_pVertex[i].nWeights = nWeights;

        // Normalize weights
        f32 TotalW=0.0f;
        for( j=0; j<m_pVertex[i].nWeights; j++ )
            TotalW += m_pVertex[i].Weight[j].Weight;
        for( j=0; j<m_pVertex[i].nWeights; j++ )
            m_pVertex[i].Weight[j].Weight /= TotalW;
    }
}

//=========================================================================

void rawmesh::CollapseMeshes( const char* pMeshName )
{
    for( s32 i=0; i<m_nFacets; i++ )
    {
        m_pFacet[i].iMesh = 0;
    }

    m_nSubMeshs = 1;
    x_strcpy( m_pSubMesh[0].Name, pMeshName );
}

//=========================================================================

void rawmesh::ComputeMeshBBox( s32 iMesh, bbox& BBox )
{
    s32 i,j;

    BBox.Clear();

    for( i=0; i<m_nFacets; i++ )
    {
        if( m_pFacet[i].iMesh == iMesh )
        {
            for( j=0; j<m_pFacet[i].nVertices; j++ )
                BBox += m_pVertex[m_pFacet[i].iVertex[j]].Position;
        }
    }
}

//=========================================================================

xbool rawmesh::IsolateSubmesh( s32 iSubMesh, rawmesh& NewMesh,
                               xbool RemoveFromRawMesh /* = FALSE */)
{
    NewMesh.Kill();

    if (iSubMesh < 0)
        return FALSE;
    if (iSubMesh >= m_nSubMeshs)
        return FALSE;

    //
    //  Make straight copies of data we don't affect
    //
    //  BONES
    //
    if (m_nBones > 0)
    {
        NewMesh.m_pBone = new rawmesh::bone[ m_nBones ];
        ASSERT(NewMesh.m_pBone);
        NewMesh.m_nBones = m_nBones;
        x_memcpy( NewMesh.m_pBone, m_pBone, sizeof(rawmesh::bone)*NewMesh.m_nBones );
    }
    else
    {
        NewMesh.m_pBone  = NULL;
        NewMesh.m_nBones = 0;
    }

    //
    //  UV Anim Frames
    //
    if (m_nUVWAnimFrames > 0)
    {
        NewMesh.m_pUVWAnimFrame = new rawmesh::uvanim_frame[ m_nUVWAnimFrames ];
        ASSERT(NewMesh.m_pUVWAnimFrame);
        NewMesh.m_nUVWAnimFrames = m_nUVWAnimFrames;
        x_memcpy( NewMesh.m_pUVWAnimFrame, m_pUVWAnimFrame, sizeof(rawmesh::uvanim_frame)*NewMesh.m_nTextures );
    }
    else
    {
        NewMesh.m_pUVWAnimFrame  = NULL;
        NewMesh.m_nUVWAnimFrames = 0;
    }

    //
    //  Submeshes
    //
    ASSERT( m_nSubMeshs > 0 );
    
    NewMesh.m_pSubMesh = new rawmesh::sub_mesh[ 1 ];
    ASSERT(NewMesh.m_pSubMesh);
    NewMesh.m_nSubMeshs = 1;
    x_memcpy( NewMesh.m_pSubMesh, &(m_pSubMesh[iSubMesh]), sizeof(rawmesh::sub_mesh) );
    
    
    //
    // Materials:
    //
    if (m_nMaterials > 0)
    {
        NewMesh.m_pMaterial = new rawmesh::material[ m_nMaterials ];
        ASSERT(NewMesh.m_pMaterial);
        NewMesh.m_nMaterials = m_nMaterials;
        x_memcpy( NewMesh.m_pMaterial, m_pMaterial, sizeof(rawmesh::material)*NewMesh.m_nMaterials);
    }
    else
    {
        NewMesh.m_pMaterial  = NULL;
        NewMesh.m_nMaterials = 0;
    }

    //
    //  Textures
    //
    //  The # of textures is the # of original textures + the # of lightmaps 
    //  Allocate storage, and fill out the filenames
    //
    if (m_nTextures > 0)
    {
        NewMesh.m_nTextures = m_nTextures;

        NewMesh.m_pTexture = new rawmesh::texture[ NewMesh.m_nTextures ];
        ASSERT(NewMesh.m_pTexture);
        
        if (m_nTextures > 0)
            x_memcpy( NewMesh.m_pTexture, m_pTexture, sizeof(rawmesh::texture)*m_nTextures );        
    }
    else
    {
        NewMesh.m_pTexture  = NULL;
        NewMesh.m_nTextures = 0;
    }

    // Verts and Facets:
    //
    //  Each facet will generate 3 unique verts, which will be consolidated later.
    //  These are done before the material construction, but the facet iMaterial
    //  member will be touched up in the next step.
    //
    s32     i;
    s32     nFacets = 0;

    for (i=0;i<m_nFacets;i++)
    {
        if (m_pFacet[i].iMesh == iSubMesh)
            nFacets++;
    }


    NewMesh.m_nVertices = nFacets * 3;
    NewMesh.m_pVertex = new rawmesh::vertex[ NewMesh.m_nVertices ];
    ASSERT(NewMesh.m_pVertex);

    NewMesh.m_nFacets = nFacets;
    NewMesh.m_pFacet = new rawmesh::facet[ NewMesh.m_nFacets ];
    ASSERT(NewMesh.m_pFacet );

    rawmesh::vertex*    pVert     = NewMesh.m_pVertex;
    s32                 iVert     = 0;
    rawmesh::facet*     pFacet    = NewMesh.m_pFacet;
    
    for (i=0;i<m_nFacets;i++)
    {
        if (m_pFacet[i].iMesh == iSubMesh)
        {
            *pFacet = m_pFacet[i];

            pFacet->iVertex[0] = iVert+0;
            pFacet->iVertex[1] = iVert+1;
            pFacet->iVertex[2] = iVert+2;
            iVert+=3;       

            pFacet->iMesh = 0;
        
            *pVert = m_pVertex[ m_pFacet[i].iVertex[0] ];
            pVert++;

            *pVert = m_pVertex[ m_pFacet[i].iVertex[1] ];
            pVert++;

            *pVert = m_pVertex[ m_pFacet[i].iVertex[2] ];
            pVert++;

            pFacet++;
        }
    }

    //
    // Clear the mesh
    //
    NewMesh.CleanMesh();
    if (RemoveFromRawMesh)
        CleanMesh(iSubMesh);

    return TRUE;
}

//=========================================================================

xbool rawmesh::IsolateSubmesh( const char* pMeshName, rawmesh& NewMesh )
{
    s32  i;

    if (pMeshName == NULL)
        return FALSE;

    for ( i=0;i<m_nSubMeshs;i++ )
    {
        if ( x_strcmp( m_pSubMesh[i].Name, pMeshName ) == 0 )
        {
            return IsolateSubmesh( i, NewMesh );
        }
    }

    return FALSE;
}

//=========================================================================

xbool rawmesh::IsBoneUsed( s32 iBone )
{
    s32 i,j;

    for( i=0; i<m_nVertices; i++ )
    {
        for( j=0; j<m_pVertex[i].nWeights; j++ )
        if( m_pVertex[i].Weight[j].iBone == iBone )
            return TRUE;
    }

    return FALSE;
}

//=========================================================================

void rawmesh::CollapseNormals( radian ThresholdAngle )
{
    f32     TargetAngle = x_cos( ThresholdAngle );

    struct hash
    {
        s32 iNext;
    };

    struct tempv
    {
        vector3 NewNormal;
        s32     Index;          // Inde to the original vertex
        s32     iNext;          // next node in the has 
    };

    if( m_nVertices <= 0 )
        x_throw( "RawMesh has no vertices" );

    s32         i;
    hash*       pHash     = NULL;
    tempv*      pTempV    = NULL;
    s32         HashSize  = MAX( 20, m_nVertices*10 );
    f32         MaxX, MinX, Shift;

    // begin the error block
    x_try;

    // Allocate memory
    pHash   = new hash  [ HashSize ];
    pTempV  = new tempv [ m_nVertices ];

    if( pTempV == NULL || pHash == NULL )
        x_throw( "Out of memory" );

    // Initialize the hash with terminators
    for( i=0; i<HashSize; i++) 
    {
        pHash[i].iNext = -1;
    }

    // Fill the nodes for each of the dimensions
    MaxX = m_pVertex[0].Position.GetX();
    MinX = MaxX;
    for( i=0; i<m_nVertices; i++) 
    {
        pTempV[i].Index         =  i;
        pTempV[i].iNext         = -1;
        pTempV[i].NewNormal.Set(0,0,0);
       
        MaxX = MAX( MaxX, m_pVertex[i].Position.GetX() );
        MinX = MIN( MinX, m_pVertex[i].Position.GetX() );
    }

    // Hash all the vertices into the hash table
    Shift = HashSize/(MaxX-MinX+1);
    for( i=0; i<m_nVertices; i++) 
    {
        s32 OffSet = (s32)(( m_pVertex[i].Position.GetX() - MinX ) * Shift);

        ASSERT(OffSet >= 0 );
        ASSERT(OffSet < HashSize );

        pTempV[i].iNext  = pHash[ OffSet ].iNext;
        pHash[ OffSet ].iNext = i;
    }

    // Loop through all hash entries, and begin the collapse process
    for( i=0; i<HashSize; i++ )
    {
        for( s32 k = pHash[i].iNext; k != -1; k = pTempV[k].iNext )
        {
            s32         j;
            vector3     SrcN = m_pVertex[ pTempV[k].Index ].Normal[0];
            vector3     SrcP = m_pVertex[ pTempV[k].Index ].Position;
            vector3     ResultN = SrcN;
            
            for( j = pHash[i].iNext; j != -1; j = pTempV[j].iNext )
            {                
                if (j==k)
                    continue;
                
                vector3 D = m_pVertex[ pTempV[j].Index ].Position - SrcP;

                //  If the verts don't share the same position, continue
                if (D.Length() > 0.001f)
                    continue;

                //
                //  Check the normals to see if the 2nd vert's norm is within the
                //  allowable threshold
                //
                vector3 N = m_pVertex[ pTempV[j].Index ].Normal[0];

                f32     T = SrcN.Dot( N );
                if ( T >= TargetAngle )
                {
                    // Merge in this normal
                    ResultN += N;
                }
            }

            // Search in the hash on the right
            if( (i+1)< HashSize )
            {
                for( j = pHash[i+1].iNext; j != -1; j = pTempV[j].iNext )
                {                
                    vector3 D = m_pVertex[ pTempV[j].Index ].Position - SrcP;

                    //  If the verts don't share the same position, continue
                    if (D.Length() > 0.001f)
                        continue;

                    //
                    //  Check the normals to see if the 2nd vert's norm is within the
                    //  allowable threshold
                    //
                    vector3 N = m_pVertex[ pTempV[j].Index ].Normal[0];

                    f32     T = SrcN.Dot( N );
                    if ( T >= TargetAngle )
                    {
                        // Merge in this normal
                        ResultN += N;
                    }
                }
            }
            
            // Renormalize the resultant normal
            ResultN.Normalize();

            pTempV[k].NewNormal = ResultN;
        }
    }

    for (i=0;i<m_nVertices;i++)
    {
        m_pVertex[ pTempV[i].Index ].Normal[0] = pTempV[i].NewNormal;
    }

    if( pHash   ) delete []pHash;
    if( pTempV  ) delete []pTempV;
    pHash = NULL;
    pTempV = NULL;

    // Handle the errors
    x_catch_begin;

    if( pHash   ) delete []pHash;
    if( pTempV  ) delete []pTempV;
    pHash = NULL;
    pTempV = NULL;

    x_catch_end_ret;

    if( pHash   ) delete []pHash;
    if( pTempV  ) delete []pTempV;
    pHash = NULL;
    pTempV = NULL;
}

//=========================================================================

/*void rawmesh::DeleteDummyBones( void )
{
    s32 iBone = 0;
    while(iBone < m_nBones)
    {
        xstring S(m_pBone[iBone].Name);
        S.MakeLower();
        if(S.Find("dummy") != -1)
        {
            //Check if it is the root.  If it is, make sure it is not the only root; that is,
            // we can only delete a root bone if it only has one child (because then its child
            // can become the new root)
            if(m_pBone[iBone].iParent == -1)
            {
                s32 nChildren = 0;
                for(s32 count = 0; count < m_nBones; count++)
                {
                    if(m_pBone[count].iParent == iBone)
                        nChildren++;
                }
                if(nChildren == 1)
                {
                    x_DebugMsg("Bone is root, but can be removed: '%s'\n", m_pBone[iBone].Name);                
                    DeleteBone(iBone);
                    iBone = 0;
                }
                else
                {
                    x_DebugMsg("Bone is sole remaining root: '%s'\n", m_pBone[iBone].Name);
                    iBone++;
                }
            }
            else
            {
                DeleteBone(iBone);
                iBone = 0;
            }
        }
        else
        {
            iBone++;
        }
    }
    for(iBone = 0; iBone < m_nBones; iBone++)
    {
        x_DebugMsg("Bone Index: %3d Parent: %3d Name: '%s'\n", iBone, m_pBone[iBone].iParent, m_pBone[iBone].Name);
    }
}*/
    
//=========================================================================
void rawmesh::DeleteBone          ( const char* pBoneName )
{
    s32 iBone = this->GetBoneIDFromName(pBoneName);
    if(iBone != -1)
        DeleteBone(iBone);
    return;
}

//=========================================================================

void rawmesh::DeleteBone( s32 iBone )
{
    //x_DebugMsg("MESH: Deleting bone: '%s'\n", m_pBone[iBone].Name);
    s32 i,j;
    ASSERT( m_nBones > 1 );

    //
    // Allocate new bones and frames
    //
    s32 nNewBones = m_nBones-1;
    bone* pNewBone = new bone[ nNewBones ];
    //frame* pNewFrame = new frame[ nNewBones * m_nFrames ];
    //ASSERT( pNewBone && pNewFrame );
    ASSERT( pNewBone );

    //
    // Build new hierarchy
    //
    {
        // Copy over remaining bones
        j=0;
        for( i=0; i<m_nBones; i++ )
        if( i != iBone )
        {
            pNewBone[j] = m_pBone[i];
            j++;
        }

        // Patch children of bone
        for( i=0; i<nNewBones; i++ )
        if( pNewBone[i].iParent == iBone )
        {
            pNewBone[i].iParent = m_pBone[iBone].iParent;
        }

        // Patch references to any bone > iBone
        for( i=0; i<nNewBones; i++ )
        if( pNewBone[i].iParent > iBone )
        {
            pNewBone[i].iParent--;
        }
    }

    //
    // Loop through frames of animation
    //
    /*matrix4* pL2W = (matrix4*)x_malloc(sizeof(matrix4)*m_nBones);
    for( i=0; i<m_nFrames; i++ )
    {
        // Compute matrices for current animation.
        for( j=0; j<m_nBones; j++ )
        {
            frame* pF = &m_pFrame[ i*m_nBones+j ];

            pL2W[j].Setup( pF->Scale, pF->Rotation, pF->Translation );

            // Concatenate with parent
            if( m_pBone[j].iParent != -1 )
            {
                pL2W[j] = pL2W[m_pBone[j].iParent] * pL2W[j];
            }
        }

        // Apply original bind matrices
        for( j=0; j<m_nBones; j++ )
        {
            pL2W[j] = pL2W[j] * m_pBone[j].BindMatrixInv;
        }

        // Shift bones down to align with NewBones
        for( j=iBone+1; j<m_nBones; j++ )
            pL2W[j-1] = pL2W[j];


        // Remove bind translation and scale matrices
        for( j=0; j<nNewBones; j++ )
        {
            pL2W[j] = pL2W[j] * pNewBone[j].BindMatrix;
        }

        // Convert back to local space transform
        for( j=nNewBones-1; j>0; j-- )
        if( pNewBone[j].iParent != -1 )
        {
            matrix4 PM = pL2W[ pNewBone[j].iParent ];
            PM.InvertSRT();
            pL2W[j] = PM * pL2W[j];
        }

        // Pull out rotation scale and translation
        for( j=0; j<nNewBones; j++ )
        {
            frame* pF       = &pNewFrame[i*nNewBones + j];
            pL2W[j].DecomposeSRT( pF->Scale, pF->Rotation, pF->Translation );
        }
    }
    x_free(pL2W);*/

    // free current allocations
    delete[] m_pBone;
    //delete[] m_pFrame;

    m_nBones = nNewBones;
    m_pBone = pNewBone;
    //m_pFrame = pNewFrame;
    //x_DebugMsg("MESH: After delete, m_nBones = %d\n", m_nBones);
    
}

//=========================================================================

s32 rawmesh::GetBoneIDFromName( const char* pBoneName ) const
{
    s32 i;
    for( i=0; i<m_nBones; i++ )
    if( x_stricmp(pBoneName,m_pBone[i].Name) == 0 )
        return i;
    return -1;
}

//=========================================================================

void rawmesh::ApplyNewSkeleton    ( rawanim& Skel )
{
    s32 i,j;

    // Transform all verts into local space of current skeleton
    for(i = 0; i < m_nVertices; i++)
    {
        s32 iBone = m_pVertex[i].Weight[0].iBone;
        vector3 P = m_pVertex[i].Position;
        matrix4 BM;
        BM.Identity();
        BM.Scale( m_pBone[iBone].Scale );
        BM.Rotate( m_pBone[iBone].Rotation );
        BM.Translate( m_pBone[iBone].Position );
        BM.InvertSRT();
        m_pVertex[i].Position = BM * P;
    }

    // Remap bone indices
    //x_DebugMsg("**************************************\n");
    //x_DebugMsg("**************************************\n");
    //x_DebugMsg("**************************************\n");
    //x_DebugMsg("**************************************\n");
    for(i = 0; i < m_nVertices; i++)
    {
        rawmesh::vertex* pVertex = &m_pVertex[i];
        for(s32 iWeight = 0; iWeight < pVertex->nWeights; iWeight++)
        {
            rawmesh::weight* pWeight = &pVertex->Weight[iWeight];

            s32 oldBoneId = pWeight->iBone;
            //const char* oldBoneName = m_pBone[oldBoneId].Name;

            s32 newBoneId = -1;
            s32 curBoneId = oldBoneId;
            while( (newBoneId==-1) && (curBoneId!=-1) )
            {
                const char* curBoneName = m_pBone[curBoneId].Name;

                // Look for matching bone in Skel
                for( j=0; j<Skel.m_nBones; j++ )
                if( x_stricmp( curBoneName, Skel.m_pBone[j].Name ) == 0 )
                    break;

                if( j!=Skel.m_nBones )
                {
                    newBoneId = j;
                    break;
                }

                // Move up hierarchy to parent
                curBoneId = m_pBone[curBoneId].iParent;
            }

            if( newBoneId == -1 )
                newBoneId = 0;

            //ASSERT( m_pBone[pWeight->iBone].Position.Difference( Skel.GetBone(newBoneId).BindTranslation ) < 0.0001f );
            //ASSERT( m_pBone[pWeight->iBone].Rotation.Difference( Skel.GetBone(newBoneId).BindRotation ) < 0.0001f );
            //x_DebugMsg("For old bone of %d, found new bone %d\n", pWeight->iBone, newBoneId);
            pWeight->iBone = newBoneId;
            //x_DebugMsg("%s -> %s\n",m_pBone[oldBoneId].Name,Skel.m_pBone[newBoneId].Name);
        }
    }
    //x_DebugMsg("**************************************\n");
    //x_DebugMsg("**************************************\n");
    //x_DebugMsg("**************************************\n");
    //x_DebugMsg("**************************************\n");

    // Copy new bone information in
    m_nBones = Skel.m_nBones;
    if(m_pBone)
        delete[] m_pBone;
    m_pBone = new rawmesh::bone[m_nBones];
    for(s32 count = 0; count < m_nBones; count++)
    {
        //m_pBone[count] = Skel.m_pBone[count];

        m_pBone[count].iParent=         Skel.m_pBone[count].iParent;
        m_pBone[count].nChildren=         Skel.m_pBone[count].nChildren;
        x_strcpy(m_pBone[count].Name,Skel.m_pBone[count].Name);

        m_pBone[count].Position =      Skel.m_pBone[count].BindTranslation;
        m_pBone[count].Rotation =     Skel.m_pBone[count].BindRotation;
        m_pBone[count].Scale =     Skel.m_pBone[count].BindScale;

    }

    // Transform all verts into model space of new skeleton
    for(i = 0; i < m_nVertices; i++)
    {
        s32 iBone = m_pVertex[i].Weight[0].iBone;
        vector3 P = m_pVertex[i].Position;
        matrix4 BM;
        BM.Identity();
        BM.Scale( m_pBone[iBone].Scale );
        BM.Rotate( m_pBone[iBone].Rotation );
        BM.Translate( m_pBone[iBone].Position );
        m_pVertex[i].Position = BM * P;
    }

}


//=========================================================================

void rawmesh::ApplyNewSkeleton( const rawmesh& Skel )
{
    s32 i;

    // Transform all verts into local space of current skeleton
    for(i = 0; i < m_nVertices; i++)
    {
        s32 iBone = m_pVertex[i].Weight[0].iBone;
        vector3 P = m_pVertex[i].Position;
        matrix4 BM;
        BM.Identity();
        BM.Scale( m_pBone[iBone].Scale );
        BM.Rotate( m_pBone[iBone].Rotation );
        BM.Translate( m_pBone[iBone].Position );
        BM.InvertSRT();
        m_pVertex[i].Position = BM * P;
    }

    for(s32 iVertex = 0; iVertex < m_nVertices; iVertex++)
    {
        vertex* pVertex = &m_pVertex[iVertex];
        for(s32 iWeight = 0; iWeight < pVertex->nWeights; iWeight++)
        {
            weight* pWeight = &pVertex->Weight[iWeight];
            s32 oldBoneId = pWeight->iBone;
            const char* oldBoneName = this->m_pBone[oldBoneId].Name;
            s32 newBoneId = -1;
            s32 curBoneId = oldBoneId;
            const char* curBoneName = oldBoneName;
            while(newBoneId == -1)
            {
                //x_DebugMsg("Looking in new skeleton for name '%s'\n", curBoneName);
                newBoneId = Skel.GetBoneIDFromName(curBoneName);
                curBoneId = m_pBone[curBoneId].iParent;
                ASSERT((newBoneId !=-1) || (curBoneId != -1));
                curBoneName = m_pBone[curBoneId].Name;
            }
            //x_DebugMsg("For old bone of %d, found new bone %d\n", pWeight->iBone, newBoneId);
            pWeight->iBone = newBoneId;
        }
    }

    // Copy new bone information in
    m_nBones = Skel.m_nBones;
    if(m_pBone)
        delete[] m_pBone;
    m_pBone = new bone[m_nBones];
    if( m_pBone == NULL )
        x_throw( "Ouf of memory" );

    for(s32 count = 0; count < m_nBones; count++)
    {
        const bone& Bone = Skel.m_pBone[count];
        m_pBone[count] = Bone;
    }

    // Transform all verts into local space of current skeleton
    for(i = 0; i < m_nVertices; i++)
    {
        s32 iBone = m_pVertex[i].Weight[0].iBone;
        vector3 P = m_pVertex[i].Position;
        matrix4 BM;
        BM.Identity();
        BM.Scale( m_pBone[iBone].Scale );
        BM.Rotate( m_pBone[iBone].Rotation );
        BM.Translate( m_pBone[iBone].Position );
        m_pVertex[i].Position = BM * P;
    }
}

//=========================================================================

bbox rawmesh::GetSubmeshBBox( s32 iSubmesh )
{
    bbox BBox;
    BBox.Clear();
    for( s32 i=0; i<m_nFacets; i++ )
    if( m_pFacet[i].iMesh == iSubmesh )
    {
        for( s32 j=0; j<m_pFacet[i].nVertices; j++ )
            BBox += m_pVertex[ m_pFacet[i].iVertex[j] ].Position;
    }
    return BBox;
}

//=========================================================================


