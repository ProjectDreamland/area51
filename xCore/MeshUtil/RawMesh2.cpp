#ifdef TARGET_PC
#include <crtdbg.h>
#endif

#include "RawMesh2.hpp"
#include "TextIn.hpp"
#include "TextOut.hpp"
#include "rawmesh.hpp"

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


static
s32 MeshSort( const void* pSubMesh1, const void* pSubMesh2 )
{
    return x_stricmp( (const char*)pSubMesh1, (const char*)pSubMesh2 );
}

//=========================================================================
// PROTOTYPES
//=========================================================================

static void SetupMat_Default( rawmesh2::material& M2, rawmesh::material& M );

//=========================================================================

void rawmesh2::SetRenameDuplicates ( xbool b )
{
     s_RawMeshRenamesDuplicates = b;
}

//=========================================================================

rawmesh2::rawmesh2( void )
{
    x_memset( this, 0, sizeof(*this) );
}

//=========================================================================

rawmesh2::~rawmesh2( void )
{    
    Kill();
}

//=========================================================================

void rawmesh2::Kill( void )
{
    if(m_pBone)         delete[]m_pBone;
    if(m_pRigidBodies)  delete[]m_pRigidBodies;
    if(m_pVertex)       delete[]m_pVertex;
    if(m_pFacet)        delete[]m_pFacet;
    if(m_pTexture)      delete[]m_pTexture;
    if(m_pMaterial)     delete[]m_pMaterial;
    if(m_pSubMesh)      delete[]m_pSubMesh;
    if(m_pParamKey)     delete[]m_pParamKey;

    x_memset( this, 0, sizeof(*this) );
}

//=========================================================================

xbool rawmesh2::Load( const char* pFileName )
{
    Kill();

    //==-----------------------------------------
    //  First, we need to crack the file open, and
    //  figure out which version it is.
    //==-----------------------------------------
    text_in     File;
    xbool       bIsAtLeastMATX2 = FALSE;
    s32         FileVersion = 0;

    // Copy source file
    if( pFileName )
        x_strcpy( m_SourceFile, pFileName );

    //==---------------------
    // Open the file
    //==---------------------
    File.OpenFile( pFileName );

    //==---------------------
    // Read the first header
    //==---------------------
    if ( FALSE == File.ReadHeader() )
        return FALSE;

    //==---------------------
    //  Check to see if it is
    //  a version number
    //==---------------------
    if ( x_stricmp( File.GetHeaderName(), "MATXVersion" ) == 0)
    {
        if( File.ReadFields() == FALSE )                    
        {
            return FALSE;
        }
        bIsAtLeastMATX2 = TRUE;
        File.GetS32( "Version", FileVersion );
    }

    //==-----------------------------------------
    //  Now, load the file
    //==-----------------------------------------
    if ( bIsAtLeastMATX2 )
    {
        return LoadMatx2( File );
    }
    else
    {   
        //==-------------------------------------
        //  It's an old matx, so use the
        //  original rawmesh to load it, and
        //  then translate the data.
        //==-------------------------------------
        File.CloseFile();


        rawmesh RM;

        if (RM.Load(pFileName))
        {
            BuildFromRM(RM);
            RM.Kill();
            return TRUE;
        }
    }

    return FALSE;
}

//=========================================================================

