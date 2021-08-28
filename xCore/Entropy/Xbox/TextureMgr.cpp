#include "texturemgr.hpp"
#include "..\Auxiliary\Bitmap\aux_Bitmap.hpp"
#include <e_ScratchMem.hpp>
#include "x_threads.hpp"

#ifdef TARGET_XBOX
#   include "xbox_private.hpp"
#endif

// Remember that you need at least 5MB in texture RAM (slideshows)
#ifndef CONFIG_RETAIL
    u32 DXT5_Allocs = 0;
    u32 DXT3_Allocs = 0;
    u32 DXT1_Allocs = 0;
#endif

heap::basic texture_factory::m_Allocator[];

#if 0
#define ENTER_CRIT EnterCriticalSection( &s_Crit );
#define LEAVE_CRIT LeaveCriticalSection( &s_Crit );
#else
#define ENTER_CRIT
#define LEAVE_CRIT
#endif

static CRITICAL_SECTION s_Crit;



//==============================================================================
//==============================================================================
//==============================================================================
//  XBOX Texture Memory Manager
//==============================================================================
//==============================================================================
//==============================================================================



//=============================================================================

bool PowerOfTwo( s32 W )
{
    bool bPowOf2 = false;
    {
        switch( W )
        {
            case 1024:
            case  512:
            case  256:
            case  128:
            case   64:
            case   32:
            case   16:
            case    8:
            case    4:
                bPowOf2 = true;
                break;
        }
    }
    return bPowOf2;
}

//=============================================================================

void SanityCheckTNV( void )
{
#if 0//ndef CONFIG_RETAIL
    u32 TotalSize = g_TextureFactory.GetGeneralPool().GetSize();
    u32 BotSize   = g_TextureFactory.GetGeneralPool().GetUsed();
    u32 TopSize   =    g_VertFactory.GetGeneralPool().GetUsed();

    u32 BotPtr = (u32)g_TextureFactory.GetGeneralPool().m_pCur;
    u32 TopPtr = (u32)   g_VertFactory.GetGeneralPool().m_pCur;

    if( ((BotSize+TopSize) >= TotalSize) && (BotPtr < TopPtr))
    {
        D3D__pDevice->Clear( 0,0,D3DCLEAR_TARGET,D3DCOLOR_RGBA( 127,127,0,0),0.0f,0 );
        D3D__pDevice->Present( 0,0,0,0 );
    __asm int 3
    __asm int 3
    }
#endif
}

//=============================================================================

void texture_factory::Init( void )
{
    InitializeCriticalSection( &s_Crit );
}

//=============================================================================

void texture_factory::Kill( void )
{
    DeleteCriticalSection( &s_Crit );
}

//=============================================================================

static s32 s_PoolIndices[3]={ kPOOL_GENERAL,kPOOL_TILED,kPOOL_TEMP };
static s32 s_Ghosting = FALSE;

void texture_factory::GhostGeneralIntoTemp( xbool bGhosting )
{
    g_Texture.Clear(0);
    g_Texture.Clear(1);
    g_Texture.Clear(2);
    g_Texture.Clear(3);

    ENTER_CRIT
    {
        static u32 EntryCount = 0;

        if( bGhosting )
        {
            if( !EntryCount )
            {
                // Point temp pool at end of general pool *********************

                u8* Ptr = (u8*)m_Allocator[kPOOL_GENERAL].GetBase();
                u32 Len = (u32)m_Allocator[kPOOL_GENERAL].GetSize();
                Ptr += Len;

                m_Allocator[kPOOL_TEMP].basic::basic(
                    Ptr,
                    D3DTEXTURE_ALIGNMENT,
                    1048576*4,
                    heap::DIR_REVERSE
                );

                // Swap general and temp pools ********************************

                s_Ghosting = bGhosting;
                s_PoolIndices[2] = kPOOL_GENERAL;
                s_PoolIndices[0] = kPOOL_TEMP;
            }
            EntryCount++;
        }
        else if( !(--EntryCount) )
        {
            s_Ghosting = bGhosting;
            m_Allocator[kPOOL_TEMP].basic::~basic();
                s_PoolIndices[0] = kPOOL_GENERAL;
                s_PoolIndices[2] = kPOOL_TEMP;
        }
    }
    LEAVE_CRIT
}

//=============================================================================

texture_factory::handle texture_factory::GetHandleFromPointer( void* pBuffer )
{
    return NULL;
}

//=============================================================================

