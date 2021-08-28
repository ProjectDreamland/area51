//==============================================================================
//
//  Load_BMP.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_debug.hpp"
#include "x_memory.hpp"
#include "x_bitmap.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

#define C_WIN   1       // Image class 
#define C_OS2   2

#define BI_RGB  0       // Compression type 
#define BI_RLE8 1
#define BI_RLE4 2

#define BMP_HEADER_LEN  18
#define WIN_HEADER_LEN  36
#define OS2_HEADER_LEN   8
#define GRAB2(bp) ((s32)(((u32)(bp)[0]) | (((u32)(bp)[1])<<8)))
#define GRAB4(bp) ((s32)(((u32)(bp)[0]) | (((u32)(bp)[1])<<8) | (((u32)(bp)[2])<<16) | (((u32)(bp)[3])<<24)))

//==============================================================================
//  TYPES
//==============================================================================

// This structure does not match the layout of the .BMP file.  It's just used
// to hold the data.

struct bmp_header
{
    s32         FileSize;               // Size of file in bytes 
    s32         XHotSpot;               // Not used 
    s32         YHotSpot;               // Not used 
    s32         OffBits;                // Offset of image bits from start of header 
    s32         HeaderSize;             // Size of info header in bytes 

    s32         Width;                  // Image width in pixels 
    s32         Height;                 // Image height in pixels 
    s32         Planes;                 // Planes. Must == 1 
    s32         BitCount;               // Bits per pixels. Must be 1, 4, 8 or 24 
    s32         Compression;            // Compression type 
    s32         SizeImage;              // Size of image in bytes 
    s32         XPelsPerMeter;          // X pixels per meter 
    s32         YPelsPerMeter;          // Y pixels per meter 
    s32         ClrUsed;                // Number of colormap entries (0 == max) 
    s32         ClrImportant;           // Number of important colors 

