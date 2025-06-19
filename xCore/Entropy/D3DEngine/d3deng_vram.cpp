///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

#include "e_Engine.hpp"

///////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////

#define MAX_TEXTURES 5000

///////////////////////////////////////////////////////////////////////////
// TYPES
///////////////////////////////////////////////////////////////////////////

union d3d_tex_node
{
    IDirect3DTexture9*  pTexture;
    s32                 iNext;    
};

///////////////////////////////////////////////////////////////////////////
// LOCALS
///////////////////////////////////////////////////////////////////////////

static s32          s_LastActiveID = 0;
static d3d_tex_node s_List[ MAX_TEXTURES ];
static s32          s_iEmpty;


///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////


//=============================================================================

void vram_Init( void )
{
    //
    // Initialize the empty list
    //
    for( s32 i=0; i<MAX_TEXTURES-1; i++ )
    {
        s_List[i].iNext = i+1;
    }
    s_List[i].iNext = 0;    // Zero is the universal NULL
    s_iEmpty        = 1;    // Leave empty the node 0
}

//=============================================================================

void vram_Kill( void )
{
    for( s32 i=0; i<MAX_TEXTURES; i++ )
    {
        if( s_List[i].iNext > MAX_TEXTURES )
        {
            s_List[i].pTexture->Release();
        }
    }
}

//=============================================================================

static
s32 AddNode( IDirect3DTexture9* pTexture )
{
    s32 NewID;

    ASSERTS( s_iEmpty != 0, "Out of texture space" );

    NewID    = s_iEmpty;
    s_iEmpty = s_List[ s_iEmpty ].iNext;

    ASSERT( NewID > 0 );
    ASSERT( NewID < MAX_TEXTURES );

    s_List[ NewID ].pTexture = pTexture;

    ASSERT( (s_List[ NewID ].iNext >= MAX_TEXTURES) || (s_List[ NewID ].iNext==0));

    return NewID;
}

//=============================================================================

s32 vram_LoadTexture( const char* pFileName )
{
    dxerr               Error;
    IDirect3DTexture9*  pTexture = NULL;

    Error = D3DXCreateTextureFromFile( g_pd3dDevice, pFileName, &pTexture );
    ASSERT( Error == 0 );
    ASSERT( pTexture );

    return AddNode( pTexture );
}

//=============================================================================

void vram_Activate( void )
{
    ASSERT( eng_InBeginEnd() );

    s_LastActiveID = 0;
    
    if( g_pd3dDevice )
        g_pd3dDevice->SetTexture( 0, NULL );
}

//=============================================================================

void vram_Activate( s32 VRAM_ID )
{
    // Note: VRAM_ID == 0 means bitmap not registered!
    ASSERT( VRAM_ID > 0 );              
    ASSERT( VRAM_ID < MAX_TEXTURES );
    ASSERT( eng_InBeginEnd() );

    s_LastActiveID = VRAM_ID;
    
    if( g_pd3dDevice )
    {
        ASSERT( s_List[ VRAM_ID ].iNext > MAX_TEXTURES );
        g_pd3dDevice->SetTexture( 0, s_List[ VRAM_ID ].pTexture );
    }
}

//=============================================================================

s32 vram_Register( IDirect3DTexture9* pTexture )
{
	//GS: So, I added this code to fix the shaders working with PIP.
	//This code was borrowed from the Xbox implementation.
	//So I’m not sure if it’s even needed here.
    s32 Index = AddNode( pTexture );
    return Index;
}

//=============================================================================