texture_factory::handle texture_factory::Create( const char* pResourceName,u32 Pitch,u32 XRes,u32 YRes,D3DFORMAT Format,s32 MemoryPool )
{
    handle Result = NULL;

    ENTER_CRIT
    {
        SanityCheckTNV();

        // Allocates header *******************************************************

        handle Handle = new buffer;

        // Populate struct ********************************************************

        if( (Format >= D3DFMT_LIN_A1R5G5B5) && (Format <= D3DFMT_LIN_F16) )
            Handle->m_Linear = u32( Format );
        if( !Pitch )
            m_Pitch = XGBytesPerPixelFromFormat( Format )*XRes;
        else
            m_Pitch = Pitch;

        XGSetTextureHeader( XRes,YRes,1,0,Format,0,Handle,0,Pitch );

        // Allocate texture buffer ************************************************

        u32 Size = m_Pitch*YRes;
        u8* Data = (u8*)m_Allocator[s_PoolIndices[MemoryPool]].Alloc( pResourceName,Size );
        SanityCheckTNV();

        // Align to 128 byte boundary *********************************************

        Handle->Data     = u32(Data) & 0xfffffff;
        Handle->m_iPool  = s_PoolIndices[MemoryPool];
        Handle->m_bAlias = s_Ghosting;
        Handle->m_Ptr    = Data;
        Handle->m_Len    = Size;

        // Track DXT allocations for debug ****************************************

    #ifndef CONFIG_RETAIL
        switch( Format )
        {
            case D3DFMT_DXT5: DXT5_Allocs += Size; break;
            case D3DFMT_DXT3: DXT3_Allocs += Size; break;
            case D3DFMT_DXT1: DXT1_Allocs += Size; break;
        }
    #endif

        Result = Handle;
    }
    LEAVE_CRIT

    return Result;
}

//=============================================================================

texture_factory::cube_handle texture_factory::CreateTiledCubeMap( const char* pResourceName,D3DFORMAT Format,u32 TileIndex,u32 Bpp,u32 nMips,u32 Edge )
{
    cube_handle Result = NULL;

    ENTER_CRIT
    {
        SanityCheckTNV();

        // allocate from tiled buffer *********************************************

        cube_handle Handle = CreateCubeMap( pResourceName,Format,Bpp,nMips,Edge,kPOOL_TILED );

        // setup tiling info ******************************************************

        if( Handle )
        {
            for( s32 i=0;i<6;i++ )
            {
                IDirect3DSurface8* pCubeMap;
                Handle->GetCubeMapSurface((D3DCUBEMAP_FACES)i,0,&pCubeMap );

                D3DSURFACE_DESC Desc;
                pCubeMap->GetDesc( &Desc );
                u32 Pitch = Desc.Size/Desc.Height;
                switch( Pitch )
                {
                    case D3DTILE_PITCH_0200:
                    case D3DTILE_PITCH_0300:
                    case D3DTILE_PITCH_0400:
                    case D3DTILE_PITCH_0500:
                    case D3DTILE_PITCH_0600:
                    case D3DTILE_PITCH_0700:
                    case D3DTILE_PITCH_0800:
                    case D3DTILE_PITCH_0A00:
                    case D3DTILE_PITCH_0C00:
                    case D3DTILE_PITCH_0E00:
                    case D3DTILE_PITCH_1000:
                    case D3DTILE_PITCH_1400:
                    case D3DTILE_PITCH_1800:
                    case D3DTILE_PITCH_1C00:
                    case D3DTILE_PITCH_2000:
                    case D3DTILE_PITCH_2800:
                    case D3DTILE_PITCH_3000:
                    case D3DTILE_PITCH_3800:
                    case D3DTILE_PITCH_4000:
                    case D3DTILE_PITCH_5000:
                    case D3DTILE_PITCH_6000:
                    case D3DTILE_PITCH_7000:
                    case D3DTILE_PITCH_8000:
                    case D3DTILE_PITCH_A000:
                    case D3DTILE_PITCH_C000:
                    case D3DTILE_PITCH_E000:
                    {
                        D3DTILE Tile;
                        x_memset( &Tile,0,sizeof( Tile ));

                        Tile.pMemory = (void*)(u32(Handle->Data) | 0x80000000);
                        Tile.Pitch   = Desc.Size/Desc.Height;
                        Tile.Size    = Desc.Size;

                        g_pd3dDevice->SetTile( TileIndex, NULL );
                        g_pd3dDevice->SetTile( TileIndex,&Tile );
                        break;
                    }
                }
                pCubeMap->Release();
            }
            Result = Handle;
        }
    }
    LEAVE_CRIT

    return Result;
}

//=============================================================================

texture_factory::volume_handle texture_factory::CreateVolume( const char* pResourceName,D3DFORMAT Format,u32 Bpp,u32 nMips,u32 W,u32 H,u32 D,s32 MemoryPool )
{
    volume_handle Result = NULL;

    ENTER_CRIT
    {
        SanityCheckTNV();

        volume_handle Handle;
        u32 nBytes,Byp;
        u8* pBytes;
        {
            // Get cube map info **************************************************

            Handle = new volume_buffer;
            m_Pitch = ( W*Bpp )>>3;
            Byp     = m_Pitch/W;

            // Create texture *****************************************************

            nBytes = m_Pitch*H*D;
            pBytes = (u8*)m_Allocator[s_PoolIndices[MemoryPool]].Alloc( pResourceName,nBytes );
            SanityCheckTNV();

            // Apply cubic information ********************************************

            XGSetVolumeTextureHeader( W,H,D,nMips+1,0,Format,0,Handle,0,m_Pitch );

            // Update header ******************************************************

            Handle->Data     = u32(pBytes) & 0xfffffff;
            Handle->m_iPool  = s_PoolIndices[MemoryPool];
            Handle->m_bAlias = s_Ghosting;
            Handle->m_Len    = nBytes;
            Handle->m_Ptr    = pBytes;

            // Track DXT allocations for debug ************************************

        #ifndef CONFIG_RETAIL
            switch( Format )
            {
                case D3DFMT_DXT5: DXT5_Allocs += nBytes; break;
                case D3DFMT_DXT3: DXT3_Allocs += nBytes; break;
                case D3DFMT_DXT1: DXT1_Allocs += nBytes; break;
            }
        #endif
        }
        Result = Handle;
    }
    LEAVE_CRIT

    return Result;
}