xbool rawmesh2::LoadMatx2( text_in& File )
{
    while( File.ReadHeader() == TRUE )
    {
        if( x_stricmp( File.GetHeaderName(), "TimeStamp" ) == 0 )
        {
            if( File.ReadFields() == FALSE )
            {
                return FALSE;
            }

            f32 Time[3];
            File.GetField( "Time:ddd", &Time[0], &Time[1], &Time[2] );
            File.GetField( "Date:ddd", &m_ExportDate[0], &m_ExportDate[1], &m_ExportDate[2] );
        }
        else if( x_stricmp( File.GetHeaderName(), "UserInfo" ) == 0 )
        {
            if( File.ReadFields() == FALSE )
            {
                return FALSE;
            }

            File.GetField( "UserName:s",     m_UserName );
            File.GetField( "ComputerName:s", m_ComputerName );
        }
        else if( x_stricmp( File.GetHeaderName(), "Hierarchy" ) == 0 )
        {
            // Allocate the bone count
            m_nBones = File.GetHeaderCount();
            m_pBone  = new bone[ m_nBones ];
            ASSERT( m_pBone );
            x_memset( m_pBone, 0, sizeof(bone)*m_nBones );

            // Read each of the fields in the file
            for( s32 i=0; i<File.GetHeaderCount(); i++ )
            {
                bone& Bone = m_pBone[i];
                s32   Index;

                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                File.GetField( "Index:d",      &Index          );
                File.GetField( "Name:s",       Bone.Name       );
                File.GetField( "nChildren:d",  &Bone.nChildren );
                File.GetField( "iParent:d",    &Bone.iParent   );
                File.GetField( "Scale:fff",    &Bone.Scale.GetX(), &Bone.Scale.GetY(), &Bone.Scale.GetZ() );
                File.GetField( "Rotate:ffff",  &Bone.Rotation.X, &Bone.Rotation.Y, &Bone.Rotation.Z, &Bone.Rotation.W );
                File.GetField( "Pos:fff",      &Bone.Position.GetX(), &Bone.Position.GetY(), &Bone.Position.GetZ() );
                Bone.LODGroup = -1;
                File.GetField( "LODGroup:d",   &Bone.LODGroup );
            }
        }
        else if( x_stricmp( File.GetHeaderName(), "RigidBodies" ) == 0 )
        {
            // Allocate the rigid body count
            m_nRigidBodies = File.GetHeaderCount();
            m_pRigidBodies = new rigid_body[ m_nRigidBodies ];
            ASSERT( m_pRigidBodies );
            x_memset( m_pRigidBodies, 0, sizeof(rigid_body)*m_nRigidBodies );

            // Read each of the fields in the file
            for( s32 i=0; i<File.GetHeaderCount(); i++ )
            {
                rigid_body& RigidBody = m_pRigidBodies[i];
                s32         Index;

                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                File.GetField( "Index:d",           &Index               );
                File.GetField( "Name:s",            RigidBody.Name       );
                File.GetField( "Type:s",            RigidBody.Type       );
                File.GetField( "Mass:f",            &RigidBody.Mass      );
                File.GetField( "iParent:d",         &RigidBody.iParent   );
                File.GetField( "Body_Scale:fff",    &RigidBody.BodyScale.GetX(),     &RigidBody.BodyScale.GetY(),     &RigidBody.BodyScale.GetZ() );
                File.GetField( "Body_Rotate:ffff",  &RigidBody.BodyRotation.X,       &RigidBody.BodyRotation.Y,       &RigidBody.BodyRotation.Z, &RigidBody.BodyRotation.W );
                File.GetField( "Body_Pos:fff",      &RigidBody.BodyPosition.GetX(),  &RigidBody.BodyPosition.GetY(),  &RigidBody.BodyPosition.GetZ() );
                File.GetField( "Pivot_Scale:fff",   &RigidBody.PivotScale.GetX(),    &RigidBody.PivotScale.GetY(),    &RigidBody.PivotScale.GetZ() );
                File.GetField( "Pivot_Rotate:ffff", &RigidBody.PivotRotation.X,      &RigidBody.PivotRotation.Y,      &RigidBody.PivotRotation.Z, &RigidBody.PivotRotation.W );
                File.GetField( "Pivot_Pos:fff",     &RigidBody.PivotPosition.GetX(), &RigidBody.PivotPosition.GetY(), &RigidBody.PivotPosition.GetZ() );
                File.GetField( "Radius:f",          &RigidBody.Radius          );
                File.GetField( "Width:f",           &RigidBody.Width           );
                File.GetField( "Height:f",          &RigidBody.Height          );
                File.GetField( "Length:f",          &RigidBody.Length          );
                File.GetField( "TX_Act:d",          &RigidBody.DOF[0].bActive  );
                File.GetField( "TX_Lim:d",          &RigidBody.DOF[0].bLimited );
                File.GetField( "TX_Min:f",          &RigidBody.DOF[0].Min      );
                File.GetField( "TX_Max:f",          &RigidBody.DOF[0].Max      );
                File.GetField( "TY_Act:d",          &RigidBody.DOF[1].bActive  );
                File.GetField( "TY_Lim:d",          &RigidBody.DOF[1].bLimited );
                File.GetField( "TY_Min:f",          &RigidBody.DOF[1].Min      );
                File.GetField( "TY_Max:f",          &RigidBody.DOF[1].Max      );
                File.GetField( "TZ_Act:d",          &RigidBody.DOF[2].bActive  );
                File.GetField( "TZ_Lim:d",          &RigidBody.DOF[2].bLimited );
                File.GetField( "TZ_Min:f",          &RigidBody.DOF[2].Min      );
                File.GetField( "TZ_Max:f",          &RigidBody.DOF[2].Max      );
                File.GetField( "RX_Act:d",          &RigidBody.DOF[3].bActive  );
                File.GetField( "RX_Lim:d",          &RigidBody.DOF[3].bLimited );
                File.GetField( "RX_Min:f",          &RigidBody.DOF[3].Min      );
                File.GetField( "RX_Max:f",          &RigidBody.DOF[3].Max      );
                File.GetField( "RY_Act:d",          &RigidBody.DOF[4].bActive  );
                File.GetField( "RY_Lim:d",          &RigidBody.DOF[4].bLimited );
                File.GetField( "RY_Min:f",          &RigidBody.DOF[4].Min      );
                File.GetField( "RY_Max:f",          &RigidBody.DOF[4].Max      );
                File.GetField( "RZ_Act:d",          &RigidBody.DOF[5].bActive  );
                File.GetField( "RZ_Lim:d",          &RigidBody.DOF[5].bLimited );
                File.GetField( "RZ_Min:f",          &RigidBody.DOF[5].Min      );
                File.GetField( "RZ_Max:f",          &RigidBody.DOF[5].Max      );
            }
        }
        else if( x_stricmp( File.GetHeaderName(), "Vertices"     ) == 0 )
        {
            // Allocate the vertex count
            m_nVertices = File.GetHeaderCount();
            m_pVertex   = new vertex[ m_nVertices ];
            ASSERT( m_pVertex );
            x_memset( m_pVertex, 0, sizeof(vertex)*m_nVertices );

            // Read each of the fields in the file
            for( s32 i=0; i<File.GetHeaderCount(); i++ )
            {
                vertex& Vertex = m_pVertex[i];
                s32     Index;

                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                File.GetField( "Index:d",      &Index          );
                File.GetField( "Pos:fff",      &Vertex.Position.GetX(), &Vertex.Position.GetY(), &Vertex.Position.GetZ() );
                File.GetField( "nNormals:d",   &Vertex.nNormals );
                File.GetField( "nUVSets:d",    &Vertex.nUVs     );
                File.GetField( "nColors:d",    &Vertex.nColors  );
                File.GetField( "nWeights:d",   &Vertex.nWeights );

                Vertex.nWeights = MIN(Vertex.nWeights,VERTEX_MAX_WEIGHT);
            }
        }
        else if( x_stricmp( File.GetHeaderName(), "Colors"     ) == 0 )
        {
            // Allocate the vertex count
            s32 nColors = File.GetHeaderCount();

            // Read each of the fields in the file
            for( s32 i=0; i<nColors; i++ )
            {
                s32     iVertex, SubIndex;
                f32     R,G,B,A;

                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                File.GetField( "iVertex:d",    &iVertex          );
                vertex& Vertex = m_pVertex[iVertex];

                File.GetField( "Index:d",      &SubIndex          );
                xcolor& C = Vertex.Color[SubIndex];

                File.GetField( "Color:ffff",   &R, &G, &B, &A );
                C.SetfRGBA( R, G, B, A );
            }
        }
        else if( x_stricmp( File.GetHeaderName(), "Normals"      )  == 0 )
        {
            ASSERT( File.GetHeaderCount() >= m_nVertices );
    
            for( s32 i=0; i<m_nVertices; i++ )
            for( s32 j=0; j<m_pVertex[i].nNormals; j++ )
            {
                s32     Index, SubIndex;

                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                File.GetField( "iVertex:d",    &Index           );
                File.GetField( "Index:d",      &SubIndex        );

                ASSERT( Index == i );
                ASSERT( SubIndex == j );

                File.GetField( "Normal:fff", &m_pVertex[i].Normal[j].GetX(), &m_pVertex[i].Normal[j].GetY(), &m_pVertex[i].Normal[j].GetZ() );
            }
        }
        else if( x_stricmp( File.GetHeaderName(), "UVSet"        ) == 0 )
        {    
            for( s32 i=0; i<m_nVertices; i++ )
            for( s32 j=0; j<m_pVertex[i].nUVs; j++ )
            {
                s32     Index, SubIndex;

                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                File.GetField( "iVertex:d",    &Index           );
                File.GetField( "Index:d",      &SubIndex        );

                ASSERT( Index == i );
                ASSERT( SubIndex == j );

                File.GetField( "UV:ff", &m_pVertex[i].UV[j].X, &m_pVertex[i].UV[j].Y );
            }            
        }
        else if( x_stricmp( File.GetHeaderName(), "Skin"         ) == 0 )
        {
            s32 m_nSkin = File.GetHeaderCount();
            if(m_nSkin > m_nVertices)
                m_nSkin = m_nVertices;
            for( s32 i=0; i<m_nSkin; i++ )
            for( s32 j=0; j<m_pVertex[i].nWeights; j++ )
            {
                s32     Index, SubIndex;

                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                File.GetField( "iVertex:d",    &Index           );
                File.GetField( "Index:d",      &SubIndex        );

                ASSERT( Index == i );
                ASSERT( SubIndex == j );

                File.GetField( "iBone:d",  &m_pVertex[i].Weight[j].iBone );
                File.GetField( "Weight:f", &m_pVertex[i].Weight[j].Weight );
            }        
            for( s32 k=m_nSkin; k < m_nVertices; k++ )
            {
                m_pVertex[k].Weight[0].iBone  = 0;
                m_pVertex[k].Weight[0].Weight = 1.0f;
            }
        }        
        else if( x_stricmp( File.GetHeaderName(), "Polygons"     ) == 0 )
        {
            // Allocate the vertex count
            m_nFacets   = File.GetHeaderCount();
            m_pFacet    = new facet[ m_nFacets ];
            ASSERT( m_pFacet );
            x_memset( m_pFacet, 0, sizeof(facet)*m_nFacets );

            // Read each of the fields in the file
            for( s32 i=0; i<m_nFacets; i++ )
            {
                facet&  Facet = m_pFacet[i];
                s32     Index;

                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                File.GetField( "Index:d",      &Index           );
                File.GetField( "iMesh:d",      &Facet.iMesh     );
                File.GetField( "nVerts:d",     &Facet.nVertices );
                File.GetField( "Normal:fff",   &Facet.Plane.Normal.GetX(), &Facet.Plane.Normal.GetY(), &Facet.Plane.Normal.GetZ() );
                File.GetField( "iMaterial:d",  &Facet.iMaterial );
            }            
        }
        else if( x_stricmp( File.GetHeaderName(), "FacetIndex"   )  == 0 )
        {
            ASSERT( File.GetHeaderCount() >= m_nFacets );
    
            for( s32 i=0; i<m_nFacets; i++ )
            for( s32 j=0; j<m_pFacet[i].nVertices; j++ )
            {
                s32     Index, SubIndex;

                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                File.GetField( "iFacet:d",     &Index           );
                File.GetField( "Index:d",      &SubIndex        );

                ASSERT( Index == i );
                ASSERT( SubIndex == j );

                File.GetField( "iVertex:d",  &m_pFacet[i].iVertex[j] );
            }            
        }
        else if( x_stricmp( File.GetHeaderName(), "Mesh" ) == 0 )
        {
            xarray<char *> MeshNames;
            xarray<u32>    MeshCounts;
            char           MeshNameTmp[512];

            m_nSubMeshs = File.GetHeaderCount();
    
            m_pSubMesh  = new sub_mesh  [ m_nSubMeshs ];
            if( m_pSubMesh == NULL )
                x_throw( "Out of memory" );

            x_memset( m_pSubMesh, 0, sizeof(sub_mesh)*m_nSubMeshs );
            
            s32 i=0;
            // Read each of the fields in the file
            for( i=0; i<m_nSubMeshs; i++ )
            {
                sub_mesh& SubMesh = m_pSubMesh[i];
                s32       Index;

                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                File.GetField( "Index:d",  &Index );
                if( Index != i ) x_throw( "Wrong index from a mesh field" );

                File.GetField( "Name:s", MeshNameTmp );                
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
                    x_strncpy( SubMesh.Name, MeshNameTmp, sizeof(SubMesh.Name)-1 );

                    // Make sure that none of the sub meshes have the same name.
                    for(s32 count = 0; count < i ; count++)
                    {
                        if(x_strcmp((const char*)m_pSubMesh[count].Name, (const char*)SubMesh.Name) == 0)
                        {
                            x_printf( "ERROR: Multiple meshes with the same name [%s]\n", File.GetFileName() );
                        }
                    }
                    //File.GetField( "Name:s",  &SubMesh.Name );
                }                
            }
            
            (void)MeshSort;
            //x_qsort( m_pSubMesh[0], m_nSubMeshs, sizeof(sub_mesh), MeshSort );
        }
        
        //==-------------------------------------
        //==-------------------------------------
        //==-------------------------------------
        //  MATERIAL DATA BEGINS HERE
        //==-------------------------------------
        //==-------------------------------------
        //==-------------------------------------
        else if( x_stricmp( File.GetHeaderName(), "Material_Textures" ) == 0 )
        {
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
                File.GetField( "Filename:s",    Texture.FileName );
            }
        }
        else if (x_stricmp( File.GetHeaderName(), "Material_ParamPkg_Keys" ) == 0)
        {
            // Load the keys
            m_nParamKeys = File.GetHeaderCount();
            m_pParamKey  = new f32[ m_nParamKeys ];
            ASSERT( m_pParamKey );
            x_memset( m_pParamKey, 0, sizeof(f32)*m_nParamKeys );

            for( s32 i=0; i<m_nParamKeys; i++ )
            {
                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                s32     Index;
                File.GetField( "Index:d",   &Index              );
                ASSERTS(Index == i,"Reading went out of sync during load of Material_ParamPkg_Keys block" );
                File.GetField( "Key:f",     &m_pParamKey[i]     );
            }
        }
        else if( x_stricmp( File.GetHeaderName(), "Materials"    ) == 0 )
        {
            // Allocate storage and load materials
            m_nMaterials = File.GetHeaderCount();
            m_pMaterial = new material[ m_nMaterials ];
            ASSERT( m_pMaterial );
            x_memset( m_pMaterial, 0, sizeof(material)*m_nMaterials );

            for( s32 i=0; i<m_nMaterials; i++ )
            {
                material&   Material = m_pMaterial[i];
                s32         Index;
                char        LightingType[256];
                char        BlendType[256];   
                char        TintType[256];

                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                File.GetField( "Index:d",               &Index                  );

                ASSERTS(Index == i,xfs("Reading went out of sync during load of Material block at element %ld",i) );

                File.GetField( "Name:s",                Material.Name           );
                File.GetField( "Type:d",                &Material.Type           );
                File.GetField( "LightingType:s",        LightingType            );
                File.GetField( "BlendType:s",           BlendType               );
                File.GetField( "TwoSided:d",            &Material.bTwoSided     );

                File.GetField( "RandomAnim:d",          &Material.bRandomizeAnim);
                File.GetField( "SortBias:d",            &Material.Sort          );
                File.GetField( "TintType:s",            TintType                );
                File.GetField( "Punchthrough:d",        &Material.bPunchthrough );
                File.GetField( "VertexAlpha:d",         &Material.bHasVariableVertAlpha     );
                File.GetField( "ExposeName:d",          &Material.bExposeName   );

                if( !x_strcmp( LightingType, "SELF_ILLUM" ) )
                    Material.LightingType = LIGHTING_TYPE_SELF_ILLUM;
                
                //
                //  We don't know how many keys, or what the first one will be
                //
                Material.iFirstKey = 999999;
                Material.nKeys     = 0;
            }
        }
        else if( x_stricmp( File.GetHeaderName(), "Material_Maps"    ) == 0 )
        {
            // Load the maps
            s32 nMaps = File.GetHeaderCount();
            s32 i;

            for (i=0;i<nMaps;i++)
            {
                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                s32     Index;
                s32     iMaterial;
                s32     iMap;
                char    Filter[256];
                char    UAddr[256];
                char    VAddr[256];
                char    RGBASource[256];

                File.GetField( "Index:d", &Index );

                ASSERTS(Index == i,xfs("Reading went out of sync during load of Material_Maps block at element %ld",i) );

                File.GetField( "iMaterial:d",   &iMaterial );
                File.GetField( "iMap:d",        &iMap );
                
                ASSERTS( (iMaterial >= 0)  && (iMaterial < m_nMaterials      ),xfs("Material map index %ld has out of bounds material ID [%ld]",i,iMaterial));
                ASSERTS( (iMap      >= -1) && (iMap      < MATERIAL_MAX_MAPS ),xfs("Material map index %ld has out of bounds map ID [%ld]",i,iMap));

                map&    Map = m_pMaterial[ iMaterial ].Map[ iMap ];

                File.GetField( "iTextures:d",   &Map.iTexture   );
                File.GetField( "nTextures:d",   &Map.nTextures  );
                File.GetField( "TextureFPS:d",  &Map.TextureFPS );
                File.GetField( "iUV:d",         &Map.iUVChannel );
                File.GetField( "RGBASource:s",  RGBASource      );
                File.GetField( "FilterType:s",  Filter          );
                File.GetField( "UAddress:s",    UAddr           );
                File.GetField( "VAddress:s",    VAddr           );

                if (x_stricmp( Filter, "Point" ) == 0)
                    Map.FilterType = FILTER_TYPE_POINT;
                else if (x_stricmp( Filter, "Bilinear" ) == 0)
                    Map.FilterType = FILTER_TYPE_BILINEAR;
                else if (x_stricmp( Filter, "Trilinear" ) == 0)
                    Map.FilterType = FILTER_TYPE_TRILINEAR;
                else if (x_stricmp( Filter, "Anisotropic" ) == 0)
                    Map.FilterType = FILTER_TYPE_ANISOTROPIC;
                else
                    ASSERTS( FALSE, xfs("Encountered an unknown Material_Maps FilterType type [%s] at index %ld",Filter,i));


                if (x_stricmp( UAddr, "Wrap" ) == 0)
                    Map.UAddress = ADDRESS_TYPE_WRAP;
                else if (x_stricmp( UAddr, "Clamp" ) == 0)
                    Map.UAddress = ADDRESS_TYPE_CLAMP;
                else if (x_stricmp( UAddr, "Mirror" ) == 0)
                    Map.UAddress = ADDRESS_TYPE_MIRROR;
                else
                    ASSERTS( FALSE, xfs("Encountered an unknown Material_Maps UAddress type [%s] at index %ld",UAddr,i));


                if (x_stricmp( VAddr, "Wrap" ) == 0)
                    Map.VAddress = ADDRESS_TYPE_WRAP;
                else if (x_stricmp( VAddr, "Clamp" ) == 0)
                    Map.VAddress = ADDRESS_TYPE_CLAMP;
                else if (x_stricmp( VAddr, "Mirror" ) == 0)
                    Map.VAddress = ADDRESS_TYPE_MIRROR;
                else
                    ASSERTS( FALSE, xfs("Encountered an unknown Material_Maps VAddress type [%s] at index %ld",VAddr,i));

                if (x_stricmp( RGBASource, "RGB" ) == 0)
                    Map.RGBASource = RGBA_SOURCE_TYPE_RGB;
                else if (x_stricmp( RGBASource, "R" ) == 0)
                    Map.RGBASource = RGBA_SOURCE_TYPE_R;
                else if (x_stricmp( RGBASource, "G" ) == 0)
                    Map.RGBASource = RGBA_SOURCE_TYPE_G;
                else if (x_stricmp( RGBASource, "B" ) == 0)
                    Map.RGBASource = RGBA_SOURCE_TYPE_B;
                else if (x_stricmp( RGBASource, "A" ) == 0)
                    Map.RGBASource = RGBA_SOURCE_TYPE_A;
                else
                    ASSERTS( FALSE, xfs("Encountered an unknown Material_Maps RGBASource type [%s] at index %ld",VAddr,i));
            }
        }
        else if( x_stricmp( File.GetHeaderName(), "Material_ParamPkg"    ) == 0 )
        {
            // Load the param packages
            s32 nPackages = File.GetHeaderCount();
            s32 i;

            for (i=0;i<nPackages;i++)
            {
                if( File.ReadFields() == FALSE )                    
                {
                    return FALSE;
                }

                s32     Index;
                s32     iMaterial;
                s32     iMap;
                char    Package[256];
                char    Mode[256];

                param_pkg*  pPkg = NULL;

                File.GetField( "Index:d", &Index );

                ASSERTS(Index == i,xfs("Reading went out of sync during load of Material_ParamPkg block at element %ld",i) );

                File.GetField( "iMaterial:d",   &iMaterial );
                File.GetField( "iMap:d",        &iMap );
                File.GetField( "iPackage:s",    Package );

                ASSERTS( (iMaterial >= 0)  && (iMaterial < m_nMaterials      ),xfs("Param pkg index %ld has out of bounds material ID [%ld]",i,iMaterial));
                ASSERTS( (iMap      >= -1) && (iMap      < MATERIAL_MAX_MAPS ),xfs("Param pkg index %ld has out of bounds map ID [%ld]",i,iMap));

                //==---------------------------------------
                //  From this, we can determine which 
                //  parameter package we are dealing with
                //==---------------------------------------

                if ( iMap == -1 )
                {
                    //==---------------------------------------
                    //  The param pkg is part of the material
                    //==---------------------------------------
                    if ( x_stricmp( Package, "Tint Color" ) == 0)
                        pPkg = &(m_pMaterial[ iMaterial ].TintColor);
                    else if ( x_stricmp( Package, "Tint Alpha" ) == 0)
                        pPkg = &(m_pMaterial[ iMaterial ].TintAlpha);
                    else if ( x_stricmp( Package, "Constant0" ) == 0)
                        pPkg = &(m_pMaterial[ iMaterial ].Constants[0]);
                    else if ( x_stricmp( Package, "Constant1" ) == 0)
                        pPkg = &(m_pMaterial[ iMaterial ].Constants[1]);
                    else if ( x_stricmp( Package, "Constant2" ) == 0)
                        pPkg = &(m_pMaterial[ iMaterial ].Constants[2]);
                    else if ( x_stricmp( Package, "Constant3" ) == 0)
                        pPkg = &(m_pMaterial[ iMaterial ].Constants[3]);
                    else if ( x_stricmp( Package, "Constant4" ) == 0)
                        pPkg = &(m_pMaterial[ iMaterial ].Constants[4]);
                    else if ( x_stricmp( Package, "Constant5" ) == 0)
                        pPkg = &(m_pMaterial[ iMaterial ].Constants[5]);
                    else if ( x_stricmp( Package, "Constant6" ) == 0)
                        pPkg = &(m_pMaterial[ iMaterial ].Constants[6]);
                    else if ( x_stricmp( Package, "Constant7" ) == 0)
                        pPkg = &(m_pMaterial[ iMaterial ].Constants[7]);
                    else
                        ASSERTS( FALSE, xfs("Encountered an unknown param pkg type [%s] at index %ld",Package,i));
                }
                else
                {
                    //==---------------------------------------
                    //  The param pkg is part of a map
                    //==---------------------------------------
                    if ( x_stricmp( Package, "UV Translation" ) == 0)
                        pPkg = &(m_pMaterial[ iMaterial ].Map[ iMap ].UVTranslation);
                    else if ( x_stricmp( Package, "UV Rotation" ) == 0)
                        pPkg = &(m_pMaterial[ iMaterial ].Map[ iMap ].UVRotation);
                    else if ( x_stricmp( Package, "UV Scale" ) == 0)
                        pPkg = &(m_pMaterial[ iMaterial ].Map[ iMap ].UVScale);
                    else
                        ASSERTS( FALSE, xfs("Encountered an unknown param pkg type [%s] at index %ld",Package,i));
                }

                ASSERT( pPkg );

                //==---------------------------------------
                //  Now that we have the package, we can
                //  load the rest of the data
                //==---------------------------------------
                File.GetField( "Current_0:f",       &(pPkg->Current[0])     );
                File.GetField( "Current_1:f",       &(pPkg->Current[1])     );
                File.GetField( "Current_2:f",       &(pPkg->Current[2])     );
                File.GetField( "Current_3:f",       &(pPkg->Current[3])     );

                File.GetField( "ModeType:s",        Mode                    );

                File.GetField( "FPS:d",             &(pPkg->FPS)            );
                File.GetField( "iKeys:d",           &(pPkg->iFirstKey)      );
                File.GetField( "nKeys:d",           &(pPkg->nKeys)          );
                File.GetField( "nParamsPerKey:d",   &(pPkg->nParamsPerKey)  );
                
                if (x_stricmp( Mode, "CYCLE" ) == 0)
                    pPkg->ModeType = ANIM_TYPE_CYCLE;
                else if (x_stricmp( Mode, "CLAMP" ) == 0)
                    pPkg->ModeType = ANIM_TYPE_CLAMP;
                else if (x_stricmp( Mode, "PING_PONG" ) == 0)
                    pPkg->ModeType = ANIM_TYPE_PING_PONG;
                else if (x_stricmp( Mode, "LINEAR" ) == 0)
                    pPkg->ModeType = ANIM_TYPE_LINEAR;
                else if (x_stricmp( Mode, "RELATIVE_REPEAT" ) == 0)
                    pPkg->ModeType = ANIM_TYPE_RELATIVE_REPEAT;
                else
                    ASSERTS( FALSE, xfs("Encountered an unknown param pkg anim mode [%s] at index %ld",Mode,i));


                if ((pPkg->iFirstKey < m_pMaterial[ iMaterial ].iFirstKey) && (pPkg->iFirstKey >= 0))
                    m_pMaterial[ iMaterial ].iFirstKey = pPkg->iFirstKey;
                
                m_pMaterial[ iMaterial ].nKeys += pPkg->nKeys * pPkg->nParamsPerKey;


            }
        }
        //==-------------------------------------
        //==-------------------------------------
        //==-------------------------------------
        //  END OF MATERIAL DATA
        //==-------------------------------------
        //==-------------------------------------
        //==-------------------------------------
        
        else File.SkipToNextHeader();
    }

    //
    // Did we rich the end of file successfully?
    //
    if( File.IsEOF() == FALSE )
        return FALSE;

    //
    //  Now, we do a little post cleanup, where:
    //  If the a material has bHasVariableVertAlpha == FALSE,
    //  we sweep through all verts of all faces using that material,
    //  and copy the red channel into the alpha channel
    //

    s32     i;

    if (m_pMaterial)
    {
        for (i=0;i<m_nFacets;i++)
        {
            s32 iMat = m_pFacet[i].iMaterial;
            s32 iMatType = m_pMaterial[ iMat ].Type;

            xbool   Smear = FALSE;

            if ((iMatType == 4) || (iMatType == 6) || (iMatType == 8) || (iMatType == 9))
                Smear = TRUE;

            if (Smear)
            {
                s32 j;
                for (j=0;j<m_pFacet[i].nVertices;j++)
                {
                    vertex& Vert = m_pVertex[ m_pFacet[ i ].iVertex[ j ] ];
                    s32 k;
                    for (k=0;k<VERTEX_MAX_COLOR;k++)
                    {
                        Vert.Color[k].A = Vert.Color[k].R;
                    }
                }
            }
        }      
    }
  

    //
    //  Cleanup the hacked iFirstKey values
    //  Because this data isn't in the matx, we have to build it as
    //  we go.  If a material doesn't have any keys, it will never
    //  set iFirstKey to a valid value, so we change it to zero here.
    //
    for (i=0;i<m_nMaterials;i++)
    {
        if (m_pMaterial[i].iFirstKey == 999999)
            m_pMaterial[i].iFirstKey = 0;
    }

    // Compute all bone related info
    ComputeBoneInfo() ;

    return TRUE;
}

