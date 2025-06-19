#include "Compress.hpp"
#include "SoundPackager.hpp"
#include "ExportPackage.hpp"
#include "dsptool.hpp"
#include "Endian.hpp"
#include "..\3rdparty\miles6\include\mss.h"
#undef WAVE_FORMAT_IMA_ADPCM // CJ: Because mss.h defines this by hand, bad miles!
#include "imaadpcm.h"
#include "lame.h"

//#define PCM_ON
//#define WRITE_MP3_FILE
extern u32 g_SampleRate;

//------------------------------------------------------------------------------

u32 CompressAudioFileGCN_ADPCM( X_FILE* in, X_FILE* out, s32* NumChannels, s32* LipSyncSize )
{
    // Create appropriate audio file object based on file signature
    audio_file* AudioFile = audio_file::Create(in);
    s32         TotalCompressedSize = 0;

    if( AudioFile->Open( in ) )
    {
        xarray<audio_file::breakpoint> BreakPoints;
        s32  SampleRate         = AudioFile->GetSampleRate();
        s32  nChannels          = AudioFile->GetNumChannels();
        s32  nSamples           = AudioFile->GetNumSamples();
        s32  LoopStart          = AudioFile->GetLoopStart();
        s32  LoopEnd            = AudioFile->GetLoopEnd();
        s16* pSampleBuffer      = (s16*)x_malloc( sizeof(s16) * nSamples );
        s32  HeaderSize         = 4 * sizeof(s32) + sizeof( gcn_adpcminfo );
        s32  CompressionMethod;
        s32  i;
        s32  CompressedSize;
        s32  BreakPointSize;
        s32  nBreakPoints;

        xarray<void*> pCompressedBuffer;

        // Currently only support mono and stereo.
        ASSERT( (nChannels == 1) || (nChannels == 2) );
        g_SampleRate = SampleRate;

        // Get the breakpoints.
        AudioFile->GetBreakpoints( BreakPoints );

        // Set the number of channels.
        *NumChannels = nChannels;

        if( s_Verbose)
        {
            if( nChannels == 1 )
            {
                x_printf( "(ADPCM mono)\n" );
                x_DebugMsg( "(ADPCM mono)\n" );
            }
            else
            {
                x_printf( "(ADPCM stereo)\n" );
                x_DebugMsg( "(ADPCM stereo)\n" );
            }
        }

        // Multi-channel sound?
        if( nChannels > 1 )
        {
            CompressionMethod = GCN_ADPCM_NON_INTERLEAVED_STEREO;
        }
        else
        {
            CompressionMethod = GCN_ADPCM_MONO;
        }

        // Write out compression method.
        s32 gcn_CompressionMethod = reverse_endian_32( CompressionMethod );
        x_fwrite( &gcn_CompressionMethod, sizeof(s32), 1, out );

        // Write out the compressed size.
#ifndef PCM_ON
        CompressedSize = (dsptool_getBytesForAdpcmBuffer( nSamples )+31)&~31;
#else
        CompressedSize = sizeof(s16) * nSamples;
        CompressedSize = (CompressedSize+31) & ~31;
#endif
        s32 gcn_CompressedSize = CompressedSize;
        gcn_CompressedSize = reverse_endian_32( gcn_CompressedSize );
        x_fwrite( &gcn_CompressedSize, sizeof(s32), 1, out );

        // Write out the lip sync size in bytes.
        *LipSyncSize = GetLipSyncSize( nSamples, SampleRate );
        s32 gcn_LipSyncSize = *LipSyncSize;
        gcn_LipSyncSize = reverse_endian_32( gcn_LipSyncSize );
        x_fwrite( &gcn_LipSyncSize, sizeof(s32), 1, out );

        // Write out the break point size in byte.
        BreakPointSize = GetBreakPointSize( BreakPoints, nBreakPoints );
        s32 gcn_BreakPointSize = BreakPointSize;
        gcn_BreakPointSize = reverse_endian_32( gcn_BreakPointSize );
        x_fwrite( &gcn_BreakPointSize, sizeof(s32), 1, out );

        // Write out header size in bytes.
        s32 gcn_HeaderSize = reverse_endian_32( HeaderSize );
        x_fwrite( &gcn_HeaderSize, sizeof(s32), 1, out );

        // Compress each channel...
        for( i=0 ; i<nChannels ; i++ )
        {
            gcn_adpcminfo ADPCM;

            // Allocate space for the compressed data.
            pCompressedBuffer.Append() = x_malloc( CompressedSize );

            // Read the the uncompressed waveform data.
            AudioFile->GetChannelData( pSampleBuffer, i );

#ifdef PCM_ON
            for( s32 k=0 ; k<nSamples ; k++ )
            {
                pSampleBuffer[k] = reverse_endian_16( pSampleBuffer[k] );
            }
#endif

            // Nuke the ADPCM header.
            x_memset( &ADPCM, 0, sizeof(gcn_adpcminfo) );
            x_memset( pCompressedBuffer[i], 0, CompressedSize );

            // ADPCM Compress the sample.
#ifndef PCM_ON
            dsptool_encode( (s16*)pSampleBuffer, (u8*)pCompressedBuffer[i], &ADPCM, nSamples );
#else
            x_memcpy( pCompressedBuffer[i], pSampleBuffer, CompressedSize );
#endif
            if( AudioFile->IsLooped() )
            {
                if( LoopEnd > 1 )
                    LoopEnd--;
#ifndef PCM_ON
                dsptool_getLoopContext( (u8*)pCompressedBuffer[i], &ADPCM, LoopStart );
#endif
            }
            else
            {
                LoopStart = LoopEnd = 0;
            }

            ADPCM.gain              = 0;
            ADPCM.pred_scale        = (u16)*((u8*)pCompressedBuffer[i]);
            ADPCM.yn1               = 0;
            ADPCM.yn2               = 0;

            // Write out the number of samples, sample rate and loop points
            s32 gcn_nSamples     = reverse_endian_32( nSamples );
            s32 gcn_SampleRate   = reverse_endian_32( SampleRate );
            s32 gcn_LoopStart    = reverse_endian_32( LoopStart );
            s32 gcn_LoopEnd      = reverse_endian_32( LoopEnd );
            x_fwrite( &gcn_nSamples,     sizeof(s32), 1, out );
            x_fwrite( &gcn_SampleRate,   sizeof(s32), 1, out );
            x_fwrite( &gcn_LoopStart,    sizeof(s32), 1, out );
            x_fwrite( &gcn_LoopEnd,      sizeof(s32), 1, out );

            // Byteswap ADPCM header
            for( s32 j=0 ; j<16 ; j++ )
            {
                ADPCM.coef[j] = reverse_endian_16( ADPCM.coef[j] );
            }

            ADPCM.gain            = reverse_endian_16( ADPCM.gain );
            ADPCM.pred_scale      = reverse_endian_16( ADPCM.pred_scale );
            ADPCM.yn1             = reverse_endian_16( ADPCM.yn1 );
            ADPCM.yn2             = reverse_endian_16( ADPCM.yn2 );
            ADPCM.loop_pred_scale = reverse_endian_16( ADPCM.loop_pred_scale );
            ADPCM.loop_yn1        = reverse_endian_16( ADPCM.loop_yn1 );
            ADPCM.loop_yn2        = reverse_endian_16( ADPCM.loop_yn2 );

            // Write out the header
            x_fwrite( &ADPCM, sizeof(ADPCM), 1, out );
        }

        // Free buffer.
        x_free( pSampleBuffer );

        // Now write out each channels waveform data.
        for( i=0 ; i<nChannels ; i++ )
        {
            // Keep track of total compressed size.
            TotalCompressedSize += CompressedSize;

            // Write out the compressed data.
            x_fwrite( pCompressedBuffer[i], CompressedSize, 1, out );
            
            // Free the compressed buffer memory.
            x_free( pCompressedBuffer[i] );
        }

        // Write out the lip sync data.
        WriteLipSyncData( AudioFile, out );

        // Write out the break points.
        WriteBreakPoints( BreakPoints, out, TRUE );
        
        // Close and delete audio file
        AudioFile->Close();
        delete AudioFile;
    }

    return TotalCompressedSize;
}