    s32         Class;                  // Windows or OS/2 
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

static
xbool ReadHeader( X_FILE* pFile, bmp_header* pHeader )
{
    byte Buf[ WIN_HEADER_LEN ];  // Largest we'll need. 

    // Reader the header.
    if( x_fread( Buf, 1, BMP_HEADER_LEN, pFile ) != BMP_HEADER_LEN )
        return( FALSE );

    // Check BMP signature.
    if( (Buf[0]!='B') || (Buf[1]!='M') )
        return( FALSE );

    // Copy over first set of info
    pHeader->FileSize   = GRAB4( &Buf[ 2] );
    pHeader->XHotSpot   = GRAB2( &Buf[ 6] );
    pHeader->YHotSpot   = GRAB2( &Buf[ 8] );
    pHeader->OffBits    = GRAB4( &Buf[10] );
    pHeader->HeaderSize = GRAB4( &Buf[14] );

    // Use header size to determine which class we are dealing with.
    if( pHeader->HeaderSize == 40 )
    {
        if( x_fread( Buf, 1, WIN_HEADER_LEN, pFile ) != WIN_HEADER_LEN )
            return( FALSE );

        pHeader->Class         = C_WIN;
        pHeader->Width         = GRAB4( &Buf[ 0] );
        pHeader->Height        = GRAB4( &Buf[ 4] );            
        pHeader->Planes        = GRAB2( &Buf[ 8] );
        pHeader->BitCount      = GRAB2( &Buf[10] );
        pHeader->Compression   = GRAB4( &Buf[12] );
        pHeader->SizeImage     = GRAB4( &Buf[16] );
        pHeader->XPelsPerMeter = GRAB4( &Buf[20] );
        pHeader->YPelsPerMeter = GRAB4( &Buf[24] );
        pHeader->ClrUsed       = GRAB4( &Buf[28] );
        pHeader->ClrImportant  = GRAB4( &Buf[32] );
    }
    else    
    if( pHeader->HeaderSize == 12 )
    {
        if( x_fread( Buf, 1, OS2_HEADER_LEN, pFile ) != OS2_HEADER_LEN)
            return( FALSE );

        pHeader->Class           = C_OS2;
        pHeader->Width           = GRAB2( &Buf[0] );
        pHeader->Height          = GRAB2( &Buf[2] );
        pHeader->Planes          = GRAB2( &Buf[4] );
        pHeader->BitCount        = GRAB2( &Buf[6] );
        pHeader->Compression     = BI_RGB;
        pHeader->SizeImage       = 0;
        pHeader->XPelsPerMeter   = 0; 
        pHeader->YPelsPerMeter   = 0;
        pHeader->ClrUsed         = 0;
        pHeader->ClrImportant    = 0; 
    }
    else
    {
        return( FALSE );
    }

    // Check if this is reasonable data.
    if(    (pHeader->BitCount !=  4)
        && (pHeader->BitCount !=  8)
        && (pHeader->BitCount != 24)
        && (pHeader->BitCount != 32) )
        return( FALSE );

    if(    (     (pHeader->Compression != BI_RGB) 
              && (pHeader->Compression != BI_RLE8) 
              && (pHeader->Compression != BI_RLE4) )
        || ((pHeader->Compression == BI_RLE8) && (pHeader->BitCount != 8))
        || ((pHeader->Compression == BI_RLE4) && (pHeader->BitCount != 4)) )
        return( FALSE );

    if( pHeader->Planes != 1 )
        return( FALSE );

    // Fix up a few things.
    if( pHeader->BitCount < 24 )
    {
        if( pHeader->ClrUsed == 0 || pHeader->ClrUsed > (1 << pHeader->BitCount) )
            pHeader->ClrUsed = (1 << pHeader->BitCount);
    }
    else
        pHeader->ClrUsed = 0;

    return( TRUE );
}

//==============================================================================

xbool bmp_Load( xbitmap& Bitmap, const char* pFileName )
{
    X_FILE*     pFile = NULL;
    byte*       pData = NULL;
    byte*       pClut = NULL;
    s32         ClutSize = 0;
    s32         DataSize = 0;
    bmp_header  Header;

    //
    // Open file and read header info.
    //

    // Check parameters.
    ASSERT( pFileName );

    // Attempt to open the specified file for read.
    pFile = x_fopen( pFileName, "rb" );
    if( !pFile )
        goto exit_failed;

    // Read the header data into the buffer.
    if( !ReadHeader( pFile, &Header ) )
        goto exit_failed;

    //
    // Process clut data.
    //

    // Read clut if present.
    if( Header.BitCount < 16 ) 
    {
        s32   i;
        s32   ColorSize;
        byte  Buf[4*256];
        byte* Color;
        s32   MaxColors = (1 << Header.BitCount);   // Maximum number of colors 

        // Allocate and initialize the clut.
        pClut    = (byte*)x_malloc( 4 * MaxColors );
        ClutSize = 4 * MaxColors;
        if( pClut == NULL )
            goto exit_failed;
        x_memset( pClut, 0, 4 * MaxColors );

        // Read in colors.
        if( Header.Class == C_WIN )  ColorSize = 4;
        else                         ColorSize = 3;

        Color = pClut;
        if( x_fread( Buf, 1, ColorSize*Header.ClrUsed, pFile ) != ColorSize*Header.ClrUsed )
            goto exit_failed;

        for( i = 0; i < Header.ClrUsed; i++ )
        {
            Color[0] = Buf[(i*ColorSize)+0];
            Color[1] = Buf[(i*ColorSize)+1];
            Color[2] = Buf[(i*ColorSize)+2];
            Color[3] = 255;
            Color   += 4;
        }
    }

    //
    // Process pixel data.
    //

    // Decide on data size and allocate it.
    if( Header.BitCount < 8 )
    {
        DataSize = 0;
        if( Header.BitCount == 4 )  DataSize = (Header.Width & 0x01);
        if( Header.BitCount == 1 )  DataSize = (Header.Width & 0x03);
        DataSize = ((Header.Width + DataSize) * Header.Height * Header.BitCount) / 8;
    }
    else
    {
        DataSize = (Header.Width * Header.Height * Header.BitCount) / 8;
    }

    pData = (byte*)x_malloc( DataSize );
    if( pData == NULL )
        goto exit_failed;

    //--------------------------------------------------------------------------
    // PIXEL READING CODE -- 4 BIT
    //--------------------------------------------------------------------------
    if( Header.BitCount == 4 )
    {
        byte* pDataCursor;
        s32   Illen, x, y;

        Illen       = (Header.Width+1) / 2;
        pDataCursor = pData + (Header.Height -1) * Illen;    // Start at bottom.

        // Are we doing 4bit RLE?
        if( Header.Compression == BI_RLE4 )
        {
            s32 d, e;
            x_memset( pData, 0, (Header.Height * Header.Width * Header.BitCount) / 8 );

            for( x = y = 0; ; )
            {
                s32 i;

                // Read control character.
                if( (d = x_fgetc( pFile )) == X_EOF )
                    goto exit_failed;

                // Check if this is a run of pixels.
                if( d != 0 )             
                {
                    x += d;

                    // Make sure that we still in bonds.
                    if( (x > Header.Width) || (y > Header.Height) )
                    {
                        x -= d;             // Ignore this run.
                        if( (e = x_fgetc( pFile )) == X_EOF )
                            goto exit_failed;
                        continue;
                    }

                    // Get the bits that needs to be repeated.
                    if( (e = x_fgetc( pFile )) == X_EOF )
                        goto exit_failed;

                    // Do the current run
                    for( i = (d>>1); i > 0; i-- ) 
                        *(pDataCursor++) = (u8)e;
                    if( d & 1 ) 
                        *(pDataCursor++) = (u8)e;

                    continue;
                }

                // We didn't have a run so read control character.
                if( (d = x_fgetc( pFile )) == X_EOF )
                    goto exit_failed;

                // End of line.
                if( d == 0 )             
                {
                    pDataCursor -= ( x/2 + Illen );
                    x = 0;
                    y++;
                    continue;
                }

                // End of bitmap.
                if( d == 1 )     
                    break;

                // Delta.
                if( d == 2 )             
                {
                    if( ((d = x_fgetc( pFile )) == X_EOF) || 
                        ((e = x_fgetc( pFile )) == X_EOF) )
                        goto exit_failed;

                    x          += d;
                    y          += e;
                    pDataCursor += d;
                    pDataCursor -= (e * Illen);
                    continue;
                }

                // Else, run of literals.
                x += d;

                // Make sure that we are going to be in bonds.
                if( (x > Header.Width)  || (y > Header.Height) )
                {
                    s32 Btr = d/2 + (d & 1) + (((d+1) & 2) >> 1);
                    x -= d;             // Ignore this run.
                    for( ; Btr > 0; Btr-- )
                        if( (e = x_fgetc( pFile )) == X_EOF )
                            goto exit_failed;

                    continue;
                }

                // Do the literals.
                for( i = (d>>1); i > 0; i-- )
                {
                    if( (e = x_fgetc(pFile)) == X_EOF )
                        goto exit_failed;
                    *(pDataCursor++) = (u8)e;
                }

                // Handle odd literals.
                if( d & 1 )
                {
                    if( (e = x_fgetc(pFile)) == X_EOF )
                        goto exit_failed;
                    *(pDataCursor++) = (u8)(e >> 4);
                }

                // Read pad byte.
                if( (d+1) & 2 )
                {
                    if( x_fgetc( pFile ) == X_EOF )
                        goto exit_failed;
                }
            }
        }
        else    
        {
            // No 4 bit RLE compression.
            int d, s, p, i, e;

            d = Header.Width / 2;       // Double pixel count.
            s = Header.Width & 1;       // Single pixel.
            p = (4 - (d + s)) & 0x3;    // Byte pad.
            
            // Check for fast special (USUAL) case.
            if( (s==0) && (p==0) )
            {
                pDataCursor = &pData[0];
                if( x_fread( (byte*)pDataCursor, 1, d*Header.Height, pFile ) != d*Header.Height )
                    goto exit_failed;
                pDataCursor += d*Header.Height;
                for( y = 0; y < Header.Height/2; y++ )
                {
                    for( i = 0; i < d; i++ )
                    {
                        e = pData[ (Illen * y) + i ];
                        pData[ (Illen * y) + i ] = pData[ ((Header.Height-1-y) * Illen) + i ];
                        pData[ ((Header.Height-1-y) * Illen) + i ] = (u8)e;
                    }
                }
            }
            else
            {
                for( y = Header.Height; y > 0; y--, pDataCursor = &pData[ (y-1)*Illen ] )
                {
                    // Read two pixels at a time.
                    x_fread( (byte*)pDataCursor, 1, d, pFile );
                    pDataCursor += d;
    
                    // If we have an extra pixel that falls in half a byte.
                    if( s )
                    {
                        if( (e = x_fgetc(pFile)) == X_EOF )
                            goto exit_failed;
                        *(pDataCursor++) = (u8)(e >> 4);
                    }
    
                    // Read off what ever padding bytes are left.
                    for( i = 0; i < p; i++ )
                    {
                        if( x_fgetc( pFile ) == X_EOF )
                            goto exit_failed;
                    }
                }
            }
        }
    }
    else 
    //--------------------------------------------------------------------------
    // PIXEL READING CODE -- 8 BIT
    //--------------------------------------------------------------------------
    if( Header.BitCount == 8 )
    {
        byte* pDataCursor;
        s32   Illen, x, y;

        Illen       = Header.Width;
        pDataCursor = pData + (Header.Height-1) * Illen;    // Start at bottom.

        if( Header.Compression == BI_RLE8 )
        {
            s32 d, e;

            x_memset( pData, 0, (Header.Height * Header.Width * Header.BitCount) / 8 );

            for( x = y = 0; ; )
            {
                // Read control character.
                if( (d = x_fgetc( pFile )) == X_EOF )
                    goto exit_failed;

                // Run of pixels.
                if( d != 0 )            
                {
                    x += d;

                    // Make sure that stays in bounds.
                    if( (x > Header.Width) || (y > Header.Height) )
                    {
                        x -= d;                     // Ignore this run.
                        if( (e = x_fgetc( pFile )) == X_EOF )
                            goto exit_failed;
                        continue;
                    }

                    // Read the character to repeat.
                    if( (e = x_fgetc( pFile )) == X_EOF )
                        goto exit_failed;

                    // Copy the character.
                    x_memset( pDataCursor, (u8)e, d );
                    pDataCursor += d;

                    continue;
                }

                // Is not a run, read next control character.
                if( (d = x_fgetc(pFile)) == X_EOF ) 
                    goto exit_failed;

                // End of line.
                if( d == 0 )             
                {
                    pDataCursor -= (x + Illen);
                    x = 0;
                    y++;
                    continue;
                }

                // End of bitmap.
                if( d == 1 )
                    break;

                // Delta.
                if( d == 2 )             
                {
                    if( ((d = x_fgetc( pFile )) == X_EOF) || 
                        ((e = x_fgetc( pFile )) == X_EOF) )
                        goto exit_failed;

                    x           += d;
                    y           += e;
                    pDataCursor += d;
                    pDataCursor -= (e * Illen);
                    continue;
                }

                // Else, run of literals.
                x += d;

                // Make sure that we are going to be in bounds.
                if( (x > Header.Width) || (y > Header.Height) )
                {
                    int Btr = d + (d & 1);
                    x -= d;                     // Ignore this run.
                    for( ; Btr > 0; Btr-- )
                    {
                        if( (e = x_fgetc( pFile )) == X_EOF )
                            goto exit_failed;
                    }
                    continue;
                }

                // Read literals directly into our Buffer.
                if( x_fread( pDataCursor, d, 1, pFile ) != d )
                    goto exit_failed;

                pDataCursor += d;

                // Read pad byte.
                if( d & 1 ) 
                {
                    if( x_fgetc( pFile ) == X_EOF )
                        goto exit_failed;
                }
            }
        }
        else    
        {
            // No 8 bit RLE compression.
            byte Pad[4];
            int  Padlen;

            // Extra bytes to word boundary.
            Padlen = ((Header.Width + 3) & ~3) - Illen; 

            // Check for fast special (USUAL) case.
            if( Padlen == 0 )
            {
                s32 e, i;
                pDataCursor = &pData[0];
                x_fread( (byte*)pDataCursor, 1, Illen*Header.Height, pFile );
                pDataCursor += Illen*Header.Height;
                for( y = 0; y < Header.Height/2; y++ )
                {
                    for( i = 0; i < Illen; i++ )
                    {
                        e = pData[ (y*Illen) + i ];
                        pData[ (y*Illen) + i ] = pData[ ((Header.Height-1-y) * Illen) + i ];
                        pData[ ((Header.Height-1-y) * Illen) + i ] = (u8)e;
                    }
                }
            }
            else
            {
                for( y = Header.Height; y > 0; y--, pDataCursor -= Illen )
                {
                    if( x_fread( pDataCursor, 1, Illen, pFile ) != Illen )
                        goto exit_failed;
    
                    if( x_fread( Pad, 1, Padlen, pFile ) != Padlen )
                        goto exit_failed;
                }
            }
        }
    }
    else 
    //--------------------------------------------------------------------------
    // PIXEL READING CODE -- 24/32 bit
    //--------------------------------------------------------------------------
    if( (Header.BitCount == 24) || (Header.BitCount == 32) )
    {
        byte* pDataCursor;
        byte* Pad[4];
        s32   Illen, Padlen, y;
        s32   BytesPerPixel;

        BytesPerPixel = (Header.BitCount/8);
        Illen         = Header.Width * BytesPerPixel;
        Padlen        = (((Header.Width * BytesPerPixel) + 3) & ~3) - Illen;   // Extra bytes to word boundary.
        pDataCursor   = pData + (Header.Height-1) * Illen;                     // Start at bottom.

        for( y = Header.Height; y > 0; y--, pDataCursor -= Illen )
        {
            if( x_fread( pDataCursor, 1, Illen, pFile ) != Illen )
                goto exit_failed;

            if( x_fread( Pad, 1, Padlen, pFile ) != Padlen )
                goto exit_failed;
        }
    }
    else
    {
        // Unknown bitcount.
        goto exit_failed;
    }

    // Done reading file.
    x_fclose( pFile );

    // Build the xbitmap.
    {
        xbitmap::format     Format = xbitmap::FMT_NULL;
        s32                 PWidth;

        switch( Header.BitCount )
        {
            #ifdef LITTLE_ENDIAN
                case  4:    Format = xbitmap::FMT_P4_URGB_8888; break;
                case  8:    Format = xbitmap::FMT_P8_URGB_8888; break;
                case 24:    Format = xbitmap::FMT_24_BGR_888;   break;
                case 32:    Format = xbitmap::FMT_32_URGB_8888; break;
            #else // BIG_ENDIAN
                case  4:    Format = xbitmap::FMT_P4_BGRU_8888; break;      // UNTESTED
                case  8:    Format = xbitmap::FMT_P8_BGRU_8888; break;
                case 24:    Format = xbitmap::FMT_24_BGR_888;   break;      // UNTESTED
                case 32:    Format = xbitmap::FMT_32_BGRU_8888; break;      // UNTESTED
            #endif
        }

        PWidth = (DataSize*8) / Header.Height;            
        PWidth = PWidth / Header.BitCount; 

        Bitmap.Setup( Format,    
                      Header.Width,
                      Header.Height,
                      TRUE,
                      pData,
                      pClut ? TRUE : FALSE,
                      pClut,
                      PWidth );

        ASSERT( Bitmap.GetClutSize() == ClutSize );
    }

    // SUCCESS!
    return( TRUE );

    // FAILED!
    exit_failed:
    if( pFile ) x_fclose( pFile );
    if( pData ) x_free( pData );
    if( pClut ) x_free( pClut );
    return( FALSE );
}

//==============================================================================

xbool bmp_Info( const char* pFileName, xbitmap::info& Info )
{
	xbool		Success	= FALSE;
    X_FILE*     pFile	= NULL;
    bmp_header  Header;

    // Check parameters.
    ASSERT( pFileName );

    // Open file and read header info.
    pFile = x_fopen( pFileName, "rb" );
    if( pFile )
	{
		// Read the header data into the buffer.
		if( ReadHeader( pFile, &Header ) )
		{
			Info.W     = Header.Width;
			Info.H     = Header.Height;
			Info.nMips = 0;
			switch( Header.BitCount )
			{
				#ifdef LITTLE_ENDIAN
					case  4:    Info.Format = xbitmap::FMT_P4_URGB_8888; break;
					case  8:    Info.Format = xbitmap::FMT_P8_URGB_8888; break;
					case 24:    Info.Format = xbitmap::FMT_24_BGR_888;   break;
					case 32:    Info.Format = xbitmap::FMT_32_URGB_8888; break;
				#else // BIG_ENDIAN
					case  4:    Info.Format = xbitmap::FMT_P4_BGRU_8888; break;      // UNTESTED
					case  8:    Info.Format = xbitmap::FMT_P8_BGRU_8888; break;
					case 24:    Info.Format = xbitmap::FMT_24_BGR_888;   break;      // UNTESTED
					case 32:    Info.Format = xbitmap::FMT_32_BGRU_8888; break;      // UNTESTED
				#endif
			}
		}

		// Close the file
		x_fclose( pFile );
	}

	// Return success code
	return Success;
}

//==============================================================================