//=========================================================================

void rawmesh2::Save( const char* pFileName )
{
    (void)pFileName;
}

//=========================================================================

bbox rawmesh2::GetBBox( void )
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
s32 CompareFacetByMaterial( const rawmesh2::facet* pA, const rawmesh2::facet* pB )
{
    if( pA->iMesh < pB->iMesh ) return -1;
    if( pA->iMesh > pB->iMesh ) return  1;

    if( pA->iMaterial < pB->iMaterial ) return -1;
    return pA->iMaterial > pB->iMaterial;
}

//=========================================================================

void rawmesh2::SortFacetsByMaterial( void )
{
    x_qsort( m_pFacet, m_nFacets, sizeof(facet),
            (compare_fn*)CompareFacetByMaterial );
}

//=========================================================================

static rawmesh2 *g_pCompare;
s32 CompareFacetByMaterialAndBone( const rawmesh2::facet* pA, const rawmesh2::facet* pB )
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

void rawmesh2::SortFacetsByMaterialAndBone( void )
{
    g_pCompare = this;
    x_qsort( m_pFacet, m_nFacets, sizeof(facet),
            (compare_fn*)CompareFacetByMaterialAndBone );
}

//=========================================================================

static xbool TempVCompare( rawmesh2::vertex& A, rawmesh2::vertex& B )
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

