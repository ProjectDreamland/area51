//==============================================================================
//  
//  x_bitmap_io.cpp
//  
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

/* std includes */

#ifndef X_BITMAP_HPP
#include "..\x_bitmap.hpp"
#endif

#ifndef X_MEMORY_HPP
#include "..\x_memory.hpp"
#endif

#ifndef X_STRING_HPP
#include "..\x_string.hpp"
#endif

//==============================================================================
//  TYPES
//==============================================================================

struct io_buffer
{
    s32             DataSize;
    s32             ClutSize;
    s32             Width;
    s32             Height;
    s32             PW;
    u32             Flags;
    s32             NMips;
    xbitmap::format Format;
};

//==============================================================================
//  FUNCTIONS
//==============================================================================
//  Binary data written to file is in PC Little Endian format.

static
xbool xbmp_Save( const xbitmap& Bitmap, X_FILE* pFile )
{
    io_buffer   Buffer;
    s32         BytesWritten;
    s32         BytesToWrite;

    // Convert the xbitmap header into our simple io_buffer form.
    Buffer.DataSize = Bitmap.GetDataSize();
    Buffer.ClutSize = Bitmap.GetClutSize();
    Buffer.Width    = Bitmap.GetWidth();
    Buffer.Height   = Bitmap.GetHeight();
    Buffer.PW       = Bitmap.GetPWidth();
    Buffer.Flags    = Bitmap.GetFlags();
    Buffer.NMips    = Bitmap.GetNMips();
    Buffer.Format   = Bitmap.GetFormat();

    // Make sure we have Little Endian data.
    #ifdef BIG_ENDIAN
    Buffer.DataSize = ENDIAN_SWAP_32( Buffer.DataSize );
    Buffer.ClutSize = ENDIAN_SWAP_32( Buffer.ClutSize );
    Buffer.Width    = ENDIAN_SWAP_32( Buffer.Width    );
    Buffer.Height   = ENDIAN_SWAP_32( Buffer.Height   );
    Buffer.PW       = ENDIAN_SWAP_32( Buffer.PW       );
    Buffer.Flags    = ENDIAN_SWAP_32( Buffer.Flags    );
    Buffer.NMips    = ENDIAN_SWAP_32( Buffer.NMips    );
    Buffer.Format   = (xbitmap::format)ENDIAN_SWAP_32( Buffer.Format   );
    #endif

    // Now we can write out the header data.
    BytesWritten = x_fwrite( &Buffer, 1, sizeof(io_buffer), pFile );
    if( BytesWritten != sizeof(io_buffer) )
        return( FALSE );

    // Write out the pixel data.
    BytesToWrite = Bitmap.GetDataSize();
    BytesWritten = x_fwrite( Bitmap.GetPixelData(-1),
                             1, 
                             BytesToWrite, 
                             pFile );
    if( BytesWritten != BytesToWrite )
        return( FALSE );

    // If present, write out clut data.
    BytesToWrite = Bitmap.GetClutSize();
    if( BytesToWrite )
    {
        BytesWritten = x_fwrite( Bitmap.GetClutData(), 
                                 1, 
                                 BytesToWrite, 
                                 pFile );
        if( BytesWritten != BytesToWrite )
            return( FALSE );
    }

    // We made it!
    return( TRUE );
}

//==============================================================================

xbool xbitmap::Save( X_FILE* pFile ) const
{
    ASSERT( pFile );
    ASSERT( m_Flags & FLAG_VALID );

    #ifdef BIG_ENDIAN
    {
        // Ugh!  This is a pain.
        xbitmap EndianMangledCopy( *this );
        EndianMangledCopy.ToggleEndian();
        return( xbmp_Save( EndianMangledCopy, pFile ) );
    }
    #endif

    #ifdef LITTLE_ENDIAN
    {
        // Ahh!  This is easy!
        return( xbmp_Save( *this, pFile ) );
    }
    #endif
}