s32 vram_Register( const xbitmap& Bitmap )
{
    IDirect3DTexture9*  pSurface = NULL;
    dxerr               Error;
    D3DFORMAT           Format;

    ASSERT( Bitmap.GetClutData() == NULL && "Not clut is suported" );

    //
    // Get the DX format
    //
    switch( Bitmap.GetFormat() )
    {
        case xbitmap::FMT_32_ARGB_8888: Format = D3DFMT_A8R8G8B8; break;
        case xbitmap::FMT_32_URGB_8888: Format = D3DFMT_X8R8G8B8; break;
        case xbitmap::FMT_16_ARGB_4444: Format = D3DFMT_A4R4G4B4; break;
        case xbitmap::FMT_16_ARGB_1555: Format = D3DFMT_A1R5G5B5; break;
        case xbitmap::FMT_16_URGB_1555: Format = D3DFMT_X1R5G5B5; break;
        case xbitmap::FMT_16_RGB_565:   Format = D3DFMT_R5G6B5;   break;
        case xbitmap::FMT_DXT3:         Format = D3DFMT_DXT3;     break;
        case xbitmap::FMT_DXT1:         Format = D3DFMT_DXT1;     break;
        default:
            {
                ASSERT( FALSE );
            }
    }

    if( g_pd3dDevice )
    {
        //
        // Create the texture
        //
        Error = g_pd3dDevice->CreateTexture( Bitmap.GetWidth(), 
                                             Bitmap.GetHeight(), 
                                             0, 0, 
                                             Format,
                                             D3DPOOL_MANAGED,
                                             &pSurface,
                                             NULL );
        ASSERT( Error == 0 );

        //
        // Copy texture
        //
        {
            D3DLOCKED_RECT  Locked;
            char*           pSrcData;   
            char*           pDestData;
            s32             BitmapPitch;

            x_memset( &Locked, 0, sizeof(Locked) );
            Error = pSurface->LockRect( 0, &Locked, NULL, 0 );
            ASSERT( Error == 0 );

            pSrcData    = (char*)Bitmap.GetPixelData();
            pDestData   = (char*)Locked.pBits;

            if( (Bitmap.GetFormat() == xbitmap::FMT_DXT1) || (Bitmap.GetFormat() == xbitmap::FMT_DXT3) )
            {
                x_memcpy( pDestData, pSrcData, Bitmap.GetMipDataSize(0) );
            }
            else
            {
                BitmapPitch = (Bitmap.GetPWidth() * Bitmap.GetFormatInfo().BPP)/8 ;

                for( s32 y=0; y<Bitmap.GetHeight(); y++ )
                {
                    x_memcpy( pDestData, pSrcData, BitmapPitch );
                    pDestData += Locked.Pitch;
                    pSrcData  += BitmapPitch;

                }
            }

            pSurface->UnlockRect( 0 );
        }

        //
        // Create the mipmaps
        //
        Error = D3DXFilterTexture( pSurface, NULL, D3DX_DEFAULT, D3DX_DEFAULT  );
        ASSERT( Error == 0 );
    }

    //
    // Add texture in the list
    //
    s32 Index = AddNode( pSurface );
    Bitmap.SetVRAMID( Index );
    return Index;
}

//=============================================================================

// Util function for getting height
s32 GetHeight(const xbitmap& BMP, s32 x, s32 y)
{
    xcolor Col = BMP.GetPixelColor(x,y) ;
    s32    H   = (Col.R + Col.G + Col.B) / 3 ;
    return H ;
}

s32 vram_RegisterDuDv( const xbitmap& Bitmap )
{
    IDirect3DTexture9*  pSurface = NULL;
    dxerr               Error;
    D3DFORMAT           Format = D3DFMT_X8L8V8U8 ;
    //enum _D3DFORMAT Format = D3DFMT_V8U8 ;
    //enum _D3DFORMAT Format = D3DFMT_L6V5U5 ;

    //
    // Create the bump map texture
    //
    Error = g_pd3dDevice->CreateTexture( Bitmap.GetWidth(), 
                                         Bitmap.GetHeight(), 
                                         0, 0, 
                                         Format,
                                         D3DPOOL_MANAGED,
                                         &pSurface,
                                         NULL );
    ASSERT( Error == 0 );

    // Create the texture from the bitmap
    D3DLOCKED_RECT  d3dlr;
    pSurface->LockRect( 0, &d3dlr, 0, 0 );
    DWORD dwDstPitch = (DWORD)d3dlr.Pitch;
    BYTE* pDst       = (BYTE*)d3dlr.pBits;
    for( s32 y=0; y< Bitmap.GetHeight(); y++ )
    {
        BYTE* pDstT  = pDst;

        for( s32 x=0; x< Bitmap.GetWidth(); x++ )
        {
            s32 v00 = GetHeight(Bitmap, x,y) ;                              // Get the current pixel
            s32 v01 = GetHeight(Bitmap, MIN(x+1, Bitmap.GetWidth()-1), y) ; // and the pixel to the right
            s32 vM1 = GetHeight(Bitmap, MAX(x-1,0), y) ;                    // and the pixel to the left
            s32 v10 = GetHeight(Bitmap, x, MIN(y+1, Bitmap.GetHeight()-1)); // and the pixel one line below.
            s32 v1M = GetHeight(Bitmap, x, MAX(y-1, 0));                    // and the pixel one line above.

            s32 iDu = (vM1-v01); // The delta-u bump value
            s32 iDv = (v1M-v10); // The delta-v bump value

            if( (v00 < vM1) && (v00 < v01) )  // If we are at valley
            {
                iDu = vM1-v00;                 // Choose greater of 1st order diffs
                if( iDu < v00-v01 )
                    iDu = v00-v01;
            }

            // The luminance bump value (land masses are less shiny)
            WORD uL = ( v00>1 ) ? 63 : 127;
            // Use luminance of max
            uL = 255 ;
            switch( Format )
            {
                case D3DFMT_V8U8:
                    *pDstT++ = (BYTE)iDu;
                    *pDstT++ = (BYTE)iDv;
                    break;

                case D3DFMT_L6V5U5:
                    *(WORD*)pDstT  = (WORD)( ( (iDu>>3) & 0x1f ) <<  0 );
                    *(WORD*)pDstT |= (WORD)( ( (iDv>>3) & 0x1f ) <<  5 );
                    *(WORD*)pDstT |= (WORD)( ( ( uL>>2) & 0x3f ) << 10 );
                    pDstT += 2;
                    break;

                case D3DFMT_X8L8V8U8:
                    *pDstT++ = (BYTE)iDu;
                    *pDstT++ = (BYTE)iDv;
                    *pDstT++ = (BYTE)uL;
                    *pDstT++ = (BYTE)0L;
                    break;
            }
        }

        // Move to the next line
        pDst += dwDstPitch;
    }

    pSurface->UnlockRect(0);


    //
    // Create the mipmaps
    //
    Error = D3DXFilterTexture( pSurface, NULL, D3DX_DEFAULT, D3DX_DEFAULT  );
    ASSERT( Error == 0 );

    //
    // Add texture in the list
    //
    s32 Index = AddNode( pSurface );
    Bitmap.SetVRAMID( Index );
    return Index;
}