static xbool CompareFaces( rawmesh2::facet& A, rawmesh2::facet& B )
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

void rawmesh2::CleanMesh( s32 iSubMesh /* = -1 */) // Remove this submesh
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
                x_throw( "Error while cleaning the materials in the rawmesh2" );

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
        
        // Go throough the materials and mark all the used textures
        for( i=0; i<m_nMaterials; i++ )
        for( s32 j=0; j<MATERIAL_MAX_MAPS; j++ )
        {
            map& Map = m_pMaterial[i].Map[j];

            if( Map.nTextures != 0 )
            {
                pUsed[ Map.iTexture ].Used = TRUE;
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
        for( s32 j=0; j<MATERIAL_MAX_MAPS; j++ )
        {
            map& Map = m_pMaterial[i].Map[j];

            if( Map.nTextures != 0 )
            {
                Map.iTexture = pUsed[ Map.iTexture ].ReIndex;
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
    // Remove unwanted meshes
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

    //
    // Sort the meshes so that they are in alphabetical order
    //    
    {
        s32 i, j;

        struct mesh_info
        {
            sub_mesh    Mesh;
            s32         iOriginal;
        };

        mesh_info* pMesh  = new mesh_info[ m_nSubMeshs ];
        s32*       pRemap = new s32[ m_nSubMeshs ];
        if( pMesh == NULL )
            x_throw( "Out of meory" );

        for( i=0; i<m_nSubMeshs; i++ )
        {
            pMesh[i].Mesh       = m_pSubMesh[i];    
            pMesh[i].iOriginal  = i;
        }

        // sort the meshes
        xbool bSorted = FALSE;
        for ( j = 0; j < m_nSubMeshs && !bSorted; j++ )
        {
            bSorted = TRUE;
            for ( i = 0; i < m_nSubMeshs-1-j; i++ )
            {
                if( x_strcmp( pMesh[i].Mesh.Name, pMesh[i+1].Mesh.Name ) > 0 )
                {
                    bSorted        = FALSE;
                    mesh_info Temp = pMesh[i+1];
                    pMesh[i+1]     = pMesh[i];
                    pMesh[i]       = Temp;
                }
            }
        }
        
        // sanity check to make sure we know how to sort
        for ( i = 0; i < m_nSubMeshs; i++ )
        {
            if ( i < m_nSubMeshs-1 )
            {
                ASSERT( x_strcmp( pMesh[i].Mesh.Name, pMesh[i+1].Mesh.Name ) <= 0 );
            }
        }

        // copy over the original submeshes
        for( i=0; i<m_nSubMeshs; i++ )
        {
            m_pSubMesh[i] = pMesh[i].Mesh;
        }

        // build a remap table for the faces
        for( i=0; i<m_nSubMeshs; i++ )
        {
            for ( j=0; j<m_nSubMeshs; j++ )
            {
                if ( pMesh[j].iOriginal == i )
                {
                    pRemap[i] = j;
                    break;
                }
            }
        }

        // remap the faces
        for( i=0; i<m_nFacets; i++ )
        {
            m_pFacet[i].iMesh = pRemap[m_pFacet[i].iMesh];
        }

        delete []pMesh;
        delete []pRemap;
    }    
}

//=========================================================================

void rawmesh2::SanityCheck( void )
{

    //
    // Check that we have a valid number of materials
    //
    if( m_nMaterials < 0 ) 
        x_throw( "The Rawmesh it saying that it has a negative number of materials!");

    if( m_nMaterials > 1000 ) 
        x_throw( "The rawmesh2 has more than 1000 materials right now that is a sign of a problem" );
    
    if( m_nVertices < 0 )
        x_throw( "THe rawmesh2 has a negative number of vertices!" );

    if( m_nVertices > 100000000 )
        x_throw( "The rawmesh2 seems to have more that 100 million vertices that is consider bad" );

    if( m_nFacets < 0 )
        x_throw( "We have a negative number of facets" );

    if( m_nFacets > 100000000 )
        x_throw( "THe rawmesh2 has more thatn 100 million facets that is consider worng" );

    if( m_nBones < 0 ) 
        x_throw ("We have a negative count for bones" );
    
    if( m_nBones > 100000 )
        x_throw ("We have more than 100,000 bones this may be a problem" );

    if( m_nTextures < 0 )
        x_throw( "Found a negative number of textures" );

    if( m_nSubMeshs < 0 )
        x_throw( "We have a negative number of meshes this is not good" );

    if( m_nSubMeshs > 100000 )
        x_throw( "We have more than 100,000 meshes this may be a problem" );

    //
    // Check the facets.
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
    // Check the vertices.
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

                //if( V.nUVs != m_pMaterial[ Facet.iMaterial ].GetUVChanelCount() )
                //    x_throw( xfs("I found a vertex that didn't have the right number of UVs for the material been used. V#:%d, F#:%d",j,i));

            }
        }
    }
}