//==============================================================================

xbool xbitmap::Save( const char* pFileName ) const
{
    X_FILE* pFile;
    xbool   Success;

    ASSERT( pFileName );

    pFile = x_fopen( pFileName, "wb" );
    if( !pFile )  return( FALSE );
    Success = Save( pFile );
    x_fclose( pFile );
    return( Success );
}

//==============================================================================

xbool xbitmap::Info( const char* pFileName, info& BitmapInfo )
{
    X_FILE*     pFile;
    xbool       Success = FALSE;
    io_buffer   Buffer;
    s32         BytesRead;

    ASSERT( pFileName );

    pFile = x_fopen( pFileName, "rb" );
    if( pFile )
    {
        // Read in our simple io_buffer for the header data.
        BytesRead = x_fread( &Buffer, 1, sizeof(io_buffer), pFile );
        if( BytesRead == sizeof(io_buffer) )
        {
            // Make sure we have correctly read Little Endian data.
            #ifdef BIG_ENDIAN
            Buffer.DataSize = ENDIAN_SWAP_32( Buffer.DataSize );
            Buffer.ClutSize = ENDIAN_SWAP_32( Buffer.ClutSize );
            Buffer.Width    = ENDIAN_SWAP_32( Buffer.Width    );
            Buffer.Height   = ENDIAN_SWAP_32( Buffer.Height   );
            Buffer.PW       = ENDIAN_SWAP_32( Buffer.PW       );
            Buffer.Flags    = ENDIAN_SWAP_32( Buffer.Flags    );
            Buffer.NMips    = ENDIAN_SWAP_32( Buffer.NMips    );
            Buffer.Format   = (xbitmap::format)ENDIAN_SWAP_32( Buffer.Format   );
            #endif

            // Close the file
            x_fclose( pFile );

            // Fill in the info structure
            BitmapInfo.W      = Buffer.Width;
            BitmapInfo.H      = Buffer.Height;
            BitmapInfo.nMips  = Buffer.NMips;
            BitmapInfo.Format = Buffer.Format;
            Success = TRUE;
        }
    }

    return( Success );
}

//==============================================================================