//------------------------------------------------------------------------------

u32 CompressAudioFileGCN_MP3( X_FILE* in, X_FILE* out, s32* NumChannels, s32* LipSyncSize )
{
    // Create appropriate audio file object based on file signature
    audio_file* AudioFile = audio_file::Create(in);
    s32         TotalCompressedSize = 0;

    if( AudioFile->Open( in ) )
    {
        xarray<audio_file::breakpoint> BreakPoints;
        s32  SampleRate         = AudioFile->GetSampleRate();
        s32  nChannels          = AudioFile->GetNumChannels();
        s32  nSampleAdjust      = 0;
        s32  nBytesIgnore       = 0xd8;
        s32  nSamples           = AudioFile->GetNumSamples();
        s32  nSamplesPadded     = nSamples + nSampleAdjust;
        s32  LoopStart          = AudioFile->GetLoopStart();
        s32  LoopEnd            = AudioFile->GetLoopEnd();
        s16* pSampleBuffer      = NULL;
        s16* pSampleBufferL     = NULL;
        s16* pSampleBufferR     = NULL;
        s32  HeaderSize         = 4 * sizeof(s32);
        s32  CompressionMethod;
        s32  i;
        s32  BreakPointSize;
        s32  nBreakPoints;

        void* pCompressedBuffer;
        s32   CompressedSize;

        // Get the breakpoints.
        AudioFile->GetBreakpoints( BreakPoints );

        // Set the number of channels.
        *NumChannels = nChannels;
        g_SampleRate = SampleRate;

        if( nChannels == 1 )
        {
            if( s_Verbose)
            {
                x_printf( "(MP3 mono)\n" );
                x_DebugMsg( "(MP3 mono)\n" );
            }

            // Set it.
            CompressionMethod = GCN_MP3_MONO;

            // Write out compression method.
            s32 gcn_CompressionMethod = reverse_endian_32( CompressionMethod );
            x_fwrite( &gcn_CompressionMethod, sizeof(s32), 1, out );

            // Allocate a buffer for the source data and clear it
            pSampleBuffer = (s16*)x_malloc( sizeof(s16) * nSamplesPadded );
            x_memset( pSampleBuffer, 0, sizeof(s16) * nSamplesPadded );

            // Read the uncompressed waveform data.
            AudioFile->GetChannelData( pSampleBuffer + nSampleAdjust, 0 );

            // Allocate buffer for compressed data
            s32 CompressedDataSize = (s32)(nSamplesPadded * 1.25f + 7200);
            u8* pCompressedData = (u8*)x_malloc( CompressedDataSize );
            ASSERT( pCompressedData );
            pCompressedBuffer = pCompressedData;

            // Compress to MP3
            lame_global_flags* gfp = lame_init();
            lame_set_num_samples        ( gfp, nSamplesPadded );
            lame_set_num_channels       ( gfp, 1 );
            lame_set_in_samplerate      ( gfp, SampleRate );
            lame_set_out_samplerate     ( gfp, SampleRate );
            lame_set_quality            ( gfp, 0 );             // TODO: Set this from parameters
            lame_set_mode               ( gfp, MONO );
            lame_set_compression_ratio  ( gfp, 11.0f );
            s32 Inited = lame_init_params( gfp );
            ASSERT( Inited != -1 );

            s32 Size = lame_encode_buffer( gfp, pSampleBuffer, NULL, nSamplesPadded, pCompressedData, CompressedDataSize );
            ASSERT( Size >= 0 );
            Size += lame_encode_finish( gfp, pCompressedData+Size, CompressedDataSize-Size );

            CompressedSize = Size;

            if( AudioFile->IsLooped() )
            {
                if( LoopEnd > 1 )
                    LoopEnd--;
            }
            else
            {
                LoopStart = LoopEnd = 0;
            }

            // Write out the compressed size.
            CompressedSize -= nBytesIgnore;
            s32 gcn_CompressedSize = reverse_endian_32( CompressedSize );
            x_fwrite( &gcn_CompressedSize, sizeof(s32), 1, out );

            // Write out the lip sync size in bytes.
            *LipSyncSize = GetLipSyncSize( nSamples, SampleRate );
            s32 gcn_LipSyncSize = *LipSyncSize;
            gcn_LipSyncSize = reverse_endian_32( gcn_LipSyncSize );
            x_fwrite( &gcn_LipSyncSize, sizeof(s32), 1, out );

            // Write out the break point size in byte.
            BreakPointSize = GetBreakPointSize( BreakPoints, nBreakPoints );
            s32 gcn_BreakPointSize = reverse_endian_32( BreakPointSize );
            x_fwrite( &gcn_BreakPointSize, sizeof(s32), 1, out );

            // Write out header size in bytes.
            s32 gcn_HeaderSize = reverse_endian_32( HeaderSize );
            x_fwrite( &gcn_HeaderSize, sizeof(s32), 1, out );

            // Write out the number of samples, sample rate and loop points
            s32 gcn_nSamples     = reverse_endian_32( nSamples );
            s32 gcn_SampleRate   = reverse_endian_32( SampleRate );
            s32 gcn_LoopStart    = reverse_endian_32( LoopStart );
            s32 gcn_LoopEnd      = reverse_endian_32( LoopEnd );
            x_fwrite( &gcn_nSamples,   sizeof(s32), 1, out );
            x_fwrite( &gcn_SampleRate, sizeof(s32), 1, out );
            x_fwrite( &gcn_LoopStart,  sizeof(s32), 1, out );
            x_fwrite( &gcn_LoopEnd,    sizeof(s32), 1, out );

            // Free buffer.
            x_free( pSampleBuffer );

            // Keep track of total compressed size.
            TotalCompressedSize += CompressedSize;

            // Write out the compressed data.
            pCompressedBuffer = (void*)((u32)pCompressedBuffer + nBytesIgnore);
            x_fwrite( pCompressedBuffer, CompressedSize, 1, out );

            // Debug code to write out the compressed MP3 as a file
#ifdef WRITE_MP3_FILE
            static s32 nFile = 0;
            xbytestream b;
            b.Append( pCompressedBuffer, CompressedSize );
            b.SaveFile( MP3Filename );
#endif

            // Free the compressed buffer memory.
            pCompressedBuffer = (void*)((u32)pCompressedBuffer - nBytesIgnore);
            x_free( pCompressedBuffer );
        }
        else if( nChannels == 2 )
        {
            if( s_Verbose)
            {
                x_printf( "(MP3 stereo)\n" );
                x_DebugMsg( "(MP3 stereo)\n" );
            }

            // Set it.
            CompressionMethod = GCN_MP3_INTERLEAVED_STEREO;

            // Write out compression method.
            s32 gcn_CompressionMethod = reverse_endian_32( CompressionMethod );
            x_fwrite( &gcn_CompressionMethod, sizeof(s32), 1, out );

            // Allocate a buffer for the source data and clear it
            pSampleBufferL = (s16*)x_malloc( sizeof(s16) * nSamplesPadded );
            x_memset( pSampleBufferL, 0, sizeof(s16) * nSamplesPadded );
            pSampleBufferR = (s16*)x_malloc( sizeof(s16) * nSamplesPadded );
            x_memset( pSampleBufferR, 0, sizeof(s16) * nSamplesPadded );

            // Read the uncompressed waveform data.
            AudioFile->GetChannelData( pSampleBufferL+nSampleAdjust, 0 );
            AudioFile->GetChannelData( pSampleBufferR+nSampleAdjust, 1 );

            // Allocate buffer for compressed data
            s32 CompressedDataSize = (s32)(nSamplesPadded * 1.25f + 7200);
            u8* pCompressedData = (u8*)x_malloc( CompressedDataSize );
            ASSERT( pCompressedData );
            pCompressedBuffer = pCompressedData;

            // Compress to MP3
            lame_global_flags* gfp = lame_init();
            lame_set_num_samples        ( gfp, nSamplesPadded );
            lame_set_num_channels       ( gfp, 2 );
            lame_set_in_samplerate      ( gfp, SampleRate );
            lame_set_out_samplerate     ( gfp, SampleRate );
            lame_set_quality            ( gfp, 0 );             // TODO: Set this from parameters
            lame_set_mode               ( gfp, STEREO );
            lame_set_compression_ratio  ( gfp, 11.0f );
            s32 Inited = lame_init_params( gfp );
            ASSERT( Inited != -1 );

            s32 Size = lame_encode_buffer( gfp, pSampleBufferL, pSampleBufferR, nSamplesPadded, pCompressedData, CompressedDataSize );
            ASSERT( Size >= 0 );
            Size += lame_encode_finish( gfp, pCompressedData+Size, CompressedDataSize-Size );

            CompressedSize = Size;

            if( AudioFile->IsLooped() )
            {
                if( LoopEnd > 1 )
                    LoopEnd--;
            }
            else
            {
                LoopStart = LoopEnd = 0;
            }

            // Write out the compressed size.
            CompressedSize -= nBytesIgnore;
            s32 gcn_CompressedSize = reverse_endian_32( CompressedSize );
            x_fwrite( &gcn_CompressedSize, sizeof(s32), 1, out );

            // Write out the lip sync size in bytes.
            *LipSyncSize = GetLipSyncSize( nSamples, SampleRate );
            s32 gcn_LipSyncSize = *LipSyncSize;
            gcn_LipSyncSize = reverse_endian_32( gcn_LipSyncSize );
            x_fwrite( &gcn_LipSyncSize, sizeof(s32), 1, out );

            // Write out the break point size in byte.
            BreakPointSize = GetBreakPointSize( BreakPoints, nBreakPoints );
            s32 gcn_BreakPointSize = reverse_endian_32( BreakPointSize );
            x_fwrite( &gcn_BreakPointSize, sizeof(s32), 1, out );

            // Write out header size in bytes.
            s32 gcn_HeaderSize = reverse_endian_32( HeaderSize );
            x_fwrite( &gcn_HeaderSize, sizeof(s32), 1, out );

            // Compress each channel...
            for( i=0 ; i<nChannels ; i++ )
            {
                // Write out the number of samples, sample rate and loop points
                s32 gcn_nSamples     = reverse_endian_32( nSamples );
                s32 gcn_SampleRate   = reverse_endian_32( SampleRate );
                s32 gcn_LoopStart    = reverse_endian_32( LoopStart );
                s32 gcn_LoopEnd      = reverse_endian_32( LoopEnd );
                x_fwrite( &gcn_nSamples,   sizeof(s32), 1, out );
                x_fwrite( &gcn_SampleRate, sizeof(s32), 1, out );
                x_fwrite( &gcn_LoopStart,  sizeof(s32), 1, out );
                x_fwrite( &gcn_LoopEnd,    sizeof(s32), 1, out );
            }

            // Free buffer.
            x_free( pSampleBufferL );
            x_free( pSampleBufferR );

            // Keep track of total compressed size.
            TotalCompressedSize += CompressedSize;

            // Write out the compressed data.
            pCompressedBuffer = (void*)((u32)pCompressedBuffer + nBytesIgnore);
            x_fwrite( pCompressedBuffer, CompressedSize, 1, out );

            // Debug code to write out the compressed MP3 as a file
#ifdef WRITE_MP3_FILE
            static s32 nFile = 0;
            xbytestream b;
            b.Append( pCompressedBuffer, CompressedSize );
            b.SaveFile( MP3Filename );
#endif

            // Free the compressed buffer memory.
            pCompressedBuffer = (void*)((u32)pCompressedBuffer - nBytesIgnore);
            x_free( pCompressedBuffer );
        }
        else
        {
            ASSERT(0);  // Unsupported number of channels
        }

        // Write out the lip sync data.
        WriteLipSyncData( AudioFile, out );

        // Write out the break points.
        WriteBreakPoints( BreakPoints, out, TRUE );
        
        // Close and delete audio file
        AudioFile->Close();
        delete AudioFile;
    }

    return TotalCompressedSize;
}

//------------------------------------------------------------------------------
