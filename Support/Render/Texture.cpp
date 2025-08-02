#include "Texture.hpp"
#include "x_array.hpp"
#include "e_VRAM.hpp"

#if !( defined(TARGET_PS2) && defined(CONFIG_RETAIL) )
xbool      g_OutputTGA  = FALSE;
static s32 s_Count      = 0;
#endif // !( defined(TARGET_PS2) && defined(CONFIG_RETAIL) )

static s32 s_nTexture   = 0;
static s32 s_TextureMem = 0;

//=============================================================================
//    TYPES
//=============================================================================

static struct texture_loader : public rsc_loader
{
    texture_loader( void ) : rsc_loader( "TEXTURE", ".xbmp" ) {}

    //-------------------------------------------------------------------------

    virtual void* PreLoad( X_FILE* FP )
    {
        MEMORY_OWNER( "TEXTURE DATA" );
        texture* pTexture = new texture;
        ASSERT( pTexture );

        pTexture->m_Bitmap.Load( FP );

        // sanity check
        switch ( pTexture->m_Bitmap.GetFormat() )
        {
        #ifdef TARGET_XBOX
        case xbitmap::FMT_P8_RGBA_8888:
        case xbitmap::FMT_32_ARGB_8888:
        case xbitmap::FMT_32_URGB_8888:
        case xbitmap::FMT_16_ARGB_4444:
        case xbitmap::FMT_16_ARGB_1555:
        case xbitmap::FMT_16_URGB_1555:
        case xbitmap::FMT_16_RGB_565:
        case xbitmap::FMT_DXT1:
        case xbitmap::FMT_DXT3:
        case xbitmap::FMT_DXT5:
        case xbitmap::FMT_A8:
            break;
        #endif

        #ifdef TARGET_PS2
        case xbitmap::FMT_32_ABGR_8888:
        case xbitmap::FMT_32_UBGR_8888:
        case xbitmap::FMT_16_ABGR_1555:
        case xbitmap::FMT_16_UBGR_1555:
        case xbitmap::FMT_P8_ABGR_8888:
        case xbitmap::FMT_P8_UBGR_8888:
        case xbitmap::FMT_P4_ABGR_8888:
        case xbitmap::FMT_P4_UBGR_8888:
            break;
        #endif

        #ifdef TARGET_PC
        case xbitmap::FMT_32_ARGB_8888:
        case xbitmap::FMT_32_URGB_8888:
        case xbitmap::FMT_16_ARGB_4444:
        case xbitmap::FMT_16_ARGB_1555:
        case xbitmap::FMT_16_URGB_1555:
        case xbitmap::FMT_16_RGB_565:
        case xbitmap::FMT_DXT1:
        case xbitmap::FMT_DXT3:
        case xbitmap::FMT_DXT5:
            break;
        #endif

        default:
            x_throw( "Invalid texture format." );
            delete pTexture;
            return NULL;
        }

        s_TextureMem += pTexture->m_Bitmap.GetDataSize();
        s_nTexture++;

        return( pTexture );
    }

    //-------------------------------------------------------------------------

    virtual void* Resolve( void* pData ) 
    {
        texture* pTexture = (texture*)pData;

        vram_Register( pTexture->m_Bitmap );

#if !( defined(TARGET_PS2) && defined(CONFIG_RETAIL) )

        if( g_OutputTGA == TRUE )
        {
            pTexture->m_Bitmap.SaveTGA( xfs( "C:\\GameData\\A51\\Test\\%03d.tga", s_Count++ ) );
        }

#endif // !( defined(TARGET_PS2) && defined(CONFIG_RETAIL) )

        return( pTexture );
    }

    //-------------------------------------------------------------------------

    virtual void Unload( void* pData )
    {
        texture* pTexture = (texture*)pData;

        s_TextureMem -= pTexture->m_Bitmap.GetDataSize();
        s_nTexture--;

        vram_Unregister( pTexture->m_Bitmap );

        delete pTexture;
    }

} s_Texture_Loader;

//=============================================================================

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
            s_TextureMem += pCubemap->m_Bitmap[i].GetDataSize();
            s_nTexture++;
        }

        return( pCubemap );
    }

    //-------------------------------------------------------------------------

    virtual void* Resolve( void* pData ) 
    {
        cubemap* pCubemap = (cubemap*)pData;

        #ifdef TARGET_XBOX
        {
            D3DFORMAT Format;
            switch( pCubemap->m_Bitmap[0].GetFormat( ))
            {
                case xbitmap::FMT_32_ARGB_8888: Format = D3DFMT_A8R8G8B8; break;
                case xbitmap::FMT_32_URGB_8888: Format = D3DFMT_X8R8G8B8; break;
                case xbitmap::FMT_16_ARGB_4444: Format = D3DFMT_A4R4G4B4; break;
                case xbitmap::FMT_16_ARGB_1555: Format = D3DFMT_A1R5G5B5; break;
                case xbitmap::FMT_16_URGB_1555: Format = D3DFMT_X1R5G5B5; break;
                case xbitmap::FMT_16_RGB_565  : Format = D3DFMT_R5G6B5  ; break;
                case xbitmap::FMT_DXT1        : Format = D3DFMT_DXT1    ; break;
                case xbitmap::FMT_DXT3        : Format = D3DFMT_DXT3    ; break;
                case xbitmap::FMT_DXT5        : Format = D3DFMT_DXT5    ; break;
                case xbitmap::FMT_A8          : Format = D3DFMT_A8      ; break;

                default:
                    ASSERT(0);
                    break;
            }
            pCubemap->m_hTexture = g_TextureFactory.CreateCubeMap( m_pFileName,Format,pCubemap->m_Bitmap );
        }
        #else
        for ( s32 i = 0; i < 6; i++ )
            vram_Register( pCubemap->m_Bitmap[i] );
        #endif

        return( pCubemap );
    }

    //-------------------------------------------------------------------------

    virtual void Unload( void* pData )
    {
        cubemap* pCubemap = (cubemap*)pData;

        #ifdef TARGET_XBOX
        delete pCubemap->m_hTexture;
        #else
        for ( s32 i = 0; i < 6; i++ )
        {
            s_TextureMem -= pCubemap->m_Bitmap[i].GetDataSize();
            s_nTexture--;

            vram_Unregister( pCubemap->m_Bitmap[i] );
        }
        #endif

        delete pCubemap;
    }

} s_Cubemap_Loader;

//=============================================================================

texture::texture( void )
{
}

//=============================================================================

void texture::GetStats( s32* pNumTextureLoaded, s32* pTextureMemorySize )
{
    *pNumTextureLoaded  = s_nTexture;
    *pTextureMemorySize = s_TextureMem;
}

//=============================================================================

cubemap::cubemap( void )
{
}

//=============================================================================