//=============================================================================

texture_factory::cube_handle texture_factory::CreateCubeMap( const char* pResourceName,D3DFORMAT Format,u32 Bpp,u32 nMips,u32 Edge,s32 MemoryPool )
{
    cube_handle Result = NULL;

    ENTER_CRIT
    {
        SanityCheckTNV();

        cube_handle Handle;
        u32 nBytes,Byp;
        u8* pBytes;
        {
            //==-------------------------------------------------------------------
            //
            //  Get cube map info
            //
            Handle = new cube_buffer;
            m_Pitch = ( Edge*Bpp )>>3;
            Byp     = m_Pitch/Edge;

            //==-------------------------------------------------------------------
            //
            //  Create texture
            //
            nBytes = m_Pitch*Edge*6;
            pBytes = (u8*)m_Allocator[s_PoolIndices[MemoryPool]].Alloc( pResourceName,nBytes );
            SanityCheckTNV();
            //
            //  Apply cubic information
            //
            XGSetCubeTextureHeader( Edge,nMips+1,0,Format,0,Handle,0,m_Pitch );
            //
            //  Update header
            //
            Handle->Data     = u32(pBytes) & 0xfffffff;
            Handle->m_iPool  = s_PoolIndices[MemoryPool];
            Handle->m_bAlias = s_Ghosting;
            Handle->m_Len    = nBytes;
            Handle->m_Ptr    = pBytes;

            // Track DXT allocations for debug ************************************

        #ifndef CONFIG_RETAIL
            switch( Format )
            {
                case D3DFMT_DXT5: DXT5_Allocs += nBytes; break;
                case D3DFMT_DXT3: DXT3_Allocs += nBytes; break;
                case D3DFMT_DXT1: DXT1_Allocs += nBytes; break;
            }
        #endif
        }
        Result = Handle;
    }
    LEAVE_CRIT

    return Result;
}

//=============================================================================

texture_factory::cube_handle texture_factory::CreateCubeMap( const char* pResourceName,D3DFORMAT Format,xbitmap* pBitmap,s32 MemoryPool )
{
    cube_handle Result = NULL;

    ENTER_CRIT
    {
        SanityCheckTNV();

        #ifdef CONFIG_QA
        if( !pBitmap )__asm int 3
        #endif

        cube_handle Handle;
        {
            //==---------------------------------------------------------------
            //
            //  Create texture
            //
            u32 nMips = pBitmap->GetNMips();
            u32 Edge  = pBitmap->GetWidth();
            u32 Bpp   = pBitmap->GetFormatInfo().BPP;
            u32 Byp   = Bpp>>3;

            Handle = CreateCubeMap(
                pResourceName,
                Format,
                Bpp,
                nMips,
                Edge,
                MemoryPool
            );

            //==---------------------------------------------------------------
            //
            //  Update unmipped texture
            //
            u32 nBytes = pBitmap->GetDataSize();
            if( ! nMips )
            {
                for( s32 i=0;i<6;i++ )
                {
                    xbitmap& Bitmap = pBitmap[i];
                    u8*     pSource = (u8*)Bitmap.GetPixelData( );
                    u8*     pDest   = (u8*)Handle->Lock( i,0 );

                    // handle preregistered bitmaps ...............................

                    if( !pSource )
                    {
                        ASSERT( Bitmap.GetFlags() & xbitmap::FLAG_XBOX_PRE_REGISTERED );
                        texture_factory::handle Handle = (texture_factory::handle)Bitmap.GetVRAMID();
                        pSource = (u8*)Handle->GetPtr( );
                        x_memcpy( pDest,pSource,nBytes );
                    }

                    // the more complicated path ..................................
                    else
                    {
                        switch( Format )
                        {
                            case D3DFMT_DXT1:
                            case D3DFMT_DXT3:
                                x_memcpy( pDest,pSource,nBytes );
                                break;

                            default:
                                XGSwizzleRect( pSource,0,NULL,pDest,Edge,Edge,NULL,Byp );
                                break;
                        }
                    }
                    Handle->Unlock( );
                }
                LEAVE_CRIT
                return Handle;
            }

            //==---------------------------------------------------------------
            //
            //  Update mip levels
            //
            u32 nMipBytes;
            for( s32 i=0;i<6;i++ )
            {
                xbitmap& Bitmap = pBitmap[i];
                for( u32 iMip=0;iMip<=nMips;iMip++ )
                {
                    u8* pSource = (u8*)Bitmap.GetPixelData(iMip);
                    if( pSource )
                    {
                        u8* pDest = (u8*)Handle->Lock(i,iMip);
                        {
                            switch( Format )
                            {
                                case D3DFMT_DXT1:
                                case D3DFMT_DXT3:
                                    nMipBytes = Bitmap.GetMipDataSize( iMip );
                                    x_memcpy( pDest,pSource,nMipBytes );
                                    break;

                                default:
                                    XGSwizzleRect( pSource,0,NULL,pDest,Edge,Edge,NULL,Byp );
                                    Edge >>= 1;
                                    break;
                            }
                            Handle->Unlock( );
                        }
                        #ifdef X_ASSERT
                        ASSERT( nBytes >= nMipBytes );
                        nBytes -= nMipBytes;
                        #endif
                    }
                    #ifdef CONFIG_QA
                    else __asm int 3
                    #endif
                }
            }
        }
        Result = Handle;
    }
    LEAVE_CRIT

    return Result;
}

