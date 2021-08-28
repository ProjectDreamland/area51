
#ifndef GEOM_HPP
#define GEOM_HPP

//=========================================================================
// INCLUDES
//=========================================================================
#include "x_files.hpp"
#include "Auxiliary\MiscUtils\Fileio.hpp"

//=========================================================================
// GEOM
//=========================================================================
struct geom
{
//-------------------------------------------------------------------------

    struct mesh                 // Not used right now
    {
        u64     SMFlags;
        f32     MinDistance;
        bbox    BBox;

        void FileIO( fileio& File )
        {
            File.Static ( SMFlags );        
            File.Static ( MinDistance );        
            File.Static ( BBox );        
            
//            File.Dynamic( pList, 4 );
        }


    };

    struct submesh              // Not used right now
    {
        s32     nDLists;
        s32     iDList;

        void FileIO( fileio& File )
        {
            File.Static ( nDLists );        
            File.Static ( iDList  );        
        }

    };

    struct vertex
    {
        vector3 Pos;
        vector3 Normal;
        vector2 UV;
        xcolor  Color;

        void FileIO( fileio& File )
        {
            File.Static ( Pos    );        
            File.Static ( Normal );                    
            File.Static ( UV     );        
            File.Static ( Color  );        
        }
    };

    struct material             // Not use right now
    {
        s32 nTextures;
        s32 iTexture;

        void FileIO( fileio& File )
        {
            File.Static ( nTextures );        
            File.Static ( iTexture  );                    
        }

    };

    struct texture              // Not Use right now
    {
        char FileName[256];          

        void FileIO( fileio& File )
        {
            File.Static( FileName, 256 );                    
        }        
    };

    struct dlist_pc
    {
        s32     iMaterial;      // Not use right now

        s32     nVerts;
        s32     iVert;

        s32     nIndices;
        s32     iIndices;

        void FileIO( fileio& File )
        {
            File.Static ( iMaterial );                    
            File.Static ( nVerts    );                    
            File.Static ( iVert     );                    
            File.Static ( nIndices  );                    
            File.Static ( iIndices  );                    
        }
    };


//-------------------------------------------------------------------------
    void FileIO( fileio& File )
    {
        ASSERT( sizeof(platform) == sizeof(s32) );
        //File.Static( *(u32*)m_Platfrom );                    

        File.Static( m_nVerts );                    
        File.Static( m_pVertex, m_nVerts );                    
        File.Static( m_nIndices );                    
        File.Static( m_pIndex, m_nIndices );                    
        File.Static( m_nDLists );                    
        File.Static( m_pDList, m_nDLists );                    
        File.Static( m_nMeshes );                    
        File.Static( m_pMesh, m_nMeshes );                    

        File.Static( m_nSubMeshs );                    
        File.Static( m_pSubMesh, m_nSubMeshs );                    
        File.Static( m_nMaterials );                    
        File.Static( m_pMaterial, m_nMaterials );                    

        File.Static( m_nTextures );                    
        File.Static( m_pTexture, m_nTextures );                    
    }

    platform    m_Platfrom;

    s32         m_nVerts;
    vertex*     m_pVertex;

    s32         m_nIndices;
    u16*        m_pIndex;

    s32         m_nDLists;
    dlist_pc*   m_pDList;

    s32         m_nMeshes;
    mesh*       m_pMesh;

    s32         m_nSubMeshs;
    submesh*    m_pSubMesh;

    s32         m_nMaterials;
    material*   m_pMaterial;

    s32         m_nTextures;
    texture*    m_pTexture;    

    
};

//=========================================================================
// END
//=========================================================================
#endif