//=========================================================================

void rawmesh2::CleanWeights( s32 MaxNumWeights, f32 MinWeightValue )
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

void rawmesh2::CollapseMeshes( const char* pMeshName )
{
    for( s32 i=0; i<m_nFacets; i++ )
    {
        m_pFacet[i].iMesh = 0;
    }

    m_nSubMeshs = 1;
    x_strcpy( m_pSubMesh[0].Name, pMeshName );
}

//=========================================================================

void rawmesh2::ComputeMeshBBox( s32 iMesh, bbox& BBox )
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

// Computes bone bboxes and the number of bones used by each submesh
void rawmesh2::ComputeBoneInfo( void )
{
    s32 i,j,k ;

    //=====================================================================
    // Compute bone bboxes
    //=====================================================================

    // Clear all bone bboxes
    for (i = 0 ; i < m_nBones ; i++)
        m_pBone[i].BBox.Clear() ;

    // Loop through all the verts and add to bone bboxes
    for (i = 0 ; i < m_nVertices ; i++)
    {
        // Lookup vert
        vertex& Vertex = m_pVertex[i] ;

        // Loop through all weights in vertex
        for (s32 j = 0 ; j < Vertex.nWeights ; j++)
        {
            // Lookup bone that vert is attached to
            s32 iBone = Vertex.Weight[j].iBone ;
            ASSERT(iBone >= 0) ;
            ASSERT(iBone < m_nBones) ;

            // Add to bone bbox
            m_pBone[iBone].BBox += Vertex.Position ;
        }
    }

    // If a bone has no geometry attached, then set bounds to the bone position
    for (i = 0 ; i < m_nBones ; i++)
    {
        // Lookup bone
        bone& Bone = m_pBone[i] ;

        // If bbox is empty, just use the bone position
        if (Bone.BBox.Min.GetX() > Bone.BBox.Max.GetX())
            Bone.BBox += Bone.Position ;

        // Inflate slightly do get rid of any degenerate (flat) sides
        Bone.BBox.Inflate(0.1f, 0.1f, 0.1f) ;
    }

    //=====================================================================
    // Compute # of bones used by each sub-mesh
    // Bones are arranged in LOD order, so we can just use the (MaxBoneUsed+1)
    //=====================================================================
    
    // Clear values
    for (i = 0 ; i < m_nSubMeshs ; i++)
        m_pSubMesh[i].nBones = 0 ;

    // Loop through all faces
    for (i = 0 ; i < m_nFacets ; i++)
    {
        // Lookup face and the mesh it's part of
        facet&      Face = m_pFacet[i] ;
        sub_mesh&   Mesh = m_pSubMesh[Face.iMesh] ;

        // Loop through all verts in each face
        for (j = 0 ; j < Face.nVertices ; j++)
        {
            // Lookup vert
            vertex& Vert = m_pVertex[Face.iVertex[j]] ;

            // Loop through all weights in vert
            for (k = 0 ; k < Vert.nWeights ; k++)
            {
                // Update mesh bone count
                Mesh.nBones = MAX(Mesh.nBones, Vert.Weight[k].iBone) ;
            }
        }
    }

    // We want the actual number of bones used so fix up
    for (i = 0 ; i < m_nSubMeshs ; i++)
        m_pSubMesh[i].nBones++ ;
}

//=========================================================================

xbool rawmesh2::IsolateSubmesh( s32 iSubMesh, rawmesh2& NewMesh,
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
        NewMesh.m_pBone = new rawmesh2::bone[ m_nBones ];
        ASSERT(NewMesh.m_pBone);
        NewMesh.m_nBones = m_nBones;
        x_memcpy( NewMesh.m_pBone, m_pBone, sizeof(rawmesh2::bone)*NewMesh.m_nBones );
    }
    else
    {
        NewMesh.m_pBone  = NULL;
        NewMesh.m_nBones = 0;
    }

    //
    //  Submeshes
    //
    ASSERT( m_nSubMeshs > 0 );
    
    NewMesh.m_pSubMesh = new rawmesh2::sub_mesh[ 1 ];
    ASSERT(NewMesh.m_pSubMesh);
    NewMesh.m_nSubMeshs = 1;
    x_memcpy( NewMesh.m_pSubMesh, &(m_pSubMesh[iSubMesh]), sizeof(rawmesh2::sub_mesh) );
    
    
    //
    // Materials:
    //
    if (m_nMaterials > 0)
    {
        NewMesh.m_pMaterial = new rawmesh2::material[ m_nMaterials ];
        ASSERT(NewMesh.m_pMaterial);
        NewMesh.m_nMaterials = m_nMaterials;
        x_memcpy( NewMesh.m_pMaterial, m_pMaterial, sizeof(rawmesh2::material)*NewMesh.m_nMaterials);
    }
    else
    {
        NewMesh.m_pMaterial  = NULL;
        NewMesh.m_nMaterials = 0;
    }

    //
    //  Param Keys:
    //
    if (m_nParamKeys > 0)
    {
        NewMesh.m_pParamKey = new f32[ m_nParamKeys ];
        ASSERT(NewMesh.m_pParamKey);
        NewMesh.m_nParamKeys = m_nParamKeys;
        x_memcpy( NewMesh.m_pParamKey, m_pParamKey, sizeof(f32)*NewMesh.m_nParamKeys);
    }
    else
    {
        NewMesh.m_pParamKey  = NULL;
        NewMesh.m_nParamKeys = 0;
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

        NewMesh.m_pTexture = new rawmesh2::texture[ NewMesh.m_nTextures ];
        ASSERT(NewMesh.m_pTexture);
        
        if (m_nTextures > 0)
            x_memcpy( NewMesh.m_pTexture, m_pTexture, sizeof(rawmesh2::texture)*m_nTextures );        
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
    NewMesh.m_pVertex = new rawmesh2::vertex[ NewMesh.m_nVertices ];
    ASSERT(NewMesh.m_pVertex);

    NewMesh.m_nFacets = nFacets;
    NewMesh.m_pFacet = new rawmesh2::facet[ NewMesh.m_nFacets ];
    ASSERT(NewMesh.m_pFacet );

    rawmesh2::vertex*    pVert     = NewMesh.m_pVertex;
    s32                 iVert     = 0;
    rawmesh2::facet*     pFacet    = NewMesh.m_pFacet;
    
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

xbool rawmesh2::IsolateSubmesh( const char* pMeshName, rawmesh2& NewMesh )
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

xbool rawmesh2::IsBoneUsed( s32 iBone )
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

void rawmesh2::CollapseNormals( radian ThresholdAngle )
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

    // Handle the errors
    x_catch_begin;

    if( pHash   ) delete []pHash;
    if( pTempV  ) delete []pTempV;
    pHash = 0;
    pTempV = 0;

    x_catch_end_ret;

    if ( pHash ) delete [] pHash;
    if ( pTempV ) delete [] pTempV;
}

//=========================================================================

void rawmesh2::DeleteBone          ( const char* pBoneName )
{
    s32 iBone = this->GetBoneIDFromName(pBoneName);
    if(iBone != -1)
        DeleteBone(iBone);
    return;
}

//=========================================================================

void rawmesh2::DeleteBone( s32 iBone )
{
    //x_DebugMsg("MESH: Deleting bone: '%s'\n", m_pBone[iBone].Name);
    s32 i,j;
    ASSERT( m_nBones > 1 );

    //
    // Allocate new bones and frames
    //
    s32 nNewBones = m_nBones-1;
    bone* pNewBone = new bone[ nNewBones ];
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

    // free current allocations
    delete[] m_pBone;

    m_nBones = nNewBones;
    m_pBone = pNewBone;
    //x_DebugMsg("MESH: After delete, m_nBones = %d\n", m_nBones);
}

//=========================================================================

s32 rawmesh2::GetBoneIDFromName( const char* pBoneName, xbool bAnywhere ) const
{
    s32 i;
    
    if( bAnywhere )
    {
        for( i = 0; i<m_nBones; i++ )
        {
            if( x_stristr( m_pBone[i].Name, pBoneName ) )
                return i;
        }
    }
    else
    {

        for( i = 0; i<m_nBones; i++ )
        {
            if( x_stricmp( m_pBone[i].Name, pBoneName ) == 0 )
                return i;
        }                
    }
                
    return -1;
}

//=========================================================================

