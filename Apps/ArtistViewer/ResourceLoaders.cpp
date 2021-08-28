//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "Render\Render.hpp"
#include "Render\RigidGeom.hpp"
#include "Render\SkinGeom.hpp"
#include "Loco\Loco.hpp"
#include "Animation\AnimPlayer.hpp"
#include "Config.hpp"


//==============================================================================
// LOADERS FOR PC AND PS2!
//==============================================================================

//==============================================================================

static struct texture_loader : public rsc_loader
{
    texture_loader( void ) : rsc_loader( "TEXTURE", ".xbmp" ) {}

	//-------------------------------------------------------------------------

    virtual void* PreLoad( X_FILE* FP )
    {
        texture* pTexture = new texture;
        ASSERT( pTexture );

        pTexture->m_Bitmap.Load( FP );
        
        return( pTexture );
    }

	//-------------------------------------------------------------------------

    virtual void* Resolve( void* pData ) 
    {
        texture* pTexture = (texture*)pData;

        vram_Register( pTexture->m_Bitmap );

        return( pTexture );
    }

	//-------------------------------------------------------------------------

    virtual void Unload( void* pData )
    {
        texture* pTexture = (texture*)pData;

        vram_Unregister( pTexture->m_Bitmap );

        delete pTexture;
    }

} s_Texture_Loader;

//==============================================================================

static struct cubemap_loader : public rsc_loader
{
    cubemap_loader( void ) : rsc_loader( "CUBEMAP", ".envmap" ) {}

	//-------------------------------------------------------------------------

    virtual void* PreLoad( X_FILE* FP )
    {
        cubemap* pCubemap = new cubemap;
        ASSERT( pCubemap );

        for ( s32 i = 0; i < 6; i++ )
        {
            pCubemap->m_Bitmap[i].Load( FP );
        }

        return( pCubemap );
    }

	//-------------------------------------------------------------------------

    virtual void* Resolve( void* pData ) 
    {
        cubemap* pCubemap = (cubemap*)pData;

        for ( s32 i = 0; i < 6; i++ )
            vram_Register( pCubemap->m_Bitmap[i] );

        return( pCubemap );
    }

	//-------------------------------------------------------------------------

    virtual void Unload( void* pData )
    {
        cubemap* pCubemap = (cubemap*)pData;

        for ( s32 i = 0; i < 6; i++ )
        {
            vram_Unregister( pCubemap->m_Bitmap[i] );
        }

        delete pCubemap;
    }

} s_Cubemap_Loader;

//==============================================================================

static struct skin_loader : public rsc_loader
{
    //-------------------------------------------------------------------------
    
    skin_loader( void ) : rsc_loader( "SKIN GEOM", ".skingeom" ) {}

    //-------------------------------------------------------------------------
    
    virtual void* PreLoad ( X_FILE* FP )
    {
        fileio File;
        return( File.PreLoad( FP ) );
    }

    //-------------------------------------------------------------------------
    
    virtual void* Resolve ( void* pData ) 
    {
        fileio      File;
        skin_geom* pSkinGeom = NULL;

        File.Resolved( (fileio::resolve*)pData, pSkinGeom );

        return( pSkinGeom );
    }

    //-------------------------------------------------------------------------
    
    virtual void Unload( void* pData )
    {
        skin_geom* pSkinGeom = (skin_geom*)pData;
        ASSERT( pSkinGeom );
    
        delete pSkinGeom;
    }

} s_Skin_Geom_Loader;

//==============================================================================

static struct rigid_loader : public rsc_loader
{
    //-------------------------------------------------------------------------
    
    rigid_loader( void ) : rsc_loader( "RIGID GEOM", ".rigidgeom" ) {}

    //-------------------------------------------------------------------------
    
    virtual void* PreLoad ( X_FILE* FP )
    {
        fileio File;
        return( File.PreLoad( FP ) );
    }

    //-------------------------------------------------------------------------
    
    virtual void* Resolve ( void* pData ) 
    {
        fileio      File;
        rigid_geom* pRigidGeom = NULL;

        File.Resolved( (fileio::resolve*)pData, pRigidGeom );

        return( pRigidGeom );
    }

    //-------------------------------------------------------------------------
    
    virtual void Unload( void* pData )
    {
        rigid_geom* pRigidGeom = (rigid_geom*)pData;
        ASSERT( pRigidGeom );
    
        delete pRigidGeom;
    }

} s_Rigid_Geom_Loader;

//==============================================================================