//=============================================================================

texture_factory::handle texture_factory::Create( const char* pResourceName,xbitmap& Bitmap,s32 MemoryPool )
{
    handle Result = NULL;

    ENTER_CRIT
    {
        u32 nBytes,nMips,Byp;
        handle Handle;
        u8* pBytes;
        {
            // Figure out the format to call it ***********************************

            D3DFORMAT Format;
            switch( Bitmap.GetFormat() )
            {
                case xbitmap::FMT_P8_RGBA_8888: Format = D3DFMT_P8      ; break;
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
                    ASSERT( FALSE );
                    break;
            }

            // Create texture resource ********************************************

            s32 W = Bitmap.GetWidth();
            s32 H = Bitmap.GetHeight();

            Handle  = new buffer;
            Byp     = XGBytesPerPixelFromFormat( Format );
            nMips   = Bitmap.GetNMips();
            m_Pitch = W * Byp;

            // Create texture *****************************************************

            SanityCheckTNV();
            nBytes = Bitmap.GetDataSize();
            pBytes = (u8*)m_Allocator[s_PoolIndices[MemoryPool]].Alloc( pResourceName,nBytes );
            SanityCheckTNV();

            // Handle non-power of two special case *******************************

            if( !PowerOfTwo(W) )
            {
                switch( Format )
                {
                    case D3DFMT_A8R8G8B8: Format = D3DFMT_LIN_A8R8G8B8; break;
                    case D3DFMT_X8R8G8B8: Format = D3DFMT_LIN_X8R8G8B8; break;
                    case D3DFMT_A1R5G5B5: Format = D3DFMT_LIN_A1R5G5B5; break;
                    case D3DFMT_X1R5G5B5: Format = D3DFMT_LIN_X1R5G5B5; break;
                    case D3DFMT_A4R4G4B4: Format = D3DFMT_LIN_A4R4G4B4; break;
                    case D3DFMT_R5G6B5  : Format = D3DFMT_LIN_R5G6B5  ; break;
                    case D3DFMT_A8      : Format = D3DFMT_LIN_A8      ; break;
                    default:
                        ASSERT(0);
                        break;
                }
            }

            // Update header ******************************************************

            XGSetTextureHeader( W,H,nMips+1,0,Format,0,Handle,0,0 );
            ((IDirect3DTexture8*)Handle)->Register( (void*)pBytes );
            Handle->m_iPool  = s_PoolIndices[MemoryPool];
            Handle->m_bAlias = s_Ghosting;
            Handle->m_Len    = nBytes;
            Handle->m_Ptr    = pBytes;
            Handle->m_iTile  = -1;
            x_memset(
                &Handle->m_Tile,
                0,
                sizeof(D3DTILE)
            );

            // Copy texels ********************************************************

            if(!( Bitmap.GetFlags() & xbitmap::FLAG_XBOX_PRE_REGISTERED ))
            {
                // no mip levels - swizzle ........................................

                if( ! nMips )
                {
                    u8* pSource = (u8*)Bitmap.GetPixelData( );
                    if( pSource )
                    {
                        u8* pDest = (u8*)Handle->Lock(0);
                        if( PowerOfTwo(W) )
                        {
                            switch( Format )
                            {
                                case D3DFMT_DXT1:
                                case D3DFMT_DXT3:
                                case D3DFMT_DXT5:
                                    x_memcpy( pDest,pSource,nBytes );
                                    break;

                                default:
                                    XGSwizzleRect( pSource,0,NULL,pDest,W,H,NULL,Byp );
                                    break;
                            }
                        }
                        else
                        {
                            x_memcpy( pDest,pSource,nBytes );
                        }
                        Handle->Unlock( );
                    }
                    else
                    {
                        __asm int 3 // for debugging retail
                    }
                }

                // mip levels - copy ..............................................

                else
                {
                    u32 nMipBytes;
                    for( u32 iMip=0;iMip<=nMips;iMip++ )
                    {
                        u8* pSource = (u8*)Bitmap.GetPixelData(iMip);
                        if( pSource )
                        {
                            u8* pDest = (u8*)Handle->Lock(iMip);
                            if( PowerOfTwo(W) )
                            {
                                switch( Format )
                                {
                                    case D3DFMT_DXT1:
                                    case D3DFMT_DXT3:
                                    case D3DFMT_DXT5:
                                        nMipBytes = Bitmap.GetMipDataSize( iMip );
                                        x_memcpy( pDest,pSource,nMipBytes );
                                        break;

                                    default:
                                        XGSwizzleRect( pSource,0,NULL,pDest,W,H,NULL,Byp );
                                        nMipBytes = Bitmap.GetMipDataSize( iMip );
                                        W >>= 1;
                                        H >>= 1;
                                        break;
                                }
                            }
                            else
                            {
                                nMipBytes = Bitmap.GetMipDataSize( iMip );
                                x_memcpy( pDest,pSource,nMipBytes );
                            }
                            Handle->Unlock( );
                        }
                        else
                        {
                            __asm int 3 // for debugging retail
                        }

                        #ifdef X_ASSERT
                        ASSERT( nBytes >= nMipBytes );
                        nBytes -= nMipBytes;
                        #endif
                    }
                }
            }

            // Create palette resource ********************************************

            switch( Bitmap.GetFormat() )
            {
                case xbitmap::FMT_P8_RGBA_8888: // converts to P8_BGR_888
                {
                    palette  * pClut = new palette;
                    Handle->m_pClut = pClut;
                    u8       * pPalette;
                    {
                        nBytes   = 1024;
                        pBytes   = (u8*)m_Allocator[s_PoolIndices[MemoryPool]].Alloc( pResourceName,nBytes );
                        pPalette = (u8*)Bitmap.GetClutData();
                        SanityCheckTNV();

                        s32  i,c,n = Bitmap.GetClutSize();
                        for( i=c=0;i<n;i+=4,c+=3 )
                        {
                            pBytes[c+0] = pPalette[i+2];
                            pBytes[c+1] = pPalette[i+1];
                            pBytes[c+2] = pPalette[i+0];
                        }

                        XGSetPaletteHeader( D3DPALETTE_256,pClut,0 );

                        pClut->Data  = u32(pBytes) & 0xfffffff;
                        pClut->m_Len = nBytes;
                        pClut->m_Ptr = pBytes;
                    }
                    break;
                }
                default:
                    Handle->m_pClut = NULL;
                    break;
            }

            // Track DXT allocations for debug ************************************

        #ifndef CONFIG_RETAIL
            switch( Format )
            {
                case D3DFMT_DXT5: DXT5_Allocs += nBytes; break;
                case D3DFMT_DXT3: DXT3_Allocs += nBytes; break;
                case D3DFMT_DXT1: DXT1_Allocs += nBytes; break;
            }
        #endif
        }
        Result = Handle;
    }
    LEAVE_CRIT

    return Result;
}