s32 rawmesh2::GetRigidBodyIDFromName( const char* pRigidBodyName, xbool bAnywhere ) const
{
    s32 i;
    
    if( bAnywhere )
    {
        for( i = 0; i<m_nRigidBodies; i++ )
        {
            if( x_stristr( m_pRigidBodies[i].Name, pRigidBodyName ) )
                return i;
        }
    }
    else
    {
        for( i = 0; i<m_nRigidBodies; i++ )
        {
            if( x_stricmp( m_pRigidBodies[i].Name, pRigidBodyName ) == 0 )
                return i;
        }
    }
                        
    return -1;
}

//=========================================================================

void rawmesh2::ApplyNewSkeleton    ( rawanim& Skel )
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
        rawmesh2::vertex* pVertex = &m_pVertex[i];
        for(s32 iWeight = 0; iWeight < pVertex->nWeights; iWeight++)
        {
            rawmesh2::weight* pWeight = &pVertex->Weight[iWeight];

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
    m_pBone = new rawmesh2::bone[m_nBones];
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

    // Compute all bone related info
    ComputeBoneInfo() ;
}

//=========================================================================

void rawmesh2::ApplyNewSkeleton( const rawmesh2& Skel )
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
//=========================================================================
//=========================================================================
// CONVERT FROM RAWMESH TO RAWMESH2
//=========================================================================
//=========================================================================
//=========================================================================

void SetupMap( rawmesh2::map& M, rawmesh::tex_material& TM )
{
    x_memset( &M, 0, sizeof(rawmesh2::map) );

    M.FilterType    = rawmesh2::FILTER_TYPE_BILINEAR;
    M.RGBASource    = rawmesh2::RGBA_SOURCE_TYPE_RGB;
    M.UAddress      = rawmesh2::ADDRESS_TYPE_WRAP;
    M.VAddress      = rawmesh2::ADDRESS_TYPE_WRAP;
    M.nTextures     = 1;
    M.TextureFPS    = 0;
    M.iTexture      = TM.iTexture;
    M.iUVChannel    = TM.iChanel;

    switch( TM.FilterType )
    {
        case rawmesh::FILTER_POINT:  M.FilterType = rawmesh2::FILTER_TYPE_POINT; break;
        case rawmesh::FILTER_BILINEAR:  M.FilterType = rawmesh2::FILTER_TYPE_BILINEAR; break;
        case rawmesh::FILTER_TRILINEAR:  M.FilterType = rawmesh2::FILTER_TYPE_TRILINEAR; break;
        case rawmesh::FILTER_ANISOTROPIC:  M.FilterType = rawmesh2::FILTER_TYPE_POINT; break;
    }
    
    switch( TM.UAddress )
    {
        case rawmesh::TEXTURE_WRAP:  M.UAddress = rawmesh2::ADDRESS_TYPE_WRAP; break;
        case rawmesh::TEXTURE_CLAMP:  M.UAddress = rawmesh2::ADDRESS_TYPE_CLAMP; break;
        case rawmesh::TEXTURE_MIRROR:  M.UAddress = rawmesh2::ADDRESS_TYPE_WRAP; break;
    }

    switch( TM.VAddress )
    {
        case rawmesh::TEXTURE_WRAP:  M.VAddress = rawmesh2::ADDRESS_TYPE_WRAP; break;
        case rawmesh::TEXTURE_CLAMP:  M.VAddress = rawmesh2::ADDRESS_TYPE_CLAMP; break;
        case rawmesh::TEXTURE_MIRROR:  M.VAddress = rawmesh2::ADDRESS_TYPE_WRAP; break;
    }
}

//=========================================================================