//=============================================================================

void vram_Unregister( s32 VRAM_ID )
{
    ASSERT( VRAM_ID >= 0 );
    ASSERT( VRAM_ID < MAX_TEXTURES );

    if( VRAM_ID == s_LastActiveID ) s_LastActiveID = 0;

    if( s_List[ VRAM_ID ].pTexture )
        s_List[ VRAM_ID ].pTexture->Release();

    s_List[ VRAM_ID ].iNext = s_iEmpty;
    s_iEmpty = VRAM_ID;
}

//=============================================================================

void vram_Unregister( const xbitmap& Bitmap  )
{
    vram_Unregister( Bitmap.GetVRAMID() );

    // Signal bitmap is no longer in VRAM.
    Bitmap.SetVRAMID( 0 );
}

//=============================================================================

void vram_Activate( const xbitmap& Bitmap  )
{
    vram_Activate( Bitmap.GetVRAMID() );
}

//=============================================================================

xbool vram_IsActive( const xbitmap& Bitmap )
{
    return( (s_LastActiveID != 0) && (s_LastActiveID == Bitmap.GetVRAMID()) );
}

//=============================================================================

void vram_Flush( void )
{
}

//=============================================================================

s32 vram_GetNRegistered( void )
{
    s32 nRegister = 0;

    for( s32 i=0; i<MAX_TEXTURES; i++ )
    {
        if( s_List[i].iNext > MAX_TEXTURES )
        {
            nRegister++;
        }
    }

    return nRegister;
}

//=============================================================================

s32 vram_GetRegistered( s32 ID )
{
    s32 nRegister = 0;

    for( s32 i=0; i<MAX_TEXTURES; i++ )
    {
        if( s_List[i].iNext > MAX_TEXTURES )
        {
            nRegister++;
            if( nRegister == ID ) return i;
        }
    }

    ASSERT( 0 );
    return -1;
}

//=============================================================================

void vram_PrintStats( void )
{
    x_printfxy( 0,  6, "Free Vmem:   %d", g_pd3dDevice->GetAvailableTextureMem() );
    x_printfxy( 0,  7, "NTextures:   %d", vram_GetNRegistered() );
}

//=============================================================================

void vram_SanityCheck( void )
{

}

//=============================================================================

IDirect3DTexture9* vram_GetSurface( s32 VRAM_ID )
{
    // Note: VRAM_ID == 0 means bitmap not registered!
    ASSERT( VRAM_ID > 0 );
    ASSERT( VRAM_ID < MAX_TEXTURES );
    ASSERT( s_List[ VRAM_ID ].iNext > MAX_TEXTURES );

    return s_List[ VRAM_ID ].pTexture;
}

//=============================================================================

IDirect3DTexture9*  vram_GetSurface( const xbitmap& Bitmap ) 
{
    return vram_GetSurface( Bitmap.GetVRAMID() );
}

//=============================================================================