//=============================================================================

texture_factory::handle texture_factory::CreateZ( const char* pResourceName,u32 Pitch,u32 XRes,u32 YRes,IDirect3DSurface8*& pSurface,D3DFORMAT Format,s32 MemoryPool )
{
    handle Result = NULL;

    ENTER_CRIT
    {
        SanityCheckTNV();
        texture_factory::handle Handle = Create( pResourceName,Pitch,XRes,YRes,Format,MemoryPool );
        if( Handle )
            Handle->GetSurfaceLevel( 0,&pSurface );
        Result = Handle;
    }
    LEAVE_CRIT

    return Result;
}

//=============================================================================

texture_factory::handle texture_factory::CreateTiledZ( const char* pResourceName,u32 Pitch,u32 XRes,u32 YRes,IDirect3DSurface8*& pSurface,D3DFORMAT Format,u32 TileIndex,bool bCompressed )
{
    handle Result = NULL;

    ENTER_CRIT
    {
        SanityCheckTNV();

        // allocate from tiled buffer *********************************************

        texture_factory::handle Handle = Create( pResourceName,Pitch,XRes,YRes,Format,kPOOL_TILED );
        if( Handle )
            Handle->GetSurfaceLevel( 0,&pSurface );

        // setup memory tiling ****************************************************

        if( Handle )
        {
            D3DSURFACE_DESC Desc;
            pSurface->GetDesc( &Desc );

            D3DTILE &Tile = Handle->m_Tile;
            x_memset(
                &Tile,0,
                sizeof(D3DTILE)
            );
            Handle->m_iTile = TileIndex;
            Tile.Size       = (Desc.Size+(D3DTILE_ALIGNMENT-1))&~(D3DTILE_ALIGNMENT-1);
            Tile.Flags      = D3DTILE_FLAGS_ZBUFFER | D3DTILE_FLAGS_Z32BITS;
            Tile.pMemory    = (void*)(u32(Handle->Data) | 0x80000000);
            Tile.Pitch      = Pitch;

            if( bCompressed )
            {
                D3DTILE Tile1;
                g_pd3dDevice->GetTile( TileIndex-2,&Tile1 );
                Tile.ZStartTag = D3DTILE_ZENDTAG( &Tile1 );
                Tile.Flags    |= D3DTILE_FLAGS_ZCOMPRESS;
                Tile.ZOffset   = 0;
            }
        }
        Result = Handle;
    }
    LEAVE_CRIT

    return Result;
}