static
void SetupMat_Default( rawmesh2::material& M2 )
{
    M2.Type                     = 0;
    M2.LightingType             = rawmesh2::LIGHTING_TYPE_STATIC_AND_DYNAMIC;
    M2.BlendType                = rawmesh2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmesh2::TINT_TYPE_FINAL_OUTPUT;
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

rawmesh2::lighting_type GetLightingType( rawmesh::material& M )
{
    switch( M.IllumType )
    {
    case rawmesh::ILLUM_TYPE_NONE:
        return rawmesh2::LIGHTING_TYPE_SELF_ILLUM;
    case rawmesh::ILLUM_TYPE_DYNAMIC_VERTEX:
        return rawmesh2::LIGHTING_TYPE_STATIC_AND_DYNAMIC;
    case rawmesh::ILLUM_TYPE_DYNAMIC_VERTEX_MONOCROM:
        return rawmesh2::LIGHTING_TYPE_STATIC_AND_DYNAMIC;
    case rawmesh::ILLUM_TYPE_LIGHTMAP:
        return rawmesh2::LIGHTING_TYPE_STATIC_AND_DYNAMIC;
    default:
        return rawmesh2::LIGHTING_TYPE_STATIC_AND_DYNAMIC;
    }
}

//=========================================================================

static
void SetupMat_Default( rawmesh2::material& M2, rawmesh::material& M )
{
    (void)M;

    M2.Type                     = 0;
    M2.LightingType             = GetLightingType( M );
    M2.BlendType                = rawmesh2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmesh2::TINT_TYPE_FINAL_OUTPUT;
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
void SetupMat_SolidColor( rawmesh2::material& M2, rawmesh::material& M )
{
    SetupMat_Default( M2 );

    M2.Type                     = 0;
    M2.LightingType             = GetLightingType( M );
    M2.BlendType                = rawmesh2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmesh2::TINT_TYPE_FINAL_OUTPUT;
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

    // There are some special traits with this material
    //
    // It can't be self-illuminated, since there is no texture.
    // -vertex colors must be used, so we alter the lighting type 
    //  if necessary
    if (M2.LightingType == rawmesh2::LIGHTING_TYPE_SELF_ILLUM)
        M2.LightingType = rawmesh2::LIGHTING_TYPE_STATIC;
}

//=========================================================================

static
void SetupMat_1Texture( rawmesh2::material& M2, rawmesh::material& M )
{
    rawmesh::tex_material* pBase = M.GetTexMap( rawmesh::TEX_TYPE_BASE1     );

    SetupMat_Default( M2 );

    M2.Type                     = 1;
    M2.LightingType             = GetLightingType( M );
    M2.BlendType                = rawmesh2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmesh2::TINT_TYPE_NONE;
    M2.bTwoSided                = M.bDoubleSide;
    M2.bPunchthrough            = FALSE;
    M2.bRandomizeAnim           = FALSE;

    M2.nMaps = 1;
    SetupMap( M2.Map[0], *pBase );
}

//=========================================================================

static
void SetupMat_1TextureLightmap( rawmesh2::material& M2, rawmesh::material& M )
{
    rawmesh::tex_material* pBase = M.GetTexMap( rawmesh::TEX_TYPE_BASE1 );
    rawmesh::tex_material* pLM   = M.GetTexMap( rawmesh::TEX_TYPE_SELF_ILLUM );

    SetupMat_Default( M2 );

    M2.Type                     = 7;
    M2.LightingType             = GetLightingType( M );
    M2.BlendType                = rawmesh2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmesh2::TINT_TYPE_NONE;
    M2.bTwoSided                = M.bDoubleSide;
    M2.bPunchthrough            = FALSE;
    M2.bRandomizeAnim           = FALSE;

    M2.nMaps = 2;
    SetupMap( M2.Map[0], *pBase );
    SetupMap( M2.Map[3], *pLM );
}

//=========================================================================

static
void SetupMat_1TextureOpacity( rawmesh2::material& M2, rawmesh::material& M )
{
    rawmesh::tex_material* pBase   = M.GetTexMap( rawmesh::TEX_TYPE_BASE1 );
    rawmesh::tex_material* pAlpha  = M.GetTexMap( rawmesh::TEX_TYPE_OPACITY_ALPHA );
    rawmesh::tex_material* pPunch  = M.GetTexMap( rawmesh::TEX_TYPE_OPACITY_PUNCH );
    ASSERT( pAlpha || pPunch );

    SetupMat_Default( M2 );

    M2.Type                     = 2;
    M2.LightingType             = GetLightingType( M );
    M2.BlendType                = rawmesh2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmesh2::TINT_TYPE_NONE;
    M2.bTwoSided                = M.bDoubleSide;
    M2.bPunchthrough            = (pPunch) ? (TRUE):(FALSE);
    M2.bRandomizeAnim           = FALSE;

    M2.nMaps = 2;
    SetupMap( M2.Map[0], *pBase );

    if( pPunch )
        SetupMap( M2.Map[4], *pPunch );
    else
        SetupMap( M2.Map[4], *pAlpha );

    M2.Map[4].RGBASource = rawmesh2::RGBA_SOURCE_TYPE_A;
}

//=========================================================================

static
void SetupMat_2TexturesBlendedTexture( rawmesh2::material& M2, rawmesh::material& M )
{
    rawmesh::tex_material* pBaseA  = M.GetTexMap( rawmesh::TEX_TYPE_BASE1 );
    rawmesh::tex_material* pBaseB  = M.GetTexMap( rawmesh::TEX_TYPE_BASE2 );
    rawmesh::tex_material* pMix    = M.GetTexMap( rawmesh::TEX_TYPE_BLEND_R );

    SetupMat_Default( M2 );

    M2.Type                     = 3;
    M2.LightingType             = GetLightingType( M );
    M2.BlendType                = rawmesh2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmesh2::TINT_TYPE_NONE;
    M2.bTwoSided                = M.bDoubleSide;
    M2.bPunchthrough            = FALSE;
    M2.bRandomizeAnim           = FALSE;

    M2.nMaps = 3;
    SetupMap( M2.Map[0], *pBaseA );
    SetupMap( M2.Map[1], *pBaseB );
    SetupMap( M2.Map[2], *pMix );

    M2.Map[2].RGBASource = rawmesh2::RGBA_SOURCE_TYPE_R;
}

//=========================================================================

static
void SetupMat_2TexturesBlendedTextureLightmap( rawmesh2::material& M2, rawmesh::material& M )
{
    rawmesh::tex_material* pBaseA  = M.GetTexMap( rawmesh::TEX_TYPE_BASE1 );
    rawmesh::tex_material* pBaseB  = M.GetTexMap( rawmesh::TEX_TYPE_BASE2 );
    rawmesh::tex_material* pMix    = M.GetTexMap( rawmesh::TEX_TYPE_BLEND_R );
    rawmesh::tex_material* pLM     = M.GetTexMap( rawmesh::TEX_TYPE_SELF_ILLUM );

    SetupMat_Default( M2 );

    M2.Type                     = 5;
    M2.LightingType             = GetLightingType( M );
    M2.BlendType                = rawmesh2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmesh2::TINT_TYPE_NONE;
    M2.bTwoSided                = M.bDoubleSide;
    M2.bPunchthrough            = FALSE;
    M2.bRandomizeAnim           = FALSE;
       

    M2.nMaps = 3;
    SetupMap( M2.Map[0], *pBaseA );
    SetupMap( M2.Map[1], *pBaseB );
    SetupMap( M2.Map[2], *pMix );
    SetupMap( M2.Map[3], *pLM );

    M2.Map[2].RGBASource = rawmesh2::RGBA_SOURCE_TYPE_R;
}

//=========================================================================

static
void SetupMat_2TexturesBlendedVertex( rawmesh2::material& M2, rawmesh::material& M )
{
    rawmesh::tex_material* pBaseA  = M.GetTexMap( rawmesh::TEX_TYPE_BASE1 );
    rawmesh::tex_material* pBaseB  = M.GetTexMap( rawmesh::TEX_TYPE_BASE2 );

    SetupMat_Default( M2 );

    M2.Type                     = 4;
    M2.LightingType             = GetLightingType( M );
    M2.BlendType                = rawmesh2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmesh2::TINT_TYPE_NONE;
    M2.bTwoSided                = M.bDoubleSide;
    M2.bPunchthrough            = FALSE;
    M2.bRandomizeAnim           = FALSE;

    M2.nMaps = 2;
    SetupMap( M2.Map[0], *pBaseA );
    SetupMap( M2.Map[1], *pBaseB );
}

//=========================================================================

static
void SetupMat_2TexturesBlendedVertexWithOpacity( rawmesh2::material& M2, rawmesh::material& M )
{
    rawmesh::tex_material* pBaseA  = M.GetTexMap( rawmesh::TEX_TYPE_BASE1 );
    rawmesh::tex_material* pBaseB  = M.GetTexMap( rawmesh::TEX_TYPE_BASE2 );
    rawmesh::tex_material* pAlpha  = M.GetTexMap( rawmesh::TEX_TYPE_OPACITY_ALPHA );
    rawmesh::tex_material* pPunch  = M.GetTexMap( rawmesh::TEX_TYPE_OPACITY_PUNCH );
    ASSERT( pAlpha || pPunch );


    SetupMat_Default( M2 );

    M2.Type                     = 8;
    M2.LightingType             = GetLightingType( M );
    M2.BlendType                = rawmesh2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmesh2::TINT_TYPE_NONE;
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

    M2.Map[4].RGBASource = rawmesh2::RGBA_SOURCE_TYPE_A;
}

//=========================================================================

static
void SetupMat_WetMaterial( rawmesh2::material& M2, rawmesh::material& M )
{
    rawmesh::tex_material* pBaseA  = M.GetTexMap( rawmesh::TEX_TYPE_BASE1 );
    rawmesh::tex_material* pBaseB  = M.GetTexMap( rawmesh::TEX_TYPE_BASE2 );
    rawmesh::tex_material* pGloss  = M.GetTexMap( rawmesh::TEX_TYPE_INTENSITY );
    rawmesh::tex_material* pWater  = M.GetTexMap( rawmesh::TEX_TYPE_ANISOTROPIC );

    SetupMat_Default( M2 );

    M2.Type                     = 9;
    M2.LightingType             = GetLightingType( M );
    M2.BlendType                = rawmesh2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmesh2::TINT_TYPE_NONE;
    M2.bTwoSided                = M.bDoubleSide;
    M2.bPunchthrough            = FALSE;
    M2.bRandomizeAnim           = FALSE;

    M2.Constants[0].Current[0]  = 0.45f;
    M2.Constants[0].nKeys       = 1;
    M2.Constants[0].nParamsPerKey = 1; 

    M2.Constants[1].Current[0]  = 1.0f / 8.0f;      // 1 tile per 8 seconds
    M2.Constants[1].nKeys       = 1;
    M2.Constants[1].nParamsPerKey = 1; 


    M2.nMaps = 4;
    SetupMap( M2.Map[0], *pBaseA );
    SetupMap( M2.Map[1], *pBaseB );
    SetupMap( M2.Map[5], *pGloss );
    SetupMap( M2.Map[6], *pWater );

    M2.Map[5].RGBASource = rawmesh2::RGBA_SOURCE_TYPE_R;
}
//=========================================================================

static
void SetupMat_SingleBaseEnviroMask( rawmesh2::material& M2, rawmesh::material& M )
{
    rawmesh::tex_material* pBaseA  = M.GetTexMap( rawmesh::TEX_TYPE_BASE1 );
    rawmesh::tex_material* pGloss  = M.GetTexMap( rawmesh::TEX_TYPE_INTENSITY );
    rawmesh::tex_material* pEnviro = M.GetTexMap( rawmesh::TEX_TYPE_ENVIROMENT );

    SetupMat_Default( M2 );

    M2.Type                     = 10;
    M2.LightingType             = GetLightingType( M );
    M2.BlendType                = rawmesh2::BLEND_TYPE_OVERWRITE;
    M2.TintType                 = rawmesh2::TINT_TYPE_NONE;
    M2.bTwoSided                = M.bDoubleSide;
    M2.bPunchthrough            = FALSE;
    M2.bRandomizeAnim           = FALSE;

    M2.Constants[0].Current[0]      = 0.5f;
    M2.Constants[0].nKeys           = 1;
    M2.Constants[0].nParamsPerKey   = 1; 


    M2.nMaps = 3;
    SetupMap( M2.Map[0], *pBaseA );
    SetupMap( M2.Map[5], *pGloss );
    SetupMap( M2.Map[6], *pEnviro );

    M2.Map[5].RGBASource = rawmesh2::RGBA_SOURCE_TYPE_R;
}


//=========================================================================

void rawmesh2::BuildFromRM( rawmesh& RM )
{
    Kill();

    s32     i,j;

    //
    // Move the easy stuff over
    //

    if (RM.m_nBones > 0)
    {
        m_pBone = new bone[ RM.m_nBones ];
        m_nBones = RM.m_nBones;
        x_memcpy( m_pBone, RM.m_pBone, sizeof(bone)*m_nBones );
    }
    else
        m_pBone = NULL;
    
    RMESH_SANITY;

    m_pVertex = new vertex[ RM.m_nVertices ];
    m_nVertices = RM.m_nVertices;
    m_nVFrames = RM.m_nVFrames;
    for (i=0;i<m_nVertices;i++)
    {
        m_pVertex[i].Position       = RM.m_pVertex[i].Position;
        m_pVertex[i].iFrame         = RM.m_pVertex[i].iFrame;
        m_pVertex[i].nWeights       = RM.m_pVertex[i].nWeights;
        m_pVertex[i].nNormals       = RM.m_pVertex[i].nNormals;
        m_pVertex[i].nUVs           = RM.m_pVertex[i].nUVs;
        m_pVertex[i].nColors        = RM.m_pVertex[i].nColors;
        m_pVertex[i].Position       = RM.m_pVertex[i].Position;

        for (j=0;j<m_pVertex[i].nUVs;j++)
            m_pVertex[i].UV[j] = RM.m_pVertex[i].UV[j];

        for (j=0;j<m_pVertex[i].nNormals;j++)
            m_pVertex[i].Normal[j] = RM.m_pVertex[i].Normal[j];

        for (j=0;j<m_pVertex[i].nColors;j++)
        {
            m_pVertex[i].Color[j].R = 255;
            m_pVertex[i].Color[j].G = 255;
            m_pVertex[i].Color[j].B = 255;

            m_pVertex[i].Color[j].A = RM.m_pVertex[i].Color[j].R;
        }

        for (j=0;j<m_pVertex[i].nWeights;j++)
        {
            m_pVertex[i].Weight[j].iBone  = RM.m_pVertex[i].Weight[j].iBone;
            m_pVertex[i].Weight[j].Weight = RM.m_pVertex[i].Weight[j].Weight;
        }
    }    
    
    RMESH_SANITY

    m_pFacet = new facet[ RM.m_nFacets ];
    m_nFacets = RM.m_nFacets;
    for (i=0;i<m_nFacets;i++)
    {
        m_pFacet[i].iMesh       = RM.m_pFacet[i].iMesh;
        m_pFacet[i].nVertices   = RM.m_pFacet[i].nVertices;
        m_pFacet[i].iMaterial   = RM.m_pFacet[i].iMaterial;
        m_pFacet[i].Plane       = RM.m_pFacet[i].Plane;

        ASSERT( m_pFacet[i].nVertices < FACET_MAX_VERTICES );

        for (j=0;j<m_pFacet[i].nVertices;j++)
            m_pFacet[i].iVertex[j] = RM.m_pFacet[i].iVertex[j];
    }
    
    RMESH_SANITY

    m_nTextures = RM.m_nTextures;
    if (m_nTextures > 0)
    {
        m_pTexture = new texture[ RM.m_nTextures ];
        x_memcpy( m_pTexture, RM.m_pTexture, sizeof(texture)*m_nTextures );
        RMESH_SANITY
    }
    else
        m_pTexture = NULL;

    m_nSubMeshs = RM.m_nSubMeshs;
    if (m_nSubMeshs > 0)
    {
        m_pSubMesh = new sub_mesh[ RM.m_nSubMeshs ];
        x_memcpy( m_pSubMesh, RM.m_pSubMesh, sizeof(sub_mesh)*m_nSubMeshs );
        RMESH_SANITY
    }
    else
        m_pSubMesh = NULL;


    //
    //  Determine the materials that we will need
    //
    m_nMaterials = RM.m_nMaterials;


    // Allocate materials
    m_pMaterial = new material[ m_nMaterials ];
    x_memset( m_pMaterial, 0, sizeof(material)*m_nMaterials );
    for( s32 m=0; m<m_nMaterials; m++ )
    {
        RMESH_SANITY
        // Get easy access to the two materials
        rawmesh2::material& M2 =    m_pMaterial[ m ];
        rawmesh::material& M   = RM.m_pMaterial[ m ];

        // Clear new material and copy over name
        x_memset( &M2, 0, sizeof(rawmesh2::material) );
        x_strcpy( M2.Name, M.Name );

        // HACKITY HACK HACK
        // This is an alpha based material if the blend mode is set to subtract
        
        if ( M.CompType == rawmesh::COMPOSITION_TYPE_SUB)
            M2.bHasVariableVertAlpha = TRUE;
        else
            M2.bHasVariableVertAlpha = FALSE;
       
        //
        // Try to recognize original material intent and call correct
        // conversion
        //

        rawmesh::tex_material* pBase[4];
        rawmesh::tex_material* pMix[3];
        rawmesh::tex_material* pSelfIllum;
        rawmesh::tex_material* pOpacity[2];
        rawmesh::tex_material* pIntensity;
        rawmesh::tex_material* pAniso;
        rawmesh::tex_material* pEnviro;
        rawmesh::tex_material* pSpecular;

        pBase[0]     = M.GetTexMap( rawmesh::TEX_TYPE_BASE1         );
        pBase[1]     = M.GetTexMap( rawmesh::TEX_TYPE_BASE2         );
        pBase[2]     = M.GetTexMap( rawmesh::TEX_TYPE_BASE3         );
        pBase[3]     = M.GetTexMap( rawmesh::TEX_TYPE_BASE4         );
        pMix[0]      = M.GetTexMap( rawmesh::TEX_TYPE_BLEND_R       );
        pMix[1]      = M.GetTexMap( rawmesh::TEX_TYPE_BLEND_RG      );
        pMix[2]      = M.GetTexMap( rawmesh::TEX_TYPE_BLEND_RGB     );
        pOpacity[0]  = M.GetTexMap( rawmesh::TEX_TYPE_OPACITY_ALPHA );
        pOpacity[1]  = M.GetTexMap( rawmesh::TEX_TYPE_OPACITY_PUNCH );
        pSelfIllum   = M.GetTexMap( rawmesh::TEX_TYPE_SELF_ILLUM    );
        pAniso       = M.GetTexMap( rawmesh::TEX_TYPE_ANISOTROPIC   );
        pIntensity   = M.GetTexMap( rawmesh::TEX_TYPE_INTENSITY     );
        pEnviro      = M.GetTexMap( rawmesh::TEX_TYPE_ENVIROMENT    );
        pSpecular    = M.GetTexMap( rawmesh::TEX_TYPE_SPECULAR      );

        xbool bOpacity = (pOpacity[0] || pOpacity[1]);

        if( pBase[0] && !pBase[1] && pEnviro && pIntensity && !pMix[0] )
        {
            // Single base with enviro and mask
            x_DebugMsg("MAT: Single Base with Enviro and mask<%s>\n",M.Name);
            SetupMat_SingleBaseEnviroMask(M2,M); //
        }
        else
        if( pBase[0] && pBase[1] && pAniso && pIntensity && pMix[0] &&(pMix[0]->ChanelType == rawmesh::TEX_MATERIAL_VERTEX))
        {
            // Wet material for roastmutton
            x_DebugMsg("MAT: Wet material <%s>\n",M.Name);
            SetupMat_WetMaterial(M2,M); //
        }
        else
        if( pMix[0] && pBase[0] && pBase[1] && (pMix[0]->ChanelType == rawmesh::TEX_MATERIAL_VERTEX) && bOpacity )
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
        if( pMix[0] && pBase[0] && pBase[1] && (pMix[0]->ChanelType == rawmesh::TEX_MATERIAL_VERTEX) && (!bOpacity) )
        {
            // Two base textures blended using vertex alpha
            x_DebugMsg("MAT: 2 Base blended using vert alpha <%s>\n",M.Name);
            SetupMat_2TexturesBlendedVertex(M2,M); //
        }
        else
        if( pMix[0] && pBase[0] && pBase[1] && (pMix[0]->ChanelType == rawmesh::TEX_MATERIAL_TEXTURE) && (!bOpacity) && (!pSelfIllum) )
        {
            // Two base textures blended using blend texture
            x_DebugMsg("MAT: 2 Base blended using blend texture <%s>\n",M.Name);
            SetupMat_2TexturesBlendedTexture(M2,M); //
        }
        else 
        if( pMix[0] && pBase[0] && pBase[1] && (pMix[0]->ChanelType == rawmesh::TEX_MATERIAL_TEXTURE) && (!bOpacity) && (pSelfIllum) )
        {
            // Two base textures blended using blend texture with final lightmap (in self illum slot of old material)
            x_DebugMsg("MAT: 2 Base blended using blend texture, * lightmap<%s>\n",M.Name);
            SetupMat_2TexturesBlendedTextureLightmap(M2,M); //
        }
        else
        if( pBase[0] && (pBase[0]->ChanelType == rawmesh::TEX_MATERIAL_TEXTURE) && bOpacity  )
        {
            // Single texture with opacity
            x_DebugMsg("MAT: 1 Base with Opacity <%s>\n",M.Name);
            SetupMat_1TextureOpacity(M2,M); //
        }
        else
        if( pBase[0] && (pBase[0]->ChanelType == rawmesh::TEX_MATERIAL_TEXTURE) && pSelfIllum && (!bOpacity)  )
        {
            // Single texture with lightmap applied
            x_DebugMsg("MAT: 1 Base with Lightmap <%s>\n",M.Name);
            SetupMat_1TextureLightmap(M2,M); //
        }
        else
        if( pBase[0] && (pBase[0]->ChanelType == rawmesh::TEX_MATERIAL_TEXTURE) && (!bOpacity) )
        {
            // Plain old single-texture
            x_DebugMsg("MAT: 1 Base <%s>\n",M.Name);
            SetupMat_1Texture(M2,M); //
        }
        else
        if( pBase[0] && (pBase[0]->ChanelType == rawmesh::TEX_MATERIAL_VERTEX) && (!bOpacity)  )
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
        RMESH_SANITY
    }
}

//=========================================================================

void rawmesh2::PrintHierarchy( void ) const
{
    // Show hierarchy in debug window
    x_printf("\nHierarchy is %d Bones:\n", m_nBones) ;
    for( s32 i = 0; i < m_nBones; i++ )
    {
        // Print bone index
        x_printf( "%3d: ", i );

        // Print indent
        s32 iParent = m_pBone[i].iParent ;
        while(iParent != -1)
        {
            x_printf(" ") ;
            iParent = m_pBone[iParent].iParent ;
        }

        // Print name
        iParent = m_pBone[i].iParent ;
        ASSERT(iParent < i) ;
        if (iParent != -1)
            x_printf("\"%s\" (Parent=\"%s\", LODGroup=%d)\n", 
            m_pBone[i].Name, 
            m_pBone[iParent].Name,
            m_pBone[i].LODGroup );
        else
        {
            x_printf("\"%s\" (LODGroup=%d)\n", 
                m_pBone[i].Name,
                m_pBone[i].LODGroup );
        }            
    }
    x_printf("\n") ;
}

//=========================================================================

void rawmesh2::PrintRigidBodies( void ) const
{
    // Show rigid bodies in debug window
    x_printf( "\n%d Rigid bodies found:\n", m_nRigidBodies ) ;
    for( s32 i = 0; i < m_nRigidBodies; i++ )
    {
        // Print rigid body index
        x_printf( "%3d: ", i );

        // Print indent
        s32 iParent = m_pRigidBodies[i].iParent ;
        while(iParent != -1)
        {
            x_printf(" ") ;
            iParent = m_pRigidBodies[iParent].iParent ;
        }

        // Print name
        iParent = m_pRigidBodies[i].iParent ;
        ASSERT(iParent < i) ;
        if (iParent != -1)
            x_printf("\"%s\" (Parent=\"%s\")\n", m_pRigidBodies[i].Name, m_pRigidBodies[iParent].Name) ;
        else
            x_printf("\"%s\"\n", m_pRigidBodies[i].Name) ;
    }
    x_printf("\n") ;
}

//=========================================================================