xbool xbitmap::Load( X_FILE* pFile )
{
    s32         BytesRead;
    xbool       Success = FALSE;
    io_buffer   Buffer;

    MEMORY_OWNER( "XBMP DATA" );

    ASSERT( pFile );

    // Prepare for incoming data.
    Kill();

    // Read in our simple io_buffer for the header data.
    BytesRead = x_fread( &Buffer, 1, sizeof(io_buffer), pFile );
    if( BytesRead != sizeof(io_buffer) )
        goto EXIT;

    // Make sure we have correctly read Little Endian data.
    #ifdef BIG_ENDIAN
    Buffer.DataSize = ENDIAN_SWAP_32( Buffer.DataSize );
    Buffer.ClutSize = ENDIAN_SWAP_32( Buffer.ClutSize );
    Buffer.Width    = ENDIAN_SWAP_32( Buffer.Width    );
    Buffer.Height   = ENDIAN_SWAP_32( Buffer.Height   );
    Buffer.PW       = ENDIAN_SWAP_32( Buffer.PW       );
    Buffer.Flags    = ENDIAN_SWAP_32( Buffer.Flags    );
    Buffer.NMips    = ENDIAN_SWAP_32( Buffer.NMips    );
    Buffer.Format   = (xbitmap::format)ENDIAN_SWAP_32( Buffer.Format   );
    #endif

    // Stick the values we read in the correct places.
    m_DataSize      =      Buffer.DataSize;
    m_ClutSize      =      Buffer.ClutSize;
    m_Width         = (s16)Buffer.Width;
    m_Height        = (s16)Buffer.Height;
    m_PW            = (s16)Buffer.PW;
    m_Flags         = (u16)Buffer.Flags;
    m_NMips         =  (s8)Buffer.NMips;
    m_Format        =  (s8)Buffer.Format;

    // On Xbox everything can be a texture object but because
    // GetPixelColor(x,y) doesn't support swizzling yet we
    // can't optimise everything.
    #if defined TARGET_XBOX
    if( 1 )
    {
        if( m_NMips )
        {
            s32 nBytes  = (m_NMips+1)*sizeof(mip);
            m_Data.pMip = (mip *)x_malloc(nBytes);
            BytesRead   = x_fread( m_Data.pMip,1,nBytes,pFile );
        }
        m_Flags |=  xbitmap::FLAG_XBOX_PRE_REGISTERED;
        m_Flags |=  xbitmap::FLAG_XBOX_DATA_SWIZZLED;
        m_Flags &= ~xbitmap::FLAG_DATA_OWNED;

        m_VRAMID=( s32 )xbox_AllocateTexels( *this,pFile );
    }
    else
    #endif
    {
        // Read in the pixel data block.
        m_Data.pPixel = (byte*)x_malloc( m_DataSize );
        ASSERT( m_Data.pPixel );

        m_Flags       |= FLAG_DATA_OWNED;
        BytesRead      = x_fread( m_Data.pPixel, 1, m_DataSize, pFile );
        if( BytesRead != m_DataSize )
            goto EXIT_FREE;

        // If present, read in clut data.
        if( m_ClutSize )
        {
            m_pClut   = (byte*)x_malloc( m_ClutSize );
            m_Flags  |= FLAG_CLUT_OWNED;
            BytesRead = x_fread( m_pClut, 1, m_ClutSize, pFile );
            if( BytesRead != m_ClutSize )
                goto EXIT_FREE;
        }
    }

    // Make sure we have correctly read Little Endian data.
    #ifdef BIG_ENDIAN
    ToggleEndian();
    #endif

    #ifdef TARGET_GCN
    DCFlushRange((void *) m_Data.pPixel, m_DataSize);
    if( m_ClutSize )
        DCFlushRange((void *) m_pClut, m_ClutSize);
    #endif  

    // We made it!
    Success = TRUE;
    goto EXIT;

    // Off-ramps.
    EXIT_FREE:      Kill();
    EXIT:           return( Success );
}

//==============================================================================

xbool xbitmap::Load( const char* pFileName )
{
    X_FILE* pFile;
    xbool   Success;

    ASSERT( pFileName );
#ifdef TARGET_XBOX
    xbox_SetAllocationName( pFileName );
#endif
    pFile = x_fopen( pFileName, "rb" );
    if( !pFile )  return( FALSE );
    Success = Load( pFile );
    x_fclose( pFile );
    return( Success );
}

//==============================================================================

static 
void ToggleEndian16( u16* pData, s32 Count )
{
    while( Count )
    {
        *pData = ENDIAN_SWAP_16( *pData );
        pData++;
        Count--;
    }
}

//==============================================================================

static 
void ToggleEndian32( u32* pData, s32 Count )
{
    while( Count )
    {
        *pData = ENDIAN_SWAP_32( *pData );
        pData++;
        Count--;
    }
}

//==============================================================================

