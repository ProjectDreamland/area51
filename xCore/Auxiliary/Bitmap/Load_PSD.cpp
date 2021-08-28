//==============================================================================
//
//  Load_PSD.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_debug.hpp"
#include "x_memory.hpp"
#include "x_bitmap.hpp"

#ifdef EXIT_FAILURE 
#undef EXIT_FAILURE
#endif

//==============================================================================
//  TYPES
//==============================================================================

// 
//  This structure does not match the layout of the header data in a TGA file.
//  It simply has fields for all of the data members needed for this simple
//  loader.
//

struct psd_info
{                                                           
    s32     Signature;      // 8BPS
    s16     Version;        // Version
    s16     nChannels;      // Number of Channels (3=RGB) (4=RGBA)
    s32     Height;         // Number of Image Rows
    s32     Width;          // Number of Image Columns
    s16     Depth;          // Number of Bits per Channel
    s16     Mode;           // Image Mode (0=Bitmap)(1=Grayscale)(2=Indexed)(3=RGB)(4=CYMK)(7=Multichannel)
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

static
xbool psd_ReadData( byte*       pData, 
                    X_FILE*     pFile, 
                    psd_info&   Info,
                    s32         NPixels, 
                    s32         BytesPerPixel )
{
    byte    Buffer[2];
    byte*   pPlane = NULL;
    byte*   pRowTable = NULL;
    s32     iPlane;
    s32     BytesRead;
    s16     CompressionType;
    s32     iPixel;
    s32     iRow;
    s32     CompressedBytes;
    s32     iByte;
    s32     Count;
    byte    Value;

    // Read Compression Type
    BytesRead = x_fread( Buffer, 1, 2, pFile );
    if( BytesRead != 2 )
        goto EXIT_FAILURE;
    CompressionType = ((s32)Buffer[0] <<  8) +
                      ((s32)Buffer[1] <<  0);

    // Allocate Plane Buffer
    pPlane = (byte*)x_malloc( NPixels );
    ASSERT( pPlane );
    if( !pPlane ) goto EXIT_FAILURE;

    // Uncompressed?
    if( CompressionType == 0 )
    {
        // Loop through the planes
        for( iPlane=0 ; iPlane<Info.nChannels ; iPlane++ )
        {
            s32 iWritePlane = iPlane;
            if( iWritePlane > BytesPerPixel-1 ) iWritePlane = BytesPerPixel-1;

            // Read Plane
            BytesRead = x_fread( pPlane, 1, NPixels, pFile );
            if( BytesRead != NPixels )
                goto EXIT_FAILURE;

            // Move Plane to Image Data
            for( iPixel=0 ; iPixel<NPixels ; iPixel++ )
            {
                pData[iWritePlane + (iPixel*BytesPerPixel)] = pPlane[iPixel];
            }
        }
    }
    // RLE?
    else if( CompressionType == 1 )
    {
        // Allocate Row Table
        pRowTable = (byte*)x_malloc( Info.nChannels*Info.Height*2 );
        ASSERT( pRowTable );
        if( pRowTable == NULL )
            goto EXIT_FAILURE;

        // Load RowTable
        BytesRead = x_fread( pRowTable, 2, Info.nChannels*Info.Height, pFile );
        if( BytesRead != Info.nChannels*Info.Height )
            goto EXIT_FAILURE;

        // Loop through the planes
        for( iPlane=0 ; iPlane<Info.nChannels ; iPlane++ )
        {
            s32 iWritePlane = iPlane;
            if( iWritePlane > BytesPerPixel-1 ) iWritePlane = BytesPerPixel-1;

            // Loop through the rows
            for( iRow=0 ; iRow<Info.Height ; iRow++ )
            {
                // Load a row
                CompressedBytes = (pRowTable[(iPlane*Info.Height+iRow)*2  ] << 8) +
                                  (pRowTable[(iPlane*Info.Height+iRow)*2+1] << 0);
                BytesRead = x_fread( pPlane, 1, CompressedBytes, pFile );
                if( BytesRead != CompressedBytes )
                    goto EXIT_FAILURE;

                // Decompress Row
                iPixel = 0;
                iByte = 0;
                while( (iPixel < Info.Width) && (iByte < CompressedBytes) )
                {
                    s8 code = (s8)pPlane[iByte++];

                    // Is it a repeat?
                    if( code < 0 )
                    {
                        Count = -(s32)code + 1;
                        Value = pPlane[iByte++];
                        while( Count-- > 0 )
                        {
                            pData[iWritePlane + (iPixel*BytesPerPixel) + (iRow*Info.Width*BytesPerPixel)] = Value;
                            iPixel++;
                        }
                    }
                    // Must be a literal then
                    else
                    {
                        Count = (s32)code + 1;
                        while( Count-- > 0 )
                        {
                            Value = pPlane[iByte++];
                            pData[iWritePlane + (iPixel*BytesPerPixel) + (iRow*Info.Width*BytesPerPixel)] = Value;
                            iPixel++;
                        }
                    }
                }

                // Confirm that we decoded the right number of bytes
                ASSERT( iByte  == CompressedBytes );
                ASSERT( iPixel == Info.Width );
            }
        }
    }
    else
        goto EXIT_FAILURE;


    // Free Buffers
    if( pPlane )    x_free( pPlane );
    if( pRowTable ) x_free( pRowTable );

    // Success!
    return( TRUE );

    // Failure!
    EXIT_FAILURE:
    if( pPlane    ) x_free( pPlane );
    if( pRowTable ) x_free( pRowTable );
    return( FALSE );
}

//==============================================================================

xbool psd_Load( xbitmap& Bitmap, const char* pFileName )
{
    psd_info    Info;
    byte        Buffer[26];
    X_FILE*     pFile = NULL;
    byte*       pData = NULL;
    byte*       pClut = NULL;
    byte*       pClutTmp = NULL;
    s32         ClutSize;
    s32         NPixels;
    s32         BytesPerPixel = 1;
    s32         DataSize;
    s32         BytesRead;
    s32         ImageResourceSize;
    s32         LayerAndMaskSize;

    xbitmap::format Format = xbitmap::FMT_NULL;

    ASSERT( pFileName );

    // Attempt to open the specified file for read.
    pFile = x_fopen( pFileName, "rb" );
    if( !pFile )
        goto EXIT_FAILURE;

    // Read the header data into the buffer.
    BytesRead = x_fread( &Buffer, 1, 26, pFile );
    if( BytesRead != 26 )
        goto EXIT_FAILURE;

    // Pick out the values we need from the header data in the buffer.
    // The file is in PC Little Endian.  Make sure to read it safely.
    Info.Signature      = ((s32)Buffer[ 0] << 24) +
                          ((s32)Buffer[ 1] << 16) +
                          ((s32)Buffer[ 2] <<  8) +
                          ((s32)Buffer[ 3] <<  0);
    Info.Version        = ((s32)Buffer[ 4] <<  8) +
                          ((s32)Buffer[ 5] <<  0);
    Info.nChannels      = ((s32)Buffer[12] <<  8) +
                          ((s32)Buffer[13] <<  0);
    Info.Height         = ((s32)Buffer[14] << 24) +
                          ((s32)Buffer[15] << 16) +
                          ((s32)Buffer[16] <<  8) +
                          ((s32)Buffer[17] <<  0);
    Info.Width          = ((s32)Buffer[18] << 24) +
                          ((s32)Buffer[19] << 16) +
                          ((s32)Buffer[20] <<  8) +
                          ((s32)Buffer[21] <<  0);
    Info.Depth          = ((s32)Buffer[22] <<  8) +
                          ((s32)Buffer[23] <<  0);
    Info.Mode           = ((s32)Buffer[24] <<  8) +
                          ((s32)Buffer[25] <<  0);

    // Fail on bad signature or version
    if( (Info.Signature != 0x38425053) || (Info.Version != 1) )
        goto EXIT_FAILURE;

    // Fail on unsupported formats.
    if( (Info.Mode != 2) && (Info.Mode != 3) )
        goto EXIT_FAILURE;

    // Only allow 1 channel on indexed images
    if( (Info.Mode == 2) && (Info.nChannels != 1) )
        goto EXIT_FAILURE;

    // Read CLUT size
    BytesRead = x_fread( &Buffer, 1, 4, pFile );
    if( BytesRead != 4 )
        goto EXIT_FAILURE;
    ClutSize = ((s32)Buffer[ 0] << 24) +
               ((s32)Buffer[ 1] << 16) +
               ((s32)Buffer[ 2] <<  8) +
               ((s32)Buffer[ 3] <<  0);

    // Read Clut for an indexed color mode, skip otherwise
    if( Info.Mode == 2 )
    {
        // Allocate and Load Clut
        pClut = (byte*)x_malloc( ClutSize );
        ASSERT( pClut );
        if( pClut == NULL )
            goto EXIT_FAILURE;
        pClutTmp = (byte*)x_malloc( ClutSize );
        ASSERT( pClutTmp );
        if( pClutTmp == NULL )
            goto EXIT_FAILURE;
        BytesRead = x_fread( pClutTmp, 1, ClutSize, pFile );
        if( BytesRead != ClutSize )
            goto EXIT_FAILURE;
        for( s32 i=0 ; i<256 ; i++ )
        {
            pClut[3*i+0] = pClutTmp[  0+i];
            pClut[3*i+1] = pClutTmp[256+i];
            pClut[3*i+2] = pClutTmp[512+i];
        }

        x_free( pClutTmp );
        pClutTmp = NULL;
    }
    else
    {
        // Skip Clut
        x_fseek( pFile, ClutSize, X_SEEK_CUR );
    }

    // Skip Image Resource Section
    BytesRead = x_fread( &Buffer, 1, 4, pFile );
    if( BytesRead != 4 )
        goto EXIT_FAILURE;
    ImageResourceSize = ((s32)Buffer[ 0] << 24) +
                        ((s32)Buffer[ 1] << 16) +
                        ((s32)Buffer[ 2] <<  8) +
                        ((s32)Buffer[ 3] <<  0);
    x_fseek( pFile, ImageResourceSize, X_SEEK_CUR );

    // Skip Layer and Mask Section
    BytesRead = x_fread( &Buffer, 1, 4, pFile );
    if( BytesRead != 4 )
        goto EXIT_FAILURE;
    LayerAndMaskSize = ((s32)Buffer[ 0] << 24) +
                       ((s32)Buffer[ 1] << 16) +
                       ((s32)Buffer[ 2] <<  8) +
                       ((s32)Buffer[ 3] <<  0);
    x_fseek( pFile, LayerAndMaskSize, X_SEEK_CUR );

    // Determine number of bytes per pixel
    switch( Info.Mode )
    {
    case 2:
        Format = xbitmap::FMT_P8_RGB_888;
        BytesPerPixel = 1;
        break;
    case 3:
        if( Info.nChannels == 3 )
        {
            Format = xbitmap::FMT_24_RGB_888;
            BytesPerPixel = 3;
        }
        else
        {
            Format = xbitmap::FMT_32_ABGR_8888;
            BytesPerPixel = 4;
        }
        break;
    }

    // Allocate pixel data.
    NPixels  = Info.Width * Info.Height;
    DataSize = NPixels * BytesPerPixel;
    pData    = (byte*)x_malloc( DataSize );
    ASSERT( pData );

    // Load the pixel data.
    if( !psd_ReadData( pData,
                       pFile,
                       Info,
                       NPixels,
                       BytesPerPixel ) )
    {
        goto EXIT_FAILURE;
    }

    // Done reading from file.
    x_fclose( pFile );

    // Set up the xbitmap.
    {
        ASSERT( Format != xbitmap::FMT_NULL );

        if( Format == xbitmap::FMT_P8_RGB_888 )
        {
            Bitmap.Setup( Format, 
                          Info.Width, 
                          Info.Height, 
                          TRUE, 
                          pData,
                          TRUE,
                          pClut );
        }
        else
        {
            Bitmap.Setup( Format, 
                          Info.Width, 
                          Info.Height, 
                          TRUE, 
                          pData );
        }
    }

    // Success!
    return( TRUE );

    // Failure!
    EXIT_FAILURE:
    if( pData )     x_free  ( pData );
    if( pClut )     x_free  ( pClut );
    if( pClutTmp )  x_free  ( pClutTmp );
    if( pFile )     x_fclose( pFile );
    return( FALSE );
}

//==============================================================================

xbool psd_Info( const char* pFileName, xbitmap::info& BitmapInfo )
{
    psd_info    Info;
    byte        Buffer[26];
	s32			BytesRead;
    X_FILE*     pFile = NULL;

    xbitmap::format Format = xbitmap::FMT_NULL;

    ASSERT( pFileName );

    // Attempt to open the specified file for read.
    pFile = x_fopen( pFileName, "rb" );
    if( !pFile )
        goto EXIT_FAILURE;

    // Read the header data into the buffer.
    BytesRead = x_fread( &Buffer, 1, 26, pFile );
    if( BytesRead != 26 )
        goto EXIT_FAILURE;

    // Pick out the values we need from the header data in the buffer.
    // The file is in PC Little Endian.  Make sure to read it safely.
    Info.Signature      = ((s32)Buffer[ 0] << 24) +
                          ((s32)Buffer[ 1] << 16) +
                          ((s32)Buffer[ 2] <<  8) +
                          ((s32)Buffer[ 3] <<  0);
    Info.Version        = ((s32)Buffer[ 4] <<  8) +
                          ((s32)Buffer[ 5] <<  0);
    Info.nChannels      = ((s32)Buffer[12] <<  8) +
                          ((s32)Buffer[13] <<  0);
    Info.Height         = ((s32)Buffer[14] << 24) +
                          ((s32)Buffer[15] << 16) +
                          ((s32)Buffer[16] <<  8) +
                          ((s32)Buffer[17] <<  0);
    Info.Width          = ((s32)Buffer[18] << 24) +
                          ((s32)Buffer[19] << 16) +
                          ((s32)Buffer[20] <<  8) +
                          ((s32)Buffer[21] <<  0);
    Info.Depth          = ((s32)Buffer[22] <<  8) +
                          ((s32)Buffer[23] <<  0);
    Info.Mode           = ((s32)Buffer[24] <<  8) +
                          ((s32)Buffer[25] <<  0);

    // Fail on bad signature or version
    if( (Info.Signature != 0x38425053) || (Info.Version != 1) )
        goto EXIT_FAILURE;

    // Fail on unsupported formats.
    if( (Info.Mode != 2) && (Info.Mode != 3) )
        goto EXIT_FAILURE;

    // Only allow 1 channel on indexed images
    if( (Info.Mode == 2) && (Info.nChannels != 1) )
        goto EXIT_FAILURE;

    // Determine number of bytes per pixel
    switch( Info.Mode )
    {
    case 2:
        Format = xbitmap::FMT_P8_RGB_888;
        break;
    case 3:
        if( Info.nChannels == 3 )
        {
            Format = xbitmap::FMT_24_RGB_888;
        }
        else
        {
            Format = xbitmap::FMT_32_ABGR_8888;
        }
        break;
    }

    // Done reading from file.
    x_fclose( pFile );

	// Fill in the xbitmap::info struct
	BitmapInfo.W      = Info.Width;
	BitmapInfo.H      = Info.Height;
	BitmapInfo.nMips  = 0;
	BitmapInfo.Format = Format;

    // Success!
    return( TRUE );

    // Failure!
    EXIT_FAILURE:
    if( pFile )     x_fclose( pFile );
    return( FALSE );
}

//==============================================================================
