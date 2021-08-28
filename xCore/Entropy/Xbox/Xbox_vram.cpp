///////////////////////////////////////////////////////////////////////////
// INCLUDES
///////////////////////////////////////////////////////////////////////////

#include "e_Engine.hpp"

#include "xbox/TextureMgr.hpp"
#include "xbox/VertexMgr.hpp"
#include "xbox/IndexMgr.hpp"
#include "xbox/PushMgr.hpp"

///////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////

#define MAX_TEXTURES 3000

///////////////////////////////////////////////////////////////////////////
// TYPES
///////////////////////////////////////////////////////////////////////////

union d3d_tex_node
{
    texture_factory::handle pTexture;
    s32 iNext;    
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


//=========================================================================

void vram_Init( void )
{
    //
    // Initialize the empty list
    //
    s32 i=0;
    for( ; i<MAX_TEXTURES-1; i++ )
    {
        s_List[i].pTexture = NULL;
        s_List[i].iNext = i+1;
    }
    s_List[i].iNext = 0;    // Zero is the universal NULL
    s_iEmpty        = 1;    // Leave empty the node 0
}

//=========================================================================

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

//=========================================================================

static
s32 AddNode( texture_factory::handle pTexture )
{
    s32 NewID;

    ASSERTS( s_iEmpty != 0, "Out of texture space" );

    NewID    = s_iEmpty;
    s_iEmpty = s_List[ s_iEmpty ].iNext;

    ASSERT( NewID > 0 );
    ASSERT( NewID < MAX_TEXTURES );

    s_List[ NewID ].pTexture = pTexture;

    ASSERT( s_List[ NewID ].iNext >= MAX_TEXTURES );

    return NewID;
}

//=========================================================================

s32 vram_LoadTexture( const char* pFileName )
{
    ASSERT( FALSE );
    return 0;
}

//=========================================================================

void vram_Activate( void )
{
    ASSERT( eng_InBeginEnd() );

    s_LastActiveID = 0;
    g_Texture.Clear( 0 );
    g_Texture.Clear( 1 );
    g_Texture.Clear( 2 );
    g_Texture.Clear( 3 );
}

//=========================================================================

void vram_Activate( s32 Stage,s32 VRAM_ID )
{
    u32 Base = g_TextureFactory.GetGeneralPool().GetBase();
    u32 Size = g_TextureFactory.GetGeneralPool().GetSize();

    // VRAM_ID is actually a preregistered handle!
    if( VRAM_ID > MAX_TEXTURES )
    {
        s_LastActiveID = VRAM_ID;
        g_Texture.Set( Stage, (texture_factory::handle)VRAM_ID );
        return;
    }

    // Note: VRAM_ID == 0 means bitmap not registered!
    ASSERT( VRAM_ID > 0 );
    ASSERT( s_List[ VRAM_ID ].iNext > MAX_TEXTURES );
    ASSERT( eng_InBeginEnd() );

    s_LastActiveID = VRAM_ID;
    g_Texture.Set( Stage, s_List[ VRAM_ID ].pTexture );
}

//=========================================================================

void vram_Activate( s32 VRAM_ID )
{
    vram_Activate( 0,VRAM_ID );
}

//=========================================================================

s32 vram_Register( texture_factory::handle Handle )
{
    s32 Index = AddNode( Handle );
    return Index;
}

//=========================================================================

s32 vram_Register( const xbitmap& Bitmap )
{
    if( Bitmap.GetFlags() & xbitmap::FLAG_XBOX_PRE_REGISTERED )
        return-1;

    texture_factory::handle Handle;
    {
        Handle = g_TextureFactory.Create( "VRAM Register",(xbitmap&) Bitmap );
        ASSERT( Handle );
    }
    s32 Index = vram_Register( Handle );
    Bitmap.SetVRAMID( Index );
    return Index;
}

//=========================================================================

// Util function for getting height
s32 GetHeight(const xbitmap& BMP, s32 x, s32 y)
{
    xcolor Col = BMP.GetPixelColor(x,y);
    s32    H   = (Col.R + Col.G + Col.B) / 3;
    return H;
}

s32 vram_RegisterDuDv( const xbitmap& Bitmap )
{
/*  D3DFORMAT Format = D3DFMT_X8L8V8U8;
    //
    // Create the bump map texture
    //
    texture_factory::handle Handle = NULL;
    {
        Handle = g_TextureFactory.Create(
            Bitmap.GetWidth (),
            Bitmap.GetHeight(),
            Format,
            Bitmap.GetDataSize()
        );
        ASSERT( Handle );
    }
    //
    // Create the texture from the bitmap
    //
    D3DLOCKED_RECT  d3dlr;
    Handle->LockRect( 0, &d3dlr, 0, 0 );
    DWORD dwDstPitch = (DWORD)d3dlr.Pitch;
    BYTE* pDst       = (BYTE*)d3dlr.pBits;
    for( s32 y=0; y< Bitmap.GetHeight(); y++ )
    {
        BYTE* pDstT  = pDst;

        for( s32 x=0; x< Bitmap.GetWidth(); x++ )
        {
            s32 v00 = GetHeight(Bitmap, x,y);                              // Get the current pixel
            s32 v01 = GetHeight(Bitmap, MIN(x+1, Bitmap.GetWidth()-1), y); // and the pixel to the right
            s32 vM1 = GetHeight(Bitmap, MAX(x-1,0), y);                    // and the pixel to the left
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
            uL = 255;
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

    Handle->UnlockRect(0);
    //
    // Create the mipmaps
    //
    VERIFY( !D3DXFilterTexture( Handle, NULL, D3DX_DEFAULT, D3DX_DEFAULT  ));
    //
    // Add texture in the list
    //
    s32 Index = AddNode( Handle );
    Bitmap.SetVRAMID( Index );
    return Index;
*/
return-1;
}

//=========================================================================

void vram_Unregister( s32 VRAM_ID )
{
    if( VRAM_ID == s_LastActiveID ) s_LastActiveID = 0;
    if( VRAM_ID <= MAX_TEXTURES )
    {
        ASSERT( VRAM_ID >= 0 );
        ASSERT( s_List[ VRAM_ID ].iNext > MAX_TEXTURES );

        // validation (just in case)
        d3d_tex_node& Node = s_List[ VRAM_ID ];
        if( Node.pTexture )
        {
            delete Node.pTexture;
            Node.pTexture = NULL;
        }
        s_List[ VRAM_ID ].iNext = s_iEmpty;
        s_iEmpty = VRAM_ID;
    }
}

//=========================================================================

void vram_Unregister( const xbitmap& Bitmap  )
{
    // Most bitmaps are always registered
    if( Bitmap.GetVRAMID() <= MAX_TEXTURES )
    {
        vram_Unregister( Bitmap.GetVRAMID() );
    }

    if( !(Bitmap.GetFlags() & xbitmap::FLAG_XBOX_PRE_REGISTERED) )
    {
        // Signal bitmap is no longer in VRAM.
        Bitmap.SetVRAMID( 0 );
    }
}

//=========================================================================

void vram_Activate( const xbitmap& Bitmap  )
{
    s32 VRAMID = Bitmap.GetVRAMID();
    if( VRAMID > MAX_TEXTURES )
    {
        g_Texture.Set( 0,texture_factory::handle( VRAMID ));
        s_LastActiveID = VRAMID;
        return;
    }
    vram_Activate( Bitmap.GetVRAMID() );
}

//=========================================================================

xbool vram_IsActive( const xbitmap& Bitmap )
{
    return( s_LastActiveID && s_LastActiveID == Bitmap.GetVRAMID( ))
      &&  ( Bitmap.GetFlags() & xbitmap::FLAG_XBOX_PRE_REGISTERED );
}

//=========================================================================

void vram_Flush( void )
{
}

//=========================================================================

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

//=========================================================================

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

//=========================================================================

void vram_PrintStats( void )
{
    const heap::basic& Heap = g_TextureFactory.GetGeneralPool( );
    x_printfxy( 0,  6, "Free Vmem:   %d", Heap.GetFree( ));
    x_printfxy( 0,  7, "NTextures:   %d", vram_GetNRegistered( ));
}

//=========================================================================

void vram_SanityCheck( void )
{

}

//=========================================================================

texture_factory::handle vram_GetSurface( s32 VRAM_ID )
{
    if( VRAM_ID > MAX_TEXTURES )
    {
        return texture_factory::handle( VRAM_ID );
    }

    // Note: VRAM_ID == 0 means bitmap not registered!
    ASSERT( VRAM_ID > 0 );
    ASSERT( s_List[ VRAM_ID ].iNext > MAX_TEXTURES );

    return s_List[ VRAM_ID ].pTexture;
}

//=========================================================================

texture_factory::handle vram_GetSurface( const xbitmap& Bitmap ) 
{
    return vram_GetSurface( Bitmap.GetVRAMID() );
}

//=========================================================================
s32 vram_RegisterLocked( s32 Width, s32 Height, s32 BPP )
{
    ASSERT( BPP==32 );
    g_TextureFactory.GhostGeneralIntoTemp( true );
    texture_factory::handle Handle;
    {
        D3DFORMAT Format = D3DFMT_A8R8G8B8;
        Handle = g_TextureFactory.Create(
            "VRAM Register (locked)",
            Width*XGBytesPerPixelFromFormat(Format),
            Width,
            Height,
            Format
        );
        ASSERT( Handle );
    }
    g_TextureFactory.GhostGeneralIntoTemp( false );
    return AddNode( Handle );
}


//=========================================================================