void xbitmap::ToggleEndian( void )
{
    // Is there a clut which is 16 bits per entry?
    if( (m_FormatInfo[ m_Format ].ClutBased) && 
        (m_FormatInfo[ m_Format ].BPC == 16) )
    {
        ASSERT( m_pClut );
        ToggleEndian16( (u16*)m_pClut, m_ClutSize >> 1 );
    }

    // Is there a clut which is 32 bits per entry?
    if( (m_FormatInfo[ m_Format ].ClutBased) && 
        (m_FormatInfo[ m_Format ].BPC == 32) )
    {
        ToggleEndian32( (u32*)m_pClut, m_ClutSize >> 2 );
    }

    // We proceed a little differently if we have mip records.
    if( m_NMips == 0 )
    {
        // We need only toggle the data, and only if it is 16 or 32 bits.

        // Toggle data if it is 16 bit.
        if( m_FormatInfo[ m_Format ].BPP == 16 )
            ToggleEndian16( (u16*)m_Data.pPixel, m_PW * m_Height );

        // Toggle data if it is 32 bit.
        if( m_FormatInfo[ m_Format ].BPP == 32 )
            ToggleEndian32( (u32*)m_Data.pPixel, m_PW * m_Height );
    }
    else
    {
        s32 i;

        // There are mip records and perhaps multiple mips of the data.
        // Process each mip.

        for( i = 0; i < m_NMips; i++ )
        {
            // Toggle data for mip 'i' if it is 16 bit.
            if( m_FormatInfo[ m_Format ].BPP == 16 )
                ToggleEndian16( (u16*)(m_Data.pPixel + m_Data.pMip[i].Offset),
                                m_Data.pMip[i].Width * m_Data.pMip[i].Height );

            // Toggle data for mip 'i' if it is 32 bit.
            if( m_FormatInfo[ m_Format ].BPP == 32 )
                ToggleEndian32( (u32*)(m_Data.pPixel + m_Data.pMip[i].Offset),
                                m_Data.pMip[i].Width * m_Data.pMip[i].Height );

            // Finally, toggle the mip record's values.
            m_Data.pMip[i].Offset = ENDIAN_SWAP_32( m_Data.pMip[i].Offset );
            m_Data.pMip[i].Width  = ENDIAN_SWAP_16( m_Data.pMip[i].Width  );
            m_Data.pMip[i].Height = ENDIAN_SWAP_16( m_Data.pMip[i].Height );
        }
    }
}

//==============================================================================

#if !( defined( TARGET_PS2 ) && defined( CONFIG_RETAIL ) )

//==============================================================================

static
void xbmp_Dump32( X_FILE* pFile, u32* pData, s32 PerRow, s32 Rows )
{
    s32 i;
    while( Rows-- )
    {
        x_fprintf( pFile, "   " );
        for( i = 0; i < PerRow; i++ )
        {
            x_fprintf( pFile, " 0x%08X,", *pData );
            pData++;
        }
        x_fprintf( pFile, "\n" );
    }
}

//==============================================================================

static
void xbmp_Dump24( X_FILE* pFile, byte* pData, s32 PerRow, s32 Rows )
{
    s32 i;
    while( Rows-- )
    {
        x_fprintf( pFile, "  " );
        for( i = 0; i < PerRow; i++ )
        {
            x_fprintf( pFile, "  0x%02X,0x%02X,0x%02X,", 
                              pData[0], pData[1], pData[2] ); 
            pData += 3;
        }
        x_fprintf( pFile, "\n" );
    }
}

//==============================================================================

static
void xbmp_Dump16( X_FILE* pFile, u16* pData, s32 PerRow, s32 Rows )
{
    s32 i;
    while( Rows-- )
    {
        x_fprintf( pFile, "   " );
        for( i = 0; i < PerRow; i++ )
        {
            x_fprintf( pFile, " 0x%04X,", *pData );
            pData++;
        }
        x_fprintf( pFile, "\n" );
    }
}

//==============================================================================

static
void xbmp_Dump8( X_FILE* pFile, byte* pData, s32 PerRow, s32 Rows )
{
    s32 i;
    while( Rows-- )
    {
        x_fprintf( pFile, "    " );
        for( i = 0; i < PerRow; i++ )
        {
            x_fprintf( pFile, "0x%02X,", *pData );
            pData++;
        }
        x_fprintf( pFile, "\n" );
    }
}

//==============================================================================

static
void xbmp_Dump4( X_FILE* pFile, byte* pData, s32 PerRow, s32 Rows )
{
    s32 i;
    while( Rows-- )
    {
        x_fprintf( pFile, "    " );
        for( i = 0; i < PerRow; i += 2 )
        {
            x_fprintf( pFile, "0x%02X,", *pData );
            pData++;
        }
        x_fprintf( pFile, "\n" );
    }
}