//=============================================================================

texture_factory::handle texture_factory::CreateTiled( const char* pResourceName,u32 Pitch,u32 XRes,u32 YRes,IDirect3DSurface8*& pSurface,D3DFORMAT Format,u32 TileIndex )
{
    handle Result = NULL;

    ENTER_CRIT
    {
        SanityCheckTNV();

        // allocate from tiled buffer *********************************************

        texture_factory::handle Handle = Create( pResourceName,Pitch,XRes,YRes,Format,kPOOL_TILED );

        // setup tiling for buffer ************************************************

        if( Handle )
        {
            Handle->GetSurfaceLevel( 0,&pSurface );

            // This is because sometimes the surface version didn't match
            // the texture version. They must point to the same thing.
            // Stupid... aarrgghhh.

            pSurface->Data = Handle->Data;
            {
                switch( Pitch )
                {
                    case D3DTILE_PITCH_0200:
                    case D3DTILE_PITCH_0300:
                    case D3DTILE_PITCH_0400:
                    case D3DTILE_PITCH_0500:
                    case D3DTILE_PITCH_0600:
                    case D3DTILE_PITCH_0700:
                    case D3DTILE_PITCH_0800:
                    case D3DTILE_PITCH_0A00:
                    case D3DTILE_PITCH_0C00:
                    case D3DTILE_PITCH_0E00:
                    case D3DTILE_PITCH_1000:
                    case D3DTILE_PITCH_1400:
                    case D3DTILE_PITCH_1800:
                    case D3DTILE_PITCH_1C00:
                    case D3DTILE_PITCH_2000:
                    case D3DTILE_PITCH_2800:
                    case D3DTILE_PITCH_3000:
                    case D3DTILE_PITCH_3800:
                    case D3DTILE_PITCH_4000:
                    case D3DTILE_PITCH_5000:
                    case D3DTILE_PITCH_6000:
                    case D3DTILE_PITCH_7000:
                    case D3DTILE_PITCH_8000:
                    case D3DTILE_PITCH_A000:
                    case D3DTILE_PITCH_C000:
                    case D3DTILE_PITCH_E000:
                    {
                        D3DSURFACE_DESC Desc;
                        pSurface->GetDesc( &Desc );

                        D3DTILE   &Tile = Handle->m_Tile;
                        x_memset( &Tile,0,sizeof(D3DTILE) );
                        Handle->m_iTile = TileIndex;
                        Tile.Size       = (Desc.Size+(D3DTILE_ALIGNMENT-1))&(~(D3DTILE_ALIGNMENT-1));
                        Tile.pMemory    = Handle->GetPtr();
                        Tile.Pitch      = Pitch;
                        break;
                    }

                    default:
                        x_DebugMsg( "Warning: buffer too small to be tiled\n" );
                        break;
                }
            }
            Result = Handle;
        }
    }
    return Result;
}

//=============================================================================

texture_factory::handle texture_factory::Create( const char* pResourceName,u32 Pitch,u32 XRes,u32 YRes,IDirect3DSurface8*& pSurface,D3DFORMAT Format,s32 MemoryPool )
{
    handle Result = NULL;

    ENTER_CRIT
    {
        SanityCheckTNV();
        texture_factory::handle Handle = Create( pResourceName,Pitch,XRes,YRes,Format,MemoryPool );
        if( Handle )
            Handle->GetSurfaceLevel( 0,&pSurface );
        Result = Handle;
    }
    LEAVE_CRIT

    return Result;
}

//=============================================================================

texture_factory::handle texture_factory::Alias( handle Handle,void* Data,aliasing_style Style )
{
    handle Result = NULL;

    ENTER_CRIT
    {
        switch( Style )
        {
            case ALIAS_FROM_SCRATCH:
                Result = (handle)smem_BufferAlloc( sizeof(buffer) );
                break;

            case ALIAS_FROM_MAIN:
                Result = new buffer;
                break;

            default:
                ASSERT(0);
                break;
        }
        x_memcpy( Result,Handle,sizeof(buffer) );
        Result->Data  = ((u32)Data) & 0xfffffff;
        Result->m_Ptr = ((u8*)Data);
        Result->m_bAlias = true;
    }
    LEAVE_CRIT

    return Result;
}

//=============================================================================