//==============================================================================

xbool xbitmap::DumpSourceCode( const char* pFileName ) const
{
    X_FILE* pFile;
    s32     DataSize;
    byte*   pData;

    ASSERT( pFileName );
    pFile = x_fopen( pFileName, "wt" );
    if( !pFile )
        return( FALSE );

    DataSize = (m_PW * m_Height * m_FormatInfo[m_Format].BPP) >> 3;
    pData    = m_NMips ? 
               m_Data.pPixel + m_Data.pMip[0].Offset :
               m_Data.pPixel;

    x_fprintf( pFile, "// Format   : %6d (%s)\n",   
                      (s32)m_Format, 
                      m_FormatInfo[m_Format].pString );
    x_fprintf( pFile, "// Width    : %6d pixels\n", (s32)m_Width    );
    x_fprintf( pFile, "// Height   : %6d pixels\n", (s32)m_Height   );
    x_fprintf( pFile, "// PW       : %6d pixels\n", (s32)m_PW       );
    x_fprintf( pFile, "// DataSize : %6d bytes\n",       DataSize   );
    x_fprintf( pFile, "// ClutSize : %6d bytes\n",       m_ClutSize );
    x_fprintf( pFile, "// Flags    : 0x%04X\n",          m_Flags    );
    x_fprintf( pFile, "\n" );

    if( m_ClutSize )
    {
        s32 Rows;

        if( m_FormatInfo[ m_Format ].BPP == 4 )   Rows = 1;
        else                                      Rows = 16;

        switch( m_FormatInfo[ m_Format ].BPC )
        {
        case 32:    x_fprintf( pFile, "u32  Clut[] __attribute__((aligned(16))) =\n{\n" );  break;
        case 24:    x_fprintf( pFile, "byte Clut[] __attribute__((aligned(16))) =\n{\n" );  break;
        case 16:    x_fprintf( pFile, "u16  Clut[] __attribute__((aligned(16))) =\n{\n" );  break;
        default:    ASSERT( FALSE );
        } 
   
        switch( m_FormatInfo[ m_Format ].BPC )
        {
        case 32:    xbmp_Dump32( pFile, (u32*)m_pClut, 16, Rows );  break;
        case 24:    xbmp_Dump24( pFile,       m_pClut, 16, Rows );  break;
        case 16:    xbmp_Dump16( pFile, (u16*)m_pClut, 16, Rows );  break;
        default:    ASSERT( FALSE );
        }

        x_fprintf( pFile, "};\n\n" );
    }

    switch( m_FormatInfo[ m_Format ].BPP )
    {
    case 32:    x_fprintf( pFile, "u32  Data[] __attribute__((aligned(16))) =\n{\n" );  break;
    case 24:    x_fprintf( pFile, "byte Data[] __attribute__((aligned(16))) =\n{\n" );  break;
    case 16:    x_fprintf( pFile, "u16  Data[] __attribute__((aligned(16))) =\n{\n" );  break;
    case  8:    x_fprintf( pFile, "byte Data[] __attribute__((aligned(16))) =\n{\n" );  break;
    case  4:    x_fprintf( pFile, "byte Data[] __attribute__((aligned(16))) =\n{\n" );  break;
    default:    ASSERT( FALSE );
    } 

    switch( m_FormatInfo[ m_Format ].BPP )
    {
    case 32:    xbmp_Dump32( pFile, (u32*)pData, m_PW, m_Height );  break;
    case 24:    xbmp_Dump24( pFile,       pData, m_PW, m_Height );  break;
    case 16:    xbmp_Dump16( pFile, (u16*)pData, m_PW, m_Height );  break;
    case  8:    xbmp_Dump8 ( pFile,       pData, m_PW, m_Height );  break;
    case  4:    xbmp_Dump4 ( pFile,       pData, m_PW, m_Height );  break;
    default:    ASSERT( FALSE );
    }

    x_fprintf( pFile, "};\n\n" );
    x_fclose ( pFile );
    return( TRUE );
}

//==============================================================================

xbool xbitmap::SavePaletteTGA( const char* pFileName ) const
{
    // Be sure it's palettized
    if( GetBPP() > 8 )
        return FALSE;

    s32 W = (1 << GetBPP());
    s32 H = 32;

    xcolor* pData = (xcolor*)x_malloc(sizeof(xcolor)*W*H);

    xbitmap BMP;
    BMP.Setup( xbitmap::FMT_XCOLOR, W, H, FALSE, (byte*)pData );

    // fill out data
    for( s32 x=0; x<W; x++ )
    {
        xcolor C = GetClutColor(x);
        for( s32 y=0; y<H; y++ )
            pData[ x+y*W ] = C;
    }

    xbool Result = BMP.SaveTGA( pFileName );

    x_free(pData);
    return Result;
}

//==============================================================================

xbool xbitmap::SaveTGA( const char* pFileName ) const
{
    X_FILE* pFile;
    format  Format;
    byte    Header[18];
    xbitmap Clone( *this );
    Clone.m_VRAMID = 0;

    ASSERT( m_Flags & FLAG_VALID );
    ASSERT( pFileName );

#ifdef LITTLE_ENDIAN
    Format = FMT_32_ARGB_8888;
#endif

#ifdef BIG_ENDIAN
    Format = FMT_32_BGRA_8888;
#endif

    // Convert to appropriate 32 bit format.
    Clone.BuildMips( 0 );
    Clone.ConvertFormat( Format );

    // Build the header information.
    x_memset( Header, 0, 18 );
    Header[ 2] = 2;     // Image type.
    Header[12] = (Clone.m_PW     >> 0) & 0xFF;
    Header[13] = (Clone.m_PW     >> 8) & 0xFF;
    Header[14] = (Clone.m_Height >> 0) & 0xFF;
    Header[15] = (Clone.m_Height >> 8) & 0xFF;
    Header[16] = 32;    // Bit depth.
    Header[17] = 32;    // NOT flipped vertically.

    // Open the file.
    pFile = x_fopen( pFileName, "wb" );
    if( !pFile )
        return( FALSE );

    // Write out the data.
    x_fwrite( Header, 1, 18, pFile );
    x_fwrite( Clone.m_Data.pPixel, 1, Clone.m_PW * Clone.m_Height * 4, pFile );
    x_fclose( pFile );

    return( TRUE );
}

//==============================================================================

xbool xbitmap::SaveMipsTGA( const char* pFileName ) const
{
    char FileName[ X_MAX_PATH  ];
    char Drive   [ X_MAX_DRIVE ];
    char Dir     [ X_MAX_DIR   ];
    char FName   [ X_MAX_FNAME ];
    char Ext     [ X_MAX_EXT   ];
    s32  i;

    if( m_NMips == 0 )
    {
        return( SaveTGA( pFileName ) );
    }

    x_splitpath( pFileName, Drive, Dir, FName, Ext );

    for( i = 0; i <= m_NMips; i++ )
    {
        xbitmap Temp;
        xbool   Result;

        Temp.Setup( GetFormat(), 
                    GetWidth(i), 
                    GetHeight(i), 
                    FALSE, 
                    (byte*)GetPixelData(i), 
                    FALSE, 
                    (byte*)GetClutData() );

        x_makepath( FileName, Drive, Dir, xfs( "%s%02d", FName, i ), Ext );
        Result = Temp.SaveTGA( FileName );

        if( !Result )
            return( FALSE );
    }

    return( TRUE );
}

//==============================================================================

#endif // !( defined( TARGET_PS2 ) && defined( CONFIG_RETAIL ) )

//==============================================================================