texture_factory::handle texture_factory::Alias( u32 Pitch,u32 XRes,u32 YRes,D3DFORMAT Format,void* Data,aliasing_style Style )
{
    handle Result = NULL;

    ENTER_CRIT
    {
        // Allocates header *******************************************************

        SanityCheckTNV();
        switch( Style )
        {
            case ALIAS_FROM_SCRATCH:
                Result = (handle)smem_BufferAlloc( sizeof(buffer) );
                break;

            case ALIAS_FROM_MAIN:
                Result = new buffer;
                break;

            default:
                ASSERT(0);
                break;
        }

        // Populate struct ********************************************************

        x_memset( Result,0,sizeof(buffer) );
        if( (Format >= D3DFMT_LIN_A1R5G5B5) && (Format <= D3DFMT_LIN_F16) )
            Result->m_Linear = u32( Format );
        if( !Pitch )
            m_Pitch = XGBytesPerPixelFromFormat( Format )*XRes;
        else
            m_Pitch = Pitch;

        XGSetTextureHeader( XRes,YRes,1,0,Format,0,Result,0,Pitch );

        // Align to 128 byte boundary *********************************************

        Result->Data     = u32(Data) & 0xfffffff;
        Result->m_Len    = m_Pitch*YRes;
        Result->m_bAlias = s_Ghosting;
        Result->m_Ptr    = (u8*)Data;
        Result->m_iPool  = -1;
        Result->m_iTile  = -1;
    }
    LEAVE_CRIT

    return Result;
}






//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================




//=============================================================================

void texture_factory::buffer::Register( xbitmap& Bitmap )
{
    SanityCheckTNV();

    if( Bitmap.GetFlags() & xbitmap::FLAG_XBOX_PRE_REGISTERED )
    {
    }
    else
    {
        xbitmap Temp( Bitmap );
        Bitmap.Kill();
        Bitmap.Setup
        (
            Temp.GetFormat( ),
            Temp.GetWidth ( ),
            Temp.GetHeight( ),
            FALSE,
            m_Ptr
        );
    }
}

//=============================================================================

void texture_factory::buffer::Init( )
{
    SanityCheckTNV();

    m_bAlias = 0;
    m_Linear = 0;
    m_pClut  = 0;
    m_iTile  =-1;
    m_iPool  = 0;
    m_Lock   = 0;
    m_Len    = 0;
    m_Ptr    = 0;
}

//=============================================================================

void texture_factory::buffer::Kill()
{
    SanityCheckTNV();
    if( !m_bAlias )
        g_TextureFactory.m_Allocator[m_iPool].Free( m_Ptr );
    Init( );
}

//=============================================================================

void texture_factory::volume_buffer::Init( void )
{
    SanityCheckTNV();

    m_bAlias = 0;
    m_Linear = 0;
    m_Lock   = 0;
    m_Len    = 0;
    m_Ptr    = 0;
}

//=============================================================================

void texture_factory::volume_buffer::Kill( void )
{
    SanityCheckTNV();

    if( !m_bAlias )
        g_TextureFactory.m_Allocator[m_iPool].Free( m_Ptr );

    Init( );
}

//=============================================================================

void* texture_factory::volume_buffer::Lock( u32 iLevel )
{
    SanityCheckTNV();

    m_Lock++;
    D3DLOCKED_BOX Lb;
    VERIFY( LockBox( iLevel,&Lb,NULL,0 )==0 );
    return Lb.pBits;
}

//=============================================================================

void texture_factory::volume_buffer::Unlock( void )
{
    SanityCheckTNV();

    ASSERT( m_Lock > 0 );
    if( !-- m_Lock )
    {
        VERIFY( !UnlockBox( 0 ));
__asm wbinvd
    }
}

//=============================================================================

void* texture_factory::buffer::Lock( u32 iLevel )
{
    SanityCheckTNV();

    m_Lock++;
    D3DLOCKED_RECT Lr;
    VERIFY( !LockRect( iLevel, &Lr, NULL, 0 ));
    return Lr.pBits;
}

//=============================================================================

void texture_factory::buffer::Unlock( void )
{
    SanityCheckTNV();

    ASSERT( m_Lock > 0 );
    if( !-- m_Lock )
    {
        VERIFY( !UnlockRect( 0 ));
__asm wbinvd
    }
}






//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================




//=============================================================================

void texture_factory::cube_buffer::Init( )
{
    SanityCheckTNV();

    m_bAlias = 0;
    m_Linear = 0;
    m_Lock   = 0;
    m_Len    = 0;
    m_Ptr    = 0;
}

//=============================================================================

void texture_factory::cube_buffer::Kill()
{
    SanityCheckTNV();

	BlockUntilNotBusy( );

    if( !m_bAlias )
        g_TextureFactory.m_Allocator[m_iPool].Free( m_Ptr );

    Init( );
}

//=============================================================================

void* texture_factory::cube_buffer::Lock( u32 iFace,u32 iLevel )
{
    SanityCheckTNV();

    m_Lock++;
    D3DLOCKED_RECT Lr;
    VERIFY( LockRect( (D3DCUBEMAP_FACES)iFace,iLevel, &Lr, NULL, 0 ) == 0 );
    return Lr.pBits;
}

//=============================================================================

void texture_factory::cube_buffer::Unlock( )
{
    SanityCheckTNV();

    ASSERT( m_Lock > 0 );
    if( !-- m_Lock )
    {
__asm wbinvd
    }
}

//=============================================================================

static bool SetTile( texture_factory::handle pTarget )
{
    bool bResult = false;
    if( pTarget )
    {
        D3DTILE* pTile = pTarget->GetTilePointer();
        s32 TileIndex = pTarget->GetTileId();
        if( TileIndex > -1 )
        {
            ASSERT( TileIndex < D3DTILE_MAXTILES );

            g_pd3dDevice->SetTile( TileIndex,NULL  );
            g_pd3dDevice->SetTile( TileIndex,pTile );
            bResult = true;
        }
    }
    return bResult;
}

//=============================================================================

void texture_mgr::Set( u32 iStage,texture_factory::handle pTex )
{
    SanityCheckTNV();

    ASSERT( iStage < D3DTSS_MAXSTAGES );
    if( pTex )
    {
        IDirect3DBaseTexture8* pSlot = m_Stage[iStage];
        if( pSlot && (pSlot->Data==pTex->Data) )
        {
            return;
        }
    }
    g_pd3dDevice->SetTexture( iStage,pTex );
    m_Stage[iStage] = pTex;
}

//=============================================================================

void texture_mgr::Set( u32 iStage,IDirect3DBaseTexture8* pTex )
{
    SanityCheckTNV();

    ASSERT( iStage < D3DTSS_MAXSTAGES );
    if( pTex )
    {
        IDirect3DBaseTexture8* pSlot = m_Stage[iStage];
        if( pSlot && (pSlot->Data==pTex->Data) )
        {
            return;
        }
    }
    g_pd3dDevice->SetTexture( iStage,pTex );
    m_Stage[iStage] = pTex;
}



//==============================================================================
//==============================================================================
//==============================================================================
//  XBOX Hacky functions
//==============================================================================
//==============================================================================
//==============================================================================


/* Kill texels. Req. for A51.
   */

void xbox_FreeTexels( xbitmap& Bitmap )
{
    texture_factory::handle Handle=( texture_factory::handle )Bitmap.GetVRAMID();
    ASSERT( Handle != NULL );
    if( Handle )
    {
        Bitmap.SetVRAMID(0);
        delete Handle;
    }
}

static const char* s_pResourceName = NULL;
void xbox_SetAllocationName( const char* pResourceName )
{
    s_pResourceName = pResourceName;
}

/* Required for A51 to reduce the memory footprint of textures.
   Before this they took twice the memory needed.
   */

void* xbox_AllocateTexels( xbitmap& Bitmap,X_FILE* pFile )
{
    const char* pResourceName = "xbox_AllocateTexels()";
    if( s_pResourceName )
          pResourceName = s_pResourceName;
    texture_factory::handle Handle =
        g_TextureFactory.Create( pResourceName,Bitmap );

    // Now copy the pixels over ...............................................

    D3DSURFACE_DESC Desc;
    Handle->GetLevelDesc( 0,&Desc );

    s32 nMips = Bitmap.GetNMips();
    if( nMips )
    {
        for( s32 iMip=0;iMip<=nMips;iMip++ )
        {
            u32   nDst = Bitmap.GetMipDataSize( iMip );
            s32      H = Bitmap.GetHeight( iMip );
            s32      W = Bitmap.GetWidth ( iMip );
            void* pDst = Handle->Lock    ( iMip );
            {
                switch( Desc.Format )
                {
                    case D3DFMT_DXT1:
                    case D3DFMT_DXT3:
                    case D3DFMT_DXT5:
                        x_fread( pDst,1,nDst,pFile );
                        break;

                    default:
                    {
                        u8* pSrc = (u8*)x_malloc( nDst );
                        x_fread( pSrc,1,nDst,pFile );
                        XGSwizzleRect(
                            pSrc,
                            0,
                            NULL,
                            pDst,
                            W,
                            H,
                            NULL,
                            XGBytesPerPixelFromFormat( Desc.Format ));
                        x_free( pSrc );
                        break;
                    }
                }
            }
            Handle->Unlock();
        }
    }
    else
    {
        u8* pDst = (u8*)Handle->Lock(0);
        u32 nDst = Bitmap.GetDataSize();
        s32    H = Bitmap.GetHeight();
        s32    W = Bitmap.GetWidth();
        switch( Desc.Format )
        {
            case D3DFMT_DXT1:
            case D3DFMT_DXT3:
            case D3DFMT_DXT5:
                x_fread( pDst,1,nDst,pFile );
                break;

            default:
            {
                u8* pSrc = (u8*)x_malloc( nDst );
                x_fread( pSrc,1,nDst,pFile );
                if( !PowerOfTwo(W) )
                {
                    s32 Stride = XGBytesPerPixelFromFormat( Desc.Format )*W;
                    ASSERT( Stride == nDst/H );
                    x_memcpy( pDst,pSrc,nDst );
                }
                else
                {
                    XGSwizzleRect( pSrc,0,NULL,pDst,W,H,NULL,XGBytesPerPixelFromFormat( Desc.Format ));
                }
                x_free( pSrc );
                break;
            }
        }
        Handle->Unlock();
    }

__asm wbinvd

    ASSERT( Handle );
    return  Handle;
}

//=============================================================================

void texture_factory::palette::Init( void )
{
    m_Len = 0;
    m_Ptr = 0;
}

//=============================================================================

void texture_factory::palette::Kill( void )
{
	BlockUntilNotBusy( );

    g_TextureFactory.m_Allocator[kPOOL_GENERAL].Free( m_Ptr );

    Init( );
}
